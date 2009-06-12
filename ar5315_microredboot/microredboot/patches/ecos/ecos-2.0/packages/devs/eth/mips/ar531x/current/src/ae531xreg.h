//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2003 Atheros Communications, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting the copyright
// holders.
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Atheros Communications, Inc.
// Contributors: Atheros Engineering
// Date:         2003-10-22
// Purpose:      
// Description:  AR531X ethernet hardware driver
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================
/*
 * See README to understand the decomposition of the ethernet driver.
 *
 * Register definitions for Atheros AR531X Ethernet MAC.
 */

#ifndef _AE531XREG_H_
#define _AE531XREG_H_

#define AE531X_MAC_OFFSET 0x0000
#define AE531X_PHY_OFFSET 0x0000 /* Same as MAC offset */
#define AE531X_DMA_OFFSET 0x1000

/***********************************************************/
/* MAC110 registers, base address is BAR+AE531X_MAC_OFFSET */
/***********************************************************/
#define MacControl            0x00  /* control */
#define MacAddrHigh           0x04  /* address high */
#define MacAddrLow            0x08  /* address low */
#define MacMultiHashHigh      0x0C  /* multicast hash table high */
#define MacMultiHashLow       0x10  /* multicast hash table low */
#define MacMiiAddr            0x14  /* MII address */
#define MacMiiData            0x18  /* MII data */
#define MacFlowControl        0x1C  /* Flow control */
#define MacVlan1Tag           0x4C  /* VLAN1 tag */
#define MacVlan2Tag           0x50  /* VLAN2 tag */


/***************************************************************/
/* DMA engine registers, base address is BAR+AE531X_DMA_OFFSET */
/***************************************************************/
#define DmaBusMode      0x00 /* CSR0 - Bus Mode */
#define DmaTxPollDemand 0x04 /* CSR1 - Transmit Poll Demand */
#define DmaRxPollDemand 0x08 /* CSR2 - Receive Poll Demand */
#define DmaRxBaseAddr   0x0C /* CSR3 - Receive list base address */
#define DmaTxBaseAddr   0x10 /* CSR4 - Transmit list base address */
#define DmaStatus       0x14 /* CSR5 - Dma status */
#define DmaControl      0x18 /* CSR6 - Dma control */
#define DmaIntrEnb      0x1C /* CSR7 - Interrupt enable */
#define DmaOverflowCnt  0x20 /* CSR8 - Missed Frame and Buff Overflow counter */
#define DmaTxCurrAddr   0x50 /* CSR20 - Current host transmit buffer address */
#define DmaRxCurrAddr   0x54 /* CSR21 - Current host receive buffer address */

