/*
  This file is part of libmicrohttpd
  Copyright (C) 2016-2023 Karlson2k (Evgeny Grin)

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*/

/**
 * @file microhttpd/mhd_threads.h
 * @brief  Header for platform-independent threads abstraction
 * @author Karlson2k (Evgeny Grin)
 *
 * Provides basic abstraction for threads.
 * Any functions can be implemented as macro on some platforms
 * unless explicitly marked otherwise.
 * Any function argument can be skipped in macro, so avoid
 * variable modification in function parameters.
 *
 * @warning Unlike pthread functions, most of functions return
 *          nonzero on success.
 */

#ifndef MHD_THREADS_H
#define MHD_THREADS_H 1

#include "mhd_options.h"
#ifdef HAVE_STDDEF_H
#  include <stddef.h> /* for size_t */
#elif defined(HAVE_STDLIB_H)
#  include <stdlib.h> /* for size_t */
#else /* ! HAVE_STDLIB_H */
#  include <stdio.h>  /* for size_t */
#endif /* ! HAVE_STDLIB_H */

#if defined(MHD_USE_POSIX_THREADS)
#  undef HAVE_CONFIG_H
#  include <pthread.h>
#  define HAVE_CONFIG_H 1
#  ifndef MHD_USE_THREADS
#    define MHD_USE_THREADS 1
#  endif
#elif defined(MHD_USE_W32_THREADS)
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN 1
#  endif /* !WIN32_LEAN_AND_MEAN */
#  include <windows.h>
#  ifndef MHD_USE_THREADS
#    define MHD_USE_THREADS 1
#  endif
#else
#  error No threading API is available.
#endif

#ifdef HAVE_STDBOOL_H
#  include <stdbool.h>
#endif /* HAVE_STDBOOL_H */

#if defined(MHD_USE_POSIX_THREADS) && defined(MHD_USE_W32_THREADS)
#  error Both MHD_USE_POSIX_THREADS and MHD_USE_W32_THREADS are defined
#endif /* MHD_USE_POSIX_THREADS && MHD_USE_W32_THREADS */

#ifndef MHD_NO_THREAD_NAMES
#  if defined(MHD_USE_POSIX_THREADS)
#    if defined(HAVE_PTHREAD_SETNAME_NP_GNU) || \
  defined(HAVE_PTHREAD_SET_NAME_NP_FREEBSD) || \
  defined(HAVE_PTHREAD_SETNAME_NP_DARWIN) || \
  defined(HAVE_PTHREAD_SETNAME_NP_NETBSD) || \
  defined(HAVE_PTHREAD_ATTR_SETNAME_NP_NETBSD) || \
  defined(HAVE_PTHREAD_ATTR_SETNAME_NP_IBMI)
#      define MHD_USE_THREAD_NAME_
#    endif /* HAVE_PTHREAD_SETNAME_NP */
#  elif defined(MHD_USE_W32_THREADS)
#    ifdef _MSC_FULL_VER
/* Thread names only available with VC compiler */
#      define MHD_USE_THREAD_NAME_
#    endif /* _MSC_FULL_VER */
#  endif
#endif

/* ** Thread handle - used to control the thread ** */

#if defined(MHD_USE_POSIX_THREADS)
/**
 * Wait until specified thread is ended and free thread handle on success.
 * @param thread handle to watch
 * @return nonzero on success, zero otherwise
 */
#  define MHD_join_thread_(native_handle) \
  (! pthread_join ((native_handle), NULL))
#elif defined(MHD_USE_W32_THREADS)
/**
 * Wait until specified thread is ended and free thread handle on success.
 * @param thread handle to watch
 * @return nonzero on success, zero otherwise
 */
#  define MHD_join_thread_(native_handle) \
  ( (WAIT_OBJECT_0 == WaitForSingleObject ( (native_handle), INFINITE)) ? \
    (CloseHandle ( (native_handle)), ! 0) : 0 )
#endif

#if defined(MHD_USE_POSIX_THREADS)
/**
 * The native type to control the thread from other threads
 */
typedef pthread_t MHD_thread_handle_native_;
#elif defined(MHD_USE_W32_THREADS)
/**
 * The native type to control the thread from other threads
 */
typedef HANDLE MHD_thread_handle_native_;
#endif

#if defined(MHD_USE_POSIX_THREADS)
#  if defined(__gnu_linux__) || \
  (defined(__linux__) && defined(__GLIBC__))
/* The next part of code is disabled because it relies on undocumented
   behaviour.
   It could be enabled for neglectable performance and size improvements. */
