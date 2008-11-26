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
#include "mvUsb.h"
#include "mvCpu.h"
#include "mvCpuIf.h"
#include "mvBoardEnvLib.h"
#include "mvGpp.h"

MV_TARGET usbAddrDecPrioTab[] = 
{
    SDRAM_CS0,
    SDRAM_CS1,
    SDRAM_CS2,
    SDRAM_CS3,
    PEX0_MEM,
    PCI0_MEM,
    TBL_TERM
};

MV_U32  mvUsbGetCapRegAddr(int devNo)
{
    return (INTER_REGS_BASE | MV_USB_CORE_CAP_LENGTH_REG(devNo));
}

#ifdef MV_USB_VOLTAGE_FIX


/* GPIO Settings for Back voltage problem workaround */
int  mvUsbGppInit(int dev)
{        
	MV_U32	regVal;
    int 	gppNo = mvBoardUSBVbusGpioPinGet(dev);
    
    /* DDR1 => gpp5, DDR2 => gpp1 */
    if(gppNo != -1)
    {
        /*mvOsPrintf("mvUsbGppInit: gppNo=%d\n", gppNo);*/

        /* MPP Control Register - set to GPP*/
        regVal = MV_REG_READ(mvMppRegGet((unsigned int)(gppNo/8)));
        regVal &= ~(0xf << ((gppNo%8)*4));
        MV_REG_WRITE(mvMppRegGet((unsigned int)(gppNo/8)), regVal);

        /* GPIO Data Out Enable Control Register - set to input*/
        regVal = MV_REG_READ(GPP_DATA_OUT_EN_REG(0));
        regVal |= (1<<gppNo);
        MV_REG_WRITE(GPP_DATA_OUT_EN_REG(0), regVal);

        /* GPIO Data In Polarity Register */
        regVal = MV_REG_READ(GPP_DATA_IN_POL_REG(0));
        regVal &= ~(1<<gppNo);
        MV_REG_WRITE(GPP_DATA_IN_POL_REG(0), regVal);

        regVal = MV_REG_READ(MV_USB_PHY_POWER_CTRL_REG(dev));
        regVal &= ~(7 << 24);
        MV_REG_WRITE(MV_USB_PHY_POWER_CTRL_REG(dev), regVal);
    }
    return gppNo;
}

void    mvUsbBackVoltageUpdate(int dev, int gppNo)
{
    MV_U32  gppData, regVal, gppInv;

    gppInv = MV_REG_READ(GPP_DATA_IN_POL_REG(0));
    gppData = MV_REG_READ(GPP_DATA_IN_REG(0));
    regVal = MV_REG_READ(MV_USB_PHY_POWER_CTRL_REG(dev));

    if( ((gppInv & (1 << gppNo)) == 0) && 
        ((gppData & (1 << gppNo)) != 0) )
    {
        /* VBUS appear */
        regVal |= (7 << 24);
        gppInv |= (1 << gppNo);
        /*mvOsPrintf("VBUS appear: dev=%d, gpp=%d\n", dev, gppNo);*/
    }
    else if( ((gppInv & (1 << gppNo)) != 0) && 
             ((gppData & (1 << gppNo)) != 0) )
    {
        /* VBUS disappear */
        regVal &= ~(7 << 24);
        gppInv &= ~(1 << gppNo);
        /*mvOsPrintf("VBUS disappear: dev=%d, gpp=%d\n", dev, gppNo);*/
    }
    else
    {
        /* No changes */
        return;
    }
    MV_REG_WRITE(MV_USB_PHY_POWER_CTRL_REG(dev), regVal);
    MV_REG_WRITE(GPP_DATA_IN_POL_REG(0), gppInv);
}
#endif /* MV_USB_VOLTAGE_FIX */


