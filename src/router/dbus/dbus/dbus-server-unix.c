/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* dbus-server-unix.c Server implementation for Unix network protocols.
 *
 * Copyright (C) 2002, 2003, 2004  Red Hat Inc.
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

#include <config.h>
#include "dbus-internals.h"
#include "dbus-server-socket.h"
#include "dbus-server-launchd.h"
#include "dbus-transport-unix.h"
#include "dbus-connection-internal.h"
#include "dbus-sysdeps-unix.h"
#include "dbus-string.h"

/**
 * @defgroup DBusServerUnix DBusServer implementations for UNIX
 * @ingroup  DBusInternals
 * @brief Implementation details of DBusServer on UNIX
 *
 * @{
 */

/**
 * Tries to interpret the address entry in a platform-specific
 * way, creating a platform-specific server type if appropriate.
 * Sets error if the result is not OK.
 *
 * @param entry an address entry
 * @param server_p location to store a new DBusServer, or #NULL on failure.
 * @param error location to store rationale for failure on bad address
 * @returns the outcome
 *
 */
DBusServerListenResult
_dbus_server_listen_platform_specific (DBusAddressEntry *entry,
                                       DBusServer      **server_p,
                                       DBusError        *error)
{
  const char *method;

  *server_p = NULL;

  method = dbus_address_entry_get_method (entry);
  if (strcmp (method, "systemd") == 0)
    {
      int i, n;
      DBusSocket *fds;
      DBusString address;

      n = _dbus_listen_systemd_sockets (&fds, error);
      if (n < 0)
        {
          _DBUS_ASSERT_ERROR_IS_SET (error);
          return DBUS_SERVER_LISTEN_DID_NOT_CONNECT;
        }

      if (!_dbus_string_init (&address))
          goto systemd_oom;

      for (i = 0; i < n; i++)
        {
          if (i > 0)
            {
              if (!_dbus_string_append (&address, ";"))
                goto systemd_oom;
            }
          if (!_dbus_append_address_from_socket (fds[i], &address, error))
            goto systemd_err;
        }

      *server_p = _dbus_server_new_for_socket (fds, n, &address, NULL, error);
      if (*server_p == NULL)
        goto systemd_err;

      dbus_free (fds);
      _dbus_string_free (&address);

      return DBUS_SERVER_LISTEN_OK;

  systemd_oom:
      _DBUS_SET_OOM (error);
  systemd_err:
      for (i = 0; i < n; i++)
        {
          _dbus_close_socket (&fds[i], NULL);
        }
      dbus_free (fds);
      _dbus_string_free (&address);

      return DBUS_SERVER_LISTEN_DID_NOT_CONNECT;
    }
#ifdef DBUS_ENABLE_LAUNCHD
  else if (strcmp (method, "launchd") == 0)
    {
      const char *launchd_env_var = dbus_address_entry_get_value (entry, "env");
      if (launchd_env_var == NULL)
        {
          _dbus_set_bad_address (error, "launchd", "env", NULL);
          return DBUS_SERVER_LISTEN_DID_NOT_CONNECT;
        }
      *server_p = _dbus_server_new_for_launchd (launchd_env_var, error);

      if (*server_p != NULL)
        {
          _DBUS_ASSERT_ERROR_IS_CLEAR(error);
          return DBUS_SERVER_LISTEN_OK;
        }
      else
        {
          _DBUS_ASSERT_ERROR_IS_SET(error);
          return DBUS_SERVER_LISTEN_DID_NOT_CONNECT;
        }
    }
#endif
  else
    {
      /* If we don't handle the method, we return NULL with the
       * error unset
       */
      _DBUS_ASSERT_ERROR_IS_CLEAR(error);
      return DBUS_SERVER_LISTEN_NOT_HANDLED;
    }
}

/** @} */
