package inet.linklayer.queue;

//
// This module implements a passive FIFO queue
// with unlimited buffer space. It can be combined
// with algorithmic droppers and schedulers to form
// an IOutputQueue compound module.
//
// The C++ class implements the IQueueAccess and IPassiveQueue interfaces.
//
simple FIFOQueue
{
    parameters:
        string queueName = default("l2queue"); // name of the cQueue object, used in the 'q' tag of the display string
        @display("i=block/passiveq");
        @signal[rcvdPk](type=cPacket);
        @signal[enqueuePk](type=cPacket);
        @signal[dequeuePk](type=cPacket);
        @signal[dropPkByQueue](type=cPacket);
        @signal[queueingTime](type=simtime_t; unit=s);
        @signal[queueLength](type=long);
        @statistic[rcvdPk](title="received packets"; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[dropPk](title="dropped packets"; source=dropPkByQueue; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[queueingTime](title="queueing time"; record=histogram,vector; interpolationmode=none);
        @statistic[queueLength](title="queue length"; record=max,timeavg,vector; interpolationmode=sample-hold);
    gates:
        input in[];
        output out;
}
