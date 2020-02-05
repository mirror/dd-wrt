/*
  chronyd/chronyc - Programs for keeping computer clocks accurate.

 **********************************************************************
 * Copyright (C) Miroslav Lichvar  2014
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

  Functions implementing an array with automatic memory allocation.

  */

#include "config.h"

#include "sysincl.h"

#include "array.h"
#include "memory.h"

struct ARR_Instance_Record {
  void *data;
  unsigned int elem_size;
  unsigned int used;
  unsigned int allocated;
};

ARR_Instance
ARR_CreateInstance(unsigned int elem_size)
{
  ARR_Instance array;

  assert(elem_size > 0);

  array = MallocNew(struct ARR_Instance_Record);

  array->data = NULL;
  array->elem_size = elem_size;
  array->used = 0;
  array->allocated = 0;

  return array;
}

void
ARR_DestroyInstance(ARR_Instance array)
{
  Free(array->data);
  Free(array);
}

static void
realloc_array(ARR_Instance array, unsigned int min_size)
{
  assert(min_size <= 2 * min_size);
  if (array->allocated >= min_size && array->allocated <= 2 * min_size)
    return;

  if (array->allocated < min_size) {
    while (array->allocated < min_size)
      array->allocated = array->allocated ? 2 * array->allocated : 1;
  } else {
    array->allocated = min_size;
  }

  array->data = Realloc2(array->data, array->allocated, array->elem_size);
}

void *
ARR_GetNewElement(ARR_Instance array)
{
  array->used++;
  realloc_array(array, array->used);
  return ARR_GetElement(array, array->used - 1);
}

void *
ARR_GetElement(ARR_Instance array, unsigned int index)
{
  assert(index < array->used);
  return (void *)((char *)array->data + (size_t)index * array->elem_size);
}

void *
ARR_GetElements(ARR_Instance array)
{
  /* Return a non-NULL pointer when the array has zero size */
  if (!array->data) {
    assert(!array->used);
    return array;
  }

  return array->data;
}

void
ARR_AppendElement(ARR_Instance array, void *element)
{
  void *e;

  e = ARR_GetNewElement(array);
  memcpy(e, element, array->elem_size);
}

void
ARR_SetSize(ARR_Instance array, unsigned int size)
{
  realloc_array(array, size);
  array->used = size;
}

unsigned int
ARR_GetSize(ARR_Instance array)
{
  return array->used;
}
