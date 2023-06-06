/*
 * Copyright © 2018-2022 Ralf Habacker <ralf.habacker@freenet.de>
 * SPDX-License-Identifier: MIT
 */

/**
 * This test checks whether a client can connect to a dbus daemon configured
 * for a default, user-defined and installation path related autostart and
 * whether it can connect to a server having a different autolaunch
 * configuration.
 */

#include "config.h"

#include "dbus/dbus-file.h"
#include "dbus/dbus-internals.h"
#include "dbus/dbus-sysdeps.h"
#include "dbus/dbus-test-tap.h"
#include "dbus/dbus-test.h"
#include "dbus/dbus.h"
#include "test/test-utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/* dbus_bus_get does not work yet */
static dbus_bool_t use_bus_get = FALSE;

static int add_wait_time = 0;

#define oom() _dbus_test_fatal ("Out of memory")

static dbus_bool_t
call_method (DBusConnection *conn,
               DBusError *error,
               int timeout,
               const char *interface,
               const char *method_str)
{
  DBusMessage *method;
  DBusMessage *reply;
  dbus_bool_t result = TRUE;

  dbus_error_init (error);

  method = dbus_message_new_method_call (DBUS_SERVICE_DBUS,
                                         DBUS_PATH_DBUS,
                                         interface,
                                         method_str);

  reply = dbus_connection_send_with_reply_and_block (conn, method, timeout, error);
  dbus_message_unref (method);
  if (reply == NULL)
    {
      result = FALSE;
      goto out;
    }

  /* ..._send_with_reply_and_block converts ERROR messages into errors */
  _dbus_assert (dbus_message_get_type (reply) != DBUS_MESSAGE_TYPE_ERROR);
  dbus_message_unref (reply);
  result = TRUE;

out:
  _DBUS_ASSERT_ERROR_XOR_BOOL (error, result);
  return result;
}

static dbus_bool_t
_server_check_connection (DBusConnection *conn,
                          DBusError *error)
{
  if (use_bus_get)
    return call_method (conn, error, -1, DBUS_INTERFACE_PEER, "GetMachineId");
  else
    return call_method (conn, error, -1, DBUS_INTERFACE_DBUS, "Hello");
}

static HANDLE autolaunch_handle = NULL;

static dbus_bool_t
_server_shutdown (DBusConnection *conn,
                  const char *scope,
                  int timeout,
                  DBusError *error)
{
  _dbus_assert (autolaunch_handle != NULL);

  _dbus_test_diag ("Shutting down dbus-daemon (handle=%p)", autolaunch_handle);
  if (!TerminateProcess (autolaunch_handle, 1))
    _dbus_test_fatal ("Unable to terminate dbus-daemon (handle=%p) : %s",
                      /* this string is leaked, but we're crashing anyway */
                      autolaunch_handle, _dbus_win_error_string (GetLastError ()));

  _dbus_test_diag ("Return value from closing autolaunch_handle is %d", CloseHandle (autolaunch_handle));
  autolaunch_handle = NULL;
  _dbus_test_win_set_autolaunch_handle_location (NULL);
  return TRUE;
}

typedef enum
{
  RUN_TEST_DEFAULT = 0,
  RUN_TEST_EXPECT_CONNECTION_TO_FAIL = (1 << 0),
} RunTestFlags;

static dbus_bool_t
check_results (DBusConnection *conn,
               DBusString *server_address,
               DBusString *address,
               const char *scope,
               RunTestFlags flags,
               DBusError *error)
{
  if (add_wait_time)
    _dbus_sleep_milliseconds (add_wait_time);

  if (dbus_error_is_set (error))
    _dbus_test_diag ("Error is set: %s %s", error->name, error->message);

  if (conn == NULL)
    {
      if (!dbus_error_is_set (error))
        _dbus_test_fatal ("Failed to autolaunch session bus and no error was set");

      if (flags & RUN_TEST_EXPECT_CONNECTION_TO_FAIL)
        return TRUE;

      _dbus_test_diag ("autolaunch unexpectedly failed: %s: %s", error->name, error->message);
      return FALSE;
    }
  else
    {
      if (dbus_error_is_set (error))
        _dbus_test_fatal ("Successfully autolaunched session bus but error was set: %s: %s", error->name, error->message);

      if (flags & RUN_TEST_EXPECT_CONNECTION_TO_FAIL)
        {
          _dbus_test_diag ("autolaunch unexpectedly succeeded");
          return FALSE;
        }
      _dbus_test_diag ("Client connection succeeded - uses '%s'", _dbus_string_get_const_data (address));
    }

  if (add_wait_time)
    _dbus_sleep_milliseconds (add_wait_time);

  _dbus_test_diag ("Server returned bus address '%s'", _dbus_string_get_const_data (server_address));
  if (!_server_check_connection (conn, error))
    {
      _dbus_test_diag ("Could not execute server function");
      return FALSE;
    }
  else
    _dbus_test_diag ("Calling server function succeeded");

  return TRUE;
}

