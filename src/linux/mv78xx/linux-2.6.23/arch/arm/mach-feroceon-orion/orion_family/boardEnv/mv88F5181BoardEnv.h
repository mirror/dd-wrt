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


#ifndef __INCmvBoard88F5181EnvSpech
#define __INCmvBoard88F5181EnvSpech



/* 88F5181 and 88F5281 based boards ID numbers */
/* =========================================== */
#define BOARD_ID_88F5181_5281_BASE	0x0

#define RD_88F5181_GTW_FE_OLD           0xd
#define RD_88F5181_GTW_GE_OLD           0xe
#define RD_88F5181_POS_NAS_OLD		0xab
#define DB_88F5X81_DDR2_OLD		0xcd
#define DB_88F5X81_DDR1_OLD		0xce

#define DB_88F1181_DDR1			(BOARD_ID_88F5181_5281_BASE+0x0) /* obsolete */
#define DB_88F1181_DDR2			(BOARD_ID_88F5181_5281_BASE+0x1) /* obsolete */
#define DB_88F5181_5281_DDR1		(BOARD_ID_88F5181_5281_BASE+0x2)
#define DB_88F5181_5281_DDR2		(BOARD_ID_88F5181_5281_BASE+0x3)
#define DB_88F5181_DDR1_PRPMC		(BOARD_ID_88F5181_5281_BASE+0x4)
#define DB_88F5181_DDR1_PEXPCI		(BOARD_ID_88F5181_5281_BASE+0x5)
#define RD_88F5181_POS_NAS 		(BOARD_ID_88F5181_5281_BASE+0x6)
#define DB_88F5X81_DDR2			(BOARD_ID_88F5181_5281_BASE+0x7)
#define DB_88F5X81_DDR2_OPEN_LINUX_ID	1358
#define DB_88F5X81_DDR1			(BOARD_ID_88F5181_5281_BASE+0x8)
#define RD_88F5181_VOIP			(BOARD_ID_88F5181_5281_BASE+0x9) /* obsolete */
#define DB_88F5181_DDR1_MNG		(BOARD_ID_88F5181_5281_BASE+0xA)
#define RD_88F5181_GTW_FE		(BOARD_ID_88F5181_5281_BASE+0xB)
#define RD_88F5181_GTW_GE		(BOARD_ID_88F5181_5281_BASE+0xC)
#define DB_88F5X8X_FPGA_DDR1		(BOARD_ID_88F5181_5281_BASE+0xD)

#define BOARD_ID_88F5181_5281_MAX	(BOARD_ID_88F5181_5281_BASE+0xE)

#define DB_88F5181_5281_DDR1_MPP0_7	       	0x00032222
#define DB_88F5181_5281_DDR1_MPP8_15	   	0x11111111
#define DB_88F5181_5281_DDR1_MPP16_23  		0x1111
#define DB_88F5181_5281_DDR1_MPP_DEV		0x0
#define DB_88F5181_5281_DDR1_OE  		0xFFFFFFFF
#define DB_88F5181_5281_DDR1_OUT_VAL  		0x0

#define DB_88F5181_5281_DDR2_MPP0_7	       	0x00222203
#define DB_88F5181_5281_DDR2_MPP8_15	   	0x00001133
#define DB_88F5181_5281_DDR2_MPP16_23  		0x0
#define DB_88F5181_5281_DDR2_MPP_DEV		0x0
#define DB_88F5181_5281_DDR2_OE  		0xFFFF3F17
#define DB_88F5181_5281_DDR2_OUT_VAL  		BIT15

#define DB_88F5X81_DDRX_MPP0_7	       		0x33222203
#define DB_88F5X81_DDRX_MPP8_15			0x44000033
#define DB_88F5X81_DDRX_MPP16_23  		0x0
#define DB_88F5X81_DDRX_MPP_DEV			0x0
#define DB_88F5X81_DDRX_OE  			0xFFFDFF17
#define DB_88F5X81_DDRX_OUT_VAL  		0x0

