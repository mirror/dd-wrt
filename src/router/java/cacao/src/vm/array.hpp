/* src/vm/array.hpp - Java array functions

   Copyright (C) 1996-2014
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


#ifndef ARRAY_HPP_
#define ARRAY_HPP_ 1

#include "config.h"
#include <cassert>                      // for assert
#include <cstdio>                       // for NULL, printf
#include <stdint.h>                     // for int32_t, int8_t, int16_t, etc
#include "mm/gc.hpp"                    // for GCCriticalSection, etc
#include "threads/lockword.hpp"         // for Lockword
#include "vm/class.hpp"                 // for classinfo
#include "vm/exceptions.hpp"
#include "vm/global.hpp"                // for java_handle_t, java_array_t, etc
#include "vm/jit/builtin.hpp"           // for builtin_canstore
#include "vm/os.hpp"                    // for os
#include "vm/primitive.hpp"             // for primitivetypeinfo, etc
#include "vm/types.hpp"                 // for s4, s2
#include "vm/vftbl.hpp"                 // for vftbl_t

/* array types ****************************************************************/

/* CAUTION: Don't change the numerical values! These constants (with
   the exception of ARRAYTYPE_OBJECT) are used as indices in the
   primitive type table. */

enum ArrayType {
	ARRAYTYPE_INT     = PRIMITIVETYPE_INT,
	ARRAYTYPE_LONG    = PRIMITIVETYPE_LONG,
	ARRAYTYPE_FLOAT   = PRIMITIVETYPE_FLOAT,
	ARRAYTYPE_DOUBLE  = PRIMITIVETYPE_DOUBLE,
	ARRAYTYPE_BYTE    = PRIMITIVETYPE_BYTE,
	ARRAYTYPE_CHAR    = PRIMITIVETYPE_CHAR,
	ARRAYTYPE_SHORT   = PRIMITIVETYPE_SHORT,
	ARRAYTYPE_BOOLEAN = PRIMITIVETYPE_BOOLEAN,
	ARRAYTYPE_OBJECT  = PRIMITIVETYPE_VOID     // don't use as index!
};


/* arraydescriptor *************************************************************

   For every array class an arraydescriptor is allocated which
   describes the array class. The arraydescriptor is referenced from
   the vftbl of the array class.

*******************************************************************************/

struct arraydescriptor {
	vftbl_t   *componentvftbl; /* vftbl of the component type, NULL for primit. */
	vftbl_t   *elementvftbl;   /* vftbl of the element type, NULL for primitive */
	ArrayType  arraytype;      /* ARRAYTYPE_* constant                          */
	ArrayType  elementtype;    /* ARRAYTYPE_* constant                          */
	s4         dataoffset;     /* offset of the array data from object pointer  */
	s4         componentsize;  /* size of a component in bytes                  */
	s2         dimension;      /* dimension of the array (always >= 1)          */
};


/**
 * This is a generic accessor class for Java arrays (of unspecified type),
 * which can be used to safely operate on Java arrays in native code.
 */
class Array {
protected:
	// Handle of Java array.
	java_handle_array_t* _handle;

private:
	// We don't want a Java arrays to be copied.
	Array(Array* a) {}
	Array(Array& a) {}

public:
	Array(java_handle_t* h);
	Array(int32_t length, classinfo* arrayclass);
	virtual ~Array() {}

	// Getters.
	virtual java_handle_array_t* get_handle() const { return _handle; }
	int32_t                      get_length() const;

	// Null checks.
	bool is_null    () const;
	bool is_non_null() const;

	// Safe element modification functions for primitive values
	imm_union get_primitive_element(int32_t index);
	void      set_primitive_element(int32_t index, imm_union value);

	// Safe element modification functions for boxed values
	java_handle_t* get_boxed_element(int32_t index);
	void           set_boxed_element(int32_t index, java_handle_t *o);
};


/**
 * Constructor checks if passed handle really is a Java array.
 */
inline Array::Array(java_handle_t* h)
{
	if (h == NULL) {
		_handle = NULL;
		return;
	}

#if 0
	classinfo* c;
	LLNI_class_get(h, c);
	if (!class_is_array(c)) {
		printf("Array::Array(): WARNING, passed handle is not an array\n");
 		//exceptions_throw_illegalargumentexception("Argument is not an array");
		exceptions_throw_illegalargumentexception();
		_handle = NULL;
		return;
	}
#endif

	_handle = h;
}

