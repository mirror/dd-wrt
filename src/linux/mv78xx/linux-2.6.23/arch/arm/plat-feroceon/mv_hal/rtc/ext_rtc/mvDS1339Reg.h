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


#ifndef __INCmvDS1339Regh
#define __INCmvDS1339Regh

#ifndef MSK
#define MSK(n)						((1 << (n)) - 1)
#endif

#define RTC_CLOCK_SECONDS			0x0
#define RTC_CLOCK_MINUTES			0x1
#define RTC_CLOCK_HOUR				0x2
#define RTC_CLOCK_DAY				0x3
#define RTC_CLOCK_DATE				0x4
#define RTC_CLOCK_MONTH_CENTURY		0x5
#define RTC_CLOCK_YEAR				0x6
#define RTC_ALARM_SECONDS			0x7
#define RTC_ALARM_MINUTES			0x8
#define RTC_ALARM_HOUR				0x9
#define RTC_ALARM_DAY_DATE			0xa
#define RTC_ALARM2_MINUTES			0xb
#define RTC_ALARM2_HOUR				0xc
#define RTC_ALARM2_DAY_DATE 		0xd
#define RTC_CONTROL					0xe
#define RTC_STATUS					0xf
#define RTC_TRICKLE_CHARGE			0x10

#define FIRST_REG_OFF				RTC_CLOCK_SECONDS
#define LAST_REG_OFF				RTC_TRICKLE_CHARGE


/* RTC_CLOCK_SECONDS   0x0*/

#define RTC_CLOCK_SECONDS_SHF		0
#define RTC_CLOCK_SECONDS_MSK		(MSK(4)<<RTC_CLOCK_SECONDS_SHF)

#define RTC_CLOCK_10SECONDS_SHF		4
#define RTC_CLOCK_10SECONDS_MSK		(MSK(3)<<RTC_CLOCK_10SECONDS_SHF)

/* RTC_CLOCK_MINUTES   0x1*/

#define RTC_CLOCK_MINUTES_SHF		0
#define RTC_CLOCK_MINUTES_MSK		(MSK(4)<<RTC_CLOCK_MINUTES_SHF)

#define RTC_CLOCK_10MINUTES_SHF		4
#define RTC_CLOCK_10MINUTES_MSK		(MSK(3)<<RTC_CLOCK_10MINUTES_SHF)

/* RTC_CLOCK_HOUR      0x2*/

#define RTC_CLOCK_HOURS_SHF				0
#define RTC_CLOCK_HOURS_MSK				(MSK(4)<<RTC_CLOCK_HOURS_SHF)

#define RTC_CLOCK_10HOURS_SHF			4
#define RTC_CLOCK_10HOURS_MSK1			(MSK(1)<<RTC_CLOCK_10HOURS_SHF)
#define RTC_CLOCK_10HOURS_MSK2			(MSK(2)<<RTC_CLOCK_10HOURS_SHF)
#define RTC_CLOCK_10HOURS_BIT			BIT4

#define RTC_CLOCK_10HOURSAMPM_SHF		5
#define RTC_CLOCK_10HOURSAMPM_MSK		(MSK(1)<<RTC_CLOCK_10HOURSAMPM_SHF)
#define RTC_CLOCK_10HOURSAMPM_BIT		BIT5

#define RTC_CLOCK_HOUR_12_24			6
#define RTC_CLOCK_HOUR_12_24_MSK		(MSK(1)<<RTC_CLOCK_HOUR_12_24_SHF)
#define RTC_CLOCK_HOUR_12_24_BIT		BIT6


/* RTC_CLOCK_DAY       	0x3*/

#define RTC_CLOCK_DAY_SHF				0
#define RTC_CLOCK_DAY_MSK				(MSK(3)<<RTC_CLOCK_DAY_SHF)

/* RTC_CLOCK_DATE      	0x4*/

#define RTC_CLOCK_DATE_SHF				0
#define RTC_CLOCK_DATE_MSK				(MSK(4)<<RTC_CLOCK_DATE_SHF)

#define RTC_CLOCK_10DATE_SHF			4
#define RTC_CLOCK_10DATE_MSK			(MSK(2)<<RTC_CLOCK_10DATE_SHF)


/* RTC_CLOCK_MONTH_CENTURY    	0x5*/

#define RTC_CLOCK_MONTH_SHF				0
#define RTC_CLOCK_MONTH_MSK				(MSK(4)<<RTC_CLOCK_MONTH_SHF)

#define RTC_CLOCK_10MONTH_SHF			4
#define RTC_CLOCK_10MONTH_MSK			(MSK(1)<<RTC_CLOCK_10MONTH_SHF)
#define RTC_CLOCK_10MONTH_BIT			BIT4


