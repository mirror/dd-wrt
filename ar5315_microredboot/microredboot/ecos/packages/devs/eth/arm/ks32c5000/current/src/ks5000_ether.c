//==========================================================================
//
//      ks5000_ether.c
//
//      
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
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
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    gthomas
// Contributors: gthomas, jskov
//               Grant Edwards <grante@visi.com>
// Date:         2001-07-31
// Purpose:      
// Description:  
//
//####DESCRIPTIONEND####
//
//========================================================================*/

#include <pkgconf/system.h>
#include <pkgconf/devs_eth_arm_ks32c5000.h>
#include <pkgconf/io_eth_drivers.h>

#include <errno.h>
#if defined(CYGPKG_IO)
#include <pkgconf/io.h>
#include <cyg/io/io.h>
#include <cyg/io/devtab.h>
#endif
// need to provide fake values for errno?
#ifndef EIO
# define EIO 1
#endif
#ifndef EINVAL
# define EINVAL 2
#endif

#include <cyg/infra/cyg_type.h>  // Common type definitions and support
                                 // including endian-ness
#include <cyg/infra/diag.h>
#include <cyg/io/eth/netdev.h>
#include <cyg/io/eth/eth_drv.h>
#include <cyg/io/eth/eth_drv_stats.h>
#include <cyg/hal/hal_intr.h>

#if defined(CYGPKG_REDBOOT)
#include <pkgconf/redboot.h>
#endif

#ifndef CYGINT_IO_ETH_INT_SUPPORT_REQUIRED
#define cyg_drv_interrupt_unmask(v) /* noop */
#define cyg_drv_interrupt_mask(v)   /* noop */
#define cyg_drv_isr_lock()          /* noop */
#define cyg_drv_isr_unlock()        /* noop */
#define cyg_drv_mutex_init(m)       /* noop */
#define cyg_drv_mutex_lock(m)       /* noop */
#define cyg_drv_mutex_unlock(m)     /* noop */
#define cyg_drv_dsr_lock()          /* noop */
#define cyg_drv_dsr_unlock()        /* noop */
#endif

#define HavePHYinterrupt 0

#include "std.h"
#include "ks5000_regs.h"
#include "ks5000_ether.h"

#if CYGINT_DEVS_ETH_ARM_KS32C5000_PHY
#include "phy.h"
#endif

// Set up the level of debug output
#if CYGPKG_DEVS_ETH_ARM_KS32C5000_DEBUG_LEVEL > 0
#define debug1_printf(args...) diag_printf(args)
#else
#define debug1_printf(args...) /* noop */
#endif
#if CYGPKG_DEVS_ETH_ARM_KS32C5000_DEBUG_LEVEL > 1
#define debug2_printf(args...) diag_printf(args)
#else
#define debug2_printf(args...) /* noop */
#endif

#define Bit(n) (1<<(n))

// enable/disable software verification of rx CRC
// should be moved to user-controlled valud in CDL file

#if defined(CYG_HAL_CPUTYPE_KS32C5000A)
#define SoftwareCRC 1
#include <cyg/crc/crc.h>
#else
#define SoftwareCRC 0
#endif

// --------------------------------------------------------------
// RedBoot configuration options for managing ESAs for us

// Decide whether to have redboot config vars for it...
#if defined(CYGSEM_REDBOOT_FLASH_CONFIG) && defined(CYGPKG_REDBOOT_NETWORKING)
#include <redboot.h>
#include <flash_config.h>

#ifdef CYGSEM_DEVS_ETH_ARM_KS32C5000_REDBOOT_HOLDS_ESA_ETH0
RedBoot_config_option("Network hardware address [MAC] for eth0",
                      eth0_esa_data,
                      ALWAYS_ENABLED, true,
                      CONFIG_ESA, 0);
#endif

#endif  // CYGPKG_REDBOOT_NETWORKING && CYGSEM_REDBOOT_FLASH_CONFIG

// and initialization code to read them
// - independent of whether we are building RedBoot right now:
#ifdef CYGPKG_DEVS_ETH_ARM_KS32C5000_REDBOOT_HOLDS_ESA

#include <cyg/hal/hal_if.h>

#ifndef CONFIG_ESA
#define CONFIG_ESA (6)
#endif

#define CYGHWR_DEVS_ETH_ARM_KS32C5000_GET_ESA( mac_address, ok )                 \
CYG_MACRO_START                                                                  \
    ok = CYGACC_CALL_IF_FLASH_CFG_OP( CYGNUM_CALL_IF_FLASH_CFG_GET,              \
                                      "eth0_esa_data", mac_address, CONFIG_ESA); \
CYG_MACRO_END

#endif // CYGPKG_DEVS_ETH_I82559_ETH_REDBOOT_HOLDS_ESA


#if CYGINT_DEVS_ETH_ARM_KS32C5000_PHY
// functions to read/write Phy chip registers via MII interface
// on 32c5000.  These need to be non-static since they're used
// by PHY-specific routines in a different file.
#define PHYREGWRITE	0x0400
#define MiiStart       0x0800

void MiiStationWrite(U32 RegAddr, U32 PhyAddr, U32 PhyWrData)
{
  STADATA = PhyWrData ;
  STACON = RegAddr | (PhyAddr<<5) |  MiiStart | PHYREGWRITE ;
  while (STACON & MiiStart)  
    ;
  //debug1_printf("PHY Wr %x:%02x := %04x\n",PhyAddr, RegAddr, PhyWrData) ;
}

U32 MiiStationRead(U32 RegAddr, U32 PhyAddr)
{
  U32	PhyRdData;
  STACON = RegAddr | (PhyAddr<<5) |  MiiStart;
  while (STACON & MiiStart)
    ;
  PhyRdData = STADATA;
  //debug1_printf("PHY Rd %x:%02x  %04x\n",PhyAddr,RegAddr,PhyRdData) ;
  return PhyRdData ;
}
#endif

// miscellaneous data structures

typedef  BYTE  ETH_ADDR[6] __attribute__((packed));

typedef  struct tagETH_HEADER 
{
  ETH_ADDR daddr __attribute__((packed));
  ETH_ADDR saddr __attribute__((packed));
  WORD type      __attribute__((packed));
} ETH_HEADER __attribute__((packed));

#define ETH_HEADER_SIZE   14

