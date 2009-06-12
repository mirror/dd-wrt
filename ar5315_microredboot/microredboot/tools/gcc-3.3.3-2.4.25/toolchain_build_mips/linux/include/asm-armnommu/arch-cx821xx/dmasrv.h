/****************************************************************************
*
*	Name:			dmasrv.h
*
*	Description:	
*
*	Copyright:		(c) 2002 Conexant Systems Inc.
*
*****************************************************************************

  This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option)
any later version.

  This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

  You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc., 59
Temple Place, Suite 330, Boston, MA 02111-1307 USA

****************************************************************************
*  $Author: gerg $
*  $Revision: 1.1 $
*  $Modtime: 11/08/02 12:50p $
****************************************************************************/

#ifndef DMASRV_H
#define DMASRV_H

#define SIMULATION_ENABLE        (1)
#define DBG_ENABLE_LOOPBACK      (1 && SIMULATION_ENABLE)
#define DBG_ENABLE_USB_LOOPBACK  (0 && DBG_ENABLE_LOOPBACK)

#ifdef BM_LOOP
#define DBG_ENABLE_DSL_LOOPBACK  (1 && DBG_ENABLE_LOOPBACK)
#else
#define DBG_ENABLE_DSL_LOOPBACK  (0 && DBG_ENABLE_LOOPBACK)
#endif

#define DBG_ENABLE_USB_TRANSMISSON_SIMULATION      (1 && SIMULATION_ENABLE)


#define DMA_BLK_SIZE 56

#define DMA_OPEN  TRUE
#define DMA_CLOSED   FALSE
#define DMA_ENABLED  TRUE
#define DMA_DISBLED  FALSE

enum DMA_CHANNEL_NUM
{
   DMA_CHANNEL_EMAC1_TX = 1,
   DMA_CHANNEL_EMAC1_RX,
   DMA_CHANNEL_EMAC2_TX,
   DMA_CHANNEL_EMAC2_RX,
   DMA_CHANNEL_DSL_TX,
   DMA_CHANNEL_DSL_RX,
   DMA_CHANNEL_M2M_IN,
   DMA_CHANNEL_M2M_OUT,
   DMA_CHANNEL_USB_TX_EP3,
   DMA_CHANNEL_USB_TX_EP2,
   DMA_CHANNEL_USB_TX_EP1,
   DMA_CHANNEL_USB_TX_EP0,
   DMA_CHANNEL_USB_RX_EP3,
   DMA_CHANNEL_USB_RX_EP2,
   DMA_CHANNEL_USB_RX_EP1,
   DMA_CHANNEL_USB_RX_EP0,
   DMA_CHANNEL_MAX
};

typedef unsigned char (*PDMACALLBACK)(void* pUserData, UINT32* pBuffAddr, UINT32 uBuffSize);

typedef struct tagDMA_CHANNEL
{
   unsigned char bOpen;                /* TRUE = channel open, FALSE = closed */
   unsigned char bEnabled;             /* TRUE = enabled, FALSE = disabled */
   UINT32* pBuffAddr;         /* Pointer to DMA buffer */
   UINT32 uBuffSize;          /* DMA buffer size, in bytes */
   PDMACALLBACK pDMACallback; /* DMA IRQ callback function pointer */
   void* pUserData;           /* DMA IRQ callback function parameter */
} DMA_CHANNEL;

extern DMA_CHANNEL DmaChannelTbl[ DMA_CHANNEL_MAX ];

void DmaInit( void );
BOOL DmaSendBuffer(UINT32 uDMACh, UINT32* pBuffer, UINT32	uBufferSize);
BOOL DmaEnableChannel( UINT32 uDMACh );
BOOL DmaDisableChannel( UINT32 uDMACh );
BOOL DmaOpenChannel( UINT32 uDMACh);
BOOL DmaCloseChannel( UINT32 uDMACh );

BOOL DMASetCallback( UINT8 uDMACh, PDMACALLBACK pDMACallback, void* pUserData );

BOOL AddDMABufferList ( UINT32 uDMACh,UINT32* pSBuff,UINT32 uBuffSize);
BOOL DMAFlushChannelBuff(UINT32 uDMACh);

#endif /* DMASRV_H */ 
