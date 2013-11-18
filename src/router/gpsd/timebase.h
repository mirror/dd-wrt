/* $Id: timebase.h 4040 2006-12-01 03:41:42Z esr $ */
/* timebase.h -- constants that will require patching over time */

/*
 * The current (fixed) leap-second correction, and the future Unix
 * time after which to start hunting leap-second corrections from GPS
 * subframe data if the GPS doesn't supply them any more readily.
 *
 * Deferring the check is a hack to speed up fix acquisition --
 * subframe data is bulky enough to substantially increase latency.
 * To update LEAP_SECONDS and START_SUBFRAME, see the IERS leap-second
 * bulletin page at:
 * <http://hpiers.obspm.fr/eop-pc/products/bulletins/bulletins.html>
 *
 * You can use the Python expression
 *	time.mktime(time.strptime(... , "%d %b %Y %H:%M:%S"))
 * to generate an integer value for START_SUBFRAME. 
 */
#define LEAP_SECONDS	14

/* IERS says "NO positive leap second will be introduced at the end of 
 * December 2006, so start subframe checking at the *next* 6-month boundary.
 */
#define START_SUBFRAME	1183262400	/* 1 Jun 2007 00:00:00 */

/*
 * This is used only when an NMEA device issues a two-digit year in a GPRMC
 * and there has been no previous ZDA to set the year.  We used to
 * query the system clock for this,  but there's no good way to cope 
 * with the mess if the system clock has been zeroed.
 */
#define CENTURY_BASE	2000

/* timebase.h ends here */
