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

#ifndef __INCmvCtrlEnvSpech
#define __INCmvCtrlEnvSpech

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* includes */
#include "mvDeviceId.h"

#define SOC_NAME_PREFIX	"MV"

#define INTER_REGS_SIZE             _1M

#define MV78XX0_ETH_MAX_PORTS	     2
#define MV78200_ETH_MAX_PORTS	     4

#define INTERNAL_REG_BASE_DEFAULT   0xD0000000


/* Define Register base adress for each unit */

#if defined(MV78200)
	#define MV_MAX_CPU			2
	#define CPU_IF_BASE(cpu)		(0x20000 + ((cpu) << 14))
#else
	#define MV_MAX_CPU			1
	#define CPU_IF_BASE(cpu)		0x20000
#endif

#define AHB_TO_MBUS_BASE          		CPU_IF_BASE

#define DRAM_BASE                   		0x00000
#define DEVICE_BUS_BASE             		0x10000

#if defined(MV_ASMLANGUAGE)
	#define PEX_IF_BASE(pexIf)      	0x40000
#else
    #define PEX_IF_BASE(pexIf)		(((pexIf) < 4) ? \
					 (0x40000 + (pexIf) * 0x4000) : \
					 (0x80000 + ((pexIf) - 4) * 0x4000))
#endif

#define MV_ETH_VERSION 		    	3
#define MV_ETH_MAX_RXQ              	8
#define MV_ETH_MAX_TXQ              	8
#define MV_ETH_MAX_PORTS		4

#define MV_ETH_REG_BASE(port)          	((((port) < 2)? 0x72000 : 0x32000) \
							+ (0x4000*(port & 1)))
#define INT_CTRL_BASE               (0x20200+(whoAmI()*0x4000))
#define CNTMR_BASE		    (0x20300+(whoAmI()*0x4000))
#define USB_REG_BASE(dev)           (0x50000 + (0x1000*(dev)))

#define IDMA_UNIT_BASE		    0x60800
#define XOR_UNIT_BASE(unit)         0x60900

#define WD_BASE                     0x10000
#define TWSI_SLAVE_BASE(chanNum)    (0x11000 + (0x100 * (chanNum)))

#define GPP_BASE                    0x10000
#define MV_UART_CHAN_BASE(chanNum)  (0x12000 + (0x100 * (chanNum)))
#define MPP_BASE                    0x10000
#define SATA_REG_BASE               0xA0000

/* MPP control registers offsets */
#define MPP_CONTROL_REG(groupNum)   (MPP_BASE + ((groupNum) * 4))

/* This define describes the maximum controller supported DRAM chip select  */
/* also known as banks                                                      */
#define MV_DRAM_MAX_CS      4
#define MV_INCLUDE_SDRAM_CS0
#define MV_INCLUDE_SDRAM_CS1
#define MV_INCLUDE_SDRAM_CS2
#define MV_INCLUDE_SDRAM_CS3 

#define MV_INCLUDE_DEVICE_CS0
#define MV_INCLUDE_DEVICE_CS1
#define MV_INCLUDE_DEVICE_CS2
#define MV_INCLUDE_DEVICE_CS3

/* This define describes maximum number of MPP groups.                      */
/* Each group describes an MPP register.                                    */
#define MV_MPP_MAX_GROUP    7

/* This define describes maximum of GPP groups supported by controller.     */
#define MV_GPP_MAX_GROUP    1

/* This define describes the maximum number of supported IDMA channels.     */
#define MV_IDMA_MAX_CHAN    4

/* This define describes the maximum number of available Timer/counters.    */
#define MV_CNTMR_MAX_COUNTER 4

#define MV_SATA_MAX_CHAN     2
/* This define describes the maximum number of available UART channels.     */
#define MV_UART_MAX_CHAN     2

/* This define describes the maximum number of available XOR channels.      */
#define MV_XOR_MAX_UNIT   		1	
#define MV_XOR_MAX_CHAN_PER_UNIT	2 
#define MV_XOR_MAX_CHAN     		2

/* This define describes the maximum number of supported PCI Interfaces 	*/
#define MV_PCI_MAX_IF		0
#define MV_PCI_START_IF		0

