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

#include "mbim-cid.h"
#include "mbim-uuid.h"
#include "mbim-enum-types.h"

/**
 * SECTION: mbim-cid
 * @title: Command IDs
 * @short_description: Generic command handling routines.
 *
 * This section defines the interface of the known command IDs.
 */

typedef struct {
    gboolean set;
    gboolean query;
    gboolean notify;
} CidConfig;

#define NO_SET    FALSE
#define NO_QUERY  FALSE
#define NO_NOTIFY FALSE

#define SET    TRUE
#define QUERY  TRUE
#define NOTIFY TRUE

/* Note: index of the array is CID-1 */
#define MBIM_CID_BASIC_CONNECT_LAST MBIM_CID_BASIC_CONNECT_MULTICARRIER_PROVIDERS
static const CidConfig cid_basic_connect_config [MBIM_CID_BASIC_CONNECT_LAST] = {
    { NO_SET, QUERY,    NO_NOTIFY }, /* MBIM_CID_BASIC_CONNECT_DEVICE_CAPS */
    { NO_SET, QUERY,    NOTIFY    }, /* MBIM_CID_BASIC_CONNECT_SUBSCRIBER_READY_STATUS */
    { SET,    QUERY,    NOTIFY    }, /* MBIM_CID_BASIC_CONNECT_RADIO_STATE */
    { SET,    QUERY,    NO_NOTIFY }, /* MBIM_CID_BASIC_CONNECT_PIN */
    { NO_SET, QUERY,    NO_NOTIFY }, /* MBIM_CID_BASIC_CONNECT_PIN_LIST */
    { SET,    QUERY,    NO_NOTIFY }, /* MBIM_CID_BASIC_CONNECT_HOME_PROVIDER */
    { SET,    QUERY,    NOTIFY    }, /* MBIM_CID_BASIC_CONNECT_PREFERRED_PROVIDERS */
    { NO_SET, QUERY,    NO_NOTIFY }, /* MBIM_CID_BASIC_CONNECT_VISIBLE_PROVIDERS */
    { SET,    QUERY,    NOTIFY    }, /* MBIM_CID_BASIC_CONNECT_REGISTER_STATE */
    { SET,    QUERY,    NOTIFY    }, /* MBIM_CID_BASIC_CONNECT_PACKET_SERVICE */
    { SET,    QUERY,    NOTIFY    }, /* MBIM_CID_BASIC_CONNECT_SIGNAL_STATE */
    { SET,    QUERY,    NOTIFY    }, /* MBIM_CID_BASIC_CONNECT_CONNECT */
    { SET,    QUERY,    NOTIFY    }, /* MBIM_CID_BASIC_CONNECT_PROVISIONED_CONTEXTS */
    { SET,    NO_QUERY, NO_NOTIFY }, /* MBIM_CID_BASIC_CONNECT_SERVICE_ACTIVATION */
    { NO_SET, QUERY,    NOTIFY    }, /* MBIM_CID_BASIC_CONNECT_IP_CONFIGURATION */
    { NO_SET, QUERY,    NO_NOTIFY }, /* MBIM_CID_BASIC_CONNECT_DEVICE_SERVICES */
    { NO_SET, NO_QUERY, NO_NOTIFY }, /* 17 reserved */
    { NO_SET, NO_QUERY, NO_NOTIFY }, /* 18 reserved */
    { SET,    NO_QUERY, NO_NOTIFY }, /* MBIM_CID_BASIC_CONNECT_DEVICE_SERVICE_SUBSCRIBE_LIST */
    { NO_SET, QUERY,    NO_NOTIFY }, /* MBIM_CID_BASIC_CONNECT_PACKET_STATISTICS */
    { SET,    QUERY,    NO_NOTIFY }, /* MBIM_CID_BASIC_CONNECT_NETWORK_IDLE_HINT */
    { NO_SET, QUERY,    NOTIFY    }, /* MBIM_CID_BASIC_CONNECT_EMERGENCY_MODE */
    { SET,    QUERY,    NO_NOTIFY }, /* MBIM_CID_BASIC_CONNECT_IP_PACKET_FILTERS */
    { SET,    QUERY,    NOTIFY    }, /* MBIM_CID_BASIC_CONNECT_MULTICARRIER_PROVIDERS */
};

