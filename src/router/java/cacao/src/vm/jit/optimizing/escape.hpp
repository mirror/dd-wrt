/* src/vm/optimizing/escape.hpp

   Copyright (C) 2008-2013
   CACAOVM - Verein zu Foerderung der freien virtuellen Machine CACAO

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

#ifndef ESCAPE_HPP_
#define ESCAPE_HPP_

#include "vm/jit/jit.hpp"

struct methodinfo;

typedef enum {
	ESCAPE_UNKNOWN,
	ESCAPE_NONE,
	ESCAPE_METHOD,
	ESCAPE_METHOD_RETURN,
	ESCAPE_GLOBAL
} escape_state_t;

static inline escape_state_t escape_state_from_u1(u1 x) {
	return (escape_state_t)(x & ~0x80);
}

static inline u1 escape_state_to_u1(escape_state_t x) {
	return (u1)x;
}

void escape_analysis_perform(jitdata *jd);

void escape_analysis_escape_check(void *vp);

void bc_escape_analysis_perform(methodinfo *m);

bool escape_is_monomorphic(methodinfo *caller, methodinfo *callee);

#endif // ESCAPE_HPP_


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