#define RTC_CLOCK_CENTURY_SHF			7
#define RTC_CLOCK_CENTURY_MSK			(MSK(1)<<RTC_CLOCK_CENTURY_SHF)
#define RTC_CLOCK_CENTURY_BIT			BIT7


/* RTC_CLOCK_YEAR      	0x6*/

#define RTC_CLOCK_YEAR_SHF				0
#define RTC_CLOCK_YEAR_MSK				(MSK(4)<<RTC_CLOCK_YEAR_SHF)

#define RTC_CLOCK_10YEAR_SHF			4
#define RTC_CLOCK_10YEAR_MSK			(MSK(4)<<RTC_CLOCK_10YEAR_SHF)


/* RTC_ALARM_SECONDS  	0x7*/

#define RTC_CLOCK_SECONDS_ALRM1_SHF		0
#define RTC_CLOCK_SECONDS_ALRM1_MSK		(MSK(4)<<RTC_CLOCK_SECONDS_ALRM1_SHF)

#define RTC_CLOCK_10SECONDS_ALRM1_SHF	4
#define RTC_CLOCK_10SECONDS_ALRM1_MSK	(MSK(3)<<RTC_CLOCK_10SECONDS_ALRM1_SHF)

#define RTC_CLOCK_A1M1_SHF				0
#define RTC_CLOCK_A1M1_MSK				(MSK(1)<<RTC_CLOCK_A1M1_SHF)
#define RTC_CLOCK_A1M1_BIT				BIT7


/* RTC_ALARM_MINUTES   	0x8*/

#define RTC_CLOCK_MINUTES_ALRM1_SHF		0
#define RTC_CLOCK_MINUTES_ALRM1_MSK		(MSK(4)<<RTC_CLOCK_MINUTES_ALRM1_SHF)

#define RTC_CLOCK_10MINUTES_ALRM1_SHF	4
#define RTC_CLOCK_10MINUTES_ALRM1_MSK	(MSK(3)<<RTC_CLOCK_10MINUTES_ALRM1_SHF)

#define RTC_CLOCK_A1M2_SHF				0
#define RTC_CLOCK_A1M2_MSK				(MSK(1)<<RTC_CLOCK_A1M2_SHF)
#define RTC_CLOCK_A1M2_BIT				BIT7

/* RTC_ALARM_HOUR      	0x9*/

#define RTC_CLOCK_HOURS_ALRM1_SHF		0
#define RTC_CLOCK_HOURS_ALRM1_MSK		(MSK(4)<<RTC_CLOCK_HOURS_ALRM1_SHF)

#define RTC_CLOCK_10HOURS_ALRM1_SHF		4
#define RTC_CLOCK_10HOURS_ALRM1_MSK1	(MSK(1)<<RTC_CLOCK_10HOURS_ALRM1_SHF)
#define RTC_CLOCK_10HOURS_ALRM1_MSK2	(MSK(2)<<RTC_CLOCK_10HOURS_ALRM1_SHF)
#define RTC_CLOCK_10HOURS_ALRM1_BIT		BIT4

#define RTC_CLOCK_10HOURS_ALRM1AMPM_SHF	5
#define RTC_CLOCK_10HOURS_ALRM1AMPM_MSK	(MSK(1)<<RTC_CLOCK_10HOURS_ALRM1AMPM_SHF)
#define RTC_CLOCK_10HOURS_ALRM1AMPM_BIT	BIT5

#define RTC_CLOCK_HOUR_ALRM1_12_24		6
#define RTC_CLOCK_HOUR_ALRM1_12_24_MSK	(MSK(1)<<RTC_CLOCK_HOUR_ALRM1_12_24_SHF)
#define RTC_CLOCK_HOUR_ALRM1_12_24_BIT	BIT6

#define RTC_CLOCK_A1M3_SHF				0
#define RTC_CLOCK_A1M3_MSK				(MSK(1)<<RTC_CLOCK_A1M3_SHF)
#define RTC_CLOCK_A1M3_BIT				BIT7


/* RTC_ALARM_DAY_DATE  	0xa*/

#define RTC_CLOCK_DAY_ALRM1_SHF			0
#define RTC_CLOCK_DAY_ALRM1_MSK			(MSK(3)<<RTC_CLOCK_DAY_ALRM1_SHF)

#define RTC_CLOCK_DATE_ALRM1_SHF		0
#define RTC_CLOCK_DATE_ALRM1_MSK		(MSK(4)<<RTC_CLOCK_DATE_ALRM1_SHF)

