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


#include "mvOs.h"
#include "mvDebug.h"
#include "mvDeviceId.h"
#include "usb/mvUsb.h"

/* ronen should be cleaned */
#include "cpu/mvCpu.h"
#include "ctrlEnv/sys/mvCpuIf.h"
#include "boardEnv/mvBoardEnvLib.h"
#include "gpp/mvGpp.h"


MV_U32  mvUsbGetCapRegAddr(int devNo)
{
    return (INTER_REGS_BASE | MV_USB_CORE_CAP_LENGTH_REG(devNo));
}

#ifdef MV_USB_VOLTAGE_FIX

/* GPIO Settings for Back voltage problem workaround */
MV_U8  mvUsbGppInit(int dev)
{
    MV_U32  regVal;
    MV_U8   gppNo = (MV_U8)mvBoardUSBVbusGpioPinGet(dev);
    MV_U8   gppVbusEnNo = (MV_U8)mvBoardUSBVbusEnGpioPinGet(dev);

    /* DDR1 => gpp5, DDR2 => gpp1 */
    if(gppNo != (MV_U8)N_A)
    {
        /*mvOsPrintf("mvUsbGppInit: gppNo=%d\n", gppNo);*/

        /* MPP Control Register - set to GPP*/
        regVal = MV_REG_READ(mvCtrlMppRegGet((unsigned int)(gppNo/8)));
        regVal &= ~(0xf << ((gppNo%8)*4));
        MV_REG_WRITE(mvCtrlMppRegGet((unsigned int)(gppNo/8)), regVal);


        if(gppNo < 32)
	{
		/* GPIO Data Out Enable Control Register - set to input*/
        	mvGppTypeSet(0, (1<<gppNo), MV_GPP_IN & (1<<gppNo) );
		/* GPIO Data In Polarity Register */
		mvGppPolaritySet(0, (1<<gppNo), (0<<gppNo) );
	}
	else
	{
		/* GPIO Data Out Enable Control Register - set to input*/
		mvGppTypeSet(1, (1<<(gppNo-32)), MV_GPP_IN & (1<<(gppNo-32)) );
		/* GPIO Data In Polarity Register */
		mvGppPolaritySet(1, (1<<(gppNo-32)), (0<<(gppNo-32)) );
	}

        /* GPIO Data In Polarity Register */
       /* mvGppPolaritySet(mppGrp, (1<<gppNo), (0<<gppNo) ); */

        regVal = MV_REG_READ(MV_USB_PHY_POWER_CTRL_REG(dev));
        regVal &= ~(7 << 24);
        MV_REG_WRITE(MV_USB_PHY_POWER_CTRL_REG(dev), regVal);
    }

    /* for device - reset vbus enable gpp, for host - set by default */
    if(gppVbusEnNo != (MV_U8)N_A)
    {
	/*mvOsPrintf("mvUsbGppInit: gppVbusEnNo = %d \n", gppVbusEnNo);*/

	regVal = MV_REG_READ(GPP_DATA_OUT_REG((gppVbusEnNo/32)));
	if(gppVbusEnNo < 32)
	{
		MV_REG_WRITE(GPP_DATA_OUT_REG(0), (regVal & ~(1 << gppVbusEnNo)));
	}
	else
	{
		MV_REG_WRITE(GPP_DATA_OUT_REG(1), (regVal & ~(1 << (gppVbusEnNo - 32))));
	}

    }

    return gppNo;
}

int    mvUsbBackVoltageUpdate(int dev, MV_U8 gppNo)
{
    int     vbusChange = 0;
    MV_U32  gppData, regVal, gppInv;

    if(gppNo < 32)
    {
    	gppInv = mvGppPolarityGet(0, (1 << gppNo));
	    gppData = mvGppValueGet(0, (1 << gppNo));
    }
    else
    {
	    gppInv = mvGppPolarityGet(1, (1 << (gppNo-32)));
	    gppData = mvGppValueGet(1, (1 << (gppNo-32)));
    }
    regVal = MV_REG_READ(MV_USB_PHY_POWER_CTRL_REG(dev));

    if( (gppInv == 0) &&
        (gppData != 0) )
    {
        /* VBUS appear */
        regVal |= (7 << 24);
	    
        if(gppNo < 32)
            gppInv |= (1 << gppNo);
	    else
		    gppInv |= (1 << (gppNo-32));

        /*mvOsPrintf("VBUS appear: dev=%d, gpp=%d\n", dev, gppNo);*/
        vbusChange = 1;
    }
    else if( (gppInv != 0) &&
             (gppData != 0) )
    {
        /* VBUS disappear */
        regVal &= ~(7 << 24);
	    
        if(gppNo < 32)
        	gppInv &= ~(1 << gppNo);
	    else
		    gppInv &= ~(1 << (gppNo-32));

        /*mvOsPrintf("VBUS disappear: dev=%d, gpp=%d\n", dev, gppNo);*/
        vbusChange = 2;
    }
    else
    {
        /* No changes */
        return vbusChange;
    }
    MV_REG_WRITE(MV_USB_PHY_POWER_CTRL_REG(dev), regVal);

    if(gppNo < 32)
	    mvGppPolaritySet(0, (1<<gppNo), gppInv);
    else
	    mvGppPolaritySet(1, (1<<(gppNo-32)), gppInv);

    return vbusChange;
}
#endif /* MV_USB_VOLTAGE_FIX */

