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
#include "ctrlEnv/sys/mvCpuIf.h"
#include "ddr1_2/mvDramIfRegs.h"
#include "ethfp/mvEth.h"
#include "ctrlEnv/mvCtrlEnvPadCalibration.h"

#define ENABLE_ETHERNET_PAD_CALIBRATION
#undef  DEBUG_ETHERNET_PAD_CALIBRATION

/* defines  */
#ifdef MV_DEBUG
	#define DB(x)	x
#else
	#define DB(x)
#endif	

#define MAX_LockN	31
#define R_NA		0
      

/* Local */

static MV_CTRL_ENV_DEVICE_PROCESS_TYPE deviceProcessType2_5V[MAX_LockN+1]={
{ R_NA, R_NA, R_NA },	/* LockN =  0 */
{ R_NA, R_NA, R_NA },	/* LockN =  1 */
{ R_NA, R_NA, R_NA },	/* LockN =  2 */
{ R_NA, R_NA, R_NA },	/* LockN =  3 */
{ R_NA, R_NA, R_NA },	/* LockN =  4 */
{ R_NA, R_NA, 5447 },	/* LockN =  5 */
{ R_NA, R_NA, 4539 },	/* LockN =  6 */
{ 5295, R_NA, 3890 },   /* LockN =  7  */ 
{ 4633, R_NA, 3404 },   /* LockN =  8  */ 
{ 4118, R_NA, 3026 },   /* LockN =  9  */ 
{ 3706, 5338, 2723 },   /* LockN = 10  */ 
{ 3369, 4853, 2476 },   /* LockN = 11  */ 
{ 3089, 4448, 2269 },   /* LockN = 12  */ 
{ 2851, 4106, 2095 },   /* LockN = 13  */ 
{ 2647, 3813, R_NA },   /* LockN = 14  */ 
{ 2471, 3559, R_NA },   /* LockN = 15  */ 
{ 2317, 3336, R_NA },   /* LockN = 16  */ 
{ 2180, 3140, R_NA },   /* LockN = 17  */ 
{ 2059, 2966, R_NA },   /* LockN = 18  */ 
{ R_NA, 2810,	-1 },	/* LockN = 19 */
{ R_NA, 2669,	-1 },	/* LockN = 20 */
{ R_NA, 2542,	-1 },	/* LockN = 21 */
{ R_NA, 2426,	-1 },	/* LockN = 22 */
{ R_NA, 2321,	-1 },	/* LockN = 23 */
{ R_NA, 2224,	-1 },	/* LockN = 24 */
{ R_NA, 2135,	-1 },	/* LockN = 25 */
{ R_NA, R_NA, R_NA },	/* LockN = 26 */
{ R_NA, R_NA, R_NA },	/* LockN = 27 */
{ R_NA, R_NA, R_NA },	/* LockN = 28 */
{ R_NA, R_NA, R_NA },	/* LockN = 29 */
{ R_NA, R_NA, R_NA },	/* LockN = 30 */
{ R_NA, R_NA, R_NA },	/* LockN = 31 */
};                             

static MV_CTRL_ENV_DEVICE_PROCESS_TYPE deviceProcessType1_8V[MAX_LockN+1]={
{ R_NA, R_NA, R_NA },	/* LockN =  0 */
{ R_NA, R_NA, R_NA },	/* LockN =  1 */
{ R_NA, R_NA, R_NA },	/* LockN =  2 */
{ R_NA, R_NA, R_NA },	/* LockN =  3 */
{ R_NA, R_NA, R_NA },	/* LockN =  4 */
{ R_NA, R_NA, R_NA },	/* LockN =  5 */
{ R_NA, R_NA, 5617 },	/* LockN =  6 */
{ R_NA, R_NA, 4815 },   /* LockN =  7  */ 
{ R_NA, R_NA, 4213 },   /* LockN =  8  */ 
{ 5564, R_NA, 3745 },   /* LockN =  9  */ 
{ 5008, R_NA, 3370 },   /* LockN = 10  */ 
{ 4553, R_NA, 3064 },   /* LockN = 11  */ 
{ 4173, R_NA, 2808 },   /* LockN = 12  */ 
{ 3852, R_NA, 2592 },   /* LockN = 13  */ 
{ 3577, 5699, 2407 },   /* LockN = 14  */ 
{ 3339, 5319, 2247 },   /* LockN = 15  */ 
{ 3130, 4987, 2106 },   /* LockN = 16  */ 
{ 2946, 4693, R_NA },   /* LockN = 17  */ 
{ 2782, 4432, R_NA },   /* LockN = 18  */ 
{ 2636, 4199,	-1 },	/* LockN = 19 */
{ 2504, 3989,	-1 },	/* LockN = 20 */
{ 2385, 3799,	-1 },	/* LockN = 21 */
{ 2276, 3627,	-1 },	/* LockN = 22 */
{ 2177, 3469,	-1 },	/* LockN = 23 */
{ R_NA, 3324,	-1 },	/* LockN = 24 */
{ R_NA, 3191,	-1 },	/* LockN = 25 */
{ R_NA, 3069, R_NA },	/* LockN = 26 */
{ R_NA, 2955, R_NA },	/* LockN = 27 */
{ R_NA, 2849, R_NA },	/* LockN = 28 */
{ R_NA, 2751, R_NA },	/* LockN = 29 */
{ R_NA, 2659, R_NA },	/* LockN = 30 */
{ R_NA, 2574, R_NA },	/* LockN = 31 */
};


