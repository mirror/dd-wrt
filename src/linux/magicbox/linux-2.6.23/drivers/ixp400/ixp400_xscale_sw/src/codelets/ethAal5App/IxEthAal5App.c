/**
 * @file IxEthAal5App
 *
 * @date  June-2002 
 *
 * @brief This file contains the implementation of the IxEthAal5App
 * component.
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

#ifdef __vxworks
#include <end.h>    /* END drivers */
#include <endLib.h> /* END drivers */
#include <sysLib.h> /* system clock */
#endif

#include "IxOsal.h"
#include "IxQMgr.h"
#include "IxNpeDl.h"
#include "IxNpeMh.h"
#include "IxAtmdAcc.h"
#include "IxAtmdAccCtrl.h"
#include "IxAtmm.h"
#include "IxEthDB.h"
#include "IxEthDBPortDefs.h"
#include "IxAdsl.h"
#include "IxFeatureCtrl.h"

#ifdef __wince
#include "pkfuncs.h"
#include "oalintr.h"
#endif

#include "IxEthAal5App.h"
#include "IxEthAal5App_p.h"

#if defined(__wince) && defined(IX_USE_SERCONSOLE)
#include "IxSerConsole.h"
#define printf ixSerPrintf
#define gets ixSerGets
#endif

/* Define IXEAA_DEBUG to enable verbose debugging mode */
#undef IXEAA_DEBUG

/*************************** USEFULL DEFINES *********************************/
#define ATM_CELL_SIZE 53
#define AAL5_PAYLOAD_LENGTH           48
/* Typically mbuffer data is allocated to size 2048 - this is enough to hold 
   largest possible Ethernet frame. Here it is 2064=43*48, the requirement of
   AtmdAcc component is that mLen field of mbuffer is set to n*48. */
#define IX_EAA_DEFAULT_MBUF_DATA_SIZE 2064

/* User space is 8 words at the beginning of mbuffer data. First 4 bytes are 
   used to store information about the port (Eth 0,1 or Atm 0,1,2...) to which 
   buffer belongs. This information speeds up buffer replenishing (recycling). 
   Last 2 bytes hold RFC1483 header used for Ethernet frame encapsulation into 
   ATM packet. Thanks to that inserting of this header into mbuffer data is 
   avoided when eth. frame needs to be encapsulated.
   User data is not visible to ixEthAcc or AtmdAcc components - mbuffer data 
   pointer (mData) points to first byte after user data. */
#define IX_EAA_MBUF_USER_SPACE      (8*sizeof(UINT32))


/*
 * Atm and eth mark are placed on the beginning of buffer data (user space) to 
 * mark that particular buffer belongs to Eth or ATM
 */
#define IX_EAA_ATM_MARK             0x00000100
#define IX_EAA_ETH_MARK             0x00000200
/*
 * Atm or Eth mask are stored in first word of user data location [8:15] (bits), 
 * while VC id occupies location [0:7]
 */
#define IX_EAA_MARK_MASK            0xFF00
#define IX_EAA_ATM_VC_MASK          0x00FF
#define IX_EAA_ETH_PORT_MASK        0x00FF

#define IX_EAA_AAL5_TRAILER_SIZE                    8
#define IX_EAA_AAL5_LENGTH_POS_FROM_END_OF_TRAILER  6
#define IX_EAA_AAL5_TRAILER_FCS_SIZE                4


#define IX_EAA_MIN_ETH_FRAME_SIZE   60
#define IX_EAA_MAX_ETH_FRAME_SIZE   1514

#define IX_EAA_NUM_ETH_PORTS        IXP400_ETH_ACC_MII_MAX_ADDR

#define IX_EAA_ETH_FCS_SIZE         4
/*
 * This is RFC1483 header used for encapsulation Ethernet frames by bridge
 * applications in VcMux mode. Header consist of two bytes set to 0. Refer
 * to RFC 1483 document for more details.
 */
#define IX_EAA_RFC1483_VCMUX_ROUTED_IP_HEADER          0x4500
#define IX_EAA_RFC1483_VCMUX_ROUTED_HEADER_SIZE        14
#define IX_EAA_RFC1483_VCMUX_BRIDGED_HEADER            0x0000
#define IX_EAA_RFC1483_VCMUX_BRIDGED_HEADER_SIZE       2

#define IX_UTOPIA_PHY_ADDR          0

/* Define maximum upload and download rates
   available on utopia port. Because this application doesn't 
   use real time protocols (VBR), those rates doesn't have any 
   effect on real throughput. Moreover real data rates can be 
   limited by type of media connected to Utopia (like Adsl) */
#define IX_EAA_UPLOAD_RATE          800000
#define IX_EAA_DOWNLOAD_RATE        53000000

/* Read comments in atmdAcc.h for IxAtmdAccAPI and minimumReplenishCount to
   fully understand meaning of this value. */
#define IX_EAA_MIN_REPLENISH_COUNT  1

#define IX_EAA_INITIALIZED          1

/**
 * @def NPE_A_IMAGE_ID_SINGLE_PORT_SPHY
 * @brief NPE A image to load for the required features
 *
 * This is an NPE-A image supporting ATM SPHY (single port)
 */
#define NPE_A_IMAGE_ID_SINGLE_PORT_SPHY (IX_NPEDL_NPEIMAGE_NPEA_HSS0_ATM_SPHY_1_PORT)

/**
 * @def NPE_A_IMAGE_ID_SINGLE_PORT_MPHY
 * @brief NPE A image to load for the required features
 *
 * This is an NPE-A image supporting ATM MPHY (single port)
 */
#define NPE_A_IMAGE_ID_SINGLE_PORT_MPHY (IX_NPEDL_NPEIMAGE_NPEA_HSS0_ATM_MPHY_1_PORT)

/**
 * @def NPE_A_IMAGE_ID_MULTI_PORT_MPHY
 * @brief NPE A image to load for the required features
 *
 * This is an NPE-A image supporting ATM MPHY (multi port)
 */
#define NPE_A_IMAGE_ID_MULTI_PORT_MPHY  (IX_NPEDL_NPEIMAGE_NPEA_ATM_MPHY_12_PORT)

/**
 * @def NPE_B_IMAGE_ID
 * @brief NPE B image to load for the required features
 *
 * This is an NPE-B image with Ethernet support
 */
#define NPE_B_IMAGE_ID IX_NPEDL_NPEIMAGE_NPEB_ETH

/**
 * @def NPE_C_IMAGE_ID
 * @brief NPE C image to load for the required features
 *
 * This is an NPE-C image with Ethernet support
 */
#define NPE_C_IMAGE_ID IX_NPEDL_NPEIMAGE_NPEC_ETH

/**
 * @def S
 * @brief Multiplier for @c ixOsalSleep to work in seconds
 */
#define S 1000

/* ADSL line number is 0 */
#define IX_EAA_ADSL_LINE_NUM 0
/* ADSL line type is normal */
#define IX_EAA_ADSL_LINE_TYPE 0
/* ADSL phy type is CPE */
#define IX_EAA_ADSL_PHY_TYPE_CPE 0

/* Thread Priority is set to low for EthAal5AppShowTask */
#define IX_EAA_THREAD_PRI_LOW 140

/* Thread Priority is set to low for ixEAAEthLinkStateMonitorLoop */
#define IX_EAA_LINE_MONITOR_PRI 140

#define PERIPHERAL_BUS_CLOCK     66 /* Preipheral Bus frequency 66MHz */
#define TIMESTAMP_TICKS_PER_MSEC ((PERIPHERAL_BUS_CLOCK) * 1000) /*Time stamp*/
#define MAX_TIME_LIMIT_IN_MSEC   120000 /* 120 sec */
#define IX_EAA_MAX_TIME_SLEEP    15000 /* 15 sec*/
#define IX_EAA_MAX_MAC           8 /* 8 MAC address */

#define IX_EAA_INVALID_PHY_ADDR  0xffffffff /*Invalid PHY Address*/

#define IX_ETHAAL5APP_THREAD_STACK_SIZE 10240 /* Thread stack size */

/* By default, the number of pre-configured ATM VCs. */
#define IX_EAA_NUM_PRE_CONFIGURED_ATM_VCS 8 

/*
 * Typedef declarations global to this file only.
 */

/* Eth Mii Duplex Mode Enumeration */
typedef enum { 
    IX_EAA_UNKNOWN_MODE, 
    IX_EAA_FULL_DUPLEX_MODE, 
    IX_EAA_HALF_DUPLEX_MODE 
} IxEAAEthDuplexMode;

/************************* CONFIGURATION DATA ********************************/
/* All variables below are used during configuration
   phase and are initialized to default values. User can use functions
   described below to change configuration, however it must be done
   before starting application - !!! NO DYNAMIC CONFIGURATION CHANGES ARE
   ALLOWED IN CURRENT VERSION !!! */
static IxEthAccMacAddr macAddr1 = { { 0x00, 0x11, 0x02, 0x03, 0x04, 0x05 } };
static IxEthAccMacAddr macAddr2 = { { 0x00, 0x21, 0x02, 0x03, 0x04, 0x05 } };
static int             atmNumPortsEnabled = IX_EAA_NUM_ATM_PORTS;

/* Initialize phyAddresses to invalid addresses */
static UINT32 phyAddresses[IX_EAA_NUM_ETH_PORTS];

static BOOL routedProtocol = FALSE;

IxAtmmVc             ixEAAAtmTxVc[IX_EAA_NUM_ATM_VCS];
IxAtmmVc             ixEAAAtmRxVc[IX_EAA_NUM_ATM_VCS];

/**********************************************************************/

/* This variable is set to 1, after application is fully initialized */
static int      appState = 0;

/* Phy mode for Utopia interface - this variable can be modified during
   initialization process */
#if IX_UTOPIAMODE == 1
static  IxAtmmPhyMode phyMode = IX_ATMM_SPHY_MODE;
#else
static  IxAtmmPhyMode phyMode = IX_ATMM_MPHY_MODE;
#endif
/* Keeps UTOPIA settings for each port */
IxAtmmPortCfg   portCfgs[IX_EAA_NUM_ATM_PORTS];

/* Keeps connection Id's for each Atm port (each Atm port has one VC opened) */
IxAtmConnId     atmVcRxId[ IX_EAA_NUM_ATM_VCS ];
IxAtmConnId     atmVcTxId[ IX_EAA_NUM_ATM_VCS ];

/* Each Atm VC and each Ethernet port has separate pool of buffers 
   declared here. */
static IX_OSAL_MBUF_POOL *mBufAtmPool[ IX_EAA_NUM_ATM_VCS ];
static IX_OSAL_MBUF_POOL *mBufEthPool[ IX_EAA_NUM_ETH_PORTS ];

/* Each VC (so each Utopia Phy) will be able to learn one Mac address */
IxEthAccMacAddr atmSrcMacAddrDBase[ IX_EAA_NUM_ATM_VCS ];
UINT8 atmMacAddrLookup[ IX_EAA_NUM_ATM_VCS ][2 * IX_IEEE803_MAC_ADDRESS_SIZE];
UINT32 atmPortIdLookup[ IX_EAA_NUM_ATM_VCS ];
/* Each port will be able to remember the last Mac address */
IxEthAccMacAddr ethSrcMacAddrDBase[ IX_EAA_NUM_ETH_PORTS ];

/* array used to decide if an extra cell is needed to build a 
   an AAL5 PDU trailer */
int extraAtmCellNeeded[48] = { 
 1, 0, 0, 0 ,0, 0, 0, 0,
 0, 0, 0, 0 ,0, 0, 0, 0,
 0, 0, 0, 0 ,0, 0, 0, 0,
 0, 0, 0, 0 ,0, 0, 0, 0,
 0, 0, 0, 0 ,0, 0, 0, 0,
 0, 1, 1, 1 ,1, 1, 1, 1
};

/* Flag to set the ShowTask's loop */
BOOL ixEthAal5AppShowTaskLoopEnable = FALSE;

/*************************** STATISTICS DATA *********************************/
/* The following variables are not used in normal usage. To use these variables
* need to specify IX_DEBUG=1 
*/
#ifndef NDEBUG
unsigned int    ixEAANumAtmRxChainedBuffers;
unsigned int    ixEAANumAtmRxMACDroppedBuffers;
unsigned int    ixEAANumAtmRxDropBuffers;
unsigned int    ixEAANumEthRxChainedBuffers;
unsigned int    ixEAANumEthRxMACDroppedBuffers;
unsigned int    ixEAANumEthRxDropBuffers;
#endif

