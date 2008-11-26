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


/* includes */
#include "mvCommon.h"
#include "mvCtrlEnvLib.h"
#include "mvCtrlEnvPadCalibration.h"
#include "ctrlEnv/sys/mvCpuIf.h"

#if defined(MV_INCLUDE_PCI) || defined(MV_INCLUDE_PEX)
#include "pci-if/mvPciIf.h"
#endif

#if defined(MV_INCLUDE_PCI)
#include "pci/mvPci.h"
#include "ctrlEnv/sys/mvSysPci.h"
#endif

#if defined(MV_INCLUDE_PEX)
#include "ctrlEnv/sys/mvSysPex.h"
#endif

#if defined(MV_INCLUDE_IDMA)
#include "ctrlEnv/sys/mvSysIdma.h"
#endif

#if defined(MV_INCLUDE_GIG_ETH)
#include "ctrlEnv/sys/mvSysGbe.h"
#endif

#if defined(MV_INCLUDE_XOR)
#include "ctrlEnv/sys/mvSysXor.h"
#endif

#if defined (MV_INCLUDE_INTEG_MFLASH)
#include "mflash/mvPMFlashSpec.h"
#endif

#if defined(MV_INCLUDE_SATA)
#include "ctrlEnv/sys/mvSysSata.h"
#endif

#if defined(MV_INCLUDE_USB)
#include "ctrlEnv/sys/mvSysUsb.h"
#endif

/* defines  */
#ifdef MV_DEBUG
	#define DB(x)	x
#else
	#define DB(x)
#endif	

/* Global paramters initial value '-1' to indicate they are uninitialized.	*/
/* In case of data section is located in ROM, this value will not be able 	*/
/* to change.																*/
MV_U32 ctrlDevModel = -1;
MV_U32 ctrlDevRev   = -1;


/*******************************************************************************
* mv64xxxInit - Initialize Marvell controller environment.
*
* DESCRIPTION:
*       This function get environment information and initialize controller
*       internal/external environment. For example
*       1) MPP settings according to board MPP macros.
*		NOTE: It is the user responsibility to shut down all DMA channels
*		in device and disable controller sub units interrupts during 
*		boot process.
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
*******************************************************************************/
MV_STATUS mvCtrlEnvInit(MV_VOID)
{
    MV_U32 mppGroup;
	/* Read MPP group from board level and assign to MPP register */
	for (mppGroup = 0; mppGroup < MV_MPP_MAX_GROUP; mppGroup++)
	{
		MV_REG_WRITE(mvCtrlMppRegGet(mppGroup), mvBoardMppGet(mppGroup));
	}
 	
#if defined(MV_88F6183)
	/* Add pad calibration for 6183 */
	mvCtrlEnvPadCalibrationInit();
#endif

	return MV_OK;
}

/*******************************************************************************
* mvCtrlMppRegGet - return reg address of mpp group
*
* DESCRIPTION:
*
* INPUT:
*       mppGroup - MPP group.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_U32 - Register address.
*
*******************************************************************************/
MV_U32 mvCtrlMppRegGet(MV_U32 mppGroup)
{
        MV_U32 ret;

        switch(mppGroup){
                case (0):       ret = MPP_CONTROL_REG0;
                                break;
                case (1):       ret = MPP_CONTROL_REG1;
                                break;
#if !defined(MV_88F6082)
                case (2):       ret = MPP_CONTROL_REG2;
                                break;
#if defined(MV_88F6183)
                case (3):       ret = MPP_CONTROL_REG3;
                                break;
#else
                case (3):       ret = DEV_MULTI_CONTROL;
                                break;
#endif
#endif
                default:        ret = MPP_CONTROL_REG0;
                                break;
        }
        return ret;
}
#if defined(MV_INCLUDE_PCI)
/*******************************************************************************
* mvCtrlPciMaxIfGet - Get Marvell controller number of PCI interfaces.
*
* DESCRIPTION:
*       This function returns Marvell controller number of PCI interfaces.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Marvell controller number of PCI interfaces. If controller 
*		ID is undefined the function returns '0'.
*
*******************************************************************************/
MV_U32 mvCtrlPciMaxIfGet(MV_VOID)
{

	return MV_PCI_MAX_IF;
}
#endif

