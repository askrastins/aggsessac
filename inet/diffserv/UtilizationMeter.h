#ifndef __INET_UTILIZATIONMETER_H
#define __INET_UTILIZATIONMETER_H

#include "INETDefs.h"
#include "IPv4Datagram.h"

/**
 * UtilizationMeter.
 */
class INET_API UtilizationMeter : public cSimpleModule
{
  public:
    static long sumOfSendBits;              // Sum of bits in time
    static double AvailBw;                  // Available traffic load
    static double MaxAvailBw;               // Max available traffic load
    static void sumOfFrameBits(int framesize);
    static void addReqBwToVector(int reqbw);
    static double getAvailBw();
    static void setAvailBw(double rate);
    static std::vector<int> AcceptedFlowVector;    // reqBw info from accepted flow

    //public statistic
    static simsignal_t AvailBwSignal;							//Available bandwidth
    static simsignal_t numOfActiveFlowSignal;					//number of active flow
    static simsignal_t hostSendIntervalSignal;                  //ne session generator (sendInterval)



  protected:
    //double MaxAvailBw;
    double UpperThrUtil;    // upper utilization threshold to enable the AggSessAC (bits/sec)
    double LowerThrUtil;    // lower utilization threshold to enable the AggSessAC (bits/sec)
    double ImpactOfSynTraff; // SYN generator traffic load and impact on data traffic. It is important for thresholdAC method (dtQueue)
    bool colorAwareMode;
    bool aggSessAC_method;  // set AggSessAC method
    bool thresholdAC_method; // set thresholdAC method
    int k;                  // under the upper threshold k=0 and up to the upper threshold k=1
    int color;              // output gate color
    long deleteID;          // (counter)
    long numDeleted;        // number of deleted flows (counter)
    int rBw;
    int parIdx1;            // index of the 'sendInterval' parameter
    int parIdx2;            // index of the 'messageLength' parameter
    double interval;        // generated send interval
    long sumOfSYNinBits;    // sum of syn packets (bits)

    //variable for rateCalc() method
    double currTrafLoad;            // input data rate;
    double currSYNGenTrafLoad;      // traffic rate of Syn generator;
    double currentsendInt;
    long currentMsgLen;
    long currentRate;               // current data rate
    long preferredRate;             // next rate after accepted SYN packet with reqBw
    int pkts_per_sec;               // number of pkts per second

    simtime_t lastUpdateTime;		//for rate calculation
    simtime_t previousRateTime;     //for rate calculation
    double timeInterval;
    int tos;
    int serviceType;

    int numRcvd;
    int numRed;

    // statistics
    static simsignal_t k_valueSignal;               // Which method is used: threshold k=0 or AggSessAC k=1
    static simsignal_t colorSignal;
    static simsignal_t dataRateSignal;              // input data rate calculated by rateCalc()
    static simsignal_t currSYNGenDataRateSignal;    // traffic rate of Syn generator;
    static simsignal_t currentRateSignal;           // current input data rate
    static simsignal_t deletedFlowCounterSignal;

    //static simsignal_t AvailTrafLoadSignal;

  public:
    UtilizationMeter() {}

  protected:
    virtual int numInitStages() const { return 3; }

    virtual void initialize(int stage);

    virtual void handleMessage(cMessage *msg);

    void sumOfSYNFrameBits(int framesize);

    virtual int methodSelection(cMessage *msg);

    int checkToS(cMessage *msg);

    int getframeSizeInBits(cMessage *msg);

    void rateCalc();

    void deleteFlow(int flowid);

};

#endif