static dbus_bool_t
run_test (const char *server_scope, const char *scope, const char *test_data_dir, RunTestFlags flags)
{
  DBusConnection *conn = NULL;
  DBusError error;
  DBusString server_address = _DBUS_STRING_INIT_INVALID;
  DBusString address = _DBUS_STRING_INIT_INVALID;
  DBusString session_parameter = _DBUS_STRING_INIT_INVALID;
  dbus_bool_t result = FALSE;
  TestMainContext *ctx;
  _dbus_assert (test_data_dir);

  ctx = test_main_context_get ();

  dbus_error_init (&error);

  if (!_dbus_string_init (&server_address))
    oom ();

  if (!_dbus_string_init (&address))
    oom ();

  _dbus_test_diag ("run test");

  if (*server_scope != '\0')
    {
      if (!_dbus_string_append_printf (&server_address, "autolaunch:scope=%s", server_scope))
        oom ();
    }
  else if (!_dbus_string_append_printf (&server_address, "autolaunch:"))
    {
      oom ();
    }

  if (*scope != '\0')
    {
      if (!_dbus_string_append_printf (&address, "autolaunch:scope=%s", scope))
        oom ();
    }
  else if (!_dbus_string_append_printf (&address, "autolaunch:"))
    {
      oom ();
    }

  if (!_dbus_string_init (&session_parameter))
    oom ();

  /* We haven't implemented any form of escaping quotes,
   * but Windows doesn't allow filenames to contain quotes
   * so it shouldn't matter. */
  _dbus_test_check (strchr (test_data_dir, '"') == NULL);

  _dbus_test_check (strchr (_dbus_string_get_const_data (&server_address), '"') == NULL);

  if (!_dbus_string_append_printf (&session_parameter, "\"--config-file=%s/%s\" \"--address=%s\"", test_data_dir, "valid-config-files/listen-autolaunch-win.conf", _dbus_string_get_const_data (&server_address)))
    {
      oom ();
    }

  _dbus_test_win_autolaunch_set_command_line_parameter (_dbus_string_get_const_data (&session_parameter));

  _dbus_test_diag ("Autolaunch handle initially %p", autolaunch_handle);
  _dbus_test_win_set_autolaunch_handle_location (&autolaunch_handle);

  if (use_bus_get)
    {
      dbus_setenv ("DBUS_SESSION_BUS_ADDRESS", _dbus_string_get_const_data (&address));
      _dbus_test_diag ("got env %s", getenv ("DBUS_SESSION_BUS_ADDRESS"));
      conn = dbus_bus_get_private (DBUS_BUS_SESSION, &error);
      dbus_connection_set_exit_on_disconnect (conn, FALSE);
    }
  else
    {
      conn = dbus_connection_open_private (_dbus_string_get_const_data (&address), &error);
    }

  _dbus_test_diag ("After attempting to connect: autolaunch handle is %p", autolaunch_handle);
  if (conn)
    test_connection_setup (ctx, conn);

  result = check_results (conn, &server_address, &address, scope, flags, &error);

  if (conn)
    {
      _dbus_test_diag("Shutdown connection '%p'", conn);
      test_connection_shutdown (ctx, conn);
      dbus_connection_close (conn);
      dbus_connection_unref (conn);
    }

  _server_shutdown (conn, scope, -1, &error);
  _dbus_test_diag ("server has been shut down");

  _dbus_string_free (&address);
  _dbus_string_free (&server_address);
  _dbus_string_free (&session_parameter);

  test_main_context_unref (ctx);

  return result;
}

static dbus_bool_t
run_test_okay (const char *scope, const char *test_data_dir)
{
  return run_test (scope, scope, test_data_dir, RUN_TEST_DEFAULT);
}

static dbus_bool_t
_dbus_autolaunch_default_test (const char *test_data_dir)
{
  return run_test_okay ("", test_data_dir);
}

static dbus_bool_t
_dbus_autolaunch_custom_scope_test (const char *test_data_dir)
{
  return run_test_okay ("123", test_data_dir);
}

static dbus_bool_t
_dbus_autolaunch_install_path_scope_test (const char *test_data_dir)
{
  return run_test_okay ("*install-path", test_data_dir);
}

static dbus_bool_t
_dbus_autolaunch_user_scope_test (const char *test_data_dir)
{
  return run_test_okay ("*user", test_data_dir);
}

static dbus_bool_t
_dbus_autolaunch_loop_test (const char *test_data_dir, dbus_bool_t same_scope)
{
  int i;
  int max = 10;

  for (i = 0; i < max; i++)
    {
      char s[2] = { i+'A', 0 };
      if (!run_test_okay (same_scope ? "A" : s, test_data_dir))
        _dbus_test_not_ok ("%d", max);
      else
        _dbus_test_ok ("%d", max);
      if (add_wait_time)
        _dbus_sleep_milliseconds (add_wait_time);
    }
  return TRUE;
}

static dbus_bool_t
_dbus_autolaunch_same_scope_loop_test (const char *test_data_dir)
{
  return _dbus_autolaunch_loop_test (test_data_dir, TRUE);
}

static dbus_bool_t
_dbus_autolaunch_different_scope_loop_test (const char *test_data_dir)
{
  return _dbus_autolaunch_loop_test (test_data_dir, FALSE);
}

static DBusTestCase tests[] = {
  { "default", _dbus_autolaunch_default_test },
  { "custom", _dbus_autolaunch_custom_scope_test },
  { "install-path", _dbus_autolaunch_install_path_scope_test },
  { "user", _dbus_autolaunch_user_scope_test },
  { "loop", _dbus_autolaunch_same_scope_loop_test },
  { "different-scope-loop", _dbus_autolaunch_different_scope_loop_test },
};

int
main (int argc,
      char **argv)
{
  return _dbus_test_main (argc, argv, _DBUS_N_ELEMENTS (tests), tests,
                          DBUS_TEST_FLAGS_CHECK_MEMORY_LEAKS,
                          NULL, NULL);
}
