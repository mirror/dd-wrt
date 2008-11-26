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

#define DB_CUS1_BOARD_PCI_IF_NUM		0x1
#define DB_CUS1_BOARD_TWSI_DEF_NUM		0x0
#define DB_CUS1_BOARD_MAC_INFO_NUM		0x1
#define DB_CUS1_BOARD_GPP_INFO_NUM		0x0
#define DB_CUS1_BOARD_DEBUG_LED_NUM		0x2
#define DB_CUS1_BOARD_MPP_CONFIG_NUM		0x1
#define DB_CUS1_BOARD_DEVICE_CONFIG_NUM		0x1


MV_BOARD_PCI_IF db88f5181Cus1InfoBoardPciIf[] = 
	/* {pciDevNum, {intAGppPin, intBGppPin, intCGppPin, intDGppPin}} */					
	{{7, {N_A, N_A, N_A, N_A}}				/* pciSlot0 */					    
	};


MV_U8			rd88f5181Cus1InfoBoardDebugLedIf[DB_CUS1_BOARD_DEBUG_LED_NUM] =
	{0, 11};

MV_BOARD_MAC_INFO	rd88f5181Cus1InfoBoardMacInfo[DB_CUS1_BOARD_MAC_INFO_NUM] = 
	/* {{MV_BOARD_MAC_SPEED	boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{{BOARD_MAC_SPEED_AUTO, 0x8}}; 

/*MV_BOARD_GPP_INFO	rd88f5181Cus1InfoBoardGppInfo[DB_CUS1_BOARD_GPP_INFO_NUM] = 
*/
	/* {{MV_BOARD_DEV_CLASS	devClass, MV_U8	gppPinNum}} */
/*	{{BOARD_DEV_RTC, 8},
	{BOARD_DEV_USB_VBUS, 9},
	{BOARD_DEV_REF_CLCK,0}};
*/

MV_BOARD_MPP_INFO rd88f5181Cus1InfoBoardMppConfigValue[DB_CUS1_BOARD_MPP_CONFIG_NUM] = 
	/* {{MV_U32 mpp0_7, MV_U32 mpp8_15, MV_U32 mpp16_23, MV_U32 mppDev}} */
	{{{RD_88F5181L_CUS1_MPP0_7,			/* mpp0_7 */
	RD_88F5181L_CUS1_MPP8_15,			/* mpp8_15 */
	RD_88F5181L_CUS1_MPP16_23,			/* mpp16_23 */
	RD_88F5181L_CUS1_MPP_DEV}}};			/* mppDev */						

MV_DEV_CS_INFO rd88f5181Cus1InfoBoardDeCsInfo[DB_CUS1_BOARD_DEVICE_CONFIG_NUM] = 
		/*{params, devType, devWidth}*/			   
		{{3, 0x8fdfffff, BOARD_DEV_NOR_FLASH, 16}};	/* bootCs */                    

MV_BOARD_INFO rd88f5181Cus1Info = {
	"RD-88F5181L-Customer 1",			/* boardName[MAX_BOARD_NAME_LEN] */
	DB_CUS1_BOARD_MPP_CONFIG_NUM,			/* numBoardMppConfig */
	rd88f5181Cus1InfoBoardMppConfigValue,
	((1 << 4)|(1 << 6)|(1 << 8)),			/* intsGppMask */
	DB_CUS1_BOARD_DEVICE_CONFIG_NUM,		/* numBoardDevIf */
	rd88f5181Cus1InfoBoardDeCsInfo,
	DB_CUS1_BOARD_PCI_IF_NUM,			/* numBoardPciIf */
	db88f5181Cus1InfoBoardPciIf,
	DB_CUS1_BOARD_TWSI_DEF_NUM,			/* numBoardTwsiDev */
	NULL,					
	DB_CUS1_BOARD_MAC_INFO_NUM,			/* numBoardMacInfo */
	rd88f5181Cus1InfoBoardMacInfo,
	DB_CUS1_BOARD_GPP_INFO_NUM,			/* numBoardGppInfo */
	NULL,
	DB_CUS1_BOARD_DEBUG_LED_NUM,			/* activeLedsNumber */              
	rd88f5181Cus1InfoBoardDebugLedIf,
	0,						/* ledsPolarity */		
	RD_88F5181L_CUS1_GPP_OE,			/* gppOutEnVal */
	N_A,						/* gppPolarityVal */
};


MV_BOARD_INFO*	boardInfoTbl[]	=	{&rd88f5181Cus1Info};

#define	BOARD_ID_BASE			BOARD_ID_CUSTOMER_BASE
#define MV_MAX_BOARD_ID			BOARD_ID_CUSTOMER_MAX
