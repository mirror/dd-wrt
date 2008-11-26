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
#define DB_88F1281_BOARD_PCI_IF_NUM		0x0
#define DB_88F1281_BOARD_TWSI_DEF_NUM		0x1
#define DB_88F1281_BOARD_MAC_INFO_NUM		0x0
#define DB_88F1281_BOARD_GPP_INFO_NUM		0x1
#define DB_88F1281_BOARD_DEBUG_LED_NUM		0x4
#define DB_88F1281_BOARD_MPP_CONFIG_NUM		0x1
#define DB_88F1281_BOARD_DEVICE_CONFIG_NUM	0x2

MV_U8			db88f1281ddr2InfoBoardDebugLedIf[DB_88F1281_BOARD_DEBUG_LED_NUM] =
	{2, 3, 6, 7};

MV_BOARD_TWSI_INFO	db88f1281ddr2InfoBoardTwsiDev[DB_88F1281_BOARD_TWSI_DEF_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{{BOARD_DEV_RTC, 0x68, ADDR7_BIT}};

MV_BOARD_GPP_INFO	db88f1281ddr2InfoBoardGppInfo[DB_88F1281_BOARD_GPP_INFO_NUM] = 
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
	{{BOARD_DEV_RTC, 1}};

MV_BOARD_MPP_INFO	db88f1281ddr2InfoBoardMppConfigValue[DB_88F1281_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{DB_88F1281_DDR2_MPP0_7,			/* mpp0_7 */
	DB_88F1281_DDR2_MPP8_15,			/* mpp8_15 */
	N_A,						/* mpp16_23 */
	N_A}}						/* mppDev */						
	};

#if defined(MV_NAND)
MV_DEV_CS_INFO db88f1281ddr2InfoBoardDeCsInfo[DB_88F1281_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/			   
		{{ 0, 0x8fcfffff, BOARD_DEV_NAND_FLASH, 8},  /* devCs2/NandCs */
		{ 1, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}};   /* bootCs */
#else
MV_DEV_CS_INFO db88f1281ddr2InfoBoardDeCsInfo[DB_88F1281_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/			   
			    {{ 0, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8},  /* devCs2/flashCs */
			     { 1, 0x8fcfffff, BOARD_DEV_NOR_FLASH, 8}}; /* bootCs */
#endif /* defined(MV_NAND) */

MV_BOARD_INFO db88f1281ddr2Info = {
	"DB-88F1281-DDR2",				/* boardName[MAX_BOARD_NAME_LEN] */
	DB_88F1281_BOARD_MPP_CONFIG_NUM,		/* numBoardMppConfig */
	db88f1281ddr2InfoBoardMppConfigValue,
	((1 << 1)),					/* intsGppMask */
	DB_88F1281_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	db88f1281ddr2InfoBoardDeCsInfo,
	DB_88F1281_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	NULL,
	DB_88F1281_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	db88f1281ddr2InfoBoardTwsiDev,					
	DB_88F1281_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	NULL,
	DB_88F1281_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	db88f1281ddr2InfoBoardGppInfo,
	DB_88F1281_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	db88f1281ddr2InfoBoardDebugLedIf,
	N_A,						/* ledsPolarity */		
	DB_88F1281_DDR2_OE,				/* gppOutEnVal */
	DB_88F1281_DDR2_OE_VAL,				/* gppOutVal */
	0x1						/* gppPolarityVal */
};


MV_BOARD_INFO*	boardInfoTbl[1] = 	{&db88f1281ddr2Info, 
					};


#define	BOARD_ID_BASE				BOARD_ID_88F1281_BASE
#define MV_MAX_BOARD_ID				BOARD_ID_88F1281_MAX
