/*====================================================================*
 *
 *   FILE *efreopen(char const *filename, char const *openmode, FILE *fp);
 *
 *   files.h
 *
 *   attempt to reopen a file using freopen(); return a valid file
 *   pointer on success; print an error message to stderr and then
 *   return NULL on failure;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef EFREOPEN_SOURCE
#define EFREOPEN_SOURCE

#include <stdio.h>
#include <errno.h>

#include "../tools/files.h"
#include "../tools/error.h"

FILE *efreopen (char const *filename, char const *openmode, FILE *fp)

{
	if ((fp = freopen (filename, openmode, fp)) == (FILE *)(0))
	{
		error (0, errno, "%s", filename);
	}
	return (fp);
}


#endif

