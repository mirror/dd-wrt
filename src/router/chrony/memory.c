/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2014, 2017
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * 
 **********************************************************************

  =======================================================================

  Utility functions for memory allocation.

  */

#include "config.h"

#include "logging.h"
#include "memory.h"

void *
Malloc(size_t size)
{
  void *r;

  r = malloc(size);
  if (!r && size)
    LOG_FATAL("Could not allocate memory");

  return r;
}

void *
Realloc(void *ptr, size_t size)
{
  void *r;

  r = realloc(ptr, size);
  if (!r && size)
    LOG_FATAL("Could not allocate memory");

  return r;
}

static size_t
get_array_size(size_t nmemb, size_t size)
{
  size_t array_size;

  array_size = nmemb * size;

  /* Check for overflow */
  if (nmemb > 0 && array_size / nmemb != size)
    LOG_FATAL("Could not allocate memory");

  return array_size;
}

void *
Malloc2(size_t nmemb, size_t size)
{
  return Malloc(get_array_size(nmemb, size));
}

void *
Realloc2(void *ptr, size_t nmemb, size_t size)
{
  return Realloc(ptr, get_array_size(nmemb, size));
}

char *
Strdup(const char *s)
{
  void *r;

  r = strdup(s);
  if (!r)
    LOG_FATAL("Could not allocate memory");

  return r;
}
