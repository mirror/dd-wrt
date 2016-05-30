/* src/vm/stackmap.hpp - class attribute StackMapTable

   Copyright (C) 2006, 2007, 2008
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/


#ifndef STACKMAP_HPP_
#define STACKMAP_HPP_ 1

/* forward typedefs ***********************************************************/

typedef struct stack_map_t                       stack_map_t;
typedef union  stack_map_frame_t                 stack_map_frame_t;
typedef struct same_locals_1_stack_item_frame_t  same_locals_1_stack_item_frame_t;
typedef struct same_locals_1_stack_item_frame_extended_t same_locals_1_stack_item_frame_extended_t;
typedef struct chop_frame_t                      chop_frame_t;
typedef struct same_frame_extended_t             same_frame_extended_t;
typedef struct append_frame_t                    append_frame_t;
typedef struct full_frame_t                      full_frame_t;

typedef union  verification_type_info_t          verification_type_info_t;
typedef struct Top_variable_info_t	             Top_variable_info_t;
typedef struct Integer_variable_info_t           Integer_variable_info_t;
typedef struct Float_variable_info_t             Float_variable_info_t;
typedef struct Long_variable_info_t              Long_variable_info_t;
typedef struct Double_variable_info_t            Double_variable_info_t;
typedef struct Null_variable_info_t              Null_variable_info_t;
typedef struct UninitializedThis_variable_info_t UninitializedThis_variable_info_t;
typedef struct Object_variable_info_t            Object_variable_info_t;
typedef struct Uninitialized_variable_info_t     Uninitialized_variable_info_t;


#include "config.h"
#include "vm/types.hpp"

struct methodinfo;

namespace cacao { struct ClassBuffer; }

/* verification_type_info *****************************************************/

#define ITEM_Top                  0
#define ITEM_Integer              1
#define ITEM_Float                2
#define ITEM_Double               3
#define ITEM_Long                 4
#define ITEM_Null                 5
#define ITEM_UninitializedThis    6
#define ITEM_Object               7
#define ITEM_Uninitialized        8

struct Top_variable_info_t {
	u1 tag;
};

struct Integer_variable_info_t {
	u1 tag;
};

struct Float_variable_info_t {
	u1 tag;
};

struct Long_variable_info_t {
	u1 tag;
};

struct Double_variable_info_t {
	u1 tag;
};

struct Null_variable_info_t {
	u1 tag;
};

struct UninitializedThis_variable_info_t {
	u1 tag;
};

struct Object_variable_info_t {
	u1 tag;
	u2 cpool_index;
};

struct Uninitialized_variable_info_t {
	u1 tag;
	u2 offset;
};

union verification_type_info_t {
	u1 tag;
	Top_variable_info_t	              Top_variable_info;
	Integer_variable_info_t           Integer_variable_info;
	Float_variable_info_t             Float_variable_info;
	Long_variable_info_t              Long_variable_info;
	Double_variable_info_t            Double_variable_info;
	Null_variable_info_t              Null_variable_info;
	UninitializedThis_variable_info_t UninitializedThis_variable_info;
	Object_variable_info_t            Object_variable_info;
	Uninitialized_variable_info_t     Uninitialized_variable_info;
};


/* stack_map_t ****************************************************************/

struct stack_map_t {
	u2                 attribute_name_index;
	u4                 attribute_length;
	u2                 number_of_entries;
	stack_map_frame_t *entries;
};


/* same_locals_1_stack_item_frame_t *******************************************/

struct same_locals_1_stack_item_frame_t {
	u1                       frame_type;
	verification_type_info_t stack[1];
};


/* same_locals_1_stack_item_frame_extended_t **********************************/

struct same_locals_1_stack_item_frame_extended_t {
	u1                       frame_type;
	u2                       offset_delta;
	verification_type_info_t stack[1];
};


/* chop_frame_t ***************************************************************/

struct chop_frame_t {
	u1 frame_type;
	u2 offset_delta;
};


/* same_frame_extended_t ******************************************************/

struct same_frame_extended_t {
	u1 frame_type;
	u2 offset_delta;
};


/* append_frame_t *************************************************************/

struct append_frame_t {
	u1                        frame_type;
	u2                        offset_delta;
	verification_type_info_t *locals;
};


/* full_frame_t ***************************************************************/

struct full_frame_t {
	u1                        frame_type;
	u2                        offset_delta;
	u2                        number_of_locals;
	verification_type_info_t *locals;
	u2                        number_of_stack_items;
	verification_type_info_t *stack;
};


/* stack_map_frame_t **********************************************************/

#define FRAME_TYPE_SAME                                 63   /* 0-63          */
#define FRAME_TYPE_SAME_LOCALS_1_STACK_ITEM             127  /* 0-127         */
#define FRAME_TYPE_RESERVED                             246  /* 128-246       */
#define FRAME_TYPE_SAME_LOCALS_1_STACK_ITEM_EXTENDED    247  /* 247           */
#define FRAME_TYPE_CHOP                                 250  /* 248-250       */
#define FRAME_TYPE_SAME_FRAME_EXTENDED                  251  /* 251           */
#define FRAME_TYPE_APPEND                               254  /* 252-254       */
#define FRAME_TYPE_FULL_FRAME                           255  /* 255           */

union stack_map_frame_t {
	u1                                        frame_type;
	same_locals_1_stack_item_frame_t          same_locals_1_stack_item_frame;
	same_locals_1_stack_item_frame_extended_t same_locals_1_stack_item_frame_extended;
	chop_frame_t                              chop_frame;
	same_frame_extended_t                     same_frame_extended;
	append_frame_t                            append_frame;
	full_frame_t                              full_frame;
};

/* function prototypes ********************************************************/

bool stackmap_load_attribute_stackmaptable(cacao::ClassBuffer& cb, methodinfo *m);

#endif // STACKMAP_HPP_


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
