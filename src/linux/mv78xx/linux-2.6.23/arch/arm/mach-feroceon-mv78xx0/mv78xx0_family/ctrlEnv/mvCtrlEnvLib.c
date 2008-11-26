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
#include "ctrlEnv/mvCtrlEnvLib.h"
#ifdef MV_INCLUDE_PEX
#include "pex/mvPex.h"
#include "ctrlEnv/sys/mvSysPex.h"
#endif
#include "cpu/mvCpu.h"

#include "ctrlEnv/sys/mvCpuIf.h"
#include "eth/mvEth.h"
#include "ctrlEnv/sys/mvSysGbe.h"
#ifdef MV_INCLUDE_IDMA
#include "idma/mvIdma.h"
#include "ctrlEnv/sys/mvSysIdma.h"
#endif
#ifdef MV_INCLUDE_XOR
#include "xor/mvXor.h"
#include "ctrlEnv/sys/mvSysXor.h"
#endif
#ifdef MV_INCLUDE_USB
#include "usb/mvUsb.h"
#include "ctrlEnv/sys/mvSysUsb.h"
#endif
#ifdef MV_INCLUDE_SATA
#include "ctrlEnv/sys/mvSysSata.h"
#endif

#include "device/mvDevice.h"

#define PCI_CLASS_CODE_AND_REVISION_ID			    0x008

#define PCCRIR_REVID_OFFS		0		/* Revision ID */
#define PCCRIR_REVID_MASK		(0xff << PCCRIR_REVID_OFFS)



/* defines  */
#ifdef MV_DEBUG
	#define DB(x)	x
#else
	#define DB(x)
#endif	


/* Global paramters initial value '-1' to indicate they are uninitialized.	*/
/* In case of data section is located in ROM, this value will not be able 	*/
/* to change.																*/
MV_32 ctrlDevModel = -1;
MV_32 ctrlDevRev   = -1;

/*******************************************************************************
* mvCtrlEnvInit - Initialize Marvell controller environment.
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
		MV_REG_WRITE(MPP_CONTROL_REG(mppGroup), mvBoardMppGet(mppGroup));
	}
	/*Only CPU0 bridge influences the system*/
	MV_REG_BIT_SET(CPU_CTRL_STAT_REG(0), CCSR_PEX0_ENABLE);
	MV_REG_BIT_SET(CPU_CTRL_STAT_REG(0), CCSR_PEX1_ENABLE);	
	return MV_OK;
}

/*******************************************************************************
* mvCtrlSramSizeGet - Get Marvell controller integrated SRAM size
*
* DESCRIPTION:
*       This function returns Marvell controller integrated SRAM size
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       function returns '0'.
*
*******************************************************************************/
MV_U32	  mvCtrlSramSizeGet(MV_VOID)
{
		return 0;
}

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
*       Marvell controller number of PCI interfaces.
*
*******************************************************************************/
MV_U32 mvCtrlPciMaxIfGet(MV_VOID)
{
	return  0;
}

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
*       Marvell controller number of PEX interfaces.
*
*******************************************************************************/
MV_U32 mvCtrlPexMaxIfGet(MV_VOID)
{
	return MV_PEX_MAX_IF;
}

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
	return mvCtrlPexMaxIfGet();
}
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
	if (MV_78200_DEV_ID == mvCtrlModelGet())
		return MV78200_ETH_MAX_PORTS;

	return MV78XX0_ETH_MAX_PORTS;
}

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
	return MV_USB_MAX_CHAN;
}


#if defined(MV_INCLUDE_PEX)

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

	MV_32 tmpCtrlDevModel;

	/* Check if global variable has already been intialized */
	if (-1 == ctrlDevModel)
	{
		/* Extract device number 		*/
		tmpCtrlDevModel = MV_REG_READ(PEX_CFG_DIRECT_ACCESS(0,PEX_DEVICE_AND_VENDOR_ID));
		tmpCtrlDevModel &= PXDAVI_DEV_ID_MASK;
		tmpCtrlDevModel = (tmpCtrlDevModel >> PXDAVI_DEV_ID_OFFS);
#ifndef MV_UBOOT
		ctrlDevModel = tmpCtrlDevModel;
#endif
	}
	else
	{
		tmpCtrlDevModel = ctrlDevModel;
	}

	if (MV_78XX0_ZY_DEV_ID == tmpCtrlDevModel)
	{
		tmpCtrlDevModel = MV_78XX0_DEV_ID;
	}

	return (MV_U16)tmpCtrlDevModel;
}

