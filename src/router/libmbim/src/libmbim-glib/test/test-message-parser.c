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

#include "mbim-basic-connect.h"
#include "mbim-sms.h"
#include "mbim-ussd.h"
#include "mbim-auth.h"
#include "mbim-stk.h"
#include "mbim-ms-firmware-id.h"
#include "mbim-message.h"
#include "mbim-cid.h"
#include "mbim-utils.h"

#if defined ENABLE_TEST_MESSAGE_TRACES
static void
test_message_trace (const guint8 *computed,
                    guint32       computed_size,
                    const guint8 *expected,
                    guint32       expected_size)
{
    gchar *message_str;
    gchar *expected_str;

    message_str = __mbim_utils_str_hex (computed, computed_size, ':');
    expected_str = __mbim_utils_str_hex (expected, expected_size, ':');

    /* Dump all message contents */
    g_print ("\n"
             "Message str:\n"
             "'%s'\n"
             "Expected str:\n"
             "'%s'\n",
             message_str,
             expected_str);

    /* If they are different, tell which are the different bytes */
    if (computed_size != expected_size ||
        memcmp (computed, expected, expected_size)) {
        guint32 i;

        for (i = 0; i < MIN (computed_size, expected_size); i++) {
            if (computed[i] != expected[i])
                g_print ("Byte [%u] is different (computed: 0x%02X vs expected: 0x%02x)\n", i, computed[i], expected[i]);
        }
    }

    g_free (message_str);
    g_free (expected_str);
}
#else
#define test_message_trace(...)
#endif

static void
test_message_parser_basic_connect_visible_providers (void)
{
    MbimProvider **providers;
    guint32 n_providers;
    MbimMessage *response;
    GError *error = NULL;
    const guint8 buffer [] =  {
        /* header */
        0x03, 0x00, 0x00, 0x80, /* type */
        0xB4, 0x00, 0x00, 0x00, /* length */
        0x02, 0x00, 0x00, 0x00, /* transaction id */
        /* fragment header */
        0x01, 0x00, 0x00, 0x00, /* total */
        0x00, 0x00, 0x00, 0x00, /* current */
        /* command_done_message */
        0xA2, 0x89, 0xCC, 0x33, /* service id */
        0xBC, 0xBB, 0x8B, 0x4F,
        0xB6, 0xB0, 0x13, 0x3E,
        0xC2, 0xAA, 0xE6, 0xDF,
        0x08, 0x00, 0x00, 0x00, /* command id */
        0x00, 0x00, 0x00, 0x00, /* status code */
        0x84, 0x00, 0x00, 0x00, /* buffer length */
        /* information buffer */
        0x02, 0x00, 0x00, 0x00, /* 0x00 providers count */
        0x14, 0x00, 0x00, 0x00, /* 0x04 provider 0 offset */
        0x38, 0x00, 0x00, 0x00, /* 0x08 provider 0 length */
        0x4C, 0x00, 0x00, 0x00, /* 0x0C provider 1 offset */
        0x38, 0x00, 0x00, 0x00, /* 0x10 provider 1 length */
        /* data buffer... struct provider 0 */
        0x20, 0x00, 0x00, 0x00, /* 0x14 [0x00] id offset */
        0x0A, 0x00, 0x00, 0x00, /* 0x18 [0x04] id length */
        0x08, 0x00, 0x00, 0x00, /* 0x1C [0x08] state */
        0x2C, 0x00, 0x00, 0x00, /* 0x20 [0x0C] name offset */
        0x0C, 0x00, 0x00, 0x00, /* 0x24 [0x10] name length */
        0x01, 0x00, 0x00, 0x00, /* 0x28 [0x14] cellular class */
        0x0B, 0x00, 0x00, 0x00, /* 0x2C [0x18] rssi */
        0x00, 0x00, 0x00, 0x00, /* 0x30 [0x1C] error rate */
        0x32, 0x00, 0x31, 0x00, /* 0x34 [0x20] id string (10 bytes) */
        0x34, 0x00, 0x30, 0x00,
        0x33, 0x00, 0x00, 0x00,
        0x4F, 0x00, 0x72, 0x00, /* 0x40 [0x2C] name string (12 bytes) */
        0x61, 0x00, 0x6E, 0x00,
        0x67, 0x00, 0x65, 0x00,
        /* data buffer... struct provider 1 */
        0x20, 0x00, 0x00, 0x00, /* 0x4C [0x00] id offset */
        0x0A, 0x00, 0x00, 0x00, /* 0x50 [0x04] id length */
        0x19, 0x00, 0x00, 0x00, /* 0x51 [0x08] state */
        0x2C, 0x00, 0x00, 0x00, /* 0x54 [0x0C] name offset */
        0x0C, 0x00, 0x00, 0x00, /* 0x58 [0x10] name length */
        0x01, 0x00, 0x00, 0x00, /* 0x5C [0x14] cellular class */
        0x0B, 0x00, 0x00, 0x00, /* 0x60 [0x18] rssi */
        0x00, 0x00, 0x00, 0x00, /* 0x64 [0x1C] error rate */
        0x32, 0x00, 0x31, 0x00, /* 0x68 [0x20] id string (10 bytes) */
        0x34, 0x00, 0x30, 0x00,
        0x33, 0x00, 0x00, 0x00,
        0x4F, 0x00, 0x72, 0x00, /* 0x74 [0x2C] name string (12 bytes) */
        0x61, 0x00, 0x6E, 0x00,
        0x67, 0x00, 0x65, 0x00 };

    response = mbim_message_new (buffer, sizeof (buffer));

    g_assert (mbim_message_visible_providers_response_parse (
                  response,
                  &n_providers,
                  &providers,
                  &error));

    g_assert_no_error (error);

    g_assert_cmpuint (n_providers, ==, 2);

    /* Provider [0]
     * Provider ID: '21403'
     * Provider Name: 'Orange'
     * State: 'visible'
     * Cellular class: 'gsm'
     * RSSI: '11'
     * Error rate: '0'
     */
    g_assert_cmpstr (providers[0]->provider_id, ==, "21403");
    g_assert_cmpstr (providers[0]->provider_name, ==, "Orange");
    g_assert_cmpuint (providers[0]->provider_state, ==, MBIM_PROVIDER_STATE_VISIBLE);
    g_assert_cmpuint (providers[0]->cellular_class, ==, MBIM_CELLULAR_CLASS_GSM);
    g_assert_cmpuint (providers[0]->rssi, ==, 11);
    g_assert_cmpuint (providers[0]->error_rate, ==, 0);

    /* Provider [1]:
     * Provider ID: '21403'
     * Provider Name: 'Orange'
     * State: 'home, visible, registered'
     * Cellular class: 'gsm'
     * RSSI: '11'
     * Error rate: '0'
     */
    g_assert_cmpstr (providers[1]->provider_id, ==, "21403");
    g_assert_cmpstr (providers[1]->provider_name, ==, "Orange");
    g_assert_cmpuint (providers[1]->provider_state, ==, (MBIM_PROVIDER_STATE_HOME |
                                                         MBIM_PROVIDER_STATE_VISIBLE |
                                                         MBIM_PROVIDER_STATE_REGISTERED));
    g_assert_cmpuint (providers[1]->cellular_class, ==, MBIM_CELLULAR_CLASS_GSM);
    g_assert_cmpuint (providers[1]->rssi, ==, 11);
    g_assert_cmpuint (providers[1]->error_rate, ==, 0);

    mbim_provider_array_free (providers);
    mbim_message_unref (response);
}