#define RTC_CLOCK_10DATE_ALRM1_SHF		4
#define RTC_CLOCK_10DATE_ALRM1_MSK		(MSK(2)<<RTC_CLOCK_10DATE_ALRM1_SHF)

#define RTC_CLOCK_DY_DT_ALRM1_SHF		6
#define RTC_CLOCK_DY_DT_ALRM1_MSK		(MSK(1)<<RTC_CLOCK_DY_DT_ALRM1_SHF)
#define RTC_CLOCK_DY_DT_ALRM1_BIT		BIT6

#define RTC_CLOCK_A1M4_SHF				0
#define RTC_CLOCK_A1M4_MSK				(MSK(1)<<RTC_CLOCK_A1M4_SHF)
#define RTC_CLOCK_A1M4_BIT				BIT7

/* RTC_ALARM2_MINUTES  	0xb*/

#define RTC_CLOCK_MINUTES_ALRM2_SHF		0
#define RTC_CLOCK_MINUTES_ALRM2_MSK		(MSK(4)<<RTC_CLOCK_MINUTES_ALRM2_SHF)

#define RTC_CLOCK_10MINUTES_ALRM2_SHF	4
#define RTC_CLOCK_10MINUTES_ALRM2_MSK	(MSK(3)<<RTC_CLOCK_10MINUTES_ALRM2_SHF)

#define RTC_CLOCK_A2M2_SHF				0
#define RTC_CLOCK_A2M2_MSK				(MSK(1)<<RTC_CLOCK_A2M2_SHF)
#define RTC_CLOCK_A2M2_BIT				BIT7

/* RTC_ALARM2_HOUR     	0xc*/

#define RTC_CLOCK_HOURS_ALRM2_SHF		0
#define RTC_CLOCK_HOURS_ALRM2_MSK		(MSK(4)<<RTC_CLOCK_HOURS_ALRM2_SHF)

#define RTC_CLOCK_10HOURS_ALRM2_SHF		4
#define RTC_CLOCK_10HOURS_ALRM2_MSK1	(MSK(1)<<RTC_CLOCK_10HOURS_ALRM2_SHF)
#define RTC_CLOCK_10HOURS_ALRM2_MSK2	(MSK(2)<<RTC_CLOCK_10HOURS_ALRM2_SHF)
#define RTC_CLOCK_10HOURS_ALRM2_BIT		BIT4

#define RTC_CLOCK_10HOURS_ALRM2AMPM_SHF	5
#define RTC_CLOCK_10HOURS_ALRM2AMPM_MSK	(MSK(1)<<RTC_CLOCK_10HOURS_ALRM2AMPM_SHF)
#define RTC_CLOCK_10HOURS_ALRM2AMPM_BIT	BIT5

#define RTC_CLOCK_HOUR_ALRM2_12_24		6
#define RTC_CLOCK_HOUR_ALRM2_12_24_MSK	(MSK(1)<<RTC_CLOCK_HOUR_ALRM2_12_24_SHF)
#define RTC_CLOCK_HOUR_ALRM2_12_24_BIT	BIT6


#define RTC_CLOCK_A2M3_SHF 				0
#define RTC_CLOCK_A2M3_MSK				(MSK(1)<<RTC_CLOCK_A2M3_SHF)
#define RTC_CLOCK_A2M3_BIT				BIT7

/* RTC_ALARM2_DAY_DATE 	0xd*/

#define RTC_CLOCK_DAY_ALRM2_SHF			0
#define RTC_CLOCK_DAY_ALRM2_MSK			(MSK(3)<<RTC_CLOCK_DAY_ALRM2_SHF)

#define RTC_CLOCK_DATE_ALRM2_SHF		0
#define RTC_CLOCK_DATE_ALRM2_MSK		(MSK(4)<<RTC_CLOCK_DATE_ALRM2_SHF)

#define RTC_CLOCK_10DATE_ALRM2_SHF		4
#define RTC_CLOCK_10DATE_ALRM2_MSK		(MSK(2)<<RTC_CLOCK_10DATE_ALRM2_SHF)

#define RTC_CLOCK_DY_DT_ALRM2_SHF		6
#define RTC_CLOCK_DY_DT_ALRM2_MSK		(MSK(1)<<RTC_CLOCK_DY_DT_ALRM2_SHF)
#define RTC_CLOCK_DY_DT_ALRM2_BIT		BIT6


#define RTC_CLOCK_A2M4_SHF				0
#define RTC_CLOCK_A2M4_MSK				(MSK(1)<<RTC_CLOCK_A2M4_SHF)
#define RTC_CLOCK_A2M4_BIT				BIT7

/* RTC_CONTROL  			0xe*/

