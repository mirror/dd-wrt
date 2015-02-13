/*====================================================================*
 *
 *   char const * codename (struct _code_ const list [], size_t size, code_t code, char const * name);
 *
 *   symbol.h
 *
 *   return the name associated with a given code by searching a name
 *   table arranged in ascending order by code; return the table name
 *   if the code is found or argument name if not;
 *
 *   typedefs code_t and struct _code_ are defined in codes.h;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef CODENAME_SOURCE
#define CODENAME_SOURCE

#include <unistd.h>

#include "../tools/symbol.h"

char const * codename (struct _code_ const list [], size_t size, code_t code, char const * name)

{
	size_t lower = 0;
	size_t upper = size;
	while (lower < upper)
	{
		size_t index = (lower + upper) >> 1;
		signed order = code - list [index].code;
		if (order < 0)
		{
			upper = index - 0;
			continue;
		}
		if (order > 0)
		{
			lower = index + 1;
			continue;
		}
		return (list [index].name);
	}
	return (name);
}


#endif

