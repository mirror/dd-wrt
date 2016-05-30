/* src/vm/primitive.hpp - primitive types

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


#ifndef PRIMITIVE_HPP_
#define PRIMITIVE_HPP_ 1

#include "config.h"

#include <stdint.h>

#include "vm/global.hpp"
#include "vm/linker.hpp"
#include "vm/utf8.hpp"

struct classinfo;

/* primitive data types *******************************************************/

/**
 * JVM types
 *
 * These values are used in parsed descriptors and in some other
 * places were the different types handled internally as TYPE_INT have
 * to be distinguished.
 *
 * CAUTION: Don't change the numerical values! These constants are
 * used as indices into the primitive type table.
 *
 * @todo PRIMITIVETYPE_DUMMY1 is only used to keep this enum interchangeable
 * with the Type enum. It is an artefact from the transition from C to C++
 * (i.e. #defines to enums). Should be replaced (e.g. by a new enum).
 */
enum PrimitiveType {
	PRIMITIVETYPE_INT     = TYPE_INT,
	PRIMITIVETYPE_LONG    = TYPE_LNG,
	PRIMITIVETYPE_FLOAT   = TYPE_FLT,
	PRIMITIVETYPE_DOUBLE  = TYPE_DBL,
	PRIMITIVETYPE_DUMMY1  = TYPE_ADR,  // XXX not used!
	PRIMITIVETYPE_BYTE    = 5,
	PRIMITIVETYPE_CHAR    = 6,
	PRIMITIVETYPE_SHORT   = 7,
	PRIMITIVETYPE_BOOLEAN = 8,
	PRIMITIVETYPE_VOID    = TYPE_VOID,

	PRIMITIVETYPE_MAX     = 11  // maximum value of a primitive type
};

class Primitive {
public:
	static void initialize_table();
	static void post_initialize_table();

	static classinfo*     get_class_by_name(Utf8String name);
	static classinfo*     get_class_by_type(int type);
	static classinfo*     get_class_by_char(char ch);
	static classinfo*     get_arrayclass_by_name(Utf8String name);
	static classinfo*     get_arrayclass_by_type(int type);

	static int            get_type_by_wrapperclass(classinfo *c);
	static int            get_type_by_primitiveclass(classinfo *c);

	static java_handle_t* box(int type, imm_union value);

	static java_handle_t* box(uint8_t value);
	static java_handle_t* box(int8_t value);
	static java_handle_t* box(uint16_t value);
	static java_handle_t* box(int16_t value);
	static java_handle_t* box(int32_t value);
	static java_handle_t* box(int64_t value);
	static java_handle_t* box(float value);
	static java_handle_t* box(double value);

	static imm_union      unbox(java_handle_t *o);
	static bool           unbox_typed(java_handle_t *o, int type, imm_union* value);

	static uint8_t        unbox_boolean(java_handle_t* o);
	static int8_t         unbox_byte(java_handle_t* o);
	static uint16_t       unbox_char(java_handle_t* o);
	static int16_t        unbox_short(java_handle_t* o);
	static int32_t        unbox_int(java_handle_t* o);
	static int64_t        unbox_long(java_handle_t* o);
	static float          unbox_float(java_handle_t* o);
	static double         unbox_double(java_handle_t* o);
};

/* primitivetypeinfo **********************************************************/

struct primitivetypeinfo {
	const char* cname;                   /* char name of primitive class      */
	Utf8String  name;                    /* name of primitive class           */
	classinfo*  class_wrap;              /* class for wrapping primitive type */
	classinfo*  class_primitive;         /* primitive class                   */
	const char* wrapname;                /* name of class for wrapping        */
	const char  typesig;                 /* one character type signature      */
	const char* arrayname;               /* name of primitive array class     */
	classinfo*  arrayclass;              /* primitive array class             */
};

/* global variables ***********************************************************/

/* This array can be indexed by the PRIMITIVETYPE_ and ARRAYTYPE_
   constants (except ARRAYTYPE_OBJECT). */

extern primitivetypeinfo primitivetype_table[PRIMITIVETYPE_MAX];

/* this function is in src/vm/primitivecore.c */
void       primitive_init(void);
void       primitive_postinit(void);


#endif // PRIMITIVE_HPP_


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
