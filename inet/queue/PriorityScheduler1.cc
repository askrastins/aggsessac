/**
* Action customized to meet the AggSessAC needs
* requestPacket() from aggsessACQueue according to bundle size and waiting time triger 
* read aggsessACQueue only when counter AggSessACQueue::numOfAcceptedPkt > 0
*/

#include "PriorityScheduler1.h"

Define_Module(PriorityScheduler1);

bool PriorityScheduler1::schedulePacket()
{
    int i = 0;
    //double wt = AggSessACQueue::nextProcessTime;
    //simtime_t currentTime = simTime();
    for (std::vector<IPassiveQueue*>::iterator it = inputQueues.begin(); it != inputQueues.end(); ++it)
    {
        IPassiveQueue *inputQueue = *it;
        int a = AggSessACQueue::numOfAcceptedPkt;


        if (!inputQueue->isEmpty() && i == 0) //  Each time Scheduler the first check AggSessACQueue, requestPacket() send to aggSessACQueue only if a=0   
           {
            if (a > 0 )
            {
                inputQueue->requestPacket();
                return true;
            }
           }
        if (!inputQueue->isEmpty() && i > 0)
        {
            inputQueue->requestPacket();
            return true;
        }
        i++;

    }
    return false;
}
