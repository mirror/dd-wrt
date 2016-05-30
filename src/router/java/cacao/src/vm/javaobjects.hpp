/* src/vm/javaobjects.hpp - functions to create and access Java objects

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO
   Copyright (C) 2008, 2009 Theobroma Systems Ltd.

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


#ifndef JAVAOBJECTS_HPP_
#define JAVAOBJECTS_HPP_ 1

#include "config.h"

#include <stdint.h>

#include "mm/memory.hpp"

#include "native/jni.hpp"                      // for jclass, jsize
#include "native/llni.hpp"

#include "threads/atomic.hpp"

#include "vm/class.hpp"
#include "vm/field.hpp"
#include "vm/global.hpp"
#include "vm/globals.hpp"
#include "vm/method.hpp"
#include "vm/string.hpp"

#include "vm/array.hpp"

/**
 * This class provides low-level functions to access Java object
 * instance fields.
 *
 * These functions do NOT take care about the GC critical section!
 * Please use FieldAccess wherever possible.
 */
class RawFieldAccess {
protected:
	template<class T> static inline T    raw_get(void* address, const off_t offset);
	template<class T> static inline void raw_set(void* address, const off_t offset, T value);
};


template<class T> inline T RawFieldAccess::raw_get(void* address, const off_t offset)
{
	T* p = (T*) (((uintptr_t) address) + offset);
	return *p;
}


template<class T> inline void RawFieldAccess::raw_set(void* address, const off_t offset, T value)
{
	T* p = (T*) (((uintptr_t) address) + offset);
	*p = value;
}


/**
 * This classes provides functions to access Java object instance
 * fields.  These functions enter a critical GC section before
 * accessing the Java object throught the handle and leave it
 * afterwards.
 */
class FieldAccess : private RawFieldAccess {
public:
	// Normal field accessors.
	template<class T> static inline T    get(java_handle_t* h, const off_t offset);
	template<class T> static inline void set(java_handle_t* h, const off_t offset, T value);

	// Volatile field accessors.
	template<class T> static inline T    get_volatile(java_handle_t* h, const off_t offset);
	template<class T> static inline void set_volatile(java_handle_t* h, const off_t offset, T value);
};


template<class T> inline T FieldAccess::get(java_handle_t* h, const off_t offset)
{
	// This function is inside a critical section.
	GCCriticalSection cs;

	// XXX This should be _handle->get_object();
	java_object_t* ho = LLNI_UNWRAP(h);
	return raw_get<T>(ho, offset);
}

template<> inline java_handle_t* FieldAccess::get(java_handle_t* h, const off_t offset)
{
	// This function is inside a critical section.
	GCCriticalSection cs;

	// XXX This should be _handle->get_object();
	java_object_t* o = LLNI_UNWRAP(h);
	java_object_t* result = raw_get<java_object_t*>(o, offset);
	return LLNI_WRAP(result);
}	


template<class T> inline void FieldAccess::set(java_handle_t* h, const off_t offset, T value)
{
	// This function is inside a critical section.
	GCCriticalSection cs;

	java_object_t* ho = LLNI_UNWRAP(h);
	raw_set(ho, offset, value);
}

template<> inline void FieldAccess::set<java_handle_t*>(java_handle_t* h, const off_t offset, java_handle_t* value)
{
	// This function is inside a critical section.
	GCCriticalSection cs;

	// XXX This should be h->get_object();
	java_object_t* o      = LLNI_UNWRAP(h);
	java_object_t* ovalue = LLNI_UNWRAP(value);
	raw_set(o, offset, ovalue);
}


template<class T> inline T FieldAccess::get_volatile(java_handle_t* h, const off_t offset)
{
	// This function is inside a critical section.
	GCCriticalSection cs;

	// XXX This should be _handle->get_object();
	java_object_t* ho = LLNI_UNWRAP(h);
	return raw_get<volatile T>(ho, offset);
}

template<> inline java_handle_t* FieldAccess::get_volatile(java_handle_t* h, const off_t offset)
{
	// This function is inside a critical section.
	GCCriticalSection cs;

	// XXX This should be _handle->get_object();
	java_object_t* o = LLNI_UNWRAP(h);
	java_object_t* result = (java_object_t*) raw_get<volatile java_object_t*>(o, offset);
	return LLNI_WRAP(result);
}	


template<class T> inline void FieldAccess::set_volatile(java_handle_t* h, const off_t offset, T value)
{
	// This function is inside a critical section.
	GCCriticalSection cs;

	java_object_t* ho = LLNI_UNWRAP(h);
	raw_set(ho, offset, (volatile T) value);

	// Memory barrier for the Java Memory Model.
	Atomic::memory_barrier();
}

template<> inline void FieldAccess::set_volatile<java_handle_t*>(java_handle_t* h, const off_t offset, java_handle_t* value)
{
	// This function is inside a critical section.
	GCCriticalSection cs;

	// XXX This should be h->get_object();
	java_object_t* o      = LLNI_UNWRAP(h);
	java_object_t* ovalue = LLNI_UNWRAP(value);
	raw_set(o, offset, (volatile java_object_t*) ovalue);

	// Memory barrier for the Java Memory Model.
	Atomic::memory_barrier();
}


/**
 * java/lang/Object
 *
 * Object layout:
 *
 * 0. object header
 */
class java_lang_Object {
protected:
	// Handle of Java object.
	java_handle_t* _handle;

public:
	java_lang_Object() : _handle(NULL) {}
	java_lang_Object(java_handle_t* h) : _handle(h) {}
	virtual ~java_lang_Object() {}

	// Getters.
	virtual java_handle_t* get_handle  () const { return _handle; }
	vftbl_t*               get_vftbl   () const;
	classinfo*             get_Class   () const;
	int32_t                get_hashcode() const;

	bool is_null    () const;
	bool is_non_null() const;
};


inline vftbl_t* java_lang_Object::get_vftbl() const
{
	// This function is inside a critical section.
	GCCriticalSection cs;

	// XXX This should be h->get_object();
	java_object_t* o = LLNI_UNWRAP(_handle);
	return o->vftbl;
}

inline classinfo* java_lang_Object::get_Class() const
{
	return get_vftbl()->clazz;
}

inline int32_t java_lang_Object::get_hashcode() const
{
#if defined(ENABLE_GC_CACAO)
	return heap_get_hashcode(_handle);
#else
	// This function is inside a critical section.
	GCCriticalSection cs;

	// XXX This should be h->get_object();
	java_object_t* o = LLNI_UNWRAP(_handle);
	return (int32_t) (intptr_t) o;
#endif
}


inline bool java_lang_Object::is_null() const
{
	return (_handle == NULL);
}

inline bool java_lang_Object::is_non_null() const
{
	return (_handle != NULL);
}


/**
 * java/lang/Boolean
 *
 * Object layout:
 *
 * 0. object header
 * 1. boolean value;
 */
class java_lang_Boolean : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_value = MEMORY_ALIGN(sizeof(java_object_t), sizeof(int32_t));

public:
	java_lang_Boolean(java_handle_t* h) : java_lang_Object(h) {}

	uint8_t get_value();
	void    set_value(uint8_t value);
};

inline uint8_t java_lang_Boolean::get_value()
{
	return get<int32_t>(_handle, offset_value);
}

inline void java_lang_Boolean::set_value(uint8_t value)
{
	set(_handle, offset_value, (uint32_t) value);
}


/**
 * java/lang/Byte
 *
 * Object layout:
 *
 * 0. object header
 * 1. byte value;
 */
class java_lang_Byte : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_value = MEMORY_ALIGN(sizeof(java_object_t), sizeof(int32_t));

public:
	java_lang_Byte(java_handle_t* h) : java_lang_Object(h) {}

	int8_t get_value();
	void   set_value(int8_t value);
};

inline int8_t java_lang_Byte::get_value()
{
	return get<int32_t>(_handle, offset_value);
}

inline void java_lang_Byte::set_value(int8_t value)
{
	set(_handle, offset_value, (int32_t) value);
}


/**
 * java/lang/Character
 *
 * Object layout:
 *
 * 0. object header
 * 1. char value;
 */
class java_lang_Character : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_value = MEMORY_ALIGN(sizeof(java_object_t), sizeof(int32_t));

public:
	java_lang_Character(java_handle_t* h) : java_lang_Object(h) {}

	uint16_t get_value();
	void     set_value(uint16_t value);
};

inline uint16_t java_lang_Character::get_value()
{
	return get<int32_t>(_handle, offset_value);
}

inline void java_lang_Character::set_value(uint16_t value)
{
	set(_handle, offset_value, (uint32_t) value);
}


/**
 * java/lang/Short
 *
 * Object layout:
 *
 * 0. object header
 * 1. short value;
 */
class java_lang_Short : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_value = MEMORY_ALIGN(sizeof(java_object_t), sizeof(int32_t));

public:
	java_lang_Short(java_handle_t* h) : java_lang_Object(h) {}

	int16_t get_value();
	void    set_value(int16_t value);
};

inline int16_t java_lang_Short::get_value()
{
	return get<int32_t>(_handle, offset_value);
}

inline void java_lang_Short::set_value(int16_t value)
{
	set(_handle, offset_value, (int32_t) value);
}


/**
 * java/lang/Integer
 *
 * Object layout:
 *
 * 0. object header
 * 1. int value;
 */
class java_lang_Integer : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_value = MEMORY_ALIGN(sizeof(java_object_t), sizeof(int32_t));

public:
	java_lang_Integer(java_handle_t* h) : java_lang_Object(h) {}

	int32_t get_value();
	void    set_value(int32_t value);
};

inline int32_t java_lang_Integer::get_value()
{
	return get<int32_t>(_handle, offset_value);
}

inline void java_lang_Integer::set_value(int32_t value)
{
	set(_handle, offset_value, value);
}


/**
 * java/lang/Long
 *
 * Object layout:
 *
 * 0. object header
 * 1. long value;
 */
class java_lang_Long : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_value = MEMORY_ALIGN(sizeof(java_object_t), sizeof(int64_t));

public:
	java_lang_Long(java_handle_t* h) : java_lang_Object(h) {}

	int64_t get_value();
	void    set_value(int64_t value);
};

inline int64_t java_lang_Long::get_value()
{
	return get<int64_t>(_handle, offset_value);
}

inline void java_lang_Long::set_value(int64_t value)
{
	set(_handle, offset_value, value);
}


/**
 * java/lang/Float
 *
 * Object layout:
 *
 * 0. object header
 * 1. float value;
 */
class java_lang_Float : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_value = MEMORY_ALIGN(sizeof(java_object_t), sizeof(float));

public:
	java_lang_Float(java_handle_t* h) : java_lang_Object(h) {}

	float get_value();
	void  set_value(float value);
};

inline float java_lang_Float::get_value()
{
	return get<float>(_handle, offset_value);
}

inline void java_lang_Float::set_value(float value)
{
	set(_handle, offset_value, value);
}


/**
 * java/lang/Double
 *
 * Object layout:
 *
 * 0. object header
 * 1. double value;
 */
class java_lang_Double : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_value = MEMORY_ALIGN(sizeof(java_object_t), sizeof(double));

public:
	java_lang_Double(java_handle_t* h) : java_lang_Object(h) {}

	double get_value();
	void   set_value(double value);
};

inline double java_lang_Double::get_value()
{
	return get<double>(_handle, offset_value);
}

inline void java_lang_Double::set_value(double value)
{
	set(_handle, offset_value, value);
}


#if defined(ENABLE_JAVASE)

/**
 * java/lang/management/MemoryUsage
 *
 * Object layout:
 *
 * 0. object header
 * [other fields are not used]
 */
class java_lang_management_MemoryUsage : public java_lang_Object, private FieldAccess {
public:
	java_lang_management_MemoryUsage(java_handle_t* h) : java_lang_Object(h) {}
	java_lang_management_MemoryUsage(int64_t init, int64_t used, int64_t commited, int64_t maximum);
};