/*******************************************************************************
* mvCtrlRevGet - Get Marvell controller revision
*
* DESCRIPTION:
*       This function returns 16bit describing the device revision as defined
*       in PCI Class Code and Revision ID configuration register offset 0x8.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       16bit desscribing Marvell controller revision 
*
*******************************************************************************/
MV_U8 mvCtrlRevGet(MV_VOID)
{

	MV_32 tmpCtrlDevRev;

    /* Check if global variable has already been intialized */
	if (-1 == ctrlDevRev)
	{
		tmpCtrlDevRev = MV_REG_READ(PEX_CFG_DIRECT_ACCESS(0,PEX_CLASS_CODE_AND_REVISION_ID)); 
		tmpCtrlDevRev &= PXCCARI_REVID_MASK;
		tmpCtrlDevRev = (tmpCtrlDevRev >> PXCCARI_REVID_OFFS);
#ifndef MV_UBOOT
		ctrlDevRev = tmpCtrlDevRev;
#endif
	}
	else
	{
		tmpCtrlDevRev = ctrlDevRev;
	}

	return (MV_U8)tmpCtrlDevRev;
}


#else
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
	MV_U32	localBusNum; 
	MV_U32	localDevNum; 
	MV_U32  pciData;
	MV_32 tmpCtrlDevModel;
	MV_BOOL ifPwrOn;
	/* Check if global variable has already been intialized */
	/* In U-boot we are still in the flash so we skeep it */
#ifndef MV_UBOOT
	if (-1 == ctrlDevModel)
	{
#endif
		ifPwrOn = mvCtrlPwrClckGet(PEX_UNIT_ID, PEX_DEFAULT_IF);
		if (MV_FALSE == ifPwrOn)
		{
			mvCtrlPwrClckSet(PEX_UNIT_ID, PEX_DEFAULT_IF, MV_TRUE);
		}
		/* Get PCI local bus number 	*/
		localBusNum = mvPciLocalBusNumGet(PEX_DEFAULT_IF);
		/* Get PCI local dev number 	*/
		localDevNum = mvPciLocalDevNumGet(PEX_DEFAULT_IF);
		
		/* Read configuration register 0	*/
		pciData=mvPciConfigRead(PEX_DEFAULT_IF, localBusNum, localDevNum, 0, 0);
	
		/* Extract device number 		*/
		tmpCtrlDevModel = ((pciData & PDVIR_DEV_ID_MASK) >> PDVIR_DEV_ID_OFFS);
		if (MV_FALSE == ifPwrOn)
		{
			mvCtrlPwrClckSet(PEX_UNIT_ID, PEX_DEFAULT_IF, MV_FALSE);
		}
#ifndef MV_UBOOT
		ctrlDevModel = tmpCtrlDevModel;
	}
	else
	{
		tmpCtrlDevModel = ctrlDevModel;
	}
#endif
	if (MV_78XX0_ZY_DEV_ID == tmpCtrlDevModel)
	{
		tmpCtrlDevModel = MV_78XX0_DEV_ID;
	}

	return (MV_U16)tmpCtrlDevModel;
}

/*******************************************************************************
* mvCtrlRevGet - Get Marvell controller revision
*
* DESCRIPTION:
*       This function returns 16bit describing the device revision as defined
*       in PCI Class Code and Revision ID configuration register offset 0x8.
*
* INPUT:
*       None.
*
* OUTPUT:
*       None.
*
* RETURN:
*       16bit desscribing Marvell controller revision 
*
*******************************************************************************/
MV_U8 mvCtrlRevGet(MV_VOID)
{
	MV_U32	localBusNum; 
	MV_U32	localDevNum; 
	MV_U32  pciData;
	MV_32 tmpCtrlDevRev;

    /* Check if global variable has already been intialized */
	if (-1 == ctrlDevRev)
	{
		//MV_BOOL ifPwrOn = mvCtrlPwrClckGet(PEX_UNIT_ID, pciIf);
		//if (MV_FALSE == ifPwrOn)
		{
		//	mvCtrlPwrClckSet(PEX_UNIT_ID, PEX_DEFAULT_IF, MV_TRUE);
		}
		/* Get PCI local bus number 	*/
		localBusNum = mvPciLocalBusNumGet(PEX_DEFAULT_IF);
		/* Get PCI local dev number 	*/
		localDevNum = mvPciLocalDevNumGet(PEX_DEFAULT_IF);
		
		/* Read configuration register 8 */
		pciData=mvPciConfigRead(PEX_DEFAULT_IF, localBusNum, localDevNum, 0, 8);
	
		/* Extract revision number 		*/
		tmpCtrlDevRev = ((pciData & PCCRIR_REVID_MASK) >> PCCRIR_REVID_OFFS);

		ctrlDevRev = tmpCtrlDevRev;
	}
		//if (MV_FALSE == ifPwrOn)
		//{
		//	mvCtrlPwrClckSet(PEX_UNIT_ID, pciIf, MV_FALSE);
		//}
	}
	else
	{
		tmpCtrlDevRev = ctrlDevRev;
	}

	return (MV_U8)tmpCtrlDevRev;
}

