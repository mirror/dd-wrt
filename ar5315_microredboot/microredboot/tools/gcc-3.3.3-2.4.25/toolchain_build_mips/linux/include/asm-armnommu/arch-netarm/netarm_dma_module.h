/*
 * include/asm-armnommu/arch-netarm/netarm_dma_module.h
 *
 * Copyright (C) 2000 NETsilicon, Inc.
 * Copyright (C) 2000 WireSpeed Communications Corporation
 *
 * This software is copyrighted by WireSpeed. LICENSEE agrees that
 * it will not delete this copyright notice, trademarks or protective
 * notices from any copy made by LICENSEE.
 *
 * This software is provided "AS-IS" and any express or implied 
 * warranties or conditions, including but not limited to any
 * implied warranties of merchantability and fitness for a particular
 * purpose regarding this software. In no event shall WireSpeed
 * be liable for any indirect, consequential, or incidental damages,
 * loss of profits or revenue, loss of use or data, or interruption
 * of business, whether the alleged damages are labeled in contract,
 * tort, or indemnity.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * author(s) : Joe deBlaquiere
 *             David Smith
 *
 * Updated by Arthur Shipkowski <art@videon-central.com>
 *  to offer the Forth-Systeme method of DMA configuration for their NS7520 
 *  Ethernet driver. Ideally, I'd like to deprecate the original method of 
 *  DMA configuration, which isn't quite as elegant as their method.
 *  
 */

#ifndef __NETARM_DMA_MODULE_REGISTERS_H
#define __NETARM_DMA_MODULE_REGISTERS_H

/* GEN unit register offsets */

#define	NETARM_DMA_MODULE_BASE		(0xFF900000)

#ifdef OLD_STYLE_NETARM_DMA_CONFIGURATION
#define get_dma_reg_addr(c) ((volatile unsigned int *)(NETARM_DMA_MODULE_BASE + (c)))

#define	NETARM_DMA1A_BFR_DESCRPTOR_PTR	(0x00)
#define	NETARM_DMA1A_CONTROL		(0x10)
#define	NETARM_DMA1A_STATUS		(0x14)
#define	NETARM_DMA1B_BFR_DESCRPTOR_PTR	(0x20)
#define	NETARM_DMA1B_CONTROL		(0x30)
#define	NETARM_DMA1B_STATUS		(0x34)
#define	NETARM_DMA1C_BFR_DESCRPTOR_PTR	(0x40)
#define	NETARM_DMA1C_CONTROL		(0x50)
#define	NETARM_DMA1C_STATUS		(0x54)
#define	NETARM_DMA1D_BFR_DESCRPTOR_PTR	(0x60)
#define	NETARM_DMA1D_CONTROL		(0x70)
#define	NETARM_DMA1D_STATUS		(0x74)

#define	NETARM_DMA2_BFR_DESCRPTOR_PTR	(0x80)
#define	NETARM_DMA2_CONTROL		(0x90)
#define	NETARM_DMA2_STATUS		(0x94)

#define	NETARM_DMA3_BFR_DESCRPTOR_PTR	(0xA0)
#define	NETARM_DMA3_CONTROL		(0xB0)
#define	NETARM_DMA3_STATUS		(0xB4)

#define	NETARM_DMA4_BFR_DESCRPTOR_PTR	(0xC0)
#define	NETARM_DMA4_CONTROL		(0xD0)
#define	NETARM_DMA4_STATUS		(0xD4)

#define	NETARM_DMA5_BFR_DESCRPTOR_PTR	(0xE0)
#define	NETARM_DMA5_CONTROL		(0xF0)
#define	NETARM_DMA5_STATUS		(0xF4)

#define	NETARM_DMA6_BFR_DESCRPTOR_PTR	(0x100)
#define	NETARM_DMA6_CONTROL		(0x110)
#define	NETARM_DMA6_STATUS		(0x114)

#define	NETARM_DMA7_BFR_DESCRPTOR_PTR	(0x120)
#define	NETARM_DMA7_CONTROL		(0x130)
#define	NETARM_DMA7_STATUS		(0x134)

#define	NETARM_DMA8_BFR_DESCRPTOR_PTR	(0x140)
#define	NETARM_DMA8_CONTROL		(0x150)
#define	NETARM_DMA8_STATUS		(0x154)

