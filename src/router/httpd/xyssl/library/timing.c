/*
 *  Portable interface to the CPU cycle counter
 *
 *  Copyright (C) 2003-2006  Christophe Devine, Brian Gladman
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License, version 2.1 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

#include "timing.h"

#ifdef WIN32

#include <windows.h>
#include <winbase.h>

struct _hr_time
{
    LARGE_INTEGER start;
};

#else

#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

struct _hr_time
{
    struct timeval start;
};

#endif

#if (defined(_MSC_VER) && defined(_M_IX86)) || defined(__WATCOMC__)

unsigned long hardclock( void )
{
    unsigned long tsc;
    __asm   rdtsc
    __asm   mov  [tsc], eax
    return( tsc );
}

#else /*#if (defined(_MSC_VER) && defined(_M_IX86)) || defined(__WATCOMC__)*/
#if defined(__GNUC__)
#if defined(__i386__) && defined(HAVE_RDTSC)

unsigned long hardclock( void )
{
    unsigned long tsc;
    asm( "rdtsc" : "=a" (tsc) );
    return( tsc );
}

#else /*#if defined(__i386__) && defined(HAVE_RDTSC)*/
#if defined(__amd64__) || defined(__x86_64__)

unsigned long hardclock( void )
{
    unsigned long lo, hi;
    asm( "rdtsc" : "=a" (lo), "=d" (hi) ); 
    return( lo | (hi << 32) );
}

#else /*#if defined(__amd64__) || defined(__x86_64__)*/
#if defined(__sparc__)

unsigned long hardclock( void )
{
    unsigned long tick;
    asm( ".byte 0x83, 0x41, 0x00, 0x00" );
    asm( "mov   %%g1, %0" : "=r" (tick) );
    return( tick );
}

#else /*#if defined(__sparc__)*/
#if defined(__alpha__)

unsigned long hardclock( void )
{
    unsigned long cc;
    asm( "rpcc %0" : "=r" (cc) );
    return( cc & 0xFFFFFFFF );
}

#else /*#if defined(__alpha__)*/
#if defined(__ia64__)

unsigned long hardclock( void )
{
    unsigned long itc;
    asm( "mov %0 = ar.itc" : "=r" (itc) );
    return( itc );
}

#else /*#if defined(__ia64__)*/
#if defined(__powerpc__) || defined(__ppc__)

unsigned long hardclock( void )
{
    unsigned long tbl, tbu0, tbu1;

    do
    {
        asm( "mftbu %0" : "=r" (tbu0) );
        asm( "mftb  %0" : "=r" (tbl ) );
        asm( "mftbu %0" : "=r" (tbu1) );
    }
    while( tbu0 != tbu1 );

    return( tbl );
}
#else



static int hardclock_init = 0;
static struct timeval tv_init;

unsigned long hardclock( void )
{
    struct timeval tv_cur;

    if( hardclock_init == 0 )
    {
        gettimeofday( &tv_init, NULL );
        hardclock_init = 1;
    }

    gettimeofday( &tv_cur, NULL );
    return( ( tv_cur.tv_sec  - tv_init.tv_sec  ) * 1000000
          + ( tv_cur.tv_usec - tv_init.tv_usec ) );
}
#endif /* PPC32 */
#endif /* IA64  */
#endif /* Alpha */
#endif /* SPARC */
#endif /* AMD64 */
#endif /* i386  */

#else

static int hardclock_init = 0;
static struct timeval tv_init;

unsigned long hardclock( void )
{
    struct timeval tv_cur;

    if( hardclock_init == 0 )
    {
        gettimeofday( &tv_init, NULL );
        hardclock_init = 1;
    }

    gettimeofday( &tv_cur, NULL );
    return( ( tv_cur.tv_sec  - tv_init.tv_sec  ) * 1000000
          + ( tv_cur.tv_usec - tv_init.tv_usec ) );
}

#endif /* GNUC */
#endif /* MSVC */

int alarmed = 0;

#ifdef WIN32

unsigned long set_timer( struct hr_time *val, int reset )
{
    unsigned long delta;
    LARGE_INTEGER offset, hfreq;
    struct _hr_time *t = (struct _hr_time *) val;

    QueryPerformanceCounter(  &offset );
    QueryPerformanceFrequency( &hfreq );

    delta = (unsigned long)( ( 1000 *
        ( offset.QuadPart - t->start.QuadPart ) ) /
           hfreq.QuadPart );

    if( reset )
        QueryPerformanceCounter( &t->start );

    return( delta );
}

DWORD WINAPI TimerProc( LPVOID uElapse )
{   
    Sleep( (DWORD) uElapse );
    alarmed = 1; 
    return( TRUE );
}

void set_alarm( int seconds )
{   
    DWORD ThreadId;

    alarmed = 0; 
    CloseHandle( CreateThread( NULL, 0, TimerProc,
        (LPVOID) ( seconds * 1000 ), 0, &ThreadId ) );
}

#else

unsigned long set_timer( struct hr_time *val, int reset )
{
    unsigned long delta;
    struct timeval offset;
    struct _hr_time *t = (struct _hr_time *) val;

    gettimeofday( &offset, NULL );

    delta = ( offset.tv_sec  - t->start.tv_sec  ) * 1000
          + ( offset.tv_usec - t->start.tv_usec ) / 1000;

    if( reset )
    {
        t->start.tv_sec  = offset.tv_sec;
        t->start.tv_usec = offset.tv_usec;
    }

    return( delta );
}

static void sighandler( int signum )
{   
    alarmed = 1;
    signal( signum, sighandler );
}

void set_alarm( int seconds )
{
    alarmed = 0;
    signal( SIGALRM, sighandler );
    alarm( seconds );
}

#endif
