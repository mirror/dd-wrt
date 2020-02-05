/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Richard P. Curnow  1997-2002
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

  Header file for memory functions
  */

#ifndef GOT_MEMORY_H
#define GOT_MEMORY_H

#include "sysincl.h"

/* Wrappers checking for errors */
extern void *Malloc(size_t size);
extern void *Realloc(void *ptr, size_t size);
extern void *Malloc2(size_t nmemb, size_t size);
extern void *Realloc2(void *ptr, size_t nmemb, size_t size);
extern char *Strdup(const char *s);

/* Convenient macros */
#define MallocNew(T) ((T *) Malloc(sizeof(T)))
#define MallocArray(T, n) ((T *) Malloc2(n, sizeof(T)))
#define ReallocArray(T, n, x) ((T *) Realloc2((void *)(x), n, sizeof(T)))
#define Free(x) free(x)

#endif /* GOT_MEMORY_H */
