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


#ifndef __INCmvCpuIfconfigh
#define __INCmvCpuIfConfigh

#include "mvSysHwConfig.h"
#include "mvCpuIfRegs.h"

#if defined(MV_88F1181)

/* CPU control register map */
/* Set bits means value is about to change according to new value */
#define CPU_CONFIG_DEFAULT_MASK		(CCR_VEC_INIT_LOC_MASK	|	\
									CCR_AHB_ERROR_PROP_MASK)
									

#define CPU_CONFIG_DEFAULT			(CCR_VEC_INIT_LOC_FF00	)
		 
/* CPU Control and status defaults */
#define CPU_CTRL_STAT_DEFAULT_MASK		(CCSR_PCI_ACCESS_MASK)
									

#define CPU_CTRL_STAT_DEFAULT			(CCSR_PCI_ACCESS_ENABLE)
		 
#else

/* CPU control register map */
/* Set bits means value is about to change according to new value */
#define CPU_CONFIG_DEFAULT_MASK		(CCR_VEC_INIT_LOC_MASK	|	\
									CCR_AHB_ERROR_PROP_MASK)
									

#define CPU_CONFIG_DEFAULT			(CCR_VEC_INIT_LOC_FF00	)
		 
/* CPU Control and status defaults */
#define CPU_CTRL_STAT_DEFAULT_MASK		(CCSR_PCI_ACCESS_MASK)
									

#define CPU_CTRL_STAT_DEFAULT			(CCSR_PCI_ACCESS_ENABLE)

/* Ratio options for CPU to DDR */
#define CPU_2_DDR_CLK_1x1	    1
#define CPU_2_DDR_CLK_1x2	    2
#define CPU_2_DDR_CLK_1x3	    3
#define CPU_2_DDR_CLK_1x4	    4
#define CPU_2_DDR_CLK_1x5	    5
#define CPU_2_DDR_CLK_1x6	    6
#define CPU_2_DDR_CLK_1x7	    7

/* Default values for CPU to Mbus-L DDR Interface Tick Driver and 	*/
/* CPU to Mbus-L Tick Sample fields in CPU config register			*/
#define TICK_DRV_1x2	0
#define TICK_SMPL_1x2	0
#define TICK_DRV_1x3	1
#define TICK_SMPL_1x3	2
#define TICK_DRV_1x4	2
#define TICK_SMPL_1x4	2

#define CPU_2_MBUSL_DDR_CLK_1x2									\
		((TICK_DRV_1x2  << CCR_CPU_2_MBUSL_TICK_DRV_OFFS) | 	\
		 (TICK_SMPL_1x2 << CCR_CPU_2_MBUSL_TICK_SMPL_OFFS))
#define CPU_2_MBUSL_DDR_CLK_1x3									\
		 ((TICK_DRV_1x3  << CCR_CPU_2_MBUSL_TICK_DRV_OFFS) | 	\
		  (TICK_SMPL_1x3 << CCR_CPU_2_MBUSL_TICK_SMPL_OFFS))
#define CPU_2_MBUSL_DDR_CLK_1x4									\
		 ((TICK_DRV_1x4  << CCR_CPU_2_MBUSL_TICK_DRV_OFFS) | 	\
		  (TICK_SMPL_1x4 << CCR_CPU_2_MBUSL_TICK_SMPL_OFFS))
		 

#define CPU_FTDLL_IC_CONFIG_DEFAULT        0x1b
#define CPU_FTDLL_DC_CONFIG_DEFAULT        0x2
#endif /* #if defined(MV_88F1181) */


#endif /* __INCmvCpuIfConfigh */
