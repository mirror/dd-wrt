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

#ifndef __INCmvSMFlashSpecH
#define __INCmvSMFlashSpecH

/* Constants */
#define		MV_SMFLASH_PAGE_ALLIGN_MASK		    0x3F
#define		MV_SMFLASH_PAGE_SIZE			    (MV_SMFLASH_PAGE_ALLIGN_MASK+1) /* 64 byte */

#define		MV_SMFLASH_WREN_CMND_OPCD		    0x06	/* Write Enable */
#define		MV_SMFLASH_WRDI_CMND_OPCD		    0x04	/* Write Disable */
#define		MV_SMFLASH_RDSR_CMND_OPCD		    0x05	/* Read Status Register */
#define		MV_SMFLASH_READ_CMND_OPCD		    0x03	/* Sequential Read */
#define		MV_SMFLASH_PRGRM_CMND_OPCD		    0x02	/* Page Program */
#define		MV_SMFLASH_PRGRM_CMP_CMND_OPCD	    0x07	/* Page Program and compare */
#define		MV_SMFLASH_PAGE_ERASE_CMND_OPCD	    0x42	/* Sector Erase */
#define		MV_SMFLASH_MAIN_ERASE_CMND_OPCD	    0x62	/* Main region Erase */
#define		MV_SMFLASH_RDID_CMND_OPCD		    0x15	/* Read ID */
#define		MV_SMFLASH_IREAD_CMND_OPCD		    0x83	/* Read from Information region */
#define		MV_SMFLASH_IPRGRM_CMND_OPCD		    0x82	/* Program the information region */
#define		MV_SMFLASH_IPRGRM_CMP_CMND_OPCD	    0x87	/* Program and compare information region */
#define		MV_SMFLASH_SET_CFG_3_CMND_OPCD	    0xB2	/* Set configuration register 3 */
#define		MV_SMFLASH_SET_CFG_4_CMND_OPCD	    0xB3	/* Set configuration register 4 */
#define		MV_SMFLASH_READ_CFG_3_CMND_OPCD	    0xB6	/* Read configuration register 3 */
#define		MV_SMFLASH_READ_CFG_4_CMND_OPCD	    0xB7	/* Read configuration register 4 */
#define		MV_SMFLASH_IPG_ERASE_CMND_OPCD	    0xC2	/* Erase the information page */
#define		MV_SMFLASH_CHIP_ERASE_CMND_OPCD	    0xE2	/* Chip erase (main + information) */
#define		MV_SMFLASH_SHUTDOWN_CMND_OPCD	    0xF0	/* Set flash in power down mode */

#define		MV_SMFLASH_PAGE_ERASE_CMND_LEN	    4		/* 1B opcode + 3B address */
#define		MV_SMFLASH_PRGRM_CMND_LEN   		4		/* 1B opcode + 3B address */
#define		MV_SMFLASH_PRGRM_CMP_CMND_LEN	    4		/* 1B opcode + 3B address */
#define		MV_SMFLASH_IPRGRM_CMND_LEN		    4		/* 1B opcode + 3B address */
#define		MV_SMFLASH_IPRGRM_CMP_CMND_LEN  	4		/* 1B opcode + 3B address */
#define		MV_SMFLASH_READ_CMND_LEN		    4		/* 1B opcode + 3B address */
#define		MV_SMFLASH_IREAD_CMND_LEN		    4		/* 1B opcode + 3B address */
#define		MV_SMFLASH_RDID_CMND_LEN		    1		/* 1B opcode */
#define		MV_SMFLASH_RDID_RPLY_LEN		    6		/* 4B manuf ID + 2B device ID */

/* Status Register Bit Masks */
#define		MV_SMFLASH_STATUS_REG_WIP_OFFSET    0	    /* bit 0 - Write in Progress */
#define		MV_SMFLASH_STATUS_REG_WEL_OFFSET    1	    /* bit 1 - Write enable Latch */
#define		MV_SMFLASH_STATUS_REG_CMP_OFFSET    6	    /* bit 6 - Compare status */
#define		MV_SMFLASH_STATUS_REG_WP_OFFSET	    7	    /* bit 7 - Write protect status */
#define		MV_SMFLASH_STATUS_REG_WIP_MASK	    (0x1 << MV_SMFLASH_STATUS_REG_WIP_OFFSET)
#define		MV_SMFLASH_STATUS_REG_WEL_MASK	    (0x1 << MV_SMFLASH_STATUS_REG_WEL_OFFSET)
#define		MV_SMFLASH_STATUS_REG_CMP_MASK	    (0x1 << MV_SMFLASH_STATUS_REG_CMP_OFFSET)
#define		MV_SMFLASH_STATUS_REG_WP_MASK	    (0x1 << MV_SMFLASH_STATUS_REG_WP_OFFSET)

/* Serial Interface Configuration 4 Masks */
#define		MV_SMFLASH_SRL_CFG4_PG_SIZE_OFFSET  0       /* bit 0 - Page size */
#define		MV_SMFLASH_SRL_CFG4_WP_OFFSET		7       /* bit 7 - Write protection */
#define		MV_SMFLASH_SRL_CFG4_PG_SIZE_MASK    (0x1 << MV_SMFLASH_SRL_CFG4_PG_SIZE_OFFSET)
#define		MV_SMFLASH_SRL_CFG4_WP_MASK		    (0x1 << MV_SMFLASH_SRL_CFG4_WP_OFFSET)

#endif /* __INCmvSMFlashSpecH */

