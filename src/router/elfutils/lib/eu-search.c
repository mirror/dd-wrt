/* Definitions for thread-safe tsearch/tfind
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "eu-search.h"

void *eu_tsearch (const void *key, search_tree *tree,
		  int (*compare)(const void *, const void *))
{
  rwlock_wrlock (tree->lock);
  void *ret = tsearch (key, &tree->root, compare);
  rwlock_unlock (tree->lock);

  return ret;
}

void *eu_tfind (const void *key, search_tree *tree,
	        int (*compare)(const void *, const void *))
{
  rwlock_rdlock (tree->lock);
  void *ret = tfind (key, &tree->root, compare);
  rwlock_unlock (tree->lock);

  return ret;
}

void *eu_tdelete (const void *key, search_tree *tree,
		  int (*compare)(const void *, const void *))
{
  rwlock_wrlock (tree->lock);
  void *ret = tdelete (key, &tree->root, compare);
  rwlock_unlock (tree->lock);

  return ret;
}

void eu_tdestroy (search_tree *tree, void (*free_node)(void *))
{
  rwlock_wrlock (tree->lock);

  tdestroy (tree->root, free_node);
  tree->root = NULL;

  rwlock_unlock (tree->lock);
}

void eu_search_tree_init (search_tree *tree)
{
  tree->root = NULL;
  rwlock_init (tree->lock);
}

void eu_search_tree_fini (search_tree *tree, void (*free_node)(void *))
{
  eu_tdestroy (tree, free_node);
  rwlock_fini (tree->lock);
}
