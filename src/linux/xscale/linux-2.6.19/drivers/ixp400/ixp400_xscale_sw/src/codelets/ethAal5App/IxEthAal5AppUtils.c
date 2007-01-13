/*
 * FileName:    IxEthAal5AppUtils.c
 *
 * Created:     15 May 2002
 *
 * Description: Codelet utils for EthAal5 Application
 *
 * 
 * @par
 * IXP400 SW Release Crypto version 2.3
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2005, Intel Corporation.
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
 * System defined include files
 */

/*
 * User defined include files
 */
#include "IxOsal.h"
#include "IxAtmm.h"
#include "IxAtmTypes.h"
#include "IxAtmSch.h"
#include "IxAtmdAcc.h"
#include "IxNpeMh.h"

#include "IxQMgr.h"
#include "IxEthAal5App_p.h"

/*
 * #defines and macros
 */
#define NO_UTOPIA_BUILD_ID_        1

#define BSR_CTRL1 0x2

#define EXPBUS_REGS_LOCATION    IX_OSAL_IXP400_EXP_BUS_PHYS_BASE

/* EXPBus location in memory map */
#define EXP_MEMMAP_BOOTMODE     IX_OSAL_IXP400_EXP_BUS_BOOT_PHYS_BASE
#define EXP_MEMMAP_NORMALMODE   IX_OSAL_IXP400_EXP_BUS_PHYS_BASE 

/* EXPBus register address definitions */
#define EXP_TIMING_CS5  (EXPBUS_REGS_LOCATION + 0x14)

#define EXPBUS_STIMULUS_CS      5       /* CS line index*/
#define EXP_TIMING_CS_STIMULUS  EXP_TIMING_CS5 /* CS cfg register address 
						  dedicated to stimulus */

/* EXPBus configuration */
#define EXP_CNFG0       (EXPBUS_REGS_LOCATION + 0x20) /* Configuration 0 */
#define EXP_CNFG1       (EXPBUS_REGS_LOCATION + 0x24) /* Configuration 1 */
#define EXP_CNFG2       (EXPBUS_REGS_LOCATION + 0x28) /* Configuration 2 */

/* EXPBus EXP_TIMING_CSx bit definitions */
#define EXP_TIMING_CS_EN_MASK   0x80000000  /* Bit 31 - Chip select enabled 
					       when set to 1 */
#define EXP_TIMING_CS_T1_MASK   0x30000000  /* Bit 29:28 - T1 Address timing 
					       (0 - normal timing) */
#define EXP_TIMING_CS_T2_MASK   0x0C000000  /* Bit 27:26 - T2 setup phase 
					       timing (0 - timing) */
#define EXP_TIMING_CS_T3_MASK   0x03C00000  /* Bit 25:22 - T3 strobe phase 
					       timing (0 - normal timing) */
#define EXP_TIMING_CS_T4_MASK   0x00300000  /* Bit 21:20 - T4 hold phase 
					       timing (0 - normal timing) */
#define EXP_TIMING_CS_T5_MASK   0x000F0000  /* Bit 19:16 - T5 recovery phase 
					       timing (0 - normal timing) */
#define EXP_TIMING_CS_CNFG_MASK 0x00003C00  /* Bit 13:10 - device mem size = 
					       2^(9+CNFG) : 0 - 2^9 = 512 
					       bytes */
#define EXP_TIMING_CS_WR_MASK   0x00000002  /* Bit 1 - WR_EN - enables writes 
					       when set to 1 */

/* configuration used by stimulus */
#define EXP_TIMING_CFG_STIMULUS  ( EXP_TIMING_CS_EN_MASK | EXP_TIMING_CS_T1_MASK |EXP_TIMING_CS_T2_MASK | EXP_TIMING_CS_T3_MASK | EXP_TIMING_CS_T4_MASK | EXP_TIMING_CS_T5_MASK | EXP_TIMING_CS_CNFG_MASK | EXP_TIMING_CS_WR_MASK )

/* EXPBus EXP_CNFG0 bit definitions Location of EXPBus in memory space: 
 * 1 - 0x0, 0 - 0x50000000*/
