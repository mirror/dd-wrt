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

#ifndef __INCmvPMFlashSpecH
#define __INCmvPMFlashSpecH

#include "mvMFlashSpec.h"

/* default values for the timing registers */
#define		MV_PMFLASH_T1                       0x7     /* Read Access Time T1 – sunol2 default */
#define		MV_PMFLASH_T2                       0x7     /* Read Access Time T2 – sunol2 default */
#define		MV_PMFLASH_T7                       0x29    /* Divider Number for T7 – sunol2 default */
#define		MV_PMFLASH_PCLK_OUT                 0x0     /* Hold Time between reference clock and PCLK */
#define		MV_PMFLASH_SLEW_0_1                 0x2     /* slew rate for PCLK_OUT bits 0-1 */
#define		MV_PMFLASH_SLEW_2_3                 0x3     /* slew rate for PCLK_OUT bits 2-3 */

/* Programming allignment mask */
#define		MV_PMFLASH_PROG_WORDS_PER_CHUNK		(MV_MFLASH_PROG_CHUNK_SIZE / 2)

/* IRead Allignmet definitions */
#define		MV_PMFLASH_IREAD_ALIGN_MASK			0x0007  /* IREAD should be alligned to 8bytes */
#define		MV_PMFLASH_IREAD_CHUNK_SIZE			(MV_PMFLASH_IREAD_ALIGN_MASK + 1)

/* wrod count for the Indirect read commands (IREAD and READ ID) */
#define		MV_PMFLASH_IREAD_16BIT_COUNT		0x4			/* 8 bytes per IREAD command */
#define		MV_PMFLASH_READ_ID_16BIT_COUNT		0x3			/* 6 bytes = 4 manf ID + 2 Dev ID */

/* MFlash command opcodes */
#define		MV_PMFLASH_OPCD_PROGRAM				0x02
#define		MV_PMFLASH_OPCD_PRG_CMP				0x07
#define		MV_PMFLASH_OPCD_IPROGRAM			0x82
#define		MV_PMFLASH_OPCD_IPRG_CMP			0x87
#define		MV_PMFLASH_OPCD_PAGE_ERASE			0x52
#define		MV_PMFLASH_OPCD_IPAGE_ERASE			0xD2
#define		MV_PMFLASH_OPCD_CHIP_ERASE			0xE2
#define		MV_PMFLASH_OPCD_MAIN_ERASE			0x62
#define		MV_PMFLASH_OPCD_RDSR				0x05
#define		MV_PMFLASH_OPCD_IREAD				0x83
#define		MV_PMFLASH_OPCD_SET_CFG1			0xB0
#define		MV_PMFLASH_OPCD_SET_CFG2			0xB1
#define		MV_PMFLASH_OPCD_SET_CFG3			0xB2
#define		MV_PMFLASH_OPCD_SET_CFG4			0xB3
#define		MV_PMFLASH_OPCD_READ_CFG1			0xB4
#define		MV_PMFLASH_OPCD_READ_CFG2			0xB5
#define		MV_PMFLASH_OPCD_READ_CFG3			0xB6
#define		MV_PMFLASH_OPCD_READ_CFG4			0xB7
#define		MV_PMFLASH_OPCD_SHUTDOWN			0xA4
#define		MV_PMFLASH_OPCD_SET_SLEW			0xA5
#define		MV_PMFLASH_OPCD_READ_ID				0x15

/* Marvell Flash Device Controller Registers */
#define		MV_PMFLASH_CTRLR_OFST				0x10500
#define		MV_PMFLASH_IF_CTRL_REG				(MV_PMFLASH_CTRLR_OFST + 0x00)
#define		MV_PMFLASH_IF_CFG_REG				(MV_PMFLASH_CTRLR_OFST + 0x04)
#define		MV_PMFLASH_ADDR_LOW_REG				(MV_PMFLASH_CTRLR_OFST + 0x08)
#define		MV_PMFLASH_ADDR_HI_REG				(MV_PMFLASH_CTRLR_OFST + 0x0c)
#define		MV_PMFLASH_CMD_OPCODE_REG			(MV_PMFLASH_CTRLR_OFST + 0x10)
#define		MV_PMFLASH_IF_STATUS_REG			(MV_PMFLASH_CTRLR_OFST + 0x14)
#define		MV_PMFLASH_STATUS_REG				(MV_PMFLASH_CTRLR_OFST + 0x18)
#define		MV_PMFLASH_DATA_REG					(MV_PMFLASH_CTRLR_OFST + 0x20)

