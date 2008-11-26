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

#ifndef __INCmvDeviceRegsH
#define __INCmvDeviceRegsH

#ifndef MV_ASMLANGUAGE
#include "ctrlEnv/mvCtrlEnvLib.h"
/* This enumerator describes the Marvell controller possible devices that   */
/* can be connected to its device interface.                                */
typedef enum _mvDevice
{
#if defined(MV_INCLUDE_DEVICE_CS0)
	DEV_CS0 = 0,    /* Device connected to dev CS[0]    */
#endif
#if defined(MV_INCLUDE_DEVICE_CS1)
	DEV_CS1 = 1,        /* Device connected to dev CS[1]    */
#endif
#if defined(MV_INCLUDE_DEVICE_CS2)
	DEV_CS2 = 2,        /* Device connected to dev CS[2]    */
#endif
#if defined(MV_INCLUDE_DEVICE_CS3)
	DEV_CS3 = 3,        /* Device connected to dev CS[2]    */
#endif
#if defined(MV_INCLUDE_DEVICE_CS4)
	DEV_CS4 = 4,        /* Device connected to BOOT dev    */
#endif
	MV_DEV_MAX_CS = MV_DEVICE_MAX_CS
}MV_DEVICE;

 
#endif /* MV_ASMLANGUAGE */


/* registers offsets */

#if defined(MV_88F5182) || defined(MV_88W8660) || defined(MV_88F5082)
#define DEV_NAND_CTRL_REG			0x104E8
#define DEV_RGMII_AC_TIMING_REG			0x104F0
#else
#define DEV_NAND_CTRL_REG			0x10470
#define DEV_RGMII_AC_TIMING_REG			0x104E8
#endif


#define DEV_INTERF_CTRL_REG				0x104C0
#define DEV_INTERF_XBAR_TIMEOUT_REG		0x104C4

#define DEV_INTR_CAUSE_REG				0x104D0
#define DEV_INTR_MASK_REG				0x104D4


/* Device Bank Parameters register fields (DBP_REG)*/
/* Boot Device Bank Parameters (DBP) register fields (DEV_BOOT_BANK_PARAM_REG)*/

#define DBP_TURNOFF_OFFS		0
#define DBP_TURNOFF_MASK		(0x7 << DBP_TURNOFF_OFFS)
#define MAX_DBP_TURNOFF			0x7

#define DBP_ACC2FIRST_OFFS		3
#define DBP_ACC2FIRST_MASK		(0xf << DBP_ACC2FIRST_OFFS)
#define MAX_DBP_ACC2FIRST		0xf

#define DBP_ACC2NEXT_OFFS		7
#define DBP_ACC2NEXT_MASK		(0xf << DBP_ACC2NEXT_OFFS)
#define MAX_DBP_ACC2NEXT			0xf


#define DBP_ALE2WR_OFFS			11
#define DBP_ALE2WR_MASK			(0x7 << DBP_ALE2WR_OFFS)
#define MAX_DBP_ALE2WR			0x7


#define DBP_WRLOW_OFFS			14
#define DBP_WRLOW_MASK			(0x7 << DBP_WRLOW_OFFS)
#define MAX_DBP_WRLOW			0x7


#define DBP_WRHIGH_OFFS			17
#define DBP_WRHIGH_MASK			(0x7 << DBP_WRHIGH_OFFS)
#define MAX_DBP_WRHIGH			0x7


#define DBP_DEVWIDTH_OFFS		20 /* Device Width */
#define DBP_DEVWIDTH_MASK		(0x3 << DBP_DEVWIDTH_OFFS)
#define DBP_DEVWIDTH_8BIT		(0x0 << DBP_DEVWIDTH_OFFS)
#define DBP_DEVWIDTH_16BIT		(0x1 << DBP_DEVWIDTH_OFFS)
#define DBP_DEVWIDTH_32BIT		(0x2 << DBP_DEVWIDTH_OFFS)


