#ifndef __INET_AGGSESSACQUEUE_H_
#define __INET_AGGSESSACQUEUE_H_

#include <omnetpp.h>
#include "INETDefs.h"
#include "IPv4Datagram.h"
#include "PassiveQueueBase2.h"
#include <map>

/**
 * AggSessAC(Aggregated Session Admission Control) queue, completely modified DropTailQueue.
 * Realize Connection Admission Control (CAC) algorithm.
 */
 
class INET_API AggSessACQueue : public PassiveQueueBase2
{
  public:

    static int numOfAcceptedPkt;        // number of accepted sessions 
    static int maxBundleSize;         	// maximum size of bundle 
    static double maxDelay;             // maximum allowed time of aggregation period before CAC decision
    static int qlen;                    // queue length
    static double nextProcessTime;      // current time + maxDelay
    void checkAggSessConditions(); 		// check conditions. The friend function must have the class to which it is declared as friend passed to it in argument.


  protected:
    // configuration
    int frameCapacity;              // Queue size
    double waitingTime;             // actual waiting time between AggSessAC decisions
    double previousTime;			
    long numRejectedPkt;			// number of rejected sessions
    int rejected;                   // Tkenv skaitîtâjs
    int accepted;                   // Tkenv skaitîtâjs
    long cum_revenue;               // cumulative sum of accepted ToS value

    //timer
    simtime_t timer;
    cMessage *timerEvent;



    // compare value by key: always returns true - keys are placed in map one behind the another
    class compare_1 { // simple comparison function
       public:
          bool operator()(cMessage* k1, cMessage* k2) const { return true; }
    };

    // Comparison function compare key
    class compare_2 {
           public:
              bool operator()(int v1, int v2) const { return v1 >= v2; } // sorts its element by key.
        };

    // PacketInfo fields:
    int tos;            //priorities (Tos)
    int reqBw;          //reqBw
    bool isSYN;
    int requsetBw;      // reqBw of enqueued packet
    int serviceType;    // priorities of enqueued packet
    int frameSize;      // frame size in bits

    //variable for addFlow() method
    int parIdx1;            // index of the 'sendInterval' parameter
    int parIdx2;            // index of the 'messageLength' parameter
    double interval;        // generated send interval
    double currentsendInt;
    long currentMsgLen;
    long currentRate;        // current data rate
    long nextRate;           // next rate after accepted SYN packet with reqBw
    int pktsRate;           //  number of pkts per second

    //Available bandwidth
    double avBand;

    //typedef
    typedef std::map<cMessage*,int,compare_1 > dictionary;          // unsorted map contain cMessage<->int pair info
    typedef std::map<int,cMessage*, compare_2> dictionary2;         // sorted map by key, contain int<->cMessage pair info
    typedef dictionary::iterator dictit;                            // iterator for dictionary
    typedef dictionary2::iterator dictit2;                          // iterator for dictionary2
    std::list<cMessage*> lt;

    // state
    bool allowSend;
    bool allowImmediately;          
    cQueue queue;                   
    cGate *outGate;

    std::vector<int> tosArr;        // container with ToS value
    dictionary reqMsgMap;           // table contain msg<->ToS pair info
    dictionary2 sortedMap;          // table contain msg<->ToS pair sorted by ToS field
    dictionary2 acceptedMap;        // table contain accepted sessions
    dictionary2 rejectedMap;        // table contain rejected sessions
    dictit dit;                     // iterator
    dictit2 dit2;                   // iterator


    // statistics
    static simsignal_t queueLengthSignal;						// actual queue size
    static simsignal_t numOfRejectedFlowsByDecisionSignal;     	// number of rejected sessions by AggSessAC method
    static simsignal_t numOfAcceptedFlowsByDecisionSignal;     	// number of accepted sessions by AggSessAC method
    static simsignal_t totalqueueLengthSignal;      			// queue size before AggSessAC decision
    static simsignal_t waitingTimeSignal;           			// actual waiting time
    static simsignal_t accepted_tosOfSYNPkSignal;            	// priorities of accepted sessions
    static simsignal_t accepted_reqBwOfSYNPkSignal;          	// reqBw of accepted sessions
    static simsignal_t rejected_tosOfSYNPkSignal;           	// priorities of rejected sessions
    static simsignal_t rejected_reqBwOfSYNPkSignal;         	// reqBw of rejected sessions
    static simsignal_t cumulative_acceptedSignal;          		// Cumulative accepted sessions
    static simsignal_t cumulative_rejectedSignal;          		// Cumulative rejected sessions
    static simsignal_t nextProcessTimeSignal;
    static simsignal_t timerSignal;
    static simsignal_t cum_revenueSignal;						// obtained revenue

    int getframeSizeInBits(cMessage *msg);
    int getToS(cMessage *msg);
    int getReqBw(cMessage *msg);
    void deleteRejectedPkt();
    void walkOnQueue();

    void addFlow(cMessage *msg);
    cMessage *sendDecision(cMessage *msg);
    cMessage *takeAcceptedPkt();


  protected:
    virtual void initialize();

    /**
     * Redefined from PassiveQueueBase2.
     */
    virtual cMessage *enqueue(cMessage *msg);

    /**
     * Redefined from PassiveQueueBase2.
     */
    virtual cMessage *dequeue();

    /**
     * Redefined from PassiveQueueBase2.
     */
    virtual void sendOut(cMessage *msg);

    /**
     * Redefined from IPassiveQueue.
     */
    virtual bool isEmpty();

    //virtual void handleMessage(cMessage *msg);
};

#endif