/* Note: index of the array is CID-1 */
#define MBIM_CID_SMS_LAST MBIM_CID_SMS_MESSAGE_STORE_STATUS
static const CidConfig cid_sms_config [MBIM_CID_SMS_LAST] = {
    { SET,    QUERY,    NOTIFY    }, /* MBIM_CID_SMS_CONFIGURATION */
    { NO_SET, QUERY,    NOTIFY    }, /* MBIM_CID_SMS_READ */
    { SET,    NO_QUERY, NO_NOTIFY }, /* MBIM_CID_SMS_SEND */
    { SET,    NO_QUERY, NO_NOTIFY }, /* MBIM_CID_SMS_DELETE */
    { NO_SET, QUERY,    NOTIFY    }, /* MBIM_CID_SMS_MESSAGE_STORE_STATUS */
};

/* Note: index of the array is CID-1 */
#define MBIM_CID_USSD_LAST MBIM_CID_USSD
static const CidConfig cid_ussd_config [MBIM_CID_USSD_LAST] = {
    { SET, NO_QUERY, NOTIFY }, /* MBIM_CID_USSD */
};

/* Note: index of the array is CID-1 */
#define MBIM_CID_PHONEBOOK_LAST MBIM_CID_PHONEBOOK_WRITE
static const CidConfig cid_phonebook_config [MBIM_CID_PHONEBOOK_LAST] = {
    { NO_SET, QUERY,    NOTIFY    }, /* MBIM_CID_PHONEBOOK_CONFIGURATION */
    { NO_SET, QUERY,    NO_NOTIFY }, /* MBIM_CID_PHONEBOOK_READ */
    { SET,    NO_QUERY, NO_NOTIFY }, /* MBIM_CID_PHONEBOOK_DELETE */
    { SET,    NO_QUERY, NO_NOTIFY }, /* MBIM_CID_PHONEBOOK_WRITE */
};

/* Note: index of the array is CID-1 */
#define MBIM_CID_STK_LAST MBIM_CID_STK_ENVELOPE
static const CidConfig cid_stk_config [MBIM_CID_STK_LAST] = {
    { SET, QUERY,    NOTIFY    }, /* MBIM_CID_STK_PAC */
    { SET, NO_QUERY, NO_NOTIFY }, /* MBIM_CID_STK_TERMINAL_RESPONSE */
    { SET, QUERY,    NO_NOTIFY }, /* MBIM_CID_STK_ENVELOPE */
};

/* Note: index of the array is CID-1 */
#define MBIM_CID_AUTH_LAST MBIM_CID_AUTH_SIM
static const CidConfig cid_auth_config [MBIM_CID_AUTH_LAST] = {
    { NO_SET, QUERY, NO_NOTIFY }, /* MBIM_CID_AUTH_AKA */
    { NO_SET, QUERY, NO_NOTIFY }, /* MBIM_CID_AUTH_AKAP */
    { NO_SET, QUERY, NO_NOTIFY }, /* MBIM_CID_AUTH_SIM */
};

/* Note: index of the array is CID-1 */
#define MBIM_CID_DSS_LAST MBIM_CID_DSS_CONNECT
static const CidConfig cid_dss_config [MBIM_CID_DSS_LAST] = {
    { SET, NO_QUERY, NO_NOTIFY }, /* MBIM_CID_DSS_CONNECT */
};

/* Note: index of the array is CID-1 */
#define MBIM_CID_MS_FIRMWARE_ID_LAST MBIM_CID_MS_FIRMWARE_ID_GET
static const CidConfig cid_ms_firmware_id_config [MBIM_CID_MS_FIRMWARE_ID_LAST] = {
    { NO_SET, QUERY, NO_NOTIFY }, /* MBIM_CID_MS_FIRMWARE_ID_GET */
};

/* Note: index of the array is CID-1 */
#define MBIM_CID_MS_HOST_SHUTDOWN_LAST MBIM_CID_MS_HOST_SHUTDOWN_NOTIFY
static const CidConfig cid_ms_host_shutdown_config [MBIM_CID_MS_HOST_SHUTDOWN_LAST] = {
    { SET, NO_QUERY, NO_NOTIFY }, /* MBIM_CID_MS_HOST_SHUTDOWN_NOTIFY */
};

/* Note: index of the array is CID-1 */
#define MBIM_CID_PROXY_CONTROL_LAST MBIM_CID_PROXY_CONTROL_CONFIGURATION
static const CidConfig cid_proxy_control_config [MBIM_CID_PROXY_CONTROL_LAST] = {
    { SET, NO_QUERY, NO_NOTIFY }, /* MBIM_CID_PROXY_CONTROL_CONFIGURATION */
};

/* Note: index of the array is CID-1 */
#define MBIM_CID_QMI_LAST MBIM_CID_QMI_MSG
static const CidConfig cid_qmi_config [MBIM_CID_QMI_LAST] = {
    { SET, NO_QUERY, NO_NOTIFY }, /* MBIM_CID_QMI_MSG */
};

