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

#ifndef __INCmvCtrlEnvSpech
#define __INCmvCtrlEnvSpech

#include "mvDeviceId.h"
#include "mvSysHwConfig.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* includes */


/* defines  */
/* This define describes the TWSI interrupt bit and location */
#define TWSI_CPU_MAIN_INT_CAUSE_REG		0x20200
#define TWSI_CPU_MAIN_INT_BIT			0x20

/* 0x5181 revisions*/
#define MV_5181_A0_REV          	0x0
#define MV_5181_A0_ID           	((MV_5181_DEV_ID << 16) | MV_5181_A0_REV)
#define MV_5181_A0_NAME         	"88F5181 A0"
#define MV_5181_A1_REV          0x1
#define MV_5181_A1_ID           	((MV_5181_DEV_ID << 16) | MV_5181_A1_REV)
#define MV_5181_A1_NAME         	"88F5181 A1"
#define MV_5181_B0_REV          0x2
#define MV_5181_B0_ID           	((MV_5181_DEV_ID << 16) | MV_5181_B0_REV)
#define MV_5181_B0_NAME         	"88F5181 B0"
#define MV_5181_B1_REV          0x3
#define MV_5181_B1_ID           	((MV_5181_DEV_ID << 16) | MV_5181_B1_REV)
#define MV_5181_B1_NAME         	"88F5181 B1"
/* 0x5281 revisions*/
#define MV_5281_A0_REV          0x0
#define MV_5281_A0_ID           ((MV_5281_DEV_ID << 16) | MV_5281_A0_REV)
#define MV_5281_A0_NAME         "88F5281 A0"
#define MV_5281_B0_REV          0x1
#define MV_5281_B0_ID           ((MV_5281_DEV_ID << 16) | MV_5281_B0_REV)
#define MV_5281_B0_NAME         "88F5281 B0"
#define MV_5281_C0_REV          0x2
#define MV_5281_C0_ID           ((MV_5281_DEV_ID << 16) | MV_5281_C0_REV)
#define MV_5281_C0_NAME         "88F5281 C0"
#define MV_5281_C1_REV          0x3
#define MV_5281_C1_ID           ((MV_5281_DEV_ID << 16) | MV_5281_C1_REV)
#define MV_5281_C1_NAME         "88F5281 C1"
#define MV_5281_D0_REV          0x4
#define MV_5281_D0_ID           ((MV_5281_DEV_ID << 16) | MV_5281_D0_REV)
#define MV_5281_D0_NAME         "88F5281 D0"
#define MV_5281_D1_REV          0x5
#define MV_5281_D1_ID           ((MV_5281_DEV_ID << 16) | MV_5281_D1_REV)
#define MV_5281_D1_NAME         "88F5281 D1"
#define MV_5281_D2_REV          0x6
#define MV_5281_D2_ID           ((MV_5281_DEV_ID << 16) | MV_5281_D2_REV)
#define MV_5281_D2_NAME         "88F5281 D2"
/* 0x5182 revisions*/
#define MV_5182_A0_REV          0x0
#define MV_5182_A0_ID           ((MV_5182_DEV_ID << 16) | MV_5182_A0_REV)
#define MV_5182_A0_NAME         "88F5182 A0"
#define MV_5182_A1_REV          0x1
#define MV_5182_A1_ID           ((MV_5182_DEV_ID << 16) | MV_5182_A1_REV)
#define MV_5182_A1_NAME         "88F5182 A1"
#define MV_5182_A2_REV          0x2
#define MV_5182_A2_ID           ((MV_5182_DEV_ID << 16) | MV_5182_A2_REV)
#define MV_5182_A2_NAME         "88F5182 A2"
/* 0x5181L revisions*/
#define MV_5181L_A0_REV         0x8
#define MV_5181L_A0_ID          ((MV_5181_DEV_ID << 16) | MV_5181L_A0_REV)
#define MV_5181L_A0_NAME        "88F5181L A0"
#define MV_5181L_A1_REV         0x9
#define MV_5181L_A1_ID          ((MV_5181_DEV_ID << 16) | MV_5181L_A1_REV)
#define MV_5181L_A1_NAME        "88F5181L A1"

/* 0x8660 revisions */
#define MV_8660_A0_REV          0x0
#define MV_8660_A0_ID           ((MV_8660_DEV_ID << 16) | MV_8660_A0_REV)
#define MV_8660_A0_NAME         "88W8660 A0"
#define MV_8660_A1_REV          0x1
#define MV_8660_A1_ID           ((MV_8660_DEV_ID << 16) | MV_8660_A1_REV)
#define MV_8660_A1_NAME         "88W8660 A1"