/**
 * Creates an array of the given array type on the heap.
 * The handle pointer to the array can be NULL in case of an exception.
 */
inline Array::Array(int32_t size, classinfo* arrayclass)
{
	// Sanity check.
	assert(class_is_array(arrayclass));

	if (size < 0) {
		exceptions_throw_negativearraysizeexception();
		_handle = NULL;
		return;
	}

	arraydescriptor* desc          = arrayclass->vftbl->arraydesc;
	int32_t          dataoffset    = desc->dataoffset;
	int32_t          componentsize = desc->componentsize;
	int32_t          actualsize    = dataoffset + size * componentsize;

	// Check for overflow.

	if (((u4) actualsize) < ((u4) size)) {
		exceptions_throw_outofmemoryerror();
		_handle = NULL;
		return;
	}

	java_array_t* a = (java_array_t*) heap_alloc(actualsize, (desc->arraytype == ARRAYTYPE_OBJECT), NULL, true);

	if (a == NULL) {
		_handle = NULL;
		return;
	}

	LLNI_vftbl_direct(a) = arrayclass->vftbl;

	Lockword(a->objheader.lockword).init();

	a->size = size;

	_handle = (java_handle_array_t*) a;
}

inline int32_t Array::get_length() const
{
	if (is_null()) {
		printf("Array::get_length(): WARNING, got null-pointer\n");
		exceptions_throw_nullpointerexception();
		return -1;
	}

	// XXX Fix me!
	int32_t length = ((java_array_t*) _handle)->size;

	return length;
}

inline bool Array::is_null() const
{
	return (_handle == NULL);
}

inline bool Array::is_non_null() const
{
	return (_handle != NULL);
}


/**
 * This is a template of an accessor class for Java arrays
 * of a specific type.
 */
template<class T> class ArrayTemplate : public Array {
protected:
	ArrayTemplate(int32_t length, classinfo* arrayclass) : Array(length, arrayclass) {}

public:
	ArrayTemplate(java_handle_array_t* h) : Array(h) {}

	// XXX This should be protected or private!
	virtual T* get_raw_data_ptr() = 0;

	// Safe element modification functions
	T    get_element(int32_t index);
	void set_element(int32_t index, T value);

	// Region copy functions
	void get_region(int32_t offset, int32_t count, T* buffer);
	void set_region(int32_t offset, int32_t count, const T* buffer);
};


template<class T> inline T ArrayTemplate<T>::get_element(int32_t index)
{
	if (is_null()) {
		exceptions_throw_nullpointerexception();
		return 0;
	}

	if ((index < 0) || (index >= get_length())) {
		exceptions_throw_arrayindexoutofboundsexception();
		return 0;
	}

	T* ptr = get_raw_data_ptr();

	return ptr[index];
}

template<class T> inline void ArrayTemplate<T>::set_element(int32_t index, T value)
{
	if (is_null()) {
		exceptions_throw_nullpointerexception();
		return;
	}

	if ((index < 0) || (index >= get_length())) {
		exceptions_throw_arrayindexoutofboundsexception();
		return;
	}

	T* ptr = get_raw_data_ptr();

	ptr[index] = value;
}

template<> inline void ArrayTemplate<java_handle_t*>::set_element(int32_t index, java_handle_t* value)
{
	if (is_null()) {
		exceptions_throw_nullpointerexception();
		return;
	}

	// Sanity check.
	assert(((java_array_t*) get_handle())->objheader.vftbl->arraydesc->arraytype == ARRAYTYPE_OBJECT);

	// Check if value can be stored
	if (!builtin_canstore(get_handle(), value)) {
		exceptions_throw_illegalargumentexception();
		return;
	}

	if ((index < 0) || (index >= get_length())) {
		exceptions_throw_arrayindexoutofboundsexception();
		return;
	}

	java_handle_t** ptr = get_raw_data_ptr();

	ptr[index] = value;
}

template<class T> inline void ArrayTemplate<T>::get_region(int32_t offset, int32_t count, T* buffer)
{
	// Copy the array region inside a GC critical section.
	GCCriticalSection cs;

	const T* ptr = get_raw_data_ptr();

	os::memcpy(buffer, ptr + offset, sizeof(T) * count);
}

template<class T> inline void ArrayTemplate<T>::set_region(int32_t offset, int32_t count, const T* buffer)
{
	// Copy the array region inside a GC critical section.
	GCCriticalSection cs;

	T* ptr = get_raw_data_ptr();

	os::memcpy(ptr + offset, buffer, sizeof(T) * count);
}