#define DBP_TURNOFFEXT			BIT22 /* TurnOff Extension */
#define DBP_ACC2FIRSTEXT		BIT23 /* Acc2First Extension */
#define DBP_ACC2NEXTEXT			BIT24 /* Acc2Next Extension */
#define DBP_ALE2WREXT			BIT25 /* ALE2Wr Extension */
#define DBP_WRLOWEXT			BIT26 /* WrLow Extension */
#define DBP_WRHIGHEXT			BIT27 /* WrHigh Extension */

#define DBP_BADRSKEW_OFFS		28
#define DBP_BADRSKEW_MASK		(0x3 << DBP_BADRSKEW_OFFS)
#define DBP_BADRSKEW_NOGAP		(0x0 << DBP_BADRSKEW_OFFS)
#define DBP_BADRSKEW_1CYCLE		(0x1 << DBP_BADRSKEW_OFFS)
#define DBP_BADRSKEW_2CYCLE		(0x2 << DBP_BADRSKEW_OFFS)

#define DBP_BADRSKEW_OFFS		28
#define DBP_BADRSKEW_MASK		(0x3 << DBP_BADRSKEW_OFFS)
#define DBP_BADRSKEW_NOGAP		(0x0 << DBP_BADRSKEW_OFFS)
#define DBP_BADRSKEW_1CYCLE		(0x1 << DBP_BADRSKEW_OFFS)
#define DBP_BADRSKEW_2CYCLE		(0x2 << DBP_BADRSKEW_OFFS)

#define DBP_RESRV_OFFS			30
#define DBP_RESRV_MASK			(0x3 << DBP_RESRV_OFFS)
#define DBP_RESRV_VAL			(0x2 << DBP_RESRV_OFFS)



/* Device Interface Control register fields (DIC) (DIC_REG)*/

#define DIC_TIMEOUT_OFFS 	0 /* Timeout Timer Preset Value. */
#define DIC_TIMEOUT_MASK 	(0xffff << DIC_TIMEOUT_OFFS)
#define MAX_DIC_TIMEOUT		0xffff

#define DIC_RDTRIG			BIT16 /* Read Trigger Control */
#define DIC_RES18			BIT18 /* must be 1 */
#define DIC_PERRPROP		BIT19 /* Parity error propagation enable */
#define DIC_PARSEL			BIT20 /* Even or Odd parity select */
#define DIC_FORCEPAREN		BIT21 /* Force Parity Enable */

#define DIC_RDDPPAR_OFFS	22 /* Read data path parity select */
#define DIC_RDDPPAR_MASK	(1 << DIC_RDDPPAR_OFFS)
#define DIC_RDDPPAR_EVEN	(0 << DIC_RDDPPAR_OFFS)
#define DIC_RDDPPAR_ODD		(1 << DIC_RDDPPAR_OFFS)

#define DIC_WRDPPAR_OFFS	23 /* Write data path parity select*/
#define DIC_WRDPPAR_MASK	(1 << DIC_WRDPPAR_OFFS)
#define DIC_WRDPPAR_EVEN	(0 << DIC_WRDPPAR_OFFS)
#define DIC_WRDPPAR_ODD		(1 << DIC_WRDPPAR_OFFS)

#define DIC_FORCEPAR_OFFS	24
#define DIC_FORCEPAR_MASK	(0xf << DIC_FORCEPAR_OFFS)
#define MAX_DIC_FORCEPAR	0xf

#define DIC_RES30_OFFS		30	/* must be 0 */
#define DIC_RES30_MASK		(0x3 << DIC_RES30_OFFS

/* Device Interface Crossbar Control Low (DICCL) register (DIC_L_XBAR_REG)	*/
/* Device Interface Crossbar Control High (DICCH) register (DIC_H_XBAR_REG)	*/

#define DICCL_OFFS(num)		(4*num)
#define DICCL_MASK(num)		(0xf << DICCL_OFFS(num))

#define DICCH_OFFS(num)		(4*(num-8))
#define DICCH__MASK(num)	(0xf << DICCH_OFFS(num))

#define DICC_CPU	0x2
#define DICC_PCI0	0x3
#define DICC_PCI1	0x4
#define DICC_MPSC	0x5
#define DICC_IDMA	0x6
#define DICC_GETH	0x7


#endif /* #ifndef __INCmvDeviceRegsH */