/* Perform calibration of USB Phy */
void    mvUsbAutoCalibration(int dev)
{
    MV_U32          phyVal, regVal; 
    volatile MV_U32 timeout;

    /* set bit 31 of register 0x454 (LB_TEST_SEL) – Force the phy to be in HS. */
    phyVal = MV_REG_READ( MV_USB_PHY_TEST_GROUP_CTRL_REG_1(dev) );
    MV_REG_WRITE( MV_USB_PHY_TEST_GROUP_CTRL_REG_1(dev), phyVal | (1 << 31) );

/*
    timeout = 1*(mvCpuPclkGet()/1000);
    while(timeout > 0)
        timeout--;
*/
    /* USB PHY Tx Control Register Register 0x420 */
    /* Run the auto calibration sequence bit12 (0->1->0) */
    regVal = MV_REG_READ(MV_USB_PHY_TX_CTRL_REG(dev)); 

    /*   Bit[12] = 1 (REG_RCAL_START = 1). */
    regVal |= (1 << 12);
    MV_REG_WRITE(MV_USB_PHY_TX_CTRL_REG(dev), regVal);

    timeout = 1*(mvCpuPclkGet()/1000);
    while(timeout > 0)
        timeout--;

    /* Skip Calibration for Orion1-A0, A1, B0 and Orion2-A0 */
    if( ( (mvCtrlModelGet() == MV_5181_DEV_ID) && (mvCtrlRevGet() < MV_5181_B1_REV) ) ||
        ( (mvCtrlModelGet() == MV_5281_DEV_ID) && (mvCtrlRevGet() < MV_5281_B0_REV) ) )
    {
        /* Do nothing */
        mvOsPrintf("Skip autocalibration on dev=%d: dev, model=%d, rev=%d\n", 
                dev, mvCtrlModelGet(), mvCtrlRevGet());
    }
    else
    {
        /* 1 to 0 transaction will start Calibration */
        /*   Bit[12] = 0 (REG_RCAL_START = 0). */
        regVal &= ~(1 << 12);
        MV_REG_WRITE(MV_USB_PHY_TX_CTRL_REG(dev), regVal);

        /* wait 200 milli sec */
        timeout = 30*(mvCpuPclkGet()/1000);
        while(timeout > 0)
            timeout--;
    /*    mvOsDelay(200);*/

        mvOsPrintf("Auto-calibrartion on USB Phy #%d is finished\n", dev);
    }

    /* clear bit 31 of register 0x454 (LB_TEST_SEL)  */
    phyVal &= ~(1 << 31);
    MV_REG_WRITE( MV_USB_PHY_TEST_GROUP_CTRL_REG_1(dev), phyVal);
}

/*******************************************************************************
* usbWinOverlapDetect - Detect USB address windows overlapping
*
* DESCRIPTION:
*       An unpredicted behaviur is expected in case USB address decode 
*       windows overlapps.
*       This function detects USB address decode windows overlapping of a 
*       specified window. The function does not check the window itself for 
*       overlapping. The function also skipps disabled address decode windows.
*
* INPUT:
*       winNum      - address decode window number.
*       pAddrDecWin - An address decode window struct.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE if the given address window overlap current address
*       decode map, MV_FALSE otherwise, MV_ERROR if reading invalid data 
*       from registers.
*
*******************************************************************************/
static MV_STATUS usbWinOverlapDetect(int dev, MV_U32 winNum, 
                                     MV_ADDR_WIN *pAddrWin)
{
    MV_U32          winNumIndex;
    MV_USB_DEC_WIN  addrDecWin;

    for(winNumIndex=0; winNumIndex<MV_USB_MAX_ADDR_DECODE_WIN; winNumIndex++)
    {
        /* Do not check window itself       */
        if (winNumIndex == winNum)
        {
            continue;
        }

        /* Get window parameters    */
        if (MV_OK != mvUsbWinGet(dev, winNumIndex, &addrDecWin))
        {
            mvOsPrintf("%s: ERR. TargetWinGet failed\n", __FUNCTION__);
            return MV_ERROR;
        }

        /* Do not check disabled windows    */
        if(addrDecWin.enable == MV_FALSE)
        {
            continue;
        }

        if (MV_TRUE == ctrlWinOverlapTest(pAddrWin, &(addrDecWin.addrWin)))
        {
            return MV_TRUE;
        }        
    }
    return MV_FALSE;
}