/**********************************************************/
/* MAC Control register layout                            */
/**********************************************************/
#define MacFilterOff           0x80000000 /* Receive all incoming packets RW */
#define MacFilterOn                     0 /* Receive filtered packets only 0 */
#define MacBigEndian           0x40000000 /* Big endian mode RW */
#define MacLittleEndian                 0 /* Little endian 0 */
#define MacHeartBeatOff        0x10000000 /* Heartbeat signal qual disable RW*/
#define MacHeartBeatOn                  0 /* Heartbeat signal qual enable 0 */
#define MacSelectSrl           0x08000000 /* Select SRL port RW */
#define MacSelectMii                    0 /* Select MII port 0 */
#define MacDisableRxOwn        0x00800000 /* Disable receive own packets RW */
#define MacEnableRxOwn                  0 /* Enable receive own packets 0 */
#define MacLoopbackExt         0x00400000 /* External loopback RW */
#define MacLoopbackInt         0x00200000 /* Internal loopback */
#define MacLoopbackOff                  0 /* Normal mode 00 */
#define MacFullDuplex          0x00100000 /* Full duplex mode RW */
#define MacHalfDuplex                   0 /* Half duplex mode 0 */
#define MacMulticastFilterOff  0x00080000 /* Pass all multicast packets RW */
#define MacMulticastFilterOn            0 /* Pass filtered mcast packets 0 */
#define MacPromiscuousModeOn   0x00040000 /* Receive all valid packets RW 1 */
#define MacPromiscuousModeOff           0 /* Receive filtered packets only */
#define MacFilterInverse       0x00020000 /* Inverse filtering RW */
#define MacFilterNormal                 0 /* Normal filtering 0 */
#define MacBadFramesEnable     0x00010000 /* Pass bad frames RW */
#define MacBadFramesDisable             0 /* Do not pass bad frames 0 */
#define MacPerfectFilterOff    0x00008000 /* Hash filtering only RW */
#define MacPerfectFilterOn              0 /* Both perfect and hash filtering 0 */
#define MacHashFilterOn        0x00002000 /* perform hash filtering RW */
#define MacHashFilterOff                0 /* perfect filtering only 0 */
#define MacLateCollisionOn     0x00001000 /* Enable late collision control RW */
#define MacLateCollisionOff             0 /* Disable late collision control 0 */
#define MacBroadcastDisable    0x00000800 /* Disable reception of bcast frames RW */
#define MacBroadcastEnable              0 /* Enable broadcast frames 0 */
#define MacRetryDisable        0x00000400 /* Disable retransmission RW */
#define MacRetryEnable                  0 /* Enable retransmission 0 */
#define MacPadStripEnable      0x00000100 /* Pad stripping enable RW */
#define MacPadStripDisable              0 /* Pad stripping disable 0 */
#define MacBackoff                      0 /* Backoff Limit RW 00 */
#define MacDeferralCheckEnable 0x00000020 /* Deferral check enable RW */
#define MacDeferralCheckDisable         0 /* Deferral check disable 0 */
#define MacTxEnable            0x00000008 /* Transmitter enable RW */
#define MacTxDisable                    0 /* Transmitter disable 0 */
#define MacRxEnable            0x00000004 /* Receiver enable RW */
#define MacRxDisable                    0 /* Receiver disable 0 */


/**********************************************************/
/* MII address register layout                            */
/**********************************************************/
#define MiiDevMask   0x0000F800 /* MII device address */
#define MiiDevShift          11
#define MiiRegMask   0x000007C0 /* MII register */
#define MiiRegShift           6
#define MiiWrite     0x00000002 /* Write to register */
#define MiiRead               0 /* Read from register */
#define MiiBusy      0x00000001 /* MII interface is busy */

/**********************************************************/
/* MII Data register layout                               */
/**********************************************************/
#define MiiDataMask  0x0000FFFF /* MII Data */

/**********************************************************/
/* MAC flow control register layout                       */
/**********************************************************/
#define MacPauseTimeMask      0xFFFF0000  /* PAUSE TIME field in ctrl frame */
#define MacPauseTimeShift             15
#define MacControlFrameEnable 0x00000004  /* Enable pass ctrl frames to host */
#define MacControlFrameDisable         0  /* Do not pass ctrl frames to host */
#define MacFlowControlEnable  0x00000002  /* Enable flow control */
#define MacFlowControlDisable          0  /* Disable flow control */
#define MacSendPauseFrame     0x00000001  /* send pause frame */

/**********************************************************/
/* DMA bus mode register layout                           */
/**********************************************************/
#define DmaRxAlign16            0x01000000 /* Force all rx buffers to align on odd hw bndry */
#define DmaBigEndianDes         0x00100000 /* Big endian data buffer descriptors RW */
#define DmaLittleEndianDesc              0 /* Little endian data descriptors */
#define DmaBurstLength32        0x00002000 /* Dma burst length 32 RW */
#define DmaBurstLength16        0x00001000 /* Dma burst length 16 */
#define DmaBurstLength8         0x00000800 /* Dma burst length 8 */
#define DmaBurstLength4         0x00000400 /* Dma burst length 4 */
#define DmaBurstLength2         0x00000200 /* Dma burst length 2 */
#define DmaBurstLength1         0x00000100 /* Dma burst length 1 */
#define DmaBurstLength0         0x00000000 /* Dma burst length 0 */
#define DmaBigEndianData        0x00000080 /* Big endian data buffers RW */
#define DmaLittleEndianData              0 /* Little endian data buffers 0 */
#define DmaDescriptorSkip16     0x00000040 /* number of dwords to skip RW */
#define DmaDescriptorSkip8      0x00000020 /* between two unchained descriptors */
#define DmaDescriptorSkip4      0x00000010
#define DmaDescriptorSkip2      0x00000008
#define DmaDescriptorSkip1      0x00000004
#define DmaDescriptorSkip0               0
#define DmaReceivePriorityOff   0x00000002 /* equal rx and tx priorities RW */
#define DmaReceivePriorityOn             0 /* Rx has prioryty over Tx 0 */
#define DmaResetOn              0x00000001 /* Reset DMA engine RW */
#define DmaResetOff                      0

