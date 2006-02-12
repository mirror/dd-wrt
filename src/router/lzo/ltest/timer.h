/* timer.h -- high resolution timer

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-2002 Markus Franz Xaver Johannes Oberhumer
   All Rights Reserved.

   The LZO library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of
   the License, or (at your option) any later version.

   The LZO library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with the LZO library; see the file COPYING.
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer
   <markus@oberhumer.com>
 */



/*************************************************************************
//
**************************************************************************/

#if defined(TIME_WITH_SYS_TIME)
#  include <sys/time.h>
#  include <time.h>
#else
#  include <time.h>
#endif
#if !defined(CLOCKS_PER_SEC) && defined(CLK_TCK)
#  define CLOCKS_PER_SEC		CLK_TCK
#endif


/*************************************************************************
//
**************************************************************************/

#if 0 && defined(MAINT) && defined(__GNUC__) && defined(__i386__)

#  define my_clock_t		unsigned long long
#  define MY_CLOCKS_PER_SEC	(133*1024*1024.0)

#define RDTSC1(dest) \
	__asm__ __volatile__ \
	       (".byte 0x0F, 0x31\n" \
	        "movl %%eax,(%%edi)\n" \
	        "movl %%edx,4(%%edi)\n" \
	        "cld \n" \
	        "nop \n nop \n nop \n" \
	        "nop \n nop \n nop \n" \
	        "nop \n nop \n" \
	        : : "D" (dest) : "eax", "edx")

#define RDTSC2(dest) \
	__asm__ __volatile__ \
	       ("clc \n" \
	        ".byte 0x0F, 0x31\n" \
	        "movl %%eax,(%%edi)\n" \
	        "movl %%edx,4(%%edi)\n" \
	        : : "D" (dest) : "eax", "edx")


static my_clock_t my_clock(void)
{
	my_clock_t now;
	RDTSC2(&now);
	return now;
}


/*************************************************************************
//
**************************************************************************/

#elif 0 && defined(__DJGPP__)

#  include <dpmi.h>

extern int __bss_count;
static int wuclock_bss = -1;
static uclock_t wuclock_probe(void);
static uclock_t wuclock(void)
{
	static struct
	{
		unsigned short lo, hi;
	} vtd;   /* address of the Virtual Timer Device API entry point */
	static uclock_t base = 0;
	uclock_t rv;
	__dpmi_regs regs;

	if (wuclock_bss != __bss_count)
	{
		memset(&regs,0,sizeof(regs));
		regs.x.ax = 0x1684;
		regs.x.bx = 0x0005;
		regs.x.di = 0;
		regs.x.es = 0;
		if (__dpmi_simulate_real_mode_interrupt(0x2F,&regs))
			return -1; /* error: VTD API not available */

    	vtd.lo = regs.x.di;
    	vtd.hi = regs.x.es;
    	if (!(vtd.lo | vtd.hi))
			return -1; /* error: VTD API not available */

		base = 0;
		wuclock_bss = __bss_count;
	}

	memset(&regs,0,sizeof(regs));
	regs.x.ax = 0x0100;
	regs.x.cs = vtd.hi;
	regs.x.ip = vtd.lo;
	if (__dpmi_simulate_real_mode_procedure_retf(&regs))
		return -1; /* error: VTD API call failed */

	*((unsigned long *)&rv + 0) = regs.d.eax;
	*((unsigned long *)&rv + 1) = regs.d.edx;
	if (!base)
		base = rv;
	return rv - base;
}

static const char *my_clock_desc = "wuclock_probe()";
static uclock_t (*my_clock)(void) = wuclock_probe;
static uclock_t wuclock_probe(void)
{
	uclock_t r = wuclock();  /* test if VTD API is available */
	if (r == (uclock_t)-1)
	{
		my_clock_desc = "uclock()";
		my_clock = uclock;
		return uclock();
	}
	else
	{
		my_clock_desc = "wuclock()";
		my_clock = wuclock;
		return r;
	}
}

