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
#include "mvCtrlEnvLib.h"
#include "mvPci.h"
#include "mvPex.h"
#include "mvCpuIf.h"
#if defined(MV_88F5181)
#include "mvIdma.h"
extern void mvEthAddrDecShow(void);
#if defined(MV_88F5182)
#include "mvXor.h"
#endif
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
		MV_REG_WRITE(mvMppRegGet(mppGroup), mvBoardMppGet(mppGroup));
	}

	return MV_OK;
}

/*******************************************************************************
* mvMppRegGet - return reg address of mpp group
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
MV_U32 mvMppRegGet(MV_U32 mppGroup)
{
        MV_U32 ret;

        switch(mppGroup){
                case (0):       ret = MPP_CONTROL_REG0;
                                break;
                case (1):       ret = MPP_CONTROL_REG1;
                                break;
                case (2):       ret = MPP_CONTROL_REG2;
                                break;
                case (3):       ret = DEV_MULTI_CONTROL;
                                break;
                default:        ret = MPP_CONTROL_REG0;
                                break;
        }
        return ret;
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
*       Marvell controller number of PCI interfaces. If controller 
*		ID is undefined the function returns '0'.
*
*******************************************************************************/
MV_U32 mvCtrlPciMaxIfGet(MV_VOID)
{

	return MV_PCI_MAX_IF;
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

	return MV_PCI_IF_MAX_IF;
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
*       Marvell controller number of PEX interfaces. If controller 
*		ID is undefined the function returns '0'.
*
*******************************************************************************/
MV_U32 mvCtrlPexMaxIfGet(MV_VOID)
{

	return MV_PEX_MAX_IF;
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
	return MV_ETH_MAX_PORTS;
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
#if defined(MV_88F1181)
    return MV_USB_1181_MAX;
#else
    if(mvCtrlModelGet() == MV_5182_DEV_ID)
		return MV_USB_5182_MAX;
        
    return MV_USB_5181_MAX;

#endif /* MV_88F1181 */    
}

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
#if defined(MV_88F1181)

	return MV_1181_DEV_ID;

#elif defined(MV_88F5181)

	return ((MV_REG_READ(0x40000) & PDVIR_DEV_ID_MASK) >> PDVIR_DEV_ID_OFFS);

#else
#   error "CHIP not selected"
#endif /* #if defined(MV_88F1181) */
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

	revNum = (MV_U8)MV_REG_READ(PEX_CFG_DIRECT_ACCESS(0, PEX_CLASS_CODE_AND_REVISION_ID));

	return revNum;
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
#if defined(MV_88F1181)

	mvOsSPrintf (pNameBuff, "%s Rev %d", MV_1181_DEV_NAME, mvCtrlRevGet()); 

#elif defined(MV_88F5181)

	mvOsSPrintf (pNameBuff, "MV88F%x Rev %d", mvCtrlModelGet(), mvCtrlRevGet()); 
#elif defined(MV_88W8660)

	mvOsSPrintf (pNameBuff, "MV88W%x Rev %d", mvCtrlModelGet(), mvCtrlRevGet());
#else
#   error "CHIP not selected"
#endif /* #if defined(MV_88F1181) */


	
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
	case SDRAM_CS0:      /* SDRAM chip select 0*/
			return "SDRAM_CS0";
	case SDRAM_CS1:      /* SDRAM chip select 1*/
			return "SDRAM_CS1";
	case SDRAM_CS2:      /* SDRAM chip select 2*/
			return "SDRAM_CS2";
	case SDRAM_CS3:      /* SDRAM chip select 3*/
			return "SDRAM_CS3";
	case PEX0_MEM:		/* PCI Express 0 Memory*/
			return "PEX0_MEM";
	case PEX0_IO:		/* PCI Express 0 IO*/
			return "PEX0_IO";
#if defined(MV_88F1181)
	case PEX1_MEM:		/* PCI Express 1 Memory*/
			return "PEX1_MEM";
	case PEX1_IO:		/* PCI Express 1 IO*/
			return "PEX1_IO";
#elif defined(MV_88F5181)
	case PCI0_MEM:		/* PCI Memory*/
			return "PCI0_MEM";
	case PCI0_IO:			/* PCI IO*/
			return "PCI0_IO";
#else                                                
#   error "CHIP not selected"                        
#endif                                               
	case INTER_REGS:     /* Internal registers*/
			return "INTER_REGS";
#if defined(MV_88F5181)
	case DEVICE_CS0:     /* Device chip select 0*/
			return "DEVICE_CS0";
	case DEVICE_CS1:     /* Device chip select 0*/
			return "DEVICE_CS1";
	case DEVICE_CS2:     /* Device chip select 0*/
			return "DEVICE_CS2";
#elif defined (MV_88F1181)                           
	case FLASH_CS:     	/* Flash chip select*/
			return "DEVICE_CS2";
#else                                                
#   error "CHIP not selected"                        
#endif                                               
	case DEV_BOOCS:    	/* Flash Boot chip select*/
			return "DEV_BOOCS";

#if defined(MV_88F5182) || defined (MV_88F5181L)
	case CRYPT_ENG:    	/* Crypto Engine*/
			return "CRYPTO ENG";
#endif

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
    mvCpuIfAddDecShow();
    mvAhbToMbusAddDecShow();
	mvPexAddrDecShow();
#if defined(MV_88F5181)
	mvPciAddrDecShow();
	mvDmaAddrDecShow();
	mvEthAddrDecShow();
    /*mvUsbAddrDecShow();*/
#endif  /* #if defined(MV_88F5181) */
#if defined(MV_88F5182)
	mvXorAddrDecShow();
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

    retSize = (1 << (msbBit+1));
    
    if (retSize < alignment)
    {
        return alignment;
    }
    else
    {
        return retSize;
    }
}