#define DB_88F5181_DDR1_PRPMC_MPP0_7	    	0x00000003
#define DB_88F5181_DDR1_PRPMC_MPP8_15	   	0x10001111
#define DB_88F5181_DDR1_PRPMC_MPP16_23  	0x00001111
#define DB_88F5181_DDR1_PRPMC_MPP_DEV		0x0
#define DB_88F5181_DDR1_PRPMC_OE  		0xFFFF0FFF
#define DB_88F5181_DDR1_PRPMC_OUT_VAL  		0x0

#define RD_88F5181_POS_NAS_MPP0_7	       	0x0
#define RD_88F5181_POS_NAS_MPP8_15		0x0
#define RD_88F5181_POS_NAS_MPP16_23  		0x0
#define RD_88F5181_POS_NAS_MPP_DEV		0x0
#define RD_88F5181_POS_NAS_OE  			0x00000BDF
#define RD_88F5181_POS_NAS_OUT_VAL  		0x0

#define RD_88F5181_VOIP_MPP0_7	       		0x00000003
#define RD_88F5181_VOIP_MPP8_15			0x00000101
#define RD_88F5181_VOIP_MPP16_23  		0x0
#define RD_88F5181_VOIP_MPP_DEV			0x0
#define RD_88F5181_VOIP_OE  			0x000005F5
#define RD_88F5181_VOIP_OUT_VAL  		0x0


#define DB_88F5X81_DDRX_MPP0_7_NB               0x33442203
#define DB_88F5X81_DDRX_MPP8_15_NB              0x44000033
#define DB_88F5X81_DDRX_MPP16_23_NB             0x0
#define DB_88F5X81_DDRX_MPP_DEV_NB              0x0

#define DB_88F5181_DDR1_MNG_MPP0_7              0x00000003
#define DB_88F5181_DDR1_MNG_MPP8_15             0x10001111
#define DB_88F5181_DDR1_MNG_MPP16_23            0x00001111
#define DB_88F5181_DDR1_MNG_MPP_DEV             0x0
#define DB_88F5181_DDR1_MNG_GPP_OE             	0xFFFF8FF8
#define DB_88F5181_DDR1_MNG_GPP_OUT_VAL  	0x0

#define DB_88F5181_DDR1_PEXPCI_MPP0_7	    	0x00330000
#define DB_88F5181_DDR1_PEXPCI_MPP8_15	   	0x0
#define DB_88F5181_DDR1_PEXPCI_MPP16_23  	0x0
#define DB_88F5181_DDR1_PEXPCI_MPP_DEV		0x0
#define DB_88F5181_DDR1_PEXPCI_OE  		0xFFFDFF17
#define DB_88F5181_DDR1_PEXPCI_OUT_VAL  	0x0

#define RD_88F5181_GTW_FE_MPP0_7	    	0x55000003
#define RD_88F5181_GTW_FE_MPP8_15	   	0x00000101
#define RD_88F5181_GTW_FE_MPP16_23  		0x0
#define RD_88F5181_GTW_FE_MPP_DEV		0x0
#define RD_88F5181_GTW_FE_OE  			0xFFFF0FFC
#define RD_88F5181_GTW_FE_OUT_VAL  		0xF001

#define RD_88F5181_GTW_GE_MPP0_7	    	0x55000003
#define RD_88F5181_GTW_GE_MPP8_15	   	0x11110010
#define RD_88F5181_GTW_GE_MPP16_23  		0x1111
#define RD_88F5181_GTW_GE_MPP_DEV		0x0
#define RD_88F5181_GTW_GE_OE  			0xFFFF07F0
#define RD_88F5181_GTW_GE_OUT_VAL  		0x80F

#define DB_88F5X8X_DDRX_MPP0_7	    		0x00222222
#define DB_88F5X8X_DDRX_MPP8_15	   		0x33330000
#define DB_88F5X8X_DDRX_MPP16_23  		0x0
#define DB_88F5X8X_DDRX_MPP_DEV			0x0
#define DB_88F5X8X_DDRX_OE  			0xFFFF0330
#define DB_88F5X8X_DDRX_OUT_VAL  		0x0

#endif /* __INCmvBoard88F5181EnvSpech */
