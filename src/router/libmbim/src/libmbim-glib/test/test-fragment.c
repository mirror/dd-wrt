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
 * Copyright (C) 2013 - 2014 Aleksander Morgado <aleksander@aleksander.es>
 */

#include <config.h>
#include <string.h>

#include "mbim-message.h"
#include "mbim-message-private.h"

static void
test_fragment_receive_single (void)
{
    MbimMessage *message;
    const guint8 *fragment_information_buffer;
    guint32 fragment_information_buffer_length;

    /* This buffer contains a single message composed of a single fragment.
     * We don't really care about the actual data included within the fragment. */
    const guint8 buffer [] =  {
        0x07, 0x00, 0x00, 0x80, /* indications have fragments */
        0x1C, 0x00, 0x00, 0x00, /* length of this fragment */
        0x01, 0x00, 0x00, 0x00, /* transaction id */
        0x01, 0x00, 0x00, 0x00, /* total fragments */
        0x00, 0x00, 0x00, 0x00, /* current fragment */
        0x01, 0x02, 0x03, 0x04, /* frament data */
        0x05, 0x06, 0x07, 0x08 };

    const guint8 data [] = {
        0x01, 0x02, 0x03, 0x04, /* same data as in the fragment */
        0x05, 0x06, 0x07, 0x08
    };

    message = mbim_message_new (buffer, sizeof (buffer));
    g_assert         (_mbim_message_is_fragment          (message));
    g_assert_cmpuint (_mbim_message_fragment_get_total   (message), ==, 1);
    g_assert_cmpuint (_mbim_message_fragment_get_current (message), ==, 0);

    fragment_information_buffer = (_mbim_message_fragment_get_payload (
                                       message,
                                       &fragment_information_buffer_length));

    g_assert_cmpuint (fragment_information_buffer_length, ==, sizeof (data));
    g_assert (memcmp (fragment_information_buffer, data, fragment_information_buffer_length) == 0);

    mbim_message_unref (message);
}

static void
test_fragment_receive_multiple (void)
{
    GByteArray *bytearray;
    MbimMessage *message;
    GError *error = NULL;
    const guint8 *fragment_information_buffer;
    guint32 fragment_information_buffer_length;

    /* This buffer contains several fragments of a single message.
     * We don't really care about the actual data included within the fragments. */
    const guint8 buffer [] =  {
        /* First fragment */
        0x07, 0x00, 0x00, 0x80, /* indications have fragments */
        0x1C, 0x00, 0x00, 0x00, /* length of this fragment */
        0x01, 0x00, 0x00, 0x00, /* transaction id */
        0x04, 0x00, 0x00, 0x00, /* total fragments */
        0x00, 0x00, 0x00, 0x00, /* current fragment */
        0x00, 0x01, 0x02, 0x03, /* frament data */
        0x04, 0x05, 0x06, 0x07,
        /* Second fragment */
        0x07, 0x00, 0x00, 0x80, /* indications have fragments */
        0x1C, 0x00, 0x00, 0x00, /* length of this fragment */
        0x01, 0x00, 0x00, 0x00, /* transaction id */
        0x04, 0x00, 0x00, 0x00, /* total fragments */
        0x01, 0x00, 0x00, 0x00, /* current fragment */
        0x08, 0x09, 0x0A, 0x0B, /* frament data */
        0x0C, 0x0D, 0x0E, 0x0F,
        /* Third fragment */
        0x07, 0x00, 0x00, 0x80, /* indications have fragments */
        0x1C, 0x00, 0x00, 0x00, /* length of this fragment */
        0x01, 0x00, 0x00, 0x00, /* transaction id */
        0x04, 0x00, 0x00, 0x00, /* total fragments */
        0x02, 0x00, 0x00, 0x00, /* current fragment */
        0x10, 0x11, 0x12, 0x13, /* frament data */
        0x14, 0x15, 0x16, 0x17,
        /* Fourth fragment */
        0x07, 0x00, 0x00, 0x80, /* indications have fragments */
        0x18, 0x00, 0x00, 0x00, /* length of this fragment */
        0x01, 0x00, 0x00, 0x00, /* transaction id */
        0x04, 0x00, 0x00, 0x00, /* total fragments */
        0x03, 0x00, 0x00, 0x00, /* current fragment */
        0x18, 0x19, 0x1A, 0x1B  /* frament data */
    };

    const guint8 data [] = {
        0x00, 0x01, 0x02, 0x03, /* same data as in the fragments */
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B,
        0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13,
        0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1A, 0x1B
    };

    bytearray = g_byte_array_new ();
    g_byte_array_append (bytearray, buffer, sizeof (buffer));

    /* First fragment creates the message */
    message = _mbim_message_fragment_collector_init ((const MbimMessage *)bytearray, &error);
    g_assert_no_error (error);
    g_assert_cmpuint (_mbim_message_fragment_get_total   (message), ==, 4);
    g_assert_cmpuint (_mbim_message_fragment_get_current (message), ==, 0);
    g_assert         (_mbim_message_fragment_collector_complete (message) == FALSE);
    g_byte_array_remove_range (bytearray, 0, mbim_message_get_message_length ((const MbimMessage *)bytearray));


    /* Add second fragment */
    g_assert (_mbim_message_fragment_collector_add (message, (const MbimMessage *)bytearray, &error));
    g_assert_no_error (error);
    g_assert_cmpuint (_mbim_message_fragment_get_total   (message), ==, 4);
    g_assert_cmpuint (_mbim_message_fragment_get_current (message), ==, 1);
    g_assert         (_mbim_message_fragment_collector_complete (message) == FALSE);
    g_byte_array_remove_range (bytearray, 0, mbim_message_get_message_length ((const MbimMessage *)bytearray));

    /* Add third fragment */
    g_assert (_mbim_message_fragment_collector_add (message, (const MbimMessage *)bytearray, &error));
    g_assert_no_error (error);
    g_assert_cmpuint (_mbim_message_fragment_get_total   (message), ==, 4);
    g_assert_cmpuint (_mbim_message_fragment_get_current (message), ==, 2);
    g_assert         (_mbim_message_fragment_collector_complete (message) == FALSE);
    g_byte_array_remove_range (bytearray, 0, mbim_message_get_message_length ((const MbimMessage *)bytearray));

    /* Add fourth fragment */
    g_assert (_mbim_message_fragment_collector_add (message, (const MbimMessage *)bytearray, &error));
    g_assert_no_error (error);
    g_assert_cmpuint (_mbim_message_fragment_get_total   (message), ==, 4);
    g_assert_cmpuint (_mbim_message_fragment_get_current (message), ==, 3);
    g_assert         (_mbim_message_fragment_collector_complete (message) == TRUE);
    g_assert_cmpuint (_mbim_message_fragment_get_total   (message), ==, 1);
    g_assert_cmpuint (_mbim_message_fragment_get_current (message), ==, 0);
    g_byte_array_remove_range (bytearray, 0, mbim_message_get_message_length ((const MbimMessage *)bytearray));

    /* Compare all compiled data */
    fragment_information_buffer = (_mbim_message_fragment_get_payload (
                                       message,
                                       &fragment_information_buffer_length));
    g_assert_cmpuint (fragment_information_buffer_length, ==, sizeof (data));
    g_assert (memcmp (fragment_information_buffer, data, fragment_information_buffer_length) == 0);

    mbim_message_unref (message);
}

