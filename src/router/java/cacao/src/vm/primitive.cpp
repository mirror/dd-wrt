/* src/vm/primitive.cpp - primitive types

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


#include "config.h"

#include <assert.h>
#include <stdint.h>

#include "native/llni.hpp"

#include "toolbox/logging.hpp"

#include "vm/jit/builtin.hpp"
#include "vm/class.hpp"
#include "vm/global.hpp"
#include "vm/globals.hpp"
#include "vm/javaobjects.hpp"
#include "vm/options.hpp"
#include "vm/os.hpp"
#include "vm/primitive.hpp"
#include "vm/utf8.hpp"
#include "vm/vm.hpp"


/* primitivetype_table *********************************************************

   Structure for primitive classes: contains the class for wrapping
   the primitive type, the primitive class, the name of the class for
   wrapping, the one character type signature and the name of the
   primitive class.
 
   CAUTION: Don't change the order of the types. This table is indexed
   by the ARRAYTYPE_ constants (except ARRAYTYPE_OBJECT).

*******************************************************************************/

primitivetypeinfo primitivetype_table[PRIMITIVETYPE_MAX] = {
	{ "int"     , NULL, NULL, NULL, "java/lang/Integer",   'I', "[I", NULL },
	{ "long"    , NULL, NULL, NULL, "java/lang/Long",      'J', "[J", NULL },
	{ "float"   , NULL, NULL, NULL, "java/lang/Float",     'F', "[F", NULL },
	{ "double"  , NULL, NULL, NULL, "java/lang/Double",    'D', "[D", NULL },
	{ NULL      , NULL, NULL, NULL, NULL,                   0 , NULL, NULL },
	{ "byte"    , NULL, NULL, NULL, "java/lang/Byte",      'B', "[B", NULL },
	{ "char"    , NULL, NULL, NULL, "java/lang/Character", 'C', "[C", NULL },
	{ "short"   , NULL, NULL, NULL, "java/lang/Short",     'S', "[S", NULL },
	{ "boolean" , NULL, NULL, NULL, "java/lang/Boolean",   'Z', "[Z", NULL },
	{ NULL      , NULL, NULL, NULL, NULL,                   0 , NULL, NULL },
#if defined(ENABLE_JAVASE)
   	{ "void"    , NULL, NULL, NULL, "java/lang/Void",      'V', NULL, NULL }
#else
	{ NULL      , NULL, NULL, NULL, NULL,                   0 , NULL, NULL },
#endif
};


/**
 * Fill the primitive type table with the primitive-type classes,
 * array-classes and wrapper classes.  This is important in the VM
 * startup.
 *
 * We split this primitive-type table initialization because of
 * annotations in the bootstrap classes.
 *
 * But we may get a problem if we have annotations in:
 *
 * java/lang/Object
 * java/lang/Cloneable
 * java/io/Serializable
 *
 * Also see: loader_preinit and linker_preinit.
 */
