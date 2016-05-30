/* src/vm/globals.cpp - global variables

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


#include "config.h"

struct classinfo;

// Classes.

/* Important system classes. */

classinfo *class_java_lang_Object;
classinfo *class_java_lang_Class;
classinfo *class_java_lang_ClassLoader;
classinfo *class_java_lang_Cloneable;
classinfo *class_java_lang_SecurityManager;
classinfo *class_java_lang_String;
classinfo *class_java_lang_System;
classinfo *class_java_lang_Thread;
classinfo *class_java_lang_ThreadGroup;
classinfo *class_java_lang_Throwable;
classinfo *class_java_io_Serializable;

#if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)
classinfo *class_java_lang_VMSystem;
classinfo *class_java_lang_VMThread;
classinfo *class_java_lang_VMThrowable;
#endif

/* Important system exceptions. */

classinfo *class_java_lang_Exception;
classinfo *class_java_lang_ClassNotFoundException;
classinfo *class_java_lang_RuntimeException;

#if defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)
classinfo *class_sun_misc_Signal;
classinfo *class_sun_reflect_MagicAccessorImpl;
classinfo *class_sun_reflect_MethodAccessorImpl;
classinfo *class_sun_reflect_ConstructorAccessorImpl;
#endif

#if defined(ENABLE_JAVASE)
classinfo *class_java_lang_Void;
#endif
classinfo *class_java_lang_Boolean;
classinfo *class_java_lang_Byte;
classinfo *class_java_lang_Character;
classinfo *class_java_lang_Short;
classinfo *class_java_lang_Integer;
classinfo *class_java_lang_Long;
classinfo *class_java_lang_Float;
classinfo *class_java_lang_Double;

/* some classes which may be used more often */

#if defined(ENABLE_JAVASE)
classinfo *class_java_lang_StackTraceElement;
classinfo *class_java_lang_reflect_Constructor;
classinfo *class_java_lang_reflect_Field;
classinfo *class_java_lang_reflect_Method;
classinfo *class_java_security_PrivilegedAction;
classinfo *class_java_util_Vector;
classinfo *class_java_util_HashMap;

# if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)
classinfo *class_java_lang_reflect_VMConstructor;
classinfo *class_java_lang_reflect_VMField;
classinfo *class_java_lang_reflect_VMMethod;
# endif

classinfo *arrayclass_java_lang_Object;

# if defined(ENABLE_ANNOTATIONS)
classinfo *class_sun_reflect_ConstantPool;
#  if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)
classinfo *class_sun_reflect_annotation_AnnotationParser;
#  endif
# endif
#endif

/* pseudo classes for the typechecker */

classinfo *pseudo_class_Arraystub;
classinfo *pseudo_class_Null;
classinfo *pseudo_class_New;


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
