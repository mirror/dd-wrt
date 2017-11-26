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

#include "mbim-cid.h"

static void
test_common (MbimService service,
             guint       cid,
             gboolean    can_set,
             gboolean    can_query,
             gboolean    can_notify)
{
    g_assert (mbim_cid_can_set    (service, cid) == can_set);
    g_assert (mbim_cid_can_query  (service, cid) == can_query);
    g_assert (mbim_cid_can_notify (service, cid) == can_notify);
}

static void
test_cid_basic_connect (void)
{
    test_common (MBIM_SERVICE_BASIC_CONNECT,
                 MBIM_CID_BASIC_CONNECT_DEVICE_CAPS,
                 FALSE, TRUE, FALSE);

    test_common (MBIM_SERVICE_BASIC_CONNECT,
                 MBIM_CID_BASIC_CONNECT_MULTICARRIER_PROVIDERS,
                 TRUE, TRUE, TRUE);
}

static void
test_cid_sms (void)
{
    test_common (MBIM_SERVICE_SMS,
                 MBIM_CID_SMS_CONFIGURATION,
                 TRUE, TRUE, TRUE);

    test_common (MBIM_SERVICE_SMS,
                 MBIM_CID_SMS_MESSAGE_STORE_STATUS,
                 FALSE, TRUE, TRUE);
}

static void
test_cid_ussd (void)
{
    test_common (MBIM_SERVICE_USSD,
                 MBIM_CID_USSD,
                 TRUE, FALSE, TRUE);
}

static void
test_cid_phonebook (void)
{
    test_common (MBIM_SERVICE_PHONEBOOK,
                 MBIM_CID_PHONEBOOK_CONFIGURATION,
                 FALSE, TRUE, TRUE);

    test_common (MBIM_SERVICE_PHONEBOOK,
                 MBIM_CID_PHONEBOOK_WRITE,
                 TRUE, FALSE, FALSE);
}

static void
test_cid_stk (void)
{
    test_common (MBIM_SERVICE_STK,
                 MBIM_CID_STK_PAC,
                 TRUE, TRUE, TRUE);

    test_common (MBIM_SERVICE_STK,
                 MBIM_CID_STK_ENVELOPE,
                 TRUE, TRUE, FALSE);
}

static void
test_cid_auth (void)
{
    test_common (MBIM_SERVICE_AUTH,
                 MBIM_CID_AUTH_AKA,
                 FALSE, TRUE, FALSE);

    test_common (MBIM_SERVICE_AUTH,
                 MBIM_CID_AUTH_SIM,
                 FALSE, TRUE, FALSE);
}

static void
test_cid_dss (void)
{
    test_common (MBIM_SERVICE_DSS,
                 MBIM_CID_DSS_CONNECT,
                 TRUE, FALSE, FALSE);
}

static void
test_cid_ms_firmware_id (void)
{
    test_common (MBIM_SERVICE_MS_FIRMWARE_ID,
                 MBIM_CID_MS_FIRMWARE_ID_GET,
                 FALSE, TRUE, FALSE);
}

static void
test_cid_ms_host_shutdown (void)
{
    test_common (MBIM_SERVICE_MS_HOST_SHUTDOWN,
                 MBIM_CID_MS_HOST_SHUTDOWN_NOTIFY,
                 TRUE, FALSE, FALSE);
}

int main (int argc, char **argv)
{
    g_test_init (&argc, &argv, NULL);

    g_test_add_func ("/libmbim-glib/cid/basic-connect",    test_cid_basic_connect);
    g_test_add_func ("/libmbim-glib/cid/sms",              test_cid_sms);
    g_test_add_func ("/libmbim-glib/cid/ussd",             test_cid_ussd);
    g_test_add_func ("/libmbim-glib/cid/phonebook",        test_cid_phonebook);
    g_test_add_func ("/libmbim-glib/cid/stk",              test_cid_stk);
    g_test_add_func ("/libmbim-glib/cid/auth",             test_cid_auth);
    g_test_add_func ("/libmbim-glib/cid/dss",              test_cid_dss);
    g_test_add_func ("/libmbim-glib/cid/ms-firmware-id",   test_cid_ms_firmware_id);
    g_test_add_func ("/libmbim-glib/cid/ms-host-shutdown", test_cid_ms_host_shutdown);

    return g_test_run ();
}
