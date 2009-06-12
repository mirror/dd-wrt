/* Type information for emit-rtl.c.
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

const struct ggc_root_tab gt_ggc_r_gt_emit_rtl_h[] = {
  {
    &static_regno_reg_rtx[0],
    1 * (FIRST_PSEUDO_REGISTER),
    sizeof (static_regno_reg_rtx[0]),
    &gt_ggc_mx_rtx_def

  },
  LAST_GGC_ROOT_TAB
};

const struct ggc_root_tab gt_ggc_rd_gt_emit_rtl_h[] = {
  { &free_sequence_stack, 1, sizeof (free_sequence_stack), NULL },
  LAST_GGC_ROOT_TAB
};

const struct ggc_cache_tab gt_ggc_rc_gt_emit_rtl_h[] = {
  {
    &const_double_htab,
    1,
    sizeof (const_double_htab),
    &gt_ggc_mx_rtx_def
,
    &ggc_marked_p
  },
  {
    &mem_attrs_htab,
    1,
    sizeof (mem_attrs_htab),
    &gt_ggc_mx_mem_attrs
,
    &ggc_marked_p
  },
  {
    &const_int_htab,
    1,
    sizeof (const_int_htab),
    &gt_ggc_mx_rtx_def
,
    &ggc_marked_p
  },
  LAST_GGC_CACHE_TAB
};

