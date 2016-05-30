/* src/vm/jit/verify/typeinfo.hpp - type system used by the type checker

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

#ifndef _TYPEINFO_H
#define _TYPEINFO_H

#include "config.h"
#include "toolbox/assert.hpp" 
#include "vm/array.hpp"
#include "vm/global.hpp"
#include "vm/globals.hpp"
#include "vm/references.hpp"
#include "vm/types.hpp"

struct instruction;
struct varinfo;
struct typeinfo_t;
struct typeinfo_mergedlist_t;
struct typedescriptor_t;

/* configuration **************************************************************/

/*
 * TYPECHECK_STATISTICS activates gathering statistical information.
 * TYPEINFO_DEBUG activates debug checks and debug helpers in typeinfo.cpp
 * TYPECHECK_DEBUG activates debug checks in typecheck.c
 * TYPEINFO_DEBUG_TEST activates the typeinfo test at startup.
 * TYPECHECK_VERBOSE_IMPORTANT activates important debug messages
 * TYPECHECK_VERBOSE activates all debug messages
 * TYPEINFO_VERBOSE activates debug prints in typeinfo.c
 */
#ifdef ENABLE_VERIFIER
#ifndef NDEBUG
/*#define TYPECHECK_STATISTICS*/
#define TYPEINFO_DEBUG
/*#define TYPEINFO_VERBOSE*/
#define TYPECHECK_DEBUG
/*#define TYPEINFO_DEBUG_TEST*/
/*#define TYPECHECK_VERBOSE*/
/*#define TYPECHECK_VERBOSE_IMPORTANT*/
#if defined(TYPECHECK_VERBOSE) || defined(TYPECHECK_VERBOSE_IMPORTANT)
#define TYPECHECK_VERBOSE_OPT
#endif
#endif
#endif

#ifdef TYPECHECK_VERBOSE_OPT
extern bool opt_typecheckverbose;
#endif

/* types **********************************************************************/

/* typecheck_result - return type for boolean and tristate  functions     */
/*                    which may also throw exceptions (typecheck_FAIL).   */

/* NOTE: Use the enum values, not the uppercase #define macros!          */
#define TYPECHECK_MAYBE  0x02
#define TYPECHECK_FAIL   0x04

enum typecheck_result {
	typecheck_FALSE = false,
	typecheck_TRUE  = true,
	typecheck_MAYBE = TYPECHECK_MAYBE,
	typecheck_FAIL  = TYPECHECK_FAIL
};

/* check that typecheck_MAYBE is not ambiguous */
#if TYPECHECK_MAYBE == true
#error "`typecheck_MAYBE` must not be the same as `true`"
#endif
#if TYPECHECK_MAYBE == false
#error "`typecheck_MAYBE` must not be the same as `false`"
#endif

/* check that typecheck_FAIL is not ambiguous */
#if (true & TYPECHECK_FAIL) != 0
#error "`true` must not have bit 0x02 set (conflicts with typecheck_FAIL)"
#endif

/* data structures for the type system ****************************************/

/* The typeinfo structure stores detailed information on address types.
 * (stack elements, variables, etc. with type == TYPE_ADR.)
 *
 * There are two kinds of address types which can be distinguished by
 * the value of the typeclass field:
 *
 * 1) typeclass == NULL: returnAddress type
 *                       use typeinfo::is_primitive() to test for this
 *
 * 2) typeclass != NULL: reference type
 *                       use typeinfo::is_reference() to test for this
 *
 * Note: For non-address types either there is no typeinfo allocated
 * or the fields of the typeinfo struct contain undefined values!
 * DO NOT access the typeinfo for non-address types!
 *
 * CAUTION: The typeinfo structure should be considered opaque outside of
 *          typeinfo.[ch]. Please use the macros and functions defined here to
 *          access typeinfo structures!
 */

