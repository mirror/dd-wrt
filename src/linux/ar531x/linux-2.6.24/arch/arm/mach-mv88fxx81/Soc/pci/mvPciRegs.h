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

#ifndef __INCPCIREGSH
#define __INCPCIREGSH


/* defines */
#define MAX_PCI_DEVICES         32
#define PCI_MAX_PROT_WIN		6
#define MAX_PCI_FUNCS           8
#define MAX_PCI_BUSSES          256

/* 4KB granularity */
#define MINIMUM_WINDOW_SIZE     0x1000
#define MINIMUM_BAR_SIZE        0x1000
#define MINIMUM_BAR_SIZE_MASK	0xFFFFF000
#define BAR_SIZE_OFFS			12
#define BAR_SIZE_MASK			(0xFFFFF << BAR_SIZE_OFFS)

#define PCI_IO_WIN_NUM          1   /* Number of PCI_IO windows  */
#define PCI_MEM_WIN_NUM         4   /* Number of PCI_MEM windows */


/* enumerators */

/* This enumerator described the possible PCI slave targets.    	   */
/* PCI slave targets are designated memory/IO address spaces that the  */
/* PCI slave targets can access. They are also refered as "targets"    */
/* this enumeratoe order is determined by the content of : 
		PCI_BASE_ADDR_ENABLE_REG 				 					*/

typedef enum _mvPCIBars
{
    PCI_BAR_TBL_TERM = -1, /* none valid bar, used as bars list terminator */
    CS0_BAR,      		/* SDRAM chip select 0 bar*/
    CS1_BAR,      		/* SDRAM chip select 1 bar*/
    CS2_BAR,      		/* SDRAM chip select 2 bar*/
    CS3_BAR,      		/* SDRAM chip select 3 bar*/
    DEVCS0_BAR,     	/* Device chip select 0 bar*/
    DEVCS1_BAR,     	/* Device chip select 1 bar*/
    DEVCS2_BAR,     	/* Device chip select 2 bar*/
    BOOTCS_BAR,      	/* Boot device chip select bar*/
	MEM_INTER_REGS_BAR, /* Memory Mapped Internal bar */
	IO_INTER_REGS_BAR,  /* IO Mapped Internal bar */
    P2P_MEM0,      		/* P2P memory 0 */
    P2P_IO,        		/* P2P IO */
	PCI_MAX_BARS

}MV_PCI_BAR;


#define MV_PCI_BAR_IS_DRAM_BAR(bar)   \
                            ((bar >= CS0_BAR) && (bar <= CS3_BAR))


/* registers offsetes defines */

/****************************************/
/* PCI Slave Address Decoding registers */
/****************************************/
#define PCI_CS0_BAR_SIZE_REG(pciIf)					(0x30c08 + ((pciIf) * 0x80))
#define PCI_CS1_BAR_SIZE_REG(pciIf)					(0x30d08 + ((pciIf) * 0x80))
#define PCI_CS2_BAR_SIZE_REG(pciIf)					(0x30c0c + ((pciIf) * 0x80))
#define PCI_CS3_BAR_SIZE_REG(pciIf)					(0x30d0c + ((pciIf) * 0x80))
#define PCI_DEVCS0_BAR_SIZE_REG(pciIf)				(0x30c10 + ((pciIf) * 0x80))
#define PCI_DEVCS1_BAR_SIZE_REG(pciIf)				(0x30d10 + ((pciIf) * 0x80))
#define PCI_DEVCS2_BAR_SIZE_REG(pciIf)				(0x30d18 + ((pciIf) * 0x80))
#define PCI_BOOTCS_BAR_SIZE_REG(pciIf)				(0x30d14 + ((pciIf) * 0x80))
#define PCI_P2P_MEM0_BAR_SIZE_REG(pciIf)			(0x30d1c + ((pciIf) * 0x80))
#define PCI_P2P_IO_BAR_SIZE_REG(pciIf)				(0x30d24 + ((pciIf) * 0x80))
#define PCI_EXPAN_ROM_BAR_SIZE_REG(pciIf)			(0x30d2c + ((pciIf) * 0x80)) 
#define PCI_BASE_ADDR_ENABLE_REG(pciIf)				(0x30c3c + ((pciIf) * 0x80)) 
#define PCI_CS0_ADDR_REMAP_REG(pciIf)				(0x30c48 + ((pciIf) * 0x80)) 
#define PCI_CS1_ADDR_REMAP_REG(pciIf)				(0x30d48 + ((pciIf) * 0x80)) 
#define PCI_CS2_ADDR_REMAP_REG(pciIf)				(0x30c4c + ((pciIf) * 0x80)) 
#define PCI_CS3_ADDR_REMAP_REG(pciIf)				(0x30d4c + ((pciIf) * 0x80)) 
#define PCI_DEVCS0_ADDR_REMAP_REG(pciIf)			(0x30c50 + ((pciIf) * 0x80)) 
#define PCI_DEVCS1_ADDR_REMAP_REG(pciIf)			(0x30d50 + ((pciIf) * 0x80)) 
#define PCI_DEVCS2_ADDR_REMAP_REG(pciIf)			(0x30d58 + ((pciIf) * 0x80)) 
#define PCI_BOOTCS_ADDR_REMAP_REG(pciIf)			(0x30d54 + ((pciIf) * 0x80)) 
#define PCI_P2P_MEM0_ADDR_REMAP_LOW_REG(pciIf)		(0x30d5c + ((pciIf) * 0x80)) 
#define PCI_P2P_MEM0_ADDR_REMAP_HIGH_REG(pciIf)		(0x30d60 + ((pciIf) * 0x80)) 
#define PCI_P2P_IO_ADDR_REMAP_REG(pciIf)			(0x30d6c + ((pciIf) * 0x80)) 
#define PCI_EXPAN_ROM_ADDR_REMAP_REG(pciIf)			(0x30f38 + ((pciIf) * 0x80))
#define PCI_DRAM_BAR_BANK_SELECT_REG(pciIf)			(0x30c1c + ((pciIf) * 0x80))
#define PCI_ADDR_DECODE_CONTROL_REG(pciIf)			(0x30d3c + ((pciIf) * 0x80))

