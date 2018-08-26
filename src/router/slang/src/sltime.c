/* time related system calls */
/*
Copyright (C) 2004-2011 John E. Davis

This file is part of the S-Lang Library.

The S-Lang Library is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The S-Lang Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
USA.
*/

#include "slinclud.h"

#include <sys/types.h>
#include <time.h>
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#if defined(__BORLANDC__)
# include <dos.h>
#endif
#if defined(__GO32__) || (defined(__WATCOMC__) && !defined(__QNX__))
# include <dos.h>
# include <bios.h>
#endif

#include <errno.h>

#include "slang.h"
#include "_slang.h"

#ifdef __WIN32__
#include <windows.h>
/* Sleep is defined badly in MSVC... */
# ifdef _MSC_VER
#  define sleep(n) _sleep((n)*1000)
# else
#  ifdef sleep
#   undef sleep
#  endif
#  define sleep(x) if(x)Sleep((x)*1000)
# endif
#endif

/* Turns on/off intrinsics that _may_ get added in the future */
#define USE_FROM_FUTURE		0

#if ((SIZEOF_LONG < 8) && defined(HAVE_LONG_LONG))
# define SLANG_TIME_T_TYPE SLANG_LLONG_TYPE
typedef long long _sltime_t;
# define PUSH_TIME_T SLang_push_long_long
# define POP_TIME_T SLang_pop_long_long
#else
# define SLANG_TIME_T_TYPE SLANG_LONG_TYPE
typedef long _sltime_t;
# define PUSH_TIME_T SLang_push_long
# define POP_TIME_T SLang_pop_long
#endif

static int pop_time_t (time_t *tt)
{
   _sltime_t t;
   if (-1 == POP_TIME_T(&t))
     return -1;
   *tt = (time_t)t;
   return 0;
}

static int push_time_t (time_t t)
{
   return PUSH_TIME_T ((_sltime_t) t);
}

#if defined(IBMPC_SYSTEM)
/* For other system (Unix and VMS), _pSLusleep is in sldisply.c */
int _pSLusleep (unsigned long s)
{
   sleep (s/1000000L);
   s = s % 1000000L;

# if defined(__WIN32__)
   Sleep (s/1000);
#else
# if defined(__IBMC__)
   DosSleep(s/1000);
# else
#  if defined(_MSC_VER)
   _sleep (s/1000);
#  endif
# endif
#endif
   return 0;
}
#endif

#if defined(__IBMC__) && !defined(_AIX)
/* sleep is not a standard function in VA3. */
unsigned int sleep (unsigned int seconds)
{
   DosSleep(1000L * ((long)seconds));
   return 0;
}
#endif

/* Older POSIX standards had a different interfaces for these.  Avoid them. */
#if !defined(_POSIX_C_SOURCE) || (_POSIX_C_SOURCE < 199506L)
# undef HAVE_CTIME_R
# undef HAVE_GMTIME_R
# undef HAVE_LOCALTIME_R
#endif

static char *ctime_cmd (void)
{
   char *t;
   time_t tt;
#ifdef HAVE_CTIME_R
   static char buf[64];
#endif
   if (-1 == pop_time_t (&tt))
     return NULL;
#ifdef HAVE_CTIME_R
   t = ctime_r (&tt, buf);
#else
   t = ctime (&tt);
#endif
   t[24] = 0;  /* knock off \n */
   return (t);
}

static void sleep_cmd (void)
{
   unsigned int secs;
#if SLANG_HAS_FLOAT
   unsigned long usecs;
   double x;

   if (-1 == SLang_pop_double (&x))
     return;

   if (x < 0.0)
     x = 0.0;
   secs = (unsigned int) x;
   sleep (secs);
   x -= (double) secs;
   usecs = (unsigned long) (1e6 * x);
   if (usecs > 0) _pSLusleep (usecs);
#else
   if (-1 == SLang_pop_uinteger (&secs))
     return;
   if (secs != 0) sleep (secs);
#endif
}

static void _time_cmd (void)
{
   (void) push_time_t (time(NULL));
}