#if defined(MV_INCLUDE_PCI) || defined(MV_INCLUDE_PEX)
/*******************************************************************************
* mvCtrlPciIfiMaxIfGet - Get Marvell controller number of PCI interfaces.
*
* DESCRIPTION:
*       This function returns Marvell controller number of PCI interfaces.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Marvell controller number of PCI interfaces. If controller 
*		ID is undefined the function returns '0'.
*
*******************************************************************************/
MV_U32 mvCtrlPciIfMaxIfGet(MV_VOID)
{

	return MV_PCI_IF_MAX_IF;
}
#endif

#if defined(MV_INCLUDE_PEX)
/*******************************************************************************
* mvCtrlPexMaxIfGet - Get Marvell controller number of PEX interfaces.
*
* DESCRIPTION:
*       This function returns Marvell controller number of PEX interfaces.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Marvell controller number of PEX interfaces. If controller 
*		ID is undefined the function returns '0'.
*
*******************************************************************************/
MV_U32 mvCtrlPexMaxIfGet(MV_VOID)
{

	return MV_PEX_MAX_IF;
}
#endif

#if defined(MV_INCLUDE_GIG_ETH) || defined(MV_INCLUDE_UNM_ETH)
/*******************************************************************************
* mvCtrlEthMaxPortGet - Get Marvell controller number of etherent ports.
*
* DESCRIPTION:
*       This function returns Marvell controller number of etherent port.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Marvell controller number of etherent port. If controller 
*		ID is undefined the function returns '0'.
*
*******************************************************************************/
MV_U32 mvCtrlEthMaxPortGet(MV_VOID)
{
	return MV_ETH_MAX_PORTS;
}
#endif

#if defined(MV_INCLUDE_IDMA)
/*******************************************************************************
* mvCtrlIdmaMaxChanGet - Get Marvell controller number of IDMA channels.
*
* DESCRIPTION:
*       This function returns Marvell controller number of IDMA channels.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Marvell controller number of IDMA channels. If controller 
*		ID is undefined the function returns '0'.
*
*******************************************************************************/
MV_U32 mvCtrlIdmaMaxChanGet(MV_VOID)
{
	return MV_IDMA_MAX_CHAN; 
}
#endif

#if defined(MV_INCLUDE_USB)
/*******************************************************************************
* mvCtrlUsbHostMaxGet - Get number of Marvell Usb  controllers
*
* DESCRIPTION:
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       returns number of Marvell USB  controllers.
*
*******************************************************************************/
MV_U32 mvCtrlUsbMaxGet(void)
{
    return MV_USB_MAX;

}
#endif


#if defined(MV_INCLUDE_NAND)

/*******************************************************************************
* mvCtrlNandSupport - Return if this controller has integrated NAND flash support
*
* DESCRIPTION:
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_TRUE if NAND is supported and MV_FALSE otherwise
*
*******************************************************************************/

MV_U32	  mvCtrlNandSupport(MV_VOID)
{
#if defined(MV_88F5181)
	if (mvCtrlModelGet() == MV_5281_DEV_ID)
	{
		return MV_NAND_MAX;
	}
	else
	{
		return 0;
	}
#else
		return MV_NAND_MAX;
#endif
}
#endif

