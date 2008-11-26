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

#ifndef __INCmvIdmaSpech
#define __INCmvIdmaSpech


/* defines */


/* IDMA channel rgisters       												*/
#define IDMA_BYTE_COUNT_REG(chan)          	(IDMA_UNIT_BASE + 0x0 + ((chan) * 4))
#define IDMA_SRC_ADDR_REG(chan)            	(IDMA_UNIT_BASE + 0x10 + ((chan) * 4))
#define IDMA_DST_ADDR_REG(chan)            	(IDMA_UNIT_BASE + 0x20 + ((chan) * 4))
#define IDMA_NEXT_DESC_PTR_REG(chan)       	(IDMA_UNIT_BASE + 0x30 + ((chan) * 4))
#define IDMA_CURR_DESC_PTR_REG(chan)	   	(IDMA_UNIT_BASE + 0x70 + ((chan) * 4))

/* IDMA Channel Control														*/
#define IDMA_CTRL_LOW_REG(chan)			(IDMA_UNIT_BASE + 0x40 + ((chan) * 4))
#define IDMA_CTRL_HIGH_REG(chan)		(IDMA_UNIT_BASE + 0x80 + ((chan) * 4))

 /*  IDMA Interrupt Register  */
#define IDMA_CAUSE_REG            		(IDMA_UNIT_BASE + 0xc0)
#define IDMA_MASK_REG           		(IDMA_UNIT_BASE + 0x8c4)
#define IDMA_ERROR_ADDR_REG                 	(IDMA_UNIT_BASE + 0x8c8)
#define IDMA_ERROR_SELECT_REG               	(IDMA_UNIT_BASE + 0x8cc)


/* DMA register fileds */


/* IDMA Channel Byte Count Register (ICBCR) */
#define ICBCR_BYTECNT_OFFS			0		/* Byte count field offset		*/
#define ICBCR_BYTECNT_MASK_64K  	0xFFFF	/* Maximum Byte count for 64K 	*/
#define ICBCR_BYTECNT_MASK_16M  	0xFFFFFF/* Maximum Byte count for 16M	*/
#define ICBCR_BYTECNT_LEFT_OFFS		30		/* Applicable for 16M mode		*/
#define ICBCR_BYTECNT_LEFT			(1 << ICBCR_BYTECNT_LEFT_OFFS)
#define ICBCR_DESC_OWNER_OFFS		31		/* Descriptor owned by DMA/CPU	*/
#define ICBCR_DESC_OWNER_MASK		(1 << ICBCR_DESC_OWNER_OFFS)
#define ICBCR_DESC_OWNER_BY_DMA		(0 << ICBCR_DESC_OWNER_OFFS)
#define ICBCR_DESC_OWNED_BY_CPU		(1 << ICBCR_DESC_OWNER_OFFS)

               
/* IDMA Channel Control Low Register (ICCLR)								*/
/* Data Transfer Limit Note: If an IDMA accesses a cache coherent DRAM 		*/
/* region, the burst limit must not exceed 32 bytes.						*/
#define B_8BYTE		0                
#define B_16BYTE	1
#define B_32BYTE	3           
#define B_64BYTE	7    
#define B_128BYTE	4

