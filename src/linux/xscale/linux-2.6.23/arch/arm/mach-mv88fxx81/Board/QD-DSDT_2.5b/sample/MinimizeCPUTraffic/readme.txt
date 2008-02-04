========================================================================
		Setup for CPU Port which is not a member of any VLAN
========================================================================

Previous SOHO switch devices were low port count and/or used for Routers.
In this environment, the CPU must be a member of all VLANs, so it can
route the frames from one VLAN to another.
In a high port count managed switch, the CPU is not a router but the 
manager of the switch. In this environment, the CPU doesn't want to be a 
member of any VLAN. If it is, it can get saturated with non-management 
frames preventing it from receiving the important management frames.

In order to support the feature, the following has to be provided:

1. For the devices that support gsysSetARPDest API:
	0) Remove CPU port from VLAN Member Table.
	1) Mirror ARPs to the CPU with To_CPU Marvell Tag.
	2) Convert unicast frames directed to the CPU into To_CPU Marvell Tag.

2. For the devices that support gprtSetARPtoCPU API:
	0) Remove CPU port from VLAN Member Table.
	1) Enable ARP to CPU for each port.