/* At all times *exactly one* of the following conditions is true for
 * a particular typeinfo struct:
 *
 * A) typeclass == NULL
 *
 *        This is a returnAddress type.
 *
 *        Use typeinfo::is_primitive() to check for this.
 *        Use typeinfo::returnaddress() to access the pointer in elementclass.
 *        Don't access other fields of the struct.
 *
 * B) typeclass == pseudo_class_Null
 *
 *        This is the null-reference type.
 *        Use typeinfo::is_nulltype() to check for this.
 *        Don't access other fields of the struct.
 *
 * C) typeclass == pseudo_class_New
 *
 *        This is an 'uninitialized object' type. elementclass can be
 *        cast to instruction* and points to the NEW instruction
 *        responsible for creating this type.
 *
 *        Use typeinfo::newobject_instruction to access the pointer in
 *        elementclass.
 *        Don't access other fields of the struct.
 *
 * D) typeclass == pseudo_class_Arraystub
 *
 *        This type is used to represent the result of merging array types
 *        with incompatible component types. An arraystub allows no access
 *        to its components (since their type is undefined), but it allows
 *        operations which act directly on an arbitrary array type (such as
 *        requesting the array size).
 *
 *        NOTE: An array stub does *not* count as an array. It has dimension
 *              zero.
 *
 *        Otherwise like a normal class reference type.
 *        Don't access other fields of the struct.
 *
 * E) typeclass is an array class
 *
 *        An array reference.
 *            elementclass...typeclass of the element type
 *            dimension......dimension of the array (>=1)
 *            elementtype....element type (ARRAYTYPE_...)
 *            merged.........mergedlist of the element type
 *
 *        Use typeinfo:is_array to check for this case.
 *
 *        The elementclass may be one of the following:
 *        1) pseudo_class_Arraystub
 *        2) an unresolved type
 *        3) a loaded interface
 *        4) a loaded (non-pseudo-,non-array-)class != (BOOTSTRAP)java.lang.Object
 *                Note: `merged` may be used
 *        5) (BOOTSTRAP)java.lang.Object
 *                Note: `merged` may be used
 *
 *        For the semantics of the merged field in cases 4) and 5) consult the 
 *        corresponding descriptions with `elementclass` replaced by `typeclass`.
 *
 * F) typeclass is an unresolved type (a symbolic class/interface reference)
 *
 *        The type has not been resolved yet. (Meaning it corresponds to an
 *        unloaded class or interface).
 *        Don't access other fields of the struct.
 *
 * G) typeclass is a loaded interface
 *
 *        An interface reference type.
 *        Don't access other fields of the struct.
 *
 * H) typeclass is a loaded (non-pseudo-,non-array-)class != (BOOTSTRAP)java.lang.Object
 *
 *        A loaded class type.
 *        All classref_or_classinfos in u.merged.list (if any) are
 *        loaded subclasses of typeclass (no interfaces, array classes, or
 *        unresolved types).
 *        Don't access other fields of the struct.
 *
 * I) typeclass is (BOOTSTRAP)java.lang.Object
 *
 *        The most general kind of reference type.
 *        In this case u.merged.count and u.merged.list
 *        are valid and may be non-zero.
 *        The classref_or_classinfos in u.merged.list (if any) may be
 *        classes, interfaces, pseudo classes or unresolved types.
 *        Don't access other fields of the struct.
 */

/* The following algorithm is used to determine if the type described
 * by this typeinfo struct supports the interface X:  * XXX add MAYBE *
 *
 *     1) If typeclass is X or a subinterface of X the answer is "yes".
 *     2) If typeclass is a (pseudo) class implementing X the answer is "yes".
 *     3) If typeclass is not an array and u.merged.count>0
 *        and all classes/interfaces in u.merged.list implement X
 *        the answer is "yes".
 *     4) If none of the above is true the answer is "no".
 */

/*
 * CAUTION: The typeinfo structure should be considered opaque outside of
 *          typeinfo.[ch]. Please use the macros and functions defined here to
 *          access typeinfo structures!
 */