/* PCI Bars Size Registers (PBSR) */
#define PBSR_SIZE_OFFS				12
#define PBSR_SIZE_MASK				(0xfffff << PBSR_SIZE_OFFS)

/* Base Address Registers Enable Register (BARER) */
#define BARER_ENABLE(target)		(1 << (target))

/* PCI Base Address Remap Registers (PBARR) */
#define PBARR_REMAP_OFFS			12
#define PBARR_REMAP_MASK			(0xfffff << PBARR_REMAP_OFFS)
#define PBARR_REMAP_ALIGNMENT		(1 << PBARR_REMAP_OFFS)

/* PCI DRAM Bar Bank Select Register (PDBBSR) */
#define PDBBSR_DRAM_BANK_OFFS(bank)	((bank) * 2)
#define PDBBSR_DRAM_BANK_MASK(bank)	(0x3 << PDBBSR_DRAM_BANK_OFFS(bank))

/* PCI Address Decode Control Register (PADCR)*/
#define PADCR_REMAP_REG_WR_DIS		BIT0
#define PADCR_MSG_REG_ACC			BIT3

#define PADCR_VPD_HIGH_ADDR_OFFS	8 /* Bits [31:15] of the VPD address */
#define PADCR_VPD_HIGH_ADDR_MASK	(0x1ffff << PADCR_VPD_HIGH_ADDR_OFFS)

/* PCI Headers Retarget Control Register (PHRCR) */
#define PHRCR_ENABLE				BIT0
#define PHRCR_BUFF_SIZE_OFFS		1 
#define PHRCR_BUFF_SIZE_MASK		(0x7 << PHRCR_BUFF_SIZE_OFFS)
#define PHRCR_BUFF_SIZE_258BYTE		(0x0 << PHRCR_BUFF_SIZE_OFFS)
#define PHRCR_BUFF_SIZE_512BYTE		(0x1 << PHRCR_BUFF_SIZE_OFFS)
#define PHRCR_BUFF_SIZE_1KB			(0x2 << PHRCR_BUFF_SIZE_OFFS)
#define PHRCR_BUFF_SIZE_2KB			(0x3 << PHRCR_BUFF_SIZE_OFFS)
#define PHRCR_BUFF_SIZE_4KB			(0x4 << PHRCR_BUFF_SIZE_OFFS)
#define PHRCR_BUFF_SIZE_8KB			(0x5 << PHRCR_BUFF_SIZE_OFFS)
#define PHRCR_MASK1_OFFS			16
#define PHRCR_MASK1_MASK			(0xffff << PHRCR_MASK1_OFFS)

/* PCI Headers Retarget Base Register (PHRBR) */
#define PHRBR_BASE_OFFS				16
#define PHRBR_BASE_MASK				(0xffff << PHRBR_BASE_OFFS)

/* PCI Headers Retarget Base High Register (PHRBHR) */
#define PHRBHR_BASE_OFFS			0
#define PHRBHR_BASE_MASK			(0xffffffff << PHRBHR_BASE_OFFS)


/*************************/
/* PCI control registers */
/*************************/
/* maen : should add new registers */
#define PCI_CMD_REG(pciIf)				 		(0x30c00  + ((pciIf) * 0x80))
#define PCI_MODE_REG(pciIf)				 		(0x30d00  + ((pciIf) * 0x80))
#define PCI_RETRY_REG(pciIf)					(0x30c04  + ((pciIf) * 0x80))
#define PCI_DISCARD_TIMER_REG(pciIf)			(0x30d04  + ((pciIf) * 0x80))
#define PCI_ARBITER_CTRL_REG(pciIf)				(0x31d00 + ((pciIf) * 0x80))
#define PCI_P2P_CONFIG_REG(pciIf)				(0x31d14 + ((pciIf) * 0x80))
#define PCI_ACCESS_CTRL_BASEL_REG(pciIf, targetWin) \
							(0x31e00 + ((pciIf) * 0x80) + ((targetWin) * 0x10))
