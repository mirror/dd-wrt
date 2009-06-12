/* Type information for GCC.
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
#include "config.h"
#include "system.h"
#include "varray.h"
#include "hashtab.h"
#include "splay-tree.h"
#include "bitmap.h"
#include "tree.h"
#include "rtl.h"
#include "function.h"
#include "insn-config.h"
#include "expr.h"
#include "hard-reg-set.h"
#include "basic-block.h"
#include "cselib.h"
#include "insn-addr.h"
#include "ssa.h"
#include "optabs.h"
#include "libfuncs.h"
#include "debug.h"
#include "ggc.h"

void
gt_ggc_mx_elt_loc_list (x_p)
      void *x_p;
{
  struct elt_loc_list * const x = (struct elt_loc_list *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_12elt_loc_list ((*x).next);
      gt_ggc_m_7rtx_def ((*x).loc);
      gt_ggc_m_7rtx_def ((*x).setting_insn);
  }
}

void
gt_ggc_mx_cselib_val_struct (x_p)
      void *x_p;
{
  struct cselib_val_struct * const x = (struct cselib_val_struct *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      switch (1)
        {
        case 1:
          gt_ggc_m_7rtx_def ((*x).u.val_rtx);
          break;
        default:
          break;
        }
      gt_ggc_m_12elt_loc_list ((*x).locs);
      gt_ggc_m_8elt_list ((*x).addr_list);
  }
}

void
gt_ggc_mx_varray_head_tag (x_p)
      void *x_p;
{
  struct varray_head_tag * const x = (struct varray_head_tag *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      switch ((*x).type)
        {
        case VARRAY_DATA_GENERIC:
          {
            size_t i1_0;
            const size_t ilimit1_0 = ((*x).num_elements);
            for (i1_0 = 0; i1_0 < ilimit1_0; i1_0++) {
              abort();
            }
          }
          break;
        case VARRAY_DATA_CPTR:
          break;
        case VARRAY_DATA_RTX:
          {
            size_t i3_0;
            const size_t ilimit3_0 = ((*x).num_elements);
            for (i3_0 = 0; i3_0 < ilimit3_0; i3_0++) {
              gt_ggc_m_7rtx_def ((*x).data.rtx[i3_0]);
            }
          }
          break;
        case VARRAY_DATA_RTVEC:
          {
            size_t i4_0;
            const size_t ilimit4_0 = ((*x).num_elements);
            for (i4_0 = 0; i4_0 < ilimit4_0; i4_0++) {
              gt_ggc_m_9rtvec_def ((*x).data.rtvec[i4_0]);
            }
          }
          break;
        case VARRAY_DATA_TREE:
          {
            size_t i5_0;
            const size_t ilimit5_0 = ((*x).num_elements);
            for (i5_0 = 0; i5_0 < ilimit5_0; i5_0++) {
              gt_ggc_m_9tree_node ((*x).data.tree[i5_0]);
            }
          }
          break;
        case VARRAY_DATA_BITMAP:
          {
            size_t i6_0;
            const size_t ilimit6_0 = ((*x).num_elements);
            for (i6_0 = 0; i6_0 < ilimit6_0; i6_0++) {
              gt_ggc_m_15bitmap_head_def ((*x).data.bitmap[i6_0]);
            }
          }
          break;
        case VARRAY_DATA_CONST_EQUIV:
          {
            size_t i7_0;
            const size_t ilimit7_0 = ((*x).num_elements);
            for (i7_0 = 0; i7_0 < ilimit7_0; i7_0++) {
              gt_ggc_m_7rtx_def ((*x).data.const_equiv[i7_0].rtx);
            }
          }
          break;
        case VARRAY_DATA_TE:
          {
            size_t i8_0;
            const size_t ilimit8_0 = ((*x).num_elements);
            for (i8_0 = 0; i8_0 < ilimit8_0; i8_0++) {
              gt_ggc_m_8elt_list ((*x).data.te[i8_0]);
            }
          }
          break;
        default:
          break;
        }
  }
}

void
gt_ggc_mx_elt_list (x_p)
      void *x_p;
{
  struct elt_list * const x = (struct elt_list *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_8elt_list ((*x).next);
      gt_ggc_m_17cselib_val_struct ((*x).elt);
  }
}

void
gt_ggc_mx_real_value (x_p)
      void *x_p;
{
  struct real_value * const x = (struct real_value *)x_p;
  if (ggc_test_and_set_mark (x))
    {
  }
}

void
gt_ggc_mx_optab (x_p)
      void *x_p;
{
  struct optab * const x = (struct optab *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      {
        size_t i1_0;
        const size_t ilimit1_0 = (NUM_MACHINE_MODES);
        for (i1_0 = 0; i1_0 < ilimit1_0; i1_0++) {
          gt_ggc_m_7rtx_def ((*x).handlers[i1_0].libfunc);
        }
      }
  }
}

void
gt_ggc_mx_mem_attrs (x_p)
      void *x_p;
{
  struct mem_attrs * const x = (struct mem_attrs *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_9tree_node ((*x).expr);
      gt_ggc_m_7rtx_def ((*x).offset);
      gt_ggc_m_7rtx_def ((*x).size);
  }
}

void
gt_ggc_mx_function (x_p)
      void *x_p;
{
  struct function * const x = (struct function *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_9eh_status ((*x).eh);
      gt_ggc_m_11stmt_status ((*x).stmt);
      gt_ggc_m_11expr_status ((*x).expr);
      gt_ggc_m_11emit_status ((*x).emit);
      gt_ggc_m_13varasm_status ((*x).varasm);
      gt_ggc_m_9tree_node ((*x).decl);
      gt_ggc_m_8function ((*x).outer);
      gt_ggc_m_7rtx_def ((*x).arg_offset_rtx);
      gt_ggc_m_7rtx_def ((*x).return_rtx);
      gt_ggc_m_7rtx_def ((*x).internal_arg_pointer);
      gt_ggc_m_20initial_value_struct ((*x).hard_reg_initial_vals);
      gt_ggc_m_9tree_node ((*x).x_nonlocal_labels);
      gt_ggc_m_7rtx_def ((*x).x_nonlocal_goto_handler_slots);
      gt_ggc_m_7rtx_def ((*x).x_nonlocal_goto_handler_labels);
      gt_ggc_m_7rtx_def ((*x).x_nonlocal_goto_stack_level);
      gt_ggc_m_7rtx_def ((*x).x_cleanup_label);
      gt_ggc_m_7rtx_def ((*x).x_return_label);
      gt_ggc_m_7rtx_def ((*x).computed_goto_common_label);
      gt_ggc_m_7rtx_def ((*x).computed_goto_common_reg);
      gt_ggc_m_7rtx_def ((*x).x_save_expr_regs);
      gt_ggc_m_7rtx_def ((*x).x_stack_slot_list);
      gt_ggc_m_9tree_node ((*x).x_rtl_expr_chain);
      gt_ggc_m_7rtx_def ((*x).x_tail_recursion_label);
      gt_ggc_m_7rtx_def ((*x).x_tail_recursion_reentry);
      gt_ggc_m_7rtx_def ((*x).x_arg_pointer_save_area);
      gt_ggc_m_7rtx_def ((*x).x_clobber_return_insn);
      gt_ggc_m_9tree_node ((*x).x_context_display);
      gt_ggc_m_9tree_node ((*x).x_trampoline_list);
      gt_ggc_m_7rtx_def ((*x).x_parm_birth_insn);
      gt_ggc_m_7rtx_def ((*x).x_last_parm_insn);
      if ((*x).x_parm_reg_stack_loc != NULL) {
        size_t i1;
        ggc_set_mark ((*x).x_parm_reg_stack_loc);
        for (i1 = 0; i1 < (size_t)(((*x)).x_max_parm_reg); i1++) {
          gt_ggc_m_7rtx_def ((*x).x_parm_reg_stack_loc[i1]);
        }
      }
      gt_ggc_m_9temp_slot ((*x).x_temp_slots);
      gt_ggc_m_14var_refs_queue ((*x).fixup_var_refs_queue);
      gt_ggc_m_9rtvec_def ((*x).original_arg_vector);
      gt_ggc_m_9tree_node ((*x).original_decl_initial);
      gt_ggc_m_7rtx_def ((*x).inl_last_parm_insn);
      gt_ggc_m_16machine_function ((*x).machine);
      gt_ggc_m_17language_function ((*x).language);
      gt_ggc_m_7rtx_def ((*x).epilogue_delay_list);
  }
}

void
gt_ggc_mx_expr_status (x_p)
      void *x_p;
{
  struct expr_status * const x = (struct expr_status *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_7rtx_def ((*x).x_saveregs_value);
      gt_ggc_m_7rtx_def ((*x).x_apply_args_value);
      gt_ggc_m_7rtx_def ((*x).x_forced_labels);
      gt_ggc_m_7rtx_def ((*x).x_pending_chain);
  }
}

void
gt_ggc_mx_emit_status (x_p)
      void *x_p;
{
  struct emit_status * const x = (struct emit_status *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_7rtx_def ((*x).x_first_insn);
      gt_ggc_m_7rtx_def ((*x).x_last_insn);
      gt_ggc_m_9tree_node ((*x).sequence_rtl_expr);
      gt_ggc_m_14sequence_stack ((*x).sequence_stack);
      ggc_mark ((*x).regno_pointer_align);
      if ((*x).regno_decl != NULL) {
        size_t i1;
        ggc_set_mark ((*x).regno_decl);
        for (i1 = 0; i1 < (size_t)(((*x)).regno_pointer_align_length); i1++) {
          gt_ggc_m_9tree_node ((*x).regno_decl[i1]);
        }
      }
      if ((*x).x_regno_reg_rtx != NULL) {
        size_t i2;
        ggc_set_mark ((*x).x_regno_reg_rtx);
        for (i2 = 0; i2 < (size_t)(((*x)).regno_pointer_align_length); i2++) {
          gt_ggc_m_7rtx_def ((*x).x_regno_reg_rtx[i2]);
        }
      }
  }
}

void
gt_ggc_mx_sequence_stack (x_p)
      void *x_p;
{
  struct sequence_stack * const x = (struct sequence_stack *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_7rtx_def ((*x).first);
      gt_ggc_m_7rtx_def ((*x).last);
      gt_ggc_m_9tree_node ((*x).sequence_rtl_expr);
      gt_ggc_m_14sequence_stack ((*x).next);
  }
}

void
gt_ggc_mx_var_refs_queue (x_p)
      void *x_p;
{
  struct var_refs_queue * const x = (struct var_refs_queue *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_7rtx_def ((*x).modified);
      gt_ggc_m_14var_refs_queue ((*x).next);
  }
}

void
gt_ggc_mx_bitmap_head_def (x_p)
      void *x_p;
{
  struct bitmap_head_def * const x = (struct bitmap_head_def *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_18bitmap_element_def ((*x).first);
      gt_ggc_m_18bitmap_element_def ((*x).current);
  }
}

void
gt_ggc_mx_bitmap_element_def (x_p)
      void *x_p;
{
  struct bitmap_element_def * const x = (struct bitmap_element_def *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      gt_ggc_m_18bitmap_element_def ((*x).next);
      gt_ggc_m_18bitmap_element_def ((*x).prev);
  }
}

void
gt_ggc_mx_rtvec_def (x_p)
      void *x_p;
{
  struct rtvec_def * const x = (struct rtvec_def *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      {
        size_t i1_0;
        const size_t ilimit1_0 = (((*x)).num_elem);
        for (i1_0 = 0; i1_0 < ilimit1_0; i1_0++) {
          gt_ggc_m_7rtx_def ((*x).elem[i1_0]);
        }
      }
  }
}

void
gt_ggc_mx_rtx_def (x_p)
      void *x_p;
{
  struct rtx_def * x = (struct rtx_def *)x_p;
  struct rtx_def * xlimit = x;
  while (ggc_test_and_set_mark (xlimit))
   xlimit = (RTX_NEXT (&(*xlimit)));
  if (x != xlimit)
    for (;;)
      {
        struct rtx_def * const xprev = (RTX_PREV (&(*x)));
        if (xprev == NULL) break;
        x = xprev;
        ggc_set_mark (xprev);
      }
  while (x != xlimit)
    {
      switch (GET_CODE (&(*x)))
        {
        case PHI:
          gt_ggc_m_9rtvec_def ((*x).fld[0].rtvec);
          break;
        case US_TRUNCATE:
          break;
        case SS_TRUNCATE:
          break;
        case US_MINUS:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case SS_MINUS:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case US_PLUS:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case SS_PLUS:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case VEC_DUPLICATE:
          break;
        case VEC_CONCAT:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case VEC_SELECT:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case VEC_MERGE:
          gt_ggc_m_7rtx_def ((*x).fld[2].rtx);
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case CALL_PLACEHOLDER:
          gt_ggc_m_7rtx_def ((*x).fld[3].rtx);
          gt_ggc_m_7rtx_def ((*x).fld[2].rtx);
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case CONSTANT_P_RTX:
          break;
        case RANGE_LIVE:
          gt_ggc_m_18bitmap_element_def ((*x).fld[0].rtbit);
          break;
        case RANGE_VAR:
          gt_ggc_m_9tree_node ((*x).fld[1].rttree);
          break;
        case RANGE_REG:
          gt_ggc_m_9tree_node ((*x).fld[9].rttree);
          gt_ggc_m_9tree_node ((*x).fld[8].rttree);
          break;
        case RANGE_INFO:
          gt_ggc_m_18bitmap_element_def ((*x).fld[10].rtbit);
          gt_ggc_m_18bitmap_element_def ((*x).fld[9].rtbit);
          gt_ggc_m_9rtvec_def ((*x).fld[2].rtvec);
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case LO_SUM:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case HIGH:
          break;
        case ZERO_EXTRACT:
          gt_ggc_m_7rtx_def ((*x).fld[2].rtx);
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case SIGN_EXTRACT:
          gt_ggc_m_7rtx_def ((*x).fld[2].rtx);
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case FFS:
          break;
        case SQRT:
          break;
        case ABS:
          break;
        case UNSIGNED_FIX:
          break;
        case UNSIGNED_FLOAT:
          break;
        case FIX:
          break;
        case FLOAT:
          break;
        case FLOAT_TRUNCATE:
          break;
        case FLOAT_EXTEND:
          break;
        case TRUNCATE:
          break;
        case ZERO_EXTEND:
          break;
        case SIGN_EXTEND:
          break;
        case LTGT:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case UNLT:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case UNLE:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case UNGT:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case UNGE:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case UNEQ:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case ORDERED:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case UNORDERED:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case LTU:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case LEU:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case GTU:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case GEU:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case LT:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case LE:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case GT:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case GE:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case EQ:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case NE:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case POST_MODIFY:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case PRE_MODIFY:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case POST_INC:
          break;
        case POST_DEC:
          break;
        case PRE_INC:
          break;
        case PRE_DEC:
          break;
        case UMAX:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case UMIN:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case SMAX:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case SMIN:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case ROTATERT:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case LSHIFTRT:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case ASHIFTRT:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case ROTATE:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case ASHIFT:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case NOT:
          break;
        case XOR:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case IOR:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case AND:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case UMOD:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case UDIV:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case MOD:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case DIV:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case MULT:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case NEG:
          break;
        case MINUS:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case PLUS:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case COMPARE:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case COND:
          gt_ggc_m_9rtvec_def ((*x).fld[0].rtvec);
          break;
        case IF_THEN_ELSE:
          gt_ggc_m_7rtx_def ((*x).fld[2].rtx);
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case QUEUED:
          gt_ggc_m_7rtx_def ((*x).fld[4].rtx);
          gt_ggc_m_7rtx_def ((*x).fld[3].rtx);
          gt_ggc_m_7rtx_def ((*x).fld[2].rtx);
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case ADDRESSOF:
          gt_ggc_m_9tree_node ((*x).fld[2].rttree);
          break;
        case CC0:
          break;
        case SYMBOL_REF:
          break;
        case LABEL_REF:
          gt_ggc_m_7rtx_def ((*x).fld[2].rtx);
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case MEM:
          gt_ggc_m_9mem_attrs ((*x).fld[1].rtmem);
          break;
        case CONCAT:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case STRICT_LOW_PART:
          break;
        case SUBREG:
          break;
        case SCRATCH:
          break;
        case REG:
          break;
        case VALUE:
          break;
        case PC:
          break;
        case CONST:
          break;
        case CONST_STRING:
          break;
        case CONST_VECTOR:
          gt_ggc_m_9rtvec_def ((*x).fld[0].rtvec);
          break;
        case CONST_DOUBLE:
          break;
        case CONST_INT:
          break;
        case RESX:
          break;
        case TRAP_IF:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case RETURN:
          break;
        case CALL:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case CLOBBER:
          break;
        case USE:
          break;
        case SET:
          gt_ggc_m_7rtx_def ((*x).fld[0].rtx);
          break;
        case PREFETCH:
          gt_ggc_m_7rtx_def ((*x).fld[2].rtx);
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case ADDR_DIFF_VEC:
          gt_ggc_m_7rtx_def ((*x).fld[3].rtx);
          gt_ggc_m_7rtx_def ((*x).fld[2].rtx);
          gt_ggc_m_9rtvec_def ((*x).fld[1].rtvec);
          break;
        case ADDR_VEC:
          gt_ggc_m_9rtvec_def ((*x).fld[0].rtvec);
          break;
        case UNSPEC_VOLATILE:
          gt_ggc_m_9rtvec_def ((*x).fld[0].rtvec);
          break;
        case UNSPEC:
          gt_ggc_m_9rtvec_def ((*x).fld[0].rtvec);
          break;
        case ASM_OPERANDS:
          gt_ggc_m_9rtvec_def ((*x).fld[4].rtvec);
          gt_ggc_m_9rtvec_def ((*x).fld[3].rtvec);
          break;
        case ASM_INPUT:
          break;
        case PARALLEL:
          gt_ggc_m_9rtvec_def ((*x).fld[0].rtvec);
          break;
        case COND_EXEC:
          gt_ggc_m_7rtx_def ((*x).fld[0].rtx);
          break;
        case NOTE:
          switch (NOTE_LINE_NUMBER (&(*x)))
            {
            case NOTE_INSN_EXPECTED_VALUE:
              gt_ggc_m_7rtx_def ((*x).fld[4].rtx);
              break;
            case NOTE_INSN_BLOCK_BEG:
              gt_ggc_m_9tree_node ((*x).fld[4].rttree);
              break;
            case NOTE_INSN_BLOCK_END:
              gt_ggc_m_9tree_node ((*x).fld[4].rttree);
              break;
            default:
              break;
            }
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case CODE_LABEL:
          gt_ggc_m_7rtx_def ((*x).fld[5].rtx);
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case BARRIER:
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case CALL_INSN:
          gt_ggc_m_7rtx_def ((*x).fld[9].rtx);
          gt_ggc_m_7rtx_def ((*x).fld[8].rtx);
          gt_ggc_m_7rtx_def ((*x).fld[7].rtx);
          gt_ggc_m_7rtx_def ((*x).fld[5].rtx);
          gt_ggc_m_9tree_node ((*x).fld[4].rttree);
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case JUMP_INSN:
          gt_ggc_m_7rtx_def ((*x).fld[9].rtx);
          gt_ggc_m_7rtx_def ((*x).fld[8].rtx);
          gt_ggc_m_7rtx_def ((*x).fld[7].rtx);
          gt_ggc_m_7rtx_def ((*x).fld[5].rtx);
          gt_ggc_m_9tree_node ((*x).fld[4].rttree);
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case INSN:
          gt_ggc_m_7rtx_def ((*x).fld[8].rtx);
          gt_ggc_m_7rtx_def ((*x).fld[7].rtx);
          gt_ggc_m_7rtx_def ((*x).fld[5].rtx);
          gt_ggc_m_9tree_node ((*x).fld[4].rttree);
          gt_ggc_m_7rtx_def ((*x).fld[1].rtx);
          break;
        case ATTR_FLAG:
          break;
        case EQ_ATTR:
          break;
        case SET_ATTR_ALTERNATIVE:
          gt_ggc_m_9rtvec_def ((*x).fld[1].rtvec);
          break;
        case SET_ATTR:
          break;
        case ATTR:
          break;
        case DEFINE_ATTR:
          break;
        case DEFINE_INSN_RESERVATION:
          break;
        case DEFINE_RESERVATION:
          break;
        case AUTOMATA_OPTION:
          break;
        case DEFINE_AUTOMATON:
          break;
        case DEFINE_BYPASS:
          break;
        case ABSENCE_SET:
          break;
        case PRESENCE_SET:
          break;
        case EXCLUSION_SET:
          break;
        case DEFINE_QUERY_CPU_UNIT:
          break;
        case DEFINE_CPU_UNIT:
          break;
        case ADDRESS:
          break;
        case SEQUENCE:
          gt_ggc_m_9rtvec_def ((*x).fld[0].rtvec);
          break;
        case DEFINE_COND_EXEC:
          gt_ggc_m_9rtvec_def ((*x).fld[0].rtvec);
          break;
        case DEFINE_ASM_ATTRIBUTES:
          gt_ggc_m_9rtvec_def ((*x).fld[0].rtvec);
          break;
        case DEFINE_FUNCTION_UNIT:
          gt_ggc_m_9rtvec_def ((*x).fld[6].rtvec);
          break;
        case DEFINE_DELAY:
          gt_ggc_m_9rtvec_def ((*x).fld[1].rtvec);
          break;
        case DEFINE_EXPAND:
          gt_ggc_m_9rtvec_def ((*x).fld[1].rtvec);
          break;
        case DEFINE_COMBINE:
          gt_ggc_m_9rtvec_def ((*x).fld[0].rtvec);
          break;
        case DEFINE_PEEPHOLE2:
          gt_ggc_m_9rtvec_def ((*x).fld[2].rtvec);
          gt_ggc_m_9rtvec_def ((*x).fld[0].rtvec);
          break;
        case DEFINE_INSN_AND_SPLIT:
          gt_ggc_m_9rtvec_def ((*x).fld[7].rtvec);
          gt_ggc_m_9rtvec_def ((*x).fld[5].rtvec);
          gt_ggc_m_9rtvec_def ((*x).fld[1].rtvec);
          break;
        case DEFINE_SPLIT:
          gt_ggc_m_9rtvec_def ((*x).fld[2].rtvec);
          gt_ggc_m_9rtvec_def ((*x).fld[0].rtvec);
          break;
        case DEFINE_PEEPHOLE:
          gt_ggc_m_9rtvec_def ((*x).fld[3].rtvec);
          gt_ggc_m_9rtvec_def ((*x).fld[0].rtvec);
          break;
        case DEFINE_INSN:
          gt_ggc_m_9rtvec_def ((*x).fld[4].rtvec);
          gt_ggc_m_9rtvec_def ((*x).fld[1].rtvec);
          break;
        case MATCH_INSN:
          break;
        case MATCH_PAR_DUP:
          gt_ggc_m_9rtvec_def ((*x).fld[1].rtvec);
          break;
        case MATCH_OP_DUP:
          gt_ggc_m_9rtvec_def ((*x).fld[1].rtvec);
          break;
        case MATCH_PARALLEL:
          gt_ggc_m_9rtvec_def ((*x).fld[2].rtvec);
          break;
        case MATCH_OPERATOR:
          gt_ggc_m_9rtvec_def ((*x).fld[2].rtvec);
          break;
        case MATCH_DUP:
          break;
        case MATCH_SCRATCH:
          break;
        case MATCH_OPERAND:
          break;
        case INSN_LIST:
          gt_ggc_m_7rtx_def ((*x).fld[0].rtx);
          break;
        case EXPR_LIST:
          gt_ggc_m_7rtx_def ((*x).fld[0].rtx);
          break;
        case INCLUDE:
          break;
        case NIL:
          break;
        case UNKNOWN:
          break;
        default:
          break;
        }
      x = (RTX_NEXT (&(*x)));
  }
}

void
gt_ggc_m_P9tree_node4htab (x_p)
      void *x_p;
{
  struct htab * const x = (struct htab *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      if ((*x).entries != NULL) {
        size_t i1;
        ggc_set_mark ((*x).entries);
        for (i1 = 0; i1 < (size_t)(((*x)).size); i1++) {
          gt_ggc_m_9tree_node ((*x).entries[i1]);
        }
      }
  }
}

void
gt_ggc_m_P9mem_attrs4htab (x_p)
      void *x_p;
{
  struct htab * const x = (struct htab *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      if ((*x).entries != NULL) {
        size_t i1;
        ggc_set_mark ((*x).entries);
        for (i1 = 0; i1 < (size_t)(((*x)).size); i1++) {
          gt_ggc_m_9mem_attrs ((*x).entries[i1]);
        }
      }
  }
}

void
gt_ggc_m_P7rtx_def4htab (x_p)
      void *x_p;
{
  struct htab * const x = (struct htab *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      if ((*x).entries != NULL) {
        size_t i1;
        ggc_set_mark ((*x).entries);
        for (i1 = 0; i1 < (size_t)(((*x)).size); i1++) {
          gt_ggc_m_7rtx_def ((*x).entries[i1]);
        }
      }
  }
}

void
gt_ggc_m_P17cselib_val_struct4htab (x_p)
      void *x_p;
{
  struct htab * const x = (struct htab *)x_p;
  if (ggc_test_and_set_mark (x))
    {
      if ((*x).entries != NULL) {
        size_t i1;
        ggc_set_mark ((*x).entries);
        for (i1 = 0; i1 < (size_t)(((*x)).size); i1++) {
          gt_ggc_m_17cselib_val_struct ((*x).entries[i1]);
        }
      }
  }
}

/* GC roots.  */

