/* src/vm/jit/show.hpp - showing the intermediate representation

   Copyright (C) 1996-2005, 2006, 2008
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


#ifndef _SHOW_HPP
#define _SHOW_HPP

#include "config.h"                     // for ENABLE_DEBUG_FILTER
#include "vm/types.hpp"                 // for s4

struct basicblock;
struct instruction;
struct jitdata;
struct methodinfo;

/* compiler stage defines *****************************************************/

#define SHOW_INSTRUCTIONS  0
#define SHOW_PARSE         1
#define SHOW_STACK         2
#define SHOW_CFG           3
#define SHOW_REGS          4
#define SHOW_CODE          5


/* function prototypes ********************************************************/

#if !defined(NDEBUG)
extern const char *show_jit_type_names[];
extern const char  show_jit_type_letters[];

bool show_init(void);

void show_method(jitdata *jd, int stage);
void show_basicblock(jitdata *jd, basicblock *bptr, int stage);
void show_icmd(jitdata *jd, instruction *iptr, bool deadcode, int stage);
void show_variable(jitdata *jd, s4 index, int stage);
void show_variable_array(jitdata *jd, s4 *vars, int n, int stage);
void show_javalocals_array(jitdata *jd, s4 *vars, int n, int stage);
void show_allocation(s4 type, s4 flags, s4 regoff);
#endif /* !defined(NDEBUG) */

/* Debug output filtering */

#if defined(ENABLE_DEBUG_FILTER)
void show_filters_init(void);
#define SHOW_FILTER_FLAG_VERBOSECALL_INCLUDE 0x01
#define SHOW_FILTER_FLAG_VERBOSECALL_EXCLUDE 0x02
#define SHOW_FILTER_FLAG_SHOW_METHOD 0x04
void show_filters_apply(methodinfo *m);
int show_filters_test_verbosecall_enter(methodinfo *m);
int show_filters_test_verbosecall_exit(methodinfo *m);
#endif

#endif // _SHOW_HPP


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
