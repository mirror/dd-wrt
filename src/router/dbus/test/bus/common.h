/*
 * Copyright 2022 Collabora Ltd.
 * SPDX-License-Identifier: MIT
 */

#ifndef TEST_BUS_COMMON_H
#define TEST_BUS_COMMON_H

#ifndef DBUS_ENABLE_EMBEDDED_TESTS
#error This file is only relevant for the embedded tests
#endif

#include "bus/test.h"
#include "test/test-utils.h"

int bus_test_main (int                  argc,
                   char               **argv,
                   size_t               n_tests,
                   const DBusTestCase  *tests);

#endif