// Tx/Rx common descriptor structure
typedef  struct tagFRAME_DESCRIPTOR 
{
  LWORD FrameDataPtr;
  LWORD Reserved;   /*  cf: RX-reserved, TX-Reserved(25bits) + Control bits(7bits) */
  LWORD StatusAndFrameLength;
  struct tagFRAME_DESCRIPTOR   *NextFD;
} FRAME_DESCRIPTOR;

typedef struct
{
  U8 DestinationAddr[6];
  U8 SourceAddr[6];
  U8 LengthOrType[2];
  U8 LLCData[1506];
} MAC_FRAME;

#if defined(CYGPKG_NET)
struct ether_drv_stats ifStats;
#endif

#if defined(CYGINT_IO_ETH_INT_SUPPORT_REQUIRED)
static cyg_drv_mutex_t txMutex;
#endif

typedef struct
{
  LWORD BTxNLErr;
  LWORD BTxNOErr;
  LWORD BTxEmptyErr;
} BDMA_TX_ERR;

typedef struct
{
  LWORD BRxNLErr;
  LWORD BRxNOErr;
  LWORD BRxMSOErr;
  LWORD BRxEmptyErr;
  LWORD sBRxSEarly;
  LWORD noBufferAvail;
  LWORD queueOverflow;
  LWORD bad;
} BDMA_RX_ERR;


// interrupt entry counters
U32 ks5000_MAC_Rx_Cnt;
U32 ks5000_MAC_Tx_Cnt;
U32 ks5000_MAC_Phy_Cnt;
U32 ks5000_BDMA_Tx_Isr_Cnt;
U32 ks5000_BDMA_Tx_Dsr_Cnt;
U32 ks5000_BDMA_Rx_Isr_Cnt;
U32 ks5000_BDMA_Rx_Dsr_Cnt;


// packet and byte counters
static U32 MAC_Tx_Pkts;
static U32 MAC_Tx_Octets;
// static U32 BDMA_Rx_Pkts;
// static U32 BDMA_Rx_Octets;

// configuration values
static volatile U32 MACConfigVar;
static volatile U32 CAMConfigVar = CAMCON_COMP_EN | CAMCON_BROAD_ACC;
static volatile U32 MACTxConfigVar = 
  /* MACTXCON_EN_UNDER | */ 
  MACTXCON_EN_DEFER |
  MACTXCON_EN_NCARR | 
  MACTXCON_EN_EXCOLL |
  MACTXCON_EN_LATE_COLL | 
  MACTXCON_ENTX_PAR |
  MACTXCON_EN_COMP;
static volatile U32 MACRxConfigVar = 
  MACRXCON_RX_EN | 
  MACRXCON_EN_ALIGN |
  MACRXCON_EN_CRC_ERR | 
  MACRXCON_EN_OVER |
  MACRXCON_EN_LONG_ERR | 
  MACRXCON_EN_RX_PAR;

static volatile U32 BDMATxConfigVar =  
  BDMATXCON_MSL111 |
  BDMATXCON_STP_SKP | 
  3;   /* burst size - 1 */

#define EtherFramePadding 2

#if EtherFramePadding == 0
#define BDMARXCON_ALIGN BDMARXCON_WA00
#elif EtherFramePadding == 1
#define BDMARXCON_ALIGN BDMARXCON_WA01
#elif EtherFramePadding == 2
#define BDMARXCON_ALIGN BDMARXCON_WA10
#elif EtherFramePadding == 3
#define BDMARXCON_ALIGN BDMARXCON_WA11
#else
#error "EtherFramePadding must be 0,1,2 or 3"
#endif

#if (CYG_BYTEORDER == CYG_MSBFIRST) // Big endian
static volatile U32 BDMARxConfigVar = 
  BDMARXCON_DIE | 
  BDMARXCON_EN |
  BDMARXCON_BIG | 
  BDMARXCON_MA_INC |
  BDMARXCON_NOIE | 
  BDMARXCON_ALIGN |
  BDMARXCON_STP_SKP | 
  15;  /* burst size - 1 */
  
#else // Little endian
static volatile U32 BDMARxConfigVar = 
  BDMARXCON_DIE | 
  BDMARXCON_EN |
  BDMARXCON_LITTLE | 
  BDMARXCON_MA_INC |
  BDMARXCON_NOIE | 
  BDMARXCON_ALIGN |
  BDMARXCON_STP_SKP | 
  15;  /* burst size - 1 */
#endif


/* Global variables For BDMA Error Report */

static BDMA_TX_ERR BDMATxErrCnt = {0,0,0};
static BDMA_RX_ERR BDMARxErrCnt = {0,0,0,0,0};


static void Init_TxFrameDescriptorArray(void);
static void Init_RxFrameDescriptorArray(void);

// number of ethernet buffers should be enough to keep both rx
// and tx queues full plus some extras for in-process packets

#if defined(CYGPKG_REDBOOT)
#define NUM_ETH_BUFFERS 10
#define MAX_RX_FRAME_DESCRIPTORS        4     // Max number of Rx Frame Descriptors
#define MAX_TX_FRAME_DESCRIPTORS  	4     // Max number of Tx Frame Descriptors
#else
#define NUM_ETH_BUFFERS 80
#define MAX_RX_FRAME_DESCRIPTORS        32     // Max number of Rx Frame Descriptors
#define MAX_TX_FRAME_DESCRIPTORS  	32     // Max number of Tx Frame Descriptors
#endif

static FRAME_DESCRIPTOR _rxFrameDescrArray[MAX_RX_FRAME_DESCRIPTORS] __attribute__((aligned(16)));
static FRAME_DESCRIPTOR _txFrameDescrArray[MAX_TX_FRAME_DESCRIPTORS] __attribute__((aligned(16)));

/* define aliases that will set the no-cache bit */
#define rxFrameDescrArray ((FRAME_DESCRIPTOR*)(((unsigned)_rxFrameDescrArray)|0x4000000))
#define txFrameDescrArray ((FRAME_DESCRIPTOR*)(((unsigned)_txFrameDescrArray)|0x4000000))

static volatile FRAME_DESCRIPTOR *rxReadPointer;
static volatile FRAME_DESCRIPTOR *txDonePointer;
static volatile FRAME_DESCRIPTOR *txWritePointer;

