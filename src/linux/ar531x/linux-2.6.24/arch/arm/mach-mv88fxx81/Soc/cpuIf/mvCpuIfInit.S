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

#define MV_ASMLANGUAGE
#include "mvOsAsm.h"
#include "mvBoardEnvSpec.h"
#include "mvCtrlEnvSpec.h"
#include "mvCpuIfConfig.h"
#include "mvPexRegs.h"


/*******************************************************************************
* mvCpuIfPreInit - Make early initialization of CPU interface.
*
* DESCRIPTION:
*       The function will initialize the CPU interface parameters that must 
*       be initialize before any BUS activity towards the DDR interface, 
*       which means it must be executed from ROM. Because of that, the function 
*       is implemented in assembly code.
*       The function configure the following CPU config register parameters:
*       1) CPU2MbusLTickDrv
*       2) CPU2MbusLTickSample.
*       NOTE: This function must be called AFTER the internal register 
*       base is modified to INTER_REGS_BASE.
*
* INPUT:        
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*       r11 holds return function address.
*******************************************************************************/
       .globl _mvCpuIfPreInit
_mvCpuIfPreInit:

        mov     r11, LR     		/* Save link register */
     
        /* This function is for Orion2 B0 and up only */
	
        /* Read device ID  */
        MV_REG_READ_ASM(r4, r5, PEX_CFG_DIRECT_ACCESS(0,PEX_DEVICE_AND_VENDOR_ID))
        mov     r4, r4, LSR #16         /* r4 = r4 >> 16 */
        
        ldr     r5, =MV_1181_DEV_ID
        cmp     r4, r5
        beq     done

        /* Return if Orion1 */
        ldr     r5, =MV_5181_DEV_ID
        cmp     r4, r5
        beq     done
        
        /* Return if Orion1 */
        ldr     r5, =MV_5182_DEV_ID
        cmp     r4, r5
        beq     done

	/* Return if Shark */
        ldr     r5, =MV_8660_DEV_ID
        cmp     r4, r5
        beq     done
                
        /* goto calcConfigReg if bigger than Orion2*/     
        ldr     r5, =MV_5281_DEV_ID
        cmp     r4, r5
        bne     calcConfigReg

        /* Return if Orion2 and device revision A0 */
        MV_REG_READ_ASM(r4, r5, PEX_CFG_DIRECT_ACCESS(0,PEX_CLASS_CODE_AND_REVISION_ID));
        ldr    r5, =0x000000FF                  /* Mask for calss ID */
        and    r4, r4, r5
        cmp    r4, #0
        beq    done
        
        
calcConfigReg:           
        /* Get the "sample on reset" register */
        MV_REG_READ_ASM (r4, r5, MPP_SAMPLE_AT_RESET)
        ldr    r5, =MSAR_ARMDDRCLCK_MASK
        and    r5, r4, r5
        tst    r4, #MSAR_ARMDDRCLCK_H_MASK
        beq    1f
        orr    r5, r5, #BIT8
1:
        ldr    r4, =CPU_2_MBUSL_DDR_CLK_1x2
        cmp    r5, #MSAR_ARMDDRCLCK_333_167
        beq    setConfigReg 
        cmp    r5, #MSAR_ARMDDRCLCK_400_200
        beq    setConfigReg 
        cmp    r5, #MSAR_ARMDDRCLCK_466_233
        beq    setConfigReg 
        cmp    r5, #MSAR_ARMDDRCLCK_500_250
        beq    setConfigReg 
        cmp    r5, #MSAR_ARMDDRCLCK_533_266
        beq    setConfigReg 
        
        ldr    r4, =CPU_2_MBUSL_DDR_CLK_1x3
        cmp    r5, #MSAR_ARMDDRCLCK_400_133
        beq    setConfigReg 
        cmp    r5, #MSAR_ARMDDRCLCK_500_167
        beq    setConfigReg 
        cmp    r5, #MSAR_ARMDDRCLCK_600_200
        beq    setConfigReg 
        cmp    r5, #MSAR_ARMDDRCLCK_550_183
        beq    setConfigReg 
        cmp    r5, #MSAR_ARMDDRCLCK_600_300
        beq    setConfigReg 
        cmp    r5, #MSAR_ARMDDRCLCK_450_150
        beq    setConfigReg 
        cmp    r5, #MSAR_ARMDDRCLCK_533_178
        beq    setConfigReg 
        cmp    r5, #MSAR_ARMDDRCLCK_575_192
        beq    setConfigReg 
        
        ldr    r4, =CPU_2_MBUSL_DDR_CLK_1x4
        cmp    r5, #MSAR_ARMDDRCLCK_533_133
        beq    setConfigReg 
        cmp    r5, #MSAR_ARMDDRCLCK_667_167
        beq    setConfigReg 
        cmp    r5, #MSAR_ARMDDRCLCK_800_200
        beq    setConfigReg 
        cmp    r5, #MSAR_ARMDDRCLCK_700_175
        beq    setConfigReg 
        cmp    r5, #MSAR_ARMDDRCLCK_733_183
        beq    setConfigReg 
        cmp    r5, #MSAR_ARMDDRCLCK_750_187
        beq    setConfigReg 
        cmp    r5, #MSAR_ARMDDRCLCK_775_194
        beq    setConfigReg 
        cmp    r5, #MSAR_ARMDDRCLCK_500_125
        beq    setConfigReg 
        cmp    r5, #MSAR_ARMDDRCLCK_600_150
        beq    setConfigReg 
        
                       
        /* Note; x4 and x5 ratios are not supported */
        ldr    r4, =0

setConfigReg:
        /* Read CPU Config register */
        MV_REG_READ_ASM (r7, r5, CPU_CONFIG_REG)
        ldr    r5, =~(CCR_CPU_2_MBUSL_TICK_DRV_MASK | CCR_CPU_2_MBUSL_TICK_SMPL_MASK)
        and    r7, r7, r5       /* Clear register fields */
        orr    r7, r7, r4       /* Set the values according to the findings */
        MV_REG_WRITE_ASM (r7, r5, CPU_CONFIG_REG)

done:                                                                
        mov     PC, r11         /* r11 is saved link register */