static void
test_message_parser_basic_connect_subscriber_ready_status (void)
{
    MbimSubscriberReadyState ready_state;
    gchar *subscriber_id;
    gchar *sim_iccid;
    MbimReadyInfoFlag ready_info;
    guint32 telephone_numbers_count;
    gchar **telephone_numbers;
    MbimMessage *response;
    GError *error = NULL;
    const guint8 buffer [] =  {
        /* header */
        0x03, 0x00, 0x00, 0x80, /* type */
        0xB4, 0x00, 0x00, 0x00, /* length */
        0x02, 0x00, 0x00, 0x00, /* transaction id */
        /* fragment header */
        0x01, 0x00, 0x00, 0x00, /* total */
        0x00, 0x00, 0x00, 0x00, /* current */
        /* command_message */
        0xA2, 0x89, 0xCC, 0x33, /* service id */
        0xBC, 0xBB, 0x8B, 0x4F,
        0xB6, 0xB0, 0x13, 0x3E,
        0xC2, 0xAA, 0xE6, 0xDF,
        0x02, 0x00, 0x00, 0x00, /* command id */
        0x00, 0x00, 0x00, 0x00, /* status code */
        0x84, 0x00, 0x00, 0x00, /* buffer_length */
        /* information buffer */
        0x01, 0x00, 0x00, 0x00, /* 0x00 ready state */
        0x5C, 0x00, 0x00, 0x00, /* 0x04 subscriber id (offset) */
        0x1E, 0x00, 0x00, 0x00, /* 0x08 subscriber id (size) */
        0x7C, 0x00, 0x00, 0x00, /* 0x0C sim iccid (offset) */
        0x28, 0x00, 0x00, 0x00, /* 0x10 sim iccid (size) */
        0x00, 0x00, 0x00, 0x00, /* 0x14 ready info */
        0x02, 0x00, 0x00, 0x00, /* 0x18 telephone numbers count */
        0x2C, 0x00, 0x00, 0x00, /* 0x1C telephone number #1 (offset) */
        0x16, 0x00, 0x00, 0x00, /* 0x20 telephone number #1 (size) */
        0x44, 0x00, 0x00, 0x00, /* 0x24 telephone number #2 (offset) */
        0x16, 0x00, 0x00, 0x00, /* 0x28 telephone number #2 (size) */
        /* data buffer */
        0x31, 0x00, 0x31, 0x00, /* 0x2C telephone number #1 (data) */
        0x31, 0x00, 0x31, 0x00,
        0x31, 0x00, 0x31, 0x00,
        0x31, 0x00, 0x31, 0x00,
        0x31, 0x00, 0x31, 0x00,
        0x31, 0x00, 0x00, 0x00, /* last 2 bytes are padding */
        0x30, 0x00, 0x30, 0x00, /* 0x44 telephone number #2 (data) */
        0x30, 0x00, 0x30, 0x00,
        0x30, 0x00, 0x30, 0x00,
        0x30, 0x00, 0x30, 0x00,
        0x30, 0x00, 0x30, 0x00,
        0x30, 0x00, 0x00, 0x00, /* last 2 bytes are padding */
        0x33, 0x00, 0x31, 0x00, /* 0x5C subscriber id (data) */
        0x30, 0x00, 0x34, 0x00,
        0x31, 0x00, 0x30, 0x00,
        0x30, 0x00, 0x30, 0x00,
        0x30, 0x00, 0x31, 0x00,
        0x31, 0x00, 0x30, 0x00,
        0x37, 0x00, 0x36, 0x00,
        0x31, 0x00, 0x00, 0x00, /* last 2 bytes are padding */
        0x38, 0x00, 0x39, 0x00, /* 0x7C sim iccid (data) */
        0x30, 0x00, 0x31, 0x00,
        0x30, 0x00, 0x31, 0x00,
        0x30, 0x00, 0x34, 0x00,
        0x30, 0x00, 0x35, 0x00,
        0x34, 0x00, 0x36, 0x00,
        0x30, 0x00, 0x31, 0x00,
        0x31, 0x00, 0x30, 0x00,
        0x30, 0x00, 0x36, 0x00,
        0x31, 0x00, 0x32, 0x00 };

    response = mbim_message_new (buffer, sizeof (buffer));

    g_assert (mbim_message_subscriber_ready_status_response_parse (
                  response,
                  &ready_state,
                  &subscriber_id,
                  &sim_iccid,
                  &ready_info,
                  &telephone_numbers_count,
                  &telephone_numbers,
                  &error));

    g_assert_no_error (error);

    g_assert_cmpuint (ready_state, ==, MBIM_SUBSCRIBER_READY_STATE_INITIALIZED);
    g_assert_cmpstr (subscriber_id, ==, "310410000110761");
    g_assert_cmpstr (sim_iccid, ==, "89010104054601100612");
    g_assert_cmpuint (ready_info, ==, 0);
    g_assert_cmpuint (telephone_numbers_count, ==, 2);
    g_assert_cmpstr (telephone_numbers[0], ==, "11111111111");
    g_assert_cmpstr (telephone_numbers[1], ==, "00000000000");
    g_assert (telephone_numbers[2] == NULL);

    g_free (subscriber_id);
    g_free (sim_iccid);
    g_strfreev (telephone_numbers);

    mbim_message_unref (response);
}

static void
test_message_parser_basic_connect_device_caps (void)
{
    MbimMessage *response;
    MbimDeviceType device_type;
    MbimCellularClass cellular_class;
    MbimVoiceClass voice_class;
    MbimSimClass sim_class;
    MbimDataClass data_class;
    MbimSmsCaps sms_caps;
    MbimCtrlCaps ctrl_caps;
    guint32 max_sessions;
    gchar *custom_data_class;
    gchar *device_id;
    gchar *firmware_info;
    gchar *hardware_info;
    GError *error = NULL;
    const guint8 buffer [] =  { 0x03, 0x00, 0x00, 0x80,
                                0xD0, 0x00, 0x00, 0x00,
                                0x02, 0x00, 0x00, 0x00,
                                0x01, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                0xA2, 0x89, 0xCC, 0x33,
                                0xBC, 0xBB, 0x8B, 0x4F,
                                0xB6, 0xB0, 0x13, 0x3E,
                                0xC2, 0xAA, 0xE6, 0xDF,
                                0x01, 0x00, 0x00, 0x00,
                                0x00, 0x00, 0x00, 0x00,
                                0xA0, 0x00, 0x00, 0x00,
                                0x02, 0x00, 0x00, 0x00,
                                0x01, 0x00, 0x00, 0x00,
                                0x01, 0x00, 0x00, 0x00,
                                0x02, 0x00, 0x00, 0x00,
                                0x1F, 0x00, 0x00, 0x80,
                                0x03, 0x00, 0x00, 0x00,
                                0x01, 0x00, 0x00, 0x00,
                                0x01, 0x00, 0x00, 0x00,
                                0x40, 0x00, 0x00, 0x00,
                                0x0A, 0x00, 0x00, 0x00,
                                0x4C, 0x00, 0x00, 0x00,
                                0x1E, 0x00, 0x00, 0x00,
                                0x6C, 0x00, 0x00, 0x00,
                                0x1E, 0x00, 0x00, 0x00,
                                0x8C, 0x00, 0x00, 0x00,
                                0x12, 0x00, 0x00, 0x00,
                                0x48, 0x00, 0x53, 0x00,
                                0x50, 0x00, 0x41, 0x00,
                                0x2B, 0x00, 0x00, 0x00,
                                0x33, 0x00, 0x35, 0x00,
                                0x33, 0x00, 0x36, 0x00,
                                0x31, 0x00, 0x33, 0x00,
                                0x30, 0x00, 0x34, 0x00,
                                0x38, 0x00, 0x38, 0x00,
                                0x30, 0x00, 0x34, 0x00,
                                0x36, 0x00, 0x32, 0x00,
                                0x32, 0x00, 0x00, 0x00,
                                0x31, 0x00, 0x31, 0x00,
                                0x2E, 0x00, 0x38, 0x00,
                                0x31, 0x00, 0x30, 0x00,
                                0x2E, 0x00, 0x30, 0x00,
                                0x39, 0x00, 0x2E, 0x00,
                                0x30, 0x00, 0x30, 0x00,
                                0x2E, 0x00, 0x30, 0x00,
                                0x30, 0x00, 0x00, 0x00,
                                0x43, 0x00, 0x50, 0x00,
                                0x31, 0x00, 0x45, 0x00,
                                0x33, 0x00, 0x36, 0x00,
                                0x37, 0x00, 0x55, 0x00,
                                0x4D, 0x00, 0x00, 0x00 };

    response = mbim_message_new (buffer, sizeof (buffer));

    g_assert (mbim_message_device_caps_response_parse (
                  response,
                  &device_type,
                  &cellular_class,
                  &voice_class,
                  &sim_class,
                  &data_class,
                  &sms_caps,
                  &ctrl_caps,
                  &max_sessions,
                  &custom_data_class,
                  &device_id,
                  &firmware_info,
                  &hardware_info,
                  &error));

    g_assert_no_error (error);

    g_assert_cmpuint (device_type, ==, MBIM_DEVICE_TYPE_REMOVABLE);
    g_assert_cmpuint (cellular_class, ==, MBIM_CELLULAR_CLASS_GSM);
    g_assert_cmpuint (sim_class, ==, MBIM_SIM_CLASS_REMOVABLE);
    g_assert_cmpuint (data_class, ==, (MBIM_DATA_CLASS_GPRS |
                                       MBIM_DATA_CLASS_EDGE |
                                       MBIM_DATA_CLASS_UMTS |
                                       MBIM_DATA_CLASS_HSDPA |
                                       MBIM_DATA_CLASS_HSUPA |
                                       MBIM_DATA_CLASS_CUSTOM));
    g_assert_cmpuint (sms_caps, ==, (MBIM_SMS_CAPS_PDU_RECEIVE | MBIM_SMS_CAPS_PDU_SEND));
    g_assert_cmpuint (ctrl_caps, ==, MBIM_CTRL_CAPS_REG_MANUAL);
    g_assert_cmpuint (max_sessions, ==, 1);
    g_assert_cmpstr (custom_data_class, ==, "HSPA+");
    g_assert_cmpstr (device_id, ==, "353613048804622");
    g_assert_cmpstr (firmware_info, ==, "11.810.09.00.00");
    g_assert_cmpstr (hardware_info, ==, "CP1E367UM");

    g_free (custom_data_class);
    g_free (device_id);
    g_free (firmware_info);
    g_free (hardware_info);

    mbim_message_unref (response);
}

