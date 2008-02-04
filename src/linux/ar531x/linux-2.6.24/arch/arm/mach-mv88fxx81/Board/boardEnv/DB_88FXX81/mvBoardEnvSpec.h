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


#ifndef __INCmvBoardEnvSpech
#define __INCmvBoardEnvSpech

#include "mvSysHwConfig.h"


/* For future use */
#define BD_ID_DATA_START_OFFS	0x0
#define BD_DETECT_SEQ_OFFS		0x0
#define BD_SYS_NUM_OFFS			0x4
#define BD_NAME_OFFS			0x8

#define DB_88F1181_DDR1				0x0
#define DB_88F1181_DDR2				0x1
#define DB_88F5181_5281_DDR1		0x2
#define DB_88F5181_5281_DDR2		0x3
#define DB_88F5181_DDR1_PRPMC		0x4
#define DB_88F5181_DDR1_PEXPCI		0x5
#define RD_88F5181_POS_NAS 			0x6
#define DB_88F5X81_DDR2				0x7
#define DB_88F5X81_DDR1				0x8
#define RD_88F5181_VOIP				0x9
#define DB_88F5182_DDR2				0xA
#define RD_88F5182_2XSATA			0xB
#define DB_88F5181L_DDR2_2XTDM			0xC
#define RD_88F5181L_VOIP_FE			0xD
#define RD_88F5181L_VOIP_GE			0xE
#define RD_88F5182_2XSATA3			0xF
#define DB_88W8660_DDR2				0x10
#define RD_88W8660_DDR1				0x11

#define MV_MAX_BOARD_ID				(RD_88W8660_DDR1 + 1)





/* MPP possible values in the remarks should be updated from the board
   sheet or taken from HW team */


/* I2C bus addresses */
#define MV_BOARD_CTRL_I2C_ADDR			0x0     /* Controller slave addr */
#define MV_BOARD_CTRL_I2C_ADDR_TYPE 		ADDR7_BIT
#define MV_BOARD_DIMM0_I2C_ADDR			0x56
#define MV_BOARD_DIMM0_I2C_ADDR_TYPE 		ADDR7_BIT
#define MV_BOARD_DIMM1_I2C_ADDR			0x54
#define MV_BOARD_DIMM1_I2C_ADDR_TYPE 		ADDR7_BIT
#define MV_BOARD_EEPROM_I2C_ADDR	    	0x51
#define MV_BOARD_EEPROM_I2C_ADDR_TYPE 		ADDR7_BIT
#define MV_BOARD_MAIN_EEPROM_I2C_ADDR	   	0x50
#define MV_BOARD_MAIN_EEPROM_I2C_ADDR_TYPE 	ADDR7_BIT

/* Eeprom board data */
#define MV_BOARD_ID_EEPROM                      MV_BOARD_MAIN_EEPROM_I2C_ADDR
#define MV_BOARD_ID_EEPROM_OFFSET0              0x1F0		/* last 16byte in 0.5KByte EEPROMS */
#define MV_BOARD_ID_EEPROM_OFFSET1              0x1FF0		/* last 16byte in 8KByte EEPROMS */
#define MV_BOARD_I2C_MAGIC                      0xFEEDFEED


#define BOOT_FLASH_INDEX					0
#define MAIN_FLASH_INDEX					1

/* Boot Flash definitions */
#define	MV_BOARD_BOOT_FLASH_BASE_ADRS		mvBoardGetDeviceBaseAddr(BOOT_FLASH_INDEX,	\
																	 BOARD_DEV_NOR_FLASH)
#define MV_BOARD_BOOT_FLASH_BUS_WIDTH	    mvBoardGetDeviceBusWidth(BOOT_FLASH_INDEX,	\
																	  BOARD_DEV_NOR_FLASH)
#define MV_BOARD_BOOT_FLASH_DEVICE_WIDTH	mvBoardGetDeviceWidth(BOOT_FLASH_INDEX,	\
																   BOARD_DEV_NOR_FLASH)

/* Board main flash */
#define	MV_BOARD_FLASH_BASE_ADRS    	mvBoardGetDeviceBaseAddr(MAIN_FLASH_INDEX,	\
																	 BOARD_DEV_NOR_FLASH)
#define MV_BOARD_FLASH_BUS_WIDTH		mvBoardGetDeviceBusWidth(MAIN_FLASH_INDEX,	\
																	  BOARD_DEV_NOR_FLASH)
#define MV_BOARD_FLASH_DEVICE_WIDTH		mvBoardGetDeviceWidth(MAIN_FLASH_INDEX,	\
																   BOARD_DEV_NOR_FLASH)


/* Clocks stuff */
#define MV_BOARD_DEFAULT_TCLK	166000000	/* Default Tclk 1663MHz 		*/
#define MV_BOARD_DEFAULT_SYSCLK	200000000	/* Default SysClk 200MHz 	*/
#define MV_BOARD_DEFAULT_PCLK	400000000   /* Default Pclock 400 MHZ*/

#define MV_BOARD_REF_CLOCK	3686400	/* Refrence Clock 3.6864MHz 	*/


/* Supported clocks */
#define MV_BOARD_TCLK_100MHZ	100000000   
#define MV_BOARD_TCLK_125MHZ	125000000	/* Using 50MHz Xtal */
#define MV_BOARD_TCLK_133MHZ	133000000   
#define MV_BOARD_TCLK_150MHZ	150000000   /* Using 50MHz Xtal */
#define MV_BOARD_TCLK_166MHZ	166000000   
#define MV_BOARD_TCLK_200MHZ	200000000   

#define MV_BOARD_SYSCLK_100MHZ	100000000   
#define MV_BOARD_SYSCLK_125MHZ	125000000   /* Using 50MHz Xtal */
#define MV_BOARD_SYSCLK_133MHZ	133000000   
#define MV_BOARD_SYSCLK_150MHZ	150000000   /* Using 50MHz Xtal */
#define MV_BOARD_SYSCLK_166MHZ	166000000   
#define MV_BOARD_SYSCLK_200MHZ	200000000   



#define DB_88F1181_DDR1_INFO	{												\
																				\
	"DB-88F1181-DDR1",						/* boardName[MAX_BOARD_NAME_LEN] */	\
																				\
	{0x00110001,							/* mpp0_7 */						\
	0x00001111,								/* mpp8_15 */                       \
	N_A,									/* mpp16_23 */                      \
	N_A},									/* mppDev */                        \
																				\
	(1 << 1),								/* intsGppMask */                   \
																				\
	/*{params, devType, devWidth}*/			/* devCsInfo[MV_BOARD_MAX_DEV] */   \
																				\
	{{ N_A, N_A, N_A},						/* devCs0 */                        \
	 { N_A, N_A, N_A},						/* devCs1 */                        \
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 16},	/* devCs2/flashCs */            \
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}},	/* bootCs */                    \
																				\
	/*pciBoardIf[MV_BOARD_MAX_PCI_IF];*/										\
																				\
	{{N_A,									/* firstSlotDevNum */               \
	0,		   		 						/* pciSlotsNum */                   \
																				\
	/*pciSlot[MV_BOARD_MAX_PCI_SLOTS]*/											\
	/* {{intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					\
																				\
	{{ {N_A, N_A, N_A, N_A}},			/* pciSlot0 */							\
	 { {N_A, N_A, N_A, N_A}},			/* pciSlot1 */                      	\
	 { {N_A, N_A, N_A, N_A}}}}},		/* pciSlot2 */                      	\
																				\
	0x68,									/* rtcTwsiAddr */                   \
    ADDR7_BIT,								/* rtcTwsiAddrType */               \
	0x63,									/* pexPciBridgeTwsiAddr */          \
	ADDR7_BIT,								/* pexPciBridgeTwsiAddrType */      \
																				\
	{N_A},									/* ethPhyAddr[MV_BOARD_MAX_PORTS] */\
																				\
	1,										/* rtcIntPin */                     \
	N_A,									/* switchIntPin */                  \
	{N_A,N_A},						/* vbusUsbGppPin[MV_BOARD_MAX_USB_IF] */	\
																				\
	8,										/* activeLedsNumber */              \
																				\
	{2,										/* led0GppPin */                    \
	3,										/* led1GppPin */                    \
	6,										/* led2GppPin */                    \
	7,										/* led3GppPin */                    \
	12,										/* led4GppPin */                    \
	13,										/* led5GppPin */                    \
	14,										/* led6GppPin */                    \
	15},									/* led7GppPin */            		\
	1,										/* ledsPolarity */					\
	0										/* refClkGppPin */					\
}


