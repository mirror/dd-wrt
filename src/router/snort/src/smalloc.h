/* $Id$ */
/*
** Copyright (C) 2002-2011 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

/*
 * Snort frontend to malloc
 */

#ifndef __SMALLOC_H__
#define __SMALLOC_H__


#include <stdlib.h>

#include "debug.h"



/* MALLOC flags */

#define	M_EXIT		1	/* exit if memory allocation fails (default) */
#define	M_DONTEXIT	2	/* don't exit if memory allocation fails */
#define	M_ZERO		4	/* zero out the allocated memory */


#define	MALLOC(ptr, cast, size, flags)					\
do {									\
	(ptr) = (cast) malloc((size));					\
	if (!((flags) & M_DONTEXIT) && ((ptr)  == NULL))		\
	{								\
		DebugMessage(DEBUG_ALL, "malloc: out of memory (allocating %d bytes)\n", (size));							\
		exit(1);						\
	}								\
	if (((flags) & M_ZERO) && ((ptr) != NULL))			\
		memset((ptr), '\0', (size));				\
} while (0)


#define	FREE(ptr)							\
do {									\
	if ((ptr) == NULL)						\
	{								\
		DebugMessage(DEBUG_ALL, "free: NULL pointer given as an argument\n");									\
		exit(1);						\
	}								\
	free((ptr));							\
	ptr = NULL;							\
} while(0)



#endif	/* __SMALLOC_H__ */