#define	NETARM_DMA9_BFR_DESCRPTOR_PTR	(0x160)
#define	NETARM_DMA9_CONTROL		(0x170)
#define	NETARM_DMA9_STATUS		(0x174)

#define	NETARM_DMA10_BFR_DESCRPTOR_PTR	(0x180)
#define	NETARM_DMA10_CONTROL		(0x190)
#define	NETARM_DMA10_STATUS		(0x194)

/* select bitfield defintions */

/* DMA Control Register ( 0xFF90_0XX0 ) */

#define NETARM_DMA_CTL_ENABLE		(0x80000000)

#define NETARM_DMA_CTL_ABORT		(0x40000000)

#define NETARM_DMA_CTL_BUS_100_PERCENT	(0x00000000)
#define NETARM_DMA_CTL_BUS_75_PERCENT	(0x10000000)
#define NETARM_DMA_CTL_BUS_50_PERCENT	(0x20000000)
#define NETARM_DMA_CTL_BUS_25_PERCENT	(0x30000000)

#define NETARM_DMA_CTL_BUS_MASK		(0x30000000)

#define NETARM_DMA_CTL_MODE_FB_TO_MEM	(0x00000000)
#define NETARM_DMA_CTL_MODE_FB_FROM_MEM	(0x04000000)
#define NETARM_DMA_CTL_MODE_MEM_TO_MEM	(0x08000000)

#define NETARM_DMA_CTL_BURST_NONE	(0x00000000)
#define NETARM_DMA_CTL_BURST_8_BYTE	(0x01000000)
#define NETARM_DMA_CTL_BURST_16_BYTE	(0x02000000)

#define NETARM_DMA_CTL_BURST_MASK	(0x03000000)

#define NETARM_DMA_CTL_SRC_INCREMENT	(0x00200000)

#define NETARM_DMA_CTL_DST_INCREMENT	(0x00100000)

/* these apply only to ext xfers on DMA 3 or 4 */

#define NETARM_DMA_CTL_CH_3_4_REQ_EXT	(0x00800000)

#define NETARM_DMA_CTL_CH_3_4_DATA_32	(0x00000000)
#define NETARM_DMA_CTL_CH_3_4_DATA_16	(0x00010000)
#define NETARM_DMA_CTL_CH_3_4_DATA_8	(0x00020000)

#define NETARM_DMA_CTL_STATE(X)	((X) & 0xFC00)
#define NETARM_DMA_CTL_INDEX(X)	((X) & 0x03FF)

/* DMA Status Register ( 0xFF90_0XX4 ) */

#define NETARM_DMA_STAT_NC_INTPEN	(0x80000000)
#define NETARM_DMA_STAT_EC_INTPEN	(0x40000000)
#define NETARM_DMA_STAT_NR_INTPEN	(0x20000000)
#define NETARM_DMA_STAT_CA_INTPEN	(0x10000000)
#define NETARM_DMA_STAT_INTPEN_MASK	(0xF0000000)

#define NETARM_DMA_STAT_NC_INT_EN	(0x00800000)
#define NETARM_DMA_STAT_EC_INT_EN	(0x00400000)
#define NETARM_DMA_STAT_NR_INT_EN	(0x00200000)
#define NETARM_DMA_STAT_CA_INT_EN	(0x00100000)
#define NETARM_DMA_STAT_INT_EN_MASK	(0x00F00000)

#define NETARM_DMA_STAT_WRAP		(0x00080000)
#define NETARM_DMA_STAT_IDONE		(0x00040000)
#define NETARM_DMA_STAT_LAST		(0x00020000)
#define NETARM_DMA_STAT_FULL		(0x00010000)

#define	NETARM_DMA_STAT_BUFLEN(X)	((X) & 0x7FFF)

/* DMA Buffer Descriptor Word 0 bitfields. */

#define NETARM_DMA_BD0_WRAP		(0x80000000)
#define NETARM_DMA_BD0_IDONE		(0x40000000)
#define NETARM_DMA_BD0_LAST		(0x20000000)
#define NETARM_DMA_BD0_BUFPTR_MASK	(0x1FFFFFFF)

/* DMA Buffer Descriptor Word 1 bitfields. */