/**
 * Actual implementations of common Java array access classes.
 */
class BooleanArray : public ArrayTemplate<uint8_t> {
public:
	BooleanArray(java_handle_booleanarray_t* h) : ArrayTemplate<uint8_t>(h) {}
	BooleanArray(int32_t length) : ArrayTemplate<uint8_t>(length, primitivetype_table[ARRAYTYPE_BOOLEAN].arrayclass) {}
	uint8_t* get_raw_data_ptr() { return ((java_booleanarray_t*) get_handle())->data; }
};

class ByteArray : public ArrayTemplate<int8_t> {
public:
	ByteArray(java_handle_bytearray_t* h) : ArrayTemplate<int8_t>(h) {}
	ByteArray(int32_t length) : ArrayTemplate<int8_t>(length, primitivetype_table[ARRAYTYPE_BYTE].arrayclass) {}
	int8_t* get_raw_data_ptr() { return (int8_t*) ((java_bytearray_t*) get_handle())->data; }
};

class CharArray : public ArrayTemplate<uint16_t> {
public:
	CharArray(java_handle_chararray_t* h) : ArrayTemplate<uint16_t>(h) {}
	CharArray(int32_t length) : ArrayTemplate<uint16_t>(length, primitivetype_table[ARRAYTYPE_CHAR].arrayclass) {}
	uint16_t* get_raw_data_ptr() { return ((java_chararray_t*) get_handle())->data; }
};

class ShortArray : public ArrayTemplate<int16_t> {
public:
	ShortArray(java_handle_shortarray_t* h) : ArrayTemplate<int16_t>(h) {}
	ShortArray(int32_t length) : ArrayTemplate<int16_t>(length, primitivetype_table[ARRAYTYPE_SHORT].arrayclass) {}
	int16_t* get_raw_data_ptr() { return ((java_shortarray_t*) get_handle())->data; }
};

class IntArray : public ArrayTemplate<int32_t> {
public:
	IntArray(java_handle_intarray_t* h) : ArrayTemplate<int32_t>(h) {}
	IntArray(int32_t length) : ArrayTemplate<int32_t>(length, primitivetype_table[ARRAYTYPE_INT].arrayclass) {}
	int32_t* get_raw_data_ptr() { return ((java_intarray_t*) get_handle())->data; }
};

class LongArray : public ArrayTemplate<int64_t> {
public:
	LongArray(java_handle_longarray_t* h) : ArrayTemplate<int64_t>(h) {}
	LongArray(int32_t length) : ArrayTemplate<int64_t>(length, primitivetype_table[ARRAYTYPE_LONG].arrayclass) {}
	int64_t* get_raw_data_ptr() { return ((java_longarray_t*) get_handle())->data; }
};

class FloatArray : public ArrayTemplate<float> {
public:
	FloatArray(java_handle_floatarray_t* h) : ArrayTemplate<float>(h) {}
	FloatArray(int32_t length) : ArrayTemplate<float>(length, primitivetype_table[ARRAYTYPE_FLOAT].arrayclass) {}
	float* get_raw_data_ptr() { return ((java_floatarray_t*) get_handle())->data; }
};

class DoubleArray : public ArrayTemplate<double> {
public:
	DoubleArray(java_handle_doublearray_t* h) : ArrayTemplate<double>(h) {}
	DoubleArray(int32_t length) : ArrayTemplate<double>(length, primitivetype_table[ARRAYTYPE_DOUBLE].arrayclass) {}
	double* get_raw_data_ptr() { return ((java_doublearray_t*) get_handle())->data; }
};

/**
 * Actual implementation of access class for Java Object arrays.
 */
class ObjectArray : public ArrayTemplate<java_handle_t*> {
public:
	ObjectArray(java_handle_objectarray_t* h) : ArrayTemplate<java_handle_t*>(h) {}
	ObjectArray(int32_t length, classinfo* componentclass);
	java_handle_t** get_raw_data_ptr() { return ((java_objectarray_t*) get_handle())->data; }
};

/**
 * Actual implementation of access class for java.lang.Class arrays.
 */
class ClassArray : public ArrayTemplate<classinfo*> {
public:
	ClassArray(int32_t length);
	classinfo** get_raw_data_ptr() { return (classinfo**) ((java_objectarray_t*) get_handle())->data; }
};

#endif // ARRAY_HPP_


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
