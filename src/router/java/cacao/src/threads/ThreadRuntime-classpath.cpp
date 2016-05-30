/* src/threads/ThreadRuntime-classpath.cpp - thread functions specific to the GNU classpath library

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

#include "threads/ThreadRuntime.hpp"
#include "mm/gc.hpp"
#include "threads/threadlist.hpp"
#include "vm/globals.hpp"
#include "vm/global.hpp"
#include "vm/javaobjects.hpp"
#include "vm/exceptions.hpp"
#include "vm/vm.hpp"

using namespace cacao;

classinfo *ThreadRuntime::get_thread_class_from_object(java_handle_t *object) {
	return class_java_lang_VMThread;
}

java_handle_t *ThreadRuntime::get_vmthread_handle(const java_lang_Thread &jlt) {
	java_lang_VMThread jlvmt(jlt.get_vmThread());
	return jlvmt.get_handle();
}

java_handle_t *ThreadRuntime::get_thread_exception_handler(const java_lang_Thread &jlt)
{
	return jlt.get_exceptionHandler();
}

methodinfo *ThreadRuntime::get_threadgroup_remove_method(classinfo *c)
{
	return class_resolveclassmethod(c,
									utf8::removeThread,
									utf8::java_lang_Thread__V,
									class_java_lang_ThreadGroup,
									true);
}

methodinfo *ThreadRuntime::get_thread_init_method()
{
	return class_resolveclassmethod(class_java_lang_Thread,
									utf8::init,
									Utf8String::from_utf8("(Ljava/lang/VMThread;Ljava/lang/String;IZ)V"),
									class_java_lang_Thread,
									true);
}

void ThreadRuntime::setup_thread_vmdata(const java_lang_Thread& jlt, threadobject *t)
{
	/* Get the java.lang.VMThread object and do some sanity checks. */
	java_lang_VMThread jlvmt(jlt.get_vmThread());

	assert(jlvmt.get_handle() != NULL);
	assert(jlvmt.get_vmdata() == NULL);

	jlvmt.set_vmdata(t);
}

void ThreadRuntime::print_thread_name(const java_lang_Thread& jlt, FILE *stream)
{
	java_handle_t* name = jlt.get_name();
	JavaString(name).fprint(stream);
}

void ThreadRuntime::set_javathread_state(threadobject *t, int state)
{
}

threadobject *ThreadRuntime::get_thread_from_object(java_handle_t *h)
{
	java_lang_VMThread jlvmt(h);
	return jlvmt.get_vmdata();
}

void ThreadRuntime::thread_create_initial_threadgroups(java_handle_t **threadgroup_system, java_handle_t **threadgroup_main)
{
	/* Allocate and initialize the main thread group. */

	*threadgroup_main = native_new_and_init(class_java_lang_ThreadGroup);

	if (*threadgroup_main == NULL)
		vm_abort("thread_create_initial_threadgroups: failed to allocate main threadgroup");

	/* Use the same threadgroup for system as for main. */

	*threadgroup_system = *threadgroup_main;
}

bool ThreadRuntime::invoke_thread_initializer(java_lang_Thread& jlt, threadobject *t, methodinfo *thread_method_init, java_handle_t *name, java_handle_t *group)
{
	java_handle_t *h = builtin_new(class_java_lang_VMThread);

	if (h == NULL)
		return false;

	// Create and initialize a java.lang.VMThread object.
	java_lang_VMThread jlvmt(h, jlt.get_handle(), t);

	/* Call:
	   java.lang.Thread.<init>(Ljava/lang/VMThread;Ljava/lang/String;IZ)V */

	bool isdaemon = thread_is_daemon(t);

	(void) vm_call_method(thread_method_init, jlt.get_handle(), jlvmt.get_handle(),
						  name, NORM_PRIORITY, isdaemon);

	if (exceptions_get_exception())
		return false;

	// Set the ThreadGroup in the Java thread object.
	jlt.set_group(group);

	/* Add thread to the threadgroup. */

	classinfo* c;
	LLNI_class_get(group, c);

	methodinfo* m = class_resolveclassmethod(c,
											 utf8::addThread,
											 utf8::java_lang_Thread__V,
											 class_java_lang_ThreadGroup,
											 true);

	if (m == NULL)
		return false;

	(void) vm_call_method(m, group, jlt.get_handle());

	if (exceptions_get_exception())
		return false;

	return true;
}

void ThreadRuntime::clear_heap_reference(java_lang_Thread& jlt)
{
	// Nothing to do.
}

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
