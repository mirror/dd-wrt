/* src/vm/references.hpp - references to classes/fields/methods

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

#ifndef REFERENCES_HPP_
#define REFERENCES_HPP_ 1

#include "config.h"

#include "vm/global.hpp"
#include "vm/types.hpp"
#include "vm/utf8.hpp"

struct classinfo;
struct typedesc;
struct fieldinfo;
struct methoddesc;
struct methodinfo;

/* constant_classref **********************************************************/

/// a value that never occurrs in classinfo.header.vftbl
#define CLASSREF_PSEUDO_VFTBL (reinterpret_cast<void *>(1))

struct constant_classref {
	void* const      pseudo_vftbl;     /* for distinguishing it from classinfo     */
	classinfo* const referer;          /* class containing the reference           */
	const Utf8String name;             /* name of the class refered to             */

	constant_classref(classinfo *referer, Utf8String name)
	 : pseudo_vftbl(CLASSREF_PSEUDO_VFTBL), referer(referer), name(name) {}
};


/* classref_or_classinfo ******************************************************/

/**
 * @Cpp11 Replace static constructors with regular constructors.
 *        With C++98 this breaks other unions that use classref_or_classinfo.
 */
union classref_or_classinfo {
	constant_classref *ref;       /* a symbolic class reference               */
	classinfo         *cls;       /* an already loaded class                  */
	void              *any;       /* used for general access (x != NULL,...)  */

	bool is_classref()  const { return ref->pseudo_vftbl == CLASSREF_PSEUDO_VFTBL; }
	bool is_classinfo() const { return !is_classref(); }
};


/* parseddesc_t ***************************************************************/

union parseddesc_t {
	typedesc   *fd;               /* parsed field descriptor                  */
	methoddesc *md;               /* parsed method descriptor                 */

	// test against NULL;
	operator bool() const { return fd; }
};

/* structs ********************************************************************/

/**
 * Fieldref, Methodref and InterfaceMethodref
 */
struct constant_FMIref {
	constant_FMIref(constant_classref *ref,
	                Utf8String         name,
	                Utf8String         descriptor,
	                parseddesc_t       parseddesc)
	 : name(name), descriptor(descriptor), parseddesc(parseddesc) {
		p.classref = ref;
	}

	union {
		// set when FMIref is unresolved
		constant_classref *classref;  /* class having this field/meth./intfm. */

		// set when FMIref is resolved
		fieldinfo         *field;     /* resolved field                       */
		methodinfo        *method;    /* resolved method                      */
	} p;
	const Utf8String   name;       /* field/method/interfacemethod name             */
	const Utf8String   descriptor; /* field/method/intfmeth. type descriptor string */
	const parseddesc_t parseddesc; /* parsed descriptor                             */

	bool is_resolved() const { return p.classref->pseudo_vftbl != CLASSREF_PSEUDO_VFTBL; }
};

/// The kinds of methodhandles that can appear in a class file
enum MethodHandleKind {
	REF_getField          = 1,
	REF_getStatic         = 2,
	REF_putField          = 3,
	REF_putStatic         = 4,
	REF_invokeVirtual     = 5,
	REF_invokeStatic      = 6,
	REF_invokeSpecial     = 7,
	REF_newInvokeSpecial  = 8,
	REF_invokeInterface   = 9
};

/***
 * A MethodHandle constant stored in the constant pool
 */
struct constant_MethodHandle {
	constant_MethodHandle(MethodHandleKind kind, constant_FMIref *fmi)
	 : kind(kind), fmi(fmi), handle(NULL) {}

	const MethodHandleKind kind;
	constant_FMIref *const fmi;

	/// resolved java.lang.invoke.MethodHandle object
	java_object_t          *handle;
};

/**
 * A MethodType constant stored in the constant pool
 */
struct constant_MethodType {
	constant_MethodType(Utf8String desc, methoddesc *parseddesc)
	 : descriptor(desc), parseddesc(parseddesc), type(NULL) {}

	const Utf8String        descriptor;
	const methoddesc *const parseddesc;

	/// resolved java.lang.invoke.MethodType object
	java_object_t *type;
};

/**
 * An invokedynamic call site.
 */
struct constant_InvokeDynamic {
	constant_InvokeDynamic(uint16_t bsm, Utf8String name, Utf8String desc, methoddesc *parseddesc)
	 : bootstrap_method_index(bsm), name(name), descriptor(desc), parseddesc(parseddesc), handle(NULL) {}

	const uint16_t          bootstrap_method_index;
	const Utf8String        name;
	const Utf8String        descriptor;
	const methoddesc *const parseddesc;

	/// resolved java.lang.invoke.MethodHandle object for this CallSite
	java_object_t *handle;
};


/* inline functions ***********************************************************/

/**
 * Functions for casting a classref/classinfo to a classref_or_classinfo
 * @Cpp11 Replace with constructors in classref_or_classinfo.
 *        With C++98 this breaks other unions that use classref_or_classinfo.
 * @{
 */
static inline classref_or_classinfo to_classref_or_classinfo(classinfo *c) {
	classref_or_classinfo out;
	out.cls = c;
	return out;
}
static inline classref_or_classinfo to_classref_or_classinfo(constant_classref *c) {
	classref_or_classinfo out;
	out.ref = c;
	return out;
}
/**
 * @}
 */


/* macros *********************************************************************/

/* macro for accessing the name of a classref/classinfo                       */
#define CLASSREF_OR_CLASSINFO_NAME(value) \
	((value).is_classref() ? (value).ref->name : (value).cls->name)

/* macro for accessing the class name of a method reference                   */
#define METHODREF_CLASSNAME(fmiref) \
	((fmiref)->is_resolved() ? (fmiref)->p.method->clazz->name \
								: (fmiref)->p.classref->name)

/* macro for accessing the class name of a method reference                   */
#define FIELDREF_CLASSNAME(fmiref) \
	((fmiref)->is_resolved() ? (fmiref)->p.field->clazz->name \
								: (fmiref)->p.classref->name)

#endif // REFERENCES_HPP_

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

