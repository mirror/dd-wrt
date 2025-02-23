/* -*- mode: C; c-file-style: "gnu"; indent-tabs-mode: nil; -*- */
/*
 * Copyright © 2013 Intel Corporation
 * SPDX-License-Identifier: AFL-2.1 OR GPL-2.0-or-later
 *
 * Do not include other private headers in this one, particularly
 * dbus-sysdeps.h: it gets included into C++ code which is not
 * compatible with our use of <stdatomic.h>.
 */

#ifndef DBUS_INIT_WIN_H
#define DBUS_INIT_WIN_H

void        _dbus_threads_windows_init_global (void);
void        _dbus_threads_windows_ensure_ctor_linked (void);

#endif