#endif

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
#ifndef MV_UBOOT
	if (-1 == ctrlDevModel)
	    ctrlDevModel = mvCtrlModelGet();
	return ((ctrlDevModel << 16) | mvCtrlRevGet());
#else
	return ((mvCtrlModelGet() << 16) | mvCtrlRevGet());
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
#ifndef MV_UBOOT
	if (-1 == ctrlDevModel)
	    ctrlDevModel = mvCtrlModelGet();
	switch(ctrlDevModel) {
#else
	switch(mvCtrlModelGet()) {
#endif 
	case MV_78XX0_DEV_ID:
		sprintf (pNameBuff, "%s", MV_78XX0_NAME);
		break;
	case MV_78100_DEV_ID:
		sprintf (pNameBuff, "%s", MV_78100_NAME);
		break;
	case MV_78200_DEV_ID:
		sprintf (pNameBuff, "%s", MV_78200_NAME);
		break;
	default:
		sprintf (pNameBuff, "Controller device ID 0x%x unknown.\n", mvCtrlModelGet());
		return MV_ERROR;
	}
	
	return MV_OK;
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
	case MV_78XX0_Z0_ID:
		mvOsSPrintf (pNameBuff, "%s", MV_78XX0_Z0_NAME); 
		break;
	case MV_78XX0_Y0_ID:
		mvOsSPrintf (pNameBuff, "%s", MV_78XX0_Y0_NAME); 
		break;
	case MV_78100_A0_ID:
		mvOsSPrintf (pNameBuff, "%s", MV_78100_A0_NAME); 
		break;
	case MV_78200_A0_ID:
		mvOsSPrintf (pNameBuff, "%s", MV_78200_A0_NAME); 
		break;
	default:
		mvCtrlNameGet(pNameBuff);
		break;
	}


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
    return MPP_CONTROL_REG(mppGroup);
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
    
    winBase1 = pAddrWin1->baseLow;
    winBase2 = pAddrWin2->baseLow;
    winTop1  = winBase1 + pAddrWin1->size;
    winTop2  = winBase2 + pAddrWin2->size;
    
    if (((winBase1 < winTop2 ) && ( winTop2 < winTop1)) ||
        ((winBase1 < winBase2) && (winBase2 < winTop1)))
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
	switch( target ) 
	{
		case SDRAM_CS0: 
			return "SDRAM_CS0....";
		case SDRAM_CS1: 
			return "SDRAM_CS1....";
		case SDRAM_CS2: 
			return "SDRAM_CS2....";
		case SDRAM_CS3: 
			return "SDRAM_CS3....";
		case DEVICE_CS0: 
			return "DEVICE_CS0...";
		case DEVICE_CS1: 
			return "DEVICE_CS1...";
		case DEVICE_CS2: 
			return "DEVICE_CS2...";
		case DEVICE_CS3: 
			return "DEVICE_CS3...";
#if !defined (MV78XX0_Z0)
		case SPI_CS: 
			return "DEVICE_SPI...";
#endif
		case DEV_BOOCS: 
			return "DEV_BOOCS....";
		case PCI0_IO: 
			return "PEX0_IO......";
		case PCI0_MEM0: 
			return "PEX0_MEM0....";
		case PCI1_IO: 
			return "PEX1_IO......";
		case PCI1_MEM0: 
			return "PEX1_MEM0....";
		case PCI2_IO: 
			return "PEX2_IO......";
		case PCI2_MEM0: 
			return "PEX2_MEM0....";
		case PCI3_IO: 
			return "PEX3_IO......";
		case PCI3_MEM0: 
			return "PEX3_MEM0....";
		case PCI4_IO: 
			return "PEX4_IO......";
		case PCI4_MEM0: 
			return "PEX4_MEM0....";
		case PCI5_IO: 
			return "PEX5_IO......";
		case PCI5_MEM0: 
			return "PEX5_MEM0....";
		case PCI6_IO: 
			return "PEX6_IO......";
		case PCI6_MEM0: 
			return "PEX6_MEM0....";
		case PCI7_IO: 
			return "PEX7_IO......";
		case PCI7_MEM0: 
			return "PEX7_MEM0....";
		case CRYPT_ENG: 
			return "CRYPT_ENG.....";
		case INTER_REGS: 
			return "INTER_REGS...";
		case USB_IF: 
			return "USB_IF.......";
		default:
			 return "target unknown";
	}
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
	mvCpuIfAddrDecShow(whoAmI());
