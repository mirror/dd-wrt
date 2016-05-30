/* src/native/vm/gnuclasspath/java_lang_reflect_VMField.cpp

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


#include "config.h"

#include <assert.h>
#include <stdint.h>

#include "native/jni.hpp"
#include "native/llni.hpp"
#include "native/native.hpp"

#if defined(ENABLE_JNI_HEADERS)
# include "native/vm/include/java_lang_reflect_VMField.h"
#endif

#if defined(ENABLE_ANNOTATIONS)
// REMOVEME
# include "native/vm/reflection.hpp"
#endif

#include "vm/access.hpp"
#include "vm/descriptor.hpp"
#include "vm/exceptions.hpp"
#include "vm/global.hpp"
#include "vm/initialize.hpp"
#include "vm/javaobjects.hpp"
#include "vm/loader.hpp"
#include "vm/primitive.hpp"
#include "vm/resolve.hpp"
#include "vm/string.hpp"
#include "vm/utf8.hpp"
#include "vm/vm.hpp"

#include "vm/jit/builtin.hpp"

/* _field_access_check *********************************************************

   Checks if the field can be accessed.

   RETURN VALUE:
      true......field can be accessed, or
      false.....otherwise (maybe an Exception was thrown).

*******************************************************************************/

static bool _field_access_check(const java_lang_reflect_VMField& rvmf, fieldinfo *f, java_handle_t *o)
{
	// Check if we should bypass security checks (AccessibleObject).

	java_lang_reflect_Field rf(rvmf.get_f());
	int32_t override = rf.get_flag();

	if (override == false) {
		/* This function is always called like this:
		       [0] java.lang.reflect.VMField.xxx (Native Method)
		       [1] java.lang.reflect.Field.xxx
		       [2] <caller>
		*/

		if (!access_check_field(f, 2))
			return false;
	}

	/* some general checks */

	if (f->flags & ACC_STATIC) {
		/* initialize class if required */

		if (!(f->clazz->state & CLASS_INITIALIZED))
			if (!initialize_class(f->clazz))
				return false;

		/* everything is ok */

		return true;
	}
	else {
		/* obj is required for not-static fields */

		if (o == NULL) {
			exceptions_throw_nullpointerexception();
			return false;
		}
	
		if (builtin_instanceof(o, f->clazz))
			return true;
	}

	/* exception path */

	exceptions_throw_illegalargumentexception();
	return false;
}


/* _field_get_type *************************************************************

   Returns the content of the given field.

*******************************************************************************/

#define _FIELD_GET_TYPE(name, type, uniontype) \
static inline type _field_get_##name(fieldinfo *f, java_handle_t* h) \
{ \
	type ret; \
	if (f->flags & ACC_STATIC) { \
		ret = f->value->uniontype; \
	} else { \
		LLNI_CRITICAL_START; \
		ret = *(type *) (((intptr_t) LLNI_DIRECT(h)) + f->offset); \
		LLNI_CRITICAL_END; \
	} \
	return ret; \
}

static inline java_handle_t *_field_get_handle(fieldinfo *f, java_handle_t* h)
{
	java_object_t* result;
	java_handle_t* hresult;

	LLNI_CRITICAL_START;

	if (f->flags & ACC_STATIC) {
		result = (java_object_t*) f->value->a;
	} else {
		result = *(java_object_t**) (((intptr_t) LLNI_DIRECT(h)) + f->offset);
	}

	hresult = LLNI_WRAP(result);

	LLNI_CRITICAL_END;

	return hresult;
}

_FIELD_GET_TYPE(int,    int32_t, i)
_FIELD_GET_TYPE(long,   int64_t, l)
_FIELD_GET_TYPE(float,  float,   f)
_FIELD_GET_TYPE(double, double,  d)


/* _field_set_type *************************************************************

   Sets the content of the given field to the given value.

*******************************************************************************/

#define _FIELD_SET_TYPE(name, type, uniontype) \
static inline void _field_set_##name(fieldinfo* f, java_handle_t* h, type value) \
{ \
	if (f->flags & ACC_STATIC) { \
		f->value->uniontype = value; \
	} else { \
		LLNI_CRITICAL_START; \
		*(type *) (((intptr_t) LLNI_DIRECT(h)) + f->offset) = value; \
		LLNI_CRITICAL_END; \
	} \
}

