/* src/toolbox/bitvector.c - bitvector implementation

   Copyright (C) 2005-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
   Copyright (C) 2008 Theobroma Systems Ltd.

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
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.

*/



#include "toolbox/bitvector.hpp"
#include <assert.h>
#include "config.h"
#include "mm/dumpmemory.hpp"

#if !defined(NDEBUG)

/// define BV_DEBUG_CHECK to activate the bound checks
// #define BV_DEBUG_CHECK

#endif

#if defined(BV_DEBUG_CHECK)
#define _BV_CHECK_BOUNDS(i,l,h) assert( ((i) >= (l)) && ((i) < (h)));
#define _BV_ASSERT(a)           assert((a));
#else
#define _BV_CHECK_BOUNDS(i,l,h);
#define _BV_ASSERT(a);
#endif

/******************************************************************************

Bitvector Implementation

******************************************************************************/

#ifdef BV_DEBUG_CHECK

  /* Number of ints needed for size bits */

# define BV_NUM_INTS(size) (((((size) + 7)/ 8) + sizeof(int) - 1)\
							/ sizeof(int) + 1)

  /* Get index in bitvector */

# define BV_INT_INDEX(bit) ( ((bit) / 8) / sizeof(int) + 1)

  /* Get bit index inside int */

# define BV_BIT_INDEX(bit, index) ( (bit) - (index - 1) * sizeof(int) * 8 );

#else

  /* Number of ints needed for size bits */

# define BV_NUM_INTS(size) (((((size) + 7)/ 8) + sizeof(int) - 1) / sizeof(int))

  /* Get index in bitvector */

# define BV_INT_INDEX(bit) ( ((bit) / 8) / sizeof(int) )

  /* Get bit index inside int */

# define BV_BIT_INDEX(bit, index) ( (bit) - (index) * sizeof(int) * 8 );

#endif

/************************************************************************
bv_to_string

Transforms the bitvector bv to a string of 1's and 0's.

IN: bitvector bv      bitvector created with bv_new()
    int       size    size of bitvector bv

IN/OUT:   char      *string allocated buffer, at least size + 1 elements

RETURN:pointer to string
******************************************************************************/
char *bv_to_string(bitvector bv, char *string, int size) {
	int i;

	_BV_ASSERT(bv[0] == size);

	for(i = 0; i < size; i++)
		if (bv_get_bit(bv, i))
			string[i]='1';
		else
			string[i]='0';

	string[i]=0;
	return string;
}

/******************************************************************************
bv_new

Creates a new bitvector and initializes all bits to 0.

IN: int       size    size of bitvector bv

RETURN: bitvector

*******************************************************************************/
bitvector bv_new(int size) {
	/* Number of ints needed for size bits */
    /* n = (((size+7)/8) + sizeof(int) - 1)/sizeof(int);  */
	int n = BV_NUM_INTS(size);

	int *bv = (int*) DumpMemory::allocate(sizeof(int) * n);

	for(int i = 0; i < n; i++) bv[i] = 0;

#ifdef BV_DEBUG_CHECK
	bv[0] = size;
#endif

	return bv;
}

/******************************************************************************
bv_get_bit

Checks if a specific bit of the bitvector is set.

IN:   bitvector bv
      int       bit    Index of bit to check (0..size(

RETURN:  bool      true if bit is set otherwise false
*******************************************************************************/
bool bv_get_bit(bitvector bv, int bit) {
	_BV_ASSERT(bit >= 0);

	int i = BV_INT_INDEX(bit);
	int n = BV_BIT_INDEX(bit, i);

	_BV_ASSERT(i < (BV_NUM_INTS(bv[0])));
	return (bv[i] & (1<<n));
}

/******************************************************************************
bv_set_bit

Sets a specific bit of the bitvector

IN:   bitvector bv
      int       bit    Index of bit to set (0..size(
*******************************************************************************/
void bv_set_bit(bitvector bv, int bit) {
	_BV_ASSERT(bit >= 0);

	int i = BV_INT_INDEX(bit);
	int n = BV_BIT_INDEX(bit, i);

	_BV_ASSERT(i < BV_NUM_INTS(bv[0]));

	bv[i] |= 1<<n;
}

/******************************************************************************
bv_reset_bit

Resets a specific bit of the bitvector

IN:   bitvector bv
      int       bit    Index of bit to reset (0..size(
*******************************************************************************/
void bv_reset_bit(bitvector bv, int bit) {
	_BV_ASSERT(bit >= 0);

	int i = BV_INT_INDEX(bit);
	int n = BV_BIT_INDEX(bit, i);

	_BV_ASSERT(i < BV_NUM_INTS(bv[0]));

	bv[i] &= ~(1<<n);
}

