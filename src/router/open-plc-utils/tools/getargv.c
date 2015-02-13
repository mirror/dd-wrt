/*====================================================================*
 *
 *   Copyright (c) 2012 Qualcomm Atheros Inc.
 *
 *   Permission to use, copy, modify, and/or distribute this software
 *   for any purpose with or without fee is hereby granted, provided
 *   that the above copyright notice and this permission notice appear
 *   in all copies.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 *   WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL
 *   THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
 *   CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 *   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 *   NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 *   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *
 *   signed getargv (signed argc, char const * argv [])
 *
 *   symbol.h
 *
 *   read one line from stdin; fill argv [] with fields from that
 *   line; return the number of fields found; ignore blank lines
 *   and script style comment lines; this implementation inserts
 *   a program name at argv [0] to emulate a true argv [];
 *
 *--------------------------------------------------------------------*/

#ifndef GETARGV_SOURCE
#define GETARGV_SOURCE

#include <stdio.h>
#include <ctype.h>
#include <memory.h>

#include "../tools/symbol.h"
#include "../tools/chars.h"

signed getargv (signed argc, char const * argv [])

{
	extern char const * program_name;
	static char string [1024];
	char const ** argp = argv;
	char * sp = string;
	signed c = getc (stdin);
	memset (string, 0, sizeof (string));
	memset ((char **)(argv), 0, argc * sizeof (char const *));
	while (nobreak (c))
	{
		if (isspace (c))
		{
			do
			{
				c = getc (stdin);
			}
			while (isspace (c));
		}
		if (c == '#')
		{
			do
			{
				c = getc (stdin);
			}
			while (nobreak (c));
			c = getc (stdin);
			continue;
		}
		*argp++ = program_name;
		*argp++ = sp = string;
		while (nobreak (c))
		{
			if (c == '#')
			{
				do
				{
					c = getc (stdin);
				}
				while (nobreak (c));
				break;
			}
			if (isblank (c))
			{
				c = (char)(0);
				*argp = sp + 1;
			}
			else if (sp == *argp)
			{
				if ((signed)(argp - argv) < argc)
				{
					argp++;
				}
			}
			*sp++ = (char)(c);
			c = getc (stdin);
		}
		*argp = (char const *)(0);
		*sp = (char)(0);
	}
	return ((unsigned)(argp - argv));
}

#endif