void Primitive::initialize_table()
{  
	TRACESUBSYSTEMINITIALIZATION("primitive_init");

	/* Load and link primitive-type classes and array-classes. */

	for (int i = 0; i < PRIMITIVETYPE_MAX; i++) {
		/* Skip dummy entries. */

		if (primitivetype_table[i].cname == NULL)
			continue;

		/* create UTF-8 name */

		Utf8String name = Utf8String::from_utf8(primitivetype_table[i].cname);

		primitivetype_table[i].name = name;

		/* create primitive class */

		classinfo *c = class_create_classinfo(name);

		/* Primitive type classes don't have a super class. */

		c->super = NULL;

		/* set flags and mark it as primitive class */

		c->flags = ACC_PUBLIC | ACC_FINAL | ACC_ABSTRACT | ACC_CLASS_PRIMITIVE;
		
		/* prevent loader from loading primitive class */

		c->state |= CLASS_LOADED;

		/* INFO: don't put primitive classes into the classcache */

		if (!link_class(c))
			vm_abort("linker_init: linking failed");

		/* Just to be sure. */

		assert(c->state & CLASS_LOADED);
		assert(c->state & CLASS_LINKED);

		primitivetype_table[i].class_primitive = c;

		/* Create primitive array class. */

		if (primitivetype_table[i].arrayname != NULL) {
			Utf8String  u  = Utf8String::from_utf8(primitivetype_table[i].arrayname);
			classinfo  *ac = class_create_classinfo(u);
			ac = load_newly_created_array(ac, NULL);

			if (ac == NULL)
				vm_abort("primitive_init: loading failed");

			assert(ac->state & CLASS_LOADED);

			if (!link_class(ac))
				vm_abort("primitive_init: linking failed");

			/* Just to be sure. */

			assert(ac->state & CLASS_LOADED);
			assert(ac->state & CLASS_LINKED);

			primitivetype_table[i].arrayclass = ac;
		}
	}

	/* We use two for-loops to have the array-classes already in the
	   primitive-type table (hint: annotations in wrapper-classes). */

	for (int i = 0; i < PRIMITIVETYPE_MAX; i++) {
		/* Skip dummy entries. */

		if (primitivetype_table[i].cname == NULL)
			continue;

		/* Create class for wrapping the primitive type. */

		Utf8String  u = Utf8String::from_utf8(primitivetype_table[i].wrapname);
		classinfo  *c = load_class_bootstrap(u);

		if (c == NULL)
			vm_abort("primitive_init: loading failed");

		if (!link_class(c))
			vm_abort("primitive_init: linking failed");

		/* Just to be sure. */

		assert(c->state & CLASS_LOADED);
		assert(c->state & CLASS_LINKED);

		primitivetype_table[i].class_wrap = c;
	}
}


/**
 * Finish the primitive-type table initialization.  In this step we
 * set the vftbl of the primitive-type classes.
 *
 * This is necessary because java/lang/Class is loaded and linked
 * after the primitive types have been linked.
 *
 * We have to do that in an extra function, as the primitive types are
 * not stored in the classcache.
 */
void Primitive::post_initialize_table()
{
	TRACESUBSYSTEMINITIALIZATION("primitive_postinit");

	assert(class_java_lang_Class);
	assert(class_java_lang_Class->vftbl);

	for (int i = 0; i < PRIMITIVETYPE_MAX; i++) {
		/* Skip dummy entries. */

		if (primitivetype_table[i].cname == NULL)
			continue;

		classinfo *c = primitivetype_table[i].class_primitive;

		c->object.header.vftbl = class_java_lang_Class->vftbl;
	}
}


/**
 * Returns the primitive class of the given class name.
 *
 * @param name Name of the class.
 *
 * @return Class structure.
 */
classinfo* Primitive::get_class_by_name(Utf8String name)
{
	/* search table of primitive classes */

	for (int i = 0; i < PRIMITIVETYPE_MAX; i++)
		if (primitivetype_table[i].name == name)
			return primitivetype_table[i].class_primitive;

	/* keep compiler happy */

	assert(false);
	return NULL;
}


/**
 * Returns the primitive class of the given type.
 *
 * @param type Integer type of the class.
 *
 * @return Class structure.
 */
classinfo* Primitive::get_class_by_type(int type)
{
	return primitivetype_table[type].class_primitive;
}


/**
 * Returns the primitive class of the given type.
 *
 * @param ch 
 *
 * @return Class structure.
 */
classinfo* Primitive::get_class_by_char(char ch)
{
	int index;

	switch (ch) {
	case 'I':
		index = PRIMITIVETYPE_INT;
		break;
	case 'J':
		index = PRIMITIVETYPE_LONG;
		break;
	case 'F':
		index = PRIMITIVETYPE_FLOAT;
		break;
	case 'D':
		index = PRIMITIVETYPE_DOUBLE;
		break;
	case 'B':
		index = PRIMITIVETYPE_BYTE;
		break;
	case 'C':
		index = PRIMITIVETYPE_CHAR;
		break;
	case 'S':
		index = PRIMITIVETYPE_SHORT;
		break;
	case 'Z':
		index = PRIMITIVETYPE_BOOLEAN;
		break;
	case 'V':
		index = PRIMITIVETYPE_VOID;
		break;
	default:
		return NULL;
	}

	return primitivetype_table[index].class_primitive;
}


