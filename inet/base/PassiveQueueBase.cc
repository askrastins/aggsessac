#include <algorithm>

#include "PassiveQueueBase.h"

#include "INETDefs.h"
#include "DiffservUtil.h"
#include "IPv4Datagram.h"

simsignal_t PassiveQueueBase::rcvdPkSignal = registerSignal("rcvdPk");
simsignal_t PassiveQueueBase::enqueuePkSignal = registerSignal("enqueuePk");
simsignal_t PassiveQueueBase::dequeuePkSignal = registerSignal("dequeuePk");
simsignal_t PassiveQueueBase::dropPkByQueueSignal = registerSignal("dropPkByQueue");
simsignal_t PassiveQueueBase::queueingTimeSignal = registerSignal("queueingTime");

void PassiveQueueBase::initialize()
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

void PassiveQueueBase::handleMessage(cMessage *msg)
{
    numQueueReceived++;

    emit(rcvdPkSignal, msg);

    if (packetRequested > 0)
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
        cMessage *droppedMsg = enqueue(msg);
        if (msg != droppedMsg)
            emit(enqueuePkSignal, msg);

        if (droppedMsg)
        {
            numQueueDropped++;
            emit(dropPkByQueueSignal, droppedMsg);
            delete droppedMsg;
        }
        else
            notifyListeners();
    }

    if (ev.isGUI())
    {
        char buf[40];
        sprintf(buf, "q rcvd: %d\nq dropped: %d", numQueueReceived, numQueueDropped);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

void PassiveQueueBase::requestPacket()
{
    Enter_Method("requestPacket()");

    cMessage *msg = dequeue();
    if (msg == NULL)
    {
        packetRequested++;
    }
    else
    {
        emit(dequeuePkSignal, msg);
        emit(queueingTimeSignal, simTime() - msg->getArrivalTime());
        sendOut(msg);
    }
}

void PassiveQueueBase::clear()
{
    cMessage *msg;

    while (NULL != (msg = dequeue()))
        delete msg;

    packetRequested = 0;
}

cMessage *PassiveQueueBase::pop()
{
    return dequeue();
}

void PassiveQueueBase::finish()
{
}

void PassiveQueueBase::addListener(IPassiveQueueListener *listener)
{
    std::list<IPassiveQueueListener*>::iterator it = find(listeners.begin(), listeners.end(), listener);
    if (it == listeners.end())
        listeners.push_back(listener);
}

void PassiveQueueBase::removeListener(IPassiveQueueListener *listener)
{
    std::list<IPassiveQueueListener*>::iterator it = find(listeners.begin(), listeners.end(), listener);
    if (it != listeners.end())
        listeners.erase(it);
}

void PassiveQueueBase::notifyListeners()
{
    for (std::list<IPassiveQueueListener*>::iterator it = listeners.begin(); it != listeners.end(); ++it)
        (*it)->packetEnqueued(this);
}