/*******************************************************************************
* mvCtrlModelGet - Get Marvell controller device model (Id)
*
* DESCRIPTION:
*       This function returns 16bit describing the device model (ID) as defined
*       in PCI Device and Vendor ID configuration register offset 0x0.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       16bit desscribing Marvell controller ID 
*
*******************************************************************************/
MV_U16 mvCtrlModelGet(MV_VOID)
{
#if defined(MV_88F5180N)
	return MV_5180_DEV_ID;
#elif defined(MV_88F5082)
	return MV_5082_DEV_ID;
#else
#if defined(MV_INCLUDE_PEX)
	MV_U32 devId;

#if defined(MV_INCLUDE_CLK_PWR_CNTRL)
	/* Check pex power state */
	MV_U32 pexPower;
	pexPower = mvCtrlPwrClckGet(PEX_UNIT_ID,0);
	if (pexPower == MV_FALSE)
	{
		mvCtrlPwrClckSet(PEX_UNIT_ID, 0, MV_TRUE);
		mvPexPowerUp(0);
	}
#endif

	devId =  ((MV_REG_READ(PEX_CFG_DIRECT_ACCESS(0,PCI_DEVICE_AND_VENDOR_ID))
					 & PDVIR_DEV_ID_MASK) >> PDVIR_DEV_ID_OFFS);

#if defined(MV_INCLUDE_CLK_PWR_CNTRL)
	/* Return to power off state */
	if (pexPower == MV_FALSE)
	{
		mvPexPowerDown(0);
		mvCtrlPwrClckSet(PEX_UNIT_ID, 0, MV_FALSE);
	}
#endif

	return devId;

#elif defined(MV_INCLUDE_PCI)
	MV_U32 pciData = 0;
	MV_U32 dev = mvPciIfLocalDevNumGet(0);
	MV_U32 bus = mvPciIfLocalBusNumGet(0);

	pciData = mvPciIfConfigRead(0, bus, dev, 0, PCI_DEVICE_AND_VENDOR_ID);

	return ((pciData & PDVIR_DEV_ID_MASK) >> PDVIR_DEV_ID_OFFS);
#else
	#error "No Way to get Device ID"
#endif
#endif /* MV_88F5082 */
}
/*******************************************************************************
* mvCtrlRevGet - Get Marvell controller device revision number
*
* DESCRIPTION:
*       This function returns 8bit describing the device revision as defined
*       in PCI Express Class Code and Revision ID Register.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       8bit desscribing Marvell controller revision number 
*
*******************************************************************************/
MV_U8 mvCtrlRevGet(MV_VOID)
{
	MV_U8 revNum;
#if defined(MV_INCLUDE_PEX)
#if defined(MV_INCLUDE_CLK_PWR_CNTRL)
	/* Check pex power state */
	MV_U32 pexPower;
	pexPower = mvCtrlPwrClckGet(PEX_UNIT_ID,0);
	if (pexPower == MV_FALSE)
		mvCtrlPwrClckSet(PEX_UNIT_ID, 0, MV_TRUE);
#endif
	revNum = (MV_U8)MV_REG_READ(PEX_CFG_DIRECT_ACCESS(0,PCI_CLASS_CODE_AND_REVISION_ID));
#if defined(MV_INCLUDE_CLK_PWR_CNTRL)
	/* Return to power off state */
	if (pexPower == MV_FALSE)
		mvCtrlPwrClckSet(PEX_UNIT_ID, 0, MV_FALSE);
#endif
#elif defined(MV_INCLUDE_PCI)
	MV_U32 dev = mvPciIfLocalDevNumGet(0);
	MV_U32 bus = mvPciIfLocalBusNumGet(0);

	revNum = mvPciIfConfigRead(0, bus, dev, 0, PCI_CLASS_CODE_AND_REVISION_ID);
#endif
#if defined(MV_INCLUDE_PEX) || defined(MV_INCLUDE_PCI)
	return ((revNum & PCCRIR_REVID_MASK) >> PCCRIR_REVID_OFFS);
#else
	#error "No Way to get Revision ID"
#endif
}

/*******************************************************************************
* mvCtrlNameGet - Get Marvell controller name
*
* DESCRIPTION:
*       This function returns a string describing the device model and revision.
*
* INPUT:
*       None.
*
* OUTPUT:
*       pNameBuff - Buffer to contain device name string. Minimum size 30 chars.
*
* RETURN:
*       
*       MV_ERROR if informantion can not be read.
*******************************************************************************/
MV_STATUS mvCtrlNameGet(char *pNameBuff)
{
	mvOsSPrintf (pNameBuff, "%s%x Rev %d", SOC_NAME_PREFIX, 
				mvCtrlModelGet(), mvCtrlRevGet()); 
	
	return MV_OK;
}

/*******************************************************************************
* mvCtrlModelRevGet - Get Controller Model (Device ID) and Revision
*
* DESCRIPTION:
*       This function returns 32bit value describing both Device ID and Revision
*       as defined in PCI Express Device and Vendor ID Register and device revision
*	    as defined in PCI Express Class Code and Revision ID Register.
     
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit describing both controller device ID and revision number
*
*******************************************************************************/
MV_U32	mvCtrlModelRevGet(MV_VOID)
{
	return ((mvCtrlModelGet() << 16) | mvCtrlRevGet());
}

/*******************************************************************************
* mvCtrlNameGet - Get Marvell controller name
*
* DESCRIPTION:
*       This function returns a string describing the device model and revision.
*
* INPUT:
*       None.
*
* OUTPUT:
*       pNameBuff - Buffer to contain device name string. Minimum size 30 chars.
*
* RETURN:
*       
*       MV_ERROR if informantion can not be read.
*******************************************************************************/

