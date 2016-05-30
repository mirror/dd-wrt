/* src/vm/signal.cpp - machine independent signal functions

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

#include <errno.h>                      // for EINTR
#include <signal.h>                     // for sigaction, sigemptyset, etc
#include <cstdlib>                      // for NULL
#include "arch.hpp"
#include "class.hpp"                    // for class_resolvemethod
#include "config.h"
#include "global.hpp"                   // for functionptr
#include "mm/gc.hpp"                    // for heap_alloc
#include "mm/memory.hpp"                // for GCNEW
#include "threads/thread.hpp"           // for thread_set_state_runnable, etc
#include "threads/threadlist.hpp"       // for ThreadList
#include "toolbox/logging.hpp"          // for log_println
#include "utf8.hpp"                     // for Utf8String
#include "vm/exceptions.hpp"            // for exceptions_get_exception, etc
#include "vm/globals.hpp"               // for class_sun_misc_Signal
#include "vm/options.hpp"
#include "vm/os.hpp"                    // for os
#include "vm/signallocal.hpp"           // for md_signal_handler_sigsegv, etc
#include "vm/vm.hpp"                    // for vm_abort, vm_call_method, etc

#ifndef __SIGRTMIN /* if arch has not defined it in bits/signum.h... */
# define __SIGRTMIN 32
#endif
#ifndef __SIGRTMAX
#define __SIGRTMAX (_NSIG - 1)
#endif
struct methodinfo;

/* function prototypes ********************************************************/

void signal_handler_sighup(int sig, siginfo_t *siginfo, void *_p);
void signal_handler_sigusr1(int sig, siginfo_t *siginfo, void *_p);


/* signal_init *****************************************************************

   Initializes the signal subsystem and installs the signal handler.

*******************************************************************************/

bool signal_init(void)
{
#if !defined(__CYGWIN__)
	sigset_t mask;

	TRACESUBSYSTEMINITIALIZATION("signal_init");

#if defined(__LINUX__)
	/* XXX Remove for exact-GC. */
	if (threads_pthreads_implementation_nptl) {
#endif

	/* Block the following signals (SIGINT for <ctrl>-c, SIGQUIT for
	   <ctrl>-\).  We enable them later in signal_thread, but only for
	   this thread. */

	if (sigemptyset(&mask) != 0)
		os::abort_errno("signal_init: sigemptyset failed");

#if !defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)
	/* Let OpenJDK handle SIGINT itself. */

	if (sigaddset(&mask, SIGINT) != 0)
		os::abort_errno("signal_init: sigaddset failed");
#endif

#if !defined(__FREEBSD__)
	if (sigaddset(&mask, SIGQUIT) != 0)
		os::abort_errno("signal_init: sigaddset failed");
#endif

	if (sigprocmask(SIG_BLOCK, &mask, NULL) != 0)
		os::abort_errno("signal_init: sigprocmask failed");

#if defined(__LINUX__)
	/* XXX Remove for exact-GC. */
	}
#endif

#if defined(ENABLE_GC_BOEHM)
	/* Allocate something so the garbage collector's signal handlers
	   are installed. */

	(void) GCNEW(int);
#endif

	/* Install signal handlers for signals we want to catch in all
	   threads. */

#if defined(ENABLE_JIT)
# if defined(ENABLE_INTRP)
	if (!opt_intrp) {
# endif
		/* SIGSEGV handler */

		signal_register_signal(SIGSEGV, (functionptr) md_signal_handler_sigsegv,
							   SA_NODEFER | SA_SIGINFO);

#  if defined(SIGBUS)
		signal_register_signal(SIGBUS, (functionptr) md_signal_handler_sigsegv,
							   SA_NODEFER | SA_SIGINFO);
#  endif

#  if SUPPORT_HARDWARE_DIVIDE_BY_ZERO
		/* SIGFPE handler */

		signal_register_signal(SIGFPE, (functionptr) md_signal_handler_sigfpe,
							   SA_NODEFER | SA_SIGINFO);
#  endif

#  if defined(__ALPHA__) || defined(__ARM__) || defined(__I386__) || defined(__MIPS__) || defined(__POWERPC__) || defined(__POWERPC64__) || defined(__S390__) || defined(__X86_64__)
		/* XXX use better defines for that (in arch.h) */
		/* SIGILL handler */

		signal_register_signal(SIGILL, (functionptr) md_signal_handler_sigill,
							   SA_NODEFER | SA_SIGINFO);
#  endif

#  if defined(__POWERPC__)
		/* XXX use better defines for that (in arch.h) */
		/* SIGTRAP handler */

		signal_register_signal(SIGTRAP, (functionptr) md_signal_handler_sigtrap,
							   SA_NODEFER | SA_SIGINFO);
#  endif
# if defined(ENABLE_INTRP)
	}
# endif

#if defined(__DARWIN__)
	do {
		struct utsname name;
		kern_return_t kr;

		/* Check if we're on 10.4 (Tiger/8.x) or earlier */
		if (uname(&name) != 0) 
			break;

		/* Make sure the string is large enough */
		/* Check the major number (ascii comparison) */
		/* Verify that we're not looking at '10.' by checking for a trailing period. */
		if (name.release[0] == '\0' || name.release[0] > '8' || name.release[1] != '.')
			break;

		/* Reset CrashReporter's task signal handler */
		kr = task_set_exception_ports(mach_task_self(),
									  EXC_MASK_BAD_ACCESS
#  if defined(__I386__) || defined(__X86_64__)
									  | EXC_MASK_BAD_INSTRUCTION
#endif
									  , MACH_PORT_NULL,
									  EXCEPTION_STATE_IDENTITY,
									  MACHINE_THREAD_STATE);

		assert(kr == KERN_SUCCESS);
	} while (false);
#endif
#endif /* !defined(ENABLE_JIT) */

	/* SIGHUP handler for threads_thread_interrupt */

	signal_register_signal(Signal_INTERRUPT_SYSTEM_CALL, (functionptr) signal_handler_sighup, 0);

	/* SIGUSR1 handler for thread suspension */

	signal_register_signal(SIGUSR1, (functionptr) signal_handler_sigusr1, SA_SIGINFO);

#ifdef ENABLE_PROFILING
	/* SIGUSR2 handler for profiling sampling */

	signal_register_signal(SIGUSR2, (functionptr) md_signal_handler_sigusr2, SA_SIGINFO);
#endif

#endif /* !defined(__CYGWIN__) */

	return true;
}


