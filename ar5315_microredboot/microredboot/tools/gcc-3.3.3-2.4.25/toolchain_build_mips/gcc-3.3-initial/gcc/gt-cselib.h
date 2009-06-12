/* Type information for cselib.c.
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

/* GC roots.  */

const struct ggc_root_tab gt_ggc_r_gt_cselib_h[] = {
  {
    &callmem,
    1,
    sizeof (callmem),
    &gt_ggc_mx_rtx_def

  },
  {
    &used_regs,
    1,
    sizeof (used_regs),
    &gt_ggc_mx_varray_head_tag

  },
  {
    &reg_values,
    1,
    sizeof (reg_values),
    &gt_ggc_mx_varray_head_tag

  },
  {
    &hash_table,
    1,
    sizeof (hash_table),
    &gt_ggc_m_P17cselib_val_struct4htab
  },
  LAST_GGC_ROOT_TAB
};

const struct ggc_root_tab gt_ggc_rd_gt_cselib_h[] = {
  { &empty_elt_loc_lists, 1, sizeof (empty_elt_loc_lists), NULL },
  { &empty_elt_lists, 1, sizeof (empty_elt_lists), NULL },
  { &empty_vals, 1, sizeof (empty_vals), NULL },
  { &used_regs_old, 1, sizeof (used_regs_old), NULL },
  { &reg_values_old, 1, sizeof (reg_values_old), NULL },
  LAST_GGC_ROOT_TAB
};

