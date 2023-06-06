/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * Copyright 2003-2009 Red Hat, Inc.
 * Copyright 2011-2022 Collabora Ltd.
 * SPDX-License-Identifier: AFL-2.1 or GPL-2.0-or-later
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
 */

#include <config.h>

#include "bus/audit.h"
#include "bus/selinux.h"

#include "test/bus/common.h"
#include "test/test-utils.h"

#ifndef DBUS_ENABLE_EMBEDDED_TESTS
#error This file is only relevant for the embedded tests
#endif

static void
test_pre_hook (void)
{
}

static void
test_post_hook (void)
{
  if (_dbus_getenv ("DBUS_TEST_SELINUX"))
    bus_selinux_shutdown ();

  bus_audit_shutdown ();
}

int
bus_test_main (int                  argc,
               char               **argv,
               size_t               n_tests,
               const DBusTestCase  *tests)
{
  return _dbus_test_main (argc, argv, n_tests, tests,
                          (DBUS_TEST_FLAGS_CHECK_MEMORY_LEAKS |
                           DBUS_TEST_FLAGS_CHECK_FD_LEAKS |
                           DBUS_TEST_FLAGS_REQUIRE_DATA),
                          test_pre_hook, test_post_hook);
}
