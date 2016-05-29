/*
 * Copyright (C) 2010 Robert Lougher <rob@jamvm.org.uk>.
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

#define CLASSLIB_SYMBOLS_DO(action)  \
    action(f, "f"), \
    action(m, "m"), \
    action(pd, "pd"), \
    action(cap, "cap"), \
    action(cons, "cons"), \
    action(data, "data"), \
    action(root, "root"), \
    action(type, "type"), \
    action(flag, "flag"), \
    action(thread, "thread"), \
    action(vmData, "vmData"), \
    action(vmdata, "vmdata"), \
    action(vmThread, "vmThread"), \
    action(threadId, "threadId"), \
    action(hashtable, "hashtable"), \
    action(addThread, "addThread"), \
    action(removeThread, "removeThread"), \
    action(exceptionHandler, "exceptionHandler"), \
    action(createBootPackage, "createBootPackage"), \
    action(newLibraryUnloader, "newLibraryUnloader"), \
    action(java_lang_VMThread, "java/lang/VMThread"), \
    action(java_lang_VMRuntime, "java/lang/VMRuntime"), \
    action(java_lang_VMThrowable, "java/lang/VMThrowable"), \
    action(java_lang_reflect_VMField, "java/lang/reflect/VMField"), \
    action(gnu_classpath_Pointer32, "gnu/classpath/Pointer32"), \
    action(gnu_classpath_Pointer64, "gnu/classpath/Pointer64"), \
    action(java_lang_VMClassLoader, "java/lang/VMClassLoader"), \
    action(java_lang_reflect_VMMethod, "java/lang/reflect/VMMethod"), \
    action(array_java_lang_Package, "[Ljava/lang/Package;"), \
    action(sig_java_lang_VMThread, "Ljava/lang/VMThread;"), \
    action(sig_gnu_classpath_Pointer, "Lgnu/classpath/Pointer;"), \
    action(sig_java_lang_reflect_VMField, "Ljava/lang/reflect/VMField;"), \
    action(sig_java_lang_reflect_VMMethod, "Ljava/lang/reflect/VMMethod;"), \
    action(java_lang_reflect_VMConstructor, \
           "java/lang/reflect/VMConstructor"), \
    action(jamvm_java_lang_VMClassLoaderData, \
           "jamvm/java/lang/VMClassLoaderData"), \
    action(java_nio_DirectByteBufferImpl_ReadWrite, \
           "java/nio/DirectByteBufferImpl$ReadWrite"), \
    action(sig_java_lang_reflect_VMConstructor, \
           "Ljava/lang/reflect/VMConstructor;"), \
    action(sig_java_security_ProtectionDomain, \
           "Ljava/security/ProtectionDomain;"), \
    action(_java_lang_String_I__java_lang_Package, \
           "(Ljava/lang/String;I)Ljava/lang/Package;"), \
    action(_java_lang_VMThread_java_lang_String_I_Z__V, \
           "(Ljava/lang/VMThread;Ljava/lang/String;IZ)V"), \
    action(_java_lang_Object_gnu_classpath_Pointer_III__V, \
           "(Ljava/lang/Object;Lgnu/classpath/Pointer;III)V")