#define DB_88F1181_DDR2_INFO	{												\
																				\
	"DB-88F1181-DDR2",						/* boardName[MAX_BOARD_NAME_LEN] */	\
																				\
	{0x00110001,							/* mpp0_7 */						\
	0x00001111,								/* mpp8_15 */                       \
	N_A,									/* mpp16_23 */                      \
	N_A},									/* mppDev */                        \
																				\
	(1 << 1),								/* intsGppMask */                   \
																				\
	/*{params, devType, devWidth}*/			/* devCsInfo[MV_BOARD_MAX_DEV] */   \
																				\
	{{ N_A, N_A, N_A},						/* devCs0 */                        \
	 { N_A, N_A, N_A},						/* devCs1 */                        \
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 16},	/* devCs2/flashCs */            \
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}},	/* bootCs */                    \
																				\
	/*pciBoardIf[MV_BOARD_MAX_PCI_IF];*/	  									\
																				\
	{{N_A,									/* firstSlotDevNum */               \
	0,	  		  							/* pciSlotsNum */                   \
																				\
	/*pciSlot[MV_BOARD_MAX_PCI_SLOTS]*/											\
	/* {{intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					\
																				\
	{{ {N_A, N_A, N_A, N_A}},			/* pciSlot0 */							\
	 { {N_A, N_A, N_A, N_A}},			/* pciSlot1 */                      	\
	 { {N_A, N_A, N_A, N_A}}}}},		/* pciSlot2 */                      	\
																				\
	0x68,									/* rtcTwsiAddr */                   \
    ADDR7_BIT,								/* rtcTwsiAddrType */               \
	0x63,									/* pexPciBridgeTwsiAddr */          \
	ADDR7_BIT,								/* pexPciBridgeTwsiAddrType */      \
																				\
	{N_A},									/* ethPhyAddr[MV_BOARD_MAX_PORTS] */\
																				\
	1,										/* rtcIntPin */                     \
	N_A,									/* switchIntPin */                  \
	{N_A,N_A},						/* vbusUsbGppPin[MV_BOARD_MAX_USB_IF] */	\
																				\
	8,										/* activeLedsNumber */              \
																				\
	{2,										/* led0GppPin */                    \
	3,										/* led1GppPin */                    \
	6,										/* led2GppPin */                    \
	7,										/* led3GppPin */                    \
	12,										/* led4GppPin */                    \
	13,										/* led5GppPin */                    \
	14,										/* led6GppPin */                    \
	15},									/* led7GppPin */                    \
	1,										/* ledsPolarity */					\
	0										/* refClkGppPin */					\
}


#define DB_88F5181_5281_DDR1_MPP0_7	       	0x00332222
#define DB_88F5181_5281_DDR1_MPP8_15	   	0x11111111
#define DB_88F5181_5281_DDR1_MPP16_23  		0x1111
#define DB_88F5181_5281_DDR1_MPP_DEV		0x0


#define DB_88F5181_5281_DDR1_INFO	{											\
																				\
	"DB-88F5181-DDR1",						/* boardName[MAX_BOARD_NAME_LEN] */	\
																				\
	{DB_88F5181_5281_DDR1_MPP0_7,			/* mpp0_7 */						\
	DB_88F5181_5281_DDR1_MPP8_15,			/* mpp8_15 */                       \
	DB_88F5181_5281_DDR1_MPP16_23,			/* mpp16_23 */                      \
	DB_88F5181_5281_DDR1_MPP_DEV},			/* mppDev */                        \
																				\
	((1<<1)|(1 << 6)|(1<<7)),				/* intsGppMask */                   \
																				\
	/*{params, devType, devWidth}*/			/* devCsInfo[MV_BOARD_MAX_DEV] */   \
																				\
	{{ 0x8fcfffff, N_A, N_A},				/* devCs0 */                        \
	 { 0x8fefffff, BOARD_DEV_NOR_FLASH, 16},   	/* devCs1 */                    \
	 { 0x8fcfffff, BOARD_DEV_SEVEN_SEG, N_A},	/* devCs2/flashCs */            \
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}},	/* bootCs */                    \
																				\
	/*pciBoardIf[MV_BOARD_MAX_PCI_IF];*/	  									\
																				\
	{{0x7,									/* firstSlotDevNum */               \
	3,										/* pciSlotsNum */                   \
																				\
	/*pciSlot[MV_BOARD_MAX_PCI_SLOTS]*/											\
	/* {{intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					\
																				\
	{{ {7, 7, 7, 7}},					/* pciSlot0 */							\
	 { {6, 6, 6, 6}},					/* pciSlot1 */                      	\
	 { {6, 6, 6, 6}}}}},				/* pciSlot2 */                      	\
																				\
	0x68,									/* rtcTwsiAddr */                   \
    ADDR7_BIT,								/* rtcTwsiAddrType */               \
	N_A,									/* pexPciBridgeTwsiAddr */          \
	N_A,									/* pexPciBridgeTwsiAddrType */      \
																				\
	{0x8},									/* ethPhyAddr[MV_BOARD_MAX_PORTS] */\
																				\
	1,										/* rtcIntPin */                     \
	N_A,									/* switchIntPin */                  \
	{5,N_A},						/* vbusUsbGppPin[MV_BOARD_MAX_USB_IF] */	\
																				\
	4,										/* activeLedsNumber */              \
																				\
	{N_A,									/* led0GppPin */                    \
	N_A,									/* led1GppPin */                    \
	N_A,									/* led2GppPin */                    \
	N_A,									/* led3GppPin */                    \
	N_A,									/* led4GppPin */                    \
	N_A,									/* led5GppPin */                    \
	N_A,									/* led6GppPin */                    \
	N_A},									/* led7GppPin */                    \
	N_A,	   								/* ledsPolarity */					\
	0										/* refClkGppPin */					\
}

#define DB_88F5181_5281_DDR2_MPP0_7	       	0x00222203
#define DB_88F5181_5281_DDR2_MPP8_15	   	0x00001133
#define DB_88F5181_5281_DDR2_MPP16_23  		0x0
#define DB_88F5181_5281_DDR2_MPP_DEV		0x0

