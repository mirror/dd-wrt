/* src/vm/linker.hpp - class linker header

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


#ifndef LINKER_HPP_
#define LINKER_HPP_ 1

#include "config.h"
#include "arch.hpp"        // for USES_NEW_SUBTYPE
#include "vm/utf8.hpp"

struct java_object_t;
struct classinfo;
class Mutex;


/* global variables ***********************************************************/

/* This lock must be taken while renumbering classes or while atomically      */
/* accessing classes.                                                         */

#if USES_NEW_SUBTYPE

#define LOCK_CLASSRENUMBER_LOCK   /* nothing */
#define UNLOCK_CLASSRENUMBER_LOCK /* nothing */

#else
extern Mutex *linker_classrenumber_lock;

#define LOCK_CLASSRENUMBER_LOCK   linker_classrenumber_lock->lock()
#define UNLOCK_CLASSRENUMBER_LOCK linker_classrenumber_lock->unlock()

#endif


/* function prototypes ********************************************************/

void       linker_preinit(void);
void       linker_init(void);

void linker_create_string_later(java_object_t **a, Utf8String u);
void linker_initialize_deferred_strings();

classinfo *link_class(classinfo *c);

#endif // LINKER_HPP_


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
