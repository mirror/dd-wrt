/* Type information for stmt.c.
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
gt_ggc_mx_goto_fixup (x_p)
      void *x_p;
{
  struct goto_fixup * const x = (struct goto_fixup *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_10goto_fixup ((*x).next);
      gt_ggc_m_7rtx_def ((*x).before_jump);
      gt_ggc_m_9tree_node ((*x).target);
      gt_ggc_m_9tree_node ((*x).context);
      gt_ggc_m_7rtx_def ((*x).target_rtl);
      gt_ggc_m_7rtx_def ((*x).stack_level);
      gt_ggc_m_9tree_node ((*x).cleanup_list_list);
  }
}

void
gt_ggc_mx_label_chain (x_p)
      void *x_p;
{
  struct label_chain * const x = (struct label_chain *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_11label_chain ((*x).next);
      gt_ggc_m_9tree_node ((*x).label);
  }
}

void
gt_ggc_mx_nesting (x_p)
      void *x_p;
{
  struct nesting * const x = (struct nesting *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_7nesting ((*x).all);
      gt_ggc_m_7nesting ((*x).next);
      gt_ggc_m_7rtx_def ((*x).exit_label);
      switch (((*x)).desc)
        {
        case COND_NESTING:
          gt_ggc_m_7rtx_def ((*x).data.cond.endif_label);
          gt_ggc_m_7rtx_def ((*x).data.cond.next_label);
          break;
        case LOOP_NESTING:
          gt_ggc_m_7rtx_def ((*x).data.loop.start_label);
          gt_ggc_m_7rtx_def ((*x).data.loop.end_label);
          gt_ggc_m_7rtx_def ((*x).data.loop.alt_end_label);
          gt_ggc_m_7rtx_def ((*x).data.loop.continue_label);
          break;
        case BLOCK_NESTING:
          gt_ggc_m_7rtx_def ((*x).data.block.stack_level);
          gt_ggc_m_7rtx_def ((*x).data.block.first_insn);
          gt_ggc_m_7nesting ((*x).data.block.innermost_stack_block);
          gt_ggc_m_9tree_node ((*x).data.block.cleanups);
          gt_ggc_m_9tree_node ((*x).data.block.outer_cleanups);
          gt_ggc_m_11label_chain ((*x).data.block.label_chain);
          gt_ggc_m_7rtx_def ((*x).data.block.last_unconditional_cleanup);
          break;
        case CASE_NESTING:
          gt_ggc_m_7rtx_def ((*x).data.case_stmt.start);
          gt_ggc_m_9case_node ((*x).data.case_stmt.case_list);
          gt_ggc_m_9tree_node ((*x).data.case_stmt.default_label);
          gt_ggc_m_9tree_node ((*x).data.case_stmt.index_expr);
          gt_ggc_m_9tree_node ((*x).data.case_stmt.nominal_type);
          break;
        default:
          break;
        }
  }
}

void
gt_ggc_mx_case_node (x_p)
      void *x_p;
{
  struct case_node * const x = (struct case_node *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_9case_node ((*x).left);
      gt_ggc_m_9case_node ((*x).right);
      gt_ggc_m_9case_node ((*x).parent);
      gt_ggc_m_9tree_node ((*x).low);
      gt_ggc_m_9tree_node ((*x).high);
      gt_ggc_m_9tree_node ((*x).code_label);
  }
}

void
gt_ggc_mx_stmt_status (x_p)
      void *x_p;
{
  struct stmt_status * const x = (struct stmt_status *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_7nesting ((*x).x_block_stack);
      gt_ggc_m_7nesting ((*x).x_stack_block_stack);
      gt_ggc_m_7nesting ((*x).x_cond_stack);
      gt_ggc_m_7nesting ((*x).x_loop_stack);
      gt_ggc_m_7nesting ((*x).x_case_stack);
      gt_ggc_m_7nesting ((*x).x_nesting_stack);
      gt_ggc_m_9tree_node ((*x).x_last_expr_type);
      gt_ggc_m_7rtx_def ((*x).x_last_expr_value);
      gt_ggc_m_10goto_fixup ((*x).x_goto_fixup_chain);
  }
}
