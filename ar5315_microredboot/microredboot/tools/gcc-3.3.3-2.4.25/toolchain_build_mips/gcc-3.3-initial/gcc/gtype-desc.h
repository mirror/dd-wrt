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

/* Enumeration of types known.  */
enum gt_types_enum {
 gt_ggc_e_11align_stack, 
 gt_ggc_e_13binding_level, 
 gt_ggc_e_15deferred_string, 
 gt_ggc_e_24constant_descriptor_tree, 
 gt_ggc_e_13pool_constant, 
 gt_ggc_e_23constant_descriptor_rtx, 
 gt_ggc_e_9type_hash, 
 gt_ggc_e_10goto_fixup, 
 gt_ggc_e_11label_chain, 
 gt_ggc_e_7nesting, 
 gt_ggc_e_9case_node, 
 gt_ggc_e_9eh_region, 
 gt_ggc_e_13ehl_map_entry, 
 gt_ggc_e_12elt_loc_list, 
 gt_ggc_e_17cselib_val_struct, 
 gt_ggc_e_15varray_head_tag, 
 gt_ggc_e_8elt_list, 
 gt_ggc_e_12reg_info_def, 
 gt_ggc_e_14lang_tree_node, 
 gt_ggc_e_9lang_decl, 
 gt_ggc_e_9lang_type, 
 gt_ggc_e_10die_struct, 
 gt_ggc_e_10real_value, 
 gt_ggc_e_5optab, 
 gt_ggc_e_15basic_block_def, 
 gt_ggc_e_9mem_attrs, 
 gt_ggc_e_17language_function, 
 gt_ggc_e_16machine_function, 
 gt_ggc_e_9temp_slot, 
 gt_ggc_e_20initial_value_struct, 
 gt_ggc_e_13varasm_status, 
 gt_ggc_e_11stmt_status, 
 gt_ggc_e_9eh_status, 
 gt_ggc_e_8function, 
 gt_ggc_e_11expr_status, 
 gt_ggc_e_11emit_status, 
 gt_ggc_e_14sequence_stack, 
 gt_ggc_e_14var_refs_queue, 
 gt_ggc_e_15bitmap_head_def, 
 gt_ggc_e_18bitmap_element_def, 
 gt_ggc_e_9tree_node, 
 gt_ggc_e_9rtvec_def, 
 gt_ggc_e_7rtx_def, 
 gt_e_P15deferred_string4htab, 
 gt_e_P9type_hash4htab, 
 gt_e_P13ehl_map_entry4htab, 
 gt_e_P9tree_node4htab, 
 gt_e_P9mem_attrs4htab, 
 gt_e_P7rtx_def4htab, 
 gt_e_P17cselib_val_struct4htab, 
 gt_types_enum_last
};

/* GC marker procedures.  */
#define gt_ggc_m_11align_stack(X) do { \
  if (X != NULL) gt_ggc_mx_align_stack (X);\
  } while (0)
extern void gt_ggc_mx_align_stack PARAMS ((void *));
#define gt_ggc_m_13binding_level(X) do { \
  if (X != NULL) gt_ggc_mx_binding_level (X);\
  } while (0)
extern void gt_ggc_mx_binding_level PARAMS ((void *));
#define gt_ggc_m_15deferred_string(X) do { \
  if (X != NULL) gt_ggc_mx_deferred_string (X);\
  } while (0)
extern void gt_ggc_mx_deferred_string PARAMS ((void *));
#define gt_ggc_m_24constant_descriptor_tree(X) do { \
  if (X != NULL) gt_ggc_mx_constant_descriptor_tree (X);\
  } while (0)
extern void gt_ggc_mx_constant_descriptor_tree PARAMS ((void *));
#define gt_ggc_m_13pool_constant(X) do { \
  if (X != NULL) gt_ggc_mx_pool_constant (X);\
  } while (0)
extern void gt_ggc_mx_pool_constant PARAMS ((void *));
#define gt_ggc_m_23constant_descriptor_rtx(X) do { \
  if (X != NULL) gt_ggc_mx_constant_descriptor_rtx (X);\
  } while (0)
extern void gt_ggc_mx_constant_descriptor_rtx PARAMS ((void *));
#define gt_ggc_m_9type_hash(X) do { \
  if (X != NULL) gt_ggc_mx_type_hash (X);\
  } while (0)
