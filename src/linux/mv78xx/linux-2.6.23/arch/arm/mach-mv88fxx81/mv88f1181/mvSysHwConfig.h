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

#ifndef __INCmvSysHwConfigh
#define __INCmvSysHwConfigh

#ifndef MV_88F1181
#	define MV_88F1181
#endif

/* includes */
#define _1K         0x00000400
#define _4K         0x00001000
#define _8K         0x00002000
#define _16K        0x00004000
#define _32K        0x00008000
#define _64K        0x00010000
#define _128K       0x00020000
#define _256K       0x00040000
#define _512K       0x00080000

#define _1M         0x00100000
#define _2M         0x00200000
#define _4M         0x00400000
#define _8M         0x00800000
#define _16M        0x01000000
#define _32M        0x02000000
#define _64M        0x04000000
#define _128M       0x08000000
#define _256M       0x10000000
#define _512M       0x20000000

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


#define PEX1_MEM_BASE 0xe8000000
#define PEX1_MEM_SIZE _128M

#define PEX1_IO_BASE 0xf2100000
#define PEX1_IO_SIZE _1M

/* used for WA */
#define PEX_CONFIG_RW_WA_TARGET FLASH_CS
#define PEX_CONFIG_RW_WA_USE_ORIGINAL_WIN_VALUES 1
#define PEX_CONFIG_RW_WA_SIZE FLASH_CS_SIZE
#define PEX_CONFIG_RW_WA_BASE FLASH_CS_BASE

#define FLASH_CS_BASE 0xF0000000
#define FLASH_CS_SIZE _16M

#define LARGE_FLASH_BASE 	FLASH_CS_BASE

/* Internal registers: size is defined in Controllerenvironment */
#define INTER_REGS_BASE	0xF1000000

#define BOOTDEV_CS_BASE	0xff800000
#define BOOTDEV_CS_SIZE _8M

/* DRAM detection stuff */
#define MV_DRAM_AUTO_SIZE

#define DRAM_DETECT_FLAG_ADDR   0x03000000
#define DRAM_CONFIG_ROM_ADDR    0x03000004
 
/* We use the following registers to store DRAM interface pre configuration   */
/* auto-detection results						      */
/* IMPORTANT: We are using mask register for that purpose. Before writing     */
/* to units mask register, make sure main maks register is set to disable     */
/* all interrupts.                                                            */
#define DRAM_BUF_REG0   0x1011c /* sdram bank 0 size            */
#define DRAM_BUF_REG1   0x20318 /* sdram config                         */
#define DRAM_BUF_REG2   0x20114 /* sdram mode                           */
#define DRAM_BUF_REG3   0x20320 /* dunit control low            */
#define DRAM_BUF_REG4   0x20404 /* sdram address control        */
#define DRAM_BUF_REG5   0x2040c /* sdram timing control low     */
#define DRAM_BUF_REG6   0x40108 /* sdram timing control high    */
#define DRAM_BUF_REG7   0x40114 /* sdram ODT control low        */
#define DRAM_BUF_REG8   0x41910 /* sdram ODT control high       */
#define DRAM_BUF_REG9   0x41a08 /* sdram Dunit ODT control      */
#define DRAM_BUF_REG10  0x41a30 /* sdram Extended Mode              */
 
/* Following the pre-configuration registers default values restored after    */
/* auto-detection is done                                                     */
#define DRAM_BUF_REG0_DV    0           /* GPIO Interrupt Level Mask Reg      */
#define DRAM_BUF_REG1_DV        0           /* ARM Timer 1 reload register        */
#define DRAM_BUF_REG2_DV    0           /* AHB to MBUS Bridge int Mask Reg    */
#define DRAM_BUF_REG3_DV        0           /* ARM Watchdog Timer Register        */
#define DRAM_BUF_REG4_DV        0           /* Host to ARM Doorbel Mask Register  */
#define DRAM_BUF_REG5_DV        0           /* ARM To Host Doorbel Mask Register  */
#define DRAM_BUF_REG6_DV        0           /* PCI Exp Uncorrectable Err Mask Reg */
#define DRAM_BUF_REG7_DV        0           /* PCI Exp Correctable Err Mask Reg   */
#define DRAM_BUF_REG8_DV        0           /* PCI Express interrupt Mask Reg     */
#define DRAM_BUF_REG9_DV        0           /* PCI Express Spare Register         */
#define DRAM_BUF_REG10_DV       0x012C0004  /* PCI Exp Acknowledge Timers (x4) Reg*/

/* Pex\PCI stuff */
#define PEX0_HOST_BUS_NUM               0
#define PEX0_HOST_DEV_NUM               0
/* we have a bridge */
#define PEX1_HOST_BUS_NUM               2
#define PEX1_HOST_DEV_NUM               0
                                                                                                                             
/* no pci in MV_88F5181 */
#define PCI0_HOST_BUS_NUM               0
#define PCI0_HOST_DEV_NUM               0
#define PCI1_HOST_BUS_NUM               0
#define PCI1_HOST_DEV_NUM               0

/* Board clock detection */
#define TCLK_AUTO_DETECT         /* Use Tclk auto detection 	*/
#define SYSCLK_AUTO_DETECT	/* Use SysClk auto detection 	*/
#define PCLCK_AUTO_DETECT       /* Use PClk auto detection 	*/


/* Memory uncached, HW or SW cache coherency is not needed */
#define MV_UNCACHED             0   
/* Memory cached, HW cache coherency supported in WriteThrough mode */
#define MV_CACHE_COHER_HW_WT    1
/* Memory cached, HW cache coherency supported in WriteBack mode */
#define MV_CACHE_COHER_HW_WB    2
/* Memory cached, No HW cache coherency, Cache coherency must be in SW */
#define MV_CACHE_COHER_SW       3
			   
#endif /* __INCmvSysHwConfigh */
