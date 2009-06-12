/* Type information for varasm.c.
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
gt_ggc_mx_deferred_string (x_p)
      void *x_p;
{
  struct deferred_string * const x = (struct deferred_string *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_9tree_node ((*x).exp);
  }
}

void
gt_ggc_mx_constant_descriptor_tree (x_p)
      void *x_p;
{
  struct constant_descriptor_tree * const x = (struct constant_descriptor_tree *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_24constant_descriptor_tree ((*x).next);
      gt_ggc_m_7rtx_def ((*x).rtl);
      gt_ggc_m_9tree_node ((*x).value);
  }
}

void
gt_ggc_mx_pool_constant (x_p)
      void *x_p;
{
  struct pool_constant * const x = (struct pool_constant *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_23constant_descriptor_rtx ((*x).desc);
      gt_ggc_m_13pool_constant ((*x).next);
      gt_ggc_m_13pool_constant ((*x).next_sym);
      gt_ggc_m_7rtx_def ((*x).constant);
  }
}

void
gt_ggc_mx_constant_descriptor_rtx (x_p)
      void *x_p;
{
  struct constant_descriptor_rtx * const x = (struct constant_descriptor_rtx *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_23constant_descriptor_rtx ((*x).next);
      gt_ggc_m_7rtx_def ((*x).rtl);
      switch (((*x).value).kind >= RTX_INT)
        {
        case 1:
          gt_ggc_m_7rtx_def ((*x).value.un.addr.base);
          break;
        case 0:
          break;
        case 2:
          {
            size_t i1_0;
            const size_t ilimit1_0 = (16);
            for (i1_0 = 0; i1_0 < ilimit1_0; i1_0++) {
            }
          }
          break;
        default:
          break;
        }
  }
}

void
gt_ggc_mx_varasm_status (x_p)
      void *x_p;
{
  struct varasm_status * const x = (struct varasm_status *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      if ((*x).x_const_rtx_hash_table != NULL) {
        size_t i1;
        ggc_set_mark ((*x).x_const_rtx_hash_table);
        for (i1 = 0; i1 < (size_t)(MAX_RTX_HASH_TABLE); i1++) {
          gt_ggc_m_23constant_descriptor_rtx ((*x).x_const_rtx_hash_table[i1]);
        }
      }
      if ((*x).x_const_rtx_sym_hash_table != NULL) {
        size_t i2;
        ggc_set_mark ((*x).x_const_rtx_sym_hash_table);
        for (i2 = 0; i2 < (size_t)(MAX_RTX_HASH_TABLE); i2++) {
          gt_ggc_m_13pool_constant ((*x).x_const_rtx_sym_hash_table[i2]);
        }
      }
      gt_ggc_m_13pool_constant ((*x).x_first_pool);
      gt_ggc_m_13pool_constant ((*x).x_last_pool);
  }
}

void
gt_ggc_m_P15deferred_string4htab (x_p)
      void *x_p;
{
  struct htab * const x = (struct htab *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      if ((*x).entries != NULL) {
        size_t i1;
        ggc_set_mark ((*x).entries);
        for (i1 = 0; i1 < (size_t)(((*x)).size); i1++) {
          gt_ggc_m_15deferred_string ((*x).entries[i1]);
        }
      }
  }
}

/* GC roots.  */

const struct ggc_root_tab gt_ggc_r_gt_varasm_h[] = {
  {
    &weak_decls,
    1,
    sizeof (weak_decls),
    &gt_ggc_mx_tree_node

  },
  {
    &const_str_htab,
    1,
    sizeof (const_str_htab),
    &gt_ggc_m_P15deferred_string4htab
  },
  {
    &const_hash_table[0],
    1 * (MAX_HASH_TABLE),
    sizeof (const_hash_table[0]),
    &gt_ggc_mx_constant_descriptor_tree

  },
  LAST_GGC_ROOT_TAB
};