/*******************************************************************************
* mvUsbInit - Initialize IDMA engine
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
MV_STATUS   mvUsbInit(int dev, MV_BOOL isHost)
{
    int             winNum;
    MV_USB_DEC_WIN  usbWin;
    MV_CPU_DEC_WIN  cpuAddrDecWin;
    MV_U32          regVal, status, winPrioIndex = 0;

    mvDebugModuleEnable(MV_MODULE_USB, MV_TRUE);
    mvDebugModuleSetFlags(MV_MODULE_USB, 
						  MV_DEBUG_FLAG_INIT | 
						  MV_DEBUG_FLAG_ERR  | 
                          MV_DEBUG_FLAG_STATS);


    /* For the Orion Pos-NAS we have an GPP output selecting the mode of the USB */
    if(mvBoardIdGet() == RD_88F5181_POS_NAS)
    {
	    unsigned int gppNo = 0x5, temp; /* GPP USB Mode select */

    	/* MPP Control Register - set to GPP */
    	regVal = MV_REG_READ(mvMppRegGet((unsigned int)(gppNo/8)));
    	regVal &= ~(0xf << ((gppNo%8)*4));
    	MV_REG_WRITE(mvMppRegGet((unsigned int)(gppNo/8)), regVal);

        /* GPIO Data Out Enable Control Register - set to output*/
        regVal = MV_REG_READ(GPP_DATA_OUT_EN_REG(0));
        regVal &= ~(1<<gppNo);
        MV_REG_WRITE(GPP_DATA_OUT_EN_REG(0), regVal);

	    temp = MV_REG_READ(GPP_DATA_OUT_REG(0));

	    /* for host mode should be set to 0 */
    	if(isHost)
    	{
		    temp &= ~(1 << gppNo);
    	}
    	else
    	{
		    temp |= (1 << gppNo);
    	}
        MV_REG_WRITE(GPP_DATA_OUT_REG(0), temp);

        /* delay 10 msec */
        mvOsDelay(10);
    }

    /* Clear Interrupt Cause and Mask registers */
    MV_REG_WRITE(MV_USB_BRIDGE_INTR_CAUSE_REG(dev), 0);
    MV_REG_WRITE(MV_USB_BRIDGE_INTR_MASK_REG(dev), 0);

    /* Reset controller */
    regVal = MV_REG_READ(MV_USB_CORE_CMD_REG(dev));
    MV_REG_WRITE(MV_USB_CORE_CMD_REG(dev), regVal | MV_USB_CORE_CMD_RESET_MASK);
    while( MV_REG_READ(MV_USB_CORE_CMD_REG(dev)) & MV_USB_CORE_CMD_RESET_MASK);

    /********* Update USB PHY configuration **********/

    /* The new register 0x360 USB 2.0 IPG Metal Fix Register
     * dont' exists in the following chip revisions:
     * Orion1 B1 (id=0x0x5181, rev=3) and before
     * Orion1-VoIP A0 (id=0x0x5181, rev=8) 
     * Orion1-NAS A1 (id=0x5182, rev=1) and before
     * Orion2 B0 (id=0x5281, rev=1) and before
     */
    if( ((mvCtrlModelGet() == MV_5181_DEV_ID) && 
         ((mvCtrlRevGet() <= MV_5181_B1_REV) || (mvCtrlRevGet() == MV_5181L_A0_REV))) ||
        ((mvCtrlModelGet() == MV_5182_DEV_ID) && 
         (mvCtrlRevGet() <= MV_5182_A1_REV)) ||
        ((mvCtrlModelGet() == MV_5281_DEV_ID) &&
         (mvCtrlRevGet() <= MV_5281_B0_REV)) )
    {
        /* Do nothing */
    }
    else
    {
        /* Change value of new register 0x360 */
        regVal = MV_REG_READ(MV_USB_BRIDGE_IPG_REG(dev)); 

        /*  Change bits[14:8] - IPG for non Start of Frame Packets 
         *  from 0x9(default) to 0xC 
         */
        regVal &= ~(0x7F << 8);
        regVal |= (0xC << 8);
    
        MV_REG_WRITE(MV_USB_BRIDGE_IPG_REG(dev), regVal);
    }

    /******* USB 2.0 Power Control Register 0x400 *******/
	regVal= MV_REG_READ(MV_USB_PHY_POWER_CTRL_REG(dev));

    /* Bits 7:6 (BG_VSEL) = 0x1 */
	regVal &= ~(0x3 << 6);
	regVal |= (0x1 << 6);

	MV_REG_WRITE(MV_USB_PHY_POWER_CTRL_REG(dev),regVal);

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

	/* Force Auto calibration to 0x8 */

    /* Bit[13] = 0x1, (REG_EXT_RCAL_EN = 0x1) */
	regVal |= (1<<13);

    /* Bit[6:3] = 0x8 (IMP_CAL = 0x8) */
	regVal &= ~(0xf << 3);
	regVal |= (0x8 << 3);

	MV_REG_WRITE(MV_USB_PHY_TX_CTRL_REG(dev), regVal); 


    /******* USB PHY Rx Control Register 0x430 *******/
    regVal = MV_REG_READ(MV_USB_PHY_RX_CTRL_REG(dev)); 

    /* bit[8:9] - (DISCON_THRESHOLD ). */
    /* Orion1-A0/A1/B0 = 11, Orion2-A0 = 10, Orion1-B1 and Orion2-B0 later = 00 */
    regVal &= ~(0x3 << 8);
    if( (mvCtrlModelGet() == MV_5181_DEV_ID) && (mvCtrlRevGet() <= MV_5181_B0_REV) ) 
    {
	    regVal |= (0x3 << 8);
    }
    else if((mvCtrlModelGet() == MV_5281_DEV_ID) && (mvCtrlRevGet() == MV_5281_A0_REV) )
    {
        regVal |= (0x2 << 8);
    }

    /* bit[21] = 0 (CDR_FASTLOCK_EN = 0). */
    regVal &= ~(1 << 21);

    /* bit[27:26] = 00 ( EDGE_DET_SEL = 00). */
    regVal &= ~(0x3 << 26);
    
    /* Bits[31:30] = RXDATA_BLOCK_LENGHT = 0x3 */
    regVal |= (0x3 << 30);

    /* Bits 7:4 (SQ_THRESH) = 0x1 */
	regVal &= ~(0xf << 4);
	regVal |= (0x1 << 4);
	
    MV_REG_WRITE(MV_USB_PHY_RX_CTRL_REG(dev), regVal);

    /******* USB PHY IVREF Control Register 0x440 *******/
    regVal = MV_REG_READ(MV_USB_PHY_IVREF_CTRL_REG(dev)); 

    /*Bits[1:0] = 0x2 (PLLVDD12 = 0x2)*/
    regVal &= ~(0x3 << 0);
    regVal |= (0x2 << 0);
    
	/* Bits 5:4 (RXVDD) = 0x3; */
    regVal &= ~(0x3 << 4);
    regVal |= (0x3 << 4);

    /* Bit 19 (Reserved) = 0x0; */
	regVal &= ~(0x1 << 19);
    
    MV_REG_WRITE(MV_USB_PHY_IVREF_CTRL_REG(dev), regVal);

    /***** USB PHY TEST GROUP CONTROL Register: 0x450 *****/
    regVal = MV_REG_READ(MV_USB_PHY_TEST_GROUP_CTRL_REG_0(dev)); 

    /* bit[15] = 0 (REG_FIFO_SQ_RST = 0). */
    regVal &= ~(1 << 15);

    MV_REG_WRITE(MV_USB_PHY_TEST_GROUP_CTRL_REG_0(dev), regVal);
    	
    /* Go through all windows in user table until table terminator          */
    winNum = 0; 
    while( (usbAddrDecPrioTab[winPrioIndex] != TBL_TERM) && 
           (winNum < MV_USB_MAX_ADDR_DECODE_WIN) )
    {
        /* first get attributes from CPU If */
        status = mvCpuIfTargetWinGet(usbAddrDecPrioTab[winPrioIndex], 
									 &cpuAddrDecWin);

        if(MV_NO_SUCH == status)
        {
            winPrioIndex++;
            continue;
        }
		if (MV_OK != status)
		{
            mvOsPrintf("%s: ERR. mvCpuIfTargetWinGet failed\n", __FUNCTION__);
			return MV_ERROR;
		}

        if (cpuAddrDecWin.enable == MV_TRUE)
        {
            usbWin.addrWin.baseHigh = cpuAddrDecWin.addrWin.baseHigh;
            usbWin.addrWin.baseLow  = cpuAddrDecWin.addrWin.baseLow;
            usbWin.addrWin.size     = cpuAddrDecWin.addrWin.size;
            usbWin.enable           = MV_TRUE;
            usbWin.target           = usbAddrDecPrioTab[winPrioIndex];

            if(MV_OK != mvUsbWinSet(dev, winNum, &usbWin))
            {
                return MV_ERROR;
            }
            winNum++;
        }
        winPrioIndex++;         
    }
  
    regVal = MV_REG_READ(MV_USB_CORE_MODE_REG(dev)); 
    regVal &= ~MV_USB_CORE_MODE_MASK;
    if(isHost)
    {
        /* Set HOST mode */
        regVal |= MV_USB_CORE_MODE_HOST; 
    }
    else
    {
        /* Set DEVICE mode */
	    regVal |= MV_USB_CORE_MODE_DEVICE;
    }

    /* Workarround for USB underrun problem */
    regVal |= MV_USB_CORE_STREAM_DISABLE_MASK;

    MV_REG_WRITE(MV_USB_CORE_MODE_REG(dev), regVal); 

    return MV_OK;
}