static cyg_drv_mutex_t oldRxMutex;
static cyg_drv_cond_t  oldRxCond;


static bool configDone;

/*----------------------------------------------------------------------
 * Data structures used to manage ethernet buffers
 */
#define MAX_ETH_FRAME_SIZE 1520
  
typedef struct tEthBufferTag
{
  unsigned char data[MAX_ETH_FRAME_SIZE+8];
  unsigned length;
  unsigned userData;
  struct tEthBufferTag *next;
  struct tEthBufferTag *prev;
}tEthBuffer;


typedef struct 
{
  tEthBuffer *head;
  tEthBuffer *tail;
}tEthBufQueue;

#define EmptyQueue {NULL,NULL}

static void ethBufQueueClear(tEthBufQueue *q)
{
  q->head = NULL;
  q->tail = NULL;
}

static tEthBuffer *ethBufQueueGet(tEthBufQueue *q)
{
  tEthBuffer *r;
  
  r = q->head;
  
  if (r)
    q->head = r->next;
  
  return r;
}

static void ethBufQueuePut(tEthBufQueue *q, tEthBuffer *b)
{
  b->next = NULL;
  
  if (!q->head)
    {
      q->head = b;
      q->tail = b;
    }
  else
    {
      q->tail->next = b;
      q->tail = b;
    }
}

#if 0
// not used at the moment
static bool ethBufQueueEmpty(tEthBufQueue *q)
{
  return q->head != NULL;
}
#endif

/*----------------------------------------------------------------------
 * Free pool and routines to manipulate it.
 */

static tEthBuffer __bufferPool[NUM_ETH_BUFFERS] __attribute__((aligned(16)));
#define bufferPool ((tEthBuffer*)((unsigned)__bufferPool|0x4000000))

static tEthBufQueue freeList;
static int freeCount;

// do not call from ISR routine
static void freeBuffer(tEthBuffer *b)
{
  cyg_drv_isr_lock();
  ++freeCount;
  ethBufQueuePut(&freeList,b);
  cyg_drv_isr_unlock();
}

static int allocFail;

void bufferListError(void)
{
  while (1)
    ;
}

// do not call from ISR routine
static tEthBuffer *allocBuffer(void)
{
  tEthBuffer *r;
  cyg_drv_isr_lock();
  r = ethBufQueueGet(&freeList);
  cyg_drv_isr_unlock();
  if (r)
    --freeCount;
  else
    {
      ++allocFail;
      if (freeCount)
        bufferListError();
    }
  return r;
}

// call only from ISR routine or init
static void isrFreeBuffer(tEthBuffer *b)
{
  ++freeCount;
  ethBufQueuePut(&freeList,b);
}

#if 0
// not used at the moment

// call only from ISR routine or init
static tEthBuffer *isrAllocBuffer(void)
{
  tEthBuffer *r;
  r = ethBufQueueGet(&freeList);
  if (r)
    --freeCount;
  else
    {
      ++allocFail;
      if (freeCount)
        bufferListError();
    }
  return r;
}
#endif

static void initFreeList(void)
{
  int i;
  ethBufQueueClear(&freeList);
  freeCount = 0;
  for (i=0; i<NUM_ETH_BUFFERS; ++i)
    isrFreeBuffer(bufferPool+i);
}


//----------------------------------------------------------------------
// queue a buffer for transmit
//
// returns true if buffer was queued.

static int ks32c5000_eth_buffer_send(tEthBuffer *buf)
{
#if defined(CYGINT_IO_ETH_INT_SUPPORT_REQUIRED)
  while (!configDone)
    cyg_thread_delay(10);
#endif

  if (txWritePointer->FrameDataPtr & FRM_OWNERSHIP_BDMA)
    {
      // queue is full!  make sure transmit is running
      BDMATXCON |= BDMATXCON_EN;
      MACTXCON |= MACTXCON_TX_EN;
      return 0;
    }
  
  cyg_drv_mutex_lock(&txMutex);

  // free old buffer if we need to
  
  cyg_drv_isr_lock();
  if (txWritePointer->FrameDataPtr)
    {
      freeBuffer((tEthBuffer*)txWritePointer->FrameDataPtr);
      txWritePointer->FrameDataPtr = 0;
    }
  cyg_drv_isr_unlock();

  MAC_Tx_Pkts += 1;
  MAC_Tx_Octets += buf->length;

  // fill in the packet descriptor

#if (CYG_BYTEORDER == CYG_MSBFIRST) // Big endian
  txWritePointer->Reserved = (TXFDCON_PADDING_MODE | TXFDCON_CRC_MODE |
                    TXFDCON_SRC_ADDR_INC | TXFDCON_BIG_ENDIAN |
                    TXFDCON_WIDGET_ALIGN00 | TXFDCON_MAC_TX_INT_EN);
#else // Little endian
  txWritePointer->Reserved = (TXFDCON_PADDING_MODE | TXFDCON_CRC_MODE |
                    TXFDCON_SRC_ADDR_INC | TXFDCON_LITTLE_ENDIAN |
                    TXFDCON_WIDGET_ALIGN00 | TXFDCON_MAC_TX_INT_EN);
#endif
  
  txWritePointer->StatusAndFrameLength = buf->length;
  txWritePointer->FrameDataPtr = ((unsigned)buf | FRM_OWNERSHIP_BDMA);
  
  txWritePointer = txWritePointer->NextFD;
  
  cyg_drv_mutex_unlock(&txMutex);
    
  // start transmit

#if defined(CYGPKG_NET)  
  ++ifStats.tx_count;
#endif
  
  BDMATXCON |= BDMATXCON_EN;
  MACTXCON |= MACTXCON_TX_EN;
  return 1;
}


//======================================================================
// check to see if there's a frame waiting

static int rx_frame_avail(void)
{
  if (rxReadPointer->FrameDataPtr & FRM_OWNERSHIP_BDMA)
    {
      // queue is empty -- make sure Rx is running
      if (!(BDMARXCON & BDMARXCON_EN))
          {
            ++BDMARxErrCnt.queueOverflow;
            BDMARXCON |= BDMARXCON_EN;
          }
      return 0;
    }
  else
    return 1;
}