/* This define describes the maximum number of supported PEX Interfaces 	*/
#define MV_DISABLE_PEX_DEVICE_BAR 
#define MV_PEX_START_IF		MV_PCI_MAX_IF

/* This define describes the maximum number of supported PEX & PCI Interfaces 	*/
#define MV_PCI_IF_MAX_IF   	(MV_PEX_MAX_IF+MV_PCI_MAX_IF)
#define MV_PEX_MAX_IF	    8
#define PEX_DEFAULT_IF	    0
#define MV_INCLUDE_PEX1
#define MV_INCLUDE_PEX2
#define MV_INCLUDE_PEX3
#define PEX0_MEM	    PCI0_MEM0
#define PEX1_MEM	    PCI1_MEM0
#define PEX2_MEM	    PCI2_MEM0
#define PEX3_MEM	    PCI3_MEM0
/* This define describes the maximum number of available USB channels.      */
#define MV_USB_MAX_CHAN     3
#define MV_USB_VERSION      1

/* CESA version #2: One channel, 2KB SRAM, TDMA */
#define MV_CESA_VERSION		 	2
#define MV_CESA_REG_BASE                0x9D000
#define MV_CESA_TDMA_REG_BASE           0x90000
#define MV_CESA_SRAM_SIZE               2*1024 

/* main interrupt */
#define MV_TWSI_MAX_CHAN     2
#define TWSI_CPU_MAIN_INT_CAUSE_REG	CPU_INT_LOW_REG(whoAmI())
#define TWSI0_CPU_MAIN_INT_BIT	     	0x4
#define TWSI1_CPU_MAIN_INT_BIT	     	0x8


/* These macros help units to identify a target Xbar group */
#define MV_TARGET_IS_DRAM(target)   \
                            ((target >= SDRAM_CS0) && (target <= SDRAM_CS3))

#define MV_TARGET_IS_DEVICE(target) \
                            ((target >= DEVICE_CS0) && (target <= DEV_BOOCS))
                            
#define MV_TARGET_IS_PCI0(target)   \
                            ((target >= PCI0_IO) && (target <= PCI0_MEM0))
                            
#define MV_TARGET_IS_PCI1(target)   \
                            ((target >= PCI1_IO) && (target <= PCI1_MEM0))

#define MV_TARGET_IS_PCI2(target)   \
                            ((target >= PCI2_IO) && (target <= PCI2_MEM0))
                            
#define MV_TARGET_IS_PCI3(target)   \
                            ((target >= PCI3_IO) && (target <= PCI3_MEM0))

#define MV_TARGET_IS_PCI4(target)   \
                            ((target >= PCI4_IO) && (target <= PCI4_MEM0))

#define MV_TARGET_IS_PCI5(target)   \
                            ((target >= PCI5_IO) && (target <= PCI5_MEM0))
/*No conventional PCI*/
#define MV_TARGET_IS_PEX(target)    1 
#define MV_TARGET_IS_PCI(target)    0

#define MV_TARGET_IS_PCI_IO(target) \
          ((target == PCI0_IO) || (target == PCI1_IO)|| (target == PCI2_IO) || \
          (target == PCI3_IO) || (target == PCI4_IO)|| (target == PCI5_IO))

#define	MV_TARGET_IS_AS_BOOT(target) (0)

#define MV_CHANGE_BOOT_CS(target)	((target) == DEV_BOOCS)?\
					sampleAtResetTargetArray[((MV_REG_READ(CPU_RESET_SAMPLE_L_REG)\
						 & MSAR_BOOTDEV_MASK) >> MSAR_BOOTDEV_OFFS)]\
					:(target)
#if !defined(MV78XX0_Z0)
#define BOOT_TARGETS_NAME_ARRAY {       \
    DEV_BOOCS,          	\
    SPI_CS,          	\
    TBL_TERM,         	\
}
#else
#define BOOT_TARGETS_NAME_ARRAY {       \
    DEV_BOOCS,          	\
    TBL_TERM,         	\
}
#endif

#if defined (MV_INCLUDE_PEX)
	#define PCI_IF0_MEM0		PEX0_MEM
	#define PCI_IF0_IO		PEX0_IO