MV_STATUS mvCtrlModelRevNameGet(char *pNameBuff)
{

	switch (mvCtrlModelRevGet())
	{
	case MV_5181_A0_ID:
		mvOsSPrintf (pNameBuff, "%s",MV_5181_A0_NAME); 
		break;
	case MV_5181_A1_ID:
		mvOsSPrintf (pNameBuff, "%s",MV_5181_A1_NAME); 
		break;
	case MV_5181_B0_ID:
		mvOsSPrintf (pNameBuff, "%s",MV_5181_B0_NAME); 
		break;
	case MV_5181_B1_ID:
		mvOsSPrintf (pNameBuff, "%s",MV_5181_B1_NAME);
		break;
	case MV_5281_A0_ID:
		mvOsSPrintf (pNameBuff, "%s",MV_5281_A0_NAME);
		break;
	case MV_5281_B0_ID:
		mvOsSPrintf (pNameBuff, "%s",MV_5281_B0_NAME);
		break;
	case MV_5281_C0_ID:
		mvOsSPrintf (pNameBuff, "%s",MV_5281_C0_NAME);
		break;
	case MV_5281_C1_ID:
		mvOsSPrintf (pNameBuff, "%s",MV_5281_C1_NAME);
		break;
	case MV_5281_D0_ID:
		mvOsSPrintf (pNameBuff, "%s",MV_5281_D0_NAME);
		break;
	case MV_5281_D1_ID:
		mvOsSPrintf (pNameBuff, "%s",MV_5281_D1_NAME);
		break;
	case MV_5281_D2_ID:
		mvOsSPrintf (pNameBuff, "%s",MV_5281_D2_NAME);
		break;
	case MV_5082_A2_ID:
		mvOsSPrintf (pNameBuff, "%s",MV_5082_A2_NAME);
		break;
        case MV_5182_A0_ID:
                mvOsSPrintf (pNameBuff, "%s",MV_5182_A0_NAME);
                break;
        case MV_5182_A1_ID:
                mvOsSPrintf (pNameBuff, "%s",MV_5182_A1_NAME);
                break;
        case MV_5182_A2_ID:
                mvOsSPrintf (pNameBuff, "%s",MV_5182_A2_NAME);
                break;
	case MV_5181L_A0_ID:
		mvOsSPrintf (pNameBuff, "%s",MV_5181L_A0_NAME);
		break;
	case MV_5181L_A1_ID:
		mvOsSPrintf (pNameBuff, "%s",MV_5181L_A1_NAME);
		break;
	case MV_8660_A0_ID:
		mvOsSPrintf (pNameBuff, "%s",MV_8660_A0_NAME);
		break;
	case MV_8660_A1_ID:
		mvOsSPrintf (pNameBuff, "%s",MV_8660_A1_NAME);
		break;
	case MV_1281_A0_ID:
		mvOsSPrintf (pNameBuff, "%s",MV_1281_A0_NAME);
		break;
	case MV_6183_A0_ID:
		mvOsSPrintf (pNameBuff, "%s",MV_6183_A0_NAME);
		break;
	case MV_6183_1_ID:
		mvOsSPrintf (pNameBuff, "%s",MV_6183_1_NAME);
		break;
	case MV_6183_A1_ID:
		mvOsSPrintf (pNameBuff, "%s",MV_6183_A1_NAME);
		break;
	case MV_6183_B0_ID:
		mvOsSPrintf (pNameBuff, "%s",MV_6183_B0_NAME);
		break;
	case MV_6082_A0_ID:
		if( (MV_REG_READ(0x10018) & BIT3) == 0)
			mvOsSPrintf (pNameBuff, "%s",MV_6082_A0_NAME);
		else
			mvOsSPrintf (pNameBuff, "%s",MV_6082L_A0_NAME);
		break;
	case MV_6082_A1_ID:
		mvOsSPrintf (pNameBuff, "%s",MV_6082_A1_NAME);
		break;
	default:
		mvCtrlNameGet(pNameBuff);
		break;
	}


	return MV_OK;
}

/*******************************************************************************
* ctrlWinOverlapTest - Test address windows for overlaping.
*
* DESCRIPTION:
*       This function checks the given two address windows for overlaping.
*
* INPUT:
*       pAddrWin1 - Address window 1.
*       pAddrWin2 - Address window 2.
*
* OUTPUT:
*       None.
*
* RETURN:
*       
*       MV_TRUE if address window overlaps, MV_FALSE otherwise.
*******************************************************************************/
MV_STATUS ctrlWinOverlapTest(MV_ADDR_WIN *pAddrWin1, MV_ADDR_WIN *pAddrWin2)
{
    MV_U32 winBase1, winBase2;
    MV_U32 winTop1, winTop2;
    
	/* check if we have overflow than 4G*/
	if (((0xffffffff - pAddrWin1->baseLow) < pAddrWin1->size-1)||
	   ((0xffffffff - pAddrWin2->baseLow) < pAddrWin2->size-1))
	{
		return MV_TRUE;
	}

    winBase1 = pAddrWin1->baseLow;
    winBase2 = pAddrWin2->baseLow;
    winTop1  = winBase1 + pAddrWin1->size-1;
    winTop2  = winBase2 + pAddrWin2->size-1;

    
    if (((winBase1 <= winTop2 ) && ( winTop2 <= winTop1)) ||
        ((winBase1 <= winBase2) && (winBase2 <= winTop1)))
    {
        return MV_TRUE;
    }
    else
    {
        return MV_FALSE;
    }
}

