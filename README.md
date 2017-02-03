# aggsessac
Aggregated Session Admission Control (AggSessAC)
==================================
  The author of this project has developed a new connection admission control method for high-speed internet networks. Taking into account the increasing data flow intensity (new flows/s), which on the high speed Gigabit networks has already been measured with a magnitude of 10^3 or higher, project author proposes idea to perform a temporary accumulation of a new connection requests prior to a decision making by directing a new session initialization packets to the separate queue. Connection request represents the packets that belonging to a new single data flow (session). Such action get opportunity to make to make mutual comparition among collected requests. AggSessAC is able to provide cross-evaluation of several connection requests, thus expanding the decision-making capabilities by enabling flow-level selective, priority-based and pro-active admission management. Developed AggSessAC solution that is based on short-term (us-ms) collection of requests allows to bring the virtualization of parallel processing into connection admission control and reduce uncertain conditions at the moment of decision-making. AggSessAC solution increases the quality of the admission decisions made and as a result the more accurately to fulfill the management policy.
  
  Within AggSessAC method a new connection request accumulation is controlled by two parameters, with can be modified via TC102.ned file within aggsessac project: 
  
1) maximum waiting time - time of aggregation period for new connection request accumulation

2) maximum bundle size - queue size for connection requests


How to start AggSessAC project in the OMNeT++
=========================================

OMNeT++ available: https://omnetpp.org/omnetpp
After successful OMNET ++ installation is required to download The INET Framework  from the download link on the https://inet.omnetpp.org/Download.html, unpack it, and follow the instructions in the INSTALL file. Documentation and tutorials are also available above mentioned web pages. 

Before you run the  AggSessAC project is necessary to supplement the INET Framework library to the my developed source (.cc), the header (.H) and (.ned) files:
Download from this repository ‘inet’ folder, which contains the necessary .cc, .h, .ned files. 
Go to your INET framework src directory, for example: C:\...\omnetpp-4.6\samples\inet\src\
and copy following files to your INET directory:

Copy
--------------------
AggSessACQueue.cc
AggSessACQueue.h
AggSessACQueue.ned
DataThrQueue.cc
DataThrQueue.h
DataThrQueue.ned
FIFOQueue.cc
FIFOQueue.h
FIFOQueue.ned
PriorityScheduler1.cc
PriorityScheduler1.h
PriorityScheduler1.ned
to your …\samples\inet\src\linklayer\queue
-----------------------

UtilizationMeter.cc
UtilizationMeter.h
UtilizationMeter.ned
MultiFieldClassifier.cc
MultiFieldClassifier.h
MultiFieldClassifier.ned
SYNMarker.cc
SYNMarker.h
SYNMarker.ned
to your …\samples\inet\src\networklayer\diffserv
-----------------------

PassiveQueueBase.cc, 
PassiveQueueBase.h, 
PassiveQueueBase2.c, 
PassiveQueueBase2.h 
to your	…\samples\inet\src\base
-----------------------

IPvXTrafGen.cc
IPvXTrafGen.h
IPvXTrafGen.ned
to your … samples\inet\src\applications\generic


Then you can download full AggSessAC project its available in this repository within folder ‘aggsessac’.

Finally, can make any changes and modifications if you want.

/Andris/
 