unsigned int    ixEAANumAtmRxEthFramesForwarded;
unsigned int    ixEAANumAtmRxPackets;
unsigned int    ixEAANumEthRxEthFramesForwarded;
unsigned int    ixEAANumEthRxFrames;  

#ifndef __linux /* for vxWorks and WinCE */
static IxOsalThread pollTask;
#endif

/* Eth Mii-MAC Monitoring Task */
static IxOsalThread lineMonitorTask;

static IxQMgrDispatcherFuncPtr dispatcherFunc ;

#ifndef __linux /* for vxWorks and WinCE */
/* IxQMgrDispatcher's sleep duration */
static UINT32 dispatcherSleepDuration = 0;
#endif
  
/*****************************************************************************/

/************************ FUNCTION DECLARATIONS ******************************/
void ixEAAAtmBufferRecycle( IxAtmConnId vcId, IX_OSAL_MBUF * mbufPtr );
void ixEAAAtmRxCallback(IxAtmLogicalPort port,
			IxAtmdAccUserId userId,
                        IxAtmdAccPduStatus status,
                        IxAtmdAccClpStatus clp,
                        IX_OSAL_MBUF * mbufPtr);
void ixEAAAtmTxDoneCallback(IxAtmdAccUserId userId, IX_OSAL_MBUF * mbufPtr);

void ixEAAEthBufferRecycle( IxEthAccPortId portId, IX_OSAL_MBUF * mbufPtr );
void ixEAAEthRxCallback(UINT32 callbackTag, IX_OSAL_MBUF *buffer, IxEthAccPortId portId);
void ixEAAEthTxDoneCallback(UINT32 callbackTag, IX_OSAL_MBUF *buffer);
void ixEthAal5AppShowTask(void);
void ixEAAAddMAC( int vc, 
                  UINT8 mac1, UINT8 mac2, UINT8 mac3,  
                  UINT8 mac4, UINT8 mac5, UINT8 mac6 );
void ixEAAShowMAC(void);
void ixEAABufferInit(IX_OSAL_MBUF *mbufPtr, UINT32 poolId);
void ixEAAAtmRxVcFreeLowCallback(IxAtmdAccUserId userId);
IX_STATUS ixEAAUtopiaInit( void );
IX_STATUS ixEAAATMInit( void );
IX_STATUS ixEAAVCInit( void );
IX_STATUS ixEAAEthNpeInit(void);
IX_STATUS ixEthInit(BOOL speed, BOOL duplex, BOOL autoneg);
IX_STATUS ixEAAEthChannelInit( void );
int ixEAAShow(void);
int ixEAAMain( void );
void ixEAAEthLinkStateMonitorLoop(void);
IX_STATUS ixEAASetupVc(UINT32 numberOfVc, UINT32 numberOfPorts);
#ifdef __vxworks
void ixEAAQDispatcherSleepDurationSet(UINT32 timeInMS);
#endif
/*****************************************************************************/

/*>>>>>>>>>>>>>>>>>>>>>>>> CONFIGURATION FUNCTIONS <<<<<<<<<<<<<<<<<<<<<<<<<<*/
/*
 * This function can be used to add MAC address to Atm data base and assign it
   to particular port. Valid ports are 0-7 */
void ixEAAAddMAC( int vc, 
                  UINT8 mac1, UINT8 mac2, UINT8 mac3,  
                  UINT8 mac4, UINT8 mac5, UINT8 mac6 )
{
    if( vc < 0 || vc >= IX_EAA_NUM_ATM_VCS )
    {
        printf("Invalid Atm VC specified - must be in range 0 to IX_EAA_NUM_ATM_VCS (i.e. %d)\n",
	       IX_EAA_NUM_ATM_VCS);
        return;
    }

    atmSrcMacAddrDBase[vc].macAddress[0] = mac1;
    atmSrcMacAddrDBase[vc].macAddress[1] = mac2;
    atmSrcMacAddrDBase[vc].macAddress[2] = mac3;
    atmSrcMacAddrDBase[vc].macAddress[3] = mac4;
    atmSrcMacAddrDBase[vc].macAddress[4] = mac5;
    atmSrcMacAddrDBase[vc].macAddress[5] = mac6;

    printf("\nVC %d with MAC Address %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n",
	   vc, mac1, mac2, mac3, mac4, mac5, mac6);
}

void ixEAAShowMAC(void)
{
    int vc;
    for (vc = 0 ; vc< IX_EAA_NUM_ATM_VCS ; vc++)
    {
        printf("\nVC %d with MAC Address %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x\n",
            vc,
	    atmSrcMacAddrDBase[vc].macAddress[0],
 	    atmSrcMacAddrDBase[vc].macAddress[1],
	    atmSrcMacAddrDBase[vc].macAddress[2],
	    atmSrcMacAddrDBase[vc].macAddress[3],
	    atmSrcMacAddrDBase[vc].macAddress[4],
	    atmSrcMacAddrDBase[vc].macAddress[5]);
    }

    return;
}

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
/*
 * ixEAABufferInit function initialize the beginning of mbuf data with
 * a poolId (used to replenish the mbuf to its correct VC channel
 * or ethernet port, and the RFC 1483 header placeholder.
 */ 
void ixEAABufferInit(IX_OSAL_MBUF *mbufPtr, UINT32 poolId)
{
    unsigned char *payloadPtr;

    /* initialize first 4 bytes of mData to poolId and
       increment mData pointer (so poolId will be hidden for
       AtmdAcc and ixEthAcc components */
    IX_OSAL_MBUF_POOL_MDATA_RESET(mbufPtr);
    payloadPtr = (unsigned char *)IX_OSAL_MBUF_MDATA(mbufPtr);
    IX_OSAL_WRITE_BE_SHARED_LONG(((UINT32*)IX_OSAL_MBUF_MDATA(mbufPtr)),poolId);
    IX_OSAL_MBUF_MDATA(mbufPtr) += IX_EAA_MBUF_USER_SPACE;
    /* initialize last 2 bytes to RFC1483 header related to VC based 
       multiplexing of Ethernet/802.3 PDUs. */
    IX_OSAL_MBUF_MDATA(mbufPtr) -= IX_EAA_RFC1483_VCMUX_BRIDGED_HEADER_SIZE;
    IX_OSAL_WRITE_BE_SHARED_SHORT(IX_OSAL_MBUF_MDATA(mbufPtr),IX_EAA_RFC1483_VCMUX_BRIDGED_HEADER);
    IX_OSAL_MBUF_MDATA(mbufPtr) += IX_EAA_RFC1483_VCMUX_BRIDGED_HEADER_SIZE;
    /* note that mData is moved by 8 words from original start */
    /* flush the 8 words of user space */
    IX_OSAL_CACHE_FLUSH((UINT32)payloadPtr, IX_EAA_MBUF_USER_SPACE);
    IX_OSAL_CACHE_FLUSH((UINT32)IX_OSAL_MBUF_MDATA(mbufPtr), IX_OSAL_MBUF_MLEN(mbufPtr));
}

/*>>>>>>>>>>> ATMDACC CALLBACK AND BUFFER PROCESSING FUNCTIONS <<<<<<<<<<<<<<*/

/*
 * ixEAAAtmBufferRecycle function returns mbuf pointed by mbufPtr back to 
 * AtmdAcc component free buffer receive queue. Buffer is unchained (and any
 * following buffers in the chain). Length fields and data pointers are reset to 
 * the original state. Then buffer is replenished to the VC associated with vcId
 */ 
void ixEAAAtmBufferRecycle( IxAtmConnId vcId, IX_OSAL_MBUF * mbufPtr )
{
    if( IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr) == NULL)
    {
        IX_OSAL_MBUF_POOL_MDATA_RESET(mbufPtr);
	
	/* invalidate the part of cache possibly used (including aal5 trailer) */
        IX_OSAL_CACHE_INVALIDATE(
	    (UINT32) IX_OSAL_MBUF_MDATA(mbufPtr), 
	    IX_OSAL_MBUF_MLEN(mbufPtr)+ IX_EAA_MBUF_USER_SPACE + 48);
	
        IX_OSAL_MBUF_MDATA(mbufPtr) += IX_EAA_MBUF_USER_SPACE;
	IX_OSAL_MBUF_MDATA(mbufPtr) -= IX_EAA_RFC1483_VCMUX_BRIDGED_HEADER_SIZE;
        IX_OSAL_MBUF_MLEN(mbufPtr) = IX_EAA_DEFAULT_MBUF_DATA_SIZE;
	
        /* Return buffer to AtmdAcc Rx component */
        if (ixAtmdAccRxVcFreeReplenish (vcId, mbufPtr) != IX_SUCCESS)
        {
            ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		      "Failed to pass Rx free buffer to AtmdAcc\n",
                        0, 0, 0, 0, 0, 0);
            /* If buffer replenishing fails, it means AtmdAcc component can not
               accept more free buffers (internal free receive queue is full).
               This should never happened, because this application replenish
               only buffers that were passed to Rx callback. Therefore we assert
               in such situation */
            IX_OSAL_ASSERT(FALSE);
        }
    }
    else
    {
	IX_OSAL_MBUF* mbufCurPtr = mbufPtr;

	/* if buffer is chained, then update statistics, unchain it and replenish */
	if( IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr) )
	    IX_ETHAAL5APP_CODELET_DEBUG_DO(ixEAANumAtmRxChainedBuffers++;);

	/* Unchain, reset and replenish buffers */
	while( mbufCurPtr )
	{
	    mbufPtr = mbufCurPtr;
	    /* Obtain next buffer from the chain */
	    mbufCurPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufCurPtr);
	    /* Reset current buffer */
	    IX_OSAL_MBUF_POOL_MDATA_RESET(mbufPtr);
	    IX_OSAL_MBUF_MDATA(mbufPtr) += IX_EAA_MBUF_USER_SPACE;
	    IX_OSAL_MBUF_MDATA(mbufPtr) -= IX_EAA_RFC1483_VCMUX_BRIDGED_HEADER_SIZE;
	    
	    IX_OSAL_MBUF_MLEN(mbufPtr) = IX_EAA_DEFAULT_MBUF_DATA_SIZE;
	    IX_OSAL_MBUF_PKT_LEN(mbufPtr) = IX_EAA_DEFAULT_MBUF_DATA_SIZE;
	    /* unchain current buffer */
	    IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr) = NULL;
	    IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(mbufPtr) = NULL;
	    /* Return buffer to AtmdAcc Rx component */
	    IX_OSAL_CACHE_INVALIDATE((UINT32) IX_OSAL_MBUF_MDATA(mbufPtr), 
	                             IX_OSAL_MBUF_MLEN(mbufPtr));
	    
	    if (ixAtmdAccRxVcFreeReplenish (vcId, mbufPtr) != IX_SUCCESS)
	    {
		ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
			  "Failed to pass Rx free buffer to AtmdAcc\n",
			    0, 0, 0, 0, 0, 0);
		/* If buffer replenishing fails, it means AtmdAcc component can not
		   accept more free buffers (internal free receive queue is full).
		   This should never happened, because this application replenish
		   only buffers that were passed to Rx callback. Therefore we assert
		   in such situation */
		IX_OSAL_ASSERT(FALSE);
	    }
	}
    }
}

/* ---------------------------------------------------
*/
#ifdef IXEAA_DEBUG
static void
mbufDump (char *s, IX_OSAL_MBUF * mbufPtr)
{
    printf (">> %s >>", s);
    while (mbufPtr)
    {
        unsigned char *ptChar = (unsigned char *)IX_OSAL_MBUF_MDATA(mbufPtr);
        unsigned int len = IX_OSAL_MBUF_MLEN(mbufPtr);
        unsigned int cpt = 0;

        printf ("\nMbuf length %u : ", len);
        while (len--)
        {
            if ((cpt++ % 16) == 0)
            {
               printf("\n%8.8x : ", cpt - 1);
            }
            printf ("%2.2x ", (int) *ptChar++);
        }
        printf ("\n");
        mbufPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr);
    } /* end of while(mbufPtr) */
    printf (">>>>>\n");
}
#endif

#ifdef _DIAB_TOOL
__asm volatile void _pduPrefetch (volatile UINT8* pdu) 
{
% reg pdu;
	pld [pdu];
}
#endif /* #ifdef _DIAB_TOOL */