#  if 0 /* Disabled code */
/**
 * The native invalid value for native thread handle
 */
#    define MHD_THREAD_HANDLE_NATIVE_VALUE_INVALID_ \
     ((MHD_thread_handle_native_) 0)
#  endif /* Disabled code */
#  endif /* __gnu_linux__ || (__linux__ && __GLIBC__) */
#elif defined(MHD_USE_W32_THREADS)
/**
 * The native invalid value for native thread handle
 */
#  define MHD_THREAD_HANDLE_NATIVE_VALUE_INVALID_ \
  ((MHD_thread_handle_native_) NULL)
#endif /* MHD_USE_W32_THREADS */

#if ! defined(MHD_THREAD_HANDLE_NATIVE_VALUE_INVALID_)
/**
 * Structure with thread handle and validity flag
 */
struct MHD_thread_handle_struct_
{
  bool valid;                       /**< true if native handle is set */
  MHD_thread_handle_native_ native; /**< the native thread handle */
};
/**
 * Type with thread handle that can be set to invalid value
 */
typedef struct MHD_thread_handle_struct_ MHD_thread_handle_;

/**
 * Set variable pointed by @a handle_ptr to invalid (unset) value
 */
#  define MHD_thread_handle_set_invalid_(handle_ptr) \
   ((handle_ptr)->valid = false)
/**
 * Set native handle in variable pointed by @a handle_ptr
 * to @a native_val value
 */
#  define MHD_thread_handle_set_native_(handle_ptr,native_val) \
   ((handle_ptr)->valid = true, (handle_ptr)->native = native_val)
/**
 * Check whether native handle value is set in @a handle_var variable
 */
#  define MHD_thread_handle_is_valid_(handle_var) \
   ((handle_var).valid)
/**
 * Get native handle value from @a handle_var variable
 */
#  define MHD_thread_handle_get_native_(handle_var) \
   ((handle_var).native)
#else  /* MHD_THREAD_HANDLE_NATIVE_INVALID_ */
/**
 * Type with thread handle that can be set to invalid value
 */
typedef MHD_thread_handle_native_ MHD_thread_handle_;

/**
 * Set variable pointed by @a handle_ptr to invalid (unset) value
 */
#  define MHD_thread_handle_set_invalid_(handle_ptr) \
    ((*(handle_ptr)) = MHD_THREAD_HANDLE_NATIVE_VALUE_INVALID_)
/**
 * Set native handle in the variable pointed by @a handle_ptr
 * to @a native_val value
 */
#  define MHD_thread_handle_set_native_(handle_ptr,native_val) \
    ((*(handle_ptr)) = native_val)
/**
 * Check whether native handle value is set in @a handle_var variable
 */
#  define MHD_thread_handle_is_valid_(handle_var) \
    (MHD_THREAD_HANDLE_NATIVE_VALUE_INVALID_ != handle_var)
/**
 * Get native handle value from @a handle_var variable
 */
#  define MHD_thread_handle_get_native_(handle_var) \
    (handle_var)
/**
 * Get pointer to native handle stored the variable pointed by @a handle_ptr
 * @note This macro could not available if direct manipulation of
 *       the native handle is not possible
 */
#  define MHD_thread_handle_get_native_ptr_(handle_ptr) \
    (handle_ptr)
#endif /* MHD_THREAD_HANDLE_NATIVE_INVALID_ */


/* ** Thread ID - used to check threads match ** */

#if defined(MHD_USE_POSIX_THREADS)
/**
 * The native type used to check whether current thread matches expected thread
 */
typedef pthread_t MHD_thread_ID_native_;

/**
 * Function to get the current thread native ID.
 */
#  define MHD_thread_ID_native_current_ pthread_self

/**
 * Check whether two native thread IDs are equal.
 * @return non-zero if equal, zero if not equal
 */
#  define MHD_thread_ID_native_equal_(id1,id2) \
  (pthread_equal(id1,id2))
#elif defined(MHD_USE_W32_THREADS)
/**
 * The native type used to check whether current thread matches expected thread
 */
typedef DWORD MHD_thread_ID_native_;

/**
 * Function to get the current thread native ID.
 */
#  define MHD_thread_ID_native_current_ GetCurrentThreadId

/**
 * Check whether two native thread IDs are equal.
 * @return non-zero if equal, zero if not equal
 */
#  define MHD_thread_ID_native_equal_(id1,id2) \
  ((id1) == (id2))
#endif

/**
 * Check whether specified thread ID matches current thread.
 * @param id the thread ID to match
 * @return nonzero on match, zero otherwise
 */
