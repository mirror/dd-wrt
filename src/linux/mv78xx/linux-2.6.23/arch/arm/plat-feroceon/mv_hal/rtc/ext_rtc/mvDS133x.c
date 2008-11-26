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
#include "rtc/ext_rtc/mvDS133x.h"
#include "rtc/ext_rtc/mvDS133xReg.h"

static MV_VOID mvRtcCharWrite(MV_U32 offset, MV_U8 data);
static MV_VOID mvRtcCharRead(MV_U32 offset, MV_U8 *data);

#define RTC_I2C_CH MV_BOARD_RTC_I2C_CHANNEL

/*******************************************************************************
* mvRtcDS133xAlarmSet - Set the Alarm of the Real time clock
*
* DESCRIPTION:
*
* INPUT:
*       time - A pointer to a structure RTC_TIME (defined in mvDS133x.h).
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID mvRtcDS133xAlarmSet(MV_RTC_TIME* time)
{
	return;
}
/*******************************************************************************
* mvRtcDS133xTimeSet - Update the Real Time Clock.
*
* DESCRIPTION:
*       This function sets a new time and date to the RTC from the given 
*       structure RTC_TIME . All fields within the structure must be assigned 
*		with a value prior to the use of this function.
*
* INPUT:
*       time - A pointer to a structure RTC_TIME (defined in mvDS133x.h).
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID mvRtcDS133xTimeSet(MV_RTC_TIME* time)
{
	MV_U8 tempValue;
	MV_U8 tens;
	MV_U8 single;

	/* seconds */
	tens = time->seconds / 10;
	single = time->seconds % 10;
	tempValue = ((tens  << RTC_CLOCK_10SECONDS_SHF )& RTC_CLOCK_10SECONDS_MSK )|
				(( single << RTC_CLOCK_SECONDS_SHF)&RTC_CLOCK_SECONDS_MSK) ;
	mvRtcCharWrite(RTC_CLOCK_SECONDS,tempValue);

	/* minutes */
	tens = time->minutes / 10;
	single = time->minutes % 10;
	tempValue = ((tens  << RTC_CLOCK_10MINUTES_SHF )& RTC_CLOCK_10MINUTES_MSK )|
			    (( single << RTC_CLOCK_MINUTES_SHF)& RTC_CLOCK_MINUTES_MSK) ;
	mvRtcCharWrite(RTC_CLOCK_MINUTES,tempValue);

	/* hours (24) */
	tens = time->hours / 10;
	single = time->hours % 10;
	tempValue = ((tens << RTC_CLOCK_10HOURS_SHF) & RTC_CLOCK_10HOURS_MSK2 )|
				(( single << RTC_CLOCK_HOURS_SHF ) & RTC_CLOCK_HOURS_MSK);
	mvRtcCharWrite(RTC_CLOCK_HOUR,tempValue);
  
	/* day */
	single = ++(time->day);
	tempValue = ((single << RTC_CLOCK_DAY_SHF ) & RTC_CLOCK_DAY_MSK) ;
	mvRtcCharWrite(RTC_CLOCK_DAY,tempValue);

	/* date */
	tens = time->date / 10;
	single = time->date % 10;
	tempValue = ((tens << RTC_CLOCK_10DATE_SHF ) & RTC_CLOCK_10DATE_MSK )|
				((single << RTC_CLOCK_DATE_SHF )& RTC_CLOCK_DATE_MSK) ;

	/* month */
	mvRtcCharWrite( RTC_CLOCK_DATE,tempValue);
	tens = time->month / 10;
	single = time->month % 10;
	tempValue = ((tens << RTC_CLOCK_10MONTH_SHF ) & RTC_CLOCK_10MONTH_MSK )|
				((single << RTC_CLOCK_MONTH_SHF)& RTC_CLOCK_MONTH_MSK);
	mvRtcCharWrite( RTC_CLOCK_MONTH_CENTURY,tempValue);
    
	/* year */
	tens = time->year / 10;
	single = time->year % 10;
	tempValue = ((tens << RTC_CLOCK_10YEAR_SHF) & RTC_CLOCK_10YEAR_MSK )|
				((single << RTC_CLOCK_YEAR_SHF) & RTC_CLOCK_YEAR_MSK);
	mvRtcCharWrite(RTC_CLOCK_YEAR,tempValue);

	return;
}

