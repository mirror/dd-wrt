/*
 * Copyright © 2018 Collabora Ltd.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <config.h>

#include <dbus/dbus.h>
#include "dbus/dbus-sysdeps.h"
#include "test-utils-glib.h"

#ifdef G_OS_UNIX
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "dbus/dbus-sysdeps-unix.h"
#endif

typedef struct
{
  size_t n_fds;
  int *fds;
} Fixture;

static void
setup (Fixture *f G_GNUC_UNUSED,
       gconstpointer context G_GNUC_UNUSED)
{
}

/* In an unfortunate clash of terminology, GPid is actually a process
 * handle (a HANDLE, vaguely analogous to a file descriptor) on Windows.
 * For _dbus_command_for_pid() we need an actual process ID. */
#ifdef G_OS_UNIX
# define NO_PROCESS 0
# ifndef G_PID_FORMAT
#   define G_PID_FORMAT "i"
# endif

static unsigned long
get_pid (pid_t process)
{
  return process;
}

static void
terminate_and_wait (pid_t process)
{
  int ret;

  kill (process, SIGTERM);

  do
    {
      ret = waitpid (process, NULL, 0);
    }
  while (ret == -1 && errno == EINTR);
}
#else
# define NO_PROCESS NULL
# ifndef G_PID_FORMAT
#   define G_PID_FORMAT "p"
# endif

static unsigned long
get_pid (HANDLE process)
{
  return GetProcessId (process);
}

static void
terminate_and_wait (HANDLE process)
{
  TerminateProcess (process, 1);
  WaitForSingleObject (process, INFINITE);
}
#endif

static void
test_command_for_pid (Fixture *f,
                      gconstpointer context G_GNUC_UNUSED)
{
  gchar *argv[] = { NULL, NULL, NULL };
  GError *error = NULL;
  GPid process = NO_PROCESS;
  unsigned long pid;
  DBusError d_error = DBUS_ERROR_INIT;
  DBusString string;

  argv[0] = test_get_helper_executable ("test-sleep-forever" DBUS_EXEEXT);

  if (argv[0] == NULL)
    {
      g_test_skip ("DBUS_TEST_EXEC not set");
      return;
    }

  argv[1] = g_strdup ("bees");

  if (!g_spawn_async (NULL, argv, NULL,
                      (G_SPAWN_STDOUT_TO_DEV_NULL |
                       G_SPAWN_STDERR_TO_DEV_NULL |
                       G_SPAWN_DO_NOT_REAP_CHILD),
                      NULL, NULL,
                      &process, &error))
    g_error ("Unable to run %s: %s", argv[0], error->message);

  pid = get_pid (process);
  g_test_message ("Process ID of process handle %" G_PID_FORMAT ": %lu",
                  process, pid);

  if (!_dbus_string_init (&string))
    g_error ("out of memory");

  if (_dbus_command_for_pid (pid, &string, strlen (argv[0]) + 1024, &d_error))
    {
      gchar *expected;

      g_test_message ("Process %lu: \"%s\"", pid,
                      _dbus_string_get_const_data (&string));

      expected = g_strdup_printf ("%s %s", argv[0], argv[1]);

      g_assert_cmpstr (_dbus_string_get_const_data (&string), ==,
                       expected);
      g_assert_cmpuint (_dbus_string_get_length (&string), ==,
                        strlen (expected));
      g_free (expected);
    }
  else
    {
      g_test_message ("Unable to get command for process %lu: %s: %s",
                      pid, d_error.name, d_error.message);
      g_assert_nonnull (d_error.name);
      g_assert_nonnull (d_error.message);
      dbus_error_free (&d_error);
    }

  if (!_dbus_string_set_length (&string, 0))
    g_error ("out of memory");

  /* Test truncation */
  if (_dbus_command_for_pid (pid, &string, 10, NULL))
    {
      gchar *expected;

      g_test_message ("Process %lu (truncated): \"%s\"", pid,
                      _dbus_string_get_const_data (&string));

      expected = g_strdup_printf ("%s %s", argv[0], argv[1]);
      expected[10] = '\0';

      g_assert_cmpstr (_dbus_string_get_const_data (&string), ==,
                       expected);
      g_assert_cmpuint (_dbus_string_get_length (&string), ==, 10);
      g_free (expected);
    }
  else
    {
      g_test_message ("Unable to get command for process %lu", pid);
    }

  if (process != NO_PROCESS)
    terminate_and_wait (process);

  _dbus_string_free (&string);
  g_spawn_close_pid (process);
  g_free (argv[1]);
  g_free (argv[0]);
}

