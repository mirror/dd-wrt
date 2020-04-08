#ifndef __UCLIBC_COMPAT_H
#define __UCLIBC_COMPAT_H

#define _XOPEN_SOURCE
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <features.h>
#include <errno.h>

#ifndef __UCLIBC_HAS_THREADS__
//#define __UCLIBC_HAS_THREADS__
#endif

#include <sys/poll.h>
#include <pthread.h>
#include <sys/cdefs.h>

#ifdef __UCLIBC__
#include <bits/libc-lock.h>
#else
#undef __UCLIBC_MUTEX_STATIC
#undef __UCLIBC_MUTEX_LOCK
#undef __UCLIBC_MUTEX_UNLOCK
#define __UCLIBC_MUTEX_STATIC(M,I) static pthread_mutex_t M = I
#define __UCLIBC_MUTEX_LOCK(M) pthread_mutex_lock(&(M))
#define __UCLIBC_MUTEX_UNLOCK(M) pthread_mutex_unlock(&(M))
#endif

#define smallint int

#define _(...) __VA_ARGS__
#define internal_function
#define attribute_hidden
#define attribute_unused
#define attribute_noreturn
#define libc_hidden_def(...)

#ifndef libc_hidden_proto
#define libc_hidden_proto(name, attrs...)
#endif

#define __set_errno(_val) errno = _val

# define attribute_tls_model_ie __attribute__ ((tls_model ("initial-exec")))

# define __libc_tsd_define(CLASS, KEY)  \
  CLASS __thread void *__libc_tsd_##KEY attribute_tls_model_ie;

# define __libc_tsd_address(KEY)    (&__libc_tsd_##KEY)
# define __libc_tsd_get(KEY)        (__libc_tsd_##KEY)
# define __libc_tsd_set(KEY, VALUE) (__libc_tsd_##KEY = (VALUE))


#endif
