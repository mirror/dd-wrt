/**
 * @file IxTimeSyncAccCodelet.c
 *
 * @author Intel Corporation
 *
 * @date 21 December 2004
 *
 * @brief  Codelet for IXP46X Time Sync Access Component.
 *
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
 */

#if defined(__ixp46X)

/********************************************************************* 
 *	user include file 
 *********************************************************************/

#include "IxTimeSyncAccCodelet.h"		    

/*********************************************************************
 *	PRIVATE function prototype
 *********************************************************************/
PRIVATE void ixTimeSyncAccCodeletQuit (void);
PRIVATE IX_STATUS ixTimeSyncAccCodeletEthInit (void);
PRIVATE void ixTimeSyncAccCodeletPortDisable (void);
PRIVATE IX_OSAL_MBUF *ixTimeSyncAccCodeletMbufAllocate (void); 
PRIVATE void ixTimeSyncAccCodeletMbufsFree (void);
PRIVATE IX_STATUS ixTimeSyncAccCodeletDispatcherStart (void);
PRIVATE void ixTimeSyncAccCodeletDispatcherStop (void);
PRIVATE void ixTimeSyncAccCodeletDummyCB (
	UINT32 cbTag, 
	IX_OSAL_MBUF* mBufPtr, 
	UINT32 reserved);
PRIVATE void ixTimeSyncAccCodeletTxDoneCB (
	UINT32 cbTag, 
	IX_OSAL_MBUF* mBufPtr);
PRIVATE IX_STATUS ixTimeSyncAccCodeletPortConfigure (IxEthAccPortId portId);
PRIVATE IX_STATUS ixTimeSyncAccCodeletNewThreadCreate (
	IxOsalVoidFnPtr func, 
	char *label);
PRIVATE void ixTimeSyncAccCodeletPTPMsgBuild (UINT8 control);
PRIVATE void ixTimeSyncAccCodeletPTPMsgTransmit (void);
PRIVATE void ixTimeSyncAccCodeletPTPMsgTransmitStop (void);
PRIVATE void ixTimeSyncAccCodeletPTPMsgShow (
	BOOL receive,
	IxTimeSyncAcc1588PTPPort channel,
	IxTimeSyncAccPtpMsgData *ptpMsgData		
	);
PUBLIC IX_STATUS ixTimeSyncAccCodeletPTPMsgCheck (void);
PRIVATE void ixTimeSyncAccCodeletTargetTimeHitCallback (void);
PRIVATE IX_STATUS ixTimeSyncAccCodeletTargetTimeSet (void);
PRIVATE IX_STATUS ixTimeSyncAccCodeletSystemTimeClear (void);
PRIVATE void ixTimeSyncAccCodeletTargetTimeClear (void);

/*********************************************************************
 *	PRIVATE variables
 *********************************************************************/
PRIVATE IxOsalSemaphore ixTimeSyncAccCodeletSemId;

PRIVATE char *ixTimeSyncAccCodeletTSChannelLabel[] =
{
	"NPE-A",	/* channel 0 */
	"NPE-B",	/* channel 1 */
	"NPE-C",	/* channel 2 */	
	"INVALID"	/* invalid channel */
};

PRIVATE char *ixTimeSyncAccCodeletTSChannelModeLabel[] =
{
	"MASTER",	/* master mode */
	"SLAVE",	/* slave mode */
	"ANY MODE",	/* any mode */
	"INVALID"	/* invalid mode */
};

PRIVATE char *ixTimeSyncAccCodeletPTPMessageLabel[] =
{
	"SYNC",		/* PTP Sync message */
	"DELAY_REQ",	/* PTP Delay_Req message */
	"UNKNOWN"	/* unknown message */
};

/* 
 * PTP message type list. 
 * Note: The entry of this array is accessed by using Time Sync Channel mode 
 *	 as entry index.
 * 	 e.g. Master mode -> value = 0 -> message type for index 0 -> Sync
 *	      Slave mode -> value = 1 -> message type for index 1 -> Delay_Req
 */
PRIVATE IxTimeSyncAcc1588PTPMsgType ixTimeSyncAccCodeletPTPMsgTypeList[] =
{ 
	IX_TIMESYNCACC_1588PTP_MSGTYPE_SYNC,	/* PTP Sync message */
	IX_TIMESYNCACC_1588PTP_MSGTYPE_DELAYREQ	/* PTP Delay_Req message */
};

/* port ID list */
PRIVATE IxEthAccPortId ixTimeSyncAccCodeletPortIdList[] =
{
	IX_ETH_PORT_3,	/* NPE A's port ID (channel 0) */
	IX_ETH_PORT_1,	/* NPE B's port ID (channel 1) */
	IX_ETH_PORT_2	/* NPE C's port ID (channel 2) */	
};

/* pre-defined configuration table */ 
PRIVATE IxTimeSyncAccCodeletTSChannelConfig ixTimeSyncAccCodeletConfigList[IX_TIMESYNCACC_CODELET_MAX_CONFIGURATIONS] = 
{
	/* configuration 0: NPE A - Master, NPE B - Master, NPE C - Slave */
	{	
        	{IX_TIMESYNCACC_1588PTP_PORT_SLAVE, 	/* NPE A */
        	 IX_TIMESYNCACC_1588PTP_PORT_SLAVE, 	/* NPE B */
		 IX_TIMESYNCACC_1588PTP_PORT_MASTER}  	/* NPE C */
	},
	/* configuration 1: NPE A - Master, NPE B - Slave, NPE C - Master */
	{	
        	{IX_TIMESYNCACC_1588PTP_PORT_SLAVE, 	/* NPE A */
        	 IX_TIMESYNCACC_1588PTP_PORT_MASTER, 	/* NPE B */
		 IX_TIMESYNCACC_1588PTP_PORT_SLAVE}	/* NPE C */  
	},
	/* configuration 2 : NPE A - Slave, NPE B - Master, NPE C - Master */
	{	
		{IX_TIMESYNCACC_1588PTP_PORT_MASTER,  	/* NPE A */
        	 IX_TIMESYNCACC_1588PTP_PORT_SLAVE, 	/* NPE B */
        	 IX_TIMESYNCACC_1588PTP_PORT_SLAVE}	/* NPE C */ 
	}
};

/* global configuration pointer */
PRIVATE IxTimeSyncAccCodeletTSChannelConfig *ixTimeSyncAccCodeletConfigPtr;

/* array of mBuf pointers, index of array represents ethernet port number */
PRIVATE IX_OSAL_MBUF *ixTimeSyncAccCodeletGlobalMBuf[IX_TIMESYNCACC_CODELET_MAX_TS_CHANNELS];

/* buffer for PTP message */
PRIVATE UINT8 ixTimeSyncAccCodeletPtpMsgData[IX_TIMESYNCACC_CODELET_UDP_FRAME_LEN];

/* function pointer for Q MGR dispatcher */
PRIVATE IxQMgrDispatcherFuncPtr ixTimeSyncAccCodeletDispatcherFunc;