/* USB Phy init (change from defaults) specific for 65nm (78100 78200 A0 and later) */
void    mvUsbPhy65nmNewInit(int dev)
{
    MV_U32          regVal;

    /******* USB PHY PLL Control Register 0x410 *******/
    regVal = MV_REG_READ(MV_USB_PHY_PLL_CTRL_REG(dev)); 
    
    /* bit[21]	(VCOCAL_ START) */ 
    regVal |= (0x1 << 21);
    MV_REG_WRITE(MV_USB_PHY_PLL_CTRL_REG(dev), regVal); 
    regVal &= ~(0x1 << 21);
    MV_REG_WRITE(MV_USB_PHY_PLL_CTRL_REG(dev), regVal); 
    /*-------------------------------------------------*/

    /******* USB PHY Tx Control Register Register 0x420 *******/
    regVal = MV_REG_READ(MV_USB_PHY_TX_CTRL_REG(dev)); 

    /* Force impedance auto calibrate */
	/* bit[12]	(REG_RCAL_START) = 1 */ 
    regVal |= (0x1 << 12);
    MV_REG_WRITE(MV_USB_PHY_TX_CTRL_REG(dev), regVal); 

    /* bit[12]	(REG_RCAL_START) = 0 */ 
    regVal &= ~(0x1 << 12);
    MV_REG_WRITE(MV_USB_PHY_TX_CTRL_REG(dev), regVal); 
    /*-------------------------------------------------*/

    /******* USB PHY Rx Control Register 0x430 *******/
    regVal = MV_REG_READ(MV_USB_PHY_RX_CTRL_REG(dev)); 

    /* bits[7:4]	SQ_THRESH	= 0x7 */
    regVal &= ~(0xf << 4);
    regVal |= (0x7 << 4);

    MV_REG_WRITE(MV_USB_PHY_RX_CTRL_REG(dev), regVal);
    /*-------------------------------------------------*/

    /******* USB PHY IVREF Control Register 0x440 *******/
    regVal = MV_REG_READ(MV_USB_PHY_IVREF_CTRL_REG(dev)); 
    /* bits[9:8]	TXVDD12 = 0x3 */
    regVal &= ~(0x3 << 8);
    regVal |= (0x3 << 8);
    
    MV_REG_WRITE(MV_USB_PHY_IVREF_CTRL_REG(dev), regVal);
    /*-------------------------------------------------*/
    /***** USB PHY TEST GROUP CONTROL Register: 0x450 *****/

    /* Nothing to change */
}

/* USB Phy init (change from defaults) specific for 65nm (78XX0 and 6281) */
void    mvUsbPhy65nmInit(int dev)
{
    MV_U32      regVal;

    /******* USB PHY PLL Control Register 0x410 *******/
    regVal = MV_REG_READ(MV_USB_PHY_PLL_CTRL_REG(dev)); 
	
    /* bit[14:12]	(ICP) = 6 */ 
    regVal &= ~(0x7 << 12);
    regVal |= (0x6 << 12);

    /* bit[12]	(KVCO_EXT) = 1 */ 
    regVal |= (0x1 << 22);

    MV_REG_WRITE(MV_USB_PHY_PLL_CTRL_REG(dev), regVal); 
    /*-------------------------------------------------*/

    /******* USB PHY Tx Control Register Register 0x420 *******/
    regVal = MV_REG_READ(MV_USB_PHY_TX_CTRL_REG(dev)); 
	
    /* bit[11]	(LOWVDD_EN)	= 1 */ 
    regVal |= (0x1 << 11);

    /* bit[12]	(REG_RCAL_START) = 1 */ 
    regVal |= (0x1 << 12);

    /* bit[21]	(TX_BLOCK_EN)	= 0 */ 
    regVal &= ~(0x1 << 21);

    /* bit[31]  (HS_STRESS_CTRL) = 0 */
    regVal &= ~(0x1 << 31);
	
    MV_REG_WRITE(MV_USB_PHY_TX_CTRL_REG(dev), regVal); 
    /* Force impedance auto calibrate */
    /* bit[12]	(REG_RCAL_START) = 0 */ 
    regVal &= ~(0x1 << 12);
    MV_REG_WRITE(MV_USB_PHY_TX_CTRL_REG(dev), regVal); 
    /*-------------------------------------------------*/

    /******* USB PHY Rx Control Register 0x430 *******/
    regVal = MV_REG_READ(MV_USB_PHY_RX_CTRL_REG(dev)); 
    
    /* bits[3:2]	LPL_COEF	= 0x0 (1/8) */
    regVal &= ~(0x3 << 2);

    /* bits[7:4]	SQ_THRESH	= 0x7 */
    regVal &= ~(0xf << 4);
    regVal |= (0x7 << 4);

    /* bits[16:15]	REG_SQ_LENGTH	= 0x1 */
    regVal &= ~(0x3 << 15);
    regVal |= (0x1 << 15);

    /* bit[21]	CDR_FASTLOCK_EN	= 0x0 */
    regVal &= ~(0x1 << 21);

    /* bits[27:26]	EDGE_DET	= 0x0 (1 Tran)*/
    regVal &= ~(0x3 << 26);

    MV_REG_WRITE(MV_USB_PHY_RX_CTRL_REG(dev), regVal);
    /*-------------------------------------------------*/

	/******* USB PHY IVREF Control Register 0x440 *******/
	regVal = MV_REG_READ(MV_USB_PHY_IVREF_CTRL_REG(dev)); 

	/* bits[9:8]	TXVDD12 = 0x3 */
	regVal &= ~(0x3 << 8);
	regVal |= (0x3 << 8);
    
    MV_REG_WRITE(MV_USB_PHY_IVREF_CTRL_REG(dev), regVal);
    /*-------------------------------------------------*/

    /***** USB PHY TEST GROUP CONTROL Register: 0x450 *****/
    regVal = MV_REG_READ(MV_USB_PHY_TEST_GROUP_CTRL_REG_0(dev)); 

    /* bit[15]	REG_FIFO_SQ_RST	= 0x0 */
    regVal &= ~(0x1 << 15);

    MV_REG_WRITE(MV_USB_PHY_TEST_GROUP_CTRL_REG_0(dev), regVal);
    /*-------------------------------------------------*/
}

