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


#ifndef __INCmvCtrlEnvAsmh
#define __INCmvCtrlEnvAsmh
#include "pci/mvPciRegs.h"

/* Read device ID into toReg bits 15:0 from 0xd0000000 */
/* defines  */
#if defined(MV_88F5180N)
#define MV_DV_CTRL_MODEL_GET_ASM(toReg, tmpReg) \
	ldr	toReg, =MV_5180_DEV_ID
#elif defined(MV_88F5082)
#define MV_DV_CTRL_MODEL_GET_ASM(toReg, tmpReg) \
	ldr	toReg, =MV_5082_DEV_ID
#else
#if defined(MV_INCLUDE_PEX)
#define MV_DV_CTRL_MODEL_GET_ASM(toReg, tmpReg) \
        MV_DV_REG_READ_ASM(toReg, tmpReg, PEX_CFG_DIRECT_ACCESS(0,PEX_DEVICE_AND_VENDOR_ID));\
        mov     toReg, toReg, LSR #PXDAVI_DEV_ID_OFFS         /* toReg = toReg >> 16 */

#elif defined(MV_INCLUDE_PCI)
#define MV_DV_CTRL_MODEL_GET_ASM(toReg, tmpReg) \
	/* Read bus number */				\
	MV_DV_REG_READ_ASM(toReg, tmpReg, PCI_P2P_CONFIG_REG(0));\
	mov 	tmpReg, toReg;				\
	and 	toReg, toReg, #PPCR_BUS_NUM_MASK;		\
	/* Read dev number */				\
	and 	tmpReg, tmpReg, #PPCR_DEV_NUM_MASK;		\
	orr	toReg, tmpReg, LSR #(PPCR_DEV_NUM_OFFS - PCAR_DEVICE_NUM_OFFS); /* toReg = toReg | (tmpReg >> 13) */\
	/* Set PCI config enable BIT31 */		\
	orr 	toReg, toReg, #PCAR_CONFIG_EN;			\
	/* Write the address to the PCI configuration address register */\
	MV_DV_REG_WRITE_ASM(toReg, tmpReg, PCI_CONFIG_ADDR_REG(0));\
	/* In order to let the PCI controller absorbed the address of the read 	*/\
	/* transaction we perform a validity check that the address was written */\
	MV_DV_REG_READ_ASM(toReg, tmpReg, PCI_CONFIG_ADDR_REG(0));	\
	/* Read the Data returned in the PCI Data register */\
	MV_DV_REG_READ_ASM(toReg, tmpReg, PCI_CONFIG_DATA_REG(0));	\
        mov     toReg, toReg, LSR #PDVIR_DEV_ID_OFFS         /* toReg = toReg >> 16 */
#else
	#error "No Way to get Device ID"
#endif /* MV_INCLUDE_PEX */
#endif /* MV_88F5180N */

/* Read device ID into toReg bits 15:0 from 0xf1000000*/

#if defined(MV_88F5180N)
#define MV_CTRL_MODEL_GET_ASM(toReg, tmpReg) \
	ldr	toReg, =MV_5180_DEV_ID
#elif defined(MV_88F5082)
#define MV_CTRL_MODEL_GET_ASM(toReg, tmpReg) \
	ldr	toReg, =MV_5082_DEV_ID
#else
#if defined(MV_INCLUDE_PEX)
#define MV_CTRL_MODEL_GET_ASM(toReg, tmpReg) \
        MV_REG_READ_ASM(toReg, tmpReg, PEX_CFG_DIRECT_ACCESS(0,PEX_DEVICE_AND_VENDOR_ID));\
        mov     toReg, toReg, LSR #PXDAVI_DEV_ID_OFFS         /* toReg = toReg >> 16 */

#elif defined(MV_INCLUDE_PCI)
#define MV_CTRL_MODEL_GET_ASM(toReg, tmpReg) \
	/* Read bus number */				\
	MV_REG_READ_ASM(toReg, tmpReg, PCI_P2P_CONFIG_REG(0));\
	mov 	tmpReg, toReg;				\
	and 	toReg, toReg, #PPCR_BUS_NUM_MASK;		\
	/* Read dev number */				\
	and 	tmpReg, tmpReg, #PPCR_DEV_NUM_MASK;		\
	orr	toReg, tmpReg, LSR #(PPCR_DEV_NUM_OFFS - PCAR_DEVICE_NUM_OFFS); /* toReg = toReg | (tmpReg >> 13) */\
	/* Set PCI config enable BIT31 */		\
	orr 	toReg, toReg, #PCAR_CONFIG_EN;			\
	/* Write the address to the PCI configuration address register */\
	MV_REG_WRITE_ASM(toReg, tmpReg, PCI_CONFIG_ADDR_REG(0));\
	/* In order to let the PCI controller absorbed the address of the read 	*/\
	/* transaction we perform a validity check that the address was written */\
	MV_REG_READ_ASM(toReg, tmpReg, PCI_CONFIG_ADDR_REG(0));	\
	/* Read the Data returned in the PCI Data register */\
	MV_REG_READ_ASM(toReg, tmpReg, PCI_CONFIG_DATA_REG(0));	\
        mov     toReg, toReg, LSR #PDVIR_DEV_ID_OFFS         /* toReg = toReg >> 16 */
