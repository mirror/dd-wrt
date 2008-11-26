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
#ifndef __INCmvDeviceRegsH
#define __INCmvDeviceRegsH

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define MV_DEVICE_MAX_XBAR_TIMEOUT  0x0FFF
#define DEV_BANK_PARAM_REG_DV	    0x80000000
/* registers offsets */

#if defined(MV78XX0)

static INLINE MV_U32 DEV_BANK_PARAM_REG(int num)
{
	switch(num)
	{
		case (BOOT_CS):
			return DEVICE_BUS_BASE + 0x400;
		case (DEV_CS0):
			return DEVICE_BUS_BASE + 0x408;
		case (DEV_CS1):
			return DEVICE_BUS_BASE + 0x410;
		case (DEV_CS2):
			return DEVICE_BUS_BASE + 0x418;
		case (DEV_CS3):
			return DEVICE_BUS_BASE + 0x420;
		default: 
			return 0xFFFFFFFF;

	}	    
}

#define DEV_BANK_PARAM_REG_WR(num)		(DEV_BANK_PARAM_REG(num)+0x4)
#define DEV_NAND_CTRL_REG			    (DEVICE_BUS_BASE + 0x0470)

/* Device Bank Parameters register fields (DBP_REG)*/
/* Boot Device Bank Parameters (DBP) register fields (DEV_BOOT_BANK_PARAM_REG)*/
/* DBP_XXX_MASK_HIGH is the offset of the extend bit from the msb of the input value */

#define DBP_TURNOFF_OFFS_LOW		0
#define DBP_TURNOFF_MASK_LOW		0x3F
#define MAX_DBP_TURNOFF				0xf


#define DBP_TURNOFF_SET(value)			\
((value & DBP_TURNOFF_MASK_LOW) << DBP_TURNOFF_OFFS_LOW)

#define DBP_TURNOFF_GET(value)			\
((value >> DBP_TURNOFF_OFFS_LOW) & DBP_TURNOFF_MASK_LOW)

#define DBP_ACC2FIRST_OFFS_LOW		6
#define DBP_ACC2FIRST_MASK_LOW		0x3f
#define MAX_DBP_ACC2FIRST			0x3f

#define DBP_ACC2FIRST_SET(value)			\
((value & DBP_ACC2FIRST_MASK_LOW) << DBP_ACC2FIRST_OFFS_LOW) 

#define DBP_ACC2FIRST_GET(value)			\
((value >> DBP_ACC2FIRST_OFFS_LOW) & DBP_ACC2FIRST_MASK_LOW)

#define DBP_ACC2NEXT_OFFS_LOW		17
#define DBP_ACC2NEXT_MASK_LOW		0x3f
#define MAX_DBP_ACC2NEXT			0x3f

#define DBP_ACC2NEXT_SET(value)			\
((value & DBP_ACC2FIRST_MASK_LOW) << DBP_ACC2FIRST_OFFS_LOW) 

#define DBP_ACC2NEXT_GET(value)			\
((value >> DBP_ACC2NEXT_OFFS_LOW) & DBP_ACC2NEXT_MASK_LOW) 

#define DBP_DEVWIDTH_OFFS		30 /* Device Width */
#define DBP_DEVWIDTH_MASK		(0x3 << DBP_DEVWIDTH_OFFS)
#define DBP_DEVWIDTH_8BIT		(0x0 << DBP_DEVWIDTH_OFFS)
#define DBP_DEVWIDTH_16BIT		(0x1 << DBP_DEVWIDTH_OFFS)
#define DBP_DEVWIDTH_32BIT		(0x2 << DBP_DEVWIDTH_OFFS)

#define DBP_BADRSKEW_OFFS		28
#define DBP_BADRSKEW_MASK		(0x3 << DBP_BADRSKEW_OFFS)
#define DBP_BADRSKEW_NOGAP		(0x0 << DBP_BADRSKEW_OFFS)
#define DBP_BADRSKEW_1CYCLE		(0x1 << DBP_BADRSKEW_OFFS)
#define DBP_BADRSKEW_2CYCLE		(0x2 << DBP_BADRSKEW_OFFS)

#define DBP_ALE2WR_OFFS_LOW			0
#define DBP_ALE2WR_MASK_LOW			0x3f
#define MAX_DBP_ALE2WR				0x3F


#define DBP_ALE2WR_SET(value)			\
((value & DBP_ALE2WR_MASK_LOW) << DBP_ALE2WR_OFFS_LOW) 

