#include "UtilizationMeter.h"
#include "DiffservUtil.h"
#include "IPv4Datagram.h"
#include "EtherMACBase.h"
#include "EtherMAC.h"
#include "AggSessACQueue.h" //cordinated and common AggSessACQueue::sendIntervalSignal logging

using namespace DiffservUtil;

Define_Module(UtilizationMeter);

//
// UtilizationMeter calculate/monitor/accounting actual data rate, 
// if utilization threshold not exceed send packet to GREEN gate (dataThrQueue)
// else to RED gate (AggSessACQueue).
// 

//logging info
simsignal_t UtilizationMeter::k_valueSignal = registerSignal("k_value");
simsignal_t UtilizationMeter::dataRateSignal = registerSignal("currTrafLoad");
simsignal_t UtilizationMeter::currSYNGenDataRateSignal = registerSignal("currSYNGenTrafLoad");
simsignal_t UtilizationMeter::currentRateSignal = registerSignal("currentRate"); 
simsignal_t UtilizationMeter::colorSignal = registerSignal("color");
simsignal_t UtilizationMeter::AvailBwSignal = registerSignal("availBw");
simsignal_t UtilizationMeter::numOfActiveFlowSignal = registerSignal("numOfActiveFlow");
simsignal_t UtilizationMeter::hostSendIntervalSignal = registerSignal("hostSendInt");       // calculated every 50 ms SNY Generator host sendInterval
simsignal_t UtilizationMeter::deletedFlowCounterSignal = registerSignal("deletedFlowCounter");
long UtilizationMeter::sumOfSendBits = 0;   // static field
double UtilizationMeter::AvailBw;           // static field
double UtilizationMeter::MaxAvailBw;        // static field
std::vector<int> UtilizationMeter::AcceptedFlowVector;

void UtilizationMeter::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    if (stage == 0)
    {
        numRcvd = 0;
        numRed = 0;
        k=100;            // Situation when both method thresholdAC and AggSessAC are used parallel: threshold k=0 or AggSessAC k=1
        WATCH(numRcvd);
        WATCH(numRed);
        currTrafLoad = 0;
        currSYNGenTrafLoad = 0;
        sumOfSYNinBits = 0;
        numDeleted = 0;


        emit(k_valueSignal, k);
        emit(dataRateSignal, currTrafLoad);

        colorAwareMode = par("colorAwareMode");
        aggSessAC_method = par("aggSessAC_method");
        thresholdAC_method = par("thresholdAC_method");

        color = GREEN;   // Green = 0, yellow = 1, red = 2
    }
    else if (stage == 2)
    {
        //upper threshold on aggSessAC method
        const char *upperThrUtilStr = par("upperThrUtil");
        UpperThrUtil = parseInformationRate(upperThrUtilStr, "upperThrUtil", *this, 0);
        //lower threshold on aggSessAC method
        const char *lowerThrUtilStr = par("lowerThrUtil");
        LowerThrUtil = parseInformationRate(lowerThrUtilStr, "lowerThrUtil", *this, 0);

        //syn generated traffic impact on dtQueue at ThresholdAC method;
        const char *impactOfSynTraffStr = par("impactOfSynTraff");
        ImpactOfSynTraff = parseInformationRate(impactOfSynTraffStr, "impactOfSynTraff", *this, 0);
        //Maximum available bandwidth
        const char *maxAvailBwStr = par("maxAvailBw");
        UtilizationMeter::MaxAvailBw = parseInformationRate(maxAvailBwStr, "maxAvailBw", *this, 0) - ImpactOfSynTraff;

        UtilizationMeter::AvailBw = MaxAvailBw;
        lastUpdateTime = simTime();
        previousRateTime = simTime();
        currentRate = 0;
    }
}

void UtilizationMeter::sumOfFrameBits(int framesize)
{
    sumOfSendBits = sumOfSendBits +  framesize;
}

void UtilizationMeter::sumOfSYNFrameBits(int framesize)
{
    sumOfSYNinBits = sumOfSYNinBits +  framesize;
}

//get reqBw from accepted flow vector
void UtilizationMeter::addReqBwToVector(int reqbw)
{
    AcceptedFlowVector.push_back(reqbw);
}

//calculate available bandwidth
void UtilizationMeter::setAvailBw(double rate)
{
    if (rate>=10000000)
    {
        int uuu=0;
    }
    AvailBw = MaxAvailBw - rate;
}

