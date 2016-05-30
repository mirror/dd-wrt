/* src/vm/javaobjects.cpp - functions to create and access Java objects

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


#include "config.h"

#include <stdint.h>

#include "native/vm/reflection.hpp"

#include "vm/access.hpp"
#include "vm/jit/builtin.hpp"
#include "vm/global.hpp"
#include "vm/globals.hpp"
#include "vm/initialize.hpp"
#include "vm/javaobjects.hpp"
#include "vm/vm.hpp"

#include <map>

#if defined(ENABLE_JAVASE)

/**
 * Invokes the static Java method getSystemClassLoader().
 *
 * @return Return value of the invocation or NULL in
 * case of an exception.
 */
java_handle_t* java_lang_ClassLoader::invoke_getSystemClassLoader()
{
	methodinfo    *m;
	java_handle_t *clo;
	classloader_t *cl;

	assert(class_java_lang_Object);
	assert(class_java_lang_ClassLoader);
	assert(class_java_lang_ClassLoader->state & CLASS_LINKED);

	m = class_resolveclassmethod(class_java_lang_ClassLoader,
								 utf8::getSystemClassLoader,
								 utf8::void__java_lang_ClassLoader,
								 class_java_lang_Object,
								 false);

	if (m == NULL)
		return NULL;

	clo = vm_call_method(m, NULL);

	if (clo == NULL)
		return NULL;

	cl = loader_hashtable_classloader_add(clo);

	return cl;
}


/**
 * Constructs a new instance of the class by calling the
 * appropriate Java initializer.
 */
java_lang_management_MemoryUsage::java_lang_management_MemoryUsage(int64_t init, int64_t used, int64_t commited, int64_t maximum)
{
	// Load the class.
	// XXX Maybe this should be made global at some points.
	classinfo* class_java_lang_management_MemoryUsage;
	if (!(class_java_lang_management_MemoryUsage = load_class_bootstrap(Utf8String::from_utf8("java/lang/management/MemoryUsage"))))
		return;

	// Find the appropriate initializer.
	// XXX Maybe this should be made global at some points.
	methodinfo* m = class_findmethod(class_java_lang_management_MemoryUsage,
	                                 utf8::init,
	                                 Utf8String::from_utf8("(JJJJ)V"));

	if (m == NULL)
		return;

	// Instantiate a new object.
	_handle = builtin_new(class_java_lang_management_MemoryUsage);

	if (is_null())
		return;

	// Call initializer.
	(void) vm_call_method(m, _handle, init, used, commited, maximum);
}


/**
 * Constructs a Java object with the given
 * java.lang.reflect.Constructor.
 *
 * @param args     Constructor arguments.
 *
 * @return Handle to Java object.
 */
java_handle_t* java_lang_reflect_Constructor::new_instance(java_handle_objectarray_t* args)
{
	methodinfo* m = get_method();

	// Should we bypass security the checks (AccessibleObject)?
	if (get_override() == false) {
		/* This method is always called like this:
		       [0] java.lang.reflect.Constructor.constructNative (Native Method)
		       [1] java.lang.reflect.Constructor.newInstance
		       [2] <caller>
		*/

		if (!access_check_method(m, 2))
			return NULL;
	}

	// Create a Java object.
	java_handle_t* h = builtin_new(m->clazz);

	if (h == NULL)
		return NULL;

	// Call initializer.
	(void) Reflection::invoke(m, h, args);

	return h;
}


/**
 * Invokes the given method.
 *
 * @param args Method arguments.
 *
 * @return return value of the method
 */
