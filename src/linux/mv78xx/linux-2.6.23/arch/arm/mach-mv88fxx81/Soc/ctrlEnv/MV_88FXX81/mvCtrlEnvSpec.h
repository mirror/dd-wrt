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

/* includes */


/* defines  */

#define MV_1181_DEV_ID			0x1181
#define MV_1181_DEV_NAME		"88F1181"

#define MV_5181_DEV_NAME		"88F5181"
#define MV_5181_DEV_ID			0x5181
#define MV_5281_DEV_ID			0x5281
#define MV_5182_DEV_ID			0x5182
#define MV_8660_DEV_ID			0x8660

/* 0x5181 revisions*/
#define MV_5181_A0_REV			0x0
#define MV_5181_A0_ID			((MV_5181_DEV_ID << 16) | MV_5181_A0_REV)
#define MV_5181_A0_NAME			"88F5181 A0"
#define MV_5181_A1_REV			0x1
#define MV_5181_A1_ID			((MV_5181_DEV_ID << 16) | MV_5181_A1_REV)
#define MV_5181_A1_NAME			"88F5181 A1"
#define MV_5181_B0_REV			0x2
#define MV_5181_B0_ID			((MV_5181_DEV_ID << 16) | MV_5181_B0_REV)
#define MV_5181_B0_NAME			"88F5181 B0"
#define MV_5181_B1_REV			0x3
#define MV_5181_B1_ID			((MV_5181_DEV_ID << 16) | MV_5181_B1_REV)
#define MV_5181_B1_NAME			"88F5181 B1"
/* 0x5281 revisions*/
#define MV_5281_A0_REV			0x0
#define MV_5281_A0_ID			((MV_5281_DEV_ID << 16) | MV_5281_A0_REV)
#define MV_5281_A0_NAME			"88F5281 A0"
#define MV_5281_B0_REV			0x1
#define MV_5281_B0_ID			((MV_5281_DEV_ID << 16) | MV_5281_B0_REV)
#define MV_5281_B0_NAME			"88F5281 B0"
#define MV_5281_C0_REV			0x2
#define MV_5281_C0_ID			((MV_5281_DEV_ID << 16) | MV_5281_C0_REV)
#define MV_5281_C0_NAME			"88F5281 C0"
#define MV_5281_C1_REV			0x3
#define MV_5281_C1_ID			((MV_5281_DEV_ID << 16) | MV_5281_C1_REV)
#define MV_5281_C1_NAME			"88F5281 C1"
/* 0x5182 revisions*/
#define MV_5182_A0_REV			0x0
#define MV_5182_A0_ID			((MV_5182_DEV_ID << 16) | MV_5182_A0_REV)
#define MV_5182_A0_NAME			"88F5182 A0"
#define MV_5182_A1_REV			0x1
#define MV_5182_A1_ID			((MV_5182_DEV_ID << 16) | MV_5182_A1_REV)
#define MV_5182_A1_NAME			"88F5182 A1"
#define MV_5182_A2_REV			0x2
#define MV_5182_A2_ID			((MV_5182_DEV_ID << 16) | MV_5182_A2_REV)
#define MV_5182_A2_NAME			"88F5182 A2"
/* 0x5181L revisions*/
#define MV_5181L_A0_REV			0x8
#define MV_5181L_A0_ID			((MV_5181_DEV_ID << 16) | MV_5181L_A0_REV)
#define MV_5181L_A0_NAME		"88F5181L A0"
#define MV_5181L_A1_REV			0x9
#define MV_5181L_A1_ID			((MV_5181_DEV_ID << 16) | MV_5181L_A1_REV)
#define MV_5181L_A1_NAME		"88F5181L A1"

/* 0x8660 revisions */
#define MV_8660_A0_REV			0x8
#define MV_8660_A0_ID			((MV_8660_DEV_ID << 16) | MV_8660_A0_REV)
#define MV_8660_A0_NAME			"88W8660 A0"


