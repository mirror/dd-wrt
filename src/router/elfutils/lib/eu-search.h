/* Calls for thread-safe tsearch/tfind
   Copyright (C) 2023 Rice University
   This file is part of elfutils.

   This file is free software; you can redistribute it and/or modify
   it under the terms of either

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at
       your option) any later version

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at
       your option) any later version

   or both in parallel, as here.

   elfutils is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see <http://www.gnu.org/licenses/>.  */

#ifndef EU_SEARCH_H
#define EU_SEARCH_H 1

#include <stdlib.h>
#include <search.h>
#include <locks.h>

typedef struct
{
  void *root;
  rwlock_define (, lock);
} search_tree;

/* Search TREE for KEY and add KEY if not found. Synchronized using
   TREE's lock.  */
extern void *eu_tsearch (const void *key, search_tree *tree,
			 int (*compare)(const void *, const void *));

/* Search TREE for KEY. Synchronized with TREE's lock.  */
extern void *eu_tfind (const void *key, search_tree *tree,
		       int (*compare)(const void *, const void *));

/* Delete key from TREE. Synchronized with TREE's lock.  */
extern void *eu_tdelete (const void *key, search_tree *tree,
		         int (*compare)(const void *, const void *));

/* Search TREE for KEY and add KEY if not found.  No locking is performed.  */
static inline void *
eu_tsearch_nolock (const void *key, search_tree *tree,
		   int (*compare)(const void *, const void *))
{
  return tsearch (key, &tree->root, compare);
}

/* Search TREE for KEY.  No locking is performed.  */
static inline void *
eu_tfind_nolock (const void *key, search_tree *tree,
		 int (*compare)(const void *, const void *))
{
  return tfind (key, &tree->root, compare);
}

/* Delete key from TREE.  No locking is performed.  */
static inline void *
eu_tdelete_nolock (const void *key, search_tree *tree,
		   int (*compare)(const void *, const void *))
{
  return tdelete (key, &tree->root, compare);
}

/* Free all nodes from TREE.  */
void eu_tdestroy (search_tree *tree, void (*free_node)(void *));

/* Initialize TREE's root and lock.  */
void eu_search_tree_init (search_tree *tree);

/* Free all nodes from TREE as well as TREE's lock.  */
void eu_search_tree_fini (search_tree *tree, void (*free_node)(void *));

#endif