//======================================================================
// de-queue a receive buffer
static tEthBuffer *ks32c5000_eth_get_recv_buffer(void)
{
  unsigned RxStatusAndLength;
  tEthBuffer *RxBufPtr;
  tEthBuffer *emptyBuf;
#if SoftwareCRC  
  unsigned crc, crclen;
  int crcOK;
#else
# define crcOK 1
#endif

  while (1)
    {
      if (rxReadPointer->FrameDataPtr & FRM_OWNERSHIP_BDMA)
        {
          // queue is empty -- make sure Rx is running
          if (!(BDMARXCON & BDMARXCON_EN))
            {
              ++BDMARxErrCnt.queueOverflow;
              BDMARXCON |= BDMARXCON_EN;
            }
          return NULL;
        }
  
      RxBufPtr = (tEthBuffer*)rxReadPointer->FrameDataPtr;
      RxStatusAndLength = rxReadPointer->StatusAndFrameLength;
      
      // counting on short-circuit && evaluation below to only
      // allocate a fresh buffer if rx packet is good!!

#if defined (CYGPKG_NET)      
      ++ifStats.rx_count;
#endif      
      
#if SoftwareCRC
      crclen = (RxStatusAndLength & 0xffff) - 4;
      crc = cyg_ether_crc32(RxBufPtr->data+2,crclen);
      crcOK = ((U08)(crc>>0) == RxBufPtr->data[2+crclen+0] &&
               (U08)(crc>>8) == RxBufPtr->data[2+crclen+1] &&
               (U08)(crc>>16) == RxBufPtr->data[2+crclen+2] &&
               (U08)(crc>>24) == RxBufPtr->data[2+crclen+3]);
#endif
      if ((RxStatusAndLength & (RXFDSTAT_GOOD<<16)) 
          && crcOK
          && (emptyBuf = allocBuffer()))
        {
          // good packet and we've got a fresh buffer to take
          // it's place in the receive queue
          rxReadPointer->FrameDataPtr = (unsigned)emptyBuf | FRM_OWNERSHIP_BDMA;
          rxReadPointer = rxReadPointer->NextFD;
          RxBufPtr->length = RxStatusAndLength & 0xffff;
#if defined(CYGPKG_NET)
          ++ifStats.rx_deliver;
#endif          
          return RxBufPtr;
        }
      else
        {
          // bad packet or out of buffers.  either way we
          // ignore this packet, and reuse the buffer
#if defined(CYGPKG_NET)
          if (RxStatusAndLength & (RXFDSTAT_GOOD<<16) && crcOK)
            ++ifStats.rx_resource;
          else
            ++ifStats.rx_crc_errors;
#endif          
          rxReadPointer->FrameDataPtr |= FRM_OWNERSHIP_BDMA;
          rxReadPointer = rxReadPointer->NextFD;
        }
    }
}

//======================================================================
static int EthInit(U08* mac_address)
{
#if CYGINT_DEVS_ETH_ARM_KS32C5000_PHY && defined(CYGPKG_NET)
  unsigned linkStatus;
#endif

  if (mac_address)
    debug2_printf("EthInit(%02x:%02x:%02x:%02x:%02x:%02x)\n",
                mac_address[0],mac_address[1],mac_address[2],
                mac_address[3],mac_address[4],mac_address[5]);
  else
    debug2_printf("EthInit(NULL)\n");

#if CYGINT_DEVS_ETH_ARM_KS32C5000_PHY
  PhyReset();
#endif
  
  /* Set the initial condition of the BDMA. */
  BDMARXCON = BDMARXCON_RESET;
  BDMATXCON = BDMATXCON_RESET;
  BDMARXLSZ = MAX_ETH_FRAME_SIZE;

  BDMARXPTR = (U32)rxReadPointer;
  BDMATXPTR = (U32)txWritePointer;
  
  MACCON = MACON_SW_RESET;
  MACCON = MACConfigVar;
	 
  CAMCON = CAMConfigVar;
  
  // set up our MAC address
  if (mac_address)
    {
      *((volatile U32*)CAM_BaseAddr) = 
        (mac_address[0]<<24) |
        (mac_address[1]<<16) |
        (mac_address[2]<< 8) |
        (mac_address[3]<< 0);
      *((volatile U16*)(CAM_BaseAddr+4)) =
        (mac_address[4]<< 8) |
        (mac_address[5]<< 0);
    }
  
  // CAM Enable
  CAMEN = 0x0001;
  
  // update the Configuration of BDMA and MAC to begin Rx/Tx

  BDMARXCON = BDMARxConfigVar;
  MACRXCON = MACRxConfigVar;

  BDMATXCON = BDMATxConfigVar;
  MACTXCON =  MACTxConfigVar;

    debug2_printf("ks32C5000 eth: %02x:%02x:%02x:%02x:%02x:%02x  ",
              *((volatile unsigned char*)CAM_BaseAddr+0),
              *((volatile unsigned char*)CAM_BaseAddr+1),
              *((volatile unsigned char*)CAM_BaseAddr+2),
              *((volatile unsigned char*)CAM_BaseAddr+3),
              *((volatile unsigned char*)CAM_BaseAddr+4),
              *((volatile unsigned char*)CAM_BaseAddr+5));

#if CYGINT_DEVS_ETH_ARM_KS32C5000_PHY && defined(CYGPKG_NET)
  // Read link status to be up to date
  linkStatus = PhyStatus();
  if (linkStatus & PhyStatus_FullDuplex)
    ifStats.duplex = 3;
  else
    ifStats.duplex = 2;

  if (linkStatus & PhyStatus_LinkUp)
    ifStats.operational = 3;
  else
    ifStats.operational = 2;

  if (linkStatus & PhyStatus_100Mb)
    ifStats.speed = 100000000;
  else
    ifStats.speed = 10000000;
#endif

#if SoftwareCRC
  debug2_printf("Software CRC\n");
#else
  debug2_printf("Hardware CRC\n");
#endif
  
  return 0;
}

//======================================================================
static void Init_TxFrameDescriptorArray(void)
{
  FRAME_DESCRIPTOR *pFrameDescriptor;
  int i;
  
  // Each Frame Descriptor's frame data pointer points is NULL
  // if not in use, otherwise points to an ethBuffer

  pFrameDescriptor = txFrameDescrArray;
  
  for(i=0; i < MAX_TX_FRAME_DESCRIPTORS; i++) 
    {
      pFrameDescriptor->FrameDataPtr = 0;
      pFrameDescriptor->Reserved = 0;
      pFrameDescriptor->StatusAndFrameLength = 0;
      pFrameDescriptor->NextFD = pFrameDescriptor+1;
      pFrameDescriptor++;
    }

  // fix up the last pointer to loop back to the first

  txFrameDescrArray[MAX_TX_FRAME_DESCRIPTORS-1].NextFD = txFrameDescrArray;
  
  txDonePointer = txWritePointer = txFrameDescrArray;
  
  return;
}


