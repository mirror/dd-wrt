/* src/native/jni.hpp - JNI types and data structures

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


/* jni.hpp *********************************************************************

   ATTENTION: We include this file before we actually define our own
   jni.h.  We do this because otherwise we can get into unresolvable
   circular header dependencies.

   GNU Classpath's headers define:

   #define __CLASSPATH_JNI_MD_H__
   #define _CLASSPATH_JNI_H

   and jni.h uses:

   _CLASSPATH_VM_JNI_TYPES_DEFINED
   
   OpenJDK's headers define:

   #define _JAVASOFT_JNI_MD_H_
   #define _JAVASOFT_JNI_H_

   and jni.h uses:

   JNI_TYPES_ALREADY_DEFINED_IN_JNI_MD_H

   CLASSPATH_JNI_MD_H and CLASSPATH_JNI_H are defined in config.h.

   We include both headers (jni.h and jni_md.h) with the absolute path
   so we can be sure that the preprocessor does not pick up another
   header from the search path.  Furthermore we include jni_md.h
   before jni.h as the latter includes the former.

*******************************************************************************/


#ifndef _JNI_HPP
#define _JNI_HPP

#include <stdint.h>                     // for int32_t
#include "config.h"                     // for INCLUDE_JNI_H, etc
#include "native/jni.hpp"               // for JNI_VERSION_1_6, etc
#include "vm/global.hpp"                // for java_handle_t, etc

struct hashtable_global_ref_entry;

// Define these to override the typedefs in jni.h
#define _CLASSPATH_VM_JNI_TYPES_DEFINED          1 // GNU Classpath
#define JNI_TYPES_ALREADY_DEFINED_IN_JNI_MD_H    1 // OpenJDK

// First include jni_md.h so we have the Java primitive types.
#include INCLUDE_JNI_MD_H


// Include the C++ wrapper classes.
//#include "vm/javaobjects.hpp"

// Some additional JNI version numbers, we currently support JNI 1.6.
#define JNI_VERSION_SUPPORTED JNI_VERSION_1_6
#define JNI_VERSION_CACAO     0xcaca0000


// Typedef the JNI types.
typedef java_handle_t*              jobject;
typedef java_handle_t*              jclass;
typedef java_handle_t*              jstring;
typedef java_handle_t*              jthrowable;
typedef java_handle_t*              jweak; // FIXME
typedef java_handle_array_t*        jarray;
typedef java_handle_objectarray_t*  jobjectArray;
typedef java_handle_booleanarray_t* jbooleanArray;
typedef java_handle_bytearray_t*    jbyteArray;
typedef java_handle_chararray_t*    jcharArray;
typedef java_handle_shortarray_t*   jshortArray;
typedef java_handle_intarray_t*     jintArray;
typedef java_handle_longarray_t*    jlongArray;
typedef java_handle_floatarray_t*   jfloatArray;
typedef java_handle_doublearray_t*  jdoubleArray;


// We need some additional stuff for the various JNI headers.
#if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH) || defined(WITH_JAVA_RUNTIME_LIBRARY_CLDC1_1)

// These typedefs and defines are copied from GNU Classpath's jni.h
#define JNI_TRUE true
#define JNI_FALSE false

class _Jv_JNIEnv;
class _Jv_JavaVM;
typedef _Jv_JNIEnv JNIEnv;
typedef _Jv_JavaVM JavaVM;

#elif defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)

// These typedefs are copied from OpenJDK's jni.h
typedef unsigned char   jboolean;
typedef unsigned short  jchar;
typedef short           jshort;
typedef float           jfloat;
typedef double          jdouble;

typedef jint            jsize;

typedef union jvalue {
    jboolean z;
    jbyte    b;
    jchar    c;
    jshort   s;
    jint     i;
    jlong    j;
    jfloat   f;
    jdouble  d;
    jobject  l;
} jvalue;

struct _jfieldID;
typedef struct _jfieldID *jfieldID;

struct _jmethodID;
typedef struct _jmethodID *jmethodID;

/* Return values from jobjectRefType */
typedef enum _jobjectType {
     JNIInvalidRefType    = 0,
     JNILocalRefType      = 1,
     JNIGlobalRefType     = 2,
     JNIWeakGlobalRefType = 3
} jobjectRefType;

#endif


// Now include jni.h to complete the JNI header.
#include INCLUDE_JNI_H

/* CACAO related stuff ********************************************************/

extern "C" const JNIInvokeInterface_  _Jv_JNIInvokeInterface;
extern "C" JNINativeInterface_        _Jv_JNINativeInterface;


/* hashtable_global_ref_entry *************************************************/

typedef struct hashtable_global_ref_entry hashtable_global_ref_entry;

struct hashtable_global_ref_entry {
	java_object_t              *o;      /* object pointer of global ref       */
	int32_t                     refs;   /* references of the current pointer  */
	hashtable_global_ref_entry *hashlink; /* link for external chaining       */
};


/* function prototypes ********************************************************/

bool jni_init(void);
bool jni_version_check(int version);


#endif // _JNI_HPP


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
