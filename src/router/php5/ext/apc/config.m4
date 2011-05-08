dnl
dnl $Id: config.m4 304101 2010-10-05 14:37:36Z kalle $
dnl

PHP_ARG_ENABLE(apc, whether to enable APC support,
[  --enable-apc           Enable APC support])

AC_ARG_ENABLE(apc-debug,
[  --enable-apc-debug     Enable APC debugging], 
[
  PHP_APC_DEBUG=yes
], 
[
  PHP_APC_DEBUG=no
])

AC_MSG_CHECKING(whether we should enable cache request file info)
AC_ARG_ENABLE(apc-filehits,
[  --enable-apc-filehits   Enable per request file info about files used from the APC cache (ie: apc_cache_info('filehits')) ],
[
  PHP_APC_FILEHITS=$enableval
	AC_MSG_RESULT($enableval)
], 
[
  PHP_APC_FILEHITS=no
	AC_MSG_RESULT(no)
])

AC_MSG_CHECKING(whether we should use mmap)
AC_ARG_ENABLE(apc-mmap,
[  --disable-apc-mmap
                          Disable mmap support and use IPC shm instead],
[
  PHP_APC_MMAP=$enableval
  AC_MSG_RESULT($enableval)
], [
  PHP_APC_MMAP=yes
  AC_MSG_RESULT(yes)
])

AC_MSG_CHECKING(whether we should use semaphore locking instead of fcntl)
AC_ARG_ENABLE(apc-sem,
[  --enable-apc-sem
                          Enable semaphore locks instead of fcntl],
[
  PHP_APC_SEM=$enableval
  AC_MSG_RESULT($enableval)
], [
  PHP_APC_SEM=no
  AC_MSG_RESULT(no)
])

AC_MSG_CHECKING(whether we should use pthread mutex locking)
AC_ARG_ENABLE(apc-pthreadmutex,
[  --disable-apc-pthreadmutex
                          Disable pthread mutex locking ],
[
  PHP_APC_PTHREADMUTEX=$enableval
  AC_MSG_RESULT($enableval)
],
[
  PHP_APC_PTHREADMUTEX=yes
  AC_MSG_RESULT(yes)
])

if test "$PHP_APC_PTHREADMUTEX" != "no"; then
	orig_LIBS="$LIBS"
	LIBS="$LIBS -lpthread"
	AC_TRY_RUN(
			[
				#include <sys/types.h>
				#include <pthread.h>
                                main() {
				pthread_mutex_t mutex;
				pthread_mutexattr_t attr;	

				if(pthread_mutexattr_init(&attr)) { 
					puts("Unable to initialize pthread attributes (pthread_mutexattr_init).");
					return -1; 
				}
				if(pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED)) { 
					puts("Unable to set PTHREAD_PROCESS_SHARED (pthread_mutexattr_setpshared), your system may not support shared mutex's.");
					return -1; 
				}	
				if(pthread_mutex_init(&mutex, &attr)) { 
					puts("Unable to initialize the mutex (pthread_mutex_init).");
					return -1; 
				}
				if(pthread_mutexattr_destroy(&attr)) { 
					puts("Unable to destroy mutex attributes (pthread_mutexattr_destroy).");
					return -1; 
				}
				if(pthread_mutex_destroy(&mutex)) { 
					puts("Unable to destroy mutex (pthread_mutex_destroy).");
					return -1; 
				}

				puts("pthread mutex's are supported!");
				return 0;
                                }
			],
			[ dnl -Success-
				PHP_ADD_LIBRARY(pthread)
			],
			[ dnl -Failure-
				AC_MSG_WARN([It doesn't appear that pthread mutex's are supported on your system])
  			PHP_APC_PTHREADMUTEX=no
			],
			[
				PHP_ADD_LIBRARY(pthread)
			]
	)
	LIBS="$orig_LIBS"
fi

AC_MSG_CHECKING(whether we should use spin locks)
AC_ARG_ENABLE(apc-spinlocks,
[  --enable-apc-spinlocks
                          Enable spin locks  EXPERIMENTAL ],
[
  PHP_APC_SPINLOCKS=$enableval
  AC_MSG_RESULT($enableval)
],
[
  PHP_APC_SPINLOCKS=no
  AC_MSG_RESULT(no)
])

AC_MSG_CHECKING(whether we should enable memory protection)
AC_ARG_ENABLE(apc-memprotect,
[  --enable-apc-memprotect
                          Enable mmap/shm memory protection],
[
  PHP_APC_MEMPROTECT=$enableval
  AC_MSG_RESULT($enableval)
], [
  PHP_APC_MEMPROTECT=no
  AC_MSG_RESULT(no)
])