/* function map for each module's unload function */
PRIVATE IxTimeSyncAccCodeletUninitFuncMap ixTimeSyncAccCodeletUninitFuncMap[] =
{
	{(IxOsalVoidFnPtr)ixTimeSyncAccCodeletMbufsFree, IX_TIMESYNCACC_CODELET_INVALID_PARAM, FALSE},
	{(IxOsalVoidFnPtr)ixQMgrUnload, IX_TIMESYNCACC_CODELET_INVALID_PARAM, FALSE},
	{(IxOsalVoidFnPtr)ixTimeSyncAccCodeletDispatcherStop, IX_TIMESYNCACC_CODELET_INVALID_PARAM, FALSE},
	{(IxOsalVoidFnPtr)ixNpeMhUnload, IX_TIMESYNCACC_CODELET_INVALID_PARAM, FALSE}, 
	{(IxOsalVoidFnPtr)ixNpeDlUnload, IX_TIMESYNCACC_CODELET_INVALID_PARAM, FALSE},
	{(IxOsalVoidFnPtr)ixNpeDlNpeStopAndReset, IX_NPEDL_NPEID_NPEC, FALSE},
	{(IxOsalVoidFnPtr)ixNpeDlNpeStopAndReset, IX_NPEDL_NPEID_NPEB, FALSE},
	{(IxOsalVoidFnPtr)ixNpeDlNpeStopAndReset, IX_NPEDL_NPEID_NPEA, FALSE},
	{(IxOsalVoidFnPtr)ixEthAccUnload, IX_TIMESYNCACC_CODELET_INVALID_PARAM, FALSE},
	{(IxOsalVoidFnPtr)ixTimeSyncAccCodeletPortDisable, IX_TIMESYNCACC_CODELET_INVALID_PARAM, FALSE},
	{(IxOsalVoidFnPtr)ixTimeSyncAccCodeletPTPMsgTransmitStop, IX_TIMESYNCACC_CODELET_INVALID_PARAM, FALSE}
};

/* MAC addresses for each NPE */
PRIVATE IxEthAccMacAddr ixTimeSyncAccCodeletNpeMacAddr[] = 
{
	{{0x2, 0x0, 0xa, 0xb, 0xc, 0xd}},
	{{0x2, 0x0, 0xe, 0xf, 0xa, 0xb}},
	{{0x2, 0x0, 0x6, 0x7, 0x8, 0x9}}
};

/* default PTP domain IP address */
PRIVATE UINT8 ixTimeSyncAccCodeletPTPMulticastAddress[] = {224, 0, 1, 129};

/* codelet termination flag */
PRIVATE BOOL ixTimeSyncAccCodeletTerminate = TRUE;

/* PTP message transmission halt flag */
PRIVATE BOOL ixTimeSyncAccCodeletTxHalt;

/*********************************************************************
 *	PUBLIC functions 
 *********************************************************************/

PUBLIC IX_STATUS
ixTimeSyncAccCodeletMain (UINT32 configIndex)
{
	IxTimeSyncAccStatus tsStatus;
	IxTimeSyncAcc1588PTPPort tsChannel;
	IxTimeSyncAcc1588PTPPortMode tsChannelMode;

	/* check the validity of configuration */
	if (IX_TIMESYNCACC_CODELET_MAX_CONFIGURATIONS <= configIndex)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletMain: invalid configuration\n",
			0, 0, 0, 0, 0, 0);
		ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixTimeSyncAccCodeletMain: Usage :\n", 
			0, 0, 0, 0, 0, 0);
		ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixTimeSyncAccCodeletMain:\t-> ixTimeSyncAccCodeletMain <x>\n\n",
			0, 0, 0, 0, 0, 0);
		ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixTimeSyncAccCodeletMain:\twhere <x> =\n",
			0, 0, 0, 0, 0, 0);
		ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixTimeSyncAccCodeletMain:\t0 -> NPE A - Slave,  NPE B - Slave,  NPE C - Master (default)\n",
			0, 0, 0, 0, 0, 0);  
		ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixTimeSyncAccCodeletMain:\t1 -> NPE A - Slave,  NPE B - Master, NPE C - Slave\n",
			0, 0, 0, 0, 0, 0); 
		ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixTimeSyncAccCodeletMain:\t2 -> NPE A - Master, NPE B - Slave,  NPE C - Slave\n",
			0, 0, 0, 0, 0, 0);  

		return IX_FAIL;
	}

    	/* Disable UTOPIA to enable Ethernet on Npe-A */
    	ixFeatureCtrlWrite (ixFeatureCtrlRead() | ((UINT32)1<<IX_FEATURECTRL_UTOPIA));

	/* set termination flag to false */
	ixTimeSyncAccCodeletTerminate = FALSE;

	/* save global configuration pointer */
	ixTimeSyncAccCodeletConfigPtr = &ixTimeSyncAccCodeletConfigList[configIndex];

	/* write default frequency scale value to Addend Register */
	tsStatus = ixTimeSyncAccTickRateSet (IX_TIMESYNCACC_CODELET_FSV_DEFAULT);
	if (IX_TIMESYNCACC_SUCCESS != tsStatus)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletMain: failed to set frequency scale value, error code %d\n",
			tsStatus, 0, 0, 0, 0, 0);

		return IX_FAIL;
	}
	
	/* configure all Time Sync channels */
	for (tsChannel = IX_TIMESYNCACC_NPE_A_1588PTP_PORT; 
	     tsChannel < IX_TIMESYNCACC_CODELET_MAX_TS_CHANNELS; 
	     tsChannel++)
	{
		/* get channel operating mode from the configuration table */
		tsChannelMode = ixTimeSyncAccCodeletConfigPtr->tsChannelMode[tsChannel];

		tsStatus = ixTimeSyncAccPTPPortConfigSet(tsChannel, tsChannelMode);
		if (IX_TIMESYNCACC_SUCCESS != tsStatus)
		{
			ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletMain: failed to configure %s channel to %s mode, error code %d\n",
				(UINT32)(ixTimeSyncAccCodeletTSChannelLabel[tsChannel]),
				(UINT32)(ixTimeSyncAccCodeletTSChannelModeLabel[tsChannelMode]), 
				tsStatus, 0, 0, 0);
		
			return IX_FAIL;
		}
	}

	ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "\nixTimeSyncAccCodeletMain: TS channel configuration: NPE A - %s, NPE B - %s, NPE C - %s\n\n",
		(UINT32)(ixTimeSyncAccCodeletTSChannelModeLabel[ixTimeSyncAccCodeletConfigPtr->tsChannelMode[0]]),
		(UINT32)(ixTimeSyncAccCodeletTSChannelModeLabel[ixTimeSyncAccCodeletConfigPtr->tsChannelMode[1]]),
		(UINT32)(ixTimeSyncAccCodeletTSChannelModeLabel[ixTimeSyncAccCodeletConfigPtr->tsChannelMode[2]]),
		0, 0, 0);

	/* set target time using default interval */
	if (IX_SUCCESS != ixTimeSyncAccCodeletTargetTimeSet ())
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletMain: failed to set target time\n",
			0, 0, 0, 0, 0, 0);

		return IX_FAIL;
	}

		
	/* initialize ethernet components to transmit PTP messages */
	if (IX_SUCCESS != ixTimeSyncAccCodeletEthInit ())
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletMain: failed to initialize ethernet components\n",
			0, 0, 0, 0, 0, 0);

		/* unload all initialized modules and free all resources */
		ixTimeSyncAccCodeletUninit ();
		return IX_FAIL;
	}

	return IX_SUCCESS;

} /* end of ixTimeSyncAccCodeletMain function */


PUBLIC void
ixTimeSyncAccCodeletUninit ()
{
	int moduleId;
	IxOsalVoidFnPtr func;
	IxTimeSyncAccCodeletUninitFuncPtr func1;

	if (TRUE == ixTimeSyncAccCodeletTerminate)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletUninit: timeSyncAcc codelet is either not loaded or in the process of being terminated\n",
		0, 0, 0, 0, 0, 0);
	
		return;
	}

	/* set termination flag  */
	ixTimeSyncAccCodeletTerminate = TRUE;

	/* unload every supporting modules and free all resources */
	for (moduleId = IX_TIMESYNCACC_CODELET_TX_PTP; 
	     moduleId >= IX_TIMESYNCACC_CODELET_MBUF_ALLOC; 
	     moduleId--)
	{
		if (TRUE == ixTimeSyncAccCodeletUninitFuncMap[moduleId].initialized)
		{
			if (IX_TIMESYNCACC_CODELET_INVALID_PARAM == ixTimeSyncAccCodeletUninitFuncMap[moduleId].funcParameter)
			{
				func = (IxOsalVoidFnPtr)ixTimeSyncAccCodeletUninitFuncMap[moduleId].funcPtr;
				(*func)();
			}
			else
			{
				func1 = (IxTimeSyncAccCodeletUninitFuncPtr)ixTimeSyncAccCodeletUninitFuncMap[moduleId].funcPtr;
				(*func1)(ixTimeSyncAccCodeletUninitFuncMap[moduleId].funcParameter);
			}

			ixTimeSyncAccCodeletUninitFuncMap[moduleId].initialized = FALSE;
		} 
	}

	/* unload ethDB */
	if (IX_ETH_DB_SUCCESS != ixEthDBUnload ()) 
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletUninit: failed to unload ethDB\n",
			0, 0, 0, 0, 0, 0);
	} 

	/* wait for a while for the codelet to disable target time */
	ixOsalSleep (IX_TIMESYNCACC_CODELET_TARGET_TIME_HIT_INTERVAL);
	
	ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixTimeSyncAccCodeletUninit: timeSyncAcc codelet execution was terminated\n",
		0, 0, 0, 0, 0, 0);

	return;
} /* end of ixTimeSyncAccCodeletUninit function */