//======================================================================
static void Init_RxFrameDescriptorArray(void)
{
  FRAME_DESCRIPTOR *pFrameDescriptor;
  int i;
  
  // Each Frame Descriptor's frame data pointer points to
  // an ethBuffer struct

  pFrameDescriptor = rxFrameDescrArray;
  
  for(i=0; i < MAX_RX_FRAME_DESCRIPTORS; i++) 
    {
      pFrameDescriptor->FrameDataPtr = ((unsigned)allocBuffer() | FRM_OWNERSHIP_BDMA);
      pFrameDescriptor->Reserved = 0;
      pFrameDescriptor->StatusAndFrameLength = 0;
      pFrameDescriptor->NextFD = pFrameDescriptor+1;
      pFrameDescriptor++;
    }
  
  // fix up the last pointer to loop back to the first

  rxFrameDescrArray[MAX_RX_FRAME_DESCRIPTORS-1].NextFD = rxFrameDescrArray;
  
  rxReadPointer = rxFrameDescrArray;
  
  return;
}

#if CYGINT_DEVS_ETH_ARM_KS32C5000_PHY

#if HavePHYinterrupt
static unsigned linkStatus;

static cyg_uint32 MAC_Phy_isr(cyg_vector_t vector, cyg_addrword_t data)
{
  cyg_drv_interrupt_acknowledge(vector);
  PhyInterruptAck();
  ++ks5000_MAC_Phy_Cnt;
  linkStatus = PhyStatus();
  if (linkStatus & PhyStatus_FullDuplex)  
    MACConfigVar |= (MACON_FULL_DUP);
  else
    MACConfigVar &= ~(MACON_FULL_DUP);
  
#if defined(CYGPKG_NET)      
  if (linkStatus & PhyStatus_FullDuplex)
    ifStats.duplex = 3;
  else
    ifStats.duplex = 2;
  
  if (linkStatus & PhyStatus_LinkUp)
    ifStats.operational = 3;
  else
    ifStats.operational = 2;

  if (linkStatus & PhyStatus_100Mb)
    ifStats.speed = 100000000;
  else
    ifStats.speed = 10000000;
#endif  
  
  MACCON = MACConfigVar;
  return CYG_ISR_HANDLED;
}
#endif
#endif

static void ks32c5000_handle_tx_complete(void)
{

  // record status and then free any buffers we're done with
  
  while (txDonePointer->FrameDataPtr && !(txDonePointer->FrameDataPtr & FRM_OWNERSHIP_BDMA))
    {
#if defined(CYGPKG_NET)
      U32 txStatus;
        
      txStatus = txDonePointer->StatusAndFrameLength>>16;
    
      ++ks5000_MAC_Tx_Cnt;
      ++ifStats.interrupts;
  
      if (txStatus & MACTXSTAT_COMP)
        ++ifStats.tx_complete;
      
      if (txStatus & (MACTXSTAT_EX_COLL | MACTXSTAT_DEFFERED |
                            MACTXSTAT_UNDER | MACTXSTAT_DEFER |
                            MACTXSTAT_NCARR | MACTXSTAT_SIG_QUAL |
                            MACTXSTAT_LATE_COLL | MACTXSTAT_PAR |
                            MACTXSTAT_PAUSED | MACTXSTAT_HALTED)) 
        {
          // transmit failed, log errors
          if (txStatus & MACTXSTAT_EX_COLL)
            ++ifStats.tx_max_collisions;
          if (txStatus & MACTXSTAT_DEFFERED)
            ++ifStats.tx_deferred;
          if (txStatus & MACTXSTAT_UNDER)
            ++ifStats.tx_underrun;
          if (txStatus & MACTXSTAT_DEFER)
            ;
          if (txStatus & MACTXSTAT_NCARR)
            ++ifStats.tx_carrier_loss;
          if (txStatus & MACTXSTAT_SIG_QUAL)
            ++ifStats.tx_sqetesterrors;
          if (txStatus & MACTXSTAT_LATE_COLL)
            ++ifStats.tx_late_collisions;
          if (txStatus & MACTXSTAT_PAR)
            ;
          if (txStatus & MACTXSTAT_PAUSED)
            ;
          if (txStatus & MACTXSTAT_HALTED)
            ;
        }
      else
        {
          // transmit OK
          int collisionCnt = txStatus & 0x0f;
          ++ifStats.tx_good;
          if (collisionCnt)
            {
              if (collisionCnt == 1)
                ++ifStats.tx_single_collisions;
              else
                ++ifStats.tx_mult_collisions;
              ifStats.tx_total_collisions += collisionCnt;
            }
        }
#endif      
      isrFreeBuffer((tEthBuffer*)txDonePointer->FrameDataPtr);
      txDonePointer->FrameDataPtr = 0;
      txDonePointer = txDonePointer->NextFD;
    }
}


//======================================================================
static cyg_uint32 MAC_Tx_isr(cyg_vector_t vector, cyg_addrword_t data)
{
  cyg_drv_interrupt_acknowledge(vector);
  ks32c5000_handle_tx_complete();
  return CYG_ISR_HANDLED;
}

static unsigned accumulatedMaxRxStatus=0;