#if defined(__GO32__)
static char *djgpp_current_time (void) /*{{{*/
{
   union REGS rg;
   unsigned int year;
   unsigned char month, day, weekday, hour, minute, sec;
   char days[] = "SunMonTueWedThuFriSat";
   char months[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
   static char the_date[26];

   rg.h.ah = 0x2A;
#ifndef __WATCOMC__
   int86(0x21, &rg, &rg);
   year = rg.x.cx & 0xFFFF;
#else
   int386(0x21, &rg, &rg);
   year = rg.x.ecx & 0xFFFF;
#endif

   month = 3 * (rg.h.dh - 1);
   day = rg.h.dl;
   weekday = 3 * rg.h.al;

   rg.h.ah = 0x2C;

#ifndef __WATCOMC__
   int86(0x21, &rg, &rg);
#else
   int386(0x21, &rg, &rg);
#endif

   hour = rg.h.ch;
   minute = rg.h.cl;
   sec = rg.h.dh;

   /* we want this form: Thu Apr 14 15:43:39 1994\n  */
   sprintf(the_date, "%.3s %.3s%3d %02d:%02d:%02d %d\n",
	   days + weekday, months + month,
	   day, hour, minute, sec, year);
   return the_date;
}

/*}}}*/

#endif

char *SLcurrent_time_string (void) /*{{{*/
{
   char *the_time;
#ifndef __GO32__
   time_t myclock;

   myclock = time((time_t *) 0);
   the_time = (char *) ctime(&myclock);
#else
   the_time = djgpp_current_time ();
#endif
   /* returns the form Sun Sep 16 01:03:52 1985\n\0 */
   the_time[24] = '\0';
   return(the_time);
}

/*}}}*/

static SLang_CStruct_Field_Type TM_Struct [] =
{
   MAKE_CSTRUCT_INT_FIELD(struct tm, tm_sec, "tm_sec", 0),
   MAKE_CSTRUCT_INT_FIELD(struct tm, tm_min, "tm_min", 0),
   MAKE_CSTRUCT_INT_FIELD(struct tm, tm_hour, "tm_hour", 0),
   MAKE_CSTRUCT_INT_FIELD(struct tm, tm_mday, "tm_mday", 0),
   MAKE_CSTRUCT_INT_FIELD(struct tm, tm_mon, "tm_mon", 0),
   MAKE_CSTRUCT_INT_FIELD(struct tm, tm_year, "tm_year", 0),
   MAKE_CSTRUCT_INT_FIELD(struct tm, tm_wday, "tm_wday", 0),
   MAKE_CSTRUCT_INT_FIELD(struct tm, tm_yday, "tm_yday", 0),
   MAKE_CSTRUCT_INT_FIELD(struct tm, tm_isdst, "tm_isdst", 0),
   SLANG_END_CSTRUCT_TABLE
};

static int validate_tm (struct tm *tms)
{
   if ((tms->tm_sec < 0) || (tms->tm_sec > 61)   /* allows for leap-seconds */
       || (tms->tm_min < 0) || (tms->tm_min > 59)
       || (tms->tm_hour < 0) || (tms->tm_hour > 23)
       || (tms->tm_mday < 1) || (tms->tm_mday > 31)
       || (tms->tm_mon < 0) || (tms->tm_mon > 11)
       || (tms->tm_wday < 0) || (tms->tm_wday > 6)
       || (tms->tm_yday < 0) || (tms->tm_yday > 365))
     {
	_pSLang_verror (SL_INVALID_PARM, "Time structure contains invalid values");
	return -1;
     }
   /* The man page specifies no range for is_dst.  Map it to -1,0,1 for safety */
   if (tms->tm_isdst < -1) tms->tm_isdst = -1;
   else if (tms->tm_isdst > 1) tms->tm_isdst = 1;
   return 0;
}

static int push_tm_struct (struct tm *tms)
{
   if (tms == NULL)
     return SLang_push_null ();
   return SLang_push_cstruct ((VOID_STAR) tms, TM_Struct);
}

static int pop_tm_struct (struct tm *tms)
{
   /* The memset is necessary because GLIBC has extra fields that may be
    * meaningful.  In fact, without it strftime will generate access errors
    */
   memset ((char *) tms, 0, sizeof (*tms));
   if (-1 == SLang_pop_cstruct (tms, TM_Struct))
     return -1;

   return validate_tm (tms);
}

#ifdef HAVE_GMTIME
static int call_gmtime (time_t t, struct tm *tmp)
{
   struct tm *_tmp;
# ifdef HAVE_GMTIME_R
   if (NULL != (_tmp = gmtime_r (&t, tmp)))
     return 0;
# else
   _tmp = gmtime (&t);
   if (_tmp != NULL)
     {
	*tmp = *_tmp;
	return 0;
     }
# endif
   SLang_verror (SL_RunTime_Error, "libc gmtime returned NULL");
   return -1;
}
#endif

static int call_localtime (time_t t, struct tm *tmp)
{
   struct tm *_tmp;
#ifdef HAVE_LOCALTIME_R
   if (NULL != (_tmp = localtime_r (&t, tmp)))
     return 0;
#else
   _tmp = localtime (&t);
   if (_tmp != NULL)
     {
	*tmp = *_tmp;
	return 0;
     }
#endif
   SLang_verror (SL_RunTime_Error, "libc localtime returned NULL");
   return -1;
}


static void localtime_cmd (void)
{
   time_t t;
   struct tm tm;

   if (-1 == pop_time_t (&t))
     return;
   if (0 == call_localtime (t, &tm))
     (void) push_tm_struct (&tm);
}

static void gmtime_cmd (void)
{
   time_t t;
   struct tm tm;

   if (-1 == pop_time_t (&t))
     return;

#ifdef HAVE_GMTIME
   if (0 == call_gmtime (t, &tm))
     (void) push_tm_struct (&tm);
#else
   if (0 == call_localtime (t, &tm))
     (void) push_tm_struct (&tm);
#endif
}

#ifdef HAVE_GMTIME
/* Implement gmtime using a binary search.  The linux manpage says to
 * play games with the TZ environment variable but that is too ugly for
 * my taste.
 */
static int tm_cmp (struct tm *a, struct tm *b)
{
   if (a->tm_year != b->tm_year)
     return a->tm_year - b->tm_year;
   if (a->tm_yday != b->tm_yday)
     return a->tm_yday - b->tm_yday;
   if (a->tm_hour - b->tm_hour)
     return a->tm_hour - b->tm_hour;
   return (a->tm_min - b->tm_min)*60 + (a->tm_sec - b->tm_sec);
}

static int timegm_internal (struct tm *tmp, time_t *tp)
{
   struct tm tm0;
   struct tm tm1;
   time_t t0, t1, t, dt;
   static time_t delta;
   static int delta_ok = 0;
   int diff;

   /* Construct t0 and t1 to bracket the answer.  First, guess a value close
    * to the correct answer.  Consider:
    *    tm_utc = gmtime(t)
    *    tm_loc = localtime(t)
    *    delta = tm_loc - tm_utc
    * Then we expect
    *    t = mktime (tm_loc) = timegm(tm_utc)
    *      = mktime (tm_utc + delta)
    *      = mktime (tm_utc) + delta
    * ==> timegm(tm) = mktime(tm) + delta
    * ==> tm = gmtime(mktime(tm+delta))
    *        = gmtime(mktime(tm)) + delta
    *    delta = tm - gmtime(mktime(tm))
    */
   if (delta_ok == 0)
     {
	/* localtime Jan 15, 2000 00:00:00 */
	memset ((char *)&tm1, 0, sizeof(tm0));
	tm1.tm_mday = 15;
	tm1.tm_year = 100;

	/* want mktime(tm1), but mktime will modify tm1, so use tm0 as a tmp */
	tm0 = tm1; t1 = mktime (&tm0);
	if (-1 == call_gmtime (t1, &tm0))
	  return -1;
	/* Compute delta = tm1 - tm0 */
	tm1.tm_hour += 24*(tm1.tm_mday - tm0.tm_mday);
	delta = (tm1.tm_hour - tm0.tm_hour)*3600
	  + (tm1.tm_min - tm0.tm_min)*60 + (tm1.tm_sec - tm0.tm_sec);
	delta_ok = 1;
     }

   tm0 = *tmp;			       /* do not allow mktime to mess with *tmp */
   t = mktime (&tm0) + delta;

   /* Find a lower bound */
   dt = 0;
   while (1)
     {
	int wrapped = 0;
	t0 = t - dt;
	while (t0 > t)/* handle wrapping */
	  {
	     wrapped = 1;
	     t0++;
	  }
	if (-1 == call_gmtime (t0, &tm0))
	  return -1;
	diff = tm_cmp (&tm0, tmp);
	if (diff == 0)
	  {
	     *tp = t0;
	     return 0;
	  }
	if (diff < 0)
	  break;

	if (wrapped)
	  {
	     SLang_verror (SL_Internal_Error, "timegm: Unable to find a lower limit");
	     return -1;
	  }

	dt = dt*2 + 1;
     }

   /* upper bound */
   dt = 1;
   while (1)
     {
	int wrapped = 0;
	t1 = t + dt;
	while (t1 < t)/* handle wrapping */
	  {
	     wrapped = 1;
	     t1--;
	  }
	if (-1 == call_gmtime (t1, &tm1))
	  return -1;
	diff = tm_cmp (&tm1, tmp);
	if (diff == 0)
	  {
	     *tp = t1;
	     return 0;
	  }
	if (diff>0)
	  break;

	if (wrapped)
	  {
	     SLang_verror (SL_Internal_Error, "timegm: Unable to find an upper limit");
	     return -1;
	  }
	dt = dt * 2;
     }

   do
     {
	struct tm tmx;
	t = t0 + (t1-t0)/2;
	if (-1 == call_gmtime (t, &tmx))
	  return -1;
	diff = tm_cmp (&tmx, tmp);
	if (diff == 0)
	  {
	     *tp = t;
	     return 0;
	  }
	if (diff < 0)
	  {
	     if (t0 == t)
	       break;
	     t0 = t; tm0 = tmx;
	  }
	else
	  {
	     t1 = t; tm1 = tmx;
	  }
     }
   while (t0 + 1 < t1);
   *tp = t1;
   return 0;
}

static void timegm_cmd (void)
{
   struct tm tm;
   time_t t;

   if (-1 == pop_tm_struct (&tm))
     return;
   if (-1 == timegm_internal (&tm, &t))
     return;
   (void) push_time_t (t);
}

#endif

#ifdef HAVE_MKTIME
static void mktime_cmd (void)
{
   struct tm tm;

   if (-1 == SLang_pop_cstruct (&tm, TM_Struct))
     return;

   (void) push_time_t (mktime (&tm));
}
#endif

#ifdef HAVE_TIMES

# ifdef HAVE_SYS_TIMES_H
#  include <sys/times.h>
# endif

#include <limits.h>

#ifdef CLK_TCK
# define SECS_PER_TICK (1.0/(double)CLK_TCK)
#else
# ifdef CLOCKS_PER_SEC
#  define SECS_PER_TICK (1.0/(double)CLOCKS_PER_SEC)
# else
#  define SECS_PER_TICK (1.0/60.0)
# endif
#endif

typedef struct
{
   double tms_utime;
   double tms_stime;
   double tms_cutime;
   double tms_cstime;
}
TMS_Type;

static SLang_CStruct_Field_Type TMS_Struct [] =
{
   MAKE_CSTRUCT_FIELD(TMS_Type, tms_utime, "tms_utime", SLANG_DOUBLE_TYPE, 0),
   MAKE_CSTRUCT_FIELD(TMS_Type, tms_stime, "tms_stime", SLANG_DOUBLE_TYPE, 0),
   MAKE_CSTRUCT_FIELD(TMS_Type, tms_cutime, "tms_cutime", SLANG_DOUBLE_TYPE, 0),
   MAKE_CSTRUCT_FIELD(TMS_Type, tms_cstime, "tms_cstime", SLANG_DOUBLE_TYPE, 0),
   SLANG_END_CSTRUCT_TABLE
};

static void times_cmd (void)
{
   TMS_Type d;
   struct tms t;

   (void) times (&t);

   d.tms_utime = SECS_PER_TICK * (double)t.tms_utime;
   d.tms_stime = SECS_PER_TICK * (double)t.tms_stime;
   d.tms_cutime = SECS_PER_TICK * (double)t.tms_cutime;
   d.tms_cstime = SECS_PER_TICK * (double)t.tms_cstime;
   (void) SLang_push_cstruct ((VOID_STAR)&d, TMS_Struct);
}

static struct tms Tic_TMS;

static void _tic_cmd (void)
{
   times (&Tic_TMS);
}

static double _toc_cmd (void)
{
   struct tms t;
   double d;

   (void) times (&t);

   d = ((t.tms_utime - Tic_TMS.tms_utime)
	+ (t.tms_stime - Tic_TMS.tms_stime)) * SECS_PER_TICK;
   Tic_TMS = t;
   return d;
}

#endif				       /* HAVE_TIMES */

#ifdef __WIN32__
static LARGE_INTEGER Start_Time, Frequency;
#else
# ifdef HAVE_GETTIMEOFDAY
static struct timeval Start_Time;
# else
static time_t Start_Time;
# endif
#endif

static void tic_cmd (void)
{
#ifdef __WIN32__
   QueryPerformanceCounter(&Start_Time);
   QueryPerformanceFrequency(&Frequency);
#else
# ifdef HAVE_GETTIMEOFDAY
   (void) gettimeofday (&Start_Time, NULL);
# else
   Start_Time = time (NULL);
# endif
#endif
}

static double toc_cmd (void)
{
#ifdef __WIN32__
   LARGE_INTEGER t;

   QueryPerformanceCounter(&t);
   return (t.QuadPart-Start_Time.QuadPart)/(double)Frequency.QuadPart;
#else
# ifdef HAVE_GETTIMEOFDAY
   struct timeval tv;
   (void) gettimeofday (&tv, NULL);
   return (double) tv.tv_sec - (double) Start_Time.tv_sec
     + ((double) tv.tv_usec - (double) Start_Time.tv_usec) * 1e-6;
# else
   return (double) time(NULL) - Start_Time;
# endif
#endif
}

#ifdef SLANG_HAS_FLOAT
# if USE_FROM_FUTURE
static double ftime_cmd (void)
{
#  ifdef HAVE_GETTIMEOFDAY
   struct timeval tv;
   (void) gettimeofday (&tv, NULL);
   return (double) tv.tv_sec + (double) tv.tv_usec*1e-6;
#  else
   return (double) _time_cmd ();
#  endif
}
# endif
#endif

static void strftime_cmd (void)
{
   /* Rather then using some sort of portable version of strftime, which would
    * miss the locale-specific features, just call the system routine.  However,
    * it cannot be called blindly because some versions (e.g., the the one from
    * c.snippets.org) do no input checking, and use code such as
    *
    *   static char *day[] = {"Sunday", "Monday", ..., "Saturday"};
    *   [...]
    *   switch (*f++)
    *     {
    *      case 'A' :
    *        r = day[t->tm_wday];
    *        break;
    *    [...]
    *
    * and lead to a SEGV if t->tm_wday is not in the range [0:6].
    */
   struct tm tms;
   char buf[4096];
   int status;
   char *fmt;

   if (SLang_Num_Function_Args == 1)
     {
	time_t t = time(NULL);
	if (-1 == call_localtime (t, &tms))
	  return;
	if (-1 == validate_tm (&tms))
	  return;
     }
   else if (-1 == pop_tm_struct (&tms))
     return;

   if (-1 == SLang_pop_slstring (&fmt))
     return;

   /* Ugh.  The man page says:
    *
    *  The strftime() function returns the number of characters placed in  the
    *  array  s,  not  including  the  terminating NUL character, provided the
    *  string, including the terminating NUL, fits.  Otherwise, it returns  0,
    *  and  the contents of the array is undefined.  (Thus at least since libc
    *  4.4.4; very old versions of libc, such as libc 4.4.1, would return  max
    *  if the array was too small.)
    *
    *  Note  that  the  return value 0 does not necessarily indicate an error;
    *  for example, in many locales %p yields an empty string.
    *
    * Was this too designed by committee?
    */
   status = strftime (buf, sizeof(buf), fmt, &tms);
   if (status == 0)
     buf[0] = 0;
   buf[sizeof(buf)-1] = 0;
   (void) SLang_push_string (buf);
   SLang_free_slstring (fmt);
}

static SLang_Intrin_Fun_Type Time_Funs_Table [] =
{
   MAKE_INTRINSIC_0("ctime", ctime_cmd, SLANG_STRING_TYPE),
   MAKE_INTRINSIC_0("sleep", sleep_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("_time", _time_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("time", SLcurrent_time_string, SLANG_STRING_TYPE),
   MAKE_INTRINSIC_0("localtime", localtime_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("gmtime", gmtime_cmd, SLANG_VOID_TYPE),
#ifdef HAVE_GMTIME
   MAKE_INTRINSIC_0("timegm", timegm_cmd, SLANG_VOID_TYPE),
#endif
#ifdef HAVE_MKTIME
   MAKE_INTRINSIC_0("mktime", mktime_cmd, SLANG_VOID_TYPE),
#endif
   MAKE_INTRINSIC_0("strftime", strftime_cmd, SLANG_VOID_TYPE),
#ifdef HAVE_TIMES
   MAKE_INTRINSIC_0("times", times_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("_tic", _tic_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("_toc", _toc_cmd, SLANG_DOUBLE_TYPE),
#endif
   MAKE_INTRINSIC_0("tic", tic_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("toc", toc_cmd, SLANG_DOUBLE_TYPE),
#if SLANG_HAS_FLOAT
# if USE_FROM_FUTURE
   MAKE_INTRINSIC_0("ftime", ftime_cmd, SLANG_DOUBLE_TYPE),
# endif
#endif
   SLANG_END_INTRIN_FUN_TABLE
};

int _pSLang_init_sltime (void)
{
   if (sizeof (time_t) > sizeof(_sltime_t))
     {
	SLang_exit_error ("S-Lang library not built properly.  Fix _sltime_t in sltime.c");
     }
#ifdef HAVE_TIMES
   (void) tic_cmd ();
#endif
   return SLadd_intrin_fun_table (Time_Funs_Table, NULL);
}

