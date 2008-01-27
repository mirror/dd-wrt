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
#ifndef __INCmvXorSpech
#define __INCmvXorSpech

#ifdef __cplusplus
extern "C" {
#endif

/* General XOR */
#define XOR_MAX_ADDR_DEC_WIN	8	/* Maximum address decode windows		*/
#define XOR_MAX_OVERRIDE_WIN	4	/* Maximum address override windows		*/
#define XOR_MAX_REMAP_WIN       4	/* Maximum address arbiter windows		*/
#define MV_XOR_MAX_CHAN         2


typedef enum _mvXorOverrideTarget
{
    SRC_ADDR0, /* Source Address #0 Control */
    SRC_ADDR1, /* Source Address #1 Control */
    SRC_ADDR2, /* Source Address #2 Control */
    SRC_ADDR3, /* Source Address #3 Control */
    SRC_ADDR4, /* Source Address #4 Control */
    SRC_ADDR5, /* Source Address #5 Control */
    SRC_ADDR6, /* Source Address #6 Control */
    SRC_ADDR7, /* Source Address #7 Control */
    XOR_DST_ADDR, /* Destination Address Control */
    XOR_NEXT_DESC /* Next Descriptor Address Control */
}MV_XOR_OVERRIDE_TARGET;

/* defines */
#define XOR_UNIT_BASE                        0x60000

/* XOR Engine Control Register Map */
#define XOR_CHANNEL_ARBITER_REG             (XOR_UNIT_BASE+(0x900))
#define XOR_CONFIG_REG(chan)                (XOR_UNIT_BASE+(0x910 + ((chan)    * 4)))
#define XOR_ACTIVATION_REG(chan)            (XOR_UNIT_BASE+(0x920 + ((chan)    * 4)))
                                                                
/* XOR Engine Interrupt Register Map */                         
#define XOR_CAUSE_REG                       (XOR_UNIT_BASE+(0x930))             
#define XOR_MASK_REG           		        (XOR_UNIT_BASE+(0x940))             
#define XOR_ERROR_CAUSE_REG                 (XOR_UNIT_BASE+(0x950))             
#define XOR_ERROR_ADDR_REG                  (XOR_UNIT_BASE+(0x960))             
                                                                
/* XOR Engine Descriptor Register Map */                        
#define XOR_NEXT_DESC_PTR_REG(chan)       	(XOR_UNIT_BASE+(0xB00 + ((chan)    * 4)))
#define XOR_CURR_DESC_PTR_REG(chan)	   	    (XOR_UNIT_BASE+(0xB10 + ((chan)    * 4)))
#define XOR_BYTE_COUNT_REG(chan)          	(XOR_UNIT_BASE+(0xB20 + ((chan)    * 4)))
                                                                
/* XOR Engine Address Decoding Register Map */                  
#define XOR_WINDOW_CTRL_REG(chan)           (XOR_UNIT_BASE+(0xB40 + ((chan)    * 4)))
#define XOR_BASE_ADDR_REG(winNum)     		(XOR_UNIT_BASE+(0xB50 + ((winNum)  * 4)))
#define XOR_SIZE_MASK_REG(winNum)           (XOR_UNIT_BASE+(0xB70 + ((winNum)  * 4)))
#define XOR_HIGH_ADDR_REMAP_REG(winNum)     (XOR_UNIT_BASE+(0xB90 + ((winNum)  * 4)))
#define XOR_OVERRIDE_CTRL_REG(chan)     	(XOR_UNIT_BASE+(0xBA0 + ((chan)    * 4)))

/* XOR Engine ECC/MemInit Register Map */
#define XOR_DST_PTR_REG(chan)            	(XOR_UNIT_BASE+(0xBB0 + ((chan)    * 4)))
#define XOR_BLOCK_SIZE_REG(chan)            (XOR_UNIT_BASE+(0xBC0 + ((chan)    * 4)))
#define XOR_TIMER_MODE_CTRL_REG             (XOR_UNIT_BASE+(0xBD0))
#define XOR_TIMER_MODE_INIT_VAL_REG         (XOR_UNIT_BASE+(0xBD4))
#define XOR_TIMER_MODE_CURR_VAL_REG         (XOR_UNIT_BASE+(0xBD8))
#define XOR_INIT_VAL_LOW_REG                (XOR_UNIT_BASE+(0xBE0))
#define XOR_INIT_VAL_HIGH_REG               (XOR_UNIT_BASE+(0xBE4))

/* XOR Engine Debug Register Map */
#define XOR_DEBUG_REG                       (XOR_UNIT_BASE+(0x970))


/* XOR register fileds */


/* XOR Engine Channel Arbiter Register */
#define XECAR_SLICE_OFFS(sliceNum)          (sliceNum)
#define XECAR_SLICE_MASK(sliceNum)          (1 << (XECAR_SLICE_OFFS(sliceNum)))

/* XOR Engine [0..1] Configuration Registers (XExCR) */
#define XEXCR_OPERATION_MODE_OFFS           (0)
#define XEXCR_OPERATION_MODE_MASK           (7 << XEXCR_OPERATION_MODE_OFFS)
#define XEXCR_OPERATION_MODE_XOR            (0 << XEXCR_OPERATION_MODE_OFFS)
#define XEXCR_OPERATION_MODE_CRC            (1 << XEXCR_OPERATION_MODE_OFFS)
#define XEXCR_OPERATION_MODE_DMA            (2 << XEXCR_OPERATION_MODE_OFFS)
#define XEXCR_OPERATION_MODE_ECC            (3 << XEXCR_OPERATION_MODE_OFFS)
#define XEXCR_OPERATION_MODE_MEM_INIT       (4 << XEXCR_OPERATION_MODE_OFFS)

#define XEXCR_SRC_BURST_LIMIT_OFFS          (4)
#define XEXCR_SRC_BURST_LIMIT_MASK          (7 << XEXCR_SRC_BURST_LIMIT_OFFS)
#define XEXCR_DST_BURST_LIMIT_OFFS          (8)
#define XEXCR_DST_BURST_LIMIT_MASK          (7 << XEXCR_DST_BURST_LIMIT_OFFS)
#define XEXCR_DRD_RES_SWP_OFFS              (12)
#define XEXCR_DRD_RES_SWP_MASK              (1 << XEXCR_DRD_RES_SWP_OFFS)
#define XEXCR_DWR_REQ_SWP_OFFS              (13)
#define XEXCR_DWR_REQ_SWP_MASK              (1 << XEXCR_DWR_REQ_SWP_OFFS)
#define XEXCR_DES_SWP_OFFS                  (14)
#define XEXCR_DES_SWP_MASK                  (1 << XEXCR_DES_SWP_OFFS)
#define XEXCR_REG_ACC_PROTECT_OFFS          (15)
#define XEXCR_REG_ACC_PROTECT_MASK          (1 << XEXCR_REG_ACC_PROTECT_OFFS)


/* XOR Engine [0..1] Activation Registers (XExACTR) */
#define XEXACTR_XESTART_OFFS                (0)
#define XEXACTR_XESTART_MASK                (1 << XEXACTR_XESTART_OFFS)
#define XEXACTR_XESTOP_OFFS                 (1)
#define XEXACTR_XESTOP_MASK                 (1 << XEXACTR_XESTOP_OFFS)
#define XEXACTR_XEPAUSE_OFFS                (2)
#define XEXACTR_XEPAUSE_MASK                (1 << XEXACTR_XEPAUSE_OFFS)
#define XEXACTR_XERESTART_OFFS              (3)
#define XEXACTR_XERESTART_MASK              (1 << XEXACTR_XERESTART_OFFS)
#define XEXACTR_XESTATUS_OFFS               (4)
#define XEXACTR_XESTATUS_MASK               (3 << XEXACTR_XESTATUS_OFFS)
#define XEXACTR_XESTATUS_IDLE               (0 << XEXACTR_XESTATUS_OFFS)
#define XEXACTR_XESTATUS_ACTIVE             (1 << XEXACTR_XESTATUS_OFFS)
#define XEXACTR_XESTATUS_PAUSED             (2 << XEXACTR_XESTATUS_OFFS)

/* XOR Engine Interrupt Cause Register (XEICR) */
#define XEICR_CHAN_OFFS					16
#define XEICR_CAUSE_OFFS(chan)   		(chan * XEICR_CHAN_OFFS)
#define XEICR_CAUSE_MASK(chan, cause)   (1 << (cause + XEICR_CAUSE_OFFS(chan)))
#define XEICR_COMP_MASK_ALL				0x000f000f
#define XEICR_COMP_MASK(chan)			(0x000f << XEICR_CAUSE_OFFS(chan))
#define XEICR_ERR_MASK					0x03800380

/* XOR Engine Error Cause Register (XEECR) */
#define XEECR_ERR_TYPE_OFFS				0
#define XEECR_ERR_TYPE_MASK				(0x1f << XEECR_ERR_TYPE_OFFS)

/* XOR Engine Error Address Register (XEEAR) */
#define XEEAR_ERR_ADDR_OFFS                 (0)
#define XEEAR_ERR_ADDR_MASK                 (0xFFFFFFFF << XEEAR_ERR_ADDR_OFFS)

/* XOR Engine [0..1] Next Descriptor Pointer Register (XExNDPR) */
#define XEXNDPR_NEXT_DESC_PTR_OFFS          (0)
#define XEXNDPR_NEXT_DESC_PTR_MASK          (0xFFFFFFFF << XEXNDPR_NEXT_DESC_PTR_OFFS)

/* XOR Engine [0..1] Current Descriptor Pointer Register (XExCDPR) */
#define XEXCDPR_CURRENT_DESC_PTR_OFFS       (0)
#define XEXCDPR_CURRENT_DESC_PTR_MASK       (0xFFFFFFFF << XEXCDPR_CURRENT_DESC_PTR_OFFS)

/* XOR Engine [0..1] Byte Count Register (XExBCR) */
#define XEXBCR_BYTE_CNT_OFFS                (0)
#define XEXBCR_BYTE_CNT_MASK                (0xFFFFFFFF << XEXBCR_BYTE_CNT_OFFS)

/* XOR Engine [0..1] Window Control Registers (XExWCR) */
#define XEXWCR_WIN_EN_OFFS(winNum)          (winNum)
#define XEXWCR_WIN_EN_MASK(winNum)          (1 << (XEXWCR_WIN_EN_OFFS(winNum)))
#define XEXWCR_WIN_EN_ENABLE(winNum)        (1 << (XEXWCR_WIN_EN_OFFS(winNum)))
#define XEXWCR_WIN_EN_DISABLE(winNum)       (0 << (XEXWCR_WIN_EN_OFFS(winNum)))

#define XEXWCR_WIN_ACC_OFFS(winNum)         ((2 * winNum) + 16)
#define XEXWCR_WIN_ACC_MASK(winNum)         (3 << (XEXWCR_WIN_ACC_OFFS(winNum)))
#define XEXWCR_WIN_ACC_NO_ACC(winNum)       (0 << (XEXWCR_WIN_ACC_OFFS(winNum)))
#define XEXWCR_WIN_ACC_RO(winNum)           (1 << (XEXWCR_WIN_ACC_OFFS(winNum)))
#define XEXWCR_WIN_ACC_RW(winNum)           (3 << (XEXWCR_WIN_ACC_OFFS(winNum)))

/* XOR Engine Base Address Registers (XEBARx) */
#define XEBARX_TARGET_OFFS                  (0)
#define XEBARX_TARGET_MASK                  (0xF << XEBARX_TARGET_OFFS)
#define XEBARX_ATTR_OFFS                    (8)
#define XEBARX_ATTR_MASK                    (0xFF << XEBARX_ATTR_OFFS)
#define XEBARX_BASE_OFFS                    (16)
#define XEBARX_BASE_MASK                    (0xFFFF << XEBARX_BASE_OFFS)

/* XOR Engine Size Mask Registers (XESMRx) */
#define XESMRX_SIZE_MASK_OFFS               (16)
#define XESMRX_SIZE_MASK_MASK               (0xFFFF << XESMRX_SIZE_MASK_OFFS)

/* XOR Engine High Address Remap Register (XEHARRx1) */
#define XEHARRX_REMAP_OFFS                  (0)
#define XEHARRX_REMAP_MASK                  (0xFFFFFFFF << XEHARRX_REMAP_OFFS)

/* XOR Engine [0..1] Address Override Control Register (XExAOCR) */
#define XEXAOCR_OVR_EN_OFFS(target)         (3 * target)
#define XEXAOCR_OVR_EN_MASK(target)         (1 << (XEXAOCR_OVR_EN_OFFS(target)))
#define XEXAOCR_OVR_PTR_OFFS(target)        ((3 * target) + 1)
#define XEXAOCR_OVR_PTR_MASK(target)        (3 << (XEXAOCR_OVR_PTR_OFFS(target)))
#define XEXAOCR_OVR_BAR(winNum,target)      (winNum << (XEXAOCR_OVR_PTR_OFFS(target)))

/* XOR Engine [0..1] Destination Pointer Register (XExDPR0) */
#define XEXDPR_DST_PTR_OFFS                 (0)
#define XEXDPR_DST_PTR_MASK                 (0xFFFFFFFF << XEXDPR_DST_PTR_OFFS)
#define XEXDPR_DST_PTR_XOR_MASK             (0x3F)
#define XEXDPR_DST_PTR_DMA_MASK             (0x1F)
#define XEXDPR_DST_PTR_CRC_MASK             (0x1F)

/* XOR Engine[0..1] Block Size Registers (XExBSR) */
#define XEXBSR_BLOCK_SIZE_OFFS              (0)
#define XEXBSR_BLOCK_SIZE_MASK              (0xFFFFFFFF << XEXBSR_BLOCK_SIZE_OFFS)
#define XEXBSR_BLOCK_SIZE_MIN_VALUE         (128)
#define XEXBSR_BLOCK_SIZE_MAX_VALUE         (0xFFFFFFFF)

/* XOR Engine Timer Mode Control Register (XETMCR) */
#define XETMCR_TIMER_EN_OFFS                (0)
#define XETMCR_TIMER_EN_MASK                (1 << XETMCR_TIMER_EN_OFFS)
#define XETMCR_TIMER_EN_ENABLE              (1 << XETMCR_TIMER_EN_OFFS)
#define XETMCR_TIMER_EN_DISABLE             (0 << XETMCR_TIMER_EN_OFFS)
#define XETMCR_SECTION_SIZE_CTRL_OFFS       (8)
#define XETMCR_SECTION_SIZE_CTRL_MASK       (0x1F << XETMCR_SECTION_SIZE_CTRL_OFFS)
#define XETMCR_SECTION_SIZE_MIN_VALUE       (7)
#define XETMCR_SECTION_SIZE_MAX_VALUE       (31)

/* XOR Engine Timer Mode Initial Value Register (XETMIVR) */
#define XETMIVR_TIMER_INIT_VAL_OFFS         (0)
#define XETMIVR_TIMER_INIT_VAL_MASK         (0xFFFFFFFF << XETMIVR_TIMER_INIT_VAL_OFFS)

/* XOR Engine Timer Mode Current Value Register (XETMCVR) */
#define XETMCVR_TIMER_CRNT_VAL_OFFS         (0)
#define XETMCVR_TIMER_CRNT_VAL_MASK         (0xFFFFFFFF << XETMCVR_TIMER_CRNT_VAL_OFFS)

/* XOR Engine Initial Value Register Low (XEIVRL) */
#define XEIVRL_INIT_VAL_L_OFFS              (0)
#define XEIVRL_INIT_VAL_L_MASK              (0xFFFFFFFF << XEIVRL_INIT_VAL_L_OFFS)

/* XOR Engine Initial Value Register High (XEIVRH) */
#define XEIVRH_INIT_VAL_H_OFFS              (0)
#define XEIVRH_INIT_VAL_H_MASK              (0xFFFFFFFF << XEIVRH_INIT_VAL_H_OFFS)

/* XOR Engine Debug Register (XEDBR) */
#define XEDBR_PARITY_ERR_INSR_OFFS          (0)
#define XEDBR_PARITY_ERR_INSR_MASK          (1 << XEDBR_PARITY_ERR_INSR_OFFS)
#define XEDBR_XBAR_ERR_INSR_OFFS            (1)
#define XEDBR_XBAR_ERR_INSR_MASK            (1 << XEDBR_XBAR_ERR_INSR_OFFS)


#ifdef __cplusplus
}
#endif

#endif	/* __INCmvXorSpech */