#define DB_88F5181_5281_DDR2_INFO	{											\
																				\
	"DB-88F5181-DDR2",						/* boardName[MAX_BOARD_NAME_LEN] */	\
																				\
	{DB_88F5181_5281_DDR2_MPP0_7,			/* mpp0_7 */						\
	DB_88F5181_5281_DDR2_MPP8_15,			/* mpp8_15 */                       \
	DB_88F5181_5281_DDR2_MPP16_23,			/* mpp16_23 */                      \
	DB_88F5181_5281_DDR2_MPP_DEV},			/* mppDev */                        \
																				\
	((1 << 10)|(1 << 12)|(1<<13)),				/* intsGppMask */                   \
																				\
	/*{params, devType, devWidth}*/			/* devCsInfo[MV_BOARD_MAX_DEV] */   \
																				\
	{{ 0x8fcfffff, N_A, N_A},				/* devCs0 */                        \
	 { 0x8fefffff, BOARD_DEV_NOR_FLASH, 16},	/* devCs1 */                    \
	 { 0x8fcfffff, N_A, N_A},				/* devCs2/flashCs */                \
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}},	/* bootCs */                    \
																				\
	/*pciBoardIf[MV_BOARD_MAX_PCI_IF];*/	  									\
																				\
	{{0x7,									/* firstSlotDevNum */               \
	3,										/* pciSlotsNum */                   \
																				\
	/*pciSlot[MV_BOARD_MAX_PCI_SLOTS]*/											\
	/* {{intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					\
																				\
	{{ {12, 12, 12, 12}},				/* pciSlot0 */							\
	 { {13, 13, 13, 13}},				/* pciSlot1 */                      	\
	 { {13, 13, 13, 13}}}}},			/* pciSlot2 */                      	\
																				\
	0x68,									/* rtcTwsiAddr */                   \
    ADDR7_BIT,								/* rtcTwsiAddrType */               \
	N_A,									/* pexPciBridgeTwsiAddr */          \
	N_A,									/* pexPciBridgeTwsiAddrType */      \
																				\
	{0x8},									/* ethPhyAddr[MV_BOARD_MAX_PORTS] */\
																				\
	10,									/* rtcIntPin */                     \
	N_A,									/* switchIntPin */                  \
	{1,N_A},						/* vbusUsbGppPin[MV_BOARD_MAX_USB_IF] */	\
																				\
	4,										/* activeLedsNumber */              \
																				\
	{14,									/* led0GppPin */                    \
	15,										/* led1GppPin */                    \
	6,										/* led2GppPin */                    \
	7,										/* led3GppPin */                    \
	N_A,									/* led4GppPin */                    \
	N_A,									/* led5GppPin */                    \
	N_A,									/* led6GppPin */                    \
	N_A},									/* led7GppPin */                    \
	1,										/* ledsPolarity */					\
	0										/* refClkGppPin */					\
}

#define DB_88F5181_DDR1_PRPMC_MPP0_7	    0x00000003
#define DB_88F5181_DDR1_PRPMC_MPP8_15	   	0x10001111
#define DB_88F5181_DDR1_PRPMC_MPP16_23  	0x00001111
#define DB_88F5181_DDR1_PRPMC_MPP_DEV		0x0


#define DB_88F5181_DDR1_PRPMC_INFO	{											\
																				\
	"DB-88F5181-DDR1-PRPMC",				/* boardName[MAX_BOARD_NAME_LEN] */	\
																				\
	{DB_88F5181_DDR1_PRPMC_MPP0_7,			/* mpp0_7 */						\
	DB_88F5181_DDR1_PRPMC_MPP8_15,			/* mpp8_15 */                       \
	DB_88F5181_DDR1_PRPMC_MPP16_23,			/* mpp16_23 */                      \
	DB_88F5181_DDR1_PRPMC_MPP_DEV}, 		/* mppDev */                        \
																				\
	((1 << 6)|(1 << 5)|(1<<4)|(1<<7)),		/* intsGppMask */                   \
																				\
	/*{params, devType, devWidth}*/			/* devCsInfo[MV_BOARD_MAX_DEV] */   \
																				\
	{{ 0x8fcfffff, N_A, N_A},				/* devCs0 */                        \
	 { 0x8fefffff, BOARD_DEV_NOR_FLASH, 16},	/* devCs1 */                    \
	 { 0x8fcfffff, N_A, N_A},				/* devCs2/flashCs */                \
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}},	/* bootCs */                    \
																				\
	/*pciBoardIf[MV_BOARD_MAX_PCI_IF];*/	  									\
																				\
	{{N_A,									/* firstSlotDevNum */               \
	N_A,									/* pciSlotsNum */                   \
																				\
	/*pciSlot[MV_BOARD_MAX_PCI_SLOTS]*/											\
	/* {{intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					\
																				\
	{{ {6, 5, 4, 7}},					/* pciSlot0 */							\
	 { {N_A, N_A, N_A, N_A}},			/* pciSlot1 */                      	\
	 { {N_A, N_A, N_A, N_A}}}}},		/* pciSlot2 */                      	\
																				\
	0x68,									/* rtcTwsiAddr */                   \
    ADDR7_BIT,								/* rtcTwsiAddrType */               \
	N_A,									/* pexPciBridgeTwsiAddr */          \
	N_A,									/* pexPciBridgeTwsiAddrType */      \
																				\
	{0x1f},									/* ethPhyAddr[MV_BOARD_MAX_PORTS] */\
																				\
	N_A,									/* rtcIntPin */                     \
	N_A,									/* switchIntPin */                  \
	{N_A,N_A},						/* vbusUsbGppPin[MV_BOARD_MAX_USB_IF] */	\
																				\
	3,										/* activeLedsNumber */              \
																				\
	{12,									/* led0GppPin */                    \
	13,										/* led1GppPin */                    \
	14,										/* led2GppPin */                    \
	N_A,									/* led3GppPin */                    \
	N_A,									/* led4GppPin */                    \
	N_A,									/* led5GppPin */                    \
	N_A,									/* led6GppPin */                    \
	N_A},									/* led7GppPin */                    \
	0,										/* ledsPolarity */					\
	0										/* refClkGppPin */					\
}


#define DB_88F5181_DDR1_PEXPCI_MPP0_7	    0x00330000
#define DB_88F5181_DDR1_PEXPCI_MPP8_15	   	0x0
#define DB_88F5181_DDR1_PEXPCI_MPP16_23  	0x0
#define DB_88F5181_DDR1_PEXPCI_MPP_DEV		0x0


#define DB_88F5181_DDR1_PEXPCI_INFO	{											\
																				\
	"DB-88F5181-DDR1-PEX_PCI",				/* boardName[MAX_BOARD_NAME_LEN] */	\
																				\
	{DB_88F5181_DDR1_PEXPCI_MPP0_7,			/* mpp0_7 */						\
	DB_88F5181_DDR1_PEXPCI_MPP8_15,			/* mpp8_15 */                       \
	DB_88F5181_DDR1_PEXPCI_MPP16_23,		/* mpp16_23 */                      \
	DB_88F5181_DDR1_PEXPCI_MPP_DEV},		/* mppDev */                        \
																				\
	((1 << 6)|(1 << 5)|(1<<4)|(1<<7)),		/* intsGppMask */                   \
																				\
	/*{params, devType, devWidth}*/			/* devCsInfo[MV_BOARD_MAX_DEV] */   \
																				\
	{{ 0x8fcfffff, N_A, N_A},				/* devCs0 */                        \
	 { 0x8fefffff, BOARD_DEV_NOR_FLASH, 16},	/* devCs1 */                    \
	 { 0x8fcfffff, N_A, N_A},				/* devCs2/flashCs */                \
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}},	/* bootCs */                    \
																				\
	/*pciBoardIf[MV_BOARD_MAX_PCI_IF];*/	  									\
																				\
	{{N_A,									/* firstSlotDevNum */               \
	N_A,									/* pciSlotsNum */                   \
																				\
	/*pciSlot[MV_BOARD_MAX_PCI_SLOTS]*/											\
	/* {{intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					\
																				\
	{{ {3, 6, 7, 7}},		   	   		 /* pciSlot0 */							\
	 { {N_A, N_A, N_A, N_A}},			/* pciSlot1 */                     	 	\
	 { {N_A, N_A, N_A, N_A}}}}},		/* pciSlot2 */                     	 	\
																				\
	0x68,									/* rtcTwsiAddr */                   \
    ADDR7_BIT,								/* rtcTwsiAddrType */               \
	N_A,									/* pexPciBridgeTwsiAddr */          \
	N_A,									/* pexPciBridgeTwsiAddrType */      \
																				\
	{0x8},									/* ethPhyAddr[MV_BOARD_MAX_PORTS] */\
																				\
	N_A,									/* rtcIntPin */                     \
	N_A,									/* switchIntPin */                  \
	{N_A,N_A},						/* vbusUsbGppPin[MV_BOARD_MAX_USB_IF] */	\
																				\
	4,										/* activeLedsNumber */              \
																				\
	{12,									/* led0GppPin */                    \
	13,										/* led1GppPin */                    \
	14,										/* led2GppPin */                    \
	15,							  		  	/* led3GppPin */                    \
	N_A,									/* led4GppPin */                    \
	N_A,									/* led5GppPin */                    \
	N_A,									/* led6GppPin */                    \
	N_A},									/* led7GppPin */                    \
	0,										/* ledsPolarity */					\
	0										/* refClkGppPin */					\
}

