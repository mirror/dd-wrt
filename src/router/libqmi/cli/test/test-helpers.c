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

#include <glib.h>
#include "qmicli-helpers.h"

static void
test_helpers_raw_printable_1 (void)
{
    GArray *array;
    gchar *printable;
    static guint8 buffer[8] = {
        0x0F, 0x50, 0xEB, 0xE2, 0xB6, 0x00, 0x00, 0x00
    };
    static const gchar *expected =
        "0F:\n"
        "50:\n"
        "EB:\n"
        "E2:\n"
        "B6:\n"
        "00:\n"
        "00:\n"
        "00\n";

    array = g_array_sized_new (FALSE, FALSE, 1, 8);
    g_array_insert_vals (array, 0, buffer, 8);

    printable = qmicli_get_raw_data_printable (array, 3, "");

    g_assert_cmpstr (printable, ==, expected);
    g_free (printable);
    g_array_unref (array);
}

static void
test_helpers_raw_printable_2 (void)
{
    GArray *array;
    gchar *printable;
    static guint8 buffer[8] = {
        0x0F, 0x50, 0xEB, 0xE2, 0xB6, 0x00, 0x00, 0x00
    };
    static const gchar *expected =
        "\t0F:50:\n"
        "\tEB:E2:\n"
        "\tB6:00:\n"
        "\t00:00\n";

    array = g_array_sized_new (FALSE, FALSE, 1, 8);
    g_array_insert_vals (array, 0, buffer, 8);

    /* When passing 7, we'll be really getting 6 (the closest lower multiple of 3) */
    printable = qmicli_get_raw_data_printable (array, 7, "\t");

    g_assert_cmpstr (printable, ==, expected);
    g_free (printable);
    g_array_unref (array);
}

static void
test_helpers_raw_printable_3 (void)
{
    GArray *array;
    gchar *printable;
    static guint8 buffer[8] = {
        0x0F, 0x50, 0xEB, 0xE2, 0xB6, 0x00, 0x00, 0x00
    };
    static const gchar *expected =
        "\t\t\t0F:50:EB:E2:\n"
        "\t\t\tB6:00:00:00\n";

    array = g_array_sized_new (FALSE, FALSE, 1, 8);
    g_array_insert_vals (array, 0, buffer, 8);

    printable = qmicli_get_raw_data_printable (array, 12, "\t\t\t");

    g_assert_cmpstr (printable, ==, expected);
    g_free (printable);
    g_array_unref (array);
}

static void
test_helpers_raw_printable_4 (void)
{
    GArray *array;
    gchar *printable;
    static guint8 buffer[8] = {
        0x0F, 0x50, 0xEB, 0xE2, 0xB6, 0x00, 0x00, 0x00
    };
    static const gchar *expected =
        "\t0F:50:EB:E2:B6:00:00:00\n";

    array = g_array_sized_new (FALSE, FALSE, 1, 8);
    g_array_insert_vals (array, 0, buffer, 8);

    printable = qmicli_get_raw_data_printable (array, 24, "\t");

    g_assert_cmpstr (printable, ==, expected);
    g_free (printable);
    g_array_unref (array);
}

int main (int argc, char **argv)
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/qmicli/helpers/raw-printable/1",  test_helpers_raw_printable_1);
    g_test_add_func ("/qmicli/helpers/raw-printable/2",  test_helpers_raw_printable_2);
    g_test_add_func ("/qmicli/helpers/raw-printable/3",  test_helpers_raw_printable_3);
    g_test_add_func ("/qmicli/helpers/raw-printable/4",  test_helpers_raw_printable_4);

    return g_test_run ();
}
