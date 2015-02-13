/*====================================================================*
 *
 *   size_t hexload (void * memory, size_t extent, FILE * fp);
 *
 *   memory.h
 *
 *   read a file and convert hexadecimal octets to binary bytes then
 *   store them in consecutive memory locations up to a given length;
 *   return the actual number of bytes stored;
 *
 *   digits may be consecutive or separated by white space or comment
 *   text; a colon terminates a frame, to allow multiple frames in a
 *   on one file;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef HEXLOAD_SOURCE
#define HEXLOAD_SOURCE

#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <errno.h>

#include "../tools/memory.h"
#include "../tools/error.h"
#include "../tools/chars.h"

/*====================================================================*
 *   private variables;
 *--------------------------------------------------------------------*/

static unsigned row = 1;
static unsigned col = 1;

/*====================================================================*
 *
 *   signed fpgetc (FILE * fp)
 *
 *   return the next input character after updating the file cursor
 *   position;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

static signed fpgetc (FILE * fp)

{
	extern unsigned row;
	extern unsigned col;
	signed c = getc (fp);
	if (c == '\n')
	{
		row++;
		col = 0;
	}
	else
	{
		col++;
	}
	return (c);
}


/*====================================================================*
 *
 *   size_t hexload (void * memory, size_t extent, FILE * fp);
 *
 *   memory.h
 *
 *   read a file and convert hexadecimal octets to binary bytes then
 *   store them in consecutive memory locations up to a given length;
 *   return the actual number of bytes stored;
 *
 *   digits may be consecutive or separated by white space or comment
 *   text; a colon terminates a frame, to allow multiple frames in a
 *   on one file;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

size_t hexload (void * memory, size_t extent, FILE * fp)

{
	extern unsigned row;
	extern unsigned col;
	byte * origin = (uint8_t *)(memory);
	byte * offset = (uint8_t *)(memory);
	unsigned digits = sizeof (* offset) << 1;
	unsigned digit = 0;
	signed c = EOF;
	while ((extent) && ((c = fpgetc (fp)) != EOF) && (c != ';'))
	{
		if (isspace (c))
		{
			continue;
		}
		if (c == '#')
		{
			do
			{
				c = fpgetc (fp);
			}
			while (nobreak (c));
			continue;
		}
		if (c == '/')
		{
			c = fpgetc (fp);
			if (c == '/')
			{
				do
				{
					c = fpgetc (fp);
				}
				while (nobreak (c));
				continue;
			}
			if (c == '*')
			{
				while ((c != '/') && (c != EOF))
				{
					while ((c != '*') && (c != EOF))
					{
						c = fpgetc (fp);
					}
					c = fpgetc (fp);
				}
				continue;
			}
			continue;
		}
		if ((c >= '0') && (c <= '9'))
		{
			*offset *= 16;
			*offset += c - '0';
			if (!(++digit % digits))
			{
				offset++;
				extent--;
			}
			continue;
		}
		if ((c >= 'A') && (c <= 'F'))
		{
			*offset *= 16;
			*offset += 10;
			*offset += c - 'A';
			if (!(++digit % digits))
			{
				offset++;
				extent--;
			}
			continue;
		}
		if ((c >= 'a') && (c <= 'f'))
		{
			*offset *= 16;
			*offset += 10;
			*offset += c - 'a';
			if (!(++digit % digits))
			{
				offset++;
				extent--;
			}
			continue;
		}

#if 1

		error (1, ENOTSUP, "Unexpected character '%c': row %d: col %d", c, row, col);

#else

		return ((size_t)(-1));

#endif

	}
	if (digit & 1)
	{

#if 1

		error (1, ENOTSUP, "Odd digit count (%d) in source", digit);

#else

		return ((size_t)(-1));

#endif

	}
	return (offset - origin);
}


#endif

