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

#ifndef __INCSysPEXH
#define __INCSysPEXH

#include "mvCommon.h"
#include "ctrlEnv/sys/mvCpuIf.h"
#include "ctrlEnv/mvCtrlEnvLib.h"
#include "ctrlEnv/mvCtrlEnvAddrDec.h"
#include "pex/mvPex.h"


#define MV_PEX_WIN_DEFAULT		6
#define MV_PEX_WIN_EXP_ROM		7
#define PEX_GRNERAL_WIN_NUM	    	6
#define PEX_MAX_TARGET_WIN		8


#define PEX_MAX_BARS			3
#define PEX_INTER_REGS_BAR		0
#define PEX_DRAM_BAR			1
#define PEX_DEVICE_BAR			2

/*************************************/
/* PCI Express BAR Control Registers */
/*************************************/
#define PEX_BAR_CTRL_REG(pexIf, bar)    ((PEX_IF_BASE(pexIf)) + 0x1800 + (bar * 4))
#define PEX_EXP_ROM_BAR_CTRL_REG(pexIf)	((PEX_IF_BASE(pexIf)) + 0x180C)
/************************************************/
/* PCI Express Address Window Control Registers */
/************************************************/
#define PEX_WIN_REG_OFFS(winNum)    ((winNum >= 5) ? 0x60 : (winNum * 0x10))

#define PEX_WIN_CTRL_REG(pexIf,winNum)       \
    ((PEX_IF_BASE(pexIf)) + 0x1820 + PEX_WIN_REG_OFFS(winNum))

#define PEX_WIN_BASE_REG(pexIf,winNum)       \
    ((PEX_IF_BASE(pexIf)) + 0x1824 + PEX_WIN_REG_OFFS(winNum))

#define PEX_WIN_REMAP_REG(pexIf,winNum)      \
    ((PEX_IF_BASE(pexIf)) + 0x182C + PEX_WIN_REG_OFFS(winNum))

#define PEX_WIN_REMAP_HIGH_REG(pexIf,winNum) \
    ((winNum >= 4) ? ((PEX_IF_BASE(pexIf)) + 0x1870 + ((winNum - 4) * 0x20)) : 0)

#define PEX_WIN_DEFAULT_CTRL_REG(pexIf)         ((PEX_IF_BASE(pexIf)) + 0x18B0)
#define PEX_WIN_EXP_ROM_CTRL_REG(pexIf)         ((PEX_IF_BASE(pexIf)) + 0x18C0)
#define PEX_WIN_EXP_ROM_REMAP_REG(pexIf)        ((PEX_IF_BASE(pexIf)) + 0x18C4)

/* PCI Express Window Control Register */
/* PEX_WIN_CTRL_REG (PXWCR) */
#define	PXWCR_WIN_EN_OFFS				0
#define	PXWCR_WIN_EN					BIT0 /* Window Enable.*/
#define	PXWCR_WIN_BAR_MAP_OFFS			1    /* Mapping to BAR.*/
#define	PXWCR_WIN_BAR_MAP_MASK			(1 << PXWCR_WIN_BAR_MAP_OFFS)
#define	PXWCR_WIN_BAR_MAP_BAR(barNum)	((barNum-1) << PXWCR_WIN_BAR_MAP_OFFS)
#define PXWCR_SLV_WR_SPLT_OFFS          2
#define PXWCR_SLV_WR_SPLT_MASK          (1 << PXWCR_SLV_WR_SPLT_OFFS)
#define PXWCR_SLV_WR_SPLT_128B          (0 << PXWCR_SLV_WR_SPLT_OFFS)
#define PXWCR_SLV_WR_SPLT_32B           (1 << PXWCR_SLV_WR_SPLT_OFFS)
#define	PXWCR_TARGET_OFFS				4  /*Unit ID */
#define	PXWCR_TARGET_MASK				(0xf << PXWCR_TARGET_OFFS)
#define	PXWCR_ATTRIB_OFFS				8  /* target attributes */
#define	PXWCR_ATTRIB_MASK				(0xff << PXWCR_ATTRIB_OFFS)
#define	PXWCR_SIZE_OFFS					16 /* size */
#define	PXWCR_SIZE_MASK					(0xffff << PXWCR_SIZE_OFFS)
#define	PXWCR_SIZE_ALIGNMENT			0x10000

/* PCI Express Window Base Register */
/* PEX_WIN_BASE_REG (PXWBR)*/
#define PXWBR_BASE_OFFS					16 /* address[31:16] */
#define PXWBR_BASE_MASK					(0xffff << PXWBR_BASE_OFFS)
#define PXWBR_BASE_ALIGNMENT			0x10000