static void
test_message_parser_basic_connect_ip_configuration (void)
{
    MbimMessage *response;
    guint32 session_id;
    MbimIPConfigurationAvailableFlag ipv4configurationavailable;
    MbimIPConfigurationAvailableFlag ipv6configurationavailable;
    guint32 ipv4addresscount;
    MbimIPv4Element **ipv4address;
    guint32 ipv6addresscount;
    MbimIPv6Element **ipv6address;
    const MbimIPv4 *ipv4gateway;
    const MbimIPv6 *ipv6gateway;
    guint32 ipv4dnsservercount;
    MbimIPv4 *ipv4dnsserver;
    guint32 ipv6dnsservercount;
    MbimIPv6 *ipv6dnsserver;
    guint32 ipv4mtu;
    guint32 ipv6mtu;
    GError *error = NULL;
    const guint8 buffer [] =  {
        /* header */
        0x03, 0x00, 0x00, 0x80, /* type */
        0x80, 0x00, 0x00, 0x00, /* length */
        0x1A, 0x00, 0x00, 0x00, /* transaction id */
        /* fragment header */
        0x01, 0x00, 0x00, 0x00, /* total */
        0x00, 0x00, 0x00, 0x00, /* current */
        /* command_done_message */
        0xA2, 0x89, 0xCC, 0x33, /* service id */
        0xBC, 0xBB, 0x8B, 0x4F,
        0xB6, 0xB0, 0x13, 0x3E,
        0xC2, 0xAA, 0xE6, 0xDF,
        0x0F, 0x00, 0x00, 0x00, /* command id */
        0x00, 0x00, 0x00, 0x00, /* status code */
        0x50, 0x00, 0x00, 0x00, /* buffer length */
        /* information buffer */
        0x00, 0x00, 0x00, 0x00, /* session id */
        0x0F, 0x00, 0x00, 0x00, /* IPv4ConfigurationAvailable */
        0x00, 0x00, 0x00, 0x00, /* IPv6ConfigurationAvailable */
        0x01, 0x00, 0x00, 0x00, /* IPv4 element count */
        0x3C, 0x00, 0x00, 0x00, /* IPv4 element offset */
        0x00, 0x00, 0x00, 0x00, /* IPv6 element count */
        0x00, 0x00, 0x00, 0x00, /* IPv6 element offset */
        0x44, 0x00, 0x00, 0x00, /* IPv4 gateway offset */
        0x00, 0x00, 0x00, 0x00, /* IPv6 gateway offset */
        0x02, 0x00, 0x00, 0x00, /* IPv4 DNS count */
        0x48, 0x00, 0x00, 0x00, /* IPv4 DNS offset */
        0x00, 0x00, 0x00, 0x00, /* IPv6 DNS count */
        0x00, 0x00, 0x00, 0x00, /* IPv6 DNS offset */
        0xDC, 0x05, 0x00, 0x00, /* IPv4 MTU */
        0x00, 0x00, 0x00, 0x00, /* IPv6 MTU */
        /* data buffer */
        0x1C, 0x00, 0x00, 0x00, /* IPv4 element (netmask) */
        0xD4, 0x49, 0x22, 0xF8, /* IPv4 element (address) */
        0xD4, 0x49, 0x22, 0xF1, /* IPv4 gateway */
        0xD4, 0xA6, 0xD2, 0x50, /* IPv4 DNS1 */
        0xD4, 0x49, 0x20, 0x43  /* IPv4 DNS2 */
    };

    response = mbim_message_new (buffer, sizeof (buffer));

    g_assert (mbim_message_ip_configuration_response_parse (
                  response,
                  &session_id,
                  &ipv4configurationavailable,
                  &ipv6configurationavailable,
                  &ipv4addresscount,
                  &ipv4address,
                  &ipv6addresscount,
                  &ipv6address,
                  &ipv4gateway,
                  &ipv6gateway,
                  &ipv4dnsservercount,
                  &ipv4dnsserver,
                  &ipv6dnsservercount,
                  &ipv6dnsserver,
                  &ipv4mtu,
                  &ipv6mtu,
                  &error));

    /*
     *   IPv4 configuration available: 'address, gateway, dns, mtu'
     *     IP addresses (1)
     *       IP [0]: '212.166.228.25/28'
     *     Gateway: '212.166.228.26'
     *     DNS addresses (2)
     *       DNS [0]: '212.166.210.80'
     *       DNS [1]: '212.73.32.67'
     *     MTU: '1500'
     */

    g_assert_cmpuint (session_id, ==, 0);
    g_assert_cmpuint (ipv4configurationavailable, ==, (MBIM_IP_CONFIGURATION_AVAILABLE_FLAG_ADDRESS |
                                                       MBIM_IP_CONFIGURATION_AVAILABLE_FLAG_GATEWAY |
                                                       MBIM_IP_CONFIGURATION_AVAILABLE_FLAG_DNS |
                                                       MBIM_IP_CONFIGURATION_AVAILABLE_FLAG_MTU));
    g_assert_cmpuint (ipv6configurationavailable, ==, MBIM_IP_CONFIGURATION_AVAILABLE_FLAG_NONE);

    {
        MbimIPv4 addr = { .addr = { 0xD4, 0x49, 0x22, 0xF8 } };

        g_assert_cmpuint (ipv4addresscount, ==, 1);
        g_assert_cmpuint (ipv4address[0]->on_link_prefix_length, ==, 28);
        g_assert (memcmp (&addr, &(ipv4address[0]->ipv4_address), 4) == 0);
    }

    {
        MbimIPv4 gateway_addr = { .addr = { 0xD4, 0x49, 0x22, 0xF1 } };

        g_assert (memcmp (&gateway_addr, ipv4gateway, 4) == 0);
    }

    {
        MbimIPv4 dns_addr_1 = { .addr = { 0xD4, 0xA6, 0xD2, 0x50 } };
        MbimIPv4 dns_addr_2 = { .addr = { 0xD4, 0x49, 0x20, 0x43 } };

        g_assert_cmpuint (ipv4dnsservercount, ==, 2);
        g_assert (memcmp (&dns_addr_1, &ipv4dnsserver[0], 4) == 0);
        g_assert (memcmp (&dns_addr_2, &ipv4dnsserver[1], 4) == 0);
    }

    g_assert_cmpuint (ipv4mtu, ==, 1500);

    g_assert_cmpuint (ipv6addresscount, ==, 0);
    g_assert (ipv6address == NULL);
    g_assert (ipv6gateway == NULL);
    g_assert_cmpuint (ipv6dnsservercount, ==, 0);
    g_assert (ipv6dnsserver == NULL);

    mbim_ipv4_element_array_free (ipv4address);
    mbim_ipv6_element_array_free (ipv6address);
    g_free (ipv4dnsserver);
    g_free (ipv6dnsserver);

    mbim_message_unref (response);
}

static void
test_message_parser_basic_connect_service_activation (void)
{
    MbimMessage *response;
    GError *error = NULL;
    guint32 nw_error;
    const guint8 *databuffer;
    guint32 databuffer_size;
    const guint8 expected_databuffer [] =  {
        0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08
    };
    const guint8 buffer [] =  {
        /* header */
        0x03, 0x00, 0x00, 0x80, /* type */
        0x3C, 0x00, 0x00, 0x00, /* length */
        0x02, 0x00, 0x00, 0x00, /* transaction id */
        /* fragment header */
        0x01, 0x00, 0x00, 0x00, /* total */
        0x00, 0x00, 0x00, 0x00, /* current */
        /* command_done_message */
        0xA2, 0x89, 0xCC, 0x33, /* service id */
        0xBC, 0xBB, 0x8B, 0x4F,
        0xB6, 0xB0, 0x13, 0x3E,
        0xC2, 0xAA, 0xE6, 0xDF,
        0x0E, 0x00, 0x00, 0x00, /* command id */
        0x00, 0x00, 0x00, 0x00, /* status code */
        0x0C, 0x00, 0x00, 0x00, /* buffer length */
        /* information buffer */
        0x06, 0x00, 0x00, 0x00, /* nw error */
        0x01, 0x02, 0x03, 0x04, /* buffer */
        0x05, 0x06, 0x07, 0x08  };

    response = mbim_message_new (buffer, sizeof (buffer));

    g_assert (mbim_message_service_activation_response_parse (
                  response,
                  &nw_error,
                  &databuffer_size,
                  &databuffer,
                  &error));

    g_assert_no_error (error);

    g_assert_cmpuint (nw_error, ==, MBIM_NW_ERROR_ILLEGAL_ME);
    g_assert_cmpuint (databuffer_size, ==, sizeof (expected_databuffer));
    g_assert (memcmp (databuffer, expected_databuffer, databuffer_size) == 0);

    mbim_message_unref (response);
}

static void
test_message_parser_basic_connect_register_state (void)
{
    MbimMessage *response;
    MbimNwError nw_error;
    MbimRegisterState register_state;
    MbimRegisterMode register_mode;
    MbimDataClass available_data_classes;
    MbimCellularClass current_cellular_class;
    gchar *provider_id;
    gchar *provider_name;
    gchar *roaming_text;
    MbimRegistrationFlag registration_flag;
    GError *error = NULL;
    const guint8 buffer [] =  {
        /* header */
        0x03, 0x00, 0x00, 0x80, /* type */
        0x6C, 0x00, 0x00, 0x00, /* length */
        0x12, 0x00, 0x00, 0x00, /* transaction id */
        /* fragment header */
        0x01, 0x00, 0x00, 0x00, /* total */
        0x00, 0x00, 0x00, 0x00, /* current */
        /* command_done message */
        0xA2, 0x89, 0xCC, 0x33, /* service id */
        0xBC, 0xBB, 0x8B, 0x4F,
        0xB6, 0xB0, 0x13, 0x3E,
        0xC2, 0xAA, 0xE6, 0xDF,
        0x09, 0x00, 0x00, 0x00, /* command id */
        0x00, 0x00, 0x00, 0x00, /* status code */
        0x3C, 0x00, 0x00, 0x00, /* buffer length */
        /* information buffer */
        0x00, 0x00, 0x00, 0x00, /* nw error */
        0x03, 0x00, 0x00, 0x00, /* register state */
        0x01, 0x00, 0x00, 0x00, /* register mode */
        0x1C, 0x00, 0x00, 0x00, /* available data classes */
        0x01, 0x00, 0x00, 0x00, /* current cellular class */
        0x30, 0x00, 0x00, 0x00, /* provider id offset */
        0x0A, 0x00, 0x00, 0x00, /* provider id size */
        0x00, 0x00, 0x00, 0x00, /* provider name offset */
        0x00, 0x00, 0x00, 0x00, /* provider name size */
        0x00, 0x00, 0x00, 0x00, /* roaming text offset */
        0x00, 0x00, 0x00, 0x00, /* roaming text size */
        0x02, 0x00, 0x00, 0x00, /* registration flag */
        /* data buffer */
        0x32, 0x00, 0x36, 0x00,
        0x30, 0x00, 0x30, 0x00,
        0x36, 0x00, 0x00, 0x00 };

    response = mbim_message_new (buffer, sizeof (buffer));


    g_assert (mbim_message_register_state_response_parse (
                  response,
                  &nw_error,
                  &register_state,
                  &register_mode,
                  &available_data_classes,
                  &current_cellular_class,
                  &provider_id,
                  &provider_name,
                  &roaming_text,
                  &registration_flag,
                  &error));

    g_assert_no_error (error);

    g_assert_cmpuint (nw_error, ==, MBIM_NW_ERROR_UNKNOWN);
    g_assert_cmpuint (register_state, ==, MBIM_REGISTER_STATE_HOME);
    g_assert_cmpuint (register_mode, ==, MBIM_REGISTER_MODE_AUTOMATIC);
    g_assert_cmpuint (available_data_classes, ==, (MBIM_DATA_CLASS_UMTS |
                                                   MBIM_DATA_CLASS_HSDPA |
                                                   MBIM_DATA_CLASS_HSUPA));
    g_assert_cmpuint (current_cellular_class, ==, MBIM_CELLULAR_CLASS_GSM);
    g_assert_cmpstr (provider_id, ==, "26006");
    g_free (provider_id);
    g_assert (provider_name == NULL);
    g_assert (roaming_text == NULL);
    g_assert_cmpuint (registration_flag, ==, MBIM_REGISTRATION_FLAG_PACKET_SERVICE_AUTOMATIC_ATTACH);

    mbim_message_unref (response);
}

