/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * qmicli -- Command line interface to control QMI devices
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
 * Copyright (C) 2012 Aleksander Morgado <aleksander@gnu.org>
 */

#include <glib.h>

#include <libqmi-glib.h>

#ifndef __QMICLI_HELPERS_H__
#define __QMICLI_HELPERS_H__

gchar *qmicli_get_raw_data_printable (const GArray *data,
                                      gsize max_line_length,
                                      const gchar *new_line_prefix);

gboolean qmicli_read_pin_id_from_string         (const gchar *str,
                                                 QmiDmsUimPinId *out);
gboolean qmicli_read_operating_mode_from_string (const gchar *str,
                                                 QmiDmsOperatingMode *out);
gboolean qmicli_read_facility_from_string       (const gchar *str,
                                                 QmiDmsUimFacility *out);
gboolean qmicli_read_enable_disable_from_string (const gchar *str,
                                                 gboolean *out);
gboolean qmicli_read_firmware_id_from_string    (const gchar *str,
                                                 QmiDmsFirmwareImageType *out_type,
                                                 guint *out_index);
gboolean qmicli_read_non_empty_string           (const gchar *str,
                                                 const gchar *description,
                                                 gchar **out);
gboolean qmicli_read_uint_from_string           (const gchar *str,
                                                 guint *out);

#endif /* __QMICLI_H__ */
