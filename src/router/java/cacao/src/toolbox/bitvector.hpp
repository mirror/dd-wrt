/* src/toolbox/bitvector.hpp - bitvector header

   Copyright (C) 2005-2013
   CACAOVM - Verein zu Foerderung der freien virtuellen Machine CACAO

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

   Contact: cacao@complang.tuwien.ac.at

   Authors: Christian Ullrich


*/


#ifndef BITVECTOR_HPP_
#define BITVECTOR_HPP_ 1

typedef int *bitvector;

/* function prototypes */
char *bv_to_string(bitvector bv, char *string, int size);
bitvector bv_new(int size);           /* Create a new Bitvector for size Bits */
                                      /* All bits are reset                   */
void bv_set_bit(bitvector bv, int bit);    /* set Bit bit of bitvector       */
void bv_reset_bit(bitvector bv, int bit);  /* reset Bit bit of bitvector     */
void bv_reset(bitvector bv, int size);     /* reset the whole bitvector      */
bool bv_is_empty(bitvector bv, int size);  /* Returns if no Bit is set       */
bool bv_get_bit(bitvector bv, int bit);    /* Returns if Bit bit is set      */
bool bv_equal(bitvector s1, bitvector s2, int size);

/* copy the whole bitvector    */

void bv_copy(bitvector dst, bitvector src, int size);

/* d = s1 \ s2     */

void bv_minus(bitvector d, bitvector s1, bitvector s2, int size);

/* d = s1 union s2 */

void bv_union(bitvector d, bitvector s1, bitvector s2, int size);

#endif // BITVECTOR_HPP


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