#define PCI_ACCESS_CTRL_BASEH_REG(pciIf, targetWin) \
							(0x31e04 + ((pciIf) * 0x80) + ((targetWin) * 0x10))
#define PCI_ACCESS_CTRL_SIZE_REG(pciIf, targetWin)	\
							(0x31e08 + ((pciIf) * 0x80) + ((targetWin) * 0x10))
							
#define PCI_DLL_CTRL_REG(pciIf)	   		 		(0x31d20  + ((pciIf) * 0x80))

/* PCI Dll Control (PDC)*/
#define PDC_DLL_EN					BIT0


/* PCI Command Register (PCR) */
#define PCR_MASTER_BYTE_SWAP_EN     BIT0
#define PCR_MASTER_WR_COMBINE_EN    BIT4
#define PCR_MASTER_RD_COMBINE_EN    BIT5
#define PCR_MASTER_WR_TRIG_WHOLE    BIT6
#define PCR_MASTER_RD_TRIG_WHOLE    BIT7
#define PCR_MASTER_MEM_RD_LINE_EN   BIT8
#define PCR_MASTER_MEM_RD_MULT_EN   BIT9
#define PCR_MASTER_WORD_SWAP_EN     BIT10
#define PCR_SLAVE_WORD_SWAP_EN      BIT11
#define PCR_NS_ACCORDING_RCV_TRANS  BIT14
#define PCR_MASTER_PCIX_REQ64N_EN   BIT15
#define PCR_SLAVE_BYTE_SWAP_EN      BIT16
#define PCR_MASTER_DAC_EN           BIT17
#define PCR_MASTER_M64_ALLIGN       BIT18
#define PCR_ERRORS_PROPAGATION_EN   BIT19
#define PCR_SLAVE_SWAP_ENABLE       BIT20
#define PCR_MASTER_SWAP_ENABLE      BIT21
#define PCR_MASTER_INT_SWAP_EN      BIT22
#define PCR_LOOP_BACK_ENABLE        BIT23
#define PCR_SLAVE_INTREG_SWAP_OFFS  24
#define PCR_SLAVE_INTREG_SWAP_MASK  0x3
#define PCR_SLAVE_INTREG_BYTE_SWAP  \
                             (MV_BYTE_SWAP << PCR_SLAVE_INT_REG_SWAP_MASK)
#define PCR_SLAVE_INTREG_NO_SWAP    \
                             (MV_NO_SWAP   << PCR_SLAVE_INT_REG_SWAP_MASK)
#define PCR_SLAVE_INTREG_BYTE_WORD  \
                             (MV_BYTE_WORD_SWAP << PCR_SLAVE_INT_REG_SWAP_MASK)
#define PCR_SLAVE_INTREG_WORD_SWAP  \
                             (MV_WORD_SWAP << PCR_SLAVE_INT_REG_SWAP_MASK)
#define PCR_RESET_REASSERTION_EN    BIT26
#define PCR_PCI_TO_CPU_REG_ORDER_EN BIT28
#define PCR_CPU_TO_PCI_ORDER_EN     BIT29
#define PCR_PCI_TO_CPU_ORDER_EN     BIT30

/* PCI Mode Register (PMR) */
#define PMR_PCI_ID_OFFS 			0  /* PCI Interface ID */
#define PMR_PCI_ID_MASK 			(0x1 << PMR_PCI_ID_OFFS)
#define PMR_PCI_ID_PCI(pciNum) 		((pciNum) << PCI_MODE_PCIID_OFFS)

#define PMR_PCI_64_OFFS				2 	/* 64-bit PCI Interface */
#define PMR_PCI_64_MASK				(0x1 << PMR_PCI_64_OFFS)
#define PMR_PCI_64_64BIT			(0x1 << PMR_PCI_64_OFFS)
#define PMR_PCI_64_32BIT			(0x0 << PMR_PCI_64_OFFS)

#define PMR_PCI_MODE_OFFS			4 	/* PCI interface mode of operation */
#define PMR_PCI_MODE_MASK			(0x3 << PMR_PCI_MODE_OFFS)
#define PMR_PCI_MODE_CONV			(0x0 << PMR_PCI_MODE_OFFS)
#define PMR_PCI_MODE_PCIX_66MHZ		(0x1 << PMR_PCI_MODE_OFFS)
#define PMR_PCI_MODE_PCIX_100MHZ	(0x2 << PMR_PCI_MODE_OFFS)
#define PMR_PCI_MODE_PCIX_133MHZ	(0x3 << PMR_PCI_MODE_OFFS)