if test "$PHP_APC" != "no"; then
  test "$PHP_APC_MMAP" != "no" && AC_DEFINE(APC_MMAP, 1, [ ])
  test "$PHP_APC_FILEHITS" != "no" && AC_DEFINE(APC_FILEHITS, 1, [ ])

	if test "$PHP_APC_DEBUG" != "no"; then
		AC_DEFINE(__DEBUG_APC__, 1, [ ])
	fi

	if test "$PHP_APC_SEM" != "no"; then
		AC_DEFINE(APC_SEM_LOCKS, 1, [ ])
	elif test "$PHP_APC_SPINLOCKS" != "no"; then
		AC_DEFINE(APC_SPIN_LOCKS, 1, [ ]) 
	elif test "$PHP_APC_PTHREADMUTEX" != "no"; then 
		AC_DEFINE(APC_PTHREADMUTEX_LOCKS, 1, [ ])
	else 
		AC_DEFINE(APC_FCNTL_LOCKS, 1, [ ])
	fi
  
	if test "$PHP_APC_MEMPROTECT" != "no"; then
		AC_DEFINE(APC_MEMPROTECT, 1, [ shm/mmap memory protection ])
	fi

  AC_CACHE_CHECK(for zend_set_lookup_function_hook, php_cv_zend_set_lookup_function_hook,
  [
    orig_cflags=$CFLAGS
    CFLAGS="$INCLUDES $EXTRA_INCLUDES"
    AC_TRY_COMPILE([
#include "main/php.h"
#include "Zend/zend_API.h"
    ], [#ifndef zend_set_lookup_function_hook
	(void) zend_set_lookup_function_hook;
#endif], [
      php_cv_zend_set_lookup_function_hook=yes
    ],[
      php_cv_zend_set_lookup_function_hook=no
    ])
    CFLAGS=$orig_cflags
  ])
  if test "$php_cv_zend_set_lookup_function_hook" = "yes"; then
    AC_DEFINE(APC_HAVE_LOOKUP_HOOKS, 1, [ ])
  else
    AC_DEFINE(APC_HAVE_LOOKUP_HOOKS, 0, [ ])
  fi

  AC_CHECK_FUNCS(sigaction)
  AC_CACHE_CHECK(for union semun, php_cv_semun,
  [
    AC_TRY_COMPILE([
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
    ], [union semun x; x.val=1], [
      php_cv_semun=yes
    ],[
      php_cv_semun=no
    ])
  ])
  if test "$php_cv_semun" = "yes"; then
    AC_DEFINE(HAVE_SEMUN, 1, [ ])
  else
    AC_DEFINE(HAVE_SEMUN, 0, [ ])
  fi

  AC_MSG_CHECKING(whether we should enable valgrind support)
  AC_ARG_ENABLE(valgrind-checks,
  [  --disable-valgrind-checks
                          Disable valgrind based memory checks],
  [
    PHP_APC_VALGRIND=$enableval
    AC_MSG_RESULT($enableval)
  ], [
    PHP_APC_VALGRIND=yes
    AC_MSG_RESULT(yes)
    AC_CHECK_HEADER(valgrind/memcheck.h, 
  		[AC_DEFINE([HAVE_VALGRIND_MEMCHECK_H],1, [enable valgrind memchecks])])
  ])

  apc_sources="apc.c php_apc.c \
               apc_cache.c \
               apc_compile.c \
               apc_debug.c \
               apc_fcntl.c \
               apc_main.c \
               apc_mmap.c \
               apc_sem.c \
               apc_shm.c \
               apc_pthreadmutex.c \
               apc_spin.c \
               pgsql_s_lock.c \
               apc_sma.c \
               apc_stack.c \
               apc_zend.c \
               apc_rfc1867.c \
               apc_signal.c \
               apc_pool.c \
               apc_iterator.c \
               apc_bin.c \
               apc_string.c "

  PHP_CHECK_LIBRARY(rt, shm_open, [PHP_ADD_LIBRARY(rt,,APC_SHARED_LIBADD)])
  PHP_NEW_EXTENSION(apc, $apc_sources, $ext_shared,, \\$(APC_CFLAGS))
  PHP_SUBST(APC_SHARED_LIBADD)
  PHP_SUBST(APC_CFLAGS)
  AC_DEFINE(HAVE_APC, 1, [ ])
fi

