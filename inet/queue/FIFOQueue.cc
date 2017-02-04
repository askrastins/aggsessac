#include "INETDefs.h"

#include "FIFOQueue.h"


Define_Module(FIFOQueue);

simsignal_t FIFOQueue::queueLengthSignal = registerSignal("queueLength");

void FIFOQueue::initialize()
{
    PassiveQueueBase2::initialize();
    queue.setName(par("queueName"));
    outGate = gate("out");
}

cMessage *FIFOQueue::enqueue(cMessage *msg)
{
    cPacket *packet = check_and_cast<cPacket*>(msg);
    queue.insert(packet);
    byteLength += packet->getByteLength();
    emit(queueLengthSignal, queue.length());
    return NULL;
}

cMessage *FIFOQueue::dequeue()
{
    if (queue.empty())
        return NULL;

    cPacket *packet = check_and_cast<cPacket*>(queue.pop());
    byteLength -= packet->getByteLength();
    emit(queueLengthSignal, queue.length());
    return packet;
}

void FIFOQueue::sendOut(cMessage *msg)
{
    send(msg, outGate);
}

bool FIFOQueue::isEmpty()
{
    return queue.empty();
}

