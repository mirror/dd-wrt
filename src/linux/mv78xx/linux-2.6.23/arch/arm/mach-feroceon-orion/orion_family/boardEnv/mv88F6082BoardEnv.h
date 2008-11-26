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


#ifndef __INCmvBoard88F6082BoardEnvSpech
#define __INCmvBoard88F6082BoardEnvSpech


/* 88F6082 based boards ID numbers */
/* =============================== */
#define BOARD_ID_88F6082_BASE		0x70

/* New board ID numbers */
#define DB_88F6082_BP				(BOARD_ID_88F6082_BASE+0x0)
#define RD_88F6082_NAS				(BOARD_ID_88F6082_BASE+0x2)
#define DB_88F6082L_BP				(BOARD_ID_88F6082_BASE+0x4)
#define RD_88F6082_MICRO_DAS_NAS		(BOARD_ID_88F6082_BASE+0x6)
#define RD_88F6082_DX243_24G			(BOARD_ID_88F6082_BASE+0x7)
#define BOARD_ID_88F6082_MAX			(BOARD_ID_88F6082_BASE+0x8)


#define RD_88F6082_NAS_MPP0_7       		0x01111300
#define RD_88F6082_NAS_MPP8_15			0x11110011
#define RD_88F6082_NAS_MPP0_7_NB       		0x01111300
#define RD_88F6082_NAS_MPP8_15_NB		0x11110011
#define RD_88F6082_NAS_OE			0xFFFFF7FF
#define RD_88F6082_NAS_OE_VAL			0x0

#define DB_88F6082_BP_MPP0_7			0x01111000
#define DB_88F6082_BP_MPP8_15			0x22221111
#define DB_88F6082_BP_MPP0_7_NB			0x01111000
#define DB_88F6082_BP_MPP8_15_NB		0x11111111
#define DB_88F6082_BP_OE			0xFFFFFF79
#define DB_88F6082_BP_OE_VAL			0x86

#define DB_88F6082L_BP_MPP0_7			0x01111000
#define DB_88F6082L_BP_MPP8_15			0x22221111
#define DB_88F6082L_BP_MPP0_7_NB		0x01111000
#define DB_88F6082L_BP_MPP8_15_NB		0x11111111
#define DB_88F6082L_BP_OE			0xFFFFFF79
#define DB_88F6082L_BP_OE_VAL			BIT7

/* The MPP config and output enable is delibarate done for input.
   The change from input to output for example on the HDD power MPP
   is done inside the code when setting the power enable. */
#define RD_88F6082_MICRO_DAS_NAS_MPP0_7	    	0x00001000
#define RD_88F6082_MICRO_DAS_NAS_MPP8_15	0x00000011
#define RD_88F6082_MICRO_DAS_NAS_OE		0xFFFFFFFF
#define RD_88F6082_MICRO_DAS_NAS_OE_VAL		0x0

#define RD_88F6082_DX243_MPP0_7			0x01110000
#define RD_88F6082_DX243_MPP8_15		0x22220011
#define RD_88F6082_DX243_MPP0_7_NB		0x01110000
#define RD_88F6082_DX243_MPP8_15_NB		0x11110011
#define RD_88F6082_DX243_OE			0xFFFFFFF9
#define RD_88F6082_DX243_OE_VAL			0x0

#endif /* __INCmvBoard88F6082EnvSpech */
