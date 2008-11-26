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


#ifndef __INCmvRtcRegh
#define __INCmvRtcRegh


/* RTC Time Register */
#define RTC_TIME_REG				0x10300

/* RTC Date Register */
#define RTC_DATE_REG				0x10304


/* RTC Time Bit Definitions */
#define RTC_TIME_SECONDS_SHF			0
#define RTC_TIME_SECONDS_MSK			(0xF << RTC_TIME_SECONDS_SHF)
#define RTC_TIME_10SECONDS_SHF			4
#define RTC_TIME_10SECONDS_MSK			(0x7 << RTC_TIME_10SECONDS_SHF)

#define RTC_TIME_MINUTES_SHF			8
#define RTC_TIME_MINUTES_MSK			(0xF << RTC_TIME_MINUTES_SHF)
#define RTC_TIME_10MINUTES_SHF			12
#define RTC_TIME_10MINUTES_MSK			(0x7 << RTC_TIME_10MINUTES_SHF)

#define RTC_TIME_HOUR_SHF			16
#define RTC_TIME_HOUR_MSK			(0xF << RTC_TIME_HOUR_SHF)
#define RTC_TIME_10HOUR_SHF			20
#define RTC_TIME_10HOUR_MSK			(0x3 << RTC_TIME_10HOUR_SHF)


#define RTC_TIME_HOUR_FORMAT_SHF		22
#define RTC_TIME_HOUR_FORMAT_12_MSK		(1 << RTC_TIME_HOUR_FORMAT_SHF)
#define RTC_TIME_HOUR_FORMAT_24_MSK		(0 << RTC_TIME_HOUR_FORMAT_SHF)

#define RTC_TIME_DAY_SHF			24
#define RTC_TIME_DAY_MSK			(0x7 << RTC_TIME_DAY_SHF)

/* RTC Date Bit Definitions */
#define RTC_DATE_DAY_SHF			0
#define RTC_DATE_DAY_MSK			(0xF << RTC_DATE_DAY_SHF)
#define RTC_DATE_10DAY_SHF			4
#define RTC_DATE_10DAY_MSK			(0x3 << RTC_DATE_10DAY_SHF)

#define RTC_DATE_MONTH_SHF			8
#define RTC_DATE_MONTH_MSK			(0xF << RTC_DATE_MONTH_SHF)
#define RTC_DATE_10MONTH_SHF			12
#define RTC_DATE_10MONTH_MSK			(0x1 << RTC_DATE_10MONTH_SHF)

#define RTC_DATE_YEAR_SHF			16
#define RTC_DATE_YEAR_MSK			(0xF << RTC_DATE_YEAR_SHF)
#define RTC_DATE_10YEAR_SHF			20
#define RTC_DATE_10YEAR_MSK			(0xF << RTC_DATE_10YEAR_SHF)

#endif /* __INCmvRtcRegh */