#define DBP_ALE2WR_GET(value)			\
((value >> DBP_ALE2WR_OFFS_LOW) & DBP_ALE2WR_MASK_LOW) 

#define DBP_WRLOW_OFFS_LOW			8
#define DBP_WRLOW_MASK_LOW			0x3F
#define MAX_DBP_WRLOW				0x3F

#define DBP_WRLOW_SET(value)			\
((value & DBP_WRLOW_MASK_LOW) << DBP_WRLOW_OFFS_LOW)

#define DBP_WRLOW_GET(value)			\
((value >> DBP_WRLOW_OFFS_LOW) & DBP_WRLOW_MASK_LOW)

#define DBP_WRHIGH_OFFS_LOW			16
#define DBP_WRHIGH_MASK_LOW			0x3F
#define MAX_DBP_WRHIGH				0x3F

#define DBP_WRHIGH_SET(value)			\
((value & DBP_WRHIGH_MASK_LOW) << DBP_WRHIGH_OFFS_LOW)

#define DBP_WRHIGH_GET(value)			\
((value >> DBP_WRHIGH_OFFS_LOW) & DBP_WRHIGH_MASK_LOW)


/* Device Interface Control register fields (DIC) (DIC_REG)*/
#define DIC_TIMEOUT_OFFS 	0 /* Timeout Timer Preset Value. */
#define DIC_TIMEOUT_MASK 	(0xffff << DIC_TIMEOUT_OFFS)
#define MAX_DIC_TIMEOUT		0xffff

/* NAND Flash Control register fields (NF) (NF_REG)*/
#define NF_BOOTCS_OFFS 				0 /* Define if BOOTCS is connected to NAND Flash */
#define NF_BOOT_MASK 				(1 << NF_BOOTCS_OFFS)
#define NF_BOOT_NC				    (0 << NF_BOOTCS_OFFS)
#define NF_BOOT_C				    (1 << NF_BOOTCS_OFFS)

#define NF_BOOTCS_CE_ACT_OFFS 		1 /* Define if NAND Flash on BOOTCS is CE care or CE don't care */
#define NF_BOOTCS_CE_ACT_MASK 		(1 << NF_BOOTCS_CE_ACT_OFFS)
#define NF_BOOTCS_CE_ACT_NCARE		(0 << NF_BOOTCS_CE_ACT_OFFS)
#define NF_BOOTCS_CE_ACT_CARE		(1 << NF_BOOTCS_CE_ACT_OFFS)

#define NF_CS0_OFFS 				2 /* Define if CS0 is connected to NAND Flash */
#define NF_CS0_MASK 				(1 << NF_CS0_OFFS)
#define NF_CS0_NC				    (0 << NF_CS0_OFFS)
#define NF_CS0_C				    (1 << NF_CS0_OFFS)

#define NF_CS0_CE_ACT_OFFS 			3 /* Define if NAND Flash on CS0 is CE care or CE don't care */
#define NF_CS0_CE_ACT_MASK 			(1 << NF_CS0_CE_ACT_OFFS)
#define NF_CS0_CE_ACT_NCARE			(0 << NF_CS0_CE_ACT_OFFS)
#define NF_CS0_CE_ACT_CARE			(1 << NF_CS0_CE_ACT_OFFS)

#define NF_CS1_OFFS 				4 /* Define if CS1 is connected to NAND Flash */
#define NF_CS1_MASK 				(1 << NF_CS1_OFFS)
#define NF_CS1_NC				    (0 << NF_CS1_OFFS)
#define NF_CS1_C				    (1 << NF_CS1_OFFS)

#define NF_CS1_CE_ACT_OFFS 			5 /* Define if NAND Flash on CS1 is CE care or CE don't care */
#define NF_CS1_CE_ACT_MASK 			(1 << NF_CS1_CE_ACT_OFFS)
#define NF_CS1_CE_ACT_NCARE			(0 << NF_CS1_CE_ACT_OFFS)
#define NF_CS1_CE_ACT_CARE			(1 << NF_CS1_CE_ACT_OFFS)

#define NF_CS2_OFFS 				6 /* Define if CS2 is connected to NAND Flash */
#define NF_CS2_MASK 				(1 << NF_CS2_OFFS)
#define NF_CS2_NC				    (0 << NF_CS2_OFFS)
#define NF_CS2_C				    (1 << NF_CS2_OFFS)