/**
 * Returns the primitive array-class of the given primitive class
 * name.
 *
 * @param name Name of the class.
 *
 * @return Class structure.
 */
classinfo* Primitive::get_arrayclass_by_name(Utf8String name)
{
	/* search table of primitive classes */

	for (int i = 0; i < PRIMITIVETYPE_MAX; i++)
		if (primitivetype_table[i].name == name)
			return primitivetype_table[i].arrayclass;

	/* keep compiler happy */

	assert(false);
	return NULL;
}


/**
 * Returns the primitive array-class of the given type.
 *
 * @param type Integer type of the class.
 *
 * @return Class structure.
 */
classinfo* Primitive::get_arrayclass_by_type(int type)
{
	return primitivetype_table[type].arrayclass;
}


/**
 * Returns the primitive type of the given wrapper-class.
 *
 * @param c Class structure.
 *
 * @return Integer type of the class.
 */
int Primitive::get_type_by_wrapperclass(classinfo *c)
{
	/* Search primitive table. */

	for (int i = 0; i < PRIMITIVETYPE_MAX; i++)
		if (primitivetype_table[i].class_wrap == c)
			return i;

	/* Invalid primitive wrapper-class. */

	return -1;
}


/**
 * Returns the primitive type of the given primitive-class.
 *
 * @param c Class structure.
 *
 * @return Integer type of the class.
 */
int Primitive::get_type_by_primitiveclass(classinfo *c)
{
	/* Search primitive table. */

	for (int i = 0; i < PRIMITIVETYPE_MAX; i++)
		if (primitivetype_table[i].class_primitive == c)
			return i;
	
	/* Invalid primitive class. */

	return -1;
}


/**
 * Box a primitive of the given type.  If the type is an object,
 * simply return it.
 *
 * @param type  Type of the passed value.
 * @param value Value to box.
 *
 * @return Handle of the boxing Java object.
 */
java_handle_t* Primitive::box(int type, imm_union value)
{
	java_handle_t* o;

	switch (type) {
	case PRIMITIVETYPE_BOOLEAN:
		o = box((uint8_t) value.i);
		break;
	case PRIMITIVETYPE_BYTE:
		o = box((int8_t) value.i);
		break;
	case PRIMITIVETYPE_CHAR:
		o = box((uint16_t) value.i);
		break;
	case PRIMITIVETYPE_SHORT:
		o = box((int16_t) value.i);
		break;
	case PRIMITIVETYPE_INT:
		o = box(value.i);
		break;
	case PRIMITIVETYPE_LONG:
		o = box(value.l);
		break;
	case PRIMITIVETYPE_FLOAT:
		o = box(value.f);
		break;
	case PRIMITIVETYPE_DOUBLE:
		o = box(value.d);
		break;
	case PRIMITIVETYPE_VOID:
		o = (java_handle_t*) value.a;
		break;
	default:
		o = NULL;
		os::abort("Primitive::box: Invalid primitive type %d", type);
	}

	return o;
}


/**
 * Unbox a primitive of the given type.  If the type is an object,
 * simply return it.
 *
 * @param h Handle of the Java object.
 *
 * @return Unboxed value as union.
 */
imm_union Primitive::unbox(java_handle_t *h)
{
	classinfo *c;
	imm_union  value;

	if (h == NULL) {
		value.a = NULL;
		return value;
	}

	LLNI_class_get(h, c);

	int type = get_type_by_wrapperclass(c);

	switch (type) {
	case PRIMITIVETYPE_BOOLEAN:
		value.i = unbox_boolean(h);
		break;
	case PRIMITIVETYPE_BYTE:
		value.i = unbox_byte(h);
		break;
	case PRIMITIVETYPE_CHAR:
		value.i = unbox_char(h);
		break;
	case PRIMITIVETYPE_SHORT:
		value.i = unbox_short(h);
		break;
	case PRIMITIVETYPE_INT:
		value.i = unbox_int(h);
		break;
	case PRIMITIVETYPE_LONG:
		value.l = unbox_long(h);
		break;
	case PRIMITIVETYPE_FLOAT:
		value.f = unbox_float(h);
		break;
	case PRIMITIVETYPE_DOUBLE:
		value.d = unbox_double(h);
		break;
	case -1:
		/* If type is -1 the object is not a primitive box but a
		   normal object. */
		value.a = h;
		break;
	default:
		os::abort("Primitive::unbox: Invalid primitive type %d", type);
	}

	return value;
}


