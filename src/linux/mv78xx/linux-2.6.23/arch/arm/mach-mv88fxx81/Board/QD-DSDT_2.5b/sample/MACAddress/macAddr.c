#include <Copyright.h>
/********************************************************************************
* macAddr.c
*
* DESCRIPTION:
*	This sample will demonstrate how to add/delete a static MAC Address 
*	into/from the QuaterDeck MAC Address Data Base.
*		
* DEPENDENCIES:   None.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/
#include "msSample.h"

/*
 *	Add the CPU MAC address into the QuaterDeck MAC Address database.
 *	Input - None
*/
GT_STATUS sampleAddCPUMac(GT_QD_DEV *dev)
{
	GT_STATUS status;
	GT_ATU_ENTRY macEntry;

	/* 
	 *	Assume that Ethernet address for the CPU MAC is
	 *	00-50-43-00-01-02.
	*/
	macEntry.macAddr.arEther[0] = 0x00;
	macEntry.macAddr.arEther[1] = 0x50;
	macEntry.macAddr.arEther[2] = 0x43;
	macEntry.macAddr.arEther[3] = 0x00;
	macEntry.macAddr.arEther[4] = 0x01;
	macEntry.macAddr.arEther[5] = 0x02;

	macEntry.portVec = 1 << dev->cpuPortNum; 	/* CPU Port number. 7bits are used for portVector. */

	macEntry.prio = 0;			/* Priority (2bits). When these bits are used they override
								any other priority determined by the frame's data. This value is
								meaningful only if the device does not support extended priority
								information such as MAC Queue Priority and MAC Frame Priority */

	macEntry.exPrio.macQPri = 0;	/* If device doesnot support MAC Queue Priority override, 
									this field is ignored. */
	macEntry.exPrio.macFPri = 0;	/* If device doesnot support MAC Frame Priority override, 
									this field is ignored. */
	macEntry.exPrio.useMacFPri = 0;	/* If device doesnot support MAC Frame Priority override, 
									this field is ignored. */

	macEntry.entryState.ucEntryState = GT_UC_STATIC;
								/* This address is locked and will not be aged out.
								Refer to GT_ATU_UC_STATE in msApiDefs.h for other option. */

	/* 
	 *	Add the MAC Address.
	 */
	if((status = gfdbAddMacEntry(dev,&macEntry)) != GT_OK)
	{
		MSG_PRINT(("gfdbAddMacEntry returned fail.\n"));
		return status;
	}

	return GT_OK;
}


/*
 *	Delete the CPU MAC address from the QuaterDeck MAC Address database.
 *	Input - None
*/
GT_STATUS sampleDelCPUMac(GT_QD_DEV *dev)
{
	GT_STATUS status;
	GT_ATU_ENTRY macEntry;

	/* 
	 *	Assume that Ethernet address for the CPU MAC is
	 *	00-50-43-00-01-02.
	*/
	macEntry.macAddr.arEther[0] = 0x00;
	macEntry.macAddr.arEther[1] = 0x50;
	macEntry.macAddr.arEther[2] = 0x43;
	macEntry.macAddr.arEther[3] = 0x00;
	macEntry.macAddr.arEther[4] = 0x01;
	macEntry.macAddr.arEther[5] = 0x02;

	/* 
	 *	Delete the CPU MAC Address.
	 */
	if((status = gfdbDelMacEntry(dev,&macEntry.macAddr)) != GT_OK)
	{
		MSG_PRINT(("gfdbDelMacEntry returned fail.\n"));
		return status;
	}

	return GT_OK;
}


