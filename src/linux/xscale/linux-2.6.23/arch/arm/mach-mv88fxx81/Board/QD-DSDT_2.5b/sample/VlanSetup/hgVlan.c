#include <Copyright.h>
/********************************************************************************
* hgVlan.c
*
* DESCRIPTION:
*       Setup the VLAN table of QuaterDeck so that it can be used as a Home 
*		Gateway.
*		
*
* DEPENDENCIES:   None.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/
#include "msSample.h"

static GT_STATUS sampleHomeGatewayVlan(GT_QD_DEV *dev, 
		                       GT_LPORT numOfPorts, 
				       GT_LPORT cpuPort);

/*
 *  Get the required parameter from QuarterDeck driver.
 *	Notes: This routine should be called after QuarterDeck Driver has been initialized.
 *		(Refer to Initialization Sample)
*/

GT_STATUS sampleVlanSetup(GT_QD_DEV *dev)
{
	sampleHomeGatewayVlan(dev,dev->numOfPorts, dev->cpuPortNum);

	return GT_OK;
}


/*
 *	WAN Port (Port 0) and CPU Port (Port 5) are in VLAN 1 and
 *	all ports (including CPU Port) except WAN Port are in VLAN 2.
 *	1) Set PVID for each port. (CPU port has PVID 2, which is the same as LAN)
 *	2) Set Port Based VLAN Map for each port. (CPU port's VLAN Map is set for all LAN ports)
 *  Notes: 
 *		1) Trailer Mode
 *			When Ethernet Device, which is directly connected to CPU port, sends out a packet
 *			to WAN, DPV in Trailer Tag should have WAN port bit set (bit 0 in this case), and
 *			to LAN, Trailer Tag should be set to 0. 
 *			Restriction : Only one group of VLAN can have multiple ports.
 *		2) Header Mode
 *			When Ethernet Device, which is directly connected to CPU port, sends out a packet
 *			to WAN, VlanTable in Header Tag should have WAN ports bits set (bit 0 in this case), and
 *			to LAN, VlanTable in Header Tag should have LAN ports bits set (bit 1~4 and 6 in this case)
*/
static GT_STATUS sampleHomeGatewayVlan(GT_QD_DEV *dev,GT_LPORT numOfPorts, GT_LPORT cpuPort)
{
	GT_STATUS status;
	GT_LPORT index,port,portToSet;
	GT_LPORT portList[MAX_SWITCH_PORTS];

	/* 
	 *  set PVID for each port.
	 *	the first port(port 0, WAN) has default VID 2 and all others has 1.
	 */

	if((status = gvlnSetPortVid(dev,0,2)) != GT_OK)
	{
		MSG_PRINT(("gprtSetPortVid returned fail.\n"));
		return status;
	}

	for (port=1; port<numOfPorts; port++)
	{
		if((status = gvlnSetPortVid(dev,port,1)) != GT_OK)
		{
			MSG_PRINT(("gprtSetPortVid returned fail.\n"));
			return status;
		}
	}

	/* 
	 *  set Port VLAN Mapping.
	 *	port 0 (WAN) and cpu port are in a vlan 2.
	 *	And all the rest ports (LAN) and cpu port are in a vlan 1.
	 */

	/* port 0 : set cpuPort only */
	portList[0] = cpuPort;
	if((status = gvlnSetPortVlanPorts(dev,0,portList,1)) != GT_OK)
	{
		MSG_PRINT(("gvlnSetPortVlanPorts returned fail.\n"));
		return status;
	}

	/* set all ports except port 0 and itself */
	for (portToSet=1; portToSet<numOfPorts; portToSet++)
	{
		/* port 0 and cpuPort will be taken cared seperately. */
		if (portToSet == cpuPort)
		{
			continue;
		}

		index = 0;
		for (port=1; port<numOfPorts; port++)
		{
			if (port == portToSet)
			{
				continue;
			}
			portList[index++] = port;
		}

		if((status = gvlnSetPortVlanPorts(dev,portToSet,portList,index)) != GT_OK)
		{
			MSG_PRINT(("gvlnSetPortVlanPorts returned fail.\n"));
			return status;
		}
	}

	/* cpuPort : set all port except cpuPort and WAN port */
	index = 0;
	for (port=1; port<numOfPorts; port++)
	{
		if (port == cpuPort)
		{
			continue;
		}
		portList[index++] = port;
	}

	if((status = gvlnSetPortVlanPorts(dev,cpuPort,portList,index)) != GT_OK)
	{
		MSG_PRINT(("gvlnSetPortVlanPorts returned fail.\n"));
		return status;
	}

	return GT_OK;
}