#if defined(MV_88F1181)
#elif defined(MV_88F5181)

#define ETH_REG_BASE            0x72000
#define USB_REG_BASE(dev)       (((dev) == 0) ? 0x50000 : 0xA0000)

#else
#   error "CHIP not selected"
#endif /* #if defined(MV_88F1181) */


#define INTER_REGS_SIZE	 	 	_1M


/* This define describes the maximum controller supported DRAM chip select 	*/
/* also known as banks                                                     	*/
#ifndef MV_88W8660
#define MV_DRAM_MAX_CS      4
#else
#define MV_DRAM_MAX_CS      2
#endif


/* This define describes maximum of GPP groups supported by controller. 	*/
#define MV_GPP_MAX_GROUP    1


/* This define describes the maximum number of available Timer/counters.  	*/
#define MV_CNTMR_MAX_COUNTER 2

/* This define describes the maximum number of available UART channels.  	*/
#define MV_UART_MAX_CHAN	 2


#if defined(MV_88F1181)

/* This define describes maximum number of MPP groups.                      */
/* Each group describes an MPP register.  	                                */
#define MV_MPP_MAX_GROUP    2

/* This define describes the maximum number of supported PCI\PCIX Interfaces*/
#define MV_PCI_MAX_IF		0
#define MV_PCI_START_IF		0

/* This define describes the maximum number of supported PEX Interfaces 	*/
#define MV_PEX_MAX_IF		2
#define MV_PEX_START_IF		MV_PCI_MAX_IF

/* This define describes the maximum number of supported PCI Interfaces 	*/
#define MV_PCI_IF_MAX_IF   	(MV_PEX_MAX_IF+MV_PCI_MAX_IF)

/* This define describes the maximum number of supported Ethernet ports 	*/
#define MV_ETH_MAX_PORTS	0

/* This define describes the maximum number of supported IDMA channels. 	*/
#define MV_IDMA_MAX_CHAN    0

/* This define describes the the support of USB 	*/
#define MV_USB_1181_MAX   	0


#elif defined(MV_88F5181)

/* This define describes maximum number of MPP groups.                      */
/* Each group describes an MPP register.  	                                */

#if defined(MV_88F5182)
#define MV_MPP_MAX_GROUP    3
#elif  defined(MV_88W8660)
#define MV_MPP_MAX_GROUP    2
#else
#define MV_MPP_MAX_GROUP    4
#endif
/* This define describes the maximum number of supported PEX Interfaces 	*/
#define MV_PEX_MAX_IF		1
#define MV_PEX_START_IF		0

/* This define describes the maximum number of supported PCI/PCIX Interfaces  */
#define MV_PCI_MAX_IF		1
#define MV_PCI_START_IF		MV_PEX_MAX_IF


/* This define describes the maximum number of supported PCI Interfaces 	*/
#define MV_PCI_IF_MAX_IF		(MV_PEX_MAX_IF+MV_PCI_MAX_IF)

/* This define describes the maximum number of supported Ethernet ports 	*/
#define MV_ETH_MAX_PORTS	1

/* This define describes the maximum number of supported IDMA channels. 	*/
#define MV_IDMA_MAX_CHAN    4

/* This define describes the the support of USB 	*/
/* Orion 1/2 */
#define MV_USB_5181_MAX   	1

/* Orion-NAS */
#define MV_USB_5182_MAX   	2

#define SATA_REG_BASE           0x80000


#else
#   error "CHIP not selected"
#endif /* #if defined(MV_88F1181) */



/* Controler environment registers offsets */
#define MPP_CONTROL_REG0				0x10000
#define MPP_CONTROL_REG1				0x10004
#define MPP_CONTROL_REG2				0x10050
#define DEV_MULTI_CONTROL				0x10008
#define MPP_SAMPLE_AT_RESET				0x10010
#define MV_UART_CHAN_BASE(chanNum)			(0x12000 + (chanNum * 0x100))

