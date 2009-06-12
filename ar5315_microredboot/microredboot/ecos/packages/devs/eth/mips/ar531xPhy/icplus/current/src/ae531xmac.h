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
 * This file contains OS-independent pure software definitions for
 * ethernet support on the AR531X platform.
 */

#ifndef _AE531XMAC_H_
#define _AE531XMAC_H_

#define CONFIG_VENETDEV 0      /* Not supported */

#if !defined(CYGNUM_USE_REALTEK_ENET_PHY)
#define CYGNUM_USE_REALTEK_ENET_PHY 0
#endif

#if !defined(CYGNUM_USE_KENDIN_ENET_PHY)
#define CYGNUM_USE_KENDIN_ENET_PHY 0
#endif

#if !defined(CYGNUM_USE_MARVELL_ENET_PHY)
#define CYGNUM_USE_MARVELL_ENET_PHY 0
#endif

/*
 * DEBUG switches to control verbosity.
 * Just modify the value of ae531x_debug.
 */
#define AE531X_DEBUG_ALL         0xffffffff
#define AE531X_DEBUG_ERROR       0x00000001 /* Unusual conditions and Errors */
#define AE531X_DEBUG_ARRIVE      0x00000002 /* Arrive into a function */
#define AE531X_DEBUG_LEAVE       0x00000004 /* Leave a function */
#define AE531X_DEBUG_RESET       0x00000008 /* Reset */
#define AE531X_DEBUG_TX          0x00000010 /* Transmit */
#define AE531X_DEBUG_TX_REAP     0x00000020 /* Transmit Descriptor Reaping */
#define AE531X_DEBUG_RX          0x00000040 /* Receive */
#define AE531X_DEBUG_RX_STOP     0x00000080 /* Receive Early Stop */
#define AE531X_DEBUG_INT         0x00000100 /* Interrupts */
#define AE531X_DEBUG_LINK_CHANGE 0x00000200 /* PHY Link status changed */

extern int ae531x_debug;

#define AE531X_PRINT(FLG, X)                        \
{                                                   \
    if (ae531x_debug & (FLG)) {                     \
        DEBUG_PRINTF("%s#%d:%s ",                   \
                     __FILE__,                      \
                     __LINE__,                      \
                     __FUNCTION__);                 \
        DEBUG_PRINTF X;                             \
    }                                               \
}

#define ARRIVE() AE531X_PRINT(AE531X_DEBUG_ARRIVE, ("Arrive{\n"))
#define LEAVE() AE531X_PRINT(AE531X_DEBUG_LEAVE, ("}Leave\n"))

#define RegRead(addr)	\
	(*(volatile unsigned int *)(addr))

#define RegWrite(val,addr)	\
	((*(volatile unsigned int *)(addr)) = (val))

/*****************************************************************
 * MAC-independent interface to be used by PHY code
 *
 * These functions are provided by the MAC layer for use by the PHY layer.
 */
#define phyRegRead ae531x_MiiRead
#define phyRegWrite ae531x_MiiWrite
#define phyLinkLost(ethUnit) ae531x_unitLinkLost(ethUnit)
#define phyLinkGained(ethUnit) ae531x_unitLinkGained(ethUnit)

void ae531x_unitLinkLost(int unit);
void ae531x_unitLinkGained(int unit);

#define ETH_CRC_LEN       4

/*****************************************************************
 * Descriptor queue
 */
typedef struct ae531x_queue {
    VIRT_ADDR   firstDescAddr;  /* descriptor array address */
    VIRT_ADDR   lastDescAddr;   /* last descriptor address */
    VIRT_ADDR   curDescAddr;    /* current descriptor address */
    VIRT_ADDR   reapDescAddr;   /* current tail of tx descriptors reaped */
    UINT16      count;          /* number of elements */
} AE531X_QUEUE;

/* Given a descriptor, return the next one in a circular list */
#define AE531X_QUEUE_ELE_NEXT_GET(q, descAddr)                          \
        ((descAddr) == (q)->lastDescAddr) ? (q)->firstDescAddr :    \
        (VIRT_ADDR)((UINT32)(descAddr) + AE531X_QUEUE_ELE_SIZE)