static void
test_message_parser_sms_read_zero_pdu (void)
{
    MbimSmsFormat format;
    guint32 messages_count;
    MbimSmsPduReadRecord **pdu_messages;
    MbimSmsCdmaReadRecord **cdma_messages;
    MbimMessage *response;
    GError *error = NULL;
    const guint8 buffer [] =  {
        /* header */
        0x03, 0x00, 0x00, 0x80, /* type */
        0xB0, 0x00, 0x00, 0x00, /* length */
        0x02, 0x00, 0x00, 0x00, /* transaction id */
        /* fragment header */
        0x01, 0x00, 0x00, 0x00, /* total */
        0x00, 0x00, 0x00, 0x00, /* current */
        /* command_done_message */
        0x53, 0x3F, 0xBE, 0xEB, /* service id */
        0x14, 0xFE, 0x44, 0x67,
        0x9F, 0x90, 0x33, 0xA2,
        0x23, 0xE5, 0x6C, 0x3F,
        0x02, 0x00, 0x00, 0x00, /* command id */
        0x00, 0x00, 0x00, 0x00, /* status code */
        0x38, 0x00, 0x00, 0x00, /* buffer length */
        /* information buffer */
        0x00, 0x00, 0x00, 0x00, /* 0x00 format */
        0x00, 0x00, 0x00, 0x00, /* 0x04 messages count */
    };

    response = mbim_message_new (buffer, sizeof (buffer));

    g_assert (mbim_message_sms_read_response_parse (
                  response,
                  &format,
                  &messages_count,
                  &pdu_messages,
                  &cdma_messages,
                  &error));
    g_assert_no_error (error);

    g_assert_cmpuint (format, ==, MBIM_SMS_FORMAT_PDU);
    g_assert_cmpuint (messages_count, ==, 0);
    g_assert (pdu_messages == NULL);
    g_assert (cdma_messages == NULL);

    mbim_message_unref (response);
}

static void
test_message_parser_sms_read_single_pdu (void)
{
    MbimSmsFormat format;
    guint32 messages_count;
    MbimSmsPduReadRecord **pdu_messages;
    MbimSmsCdmaReadRecord **cdma_messages;
    MbimMessage *response;
    GError *error = NULL;
    const guint8 buffer [] =  {
        /* header */
        0x03, 0x00, 0x00, 0x80, /* type */
        0xB0, 0x00, 0x00, 0x00, /* length */
        0x02, 0x00, 0x00, 0x00, /* transaction id */
        /* fragment header */
        0x01, 0x00, 0x00, 0x00, /* total */
        0x00, 0x00, 0x00, 0x00, /* current */
        /* command_done_message */
        0x53, 0x3F, 0xBE, 0xEB, /* service id */
        0x14, 0xFE, 0x44, 0x67,
        0x9F, 0x90, 0x33, 0xA2,
        0x23, 0xE5, 0x6C, 0x3F,
        0x02, 0x00, 0x00, 0x00, /* command id */
        0x00, 0x00, 0x00, 0x00, /* status code */
        0x60, 0x00, 0x00, 0x00, /* buffer length */
        /* information buffer */
        0x00, 0x00, 0x00, 0x00, /* 0x00 format */
        0x01, 0x00, 0x00, 0x00, /* 0x04 messages count */
        0x10, 0x00, 0x00, 0x00, /* 0x08 message 1 offset */
        0x20, 0x00, 0x00, 0x00, /* 0x0C message 1 length */
        /* data buffer... message 1 */
        0x07, 0x00, 0x00, 0x00, /* 0x10 0x00 message index */
        0x03, 0x00, 0x00, 0x00, /* 0x14 0x04 message status */
        0x10, 0x00, 0x00, 0x00, /* 0x18 0x08 pdu data offset (w.r.t. pdu start */
        0x10, 0x00, 0x00, 0x00, /* 0x1C 0x0C pdu data length */
        /*    pdu data... */
        0x01, 0x02, 0x03, 0x04, /* 0x20 0x10 */
        0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C,
        0x0D, 0x0E, 0x0F, 0x00
    };

    const guint8 expected_pdu [] = {
        0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C,
        0x0D, 0x0E, 0x0F, 0x00
    };

    response = mbim_message_new (buffer, sizeof (buffer));

    g_assert (mbim_message_sms_read_response_parse (
                  response,
                  &format,
                  &messages_count,
                  &pdu_messages,
                  &cdma_messages,
                  &error));
    g_assert_no_error (error);

    g_assert_cmpuint (format, ==, MBIM_SMS_FORMAT_PDU);
    g_assert_cmpuint (messages_count, ==, 1);
    g_assert (pdu_messages != NULL);
    g_assert (cdma_messages == NULL);

    g_assert_cmpuint (pdu_messages[0]->message_index, ==, 7);
    g_assert_cmpuint (pdu_messages[0]->message_status, ==, MBIM_SMS_STATUS_SENT);
    test_message_trace (pdu_messages[0]->pdu_data,
                        pdu_messages[0]->pdu_data_size,
                        expected_pdu,
                        sizeof (expected_pdu));
    g_assert_cmpuint (pdu_messages[0]->pdu_data_size, ==, sizeof (expected_pdu));
    g_assert (memcmp (pdu_messages[0]->pdu_data, expected_pdu, sizeof (expected_pdu)) == 0);

    mbim_sms_pdu_read_record_array_free (pdu_messages);
    mbim_message_unref (response);
}