#define RD_88F5181_POS_NAS_MPP0_7	       	0x0
#define RD_88F5181_POS_NAS_MPP8_15			0x0
#define RD_88F5181_POS_NAS_MPP16_23  		0x0
#define RD_88F5181_POS_NAS_MPP_DEV			0x0


#define RD_88F5181_POS_NAS_INFO	{												\
																				\
	"RD-88F5181-88SX7042-2xSATA",			/* boardName[MAX_BOARD_NAME_LEN] */	\
																				\
	{RD_88F5181_POS_NAS_MPP0_7,				/* mpp0_7 */						\
	RD_88F5181_POS_NAS_MPP8_15, 			/* mpp8_15 */                       \
	RD_88F5181_POS_NAS_MPP16_23,			/* mpp16_23 */                      \
	RD_88F5181_POS_NAS_MPP_DEV},			/* mppDev */                        \
																				\
	((1 << 8)|(1 << 4)|(1 << 6)),			/* intsGppMask */                   \
																				\
	/*{params, devType, devWidth}*/			/* devCsInfo[MV_BOARD_MAX_DEV] */   \
																				\
	{{ 0x8fcfffff, N_A, N_A},				/* devCs0 */                        \
	 { 0x8fefffff, N_A, N_A},					/* devCs1 */                    \
	 { 0x8fcfffff, N_A, N_A},				/* devCs2/flashCs */                \
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}},	/* bootCs */                    \
																				\
	/*pciBoardIf[MV_BOARD_MAX_PCI_IF];*/	  									\
																				\
	{{0x7,									/* firstSlotDevNum */               \
	1,										/* pciSlotsNum */                   \
																				\
	/*pciSlot[MV_BOARD_MAX_PCI_SLOTS]*/											\
	/* {{intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					\
																				\
	{{ {6, 4, N_A, N_A}},				/* pciSlot0 */							\
	 { {N_A, N_A, N_A, N_A}},  			/* pciSlot1 */                      	\
	 { {N_A, N_A, N_A, N_A}}}}},		/* pciSlot2 */                      	\
																				\
	0x68,									/* rtcTwsiAddr */                   \
    ADDR7_BIT,								/* rtcTwsiAddrType */               \
	N_A,									/* pexPciBridgeTwsiAddr */          \
	N_A,									/* pexPciBridgeTwsiAddrType */      \
																				\
	{0x8},									/* ethPhyAddr[MV_BOARD_MAX_PORTS] */\
																				\
	8,										/* rtcIntPin */                     \
	N_A,									/* switchIntPin */                  \
	{9,N_A},						/* vbusUsbGppPin[MV_BOARD_MAX_USB_IF] */	\
																				\
	4,										/* activeLedsNumber */              \
																				\
	{12,									/* led0GppPin */                    \
	13,										/* led1GppPin */                    \
	14,										/* led2GppPin */                    \
	15,							 		   	/* led3GppPin */                    \
	N_A,									/* led4GppPin */                    \
	N_A,									/* led5GppPin */                    \
	N_A,									/* led6GppPin */                    \
	N_A},									/* led7GppPin */                    \
	0,										/* ledsPolarity */					\
	0										/* refClkGppPin */					\
}

#define DB_88F5X81_DDRX_MPP0_7	       	0x33222203
#define DB_88F5X81_DDRX_MPP8_15			0x44000033
#define DB_88F5X81_DDRX_MPP16_23  		0x0
#define DB_88F5X81_DDRX_MPP_DEV			0x0


#define DB_88F5X81_DDR2_INFO	{												\
																				\
	"DB-88F5X81-DDR2-A",					/* boardName[MAX_BOARD_NAME_LEN] */	\
																				\
	{DB_88F5X81_DDRX_MPP0_7,				/* mpp0_7 */						\
	DB_88F5X81_DDRX_MPP8_15,				/* mpp8_15 */                       \
	DB_88F5X81_DDRX_MPP16_23,				/* mpp16_23 */                      \
	DB_88F5X81_DDRX_MPP_DEV},				/* mppDev */                        \
																				\
	((1<<10)|(1 << 12)|(1<<13)),				/* intsGppMask */                   \
																				\
	/*{params, devType, devWidth}*/			/* devCsInfo[MV_BOARD_MAX_DEV] */   \
																				\
	{{ 0x8fcfffff, BOARD_DEV_SEVEN_SEG, N_A}, 	/* devCs0 */                        \
	 { 0x8fefffff, BOARD_DEV_NOR_FLASH, 16},   	/* devCs1 */                    \
	 { 0x8fcfffff, BOARD_DEV_NAND_FLASH, 8},	/* devCs2/flashCs */            \
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}},	/* bootCs */                    \
																				\
	/*pciBoardIf[MV_BOARD_MAX_PCI_IF];*/	  									\
																				\
	{{0x7,									/* firstSlotDevNum */               \
	3,										/* pciSlotsNum */                   \
																				\
	/*pciSlot[MV_BOARD_MAX_PCI_SLOTS]*/											\
	/* {{intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					\
																				\
	{{ {12, 12, 12, 12}},				/* pciSlot0 */							\
	 { {13, 13, 13, 13}},				/* pciSlot1 */                      	\
	 { {13, 13, 13, 13}}}}},			/* pciSlot2 */                      	\
																				\
	0x68,									/* rtcTwsiAddr */                   \
    ADDR7_BIT,								/* rtcTwsiAddrType */               \
	N_A,									/* pexPciBridgeTwsiAddr */          \
	N_A,									/* pexPciBridgeTwsiAddrType */      \
																				\
	{0x8},									/* ethPhyAddr[MV_BOARD_MAX_PORTS] */\
																				\
	10,										/* rtcIntPin */                     \
	N_A,									/* switchIntPin */                  \
	{1, N_A},						/* vbusUsbGppPin[MV_BOARD_MAX_USB_IF] */	\
																				\
	4,										/* activeLedsNumber */              \
																				\
	{N_A,									/* led0GppPin */                    \
	N_A,									/* led1GppPin */                    \
	N_A,									/* led2GppPin */                    \
	N_A,									/* led3GppPin */                    \
	N_A,									/* led4GppPin */                    \
	N_A,									/* led5GppPin */                    \
	N_A,									/* led6GppPin */                    \
	N_A},									/* led7GppPin */                    \
	N_A,	   								/* ledsPolarity */					\
	11										/* refClkGppPin */					\
}


