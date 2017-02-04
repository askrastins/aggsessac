package inet.networklayer.diffserv;

//
// This module sets the ToS(lower six bit - dscp) field of IP datagrams
// 
// Packets arrived at gate in[i] are marked with the ith value. 
//
simple SYNMarker
{
    parameters:
        //string dscps; //

        @display("i=block/star");

		//logojam visu nepieciesamo info
        @signal[markSYNPk](type=cPacket);
        @signal[tosOfSYNPk](type=long);
        @signal[reqBwOfSYNPk](type=long);
        @signal[distributionTestInt](type=long);
        @signal[distributionTestDouble](type=double);
        @signal[synSendTime](type=simtime_t; unit=ms);
        @statistic[markSYNPk](title="SYN packets marked"; source=markSYNPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[tosOfSYNPk](title="ToS in SYN packet"; source=tosOfSYNPk; record=histogram,vector; interpolationmode=none);
        @statistic[reqBwOfSYNPk](title="reqBw in SYN packet"; source=reqBwOfSYNPk; record=histogram,vector; interpolationmode=none);
        @statistic[distributionTestInt](title="distribution test"; source=distributionTestInt; record=histogram,vector; interpolationmode=none);
        @statistic[distributionTestDouble](title="distribution test"; source=distributionTestDouble; record=histogram,vector; interpolationmode=none);
        @statistic[synSendTime](title="SYN sendInterval time"; source=synSendTime; record=histogram,vector; interpolationmode=none);
        
    gates:
        input in[];
        output out;
}