#ifdef G_OS_UNIX
static gboolean
check_valid_fd (int fd,
                gboolean *close_on_exec)
{
  int flags = fcntl (fd, F_GETFD);

  if (flags < 0)
    {
      int saved_errno = errno;

      g_assert_cmpint (saved_errno, ==, EBADF);
      return FALSE;
    }

  if (close_on_exec != NULL)
    *close_on_exec = ((flags & FD_CLOEXEC) != 0);

  return TRUE;
}

static void
fixture_open_some_fds (Fixture *f)
{
  const size_t n = 50;  /* must be divisible by 2, otherwise arbitrary */
  size_t i;
  const char *error_str = NULL;

  /* Ensure that fds[] will not contain 0, 1 or 2 */
  if (!_dbus_ensure_standard_fds (0, &error_str))
    g_error ("_dbus_ensure_standard_fds: %s (%s)",
             error_str, g_strerror (errno));

  g_assert_cmpuint (f->n_fds, ==, 0);
  g_assert_null (f->fds);
  f->n_fds = n;
  f->fds = g_new0 (int, n);

  for (i = 0; i < n; i++)
    f->fds[i] = -1;

  for (i = 0; i < n; i += 2)
    {
      gboolean cloexec;

      if (pipe (&f->fds[i]) < 0)
        g_error ("pipe: %s", g_strerror (errno));

      g_assert_cmpint (f->fds[i], >=, 3);
      g_assert_cmpint (f->fds[i + 1], >=, 3);
      _dbus_fd_clear_close_on_exec (f->fds[i]);
      _dbus_fd_clear_close_on_exec (f->fds[i + 1]);
      g_assert_true (check_valid_fd (f->fds[i], &cloexec));
      g_assert_false (cloexec);
      g_assert_true (check_valid_fd (f->fds[i + 1], &cloexec));
      g_assert_false (cloexec);
    }
}

static void
test_close_all (Fixture *f,
                gconstpointer context G_GNUC_UNUSED)
{
  size_t i;

  fixture_open_some_fds (f);

  for (i = 0; i < f->n_fds; i++)
    g_assert_true (check_valid_fd (f->fds[i], NULL));

  _dbus_close_all ();

  for (i = 0; i < f->n_fds; i++)
    {
      g_assert_false (check_valid_fd (f->fds[i], NULL));
      /* Don't re-close the fd in teardown */
      f->fds[i] = -1;
    }
}

static void
test_set_all_close_on_exec (Fixture *f,
                            gconstpointer context G_GNUC_UNUSED)
{
  size_t i;

  fixture_open_some_fds (f);

  for (i = 0; i < f->n_fds; i++)
    {
      gboolean cloexec;

      g_assert_true (check_valid_fd (f->fds[i], &cloexec));
      g_assert_false (cloexec);
    }

  _dbus_fd_set_all_close_on_exec ();

  for (i = 0; i < f->n_fds; i++)
    {
      gboolean cloexec;

      g_assert_true (check_valid_fd (f->fds[i], &cloexec));
      g_assert_true (cloexec);
    }
}
#endif

static void
teardown (Fixture *f G_GNUC_UNUSED,
          gconstpointer context G_GNUC_UNUSED)
{
#ifdef G_OS_UNIX
  size_t i;

  for (i = 0; i < f->n_fds; i++)
    {
      if (f->fds[i] >= 0)
        _dbus_close (f->fds[i], NULL);
    }

  g_free (f->fds);
#endif
}

int
main (int argc,
      char **argv)
{
  int ret;

  test_init (&argc, &argv);

  g_test_add ("/sysdeps/command_for_pid",
              Fixture, NULL, setup, test_command_for_pid, teardown);

#ifdef G_OS_UNIX
  g_test_add ("/sysdeps/close_all",
              Fixture, NULL, setup, test_close_all, teardown);
  g_test_add ("/sysdeps/set_all_close_on_exec",
              Fixture, NULL, setup, test_set_all_close_on_exec, teardown);
#endif

  ret = g_test_run ();
  dbus_shutdown ();
  return ret;
}