/* PCI Express Window Remap Register */
/* PEX_WIN_REMAP_REG (PXWRR)*/
#define PXWRR_REMAP_EN					BIT0
#define PXWRR_REMAP_OFFS				16
#define PXWRR_REMAP_MASK				(0xffff << PXWRR_REMAP_OFFS)
#define PXWRR_REMAP_ALIGNMENT			0x10000

/* PCI Express Window Remap (High) Register */
/* PEX_WIN_REMAP_HIGH_REG (PXWRHR)*/
#define PXWRHR_REMAP_HIGH_OFFS			0
#define PXWRHR_REMAP_HIGH_MASK			(0xffffffff << PXWRHR_REMAP_HIGH_OFFS)

/* PCI Express Default Window Control Register */
/* PEX_WIN_DEFAULT_CTRL_REG (PXWDCR) */
#define PXWDCR_SLV_WR_SPLT_CNT_OFFS     2
#define PXWDCR_SLV_WR_SPLT_CNT_MASK     (1 << PXWDCR_SLV_WR_SPLT_CNT_OFFS)
#define PXWDCR_SLV_WR_SPLT_CNT_128B     (0 << PXWDCR_SLV_WR_SPLT_CNT_OFFS)
#define PXWDCR_SLV_WR_SPLT_CNT_32B      (1 << PXWDCR_SLV_WR_SPLT_CNT_OFFS)
#define	PXWDCR_TARGET_OFFS				4  /*Unit ID */
#define	PXWDCR_TARGET_MASK				(0xf << PXWDCR_TARGET_OFFS)
#define	PXWDCR_ATTRIB_OFFS				8  /* target attributes */
#define	PXWDCR_ATTRIB_MASK				(0xff << PXWDCR_ATTRIB_OFFS)

/* PCI Express Expansion ROM Window Control Register */
/* PEX_WIN_EXP_ROM_CTRL_REG (PXWERCR)*/
#define	PXWERCR_TARGET_OFFS				4  /*Unit ID */
#define	PXWERCR_TARGET_MASK				(0xf << PXWERCR_TARGET_OFFS)
#define	PXWERCR_ATTRIB_OFFS				8  /* target attributes */
#define	PXWERCR_ATTRIB_MASK				(0xff << PXWERCR_ATTRIB_OFFS)

/* PCI Express Expansion ROM Window Remap Register */
/* PEX_WIN_EXP_ROM_REMAP_REG (PXWERRR)*/
#define PXWERRR_REMAP_EN				BIT0
#define PXWERRR_REMAP_OFFS				16
#define PXWERRR_REMAP_MASK				(0xffff << PXWERRR_REMAP_OFFS)
#define PXWERRR_REMAP_ALIGNMENT			0x10000


/* PCI Express BAR Control Register */
/* PEX_BAR_CTRL_REG (PXBCR) */
#define PXBCR_BAR_EN				BIT0
#define PXBCR_BAR_SIZE_OFFS			16
#define PXBCR_BAR_SIZE_MASK			(0xffff << PXBCR_BAR_SIZE_OFFS)
#define PXBCR_BAR_SIZE_ALIGNMENT	0x10000

/* PCI Express Expansion ROM BAR Control Register */
/* PEX_EXP_ROM_BAR_CTRL_REG (PXERBCR) */
#define PXERBCR_EXPROM_EN			BIT0
#define PXERBCR_EXPROMSZ_OFFS		19
#define PXERBCR_EXPROMSZ_MASK		(0x7 << PXERBCR_EXPROMSZ_OFFS)
#define PXERBCR_EXPROMSZ_512KB		(0x0 << PXERBCR_EXPROMSZ_OFFS)
#define PXERBCR_EXPROMSZ_1024KB		(0x1 << PXERBCR_EXPROMSZ_OFFS)
#define PXERBCR_EXPROMSZ_2048KB		(0x3 << PXERBCR_EXPROMSZ_OFFS)
#define PXERBCR_EXPROMSZ_4096KB		(0x7 << PXERBCR_EXPROMSZ_OFFS)


/* PCI Express BAR0 Internal Register*/
/* PEX BAR0_INTER_REG (PXBIR)*/
#define PXBIR_IOSPACE			BIT0	/* Memory Space Indicator */
#define PXBIR_TYPE_OFFS			1	    /* BAR Type/Init Val. */ 
#define PXBIR_TYPE_MASK			(0x3 << PXBIR_TYPE_OFFS)
#define PXBIR_TYPE_32BIT_ADDR	(0x0 << PXBIR_TYPE_OFFS)
#define PXBIR_TYPE_64BIT_ADDR	(0x2 << PXBIR_TYPE_OFFS)
#define PXBIR_PREFETCH_EN		BIT3 	/* Prefetch Enable */
#define PXBIR_BASE_OFFS		    20		/* Base address. Address bits [31:20] */
#define PXBIR_BASE_MASK		    (0xfff << PXBIR_BASE_OFFS)
#define PXBIR_BASE_ALIGNMET	    (1 << PXBIR_BASE_OFFS)

