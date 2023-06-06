/*
 * Copyright © 2010-2015 Ralf Habacker
 * Copyright © 2015-2019 Collabora Ltd.
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
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifdef DBUS_INSIDE_DBUS_H
#error "You can't include dbus-macros-internal.h in the public header dbus.h"
#endif

#ifndef DBUS_MACROS_INTERNAL_H
#define DBUS_MACROS_INTERNAL_H

#include <dbus/dbus-macros.h>

#ifdef DBUS_ENABLE_EMBEDDED_TESTS
# define DBUS_EMBEDDED_TESTS_EXPORT DBUS_PRIVATE_EXPORT
#else
# define DBUS_EMBEDDED_TESTS_EXPORT /* nothing */
#endif

#if defined(DBUS_PRIVATE_EXPORT)
  /* value forced by compiler command line, don't redefine */
#elif defined(_WIN32)
#  if defined(DBUS_STATIC_BUILD)
#    define DBUS_PRIVATE_EXPORT /* no decoration */
#  elif defined(dbus_1_EXPORTS)
#    define DBUS_PRIVATE_EXPORT __declspec(dllexport)
#  else
#    define DBUS_PRIVATE_EXPORT __declspec(dllimport)
#  endif
#elif defined(__GNUC__) && __GNUC__ >= 4
#  define DBUS_PRIVATE_EXPORT __attribute__ ((__visibility__ ("default")))
#else
#  define DBUS_PRIVATE_EXPORT /* no decoration */
#endif

#endif
