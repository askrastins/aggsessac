//
// AggSessAC compound module
// Consist from mFClassifier, utilMeter, fifo(defaultQueue), dtQueue (dataQueu), aggSessACQueue and priorScheduler1
// 

package aggsessac.NED;

import inet.linklayer.queue.AggSessACQueue;
import inet.linklayer.queue.DataThrQueue;
//import inet.linklayer.ITrafficConditioner;
import inet.linklayer.queue.DropTailQueue;
import inet.linklayer.queue.FIFOQueue;
import inet.linklayer.queue.PriorityScheduler1;
import inet.networklayer.diffserv.MultiFieldClassifier;
import inet.linklayer.IOutputQueue;
import inet.networklayer.diffserv.UtilizationMeter;


//
// Module consist of input output gate
// delayScheduler = priorutyScheduler determine how to process queues
// first queue attached to gate 0, etc. 
//
module TC102 like IOutputQueue
{
    parameters:
        bool thresholdAC = false; 			// switch on/off ThresholdAC method
        bool aggSessAC = true;				// switch on/off AggSessAC method 
        int QCapacity = 100;				// aggSessACqueue size >> bundle size no conflicts
        int bundleSize = 10;				// max bundle size
        double maxQueueTime = 0.05; 		// max waiting time (ms)
        string maxAvBw = "10000kbps";		// Max available traffic load
		
		// if use ThersholdAC and AggSessAC together, this regime can be switched on/off via utilizationMeter.cc
		string upThrUtil = "9500kbps"; 		// upper threshold utilization for enable AggSessAC, either absolute bitrate (kbps), or relative to the link's datarate (e.g. "20%")
        string lowThrUtil = "9000kbps";   	// lower threshold utilization for enable ThresholdAC, either absolute bitrate (kbps), or relative to the link's datarate (e.g. "20%") 
        
        string synTraff = "50kbps"; 		// test		
        
		@display("bgb=502,252");
    gates:
        input in;
        output out;

    submodules:
        mFClassifier: MultiFieldClassifier {
            // forward IPv4Datagrams/IPv6Datagrams to gate 0
            filters = xml("<filters><filter gate=\"0\"/></filters>");
            @display("p=58,118");
        }
        fifo: FIFOQueue {
            @display("p=277,186");
        }
        aggSessACQueue: AggSessACQueue {
            frameCapacity = QCapacity;  		// set up AggSessACQueue parameter  frameCapacity to TC102:QCapacity
            maxBundleSize = bundleSize;			// set up AggSessACQueue parameter  maxNumOfPackets to TC102:bundleSize
            maxDelay = maxQueueTime;			// set up AggSessACQueue parameter  maxDelay to TC102:maxQueueTime
            @display("p=277,39");
        }
        priorScheduler1: PriorityScheduler1 {
            @display("p=423,118");

        }
        utilMeter: UtilizationMeter {
            @display("p=156,78");
            thresholdAC_method = thresholdAC;
            aggSessAC_method = aggSessAC;
            impactOfSynTraff = synTraff;
            maxAvailBw = maxAvBw;

            upperThrUtil = upThrUtil;			// set up UtilizationMeter parameter upperThrUtil to TC102:upThrUtil
            lowerThrUtil = lowThrUtil;			// set up UtilizationMeter parameter lowerThrUtil to TC102:lowThrUtil
        }
        dtQueue: DataThrQueue {
            frameCapacity = QCapacity;
            @display("p=277,118");
        }
		
		//create compound module
		//connection between single modules: mFClassifier, utilMeter fifo(defaultQueue), dtQueue (dataQueu), aggSessACQueue and priorScheduler1
    connections:
        in --> mFClassifier.in; 						// packets income over classifier
        mFClassifier.outs++ --> utilMeter.in++;  		
        utilMeter.redOut --> aggSessACQueue.in;			
        utilMeter.greenOut --> dtQueue.in;				
        mFClassifier.defaultOut --> fifo.in++;			
        aggSessACQueue.out --> priorScheduler1.in++;	
        dtQueue.out --> priorScheduler1.in++;			     
        fifo.out --> priorScheduler1.in++;				
        priorScheduler1.out --> out;					// packets exit over priorScheduler1 

}