/*******************************************************************************
* mvUsbWinSet - Set USB target address window
*
* DESCRIPTION:
*       This function sets a peripheral target (e.g. SDRAM bank0, PCI_MEM0) 
*       address window, also known as address decode window. 
*       After setting this target window, the USB will be able to access the 
*       target within the address window. 
*
* INPUT:
*       winNum      - USB target address decode window number.
*       pAddrDecWin - USB target window data structure.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_ERROR if address window overlapps with other address decode windows.
*       MV_BAD_PARAM if base address is invalid parameter or target is 
*       unknown.
*
*******************************************************************************/
MV_STATUS mvUsbWinSet(int dev, MV_U32 winNum, MV_USB_DEC_WIN *pAddrDecWin)
{
    MV_TARGET_ATTRIB    targetAttribs;
    MV_DEC_REGS         decRegs;

    /* Parameter checking   */
    if (winNum >= MV_USB_MAX_ADDR_DECODE_WIN)
    {
        mvOsPrintf("%s: ERR. Invalid win num %d\n",__FUNCTION__, winNum);
        return MV_BAD_PARAM;
    }
    
    /* Check if the requested window overlapps with current windows         */
    if (MV_TRUE == usbWinOverlapDetect(dev, winNum, &pAddrDecWin->addrWin))
    {
        mvOsPrintf("%s: ERR. Window %d overlap\n", __FUNCTION__, winNum);
        return MV_ERROR;
    }

	/* check if address is aligned to the size */
	if(MV_IS_NOT_ALIGN(pAddrDecWin->addrWin.baseLow, pAddrDecWin->addrWin.size))
	{
		mvOsPrintf("mvUsbWinSet:Error setting USB window %d to "\
				   "target %s.\nAddress 0x%08x is unaligned to size 0x%x.\n",
				   winNum,
				   mvCtrlTargetNameGet(pAddrDecWin->target), 
				   pAddrDecWin->addrWin.baseLow,
				   pAddrDecWin->addrWin.size);
		return MV_ERROR;
	}


    decRegs.baseReg = 0;
    decRegs.sizeReg = 0;

    if (MV_OK != mvCtrlAddrDecToReg(&(pAddrDecWin->addrWin),&decRegs))
    {
        mvOsPrintf("%s: mvCtrlAddrDecToReg Failed\n", __FUNCTION__);
        return MV_ERROR;
    }

    mvCtrlAttribGet(pAddrDecWin->target, &targetAttribs);
                                                                                                                         
    /* set attributes */
    decRegs.sizeReg &= ~MV_USB_WIN_ATTR_MASK;
    decRegs.sizeReg |= (targetAttribs.attrib << MV_USB_WIN_ATTR_OFFSET);

    /* set target ID */
    decRegs.sizeReg &= ~MV_USB_WIN_TARGET_MASK;
    decRegs.sizeReg |= (targetAttribs.targetId << MV_USB_WIN_TARGET_OFFSET);

    if (pAddrDecWin->enable == MV_TRUE)
    {
        decRegs.sizeReg |= MV_USB_WIN_ENABLE_MASK;
    }
    else
    {
        decRegs.sizeReg &= ~MV_USB_WIN_ENABLE_MASK;
    }

    MV_REG_WRITE( MV_USB_WIN_CTRL_REG(dev, winNum), decRegs.sizeReg);
    MV_REG_WRITE( MV_USB_WIN_BASE_REG(dev, winNum), decRegs.baseReg);
    
    return MV_OK;
}

