#ifndef __INET_SYNMARKER_H
#define __INET_SYNMARKER_H

#include "INETDefs.h"

/**
 * Session packet Marker. Mark packet with specific priority value and add option field - request bandwidth (ReqBW) value
 */
class INET_API SYNMarker : public cSimpleModule
{
  protected:
    //std::vector<int> dscps;
    simtime_t lastTime;
    int numRcvd;
    int numMarked;

    // statistics
    static simsignal_t markSYNPkSignal;
    static simsignal_t tosOfSYNPkSignal;
    static simsignal_t reqBwOfSYNPkSignal;
    static simsignal_t distributionTestIntSignal;
    static simsignal_t distributionTestDoubleSignal;
    static simsignal_t synSendTimeSignal; //inter-arrival time distribution of new flow

  public:
    SYNMarker() {}

  protected:
    virtual void initialize();

    virtual void handleMessage(cMessage *msg);

    virtual bool markPacket(cPacket *msg, int dscp, int reqbw);
};

#endif