#define NF_CS2_CE_ACT_OFFS 			7 /* Define if NAND Flash on CS2 is CE care or CE don't care */
#define NF_CS2_CE_ACT_MASK 			(1 << NF_CS2_CE_ACT_OFFS)
#define NF_CS2_CE_ACT_NCARE			(0 << NF_CS2_CE_ACT_OFFS)
#define NF_CS2_CE_ACT_CARE			(1 << NF_CS2_CE_ACT_OFFS)

#define NF_INIT_SEQ_OFFS 			8 /* NAND Flash initialization sequence */
#define NF_INIT_SEQ_MASK			(1 << NF_INIT_SEQ_OFFS)
#define NF_INIT_SEQ_EN				(0 << NF_INIT_SEQ_OFFS)
#define NF_INIT_SEQ_DIS				(1 << NF_INIT_SEQ_OFFS)

#define NF_OE_HIGHW_OFFS 			9 /* NAND Flash OE high width in core clocks units (value + 1) */
#define NF_OE_HIGHW_MASK			(0x1f << NF_OE_HIGHW_OFFS)
#define MAX_OE_HIGHW				(0x1f << NF_OE_HIGHW_OFFS)

#define NF_TREADY_OFFS 				14 /* NAND Flash time ready in core clocks units (value + 1) */
#define NF_TREADY_MASK				(0x1f << NF_TREADY_OFFS)
#define MAX_TREADY				    (0x1f << NF_TREADY_OFFS)

#define NF_OE_TCTRL_OFFS 			19 /* NAND Flash OE toggle control */
#define NF_OE_TCTRL_MASK			(1 << NF_OE_TCTRL_OFFS)
#define NF_OE_TCTRL_1_CYC_AFT		(0 << NF_OE_TCTRL_OFFS)
#define NF_OE_TCTRL_SAME_CYC		(1 << NF_OE_TCTRL_OFFS)

#define NF_CS3_OFFS 				20 /* Define if CS3 is connected to NAND Flash */
#define NF_CS3_MASK 				(1 << NF_CS3_OFFS)
#define NF_CS3_NC				    (0 << NF_CS3_OFFS)
#define NF_CS3_C				    (1 << NF_CS3_OFFS)

#define NF_CS3_CE_ACT_OFFS 			21 /* Define if NAND Flash on CS3 is CE care or CE don't care */
#define NF_CS3_CE_ACT_MASK 			(1 << NF_CS3_CE_ACT_OFFS)
#define NF_CS3_CE_ACT_NCARE			(0 << NF_CS3_CE_ACT_OFFS)
#define NF_CS3_CE_ACT_CARE			(1 << NF_CS3_CE_ACT_OFFS)


/* Device Interface NAND Flash Control Register (DINFCR) */
#define DINFCR_NF_CS_MASK(csNum)         \
(csNum == BOOT_CS) ?  0x1 : ((csNum == DEV_CS3) ? (0x1 << 20) : (0x1 << (((csNum+1) % MV_DEV_MAX_CS) * 2)))


#define DINFCR_NF_ACT_CE_MASK(csNum)     \
(csNum == DEV_CS3) ? (0x2 << 20) : (0x2 << (((csNum+1) % MV_DEV_MAX_CS) * 2))


#else

#define DEV_BANK_PARAM_REG(num)			(DEVICE_BUS_BASE + 0x045C + (num)*4)
#define DEV_BOOT_BANK_PARAM_REG			(DEVICE_BUS_BASE + 0x046C)
#define DEV_NAND_CTRL_REG			    (DEVICE_BUS_BASE + 0x04CC)
#define DEV_BUS_CTRL_REG			    (DEVICE_BUS_BASE + 0x04C0)
#define DEV_INTR_CAUSE_REG			    (DEVICE_BUS_BASE + 0x04D0)
#define DEV_INTR_MASK_REG			    (DEVICE_BUS_BASE + 0x04D4)
#define DEV_ERR_ADDR_REG			    (DEVICE_BUS_BASE + 0x0208)
#define DEV_PWR_SAVE_CTRL_REG			    (DEVICE_BUS_BASE + 0x0058)
#define DEV_PWR_SAVE_CTRL_MASK		0xFFFF

/* Device Bank Parameters register fields (DBP_REG)*/
/* Boot Device Bank Parameters (DBP) register fields (DEV_BOOT_BANK_PARAM_REG)*/
/* DBP_XXX_MASK_HIGH is the offset of the extend bit from the msb of the input value */

