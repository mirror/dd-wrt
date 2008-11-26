#include <Copyright.h>
/********************************************************************************
* minizeCPUTraffic.c
*
* DESCRIPTION:
*		This sample shows how to setup the CPU port not to be a member of any 
*		VLAN, while it still be a manager of a switch. 
*		
* DEPENDENCIES:
*		Please check the device's spec. if the device supports this feature.
*		At the moment this sample was written, 88E6095 was the only device support
*		this feature.
*
* FILE REVISION NUMBER:
*
* COMMENTS:
*******************************************************************************/

#include "msSample.h"

/*
	For the devices that support gsysSetARPDest API:

	0) Remove CPU port from VLAN Member Table.
	   (this sample deals with Port Based Vlan only.)
	1) Mirror ARPs to the CPU with To_CPU Marvell Tag
	2) Convert unicast frames directed to the CPU into To_CPU Marvell Tag
	Assumption : Device ID, Cascading Port, CPU Port, and Interswitch Port are
		already set properly. For more information, please refer to the 
		sample/MultiDevice/msApiInit.c
*/

GT_STATUS sampleMinimizeCPUTraffic1(GT_QD_DEV *dev, GT_U8* macAddr)
{
	GT_STATUS status;
	int i;
	GT_LPORT memPorts[16], cpuPort;
	GT_U8 memPortsLen, index;
    GT_ATU_ENTRY macEntry;

	cpuPort = (GT_LPORT)dev->cpuPortNum;

	/*
	 *	Remove CPU port from VLAN Member Table.
	*/ 
	for(i=0; i<dev->numOfPorts; i++)
	{
		if((status = gvlnGetPortVlanPorts(dev,(GT_LPORT)i,memPorts,&memPortsLen)) != GT_OK)
		{
			MSG_PRINT(("gvlnGetPortVlanPorts return Failed\n"));
			return status;
		}

		for(index=0; index<memPortsLen; index++)
		{
			if (memPorts[index] == cpuPort)
				break;
		}

		if(index != memPortsLen)
		{
			/* CPU Port is the member of the port vlan */
			if((memPortsLen-1) != index)
			{
				memPorts[index] = memPorts[memPortsLen-1];
			}
			memPortsLen--;

			if((status = gvlnSetPortVlanPorts(dev,(GT_LPORT)i,memPorts,memPortsLen)) != GT_OK)
			{
				MSG_PRINT(("gvlnSetPortVlanPorts return Failed\n"));
				return status;
			}
		}
	}

	/*
	 *	Mirror ARPs to the CPU with To_CPU Marvell Tag.
	*/
	if((status = gsysSetARPDest(dev,cpuPort)) != GT_OK)
	{
		MSG_PRINT(("gsysSetARPDest return Failed\n"));
		return status;
	}

	/*
	 *	Convert unicast frames directed to the CPU into To_CPU Marvell Tag.
	 *  This sample assumes that DBNum is not used. If DBNum is used,
	 *  the macEntry has to be added for each DBNum used.
	*/
	memset(&macEntry,0,sizeof(GT_ATU_ENTRY));
	memcpy(macEntry.macAddr.arEther,macAddr,6);
	macEntry.portVec = 1 << dev->cpuPortNum;
	macEntry.prio = 0;			/* Priority (2bits). When these bits are used they override
								any other priority determined by the frame's data */
	macEntry.entryState.ucEntryState = GT_UC_TO_CPU_STATIC;
	macEntry.DBNum = 0;
	macEntry.trunkMember = GT_FALSE;

	if((status = gfdbAddMacEntry(dev,&macEntry)) != GT_OK)
	{
		MSG_PRINT(("gfdbAddMacEntry return Failed\n"));
		return status;
	}

	
	return GT_OK;
}


/*
	For the devices that support gprtSetARPtoCPU API:

	1) Enable ARP to CPU feature fore each port.
	2) Set Egress Flood Mode to be Block Unknown DA on CPU Port.
	3) Add CPU Port's MAC into address table.
	4) Remove Broadcast address from address table.
*/

GT_STATUS sampleMinimizeCPUTraffic2(GT_QD_DEV *dev, GT_U8* macAddr)
{
	GT_STATUS status;
	int i;
	GT_LPORT cpuPort;
    GT_ATU_ENTRY macEntry;

	cpuPort = (GT_LPORT)dev->cpuPortNum;

	/*
	 *	Remove CPU port from VLAN Member Table.
	*/ 
	for(i=0; i<dev->numOfPorts; i++)
	{
		if (i == cpuPort)
			continue;

		if((status = gprtSetARPtoCPU(dev,i,GT_TRUE)) != GT_OK)
		{
			MSG_PRINT(("gprtSetARPtoCPU return Failed\n"));
			return status;
		}
	}

	/*
	 * Set Egress Flood Mode to be Block Unknown DA on CPU Port.
	*/
	if((status = gprtSetEgressFlood(dev,cpuPort,GT_BLOCK_EGRESS_UNKNOWN)) != GT_OK)
	{
		MSG_PRINT(("gprtSetEgressFlood return Failed\n"));
		return status;
	}


	/*
	 *	Add CPU's MAC into address table.
	 *  This sample assumes that DBNum is not used. If DBNum is used,
	 *  the macEntry has to be added for each DBNum used.
	*/
	memset(&macEntry,0,sizeof(GT_ATU_ENTRY));
	memcpy(macEntry.macAddr.arEther,macAddr,6);
	macEntry.portVec = 1 << dev->cpuPortNum;
	macEntry.prio = 0;			/* Priority (2bits). When these bits are used they override
								any other priority determined by the frame's data */
	macEntry.entryState.ucEntryState = GT_UC_STATIC;
	macEntry.DBNum = 0;
	macEntry.trunkMember = GT_FALSE;

	if((status = gfdbAddMacEntry(dev,&macEntry)) != GT_OK)
	{
		MSG_PRINT(("gfdbAddMacEntry return Failed\n"));
		return status;
	}

	/*
	 *  Delete BroadCast Entry from address table if exists.
	 *  This sample assumes that DBNum is not used. If DBNum is used,
	 *  the macEntry has to be added for each DBNum used.
	*/
	memset(&macEntry,0,sizeof(GT_ATU_ENTRY));
	memset(macEntry.macAddr.arEther,0xFF,6);
	gfdbDelAtuEntry(dev,&macEntry);
	
	return GT_OK;
}


