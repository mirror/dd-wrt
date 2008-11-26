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

#ifndef __INCethphyregsh
#define __INCethphyregsh

#include "ctrlEnv/mvCtrlEnvSpec.h"

/* defines */
#define ETH_PHY_TIMEOUT		10000

/* registers offsetes defines */
#ifdef MV_88W8660
#define ETH_PHY_SMI_REG			(MV_ETH_REG_BASE(0) + 0x6010)
#else
#define ETH_PHY_SMI_REG			(MV_ETH_REG_BASE(0) + 0x004)
#endif

/* SMI register fields (ETH_PHY_SMI_REG) */

#define ETH_PHY_SMI_DATA_OFFS	        0 /* Data */
#define ETH_PHY_SMI_DATA_MASK	        (0xffff << ETH_PHY_SMI_DATA_OFFS)

#define ETH_PHY_SMI_DEV_ADDR_OFFS	16 /* PHY device address */
#define ETH_PHY_SMI_DEV_ADDR_MASK       (0x1f << ETH_PHY_SMI_DEV_ADDR_OFFS)

#define ETH_PHY_SMI_REG_ADDR_OFFS	21 /* PHY device register address */
#define ETH_PHY_SMI_REG_ADDR_MASK	(0x1f << ETH_PHY_SMI_REG_ADDR_OFFS)	  

#define ETH_PHY_SMI_OPCODE_OFFS	        26	/* Write/Read opcode */
#define ETH_PHY_SMI_OPCODE_MASK	        (3 << ETH_PHY_SMI_OPCODE_OFFS)
#define ETH_PHY_SMI_OPCODE_WRITE        (0 << ETH_PHY_SMI_OPCODE_OFFS)
#define ETH_PHY_SMI_OPCODE_READ         (1 << ETH_PHY_SMI_OPCODE_OFFS)

#define ETH_PHY_SMI_READ_VALID_BIT	    27	/* Read Valid  */
#define ETH_PHY_SMI_READ_VALID_MASK	    (1 << ETH_PHY_SMI_READ_VALID_BIT)

#define ETH_PHY_SMI_BUSY_BIT		    28  /* Busy */
#define ETH_PHY_SMI_BUSY_MASK		    (1 << ETH_PHY_SMI_BUSY_BIT)  

/* PHY registers and bits */
#define ETH_PHY_CTRL_REG                0
#define ETH_PHY_STATUS_REG              1
#define ETH_PHY_AUTONEGO_AD_REG		0x4
#define ETH_PHY_1000BASE_T_CTRL_REG	9
#define ETH_PHY_SPEC_STATUS_REG         17

/* ETH_PHY_CTRL_REG bits */
#define ETH_PHY_CTRL_SPEED_MSB_BIT      6
#define ETH_PHY_CTRL_SPEED_MSB_MASK     (1 << ETH_PHY_CTRL_SPEED_MSB_BIT)

#define ETH_PHY_CTRL_COLISION_TEST_BIT  7
#define ETH_PHY_CTRL_COLISION_TEST_MASK (1 << ETH_PHY_CTRL_COLISION_TEST_BIT)

#define ETH_PHY_CTRL_DUPLEX_BIT         8
#define ETH_PHY_CTRL_DUPLEX_MASK        (1 << ETH_PHY_CTRL_DUPLEX_BIT)

#define ETH_PHY_CTRL_AN_RESTART_BIT     9
#define ETH_PHY_CTRL_AN_RESTART_MASK    (1 << ETH_PHY_CTRL_AN_RESTART_BIT)

#define ETH_PHY_CTRL_ISOLATE_BIT        10
#define ETH_PHY_CTRL_ISOLATE_MASK       (1 << ETH_PHY_CTRL_ISOLATE_BIT)

#define ETH_PHY_CTRL_POWER_DOWN_BIT     11
#define ETH_PHY_CTRL_POWER_DOWN_MASK    (1 << ETH_PHY_CTRL_POWER_DOWN_BIT)

#define ETH_PHY_CTRL_AN_ENABLE_BIT      12
#define ETH_PHY_CTRL_AN_ENABLE_MASK     (1 << ETH_PHY_CTRL_AN_ENABLE_BIT)

#define ETH_PHY_CTRL_SPEED_LSB_BIT	13
#define ETH_PHY_CTRL_SPEED_LSB_MASK	(1 << ETH_PHY_CTRL_SPEED_LSB_BIT)

#define ETH_PHY_CTRL_LOOPBACK_BIT	14
#define ETH_PHY_CTRL_LOOPBACK_MASK	(1 << ETH_PHY_CTRL_LOOPBACK_BIT)

#define ETH_PHY_CTRL_RESET_BIT          15
#define ETH_PHY_CTRL_RESET_MASK         (1 << ETH_PHY_CTRL_RESET_BIT)

/* ETH_PHY_STATUS_REG bits */
#define ETH_PHY_STATUS_AN_DONE_BIT      5
#define ETH_PHY_STATUS_AN_DONE_MASK     (1 << ETH_PHY_STATUS_AN_DONE_BIT)

/* ETH_PHY_1000BASE_T_CTRL_REG bits */
#define ETH_PHY_1000BASE_ADVERTISE_OFFSET	8
#define ETH_PHY_1000BASE_ADVERTISE_MASK		(0x3 << ETH_PHY_1000BASE_ADVERTISE_OFFSET)

/* ETH_PHY_SPEC_STATUS_REG bits */
#define ETH_PHY_SPEC_STATUS_SPEED_OFFS		14
#define ETH_PHY_SPEC_STATUS_SPEED_MASK		(0x3 << ETH_PHY_SPEC_STATUS_SPEED_OFFS)

#define ETH_PHY_SPEC_STATUS_SPEED_10MBPS	(0x0 << ETH_PHY_SPEC_STATUS_SPEED_OFFS)
#define ETH_PHY_SPEC_STATUS_SPEED_100MBPS	(0x1 << ETH_PHY_SPEC_STATUS_SPEED_OFFS)
#define ETH_PHY_SPEC_STATUS_SPEED_1000MBPS	(0x2 << ETH_PHY_SPEC_STATUS_SPEED_OFFS)


#define ETH_PHY_SPEC_STATUS_DUPLEX_BIT		13
#define ETH_PHY_SPEC_STATUS_DUPLEX_MASK		(0x1 << ETH_PHY_SPEC_STATUS_DUPLEX_BIT)

#define ETH_PHY_SPEC_STATUS_LINK_BIT		10
#define ETH_PHY_SPEC_STATUS_LINK_MASK		(0x1 << ETH_PHY_SPEC_STATUS_LINK_BIT)

/* ETH_PHY_SPEC_STATUS_REG bits */
#define ETH_PHY_LED_ACT_LNK_DV  0x4109

#endif /* __INCethphyregsh */
