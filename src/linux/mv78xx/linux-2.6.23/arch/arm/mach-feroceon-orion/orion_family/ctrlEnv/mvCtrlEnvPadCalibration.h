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
/******************************************************************************
* mvcltrlEnvPadCalibration.h - Header File for : MV-MV64560 Setting the Ethernet   *
*                         Pad Calibration Value                               *
*                                                                             *
* DESCRIPTION:                                                                *
*       This header file contains macros typedefs and the ethernet pad        *
*       calibration Value tables                                              *
*                                                                             *
* DEPENDENCIES:                                                               *
*       None.                                                                 *
*                                                                             *
*******************************************************************************/

#ifndef __mvCtrlEnvEthPad_Calibration_h__
#define __mvCtrlEnvEthPad_Calibration_h__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* includes */

/* defines  */
/*  CPU interface voltage in use. */

typedef enum __mvethcpuinterfacevoltage__
{
    MV_CTRL_ENV_CPU_INTERFACE_VOLTAGE_1_8V,
	MV_CTRL_ENV_CPU_INTERFACE_VOLTAGE_2_5,
    
}MV_CTRL_ENV_CPU_VOLTAGE;

#define MV_ETH_CPU_INTERFACE_VOLTAGE 			MV_CTRL_ENV_CPU_INTERFACE_VOLTAGE_1_8V
#define MV_CTRL_ENV_CPU_INTERFACE_VOLTAGE_MAX 	MV_CTRL_ENV_CPU_INTERFACE_VOLTAGE_2_5


/* The Ethernet interface driving strength  (Table 33 in Doc. No. MV-S300848-00, Rev. A) */	  
#undef  MV_CTRL_ENV_ETHERNET_INTERFACE_DRIVING_STRENGTH_HSTL_1_8V 
#undef  MV_CTRL_ENV_ETHERNET_INTERFACE_DRIVING_STRENGTH_3_3V 
#define MV_CTRL_ENV_ETHERNET_INTERFACE_DRIVING_STRENGTH_2_5V 	/* Marvel RD board */

typedef enum __mvCtrlEnvInterface_driving_strengthvoltage__
{
	 MV_CTRL_ENV_INTERFACE_DRIVING_STRENGTH_HSTL_1_8V, 
	 MV_CTRL_ENV_INTERFACE_DRIVING_STRENGTH_3_3V, 
	 MV_CTRL_ENV_INTERFACE_DRIVING_STRENGTH_2_5V 	/* Marvel RD board */
}MV_ETH_DRIVING_STRENGTH_VOLTAGE_ENUM;

#define MV_CTRL_ENV_INTERFACE_DRIVING_STRENGTH_MAX MV_CTRL_ENV_INTERFACE_DRIVING_STRENGTH_2_5V

#define MV_CTRL_ENV_DRIVING_STRENGTH_VOLTAGE			MV_CTRL_ENV_INTERFACE_DRIVING_STRENGTH_2_5V

#define MV_CTRL_ENV_CPU_PAD_CALIBRATION_RESISTOR	(50*100)			/* 50ohm is for single CPU */

/* CPU INTERFACE VOLTAGE */
typedef struct _mvEthDeviceProcessType
{
    int     ResistorType1;
    int     ResistorType2;
    int     ResistorType3;
} MV_CTRL_ENV_DEVICE_PROCESS_TYPE;


/* The Ethernet interface driving strength table */	  

typedef struct _EthernetPadCalibration
{
    int     padCalibrationDrvN;
    int     padCalibrationDrvP;
} MV_CTRL_ENV_ETHERNET_PAD_CALIBRATION;


typedef enum __mvethEthernetPadCalibrationType_
{
    MV_CTRL_ENV_DEVICE_PROCESS_TYPE1,
    MV_CTRL_ENV_DEVICE_PROCESS_TYPE2,
    MV_CTRL_ENV_DEVICE_PROCESS_TYPE3,
    
}MV_CTRL_ENV_ETHERNET_PAD_CALIBRATION_TYPE;


/* function */

void  mvCtrlEnvPadCalibrationInit(void);
void  mvCtrlEnvEthInterfaceDrivingStrengthSet(int Value);
int   mvCtrlEnvEthInterfaceDrivingStrengthGet(void);
void  mvCtrlEnvEthCpuPadCalibrationVoltSet(int Value);
int   mvCtrlEnvEthCpuPadCalibrationVoltGet(void);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __mvCtrlEnvEthPad_Calibration_h__ */