#if defined(MV_88F5181)
#define MSAR_TCLCK_OFFS					8
#elif defined(MV_88F1181)
#define MSAR_TCLCK_OFFS					10
#endif
#define MSAR_TCLCK_MASK				    (0x3 << MSAR_TCLCK_OFFS)
#define MSAR_TCLCK_133					(0x0 << MSAR_TCLCK_OFFS)
#define MSAR_TCLCK_150					(0x1 << MSAR_TCLCK_OFFS)
#define MSAR_TCLCK_166					(0x2 << MSAR_TCLCK_OFFS)


#if defined(MV_88F5181)
#define MSAR_ARMDDRCLCK_OFFS		   	4
#elif defined(MV_88F1181)
#define MSAR_ARMDDRCLCK_OFFS		   	6
#endif


#define MSAR_ARMDDRCLCK_MASK			(0xf << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_H_MASK			BIT23

#define MSAR_ARMDDRCLCK_333_167	  		(0x0 << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_400_200			(0x1 << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_400_133			(0x2 << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_500_167			(0x3 << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_533_133			(0x4 << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_600_200			(0x5 << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_667_167			(0x6 << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_800_200			(0x7 << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_480_160         (0xc << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_550_183         (0xd << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_525_175         (0xe << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_466_233			(0x11 << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_500_250         (0x12 << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_533_266         (0x13 << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_600_300         (0x14 << MSAR_ARMDDRCLCK_OFFS)

#define MSAR_ARMDDRCLCK_450_150         (0x15 << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_533_178         (0x16 << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_575_192         (0x17 << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_700_175         (0x18 << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_733_183         (0x19 << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_750_187         (0x1A << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_775_194         (0x1B << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_500_125         (0x1C << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_500_100         (0x1D << MSAR_ARMDDRCLCK_OFFS)
#define MSAR_ARMDDRCLCK_600_150       	(0x1E << MSAR_ARMDDRCLCK_OFFS)


#define MSAR_GIGA_PORT_MODE_OFFS		15
#define MSAR_GIGA_PORT_MODE_MASK		(0x7 <<  MSAR_GIGA_PORT_MODE_OFFS)
   	
#define MSAR_GIGA_PORT_MODE_GMII		(0x2 <<  MSAR_GIGA_PORT_MODE_OFFS)
#define MSAR_GIGA_PORT_MODE_MII			(0x3 <<  MSAR_GIGA_PORT_MODE_OFFS)
#define MSAR_GIGA_PORT_MODE_RGMII		(0x7 <<  MSAR_GIGA_PORT_MODE_OFFS)




/* These macros help units to identify a target Mbus Arbiter group */
#define MV_TARGET_IS_DRAM(target)   \
                            ((target >= SDRAM_CS0) && (target <= SDRAM_CS3))

#define MV_TARGET_IS_PEX0(target)   \
                            ((target >= PEX0_MEM) && (target <= PEX0_IO))


#if defined(MV_88F1181)
						
#define MV_TARGET_IS_PEX1(target)   \
                            ((target >= PEX1_MEM) && (target <= PEX1_IO))

#define MV_TARGET_IS_DEVICE(target) \
                            ((target >= FLASH_CS) && (target <= DEV_BOOCS))

#elif defined(MV_88F5181)

#define MV_TARGET_IS_PEX1(target)   0

#define MV_TARGET_IS_DEVICE(target) \
                            ((target >= DEVICE_CS0) && (target <= DEV_BOOCS))
                            
#define MV_TARGET_IS_PCI(target)   \
                            ((target >= PCI0_MEM) && (target <= PCI0_IO))
                            
#define MV_PCI_DRAM_BAR_TO_DRAM_TARGET(bar)   \
                            ((MV_TARGET)((MV_U32)(bar - CS0_BAR) + (MV_U32)SDRAM_CS0))

                            
