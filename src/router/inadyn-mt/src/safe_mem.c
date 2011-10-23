/*
Copyright (C) 2008 Bryan Hoover (bhoover@wecs.com)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 3
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/*

	History:
		April 2008 - memory allocation wrapper
  
*/

#define MODULE_TAG "SAFEMEM: "

#ifndef EXCLUDE_CONFIG_H

#include "config.h"

#endif

#include <stdlib.h>

#include <stdio.h>
#include <ctype.h>
#include <memory.h>
#include "debug_if.h"


static void abort_prog()
{

	printf("Memory allocation error.  Aborting program.");


	abort();
}

/*abort on malloc fail*/
void *safe_malloc(unsigned int size)
{

	void *ret=NULL;


	if (!(size))

		return NULL;

	if (!(ret=malloc(size)))

		abort_prog();


	memset(ret,0,size);


	return ret;
}

/*abort on realloc fail*/
void *safe_realloc(void *src,unsigned int size)
{

	void *ret=NULL;


	if (!(src)) {

		return safe_malloc(size);
	}
	else {

		if (!(size))

			return NULL;

		if (!(ret=realloc(src,size)))

			abort_prog();
	}

	return ret;
}

/*abort on calloc fail*/
void *safe_calloc(unsigned int num,unsigned int size)
{

	void *ret=NULL;


	if (!(ret=calloc(num,size)))

		abort_prog();


	return ret;
}
