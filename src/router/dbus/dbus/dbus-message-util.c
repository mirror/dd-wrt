/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-message-util.c Would be in dbus-message.c, but only used by bus/tests
 *
 * Copyright 2009 Red Hat, Inc.
 * Copyright 2011-2017 Collabora Ltd.
 * Copyright 2017 Endless Mobile, Inc.
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

#include "dbus-internals.h"
#include "dbus-test.h"
#include "dbus-message-private.h"
#include "dbus-marshal-recursive.h"
#include "dbus-string.h"
#ifdef HAVE_UNIX_FD_PASSING
#include "dbus-sysdeps-unix.h"
#endif
#include <dbus/dbus-test-tap.h>

#ifdef __linux__
/* Necessary for the Linux-specific fd leak checking code only */
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#endif

/**
 * @addtogroup DBusMessage
 * @{
 */

/**
 * Gets the number of unix fds attached to this message.
 *
 * @param message the message
 * @returns the number of file descriptors
 */
unsigned int
_dbus_message_get_n_unix_fds (DBusMessage *message)
{
#ifdef HAVE_UNIX_FD_PASSING
  return message->n_unix_fds;
#else
  return 0;
#endif
}

#ifdef DBUS_ENABLE_EMBEDDED_TESTS

#ifdef __linux__
struct DBusInitialFDs {
    fd_set set;
};
#endif

DBusInitialFDs *
_dbus_check_fdleaks_enter (void)
{
#ifdef __linux__
  DIR *d;
  DBusInitialFDs *fds;

  /* this is plain malloc so it won't interfere with leak checking */
  fds = malloc (sizeof (DBusInitialFDs));
  _dbus_assert (fds != NULL);

  /* This works on Linux only */

  if ((d = opendir ("/proc/self/fd")))
    {
      struct dirent *de;

      while ((de = readdir(d)))
        {
          long l;
          char *e = NULL;
          int fd;

          if (de->d_name[0] == '.')
            continue;

          errno = 0;
          l = strtol (de->d_name, &e, 10);
          _dbus_assert (errno == 0 && e && !*e);

          fd = (int) l;

          if (fd < 3)
            continue;

          if (fd == dirfd (d))
            continue;

          if (fd >= FD_SETSIZE)
            {
              _dbus_verbose ("FD %d unexpectedly large; cannot track whether "
                             "it is leaked\n", fd);
              continue;
            }

          FD_SET (fd, &fds->set);
        }

      closedir (d);
    }

  return fds;
#else
  return NULL;
#endif
}

void
_dbus_check_fdleaks_leave (DBusInitialFDs *fds)
{
#ifdef __linux__
  DIR *d;

  /* This works on Linux only */

  if ((d = opendir ("/proc/self/fd")))
    {
      struct dirent *de;

      while ((de = readdir(d)))
        {
          long l;
          char *e = NULL;
          int fd;

          if (de->d_name[0] == '.')
            continue;

          errno = 0;
          l = strtol (de->d_name, &e, 10);
          _dbus_assert (errno == 0 && e && !*e);

          fd = (int) l;

          if (fd < 3)
            continue;

          if (fd == dirfd (d))
            continue;

          if (fd >= FD_SETSIZE)
            {
              _dbus_verbose ("FD %d unexpectedly large; cannot track whether "
                             "it is leaked\n", fd);
              continue;
            }

          if (FD_ISSET (fd, &fds->set))
            continue;

          _dbus_test_fatal ("file descriptor %i leaked in %s.", fd, __FILE__);
        }

      closedir (d);
    }

  free (fds);
#else
  _dbus_assert (fds == NULL);
#endif
}

#endif /* DBUS_ENABLE_EMBEDDED_TESTS */
