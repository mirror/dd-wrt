/* time related system calls */
/*
Copyright (C) 2004-2009 John E. Davis

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

static char *ctime_cmd (long *tt)
{
   char *t;

   t = ctime ((time_t *) tt);
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

static long _time_cmd (void)
{
   return (long) time (NULL);
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

static void localtime_cmd (long *t)
{
   time_t tt = (time_t) *t;
   (void) push_tm_struct (localtime (&tt));
}
   
static void gmtime_cmd (long *t)
{
#ifdef HAVE_GMTIME
   time_t tt = (time_t) *t;
   (void) push_tm_struct (gmtime (&tt));
#else
   localtime_cmd (t);
#endif
}

#ifdef HAVE_MKTIME
static long mktime_cmd (void)
{
   struct tm t;

   if (-1 == SLang_pop_cstruct (&t, TM_Struct))
     return (long)-1;
   
   return (long) mktime (&t);
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
	tms = *localtime(&t);
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
   MAKE_INTRINSIC_1("ctime", ctime_cmd, SLANG_STRING_TYPE, SLANG_LONG_TYPE),
   MAKE_INTRINSIC_0("sleep", sleep_cmd, SLANG_VOID_TYPE),
   MAKE_INTRINSIC_0("_time", _time_cmd, SLANG_LONG_TYPE),
   MAKE_INTRINSIC_0("time", SLcurrent_time_string, SLANG_STRING_TYPE),
   MAKE_INTRINSIC_1("localtime", localtime_cmd, SLANG_VOID_TYPE, SLANG_LONG_TYPE),
   MAKE_INTRINSIC_1("gmtime", gmtime_cmd, SLANG_VOID_TYPE, SLANG_LONG_TYPE),
#ifdef HAVE_MKTIME
   MAKE_INTRINSIC_0("mktime", mktime_cmd, SLANG_LONG_TYPE),
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
#ifdef HAVE_TIMES
   (void) tic_cmd ();
#endif
   return SLadd_intrin_fun_table (Time_Funs_Table, NULL);
}

