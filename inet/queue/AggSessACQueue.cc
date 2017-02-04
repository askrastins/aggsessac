#include <omnetpp.h>
#include "INETDefs.h"
#include "AggSessACQueue.h"
#include "DiffservUtil.h"
#include "IPv4Datagram.h"
#include "UDPPacket.h"
#include "TCPSegment.h"
#include <map>
#include "UtilizationMeter.h"

using namespace DiffservUtil;
Define_Module(AggSessACQueue);

/**
 * AggSessAC(Aggregated Session Admission Control) queue, completely modified DropTailQueue.
 * Realize Connection Admission Control (CAC) algorithm.
 */

simsignal_t AggSessACQueue::queueLengthSignal = registerSignal("queueLength");              						// actual queue size
simsignal_t AggSessACQueue::numOfRejectedFlowsByDecisionSignal = registerSignal("numOfRejectedFlowsByDecision");   	// number of rejected sessions by AggSessAC Method
simsignal_t AggSessACQueue::numOfAcceptedFlowsByDecisionSignal = registerSignal("numOfAcceptedFlowsByDecision");   	// number of rejected sessions by AggSessAC Method
simsignal_t AggSessACQueue::totalqueueLengthSignal = registerSignal("totalqueueLength");    						// bundle size in each decision making moment)
simsignal_t AggSessACQueue::waitingTimeSignal = registerSignal("waitingTime");              						// actual waiting time
simsignal_t AggSessACQueue::nextProcessTimeSignal = registerSignal("nextProcessTime");      						// next process time
simsignal_t AggSessACQueue::timerSignal = registerSignal("timer");                          						// timer event - timer that check aggSessConditions
simsignal_t AggSessACQueue::accepted_tosOfSYNPkSignal = registerSignal("accepted_tosOfSYNPk");      // priorities of accepted sessions
simsignal_t AggSessACQueue::accepted_reqBwOfSYNPkSignal = registerSignal("accepted_reqBwOfSYNPk");  // reqBw of accepted sessions
simsignal_t AggSessACQueue::rejected_tosOfSYNPkSignal = registerSignal("rejected_tosOfSYNPk");      // priorities of rejected sessions
simsignal_t AggSessACQueue::rejected_reqBwOfSYNPkSignal = registerSignal("rejected_reqBwOfSYNPk");  // reqBw of rejected sessions
simsignal_t AggSessACQueue::cumulative_acceptedSignal = registerSignal("cumulative_accepted");      // cumulative accepted (request) sessions
simsignal_t AggSessACQueue::cumulative_rejectedSignal = registerSignal("cumulative_rejected");      // cumulative accepted (request) sessions
simsignal_t AggSessACQueue::cum_revenueSignal = registerSignal("cumulative_revenue");               // cumulative obtained revenue

// Public value for priorityScheduler1
int AggSessACQueue::numOfAcceptedPkt = 0;   

int AggSessACQueue::maxBundleSize = 0;    // (max bundle size)
double AggSessACQueue::maxDelay = 0;
int AggSessACQueue::qlen = 0;               
double AggSessACQueue::nextProcessTime;     

//initialization process
void AggSessACQueue::initialize()
{
    PassiveQueueBase2::initialize();

    queue.setName(par("queueName"));
    nextProcessTime = SIMTIME_DBL(simTime());   
    previousTime = SIMTIME_DBL(simTime());
    allowSend = true;                           
    allowImmediately = false;
    numRejectedPkt = 0;
    rejected = 0;
    accepted = 0;
    cum_revenue = 0;                   

    //Initialize timer
    timer = 0.001;
    timerEvent = new cMessage("timerEvent");
    //Generate and send initial message - timer starting
    scheduleAt(simTime()+0.001, timerEvent); // pçc 1ms

    //statistics
    emit(queueLengthSignal, queue.length());					// actual queue size
    emit(numOfRejectedFlowsByDecisionSignal, numRejectedPkt); 	// number of rejected sessions
    emit(numOfAcceptedFlowsByDecisionSignal, numOfAcceptedPkt); // number of accepted sessions
    emit(totalqueueLengthSignal, 0);							// queue size at decision-making moments
    emit(waitingTimeSignal, waitingTime);       				// log time between AggSessAC decisions


    outGate = gate("out");

    // configuration
    frameCapacity = par("frameCapacity");	// queue capacity read from .ned file
    maxBundleSize = par("maxBundleSize"); 	// bundle size read from .ned file
    maxDelay = par("maxDelay");				// waiting time read from .ned file

    WATCH(isSYN);
}