#define NETARM_DMA_BD1_STATUS_MASK	(0xFFFF0000)
#define NETARM_DMA_BD1_FULL		(0x00008000)
#define NETARM_DMA_BD1_BUFLEN_MASK	(0x00007FFF)

#ifndef	__ASSEMBLER__

/*
 * Old FlyBy DMA Buffer Description
 */
typedef	struct __NETARM_DMA_Buff_Desc_FlyBy
{
	unsigned int word0;
	unsigned int word1;
} NETARM_DMA_Buff_Desc_FlyBy, *pNETARM_DMA_Buff_Desc_FlyBy ;

/*
 * Old Memory to Memory DMA Buffer Description
 */
typedef	struct __NETARM_DMA_Buff_Desc_M_to_M
{
	unsigned int word0;
	unsigned int word1;
	unsigned int word2;
	unsigned int word3;
} NETARM_DMA_Buff_Desc_M_to_M, *pNETARM_DMA_Buff_Desc_M_to_M ;

#endif /* !__ASSEMBLER__ */

#else

#define get_dma_reg_addr(c, reg) \
	((volatile unsigned int *)(NETARM_DMA_MODULE_BASE + \
		((unsigned int) (c)) + (reg)))

#define NETARM_DMA_DESC			(0x00000000) // descriptor pointer
#define NETARM_DMA_CONTROL		(0x00000010) // control register
#define NETARM_DMA_STATUS		(0x00000014) // status register

#define NETARM_DMA_CONTROL_CE		(0x80000000) // Channel enabled
#define NETARM_DMA_CONTROL_CA		(0x40000000) // Channel abort
#define NETARM_DMA_CONTROL_BB_MA	(0x30000000) // Bus bandwidth mask
#define NETARM_DMA_CONTROL_BB_25	(0x30000000) // 25% bandwidth
#define NETARM_DMA_CONTROL_BB_50	(0x20000000) // 50% bandwidth
#define NETARM_DMA_CONTROL_BB_75	(0x10000000) // 75% bandwidth
#define NETARM_DMA_CONTROL_BB_100	(0x00000000) // 100% bandwidth
#define NETARM_DMA_CONTROL_MODE_MA	(0x0C000000) // DMA operation mode
#define NETARM_DMA_CONTROL_MODE_RES	(0x0C000000) // Reserved
#define NETARM_DMA_CONTROL_MODE_MEM2MEM	(0x08000000) // Mem-to-Mem
#define NETARM_DMA_CONTROL_MODE_MEM2PER	(0x04000000) // Mem-to-Peripheral
#define NETARM_DMA_CONTROL_MODE_PER2MEM	(0x00000000) // Peripheral-to-Mem
#define NETARM_DMA_CONTROL_BTE_MA	(0x03000000) // Burst Transfer enable
#define NETARM_DMA_CONTROL_BTE_RES	(0x03000000) // Reserved
#define NETARM_DMA_CONTROL_BTE_4 	(0x02000000) // 4 operands
#define NETARM_DMA_CONTROL_BTE_2 	(0x01000000) // 2 operands
#define NETARM_DMA_CONTROL_BTE_1 	(0x00000000) // 1 operands
#define NETARM_DMA_CONTROL_REQ		(0x00800000) // Channel Request Source
#define NETARM_DMA_CONTROL_RES1		(0x00400000) // Reserved
#define NETARM_DMA_CONTROL_SINC		(0x00200000) // Source address increment
#define NETARM_DMA_CONTROL_DINC		(0x00100000) // Destination address incr
#define NETARM_DMA_CONTROL_RES2		(0x000C0000) // Reserved
#define NETARM_DMA_CONTROL_SIZE_MA	(0x00030000) // Data operand size
#define NETARM_DMA_CONTROL_SIZE_RES	(0x00030000) // Reserved
#define NETARM_DMA_CONTROL_SIZE_8   	(0x00020000) // 8 bit
#define NETARM_DMA_CONTROL_SIZE_16	(0x00010000) // 16 bit
#define NETARM_DMA_CONTROL_SIZE_32	(0x00000000) // 32 bit
#define NETARM_DMA_CONTROL_STATE_MA	(0x0000FC00) // DMA channel state
#define NETARM_DMA_CONTROL_STATE_UPDATE	(0x00008000) // Update
#define NETARM_DMA_CONTROL_STATE_SECOP  (0x00004000) // Mem-mem second operand
#define NETARM_DMA_CONTROL_STATE_FIROP	(0x00002000) // First operand
#define NETARM_DMA_CONTROL_STATE_LOADDE	(0x00001000) // Load Destination address
#define NETARM_DMA_CONTROL_STATE_LOADSO	(0x00000800) // Load Source address
#define NETARM_DMA_CONTROL_STATE_IDLE	(0x00000400) // IDLE
#define NETARM_DMA_CONTROL_INDEX	(0x000003FF) // buffer desciptor index

