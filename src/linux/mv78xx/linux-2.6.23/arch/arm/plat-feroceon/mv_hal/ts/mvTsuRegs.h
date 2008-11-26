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
#ifndef __INC_MV_TSU_REGS_H__
#define __INC_MV_TSU_REGS_H__

#define TSU_MAX_DECODE_WIN		4

#define TSU_GLOBAL_REG_BASE		0xB4000
#define TSU_REG_BASE(port)		(0xB8000 + (port * 0x800))

#define MV_TSU_MODES_REG		(TSU_GLOBAL_REG_BASE + 0x00)


#define MV_TSU_CONFIG_REG(port)			(TSU_REG_BASE(port) + 0x00)
#define MV_TSU_DMA_PARAMS_REG(port)		(TSU_REG_BASE(port) + 0x04)
#define MV_TSU_DONE_QUEUE_BASE_REG(port)	(TSU_REG_BASE(port) + 0x08)
#define MV_TSU_DESC_QUEUE_BASE_REG(port)	(TSU_REG_BASE(port) + 0x0C)
#define MV_TSU_DONE_QUEUE_WRITE_PTR_REG(port)	(TSU_REG_BASE(port) + 0x10)
#define MV_TSU_DONE_QUEUE_READ_PTR_REG(port)	(TSU_REG_BASE(port) + 0x14)
#define MV_TSU_DESC_QUEUE_WRITE_PTR_REG(port)	(TSU_REG_BASE(port) + 0x18)
#define MV_TSU_DESC_QUEUE_READ_PTR_REG(port)	(TSU_REG_BASE(port) + 0x1C)
#define MV_TSU_ENABLE_ACCESS_REG(port)		(TSU_REG_BASE(port) + 0x2C)
#define MV_TSU_TIMESTAMP_REG(port)		(TSU_REG_BASE(port) + 0x30)
#define MV_TSU_STATUS_REG(port)			(TSU_REG_BASE(port) + 0x34)
#define MV_TSU_TIMESTAMP_CTRL_REG(port)		(TSU_REG_BASE(port) + 0x38)
#define MV_TSU_TEST_REG(port)			(TSU_REG_BASE(port) + 0x3C)
#define MV_TSU_INTERRUPT_SRC_REG(port)		(TSU_REG_BASE(port) + 0x40)
#define MV_TSU_INTERRUPT_MASK_REG(port)		(TSU_REG_BASE(port) + 0x44)
#define MV_TSU_IRQ_PARAM_REG(port)		(TSU_REG_BASE(port) + 0x48)
#define MV_TSU_DEBUG_REG(port)			(TSU_REG_BASE(port) + 0x4C)
#define MV_TSU_NEXT_DESC_1_REG(port)		(TSU_REG_BASE(port) + 0x50)
#define MV_TSU_NEXT_DESC_2_REG(port)		(TSU_REG_BASE(port) + 0x54)
#define MV_TSU_SYNCBYTE_DETECT_REG(port)	(TSU_REG_BASE(port) + 0x58)
#define MV_TSU_AGGREGATION_CTRL_REG(port)	(TSU_REG_BASE(port) + 0x60)
#define MV_TSU_TIMESTAMP_INTERVAL_REG(port)	(TSU_REG_BASE(port) + 0x64)
#define MV_TSU_CONFIG_2_REG(port)		(TSU_REG_BASE(port) + 0x68)


/* TSU Modes register.				*/
#define TSU_MODES_PAR_MODE_OFFS		14
#define TSU_MODES_PAR_MODE_MASK		(0x1 << TSU_MODES_PAR_MODE_OFFS)
#define TSU_MODES_PAR_MODE_SER		(0x0 << TSU_MODES_PAR_MODE_OFFS)
#define TSU_MODES_PAR_MODE_PAR		(0x1 << TSU_MODES_PAR_MODE_OFFS)
#define TSU_MODES_TSCK_OFF		15
#define TSU_MODES_TSCK_MASK		(0x3 << TSU_MODES_TSCK_OFF)


