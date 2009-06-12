/* Type information for tree.c.
   Copyright (C) 2002 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

/* This file is machine generated.  Do not edit.  */

void
gt_ggc_mx_type_hash (x_p)
      void *x_p;
{
  struct type_hash * const x = (struct type_hash *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_9tree_node ((*x).type);
  }
}

void
gt_ggc_m_P9type_hash4htab (x_p)
      void *x_p;
{
  struct htab * const x = (struct htab *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      if ((*x).entries != NULL) {
        size_t i1;
        ggc_set_mark ((*x).entries);
        for (i1 = 0; i1 < (size_t)(((*x)).size); i1++) {
          gt_ggc_m_9type_hash ((*x).entries[i1]);
        }
      }
  }
}

/* GC roots.  */

const struct ggc_cache_tab gt_ggc_rc_gt_tree_h[] = {
  {
    &type_hash_table,
    1,
    sizeof (type_hash_table),
    &gt_ggc_mx_type_hash
,
    &type_hash_marked_p
  },
  LAST_GGC_CACHE_TAB
};