/* Note: index of the array is CID-1 */
#define MBIM_CID_ATDS_LAST MBIM_CID_ATDS_REGISTER_STATE
static const CidConfig cid_atds_config [MBIM_CID_ATDS_LAST] = {
    { NO_SET, QUERY, NO_NOTIFY }, /* MBIM_CID_ATDS_SIGNAL */
    { NO_SET, QUERY, NO_NOTIFY }, /* MBIM_CID_ATDS_LOCATION */
    { SET,    QUERY, NO_NOTIFY }, /* MBIM_CID_ATDS_OPERATORS */
    { SET,    QUERY, NO_NOTIFY }, /* MBIM_CID_ATDS_RAT */
    { NO_SET, QUERY, NO_NOTIFY }, /* MBIM_CID_ATDS_REGISTER_STATE */
};

/**
 * mbim_cid_can_set:
 * @service: a #MbimService.
 * @cid: a command ID.
 *
 * Checks whether the given command allows setting.
 *
 * Returns: %TRUE if the command allows setting, %FALSE otherwise.
 */
gboolean
mbim_cid_can_set (MbimService service,
                  guint       cid)
{
    /* CID = 0 is never a valid command */
    g_return_val_if_fail (cid > 0, FALSE);
    /* Known service required */
    g_return_val_if_fail (service > MBIM_SERVICE_INVALID, FALSE);
    g_return_val_if_fail (service < MBIM_SERVICE_LAST, FALSE);

    switch (service) {
    case MBIM_SERVICE_BASIC_CONNECT:
        return cid_basic_connect_config[cid - 1].set;
    case MBIM_SERVICE_SMS:
        return cid_sms_config[cid - 1].set;
    case MBIM_SERVICE_USSD:
        return cid_ussd_config[cid - 1].set;
    case MBIM_SERVICE_PHONEBOOK:
        return cid_phonebook_config[cid - 1].set;
    case MBIM_SERVICE_STK:
        return cid_stk_config[cid - 1].set;
    case MBIM_SERVICE_AUTH:
        return cid_auth_config[cid - 1].set;
    case MBIM_SERVICE_DSS:
        return cid_dss_config[cid - 1].set;
    case MBIM_SERVICE_MS_FIRMWARE_ID:
        return cid_ms_firmware_id_config[cid - 1].set;
    case MBIM_SERVICE_MS_HOST_SHUTDOWN:
        return cid_ms_host_shutdown_config[cid - 1].set;
    case MBIM_SERVICE_PROXY_CONTROL:
        return cid_proxy_control_config[cid - 1].set;
    case MBIM_SERVICE_QMI:
        return cid_qmi_config[cid - 1].set;
    case MBIM_SERVICE_ATDS:
        return cid_atds_config[cid - 1].set;
    default:
        g_assert_not_reached ();
        return FALSE;
    }
}

/**
 * mbim_cid_can_query:
 * @service: a #MbimService.
 * @cid: a command ID.
 *
 * Checks whether the given command allows querying.
 *
 * Returns: %TRUE if the command allows querying, %FALSE otherwise.
 */
gboolean
mbim_cid_can_query (MbimService service,
                    guint       cid)
{
    /* CID = 0 is never a valid command */
    g_return_val_if_fail (cid > 0, FALSE);
    /* Known service required */
    g_return_val_if_fail (service > MBIM_SERVICE_INVALID, FALSE);
    g_return_val_if_fail (service < MBIM_SERVICE_LAST, FALSE);

    switch (service) {
    case MBIM_SERVICE_BASIC_CONNECT:
        return cid_basic_connect_config[cid - 1].query;
    case MBIM_SERVICE_SMS:
        return cid_sms_config[cid - 1].query;
    case MBIM_SERVICE_USSD:
        return cid_ussd_config[cid - 1].query;
    case MBIM_SERVICE_PHONEBOOK:
        return cid_phonebook_config[cid - 1].query;
    case MBIM_SERVICE_STK:
        return cid_stk_config[cid - 1].query;
    case MBIM_SERVICE_AUTH:
        return cid_auth_config[cid - 1].query;
    case MBIM_SERVICE_DSS:
        return cid_dss_config[cid - 1].query;
    case MBIM_SERVICE_MS_FIRMWARE_ID:
        return cid_ms_firmware_id_config[cid - 1].query;
    case MBIM_SERVICE_MS_HOST_SHUTDOWN:
        return cid_ms_host_shutdown_config[cid - 1].query;
    case MBIM_SERVICE_PROXY_CONTROL:
        return cid_proxy_control_config[cid - 1].query;
    case MBIM_SERVICE_QMI:
        return cid_qmi_config[cid - 1].query;
    case MBIM_SERVICE_ATDS:
        return cid_atds_config[cid - 1].query;
    default:
        g_assert_not_reached ();
        return FALSE;
    }
}

