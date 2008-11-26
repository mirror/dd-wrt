#include <Copyright.h>
/********************************************************************************
* pirl2.c
*
* DESCRIPTION:
*       Setup PIRL buckets for 88E6097 device family
*
* DEPENDENCIES:   None.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/
#include "msSample.h"


/*
 *  This setup function configures the resource 0 of Port 0 of Marvell SOHO 
 *	Switch Device with capability of PIRL to be :
 *	
 *	1) Ingress Rate    : 128Kbps (128K bit per sec)
 *	2) Custom setup for Ingress Rate : disabled
 *	3) Discarded frame : Do not account discarded frame due to queue congestion
 *	4) Filtered frame  : Account filtered frame
 *	5) Mgmt frame      : Exclude management frame from rate limiting calculation
 *	6) SA found in ATU : Exclude from ingress rate limiting calculation if the SA of the
 *						 frame is in ATU with EntryState that indicates Non Rate Limited.
 *	7) DA found in ATU : Include to ingress rate limiting calculation even though the DA of the
 *						 frame is in ATU with EntryState that indicates Non Rate Limited.
 *	8) Sampling Mode   : Disable the mode
 *	9) Action Mode     : Follow Limit action when there are not enough tokens to accept the
 *						 entire imcoming frame.
 *	10) Limit action   : Drop packets when the incoming rate exceeds the limit
 *	11) Rate type      : Rate is based on Traffic type
 *	12) Traffic type   : ARP, MGMT, Multicast, Broadcast, and Unicast frames are 
 *					  	 tracked as part of the rate resource calculation.
 *	13) Byte counted   : Account only Layer 3 bytes (IP header and payload)
 *
*/

GT_STATUS samplePIRL2Setup(GT_QD_DEV *dev)
{
	GT_STATUS status;
	GT_PIRL2_DATA pirlData;
	GT_U32		irlRes;
	GT_LPORT 	port;

	/* change Current Timer Update Interval */
	status = gpirl2SetCurTimeUpInt(dev,4);	
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

	port = 0;
	irlRes = 0;

	pirlData.ingressRate 		= 128;

	pirlData.customSetup.isValid = GT_FALSE;

	pirlData.accountQConf 		= GT_FALSE;
	pirlData.accountFiltered	= GT_TRUE;

	pirlData.mgmtNrlEn = GT_TRUE;
	pirlData.saNrlEn   = GT_TRUE;
	pirlData.daNrlEn   = GT_FALSE;
	pirlData.samplingMode = GT_FALSE;
	pirlData.actionMode = PIRL_ACTION_USE_LIMIT_ACTION;

	pirlData.ebsLimitAction		= ESB_LIMIT_ACTION_DROP;
	pirlData.bktRateType		= BUCKET_TYPE_TRAFFIC_BASED;
	pirlData.bktTypeMask		= BUCKET_TRAFFIC_BROADCAST |
								  BUCKET_TRAFFIC_MULTICAST |
								  BUCKET_TRAFFIC_UNICAST   |
								  BUCKET_TRAFFIC_MGMT_FRAME|
								  BUCKET_TRAFFIC_ARP;

	pirlData.priORpt = GT_TRUE;
	pirlData.priMask = 0;

	pirlData.byteTobeCounted	= GT_PIRL2_COUNT_ALL_LAYER3;

	status = gpirl2WriteResource(dev,port,irlRes,&pirlData);

	switch (status)
	{
		case GT_OK:
			MSG_PRINT(("PIRL2 writing completed.\n"));
			break;
		case GT_BAD_PARAM:
			MSG_PRINT(("Invalid parameters are given.\n"));
			break;
		case GT_NOT_SUPPORTED:
			MSG_PRINT(("Device is not supporting PIRL2.\n"));
			break;
		default:
			MSG_PRINT(("Failure to configure device.\n"));
			break;
	}

	return status;
}



