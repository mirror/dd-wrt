/*
 * linux/include/asm-arm/iomd.h
 *
 * Copyright (C) 1999 Russell King
 *
 * This file contains information out the IOMD ASIC used in the
 * Acorn RiscPC and subsequently integrated into the CLPS7500 chips.
 */
#include <linux/config.h>

#ifndef __ASSEMBLY__
#define __IOMD(offset)	(IO_IOMD_BASE + (offset >> 2))
#else
#define __IOMD(offset)	offset
#endif

#define IOMD_CONTROL	__IOMD(0x000)
#define IOMD_KARTTX	__IOMD(0x004)
#define IOMD_KARTRX	__IOMD(0x004)
#define IOMD_KCTRL	__IOMD(0x008)

#ifdef CONFIG_ARCH_CLPS7500
#define IOMD_IOLINES	__IOMD(0x00C)
#endif

#define IOMD_IRQSTATA	__IOMD(0x010)
#define IOMD_IRQREQA	__IOMD(0x014)
#define IOMD_IRQCLRA	__IOMD(0x014)
#define IOMD_IRQMASKA	__IOMD(0x018)

#ifdef CONFIG_ARCH_CLPS7500
#define IOMD_SUSMODE	__IOMD(0x01C)
#endif

#define IOMD_IRQSTATB	__IOMD(0x020)
#define IOMD_IRQREQB	__IOMD(0x024)
#define IOMD_IRQMASKB	__IOMD(0x028)

#define IOMD_FIQSTAT	__IOMD(0x030)
#define IOMD_FIQREQ	__IOMD(0x034)
#define IOMD_FIQMASK	__IOMD(0x038)

#ifdef CONFIG_ARCH_CLPS7500
#define IOMD_CLKCTL	__IOMD(0x03C)
#endif

#define IOMD_T0CNTL	__IOMD(0x040)
#define IOMD_T0LTCHL	__IOMD(0x040)
#define IOMD_T0CNTH	__IOMD(0x044)
#define IOMD_T0LTCHH	__IOMD(0x044)
#define IOMD_T0GO	__IOMD(0x048)
#define IOMD_T0LATCH	__IOMD(0x04c)

#define IOMD_T1CNTL	__IOMD(0x050)
#define IOMD_T1LTCHL	__IOMD(0x050)
#define IOMD_T1CNTH	__IOMD(0x054)
#define IOMD_T1LTCHH	__IOMD(0x054)
#define IOMD_T1GO	__IOMD(0x058)
#define IOMD_T1LATCH	__IOMD(0x05c)

#ifdef CONFIG_ARCH_CLPS7500
#define IOMD_IRQSTATC	__IOMD(0x060)
#define IOMD_IRQREQC	__IOMD(0x064)
#define IOMD_IRQMASKC	__IOMD(0x068)

#define IOMD_VIDMUX	__IOMD(0x06c)

#define IOMD_IRQSTATD	__IOMD(0x070)
#define IOMD_IRQREQD	__IOMD(0x074)
#define IOMD_IRQMASKD	__IOMD(0x078)
#endif

#define IOMD_ROMCR0	__IOMD(0x080)
#define IOMD_ROMCR1	__IOMD(0x084)
#ifdef CONFIG_ARCH_RPC
#define IOMD_DRAMCR	__IOMD(0x088)
#endif
#define IOMD_REFCR	__IOMD(0x08C)

#define IOMD_FSIZE	__IOMD(0x090)
#define IOMD_ID0	__IOMD(0x094)
#define IOMD_ID1	__IOMD(0x098)
#define IOMD_VERSION	__IOMD(0x09C)

#ifdef CONFIG_ARCH_RPC
#define IOMD_MOUSEX	__IOMD(0x0A0)
#define IOMD_MOUSEY	__IOMD(0x0A4)
#endif

#ifdef CONFIG_ARCH_CLPS7500
#define IOMD_MSEDAT	__IOMD(0x0A8)
#define IOMD_MSECTL	__IOMD(0x0Ac)
#endif

#ifdef CONFIG_ARCH_RPC
#define IOMD_DMATCR	__IOMD(0x0C0)
#endif
#define IOMD_IOTCR	__IOMD(0x0C4)
#define IOMD_ECTCR	__IOMD(0x0C8)
#ifdef CONFIG_ARCH_RPC
#define IOMD_DMAEXT	__IOMD(0x0CC)
#endif
#ifdef CONFIG_ARCH_CLPS7500
#define IOMD_ASTCR	__IOMD(0x0CC)
#define IOMD_DRAMCR	__IOMD(0x0D0)
#define IOMD_SELFREF	__IOMD(0x0D4)
#define IOMD_ATODICR	__IOMD(0x0E0)
#define IOMD_ATODSR	__IOMD(0x0E4)
#define IOMD_ATODCC	__IOMD(0x0E8)
#define IOMD_ATODCNT1	__IOMD(0x0EC)
#define IOMD_ATODCNT2	__IOMD(0x0F0)
#define IOMD_ATODCNT3	__IOMD(0x0F4)
#define IOMD_ATODCNT4	__IOMD(0x0F8)
#endif

#ifdef CONFIG_ARCH_RPC
#define DMA_EXT_IO0	1
#define DMA_EXT_IO1	2
#define DMA_EXT_IO2	4
#define DMA_EXT_IO3	8

#define IOMD_IO0CURA	__IOMD(0x100)
#define IOMD_IO0ENDA	__IOMD(0x104)
#define IOMD_IO0CURB	__IOMD(0x108)
#define IOMD_IO0ENDB	__IOMD(0x10C)
#define IOMD_IO0CR	__IOMD(0x110)
#define IOMD_IO0ST	__IOMD(0x114)