/* USB Phy init (change from defaults) specific for 90nm (6183 and later) */
void    mvUsbPhy90nmInit(int dev)
{
    MV_U32          regVal;

    /******* USB 2.0 Power Control Register 0x400 *******/
    regVal= MV_REG_READ(MV_USB_PHY_POWER_CTRL_REG(dev));

    /* Bits 7:6 (BG_VSEL) = 0x0 */
    regVal &= ~(0x3 << 6);

    MV_REG_WRITE(MV_USB_PHY_POWER_CTRL_REG(dev),regVal);
    /*-------------------------------------------------*/

    /******* USB PHY Tx Control Register Register 0x420 *******/
    regVal = MV_REG_READ(MV_USB_PHY_TX_CTRL_REG(dev));

    /* bit[21]	(TX_BLK_EN) = 0 for B0 only */
    if( (mvCtrlModelGet() == MV_6183_DEV_ID))
    	regVal &= ~(0x1 << 21);


    /* Force Auto calibration */
    /* Bit[13] = 0x1, (REG_EXT_RCAL_EN = 0x1) */
    regVal |= (0x1<<13);

    /* bit[11]	(LOWVDD_EN)	= 1 */
    regVal |= (0x1 << 11);

    /* Bit[6:3] (IMP_CAL) = 0x8 for A0 and B0*/
    /* Bit[6:3] (IMP_CAL) = 0xf for A1 */
    regVal &= ~(0xf << 3);
    regVal |= (0x8 << 3);

    if( (mvCtrlModelGet() == MV_6183_DEV_ID))
    {
	regVal &= ~(0x7);
	regVal |= (0x4);
    }

    MV_REG_WRITE(MV_USB_PHY_TX_CTRL_REG(dev), regVal);
    /*-------------------------------------------------*/

    /******* USB PHY Rx Control Register 0x430 *******/
    regVal = MV_REG_READ(MV_USB_PHY_RX_CTRL_REG(dev));

    /* bits[3:2]	LPF_COEF	= 0x0 */
    regVal &= ~(0x3 << 2);

    /* bits[7:4]	SQ_THRESH	= 0x0 */
    /* bits[7:4]	SQ_THRESH = 0x3 for A1 and above */
    regVal &= ~(0xf << 4);

    /* bits[16:15]	REG_SQ_LENGTH	= 0x1 */
    regVal &= ~(0x3 << 15);
    regVal |= (0x1 << 15);

    /* bit[21]	CDR_FASTLOCK_EN	= 0x0 */
    regVal &= ~(0x1 << 21);

    /* bits[27:26]	EDGE_DET_SEL	= 0x0 */
    regVal &= ~(0x3 << 26);

    MV_REG_WRITE(MV_USB_PHY_RX_CTRL_REG(dev), regVal);
	/*-------------------------------------------------*/

    /******* USB PHY IVREF Control Register 0x440 *******/
    regVal = MV_REG_READ(MV_USB_PHY_IVREF_CTRL_REG(dev));

    /* bits[7:6]	RXVDD18	            = 0x0 */
    regVal &= ~(0x3 << 6);

    /* bits[11]	SQ_CM_SEL = 0x1 for 6183 B0 only */
    if( (mvCtrlModelGet() == MV_6183_DEV_ID))
	regVal |= (0x1 << 11);

    /* bit[24]	    REG_TEST_SUSPENDM	= 0x1 */
    regVal |= (0x1 << 24);

    MV_REG_WRITE(MV_USB_PHY_IVREF_CTRL_REG(dev), regVal);
    /*-------------------------------------------------*/

    /***** USB PHY TEST GROUP CONTROL Register: 0x450 *****/
    regVal = MV_REG_READ(MV_USB_PHY_TEST_GROUP_CTRL_REG_0(dev));

    /* bit[15]	REG_FIFO_SQ_RST	= 0x0 */
    regVal &= ~(0x1 << 15);

    MV_REG_WRITE(MV_USB_PHY_TEST_GROUP_CTRL_REG_0(dev), regVal);
    /*-------------------------------------------------*/
}

