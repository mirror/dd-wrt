/* Type information for function.c.
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
gt_ggc_mx_temp_slot (x_p)
      void *x_p;
{
  struct temp_slot * const x = (struct temp_slot *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_9temp_slot ((*x).next);
      gt_ggc_m_7rtx_def ((*x).slot);
      gt_ggc_m_7rtx_def ((*x).address);
      gt_ggc_m_9tree_node ((*x).type);
      gt_ggc_m_9tree_node ((*x).rtl_expr);
  }
}

/* GC roots.  */

const struct ggc_root_tab gt_ggc_r_gt_function_h[] = {
  {
    &initial_trampoline,
    1,
    sizeof (initial_trampoline),
    &gt_ggc_mx_rtx_def

  },
  {
    &outer_function_chain,
    1,
    sizeof (outer_function_chain),
    &gt_ggc_mx_function

  },
  {
    &sibcall_epilogue,
    1,
    sizeof (sibcall_epilogue),
    &gt_ggc_mx_varray_head_tag

  },
  {
    &epilogue,
    1,
    sizeof (epilogue),
    &gt_ggc_mx_varray_head_tag

  },
  {
    &prologue,
    1,
    sizeof (prologue),
    &gt_ggc_mx_varray_head_tag

  },
  LAST_GGC_ROOT_TAB
};