#endif


                    
#define TCLK_TO_COUNTER_RATIO   1   /* counters running in Tclk */

/* MV78XX0 sample @ reset registers offsets */
/*******************************************/
#define CPU_RESET_SAMPLE_L_REG			(0x10030)
#define CPU_RESET_SAMPLE_H_REG			(0x10034)
/* S@R Register low */
#define MSAR_SYSCLCK_OFFS            	5
#define MSAR_SYSCLCK_MASK            	(0x7 << MSAR_SYSCLCK_OFFS)
#define MSAR_SYSCLCK_200               	(0x1 << MSAR_SYSCLCK_OFFS)
#define MSAR_SYSCLCK_267               	(0x2 << MSAR_SYSCLCK_OFFS)
#define MSAR_SYSCLCK_333               	(0x3 << MSAR_SYSCLCK_OFFS)
#define MSAR_SYSCLCK_400               	(0x4 << MSAR_SYSCLCK_OFFS)
#define MSAR_SYSCLCK_250               	(0x5 << MSAR_SYSCLCK_OFFS)
#define MSAR_SYSCLCK_300               	(0x6 << MSAR_SYSCLCK_OFFS)

#define MSAR_SYSCLK2CPU_OFFS            8
#define MSAR_SYSCLK2CPU_MASK            (0xF << MSAR_SYSCLK2CPU_OFFS)
/* 1_5 == sysclk * 1.5 */
#define MSAR_SYSCLK2CPU_1            	(0x0 << MSAR_SYSCLK2CPU_OFFS)
#define MSAR_SYSCLK2CPU_1_5            	(0x1 << MSAR_SYSCLK2CPU_OFFS)
#define MSAR_SYSCLK2CPU_2            	(0x2 << MSAR_SYSCLK2CPU_OFFS)
#define MSAR_SYSCLK2CPU_2_5            	(0x3 << MSAR_SYSCLK2CPU_OFFS)
#define MSAR_SYSCLK2CPU_3            	(0x4 << MSAR_SYSCLK2CPU_OFFS)
#define MSAR_SYSCLK2CPU_3_5            	(0x5 << MSAR_SYSCLK2CPU_OFFS)
#define MSAR_SYSCLK2CPU_4            	(0x6 << MSAR_SYSCLK2CPU_OFFS)
#define MSAR_SYSCLK2CPU_4_5            	(0x7 << MSAR_SYSCLK2CPU_OFFS)
#define MSAR_SYSCLK2CPU_5            	(0x8 << MSAR_SYSCLK2CPU_OFFS)

#define MSAR_CPUL2CLK_OFFS		12
#define MSAR_CPUL2CLK_MASK		(0x3 << MSAR_CPUL2CLK_OFFS)
#define MSAR_CPUL2CLK_1            	(0x0 << MSAR_CPUL2CLK_OFFS)
#define MSAR_CPUL2CLK_2            	(0x1 << MSAR_CPUL2CLK_OFFS)

#define MSAR_BOOTDEV_OFFS		23
#define MSAR_BOOTDEV_MASK		(0x3 << MSAR_BOOTDEV_OFFS)
#define MSAR_BOOTDEV_FLASH            	(0x0 << MSAR_BOOTDEV_OFFS)
#define MSAR_BOOTDEV_SPI            	(0x1 << MSAR_BOOTDEV_OFFS)
#define MSAR_BOOTDEV_DCE_NAND          	(0x2 << MSAR_BOOTDEV_OFFS)
#define MSAR_BOOTDEV_CE_NAND          	(0x3 << MSAR_BOOTDEV_OFFS)

#define MSAR_CPU1_EN_OFFS		20
#define MSAR_CPU1_EN_MASK		(0x1 << MSAR_CPU1_EN_OFFS)
#define MSAR_CPU1_DIS            	(0x0 << MSAR_CPU1_EN_OFFS)
#define MSAR_CPU1_EN			(0x1 << MSAR_CPU1_EN_OFFS)

