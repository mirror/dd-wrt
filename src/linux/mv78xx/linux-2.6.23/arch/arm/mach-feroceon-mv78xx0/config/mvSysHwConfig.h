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

#ifndef __INCmvSysHwConfigh
#define __INCmvSysHwConfigh

#include "../../../../include/linux/autoconf.h"

#define CONFIG_MARVELL	1

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

#define _1G         0x40000000
#define _2G         0x80000000

/****************************************/
/* Soc supporeted Units definitions	*/
/****************************************/

#ifdef CONFIG_MV_INCLUDE_PEX
#define MV_INCLUDE_PEX
#endif
#ifdef CONFIG_MV_INCLUDE_TWSI
#define MV_INCLUDE_TWSI
#endif
#ifdef CONFIG_MV_INCLUDE_CESA
#define MV_INCLUDE_CESA
#endif
#ifdef CONFIG_MV_INCLUDE_GIG_ETH
#define MV_INCLUDE_GIG_ETH
#endif
#ifdef CONFIG_MV_INCLUDE_INTEG_SATA
#define MV_INCLUDE_INTEG_SATA
#define MV_INCLUDE_SATA
#endif
#ifdef CONFIG_MV_INCLUDE_USB
#define MV_INCLUDE_USB
#define MV_USB_VOLTAGE_FIX
#endif
#ifdef CONFIG_MV_INCLUDE_NAND
#define MV_INCLUDE_NAND
#endif
#ifdef CONFIG_MV_INCLUDE_TDM
#define MV_INCLUDE_TDM
#endif
#ifdef CONFIG_MV_INCLUDE_XOR
#define MV_INCLUDE_XOR
#endif
#ifdef CONFIG_MV_INCLUDE_TWSI
#define MV_INCLUDE_TWSI
#endif
#ifdef CONFIG_MV_INCLUDE_UART
#define MV_INCLUDE_UART
#endif
#ifdef CONFIG_MV_INCLUDE_SPI
#define MV_INCLUDE_SPI
#endif


/* NAND flash stuff */
#ifdef CONFIG_MV_NAND_BOOT
#define MV_NAND_BOOT
#endif
#ifdef CONFIG_MV_NAND
#define MV_NAND
#endif


/****************************************************************/
/************* General    configuration ********************/
/****************************************************************/

/* Enable Clock Power Control */
#define MV_INCLUDE_CLK_PWR_CNTRL

/* Disable the DEVICE BAR in the PEX */
#define MV_DISABLE_PEX_DEVICE_BAR

/* Allow the usage of early printings during initialization */
#define MV_INCLUDE_EARLY_PRINTK

/****************************************************************/
/************* CESA configuration ********************/
/****************************************************************/

#ifdef MV_INCLUDE_CESA

#define MV_CESA_MAX_CHAN               4

/* Use 2K of SRAM */
#define MV_CESA_MAX_BUF_SIZE           1600

#endif /* MV_INCLUDE_CESA */

#if defined(CONFIG_MV_INCLUDE_GIG_ETH)


#ifdef CONFIG_MV_ETH_STATS
#define ETH_STATISTICS
#else
#undef ETH_STATISTICS
#endif

#ifdef CONFIG_MV_NFP_STATS
#define MV_FP_STATISTICS
#else
#undef MV_FP_STATISTICS
#endif

/* Default configuration for SKB_REUSE: 0 - Disabled, 1 - Enabled */
#define MV_ETH_SKB_REUSE_DEFAULT    1

/* Default configuration for TX_EN workaround: 0 - Disabled, 1 - Enabled */
#define MV_ETH_TX_EN_DEFAULT        0

/* un-comment if you want to perform tx_done from within the poll function */
/* #define ETH_TX_DONE_ISR */

/* put descriptors in uncached memory */
/* #define ETH_DESCR_UNCACHED */

