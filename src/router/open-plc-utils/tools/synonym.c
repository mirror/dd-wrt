/*====================================================================*
 *
 *   char const * synonym (char const * term, const struct _term_ list [], size_t size);
 *
 *   symbol.h
 *
 *   lookup term and return corresponding text; return the original
 *   term if lookup fails; the list must be in lexographic order by
 *   term or lookups may fail; struct _term_ is defined in types.h;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef SYNONYM_SOURCE
#define SYNONYM_SOURCE

#include <string.h>

#include "../tools/types.h"

char const * synonym (char const * term, const struct _term_ list [], size_t size)

{
	size_t lower = 0;
	size_t upper = size;
	while (lower < upper)
	{
		size_t index = (lower + upper) >> 1;
		signed order = strcmp (term, list [index].term);
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
		return (list [index].text);
	}
	return (term);
}


#endif

