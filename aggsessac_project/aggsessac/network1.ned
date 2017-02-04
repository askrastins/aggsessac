//
// Network topology
// 

package aggsessac;

import inet.applications.IUDPApp;
import inet.applications.generic.IPvXTrafGen;
import inet.applications.generic.IPvXTrafSink;
import inet.applications.udpapp.UDPBasicBurst;
import inet.base.LifecycleController;
import inet.examples.ethernet.lans.cable;
import inet.examples.inet.routerperf.BurstHost; // Definition of an IP node with a transport generator that connects to IP directly, without TCP or UDP.
import inet.networklayer.autorouting.ipv4.IPv4NetworkConfigurator;
import inet.nodes.inet.Router;
import inet.nodes.inet.StandardHost;
import inet.util.ThruputMeteringChannel;
import inet.world.scenario.ScenarioManager;
import ned.DatarateChannel;
import ned.IdealChannel;
import inet.nodes.ethernet.EtherSwitch;

//import for GenNode
import inet.applications.generic.IIPvXTrafficGenerator;
import inet.nodes.inet.NodeBase;
import inet.transport.ITCP;
import inet.examples.diffserv.onedomain.DSQueue2;


//
//A submodule type may be specified with a module parameter. The syntax uses the 'like' keyword.
//by specifying the type via a module parameter (e.g. writing <param> like

// Ja sagaidms ka mainîgâ vçrtîba nolasâma atkârtoti un tâ ik reiz var izmanâs kâ piemçram ar 
// the serviceTime parameteru tad mainîgo NED failâ apzîmç kâ 'volatile' - gaistoðu (example: 'volatile double serviceTime;')
//
// TODO documentation
//
network network1
{
    parameters:
        int numhosts;
        @display("bgb=882,200");
    submodules:
        Configurator1: IPv4NetworkConfigurator {
            @display("p=228,15");
            dumpConfig = "network1_config";
            //"<interface hosts='r1' names='eth2' address='192.168.3.1' netmask='255.255.255.0'/>\n" +
            //"<interface hosts='host*' address='192.168.3.x' netmask='255.255.255.0'/>\n" +
            //"<interface hosts='r1' names='eth2' address='192.168.3.1' netmask='255.255.255.0'/>\n" +
            //"<interface hosts='host*' address='192.168.3.x' netmask='255.255.255.0'/>\n" +
            config = xml("<config>\n" +
   				"<interface hosts='SrcHost' names='eth0' address='192.168.1.x' netmask='255.255.255.0'/>\n" +
   				"<interface hosts='host*' address='192.168.1.x' netmask='255.255.255.0'/>\n" +
   				"<interface hosts='r1' names='eth0' address='192.168.1.1' netmask='255.255.255.0'/>\n" +
  				"<interface hosts='r1' names='eth1' address='10.10.1.1' netmask='255.255.255.252'/>\n" +
  				"<interface hosts='r2' names='eth0' address='10.10.1.2' netmask='255.255.255.252'/>\n" +
  				"<interface hosts='r2' names='eth1' address='192.168.2.1' netmask='255.255.255.0'/>\n" +
   				"<interface hosts='DestHost' names='eth0' address='192.168.2.2' netmask='255.255.255.0'/>\n" +
   				
				"</config>");
        }
        SrcHost: SynGen {
            parameters:
                @display("p=47,46");
            gates:
                ethg[1];  //Ethernet MAC which supports full-duplex operation ONLY (10Mbps, 100Mbps 1Gbps)
        }
        host[numhosts]: StandardHost {
            parameters:
                @display("i=device/pc3;p=47,155");
        }
        switch: EtherSwitch {
            parameters:
                @display("p=159,97");
                csmacdSupport = false;
                macType = "EtherMACFullDuplex";
            gates:
                ethg[3];
        }
        r1: Router {
            @display("p=296,98");
            gates:
                ethg[2];  //Ethernet MAC which supports full-duplex operation ONLY (10Mbps, 100Mbps 1Gbps)
        }
        r2: Router {
            @display("p=521,98");
            gates:
                ethg[2];  //Ethernet MAC which supports full-duplex operation ONLY (10Mbps, 100Mbps 1Gbps)
        }
        DestHost: BurstHost {
            @display("p=787,98");
            gates:
                ethg[1];  //Ethernet MAC which supports full-duplex operation ONLY (10Mbps, 100Mbps 1Gbps)
        }
        lifecycleController: LifecycleController {
            @display("p=333,15");
        }
        scenarioManager: ScenarioManager {
            @display("p=437,15");
            //"<at t='6s'><tell module='lifecycleController' operation='NodeStartOperation' target='SrcHost'/></at>\n" +
            //"<at t='9s'><tell module='lifecycleController' operation='NodeShutdownOperation' target='r2'/></at>\n" +
            //"<at t='10s'><tell module='lifecycleController' operation='NodeStartOperation' target='r2'/></at>\n" +
            //"<at t='6s'><tell module='lifecycleController' operation='NodeStartOperation' target='SrcHost'/></at>\n" +
            //"<at t='9s'><tell module='lifecycleController' operation='NodeShutdownOperation' target='r2'/></at>\n" +
            //"<at t='10s'><tell module='lifecycleController' operation='NodeStartOperation' target='r2'/></at>\n" +
            //"<at t='6s'><tell module='lifecycleController' operation='NodeStartOperation' target='SrcHost'/></at>\n" +
            //"<at t='9s'><tell module='lifecycleController' operation='NodeShutdownOperation' target='r2'/></at>\n" +
            //"<at t='10s'><tell module='lifecycleController' operation='NodeStartOperation' target='r2'/></at>\n" +
            //"<at t='6s'><tell module='lifecycleController' operation='NodeStartOperation' target='SrcHost'/></at>\n" +
            //"<at t='9s'><tell module='lifecycleController' operation='NodeShutdownOperation' target='r2'/></at>\n" +
            //"<at t='10s'><tell module='lifecycleController' operation='NodeStartOperation' target='r2'/></at>\n" +
            //"<at t='6s'><tell module='lifecycleController' operation='NodeShutdownOperation' target='SrcHost'/></at>\n" +
            //"<at t='6s'><tell module='lifecycleController' operation='NodeStartOperation' target='SrcHost'/></at>\n" +
            //"<at t='9s'><tell module='lifecycleController' operation='NodeShutdownOperation' target='r2'/></at>\n" +
            //"<at t='10s'><tell module='lifecycleController' operation='NodeStartOperation' target='r2'/></at>\n" +
            //"<at t='6s'><tell module='lifecycleController' operation='NodeShutdownOperation' target='SrcHost'/></at>\n" +
            //"<at t='6s'><tell module='lifecycleController' operation='NodeStartOperation' target='SrcHost'/></at>\n" +
            //"<at t='9s'><tell module='lifecycleController' operation='NodeShutdownOperation' target='r2'/></at>\n" +
            //"<at t='10s'><tell module='lifecycleController' operation='NodeStartOperation' target='r2'/></at>\n" +
            //"<at t='6s'><tell module='lifecycleController' operation='NodeShutdownOperation' target='SrcHost'/></at>\n" +
            //"<at t='6s'><tell module='lifecycleController' operation='NodeStartOperation' target='SrcHost'/></at>\n" +
            //"<at t='9s'><tell module='lifecycleController' operation='NodeShutdownOperation' target='r2'/></at>\n" +
            //"<at t='10s'><tell module='lifecycleController' operation='NodeStartOperation' target='r2'/></at>\n" +
            //"<at t='6s'><tell module='lifecycleController' operation='NodeShutdownOperation' target='SrcHost'/></at>\n" +
            //"<at t='6s'><tell module='lifecycleController' operation='NodeStartOperation' target='SrcHost'/></at>\n" +
            //"<at t='9s'><tell module='lifecycleController' operation='NodeShutdownOperation' target='r2'/></at>\n" +
            //"<at t='10s'><tell module='lifecycleController' operation='NodeStartOperation' target='r2'/></at>\n" +
            script = xml("<script>\n" +
                //"<at t='6s'><tell module='lifecycleController' operation='NodeShutdownOperation' target='SrcHost'/></at>\n" +
                //"<at t='6s'><tell module='lifecycleController' operation='NodeStartOperation' target='SrcHost'/></at>\n" +
                //"<at t='9s'><tell module='lifecycleController' operation='NodeShutdownOperation' target='r2'/></at>\n" +
                //"<at t='10s'><tell module='lifecycleController' operation='NodeStartOperation' target='r2'/></at>\n" +
                "</script>");
        }
    connections:
        SrcHost.ethg[0] <--> eth100M <--> switch.ethg++;
        switch.ethg++ <--> eth100M <--> r1.ethg[0];
        //r1.ethg[0] <--> eth100M <--> r1.ethg[0];
        r1.ethg[1] <--> eth10M <--> r2.ethg[0];
        r2.ethg[1] <--> eth100M <--> DestHost.ethg[0];
        for i=0..numhosts-1 {
            host[i].ethg++ <--> eth100M <--> switch.ethg++;
        }
}