/* USB Phy init (change from defaults) for 150nm chips:
 *  - 645xx, 646xx
 *  - 51xx, 52xx
 *  - 6082
 */
void    mvUsbPhyInit(int dev)
{
    MV_U32          regVal;

    /* GL# USB-9 */
    /******* USB 2.0 Power Control Register 0x400 *******/
    regVal= MV_REG_READ(MV_USB_PHY_POWER_CTRL_REG(dev));

    /* Bits 7:6 (BG_VSEL) = 0x1 */
    regVal &= ~(0x3 << 6);
    regVal |= (0x1 << 6);

    MV_REG_WRITE(MV_USB_PHY_POWER_CTRL_REG(dev),regVal);

    /* GL# USB-1 */
    /******* USB PHY Tx Control Register Register 0x420 *******/
    regVal = MV_REG_READ(MV_USB_PHY_TX_CTRL_REG(dev));

    if( (mvCtrlModelGet() == MV_5181_DEV_ID) && (mvCtrlRevGet() <= MV_5181_A1_REV) )
    {
        /* For OrionI A1/A0 rev:  Bit[21] = 0 (TXDATA_BLOCK_EN  = 0). */
        regVal &= ~(1 << 21);
    }
    else
    {
        regVal |= (1 << 21);
    }

    /* Force Auto calibration */

    /* Bit[13] = 0x1, (REG_EXT_RCAL_EN = 0x1) */
    regVal |= (1<<13);

    /* Bits[2:0] (TxAmp)
     *      64560, 64660, 6082, 5181, 5182, etc = 0x4
     *      5281                                = 0x3
     */
    if(mvCtrlModelGet() == MV_5281_DEV_ID)
    {
        regVal &= ~(0x7 << 0);
        regVal |= (0x3 << 0);
    }
    else
    {
        regVal &= ~(0x7 << 0);
        regVal |= (0x4 << 0);
    }

    /* Bit[6:3] (IMP_CAL)
     *      64560, 64660 = 0xA
     *      Others       = 0x8
     *
     */
    regVal &= ~(0xf << 3);
    if( (mvCtrlModelGet() == MV64560_DEV_ID) ||
        (mvCtrlModelGet() == MV64660_DEV_ID))
    {
        regVal |= (0xA << 3);
    }
    else
    {
        regVal |= (0x8 << 3);
    }

    MV_REG_WRITE(MV_USB_PHY_TX_CTRL_REG(dev), regVal);

    /* GL# USB-3 GL# USB-9 */
    /******* USB PHY Rx Control Register 0x430 *******/
    regVal = MV_REG_READ(MV_USB_PHY_RX_CTRL_REG(dev));

    /* bit[8:9] - (DISCON_THRESHOLD ). */
    /* 88F5181-A0/A1/B0 = 11, 88F5281-A0 = 10, all other 00 */
    /* 64660-A0 = 10 */
    regVal &= ~(0x3 << 8);
    if( (mvCtrlModelGet() == MV_5181_DEV_ID) && (mvCtrlRevGet() <= MV_5181_B0_REV) )
    {
        regVal |= (0x3 << 8);
    }
    else if(mvCtrlModelGet() == MV64660_DEV_ID)
    {
        regVal |= (0x2 << 8);
    }

    /* bit[21] = 0 (CDR_FASTLOCK_EN = 0). */
    regVal &= ~(1 << 21);

    /* bit[27:26] = 00 ( EDGE_DET_SEL = 00). */
    regVal &= ~(0x3 << 26);

    /* Bits[31:30] = RXDATA_BLOCK_LENGHT = 0x3 */
    regVal |= (0x3 << 30);

    /* Bits 7:4 (SQ_THRESH)
     *      64560, 64660                = 0x1
     *      5181, 5182, 5281, 6082, etc = 0x0
     */
    regVal &= ~(0xf << 4);
    if( (mvCtrlModelGet() == MV64560_DEV_ID) ||
        (mvCtrlModelGet() == MV64660_DEV_ID))
    {
        regVal |= (0x1 << 4);
    }

    MV_REG_WRITE(MV_USB_PHY_RX_CTRL_REG(dev), regVal);

    /* GL# USB-3 GL# USB-9 */
    /******* USB PHY IVREF Control Register 0x440 *******/
    regVal = MV_REG_READ(MV_USB_PHY_IVREF_CTRL_REG(dev));

    /*Bits[1:0] = 0x2 (PLLVDD12 = 0x2)*/
    regVal &= ~(0x3 << 0);
    regVal |= (0x2 << 0);

    /* Bits 5:4 (RXVDD) = 0x3; */
    regVal &= ~(0x3 << 4);
    regVal |= (0x3 << 4);


    /* <VPLLCAL> bits[17:16] = 0x1 */
    if( (mvCtrlModelGet() == MV64560_DEV_ID) ||
        (mvCtrlModelGet() == MV64660_DEV_ID))
    {
        regVal &= ~(0x3 << 16);
        regVal |= (0x1 << 16);
    }

    /* <ISAMPLE_SEL> bits[19:18] = 0x2 */
    regVal &= ~(0x3 << 18);
    regVal |= (0x2 << 18);

    /* <SAMPLER_CTRL> bit[20] = 0x0 */
    regVal &= ~(0x1 << 20);

    /* <ICHGPBUF_SEL> bit[21] = 0x1 */
    regVal |= (0x1 << 21);

    MV_REG_WRITE(MV_USB_PHY_IVREF_CTRL_REG(dev), regVal);

    /***** USB PHY TEST GROUP CONTROL Register: 0x450 *****/
    regVal = MV_REG_READ(MV_USB_PHY_TEST_GROUP_CTRL_REG_0(dev));

    /* bit[15] = 0 (REG_FIFO_SQ_RST = 0). */
    regVal &= ~(1 << 15);

    /* <REG_FIFO_OVUF_SEL> bit[17] = 0x1 */
    if( (mvCtrlModelGet() == MV64560_DEV_ID) ||
        (mvCtrlModelGet() == MV64660_DEV_ID))
    {
        regVal |= (1 << 17);
    }

    MV_REG_WRITE(MV_USB_PHY_TEST_GROUP_CTRL_REG_0(dev), regVal);
}