extern void gt_ggc_mx_type_hash PARAMS ((void *));
#define gt_ggc_m_10goto_fixup(X) do { \
  if (X != NULL) gt_ggc_mx_goto_fixup (X);\
  } while (0)
extern void gt_ggc_mx_goto_fixup PARAMS ((void *));
#define gt_ggc_m_11label_chain(X) do { \
  if (X != NULL) gt_ggc_mx_label_chain (X);\
  } while (0)
extern void gt_ggc_mx_label_chain PARAMS ((void *));
#define gt_ggc_m_7nesting(X) do { \
  if (X != NULL) gt_ggc_mx_nesting (X);\
  } while (0)
extern void gt_ggc_mx_nesting PARAMS ((void *));
#define gt_ggc_m_9case_node(X) do { \
  if (X != NULL) gt_ggc_mx_case_node (X);\
  } while (0)
extern void gt_ggc_mx_case_node PARAMS ((void *));
#define gt_ggc_m_9eh_region(X) do { \
  if (X != NULL) gt_ggc_mx_eh_region (X);\
  } while (0)
extern void gt_ggc_mx_eh_region PARAMS ((void *));
#define gt_ggc_m_13ehl_map_entry(X) do { \
  if (X != NULL) gt_ggc_mx_ehl_map_entry (X);\
  } while (0)
extern void gt_ggc_mx_ehl_map_entry PARAMS ((void *));
#define gt_ggc_m_12elt_loc_list(X) do { \
  if (X != NULL) gt_ggc_mx_elt_loc_list (X);\
  } while (0)
extern void gt_ggc_mx_elt_loc_list PARAMS ((void *));
#define gt_ggc_m_17cselib_val_struct(X) do { \
  if (X != NULL) gt_ggc_mx_cselib_val_struct (X);\
  } while (0)
extern void gt_ggc_mx_cselib_val_struct PARAMS ((void *));
#define gt_ggc_m_15varray_head_tag(X) do { \
  if (X != NULL) gt_ggc_mx_varray_head_tag (X);\
  } while (0)
extern void gt_ggc_mx_varray_head_tag PARAMS ((void *));
#define gt_ggc_m_8elt_list(X) do { \
  if (X != NULL) gt_ggc_mx_elt_list (X);\
  } while (0)
extern void gt_ggc_mx_elt_list PARAMS ((void *));
#define gt_ggc_m_12reg_info_def(X) do { \
  if (X != NULL) gt_ggc_mx_reg_info_def (X);\
  } while (0)
extern void gt_ggc_mx_reg_info_def PARAMS ((void *));
#define gt_ggc_m_14lang_tree_node(X) do { \
  if (X != NULL) gt_ggc_mx_lang_tree_node (X);\
  } while (0)
extern void gt_ggc_mx_lang_tree_node PARAMS ((void *));
#define gt_ggc_m_9lang_decl(X) do { \
  if (X != NULL) gt_ggc_mx_lang_decl (X);\
  } while (0)
extern void gt_ggc_mx_lang_decl PARAMS ((void *));
#define gt_ggc_m_9lang_type(X) do { \
  if (X != NULL) gt_ggc_mx_lang_type (X);\
  } while (0)
extern void gt_ggc_mx_lang_type PARAMS ((void *));
#define gt_ggc_m_10die_struct(X) do { \
  if (X != NULL) gt_ggc_mx_die_struct (X);\
  } while (0)
extern void gt_ggc_mx_die_struct PARAMS ((void *));
#define gt_ggc_m_10real_value(X) do { \
  if (X != NULL) gt_ggc_mx_real_value (X);\
  } while (0)
extern void gt_ggc_mx_real_value PARAMS ((void *));
#define gt_ggc_m_5optab(X) do { \
  if (X != NULL) gt_ggc_mx_optab (X);\
  } while (0)
extern void gt_ggc_mx_optab PARAMS ((void *));
#define gt_ggc_m_15basic_block_def(X) do { \
  if (X != NULL) gt_ggc_mx_basic_block_def (X);\
  } while (0)
extern void gt_ggc_mx_basic_block_def PARAMS ((void *));
#define gt_ggc_m_9mem_attrs(X) do { \
  if (X != NULL) gt_ggc_mx_mem_attrs (X);\
  } while (0)
extern void gt_ggc_mx_mem_attrs PARAMS ((void *));
#define gt_ggc_m_17language_function(X) do { \
  if (X != NULL) gt_ggc_mx_language_function (X);\
  } while (0)
