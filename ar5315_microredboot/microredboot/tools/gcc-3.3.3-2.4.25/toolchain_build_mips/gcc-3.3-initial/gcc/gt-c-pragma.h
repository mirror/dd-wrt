/* Type information for c-pragma.c.
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
gt_ggc_mx_align_stack (x_p)
      void *x_p;
{
  struct align_stack * const x = (struct align_stack *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_9tree_node ((*x).id);
      gt_ggc_m_11align_stack ((*x).prev);
  }
}

/* GC roots.  */

const struct ggc_root_tab gt_ggc_r_gt_c_pragma_h[] = {
  {
    &pragma_extern_prefix,
    1,
    sizeof (pragma_extern_prefix),
    &gt_ggc_mx_tree_node

  },
  {
    &pending_redefine_extname,
    1,
    sizeof (pending_redefine_extname),
    &gt_ggc_mx_tree_node

  },
  {
    &pending_weaks,
    1,
    sizeof (pending_weaks),
    &gt_ggc_mx_tree_node

  },
  {
    &alignment_stack,
    1,
    sizeof (alignment_stack),
    &gt_ggc_mx_align_stack

  },
  LAST_GGC_ROOT_TAB
};

