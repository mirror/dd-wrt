/* src/native/vm/reflection.hpp - helper functions for java/lang/reflect

   Copyright (C) 2007, 2008
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


#ifndef _REFLECTION_HPP
#define _REFLECTION_HPP

#include "config.h"
#include "vm/global.hpp"

struct classinfo;
struct methodinfo;

class Reflection {
public:
	static java_handle_t* invoke(methodinfo *m, java_handle_t *o, java_handle_objectarray_t *params);

#if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH) && defined(ENABLE_ANNOTATIONS)
	static java_handle_t* get_declaredannotations(java_handle_bytearray_t *annotations, classinfo* declaringClass, classinfo *referer);
	static java_handle_objectarray_t* get_parameterannotations(java_handle_bytearray_t* parameterAnnotations, methodinfo* m, classinfo *referer);
#endif
};

#endif // _REFLECTION_HPP


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
