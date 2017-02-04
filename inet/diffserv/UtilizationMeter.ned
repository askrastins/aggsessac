package inet.networklayer.diffserv;

simple UtilizationMeter
{
    parameters:
        @display("i=block/timer");
		string maxAvailBw;		// Max available traffic load
		string upperThrUtil; 	// upper threshold utilization - switch on AggSessAC, either absolute bitrate (e.g. "100kbps"), or relative to the link's datarate (e.g. "20%")
        string lowerThrUtil;   	// lower threshold utilization - switch off AggSessAC, either absolute bitrate (e.g. "100kbps"), or relative to the link's datarate (e.g. "20%")
        string impactOfSynTraff = default  
        bool colorAwareMode = default(false); // enables color-aware mode
        bool aggSessAC_method = default (false);
        bool thresholdAC_method = default (false);
        @signal[k_value](type=long);
        @signal[currTrafLoad](type=long); 
        @signal[currSYNGenTrafLoad](type=long); 
        @signal[color](type=int); 
        @signal[availBw](type=double);
        @signal[numOfActiveFlow](type=int);
        @signal[deletedFlowCounter](type=long);
        @signal[currentRate](type=long);
        @signal[hostSendInt](type=double);
        @statistic[k_value](title="CAC method type"; source=k_value; record=vector; interpolationmode=none);
        @statistic[currTrafLoad](title="currTrafLoad"; source=currTrafLoad; record=vector; interpolationmode=none);
        @statistic[currSYNGenTrafLoad](title="SYN host data rate"; source=currSYNGenTrafLoad; record=histogram, vector; interpolationmode=none);
        @statistic[color](title="Output color"; source=color; record=histogram, vector; interpolationmode=none);
        @statistic[availBw](title="Available Bandwidth"; source=availBw; record=vector; interpolationmode=none);
        @statistic[numOfActiveFlow](title="Number of active flow"; source=numOfActiveFlow; record=vector; interpolationmode=none);
        @statistic[deletedFlowCounter](title="Number of deleted flows"; source=deletedFlowCounter; record=vector; interpolationmode=none);
        @statistic[currentRate](title="Data rate"; source=currentRate; record=vector; interpolationmode=none);
        @statistic[hostSendInt](title="sendInterval"; source=hostSendInt; record=histogram, vector; interpolationmode=none);
        
    gates:
        input in[];
        output greenOut; 	// direct to DataQueue
        output redOut;		// direct to AggSessACQueue
}