static void
test_message_parser_sms_read_multiple_pdu (void)
{
    guint32 idx;
    MbimSmsFormat format;
    guint32 messages_count;
    MbimSmsPduReadRecord **pdu_messages;
    MbimSmsCdmaReadRecord **cdma_messages;
    MbimMessage *response;
    GError *error = NULL;
    const guint8 buffer [] =  {
        /* header */
        0x03, 0x00, 0x00, 0x80, /* type */
        0xB0, 0x00, 0x00, 0x00, /* length */
        0x02, 0x00, 0x00, 0x00, /* transaction id */
        /* fragment header */
        0x01, 0x00, 0x00, 0x00, /* total */
        0x00, 0x00, 0x00, 0x00, /* current */
        /* command_done_message */
        0x53, 0x3F, 0xBE, 0xEB, /* service id */
        0x14, 0xFE, 0x44, 0x67,
        0x9F, 0x90, 0x33, 0xA2,
        0x23, 0xE5, 0x6C, 0x3F,
        0x02, 0x00, 0x00, 0x00, /* command id */
        0x00, 0x00, 0x00, 0x00, /* status code */
        0x60, 0x00, 0x00, 0x00, /* buffer length */
        /* information buffer */
        0x00, 0x00, 0x00, 0x00, /* 0x00 format */
        0x02, 0x00, 0x00, 0x00, /* 0x04 messages count */
        0x18, 0x00, 0x00, 0x00, /* 0x08 message 1 offset */
        0x20, 0x00, 0x00, 0x00, /* 0x0C message 1 length */
        0x38, 0x00, 0x00, 0x00, /* 0x10 message 2 offset */
        0x24, 0x00, 0x00, 0x00, /* 0x14 message 2 length */
        /* data buffer... message 1 */
        0x06, 0x00, 0x00, 0x00, /* 0x18 0x00 message index */
        0x03, 0x00, 0x00, 0x00, /* 0x1C 0x04 message status */
        0x10, 0x00, 0x00, 0x00, /* 0x20 0x08 pdu data offset (w.r.t. pdu start */
        0x10, 0x00, 0x00, 0x00, /* 0x24 0x0C pdu data length */
        /*    pdu data... */
        0x01, 0x02, 0x03, 0x04, /* 0x28 0x10 */
        0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C,
        0x0D, 0x0E, 0x0F, 0x00,
        /* data buffer... message 2 */
        0x07, 0x00, 0x00, 0x00, /* 0x38 0x00 message index */
        0x03, 0x00, 0x00, 0x00, /* 0x3C 0x04 message status */
        0x10, 0x00, 0x00, 0x00, /* 0x40 0x08 pdu data offset (w.r.t. pdu start */
        0x10, 0x00, 0x00, 0x00, /* 0x44 0x0C pdu data length */
        /*    pdu data... */
        0x00, 0x01, 0x02, 0x03, /* 0x48 0x10 */
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B,
        0x0C, 0x0D, 0x0E, 0x0F
    };

    const guint8 expected_pdu_idx6 [] = {
        0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C,
        0x0D, 0x0E, 0x0F, 0x00
    };

    const guint8 expected_pdu_idx7 [] = {
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B,
        0x0C, 0x0D, 0x0E, 0x0F
    };

    response = mbim_message_new (buffer, sizeof (buffer));

    g_assert (mbim_message_sms_read_response_parse (
                  response,
                  &format,
                  &messages_count,
                  &pdu_messages,
                  &cdma_messages,
                  &error));
    g_assert_no_error (error);

    g_assert_cmpuint (format, ==, MBIM_SMS_FORMAT_PDU);
    g_assert_cmpuint (messages_count, ==, 2);
    g_assert (pdu_messages != NULL);
    g_assert (cdma_messages == NULL);

    /* First message with index 6 */
    if (pdu_messages[0]->message_index == 6)
        idx = 0;
    else if (pdu_messages[1]->message_index == 6)
        idx = 1;
    else
        g_assert_not_reached ();
    g_assert_cmpuint (pdu_messages[idx]->message_index, ==, 6);
    g_assert_cmpuint (pdu_messages[idx]->message_status, ==, MBIM_SMS_STATUS_SENT);
    test_message_trace (pdu_messages[idx]->pdu_data,
                        pdu_messages[idx]->pdu_data_size,
                        expected_pdu_idx6,
                        sizeof (expected_pdu_idx6));
    g_assert_cmpuint (pdu_messages[idx]->pdu_data_size, ==, sizeof (expected_pdu_idx6));
    g_assert (memcmp (pdu_messages[idx]->pdu_data, expected_pdu_idx6, sizeof (expected_pdu_idx6)) == 0);

    /* Second message with index 7 */
    if (pdu_messages[0]->message_index == 7)
        idx = 0;
    else if (pdu_messages[1]->message_index == 7)
        idx = 1;
    else
        g_assert_not_reached ();
    g_assert_cmpuint (pdu_messages[idx]->message_index, ==, 7);
    g_assert_cmpuint (pdu_messages[idx]->message_status, ==, MBIM_SMS_STATUS_SENT);
    test_message_trace (pdu_messages[idx]->pdu_data,
                        pdu_messages[idx]->pdu_data_size,
                        expected_pdu_idx7,
                        sizeof (expected_pdu_idx7));
    g_assert_cmpuint (pdu_messages[idx]->pdu_data_size, ==, sizeof (expected_pdu_idx7));
    g_assert (memcmp (pdu_messages[idx]->pdu_data, expected_pdu_idx7, sizeof (expected_pdu_idx7)) == 0);

    mbim_sms_pdu_read_record_array_free (pdu_messages);
    mbim_message_unref (response);
}

static void
test_message_parser_ussd (void)
{
    MbimUssdResponse ussd_response;
    MbimUssdSessionState ussd_session_state;
    const guint8 *ussd_payload;
    guint32 ussd_payload_size;
    guint32 ussd_dcs;
    MbimMessage *response;
    GError *error = NULL;
    const guint8 buffer [] =  {
        /* header */
        0x03, 0x00, 0x00, 0x80, /* type */
        0x54, 0x00, 0x00, 0x00, /* length */
        0x02, 0x00, 0x00, 0x00, /* transaction id */
        /* fragment header */
        0x01, 0x00, 0x00, 0x00, /* total */
        0x00, 0x00, 0x00, 0x00, /* current */
        /* command_done_message */
        0xE5, 0x50, 0xA0, 0xC8, /* service id */
        0x5E, 0x82, 0x47, 0x9E,
        0x82, 0xF7, 0x10, 0xAB,
        0xF4, 0xC3, 0x35, 0x1F,
        0x01, 0x00, 0x00, 0x00, /* command id */
        0x00, 0x00, 0x00, 0x00, /* status code */
        0x24, 0x00, 0x00, 0x00, /* buffer length */
        /* information buffer */
        0x05, 0x00, 0x00, 0x00, /* 0x00 response */
        0x01, 0x00, 0x00, 0x00, /* 0x04 sesstion state */
        0x01, 0x00, 0x00, 0x00, /* 0x08 coding scheme */
        0x14, 0x00, 0x00, 0x00, /* 0x0C payload offset */
        0x10, 0x00, 0x00, 0x00, /* 0x10 payload length */
        /* data buffer... payload */
        0x01, 0x02, 0x03, 0x04, /* 0x14 payload */
        0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C,
        0x0D, 0x0E, 0x0F, 0x00
    };

    const guint8 expected_payload [] = {
        0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08,
        0x09, 0x0A, 0x0B, 0x0C,
        0x0D, 0x0E, 0x0F, 0x00
    };

    response = mbim_message_new (buffer, sizeof (buffer));

    g_assert (mbim_message_ussd_response_parse (
                  response,
                  &ussd_response,
                  &ussd_session_state,
                  &ussd_dcs,
                  &ussd_payload_size,
                  &ussd_payload,
                  &error));
    g_assert_no_error (error);

    g_assert_cmpuint (ussd_response, ==, MBIM_USSD_RESPONSE_NETWORK_TIMEOUT);
    g_assert_cmpuint (ussd_session_state, ==, MBIM_USSD_SESSION_STATE_EXISTING_SESSION);
    g_assert_cmpuint (ussd_dcs, ==, 0x01);

    test_message_trace (ussd_payload,
                        ussd_payload_size,
                        expected_payload,
                        sizeof (expected_payload));
    g_assert_cmpuint (ussd_payload_size, ==, sizeof (expected_payload));
    g_assert (memcmp (ussd_payload, expected_payload, sizeof (expected_payload)) == 0);

    mbim_message_unref (response);
}

static void
test_message_parser_auth_akap (void)
{
    const guint8 *res;
    guint32 res_len;
    const guint8 *ik;
    const guint8 *ck;
    const guint8 *auts;
    MbimMessage *response;
    GError *error = NULL;
    const guint8 buffer [] =  {
        /* header */
        0x03, 0x00, 0x00, 0x80, /* type */
        0x74, 0x00, 0x00, 0x00, /* length */
        0x02, 0x00, 0x00, 0x00, /* transaction id */
        /* fragment header */
        0x01, 0x00, 0x00, 0x00, /* total */
        0x00, 0x00, 0x00, 0x00, /* current */
        /* command_done_message */
        0x1D, 0x2B, 0x5F, 0xF7, /* service id */
        0x0A, 0xA1, 0x48, 0xB2,
        0xAA, 0x52, 0x50, 0xF1,
        0x57, 0x67, 0x17, 0x4E,
        0x02, 0x00, 0x00, 0x00, /* command id */
        0x00, 0x00, 0x00, 0x00, /* status code */
        0x44, 0x00, 0x00, 0x00, /* buffer length */
        /* information buffer */
        0x00, 0x01, 0x02, 0x03, /* 0x00 Res */
        0x04, 0x05, 0x06, 0x07, /* 0x04 */
        0x08, 0x09, 0x0A, 0x0B, /* 0x08 */
        0x0C, 0x0D, 0x0E, 0x0F, /* 0x0C */
        0x05, 0x00, 0x00, 0x00, /* 0x10 Reslen */
        0xFF, 0xFE, 0xFD, 0xFC, /* 0x14 IK */
        0xFB, 0xFA, 0xF9, 0xF8, /* 0x18 */
        0xF7, 0xF6, 0xF5, 0xF4, /* 0x1C */
        0xF3, 0xF2, 0xF1, 0xF0, /* 0x20 */
        0xAF, 0xAE, 0xAD, 0xAC, /* 0x24 CK */
        0xAB, 0xAA, 0xA9, 0xA8, /* 0x28 */
        0xA7, 0xA6, 0xA5, 0xA4, /* 0x2C */
        0xA3, 0xA2, 0xA1, 0xA0, /* 0x30 */
        0x7F, 0x7E, 0x7D, 0x7C, /* 0x34 Auts */
        0x7B, 0x7A, 0x79, 0x78, /* 0x38 */
        0x77, 0x76, 0x75, 0x74, /* 0x3C */
        0x73, 0x72, 0x00, 0x00, /* 0x40 */
    };

    const guint8 expected_res [] = {
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B,
        0x0C, 0x0D, 0x0E, 0x0F,
    };
    const guint8 expected_ik [] = {
        0xFF, 0xFE, 0xFD, 0xFC,
        0xFB, 0xFA, 0xF9, 0xF8,
        0xF7, 0xF6, 0xF5, 0xF4,
        0xF3, 0xF2, 0xF1, 0xF0,
    };
    const guint8 expected_ck [] = {
        0xAF, 0xAE, 0xAD, 0xAC,
        0xAB, 0xAA, 0xA9, 0xA8,
        0xA7, 0xA6, 0xA5, 0xA4,
        0xA3, 0xA2, 0xA1, 0xA0,
    };
    const guint8 expected_auts [] = {
        0x7F, 0x7E, 0x7D, 0x7C,
        0x7B, 0x7A, 0x79, 0x78,
        0x77, 0x76, 0x75, 0x74,
        0x73, 0x72
    };


    response = mbim_message_new (buffer, sizeof (buffer));

    g_assert (mbim_message_auth_akap_response_parse (
                  response,
                  &res,
                  &res_len,
                  &ik,
                  &ck,
                  &auts,
                  &error));
    g_assert_no_error (error);

    test_message_trace (res,
                        sizeof (expected_res),
                        expected_res,
                        sizeof (expected_res));
    g_assert (memcmp (res, expected_res, sizeof (expected_res)) == 0);

    g_assert_cmpuint (res_len, ==, 5);

    test_message_trace (ik,
                        sizeof (expected_ik),
                        expected_ik,
                        sizeof (expected_ik));
    g_assert (memcmp (ik, expected_ik, sizeof (expected_ik)) == 0);

    test_message_trace (ck,
                        sizeof (expected_ck),
                        expected_ck,
                        sizeof (expected_ck));
    g_assert (memcmp (ck, expected_ck, sizeof (expected_ck)) == 0);

    test_message_trace (auts,
                        sizeof (expected_auts),
                        expected_auts,
                        sizeof (expected_auts));
    g_assert (memcmp (auts, expected_auts, sizeof (expected_auts)) == 0);

    mbim_message_unref (response);
}

