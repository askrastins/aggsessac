
#include <algorithm>

#include "PassiveQueueBase2.h"

//add this
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

void PassiveQueueBase2::handleMessage(cMessage *msg) // are triggered when packet is in queue
{
    //timer self-msg message can not be included in queue statistic
    if (msg->isSelfMessage())
    {
        cMessage *timerMsg = enqueue(msg);
        if (msg != timerMsg){} // nothing
    }
    else
    {
        numQueueReceived++;
        emit(rcvdPkSignal, msg);

        if (packetRequested > 0)    // if queue is empty - no packetRequested ...
            packetRequested--;
            emit(enqueuePkSignal, msg);
            emit(dequeuePkSignal, msg);
            emit(queueingTimeSignal, SIMTIME_ZERO);
            sendOut(msg);
        }
        else
        {
            msg->setArrivalTime(simTime());
            cMessage *droppedMsg = enqueue(msg); //insert packet in queue, (enqueue), doesn't return packet
            if (msg != droppedMsg)
                emit(enqueuePkSignal, msg);
            if (droppedMsg)                     // remove if packet returned - only if Queue is overloaded 
            {
                numQueueDropped++;
                emit(dropPkByQueueSignal, droppedMsg);
                delete droppedMsg;
            }
            else
            notifyListeners(); // notification for ScheduleBase::packetEnqueued(), that queue is not empty
        }

        if (ev.isGUI())
        {
            char buf[40];
            sprintf(buf, "q rcvd: %d\nq dropped: %d", numQueueReceived, numQueueDropped);
            getDisplayString().setTagArg("t", 0, buf);
        }
    }
}

void PassiveQueueBase2::requestPacket() // called, when scheduler request packet
{
    Enter_Method("requestPacket()");

    cMessage *msg = dequeue();
    if (msg == NULL)
    {
        packetRequested++; // case with AggSessAC this is not called, because requestPacket() is called based on numOfAcceptedPkt > 0 counter
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