//Return size of frame
int AggSessACQueue::getframeSizeInBits(cMessage *msg)
{
    cPacket *packet = findIPDatagramInPacket(check_and_cast<cPacket*>(msg));
    if (!packet)
        error("AggSessACQueue::getframeSizeInBits() received a packet that does not encapsulate an IP datagram.");
    else
    {
        int packetSizeInBits = (packet->getBitLength())+144;  //Ethernet II header length: src(6)+dest(6)+etherType(2) = 14 bytes + 4 Bytes(FCS) = 18 Bytes =144 bits
        return packetSizeInBits;
    }
    return -1;
}

//Return priority
int AggSessACQueue::getToS(cMessage *msg)
{
    cPacket *packet = findIPDatagramInPacket(check_and_cast<cPacket*>(msg));
    if (!packet)
        //EV << " getToS: for pkt: "<< packet << " ###################\n";
        error("DataThrQueue::getToS received a packet that does not encapsulate an IP datagram.");
    IPv4Datagram *datagram = dynamic_cast<IPv4Datagram*>(packet);

    // tos = datagram->getTypeOfService();
    tos = datagram->getDiffServCodePoint();
    tosArr.push_back(tos); //container with priorities values
    if (tos >= 0)
    {
        UDPPacket *udpPacket = dynamic_cast<UDPPacket*>(packet);
            if (udpPacket)
            {}
        TCPSegment *tcpSegment = dynamic_cast<TCPSegment*>(packet);
            if (tcpSegment)
            {isSYN = tcpSegment->getSynBit();}
        return tos;
    }
    return -1;
}

//Return reqBw 
int AggSessACQueue::getReqBw(cMessage *msg)
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
    return -1;
}

//Delete packets of rejected sessions from queue
void AggSessACQueue::deleteRejectedPkt()
{
    emit(totalqueueLengthSignal, queue.length()); //queue size before AggSessAC decision

    numRejectedPkt =0;
    cMessage *msg; 
    for(dit2 = rejectedMap.begin(); dit2 != rejectedMap.end(); ++dit2)
    {
        msg = (cMessage*)(dit2->second); //address to pointer *msg

        EV << " Remove msg: "<< msg <<"\n";
        if (msg != NULL)
        {
          queue.remove(msg);    //remove()function can be used to remove any item known by its pointer from the queue but don't delete object
          delete msg;           //delete object
          numRejectedPkt++;
        }
    }
    emit(numOfRejectedFlowsByDecisionSignal, numRejectedPkt);

 //Check accepted sessions
    for (cQueue::Iterator queueIter(queue); !queueIter.end(); queueIter++)
        {
            cMessage *msg = (cMessage*)queueIter();
            EV << " Accepted packets in Queue:  " << msg << " \n";
        }
    numOfAcceptedPkt = queue.length();
    emit(numOfAcceptedFlowsByDecisionSignal, numOfAcceptedPkt); //update statistic
}