static void
test_message_parser_stk_pac_notification (void)
{
    const guint8 *databuffer;
    guint32 databuffer_len;
    guint32 pac_type;
    MbimMessage *response;
    GError *error = NULL;
    const guint8 buffer [] =  {
        /* header */
        0x07, 0x00, 0x00, 0x80, /* type */
        0x54, 0x00, 0x00, 0x00, /* length */
        0x02, 0x00, 0x00, 0x00, /* transaction id */
        /* fragment header */
        0x01, 0x00, 0x00, 0x00, /* total */
        0x00, 0x00, 0x00, 0x00, /* current */
        /* indicate_status_message */
        0xD8, 0xF2, 0x01, 0x31, /* service id */
        0xFC, 0xB5, 0x4E, 0x17,
        0x86, 0x02, 0xD6, 0xED,
        0x38, 0x16, 0x16, 0x4C,
        0x01, 0x00, 0x00, 0x00, /* command id */
        0x28, 0x00, 0x00, 0x00, /* buffer length */
        /* information buffer */
        0x01, 0x00, 0x00, 0x00, /* 0x00 Pac Type */
        0x04, 0x05, 0x06, 0x07, /* 0x04 Data buffer */
        0xAF, 0xAE, 0xAD, 0xAC,
        0xAB, 0xAA, 0xA9, 0xA8,
        0xA7, 0xA6, 0xA5, 0xA4,
        0xA3, 0xA2, 0xA1, 0xA0,
        0x7F, 0x7E, 0x7D, 0x7C,
        0x7B, 0x7A, 0x79, 0x78,
        0x77, 0x76, 0x75, 0x74,
        0x73, 0x72, 0x00, 0x00
    };

    const guint8 expected_databuffer [] = {
        0x04, 0x05, 0x06, 0x07,
        0xAF, 0xAE, 0xAD, 0xAC,
        0xAB, 0xAA, 0xA9, 0xA8,
        0xA7, 0xA6, 0xA5, 0xA4,
        0xA3, 0xA2, 0xA1, 0xA0,
        0x7F, 0x7E, 0x7D, 0x7C,
        0x7B, 0x7A, 0x79, 0x78,
        0x77, 0x76, 0x75, 0x74,
        0x73, 0x72, 0x00, 0x00
    };

    response = mbim_message_new (buffer, sizeof (buffer));

    g_assert (mbim_message_stk_pac_notification_parse (
                  response,
                  &pac_type,
                  &databuffer_len,
                  &databuffer,
                  &error));
    g_assert_no_error (error);

    g_assert_cmpuint (pac_type, ==, MBIM_STK_PAC_TYPE_NOTIFICATION);

    test_message_trace (databuffer,
                        sizeof (databuffer_len),
                        expected_databuffer,
                        sizeof (expected_databuffer));
    g_assert_cmpuint (databuffer_len, ==, sizeof (expected_databuffer));
    g_assert (memcmp (databuffer, expected_databuffer, sizeof (expected_databuffer)) == 0);

    mbim_message_unref (response);
}

static void
test_message_parser_stk_pac_response (void)
{
    const guint8 *databuffer;
    MbimMessage *response;
    GError *error = NULL;
    const guint8 buffer [] =  {
        /* header */
        0x03, 0x00, 0x00, 0x80, /* type */
        0x2C, 0x01, 0x00, 0x00, /* length */
        0x02, 0x00, 0x00, 0x00, /* transaction id */
        /* fragment header */
        0x01, 0x00, 0x00, 0x00, /* total */
        0x00, 0x00, 0x00, 0x00, /* current */
        /* indicate_status_message */
        0xD8, 0xF2, 0x01, 0x31, /* service id */
        0xFC, 0xB5, 0x4E, 0x17,
        0x86, 0x02, 0xD6, 0xED,
        0x38, 0x16, 0x16, 0x4C,
        0x01, 0x00, 0x00, 0x00, /* command id */
        0x00, 0x00, 0x00, 0x00, /* status code */
        0x00, 0x01, 0x00, 0x00, /* buffer length */
        /* information buffer */
        0x04, 0x05, 0x06, 0x07,
        0xAF, 0xAE, 0xAD, 0xAC,
        0xAB, 0xAA, 0xA9, 0xA8,
        0xA7, 0xA6, 0xA5, 0xA4,
        0xA3, 0xA2, 0xA1, 0xA0,
        0x7F, 0x7E, 0x7D, 0x7C,
        0x7B, 0x7A, 0x79, 0x78,
        0x77, 0x76, 0x75, 0x74,
        0x73, 0x72, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x04, 0x05, 0x06, 0x07,
        0xAF, 0xAE, 0xAD, 0xAC,
        0xAB, 0xAA, 0xA9, 0xA8,
        0xA7, 0xA6, 0xA5, 0xA4,
        0xA3, 0xA2, 0xA1, 0xA0,
        0x7F, 0x7E, 0x7D, 0x7C,
        0x7B, 0x7A, 0x79, 0x78,
        0x77, 0x76, 0x75, 0x74,
        0x73, 0x72, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x04, 0x05, 0x06, 0x07,
        0xAF, 0xAE, 0xAD, 0xAC,
        0xAB, 0xAA, 0xA9, 0xA8,
        0xA7, 0xA6, 0xA5, 0xA4,
        0xA3, 0xA2, 0xA1, 0xA0,
        0x7F, 0x7E, 0x7D, 0x7C,
        0x7B, 0x7A, 0x79, 0x78,
        0x77, 0x76, 0x75, 0x74,
        0x73, 0x72, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x04, 0x05, 0x06, 0x07,
        0xAF, 0xAE, 0xAD, 0xAC,
        0xAB, 0xAA, 0xA9, 0xA8,
        0xA7, 0xA6, 0xA5, 0xA4,
        0xA3, 0xA2, 0xA1, 0xA0,
        0x7F, 0x7E, 0x7D, 0x7C,
        0x7B, 0x7A, 0x79, 0x78,
        0x77, 0x76, 0x75, 0x74,
        0x73, 0x72, 0x00, 0x00,
    };

    const guint8 expected_databuffer [] = {
        0x04, 0x05, 0x06, 0x07,
        0xAF, 0xAE, 0xAD, 0xAC,
        0xAB, 0xAA, 0xA9, 0xA8,
        0xA7, 0xA6, 0xA5, 0xA4,
        0xA3, 0xA2, 0xA1, 0xA0,
        0x7F, 0x7E, 0x7D, 0x7C,
        0x7B, 0x7A, 0x79, 0x78,
        0x77, 0x76, 0x75, 0x74,
        0x73, 0x72, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x04, 0x05, 0x06, 0x07,
        0xAF, 0xAE, 0xAD, 0xAC,
        0xAB, 0xAA, 0xA9, 0xA8,
        0xA7, 0xA6, 0xA5, 0xA4,
        0xA3, 0xA2, 0xA1, 0xA0,
        0x7F, 0x7E, 0x7D, 0x7C,
        0x7B, 0x7A, 0x79, 0x78,
        0x77, 0x76, 0x75, 0x74,
        0x73, 0x72, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x04, 0x05, 0x06, 0x07,
        0xAF, 0xAE, 0xAD, 0xAC,
        0xAB, 0xAA, 0xA9, 0xA8,
        0xA7, 0xA6, 0xA5, 0xA4,
        0xA3, 0xA2, 0xA1, 0xA0,
        0x7F, 0x7E, 0x7D, 0x7C,
        0x7B, 0x7A, 0x79, 0x78,
        0x77, 0x76, 0x75, 0x74,
        0x73, 0x72, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00,
        0x04, 0x05, 0x06, 0x07,
        0xAF, 0xAE, 0xAD, 0xAC,
        0xAB, 0xAA, 0xA9, 0xA8,
        0xA7, 0xA6, 0xA5, 0xA4,
        0xA3, 0xA2, 0xA1, 0xA0,
        0x7F, 0x7E, 0x7D, 0x7C,
        0x7B, 0x7A, 0x79, 0x78,
        0x77, 0x76, 0x75, 0x74,
        0x73, 0x72, 0x00, 0x00
    };

    response = mbim_message_new (buffer, sizeof (buffer));

    g_assert (mbim_message_stk_pac_response_parse (
                  response,
                  &databuffer,
                  &error));
    g_assert_no_error (error);

    test_message_trace (databuffer,
                        sizeof (expected_databuffer),
                        expected_databuffer,
                        sizeof (expected_databuffer));
    g_assert (memcmp (databuffer, expected_databuffer, sizeof (expected_databuffer)) == 0);

    mbim_message_unref (response);
}