/******************************************************************************
bv_reset

Resets all bits of the bitvector

IN:   bitvector bv
      int       size    Size of the bitvector
*******************************************************************************/
void bv_reset(bitvector bv, int size) {
	_BV_ASSERT(bv[0] == size);

	int n = BV_NUM_INTS(size);

#ifdef BV_DEBUG_CHECK
	for(int i = 1; i < n; i++)
#else
	for(int i = 0; i < n; i++)
#endif
		bv[i] = 0;
}

/******************************************************************************
bv_is_empty

Checks if no bits of the bitvector are set == bitvector is "empty"

IN:   bitvector bv
      int       size    Size of the bitvector

RETURN: bool  return true if bv is empty, false otherwise
*******************************************************************************/
bool bv_is_empty(bitvector bv, int size) {
	_BV_ASSERT(bv[0] == size);

	int n = BV_NUM_INTS(size);

	bool empty = true;
#ifdef BV_DEBUG_CHECK
	for(int i = 1; (i < n) && empty; i++)
#else
	for(int i = 0; (i < n) && empty; i++)
#endif
		empty = empty && (bv[i] == 0);
	return empty;
}

/******************************************************************************
bv_copy

Copyes bitvector src to dst

IN:   bitvector dst     bitvector created with bv_new
      bitvector src     bitvector created with bv_new
      int       size    Size of the bitvector
*******************************************************************************/
void bv_copy(bitvector dst, bitvector src, int size) {
	int i,n;
	/* copy the whole bitvector    */
	_BV_ASSERT(dst[0] == size);
	_BV_ASSERT(src[0] == size);
	n = BV_NUM_INTS(size);

#ifdef BV_DEBUG_CHECK
	for(i = 1; i < n; i++)
#else
	for(i = 0; i < n; i++)
#endif
		dst[i] = src[i];
}

/******************************************************************************
bv_equal

Compares two  bitvectors

IN:   bitvector s1      bitvector created with bv_new
      bitvector s2      bitvector created with bv_new
      int       size    Size of the bitvector

RETURN: bool    true if s1==s1, false otherwise
*******************************************************************************/
bool bv_equal(bitvector s1, bitvector s2, int size) {
	bool equal = true;

	/* copy the whole bitvector    */
	_BV_ASSERT(s1[0] == size);
	_BV_ASSERT(s2[0] == size);

	if (size == 0)
		return true;

	int n = BV_NUM_INTS(size);

#ifdef BV_DEBUG_CHECK
	for(int i = 1; equal && (i < n-1); i++)
#else
	for(int i = 0; equal && (i < n-1); i++)
#endif
		equal = (s1[i] == s2[i]);

	/* Last compare maybe has to be masked! */

	int i = BV_INT_INDEX(size - 1);
	n = BV_BIT_INDEX(size - 1, i);

	_BV_ASSERT(i < BV_NUM_INTS(s1[0]));
	_BV_ASSERT(i < BV_NUM_INTS(s2[0]));

	int mask;

	if (n == (sizeof(int) * 8 - 1)) {
		/* full mask */
		mask = -1;
	} else {
		mask = (1<<(n+1)) - 1;
	}

	equal = equal && ( (s1[i]&mask) == (s2[i]&mask));

	return equal;
}

/******************************************************************************
bv_minus

d = s1 \ s2. ( set minus operator )

IN:    bitvector s1      bitvector created with bv_new
       bitvector s2      bitvector created with bv_new
       int       size    Size of the bitvector

IN/OUT:bitvector d       bitvector created with bv_new

*******************************************************************************/
void bv_minus(bitvector d, bitvector s1, bitvector s2, int size) {
    /* d = s1 - s2     */
	_BV_ASSERT(d[0] == size);
	_BV_ASSERT(s1[0] == size);
	_BV_ASSERT(s2[0] == size);
	int n = BV_NUM_INTS(size);
#ifdef BV_DEBUG_CHECK
	for(int i = 1; i < n; i++)
#else
	for(int i = 0; i < n; i++)
#endif
		d[i] = s1[i] & (~s2[i]);
}

/******************************************************************************
bv_minus

d = s1 "union" s2. ( set union operator )

IN:    bitvector s1      bitvector created with bv_new
       bitvector s2      bitvector created with bv_new
       int       size    Size of the bitvector

IN/OUT:bitvector d       bitvector created with bv_new

*******************************************************************************/
void bv_union(bitvector d, bitvector s1, bitvector s2, int size) {
    /* d = s1 union s2 */
	_BV_ASSERT(d[0] == size);
	_BV_ASSERT(s1[0] == size);
	_BV_ASSERT(s2[0] == size);
	int n = BV_NUM_INTS(size);
#ifdef BV_DEBUG_CHECK
	for(int i = 1; i < n; i++)
#else
	for(int i = 0; i < n; i++)
#endif
		d[i] = s1[i] | s2[i];
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