#define PMR_EXP_ROM_SUPPORT			BIT8	/* Expansion ROM Active */

#define PMR_PCI_RESET_OFFS			31 /* PCI Interface Reset Indication */
#define PMR_PCI_RESET_MASK			(0x1 << PMR_PCI_RESET_OFFS)
#define PMR_PCI_RESET_PCIXRST		(0x0 << PMR_PCI_RESET_OFFS)


/* PCI Retry Register (PRR) */
#define PRR_RETRY_CNTR_OFFS			16 /* Retry Counter */
#define PRR_RETRY_CNTR_MAX			0xff
#define PRR_RETRY_CNTR_MASK			(PRR_RETRY_CNTR_MAX << PRR_RETRY_CNTR_OFFS)


/* PCI Discard Timer Register (PDTR) */
#define PDTR_TIMER_OFFS				0	/* Timer */
#define PDTR_TIMER_MAX				0xffff
#define PDTR_TIMER_MIN				0x7F
#define PDTR_TIMER_MASK				(PDTR_TIMER_MAX << PDTR_TIMER_OFFS)


/* PCI Arbiter Control Register (PACR) */
#define PACR_BROKEN_DETECT_EN		BIT1	/* Broken Detection Enable */

#define PACR_BROKEN_VAL_OFFS		3	/* Broken Value */
#define PACR_BROKEN_VAL_MASK		(0xf << PACR_BROKEN_VAL_OFFS)
#define PACR_BROKEN_VAL_CONV_MIN	0x2
#define PACR_BROKEN_VAL_PCIX_MIN	0x6

#define PACR_PARK_DIS_OFFS		14	/* Parking Disable */
#define PACR_PARK_DIS_MAX_AGENT	0x3f
#define PACR_PARK_DIS_MASK		(PACR_PARK_DIS_MAX_AGENT<<PACR_PARK_DIS_OFFS)
#define PACR_PARK_DIS(agent)	((1 << (agent)) << PACR_PARK_DIS_OFFS)

#define PACR_ARB_ENABLE				BIT31	/* Enable Internal Arbiter */


/* PCI P2P Configuration Register (PPCR) */
#define PPCR_2ND_BUS_L_OFFS			0	/* 2nd PCI Interface Bus Range Lower */
#define PPCR_2ND_BUS_L_MASK			(0xff << PPCR_2ND_BUS_L_OFFS)

#define PPCR_2ND_BUS_H_OFFS			8	/* 2nd PCI Interface Bus Range Upper */
#define PPCR_2ND_BUS_H_MASK			(0xff << PPCR_2ND_BUS_H_OFFS)

#define PPCR_BUS_NUM_OFFS			16  /* The PCI interface's Bus number */
#define PPCR_BUS_NUM_MASK			(0xff << PPCR_BUS_NUM_OFFS)

#define PPCR_DEV_NUM_OFFS			24  /* The PCI interface’s Device number */
#define PPCR_DEV_NUM_MASK			(0xff << PPCR_DEV_NUM_OFFS)


/* PCI Access Control Base Low Register (PACBLR) */
#define PACBLR_EN					BIT0 /* Access control window enable */

#define PACBLR_ACCPROT				BIT4 /* Access Protect */
#define PACBLR_WRPROT				BIT5 /* Write Protect */

#define PACBLR_PCISWAP_OFFS			6 	 /* PCI slave Data Swap Control */
#define PACBLR_PCISWAP_MASK			(0x3 << PACBLR_PCISWAP_OFFS)
#define PACBLR_PCISWAP_BYTE			(0x0 << PACBLR_PCISWAP_OFFS)
#define PACBLR_PCISWAP_NO_SWAP		(0x1 << PACBLR_PCISWAP_OFFS)
#define PACBLR_PCISWAP_BYTE_WORD	(0x2 << PACBLR_PCISWAP_OFFS)
#define PACBLR_PCISWAP_WORD			(0x3 << PACBLR_PCISWAP_OFFS)

#define PACBLR_RDMBURST_OFFS		8 /* Read Max Burst */
#define PACBLR_RDMBURST_MASK		(0x3 << PACBLR_RDMBURST_OFFS)
#define PACBLR_RDMBURST_32BYTE		(0x0 << PACBLR_RDMBURST_OFFS)
#define PACBLR_RDMBURST_64BYTE		(0x1 << PACBLR_RDMBURST_OFFS)
#define PACBLR_RDMBURST_128BYTE		(0x2 << PACBLR_RDMBURST_OFFS)

