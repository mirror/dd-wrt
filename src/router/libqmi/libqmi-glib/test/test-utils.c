/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details:
 *
 * Copyright (C) 2012 Aleksander Morgado <aleksander@gnu.org>
 */

#include <glib-object.h>
#include <string.h>
#include "qmi-utils.h"

#define VAL_EQUAL(t, v, c, e) \
{ \
    if (e == QMI_ENDIAN_LITTLE) \
        g_assert_cmpint (c##_FROM_LE (t), ==, v); \
    else \
        g_assert_cmpint (c##_FROM_BE (t), ==, v); \
}

static void
test_utils_uint8 (void)
{
    static const guint8 in_buffer[8] = {
        0x0F, 0x50, 0xEB, 0xE2, 0xB6, 0x00, 0x00, 0x00
    };
    guint8 out_buffer[8] = { 0 };

    guint16 in_buffer_size;
    const guint8 *in_buffer_walker;
    guint16 out_buffer_size;
    guint8 *out_buffer_walker;

    in_buffer_size = sizeof (in_buffer);
    in_buffer_walker = &in_buffer[0];
    out_buffer_size = sizeof (out_buffer);
    out_buffer_walker = &out_buffer[0];

    while (in_buffer_size) {
        guint8 tmp;

        qmi_utils_read_guint8_from_buffer (&in_buffer_walker, &in_buffer_size, &tmp);
        qmi_utils_write_guint8_to_buffer (&out_buffer_walker, &out_buffer_size, &tmp);
    }

    g_assert_cmpuint (out_buffer_size, ==, 0);
    g_assert (memcmp (in_buffer, out_buffer, sizeof (in_buffer)) == 0);
}

static void
test_utils_int8 (void)
{
    static const guint8 in_buffer[8] = {
        0x0F, 0x50, 0xEB, 0xE2, 0xB6, 0x00, 0x00, 0x00
    };
    guint8 out_buffer[8] = { 0 };

    guint16 in_buffer_size;
    const guint8 *in_buffer_walker;
    guint16 out_buffer_size;
    guint8 *out_buffer_walker;

    in_buffer_size = sizeof (in_buffer);
    in_buffer_walker = &in_buffer[0];
    out_buffer_size = sizeof (out_buffer);
    out_buffer_walker = &out_buffer[0];

    while (in_buffer_size) {
        gint8 tmp;

        qmi_utils_read_gint8_from_buffer (&in_buffer_walker, &in_buffer_size, &tmp);
        qmi_utils_write_gint8_to_buffer (&out_buffer_walker, &out_buffer_size, &tmp);
    }

    g_assert_cmpuint (out_buffer_size, ==, 0);
    g_assert (memcmp (in_buffer, out_buffer, sizeof (in_buffer)) == 0);
}

static void
test_utils_uint16 (gconstpointer user_data)
{
    static const guint8 in_buffer[8] = {
        0x0F, 0x50, 0xEB, 0xE2, 0xB6, 0x00, 0x00, 0x00
    };
    static const guint16 values[4] = {
        0x500F, 0xE2EB, 0x00B6, 0x0000
    };
    guint8 out_buffer[8] = { 0 };
    QmiEndian endian = (QmiEndian) user_data;

    guint i;
    guint16 in_buffer_size;
    const guint8 *in_buffer_walker;
    guint16 out_buffer_size;
    guint8 *out_buffer_walker;

    in_buffer_size = sizeof (in_buffer);
    in_buffer_walker = &in_buffer[0];
    out_buffer_size = sizeof (out_buffer);
    out_buffer_walker = &out_buffer[0];
    i = 0;

    while (in_buffer_size) {
        guint16 tmp;

        qmi_utils_read_guint16_from_buffer (&in_buffer_walker, &in_buffer_size, endian, &tmp);

        VAL_EQUAL (tmp, values[i++], GUINT16, endian);

        qmi_utils_write_guint16_to_buffer (&out_buffer_walker, &out_buffer_size, endian, &tmp);
    }

    g_assert_cmpuint (out_buffer_size, ==, 0);
    g_assert (memcmp (in_buffer, out_buffer, sizeof (in_buffer)) == 0);
}

static void
test_utils_int16 (gconstpointer user_data)
{
    static const guint8 in_buffer[8] = {
        0x0F, 0x50, 0xEB, 0xE2, 0xB6, 0x00, 0x00, 0x00
    };
    static const gint16 values[4] = {
        0x500F, 0xE2EB, 0x00B6, 0x0000
    };
    guint8 out_buffer[8] = { 0 };
    QmiEndian endian = (QmiEndian) user_data;

    guint i;
    guint16 in_buffer_size;
    const guint8 *in_buffer_walker;
    guint16 out_buffer_size;
    guint8 *out_buffer_walker;

    in_buffer_size = sizeof (in_buffer);
    in_buffer_walker = &in_buffer[0];
    out_buffer_size = sizeof (out_buffer);
    out_buffer_walker = &out_buffer[0];
    i = 0;

    while (in_buffer_size) {
        gint16 tmp;

        qmi_utils_read_gint16_from_buffer (&in_buffer_walker, &in_buffer_size, endian, &tmp);

        VAL_EQUAL (tmp, values[i++], GINT16, endian);

        qmi_utils_write_gint16_to_buffer (&out_buffer_walker, &out_buffer_size, endian, &tmp);
    }

    g_assert_cmpuint (out_buffer_size, ==, 0);
    g_assert (memcmp (in_buffer, out_buffer, sizeof (in_buffer)) == 0);
}

static void
test_utils_uint16_unaligned (gconstpointer user_data)
{
    static const guint8 in_buffer[9] = {
        0x00, 0x0F, 0x50, 0xEB, 0xE2, 0xB6, 0x00, 0x00, 0x00
    };
    static guint16 values[4] = {
        0x500F, 0xE2EB, 0x00B6, 0x0000
    };
    guint8 out_buffer[8] = { 0 };
    QmiEndian endian = (QmiEndian) user_data;

    guint i;
    guint16 in_buffer_size;
    const guint8 *in_buffer_walker;
    guint16 out_buffer_size;
    guint8 *out_buffer_walker;

    in_buffer_size = sizeof (in_buffer) - 1;
    in_buffer_walker = &in_buffer[1];
    out_buffer_size = sizeof (out_buffer);
    out_buffer_walker = &out_buffer[0];
    i = 0;

    while (in_buffer_size) {
        guint16 tmp;

        qmi_utils_read_guint16_from_buffer (&in_buffer_walker, &in_buffer_size, endian, &tmp);

        VAL_EQUAL (tmp, values[i++], GUINT16, endian);

        qmi_utils_write_guint16_to_buffer (&out_buffer_walker, &out_buffer_size, endian, &tmp);
    }

    g_assert_cmpuint (out_buffer_size, ==, 0);
    g_assert (memcmp (&in_buffer[1], out_buffer, sizeof (in_buffer) - 1) == 0);
}

static void
test_utils_int16_unaligned (gconstpointer user_data)
{
    static const guint8 in_buffer[9] = {
        0x00, 0x0F, 0x50, 0xEB, 0xE2, 0xB6, 0x00, 0x00, 0x00
    };
    static gint16 values[4] = {
        0x500F, 0xE2EB, 0x00B6, 0x0000
    };
    guint8 out_buffer[8] = { 0 };
    QmiEndian endian = (QmiEndian) user_data;

    guint i;
    guint16 in_buffer_size;
    const guint8 *in_buffer_walker;
    guint16 out_buffer_size;
    guint8 *out_buffer_walker;

    in_buffer_size = sizeof (in_buffer) - 1;
    in_buffer_walker = &in_buffer[1];
    out_buffer_size = sizeof (out_buffer);
    out_buffer_walker = &out_buffer[0];
    i = 0;

    while (in_buffer_size) {
        gint16 tmp;

        qmi_utils_read_gint16_from_buffer (&in_buffer_walker, &in_buffer_size, endian, &tmp);

        VAL_EQUAL (tmp, values[i++], GINT16, endian);

        qmi_utils_write_gint16_to_buffer (&out_buffer_walker, &out_buffer_size, endian, &tmp);
    }

    g_assert_cmpuint (out_buffer_size, ==, 0);
    g_assert (memcmp (&in_buffer[1], out_buffer, sizeof (in_buffer) - 1) == 0);
}

static void
test_utils_uint32 (gconstpointer user_data)
{
    static const guint8 in_buffer[8] = {
        0x0F, 0x50, 0xEB, 0xE2, 0xB6, 0x00, 0x00, 0x00
    };
    static const guint32 values[2] = {
        0xE2EB500F, 0x000000B6
    };
    guint8 out_buffer[8] = { 0 };
    QmiEndian endian = (QmiEndian) user_data;

    guint i;
    guint16 in_buffer_size;
    const guint8 *in_buffer_walker;
    guint16 out_buffer_size;
    guint8 *out_buffer_walker;

    in_buffer_size = sizeof (in_buffer);
    in_buffer_walker = &in_buffer[0];
    out_buffer_size = sizeof (out_buffer);
    out_buffer_walker = &out_buffer[0];
    i = 0;

    while (in_buffer_size) {
        guint32 tmp;

        qmi_utils_read_guint32_from_buffer (&in_buffer_walker, &in_buffer_size, endian, &tmp);

        VAL_EQUAL (tmp, values[i++], GUINT32, endian);

        qmi_utils_write_guint32_to_buffer (&out_buffer_walker, &out_buffer_size, endian, &tmp);
    }

    g_assert_cmpuint (out_buffer_size, ==, 0);
    g_assert (memcmp (in_buffer, out_buffer, sizeof (in_buffer)) == 0);
}

static void
test_utils_int32 (gconstpointer user_data)
{
    static const guint8 in_buffer[8] = {
        0x0F, 0x50, 0xEB, 0xE2, 0xB6, 0x00, 0x00, 0x00
    };
    static const gint32 values[2] = {
        0xE2EB500F, 0x000000B6
    };
    guint8 out_buffer[8] = { 0 };
    QmiEndian endian = (QmiEndian) user_data;

    guint i;
    guint16 in_buffer_size;
    const guint8 *in_buffer_walker;
    guint16 out_buffer_size;
    guint8 *out_buffer_walker;

    in_buffer_size = sizeof (in_buffer);
    in_buffer_walker = &in_buffer[0];
    out_buffer_size = sizeof (out_buffer);
    out_buffer_walker = &out_buffer[0];
    i = 0;

    while (in_buffer_size) {
        gint32 tmp;

        qmi_utils_read_gint32_from_buffer (&in_buffer_walker, &in_buffer_size, endian, &tmp);

        VAL_EQUAL (tmp, values[i++], GINT32, endian);

        qmi_utils_write_gint32_to_buffer (&out_buffer_walker, &out_buffer_size, endian, &tmp);
    }

    g_assert_cmpuint (out_buffer_size, ==, 0);
    g_assert (memcmp (in_buffer, out_buffer, sizeof (in_buffer)) == 0);
}

static void
test_utils_uint32_unaligned (gconstpointer user_data)
{
    static const guint8 in_buffer[9] = {
        0x00, 0x0F, 0x50, 0xEB, 0xE2, 0xB6, 0x00, 0x00, 0x00
    };
    static guint32 values[2] = {
        0xE2EB500F, 0x000000B6
    };
    guint8 out_buffer[8] = { 0 };
    QmiEndian endian = (QmiEndian) user_data;

    guint i;
    guint16 in_buffer_size;
    const guint8 *in_buffer_walker;
    guint16 out_buffer_size;
    guint8 *out_buffer_walker;

    in_buffer_size = sizeof (in_buffer) - 1;
    in_buffer_walker = &in_buffer[1];
    out_buffer_size = sizeof (out_buffer);
    out_buffer_walker = &out_buffer[0];
    i = 0;

    while (in_buffer_size) {
        guint32 tmp;

        qmi_utils_read_guint32_from_buffer (&in_buffer_walker, &in_buffer_size, endian, &tmp);

        VAL_EQUAL (tmp, values[i++], GUINT32, endian);

        qmi_utils_write_guint32_to_buffer (&out_buffer_walker, &out_buffer_size, endian, &tmp);
    }

    g_assert_cmpuint (out_buffer_size, ==, 0);
    g_assert (memcmp (&in_buffer[1], out_buffer, sizeof (in_buffer) - 1) == 0);
}

static void
test_utils_int32_unaligned (gconstpointer user_data)
{
    static const guint8 in_buffer[9] = {
        0x00, 0x0F, 0x50, 0xEB, 0xE2, 0xB6, 0x00, 0x00, 0x00
    };
    static gint32 values[2] = {
        0xE2EB500F, 0x000000B6
    };
    guint8 out_buffer[8] = { 0 };

    guint i;
    guint16 in_buffer_size;
    const guint8 *in_buffer_walker;
    guint16 out_buffer_size;
    guint8 *out_buffer_walker;
    QmiEndian endian = (QmiEndian) user_data;

    in_buffer_size = sizeof (in_buffer) - 1;
    in_buffer_walker = &in_buffer[1];
    out_buffer_size = sizeof (out_buffer);
    out_buffer_walker = &out_buffer[0];
    i = 0;

    while (in_buffer_size) {
        gint32 tmp;

        qmi_utils_read_gint32_from_buffer (&in_buffer_walker, &in_buffer_size, endian, &tmp);

        VAL_EQUAL (tmp, values[i++], GINT32, endian);

        qmi_utils_write_gint32_to_buffer (&out_buffer_walker, &out_buffer_size, endian, &tmp);
    }

    g_assert_cmpuint (out_buffer_size, ==, 0);
    g_assert (memcmp (&in_buffer[1], out_buffer, sizeof (in_buffer) - 1) == 0);
}

static void
test_utils_uint64 (gconstpointer user_data)
{
    static const guint8 in_buffer[8] = {
        0x0F, 0x50, 0xEB, 0xE2, 0xB6, 0x00, 0x00, 0x00
    };
    static const guint64 values[1] = {
        0x000000B6E2EB500FULL
    };
    guint8 out_buffer[8] = { 0 };

    guint i;
    guint16 in_buffer_size;
    const guint8 *in_buffer_walker;
    guint16 out_buffer_size;
    guint8 *out_buffer_walker;
    QmiEndian endian = (QmiEndian) user_data;

    in_buffer_size = sizeof (in_buffer);
    in_buffer_walker = &in_buffer[0];
    out_buffer_size = sizeof (out_buffer);
    out_buffer_walker = &out_buffer[0];
    i = 0;

    while (in_buffer_size) {
        guint64 tmp;

        qmi_utils_read_guint64_from_buffer (&in_buffer_walker, &in_buffer_size, endian, &tmp);

        VAL_EQUAL (tmp, values[i++], GUINT64, endian);

        qmi_utils_write_guint64_to_buffer (&out_buffer_walker, &out_buffer_size, endian, &tmp);
    }

    g_assert_cmpuint (out_buffer_size, ==, 0);
    g_assert (memcmp (in_buffer, out_buffer, sizeof (in_buffer)) == 0);
}

static void
test_utils_int64 (gconstpointer user_data)
{
    static const guint8 in_buffer[8] = {
        0x0F, 0x50, 0xEB, 0xE2, 0xB6, 0x00, 0x00, 0x00
    };
    static const gint64 values[1] = {
        0x000000B6E2EB500FLL
    };
    guint8 out_buffer[8] = { 0 };

    guint i;
    guint16 in_buffer_size;
    const guint8 *in_buffer_walker;
    guint16 out_buffer_size;
    guint8 *out_buffer_walker;
    QmiEndian endian = (QmiEndian) user_data;

    in_buffer_size = sizeof (in_buffer);
    in_buffer_walker = &in_buffer[0];
    out_buffer_size = sizeof (out_buffer);
    out_buffer_walker = &out_buffer[0];
    i = 0;

    while (in_buffer_size) {
        gint64 tmp;

        qmi_utils_read_gint64_from_buffer (&in_buffer_walker, &in_buffer_size, endian, &tmp);

        VAL_EQUAL (tmp, values[i++], GINT64, endian);

        qmi_utils_write_gint64_to_buffer (&out_buffer_walker, &out_buffer_size, endian, &tmp);
    }

    g_assert_cmpuint (out_buffer_size, ==, 0);
    g_assert (memcmp (in_buffer, out_buffer, sizeof (in_buffer)) == 0);
}

static void
test_utils_uint64_unaligned (gconstpointer user_data)
{
    static const guint8 in_buffer[9] = {
        0x00, 0x0F, 0x50, 0xEB, 0xE2, 0xB6, 0x00, 0x00, 0x00
    };
    static guint64 values[1] = {
        0x000000B6E2EB500FULL
    };
    guint8 out_buffer[8] = { 0 };

    guint i;
    guint16 in_buffer_size;
    const guint8 *in_buffer_walker;
    guint16 out_buffer_size;
    guint8 *out_buffer_walker;
    QmiEndian endian = (QmiEndian) user_data;

    in_buffer_size = sizeof (in_buffer) - 1;
    in_buffer_walker = &in_buffer[1];
    out_buffer_size = sizeof (out_buffer);
    out_buffer_walker = &out_buffer[0];
    i = 0;

    while (in_buffer_size) {
        guint64 tmp;

        qmi_utils_read_guint64_from_buffer (&in_buffer_walker, &in_buffer_size, endian, &tmp);

        VAL_EQUAL (tmp, values[i++], GUINT64, endian);

        qmi_utils_write_guint64_to_buffer (&out_buffer_walker, &out_buffer_size, endian, &tmp);
    }

    g_assert_cmpuint (out_buffer_size, ==, 0);
    g_assert (memcmp (&in_buffer[1], out_buffer, sizeof (in_buffer) - 1) == 0);
}

static void
test_utils_int64_unaligned (gconstpointer user_data)
{
    static const guint8 in_buffer[9] = {
        0x00, 0x0F, 0x50, 0xEB, 0xE2, 0xB6, 0x00, 0x00, 0x00
    };
    static gint64 values[1] = {
        0x000000B6E2EB500FLL
    };
    guint8 out_buffer[8] = { 0 };

    guint i;
    guint16 in_buffer_size;
    const guint8 *in_buffer_walker;
    guint16 out_buffer_size;
    guint8 *out_buffer_walker;
    QmiEndian endian = (QmiEndian) user_data;

    in_buffer_size = sizeof (in_buffer) - 1;
    in_buffer_walker = &in_buffer[1];
    out_buffer_size = sizeof (out_buffer);
    out_buffer_walker = &out_buffer[0];
    i = 0;

    while (in_buffer_size) {
        gint64 tmp;

        qmi_utils_read_gint64_from_buffer (&in_buffer_walker, &in_buffer_size, endian, &tmp);

        VAL_EQUAL (tmp, values[i++], GINT64, endian);

        qmi_utils_write_gint64_to_buffer (&out_buffer_walker, &out_buffer_size, endian, &tmp);
    }

    g_assert_cmpuint (out_buffer_size, ==, 0);
    g_assert (memcmp (&in_buffer[1], out_buffer, sizeof (in_buffer) - 1) == 0);
}

static void
common_test_utils_uint_sized (guint n_bytes, QmiEndian endian)
{
    static const guint8 in_buffer[8] = {
        0x0F, 0x50, 0xEB, 0xE2, 0xB6, 0x00, 0x00, 0x00
    };
    guint64 value = 0x000000B6E2EB500FULL;
    guint8 expected_out_buffer[8] = { 0 };
    guint8 out_buffer[8] = { 0 };

    guint64 tmp;
    guint i;
    guint16 in_buffer_size;
    const guint8 *in_buffer_walker;
    guint16 out_buffer_size;
    guint8 *out_buffer_walker;

    /* Build expected intermediate value */
    tmp = 0xFF;
    for (i = 1; i < n_bytes; i++) {
        tmp <<= 8;
        tmp |= 0xFF;
    }
    value &= tmp;

    /* Build expected output buffer */
    memcpy (expected_out_buffer, in_buffer, n_bytes);

    in_buffer_size = sizeof (in_buffer);
    in_buffer_walker = &in_buffer[0];
    out_buffer_size = sizeof (out_buffer);
    out_buffer_walker = &out_buffer[0];
    i = 0;

    qmi_utils_read_sized_guint_from_buffer (&in_buffer_walker, &in_buffer_size, n_bytes, endian, &tmp);
    VAL_EQUAL (tmp, value, GUINT64, endian);
    qmi_utils_write_sized_guint_to_buffer (&out_buffer_walker, &out_buffer_size, n_bytes, endian, &tmp);

    g_assert_cmpuint (out_buffer_size, ==, 8 - n_bytes);
    g_assert (memcmp (expected_out_buffer, out_buffer, sizeof (expected_out_buffer)) == 0);
}

static void
test_utils_uint_sized (gconstpointer user_data)
{
    guint i;

    for (i = 1; i <= 8; i++) {
        common_test_utils_uint_sized (i, (QmiEndian) user_data);
    }
}

static void
common_test_utils_uint_sized_unaligned (guint n_bytes, QmiEndian endian)
{
    static const guint8 in_buffer[9] = {
        0x00, 0x0F, 0x50, 0xEB, 0xE2, 0xB6, 0x00, 0x00, 0x00
    };
    guint64 value = 0x000000B6E2EB500FULL;
    guint8 expected_out_buffer[8] = { 0 };
    guint8 out_buffer[8] = { 0 };

    guint64 tmp;
    guint i;
    guint16 in_buffer_size;
    const guint8 *in_buffer_walker;
    guint16 out_buffer_size;
    guint8 *out_buffer_walker;

    /* Build expected intermediate value */
    tmp = 0xFF;
    for (i = 1; i < n_bytes; i++) {
        tmp <<= 8;
        tmp |= 0xFF;
    }
    value &= tmp;

    /* Build expected output buffer */
    memcpy (expected_out_buffer, &in_buffer[1], n_bytes);

    in_buffer_size = sizeof (in_buffer) - 1;
    in_buffer_walker = &in_buffer[1];
    out_buffer_size = sizeof (out_buffer);
    out_buffer_walker = &out_buffer[0];
    i = 0;

    qmi_utils_read_sized_guint_from_buffer (&in_buffer_walker, &in_buffer_size, n_bytes, endian, &tmp);
    VAL_EQUAL (tmp, value, GUINT64, endian);
    qmi_utils_write_sized_guint_to_buffer (&out_buffer_walker, &out_buffer_size, n_bytes, endian, &tmp);

    g_assert_cmpuint (out_buffer_size, ==, 8 - n_bytes);
    g_assert (memcmp (expected_out_buffer, out_buffer, sizeof (expected_out_buffer)) == 0);
}

static void
test_utils_uint_sized_unaligned (gconstpointer user_data)
{
    guint i;

    for (i = 1; i <= 8; i++) {
        common_test_utils_uint_sized_unaligned (i, (QmiEndian) user_data);
    }
}

#define LE ((gconstpointer) QMI_ENDIAN_LITTLE)
#define BE ((gconstpointer) QMI_ENDIAN_BIG)

int main (int argc, char **argv)
{
    g_type_init ();
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/libqmi-glib/utils/uint8",  test_utils_uint8);
    g_test_add_func ("/libqmi-glib/utils/int8",   test_utils_int8);

    g_test_add_data_func ("/libqmi-glib/utils/uint16-LE",           LE, test_utils_uint16);
    g_test_add_data_func ("/libqmi-glib/utils/uint16-BE",           BE, test_utils_uint16);
    g_test_add_data_func ("/libqmi-glib/utils/int16-LE",            LE, test_utils_int16);
    g_test_add_data_func ("/libqmi-glib/utils/int16-BE",            BE, test_utils_int16);
    g_test_add_data_func ("/libqmi-glib/utils/uint16-unaligned-LE", LE, test_utils_uint16_unaligned);
    g_test_add_data_func ("/libqmi-glib/utils/uint16-unaligned-Be", BE, test_utils_uint16_unaligned);
    g_test_add_data_func ("/libqmi-glib/utils/int16-unaligned-LE",  LE, test_utils_int16_unaligned);
    g_test_add_data_func ("/libqmi-glib/utils/int16-unaligned-BE",  BE, test_utils_int16_unaligned);

    g_test_add_data_func ("/libqmi-glib/utils/uint32-LE",           LE, test_utils_uint32);
    g_test_add_data_func ("/libqmi-glib/utils/uint32-BE",           BE, test_utils_uint32);
    g_test_add_data_func ("/libqmi-glib/utils/int32-LE",            LE, test_utils_int32);
    g_test_add_data_func ("/libqmi-glib/utils/int32-BE",            BE, test_utils_int32);
    g_test_add_data_func ("/libqmi-glib/utils/uint32/unaligned-LE", LE, test_utils_uint32_unaligned);
    g_test_add_data_func ("/libqmi-glib/utils/uint32/unaligned-BE", BE, test_utils_uint32_unaligned);
    g_test_add_data_func ("/libqmi-glib/utils/int32/unaligned-LE",  LE, test_utils_int32_unaligned);
    g_test_add_data_func ("/libqmi-glib/utils/int32/unaligned-BE",  BE, test_utils_int32_unaligned);

    g_test_add_data_func ("/libqmi-glib/utils/uint64-LE",           LE, test_utils_uint64);
    g_test_add_data_func ("/libqmi-glib/utils/uint64-BE",           BE, test_utils_uint64);
    g_test_add_data_func ("/libqmi-glib/utils/int64-LE",            LE, test_utils_int64);
    g_test_add_data_func ("/libqmi-glib/utils/int64-BE",            BE, test_utils_int64);
    g_test_add_data_func ("/libqmi-glib/utils/uint64/unaligned-LE", LE, test_utils_uint64_unaligned);
    g_test_add_data_func ("/libqmi-glib/utils/uint64/unaligned-BE", BE, test_utils_uint64_unaligned);
    g_test_add_data_func ("/libqmi-glib/utils/int64/unaligned-LE",  LE, test_utils_int64_unaligned);
    g_test_add_data_func ("/libqmi-glib/utils/int64/unaligned-BE",  BE, test_utils_int64_unaligned);

    g_test_add_data_func ("/libqmi-glib/utils/uint-sized-LE",           LE, test_utils_uint_sized);
    g_test_add_data_func ("/libqmi-glib/utils/uint-sized-BE",           BE, test_utils_uint_sized);
    g_test_add_data_func ("/libqmi-glib/utils/uint-sized/unaligned-LE", LE, test_utils_uint_sized_unaligned);
    g_test_add_data_func ("/libqmi-glib/utils/uint-sized/unaligned-BE", BE, test_utils_uint_sized_unaligned);

    return g_test_run ();
}