static void
test_fragment_send_multiple_common (guint32       max_fragment_size,
                                    const guint8 *buffer,
                                    gsize         buffer_length,
                                    const guint8 *expected_buffer,
                                    gsize         expected_buffer_length)
{
    GByteArray *bytearray;
    struct fragment_info *fragments;
    guint n_fragments;
    guint i;
    GByteArray *output_bytearray;

    /* Setup message and split it into fragments */
    bytearray = g_byte_array_new ();
    g_byte_array_append (bytearray, buffer, buffer_length);
    fragments = _mbim_message_split_fragments ((const MbimMessage *)bytearray,
                                               max_fragment_size,
                                               &n_fragments);
    g_assert (fragments != NULL);

    /* Simulate that we write the fragments */
    output_bytearray = g_byte_array_new ();
    for (i = 0; i < n_fragments; i++) {
        /* Header */
        g_byte_array_append (output_bytearray,
                             (guint8 *)&fragments[i].header,
                             sizeof (fragments[i].header));
        /* Fragment header */
        g_byte_array_append (output_bytearray,
                             (guint8 *)&fragments[i].fragment_header,
                             sizeof (fragments[i].fragment_header));
        /* Data */
        g_byte_array_append (output_bytearray,
                             fragments[i].data,
                             fragments[i].data_length);
    }
    g_free (fragments);

    /* Compare with the expected buffer */
    g_assert_cmpuint (expected_buffer_length, == , output_bytearray->len);
    g_assert (memcmp (output_bytearray->data, expected_buffer, output_bytearray->len) == 0);

    g_byte_array_unref (output_bytearray);
}