#define PACBLR_RDSIZE_OFFS			10 /* Typical PCI read transaction Size. */
#define PACBLR_RDSIZE_MASK			(0x3 << PACBLR_RDSIZE_OFFS)
#define PACBLR_RDSIZE_32BYTE		(0x0 << PACBLR_RDSIZE_OFFS)
#define PACBLR_RDSIZE_64BYTE		(0x1 << PACBLR_RDSIZE_OFFS)
#define PACBLR_RDSIZE_128BYTE		(0x2 << PACBLR_RDSIZE_OFFS)
#define PACBLR_RDSIZE_256BYTE		(0x3 << PACBLR_RDSIZE_OFFS)

#define PACBLR_BASE_L_OFFS			12	/* Corresponds to address bits [31:12] */
#define PACBLR_BASE_L_MASK			(0xfffff << PACBLR_BASE_L_OFFS)
#define PACBLR_BASE_L_ALIGNMENT		(1 << PACBLR_BASE_L_OFFS)
#define PACBLR_BASE_ALIGN_UP(base)  \
                             ((base+PACBLR_BASE_L_ALIGNMENT)&PACBLR_BASE_L_MASK)
#define PACBLR_BASE_ALIGN_DOWN(base)  (base & PACBLR_BASE_L_MASK)


/* PCI Access Control Base High Register (PACBHR) 	*/
#define PACBHR_BASE_H_OFFS			0	/* Corresponds to address bits [63:32] */
#define PACBHR_CTRL_BASE_H_MASK		(0xffffffff << PACBHR_BASE_H_OFFS)

/* PCI Access Control Size Register (PACSR) 		*/
#define PACSR_WRMBURST_OFFS			8 /* Write Max Burst */
#define PACSR_WRMBURST_MASK			(0x3 << PACSR_WRMBURST_OFFS)
#define PACSR_WRMBURST_32BYTE		(0x0 << PACSR_WRMBURST_OFFS)
#define PACSR_WRMBURST_64BYTE		(0x1 << PACSR_WRMBURST_OFFS)
#define PACSR_WRMBURST_128BYTE		(0x2 << PACSR_WRMBURST_OFFS)

#define PACSR_PCI_ORDERING			BIT11 /* PCI Ordering required */

#define PACSR_SIZE_OFFS				12	/* PCI access window size */
#define PACSR_SIZE_MASK				(0xfffff << PACSR_SIZE_OFFS)
#define PACSR_SIZE_ALIGNMENT		(1 << PACSR_SIZE_OFFS)
#define PACSR_SIZE_ALIGN_UP(size)   \
                                   ((size+PACSR_SIZE_ALIGNMENT)&PACSR_SIZE_MASK)
#define PACSR_SIZE_ALIGN_DOWN(size) (size & PACSR_SIZE_MASK)


/***************************************/
/* PCI Configuration Access Registers  */
/***************************************/

#define PCI_CONFIG_ADDR_REG(pciIf)	(0x30C78 - ((pciIf) * 0x80) )
#define PCI_CONFIG_DATA_REG(pciIf)	(0x30C7C - ((pciIf) * 0x80) )
#define PCI_INT_ACK_REG(pciIf)		(0x30C34 + ((pciIf) * 0x80) )

/* PCI Configuration Address Register (PCAR) */
#define PCAR_REG_NUM_OFFS			2
#define PCAR_REG_NUM_MASK			(0x3F << PCAR_REG_NUM_OFFS)
		
#define PCAR_FUNC_NUM_OFFS			8
#define PCAR_FUNC_NUM_MASK			(0x7 << PCAR_FUNC_NUM_OFFS)
		
#define PCAR_DEVICE_NUM_OFFS		11
#define PCAR_DEVICE_NUM_MASK		(0x1F << PCAR_DEVICE_NUM_OFFS)
		
#define PCAR_BUS_NUM_OFFS			16
#define PCAR_BUS_NUM_MASK			(0xFF << PCAR_BUS_NUM_OFFS)
		
#define PCAR_CONFIG_EN				BIT31


/***************************************/
/* PCI Configuration registers */
/***************************************/

/*********************************************/
/* PCI Configuration, Function 0, Registers  */
/*********************************************/


/* Standard registers */
#define PCI_DEVICE_AND_VENDOR_ID					0x000
#define PCI_STATUS_AND_COMMAND						0x004
#define PCI_CLASS_CODE_AND_REVISION_ID			    0x008
#define PCI_BIST_HDR_TYPE_LAT_TMR_CACHE_LINE		0x00C
#define PCI_MEMORY_BAR_BASE_ADDR(barNum)		 	(0x010 + ((barNum) << 2))
#define PCI_SUBSYS_ID_AND_SUBSYS_VENDOR_ID		 	0x02C
#define PCI_EXPANSION_ROM_BASE_ADDR_REG			    0x030
#define PCI_CAPABILTY_LIST_POINTER			        0x034
#define PCI_INTERRUPT_PIN_AND_LINE					0x03C

