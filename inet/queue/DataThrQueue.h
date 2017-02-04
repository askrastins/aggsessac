#ifndef __INET_DataThrQueue_H
#define __INET_DataThrQueue_H

#include "INETDefs.h"

#include "PassiveQueueBase2.h"
#include "IPv4Datagram.h"

/**
 * Drop-front queue.
 * DataThrQueue with thresholdAC control
 */
class INET_API DataThrQueue : public PassiveQueueBase2
{
  public:
    static bool pktRejected;

  protected:
    // configuration 
    int frameCapacity;	//.ned file parametr Queue capacity (in frames)
    int tos;			//priority
    int reqBw;			//requested bandwidth
    int serviceType;	//priority
    int requsetBw;      //requested bandwidth
    int frameSize;      //send frame size in bits
    int accepted;
    int rejected;

    //variable for addFlow() method
    int parIdx1;            // index of the 'sendInterval' parameter
    int parIdx2;            // index of the 'messageLength' parameter
    double interval;        // currently it is random generated send interval
    double currentsendInt;	// msg send rate
    long currentMsgLen;		// msg length
    long currentRate;       // current data rate
    long nextRate;          // next rate after accepted SYN packet with reqBw
    int pktsRate;           // number of pkts per second
    long cum_revenue;       // obtained revenue

    long sumOfSYNinBits;

    // state
    cQueue queue;
    cGate *outGate;

    // statistics
    static simsignal_t queueLengthSignal;				// queue length
    static simsignal_t accepted_tosOfSYNPkSignal;		// priorities of accepted sessions
    static simsignal_t accepted_reqBwOfSYNPkSignal;     // accepted sessions size
    static simsignal_t rejected_tosOfSYNPkSignal;       // priorities of rejected sessions
    static simsignal_t rejected_reqBwOfSYNPkSignal;     // rejected sessions size
    static simsignal_t cumulative_acceptedSignal;       // cumulative accepted sessions
    static simsignal_t cumulative_rejectedSignal;		// cumulative rejected sessions
    static simsignal_t cum_revenueSignal;				// obtained revenue
    static simsignal_t hostSendIntervalSignal;          // SYN SendInterval, the same value is also traced under UtilMeter as well.
    static simsignal_t synDataRateSignal;				// actual data rate traffic generator
    static simsignal_t queueingTimeOfSynSignal;         // queueing time of packets of accepted sessions 

    int getframeSizeInBits(cMessage *msg);
    int getToS(cMessage *msg);
    int getReqBw(cMessage *msg);
    void sumOfSynBits(int synSize);
    void addFlow(cMessage *msg);


  protected:
    virtual void initialize();

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual cMessage *enqueue(cMessage *msg);

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual cMessage *dequeue();

    /**
     * Redefined from PassiveQueueBase.
     */
    virtual void sendOut(cMessage *msg);

    /**
     * Redefined from IPassiveQueue.
     */
    virtual bool isEmpty();
};

#endif
