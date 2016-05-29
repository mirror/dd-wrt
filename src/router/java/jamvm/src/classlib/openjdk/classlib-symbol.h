/*
 * Copyright (C) 2010, 2011, 2012, 2013, 2014
 * Robert Lougher <rob@jamvm.org.uk>.
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

#if OPENJDK_VERSION == 6
#define REMOVE_THREAD_NAME "remove"
#else
#define REMOVE_THREAD_NAME "threadTerminated"
#endif

#define CLASSLIB_SYMBOLS_DO(action) \
    action(tid, "tid"), \
    action(add, "add"), \
    action(base, "base"), \
    action(form, "form"), \
    action(type, "type"), \
    action(flags, "flags"), \
    action(rtype, "rtype"), \
    action(_JI__V, "(JI)V"), \
    action(ptypes, "ptypes"), \
    action(invoke, "invoke"), \
    action(target, "target"), \
    action(array_J, "[J"), \
    action(eetop, "eetop"), \
    action(parent, "parent"), \
    action(vmentry, "vmentry"), \
    action(classes, "classes"), \
    action(vmtarget, "vmtarget"), \
    action(dispatch, "dispatch"), \
    action(capacity, "capacity"), \
    action(shutdown, "shutdown"), \
    action(findNative, "findNative"), \
    action(linkMethod, "linkMethod"), \
    action(invokeExact, "invokeExact"), \
    action(invokeBasic, "invokeBasic"), \
    action(linkCallSite, "linkCallSite"), \
    action(threadStatus, "threadStatus"), \
    action(linkToVirtual, "linkToVirtual"), \
    action(linkToStatic, "linkToStatic"), \
    action(linkToSpecial, "linkToSpecial"), \
    action(linkToInterface, "linkToInterface"), \
    action(getFromClass, "getFromClass"), \
    action(constantPoolOop, "constantPoolOop"), \
    action(sun_misc_Signal, "sun/misc/Signal"), \
    action(removeThreadName, REMOVE_THREAD_NAME), \
    action(java_lang_Shutdown, "java/lang/Shutdown"), \
    action(___java_lang_Class, "()Ljava/lang/Class;"), \
    action(___java_lang_Object, "()Ljava/lang/Object;"), \
    action(java_lang_Exception, "java/lang/Exception"), \
    action(findMethodHandleType, "findMethodHandleType"), \
    action(linkMethodHandleConstant, "linkMethodHandleConstant"), \
    action(uncaughtExceptionHandler, "uncaughtExceptionHandler"), \
    action(java_lang_RuntimeException, "java/lang/RuntimeException"), \
    action(_java_lang_Exception__V, "(Ljava/lang/Exception;)V"), \
    action(sig_java_util_vector, "Ljava/util/Vector;"), \
    action(array_java_lang_Object, "[Ljava/lang/Object;"), \
    action(initializeSystemClass, "initializeSystemClass"), \
    action(sun_reflect_ConstantPool, "sun/reflect/ConstantPool"), \
    action(java_lang_invoke_CallSite, "java/lang/invoke/CallSite"), \
    action(java_nio_DirectByteBuffer, "java/nio/DirectByteBuffer"), \
    action(java_lang_invoke_LambdaForm, "java/lang/invoke/LambdaForm"), \
    action(java_lang_invoke_MemberName, "java/lang/invoke/MemberName"), \
    action(java_lang_invoke_MethodType, "java/lang/invoke/MethodType"), \
    action(java_lang_invoke_MethodHandle, "java/lang/invoke/MethodHandle"), \
    action(sun_reflect_MagicAccessorImpl, "sun/reflect/MagicAccessorImpl"), \
    action(java_lang_BootstrapMethodError, "java/lang/BootstrapMethodError"), \
    action(sun_reflect_MethodAccessorImpl, "sun/reflect/MethodAccessorImpl"), \
    action(sig_java_lang_invoke_LambdaForm, "Ljava/lang/invoke/LambdaForm;"), \
    action(sig_java_lang_invoke_MemberName, "Ljava/lang/invoke/MemberName;"), \
    action(sig_sun_reflect_CallerSensitive, "Lsun/reflect/CallerSensitive;"), \
    action(java_lang_invoke_MagicLambdaImpl, \
           "java/lang/invoke/MagicLambdaImpl"), \
    action(sig_java_lang_invoke_MethodHandle, \
           "Ljava/lang/invoke/MethodHandle;"), \
    action(array_java_lang_reflect_Parameter, \
           "[Ljava/lang/reflect/Parameter;"), \
    action(sun_reflect_DelegatingClassLoader, \
           "sun/reflect/DelegatingClassLoader"), \
    action(sun_reflect_ConstructorAccessorImpl, \
           "sun/reflect/ConstructorAccessorImpl"), \
    action(java_lang_ClassLoader_NativeLibrary, \
           "java/lang/ClassLoader$NativeLibrary"), \
    action(java_lang_invoke_MethodHandleNatives, \
           "java/lang/invoke/MethodHandleNatives"), \
    action(sig_java_lang_invoke_LambdaForm_Hidden, \
           "Ljava/lang/invoke/LambdaForm$Hidden;"), \
    action(sig_java_lang_invoke_LambdaForm_Compiled, \
           "Ljava/lang/invoke/LambdaForm$Compiled;"), \
    action(java_security_PrivilegedActionException, \
           "java/security/PrivilegedActionException"), \
    action(sun_reflect_UnsafeStaticFieldAccessorImpl, \
           "sun/reflect/UnsafeStaticFieldAccessorImpl"), \
    action(_java_lang_ClassLoader_java_lang_String__J, \
           "(Ljava/lang/ClassLoader;Ljava/lang/String;)J"), \
    action(_java_lang_ThreadGroup_java_lang_String__V, \
           "(Ljava/lang/ThreadGroup;Ljava/lang/String;)V"), \
    action(_java_lang_ThreadGroup_java_lang_Runnable__V, \
           "(Ljava/lang/ThreadGroup;Ljava/lang/Runnable;)V"), \
    action(java_lang_reflect_field_init_sig, \
           "(Ljava/lang/Class;Ljava/lang/String;" \
           "Ljava/lang/Class;IILjava/lang/String;[B)V"), \
    action(java_lang_reflect_cons_init_sig, \
           "(Ljava/lang/Class;[Ljava/lang/Class;[Ljava/lang/Class;" \
           "IILjava/lang/String;[B[B)V"), \
    action(java_lang_reflect_mthd_invoke_sig, \
           "(Ljava/lang/Object;[Ljava/lang/Object;)Ljava/lang/Object;"), \
    action(java_lang_invoke_MHN_linkMethod_sig, \
           "(Ljava/lang/Class;ILjava/lang/Class;Ljava/lang/String;" \
           "Ljava/lang/Object;[Ljava/lang/Object;)" \
           "Ljava/lang/invoke/MemberName;"), \
    action(java_lang_invoke_MHN_linkCallSite_sig, \
           "(Ljava/lang/Object;Ljava/lang/Object;Ljava/lang/Object;" \
           "Ljava/lang/Object;Ljava/lang/Object;[Ljava/lang/Object;)" \
           "Ljava/lang/invoke/MemberName;"), \
    action(java_lang_invoke_MHN_findMethodType_sig, \
           "(Ljava/lang/Class;[Ljava/lang/Class;)" \
           "Ljava/lang/invoke/MethodType;"), \
    action(java_lang_invoke_MHN_linkMethodHandleConstant_sig, \
           "(Ljava/lang/Class;ILjava/lang/Class;" \
           "Ljava/lang/String;Ljava/lang/Object;)" \
           "Ljava/lang/invoke/MethodHandle;"), \
    action(java_lang_reflect_param_init_sig, \
           "(Ljava/lang/String;ILjava/lang/reflect/Executable;I)V"), \
    action(java_lang_reflect_mthd_init_sig, \
           "(Ljava/lang/Class;Ljava/lang/String;[Ljava/lang/Class;" \
           "Ljava/lang/Class;[Ljava/lang/Class;IILjava/lang/String;[B[B[B)V")