/**********************************************************/
/* DMA Status register layout                             */
/**********************************************************/
#define DmaRxAbort        0x01000000 /* receiver bus abort R 0 */
#define DmaTxAbort        0x00800000 /* transmitter bus abort R 0 */
#define DmaTxState        0x00700000 /* Transmit process state R 000 */
#define DmaTxStopped      0x00000000 /* Stopped */
#define DmaTxFetching     0x00100000 /* Running - fetching the descriptor */
#define DmaTxWaiting      0x00200000 /* Running - waiting for end of transmission */
#define DmaTxReading      0x00300000 /* Running - reading the data from memory */
#define DmaTxSuspended    0x00600000 /* Suspended */
#define DmaTxClosing      0x00700000 /* Running - closing descriptor */
#define DmaRxState        0x000E0000 /* Receive process state 000 */
#define DmaRxStopped      0x00000000 /* Stopped */
#define DmaRxFetching     0x00020000 /* Running - fetching the descriptor */
#define DmaRxChecking     0x00040000 /* Running - checking for end of packet */
#define DmaRxWaiting      0x00060000 /* Running - waiting for packet */
#define DmaRxSuspended    0x00080000 /* Suspended */
#define DmaRxClosing      0x000A0000 /* Running - closing descriptor */
#define DmaRxFlushing     0x000C0000 /* Running - flushing the current frame */
#define DmaRxQueuing      0x000E0000 /* Running - queuing the recieve frame into host memory */
#define DmaIntNormal      0x00010000 /* Normal interrupt summary RW 0 */
#define DmaIntAbnormal    0x00008000 /* Abnormal interrupt summary RW 0 */
#define DmaIntEarlyRx     0x00004000 /* Early receive interrupt (Normal) RW 0 */
#define DmaIntBusError    0x00002000 /* Fatal bus error (Abnormal) RW 0 */
#define DmaIntEarlyTx     0x00000400 /* Early transmit interrupt RW 0 */
#define DmaIntRxStopped   0x00000100 /* Receive process stopped (Abnormal) RW 0 */
#define DmaIntRxNoBuffer  0x00000080 /* Receive buffer unavailable (Abnormal) RW 0*/
#define DmaIntRxCompleted 0x00000040 /* Completion of frame reception(Normal) RW 0*/
#define DmaIntTxUnderflow 0x00000020 /* Transmit underflow (Abnormal) RW 0 */
#define DmaIntTxJabber    0x00000008 /* Transmit Jabber Timeout (Abnormal) RW 0 */ 
#define DmaIntTxNoBuffer  0x00000004 /* Transmit buffer unavailable (Normal) RW 0*/
#define DmaIntTxStopped   0x00000002 /* Transmit process stopped (Abnormal) RW 0 */
#define DmaIntTxCompleted 0x00000001 /* Transmit completed (Normal) RW 0 */

