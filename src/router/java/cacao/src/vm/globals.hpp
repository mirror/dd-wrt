/* src/vm/globals.hpp - global variables

   Copyright (C) 1996-2011
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


#ifndef _GLOBALS_HPP
#define _GLOBALS_HPP

#include "config.h"

#include <stdint.h>

#include "vm/class.hpp"

// Classes.

/* Important system classes. */

extern classinfo *class_java_lang_Object;
extern classinfo *class_java_lang_Class;
extern classinfo *class_java_lang_ClassLoader;
extern classinfo *class_java_lang_Cloneable;
extern classinfo *class_java_lang_SecurityManager;
extern classinfo *class_java_lang_String;
extern classinfo *class_java_lang_System;
extern classinfo *class_java_lang_Thread;
extern classinfo *class_java_lang_ThreadGroup;
extern classinfo *class_java_lang_Throwable;
extern classinfo *class_java_io_Serializable;

/* Important system exceptions. */

extern classinfo *class_java_lang_Exception;
extern classinfo *class_java_lang_ClassNotFoundException;
extern classinfo *class_java_lang_RuntimeException;

#if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)
extern classinfo *class_java_lang_VMSystem;
extern classinfo *class_java_lang_VMThread;
extern classinfo *class_java_lang_VMThrowable;
#endif

#if defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)
extern classinfo *class_sun_misc_Signal;
extern classinfo *class_sun_reflect_MagicAccessorImpl;
extern classinfo *class_sun_reflect_MethodAccessorImpl;
extern classinfo *class_sun_reflect_ConstructorAccessorImpl;
#endif

#if defined(ENABLE_JAVASE)
extern classinfo *class_java_lang_Void;
#endif

extern classinfo *class_java_lang_Boolean;
extern classinfo *class_java_lang_Byte;
extern classinfo *class_java_lang_Character;
extern classinfo *class_java_lang_Short;
extern classinfo *class_java_lang_Integer;
extern classinfo *class_java_lang_Long;
extern classinfo *class_java_lang_Float;
extern classinfo *class_java_lang_Double;

/* some classes which may be used more often */

#if defined(ENABLE_JAVASE)
extern classinfo *class_java_lang_StackTraceElement;
extern classinfo *class_java_lang_reflect_Constructor;
extern classinfo *class_java_lang_reflect_Field;
extern classinfo *class_java_lang_reflect_Method;
extern classinfo *class_java_security_PrivilegedAction;
extern classinfo *class_java_util_Vector;
extern classinfo *class_java_util_HashMap;

# if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)
extern classinfo *class_java_lang_reflect_VMConstructor;
extern classinfo *class_java_lang_reflect_VMField;
extern classinfo *class_java_lang_reflect_VMMethod;
# endif

extern classinfo *arrayclass_java_lang_Object;

# if defined(ENABLE_ANNOTATIONS)
extern classinfo *class_sun_reflect_ConstantPool;
#  if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)
extern classinfo *class_sun_reflect_annotation_AnnotationParser;
#  endif
# endif
#endif


/* pseudo classes for the type checker ****************************************/

/*
 * pseudo_class_Arraystub
 *     (extends Object implements Cloneable, java.io.Serializable)
 *
 *     If two arrays of incompatible component types are merged,
 *     the resulting reference has no accessible components.
 *     The result does, however, implement the interfaces Cloneable
 *     and java.io.Serializable. This pseudo class is used internally
 *     to represent such results. (They are *not* considered arrays!)
 *
 * pseudo_class_Null
 *
 *     This pseudo class is used internally to represent the
 *     null type.
 *
 * pseudo_class_New
 *
 *     This pseudo class is used internally to represent the
 *     the 'uninitialized object' type.
 */

extern classinfo *pseudo_class_Arraystub;
extern classinfo *pseudo_class_Null;
extern classinfo *pseudo_class_New;

#endif // _GLOBALS_HPP


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
