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
#include "mbim-cid.h"

static void
test_message_open (void)
{
    MbimMessage *message;

    message = mbim_message_open_new (12345, 4096);
    g_assert (message != NULL);

    g_assert_cmpuint (mbim_message_get_transaction_id            (message), ==, 12345);
    g_assert_cmpuint (mbim_message_get_message_type              (message), ==, MBIM_MESSAGE_TYPE_OPEN);
    g_assert_cmpuint (mbim_message_get_message_length            (message), ==, 16);
    g_assert_cmpuint (mbim_message_open_get_max_control_transfer (message), ==, 4096);

    mbim_message_unref (message);
}

static void
test_message_open_done (void)
{
    MbimMessage *message;
    const guint8 buffer [] =  { 0x01, 0x00, 0x00, 0x80,
                                0x10, 0x00, 0x00, 0x00,
                                0x01, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00 };

    message = mbim_message_new (buffer, sizeof (buffer));

    g_assert_cmpuint (mbim_message_get_transaction_id        (message), ==, 1);
    g_assert_cmpuint (mbim_message_get_message_type          (message), ==, MBIM_MESSAGE_TYPE_OPEN_DONE);
    g_assert_cmpuint (mbim_message_get_message_length        (message), ==, 16);
    g_assert_cmpuint (mbim_message_open_done_get_status_code (message), ==, MBIM_STATUS_ERROR_NONE);

    mbim_message_unref (message);
}

static void
test_message_close (void)
{
    MbimMessage *message;

    message = mbim_message_close_new (12345);
    g_assert (message != NULL);

    g_assert_cmpuint (mbim_message_get_transaction_id (message), ==, 12345);
    g_assert_cmpuint (mbim_message_get_message_type   (message), ==, MBIM_MESSAGE_TYPE_CLOSE);
    g_assert_cmpuint (mbim_message_get_message_length (message), ==, 12);

    mbim_message_unref (message);
}

static void
test_message_close_done (void)
{
    MbimMessage *message;
    const guint8 buffer [] =  { 0x02, 0x00, 0x00, 0x80,
                                0x10, 0x00, 0x00, 0x00,
                                0x01, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00 };

    message = mbim_message_new (buffer, sizeof (buffer));

    g_assert_cmpuint (mbim_message_get_transaction_id         (message), ==, 1);
    g_assert_cmpuint (mbim_message_get_message_type           (message), ==, MBIM_MESSAGE_TYPE_CLOSE_DONE);
    g_assert_cmpuint (mbim_message_get_message_length         (message), ==, 16);
    g_assert_cmpuint (mbim_message_close_done_get_status_code (message), ==, MBIM_STATUS_ERROR_NONE);

    mbim_message_unref (message);
}

static void
test_message_command_empty (void)
{
    MbimMessage *message;
    guint32 len;

    message = mbim_message_command_new (12345,
                                        MBIM_SERVICE_BASIC_CONNECT,
                                        MBIM_CID_BASIC_CONNECT_DEVICE_CAPS,
                                        MBIM_MESSAGE_COMMAND_TYPE_QUERY);
    g_assert (message != NULL);

    g_assert_cmpuint (mbim_message_get_transaction_id (message), ==, 12345);
    g_assert_cmpuint (mbim_message_get_message_type   (message), ==, MBIM_MESSAGE_TYPE_COMMAND);
    g_assert_cmpuint (mbim_message_get_message_length (message), ==, 48);

    g_assert_cmpuint (mbim_message_command_get_service      (message), ==, MBIM_SERVICE_BASIC_CONNECT);
    g_assert_cmpuint (mbim_message_command_get_cid          (message), ==, MBIM_CID_BASIC_CONNECT_DEVICE_CAPS);
    g_assert_cmpuint (mbim_message_command_get_command_type (message), ==, MBIM_MESSAGE_COMMAND_TYPE_QUERY);

    g_assert (mbim_message_command_get_raw_information_buffer (message, &len) == NULL);
    g_assert_cmpuint (len, ==, 0);

    mbim_message_unref (message);
}