//
// via host control distribution of inter-arrival time of new session 
//
module SynGen extends NodeBase
{
    parameters:
        @node();
        @display("i=device/pc3");
        string trafGenType = "IPvXTrafGen";
    submodules:
        trafGen: IPvXTrafGen {
            parameters:
                @display("p=329,81");
     		//https://groups.google.com/forum/#!topic/omnetpp/pvC-6fYuq_c	
  				//sendIntervalFile = "sendInt_logn_6200_dev_0015.csv"; 	// can also be determined via .ini file
                //sendInterval = exponential(5ms); 						// can also be determined via .ini file
        }
        connections allowunconnected:
        // transport connections
        networkLayer.transportOut++ --> trafGen.ipIn;
        networkLayer.transportIn++ <-- trafGen.ipOut;
}


channel eth100M extends ThruputMeteringChannel
{
    parameters:
        datarate = 100Mbps;  //EtherMACFullDuplex Supported variations: 10Mb Ethernet (full-duplex mode),100Mb Ethernet (full-duplex mode), 1Gb Ethernet (full-duplex mode)
        //delay = 0.0565us; // 10m
        per = 0;
        ber = 0;
        thruputDisplayFormat = "u";
        @display("ls=#008040,3");
}
channel eth10M extends ThruputMeteringChannel
{
    parameters:
        datarate = 10Mbps;  //EtherMACFullDuplex Supported variations: 10Mb Ethernet (full-duplex mode),100Mb Ethernet (full-duplex mode), 1Gb Ethernet (full-duplex mode)
        //delay = 0.0565us; // 10m
        per = 0;
        ber = 0;
        thruputDisplayFormat = "u";
        @display("ls=#80FF80,2");
}