/*******************************************************************************
* mvUsbWinGet - Get USB peripheral target address window.
*
* DESCRIPTION:
*       Get USB peripheral target address window.
*
* INPUT:
*       winNum - USB target address decode window number.
*
* OUTPUT:
*       pAddrDecWin - USB target window data structure.
*
* RETURN:
*       MV_ERROR if register parameters are invalid.
*
*******************************************************************************/
MV_STATUS mvUsbWinGet(int dev, MV_U32 winNum, MV_USB_DEC_WIN *pAddrDecWin)
{
    MV_DEC_REGS         decRegs;
    MV_TARGET_ATTRIB    targetAttrib;
                                                                                                                         
    /* Parameter checking   */
    if (winNum >= MV_USB_MAX_ADDR_DECODE_WIN)
    {
        mvOsPrintf("%s (dev=%d): ERR. Invalid winNum %d\n", 
                    __FUNCTION__, dev, winNum);
        return MV_NOT_SUPPORTED;
    }

    decRegs.baseReg = MV_REG_READ( MV_USB_WIN_BASE_REG(dev, winNum) );
    decRegs.sizeReg = MV_REG_READ( MV_USB_WIN_CTRL_REG(dev, winNum) );
 
    if (MV_OK != mvCtrlRegToAddrDec(&decRegs, &pAddrDecWin->addrWin) )
    {
        mvOsPrintf("%s: mvCtrlRegToAddrDec Failed\n", __FUNCTION__);
        return MV_ERROR; 
    }
       
    /* attrib and targetId */
    targetAttrib.attrib = (decRegs.baseReg & MV_USB_WIN_ATTR_MASK) >> 
		MV_USB_WIN_ATTR_OFFSET;
    targetAttrib.targetId = (decRegs.baseReg & MV_USB_WIN_TARGET_MASK) >> 
		MV_USB_WIN_TARGET_OFFSET;
 
    pAddrDecWin->target = mvCtrlTargetGet(&targetAttrib);

    /* Check if window is enabled   */
    if(decRegs.sizeReg & MV_USB_WIN_ENABLE_MASK) 
    {
        pAddrDecWin->enable = MV_TRUE;
    }
    else
    {
        pAddrDecWin->enable = MV_FALSE;
    }
    return MV_OK;
}