#define NETARM_DMA_CONTROL_CE		(0x80000000) // Channel enabled

// the status register bit fields

#define NETARM_DMA_STATUS_IP_MA         (0xF8000000) // clear pending mask
#define NETARM_DMA_STATUS_IP_NC  	(0x80000000) // normal int pending
#define NETARM_DMA_STATUS_IP_EC  	(0x40000000) // error int pending
#define NETARM_DMA_STATUS_IP_NR  	(0x20000000) // buffer not ready int pend
#define NETARM_DMA_STATUS_IP_CA  	(0x10000000) // channel abort int pend
#define NETARM_DMA_STATUS_IP_PC  	(0x08000000) // premature int pending
#define NETARM_DMA_STATUS_RES1		(0x06000000) // Reserved
#define NETARM_DMA_STATUS_IE_MA		(0x01F00000) // interrupt enable mask
#define NETARM_DMA_STATUS_IE_PC  	(0x01000000) // premature int enable
#define NETARM_DMA_STATUS_IE_NC   	(0x00800000) // normal int enable
#define NETARM_DMA_STATUS_IE_EC  	(0x00400000) // error int enable
#define NETARM_DMA_STATUS_IE_NR		(0x00200000) // buffer not ready int ena
#define NETARM_DMA_STATUS_IE_CA		(0x00100000) // channel abort int enable
#define NETARM_DMA_STATUS_WRAP		(0x00080000) // last descriptor
#define NETARM_DMA_STATUS_IDONE		(0x00040000) // interrupt on done
#define NETARM_DMA_STATUS_LAST		(0x00020000) // last buffer descriptor
#define NETARM_DMA_STATUS_FULL		(0x00010000) // buffer full indicator
#define NETARM_DMA_STATUS_RES2		(0x00008000) // Reserved
#define NETARM_DMA_STATUS_BLEN		(0x00007FFF) // remaining buffer count

#ifndef	__ASSEMBLER__

// the supported DMA channels
// they represent the offsets to the Buffer Descriptor Pointer on
// the DMA base register
typedef enum 
{
    NETARM_DMA_CHANNEL_1A = 0x0000,
    NETARM_DMA_CHANNEL_1B = 0x0020,
    NETARM_DMA_CHANNEL_1C = 0x0040,
    NETARM_DMA_CHANNEL_1D = 0x0060,
    NETARM_DMA_CHANNEL_2  = 0x0080,
    NETARM_DMA_CHANNEL_3  = 0x00A0,
    NETARM_DMA_CHANNEL_4  = 0x00C0,
    NETARM_DMA_CHANNEL_5  = 0x00E0,
    NETARM_DMA_CHANNEL_6  = 0x0100,
    NETARM_DMA_CHANNEL_7  = 0x0120,
    NETARM_DMA_CHANNEL_8  = 0x0140,
    NETARM_DMA_CHANNEL_9  = 0x0160,
    NETARM_DMA_CHANNEL_10 = 0x0180,
    NETARM_DMA_CHANNEL_11 = 0x01A0,
    NETARM_DMA_CHANNEL_12 = 0x01C0,
    NETARM_DMA_CHANNEL_13 = 0x01E0
} netarm_dmaChannel_t;


/*
 * New Forth-Systeme-designed DMA Flyby buffer descriptor
 */