#ifdef MV_INCLUDE_PEX
	mvPexAddrDecShow();
#endif
#ifdef MV_INCLUDE_IDMA
	mvDmaAddrDecShow();
#endif
	mvEthAddrDecShow();
#ifdef MV_INCLUDE_XOR
	mvXorAddrDecShow();
#endif
#ifdef MV_INCLUDE_SATA
	mvSataAddrDecShow();
#endif
#ifdef MV_INCLUDE_USB
	mvUsbAddrDecShow();
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
	if (0 == regSize)
	{
		return 0;
	}
	/* Check that LSB to MSB is sequence of 1's followed by sequence of 0's	*/ 
	temp = regSize;		/* Now the size is a sequence of '1': 0x00ff		*/
	
	while(temp & 1)	/* Check that LSB is set	*/
	{
		temp = (temp >> 1); /* If LSB is set, move one bit to the right		*/	
	}

    	if (temp) /* Sequence of 1's is over. Check that we have no other 1's	*/
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
*       This function round up a given size to a size that is a power of 2.  
*		For example, for size parameter 0xa1000 and aligment 0x1000 the 
*       function will return 0x100000. For size parametr 0x200 and aligment
*       0x10000 the function will return 0x10000.
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
		    MV_REG_BIT_RESET(POWER_MNG_CTRL_REG, PMC_PEX_MASK(index));
		}
		else
		{
							MV_REG_BIT_SET(POWER_MNG_CTRL_REG,PMC_PEX_MASK(index));
		}
		break;
#endif
#if defined(MV_INCLUDE_GIG_ETH)
	case ETH_GIG_UNIT_ID:
		if (enable == MV_FALSE)
		{
		    MV_REG_BIT_RESET(POWER_MNG_CTRL_REG, PMC_GE_MASK(index));
		}
		else
		{
							MV_REG_BIT_SET(POWER_MNG_CTRL_REG, PMC_GE_MASK(index));
		}
		break;
#endif
#if defined(MV_INCLUDE_INTEG_SATA)
	case SATA_UNIT_ID:
		if (enable == MV_FALSE)
		{
		    MV_REG_BIT_RESET(POWER_MNG_CTRL_REG, PMC_SATA_MASK(index));
		}
		else
		{
							MV_REG_BIT_SET(POWER_MNG_CTRL_REG, PMC_SATA_MASK(index));
		}
		break;
#endif
#if defined(MV_INCLUDE_CESA)
	case CESA_UNIT_ID:
		if (enable == MV_FALSE)
		{
		    MV_REG_BIT_RESET(POWER_MNG_CTRL_REG, PMC_SE_MASK);
		}
		else
		{
							MV_REG_BIT_SET(POWER_MNG_CTRL_REG, PMC_SE_MASK);
		}
		break;
#endif
#if defined(MV_INCLUDE_USB)
	case USB_UNIT_ID:
		if (enable == MV_FALSE)
		{
		    MV_REG_BIT_RESET(POWER_MNG_CTRL_REG, PMC_USB_MASK(index));
		}
		else
		{
							MV_REG_BIT_SET(POWER_MNG_CTRL_REG, PMC_USB_MASK(index));
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
		if (0 == (reg & PMC_PEX_UP(index)))
		{
			state = MV_FALSE;
		}
		else state = MV_TRUE;

		break;
#endif
#if defined(MV_INCLUDE_GIG_ETH)
	case ETH_GIG_UNIT_ID:
		if (0 == (reg & PMC_GE_UP(index)))
		{
			state = MV_FALSE;
		}
		else state = MV_TRUE;
		break;
#endif
#if defined(MV_INCLUDE_SATA)
	case SATA_UNIT_ID:
		if (0 == (reg & PMC_SATA_UP(index)))
		{
			state = MV_FALSE;
		}
		else state = MV_TRUE;
		break;
#endif
#if defined(MV_INCLUDE_CESA)
	case CESA_UNIT_ID:
		if (0 == (reg & PMC_SE_UP))
		{
			state = MV_FALSE;
		}
		else state = MV_TRUE;
		break;
#endif
#if defined(MV_INCLUDE_USB)
	case USB_UNIT_ID:
		if (0 == (reg & PMC_USB_UP(index)))
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
MV_BOOL		mvCtrlPwrClckGet(MV_UNIT_ID unitId, MV_U32 index) {return MV_TRUE;} 
#endif /* #if defined(MV_INCLUDE_CLK_PWR_CNTRL) */