void typeinfo_init_classinfo(typeinfo_t *info,classinfo *c);

struct typeinfo_t {
	classref_or_classinfo  typeclass;
	classref_or_classinfo  elementclass; // valid if dimension>0 (various uses!)
	typeinfo_mergedlist_t *merged;
	u1                     dimension;
	ArrayType              elementtype;  // valid if dimension>0

	bool is_primitive() const { return typeclass.any == NULL; }
	bool is_reference() const { return typeclass.any != NULL; }

	bool is_nulltype()  const { return typeclass.cls == pseudo_class_Null;     }
	bool is_newobject() const { return typeclass.cls == pseudo_class_New;      }

	void *returnaddress() const {
		EXPENSIVE_ASSERT(is_primitive());

		return elementclass.any;
	}

	instruction *newobject_instruction() const {
		EXPENSIVE_ASSERT(is_newobject());

		return (instruction*) elementclass.any;
	}

	bool is_array()        const { return is_reference() && dimension != 0; }
	bool is_simple_array() const { return dimension == 1; }

	bool is_primitive_array(ArrayType at) const {
		return is_simple_array() && elementtype == at;
	}
	bool is_array_of_refs() const {
		return is_array() && (elementclass.any != NULL || dimension >= 2);
	}

	// queries allowing the null type

	bool maybe_array() const {
		return is_array() || is_nulltype();
	}

	bool maybe_primitive_array(ArrayType at) const {
		return is_primitive_array(at) || is_nulltype();
	}

	bool maybe_array_of_refs() const {
		return is_array_of_refs() || is_nulltype();
	}

	// Check if `this' type is assignable to a given destination type.
	typecheck_result is_assignable_to(typeinfo_t *dest) const;
	typecheck_result is_assignable_to_class(classref_or_classinfo dest) const;

	// initializing typeinfo structures

	void init_primitive() {
		typeclass.any    = NULL;
		elementclass.any = NULL;
		merged           = NULL;
		dimension        = 0;
		elementtype      = ARRAYTYPE_INT;
	}

	void init_returnaddress(void *adr) {
		typeclass.any    = NULL;
		elementclass.any = adr;
		merged           = NULL;
		dimension        = 0;
		elementtype      = ARRAYTYPE_INT;
	}

	void init_non_array_classinfo(classinfo *cls) {
		typeclass.cls    = cls;
		elementclass.any = NULL;
		merged           = NULL;
		dimension        = 0;
		elementtype      = ARRAYTYPE_INT;
	}

	/// Initialize object type java.lang.Class
	void init_java_lang_class(classref_or_classinfo cls) {
		typeclass.cls    = class_java_lang_Class;
		elementclass     = cls;
		merged           = NULL;
		dimension        = 0;
		elementtype      = ARRAYTYPE_INT;
	}

	/// Initialize object type
	void init_class(classinfo *c);
	bool init_class(classref_or_classinfo c);

	bool init_class(constant_classref *c) {
		return init_class(to_classref_or_classinfo(c));
	}

	bool init_component(const typeinfo_t& srcarray);

	bool init_from_typedesc(const typedesc *desc, u1 *type);

	void init_nulltype() {
		init_non_array_classinfo(pseudo_class_Null);
	}

	void init_newobject(instruction *instr) {
		typeclass.cls    = pseudo_class_New;
		elementclass.any = instr;
		merged           = NULL;
		dimension        = 0;
		elementtype      = ARRAYTYPE_INT;
	}

	void init_primitive_array(ArrayType arraytype) {
		init_class(primitivetype_table[arraytype].arrayclass);
	}

	// copying types (destinition is not checked or freed)

	/***
	 * makes a deep copy, the merged list (if any) is duplicated
	 * into a newly allocated array.
	 */
	static void clone(const typeinfo_t& src, typeinfo_t& dst) {
		dst = src;

		if (dst.merged)
			clone_merged(src, dst);
	}

