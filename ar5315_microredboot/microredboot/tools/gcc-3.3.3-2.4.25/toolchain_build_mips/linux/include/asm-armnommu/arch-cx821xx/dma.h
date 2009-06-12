/****************************************************************************
*
*	Name:			dma.h
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


#ifndef __ASM_ARCH_DMA_H
#define __ASM_ARCH_DMA_H


#define MAX_DMA_ADDRESS			0x0F00000

#define EMAC1_TxD_DMA_CH		1

#define DMAC_1_Ptr1				0x300000
#define DMAC_1_Ptr2				0x300030
#define DMAC_1_Cnt1				0x300060
#define DMAC_1_Cnt2				0x300090

#define EMAC1_RxD_DMA_CH		2

#define DMAC_2_Ptr1				0x300004
#define DMAC_2_Ptr2				0x300034
#define DMAC_2_Cnt1				0x300064
#define DMAC_2_Cnt2				0x300094

#define EMAC2_TxD_DMA_CH		3

#define DMAC_3_Ptr1				0x300008
#define DMAC_3_Ptr2				0x300038
#define DMAC_3_Cnt1				0x300068
#define DMAC_3_Cnt2				0x300098

#define EMAC2_RxD_DMA_CH		4

#define DMAC_4_Ptr1				0x30000c
#define DMAC_4_Ptr2				0x30003c
#define DMAC_4_Cnt1				0x30006c
#define DMAC_4_Cnt2				0x30009c


#define MAX_DMA_CHANNELS		13

#define arch_dma_init(dma_chan)

//-----------------------------------------------------------------------------

#define DMA_HW_BASE   0x00300000

#define DMA1_PTR1     ((volatile UINT32 *) DMA_HW_BASE)            /* EMAC1 TX */
#define DMA1_PTR2     ((volatile UINT32 *) (DMA_HW_BASE + 0x30))
#define DMA1_CNT1     ((volatile UINT32 *) (DMA_HW_BASE + 0x60))
#define DMA2_PTR1     ((volatile UINT32 *) (DMA_HW_BASE + 0x04))   /* EMAC1 RX */
#define DMA2_PTR2     ((volatile UINT32 *) (DMA_HW_BASE + 0x34))
#define DMA2_CNT1     ((volatile UINT32 *) (DMA_HW_BASE + 0x64))

#define DMA3_PTR1     ((volatile UINT32 *) (DMA_HW_BASE + 0x08))   /* EMAC2 TX */
#define DMA3_PTR2     ((volatile UINT32 *) (DMA_HW_BASE + 0x38))
#define DMA3_CNT1     ((volatile UINT32 *) (DMA_HW_BASE + 0x68))
#define DMA4_PTR1     ((volatile UINT32 *) (DMA_HW_BASE + 0x0C))   /* EMAC2 RX */
#define DMA4_PTR2     ((volatile UINT32 *) (DMA_HW_BASE + 0x3C))
#define DMA4_CNT1     ((volatile UINT32 *) (DMA_HW_BASE + 0x6C))

#define DMA5_PTR1     ((volatile UINT32 *) (DMA_HW_BASE + 0x10))   /* DSL TX */
#define DMA5_CNT1     ((volatile UINT32 *) (DMA_HW_BASE + 0x70))
#define DMA6_PTR1     ((volatile UINT32 *) (DMA_HW_BASE + 0x14))   /* DSL RX */
#define DMA6_CNT1     ((volatile UINT32 *) (DMA_HW_BASE + 0x74))

#define DMA7_PTR1     ((volatile UINT32 *) (DMA_HW_BASE + 0x18))   /* M2M In (SRC)   */
#define DMA7_CNT1     ((volatile UINT32 *) (DMA_HW_BASE + 0x78))
#define DMA8_PTR1     ((volatile UINT32 *) (DMA_HW_BASE + 0x1C))   /* M2M Out (DEST) */
#define DMA8_CNT1     ((volatile UINT32 *) (DMA_HW_BASE + 0x7C))


#define DMA9_PTR1  	 ((volatile UINT32 *) (DMA_HW_BASE + 0x20))   /* USB EP3 TX */
#define DMA9_CNT1  	 ((volatile UINT32 *) (DMA_HW_BASE + 0x80))
#define DMA10_PTR1 	 ((volatile UINT32 *) (DMA_HW_BASE + 0x24))   /* USB EP2 TX */
#define DMA10_CNT1 	 ((volatile UINT32 *) (DMA_HW_BASE + 0x84))
#define DMA11_PTR1 	 ((volatile UINT32 *) (DMA_HW_BASE + 0x28))   /* USB EP1 TX */
#define DMA11_CNT1 	 ((volatile UINT32 *) (DMA_HW_BASE + 0x88))
#define DMA12_PTR1 	 ((volatile UINT32 *) (DMA_HW_BASE + 0x100))  /* USB RX */
#define DMA12_CNT1 	 ((volatile UINT32 *) (DMA_HW_BASE + 0x110))
#define DMA13_PTR1 	 ((volatile UINT32 *) (DMA_HW_BASE + 0x104))  /* USB EP0 TX */
#define DMA13_CNT1 	 ((volatile UINT32 *) (DMA_HW_BASE + 0x114))

#endif /* _ASM_ARCH_DMA_H */