/**********************************************************/
/* DMA control register layout                            */
/**********************************************************/
#define DmaStoreAndForward 0x00200000 /* Store and forward RW 0 */
#define DmaTxThreshCtl256  0x0000c000 /* Non-SF threshold is 256 words */
#define DmaTxThreshCtl128  0x00008000 /* Non-SF threshold is 128 words */
#define DmaTxThreshCtl064  0x00004000 /* Non-SF threshold is 64 words */
#define DmaTxThreshCtl032  0x00000000 /* Non-SF threshold is 32 words */
#define DmaTxStart         0x00002000 /* Start/Stop transmission RW 0 */
#define DmaTxSecondFrame   0x00000004 /* Operate on second frame RW 0 */
#define DmaRxStart         0x00000002 /* Start/Stop reception RW 0 */

/**********************************************************/
/* DMA interrupt enable register layout                   */
/**********************************************************/
#define DmaIeNormal      DmaIntNormal      /* Normal interrupt enable RW 0 */
#define DmaIeAbnormal    DmaIntAbnormal    /* Abnormal interrupt enable RW 0 */
#define DmaIeEarlyRx     DmaIntEarlyRx     /* Early receive interrupt enable RW 0 */
#define DmaIeBusError    DmaIntBusError    /* Fatal bus error enable RW 0 */
#define DmaIeEarlyTx     DmaIntEarlyTx     /* Early transmit interrupt enable RW 0 */
#define DmaIeRxStopped   DmaIntRxStopped   /* Receive process stopped enable RW 0 */
#define DmaIeRxNoBuffer  DmaIntRxNoBuffer  /* Receive buffer unavailable enable RW 0 */
#define DmaIeRxCompleted DmaIntRxCompleted /* Completion of frame reception enable RW 0 */
#define DmaIeTxUnderflow DmaIntTxUnderflow /* Transmit underflow enable RW 0 */
#define DmaIeTxJabber    DmaIntTxJabber    /* Transmit jabber timeout RW 0 */
#define DmaIeTxNoBuffer  DmaIntTxNoBuffer  /* Transmit buffer unavailable enable RW 0 */
#define DmaIeTxStopped   DmaIntTxStopped   /* Transmit process stopped enable RW 0 */
#define DmaIeTxCompleted DmaIntTxCompleted /* Transmit completed enable RW 0 */

/****************************************************************/
/* DMA Missed Frame and Buffer Overflow Counter register layout */
/****************************************************************/
#define DmaRxBufferMissedFrame  0xffff0000  /* cleared on read */
#define DmaMissedFrameShift             16
#define DmaRxBufferOverflowCnt  0x0000ffff  /* cleared on read */
#define DmaMissedFrameCountMask 0x0000ffff

/**********************************************************/
/* DMA Engine descriptor layout                           */
/**********************************************************/
/* status word of DMA descriptor */
#define DescOwnByDma         0x80000000 /* Descriptor is owned by DMA engine */
#define DescFrameLengthMask  0x3FFF0000 /* Receive descriptor frame length */
#define DescFrameLengthShift         16
#define DescError            0x00008000 /* Error summary bit OR of following bits */
#define DescRxTruncated      0x00004000 /* Rx - no more descs for receive frame */
#define DescRxLengthError    0x00001000 /* Rx - frame size not matching with length field */
#define DescRxRunt           0x00000800 /* Rx - runt frame, damaged by a
                                           collision or term before 64 bytes */
#define DescRxMulticast      0x00000400 /* Rx - received frame is multicast */
#define DescRxFirst          0x00000200 /* Rx - first descriptor of the frame */
#define DescRxLast           0x00000100 /* Rx - last descriptor of the frame */
#define DescRxLongFrame      0x00000080 /* Rx - frame is longer than 1518 bytes */
#define DescRxLateColl       0x00000040 /* Rx - frame was damaged by a late collision */
#define DescRxFrameEther     0x00000020 /* Rx - Frame type Ethernet 802.3*/ 
#define DescRxMiiError       0x00000008 /* Rx - error reported by MII interface */
#define DescRxDribbling      0x00000004 /* Rx - frame contains noninteger multiple of 8 bits */
#define DescRxCrc            0x00000002 /* Rx - CRC error */
#define DescTxTimeout        0x00004000 /* Tx - Transmit jabber timeout */
#define DescTxLostCarrier    0x00000800 /* Tx - carrier lost during tramsmission */
#define DescTxNoCarrier      0x00000400 /* Tx - no carrier signal from tranceiver */
#define DescTxLateCollision  0x00000200 /* Tx - transmission aborted due to collision */
#define DescTxExcCollisions  0x00000100 /* Tx - transmission aborted after 16 collisions */
#define DescTxHeartbeatFail  0x00000080 /* Tx - heartbeat collision check failure */
#define DescTxCollMask       0x00000078 /* Tx - Collision count */
#define DescTxCollShift               3
#define DescTxExcDeferral    0x00000004 /* Tx - excessive deferral */
#define DescTxUnderflow      0x00000002 /* Tx - late data arrival from memory */
#define DescTxDeferred       0x00000001 /* Tx - frame transmision deferred */

