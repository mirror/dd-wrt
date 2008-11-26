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

#define CONFIG_MARVELL		1

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
/* Soc supporetd Units definitions		*/
/****************************************/
#ifdef CONFIG_MV_INCLUDE_PCI
#define MV_INCLUDE_PCI
#endif
#ifdef CONFIG_MV_INCLUDE_PEX
#define MV_INCLUDE_PEX
#endif
#ifdef CONFIG_MV_INCLUDE_TWSI
#define MV_INCLUDE_TWSI
#endif
#ifdef CONFIG_MV_INCLUDE_IDMA
#define MV_INCLUDE_IDMA
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
#ifdef CONFIG_MV_INCLUDE_INTEG_MFLASH
#define MV_INCLUDE_INTEG_MFLASH
#endif
#ifdef CONFIG_MV_INCLUDE_SPI
#define MV_INCLUDE_SPI
#endif
#ifdef CONFIG_MV_INCLUDE_MFLASH_MTD
#define MV_INCLUDE_MFLASH_MTD
#endif
#ifdef CONFIG_MV_INCLUDE_SFLASH_MTD
#define MV_INCLUDE_SFLASH_MTD
#endif
#ifdef CONFIG_MV_INCLUDE_AUDIO
#define MV_INCLUDE_AUDIO
#endif
#ifdef CONFIG_MV_INCLUDE_SDIO
#define MV_INCLUDE_SDIO
#endif


/* NAND flash stuff */
#ifdef CONFIG_MV_NAND_BOOT
#define MV_NAND_BOOT
#endif
#ifdef CONFIG_MV_NAND
#define MV_NAND
#endif

#ifdef CONFIG_MV88F1181
#include "mv88F1X81SysHwConfig.h"
#elif defined (CONFIG_MV88F1281)
#include "mv88F1281SysHwConfig.h"
#elif defined (CONFIG_MV88F5182)
#include "mv88F5182SysHwConfig.h"
#elif defined (CONFIG_MV88F5082)
#include "mv88F5082SysHwConfig.h"
#elif defined (CONFIG_MV88F5181L)
#include "mv88F5181LSysHwConfig.h"
#elif defined (CONFIG_MV88W8660)
#include "mv88w8660SysHwConfig.h"
#elif defined (CONFIG_MV88F5181)
#include "mv88F5X81SysHwConfig.h"
#elif defined (CONFIG_MV88F5180N)
#include "mv88F5180NSysHwConfig.h"
#elif defined (CONFIG_MV88F6082)
#include "mv88F6082SysHwConfig.h"
#elif defined(CONFIG_MV88F6183)
#include "mv88F6183SysHwConfig.h"
#else
#error "Not mvSysHwconfig was selected"
#endif

/****************************************************************/
/************* General    configuration ********************/
/****************************************************************/

/* Disable the DEVICE BAR in the PEX */
#define MV_DISABLE_PEX_DEVICE_BAR

/* Allow the usage of early printings during initialization */
#define MV_INCLUDE_EARLY_PRINTK

/****************************************************************/
/************* CESA configuration ********************/
/****************************************************************/

#ifdef MV_INCLUDE_CESA

#define MV_CESA_MAX_CHAN               1

/* Use 2K of SRAM */
#define MV_CESA_MAX_BUF_SIZE           1600

/* Use 4K of SRAM */
/*
#define MV_CESA_MAX_BUF_SIZE           3200
*/
#endif /* MV_INCLUDE_CESA */

#if defined(CONFIG_MV_INCLUDE_GIG_ETH)

#ifndef ETH_PORT0_IRQ_NUM
 #define ETH_PORT0_IRQ_NUM 	21
#endif

/* MTU */
#ifndef CONFIG_ETH_0_MTU
# define CONFIG_ETH_0_MTU	1500
#endif
#ifndef CONFIG_ETH_1_MTU
# define CONFIG_ETH_1_MTU	1500
#endif
#ifndef CONFIG_ETH_2_MTU
# define CONFIG_ETH_2_MTU	1500
#endif

/* MAC */
#ifndef CONFIG_ETH_0_MACADDR
# define CONFIG_ETH_0_MACADDR	"000000000051"
#endif
#ifndef CONFIG_ETH_1_MACADDR
# define CONFIG_ETH_1_MACADDR	"000000000052"
#endif
#ifndef CONFIG_ETH_2_MACADDR
# define CONFIG_ETH_2_MACADDR	"000000000053"
#endif

#ifdef CONFIG_EGIGA_STATIS
#define EGIGA_STATISTICS
#define MV_FP_STATISTICS
#else
#undef EGIGA_STATISTICS
#undef MV_FP_STATISTICS
#endif

/* Multi-queue support */
#ifdef CONFIG_ETH_MULTI_Q
#define INCLUDE_MULTI_QUEUE 1
#else
#undef INCLUDE_MULTI_QUEUE
#endif

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

#define MV_ETH_TX_Q_NUM		1

/* port's default queueus */
#define ETH_DEF_TXQ         0
#define ETH_DEF_RXQ         0 