# if defined(ENABLE_ANNOTATIONS)
/**
 * OpenJDK sun/reflect/ConstantPool
 *
 * Object layout:
 *
 * 0. object header
 * 1. java.lang.Object constantPoolOop;
 */
class sun_reflect_ConstantPool : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_constantPoolOop = MEMORY_ALIGN(sizeof(java_object_t), SIZEOF_VOID_P);

public:
	sun_reflect_ConstantPool(java_handle_t* h) : java_lang_Object(h) {}
	sun_reflect_ConstantPool(java_handle_t* h, jclass constantPoolOop);

	// Setters.
	void set_constantPoolOop(classinfo* value);
	void set_constantPoolOop(jclass value);
};


inline sun_reflect_ConstantPool::sun_reflect_ConstantPool(java_handle_t* h, jclass constantPoolOop) : java_lang_Object(h)
{
	set_constantPoolOop(constantPoolOop);
}


inline void sun_reflect_ConstantPool::set_constantPoolOop(classinfo* value)
{
	set(_handle, offset_constantPoolOop, value);
}

inline void sun_reflect_ConstantPool::set_constantPoolOop(jclass value)
{
	// XXX jclass is a boxed object.
	set_constantPoolOop(LLNI_classinfo_unwrap(value));
}
# endif // ENABLE_ANNOTATIONS

#endif // ENABLE_JAVASE

void jobjects_register_dyn_offsets();
bool jobjects_run_dynoffsets_hook(classinfo *c);

#if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)

/**
 * GNU Classpath java/lang/Class
 *
 * Object layout:
 *
 * 0. object header
 * 1. java.lang.Object[]             signers;
 * 2. java.security.ProtectionDomain pd;
 * 3. java.lang.Object               vmdata;
 * 4. java.lang.reflect.Constructor  constructor;
 */
class java_lang_Class : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_signers     = MEMORY_ALIGN(sizeof(java_object_t),          SIZEOF_VOID_P);
	static const off_t offset_pd          = MEMORY_ALIGN(offset_signers + SIZEOF_VOID_P, SIZEOF_VOID_P);
	static const off_t offset_vmdata      = MEMORY_ALIGN(offset_pd      + SIZEOF_VOID_P, SIZEOF_VOID_P);
	static const off_t offset_constructor = MEMORY_ALIGN(offset_vmdata  + SIZEOF_VOID_P, SIZEOF_VOID_P);

public:
	java_lang_Class(java_handle_t* h) : java_lang_Object(h) {}

	// Setters.
	void set_pd(java_handle_t* value);
};

inline void java_lang_Class::set_pd(java_handle_t* value)
{
	set(_handle, offset_pd, value);
}


/**
 * GNU Classpath java/lang/ClassLoader
 *
 * Object layout:
 *
 * 0. object header
 * 1. java.util.HashMap     definedPackages
 * 2. java.lang.ClassLoader parent
 * [other fields are not used]
 */
class java_lang_ClassLoader : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_definedPackages = MEMORY_ALIGN(sizeof(java_object_t),                  SIZEOF_VOID_P);
	static const off_t offset_parent          = MEMORY_ALIGN(offset_definedPackages + SIZEOF_VOID_P, SIZEOF_VOID_P);

public:
	java_lang_ClassLoader(java_handle_t* h) : java_lang_Object(h) {}

	// Getters.
	java_handle_t* get_parent() const;

	// Invocation wrappers for static methods.
	static java_handle_t* invoke_getSystemClassLoader();
};

inline java_handle_t* java_lang_ClassLoader::get_parent() const
{
	return get<java_handle_t*>(_handle, offset_parent);
}


/**
 * GNU Classpath java/lang/StackTraceElement
 *
 * Object layout:
 *
 * 0. object header
 * 1. java.lang.String fileName;
 * 2. int              lineNumber;
 * 3. java.lang.String declaringClass;
 * 4. java.lang.String methodName;
 * 5. boolean          isNative;
 */
class java_lang_StackTraceElement : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_fileName       = MEMORY_ALIGN(sizeof(java_object_t),                   SIZEOF_VOID_P);
	static const off_t offset_lineNumber     = MEMORY_ALIGN(offset_fileName       + SIZEOF_VOID_P,   sizeof(int32_t));
	static const off_t offset_declaringClass = MEMORY_ALIGN(offset_lineNumber     + sizeof(int32_t), SIZEOF_VOID_P);
	static const off_t offset_methodName     = MEMORY_ALIGN(offset_declaringClass + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_isNative       = MEMORY_ALIGN(offset_methodName     + SIZEOF_VOID_P,   SIZEOF_VOID_P);

public:
	java_lang_StackTraceElement(java_handle_t* h) : java_lang_Object(h) {}
	java_lang_StackTraceElement(java_handle_t* h, java_handle_t* fileName, int32_t lineNumber, java_handle_t* declaringClass, java_handle_t* methodName, uint8_t isNative);
};

inline java_lang_StackTraceElement::java_lang_StackTraceElement(java_handle_t* h, java_handle_t* fileName, int32_t lineNumber, java_handle_t* declaringClass, java_handle_t* methodName, uint8_t isNative) : java_lang_Object(h)
{
	java_lang_StackTraceElement((java_handle_t*) h);

	set(_handle, offset_fileName,       fileName);
	set(_handle, offset_lineNumber,     lineNumber);
	set(_handle, offset_declaringClass, declaringClass);
	set(_handle, offset_methodName,     methodName);
	set(_handle, offset_isNative,       isNative);
}


/**
 * GNU Classpath java/lang/String
 *
 * Object layout:
 *
 * 0. object header
 * 1. char[] value;
 * 2. int    count;
 * 3. int    cachedHashCode;
 * 4. int    offset;
 */
class java_lang_String : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_value          = MEMORY_ALIGN(sizeof(java_object_t),                   SIZEOF_VOID_P);
	static const off_t offset_count          = MEMORY_ALIGN(offset_value          + SIZEOF_VOID_P,   sizeof(int32_t));
	static const off_t offset_cachedHashCode = MEMORY_ALIGN(offset_count          + sizeof(int32_t), sizeof(int32_t));
	static const off_t offset_offset         = MEMORY_ALIGN(offset_cachedHashCode + sizeof(int32_t), sizeof(int32_t));

public:
	java_lang_String(java_handle_t* h) : java_lang_Object(h) {}
	java_lang_String(java_handle_t* h, java_handle_chararray_t* value, int32_t count, int32_t offset = 0);

	// Getters.
	java_handle_chararray_t* get_value () const;
	int32_t                  get_count () const;
	int32_t                  get_offset() const;

	// Setters.
	void set_value (java_handle_chararray_t* value);
	void set_count (int32_t value);
	void set_offset(int32_t value);

	// Raw access
	static inline void set_fields(java_handle_t *str, java_handle_chararray_t* value) {
		set(str, offset_value,  value);
		set(str, offset_count,  CharArray(value).get_length());
		set(str, offset_offset, (int32_t) 0);
		// cachedHashCode is assumed to be zero initialized
	}

	static inline java_handle_chararray_t *get_value(java_handle_t *str) {
		return get<java_handle_chararray_t*>(str, offset_value);
	}
	static inline int32_t get_count(java_handle_t *str) {
		return get<int32_t>(str, offset_count);
	}
	static inline int32_t get_offset(java_handle_t *str) {
		return get<int32_t>(str, offset_offset);
	}
};

inline java_lang_String::java_lang_String(java_handle_t* h, java_handle_chararray_t* value, int32_t count, int32_t offset) : java_lang_Object(h)
{
	set_value(value);
	set_count(count);
	set_offset(offset);
}

inline java_handle_chararray_t* java_lang_String::get_value() const
{
	return get<java_handle_chararray_t*>(_handle, offset_value);
}

inline int32_t java_lang_String::get_count() const
{
	return get<int32_t>(_handle, offset_count);
}

inline int32_t java_lang_String::get_offset() const
{
	return get<int32_t>(_handle, offset_offset);
}

inline void java_lang_String::set_value(java_handle_chararray_t* value)
{
	set(_handle, offset_value, value);
}

inline void java_lang_String::set_count(int32_t value)
{
	set(_handle, offset_count, value);
}

inline void java_lang_String::set_offset(int32_t value)
{
	set(_handle, offset_offset, value);
}

namespace runtime_str_ops {

inline jsize get_string_count(const java_lang_String &s)
{
	return s.get_count();
}

inline jsize get_string_offset(const java_lang_String &s)
{
	return s.get_offset();
}

}

/**
 * GNU Classpath java/lang/Thread
 */
class java_lang_Thread : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	static off_t offset_vmThread;
	static off_t offset_group;
	static off_t offset_name;
	static off_t offset_daemon;
	static off_t offset_priority;
	static off_t offset_exceptionHandler;

public:
	java_lang_Thread(java_handle_t* h) : java_lang_Object(h) {}

	// Getters.
	java_handle_t* get_vmThread        () const;
	java_handle_t* get_group           () const;
	java_handle_t* get_name            () const;
	int32_t        get_daemon          () const;
	int32_t        get_priority        () const;
	java_handle_t* get_exceptionHandler() const;

	// Setters.
	void set_group(java_handle_t* value);

	// Offset initializers
	static void set_vmThread_offset(int32_t off)         { offset_vmThread = off; }
	static void set_group_offset(int32_t off)            { offset_group = off; }
	static void set_name_offset(int32_t off)             { offset_name = off; }
	static void set_daemon_offset(int32_t off)           { offset_daemon = off; }
	static void set_priority_offset(int32_t off)         { offset_priority = off; }
	static void set_exceptionHandler_offset(int32_t off) { offset_exceptionHandler = off; }
};


inline java_handle_t* java_lang_Thread::get_vmThread() const
{
	return get<java_handle_t*>(_handle, offset_vmThread);
}

inline java_handle_t* java_lang_Thread::get_group() const
{
	return get<java_handle_t*>(_handle, offset_group);
}

inline java_handle_t* java_lang_Thread::get_name() const
{
	return get<java_handle_t*>(_handle, offset_name);
}

inline int32_t java_lang_Thread::get_daemon() const
{
	return get<int32_t>(_handle, offset_daemon);
}

inline int32_t java_lang_Thread::get_priority() const
{
	return get<int32_t>(_handle, offset_priority);
}

inline java_handle_t* java_lang_Thread::get_exceptionHandler() const
{
	return get<java_handle_t*>(_handle, offset_exceptionHandler);
}


inline void java_lang_Thread::set_group(java_handle_t* value)
{
	set(_handle, offset_group, value);
}


/**
 * GNU Classpath java/lang/VMThread
 *
 * Object layout:
 *
 * 0. object header
 * 1. java.lang.Thread   thread;
 * 2. boolean            running;
 * 3. java.lang.VMThread vmdata;
 */
class java_lang_VMThread : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_thread  = MEMORY_ALIGN(sizeof(java_object_t),            SIZEOF_VOID_P);
	static const off_t offset_running = MEMORY_ALIGN(offset_thread  + SIZEOF_VOID_P,   sizeof(int32_t));
	static const off_t offset_vmdata  = MEMORY_ALIGN(offset_running + sizeof(int32_t), SIZEOF_VOID_P);

public:
	java_lang_VMThread(java_handle_t* h) : java_lang_Object(h) {}
	java_lang_VMThread(java_handle_t* h, java_handle_t* thread, threadobject* vmdata);

	// Getters.
	java_handle_t* get_thread() const;
	threadobject*  get_vmdata() const;

	// Setters.
	void set_thread(java_handle_t* value);
	void set_vmdata(threadobject* value);
};


inline java_lang_VMThread::java_lang_VMThread(java_handle_t* h, java_handle_t* thread, threadobject* vmdata) : java_lang_Object(h)
{
	set_thread(thread);
	set_vmdata(vmdata);
}