/*********************************************************************
 *	PRIVATE functions
 *********************************************************************/

PRIVATE void 
ixTimeSyncAccCodeletQuit (void)
{
	/* spawn thread to terminate timeSyncAcc codelet */
	if (IX_SUCCESS != ixTimeSyncAccCodeletNewThreadCreate ((IxOsalVoidFnPtr)ixTimeSyncAccCodeletUninit, "TimeSyncCodelet Thread"))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletQuit: failed to spawn thread to terminate timeSyncAcc codelet\n",
			0, 0, 0, 0, 0, 0);
	}
	
} /* end of ixTimeSyncAccCodeletQuit function */


/**
 * @ingroup IxTimeSyncAccCodelet
 *
 * @fn ixTimeSyncAccCodeletEthInit ()
 *
 * @brief This function allocates resources and initializes every relevant
 *	  component that is needed for PTP message transmission from
 *	  each NPE.
 *
 * @return void
 */
PRIVATE IX_STATUS
ixTimeSyncAccCodeletEthInit ()
{
	UINT32 channel, count;
	IxEthAccPortId portId;
	IxFeatureCtrlComponentType npe;
	IX_OSAL_MBUF *mBufPtr;

	/* check if the device is IXP46X */
	if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP46X != ixFeatureCtrlDeviceRead ())
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletEthInit: this device is not IXP46X\n",
			0, 0, 0, 0, 0, 0);
		return IX_FAIL;
	}

	/* check if all NPEs are enabled */
	for (channel = 0, npe = IX_FEATURECTRL_NPEA; npe <= IX_FEATURECTRL_NPEC; npe++, channel++)
	{
		if (IX_FEATURE_CTRL_COMPONENT_ENABLED != ixFeatureCtrlComponentCheck (npe))
		{
			ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletEthInit: %s is not enabled\n",
				(UINT32) ixTimeSyncAccCodeletTSChannelLabel[channel], 0, 0, 0, 0, 0);
			return IX_FAIL;
		}
	}

	/* allocate mBufs */
	ixTimeSyncAccCodeletUninitFuncMap[IX_TIMESYNCACC_CODELET_MBUF_ALLOC].initialized = TRUE;
	for (count = 0; count < IX_TIMESYNCACC_CODELET_MAX_TS_CHANNELS; count++)
	{
		mBufPtr = ixTimeSyncAccCodeletMbufAllocate ();
		if (NULL != mBufPtr)
		{
			ixTimeSyncAccCodeletGlobalMBuf[count] = mBufPtr;
		}
		else
		{
			ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletEthInit: failed to allocate mBufs\n",
			0, 0, 0, 0, 0, 0);
		
			return IX_FAIL;	
		}
	}

	/* initialize Q Mgr */
	if (IX_SUCCESS != ixQMgrInit ())
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletEthInit: failed to initialize queue manager\n",
			0, 0, 0, 0, 0, 0);

		return IX_FAIL;
	}
	ixTimeSyncAccCodeletUninitFuncMap[IX_TIMESYNCACC_CODELET_Q_MGR].initialized = TRUE;

	/* start Q Mgr dispatcher */
	if (IX_SUCCESS != ixTimeSyncAccCodeletDispatcherStart ())
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletEthInit: failed to start queue manager dispatch loop\n",
			0, 0, 0, 0, 0, 0);

		return IX_FAIL;
	}
	ixTimeSyncAccCodeletUninitFuncMap[IX_TIMESYNCACC_CODELET_DISPATCHER].initialized = TRUE;
	
	/* initialize NPE message handler */
	if (IX_SUCCESS != ixNpeMhInitialize (IX_NPEMH_NPEINTERRUPTS_YES))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletEthInit: failed to initialize NPE Message Handler\n",
			0, 0, 0, 0, 0, 0);

		return IX_FAIL;
	}
	ixTimeSyncAccCodeletUninitFuncMap[IX_TIMESYNCACC_CODELET_NPE_MH].initialized = TRUE;

	/* download NPE A's image with basic Ethernet Rx/Tx and activate NPE A */ 
	if (IX_SUCCESS != ixNpeDlNpeInitAndStart (IX_NPEDL_NPEIMAGE_NPEA_ETH))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletEthInit: failed to initialize and start NPE A\n",
			0, 0, 0, 0, 0, 0);

		return IX_FAIL;
	}
	ixTimeSyncAccCodeletUninitFuncMap[IX_TIMESYNCACC_CODELET_NPE_A].initialized = TRUE;
	ixTimeSyncAccCodeletUninitFuncMap[IX_TIMESYNCACC_CODELET_NPE_DL].initialized = TRUE;

	/* download NPE B's image with basic Ethernet Rx/Tx and activate NPE B */ 
	if (IX_SUCCESS != ixNpeDlNpeInitAndStart (IX_NPEDL_NPEIMAGE_NPEB_ETH))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletEthInit: failed to initialize and start NPE B\n",
			0, 0, 0, 0, 0, 0);
		
		return IX_FAIL;
	}
	ixTimeSyncAccCodeletUninitFuncMap[IX_TIMESYNCACC_CODELET_NPE_B].initialized = TRUE;

	/* download NPE C's image with basic Ethernet Rx/Tx and activate NPE C */ 
	if (IX_SUCCESS != ixNpeDlNpeInitAndStart (IX_NPEDL_NPEIMAGE_NPEC_ETH))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletEthInit: failed to initialize and start NPE C\n",
			0, 0, 0, 0, 0, 0);
		
		return IX_FAIL;
	}
	ixTimeSyncAccCodeletUninitFuncMap[IX_TIMESYNCACC_CODELET_NPE_C].initialized = TRUE;

	/* initialize Ethernet Access component */
	if (IX_ETH_ACC_SUCCESS != ixEthAccInit())
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletEthInit: failed to initialize Ethernet access driver\n",
			0, 0, 0, 0, 0, 0);

		return IX_FAIL;
	}
	ixTimeSyncAccCodeletUninitFuncMap[IX_TIMESYNCACC_CODELET_ETH_ACC].initialized = TRUE;

	/* initialize all ethernet ports */
	for (portId = IX_ETH_PORT_1; portId <= IX_ETH_PORT_3; portId++)
	{
		if (IX_ETH_ACC_SUCCESS != ixEthAccPortInit (portId))
		{
			ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletEthInit: failed to initialize Ethernet port %d\n",
				portId, 0, 0, 0, 0, 0);

			return IX_FAIL;
		}
	}
	ixTimeSyncAccCodeletUninitFuncMap[IX_TIMESYNCACC_CODELET_ETH_PORTS].initialized = TRUE;
	
	/* spawn new thread to transmit PTP message from each NPEs */
	if (IX_SUCCESS != ixTimeSyncAccCodeletNewThreadCreate ((IxOsalVoidFnPtr)ixTimeSyncAccCodeletPTPMsgTransmit, "Tx Thread"))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletEthInit: failed to spawn Tx Thread\n",
			0, 0, 0, 0, 0, 0);

		return IX_FAIL;
	}
	ixTimeSyncAccCodeletUninitFuncMap[IX_TIMESYNCACC_CODELET_TX_PTP].initialized = TRUE;

	return IX_SUCCESS;

} /* end of ixTimeSyncAccCodeletEthInit function */

