/*====================================================================*
 *
 *   gettimeofday.c - get time of day for Windows;
 *
 *   A gettimeofday implementation for Microsoft Windows;
 *
 *   Public domain code, author "ponnada";
 *
 *--------------------------------------------------------------------*/

#ifndef GETTIMEOFDAY_SOURCE
#define GETTIMEOFDAY_SOURCE

#include <time.h>
#include <windows.h>
#include <sys/time.h>

int gettimeofday (struct timeval *tv, struct timezone *tz)

{
	FILETIME ft;
	unsigned __int64 tmpres = 0;
	static int tzflag = 0;
	if (NULL != tv)
	{
		GetSystemTimeAsFileTime (&ft);
		tmpres |= ft.dwHighDateTime;
		tmpres <<= 32;
		tmpres |= ft.dwLowDateTime;
		tmpres /= 10;
		tmpres -= DELTA_EPOCH_IN_MICROSECS;
		tv->tv_sec = (long)(tmpres / 1000000UL);
		tv->tv_usec = (long)(tmpres % 1000000UL);
	}
	if (NULL != tz)
	{
		if (!tzflag)
		{
			_tzset ();
			tzflag++;
		}
		tz->tz_minuteswest = _timezone / 60;
		tz->tz_dsttime = _daylight;
	}
	return 0;
}


#endif

