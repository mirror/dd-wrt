#include <Copyright.h>
/********************************************************************************
* pirl.c
*
* DESCRIPTION:
*       Setup PIRL buckets
*
* DEPENDENCIES:   None.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/
#include "msSample.h"


/*
 *  This setup function configures the Port 0 of Marvell SOHO Switch Device with
 *	capability of PIRL to be :
 *
 *	1) Ingress Rate    : 128Kbps (128K bit per sec)
 *	2) Discarded frame : Do not account discarded frame due to queue congestion
 *	3) Filtered frame  : Account filtered frame
 *	4) Limit action    : Drop packets when the incoming rate exceeds the limit
 *	5) Rate type       : Rate is based on Traffic type
 *	6) Traffic type    : ARP, MGMT, Multicast, Broadcast, and Unicast frames are
 *						 tracked as part of the rate resource calculation.
 *	7) Byte counted    : Account only Layer 3 bytes (IP header and payload)
 *
 *	Notes: This sample uses IRL Unit 0. The available number of IRL Units are
 *	various depending on the device. Please refer to the datasheet for detailed
 *	information.
 *
 *	Notes: Port 0 will be blocked while programming PIRL.
*/

GT_STATUS samplePIRLSetup(GT_QD_DEV *dev)
{
	GT_STATUS status;
	GT_PIRL_DATA pirlData;
	GT_U32		irlUnit;
	GT_LPORT 	port;

	/* change Current Timer Update Interval */
	status = gpirlSetCurTimeUpInt(dev,4);
	switch (status)
	{
		case GT_OK:
			break;
		case GT_NOT_SUPPORTED:
			MSG_PRINT(("Device is not supporting PIRL.\n"));
			return status;
		default:
			MSG_PRINT(("Failure to configure device.\n"));
			return status;
	}

	irlUnit = 0;
	port = 0;

	pirlData.ingressRate 		= 128;
	pirlData.accountQConf 		= GT_FALSE;
	pirlData.accountFiltered	= GT_TRUE;
	pirlData.ebsLimitAction		= ESB_LIMIT_ACTION_DROP;
	pirlData.bktRateType		= BUCKET_TYPE_TRAFFIC_BASED;
	pirlData.bktTypeMask		= BUCKET_TRAFFIC_BROADCAST |
								  BUCKET_TRAFFIC_MULTICAST |
								  BUCKET_TRAFFIC_UNICAST   |
								  BUCKET_TRAFFIC_MGMT_FRAME|
								  BUCKET_TRAFFIC_ARP;

	pirlData.byteTobeCounted	= GT_PIRL_COUNT_ALL_LAYER3;

	status = gpirlActivate(dev,irlUnit,(1<<port),&pirlData);

	switch (status)
	{
		case GT_OK:
			MSG_PRINT(("IRL Unit 0 is activated.\n"));
			break;
		case GT_BAD_PARAM:
			MSG_PRINT(("Invalid parameters are given.\n"));
			break;
		case GT_NOT_SUPPORTED:
			MSG_PRINT(("Device is not supporting PIRL.\n"));
			break;
		default:
			MSG_PRINT(("Failure to configure device.\n"));
			break;
	}

	return status;
}