static inline void _field_set_handle(fieldinfo* f, java_handle_t* h, java_handle_t* hvalue)
{
	LLNI_CRITICAL_START;

	if (f->flags & ACC_STATIC) {
		f->value->a = LLNI_DIRECT(hvalue);
	} else {
		*(java_object_t**) (((intptr_t) LLNI_DIRECT(h)) + f->offset) = LLNI_DIRECT(hvalue);
	}

	LLNI_CRITICAL_END;
}

_FIELD_SET_TYPE(int,    int32_t, i)
_FIELD_SET_TYPE(long,   int64_t, l)
_FIELD_SET_TYPE(float,  float,   f)
_FIELD_SET_TYPE(double, double,  d)


// Native functions are exported as C functions.
extern "C" {

/*
 * Class:     java/lang/reflect/VMField
 * Method:    getModifiersInternal
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_java_lang_reflect_VMField_getModifiersInternal(JNIEnv *env, jobject _this)
{
	java_lang_reflect_VMField rvmf(_this);
	fieldinfo* f = rvmf.get_field();
	return f->flags;
}


/*
 * Class:     java/lang/reflect/VMField
 * Method:    getType
 * Signature: ()Ljava/lang/Class;
 */
JNIEXPORT jclass JNICALL Java_java_lang_reflect_VMField_getType(JNIEnv *env, jobject _this)
{
	java_lang_reflect_VMField rvmf(_this);
	fieldinfo* f = rvmf.get_field();
	classinfo *ret;

	typedesc* desc = f->parseddesc;

	if (desc == NULL)
		return NULL;

	if (!resolve_class_from_typedesc(desc, true, false, &ret))
		return NULL;
	
	return (jclass) LLNI_classinfo_wrap(ret);
}


/*
 * Class:     java/lang/reflect/VMField
 * Method:    get
 * Signature: (Ljava/lang/Object;)Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL Java_java_lang_reflect_VMField_get(JNIEnv *env, jobject _this, jobject o)
{
	java_lang_reflect_VMField rvmf(_this);
	fieldinfo* f = rvmf.get_field();

	// Check if the field can be accessed.
	if (!_field_access_check(rvmf, f, o))
		return NULL;

	imm_union value;

	switch (f->parseddesc->primitivetype) {
	case PRIMITIVETYPE_BOOLEAN:
	case PRIMITIVETYPE_BYTE:
	case PRIMITIVETYPE_CHAR:
	case PRIMITIVETYPE_SHORT:
	case PRIMITIVETYPE_INT:
		value.i = _field_get_int(f, o);
		break;

	case PRIMITIVETYPE_LONG:
		value.l = _field_get_long(f, o);
		break;

	case PRIMITIVETYPE_FLOAT:
		value.f = _field_get_float(f, o);
		break;

	case PRIMITIVETYPE_DOUBLE:
		value.d = _field_get_double(f, o);
		break;

	case TYPE_ADR:
		return (jobject) _field_get_handle(f, o);

	default:
		assert(false);
		break;
	}

	// Now box the primitive types.
	java_handle_t* object = Primitive::box(f->parseddesc->primitivetype, value);

	return object;
}


/*
 * Class:     java/lang/reflect/VMField
 * Method:    getBoolean
 * Signature: (Ljava/lang/Object;)Z
 */
JNIEXPORT jboolean JNICALL Java_java_lang_reflect_VMField_getBoolean(JNIEnv *env, jobject _this, jobject o)
{
	java_lang_reflect_VMField rvmf(_this);
	fieldinfo* f = rvmf.get_field();

	// Check if the field can be accessed.
	if (!_field_access_check(rvmf, f, o))
		return 0;

	// Check the field type and return the value.
	switch (f->parseddesc->primitivetype) {
	case PRIMITIVETYPE_BOOLEAN:
		return (jint) _field_get_int(f, o);
	default:
		exceptions_throw_illegalargumentexception();
		return 0;
	}
}


/*
 * Class:     java/lang/reflect/VMField
 * Method:    getByte
 * Signature: (Ljava/lang/Object;)B
 */
JNIEXPORT jbyte JNICALL Java_java_lang_reflect_VMField_getByte(JNIEnv *env, jobject _this, jobject o)
{
	java_lang_reflect_VMField rvmf(_this);
	fieldinfo* f = rvmf.get_field();

	// Check if the field can be accessed.
	if (!_field_access_check(rvmf, f, o))
		return 0;

	// Check the field type and return the value.
	switch (f->parseddesc->primitivetype) {
	case PRIMITIVETYPE_BYTE:
		return (jint) _field_get_int(f, o);
	default:
		exceptions_throw_illegalargumentexception();
		return 0;
	}
}


/*
 * Class:     java/lang/reflect/VMField
 * Method:    getChar
 * Signature: (Ljava/lang/Object;)C
 */
JNIEXPORT jchar JNICALL Java_java_lang_reflect_VMField_getChar(JNIEnv *env, jobject _this, jobject o)
{
	java_lang_reflect_VMField rvmf(_this);
	fieldinfo* f = rvmf.get_field();

	// Check if the field can be accessed.
	if (!_field_access_check(rvmf, f, o))
		return 0;

	// Check the field type and return the value.
	switch (f->parseddesc->primitivetype) {
	case PRIMITIVETYPE_CHAR:
		return (jint) _field_get_int(f, o);
	default:
		exceptions_throw_illegalargumentexception();
		return 0;
	}
}


/*
 * Class:     java/lang/reflect/VMField
 * Method:    getShort
 * Signature: (Ljava/lang/Object;)S
 */
JNIEXPORT jshort JNICALL Java_java_lang_reflect_VMField_getShort(JNIEnv *env, jobject _this, jobject o)
{
	java_lang_reflect_VMField rvmf(_this);
	fieldinfo* f = rvmf.get_field();

	// Check if the field can be accessed.
	if (!_field_access_check(rvmf, f, o))
		return 0;

	// Check the field type and return the value.
	switch (f->parseddesc->primitivetype) {
	case PRIMITIVETYPE_BYTE:
	case PRIMITIVETYPE_SHORT:
		return (jint) _field_get_int(f, o);
	default:
		exceptions_throw_illegalargumentexception();
		return 0;
	}
}


/*
 * Class:     java/lang/reflect/VMField
 * Method:    getInt
 * Signature: (Ljava/lang/Object;)I
 */
JNIEXPORT jint JNICALL Java_java_lang_reflect_VMField_getInt(JNIEnv *env , jobject _this, jobject o)
{
	java_lang_reflect_VMField rvmf(_this);
	fieldinfo* f = rvmf.get_field();

	// Check if the field can be accessed.
	if (!_field_access_check(rvmf, f, o))
		return 0;

	// Check the field type and return the value.
	switch (f->parseddesc->primitivetype) {
	case PRIMITIVETYPE_BYTE:
	case PRIMITIVETYPE_CHAR:
	case PRIMITIVETYPE_SHORT:
	case PRIMITIVETYPE_INT:
		return (jint) _field_get_int(f, o);
	default:
		exceptions_throw_illegalargumentexception();
		return 0;
	}
}


/*
 * Class:     java/lang/reflect/VMField
 * Method:    getLong
 * Signature: (Ljava/lang/Object;)J
 */
JNIEXPORT jlong JNICALL Java_java_lang_reflect_VMField_getLong(JNIEnv *env, jobject _this, jobject o)
{
	java_lang_reflect_VMField rvmf(_this);
	fieldinfo* f = rvmf.get_field();

	// Check if the field can be accessed.
	if (!_field_access_check(rvmf, f, o))
		return 0;

	// Check the field type and return the value.
	switch (f->parseddesc->primitivetype) {
	case PRIMITIVETYPE_BYTE:
	case PRIMITIVETYPE_CHAR:
	case PRIMITIVETYPE_SHORT:
	case PRIMITIVETYPE_INT:
		return (jlong) _field_get_int(f, o);
	case PRIMITIVETYPE_LONG:
		return (jlong) _field_get_long(f, o);
	default:
		exceptions_throw_illegalargumentexception();
		return 0;
	}
}


/*
 * Class:     java/lang/reflect/VMField
 * Method:    getFloat
 * Signature: (Ljava/lang/Object;)F
 */
JNIEXPORT jfloat JNICALL Java_java_lang_reflect_VMField_getFloat(JNIEnv *env, jobject _this, jobject o)
{
	java_lang_reflect_VMField rvmf(_this);
	fieldinfo* f = rvmf.get_field();

	// Check if the field can be accessed.
	if (!_field_access_check(rvmf, f, o))
		return 0;

	// Check the field type and return the value.
	switch (f->parseddesc->primitivetype) {
	case PRIMITIVETYPE_BYTE:
	case PRIMITIVETYPE_CHAR:
	case PRIMITIVETYPE_SHORT:
	case PRIMITIVETYPE_INT:
		return (jfloat) _field_get_int(f, o);
	case PRIMITIVETYPE_LONG:
		return (jfloat) _field_get_long(f, o);
	case PRIMITIVETYPE_FLOAT:
		return (jfloat) _field_get_float(f, o);
	default:
		exceptions_throw_illegalargumentexception();
		return 0;
	}
}


/*
 * Class:     java/lang/reflect/VMField
 * Method:    getDouble
 * Signature: (Ljava/lang/Object;)D
 */
JNIEXPORT jdouble JNICALL Java_java_lang_reflect_VMField_getDouble(JNIEnv *env , jobject _this, jobject o)
{
	java_lang_reflect_VMField rvmf(_this);
	fieldinfo* f = rvmf.get_field();

	// Check if the field can be accessed.
	if (!_field_access_check(rvmf, f, o))
		return 0;

	// Check the field type and return the value.
	switch (f->parseddesc->primitivetype) {
	case PRIMITIVETYPE_BYTE:
	case PRIMITIVETYPE_CHAR:
	case PRIMITIVETYPE_SHORT:
	case PRIMITIVETYPE_INT:
		return (jdouble) _field_get_int(f, o);
	case PRIMITIVETYPE_LONG:
		return (jdouble) _field_get_long(f, o);
	case PRIMITIVETYPE_FLOAT:
		return (jdouble) _field_get_float(f, o);
	case PRIMITIVETYPE_DOUBLE:
		return (jdouble) _field_get_double(f, o);
	default:
		exceptions_throw_illegalargumentexception();
		return 0;
	}
}


/*
 * Class:     java/lang/reflect/VMField
 * Method:    set
 * Signature: (Ljava/lang/Object;Ljava/lang/Object;)V
 */
JNIEXPORT void JNICALL Java_java_lang_reflect_VMField_set(JNIEnv *env, jobject _this, jobject o, jobject value)
{
	java_lang_reflect_VMField rvmf(_this);
	fieldinfo* df = rvmf.get_field();

	classinfo *sc;
	fieldinfo *sf;

	// Check if the field can be accessed.
	if (!_field_access_check(rvmf, df, o))
		return;

	// Get the source classinfo from the object.
	if (value == NULL)
		sc = NULL;
	else
		LLNI_class_get(value, sc);

	/* The fieldid is used to set the new value, for primitive
	   types the value has to be retrieved from the wrapping
	   object */

	switch (df->parseddesc->primitivetype) {
	case PRIMITIVETYPE_BOOLEAN: {
		int32_t val;

		/* determine the field to read the value */

		if ((sc == NULL) || !(sf = class_findfield(sc, utf8::value, utf8::Z)))
			break;

		switch (sf->parseddesc->primitivetype) {
		case PRIMITIVETYPE_BOOLEAN:
			val = java_lang_Boolean(value).get_value();
			break;
		default:
			exceptions_throw_illegalargumentexception();
			return;
		}

		_field_set_int(df, o, val);
		return;
	}

	case PRIMITIVETYPE_BYTE: {
		int32_t val;

		if ((sc == NULL) || !(sf = class_findfield(sc, utf8::value, utf8::B)))
			break;

		switch (sf->parseddesc->primitivetype) {
		case PRIMITIVETYPE_BYTE:
			val = java_lang_Byte(value).get_value();
			break;
		default:	
			exceptions_throw_illegalargumentexception();
			return;
		}

		_field_set_int(df, o, val);
		return;
	}

	case PRIMITIVETYPE_CHAR: {
		int32_t val;

		if ((sc == NULL) || !(sf = class_findfield(sc, utf8::value, utf8::C)))
			break;
				   
		switch (sf->parseddesc->primitivetype) {
		case PRIMITIVETYPE_CHAR:
			val = java_lang_Character(value).get_value();
			break;
		default:
			exceptions_throw_illegalargumentexception();
			return;
		}

		_field_set_int(df, o, val);
		return;
	}

	case PRIMITIVETYPE_SHORT: {
		int32_t val;

		/* get field only by name, it can be one of B, S */

		if ((sc == NULL) || !(sf = class_findfield_by_name(sc, utf8::value)))
			break;
				   
		switch (sf->parseddesc->primitivetype) {
		case PRIMITIVETYPE_BYTE:
			val = java_lang_Byte(value).get_value();
			break;
		case PRIMITIVETYPE_SHORT:
			val = java_lang_Short(value).get_value();
			break;
		default:
			exceptions_throw_illegalargumentexception();
			return;
		}

		_field_set_int(df, o, val);
		return;
	}

	case PRIMITIVETYPE_INT: {
		int32_t val;

		/* get field only by name, it can be one of B, S, C, I */

		if ((sc == NULL) || !(sf = class_findfield_by_name(sc, utf8::value)))
			break;

		switch (sf->parseddesc->primitivetype) {
		case PRIMITIVETYPE_BYTE:
			val = java_lang_Byte(value).get_value();
			break;
		case PRIMITIVETYPE_CHAR:
			val = java_lang_Character(value).get_value();
			break;
		case PRIMITIVETYPE_SHORT:
			val = java_lang_Short(value).get_value();
			break;
		case PRIMITIVETYPE_INT:
			val = java_lang_Integer(value).get_value();
			break;
		default:
			exceptions_throw_illegalargumentexception();
			return;
		}

		_field_set_int(df, o, val);
		return;
	}

	case PRIMITIVETYPE_LONG: {
		int64_t val;

		/* get field only by name, it can be one of B, S, C, I, J */

		if ((sc == NULL) || !(sf = class_findfield_by_name(sc, utf8::value)))
			break;

		switch (sf->parseddesc->primitivetype) {
		case PRIMITIVETYPE_BYTE:
			val = java_lang_Byte(value).get_value();
			break;
		case PRIMITIVETYPE_CHAR:
			val = java_lang_Character(value).get_value();
			break;
		case PRIMITIVETYPE_SHORT:
			val = java_lang_Short(value).get_value();
			break;
		case PRIMITIVETYPE_INT:
			val = java_lang_Integer(value).get_value();
			break;
		case PRIMITIVETYPE_LONG:
			val = java_lang_Long(value).get_value();
			break;
		default:
			exceptions_throw_illegalargumentexception();
			return;
		}

		_field_set_long(df, o, val);
		return;
	}

	case PRIMITIVETYPE_FLOAT: {
		float val;

		/* get field only by name, it can be one of B, S, C, I, J, F */

		if ((sc == NULL) || !(sf = class_findfield_by_name(sc, utf8::value)))
			break;

		switch (sf->parseddesc->primitivetype) {
		case PRIMITIVETYPE_BYTE:
			val = java_lang_Byte(value).get_value();
			break;
		case PRIMITIVETYPE_CHAR:
			val = java_lang_Character(value).get_value();
			break;
		case PRIMITIVETYPE_SHORT:
			val = java_lang_Short(value).get_value();
			break;
		case PRIMITIVETYPE_INT:
			val = java_lang_Integer(value).get_value();
			break;
		case PRIMITIVETYPE_LONG:
			val = java_lang_Long(value).get_value();
			break;
		case PRIMITIVETYPE_FLOAT:
			val = java_lang_Float(value).get_value();
			break;
		default:
			exceptions_throw_illegalargumentexception();
			return;
		}

		_field_set_float(df, o, val);
		return;
	}

	case PRIMITIVETYPE_DOUBLE: {
		double val;

		/* get field only by name, it can be one of B, S, C, I, J, F, D */

		if ((sc == NULL) || !(sf = class_findfield_by_name(sc, utf8::value)))
			break;

		switch (sf->parseddesc->primitivetype) {
		case PRIMITIVETYPE_BYTE:
			val = java_lang_Byte(value).get_value();
			break;
		case PRIMITIVETYPE_CHAR:
			val = java_lang_Character(value).get_value();
			break;
		case PRIMITIVETYPE_SHORT:
			val = java_lang_Short(value).get_value();
			break;
		case PRIMITIVETYPE_INT:
			val = java_lang_Integer(value).get_value();
			break;
		case PRIMITIVETYPE_LONG:
			val = java_lang_Long(value).get_value();
			break;
		case PRIMITIVETYPE_FLOAT:
			val = java_lang_Float(value).get_value();
			break;
		case PRIMITIVETYPE_DOUBLE:
			val = java_lang_Double(value).get_value();
			break;
		default:
			exceptions_throw_illegalargumentexception();
			return;
		}

		_field_set_double(df, o, val);
		return;
	}

	case TYPE_ADR:
		/* check if value is an instance of the destination class */

		/* XXX TODO */
		/*  			if (!builtin_instanceof((java_handle_t *) value, df->class)) */
		/*  				break; */

		_field_set_handle(df, o, value);
		return;

	default:
		assert(false);
		break;
	}

	/* raise exception */

	exceptions_throw_illegalargumentexception();
}


/*
 * Class:     java/lang/reflect/VMField
 * Method:    setBoolean
 * Signature: (Ljava/lang/Object;Z)V
 */
JNIEXPORT void JNICALL Java_java_lang_reflect_VMField_setBoolean(JNIEnv *env, jobject _this, jobject o, jboolean value)
{
	java_lang_reflect_VMField rvmf(_this);
	fieldinfo* f = rvmf.get_field();

	// Check if the field can be accessed.
	if (!_field_access_check(rvmf, f, o))
		return;

	// Check the field type and set the value.
	switch (f->parseddesc->primitivetype) {
	case PRIMITIVETYPE_BOOLEAN:
		_field_set_int(f, o, value);
		break;
	default:
		exceptions_throw_illegalargumentexception();
	}
}


/*
 * Class:     java/lang/reflect/VMField
 * Method:    setByte
 * Signature: (Ljava/lang/Object;B)V
 */
JNIEXPORT void JNICALL Java_java_lang_reflect_VMField_setByte(JNIEnv *env, jobject _this, jobject o, jbyte value)
{
	java_lang_reflect_VMField rvmf(_this);
	fieldinfo* f = rvmf.get_field();

	// Check if the field can be accessed.
	if (!_field_access_check(rvmf, f, o))
		return;

	// Check the field type and set the value.
	switch (f->parseddesc->primitivetype) {
	case PRIMITIVETYPE_BYTE:
	case PRIMITIVETYPE_SHORT:
	case PRIMITIVETYPE_INT:
		_field_set_int(f, o, value);
		break;
	case PRIMITIVETYPE_LONG:
		_field_set_long(f, o, value);
		break;
	case PRIMITIVETYPE_FLOAT:
		_field_set_float(f, o, value);
		break;
	case PRIMITIVETYPE_DOUBLE:
		_field_set_double(f, o, value);
		break;
	default:
		exceptions_throw_illegalargumentexception();
	}
}


/*
 * Class:     java/lang/reflect/VMField
 * Method:    setChar
 * Signature: (Ljava/lang/Object;C)V
 */
JNIEXPORT void JNICALL Java_java_lang_reflect_VMField_setChar(JNIEnv *env, jobject _this, jobject o, jchar value)
{
	java_lang_reflect_VMField rvmf(_this);
	fieldinfo* f = rvmf.get_field();

	// Check if the field can be accessed.
	if (!_field_access_check(rvmf, f, o))
		return;

	// Check the field type and set the value.
	switch (f->parseddesc->primitivetype) {
	case PRIMITIVETYPE_CHAR:
	case PRIMITIVETYPE_INT:
		_field_set_int(f, o, value);
		break;
	case PRIMITIVETYPE_LONG:
		_field_set_long(f, o, value);
		break;
	case PRIMITIVETYPE_FLOAT:
		_field_set_float(f, o, value);
		break;
	case PRIMITIVETYPE_DOUBLE:
		_field_set_double(f, o, value);
		break;
	default:
		exceptions_throw_illegalargumentexception();
	}
}


/*
 * Class:     java/lang/reflect/VMField
 * Method:    setShort
 * Signature: (Ljava/lang/Object;S)V
 */
JNIEXPORT void JNICALL Java_java_lang_reflect_VMField_setShort(JNIEnv *env, jobject _this, jobject o, jshort value)
{
	java_lang_reflect_VMField rvmf(_this);
	fieldinfo* f = rvmf.get_field();

	// Check if the field can be accessed.
	if (!_field_access_check(rvmf, f, o))
		return;

	// Check the field type and set the value.
	switch (f->parseddesc->primitivetype) {
	case PRIMITIVETYPE_SHORT:
	case PRIMITIVETYPE_INT:
		_field_set_int(f, o, value);
		break;
	case PRIMITIVETYPE_LONG:
		_field_set_long(f, o, value);
		break;
	case PRIMITIVETYPE_FLOAT:
		_field_set_float(f, o, value);
		break;
	case PRIMITIVETYPE_DOUBLE:
		_field_set_double(f, o, value);
		break;
	default:
		exceptions_throw_illegalargumentexception();
	}
}


/*
 * Class:     java/lang/reflect/VMField
 * Method:    setInt
 * Signature: (Ljava/lang/Object;I)V
 */
JNIEXPORT void JNICALL Java_java_lang_reflect_VMField_setInt(JNIEnv *env, jobject _this, jobject o, jint value)
{
	java_lang_reflect_VMField rvmf(_this);
	fieldinfo* f = rvmf.get_field();

	// Check if the field can be accessed.
	if (!_field_access_check(rvmf, f, o))
		return;

	// Check the field type and set the value.
	switch (f->parseddesc->primitivetype) {
	case PRIMITIVETYPE_INT:
		_field_set_int(f, o, value);
		break;
	case PRIMITIVETYPE_LONG:
		_field_set_long(f, o, value);
		break;
	case PRIMITIVETYPE_FLOAT:
		_field_set_float(f, o, value);
		break;
	case PRIMITIVETYPE_DOUBLE:
		_field_set_double(f, o, value);
		break;
	default:
		exceptions_throw_illegalargumentexception();
	}
}


/*
 * Class:     java/lang/reflect/VMField
 * Method:    setLong
 * Signature: (Ljava/lang/Object;J)V
 */
JNIEXPORT void JNICALL Java_java_lang_reflect_VMField_setLong(JNIEnv *env, jobject _this, jobject o, jlong value)
{
	java_lang_reflect_VMField rvmf(_this);
	fieldinfo* f = rvmf.get_field();

	// Check if the field can be accessed.
	if (!_field_access_check(rvmf, f, o))
		return;

	// Check the field type and set the value.
	switch (f->parseddesc->primitivetype) {
	case PRIMITIVETYPE_LONG:
		_field_set_long(f, o, value);
		break;
	case PRIMITIVETYPE_FLOAT:
		_field_set_float(f, o, value);
		break;
	case PRIMITIVETYPE_DOUBLE:
		_field_set_double(f, o, value);
		break;
	default:
		exceptions_throw_illegalargumentexception();
	}
}


/*
 * Class:     java/lang/reflect/VMField
 * Method:    setFloat
 * Signature: (Ljava/lang/Object;F)V
 */
JNIEXPORT void JNICALL Java_java_lang_reflect_VMField_setFloat(JNIEnv *env, jobject _this, jobject o, jfloat value)
{
	java_lang_reflect_VMField rvmf(_this);
	fieldinfo* f = rvmf.get_field();

	// Check if the field can be accessed.
	if (!_field_access_check(rvmf, f, o))
		return;

	// Check the field type and set the value.
	switch (f->parseddesc->primitivetype) {
	case PRIMITIVETYPE_FLOAT:
		_field_set_float(f, o, value);
		break;
	case PRIMITIVETYPE_DOUBLE:
		_field_set_double(f, o, value);
		break;
	default:
		exceptions_throw_illegalargumentexception();
	}
}


/*
 * Class:     java/lang/reflect/VMField
 * Method:    setDouble
 * Signature: (Ljava/lang/Object;D)V
 */
JNIEXPORT void JNICALL Java_java_lang_reflect_VMField_setDouble(JNIEnv *env, jobject _this, jobject o, jdouble value)
{
	java_lang_reflect_VMField rvmf(_this);
	fieldinfo* f = rvmf.get_field();

	// Check if the field can be accessed.
	if (!_field_access_check(rvmf, f, o))
		return;

	// Check the field type and set the value.
	switch (f->parseddesc->primitivetype) {
	case PRIMITIVETYPE_DOUBLE:
		_field_set_double(f, o, value);
		break;
	default:
		exceptions_throw_illegalargumentexception();
	}
}


/*
 * Class:     java/lang/reflect/VMField
 * Method:    getSignature
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_java_lang_reflect_VMField_getSignature(JNIEnv *env, jobject _this)
{
	java_lang_reflect_VMField rvmf(_this);
	fieldinfo* f = rvmf.get_field();

	if (f->signature == NULL)
		return NULL;

	java_handle_t* o = JavaString::from_utf8(f->signature);

	/* in error case o is NULL */

	return o;
}