#define IOMD_IO1CURA	__IOMD(0x120)
#define IOMD_IO1ENDA	__IOMD(0x124)
#define IOMD_IO1CURB	__IOMD(0x128)
#define IOMD_IO1ENDB	__IOMD(0x12C)
#define IOMD_IO1CR	__IOMD(0x130)
#define IOMD_IO1ST	__IOMD(0x134)

#define IOMD_IO2CURA	__IOMD(0x140)
#define IOMD_IO2ENDA	__IOMD(0x144)
#define IOMD_IO2CURB	__IOMD(0x148)
#define IOMD_IO2ENDB	__IOMD(0x14C)
#define IOMD_IO2CR	__IOMD(0x150)
#define IOMD_IO2ST	__IOMD(0x154)

#define IOMD_IO3CURA	__IOMD(0x160)
#define IOMD_IO3ENDA	__IOMD(0x164)
#define IOMD_IO3CURB	__IOMD(0x168)
#define IOMD_IO3ENDB	__IOMD(0x16C)
#define IOMD_IO3CR	__IOMD(0x170)
#define IOMD_IO3ST	__IOMD(0x174)
#endif

#define IOMD_SD0CURA	__IOMD(0x180)
#define IOMD_SD0ENDA	__IOMD(0x184)
#define IOMD_SD0CURB	__IOMD(0x188)
#define IOMD_SD0ENDB	__IOMD(0x18C)
#define IOMD_SD0CR	__IOMD(0x190)
#define IOMD_SD0ST	__IOMD(0x194)

#ifdef CONFIG_ARCH_RPC
#define IOMD_SD1CURA	__IOMD(0x1A0)
#define IOMD_SD1ENDA	__IOMD(0x1A4)
#define IOMD_SD1CURB	__IOMD(0x1A8)
#define IOMD_SD1ENDB	__IOMD(0x1AC)
#define IOMD_SD1CR	__IOMD(0x1B0)
#define IOMD_SD1ST	__IOMD(0x1B4)
#endif

#define IOMD_CURSCUR	__IOMD(0x1C0)
#define IOMD_CURSINIT	__IOMD(0x1C4)

#define IOMD_VIDCUR	__IOMD(0x1D0)
#define IOMD_VIDEND	__IOMD(0x1D4)
#define IOMD_VIDSTART	__IOMD(0x1D8)
#define IOMD_VIDINIT	__IOMD(0x1DC)
#define IOMD_VIDCR	__IOMD(0x1E0)

#define IOMD_DMASTAT	__IOMD(0x1F0)
#define IOMD_DMAREQ	__IOMD(0x1F4)
#define IOMD_DMAMASK	__IOMD(0x1F8)

#define DMA_END_S	(1 << 31)
#define DMA_END_L	(1 << 30)

#define DMA_CR_C	0x80
#define DMA_CR_D	0x40
#define DMA_CR_E	0x20

#define DMA_ST_OFL	4
#define DMA_ST_INT	2
#define DMA_ST_AB	1

#ifndef IOC_CONTROL
/*
 * IOC compatability
 */
#define IOC_CONTROL	IOMD_CONTROL
#define IOC_IRQSTATA	IOMD_IRQSTATA
#define IOC_IRQREQA	IOMD_IRQREQA
#define IOC_IRQCLRA	IOMD_IRQCLRA
#define IOC_IRQMASKA	IOMD_IRQMASKA

#define IOC_IRQSTATB	IOMD_IRQSTATB
#define IOC_IRQREQB	IOMD_IRQREQB
#define IOC_IRQMASKB	IOMD_IRQMASKB

#define IOC_FIQSTAT	IOMD_FIQSTAT
#define IOC_FIQREQ	IOMD_FIQREQ
#define IOC_FIQMASK	IOMD_FIQMASK

#define IOC_T0CNTL	IOMD_T0CNTL
#define IOC_T0LTCHL	IOMD_T0LTCHL
#define IOC_T0CNTH	IOMD_T0CNTH
#define IOC_T0LTCHH	IOMD_T0LTCHH
#define IOC_T0GO	IOMD_T0GO
#define IOC_T0LATCH	IOMD_T0LATCH

#define IOC_T1CNTL	IOMD_T1CNTL
#define IOC_T1LTCHL	IOMD_T1LTCHL
#define IOC_T1CNTH	IOMD_T1CNTH
#define IOC_T1LTCHH	IOMD_T1LTCHH
#define IOC_T1GO	IOMD_T1GO
#define IOC_T1LATCH	IOMD_T1LATCH
#endif

/*
 * DMA (MEMC) compatability
 */
#define HALF_SAM	vram_half_sam
#define VDMA_ALIGNMENT	(HALF_SAM * 2)
#define VDMA_XFERSIZE	(HALF_SAM)
#define VDMA_INIT	IOMD_VIDINIT
#define VDMA_START	IOMD_VIDSTART
#define VDMA_END	IOMD_VIDEND

#ifndef __ASSEMBLY__
extern unsigned int vram_half_sam;
#define video_set_dma(start,end,offset)				\
do {								\
	outl (SCREEN_START + start, VDMA_START);		\
	outl (SCREEN_START + end - VDMA_XFERSIZE, VDMA_END);	\
	if (offset >= end - VDMA_XFERSIZE)			\
		offset |= 0x40000000;				\
	outl (SCREEN_START + offset, VDMA_INIT);		\
} while (0)
#endif