/* TSU config register.				*/
#define TSU_CFG_RESET_OFFS		0
#define TSU_CFG_RESET_MASK		(0x3 << TSU_CFG_RESET_OFFS)
#define TSU_CFG_RESET_SET		(0x1 << TSU_CFG_RESET_OFFS)
#define TSU_CFG_RESET_CLEAR		(0x2 << TSU_CFG_RESET_OFFS)
#define TSU_CFG_OPER_OFFS		2
#define TSU_CFG_OPER_MASK		(0x3 << TSU_CFG_OPER_OFFS)
#define TSU_CFG_OPER_DISABLE		(0x1 << TSU_CFG_OPER_OFFS)
#define TSU_CFG_OPER_ENABLE		(0x2 << TSU_CFG_OPER_OFFS)
#define TSU_CFG_DATA_DIR_OFFS		8
#define TSU_CFG_DATA_DIR_MASK		(0x1 << TSU_CFG_DATA_DIR_OFFS)
#define TSU_CFG_DATA_DIR_IN		(0x0 << TSU_CFG_DATA_DIR_OFFS)
#define TSU_CFG_DATA_DIR_OUT		(0x1 << TSU_CFG_DATA_DIR_OFFS)
#define TSU_CFG_DATA_MODE_OFFS		9
#define TSU_CFG_DATA_MODE_MASK		(0x1 << TSU_CFG_DATA_MODE_OFFS)
#define TSU_CFG_DATA_MODE_SER		(0x0 << TSU_CFG_DATA_MODE_OFFS)
#define TSU_CFG_DATA_MODE_PAR		(0x1 << TSU_CFG_DATA_MODE_OFFS)
#define TSU_CFG_OUT_CLOCK_OFFS		10
#define TSU_CFG_OUT_CLOCK_MASK		(0x3 << TSU_CFG_OUT_CLOCK_OFFS)
#define TSU_CFG_OUT_CLOCK_4_32		(0x0 << TSU_CFG_OUT_CLOCK_OFFS)
#define TSU_CFG_OUT_CLOCK_2_16		(0x1 << TSU_CFG_OUT_CLOCK_OFFS)
#define TSU_CFG_OUT_CLOCK_1_8		(0x2 << TSU_CFG_OUT_CLOCK_OFFS)
#define TSU_CFG_OUT_CLOCK_EXT		(0x3 << TSU_CFG_OUT_CLOCK_OFFS)
#define TSU_CFG_CLK_MODE_OFFS		12
#define TSU_CFG_CLK_MODE_MASK		(0x1 << TSU_CFG_CLK_MODE_OFFS)
#define TSU_CFG_CLK_MODE_CONT		(0x0 << TSU_CFG_CLK_MODE_OFFS)
#define TSU_CFG_CLK_MODE_GAPPED		(0x1 << TSU_CFG_CLK_MODE_OFFS)
#define TSU_CFG_TS_SYNC_OFFS		13
#define TSU_CFG_TS_SYNC_MASK		(0x1 << TSU_CFG_TS_SYNC_OFFS)
#define TSU_CFG_TS_SYNC_8BIT		(0x0 << TSU_CFG_TS_SYNC_OFFS)
#define TSU_CFG_TS_SYNC_1BIT		(0x1 << TSU_CFG_TS_SYNC_OFFS)
#define TSU_CFG_DATA_ORD_OFFS		14
#define TSU_CFG_DATA_ORD_MASK		(0x1 << TSU_CFG_DATA_ORD_OFFS)
#define TSU_CFG_DATA_ORD_MSB		(0x0 << TSU_CFG_DATA_ORD_OFFS)
#define TSU_CFG_DATA_ORD_LSB		(0x1 << TSU_CFG_DATA_ORD_OFFS)
#define TSU_CFG_TX_EDGE_OFFS		15
#define TSU_CFG_TX_EDGE_MASK		(0x1 << TSU_CFG_TX_EDGE_OFFS)
#define TSU_CFG_FREQ_MODE_OFFS		16
#define TSU_CFG_FREQ_MODE_MASK		(0x1 << TSU_CFG_FREQ_MODE_OFFS)
#define TSU_CFG_ERR_POL_OFFS		18
#define TSU_CFG_ERR_POL_MASK		(0x1 << TSU_CFG_ERR_POL_OFFS)
#define TSU_CFG_ERR_USED_OFFS		19
#define TSU_CFG_ERR_USED_MASK		(0x1 << TSU_CFG_ERR_USED_OFFS)
#define TSU_CFG_VAL_POL_OFFS		20
#define TSU_CFG_VAL_POL_MASK		(0x1 << TSU_CFG_VAL_POL_OFFS)
#define TSU_CFG_VAL_USED_OFFS		21
#define TSU_CFG_VAL_USED_MASK		(0x1 << TSU_CFG_VAL_USED_OFFS)
#define TSU_CFG_SYNC_POL_OFFS		22
#define TSU_CFG_SYNC_POL_MASK		(0x1 << TSU_CFG_SYNC_POL_OFFS)
#define TSU_CFG_SYNC_USED_OFFS		23
#define TSU_CFG_SYNC_USED_MASK		(0x1 << TSU_CFG_SYNC_USED_OFFS)
#define TSU_CFG_RESET_SET		(0x1 << TSU_CFG_RESET_OFFS)
#define TSU_CFG_PKT_SIZE_OFFS		24
#define TSU_CFG_PKT_SIZE_MASK		(0xFF << TSU_CFG_PKT_SIZE_OFFS)