static MV_CTRL_ENV_ETHERNET_PAD_CALIBRATION EthPadCalibrationTableHSTL_1_8V[]={
{11,12},	/* type 1 */
{18,20},	/* type 2 */
{ 7, 8}	/* type 3 */
};

static MV_CTRL_ENV_ETHERNET_PAD_CALIBRATION EthPadCalibrationTable3_3V[]={
{ 8, 9},	/* type 1 */
{13,13},	/* type 2 */
{ 6, 6}		/* type 3 */
};

static MV_CTRL_ENV_ETHERNET_PAD_CALIBRATION EthPadCalibrationTable2_5V[]={		/* Marvel RD board */ 
{ 7, 7},	/* type 1 */
{11,11},	/* type 2 */
{ 5, 5}		/* type 3 */
};



int CpuPadCalibrationV = MV_CTRL_ENV_CPU_INTERFACE_VOLTAGE_1_8V;
int EthInterfaceDrivingStrength = MV_CTRL_ENV_INTERFACE_DRIVING_STRENGTH_2_5V;

static void 	 ctrlEnvEthPadCalibrationSet(int port,MV_U32 drvN, MV_U32 drvP);
static MV_STATUS ctrlEnvCalculatePadCalibration(MV_U32 *pdrvN, MV_U32 *pdrvP);

/*******************************************************************************
* mvCtrlEnvPadCalibrationInit - Initialize the Ethernet Pad Calibration Value
*
* DESCRIPTION:
*       This function is Initialize the Ethernet Pad Calibration
*
* INPUT:
*       Non
* OUTPUT:
*
* RETURN:
*       None.
*
*******************************************************************************/
void	mvCtrlEnvPadCalibrationInit(void)
{
    int port;
	MV_U32	drvN,drvP;
	
	if (MV_ERROR == ctrlEnvCalculatePadCalibration (&drvN, &drvP))
		return;

    /* Init static data structures */
	for (port=0; port<BOARD_ETH_PORT_NUM; port++)
	{
		ctrlEnvEthPadCalibrationSet(port,drvN,drvP);
	} 
}

/*******************************************************************************
* ctrlEnvEthPadCalibrationSet - Setting the Ethernet Pad Calibration Value
*
* DESCRIPTION:
*       This function Setting the Ethernet Pad Calibration Value
*
* INPUT:
*       int port    port number (0,1,2)
*       int drvN   	DrvN Ethernet Pad Calibration Values
*       int drvP	DrvP Ethernet Pad Calibration Values
*
* OUTPUT:
*       See description.
*
* RETURN:
*       None.
*
*******************************************************************************/
static void ctrlEnvEthPadCalibrationSet(int port,MV_U32 drvN, MV_U32 drvP)
{
#ifdef ENABLE_ETHERNET_PAD_CALIBRATION
	MV_U32 	PortReg,regData;

	if ((0 == drvN) && (0 == drvP))				/* Legal Values? */
	{                                                       /* No quit */
		return;
	}
	PortReg = ETH_UNIT_PORTS_PADS_CALIB_0_REG + port*4;	/* calculate port offset */
    	regData = MV_REG_READ(PortReg);				/* Read port value 		 */

	regData &= ~(ETH_ETHERNET_PAD_CLIB_DRVN_MASK |          /* mask the change bits */ 
				 ETH_ETHERNET_PAD_CLIB_DRVP_MASK |
				 ETH_ETHERNET_PAD_CLIB_WR_EN_MASK);			
	MV_REG_WRITE( PortReg,regData);				/* disable Write enable bit */ 

	regData |= ((drvN << ETH_ETHERNET_PAD_CLIB_DRVN_OFFS) & ETH_ETHERNET_PAD_CLIB_DRVN_MASK) |
			   ((drvP << ETH_ETHERNET_PAD_CLIB_DRVP_OFFS) & ETH_ETHERNET_PAD_CLIB_DRVP_MASK);

	MV_REG_WRITE( PortReg,ETH_ETHERNET_PAD_CLIB_WR_EN_MASK);    		/* Enable Write enable bit */ 
	MV_REG_WRITE( PortReg,ETH_ETHERNET_PAD_CLIB_WR_EN_MASK | regData);	/* Write data */  
	MV_REG_WRITE( PortReg,regData);                                     /* disable Write enable bit */ 
#ifdef DEBUG_ETHERNET_PAD_CALIBRATION
	mvOsPrintf("mvEthPadCalibration: ETH_PORTS_PADS_CALIB_%d_REG =0x%X, Data=0x%X \n", 
			   port, PortReg, regData);
#endif
#endif
}

