/* Type information for integrate.c.
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
gt_ggc_mx_initial_value_struct (x_p)
      void *x_p;
{
  struct initial_value_struct * const x = (struct initial_value_struct *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      if ((*x).entries != NULL) {
        size_t i1;
        ggc_set_mark ((*x).entries);
        for (i1 = 0; i1 < (size_t)(((*x)).num_entries); i1++) {
          gt_ggc_m_7rtx_def ((*x).entries[i1].hard_reg);
          gt_ggc_m_7rtx_def ((*x).entries[i1].pseudo);
        }
      }
  }
}

/* GC roots.  */

const struct ggc_root_tab gt_ggc_r_gt_integrate_h[] = {
  {
    &old_cfun,
    1,
    sizeof (old_cfun),
    &gt_ggc_mx_function

  },
  LAST_GGC_ROOT_TAB
};

