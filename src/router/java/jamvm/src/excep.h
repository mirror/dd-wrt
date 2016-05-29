/*
 * Copyright (C) 2008 Robert Lougher <rob@jamvm.org.uk>.
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

#include "classlib-excep.h"

#define EXCEPTIONS_DO(action) \
    action(java_lang_LinkageError), \
    action(java_lang_InternalError), \
    action(java_lang_ClassFormatError), \
    action(java_lang_NoSuchFieldError), \
    action(java_lang_OutOfMemoryError), \
    action(java_lang_NoSuchMethodError), \
    action(java_lang_InstantiationError), \
    action(java_lang_IllegalAccessError), \
    action(java_lang_ClassCastException), \
    action(java_lang_StackOverflowError), \
    action(java_lang_ArithmeticException), \
    action(java_lang_AbstractMethodError), \
    action(java_lang_ArrayStoreException), \
    action(java_lang_NullPointerException), \
    action(java_lang_NoClassDefFoundError), \
    action(java_lang_UnsatisfiedLinkError), \
    action(java_lang_InterruptedException), \
    action(java_lang_InstantiationException), \
    action(java_lang_ClassNotFoundException), \
    action(java_lang_IllegalAccessException), \
    action(java_lang_IllegalArgumentException), \
    action(java_lang_NegativeArraySizeException), \
    action(java_lang_IllegalThreadStateException), \
    action(java_lang_IllegalMonitorStateException), \
    action(java_lang_IncompatibleClassChangeError), \
    action(java_lang_ArrayIndexOutOfBoundsException), \
    action(java_lang_StringIndexOutOfBoundsException)

#define EXCEPTION_ENUM(name) exception_##name

enum {
    CLASSLIB_EXCEPTIONS_DO(EXCEPTION_ENUM)
    EXCEPTIONS_DO(EXCEPTION_ENUM),
    MAX_EXCEPTION_ENUM
}; 
