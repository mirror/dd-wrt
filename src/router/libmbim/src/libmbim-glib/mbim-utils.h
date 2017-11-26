/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * libmbim-glib -- GLib/GIO based library to control MBIM devices
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 * Copyright (C) 2013 - 2014 Aleksander Morgado <aleksander@aleksander.es>
 */

#ifndef _LIBMBIM_GLIB_MBIM_UTILS_H_
#define _LIBMBIM_GLIB_MBIM_UTILS_H_

#if !defined (__LIBMBIM_GLIB_H_INSIDE__) && !defined (LIBMBIM_GLIB_COMPILATION)
#error "Only <libmbim-glib.h> can be included directly."
#endif

#include <glib.h>

G_BEGIN_DECLS

/* Enabling/Disabling traces */
gboolean mbim_utils_get_traces_enabled (void);
void     mbim_utils_set_traces_enabled (gboolean enabled);

/* Other private methods */

#if defined (LIBMBIM_GLIB_COMPILATION)
gchar *__mbim_utils_str_hex (gconstpointer mem,
                             gsize         size,
                             gchar         delimiter);
gboolean __mbim_user_allowed (uid_t uid,
                              GError **error);
#endif

G_END_DECLS

#endif /* _LIBMBIM_GLIB_MBIM_UTILS_H_ */
