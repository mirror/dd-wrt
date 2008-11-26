#include <Copyright.h>
/*******************************************************************************
* 802_1q.c
*
* DESCRIPTION:
*		There are three 802.1Q modes (GT_SECURE, GT_CHECK, and GT_FALLBACK).
*		In GT_SECURE mode, the VID for the given frame must be contained in 
*		the VTU, and the Ingress port must be a member of the VLAN or the 
*		frame will be discarded.
*		In GT_CHECK mode, the VID for the given frame must be contained in 
*		the VTU or the frame will be discarded (the frame will not be 
*		discarded if the Ingress port is not a memeber of the VLAN).
*		In GT_FALLBACK mode, Frames are not discarded if their VID's are not 
*		contained in the VTU. If the frame's VID is contained in the VTU, the 
*		frame is allowed to exit only those ports that are members of the 
*		frame's VLAN; otherwise the switch 'falls back' into Port Based VLAN 
*		mode for the frame (88E6021 Spec. section 3.5.2.1).
*
*		Egress Tagging for a member port of a Vlan has the following three 
*		choices:
*		1) Unmodified,
*		2) Untagged, and
*		3) Tagged
*
*		This sample shows how to utilize 802.1Q feature in the device.
*		For more information, please refer to 88E6021 Spec. section 3.5.2.3
*
* DEPENDENCIES:
*		88E6021 and 88E6063 are supporting this feature.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#include "msSample.h"


/*****************************************************************************
* sample802_1qSetup
*
* DESCRIPTION:
*		This routine will show how to configure the switch device so that it 
*		can be a Home Gateway. This example assumes that all the frames are not 
*		VLAN-Tagged.
*		1) to clear VLAN ID Table,
* 		2) to enable 802.1Q in SECURE mode for each port except CPU port,
*		3) to enable 802.1Q in FALL BACK mode for the CPU port. 
*		4) to add VLAN ID 1 with member port 0 and CPU port 
*		(untagged egress),
*		5) to add VLAN ID 2 with member the rest of the ports and CPU port 
*		(untagged egress), 
*		6) to configure the default vid of each port:
*		Port 0 have PVID 1, CPU port has PVID 3, and the rest ports have PVID 2.
*		Note: CPU port's PVID should be unknown VID, so that QuarterDeck can use 
*		VlanTable (header info) for TX.
*
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*
* COMMENTS: 
*		WARNING!!
*		If you create just two VLAN for this setup, Trailer mode or Header mode 
*		for the CPU port has to be enabled and Ethernet driver which connects to
*		CPU port should understand VLAN-TAGGING, Trailer mode, or Header mode.
*
*******************************************************************************/
GT_STATUS sample802_1qSetup(GT_QD_DEV *dev)
{
	GT_STATUS status;
	GT_DOT1Q_MODE mode;
        GT_VTU_ENTRY vtuEntry;
	GT_U16 vid;
	GT_LPORT port;
	int i;

	/*
	 *	1) Clear VLAN ID Table
	*/
	if((status = gvtuFlush(dev)) != GT_OK)
	{
		MSG_PRINT(("gvtuFlush returned fail.\n"));
		return status;
	}

	/*
	 *	2) Enable 802.1Q for each port as GT_SECURE mode except CPU port.
	*/
	mode = GT_SECURE;
	for(i=0; i<dev->numOfPorts; i++)
	{
		port = i;
		if (port == dev->cpuPortNum)
			continue;

		if((status = gvlnSetPortVlanDot1qMode(dev,port, mode)) != GT_OK)
		{
			MSG_PRINT(("gvlnSetPortVlanDot1qMode return Failed\n"));
			return status;
		}
	}

	/*
	 *	3) Enable 802.1Q for CPU port as GT_FALLBACK mode
	*/
	if((status = gvlnSetPortVlanDot1qMode(dev, dev->cpuPortNum, GT_FALLBACK)) != GT_OK)
	{
		MSG_PRINT(("gvlnSetPortVlanDot1qMode return Failed\n"));
		return status;
	}

	/*
	 *	4) Add VLAN ID 1 with Port 0 and CPU Port as members of the Vlan.
	*/
	gtMemSet(&vtuEntry,0,sizeof(GT_VTU_ENTRY));
	vtuEntry.DBNum = 0;
	vtuEntry.vid = 1;
	for(i=0; i<dev->numOfPorts; i++)
	{
		port = i;
		if((i==0) || (port == dev->cpuPortNum))
			vtuEntry.vtuData.memberTagP[port] = MEMBER_EGRESS_UNTAGGED;
		else
			vtuEntry.vtuData.memberTagP[port] = NOT_A_MEMBER;
	}

	if((status = gvtuAddEntry(dev,&vtuEntry)) != GT_OK)
	{
		MSG_PRINT(("gvtuAddEntry returned fail.\n"));
		return status;
	}

	/*
	 *	5) Add VLAN ID 2 with the rest of the Ports and CPU Port as members of 
	 *	the Vlan.
	*/
	gtMemSet(&vtuEntry,0,sizeof(GT_VTU_ENTRY));
	vtuEntry.DBNum = 0;
	vtuEntry.vid = 2;
	for(i=0; i<dev->numOfPorts; i++)
	{
		port = i;
		if(i == 0)
			vtuEntry.vtuData.memberTagP[port] = NOT_A_MEMBER;
		else
			vtuEntry.vtuData.memberTagP[port] = MEMBER_EGRESS_UNTAGGED;
	}

	if((status = gvtuAddEntry(dev,&vtuEntry)) != GT_OK)
	{
		MSG_PRINT(("gvtuAddEntry returned fail.\n"));
		return status;
	}


	/*
	 *	6) Configure the default vid for each port.
	 *	Port 0 has PVID 1, CPU port has PVID 3, and the rest ports have PVID 2.
	*/
	for(i=0; i<dev->numOfPorts; i++)
	{
		port = i;
		if(i==0)
			vid = 1;
		else if(port == dev->cpuPortNum)
			vid = 3;
		else
			vid = 2;

		if((status = gvlnSetPortVid(dev,port,vid)) != GT_OK)
		{
			MSG_PRINT(("gvlnSetPortVid returned fail.\n"));
			return status;
		}
	}

	return GT_OK;

}