/*******************************************************************************
* mvUsbHalInit - Initialize USB engine
*
* DESCRIPTION:
*       This function initialize USB unit. It set the default address decode
*       windows of the unit.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_ERROR if setting fail.
*******************************************************************************/
MV_STATUS   mvUsbHalInit(int dev, MV_BOOL isHost)
{
    MV_U32          regVal;

    /* Clear Interrupt Cause and Mask registers */
    MV_REG_WRITE(MV_USB_BRIDGE_INTR_CAUSE_REG(dev), 0);
    MV_REG_WRITE(MV_USB_BRIDGE_INTR_MASK_REG(dev), 0);

    /* Reset controller */
    regVal = MV_REG_READ(MV_USB_CORE_CMD_REG(dev));
    MV_REG_WRITE(MV_USB_CORE_CMD_REG(dev), regVal | MV_USB_CORE_CMD_RESET_MASK);
    while( MV_REG_READ(MV_USB_CORE_CMD_REG(dev)) & MV_USB_CORE_CMD_RESET_MASK);

    /* Clear bit 4 in USB bridge control register for enableing core byte swap */
    if((mvCtrlModelGet() == MV64560_DEV_ID) || (mvCtrlModelGet() == MV64660_DEV_ID))
    {
        MV_REG_WRITE(MV_USB_BRIDGE_CTRL_REG(dev),(MV_REG_READ(MV_USB_BRIDGE_CTRL_REG(dev))
                           & ~MV_USB_BRIDGE_CORE_BYTE_SWAP_MASK));
    }

    /* GL# USB-10 */
    /* The new register 0x360 USB 2.0 IPG Metal Fix Register
     * dont' exists in the following chip revisions:
     * OrionN B1 (id=0x5180, rev 3)
     * Orion1 B1 (id=0x5181, rev=3) and before
     * Orion1-VoIP A0 (id=0x5181, rev=8)
     * Orion1-NAS A1 (id=0x5182, rev=1) and before
     * Orion2 B0 (id=0x5281, rev=1) and before
     */
    if( ((mvCtrlModelGet() == MV_5181_DEV_ID) &&
         ((mvCtrlRevGet() <= MV_5181_B1_REV) || (mvCtrlRevGet() == MV_5181L_A0_REV))) ||
        ((mvCtrlModelGet() == MV_5182_DEV_ID) &&
         (mvCtrlRevGet() <= MV_5182_A1_REV)) ||
        ((mvCtrlModelGet() == MV_5180_DEV_ID) &&
         (mvCtrlRevGet() <= MV_5180N_B1_REV)) )
    {
        /* Do nothing */
    }
    else
    {
        /* Change value of new register 0x360 */
        regVal = MV_REG_READ(MV_USB_BRIDGE_IPG_REG(dev));

        /*  Change bits[14:8] - IPG for non Start of Frame Packets
         *  from 0x9(default) to 0xD
         */
        regVal &= ~(0x7F << 8);
        regVal |= (0xD << 8);

        MV_REG_WRITE(MV_USB_BRIDGE_IPG_REG(dev), regVal);
    }

    /********* Update USB PHY configuration **********/
    if(	(mvCtrlModelGet() == MV_78100_DEV_ID) || 
	    (mvCtrlModelGet() == MV_78200_DEV_ID) )
    {
        mvUsbPhy65nmNewInit(dev);
    }
    else if((mvCtrlModelGet() == MV_6281_DEV_ID) ||
	        (mvCtrlModelGet() == MV_6192_DEV_ID) ||
	        (mvCtrlModelGet() == MV_6180_DEV_ID) || 
	        (mvCtrlModelGet() == MV_78XX0_DEV_ID))
    {
        mvUsbPhy65nmInit(dev);
    }
    else if( mvCtrlModelGet() == MV_6183_DEV_ID )
    {
        mvUsbPhy90nmInit(dev);
    }
    else
    {
        mvUsbPhyInit(dev);
    }

    /* Set Mode register (Stop and Reset USB Core before) */
    /* Stop the controller */
    regVal = MV_REG_READ(MV_USB_CORE_CMD_REG(dev));
    regVal &= ~MV_USB_CORE_CMD_RUN_MASK;
    MV_REG_WRITE(MV_USB_CORE_CMD_REG(dev), regVal);

    /* Reset the controller to get default values */
    regVal = MV_REG_READ(MV_USB_CORE_CMD_REG(dev));
    regVal |= MV_USB_CORE_CMD_RESET_MASK;
    MV_REG_WRITE(MV_USB_CORE_CMD_REG(dev), regVal);

    /* Wait for the controller reset to complete */
    do
    {
        regVal = MV_REG_READ(MV_USB_CORE_CMD_REG(dev));
    } while (regVal & MV_USB_CORE_CMD_RESET_MASK);

    /* Set USB_MODE register */
    if(isHost)
    {
        regVal = MV_USB_CORE_MODE_HOST;
    }
    else
    {
        regVal = MV_USB_CORE_MODE_DEVICE | MV_USB_CORE_SETUP_LOCK_DISABLE_MASK;
    }

#if (MV_USB_VERSION == 0)
    regVal |= MV_USB_CORE_STREAM_DISABLE_MASK;
#endif

    MV_REG_WRITE(MV_USB_CORE_MODE_REG(dev), regVal);

    return MV_OK;
}


