#ifndef __UCLIBC_COMPAT_H
#define __UCLIBC_COMPAT_H

#define _XOPEN_SOURCE
#define _GNU_SOURCE

#include <features.h>
#include <errno.h>

#undef __UCLIBC_HAS_THREADS__
#include <bits/uClibc_mutex.h>
#include <sys/poll.h>

#if 0
#undef __UCLIBC_MUTEX_LOCK
#undef __UCLIBC_MUTEX_UNLOCK
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

#define __set_errno(_val) errno = _val

#endif
