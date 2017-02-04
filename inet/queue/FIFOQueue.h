#ifndef __INET_FIFOQUEUE_H
#define __INET_FIFOQUEUE_H

#include "INETDefs.h"
#include "PassiveQueueBase2.h"
#include "IQueueAccess.h"

/**
 * Passive FIFO Queue with unlimited buffer space.
 */
class INET_API FIFOQueue : public PassiveQueueBase2, public IQueueAccess
{
  protected:
    // state
    cQueue queue;
    cGate *outGate;
    int byteLength;

    // statistics
    static simsignal_t queueLengthSignal;

  public:
    FIFOQueue() : outGate(NULL), byteLength(0) {}

  protected:
    virtual void initialize();

    virtual cMessage *enqueue(cMessage *msg);

    virtual cMessage *dequeue();

    virtual void sendOut(cMessage *msg);

    virtual bool isEmpty();

    virtual int getLength() const { return queue.getLength(); }

    virtual int getByteLength() const { return byteLength; }
};

#endif
