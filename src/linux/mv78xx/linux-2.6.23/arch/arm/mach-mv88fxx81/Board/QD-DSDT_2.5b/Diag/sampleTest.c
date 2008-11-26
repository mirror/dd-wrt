#include <Copyright.h>
/********************************************************************************
* sample.c
*
* DESCRIPTION:
*		This is a sample program shows how to use DSDT APIs.
*		
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*
* COMMENTS:
*******************************************************************************/

#include "msSample.h"

#define N_OF_QD_DEVICES	2
extern GT_QD_DEV       *qdMultiDev[N_OF_QD_DEVICES];

GT_STATUS qdMultiDevStart();
GT_STATUS sampleIsolatedCPUPort(GT_QD_DEV *dev, GT_U8* macAddr);
GT_STATUS setTagMode(int vid, char* tag);
GT_STATUS crossChipTrunkSetup();


GT_STATUS enableTag()
{
	char tag[4] = {0xc0,0,0,0x1};
	setTagMode(2,tag);
	return GT_OK;
}


GT_STATUS sampleTest()
{
	GT_U8 macAddr[6] = {0x0,0x23,0x45,0x67,0x89,0xab};
	qdMultiDevStart();
	enableTag();
	sampleIsolatedCPUPort(qdMultiDev[0],macAddr);
	crossChipTrunkSetup();
	return GT_OK;
}

/*
	0) Remove CPU port from VLAN Member Table.
	   (this sample deals with Port Based Vlan only.)
	1) Mirror ARPs to the CPU with To_CPU Marvell Tag
	2) Convert unicast frames directed to the CPU into To_CPU Marvell Tag
	Assumption : Device ID, Cascading Port, CPU Port, and Interswitch Port are
		already set properly. For more information, please refer to the 
		sample/MultiDevice/msApiInit.c
*/

GT_STATUS sampleIsolatedCPUPort(GT_QD_DEV *dev, GT_U8* macAddr)
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
		MSG_PRINT(("gsysSetARPDest return Failed\n"));
		return status;
	}

	
	return GT_OK;
}


/*
	Assumption 1: Device ID, Cascading Port, CPU Port, and Interswitch Port are
		already set properly. For more information, please refer to the 
		sample/MultiDevice/msApiInit.c

	Assumption 2: Port 0,1,2 of Device 0 and Port 0 of Device 1 are member of a 
		trunk with Trunk ID 1.
*/