void        mvUsbRegs(int dev)
{
    int     win;

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
    mvOsPrintf("MV_USB_PHY_PLL_CTRL_REG         : 0x%X = 0x%08x\n", 
                MV_USB_PHY_PLL_CTRL_REG(dev), 
			   MV_REG_READ(MV_USB_PHY_PLL_CTRL_REG(dev)) );    
    mvOsPrintf("MV_USB_PHY_TX_CTRL_REG          : 0x%X = 0x%08x\n", 
                MV_USB_PHY_TX_CTRL_REG(dev), 
			   MV_REG_READ(MV_USB_PHY_TX_CTRL_REG(dev)) );    
    mvOsPrintf("MV_USB_PHY_RX_CTRL_REG          : 0x%X = 0x%08x\n", 
                MV_USB_PHY_RX_CTRL_REG(dev), 
			   MV_REG_READ(MV_USB_PHY_RX_CTRL_REG(dev)) );    
    mvOsPrintf("MV_USB_PHY_IVREF_CTRL_REG       : 0x%X = 0x%08x\n", 
                MV_USB_PHY_IVREF_CTRL_REG(dev), 
			   MV_REG_READ(MV_USB_PHY_IVREF_CTRL_REG(dev)) );    

    mvOsPrintf("MV_USB_PHY_TEST_CTRL_REG_0      : 0x%X = 0x%08x\n", 
                MV_USB_PHY_TEST_GROUP_CTRL_REG_0(dev), 
			   MV_REG_READ(MV_USB_PHY_TEST_GROUP_CTRL_REG_0(dev)) );    
    mvOsPrintf("MV_USB_PHY_TEST_CTRL_REG_1      : 0x%X = 0x%08x\n", 
                MV_USB_PHY_TEST_GROUP_CTRL_REG_1(dev), 
			   MV_REG_READ(MV_USB_PHY_TEST_GROUP_CTRL_REG_1(dev)) );    

    mvOsPrintf("MV_USB_PHY_STATUS_REG           : 0x%X = 0x%08x\n", 
                MV_USB_PHY_STATUS_REG(dev), 
			   MV_REG_READ(MV_USB_PHY_STATUS_REG(dev)) );    

    mvOsPrintf("MV_USB_PHY_FIFO_REG             : 0x%X = 0x%08x\n", 
                MV_USB_PHY_FIFO_REG(dev), 
			   MV_REG_READ(MV_USB_PHY_FIFO_REG(dev)) );                

    mvOsPrintf("\n");
    for(win=0; win<MV_USB_MAX_ADDR_DECODE_WIN; win++)
    {
        mvOsPrintf("Win #%d: CTRL_REG (0x%X) = 0x%08x\n", 
                    win, MV_USB_WIN_CTRL_REG(dev, win), 
				   MV_REG_READ(MV_USB_WIN_CTRL_REG(dev, win)));
        mvOsPrintf("        BASE_REG (0x%X) = 0x%08x\n",
                    MV_USB_WIN_BASE_REG(dev, win), 
				   MV_REG_READ(MV_USB_WIN_BASE_REG(dev, win)) );    
    } 
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