//======================================================================
static cyg_uint32 MAC_Rx_isr(cyg_vector_t vector, cyg_addrword_t data)
{
  U32 IntMACRxStatus;
  
  cyg_drv_interrupt_acknowledge(vector);
  
  IntMACRxStatus = MACRXSTAT;
  MACRXSTAT = IntMACRxStatus;

  accumulatedMaxRxStatus |= IntMACRxStatus;
  
  ++ks5000_MAC_Rx_Cnt;

#if defined(CYGPKG_NET)
  
  ++ifStats.interrupts;

  if (IntMACRxStatus & MACRXSTAT_GOOD) 
    {
      ++ifStats.rx_good;
      
      if (IntMACRxStatus & MACRXSTAT_CTL_RECD)
        ;  // we don't do anything with control packets
      
      return CYG_ISR_HANDLED;
    }

  if (IntMACRxStatus & (MACRXSTAT_ALLIGN_ERR | MACRXSTAT_CRC_ERR |
                        MACRXSTAT_OVERFLOW | MACRXSTAT_LONG_ERR |
                        MACRXSTAT_PAR | MACRXSTAT_HALTED) ) 
    {
      if (IntMACRxStatus & MACRXSTAT_ALLIGN_ERR)
        ++ifStats.rx_align_errors;
      if (IntMACRxStatus & MACRXSTAT_CRC_ERR)
        ++ifStats.rx_crc_errors;
      if (IntMACRxStatus & MACRXSTAT_OVERFLOW)
        ++ifStats.rx_overrun_errors;
      if (IntMACRxStatus & MACRXSTAT_LONG_ERR)
        ++ifStats.rx_too_long_frames;
      if (IntMACRxStatus & MACRXSTAT_PAR)
        ++ifStats.rx_symbol_errors;
      if (IntMACRxStatus & MACRXSTAT_HALTED)
        ;
    }
#endif  
  return CYG_ISR_HANDLED;
}



//======================================================================
// This interrupt only happens when errors occur
static cyg_uint32 BDMA_Tx_isr(cyg_vector_t vector, cyg_addrword_t data)
{
  U32 IntBDMATxStatus;

  cyg_drv_interrupt_acknowledge(vector);
  
  IntBDMATxStatus = BDMASTAT;
  BDMASTAT = IntBDMATxStatus;

  ++ks5000_BDMA_Tx_Isr_Cnt;
#if defined(CYGPKG_NET)  
  ++ifStats.interrupts;
#endif  
  if (IntBDMATxStatus & BDMASTAT_TX_CCP) 
    {
      debug1_printf("+-- Control Packet Transfered : %x\r",ERMPZCNT);
      debug1_printf("    Tx Control Frame Status : %x\r",ETXSTAT);
    }

  if (IntBDMATxStatus & (BDMASTAT_TX_NL|BDMASTAT_TX_NO|BDMASTAT_TX_EMPTY) )
    {
      if (IntBDMATxStatus & BDMASTAT_TX_NL)
        BDMATxErrCnt.BTxNLErr++;
      if (IntBDMATxStatus & BDMASTAT_TX_NO)
        BDMATxErrCnt.BTxNOErr++;
      if (IntBDMATxStatus & BDMASTAT_TX_EMPTY)
        BDMATxErrCnt.BTxEmptyErr++;
    }
  
  // free any buffers we're done with

  while (txDonePointer->FrameDataPtr && !(txDonePointer->FrameDataPtr & FRM_OWNERSHIP_BDMA))
    {
      freeBuffer((tEthBuffer*)txDonePointer->FrameDataPtr);
      txDonePointer->FrameDataPtr = 0;
      txDonePointer = txDonePointer->NextFD;
    }

  // don't call tx dsr for now -- it has nothing to do

  return CYG_ISR_HANDLED;
}


static void BDMA_Tx_dsr(cyg_vector_t vector, cyg_ucount32 count, cyg_addrword_t data)
{
  ++ks5000_BDMA_Tx_Dsr_Cnt;
}



//======================================================================
static cyg_uint32 BDMA_Rx_isr(cyg_vector_t vector, cyg_addrword_t data)
{
  U32 IntBDMARxStatus;
  
  cyg_drv_interrupt_acknowledge(vector);
  
  IntBDMARxStatus = BDMASTAT;
  BDMASTAT = IntBDMARxStatus;

  ++ks5000_BDMA_Rx_Isr_Cnt;
#if defined(CYGPKG_NET)  
  ++ifStats.interrupts;
#endif
  if (IntBDMARxStatus & (BDMASTAT_RX_NL  | BDMASTAT_RX_NO |
                         BDMASTAT_RX_MSO | BDMASTAT_RX_EMPTY |
                         BDMASTAT_RX_SEARLY) )
    {
//      printf("RxIsr %u\r\n", (unsigned)cyg_current_time());
      if (IntBDMARxStatus & BDMASTAT_RX_NL)
        BDMARxErrCnt.BRxNLErr++;
      if (IntBDMARxStatus & BDMASTAT_RX_NO)
        BDMARxErrCnt.BRxNOErr++;
      if (IntBDMARxStatus & BDMASTAT_RX_MSO)
        BDMARxErrCnt.BRxMSOErr++;
      if (IntBDMARxStatus & BDMASTAT_RX_EMPTY)
        BDMARxErrCnt.BRxEmptyErr++;
      if (IntBDMARxStatus & BDMASTAT_RX_SEARLY)
        BDMARxErrCnt.sBRxSEarly++;
    }
  
  return CYG_ISR_HANDLED|CYG_ISR_CALL_DSR;
}

static void eth_handle_recv_buffer(tEthBuffer*);

static cyg_handle_t  bdmaRxIntrHandle;
static cyg_handle_t  bdmaTxIntrHandle;
static cyg_handle_t  macRxIntrHandle;
static cyg_handle_t  macTxIntrHandle;

static cyg_interrupt  bdmaRxIntrObject;
static cyg_interrupt  bdmaTxIntrObject;
static cyg_interrupt  macRxIntrObject;
static cyg_interrupt  macTxIntrObject;

static int ethernetRunning;

#if HavePHYinterrupt
static cyg_handle_t  macPhyIntrHandle;
static cyg_interrupt  macPhyIntrObject;
#endif

static void ks32c5000_eth_deliver(struct eth_drv_sc *sc)
{
  unsigned short p;
  tEthBuffer *rxBuffer;
  extern void cyg_interrupt_post_dsr(CYG_ADDRWORD intr_obj);
  
  ++ks5000_BDMA_Rx_Dsr_Cnt;
  
  while (1)
    {
      if (!rx_frame_avail())
        {
          // no more frames
          return;
        }
      
      if (!(rxBuffer=ks32c5000_eth_get_recv_buffer()))
        {
          // no buffers available
          return;
        }
      
      p = *((unsigned short*)(rxBuffer->data+EtherFramePadding+ETH_HEADER_SIZE-2));
      
      if (ethernetRunning)
        eth_handle_recv_buffer(rxBuffer);
      else
        freeBuffer(rxBuffer);
    }
}


