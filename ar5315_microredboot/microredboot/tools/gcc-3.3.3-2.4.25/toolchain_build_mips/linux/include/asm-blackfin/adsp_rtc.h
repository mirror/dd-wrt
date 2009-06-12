/*
 * include/asm-frionommu/memdma.h - Function declaration for memory to 
 * memory dma for Blackfin ADSP-21535
 *  derived from:
 *  include/asm-i386/dma.h
 *  Written by Hennus Bergman, 1992.
 *  Hannu Savolainen  and John Boyd, Nov. 1992.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2,
 * or (at your option) any later version as  published by the Free 
 * Software Foundation.
 *
 */
#ifndef _ADSP_RTC_H
#define ADSP_RTC_H 1

#define BYTE_REF(addr)		 (*((volatile unsigned char*)addr))
#define HALFWORD_REF(addr) 	 (*((volatile unsigned short*)addr))
#define WORD_REF(addr)		 (*((volatile unsigned long*)addr))
#define RTC_STAT_ADDR	0xffc01400 /*RTC Status register 17-8 */
#define RTC_STAT	WORD_REF(RTC_STAT_ADDR) /*RTC Status register 17-8 */
#define RTC_ICTL_ADDR	0xffc01404 /*RTC Interrupt Control register 17-9 */
#define RTC_ICTL	HALFWORD_REF(RTC_ICTL_ADDR) /*RTC Interrupt Control register 17-9 */
#define RTC_ISTAT_ADDR	0xffc01408 /*RTC Interrupt Status register 17-10 */
#define RTC_ISTAT	HALFWORD_REF(RTC_ISTAT_ADDR) /*RTC Interrupt Status register 17-10 */
#define RTC_SWCNT_ADDR	0xffc0140c /*RTC Stopwatch Count register 17-11 */
#define RTC_SWCNT	HALFWORD_REF(RTC_SWCNT_ADDR) /*RTC Stopwatch Count register 17-11 */
#define RTC_ALARM_ADDR	0xffc01410 /*RTC Alarm register 17-12 */
#define RTC_ALARM	WORD_REF(RTC_ALARM_ADDR) /*RTC Alarm register 17-12 */
#define RTC_FAST_ADDR	0xffc01414 /*RTC Enable register 17-13 */
#define RTC_FAST	HALFWORD_REF(RTC_FAST_ADDR) /*RTC Enable register 17-13 */


#define ENABLE_PRESCALER				0x0001
#define RESET						0x0000
#define RTC_SECONDS					0x0000003f
#define RTC_MINUTES					0x00003f00
#define RTC_HOURS					0x001f0000
#define	RTC_DAYS					0xff000000
#define WRITE_COMPLETE_ENABLE				0x8000
#define WRITE_COMPLETE				 WRITE_COMPLETE_ENABLE
#define WRITE_PENDING				0X4000

/* ioctls */
#define RTC_GET_TIME			1
#define RTC_SET_TIME			2

struct rtc_time {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};
#endif
