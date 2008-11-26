========================================================================
		Port Monitor Setup
========================================================================

88E6063 device supports Port Monitoring, which allows a user to monitor
all the traffic of a certain port. 
This sample shows how to enable/disable Port Monitoring.
For more information about Port Monitoring, please refer to 88E6063 Spec.

Note :
Port monitoring supported by ClipperShip has two modes:
1. Egress only monitoring (monitor packets comming out of the 
monitored port, and
2. Egress and Ingress monitoring (monitor packet comming in and out
from the monitored port)

portMonitor.c
	sampleEgressMonitor can be used to enable Egress only port monitoring.
	samplePortMonitor can be used to enable Egress and Ingress monitoring.
	sampleDisablePortMonitor can be used to disable monitoring 
	(both Egress only mode and Egress and Ingress mode).







