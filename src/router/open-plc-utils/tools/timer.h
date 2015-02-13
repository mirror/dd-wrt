/*====================================================================*
 *
 *   timer.h - custom data type definitions and declarations;
 *
 *   this file is a subset of the original that includes only those
 *   definitions and declaration needed for toolkit programs;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef TIMER_HEADER
#define TIMER_HEADER

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdint.h>

/*====================================================================*
 *   constants;
 *--------------------------------------------------------------------*/

#ifdef WIN32
#define SLEEP(n) Sleep(n)
#else
#define SLEEP(n) usleep((n)*1000)
#endif

/*====================================================================*
 *   macros;
 *--------------------------------------------------------------------*/

#define MILLISECONDS(start,timer) ((((timer).tv_sec - (start).tv_sec) * 1000) + ((timer).tv_usec - (start).tv_usec) / 1000)
#define SECONDS(start,timer) (MILLISECONDS(start,timer) / 1000)

/*====================================================================*
 *   end definitions and declarations;
 *--------------------------------------------------------------------*/

#endif

