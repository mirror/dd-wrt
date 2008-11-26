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


#ifndef __INCSysPCIH
#define __INCSysPCIH

#include "ctrlEnv/sys/mvCpuIf.h"
#include "pci/mvPci.h"
#include "ctrlEnv/mvCtrlEnvLib.h"
#include "ctrlEnv/mvCtrlEnvAddrDec.h"

#define PCI_MAX_PROT_WIN	6

/* 4KB granularity */
#define MINIMUM_WINDOW_SIZE     0x1000
#define MINIMUM_BAR_SIZE        0x1000
#define MINIMUM_BAR_SIZE_MASK	0xFFFFF000
#define BAR_SIZE_OFFS			12
#define BAR_SIZE_MASK			(0xFFFFF << BAR_SIZE_OFFS)

#define PCI_IO_WIN_NUM          1   /* Number of PCI_IO windows  */
#define PCI_MEM_WIN_NUM         4   /* Number of PCI_MEM windows */

#ifndef MV_ASMLANGUAGE
#include "ctrlEnv/mvCtrlEnvLib.h"
typedef enum _mvPCIBars
{
    PCI_BAR_TBL_TERM = -1, /* none valid bar, used as bars list terminator */
#if defined(MV_INCLUDE_SDRAM_CS0)
    CS0_BAR,      		/* SDRAM chip select 0 bar*/
#endif	
#if defined(MV_INCLUDE_SDRAM_CS1)
    CS1_BAR,      		/* SDRAM chip select 1 bar*/
#endif	      		
#if defined(MV_INCLUDE_SDRAM_CS2)
    CS2_BAR,      		/* SDRAM chip select 2 bar*/
#endif
#if defined(MV_INCLUDE_SDRAM_CS3)
    CS3_BAR,      		/* SDRAM chip select 3 bar*/
#endif
#if defined(MV_INCLUDE_DEVICE_CS0)          		
    DEVCS0_BAR,     	/* Device chip select 0 bar*/
#endif
#if defined(MV_INCLUDE_DEVICE_CS1)          		
    DEVCS1_BAR,     	/* Device chip select 1 bar*/
#endif
#if defined(MV_INCLUDE_DEVICE_CS2)          		
    DEVCS2_BAR,     	/* Device chip select 2 bar*/
#endif
    BOOTCS_BAR,      	/* Boot device chip select bar*/
    MEM_INTER_REGS_BAR, /* Memory Mapped Internal bar */
    IO_INTER_REGS_BAR,  /* IO Mapped Internal bar */
    P2P_MEM0,      		/* P2P memory 0 */
    P2P_IO,        		/* P2P IO */
	PCI_MAX_BARS

}MV_PCI_BAR;
#endif /* MV_ASMLANGUAGE */

#if defined(MV_INCLUDE_SDRAM_CS3)
#define MV_PCI_BAR_IS_DRAM_BAR(bar)   \
                            ((bar >= CS0_BAR) && (bar <= CS3_BAR))
#elif defined(MV_INCLUDE_SDRAM_CS2)
#define MV_PCI_BAR_IS_DRAM_BAR(bar)   \
                            ((bar >= CS0_BAR) && (bar <= CS2_BAR))
#elif defined(MV_INCLUDE_SDRAM_CS1)
#define MV_PCI_BAR_IS_DRAM_BAR(bar)   \
                            ((bar >= CS0_BAR) && (bar <= CS1_BAR))
#elif defined(MV_INCLUDE_SDRAM_CS0)
#define MV_PCI_BAR_IS_DRAM_BAR(bar)   \
                            ((bar == CS0_BAR))
#endif


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

/* This structure describes a PCI BAR. It is also refered as PCI target     */
/* window to keep consistency with other address decode units in the system */
typedef struct _mvPciBarWin 
{
    MV_ADDR_WIN     addrWin;    /* Address window       */
    MV_BOOL     	enable;     /* BAR enable/disable   */
}MV_PCI_BAR_WIN;

/* This structure describes PCI region attributes                           */
typedef struct _mvPciRegionAttr
{
    MV_PROT_RIGHT   access;         /* Access protection                    */
    MV_PROT_RIGHT   write;          /* Write protection                     */
    MV_SWAP_TYPE    swapType;       /* Data swap mode for that region       */
    MV_U32		    readMaxBurst;   /* Read max burst                       */
    MV_U32		    readBurst;      /* Read burst. Conventional PCI only    */
    MV_U32		    writeMaxBurst;  /* Write max burst                      */
    MV_BOOL         pciOrder;       /* Hardware support for PCI ordering    */
}MV_PCI_REGION_ATTR;

/* The PCI slave interface supports configurable access control.            */
/* It is possible to define up to six address ranges to different           */
/* configurations. This structure describes the PCI access region           */
typedef struct _mvPciProtWin
{
    MV_ADDR_WIN         addrWin;    /* An address window                    */
    MV_PCI_REGION_ATTR  attributes; /* Window attributes                    */
    MV_BOOL             enable;     /* Window enabled/disabled              */
}MV_PCI_PROT_WIN;

/* Global Functions prototypes */

/* mvPciInit - Initialize PCI interfaces*/
MV_STATUS mvPciInit(MV_U32 pciIf, MV_PCI_MOD pciIfmod);

/* mvPciTargetWinSet - Set PCI to peripheral target address window BAR*/
MV_STATUS mvPciTargetWinSet(MV_U32 pciIf, MV_PCI_BAR slaveTarget, 
                            MV_PCI_BAR_WIN *pAddrBarWin);

/* mvPciTargetWinGet - Get PCI to peripheral target address window*/
MV_STATUS mvPciTargetWinGet(MV_U32 pciIf, MV_PCI_BAR slaveTarget, 
                            MV_PCI_BAR_WIN *pAddrBarWin);

/* mvPciTargetWinEnable - Enable/disable a PCI BAR window*/
MV_STATUS mvPciTargetWinEnable(MV_U32 pciIf,MV_PCI_BAR slaveTarget, 
							   MV_BOOL enable);

/* mvPciProtWinSet - Set PCI protection access window*/
MV_STATUS mvPciProtWinSet(MV_U32 pciIf, MV_U32 winNum, 
                          MV_PCI_PROT_WIN *pProtWin);

/* mvPciProtWinGet - Get PCI protection access window*/
MV_STATUS mvPciProtWinGet(MV_U32 pciIf, 
						  MV_U32 winNum, 
                          MV_PCI_PROT_WIN *pProtWin);

/* mvPciProtWinEnable - Get PCI protection access window*/
MV_STATUS mvPciProtWinEnable(MV_U32 pciIf, MV_U32 winNum, MV_BOOL enable);

/* mvPciTargetRemap - Set PCI to target address window remap.*/
MV_STATUS mvPciTargetRemap(MV_U32 pciIf, MV_PCI_BAR slaveTarget, 
                           MV_ADDR_WIN *pAddrWin);

/* mvPciAddrDecShow - Display address decode windows attributes */
MV_VOID mvPciAddrDecShow(MV_VOID);

#endif
