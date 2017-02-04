package inet.linklayer.queue;

//
// This module implements a strict priority scheduler.
// Work the similar as priorityQueue
// Packets arrived at the 0th gate has the higher priority, then 1st and so on.
//
simple PriorityScheduler1
{
    @display("i=block/server");

    gates:
        input in[];
        output out;
}

