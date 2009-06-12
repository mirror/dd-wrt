/****************************************************************************
*
*	Name:			pwrmanag.h
*
*	Description:	
*
*	Copyright:		(c) 2002 Conexant Systems Inc.
*
*****************************************************************************

  This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option)
any later version.

  This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

  You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc., 59
Temple Place, Suite 330, Boston, MA 02111-1307 USA

****************************************************************************
*  $Author: gerg $
*  $Revision: 1.1 $
*  $Modtime: 4/16/02 10:51a $
****************************************************************************/

#ifndef PWRMANAG_H
#define PWRMANAG_H

typedef enum
{
   DEVICE_ARMCORE,
   DEVICE_FLASH,
   DEVICE_SDRAM,
   #if CONFIG_CNXT_ADSL || CONFIG_CNXT_ADSL_MODULE
      DEVICE_FALCON,
      DEVICE_AFE,
   #endif
   DEVICE_EPHY
} eHWDevice;


typedef enum
{
   DEVICE_ON,
   DEVICE_OFF,
   DEVICE_SLEEP,
   DEVICE_RESET,
   DEVICE_SLOW_SPEED,
   DEVICE_NORMAL_SPEED
} eHWPowerState;


void sysDevicePowerCtl(eHWDevice eDeviceType, eHWPowerState ePowerState);
void sysARMCOREPwrCtl(eHWPowerState ePowerState);
void sysSDRAMPwrCtl(eHWPowerState ePowerState);
#if CONFIG_CNXT_ADSL || CONFIG_CNXT_ADSL_MODULE
   void sysFALCONPwrCtl(eHWPowerState ePowerState);
   void sysAFEPwrCtl(eHWPowerState ePowerState);
#endif //CONFIG_CNXT_ADSL || CONFIG_CNXT_ADSL_MODULE

#endif