//return available bandwidth
double UtilizationMeter::getAvailBw()
{
    return AvailBw;
}

//delete flow by changing sendInterval and remove reqBw from accepted flow vector
void UtilizationMeter::deleteFlow(int flowid)
{
    rBw = AcceptedFlowVector.at(flowid);
    AcceptedFlowVector.erase(AcceptedFlowVector.begin()+ flowid);

    cModule *targetModule = getModuleByPath("network1.host[0].udpApp[0]");
    currentsendInt = targetModule->par("sendInterval").doubleValue();   // unit (s)
    currentMsgLen = targetModule->par("messageLength").longValue()+ 18 + 20 + 8;      //+ (14Ethernet header + 4CRC = 18B) + (IP Header 20B) + (UDP header is 8 bytes);
    currentRate = (long)(round(1/currentsendInt))*(8*currentMsgLen);
    //currentRate = MaxAvailBw - getAvailBw();
    preferredRate =  currentRate - (rBw*1000);              // next rate after accepted SYN packet with reqBw
    pkts_per_sec = preferredRate/(8*currentMsgLen);         // number of pkts per second
    interval = (double)(1/(double) pkts_per_sec);           // next send interval

    parIdx1 = targetModule->findPar("sendInterval");        // Returns index of the sendInterval parameter
    parIdx2 = targetModule->findPar("messageLength");       // Returns index of the messageLength parameter
    targetModule->par(parIdx1).setDoubleValue(interval);    // Change sendInterval

    //statistic
    numDeleted++;
    emit(deletedFlowCounterSignal, numDeleted);

    setAvailBw(preferredRate);                              // Update AvailBw changes
}

//Return specific info from packet
int UtilizationMeter::checkToS(cMessage *msg)
{
    cPacket *packet = findIPDatagramInPacket(check_and_cast<cPacket*>(msg));
    if (!packet)
        error("UtilizationMeter::checkToS received a packet that does not encapsulate an IP datagram.");
    IPv4Datagram *datagram = dynamic_cast<IPv4Datagram*>(packet);
    // tos = datagram->getTypeOfService();
    tos = datagram->getDiffServCodePoint();
    if (tos >= 0)
    {
        return tos;
    }
    error("Warnings_3: UtilizationMeter::checkToS() return -1. Cannot get ToS!");
    return -1;
}

//Return size of frame
int UtilizationMeter::getframeSizeInBits(cMessage *msg)
{
    cPacket *packet = findIPDatagramInPacket(check_and_cast<cPacket*>(msg));
    if (!packet)
        error(" UtilizationMeter::getframeSizeInBits() received a packet that does not encapsulate an IP datagram.");
    else
    {
        int packetSizeInBits = (packet->getBitLength())+144;  //Ethernet II header length: src(6)+dest(6)+etherType(2) = 14 bytes + 4 Bytes(FCS) = 18 Bytes =144 bits
        return packetSizeInBits;
    }
    return -1;
}

//DatarRate Calculation (return currentTrafLoad and currentRate both value is almost similar)
void UtilizationMeter::rateCalc() //
{
 // ##### Actual link BW - amount of bits ####################
        simtime_t currentTime = simTime();
        timeInterval = SIMTIME_DBL(currentTime - previousRateTime);
        currTrafLoad = (long)(sumOfSendBits/timeInterval);
        previousRateTime = currentTime; // refresh previousRateTime
        sumOfSendBits = 0;              // reset counter
        //statistic
        emit(dataRateSignal, currTrafLoad);

 // ##### Actual Syn generator BW - amount of bits in time ####################
        currSYNGenTrafLoad = sumOfSYNinBits/timeInterval;
        sumOfSYNinBits = 0;              // reset counter
        //statistic
        emit(currSYNGenDataRateSignal, currSYNGenTrafLoad);

 // ##### Actual link BW - according to sendInterval and messageLength of sender ################
        cModule *targetModule = getModuleByPath("network1.host[0].udpApp[0]");
        currentsendInt = targetModule->par("sendInterval").doubleValue();   // unit (s)
        currentMsgLen = targetModule->par("messageLength").longValue()+ 18 + 20 + 8;      //+ (14Ethernet header + 4CRC = 18B) + (IP Header 20B) + (datagram header is 8 bytes);
        currentRate = (long)(round(1/currentsendInt))*(8*currentMsgLen);
        //statistic
        emit(AvailBwSignal, AvailBw);
        emit(currentRateSignal, currentRate);
        emit(hostSendIntervalSignal, currentsendInt);
}