inline java_handle_t* java_lang_VMThread::get_thread() const
{
	return get<java_handle_t*>(_handle, offset_thread);
}

inline threadobject* java_lang_VMThread::get_vmdata() const
{
	return get<threadobject*>(_handle, offset_vmdata);
}


inline void java_lang_VMThread::set_thread(java_handle_t* value)
{
	set(_handle, offset_thread, value);
}

inline void java_lang_VMThread::set_vmdata(threadobject* value)
{
	set(_handle, offset_vmdata, value);
}


/**
 * GNU Classpath java/lang/Throwable
 *
 * Object layout:
 *
 * 0. object header
 * 1. java.lang.String              detailMessage;
 * 2. java.lang.Throwable           cause;
 * 3. java.lang.StackTraceElement[] stackTrace;
 * 4. java.lang.VMThrowable         vmState;
 */
class java_lang_Throwable : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_detailMessage = MEMORY_ALIGN(sizeof(java_object_t),                SIZEOF_VOID_P);
	static const off_t offset_cause         = MEMORY_ALIGN(offset_detailMessage + SIZEOF_VOID_P, SIZEOF_VOID_P);
	static const off_t offset_stackTrace    = MEMORY_ALIGN(offset_cause         + SIZEOF_VOID_P, SIZEOF_VOID_P);
	static const off_t offset_vmState       = MEMORY_ALIGN(offset_stackTrace    + SIZEOF_VOID_P, SIZEOF_VOID_P);

public:
	java_lang_Throwable(java_handle_t* h) : java_lang_Object(h) {}

	// Getters.
	java_handle_t* get_detailMessage() const;
	java_handle_t* get_cause        () const;
	java_handle_t* get_vmState      () const;
};


inline java_handle_t* java_lang_Throwable::get_detailMessage() const
{
	return get<java_handle_t*>(_handle, offset_detailMessage);
}

inline java_handle_t* java_lang_Throwable::get_cause() const
{
	return get<java_handle_t*>(_handle, offset_cause);
}

inline java_handle_t* java_lang_Throwable::get_vmState() const
{
	return get<java_handle_t*>(_handle, offset_vmState);
}


/**
 * GNU Classpath java/lang/VMThrowable
 *
 * Object layout:
 *
 * 0. object header
 * 1. java.lang.Object vmdata;
 */
class java_lang_VMThrowable : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_vmdata = MEMORY_ALIGN(sizeof(java_object_t), SIZEOF_VOID_P);

public:
	java_lang_VMThrowable(java_handle_t* h) : java_lang_Object(h) {}

	java_handle_bytearray_t* get_vmdata() const;
	void                     set_vmdata(java_handle_bytearray_t* value);
};


inline java_handle_bytearray_t* java_lang_VMThrowable::get_vmdata() const
{
	return get<java_handle_bytearray_t*>(_handle, offset_vmdata);
}

inline void java_lang_VMThrowable::set_vmdata(java_handle_bytearray_t* value)
{
	set(_handle, offset_vmdata, value);
}


/**
 * GNU Classpath java/lang/reflect/VMConstructor
 *
 * Object layout:
 *
 * 0. object header
 * 1. java.lang.Class               clazz;
 * 2. int                           slot;
 * 3. byte[]                        annotations;
 * 4. byte[]                        parameterAnnotations;
 * 5. java.util.Map                 declaredAnnotations;
 * 6. java.lang.reflect.Constructor cons;
 */
class java_lang_reflect_VMConstructor : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_clazz                = MEMORY_ALIGN(sizeof(java_object_t),                         SIZEOF_VOID_P);
	static const off_t offset_slot                 = MEMORY_ALIGN(offset_clazz                + SIZEOF_VOID_P,   sizeof(int32_t));
	static const off_t offset_annotations          = MEMORY_ALIGN(offset_slot                 + sizeof(int32_t), SIZEOF_VOID_P);
	static const off_t offset_parameterAnnotations = MEMORY_ALIGN(offset_annotations          + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_declaredAnnotations  = MEMORY_ALIGN(offset_parameterAnnotations + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_cons                 = MEMORY_ALIGN(offset_declaredAnnotations  + SIZEOF_VOID_P,   SIZEOF_VOID_P);

public:
	java_lang_reflect_VMConstructor(java_handle_t* h) : java_lang_Object(h) {}
	java_lang_reflect_VMConstructor(methodinfo* m);

	// Getters.
	classinfo*               get_clazz               () const;
	int32_t                  get_slot                () const;
	java_handle_bytearray_t* get_annotations         () const;
	java_handle_bytearray_t* get_parameterAnnotations() const;
	java_handle_t*           get_declaredAnnotations () const;
	java_handle_t*           get_cons                () const;

	// Setters.
	void set_clazz               (classinfo* value);
	void set_slot                (int32_t value);
	void set_annotations         (java_handle_bytearray_t* value);
	void set_parameterAnnotations(java_handle_bytearray_t* value);
	void set_declaredAnnotations (java_handle_t* value);
	void set_cons                (java_handle_t* value);

	// Convenience functions.
	methodinfo* get_method();
};


inline java_lang_reflect_VMConstructor::java_lang_reflect_VMConstructor(methodinfo* m)
{
	_handle = builtin_new(class_java_lang_reflect_VMConstructor);

	if (is_null())
		return;

	int                      slot                 = m - m->clazz->methods;
	java_handle_bytearray_t* annotations          = method_get_annotations(m);
	java_handle_bytearray_t* parameterAnnotations = method_get_parameterannotations(m);

	set_clazz(m->clazz);
	set_slot(slot);
	set_annotations(annotations);
	set_parameterAnnotations(parameterAnnotations);
}


inline classinfo* java_lang_reflect_VMConstructor::get_clazz() const
{
	return get<classinfo*>(_handle, offset_clazz);
}

inline int32_t java_lang_reflect_VMConstructor::get_slot() const
{
	return get<int32_t>(_handle, offset_slot);
}

inline java_handle_bytearray_t* java_lang_reflect_VMConstructor::get_annotations() const
{
	return get<java_handle_bytearray_t*>(_handle, offset_annotations);
}

inline java_handle_bytearray_t* java_lang_reflect_VMConstructor::get_parameterAnnotations() const
{
	return get<java_handle_bytearray_t*>(_handle, offset_parameterAnnotations);
}

inline java_handle_t* java_lang_reflect_VMConstructor::get_declaredAnnotations() const
{
	return get<java_handle_t*>(_handle, offset_declaredAnnotations);
}

inline java_handle_t* java_lang_reflect_VMConstructor::get_cons() const
{
	return get<java_handle_t*>(_handle, offset_cons);
}

inline void java_lang_reflect_VMConstructor::set_clazz(classinfo* value)
{
	set(_handle, offset_clazz, value);
}

inline void java_lang_reflect_VMConstructor::set_slot(int32_t value)
{
	set(_handle, offset_slot, value);
}

inline void java_lang_reflect_VMConstructor::set_annotations(java_handle_bytearray_t* value)
{
	set(_handle, offset_annotations, value);
}

inline void java_lang_reflect_VMConstructor::set_parameterAnnotations(java_handle_bytearray_t* value)
{
	set(_handle, offset_parameterAnnotations, value);
}

inline void java_lang_reflect_VMConstructor::set_declaredAnnotations(java_handle_t* value)
{
	set(_handle, offset_declaredAnnotations, value);
}

inline void java_lang_reflect_VMConstructor::set_cons(java_handle_t* value)
{
	set(_handle, offset_cons, value);
}

inline methodinfo* java_lang_reflect_VMConstructor::get_method()
{
	classinfo*  c    = get_clazz();
	int32_t     slot = get_slot();
	methodinfo* m    = &(c->methods[slot]);
	return m;
}


/**
 * GNU Classpath java/lang/reflect/Constructor
 *
 * Object layout:
 *
 * 0. object header
 * 1. boolean                                     flag;
 * 2. gnu.java.lang.reflect.MethodSignatureParser p;
 * 3. java.lang.reflect.VMConstructor             cons;
 */
class java_lang_reflect_Constructor : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_flag = MEMORY_ALIGN(sizeof(java_object_t),         sizeof(int32_t));
	static const off_t offset_p    = MEMORY_ALIGN(offset_flag + sizeof(int32_t), SIZEOF_VOID_P);
	static const off_t offset_cons = MEMORY_ALIGN(offset_p    + SIZEOF_VOID_P,   SIZEOF_VOID_P);

public:
	java_lang_reflect_Constructor(java_handle_t* h) : java_lang_Object(h) {}
	java_lang_reflect_Constructor(methodinfo* m);

	java_handle_t* new_instance(java_handle_objectarray_t* args);

	// Getters.
	int32_t        get_flag() const;
	java_handle_t* get_cons() const;

	// Setters.
	void set_cons(java_handle_t* value);

	// Convenience functions.
	methodinfo* get_method  () const;
	int32_t     get_override() const;
};


inline java_lang_reflect_Constructor::java_lang_reflect_Constructor(methodinfo* m)
{
	java_lang_reflect_VMConstructor jlrvmc(m);

	if (jlrvmc.is_null())
		return;

	_handle = builtin_new(class_java_lang_reflect_Constructor);

	if (is_null())
		return;

	// Link the two Java objects.
	set_cons(jlrvmc.get_handle());
	jlrvmc.set_cons(get_handle());
}


inline int32_t java_lang_reflect_Constructor::get_flag() const
{
	return get<int32_t>(_handle, offset_flag);
}

inline java_handle_t* java_lang_reflect_Constructor::get_cons() const
{
	return get<java_handle_t*>(_handle, offset_cons);
}


inline void java_lang_reflect_Constructor::set_cons(java_handle_t* value)
{
	set(_handle, offset_cons, value);
}


inline methodinfo* java_lang_reflect_Constructor::get_method() const
{
	java_lang_reflect_VMConstructor jlrvmc(get_cons());
	return jlrvmc.get_method();
}

inline int32_t java_lang_reflect_Constructor::get_override() const
{
	return get_flag();
}


/**
 * GNU Classpath java/lang/reflect/VMField
 *
 * Object layout:
 *
 * 0. object header
 * 1. java.lang.Class         clazz;
 * 2. java.lang.String        name;
 * 3. int                     slot;
 * 4. byte[]                  annotations;
 * 5. java.lang.Map           declaredAnnotations;
 * 6. java.lang.reflect.Field f;
 */
