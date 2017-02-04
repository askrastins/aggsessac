//DataThrQueue - realize ThresholdAC admission control

#include "INETDefs.h"

#include "DataThrQueue.h"
#include "DiffservUtil.h"
#include "IPv4Datagram.h"
#include "UtilizationMeter.h"

#include "AggSessACQueue.h"

using namespace DiffservUtil;
Define_Module(DataThrQueue);
//logging info
simsignal_t DataThrQueue::queueLengthSignal = registerSignal("queueLength");
simsignal_t DataThrQueue::accepted_tosOfSYNPkSignal = registerSignal("accepted_tosOfSYNPk");     // ToS (priority) value for all incoming new session initialization packets (named as SYN)
simsignal_t DataThrQueue::accepted_reqBwOfSYNPkSignal = registerSignal("accepted_reqBwOfSYNPk"); // reqBw value for all incoming new session initialization packets (named as SYN)
simsignal_t DataThrQueue::rejected_tosOfSYNPkSignal = registerSignal("rejected_tosOfSYNPk");     // priorities of rejected session
simsignal_t DataThrQueue::rejected_reqBwOfSYNPkSignal = registerSignal("rejected_reqBwOfSYNPk"); // reqBw of rejected session
simsignal_t DataThrQueue::cumulative_acceptedSignal = registerSignal("cumulative_accepted");     // cumulative accepted (request) sessions
simsignal_t DataThrQueue::cumulative_rejectedSignal = registerSignal("cumulative_rejected");     // cumulative accepted (request) sessions
simsignal_t DataThrQueue::cum_revenueSignal = registerSignal("cumulative_revenue");              // cumulative obtained revenue
simsignal_t DataThrQueue::hostSendIntervalSignal = registerSignal("hostSendInt");				 // inter-arrival time of new session
simsignal_t DataThrQueue::synDataRateSignal = registerSignal("currSynLoad");			
simsignal_t DataThrQueue::queueingTimeOfSynSignal = registerSignal("queueingTimeOfSyn"); //queueing time of new session

bool DataThrQueue::pktRejected = true;

void DataThrQueue::initialize()
{
    //PassiveQueueBase_dTQueue::initialize();
    PassiveQueueBase2::initialize();

    queue.setName(par("queueName"));

    cum_revenue = 0;        //obtained revenue
    rejected = 0;           //counter for rejected
    accepted = 0;           //counter for accepted

    //statistics
    emit(queueLengthSignal, queue.length());

    outGate = gate("out");

    // configuration
    frameCapacity = par("frameCapacity");
}

//Return size of frame
int DataThrQueue::getframeSizeInBits(cMessage *msg)
{
    cPacket *packet = findIPDatagramInPacket(check_and_cast<cPacket*>(msg));
    if (!packet)
        error(" DataThrQueue::getframeSizeInBits() received a packet that does not encapsulate an IP datagram.");
    else
    {
        int packetSizeInBits = (packet->getBitLength())+144;  //Ethernet II header length: src(6)+dest(6)+etherType(2) = 14 bytes + 4 Bytes(FCS) = 18 Bytes =144 bits
        return packetSizeInBits;
    }
    return -1;
}

void DataThrQueue::sumOfSynBits(int synSize)
{
    sumOfSYNinBits = sumOfSYNinBits +  synSize;
}

//Return priority
int DataThrQueue::getToS(cMessage *msg)
{
    cPacket *packet = findIPDatagramInPacket(check_and_cast<cPacket*>(msg));
    if (!packet)
        error("DataThrQueue::getToS received a packet that does not encapsulate an IP datagram.");
    IPv4Datagram *datagram = dynamic_cast<IPv4Datagram*>(packet);

    tos = datagram->getDiffServCodePoint();
    if (tos >= 0)
    {
        return tos;
    }
    error("Warnings_3: DataThrQueue::getToS() return -1. cannot read ToS from IP datagram!");
    return -1;
}

//Return reqBw 
int DataThrQueue::getReqBw(cMessage *msg)
{
    cPacket *packet = findIPDatagramInPacket(check_and_cast<cPacket*>(msg));
    if (!packet)
        error("DataThrQueue::getToS received a packet that does not encapsulate an IP datagram.");
    IPv4Datagram *datagram = dynamic_cast<IPv4Datagram*>(packet);

    reqBw = datagram->getOptionCode();
    if (reqBw >= 0)
    {
        return reqBw;
    }
    error("Warnings_3: DataThrQueue::getReqBw() return -1. cannot read reqBw!");
    return -1;
}