/* Marvell Specific */
#define PCI_SCS0_BASE_ADDR_LOW			   			0x010
#define PCI_SCS0_BASE_ADDR_HIGH			   			0x014
#define PCI_SCS1_BASE_ADDR_LOW		  				0x018
#define PCI_SCS1_BASE_ADDR_HIGH			  			0x01C
#define PCI_INTER_REG_MEM_MAPPED_BASE_ADDR_L 		0x020
#define PCI_INTER_REG_MEM_MAPPED_BASE_ADDR_H		0x024

/* capability list */
#define PCI_POWER_MNG_CAPABILITY		            0x040
#define PCI_POWER_MNG_STATUS_CONTROL		        0x044
#define PCI_VPD_ADDRESS_REG	                        0x048
#define PCI_VPD_DATA_REG	                        0x04c
#define PCI_MSI_MESSAGE_CONTROL						0x050
#define PCI_MSI_MESSAGE_ADDR		                0x054
#define PCI_MSI_MESSAGE_UPPER_ADDR		            0x058
#define PCI_MSI_MESSAGE_DATA		                0x05c
#define PCIX_COMMAND		                        0x060
#define PCIX_STATUS		                            0x064
#define PCI_COMPACT_PCI_HOT_SWAP		            0x068


/*********************************************/
/* PCI Configuration, Function 1, Registers  */
/*********************************************/

#define PCI_SCS2_BASE_ADDR_LOW						0x10
#define PCI_SCS2_BASE_ADDR_HIGH						0x14
#define PCI_SCS3_BASE_ADDR_LOW		 				0x18
#define PCI_SCS3_BASE_ADDR_HIGH						0x1c


/***********************************************/
/*  PCI Configuration, Function 2, Registers   */
/***********************************************/

#define PCI_DEVCS0_BASE_ADDR_LOW					0x10
#define PCI_DEVCS0_BASE_ADDR_HIGH		 			0x14
#define PCI_DEVCS1_BASE_ADDR_LOW		 			0x18
#define PCI_DEVCS1_BASE_ADDR_HIGH		      		0x1c
#define PCI_DEVCS2_BASE_ADDR_LOW		 			0x20
#define PCI_DEVCS2_BASE_ADDR_HIGH		      		0x24

/***********************************************/
/*  PCI Configuration, Function 3, Registers   */
/***********************************************/

#define PCI_BOOTCS_BASE_ADDR_LOW					0x18
#define PCI_BOOTCS_BASE_ADDR_HIGH		      		0x1c

/***********************************************/
/*  PCI Configuration, Function 4, Registers   */
/***********************************************/

#define PCI_P2P_MEM0_BASE_ADDR_LOW				   	0x10
#define PCI_P2P_MEM0_BASE_ADDR_HIGH		 			0x14
#define PCI_P2P_IO_BASE_ADDR		               	0x20
#define PCI_INTER_REGS_IO_MAPPED_BASE_ADDR		   0x24



/* PCI Device and Vendor ID Register (PDVIR) */
#define PDVIR_VEN_ID_OFFS			0 	/* Vendor ID */
#define PDVIR_VEN_ID_MASK			(0xffff << PDVIR_VEN_ID_OFFS)

#define PDVIR_DEV_ID_OFFS			16	/* Device ID */
#define PDVIR_DEV_ID_MASK  			(0xffff << PDVIR_DEV_ID_OFFS)

/* PCI Status and Command Register (PSCR) */
#define PSCR_IO_EN			BIT0 	/* IO Enable 							  */
#define PSCR_MEM_EN			BIT1	/* Memory Enable 						  */
#define PSCR_MASTER_EN		BIT2	/* Master Enable 						  */
#define PSCR_SPECIAL_EN		BIT3	/* Special Cycle Enable 				  */
#define PSCR_MEM_WRI_INV	BIT4	/* Memory Write and Invalidate Enable	  */
#define PSCR_VGA			BIT5	/* VGA Palette Snoops 					  */
#define PSCR_PERR_EN		BIT6	/* Parity Errors Respond Enable 		  */
#define PSCR_ADDR_STEP   	BIT7    /* Address Stepping Enable (Wait Cycle En)*/
#define PSCR_SERR_EN		BIT8	/* Ability to assert SERR# line			  */
#define PSCR_FAST_BTB_EN	BIT9	/* generate fast back-to-back transactions*/
#define PSCR_CAP_LIST		BIT20	/* Capability List Support 				  */
#define PSCR_66MHZ_EN		BIT21   /* 66 MHz Capable 						  */
#define PSCR_UDF_EN			BIT22   /* User definable features 				  */
#define PSCR_TAR_FAST_BB 	BIT23   /* fast back-to-back transactions capable */
#define PSCR_DATA_PERR		BIT24   /* Data Parity reported 				  */