/* ATM Rx callback - called by AtmdAcc when new rx buffers are available for 
   particular connection */
void ixEAAAtmRxCallback(IxAtmLogicalPort port,
			IxAtmdAccUserId userId,
                        IxAtmdAccPduStatus status,
                        IxAtmdAccClpStatus clp,
                        IX_OSAL_MBUF * mbufPtr)
{
    int             len;
    UINT8*           pdu;
    IxEthAccPortId  ethPortId = 0;
    int             ethLength;
    /* There is no need to invalidate the mbuf payload : invalidate
     * is already done before replenish and there is no cache line
     * in MMU for this mbuf
     * IX_OSAL_CACHE_INVALIDATE((UINT32) IX_OSAL_MBUF_MDATA(mbufPtr), 
     *                              IX_OSAL_MBUF_MLEN(mbufPtr));
     */

#ifndef __wince
    /* preload the last cache line (AAL5 trailer) */
    pdu = IX_OSAL_MBUF_MDATA(mbufPtr);
    pdu += ((IX_OSAL_MBUF_MLEN(mbufPtr) - 
	    IX_EAA_AAL5_LENGTH_POS_FROM_END_OF_TRAILER) & ~31);
#ifdef _DIAB_TOOL
    _pduPrefetch(pdu);
#else
    __asm__ ("pld [%0]\n": : "r" (pdu)); 
#endif /* #ifdef _DIAB_TOOL */

#endif

    pdu = IX_OSAL_MBUF_MDATA(mbufPtr);
    len = IX_OSAL_MBUF_PKT_LEN(mbufPtr);

    ixEAANumAtmRxPackets++;
#ifdef IXEAA_DEBUG
    mbufDump("Atm RX", mbufPtr);
#endif

    /* Verify that pdu is valid 
       and also that buffer is not chained */
    if( status != IX_ATMDACC_AAL5_VALID ||
        IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr) != NULL)
    {
	/* Status IX_ATMDACC_MBUF_RETURN is passed to Rx callback when AtmdAcc is 
	   shutting down VC. In such a case AtmdAcc is returning all client buffers
	   through Rx callback. This application doesn't handle VC connections
	   dynamically (opening/closing VCs at runtime). It means that we can ignore
	   buffers in this situation as they no longer will be used. */

	IX_ETHAAL5APP_CODELET_DEBUG_DO(ixEAANumAtmRxDropBuffers++;);

	if( status == IX_ATMDACC_MBUF_RETURN )
	{
	    return;
	}
	
        ixEAAAtmBufferRecycle( atmVcRxId[ userId ], mbufPtr );
        return;
    }

    if (IX_OSAL_READ_BE_SHARED_SHORT((UINT16*) pdu) == IX_EAA_RFC1483_VCMUX_BRIDGED_HEADER )
    {
	routedProtocol = FALSE;

	/* Because this packet will be sent through Ethernet, therefore RFC1483
	   header must be removed from the start of pdu. This will be done by
	   incrementing mData pointer by 2 */
	IX_OSAL_MBUF_MDATA(mbufPtr) += IX_EAA_RFC1483_VCMUX_BRIDGED_HEADER_SIZE;
	pdu += IX_EAA_RFC1483_VCMUX_BRIDGED_HEADER_SIZE;
	
	/* compare the frame dst/src with the last one received */
	if (!memcmp(atmMacAddrLookup[ userId ],
		    pdu, 
		    2 * IX_IEEE803_MAC_ADDRESS_SIZE ))
	{
	    /* same source, same destination */
	    ethPortId = atmPortIdLookup[ userId ];
	}
	else
	{
	    /* Obtain destination MAC address and check if it is recognized by Ethernet
	       driver (if one of Ethernet ports ever received packet from that address */
	    if( ixEthDBFilteringDatabaseSearch((void*)&ethPortId, (IxEthDBMacAddr*) pdu) !=
		IX_ETH_ACC_SUCCESS )
	    {
		/* IMPORTANT!!! Flooding is not normally supported, what means that
		   back to back configuration (IXP4XX <-ADSL-> IXP4XX) will not transfer
		   any data. However simplified flooding can be easily implemented.
		   Code below is executed only if dest. MAC address on the Eth side
		   was not found. In such a case packet is dropped and mbuffer
		   recycled: ixEAAAtmBufferRecycle( atmVcRxId[ userId ], mbufPtr );
		   Instead of dropping packet it could be forwarded to port Eth1. In
		   such case flooding would be supported on Eth1. To implement
		   that solution code below should be commented (together with 'return;'
		   instruction 15 lines below) and one line should be added:
		   ethPortId = IX_ETH_PORT_1;. So to be precise this if(){...} statement 
		   will look like:
		   if( ixEthDBFilteringDatabaseSearch( &ethPortId, (IxEthDBMacAddr*) pdu) !=
		   IX_ETH_ACC_SUCCESS )
		   {
		   ethPortId = IX_ETH_PORT_1;
		   }
		   This will enable flooding on port Eth1 only. To enable it
		   on another port, ethPortId must be assigned corresponding value.
		*/
		IX_ETHAAL5APP_CODELET_DEBUG_DO(ixEAANumAtmRxMACDroppedBuffers++;);
		/* if destination MAC address is not recognized by eth. driver, then
		   packet will be discarded and buffer recycled */
		ixEAAAtmBufferRecycle( atmVcRxId[ userId ], mbufPtr );
		return;
	    }
	    
	    /* Obtain source MAC and add it to the 'Atm' MAC database
	       Database holds only one Mac address per Vc, so we need to copy
	       source Mac address to the Mac data base array at location pointed
	       by Vc number (userId). Source Mac address follows destinations Mac
	       address in the pdu */
	    memcpy( &atmSrcMacAddrDBase[ userId ],
		    (pdu + IX_IEEE803_MAC_ADDRESS_SIZE), 
		    IX_IEEE803_MAC_ADDRESS_SIZE );

	    /* update the internal lookup */
	    atmPortIdLookup[ userId ] = ethPortId;

	    memcpy( atmMacAddrLookup[ userId ],
		    pdu, 
		    2 * IX_IEEE803_MAC_ADDRESS_SIZE ); 
	}
	
	/* position pdu pointer on the length field in AAL5 CPCS-PDU trailer 
	   (starts 6 bytes from the end of the PDU). Pdu was already incremented
	   by IX_EAA_RFC1483_VCMUX_BRIDGED_HEADER_SIZE, so we need to take it into account
	   here as well */
	pdu += (len - IX_EAA_AAL5_LENGTH_POS_FROM_END_OF_TRAILER) - 
	    IX_EAA_RFC1483_VCMUX_BRIDGED_HEADER_SIZE;
	
	/* obtained eth frame length = PDU length - RFC1483 header size.
	   the PDU length is stored in two bytes, therefore we cast pdu to UINT16 */
	ethLength = IX_OSAL_READ_BE_SHARED_SHORT( (UINT16*) (pdu) ) - IX_EAA_RFC1483_VCMUX_BRIDGED_HEADER_SIZE;
    }
    else if (IX_OSAL_READ_BE_SHARED_SHORT((UINT16*) pdu) == IX_EAA_RFC1483_VCMUX_ROUTED_IP_HEADER )
    {
	routedProtocol = TRUE;
	
	/* position pdu pointer on the length field in AAL5 CPCS-PDU trailer 
	   (starts 6 bytes from the end of the PDU).  */
	pdu += len - IX_EAA_AAL5_LENGTH_POS_FROM_END_OF_TRAILER;
	
	/* obtained eth frame length = PDU length - RFC1483 header size.
	   the PDU length is stored in two bytes, therefore we cast pdu to UINT16 */
       ethLength = IX_OSAL_READ_BE_SHARED_SHORT((UINT16*)pdu);
	
	/* tx to eth port 1 */
	ethPortId = IX_ETH_PORT_1;
	
	/* prepend the MAC addresses and length (802.3 header) */
	IX_OSAL_MBUF_MDATA(mbufPtr) -= IX_EAA_RFC1483_VCMUX_ROUTED_HEADER_SIZE;
	pdu = IX_OSAL_MBUF_MDATA(mbufPtr);
	memcpy(pdu, &ethSrcMacAddrDBase[ethPortId], IX_IEEE803_MAC_ADDRESS_SIZE); 
	pdu += IX_IEEE803_MAC_ADDRESS_SIZE;
	memcpy(pdu, &macAddr1, IX_IEEE803_MAC_ADDRESS_SIZE); 
	pdu += IX_IEEE803_MAC_ADDRESS_SIZE;
      IX_OSAL_WRITE_BE_SHARED_SHORT((UINT16*)pdu,0x800);
	ethLength += IX_EAA_RFC1483_VCMUX_ROUTED_HEADER_SIZE;
    }
    else
    {
        /* drop the packet - this may occur for ICMP-like PDUs or bad-formed packets */
        ixEAAAtmBufferRecycle( atmVcRxId[ userId ], mbufPtr );
        return;
    }
    
    /* check that the ethernet frame length is valid */
    if( ethLength < IX_EAA_MIN_ETH_FRAME_SIZE ||
        ethLength > IX_EAA_MAX_ETH_FRAME_SIZE )
    {
        IX_ETHAAL5APP_CODELET_DEBUG_DO(ixEAANumAtmRxDropBuffers++;);
	ixEAAAtmBufferRecycle( atmVcRxId[ userId ], mbufPtr );
        return;
    }

    /* make mbuffers data length equal to Ethernet frame length and send it
       to appropriate Ethernet port */
    IX_OSAL_MBUF_MLEN(mbufPtr) = ethLength;
    IX_OSAL_MBUF_PKT_LEN(mbufPtr) = ethLength;
    IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(mbufPtr) = NULL;
    
#ifdef IXEAA_DEBUG
    mbufDump("Tx Eth", mbufPtr);
#endif

    IX_OSAL_CACHE_FLUSH((UINT32)IX_OSAL_MBUF_MDATA(mbufPtr), IX_OSAL_MBUF_MLEN(mbufPtr));

    /* submit buffer to Ethernet port */
    if( ixEthAccPortTxFrameSubmit( ethPortId, mbufPtr, 
                                   IX_ETH_ACC_TX_PRIORITY_4 ) != 
        IX_ETH_ACC_SUCCESS )
    {
#ifdef IXEAA_DEBUG
        printf("AtmdAcc - cell was not sent\n");
#endif
        /* Recycle buffer if Ethernet port didn't accept it */
        ixEAAAtmBufferRecycle( atmVcRxId[ userId ], mbufPtr );
    }
    else
    {
        ixEAANumAtmRxEthFramesForwarded++;
    }
}


/* ATM Tx done callback - called by AtmdAcc when new tx buffer was sent and is no
   longer used by atmdAcc */
void ixEAAAtmTxDoneCallback(IxAtmdAccUserId userId, IX_OSAL_MBUF * mbufPtr)
{
    UINT32  bufDestination;

    /* Obtain interface and port id from the start of the buffer data  */
    IX_OSAL_MBUF_POOL_MDATA_RESET(mbufPtr);
    bufDestination = IX_OSAL_READ_BE_SHARED_LONG( (UINT32*) IX_OSAL_MBUF_MDATA(mbufPtr) );

    /* Verify it belongs to Ethernet */
    if( (bufDestination & IX_EAA_MARK_MASK) != IX_EAA_ETH_MARK )
    {
        ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		  "!!!Wrong buffer mark - expected to belong"
                               "to Ethernet interface (buffer is lost)\n",
                    0, 0, 0, 0, 0, 0);
        IX_ETHAAL5APP_CODELET_DEBUG_DO(ixEAANumAtmRxDropBuffers++;);
        return;
    }
    /* Extract port id from the least significant byte of first word of mbuf 
       data */
    bufDestination &= IX_EAA_ETH_PORT_MASK;
    /* Verify port is in valid range */
    if( bufDestination >= IX_EAA_NUM_ETH_PORTS )
    {
        ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		  "!!!Buffer belongs to invalid Ethernet port (buffer is lost)\n",
                    0, 0, 0, 0, 0, 0);
        IX_ETHAAL5APP_CODELET_DEBUG_DO(ixEAANumAtmRxDropBuffers++;);
        return;
    }

    ixEAAEthBufferRecycle( bufDestination, mbufPtr );
}

/* We provide free buffers only, from 'ixEAAEthTxDoneCallback' or from 
   ixEAAAtmRxCallback (as a result of error condition). Basically only buffers
   that were previously returned to rx handler are given to AtmdAcc component
   after processing. We don't have additional pool of buffers */
