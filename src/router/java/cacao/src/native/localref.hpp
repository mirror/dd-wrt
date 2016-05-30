/* src/native/localref.hpp - Management of local reference tables

   Copyright (C) 1996-2005, 2006, 2007, 2008, 2010
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


#ifndef _LOCALREF_HPP
#define _LOCALREF_HPP

/* forward typedefs ***********************************************************/

typedef struct localref_table localref_table;

#include "config.h"

#include "vm/types.hpp"

#include "vm/global.hpp"

struct methodinfo;

/* localref_table **************************************************************

   ATTENTION: keep this structure a multiple of 8-bytes!!! This is
   essential for the native stub on 64-bit architectures.

*******************************************************************************/

#define LOCALREFTABLE_CAPACITY    16

struct localref_table {
	s4                 capacity;        /* table size                         */
	s4                 used;            /* currently used references          */
	s4                 firstfree;       /* head of the free list              */
	s4                 hwm;             /* high water mark                    */
	s4                 localframes;     /* number of current frames           */
	s4                 PADDING;         /* 8-byte padding                     */
	localref_table    *prev;            /* link to prev table (LocalFrame)    */
	union {
		java_object_t *ptr;
		s4 nextfree;
	} refs[LOCALREFTABLE_CAPACITY];     /* references            */
};

/* function prototypes ********************************************************/

bool localref_table_init(void);
bool localref_table_destroy(void);
void localref_table_add(localref_table *lrt);
void localref_table_remove();

bool localref_frame_push(int32_t capacity);
void localref_frame_pop_all(void);

java_handle_t *localref_add(java_object_t *o);
void           localref_del(java_handle_t *localref);

void localref_native_enter(methodinfo *m, uint64_t *argument_regs, uint64_t *argument_stack);
void localref_native_exit(methodinfo *m, uint64_t *return_regs);

#if !defined(NDEBUG)
void localref_dump(void);
#endif

#endif // _LOCALREF_HPP


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