/*******************************************************************************
* ctrlWinWithinWinTest - Test address windows for overlaping.
*
* DESCRIPTION:
*       This function checks the given win1 boundries is within
*		win2 boundries.
*
* INPUT:
*       pAddrWin1 - Address window 1.
*       pAddrWin2 - Address window 2.
*
* OUTPUT:
*       None.
*
* RETURN:
*       
*       MV_TRUE if found win1 inside win2, MV_FALSE otherwise.
*******************************************************************************/
MV_STATUS ctrlWinWithinWinTest(MV_ADDR_WIN *pAddrWin1, MV_ADDR_WIN *pAddrWin2)
{
    MV_U32 winBase1, winBase2;
    MV_U32 winTop1, winTop2;
    
    winBase1 = pAddrWin1->baseLow;
    winBase2 = pAddrWin2->baseLow;
    winTop1  = winBase1 + pAddrWin1->size -1;
    winTop2  = winBase2 + pAddrWin2->size -1;
    
    if (((winBase1 >= winBase2 ) && ( winBase1 <= winTop2)) ||
        ((winTop1  >= winBase2) && (winTop1 <= winTop2)))
    {
        return MV_TRUE;
    }
    else
    {
        return MV_FALSE;
    }
}

static const char* cntrlName[] = TARGETS_NAME_ARRAY;

/*******************************************************************************
* mvCtrlTargetNameGet - Get Marvell controller target name
*
* DESCRIPTION:
*       This function convert the trget enumeration to string.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       Target name (const MV_8 *)
*******************************************************************************/
const MV_8* mvCtrlTargetNameGet( MV_TARGET target )
{

	if (target >= MAX_TARGETS)
	{
		return "target unknown";
	}

	return cntrlName[target];
}

/*******************************************************************************
* mvCtrlAddrDecShow - Print the Controller units address decode map.
*
* DESCRIPTION:
*		This function the Controller units address decode map.
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
*******************************************************************************/
MV_VOID mvCtrlAddrDecShow(MV_VOID)
{
    mvCpuIfAddDecShow();
    mvAhbToMbusAddDecShow();
#if defined(MV_INCLUDE_PEX)
	mvPexAddrDecShow();
#endif
#if defined(MV_INCLUDE_PCI)
	mvPciAddrDecShow();
#endif
#if defined(MV_INCLUDE_IDMA)
	mvDmaAddrDecShow();
#endif
#if defined(MV_INCLUDE_USB)
    	mvUsbAddrDecShow();
#endif
#if defined(MV_INCLUDE_GIG_ETH)
	mvEthAddrDecShow();
#endif
#if defined(MV_INCLUDE_XOR)
	mvXorAddrDecShow();
#endif
#if defined(MV_INCLUDE_SATA)
    mvSataAddrDecShow();
#endif
}

