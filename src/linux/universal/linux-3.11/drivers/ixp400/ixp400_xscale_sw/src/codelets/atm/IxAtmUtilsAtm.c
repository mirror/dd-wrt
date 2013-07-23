/*
 * FileName:    IxAtmAtmUtilsAtm.c
 * Author: Intel Corporation
 * Created:     15 May 2002
 * Description: Atm Codelet utilities for Atm.
 *
 *
 * Design Notes:
 *
 *
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

/*
 * User defined include files
 */
#if defined(__wince) && defined(IX_USE_SERCONSOLE)
	#include "IxSerConsole.h"
	#define printf ixSerPrintf
	#define gets ixSerGets
#endif


#include "IxAtmm.h"
#include "IxAtmSch.h"
#include "IxAtmdAcc.h"
#include "IxNpeMh.h"
#include "IxNpeDl.h"
#include "IxQMgr.h"
#include "IxOsal.h"
#include "IxAtmCodelet_p.h"

/*
 * #defines and macros
 */
#define DISCONNECT_RETRY_DELAY     (25)
#define DISCONNECT_RETRY_COUNT     (200)
#define UTOPIA_PORT_RATE           (53 * 1000000)
#define SINGLE_PORT_SPHY_IMAGE_ID  IX_NPEDL_NPEIMAGE_NPEA_HSS0_ATM_SPHY_1_PORT
#define MULTI_PORT_MPHY_IMAGE_ID   IX_NPEDL_NPEIMAGE_NPEA_ATM_MPHY_12_PORT

#ifdef IX_UTOPIAMODE
#define IX_ATM_CODELET_NPEA_IMAGE_ID SINGLE_PORT_SPHY_IMAGE_ID 
#define IX_ATM_CODELET_PHY_MODE IX_ATMM_SPHY_MODE
#else
#define IX_ATM_CODELET_NPEA_IMAGE_ID MULTI_PORT_MPHY_IMAGE_ID 
#define IX_ATM_CODELET_PHY_MODE IX_ATMM_MPHY_MODE
#endif

/* Define for LINERATE, PCR, SCR and MBS.
 * 
 * Since the Transmit bandwidth is set to IX_ATMCODELET_LINERATE and there 
 * are 16 real-time VCs (8VBR and 8CBR), the maximum allocated bandwidth for
 * each rt-VC is IX_ATMCODELET_LINERATE divided by IX_ATMCODLEET_NUMRT_VCS, 
 * which gives approx 1391 cells/sec. This means that the CBR's PCR max 
 * value is 1391 cells/sec and VBR's SCR max value is 1391 cells/sec. 
 * This due to CAC.
 */
#define IX_ATMCODELET_LINERATE     22257 /* cells/sec. This gives approx 9Mbps, i.e. ( 22257 x 53 x 8)bps */
#define IX_ATMCODELET_PCR          18867 /* cells/sec. Approx 8Mbps */
#define IX_ATMCODELET_SCR          1391  /* cells/sec. LINERATE is divided by NUM_RT_VCS */
#define IX_ATMCODELET_MBS          100   /* cells */
#define IX_ATMCODELET_NUM_RT_VCS   16    /* 16 real-time VCs. 8 for VBR and 8 for CBR */
/*
 * Typedefs
 */
typedef struct
{
    BOOL inUse;
    IxAtmConnId connId;
    IxAtmLogicalPort port;
} ChannelInfo;

/*
 * Variable declarations global to this file. Externs are followed by
 * statics.
 */
static ChannelInfo rxChannelInfo[IX_ATM_MAX_NUM_VC];
static ChannelInfo txChannelInfo[IX_ATM_MAX_NUM_VC];

static IxAtmSchedulerVcId rxVcId[IX_ATM_MAX_NUM_VC];
static IxAtmNpeRxVcId rxNpeVcIdTable[IX_ATM_MAX_NUM_VC];

static IxAtmSchedulerVcId txVcId[IX_ATM_MAX_NUM_VC];

static IxAtmLogicalPort rxPort[IX_ATM_MAX_NUM_VC];
static IxAtmLogicalPort txPort[IX_ATM_MAX_NUM_VC];

static BOOL ixAtmCodelet8Vbr8Cbr16Ubr = FALSE; /* If TRUE - Setup multi real-time  VCs*/

/*
 * Forward declarartions
 */
PRIVATE IX_STATUS
rxVcDisconnect(IxAtmConnId rxConnId);

PRIVATE IX_STATUS
txVcDisconnect(IxAtmConnId txConnId);

PRIVATE IX_STATUS
txFreeChannelGet (UINT32 *channelId);

PRIVATE IX_STATUS
rxFreeChannelGet (UINT32 *channelId);

