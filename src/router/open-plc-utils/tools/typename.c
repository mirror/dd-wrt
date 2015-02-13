/*====================================================================*
 *
 *   char const * typename (struct _type_ const list [], size_t size, type_t type, char const * name);
 *
 *   symbol.h
 *
 *   return the name associated with a given type by searching a name
 *   table arranged in ascending order by type; return the table name
 *   if the type is found or argument name if not;
 *
 *   typedefs type_t and struct _type_ are defined in types.h;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef TYPENAME_SOURCE
#define TYPENAME_SOURCE

#include <unistd.h>

#include "../tools/symbol.h"

char const * typename (struct _type_ const list [], size_t size, type_t type, char const * name)

{
	size_t lower = 0;
	size_t upper = size;
	while (lower < upper)
	{
		size_t index = (lower + upper) >> 1;
		signed order = type - list [index].type;
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