#else
#   error "CHIP not selected"
#endif /* #if defined(MV_88F1181) */

					

#define TCLK_TO_COUNTER_RATIO   1   /* counters running in Tclk */
/* typedefs */


#ifndef MV_ASMLANGUAGE
/* This enumerator described the possible Controller paripheral targets.    */
/* Controller peripherals are designated memory/IO address spaces that the  */
/* controller can access. They are also refered as "targets"                */
typedef enum _mvTarget
{
    TBL_TERM = -1, /* none valid target, used as targets list terminator*/
    SDRAM_CS0,      /* SDRAM chip select 0                                  */  
    SDRAM_CS1,      /* SDRAM chip select 1                                  */  
    SDRAM_CS2,      /* SDRAM chip select 2                                  */  
    SDRAM_CS3,      /* SDRAM chip select 3                                  */  
	PEX0_MEM,		/* PCI Express 0 Memory 								*/
	PEX0_IO,		/* PCI Express 0 IO										*/
#if defined(MV_88F1181)
	PEX1_MEM,		/* PCI Express 1 Memory 								*/
	PEX1_IO,		/* PCI Express 1 IO		 								*/
#elif defined(MV_88F5181)
	PCI0_MEM,		/* PCI Memory 											*/
	PCI0_IO,			/* PCI IO		 									*/
#else                                                
#   error "CHIP not selected"                        
#endif                                               
    INTER_REGS,     /* Internal registers                                   */  
#if defined(MV_88F5181)
    DEVICE_CS0,     /* Device chip select 0                                 */  
    DEVICE_CS1,     /* Device chip select 0                                 */  
    DEVICE_CS2,     /* Device chip select 0                                 */  
#elif defined (MV_88F1181)                           
    FLASH_CS,     	/* Flash chip select	                                */  
#else                                                
#   error "CHIP not selected"                        
#endif                                               
	DEV_BOOCS,    	/* Flash Boot chip select		                        */  
#if defined(MV_88F5182) || defined (MV_88F5181L)
	CRYPT_ENG,    	/* Crypto Engine				                        */  
#endif
    MAX_TARGETS

}MV_TARGET;



/* This enumerator defines the Marvell controller target ID      */ 
typedef enum _mvTargetId
{
    DRAM_TARGET_ID = 0 ,    /* Port 0 -> DRAM interface         */
    DEV_TARGET_ID  = 1,     /* Port 1 -> PCI Express 		    */
#if defined(MV_88F5181)
	PCI_TARGET_ID  = 3 ,    /* Port 3 -> PCI 		 		    */
#elif defined(MV_88F1181)
    PEX1_TARGET_ID  = 3 ,    /* Port 4 -> PCI Express0 		    */
#else                                                
#   error "CHIP not selected"                        
#endif                                               
	PEX0_TARGET_ID  = 4 ,    /* Port 4 -> PCI Express0 		    */
#if defined(MV_88F5182) || defined (MV_88F5181L)
	CRYPT_TARGET_ID =9 ,	/* Port 9 --> Crypto Engine 		*/
#endif
	MAX_TARGETS_ID
}MV_TARGET_ID;

/* This enumerator defines the Marvell Units ID      */ 
typedef enum _mvUnitId
{
    AHB_TO_MBUS_UNIT_ID,   
    DRAM_UNIT_ID,
	PEX_UNIT_ID,
#if defined(MV_88F5181)
	PCI_UNIT_ID,
	ETH_UNIT_ID,
	USB_UNIT_ID,
	IDMA_UNIT_ID,
#elif defined(MV_88F1181)
#else                                                
#   error "CHIP not selected"                        
#endif                                               
	MAX_UNITS_ID
}MV_UNIT_ID;

#endif /* MV_ASMLANGUAGE */


#endif /* __INCmvCtrlEnvSpech */
