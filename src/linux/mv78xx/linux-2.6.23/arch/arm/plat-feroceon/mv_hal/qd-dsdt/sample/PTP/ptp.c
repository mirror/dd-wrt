#include <Copyright.h>
/********************************************************************************
* ptp.c
*
* DESCRIPTION:
*       Setup PTP for 88E6165 device family.
*
* DEPENDENCIES:   None.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/
#include "msSample.h"


/*
 *  PTP Init routine
 *	
 *	1) Setup each port to forward PTP frame to CPU port
 *	2) Enable PTP Interrupt (assumes that no other interrupt is used, but PTP)
 *	3) Configure PTP
 *	4) Enable PTP
 *
 *	Notes: This sample uses the following configuration
 *	1) Enables only PTP interrupt
 *	2) Assumes PTP Ethernet Type is 0x88F7
 *	3) Time Stamp is enabled only for Message ID 0, 2, and 3
 *	4) Message ID 0 and 2 use Arr0 pointer and ID 3 uses Arr1 pointer
 *	5) PTP interrtups are enabled on Port 0 ~ 5
 *
 *	Notes: Forwarding PTP fram to CPU port is based on Ether Type DSA Tag (8 bytes).
 *	Therefore, Ethernet device driver, that actually rx/tx the PTP frame,
 *	should expect/insert Ether Type DSA Tag.
*/

STATUS samplePTPInit(GT_QD_DEV *dev)
{
 	GT_PTP_CONFIG ptpCfg;
	GT_LPORT port;
	GT_STATUS status;


	/*
	 *	1) Setup each port to forward PTP frame to CPU port
	*/

	/* setup EtypeType and Policy */
	for(port=0; port<dev->numOfPorts; port++)
	{
		if ((status = gprtSetPortEType(dev, port, (GT_ETYPE)0x88F7)) != GT_OK)
		{
			MSG_PRINT(("gprtSetPortEType returned not OK\n"));
			return status;
		}

		if (port == dev->cpuPortNum)
			continue;

		if ((status = gprtSetPolicy(dev, port, POLICY_TYPE_ETYPE, FRAME_POLICY_TRAP)) != GT_OK)
		{
			MSG_PRINT(("gprtSetPolicy returned not OK\n"));
			return status;
		}
	}

	/* setup Frame Mode for CPU port */
	if ((status = gprtSetFrameMode(dev, dev->cpuPortNum, GT_FRAME_MODE_ETHER_TYPE_DSA)) != GT_OK)
	{
		MSG_PRINT(("gprtSetFrameMode return failed\n"));
		return status;
	}

	/*
	 *	2) Enable PTP Interrupt
	*/
	eventSetActive(dev, GT_AVB_INT);


	/*
	 *	3) Configure PTP
	*/
	ptpCfg.ptpEType = 0x88F7;
	ptpCfg.msgIdTSEn = 0xd;		/* id 0, 2, and 3 */
	ptpCfg.tsArrPtr = 0x8;		/* id 0 and 2 for ARR0, id 3 for ARR1 */

	/* Transport specific bits present in PTP Common Header */
	ptpCfg.transSpec = 1;		

	/* starting bit location for the Message ID field in the PTP Common Header */
	ptpCfg.msgIdStartBit = 4;	

	ptpCfg.ptpArrIntEn = 0x3F;
	ptpCfg.ptpDepIntEn = 0x3F;
	ptpCfg.disTSOverwrite = 0;


	if ((status = gptpSetConfig(dev, &ptpCfg)) != GT_OK)
	{
		MSG_PRINT(("gptpSetConfig return failed\n"));
		return status;
	}
	if ((status = gptpSetPTPEn(dev, GT_TRUE)) != GT_OK)
	{
		MSG_PRINT(("gptpSetPTPEn return failed\n"));
		return status;
	}

	return GT_OK;
}


/*
 *  PTP Interrupt Handler
 *	
 *	1) for each port that causes PTP interrup, do the followings
 *	2) check Arrival 0 Time Stamp
 *	3) check Arrival 1 Time Stamp
 *	4) check Departure Time Stamp
*/

