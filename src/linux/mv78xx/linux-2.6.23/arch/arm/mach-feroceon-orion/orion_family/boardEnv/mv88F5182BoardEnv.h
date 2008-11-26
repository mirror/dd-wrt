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


#ifndef __INCmvBoard88F5182EnvSpech
#define __INCmvBoard88F5182EnvSpech


/* 88F5182 based boards ID numbers */
/* =============================== */
#define BOARD_ID_88F5182_BASE		0x20

/* Old board ID numbers for backward compatability */
#define DB_88F5182_DDR2_OLD			0xA
#define RD_88F5182_2XSATA_OLD			0xB
#define RD_88F5182_2XSATA3_OLD			0xF
/* New board ID numbers */
#define DB_88F5182_DDR2				(BOARD_ID_88F5182_BASE+0x0)
#define RD_88F5182_2XSATA			(BOARD_ID_88F5182_BASE+0x1)
#define RD_88F5182_2XSATA_OPEN_LINUX_ID		1508
#define RD_88F5182_2XSATA3			(BOARD_ID_88F5182_BASE+0x2)
#define DB_88F5182_DDR2_A			(BOARD_ID_88F5182_BASE+0x3)

#define BOARD_ID_88F5182_MAX			(BOARD_ID_88F5182_BASE+0x4)



#define DB_88F5182_DDR2_MPP0_7			0x55222203
#define DB_88F5182_DDR2_MPP8_15			0x44550000
#define DB_88F5182_DDR2_MPP16_23  		0x0
#define DB_88F5182_DDR2_OE  			0xFFF5FFD7
#define DB_88F5182_DDR2_OUT_VAL  		0x0

#define DB_88F5182_DDR2_A_MPP0_7		0x55222203
#define DB_88F5182_DDR2_A_MPP8_15		0x44550000
#define DB_88F5182_DDR2_A_MPP16_23  		0x0
#define DB_88F5182_DDR2_A_MPP0_7NB		0x55442203
#define DB_88F5182_DDR2_A_MPP8_15NB		0x44550000
#define DB_88F5182_DDR2_A_MPP16_23NB		0x0
#define DB_88F5182_DDR2_A_OE  			0xFFF5FFD7
#define DB_88F5182_DDR2_A_OUT_VAL  		0x0

#define RD_88F5182_2XSATA_MPP0_7	       	0x00000003
#define RD_88F5182_2XSATA_MPP8_15		0x55550000
#define RD_88F5182_2XSATA_MPP16_23  		0x5555
#define RD_88F5182_2XSATA_OE  			0xFFF0F0C8
#define RD_88F5182_2XSATA_OUT_VAL  		0x402
#define RD_88F5182_2XSATA_POL			0xC8

#define RD_88F5182_2XSATA3_MPP0_7	       	0x00000003
#define RD_88F5182_2XSATA3_MPP8_15		0x55550000
#define RD_88F5182_2XSATA3_MPP16_23  		0x5555
#define RD_88F5182_2XSATA3_OE  			0xFFCF0EF8
#define RD_88F5182_2XSATA3_OUT_VAL  		0x104		/* Enable the power for HD-0 on MPP_2 and HD-1 on MPP_8 */

#endif /* __INCmvBoard88F5182EnvSpech */