#endif /* CONFIG_MV_INCLUDE_GIG_ETH */

#ifdef CONFIG_MV_INCLUDE_GIG_ETH
/****************************************************************/
/************* Ethernet driver configuration ********************/
/****************************************************************/

/* interrupt coalescing setting */
#define ETH_TX_COAL    200
#if defined CONFIG_MV88F6082
#define ETH_RX_COAL    1000
#else
#define ETH_RX_COAL    200
#endif

#define ETH_INCLUDE_TSO

/* Perion [msec] for periodic TX timer. 
 * This timer will enable TX queue to prevent stack of last packet, and
 * will care tx_done functionality.
 */
#define ETH_TX_TIMER_PERIOD     10

/* Checksum offloading */
#define TX_CSUM_OFFLOAD
#define RX_CSUM_OFFLOAD

#endif /* CONFIG_MV_INCLUDE_GIG_ETH */

#define MV_ETH_EXTRA_TX_DESCR	    20 /* a few extra Tx descriptors, because we stop the interface for Tx in advance */

#ifdef CONFIG_MV_GATEWAY

#ifdef INCLUDE_MULTI_QUEUE
#define MV_ETH_RX_Q_NUM		    4
#define ETH_NUM_OF_RX_DESCR     64
#define ETH_NUM_OF_TX_DESCR     2000
#define ETH_RX_QUEUE_QUOTA	    32   /* quota per Rx Q */
#else
#define MV_ETH_RX_Q_NUM		    1
#define ETH_NUM_OF_RX_DESCR     128
#define ETH_NUM_OF_TX_DESCR     (ETH_NUM_OF_RX_DESCR*2 + MV_ETH_EXTRA_TX_DESCR)
#endif /* INCLUDE_MULTI_QUEUE */

#elif defined(CONFIG_MV88F6082)
#ifdef INCLUDE_MULTI_QUEUE
#define MV_ETH_RX_Q_NUM             8
#define ETH_NUM_OF_RX_DESCR     64
#define ETH_NUM_OF_TX_DESCR     (ETH_NUM_OF_RX_DESCR*MV_ETH_RX_Q_NUM + MV_ETH_EXTRA_TX_DESCR)
#define ETH_RX_QUEUE_QUOTA          32   /* quota per Rx Q */
#else
#define MV_ETH_RX_Q_NUM             1
#define ETH_NUM_OF_RX_DESCR     64
#define ETH_NUM_OF_TX_DESCR     (ETH_NUM_OF_RX_DESCR*2 + MV_ETH_EXTRA_TX_DESCR)
#endif /* INCLUDE_MULTI_QUEUE */

#else /* CONFIG_MV_ETHERNET */

#ifdef INCLUDE_MULTI_QUEUE
 #define MV_ETH_RX_Q_NUM		8
 #define ETH_NUM_OF_RX_DESCR     64
 #define ETH_NUM_OF_TX_DESCR     (ETH_NUM_OF_RX_DESCR*MV_ETH_RX_Q_NUM + MV_ETH_EXTRA_TX_DESCR)
 #define ETH_RX_QUEUE_QUOTA	    32   /* quota per Rx Q */
#else
 #define MV_ETH_RX_Q_NUM		    1
 #if defined CONFIG_MV88F6082
  #define ETH_NUM_OF_RX_DESCR     64
 #else 
  #define ETH_NUM_OF_RX_DESCR     128
 #endif /* CONFIG_MV88F6082 */
 #define ETH_NUM_OF_TX_DESCR     (ETH_NUM_OF_RX_DESCR*2 + MV_ETH_EXTRA_TX_DESCR)
#endif /* INCLUDE_MULTI_QUEUE */

#endif /* CONFIG_MV_GATEWAY */
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

/* Following the pre-configuration registers default values restored after    */
/* auto-detection is done                                                     */
#define DRAM_BUF_REG0_DV    0           /* GPIO Interrupt Level Mask Reg      */
#define DRAM_BUF_REG1_DV	0           /* ARM Timer 1 reload register        */
#define DRAM_BUF_REG2_DV    0           /* AHB to MBUS Bridge int Mask Reg    */
#define DRAM_BUF_REG3_DV	0           /* ARM Watchdog Timer Register        */
#define DRAM_BUF_REG4_DV	0           /* Host to ARM Doorbel Mask Register  */
#define DRAM_BUF_REG5_DV	0           /* ARM To Host Doorbel Mask Register  */
#define DRAM_BUF_REG6_DV	0           /* PCI Exp Uncorrectable Err Mask Reg */
#define DRAM_BUF_REG7_DV	0           /* PCI Exp Correctable Err Mask Reg   */
#define DRAM_BUF_REG8_DV	0           /* PCI Express interrupt Mask Reg     */
#define DRAM_BUF_REG9_DV	0           /* PCI Express Spare Register         */
#define DRAM_BUF_REG10_DV	0x012C0004  /* PCI Exp Acknowledge Timers (x4) Reg*/
 

#endif /* __INCmvSysHwConfigh */

