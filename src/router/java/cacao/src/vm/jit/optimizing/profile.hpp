/* src/vm/jit/optimizing/profile.hpp - runtime profiling

   Copyright (C) 1996-2005, 2006, 2008, 2009
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


#ifndef _PROFILE_HPP
#define _PROFILE_HPP

#include "config.h"

#include "vm/types.hpp"

#include "vm/global.hpp"


/* CPU cycle counting macros **************************************************/

#if defined(ENABLE_PROFILING)

#define PROFILE_CYCLE_START \
	do { \
		if (JITDATA_HAS_FLAG_INSTRUMENT(jd)) { \
			emit_profile_cycle_start(cd, code); \
		} \
	} while (0)

#define PROFILE_CYCLE_STOP \
	do { \
		if (JITDATA_HAS_FLAG_INSTRUMENT(jd)) { \
			emit_profile_cycle_stop(cd, code); \
		} \
	} while (0)

#else

#define PROFILE_CYCLE_START  /* nop */
#define PROFILE_CYCLE_STOP   /* nop */

#endif


/* function prototypes ********************************************************/

bool profile_init(void);
bool profile_start_thread(void);

#if !defined(NDEBUG)
void profile_printstats(void);
#endif

#endif /* _PROFILE_HPP */


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
 */