/* Flash Interface configuration */
#define		MV_PMFLASH_SERIAL_MODE_OFFSET		0		/* bit 0 */
#define		MV_PMFLASH_RESET_OFFSET				4		/* bit 4 */
#define		MV_PMFLASH_IF_RESET_OFFSET			5		/* bit 5 */
#define		MV_PMFLASH_WP_PROTECT_OFFSET		6		/* bit 6 */
#define		MV_PMFLASH_EXT_FLASH_OFFSET			12	    /* bit 12 */
#define		MV_PMFLASH_EXT_PROGRAMMER_OFFSET	13	    /* bit 13 */
#define		MV_PMFLASH_SERIAL_MODE_MASK			(0x1  << MV_PMFLASH_SERIAL_MODE_OFFSET)
#define		MV_PMFLASH_RESET_MASK				(0x1  << MV_PMFLASH_RESET_OFFSET)
#define		MV_PMFLASH_IF_RESET_MASK			(0x1  << MV_PMFLASH_IF_RESET_OFFSET)
#define		MV_PMFLASH_WP_PROTECT_MASK			(0x1  << MV_PMFLASH_WP_PROTECT_OFFSET)
#define		MV_PMFLASH_EXT_FLASH_MASK			(0x1  << MV_PMFLASH_EXT_FLASH_OFFSET)
#define		MV_PMFLASH_EXT_PROGRAMMER_MASK		(0x1  << MV_PMFLASH_EXT_PROGRAMMER_OFFSET)

#define     MV_PMFLASH_SPI_BUS_MODE_MASK        (MV_PMFLASH_SERIAL_MODE_MASK | MV_PMFLASH_EXT_FLASH_MASK | MV_PMFLASH_EXT_PROGRAMMER_MASK)
#define     MV_PMFLASH_SPI_BUS_TO_MFLASH        0x1001
#define     MV_PMFLASH_SPI_BUS_TO_EXT_SFLASH    0x0000
#define     MV_PMFLASH_SPI_BUS_EXT_PROGRAMER    0x2001

/* Command Opcode Register Masks */
#define		MV_PMFLASH_CMD_OPCD_OFFSET			0		/* bits 0-7 */
#define     MV_PMFLASH_EAD9_8_OFFSET            8       /* bit 8 */
#define		MV_PMFLASH_WR_OPER_STRT_OFFSET		12  	/* bit 12 */
#define		MV_PMFLASH_ERASE_STRT_OFFSET		13	    /* bit 13 */
#define		MV_PMFLASH_RDSR_SET_OFFSET			14	    /* bit 14 */
#define		MV_PMFLASH_INF_READ_STRT_OFFSET		15	    /* bit 15 */
#define     MV_PMFLASH_EAD9_8_MASK              (0x3  << MV_PMFLASH_EAD9_8_OFFSET)
#define		MV_PMFLASH_CMD_OPCD_MASK			(0xFF << MV_PMFLASH_CMD_OPCD_OFFSET)
#define		MV_PMFLASH_WR_OPER_STRT_MASK		(0x1  << MV_PMFLASH_WR_OPER_STRT_OFFSET)
#define		MV_PMFLASH_ERASE_STRT_MASK			(0x1  << MV_PMFLASH_ERASE_STRT_OFFSET)
#define		MV_PMFLASH_RDSR_SET_MASK			(0x1  << MV_PMFLASH_RDSR_SET_OFFSET)
#define		MV_PMFLASH_INF_READ_STRT_MASK		(0x1  << MV_PMFLASH_INF_READ_STRT_OFFSET)

/* Flash Interface Status Register Masks */
#define     MV_PMFLASH_CMD_DONE_OFFSET			0		/* bit 0 */
#define     MV_PMFLASH_CMD_DONE_MASK			(0x1  << MV_PMFLASH_CMD_DONE_OFFSET)

/* Flash Status Register Masks */
#define		MV_PMFLASH_STATUS_OFFSET			0		/* bits 0-7 */
#define		MV_PMFLASH_STAT_WIP_OFFSET			0		/* bit 0 */
#define     MV_PMFLASH_STAT_CMP_OFFSET          6       /* bit 6 */
#define		MV_PMFLASH_STATUS_MASK				(0xFF << MV_PMFLASH_STATUS_OFFSET)
#define		MV_PMFLASH_STAT_WIP_MASK			(0x1  << MV_PMFLASH_STAT_WIP_OFFSET)
#define		MV_PMFLASH_STAT_CMP_MASK			(0x1  << MV_PMFLASH_STAT_CMP_OFFSET)

/* MFlash CONFIG4 register bit Masks */
#define		MV_PMFLASH_CFG4_PG_SZ_OFFSET		0		/* bit 0 */
#define		MV_PMFLASH_CFG4_WP_OFFSET			7		/* bit 7 */
#define		MV_PMFLASH_CFG4_PG_SZ_MASK			(0x1  << MV_PMFLASH_CFG4_PG_SZ_OFFSET)
#define		MV_PMFLASH_CFG4_WP_MASK				(0x1  << MV_PMFLASH_CFG4_WP_OFFSET)

/* MFlash CONFIG2 register bit Masks */
#define		MV_PMFLASH_CFG2_PRFTCH_OFFSET		7	    /* bit 7 */
#define		MV_PMFLASH_CFG2_PRFTCH_MASK			(0x1  << MV_PMFLASH_CFG2_PRFTCH_OFFSET)

#endif /* __INCmvPMFlashSpecH */