/*******************************************************************************
* mvRtcDS133xTimeGet - Read the time from the RTC.
*
* DESCRIPTION:
*       This function reads the current time and date from the RTC into the 
*       structure RTC_TIME (defined in mvDS133x.h).
*
* INPUT:
*       time - A pointer to a structure TIME (defined in mvDS133x.h).
*
* OUTPUT:
*       The structure RTC_TIME is updated with the time read from the RTC.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID mvRtcDS133xTimeGet(MV_RTC_TIME* time)
{
	MV_U8 tempValue;
	MV_U32 tens;
	MV_U32 single;

	/* seconds */
	mvRtcCharRead(RTC_CLOCK_SECONDS,&tempValue);
	tens = ( tempValue & RTC_CLOCK_10SECONDS_MSK) >> RTC_CLOCK_10SECONDS_SHF;   
	single = (tempValue & RTC_CLOCK_SECONDS_MSK) >> RTC_CLOCK_SECONDS_SHF ;
	time->seconds = 10*tens + single;

	/* minutes */
	mvRtcCharRead(RTC_CLOCK_MINUTES,&tempValue);
	tens = (tempValue & RTC_CLOCK_10MINUTES_MSK) >> RTC_CLOCK_10MINUTES_SHF;   
	single = (tempValue & RTC_CLOCK_MINUTES_MSK) >> RTC_CLOCK_MINUTES_SHF;
	time->minutes = 10*tens + single;

	/* hours */
	mvRtcCharRead(RTC_CLOCK_HOUR,&tempValue);
	tens = (tempValue & RTC_CLOCK_10HOURS_MSK2) >> RTC_CLOCK_10HOURS_SHF;   
	single = (tempValue & RTC_CLOCK_HOURS_MSK) >> RTC_CLOCK_HOURS_SHF;
	time->hours = 10*tens + single;

	/* day */
	mvRtcCharRead(RTC_CLOCK_DAY,&tempValue);
	time->day = (tempValue & RTC_CLOCK_DAY_MSK) >> RTC_CLOCK_DAY_SHF ;
	time->day--;

	/* date */
	mvRtcCharRead(RTC_CLOCK_DATE,&tempValue);
	tens = (tempValue & RTC_CLOCK_10DATE_MSK) >> RTC_CLOCK_10DATE_SHF;   
	single = (tempValue & RTC_CLOCK_DATE_MSK) >> RTC_CLOCK_DATE_SHF;
	time->date = 10*tens + single;

	/* century/ month */
	mvRtcCharRead(RTC_CLOCK_MONTH_CENTURY,&tempValue);
	tens = (tempValue & RTC_CLOCK_10MONTH_MSK) >> RTC_CLOCK_10MONTH_SHF;   
	single = (tempValue & RTC_CLOCK_MONTH_MSK) >> RTC_CLOCK_MONTH_SHF;
	time->month = 10*tens + single;
	time->century = (tempValue & RTC_CLOCK_CENTURY_MSK)>>RTC_CLOCK_CENTURY_SHF;

	/* year */
	mvRtcCharRead(RTC_CLOCK_YEAR,&tempValue);
	tens = (tempValue & RTC_CLOCK_10YEAR_MSK) >> RTC_CLOCK_10YEAR_SHF;   
	single = (tempValue & RTC_CLOCK_YEAR_MSK) >> RTC_CLOCK_YEAR_SHF;
	time->year = 10*tens + single;

	return;	
}

/*******************************************************************************
* mvRtcDS1339Init - Initialize the clock.
*
* DESCRIPTION:
*       This function initialize the clock registers and read\write functions
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
MV_VOID mvRtcDS1339Init(MV_VOID)
{
	MV_U8 ucTemp;

	/* We will disable interrupts as default .*/
	mvRtcCharRead(RTC1339_CONTROL,&ucTemp);
	ucTemp|=RTC_CONTROL_INTCN_BIT;
	mvRtcCharWrite(RTC1339_CONTROL,ucTemp);
    
	/* disable trickle */
	mvRtcCharWrite(RTC_TRICKLE_CHARGE,0);

	return;
}


/*******************************************************************************
* mvRtcDS1338Init - Initialize the clock.
*
* DESCRIPTION:
*       This function initialize the clock registers and read\write functions
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
MV_VOID mvRtcDS1338Init(MV_VOID)
{
	MV_U8 ucTemp;

	/* We will enable OSC .*/
	mvRtcCharRead(RTC_CLOCK_SECONDS,&ucTemp);
	ucTemp &=~RTC_CONTROL_EOSC_MSK;
	mvRtcCharWrite(RTC_CLOCK_SECONDS,ucTemp);

	return;
}

/* assumption twsi is initialized !!! */
/*******************************************************************************
* mvRtcCharRead - Read a char from the RTC.
*
* DESCRIPTION:
*       This function reads a char from the RTC offset.
*
* INPUT:
*       offset - offset
*
* OUTPUT:
*       data - char read from offset offset.
*
* RETURN:
*       None.
*
*******************************************************************************/
static MV_VOID	mvRtcCharRead(MV_U32 offset, MV_U8 *data)
{
	MV_TWSI_SLAVE   twsiSlave;
	
	twsiSlave.slaveAddr.type = mvBoardRtcTwsiAddrTypeGet();
	twsiSlave.slaveAddr.address = mvBoardRtcTwsiAddrGet();
	twsiSlave.validOffset = MV_TRUE;
	twsiSlave.offset = offset;
	twsiSlave.moreThen256 = MV_FALSE;
	mvTwsiRead (RTC_I2C_CH, &twsiSlave, data, 1);

	return;
}

/*******************************************************************************
* mvRtcCharWrite - Write a char from the RTC.
*
* DESCRIPTION:
*       This function writes a char to the RTC offset.
*
* INPUT:
*       offset - offset
*
* OUTPUT:
*       data - char write to addr address.
*
* RETURN:
*       None.
*
*******************************************************************************/
static MV_VOID mvRtcCharWrite(MV_U32 offset, MV_U8 data)
{
	MV_TWSI_SLAVE twsiSlave;
	
	twsiSlave.slaveAddr.type = mvBoardRtcTwsiAddrTypeGet();
	twsiSlave.slaveAddr.address = mvBoardRtcTwsiAddrGet();
	twsiSlave.validOffset = MV_TRUE;
	twsiSlave.offset = offset;
	twsiSlave.moreThen256 = MV_FALSE;
	mvTwsiWrite (RTC_I2C_CH, &twsiSlave, &data, 1);

	return;
}
