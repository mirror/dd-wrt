/* src/toolbox/logging.hpp - contains logging functions

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


#ifndef LOGGING_HPP_
#define LOGGING_HPP_ 1

#include "config.h"

#include "vm/os.hpp"

#include <stdio.h>
#include <stdarg.h>

#include "vm/utf8.hpp"

#include "toolbox/Debug.hpp"
#include "toolbox/OStream.hpp"

struct classinfo;
struct methodinfo;

namespace cacao {

/// The default destination for logging messages
OStream& dbg();

#ifdef ENABLE_LOGGING

/**
 * Log EXPR to OStream cacao::dbg if debugging is enabled for the given
 * subsystem.
 */
#define LOG_WITH_NAME_N(DBG_NAME, VERBOSE,  EXPR)                    \
	do {                                                             \
		if (DEBUG_COND_WITH_NAME_N( (DBG_NAME) , (VERBOSE) )) {      \
			cacao::OStream stream = cacao::dbg();                    \
																	 \
			if (cacao::Debug::thread_enabled) {                      \
				stream << "LOG: " << cacao::threadid << " ";         \
			}                                                        \
																	 \
			if (cacao::Debug::prefix_enabled) {                      \
				stream << setprefix(DBG_NAME, cacao::log_color());   \
			}                                                        \
																	 \
			{ stream << EXPR ; }                                     \
		}                                                            \
	} while (0)


/// Set the file dbg() writes to
void set_log_file(FILE *file);

/// Set the color for line prefixes of debug messages
void set_log_color(Color color);

/// Get the color for line prefixes of debug messages
Color log_color();

#else // defined(ENABLE_LOGGING)

#define LOG_WITH_NAME_N(DBG_NAME, VERBOSE,  STMT) do { } while(0)

#endif // defined(ENABLE_LOGGING)

#define LOG_WITH_NAME(DBG_NAME, STMT) LOG_WITH_NAME_N(DBG_NAME, 0, STMT)
#define LOG_N(VERBOSE, STMT) LOG_WITH_NAME_N(DEBUG_NAME, VERBOSE, STMT)
/// Analogous to DEBUG
#define LOG(STMT) LOG_N(0, STMT)
#define LOG1(STMT) LOG_N(1, STMT)
#define LOG2(STMT) LOG_N(2, STMT)
#define LOG3(STMT) LOG_N(3, STMT)

#define WARNING_MSG(EXPR_SHORT, EXPR_LONG)                                    \
	do {                                                                      \
		cacao::OStream stream = cacao::err();                                 \
																			  \
		{ stream << cacao::BoldWhite << __FILE__ << ":" << __LINE__ << ": ";} \
		{ stream << cacao::BoldMagenta << "warning: " ; }                     \
		{ stream << cacao::BoldWhite << EXPR_SHORT << cacao::reset_color ; }  \
		{ stream << cacao::nl << EXPR_LONG << cacao::nl; }                    \
	} while (0)

#ifndef NDEBUG
#define assert_msg(COND, EXPR)                                                \
	do {                                                                      \
		if ( ! (COND) ) {                                                     \
			cacao::OStream stream = cacao::err();                             \
		                                                                      \
			{ stream << cacao::BoldRed << "assertion failed: " ; }            \
			{ stream << cacao::BoldWhite<<"`"#COND"'"<< cacao::reset_color; } \
			{ stream << cacao::nl << EXPR << cacao::nl ; }                    \
			assert( COND );                                                   \
		}                                                                     \
	} while (0)
#else

#define assert_msg(COND, EXPR) /* nothing */

#endif

#define ERROR_MSG(EXPR_SHORT, EXPR_LONG)                                      \
	do {                                                                      \
		cacao::OStream stream = cacao::err();                                 \
		                                                                      \
		{ stream << cacao::BoldRed << "error: " ; }                           \
		{ stream << cacao::BoldWhite << EXPR_SHORT << cacao::reset_color ; }  \
		{ stream << cacao::nl << EXPR_LONG << cacao::nl; }                    \
	} while (0)

#define ABORT_MSG(EXPR_SHORT, EXPR_LONG)                                      \
	do {                                                                      \
		ERROR_MSG(EXPR_SHORT, EXPR_LONG);                                     \
		os::abort();                                                          \
	} while (0)

#define SHOULDNOTREACH_MSG(EXPR_LONG)                                          \
	do {                                                                       \
		ERROR_MSG("should not reach", EXPR_LONG);                              \
		os::abort();                                                           \
	} while (0)

#define UNIMPLEMENTED_MSG(EXPR_LONG)                                           \
	do {                                                                       \
		ERROR_MSG("not implemented yet", EXPR_LONG);                           \
		os::abort();                                                           \
	} while (0)

} // end namespace cacao

/* function prototypes ********************************************************/

// TODO: remove, this is just a temporary hack
//       that allows cycle-stats to be printed to the regular log file
//       so we can run make check with cycle-stats enabled.
FILE* log_get_logfile();

void log_init(const char *fname);

void log_start(void);

void log_vprint(const char *text, va_list ap);
void log_print(const char *text, ...);
void log_println(const char *text, ...);

void log_finish(void);

#define log_text(s) log_println("%s", (s))
#define dolog       log_println

/* log message functions */
void log_message_utf(const char *msg, Utf8String u);
void log_message_class(const char *msg, classinfo *c);
void log_message_class_message_class(const char *msg1, classinfo *c1,
									 const char *msg2, classinfo *c2);
void log_message_method(const char *msg, methodinfo *m);

#endif // LOGGING_HPP_

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