static void
test_fragment_send_multiple_1 (void)
{
    const guint8 buffer [] = {
        0x07, 0x00, 0x00, 0x80, /* indications have fragments */
        0x30, 0x00, 0x00, 0x00, /* length of this fragment */
        0x01, 0x00, 0x00, 0x00, /* transaction id */
        0x01, 0x00, 0x00, 0x00, /* total fragments */
        0x00, 0x00, 0x00, 0x00, /* current fragment */
        0x00, 0x01, 0x02, 0x03, /* same data as in the fragments */
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B,
        0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13,
        0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1A, 0x1B
    };

    /* This buffer contains several fragments of a single message.
     * We don't really care about the actual data included within the fragments. */
    const guint8 expected_buffer [] = {
        /* First fragment */
        0x07, 0x00, 0x00, 0x80, /* indications have fragments */
        0x1C, 0x00, 0x00, 0x00, /* length of this fragment */
        0x01, 0x00, 0x00, 0x00, /* transaction id */
        0x04, 0x00, 0x00, 0x00, /* total fragments */
        0x00, 0x00, 0x00, 0x00, /* current fragment */
        0x00, 0x01, 0x02, 0x03, /* frament data */
        0x04, 0x05, 0x06, 0x07,
        /* Second fragment */
        0x07, 0x00, 0x00, 0x80, /* indications have fragments */
        0x1C, 0x00, 0x00, 0x00, /* length of this fragment */
        0x01, 0x00, 0x00, 0x00, /* transaction id */
        0x04, 0x00, 0x00, 0x00, /* total fragments */
        0x01, 0x00, 0x00, 0x00, /* current fragment */
        0x08, 0x09, 0x0A, 0x0B, /* frament data */
        0x0C, 0x0D, 0x0E, 0x0F,
        /* Third fragment */
        0x07, 0x00, 0x00, 0x80, /* indications have fragments */
        0x1C, 0x00, 0x00, 0x00, /* length of this fragment */
        0x01, 0x00, 0x00, 0x00, /* transaction id */
        0x04, 0x00, 0x00, 0x00, /* total fragments */
        0x02, 0x00, 0x00, 0x00, /* current fragment */
        0x10, 0x11, 0x12, 0x13, /* frament data */
        0x14, 0x15, 0x16, 0x17,
        /* Fourth fragment */
        0x07, 0x00, 0x00, 0x80, /* indications have fragments */
        0x18, 0x00, 0x00, 0x00, /* length of this fragment */
        0x01, 0x00, 0x00, 0x00, /* transaction id */
        0x04, 0x00, 0x00, 0x00, /* total fragments */
        0x03, 0x00, 0x00, 0x00, /* current fragment */
        0x18, 0x19, 0x1A, 0x1B  /* frament data */
    };

    test_fragment_send_multiple_common (
        28,
        buffer, sizeof (buffer),
        expected_buffer, sizeof (expected_buffer));
}

static void
test_fragment_send_multiple_2 (void)
{
    const guint8 buffer [] = {
        0x07, 0x00, 0x00, 0x80, /* indications have fragments */
        0x2C, 0x00, 0x00, 0x00, /* length of this fragment */
        0x01, 0x00, 0x00, 0x00, /* transaction id */
        0x01, 0x00, 0x00, 0x00, /* total fragments */
        0x00, 0x00, 0x00, 0x00, /* current fragment */
        0x00, 0x01, 0x02, 0x03, /* same data as in the fragments */
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B,
        0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13,
        0x14, 0x15, 0x16, 0x17
    };

    /* This buffer contains several fragments of a single message.
     * We don't really care about the actual data included within the fragments. */
    const guint8 expected_buffer [] = {
        /* First fragment */
        0x07, 0x00, 0x00, 0x80, /* indications have fragments */
        0x1C, 0x00, 0x00, 0x00, /* length of this fragment */
        0x01, 0x00, 0x00, 0x00, /* transaction id */
        0x03, 0x00, 0x00, 0x00, /* total fragments */
        0x00, 0x00, 0x00, 0x00, /* current fragment */
        0x00, 0x01, 0x02, 0x03, /* frament data */
        0x04, 0x05, 0x06, 0x07,
        /* Second fragment */
        0x07, 0x00, 0x00, 0x80, /* indications have fragments */
        0x1C, 0x00, 0x00, 0x00, /* length of this fragment */
        0x01, 0x00, 0x00, 0x00, /* transaction id */
        0x03, 0x00, 0x00, 0x00, /* total fragments */
        0x01, 0x00, 0x00, 0x00, /* current fragment */
        0x08, 0x09, 0x0A, 0x0B, /* frament data */
        0x0C, 0x0D, 0x0E, 0x0F,
        /* Third fragment */
        0x07, 0x00, 0x00, 0x80, /* indications have fragments */
        0x1C, 0x00, 0x00, 0x00, /* length of this fragment */
        0x01, 0x00, 0x00, 0x00, /* transaction id */
        0x03, 0x00, 0x00, 0x00, /* total fragments */
        0x02, 0x00, 0x00, 0x00, /* current fragment */
        0x10, 0x11, 0x12, 0x13, /* frament data */
        0x14, 0x15, 0x16, 0x17
    };

    test_fragment_send_multiple_common (
        28,
        buffer, sizeof (buffer),
        expected_buffer, sizeof (expected_buffer));
}

int main (int argc, char **argv)
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/libmbim-glib/fragment/receive/single",   test_fragment_receive_single);
    g_test_add_func ("/libmbim-glib/fragment/receive/multiple", test_fragment_receive_multiple);
    g_test_add_func ("/libmbim-glib/fragment/send/multiple-1",  test_fragment_send_multiple_1);
    g_test_add_func ("/libmbim-glib/fragment/send/multiple-2",  test_fragment_send_multiple_2);

    return g_test_run ();
}