/* S@R Register high */
#define MSAR_TCLCK_MODE_OFFS            6
#define MSAR_TCLCK_MODE_MASK            (0x1 << MSAR_TCLCK_MODE_OFFS)
#define MSAR_TCLCK_MODE_EXT             (0x0 << MSAR_TCLCK_MODE_OFFS)
#define MSAR_TCLCK_MODE_PLL             (0x1 << MSAR_TCLCK_MODE_OFFS)
#define MSAR_TCLCK_OFFS                  7
#define MSAR_TCLCK_MASK                  (0x3 << MSAR_TCLCK_OFFS)
#define MSAR_TCLCK_167                   (0x0 << MSAR_TCLCK_OFFS)
#define MSAR_TCLCK_200                   (0x1 << MSAR_TCLCK_OFFS)
#define MSAR_TCLCK_DES_OFFS		9
#define MSAR_TCLCK_DES_MASK             (0x1 << MSAR_TCLCK_DES_OFFS)
#define MSAR_TCLCK_DES_167              (0x0 << MSAR_TCLCK_DES_OFFS)
#define MSAR_TCLCK_DES_200              (0x1 << MSAR_TCLCK_DES_OFFS)

/* CPU system reset mask register */
#define CPU_RESET_OUT_MASK_REG			(CPU_IF_BASE(whoAmI())+0x108)
#define CPU_PEX_RESET_OUT_MASK_OFF			0
#define CPU_PEX_RESET_OUT_MASK			(0x1 << CPU_PEX_RESET_OUT_MASK_OFF)
#define CPU_WD_RESET_OUT_MASK_OFF			1
#define CPU_WD_RESET_OUT_MASK			(0x1 << CPU_WD_RESET_OUT_MASK_OFF)
#define CPU_SOFT_RESET_OUT_MASK_OFF			2
#define CPU_SOFT_RESET_OUT_MASK			(0x1 << CPU_SOFT_RESET_OUT_MASK_OFF)

/* CPU soft system reset register */
#define CPU_SOFT_RESET_OUT_REG			(CPU_IF_BASE(whoAmI())+0x10C)
#define CPU_SOFT_RESET_OUT_OFF			0
#define CPU_SOFT_RESET_OUT			(0x1 << CPU_SOFT_RESET_OUT_OFF)


#define POWER_MNG_CTRL_REG				(CPU_IF_BASE(whoAmI())+0x11C)

#define PMC_GE_OFFS(port)				((port)+1)
#define PMC_GE_MASK(port)				(1 << PMC_GE_OFFS(port))
#define PMC_GE_UP(port)					(1 << PMC_GE_OFFS(port))

#define PMC_PEX_OFFS(port)				((port)+5)
#define PMC_PEX_MASK(port)				(1 << PMC_PEX_OFFS(port))
#define PMC_PEX_UP(port)				(1 << PMC_PEX_OFFS(port))

#define PMC_SATA_OFFS(port)				(((port)<<1)+14)
#define PMC_SATA_MASK(port)				(1 << PMC_SATA_OFFS(port))
#define PMC_SATA_UP(port)				(1 << PMC_SATA_OFFS(port))

#define PMC_USB_OFFS(port)				((port)+17)
#define PMC_USB_MASK(port)				(1 << PMC_USB_OFFS(port))
#define PMC_USB_UP(port)				(1 << PMC_USB_OFFS(port))

#define PMC_SE_OFFS						22
#define PMC_SE_MASK						(1 << PMC_SE_OFFS)
#define PMC_SE_UP						(1 << PMC_SE_OFFS)



/* typedefs */

#ifndef MV_ASMLANGUAGE

/* This enumerator described the possible Controller paripheral targets.    */
/* Controller peripherals are designated memory/IO address spaces that the  */
/* controller can access. They are also refered as "targets"                */