java_handle_t* java_lang_reflect_Method::invoke(java_handle_t* o, java_handle_objectarray_t* args)
{
	methodinfo* m = get_method();

	// Should we bypass security the checks (AccessibleObject)?
	if (get_override() == false) {
#if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)
		/* This method is always called like this:
		       [0] java.lang.reflect.Method.invokeNative (Native Method)
		       [1] java.lang.reflect.Method.invoke (Method.java:329)
		       [2] <caller>
		*/

		if (!access_check_method(m, 2))
			return NULL;
#elif defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)
		/* We only pass 0 here as stacktrace_get_caller_class, which
		   is called from access_check_method, skips
		   java.lang.reflect.Method.invoke(). */

		if (!access_check_method(m, 0))
			return NULL;
#else
# error unknown classpath configuration
#endif
	}

	// Check if method class is initialized.
	if (!(m->clazz->state & CLASS_INITIALIZED))
		if (!initialize_class(m->clazz))
			return NULL;

	// Call the Java method.
	java_handle_t* result = Reflection::invoke(m, o, args);

	return result;
}

struct DynOffsetEntry {
	void (*setter)(int32_t);
	const char *name;
};

typedef std::map<classinfo *, DynOffsetEntry *> RegisteredDynMap;
static RegisteredDynMap dynEntryMap;

static void register_dyn_entry_table(classinfo *c, DynOffsetEntry *entries)
{
	dynEntryMap.insert(std::make_pair(c, entries));
}

static bool runAllSetters(classinfo *c, DynOffsetEntry entries[])
{
	do {
		fieldinfo *fi = class_findfield_by_name(c, Utf8String::from_utf8(entries->name));
		if (!fi)
			return false;
		entries->setter(fi->offset);
	} while ((++entries)->setter);
	return true;
}

bool jobjects_run_dynoffsets_hook(classinfo *c)
{
	RegisteredDynMap::const_iterator it = dynEntryMap.find(c);
	if (it == dynEntryMap.end())
		return true;

	if (!runAllSetters(c, it->second))
		return false;

	return true;
}

#if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)

off_t java_lang_Thread::offset_vmThread;
off_t java_lang_Thread::offset_group;
off_t java_lang_Thread::offset_name;
off_t java_lang_Thread::offset_daemon;
off_t java_lang_Thread::offset_priority;
off_t java_lang_Thread::offset_exceptionHandler;

static DynOffsetEntry dyn_entries_java_lang_Thread[] = {
	{ &java_lang_Thread::set_vmThread_offset,         "vmThread" },
	{ &java_lang_Thread::set_group_offset,            "group" },
	{ &java_lang_Thread::set_name_offset,             "name" },
	{ &java_lang_Thread::set_daemon_offset,           "daemon" },
	{ &java_lang_Thread::set_priority_offset,         "priority" },
	{ &java_lang_Thread::set_exceptionHandler_offset, "exceptionHandler" },
	{ 0, 0 }
};

#elif defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)

off_t java_lang_Thread::offset_priority;
off_t java_lang_Thread::offset_daemon;
off_t java_lang_Thread::offset_group;
off_t java_lang_Thread::offset_uncaughtExceptionHandler;
off_t java_lang_Thread::offset_threadStatus;
#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
off_t java_lang_Thread::offset_me;
#endif

static DynOffsetEntry dyn_entries_java_lang_Thread[] = {
	{ &java_lang_Thread::set_priority_offset,                 "priority" },
	{ &java_lang_Thread::set_daemon_offset,                   "daemon" },
	{ &java_lang_Thread::set_group_offset,                    "group" },
	{ &java_lang_Thread::set_uncaughtExceptionHandler_offset, "uncaughtExceptionHandler" },
	{ &java_lang_Thread::set_threadStatus_offset,             "threadStatus" },
#ifndef WITH_JAVA_RUNTIME_LIBRARY_OPENJDK_7
	{ &java_lang_Thread::set_me_offset,                       "me" },
#endif
	{ 0, 0 }
};

#endif

void jobjects_register_dyn_offsets()
{
	register_dyn_entry_table(class_java_lang_Thread, dyn_entries_java_lang_Thread);
}

#endif // ENABLE_JAVASE


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
