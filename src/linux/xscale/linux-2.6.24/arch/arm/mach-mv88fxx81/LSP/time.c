/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/rtc.h>

#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/rtc.h>

#include "mvDS1339.h"

#define FEBRUARY                2
#define STARTOFTIME             1970
#define SECDAY                  86400L
#define SECYR                   (SECDAY * 365)
                                                                                                                             
/*
 * Note: this is wrong for 2100, but our signed 32-bit time_t will
 * have overflowed long before that, so who cares.  -- paulus
 */
#define leapyear(year)          ((year) % 4 == 0)
#define days_in_year(a)         (leapyear(a) ? 366 : 365)
#define days_in_month(a)        (month_days[(a) - 1])
                                                                                                                             
static int month_days[12] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};
                                                                                                                             
void to_tm(int tim, MV_RTC_TIME *tm)
{
        register int i;
        register long hms, day, gday;
 
        gday = day = tim / SECDAY;
        hms = tim % SECDAY;
        /* Hours, minutes, seconds are easy */
        tm->hours = hms / 3600;
        tm->minutes = (hms % 3600) / 60;
        tm->seconds = (hms % 3600) % 60;
 
        /* Number of years in days */
        for (i = STARTOFTIME; day >= days_in_year(i); i++)
                day -= days_in_year(i);
        tm->year = i;
 
        /* Number of months in days left */
        if (leapyear(tm->year))
                days_in_month(FEBRUARY) = 29;
        for (i = 1; day >= days_in_month(i); i++)
                day -= days_in_month(i);
        days_in_month(FEBRUARY) = 28;
        tm->month = i;
 
        /* Days are what is left over (+1) from all that. */
        tm->date = day + 1;
 
        /*
         * Determine the day of week. Jan. 1, 1970 was a Thursday.
         */
        tm->day = (gday + 4) % 7;
}


static int mv_set_rtc(void)
{
	MV_RTC_TIME time;
	to_tm(xtime.tv_sec, &time);
	time.year -= 2000;
	mvRtcDS1339TimeSet(&time);

	return 1;
}


extern int (*set_rtc)(void);

static inline int mv_rtc_set_time(struct rtc_time *tm)
{
	MV_RTC_TIME time;
	unsigned long temp_t;

	rtc_tm_to_time(tm, &temp_t);
	to_tm(temp_t, &time);
	/* same as in the U-Boot we use the year for century 20 only */
	time.year -= 2000;
	mvRtcDS1339TimeSet(&time);

	return 0;
}

static int mv_rtc_read_time(struct rtc_time *tm)
{
	MV_RTC_TIME time;
	unsigned long temp_t;

	mvRtcDS1339TimeGet(&time);
	/* same as in the U-Boot we use the year for century 20 only */
	temp_t = mktime ( time.year + 2000, time.month,
        			time.date, time.hours,
        			time.minutes, time.seconds);
	rtc_time_to_tm(temp_t, tm);
	
	return 0;
}

static struct rtc_ops rtc_ops = {
        .owner          = THIS_MODULE,
        .read_time      = mv_rtc_read_time,
        .set_time       = mv_rtc_set_time,
};


static int mv_rtc_init(void)
{
	MV_RTC_TIME time;
	mvRtcDS1339TimeGet(&time);

	/* same as in the U-Boot we use the year for century 20 only */
	xtime.tv_sec = mktime ( time.year + 2000, time.month,
        			time.date, time.hours,
        			time.minutes, time.seconds);

	to_tm(xtime.tv_sec, &time);
 
	set_rtc = mv_set_rtc;

	register_rtc(&rtc_ops);

	return 0;
}

__initcall(mv_rtc_init);