PRIVATE IX_STATUS
rxChannelFind (IxAtmConnId connId, UINT32 *channelId);

PRIVATE IX_STATUS
txChannelFind (IxAtmConnId connId, UINT32 *channelId);

/*
 * Function definitions.
 */

PUBLIC IX_STATUS
ixAtmUtilsAtmVcRegisterConnect (IxAtmLogicalPort port,
				 unsigned vpi,
				 unsigned vci,
				 IxAtmdAccAalType aalType,
				 IxAtmServiceCategory atmService,
				 IxAtmRxQueueId rxQueueId,
				 IxAtmdAccRxVcRxCallback rxCallback,
				 UINT32 minimumReplenishCount,
				 IxAtmdAccTxVcBufferReturnCallback bufferFreeCallback,
				 IxAtmdAccRxVcFreeLowCallback rxFreeLowCallback,
				 IxAtmdAccUserId userId,
				 IxAtmConnId *rxConnId,
				 IxAtmConnId *txConnId)
{
    IX_STATUS retval;
    IxAtmmVc txVc;
    IxAtmmVc rxVc;
    UINT32 rxFreeQueueSize;
    unsigned rxChannelIdx;
    unsigned txChannelIdx;

    if (rxFreeChannelGet(&rxChannelIdx) == IX_FAIL)
    {
        ixOsalLog(IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR, "No free Rx channell\n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    if (txFreeChannelGet(&txChannelIdx) == IX_FAIL)
    {
        ixOsalLog(IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR, "No free Tx channell\n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    rxPort[rxChannelIdx] = port;
    txPort[txChannelIdx] = port;

    ixOsalMemSet(&txVc, 0, sizeof(txVc));
    ixOsalMemSet(&rxVc, 0, sizeof(rxVc));

    /* Setup Tx Vc descriptor */
    txVc.vpi = vpi;
    txVc.vci = vci;
    txVc.direction = IX_ATMM_VC_DIRECTION_TX;
    txVc.trafficDesc.atmService = atmService;
    
    /* Verify whether ATM codelet is going to use Real-time VCs.*/
    if (ixAtmUtilsAtmRtVcsGet())
    {
	/* The traffic parameters can be changed accodingly. The definitions
	* are declared in this file */
	if (atmService == IX_ATM_UBR)
	{
	    txVc.trafficDesc.pcr = IX_ATMCODELET_LINERATE;
	}
	else if (atmService == IX_ATM_VBR)
	{
	    txVc.trafficDesc.pcr = IX_ATMCODELET_PCR;
	    txVc.trafficDesc.scr = IX_ATMCODELET_SCR;
	    txVc.trafficDesc.mbs = IX_ATMCODELET_MBS;
	}
	else if (atmService == IX_ATM_CBR)
	{
	    /* CBR's PCR value is set to LINERATE divided by NUM_RT_VCS,
	     * because the bandwidth is shared by 16 real-time VCs */
	    txVc.trafficDesc.pcr = IX_ATMCODELET_LINERATE / IX_ATMCODELET_NUM_RT_VCS; 
	}
    }
    else
    {
	/* Real-time VCs are not required.
	 * Setup using 32 UBR VCs which use PCR parameter only */
	txVc.trafficDesc.pcr = UTOPIA_PORT_RATE;
    }

    /* Setup tx VC */
    retval = ixAtmmVcRegister (port,
			       &txVc,
			       &txVcId[txChannelIdx]);

    if (retval != IX_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR, "Fail, register Tx VC \n", 0, 0, 0, 0, 0, 0);
	return retval;
    }

    /* Setup Rx Vc descriptor */
    rxVc.vpi = vpi;
    rxVc.vci = vci;
    rxVc.direction = IX_ATMM_VC_DIRECTION_RX;
    rxVc.trafficDesc.atmService = atmService;

    /* Setup rx VC */
    retval = ixAtmmVcRegister (port,
			       &rxVc,
			       &rxVcId[rxChannelIdx]);

    if (retval != IX_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR, "Fail, register Rx VC \n", 0, 0, 0, 0, 0, 0);
	return retval;
    }

    /* Connect Tx to the Vc */
    retval = ixAtmdAccTxVcConnect (port,
				   vpi,
				   vci,
				   aalType,
				   userId,
				   bufferFreeCallback,
				   txConnId);

    if (retval != IX_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR, "Fail, connect to Tx VC \n", 0, 0, 0, 0, 0, 0);
	return retval;
    }

    txChannelInfo[txChannelIdx].connId = *txConnId;
    txChannelInfo[txChannelIdx].port = port;

    /* Connect Rx to the VC */
    retval = ixAtmdAccRxVcConnect (port,
				   vpi,
				   vci,
				   aalType,
				   rxQueueId,
				   userId,
				   rxCallback,
				   minimumReplenishCount,
				   rxConnId,
				   &rxNpeVcIdTable[rxChannelIdx]);

    if (retval != IX_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR, "Fail, connect to Rx VC \n", 0, 0, 0, 0, 0, 0);
	return retval;
    }

    rxChannelInfo[rxChannelIdx].connId = *rxConnId;
    rxChannelInfo[rxChannelIdx].port = port;

    retval = ixAtmdAccRxVcFreeEntriesQuery(*rxConnId,
                                           &rxFreeQueueSize);
    if (retval != IX_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR, "Fail, get the no. entries in rx free queue\n",0,0,0,0,0,0);
	return retval;
    }

 
    /* Replenish Rx buffers if callback is not NULL*/
    if (rxFreeLowCallback != NULL)
    {
	rxFreeLowCallback(userId);
	
	retval = ixAtmdAccRxVcFreeLowCallbackRegister (*rxConnId,
						       rxFreeQueueSize / 4,
						       rxFreeLowCallback);
    }
	
    if (retval != IX_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR, "Failed to register RxFreeLowCalback \n", 0, 0, 0, 0, 0, 0);
	return retval;
    }

    /* Register replish more callback */

    retval = ixAtmdAccRxVcEnable (*rxConnId);

    if (retval != IX_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR, "Failed to enable Rx VC \n", 0, 0, 0, 0, 0, 0);
	return retval;
    }

    return IX_SUCCESS;
}