typedef struct
{  
	union
	{
		unsigned int unReg;
		struct {
			unsigned srcPtr   : 29; /* d0-d28 actual buffer pointer */
			unsigned last     : 1;  /* d29 1 if it is the last buffer */
			unsigned intr     : 1;  /* d30 interrupt if buffer closed */
			unsigned wrap     : 1;  /* d31 1 if next buffer is first */
		} bits;
	} l;
	union 
	{
		unsigned int unReg;
		struct 
		{
			unsigned bufLen : 15; /* d0-d14 size of buffer @ srcPtr */
			unsigned full   : 1;  /* d15 1 if buffer full */
			unsigned status : 16; /* d16-d31 depend on peripheral */
		} bits;
	} h;
} netarm_dmaBufferDescFb_t;

/*
 * New Forth-Systeme-designed DMA Memory-to-memory buffer descriptor
 */
typedef struct
{
	netarm_dmaBufferDescFb_t common; /* common fields for fly-by and */
					 /* mem-to-mem */
	unsigned int unDestBPtr;  /* destination buffer pointer */
	unsigned int unReserved;  /* reserved */
} netarm_dmaBufferDescMem2Mem_t;

/* all functions operate on netarm_dmaBufferFb_t if not overwise noted */
#define DMA_CHANNEL_CONTROL	0x10 /* offset to control register */
#define DMA_CHANNEL_STATUS	0x14 /* offset to status register */

/***********************************************************************
 * @Function: netarm_dmaChannelDisable
 * @Return: nothing
 * @Descr: disables DMA channel dmaChannel and disables interrupts.
 *         Clears any pending interrupts
 ***********************************************************************/

static inline void netarm_dmaChannelDisable( netarm_dmaChannel_t dmaChannel )
{
	*get_dma_reg_addr( dmaChannel, NETARM_DMA_CONTROL ) &=
	    ~(NETARM_DMA_CONTROL_CE);
	/* acknowledge pending interrupts */
	*get_dma_reg_addr( dmaChannel, NETARM_DMA_STATUS ) = 
	    NETARM_DMA_STATUS_IP_MA;
}


/***********************************************************************
 * @Function: netarm_dmaChannelEnable
 * @Return: nothing
 * @Descr: initializes the channel for the buffer descriptor and
 *         enables interrupts and channels
 ***********************************************************************/

static inline void netarm_dmaChannelEnable( netarm_dmaChannel_t dmaChannel,
			      netarm_dmaBufferDescFb_t* pFirstBufferDesc,
			      unsigned int unControlOptions,
			      unsigned int unInterruptsEnable )
{
	*get_dma_reg_addr( dmaChannel, NETARM_DMA_STATUS ) = 
	    unInterruptsEnable;

	*get_dma_reg_addr( dmaChannel, NETARM_DMA_DESC ) =
	    (unsigned int) virt_to_bus( pFirstBufferDesc );

	*get_dma_reg_addr( dmaChannel, NETARM_DMA_CONTROL ) =
	    NETARM_DMA_CONTROL_CE | unControlOptions;
	
}


/***********************************************************************
 * @Function: netarm_dmaBufferDescInit
 * @Return: nothing
 * @Descr: sets up an array of descriptors sized nDescListSize starting from
 *         pBufferDesc and may enable interrupt.
 *         Currently only suitable for fly-by buffers
 ***********************************************************************/

static inline void netarm_dmaBufferDescInit( netarm_dmaBufferDescFb_t* pBufferDesc,
			       unsigned int nDescListSize,
			       unsigned char ucIntEnable )
{
	int nDescIndex;

	for( nDescIndex = 1; nDescIndex <= nDescListSize; nDescIndex++ )
	{
	    pBufferDesc->l.bits.intr = ucIntEnable;
	    pBufferDesc->l.bits.srcPtr = 0;
	    pBufferDesc->h.bits.status = 0;
 	    pBufferDesc->h.bits.full = 0;
	    pBufferDesc->h.bits.bufLen = 0;
	    pBufferDesc->l.bits.last = 0;

	    /* prepare the buffer descriptors so the controller knows whether to
	     * stop or continue to next one
       */
	    if( nDescIndex != nDescListSize )
	    {
		pBufferDesc->l.bits.wrap = 0;
	    } else {
		pBufferDesc->l.bits.wrap = 1;
	    }

	    pBufferDesc++;
	}
}
#endif /* !__ASSEMBLER__ */

#endif /* OLD_STYLE_NETARM_DMA_CONFIGURATION */

#endif


