/* src/vm/exceptions.hpp - exception related functions prototypes

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


#ifndef EXCEPTIONS_HPP_
#define EXCEPTIONS_HPP_ 1

#include "config.h"

#include "vm/global.hpp"
#include "vm/types.hpp"
#include "vm/utf8.hpp"

struct classinfo;
struct methodinfo;

/* function prototypes ********************************************************/

java_handle_t *exceptions_get_exception(void);
void           exceptions_set_exception(java_handle_t *o);
void           exceptions_clear_exception(void);
java_handle_t *exceptions_get_and_clear_exception(void);


/* functions to generate compiler exceptions */

java_handle_t *exceptions_new_abstractmethoderror(void);
java_handle_t *exceptions_new_arraystoreexception(void);

extern "C" java_object_t *exceptions_asm_new_abstractmethoderror(u1 *sp, u1 *ra);

void exceptions_throw_abstractmethoderror(void);
void exceptions_throw_classcircularityerror(classinfo *c);
void exceptions_throw_classformaterror(classinfo *c, const char *message, ...);
void exceptions_throw_classformaterror(classinfo *c, Utf8String message);
void exceptions_throw_classnotfoundexception(Utf8String name);
void exceptions_throw_noclassdeffounderror(Utf8String name);
void exceptions_throw_noclassdeffounderror_cause(java_handle_t *cause);
void exceptions_throw_noclassdeffounderror_wrong_name(classinfo *c, Utf8String name);
void exceptions_throw_linkageerror(const char *message, classinfo *c);
void exceptions_throw_nosuchfielderror(classinfo *c, Utf8String name);
void exceptions_throw_nosuchmethoderror(classinfo *c, Utf8String name, Utf8String desc);
void exceptions_throw_exceptionininitializererror(java_handle_t *cause);
void exceptions_throw_incompatibleclasschangeerror(classinfo *c,
												   const char *message);
void exceptions_throw_instantiationerror(classinfo *c);
void exceptions_throw_internalerror(const char *message, ...);
void exceptions_throw_outofmemoryerror(void);
void exceptions_throw_verifyerror(methodinfo *m, const char *message, ...);
void exceptions_throw_verifyerror_for_stack(methodinfo *m, int type);
void exceptions_throw_unsatisfiedlinkerror(Utf8String name);
void exceptions_throw_unsupportedclassversionerror(classinfo *c);

java_handle_t *exceptions_new_arithmeticexception(void);

java_handle_t *exceptions_new_arrayindexoutofboundsexception(s4 index);
void exceptions_throw_arrayindexoutofboundsexception(void);
void exceptions_throw_arraystoreexception(void);

java_handle_t *exceptions_new_classcastexception(java_handle_t *o);

void exceptions_throw_clonenotsupportedexception(void);
void exceptions_throw_illegalaccessexception(Utf8String message);
void exceptions_throw_illegalargumentexception(void);
void exceptions_throw_illegalmonitorstateexception(void);
void exceptions_throw_interruptedexception(void);
void exceptions_throw_instantiationexception(classinfo *c);
void exceptions_throw_invocationtargetexception(java_handle_t *cause);
void exceptions_throw_negativearraysizeexception(void);

java_handle_t *exceptions_new_nullpointerexception(void);
void exceptions_throw_nullpointerexception(void);
void exceptions_throw_privilegedactionexception(java_handle_t *cause);
void exceptions_throw_stringindexoutofboundsexception(void);

java_handle_t *exceptions_fillinstacktrace(void);

void exceptions_print_exception(java_handle_t *xptr);
void exceptions_print_current_exception(void);
void exceptions_print_stacktrace(void);

extern "C" void *exceptions_handle_exception(java_object_t *xptro, void *xpc, void *pv, void *sp);

#endif // EXCEPTIONS_HPP_


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
