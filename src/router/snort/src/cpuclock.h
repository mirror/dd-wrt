/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2006-2013 Sourcefire, Inc.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef CPU_CLOCK_TICKS_H
#define CPU_CLOCK_TICKS_H

/* Assembly to find clock ticks. */
#ifdef WIN32
#include <windows.h>

/* INTEL WINDOWS */
__inline void __cputicks_msc(uint64_t *val)
{
  __int64 t;
  __asm
    {
      rdtsc;
      mov dword PTR [t],eax;
      mov dword PTR [t+4],edx;
    }
 *val = (uint64_t)t;
}
#define get_clockticks(val) __cputicks_msc(&val)

/*
#define get_clockticks(val) \
    QueryPerformanceCounter((PLARGE_INTEGER)&val)
*/


#else
#include <unistd.h>

/* INTEL LINUX/BSD/.. */
#if (defined(__i386) || defined(__amd64) || defined(__x86_64__))
#define get_clockticks(val) \
{ \
    uint32_t a, d; \
    __asm__ __volatile__ ("rdtsc" : "=a" (a), "=d" (d));  \
    val = ((uint64_t)a) | (((uint64_t)d) << 32);  \
}
#else
#if (defined(__ia64) && defined(__GNUC__) )
#define get_clockticks(val) \
{ \
    __asm__ __volatile__ ("mov %0=ar.itc" : "=r"(val)); \
}
#else
#if (defined(__ia64) && defined(__hpux))
#include <machine/sys/inline.h>
#define get_clockticks(val) \
{ \
    val = _Asm_mov_from_ar (_AREG_ITC); \
}
#else
/* POWER PC */
#if (defined(__GNUC__) && (defined(__powerpc__) || (defined(__ppc__))))
#define get_clockticks(val) \
{ \
    uint32_t tbu0, tbu1, tbl; \
    do \
    { \
        __asm__ __volatile__ ("mftbu %0" : "=r"(tbu0)); \
        __asm__ __volatile__ ("mftb %0" : "=r"(tbl)); \
        __asm__ __volatile__ ("mftbu %0" : "=r"(tbu1)); \
    } while (tbu0 != tbu1); \
    val = ((uint64_t)tbl) | (((uint64_t)tbu0) << 32);  \
}
#else
/* SPARC */
#ifdef SPARCV9
#ifdef _LP64
#define get_clockticks(val) \
{ \
    __asm__ __volatile__("rd %%tick, %0" : "=r"(val)); \
}
#else
#define get_clockticks(val) \
{ \
    uint32_t a, b; \
    __asm__ __volatile__("rd %%tick, %0\n" \
                         "srlx %0, 32, %1" \
                         : "=r"(a), "=r"(b)); \
    val = ((uint64_t)a) | (((uint64_t)b) << 32); \
}
#endif /* _LP64 */
#else
#define get_clockticks(val)
#endif /* SPARC */
#endif /* POWERPC || PPC */
#endif /* IA64 && HPUX */
#endif /* IA64 && GNUC */
#endif /* I386 || AMD64 || X86_64 */
#endif /* WIN32 */

static inline double get_ticks_per_usec (void)
{
    uint64_t start = 0, end = 0;
    get_clockticks(start);

#ifdef WIN32
    Sleep(1000);
#else
    sleep(1);
#endif
    get_clockticks(end);

    return (double)(end-start)/1e6;
}


#endif /* CPU_CLOCK_TICKS_H */