#define RTC_CONTROL_A1IE_SHF			0
#define RTC_CONTROL_A1IE_MSK			(MSK(1)<<RTC_CONTROL_A1IE_SHF)
#define RTC_CONTROL_A1IE_BIT			BIT0

#define RTC_CONTROL_A2IE_SHF			1
#define RTC_CONTROL_A2IE_MSK			(MSK(1)<<RTC_CONTROL_A2IE_SHF)
#define RTC_CONTROL_A2IE_BIT			BIT1

#define RTC_CONTROL_INTCN_SHF			2
#define RTC_CONTROL_INTCN_MSK			(MSK(1)<<RTC_CONTROL_INTCN_SHF)
#define RTC_CONTROL_INTCN_BIT			BIT2

#define RTC_CONTROL_RS1_SHF				3
#define RTC_CONTROL_RS1_MSK				(MSK(1)<<RTC_CONTROL_RS1_SHF)
#define RTC_CONTROL_RS1_BIT				BIT3

#define RTC_CONTROL_RS2_SHF				4
#define RTC_CONTROL_RS2_MSK				(MSK(1)<<RTC_CONTROL_RS2_SHF)
#define RTC_CONTROL_RS2_BIT				BIT4

#define RTC_CONTROL_BBSQI_SHF			5
#define RTC_CONTROL_BBSQI_MSK			(MSK(1)<<RTC_CONTROL_BBSQI_SHF)
#define RTC_CONTROL_BBSQI_BIT			BIT5

#define RTC_CONTROL_EOSC_SHF			7
#define RTC_CONTROL_EOSC_MSK			(MSK(1)<<RTC_CONTROL_EOSC_SHF)
#define RTC_CONTROL_EOSC_BIT			BIT7


/* RTC_STATUS  			0xf*/
#define RTC_CONTROL_A1F_SHF				0
#define RTC_CONTROL_A1F_MSK				(MSK(1)<<RTC_CONTROL_A1F_SHF)
#define RTC_CONTROL_A1F_BIT				BIT0

#define RTC_CONTROL_A2F_SHF				1
#define RTC_CONTROL_A2F_MSK				(MSK(1)<<RTC_CONTROL_A2F_SHF)
#define RTC_CONTROL_A2F_BIT				BIT1

#define RTC_CONTROL_OSF_SHF				7
#define RTC_CONTROL_OSF_MSK				(MSK(1)<<RTC_CONTROL_OSF_SHF)
#define RTC_CONTROL_OSF_BIT				BIT7

/* RTC_TRICKLE_CHARGE  	0x10*/
#define RTC_CONTROL_ROUT0_SHF			0
#define RTC_CONTROL_ROUT0_MSK			(MSK(1)<<RTC_CONTROL_ROUT0_SHF)
#define RTC_CONTROL_ROUT0_BIT			BIT0

#define RTC_CONTROL_ROUT1_SHF			1
#define RTC_CONTROL_ROUT1_MSK			(MSK(1)<<RTC_CONTROL_ROUT1_SHF)
#define RTC_CONTROL_ROUT1_BIT			BIT1

#define RTC_CONTROL_DS0_SHF				2
#define RTC_CONTROL_DS0_MSK				(MSK(1)<<RTC_CONTROL_DS0_SHF)
#define RTC_CONTROL_DS0_BIT				BIT2

#define RTC_CONTROL_DS1_SHF				3
#define RTC_CONTROL_DS1_MSK				(MSK(1)<<RTC_CONTROL_DS1_SHF)
#define RTC_CONTROL_DS1_BIT				BIT3

#define RTC_CONTROL_TCS0_SHF			4
#define RTC_CONTROL_TCS0_MSK			(MSK(1)<<RTC_CONTROL_TCS0_SHF)
#define RTC_CONTROL_TCS0_BIT			BIT4

#define RTC_CONTROL_TCS1_SHF			5
#define RTC_CONTROL_TCS1_MSK			(MSK(1)<<RTC_CONTROL_TCS1_SHF)
#define RTC_CONTROL_TCS1_BIT			BIT5

#define RTC_CONTROL_TCS2_SHF			6
#define RTC_CONTROL_TCS2_MSK			(MSK(1)<<RTC_CONTROL_TCS2_SHF)
#define RTC_CONTROL_TCS2_BIT			BIT6

#define RTC_CONTROL_TCS3_SHF			7
#define RTC_CONTROL_TCS3_MSK			(MSK(1)<<RTC_CONTROL_TCS3_SHF)
#define RTC_CONTROL_TCS3_BIT			BIT7

#endif /* __INCmvDS1339Regh */