class java_lang_reflect_VMField : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_clazz               = MEMORY_ALIGN(sizeof(java_object_t),                        SIZEOF_VOID_P);
	static const off_t offset_name                = MEMORY_ALIGN(offset_clazz               + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_slot                = MEMORY_ALIGN(offset_name                + SIZEOF_VOID_P,   sizeof(int32_t));
	static const off_t offset_annotations         = MEMORY_ALIGN(offset_slot                + sizeof(int32_t), SIZEOF_VOID_P);
	static const off_t offset_declaredAnnotations = MEMORY_ALIGN(offset_annotations         + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_f                   = MEMORY_ALIGN(offset_declaredAnnotations + SIZEOF_VOID_P,   SIZEOF_VOID_P);

public:
	java_lang_reflect_VMField(java_handle_t* h) : java_lang_Object(h) {}
	java_lang_reflect_VMField(fieldinfo* f);

	// Getters.
	classinfo*               get_clazz              () const;
	int32_t                  get_slot               () const;
	java_handle_bytearray_t* get_annotations        () const;
	java_handle_t*           get_declaredAnnotations() const;
	java_handle_t*           get_f                  () const;

	// Setters.
	void set_clazz              (classinfo* value);
	void set_name               (java_handle_t* value);
	void set_slot               (int32_t value);
	void set_annotations        (java_handle_bytearray_t* value);
	void set_declaredAnnotations(java_handle_t* value);
	void set_f                  (java_handle_t* value);

	// Convenience functions.
	fieldinfo* get_field() const;
};


inline java_lang_reflect_VMField::java_lang_reflect_VMField(fieldinfo* f)
{
	_handle = builtin_new(class_java_lang_reflect_VMField);

	if (is_null())
		return;

	java_handle_t*           name        = JavaString::literal(f->name);
	int                      slot        = f - f->clazz->fields;
	java_handle_bytearray_t* annotations = field_get_annotations(f);

	set_clazz(f->clazz);
	set_name(name);
	set_slot(slot);
	set_annotations(annotations);
}


inline classinfo* java_lang_reflect_VMField::get_clazz() const
{
	return get<classinfo*>(_handle, offset_clazz);
}

inline int32_t java_lang_reflect_VMField::get_slot() const
{
	return get<int32_t>(_handle, offset_slot);
}

inline java_handle_bytearray_t* java_lang_reflect_VMField::get_annotations() const
{
	return get<java_handle_bytearray_t*>(_handle, offset_annotations);
}

inline java_handle_t* java_lang_reflect_VMField::get_declaredAnnotations() const
{
	return get<java_handle_t*>(_handle, offset_declaredAnnotations);
}

inline java_handle_t* java_lang_reflect_VMField::get_f() const
{
	return get<java_handle_t*>(_handle, offset_f);
}


inline void java_lang_reflect_VMField::set_clazz(classinfo* value)
{
	set(_handle, offset_clazz, value);
}

inline void java_lang_reflect_VMField::set_name(java_handle_t* value)
{
	set(_handle, offset_name, value);
}

inline void java_lang_reflect_VMField::set_slot(int32_t value)
{
	set(_handle, offset_slot, value);
}

inline void java_lang_reflect_VMField::set_annotations(java_handle_bytearray_t* value)
{
	set(_handle, offset_annotations, value);
}

inline void java_lang_reflect_VMField::set_declaredAnnotations(java_handle_t* value)
{
	set(_handle, offset_declaredAnnotations, value);
}

inline void java_lang_reflect_VMField::set_f(java_handle_t* value)
{
	set(_handle, offset_f, value);
}

inline fieldinfo* java_lang_reflect_VMField::get_field() const
{
	classinfo* c    = get_clazz();
	int32_t    slot = get_slot();
	fieldinfo* f    = &(c->fields[slot]);
	return f;
}


/**
 * GNU Classpath java/lang/reflect/Field
 *
 * Object layout:
 *
 * 0. object header
 * 1. boolean                                    flag;
 * 2. gnu.java.lang.reflect.FieldSignatureParser p;
 * 3. java.lang.reflect.VMField                  f;
 */
class java_lang_reflect_Field : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_flag = MEMORY_ALIGN(sizeof(java_object_t),         sizeof(int32_t));
	static const off_t offset_p    = MEMORY_ALIGN(offset_flag + sizeof(int32_t), SIZEOF_VOID_P);
	static const off_t offset_f    = MEMORY_ALIGN(offset_p    + SIZEOF_VOID_P,   SIZEOF_VOID_P);

public:
	java_lang_reflect_Field(java_handle_t* h) : java_lang_Object(h) {}
	java_lang_reflect_Field(fieldinfo* f);

	// Getters.
	int32_t        get_flag() const;
	java_handle_t* get_f() const;

	// Setters.
	void set_f(java_handle_t* value);

	// Convenience functions.
	fieldinfo* get_field() const;
};


inline java_lang_reflect_Field::java_lang_reflect_Field(fieldinfo* f)
{
	java_lang_reflect_VMField jlrvmf(f);

	if (jlrvmf.is_null())
		return;

	_handle = builtin_new(class_java_lang_reflect_Field);

	if (is_null())
		return;

	// Link the two Java objects.
	set_f(jlrvmf.get_handle());
	jlrvmf.set_f(get_handle());
}


inline int32_t java_lang_reflect_Field::get_flag() const
{
	return get<int32_t>(_handle, offset_flag);
}

inline java_handle_t* java_lang_reflect_Field::get_f() const
{
	return get<java_handle_t*>(_handle, offset_f);
}


inline void java_lang_reflect_Field::set_f(java_handle_t* value)
{
	set(_handle, offset_f, value);
}


inline fieldinfo* java_lang_reflect_Field::get_field() const
{
	java_lang_reflect_VMField jlrvmf(get_f());
	return jlrvmf.get_field();
}


/**
 * GNU Classpath java/lang/reflect/VMMethod
 *
 * Object layout:
 *
 * 0. object header
 * 1. java.lang.Class          clazz;
 * 2. java.lang.String         name;
 * 3. int                      slot;
 * 4. byte[]                   annotations;
 * 5. byte[]                   parameterAnnotations;
 * 6. byte[]                   annotationDefault;
 * 7. java.lang.Map            declaredAnnotations;
 * 8. java.lang.reflect.Method m;
 */
class java_lang_reflect_VMMethod : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_clazz                = MEMORY_ALIGN(sizeof(java_object_t),                         SIZEOF_VOID_P);
	static const off_t offset_name                 = MEMORY_ALIGN(offset_clazz                + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_slot                 = MEMORY_ALIGN(offset_name                 + SIZEOF_VOID_P,   sizeof(int32_t));
	static const off_t offset_annotations          = MEMORY_ALIGN(offset_slot                 + sizeof(int32_t), SIZEOF_VOID_P);
	static const off_t offset_parameterAnnotations = MEMORY_ALIGN(offset_annotations          + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_annotationDefault    = MEMORY_ALIGN(offset_parameterAnnotations + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_declaredAnnotations  = MEMORY_ALIGN(offset_annotationDefault    + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_m                    = MEMORY_ALIGN(offset_declaredAnnotations  + SIZEOF_VOID_P,   SIZEOF_VOID_P);

public:
	java_lang_reflect_VMMethod(java_handle_t* h) : java_lang_Object(h) {}
	java_lang_reflect_VMMethod(methodinfo* m);

	// Getters.
	classinfo*               get_clazz               () const;
	int32_t                  get_slot                () const;
	java_handle_bytearray_t* get_annotations         () const;
	java_handle_bytearray_t* get_parameterAnnotations() const;
	java_handle_bytearray_t* get_annotationDefault   () const;
	java_handle_t*           get_declaredAnnotations () const;
	java_handle_t*           get_m                   () const;

	// Setters.
	void set_clazz               (classinfo* value);
	void set_name                (java_handle_t* value);
	void set_slot                (int32_t value);
	void set_annotations         (java_handle_bytearray_t* value);
	void set_parameterAnnotations(java_handle_bytearray_t* value);
	void set_annotationDefault   (java_handle_bytearray_t* value);
	void set_declaredAnnotations (java_handle_t* value);
	void set_m                   (java_handle_t* value);

	// Convenience functions.
	methodinfo* get_method() const;
};


inline java_lang_reflect_VMMethod::java_lang_reflect_VMMethod(methodinfo* m)
{
	_handle = builtin_new(class_java_lang_reflect_VMMethod);

	if (is_null())
		return;

	java_handle_t*           name                 = JavaString::literal(m->name);
	int                      slot                 = m - m->clazz->methods;
	java_handle_bytearray_t* annotations          = method_get_annotations(m);
	java_handle_bytearray_t* parameterAnnotations = method_get_parameterannotations(m);
	java_handle_bytearray_t* annotationDefault    = method_get_annotationdefault(m);

	set_clazz(m->clazz);
	set_name(name);
	set_slot(slot);
	set_annotations(annotations);
	set_parameterAnnotations(parameterAnnotations);
	set_annotationDefault(annotationDefault);
}

inline classinfo* java_lang_reflect_VMMethod::get_clazz() const
{
	return get<classinfo*>(_handle, offset_clazz);
}

inline int32_t java_lang_reflect_VMMethod::get_slot() const
{
	return get<int32_t>(_handle, offset_slot);
}

inline java_handle_bytearray_t* java_lang_reflect_VMMethod::get_annotations() const
{
	return get<java_handle_bytearray_t*>(_handle, offset_annotations);
}

inline java_handle_bytearray_t* java_lang_reflect_VMMethod::get_parameterAnnotations() const
{
	return get<java_handle_bytearray_t*>(_handle, offset_parameterAnnotations);
}

inline java_handle_bytearray_t* java_lang_reflect_VMMethod::get_annotationDefault() const
{
	return get<java_handle_bytearray_t*>(_handle, offset_annotationDefault);
}

inline java_handle_t* java_lang_reflect_VMMethod::get_declaredAnnotations() const
{
	return get<java_handle_t*>(_handle, offset_declaredAnnotations);
}

inline java_handle_t* java_lang_reflect_VMMethod::get_m() const
{
	return get<java_handle_t*>(_handle, offset_m);
}

inline void java_lang_reflect_VMMethod::set_clazz(classinfo* value)
{
	set(_handle, offset_clazz, value);
}

inline void java_lang_reflect_VMMethod::set_name(java_handle_t* value)
{
	set(_handle, offset_name, value);
}

inline void java_lang_reflect_VMMethod::set_slot(int32_t value)
{
	set(_handle, offset_slot, value);
}

inline void java_lang_reflect_VMMethod::set_annotations(java_handle_bytearray_t* value)
{
	set(_handle, offset_annotations, value);
}

inline void java_lang_reflect_VMMethod::set_parameterAnnotations(java_handle_bytearray_t* value)
{
	set(_handle, offset_parameterAnnotations, value);
}

inline void java_lang_reflect_VMMethod::set_annotationDefault(java_handle_bytearray_t* value)
{
	set(_handle, offset_annotationDefault, value);
}

inline void java_lang_reflect_VMMethod::set_declaredAnnotations(java_handle_t* value)
{
	set(_handle, offset_declaredAnnotations, value);
}

inline void java_lang_reflect_VMMethod::set_m(java_handle_t* value)
{
	set(_handle, offset_m, value);
}

inline methodinfo* java_lang_reflect_VMMethod::get_method() const
{
	classinfo*  c    = get_clazz();
	int32_t     slot = get_slot();
	methodinfo* m    = &(c->methods[slot]);
	return m;
}


/**
 * GNU Classpath java/lang/reflect/Method
 *
 * Object layout:
 *
 * 0. object header
 * 1. boolean                                     flag;
 * 2. gnu.java.lang.reflect.MethodSignatureParser p;
 * 3. java.lang.reflect.VMMethod                  m;
 */
class java_lang_reflect_Method : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_flag = MEMORY_ALIGN(sizeof(java_object_t),         sizeof(int32_t));
	static const off_t offset_p    = MEMORY_ALIGN(offset_flag + sizeof(int32_t), SIZEOF_VOID_P);
	static const off_t offset_m    = MEMORY_ALIGN(offset_p    + SIZEOF_VOID_P,   SIZEOF_VOID_P);

public:
	java_lang_reflect_Method(java_handle_t* h) : java_lang_Object(h) {}
	java_lang_reflect_Method(methodinfo* m);

	java_handle_t* invoke(java_handle_t* o, java_handle_objectarray_t* args);

	// Getters.
	int32_t        get_flag() const;
	java_handle_t* get_m() const;

	// Setters.
	void set_m(java_handle_t* value);

	// Convenience functions.
	methodinfo* get_method  () const;
	int32_t     get_override() const;
};


inline java_lang_reflect_Method::java_lang_reflect_Method(methodinfo* m)
{
	java_lang_reflect_VMMethod jlrvmm(m);

	if (jlrvmm.is_null())
		return;

	_handle = builtin_new(class_java_lang_reflect_Method);

	if (is_null())
		return;

	// Link the two Java objects.
	set_m(jlrvmm.get_handle());
	jlrvmm.set_m(get_handle());
}


inline int32_t java_lang_reflect_Method::get_flag() const
{
	return get<int32_t>(_handle, offset_flag);
}

inline java_handle_t* java_lang_reflect_Method::get_m() const
{
	return get<java_handle_t*>(_handle, offset_m);
}


inline void java_lang_reflect_Method::set_m(java_handle_t* value)
{
	set(_handle, offset_m, value);
}


inline methodinfo* java_lang_reflect_Method::get_method() const
{
	java_lang_reflect_VMMethod jlrvmm(get_m());
	return jlrvmm.get_method();
}

inline int32_t java_lang_reflect_Method::get_override() const
{
	return get_flag();
}


/**
 * GNU Classpath java/nio/Buffer
 *
 * Object layout:
 *
 * 0. object header
 * 1. int                   cap;
 * 2. int                   limit;
 * 3. int                   pos;
 * 4. int                   mark;
 * 5. gnu.classpath.Pointer address;
 */
class java_nio_Buffer : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_cap     = MEMORY_ALIGN(sizeof(java_object_t),          sizeof(int32_t));
	static const off_t offset_limit   = MEMORY_ALIGN(offset_cap   + sizeof(int32_t), sizeof(int32_t));
	static const off_t offset_pos     = MEMORY_ALIGN(offset_limit + sizeof(int32_t), sizeof(int32_t));
	static const off_t offset_mark    = MEMORY_ALIGN(offset_pos   + sizeof(int32_t), sizeof(int32_t));
	static const off_t offset_address = MEMORY_ALIGN(offset_mark  + sizeof(int32_t), SIZEOF_VOID_P);

public:
	java_nio_Buffer(java_handle_t* h) : java_lang_Object(h) {}

	// Getters.
	int32_t get_cap() const;
};

inline int32_t java_nio_Buffer::get_cap() const
{
	return get<int32_t>(_handle, offset_cap);
}


/**
 * GNU Classpath java/nio/DirectByteBufferImpl
 *
 * Object layout:
 *
 * 0. object header
 * 1. int                   cap;
 * 2. int                   limit;
 * 3. int                   pos;
 * 4. int                   mark;
 * 5. gnu.classpath.Pointer address;
 * 6. java.nio.ByteOrder    endian;
 * 7. byte[]                backing_buffer;
 * 8. int                   array_offset;
 * 9. java.lang.Object      owner;
 */
class java_nio_DirectByteBufferImpl : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_cap            = MEMORY_ALIGN(sizeof(java_object_t),                   sizeof(int32_t));
	static const off_t offset_limit          = MEMORY_ALIGN(offset_cap            + sizeof(int32_t), sizeof(int32_t));
	static const off_t offset_pos            = MEMORY_ALIGN(offset_limit          + sizeof(int32_t), sizeof(int32_t));
	static const off_t offset_mark           = MEMORY_ALIGN(offset_pos            + sizeof(int32_t), sizeof(int32_t));
	static const off_t offset_address        = MEMORY_ALIGN(offset_mark           + sizeof(int32_t), SIZEOF_VOID_P);
	static const off_t offset_endian         = MEMORY_ALIGN(offset_address        + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_backing_buffer = MEMORY_ALIGN(offset_endian         + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_array_offset   = MEMORY_ALIGN(offset_backing_buffer + SIZEOF_VOID_P,   sizeof(int32_t));
	static const off_t offset_owner          = MEMORY_ALIGN(offset_array_offset   + sizeof(int32_t), SIZEOF_VOID_P);

public:
	java_nio_DirectByteBufferImpl(java_handle_t* h) : java_lang_Object(h) {}

	// Getters.
	java_handle_t* get_address() const;
};


inline java_handle_t* java_nio_DirectByteBufferImpl::get_address() const
{
	return get<java_handle_t*>(_handle, offset_address);
}


/**
 * GNU Classpath gnu/classpath/Pointer
 *
 * Actually there are two classes, gnu.classpath.Pointer32 and
 * gnu.classpath.Pointer64, but we only define the abstract super
 * class and use the int/long field as void* type.
 *
 * Object layout:
 *
 * 0. object header
 * 1. int/long data;
 */
class gnu_classpath_Pointer : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_data = MEMORY_ALIGN(sizeof(java_object_t), SIZEOF_VOID_P);

public:
	gnu_classpath_Pointer(java_handle_t* h) : java_lang_Object(h) {}
	gnu_classpath_Pointer(java_handle_t* h, void* data);

	// Getters.
	void* get_data() const;

	// Setters.
	void set_data(void* value);
};

inline gnu_classpath_Pointer::gnu_classpath_Pointer(java_handle_t* h, void* data) : java_lang_Object(h)
{
	set_data(data);
}

inline void* gnu_classpath_Pointer::get_data() const
{
	return get<void*>(_handle, offset_data);
}

inline void gnu_classpath_Pointer::set_data(void* value)
{
	set(_handle, offset_data, value);
}

#endif // WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH


#if defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)

/**
 * OpenJDK java/lang/AssertionStatusDirectives
 *
 * Object layout:
 *
 * 0. object header
 * 1. java.lang.String[] classes;
 * 2. boolean[]          classEnabled;
 * 3. java.lang.String[] packages;
 * 4. boolean[]          packageEnabled;
 * 5. boolean            deflt;
 */
class java_lang_AssertionStatusDirectives : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_classes        = MEMORY_ALIGN(sizeof(java_object_t),                 SIZEOF_VOID_P);
	static const off_t offset_classEnabled   = MEMORY_ALIGN(offset_classes        + SIZEOF_VOID_P, SIZEOF_VOID_P);
	static const off_t offset_packages       = MEMORY_ALIGN(offset_classEnabled   + SIZEOF_VOID_P, SIZEOF_VOID_P);
	static const off_t offset_packageEnabled = MEMORY_ALIGN(offset_packages       + SIZEOF_VOID_P, SIZEOF_VOID_P);
	static const off_t offset_deflt          = MEMORY_ALIGN(offset_packageEnabled + SIZEOF_VOID_P, sizeof(int32_t));

public:
	java_lang_AssertionStatusDirectives(java_handle_objectarray_t* classes, java_handle_booleanarray_t* classEnabled, java_handle_objectarray_t* packages, java_handle_booleanarray_t* packageEnabled);
};