//Traversing AggSessACQueue session by session and write map<key value> = [session,priority] 
//and make decision about acception/rejection
void AggSessACQueue::walkOnQueue()
{
	// clear all vectors
    reqMsgMap.clear();
    sortedMap.clear();
    acceptedMap.clear();
    rejectedMap.clear();
    tosArr.clear();    
    int i = queue.length();
    numOfAcceptedPkt = 0; 

    EV << " ################################################# "<<" \n"; // console output tkenv
 
    cQueue::Iterator it(queue,true);
    while(!it.end())
    {
        cMessage *data = (cMessage*)it();
        i--;
        it.operator --(i);
        //EV << "  Cuurent time: " << simTime() << "\n"; // for debug

        serviceType = getToS(data);
        reqMsgMap.insert(std::pair<cMessage*,int>(data,serviceType)); // in this moment reqMsgMap is unsorted because use comparison function compare_1().
        EV << " Table reqMsgMap.insert " << data << ". \n"; // console output tkenv
    }

 
 // By definition a map is a data structure that sorts its element by key descending order by priorities.
 // build a 'sortedMap' map, with the 'reqMsgMap' map's values as keys and the 'reqMsgMap' map's keys as values.
    for(dit = reqMsgMap.begin(); dit != reqMsgMap.end(); ++dit)
    {
        if ((dit -> second) > 0) 
        {
        sortedMap.insert(dictionary2::value_type(dit->second, dit -> first));   //sortedMap - descending order by priorities
        }
    }

    EV << " ################################################# "<<" \n"; // console output tkenv

	//Check sortedMap and create accepted map with accepted sessions
    cMessage *msg;                                              
    avBand = UtilizationMeter::getAvailBw();
    for(dit2 = sortedMap.begin(); dit2 != sortedMap.end(); ++dit2)
    {
        msg = (cMessage*)(dit2->second);                        
        requsetBw = getReqBw(msg);
        serviceType = getToS(msg);
        if (avBand > (requsetBw*1000))
        {
            acceptedMap.insert(std::pair<int, cMessage*>(serviceType, msg));
            avBand = avBand - requsetBw*1000;
            accepted++;
        }
        else
        {

            rejectedMap.insert(std::pair<int, cMessage*>(serviceType, msg));
            rejected++;
            emit(rejected_tosOfSYNPkSignal, (long)serviceType);
            emit(rejected_reqBwOfSYNPkSignal, (long)requsetBw);
        }
        EV << " Table sortedMap key: " << dit2 -> first << "   value: " << dit2->second << ". \n";
    }
    emit(cumulative_acceptedSignal, accepted);
    emit(cumulative_rejectedSignal, rejected);

    // for debug
	// Check sortedMap values from the end.
    /* dit2 = sortedMap.end();
      while(dit2 != sortedMap.begin())
      {
      --dit2;
      EV << " Table sortedMap key: " << dit2 -> first << "   value: " << dit2->second << " sortedMap.size: " << sortedMap.size() << ". \n";
      }*/
}

//return accepted packets of sessions from queue
cMessage *AggSessACQueue::takeAcceptedPkt()
{
    if (queue.empty())
        return NULL;

    cMessage *msg = (cMessage *)queue.pop();

    // statistics
    emit(queueLengthSignal, queue.length());
    qlen = queue.length(); // vçrtîba iekð scheduler tiek izmantota
    return msg;
}

//check waiting time and bundle size
void AggSessACQueue::checkAggSessConditions()
{
//0)set up waiting time
        if (allowSend == true)
        {
        //logging waiting time
            waitingTime = SIMTIME_DBL(simTime()) - previousTime;
            //statistic
            emit(waitingTimeSignal, waitingTime);
            previousTime = SIMTIME_DBL(simTime());

            nextProcessTime = SIMTIME_DBL(simTime()) + maxDelay; // convert simtime to double, the first time set up nextProcessTime =  currenttime
            allowSend = false;
        }
//2)Check AggSessAC conditions
        if (((queue.length() >= maxBundleSize) || (SIMTIME_DBL(simTime()) >= nextProcessTime)) && numOfAcceptedPkt <= 0)
        {

            walkOnQueue(); 		//called walkOnQueue
            deleteRejectedPkt();//delete rejected session after decision-making process

        //logging waiting time
            waitingTime = SIMTIME_DBL(simTime()) - previousTime; //session time in aggsessACQueue
            //statistic
            emit(waitingTimeSignal, waitingTime);
            previousTime = SIMTIME_DBL(simTime());

            nextProcessTime = SIMTIME_DBL(simTime()) + maxDelay;
            emit(nextProcessTimeSignal, nextProcessTime);
        }
}

