//
// Copyright (C) 2005 Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include <algorithm>

#include "PassiveQueueBase2.h"

//Piesaistu papildus
#include "DiffservUtil.h"
#include "IPv4Datagram.h"
#include "UtilizationMeter.h"
#include "DataThrQueue.h"

simsignal_t PassiveQueueBase2::rcvdPkSignal = registerSignal("rcvdPk");
simsignal_t PassiveQueueBase2::enqueuePkSignal = registerSignal("enqueuePk");
simsignal_t PassiveQueueBase2::dequeuePkSignal = registerSignal("dequeuePk");
simsignal_t PassiveQueueBase2::dropPkByQueueSignal = registerSignal("dropPkByQueue");
simsignal_t PassiveQueueBase2::queueingTimeSignal = registerSignal("queueingTime");

void PassiveQueueBase2::initialize()
{
    // state
    packetRequested = 0;
    WATCH(packetRequested);

    // statistics
    numQueueReceived = 0;
    numQueueDropped = 0;
    WATCH(numQueueReceived);
    WATCH(numQueueDropped);
}

void PassiveQueueBase2::handleMessage(cMessage *msg) // izpildâs, kad pakete pienâkusi rindâ
{
    //ðitais IF - lai timer self-msg packets netiktu ietvertas queue statistikâ
    if (msg->isSelfMessage())
    {
        cMessage *timerMsg = enqueue(msg);
        if (msg != timerMsg){} // nedaram neko
    }
    else
    {
        numQueueReceived++;
        emit(rcvdPkSignal, msg);

        if (packetRequested > 0)    // ðitam nevajedzçtu nekad izpildîties, izpildâs ja schedulers, ir pieprasîjis paketi, bet nevienâ rindâ tâ nav bijusi..
        {
            packetRequested--;
            emit(enqueuePkSignal, msg);
            emit(dequeuePkSignal, msg);
            emit(queueingTimeSignal, SIMTIME_ZERO);
            sendOut(msg);
        }
        else
        {
            msg->setArrivalTime(simTime());
            cMessage *droppedMsg = enqueue(msg); //ievietojam paketi rindâ, ja enqueue neatgrieþ paketi atpakaï, tad pakete ir veiksmîgi ievietota rindâ
            if (msg != droppedMsg)
                emit(enqueuePkSignal, msg);
            if (droppedMsg)                     // ja pakete tika atgriezta atpakaï, izdzçðam to
            {
                numQueueDropped++;
                emit(dropPkByQueueSignal, droppedMsg);
                delete droppedMsg;
            }
            else
            notifyListeners(); // paredzçta info nodoðanai: par jaunas pkts ieraðanos rindâ tiek paziòots ScheduleBase::packetEnqueued metodei - kas izlemj vai paketi var uzreiz pieprasît vei arî jânogaida vçl
        }

        if (ev.isGUI())
        {
            char buf[40];
            sprintf(buf, "q rcvd: %d\nq dropped: %d", numQueueReceived, numQueueDropped);
            getDisplayString().setTagArg("t", 0, buf);
        }
    }
}

void PassiveQueueBase2::requestPacket() // izpildâs, kad schedulers pieprasa paketi
{
    Enter_Method("requestPacket()");

    cMessage *msg = dequeue();
    if (msg == NULL)
    {
        packetRequested++; // ðitam AggSessAC gadijumaa nevajedzçtu nekad izpildîties jo requestPacket() var izsaukt ja rindâ ir numOfAcceptedPkt > 0
    }
    else
    {
        //cPacket *packet = check_and_cast<cPacket*>(msg);
        //packet  = packet->getEncapsulatedPacket();
        //IPv4Datagram *ipv4Datagram = dynamic_cast<IPv4Datagram*>(packet);
        //if (ipv4Datagram){
        emit(dequeuePkSignal, msg);
        emit(queueingTimeSignal, simTime() - msg->getArrivalTime());
        sendOut(msg);
    }
}

void PassiveQueueBase2::clear()
{
    cMessage *msg;

    while (NULL != (msg = dequeue()))
        delete msg;

    packetRequested = 0;
}

cMessage *PassiveQueueBase2::pop()
{
    return dequeue();
}

void PassiveQueueBase2::finish()
{
}

void PassiveQueueBase2::addListener(IPassiveQueueListener *listener)
{
    std::list<IPassiveQueueListener*>::iterator it = find(listeners.begin(), listeners.end(), listener);
    if (it == listeners.end())
        listeners.push_back(listener);
}

void PassiveQueueBase2::removeListener(IPassiveQueueListener *listener)
{
    std::list<IPassiveQueueListener*>::iterator it = find(listeners.begin(), listeners.end(), listener);
    if (it != listeners.end())
        listeners.erase(it);
}

void PassiveQueueBase2::notifyListeners()
{
    for (std::list<IPassiveQueueListener*>::iterator it = listeners.begin(); it != listeners.end(); ++it)
        (*it)->packetEnqueued(this);
}