/* length word of DMA descriptor */
#define DescTxIntEnable      0x80000000 /* Tx - interrupt on completion */
#define DescTxLast           0x40000000 /* Tx - Last segment of the frame */
#define DescTxFirst          0x20000000 /* Tx - First segment of the frame */
#define DescTxDisableCrc     0x04000000 /* Tx - Add CRC disabled (first segment only) */
#define DescEndOfRing        0x02000000 /* End of descriptors ring */
#define DescChain            0x01000000 /* Second buffer address is chain address */
#define DescTxDisablePadd    0x00800000 /* disable padding */
#define DescSize2Mask        0x003FF800 /* Buffer 2 size */
#define DescSize2Shift               11
#define DescSize1Mask        0x000007FF /* Buffer 1 size */
#define DescSize1Shift                0

/**********************************************************/
/* Initial register values                                */
/**********************************************************/
/* Full-duplex mode with perfect filter on */
#define MacControlInitFdx \
       ( MacFilterOn \
       | MacLittleEndian \
       | MacHeartBeatOn \
       | MacSelectMii \
       | MacEnableRxOwn \
       | MacLoopbackOff \
       | MacFullDuplex \
       | MacMulticastFilterOn \
       | MacPromiscuousModeOff \
       | MacFilterNormal \
       | MacBadFramesDisable \
       | MacPerfectFilterOn \
       | MacHashFilterOff \
       | MacLateCollisionOff \
       | MacBroadcastEnable \
       | MacRetryEnable \
       | MacPadStripDisable \
       | MacDeferralCheckDisable \
       | MacTxEnable \
       | MacRxEnable)

/* Full-duplex mode */
#define MacFlowControlInitFdx \
        ( MacControlFrameDisable \
        | MacFlowControlEnable)

/* Half-duplex mode with perfect filter on */
#define MacControlInitHdx \
       ( MacFilterOn \
       | MacLittleEndian \
       | MacHeartBeatOn \
       | MacSelectMii \
       | MacDisableRxOwn \
       | MacLoopbackOff \
       | MacHalfDuplex \
       | MacMulticastFilterOn \
       | MacPromiscuousModeOff \
       | MacFilterNormal \
       | MacBadFramesDisable \
       | MacPerfectFilterOn \
       | MacHashFilterOff \
       | MacLateCollisionOff \
       | MacBroadcastEnable \
       | MacRetryEnable \
       | MacPadStripDisable \
       | MacDeferralCheckDisable \
       | MacTxEnable \
       | MacRxEnable)

/* Half-duplex mode */
#define MacFlowControlInitHdx \
        ( MacControlFrameDisable \
        | MacFlowControlDisable)

/* Bus Mode Rx odd half word align */
#define DmaBusModeInit  \
       ( DmaLittleEndianDesc \
       | DmaRxAlign16 \
       | DmaBurstLength4 \
       | DmaBigEndianData \
       | DmaDescriptorSkip1 \
       | DmaReceivePriorityOn \
       | DmaResetOff)

#define DmaControlInit (DmaStoreAndForward)

/* Interrupt groups */
#define DmaIntEnable \
     ( DmaIeNormal \
     | DmaIeAbnormal \
     | DmaIntBusError \
     | DmaIntRxStopped \
     | DmaIntRxNoBuffer \
     | DmaIntRxCompleted \
     | DmaIntTxUnderflow \
     | DmaIntTxStopped)