void        mvUsbPowerDown(int dev)
{
    MV_U32  regVal;

    /* Stop USB Controller core */
    regVal = MV_REG_READ(MV_USB_CORE_CMD_REG(dev));
    if(regVal & MV_USB_CORE_CMD_RUN_MASK)
    {
        mvOsPrintf("USB #%d:  Warning USB core was not disabled\n", dev);
        regVal &= ~MV_USB_CORE_CMD_RUN_MASK;
        MV_REG_WRITE(MV_USB_CORE_CMD_REG(dev), regVal);
    }

    /* Power Down USB PHY */
    regVal = MV_REG_READ(MV_USB_PHY_POWER_CTRL_REG(dev));
    regVal &= ~(MV_USB_PHY_POWER_UP_MASK | MV_USB_PHY_PLL_POWER_UP_MASK);

    MV_REG_WRITE(MV_USB_PHY_POWER_CTRL_REG(dev), regVal);
}

void        mvUsbPowerUp(int dev)
{
    MV_U32  regVal;

    /* Power Up USB PHY */
    regVal = MV_REG_READ(MV_USB_PHY_POWER_CTRL_REG(dev));
    regVal |= (MV_USB_PHY_POWER_UP_MASK | MV_USB_PHY_PLL_POWER_UP_MASK);

    MV_REG_WRITE(MV_USB_PHY_POWER_CTRL_REG(dev), regVal);

    /* Start USB core */
    regVal = MV_REG_READ(MV_USB_CORE_CMD_REG(dev));
    if(regVal & MV_USB_CORE_CMD_RUN_MASK)
    {
        mvOsPrintf("USB #%d:  Warning USB core is enabled\n", dev);
    }
    else
    {
        regVal |= MV_USB_CORE_CMD_RUN_MASK;
        MV_REG_WRITE(MV_USB_CORE_CMD_REG(dev), regVal);
    }
}


void        mvUsbRegs(int dev)
{
    mvOsPrintf("\n\tUSB-%d Bridge Registers\n\n", dev);

    mvOsPrintf("MV_USB_BRIDGE_CTRL_REG          : 0x%X = 0x%08x\n",
                    MV_USB_BRIDGE_CTRL_REG(dev),
                    MV_REG_READ(MV_USB_BRIDGE_CTRL_REG(dev)) );

    mvOsPrintf("MV_USB_BRIDGE_INTR_MASK_REG     : 0x%X = 0x%08x\n",
               MV_USB_BRIDGE_INTR_MASK_REG(dev),
               MV_REG_READ(MV_USB_BRIDGE_INTR_MASK_REG(dev)));

    mvOsPrintf("MV_USB_BRIDGE_INTR_CAUSE_REG    : 0x%X = 0x%08x\n",
               MV_USB_BRIDGE_INTR_CAUSE_REG(dev),
               MV_REG_READ(MV_USB_BRIDGE_INTR_CAUSE_REG(dev)));

    mvOsPrintf("MV_USB_BRIDGE_ERROR_ADDR_REG    : 0x%X = 0x%08x\n",
               MV_USB_BRIDGE_ERROR_ADDR_REG(dev),
               MV_REG_READ(MV_USB_BRIDGE_ERROR_ADDR_REG(dev)));

    mvOsPrintf("\n\tUSB-%d PHY Registers\n\n", dev);

    mvOsPrintf("MV_USB_PHY_POWER_CTRL_REG       : 0x%X = 0x%08x\n",
                MV_USB_PHY_POWER_CTRL_REG(dev),
               MV_REG_READ(MV_USB_PHY_POWER_CTRL_REG(dev)) );

    mvOsPrintf("MV_USB_PHY_TX_CTRL_REG          : 0x%X = 0x%08x\n",
                MV_USB_PHY_TX_CTRL_REG(dev),
               MV_REG_READ(MV_USB_PHY_TX_CTRL_REG(dev)) );
    mvOsPrintf("MV_USB_PHY_RX_CTRL_REG          : 0x%X = 0x%08x\n",
                MV_USB_PHY_RX_CTRL_REG(dev),
               MV_REG_READ(MV_USB_PHY_RX_CTRL_REG(dev)) );
    mvOsPrintf("MV_USB_PHY_IVREF_CTRL_REG       : 0x%X = 0x%08x\n",
                MV_USB_PHY_IVREF_CTRL_REG(dev),
               MV_REG_READ(MV_USB_PHY_IVREF_CTRL_REG(dev)) );

    mvOsPrintf("\n");

}