/**
 * @ingroup IxTimeSyncAccCodelet
 *
 * @fn ixTimeSyncAccCodeletPortDisable ()
 *
 * @brief Disable all ports 
 *
 * @return void
 */

PRIVATE void 
ixTimeSyncAccCodeletPortDisable ()
{	
	IxEthAccPortId portId;
	IxEthAccStatus status;

	for (portId = IX_ETH_PORT_1; portId <= IX_ETH_PORT_3; portId++)
	{
		status = ixEthAccPortDisable (portId);
		if (IX_ETH_ACC_SUCCESS != status)
		{
			ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletPortDisable: failed to disable port %d, error code %d\n", 
				portId, status, 0, 0, 0, 0);
		} 
	}

} /* end of ixTimeSyncAccCodeletPortDisable function */

/**
 * @ingroup IxTimeSyncAccCodelet
 *
 * @fn ixTimeSyncAccCodeletMbufAllocate ()
 *
 * @brief  Allocate memory for mBuf and its associated data buffer
 *
 * @return 
 * 	@li IX_OSAL_MBUF * - successfully allocated memory for mBuf
 *	@li NULL - fail
 */
PRIVATE IX_OSAL_MBUF  
*ixTimeSyncAccCodeletMbufAllocate ()
{
	IX_OSAL_MBUF *mBufPtr;	
	UINT8 *dataPtr;

	/* Allocate cache-aligned memory for mbuf header */
	mBufPtr = (IX_OSAL_MBUF *) IX_OSAL_CACHE_DMA_MALLOC (sizeof (IX_OSAL_MBUF));

	if (NULL == mBufPtr)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletMbufAllocate: failed to allocate memory for mBuf\n",
			0, 0, 0, 0, 0, 0);

		return NULL;
	}

	/* initialize mBuf */
	ixOsalMemSet (mBufPtr, 0, sizeof (IX_OSAL_MBUF));

	/* Allocate cache-aligned memory for mbuf data */
	dataPtr = (UINT8 *) IX_OSAL_CACHE_DMA_MALLOC (IX_TIMESYNCACC_CODELET_UDP_FRAME_LEN);

	if (NULL == dataPtr)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletMbufAllocate: failed to allocate memory for mBuf's data buffer\n",
			0, 0, 0, 0, 0, 0);

		return NULL;
	}

	/* initialize mBuf's data buffer */
	ixOsalMemSet (dataPtr, 0, IX_TIMESYNCACC_CODELET_UDP_FRAME_LEN);

	/* Fill in mbuf header fields */
	IX_OSAL_MBUF_MDATA (mBufPtr) = dataPtr;
	IX_OSAL_MBUF_ALLOCATED_BUFF_DATA (mBufPtr) = (UINT32)dataPtr;

	IX_OSAL_MBUF_MLEN (mBufPtr) = IX_TIMESYNCACC_CODELET_UDP_FRAME_LEN;
	IX_OSAL_MBUF_ALLOCATED_BUFF_LEN (mBufPtr) = IX_TIMESYNCACC_CODELET_UDP_FRAME_LEN;
	IX_OSAL_MBUF_PKT_LEN (mBufPtr) = IX_TIMESYNCACC_CODELET_UDP_FRAME_LEN;

	return mBufPtr;

} /* end of ixTimeSyncAccCodeletMbufAllocate function */


/**
 * @ingroup IxTimeSyncAccCodelet
 *
 * @fn ixTimeSyncAccCodeletMbufsFree ()
 *
 * @brief Free all allocated mBufs and their associated data buffer
 *
 * @return void
 */

PRIVATE void 
ixTimeSyncAccCodeletMbufsFree ()
{
	UINT32 count;
	IX_OSAL_MBUF *mBufPtr;	
	UINT8 *dataPtr;

	/* free mBufs */
	for (count = 0; count < IX_TIMESYNCACC_CODELET_MAX_TS_CHANNELS; count++)
	{
		mBufPtr = ixTimeSyncAccCodeletGlobalMBuf[count];

		if (NULL != mBufPtr)
		{
			dataPtr = (UINT8 *) IX_OSAL_MBUF_MDATA (mBufPtr);	
			if (NULL != dataPtr)
			{
				IX_OSAL_CACHE_DMA_FREE (dataPtr);
			}
			IX_OSAL_CACHE_DMA_FREE (mBufPtr); 

			ixTimeSyncAccCodeletGlobalMBuf[count] = NULL;
		}
	}
	
} /* end of ixTimeSyncAccCodeletMbufsFree function */


/**
 * @ingroup IxTimeSyncAccCodelet
 *
 * @fn ixTimeSyncAccCodeletDispatcherStart ()
 *
 * @brief Hook the QM QLOW dispatcher to the interrupt controller
 *
 * @return  IX_STATUS
 *          @li IX_SUCCESS - all function operations complete successfully 
 *          @li IX_FAIL    - any operation fails
 */

PRIVATE IX_STATUS 
ixTimeSyncAccCodeletDispatcherStart ()
{ 

	/* get dispatcher function pointer, this should be initialized once */
	ixQMgrDispatcherLoopGet(&ixTimeSyncAccCodeletDispatcherFunc);

	if (NULL == ixTimeSyncAccCodeletDispatcherFunc)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletDispatcherStart: failed to get dispatcher function pointer\n",
			0, 0, 0, 0, 0, 0);	

		return IX_FAIL;
	}

	/* Hook the QM QLOW dispatcher to the interrupt controller */ 
	if (IX_SUCCESS != ixOsalIrqBind(IX_OSAL_IXP400_QM1_IRQ_LVL,
					(IxOsalVoidFnVoidPtr)(ixTimeSyncAccCodeletDispatcherFunc),
					(void *)IX_QMGR_QUELOW_GROUP))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletDispatcherStart: failed to hook QM QLOW dispatcher to the interrupt controller\n",
			0, 0, 0, 0, 0, 0);

		return IX_FAIL;
	}

	return (IX_SUCCESS);
} /* end of ixTimeSyncAccCodeletDispatcherStart function */


/**
 * @ingroup IxTimeSyncAccCodelet
 *
 * @fn ixTimeSyncAccCodeletDispatcherStop ()
 *
 * @brief Unbind to QM1 interrupt
 *
 * @return void
 */
PRIVATE void 
ixTimeSyncAccCodeletDispatcherStop ()
{
	if (IX_SUCCESS != ixOsalIrqUnbind (IX_OSAL_IXP400_QM1_IRQ_LVL))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletDispatcherStop: failed to unbind to QM1 interrupt\n",
			0, 0, 0, 0, 0, 0);
	}

} /* end of ixTimeSyncAccCodeletDispatcherStop function */


/**
 * @ingroup IxTimeSyncAccCodelet
 *
 * @fn ixTimeSyncAccCodeletDummyCB (
	UINT32 cbTag, 
	IX_OSAL_MBUF* mBufPtr, 
	UINT32 reserved)
 *
 * @brief Rx callback. TimeSync codelet does not handle receiving PTP message.
 *	  Thus, this function is just a dummy function. Rx callback registration
 *	  is required by ethernet component. 
 *
 * @param 
 * cbTag UINT32 [in] - argument passed to callback 
 *
 * @param 
 * mBufPtr IX_OSAL_MBUF* [in] - pointer to mBuf
 *
 * @param 
 * reserved UINT32 [in] - reserved parameter 
 *
 * @return void
 */
PRIVATE void
ixTimeSyncAccCodeletDummyCB (
	UINT32 cbTag, 
	IX_OSAL_MBUF *mBufPtr, 
	UINT32 reserved)
{
} /* end of ixTimeSyncAccCodeletDummyCB function */