#if defined(ENABLE_ANNOTATIONS)
/*
 * Class:     java/lang/reflect/VMField
 * Method:    declaredAnnotations
 * Signature: ()Ljava/util/Map;
 */
JNIEXPORT jobject JNICALL Java_java_lang_reflect_VMField_declaredAnnotations(JNIEnv *env, jobject _this)
{
	java_lang_reflect_VMField rvmf(_this);

	java_handle_t* declaredAnnotations = rvmf.get_declaredAnnotations();

	// Are the annotations parsed yet?
	if (declaredAnnotations == NULL) {
		java_handle_bytearray_t* annotations    = rvmf.get_annotations();
		classinfo*               declaringClass = rvmf.get_clazz();
		classinfo*               referer        = rvmf.get_Class();

		declaredAnnotations = Reflection::get_declaredannotations(annotations, declaringClass, referer);

		rvmf.set_declaredAnnotations(declaredAnnotations);
	}

	return (jobject) declaredAnnotations;
}
#endif

} // extern "C"


/* native methods implemented by this file ************************************/

static const JNINativeMethod methods[] = {
	{ (char*) "getModifiersInternal", (char*) "()I",                                     (void*) (uintptr_t) &Java_java_lang_reflect_VMField_getModifiersInternal },
	{ (char*) "getType",              (char*) "()Ljava/lang/Class;",                     (void*) (uintptr_t) &Java_java_lang_reflect_VMField_getType              },
	{ (char*) "get",                  (char*) "(Ljava/lang/Object;)Ljava/lang/Object;",  (void*) (uintptr_t) &Java_java_lang_reflect_VMField_get                  },
	{ (char*) "getBoolean",           (char*) "(Ljava/lang/Object;)Z",                   (void*) (uintptr_t) &Java_java_lang_reflect_VMField_getBoolean           },
	{ (char*) "getByte",              (char*) "(Ljava/lang/Object;)B",                   (void*) (uintptr_t) &Java_java_lang_reflect_VMField_getByte              },
	{ (char*) "getChar",              (char*) "(Ljava/lang/Object;)C",                   (void*) (uintptr_t) &Java_java_lang_reflect_VMField_getChar              },
	{ (char*) "getShort",             (char*) "(Ljava/lang/Object;)S",                   (void*) (uintptr_t) &Java_java_lang_reflect_VMField_getShort             },
	{ (char*) "getInt",               (char*) "(Ljava/lang/Object;)I",                   (void*) (uintptr_t) &Java_java_lang_reflect_VMField_getInt               },
	{ (char*) "getLong",              (char*) "(Ljava/lang/Object;)J",                   (void*) (uintptr_t) &Java_java_lang_reflect_VMField_getLong              },
	{ (char*) "getFloat",             (char*) "(Ljava/lang/Object;)F",                   (void*) (uintptr_t) &Java_java_lang_reflect_VMField_getFloat             },
	{ (char*) "getDouble",            (char*) "(Ljava/lang/Object;)D",                   (void*) (uintptr_t) &Java_java_lang_reflect_VMField_getDouble            },
	{ (char*) "set",                  (char*) "(Ljava/lang/Object;Ljava/lang/Object;)V", (void*) (uintptr_t) &Java_java_lang_reflect_VMField_set                  },
	{ (char*) "setBoolean",           (char*) "(Ljava/lang/Object;Z)V",                  (void*) (uintptr_t) &Java_java_lang_reflect_VMField_setBoolean           },
	{ (char*) "setByte",              (char*) "(Ljava/lang/Object;B)V",                  (void*) (uintptr_t) &Java_java_lang_reflect_VMField_setByte              },
	{ (char*) "setChar",              (char*) "(Ljava/lang/Object;C)V",                  (void*) (uintptr_t) &Java_java_lang_reflect_VMField_setChar              },
	{ (char*) "setShort",             (char*) "(Ljava/lang/Object;S)V",                  (void*) (uintptr_t) &Java_java_lang_reflect_VMField_setShort             },
	{ (char*) "setInt",               (char*) "(Ljava/lang/Object;I)V",                  (void*) (uintptr_t) &Java_java_lang_reflect_VMField_setInt               },
	{ (char*) "setLong",              (char*) "(Ljava/lang/Object;J)V",                  (void*) (uintptr_t) &Java_java_lang_reflect_VMField_setLong              },
	{ (char*) "setFloat",             (char*) "(Ljava/lang/Object;F)V",                  (void*) (uintptr_t) &Java_java_lang_reflect_VMField_setFloat             },
	{ (char*) "setDouble",            (char*) "(Ljava/lang/Object;D)V",                  (void*) (uintptr_t) &Java_java_lang_reflect_VMField_setDouble            },
	{ (char*) "getSignature",         (char*) "()Ljava/lang/String;",                    (void*) (uintptr_t) &Java_java_lang_reflect_VMField_getSignature         },
#if defined(ENABLE_ANNOTATIONS)
	{ (char*) "declaredAnnotations",  (char*) "()Ljava/util/Map;",                       (void*) (uintptr_t) &Java_java_lang_reflect_VMField_declaredAnnotations  },
#endif
};


/* _Jv_java_lang_reflect_VMField_init ******************************************

   Register native functions.

*******************************************************************************/

void _Jv_java_lang_reflect_VMField_init(void)
{
	Utf8String u = Utf8String::from_utf8("java/lang/reflect/VMField");

	NativeMethods& nm = VM::get_current()->get_nativemethods();
	nm.register_methods(u, methods, NATIVE_METHODS_COUNT);
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