inline java_lang_AssertionStatusDirectives::java_lang_AssertionStatusDirectives(java_handle_objectarray_t* classes, java_handle_booleanarray_t* classEnabled, java_handle_objectarray_t* packages, java_handle_booleanarray_t* packageEnabled)
{
	classinfo* c = load_class_bootstrap(Utf8String::from_utf8("java/lang/AssertionStatusDirectives"));

	// FIXME Load the class at VM startup.
	if (c == NULL)
		return;

	_handle = builtin_new(c);

	if (is_null())
		return;

	set(_handle, offset_classes,        classes);
	set(_handle, offset_classEnabled,   classEnabled);
	set(_handle, offset_packages,       packages);
	set(_handle, offset_packageEnabled, packageEnabled);
}


/**
 * OpenJDK java/lang/ClassLoader
 *
 * Object layout:
 *
 * 0. object header
 * 1. boolean               initialized
 * 2. java.lang.ClassLoader parent
 * [other fields are not used]
 */
class java_lang_ClassLoader : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_initialized = MEMORY_ALIGN(sizeof(java_object_t),                sizeof(int32_t));
	static const off_t offset_parent      = MEMORY_ALIGN(offset_initialized + sizeof(int32_t), SIZEOF_VOID_P);

public:
	java_lang_ClassLoader(java_handle_t* h) : java_lang_Object(h) {}

	// Getters.
	java_handle_t* get_parent() const;

	// Invocation wrappers for static methods.
	static java_handle_t* invoke_getSystemClassLoader();
};

inline java_handle_t* java_lang_ClassLoader::get_parent() const
{
	return get<java_handle_t*>(_handle, offset_parent);
}


/**
 * OpenJDK java/lang/StackTraceElement
 *
 * Object layout:
 *
 * 0. object header
 * 1. java.lang.String declaringClass;
 * 2. java.lang.String methodName;
 * 3. java.lang.String fileName;
 * 4. int              lineNumber;
 */
class java_lang_StackTraceElement : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_declaringClass = MEMORY_ALIGN(sizeof(java_object_t),                 SIZEOF_VOID_P);
	static const off_t offset_methodName     = MEMORY_ALIGN(offset_declaringClass + SIZEOF_VOID_P, SIZEOF_VOID_P);
	static const off_t offset_fileName       = MEMORY_ALIGN(offset_methodName     + SIZEOF_VOID_P, SIZEOF_VOID_P);
	static const off_t offset_lineNumber     = MEMORY_ALIGN(offset_fileName       + SIZEOF_VOID_P, sizeof(int32_t));

public:
	java_lang_StackTraceElement(java_handle_t* h) : java_lang_Object(h) {}
	java_lang_StackTraceElement(java_handle_t* declaringClass, java_handle_t* methodName, java_handle_t* fileName, int32_t lineNumber);
};

inline java_lang_StackTraceElement::java_lang_StackTraceElement(java_handle_t* declaringClass, java_handle_t* methodName, java_handle_t* fileName, int32_t lineNumber)
{
	_handle = builtin_new(class_java_lang_StackTraceElement);

	if (is_null())
		return;

	set(_handle, offset_declaringClass, declaringClass);
	set(_handle, offset_methodName,     methodName);
	set(_handle, offset_fileName,       fileName);
	set(_handle, offset_lineNumber,     lineNumber);
}


/**
 * OpenJDK java/lang/String
 *
 * Object layout (JDK6):
 *
 * 0. object header
 * 1. char[] value;
 * 2. int    offset;
 * 3. int    count;
 * 4. int    hash;
 *
 * Object layout (JDK7):
 *
 * 0. object header
 * 1. char[] value;
 * 2. int    hash;
 */
class java_lang_String : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_value  = MEMORY_ALIGN(sizeof(java_object_t),           SIZEOF_VOID_P);
#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	static const off_t offset_offset = MEMORY_ALIGN(offset_value  + SIZEOF_VOID_P,   sizeof(int32_t));
	static const off_t offset_count  = MEMORY_ALIGN(offset_offset + sizeof(int32_t), sizeof(int32_t));
#else
	static const off_t offset_hash   = MEMORY_ALIGN(offset_value  + SIZEOF_VOID_P,   sizeof(int32_t));
#endif

public:
	java_lang_String(java_handle_t* h) : java_lang_Object(h) {}
	java_lang_String(java_handle_t* h, java_handle_chararray_t* value, int32_t count, int32_t offset = 0);

	// Getters.
	java_handle_chararray_t* get_value () const;
#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	int32_t                  get_offset() const;
	int32_t                  get_count () const;
#endif

	// Setters.
	void set_value (java_handle_chararray_t* value);
#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	void set_offset(int32_t value);
	void set_count (int32_t value);
#endif

	// Raw access
	static inline void set_fields(java_handle_t *str, java_handle_chararray_t *value) {
		set(str, offset_value,  value);
#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
		set(str, offset_offset, (int32_t) 0);
		set(str, offset_count,  CharArray(value).get_length());
#endif
		// hash is assumed to be zero initialized
	}

	static inline java_handle_chararray_t *get_value(java_handle_t *str) {
		return get<java_handle_chararray_t*>(str, offset_value);
	}
	static inline int32_t get_count(java_handle_t *str) {
#ifdef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
		return CharArray(get_value(str)).get_length();
#else
		return get<int32_t>(str, offset_count);
#endif
	}
	static inline int32_t get_offset(java_handle_t *str) {
#ifdef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
		return 0;
#else
		return get<int32_t>(str, offset_offset);
#endif
	}
};

inline java_lang_String::java_lang_String(java_handle_t* h, java_handle_chararray_t* value, int32_t count, int32_t offset) : java_lang_Object(h)
{
	set_value(value);
#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	set_offset(offset);
	set_count(count);
#endif
}

inline java_handle_chararray_t* java_lang_String::get_value() const
{
	return get<java_handle_chararray_t*>(_handle, offset_value);
}

#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
inline int32_t java_lang_String::get_offset() const
{
	return get<int32_t>(_handle, offset_offset);
}