const struct ggc_root_tab gt_ggc_r_gtype_desc_c[] = {
  {
    &tail_recursion_label_list,
    1,
    sizeof (tail_recursion_label_list),
    &gt_ggc_mx_rtx_def

  },
  {
    &label_value_list,
    1,
    sizeof (label_value_list),
    &gt_ggc_mx_rtx_def

  },
  {
    &insn_addresses_,
    1,
    sizeof (insn_addresses_),
    &gt_ggc_mx_varray_head_tag

  },
  {
    &ssa_definition,
    1,
    sizeof (ssa_definition),
    &gt_ggc_mx_varray_head_tag

  },
  {
    &libfunc_table[0],
    1 * (LTI_MAX),
    sizeof (libfunc_table[0]),
    &gt_ggc_mx_rtx_def

  },
  {
    &current_function_func_begin_label,
    1,
    sizeof (current_function_func_begin_label),
    &gt_ggc_mx_tree_node

  },
  {
    &current_function_decl,
    1,
    sizeof (current_function_decl),
    &gt_ggc_mx_tree_node

  },
  {
    &sizetype_tab[0],
    1 * ((int) TYPE_KIND_LAST),
    sizeof (sizetype_tab[0]),
    &gt_ggc_mx_tree_node

  },
  {
    &integer_types[0],
    1 * (itk_none),
    sizeof (integer_types[0]),
    &gt_ggc_mx_tree_node

  },
  {
    &global_trees[0],
    1 * (TI_MAX),
    sizeof (global_trees[0]),
    &gt_ggc_mx_tree_node

  },
  {
    &optab_table[0],
    1 * (OTI_MAX),
    sizeof (optab_table[0]),
    &gt_ggc_mx_optab

  },
  {
    &stack_limit_rtx,
    1,
    sizeof (stack_limit_rtx),
    &gt_ggc_mx_rtx_def

  },
  {
    &return_address_pointer_rtx,
    1,
    sizeof (return_address_pointer_rtx),
    &gt_ggc_mx_rtx_def

  },
  {
    &static_chain_incoming_rtx,
    1,
    sizeof (static_chain_incoming_rtx),
    &gt_ggc_mx_rtx_def

  },
  {
    &static_chain_rtx,
    1,
    sizeof (static_chain_rtx),
    &gt_ggc_mx_rtx_def

  },
  {
    &struct_value_incoming_rtx,
    1,
    sizeof (struct_value_incoming_rtx),
    &gt_ggc_mx_rtx_def

  },
  {
    &struct_value_rtx,
    1,
    sizeof (struct_value_rtx),
    &gt_ggc_mx_rtx_def

  },
  {
    &pic_offset_table_rtx,
    1,
    sizeof (pic_offset_table_rtx),
    &gt_ggc_mx_rtx_def

  },
  {
    &global_rtl[0],
    1 * (GR_MAX),
    sizeof (global_rtl[0]),
    &gt_ggc_mx_rtx_def

  },
  {
    &const_tiny_rtx[0][0],
    1 * (3) * ((int) MAX_MACHINE_MODE),
    sizeof (const_tiny_rtx[0][0]),
    &gt_ggc_mx_rtx_def

  },
  {
    &const_true_rtx,
    1,
    sizeof (const_true_rtx),
    &gt_ggc_mx_rtx_def

  },
  {
    &const_int_rtx[0],
    1 * (MAX_SAVED_CONST_INT * 2 + 1),
    sizeof (const_int_rtx[0]),
    &gt_ggc_mx_rtx_def

  },
  {
    &cfun,
    1,
    sizeof (cfun),
    &gt_ggc_mx_function

  },
  {
    &mips_load_reg4,
    1,
    sizeof (mips_load_reg4),
    &gt_ggc_mx_rtx_def

  },
  {
    &mips_load_reg3,
    1,
    sizeof (mips_load_reg3),
    &gt_ggc_mx_rtx_def

  },
  {
    &mips_load_reg2,
    1,
    sizeof (mips_load_reg2),
    &gt_ggc_mx_rtx_def

  },
  {
    &mips_load_reg,
    1,
    sizeof (mips_load_reg),
    &gt_ggc_mx_rtx_def

  },
  {
    &branch_cmp[0],
    1 * (2),
    sizeof (branch_cmp[0]),
    &gt_ggc_mx_rtx_def

  },
  LAST_GGC_ROOT_TAB
};


/* Used to implement the RTX_NEXT macro.  */
const unsigned char rtx_next[NUM_RTX_CODE] = {
  0,
  0,
  0,
  offsetof (struct rtx_def, fld) + 1 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 1 * sizeof (rtunion),
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 3 * sizeof (rtunion),
  0,
  0,
  0,
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  offsetof (struct rtx_def, fld) + 2 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 2 * sizeof (rtunion),
  0,
  0,
  0,
  0,
  0,
  offsetof (struct rtx_def, fld) + 2 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 2 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 2 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 2 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 2 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 2 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 1 * sizeof (rtunion),
  0,
  0,
  0,
  0,
  0,
  0,
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 1 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  0,
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  0,
  0,
  0,
  0,
  0,
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  0,
  0,
  0,
  0,
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  0,
  0,
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 1 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  0,
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  0,
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  offsetof (struct rtx_def, fld) + 0 * sizeof (rtunion),
  0,
};