#define MHD_thread_ID_native_is_current_thread_(id) \
    MHD_thread_ID_native_equal_(id, MHD_thread_ID_native_current_())


#if defined(MHD_USE_POSIX_THREADS)
#  if defined(MHD_THREAD_HANDLE_NATIVE_VALUE_INVALID_)
/**
 * The native invalid value for native thread ID
 */
#    define MHD_THREAD_ID_NATIVE_VALUE_INVALID_ \
            MHD_THREAD_HANDLE_NATIVE_VALUE_INVALID_
#  endif /* MHD_THREAD_HANDLE_NATIVE_VALUE_INVALID_ */
#elif defined(MHD_USE_W32_THREADS)
/**
 * The native invalid value for native thread ID
 */
 #  define MHD_THREAD_ID_NATIVE_VALUE_INVALID_ \
   ((MHD_thread_ID_native_) 0)
#endif /* MHD_USE_W32_THREADS */

#if ! defined(MHD_THREAD_ID_NATIVE_VALUE_INVALID_)
/**
 * Structure with thread id and validity flag
 */
struct MHD_thread_ID_struct_
{
  bool valid;                   /**< true if native ID is set */
  MHD_thread_ID_native_ native; /**< the native thread ID */
};
/**
 * Type with thread ID that can be set to invalid value
 */
typedef struct MHD_thread_ID_struct_ MHD_thread_ID_;

/**
 * Set variable pointed by @a ID_ptr to invalid (unset) value
 */
#  define MHD_thread_ID_set_invalid_(ID_ptr) \
   ((ID_ptr)->valid = false)
/**
 * Set native ID in variable pointed by @a ID_ptr
 * to @a native_val value
 */
#  define MHD_thread_ID_set_native_(ID_ptr,native_val) \
   ((ID_ptr)->valid = true, (ID_ptr)->native = native_val)
/**
 * Check whether native ID value is set in @a ID_var variable
 */
#  define MHD_thread_ID_is_valid_(ID_var) \
   ((ID_var).valid)
/**
 * Get native ID value from @a ID_var variable
 */
#  define MHD_thread_ID_get_native_(ID_var) \
    ((ID_var).native)
/**
 * Check whether @a ID_var variable is equal current thread
 */
#  define MHD_thread_ID_is_current_thread_(ID_var) \
    (MHD_thread_ID_is_valid_(ID_var) && \
     MHD_thread_ID_native_is_current_thread_((ID_var).native))
#else  /* MHD_THREAD_ID_NATIVE_INVALID_ */
/**
 * Type with thread ID that can be set to invalid value
 */
typedef MHD_thread_ID_native_ MHD_thread_ID_;

/**
 * Set variable pointed by @a ID_ptr to invalid (unset) value
 */
#  define MHD_thread_ID_set_invalid_(ID_ptr) \
    ((*(ID_ptr)) = MHD_THREAD_ID_NATIVE_VALUE_INVALID_)
/**
 * Set native ID in variable pointed by @a ID_ptr
 * to @a native_val value
 */
#  define MHD_thread_ID_set_native_(ID_ptr,native_val) \
    ((*(ID_ptr)) = native_val)
/**
 * Check whether native ID value is set in @a ID_var variable
 */
#  define MHD_thread_ID_is_valid_(ID_var) \
    (MHD_THREAD_ID_NATIVE_VALUE_INVALID_ != ID_var)
/**
 * Get native ID value from @a ID_var variable
 */
#  define MHD_thread_ID_get_native_(ID_var) \
    (ID_var)
/**
 * Check whether @a ID_var variable is equal current thread
 */
#  define MHD_thread_ID_is_current_thread_(ID_var) \
    MHD_thread_ID_native_is_current_thread_(ID_var)
#endif /* MHD_THREAD_ID_NATIVE_INVALID_ */

/**
 * Set current thread ID in variable pointed by @a ID_ptr
 */
#  define MHD_thread_ID_set_current_thread_(ID_ptr) \
    MHD_thread_ID_set_native_(ID_ptr,MHD_thread_ID_native_current_())


#if defined(MHD_USE_POSIX_THREADS)
#  if defined(MHD_THREAD_HANDLE_NATIVE_VALUE_INVALID_) && \
  ! defined(MHD_THREAD_ID_NATIVE_VALUE_INVALID_)
#    error \
  MHD_THREAD_ID_NATIVE_VALUE_INVALID_ is defined, but MHD_THREAD_ID_NATIVE_VALUE_INVALID_ is not defined