static void
test_message_command_not_empty (void)
{
    MbimMessage *message;
    const guint8 *buffer;
    guint32 len;
    const guint8 information_buffer [] = {
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07
    };

    message = mbim_message_command_new (12345,
                                        MBIM_SERVICE_BASIC_CONNECT,
                                        MBIM_CID_BASIC_CONNECT_DEVICE_CAPS,
                                        MBIM_MESSAGE_COMMAND_TYPE_QUERY);
    g_assert (message != NULL);
    mbim_message_command_append (message, information_buffer, sizeof (information_buffer));

    g_assert_cmpuint (mbim_message_get_transaction_id (message), ==, 12345);
    g_assert_cmpuint (mbim_message_get_message_type   (message), ==, MBIM_MESSAGE_TYPE_COMMAND);
    g_assert_cmpuint (mbim_message_get_message_length (message), ==, 56);

    g_assert_cmpuint (mbim_message_command_get_service      (message), ==, MBIM_SERVICE_BASIC_CONNECT);
    g_assert_cmpuint (mbim_message_command_get_cid          (message), ==, MBIM_CID_BASIC_CONNECT_DEVICE_CAPS);
    g_assert_cmpuint (mbim_message_command_get_command_type (message), ==, MBIM_MESSAGE_COMMAND_TYPE_QUERY);

    buffer = mbim_message_command_get_raw_information_buffer (message, &len);
    g_assert (buffer != NULL);
    g_assert_cmpuint (len, ==, sizeof (information_buffer));
    g_assert (memcmp (&information_buffer, buffer, sizeof (information_buffer)) == 0);

    mbim_message_unref (message);
}

static void
test_message_command_custom_service (void)
{
    static const gchar *nick = "My custom service";
    static const MbimUuid uuid_custom = {
        .a = { 0x11, 0x22, 0x33, 0x44 },
        .b = { 0x11, 0x11 },
        .c = { 0x22, 0x22 },
        .d = { 0x33, 0x33 },
        .e = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }
    };
    guint service;
    MbimMessage *message;

    service = mbim_register_custom_service (&uuid_custom, nick);
    g_assert (mbim_service_id_is_custom (service));

    message = mbim_message_command_new (0x01,
                                        service,
                                        0x11223344,
                                        MBIM_MESSAGE_COMMAND_TYPE_QUERY);
    g_assert (message);
    g_assert_cmpuint (mbim_message_command_get_service (message), ==, service);
    g_assert (mbim_uuid_cmp (mbim_message_command_get_service_id (message), &uuid_custom));
    g_assert_cmpuint (mbim_message_command_get_cid (message), ==, 0x11223344);
    g_assert_cmpuint (mbim_message_command_get_command_type (message), ==, MBIM_MESSAGE_COMMAND_TYPE_QUERY);

    mbim_message_unref (message);
}

static void
test_message_command_done (void)
{
    MbimMessage *message;
    const guint8 buffer [] =  { 0x03, 0x00, 0x00, 0x80,
                                0x3c, 0x00, 0x00, 0x00,
                                0x01, 0x00, 0x00, 0x00,
                                0x01, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                0xa2, 0x89, 0xcc, 0x33,
                                0xbc, 0xbb, 0x8b, 0x4f,
                                0xb6, 0xb0, 0x13, 0x3e,
                                0xc2, 0xaa, 0xe6, 0xdf,
                                0x04, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                0x0c, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00 };
    const guint8 expected_information_buffer [] = { 0x00, 0x00, 0x00, 0x00,
                                                    0x00, 0x00, 0x00, 0x00,
                                                    0x00, 0x00, 0x00, 0x00 };
    const guint8 *out_information_buffer;
    guint32 len;

    message = mbim_message_new (buffer, sizeof (buffer));

    g_assert_cmpuint (mbim_message_get_transaction_id           (message), ==, 1);
    g_assert_cmpuint (mbim_message_get_message_type             (message), ==, MBIM_MESSAGE_TYPE_COMMAND_DONE);
    g_assert_cmpuint (mbim_message_get_message_length           (message), ==, 60);

    g_assert_cmpuint (mbim_message_command_done_get_service     (message), ==, MBIM_SERVICE_BASIC_CONNECT);
    g_assert_cmpuint (mbim_message_command_done_get_cid         (message), ==, MBIM_CID_BASIC_CONNECT_PIN);
    g_assert_cmpuint (mbim_message_command_done_get_status_code (message), ==, MBIM_STATUS_ERROR_NONE);

    out_information_buffer = mbim_message_command_done_get_raw_information_buffer (message, &len);
    g_assert (buffer != NULL);
    g_assert_cmpuint (len, ==, sizeof (expected_information_buffer));
    g_assert (memcmp (&expected_information_buffer, out_information_buffer, sizeof (expected_information_buffer)) == 0);

    mbim_message_unref (message);
}

int main (int argc, char **argv)
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/libmbim-glib/message/open",                   test_message_open);
    g_test_add_func ("/libmbim-glib/message/open-done",              test_message_open_done);
    g_test_add_func ("/libmbim-glib/message/close",                  test_message_close);
    g_test_add_func ("/libmbim-glib/message/close-done",             test_message_close_done);
    g_test_add_func ("/libmbim-glib/message/command/empty",          test_message_command_empty);
    g_test_add_func ("/libmbim-glib/message/command/not-empty",      test_message_command_not_empty);
    g_test_add_func ("/libmbim-glib/message/command/custom-service", test_message_command_custom_service);
    g_test_add_func ("/libmbim-glib/message/command-done",           test_message_command_done);

    return g_test_run ();
}