//If session is accepted traffic generator host add new data flow with requested parameters 
//and update available bandwidth information UtilizationMeter::setAvailBw() 
void DataThrQueue::addFlow(cMessage *msg)
{
    //http://www.omnetpp.org/doc/omnetpp/manual/usman.html#sec155
       requsetBw = getReqBw(msg);
       serviceType = getToS(msg);
       cum_revenue =  cum_revenue + serviceType;

    //traffic source
       cModule *targetModule = getModuleByPath("network1.host[0].udpApp[0]");
       currentsendInt = targetModule->par("sendInterval").doubleValue();    // unit (s)
       currentMsgLen = targetModule->par("messageLength").longValue()+ 18 + 20 + 8;      // in bytes (B) MessageLength is only payload, + (14Ethernet header + 4CRC = 18B) + (IP Header 20B) + (The size of a UDP header is 8 bytes)
       currentRate = (long)(round(1/currentsendInt))*(8*currentMsgLen);
       nextRate =  currentRate + (requsetBw*1000);          // next rate after accepted SYN packet with reqBw
       pktsRate = nextRate/(8*currentMsgLen);               // number of pkts per second
       interval = (double)(1/(double) pktsRate);            // next send interval
       if (nextRate > 10000000)
       {
           int aa1 = 9; // only for debug
       }

    //statistic
       emit(hostSendIntervalSignal, interval);
       emit(accepted_tosOfSYNPkSignal, (long)serviceType);
       emit(accepted_reqBwOfSYNPkSignal, (long)requsetBw);
       emit(cum_revenueSignal, cum_revenue);

    //set new sendInterval
       parIdx1 = targetModule->findPar("sendInterval");     // Returns index of the sendInterval parameter
       parIdx2 = targetModule->findPar("messageLength");    // Returns index of the messageLength parameter
       targetModule->par(parIdx1).setDoubleValue(interval);

    //Update UtilizationMeter
       UtilizationMeter::addReqBwToVector(requsetBw);       // vector (array with accepted session)
       UtilizationMeter::setAvailBw(nextRate);              // available bandwidth
}


cMessage *DataThrQueue::enqueue(cMessage *msg)
{
    if (frameCapacity && queue.length() >= frameCapacity)
    {
        EV << "Queue full, dropping packet.\n";
        return msg;
    }
    else
    {
        queue.insert(msg);
        emit(queueLengthSignal, queue.length());
        return NULL;
    }
}

//dequeue() take packets out of queue, this method includes also ThresholdAC realization
cMessage *DataThrQueue::dequeue() 
{
    if (queue.empty())
        return NULL;

    cMessage *msg; //packet

    pktRejected = true;
    while(pktRejected)
    {
        if (queue.empty()) // if this called, mean that at least one packet was in queue and was removed
            return NULL;
        }

        msg = (cMessage *)queue.pop(); //take packet from queue

        serviceType = getToS(msg);
        if (serviceType == 0) 
        {
            // statistics
            emit(queueLengthSignal, queue.length());
            pktRejected = false;
        }
        else //Make ThresholdAC decision un save statistic info. Read all packets in queue
        {
            double avBw = UtilizationMeter::getAvailBw();
            requsetBw = getReqBw(msg);

            if (avBw >= (requsetBw*1000))
            {
                accepted++;
                // statistics
                emit(queueLengthSignal, queue.length()); 	// logging queue length
                emit(cumulative_acceptedSignal, accepted);  // logging total accepted
                pktRejected = false;
            }
            else
            {
                rejected++;
                EV << "AvailBw < reqBw, dropping packet.\n"; // for visualization in OMNET++ GUI (tkenv)

                queue.remove(msg); // drop if bandwidh not available 
                delete msg;

                //statistic
                emit(queueLengthSignal, queue.length());
                emit(rejected_tosOfSYNPkSignal, (long)serviceType);
                emit(rejected_reqBwOfSYNPkSignal, (long)requsetBw);
                emit(cumulative_rejectedSignal, rejected);
            }
            emit(queueingTimeOfSynSignal, simTime() - msg->getArrivalTime());
        }
    }

    return msg;
}

//each queue module contain sendOut function
void DataThrQueue::sendOut(cMessage *msg)
{
    frameSize = getframeSizeInBits(msg);        // accounting number of sendout bits
    UtilizationMeter::sumOfFrameBits(frameSize);
    serviceType = getToS(msg);
    if (serviceType == 0)      
        {
            send(msg, outGate);
        }
    else
        {
            addFlow(msg);
            send(msg, outGate);
        }
}

bool DataThrQueue::isEmpty()
{
    return queue.empty();
}