void        mvUsbCoreRegs(int dev, MV_BOOL isHost)
{
    mvOsPrintf("\n\t USB-%d Core %s Registers\n\n",
                    dev, isHost ? "HOST" : "DEVICE");

    mvOsPrintf("MV_USB_CORE_ID_REG                  : 0x%X = 0x%08x\n",
                MV_USB_CORE_ID_REG(dev),
               MV_REG_READ(MV_USB_CORE_ID_REG(dev)) );

    mvOsPrintf("MV_USB_CORE_GENERAL_REG             : 0x%X = 0x%08x\n",
                MV_USB_CORE_GENERAL_REG(dev),
               MV_REG_READ(MV_USB_CORE_GENERAL_REG(dev)) );

    if(isHost)
    {
        mvOsPrintf("MV_USB_CORE_HOST_REG                : 0x%X = 0x%08x\n",
                    MV_USB_CORE_HOST_REG(dev),
                   MV_REG_READ(MV_USB_CORE_HOST_REG(dev)) );

        mvOsPrintf("MV_USB_CORE_TTTX_BUF_REG            : 0x%X = 0x%08x\n",
                    MV_USB_CORE_TTTX_BUF_REG(dev),
                   MV_REG_READ(MV_USB_CORE_TTTX_BUF_REG(dev)) );

        mvOsPrintf("MV_USB_CORE_TTRX_BUF_REG            : 0x%X = 0x%08x\n",
                    MV_USB_CORE_TTRX_BUF_REG(dev),
                   MV_REG_READ(MV_USB_CORE_TTRX_BUF_REG(dev)) );
    }
    else
    {
        mvOsPrintf("MV_USB_CORE_DEVICE_REG              : 0x%X = 0x%08x\n",
                    MV_USB_CORE_DEVICE_REG(dev),
                   MV_REG_READ(MV_USB_CORE_DEVICE_REG(dev)) );
    }

    mvOsPrintf("MV_USB_CORE_TX_BUF_REG              : 0x%X = 0x%08x\n",
                MV_USB_CORE_TX_BUF_REG(dev),
               MV_REG_READ(MV_USB_CORE_TX_BUF_REG(dev)) );

    mvOsPrintf("MV_USB_CORE_RX_BUF_REG              : 0x%X = 0x%08x\n",
                MV_USB_CORE_RX_BUF_REG(dev),
               MV_REG_READ(MV_USB_CORE_RX_BUF_REG(dev)) );

    mvOsPrintf("MV_USB_CORE_CAP_LENGTH_REG          : 0x%X = 0x%08x\n",
                MV_USB_CORE_CAP_LENGTH_REG(dev),
               MV_REG_READ(MV_USB_CORE_CAP_LENGTH_REG(dev)) );

    if(isHost)
    {
        mvOsPrintf("MV_USB_CORE_CAP_HCS_PARAMS_REG      : 0x%X = 0x%08x\n",
                    MV_USB_CORE_CAP_HCS_PARAMS_REG(dev),
                   MV_REG_READ(MV_USB_CORE_CAP_HCS_PARAMS_REG(dev)) );

        mvOsPrintf("MV_USB_CORE_CAP_HCC_PARAMS_REG      : 0x%X = 0x%08x\n",
                    MV_USB_CORE_CAP_HCC_PARAMS_REG(dev),
                   MV_REG_READ(MV_USB_CORE_CAP_HCC_PARAMS_REG(dev)) );
    }
    else
    {
        mvOsPrintf("MV_USB_CORE_CAP_DCI_VERSION_REG     : 0x%X = 0x%08x\n",
                    MV_USB_CORE_CAP_DCI_VERSION_REG(dev),
                   MV_REG_READ(MV_USB_CORE_CAP_DCI_VERSION_REG(dev)) );

        mvOsPrintf("MV_USB_CORE_CAP_DCC_PARAMS_REG      : 0x%X = 0x%08x\n",
                    MV_USB_CORE_CAP_DCC_PARAMS_REG(dev),
                   MV_REG_READ(MV_USB_CORE_CAP_DCC_PARAMS_REG(dev)) );
    }

    mvOsPrintf("MV_USB_CORE_CMD_REG                 : 0x%X = 0x%08x\n",
                MV_USB_CORE_CMD_REG(dev),
               MV_REG_READ(MV_USB_CORE_CMD_REG(dev)) );

    mvOsPrintf("MV_USB_CORE_STATUS_REG              : 0x%X = 0x%08x\n",
                MV_USB_CORE_STATUS_REG(dev),
               MV_REG_READ(MV_USB_CORE_STATUS_REG(dev)) );

    mvOsPrintf("MV_USB_CORE_INTR_REG                : 0x%X = 0x%08x\n",
                MV_USB_CORE_INTR_REG(dev),
               MV_REG_READ(MV_USB_CORE_INTR_REG(dev)) );

    mvOsPrintf("MV_USB_CORE_FRAME_INDEX_REG         : 0x%X = 0x%08x\n",
                MV_USB_CORE_FRAME_INDEX_REG(dev),
               MV_REG_READ(MV_USB_CORE_FRAME_INDEX_REG(dev)) );

    mvOsPrintf("MV_USB_CORE_MODE_REG                : 0x%X = 0x%08x\n",
                MV_USB_CORE_MODE_REG(dev),
               MV_REG_READ(MV_USB_CORE_MODE_REG(dev)) );

    if(isHost)
    {
        mvOsPrintf("MV_USB_CORE_PERIODIC_LIST_BASE_REG  : 0x%X = 0x%08x\n",
                    MV_USB_CORE_PERIODIC_LIST_BASE_REG(dev),
                   MV_REG_READ(MV_USB_CORE_PERIODIC_LIST_BASE_REG(dev)) );

        mvOsPrintf("MV_USB_CORE_ASYNC_LIST_ADDR_REG     : 0x%X = 0x%08x\n",
                    MV_USB_CORE_ASYNC_LIST_ADDR_REG(dev),
                   MV_REG_READ(MV_USB_CORE_ASYNC_LIST_ADDR_REG(dev)) );

        mvOsPrintf("MV_USB_CORE_CONFIG_FLAG_REG         : 0x%X = 0x%08x\n",
                    MV_USB_CORE_CONFIG_FLAG_REG(dev),
                   MV_REG_READ(MV_USB_CORE_CONFIG_FLAG_REG(dev)) );
    }
    else
    {
        int     numEp, ep;
        MV_U32  epCtrlVal;

        mvOsPrintf("MV_USB_CORE_DEV_ADDR_REG            : 0x%X = 0x%08x\n",
                    MV_USB_CORE_DEV_ADDR_REG(dev),
                   MV_REG_READ(MV_USB_CORE_DEV_ADDR_REG(dev)) );

        mvOsPrintf("MV_USB_CORE_ENDPOINT_LIST_ADDR_REG  : 0x%X = 0x%08x\n",
                    MV_USB_CORE_ENDPOINT_LIST_ADDR_REG(dev),
                   MV_REG_READ(MV_USB_CORE_ENDPOINT_LIST_ADDR_REG(dev)) );

        mvOsPrintf("MV_USB_CORE_ENDPT_SETUP_STAT_REG    : 0x%X = 0x%08x\n",
                    MV_USB_CORE_ENDPT_SETUP_STAT_REG(dev),
                    MV_REG_READ(MV_USB_CORE_ENDPT_SETUP_STAT_REG(dev)) );

        mvOsPrintf("MV_USB_CORE_ENDPT_PRIME_REG         : 0x%X = 0x%08x\n",
                    MV_USB_CORE_ENDPT_PRIME_REG(dev),
                    MV_REG_READ(MV_USB_CORE_ENDPT_PRIME_REG(dev)) );

        mvOsPrintf("MV_USB_CORE_ENDPT_FLUSH_REG         : 0x%X = 0x%08x\n",
                    MV_USB_CORE_ENDPT_FLUSH_REG(dev),
                    MV_REG_READ(MV_USB_CORE_ENDPT_FLUSH_REG(dev)) );

        mvOsPrintf("MV_USB_CORE_ENDPT_STATUS_REG        : 0x%X = 0x%08x\n",
                    MV_USB_CORE_ENDPT_STATUS_REG(dev),
                    MV_REG_READ(MV_USB_CORE_ENDPT_STATUS_REG(dev)) );

        mvOsPrintf("MV_USB_CORE_ENDPT_COMPLETE_REG      : 0x%X = 0x%08x\n",
                    MV_USB_CORE_ENDPT_COMPLETE_REG(dev),
                    MV_REG_READ(MV_USB_CORE_ENDPT_COMPLETE_REG(dev)) );

        numEp = MV_REG_READ(MV_USB_CORE_CAP_DCC_PARAMS_REG(dev)) & 0x1F;

        for(ep=0; ep<numEp; ep++)
        {
            epCtrlVal = MV_REG_READ( MV_USB_CORE_ENDPT_CTRL_REG(dev, ep));
            if(epCtrlVal == 0)
                continue;

            mvOsPrintf("MV_USB_CORE_ENDPT_CTRL_REG[%02d]      : 0x%X = 0x%08x\n",
                    ep, MV_USB_CORE_ENDPT_CTRL_REG(dev, ep), epCtrlVal);
        }
    }

    mvOsPrintf("MV_USB_CORE_PORTSC_REG              : 0x%X = 0x%08x\n",
                MV_USB_CORE_PORTSC_REG(dev),
               MV_REG_READ(MV_USB_CORE_PORTSC_REG(dev)) );
}