/*****************************************************************************
* sampleAdmitOnlyTaggedFrame
*
* DESCRIPTION:
*		This routine will show how to configure a port to accept only vlan
*		tagged frames.
*		This routine assumes that 802.1Q has been enabled for the given port.
*
* INPUTS:
*       port - logical port to be configured.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
*
* COMMENTS: 
*		Some device support Discard Untagged feature. If so, gprtSetDiscardUntagged
*		function will do the work.
*
*******************************************************************************/
GT_STATUS sampleAdmitOnlyTaggedFrame(GT_QD_DEV *dev,GT_LPORT port)
{
	GT_STATUS status;
    GT_VTU_ENTRY vtuEntry;
	int i;

	/*
	 *	0) If device support gprtSetDiscardUntagged, call the function.
	*/
	status = gprtSetDiscardUntagged(dev, port, GT_TRUE);
	switch (status)
	{
		case GT_OK:
			MSG_PRINT(("Done.\n"));
			return status;
		case GT_NOT_SUPPORTED:
			MSG_PRINT(("Try other method.\n"));
			break;
		default:
			MSG_PRINT(("Failure accessing device.\n"));
			return status;
	}
			

	/*
	 *	1) Add VLAN ID 0xFFF with the given port as a member.
	*/
	gtMemSet(&vtuEntry,0,sizeof(GT_VTU_ENTRY));
	vtuEntry.DBNum = 0;
	vtuEntry.vid = 0xFFF;
	for(i=0; i<dev->numOfPorts; i++)
	{
		vtuEntry.vtuData.memberTagP[i] = NOT_A_MEMBER;
	}
	vtuEntry.vtuData.memberTagP[port] = MEMBER_EGRESS_TAGGED;

	if((status = gvtuAddEntry(dev,&vtuEntry)) != GT_OK)
	{
		MSG_PRINT(("gvtuAddEntry returned fail.\n"));
		return status;
	}

	/*
	 *	2) Configure the default vid for the given port with VID 0xFFF
	*/
	if((status = gvlnSetPortVid(dev,port,0xFFF)) != GT_OK)
	{
		MSG_PRINT(("gvlnSetPortVid returned fail.\n"));
		return status;
	}

	return GT_OK;

}


/*****************************************************************************
* sampleDisplayVIDTable
*
* DESCRIPTION:
*		This routine will show how to enumerate each vid entry in the VTU table
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK               - on success
*       GT_FAIL             - on error
* COMMENTS: 
*
*******************************************************************************/
GT_STATUS sampleDisplayVIDTable(GT_QD_DEV *dev)
{
	GT_STATUS status;
        GT_VTU_ENTRY vtuEntry;
	GT_LPORT port;	
	int portIndex;

	gtMemSet(&vtuEntry,0,sizeof(GT_VTU_ENTRY));
	vtuEntry.vid = 0xfff;
	if((status = gvtuGetEntryFirst(dev,&vtuEntry)) != GT_OK)
	{
		MSG_PRINT(("gvtuGetEntryCount returned fail.\n"));
		return status;
	}

	MSG_PRINT(("DBNum:%i, VID:%i \n",vtuEntry.DBNum,vtuEntry.vid));

	for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
	{
		port = portIndex;

		MSG_PRINT(("Tag%i:%#x  ",port,vtuEntry.vtuData.memberTagP[port]));
	}
	
	MSG_PRINT(("\n"));

	while(1)
	{
		if((status = gvtuGetEntryNext(dev,&vtuEntry)) != GT_OK)
		{
			break;
		}

		MSG_PRINT(("DBNum:%i, VID:%i \n",vtuEntry.DBNum,vtuEntry.vid));

		for(portIndex=0; portIndex<dev->numOfPorts; portIndex++)
		{
			port = portIndex;

			MSG_PRINT(("Tag%i:%#x  ",port,vtuEntry.vtuData.memberTagP[port]));
		}
	
		MSG_PRINT(("\n"));

	}
	return GT_OK;
}