/* TSU DMA parameters register.			*/
#define TSU_DMAP_DMA_LEN_OFFS		0
#define TSU_DMAP_DMA_LEN_MASK		(0xFFFF << TSU_DMAP_DMA_LEN_OFFS)
#define TSU_DMAP_DATA_WTRMK_OFFS	16
#define TSU_DMAP_DATA_WTRMK_MASK	(0xFF << TSU_DMAP_DATA_WTRMK_OFFS)
#define TSU_DMAP_DATA_WTRMK_MAX		0xFF
#define TSU_DMAP_DESC_Q_SIZE_OFFS	24
#define TSU_DMAP_DESC_Q_SIZE_MASK	(0xF << TSU_DMAP_DESC_Q_SIZE_OFFS)
#define TSU_DMAP_DONE_Q_SIZE_OFFS	28
#define TSU_DMAP_DONE_Q_SIZE_MASK	(0xF << TSU_DMAP_DONE_Q_SIZE_OFFS)

/* TSU Done queue base register.		*/
#define TSU_DONE_PTR_BASE_OFFS		2
#define TSU_DONE_PTR_BASE_MASK		(0x3FFFFFFF << TSU_DONE_PTR_BASE_OFFS)

/* TSU Desc queue base register.		*/
#define TSU_DESC_PTR_BASE_OFFS		2
#define TSU_DESC_PTR_BASE_MASK		(0x3FFFFFFF << TSU_DESC_PTR_BASE_OFFS)

/* TSU Done queue write pointer register.	*/
#define TSU_DONE_WRITE_PTR_OFFS		0
#define TSU_DONE_WRITE_PTR_MASK		(0xFFF << TSU_DONE_WRITE_PTR_OFFS)

/* TSU Done queue read pointer register.	*/
#define TSU_DONE_READ_PTR_OFFS		0
#define TSU_DONE_READ_PTR_MASK		(0xFFF << TSU_DONE_READ_PTR_OFFS)

/* TSU Desc queue write pointer register.	*/
#define TSU_DESC_WRITE_PTR_OFFS		0
#define TSU_DESC_WRITE_PTR_MASK		(0xFFF << TSU_DESC_WRITE_PTR_OFFS)

/* TSU Desc queue read pointer register.	*/
#define TSU_DESC_READ_PTR_OFFS		0
#define TSU_DESC_READ_PTR_MASK		(0xFFF << TSU_DESC_READ_PTR_OFFS)

/* TSU access enable reg.			*/
#define TSU_ENACC_TS_READ_OFFS		0
#define TSU_ENACC_TS_WRITE_OFFS		8
#define TSU_ENACC_DESC_WRITE_OFFS	16
#define TSU_ENACC_DESC_READ_OFFS	24

/* TSU Timestamp register.			*/
#define TSU_TMSTMP_TIMESTAMP_OFFS	0
#define TSU_TMSTMP_TIMESTAMP_MASK	(0xFFFFFFF << TSU_TMSTMP_TIMESTAMP_OFFS)

/* TSU status register.				*/
#define TSU_STATUS_OFFS			0
#define TSU_STATUS_MASK			(0x7FF << TSU_STATUS_OFFS)
#define TSU_STATUS_IF_ERR		(0x100 << TSU_STATUS_OFFS)
#define TSU_STATUS_FIFO_OVFL_ERR	(0x200 << TSU_STATUS_OFFS)
#define TSU_STATUS_CONN_ERR		(0x400 << TSU_STATUS_OFFS)