GT_STATUS sampleFixedCrossChipTrunk(GT_QD_DEV *dev[])
{
	GT_STATUS status;
	int i;
	GT_U32 mask, trunkBit, trunkId;

	/*
	 *	Enable Trunk for each member of the Trunk and set the Trunk ID (1).
	*/ 

	trunkId = 1;

	if((dev[0] == NULL) || (!dev[0]->devEnabled))
	{
		printf("Device 0 is not initialized\n");
		return GT_FAIL;
	}
	if((dev[1] == NULL) || (!dev[1]->devEnabled))
	{
		printf("Device 1 is not initialized\n");
		return GT_FAIL;
	}

	/* setup for Device 0 port 0 */
	if((status = gprtSetTrunkPort(dev[0],0,GT_TRUE,trunkId)) != GT_OK)
	{
		MSG_PRINT(("gprtSetTrunkPort return Failed\n"));
		return status;
	}

	/* setup for Device 0 port 1 */
	if((status = gprtSetTrunkPort(dev[0],1,GT_TRUE,trunkId)) != GT_OK)
	{
		MSG_PRINT(("gprtSetTrunkPort return Failed\n"));
		return status;
	}

	/* setup for Device 0 port 2 */
	if((status = gprtSetTrunkPort(dev[0],2,GT_TRUE,trunkId)) != GT_OK)
	{
		MSG_PRINT(("gprtSetTrunkPort return Failed\n"));
		return status;
	}

	/* setup for Device 1 port 0 */
	if((status = gprtSetTrunkPort(dev[1],0,GT_TRUE,trunkId)) != GT_OK)
	{
		MSG_PRINT(("gprtSetTrunkPort return Failed\n"));
		return status;
	}


	/*
	 *	Set Trunk Route Table for the given Trunk ID.
	*/ 

	/* setup for Device 0, trunk ID 1 : port 0,1,2, and 9 (cascading port, assumption1) */
	if((status = gsysSetTrunkRouting(dev[0],trunkId,0x7|0x200)) != GT_OK)
	{
		MSG_PRINT(("gsysSetTrunkRouting return Failed\n"));
		return status;
	}

	/* setup for Device 1, trunk ID 1 : port 0, and 8 (cascading port, assumption1) */
	if((status = gsysSetTrunkRouting(dev[1],trunkId,0x1|0x100)) != GT_OK)
	{
		MSG_PRINT(("gsysSetTrunkRouting return Failed\n"));
		return status;
	}


	/*
	 *	Set Trunk Mask Table for load balancing.
	*/ 

	/*
	   Trunk Mask Table for Device 0:
 
						10	9	8	7	6	5	4	3	2	1	0
	   TrunkMask[0]		1	1	1	1	1	1	1	1	0	0	1
	   TrunkMask[1]		1	1	1	1	1	1	1	1	0	1	0
	   TrunkMask[2]		1	1	1	1	1	1	1	1	1	0	0
	   TrunkMask[3]		1	1	1	1	1	1	1	1	0	0	0
	   TrunkMask[4]		1	1	1	1	1	1	1	1	0	0	1
	   TrunkMask[5]		1	1	1	1	1	1	1	1	0	1	0
	   TrunkMask[6]		1	1	1	1	1	1	1	1	1	0	0
	   TrunkMask[7]		1	1	1	1	1	1	1	1	0	0	0


	   Trunk Mask Table for Device 1:
 
						10	9	8	7	6	5	4	3	2	1	0
	   TrunkMask[0]		1	1	1	1	1	1	1	1	1	1	0
	   TrunkMask[1]		1	1	1	1	1	1	1	1	1	1	0
	   TrunkMask[2]		1	1	1	1	1	1	1	1	1	1	0
	   TrunkMask[3]		1	1	1	1	1	1	1	1	1	1	1
	   TrunkMask[4]		1	1	1	1	1	1	1	1	1	1	0
	   TrunkMask[5]		1	1	1	1	1	1	1	1	1	1	0
	   TrunkMask[6]		1	1	1	1	1	1	1	1	1	1	0
	   TrunkMask[7]		1	1	1	1	1	1	1	1	1	1	1

	*/

	/* setup for Device 0 */
	for(i=0; i<8; i++)
	{
		if((i%4) == 3)
		{
			trunkBit = 0;
		}
		else
		{
			trunkBit = 1 << (i%4);
		}		

		mask = 0x7F8 | trunkBit;
	
		if((status = gsysSetTrunkMaskTable(dev[0],i,mask)) != GT_OK)
		{
			MSG_PRINT(("gsysSetTrunkMaskTable return Failed\n"));
			return status;
		}

	}
	
	/* setup for Device 1 */
	for(i=0; i<8; i++)
	{
		if((i%4) == 3)
		{
			trunkBit = 1;
		}
		else
		{
			trunkBit = 0;
		}		

		mask = 0x7FE | trunkBit;
	
		if((status = gsysSetTrunkMaskTable(dev[1],i,mask)) != GT_OK)
		{
			MSG_PRINT(("gsysSetTrunkMaskTable return Failed\n"));
			return status;
		}

	}
	return GT_OK;
}

#define MAX_PORT_IN_TRUNK 4

typedef struct _TRUNK_SET {
	GT_U32	devIndex;
	GT_U32	port;
} TRUNK_SET;

typedef struct _TRUNK_MEMBER {
	GT_U32	trunkId;
	GT_U32	nTrunkPort;
	TRUNK_SET trunkSet[MAX_PORT_IN_TRUNK];
} TRUNK_MEMBER;

GT_STATUS sampleCrossChipTrunk(GT_QD_DEV *dev[], TRUNK_MEMBER* tm);

/*
	Setup Trunk with the following member ports:
		Port 0,1,2 of Device 0, and
		Port 0 of Device 1,
	where Device 0 is the first Switch Device Structure in qdMultiDev array 
	and Device 1 is the second Switch Device Structure in qdMultiDev array.
*/
GT_STATUS crossChipTrunkSetup()
{
	TRUNK_MEMBER tm;

	tm.trunkId = 1;
	tm.nTrunkPort = 4;
	tm.trunkSet[0].devIndex = 0;
	tm.trunkSet[0].port = 0;
	tm.trunkSet[1].devIndex = 0;
	tm.trunkSet[1].port = 1;
	tm.trunkSet[2].devIndex = 0;
	tm.trunkSet[2].port = 2;
	tm.trunkSet[3].devIndex = 1;
	tm.trunkSet[3].port = 0;

	return sampleCrossChipTrunk(qdMultiDev, &tm);
}