/* Descriptors location: DRAM/internal-SRAM */
#define ETH_DESCR_IN_SDRAM
#undef  ETH_DESCR_IN_SRAM    /* No integrated SRAM in 88Fxx81 devices */

#if defined(ETH_DESCR_IN_SRAM)
#if defined(ETH_DESCR_UNCACHED)
 #define ETH_DESCR_CONFIG_STR    "Uncached descriptors in integrated SRAM"
#else
 #define ETH_DESCR_CONFIG_STR    "Cached descriptors in integrated SRAM"
#endif
#elif defined(ETH_DESCR_IN_SDRAM)
#if defined(ETH_DESCR_UNCACHED)
 #define ETH_DESCR_CONFIG_STR    "Uncached descriptors in DRAM"
#else
 #define ETH_DESCR_CONFIG_STR    "Cached descriptors in DRAM"
#endif
#else 
 #error "Ethernet descriptors location undefined"
#endif /* ETH_DESCR_IN_SRAM or ETH_DESCR_IN_SDRAM*/

/* SW Sync-Barrier: not relevant for 88fxx81*/
/* Reasnable to define this macro when descriptors in SRAM and buffers in DRAM */
/* In RX the CPU theoretically might see himself as the descriptor owner,      */
/* although the buffer hadn't been written to DRAM yet. Performance cost.      */
/* #define INCLUDE_SYNC_BARR */

/* Buffers cache coherency method (buffers in DRAM) */
#ifndef MV_CACHE_COHER_SW
/* Taken from mvCommon.h */
/* Memory uncached, HW or SW cache coherency is not needed */
#define MV_UNCACHED             0   
/* Memory cached, HW cache coherency supported in WriteThrough mode */
#define MV_CACHE_COHER_HW_WT    1
/* Memory cached, HW cache coherency supported in WriteBack mode */
#define MV_CACHE_COHER_HW_WB    2
/* Memory cached, No HW cache coherency, Cache coherency must be in SW */
#define MV_CACHE_COHER_SW       3

#endif

/* DRAM cache coherency configuration */
#define MV_CACHE_COHERENCY  MV_CACHE_COHER_SW


#define ETHER_DRAM_COHER    MV_CACHE_COHER_SW   /* No HW coherency in 88Fxx81 devices */

#if (ETHER_DRAM_COHER == MV_CACHE_COHER_HW_WB)
 #define ETH_SDRAM_CONFIG_STR    "DRAM HW cache coherency (write-back)"
#elif (ETHER_DRAM_COHER == MV_CACHE_COHER_HW_WT)
 #define ETH_SDRAM_CONFIG_STR    "DRAM HW cache coherency (write-through)"
#elif (ETHER_DRAM_COHER == MV_CACHE_COHER_SW)
 #define ETH_SDRAM_CONFIG_STR    "DRAM SW cache-coherency"
#elif (ETHER_DRAM_COHER == MV_UNCACHED)
#   define ETH_SDRAM_CONFIG_STR  "DRAM uncached"
#else
 #error "Ethernet-DRAM undefined"
#endif /* ETHER_DRAM_COHER */


/****************************************************************/
/************* Ethernet driver configuration ********************/
/****************************************************************/

/* port's default queueus */
#define ETH_DEF_TXQ         0
#define ETH_DEF_RXQ         0 
                        
#define MV_ETH_RX_Q_NUM     CONFIG_MV_ETH_RX_Q_NUM
#define MV_ETH_TX_Q_NUM     CONFIG_MV_ETH_TX_Q_NUM

/* interrupt coalescing setting */
#define ETH_TX_COAL    		    200
#define ETH_RX_COAL    		    200

/* Checksum offloading */
#define TX_CSUM_OFFLOAD
#define RX_CSUM_OFFLOAD

/* a few extra Tx descriptors, because we stop the interface for Tx in advance */
#define MV_ETH_EXTRA_TX_DESCR	    20 

