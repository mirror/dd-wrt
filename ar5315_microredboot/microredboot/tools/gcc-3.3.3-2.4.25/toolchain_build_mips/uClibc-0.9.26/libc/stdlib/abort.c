/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/* Hacked up for uClibc by Erik Andersen */

#define _GNU_SOURCE
#include <features.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>


/* Our last ditch effort to commit suicide */ 
#if defined(__i386__)
#define ABORT_INSTRUCTION asm ("hlt")
#elif defined(__ia64__)
#define ABORT_INSTRUCTION asm ("break 0")
#elif defined(__mc68000__)
#define ABORT_INSTRUCTION asm (".long 0xffffffff")
#elif defined(__mips__)
#define ABORT_INSTRUCTION asm ("break 255")
#elif defined(__s390__)
#define ABORT_INSTRUCTION asm (".word 0")
#elif defined(__sparc__)
#define ABORT_INSTRUCTION asm ("unimp 0xf00")
#elif defined(__x86_64__)
#define ABORT_INSTRUCTION asm ("hlt")
#else
#define ABORT_INSTRUCTION
#endif

extern void weak_function _stdio_term(void);
extern void _exit __P((int __status)) __attribute__ ((__noreturn__));
static int been_there_done_that = 0;

/* Be prepared in case multiple threads try to abort().  */
#ifdef __UCLIBC_HAS_THREADS__
#include <pthread.h>
static pthread_mutex_t mylock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
# define LOCK	__pthread_mutex_lock(&mylock)
# define UNLOCK	__pthread_mutex_unlock(&mylock);
#else
# define LOCK
# define UNLOCK
#endif


/* Cause an abnormal program termination with core-dump.  */
void abort(void)
{
    sigset_t sigset;

      /* Make sure we acquire the lock before proceeding.  */
      LOCK;

    /* Unmask SIGABRT to be sure we can get it */
    if (__sigemptyset(&sigset) == 0 && __sigaddset(&sigset, SIGABRT) == 0) {
	sigprocmask(SIG_UNBLOCK, &sigset, (sigset_t *) NULL);
    }

    /* If we are using stdio, try to shut it down.  At the very least,
	 * this will attempt to commit all buffered writes.  It may also
	 * unbuffer all writable files, or close them outright.
	 * Check the stdio routines for details. */
    if (_stdio_term)
		_stdio_term();

    while (1) {
	/* Try to suicide with a SIGABRT.  */
	if (been_there_done_that == 0) {
	    been_there_done_that++;
	    UNLOCK;
	    raise(SIGABRT);
	    LOCK;
	}

	/* Still here?  Try to remove any signal handlers.  */
	if (been_there_done_that == 1) {
	    struct sigaction act;

	    been_there_done_that++;
	    memset (&act, '\0', sizeof (struct sigaction));
	    act.sa_handler = SIG_DFL;
	    __sigfillset (&act.sa_mask);
	    act.sa_flags = 0;
	    sigaction (SIGABRT, &act, NULL);
	}

	/* Still here?  Try to suicide with an illegal instruction */
	if (been_there_done_that == 2) {
	    been_there_done_that++;
	    ABORT_INSTRUCTION;
	}

	/* Still here?  Try to at least exit */
	if (been_there_done_that == 3) {
	    been_there_done_that++;
	    _exit (127);
	}

	/* Still here?  We're screwed.  Sleepy time.  Good night */
	while (1)
	    /* Try for ever and ever.  */
	    ABORT_INSTRUCTION;
    }
}

