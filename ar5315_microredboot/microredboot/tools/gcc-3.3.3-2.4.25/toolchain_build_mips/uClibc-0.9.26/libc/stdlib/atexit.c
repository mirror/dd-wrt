/* Copyright (C) 1995,1996 Robert de Bath <rdebath@cix.compulink.co.uk>
 * This file is part of the Linux-8086 C library and is distributed
 * under the GNU Library General Public License.
 */

/*
 * Dec 2000          Manuel Novoa III
 *
 *   Made atexit handling conform to standards... i.e. no args.
 *   Removed on_exit since it did not match gnu libc definition.
 *   Combined atexit and __do_exit into one object file.
 *
 * Feb 2001          Manuel Novoa III
 *
 *   Reworked file after addition of __uClibc_main.
 *   Changed name of __do_exit to atexit_handler.
 *   Changed name of __cleanup to __uClibc_cleanup.
 *   Moved declaration of __uClibc_cleanup to __uClibc_main
 *      where it is initialized with (possibly weak alias)
 *      _stdio_term.
 *
 * Jul 2001          Steve Thayer
 * 
 *   Added an on_exit implementation (that now matches gnu libc definition.)
 *   Pulled atexit_handler out of the atexit object since it is now required by
 *   on_exit as well.  Renamed it to __exit_handler.
 *   Fixed a problem where exit functions stop getting called if one of
 *   them calls exit().
 *   As a side effect of these changes, abort() no longer calls the exit
 *   functions (it now matches the gnu libc definition).
 *
 * August 2002    Erik Andersen
 *   Added locking so atexit and friends can be thread safe
 *
 */

#define _GNU_SOURCE
#include <features.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>


#ifdef __UCLIBC_HAS_THREADS__
#include <pthread.h>
extern pthread_mutex_t mylock;
# define LOCK	__pthread_mutex_lock(&mylock)
# define UNLOCK	__pthread_mutex_unlock(&mylock);
#else
# define LOCK
# define UNLOCK
#endif


typedef void (*aefuncp) (void);         /* atexit function pointer */
typedef void (*oefuncp) (int, void *);  /* on_exit function pointer */
typedef enum {
	ef_atexit,
	ef_on_exit
} ef_type; /* exit function types */

/* this is in the L_exit object */
extern void (*__exit_cleanup) (int);

/* these are in the L___do_exit object */
extern int __exit_slots;
extern int __exit_count;
extern void __exit_handler(int);
struct exit_function {
	ef_type type;	/* ef_atexit or ef_on_exit */
	union {
		aefuncp atexit;
		struct {
			oefuncp func;
			void *arg;
		} on_exit;
	} funcs;
};
#ifdef __UCLIBC_DYNAMIC_ATEXIT__
extern struct exit_function *__exit_function_table;
#else
extern struct exit_function __exit_function_table[__UCLIBC_MAX_ATEXIT];
#endif

#ifdef L_atexit
	/*
 * register a function to be called at normal program termination
 * (the registered function takes no arguments)
	 */
int atexit(aefuncp func)
{
    struct exit_function *efp;

    LOCK;
    if (func) {
#ifdef __UCLIBC_DYNAMIC_ATEXIT__
	/* If we are out of function table slots, make some more */
	if (__exit_slots < __exit_count+1) {
	    efp=realloc(__exit_function_table, 
					(__exit_slots+20)*sizeof(struct exit_function));
	    if (efp==NULL) {
		UNLOCK;
		__set_errno(ENOMEM);
		return -1;
	    }
		__exit_function_table = efp;
	    __exit_slots+=20;
	}
#else
	if (__exit_count >= __UCLIBC_MAX_ATEXIT) {
	    UNLOCK;
	    __set_errno(ENOMEM);
	    return -1;
	}
#endif
	__exit_cleanup = __exit_handler; /* enable cleanup */
	efp = &__exit_function_table[__exit_count++];
	efp->type = ef_atexit;
	efp->funcs.atexit = func;
    }
    UNLOCK;
    return 0;
}
#endif

#ifdef L_on_exit
/*
 * register a function to be called at normal program termination
 * the registered function takes two arguments:
 *     status - the exit status that was passed to the exit() function
 *     arg - generic argument
 */
int on_exit(oefuncp func, void *arg)
{
    struct exit_function *efp;

    LOCK;
    if (func) {
#ifdef __UCLIBC_DYNAMIC_ATEXIT__
	/* If we are out of function table slots, make some more */
	if (__exit_slots < __exit_count+1) {
	    efp=realloc(__exit_function_table, 
					(__exit_slots+20)*sizeof(struct exit_function));
	    if (efp==NULL) {
		UNLOCK;
		__set_errno(ENOMEM);
		return -1;
	    }
		__exit_function_table=efp;
	    __exit_slots+=20;
	}
#else
	if (__exit_count >= __UCLIBC_MAX_ATEXIT) {
	    UNLOCK;
	    __set_errno(ENOMEM);
	    return -1;
	}
#endif

	__exit_cleanup = __exit_handler; /* enable cleanup */
	efp = &__exit_function_table[__exit_count++];
	efp->type = ef_on_exit;
	efp->funcs.on_exit.func = func;
	efp->funcs.on_exit.arg = arg;
    }
    UNLOCK;
    return 0;
}
#endif

#ifdef L___exit_handler
int __exit_count = 0; /* Number of registered exit functions */
#ifdef __UCLIBC_DYNAMIC_ATEXIT__
struct exit_function *__exit_function_table = NULL;
int __exit_slots = 0; /* Size of __exit_function_table */
#else
struct exit_function __exit_function_table[__UCLIBC_MAX_ATEXIT];
#endif


/*
 * Handle the work of executing the registered exit functions
 * This is called while we are locked, so no additional locking
 * is needed...
 */
void __exit_handler(int status)
{
	struct exit_function *efp;

	/* In reverse order */
	while ( __exit_count ) {
		efp = &__exit_function_table[--__exit_count];
		switch (efp->type) {
		case ef_on_exit:
			if (efp->funcs.on_exit.func) {
				(efp->funcs.on_exit.func) (status, efp->funcs.on_exit.arg);
			}
			break;
		case ef_atexit:
			if (efp->funcs.atexit) {
				(efp->funcs.atexit) ();
			}
			break;
		}
	}
#ifdef __UCLIBC_DYNAMIC_ATEXIT__
	/* Free up memory used by the __exit_function_table structure */ 
	if (__exit_function_table)
	    free(__exit_function_table);
#endif
}
#endif

#ifdef L_exit
extern void weak_function _stdio_term(void);
void (*__exit_cleanup) (int) = 0;
#ifdef __UCLIBC_HAS_THREADS__
pthread_mutex_t mylock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#endif

#ifdef __UCLIBC_CTOR_DTOR__
extern void (*__app_fini)(void);
#endif

/*
 * Normal program termination
 */
void exit(int rv)
{
	/* Perform exit-specific cleanup (atexit and on_exit) */
	LOCK;
	if (__exit_cleanup) {
		__exit_cleanup(rv);
	}
	UNLOCK;

#ifdef __UCLIBC_CTOR_DTOR__
	if (__app_fini != NULL)
		(__app_fini)();
#endif

    /* If we are using stdio, try to shut it down.  At the very least,
	 * this will attempt to commit all buffered writes.  It may also
	 * unbuffer all writable files, or close them outright.
	 * Check the stdio routines for details. */
	if (_stdio_term) 
	    _stdio_term();

	_exit(rv);
}
#endif
