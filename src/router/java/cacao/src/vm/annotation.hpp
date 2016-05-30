/* src/vm/annotation.hpp - class annotations

   Copyright (C) 2006, 2007, 2008
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


#ifndef _ANNOTATION_HPP
#define _ANNOTATION_HPP

struct classbuffer;
struct fieldinfo;
struct methodinfo;

/* function prototypes ********************************************************/

namespace cacao {

struct ClassBuffer;

bool annotation_load_class_attribute_runtimevisibleannotations(ClassBuffer &cb);

bool annotation_load_class_attribute_runtimeinvisibleannotations(ClassBuffer &cb);

bool annotation_load_method_attribute_runtimevisibleannotations(ClassBuffer &cb, methodinfo *m);

bool annotation_load_method_attribute_runtimeinvisibleannotations(ClassBuffer &cb, methodinfo *m);

bool annotation_load_field_attribute_runtimevisibleannotations(ClassBuffer &cb, fieldinfo *f);

bool annotation_load_field_attribute_runtimeinvisibleannotations(ClassBuffer &cb, fieldinfo *f);

bool annotation_load_method_attribute_annotationdefault(ClassBuffer &cb, methodinfo *m);

bool annotation_load_method_attribute_runtimevisibleparameterannotations(ClassBuffer &cb, methodinfo *m);

bool annotation_load_method_attribute_runtimeinvisibleparameterannotations(ClassBuffer &cb, methodinfo *m);

} // end namespace cacao;

#endif /* _ANNOTATION_HPP */


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