STATUS samplePTPIntHandler(GT_QD_DEV *dev)
{
	GT_U32 int_ports, i, int_status;
	GT_STATUS status;
	GT_PTP_TS_STATUS	ptpStatus;

	/* disable AVB Interrupt */
	eventSetActive(dev, 0);

	/* read interrupt cause */
	eventGetIntStatus(dev,(GT_U16*)&int_status);
	if ((int_status & GT_AVB_INT) == 0)
	{
		/* it's not PTP interrupt */
		return GT_FAIL;	
	}

	/* read AVB Int status */
	if((status = gptpGetPTPInt(dev, &int_ports)) != GT_OK)
	{
	    MSG_PRINT(("gptpGetPTPInt return failed\n"));
		goto ret_int;
	}

	/* for each port, get the timestamp information if necessary */
	i = 0;
	while(int_ports)
	{
		if(!(int_ports & 0x1))
		{
			i++;
			int_ports >>= 1;
			continue;
		}

		/* check Arrival0 Time Stamp */
		if((status = gptpGetTimeStamped(dev, i, PTP_ARR0_TIME, &ptpStatus)) != GT_OK)
		{
		    MSG_PRINT(("gptpGetTimeStamped return failed\n"));
			goto ret_int;
		}

		if (ptpStatus.isValid == GT_TRUE)
		{
			switch(ptpStatus.status)
			{
				case PTP_INT_NORMAL:
					/* To Do: No error condition occurred. So store the time stamp and seqId */
					break;

				case PTP_INT_OVERWRITE:
					/* To Do: PTP Logic received several PTP frames and only the last one is valid */
					break;

				case PTP_INT_DROP:
					/* To Do: PTP Logic received several PTP frames and only the first one is valid */
					break;

				default:
				    MSG_PRINT(("unknown ptp status %i\n", ptpStatus.status));
					status = GT_FAIL;
					goto ret_int;

			}

			if((status = gptpResetTimeStamp(dev, i, PTP_ARR0_TIME)) != GT_OK)
			{
			    MSG_PRINT(("gptpResetTimeStamp return failed\n"));
				goto ret_int;
			}
		}
		
		/* check Arrival1 Time Stamp */
		if((status = gptpGetTimeStamped(dev, i, PTP_ARR1_TIME, &ptpStatus)) != GT_OK)
		{
		    MSG_PRINT(("gptpGetTimeStamped return failed\n"));
			goto ret_int;
		}

		if (ptpStatus.isValid == GT_TRUE)
		{
			switch(ptpStatus.status)
			{
				case PTP_INT_NORMAL:
					/* To Do: No error condition occurred. So store the time stamp and seqId */
					break;

				case PTP_INT_OVERWRITE:
					/* To Do: PTP Logic received several PTP frames and only the last one is valid */
					break;

				case PTP_INT_DROP:
					/* To Do: PTP Logic received several PTP frames and only the first one is valid */
					break;

				default:
				    MSG_PRINT(("unknown ptp status %i\n", ptpStatus.status));
					status = GT_FAIL;
					goto ret_int;
			}

			if((status = gptpResetTimeStamp(dev, i, PTP_ARR1_TIME)) != GT_OK)
			{
			    MSG_PRINT(("gptpResetTimeStamp return failed\n"));
				goto ret_int;
			}

		}
		
		/* check Departure Time Stamp */
		if((status = gptpGetTimeStamped(dev, i, PTP_DEP_TIME, &ptpStatus)) != GT_OK)
		{
		    MSG_PRINT(("gptpGetTimeStamped return failed\n"));
			goto ret_int;
		}

		if (ptpStatus.isValid == GT_TRUE)
		{
			switch(ptpStatus.status)
			{
				case PTP_INT_NORMAL:
					/* To Do: No error condition occurred. So store the time stamp and seqId */
					break;

				case PTP_INT_OVERWRITE:
					/* To Do: PTP Logic received several PTP frames and only the last one is valid */
					break;

				case PTP_INT_DROP:
					/* To Do: PTP Logic received several PTP frames and only the first one is valid */
					break;

				default:
				    MSG_PRINT(("unknown ptp status %i\n", ptpStatus.status));
					status = GT_FAIL;
					goto ret_int;
			}

			if((status = gptpResetTimeStamp(dev, i, PTP_DEP_TIME)) != GT_OK)
			{
			    MSG_PRINT(("gptpResetTimeStamp return failed\n"));
				goto ret_int;
			}

		}
		
		int_ports >>= 1;
					
	}

ret_int:
	eventSetActive(dev, GT_AVB_INT);

	return status;
}

