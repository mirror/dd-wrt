/* src/native/llni.hpp - low level native interfarce (LLNI)

   Copyright (C) 2007-2013
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


#ifndef LLNI_HPP_
#define LLNI_HPP_ 1

#include "config.h"
#include "vm/global.hpp"

struct classinfo;
struct threadobject;

/* forward defines ************************************************************/

/* LLNI wrapping / unwrapping macros *******************************************

   ATTENTION: Only use these macros inside a LLNI critical section!
   Once the ciritical section ends, all pointers onto the GC heap
   retrieved through these macros are void!

*******************************************************************************/

#if defined(ENABLE_HANDLES)
# define LLNI_WRAP(obj)      ((obj) == NULL ? NULL : localref_add(obj))
# define LLNI_UNWRAP(hdl)    ((hdl) == NULL ? NULL : (hdl)->heap_object)
# define LLNI_QUICKWRAP(obj) ((obj) == NULL ? NULL : &(obj))
# define LLNI_DIRECT(hdl)    ((hdl)->heap_object)
#else
# define LLNI_WRAP(obj)      (obj)
# define LLNI_UNWRAP(hdl)    (hdl)
# define LLNI_QUICKWRAP(obj) (obj)
# define LLNI_DIRECT(hdl)    (hdl)
#endif


#include "native/localref.hpp"

#define LLNI_class_get(obj, variable) \
	(variable) = LLNI_field_direct((java_handle_t *) obj, vftbl->clazz)


/* LLNI_equals ****************************************************************

   Test if two java_handle_t* point to the same java_object_t*.

******************************************************************************/

#define LLNI_equals(obj1, obj2, result) \
	LLNI_CRITICAL_START; \
	(result) = LLNI_UNWRAP(obj1) == LLNI_UNWRAP(obj2); \
	LLNI_CRITICAL_END


/* LLNI_classinfo_field_get ***************************************************

   Get a field from classinfo that is a java object.

******************************************************************************/

#define LLNI_classinfo_field_get(cls, field, variable) \
	LLNI_CRITICAL_START; \
	(variable) = LLNI_WRAP((cls)->field); \
	LLNI_CRITICAL_END


/* LLNI_classinfo_field_set ***************************************************

   Set a field from classinfo that is a java object.

******************************************************************************/

#define LLNI_classinfo_field_set(cls, field, variable) \
	LLNI_CRITICAL_START; \
	(cls)->field = LLNI_UNWRAP(variable); \
	LLNI_CRITICAL_END


/* LLNI classinfo wrapping / unwrapping macros *********************************

   The following macros are used to wrap or unwrap a classinfo from
   or into a handle (typically java_lang_Class). No critical sections
   are needed here, because classinfos are not placed on the GC heap.

   XXX This might change once we implement Class Unloading!

*******************************************************************************/

#define LLNI_classinfo_wrap(classinfo) \
	((java_handle_t*) LLNI_WRAP(classinfo))

#define LLNI_classinfo_unwrap(clazz) \
	((classinfo *) LLNI_UNWRAP((java_handle_t *) (clazz)))


/* XXX the direct macros have to be used inside a critical section!!! */

#define LLNI_field_direct(obj, field) (LLNI_DIRECT(obj)->field)
#define LLNI_vftbl_direct(obj)        (LLNI_DIRECT((java_handle_t *) (obj))->vftbl)


/* LLNI critical sections ******************************************************

   These macros handle the LLNI critical sections. While a critical
   section is active, the absolute position of objects on the GC heap
   is not allowed to change (no moving Garbage Collection).

   ATTENTION: Use a critical section whenever you have a direct
   pointer onto the GC heap!

*******************************************************************************/

#if defined(ENABLE_THREADS) && defined(ENABLE_GC_CACAO)
# define LLNI_CRITICAL_START           llni_critical_start()
# define LLNI_CRITICAL_END             llni_critical_end()
# define LLNI_CRITICAL_START_THREAD(t) llni_critical_start_thread(t)
# define LLNI_CRITICAL_END_THREAD(t)   llni_critical_end_thread(t)
#else
# define LLNI_CRITICAL_START
# define LLNI_CRITICAL_END
# define LLNI_CRITICAL_START_THREAD(t)
# define LLNI_CRITICAL_END_THREAD(t)
#endif

void llni_critical_start();
void llni_critical_end();
void llni_critical_start_thread(threadobject *t);
void llni_critical_end_thread(threadobject *t);

#endif // LLNI_HPP_


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
