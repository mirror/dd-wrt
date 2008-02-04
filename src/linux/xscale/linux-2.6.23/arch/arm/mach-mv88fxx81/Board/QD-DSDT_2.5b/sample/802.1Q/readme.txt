========================================================================
			802.1Q Feature
========================================================================

There are three 802.1Q modes (GT_SECURE, GT_CHECK, and GT_FALLBACK).
In GT_SECURE mode, the VID for the given frame must be contained in 
the VTU, and the Ingress port must be a member of the VLAN or the 
frame will be discarded.
In GT_CHECK mode, the VID for the given frame must be contained in 
the VTU or the frame will be discarded (the frame will not be 
discarded if the Ingress port is not a memeber of the VLAN).
In GT_FALLBACK mode, Frames are not discarded if their VID's are not 
contained in the VTU. If the frame's VID is contained in the VTU, the 
frame is allowed to exit only those ports that are members of the 
frame's VLAN; otherwise the switch 'falls back' into Port Based VLAN 
mode for the frame (88E6021 Spec. section 3.5.2.1).

Egress Tagging for a member port of a Vlan has the following three 
choices:
1) Unmodified,
2) Untagged, and
3) Tagged

This sample shows how to utilize 802.1Q feature in the device.
For more information, please refer to 88E6021 Spec. section 3.5.2.3.

802_1q.c
	sample802_1qSetup
		This routine will show
		1) how to enable 802.1Q feature for each port,
		2) how to clear VLAN ID (VTU) Table,
 		3) how to enable 802.1Q in SECURE mode for each port, 
		4) how to add VLAN ID 1 with member port 0 and CPU port 
		(unmodified egress),
		5) how to add VLAN ID 2 with member the rest of the ports and CPU port 
		(untagged egress), 
		6) how to configure the default vid of each port:
		Port 0 and CPU port have PVID 1 and the rest ports have PVID 2.

	sampleAdmitOnlyTaggedFrame
		This routine will show how to configure a port to accept only vlan
		tagged frames.
		This routine assumes that 802.1Q has been enabled for the given port.

	sampleDisplayVIDTable
		This routine will show how to enumerate each vid entry in the VTU table







