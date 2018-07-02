/*
 ***************************************************************************
 * Ralink Tech Inc.
 * Ralink Tech Inc.
 * 5F., No.36, Taiyuan St., Jhubei City,
 * Hsinchu County 302,
 * Taiwan, R.O.C.
 *
 * (c) Copyright, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************
 */

#ifndef __RALINK_DMA_CTRL_H__
#define __RALINK_DMA_CTRL_H__

#include <asm/rt2880/rt_mmap.h>

/*
 * DEFINITIONS AND MACROS
 */
#define MAX_GDMA_CHANNEL		8
#define MOD_VERSION 			"0.3"
#define RALINK_GDMA_CTRL_BASE		(RALINK_GDMA_BASE)
#define RALINK_GDMAISTS			(RALINK_GDMA_BASE + 0x80)
#define RALINK_GDMASSTS			(RALINK_GDMA_BASE + 0x84)
#define RALINK_GDMAGCT			(RALINK_GDMA_BASE + 0x88)

#define GDMA_READ_REG(addr) 		le32_to_cpu(*(volatile u32 *)(addr))
#define GDMA_WRITE_REG(addr, val)  	*((volatile uint32_t *)(addr)) = cpu_to_le32(val)

#define RALINK_IRQ_ADDR                 RALINK_INTCL_BASE
#define RALINK_REG_INTENA               (RALINK_IRQ_ADDR + 0x34)
#define RALINK_REG_INTDIS               (RALINK_IRQ_ADDR + 0x38)

/* 
 * 12bytes=GDMA Channel n Source Address(4) +
 *         GDMA Channel n Destination Address(4) +
 *         GDMA Channel n Control Register(4)
 *
 */
#define GDMA_SRC_REG(ch)		(RALINK_GDMA_BASE + ch*16)
#define GDMA_DST_REG(ch)		(GDMA_SRC_REG(ch) + 4)
#define GDMA_CTRL_REG(ch)		(GDMA_DST_REG(ch) + 4)
#define GDMA_CTRL_REG1(ch)		(GDMA_CTRL_REG(ch) + 4)

//GDMA Interrupt Status Register
#define TX_DONE_INT_STATUS_OFFSET	0
#define UMASK_INT_STATUS_OFFSET		16

//Control Reg1
#define CH_UNMASK_INTEBL_OFFSET		4
#define NEXT_UNMASK_CH_OFFSET		1
#define CH_MASK_OFFSET			0

//Control Reg
#define MODE_SEL_OFFSET			0
#define CH_EBL_OFFSET			1
#define INT_EBL_OFFSET			2
#define BRST_SIZE_OFFSET		3
#define DST_BRST_MODE_OFFSET		6
#define SRC_BRST_MODE_OFFSET		7
#define DST_DMA_REQ_OFFSET		8
#define SRC_DMA_REQ_OFFSET		12
#define TRANS_CNT_OFFSET		16

#define GDMA_PCM0_RX0			0
#define GDMA_PCM0_RX1			1
#define GDMA_PCM0_TX0			2
#define GDMA_PCM0_TX1			3

#define GDMA_PCM1_RX0			4
#define GDMA_PCM1_RX1			5
#define GDMA_PCM1_TX0			6
#define GDMA_PCM1_TX1			7

#define GDMA_I2S_TX0			4
#define GDMA_I2S_TX1			5

//#define GDMA_DEBUG
#ifdef GDMA_DEBUG
#define GDMA_PRINT(fmt, args...) printk(KERN_INFO "GDMA: " fmt, ## args)
#else
#define GDMA_PRINT(fmt, args...) { }
#endif

/*
 * TYPEDEFS AND STRUCTURES
 */

enum GdmaBusterMode {
	INC_MODE=0,
	FIX_MODE=1
};

enum GdmaBusterSize {
	BUSTER_SIZE_4B=0, 	/* 1 transfer */
	BUSTER_SIZE_8B=1, 	/* 2 transfer */
	BUSTER_SIZE_16B=2,  	/* 4 transfer */
	BUSTER_SIZE_32B=3,  	/* 8 transfer */
	BUSTER_SIZE_64B=4  	/* 16 transfer */
};

enum GdmaDmaReqNum {
	DMA_REQ0=0,
	DMA_NAND_FLASH_REQ=1,
	DMA_I2S_REQ=2,
	DMA_PCM_RX0_REQ=3,
	DMA_PCM_RX1_REQ=4,
	DMA_PCM_TX0_REQ=5,
	DMA_PCM_TX1_REQ=6,
	DMA_REQ7=7,
	DMA_MEM_REQ=8
};



typedef struct {
	uint32_t Src;
	uint32_t Dst;
	uint16_t TransCount;
	uint8_t  SoftMode;
	uint8_t  ChUnMaskIntEbl;
	uint8_t  NextUnMaskCh;
	uint8_t  ChMask;
	uint32_t  ChNum;
	enum GdmaDmaReqNum SrcReqNum;
	enum GdmaDmaReqNum DstReqNum;
	enum GdmaBusterMode SrcBurstMode;
	enum GdmaBusterMode DstBurstMode;
	enum GdmaBusterSize BurstSize;
	void (*TxDoneCallback)(uint32_t);
	void (*UnMaskIntCallback)(uint32_t);
} GdmaReqEntry;

/*
 * EXPORT FUNCTION
 */
int GdmaI2sTx(uint32_t Src, uint32_t Dst, uint8_t TxNo, uint16_t TransCount,
		void (*TxDoneCallback)(uint32_t data), 
		void (*UnMaskIntCallback)(uint32_t data));

int GdmaPcmRx(uint32_t Src, uint32_t Dst, uint8_t PcmNo, uint8_t RxNo, uint16_t TransCount,
		void (*TxDoneCallback)(uint32_t data), 
		void (*UnMaskIntCallback)(uint32_t data));

int GdmaPcmTx(uint32_t Src, uint32_t Dst, uint8_t PcmNo, uint8_t TxNo, uint16_t TransCount,
		void (*TxDoneCallback)(uint32_t data), 
		void (*UnMaskIntCallback)(uint32_t data));

int GdmaMem2Mem(uint32_t Src, uint32_t Dst, uint16_t TransCount, 
		void (*TxDoneCallback)(uint32_t data)); 

int GdmaMaskChannel(uint32_t ChNum);

int GdmaUnMaskChannel(uint32_t ChNum);

int GdmaReqQuickIns(uint32_t ChNum);


#endif
