/* src/vm/initialize.cpp - static class initializer functions

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


#include "config.h"

#include <string.h>

#include "vm/types.hpp"

#include "threads/thread.hpp"
#include "threads/atomic.hpp"           // for write_memory_barrier
#include "threads/lock.hpp"

#include "toolbox/logging.hpp"

#include "vm/jit/builtin.hpp"
#include "vm/class.hpp"
#include "vm/exceptions.hpp"
#include "vm/global.hpp"
#include "vm/globals.hpp"
#include "vm/initialize.hpp"
#include "vm/loader.hpp"
#include "vm/options.hpp"
#include "vm/vm.hpp"
#include "vm/statistics.hpp"


STAT_REGISTER_VAR(int,count_class_inits,0,"class inits","Number of class inits")

/* private functions **********************************************************/

static bool initialize_class_intern(classinfo *c);


/* initialize_init *************************************************************

   Initialize important system classes.

*******************************************************************************/

void initialize_init(void)
{
	TRACESUBSYSTEMINITIALIZATION("initialize_init");

#if defined(ENABLE_JAVASE)
# if defined(WITH_JAVA_RUNTIME_LIBRARY_GNU_CLASSPATH)

	/* Nothing. */

# elif defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)

	if (!initialize_class(class_java_lang_String))
		vm_abort("initialize_init: Initialization failed: java.lang.String");

	if (!initialize_class(class_java_lang_System))
		vm_abort("initialize_init: Initialization failed: java.lang.System");

	if (!initialize_class(class_java_lang_ThreadGroup))
		vm_abort("initialize_init: Initialization failed: java.lang.ThreadGroup");

	if (!initialize_class(class_java_lang_Thread))
		vm_abort("initialize_init: Initialization failed: java.lang.Thread");

	if (!initialize_class(class_java_lang_Class))
		vm_abort("initialize_init: Initialization failed: java.lang.Class");

# else
#  error unknown classpath configuration
# endif

#elif defined(ENABLE_JAVAME_CLDC1_1)

	/* Nothing. */

#else
# error unknown Java configuration
#endif
}

/* initialize_class ************************************************************

   In Java, every class can have a static initialization
   function. This function has to be called BEFORE calling other
   methods or accessing static variables.

*******************************************************************************/

bool initialize_class(classinfo *c)
{
	bool r;

	if (!makeinitializations)
		return true;

	LOCK_MONITOR_ENTER(c);

	/* maybe the class is already initalized or the current thread, which can
	   pass the monitor, is currently initalizing this class */

	if (class_is_or_almost_initialized(c)) {
		LOCK_MONITOR_EXIT(c);

		return true;
	}

	/* if <clinit> throw an Error before, the class was marked with an
       error and we have to throw a NoClassDefFoundError */

	if (c->state & CLASS_ERROR) {
		exceptions_throw_noclassdeffounderror(c->name);

		LOCK_MONITOR_EXIT(c);

		/* ...but return true, this is ok (mauve test) */

		return true;
	}

	/* this initalizing run begins NOW */

	c->initializing_thread = thread_get_current();
	c->state |= CLASS_INITIALIZING;

	/* call the internal function */

	r = initialize_class_intern(c);

	/* if return value is not NULL everything was ok and the class is
	   initialized */

	if (r) {
        // Let's make sure that everything is flushed out to memory before
        // marking the class as initialized.
        Atomic::write_memory_barrier();

        c->state |= CLASS_INITIALIZED;
    }

	/* this initalizing run is done */

	c->state &= ~CLASS_INITIALIZING;

	LOCK_MONITOR_EXIT(c);

	return r;
}


/* initialize_class_intern *****************************************************

   This function MUST NOT be called directly, because of thread
   <clinit> race conditions.

*******************************************************************************/

static bool initialize_class_intern(classinfo *c)
{
	methodinfo    *m;
	java_handle_t *cause;
	classinfo     *clazz;

	/* maybe the class is not already linked */

	if (!(c->state & CLASS_LINKED))
		if (!link_class(c))
			return false;

	STATISTICS(count_class_inits++);

	/* Initialize super class. */

	if (c->super != NULL) {
		if (!(c->super->state & CLASS_INITIALIZED)) {
#if !defined(NDEBUG)
			if (initverbose)
				log_message_class_message_class("Initialize super class ",
												c->super,
												" from ",
												c);
#endif

			if (!initialize_class(c->super))
				return false;
		}
	}

	/* interfaces implemented need not to be initialized (VM Spec 2.17.4) */

	m = class_findmethod(c, utf8::clinit, utf8::void__void);

	if (m == NULL) {
#if !defined(NDEBUG)
		if (initverbose)
			log_message_class("Class has no static class initializer: ", c);
#endif

		return true;
	}

	/* Sun's and IBM's JVM don't care about the static flag */
/*  	if (!(m->flags & ACC_STATIC)) { */
/*  		log_text("Class initializer is not static!"); */

#if !defined(NDEBUG)
	if (initverbose)
		log_message_class("Starting static class initializer for class: ", c);
#endif

	/* now call the initializer */

	(void) vm_call_method(m, NULL);

	/* we have an exception or error */

	cause = exceptions_get_exception();

	if (cause != NULL) {
		/* class is NOT initialized and is marked with error */

		c->state |= CLASS_ERROR;

		/* Load java/lang/Exception for the instanceof check. */

		clazz = load_class_bootstrap(utf8::java_lang_Exception);

		if (clazz == NULL)
			return false;

		/* Is this an exception?  Yes, than wrap it. */

		if (builtin_instanceof(cause, clazz)) {
			/* clear exception, because we are calling jit code again */

			exceptions_clear_exception();

			/* wrap the exception */

			exceptions_throw_exceptionininitializererror(cause);
		}

		return false;
	}

#if !defined(NDEBUG)
	if (initverbose)
		log_message_class("Finished static class initializer for class: ", c);
#endif

	return true;
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
