/* src/vm/array.cpp - Java array functions

   Copyright (C) 2007-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
   Copyright (C) 2008 Theobroma Systems Ltd.

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

#include <stdint.h>

#include "native/llni.hpp"

#include "vm/array.hpp"
#include "vm/exceptions.hpp"
#include "vm/global.hpp"
#include "vm/globals.hpp"
#include "vm/primitive.hpp"
#include "vm/vm.hpp"


/**
 * Returns a boxed element of the given Java array.
 */
java_handle_t* Array::get_boxed_element(int32_t index)
{
	vftbl_t       *v;
	int            type;
	imm_union      value;
	java_handle_t *o;

	if (is_null()) {
		exceptions_throw_nullpointerexception();
		return NULL;
	}

	v = LLNI_vftbl_direct(_handle);

	type = v->arraydesc->arraytype;

	value = get_primitive_element(index);

	o = Primitive::box(type, value);

	return o;
}


/**
 * Sets a boxed element in the given Java array.
 */
void Array::set_boxed_element(int32_t index, java_handle_t *o)
{
	vftbl_t  *v;
	int       type;
	imm_union value;

	if (is_null()) {
		exceptions_throw_nullpointerexception();
		return;
	}

	v = LLNI_vftbl_direct(_handle);

	type = v->arraydesc->arraytype;

	// Special handling for object arrays.
	if (type == ARRAYTYPE_OBJECT) {
		ObjectArray array(_handle);
		array.set_element(index, o);
		return;
	}

	// Check if primitive type can be stored.
	if (!Primitive::unbox_typed(o, type, &value)) {
/*		exceptions_throw_illegalargumentexception("argument type mismatch"); */
		exceptions_throw_illegalargumentexception();
		return;
	}

	set_primitive_element(index, value);
}


/**
 * Returns a primitive element of the given Java array.
 */
imm_union Array::get_primitive_element(int32_t index)
{
	vftbl_t  *v;
	int       type;
	imm_union value;

	if (is_null()) {
		exceptions_throw_nullpointerexception();
		value.a = NULL;
		return value;
	}

	java_handle_array_t* a = _handle;

	v = LLNI_vftbl_direct(a);

	type = v->arraydesc->arraytype;

	switch (type) {
	case ARRAYTYPE_BOOLEAN:
		{
			BooleanArray array(a);
			value.i = array.get_element(index);
		}
		break;
	case ARRAYTYPE_BYTE:
		{
			ByteArray array(a);
			value.i = array.get_element(index);
		}
		break;
	case ARRAYTYPE_CHAR:
		{
			CharArray array(a);
			value.i = array.get_element(index);
		}
		break;
	case ARRAYTYPE_SHORT:
		{
			ShortArray array(a);
			value.i = array.get_element(index);
		}
		break;
	case ARRAYTYPE_INT:
		{
			IntArray array(a);
			value.i = array.get_element(index);
		}
		break;
	case ARRAYTYPE_LONG:
		{
			LongArray array(a);
			value.l = array.get_element(index);
		}
		break;
	case ARRAYTYPE_FLOAT:
		{
			FloatArray array(a);
			value.f = array.get_element(index);
		}
		break;
	case ARRAYTYPE_DOUBLE:
		{
			DoubleArray array(a);
			value.d = array.get_element(index);
		}
		break;
	case ARRAYTYPE_OBJECT:
		{
			ObjectArray array(a);
			value.a = array.get_element(index);
		}
		break;
	default:
		vm_abort("Array::primitive_element_get: invalid array element type %d",
				 type);
	}

	return value;
}


/**
 * Sets a primitive element in the given Java array.
 */
void Array::set_primitive_element(int32_t index, imm_union value)
{
	vftbl_t *v;
	int      type;

	if (is_null()) {
		exceptions_throw_nullpointerexception();
		return;
	}

	java_handle_array_t* a = _handle;

	v = LLNI_vftbl_direct(a);

	type = v->arraydesc->arraytype;

	switch (type) {
	case ARRAYTYPE_BOOLEAN:
		{
			BooleanArray array(a);
			array.set_element(index, value.i);
		}
		break;
	case ARRAYTYPE_BYTE:
		{
			ByteArray array(a);
			array.set_element(index, value.i);
		}
		break;
	case ARRAYTYPE_CHAR:
		{
			CharArray array(a);
			array.set_element(index, value.i);
		}
		break;
	case ARRAYTYPE_SHORT:
		{
			ShortArray array(a);
			array.set_element(index, value.i);
		}
		break;
	case ARRAYTYPE_INT:
		{
			IntArray array(a);
			array.set_element(index, value.i);
		}
		break;
	case ARRAYTYPE_LONG:
		{
			LongArray array(a);
			array.set_element(index, value.l);
		}
		break;
	case ARRAYTYPE_FLOAT:
		{
			FloatArray array(a);
			array.set_element(index, value.f);
		}
		break;
	case ARRAYTYPE_DOUBLE:
		{
			DoubleArray array(a);
			array.set_element(index, value.d);
		}
		break;
	case ARRAYTYPE_OBJECT:
		{
			ObjectArray array(a);
			array.set_element(index, static_cast<java_handle_t*>(value.a));
		}
		break;
	default:
		vm_abort("array_element_primitive_set: invalid array element type %d",
				 type);
	}
}


/**
 * Creates an array of references to the given class type on the heap.
 * The handle pointer to the array can be NULL in case of an exception.
 */
ObjectArray::ObjectArray(int32_t length, classinfo* componentclass)
		: ArrayTemplate<java_handle_t*>(NULL)
{
	// Is class loaded?
	assert(componentclass->state & CLASS_LOADED);

	// Is class linked?
	if (!(componentclass->state & CLASS_LINKED))
		if (!link_class(componentclass)) {
			_handle = NULL;
			return;
		}

	classinfo* arrayclass = class_array_of(componentclass, true);

	if (arrayclass == NULL) {
		_handle = NULL;
		return;
	}

	// Delegate allocation to generic array class
	Array a(length, arrayclass);

	_handle = a.get_handle();
}


/**
 * Creates an array of references to classinfos on the heap.
 * The handle pointer to the array can be NULL in case of an exception.
 */
ClassArray::ClassArray(int32_t length)
		: ArrayTemplate<classinfo*>(NULL)
{
	// Delegate allocation to object array class
	ObjectArray oa(length, class_java_lang_Class);

	_handle = oa.get_handle();
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