/*******************************************************************************
* ctrlSizeToReg - Extract size value for register assignment.
*
* DESCRIPTION:		
*       Address decode size parameter must be programed from LSB to MSB as
*       sequence of 1's followed by sequence of 0's. The number of 1's 
*       specifies the size of the window in 64 KB granularity (e.g. a 
*       value of 0x00ff specifies 256x64k = 16 MB).
*       This function extract the size value from the size parameter according 
*		to given aligment paramter. For example for size 0x1000000 (16MB) and 
*		aligment 0x10000 (64KB) the function will return 0x00FF.
*
* INPUT:
*       size - Size.
*		alignment - Size alignment.	Note that alignment must be power of 2!
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit describing size register value correspond to size parameter. 
*		If value is '-1' size parameter or aligment are invalid.
*******************************************************************************/
MV_U32	ctrlSizeToReg(MV_U32 size, MV_U32 alignment)
{
	MV_U32 retVal;

	/* Check size parameter alignment		*/
	if ((0 == size) || (MV_IS_NOT_ALIGN(size, alignment)))
	{
		DB(mvOsPrintf("ctrlSizeToReg: ERR. Size is zero or not aligned.\n"));
		return -1;
	}
	
	/* Take out the "alignment" portion out of the size parameter */
	alignment--;	/* Now the alignmet is a sequance of '1' (e.g. 0xffff) 	*/
					/* and size is 0x1000000 (16MB) for example				*/
	while(alignment & 1)	/* Check that alignmet LSB is set	*/
	{
		size = (size >> 1); /* If LSB is set, move 'size' one bit to right	*/	
		alignment = (alignment >> 1);
	}
	
	/* If after the alignment first '0' was met we still have '1' in 		*/
	/* it then aligment is invalid (not power of 2) 						*/
	if (alignment)
	{
		DB(mvOsPrintf("ctrlSizeToReg: ERR. Alignment parameter 0x%x invalid.\n", 
															(MV_U32)alignment));
		return -1;
	}

	/* Now the size is shifted right according to aligment: 0x0100			*/
	size--;         /* Now the size is a sequance of '1': 0x00ff 			*/
    
	retVal = size ;
	
	/* Check that LSB to MSB is sequence of 1's followed by sequence of 0's	*/
	while(size & 1)	/* Check that LSB is set	*/
	{
		size = (size >> 1); /* If LSB is set, move one bit to the right		*/	
	}

    if (size) /* Sequance of 1's is over. Check that we have no other 1's	*/
	{
		DB(mvOsPrintf("ctrlSizeToReg: ERR. Size parameter 0x%x invalid.\n", 
                                                                        size));
		return -1;
	}
	
    return retVal;
	
}

/*******************************************************************************
* ctrlRegToSize - Extract size value from register value.
*
* DESCRIPTION:		
*       This function extract a size value from the register size parameter 
*		according to given aligment paramter. For example for register size 
*		value 0xff and aligment 0x10000 the function will return 0x01000000.
*
* INPUT:
*       regSize   - Size as in register format.	See ctrlSizeToReg.
*		alignment - Size alignment.	Note that alignment must be power of 2!
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit describing size. 
*		If value is '-1' size parameter or aligment are invalid.
*******************************************************************************/
MV_U32	ctrlRegToSize(MV_U32 regSize, MV_U32 alignment)
{
   	MV_U32 temp;

	/* Check that LSB to MSB is sequence of 1's followed by sequence of 0's	*/ 
	temp = regSize;		/* Now the size is a sequance of '1': 0x00ff		*/
	
	while(temp & 1)	/* Check that LSB is set	*/
	{
		temp = (temp >> 1); /* If LSB is set, move one bit to the right		*/	
	}

    if (temp) /* Sequance of 1's is over. Check that we have no other 1's	*/
	{
		DB(mvOsPrintf("ctrlRegToSize: ERR. Size parameter 0x%x invalid.\n", 
																	regSize));
	   	return -1;
	}
	

	/* Check that aligment is a power of two								*/
	temp = alignment - 1;/* Now the alignmet is a sequance of '1' (0xffff) 	*/
					
	while(temp & 1)	/* Check that alignmet LSB is set	*/
	{
		temp = (temp >> 1); /* If LSB is set, move 'size' one bit to right	*/	
	}
	
	/* If after the 'temp' first '0' was met we still have '1' in 'temp'	*/
	/* then 'temp' is invalid (not power of 2) 								*/
	if (temp)
	{
		DB(mvOsPrintf("ctrlSizeToReg: ERR. Alignment parameter 0x%x invalid.\n", 
																	alignment));
		return -1;
	}

	regSize++;      /* Now the size is 0x0100								*/

	/* Add in the "alignment" portion to the register size parameter 		*/
	alignment--;	/* Now the alignmet is a sequance of '1' (e.g. 0xffff) 	*/

	while(alignment & 1)	/* Check that alignmet LSB is set	*/
	{
		regSize   = (regSize << 1); /* LSB is set, move 'size' one bit left	*/	
		alignment = (alignment >> 1);
	}
		
    return regSize;	
}