typedef enum _mvTarget
{	
    TBL_TERM=-1,    /* Invalid Target*/
    SDRAM_CS0,      /* 0 SDRAM chip select 0                                  */  
    SDRAM_CS1,      /* 1 SDRAM chip select 1                                  */  
    SDRAM_CS2,      /* 2 SDRAM chip select 2                                  */  
    SDRAM_CS3,      /* 3 SDRAM chip select 3                                  */  
    DEVICE_CS0,     /* 4 Device chip select 0                                 */  
    DEVICE_CS1,     /* 5 Device chip select 1                                 */  
    DEVICE_CS2,     /* 6 Device chip select 2                                 */  
    DEVICE_CS3,     /* 7 Device chip select 3                                 */  
    DEV_BOOCS,      /* 8 Boot device chip select                              */
#if !defined(MV78XX0_Z0)
    SPI_CS,         /* x SPI device chip select				      */
#endif
    PCI0_IO,        /* 9 PCI 0 IO                                             */  
    PCI0_MEM0,      /* 10 PCI 0 memory 0                                       */  
    PCI1_IO,        /* 11 PCI 1 IO                                             */  
    PCI1_MEM0,      /* 12 PCI 1 memory 0                                       */  
    PCI2_IO,	    /* 13 PCI 2 IO                                             */  
    PCI2_MEM0,      /* 14 PCI 2 memory 0                                       */  
    PCI3_IO,        /* 15 PCI 3 IO                                             */  
    PCI3_MEM0,      /* 16 PCI 3 memory 0                                       */  
    PCI4_IO,        /* 17 PCI 4 IO                                             */  
    PCI4_MEM0,      /* 18 PCI 4 memory 0                                       */  
    PCI5_IO,        /* 19 PCI 4 IO                                             */  
    PCI5_MEM0,      /* 20 PCI 4 memory 0                                       */  
    PCI6_IO,        /* 21 PCI 4 IO                                             */  
    PCI6_MEM0,      /* 22 PCI 4 memory 0                                       */  
    PCI7_IO,        /* 23 PCI 4 IO                                             */  
    PCI7_MEM0,      /* 24 PCI 4 memory 0                                       */  
    CRYPT_ENG,      /* 25 Crypto Engine - map to bar 4 without remap	    */
    INTER_REGS,     /* 26 Internal registers                                   */  
    MAX_TARGETS,
    USB_IF,         /* USB interface. Note: This is a logic target!         */  
}MV_TARGET;

#define SDRAM_CS(dramScNum)     (SDRAM_CS0  + dramScNum)
#define DEVICE_CS(devNum)       (DEVICE_CS0 + devNum)
#define PCI_IO(pciIf)		(PCI0_IO   + (2 * pciIf))
#define PCI_MEM(pciIf, memNum)  (PCI0_MEM0 + (2 * pciIf))
/* convert device number to its target number */
#define DEV_TO_TARGET(dev)  	((dev) + DEVICE_CS0) 

/* This enumerator defines the Marvell controller target ID      */ 
typedef enum _mvTargetId
{
    DRAM_TARGET_ID = 0,    
    DEV_TARGET_ID = 1,     
    PCI0_TARGET_ID = 4,
    PCI1_TARGET_ID = 8,
    CRYPT_TARGET_ID =9,
    MAX_TARGET_ID
}MV_TARGET_ID;


/* This enumerator describes the Marvell controller possible devices that   */
/* can be connected to its device interface.                                */
typedef enum _mvDevice
{
    DEV_CS0,        /* Device connected to dev CS[0]    */
    DEV_CS1,        /* Device connected to dev CS[1]    */
    DEV_CS2,        /* Device connected to dev CS[2]    */
    DEV_CS3,        /* Device connected to dev CS[3]    */
    BOOT_CS,        /* Device connected to boot CS      */
    MV_DEV_MAX_CS
}MV_DEVICE;

/* This enumerator defines the Marvell Units ID      */ 
typedef enum _mvUnitId
{
    DRAM_UNIT_ID,
    PEX_UNIT_ID,
    PCI_UNIT_ID,
    ETH_GIG_UNIT_ID,
    ETH_UNM_UNIT_ID,
    USB_UNIT_ID,
    IDMA_UNIT_ID,
    XOR_UNIT_ID,
    SATA_UNIT_ID,
    TDM_UNIT_ID,
    UART_UNIT_ID,
    CESA_UNIT_ID,
#if !defined(MV78XX0_Z0)
    SPI_UNIT_ID,
#endif
    MAX_UNITS_ID,

}MV_UNIT_ID; 

#endif /* MV_ASMLANGUAGE */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __INCmvCtrlEnvSpech */
