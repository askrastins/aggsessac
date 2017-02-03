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

void PassiveQueueBase2::handleMessage(cMessage *msg) // izpild�s, kad pakete pien�kusi rind�
{
    //�itais IF - lai timer self-msg packets netiktu ietvertas queue statistik�
    if (msg->isSelfMessage())
    {
        cMessage *timerMsg = enqueue(msg);
        if (msg != timerMsg){} // nedaram neko
    }
    else
    {
        numQueueReceived++;
        emit(rcvdPkSignal, msg);

        if (packetRequested > 0)    // �itam nevajedz�tu nekad izpild�ties, izpild�s ja schedulers, ir piepras�jis paketi, bet nevien� rind� t� nav bijusi..
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
            cMessage *droppedMsg = enqueue(msg); //ievietojam paketi rind�, ja enqueue neatgrie� paketi atpaka�, tad pakete ir veiksm�gi ievietota rind�
            if (msg != droppedMsg)
                emit(enqueuePkSignal, msg);
            if (droppedMsg)                     // ja pakete tika atgriezta atpaka�, izdz��am to
            {
                numQueueDropped++;
                emit(dropPkByQueueSignal, droppedMsg);
                delete droppedMsg;
            }
            else
            notifyListeners(); // paredz�ta info nodo�anai: par jaunas pkts iera�anos rind� tiek pazi�ots ScheduleBase::packetEnqueued metodei - kas izlemj vai paketi var uzreiz piepras�t vei ar� j�nogaida v�l
        }

        if (ev.isGUI())
        {
            char buf[40];
            sprintf(buf, "q rcvd: %d\nq dropped: %d", numQueueReceived, numQueueDropped);
            getDisplayString().setTagArg("t", 0, buf);
        }
    }
}

void PassiveQueueBase2::requestPacket() // izpild�s, kad schedulers pieprasa paketi
{
    Enter_Method("requestPacket()");

    cMessage *msg = dequeue();
    if (msg == NULL)
    {
        packetRequested++; // �itam AggSessAC gadijumaa nevajedz�tu nekad izpild�ties jo requestPacket() var izsaukt ja rind� ir numOfAcceptedPkt > 0
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

