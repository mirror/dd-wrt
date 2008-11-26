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

#ifndef __INCmv88F1X81SysHwConfigh
#define __INCmv88F1X81SysHwConfigh



/* 
 *  System memory mapping 
 */

/* SDRAM: actual mapping is auto detected */
#define SDRAM_CS0_BASE  0x00000000
#define SDRAM_CS0_SIZE  _256M

#define SDRAM_CS1_BASE  0x10000000
#define SDRAM_CS1_SIZE  _256M

#define SDRAM_CS2_BASE  0x20000000
#define SDRAM_CS2_SIZE  _256M

#define SDRAM_CS3_BASE  0x30000000
#define SDRAM_CS3_SIZE  _256M

/* PEX */
#define PEX0_MEM_BASE 0xe0000000
#define PEX0_MEM_SIZE _128M

#define PEX0_IO_BASE 0xf2000000
#define PEX0_IO_SIZE _1M



/* PEX Work arround */
/* the target we will use for the workarround */
#define PEX_CONFIG_RW_WA_TARGET PEX0_MEM
/*a flag that indicates if we are going to use the 
size and base of the target we using for the workarround
window */
#define PEX_CONFIG_RW_WA_USE_ORIGINAL_WIN_VALUES 1
/* if the above flag is 0 then the following values
will be used for the workarround window base and size,
otherwise the following defines will be ignored */
#define PEX_CONFIG_RW_WA_BASE 0x50000000
#define PEX_CONFIG_RW_WA_SIZE _16M


#define PEX1_MEM_BASE 0xe8000000
#define PEX1_MEM_SIZE _128M

#define PEX1_IO_BASE 0xf2100000
#define PEX1_IO_SIZE _1M



#define FLASH_CS_BASE 0xF4000000
#define FLASH_CS_SIZE _16M

#define DEVICE_CS0_BASE	FLASH_CS_BASE
#define DEVICE_CS0_SIZE	FLASH_CS_SIZE

#define DEVICE_CS1_BASE	0xFFF80000
#define DEVICE_CS1_SIZE _512K


/* Internal registers: size is defined in Controllerenvironment */
#define INTER_REGS_BASE	0xF1000000


#define PCI_IF0_MEM0_BASE 	PEX0_MEM_BASE
#define PCI_IF0_MEM0_SIZE 	PEX0_MEM_SIZE
#define PCI_IF0_IO_BASE 	PEX0_IO_BASE
#define PCI_IF0_IO_SIZE 	PEX0_IO_SIZE

#define PCI_IF1_MEM0_BASE 	PEX1_MEM_BASE
#define PCI_IF1_MEM0_SIZE 	PEX1_MEM_SIZE
#define PCI_IF1_IO_BASE 	PEX1_IO_BASE
#define PCI_IF1_IO_SIZE 	PEX1_IO_SIZE


/* DRAM detection stuff */
#define MV_DRAM_AUTO_SIZE


/* Board clock detection */
#define TCLK_AUTO_DETECT    /* Use Tclk auto detection 		*/
#define SYSCLK_AUTO_DETECT	/* Use SysClk auto detection 	*/
#define PCLCK_AUTO_DETECT  /* Use PClk auto detection */

/* PEX-PCI\PCI-PCI Bridge*/
#define PCI0_IF_PTP		1		/* Bridge exist on pciIf0*/
#define PCI1_IF_PTP		0		/* no Bridge on pciIf1*/



#define MEM_TABLE	{	\
  { PCI_IF0_MEM0_BASE,   PCI_IF0_MEM0_BASE,   PCI_IF0_MEM0_SIZE,  MT_DEVICE }	\
 ,{ PCI_IF1_MEM0_BASE,   PCI_IF1_MEM0_BASE,   PCI_IF1_MEM0_SIZE,  MT_DEVICE }	\
 ,{ INTER_REGS_BASE, INTER_REGS_BASE, SZ_1M,  	     MT_DEVICE }				\
 ,{ PCI_IF0_IO_BASE,   PCI_IF0_IO_BASE,   PCI_IF0_IO_SIZE,  MT_DEVICE }			\
 ,{ PCI_IF1_IO_BASE,   PCI_IF1_IO_BASE,   PCI_IF1_IO_SIZE,  MT_DEVICE }			\
}


#endif /* __INCmv88F1X81SysHwConfigh */
