/*====================================================================*
 *
 *   unsigned todigit (unsigned c);
 *
 *   number.h
 *
 *   return the unsigned integer equivalent of an ASCII digit or the
 *   value UCHAR_MAX on error;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef TODIGIT_SOURCE
#define TODIGIT_SOURCE

#include <limits.h>

#include "../tools/number.h"

unsigned todigit (unsigned c)

{
	if ((c >= '0') && (c <= '9'))
	{
		return (c - '0');
	}
	if ((c >= 'A') && (c <= 'Z'))
	{
		return (c - 'A' + 10);
	}
	if ((c >= 'a') && (c <= 'z'))
	{
		return (c - 'a' + 10);
	}
	return (UCHAR_MAX);
}


#endif

