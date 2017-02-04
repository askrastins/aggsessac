package inet.applications.generic;

//
// Sends IPv4 or IPv6 datagrams to the given address at the given sendInterval.
// The sendInterval can be readed from file or a constant or a random value (e.g. exponential(1)).
// If the destAddresses parameter contains more than one address, one
// of them is randomly for each packet. An address may be given in the
// dotted decimal notation (or, for IPv6, in the usual notation with colons),
// or with the module name. (The IPvXAddressResolver class is used to resolve
// the address.) To disable the model, set destAddresses to "". with .ini or into this .ned file
//
// The peer can be ~IPvXTrafSink or another ~IPvXTrafGen (it handles received packets
// exactly like ~IPvXTrafSink).
//
simple IPvXTrafGen like IIPvXTrafficGenerator
{
    parameters:
        double startTime @unit("s") = default(1s); // time of sending the first packet
        double stopTime @unit("s") = default(-1s);  // time of finishing sending, negative values mean forever
        volatile double sendInterval @unit("s") = default(10ms); // if not set default will be 10 ms
        int numPackets = default(-1); // max number of packets to generate, -1 means forever
        int protocol; // value for ~IPv4ControlInfo / ~IPv6ControlInfo protocol field
        volatile int packetLength @unit("B"); // packet length in bytes
        string destAddresses = default(""); // list of destination addresses, separated by spaces
        string sendIntervalFile = default(""); // papildinâts ar Connection request inter-arrival time
        @display("i=block/source");
        @signal[sentPk](type=cPacket);
        @signal[rcvdPk](type=cPacket);
        @statistic[rcvdPk](title="packets received"; source=rcvdPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[sentPk](title="packets sent"; source=sentPk; record=count,"sum(packetBytes)","vector(packetBytes)"; interpolationmode=none);
        @statistic[endToEndDelay](title="end-to-end delay"; source="messageAge(rcvdPk)"; unit=s; record=histogram,vector; interpolationmode=none);
    gates:
        input ipIn @labels(IPv4ControlInfo/up);
        input ipv6In @labels(IPv6ControlInfo/up);
        output ipOut @labels(IPv4ControlInfo/down);
        output ipv6Out @labels(IPv6ControlInfo/down);
}

