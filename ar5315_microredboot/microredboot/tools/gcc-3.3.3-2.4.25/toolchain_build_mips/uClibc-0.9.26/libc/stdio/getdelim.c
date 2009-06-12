/* vi: set sw=4 ts=4: */
/* getdelim for uClibc
 *
 * Copyright (C) 2000 by Lineo, inc. and Erik Andersen
 * Copyright (C) 2000,2001 by Erik Andersen <andersen@uclibc.org>
 * Written by Erik Andersen <andersen@uclibc.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>


/* Read up to (and including) a TERMINATOR from STREAM into *LINEPTR
   (and null-terminate it). *LINEPTR is a pointer returned from malloc (or
   NULL), pointing to *N characters of space.  It is realloc'd as
   necessary.  Returns the number of characters read (not including the
   null delimiter), or -1 on error or EOF.  */
ssize_t getdelim(char **linebuf, size_t *linebufsz, int delimiter, FILE *file)
{
	static const int GROWBY = 80; /* how large we will grow strings by */

	int ch;
	int idx = 0;

	if (file == NULL || linebuf==NULL || linebufsz == NULL) {
	    __set_errno(EINVAL);
	    return -1;
	}

	if (*linebuf == NULL || *linebufsz < 2) {
		*linebuf = malloc(GROWBY);
		if (!*linebuf) {
			__set_errno(ENOMEM);
			return -1;
		}
		*linebufsz += GROWBY;
	}

	while (1) {
		ch = fgetc(file);
		if (ch == EOF)
			break;
		/* grow the line buffer as necessary */
		while (idx > *linebufsz-2) {
			*linebuf = realloc(*linebuf, *linebufsz += GROWBY);
			if (!*linebuf) {
				__set_errno(ENOMEM);
				return -1;
			}
		}
		(*linebuf)[idx++] = (char)ch;
		if ((char)ch == delimiter)
			break;
	}

	if (idx != 0)
	    (*linebuf)[idx] = 0;
	else if ( ch == EOF )
		return -1;
	return idx;
}