#define DBP_TURNOFF_OFFS_LOW		0
#define DBP_TURNOFF_MASK_LOW		0x7
#define DBP_TURNOFF_OFFS_HIGH		18			
#define DBP_TURNOFF_MASK_HIGH		0x8
#define MAX_DBP_TURNOFF				0xf

#define DBP_TURNOFF_SET(value)			\
((value & DBP_TURNOFF_MASK_LOW) << DBP_TURNOFF_OFFS_LOW) | \
((value & DBP_TURNOFF_MASK_HIGH) << DBP_TURNOFF_OFFS_HIGH)

#define DBP_TURNOFF_GET(value)			\
((value >> DBP_TURNOFF_OFFS_LOW) & DBP_TURNOFF_MASK_LOW) | \
((value >> DBP_TURNOFF_OFFS_HIGH) & DBP_TURNOFF_MASK_HIGH)

#define DBP_ACC2FIRST_OFFS_LOW		3
#define DBP_ACC2FIRST_MASK_LOW		0xf
#define DBP_ACC2FIRST_OFFS_HIGH		17
#define DBP_ACC2FIRST_MASK_HIGH		0x10
#define MAX_DBP_ACC2FIRST			0x1f

#define DBP_ACC2FIRST_SET(value)			\
((value & DBP_ACC2FIRST_MASK_LOW) << DBP_ACC2FIRST_OFFS_LOW) | \
((value & DBP_ACC2FIRST_MASK_HIGH) << DBP_ACC2FIRST_OFFS_HIGH)

#define DBP_ACC2FIRST_GET(value)			\
((value >> DBP_ACC2FIRST_OFFS_LOW) & DBP_ACC2FIRST_MASK_LOW) | \
((value >> DBP_ACC2FIRST_OFFS_HIGH) & DBP_ACC2FIRST_MASK_HIGH)


#define DBP_ACC2NEXT_OFFS_LOW		7
#define DBP_ACC2NEXT_MASK_LOW		0xf
#define DBP_ACC2NEXT_OFFS_HIGH		18
#define DBP_ACC2NEXT_MASK_HIGH		0x10
#define MAX_DBP_ACC2NEXT		    0x1f

#define DBP_ACC2NEXT_SET(value)			\
((value & DBP_ACC2FIRST_MASK_LOW) << DBP_ACC2FIRST_OFFS_LOW) | \
((value & DBP_ACC2FIRST_MASK_HIGH) << DBP_ACC2FIRST_OFFS_HIGH)

#define DBP_ACC2NEXT_GET(value)			\
((value >> DBP_ACC2NEXT_OFFS_LOW) & DBP_ACC2NEXT_MASK_LOW) | \
((value >> DBP_ACC2NEXT_OFFS_HIGH) & DBP_ACC2NEXT_MASK_HIGH)


#define DBP_ALE2WR_OFFS_LOW			11
#define DBP_ALE2WR_MASK_LOW			0x7
#define DBP_ALE2WR_OFFS_HIGH		21
#define DBP_ALE2WR_MASK_HIGH		0x8
#define MAX_DBP_ALE2WR			    0xf

#define DBP_ALE2WR_SET(value)			\
((value & DBP_ALE2WR_MASK_LOW) << DBP_ALE2WR_OFFS_LOW) | \
((value & DBP_ALE2WR_MASK_HIGH) << DBP_ALE2WR_OFFS_HIGH)

#define DBP_ALE2WR_GET(value)			\
((value >> DBP_ALE2WR_OFFS_LOW) & DBP_ALE2WR_MASK_LOW) | \
((value >> DBP_ALE2WR_OFFS_HIGH) & DBP_ALE2WR_MASK_HIGH)


#define DBP_WRLOW_OFFS_LOW			14
#define DBP_WRLOW_MASK_LOW			0x7
#define DBP_WRLOW_OFFS_HIGH			23
#define DBP_WRLOW_MASK_HIGH			0x4
#define MAX_DBP_WRLOW				0xf

#define DBP_WRLOW_SET(value)			\
((value & DBP_WRLOW_MASK_LOW) << DBP_WRLOW_OFFS_LOW) | \
((value & DBP_WRLOW_MASK_HIGH) << DBP_WRLOW_OFFS_HIGH)

#define DBP_WRLOW_GET(value)			\
((value >> DBP_WRLOW_OFFS_LOW) & DBP_WRLOW_MASK_LOW) | \
((value >> DBP_WRLOW_OFFS_HIGH) & DBP_WRLOW_MASK_HIGH)