#if (MV_ETH_RX_Q_NUM > 1)
#define ETH_NUM_OF_RX_DESCR         64
#define ETH_RX_QUEUE_QUOTA	        32   /* quota per Rx Q */
#else
#define ETH_NUM_OF_RX_DESCR         128
#endif

#define ETH_NUM_OF_TX_DESCR         (ETH_NUM_OF_RX_DESCR*4 + MV_ETH_EXTRA_TX_DESCR)

#endif /* CONFIG_MV_INCLUDE_GIG_ETH */

/****************************************************************/
/*************** Telephony configuration ************************/
/****************************************************************/
#if defined(CONFIG_MV_TDM_LINEAR_MODE)
 #define MV_TDM_LINEAR_MODE
#elif defined(CONFIG_MV_TDM_ULAW_MODE)
 #define MV_TDM_ULAW_MODE
#endif



/* We use the following registers to store DRAM interface pre configuration   */
/* auto-detection results													  */
/* IMPORTANT: We are using mask register for that purpose. Before writing     */
/* to units mask register, make sure main maks register is set to disable     */
/* all interrupts.                                                            */
#define DRAM_BUF_REG0	0x1011c	/* sdram bank 0 size	        */  
#define DRAM_BUF_REG1	0x20318	/* sdram config			        */
#define DRAM_BUF_REG2   0x20114	/* sdram mode 			        */
#define DRAM_BUF_REG3	0x20320	/* dunit control low 	        */          
#define DRAM_BUF_REG4	0x20404	/* sdram address control        */
#define DRAM_BUF_REG5	0x2040c	/* sdram timing control low     */
#if defined(MV_INCLUDE_PEX)
#define DRAM_BUF_REG6	0x40108	/* sdram timing control high    */
#define DRAM_BUF_REG7	0x40114	/* sdram ODT control low        */
#define DRAM_BUF_REG8	0x41910	/* sdram ODT control high       */
#define DRAM_BUF_REG9	0x41a08	/* sdram Dunit ODT control      */
#define DRAM_BUF_REG10	0x41a30	/* sdram Extended Mode		    */
#elif defined(MV_INCLUDE_IDMA)
#define DRAM_BUF_REG6	0x60810	/* sdram timing control high    */
#define DRAM_BUF_REG7	0x60814	/* sdram ODT control low        */
#define DRAM_BUF_REG8	0x60818	/* sdram ODT control high       */
#define DRAM_BUF_REG9	0x6081c	/* sdram Dunit ODT control      */
#define DRAM_BUF_REG10	0x60820	/* sdram Extended Mode		    */
#endif
#define DRAM_BUF_REG11  0x6083c   /* sdram Ddr2 Timing Low        */
#define DRAM_BUF_REG12  0x60870   /* sdram Ddr2 Timing High       */
#define DRAM_BUF_REG13  0x60874   /* sdram Ddr2        */

/* Following the pre-configuration registers default values restored after    */
/* auto-detection is done                                                     */
#define DRAM_BUF_REG_DV 0

/* System Mapping */
#define SDRAM_CS0_BASE  0x00000000      /* Actual base set be auto detection */
#define SDRAM_CS1_BASE  0x00800000      /* Actual base set be auto detection */
#define SDRAM_CS2_BASE  0x01000000      /* Actual base set be auto detection */
#define SDRAM_CS3_BASE  0x01800000      /* Actual base set be auto detection */

#define DEVICE_CS0_BASE 	0xf8000000U
#define DEVICE_CS1_BASE 	0xfe000000U
#define DEVICE_CS2_BASE 	0xfc000000U
#define DEVICE_CS3_BASE 	0xfd000000U