void ixEAAAtmRxVcFreeLowCallback(IxAtmdAccUserId userId)
{
}
/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/*>>>>>>>>>>>>>> ETH CALLBACK AND BUFFER PROCESSING FUNCTIONS <<<<<<<<<<<<<<<<*/
/*
 * ixEAAEthBufferRecycle function returns mbuf pointed by mbufPtr back to ixEthAcc
 * component free rx buffer queue. Buffer is unchained (and any following
 * buffers in the chain). Length fields and data pointers are reset to the 
 * original state. Then buffer is replenished to the ethernet port associated 
 * with portId. Normally this function is called from ixEAAAtmTxDoneCallback
 * to replenish ethernet buffer after forwarding it to ATM port. It can be
 * called from ixEAAEthRxCallback as a result of error condition (if for 
 * example bufer contains chain of buffers, which application doesn't support.
 */
void ixEAAEthBufferRecycle( IxEthAccPortId portId, IX_OSAL_MBUF * mbufPtr )
{
    if( IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr) == NULL )
    {
	IX_STATUS status;

	IX_OSAL_MBUF_POOL_MDATA_RESET(mbufPtr);

        /* invalidate only the possibly modified part (including aal5 trailer) */
        IX_OSAL_CACHE_INVALIDATE(
	    (UINT32) IX_OSAL_MBUF_MDATA(mbufPtr), 
	    IX_OSAL_MBUF_MLEN(mbufPtr) + IX_EAA_MBUF_USER_SPACE + 48);
        
        IX_OSAL_MBUF_MDATA(mbufPtr) += IX_EAA_MBUF_USER_SPACE;
        IX_OSAL_MBUF_MLEN(mbufPtr) = IX_EAA_DEFAULT_MBUF_DATA_SIZE;
 
        /* Return buffer to ixEthAcc Rx component */
        status = ixEthAccPortRxFreeReplenish( portId, mbufPtr);
        if (status != IX_SUCCESS)
        {
            ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		      "Failed to pass Rx free buffer to ixEthAcc\n",
                        0, 0, 0, 0, 0, 0);
            /* If buffer replenishing fails, it means ixEthAcc component can not
               accept more free buffers (internal free receive queue is full).
               This should never happened, because this application replenish
               only buffers that were passed to Rx callback. Therefore we assert
               in such situation */
            if (IX_ETH_ACC_PORT_UNINITIALIZED==status) 
	    {
              /* This happens when attempts to replenish to queue that its associated Eth NPE
                 is not enabled. If this happens, we return mbuf back to pool */
	      IX_OSAL_MBUF_POOL_PUT(mbufPtr);
            } 
        }
    }
    else
    {
	IX_OSAL_MBUF* mbufNextPtr;
	IX_STATUS status;
	
	/* if buffer is chained, then update statistics, unchain it and replenish */
	if( IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr) != NULL )
	{
	    IX_ETHAAL5APP_CODELET_DEBUG_DO(ixEAANumEthRxChainedBuffers++;);
	}
	
	/* Unchain, reset and replenish buffers */
	while( mbufPtr )
	{
	    /* Obtain next buffer from the chain */
	    mbufNextPtr = IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr);
	    /* Reset current buffer - data is changed by Atm rx callback function
	       to remove RFC1483 header and now it must be set to original value */
	    IX_OSAL_MBUF_POOL_MDATA_RESET(mbufPtr);
	    IX_OSAL_MBUF_MDATA(mbufPtr) += IX_EAA_MBUF_USER_SPACE;
	    
	    IX_OSAL_MBUF_MLEN(mbufPtr) = IX_EAA_DEFAULT_MBUF_DATA_SIZE;
	    IX_OSAL_MBUF_PKT_LEN(mbufPtr) = IX_EAA_DEFAULT_MBUF_DATA_SIZE;
	    /* unchain current buffer */
	    IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr) = NULL;
	    IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(mbufPtr) = NULL;
	    /* Return buffer to ixEthAcc Rx component */
	    IX_OSAL_CACHE_INVALIDATE((UINT32) IX_OSAL_MBUF_MDATA(mbufPtr), 
	                             IX_OSAL_MBUF_MLEN(mbufPtr));
	    
	    status = ixEthAccPortRxFreeReplenish( portId, mbufPtr);
	    if (status != IX_SUCCESS)
	    {
		ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
			  "Failed to pass Rx free buffer to ixEthAcc\n",
			    0, 0, 0, 0, 0, 0);
		/* If buffer replenishing fails, it means ixEthAcc component can not
		   accept more free buffers (internal free receive queue is full).
		   This should never happened, because this application replenish
		   only buffers that were passed to Rx callback. Therefore we assert
		   in such situation */
		if (IX_ETH_ACC_PORT_UNINITIALIZED==status) 
		{
		    /* This happens when attempts to replenish to queue that its associated Eth NPE
		       is not enabled. If this happens, we return mbuf back to pool */
		    IX_OSAL_MBUF_POOL_PUT(mbufPtr);
		} 
	    }
	    /* take the next buffer from the chain and repeat all steps as for
	       current one */
	    mbufPtr = mbufNextPtr;
	}
    }
}

/* Eth Rx callback - called by ixEthAcc when new rx buffers are available for 
   particular port (callbackTag is different for each Eth port) */
void ixEAAEthRxCallback(UINT32 callbackTag, IX_OSAL_MBUF *mbufPtr, IxEthAccPortId portId)
{
    UINT8*           pdu;
    int             ethLength;
    int             numCells;
    int             atmPortCnt;
    int             atmVcCnt = 0;
    IxAtmdAccUserId connVcId;
    UINT16         *pduLengthField;

    /* There is no need to invalidate the mbuf payload : invalidate
     * is already done before replenish and there is no cache line
     * in MMU for this mbuf
     * IX_OSAL_CACHE_INVALIDATE((UINT32) IX_OSAL_MBUF_MDATA(mbufPtr), 
     *                              IX_OSAL_MBUF_MLEN(mbufPtr));
     */

#ifdef IXEAA_DEBUG
      mbufDump("Eth RX", mbufPtr);
#endif

    ixEAANumEthRxFrames++;
    pdu = IX_OSAL_MBUF_MDATA(mbufPtr);

    /* Verify that pdu length is valid, which has to be greater than minimum 
       Ethernet frame size. Verify also that buffer is not chained */
    if( IX_OSAL_MBUF_PKT_LEN(mbufPtr) < IX_EAA_MIN_ETH_FRAME_SIZE || 
        IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr) != NULL )
    {
        if( IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(mbufPtr) != NULL )
                ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
			  "WARNING: Received Ethernet frame"
                                       "stored in chained buffers\n",
                            0, 0, 0, 0, 0, 0);
        else
            ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		      "WARNING: Received Ethernet frame"
                                       "has length less than 64 bytes\n",
                            0, 0, 0, 0, 0, 0);
        /* Frames less than 64 bytes in size and chained ones are not supported */
        ixEAAEthBufferRecycle( callbackTag, mbufPtr );
        return;
    }

    /* The packet will be sent to the atm port 0 */
    if (routedProtocol)
    {
       atmPortCnt = 0;
    }
    else
    {
    /* Check if destination MAC address is present in the 'Atm' 
       Mac database (if one of VC connections received recently packet 
       from that address). The array with source Mac addresses 
       received from Utopia needs to be searched for the occurrence. 'pdu' points
       to the start of buffer, which contains dest. mac address. Start from
       port 0 and increment port until Mac is found or all ports scanned */
	atmVcCnt = 0;
	while (atmVcCnt < IX_EAA_NUM_ATM_VCS &&
	       memcmp( pdu, &atmSrcMacAddrDBase[ atmVcCnt ], 
		       IX_IEEE803_MAC_ADDRESS_SIZE ))
	{
	    atmVcCnt++; 
	}	
    }
    /* if atmPortCnt is less than number of enable ports, then we found right 
       port, otherwise Mac address is unknown and packet will be discarded */
    if( atmVcCnt == IX_EAA_NUM_ATM_VCS )
    {
        /* IMPORTANT!!! Flooding is not normally supported, what means that
           back to back configuration (IX4XX <-ADSL-> IXP4XX) will not transfer
           any data. However simplified flooding can be easily implemented.
           Code below is executed only if dest. MAC address on the ATM side
           was not found. In such a case packet is dropped and mbuffer
           recycled: ixEAAEthBufferRecycle( callbackTag, mbufPtr );
           Instead of dropping packet it could be forwarded to Phy 1 of Utopia
           interface, thus flooding would be supported on Phy 1. To implement
           that solution code below should be commented (together with 'return;'
           instruction 9 lines below) and one line should be added:
           atmPortCnt = 1;. So to be precise this if(){...} statement will look 
           like:
           if( atmPortCnt == atmNumPortsEnabled )
           {
               atmPortCnt = 1;
           }
           This will enable flooding on Phy 1 of Utopia port only. To enable it
           on another Phy, atmPortCnt must be assigned corresponding value.
        */
#ifdef IXEAA_DEBUG
        printf("Unknown Mac...");
#endif
        IX_ETHAAL5APP_CODELET_DEBUG_DO(ixEAANumEthRxMACDroppedBuffers++;);
        /* if destination MAC address is not recognized by eth. driver, then
           packet will be discarded and buffer recycled */
        ixEAAEthBufferRecycle( callbackTag, mbufPtr );
        return;
    }

    /* Obtain VC id using port returned from MAC data base. */
    connVcId = atmVcTxId[ atmVcCnt ];

    if (routedProtocol)
    {
       /* store the last source mac address */
       memcpy(&ethSrcMacAddrDBase[callbackTag], 
           pdu + IX_IEEE803_MAC_ADDRESS_SIZE,
           IX_IEEE803_MAC_ADDRESS_SIZE);

       ethLength = IX_OSAL_MBUF_MLEN(mbufPtr);  /* obtain eth data length */
       /* Calculate length of Ethernet frame with VcMux header. FCS will not be 
       included in encapsulated frame, therefore length of frame must be 
       decremented by 4. header is removed. */
       ethLength -= IX_EAA_ETH_FCS_SIZE; 
       IX_OSAL_MBUF_MDATA(mbufPtr) += IX_EAA_RFC1483_VCMUX_ROUTED_HEADER_SIZE;
       ethLength -= IX_EAA_RFC1483_VCMUX_ROUTED_HEADER_SIZE; 
    }
    else
    {
	/* Append trailer and padding so the total length of pdu
	   fits exactly into ATM cells and trailer is right justified
	   in the last cell by adding pad field (0-47 bytes). */
	ethLength = IX_OSAL_MBUF_MLEN(mbufPtr);  /* obtain eth data length */
	/* Calculate length of Ethernet frame with VcMux header. FCS will not be 
	   included in encapsulated frame, therefore length of frame must be 
	   decremented by 4. RFC1483 header is added (2 bytes). */
	ethLength -= IX_EAA_ETH_FCS_SIZE; 
	ethLength += IX_EAA_RFC1483_VCMUX_BRIDGED_HEADER_SIZE;
	/* Because this packet will be sent through ATM, therefore RFC1483
	   header must be appended at the start of pdu. This will be done by
	   decrementing mData pointer by 2 - header is already placed before 
	   start of buffer */
	IX_OSAL_MBUF_MDATA(mbufPtr) -= IX_EAA_RFC1483_VCMUX_BRIDGED_HEADER_SIZE; 
    }
    
    /* Calculate total number of cells in packet (in two steps) */
    /* Atm packet will also contain Aal5 CPCS-PDU trailer (8 bytes in total) */
    pdu = IX_OSAL_MBUF_MDATA(mbufPtr);
    numCells = (ethLength + 47) / 48 + extraAtmCellNeeded[ethLength % 48];
    /* Since Aal5 CPCS-PDU trailer must be justified at the end of the cell, the
       total length of the packet must be modulus of 48. Calculate number of cells */
    IX_OSAL_MBUF_MLEN(mbufPtr) = numCells * 48;
    IX_OSAL_MBUF_PKT_LEN(mbufPtr) = IX_OSAL_MBUF_MLEN(mbufPtr);
    /* fill length field in AAL5 CPCS-PDU trailer and leave CRC field untouched
       (will be filled by NPE). This code assumes processor is working in
       big endian mode */
    pduLengthField = (UINT16*) (pdu + 
				IX_OSAL_MBUF_MLEN(mbufPtr) - 
				IX_EAA_AAL5_LENGTH_POS_FROM_END_OF_TRAILER);
    IX_OSAL_WRITE_BE_SHARED_SHORT(pduLengthField,(UINT16) ethLength);
    /* fill trailer padding */
    ixOsalMemSet(pdu + ethLength, 0x0, ((UINT8 *)pduLengthField - pdu) - ethLength);