/*******************************************************************************
* ctrlSizeRegRoundUp - Round up given size 
*
* DESCRIPTION:		
*       This function round up a given size to a size that fits the 
*       restrictions of size format given an aligment parameter.
*		to given aligment paramter. For example for size parameter 0xa1000 and 
*		aligment 0x1000 the function will return 0xFF000.
*
* INPUT:
*       size - Size.
*		alignment - Size alignment.	Note that alignment must be power of 2!
*
* OUTPUT:
*       None.
*
* RETURN:
*       32bit describing size value correspond to size in register.  
*******************************************************************************/
MV_U32	ctrlSizeRegRoundUp(MV_U32 size, MV_U32 alignment)
{
	MV_U32 msbBit = 0;
    MV_U32 retSize;
	
    /* Check if size parameter is already comply with restriction		*/
	if (!(-1 == ctrlSizeToReg(size, alignment)))
	{
		return size;
	}
    
    while(size)
	{
		size = (size >> 1);
        msbBit++;
	}

    retSize = (1 << msbBit);
    
    if (retSize < alignment)
    {
        return alignment;
    }
    else
    {
        return retSize;
    }
}

#if defined(MV_INCLUDE_CLK_PWR_CNTRL)
/*******************************************************************************
* mvCtrlPwrClckSet - Set Power State for specific Unit
*
* DESCRIPTION:		
*
* INPUT:
*
* OUTPUT:
*
* RETURN:
*******************************************************************************/
MV_VOID   mvCtrlPwrClckSet(MV_UNIT_ID unitId, MV_U32 index, MV_BOOL enable)
{
	switch (unitId)
    {
#if defined(MV_INCLUDE_PEX)
	case PEX_UNIT_ID:
		if (enable == MV_FALSE)
		{
			MV_REG_BIT_SET(POWER_MNG_CTRL_REG, PMC_PEXSTOPCLOCK_MASK);
		}
		else
		{
			MV_REG_BIT_RESET(POWER_MNG_CTRL_REG, PMC_PEXSTOPCLOCK_MASK);
		}
		break;
#endif
#if defined(MV_INCLUDE_GIG_ETH)
	case ETH_GIG_UNIT_ID:
		if (enable == MV_FALSE)
		{
			MV_REG_BIT_SET(POWER_MNG_CTRL_REG, PMC_GESTOPCLOCK_MASK(index));
		}
		else
		{
			MV_REG_BIT_RESET(POWER_MNG_CTRL_REG, PMC_GESTOPCLOCK_MASK(index));
		}
		break;
#endif
#if defined(MV_INCLUDE_INTEG_SATA)
	case SATA_UNIT_ID:
		if (enable == MV_FALSE)
		{
			MV_REG_BIT_SET(POWER_MNG_CTRL_REG, PMC_SATASTOPCLOCK_MASK);
		}
		else
		{
			MV_REG_BIT_RESET(POWER_MNG_CTRL_REG, PMC_SATASTOPCLOCK_MASK);
		}
		break;
#endif
#if defined(MV_INCLUDE_CESA)
	case CESA_UNIT_ID:
		if (enable == MV_FALSE)
		{
			MV_REG_BIT_SET(POWER_MNG_CTRL_REG, PMC_SESTOPCLOCK_MASK);
		}
		else
		{
			MV_REG_BIT_RESET(POWER_MNG_CTRL_REG, PMC_SESTOPCLOCK_MASK);
		}
		break;
#endif
#if defined(MV_INCLUDE_USB)
	case USB_UNIT_ID:
		if (enable == MV_FALSE)
		{
			MV_REG_BIT_SET(POWER_MNG_CTRL_REG, PMC_USBSTOPCLOCK_MASK);
		}
		else
		{
			MV_REG_BIT_RESET(POWER_MNG_CTRL_REG, PMC_USBSTOPCLOCK_MASK);
		}
		break;
#endif
	default:

		break;

	}
}

/*******************************************************************************
* mvCtrlPwrClckGet - Get Power State of specific Unit
*
* DESCRIPTION:		
*
* INPUT:
*
* OUTPUT:
*
* RETURN:
******************************************************************************/
MV_BOOL		mvCtrlPwrClckGet(MV_UNIT_ID unitId, MV_U32 index)
{
	MV_U32 reg = MV_REG_READ(POWER_MNG_CTRL_REG);
	MV_BOOL state = MV_TRUE;

	switch (unitId)
    {
#if defined(MV_INCLUDE_PEX)
	case PEX_UNIT_ID:
		if (reg & PMC_PEXSTOPCLOCK_STOP)
		{
			state = MV_FALSE;
		}
		else state = MV_TRUE;

		break;
#endif
#if defined(MV_INCLUDE_GIG_ETH)
	case ETH_GIG_UNIT_ID:
		if (reg & PMC_GESTOPCLOCK_STOP(index))
		{
			state = MV_FALSE;
		}
		else state = MV_TRUE;
		break;
#endif
#if defined(MV_INCLUDE_SATA)
	case SATA_UNIT_ID:
		if (reg & PMC_SATASTOPCLOCK_STOP)
		{
			state = MV_FALSE;
		}
		else state = MV_TRUE;
		break;
#endif
#if defined(MV_INCLUDE_CESA)
	case CESA_UNIT_ID:
		if (reg & PMC_SESTOPCLOCK_STOP)
		{
			state = MV_FALSE;
		}
		else state = MV_TRUE;
		break;
#endif
#if defined(MV_INCLUDE_USB)
	case USB_UNIT_ID:
		if (reg & PMC_USBSTOPCLOCK_STOP)
		{
			state = MV_FALSE;
		}
		else state = MV_TRUE;
		break;
#endif
	default:
		state = MV_TRUE;
		break;
	}


	return state;	
}
#else
MV_VOID   mvCtrlPwrClckSet(MV_UNIT_ID unitId, MV_U32 index, MV_BOOL enable) {return;}
MV_BOOL	  mvCtrlPwrClckGet(MV_UNIT_ID unitId, MV_U32 index) {return MV_TRUE;}
#endif /* #if defined(MV_INCLUDE_CLK_PWR_CNTRL) */