#define DB_88F5X81_DDR1_INFO	{												\
																				\
	"DB-88F5X81-DDR1-A",					/* boardName[MAX_BOARD_NAME_LEN] */	\
																				\
	{DB_88F5X81_DDRX_MPP0_7,				/* mpp0_7 */						\
	DB_88F5X81_DDRX_MPP8_15,				/* mpp8_15 */                       \
	DB_88F5X81_DDRX_MPP16_23,				/* mpp16_23 */                      \
	DB_88F5X81_DDRX_MPP_DEV},				/* mppDev */                        \
																				\
	((1<<10)|(1 << 12)|(1<<13)),				/* intsGppMask */                   \
																				\
	/*{params, devType, devWidth}*/			/* devCsInfo[MV_BOARD_MAX_DEV] */   \
																				\
	{{ 0x8fcfffff, BOARD_DEV_SEVEN_SEG, N_A}, 	/* devCs0 */                        \
	 { 0x8fefffff, BOARD_DEV_NOR_FLASH, 16},   	/* devCs1 */                    \
	 { 0x8fcfffff, BOARD_DEV_NAND_FLASH, 8},	/* devCs2/flashCs */            \
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}},	/* bootCs */                    \
																				\
	/*pciBoardIf[MV_BOARD_MAX_PCI_IF];*/	  									\
																				\
	{{0x7,									/* firstSlotDevNum */               \
	3,										/* pciSlotsNum */                   \
																				\
	/*pciSlot[MV_BOARD_MAX_PCI_SLOTS]*/											\
	/* {{intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					\
																				\
	{{ {12, 12, 12, 12}},				/* pciSlot0 */							\
	 { {13, 13, 13, 13}},				/* pciSlot1 */                      	\
	 { {13, 13, 13, 13}}}}},			/* pciSlot2 */                      	\
																				\
	0x68,									/* rtcTwsiAddr */                   \
    ADDR7_BIT,								/* rtcTwsiAddrType */               \
	N_A,									/* pexPciBridgeTwsiAddr */          \
	N_A,									/* pexPciBridgeTwsiAddrType */      \
																				\
	{0x8},									/* ethPhyAddr[MV_BOARD_MAX_PORTS] */\
																				\
	10,										/* rtcIntPin */                     \
	N_A,									/* switchIntPin */                  \
	{1, N_A},						/* vbusUsbGppPin[MV_BOARD_MAX_USB_IF] */	\
																				\
	4,										/* activeLedsNumber */              \
																				\
	{N_A,									/* led0GppPin */                    \
	N_A,									/* led1GppPin */                    \
	N_A,									/* led2GppPin */                    \
	N_A,									/* led3GppPin */                    \
	N_A,									/* led4GppPin */                    \
	N_A,									/* led5GppPin */                    \
	N_A,									/* led6GppPin */                    \
	N_A},									/* led7GppPin */                    \
	N_A,	   								/* ledsPolarity */					\
	11										/* refClkGppPin */					\
}

#define RD_88F5181_VOIP_MPP0_7	       	0x00000003
#define RD_88F5181_VOIP_MPP8_15			0x00000101
#define RD_88F5181_VOIP_MPP16_23  		0x0
#define RD_88F5181_VOIP_MPP_DEV			0x0


#define RD_88F5181_VOIP_INFO	{												\
																				\
	"RD-88F5181-VOIP-RD1",					/* boardName[MAX_BOARD_NAME_LEN] */	\
																				\
	{RD_88F5181_VOIP_MPP0_7,				/* mpp0_7 */						\
	RD_88F5181_VOIP_MPP8_15,				/* mpp8_15 */                       \
	RD_88F5181_VOIP_MPP16_23,				/* mpp16_23 */                      \
	RD_88F5181_VOIP_MPP_DEV},				/* mppDev */                        \
																				\
	((1<<3)|(1<<4)|(1 << 6)|(1<<7)),		/* intsGppMask */                   \
																				\
	/*{params, devType, devWidth}*/			/* devCsInfo[MV_BOARD_MAX_DEV] */   \
																				\
	{{ 0x8fdfffff, BOARD_DEV_FPGA, N_A}, 		/* devCs0 */                    \
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8},   	/* devCs1 */                    \
	 { 0x8fdfffff, BOARD_DEV_FPGA, N_A},  		/* devCs2/flashCs */            \
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}},	/* bootCs */                    \
																				\
	/*pciBoardIf[MV_BOARD_MAX_PCI_IF];*/	  									\
																				\
	{{1,									/* firstSlotDevNum */               \
	1,										/* pciSlotsNum */                   \
																				\
	/*pciSlot[MV_BOARD_MAX_PCI_SLOTS]*/											\
	/* {{intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					\
																				\
	{{ {4, 4, 4, 4}},	   		 		/* pciSlot0 */							\
	 { {N_A, N_A, N_A, N_A}},  			/* pciSlot1 */                      	\
	 { {N_A, N_A, N_A, N_A}}}}},		/* pciSlot2 */                      	\
																				\
	0x68,									/* rtcTwsiAddr */                   \
    ADDR7_BIT,								/* rtcTwsiAddrType */               \
	N_A,									/* pexPciBridgeTwsiAddr */          \
	N_A,									/* pexPciBridgeTwsiAddrType */      \
																				\
	{0x0},									/* ethPhyAddr[MV_BOARD_MAX_PORTS] */\
																				\
	7,										/* rtcIntPin */                     \
	6,	  		  							/* switchIntPin */                  \
	{2,N_A},						/* vbusUsbGppPin[MV_BOARD_MAX_USB_IF] */	\
																				\
	3,										/* activeLedsNumber */              \
																				\
	{13,								   	/* led0GppPin */                    \
	14,										/* led1GppPin */                    \
	15,	  	  								/* led2GppPin */                    \
	N_A,									/* led3GppPin */                    \
	N_A,									/* led4GppPin */                    \
	N_A,									/* led5GppPin */                    \
	N_A,									/* led6GppPin */                    \
	N_A},									/* led7GppPin */                    \
	0,		   								/* ledsPolarity */					\
	N_A										/* refClkGppPin */					\
}


#define DB_88F5182_DDR2_MPP0_7	       	0x55222203
#define DB_88F5182_DDR2_MPP8_15			0x44550000
#define DB_88F5182_DDR2_MPP16_23  		0x0


#define DB_88F5182_DDR2_INFO	{												\
																				\
	"DB-88F5182-DDR2",					/* boardName[MAX_BOARD_NAME_LEN] */	\
																				\
	{DB_88F5182_DDR2_MPP0_7,				/* mpp0_7 */						\
	DB_88F5182_DDR2_MPP8_15,				/* mpp8_15 */                       \
	DB_88F5182_DDR2_MPP16_23,				/* mpp16_23 */                      \
	N_A},									/* mppDev */                        \
																				\
	((1<<0)|(1 << 1)|(1<<10)),				/* intsGppMask */                   \
																				\
	/*{params, devType, devWidth}*/			/* devCsInfo[MV_BOARD_MAX_DEV] */   \
																				\
	{{ 0x8fcfffff, BOARD_DEV_SEVEN_SEG, N_A}, 	/* devCs0 */                        \
	 { 0x8fdfffff, BOARD_DEV_NOR_FLASH, 16},   	/* devCs1 */                    \
	 { 0x8fcfffff, BOARD_DEV_NAND_FLASH, 8},	/* devCs2/flashCs */            \
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}},	/* bootCs */                    \
																				\
	/*pciBoardIf[MV_BOARD_MAX_PCI_IF];*/	  									\
																				\
	{{0x7,									/* firstSlotDevNum */               \
	3,										/* pciSlotsNum */                   \
																				\
	/*pciSlot[MV_BOARD_MAX_PCI_SLOTS]*/											\
	/* {{intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					\
																				\
	{{ {0, 0, 0, 0}},					/* pciSlot0 */							\
	 { {1, 1, 1, 1}}, 	  				/* pciSlot1 */                      	\
	 { {1, 1, 1, 1}}}}}, 	   			/* pciSlot2 */                      	\
																				\
	0x68,									/* rtcTwsiAddr */                   \
    ADDR7_BIT,								/* rtcTwsiAddrType */               \
	N_A,									/* pexPciBridgeTwsiAddr */          \
	N_A,									/* pexPciBridgeTwsiAddrType */      \
																				\
	{0x8},									/* ethPhyAddr[MV_BOARD_MAX_PORTS] */\
																				\
	10,										/* rtcIntPin */                     \
	N_A,									/* switchIntPin */                  \
	{8,9},							/* vbusUsbGppPin[MV_BOARD_MAX_USB_IF] */	\
																				\
	4,										/* activeLedsNumber */              \
																				\
	{N_A,									/* led0GppPin */                    \
	N_A,									/* led1GppPin */                    \
	N_A,									/* led2GppPin */                    \
	N_A,									/* led3GppPin */                    \
	N_A,									/* led4GppPin */                    \
	N_A,									/* led5GppPin */                    \
	N_A,									/* led6GppPin */                    \
	N_A},									/* led7GppPin */                    \
	N_A,	   								/* ledsPolarity */					\
	11										/* refClkGppPin */					\
}


#define RD_88F5182_2XSATA_MPP0_7	       	0x00000003
#define RD_88F5182_2XSATA_MPP8_15			0x55550000
#define RD_88F5182_2XSATA_MPP16_23  		0x5555

#define RD_88F5182_2XSATA_INFO	{												\
																				\
	"RD-88F5182-NAS-2",					/* boardName[MAX_BOARD_NAME_LEN] */	\
																				\
	{RD_88F5182_2XSATA_MPP0_7,				/* mpp0_7 */						\
	RD_88F5182_2XSATA_MPP8_15,				/* mpp8_15 */                       \
	RD_88F5182_2XSATA_MPP16_23,				/* mpp16_23 */                      \
	N_A},									/* mppDev */                        \
																				\
	((1<<3)|(1 << 6)|(1<<7)),				/* intsGppMask */                   \
																				\
	/*{params, devType, devWidth}*/			/* devCsInfo[MV_BOARD_MAX_DEV] */   \
																				\
	{{ N_A, N_A, N_A}, 						/* devCs0 */                        \
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8},	/* devCs1 */                    \
	 { N_A, N_A, N_A},							/* devCs2/flashCs */            \
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}},	/* bootCs */                    \
																				\
	/*pciBoardIf[MV_BOARD_MAX_PCI_IF];*/	  									\
																				\
	{{0x7,									/* firstSlotDevNum */               \
	1,										/* pciSlotsNum */                   \
																				\
	/*pciSlot[MV_BOARD_MAX_PCI_SLOTS]*/											\
	/* {{intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					\
																				\
	{{ {7, 6, N_A, N_A}},		   		/* pciSlot0 */							\
	 { {N_A, N_A, N_A, N_A}},			/* pciSlot1 */                      	\
	 { {N_A, N_A, N_A, N_A}}}}},		/* pciSlot2 */                      	\
																				\
	0x68,									/* rtcTwsiAddr */                   \
    ADDR7_BIT,								/* rtcTwsiAddrType */               \
	N_A,									/* pexPciBridgeTwsiAddr */          \
	N_A,									/* pexPciBridgeTwsiAddrType */      \
																				\
	{0x8},									/* ethPhyAddr[MV_BOARD_MAX_PORTS] */\
																				\
	3,										/* rtcIntPin */                     \
	N_A,									/* switchIntPin */                  \
	{N_A,N_A},						/* vbusUsbGppPin[MV_BOARD_MAX_USB_IF] */	\
																				\
	1,										/* activeLedsNumber */              \
																				\
	{0,										/* led0GppPin */                    \
	N_A, 		 								/* led1GppPin */                \
	N_A,									/* led2GppPin */                    \
	N_A,									/* led3GppPin */                    \
	N_A,									/* led4GppPin */                    \
	N_A,									/* led5GppPin */                    \
	N_A,									/* led6GppPin */                    \
	N_A},									/* led7GppPin */                    \
	0,		   								/* ledsPolarity */					\
	N_A										/* refClkGppPin */					\
}


#define DB_88F5181L_DDR2_2XTDM_MPP0_7	   	0x00222203
#define DB_88F5181L_DDR2_2XTDM_MPP8_15	   	0x44000000
#define DB_88F5181L_DDR2_2XTDM_MPP16_23 	0x0
#define DB_88F5181L_DDR2_2XTDM_MPP_DEV		0x0


#define DB_88F5181L_DDR2_2XTDM_INFO	{											\
																				\
	"DB-88F5181L-DDR2-2xTDM",				/* boardName[MAX_BOARD_NAME_LEN] */	\
																				\
	{DB_88F5181L_DDR2_2XTDM_MPP0_7,			/* mpp0_7 */						\
	DB_88F5181L_DDR2_2XTDM_MPP8_15,			/* mpp8_15 */                       \
	DB_88F5181L_DDR2_2XTDM_MPP16_23,		/* mpp16_23 */                      \
	DB_88F5181L_DDR2_2XTDM_MPP_DEV},		/* mppDev */                        \
																				\
	((1<<8)|(1 << 9)|(1<<10)|(1<<12)|(1<<13)), 	/* intsGppMask */               \
																				\
	/*{params, devType, devWidth}*/			/* devCsInfo[MV_BOARD_MAX_DEV] */   \
																				\
	{{ 0x8fcfffff, BOARD_DEV_SEVEN_SEG, N_A}, 	/* devCs0 */                        \
	 { 0x8fdfffff, BOARD_DEV_NOR_FLASH, 16},   	/* devCs1 */                    \
	 { 0x8fcfffff, BOARD_DEV_NAND_FLASH, 8},	/* devCs2/flashCs */            \
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}},	/* bootCs */                    \
																				\
	/*pciBoardIf[MV_BOARD_MAX_PCI_IF];*/	  									\
																				\
	{{0x7,									/* firstSlotDevNum */               \
	3,										/* pciSlotsNum */                   \
																				\
	/*pciSlot[MV_BOARD_MAX_PCI_SLOTS]*/											\
	/* {{intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					\
																				\
	{{ {12, 12, 12, 12}},				/* pciSlot0 */							\
	 { {13, 13, 13, 13}},  				/* pciSlot1 */                      	\
	 { {13, 13, 13, 13}}}}},   			/* pciSlot2 */                      	\
																				\
	0x68,									/* rtcTwsiAddr */                   \
    ADDR7_BIT,								/* rtcTwsiAddrType */               \
	N_A,									/* pexPciBridgeTwsiAddr */          \
	N_A,									/* pexPciBridgeTwsiAddrType */      \
																				\
	{0x8},									/* ethPhyAddr[MV_BOARD_MAX_PORTS] */\
																				\
	10,										/* rtcIntPin */                     \
	N_A,									/* switchIntPin */                  \
	{1,N_A},						/* vbusUsbGppPin[MV_BOARD_MAX_USB_IF] */	\
																				\
	4,										/* activeLedsNumber */              \
																				\
	{N_A,									/* led0GppPin */                    \
	N_A,									/* led1GppPin */                    \
	N_A,									/* led2GppPin */                    \
	N_A,									/* led3GppPin */                    \
	N_A,									/* led4GppPin */                    \
	N_A,									/* led5GppPin */                    \
	N_A,									/* led6GppPin */                    \
	N_A},									/* led7GppPin */                    \
	N_A,	   								/* ledsPolarity */					\
	11										/* refClkGppPin */					\
}


#define RD_88F5181L_VOIP_FE_MPP0_7	    0x55000003
#define RD_88F5181L_VOIP_FE_MPP8_15	   	0x00000101
#define RD_88F5181L_VOIP_FE_MPP16_23  	0x0
#define RD_88F5181L_VOIP_FE_MPP_DEV		0x0


#define RD_88F5181L_VOIP_FE_INFO	{											\
																				\
	"RD-88F5181L-VOIP-FE",					/* boardName[MAX_BOARD_NAME_LEN] */	\
																				\
	{RD_88F5181L_VOIP_FE_MPP0_7,			/* mpp0_7 */						\
	RD_88F5181L_VOIP_FE_MPP8_15,			/* mpp8_15 */                       \
	RD_88F5181L_VOIP_FE_MPP16_23,			/* mpp16_23 */                      \
	RD_88F5181L_VOIP_FE_MPP_DEV},			/* mppDev */                        \
																				\
	((1<<2)|(1 << 3)|(1<<4)|(1<<5)|(1<<9)|(1<<11)),	/* intsGppMask */           \
																				\
	/*{params, devType, devWidth}*/			/* devCsInfo[MV_BOARD_MAX_DEV] */   \
																				\
	{{ N_A, N_A, N_A}, 						/* devCs0 */                        \
	 { N_A, N_A, N_A},							/* devCs1 */                    \
	 { N_A, N_A, N_A},							/* devCs2/flashCs */            \
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}},	/* bootCs */                    \
																				\
	/*pciBoardIf[MV_BOARD_MAX_PCI_IF];*/	  									\
																				\
	{{0x7,									/* firstSlotDevNum */               \
	1,										/* pciSlotsNum */                   \
																				\
	/*pciSlot[MV_BOARD_MAX_PCI_SLOTS]*/											\
	/* {{intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					\
																				\
	{{ {4, 3, N_A, N_A}},		   		/* pciSlot0 */							\
	 { {N_A, N_A, N_A, N_A}},			/* pciSlot1 */                      	\
	 { {N_A, N_A, N_A, N_A}}}}},		/* pciSlot2 */                      	\
																				\
	0x68,									/* rtcTwsiAddr */                   \
    ADDR7_BIT,								/* rtcTwsiAddrType */               \
	N_A,									/* pexPciBridgeTwsiAddr */          \
	N_A,									/* pexPciBridgeTwsiAddrType */      \
																				\
	{0x0},									/* ethPhyAddr[MV_BOARD_MAX_PORTS] */\
																				\
	11,										/* rtcIntPin */                     \
	N_A,									/* switchIntPin */                  \
	{N_A,N_A},						/* vbusUsbGppPin[MV_BOARD_MAX_USB_IF] */	\
																				\
	3,										/* activeLedsNumber */              \
																				\
	{12,										/* led0GppPin */                    \
	13, 		 								/* led1GppPin */                    \
	14,									/* led2GppPin */                    \
	N_A,									/* led3GppPin */                    \
	N_A,									/* led4GppPin */                    \
	N_A,									/* led5GppPin */                    \
	N_A,									/* led6GppPin */                    \
	N_A},									/* led7GppPin */                    \
	0,		   								/* ledsPolarity */					\
	N_A										/* refClkGppPin */					\
}


#ifdef CONFIG_SCM_SUPPORT
#define RD_88F5181L_VOIP_GE_MPP0_7	    0x00000003
#else
#define RD_88F5181L_VOIP_GE_MPP0_7	    0x55000003
#endif
#define RD_88F5181L_VOIP_GE_MPP8_15	   	0x11110010
#define RD_88F5181L_VOIP_GE_MPP16_23  	0x1111
#define RD_88F5181L_VOIP_GE_MPP_DEV		0x0


#define RD_88F5181L_VOIP_GE_INFO	{											\
																				\
	"RD-88F5181L-VOIP-GE",					/* boardName[MAX_BOARD_NAME_LEN] */	\
																				\
	{RD_88F5181L_VOIP_GE_MPP0_7,			/* mpp0_7 */						\
	RD_88F5181L_VOIP_GE_MPP8_15,			/* mpp8_15 */                       \
	RD_88F5181L_VOIP_GE_MPP16_23,			/* mpp16_23 */                      \
	RD_88F5181L_VOIP_GE_MPP_DEV},			/* mppDev */                        \
																				\
	((1<<4)|(1 << 5)|(1<<8)),				/* intsGppMask */                   \
																				\
	/*{params, devType, devWidth}*/			/* devCsInfo[MV_BOARD_MAX_DEV] */   \
																				\
	{{ N_A, N_A, N_A}, 						/* devCs0 */                        \
	 { N_A, N_A, N_A},							/* devCs1 */                    \
	 { N_A, N_A, N_A},							/* devCs2/flashCs */            \
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}},	/* bootCs */                    \
																				\
	/*pciBoardIf[MV_BOARD_MAX_PCI_IF];*/	  									\
																				\
	{{0x7,									/* firstSlotDevNum */               \
	1,										/* pciSlotsNum */                   \
																				\
	/*pciSlot[MV_BOARD_MAX_PCI_SLOTS]*/											\
	/* {{intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					\
																				\
	{{ {4, 10, N_A, N_A}},		   		/* pciSlot0 */							\
	 { {N_A, N_A, N_A, N_A}},			/* pciSlot1 */                      	\
	 { {N_A, N_A, N_A, N_A}}}}},		/* pciSlot2 */                      	\
																				\
	0x68,									/* rtcTwsiAddr */                   \
    ADDR7_BIT,								/* rtcTwsiAddrType */               \
	N_A,									/* pexPciBridgeTwsiAddr */          \
	N_A,									/* pexPciBridgeTwsiAddrType */      \
																				\
	{0x0},									/* ethPhyAddr[MV_BOARD_MAX_PORTS] */\
																				\
	5,										/* rtcIntPin */                     \
	N_A,									/* switchIntPin */                  \
	{N_A,N_A},						/* vbusUsbGppPin[MV_BOARD_MAX_USB_IF] */	\
																				\
	3,										/* activeLedsNumber */              \
																				\
	{1,										/* led0GppPin */                    \
	2, 		 								/* led1GppPin */                    \
	3,										/* led2GppPin */                    \
	N_A,									/* led3GppPin */                    \
	N_A,									/* led4GppPin */                    \
	N_A,									/* led5GppPin */                    \
	N_A,									/* led6GppPin */                    \
	N_A},									/* led7GppPin */                    \
	0,		   								/* ledsPolarity */					\
	N_A										/* refClkGppPin */					\
}


#define RD_88F5182_2XSATA3_MPP0_7	       	0x00000003
#define RD_88F5182_2XSATA3_MPP8_15			0x55550000
#define RD_88F5182_2XSATA3_MPP16_23  		0x5555