#ifdef IXEAA_DEBUG
    mbufDump("Tx Atm", mbufPtr);
#endif

    /* submit buffer to VC */
    IX_OSAL_CACHE_FLUSH((UINT32)IX_OSAL_MBUF_MDATA(mbufPtr), IX_OSAL_MBUF_MLEN(mbufPtr));

    if( ixAtmdAccTxVcPduSubmit( connVcId, mbufPtr, 0, numCells ) != 
        IX_SUCCESS )
    {
#ifdef IXEAA_DEBUG
        printf("AtmdAcc - cell was not sent\n");
#endif
        /* Recycle buffer if Atm port didn't accept it */
        ixEAAEthBufferRecycle( callbackTag, mbufPtr );
    }
    else
    {
        ixEAANumEthRxEthFramesForwarded++;
    }
}

/* Eth Tx done callback - called by ixEthAcc when new tx buffer was sent and is
    no longer used by ixEthAcc */
void ixEAAEthTxDoneCallback(UINT32 callbackTag, IX_OSAL_MBUF *mbufPtr)
{
    UINT32  bufDestination;

    if (IX_OSAL_MBUF_NET_POOL(mbufPtr) == mBufAtmPool[0])
    {
       /* prioritize traffic on port 0 , recognize the pool id */
	bufDestination = 0;
    }
    else
    {
	/* Obtain interface and port id from the start of the buffer data  */
	IX_OSAL_MBUF_POOL_MDATA_RESET(mbufPtr);
	bufDestination = IX_OSAL_READ_BE_SHARED_LONG( (UINT32*) IX_OSAL_MBUF_MDATA(mbufPtr));
	
	/* Verify it belongs to Ethernet */
	if( (bufDestination & IX_EAA_MARK_MASK) != IX_EAA_ATM_MARK )
	{
	    ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		      "!!!Wrong buffer mark - expected to belong to"
			"Atm interface (buffer is lost)\n",
			0, 0, 0, 0, 0, 0);
	    IX_ETHAAL5APP_CODELET_DEBUG_DO(ixEAANumEthRxDropBuffers++;);
	    return;
	}
	/* Extract port id from the least significant byte of first word of mbuf 
	   data */
	bufDestination &= IX_EAA_ATM_VC_MASK;
	/* Verify port is in valid range */
	if( bufDestination >= IX_EAA_NUM_ATM_VCS )
	{
	    ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		      "!!!Buffer belongs to invalid Atm port"
			"(buffer is lost)\n",
			0, 0, 0, 0, 0, 0);
	    IX_ETHAAL5APP_CODELET_DEBUG_DO(ixEAANumEthRxDropBuffers++;);
	    return;
	}
    }
    
    ixEAAAtmBufferRecycle( atmVcRxId[ bufDestination ], mbufPtr );
}

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


/*>>>>>>>>>>>>>>>>> ATM HARDWARE INITIALIZATION FUNCTIONS <<<<<<<<<<<<<<<<<<<*/
IX_STATUS ixEAAUtopiaInit( void )
{
    int cnt;
    int retval;

    /* Clear portCfgs */
    ixOsalMemSet(portCfgs, 0, sizeof(portCfgs));

    for (cnt = 0; cnt < atmNumPortsEnabled; cnt++)
    {
        portCfgs[ cnt ].UtopiaTxPhyAddr = IX_UTOPIA_PHY_ADDR + cnt;
        portCfgs[ cnt ].UtopiaRxPhyAddr = IX_UTOPIA_PHY_ADDR + cnt;
    }

    /* Initialize Utopia with current config */
    retval = ixAtmmUtopiaInit(atmNumPortsEnabled,
                              phyMode,
                              portCfgs,
                              IX_ATMM_UTOPIA_LOOPBACK_DISABLED);
    
    if (retval != IX_SUCCESS)
    {
        printf ("AtmmUtopia Initialization failed\n");
        return IX_FAIL;
    }

    /* Initialize Ports */
    for (cnt = 0;  cnt < atmNumPortsEnabled;  cnt++)
    {
        /* Last two parameters state maximum upload and download rates
           available on utopia phy, assuming all enabled phys saturate
           their bandwidth. Because this application doesn't use realtime
           protocols (VBR), those rates doesn't have any effect on real
           throughput. Moreover real data rates can be limited by type
           of media connected to Utopia (like Adsl) */
        retval = ixAtmmPortInitialize (cnt,
                                       IX_EAA_UPLOAD_RATE,
                                       IX_EAA_DOWNLOAD_RATE);
        if (retval != IX_SUCCESS)
        {
            printf ("Port Initialization failed for port %d\n", cnt);
            return IX_FAIL;
        }

        /* Enable the port in Atmm (AtmdAcc and AtmSch will be notified) */
        retval = ixAtmmPortEnable(cnt);
        if (retval != IX_SUCCESS)
        {
            printf ("Port Enable failed for port %d\n", cnt);
            return IX_FAIL;
        }
    }

    return IX_SUCCESS;
}

IX_STATUS ixEAAATMInit( void )
{
    int           retval;

    /* Initialize AtmSch */
    retval = ixAtmSchInit();

    if (retval != IX_SUCCESS)
    {
        printf ("AtmSch Initialization failed\n");
        return IX_FAIL;
    }

    /* Initialize AtmdAcc */
    retval = ixAtmdAccInit();

    if (retval != IX_SUCCESS)
    {
        printf ("AtmdAcc Initialization failed\n");
        return IX_FAIL;
    }
    
    printf( "\n");

    /* If not IXP42X A0 stepping, proceed to check for existence of coprocessors */ 
    if ((IX_FEATURE_CTRL_SILICON_TYPE_A0 != 
        (ixFeatureCtrlProductIdRead() & IX_FEATURE_CTRL_SILICON_STEPPING_MASK))
        || (IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X != ixFeatureCtrlDeviceRead ()))
    {
	if ((ixFeatureCtrlComponentCheck(IX_FEATURECTRL_AAL) == 
	     IX_FEATURE_CTRL_COMPONENT_ENABLED) &&
	    (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_UTOPIA) == 
	     IX_FEATURE_CTRL_COMPONENT_ENABLED)	 
	    )
	{
#if IX_UTOPIAMODE == 1
	    retval = ixNpeDlNpeInitAndStart (NPE_A_IMAGE_ID_SINGLE_PORT_SPHY);
#else
	    retval = ixNpeDlNpeInitAndStart (NPE_A_IMAGE_ID_MULTI_PORT_MPHY);
#endif
	}
    }
    else
    {
#if IX_UTOPIAMODE == 1
	retval = ixNpeDlNpeInitAndStart (NPE_A_IMAGE_ID_SINGLE_PORT_SPHY);
#else
	retval = ixNpeDlNpeInitAndStart (NPE_A_IMAGE_ID_MULTI_PORT_MPHY);
#endif
    }
        
    if (retval != IX_SUCCESS)
    {
        printf ("NPE download failed\n");
        return IX_FAIL;
    }

    /* Initialize Atmm */
    retval = ixAtmmInit();
    if (retval != IX_SUCCESS)
    {
        printf ("Atmm Initialization failed\n");
        return IX_FAIL;
    }

    return IX_SUCCESS;
}

/* This function contains the setup for various VC settings
 * with various ATM service. The VC setup is based on the number
 * of VCs and ATM ports */
IX_STATUS ixEAASetupVc(UINT32 numberOfVc, UINT32 numberOfPorts)
{
    UINT32 atmVcCnt = 0;
    UINT32 atmPortCnt = 0;
    UINT32 countCbrVc = 0;
    UINT32 countRtVbrVc = 0;
    IxAtmServiceCategory atmService = IX_ATM_UBR;
    
    const UINT32 atmVpi[ IX_EAA_NUM_PRE_CONFIGURED_ATM_VCS ] =
    { 
	IX_EAA_VC1_VPI, IX_EAA_VC2_VPI, IX_EAA_VC3_VPI, IX_EAA_VC4_VPI,
	IX_EAA_VC5_VPI, IX_EAA_VC6_VPI,IX_EAA_VC7_VPI, IX_EAA_VC8_VPI
    };

    const UINT32 atmVci[ IX_EAA_NUM_PRE_CONFIGURED_ATM_VCS ] =
    { 
	IX_EAA_VC1_VCI, IX_EAA_VC2_VCI, IX_EAA_VC3_VCI, IX_EAA_VC4_VCI,
	IX_EAA_VC5_VCI, IX_EAA_VC6_VCI, IX_EAA_VC7_VCI, IX_EAA_VC8_VCI
    };

    if ((numberOfPorts < 1 || numberOfPorts > IX_UTOPIA_MAX_PORTS) ||
	(numberOfVc < 1 || numberOfVc > IX_EAA_NUM_ATM_VCS))
    {
	return IX_FAIL;
    }

    for (atmVcCnt=0; atmVcCnt < IX_EAA_NUM_ATM_VCS; atmVcCnt++)
    {
	/* Setup for Receive and Transmit VC */
	ixEAAAtmTxVc[atmVcCnt].vpi = 
	    ixEAAAtmRxVc[atmVcCnt].vpi = atmVpi[atmVcCnt];

	ixEAAAtmTxVc[atmVcCnt].vci = 
	    ixEAAAtmRxVc[atmVcCnt].vci = atmVci[atmVcCnt];

	ixEAAAtmTxVc[atmVcCnt].direction = IX_ATMM_VC_DIRECTION_TX;
	ixEAAAtmRxVc[atmVcCnt].direction = IX_ATMM_VC_DIRECTION_RX;

	if (atmVcCnt < 2 )
	{
	    /* Setup for UBR service */
	    atmService = IX_ATM_UBR;

	    ixEAAAtmTxVc[atmVcCnt].trafficDesc.pcr = IX_EAA_DOWNLOAD_RATE;

	    printf ("\nPort = %d VPI = %d, VCI = %d, IX_ATM_UBR\n",
		    atmPortCnt, atmVpi[atmVcCnt],atmVci[atmVcCnt]);
	}
	else if (atmVcCnt >= 2 && atmVcCnt < 4   )
	{
	    /* Setup for CBR service */
	    atmService = IX_ATM_CBR;

	    if (countCbrVc == 0)
	    {
		ixEAAAtmTxVc[atmVcCnt].trafficDesc.pcr = 200 ; 
	    }
	    else
	    {
		ixEAAAtmTxVc[atmVcCnt].trafficDesc.pcr = 300;
	    }
	    countCbrVc++;

            printf ("\nPort = %d VPI = %d, VCI = %d, IX_ATM_CBR\n",
                    atmPortCnt, atmVpi[atmVcCnt],atmVci[atmVcCnt]);
	}
	else if (atmVcCnt >= 4 && atmVcCnt < 6)
	{
	    /* Setup for RTVBR service */
	    atmService = IX_ATM_RTVBR;

	    if (countRtVbrVc == 0)
	    {
		ixEAAAtmTxVc[atmVcCnt].trafficDesc.pcr = 500; 
		ixEAAAtmTxVc[atmVcCnt].trafficDesc.scr = 300;
		ixEAAAtmTxVc[atmVcCnt].trafficDesc.mbs = 5000;
	    }
	    else
	    {
		ixEAAAtmTxVc[atmVcCnt].trafficDesc.pcr = 600;
		ixEAAAtmTxVc[atmVcCnt].trafficDesc.scr = 500;
		ixEAAAtmTxVc[atmVcCnt].trafficDesc.mbs = 6000;
	    }
            countRtVbrVc++;

            printf ("\nPort = %d VPI = %d, VCI = %d, IX_ATM_RTVBR\n",
                    atmPortCnt, atmVpi[atmVcCnt],atmVci[atmVcCnt]);
	}
	else if (atmVcCnt >= 6 && atmVcCnt <8)
	{
	    /* Setup for nrt-VBR*/
	    atmService = IX_ATM_VBR;

	    ixEAAAtmTxVc[atmVcCnt].trafficDesc.pcr = 200; 
	    ixEAAAtmTxVc[atmVcCnt].trafficDesc.scr = 200;
	    ixEAAAtmTxVc[atmVcCnt].trafficDesc.mbs = 0;

            printf ("\nPort = %d VPI = %d, VCI = %d, IX_ATM_VBR\n",
                    atmPortCnt, atmVpi[atmVcCnt],atmVci[atmVcCnt]);

	}

	ixEAAAtmTxVc[atmVcCnt].trafficDesc.atmService =
	    ixEAAAtmRxVc[atmVcCnt].trafficDesc.atmService = atmService;

	atmPortCnt++;
	if (atmPortCnt >= numberOfPorts)
	{
	    atmPortCnt = 0;
	}
    }
    return IX_SUCCESS;
}

