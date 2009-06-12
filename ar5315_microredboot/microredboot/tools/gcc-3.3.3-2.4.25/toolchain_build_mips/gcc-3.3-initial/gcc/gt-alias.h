/* Type information for alias.c.
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

static void gt_ggc_ma_reg_base_value PARAMS ((void *));
static void
gt_ggc_ma_reg_base_value (x_p)
      void *x_p;
{
  size_t i;
  struct rtx_def ** const x = (struct rtx_def **)x_p;
  if (ggc_test_and_set_mark (x))
    for (i = 0; i < (reg_base_value_size); i++)
      gt_ggc_m_7rtx_def (x[i]);
}

const struct ggc_root_tab gt_ggc_r_gt_alias_h[] = {
  {
    &static_reg_base_value[0],
    1 * (FIRST_PSEUDO_REGISTER),
    sizeof (static_reg_base_value[0]),
    &gt_ggc_mx_rtx_def

  },
  {
    &reg_base_value,
    1,
    sizeof (reg_base_value),
    &gt_ggc_ma_reg_base_value
  },
  LAST_GGC_ROOT_TAB
};

