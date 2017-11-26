/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * mbimcli -- Command line interface to control QMI devices
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2014 Aleksander Morgado <aleksander@aleksander.es>
 */

#include <glib.h>

#include <libmbim-glib.h>

#ifndef __MBIMCLI_HELPERS_H__
#define __MBIMCLI_HELPERS_H__

gboolean mbimcli_read_uint_from_string (const gchar *str,
                                        guint *out);

gboolean mbimcli_print_ip_config (MbimDevice *device,
                                  MbimMessage *response,
                                  GError **error);

typedef gboolean (*MbimParseKeyValueForeachFn) (const gchar *key,
                                                const gchar *value,
                                                GError **error,
                                                gpointer user_data);

gboolean mbimcli_parse_key_value_string (const gchar *str,
                                         GError **error,
                                         MbimParseKeyValueForeachFn callback,
                                         gpointer user_data);

#endif /* __MBIMCLI_H__ */
