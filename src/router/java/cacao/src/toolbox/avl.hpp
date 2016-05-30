/* src/toolbox/avl.hpp - AVL tree implementation

   Copyright (C) 1996-2013
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


#ifndef AVL_HPP_
#define AVL_HPP_ 1

#include "vm/types.hpp"                 // for s4

struct avl_tree_t;
struct avl_node_t;

/* tree comparator prototype **************************************************/

typedef s4 avl_comparator(const void *treenode, const void *node);

/* function prototypes ********************************************************/

avl_tree_t *avl_create(avl_comparator *comparator);
bool        avl_insert(avl_tree_t *tree, void *data);
void       *avl_find(avl_tree_t *tree, void *data);

#if !defined(NDEBUG)
void        avl_dump(avl_node_t* node, s4 indent);
#endif

#endif // AVL_HPP_


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