#define PSCR_DEVSEL_TIM_OFFS 	25  /* DEVSEL timing */
#define PSCR_DEVSEL_TIM_MASK 	(0x3 << PSCR_DEVSEL_TIM_OFFS)
#define PSCR_DEVSEL_TIM_FAST	(0x0 << PSCR_DEVSEL_TIM_OFFS)
#define PSCR_DEVSEL_TIM_MED 	(0x1 << PSCR_DEVSEL_TIM_OFFS)
#define PSCR_DEVSEL_TIM_SLOW 	(0x2 << PSCR_DEVSEL_TIM_OFFS)

#define PSCR_SLAVE_TABORT	BIT27	/* Signalled Target Abort 	*/
#define PSCR_MASTER_TABORT	BIT28	/* Recieved Target Abort 	*/
#define PSCR_MABORT			BIT29	/* Recieved Master Abort 	*/
#define PSCR_SYSERR			BIT30	/* Signalled system error 	*/
#define PSCR_DET_PARERR		BIT31	/* Detect Parity Error 		*/

/* 	PCI configuration register offset=0x08 fields 
	(PCI_CLASS_CODE_AND_REVISION_ID)(PCCRI) 				*/

#define PCCRIR_REVID_OFFS		0		/* Revision ID */
#define PCCRIR_REVID_MASK		(0xff << PCCRIR_REVID_OFFS)

#define PCCRIR_FULL_CLASS_OFFS	8		/* Full Class Code */
#define PCCRIR_FULL_CLASS_MASK	(0xffffff << PCCRIR_FULL_CLASS_OFFS)

#define PCCRIR_PROGIF_OFFS		8		/* Prog .I/F*/
#define PCCRIR_PROGIF_MASK		(0xff << PCCRIR_PROGIF_OFFS)

#define PCCRIR_SUB_CLASS_OFFS	16		/* Sub Class*/
#define PCCRIR_SUB_CLASS_MASK	(0xff << PCCRIR_SUB_CLASS_OFFS)

#define PCCRIR_BASE_CLASS_OFFS	24		/* Base Class*/
#define PCCRIR_BASE_CLASS_MASK	(0xff << PCCRIR_BASE_CLASS_OFFS)

/* 	PCI configuration register offset=0x0C fields 
	(PCI_BIST_HEADER_TYPE_LATENCY_TIMER_CACHE_LINE)(PBHTLTCL) 				*/

#define PBHTLTCLR_CACHELINE_OFFS		0	/* Specifies the cache line size */
#define PBHTLTCLR_CACHELINE_MASK		(0xff << PBHTLTCLR_CACHELINE_OFFS)
	
#define PBHTLTCLR_LATTIMER_OFFS			8	/* latency timer */
#define PBHTLTCLR_LATTIMER_MASK			(0xff << PBHTLTCLR_LATTIMER_OFFS)

#define PBHTLTCLR_HEADTYPE_FULL_OFFS	16	/* Full Header Type */
#define PBHTLTCLR_HEADTYPE_FULL_MASK	(0xff << PBHTLTCLR_HEADTYPE_FULL_OFFS)

#define PBHTLTCLR_MULTI_FUNC			BIT23	/* Multi/Single function */

#define PBHTLTCLR_HEADER_OFFS			16		/* Header type */
#define PBHTLTCLR_HEADER_MASK			(0x7f << PBHTLTCLR_HEADER_OFFS)
#define PBHTLTCLR_HEADER_STANDARD		(0x0 << PBHTLTCLR_HEADER_OFFS)
#define PBHTLTCLR_HEADER_PCI2PCI_BRIDGE	(0x1 << PBHTLTCLR_HEADER_OFFS)


#define PBHTLTCLR_BISTCOMP_OFFS		24	/* BIST Completion Code */
#define PBHTLTCLR_BISTCOMP_MASK		(0xf << PBHTLTCLR_BISTCOMP_OFFS)

#define PBHTLTCLR_BISTACT			BIT30	/* BIST Activate bit */
#define PBHTLTCLR_BISTCAP			BIT31	/* BIST Capable Bit */


/* PCI Bar Base Low Register (PBBLR) */
#define PBBLR_IOSPACE			BIT0	/* Memory Space Indicator */

#define PBBLR_TYPE_OFFS			1	   /* BAR Type/Init Val. */ 
#define PBBLR_TYPE_MASK			(0x3 << PBBLR_TYPE_OFFS)
#define PBBLR_TYPE_32BIT_ADDR	(0x0 << PBBLR_TYPE_OFFS)
#define PBBLR_TYPE_64BIT_ADDR	(0x2 << PBBLR_TYPE_OFFS)

#define PBBLR_PREFETCH_EN		BIT3 	/* Prefetch Enable */

				
#define PBBLR_MEM_BASE_OFFS		4	/* Memory Bar Base address. Corresponds to
									address bits [31:4] */
#define PBBLR_MEM_BASE_MASK		(0xfffffff << PBBLR_MEM_BASE_OFFS)

#define PBBLR_IO_BASE_OFFS		2	/* IO Bar Base address. Corresponds to 
										address bits [31:2] */
