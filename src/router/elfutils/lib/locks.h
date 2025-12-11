/* Configuration definitions.
   Copyright (C) 2024 Red Hat, Inc.
   This file is part of elfutils.

   This file is free software; you can redistribute it and/or modify
   it under the terms of either

     * the GNU Lesser General Public License as published by the Free
       Software Foundation; either version 3 of the License, or (at
       your option) any later version

   or

     * the GNU General Public License as published by the Free
       Software Foundation; either version 2 of the License, or (at
       your option) any later version

   or both in parallel, as here.

   elfutils is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received copies of the GNU General Public License and
   the GNU Lesser General Public License along with this program.  If
   not, see <http://www.gnu.org/licenses/>.  */

#ifndef LOCKS_H
#define LOCKS_H     1

#ifdef USE_LOCKS
# include <pthread.h>
# include <assert.h>
# define rwlock_define(class,name)      class pthread_rwlock_t name
# define once_define(class,name)  class pthread_once_t name = PTHREAD_ONCE_INIT
# define RWLOCK_CALL(call)              \
  ({ int _err = pthread_rwlock_ ## call; assert_perror (_err); })
# define ONCE_CALL(call)  \
  ({ int _err = pthread_ ## call; assert_perror (_err); })
# define rwlock_init(lock)              RWLOCK_CALL (init (&lock, NULL))
# define rwlock_fini(lock)              RWLOCK_CALL (destroy (&lock))
# define rwlock_rdlock(lock)            RWLOCK_CALL (rdlock (&lock))
# define rwlock_wrlock(lock)            RWLOCK_CALL (wrlock (&lock))
# define rwlock_unlock(lock)            RWLOCK_CALL (unlock (&lock))
# define mutex_define(class,name)       class pthread_mutex_t name
# define MUTEX_CALL(call)		\
  ({ int _err = pthread_mutex_ ## call; assert_perror (_err); })
# define mutex_init(lock)					   \
  ({ pthread_mutexattr_t _attr;					   \
     pthread_mutexattr_init (&_attr);				   \
     pthread_mutexattr_settype (&_attr, PTHREAD_MUTEX_RECURSIVE);  \
     MUTEX_CALL (init (&lock, &_attr)); })
# define mutex_lock(_lock)		MUTEX_CALL (lock (&_lock))
# define mutex_unlock(lock)		MUTEX_CALL (unlock (&lock))
# define mutex_fini(lock)		MUTEX_CALL (destroy (&lock))
# define once(once_control, init_routine)  \
  ONCE_CALL (once (&once_control, init_routine))
#else
/* Eventually we will allow multi-threaded applications to use the
   libraries.  Therefore we will add the necessary locking although
   the macros used expand to nothing for now.  */
# define rwlock_define(class,name) class int name
# define rwlock_init(lock) ((void) (lock))
# define rwlock_fini(lock) ((void) (lock))
# define rwlock_rdlock(lock) ((void) (lock))
# define rwlock_wrlock(lock) ((void) (lock))
# define rwlock_unlock(lock) ((void) (lock))
# define mutex_define(class,name) class int name
# define mutex_init(lock) ((void) (lock))
# define mutex_lock(lock) ((void) (lock))
# define mutex_unlock(lock) ((void) (lock))
# define mutex_fini(lock) ((void) (lock))
# define once_define(class,name)
# define once(once_control, init_routine)       init_routine()
#endif  /* USE_LOCKS */

#endif  /* locks.h */
