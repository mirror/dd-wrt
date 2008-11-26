#include <Copyright.h>
/********************************************************************************
* header.c
*
* DESCRIPTION:
*		This sample shows how to enable/disable CPU port's ingress and egress 
*		Header mode. For more information about Header mode, please refer to 
*		88E6063 Data Book. Header mode sould be handled by Ethernet Device/Driver
*		as well, since 88E6063, with header mode enabled, sends out a packet with
*		header, which cannot be recognized by regular Ethernet device/driver,
*		and expects header for every received packet.
*
* DEPENDENCIES:
*		88E6051, 88E6052, and 88E6021 are not supporting this feature.
*
* FILE REVISION NUMBER:
*
* COMMENTS:
*		WARNING!!
*		When Header mode for the CPU port is enabled, Ethernet Device/Driver 
*		which connects to the CPU port should understand Header Format.
*		If Ethernet Device does not know about Header mode, then user may set
*		the device to Promiscuous mode in order to receive packets from QD's CPU
*		port. After that, it is Ethernet Device Driver's responsibility to handle
*		Header properly.
*******************************************************************************/

#include "msSample.h"

GT_STATUS sampleHeaderEnable(GT_QD_DEV *dev,GT_BOOL en)
{
	GT_STATUS status;

	/*
	 *	Enable/Disable Header mode
	*/
	if((status = gprtSetHeaderMode(dev,dev->cpuPortNum, en)) != GT_OK)
	{
		MSG_PRINT(("gprtSetHeaderMode return Failed\n"));
		return status;
	}

	return GT_OK;
}