#  define my_clock_t		uclock_t
#  define MY_CLOCKS_PER_SEC	UCLOCKS_PER_SEC
#  define my_clock_desc		my_clock_desc


/*************************************************************************
//
**************************************************************************/

#elif 1 && defined(__DJGPP__)

#  define my_clock()		uclock()
#  define my_clock_t		uclock_t
#  define MY_CLOCKS_PER_SEC	UCLOCKS_PER_SEC
#  define my_clock_desc		"uclock()"


/*************************************************************************
// adapted from djgpp library source
**************************************************************************/

#elif 1 && defined(__WATCOMC__) && (UINT_MAX == LZO_0xffffffffL) && (defined(MSDOS) || defined(__DOS__))

#  include <dos.h>
#  include <i86.h>
#  include <conio.h>

#define uclock_t				unsigned long
#define _farpeekl(_seg,_off)	(* (unsigned long *) (_off))

static int __bss_count = 0;
static int uclock_bss = -1;

static uclock_t my_clock(void)
{
  static uclock_t base = 0;
  static unsigned long last_tics = 0;
  unsigned char lsb, msb;
  unsigned long tics, otics;
  uclock_t rv;

  if (uclock_bss != __bss_count)
  {
    /* switch the timer to mode 2 (rate generator) */
    /* rather than mode 3 (square wave), which doesn't count linearly. */
    outp(0x43, 0x34);
    outp(0x40, 0xff);
    outp(0x40, 0xff);
  }

  /* Make sure the numbers we get are consistent */
  do {
    otics = _farpeekl(_dos_ds, 0x46c);
    outp(0x43, 0x00);
    lsb = inp(0x40);
    msb = inp(0x40);
    tics = _farpeekl(_dos_ds, 0x46c);
  } while (otics != tics);

  /* calculate absolute time */
  msb ^= 0xff;
  lsb ^= 0xff;
  rv = ((uclock_t)tics << 16) | (((unsigned)msb << 8) | lsb);

  if (uclock_bss != __bss_count)
  {
    uclock_bss = __bss_count;
    base = rv;
    last_tics = 0;
  }

#if 0
  if (last_tics > tics) /* midnight happened */
    base -= 0x1800b00000LL;
#endif

  last_tics = tics;

  /* return relative time */
  return rv - base;
}

#undef _farpeekl

#  define my_clock_t		uclock_t
#  define MY_CLOCKS_PER_SEC	1193180ul
#  define my_clock_desc		"uclock()"


/*************************************************************************
//
**************************************************************************/

#elif 0 && defined(HAVE_SYS_RESOURCE_H) && defined(HAVE_GETRUSAGE)

#  include <sys/resource.h>

static double my_clock(void)
{
	struct rusage ru;
	double d1, d2;
	if (getrusage(RUSAGE_SELF,&ru) != 0)
		return 0;
	d1 = ru.ru_utime.tv_sec * 1000000.0 + ru.ru_utime.tv_usec;
	d2 = ru.ru_stime.tv_sec * 1000000.0 + ru.ru_stime.tv_usec;
	return d1 + d2;
}

#  define my_clock_t			double
#  define MY_CLOCKS_PER_SEC	    1000000
#  define my_clock_desc			"getrusage()"


/*************************************************************************
//
**************************************************************************/

#elif defined(TIME_WITH_SYS_TIME) && defined(HAVE_GETTIMEOFDAY)

static double my_clock(void)
{
	struct timeval tv;
	if (gettimeofday(&tv, 0) != 0)
		return 0;
	return tv.tv_sec * 1000000.0 + tv.tv_usec;
}

#  define my_clock_t			double
#  define MY_CLOCKS_PER_SEC	    1000000
#  define my_clock_desc			"gettimeofday()"


/*************************************************************************
//
**************************************************************************/

#else

#  define my_clock()		clock()
#  define my_clock_t		clock_t
#  define MY_CLOCKS_PER_SEC	CLOCKS_PER_SEC
#  define my_clock_desc		"clock()"
#endif


/*
vi:ts=4:et
*/