#define DBP_WRHIGH_OFFS_LOW			14
#define DBP_WRHIGH_MASK_LOW			0x7
#define DBP_WRHIGH_OFFS_HIGH		24
#define DBP_WRHIGH_MASK_HIGH		0x4
#define MAX_DBP_WRHIGH				0xf

#define DBP_WRHIGH_SET(value)			\
((value & DBP_WRHIGH_MASK_LOW) << DBP_WRHIGH_OFFS_LOW) | \
((value & DBP_WRHIGH_MASK_HIGH) << DBP_WRHIGH_OFFS_HIGH)

#define DBP_WRHIGH_GET(value)			\
((value >> DBP_WRHIGH_OFFS_LOW) & DBP_WRHIGH_MASK_LOW) | \
((value >> DBP_WRHIGH_OFFS_HIGH) & DBP_WRHIGH_MASK_HIGH)

#define DBP_DEVWIDTH_OFFS		20 /* Device Width */
#define DBP_DEVWIDTH_MASK		(0x3 << DBP_DEVWIDTH_OFFS)
#define DBP_DEVWIDTH_8BIT		(0x0 << DBP_DEVWIDTH_OFFS)
#define DBP_DEVWIDTH_16BIT		(0x1 << DBP_DEVWIDTH_OFFS)

#define DBP_BADRSKEW_OFFS		28
#define DBP_BADRSKEW_MASK		(0x3 << DBP_BADRSKEW_OFFS)
#define DBP_BADRSKEW_NOGAP		(0x0 << DBP_BADRSKEW_OFFS)
#define DBP_BADRSKEW_1CYCLE		(0x1 << DBP_BADRSKEW_OFFS)
#define DBP_BADRSKEW_2CYCLE		(0x2 << DBP_BADRSKEW_OFFS)

/* Device Interface Control register fields (DIC) (DIC_REG)*/

#define DIC_TIMEOUT_OFFS 	0 /* Timeout Timer Preset Value. */
#define DIC_TIMEOUT_MASK 	(0xffff << DIC_TIMEOUT_OFFS)
#define MAX_DIC_TIMEOUT		0xffff

#define DIC_PARSEL				BIT17 /* Even or Odd parity select */
#define DIC_RDDPPAR_OFFS			17 /* Read data path parity select */
#define DIC_RDDPPAR_MASK	(1 << DIC_RDDPPAR_OFFS)
#define DIC_RDDPPAR_EVEN	(0 << DIC_RDDPPAR_OFFS)
#define DIC_RDDPPAR_ODD		(1 << DIC_RDDPPAR_OFFS)

/* NAND Flash Control register fields (NF) (NF_REG)*/

#define NF_BOOTCS_OFFS 				0 /* Define if BOOTCS is connected to NAND Flash */
#define NF_BOOT_MASK 				(1 << NF_BOOTCS_OFFS)
#define NF_BOOT_NC				    (0 << NF_BOOTCS_OFFS)
#define NF_BOOT_C				    (1 << NF_BOOTCS_OFFS)

#define NF_BOOTCS_CE_ACT_OFFS 		1 /* Define if NAND Flash on BOOTCS is CE care or CE don't care */
#define NF_BOOTCS_CE_ACT_MASK 		(1 << NF_BOOTCS_CE_ACT_OFFS)
#define NF_BOOTCS_CE_ACT_NCARE		(0 << NF_BOOTCS_CE_ACT_OFFS)
#define NF_BOOTCS_CE_ACT_CARE		(1 << NF_BOOTCS_CE_ACT_OFFS)

#define NF_CS0_OFFS 				2 /* Define if CS0 is connected to NAND Flash */
#define NF_CS0_MASK 				(1 << NF_CS0_OFFS)
#define NF_CS0_NC				    (0 << NF_CS0_OFFS)
#define NF_CS0_C				    (1 << NF_CS0_OFFS)

#define NF_CS0_CE_ACT_OFFS 			3 /* Define if NAND Flash on CS0 is CE care or CE don't care */
#define NF_CS0_CE_ACT_MASK 			(1 << NF_CS0_CE_ACT_OFFS)
#define NF_CS0_CE_ACT_NCARE			(0 << NF_CS0_CE_ACT_OFFS)
#define NF_CS0_CE_ACT_CARE			(1 << NF_CS0_CE_ACT_OFFS)

#define NF_CS1_OFFS 				4 /* Define if CS1 is connected to NAND Flash */
#define NF_CS1_MASK 				(1 << NF_CS1_OFFS)
#define NF_CS1_NC				    (0 << NF_CS1_OFFS)
#define NF_CS1_C				    (1 << NF_CS1_OFFS)

