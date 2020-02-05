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

  Header file for array functions.
  */

#ifndef GOT_ARRAY_H
#define GOT_ARRAY_H

typedef struct ARR_Instance_Record *ARR_Instance;

/* Create a new array with given element size */
extern ARR_Instance ARR_CreateInstance(unsigned int elem_size);

/* Destroy the array */
extern void ARR_DestroyInstance(ARR_Instance array);

/* Return pointer to a new element added to the end of the array */
extern void *ARR_GetNewElement(ARR_Instance array);

/* Return element with given index */
extern void *ARR_GetElement(ARR_Instance array, unsigned int index);

/* Return pointer to the internal array of elements */
extern void *ARR_GetElements(ARR_Instance array);

/* Add a new element to the end of the array */
extern void ARR_AppendElement(ARR_Instance array, void *element);

/* Set the size of the array */
extern void ARR_SetSize(ARR_Instance array, unsigned int size);

/* Return current size of the array */
extern unsigned int ARR_GetSize(ARR_Instance array);

#endif