IX_STATUS ixEAAVCInit( void )
{
    int portCnt = 0;
    int vcCnt = 0;
    int bufCnt, bufSize;
    int retval;

    /* Setup VC function is responsible to setup VCs depending on the 
     * phy setup with various ATM service*/
    retval = ixEAASetupVc(IX_EAA_NUM_ATM_VCS,IX_EAA_NUM_ATM_PORTS);

    if (retval != IX_SUCCESS)   
    {
	return IX_FAIL;
    }

    /* Initialize ATM VC */
    for (vcCnt = 0;  vcCnt < IX_EAA_NUM_ATM_VCS;  vcCnt++)
    {
	retval = ixEthAal5AppUtilsAtmVcRegisterConnect (
	             portCnt,
		     ixEAAAtmTxVc[vcCnt],
		     ixEAAAtmRxVc[vcCnt],
		     IX_ATMDACC_AAL5,
                     IX_ATM_RX_A,
                     ixEAAAtmRxCallback,
                     IX_EAA_MIN_REPLENISH_COUNT,
                     ixEAAAtmTxDoneCallback,
                     NULL /*ixEAAAtmRxVcFreeLowCallback*/,
                     vcCnt,
                     &atmVcRxId[vcCnt],
                     &atmVcTxId[vcCnt]);
    
	if (retval != IX_SUCCESS)
	{
	    printf ("VC Initialization failed for VC %d\n", 
		    vcCnt);
	    return IX_FAIL;
	}

	/* The port count depends on the PHY mode settings. 
	   If using SPHY, 1 port is enabled else if using MPHY, 
	   8 ports are enabled */ 
	portCnt++;
        if (portCnt >= atmNumPortsEnabled)
        {
            portCnt = 0;
        }

	/* mbuf pool initialisation */
	bufSize = IX_EAA_DEFAULT_MBUF_DATA_SIZE+IX_EAA_MBUF_USER_SPACE;

	mBufAtmPool[vcCnt] = IX_OSAL_MBUF_POOL_INIT(IX_EAA_NUM_BUFFERS_PER_VC,
						    bufSize,
						    "Eth Aal5 Codelet Pool for Atm");

        /* Replenish free buffers to current VC */
        for( bufCnt = 0;  bufCnt < IX_EAA_NUM_BUFFERS_PER_VC;  bufCnt++ )
        {
	    /* get a mbuf from the pool */
	    IX_OSAL_MBUF *mbufPtr;

            mbufPtr = IX_OSAL_MBUF_POOL_GET(mBufAtmPool[vcCnt]);
	    
	    /* initialise the user space part of the mbuf */
	    ixEAABufferInit(mbufPtr, IX_EAA_ATM_MARK | vcCnt);
	    
            /* Replenish bufCnt-th buffer to cnt-th VC (atmVcRxId[ cnt ]).
               Buffer is taken from cnt-th pool from position bufCnt */
            ixEAAAtmBufferRecycle ( atmVcRxId[ vcCnt ], mbufPtr );
#ifdef IXEAA_DEBUG
	    printf("Atm vcCnt = %d. Buf:%p->%p\n", vcCnt+1, mbufPtr, IX_OSAL_MBUF_MDATA(mbufPtr));
#endif
        }
    }

    return IX_SUCCESS;
}

/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/

/*>>>>>>>>>>>>>>>>> ETH HARDWARE INITIALIZATION FUNCTIONS <<<<<<<<<<<<<<<<<<<*/

IX_STATUS ixEAAEthNpeInit(void)
{

    /* If not IXP42X A0 stepping, proceed to check for existence of coprocessors */ 
    if ((IX_FEATURE_CTRL_SILICON_TYPE_A0 != 
        (ixFeatureCtrlProductIdRead() & IX_FEATURE_CTRL_SILICON_STEPPING_MASK))
        || (IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X != ixFeatureCtrlDeviceRead ()))
    {
	/* When Silicon Type is B0, only initialize and start NPE if
	 * it is available.  This codelet supports silicon with single Eth NPE.  
         */
	if (IX_FEATURE_CTRL_COMPONENT_ENABLED == 
	    ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH0))
	{
	    if(ixNpeDlNpeInitAndStart(NPE_B_IMAGE_ID) != IX_SUCCESS)
	    {
		printf("Error initialising NPE B!\n");
		return IX_FAIL;
	    }
	}
	
	if (IX_FEATURE_CTRL_COMPONENT_ENABLED == 
	    ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH1))
	{	    
	    if(ixNpeDlNpeInitAndStart(NPE_C_IMAGE_ID) != IX_SUCCESS)
	    {
		printf("Error initialising NPE C!\n");
		return IX_FAIL;
	    }
	}
    }
    else
    {
	if(ixNpeDlNpeInitAndStart(NPE_B_IMAGE_ID) != IX_SUCCESS)
	{
	    printf("Error initialising NPE B!\n");
	    return IX_FAIL;
	}
	
	if(ixNpeDlNpeInitAndStart(NPE_C_IMAGE_ID) != IX_SUCCESS)
	{
	    printf("Error initialising NPE C!\n");
	    return IX_FAIL;
	}
    }


    return IX_SUCCESS;
}

IX_STATUS ixEthInit(BOOL speed, BOOL duplex, BOOL autoneg)
{
    BOOL   getLink,
           getSpeed,
           getDuplex,
           getAutoneg;
    BOOL   phyPresent[IX_EAA_NUM_ETH_PORTS];
    UINT32 i;
    UINT32 j;
    IxOsalThreadAttr threadAttr; 

    ixOsalMemSet(phyPresent, 0, sizeof(phyPresent));

    if( ixEAAEthNpeInit() )
        return IX_FAIL;

    /*Initialize Ethernet Access component*/
    if(ixEthAccInit()!=IX_ETH_ACC_SUCCESS)
    {
        printf("ixEthAccInit failed\n");
        return IX_FAIL;
    }

    /* Initialize both ports */
    printf("Initializing port 1...");
    if(ixEthAccPortInit(IX_ETH_PORT_1)!=IX_ETH_ACC_SUCCESS)
    {
        printf("ixEthAccPortInit failed for port 1\n");
        return IX_FAIL;
    }
    printf("Succeeded\n");
    printf("Initializing port 2...");
    if(ixEthAccPortInit(IX_ETH_PORT_2)!=IX_ETH_ACC_SUCCESS)
    {
        printf("ixEthAccPortInit failed for port 2\n");
        return IX_FAIL;
    }
    printf("Succeeded\n");
    
    printf("Scanning for Ethernet PHYs...");
    if(ixEthAccMiiPhyScan(phyPresent) == IX_FAIL)
    {	
        printf("ixEthAccMiiPhyScan Failed\n");
        return IX_FAIL;
    }   
    else
    {
        printf("Phy init Succeeded\n");
        printf("PHYs discovered at addresses: ");
        j=0;
        for(i=0;i<IX_EAA_NUM_ETH_PORTS;i++)
        {
            if(phyPresent[i] == TRUE)
            {
                printf("%d ",i);
                phyAddresses[j]=i;
                ++j;
            }
        }
        printf("\n");
    }
    
    for(i=0;i<IX_EAA_NUM_ETH_PORTS; i++)
    {
	/* Reset each phy */
#ifdef IXEAA_DEBUG
        printf("Resetting PHY %d address %d\n", i, phyAddresses[i]);
#endif
        ixEthAccMiiPhyReset(phyAddresses[i]);

	/* Set each phy's config - speed, duplex, autonegotiation mode */
#ifdef IXEAA_DEBUG
        printf("Configuring PHY %d address:%d\n", i,phyAddresses[i]);
#endif
        ixEthAccMiiPhyConfig(phyAddresses[i],
                     speed, 
                     duplex, 
                     autoneg);
    }

    for(i=0;i<IX_EAA_NUM_ETH_PORTS; i++)
    {
	unsigned int retry = 20;

	if (IX_EAA_INVALID_PHY_ADDR == phyAddresses[i])
	    continue;

        printf("Get link Status PHY %d address:%d\n", i,phyAddresses[i]);
	do
	{
	    ixOsalSleep(100);
	    ixEthAccMiiLinkStatus(phyAddresses[i], 
				  &getLink, 
				  &getDuplex, 
				  &getSpeed, 
				  &getAutoneg);
	}
	while (retry-- > 0 && getLink == FALSE);
        ixEthAccMiiShow(phyAddresses[i]);	
    }

    /* obtain current link status for each phy - in case autonegotiation was used
       set the proper duplex mode */
    for(i=IX_ETH_PORT_1;i<=IX_ETH_PORT_2;i++)
    {
#ifdef IXEAA_DEBUG
        printf("Set port %d mode\n", i);
#endif
	ixEthAccMiiLinkStatus(phyAddresses[i], &getLink, &getDuplex, 
			                  &getSpeed, &getAutoneg);
	if (getLink != TRUE)
	{
          /* ensure the default duplex mode is as required */
          getDuplex = duplex;
        }
	if(getDuplex)
	{
	   ixEthAccPortDuplexModeSet (i, IX_ETH_ACC_FULL_DUPLEX);
        }
        else
        {
           ixEthAccPortDuplexModeSet (i, IX_ETH_ACC_HALF_DUPLEX);
	}
        /* additional small delay between the MAC settings */
        ixOsalSleep(100);
    }

#ifdef IXEAA_DEBUG
    printf("MII PHY configuration complete\n");
#endif

    /* Enable generation of FCS on both ports */
    if(ixEthAccPortTxFrameAppendFCSEnable (IX_ETH_PORT_1)!=IX_ETH_ACC_SUCCESS)
    {
        printf("Error enabling FCS on port 1\n");
        return IX_FAIL;
    }
    if(ixEthAccPortTxFrameAppendFCSEnable (IX_ETH_PORT_2)!=IX_ETH_ACC_SUCCESS)
    {
        printf("Error enabling FCS on port 2\n");
        return IX_FAIL;
    }

#ifdef IXEAA_DEBUG
    printf("MAC setup\n");
#endif

    /* Set default MAC addresses to both ports. They are defined at the top of
       this file. */
    if(ixEthAccPortUnicastMacAddressSet (IX_ETH_PORT_1, &macAddr1) !=IX_ETH_ACC_SUCCESS)
    {
        printf("Error setting mac on port 1\n");
        return IX_FAIL;
    }
    printf("PHY 0 MAC address is:\t");
    ixEthAccPortUnicastAddressShow(IX_ETH_PORT_1);

    if(ixEthAccPortUnicastMacAddressSet (IX_ETH_PORT_2, &macAddr2) !=IX_ETH_ACC_SUCCESS)
    {
        printf("Error setting mac on port 2\n");
        return IX_FAIL;
    }
    printf("PHY 1 MAC address is:\t");
    ixEthAccPortUnicastAddressShow(IX_ETH_PORT_2);

    /* Start a Ethernet Link Monitor task to keep Phy and MAC in sychronization */

    threadAttr.name = "LinkStateMonitor";
    threadAttr.stackSize = IX_ETHAAL5APP_THREAD_STACK_SIZE;
    threadAttr.priority = IX_EAA_LINE_MONITOR_PRI;

    /* Create thread for LinkStateMonitorLoop */
    if (IX_SUCCESS != ixOsalThreadCreate(
	&lineMonitorTask,
	&threadAttr,
	(IxOsalVoidFnVoidPtr)ixEAAEthLinkStateMonitorLoop,
	NULL))
    { 
 	printf ("Error spawning ixEAAEthLinkStateMonitorLoop task\n"); 
 	return IX_FAIL; 
    }

    /* Starting thread for LinkStateMonitorLoop */
    if (ixOsalThreadStart(&lineMonitorTask) != IX_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		  "Error starting thread ixEAAEthLinkStateMonitorLoop task\n",
		  0, 0, 0, 0, 0, 0); 
	return IX_FAIL;
    }
    return IX_SUCCESS;
}

