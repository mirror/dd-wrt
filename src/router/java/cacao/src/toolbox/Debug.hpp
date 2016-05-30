/* src/toolbox/Debug.hpp - core debugging facilities

   Copyright (C) 1996-2013
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

#ifndef DEBUG_HPP_
#define DEBUG_HPP_ 1

#include "config.h"
#include <cstring>
#include <cassert>
#include "toolbox/Option.hpp"

#ifdef ENABLE_LOGGING

namespace cacao {

struct Debug {
	/** True if we should print a prefix
	 *
	 * Can be set using the -XX:+DebugPrefix command line flag
	 * @default false
	 */
	static Option<bool> prefix_enabled;

	/** Verbosity level
	 *
	 * Higher number means more details.
	 * Can be set using the -XX:+DebugVerbose command line flag
	 * @default 0
	 */
	static Option<unsigned int> verbose;

	/** True if we should print a the thread id
	 *
	 * Can be set using the -XX:+DebugPrintThread command line flag
	 * @default false
	 */
	static Option<bool> thread_enabled;

	/** The name of system you are interested in debugging
	 *
	 * can be conviently be set via the command line flag -XX:DebugName
	 */
	static Option<const char*> debugname;

	/**
	 * Debugging of a sub system is enabled if it's name is a valid prefix
	 * of the currently set system's name (as set via set_current_system)
	 */
	static bool is_debugging_enabled(const char *system, size_t sz);

	inline static bool is_debugging_enabled(const char *system) {
		return is_debugging_enabled(system, ::std::strlen(system));
	}
};

/**
 * The debug condition.
 *
 * It is used by all DEBUG* and LOG* macros but it can also be used in code
 * for marking a longer block which should be only executed in the debug context.
 *
 * @code
 * if (DEBUG_COND_WITH_NAME_N) {
 *   ...
 * }
 * @endcode
 */
#define DEBUG_COND_WITH_NAME_N(DBG_NAME,VERBOSE)        \
	( (cacao::Debug::verbose >= (VERBOSE) ) &&             \
	  (cacao::Debug::is_debugging_enabled( (DBG_NAME) )) )

/// This macro executes STMT iff debugging of sub system DBG_NAME is enabled
#define DEBUG_WITH_NAME_N(DBG_NAME, VERBOSE, STMT)               \
	do {                                                         \
		if (DEBUG_COND_WITH_NAME_N( (DBG_NAME) , (VERBOSE) )) {  \
			STMT;                                                \
		}                                                        \
	} while (0)

} // end namespace cacao

#else

#define DEBUG_COND_WITH_NAME_N(DBG_NAME,VERBOSE)  false
#define DEBUG_WITH_NAME_N(DBG_NAME, VERBOSE, STMT) do { } while(0)

#endif // end ENABLE_LOGGING


#define DEBUG_COND_WITH_NAME(DBG_NAME) DEBUG_COND_WITH_NAME_N(DBG_NAME,0)
#define DEBUG_COND_N(VERBOSE)          DEBUG_COND_WITH_NAME_N(DEBUG_NAME,VERBOSE)
#define DEBUG_COND                     DEBUG_COND_N(0)

#define DEBUG_WITH_NAME(DBG_NAME, STMT) DEBUG_WITH_NAME_N(DBG_NAME,0, STMT)
#define DEBUG_N(VERBOSE,STMT) DEBUG_WITH_NAME_N(DEBUG_NAME,VERBOSE, STMT)

/** Execute debug statements in your current module.
 *
 * To use this macro you must define the macro DEBUG_NAME to the
 * name of your current module (should be a string literal.
 * Never do this in a header (but if you do this, undef DEBUG_NAME
 * at the end of the header)!
 */
#define DEBUG(STMT) DEBUG_N(0, STMT)
#define DEBUG1(STMT) DEBUG_N(1, STMT)
#define DEBUG2(STMT) DEBUG_N(2, STMT)
#define DEBUG3(STMT) DEBUG_N(3, STMT)

#endif // DEBUG_HPP_

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
