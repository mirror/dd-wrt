/* Type information for except.c.
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
gt_ggc_mx_eh_region (x_p)
      void *x_p;
{
  struct eh_region * const x = (struct eh_region *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_9eh_region ((*x).outer);
      gt_ggc_m_9eh_region ((*x).inner);
      gt_ggc_m_9eh_region ((*x).next_peer);
      gt_ggc_m_15bitmap_head_def ((*x).aka);
      switch ((*x).type)
        {
        case ERT_TRY:
          gt_ggc_m_9eh_region ((*x).u.try.catch);
          gt_ggc_m_9eh_region ((*x).u.try.last_catch);
          gt_ggc_m_9eh_region ((*x).u.try.prev_try);
          gt_ggc_m_7rtx_def ((*x).u.try.continue_label);
          break;
        case ERT_CATCH:
          gt_ggc_m_9eh_region ((*x).u.catch.next_catch);
          gt_ggc_m_9eh_region ((*x).u.catch.prev_catch);
          gt_ggc_m_9tree_node ((*x).u.catch.type_list);
          gt_ggc_m_9tree_node ((*x).u.catch.filter_list);
          break;
        case ERT_ALLOWED_EXCEPTIONS:
          gt_ggc_m_9tree_node ((*x).u.allowed.type_list);
          break;
        case ERT_THROW:
          gt_ggc_m_9tree_node ((*x).u.throw.type);
          break;
        case ERT_CLEANUP:
          gt_ggc_m_9tree_node ((*x).u.cleanup.exp);
          gt_ggc_m_9eh_region ((*x).u.cleanup.prev_try);
          break;
        case ERT_FIXUP:
          gt_ggc_m_9tree_node ((*x).u.fixup.cleanup_exp);
          gt_ggc_m_9eh_region ((*x).u.fixup.real_region);
          break;
        default:
          break;
        }
      gt_ggc_m_7rtx_def ((*x).label);
      gt_ggc_m_7rtx_def ((*x).landing_pad);
      gt_ggc_m_7rtx_def ((*x).post_landing_pad);
      gt_ggc_m_7rtx_def ((*x).resume);
  }
}

void
gt_ggc_mx_ehl_map_entry (x_p)
      void *x_p;
{
  struct ehl_map_entry * const x = (struct ehl_map_entry *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_7rtx_def ((*x).label);
      gt_ggc_m_9eh_region ((*x).region);
  }
}

void
gt_ggc_mx_eh_status (x_p)
      void *x_p;
{
  struct eh_status * const x = (struct eh_status *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_9eh_region ((*x).region_tree);
      if ((*x).region_array != NULL) {
        size_t i1;
        ggc_set_mark ((*x).region_array);
        for (i1 = 0; i1 < (size_t)(((*x)).last_region_number); i1++) {
          gt_ggc_m_9eh_region ((*x).region_array[i1]);
        }
      }
      gt_ggc_m_9eh_region ((*x).cur_region);
      gt_ggc_m_9eh_region ((*x).try_region);
      gt_ggc_m_7rtx_def ((*x).filter);
      gt_ggc_m_7rtx_def ((*x).exc_ptr);
      gt_ggc_m_15varray_head_tag ((*x).ttype_data);
      gt_ggc_m_15varray_head_tag ((*x).ehspec_data);
      gt_ggc_m_15varray_head_tag ((*x).action_record_data);
      gt_ggc_m_P13ehl_map_entry4htab ((*x).exception_handler_label_map);
      if ((*x).call_site_data != NULL) {
        size_t i2;
        ggc_set_mark ((*x).call_site_data);
        for (i2 = 0; i2 < (size_t)(((*x)).call_site_data_used); i2++) {
          gt_ggc_m_7rtx_def ((*x).call_site_data[i2].landing_pad);
        }
      }
      gt_ggc_m_7rtx_def ((*x).ehr_stackadj);
      gt_ggc_m_7rtx_def ((*x).ehr_handler);
      gt_ggc_m_7rtx_def ((*x).ehr_label);
      gt_ggc_m_7rtx_def ((*x).sjlj_fc);
      gt_ggc_m_7rtx_def ((*x).sjlj_exit_after);
  }
}

void
gt_ggc_m_P13ehl_map_entry4htab (x_p)
      void *x_p;
{
  struct htab * const x = (struct htab *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      if ((*x).entries != NULL) {
        size_t i1;
        ggc_set_mark ((*x).entries);
        for (i1 = 0; i1 < (size_t)(((*x)).size); i1++) {
          gt_ggc_m_13ehl_map_entry ((*x).entries[i1]);
        }
      }
  }
}

/* GC roots.  */

const struct ggc_root_tab gt_ggc_r_gt_except_h[] = {
  {
    &sjlj_fc_type_node,
    1,
    sizeof (sjlj_fc_type_node),
    &gt_ggc_mx_tree_node

  },
  {
    &type_to_runtime_map,
    1,
    sizeof (type_to_runtime_map),
    &gt_ggc_m_P9tree_node4htab
  },
  LAST_GGC_ROOT_TAB
};