/* TSU interrupt source register.		*/
#define TSU_INT_TS_IF_ERROR		(1 << 3)
#define TSU_INT_FIFO_OVFL_ERROR		(1 << 4)
#define TSU_INT_TS_CONN_ERROR		(1 << 5)
#define TSU_INT_CLOCK_SYNC_EXP		(1 << 6)

/* TSU SyncByte detect register.		*/
#define TSU_SYNC_DETECT_CNT_OFFS	0
#define TSU_SYNC_DETECT_CNT_MASK	(0xF << TSU_SYNC_DETECT_CNT_OFFS)
#define TSU_SYNC_LOSS_CNT_OFFS		4
#define TSU_SYNC_LOSS_CNT_MASK		(0xF << TSU_SYNC_LOSS_CNT_OFFS)

/* TSU Aggregation control register.		*/
#define TSU_AGGR_PCKT_NUM_OFFS		0
#define TSU_AGGR_PCKT_NUM_MASK		(0xFF << TSU_AGGR_PCKT_NUM_OFFS)
#define TSU_AGGR_TMSTMP_OFF_OFFS	8
#define TSU_AGGR_TMSTMP_OFF_MASK	(0xF << TSU_AGGR_TMSTMP_OFF_OFFS)
#define TSU_AGGR_FLUSH_ERR_OFFS		26
#define TSU_AGGR_FLUSH_ERR_MASK		(0x3 << TSU_AGGR_FLUSH_ERR_OFFS)
#define TSU_AGGR_FLUSH_ERR_DISABLE	(0x1 << TSU_AGGR_FLUSH_ERR_OFFS)
#define TSU_AGGR_FLUSH_ERR_ENABLE	(0x2 << TSU_AGGR_FLUSH_ERR_OFFS)
#define TSU_AGGR_TMSTMP_MODE_OFFS	28
#define TSU_AGGR_TMSTMP_MODE_MASK	(0x3 << TSU_AGGR_TMSTMP_MODE_OFFS)
#define TSU_AGGR_TMSTMP_TO_DONE_Q	(0x1 << TSU_AGGR_TMSTMP_MODE_OFFS)
#define TSU_AGGR_TMSTMP_TO_PCKT		(0x2 << TSU_AGGR_TMSTMP_MODE_OFFS)
#define TSU_AGGR_ENABLE_OFFS		30
#define TSU_AGGR_ENABLE_MASK		(0x3 << TSU_AGGR_ENABLE_OFFS)
#define TSU_AGGR_DISABLE		(0x1 << TSU_AGGR_ENABLE_OFFS)
#define TSU_AGGR_ENABLE			(0x2 << TSU_AGGR_ENABLE_OFFS)


/* TSU timestamp interval register.		*/
#define TSU_TMSTP_INTRVL_OFFS		0
#define TSU_TMSTP_INTRVL_MASK		(0xFFFFFFF << TSU_TMSTP_INTRVL_OFFS)

/* TSU timestamp control register.		*/
#define TSU_TMS_CTRL_TIMER_OFFS		0
#define TSU_TMS_CTRL_TIMER_MASK		(0x3 << TSU_TMS_CTRL_TIMER_OFFS)
#define TSU_TMS_CTRL_TIMER_DIS		(0x1 << TSU_TMS_CTRL_TIMER_OFFS)
#define TSU_TMS_CTRL_TIMER_EN		(0x2 << TSU_TMS_CTRL_TIMER_OFFS)
#define TSU_TMS_CTRL_AUTO_ADJ_OFFS	2
#define TSU_TMS_CTRL_AUTO_ADJ_MASK	(0x3 << TSU_TMS_CTRL_AUTO_ADJ_OFFS)
#define TSU_TMS_CTRL_AUTO_ADJ_OFF	(0x1 << TSU_TMS_CTRL_AUTO_ADJ_OFFS)
#define TSU_TMS_CTRL_AUTO_ADJ_ON	(0x2 << TSU_TMS_CTRL_AUTO_ADJ_OFFS)
#define TSU_TMS_CTRL_READ_TIMER_OFFS	4
#define TSU_TMS_CTRL_READ_TIMER_MASK	(0x1 << TSU_TMS_CTRL_READ_TIMER_OFFS)




#endif /* __INC_MV_TSU_REGS_H__ */

