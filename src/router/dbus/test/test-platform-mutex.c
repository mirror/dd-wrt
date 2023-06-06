/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* test-platform-mutex.c
 *
 * Copyright © 2022 Ralf Habacker <ralf.habacker@freenet.de>
 * SPDX-License-Identifier: AFL-2.1 OR GPL-2.0-or-later

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
#include <config.h>

#include "dbus/dbus-internals.h"
#include "dbus/dbus-spawn.h"
#include "dbus/dbus-sysdeps.h"
#include "dbus/dbus-test.h"
#include "test/test-utils.h"

#define fail_if_mutex_is_null(a) if (a == NULL) _dbus_test_fatal ("Could not create cmutex")


static dbus_bool_t
_dbus_check_cmutex_new (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  DBusCMutex *mutex = NULL;
  int count;

  count = _dbus_get_check_failed_count ();
  mutex = _dbus_platform_cmutex_new ();
  fail_if_mutex_is_null (mutex);
  _dbus_platform_cmutex_free (mutex);
  return _dbus_get_check_failed_count () == count;
}

static dbus_bool_t
_dbus_check_cmutex_lock (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  DBusCMutex *mutex = NULL;
  int count;

  count = _dbus_get_check_failed_count ();
  mutex = _dbus_platform_cmutex_new ();
  fail_if_mutex_is_null (mutex);
  _dbus_platform_cmutex_lock (mutex);
  _dbus_platform_cmutex_unlock (mutex);
  _dbus_platform_cmutex_free (mutex);
  return _dbus_get_check_failed_count () == count;
}

#ifdef DBUS_WIN
static dbus_bool_t
_dbus_check_cmutex_lock_null_pointer (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  DBusCMutex *mutex = NULL;
  int count;

  count = _dbus_get_check_failed_count ();
  _dbus_platform_cmutex_lock (mutex);
  return _dbus_get_check_failed_count () - count == 1;
}

static dbus_bool_t
_dbus_check_cmutex_double_lock (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  DBusCMutex *mutex = NULL;
  int count;

  count = _dbus_get_check_failed_count ();
  mutex = _dbus_platform_cmutex_new ();
  fail_if_mutex_is_null (mutex);
  _dbus_platform_cmutex_lock (mutex);
  _dbus_platform_cmutex_lock (mutex);
  _dbus_platform_cmutex_unlock (mutex);
  _dbus_platform_cmutex_unlock (mutex);
  _dbus_platform_cmutex_free (mutex);
  return _dbus_get_check_failed_count () == count;
}

static dbus_bool_t
_dbus_check_cmutex_unlock_null_pointer (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  DBusCMutex *mutex = NULL;
  int count;

  count = _dbus_get_check_failed_count ();
  _dbus_platform_cmutex_unlock (mutex);
  return _dbus_get_check_failed_count () - count == 1;
}

static dbus_bool_t
_dbus_check_cmutex_null_free (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  DBusCMutex *mutex = NULL;
  int count;

  count = _dbus_get_check_failed_count ();
  _dbus_platform_cmutex_free (mutex);  /* programming error (NULL isn't a mutex) */
  return _dbus_get_check_failed_count () - count == 1;
}

static dbus_bool_t
_dbus_check_cmutex_double_free (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  DBusCMutex *mutex = NULL;
  int count;

  mutex = _dbus_platform_cmutex_new ();
  fail_if_mutex_is_null (mutex);
  count = _dbus_get_check_failed_count ();
  _dbus_platform_cmutex_free (mutex);
  if (_dbus_get_check_failed_count () - count > 0)
    {
      _dbus_test_not_ok ("free'ing mutex failed");
      return FALSE;
    }
  _dbus_platform_cmutex_free (mutex);
  return _dbus_get_check_failed_count () - count == 1;
}
#else
/*
 * #NULL pointers of type DBusCMutex cannot be tested on unix-like
 * operating systems, because they are pointing to a data structure
 * and would cause a segment violation when accessed.
 */
#endif

static dbus_bool_t
_dbus_check_rmutex_new (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  DBusRMutex *mutex = NULL;
  int count;

  count = _dbus_get_check_failed_count ();
  mutex = _dbus_platform_rmutex_new ();
  fail_if_mutex_is_null (mutex);
  _dbus_platform_rmutex_free (mutex);
  return _dbus_get_check_failed_count () == count;
}

