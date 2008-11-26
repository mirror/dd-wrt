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


#ifndef __INCmvBoard88F5181LEnvSpech
#define __INCmvBoard88F5181LEnvSpech



/* 88F5181L based boards ID numbers */
/* =============================== */
#define BOARD_ID_88F5181L_BASE		0x30

/* Old board ID numbers for backward compatability */
#define DB_88F5181L_DDR2_2XTDM_OLD	0xC
#define RD_88F5181L_VOIP_FE_OLD		0xD
#define RD_88F5181L_VOIP_GE_OLD		0xE
/* New board ID numbers */
#define DB_88F5181L_DDR2_2XTDM		(BOARD_ID_88F5181L_BASE+0x0)
#define RD_88F5181L_VOIP_FE		(BOARD_ID_88F5181L_BASE+0x1)
#define RD_88F5181L_VOIP_GE		(BOARD_ID_88F5181L_BASE+0x2)
#define RD_88F5181L_VOIP_FXO_GE		(BOARD_ID_88F5181L_BASE+0x3)

#define BOARD_ID_88F5181L_MAX		(BOARD_ID_88F5181L_BASE+0x4)

#define DB_88F5181L_DDR2_2XTDM_MPP0_7	   	0x00222203
#define DB_88F5181L_DDR2_2XTDM_MPP8_15	   	0x44000000
#define DB_88F5181L_DDR2_2XTDM_MPP16_23 	0x0
#define DB_88F5181L_DDR2_2XTDM_MPP_DEV		0x0
#define DB_88F5181L_DDR2_2XTDM_OE  		0xFFFDFFD7
#define DB_88F5181L_DDR2_2XTDM_OUT_VAL  	0x0

#define RD_88F5181L_VOIP_FE_MPP0_7	    	0x55000003
#define RD_88F5181L_VOIP_FE_MPP8_15	   	0x00000101
#define RD_88F5181L_VOIP_FE_MPP16_23  		0x0
#define RD_88F5181L_VOIP_FE_MPP_DEV		0x0
#define RD_88F5181L_VOIP_FE_GPP_OE		0xFFFF0FFC
#define RD_88F5181L_VOIP_FE_GPP_IO		0xF001

#define RD_88F5181L_VOIP_GE_MPP0_7		0x55000003
#define RD_88F5181L_VOIP_GE_MPP8_15	   	0x11110010
#define RD_88F5181L_VOIP_GE_MPP16_23		0x1111
#define RD_88F5181L_VOIP_GE_MPP_DEV		0x0
#define RD_88F5181L_VOIP_GE_GPP_OE		0xFFFF07F0
#define RD_88F5181L_VOIP_GE_GPP_IO		0x80F

#define RD_88F5181L_FXO_GE_MPP0_7		0x55000003
#define RD_88F5181L_FXO_GE_MPP8_15	   	0x11110010
#define RD_88F5181L_FXO_GE_MPP16_23		0x1111
#define RD_88F5181L_FXO_GE_MPP_DEV		0x80000000
#define RD_88F5181L_FXO_GE_GPP_OE		0x7FFFF2C2
#define RD_88F5181L_FXO_GE_GPP_IO		0xBFF

#endif /* __INCmvBoard88F5181LEnvSpech */