#define EXP_CNFG0_MEMMAP_MASK   0x80000000


#define DISCONNECT_RETRY_DELAY     (25)
#define DISCONNECT_RETRY_COUNT     (200)

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

/* function prototype */
PRIVATE IX_STATUS rxFreeChannelGet (unsigned int *channelId);
PRIVATE IX_STATUS txFreeChannelGet (unsigned int *channelId);
PRIVATE IX_STATUS rxChannelFind (IxAtmConnId connId, unsigned int *channelId);
PRIVATE IX_STATUS txChannelFind (IxAtmConnId connId, unsigned int *channelId);
PRIVATE IX_STATUS txVcDisconnect(IxAtmConnId txConnId);
PRIVATE IX_STATUS rxVcDisconnect(IxAtmConnId rxConnId);

/*
 * Function definitions.
 */

PRIVATE IX_STATUS
rxFreeChannelGet (unsigned int *channelId)
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
txFreeChannelGet (unsigned int *channelId)
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
rxChannelFind (IxAtmConnId connId, unsigned int *channelId)
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
txChannelFind (IxAtmConnId connId, unsigned int *channelId)
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

PRIVATE IX_STATUS
txVcDisconnect(IxAtmConnId txConnId)
{
    IX_STATUS retval;
    unsigned int retryCount;

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
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		  "tx disconnect failed (%d)\n", retval, 0, 0, 0, 0, 0);
	return retval;
    }

    return IX_SUCCESS;
}

PRIVATE IX_STATUS
rxVcDisconnect(IxAtmConnId rxConnId)
{
    IX_STATUS retval;
    unsigned int retryCount;

    retval =  ixAtmdAccRxVcDisable(rxConnId);
    if (retval != IX_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		  "rx disable failed (%d)\n", retval, 0, 0, 0, 0, 0);
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
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		  "rx disconnect failed (%d)\n", retval, 0, 0, 0, 0, 0);
	return retval;
    }

    return IX_SUCCESS;
}


IX_STATUS
ixEthAal5AppUtilsAtmVcRegisterConnect (IxAtmLogicalPort port,
				 IxAtmmVc txVc,
				 IxAtmmVc rxVc,
				 IxAtmdAccAalType aalType,
				 IxAtmRxQueueId rxQueueId,
				 IxAtmdAccRxVcRxCallback rxCallback,
				 unsigned int minimumReplenishCount,
				 IxAtmdAccTxVcBufferReturnCallback bufferFreeCallback,
				 IxAtmdAccRxVcFreeLowCallback rxFreeLowCallback,
				 IxAtmdAccUserId userId,
				 IxAtmConnId *rxConnId,
				 IxAtmConnId *txConnId)
{
    IX_STATUS retval;
    unsigned int rxFreeQueueSize;
    unsigned rxChannelIdx;
    unsigned txChannelIdx;

    if (rxFreeChannelGet(&rxChannelIdx) == IX_FAIL)
    {
        ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		  "Failed to get a free Rx channell\n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    if (txFreeChannelGet(&txChannelIdx) == IX_FAIL)
    {
        ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		  "Failed to get a free Tx channell\n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    rxPort[rxChannelIdx] = port;
    txPort[txChannelIdx] = port;

    /* Setup tx VC */
    retval = ixAtmmVcRegister (port,
			       &txVc,
			       &txVcId[txChannelIdx]);

    if (retval != IX_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		  "Failed to register Tx VC \n", 0, 0, 0, 0, 0, 0);
	return retval;
    }

    /* Setup rx VC */
    retval = ixAtmmVcRegister (port,
			       &rxVc,
			       &rxVcId[rxChannelIdx]);

    if (retval != IX_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		  "Failed to register Rx VC \n", 0, 0, 0, 0, 0, 0);
	return retval;
    }

    /* Connect Tx to the Vc */
    retval = ixAtmdAccTxVcConnect (port,
				   txVc.vpi,
				   txVc.vci,
				   aalType,
				   userId,
				   bufferFreeCallback,
				   txConnId);

    if (retval != IX_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		  "Failed to connect to Tx VC \n", 0, 0, 0, 0, 0, 0);
	return retval;
    }

    txChannelInfo[txChannelIdx].connId = *txConnId;
    txChannelInfo[txChannelIdx].port = port;

    /* Connect Rx to the VC */
    retval = ixAtmdAccRxVcConnect (port,
				   rxVc.vpi,
				   rxVc.vci,
				   aalType,
				   rxQueueId,
				   userId,
				   rxCallback,
				   minimumReplenishCount,
				   rxConnId,
				   &rxNpeVcIdTable[rxChannelIdx]);

    if (retval != IX_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		  "Failed to connect to Rx VC \n", 0, 0, 0, 0, 0, 0);
	return retval;
    }

    rxChannelInfo[rxChannelIdx].connId = *rxConnId;
    rxChannelInfo[rxChannelIdx].port = port;

    retval = ixAtmdAccRxVcFreeEntriesQuery(*rxConnId,
                                           &rxFreeQueueSize);
    if (retval != IX_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		  "Failed to get the number of entries in rx free queue\n", 0, 0, 0, 0, 0, 0);
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
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		  "Failed to register RxFreeLowCalback \n", 0, 0, 0, 0, 0, 0);
	return retval;
    }

    /* Register replish more callback */

    retval = ixAtmdAccRxVcEnable (*rxConnId);

    if (retval != IX_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		  "Failed to enable Rx VC \n", 0, 0, 0, 0, 0, 0);
	return retval;
    }

    return IX_SUCCESS;
}

