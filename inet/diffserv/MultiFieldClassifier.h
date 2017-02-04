#ifndef __INET_MULTIFIELDCLASSIFIER_H
#define __INET_MULTIFIELDCLASSIFIER_H

#include "INETDefs.h"

/**
 * Absolute dropper.
 */
class INET_API MultiFieldClassifier : public cSimpleModule
{
  protected:
        struct Filter
        {
            int gateIndex;

            IPvXAddress srcAddr;
            int srcPrefixLength;
            IPvXAddress destAddr;
            int destPrefixLength;
            int protocol;
            int tos;
            int tosMask;
            int srcPortMin;
            int srcPortMax;
            int destPortMin;
            int destPortMax;

            Filter() : gateIndex(-1),
                       srcPrefixLength(0), destPrefixLength(0), protocol(-1), tos(0), tosMask(0),
                       srcPortMin(-1), srcPortMax(-1), destPortMin(-1), destPortMax(-1)  {}
    #ifdef WITH_IPv4
            bool matches(IPv4Datagram *datagram);
    #endif
    #ifdef WITH_IPv6
            bool matches(IPv6Datagram *datagram);
    #endif
        };

  protected:
    int numOutGates;
    std::vector<Filter> filters;

    int numRcvd;

    static simsignal_t pkClassSignal;

  protected:
    void addFilter(const Filter &filter);
    void configureFilters(cXMLElement *config);

  public:
    MultiFieldClassifier() {}

  protected:
    virtual int numInitStages() const { return 4; }

    virtual void initialize(int stage);

    virtual void handleMessage(cMessage *msg);

    virtual int classifyPacket(cPacket *packet);
};

#endif