/*******************************************************************************
* ctrlEnvCalculatePadCalibration - Calculate the Ethernet Pad Calibration Value
*
* DESCRIPTION:
*       This function is calculate the Ethernet Pad Calibration drvN and drvP Value 
*
* INPUT:
*       MV_U32 *pDrvN -   ouput pointer to drvN
*       MV_U32 *pDrvP     ouput pointer to drvP
*
* OUTPUT:
*       MV_U32 *pDrvN -   drvN value
*       MV_U32 *pDrvP     drvP value 
*
* RETURN:
*       None.
*
*******************************************************************************/
static MV_STATUS ctrlEnvCalculatePadCalibration(MV_U32 *pdrvN, MV_U32 *pdrvP)
{
#ifdef ENABLE_ETHERNET_PAD_CALIBRATION
	int 	type1,type2,type3;
	int 	Difftype1,Difftype2,Difftype3;
	int 	SelectedType;
	int 	LockN;

	*pdrvN = 0;                       /* Set illegal value */ 
	*pdrvP = 0;
	LockN = (MV_REG_READ(SDRAM_DATA_PADS_CAL_REG) & SDRAM_LOCKN_MAKS )>>SDRAM_LOCKN_OFFS;
#ifdef DEBUG_ETHERNET_PAD_CALIBRATION
	mvOsPrintf("CPU PadCalibration: LockN = 0x%X\n", LockN);
#endif

	if ((LockN < 0) || (LockN>MAX_LockN))
	{
		return MV_ERROR;
	}
	if (CpuPadCalibrationV == MV_CTRL_ENV_CPU_INTERFACE_VOLTAGE_1_8V)
	{
		type1 = deviceProcessType1_8V[LockN].ResistorType1;
		type2 = deviceProcessType1_8V[LockN].ResistorType2;
		type3 = deviceProcessType1_8V[LockN].ResistorType3;
	}
	else
	{
		type1 = deviceProcessType2_5V[LockN].ResistorType1;
		type2 = deviceProcessType2_5V[LockN].ResistorType2;
		type3 = deviceProcessType2_5V[LockN].ResistorType3;
	}
	if ((type1 == R_NA) && (type2 == R_NA) && (type3 == R_NA))	 /* all the types out of range? */
	{
		return MV_ERROR;
	}
	/* find the lower diffrent type and Resitor */
	Difftype1 = (type1 > MV_CTRL_ENV_CPU_PAD_CALIBRATION_RESISTOR)? 
		type1 - MV_CTRL_ENV_CPU_PAD_CALIBRATION_RESISTOR:MV_CTRL_ENV_CPU_PAD_CALIBRATION_RESISTOR - type1;
	Difftype2 = (type2 > MV_CTRL_ENV_CPU_PAD_CALIBRATION_RESISTOR)? 
		type2 - MV_CTRL_ENV_CPU_PAD_CALIBRATION_RESISTOR:MV_CTRL_ENV_CPU_PAD_CALIBRATION_RESISTOR - type2;
	Difftype3 = (type3 > MV_CTRL_ENV_CPU_PAD_CALIBRATION_RESISTOR)? 
		type3 - MV_CTRL_ENV_CPU_PAD_CALIBRATION_RESISTOR:MV_CTRL_ENV_CPU_PAD_CALIBRATION_RESISTOR - type3;
#ifdef DEBUG_ETHERNET_PAD_CALIBRATION
	mvOsPrintf("Ethernet Pad Calibration: type1=%d, type2=%d, type3=%d Diff1=%d, Diff2=%d, Diff3=%d \n", 
				type1,type2,type3,Difftype1,Difftype2,Difftype3);
#endif
	/* find the minimum from Difftype1, Difftype2 and Difftype3  */
	if (Difftype1 <= Difftype2)
	{
		if (Difftype1 <= Difftype3)
		{
			SelectedType = MV_CTRL_ENV_DEVICE_PROCESS_TYPE1;
		}
		else
		{
			SelectedType = MV_CTRL_ENV_DEVICE_PROCESS_TYPE3;

		}
	}
	else
	{
		if (Difftype2 <= Difftype3)
		{
			SelectedType = MV_CTRL_ENV_DEVICE_PROCESS_TYPE2;
		}
		else
		{
			SelectedType = MV_CTRL_ENV_DEVICE_PROCESS_TYPE3;

		}
	}

	/* load the correct drvN and drvP from the selected type table.  */
	if (EthInterfaceDrivingStrength == MV_CTRL_ENV_INTERFACE_DRIVING_STRENGTH_HSTL_1_8V)
	{
		*pdrvN 	= EthPadCalibrationTableHSTL_1_8V[SelectedType].padCalibrationDrvN;
		*pdrvP	= EthPadCalibrationTableHSTL_1_8V[SelectedType].padCalibrationDrvP;
	}
	if (EthInterfaceDrivingStrength == MV_CTRL_ENV_INTERFACE_DRIVING_STRENGTH_3_3V)
	{
		*pdrvN 	= EthPadCalibrationTable3_3V[SelectedType].padCalibrationDrvN;
		*pdrvP	= EthPadCalibrationTable3_3V[SelectedType].padCalibrationDrvP;
	}
	if (EthInterfaceDrivingStrength == MV_CTRL_ENV_INTERFACE_DRIVING_STRENGTH_2_5V)
	{
		*pdrvN 	= EthPadCalibrationTable2_5V[SelectedType].padCalibrationDrvN;
		*pdrvP	= EthPadCalibrationTable2_5V[SelectedType].padCalibrationDrvP;
	}

#ifdef DEBUG_ETHERNET_PAD_CALIBRATION
	mvOsPrintf("mvEthPadCalibration: SelectedType=%d,  drvN=0x%X,drvP=0x%X,  \n", 
			   SelectedType+1,*pdrvN,*pdrvP);
#endif
    return MV_OK;
#endif
}