/*
 *	Add a multicast MAC address into the QuaterDeck MAC Address database,
 *	where address is 01-00-18-1a-00-00 and frames with this destination has
 *	to be forwarding to Port 1, Port 2 and Port 4 (port starts from Port 0)
 *	Input - None
*/
GT_STATUS sampleAddMulticastAddr(GT_QD_DEV *dev)
{
	GT_STATUS status;
	GT_ATU_ENTRY macEntry;

	/* 
	 *	Assume that we want to add the following multicast address
	 *	01-50-43-00-01-02.
	*/
	macEntry.macAddr.arEther[0] = 0x01;
	macEntry.macAddr.arEther[1] = 0x50;
	macEntry.macAddr.arEther[2] = 0x43;
	macEntry.macAddr.arEther[3] = 0x00;
	macEntry.macAddr.arEther[4] = 0x01;
	macEntry.macAddr.arEther[5] = 0x02;

	/*
	 * 	Assume that a packet needs to be forwarded to the second Port (port 1),
	 *	the third Port (port 2) and cpu Port, if the frame has destination of
	 *	01-00-18-1a-00-00.
	*/
	macEntry.portVec = 	(1<<1) | /* the second port */
				(1<<2) | /* the third port */
				(1<<dev->cpuPortNum);

	macEntry.prio = 0;			/* Priority (2bits). When these bits are used they override
								any other priority determined by the frame's data. This value is
								meaningful only if the device does not support extended priority
								information such as MAC Queue Priority and MAC Frame Priority */

	macEntry.exPrio.macQPri = 0;	/* If device doesnot support MAC Queue Priority override, 
									this field is ignored. */
	macEntry.exPrio.macFPri = 0;	/* If device doesnot support MAC Frame Priority override, 
									this field is ignored. */
	macEntry.exPrio.useMacFPri = 0;	/* If device doesnot support MAC Frame Priority override, 
									this field is ignored. */

	macEntry.entryState.ucEntryState = GT_MC_STATIC;
								/* This address is locked and will not be aged out. 
								Refer to GT_ATU_MC_STATE in msApiDefs.h for other option.*/

	/* 
	 *	Add the MAC Address.
	 */
	if((status = gfdbAddMacEntry(dev,&macEntry)) != GT_OK)
	{
		MSG_PRINT(("gfdbAddMacEntry returned fail.\n"));
		return status;
	}

	return GT_OK;
}


/*
 *	Delete the Multicast MAC address of 01-00-18-1a-00-00.
 *	Input - None
*/
GT_STATUS sampleDelMulticastAddr(GT_QD_DEV *dev)
{
	GT_STATUS status;
	GT_ATU_ENTRY macEntry;

	/* 
	 *	Assume that Ethernet address for the CPU MAC is
	 *	01-50-43-00-01-02.
	*/
	macEntry.macAddr.arEther[0] = 0x01;
	macEntry.macAddr.arEther[1] = 0x50;
	macEntry.macAddr.arEther[2] = 0x43;
	macEntry.macAddr.arEther[3] = 0x00;
	macEntry.macAddr.arEther[4] = 0x01;
	macEntry.macAddr.arEther[5] = 0x02;

	/* 
	 *	Delete the given Multicast Address.
	 */
	if((status = gfdbDelMacEntry(dev,&macEntry.macAddr)) != GT_OK)
	{
		MSG_PRINT(("gfdbDelMacEntry returned fail.\n"));
		return status;
	}

	return GT_OK;
}


/*
 *	This sample function will show how to display all the MAC address
 *	in the ATU.
*/
GT_STATUS sampleShowMacEntry(GT_QD_DEV *dev)
{
	GT_STATUS status;
	GT_ATU_ENTRY tmpMacEntry;

	MSG_PRINT(("ATU List:\n"));
	memset(&tmpMacEntry,0,sizeof(GT_ATU_ENTRY));

	while(1)
	{
		/* Get the sorted list of MAC Table. */
		if((status = gfdbGetAtuEntryNext(dev,&tmpMacEntry)) != GT_OK)
		{
			return status;
		}

		MSG_PRINT(("(%02x-%02x-%02x-%02x-%02x-%02x) PortVec %#x\n",
				tmpMacEntry.macAddr.arEther[0],
				tmpMacEntry.macAddr.arEther[1],
				tmpMacEntry.macAddr.arEther[2],
				tmpMacEntry.macAddr.arEther[3],
				tmpMacEntry.macAddr.arEther[4],
				tmpMacEntry.macAddr.arEther[5],
				tmpMacEntry.portVec));
	}
	return GT_OK;
}
