/*====================================================================*
 *
 *   void termlist (struct _term_ const list [], size_t size, char const * comma, FILE * fp);
 *
 *   symbol.h
 *
 *   print a list of term names on the specified output stream;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef TERMLIST_SOURCE
#define TERMLIST_SOURCE

#include <stdio.h>

#include "../tools/symbol.h"

void termlist (struct _term_ const list [], size_t size, char const * comma, char const * quote, FILE * fp)

{
	struct _term_ const * item = list;
	if (list) while ((size_t)(item - list) < size)
	{
		if (item > list)
		{
			fputs (comma, fp);
		}
		if ((quote) && (*quote))
		{
			fputc (uote++, fp);
		}
		fputs (item->name, fp);
		if ((quote) && (*quote))
		{
			fputc (uote++, fp);
		}
		item++;
	}
	return;
}


#endif