#define NF_CS1_CE_ACT_OFFS 			5 /* Define if NAND Flash on CS1 is CE care or CE don't care */
#define NF_CS1_CE_ACT_MASK 			(1 << NF_CS1_CE_ACT_OFFS)
#define NF_CS1_CE_ACT_NCARE			(0 << NF_CS1_CE_ACT_OFFS)
#define NF_CS1_CE_ACT_CARE			(1 << NF_CS1_CE_ACT_OFFS)

#define NF_CS2_OFFS 				6 /* Define if CS2 is connected to NAND Flash */
#define NF_CS2_MASK 				(1 << NF_CS2_OFFS)
#define NF_CS2_NC				    (0 << NF_CS2_OFFS)
#define NF_CS2_C				    (1 << NF_CS2_OFFS)

#define NF_CS2_CE_ACT_OFFS 			7 /* Define if NAND Flash on CS2 is CE care or CE don't care */
#define NF_CS2_CE_ACT_MASK 			(1 << NF_CS2_CE_ACT_OFFS)
#define NF_CS2_CE_ACT_NCARE			(0 << NF_CS2_CE_ACT_OFFS)
#define NF_CS2_CE_ACT_CARE			(1 << NF_CS2_CE_ACT_OFFS)

#define NF_INIT_SEQ_OFFS 			8 /* NAND Flash initialization sequence */
#define NF_INIT_SEQ_MASK			(1 << NF_INIT_SEQ_OFFS)
#define NF_INIT_SEQ_EN				(0 << NF_INIT_SEQ_OFFS)
#define NF_INIT_SEQ_DIS				(1 << NF_INIT_SEQ_OFFS)

#define NF_OE_HIGHW_OFFS 			9 /* NAND Flash OE high width in core clocks units (value + 1) */
#define NF_OE_HIGHW_MASK			(0x1f << NF_OE_HIGHW_OFFS)
#define MAX_OE_HIGHW				(0x1f << NF_OE_HIGHW_OFFS)

#define NF_TREADY_OFFS 				14 /* NAND Flash time ready in core clocks units (value + 1) */
#define NF_TREADY_MASK				(0x1f << NF_TREADY_OFFS)
#define MAX_TREADY				    (0x1f << NF_TREADY_OFFS)

#define NF_OE_TCTRL_OFFS 			19 /* NAND Flash OE toggle control */
#define NF_OE_TCTRL_MASK			(1 << NF_OE_TCTRL_OFFS)
#define NF_OE_TCTRL_1_CYC_AFT		(0 << NF_OE_TCTRL_OFFS)
#define NF_OE_TCTRL_SAME_CYC		(1 << NF_OE_TCTRL_OFFS)

#define NF_CS3_OFFS 				6 /* Define if CS3 is connected to NAND Flash */
#define NF_CS3_MASK 				(1 << NF_CS3_OFFS)
#define NF_CS3_NC				    (0 << NF_CS3_OFFS)
#define NF_CS3_C				    (1 << NF_CS3_OFFS)

#define NF_CS3_CE_ACT_OFFS 			7 /* Define if NAND Flash on CS3 is CE care or CE don't care */
#define NF_CS3_CE_ACT_MASK 			(1 << NF_CS3_CE_ACT_OFFS)
#define NF_CS3_CE_ACT_NCARE			(0 << NF_CS3_CE_ACT_OFFS)
#define NF_CS3_CE_ACT_CARE			(1 << NF_CS3_CE_ACT_OFFS)


/* Device Interface NAND Flash Control Register (DINFCR) */
#define DINFCR_NF_CS_MASK(csNum)         \
(csNum == BOOT_CS) ?  0x1 : ((csNum == DEV_CS3) ? (0x1 << 20) : (0x1 << (((csNum+1) % MV_DEV_MAX_CS) * 2)))


#define DINFCR_NF_ACT_CE_MASK(csNum)     \
(csNum == DEV_CS3) ? (0x2 << 20) : (0x2 << (((csNum+1) % MV_DEV_MAX_CS) * 2))

/* Power Save Control Register (PSCR) */
#define PSCR_PCI_PD         BIT7
#define PSCR_PEX0_PD        BIT8
#define PSCR_PEX10_PD       BIT12
#define PSCR_PEX11_PD       BIT13
#define PSCR_PEX12_PD       BIT14
#define PSCR_PEX13_PD       BIT15
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __INCmvDeviceRegsH */