/* ---------------------------------------------------
 */
PRIVATE IX_STATUS
rxVcDisconnect(IxAtmConnId rxConnId)
{
    IX_STATUS retval;
    UINT32 retryCount;

    retval =  ixAtmdAccRxVcDisable(rxConnId);
    if (retval != IX_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR, "rx disable failed (%d)\n", retval, 0, 0, 0, 0, 0);
	return retval;
    }

    retryCount = 0;
    do
    {
	retval = ixAtmdAccRxVcTryDisconnect(rxConnId);
	if (retval == IX_ATMDACC_RESOURCES_STILL_ALLOCATED)
	{
	    ixOsalSleep(DISCONNECT_RETRY_DELAY);
	}
    }
    while ((retryCount++ < DISCONNECT_RETRY_COUNT)
	   && (retval == IX_ATMDACC_RESOURCES_STILL_ALLOCATED));

    if (retval != IX_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR, "rx disconnect failed (%d)\n",
		    retval, 0, 0, 0, 0, 0);
	return retval;
    }

    return IX_SUCCESS;
}

/* ---------------------------------------------------
 */
PRIVATE IX_STATUS
txVcDisconnect(IxAtmConnId txConnId)
{
    IX_STATUS retval;
    UINT32 retryCount;

    retryCount = 0;
    do
    {
	retval = ixAtmdAccTxVcTryDisconnect(txConnId);
	if (retval == IX_ATMDACC_RESOURCES_STILL_ALLOCATED)
	{
	    ixOsalSleep(DISCONNECT_RETRY_DELAY);
	}
    }
    while ((retryCount++ < DISCONNECT_RETRY_COUNT)
	   && (retval == IX_ATMDACC_RESOURCES_STILL_ALLOCATED));

    if (retval != IX_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR, "tx disconnect failed (%d)\n",
		    retval, 0, 0, 0, 0, 0);
	return retval;
    }

    return IX_SUCCESS;
}

/* ---------------------------------------------------
 */
PUBLIC IX_STATUS
ixAtmUtilsAtmVcUnregisterDisconnect (IxAtmConnId rxConnId, IxAtmConnId txConnId)
{
    IX_STATUS retval = IX_SUCCESS;
    UINT32 rxChannelIdx;
    UINT32 txChannelIdx;

    if (rxChannelFind(rxConnId, &rxChannelIdx) == IX_SUCCESS)
    {
        retval = rxVcDisconnect(rxConnId);
        if (retval != IX_SUCCESS)
        {
            ixOsalLog(IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR, "Failed to disconnect an rx channel\n",
			0, 0, 0, 0, 0, 0);
            return IX_FAIL;
        }

        retval = ixAtmmVcDeregister(rxPort[rxChannelIdx], rxVcId[rxChannelIdx]);
        if (retval != IX_SUCCESS)
        {
            ixOsalLog(IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR, "Failed to unregister an rx channel\n",
			0, 0, 0, 0, 0, 0);
            return IX_FAIL;
        }
        rxChannelInfo[rxChannelIdx].inUse = FALSE;
    }

    if (txChannelFind(txConnId, &txChannelIdx) == IX_SUCCESS)
    {
        retval = txVcDisconnect(txConnId);
        if (retval != IX_SUCCESS)
        {
            ixOsalLog(IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR, "Failed to disconnect a tx channel\n",
			0, 0, 0, 0, 0, 0);
            return IX_FAIL;
        }

        retval = ixAtmmVcDeregister(txPort[txChannelIdx], txVcId[txChannelIdx]);
        if (retval != IX_SUCCESS)
        {
            ixOsalLog(IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR, "Failed to unregister a tx channel\n",
			0, 0, 0, 0, 0, 0);
            return IX_FAIL;
        }
        txChannelInfo[txChannelIdx].inUse = FALSE;
    }

    return IX_SUCCESS;
}