IX_STATUS ixEAAEthChannelInit( void )
{
    int bufCnt;
    int bufSize;
    int cnt;

    /* Register Callbacks in ixEthAcc for obtaining 'Tx done' buffers */
    if( ixEthAccPortTxDoneCallbackRegister(IX_ETH_PORT_1,
					   ixEAAEthTxDoneCallback, 0) !=
        IX_ETH_ACC_SUCCESS)
    {
	printf("Failed to register Tx done callback for port 1\n");
        return IX_FAIL;
    }
    if( ixEthAccPortTxDoneCallbackRegister(IX_ETH_PORT_2,
                                           ixEAAEthTxDoneCallback, 1) !=
        IX_ETH_ACC_SUCCESS)
    {
	printf("Failed to register Tx done callback for port 2\n");
        return IX_FAIL;
    }
    /* We doesn't have any scheduling constraints... */
    ixEthAccTxSchedulingDisciplineSet( IX_ETH_PORT_1,FIFO_NO_PRIORITY );
    ixEthAccTxSchedulingDisciplineSet( IX_ETH_PORT_2,FIFO_NO_PRIORITY );

    /* Register Callbacks in ixEthAcc for receiving Rx buffers */
    if( ixEthAccPortRxCallbackRegister(IX_ETH_PORT_1,
				       (IxEthAccPortRxCallback)ixEAAEthRxCallback, 
				       0) !=
        IX_ETH_ACC_SUCCESS)
    {
	    printf("Failed to register Rx callback for port 1\n");
        return IX_FAIL;
    }
    if( ixEthAccPortRxCallbackRegister(IX_ETH_PORT_2,
				       (IxEthAccPortRxCallback)ixEAAEthRxCallback, 
				       1) !=
       IX_ETH_ACC_SUCCESS)
    {
	    printf("Failed to register Rx callback for port 2\n");
        return IX_FAIL;
    }

    /* Enable both ports */
    printf("Enabling port 1\n");
    if(ixEthAccPortEnable(IX_ETH_PORT_1) != IX_ETH_ACC_SUCCESS)
    {
	printf("ixEthAccPortEnable failed for port 1\n");
        return IX_FAIL;
    }
    printf("Enabling port 2\n");
    if(ixEthAccPortEnable(IX_ETH_PORT_2) != IX_ETH_ACC_SUCCESS)
    {
	printf("ixEthAccPortEnable failed for port 2\n");
	return IX_FAIL;
    }

    /* Replenish free buffers to both ports */
    for (cnt = IX_ETH_PORT_1; cnt <= IX_ETH_PORT_2; cnt++)
    {
	/* nbuf pool initialisation */
	bufSize = IX_EAA_DEFAULT_MBUF_DATA_SIZE+IX_EAA_MBUF_USER_SPACE;

	mBufEthPool[cnt] = IX_OSAL_MBUF_POOL_INIT(IX_EAA_NUM_BUFFERS_PER_ETH, 
                          bufSize,
                          "Eth Aal5 Codelet Pool for Eth");

	for( bufCnt = 0;  bufCnt < IX_EAA_NUM_BUFFERS_PER_ETH;  bufCnt++ )
	{
	    /* get a mbuf from the pool */
	    IX_OSAL_MBUF *mbufPtr;

            mbufPtr = IX_OSAL_MBUF_POOL_GET(mBufEthPool[cnt]);
	
	    /* initialise the user space part of the mbuf */
	    ixEAABufferInit(mbufPtr, IX_EAA_ETH_MARK | cnt);

	    /* recycle the mbuf */
	    ixEAAEthBufferRecycle( cnt, mbufPtr);
#ifdef IXEAA_DEBUG
	    printf("Eth %d. Buf:%p->%p\n", cnt+1, mbufPtr, IX_OSAL_MBUF_MDATA(mbufPtr));
#endif
	}
    }

    return IX_SUCCESS;
}

#ifndef __linux /* i.e. VxWorks* & WinCE */
/* Below we have continuous dispatching loop. Only tasks with higher 
   priority can preempt that loop. Dispatching loop guarantees faster
   buffer processing than dispatching through interrupts, however 
   Tasks with lower priority will be blocked until the loop works.
   For that reason current thread is assigned relatively low priority
   (lower than all vxWorks threads) to avoid blocking them. */
static IX_STATUS ixEAAPollTask(void *arg, void **ptrRetObj)
{
    while(1)
    {
        /* We need to dispatch only queues from lower group: serving
          'Rx' and 'TxDone' for ixEthAcc and AtmdAcc. Queue manager
          consist of upper and lower group, containing each
          32 queues. */

        (*dispatcherFunc) (IX_QMGR_QUELOW_GROUP);

        ixOsalSleep(dispatcherSleepDuration);

    }
   *ptrRetObj = NULL;
   return IX_SUCCESS;
}

#ifdef __vxworks
void ixEAAQDispatcherSleepDurationSet(UINT32 timeInMS)
{
    UINT32 sysClkRate;

    /* Get number of ticks per second*/
    sysClkRate = sysClkRateGet();

    /* 
     *  Make sure resolution of OS system clock  
     *  can support minimum sleep time duration.
     *
     *  See explanation below:
     *
     *   Resolution of OS system clock, R = sysClkRateGet() (ticks per second)
     *   So 1/R = timing interval of OS System Clock
     *   Sleep time duration, T = timeInMS  (millisecond)  
     *
     *       Minimum of T = timing interval of OS System Clock
     *       Minimum of T = 1/R * 1000 (millisecond)
     *                 T >= 1000/R 
     *             R * T >= 1000        
     *
     */ 

    /* If resolution is not sufficient for 1ms interval, 
     * i.e. R*T < 1000 
     */
    if( sysClkRate * timeInMS < 1000 )
    {
	/* Set to highest resolution in system clock */
	sysClkRateSet(1000);
    }
    dispatcherSleepDuration = timeInMS;
}
#endif /* #ifdef __vxworks */
#endif /* #ifndef __linux */

/* display the current counters */
int ixEAAShow(void)
{
    IX_ETHAAL5APP_CODELET_DEBUG_DO(
	printf("ixEAANumAtmRxDropBuffers .......... : %10u (should be 0)\n", ixEAANumAtmRxDropBuffers);
	printf("ixEAANumAtmRxChainedBuffers ....... : %10u (should be 0)\n", ixEAANumAtmRxChainedBuffers);
	printf("ixEAANumAtmRxMACDroppedBuffers .... : %10u (should be 0)\n", ixEAANumAtmRxMACDroppedBuffers);
    );
    
        printf("ixEAANumAtmRxEthFramesForwarded ... : %10u\n", ixEAANumAtmRxEthFramesForwarded);
	printf("ixEAANumAtmRxPackets .............. : %10u\n", ixEAANumAtmRxPackets);

    IX_ETHAAL5APP_CODELET_DEBUG_DO(
	printf("ixEAANumEthRxDropBuffers .......... : %10u (should be 0)\n", ixEAANumEthRxDropBuffers);
	printf("ixEAANumEthRxChainedBuffers ....... : %10u (should be 0)\n", ixEAANumEthRxChainedBuffers);
	printf("ixEAANumEthRxMACDroppedBuffers .... : %10u (should be 0)\n", ixEAANumEthRxMACDroppedBuffers);
    );

	printf("ixEAANumEthRxEthFramesForwarded ... : %10u\n", ixEAANumEthRxEthFramesForwarded);
	printf("ixEAANumEthRxFrames ............... : %10u\n", ixEAANumEthRxFrames);
	printf("\n");

        return IX_SUCCESS;
}

void ixEAAEthDBRecordsShow(void)
{
    printf ("\nEthernet Database Show Records\n");
    if (IX_ETH_DB_SUCCESS != ixEthDBFilteringDatabaseShowRecords(IX_ETH_DB_ALL_PORTS,IX_ETH_DB_ALL_RECORD_TYPES))
    {
        printf ("\nUnable to show Records\n");
    }

    return;
}

/*
 * The main function that performs initialization of the EthAal5App codelet.
 *
 * Initialize MAC data base, to be an invalid address (i.e. contain 0xffs).
 * Also initialize QMGR, NpeMh, Eth phys - 100Mbit, FULL DUPLEX, 
 * NO AUTONEGOTIATION (User can change those settings accordingly to required 
 * configuration), ATM, and Utopia interface
 *
 * If Linux* is used, use interrupt mode which is  much faster under Linux* than polling
 *
 * If vxWorks is used, use poll mode which is much faster under vxWorks than 
 * interrupts and start background QMgr queues poll
 */
int ixEAAMain( void )
{
    UINT32 invalAddr = IX_EAA_INVALID_PHY_ADDR;

#ifndef __linux
    IxOsalThreadAttr threadAttr; 
#endif

    /* Initialize MAC data base, to contain 0xffs - no valid Mac addresses 
       present */
    ixOsalMemSet( atmSrcMacAddrDBase, 0xff, IX_IEEE803_MAC_ADDRESS_SIZE);
    ixOsalMemSet( ethSrcMacAddrDBase, 0xff, IX_IEEE803_MAC_ADDRESS_SIZE);

    /* Initialize phyAddeses -- to invalid value */
    ixOsalMemSet(phyAddresses, invalAddr, sizeof(phyAddresses));

    /* Initialize statistics */
    IX_ETHAAL5APP_CODELET_DEBUG_DO(
	/* Statistics for ATM */
	ixEAANumAtmRxDropBuffers = 0;
	ixEAANumAtmRxChainedBuffers = 0;
	ixEAANumAtmRxMACDroppedBuffers = 0;

	/* Statistics for Eth */
	ixEAANumEthRxDropBuffers = 0;
	ixEAANumEthRxChainedBuffers = 0;
	ixEAANumEthRxMACDroppedBuffers = 0;
	);

    ixEAANumAtmRxEthFramesForwarded = 0;
    ixEAANumAtmRxPackets = 0;
    ixEAANumEthRxEthFramesForwarded = 0;
    ixEAANumEthRxFrames = 0;


#ifdef __vxworks
    /* The codelet cannot run if the ixe drivers 
     * is already running
     */
    if (endFindByName ("ixe", 0) != NULL)
    {
      printf("FAIL : Driver ixe0 detected\n");
      return IX_FAIL;
    }
    if (endFindByName ("ixe", 1) != NULL)
    {
      printf("FAIL : Driver ixe1 detected\n");
      return IX_FAIL;
    }
#endif

    /* Initialize QMGR */
    printf( "Initialize QManager...");

    if (ixQMgrInit() != IX_SUCCESS)
    {
        printf ("QMGR Initialization failed\n");
        return IX_FAIL;
    }
    printf( "Successful\n");

    printf( "Initialize NPE...");
    /* Initialize NpeMh */
	if (ixNpeMhInitialize(IX_NPEMH_NPEINTERRUPTS_YES) != IX_SUCCESS)
	{
	    printf ("NpeMh Initialization failed\n");
	    return IX_FAIL;
	}
    printf( "Successful\n");

    /* Initialize ATM */
    printf( "Initialize ATM stack...");
    if( ixEAAATMInit( ) )
        return IX_FAIL;
    printf( "Successful\n");

    /* initialize Utopia interface */
    printf( "Initialize Utopia...");
    if( ixEAAUtopiaInit( ) )
        return IX_FAIL;
    printf( "Successful\n");

    /* Initialize Eth phys - 100Mbit, FULL DUPLEX, AUTONEGOTIATION.
       User can change those settings accordingly to required configuration */
    if( ixEthInit(TRUE, TRUE, TRUE) )
        return IX_FAIL;
    if( ixEAAEthChannelInit( ) )
        return IX_FAIL;
    printf( "Successful\n");

    printf( "Start VC connections...");
    if( ixEAAVCInit( ) )
        return IX_FAIL;
    printf( "Successful\n");

    /* Mark that application was successfully initialized */
    appState = IX_EAA_INITIALIZED;

    /* Obtain the correct QMgr Dispatcher */
    ixQMgrDispatcherLoopGet(&dispatcherFunc);

#ifdef __linux

    /* use interrupt mode - much faster under Linux* than polling */
    printf( "Initialize QManager interrupts ...");

        /* We need to dispatch only queues from lower group: serving
          'Rx' and 'TxDone' for ixEthAcc and AtmdAcc. Queue manager
          consist of upper and lower group, containing each
          32 queues. */
    if (IX_SUCCESS != ixOsalIrqBind(IX_OSAL_IXP400_QM1_IRQ_LVL,
				    (IxOsalVoidFnVoidPtr)dispatcherFunc,
                             (void *)IX_QMGR_QUELOW_GROUP))
    {
        return IX_FAIL;
    }
    
    printf( "Successful\n");

#else /* for vxWorks and WinCE */

    /* use poll mode - much faster under vxWorks than interrupts */
    printf( "Initialize QManager task ...");
    /* start background QMgr queues poll */

    threadAttr.name = "PollTask";
    threadAttr.stackSize = IX_ETHAAL5APP_THREAD_STACK_SIZE;
    threadAttr.priority = IX_ETHAAL5APP_THREAD_PRI_MEDIUM;

    /* Create thread for PollTask */
    if (IX_SUCCESS != ixOsalThreadCreate(
	&pollTask,                          
	&threadAttr,                        
	(IxOsalVoidFnVoidPtr)ixEAAPollTask,
	NULL))
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		  "Failed to create thread for pollTask\n",
		  0, 0, 0, 0, 0, 0);
	return IX_FAIL;
    }

    /* Starting thread for PollTask */
    if (ixOsalThreadStart(&pollTask) != IX_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		  "Failed to start thread for pollTask\n",
		  0, 0, 0, 0, 0, 0);
	return IX_FAIL;
    }

    printf( "Successful starting ixEAAMain\n");

