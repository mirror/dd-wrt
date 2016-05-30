/* src/vm/utf8.inc - utf8 string functions

   Copyright (C) 1996-2005, 2006, 2007, 2008
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

// This header has no include guard on purpose!
// It is meant to be included multiple times

#ifndef UTF8
#	error "You must a function like macro called UTF8 \
	that accepts 2 arguments to use this header"
#endif

UTF8( empty, "" )

UTF8( java_lang_Object, "java/lang/Object" )

UTF8( java_lang_Class,           "java/lang/Class"           )
UTF8( java_lang_ClassLoader,     "java/lang/ClassLoader"     )
UTF8( java_lang_Cloneable,       "java/lang/Cloneable"       )
UTF8( java_lang_SecurityManager, "java/lang/SecurityManager" )
UTF8( java_lang_String,          "java/lang/String"          )
UTF8( java_lang_ThreadGroup,     "java/lang/ThreadGroup"     )

UTF8( java_lang_ClassLoader_NativeLibrary, "java/lang/ClassLoader$NativeLibrary" )

UTF8( java_lang_ref_SoftReference, "java/lang/ref/SoftReference" )

UTF8( java_lang_ref_WeakReference, "java/lang/ref/WeakReference" )

UTF8( java_lang_ref_PhantomReference, "java/lang/ref/PhantomReference" )

UTF8( java_io_Serializable, "java/io/Serializable" )

UTF8( java_lang_Throwable, "java/lang/Throwable" )
UTF8( java_lang_Error,     "java/lang/Error"     )

UTF8( java_lang_ClassCircularityError, "java/lang/ClassCircularityError" )

UTF8( java_lang_ClassFormatError, "java/lang/ClassFormatError" )

UTF8( java_lang_ExceptionInInitializerError, "java/lang/ExceptionInInitializerError" )

UTF8( java_lang_IncompatibleClassChangeError, "java/lang/IncompatibleClassChangeError" )

UTF8( java_lang_InstantiationError, "java/lang/InstantiationError" )

UTF8( java_lang_InternalError, "java/lang/InternalError" )
UTF8( java_lang_LinkageError,  "java/lang/LinkageError"  )

UTF8( java_lang_NoClassDefFoundError, "java/lang/NoClassDefFoundError" )

UTF8( java_lang_OutOfMemoryError, "java/lang/OutOfMemoryError" )

UTF8( java_lang_UnsatisfiedLinkError, "java/lang/UnsatisfiedLinkError" )

UTF8( java_lang_UnsupportedClassVersionError, "java/lang/UnsupportedClassVersionError" )

UTF8( java_lang_VerifyError, "java/lang/VerifyError" )

UTF8( java_lang_VirtualMachineError, "java/lang/VirtualMachineError" )

#if defined(ENABLE_JAVASE)
UTF8( java_lang_AbstractMethodError, "java/lang/AbstractMethodError" )

UTF8( java_lang_NoSuchFieldError, "java/lang/NoSuchFieldError" )

UTF8( java_lang_NoSuchMethodError, "java/lang/NoSuchMethodError" )
#endif

UTF8( java_lang_Exception, "java/lang/Exception" )

UTF8( java_lang_ArithmeticException, "java/lang/ArithmeticException" )

UTF8( java_lang_ArrayIndexOutOfBoundsException, "java/lang/ArrayIndexOutOfBoundsException" )

UTF8( java_lang_ArrayStoreException, "java/lang/ArrayStoreException" )

UTF8( java_lang_ClassCastException, "java/lang/ClassCastException" )

UTF8( java_lang_ClassNotFoundException, "java/lang/ClassNotFoundException" )

UTF8( java_lang_CloneNotSupportedException, "java/lang/CloneNotSupportedException" )

UTF8( java_lang_IllegalAccessException, "java/lang/IllegalAccessException" )

UTF8( java_lang_IllegalArgumentException, "java/lang/IllegalArgumentException" )

UTF8( java_lang_IllegalMonitorStateException, "java/lang/IllegalMonitorStateException" )

UTF8( java_lang_InstantiationException, "java/lang/InstantiationException" )

UTF8( java_lang_InterruptedException, "java/lang/InterruptedException" )

UTF8( java_lang_NegativeArraySizeException, "java/lang/NegativeArraySizeException" )

UTF8( java_lang_NullPointerException, "java/lang/NullPointerException" )

UTF8( java_lang_RuntimeException, "java/lang/RuntimeException" )

UTF8( java_lang_StringIndexOutOfBoundsException, "java/lang/StringIndexOutOfBoundsException" )

UTF8( java_lang_reflect_InvocationTargetException, "java/lang/reflect/InvocationTargetException" )

UTF8( java_security_PrivilegedActionException, "java/security/PrivilegedActionException" )

#if defined(ENABLE_JAVASE)
UTF8( java_lang_Void, "java/lang/Void" )
#endif

UTF8( java_lang_Boolean,   "java/lang/Boolean"   )
UTF8( java_lang_Byte,      "java/lang/Byte"      )
UTF8( java_lang_Character, "java/lang/Character" )
UTF8( java_lang_Short,     "java/lang/Short"     )
UTF8( java_lang_Integer,   "java/lang/Integer"   )
UTF8( java_lang_Long,      "java/lang/Long"      )
UTF8( java_lang_Float,     "java/lang/Float"     )
UTF8( java_lang_Double,    "java/lang/Double"    )

#if defined(ENABLE_JAVASE)
UTF8( java_lang_StackTraceElement, "java/lang/StackTraceElement" )

UTF8( java_lang_reflect_Constructor, "java/lang/reflect/Constructor" )

UTF8( java_lang_reflect_Field, "java/lang/reflect/Field"   )
UTF8( java_lang_reflect_Method, "java/lang/reflect/Method" )

# if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)
UTF8( java_lang_reflect_VMConstructor, "java/lang/reflect/VMConstructor" )
UTF8( java_lang_reflect_VMField,       "java/lang/reflect/VMField"       )
UTF8( java_lang_reflect_VMMethod,      "java/lang/reflect/VMMethod"      )
# endif

UTF8( java_util_Vector, "java/util/Vector" )
#endif

UTF8( InnerClasses,    "InnerClasses"    )
UTF8( ConstantValue,   "ConstantValue"   )
UTF8( Code,            "Code"            )
UTF8( Exceptions,	   "Exceptions"      )
UTF8( LineNumberTable, "LineNumberTable" )
UTF8( SourceFile,      "SourceFile"      )

#if defined(ENABLE_JAVASE)
UTF8( EnclosingMethod, "EnclosingMethod" )
UTF8( Signature,       "Signature"       )
UTF8( StackMapTable,   "StackMapTable"   )

# if defined(ENABLE_JVMTI)
UTF8( LocalVariableTable, "LocalVariableTable" )
# endif

# if defined(ENABLE_ANNOTATIONS)
UTF8( RuntimeVisibleAnnotations,            "RuntimeVisibleAnnotations"            )
UTF8( RuntimeInvisibleAnnotations,          "RuntimeInvisibleAnnotations"          )
UTF8( RuntimeVisibleParameterAnnotations,   "RuntimeVisibleParameterAnnotations"   )
UTF8( RuntimeInvisibleParameterAnnotations, "RuntimeInvisibleParameterAnnotations" )
UTF8( AnnotationDefault,                    "AnnotationDefault"                    )
# endif
#endif

UTF8( init,	                "<init>"               )
UTF8( clinit,	            "<clinit>"             )
UTF8( clone,                "clone"                )
UTF8( finalize,	            "finalize"             )
UTF8( invoke,               "invoke"               )
UTF8( main,                 "main"                 )
UTF8( run,                  "run"                  )

UTF8( add,                  "add"                  )
UTF8( dispatch,             "dispatch"             )
UTF8( remove,               "remove"               )
UTF8( addThread,            "addThread"            )
UTF8( removeThread,         "removeThread"         )
UTF8( put,                  "put"                  )
UTF8( get,                  "get"                  )
UTF8( uncaughtException,    "uncaughtException"    )
UTF8( value,                "value"                )

UTF8( fillInStackTrace,     "fillInStackTrace"     )
UTF8( findNative,           "findNative"           )
UTF8( getSystemClassLoader, "getSystemClassLoader" )
UTF8( initCause,            "initCause"            )
UTF8( loadClass,            "loadClass"            )
UTF8( loadClassInternal,    "loadClassInternal"    )
UTF8( printStackTrace,      "printStackTrace"      )

UTF8( division_by_zero,     "/ by zero"            )

UTF8( Z, "Z" )
UTF8( B, "B" )
UTF8( C, "C" )
UTF8( S, "S" )
UTF8( I, "I" )
UTF8( J, "J" )
UTF8( F, "F" )
UTF8( D, "D" )

UTF8( void__void,                "()V"                     )
UTF8( boolean__void,             "(Z)V"                    )
UTF8( byte__void,                "(B)V"                    )
UTF8( char__void,                "(C)V"                    )
UTF8( short__void,               "(S)V"                    )
UTF8( int__void,                 "(I)V"                    )
UTF8( long__void,                "(J)V"                    )
UTF8( float__void,               "(F)V"                    )
UTF8( double__void,              "(D)V"                    )
UTF8( void__java_lang_Object,    "()Ljava/lang/Object;"    )
UTF8( void__java_lang_Throwable, "()Ljava/lang/Throwable;" )

UTF8( void__java_lang_ClassLoader,                 "()Ljava/lang/ClassLoader;"                    )
UTF8( java_lang_ClassLoader_java_lang_String__J,   "(Ljava/lang/ClassLoader;Ljava/lang/String;)J" )
UTF8( java_lang_Exception__V,                      "(Ljava/lang/Exception;)V"                     )
UTF8( java_lang_Object__java_lang_Object,          "(Ljava/lang/Object;)Ljava/lang/Object;"       )
UTF8( java_lang_String__void,                      "(Ljava/lang/String;)V"                        )
UTF8( java_lang_String__java_lang_Class,           "(Ljava/lang/String;)Ljava/lang/Class;"        )
UTF8( java_lang_Thread__V,                         "(Ljava/lang/Thread;)V"                        )
UTF8( java_lang_Thread_java_lang_Throwable__V,     "(Ljava/lang/Thread;Ljava/lang/Throwable;)V"   )
UTF8( Ljava_lang_ThreadGroup_Ljava_lang_String__V, "(Ljava/lang/ThreadGroup;Ljava/lang/String;)V" )
UTF8( java_lang_Throwable__void,                   "(Ljava/lang/Throwable;)V"                     )
UTF8( java_lang_Throwable__java_lang_Throwable,    "(Ljava/lang/Throwable;)Ljava/lang/Throwable;" )

UTF8( null,              "null"                  )
UTF8( not_named_yet,     "\t<not_named_yet>"     )
UTF8( array_packagename, "\t<the array package>" )

#undef UTF8

/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */
