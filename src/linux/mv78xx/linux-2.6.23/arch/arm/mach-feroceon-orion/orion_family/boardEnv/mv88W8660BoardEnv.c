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


#define DB_88W8660_BOARD_PCI_IF_NUM		0x3
#define DB_88W8660_BOARD_TWSI_DEF_NUM		0x1
#define DB_88W8660_BOARD_MAC_INFO_NUM		0x1
#define DB_88W8660_BOARD_GPP_INFO_NUM		0x1
#define DB_88W8660_BOARD_DEBUG_LED_NUM		0x0
#define DB_88W8660_BOARD_MPP_CONFIG_NUM		0x2
#define DB_88W8660_BOARD_DEVICE_CONFIG_NUM	0x4

MV_BOARD_PCI_IF db88w8660ddr2InfoBoardPciIf[DB_88W8660_BOARD_PCI_IF_NUM] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	{{7, {0, 0, 0, 0}},				/* pciSlot0 */							
	 {8, {9, 9, 9, 9}},				/* pciSlot1 */                      	
	 {9, {11, 11, 11, 11}}};			/* pciSlot2 */                      

MV_BOARD_TWSI_INFO db88w8660ddr2InfoBoardTwsiDev[DB_88W8660_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO db88w8660ddr2InfoBoardMacInfo[DB_88W8660_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8}}; 

MV_BOARD_GPP_INFO db88w8660ddr2InfoBoardGppInfo[DB_88W8660_BOARD_GPP_INFO_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RTC, 10}};

MV_BOARD_MPP_INFO db88w8660ddr2InfoBoardMppConfigValue[DB_88W8660_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{DB_88W8660_DDR2_MPP0_7,			/* mpp0_7 */
	DB_88W8660_DDR2_MPP8_15,			/* mpp8_15 */
	N_A,
	N_A}},
	{{DB_88W8660_DDR2_MPP0_7NB,			/* mpp0_7 */
	DB_88W8660_DDR2_MPP8_15NB,			/* mpp8_15 */
	N_A,
	N_A}}
	};

MV_DEV_CS_INFO db88w8660ddr2InfoBoardDeCsInfo[DB_88W8660_BOARD_DEVICE_CONFIG_NUM] =
#if defined(MV_NAND_BOOT)
		/*{params, devType, devWidth}*/			   
		{{0, 0x8fcfffff, BOARD_DEV_SEVEN_SEG, N_A}, 	/* devCs0 */                       
		{1, 0x8fdfffff, BOARD_DEV_NOR_FLASH, 16},   	/* devCs1 */                    
		{2, N_A, N_A},	
		{3,0x8fcfffff, BOARD_DEV_NAND_FLASH, 8}};	/* bootCs */		   
#else 
		/*{params, devType, devWidth}*/			   
		{{0, 0x8fcfffff, BOARD_DEV_SEVEN_SEG, N_A}, 	/* devCs0 */                       
		{1, 0x8fdfffff, BOARD_DEV_NOR_FLASH, 16},   	/* devCs1 */                    
		{2, N_A, N_A},					/* devCs2/flashCs */            
		{3, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};		/* bootCs */                    
#endif
MV_BOARD_INFO db88w8660ddr2Info = {
	"DB-88W8660-DDR2",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88W8660_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88w8660ddr2InfoBoardMppConfigValue,
	((1<<9)|(1 << 10)|(1<<11)),			/* intsGppMask */
	DB_88W8660_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88w8660ddr2InfoBoardDeCsInfo,
	DB_88W8660_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	db88w8660ddr2InfoBoardPciIf,
	DB_88W8660_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88w8660ddr2InfoBoardTwsiDev,					
	DB_88W8660_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	db88w8660ddr2InfoBoardMacInfo,
	DB_88W8660_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	db88w8660ddr2InfoBoardGppInfo,
	DB_88W8660_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	NULL,
	N_A,						/* ledsPolarity */		
	DB_88W8660_DDR2_OUT_EN,				/* gppOutEnVal */
	DB_88W8660_DDR2_OUT_VAL,			/* gppOutVal */
	0xA00,						/* gppPolarityVal */
	NULL						/* pSwitchInfo */
};

#define RD_88W8660_BOARD_PCI_IF_NUM		0x1
#define RD_88W8660_BOARD_TWSI_DEF_NUM		0x1
#define RD_88W8660_BOARD_MAC_INFO_NUM		0x1
#define RD_88W8660_BOARD_GPP_INFO_NUM		0x1
#define RD_88W8660_BOARD_DEBUG_LED_NUM		0x3
#define RD_88W8660_BOARD_MPP_CONFIG_NUM		0x1
#define RD_88W8660_BOARD_DEVICE_CONFIG_NUM	0x1

MV_U8			rd88w8660InfoBoardDebugLedIf[RD_88W8660_BOARD_DEBUG_LED_NUM] =
	{6, 5, 7};

MV_BOARD_PCI_IF rd88w8660InfoBoardPciIf[RD_88W8660_BOARD_PCI_IF_NUM] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	{{7, {4, 3, N_A, N_A}}};				/* pciSlot0 */

MV_BOARD_TWSI_INFO rd88w8660InfoBoardTwsiDev[RD_88W8660_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO rd88w8660InfoBoardMacInfo[RD_88W8660_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_100M, 0x0}}; 

MV_BOARD_SWITCH_INFO	rd88w8660InfoBoardSwitchInfo[] = 
	/* MV_32 linkStatusIrq, {MV_32 qdPort0, MV_32 qdPort1, MV_32 qdPort2, MV_32 qdPort3, MV_32 qdPort4}, 
		MV_32 qdCpuPort, MV_32 smiScanMode} */
	{{9, {0, 1, 2, 3, 4}, 5, 0}};

MV_BOARD_GPP_INFO rd88w8660InfoBoardGppInfo[RD_88W8660_BOARD_GPP_INFO_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RTC, 11}};

MV_BOARD_MPP_INFO rd88w8660ddr1InfoBoardMppConfigValue[RD_88W8660_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{RD_88W8660_MPP0_7,			/* mpp0_7 */
	RD_88W8660_MPP8_15,			/* mpp8_15 */
	N_A,
	N_A}}};


MV_DEV_CS_INFO rd88w8660InfoBoardDeCsInfo[RD_88W8660_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/			   
		{{3, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};	/* bootCs */                    

MV_BOARD_INFO rd88w8660Info = {
	"RD-88W8660",				/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88W8660_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	rd88w8660ddr1InfoBoardMppConfigValue,
	((1<<3)|(1<<4)|(1<<9)|(1<<11)),			/* intsGppMask */
	RD_88W8660_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	rd88w8660InfoBoardDeCsInfo,
	RD_88W8660_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	rd88w8660InfoBoardPciIf,
	RD_88W8660_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	rd88w8660InfoBoardTwsiDev,					
	RD_88W8660_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	rd88w8660InfoBoardMacInfo,
	RD_88W8660_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	rd88w8660InfoBoardGppInfo,
	RD_88W8660_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	rd88w8660InfoBoardDebugLedIf,
	0,						/* ledsPolarity */		
	RD_88W8660_BOARD_OUT_EN,			/* gppOutEnVal */
	RD_88W8660_BOARD_OUT_VAL,			/* gppOutVal */
	RD_88W8660_BOARD_MPP_POLAR,			/* gppPolarityVal */
	rd88w8660InfoBoardSwitchInfo			/* pSwitchInfo */
};


#define RD_88W8660_AP82S_BOARD_PCI_IF_NUM		0x1
#define RD_88W8660_AP82S_BOARD_TWSI_DEF_NUM		0x1
#define RD_88W8660_AP82S_BOARD_MAC_INFO_NUM		0x1
#define RD_88W8660_AP82S_BOARD_GPP_INFO_NUM		0x1
#define RD_88W8660_AP82S_BOARD_DEBUG_LED_NUM		0x1
#define RD_88W8660_AP82S_BOARD_MPP_CONFIG_NUM		0x2
#define RD_88W8660_AP82S_BOARD_DEVICE_CONFIG_NUM	0x1

MV_U8	    rd88w8660Ap82sInfoBoardDebugLedIf[RD_88W8660_BOARD_DEBUG_LED_NUM] =
	{6};

MV_BOARD_PCI_IF rd88w8660Ap82sInfoBoardPciIf[RD_88W8660_AP82S_BOARD_PCI_IF_NUM] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	{{7, {8, 3, 8, 8}}};				/* pciSlot0 */

MV_BOARD_TWSI_INFO rd88w8660Ap82sInfoBoardTwsiDev[RD_88W8660_AP82S_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_MAC_INFO rd88w8660Ap82sInfoBoardMacInfo[RD_88W8660_AP82S_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_100M, 0x0}}; 

MV_BOARD_SWITCH_INFO	rd88w8660Ap82sInfoBoardSwitchInfo[] = 
	/* MV_32 linkStatusIrq, {MV_32 qdPort0, MV_32 qdPort1, MV_32 qdPort2, MV_32 qdPort3, MV_32 qdPort4}, 
		MV_32 qdCpuPort, MV_32 smiScanMode} */
	{{-1, {0, 1, 2, 3, 4}, 5, 0}};

MV_BOARD_GPP_INFO rd88w8660Ap82sInfoBoardGppInfo[RD_88W8660_AP82S_BOARD_GPP_INFO_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RTC, 11}};

MV_BOARD_MPP_INFO rd88w8660Ap82sInfoBoardMppConfigValue[RD_88W8660_AP82S_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{RD_88W8660_AP82S_MPP0_7,			/* mpp0_7 */
	RD_88W8660_AP82S_MPP8_15,					
	N_A,
	N_A}},
	{{RD_88W8660_AP82S_MPP0_7NB,			/* mpp0_7 */
	N_A,					
	N_A,
	N_A}}
	};

MV_DEV_CS_INFO rd88w8660Ap82sInfoBoardDeCsInfo[RD_88W8660_AP82S_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/
#if defined(MV_NAND_BOOT)			   
		{{3, 0x8fcfffff, BOARD_DEV_NAND_FLASH, 8}};	/* bootCs */ 
#else
		{{3, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};	/* bootCs */                    
#endif


MV_BOARD_INFO rd88w8660Ap82sInfo = {
	"RD-88W8660-AP82S",				/* boardName[MAX_BOARD_NAME_LEN] */
	RD_88W8660_AP82S_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	rd88w8660Ap82sInfoBoardMppConfigValue,
	((1<<2)|(1<<3)|(1<<8)|(1<<9)|(1<<11)),			/* intsGppMask */
	RD_88W8660_AP82S_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	rd88w8660Ap82sInfoBoardDeCsInfo,
	RD_88W8660_AP82S_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	rd88w8660Ap82sInfoBoardPciIf,
	RD_88W8660_AP82S_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	rd88w8660Ap82sInfoBoardTwsiDev,					
	RD_88W8660_AP82S_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	rd88w8660Ap82sInfoBoardMacInfo,
	RD_88W8660_AP82S_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	rd88w8660Ap82sInfoBoardGppInfo,
	RD_88W8660_AP82S_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	rd88w8660Ap82sInfoBoardDebugLedIf,
	0,							/* ledsPolarity */		
	RD_88W8660_AP82S_OUT_EN,				/* gppOutEnVal */
	RD_88W8660_AP82S_OUT_VAL,				/* gppOutVal */
	RD_88W8660_AP82S_MPP_POLAR,				/* gppPolarityVal */
	rd88w8660Ap82sInfoBoardSwitchInfo			/* pSwitchInfo */

};

MV_BOARD_INFO*	boardInfoTbl[3] = 	{&db88w8660ddr2Info, 
					&rd88w8660Info, 
					&rd88w8660Ap82sInfo	
					};


#define	BOARD_ID_BASE				BOARD_ID_88W8660_BASE
#define MV_MAX_BOARD_ID				BOARD_ID_88W8660_MAX