/*******************************************************************************
* mvCtrlEnvEthInterfaceDrivingStrengthSet 
*
* DESCRIPTION:
*       This function set the interface driving strength voltage
*
* INPUT:
*		None
*
* OUTPUT:
*
* RETURN:
*       None.
*
*******************************************************************************/
void mvCtrlEnvEthInterfaceDrivingStrengthSet(int Value)
{
	if (Value >MV_CTRL_ENV_INTERFACE_DRIVING_STRENGTH_MAX)
	{
		return;
	}
	EthInterfaceDrivingStrength = Value;
}
/*******************************************************************************
* mvCtrlEnvEthInterfaceDrivingStrengthGet 
*
* DESCRIPTION:
*       This function replay the interface driving strength voltage
*
* INPUT:
*		None
*
* OUTPUT:
*
* RETURN:
*       the interface driving strength voltage
*
*******************************************************************************/
int mvCtrlEnvEthInterfaceDrivingStrengthGet(void)
{
	return EthInterfaceDrivingStrength;
}

/*******************************************************************************
* mvCtrlEnvEthCpuPadCalibrationVoltSet 
*
* DESCRIPTION:
*       This function set the ethernet cpu pad calibration voltage
*
* INPUT:
*		None
*
* OUTPUT:
*
* RETURN:
*       None.
*
*******************************************************************************/
void mvCtrlEnvEthCpuPadCalibrationVoltSet(int Value)
{
	if (Value >MV_CTRL_ENV_CPU_INTERFACE_VOLTAGE_MAX)
	{
		return;
	}
	CpuPadCalibrationV = Value;
}
/*******************************************************************************
* mvCtrlEnvEthCpuPadCalibrationVoltGet 
*
* DESCRIPTION:
*       This function replay the CPU pad calibration voltage
*
* INPUT:
*		None
*
* OUTPUT:
*
* RETURN:
*       ethernet cpu pad calibration voltage
*
*******************************************************************************/
int mvCtrlEnvEthCpuPadCalibrationVoltGet(void)
{
	return CpuPadCalibrationV;
}