#define ICCLR_DST_BURST_LIM_OFFS	0
#define ICCLR_DST_BURST_LIM_MASK	(0x7 << ICCLR_DST_BURST_LIM_OFFS)
#define ICCLR_DST_BURST_LIM_8BYTE	(B_8BYTE   << ICCLR_DST_BURST_LIM_OFFS)                
#define ICCLR_DST_BURST_LIM_16BYTE	(B_16BYTE  << ICCLR_DST_BURST_LIM_OFFS)
#define ICCLR_DST_BURST_LIM_32BYTE	(B_32BYTE  << ICCLR_DST_BURST_LIM_OFFS)           
#define ICCLR_DST_BURST_LIM_64BYTE	(B_64BYTE  << ICCLR_DST_BURST_LIM_OFFS)    
#define ICCLR_DST_BURST_LIM_128BYTE	(B_128BYTE << ICCLR_DST_BURST_LIM_OFFS)
#define ICCLR_SRC_HOLD_OFFS			3   	/* Hold/increment source address*/
#define ICCLR_SRC_HOLD_MASK			(1 << ICCLR_SRC_HOLD_OFFS)
#define ICCLR_SRC_INC		   		(0 << ICCLR_SRC_HOLD_OFFS)
#define ICCLR_SRC_HOLD		   		(1 << ICCLR_SRC_HOLD_OFFS)
#define ICCLR_ACK_WIDTH_OFFS		4 	/* Asserted for one/two TCLK cycle	*/
#define ICCLR_ACK_WIDTH_MASK		(1 << ICCLR_ACK_WIDTH_OFFS)
#define ICCLR_ACK_WIDTH_ONE_TCLK	(0 << ICCLR_ACK_WIDTH_OFFS)
#define ICCLR_ACK_WIDTH_TWO_TCLK	(1 << ICCLR_ACK_WIDTH_OFFS)
#define ICCLR_DST_HOLD_OFFS			5   	/* Hold/increment source address*/
#define ICCLR_DST_HOLD_MASK			(1 << ICCLR_DST_HOLD_OFFS)
#define ICCLR_DST_INC		   		(0 << ICCLR_DST_HOLD_OFFS)
#define ICCLR_DST_HOLD		   		(1 << ICCLR_DST_HOLD_OFFS)
#define ICCLR_SRC_BURST_LIM_OFFS	6
#define ICCLR_SRC_BURST_LIM_MASK	(0x7 << ICCLR_SRC_BURST_LIM_OFFS)
#define ICCLR_SRC_BURST_LIM_8BYTE	(B_8BYTE   << ICCLR_SRC_BURST_LIM_OFFS)  
#define ICCLR_SRC_BURST_LIM_16BYTE	(B_16BYTE  << ICCLR_SRC_BURST_LIM_OFFS)
#define ICCLR_SRC_BURST_LIM_32BYTE	(B_32BYTE  << ICCLR_SRC_BURST_LIM_OFFS)    
#define ICCLR_SRC_BURST_LIM_64BYTE	(B_64BYTE  << ICCLR_SRC_BURST_LIM_OFFS)    
#define ICCLR_SRC_BURST_LIM_128BYTE	(B_128BYTE << ICCLR_SRC_BURST_LIM_OFFS)
#define ICCLR_CHAIN_MODE_OFFS	   	9
#define ICCLR_NON_CHAIN_MODE  	   	(1 << ICCLR_CHAIN_MODE_OFFS)
#define ICCLR_INT_MODE_OFFS			10  	/* Interrupt mode               */
#define ICCLR_INT_MODE_MASK			(1 << ICCLR_INT_MODE_OFFS)
#define ICCLR_INT_BYTE_CNT_ZERO		(0 << ICCLR_INT_MODE_OFFS)
#define ICCLR_INT_EVERY_NULL_PTR    (1 << ICCLR_INT_MODE_OFFS)
#define ICCLR_DEMAND_MODE_OFFS		11  	/* Demand/Block transfer mode	*/
#define ICCLR_DEMAND_MODE_MASK		(1 << ICCLR_DEMAND_MODE_OFFS)
#define ICCLR_DEMAND_MODE			(0 << ICCLR_DEMAND_MODE_OFFS)
#define ICCLR_BLOCK_MODE			(1 << ICCLR_DEMAND_MODE_OFFS)
#define ICCLR_CHAN_ENABLE			BIT12	/* Channel enable				*/
#define ICCLR_FETCH_NEXT_DESC	   	BIT13	/* Fetch next descriptor		*/
#define ICCLR_CHAN_ACTIVE	        BIT14  	/* Channel active				*/
#define ICCLR_REQ_DIR_OFFS			15		/* Request generated by src/dst	*/
#define ICCLR_REQ_DIR_MASK			(1 << ICCLR_REQ_DIR_OFFS)
#define ICCLR_REQ_DIR_SRC			(0 << ICCLR_REQ_DIR_OFFS)
#define ICCLR_REQ_DIR_DST			(1 << ICCLR_REQ_DIR_OFFS)
#define ICCLR_REQ_MODE_OFFS			16		/* DMAReqn is level/edge input	*/
#define ICCLR_REQ_MODE_MASK			(1 << ICCLR_REQ_MODE_OFFS)
#define ICCLR_REQ_MODE_LEVEL		(0 << ICCLR_REQ_MODE_OFFS)
#define ICCLR_REQ_MODE_EDGE			(1 << ICCLR_REQ_MODE_OFFS)
#define ICCLR_CLOSE_DESC_ENABLE	   	BIT17  	/* Close descriptor enable      */
#define ICCLR_EOT_ENABLE			BIT18  	/* End Of Tarnsfer (EOT) enable */
#define ICCLR_EOT_AFFECT_OFFS		19 /* EOT -> Fetch next descriptor/halt */
#define ICCLR_EOT_AFFECT_MASK		(1 << ICCLR_EOT_AFFECT_OFFS)
#define ICCLR_EOT_FETCH_NEXT		(0 << ICCLR_EOT_AFFECT_OFFS)
#define ICCLR_EOT_FETCH_HALT		(1 << ICCLR_EOT_AFFECT_OFFS)
#define ICCLR_CHANNEL_ABORT		   	BIT20  	/* Abort DMA transfer           */