static void installInterrupts(void)
{
  extern struct eth_drv_sc ks32c5000_sc;
  static bool firstTime=true;

  debug1_printf("ks5000_ether: installInterrupts()\n");
  
  if (!firstTime)
    return;
  firstTime = false;
  
  initFreeList();
  Init_RxFrameDescriptorArray();
  Init_TxFrameDescriptorArray();
  
  BDMARXPTR = (U32)rxReadPointer;
  BDMATXPTR = (U32)txWritePointer;
  
  cyg_drv_mutex_init(&txMutex);
  cyg_drv_mutex_init(&oldRxMutex);
  cyg_drv_cond_init(&oldRxCond,&oldRxMutex);

  cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_ETH_BDMA_RX,0,(unsigned)&ks32c5000_sc,BDMA_Rx_isr,eth_drv_dsr,&bdmaRxIntrHandle,&bdmaRxIntrObject);
  cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_ETH_BDMA_TX,0,0,BDMA_Tx_isr,BDMA_Tx_dsr,&bdmaTxIntrHandle,&bdmaTxIntrObject);
  cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_ETH_MAC_RX,0,0,MAC_Rx_isr,NULL,&macRxIntrHandle,&macRxIntrObject);
  cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_ETH_MAC_TX,0,0,MAC_Tx_isr,NULL,&macTxIntrHandle,&macTxIntrObject);
#if HavePHYinterrupt
  cyg_drv_interrupt_create(CYGNUM_HAL_INTERRUPT_EXT0,0,0,MAC_Phy_isr,NULL,&macPhyIntrHandle,&macPhyIntrObject);
  cyg_drv_interrupt_attach(macPhyIntrHandle);
#endif  
  
  cyg_drv_interrupt_attach(bdmaRxIntrHandle);
  cyg_drv_interrupt_attach(bdmaTxIntrHandle);
  cyg_drv_interrupt_attach(macRxIntrHandle);
  cyg_drv_interrupt_attach(macTxIntrHandle);
  
#if HavePHYinterrupt
  cyg_drv_interrupt_acknowledge(CYGNUM_HAL_INTERRUPT_EXT0);
  cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_EXT0);
#endif  
}

//======================================================================
// Driver code that interfaces to the TCP/IP stack via the common
// Ethernet interface.

    
// don't have any private data, but if we did, this is where it would go
typedef struct
{
  int j;
}ks32c5000_priv_data_t;

ks32c5000_priv_data_t ks32c5000_priv_data;    

#define eth_drv_tx_done(sc,key,retval) (sc)->funs->eth_drv->tx_done(sc,key,retval)
#define eth_drv_init(sc,enaddr)  ((sc)->funs->eth_drv->init)(sc, enaddr)
#define eth_drv_recv(sc,len)  ((sc)->funs->eth_drv->recv)(sc, len)

static bool ks32c5000_eth_init(struct cyg_netdevtab_entry *tab)
{
  unsigned char myMacAddr[6] = { CYGPKG_DEVS_ETH_ARM_KS32C5000_MACADDR };
  struct eth_drv_sc *sc = (struct eth_drv_sc *)tab->device_instance;
  bool ok;

#ifdef CYGHWR_DEVS_ETH_ARM_KS32C5000_GET_ESA
  // Get MAC address from RedBoot configuration variables
  CYGHWR_DEVS_ETH_ARM_KS32C5000_GET_ESA(&myMacAddr[0], ok);
  // If this call fails myMacAddr is unchanged and MAC address from CDL is used
#endif

  debug1_printf("ks32c5000_eth_init()\n");
  debug1_printf("  MAC address %02x:%02x:%02x:%02x:%02x:%02x\n",myMacAddr[0],myMacAddr[1],myMacAddr[2],myMacAddr[3],myMacAddr[4],myMacAddr[5]);
#if defined(CYGPKG_NET)  
  ifStats.duplex = 1;      //unknown
  ifStats.operational = 1; //unknown
  ifStats.tx_queue_len = MAX_TX_FRAME_DESCRIPTORS;
  strncpy(ifStats.description,"Ethernet device",sizeof(ifStats.description));
  ifStats.snmp_chipset[0] = 0;
  ifStats.supports_dot3 = 1; // support dot3
#endif  
  installInterrupts();
  EthInit(myMacAddr);
  cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_ETH_BDMA_RX);
  cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_ETH_BDMA_TX);
  cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_ETH_MAC_RX);
  cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_ETH_MAC_TX);
  configDone = 1;
  ethernetRunning = 1;
  eth_drv_init(sc, myMacAddr);
  return true;
}

static void ks32c5000_eth_start(struct eth_drv_sc *sc, unsigned char *enaddr, int flags)
{
  debug2_printf("ks32c5000_eth_start()\n");
  if (!ethernetRunning)
    {
      cyg_drv_interrupt_mask(CYGNUM_HAL_INTERRUPT_ETH_BDMA_RX);
      cyg_drv_interrupt_mask(CYGNUM_HAL_INTERRUPT_ETH_BDMA_TX);
      cyg_drv_interrupt_mask(CYGNUM_HAL_INTERRUPT_ETH_MAC_RX);
      cyg_drv_interrupt_mask(CYGNUM_HAL_INTERRUPT_ETH_MAC_TX);
      EthInit(enaddr);
      cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_ETH_BDMA_RX);
      cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_ETH_BDMA_TX);
      cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_ETH_MAC_RX);
      cyg_drv_interrupt_unmask(CYGNUM_HAL_INTERRUPT_ETH_MAC_TX);
      ethernetRunning = 1;
    }
}

static void ks32c5000_eth_stop(struct eth_drv_sc *sc)
{
  debug1_printf("ks32c5000_eth_stop()\n");
  ethernetRunning = 0;
}
 
