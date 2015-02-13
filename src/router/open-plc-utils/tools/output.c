/*====================================================================*
 *
 *   void output (signed indent, char const * format, ...);
 *
 *   format.h
 *
 *   print an indented and formatted string on stdout;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef OUTPUT_SOURCE
#define OUTPUT_SOURCE

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

#include "../tools/format.h"

#ifdef __GNUC__

__attribute__ ((format (printf, 2, 3)))

#endif

void output (signed indent, char const * format, ...)

{
	static char tab = '\t';
	static char end = '\n';
	while (indent-- > 0)
	{
		putc (tab, stdout);
	}
	if ((format) && (*format))
	{
		va_list arglist;
		va_start (arglist, format);
		vprintf (format, arglist);
		va_end (arglist);
	}
	putc (end, stdout);
	return;
}


#endif