IX_STATUS
ixEthAal5AppUtilsAtmVcUnregisterDisconnect (IxAtmConnId rxConnId, IxAtmConnId txConnId)
{
    IX_STATUS retval = IX_SUCCESS;
    unsigned int rxChannelIdx;
    unsigned int txChannelIdx;

    if (rxChannelFind(rxConnId, &rxChannelIdx) == IX_SUCCESS)
    {
        retval = rxVcDisconnect(rxConnId);
        if (retval != IX_SUCCESS)
        {
            ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		      "Failed to disconnect an rx channel\n", 0, 0, 0, 0, 0, 0);
            return IX_FAIL;
        }

        retval = ixAtmmVcDeregister(rxPort[rxChannelIdx], rxVcId[rxChannelIdx]);
        if (retval != IX_SUCCESS)
        {
            ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		      "Failed to unregister an rx channel\n", 0, 0, 0, 0, 0, 0);
            return IX_FAIL;
        }
        rxChannelInfo[rxChannelIdx].inUse = FALSE;
    }

    if (txChannelFind(txConnId, &txChannelIdx) == IX_SUCCESS)
    {
        retval = txVcDisconnect(txConnId);
        if (retval != IX_SUCCESS)
        {
            ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		      "Failed to disconnect a tx channel\n", 0, 0, 0, 0, 0, 0);
            return IX_FAIL;
        }

        retval = ixAtmmVcDeregister(txPort[txChannelIdx], txVcId[txChannelIdx]);
        if (retval != IX_SUCCESS)
        {
            ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		      "Failed to unregister a tx channel\n", 0, 0, 0, 0, 0, 0);
            return IX_FAIL;
        }
        txChannelInfo[txChannelIdx].inUse = FALSE;
    }

    return IX_SUCCESS;
}

IX_STATUS
ixEthAal5AppUtilsAtmAllVcsDisconnect (void)
{
    IX_STATUS retval;
    int channelIdx;

    for (channelIdx=0; channelIdx<IX_ATM_MAX_NUM_VC; channelIdx++)
    {
        retval = ixEthAal5AppUtilsAtmVcUnregisterDisconnect (rxChannelInfo[channelIdx].connId, 
						       txChannelInfo[channelIdx].connId);
        if (retval != IX_SUCCESS)
        {
	    ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		      "Failed to unregister a channel\n", 0, 0, 0, 0, 0, 0);
	    return IX_FAIL;
        }
    }
    return IX_SUCCESS;
}