/* System targers address window definitions (size)                */
#define SDRAM_CS0_SIZE  	0x00800000      /* Actual size set be auto detection */
#define SDRAM_CS1_SIZE  	0x00800000      /* Actual size set be auto detection */
#define SDRAM_CS2_SIZE  	0x00800000      /* Actual size set be auto detection */
#define SDRAM_CS3_SIZE  	0x00800000      /* Actual size set be auto detection */
#define DEVICE_CS0_SIZE  	_32M
#define DEVICE_CS1_SIZE  	_1M
#define DEVICE_CS2_SIZE  	_1M
#define DEVICE_CS3_SIZE 	_1M
#define BOOTDEV_CS_SIZE 	0x00080000  /* 512 K */
/* Chip Select Base Address */
#define BOOTDEV_CS_BASE 	0xfff00000U


#define PCIx_IO_SIZE     	_8M
#define PCIx_MEM0_SIZE  	_64M


#define PCI0_IO_SIZE     PCIx_IO_SIZE
#define PCI0_MEM0_SIZE   PCIx_MEM0_SIZE

#define PCI1_IO_SIZE    PCIx_IO_SIZE
#define PCI1_MEM0_SIZE   PCIx_MEM0_SIZE

#define PCI2_IO_SIZE     PCIx_IO_SIZE
#define PCI2_MEM0_SIZE   PCIx_MEM0_SIZE

#define PCI3_IO_SIZE     PCIx_IO_SIZE
#define PCI3_MEM0_SIZE   PCIx_MEM0_SIZE

#define PCI4_IO_SIZE     PCIx_IO_SIZE
#define PCI4_MEM0_SIZE   PCIx_MEM0_SIZE

#define PCI5_IO_SIZE     PCIx_IO_SIZE
#define PCI5_MEM0_SIZE   PCIx_MEM0_SIZE

#define PCI6_IO_SIZE     PCIx_IO_SIZE
#define PCI6_MEM0_SIZE   PCIx_MEM0_SIZE

#define PCI7_IO_SIZE     PCIx_IO_SIZE
#define PCI7_MEM0_SIZE   PCIx_MEM0_SIZE


#define CRYPTO_SIZE		_2M
#define CPU_IF_SIZE		0


#define PCI0_MEM0_BASE  0xc8000000U
#define PCI1_MEM0_BASE  0xcc000000U
#define PCI2_MEM0_BASE  0xd0000000U
#define PCI3_MEM0_BASE  0xd4000000U
#define PCI4_MEM0_BASE  0xd8000000U
#define PCI5_MEM0_BASE  0xdc000000U
#define PCI6_MEM0_BASE  0xe0000000U
#define PCI7_MEM0_BASE  0xe4000000U


#define PCI0_IO_BASE    0xf2000000U
#define PCI1_IO_BASE    0xf3000000U
#define PCI2_IO_BASE    0xf4000000U
#define PCI3_IO_BASE    0xf5000000U  
#define PCI4_IO_BASE    0xf6000000U
#define PCI5_IO_BASE    0xfa000000U
#define PCI6_IO_BASE    0xf8000000U
#define PCI7_IO_BASE    0xf9000000U

#define CRYPTO_ENG_BASE	0xf7000000U

#define SPI_CS_BASE 	0xfb000000U
#define SPI_CS_SIZE 	_8M

/* Internal registers: size is defined in Controllerenvironment */
#define INTER_REGS_BASE	0xF1000000

/* DRAM detection stuff */
#define MV_DRAM_AUTO_SIZE

/* Board clock detection */
#define TCLK_AUTO_DETECT    	/* Use Tclk auto detection   */
#define SYSCLK_AUTO_DETECT	/* Use SysClk auto detection */
#define PCLCK_AUTO_DETECT  	/* Use PClk auto detection   */
#define L2CLK_AUTO_DETECT 	/* Use L2Clk auto detection   */

/* PEX-PCI\PCI-PCI Bridge*/
#define PEX_HOST_BUS_NUM(pciIf) (pciIf)
#define PEX_HOST_DEV_NUM(pciIf) 0


#endif /* __INCmvSysHwConfigh */

