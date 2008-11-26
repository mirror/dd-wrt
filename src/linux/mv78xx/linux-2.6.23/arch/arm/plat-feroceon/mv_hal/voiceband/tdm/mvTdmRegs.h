/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File under the following licensing terms. 
Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	    this list of conditions and the following disclaimer. 

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution. 

    *   Neither the name of Marvell nor the names of its contributors may be 
        used to endorse or promote products derived from this software without 
        specific prior written permission. 
    
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

#ifndef __INCmvTdmRegsh
#define __INCmvTdmRegsh

/************************************************/
/*        TDM to Mbus Bridge Register Map       */
/************************************************/
#define TDM_SPI_MUX_REG		(TDM_REG_BASE + 0x4000)
#define TDM_MBUS_CONFIG_REG	(TDM_REG_BASE + 0x4010)
#define TDM_MISC_REG		(TDM_REG_BASE + 0x4070)
#define TDM_CLK_DIV_REG		(TDM_REG_BASE + 0x4074)

/*****************************************/
/*        TDM Control Register Map       */
/*****************************************/
#define PCM_CTRL_REG		 TDM_REG_BASE
#define TIMESLOT_CTRL_REG	(TDM_REG_BASE + 0x4)
#define FRAME_TIMESLOT_REG	(TDM_REG_BASE + 0x38)
#define PCM_CLK_RATE_DIV_REG	(TDM_REG_BASE + 0x3c)
#define INT_EVENT_MASK_REG	(TDM_REG_BASE + 0x40)
//#define INT_EVENT_REG		0xb0044
#define INT_STATUS_MASK_REG	(TDM_REG_BASE + 0x48)
#define INT_RESET_SELECT_REG	(TDM_REG_BASE + 0x4c)
#define INT_STATUS_REG		(TDM_REG_BASE + 0x50)
#define DUMMY_RX_WRITE_DATA_REG (TDM_REG_BASE + 0x54)
#define MISC_CTRL_REG		(TDM_REG_BASE + 0x58)
#define TESTBUS_MUX_SELECT_REG	(TDM_REG_BASE + 0x5c)
#define CUR_TIMESLOT_REG	(TDM_REG_BASE + 0x70)
#define TDM_REV_REG		(TDM_REG_BASE + 0x74)
#define DMA_ABORT_ADDR_REG	(TDM_REG_BASE + 0x80)
#define DMA_ABORT_INFO_REG	(TDM_REG_BASE + 0x84)

#define CH_DELAY_CTRL_REG(ch)	((TDM_REG_BASE + 0x8) | ((ch)<<2))
#define CH_SAMPLE_REG(ch)	((TDM_REG_BASE + 0x30) | ((ch)<<2))
#define CH_DBG_REG(ch)		((TDM_REG_BASE + 0x78) | ((ch)<<2))
#define CH_TX_CUR_ADDR_REG(ch)	((TDM_REG_BASE + 0x60) | ((ch)<<3))
#define CH_RX_CUR_ADDR_REG(ch)	((TDM_REG_BASE + 0x64) | ((ch)<<3))
#define CH_ENABLE_REG(ch)	((TDM_REG_BASE) | (((ch)+1)<<4))
#define CH_BUFF_OWN_REG(ch)	((TDM_REG_BASE + 0x4) | (((ch)+1)<<4))
#define CH_TX_ADDR_REG(ch)	((TDM_REG_BASE + 0x8) | (((ch)+1)<<4))
#define CH_RX_ADDR_REG(ch)	((TDM_REG_BASE + 0xc) | (((ch)+1)<<4))

/* PCM_CLK_RATE_DIV_REG bits */
#define PCM_256KHZ	1
#define PCM_512KHZ	(1<<1)
#define PCM_1024KHZ	(1<<2)
#define PCM_2048KHZ	(1<<3)
#define PCM_4096KHZ	(1<<4)
#define PCM_8192KHZ	(1<<5)

/* FRAME_TIMESLOT_REG bits */
#define TIMESLOTS4_256KHZ	(1<<2)
#define TIMESLOTS8_512KHZ	(1<<3)
#define TIMESLOTS16_1024KHZ	(1<<4)
#define TIMESLOTS32_2048KHZ	(1<<5)
#define TIMESLOTS64_4096KHZ	(1<<6)
#define TIMESLOTS128_8192KHZ	(1<<7)