/* Move the "current descriptor" forward to the next one */
#define AE531X_CONSUME_DESC(q)    \
         q->curDescAddr = AE531X_QUEUE_ELE_NEXT_GET(q, q->curDescAddr)

/*****************************************************************
 * Per-ethernet-MAC OS-independent information
 */
typedef struct ae531x_MAC_s {
    UINT32          unit;          /* MAC unit ID */
    UINT32          macBase;       /* MAC base address */
    UINT32          dmaBase;       /* DMA base address */
    UINT32          phyBase;       /* PHY base address */
    AE531X_QUEUE    txQueue;       /* Transmit descriptor queue */
    AE531X_QUEUE    rxQueue;       /* Receive descriptor queue */
    UINT16          txDescCount;   /* Transmit descriptor count */
    UINT16          rxDescCount;   /* Receive descriptor count */
    char            *pTxDescs;     /* Transmit descriptors */
    char            *pRxDescs;     /* Receive descriptors */
    BOOL            aeProcessRst;  /* flag to indicate reset in progress */
    BOOL            port_is_up;    /* flag to indicate port is up */
    void            *OSinfo;       /* OS-dependent per-MAC information */
} ae531x_MAC_t;

#define	AE531X_TX_DESC_COUNT_DEFAULT	8     /* Transmit descriptors */
#define AE531X_RX_DESC_COUNT_DEFAULT	8     /* Receive descriptors */


/*****************************************************************
 * Interfaces exported by the OS-independent MAC layer
 */
void ae531x_BeginResetMode(ae531x_MAC_t *MACInfo);
void ae531x_EndResetMode(ae531x_MAC_t *MACInfo);
BOOL ae531x_IsInResetMode(ae531x_MAC_t *MACInfo);
int ae531x_RxQueueCreate(ae531x_MAC_t *MACInfo, AE531X_QUEUE *q,
                  char *pMem, int count);
int ae531x_QueueDelete(struct ae531x_queue *q);
void ae531x_DmaReset(ae531x_MAC_t *MACInfo);
void ae531x_MACReset(ae531x_MAC_t *MACInfo);
void ae531x_EnableComm(ae531x_MAC_t *MACInfo);
void ae531x_DisableComm(ae531x_MAC_t *MACInfo);
void ae531x_reset(ae531x_MAC_t *MACInfo);
int ae531x_AllocateQueues(ae531x_MAC_t *MACInfo);
void ae531x_FreeQueues(ae531x_MAC_t *MACInfo);
void ae531x_QueueInit(AE531X_QUEUE *q, char *pMem, int count);
UINT32 ae531x_ReadMacReg(ae531x_MAC_t *MACInfo, UINT32 reg);
void ae531x_WriteMacReg(ae531x_MAC_t *MACInfo, UINT32 reg, UINT32 data);
void ae531x_SetMacReg(ae531x_MAC_t *MACInfo, UINT32 reg, UINT32 val);
void ae531x_ClearMacReg(ae531x_MAC_t *MACInfo, UINT32 reg, UINT32 val);
void ae531x_SetDmaReg(ae531x_MAC_t *MACInfo, UINT32 reg, UINT32 val);
void ae531x_ClearDmaReg(ae531x_MAC_t *MACInfo, UINT32 reg, UINT32 val);
UINT32 ae531x_ReadDmaReg(ae531x_MAC_t *MACInfo, UINT32 reg);
void ae531x_WriteDmaReg(ae531x_MAC_t *MACInfo, UINT32 reg, UINT32 data);
UINT32 ae531x_ReadMiiReg(UINT32 phyBase, UINT32 reg);
void ae531x_WriteMiiReg(UINT32 phyBase, UINT32 reg, UINT32 data);
UINT16 ae531x_MiiRead(UINT32 phyBase, UINT32 phyAddr, UINT8 reg);
void ae531x_MiiWrite(UINT32 phyBase, UINT32 phyAddr, UINT8 reg, UINT16 data);
void ae531x_DmaIntEnable(ae531x_MAC_t *MACInfo);
void ae531x_DmaIntDisable(ae531x_MAC_t *MACInfo);
void ae531x_AckIntr(ae531x_MAC_t *MACInfo, UINT32 val);
void *ae531x_rxbuf_alloc(ae531x_MAC_t *MACInfo, char **rxBptr, int *rxBSize);

#endif /* _AE531XMAC_H_ */
