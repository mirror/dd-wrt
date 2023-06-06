/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/* test-main.c  main() for make check
 *
 * Copyright 2003-2009 Red Hat, Inc.
 * Copyright 2011-2018 Collabora Ltd.
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
#include "test/bus/common.h"

static DBusTestCase tests[] =
{
  { "expire-list", bus_expire_list_test },
  { "config-parser", bus_config_parser_test },
  { "signals", bus_signals_test },
  { "activation-service-reload", bus_activation_service_reload_test },
  { "unix-fds-passing", bus_unix_fds_passing_test },
  { NULL }
};

int
main (int argc, char **argv)
{
  return bus_test_main (argc, argv, _DBUS_N_ELEMENTS (tests), tests);
}
