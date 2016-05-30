/* src/vm/jit/dseg.hpp - data segment handling stuff

   Copyright (C) 1996-2005, 2006, 2007, 2008
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


#ifndef DSEG_HPP_
#define DSEG_HPP_ 1

#include "config.h"
#include "vm/global.hpp"
#include "vm/types.hpp"

struct basicblock;
struct codegendata;
struct jitdata;

/* convenience macros *********************************************************/

#define dseg_add_functionptr(cd,value) \
    dseg_add_address((cd), (void *) (ptrint) (value))


/* dataentry ******************************************************************/

#define DSEG_FLAG_UNIQUE      0x0001
#define DSEG_FLAG_READONLY    0x0002

struct dsegentry {
	u2         type;
	u2         flags;
	s4         disp;
	imm_union  val;
	dsegentry *next;
};


/* function prototypes ********************************************************/

void dseg_finish(jitdata *jd);

s4 dseg_add_unique_s4(codegendata *cd, s4 value);
s4 dseg_add_unique_s8(codegendata *cd, s8 value);
s4 dseg_add_unique_float(codegendata *cd, float value);
s4 dseg_add_unique_double(codegendata *cd, double value);
s4 dseg_add_unique_address(codegendata *cd, void *value);

s4 dseg_add_s4(codegendata *cd, s4 value);
s4 dseg_add_s8(codegendata *cd, s8 value);
// TODO: something expects this to be declared with C linkage, why?
extern "C" s4 dseg_add_float(codegendata *cd, float value);
s4 dseg_add_double(codegendata *cd, double value);
s4 dseg_add_address(codegendata *cd, void *value);

void dseg_add_unique_target(codegendata *cd, basicblock *target);
void dseg_add_target(codegendata *cd, basicblock *target);

#if defined(__I386__) || defined(__X86_64__) || defined(__XDSPCORE__) || defined(ENABLE_INTRP)
void dseg_adddata(codegendata *cd);
void dseg_resolve_datareferences(jitdata *jd);
#endif

#if !defined(NDEBUG)
void dseg_display(jitdata *jd);
#endif


#endif // DSEG_HPP_


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