/* signal_register_signal ******************************************************

   Register the specified handler with the specified signal.

*******************************************************************************/

void signal_register_signal(int signum, functionptr handler, int flags)
{
	struct sigaction act;

	void (*function)(int, siginfo_t *, void *);

	function = (void (*)(int, siginfo_t *, void *)) handler;

	if (sigemptyset(&act.sa_mask) != 0)
		os::abort_errno("signal_register_signal: sigemptyset failed");

	act.sa_sigaction = function;
	act.sa_flags     = flags;

	if (sigaction(signum, &act, NULL) != 0)
		os::abort_errno("signal_register_signal: sigaction failed");
}


/* signal_thread ************************************************************

   This thread sets the signal mask to catch the user input signals
   (SIGINT, SIGQUIT).  We use such a thread, so we don't get the
   signals on every single thread running.

*******************************************************************************/

static void signal_thread(void)
{
	threadobject *t;
	sigset_t      mask;
	int           sig;
	int result;

	t = THREADOBJECT;

	if (sigemptyset(&mask) != 0)
		os::abort_errno("signal_thread: sigemptyset failed");

#if !defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)
	/* Let OpenJDK handle SIGINT itself. */

	if (sigaddset(&mask, SIGINT) != 0)
		os::abort_errno("signal_thread: sigaddset failed");
#endif

#if !defined(__FREEBSD__)
	if (sigaddset(&mask, SIGQUIT) != 0)
		os::abort_errno("signal_thread: sigaddset failed");
#endif

	for (;;) {
		/* just wait for a signal */

		thread_set_state_waiting(t);

		// sigwait can return EINTR (unlike what the Linux man-page
		// says).
		do {
			result = sigwait(&mask, &sig);
		} while (result == EINTR);

		if (result != 0)
			os::abort_errnum(result, "signal_thread: sigwait failed");

		thread_set_state_runnable(t);

		/* Handle the signal. */

		signal_thread_handler(sig);
	}
}


/* signal_thread_handler *******************************************************

   Handles the signals caught in the signal handler thread.  Also used
   from sun.misc.Signal with OpenJDK.

*******************************************************************************/

void signal_thread_handler(int sig)
{
	switch (sig) {
	case SIGINT:
		/* exit the vm properly */

		vm_exit(1);
		break;

	case SIGQUIT:
		/* print a thread dump */
		ThreadList::get()->dump_threads();

#if 0 && defined(ENABLE_STATISTICS)
		if (opt_stat)
			statistics_print_memory_usage();
#endif
		break;

#if defined(WITH_JAVA_RUNTIME_LIBRARY_OPENJDK)
	default: {
		// For OpenJDK we dispatch all unknown signals to Java.
		methodinfo* m = class_resolvemethod(class_sun_misc_Signal, utf8::dispatch, utf8::int__void);
		(void) vm_call_method(m, NULL, sig);

		if (exceptions_get_exception()) {
			log_println("signal_thread_handler: Java signal handler throw an exception while dispatching signal %d:", sig);
			exceptions_print_stacktrace();
			vm_abort("signal_thread_handler: Aborting...");
		}
		break;
	}
#else
	default:
		vm_abort("signal_thread_handler: Unknown signal %d", sig);
#endif
	}
}


/* signal_start_thread *********************************************************

   Starts the signal handler thread.

*******************************************************************************/

bool signal_start_thread(void)
{
	Utf8String name = Utf8String::from_utf8("Signal Handler");

	if (!threads_thread_start_internal(name, signal_thread))
		return false;

	/* everything's ok */

	return true;
}


/* signal_handler_sighup *******************************************************

   This handler is required by threads_thread_interrupt and does
   nothing.

*******************************************************************************/

void signal_handler_sighup(int sig, siginfo_t *siginfo, void *_p)
{
	/* do nothing */
}


/* signal_handler_sigusr1 ******************************************************

   Signal handler for suspending threads.

*******************************************************************************/

void signal_handler_sigusr1(int sig, siginfo_t *siginfo, void *_p)
{
	// Really suspend ourselves by acknowledging the suspension.
	threads_suspend_ack();
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