// if we test with switching between CAC method between ThresholdAC and AggSessAC (GREEN gate = threshold, RED gate = AggSessAC)
int UtilizationMeter::methodSelection(cMessage *msg)
{
    cPacket *packet = findIPDatagramInPacket(check_and_cast<cPacket*>(msg));
        if (!packet)
          error("UtilizationMeter::handleMessage() received a packet that does not encapsulate an IP datagram.");

        // update meter state between threshold/aggSessAC
        int oldColor = colorAwareMode ? getColor(packet) : -1; // return -1 if colorAwareMode not used
        int newColor;

        if (oldColor <= GREEN && currTrafLoad <= UpperThrUtil ) // upper threshold,   GREEN, YELLOW, RED - colors for naming the output of UtilizationMeter
        {
            if (currTrafLoad < LowerThrUtil)        // below lower threshold
            {newColor = GREEN; k = 100;}        // <= Threshold
                else                                // between Upper and Lower
                    if (k==1000)                    // if we come down from upper level (AggSessAC) but not reached lowerThreshold - send to RED
                        newColor = RED;         // <= AggSessAC
                    else                            // from lower to upper
                        newColor = GREEN;       // <= Threshold
        }
        else                                // exceed upper threshold set AggSessAC method
            {newColor = RED; k = 1000;}         // <= AggSessAC

        setColor(packet, newColor);
        return newColor;
}


void UtilizationMeter::handleMessage(cMessage *msg)
{
    numRcvd++;
    simtime_t currentTime = simTime();

//check ToS value
    serviceType = checkToS(msg);

//Rate monitor
    if (currentTime - previousRateTime >= 0.025) // every 25ms
    {
        rateCalc();     //calculate current traffic load and also noted by statistics
    }

//Delete flow
    if (currentTime > 0.05 && AcceptedFlowVector.size()> 8 && intuniform(0, 50) == 23) // session life control
    {
        int fid = intuniform(0, 3);
        deleteFlow(fid);
        deleteID++;        
    }

//Dynamic switching between AggSessAC and thresholdAC methods
    if (serviceType == 0)   // data packet 
    {
     //statistic
        emit(k_valueSignal, k);
        emit(colorSignal, color); // Green = 0, yellow = 1, red = 2, yellow netiek izmantots

        int flow_count = AcceptedFlowVector.size(); //number of active flows
        emit(numOfActiveFlowSignal, flow_count);

        send(msg, "greenOut");
    }
//CAC method selection
    else
    {
//Sum of SYN packets (in bits)
         int frameSize = getframeSizeInBits(msg);
         sumOfSYNFrameBits(frameSize);
/*
        if (currentTime - previousRateTime >= 0.05) // every 50 ms
            {
                color = methodSelection(msg);
            }
        //statistic
        emit(k_valueSignal, k);
        emit(colorSignal, color); // Green = 0, yellow = 1, red = 2, yellow not used

//Send msg out
        if (color == GREEN) // check utilization threshold if not exceed send to Dataqueue
            {
                send(msg, "greenOut"); //to dtQueue (Threshold)
            }
        else
            {
                numRed++;
                send(msg, "redOut"); // to AggSessACQueue (AggSessAC)
            }

*/

//Static method selection between thresholdAC and AggSessAC:
         if (thresholdAC_method == true && aggSessAC_method == true)
                 error("UtilizationMeter::handleMessage: both thresholdAC_method and aggSessAC_method can't be == true.");
         if (thresholdAC_method) //ja thresholdAC_method == true -> all session send via dtQueue (ThresholdAC only)
         {
             send(msg, "greenOut");
         }

         if (aggSessAC_method) //ja aggSessAC_method == true -> new session send to aggSessACqueue
         {
             numRed++;
             send(msg, "redOut");
         }
    }

// update Tkenv GUI visual info
    if (ev.isGUI())
    {
        char buf[50] = "";
        if (numRcvd>0) sprintf(buf+strlen(buf), "rcvd: %d ", numRcvd);
        if (numRed>0) sprintf(buf+strlen(buf), "red:%d ", numRed);
        getDisplayString().setTagArg("t", 0, buf);
    }
}