/* PCI Express BAR1 Register and PCI Express BAR2 Register*/
/* PEX BAR1_REG (PXBR) and PEX BAR2_REG (PXBR) */
#define PXBR_IOSPACE			PXBIR_IOSPACE
#define PXBR_TYPE_OFFS			PXBIR_TYPE_OFFS
#define PXBR_TYPE_MASK			PXBIR_TYPE_MASK
#define PXBR_TYPE_32BIT_ADDR	PXBIR_TYPE_32BIT_ADDR
#define PXBR_TYPE_64BIT_ADDR	PXBIR_TYPE_64BIT_ADDR
#define PXBR_PREFETCH_EN		PXBIR_PREFETCH_EN
#define PXBR_BASE_OFFS		    16		/* Base address. Address bits [31:16] */
#define PXBR_BASE_MASK		    (0xffff << PXBR_BASE_OFFS)
#define PXBR_BASE_ALIGNMET	    (1 << PXBR_BASE_OFFS)

/* This macro defines default BAR attributes */
#define PEX_BAR_DEFAULT_ATTRIB	0xc /* Memory - Prefetch - 64 bit address */

/* PEX Bar attributes */
typedef struct _mvPexBar
{
	MV_ADDR_WIN   addrWin;    /* An address window*/
	MV_BOOL       enable;     /* Address decode window is enabled/disabled    */
}MV_PEX_BAR;

/* PEX Remap Window attributes */
typedef struct _mvPexRemapWin
{
	MV_ADDR_WIN   addrWin;    /* An address window*/
	MV_BOOL       enable;     /* Address decode window is enabled/disabled    */

}MV_PEX_REMAP_WIN;

/* PEX Window attributes */
typedef struct _mvPexDecWin
{
    MV_TARGET       target;         /* Target for addr decode window        */
    MV_ADDR_WIN     addrWin;        /* Address window of target             */
    MV_BOOL     	enable;         /* Window enable/disable                */
    MV_U32          targetBar;
    MV_BOOL         slvWrSpltCnt;
} MV_PEX_DEC_WIN;


/* Global Functions prototypes */
/* mvPexHalInit - Initialize PEX interfaces*/
MV_STATUS mvPexInit(MV_U32 pexIf, MV_PEX_TYPE pexType);


/* mvPexTargetWinSet - Set PEX to peripheral target address window BAR*/
MV_STATUS mvPexTargetWinSet(MV_U32 pexIf, 
			    MV_U32 winNum, MV_PEX_DEC_WIN *pPexAddrDecWin);

/* mvPexTargetWinGet - Get PEX to peripheral target address window*/
MV_STATUS mvPexTargetWinGet(MV_U32 pexIf, MV_U32 winNum, 
			    MV_PEX_DEC_WIN *pPexAddrDecWin);
/* mvPexTargetWinEnable - Enable/disable a PEX BAR window*/
MV_STATUS mvPexTargetWinEnable(MV_U32 pexIf,
			       MV_U32 winNum, MV_BOOL enable);

/* mvPexTargetWinRemap - Set PEX to target address window remap.*/
MV_STATUS mvPexTargetWinRemap(MV_U32 pexIf, MV_U32 winNum, 
                           MV_PEX_REMAP_WIN *pAddrWin);

/* mvPexTargetWinRemapEnable -enable\disable a PEX Window remap.*/
MV_STATUS mvPexTargetWinRemapEnable(MV_U32 pexIf, MV_U32 winNum, 
                           MV_BOOL enable);

/* mvPexBarSet - Set PEX bar address and size */
MV_STATUS mvPexBarSet(MV_U32 pexIf,
		      MV_U32 barNum,
		      MV_PEX_BAR *pAddrWin);

/* mvPexBarGet - Get PEX bar address and size */
MV_STATUS mvPexBarGet(MV_U32 pexIf,
			MV_U32 barNum,
		      MV_PEX_BAR *pAddrWin);

/* mvPexBarEnable - enable\disable a PEX bar*/
MV_STATUS mvPexBarEnable(MV_U32 pexIf, MV_U32 barNum, MV_BOOL enable);

/* mvPexAddrDecShow - Display address decode windows attributes */
MV_VOID mvPexAddrDecShow(MV_VOID);

#endif