/**
 * Unbox a primitive of the given type. Also checks if the
 * boxed primitive type can be widened into the destination
 * type. This conversion is done according to
 * "The Java Language Specification, Third Edition,
 * $5.1.2 Widening Primitive Conversion".
 *
 * @param h Handle of the boxing Java object.
 * @param type Destination type of the conversion.
 * @param value Pointer to union where the resulting primitive
 * value will be stored will.
 *
 * @return True of the conversion is allowed, false otherwise.
 */
bool Primitive::unbox_typed(java_handle_t *h, int type, imm_union* value)
{
	classinfo *c;
	int        src_type;

	if (h == NULL)
		return false;

	LLNI_class_get(h, c);

	src_type = get_type_by_wrapperclass(c);

	switch (src_type) {
	case PRIMITIVETYPE_BOOLEAN:
		switch (type) {
			case PRIMITIVETYPE_BOOLEAN:
				value->i = unbox_boolean(h);
				return true;
			default:
				return false;
		}

	case PRIMITIVETYPE_BYTE:
		switch (type) {
			case PRIMITIVETYPE_BYTE:
			case PRIMITIVETYPE_SHORT:
			case PRIMITIVETYPE_INT:
				value->i = unbox_byte(h);
				return true;
			case PRIMITIVETYPE_LONG:
				value->l = unbox_byte(h);
				return true;
			case PRIMITIVETYPE_FLOAT:
				value->f = unbox_byte(h);
				return true;
			case PRIMITIVETYPE_DOUBLE:
				value->d = unbox_byte(h);
				return true;
			default:
				return false;
		}

	case PRIMITIVETYPE_CHAR:
		switch (type) {
			case PRIMITIVETYPE_CHAR:
			case PRIMITIVETYPE_INT:
				value->i = unbox_char(h);
				return true;
			case PRIMITIVETYPE_LONG:
				value->l = unbox_char(h);
				return true;
			case PRIMITIVETYPE_FLOAT:
				value->f = unbox_char(h);
				return true;
			case PRIMITIVETYPE_DOUBLE:
				value->d = unbox_char(h);
				return true;
			default:
				return false;
		}

	case PRIMITIVETYPE_SHORT:
		switch (type) {
			case PRIMITIVETYPE_SHORT:
			case PRIMITIVETYPE_INT:
				value->i = unbox_short(h);
				return true;
			case PRIMITIVETYPE_LONG:
				value->l = unbox_short(h);
				return true;
			case PRIMITIVETYPE_FLOAT:
				value->f = unbox_short(h);
				return true;
			case PRIMITIVETYPE_DOUBLE:
				value->d = unbox_short(h);
				return true;
			default:
				return false;
		}

	case PRIMITIVETYPE_INT:
		switch (type) {
			case PRIMITIVETYPE_INT:
				value->i = unbox_int(h);
				return true;
			case PRIMITIVETYPE_LONG:
				value->l = unbox_int(h);
				return true;
			case PRIMITIVETYPE_FLOAT:
				value->f = unbox_int(h);
				return true;
			case PRIMITIVETYPE_DOUBLE:
				value->d = unbox_int(h);
				return true;
			default:
				return false;
		}

	case PRIMITIVETYPE_LONG:
		switch (type) {
			case PRIMITIVETYPE_LONG:
				value->l = unbox_long(h);
				return true;
			case PRIMITIVETYPE_FLOAT:
				value->f = unbox_long(h);
				return true;
			case PRIMITIVETYPE_DOUBLE:
				value->d = unbox_long(h);
				return true;
			default:
				return false;
		}

	case PRIMITIVETYPE_FLOAT:
		switch (type) {
			case PRIMITIVETYPE_FLOAT:
				value->f = unbox_float(h);
				return true;
			case PRIMITIVETYPE_DOUBLE:
				value->d = unbox_float(h);
				return true;
			default:
				return false;
		}

	case PRIMITIVETYPE_DOUBLE:
		switch (type) {
			case PRIMITIVETYPE_DOUBLE:
				value->d = unbox_double(h);
				return true;
			default:
				return false;
		}

	default:
		os::abort("Primitive::unbox_typed: Invalid primitive type %d", type);
		return false;
	}
}


