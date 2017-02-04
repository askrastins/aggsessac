#ifndef __INET_PRIORITYSCHEDULER1_H_
#define __INET_PRIORITYSCHEDULER1_H_

#include <omnetpp.h>
#include "INETDefs.h"
#include "PriorityScheduler1.h"
#include "SchedulerBase.h"
// cooperation with aggsessACQueue to read numOfAcceptedPkt counter
#include "AggSessACQueue.h" 


/**
 * Schedule packets in strict priority order. 
 * Work the similar as priorityQueue, but requestPacket from first queue only when counter numOfAcceptedPkt>0 
 * Packets arrived at the 0th gate has the higher priority.
 */
class INET_API PriorityScheduler1 : public SchedulerBase
{
  protected:
    virtual bool schedulePacket();
};

#endif