static void
test_message_parser_stk_terminal_response (void)
{
    const guint8 *databuffer;
    guint32 databuffer_len;
    guint32 status_words;
    MbimMessage *response;
    GError *error = NULL;
    const guint8 buffer [] =  {
        /* header */
        0x03, 0x00, 0x00, 0x80, /* type */
        0x48, 0x01, 0x00, 0x00, /* length */
        0x02, 0x00, 0x00, 0x00, /* transaction id */
        /* fragment header */
        0x01, 0x00, 0x00, 0x00, /* total */
        0x00, 0x00, 0x00, 0x00, /* current */
        /* indicate_status_message */
        0xD8, 0xF2, 0x01, 0x31, /* service id */
        0xFC, 0xB5, 0x4E, 0x17,
        0x86, 0x02, 0xD6, 0xED,
        0x38, 0x16, 0x16, 0x4C,
        0x02, 0x00, 0x00, 0x00, /* command id */
        0x00, 0x00, 0x00, 0x00, /* status code */
        0x18, 0x00, 0x00, 0x00, /* buffer length */
        /* information buffer */
        0x0C, 0x00, 0x00, 0x00, /* 0x00 ResultData offset */
        0x0C, 0x00, 0x00, 0x00, /* 0x04 ResultData length */
        0xCC, 0x00, 0x00, 0x00, /* 0x08 StatusWords */
        /* databuffer */
        0x00, 0x00, 0x00, 0x00, /* 0x0C ResultData */
        0x04, 0x05, 0x06, 0x07,
        0xAF, 0xAE, 0xAD, 0xAC
    };

    const guint8 expected_databuffer [] = {
        0x00, 0x00, 0x00, 0x00,
        0x04, 0x05, 0x06, 0x07,
        0xAF, 0xAE, 0xAD, 0xAC
    };

    response = mbim_message_new (buffer, sizeof (buffer));

    g_assert (mbim_message_stk_terminal_response_response_parse (
                  response,
                  &databuffer_len,
                  &databuffer,
                  &status_words,
                  &error));
    g_assert_no_error (error);

    g_assert_cmpuint (status_words, ==, 204);

    test_message_trace (databuffer,
                        databuffer_len,
                        expected_databuffer,
                        sizeof (expected_databuffer));
    g_assert_cmpuint (databuffer_len, ==, sizeof (expected_databuffer));
    g_assert (memcmp (databuffer, expected_databuffer, sizeof (expected_databuffer)) == 0);

    mbim_message_unref (response);
}

static void
test_message_parser_stk_envelope_response (void)
{
    const guint8 *databuffer;
    MbimMessage *response;
    GError *error = NULL;
    const guint8 buffer [] =  {
        /* header */
        0x03, 0x00, 0x00, 0x80, /* type */
        0x50, 0x01, 0x00, 0x00, /* length */
        0x02, 0x00, 0x00, 0x00, /* transaction id */
        /* fragment header */
        0x01, 0x00, 0x00, 0x00, /* total */
        0x00, 0x00, 0x00, 0x00, /* current */
        /* indicate_status_message */
        0xD8, 0xF2, 0x01, 0x31, /* service id */
        0xFC, 0xB5, 0x4E, 0x17,
        0x86, 0x02, 0xD6, 0xED,
        0x38, 0x16, 0x16, 0x4C,
        0x03, 0x00, 0x00, 0x00, /* command id */
        0x00, 0x00, 0x00, 0x00, /* status code */
        0x20, 0x00, 0x00, 0x00, /* buffer length */
        /* information buffer */
        0x0C, 0x00, 0x00, 0x00,
        0x0C, 0x00, 0x00, 0x00,
        0xCC, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x0C, 0x00, 0x00, 0x00,
        0x0C, 0x00, 0x00, 0x00,
        0xCC, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };

    const guint8 expected_databuffer [] = {
        0x0C, 0x00, 0x00, 0x00,
        0x0C, 0x00, 0x00, 0x00,
        0xCC, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00,
        0x0C, 0x00, 0x00, 0x00,
        0x0C, 0x00, 0x00, 0x00,
        0xCC, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00
    };

    response = mbim_message_new (buffer, sizeof (buffer));

    g_assert (mbim_message_stk_envelope_response_parse (
                  response,
                  &databuffer,
                  &error));
    g_assert_no_error (error);

    test_message_trace (databuffer,
                        sizeof (expected_databuffer),
                        expected_databuffer,
                        sizeof (expected_databuffer));
    g_assert (memcmp (databuffer, expected_databuffer, sizeof (expected_databuffer)) == 0);

    mbim_message_unref (response);
}

static void
test_message_parser_basic_connect_ip_packet_filters_none (void)
{
    MbimPacketFilter **filters = NULL;
    guint32 n_filters;
    guint32 session_id;
    MbimMessage *response;
    GError *error = NULL;
    const guint8 buffer [] =  {
        /* header */
        0x03, 0x00, 0x00, 0x80, /* type */
        0x38, 0x01, 0x00, 0x00, /* length */
        0x02, 0x00, 0x00, 0x00, /* transaction id */
        /* fragment header */
        0x01, 0x00, 0x00, 0x00, /* total */
        0x00, 0x00, 0x00, 0x00, /* current */
        /* indicate_status_message */
        0xA2, 0x89, 0xCC, 0x33, /* service id */
        0xBC, 0xBB, 0x8B, 0x4F,
        0xB6, 0xB0, 0x13, 0x3E,
        0xC2, 0xAA, 0xE6, 0xDF,
        0x17, 0x00, 0x00, 0x00, /* command id */
        0x00, 0x00, 0x00, 0x00, /* status code */
        0x08, 0x00, 0x00, 0x00, /* buffer length */
        /* information buffer */
        0x01, 0x00, 0x00, 0x00, /* session id */
        0x00, 0x00, 0x00, 0x00  /* packet filters count */
    };

    response = mbim_message_new (buffer, sizeof (buffer));

    g_assert (mbim_message_ip_packet_filters_response_parse (
                  response,
                  &session_id,
                  &n_filters,
                  &filters,
                  &error));
    g_assert_no_error (error);

    g_assert_cmpuint (session_id, ==, 1);
    g_assert_cmpuint (n_filters, ==, 0);
    g_assert (filters == NULL);

    mbim_message_unref (response);
}

static void
test_message_parser_basic_connect_ip_packet_filters_one (void)
{
    MbimPacketFilter **filters = NULL;
    guint32 n_filters;
    guint32 session_id;
    MbimMessage *response;
    GError *error = NULL;
    const guint8 buffer [] =  {
        /* header */
        0x03, 0x00, 0x00, 0x80, /* type */
        0x5C, 0x01, 0x00, 0x00, /* length */
        0x02, 0x00, 0x00, 0x00, /* transaction id */
        /* fragment header */
        0x01, 0x00, 0x00, 0x00, /* total */
        0x00, 0x00, 0x00, 0x00, /* current */
        /* indicate_status_message */
        0xA2, 0x89, 0xCC, 0x33, /* service id */
        0xBC, 0xBB, 0x8B, 0x4F,
        0xB6, 0xB0, 0x13, 0x3E,
        0xC2, 0xAA, 0xE6, 0xDF,
        0x17, 0x00, 0x00, 0x00, /* command id */
        0x00, 0x00, 0x00, 0x00, /* status code */
        0x2C, 0x00, 0x00, 0x00, /* buffer length */
        /* information buffer */
        0x01, 0x00, 0x00, 0x00, /* 0x00 session id */
        0x01, 0x00, 0x00, 0x00, /* 0x04 packet filters count */
        0x10, 0x00, 0x00, 0x00, /* 0x08 packet filter 1 offset */
        0x1C, 0x00, 0x00, 0x00, /* 0x0C packet filter 1 length */
        /* databuffer, packet filter 1 */
        0x08, 0x00, 0x00, 0x00, /* 0x10 0x00 filter size */
        0x0C, 0x00, 0x00, 0x00, /* 0x14 0x04 filter offset (from beginning of struct) */
        0x14, 0x00, 0x00, 0x00, /* 0x18 0x08 mask offset (from beginning of struct) */
        0x01, 0x02, 0x03, 0x04, /* 0x1C 0x0C filter */
        0x05, 0x06, 0x07, 0x08,
        0xF1, 0xF2, 0xF3, 0xF4, /* 0x24 0x14 mask */
        0xF5, 0xF6, 0xF7, 0xF8,
    };

    const guint8 expected_filter[] = {
        0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08,
    };
    const guint8 expected_mask[] = {
        0xF1, 0xF2, 0xF3, 0xF4,
        0xF5, 0xF6, 0xF7, 0xF8,
    };

    response = mbim_message_new (buffer, sizeof (buffer));

    g_assert (mbim_message_ip_packet_filters_response_parse (
                  response,
                  &session_id,
                  &n_filters,
                  &filters,
                  &error));
    g_assert_no_error (error);

    g_assert_cmpuint (session_id, ==, 1);
    g_assert_cmpuint (n_filters, ==, 1);
    g_assert (filters != NULL);

    g_assert_cmpuint (filters[0]->filter_size, ==, 8);

    test_message_trace (filters[0]->packet_filter, 8,
                        expected_filter,
                        sizeof (expected_filter));
    g_assert (memcmp (filters[0]->packet_filter, expected_filter, sizeof (expected_filter)) == 0);

    test_message_trace (filters[0]->packet_mask, 8,
                        expected_mask,
                        sizeof (expected_mask));
    g_assert (memcmp (filters[0]->packet_mask, expected_mask, sizeof (expected_mask)) == 0);

    mbim_packet_filter_array_free (filters);

    mbim_message_unref (response);
}