/* 0x5180N revisions */
#define MV_5180N_B1_REV         0x3
#define MV_5180N_B1_ID          ((MV_5180_DEV_ID << 16) | MV_5180N_B1_REV)
#define MV_5180N_B1_NAME        "88F5180N B1"

/* 0x5082 revisions*/
#define MV_5082_A2_REV          0x2
#define MV_5082_A2_ID           ((MV_5082_DEV_ID << 16) | MV_5082_A2_REV)
#define MV_5082_A2_NAME         "88F5082 A2"

/* 0x1282 revisions*/
#define MV_1281_A0_REV          0x0
#define MV_1281_A0_ID           ((MV_1281_DEV_ID << 16) | MV_1281_A0_REV)
#define MV_1281_A0_NAME         "88F1281 A0"

/* 0x6082 revisions*/
#define MV_6082_A0_REV          0x0
#define MV_6082_A0_ID           ((MV_6082_DEV_ID << 16) | MV_6082_A0_REV)
#define MV_6082_A0_NAME         "88F6082 A0"
#define MV_6082L_A0_NAME        "88F6082L A0"
#define MV_6082_A1_REV          0x1
#define MV_6082_A1_ID           ((MV_6082_DEV_ID << 16) | MV_6082_A1_REV)
#define MV_6082_A1_NAME         "88F6082 A1"

/* 0x6183 revisions*/
#define MV_6183_A0_REV          0x0
#define MV_6183_A0_ID           ((MV_6183_DEV_ID << 16) | MV_6183_A0_REV)
#define MV_6183_A0_NAME         "88F6183 A0"
#define MV_6183_1_REV          0x1
#define MV_6183_1_ID           ((MV_6183_DEV_ID << 16) | MV_6183_1_REV)
#define MV_6183_1_NAME         "88F6183 1"
#define MV_6183_A1_REV          0x2
#define MV_6183_A1_ID           ((MV_6183_DEV_ID << 16) | MV_6183_A1_REV)
#define MV_6183_A1_NAME         "88F6183 A1"
#define MV_6183_B0_REV          0x3
#define MV_6183_B0_ID           ((MV_6183_DEV_ID << 16) | MV_6183_B0_REV)
#define MV_6183_B0_NAME         "88F6183 B0"

#define XOR_UNIT_BASE(unit)     0x60900
#define MV_XOR_MAX_UNIT         1
#define MV_XOR_MAX_CHAN         2 /* totol channels for all units together*/
#define MV_XOR_MAX_CHAN_PER_UNIT        2 /* channels for units */


#if defined(MV_88F1181)
#include "mv88F1X81EnvSpec.h"
#elif defined(MV_88F1281)
#include "mv88F1281EnvSpec.h"
#elif defined(MV_88F5182)
#include "mv88F5182EnvSpec.h"
#elif defined(MV_88F5082)
#include "mv88F5082EnvSpec.h"
#elif defined(MV_88F5181L)
#include "mv88F5181LEnvSpec.h"
#elif defined(MV_88W8660)
#include "mv88w8660EnvSpec.h"
#elif defined(MV_88F5181)
#include "mv88F5X81EnvSpec.h"
#elif defined(MV_88F5180N)
#include "mv88F5180NEnvSpec.h"
#elif defined(MV_88F6082)
#include "mv88F6082EnvSpec.h"
#elif defined(MV_88F6183)
#include "mv88F6183EnvSpec.h"
#else
#error "No Soc defined"
#endif                                               

#ifndef MV_ASMLANGUAGE

/* This enumerator defines the Marvell Units ID      */ 
typedef enum _mvUnitId
{
    DRAM_UNIT_ID,
    PEX_UNIT_ID,
    PCI_UNIT_ID,
    ETH_GIG_UNIT_ID,
    ETH_UNM_UNIT_ID,
    USB_UNIT_ID,
    IDMA_UNIT_ID,
    XOR_UNIT_ID,
    SATA_UNIT_ID,
    TDM_UNIT_ID,
    UART_UNIT_ID,
    CESA_UNIT_ID,
    SPI_UNIT_ID,
    AUDIO_UNIT_ID,
    MAX_UNITS_ID,

}MV_UNIT_ID;

#endif
#endif /* __INCmvCtrlEnvSpech */
