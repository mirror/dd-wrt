#include <Copyright.h>
/********************************************************************************
* qos.c
*
* DESCRIPTION:
*       Sample program which will show how to setup the Priority Queue for QoS
*		
*
* DEPENDENCIES:   None.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/
#include "msSample.h"

/*
 *	sampleQoS will enable using both IEEE 802.3ac Tag and IPv4/IPv6 Traffic 
 *	Class field and IEEE 802.3ac has a higher priority than IPv4/IPv6. 
 *	The following is the QoS mapping programmed by sampleQos:
 *	1) IEEE 802.3ac Tag (Priority 0 ~ 7, 3 bits)
 *		Priority 1~3 is using QuarterDeck Queue 0.
 *		Priority 0,4 is using QuarterDeck Queue 1.
 *		Priority 6,7 is using QuarterDeck Queue 2.
 *		Priority 5 is using QuarterDeck Queue 3.
 *	2) IPv4/IPv6 (Priority 0 ~ 63, 6 bits)
 *		Priority 0~7 is using QuaterDeck Queue 0.
 *		Priority 8~31 is using QuaterDeck Queue 1.
 *		Priority 32~55 is using QuaterDeck Queue 2.
 *		Priority 56~63 is using QuaterDeck Queue 3.
 *	3) Each port's default priority is set to 1.
*/
GT_STATUS sampleQos(GT_QD_DEV *dev)
{
	GT_STATUS status;
	GT_U8 priority;
	GT_LPORT port;

	for(port=0; port<7; port++)
	{
		/*
		 *  Use IEEE Tag
		 */
		if((status = gqosUserPrioMapEn(dev,port,GT_TRUE)) != GT_OK)
		{
			MSG_PRINT(("gqosUserPrioMapEn return Failed\n"));
			return status;
		}

		/*
		 *  Use IPv4/IPv6 priority fields (use IP)
		 */
		if((status = gqosIpPrioMapEn(dev,port,GT_TRUE)) != GT_OK)
		{
			MSG_PRINT(("gqosIpPrioMapEn return Failed\n"));
			return status;
		}

		/*
		 *  IEEE Tag has higher priority than IP priority fields
		 */
		if((status = gqosSetPrioMapRule(dev,port,GT_TRUE)) != GT_OK)
		{
			MSG_PRINT(("gqosSetPrioMapRule return Failed\n"));
			return status;
		}
	}

	/*
	 *	IEEE 802.3ac Tag (Priority 0 ~ 7, 3 bits)
	 *	Priority 1~3 is using QuarterDeck Queue 0.
	 *	Priority 0,4 is using QuarterDeck Queue 1.
	 *	Priority 6,7 is using QuarterDeck Queue 2.
	 *	Priority 5 is using QuarterDeck Queue 3.
	*/

	/*	Priority 0 is using QuarterDeck Queue 1. */
	if((status = gcosSetUserPrio2Tc(dev,0,1)) != GT_OK)
	{
		MSG_PRINT(("gcosSetUserPrio2Tc returned fail.\n"));
		return status;
	}

	/*	Priority 1 is using QuarterDeck Queue 0. */
	if((status = gcosSetUserPrio2Tc(dev,1,0)) != GT_OK)
	{
		MSG_PRINT(("gcosSetUserPrio2Tc returned fail.\n"));
		return status;
	}

	/*	Priority 2 is using QuarterDeck Queue 0. */
	if((status = gcosSetUserPrio2Tc(dev,2,0)) != GT_OK)
	{
		MSG_PRINT(("gcosSetUserPrio2Tc returned fail.\n"));
		return status;
	}

	/*	Priority 3 is using QuarterDeck Queue 0. */
	if((status = gcosSetUserPrio2Tc(dev,3,0)) != GT_OK)
	{
		MSG_PRINT(("gcosSetUserPrio2Tc returned fail.\n"));
		return status;
	}

	/*	Priority 4 is using QuarterDeck Queue 1. */
	if((status = gcosSetUserPrio2Tc(dev,4,1)) != GT_OK)
	{
		MSG_PRINT(("gcosSetUserPrio2Tc returned fail.\n"));
		return status;
	}

	/*	Priority 5 is using QuarterDeck Queue 3. */
	if((status = gcosSetUserPrio2Tc(dev,5,3)) != GT_OK)
	{
		MSG_PRINT(("gcosSetUserPrio2Tc returned fail.\n"));
		return status;
	}

	/*	Priority 6 is using QuarterDeck Queue 2. */
	if((status = gcosSetUserPrio2Tc(dev,6,2)) != GT_OK)
	{
		MSG_PRINT(("gcosSetUserPrio2Tc returned fail.\n"));
		return status;
	}

	/*	Priority 7 is using QuarterDeck Queue 2. */
	if((status = gcosSetUserPrio2Tc(dev,7,2)) != GT_OK)
	{
		MSG_PRINT(("gcosSetUserPrio2Tc returned fail.\n"));
		return status;
	}


	/*
	 *	IPv4/IPv6 (Priority 0 ~ 63, 6 bits)
	 *	Priority 0~7 is using QuaterDeck Queue 0.
	 *	Priority 8~31 is using QuaterDeck Queue 1.
	 *	Priority 32~55 is using QuaterDeck Queue 2.
	 *	Priority 56~63 is using QuaterDeck Queue 3.
	*/

	/*	Priority 0~7 is using QuaterDeck Queue 0. */
	for(priority=0; priority<8; priority++)
	{
		if((status = gcosSetDscp2Tc(dev,priority,0)) != GT_OK)
		{
			MSG_PRINT(("gcosSetDscp2Tc returned fail.\n"));
			return status;
		}
	}

	/*	Priority 8~31 is using QuaterDeck Queue 1. */
	for(priority=8; priority<32; priority++)
	{
		if((status = gcosSetDscp2Tc(dev,priority,1)) != GT_OK)
		{
			MSG_PRINT(("gcosSetDscp2Tc returned fail.\n"));
			return status;
		}
	}

	/*	Priority 32~55 is using QuaterDeck Queue 2. */
	for(priority=32; priority<56; priority++)
	{
		if((status = gcosSetDscp2Tc(dev,priority,2)) != GT_OK)
		{
			MSG_PRINT(("gcosSetDscp2Tc returned fail.\n"));
			return status;
		}
	}

	/*	Priority 56~63 is using QuaterDeck Queue 3. */
	for(priority=56; priority<64; priority++)
	{
		if((status = gcosSetDscp2Tc(dev,priority,3)) != GT_OK)
		{
			MSG_PRINT(("gcosSetDscp2Tc returned fail.\n"));
			return status;
		}
	}

	/*
	 * Each port's default priority is set to 1.
	*/
	for(port=0; port<7; port++)
	{
		if((status = gcosSetPortDefaultTc(dev,port,1)) != GT_OK)
		{
			MSG_PRINT(("gcosSetDscp2Tc returned fail.\n"));
			return status;
		}
	}

	return GT_OK;
}