#endif

    return IX_SUCCESS;
}

/* EthAal5App show task */
void
ixEthAal5AppShowTask(void)
{
    ixEthAal5AppShowTaskLoopEnable = TRUE;

    while (ixEthAal5AppShowTaskLoopEnable)
    {
	/* Sleep for 15 seconds, i.e. allow a task switch */
	ixOsalSleep (IX_EAA_MAX_TIME_SLEEP);
	ixEAAShow();
    }
}
/* This function is used by linux when removing module. The flag is used 
to set the while loop in ixEthAal5AppShowTask() function to false */
void 
ixEthAal5AppShowTaskDisable(void)
{
    ixEthAal5AppShowTaskLoopEnable = FALSE;
    ixOsalSleep(10); /* Sleep for 10ms, i.e. allow a task switch */
}

PUBLIC IX_STATUS
ixEthAal5AppCodeletMain(IxEAAModeType modeType)
{
    IxOsalThreadAttr threadAttr; 
    IxOsalThread tid;
    UINT32 startTime;
    UINT32 stopTime;
    UINT32 diffTime;
    UINT32 vcCnt;
    UINT32 nextMac;
    IxAdslLineState adslLineState;

    /* Only checks the availability of VOICE/WAN NPE (UTOPIA & AAL Coprocessor) 
     * and Eth NPE-A if silicon type is B0.  
     */
    /* If not IXP42X A0 stepping, proceed to check for existence of coprocessors */ 
    if ((IX_FEATURE_CTRL_SILICON_TYPE_A0 != 
        (ixFeatureCtrlProductIdRead() & IX_FEATURE_CTRL_SILICON_STEPPING_MASK))
        || (IX_FEATURE_CTRL_DEVICE_TYPE_IXP42X != ixFeatureCtrlDeviceRead ()))
    {
	if ((ixFeatureCtrlComponentCheck(IX_FEATURECTRL_AAL) != 
	     IX_FEATURE_CTRL_COMPONENT_ENABLED) ||
	    (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_UTOPIA) != 
	     IX_FEATURE_CTRL_COMPONENT_ENABLED) ||
	    (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH0) != 
	     IX_FEATURE_CTRL_COMPONENT_ENABLED)	 
	    )
	{
	    printf("FAIL: EthAal5App Codelet requires WAN/VOICE NPE (UTOPIA & AAL Coprocessor) & Eth NPE-A.");	 
	    return IX_FAIL;
	}
	
	/* Warns user if Eth NPE-B is unavailable, then no traffic activity is supported */
	if ((ixFeatureCtrlComponentCheck(IX_FEATURECTRL_ETH1) != 
	     IX_FEATURE_CTRL_COMPONENT_ENABLED))
	{
	    printf("WARNING: EthAal5App Codelet detects Eth NPE-B unavailable. Therefore, no traffic activity is supported in Ethernet Port 1.");
	}     
    }
        
    if (modeType < IX_EAA_UTOPIA_MODE || modeType > IX_EAA_ADSL_MODE)
    {
	printf("\nUsage :");
	printf("\n ixEthAal5AppCodeletMain(modeType)");
	printf("\n");	    
	printf("\n  modeType : 1 = Utopia");
	printf("\n             2 = ADSL");
	printf("\n");
	return IX_FAIL;
    }

    /* Main function to start and initialise the EthAal5App */
    if (IX_SUCCESS != ixEAAMain())
    {
	printf("Fail to run EthAal5App codelet");
	return IX_FAIL;
    }


    printf("Adding MAC address for %d VC for %d port...\n", 
	    IX_EAA_NUM_ATM_VCS, IX_EAA_NUM_ATM_PORTS);

    /* Adding MAC address for each VC into the local database */
    for (vcCnt = 0, nextMac = 0; vcCnt < IX_EAA_NUM_ATM_VCS; vcCnt++, nextMac++)
    {
	ixEAAAddMAC(vcCnt,
		    IX_EAA_MAC1,
		    IX_EAA_MAC2,
		    IX_EAA_MAC3,
		    IX_EAA_MAC4,
		    IX_EAA_MAC5,
		    IX_EAA_MAC6 + nextMac);
    }

    printf("Successful\n");
    
    if (modeType == IX_EAA_ADSL_MODE)
    {
	/* Open ADSL Line */
	printf("Opening ADSL line...");
	if (IX_SUCCESS != ixAdslLineOpen(IX_EAA_ADSL_LINE_NUM, 
					  IX_EAA_ADSL_LINE_TYPE, 
					  IX_EAA_ADSL_PHY_TYPE_CPE))
	{
	    printf("Fail to open ADSL line\n");
	    return IX_FAIL;
	}
	printf("Successful\n");
	
        startTime = ixOsalTimestampGet();

	do 
	{
	    /* Get line state */
	    adslLineState = ixAdslLineStateGet(0);

            stopTime = ixOsalTimestampGet();

            /* Check if the timer wrap over */
            if (stopTime < startTime)
            {
                diffTime = (0xffffffff -  stopTime + startTime);
            }
            else
            {
                diffTime = stopTime-startTime;
            }

            /* if more than 2 mins, stop the attempt and return fail */
            if(diffTime/TIMESTAMP_TICKS_PER_MSEC > MAX_TIME_LIMIT_IN_MSEC)
            {
                printf ("Fail to bring up the ADSL line\n");
                return IX_FAIL;
            }

	    ixOsalSleep(1); /* 1ms */

	} while ((IX_ADSL_LINE_STATE_UP_DUAL_LATENCY != adslLineState) &&
		 (IX_ADSL_LINE_STATE_UP_FASTCHANNEL != adslLineState) && 
		 (IX_ADSL_LINE_STATE_UP_INTERLEAVECHANNEL != adslLineState));
    }

    threadAttr.name = "ShowTask";
    threadAttr.stackSize = IX_ETHAAL5APP_THREAD_STACK_SIZE;
    threadAttr.priority = IX_EAA_THREAD_PRI_LOW;

    /* Create thread for ixEAAShow */
    if (IX_SUCCESS != ixOsalThreadCreate(
	&tid,
	&threadAttr,
	(IxOsalVoidFnVoidPtr)ixEthAal5AppShowTask,
	NULL))
    { 
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		  "Error creating thread EthAal5AppShow task\n",
		  0, 0, 0, 0, 0, 0); 
 	return IX_FAIL; 
    }  

    /* Start thread for ixEAAShow */
    if (ixOsalThreadStart(&tid) != IX_SUCCESS)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR, 
		  "Error starting thread EthAal5AppShow task\n",
		  0, 0, 0, 0, 0, 0); 
	return IX_FAIL;
    }

    printf("Every 15s, ixEAAShow() will get the current traffics counters\n");

    return IX_SUCCESS;
}

/* 
 * Eth Link State Monitoring that tries to synchronize
 * Eth Mii State with MAC state.  
 */
void ixEAAEthLinkStateMonitorLoop(void)
{
    UINT32 phyNum;
    BOOL   ethLinkUp;
    BOOL   ethSpeed100;
    BOOL   ethFullDuplex;
    BOOL   ethAutoneg;

    IxEAAEthDuplexMode lastDuplexMode[IX_EAA_NUM_ETH_PORTS]; 

    /* Initialize lastDuplexMode to unknown duplex mode*/
    for (phyNum =0; phyNum < IX_EAA_NUM_ETH_PORTS; phyNum++)
    {
	lastDuplexMode[phyNum] = IX_EAA_UNKNOWN_MODE;
    }

    /*
     * Start the infinite loop
     */
    while (TRUE)
    {
        for (phyNum = 0; phyNum < IX_EAA_NUM_ETH_PORTS; phyNum++)
	{
            /* check the PHY is initialized */
            if (IX_EAA_INVALID_PHY_ADDR == phyAddresses[phyNum])
                continue;
            
            /*
             * Determine the link status
             */
            if (IX_ETH_ACC_SUCCESS != 
                ixEthMiiLinkStatus(phyAddresses[phyNum],
                                      &ethLinkUp,
                                      &ethSpeed100,
                                      &ethFullDuplex, 
                                      &ethAutoneg))
                {
		    printf("ixEthMiiLinkStatus fails for phyAddress = %x\n", phyAddresses[phyNum]);
                }
	     
            /*
             * Update the MAC mode to match the PHY mode (MII mode). 
             * If autonegotiotion is not enabled, then the MAC mode is already 
             * set during initialisation. 
             * If autonegotiation is enabled, the MAC mode may change at any time.
             */
            if (ethLinkUp  && ethAutoneg)
	    {
                if (ethFullDuplex)
		{
                    /* check if the MAC mode is already set */
                    if (IX_EAA_FULL_DUPLEX_MODE != lastDuplexMode[phyNum])
		    {
                        ixEthAccPortDuplexModeSet (phyNum,
                                                   IX_ETH_ACC_FULL_DUPLEX);
                        lastDuplexMode[phyNum] = IX_EAA_FULL_DUPLEX_MODE;
#ifdef IXEAA_DEBUG			
                        printf("Port %d set to Full duplex\n", phyNum);
#endif
		    }
		}
                else
		{
                    /* check if the MAC mode is already set */
                    if (IX_EAA_HALF_DUPLEX_MODE != lastDuplexMode[phyNum])
		    {
                        ixEthAccPortDuplexModeSet (phyNum,
                                                   IX_ETH_ACC_HALF_DUPLEX);
                        lastDuplexMode[phyNum] = IX_EAA_HALF_DUPLEX_MODE;
#ifdef IXEAA_DEBUG
                        printf("Port %d set to Half duplex\n", phyNum);
#endif
		    }
		} /* end of if (ethFullDuplex) */
	    } /* end of if (ethlinkUp  && ethAutoneg) */
	} /* end of for phyNum */
        /*
         * This function should poll the MII link status approximately every 
         * 4 seconds
         */
        ixOsalSleep(4000);
    } /* End of while (TRUE)*/
}

#ifdef __wince
int readNumber(void)
{
    char line[256];
    gets(line);
    return atoi(line);
}

int wmain(int argc, WCHAR **argv)
{
    int ethAal5CodeletTestNr;
    BOOL ethAal5CodeletRun = TRUE;

    while(ethAal5CodeletRun)
    {
        printf("\n");
        printf("********************** ethAal5 Codelet *************************\n");
        printf("  1. Execute Ethernet AAL5 Codelet in Utopia mode.\n");
        printf("  2. Execute Ethernet AAL5 Codelet in ADSL mode.\n");
        printf("100. Exit Ethernet AAL5 Codelet.\n");
        printf("\n");
        printf("Enter test number: ");
        ethAal5CodeletTestNr = readNumber();

        switch(ethAal5CodeletTestNr)
        {
            case 1:
                ixEthAal5AppCodeletMain(IX_EAA_UTOPIA_MODE);
                break;
            case 2:
                ixEthAal5AppCodeletMain(IX_EAA_ADSL_MODE);
                break;
            case 100:
                ethAal5CodeletRun = FALSE;
                break;
            default:
                break;
        }
    }

    return 0;
}
#endif