#  elif ! defined(MHD_THREAD_HANDLE_NATIVE_VALUE_INVALID_) && \
  defined(MHD_THREAD_ID_NATIVE_VALUE_INVALID_)
#    error \
  MHD_THREAD_ID_NATIVE_VALUE_INVALID_ is not defined, but MHD_THREAD_ID_NATIVE_VALUE_INVALID_ is defined
#  endif
#endif /* MHD_USE_POSIX_THREADS */

/* When staring a new thread, the kernel (and thread implementation) may
 * pause the calling (initial) thread and start the new thread.
 * If thread identifier is assigned to variable in the initial thread then
 * the value of the identifier variable will be undefined in the new thread
 * until the initial thread continue processing.
 * However, it is also possible that the new thread created, but not executed
 * for some time while the initial thread continue execution. In this case any
 * variable assigned in the new thread will be undefined for some time until
 * they really processed by the new thread.
 * To avoid data races, a special structure MHD_thread_handle_ID_ is used.
 * The "handle" is assigned by calling (initial) thread and should be always
 * defined when checked in the initial thread.
 * The "ID" is assigned by the new thread and should be always defined when
 * checked inside the new thread.
 */
/* Depending on implementation, pthread_create() MAY set thread ID into
 * provided pointer and after it start thread OR start thread and after
 * it set thread ID. In the latter case, to avoid data races, additional
 * pthread_self() call is required in thread routine. If some platform
 * is known for setting thread ID BEFORE starting thread macro
 * MHD_PTHREAD_CREATE__SET_ID_BEFORE_START_THREAD could be defined
 * to save some resources. */
/* #define MHD_PTHREAD_CREATE__SET_ID_BEFORE_START_THREAD 1 */

/* * handle - must be valid when other thread knows that particular thread
     is started.
   * ID     - must be valid when code is executed inside thread */
#if defined(MHD_USE_POSIX_THREADS) && \
  defined(MHD_PTHREAD_CREATE__SET_ID_BEFORE_START_THREAD) && \
  defined(MHD_THREAD_HANDLE_NATIVE_VALUE_INVALID_) && \
  defined(MHD_THREAD_ID_NATIVE_VALUE_INVALID_) && \
  defined(MHD_thread_handle_get_native_ptr_)
union _MHD_thread_handle_ID_
{
  MHD_thread_handle_ handle;    /**< To be used in other threads */
  MHD_thread_ID_ ID;            /**< To be used in the thread itself */
};
typedef union _MHD_thread_handle_ID_ MHD_thread_handle_ID_;
#  define MHD_THREAD_HANDLE_ID_IS_UNION 1
#else  /* !MHD_USE_POSIX_THREADS
          || !MHD_PTHREAD_CREATE__SET_ID_BEFORE_START_THREAD
          || !MHD_THREAD_HANDLE_NATIVE_VALUE_INVALID_
          || !MHD_THREAD_ID_NATIVE_VALUE_INVALID_
          || !MHD_thread_handle_get_native_ptr_ */
struct _MHD_thread_handle_ID_
{
  MHD_thread_handle_ handle;    /**< To be used in other threads */
  MHD_thread_ID_ ID;            /**< To be used in the thread itself */
};
typedef struct _MHD_thread_handle_ID_ MHD_thread_handle_ID_;
#endif /* !MHD_USE_POSIX_THREADS
          || !MHD_PTHREAD_CREATE__SET_ID_BEFORE_START_THREAD
          || !MHD_THREAD_HANDLE_NATIVE_VALUE_INVALID_
          || !MHD_THREAD_ID_NATIVE_VALUE_INVALID_
          || !MHD_thread_handle_get_native_ptr_ */

/**
 * Set MHD_thread_handle_ID_ to invalid value
 */
#define MHD_thread_handle_ID_set_invalid_(hndl_id_ptr) \
  (MHD_thread_handle_set_invalid_(&((hndl_id_ptr)->handle)), \
   MHD_thread_ID_set_invalid_(&((hndl_id_ptr)->ID)))

/**
 * Check whether thread handle is valid.
 * To be used in threads other then the thread specified by @a hndl_id.
 */
#define MHD_thread_handle_ID_is_valid_handle_(hndl_id) \
    MHD_thread_handle_is_valid_((hndl_id).handle)

/**
 * Set native handle in variable pointed by @a hndl_id_ptr
 * to @a native_val value
 */
#define MHD_thread_handle_ID_set_native_handle_(hndl_id_ptr,native_val) \
    MHD_thread_handle_set_native_(&((hndl_id_ptr)->handle),native_val)

