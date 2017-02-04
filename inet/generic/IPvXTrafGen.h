#ifndef __INET_IPTRAFGEN_H
#define __INET_IPTRAFGEN_H

#include <vector>

#include "INETDefs.h"

#include "IPvXAddress.h"
#include "IPvXTrafSink.h"
#include "ILifecycle.h"
#include "NodeStatus.h"

#include <iostream>
#include <fstream>
#include <string>

/**
 * IP traffic generator application. See NED for more info.
 */
class INET_API IPvXTrafGen : public cSimpleModule, public ILifecycle
{
  protected:
    enum Kinds {START=100, NEXT};
    cMessage *timer;
    int protocol;
    int numPackets;
    int numReceived;
    bool isOperational;
    simtime_t startTime;
    simtime_t stopTime;
    std::vector<IPvXAddress> destAddresses;
    cPar *sendIntervalPar;
    const char* intervalFile;
    cPar *packetLengthPar;
    NodeStatus *nodeStatus;

    int numSent;
    int lineNum;
    static simsignal_t sentPkSignal;
    static simsignal_t rcvdPkSignal;

    int readInputFile(const char *filename);

  public:
    IPvXTrafGen();
    virtual ~IPvXTrafGen();
    virtual bool handleOperationStage(LifecycleOperation *operation, int stage, IDoneCallback *doneCallback);
    std::vector<std::string> sendInterArrivalVector; // Vectors contain send inter-arrival readed from file


  protected:
    virtual void scheduleNextPacket(simtime_t previous);
    virtual void cancelNextPacket();
    virtual bool isNodeUp();
    virtual bool isEnabled();

    // chooses destination address
    virtual IPvXAddress chooseDestAddr();
    virtual void sendPacket();

    virtual int numInitStages() const { return 4; }
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

    virtual void startApp();

    virtual void printPacket(cPacket *msg);
    virtual void processPacket(cPacket *msg);
};

#endif

