package inet.networklayer.diffserv;

//
// This classifier contains a list of filters that identifies
// the flows and determines their classes.
// Each filter can match the source and destination address,
// IP protocol number, source and destination ports, or ToS
// of the datagram. The first matching filter determines the
// index of the out gate. If no matching filter is found,
// then the packet will be sent through the defaultOut gate.
//
//
simple MultiFieldClassifier
{
    parameters:
        xml filters = default(xml("<filters/>"));
        @display("i=block/classifier");

        @signal[pkClass](type=long);
        @statistic[pkClass](title="packet class"; source=pkClass; record=vector; interpolationmode=none);
    gates:
        input in;
        output outs[];
        output defaultOut;
}