#if defined(MHD_thread_handle_get_native_ptr_)
/**
 * Get pointer to native handle stored the variable pointed by @a hndl_id_ptr
 * @note This macro could not available if direct manipulation of
 *       the native handle is not possible
 */
#  define MHD_thread_handle_ID_get_native_handle_ptr_(hndl_id_ptr) \
    MHD_thread_handle_get_native_ptr_(&((hndl_id_ptr)->handle))
#endif /* MHD_thread_handle_get_native_ptr_ */

/**
 * Get native thread handle from MHD_thread_handle_ID_ variable.
 */
#define MHD_thread_handle_ID_get_native_handle_(hndl_id) \
    MHD_thread_handle_get_native_((hndl_id).handle)

/**
 * Check whether thread ID is valid.
 * To be used in the thread itself.
 */
#define MHD_thread_handle_ID_is_valid_ID_(hndl_id) \
    MHD_thread_ID_is_valid_((hndl_id).ID)

#if defined(MHD_THREAD_HANDLE_ID_IS_UNION)
#  if defined(MHD_USE_W32_THREADS)
#    error MHD_thread_handle_ID_ cannot be a union with W32 threads
#  endif /* MHD_USE_W32_THREADS */
/**
 * Set current thread ID in the variable pointed by @a hndl_id_ptr
 */
#  define MHD_thread_handle_ID_set_current_thread_ID_(hndl_id_ptr) (void) 0
#else  /* ! MHD_THREAD_HANDLE_ID_IS_UNION */
/**
 * Set current thread ID in the variable pointed by @a hndl_id_ptr
 */
#  define MHD_thread_handle_ID_set_current_thread_ID_(hndl_id_ptr) \
    MHD_thread_ID_set_current_thread_(&((hndl_id_ptr)->ID))
#endif /* ! MHD_THREAD_HANDLE_ID_IS_UNION */

/**
 * Check whether provided thread ID matches current thread.
 * @param ID thread ID to match
 * @return nonzero on match, zero otherwise
 */
#define MHD_thread_handle_ID_is_current_thread_(hndl_id) \
     MHD_thread_ID_is_current_thread_((hndl_id).ID)

/**
 * Wait until specified thread is ended and free thread handle on success.
 * @param hndl_id_ handle with ID to watch
 * @return nonzero on success, zero otherwise
 */
#define MHD_thread_handle_ID_join_thread_(hndl_id) \
  MHD_join_thread_(MHD_thread_handle_ID_get_native_handle_(hndl_id))

#if defined(MHD_USE_POSIX_THREADS)
#  define MHD_THRD_RTRN_TYPE_ void*
#  define MHD_THRD_CALL_SPEC_
#elif defined(MHD_USE_W32_THREADS)
#  define MHD_THRD_RTRN_TYPE_ unsigned
#  define MHD_THRD_CALL_SPEC_ __stdcall
#endif

/**
 * Signature of main function for a thread.
 *
 * @param cls closure argument for the function
 * @return termination code from the thread
 */
typedef MHD_THRD_RTRN_TYPE_
(MHD_THRD_CALL_SPEC_ *MHD_THREAD_START_ROUTINE_)(void *cls);


/**
 * Create a thread and set the attributes according to our options.
 *
 * If thread is created, thread handle must be freed by MHD_join_thread_().
 *
 * @param handle_id     handle to initialise
 * @param stack_size    size of stack for new thread, 0 for default
 * @param start_routine main function of thread
 * @param arg argument  for start_routine
 * @return non-zero on success; zero otherwise (with errno set)
 */
int
MHD_create_thread_ (MHD_thread_handle_ID_ *handle_id,
                    size_t stack_size,
                    MHD_THREAD_START_ROUTINE_ start_routine,
                    void *arg);

#ifndef MHD_USE_THREAD_NAME_
#define MHD_create_named_thread_(t,n,s,r,a) MHD_create_thread_ ((t),(s),(r),(a))
#else  /* MHD_USE_THREAD_NAME_ */
/**
 * Create a named thread and set the attributes according to our options.
 *
 * @param handle_id     handle to initialise
 * @param thread_name   name for new thread
 * @param stack_size    size of stack for new thread, 0 for default
 * @param start_routine main function of thread
 * @param arg argument  for start_routine
 * @return non-zero on success; zero otherwise
 */
int
MHD_create_named_thread_ (MHD_thread_handle_ID_ *handle_id,
                          const char *thread_name,
                          size_t stack_size,
                          MHD_THREAD_START_ROUTINE_ start_routine,
                          void *arg);

#endif /* MHD_USE_THREAD_NAME_ */

#endif /* ! MHD_THREADS_H */
