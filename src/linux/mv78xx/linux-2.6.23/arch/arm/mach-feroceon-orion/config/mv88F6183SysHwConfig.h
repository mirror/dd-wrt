/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

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

*******************************************************************************/
/*******************************************************************************
* mvSysHwCfg.h - Marvell system HW configuration file
*
* DESCRIPTION:
*       None.
*
* DEPENDENCIES:
*       None.
*
*******************************************************************************/

#ifndef __INCmv88F6183SysHwConfigh
#define __INCmv88F6183SysHwConfigh


/* PEX Work arround */
/* the target we will use for the workarround */
#define PEX_CONFIG_RW_WA_TARGET PEX0_MEM
/*a flag that indicates if we are going to use the 
size and base of the target we using for the workarround
window */
#define PEX_CONFIG_RW_WA_USE_ORIGINAL_WIN_VALUES 0
/* if the above flag is 0 then the following values
will be used for the workarround window base and size,
otherwise the following defines will be ignored */
#define PEX_CONFIG_RW_WA_BASE 0xF0000000
#define PEX_CONFIG_RW_WA_SIZE _16M

 
/* Internal registers: size is defined in Controllerenvironment */
#define INTER_REGS_BASE	0xF1000000


/* DRAM detection stuff */
#define MV_DRAM_AUTO_SIZE

#define PCI_ARBITER_CTRL    /* Use/unuse the Marvell integrated PCI arbiter	*/
#undef	PCI_ARBITER_BOARD	/* Use/unuse the PCI arbiter on board			*/

/* Check macro validity */
#if defined(PCI_ARBITER_CTRL) && defined (PCI_ARBITER_BOARD)
	#error "Please select either integrated PCI arbiter or board arbiter"
#endif

/* Board clock detection */
#define TCLK_AUTO_DETECT    /* Use Tclk auto detection 		*/
#define SYSCLK_AUTO_DETECT	/* Use SysClk auto detection 	*/
#define PCLCK_AUTO_DETECT  /* Use PClk auto detection */


/* PEX-PCI\PCI-PCI Bridge*/
#define PCI0_IF_PTP		0		/* Bridge exist on pciIf0*/
#define PCI1_IF_PTP		0		/* Bridge exist on pciIf1*/

#endif /* __INCmv88F6183SysHwConfigh */