#define ICCLR_OVRRD_SRC_OFFS		21	
#define ICCLR_OVRRD_SRC_MASK		(0x3 << ICCLR_OVRRD_SRC_OFFS)
#define ICCLR_OVRRD_SRC_BAR(barNo)	((barNo) << ICCLR_OVRRD_SRC_OFFS)

#define ICCLR_OVRRD_DST_OFFS		23	
#define ICCLR_OVRRD_DST_MASK		(0x3 << ICCLR_OVRRD_DST_OFFS)
#define ICCLR_OVRRD_DST_BAR(barNo)	((barNo) << ICCLR_OVRRD_DST_OFFS)

#define ICCLR_OVRRD_NDSC_OFFS		25	
#define ICCLR_OVRRD_NDSC_MASK		(0x3 << ICCLR_OVRRD_NDSC_OFFS)
#define ICCLR_OVRRD_NDSC_BAR(barNo)	((barNo) << ICCLR_OVRRD_NDSC_OFFS)

#define ICCLR_DESC_MODE_OFFS        31 		/* Descriptor mode 64KB/16M    	*/
#define ICCLR_DESC_MODE_MASK        (1 << ICCLR_DESC_MODE_OFFS)
#define ICCLR_DESC_MODE_64K         (0 << ICCLR_DESC_MODE_OFFS)
#define ICCLR_DESC_MODE_16M         (1 << ICCLR_DESC_MODE_OFFS)

/* IDMA Channel Control High Register (ICCHR)								*/
#define ICCHR_ENDIANESS_OFFS		0
#define ICCHR_ENDIANESS_MASK		(1 << ICCHR_ENDIANESS_OFFS)
#define ICCHR_ENDIAN_BIG			(0 << ICCHR_ENDIANESS_OFFS)
#define ICCHR_ENDIAN_LITTLE			(1 << ICCHR_ENDIANESS_OFFS)
#define ICCHR_DESC_BYTE_SWAP_EN		BIT1	/* swap the bytes of 64-bit 	*/
											/* dword during descriptor fetch*/
#define ICCHR_DESC_DEMAND_ENABLE	BIT2	/* Descriptor Demand Mode en	*/

/* IDMA Channel Interrupt Cause Register (ICICR) */
#define ICICR_CHAN_OFFS					8
#define ICICR_CAUSE_OFFS(chan)   		(chan * ICICR_CHAN_OFFS)
#define ICICR_CAUSE_MASK_ALL(chan)      (0xFF << ICICR_CAUSE_OFFS(chan))
#define ICICR_CAUSE_MASK(chan, cause)   (1 << (cause + ICICR_CAUSE_OFFS(chan)))
#define ICICR_COMP_MASK					0x01010101
#define ICICR_ERR_MASK					0x3e3e3e3e

/* IDMA Error Select Register (IESR)	*/
#define IESR_ERR_TYPE_OFFS			0
#define IESR_ERR_TYPE_MASK			(0x1f << IESR_ERR_TYPE_OFFS)



#endif	/* __INCmvIdmaSpech */