static int ks32c5000_eth_control(struct eth_drv_sc *sc, 
                          unsigned long cmd, 
                          void *data, 
                          int len)
{
  switch (cmd)
    {
#if defined(CYGPKG_NET)      
     case ETH_DRV_GET_IF_STATS_UD:
     case ETH_DRV_GET_IF_STATS:
        {
          struct ether_drv_stats *p = (struct ether_drv_stats*)data;

#if CYGINT_DEVS_ETH_ARM_KS32C5000_PHY
          unsigned linkStatus;

          // Read link status to be up to date
          linkStatus = PhyStatus();
          if (linkStatus & PhyStatus_FullDuplex)
            ifStats.duplex = 3;
          else
            ifStats.duplex = 2;

          if (linkStatus & PhyStatus_LinkUp)
            ifStats.operational = 3;
          else
            ifStats.operational = 2;

          if (linkStatus & PhyStatus_100Mb)
            ifStats.speed = 100000000;
          else
            ifStats.speed = 10000000;
#endif

          *p = ifStats;
          return 0;
        }
#endif      
     case ETH_DRV_SET_MAC_ADDRESS: {
         int act;

         if (ETHER_ADDR_LEN != len)
             return -1;
         debug1_printf("ks32c5000_eth_control: ETH_DRV_SET_MAC_ADDRESS.\n");
         act = ethernetRunning;
         ks32c5000_eth_stop(sc);
         ks32c5000_eth_start(sc, data, 0);
         ethernetRunning = act;
         return 0;
     }
#ifdef	ETH_DRV_GET_MAC_ADDRESS
     case ETH_DRV_GET_MAC_ADDRESS: {
         if (len < ETHER_ADDR_LEN)
             return -1;
         debug1_printf("ks32c5000_eth_control: ETH_DRV_GET_MAC_ADDRESS.\n");
         memcpy(data, (void *)CAM_BaseAddr, ETHER_ADDR_LEN);
         return 0;
     }
#endif
     default:
      return -1;
    }
}

static int ks32c5000_eth_can_send_count=0;
static int ks32c5000_eth_can_send_count_OK=0;

// In case there are multiple Tx frames waiting, we should
// return how many empty Tx spots we have.  For now we just
// return 0 or 1.
static int ks32c5000_eth_can_send(struct eth_drv_sc *sc)
{
  FRAME_DESCRIPTOR *TxFp, *StartFp;
  
  // find the next unused spot in the queue

  ++ks32c5000_eth_can_send_count;
  
  StartFp = TxFp = (FRAME_DESCRIPTOR*)BDMATXPTR;
  
  while (TxFp->FrameDataPtr & FRM_OWNERSHIP_BDMA)
    {
      TxFp = TxFp->NextFD;
      if (TxFp == StartFp)
        return 0;
    }
  ++ks32c5000_eth_can_send_count_OK;
  return 1;
}

static int ks5000_eth_send_count=0;

static void ks32c5000_eth_send(struct eth_drv_sc *sc,
                               struct eth_drv_sg *sg_list,
                               int sg_len,
                               int total_len,
                               unsigned long key)
{
  unsigned char *dest;
  unsigned        len;
  tEthBuffer *buf;

  if (total_len >= MAX_ETH_FRAME_SIZE)
    {
      eth_drv_tx_done(sc,key,-EINVAL);
      return;
    }

  ++ks5000_eth_send_count;

  // allocate buffer

  buf = allocBuffer();
  if (!buf)
    {
      // out of buffers
      eth_drv_tx_done(sc,key,-EIO);
      return;
    }

  // copy data from scatter/gather list into BDMA data buffer

  len = 0;
  dest = buf->data;
  
  while (sg_len)
    {
      memcpy(dest,(unsigned char*)sg_list->buf,sg_list->len);
      len += sg_list->len;
      dest += sg_list->len;
      ++sg_list;
      --sg_len;
    }
  
  buf->length = len;
  
  // tell upper layer that we're done with this sglist

  eth_drv_tx_done(sc,key,0);
  
  // queue packet for transmit
  
  while(!ks32c5000_eth_buffer_send(buf))
    {
#if defined(CYGPKG_KERNEL)
      // wait a tick and try again.
      cyg_thread_delay(1);
#else
      // toss it.
      freeBuffer(buf);
      break;
#endif      
    }
  
}


static int ks5000_eth_rcv_count=0;
static tEthBuffer *tcpIpRxBuffer;

// called from DSR
static void eth_handle_recv_buffer(tEthBuffer* rxBuffer)
{
  extern struct eth_drv_sc ks32c5000_sc;
  tcpIpRxBuffer = rxBuffer;
  eth_drv_recv(&ks32c5000_sc,tcpIpRxBuffer->length-4);  // discard 32-bit CRC
}

static void ks32c5000_eth_recv(struct eth_drv_sc *sc,
                        struct eth_drv_sg *sg_list,
                        int sg_len)
{
  unsigned char *source;

  ++ks5000_eth_rcv_count;
  
  if (!tcpIpRxBuffer)
      return;  // no packet waiting, shouldn't be here!

  // copy data from eth buffer into scatter/gather list

  source = tcpIpRxBuffer->data + EtherFramePadding;
  
  while (sg_len)
    {
	  if (sg_list->buf)
		memcpy((unsigned char*)sg_list->buf,source,sg_list->len);
      source += sg_list->len;
      ++sg_list;
      --sg_len;
    }

  freeBuffer(tcpIpRxBuffer);
  tcpIpRxBuffer = NULL;
  return;
}

// routine called to handle ethernet controller in polled mode
static void ks32c5000_eth_poll(struct eth_drv_sc *sc)
{
  BDMA_Rx_isr(CYGNUM_HAL_INTERRUPT_ETH_BDMA_RX, 0); // Call ISR routine
  ks32c5000_eth_deliver(sc);  // handle rx frames
  ks32c5000_handle_tx_complete();
}

static int ks32c5000_eth_int_vector(struct eth_drv_sc *sc)
{
  return CYGNUM_HAL_INTERRUPT_ETH_BDMA_RX;
}


ETH_DRV_SC(ks32c5000_sc,
           &ks32c5000_priv_data, // Driver specific data
           "eth0",                // Name for this interface
           ks32c5000_eth_start,
           ks32c5000_eth_stop,
           ks32c5000_eth_control,
           ks32c5000_eth_can_send,
           ks32c5000_eth_send,
           ks32c5000_eth_recv,
           ks32c5000_eth_deliver,
           ks32c5000_eth_poll,
           ks32c5000_eth_int_vector
           );

NETDEVTAB_ENTRY(ks32c5000_netdev, 
                "ks32c5000", 
                ks32c5000_eth_init, 
                &ks32c5000_sc);

// EOF ks5000_ether.c
