/* src/toolbox/util.c - contains some utility functions

   Copyright (C) 1996-2005, 2006, 2008
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/


#include "config.h"

#include <assert.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>

#include "vm/types.hpp"

#include "vm/vm.hpp"


/* get_variable_message_length *************************************************

   This function simluates the print of a variable message and
   determines so the message length;

*******************************************************************************/

int get_variable_message_length(const char *fmt, va_list ap)
{
	int n;

	n = vsnprintf(NULL, 0, fmt, ap);

#if defined(__IRIX__)
	/* We know that IRIX returns -1 if the buffer is NULL */

	if (n == -1) {
		char *p, *np;
		s4    size;

		size = 100;                     /* start with 100-bytes               */

		p = MNEW(char, size);

		while (1) {
			/* Try to print in the allocated space. */

			n = vsnprintf(p, size, fmt, ap);

			/* If that worked, return the length. */
			if (n > -1 && n < size)
				return n;

			/* Else try again with more space. */
			size *= 2;  /* twice the old size */

			if ((np = MREALLOC(p, char, size, size)) == NULL) {
				assert(0);
			} else {
				p = np;
			}
		}
	}
#endif

	return n;
}


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 */
