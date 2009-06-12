/*
 * dma.h 
 *
 * Copyright (C) 2002-2003 Arcturus Networks Inc.
 *                         by Oleksandr Zhadan <www.ArcturusNetworks.com>
 *
 * This file includes the DMA controller definitions 
 * of the S3C2500X RISC microcontroller
 * based on the Samsung's "S3C2500X 32-bit RISC
 * microcontroller pre. User's Manual"
 */

#ifndef __ASM_ARCH_DMA_H
#define __ASM_ARCH_DMA_H

#define MAX_DMA_ADDRESS		0x01000000 /* End of the SDRAM */
#define MAX_DMA_CHANNELS        6

#define GDMA_BASE	0xf0050000

#define DPRIC		0xf0051000	/* GDMA priority configuration R */
#define DPRIF		0xf0052000	/* GDMA programmable priority R for fixed */
#define DPRIR		0xf0053000	/* GDMA programmable priority R for round-robin */

#define DCON0		0xf0050000	/* GDMA channel 0 control R		*/
#define DSAR0		0xf0050004	/* GDMA channel 0 source address R	*/
#define DDAR0		0xf0050008	/* GDMA channel 0 destination address R */
#define DTCR0		0xf005000c	/* GDMA channel 0 transfer count R	*/
#define DRER0		0xf0050010	/* GDMA channel 0 run enable R		*/
#define DIPR0		0xf0050014	/* GDMA channel 0 interrupt pending R	*/

#define DCON1		0xf0050020	/* GDMA channel 1 control R		*/
#define DSAR1		0xf0050024	/* GDMA channel 1 source address R	*/
#define DDAR1		0xf0050028	/* GDMA channel 1 destination address R */
#define DTCR1		0xf005002c	/* GDMA channel 1 transfer count R	*/
#define DRER1		0xf0050030	/* GDMA channel 1 run enable R		*/
#define DIPR1		0xf0050034	/* GDMA channel 1 interrupt pending R	*/

#define DCON2		0xf0050040	/* GDMA channel 2 control R		*/
#define DSAR2		0xf0050044	/* GDMA channel 2 source address R	*/
#define DDAR2		0xf0050048	/* GDMA channel 2 destination address R */
#define DTCR2		0xf005004c	/* GDMA channel 2 transfer count R	*/
#define DRER2		0xf0050050	/* GDMA channel 2 run enable R		*/
#define DIPR2		0xf0050054	/* GDMA channel 2 interrupt pending R	*/

#define DCON3		0xf0050060	/* GDMA channel 3 control R		*/
#define DSAR3		0xf0050064	/* GDMA channel 3 source address R	*/
#define DDAR3		0xf0050068	/* GDMA channel 3 destination address R */
#define DTCR3		0xf005006c	/* GDMA channel 3 transfer count R	*/
#define DRER3		0xf0050070	/* GDMA channel 3 run enable R		*/
#define DIPR3		0xf0050074	/* GDMA channel 3 interrupt pending R	*/

#define DCON4		0xf0050080	/* GDMA channel 4 control R		*/
#define DSAR4		0xf0050084	/* GDMA channel 4 source address R	*/
#define DDAR4		0xf0050088	/* GDMA channel 4 destination address R */
#define DTCR4		0xf005008c	/* GDMA channel 4 transfer count R	*/
#define DRER4		0xf0050090	/* GDMA channel 4 run enable R		*/
#define DIPR4		0xf0050094	/* GDMA channel 4 interrupt pending R	*/

#define DCON5		0xf00500a0	/* GDMA channel 5 control R		*/
#define DSAR5		0xf00500a4	/* GDMA channel 5 source address R	*/
#define DDAR5		0xf00500a8	/* GDMA channel 5 destination address R */
#define DTCR5		0xf00500ac	/* GDMA channel 5 transfer count R	*/
#define DRER5		0xf00500b0	/* GDMA channel 5 run enable R		*/
#define DIPR5		0xf00500b4	/* GDMA channel 5 interrupt pending R	*/

#define	DMA_RE		0x00000001
#define	DMA_EXTDREQ	0x00000002
#define	DMA_HUARTTX	0x00000004
#define	DMA_HUARTRX	0x00000006
#define	DMA_DESIN	0x00000008
#define	DMA_DESOUT	0x0000000a
#define	DMA_SB		0x00000010
#define	DMA_FB		0x00000020
#define	DMA_TS4		0x00000080
#define	DMA_TS2		0x00000040
#define	DMA_SD		0x00000100
#define	DMA_SF		0x00000200
#define	DMA_DD		0x00000400
#define	DMA_DF		0x00000800
#define	DMA_IE		0x00001000
#define	DMA_XCNT(x)	((x-1)<<13)
#define	DMA_BS		0x80000000

#endif /* _ASM_ARCH_DMA_H */
