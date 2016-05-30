/* src/vm/jit/stack.hpp - stack analysis header

   Copyright (C) 1996-2014
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


#ifndef STACK_HPP_
#define STACK_HPP_ 1

#include "config.h"                     // for ENABLE_VERIFIER
#include <stdint.h>
#include "vm/global.hpp"                // for Type

struct instruction;
struct jitdata;

/* stack element structure ****************************************************/

enum VariableFlag {
	SAVEDVAR    =   1,   // variable has to survive method invocations
	INMEMORY    =   2,   // variable stored in memory
	SAVREG      =   4,   // allocated to a saved register
	ARGREG      =   8,   // allocated to an arg register
	PASSTHROUGH =  32,   // stackslot was passed-through by an ICMD
	PREALLOC    =  64,   // preallocated var like for ARGVARS.
	                     // Used with the new var system
	INOUT       = 128    // variable is an invar or/and an outvar
};

// check flags
static inline bool IS_SAVEDVAR(s4 flags) { return flags & SAVEDVAR; }
static inline bool IS_INMEMORY(s4 flags) { return flags & INMEMORY; }


enum VariableKind {
	UNDEFVAR = 0,   // stack slot will become temp during regalloc
	TEMPVAR  = 1,   // stack slot is temp register
	STACKVAR = 2,   // stack slot is numbered stack slot
	LOCALVAR = 3,   // stack slot is local variable
	ARGVAR   = 4    // stack slot is argument variable
};

/* variable kinds */
struct stackelement_t {
	stackelement_t *prev;       /* pointer to next element towards bottom     */
	instruction    *creator;    /* instruction that created this element      */
	Type            type;       /* slot type of stack element                 */
	int32_t         flags;      /* flags (SAVED, INMEMORY)                    */
	VariableKind    varkind;    /* kind of variable or register               */
	int32_t         varnum;     /* number of variable                         */
};


/* function prototypes ********************************************************/

bool stack_init(void);

bool stack_analyse(jitdata *jd);

void stack_javalocals_store(instruction *iptr, int32_t *javalocals);

#endif // STACK_HPP_


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
