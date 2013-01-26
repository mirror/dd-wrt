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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "qmicli-helpers.h"

gchar *
qmicli_get_raw_data_printable (const GArray *data,
                               gsize max_line_length,
                               const gchar *line_prefix)
{
	gsize i;
	gsize j;
    gsize k;
	gsize new_str_length;
	gchar *new_str;
    gsize prefix_len;
    guint n_lines;
    gboolean is_new_line;

    g_return_val_if_fail (max_line_length >= 3, NULL);

    if (!data)
        return g_strdup ("");

	/* Get new string length. If input string has N bytes, we need:
	 * - 1 byte for last NUL char
	 * - 2N bytes for hexadecimal char representation of each byte...
	 * - N-1 bytes for the separator ':'
	 * So... a total of (1+2N+N-1) = 3N bytes are needed... */
	new_str_length =  3 * data->len;

    /* Effective max line length needs to be multiple of 3, we don't want to
     * split in half a given byte representation */
    while (max_line_length % 3 != 0)
        max_line_length--;

    /* Prefix len includes the newline character plus the length of the input
     * prefix */
    prefix_len = strlen (line_prefix) + 1;
    /* We don't consider the last NUL byte when counting lines to generate */
    n_lines = (new_str_length - 1) / max_line_length;
    if ((new_str_length - 1) % max_line_length != 0)
        n_lines++;

    /* Build new str length expected when we prefix the string and we limit the
     * line length */
    new_str_length += (n_lines * prefix_len);

	/* Allocate memory for new array and initialize contents to NUL */
	new_str = g_malloc0 (new_str_length);

	/* Print hexadecimal representation of each byte... */
    is_new_line = TRUE;
	for (i = 0, j = 0, k = 0; i < data->len; i++) {
        if (is_new_line) {
            strcpy (&new_str[j], line_prefix);
            j += strlen (line_prefix);
            is_new_line = FALSE;
        }

		/* Print character in output string... */
		snprintf (&new_str[j], 3, "%02X", g_array_index (data, guint8, i));
        j+=2;
        k+=2;

		if (i != (data->len - 1) ) {
			new_str[j] = ':';
            j++;
            k++;
        }

        if (k % max_line_length == 0 ||
            i == (data->len -1)) {
            new_str[j] = '\n';
            j++;
            is_new_line = TRUE;
        }
	}

	/* Set output string */
	return new_str;
}

gboolean
qmicli_read_pin_id_from_string (const gchar *str,
                                QmiDmsUimPinId *out)
{
    if (!str || str[0] == '\0') {
        g_printerr ("error: expected 'PIN' or 'PIN2', got: none\n");
        return FALSE;
    }

    if (g_str_equal (str, "PIN")) {
        *out = QMI_DMS_UIM_PIN_ID_PIN;
        return TRUE;
    }

    if (g_str_equal (str, "PIN2")) {
        *out = QMI_DMS_UIM_PIN_ID_PIN2;
        return TRUE;
    }

    g_printerr ("error: expected 'PIN' or 'PIN2', got: '%s'\n", str);
    return FALSE;
}

gboolean
qmicli_read_operating_mode_from_string (const gchar *str,
                                        QmiDmsOperatingMode *out)
{
    GType type;
    GEnumClass *enum_class;
    GEnumValue *enum_value;

    type = qmi_dms_operating_mode_get_type ();
    enum_class = G_ENUM_CLASS (g_type_class_ref (type));
    enum_value = g_enum_get_value_by_nick (enum_class, str);

    if (enum_value)
        *out = (QmiDmsOperatingMode)enum_value->value;
    else
        g_printerr ("error: invalid operating mode value given: '%s'\n", str);

    g_type_class_unref (enum_class);
    return !!enum_value;
}

gboolean
qmicli_read_facility_from_string (const gchar *str,
                                  QmiDmsUimFacility *out)
{
    GType type;
    GEnumClass *enum_class;
    GEnumValue *enum_value;

    type = qmi_dms_uim_facility_get_type ();
    enum_class = G_ENUM_CLASS (g_type_class_ref (type));
    enum_value = g_enum_get_value_by_nick (enum_class, str);

    if (enum_value)
        *out = (QmiDmsUimFacility)enum_value->value;
    else
        g_printerr ("error: invalid facility value given: '%s'\n", str);

    g_type_class_unref (enum_class);
    return !!enum_value;
}

gboolean
qmicli_read_enable_disable_from_string (const gchar *str,
                                        gboolean *out)
{
    if (!str || str[0] == '\0') {
        g_printerr ("error: expected 'disable' or 'enable', got: none\n");
        return FALSE;
    }

    if (g_str_equal (str, "disable")) {
        *out = FALSE;
        return TRUE;
    }

    if (g_str_equal (str, "enable")) {
        *out = TRUE;
        return TRUE;
    }

    g_printerr ("error: expected 'disable' or 'enable', got: '%s'\n", str);
    return FALSE;
}

gboolean
qmicli_read_non_empty_string (const gchar *str,
                              const gchar *description,
                              gchar **out)
{
    if (!str || str[0] == '\0') {
        g_printerr ("error: empty %s given\n", description);
        return FALSE;
    }

    *out = (gchar *)str;
    return TRUE;
}

gboolean
qmicli_read_firmware_id_from_string (const gchar *str,
                                     QmiDmsFirmwareImageType *out_type,
                                     guint *out_index)
{
    const gchar *index_str;

    if (g_str_has_prefix (str, "modem")) {
        *out_type = QMI_DMS_FIRMWARE_IMAGE_TYPE_MODEM;
        index_str = &str[5];
    } else if (g_str_has_prefix (str, "pri")) {
        *out_type = QMI_DMS_FIRMWARE_IMAGE_TYPE_PRI;
        index_str = &str[3];
    } else {
        g_printerr ("error: invalid firmware image type value given: '%s'\n", str);
        return FALSE;
    }

    return qmicli_read_uint_from_string (index_str, out_index);
}

gboolean
qmicli_read_uint_from_string (const gchar *str,
                              guint *out)
{
    gulong num;

    if (!str || !str[0])
        return FALSE;

    for (num = 0; str[num]; num++) {
        if (!g_ascii_isdigit (str[num]))
            return FALSE;
    }

    errno = 0;
    num = strtoul (str, NULL, 10);
    if (!errno && num <= G_MAXUINT) {
        *out = (guint)num;
        return TRUE;
    }
    return FALSE;
}