#define RD_88F5182_2XSATA3_INFO	{												\
																				\
	"RD-88F5182-NAS-3",					/* boardName[MAX_BOARD_NAME_LEN] */	\
																				\
	{RD_88F5182_2XSATA3_MPP0_7,				/* mpp0_7 */						\
	RD_88F5182_2XSATA3_MPP8_15,				/* mpp8_15 */                       \
	RD_88F5182_2XSATA3_MPP16_23,				/* mpp16_23 */                      \
	N_A},									/* mppDev */                        \
																				\
	((1<<3)|(1 << 6)|(1<<7)),				/* intsGppMask */                   \
																				\
	/*{params, devType, devWidth}*/			/* devCsInfo[MV_BOARD_MAX_DEV] */   \
																				\
	{{ N_A, N_A, N_A}, 						/* devCs0 */                        \
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8},	/* devCs1 */                    \
	 { N_A, N_A, N_A},							/* devCs2/flashCs */            \
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}},	/* bootCs */                    \
																				\
	/*pciBoardIf[MV_BOARD_MAX_PCI_IF];*/	  									\
																				\
	{{0x7,									/* firstSlotDevNum */               \
	1,										/* pciSlotsNum */                   \
																				\
	/*pciSlot[MV_BOARD_MAX_PCI_SLOTS]*/											\
	/* {{intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					\
																				\
	{{ {6, N_A, N_A, N_A}},		   		/* pciSlot0 */							\
	 { {N_A, N_A, N_A, N_A}},			/* pciSlot1 */                      	\
	 { {N_A, N_A, N_A, N_A}}}}},		/* pciSlot2 */                      	\
																				\
	0x68,									/* rtcTwsiAddr */                   \
    ADDR7_BIT,								/* rtcTwsiAddrType */               \
	N_A,									/* pexPciBridgeTwsiAddr */          \
	N_A,									/* pexPciBridgeTwsiAddrType */      \
																				\
	{0x8},									/* ethPhyAddr[MV_BOARD_MAX_PORTS] */\
																				\
	3,										/* rtcIntPin */                     \
	N_A,									/* switchIntPin */                  \
	{N_A,N_A},						/* vbusUsbGppPin[MV_BOARD_MAX_USB_IF] */	\
																				\
	2,										/* activeLedsNumber */              \
																				\
	{0,										/* led0GppPin */                    \
	7, 		 								/* led1GppPin */                    \
	N_A,									/* led2GppPin */                    \
	N_A,									/* led3GppPin */                    \
	N_A,									/* led4GppPin */                    \
	N_A,									/* led5GppPin */                    \
	N_A,									/* led6GppPin */                    \
	N_A},									/* led7GppPin */                    \
	1,		   								/* ledsPolarity */					\
	N_A										/* refClkGppPin */					\
}




#define RD_88W8660_MPP0_7	    	0x0 
#define RD_88W8660_MPP8_15	   	0x0


#define RD_88W8660_DDR1_INFO	{											\
															\
	"RD-88W8660",								/* boardName[MAX_BOARD_NAME_LEN] */	\
															\
	{RD_88W8660_MPP0_7,							/* mpp0_7 */				\
	RD_88W8660_MPP8_15,							/* mpp8_15 */                       	\
	N_A,									/* mpp16_23 */                      	\
	N_A},									/* mppDev */                        	\
															\
	((1 << 3)|(1<<4)|(1<<9)|(1<<11)),					/* intsGppMask */           		\
															\
	/*{params, devType, devWidth}*/						/* devCsInfo[MV_BOARD_MAX_DEV] */   	\
															\
	{{ N_A, N_A, N_A}, 							/* devCs0 */                        	\
	 { N_A, N_A, N_A},							/* devCs1 */                    	\
	 { N_A, N_A, N_A},							/* devCs2/flashCs */            	\
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}},				/* bootCs */		           	\
															\
	/*pciBoardIf[MV_BOARD_MAX_PCI_IF];*/	  									\
															\
	{{0x7,									/* firstSlotDevNum */               	\
	1,									/* pciSlotsNum */                   	\
															\
	/*pciSlot[MV_BOARD_MAX_PCI_SLOTS]*/										\
	/* {{intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */							\
															\
	{{ {4, 3, N_A, N_A}},		   					/* pciSlot0 */				\
	 { {N_A, N_A, N_A, N_A}},						/* pciSlot1 */                      	\
	 { {N_A, N_A, N_A, N_A}}}}},						/* pciSlot2 */                      	\
															\
	0x68,									/* rtcTwsiAddr */                   	\
    	ADDR7_BIT,								/* rtcTwsiAddrType */               	\
	N_A,									/* pexPciBridgeTwsiAddr */          	\
	N_A,									/* pexPciBridgeTwsiAddrType */      	\
															\
	{0x0},									/* ethPhyAddr[MV_BOARD_MAX_PORTS] */	\
															\
	11,									/* rtcIntPin */                     	\
	N_A,									/* switchIntPin */                  	\
	{N_A,N_A},								/* vbusUsbGppPin[MV_BOARD_MAX_USB_IF] */\
															\
	3,									/* activeLedsNumber */              	\
															\
	{6,									/* led0GppPin */                    	\
	5, 		 							/* led1GppPin */                    	\
	7,									/* led2GppPin */                    	\
	N_A,									/* led3GppPin */                    	\
	N_A,									/* led4GppPin */                    	\
	N_A,									/* led5GppPin */                    	\
	N_A,									/* led6GppPin */                    	\
	N_A},									/* led7GppPin */                    	\
	0,		   							/* ledsPolarity */			\
	N_A									/* refClkGppPin */			\
}


#define DB_88W8660_DDR2_MPP0_7	       		0x00002222
#define DB_88W8660_DDR2_MPP8_15			0x00000002

#define DB_88W8660_DDR2_INFO	{											\
															\
	"DB-88W8660-DDR2",							/* boardName[MAX_BOARD_NAME_LEN] */	\
															\
	{DB_88W8660_DDR2_MPP0_7,						/* mpp0_7 */				\
	DB_88W8660_DDR2_MPP8_15,						/* mpp8_15 */                       	\
	N_A,									/* mpp16_23 */                      	\
	N_A},									/* mppDev */                        	\
															\
	((1<<9)|(1 << 11)|(1<<10)),						/* intsGppMask */                   	\
															\
	/*{params, devType, devWidth}*/						/* devCsInfo[MV_BOARD_MAX_DEV] */   	\
															\
	{{ 0x8fcfffff, BOARD_DEV_SEVEN_SEG, N_A}, 				/* devCs0 */                        	\
	 { 0x8fdfffff, BOARD_DEV_NOR_FLASH, 16},   				/* devCs1 */                    	\
	 { 0x8fcfffff, BOARD_DEV_NAND_FLASH, 8},				/* devCs2/flashCs */            	\
	 { 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}},				/* bootCs */                    	\
															\
	/*pciBoardIf[MV_BOARD_MAX_PCI_IF];*/	  									\
															\
	{{0x7,									/* firstSlotDevNum */               	\
	3,									/* pciSlotsNum */                   	\
															\
	/*pciSlot[MV_BOARD_MAX_PCI_SLOTS]*/										\
	/* {{intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */							\
															\
	{{ {0, 0, 0, 0}},							/* pciSlot0 */				\
	 { { 9,  9,  9,  9}}, 	  						/* pciSlot1 */                      	\
	 { {11, 11, 11, 11}}}}}, 	   					/* pciSlot2 */                      	\
															\
	0x68,									/* rtcTwsiAddr */                   	\
    	ADDR7_BIT,								/* rtcTwsiAddrType */               	\
	N_A,									/* pexPciBridgeTwsiAddr */          	\
	N_A,									/* pexPciBridgeTwsiAddrType */      	\
															\
	{0x8},									/* ethPhyAddr[MV_BOARD_MAX_PORTS] */	\
															\
	10,									/* rtcIntPin */                     	\
	N_A,									/* switchIntPin */                  	\
	{N_A,N_A},								/* vbusUsbGppPin[MV_BOARD_MAX_USB_IF] */\
															\
	4,									/* activeLedsNumber */              	\
														    	\
	{N_A,									/* led0GppPin */                    	\
	N_A,									/* led1GppPin */                    	\
	N_A,									/* led2GppPin */                    	\
	N_A,									/* led3GppPin */                    	\
	N_A,									/* led4GppPin */                    	\
	N_A,									/* led5GppPin */                    	\
	N_A,									/* led6GppPin */                    	\
	N_A},									/* led7GppPin */                    	\
	N_A,	   								/* ledsPolarity */			\
	N_A									/* refClkGppPin */			\
}


#endif /* __INCmvBoardEnvSpech */