static dbus_bool_t
_dbus_check_rmutex_lock (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  DBusRMutex *mutex = NULL;
  int count;

  count = _dbus_get_check_failed_count ();
  mutex = _dbus_platform_rmutex_new ();
  fail_if_mutex_is_null (mutex);
  _dbus_platform_rmutex_lock (mutex);
  _dbus_platform_rmutex_unlock (mutex);
  _dbus_platform_rmutex_free (mutex);
  return _dbus_get_check_failed_count () == count;
}
#ifdef DBUS_WIN
static dbus_bool_t
_dbus_check_rmutex_lock_null_pointer (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  DBusRMutex *mutex = NULL;
  int count;

  count = _dbus_get_check_failed_count ();
  _dbus_platform_rmutex_lock (mutex);
  return _dbus_get_check_failed_count () - count == 1;
}
#endif

static dbus_bool_t
_dbus_check_rmutex_double_lock (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  DBusRMutex *mutex = NULL;
  int count;

  count = _dbus_get_check_failed_count ();
  mutex = _dbus_platform_rmutex_new ();
  fail_if_mutex_is_null (mutex);
  _dbus_platform_rmutex_lock (mutex);
  _dbus_platform_rmutex_lock (mutex);
  _dbus_platform_rmutex_unlock (mutex);
  _dbus_platform_rmutex_unlock (mutex);
  _dbus_platform_rmutex_free (mutex);
  return _dbus_get_check_failed_count () == count;
}

#ifdef DBUS_WIN
static dbus_bool_t
_dbus_check_rmutex_unlock_null_pointer (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  DBusRMutex *mutex = NULL;
  int count;

  count = _dbus_get_check_failed_count ();
  _dbus_platform_rmutex_unlock (mutex);
  return _dbus_get_check_failed_count () - count == 1;
}

static dbus_bool_t
_dbus_check_rmutex_null_free (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  DBusRMutex *mutex = NULL;
  int count;

  count = _dbus_get_check_failed_count ();
  _dbus_platform_rmutex_free (mutex);  /* programming error (NULL isn't a mutex) */
  return _dbus_get_check_failed_count () - count == 1;
}

static dbus_bool_t
_dbus_check_rmutex_double_free (const char *test_data_dir _DBUS_GNUC_UNUSED)
{
  DBusRMutex *mutex = NULL;
  int count;

  mutex = _dbus_platform_rmutex_new ();
  fail_if_mutex_is_null (mutex);
  count = _dbus_get_check_failed_count ();
  _dbus_platform_rmutex_free (mutex);
  if (_dbus_get_check_failed_count () - count > 0)
    {
      _dbus_test_not_ok ("free'ing mutex failed");
      return FALSE;
    }
  _dbus_platform_rmutex_free (mutex);
  return _dbus_get_check_failed_count () - count == 1;
}
#else
/*
 * #NULL pointers of type DBusRMutex cannot be tested on unix-like
 * operating systems, because they are pointing to a data structure
 * and would cause a segment violation when accessed.
 */
#endif

static DBusTestCase tests[] =
{
  { "cmutex_new", _dbus_check_cmutex_new},
  { "cmutex_lock", _dbus_check_cmutex_lock},
#ifdef DBUS_WIN
  { "cmutex_lock_null_pointer", _dbus_check_cmutex_lock_null_pointer},
  { "cmutex_double_lock", _dbus_check_cmutex_double_lock},
  { "cmutex_unlock_null_pointer", _dbus_check_cmutex_unlock_null_pointer},
  { "cmutex_null_free", _dbus_check_cmutex_null_free},
  { "cmutex_double_free", _dbus_check_cmutex_double_free},
#endif
  { "rmutex_new", _dbus_check_rmutex_new},
  { "rmutex_lock", _dbus_check_rmutex_lock},
#ifdef DBUS_WIN
  { "rmutex_lock_null_pointer", _dbus_check_rmutex_lock_null_pointer},
#endif
  { "rmutex_double_lock", _dbus_check_rmutex_double_lock},
#ifdef DBUS_WIN
  { "rmutex_unlock_null_pointer", _dbus_check_rmutex_unlock_null_pointer},
  { "rmutex_null_free", _dbus_check_rmutex_null_free},
  { "rmutex_double_free", _dbus_check_rmutex_double_free},
#endif
};

int
main (int    argc,
      char **argv)
{
  dbus_setenv ("DBUS_FATAL_WARNINGS", "0");

  return _dbus_test_main (argc, argv, _DBUS_N_ELEMENTS (tests), tests,
                          DBUS_TEST_FLAGS_CHECK_MEMORY_LEAKS,
                          NULL, NULL);
}