/**
 * @ingroup IxTimeSyncAccCodelet
 *
 * @fn ixTimeSyncAccCodeletTxDoneCB (
	UINT32 cbTag, 
	IX_OSAL_MBUF* mBufPtr, 
 *
 * @brief Tx callback. This function gives semaphore to allow 
 *	  ixTimeSyncAccCodeletPTPMsgTransmit function to transmit
 * 	  next PTP message. 
 *
 * @param 
 * cbTag UINT32 [in] - argument passed to callback 
 *
 * @param 
 * mBufPtr IX_OSAL_MBUF* [in] - pointer to mBuf
 *
 * @return void
 */
PRIVATE void 
ixTimeSyncAccCodeletTxDoneCB (
	UINT32 cbTag, 
	IX_OSAL_MBUF* mBufPtr)
{

	if (IX_SUCCESS != ixOsalSemaphorePost (&ixTimeSyncAccCodeletSemId))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletTxDoneCB: Failed to give semaphore, port Id %d\n",
				cbTag, 0, 0, 0, 0, 0);
	}
} /* end of ixTimeSyncAccCodeletTxDoneCB function */


/**
 * @ingroup IxTimeSyncAccCodelet
 *
 * @fn ixTimeSyncAccCodeletPortConfigure (
	IxEthAccPortId portId)
 *
 * @brief Setup Tx and Rx callbacks and MAC address 
 *
 * @param 
 * portId IxEthAccPortId [in] - port ID 
 *
 * @return  IX_STATUS
 *          @li IX_SUCCESS - all function operations complete successfully 
 *          @li IX_FAIL    - any operation fails
 */
PRIVATE IX_STATUS
ixTimeSyncAccCodeletPortConfigure (IxEthAccPortId portId)
{
	IxEthAccMacAddr npeMacAddr;

	if (IX_ETH_PORT_3 < portId)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletPortConfigure: invalid port Id %d\n",
				portId, 0, 0, 0, 0, 0);
		return IX_FAIL;
	}

	/* setup TX callback */
	if (IX_ETH_ACC_SUCCESS != 
		ixEthAccPortTxDoneCallbackRegister (portId, ixTimeSyncAccCodeletTxDoneCB, portId))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletPortConfigure: Failed to register Tx callback for port %d\n",
				portId, 0, 0, 0, 0, 0);
		return IX_FAIL;
	}

	/* setup RX callback */
	if (IX_ETH_ACC_SUCCESS != 
		ixEthAccPortRxCallbackRegister (portId, ixTimeSyncAccCodeletDummyCB, portId))
	{
	    	ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletPortConfigure: Failed to register Rx callback for port %d\n",
				portId, 0, 0, 0, 0, 0);
		return IX_FAIL;
	}

	/* setup MAC address */
	ixOsalMemCopy (npeMacAddr.macAddress, 
		&ixTimeSyncAccCodeletNpeMacAddr[portId], 
		IX_IEEE803_MAC_ADDRESS_SIZE);  

	if (IX_ETH_ACC_SUCCESS != ixEthAccPortUnicastMacAddressSet(portId, &npeMacAddr))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletPortConfigure: failed to set the Unicast MAC Address of port %d\n", 
			portId, 0, 0, 0, 0, 0);

		return IX_FAIL;
	}
	return IX_SUCCESS;

} /* end of ixTimeSyncAccCodeletPortConfigure function */


/**
 * @ingroup IxTimeSyncAccCodelet
 *
 * @fn ixTimeSyncAccCodeletNewThreadCreate (
 	IxOsalVoidFnPtr func, 
	char *label)
 *
 * @brief Spawn a thread to execute given function
 *
 * @param 
 * func IxOsalVoidFnPtr [in] - pointer to the function that will be
 *			       executed after the thread is spawned. 
 * @param 
 * label char* [in] - pointer to the Thread name's buffer  
 *
 * @return IX_STATUS 
 *          @li IX_SUCCESS - create and start Thread successfully 
 *          @li IX_FAIL    - fail
 */
PRIVATE IX_STATUS 
ixTimeSyncAccCodeletNewThreadCreate (
	IxOsalVoidFnPtr func, 
	char *label)
{
	IxOsalThread threadId;
	IxOsalThreadAttr threadAttr;

	/* check the validity of function pointer */
	if (NULL == func)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletNewThreadCreate: NULL function pointer\n", 
			0, 0, 0, 0, 0, 0);
		return IX_FAIL;
	}

	/* check the validity of Thread name's buffer pointer */
	if (NULL == label) 
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletNewThreadCreate: NULL Thread name's pointer\n", 
			0, 0, 0, 0, 0, 0);
		return IX_FAIL;
	}
	
	/* zero out the thread attribute buffer */
	ixOsalMemSet ((void *)&threadAttr, 0, sizeof (IxOsalThreadAttr));

	/* setup thread attribute */
	threadAttr.name = label;
	threadAttr.priority = IX_OSAL_DEFAULT_THREAD_PRIORITY;
	
	if (IX_SUCCESS != ixOsalThreadCreate (&threadId, 
				&threadAttr,
				(IxOsalVoidFnVoidPtr)func,
				NULL))
	{ 
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletNewThreadCreate: Failed to spawn %s thread\n", 
			(UINT32) label, 0, 0, 0, 0, 0);

		return IX_FAIL;
	}
  	
	if (IX_SUCCESS != ixOsalThreadStart (&threadId))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletNewThreadCreate: Failed to start %s thread\n", 
			(UINT32) label, 0, 0, 0, 0, 0);
				
		return IX_FAIL;
	} 

	return IX_SUCCESS;

} /* end of ixTimeSyncAccCodeletNewThreadCreate function */


/**
 * @ingroup IxTimeSyncAccCodelet
 *
 * @fn ixTimeSyncAccCodeletPTPMsgBuild (
	UINT8 control)
 *
 * @brief Build PTP message using UDP protocol.
 *
 * UDP headers for all PTP messages
 *
 *	 N	Byte Count	Field Name
 *
 *	 0	6		destination MAC address
 * 	 6	6		source MAC address
 *	 12	2		type (0x0800 = IP datagram)
 *	 14	1		version IPV4, IP header length
 *	 15	1		type of service
 *	 16	2		IP datagram length
 *	 18	2		datagram sequence number
 *	 20	2		flags/fragments
 *	 22	1		time to live (TTL)
 *	 23	1		Protocol: UDP
 *	 24	2		IP header checksum
 *	 26	4		source IP address
 *	 30	4		destination IP address
 *	 34	2		source port number
 *	 36	2		destination port number
 *	 38	2		UDP length
 *	 40	2		UDP checksum
 *
 * PTP Sync and Delay_Req message specification (UDP user payload portion)
 *
 *	 42	2		version PTP
 *	 44	2		version network
 *	 46	16		subdomain
 *	 62	1		message type
 *	 63	1		source communication technology
 *	 64	6		source UUID
 *	 70	2		source port ID
 *	 72	2		sequence ID	
 *	 74	1		control
 *	 75	1		reserved
 *	 76	2		flags
 *	 78	4		reserved
 *	 82	4		origin timestamp (seconds)
 *	 86	4		origin timestamp (nanoseconds)
 *	 90	2		epoch number
 *	 92	2		current UTC offset
 *	 94	1		reserved
 *	 95	1		grandmaster communication technology
 *	 96	6		grandmaster clock UUID
 *	 102	2		grandmaster port ID
 *	 104	2		grandmaster sequence ID
 *	 106	3		reserved
 *	 109	1		grandmaster clock stratum
 *	 110	4		grandmaster clock identifier
 *	 114	2		reserved
 *	 116	2		grandmaster clock variance
 *	 118	1		reserved
 *	 119	1		grandmaster preferred
 *	 120	1		reserved
 *	 121	1		grandmaster's boundary clock
 *	 122	3		reserved
 *	 125	1		sync interval
 *	 126	2		reserved
 *	 128	2		local clock variance
 *	 130	2		reserved
 *	 132	2		local steps removed
 *	 134	3		reserved
 *	 137	1		local clock stratum
 *	 138	4		local clock identifier
 *	 142	1		reserved
 *	 143	1		parent comminucation technology
 *	 144	6		parent UUID
 *	 150	2		reserved
 *	 152	2		parent port field
 *	 154	2		reserved
 *	 156	2		estimate master variance
 *	 158	4		estimate master drift
 *	 162	3		reserved
 *	 165	1		utc reasonable
 *
 * This function only fills the relevant fields that the Time Sync
 * hardware reads to recognize the frame as PTP message. 
 * 
 * @param 
 * control UINT8 [in] - control field of PTP message
 *	- 0 : Sync Message
 *	- 1 : Delay_Req Message 
 *
 * @return void 
 */
