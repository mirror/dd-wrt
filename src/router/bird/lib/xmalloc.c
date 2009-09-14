/*
 *	BIRD Library -- malloc() With Checking
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#include <stdlib.h>

#include "nest/bird.h"
#include "lib/resource.h"

#ifndef HAVE_LIBDMALLOC

/**
 * xmalloc - malloc with checking
 * @size: block size
 *
 * This function is equivalent to malloc() except that in case of
 * failure it calls die() to quit the program instead of returning
 * a %NULL pointer.
 *
 * Wherever possible, please use the memory resources instead.
 */
void *
xmalloc(unsigned size)
{
  void *p = malloc(size);
  if (p)
    return p;
  die("Unable to allocate %d bytes of memory", size);
}

/**
 * xrealloc - realloc with checking
 * @ptr: original memory block
 * @size: block size
 *
 * This function is equivalent to realloc() except that in case of
 * failure it calls die() to quit the program instead of returning
 * a %NULL pointer.
 *
 * Wherever possible, please use the memory resources instead.
 */
void *
xrealloc(void *ptr, unsigned size)
{
  void *p = realloc(ptr, size);
  if (p)
    return p;
  die("Unable to allocate %d bytes of memory", size);
}

#endif