/* PCM_CTRL_REG bits */
#define MASTER_PCLK_OFFS	0
#define MASTER_PCLK_TDM		(0<<MASTER_PCLK_OFFS)
#define MASTER_PCLK_EXTERNAL	(1<<MASTER_PCLK_OFFS)
#define MASTER_FS_OFFS		1
#define MASTER_FS_TDM		(0<<MASTER_FS_OFFS)
#define MASTER_FS_EXTERNAL	(1<<MASTER_FS_OFFS)
#define DATA_POLAR_OFFS		2
#define DATA_POLAR_NEG		(0<<DATA_POLAR_OFFS)
#define DATA_POLAR_POS		(1<<DATA_POLAR_OFFS)
#define FS_POLAR_OFFS		3
#define FS_POLAR_NEG		(0<<FS_POLAR_OFFS)
#define FS_POLAR_POS		(1<<FS_POLAR_OFFS)
#define INVERT_FS_OFFS		4
#define INVERT_FS_HI		(0<<INVERT_FS_OFFS)
#define INVERT_FS_LO		(1<<INVERT_FS_OFFS)
#define FS_TYPE_OFFS		5
#define FS_TYPE_SHORT		(0<<FS_TYPE_OFFS)
#define FS_TYPE_LONG		(1<<FS_TYPE_OFFS)
#define PCM_SAMPLE_SIZE_OFFS	6
#define PCM_SAMPLE_SIZE_1	(0<<PCM_SAMPLE_SIZE_OFFS)
#define PCM_SAMPLE_SIZE_2	(1<<PCM_SAMPLE_SIZE_OFFS)
#define CH_DELAY_OFFS		8
#define CH_DELAY_DISABLE	(0<<CH_DELAY_OFFS)
#define CH_DELAY_ENABLE		(3<<CH_DELAY_OFFS)
#define CH_QUALITY_OFFS		10
#define CH_QUALITY_DISABLE	(0<<CH_QUALITY_OFFS)
#define CH_QUALITY_ENABLE	(3<<CH_QUALITY_OFFS)
#define QUALITY_POLARITY_OFFS	12
#define QUALITY_POLARITY_NEG	(0<<QUALITY_POLARITY_OFFS)
#define QUALITY_POLARITY_POS	(1<<QUALITY_POLARITY_OFFS)
#define QUALITY_TYPE_OFFS	13
#define QUALITY_TYPE_TIME_SLOT	(0<<QUALITY_TYPE_OFFS)
#define QUALITY_TYPE_MSB	(3<<QUALITY_TYPE_OFFS)
#define CS_CTRL_OFFS		15
#define CS_CTRL_DONT_CARE	(0<<CS_CTRL_OFFS)
#define CS_CTRL			(1<<CS_CTRL_OFFS)
#define CS_CTRL_0		(0<<CS_CTRL_OFFS)
#define CS_CTRL_1		(1<<CS_CTRL_OFFS)
#define WIDEBAND_OFFS		16
#define WIDEBAND_OFF		(0<<WIDEBAND_OFFS)
#define WIDEBAND_ON		(1<<WIDEBAND_OFFS)
#define PERF_GBUS_OFFS		31
#define PERF_GBUS_ONE_ACCESS	(0<<PERF_GBUS_OFFS)
#define PERF_GBUS_TWO_ACCESS	(1<<PERF_GBUS_OFFS)

/* TIMESLOT_CTRL_REG bits */
#define CH0_RX_SLOT_OFFS	0
#define CH0_TX_SLOT_OFFS	8
#define CH1_RX_SLOT_OFFS	16
#define CH1_TX_SLOT_OFFS	24

/* CH_SAMPLE_REG bits */
#define TOTAL_CNT_OFFS		0
#define TOTAL_CNT_MASK		(0xff<<TOTAL_CNT_OFFS)
#define INT_CNT_OFFS		8
#define INT_CNT_MASK		(0xff<<INT_CNT_OFFS)

/* CH_BUFF_OWN_REG bits */
#define RX_OWN_BYTE_OFFS	0
#define TX_OWN_BYTE_OFFS	1
#define OWNER_MASK		1
#define OWN_BY_SW		0
#define OWN_BY_HW		1

/* CH_ENABLE_REG bits */
#define RX_ENABLE_BYTE_OFFS	0
#define TX_ENABLE_BYTE_OFFS	1
#define CH_ENABLE		1
#define CH_DISABLE		0