#define DmaIntDisable 0

#define DmaAllIntCauseMask \
      ( DmaIeNormal  \
      | DmaIeAbnormal  \
      | DmaIntEarlyRx  \
      | DmaIntBusError \
      | DmaIntEarlyTx  \
      | DmaIntRxStopped \
      | DmaIntRxNoBuffer \
      | DmaIntRxCompleted \
      | DmaIntTxUnderflow \
      | DmaIntTxJabber \
      | DmaIntTxNoBuffer \
      | DmaIntTxStopped \
      | DmaIntTxCompleted)

#define UnhandledIntrMask    \
       (DmaAllIntCauseMask   \
       & ~(DmaIntRxNoBuffer  \
         | DmaIntTxStopped   \
         | DmaIntTxJabber    \
         | DmaIntTxUnderflow \
         | DmaIntBusError    \
         | DmaIntRxCompleted ))

#define DescRxErrors    \
      (DescRxTruncated  \
     | DescRxRunt       \
     | DescRxLateColl   \
     | DescRxMiiError   \
     | DescRxCrc)

#define DescTxErrors        \
     ( DescTxTimeout        \
     | DescTxLateCollision  \
     | DescTxExcCollisions  \
     | DescTxExcDeferral    \
     | DescTxUnderflow)

/**********************************************************/
/* Descriptor Layout                                      */
/**********************************************************/
#define AE531X_DESC_STATUS     0x00 /* Status offset */
#define AE531X_DESC_CTRLEN     0x04 /* Control and Length offset */ 
#define AE531X_DESC_BUFPTR     0x08 /* Buffer pointer offset */
#define AE531X_DESC_LNKBUF     0x0c /* Link field offset, or ptr to 2nd buf */
#define AE531X_DESC_SWPTR      0x10 /* OS-Dependent software pointer */

#define AE531X_DESC_SIZE       0x10 /* 4 words, 16 bytes */
#define AE531X_QUEUE_ELE_SIZE  0x14 /* with software pointer extension */

/* Accessors to the dma descriptor fields */
#define AE531X_DESC_STATUS_GET(ptr) \
    *(volatile UINT32 *)((UINT32)(ptr) + AE531X_DESC_STATUS)

#define AE531X_DESC_STATUS_SET(ptr, val)  \
    AE531X_DESC_STATUS_GET(ptr) = (val)

#define AE531X_DESC_CTRLEN_GET(ptr) \
    *(volatile UINT32 *)((UINT32)ptr + AE531X_DESC_CTRLEN)

#define AE531X_DESC_CTRLEN_SET(ptr, val)  \
    AE531X_DESC_CTRLEN_GET(ptr) = (val)

#define AE531X_DESC_BUFPTR_GET(ptr) \
    *(volatile UINT32 *)((UINT32)ptr + AE531X_DESC_BUFPTR)

#define AE531X_DESC_BUFPTR_SET(ptr,val)  \
    AE531X_DESC_BUFPTR_GET(ptr) = (UINT32)(val)

#define AE531X_DESC_LNKBUF_GET(ptr)  \
    *(volatile UINT32 *)((UINT32)ptr + AE531X_DESC_LNKBUF)

#define AE531X_DESC_LNKBUF_SET(ptr, val)  \
    AE531X_DESC_LNKBUF_GET(ptr) = (val)

#define AE531X_DESC_SWPTR_GET(ptr) \
    (void *)(*(volatile UINT32 *) ((UINT32)ptr + AE531X_DESC_SWPTR))

#define AE531X_DESC_SWPTR_SET(ptr,val)   \
    AE531X_DESC_SWPTR_GET(ptr) = (void *)(val)

/* Get size of Rx data from desc, in bytes */
#define AE531X_DESC_STATUS_RX_SIZE(x) \
        (((x) & DescFrameLengthMask) >> DescFrameLengthShift)

#endif /* _AE531XREG_H_ */