inline int32_t java_lang_String::get_count() const
{
	return get<int32_t>(_handle, offset_count);
}
#endif

inline void java_lang_String::set_value(java_handle_chararray_t* value)
{
	set(_handle, offset_value, value);
}

#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
inline void java_lang_String::set_offset(int32_t value)
{
	set(_handle, offset_offset, value);
}

inline void java_lang_String::set_count(int32_t value)
{
	set(_handle, offset_count, value);
}
#endif

#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
namespace jdk6_str_ops {

inline jsize get_string_count(const java_lang_String &s)
{
	return s.get_count();
}

inline jsize get_string_offset(const java_lang_String &s)
{
	return s.get_offset();
}

}
#endif

namespace jdk7_str_ops {

inline jsize get_string_count(const java_lang_String &s)
{
	return CharArray(s.get_value()).get_length();
}

inline jsize get_string_offset(const java_lang_String &s)
{
	return 0;
}

}

#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
namespace runtime_str_ops = jdk6_str_ops;
#else
namespace runtime_str_ops = jdk7_str_ops;
#endif


/**
 * OpenJDK java/lang/Thread
 */
class java_lang_Thread : public java_lang_Object, private FieldAccess {
private:
	static off_t offset_priority;
	static off_t offset_daemon;
	static off_t offset_group;
	static off_t offset_uncaughtExceptionHandler;
	static off_t offset_threadStatus;
#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	static off_t offset_me;
#endif

public:
	java_lang_Thread(java_handle_t* h) : java_lang_Object(h) {}

	// Getters.
	int32_t        get_priority                () const;
	int32_t        get_daemon                  () const;
	java_handle_t* get_group                   () const;
	java_handle_t* get_uncaughtExceptionHandler() const;

	// Setters.
	void set_priority    (int32_t value);
	void set_group       (java_handle_t* value);
	void set_threadStatus(int32_t value);
#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	void set_me          (java_handle_t* value);
#endif

	// Offset initializers
	static void set_priority_offset(int32_t off)     { offset_priority = off; }
	static void set_daemon_offset(int32_t off)       { offset_daemon = off; }
	static void set_group_offset(int32_t off)        { offset_group = off; }
	static void set_uncaughtExceptionHandler_offset(int32_t off) { offset_uncaughtExceptionHandler = off; }
	static void set_threadStatus_offset(int32_t off) { offset_threadStatus = off; }
#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	static void set_me_offset(int32_t off) { offset_me = off; }
#endif
};


inline int32_t java_lang_Thread::get_priority() const
{
	return get<int32_t>(_handle, offset_priority);
}

inline int32_t java_lang_Thread::get_daemon() const
{
	return get<int32_t>(_handle, offset_daemon);
}

inline java_handle_t* java_lang_Thread::get_group() const
{
	return get<java_handle_t*>(_handle, offset_group);
}

inline java_handle_t* java_lang_Thread::get_uncaughtExceptionHandler() const
{
	return get<java_handle_t*>(_handle, offset_uncaughtExceptionHandler);
}


inline void java_lang_Thread::set_priority(int32_t value)
{
	set(_handle, offset_priority, value);
}

inline void java_lang_Thread::set_group(java_handle_t* value)
{
	set(_handle, offset_group, value);
}

inline void java_lang_Thread::set_threadStatus(int32_t value)
{
	set(_handle, offset_threadStatus, value);
}

#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
inline void java_lang_Thread::set_me(java_handle_t* value)
{
	set(_handle, offset_me, value);
}
#endif


/**
 * OpenJDK java/lang/Throwable
 *
 * Object layout:
 *
 * 0. object header
 * 1. java.lang.Object              backtrace;
 * 2. java.lang.String              detailMessage;
 * 3. java.lang.Throwable           cause;
 * 4. java.lang.StackTraceElement[] stackTrace;
 */
class java_lang_Throwable : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_backtrace     = MEMORY_ALIGN(sizeof(java_object_t),                SIZEOF_VOID_P);
	static const off_t offset_detailMessage = MEMORY_ALIGN(offset_backtrace     + SIZEOF_VOID_P, SIZEOF_VOID_P);
	static const off_t offset_cause         = MEMORY_ALIGN(offset_detailMessage + SIZEOF_VOID_P, SIZEOF_VOID_P);
	static const off_t offset_stackTrace    = MEMORY_ALIGN(offset_cause         + SIZEOF_VOID_P, SIZEOF_VOID_P);

public:
	java_lang_Throwable(java_handle_t* h) : java_lang_Object(h) {}
	java_lang_Throwable(java_handle_t* h, java_handle_bytearray_t* backtrace);

	// Getters.
	java_handle_bytearray_t* get_backtrace    () const;
	java_handle_t*           get_detailMessage() const;
	java_handle_t*           get_cause        () const;

	// Setters.
	void set_backtrace(java_handle_bytearray_t* value);
};


inline java_lang_Throwable::java_lang_Throwable(java_handle_t* h, java_handle_bytearray_t* backtrace) : java_lang_Object(h)
{
	set_backtrace(backtrace);
}


inline java_handle_bytearray_t* java_lang_Throwable::get_backtrace() const
{
	return get<java_handle_bytearray_t*>(_handle, offset_backtrace);
}

inline java_handle_t* java_lang_Throwable::get_detailMessage() const
{
	return get<java_handle_t*>(_handle, offset_detailMessage);
}

inline java_handle_t* java_lang_Throwable::get_cause() const
{
	return get<java_handle_t*>(_handle, offset_cause);
}


inline void java_lang_Throwable::set_backtrace(java_handle_bytearray_t* value)
{
	set(_handle, offset_backtrace, value);
}


/**
 * OpenJDK java/lang/reflect/Constructor
 *
 * Object layout:
 *
 * 0.  object header
 * 1.  boolean                                               override;
 * 2.  java.lang.Class                                       clazz;
 * 3.  int                                                   slot;
 * 4.  java.lang.Class[]                                     parameterTypes;
 * 5.  java.lang.Class[]                                     exceptionTypes;
 * 6.  int                                                   modifiers;
 * 7.  java.lang.String                                      signature;
 * 8.  sun.reflect.generics.repository.ConstructorRepository genericInfo;
 * 9.  byte[]                                                annotations;
 * 10. byte[]                                                parameterAnnotations;
 * 11. java.lang.Class                                       securityCheckCache;
 * 12. sun.reflect.ConstructorAccessor                       constructorAccessor;
 * 13. java.lang.reflect.Constructor                         root;
 * 14. java.util.Map                                         declaredAnnotations;
 */
class java_lang_reflect_Constructor : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_override             = MEMORY_ALIGN(sizeof(java_object_t),                         sizeof(int32_t));
#ifdef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	static const off_t offset_securityCheckCache            = MEMORY_ALIGN(offset_override                      + sizeof(int32_t), SIZEOF_VOID_P);
	/* Members of the actual class java.lang.reflect.Constructor */
	static const off_t offset_clazz                         = MEMORY_ALIGN(offset_securityCheckCache            + sizeof(int32_t), SIZEOF_VOID_P);
#else
	static const off_t offset_clazz                         = MEMORY_ALIGN(offset_override                      + sizeof(int32_t), SIZEOF_VOID_P);
