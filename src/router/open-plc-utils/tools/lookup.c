/*====================================================================*
 *
 *   signed lookup (char const * name, struct _code_ const list [], size_t size);
 *
 *   symbol.h
 *
 *   search a name list and return the associated name; return the
 *   corresponding code on success or -1 on failure; the search is
 *   case insensitive;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef LOOKUP_SOURCE
#define LOOKUP_SOURCE

#include <unistd.h>
#include <string.h>

#include "../tools/symbol.h"

signed lookup (char const * name, struct _code_ const list [], size_t size)

{
	struct _code_ const * item = list;
	if ((name) && (*name)) while ((unsigned)(item - list) < size)
	{
		if (!strcasecmp (item->name, name))
		{
			return (item->code);
		}
		item++;
	}
	return (-1);
}


#endif