PRIVATE void 
ixTimeSyncAccCodeletPTPMsgBuild (UINT8 control)
{		
	UINT8 *compPtr;
							
	compPtr = ixTimeSyncAccCodeletPtpMsgData;

	ixOsalMemSet (compPtr, 0, IX_TIMESYNCACC_CODELET_UDP_FRAME_LEN); 

	compPtr[12] = IX_TIMESYNCACC_CODELET_MSB_VALUE(IX_TIMESYNCACC_CODELET_IP_DATAGRAM);
	compPtr[13] = IX_TIMESYNCACC_CODELET_LSB_VALUE(IX_TIMESYNCACC_CODELET_IP_DATAGRAM);
	 
	compPtr[14] = IX_TIMESYNCACC_CODELET_IP_HEADER_LEN;

	compPtr[16] = IX_TIMESYNCACC_CODELET_MSB_VALUE(IX_TIMESYNCACC_CODELET_IP_DATAGRAM_LEN);
	compPtr[17] = IX_TIMESYNCACC_CODELET_LSB_VALUE(IX_TIMESYNCACC_CODELET_IP_DATAGRAM_LEN);

	compPtr[22] = IX_TIMESYNCACC_CODELET_TIME_TO_LIVE;
	compPtr[23] = IX_TIMESYNCACC_CODELET_UDP_PROTOCOL;

	compPtr[30] = ixTimeSyncAccCodeletPTPMulticastAddress[0];
	compPtr[31] = ixTimeSyncAccCodeletPTPMulticastAddress[1];
	compPtr[32] = ixTimeSyncAccCodeletPTPMulticastAddress[2];
	compPtr[33] = ixTimeSyncAccCodeletPTPMulticastAddress[3];

	compPtr[36] = IX_TIMESYNCACC_CODELET_MSB_VALUE(IX_TIMESYNCACC_CODELET_PTP_EVENT_PORT); 
	compPtr[37] = IX_TIMESYNCACC_CODELET_LSB_VALUE(IX_TIMESYNCACC_CODELET_PTP_EVENT_PORT);

	compPtr[39] = IX_TIMESYNCACC_CODELET_UDP_PAYLOAD_LEN;

	compPtr[62] = IX_TIMESYNCACC_CODELET_PTP_MESSAGE_TYPE;

	compPtr[74] = control;

} /* end of ixTimeSyncAccCodeletPTPMsgBuild function */


/**
 * @ingroup IxTimeSyncAccCodelet
 *
 * @fn ixTimeSyncAccCodeletPTPMsgTransmit ()
 *
 * @brief  Transmit Sync message from master port and Delay_Req message
 *	   from slave port every 2 seconds.  
 *
 * @return void
 */
PRIVATE void
ixTimeSyncAccCodeletPTPMsgTransmit ()
{
	IX_OSAL_MBUF *mBufPtr;
	IxEthAccPortId portId = 0; 
	IxTimeSyncAcc1588PTPPort tsChannel;
	IxTimeSyncAcc1588PTPMsgType txMsgType;
	IxTimeSyncAcc1588PTPPortMode tsChannelMode;

	/* clear PTP message transmission halt flag */
	ixTimeSyncAccCodeletTxHalt = FALSE;

	for (tsChannel = IX_TIMESYNCACC_NPE_A_1588PTP_PORT; 
	     tsChannel < IX_TIMESYNCACC_CODELET_MAX_TS_CHANNELS; 
	     tsChannel++)
	{
		
		portId = ixTimeSyncAccCodeletPortIdList[tsChannel];
		tsChannelMode = ixTimeSyncAccCodeletConfigPtr->tsChannelMode[tsChannel];
		txMsgType = ixTimeSyncAccCodeletPTPMsgTypeList[tsChannelMode]; 
	
		/* build PTP message */
		ixTimeSyncAccCodeletPTPMsgBuild (txMsgType);

		if (IX_SUCCESS != ixTimeSyncAccCodeletPortConfigure (portId))
		{
			ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletPTPMsgTransmit: failed to configure port %d\n", 
				portId, 0, 0, 0, 0, 0);

			/* terminate time sync codelet execution */
			ixTimeSyncAccCodeletUninit ();

			return;
		}
   
		if (IX_ETH_ACC_SUCCESS != ixEthAccPortEnable (portId))
		{
			ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletPTPMsgTransmit: failed to enable port %d\n", 
				portId, 0, 0, 0, 0, 0);
			
			/* terminate time sync codelet execution */
			ixTimeSyncAccCodeletUninit ();

			return;
		} 

		mBufPtr = ixTimeSyncAccCodeletGlobalMBuf[portId];

		if (NULL == mBufPtr)
		{
	  		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletPTPMsgTransmit: NULL mBuf pointer, port Id %d\n", 
				portId, 0, 0, 0, 0, 0);

			/* terminate time sync codelet execution */
			ixTimeSyncAccCodeletUninit ();

			return;
		}
		
		/* copy PTP message data to mBuf's data buffer */	
		ixOsalMemCopy (IX_OSAL_MBUF_MDATA(mBufPtr), 
			ixTimeSyncAccCodeletPtpMsgData, 
			IX_TIMESYNCACC_CODELET_UDP_FRAME_LEN);

	} /* end of for loop */

	if (IX_SUCCESS != ixOsalSemaphoreInit (&ixTimeSyncAccCodeletSemId, IX_TIMESYNCACC_CODELET_MAX_TS_CHANNELS))
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletPTPMsgTransmit: failed to create semaphore\n",
			0, 0, 0, 0, 0, 0);
		
		/* terminate time sync codelet execution */
		ixTimeSyncAccCodeletUninit ();

		return;
	}
	
	do 
	{
		/* halt PTP message transmission */
		if (TRUE == ixTimeSyncAccCodeletTxHalt)
		{
			ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixTimeSyncAccCodeletPTPMsgTransmit: PTP message transmission was halted\n",
				0, 0, 0, 0, 0, 0);

			return;
		}

		/* sleep and wait for interval time to elapse before transmitting next PTP message */
		ixOsalSleep (IX_TIMESYNCACC_CODELET_PTP_MSG_XMIT_INTERVAL);

		for (portId = IX_ETH_PORT_1; portId <= IX_ETH_PORT_3; portId++)
		{
			if (IX_SUCCESS != ixOsalSemaphoreWait (&ixTimeSyncAccCodeletSemId, IX_TIMESYNCACC_CODELET_PTP_MSG_XMIT_INTERVAL))
			{
				ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletPTPMsgTransmit: PTP message transmission error at port %d\n",
					portId, 0, 0, 0, 0, 0);
				
				/* terminate time sync codelet execution */
				ixTimeSyncAccCodeletUninit ();

				return;
	
			}

			if (IX_ETH_ACC_SUCCESS != ixEthAccPortTxFrameSubmit (portId, 
									ixTimeSyncAccCodeletGlobalMBuf[portId], 
									IX_ETH_ACC_TX_DEFAULT_PRIORITY))
			{
				ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletPTPMsgTransmit: failed to transmit PTP message from port %d\n", 
					portId, 0, 0, 0, 0, 0);

				
				/* terminate time sync codelet execution */
				ixTimeSyncAccCodeletUninit ();

				return;
			}
		} /* end of for loop */
				
	} while (TRUE);

} /* end of ixTimeSyncAccCodeletPTPMsgTransmit function */


