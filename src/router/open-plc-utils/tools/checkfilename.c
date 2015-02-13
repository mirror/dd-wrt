/*====================================================================*
 *
 *   bool checkfilename (char const * pathname);
 *
 *   files.h
 *
 *   confirm that the filename portion of a pathname string contains
 *   only letters, digits, periods, underscores and hyphens;
 *
 *   this prevents users from entering an Ethernet address where a
 *   filename should appear on the command line; Ethernet addresses
 *   are also valid filenames;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef CHECKFILENAME_SOURCE
#define CHECKFILENAME_SOURCE

#include <string.h>
#include <ctype.h>

#include "../tools/files.h"

bool checkfilename (char const * pathname)

{
	char const * filename = pathname;
	while (*pathname)
	{
		if ((*pathname == '/') || (*pathname == '\\'))
		{
			filename = pathname + 1;
		}
		pathname++;
	}
	while (isalnum (*filename) || (*filename == '.') || (*filename == '_') || (*filename == '-'))
	{
		filename++;
	}
	return (*filename == (char) (0));
}


#endif