#define PBBLR_IO_BASE_MASK		(0x3fffffff << PBBLR_IO_BASE_OFFS)


#define PBBLR_BASE_OFFS			12		/* Base address. Address bits [31:12] */
#define PBBLR_BASE_MASK			(0xfffff << PBBLR_BASE_OFFS)
#define PBBLR_BASE_ALIGNMET		(1 << PBBLR_BASE_OFFS)


/* PCI Bar Base High Fegister (PBBHR) */
#define PBBHR_BASE_OFFS			0		/* Base address. Address bits [31:12] */
#define PBBHR_BASE_MASK			(0xffffffff << PBBHR_BASE_OFFS)


/* 	PCI configuration register offset=0x2C fields 
	(PCI_SUBSYSTEM_ID_AND_SUBSYSTEM_VENDOR_ID)(PSISVI) 				*/

#define PSISVIR_VENID_OFFS	0	/* Subsystem Manufacturer Vendor ID Number */
#define PSISVIR_VENID_MASK	(0xffff << PSISVIR_VENID_OFFS)

#define PSISVIR_DEVID_OFFS	16	/* Subsystem Device ID Number */
#define PSISVIR_DEVID_MASK	(0xffff << PSISVIR_DEVID_OFFS)

/* 	PCI configuration register offset=0x30 fields 
	(PCI_EXPANSION_ROM_BASE_ADDR_REG)(PERBA) 				*/

#define PERBAR_EXPROMEN		BIT0	/* Expansion ROM Enable */

#define PERBAR_BASE_OFFS		12		/* Expansion ROM Base Address */
#define PERBAR_BASE_MASK		(0xfffff << PERBAR_BASE_OFFS) 	

/* 	PCI configuration register offset=0x34 fields 
	(PCI_CAPABILTY_LIST_POINTER)(PCLP) 				*/

#define PCLPR_CAPPTR_OFFS	0		/* Capability List Pointer */
#define PCLPR_CAPPTR_MASK	(0xff << PCLPR_CAPPTR_OFFS)

/* 	PCI configuration register offset=0x3C fields 
	(PCI_INTERRUPT_PIN_AND_LINE)(PIPL) 				*/

#define PIPLR_INTLINE_OFFS	0	/* Interrupt line (IRQ) */
#define PIPLR_INTLINE_MASK	(0xff << PIPLR_INTLINE_OFFS)

#define PIPLR_INTPIN_OFFS	8	/* interrupt pin (A,B,C,D) */
#define PIPLR_INTPIN_MASK	(0xff << PIPLR_INTPIN_OFFS)

#define PIPLR_MINGRANT_OFFS	16	/* Minimum Grant on 250 nano seconds units */
#define PIPLR_MINGRANT_MASK	(0xff << PIPLR_MINGRANT_OFFS)

#define PIPLR_MAXLATEN_OFFS	24	/* Maximum latency on 250 nano seconds units */
#define PIPLR_MAXLATEN_MASK	(0xff << PIPLR_MAXLATEN_OFFS)

/* PCIX_STATUS  register fields (PXS) */

#define PXS_FN_OFFS		0	/* Description Number */
#define PXS_FN_MASK		(0x7 << PXS_FN_OFFS)

#define PXS_DN_OFFS		3	/* Device Number */
#define PXS_DN_MASK		(0x1f << PXS_DN_OFFS)

#define PXS_BN_OFFS		8	/* Bus Number */
#define PXS_BN_MASK		(0xff << PXS_BN_OFFS)


/* PCI Error Report Register Map */
#define PCI_SERRN_MASK_REG(pciIf)		(0x30c28  + (pciIf * 0x80))
#define PCI_CAUSE_REG(pciIf)			(0x31d58 + (pciIf * 0x80))
#define PCI_MASK_REG(pciIf)				(0x31d5C + (pciIf * 0x80))
#define PCI_ERROR_ADDR_LOW_REG(pciIf)	(0x31d40 + (pciIf * 0x80))
#define PCI_ERROR_ADDR_HIGH_REG(pciIf)	(0x31d44 + (pciIf * 0x80))
#define PCI_ERROR_ATTRIBUTE_REG(pciIf)	(0x31d48 + (pciIf * 0x80))
#define PCI_ERROR_COMMAND_REG(pciIf)	(0x31d50 + (pciIf * 0x80))

/* PCI Interrupt Cause Register (PICR) */
#define PICR_ERR_SEL_OFFS           27
#define PICR_ERR_SEL_MASK           (0x1f << PICR_ERR_SEL_OFFS)

/* PCI Error Command Register (PECR) */
#define PECR_ERR_CMD_OFFS			0
#define PECR_ERR_CMD_MASK			(0xf << PECR_ERR_CMD_OFFS)
#define PECR_DAC					BIT4

#endif /* #ifndef __INCPCIREGSH */

