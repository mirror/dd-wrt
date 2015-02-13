/*====================================================================*
 *
 *   signed debug (signed status, char const * string, char const * format, ...);
 *
 *   error.h
 *
 *   variation of the GNU error() function that accepts a message in
 *   place of an error code and always returns -1;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef DEBUG_SOURCE
#define DEBUG_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "../tools/types.h"
#include "../tools/error.h"

#ifdef __GNUC__

__attribute__ ((format (printf, 3, 4)))

#endif

signed debug (signed status, char const * string, char const * format, ...)

{
	extern char const * program_name;
	if ((program_name) && (* program_name))
	{
		fprintf (stderr, "%s: ", program_name);
	}
	if ((string) && (* string))
	{
		fprintf (stderr, "%s: ", string);
	}
	if ((format) && (*format))
	{
		va_list arglist;
		va_start (arglist, format);
		vfprintf (stderr, format, arglist);
		va_end (arglist);
	}
	fprintf (stderr, "\n");
	fflush (stderr);
	if (status)
	{
		exit (status);
	}
	return (-1);
}


#endif

