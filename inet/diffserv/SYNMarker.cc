//Allows set up different type of input traffic pattern 
//with different distribution of ToS (priorities) 
//and requested performance (bandwidth)

#ifdef WITH_IPv4
#include "IPv4Datagram.h"
#endif

#include "DSCP_m.h"
#include "SYNMarker.h"

#include "DiffservUtil.h"

using namespace DiffservUtil;

Define_Module(SYNMarker);

//logging info
simsignal_t SYNMarker::markSYNPkSignal = registerSignal("markSYNPk");
simsignal_t SYNMarker::tosOfSYNPkSignal = registerSignal("tosOfSYNPk"); //ToS (priorities)
simsignal_t SYNMarker::reqBwOfSYNPkSignal = registerSignal("reqBwOfSYNPk"); //Bandwidth
simsignal_t SYNMarker::distributionTestIntSignal = registerSignal("distributionTestInt"); //test distribution of inter-arrival time of new sessions
simsignal_t SYNMarker::distributionTestDoubleSignal = registerSignal("distributionTestDouble");
simsignal_t SYNMarker::synSendTimeSignal = registerSignal("synSendTime"); // inter-arrival time of new sessionsendInterval distribution

void SYNMarker::initialize()
{
    //
    //##########################################################
        //parseDSCPs(par("dscps"), "dscps", dscps); //
        //if (dscps.empty())
        //    dscps.push_back(DSCP_BE);
        //while ((int)dscps.size() < gateSize("in"))
        //    dscps.push_back(dscps.back());

    lastTime = 0;

    numRcvd = 0;
    numMarked = 0;
    WATCH(numRcvd);
    WATCH(numMarked);
}

// packet handling, uncommnet necessary distribution
void SYNMarker::handleMessage(cMessage *msg)
{
    cPacket *packet = dynamic_cast<cPacket*>(msg);
    if (packet)
    {
        numRcvd++;
        //DSCP field (lower six bit of Tos/TrafficClass)
        //int dscp = parseDSCP("AF31", "dscps"); //AF31 = decimal 26
//Intuniform
        //int dscp = intuniform(1, 7);            //param a, b  the interval, a<b, @param rng the underlying random number generator
        //if (dscp > 7){dscp = 1;}
//Exponential distribution of ToS values
        int dscp = round(exponential(2)+1);
            if (dscp==9) {dscp=8;} else if (dscp==10) {dscp=7;} else if (dscp>10) {dscp=1;}
//Beta distribution of ToS values
        //int dscp = round((beta(0.5, 0.5)*7)+1);
//Beta2 distribution of ToS values
        //int dscp = round((beta(2, 1)*8));
        //    if (dscp<1) {dscp=1;}

        std::string dscp_str = dscpToString(dscp);

//reqBW kbits/s
        //int reqBw = intuniform(50, 1000);
        int reqBw = round(gamma_d(3.0, 2.0)*100);
        if (reqBw < 50) {reqBw = 75;} else if (reqBw > 2000) {reqBw = 680;}

        simtime_t currentTime = simTime();
        emit(synSendTimeSignal, simTime()-lastTime);
        lastTime = currentTime;

  //distribution test
        //int v = poisson(1.5);
        //int v = binomial(7, 0.3);
        //int v = geometric(0.3);
  //convert to discrete
        //int v = round(exponential(2)+1);
            //if (v==9) {v=8;} else if (v==10) {v=7;} else if (v>10) {v=1;}
        //int v = round(gamma_d(2.0, 2.0));
        //int v = round((beta(2, 1)*8));
            //if (v<1) {v=1;}
        //int v = round(erlang_k(1.0, 2.0)+1);
        //int v = round(chi_square(2));
        //int v = round(student_t(2));
        //int v = round((cauchy(3.0, 3.0)*10));
        //int v = round((lognormal(0, 0.25)*10));
        //int v = round(weibull(1, 1.5));
        //int v = round(pareto_shifted(3, 7, 0));
        //int v = round(gamma_d(3.0, 2.0)*100);

        //emit(distributionTestIntSignal, (long)v);

        if (markPacket(packet, dscp, reqBw))
        {
            //statistic
            emit(markSYNPkSignal, packet);
            emit(tosOfSYNPkSignal, (long)dscp);
            emit(reqBwOfSYNPkSignal, (long)reqBw);
            numMarked++;
        }

        send(packet, "out");
    }
    else
        throw cRuntimeError("SYNMarker expects cPackets, this wasn't cPacket");

    if (ev.isGUI())
    {
        char buf[50] = "";
        if (numRcvd>0) sprintf(buf+strlen(buf), "rcvd: %d ", numRcvd);
        if (numMarked>0) sprintf(buf+strlen(buf), "mark:%d ", numMarked);
        getDisplayString().setTagArg("t", 0, buf);
    }
}

//packet marking
bool SYNMarker::markPacket(cPacket *packet, int dscp, int reqBw)
{
    EV << "Marking packet with dscp= " << dscpToString(dscp) << " and reqBw= " << dscpToString(reqBw) << "\n";

    for ( ; packet; packet = packet->getEncapsulatedPacket())
    {
#ifdef WITH_IPv4
        IPv4Datagram *ipv4Datagram = dynamic_cast<IPv4Datagram *>(packet);
        if (ipv4Datagram)
        {
            ipv4Datagram->setDiffServCodePoint(dscp);
            ipv4Datagram->setOptionCode(reqBw);
            return true;
        }
#endif
    }
    return false;
}