	/// functions for merging types

	typecheck_result merge(methodinfo *m, const typeinfo_t* t);
private:
	static void clone_merged(const typeinfo_t& src, typeinfo_t& dst);
};


struct typeinfo_mergedlist_t {
	s4                    count;
	classref_or_classinfo list[1];       /* variable length!                        */
};

/* a type descriptor stores a basic type and the typeinfo                */
/* this is used for storing the type of a local variable, and for        */
/* storing types in the signature of a method                            */

struct typedescriptor_t {
	typeinfo_t      typeinfo; /* valid if type == TYPE_ADR               */
	Type            type;     /* basic type (TYPE_INT, ...)              */

	bool is_returnaddress() const { return type == TYPE_RET && typeinfo.is_primitive(); }
	bool is_reference()     const { return type == TYPE_ADR && typeinfo.is_reference(); }
};


/****************************************************************************/
/* MACROS                                                                   */
/****************************************************************************/

/* NOTE: The TYPEINFO macros take typeinfo *structs*, not pointers as
 *       arguments.  You have to dereference any pointers.
 */

/* typevectors **************************************************************/

#define TYPEVECTOR_SIZE(size)						\
    ((size) * sizeof(varinfo))

#define DNEW_TYPEVECTOR(size)						\
    ((varinfo *) DumpMemory::allocate(TYPEVECTOR_SIZE(size)))

#define DMNEW_TYPEVECTOR(num,size)					\
    ((varinfo *) DumpMemory::allocate((num) * TYPEVECTOR_SIZE(size)))

#define MGET_TYPEVECTOR(array,index,size) \
    ((varinfo*) (((u1*)(array)) + TYPEVECTOR_SIZE(size) * (index)))


/****************************************************************************/
/* FUNCTIONS                                                                */
/****************************************************************************/

/* typevector functions *****************************************************/

/* element read-only access */
bool typevector_checktype(varinfo *set,int index,int type);
bool typevector_checkreference(varinfo *set,int index);
bool typevector_checkretaddr(varinfo *set,int index);

/* element write access */
void typevector_store(varinfo *set,int index, Type type,typeinfo_t *info);
void typevector_store_retaddr(varinfo *set,int index,typeinfo_t *info);
bool typevector_init_object(varinfo *set,void *ins,classref_or_classinfo initclass,int size);

/* vector functions */
varinfo         *typevector_copy(varinfo *src,int size);
void             typevector_copy_inplace(varinfo *src,varinfo *dst,int size);
typecheck_result typevector_merge(methodinfo *m,varinfo *dst,varinfo *y,int size);

/* initialization functions *************************************************/

/* RETURN VALUE (bool):
 *     true.............ok,
 *     false............an exception has been thrown.
 *
 * RETURN VALUE (int):
 *     >= 0.............ok,
 *     -1...............an exception has been thrown.
 */
int  typedescriptors_init_from_methoddesc(typedescriptor_t *td,
										  methoddesc *desc,
										  int buflen,bool twoword,int startindex,
										  typedescriptor_t *returntype);
bool typeinfo_init_varinfos_from_methoddesc(varinfo *vars,
										  methoddesc *desc,
										  int buflen, int startindex,
										  s4 *map,
										  typedescriptor_t *returntype);

/* debugging helpers ********************************************************/

#ifdef TYPEINFO_DEBUG

#include <stdio.h>

void typeinfo_test();
void typeinfo_print_class(FILE *file, classref_or_classinfo c);
void typeinfo_print(FILE *file, const typeinfo_t *info, int indent);
void typeinfo_print_short(FILE *file, const typeinfo_t *info);
void typeinfo_print_type(FILE *file, int type, const typeinfo_t *info);
void typedescriptor_print(FILE *file, typedescriptor_t *td);
void typevector_print(FILE *file, varinfo *vec, int size);

#endif /* TYPEINFO_DEBUG */

#endif /* _TYPEINFO_H */


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
