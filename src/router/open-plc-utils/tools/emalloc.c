/*====================================================================*
 *
 *   void * emalloc (size_t length)
 *
 *   error.h
 *
 *   attempt to allocate memory using malloc(); return the memory address
 *   on success; print an error message on stderr and then terminate the
 *   program on failure;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef EMALLOC_SOURCE
#define EMALLOC_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "../tools/error.h"

void * emalloc (size_t length)

{
	void * memory = malloc (length);
	if (!memory)
	{
		error (1, errno, "need %lu bytes", (long)(length));
	}
	return (memory);
}


#endif

