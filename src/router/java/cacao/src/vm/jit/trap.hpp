/* src/vm/jit/trap.hpp - hardware traps

   Copyright (C) 2008-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
   Copyright (C) 2009 Theobroma Systems Ltd.

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


#ifndef TRAP_HPP_
#define TRAP_HPP_ 1

#include "config.h"

#include <stdint.h>

#include "vm/options.hpp"

#include "vm/jit/executionstate.hpp"


/**
 * Contains information about a decoded trap instruction.
 */
typedef struct trapinfo_t {
	int      type;   ///< Specific trap type (see md-trap.h).
	intptr_t value;  ///< Value (numeric or address) passed with the trap.
} trapinfo_t;


/**
 * Trap signal number defines. Use these instead of the signal
 * numbers provided by your specific OS.
 */
enum {
	TRAP_SIGRESERVED = 0,
	TRAP_SIGSEGV     = 1,
	TRAP_SIGILL      = 2,
	TRAP_SIGTRAP     = 3,
	TRAP_SIGFPE      = 4,
	TRAP_SIGEND
};

/* Include machine dependent trap stuff. */

#include "md-trap.hpp"


/* function prototypes ********************************************************/

void trap_init(void);

void trap_handle(int sig, void* xpc, void* context);

bool md_trap_decode(trapinfo_t* trp, int sig, void* xpc, executionstate_t* es);

#endif // TRAP_HPP_


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