/* INT_STATUS_REG bits */
#define RX_OVERFLOW_BIT(ch)	(1<<(0+(ch)*2))
#define TX_UNDERFLOW_BIT(ch)	(1<<(1+((ch)*2)))
#define RX_BIT(ch)		(1<<(4+((ch)*2)))
#define TX_BIT(ch)		(1<<(5+((ch)*2)))
#define RX_IDLE_BIT(ch)		(1<<(8+((ch)*2)))
#define TX_IDLE_BIT(ch)		(1<<(9+((ch)*2)))
#define RX_FIFO_FULL(ch)	(1<<(12+((ch)*2)))
#define TX_FIFO_EMPTY(ch)	(1<<(13+((ch)*2)))
#define DMA_ABORT_BIT		(1<<16)
#define SLIC_INT_BIT		(1<<17)

/* TDU_INTR_SET_RESET bits */
#define CLEAR_MODE_OFFS		0
#define CLEAR_ON_READ		(1<<CLEAR_MODE_OFFS)
#define CLEAR_ON_ZERO		(0<<CLEAR_MODE_OFFS)

/* CH_DELAY_CTRL_REG bits */
#define CH_RX_DELAY_OFFS	0
#define CH_RX_DELAY_MASK	(0x3ff<<CH_RX_DELAY_OFFS)
#define CH_TX_DELAY_OFFS	16
#define CH_TX_DELAY_MASK	(0x3ff<<CH_RX_DELAY_OFFS)


/*********************************/
/*        SPI Register Map       */
/*********************************/
#define SPI_CLK_PRESCALAR_REG	(TDM_REG_BASE +0x3100)
#define SPI_GLOBAL_CTRL_REG	(TDM_REG_BASE +0x3104)
#define SPI_CTRL_REG		(TDM_REG_BASE +0x3108)
#define SPI_MICRO_WIRE_CTRL_REG	(TDM_REG_BASE +0x310c)
#define SPI_TWSI_CTRL_REG	(TDM_REG_BASE +0x3110)
#define SPI_MV_STATUS_REG	(TDM_REG_BASE +0x311c)
#define SPI_INT_MASK_REG	(TDM_REG_BASE +0x3120)
#define SPI_DATA_REG		(TDM_REG_BASE +0x3124)
#define SPI_ADDR_REG		(TDM_REG_BASE +0x3128)
#define SPI_CODEC_CMD_LO_REG	(TDM_REG_BASE +0x3130)
#define SPI_CODEC_CMD_HI_REG	(TDM_REG_BASE +0x3134)
#define SPI_CODEC_CTRL_REG	(TDM_REG_BASE +0x3138)
#define SPI_CODEC_READ_DATA_REG	(TDM_REG_BASE +0x313c)

/* SPI_CTRL_REG bits */
#define SPI_STAT_OFFS	10
#define SPI_STAT_MASK	(1<<SPI_STAT_OFFS)
#define SPI_READY	(0<<SPI_STAT_OFFS)
#define SPI_ACTIVE	(1<<SPI_STAT_OFFS)

/* SPI_GLOBAL_CTRL_REG bits */
#define SPI_GLOBAL_ENABLE_OFFS	0
#define SPI_GLOBAL_ENABLE_MASK	(1<<SPI_GLOBAL_ENABLE_OFFS)
#define SPI_GLOBAL_DISABLE	(0<<SPI_GLOBAL_ENABLE_OFFS)
#define SPI_GLOBAL_ENABLE	(1<<SPI_GLOBAL_ENABLE_OFFS)

/* SPI_CODEC_CTRL_REG bits */
#define TRANSFER_BYTES_OFFS	0
#define TRANSFER_BYTES(count)	((count-1)<<TRANSFER_BYTES_OFFS)
#define ENDIANESS_MODE_OFFS	2
#define ENDIANESS_MSB_MODE	(0<<ENDIANESS_MODE_OFFS)
#define ENDIANESS_LSB_MODE	(1<<ENDIANESS_MODE_OFFS)
#define RD_WR_MODE_OFFS		3
#define WR_MODE			(0<<RD_WR_MODE_OFFS)
#define RD_MODE			(1<<RD_WR_MODE_OFFS)
#define READ_BYTES_OFFS		4
#define READ_1_BYTE		(0<<READ_BYTES_OFFS)
#define READ_2_BYTE		(1<<READ_BYTES_OFFS)
#define CLK_SPEED_OFFS		5
#define CLK_SPEED_LO_DIV	(0<<CLK_SPEED_OFFS)
#define CLK_SPEED_HI_DIV	(1<<CLK_SPEED_OFFS)
#define CS_HI_CNT_VAL_RD_OFFS	6
#define CS_HI_CNT_VAL_RD_MASK	(0xff3<<CS_HI_CNT_VAL_RD_OFFS)



#endif /*__INCmvTdmRegsh*/

