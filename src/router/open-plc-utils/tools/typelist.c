/* ===================================================================*
 *
 *   void typelist (struct _type_ const list [], size_t size, char const * comma, FILE * fp);
 *
 *   symbol.h
 *
 *   print a list of type names on the specified output stream;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef TYPELIST_SOURCE
#define TYPELIST_SOURCE

#include <stdio.h>

#include "../tools/symbol.h"

void typelist (struct _type_ const list [], size_t size, char const * comma, char const * quote, FILE * fp)

{
	struct _type_ const * item = list;
	if (list) while ((size_t)(item - list) < size)
	{
		if (item > list)
		{
			fputs (comma, fp);
		}
		if ((quote) && (*quote))
		{
			fputc (*quote++, fp);
		}
		fputs (item->name, fp);
		if ((quote) && (*quote))
		{
			fputc (*quote++, fp);
		}
		item++;
	}
	return;
}


#endif