#else
	#error "No Way to get Device ID"
#endif /* MV_INCLUDE_PEX */
#endif /* MV_88F5180N */


/* Read Revision into toReg bits 7:0 0xd0000000*/
#if defined(MV_88F5180N)
#define MV_DV_CTRL_REV_GET_ASM(toReg, tmpReg) \
	ldr	toReg, =MV_5180N_B1_REV
#else
#if defined(MV_INCLUDE_PEX)
#define MV_DV_CTRL_REV_GET_ASM(toReg, tmpReg)	\
        /* Read device revision */			\
        MV_DV_REG_READ_ASM(toReg, tmpReg, PEX_CFG_DIRECT_ACCESS(0,PEX_CLASS_CODE_AND_REVISION_ID));\
        and     toReg, toReg, #PXCCARI_REVID_MASK                  /* Mask for calss ID */
#elif defined(MV_INCLUDE_PCI)
#define MV_DV_CTRL_REV_GET_ASM(toReg, tmpReg)		\
	/* Read bus number */				\
	MV_DV_REG_READ_ASM(toReg, tmpReg, PCI_P2P_CONFIG_REG(0));\
	mov 	tmpReg, toReg;				\
	and 	toReg, toReg, #PPCR_BUS_NUM_MASK;		\
	/* Read dev number */				\
	and 	tmpReg, tmpReg, #PPCR_DEV_NUM_MASK;		\
	orr	toReg, tmpReg, LSR #(PPCR_DEV_NUM_OFFS - PCAR_DEVICE_NUM_OFFS); /* toReg = toReg | (tmpReg >> 13) */\
	/* Set PCI config enable BIT31 */		\
	orr 	toReg, toReg, #PCAR_CONFIG_EN;			\
	orr 	toReg, toReg, #(PCI_CLASS_CODE_AND_REVISION_ID);\
	/* Write the address to the PCI configuration address register */\
	MV_DV_REG_WRITE_ASM(toReg, tmpReg, PCI_CONFIG_ADDR_REG(0));\
	/* In order to let the PCI controller absorbed the address of the read 	*/\
	/* transaction we perform a validity check that the address was written */\
	MV_DV_REG_READ_ASM(toReg, tmpReg, PCI_CONFIG_ADDR_REG(0));\
	/* Read the Data returned in the PCI Data register */\
	MV_DV_REG_READ_ASM(toReg, tmpReg, PCI_CONFIG_DATA_REG(0));\
	and toReg, toReg, #PCCRIR_REVID_MASK
#else
	#error "No Way to get Revision ID"
#endif /* MV_INCLUDE_PEX */
#endif /* MV_88F5180N */

/* Read Revision into toReg bits 7:0 0xf1000000*/
#if defined(MV_88F5180N)
#define MV_CTRL_REV_GET_ASM(toReg, tmpReg) \
	ldr	toReg, =MV_5180N_B1_REV
#else
#if defined(MV_INCLUDE_PEX)
#define MV_CTRL_REV_GET_ASM(toReg, tmpReg)	\
        /* Read device revision */			\
        MV_REG_READ_ASM(toReg, tmpReg, PEX_CFG_DIRECT_ACCESS(0,PEX_CLASS_CODE_AND_REVISION_ID));\
        and     toReg, toReg, #PXCCARI_REVID_MASK                  /* Mask for calss ID */
#elif defined(MV_INCLUDE_PCI)
#define MV_CTRL_REV_GET_ASM(toReg, tmpReg)		\
	/* Read bus number */				\
	MV_REG_READ_ASM(toReg, tmpReg, PCI_P2P_CONFIG_REG(0));\
	mov 	tmpReg, toReg;				\
	and 	toReg, toReg, #PPCR_BUS_NUM_MASK;		\
	/* Read dev number */				\
	and 	tmpReg, tmpReg, #PPCR_DEV_NUM_MASK;		\
	orr	toReg, tmpReg, LSR #(PPCR_DEV_NUM_OFFS - PCAR_DEVICE_NUM_OFFS); /* toReg = toReg | (tmpReg >> 13) */\
	/* Set PCI config enable BIT31 */		\
	orr 	toReg, toReg, #PCAR_CONFIG_EN;			\
	orr 	toReg, toReg, #(PCI_CLASS_CODE_AND_REVISION_ID);\
	/* Write the address to the PCI configuration address register */\
	MV_REG_WRITE_ASM(toReg, tmpReg, PCI_CONFIG_ADDR_REG(0));\
	/* In order to let the PCI controller absorbed the address of the read 	*/\
	/* transaction we perform a validity check that the address was written */\
	MV_REG_READ_ASM(toReg, tmpReg, PCI_CONFIG_ADDR_REG(0));\
	/* Read the Data returned in the PCI Data register */\
	MV_REG_READ_ASM(toReg, tmpReg, PCI_CONFIG_DATA_REG(0));\
	and toReg, toReg, #PCCRIR_REVID_MASK
#else
	#error "No Way to get Revision ID"
#endif /* MV_INCLUDE_PEX */
#endif /* MV_88F5180N */

#endif /* __INCmvCtrlEnvAsmh */
