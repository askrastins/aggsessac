package inet.linklayer.queue;

import inet.linklayer.IOutputQueue;

// 
// Queue returns a packet from the queue, when 'bundle size' or 'max waiting time' are reached
// Used in network interfaces.
// Realize Connection Admission Control (CAC) algorithm.
//
simple AggSessACQueue like IOutputQueue
{
    parameters:
        int frameCapacity = default(100);
        int maxBundleSize = default(10); // maximum size of bundle
        double maxDelay = default(0.01); // default=10ms (maximum allowed time of aggregation period)
        string queueName = default("l2queue"); // name of the inner cQueue object, used in the 'q' tag of the display string
        @display("i=block/queue");
        @signal[rcvdPk](type=cPacket);
        @signal[enqueuePk](type=cPacket);
        @signal[dequeuePk](type=cPacket);
        @signal[dropPkByQueue](type=cPacket);
        @signal[queueingTime](type=simtime_t; unit=s);
        @signal[queueLength](type=long);
        @signal[numOfRejectedFlowsByDecision](type=long);
        @signal[acceptedFlowsCount](type=long);
        @signal[totalqueueLength](type=long);
        @signal[waitingTime](type=double);
        @signal[nextProcessTime](type=double); 
        @signal[timer](type=double);    
        @signal[accepted_tosOfSYNPk](type=long);
        @signal[accepted_reqBwOfSYNPk](type=long);
        @signal[rejected_tosOfSYNPk](type=long);
        @signal[rejected_reqBwOfSYNPk](type=long);
        @signal[cumulative_accepted](type=long);
        @signal[cumulative_rejected](type=long);
        @signal[cumulative_revenue](type=long);
       
        @statistic[rcvdPk](title="received packets"; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[dropPk](title="dropped packets"; source=dropPkByQueue; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[queueingTime](title="queueing time"; record=histogram,vector; interpolationmode=none);
        @statistic[queueLength](title="queue length"; record=histogram,max,timeavg,vector; interpolationmode=sample-hold);
        @statistic[numOfRejectedFlowsByDecision](title="rejected flows each decision"; source=numOfRejectedFlowsByDecision; record=histogram,max,timeavg,vector; interpolationmode=none);
        @statistic[numOfAcceptedFlowsByDecision](title="accepted flows each decision"; source=numOfAcceptedFlowsByDecision; record=histogram,max,timeavg,vector; interpolationmode=none);
        @statistic[totalqueueLength](title="Total queue length"; source=totalqueueLength; record=histogram,max,timeavg,vector; interpolationmode=none);
        @statistic[waitingTime](title="waitingTime"; record=histogram,vector; interpolationmode=none);
        @statistic[nextProcessTime](title="nextProcessTime"; record=histogram,vector; interpolationmode=none);
        @statistic[timer](title="timer"; source=timer; record=histogram,vector; interpolationmode=none);
        @statistic[accepted_tosOfSYNPk](title="ToS in SYN packet"; source=accepted_tosOfSYNPk; record=histogram, vector; interpolationmode=none);
        @statistic[accepted_reqBwOfSYNPk](title="reqBw in SNY packet"; source=accepted_reqBwOfSYNPk; record=histogram, vector; interpolationmode=none);
        @statistic[rejected_tosOfSYNPk](title="rejected ToS"; source=rejected_tosOfSYNPk; record=histogram,vector; interpolationmode=none);
        @statistic[rejected_reqBwOfSYNPk](title="rejected reqBw"; source=rejected_reqBwOfSYNPk; record=histogram,vector; interpolationmode=none);
        @statistic[cumulative_accepted](title="cumulative accepted flows"; source=cumulative_accepted; record=count,vector; interpolationmode=none);
        @statistic[cumulative_rejected](title="cumulative rejected flows"; source=cumulative_rejected; record=count,vector; interpolationmode=none);
        @statistic[cumulative_revenue](title="cumulative sum of accepted ToS value"; source=cumulative_revenue; record= vector; interpolationmode=none);
        
    gates:
        input in;
        output out;
}