extern void gt_ggc_mx_language_function PARAMS ((void *));
#define gt_ggc_m_16machine_function(X) do { \
  if (X != NULL) gt_ggc_mx_machine_function (X);\
  } while (0)
extern void gt_ggc_mx_machine_function PARAMS ((void *));
#define gt_ggc_m_9temp_slot(X) do { \
  if (X != NULL) gt_ggc_mx_temp_slot (X);\
  } while (0)
extern void gt_ggc_mx_temp_slot PARAMS ((void *));
#define gt_ggc_m_20initial_value_struct(X) do { \
  if (X != NULL) gt_ggc_mx_initial_value_struct (X);\
  } while (0)
extern void gt_ggc_mx_initial_value_struct PARAMS ((void *));
#define gt_ggc_m_13varasm_status(X) do { \
  if (X != NULL) gt_ggc_mx_varasm_status (X);\
  } while (0)
extern void gt_ggc_mx_varasm_status PARAMS ((void *));
#define gt_ggc_m_11stmt_status(X) do { \
  if (X != NULL) gt_ggc_mx_stmt_status (X);\
  } while (0)
extern void gt_ggc_mx_stmt_status PARAMS ((void *));
#define gt_ggc_m_9eh_status(X) do { \
  if (X != NULL) gt_ggc_mx_eh_status (X);\
  } while (0)
extern void gt_ggc_mx_eh_status PARAMS ((void *));
#define gt_ggc_m_8function(X) do { \
  if (X != NULL) gt_ggc_mx_function (X);\
  } while (0)
extern void gt_ggc_mx_function PARAMS ((void *));
#define gt_ggc_m_11expr_status(X) do { \
  if (X != NULL) gt_ggc_mx_expr_status (X);\
  } while (0)
extern void gt_ggc_mx_expr_status PARAMS ((void *));
#define gt_ggc_m_11emit_status(X) do { \
  if (X != NULL) gt_ggc_mx_emit_status (X);\
  } while (0)
extern void gt_ggc_mx_emit_status PARAMS ((void *));
#define gt_ggc_m_14sequence_stack(X) do { \
  if (X != NULL) gt_ggc_mx_sequence_stack (X);\
  } while (0)
extern void gt_ggc_mx_sequence_stack PARAMS ((void *));
#define gt_ggc_m_14var_refs_queue(X) do { \
  if (X != NULL) gt_ggc_mx_var_refs_queue (X);\
  } while (0)
extern void gt_ggc_mx_var_refs_queue PARAMS ((void *));
#define gt_ggc_m_15bitmap_head_def(X) do { \
  if (X != NULL) gt_ggc_mx_bitmap_head_def (X);\
  } while (0)
extern void gt_ggc_mx_bitmap_head_def PARAMS ((void *));
#define gt_ggc_m_18bitmap_element_def(X) do { \
  if (X != NULL) gt_ggc_mx_bitmap_element_def (X);\
  } while (0)
extern void gt_ggc_mx_bitmap_element_def PARAMS ((void *));
#define gt_ggc_m_9tree_node(X) do { \
  if (X != NULL) gt_ggc_mx_tree_node (X);\
  } while (0)
#define gt_ggc_mx_tree_node gt_ggc_mx_lang_tree_node
#define gt_ggc_m_9rtvec_def(X) do { \
  if (X != NULL) gt_ggc_mx_rtvec_def (X);\
  } while (0)
extern void gt_ggc_mx_rtvec_def PARAMS ((void *));
#define gt_ggc_m_7rtx_def(X) do { \
  if (X != NULL) gt_ggc_mx_rtx_def (X);\
  } while (0)
extern void gt_ggc_mx_rtx_def PARAMS ((void *));
extern void gt_ggc_m_P15deferred_string4htab PARAMS ((void *));
extern void gt_ggc_m_P9type_hash4htab PARAMS ((void *));
extern void gt_ggc_m_P13ehl_map_entry4htab PARAMS ((void *));
extern void gt_ggc_m_P9tree_node4htab PARAMS ((void *));
extern void gt_ggc_m_P9mem_attrs4htab PARAMS ((void *));
extern void gt_ggc_m_P7rtx_def4htab PARAMS ((void *));
extern void gt_ggc_m_P17cselib_val_struct4htab PARAMS ((void *));