#if defined(MV_INCLUDE_INTEG_MFLASH) || defined (MV_INCLUDE_SPI)
/*******************************************************************************
* mvCtrlSpiBusModeSet - set the connectivity of the SPI bus
*
* DESCRIPTION:
*       configure the MFlash controller to one of the 4 options
*
* INPUT:
*       spiMode: SPI bus mode
*
* OUTPUT:
*       None.
*
* RETURN:
*       Success or Error code.
*       
*
*******************************************************************************/
MV_STATUS mvCtrlSpiBusModeSet(MV_SPI_CONN_MODE spiMode)
{
#if defined (MV_INCLUDE_INTEG_MFLASH) && defined (MV_INCLUDE_SPI)

    /* Read and clear the 3 bits related to the SPI mode */
    MV_U32 reg = (MV_REG_READ(MV_PMFLASH_IF_CFG_REG) & ~MV_PMFLASH_SPI_BUS_MODE_MASK);

    switch (spiMode)
	{
		case MV_SPI_CONN_TO_MFLASH:
            reg |= MV_PMFLASH_SPI_BUS_TO_MFLASH;
			break;

		case MV_SPI_CONN_TO_EXT_FLASH:
            reg |= MV_PMFLASH_SPI_BUS_TO_EXT_SFLASH;
			break;

		case MV_SPI_CONN_MFLASH_TO_EXT_PROG:
            reg |= MV_PMFLASH_SPI_BUS_EXT_PROGRAMER;
			break;

		default:
			return MV_BAD_PARAM;
	}

    /* write back the register with the updated 3 bits */
    MV_REG_WRITE(MV_PMFLASH_IF_CFG_REG, reg);
#endif
	return MV_OK;
}

/*******************************************************************************
* mvCtrlSpiBusModeDetect - Detect the configuration of the SPI interface in the
*                       hardware.
*
* DESCRIPTION:
*       Detect the SPI connectivity. Whether the SPI is used to access the 
*       internal MFlash or to access an external device.
*
* INPUT:
*       None
*
* OUTPUT:
*       None
*
* RETURN:
*       SPI mode
*       
*
*******************************************************************************/
MV_SPI_CONN_MODE mvCtrlSpiBusModeDetect(void)
{
#if defined (MV_INCLUDE_INTEG_MFLASH) && defined (MV_INCLUDE_SPI)
    /* Read the regiater and mask away all bits except the 3 relevant bits */
    MV_U32 reg = (MV_REG_READ(MV_PMFLASH_IF_CFG_REG) & MV_PMFLASH_SPI_BUS_MODE_MASK);

    /* check all possibilities */
    switch (reg)
    {
        case MV_PMFLASH_SPI_BUS_TO_MFLASH:
            return MV_SPI_CONN_TO_MFLASH;

        case MV_PMFLASH_SPI_BUS_TO_EXT_SFLASH:
            return MV_SPI_CONN_TO_EXT_FLASH;

        case MV_PMFLASH_SPI_BUS_EXT_PROGRAMER:
            return MV_SPI_CONN_MFLASH_TO_EXT_PROG;
    }

    return MV_SPI_CONN_UNKNOWN;
#elif defined (MV_INCLUDE_SPI)
    return MV_SPI_CONN_TO_EXT_FLASH;
#else
    return MV_SPI_CONN_UNKNOWN;
#endif
}
#endif /* #if defined (MV_INCLUDE_INTEG_MFLASH) */

