/*
 * Copyright (C) 2008, 2010, 2012, 2013 Robert Lougher <rob@jamvm.org.uk>.
 *
 * This file is part of JamVM.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "classlib-symbol.h"

extern char *symbol_values[];
#define SYMBOL_NAME_ENUM(name) symbol_##name
#define SYMBOL(name) symbol_values[SYMBOL_NAME_ENUM(name)]

#define SYMBOLS_DO(action) \
    /* Method and field names, etc. */\
    action(I, "I"), \
    action(J, "J"), \
    action(Z, "Z"), \
    action(put, "put"), \
    action(run, "run"), \
    action(main, "main"), \
    action(name, "name"), \
    action(exit, "exit"), \
    action(slot, "slot"), \
    action(clazz, "clazz"), \
    action(queue, "queue"), \
    action(group, "group"), \
    action(count, "count"), \
    action(value, "value"), \
    action(create, "create"), \
    action(daemon, "daemon"), \
    action(offset, "offset"), \
    action(valueOf, "valueOf"), \
    action(enqueue, "enqueue"), \
    action(address, "address"), \
    action(referent, "referent"), \
    action(priority, "priority"), \
    action(finalize, "finalize"), \
    action(backtrace, "backtrace"), \
    action(initCause, "initCause"), \
    action(loadClass, "loadClass"), \
    action(returnType, "returnType"), \
    action(declaringClass, "declaringClass"), \
    action(parameterTypes, "parameterTypes"), \
    action(printStackTrace, "printStackTrace"), \
    action(fillInStackTrace, "fillInStackTrace"), \
    action(uncaughtException, "uncaughtException"), \
    action(contextClassLoader, "contextClassLoader"), \
    action(getSystemClassLoader, "getSystemClassLoader"), \
    \
    /* Constant pool attribute names */\
    action(Code, "Code"), \
    action(Signature, "Signature"), \
    action(Synthetic, "Synthetic"), \
    action(Exceptions, "Exceptions"), \
    action(SourceFile, "SourceFile"), \
    action(InnerClasses, "InnerClasses"), \
    action(ConstantValue, "ConstantValue"), \
    action(LineNumberTable, "LineNumberTable"), \
    action(EnclosingMethod, "EnclosingMethod"), \
    action(MethodParameters, "MethodParameters"), \
    action(BootstrapMethods, "BootstrapMethods"), \
    action(AnnotationDefault, "AnnotationDefault"), \
    action(RuntimeVisibleAnnotations, "RuntimeVisibleAnnotations"), \
    action(RuntimeVisibleTypeAnnotations, "RuntimeVisibleTypeAnnotations"), \
    action(RuntimeVisibleParameterAnnotations, "RuntimeVisibleParameterAnnotations"), \
    \
    /* Primitive type names */\
    action(int, "int"), \
    action(void, "void"), \
    action(byte, "byte"), \
    action(char, "char"), \
    action(long, "long"), \
    action(short, "short"), \
    action(float, "float"), \
    action(double, "double"), \
    action(boolean, "boolean"), \
    \
    /* Class and object initialiser names */\
    action(object_init, "<init>"), \
    action(class_init, "<clinit>"), \
    \
    /* Class names */\
    action(java_lang_Byte, "java/lang/Byte"), \
    action(java_lang_Long, "java/lang/Long"), \
    action(java_lang_Enum, "java/lang/Enum"), \
    action(java_lang_Short, "java/lang/Short"), \
    action(java_lang_Float, "java/lang/Float"), \
    action(java_nio_Buffer, "java/nio/Buffer"), \
    action(java_lang_Class, "java/lang/Class"), \
    action(java_lang_Number, "java/lang/Number"), \
    action(java_lang_Double, "java/lang/Double"), \
    action(java_lang_Object, "java/lang/Object"), \
    action(java_lang_String, "java/lang/String"), \
    action(java_lang_Thread, "java/lang/Thread"), \
    action(java_lang_System, "java/lang/System"), \
    action(java_lang_Boolean, "java/lang/Boolean"), \
    action(java_lang_Integer, "java/lang/Integer"), \
    action(java_util_HashMap, "java/util/HashMap"), \
    action(java_lang_Character, "java/lang/Character"), \
    action(java_lang_Throwable, "java/lang/Throwable"), \
    action(java_lang_Cloneable, "java/lang/Cloneable"), \
    action(java_io_Serializable, "java/io/Serializable"), \
    action(java_lang_ThreadGroup, "java/lang/ThreadGroup"), \
    action(java_lang_ClassLoader, "java/lang/ClassLoader"), \
    action(java_lang_reflect_Field, "java/lang/reflect/Field"), \
    action(java_lang_ref_Reference, "java/lang/ref/Reference"), \
    action(sun_reflect_annotation_AnnotationInvocationHandler, \
           "sun/reflect/annotation/AnnotationInvocationHandler"), \
    action(java_lang_reflect_Method, "java/lang/reflect/Method"), \
    action(java_lang_StackTraceElement, "java/lang/StackTraceElement"), \
    action(java_lang_ref_SoftReference, "java/lang/ref/SoftReference"), \
    action(java_lang_ref_WeakReference, "java/lang/ref/WeakReference"), \
    action(java_lang_reflect_Constructor, "java/lang/reflect/Constructor"), \
    action(java_lang_ref_PhantomReference, "java/lang/ref/PhantomReference"), \
    \
    /* Exception class names */\
    action(java_lang_Error, "java/lang/Error"), \
    action(java_lang_LinkageError, "java/lang/LinkageError"), \
    action(java_lang_InternalError, "java/lang/InternalError"), \
    action(java_lang_ClassFormatError, "java/lang/ClassFormatError"), \
    action(java_lang_OutOfMemoryError, "java/lang/OutOfMemoryError"), \
    action(java_lang_NoSuchFieldError, "java/lang/NoSuchFieldError"), \
    action(java_lang_NoSuchMethodError, "java/lang/NoSuchMethodError"), \
    action(java_lang_ClassCastException, "java/lang/ClassCastException"), \
    action(java_lang_StackOverflowError, "java/lang/StackOverflowError"), \
    action(java_lang_InstantiationError, "java/lang/InstantiationError"), \
    action(java_lang_IllegalAccessError, "java/lang/IllegalAccessError"), \
    action(java_lang_ArithmeticException, "java/lang/ArithmeticException"), \
    action(java_lang_AbstractMethodError, "java/lang/AbstractMethodError"), \
    action(java_lang_ArrayStoreException, "java/lang/ArrayStoreException"), \
    action(java_lang_UnsatisfiedLinkError, "java/lang/UnsatisfiedLinkError"), \
    action(java_lang_InterruptedException, "java/lang/InterruptedException"), \
    action(java_lang_NullPointerException, "java/lang/NullPointerException"), \
    action(java_lang_NoClassDefFoundError, "java/lang/NoClassDefFoundError"), \
    action(java_lang_IllegalAccessException, "java/lang/IllegalAccessException"), \
    action(java_lang_ClassNotFoundException, "java/lang/ClassNotFoundException"), \
    action(java_lang_InstantiationException, "java/lang/InstantiationException"), \
    action(java_lang_IllegalArgumentException, "java/lang/IllegalArgumentException"), \
    action(java_lang_NegativeArraySizeException, "java/lang/NegativeArraySizeException"), \
    action(java_lang_ExceptionInInitializerError, "java/lang/ExceptionInInitializerError"), \
    action(java_lang_IllegalThreadStateException, "java/lang/IllegalThreadStateException"), \
    action(java_lang_IllegalMonitorStateException, "java/lang/IllegalMonitorStateException"), \
    action(java_lang_IncompatibleClassChangeError, "java/lang/IncompatibleClassChangeError"), \
    action(java_lang_ArrayIndexOutOfBoundsException, "java/lang/ArrayIndexOutOfBoundsException"), \
    action(java_lang_StringIndexOutOfBoundsException, "java/lang/StringIndexOutOfBoundsException"), \
    \
    /* Array class names */\
    action(array_C, "[C"), \
    action(array_java_lang_Class, "[Ljava/lang/Class;"), \
    action(array_java_lang_String, "[Ljava/lang/String;"), \
    action(array_java_lang_reflect_Field, "[Ljava/lang/reflect/Field;"), \
    action(array_java_lang_reflect_Method, "[Ljava/lang/reflect/Method;"), \
    action(array_java_lang_StackTraceElement, "[Ljava/lang/StackTraceElement;"), \
    action(array_java_lang_reflect_Constructor, "[Ljava/lang/reflect/Constructor;"), \
    \
    /* Field signatures */\
    action(sig_java_lang_Class, "Ljava/lang/Class;"), \
    action(sig_java_lang_Object, "Ljava/lang/Object;"), \
    action(sig_java_lang_String, "Ljava/lang/String;"), \
    action(sig_java_lang_Thread, "Ljava/lang/Thread;"), \
    action(sig_java_lang_ThreadGroup, "Ljava/lang/ThreadGroup;"), \
    action(sig_java_lang_ClassLoader, "Ljava/lang/ClassLoader;"), \
    action(sig_java_lang_reflect_Field, "Ljava/lang/reflect/Field;"), \
    action(sig_java_lang_reflect_Method, "Ljava/lang/reflect/Method;"), \
    action(sig_java_lang_reflect_Constructor, "Ljava/lang/reflect/Constructor;"), \
    action(sig_java_lang_ref_ReferenceQueue, "Ljava/lang/ref/ReferenceQueue;"), \
    action(sig_java_lang_Thread_UncaughtExceptionHandler, "Ljava/lang/Thread$UncaughtExceptionHandler;"), \
    \
    /* Method signatures */\
    action(___V, "()V"), \
    action(___Z, "()Z"), \
    action(_I__V, "(I)V"), \
    action(_J__V, "(J)V"), \
    action(_java_lang_Thread_java_lang_Throwable__V, \
           "(Ljava/lang/Thread;Ljava/lang/Throwable;)V"), \
    action(_java_lang_Throwable__java_lang_Throwable, \
           "(Ljava/lang/Throwable;)Ljava/lang/Throwable;"), \
    action(_java_lang_String__V, "(Ljava/lang/String;)V"), \
    action(_java_lang_Thread__V, "(Ljava/lang/Thread;)V"), \
    action(_java_lang_Throwable__V, "(Ljava/lang/Throwable;)V"), \
    action(_array_java_lang_String__V, "([Ljava/lang/String;)V"), \
    action(___java_lang_ClassLoader, "()Ljava/lang/ClassLoader;"), \
    action(_java_lang_Object_java_lang_Object__java_lang_Object, \
           "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;"), \
    action(_java_lang_String_java_lang_String_java_lang_String_I__V, \
           "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V"), \
    action(_java_lang_String__java_lang_Class, "(Ljava/lang/String;)Ljava/lang/Class;")

#define SYMBOL_ENUM(name, value) SYMBOL_NAME_ENUM(name)
enum {
    CLASSLIB_SYMBOLS_DO(SYMBOL_ENUM),
    SYMBOLS_DO(SYMBOL_ENUM),
    MAX_SYMBOL_ENUM
}; 