/**
 * mbim_cid_can_notify:
 * @service: a #MbimService.
 * @cid: a command ID.
 *
 * Checks whether the given command allows notifying.
 *
 * Returns: %TRUE if the command allows notifying, %FALSE otherwise.
 */
gboolean
mbim_cid_can_notify (MbimService service,
                     guint       cid)
{
    /* CID = 0 is never a valid command */
    g_return_val_if_fail (cid > 0, FALSE);
    /* Known service required */
    g_return_val_if_fail (service > MBIM_SERVICE_INVALID, FALSE);
    g_return_val_if_fail (service < MBIM_SERVICE_LAST, FALSE);

    switch (service) {
    case MBIM_SERVICE_BASIC_CONNECT:
        return cid_basic_connect_config[cid - 1].notify;
    case MBIM_SERVICE_SMS:
        return cid_sms_config[cid - 1].notify;
    case MBIM_SERVICE_USSD:
        return cid_ussd_config[cid - 1].notify;
    case MBIM_SERVICE_PHONEBOOK:
        return cid_phonebook_config[cid - 1].notify;
    case MBIM_SERVICE_STK:
        return cid_stk_config[cid - 1].notify;
    case MBIM_SERVICE_AUTH:
        return cid_auth_config[cid - 1].notify;
    case MBIM_SERVICE_DSS:
        return cid_dss_config[cid - 1].notify;
    case MBIM_SERVICE_MS_FIRMWARE_ID:
        return cid_ms_firmware_id_config[cid - 1].notify;
    case MBIM_SERVICE_MS_HOST_SHUTDOWN:
        return cid_ms_host_shutdown_config[cid - 1].notify;
    case MBIM_SERVICE_PROXY_CONTROL:
        return cid_proxy_control_config[cid - 1].notify;
    case MBIM_SERVICE_QMI:
        return cid_qmi_config[cid - 1].notify;
    case MBIM_SERVICE_ATDS:
        return cid_atds_config[cid - 1].notify;
    default:
        g_assert_not_reached ();
        return FALSE;
    }
}

/**
 * mbim_cid_get_printable:
 * @service: a #MbimService.
 * @cid: a command ID.
 *
 * Gets a printable string for the command specified by the @service and the
 * @cid.
 *
 * Returns: (transfer none): a constant string.
 */
const gchar *
mbim_cid_get_printable (MbimService service,
                        guint       cid)
{
    /* CID = 0 is never a valid command */
    g_return_val_if_fail (cid > 0, NULL);
    /* Known service required */
    g_return_val_if_fail (service >= MBIM_SERVICE_INVALID, NULL);
    g_return_val_if_fail (service < MBIM_SERVICE_LAST, NULL);

    switch (service) {
    case MBIM_SERVICE_INVALID:
        return "invalid";
    case MBIM_SERVICE_BASIC_CONNECT:
        return mbim_cid_basic_connect_get_string (cid);
    case MBIM_SERVICE_SMS:
        return mbim_cid_sms_get_string (cid);
    case MBIM_SERVICE_USSD:
        return mbim_cid_ussd_get_string (cid);
    case MBIM_SERVICE_PHONEBOOK:
        return mbim_cid_phonebook_get_string (cid);
    case MBIM_SERVICE_STK:
        return mbim_cid_stk_get_string (cid);
    case MBIM_SERVICE_AUTH:
        return mbim_cid_auth_get_string (cid);
    case MBIM_SERVICE_DSS:
        return mbim_cid_dss_get_string (cid);
    case MBIM_SERVICE_MS_FIRMWARE_ID:
        return mbim_cid_ms_firmware_id_get_string (cid);
    case MBIM_SERVICE_MS_HOST_SHUTDOWN:
        return mbim_cid_ms_host_shutdown_get_string (cid);
    case MBIM_SERVICE_PROXY_CONTROL:
        return mbim_cid_proxy_control_get_string (cid);
    case MBIM_SERVICE_QMI:
        return mbim_cid_qmi_get_string (cid);
    case MBIM_SERVICE_ATDS:
        return mbim_cid_atds_get_string (cid);
    default:
        g_assert_not_reached ();
        return NULL;
    }
}
