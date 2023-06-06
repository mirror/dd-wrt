/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-threads-internal.h  D-Bus thread primitives
 *
 * Copyright (C) 2002, 2005 Red Hat Inc.
 *
 * SPDX-License-Identifier: AFL-2.1 OR GPL-2.0-or-later
 *
 * Licensed under the Academic Free License version 2.1
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#ifndef DBUS_THREADS_INTERNAL_H
#define DBUS_THREADS_INTERNAL_H

#include <dbus/dbus-macros.h>
#include <dbus/dbus-types.h>
#include <dbus/dbus-threads.h>

/**
 * @addtogroup DBusThreadsInternals
 * @{
 */

/**
 * A mutex which is recursive if possible, else non-recursive.
 * This is typically recursive, but that cannot be relied upon.
 */
typedef struct DBusRMutex DBusRMutex;

/**
 * A mutex suitable for use with condition variables.
 * This is typically non-recursive.
 */
typedef struct DBusCMutex DBusCMutex;

/** @} */

DBUS_BEGIN_DECLS

DBUS_PRIVATE_EXPORT
void         _dbus_rmutex_lock               (DBusRMutex       *mutex);
DBUS_PRIVATE_EXPORT
void         _dbus_rmutex_unlock             (DBusRMutex       *mutex);
void         _dbus_rmutex_new_at_location    (DBusRMutex      **location_p);
void         _dbus_rmutex_free_at_location   (DBusRMutex      **location_p);

void         _dbus_cmutex_lock               (DBusCMutex       *mutex);
void         _dbus_cmutex_unlock             (DBusCMutex       *mutex);
void         _dbus_cmutex_new_at_location    (DBusCMutex      **location_p);
void         _dbus_cmutex_free_at_location   (DBusCMutex      **location_p);

DBusCondVar* _dbus_condvar_new               (void);
void         _dbus_condvar_free              (DBusCondVar       *cond);
void         _dbus_condvar_wait              (DBusCondVar       *cond,
                                              DBusCMutex        *mutex);
dbus_bool_t  _dbus_condvar_wait_timeout      (DBusCondVar       *cond,
                                              DBusCMutex        *mutex,
                                              int                timeout_milliseconds);
void         _dbus_condvar_wake_one          (DBusCondVar       *cond);
void         _dbus_condvar_new_at_location   (DBusCondVar      **location_p);
void         _dbus_condvar_free_at_location  (DBusCondVar      **location_p);

/* Private to threading implementations and dbus-threads.c */

/**
 * Creates a new mutex which is recursive if possible
 *
 * This mutex is used to avoid deadlocking if we hold them while
 * calling user code.
 *
 * @return  mutex instance or #NULL on OOM
 */
DBUS_EMBEDDED_TESTS_EXPORT
DBusRMutex  *_dbus_platform_rmutex_new       (void);

/**
 * Free a recursive usable mutex
 *
 * @param mutex the mutex instance to free
 */
DBUS_EMBEDDED_TESTS_EXPORT
void         _dbus_platform_rmutex_free      (DBusRMutex       *mutex);

/**
 * Locks a recursively usable mutex
 *
 * @param mutex the mutex instance to lock
 *
 * Unlike _dbus_cmutex_lock(), it is valid for the same thread
 * to lock a recursive mutex more than once, and it will not
 * deadlock. Each call to this function must be paired with a
 * corresponding call to _dbus_rmutex_unlock().
 */
DBUS_EMBEDDED_TESTS_EXPORT
void         _dbus_platform_rmutex_lock      (DBusRMutex       *mutex);

/**
 * Release a recursively usable mutex
 *
 * @param mutex the mutex instance to release
 */
DBUS_EMBEDDED_TESTS_EXPORT
void         _dbus_platform_rmutex_unlock    (DBusRMutex       *mutex);

/**
 * Creates a new mutex suitable for use with condition variables
 *
 * @return  mutex instance or #NULL on OOM
 */
DBUS_EMBEDDED_TESTS_EXPORT
DBusCMutex  *_dbus_platform_cmutex_new       (void);

/**
 * Implementation of _dbus_rmutex_new_at_location().
 * This should only be called internally by the threading implementation.
 */
DBUS_EMBEDDED_TESTS_EXPORT
void         _dbus_platform_cmutex_free      (DBusCMutex       *mutex);

/**
 * Locks a mutex suitable for use with condition variables
 *
 * @param mutex the mutex instance to lock
 *
 * @note On Windows, after a thread obtains ownership of a mutex,
 * it can specify the same mutex in repeated calls to the dbus
 * platform related mutex lock functions without blocking its
 * execution. This prevents a thread from deadlocking itself
 * while waiting for a mutex that it already owns. On unix
 * like os, calling the dbus platform related mutex lock
 * functions the second time is a programming error.
 */
DBUS_EMBEDDED_TESTS_EXPORT
void         _dbus_platform_cmutex_lock      (DBusCMutex       *mutex);

/**
 * Release a mutex suitable for use with condition variables
 *
 * @param mutex the mutex instance to release
 */
DBUS_EMBEDDED_TESTS_EXPORT
void         _dbus_platform_cmutex_unlock    (DBusCMutex       *mutex);

DBusCondVar* _dbus_platform_condvar_new      (void);
void         _dbus_platform_condvar_free     (DBusCondVar       *cond);
void         _dbus_platform_condvar_wait     (DBusCondVar       *cond,
                                              DBusCMutex        *mutex);
dbus_bool_t  _dbus_platform_condvar_wait_timeout (DBusCondVar   *cond,
                                              DBusCMutex        *mutex,
                                              int                timeout_milliseconds);
void         _dbus_platform_condvar_wake_one (DBusCondVar       *cond);

DBUS_END_DECLS

#endif /* DBUS_THREADS_INTERNAL_H */