/*
 *	This setup function shows how to configure Ingress Rate of 128Kbps with the
 *	custom data information.
 *  it configures the resource 0 of Port 0 of Marvell SOHO Switch Device with 
 *	capability of PIRL to be :
 *	
 *	1) Custom setup for Ingress Rate : Enabled
 *	2) Custom EBS Limit : 0xFFFFFF
 *	3) Custom CBS Limit : 0x200000
 *	4) Custom Bucket Increament  : 0x3D
 *	5) Custom Bucket Rate Factor : 2
 *	6) Discarded frame : Do not account discarded frame due to queue congestion
 *	7) Filtered frame  : Account filtered frame
 *	8) Mgmt frame      : Exclude management frame from rate limiting calculation
 *	9) SA found in ATU : Exclude from ingress rate limiting calculation if the SA of the
 *						 frame is in ATU with EntryState that indicates Non Rate Limited.
 *	10) DA found in ATU : Include to ingress rate limiting calculation even though the DA of the
 *						 frame is in ATU with EntryState that indicates Non Rate Limited.
 *	11) Sampling Mode   : Disable the mode
 *	12) Action Mode     : Follow Limit action when there are not enough tokens to accept the
 *						 entire imcoming frame.
 *	13) Limit action   : Drop packets when the incoming rate exceeds the limit
 *	14) Rate type      : Rate is based on Traffic type
 *	15) Traffic type   : ARP, MGMT, Multicast, Broadcast, and Unicast frames are 
 *					  	 tracked as part of the rate resource calculation.
 *	16) Byte counted   : Account only Layer 3 bytes (IP header and payload)
 *
*/

GT_STATUS samplePIRL2CustomSetup(GT_QD_DEV *dev)
{
	GT_STATUS status;
	GT_PIRL2_DATA pirlData;
	GT_U32		irlRes;
	GT_LPORT 	port;

	port = 0;
	irlRes = 0;

	pirlData.customSetup.isValid = GT_TRUE;
	pirlData.customSetup.ebsLimit = 0xFFFFFF;
	pirlData.customSetup.cbsLimit = 0x200000;
	pirlData.customSetup.bktIncrement = 0x3D;
	pirlData.customSetup.bktRateFactor = 2;

	pirlData.accountQConf 		= GT_FALSE;
	pirlData.accountFiltered	= GT_TRUE;

	pirlData.mgmtNrlEn = GT_TRUE;
	pirlData.saNrlEn   = GT_TRUE;
	pirlData.daNrlEn   = GT_FALSE;
	pirlData.samplingMode = GT_FALSE;
	pirlData.actionMode = PIRL_ACTION_USE_LIMIT_ACTION;

	pirlData.ebsLimitAction		= ESB_LIMIT_ACTION_DROP;
	pirlData.bktRateType		= BUCKET_TYPE_TRAFFIC_BASED;
	pirlData.bktTypeMask		= BUCKET_TRAFFIC_BROADCAST |
								  BUCKET_TRAFFIC_MULTICAST |
								  BUCKET_TRAFFIC_UNICAST   |
								  BUCKET_TRAFFIC_MGMT_FRAME|
								  BUCKET_TRAFFIC_ARP;

	pirlData.priORpt = GT_TRUE;
	pirlData.priMask = 0;

	pirlData.byteTobeCounted	= GT_PIRL2_COUNT_ALL_LAYER3;

	status = gpirl2WriteResource(dev,port,irlRes,&pirlData);

	switch (status)
	{
		case GT_OK:
			MSG_PRINT(("PIRL2 writing completed.\n"));
			break;
		case GT_BAD_PARAM:
			MSG_PRINT(("Invalid parameters are given.\n"));
			break;
		case GT_NOT_SUPPORTED:
			MSG_PRINT(("Device is not supporting PIRL2.\n"));
			break;
		default:
			MSG_PRINT(("Failure to configure device.\n"));
			break;
	}

	return status;
}