GT_STATUS sampleCrossChipTrunk(GT_QD_DEV *dev[], TRUNK_MEMBER* tm)
{
	GT_STATUS status;
	int i,j,index;
	GT_U32 mask, trunkId;
	TRUNK_SET* ts;
	GT_U32 portVec[N_OF_QD_DEVICES];	
	GT_U32 casecadeVec = 0xC0;	/* Port 6 and 7. ToDo : get this value from user or device */

	/*
	 *	Enable Trunk for each member of the Trunk and set the Trunk ID (1).
	*/ 

	printf("Setting TRUNK\n");
	printf("Trunk ID : %i\n",(unsigned int)tm->trunkId);
	printf("N Ports  : %i\n",(unsigned int)tm->nTrunkPort);
	printf("1st Port  : Dev %i, Port %i\n",
			(unsigned int)tm->trunkSet[0].devIndex,(unsigned int)tm->trunkSet[0].port);
	printf("2nd Port  : Dev %i, Port %i\n",
			(unsigned int)tm->trunkSet[1].devIndex,(unsigned int)tm->trunkSet[1].port);
	printf("3rd Port  : Dev %i, Port %i\n",
			(unsigned int)tm->trunkSet[2].devIndex,(unsigned int)tm->trunkSet[2].port);
	printf("4th Port  : Dev %i, Port %i\n",
			(unsigned int)tm->trunkSet[3].devIndex,(unsigned int)tm->trunkSet[3].port);

	trunkId = tm->trunkId;

	for(i=0; i<N_OF_QD_DEVICES; i++)
		portVec[i] = 0;

	printf("Enabling TRUNK for each member port.\n");
	for(i=0; i<tm->nTrunkPort; i++)
	{
		ts = &tm->trunkSet[i];

		if(ts->devIndex >= N_OF_QD_DEVICES)
		{
			printf("Device %i is supported. Max Device Number is %i\n",(unsigned int)ts->devIndex,N_OF_QD_DEVICES-1);
			return GT_FAIL;
		}

		if((dev[ts->devIndex] == NULL) || (!dev[ts->devIndex]->devEnabled))
		{
			printf("Device %i is not initialized\n",(unsigned int)ts->devIndex);
			return GT_FAIL;
		}

		/* enabled trunk on the given port */
		if((status = gprtSetTrunkPort(dev[ts->devIndex],ts->port,GT_TRUE,trunkId)) != GT_OK)
		{
			MSG_PRINT(("gprtSetTrunkPort return Failed\n"));
			return status;
		}

		portVec[ts->devIndex] |= (1 << ts->port);
	}

	/*
	 *	Set Trunk Route Table for the given Trunk ID.
	*/ 
	printf("Setting TRUNK Routing Table\n");
	for(i=0; i<N_OF_QD_DEVICES; i++)
	{
		if((dev[i] == NULL) || (!dev[i]->devEnabled))
		{
			printf("Device %i is not initialized\n",i);
			break;
		}

		if((status = gsysSetTrunkRouting(dev[i],trunkId,portVec[i]|casecadeVec)) != GT_OK)
		{
			MSG_PRINT(("gsysSetTrunkRouting return Failed\n"));
			return status;
		}
	}

	/*
	 *	Set Trunk Mask Table for load balancing.
	*/ 
	printf("Setting TRUNK Mask for Load Balancing\n");
	for(i=0; i<8; i++)
	{
		/* choose a port to be used for the given addr combo index */
		index = i % tm->nTrunkPort;
		ts = &tm->trunkSet[index];
		
		for(j=0; j<N_OF_QD_DEVICES; j++)
		{
			if((dev[j] == NULL) || (!dev[j]->devEnabled))
			{
				printf("Device %i is not initialized\n",j);
				continue;
			}

			if(portVec[j] == 0)
				continue;

			if((status = gsysGetTrunkMaskTable(dev[j],i,&mask)) != GT_OK)
			{
				MSG_PRINT(("gsysGetTrunkMaskTable return Failed\n"));
				return status;
			}

			mask &= ~portVec[j];

			if(ts->devIndex == j)
				mask |= (1 << ts->port);
			
			if((status = gsysSetTrunkMaskTable(dev[j],i,mask)) != GT_OK)
			{
				MSG_PRINT(("gsysSetTrunkMaskTable return Failed\n"));
				return status;
			}
		}
	}
	
	return GT_OK;
}