/**
 * Box a primitive type.
 */
java_handle_t* Primitive::box(uint8_t value)
{
	java_handle_t *h = builtin_new(class_java_lang_Boolean);

	if (h == NULL)
		return NULL;

	java_lang_Boolean b(h);
	b.set_value(value);

	return h;
}

java_handle_t* Primitive::box(int8_t value)
{
	java_handle_t *h = builtin_new(class_java_lang_Byte);

	if (h == NULL)
		return NULL;

	java_lang_Byte b(h);
	b.set_value(value);

	return h;
}

java_handle_t* Primitive::box(uint16_t value)
{
	java_handle_t *h = builtin_new(class_java_lang_Character);

	if (h == NULL)
		return NULL;

	java_lang_Character c(h);
	c.set_value(value);

	return h;
}

java_handle_t* Primitive::box(int16_t value)
{
	java_handle_t *h = builtin_new(class_java_lang_Short);

	if (h == NULL)
		return NULL;

	java_lang_Short s(h);
	s.set_value(value);

	return h;
}

java_handle_t* Primitive::box(int32_t value)
{
	java_handle_t *h = builtin_new(class_java_lang_Integer);

	if (h == NULL)
		return NULL;

	java_lang_Integer i(h);
	i.set_value(value);

	return h;
}

java_handle_t* Primitive::box(int64_t value)
{
	java_handle_t *h = builtin_new(class_java_lang_Long);

	if (h == NULL)
		return NULL;

	java_lang_Long l(h);
	l.set_value(value);

	return h;
}

java_handle_t* Primitive::box(float value)
{
	java_handle_t *h = builtin_new(class_java_lang_Float);

	if (h == NULL)
		return NULL;

	java_lang_Float f(h);
	f.set_value(value);

	return h;
}

java_handle_t* Primitive::box(double value)
{
	java_handle_t *h = builtin_new(class_java_lang_Double);

	if (h == NULL)
		return NULL;

	java_lang_Double d(h);
	d.set_value(value);

	return h;
}



/**
 * Unbox a primitive type.
 */

// template<class T> T Primitive::unbox(java_handle_t *h)
// {
// 	return java_lang_Boolean::get_value(h);
// }

inline uint8_t Primitive::unbox_boolean(java_handle_t *h)
{
	java_lang_Boolean b(h);
	return b.get_value();
}

inline int8_t Primitive::unbox_byte(java_handle_t *h)
{
	java_lang_Byte b(h);
	return b.get_value();
}

inline uint16_t Primitive::unbox_char(java_handle_t *h)
{
	java_lang_Character c(h);
	return c.get_value();
}

inline int16_t Primitive::unbox_short(java_handle_t *h)
{
	java_lang_Short s(h);
	return s.get_value();
}

inline int32_t Primitive::unbox_int(java_handle_t *h)
{
	java_lang_Integer i(h);
	return i.get_value();
}

inline int64_t Primitive::unbox_long(java_handle_t *h)
{
	java_lang_Long l(h);
	return l.get_value();
}

inline float Primitive::unbox_float(java_handle_t *h)
{
	java_lang_Float f(h);
	return f.get_value();
}

inline double Primitive::unbox_double(java_handle_t *h)
{
	java_lang_Double d(h);
	return d.get_value();
}


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