//If session is accepted traffic generator host add new data flow with requested parameters
void AggSessACQueue::addFlow(cMessage *msg)
{
    //http://www.omnetpp.org/doc/omnetpp/manual/usman.html#sec155
       requsetBw = getReqBw(msg);
       serviceType = getToS(msg);
       cum_revenue =  cum_revenue + serviceType;

    //Control of traffic source
       cModule *targetModule = getModuleByPath("network1.host[0].udpApp[0]");
       currentsendInt = targetModule->par("sendInterval").doubleValue();    // unit (s)
       currentMsgLen = targetModule->par("messageLength").longValue()+ 18 + 20 + 8;      //+ (14Ethernet header + 4CRC = 18B) + (IP Header 20B) + (UDP header is 8 bytes);
       currentRate = (long)(round(1/currentsendInt))*(8*currentMsgLen);
       nextRate =  currentRate + (requsetBw*1000);          // next rate after accepted SYN packet with reqBw
       pktsRate = nextRate/(8*currentMsgLen);               // number of pkts per second
       interval = (double)(1/(double) pktsRate);            // next send interval

    //statistic
       emit(accepted_tosOfSYNPkSignal, (long)serviceType);
       emit(accepted_reqBwOfSYNPkSignal, (long)requsetBw);
       emit(cum_revenueSignal, cum_revenue);

       parIdx1 = targetModule->findPar("sendInterval");     // Returns index of the sendInterval parameter
       parIdx2 = targetModule->findPar("messageLength");    // Returns index of the messageLength parameter
       targetModule->par(parIdx1).setDoubleValue(interval);

    //Update UtilizationMeter
       UtilizationMeter::addReqBwToVector(requsetBw);       // accounting accepted sessions 
       UtilizationMeter::setAvailBw(nextRate);              // available bandwidth
}

// insert packet in queue
cMessage *AggSessACQueue::enqueue(cMessage *msg)
{
    //Timer that check AggSessConditions every 1ms
    if (msg == timerEvent) //(msg == timerEvent) or (msg->isSelfMessage() or msg->isScheduled()) can as well
    {
        checkAggSessConditions();   
        cancelEvent(timerEvent);    //waiting time timer reset
        double ctimer = SIMTIME_DBL(simTime());
        emit(timerSignal, ctimer);
        scheduleAt(simTime() + timer, timerEvent); //scheduleAt(simTime()+timeout, timeoutEvent) or scheduleAt(simTime() + delay, msg);
        return NULL;
     }
     else   // otherwise take packet and insert in queue
     {
        if (frameCapacity && queue.length() >= frameCapacity)
        {
            EV << "Queue full, dropping packet.\n";
            return msg;
        }
        else // continue session aggregation process
        {
            checkAggSessConditions();
            queue.insert(msg);
            emit(queueLengthSignal, queue.length());
            qlen = queue.length();      
            return NULL;
        }
     }
}

//take out packets from queue
cMessage *AggSessACQueue::dequeue()
{
    if (queue.empty())
        return NULL;

    if (numOfAcceptedPkt > 0)
    {
        cMessage *msg = takeAcceptedPkt();
            if (msg == NULL)
                {
                error("Warnings_1: dequeue return NULL. dequeue problem1: cannot read packet!");
                }
            numOfAcceptedPkt--;
            checkAggSessConditions(); 
            return msg;
    }
    else
    {
        checkAggSessConditions();
        cMessage *msg = takeAcceptedPkt();
        if (msg == NULL)
                {
                    error("Warnings_2: dequeue return NULL. dequeue problem2: cannot read packet!");
                }
        numOfAcceptedPkt--;
        return msg;
    }
}

//each queue module contain sendOut function
void AggSessACQueue::sendOut(cMessage *msg)
{
    frameSize = getframeSizeInBits(msg);            
    UtilizationMeter::sumOfFrameBits(frameSize);    
    serviceType = getToS(msg);
    if (serviceType == 0)
    {
        error("Warnings_3: AggSessACQueue::sendOut ToS == 0!");
    }
    addFlow(msg);
    send(msg, outGate);

    if (ev.isGUI())
           {
             char buf[40];
             sprintf(buf, "q accepted: %d\nq rejected: %d", accepted, rejected);
             getDisplayString().setTagArg("t", 0, buf);
           }
}

bool AggSessACQueue::isEmpty()
{
    return queue.empty();
}