/* ---------------------------------------------------
 */
PUBLIC IX_STATUS
ixAtmUtilsAtmAllVcsDisconnect (void)
{
    IX_STATUS retval;
    int channelIdx;

    for (channelIdx=0; channelIdx<IX_ATM_MAX_NUM_VC; channelIdx++)
    {
        retval = ixAtmUtilsAtmVcUnregisterDisconnect (rxChannelInfo[channelIdx].connId, 
						       txChannelInfo[channelIdx].connId);
        if (retval != IX_SUCCESS)
        {
	    ixOsalLog(IX_OSAL_LOG_LVL_ERROR,IX_OSAL_LOG_DEV_STDERR, "Failed to unregister a channel\n",
			0, 0, 0, 0, 0, 0);
	    return IX_FAIL;
        }
    }
    return IX_SUCCESS;
}

/* ---------------------------------------------------
 */
PUBLIC IX_STATUS
ixAtmUtilsAtmImageDownload (unsigned numPorts, 
			     IxAtmmPhyMode *phyMode)
{
    IX_STATUS retval = IX_SUCCESS;

    retval = ixNpeDlNpeInitAndStart (IX_ATM_CODELET_NPEA_IMAGE_ID);
    if (retval != IX_SUCCESS)
    {
	printf ("Failed to download NPEA image\n");
    }

    if (retval == IX_SUCCESS)
    {
	*phyMode = IX_ATM_CODELET_PHY_MODE;
    }
    else
    {
	printf("NPEA download failed \n");
    }

    return retval;
}

PRIVATE IX_STATUS
rxFreeChannelGet (UINT32 *channelId)
{
    int i;
    for (i = 0; i < IX_ATM_MAX_NUM_VC; i ++)
    {
	if (!rxChannelInfo[i].inUse)
	{	    
	    *channelId = i;
	    rxChannelInfo[i].inUse = TRUE;
	    return IX_SUCCESS;
	}
    }
    return IX_FAIL;
}

PRIVATE IX_STATUS
txFreeChannelGet (UINT32 *channelId)
{
    int i;
    for (i = 0; i < IX_ATM_MAX_NUM_VC; i ++)
    {
	if (!txChannelInfo[i].inUse)
	{	    
	    *channelId = i;
	    txChannelInfo[i].inUse = TRUE;
	    return IX_SUCCESS;
	}
    }
    return IX_FAIL;
}

PRIVATE IX_STATUS
rxChannelFind (IxAtmConnId connId, UINT32 *channelId)
{
    int i;
    for (i = 0; i < IX_ATM_MAX_NUM_VC; i ++)
    {
	if ((rxChannelInfo[i].inUse) && (rxChannelInfo[i].connId == connId))
	{	    
	    *channelId = i;
	    return IX_SUCCESS;
	}
    }
    return IX_FAIL;
}

PRIVATE IX_STATUS
txChannelFind (IxAtmConnId connId, UINT32 *channelId)
{
    int i;
    for (i = 0; i < IX_ATM_MAX_NUM_VC; i ++)
    {
	if ((txChannelInfo[i].inUse) && (txChannelInfo[i].connId == connId))
	{	    
	    *channelId = i;
	    return IX_SUCCESS;
	}
    }
    return IX_FAIL;
}
         
/* This function setup the ATM codelet to use 8VBR, 8CBR, and 16UBR VCs */
PUBLIC void
ixAtmUtilsAtmRtVcsSet(void)
{
    ixAtmCodelet8Vbr8Cbr16Ubr = TRUE;
}

/* This function is responsible to return the TRUE or FALSE status.
 * It is to indicate whether the ATM codelet should setup real-time VCS
 * If TRUE, real-time VCs will be setup. If FALSE, real-time VCS will
 *  not be setup. */
PUBLIC BOOL
ixAtmUtilsAtmRtVcsGet(void)
{
    return ixAtmCodelet8Vbr8Cbr16Ubr;
}

/* This function is responsible to unset the global variable to FALSE
 * so that the ATM codelet will not setup real-time VCs */
PUBLIC void
ixAtmUtilsAtmRtVcsUnset(void)
{
    ixAtmCodelet8Vbr8Cbr16Ubr = FALSE;
}
