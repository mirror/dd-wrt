========================================================================
		VLAN Setup for Home Gateway Solution
========================================================================

Vlan Setup Program will show how to setup the QuaterDeck's vlan 
for Home Gateway.
In the sample program, Port 0 (WAN port) and CPU Port (Port 5) are in 
a VLAN 2, and Port 1 ~ Port 6 (including CPU Port) are in a VLAN 1.

VLAN MAP setting for the given sample program is:
Port 0 (WAN) = 0x20,
Port 1 (LAN) = 0x7C,
Port 2 (LAN) = 0x7A,
Port 3 (LAN) = 0x76,
Port 4 (LAN) = 0x6E,
Port 5 (CPU) = 0x5E, and
Port 6 (LAN) = 0x3E

Notes: 
	1) Trailer Mode is enabled:
		When Ethernet Device, which is directly connected to CPU port, sends out a packet
		to WAN, DPV in Trailer Tag should have WAN port bit set (bit 0 in this case), and
		to LAN, Trailer Tag should be set to 0. 
		Restriction : Only one group of VLANs can have multiple ports.
	2) Header Mode is enabled:
		When Ethernet Device, which is directly connected to CPU port, sends out a packet
		to WAN, VlanTable in Header Tag should have WAN ports bits set (bit 0 in this case), and
		to LAN, VlanTable in Header Tag should have LAN ports bits set (bit 1~4 and 6 in this case)

hgVlan.c
	sampleHGVlanSetup is the main function for the Home Gateway setup.