static void
test_message_parser_basic_connect_ip_packet_filters_two (void)
{
    MbimPacketFilter **filters = NULL;
    guint32 n_filters;
    guint32 session_id;
    MbimMessage *response;
    GError *error = NULL;
    const guint8 buffer [] =  {
        /* header */
        0x03, 0x00, 0x00, 0x80, /* type */
        0x88, 0x01, 0x00, 0x00, /* length */
        0x02, 0x00, 0x00, 0x00, /* transaction id */
        /* fragment header */
        0x01, 0x00, 0x00, 0x00, /* total */
        0x00, 0x00, 0x00, 0x00, /* current */
        /* indicate_status_message */
        0xA2, 0x89, 0xCC, 0x33, /* service id */
        0xBC, 0xBB, 0x8B, 0x4F,
        0xB6, 0xB0, 0x13, 0x3E,
        0xC2, 0xAA, 0xE6, 0xDF,
        0x17, 0x00, 0x00, 0x00, /* command id */
        0x00, 0x00, 0x00, 0x00, /* status code */
        0x58, 0x00, 0x00, 0x00, /* buffer length */
        /* information buffer */
        0x01, 0x00, 0x00, 0x00, /* 0x00 session id */
        0x02, 0x00, 0x00, 0x00, /* 0x04 packet filters count */
        0x18, 0x00, 0x00, 0x00, /* 0x08 packet filter 1 offset */
        0x1C, 0x00, 0x00, 0x00, /* 0x0C packet filter 1 length */
        0x34, 0x00, 0x00, 0x00, /* 0x10 packet filter 2 offset */
        0x24, 0x00, 0x00, 0x00, /* 0x14 packet filter 2 length */
        /* databuffer, packet filter 2 */
        0x08, 0x00, 0x00, 0x00, /* 0x18 0x00 filter size */
        0x0C, 0x00, 0x00, 0x00, /* 0x1C 0x04 filter offset (from beginning of struct) */
        0x14, 0x00, 0x00, 0x00, /* 0x20 0x08 mask offset (from beginning of struct) */
        0x01, 0x02, 0x03, 0x04, /* 0x24 0x0C filter */
        0x05, 0x06, 0x07, 0x08,
        0xF1, 0xF2, 0xF3, 0xF4, /* 0x2C 0x14 mask */
        0xF5, 0xF6, 0xF7, 0xF8,
        /* databuffer, packet filter 2 */
        0x0C, 0x00, 0x00, 0x00, /* 0x34 0x00 filter size */
        0x0C, 0x00, 0x00, 0x00, /* 0x38 0x04 filter offset (from beginning of struct) */
        0x18, 0x00, 0x00, 0x00, /* 0x3C 0x08 mask offset (from beginning of struct) */
        0x01, 0x02, 0x03, 0x04, /* 0x40 0x0C filter */
        0x05, 0x06, 0x07, 0x08,
        0x05, 0x06, 0x07, 0x08,
        0xF1, 0xF2, 0xF3, 0xF4, /* 0x4C 0x18 mask */
        0xF5, 0xF6, 0xF7, 0xF8,
        0xF5, 0xF6, 0xF7, 0xF8,
    };

    const guint8 expected_filter1[] = {
        0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08,
    };
    const guint8 expected_mask1[] = {
        0xF1, 0xF2, 0xF3, 0xF4,
        0xF5, 0xF6, 0xF7, 0xF8,
    };
    const guint8 expected_filter2[] = {
        0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08,
        0x05, 0x06, 0x07, 0x08,
    };
    const guint8 expected_mask2[] = {
        0xF1, 0xF2, 0xF3, 0xF4,
        0xF5, 0xF6, 0xF7, 0xF8,
        0xF5, 0xF6, 0xF7, 0xF8,
    };

    response = mbim_message_new (buffer, sizeof (buffer));

    g_assert (mbim_message_ip_packet_filters_response_parse (
                  response,
                  &session_id,
                  &n_filters,
                  &filters,
                  &error));
    g_assert_no_error (error);

    g_assert_cmpuint (session_id, ==, 1);
    g_assert_cmpuint (n_filters, ==, 2);
    g_assert (filters != NULL);

    g_assert_cmpuint (filters[0]->filter_size, ==, 8);
    test_message_trace (filters[0]->packet_filter, 8,
                        expected_filter1,
                        sizeof (expected_filter1));
    g_assert (memcmp (filters[0]->packet_filter, expected_filter1, sizeof (expected_filter1)) == 0);
    test_message_trace (filters[0]->packet_mask, 8,
                        expected_mask1,
                        sizeof (expected_mask1));
    g_assert (memcmp (filters[0]->packet_mask, expected_mask1, sizeof (expected_mask1)) == 0);

    g_assert_cmpuint (filters[1]->filter_size, ==, 12);
    test_message_trace (filters[1]->packet_filter, 12,
                        expected_filter2,
                        sizeof (expected_filter2));
    g_assert (memcmp (filters[1]->packet_filter, expected_filter2, sizeof (expected_filter2)) == 0);
    test_message_trace (filters[1]->packet_mask, 12,
                        expected_mask2,
                        sizeof (expected_mask2));
    g_assert (memcmp (filters[1]->packet_mask, expected_mask2, sizeof (expected_mask2)) == 0);

    mbim_packet_filter_array_free (filters);

    mbim_message_unref (response);
}

static void
test_message_parser_ms_firmware_id_get (void)
{
    const MbimUuid *firmware_id;
    MbimMessage *response;
    GError *error = NULL;
    const guint8 buffer [] = {
        /* header */
        0x03, 0x00, 0x00, 0x80, /* type */
        0x40, 0x00, 0x00, 0x00, /* length */
        0x02, 0x00, 0x00, 0x00, /* transaction id */
        /* fragment header */
        0x01, 0x00, 0x00, 0x00, /* total */
        0x00, 0x00, 0x00, 0x00, /* current */
        /* command_done_message */
        0xE9, 0xF7, 0xDE, 0xA2, /* service id */
        0xFE, 0xAF, 0x40, 0x09,
        0x93, 0xCE, 0x90, 0xA3,
        0x69, 0x41, 0x03, 0xB6,
        0x01, 0x00, 0x00, 0x00, /* command id */
        0x00, 0x00, 0x00, 0x00, /* status code */
        0x10, 0x00, 0x00, 0x00, /* buffer length */
        /* information buffer */
        0x00, 0x11, 0x22, 0x33, /* firmware id */
        0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xAA, 0xBB,
        0xCC, 0xDD, 0xEE, 0xFF  };

    const MbimUuid expected_firmware_id = {
        .a = { 0x00, 0x11, 0x22, 0x33 },
        .b = { 0x44, 0x55 },
        .c = { 0x66, 0x77 },
        .d = { 0x88, 0x99 },
        .e = { 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF }
    };

    response = mbim_message_new (buffer, sizeof (buffer));

    g_assert (mbim_message_ms_firmware_id_get_response_parse (
                  response,
                  &firmware_id,
                  &error));

    g_assert_no_error (error);

    g_assert (mbim_uuid_cmp (firmware_id, &expected_firmware_id));

    mbim_message_unref (response);
}

int main (int argc, char **argv)
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/libmbim-glib/message/parser/basic-connect/visible-providers", test_message_parser_basic_connect_visible_providers);
    g_test_add_func ("/libmbim-glib/message/parser/basic-connect/subscriber-ready-status", test_message_parser_basic_connect_subscriber_ready_status);
    g_test_add_func ("/libmbim-glib/message/parser/basic-connect/device-caps", test_message_parser_basic_connect_device_caps);
    g_test_add_func ("/libmbim-glib/message/parser/basic-connect/ip-configuration", test_message_parser_basic_connect_ip_configuration);
    g_test_add_func ("/libmbim-glib/message/parser/basic-connect/service-activation", test_message_parser_basic_connect_service_activation);
    g_test_add_func ("/libmbim-glib/message/parser/basic-connect/register-state", test_message_parser_basic_connect_register_state);
    g_test_add_func ("/libmbim-glib/message/parser/sms/read/zero-pdu", test_message_parser_sms_read_zero_pdu);
    g_test_add_func ("/libmbim-glib/message/parser/sms/read/single-pdu", test_message_parser_sms_read_single_pdu);
    g_test_add_func ("/libmbim-glib/message/parser/sms/read/multiple-pdu", test_message_parser_sms_read_multiple_pdu);
    g_test_add_func ("/libmbim-glib/message/parser/ussd", test_message_parser_ussd);
    g_test_add_func ("/libmbim-glib/message/parser/auth/akap", test_message_parser_auth_akap);
    g_test_add_func ("/libmbim-glib/message/parser/stk/pac/notification", test_message_parser_stk_pac_notification);
    g_test_add_func ("/libmbim-glib/message/parser/stk/pac/response", test_message_parser_stk_pac_response);
    g_test_add_func ("/libmbim-glib/message/parser/stk/terminal/response", test_message_parser_stk_terminal_response);
    g_test_add_func ("/libmbim-glib/message/parser/stk/envelope/response", test_message_parser_stk_envelope_response);
    g_test_add_func ("/libmbim-glib/message/parser/basic-connect/ip-packet-filters/none", test_message_parser_basic_connect_ip_packet_filters_none);
    g_test_add_func ("/libmbim-glib/message/parser/basic-connect/ip-packet-filters/one", test_message_parser_basic_connect_ip_packet_filters_one);
    g_test_add_func ("/libmbim-glib/message/parser/basic-connect/ip-packet-filters/two", test_message_parser_basic_connect_ip_packet_filters_two);
    g_test_add_func ("/libmbim-glib/message/parser/ms-firmware-id/get", test_message_parser_ms_firmware_id_get);

    return g_test_run ();
}