/**
 * @ingroup IxTimeSyncAccCodelet
 *
 * @fn ixTimeSyncAccCodeletPTPMsgTransmitStop ()
 *
 * @brief Stop PTP message transmission and destroy semaphore that is used in the 
 *	  transmission.  
 *
 * @return void
 */
PRIVATE void 
ixTimeSyncAccCodeletPTPMsgTransmitStop ()
{
	/* set PTP message transmission halt flag */
	ixTimeSyncAccCodeletTxHalt = TRUE;

	/* wait for a while to let unfinished PTP message transmission to complete */
	ixOsalSleep (IX_TIMESYNCACC_CODELET_PTP_MSG_XMIT_INTERVAL);

	if (NULL != ixTimeSyncAccCodeletSemId)
	{
		if (IX_SUCCESS != ixOsalSemaphoreDestroy (&ixTimeSyncAccCodeletSemId))
		{
			ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletPTPMsgTransmitStop: failed to destroy semaphore\n",
				0, 0, 0, 0, 0, 0);
		}
		ixTimeSyncAccCodeletSemId = NULL;
	}

} /* end of ixTimeSyncAccCodeletPTPMsgTransmitStop function */


/**
 * @ingroup IxTimeSyncAccCodelet
 *
 * @fn ixTimeSyncAccCodeletPTPMsgShow (
 	BOOL receive,
	IxTimeSyncAcc1588PTPPort channel,
	IxTimeSyncAccPtpMsgData *ptpMsgData)
 *
 * @brief Display PTP message type and relevant information 
 *
 * @param 
 * receive BOOL [in] - direction of detected message.
 *	- FALSE : PTP message is transmitted.
 *	- TRUE  : PTP message is received.
 *
 * channel IxTimeSyncAcc1588PTPPort [in] - channel number 
 *
 * ptpMsgData IxTimeSyncAccPtpMsgData* [in] - pointer to 
 *		IxTimeSyncAccPtpMsgData buffer.
 *
 * @return void
 */
PRIVATE void 
ixTimeSyncAccCodeletPTPMsgShow (
	BOOL receive,
	IxTimeSyncAcc1588PTPPort tsChannel,
	IxTimeSyncAccPtpMsgData *ptpMsgData		
	)
{
	/* get the operation mode at a given channel */
	IxTimeSyncAcc1588PTPPortMode tsChannelMode = ixTimeSyncAccCodeletConfigPtr->tsChannelMode[tsChannel];
	/* PTP message is detected */
	ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixTimeSyncAccCodeletPTPMsgShow: %s channel (%s) %s %s Msg @ system time %08x %08x\n", 
		(UINT32) (ixTimeSyncAccCodeletTSChannelLabel[tsChannel]),
		(UINT32) (ixTimeSyncAccCodeletTSChannelModeLabel[tsChannelMode]),
		(UINT32) (receive ? "received" : "transmitted"),
		(UINT32) (ixTimeSyncAccCodeletPTPMessageLabel[ptpMsgData->ptpMsgType]),
		ptpMsgData->ptpTimeStamp.timeValueHighWord,
		ptpMsgData->ptpTimeStamp.timeValueLowWord); 

	if (TRUE == receive)
	{
		/* show sequence number and UuId of received message */
		ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixTimeSyncAccCodeletPTPMsgShow: \tptpSequenceNumber %08x ptpUuid %08x %08x\n", 
			ptpMsgData->ptpSequenceNumber, ptpMsgData->ptpUuid.uuidValueHighHalfword,
		 	ptpMsgData->ptpUuid.uuidValueLowWord, 0, 0, 0);
	}

} /* end of ixTimeSyncAccCodeletPTPMsgShow function */


/**
 * @ingroup IxTimeSyncAccCodelet
 *
 * @fn ixTimeSyncAccCodeletPTPMsgCheck (void)
 *
 * @brief  Check if any PTP message is received or transmitted at 
 * 	   any channel.
 *
 * @param  void 
 *
 * @return  IX_STATUS
 *          @li IX_SUCCESS - no error detected during check 
 *          @li IX_FAIL    - error detected during check 
 */
PUBLIC IX_STATUS 
ixTimeSyncAccCodeletPTPMsgCheck (void)
{
	IxTimeSyncAccPtpMsgData ptpMsgData;
	IxTimeSyncAccStatus tsStatus;
	IxTimeSyncAcc1588PTPPort channel;

	for (channel = IX_TIMESYNCACC_NPE_A_1588PTP_PORT; 
	     channel < IX_TIMESYNCACC_CODELET_MAX_TS_CHANNELS; 
	     channel++)
	{
		/* initialize ptpMsgData buffer */
		ixOsalMemSet ((void *)&ptpMsgData, 0xff, sizeof (IxTimeSyncAccPtpMsgData));

		/* check if any PTP message is received */
		tsStatus = ixTimeSyncAccPTPRxPoll (channel, &ptpMsgData);	

		/* PTP message is detected */
		if (IX_TIMESYNCACC_SUCCESS == tsStatus)
		{
			ixTimeSyncAccCodeletPTPMsgShow (TRUE, channel, &ptpMsgData);
		}
		else if  (IX_TIMESYNCACC_NOTIMESTAMP == tsStatus)
		{
			ixOsalLog (IX_OSAL_LOG_LVL_DEBUG1, IX_OSAL_LOG_DEV_STDOUT, "ixTimeSyncAccCodeletPTPMsgCheck: no new PTP message is received at %s channel\n",
				(UINT32) (ixTimeSyncAccCodeletTSChannelLabel[channel]), 
				0, 0, 0, 0, 0);
		}
		/* error is detected */
		else 
		{
			ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletPTPMsgCheck: failed to determine if PTP message is received at channel %d, error code %d\n",
				channel, tsStatus, 0, 0, 0, 0);
			return IX_FAIL; 
		}
		
		/* initialize ptpMsgData buffer */
		ixOsalMemSet ((void *)&ptpMsgData, 0xff, sizeof (IxTimeSyncAccPtpMsgData));

		/* check if any PTP message is transmitted */
		tsStatus = ixTimeSyncAccPTPTxPoll (channel, &ptpMsgData);	

		/* PTP message is detected */
		if (IX_TIMESYNCACC_SUCCESS == tsStatus)
		{
			ixTimeSyncAccCodeletPTPMsgShow (FALSE, channel, &ptpMsgData);
		}
		else if  (IX_TIMESYNCACC_NOTIMESTAMP == tsStatus)
		{
			ixOsalLog (IX_OSAL_LOG_LVL_DEBUG1, IX_OSAL_LOG_DEV_STDOUT, "ixTimeSyncAccCodeletPTPMsgCheck: no new PTP message is transmitted at %s channel\n",
				(UINT32) (ixTimeSyncAccCodeletTSChannelLabel[channel]), 
				0, 0, 0, 0, 0);
		}
		/* error is detected */
		else 
		{
			ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletPTPMsgCheck: failed to determine if PTP message is transmitted at channel %d, error code %d\n",
				channel, tsStatus, 0, 0, 0, 0);
			return IX_FAIL; 
		}
	}
	ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "\n", 0, 0, 0, 0, 0, 0); 

	return IX_SUCCESS;
} /* end of ixTimeSyncAccCodeletPTPMsgCheck function */

	
/**
 * @ingroup IxTimeSyncAccCodelet
 *
 * @fn ixTimeSyncAccCodeletTargetTimeHitCallback (void)
 *
 * @brief  When target time is hit, first clear target time. Then,
 *         check if any PTP message is received or transmitted at
 *	   any channel. If termination is not requested, set next
 * 	   target time (approximately 1 second later).
 *
 * @param  void 
 *
 * @return void 
 */