#endif
	static const off_t offset_slot                 = MEMORY_ALIGN(offset_clazz                + SIZEOF_VOID_P,   sizeof(int32_t));
	static const off_t offset_parameterTypes       = MEMORY_ALIGN(offset_slot                 + sizeof(int32_t), SIZEOF_VOID_P);
	static const off_t offset_exceptionTypes       = MEMORY_ALIGN(offset_parameterTypes       + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_modifiers            = MEMORY_ALIGN(offset_exceptionTypes       + SIZEOF_VOID_P,   sizeof(int32_t));
	static const off_t offset_signature            = MEMORY_ALIGN(offset_modifiers            + sizeof(int32_t), SIZEOF_VOID_P);
	static const off_t offset_genericInfo          = MEMORY_ALIGN(offset_signature            + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_annotations          = MEMORY_ALIGN(offset_genericInfo          + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_parameterAnnotations = MEMORY_ALIGN(offset_annotations          + SIZEOF_VOID_P,   SIZEOF_VOID_P);
#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	static const off_t offset_securityCheckCache   = MEMORY_ALIGN(offset_parameterAnnotations + SIZEOF_VOID_P,   SIZEOF_VOID_P);
#endif
	static const off_t offset_constructorAccessor  = MEMORY_ALIGN(offset_securityCheckCache   + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_root                 = MEMORY_ALIGN(offset_constructorAccessor  + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_declaredAnnotations  = MEMORY_ALIGN(offset_root                 + SIZEOF_VOID_P,   SIZEOF_VOID_P);

public:
	java_lang_reflect_Constructor(java_handle_t* h) : java_lang_Object(h) {}
	java_lang_reflect_Constructor(methodinfo* m);

	java_handle_t* new_instance(java_handle_objectarray_t* args);

	// Getters.
	int32_t                  get_override   () const;
	classinfo*               get_clazz      () const;
	int32_t                  get_slot       () const;
	java_handle_bytearray_t* get_annotations() const;

	// Setters.
	void set_clazz               (classinfo* value);
	void set_slot                (int32_t value);
	void set_parameterTypes      (java_handle_objectarray_t* value);
	void set_exceptionTypes      (java_handle_objectarray_t* value);
	void set_modifiers           (int32_t value);
	void set_signature           (java_handle_t* value);
	void set_annotations         (java_handle_bytearray_t* value);
	void set_parameterAnnotations(java_handle_bytearray_t* value);

	// Convenience functions.
	methodinfo* get_method();
};


inline java_lang_reflect_Constructor::java_lang_reflect_Constructor(methodinfo* m)
{
	_handle = builtin_new(class_java_lang_reflect_Constructor);

	if (is_null())
		return;

	int                        slot                 = m - m->clazz->methods;
	java_handle_objectarray_t* parameterTypes       = method_get_parametertypearray(m);
	java_handle_objectarray_t* exceptionTypes       = method_get_exceptionarray(m);
	java_handle_bytearray_t*   annotations          = method_get_annotations(m);
	java_handle_bytearray_t*   parameterAnnotations = method_get_parameterannotations(m);

	set_clazz(m->clazz);
	set_slot(slot);
	set_parameterTypes(parameterTypes);
	set_exceptionTypes(exceptionTypes);
	set_modifiers(m->flags & ACC_CLASS_REFLECT_MASK);
	set_signature(m->signature ? JavaString::from_utf8(m->signature) : NULL);
	set_annotations(annotations);
	set_parameterAnnotations(parameterAnnotations);
}


inline int32_t java_lang_reflect_Constructor::get_override() const
{
	return get<int32_t>(_handle, offset_override);
}

inline classinfo* java_lang_reflect_Constructor::get_clazz() const
{
	return get<classinfo*>(_handle, offset_clazz);
}

inline int32_t java_lang_reflect_Constructor::get_slot() const
{
	return get<int32_t>(_handle, offset_slot);
}

inline java_handle_bytearray_t* java_lang_reflect_Constructor::get_annotations() const
{
	return get<java_handle_bytearray_t*>(_handle, offset_annotations);
}


inline void java_lang_reflect_Constructor::set_clazz(classinfo* value)
{
	set(_handle, offset_clazz, value);
}

inline void java_lang_reflect_Constructor::set_slot(int32_t value)
{
	set(_handle, offset_slot, value);
}

inline void java_lang_reflect_Constructor::set_parameterTypes(java_handle_objectarray_t* value)
{
	set(_handle, offset_parameterTypes, value);
}

inline void java_lang_reflect_Constructor::set_exceptionTypes(java_handle_objectarray_t* value)
{
	set(_handle, offset_exceptionTypes, value);
}

inline void java_lang_reflect_Constructor::set_modifiers(int32_t value)
{
	set(_handle, offset_modifiers, value);
}

inline void java_lang_reflect_Constructor::set_signature(java_handle_t* value)
{
	set(_handle, offset_signature, value);
}

inline void java_lang_reflect_Constructor::set_annotations(java_handle_bytearray_t* value)
{
	set(_handle, offset_annotations, value);
}

inline void java_lang_reflect_Constructor::set_parameterAnnotations(java_handle_bytearray_t* value)
{
	set(_handle, offset_parameterAnnotations, value);
}


inline methodinfo* java_lang_reflect_Constructor::get_method()
{
	classinfo*  c    = get_clazz();
	int32_t     slot = get_slot();
	methodinfo* m    = &(c->methods[slot]);
	return m;
}


/**
 * OpenJDK java/lang/reflect/Field
 *
 * Object layout:
 *
 * 0.  object header
 * 1.  boolean                                         override;
 * 2.  java.lang.Class                                 clazz;
 * 3.  int                                             slot;
 * 4.  java.lang.String                                name;
 * 5.  java.lang.Class                                 type;
 * 6.  int                                             modifiers;
 * 7.  java.lang.String                                signature;
 * 8.  sun.reflect.generics.repository.FieldRepository genericInfo;
 * 9.  byte[]                                          annotations;
 * 10. sun.reflect.FieldAccessor                       fieldAccessor;
 * 11. sun.reflect.FieldAccessor                       overrideFieldAccessor;
 * 12. java.lang.reflect.Field                         root;
 * 13. java.lang.Class                                 securityCheckCache;
 * 14. java.lang.Class                                 securityCheckTargetClassCache;
 * 15. java.util.Map                                   declaredAnnotations;
 */
class java_lang_reflect_Field : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	/* Fields of extended class
	 * java.lang.reflect.AccessibleObject */
	static const off_t offset_override                      = MEMORY_ALIGN(sizeof(java_object_t),                                  sizeof(int32_t));
#ifdef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	static const off_t offset_securityCheckCache            = MEMORY_ALIGN(offset_override                      + sizeof(int32_t), SIZEOF_VOID_P);
	/* Members of the actual class java.lang.reflect.Field */
	static const off_t offset_clazz                         = MEMORY_ALIGN(offset_securityCheckCache            + sizeof(int32_t), SIZEOF_VOID_P);
#else
	static const off_t offset_clazz                         = MEMORY_ALIGN(offset_override                      + sizeof(int32_t), SIZEOF_VOID_P);
#endif
	static const off_t offset_slot                          = MEMORY_ALIGN(offset_clazz                         + SIZEOF_VOID_P,   sizeof(int32_t));
	static const off_t offset_name                          = MEMORY_ALIGN(offset_slot                          + sizeof(int32_t), SIZEOF_VOID_P);
	static const off_t offset_type                          = MEMORY_ALIGN(offset_name                          + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_modifiers                     = MEMORY_ALIGN(offset_type                          + SIZEOF_VOID_P,   sizeof(int32_t));
	static const off_t offset_signature                     = MEMORY_ALIGN(offset_modifiers                     + sizeof(int32_t), SIZEOF_VOID_P);
	static const off_t offset_genericInfo                   = MEMORY_ALIGN(offset_signature                     + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_annotations                   = MEMORY_ALIGN(offset_genericInfo                   + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_fieldAccessor                 = MEMORY_ALIGN(offset_annotations                   + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_overrideFieldAccessor         = MEMORY_ALIGN(offset_fieldAccessor                 + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_root                          = MEMORY_ALIGN(offset_overrideFieldAccessor         + SIZEOF_VOID_P,   SIZEOF_VOID_P);
#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	static const off_t offset_securityCheckCache            = MEMORY_ALIGN(offset_root                          + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_securityCheckTargetClassCache = MEMORY_ALIGN(offset_securityCheckCache            + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_declaredAnnotations           = MEMORY_ALIGN(offset_securityCheckTargetClassCache + SIZEOF_VOID_P,   SIZEOF_VOID_P);
#endif

public:
	java_lang_reflect_Field(java_handle_t* h) : java_lang_Object(h) {}
	java_lang_reflect_Field(fieldinfo* f);

	// Getters.
	int32_t                  get_override   () const;
	classinfo*               get_clazz      () const;
	int32_t                  get_slot       () const;
	java_handle_bytearray_t* get_annotations() const;

	// Setters.
	void set_clazz      (classinfo* value);
	void set_slot       (int32_t value);
	void set_name       (java_handle_t* value);
	void set_type       (classinfo* value);
	void set_modifiers  (int32_t value);
	void set_signature  (java_handle_t* value);
	void set_annotations(java_handle_bytearray_t* value);

	// Convenience functions.
	fieldinfo* get_field() const;
};


inline java_lang_reflect_Field::java_lang_reflect_Field(fieldinfo* f)
{
	_handle = builtin_new(class_java_lang_reflect_Field);

	// OOME.
	if (is_null())
		return;

	set_clazz(f->clazz);
	set_slot(f - f->clazz->fields);
	set_name(JavaString::literal(f->name));
	set_type(field_get_type(f));
	set_modifiers(f->flags);
	set_signature(f->signature ? JavaString::from_utf8(f->signature) : NULL);
	set_annotations(field_get_annotations(f));
}


inline int32_t java_lang_reflect_Field::get_override() const
{
	return get<int32_t>(_handle, offset_override);
}

inline classinfo* java_lang_reflect_Field::get_clazz() const
{
	return get<classinfo*>(_handle, offset_clazz);
}

inline int32_t java_lang_reflect_Field::get_slot() const
{
	return get<int32_t>(_handle, offset_slot);
}

inline java_handle_bytearray_t* java_lang_reflect_Field::get_annotations() const
{
	return get<java_handle_bytearray_t*>(_handle, offset_annotations);
}


inline void java_lang_reflect_Field::set_clazz(classinfo* value)
{
	set(_handle, offset_clazz, value);
}

inline void java_lang_reflect_Field::set_slot(int32_t value)
{
	set(_handle, offset_slot, value);
}

inline void java_lang_reflect_Field::set_name(java_handle_t* value)
{
	set(_handle, offset_name, value);
}

inline void java_lang_reflect_Field::set_type(classinfo* value)
{
	set(_handle, offset_type, value);
}

inline void java_lang_reflect_Field::set_modifiers(int32_t value)
{
	set(_handle, offset_modifiers, value);
}

inline void java_lang_reflect_Field::set_signature(java_handle_t* value)
{
	set(_handle, offset_signature, value);
}

inline void java_lang_reflect_Field::set_annotations(java_handle_bytearray_t* value)
{
	set(_handle, offset_annotations, value);
}


inline fieldinfo* java_lang_reflect_Field::get_field() const
{
	classinfo* c    = get_clazz();
	int32_t    slot = get_slot();
	fieldinfo* f    = &(c->fields[slot]);
	return f;
}


/**
 * OpenJDK java/lang/reflect/Method
 *
 * Object layout:
 *
 * 0.  object header
 * 1.  boolean                                               override;
 * 2.  java.lang.Class                                       clazz;
 * 3.  int                                                   slot;
 * 4.  java.lang.String                                      name;
 * 5.  java.lang.Class                                       returnType;
 * 6.  java.lang.Class[]                                     parameterTypes;
 * 7.  java.lang.Class[]                                     exceptionTypes;
 * 8.  int                                                   modifiers;
 * 9.  java.lang.String                                      signature;
 * 10  sun.reflect.generics.repository.ConstructorRepository genericInfo;
 * 11. byte[]                                                annotations;
 * 12. byte[]                                                parameterAnnotations;
 * 13. byte[]                                                annotationDefault;
 * 14. sun.reflect.MethodAccessor                            methodAccessor;
 * 15. java.lang.reflect.Method                              root;
 * 16. java.lang.Class                                       securityCheckCache;
 * 17. java.lang.Class                                       securityCheckTargetClassCache;
 * 18. java.util.Map                                         declaredAnnotations;
 */
class java_lang_reflect_Method : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	/* Fields of extended class
	 * java.lang.reflect.AccessibleObject */
	static const off_t offset_override                      = MEMORY_ALIGN(sizeof(java_object_t),                                  sizeof(int32_t));
#ifdef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	static const off_t offset_securityCheckCache            = MEMORY_ALIGN(offset_override                      + sizeof(int32_t), SIZEOF_VOID_P);
	/* Members of the actual class java.lang.reflect.Method */
	static const off_t offset_clazz                         = MEMORY_ALIGN(offset_securityCheckCache            + sizeof(int32_t), SIZEOF_VOID_P);
#else
	static const off_t offset_clazz                         = MEMORY_ALIGN(offset_override                      + sizeof(int32_t), SIZEOF_VOID_P);
#endif
	static const off_t offset_slot                          = MEMORY_ALIGN(offset_clazz                         + SIZEOF_VOID_P,   sizeof(int32_t));
	static const off_t offset_name                          = MEMORY_ALIGN(offset_slot                          + sizeof(int32_t), SIZEOF_VOID_P);
	static const off_t offset_returnType                    = MEMORY_ALIGN(offset_name                          + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_parameterTypes                = MEMORY_ALIGN(offset_returnType                    + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_exceptionTypes                = MEMORY_ALIGN(offset_parameterTypes                + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_modifiers                     = MEMORY_ALIGN(offset_exceptionTypes                + SIZEOF_VOID_P,   sizeof(int32_t));
	static const off_t offset_signature                     = MEMORY_ALIGN(offset_modifiers                     + sizeof(int32_t), SIZEOF_VOID_P);
	static const off_t offset_genericInfo                   = MEMORY_ALIGN(offset_signature                     + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_annotations                   = MEMORY_ALIGN(offset_genericInfo                   + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_parameterAnnotations          = MEMORY_ALIGN(offset_annotations                   + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_annotationDefault             = MEMORY_ALIGN(offset_parameterAnnotations          + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_methodAccessor                = MEMORY_ALIGN(offset_annotationDefault             + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_root                          = MEMORY_ALIGN(offset_methodAccessor                + SIZEOF_VOID_P,   SIZEOF_VOID_P);
#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	static const off_t offset_securityCheckCache            = MEMORY_ALIGN(offset_root                          + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_securityCheckTargetClassCache = MEMORY_ALIGN(offset_securityCheckCache            + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_declaredAnnotations           = MEMORY_ALIGN(offset_securityCheckTargetClassCache + SIZEOF_VOID_P,   SIZEOF_VOID_P);
#endif

public:
	java_lang_reflect_Method(java_handle_t* h) : java_lang_Object(h) {}
	java_lang_reflect_Method(methodinfo* m);

	java_handle_t* invoke(java_handle_t* o, java_handle_objectarray_t* args);

	// Getters.
	int32_t                  get_override            () const;
	classinfo*               get_clazz               () const;
	int32_t                  get_slot                () const;
	java_handle_bytearray_t* get_annotations         () const;
	java_handle_bytearray_t* get_parameterAnnotations() const;
	java_handle_bytearray_t* get_annotationDefault   () const;

	// Setters.

	// Convenience functions.
	methodinfo* get_method() const;
};


inline java_lang_reflect_Method::java_lang_reflect_Method(methodinfo* m)
{
	_handle = builtin_new(class_java_lang_reflect_Method);

	if (is_null())
		return;

	set(_handle, offset_clazz,                m->clazz);
	set(_handle, offset_slot,                 (int32_t) (m - m->clazz->methods)); // This cast is important (see PR100).
	set(_handle, offset_name,                 JavaString::literal(m->name));
	set(_handle, offset_returnType,           method_returntype_get(m));
	set(_handle, offset_parameterTypes,       method_get_parametertypearray(m));
	set(_handle, offset_exceptionTypes,       method_get_exceptionarray(m));
	set(_handle, offset_modifiers,            (int32_t) (m->flags & ACC_CLASS_REFLECT_MASK));
	set(_handle, offset_signature,            m->signature ? JavaString::from_utf8(m->signature) : NULL);
	set(_handle, offset_annotations,          method_get_annotations(m));
	set(_handle, offset_parameterAnnotations, method_get_parameterannotations(m));
	set(_handle, offset_annotationDefault,    method_get_annotationdefault(m));
}


inline int32_t java_lang_reflect_Method::get_override() const
{
	return get<int32_t>(_handle, offset_override);
}

inline classinfo* java_lang_reflect_Method::get_clazz() const
{
	return get<classinfo*>(_handle, offset_clazz);
}

inline int32_t java_lang_reflect_Method::get_slot() const
{
	return get<int32_t>(_handle, offset_slot);
}

inline java_handle_bytearray_t* java_lang_reflect_Method::get_annotations() const
{
	return get<java_handle_bytearray_t*>(_handle, offset_annotations);
}

inline java_handle_bytearray_t* java_lang_reflect_Method::get_parameterAnnotations() const
{
	return get<java_handle_bytearray_t*>(_handle, offset_parameterAnnotations);
}

inline java_handle_bytearray_t* java_lang_reflect_Method::get_annotationDefault() const
{
	return get<java_handle_bytearray_t*>(_handle, offset_annotationDefault);
}


inline methodinfo* java_lang_reflect_Method::get_method() const
{
	classinfo*  c    = get_clazz();
	int32_t     slot = get_slot();
	methodinfo* m    = &(c->methods[slot]);
	return m;
}


/**
 * OpenJDK java/nio/Buffer
 *
 * Object layout:
 *
 * 0. object header
 * 1. int  mark;
 * 2. int  position;
 * 3. int  limit;
 * 4. int  capacity;
 * 5. long address;
 */
class java_nio_Buffer : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_mark     = MEMORY_ALIGN(sizeof(java_object_t),          sizeof(int32_t));
	static const off_t offset_position = MEMORY_ALIGN(offset_mark     + sizeof(int32_t), sizeof(int32_t));
	static const off_t offset_limit    = MEMORY_ALIGN(offset_position + sizeof(int32_t), sizeof(int32_t));
	static const off_t offset_capacity = MEMORY_ALIGN(offset_limit    + sizeof(int32_t), sizeof(int32_t));
	static const off_t offset_address  = MEMORY_ALIGN(offset_capacity + sizeof(int32_t), sizeof(int64_t));

public:
	java_nio_Buffer(java_handle_t* h) : java_lang_Object(h) {}

	// Getters.
	void* get_address() const;
	int32_t get_capacity() const;
};


inline void* java_nio_Buffer::get_address() const
{
	return get<void*>(_handle, offset_address);
}

inline int32_t java_nio_Buffer::get_capacity() const
{
	return get<int32_t>(_handle, offset_capacity);
}

#endif // WITH_JAVA_RUNTIME_LIBRARY_OPENJDK


#if defined(WITH_JAVA_RUNTIME_LIBRARY_CLDC1_1)

/**
 * CLDC 1.1 com/sun/cldchi/jvm/FileDescriptor
 *
 * Object layout:
 *
 * 0. object header
 * 1. long   pointer;
 * 2. int    position;
 * 3. int    length;
 */
class com_sun_cldchi_jvm_FileDescriptor : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_pointer  = MEMORY_ALIGN(sizeof(java_object_t),             sizeof(int64_t));
	static const off_t offset_position = MEMORY_ALIGN(offset_pointer  + sizeof(int64_t), sizeof(int32_t));
	static const off_t offset_length   = MEMORY_ALIGN(offset_position + sizeof(int32_t), sizeof(int32_t));

public:
	com_sun_cldchi_jvm_FileDescriptor(java_handle_t* h) : java_lang_Object(h) {}
	com_sun_cldchi_jvm_FileDescriptor(java_handle_t* h, int64_t pointer, int32_t position, int32_t length);
	com_sun_cldchi_jvm_FileDescriptor(java_handle_t* h, com_sun_cldchi_jvm_FileDescriptor& fd);

	// Getters.
	int64_t get_pointer () const;
	int32_t get_position() const;
	int32_t get_length  () const;

	// Setters.
	void set_pointer (int64_t value);
	void set_position(int32_t value);
	void set_length  (int32_t value);
};


inline com_sun_cldchi_jvm_FileDescriptor::com_sun_cldchi_jvm_FileDescriptor(java_handle_t* h, int64_t pointer, int32_t position, int32_t length) : java_lang_Object(h)
{
	set_pointer(pointer);
	set_position(position);
	set_length(length);
}

inline com_sun_cldchi_jvm_FileDescriptor::com_sun_cldchi_jvm_FileDescriptor(java_handle_t* h, com_sun_cldchi_jvm_FileDescriptor& fd) : java_lang_Object(h)
{
	com_sun_cldchi_jvm_FileDescriptor(h, fd.get_pointer(), fd.get_position(), fd.get_length());
}


inline int64_t com_sun_cldchi_jvm_FileDescriptor::get_pointer() const
{
	return get<int64_t>(_handle, offset_pointer);
}

inline int32_t com_sun_cldchi_jvm_FileDescriptor::get_position() const
{
	return get<int32_t>(_handle, offset_position);
}

inline int32_t com_sun_cldchi_jvm_FileDescriptor::get_length() const
{
	return get<int32_t>(_handle, offset_length);
}


inline void com_sun_cldchi_jvm_FileDescriptor::set_pointer(int64_t value)
{
	set(_handle, offset_pointer, value);
}

inline void com_sun_cldchi_jvm_FileDescriptor::set_position(int32_t value)
{
	set(_handle, offset_position, value);
}

inline void com_sun_cldchi_jvm_FileDescriptor::set_length(int32_t value)
{
	set(_handle, offset_length, value);
}


/**
 * CLDC 1.1 java/lang/String
 *
 * Object layout:
 *
 * 0. object header
 * 1. char[] value;
 * 2. int    offset;
 * 3. int    count;
 */
class java_lang_String : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_value  = MEMORY_ALIGN(sizeof(java_object_t),           SIZEOF_VOID_P);
	static const off_t offset_offset = MEMORY_ALIGN(offset_value  + SIZEOF_VOID_P,   sizeof(int32_t));
	static const off_t offset_count  = MEMORY_ALIGN(offset_offset + sizeof(int32_t), sizeof(int32_t));

public:
	java_lang_String(java_handle_t* h) : java_lang_Object(h) {}
	java_lang_String(java_handle_t* h, java_handle_chararray_t* value, int32_t count, int32_t offset = 0);

	// Getters.
	java_handle_chararray_t* get_value () const;
	int32_t                  get_offset() const;
	int32_t                  get_count () const;

	// Setters.
	void set_value (java_handle_chararray_t* value);
	void set_offset(int32_t value);
	void set_count (int32_t value);

	// Raw access
	static inline void set_fields(java_handle_t *str, java_handle_chararray_t* value) {
		set(str, offset_value,  value);
		set(str, offset_offset, 0);
		set(str, offset_count,  CharArray(value).get_length());
	}

	static inline java_handle_chararray_t *get_value(java_handle_t *str) {
		return get<java_handle_chararray_t*>(str, offset_value);
	}
	static inline int32_t get_count(java_handle_t *str) {
		return get<int32_t>(str, offset_count);
	}
	static inline int32_t get_offset(java_handle_t *str) {
		return get<int32_t>(str, offset_offset);
	}
};

inline java_lang_String::java_lang_String(java_handle_t* h, java_handle_chararray_t* value, int32_t count, int32_t offset) : java_lang_Object(h)
{
	set_value(value);
	set_offset(offset);
	set_count(count);
}

inline java_handle_chararray_t* java_lang_String::get_value() const
{
	return get<java_handle_chararray_t*>(_handle, offset_value);
}

inline int32_t java_lang_String::get_offset() const
{
	return get<int32_t>(_handle, offset_offset);
}

inline int32_t java_lang_String::get_count() const
{
	return get<int32_t>(_handle, offset_count);
}

inline void java_lang_String::set_value(java_handle_chararray_t* value)
{
	set(_handle, offset_value, value);
}

inline void java_lang_String::set_offset(int32_t value)
{
	set(_handle, offset_offset, value);
}

inline void java_lang_String::set_count(int32_t value)
{
	set(_handle, offset_count, value);
}


/**
 * CLDC 1.1 java/lang/Thread
 *
 * Object layout:
 *
 * 0. object header
 * 1. int                priority;
 * 2. java.lang.Runnable runnable;
 * 3. java.lang.Object   vm_thread;
 * 4. int                is_terminated;
 * 5. int                is_stillborn;
 * 6. char[]             name;
 */
class java_lang_Thread : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_priority      = MEMORY_ALIGN(sizeof(java_object_t),              sizeof(int32_t));
	static const off_t offset_runnable      = MEMORY_ALIGN(offset_priority      + sizeof(int32_t), SIZEOF_VOID_P);
	static const off_t offset_vm_thread     = MEMORY_ALIGN(offset_runnable      + SIZEOF_VOID_P,   SIZEOF_VOID_P);
	static const off_t offset_is_terminated = MEMORY_ALIGN(offset_vm_thread     + SIZEOF_VOID_P,   sizeof(int32_t));
	static const off_t offset_is_stillborn  = MEMORY_ALIGN(offset_is_terminated + sizeof(int32_t), sizeof(int32_t));
	static const off_t offset_name          = MEMORY_ALIGN(offset_is_stillborn  + sizeof(int32_t), SIZEOF_VOID_P);

public:
	java_lang_Thread(java_handle_t* h) : java_lang_Object(h) {}
// 	java_lang_Thread(threadobject* t);

	// Getters.
	int32_t                  get_priority () const;
	threadobject*            get_vm_thread() const;
	java_handle_chararray_t* get_name     () const;

	// Setters.
	void set_vm_thread(threadobject* value);
};


inline int32_t java_lang_Thread::get_priority() const
{
	return get<int32_t>(_handle, offset_priority);
}

inline threadobject* java_lang_Thread::get_vm_thread() const
{
	return get<threadobject*>(_handle, offset_vm_thread);
}

inline java_handle_chararray_t* java_lang_Thread::get_name() const
{
	return get<java_handle_chararray_t*>(_handle, offset_name);
}


inline void java_lang_Thread::set_vm_thread(threadobject* value)
{
	set(_handle, offset_vm_thread, value);
}


/**
 * CLDC 1.1 java/lang/Throwable
 *
 * Object layout:
 *
 * 0. object header
 * 1. java.lang.String detailMessage;
 * 2. java.lang.Object backtrace;
 */
class java_lang_Throwable : public java_lang_Object, private FieldAccess {
private:
	// Static offsets of the object's instance fields.
	// TODO These offsets need to be checked on VM startup.
	static const off_t offset_detailMessage = MEMORY_ALIGN(sizeof(java_object_t),                SIZEOF_VOID_P);
	static const off_t offset_backtrace     = MEMORY_ALIGN(offset_detailMessage + SIZEOF_VOID_P, SIZEOF_VOID_P);

public:
	java_lang_Throwable(java_handle_t* h) : java_lang_Object(h) {}

	// Getters.
	java_handle_t*           get_detailMessage() const;
	java_handle_bytearray_t* get_backtrace    () const;

	// Setters.
	void set_backtrace(java_handle_bytearray_t* value);
};


inline java_handle_t* java_lang_Throwable::get_detailMessage() const
{
	return get<java_handle_t*>(_handle, offset_detailMessage);
}

inline java_handle_bytearray_t* java_lang_Throwable::get_backtrace() const
{
	return get<java_handle_bytearray_t*>(_handle, offset_backtrace);
}


inline void java_lang_Throwable::set_backtrace(java_handle_bytearray_t* value)
{
	set(_handle, offset_backtrace, value);
}

#endif // WITH_JAVA_RUNTIME_LIBRARY_CLDC1_1

#endif // JAVAOBJECTS_HPP_


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
