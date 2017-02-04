//
// Module mark packet with appropriate priorities
// Allows set up different type of input traffic pattern 
// with different distribution of ToS (priorities) 
// 

package aggsessac.NED;

//
// Traffic conditioner (TC)
//
import inet.linklayer.ITrafficConditioner;
import inet.networklayer.diffserv.MultiFieldClassifier;
import inet.networklayer.diffserv.SYNMarker;


module TCsyn like ITrafficConditioner
{
    parameters:
        @display("i=block/classifier;bgb=242,199");

    gates:
        input in;
        output out;
    submodules:
        classifier: MultiFieldClassifier {
            @display("p=62,91");
        }
        synMarker: SYNMarker {
            @display("p=181,91");
        }
    connections:
        in --> classifier.in;
        classifier.defaultOut --> synMarker.in++;
        synMarker.out --> out;
}