PRIVATE void
ixTimeSyncAccCodeletTargetTimeHitCallback (void)
{
	/* 
	 * clear target time to prevent second interrupt 
	 * from being generated on same event
	 */
	ixTimeSyncAccCodeletTargetTimeClear ();

	/* check if any PTP message is detected */
	if (IX_SUCCESS != ixTimeSyncAccCodeletPTPMsgCheck ())
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletTargetTimeHitCallback: PTP message check failed, terminate timeSyncAcc codelet execution\n",
			0, 0, 0, 0, 0, 0);

		/* terminate time sync codelet execution */
		ixTimeSyncAccCodeletQuit ();

		return;
	}

	/* codelet execution was requested to be terminated */
	if (TRUE == ixTimeSyncAccCodeletTerminate)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT, "ixTimeSyncAccCodeletTargetTimeHitCallback: target time was cleared and disabled\n",
			0, 0, 0, 0, 0, 0);
		return;
	}

	/* set next target time */
	if (IX_SUCCESS != ixTimeSyncAccCodeletTargetTimeSet ())
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletTargetTimeHitCallback: failed to set target time, terminate timeSyncAcc codelet execution\n",
			0, 0, 0, 0, 0, 0);

		/* terminate time sync codelet execution */
		ixTimeSyncAccCodeletQuit ();
	}

} /* end of ixTimeSyncAccCodeletTargetTimeHitCallback function */


/**
 * @ingroup IxTimeSyncAccCodelet
 *
 * @fn ixTimeSyncAccCodeletTargetTimeSet (void)
 *
 * @brief  Set target time and enable target time interrupt.  
 *
 * @param  void 
 *
 * @return  IX_STATUS
 *          @li IX_SUCCESS - set target time successfully
 *          @li IX_FAIL    - fails
 */
PRIVATE IX_STATUS 
ixTimeSyncAccCodeletTargetTimeSet (void)
{
	IxTimeSyncAccTimeValue	tsSystemTime, tsTargetTime;
	IxTimeSyncAccStatus tsStatus;
	UINT32 interval = IX_TIMESYNCACC_CODELET_TARGET_TIME_HIT_INTERVAL;

	/* get current system time */	
	tsStatus = ixTimeSyncAccSystemTimeGet (&tsSystemTime);
	if (IX_TIMESYNCACC_SUCCESS != tsStatus)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletTargetTimeSet: failed to get system time, error code %d\n",
			tsStatus, 0, 0, 0, 0, 0);
		
		return IX_FAIL;
	}

	/* calculate new target time using the given interval */
	tsTargetTime.timeValueLowWord = tsSystemTime.timeValueLowWord + interval;	
	tsTargetTime.timeValueHighWord = tsSystemTime.timeValueHighWord;

	/* if next target time's lower 32 bit value will roll over */
	if (tsTargetTime.timeValueLowWord < tsSystemTime.timeValueLowWord)
	{
		/* if next target time's upper 32 bit value will roll over */
		if (IX_TIMESYNCACC_CODELET_ROLLOVER_VALUE == tsSystemTime.timeValueHighWord)
		{
			/* reset system time */
			if (IX_SUCCESS != ixTimeSyncAccCodeletSystemTimeClear ())
			{
				ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletTargetTimeSet: failed to reset system time\n",
					0, 0, 0, 0, 0, 0);

				return IX_FAIL;
			}

			/* re-calculate new target time */
			tsTargetTime.timeValueLowWord = interval;
			tsTargetTime.timeValueHighWord = 0;
		}
		/* increment next target time's upper 32 bit value */
		else
		{
			tsTargetTime.timeValueHighWord = tsSystemTime.timeValueHighWord + 1;	
		}
	} /* end of if (tsTargetTime.timeValueLowWord < tsSystemTime.timeValueLowWord) */
	
	/* write next target time to registers */	
	tsStatus = ixTimeSyncAccTargetTimeSet (tsTargetTime);
	if (IX_TIMESYNCACC_SUCCESS != tsStatus)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletTargetTimeSet: failed to set next target time, error code %d\n",
			tsStatus, 0, 0, 0, 0, 0);
		return IX_FAIL;
	}

	/* enable target time interrupt */
	tsStatus = ixTimeSyncAccTargetTimeInterruptEnable (
			(IxTimeSyncAccTargetTimeCallback) ixTimeSyncAccCodeletTargetTimeHitCallback);
	if (IX_TIMESYNCACC_SUCCESS != tsStatus)
	{
		/* clear target time */
		ixTimeSyncAccCodeletTargetTimeClear ();

		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletTargetTimeSet: failed to enable Target Time interrupt, error code %d\n",
			tsStatus, 0, 0, 0, 0, 0);
		return IX_FAIL;
	}
	
	ixOsalLog (IX_OSAL_LOG_LVL_DEBUG1, IX_OSAL_LOG_DEV_STDOUT, "ixTimeSyncAccCodeletTargetTimeSet: current system time %08x%08x (hex) next target time %08x%08x (hex)\n",
		tsSystemTime.timeValueHighWord,tsSystemTime.timeValueLowWord,
		tsTargetTime.timeValueHighWord,tsTargetTime.timeValueLowWord, 0, 0);

	return IX_SUCCESS;
} /* end of ixTimeSyncAccCodeletTargetTimeSet function */


/**
 * @ingroup IxTimeSyncAccCodelet
 *
 * @fn ixTimeSyncAccCodeletSystemTimeClear (void)
 *
 * @brief  Clear system time to zero.  
 *
 * @param  void 
 *
 * @return  IX_STATUS
 *          @li IX_SUCCESS - clear system time successfully
 *          @li IX_FAIL    - fails
 */
PRIVATE IX_STATUS 
ixTimeSyncAccCodeletSystemTimeClear (void)
{
	IxTimeSyncAccTimeValue	tsSystemTime;
	IxTimeSyncAccStatus	tsStatus;
	
	tsSystemTime.timeValueLowWord = 0; 
	tsSystemTime.timeValueHighWord = 0;

	tsStatus = ixTimeSyncAccSystemTimeSet (tsSystemTime);
	if (IX_TIMESYNCACC_SUCCESS != tsStatus)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletSystemTimeClear: failed to clear system time, error code %d\n",
			tsStatus, 0, 0, 0, 0, 0);
		return IX_FAIL;
	}
	return IX_SUCCESS;

} /* end of ixTimeSyncAccCodeletSystemTimeClear function */


/**
 * @ingroup IxTimeSyncAccCodelet
 *
 * @fn ixTimeSyncAccCodeletTargetTimeClear (void)
 *
 * @brief  Clear Target Time Hit condition by setting Target Time to  
 *	   maximum value and disabling Target Time interrupt.
 *
 * @param  void 
 *
 * @return void 
 */
PRIVATE void 
ixTimeSyncAccCodeletTargetTimeClear (void)
{
	IxTimeSyncAccTimeValue	tsTargetTime;
	IxTimeSyncAccStatus	tsStatus;
	
	tsTargetTime.timeValueLowWord = 0xFFFFFFFF;
	tsTargetTime.timeValueHighWord = 0xFFFFFFFF;

	tsStatus = ixTimeSyncAccTargetTimeSet (tsTargetTime);
	if (IX_TIMESYNCACC_SUCCESS != tsStatus)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletTargetTimeClear: failed to clear Target Time, error code %d\n",
			tsStatus, 0, 0, 0, 0, 0);
	}

	/* disable Target Time interrupt */	
	tsStatus = ixTimeSyncAccTargetTimeInterruptDisable ();
	if (IX_TIMESYNCACC_SUCCESS != tsStatus)
	{
		ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, "ixTimeSyncAccCodeletTargetTimeClear: failed to disable Target Time interrupt, error code %d\n",
			tsStatus, 0, 0, 0, 0, 0); 
	}
} /* end of ixTimeSyncAccCodeletTargetTimeClear () function */

#endif /* __ixp46X */


