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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>

#include <glib.h>
#include <gio/gio.h>

#include <libqmi-glib.h>

#include "qmicli.h"

/* Context */
typedef struct {
    QmiDevice *device;
    QmiClientNas *client;
    GCancellable *cancellable;
} Context;
static Context *ctx;

/* Options */
static gboolean get_signal_strength_flag;
static gboolean get_signal_info_flag;
static gboolean get_home_network_flag;
static gchar *set_roaming_str;
static gchar *set_lte_str;
static gboolean get_serving_system_flag;
static gboolean get_system_info_flag;
static gboolean get_technology_preference_flag;
static gboolean get_system_selection_preference_flag;
static gboolean network_scan_flag;
static gboolean reset_flag;
static gboolean noop_flag;

static GOptionEntry entries[] = {
    { "nas-get-signal-strength", 0, 0, G_OPTION_ARG_NONE, &get_signal_strength_flag,
      "Get signal strength",
      NULL
    },
    { "nas-get-signal-info", 0, 0, G_OPTION_ARG_NONE, &get_signal_info_flag,
      "Get signal info",
      NULL
    },
    { "nas-get-home-network", 0, 0, G_OPTION_ARG_NONE, &get_home_network_flag,
      "Get home network",
      NULL
    },
    { "nas-set-roaming", 0, 0, G_OPTION_ARG_STRING, &set_roaming_str,
      "Set Roaming",
      "OFF|ANY",
    },
    { "nas-set-network-mode", 0, 0, G_OPTION_ARG_STRING, &set_lte_str,
      "Set Network Mode",
      "LTE|LTEUMTS|UMTSGSM|GSMUMTS|GSM|ANY",
    },
    { "nas-get-serving-system", 0, 0, G_OPTION_ARG_NONE, &get_serving_system_flag,
      "Get serving system",
      NULL
    },
    { "nas-get-system-info", 0, 0, G_OPTION_ARG_NONE, &get_system_info_flag,
      "Get system info",
      NULL
    },
    { "nas-get-technology-preference", 0, 0, G_OPTION_ARG_NONE, &get_technology_preference_flag,
      "Get technology preference",
      NULL
    },
    { "nas-get-system-selection-preference", 0, 0, G_OPTION_ARG_NONE, &get_system_selection_preference_flag,
      "Get system selection preference",
      NULL
    },
    { "nas-network-scan", 0, 0, G_OPTION_ARG_NONE, &network_scan_flag,
      "Scan networks",
      NULL
    },
    { "nas-reset", 0, 0, G_OPTION_ARG_NONE, &reset_flag,
      "Reset the service state",
      NULL
    },
    { "nas-noop", 0, 0, G_OPTION_ARG_NONE, &noop_flag,
      "Just allocate or release a NAS client. Use with `--client-no-release-cid' and/or `--client-cid'",
      NULL
    },
    { NULL }
};

GOptionGroup *
qmicli_nas_get_option_group (void)
{
	GOptionGroup *group;

	group = g_option_group_new ("nas",
	                            "NAS options",
	                            "Show Network Access Service options",
	                            NULL,
	                            NULL);
	g_option_group_add_entries (group, entries);

	return group;
}

gboolean
qmicli_nas_options_enabled (void)
{
    static guint n_actions = 0;
    static gboolean checked = FALSE;

    if (checked)
        return !!n_actions;

    n_actions = (get_signal_strength_flag +
                 get_signal_info_flag +
                 get_home_network_flag +
                 !!set_roaming_str +
                 !!set_lte_str +
                 get_serving_system_flag +
                 get_system_info_flag +
                 get_technology_preference_flag +
                 get_system_selection_preference_flag +
                 network_scan_flag +
                 reset_flag +
                 noop_flag);

    if (n_actions > 1) {
        g_printerr ("error: too many NAS actions requested\n");
        exit (EXIT_FAILURE);
    }

    checked = TRUE;
    return !!n_actions;
}

static void
context_free (Context *context)
{
    if (!context)
        return;

    if (context->cancellable)
        g_object_unref (context->cancellable);
    if (context->device)
        g_object_unref (context->device);
    if (context->client)
        g_object_unref (context->client);
    g_slice_free (Context, context);
}

static void
shutdown (gboolean operation_status)
{
    /* Cleanup context and finish async operation */
    context_free (ctx);
    qmicli_async_operation_done (operation_status);
}

static gdouble
get_db_from_sinr_level (QmiNasEvdoSinrLevel level)
{
    switch (level) {
    case QMI_NAS_EVDO_SINR_LEVEL_0: return -9.0;
    case QMI_NAS_EVDO_SINR_LEVEL_1: return -6;
    case QMI_NAS_EVDO_SINR_LEVEL_2: return -4.5;
    case QMI_NAS_EVDO_SINR_LEVEL_3: return -3;
    case QMI_NAS_EVDO_SINR_LEVEL_4: return -2;
    case QMI_NAS_EVDO_SINR_LEVEL_5: return 1;
    case QMI_NAS_EVDO_SINR_LEVEL_6: return 3;
    case QMI_NAS_EVDO_SINR_LEVEL_7: return 6;
    case QMI_NAS_EVDO_SINR_LEVEL_8: return +9;
    default:
        g_warning ("Invalid SINR level '%u'", level);
        return -G_MAXDOUBLE;
    }
}

static void
get_signal_info_ready (QmiClientNas *client,
                       GAsyncResult *res)
{
    QmiMessageNasGetSignalInfoOutput *output;
    GError *error = NULL;
    gint8 rssi;
    gint16 ecio;
    QmiNasEvdoSinrLevel sinr_level;
    gint32 io;
    gint8 rsrq;
    gint16 rsrp;
    gint16 snr;
    gint8 rscp;

    output = qmi_client_nas_get_signal_info_finish (client, res, &error);
    if (!output) {
        g_printerr ("error: operation failed: %s\n", error->message);
        g_error_free (error);
        shutdown (FALSE);
        return;
    }

    if (!qmi_message_nas_get_signal_info_output_get_result (output, &error)) {
        g_printerr ("error: couldn't get signal info: %s\n", error->message);
        g_error_free (error);
        qmi_message_nas_get_signal_info_output_unref (output);
        shutdown (FALSE);
        return;
    }

    g_print ("[%s] Successfully got signal info\n",
             qmi_device_get_path_display (ctx->device));

    /* CDMA... */
    if (qmi_message_nas_get_signal_info_output_get_cdma_signal_strength (output,
                                                                         &rssi,
                                                                         &ecio,
                                                                         NULL)) {
        g_print ("CDMA:\n"
                 "\tRSSI: '%d dBm'\n"
                 "\tECIO: '%.1lf dBm'\n",
                 rssi,
                 (-0.5)*((gdouble)ecio));
    }

    /* HDR... */
    if (qmi_message_nas_get_signal_info_output_get_hdr_signal_strength (output,
                                                                        &rssi,
                                                                        &ecio,
                                                                        &sinr_level,
                                                                        &io,
                                                                        NULL)) {
        g_print ("HDR:\n"
                 "\tRSSI: '%d dBm'\n"
                 "\tECIO: '%.1lf dBm'\n"
                 "\tSINR (%u): '%.1lf dB'\n"
                 "\tIO: '%d dBm'\n",
                 rssi,
                 (-0.5)*((gdouble)ecio),
                 sinr_level, get_db_from_sinr_level (sinr_level),
                 io);
    }

    /* GSM */
    if (qmi_message_nas_get_signal_info_output_get_gsm_signal_strength (output,
                                                                        &rssi,
                                                                        NULL)) {
        g_print ("GSM:\n"
                 "\tRSSI: '%d dBm'\n",
                 rssi);
    }

    /* WCDMA... */
    if (qmi_message_nas_get_signal_info_output_get_wcdma_signal_strength (output,
                                                                          &rssi,
                                                                          &ecio,
                                                                          NULL)) {
        g_print ("WCDMA:\n"
                 "\tRSSI: '%d dBm'\n"
                 "\tECIO: '%.1lf dBm'\n",
                 rssi,
                 (-0.5)*((gdouble)ecio));
    }

    /* LTE... */
    if (qmi_message_nas_get_signal_info_output_get_lte_signal_strength (output,
                                                                        &rssi,
                                                                        &rsrq,
                                                                        &rsrp,
                                                                        &snr,
                                                                        NULL)) {
        g_print ("LTE:\n"
                 "\tRSSI: '%d dBm'\n"
                 "\tRSRQ: '%d dB'\n"
                 "\tRSRP: '%d dBm'\n"
                 "\tSNR: '%.1lf dBm'\n",
                 rssi,
                 rsrq,
                 rsrp,
                 (0.1) * ((gdouble)snr));
    }

    /* TDMA */
    if (qmi_message_nas_get_signal_info_output_get_tdma_signal_strength (output,
                                                                         &rscp,
                                                                         NULL)) {
        g_print ("TDMA:\n"
                 "\tRSCP: '%d dBm'\n",
                 rscp);
    }

    qmi_message_nas_get_signal_info_output_unref (output);
    shutdown (TRUE);
}

static QmiMessageNasGetSignalStrengthInput *
get_signal_strength_input_create (void)
{
    GError *error = NULL;
    QmiMessageNasGetSignalStrengthInput *input;
    QmiNasSignalStrengthRequest mask;

    mask = (QMI_NAS_SIGNAL_STRENGTH_REQUEST_RSSI |
            QMI_NAS_SIGNAL_STRENGTH_REQUEST_ECIO |
            QMI_NAS_SIGNAL_STRENGTH_REQUEST_IO |
            QMI_NAS_SIGNAL_STRENGTH_REQUEST_SINR |
            QMI_NAS_SIGNAL_STRENGTH_REQUEST_RSRQ |
            QMI_NAS_SIGNAL_STRENGTH_REQUEST_LTE_SNR |
            QMI_NAS_SIGNAL_STRENGTH_REQUEST_LTE_RSRP);

    input = qmi_message_nas_get_signal_strength_input_new ();
    if (!qmi_message_nas_get_signal_strength_input_set_request_mask (
            input,
            mask,
            &error)) {
        g_printerr ("error: couldn't create input data bundle: '%s'\n",
                    error->message);
        g_error_free (error);
        qmi_message_nas_get_signal_strength_input_unref (input);
        input = NULL;
    }

    return input;
}

static void
get_signal_strength_ready (QmiClientNas *client,
                           GAsyncResult *res)
{
    QmiMessageNasGetSignalStrengthOutput *output;
    GError *error = NULL;
    GArray *array;
    QmiNasRadioInterface radio_interface;
    gint8 strength;
    gint32 io;
    QmiNasEvdoSinrLevel sinr_level;
    gint8 rsrq;
    gint16 rsrp;
    gint16 snr;

    output = qmi_client_nas_get_signal_strength_finish (client, res, &error);
    if (!output) {
        g_printerr ("error: operation failed: %s\n", error->message);
        g_error_free (error);
        shutdown (FALSE);
        return;
    }

    if (!qmi_message_nas_get_signal_strength_output_get_result (output, &error)) {
        g_printerr ("error: couldn't get signal strength: %s\n", error->message);
        g_error_free (error);
        qmi_message_nas_get_signal_strength_output_unref (output);
        shutdown (FALSE);
        return;
    }

    qmi_message_nas_get_signal_strength_output_get_signal_strength (output,
                                                                    &strength,
                                                                    &radio_interface,
                                                                    NULL);

    g_print ("[%s] Successfully got signal strength\n"
             "Current:\n"
             "\tNetwork '%s': '%d dBm'\n",
             qmi_device_get_path_display (ctx->device),
             qmi_nas_radio_interface_get_string (radio_interface),
             strength);

    /* Other signal strengths in other networks... */
    if (qmi_message_nas_get_signal_strength_output_get_strength_list (output, &array, NULL)) {
        guint i;

        g_print ("Other:\n");
        for (i = 0; i < array->len; i++) {
            QmiMessageNasGetSignalStrengthOutputStrengthListElement *element;

            element = &g_array_index (array, QmiMessageNasGetSignalStrengthOutputStrengthListElement, i);
            g_print ("\tNetwork '%s': '%d dBm'\n",
                     qmi_nas_radio_interface_get_string (element->radio_interface),
                     element->strength);
        }
    }

    /* RSSI... */
    if (qmi_message_nas_get_signal_strength_output_get_rssi_list (output, &array, NULL)) {
        guint i;

        g_print ("RSSI:\n");
        for (i = 0; i < array->len; i++) {
            QmiMessageNasGetSignalStrengthOutputRssiListElement *element;

            element = &g_array_index (array, QmiMessageNasGetSignalStrengthOutputRssiListElement, i);
            g_print ("\tNetwork '%s': '%d dBm'\n",
                     qmi_nas_radio_interface_get_string (element->radio_interface),
                     (-1) * element->rssi);
        }
    }

    /* ECIO... */
    if (qmi_message_nas_get_signal_strength_output_get_ecio_list (output, &array, NULL)) {
        guint i;

        g_print ("ECIO:\n");
        for (i = 0; i < array->len; i++) {
            QmiMessageNasGetSignalStrengthOutputEcioListElement *element;

            element = &g_array_index (array, QmiMessageNasGetSignalStrengthOutputEcioListElement, i);
            g_print ("\tNetwork '%s': '%.1lf dBm'\n",
                     qmi_nas_radio_interface_get_string (element->radio_interface),
                     (-0.5) * ((gdouble)element->ecio));
        }
    }

    /* IO... */
    if (qmi_message_nas_get_signal_strength_output_get_io (output, &io, NULL)) {
        g_print ("IO: '%d dBm'\n", io);
    }

    /* SINR level */
    if (qmi_message_nas_get_signal_strength_output_get_sinr (output, &sinr_level, NULL)) {
        g_print ("SINR: (%u) '%.1lf dB'\n",
                 sinr_level, get_db_from_sinr_level (sinr_level));
    }

    /* RSRQ */
    if (qmi_message_nas_get_signal_strength_output_get_rsrq (output, &rsrq, &radio_interface, NULL)) {
        g_print ("RSRQ:\n"
                 "\tNetwork '%s': '%d dB'\n",
                 qmi_nas_radio_interface_get_string (radio_interface),
                 rsrq);
    }

    /* LTE SNR */
    if (qmi_message_nas_get_signal_strength_output_get_lte_snr (output, &snr, NULL)) {
        g_print ("SNR:\n"
                 "\tNetwork '%s': '%.1lf dB'\n",
                 qmi_nas_radio_interface_get_string (QMI_NAS_RADIO_INTERFACE_LTE),
                 (0.1) * ((gdouble)snr));
    }

    /* LTE RSRP */
    if (qmi_message_nas_get_signal_strength_output_get_lte_rsrp (output, &rsrp, NULL)) {
        g_print ("RSRP:\n"
                 "\tNetwork '%s': '%d dBm'\n",
                 qmi_nas_radio_interface_get_string (QMI_NAS_RADIO_INTERFACE_LTE),
                 rsrp);
    }

    /* Just skip others for now */

    qmi_message_nas_get_signal_strength_output_unref (output);
    shutdown (TRUE);
}

static void
set_roaming_ready (QmiClientNas *client,
                        GAsyncResult *res)
{
    QmiMessageNasSetSystemSelectionPreferenceOutput *output;
    GError *error = NULL;

    output = qmi_client_nas_set_system_selection_preference_finish(client, res, &error);
    if (!output) {
        g_printerr ("error: operation failed: %s\n", error->message);
        g_error_free (error);
        shutdown (FALSE);
        return;
    }
    qmi_message_nas_set_system_selection_preference_output_unref (output);
    shutdown (TRUE);
}

static void
get_home_network_ready (QmiClientNas *client,
                        GAsyncResult *res)
{
    QmiMessageNasGetHomeNetworkOutput *output;
    GError *error = NULL;

    output = qmi_client_nas_get_home_network_finish (client, res, &error);
    if (!output) {
        g_printerr ("error: operation failed: %s\n", error->message);
        g_error_free (error);
        shutdown (FALSE);
        return;
    }

    if (!qmi_message_nas_get_home_network_output_get_result (output, &error)) {
        g_printerr ("error: couldn't get home network: %s\n", error->message);
        g_error_free (error);
        qmi_message_nas_get_home_network_output_unref (output);
        shutdown (FALSE);
        return;
    }

    g_print ("[%s] Successfully got home network:\n",
             qmi_device_get_path_display (ctx->device));

    {
        guint16 mcc;
        guint16 mnc;
        const gchar *description;

        qmi_message_nas_get_home_network_output_get_home_network (
            output,
            &mcc,
            &mnc,
            &description,
            NULL);

        g_print ("\tHome network:\n"
                 "\t\tMCC: '%" G_GUINT16_FORMAT"'\n"
                 "\t\tMNC: '%" G_GUINT16_FORMAT"'\n"
                 "\t\tDescription: '%s'\n",
                 mcc,
                 mnc,
                 description);
    }

    {
        guint16 sid;
        guint16 nid;

        if (qmi_message_nas_get_home_network_output_get_home_system_id (
                output,
                &sid,
                &nid,
                NULL)) {
            g_print ("\t\tSID: '%" G_GUINT16_FORMAT"'\n"
                     "\t\tNID: '%" G_GUINT16_FORMAT"'\n",
                     sid,
                     nid);
        }
    }

    {
        guint16 mcc;
        guint16 mnc;

        if (qmi_message_nas_get_home_network_output_get_home_network_3gpp2 (
                output,
                &mcc,
                &mnc,
                NULL, /* display_description */
                NULL, /* description_encoding */
                NULL, /* description */
                NULL)) {
            g_print ("\t3GPP2 Home network (extended):\n"
                     "\t\tMCC: '%" G_GUINT16_FORMAT"'\n"
                     "\t\tMNC: '%" G_GUINT16_FORMAT"'\n",
                     mcc,
                     mnc);

            /* TODO: convert description to UTF-8 and display */
        }
    }

    qmi_message_nas_get_home_network_output_unref (output);
    shutdown (TRUE);
}

static void
get_serving_system_ready (QmiClientNas *client,
                          GAsyncResult *res)
{
    QmiMessageNasGetServingSystemOutput *output;
    GError *error = NULL;

    output = qmi_client_nas_get_serving_system_finish (client, res, &error);
    if (!output) {
        g_printerr ("error: operation failed: %s\n", error->message);
        g_error_free (error);
        shutdown (FALSE);
        return;
    }

    if (!qmi_message_nas_get_serving_system_output_get_result (output, &error)) {
        g_printerr ("error: couldn't get serving system: %s\n", error->message);
        g_error_free (error);
        qmi_message_nas_get_serving_system_output_unref (output);
        shutdown (FALSE);
        return;
    }

    g_print ("[%s] Successfully got serving system:\n",
             qmi_device_get_path_display (ctx->device));

    {
        QmiNasRegistrationState registration_state;
        QmiNasAttachState cs_attach_state;
        QmiNasAttachState ps_attach_state;
        QmiNasNetworkType selected_network;
        GArray *radio_interfaces;
        guint i;

        qmi_message_nas_get_serving_system_output_get_serving_system (
            output,
            &registration_state,
            &cs_attach_state,
            &ps_attach_state,
            &selected_network,
            &radio_interfaces,
            NULL);

        g_print ("\tRegistration state: '%s'\n"
                 "\tCS: '%s'\n"
                 "\tPS: '%s'\n"
                 "\tSelected network: '%s'\n"
                 "\tRadio interfaces: '%u'\n",
                 qmi_nas_registration_state_get_string (registration_state),
                 qmi_nas_attach_state_get_string (cs_attach_state),
                 qmi_nas_attach_state_get_string (ps_attach_state),
                 qmi_nas_network_type_get_string (selected_network),
                 radio_interfaces->len);

        for (i = 0; i < radio_interfaces->len; i++) {
            QmiNasRadioInterface iface;

            iface = g_array_index (radio_interfaces, QmiNasRadioInterface, i);
            g_print ("\t\t[%u]: '%s'\n", i, qmi_nas_radio_interface_get_string (iface));
        }
    }

    {
        QmiNasRoamingIndicatorStatus roaming;

        if (qmi_message_nas_get_serving_system_output_get_roaming_indicator (
                output,
                &roaming,
                NULL)) {
            g_print ("\tRoaming status: '%s'\n",
                     qmi_nas_roaming_indicator_status_get_string (roaming));
        }
    }

    {
        GArray *data_service_capability;

        if (qmi_message_nas_get_serving_system_output_get_data_service_capability (
                output,
                &data_service_capability,
                NULL)) {
            guint i;

            g_print ("\tData service capabilities: '%u'\n",
                     data_service_capability->len);

            for (i = 0; i < data_service_capability->len; i++) {
                QmiNasDataCapability cap;

                cap = g_array_index (data_service_capability, QmiNasDataCapability, i);
                g_print ("\t\t[%u]: '%s'\n", i, qmi_nas_data_capability_get_string (cap));
            }
        }
    }

    {
        guint16 current_plmn_mcc;
        guint16 current_plmn_mnc;
        const gchar *current_plmn_description;

        if (qmi_message_nas_get_serving_system_output_get_current_plmn (
                output,
                &current_plmn_mcc,
                &current_plmn_mnc,
                &current_plmn_description,
                NULL)) {
            g_print ("\tCurrent PLMN:\n"
                     "\t\tMCC: '%" G_GUINT16_FORMAT"'\n"
                     "\t\tMNC: '%" G_GUINT16_FORMAT"'\n"
                     "\t\tDescription: '%s'\n",
                     current_plmn_mcc,
                     current_plmn_mnc,
                     current_plmn_description);
        }
    }

    {
        guint16 sid;
        guint16 nid;

        if (qmi_message_nas_get_serving_system_output_get_cdma_system_id (
                output,
                &sid,
                &nid,
                NULL)) {
            g_print ("\tCDMA System ID:\n"
                     "\t\tSID: '%" G_GUINT16_FORMAT"'\n"
                     "\t\tNID: '%" G_GUINT16_FORMAT"'\n",
                     sid, nid);
        }
    }

    {
        guint16 id;
        gint32 latitude;
        gint32 longitude;

        if (qmi_message_nas_get_serving_system_output_get_cdma_base_station_info (
                output,
                &id,
                &latitude,
                &longitude,
                NULL)) {
            gdouble latitude_degrees;
            gdouble longitude_degrees;

            /* TODO: give degrees, minutes, seconds */
            latitude_degrees = ((gdouble)latitude * 0.25)/3600.0;
            longitude_degrees = ((gdouble)longitude * 0.25)/3600.0;

            g_print ("\tCDMA Base station info:\n"
                     "\t\tBase station ID: '%" G_GUINT16_FORMAT"'\n"
                     "\t\tLatitude: '%lf'º\n"
                     "\t\tLongitude: '%lf'º\n",
                     id, latitude_degrees, longitude_degrees);
        }
    }

    {
        GArray *roaming_indicators;

        if (qmi_message_nas_get_serving_system_output_get_roaming_indicator_list (
                output,
                &roaming_indicators,
                NULL)) {
            guint i;

            g_print ("\tRoaming indicators: '%u'\n",
                     roaming_indicators->len);

            for (i = 0; i < roaming_indicators->len; i++) {
                QmiMessageNasGetServingSystemOutputRoamingIndicatorListElement *element;

                element = &g_array_index (roaming_indicators, QmiMessageNasGetServingSystemOutputRoamingIndicatorListElement, i);
                g_print ("\t\t[%u]: '%s' (%s)\n",
                         i,
                         qmi_nas_roaming_indicator_status_get_string (element->roaming_indicator),
                         qmi_nas_radio_interface_get_string (element->radio_interface));
            }
        }
    }

    {
        QmiNasRoamingIndicatorStatus roaming;

        if (qmi_message_nas_get_serving_system_output_get_default_roaming_indicator (
                output,
                &roaming,
                NULL)) {
            g_print ("\tDefault roaming status: '%s'\n",
                     qmi_nas_roaming_indicator_status_get_string (roaming));
        }
    }

    {
        guint8 leap_seconds;
        gint8 local_time_offset;
        gboolean daylight_saving_time;

        if (qmi_message_nas_get_serving_system_output_get_time_zone_3gpp2 (
                output,
                &leap_seconds,
                &local_time_offset,
                &daylight_saving_time,
                NULL)) {
            g_print ("\t3GPP2 time zone:\n"
                     "\t\tLeap seconds: '%u' seconds\n"
                     "\t\tLocal time offset: '%d' minutes\n"
                     "\t\tDaylight saving time: '%s'\n",
                     leap_seconds,
                     (gint)local_time_offset * 30,
                     daylight_saving_time ? "yes" : "no");
        }
    }

    {
        guint8 cdma_p_rev;

        if (qmi_message_nas_get_serving_system_output_get_cdma_p_rev (
                output,
                &cdma_p_rev,
                NULL)) {
            g_print ("\tCDMA P_Rev: '%u'\n", cdma_p_rev);
        }
    }

    {
        gint8 time_zone;

        if (qmi_message_nas_get_serving_system_output_get_time_zone_3gpp (
                output,
                &time_zone,
                NULL)) {
            g_print ("\t3GPP time zone offset: '%d' minutes\n",
                     (gint)time_zone * 15);
        }
    }

    {
        guint8 adjustment;

        if (qmi_message_nas_get_serving_system_output_get_daylight_saving_time_adjustment_3gpp (
                output,
                &adjustment,
                NULL)) {
            g_print ("\t3GPP daylight saving time adjustment: '%u' hours\n",
                     adjustment);
        }
    }

    {
        guint16 lac;

        if (qmi_message_nas_get_serving_system_output_get_lac_3gpp (
                output,
                &lac,
                NULL)) {
            g_print ("\t3GPP location area code: '%" G_GUINT16_FORMAT"'\n", lac);
        }
    }

    {
        guint32 cid;

        if (qmi_message_nas_get_serving_system_output_get_cid_3gpp (
                output,
                &cid,
                NULL)) {
            g_print ("\t3GPP cell ID: '%u'\n", cid);
        }
    }

    {
        gboolean concurrent;

        if (qmi_message_nas_get_serving_system_output_get_concurrent_service_info_3gpp2 (
                output,
                &concurrent,
                NULL)) {
            g_print ("\t3GPP2 concurrent service info: '%s'\n",
                     concurrent ? "available" : "not available");
        }
    }

    {
        gboolean prl;

        if (qmi_message_nas_get_serving_system_output_get_prl_indicator_3gpp2 (
                output,
                &prl,
                NULL)) {
            g_print ("\t3GPP2 PRL indicator: '%s'\n",
                     prl ? "system in PRL" : "system not in PRL");
        }
    }

    {
        gboolean supported;

        if (qmi_message_nas_get_serving_system_output_get_dtm_support (
                output,
                &supported,
                NULL)) {
            g_print ("\tDual transfer mode: '%s'\n",
                     supported ? "supported" : "not supported");
        }
    }

    {
        QmiNasServiceStatus status;
        QmiNasNetworkServiceDomain capability;
        QmiNasServiceStatus hdr_status;
        gboolean hdr_hybrid;
        gboolean forbidden;

        if (qmi_message_nas_get_serving_system_output_get_detailed_service_status (
                output,
                &status,
                &capability,
                &hdr_status,
                &hdr_hybrid,
                &forbidden,
                NULL)) {
            g_print ("\tDetailed status:\n"
                     "\t\tStatus: '%s'\n"
                     "\t\tCapability: '%s'\n"
                     "\t\tHDR Status: '%s'\n"
                     "\t\tHDR Hybrid: '%s'\n"
                     "\t\tForbidden: '%s'\n",
                     qmi_nas_service_status_get_string (status),
                     qmi_nas_network_service_domain_get_string (capability),
                     qmi_nas_service_status_get_string (hdr_status),
                     hdr_hybrid ? "yes" : "no",
                     forbidden ? "yes" : "no");
        }
    }

    {
        guint16 mcc;
        guint8 imsi_11_12;

        if (qmi_message_nas_get_serving_system_output_get_cdma_system_info (
                output,
                &mcc,
                &imsi_11_12,
                NULL)) {
            g_print ("\tCDMA system info:\n"
                     "\t\tMCC: '%" G_GUINT16_FORMAT"'\n"
                     "\t\tIMSI_11_12: '%u'\n",
                     mcc,
                     imsi_11_12);
        }
    }

    {
        QmiNasHdrPersonality personality;

        if (qmi_message_nas_get_serving_system_output_get_hdr_personality (
                output,
                &personality,
                NULL)) {
            g_print ("\tHDR personality: '%s'\n",
                     qmi_nas_hdr_personality_get_string (personality));
        }
    }

    {
        guint16 tac;

        if (qmi_message_nas_get_serving_system_output_get_lte_tac (
                output,
                &tac,
                NULL)) {
            g_print ("\tLTE tracking area code: '%" G_GUINT16_FORMAT"'\n", tac);
        }
    }

    {
        QmiNasCallBarringStatus cs_status;
        QmiNasCallBarringStatus ps_status;

        if (qmi_message_nas_get_serving_system_output_get_call_barring_status (
                output,
                &cs_status,
                &ps_status,
                NULL)) {
            g_print ("\tCall barring status:\n"
                     "\t\tCircuit switched: '%s'\n"
                     "\t\tPacket switched: '%s'\n",
                     qmi_nas_call_barring_status_get_string (cs_status),
                     qmi_nas_call_barring_status_get_string (ps_status));
        }
    }

    {
        guint16 code;

        if (qmi_message_nas_get_serving_system_output_get_umts_primary_scrambling_code (
                output,
                &code,
                NULL)) {
            g_print ("\tUMTS primary scrambling code: '%" G_GUINT16_FORMAT"'\n", code);
        }
    }

    {
        guint16 mcc;
        guint16 mnc;
        gboolean has_pcs_digit;

        if (qmi_message_nas_get_serving_system_output_get_mnc_pcs_digit_include_status (
                output,
                &mcc,
                &mnc,
                &has_pcs_digit,
                NULL)) {
            g_print ("\tFull operator code info:\n"
                     "\t\tMCC: '%" G_GUINT16_FORMAT"'\n"
                     "\t\tMNC: '%" G_GUINT16_FORMAT"'\n"
                     "\t\tMNC with PCS digit: '%s'\n",
                     mcc,
                     mnc,
                     has_pcs_digit ? "yes" : "no");
        }
    }

    qmi_message_nas_get_serving_system_output_unref (output);
    shutdown (TRUE);
}

static void
get_system_info_ready (QmiClientNas *client,
                       GAsyncResult *res)
{
    QmiMessageNasGetSystemInfoOutput *output;
    GError *error = NULL;

    output = qmi_client_nas_get_system_info_finish (client, res, &error);
    if (!output) {
        g_printerr ("error: operation failed: %s\n", error->message);
        g_error_free (error);
        shutdown (FALSE);
        return;
    }

    if (!qmi_message_nas_get_system_info_output_get_result (output, &error)) {
        g_printerr ("error: couldn't get system info: %s\n", error->message);
        g_error_free (error);
        qmi_message_nas_get_system_info_output_unref (output);
        shutdown (FALSE);
        return;
    }

    g_print ("[%s] Successfully got system info:\n",
             qmi_device_get_path_display (ctx->device));

    /* CDMA 1x */
    {
        QmiNasServiceStatus service_status;
        gboolean preferred_data_path;
        gboolean domain_valid;
        QmiNasNetworkServiceDomain domain;
        gboolean service_capability_valid;
        QmiNasNetworkServiceDomain service_capability;
        gboolean roaming_status_valid;
        QmiNasRoamingStatus roaming_status;
        gboolean forbidden_valid;
        gboolean forbidden;
        gboolean prl_match_valid;
        gboolean prl_match;
        gboolean p_rev_valid;
        guint8 p_rev;
        gboolean base_station_p_rev_valid;
        guint8 base_station_p_rev;
        gboolean concurrent_service_support_valid;
        gboolean concurrent_service_support;
        gboolean cdma_system_id_valid;
        guint16 sid;
        guint16 nid;
        gboolean base_station_info_valid;
        guint16 base_station_id;
        gint32 base_station_latitude;
        gint32 base_station_longitude;
        gboolean packet_zone_valid;
        guint16 packet_zone;
        gboolean network_id_valid;
        const gchar *mcc;
        const gchar *mnc;
        guint16 geo_system_index;
        guint16 registration_period;

        if (qmi_message_nas_get_system_info_output_get_cdma_service_status (
                output,
                &service_status,
                &preferred_data_path,
                NULL)) {
            g_print ("\tCDMA 1x service:\n"
                     "\t\tStatus: '%s'\n"
                     "\t\tPreferred data path: '%s'\n",
                     qmi_nas_service_status_get_string (service_status),
                     preferred_data_path ? "yes" : "no");

            if (qmi_message_nas_get_system_info_output_get_cdma_system_info (
                    output,
                    &domain_valid, &domain,
                    &service_capability_valid, &service_capability,
                    &roaming_status_valid, &roaming_status,
                    &forbidden_valid, &forbidden,
                    &prl_match_valid, &prl_match,
                    &p_rev_valid, &p_rev,
                    &base_station_p_rev_valid, &base_station_p_rev,
                    &concurrent_service_support_valid, &concurrent_service_support,
                    &cdma_system_id_valid, &sid, &nid,
                    &base_station_info_valid, &base_station_id, &base_station_longitude, &base_station_latitude,
                    &packet_zone_valid, &packet_zone,
                    &network_id_valid, &mcc, &mnc,
                    NULL)) {
                if (domain_valid)
                    g_print ("\t\tDomain: '%s'\n", qmi_nas_network_service_domain_get_string (domain));
                if (service_capability_valid)
                    g_print ("\t\tService capability: '%s'\n", qmi_nas_network_service_domain_get_string (service_capability));
                if (roaming_status_valid)
                    g_print ("\t\tRoaming status: '%s'\n", qmi_nas_roaming_status_get_string (roaming_status));
                if (forbidden_valid)
                    g_print ("\t\tForbidden: '%s'\n", forbidden ? "yes" : "no");
                if (prl_match_valid)
                    g_print ("\t\tPRL match: '%s'\n", prl_match ? "yes" : "no");
                if (p_rev_valid)
                    g_print ("\t\tP-Rev: '%u'\n", p_rev);
                if (base_station_p_rev_valid)
                    g_print ("\t\tBase station P-Rev: '%u'\n", base_station_p_rev);
                if (concurrent_service_support_valid)
                    g_print ("\t\tConcurrent service support: '%s'\n", concurrent_service_support ? "yes" : "no");
                if (cdma_system_id_valid) {
                    g_print ("\t\tSID: '%" G_GUINT16_FORMAT"'\n", sid);
                    g_print ("\t\tNID: '%" G_GUINT16_FORMAT"'\n", nid);
                }
                if (base_station_info_valid) {
                    gdouble latitude_degrees;
                    gdouble longitude_degrees;

                    /* TODO: give degrees, minutes, seconds */
                    latitude_degrees = ((gdouble)base_station_latitude * 0.25)/3600.0;
                    longitude_degrees = ((gdouble)base_station_longitude * 0.25)/3600.0;
                    g_print ("\t\tBase station ID: '%" G_GUINT16_FORMAT"'\n", base_station_id);
                    g_print ("\t\tBase station latitude: '%lf'º\n", latitude_degrees);
                    g_print ("\t\tBase station longitude: '%lf'º\n", longitude_degrees);
                }
                if (packet_zone_valid)
                    g_print ("\t\tPacket zone: '%" G_GUINT16_FORMAT "'\n", packet_zone);
                if (network_id_valid) {
                    g_print ("\t\tMCC: '%s'\n", mcc);
                    if ((guchar)mnc[2] == 0xFF)
                        g_print ("\t\tMNC: '%.2s'\n", mnc);
                    else
                        g_print ("\t\tMNC: '%.3s'\n", mnc);
                }
            }

            if (qmi_message_nas_get_system_info_output_get_additional_cdma_system_info (
                    output,
                    &geo_system_index,
                    &registration_period,
                    NULL)) {
                if (geo_system_index != 0xFFFF)
                    g_print ("\t\tGeo system index: '%" G_GUINT16_FORMAT "'\n", geo_system_index);
                if (registration_period != 0xFFFF)
                    g_print ("\t\tRegistration period: '%" G_GUINT16_FORMAT "'\n", registration_period);
            }
        }
    }

    /* CDMA 1xEV-DO */
    {
        QmiNasServiceStatus service_status;
        gboolean preferred_data_path;
        gboolean domain_valid;
        QmiNasNetworkServiceDomain domain;
        gboolean service_capability_valid;
        QmiNasNetworkServiceDomain service_capability;
        gboolean roaming_status_valid;
        QmiNasRoamingStatus roaming_status;
        gboolean forbidden_valid;
        gboolean forbidden;
        gboolean prl_match_valid;
        gboolean prl_match;
        gboolean personality_valid;
        QmiNasHdrPersonality personality;
        gboolean protocol_revision_valid;
        QmiNasHdrProtocolRevision protocol_revision;
        gboolean is_856_system_id_valid;
        const gchar *is_856_system_id;
        guint16 geo_system_index;

        if (qmi_message_nas_get_system_info_output_get_hdr_service_status (
                output,
                &service_status,
                &preferred_data_path,
                NULL)) {
            g_print ("\tCDMA 1xEV-DO (HDR) service:\n"
                     "\t\tStatus: '%s'\n"
                     "\t\tPreferred data path: '%s'\n",
                     qmi_nas_service_status_get_string (service_status),
                     preferred_data_path ? "yes" : "no");

            if (qmi_message_nas_get_system_info_output_get_hdr_system_info (
                    output,
                    &domain_valid, &domain,
                    &service_capability_valid, &service_capability,
                    &roaming_status_valid, &roaming_status,
                    &forbidden_valid, &forbidden,
                    &prl_match_valid, &prl_match,
                    &personality_valid, &personality,
                    &protocol_revision_valid, &protocol_revision,
                    &is_856_system_id_valid, &is_856_system_id,
                    NULL)) {
                if (domain_valid)
                    g_print ("\t\tDomain: '%s'\n", qmi_nas_network_service_domain_get_string (domain));
                if (service_capability_valid)
                    g_print ("\t\tService capability: '%s'\n", qmi_nas_network_service_domain_get_string (service_capability));
                if (roaming_status_valid)
                    g_print ("\t\tRoaming status: '%s'\n", qmi_nas_roaming_status_get_string (roaming_status));
                if (forbidden_valid)
                    g_print ("\t\tForbidden: '%s'\n", forbidden ? "yes" : "no");
                if (prl_match_valid)
                    g_print ("\t\tPRL match: '%s'\n", prl_match ? "yes" : "no");
                if (personality_valid)
                    g_print ("\t\tPersonality: '%s'\n", qmi_nas_hdr_personality_get_string (personality));
                if (protocol_revision_valid)
                    g_print ("\t\tProtocol revision: '%s'\n", qmi_nas_hdr_protocol_revision_get_string (protocol_revision));
                if (is_856_system_id_valid)
                    g_print ("\t\tIS-856 system ID: '%s'\n", is_856_system_id);
            }

            if (qmi_message_nas_get_system_info_output_get_additional_hdr_system_info (
                    output,
                    &geo_system_index,
                    NULL)) {
                if (geo_system_index != 0xFFFF)
                    g_print ("\t\tGeo system index: '%" G_GUINT16_FORMAT "'\n", geo_system_index);
            }
        }
    }

    /* GSM */
    {
        QmiNasServiceStatus service_status;
        QmiNasServiceStatus true_service_status;
        gboolean preferred_data_path;
        gboolean domain_valid;
        QmiNasNetworkServiceDomain domain;
        gboolean service_capability_valid;
        QmiNasNetworkServiceDomain service_capability;
        gboolean roaming_status_valid;
        QmiNasRoamingStatus roaming_status;
        gboolean forbidden_valid;
        gboolean forbidden;
        gboolean lac_valid;
        guint16 lac;
        gboolean cid_valid;
        guint32 cid;
        gboolean registration_reject_info_valid;
        QmiNasNetworkServiceDomain registration_reject_domain;
        guint8 registration_reject_cause;
        gboolean network_id_valid;
        const gchar *mcc;
        const gchar *mnc;
        gboolean egprs_support_valid;
        gboolean egprs_support;
        gboolean dtm_support_valid;
        gboolean dtm_support;
        guint16 geo_system_index;
        QmiNasCellBroadcastCapability cell_broadcast_support;
        QmiNasCallBarringStatus call_barring_status_cs;
        QmiNasCallBarringStatus call_barring_status_ps;
        QmiNasNetworkServiceDomain cipher_domain;

        if (qmi_message_nas_get_system_info_output_get_gsm_service_status (
                output,
                &service_status,
                &true_service_status,
                &preferred_data_path,
                NULL)) {
            g_print ("\tGSM service:\n"
                     "\t\tStatus: '%s'\n"
                     "\t\tTrue Status: '%s'\n"
                     "\t\tPreferred data path: '%s'\n",
                     qmi_nas_service_status_get_string (service_status),
                     qmi_nas_service_status_get_string (true_service_status),
                     preferred_data_path ? "yes" : "no");

            if (qmi_message_nas_get_system_info_output_get_gsm_system_info (
                    output,
                    &domain_valid, &domain,
                    &service_capability_valid, &service_capability,
                    &roaming_status_valid, &roaming_status,
                    &forbidden_valid, &forbidden,
                    &lac_valid, &lac,
                    &cid_valid, &cid,
                    &registration_reject_info_valid, &registration_reject_domain, &registration_reject_cause,
                    &network_id_valid, &mcc, &mnc,
                    &egprs_support_valid, &egprs_support,
                    &dtm_support_valid, &dtm_support,
                    NULL)) {
                if (domain_valid)
                    g_print ("\t\tDomain: '%s'\n", qmi_nas_network_service_domain_get_string (domain));
                if (service_capability_valid)
                    g_print ("\t\tService capability: '%s'\n", qmi_nas_network_service_domain_get_string (service_capability));
                if (roaming_status_valid)
                    g_print ("\t\tRoaming status: '%s'\n", qmi_nas_roaming_status_get_string (roaming_status));
                if (forbidden_valid)
                    g_print ("\t\tForbidden: '%s'\n", forbidden ? "yes" : "no");
                if (lac_valid)
                    g_print ("\t\tLocation Area Code: '%" G_GUINT16_FORMAT"'\n", lac);
                if (cid_valid)
                    g_print ("\t\tCell ID: '%" G_GUINT16_FORMAT"'\n", cid);
                if (registration_reject_info_valid)
                    g_print ("\t\tRegistration reject: '%s' (%u)\n",
                             qmi_nas_network_service_domain_get_string (registration_reject_domain),
                             registration_reject_cause);
                if (network_id_valid) {
                    g_print ("\t\tMCC: '%s'\n", mcc);
                    if ((guchar)mnc[2] == 0xFF)
                        g_print ("\t\tMNC: '%.2s'\n", mnc);
                    else
                        g_print ("\t\tMNC: '%.3s'\n", mnc);
                }
                if (egprs_support_valid)
                    g_print ("\t\tE-GPRS supported: '%s'\n", egprs_support ? "yes" : "no");
                if (dtm_support_valid)
                    g_print ("\t\tDual Transfer Mode supported: '%s'\n", dtm_support ? "yes" : "no");
            }

            if (qmi_message_nas_get_system_info_output_get_additional_gsm_system_info (
                    output,
                    &geo_system_index,
                    &cell_broadcast_support,
                    NULL)) {
                if (geo_system_index != 0xFFFF)
                    g_print ("\t\tGeo system index: '%" G_GUINT16_FORMAT "'\n", geo_system_index);
                g_print ("\t\tCell broadcast support: '%s'\n", qmi_nas_cell_broadcast_capability_get_string (cell_broadcast_support));
            }

            if (qmi_message_nas_get_system_info_output_get_gsm_call_barring_status (
                    output,
                    &call_barring_status_cs,
                    &call_barring_status_ps,
                    NULL)) {
                g_print ("\t\tCall barring status (CS): '%s'\n", qmi_nas_call_barring_status_get_string (call_barring_status_cs));
                g_print ("\t\tCall barring status (PS): '%s'\n", qmi_nas_call_barring_status_get_string (call_barring_status_ps));
            }

            if (qmi_message_nas_get_system_info_output_get_gsm_cipher_domain (
                    output,
                    &cipher_domain,
                    NULL)) {
                g_print ("\t\tCipher Domain: '%s'\n", qmi_nas_network_service_domain_get_string (cipher_domain));
            }
        }
    }

    /* WCDMA */
    {
        QmiNasServiceStatus service_status;
        QmiNasServiceStatus true_service_status;
        gboolean preferred_data_path;
        gboolean domain_valid;
        QmiNasNetworkServiceDomain domain;
        gboolean service_capability_valid;
        QmiNasNetworkServiceDomain service_capability;
        gboolean roaming_status_valid;
        QmiNasRoamingStatus roaming_status;
        gboolean forbidden_valid;
        gboolean forbidden;
        gboolean lac_valid;
        guint16 lac;
        gboolean cid_valid;
        guint32 cid;
        gboolean registration_reject_info_valid;
        QmiNasNetworkServiceDomain registration_reject_domain;
        guint8 registration_reject_cause;
        gboolean network_id_valid;
        const gchar *mcc;
        const gchar *mnc;
        gboolean hs_call_status_valid;
        QmiNasWcdmaHsService hs_call_status;
        gboolean hs_service_valid;
        QmiNasWcdmaHsService hs_service;
        gboolean primary_scrambling_code_valid;
        guint16 primary_scrambling_code;
        guint16 geo_system_index;
        QmiNasCellBroadcastCapability cell_broadcast_support;
        QmiNasCallBarringStatus call_barring_status_cs;
        QmiNasCallBarringStatus call_barring_status_ps;
        QmiNasNetworkServiceDomain cipher_domain;

        if (qmi_message_nas_get_system_info_output_get_wcdma_service_status (
                output,
                &service_status,
                &true_service_status,
                &preferred_data_path,
                NULL)) {
            g_print ("\tWCDMA service:\n"
                     "\t\tStatus: '%s'\n"
                     "\t\tTrue Status: '%s'\n"
                     "\t\tPreferred data path: '%s'\n",
                     qmi_nas_service_status_get_string (service_status),
                     qmi_nas_service_status_get_string (true_service_status),
                     preferred_data_path ? "yes" : "no");

            if (qmi_message_nas_get_system_info_output_get_wcdma_system_info (
                    output,
                    &domain_valid, &domain,
                    &service_capability_valid, &service_capability,
                    &roaming_status_valid, &roaming_status,
                    &forbidden_valid, &forbidden,
                    &lac_valid, &lac,
                    &cid_valid, &cid,
                    &registration_reject_info_valid, &registration_reject_domain, &registration_reject_cause,
                    &network_id_valid, &mcc, &mnc,
                    &hs_call_status_valid, &hs_call_status,
                    &hs_service_valid, &hs_service,
                    &primary_scrambling_code_valid, &primary_scrambling_code,
                NULL)) {
                if (domain_valid)
                    g_print ("\t\tDomain: '%s'\n", qmi_nas_network_service_domain_get_string (domain));
                if (service_capability_valid)
                    g_print ("\t\tService capability: '%s'\n", qmi_nas_network_service_domain_get_string (service_capability));
                if (roaming_status_valid)
                    g_print ("\t\tRoaming status: '%s'\n", qmi_nas_roaming_status_get_string (roaming_status));
                if (forbidden_valid)
                    g_print ("\t\tForbidden: '%s'\n", forbidden ? "yes" : "no");
                if (lac_valid)
                    g_print ("\t\tLocation Area Code: '%" G_GUINT16_FORMAT"'\n", lac);
                if (cid_valid)
                    g_print ("\t\tCell ID: '%" G_GUINT16_FORMAT"'\n", cid);
                if (registration_reject_info_valid)
                    g_print ("\t\tRegistration reject: '%s' (%u)\n",
                             qmi_nas_network_service_domain_get_string (registration_reject_domain),
                             registration_reject_cause);
                if (network_id_valid) {
                    g_print ("\t\tMCC: '%s'\n", mcc);
                    if ((guchar)mnc[2] == 0xFF)
                        g_print ("\t\tMNC: '%.2s'\n", mnc);
                    else
                        g_print ("\t\tMNC: '%.3s'\n", mnc);
                }
                if (hs_call_status_valid)
                    g_print ("\t\tHS call status: '%s'\n", qmi_nas_wcdma_hs_service_get_string (hs_call_status));
                if (hs_service_valid)
                    g_print ("\t\tHS service: '%s'\n", qmi_nas_wcdma_hs_service_get_string (hs_service));
                if (primary_scrambling_code_valid)
                    g_print ("\t\tPrimary scrambling code: '%" G_GUINT16_FORMAT"'\n", primary_scrambling_code);
            }

            if (qmi_message_nas_get_system_info_output_get_additional_wcdma_system_info (
                    output,
                    &geo_system_index,
                    &cell_broadcast_support,
                    NULL)) {
                if (geo_system_index != 0xFFFF)
                    g_print ("\t\tGeo system index: '%" G_GUINT16_FORMAT "'\n", geo_system_index);
                g_print ("\t\tCell broadcast support: '%s'\n", qmi_nas_cell_broadcast_capability_get_string (cell_broadcast_support));
            }

            if (qmi_message_nas_get_system_info_output_get_wcdma_call_barring_status (
                    output,
                    &call_barring_status_cs,
                    &call_barring_status_ps,
                    NULL)) {
                g_print ("\t\tCall barring status (CS): '%s'\n", qmi_nas_call_barring_status_get_string (call_barring_status_cs));
                g_print ("\t\tCall barring status (PS): '%s'\n", qmi_nas_call_barring_status_get_string (call_barring_status_ps));
            }

            if (qmi_message_nas_get_system_info_output_get_wcdma_cipher_domain (
                    output,
                    &cipher_domain,
                    NULL)) {
                g_print ("\t\tCipher Domain: '%s'\n", qmi_nas_network_service_domain_get_string (cipher_domain));
            }
        }
    }

    /* LTE */
    {
        QmiNasServiceStatus service_status;
        QmiNasServiceStatus true_service_status;
        gboolean preferred_data_path;
        gboolean domain_valid;
        QmiNasNetworkServiceDomain domain;
        gboolean service_capability_valid;
        QmiNasNetworkServiceDomain service_capability;
        gboolean roaming_status_valid;
        QmiNasRoamingStatus roaming_status;
        gboolean forbidden_valid;
        gboolean forbidden;
        gboolean lac_valid;
        guint16 lac;
        gboolean cid_valid;
        guint32 cid;
        gboolean registration_reject_info_valid;
        QmiNasNetworkServiceDomain registration_reject_domain;
        guint8 registration_reject_cause;
        gboolean network_id_valid;
        const gchar *mcc;
        const gchar *mnc;
        gboolean tac_valid;
        guint16 tac;
        guint16 geo_system_index;
        gboolean voice_support;
        gboolean embms_coverage_info_support;

        if (qmi_message_nas_get_system_info_output_get_lte_service_status (
                output,
                &service_status,
                &true_service_status,
                &preferred_data_path,
                NULL)) {
            g_print ("\tLTE service:\n"
                     "\t\tStatus: '%s'\n"
                     "\t\tTrue Status: '%s'\n"
                     "\t\tPreferred data path: '%s'\n",
                     qmi_nas_service_status_get_string (service_status),
                     qmi_nas_service_status_get_string (true_service_status),
                     preferred_data_path ? "yes" : "no");

            if (qmi_message_nas_get_system_info_output_get_lte_system_info (
                    output,
                    &domain_valid, &domain,
                    &service_capability_valid, &service_capability,
                    &roaming_status_valid, &roaming_status,
                    &forbidden_valid, &forbidden,
                    &lac_valid, &lac,
                    &cid_valid, &cid,
                    &registration_reject_info_valid,&registration_reject_domain,&registration_reject_cause,
                    &network_id_valid, &mcc, &mnc,
                    &tac_valid, &tac,
                    NULL)) {
                if (domain_valid)
                    g_print ("\t\tDomain: '%s'\n", qmi_nas_network_service_domain_get_string (domain));
                if (service_capability_valid)
                    g_print ("\t\tService capability: '%s'\n", qmi_nas_network_service_domain_get_string (service_capability));
                if (roaming_status_valid)
                    g_print ("\t\tRoaming status: '%s'\n", qmi_nas_roaming_status_get_string (roaming_status));
                if (forbidden_valid)
                    g_print ("\t\tForbidden: '%s'\n", forbidden ? "yes" : "no");
                if (lac_valid)
                    g_print ("\t\tLocation Area Code: '%" G_GUINT16_FORMAT"'\n", lac);
                if (cid_valid)
                    g_print ("\t\tCell ID: '%" G_GUINT16_FORMAT"'\n", cid);
                if (registration_reject_info_valid)
                    g_print ("\t\tRegistration reject: '%s' (%u)\n",
                             qmi_nas_network_service_domain_get_string (registration_reject_domain),
                             registration_reject_cause);
                if (network_id_valid) {
                    g_print ("\t\tMCC: '%s'\n", mcc);
                    if ((guchar)mnc[2] == 0xFF)
                        g_print ("\t\tMNC: '%.2s'\n", mnc);
                    else
                        g_print ("\t\tMNC: '%.3s'\n", mnc);
                }
                if (tac_valid)
                    g_print ("\t\tTracking Area Code: '%" G_GUINT16_FORMAT"'\n", tac);
            }

            if (qmi_message_nas_get_system_info_output_get_additional_lte_system_info (
                    output,
                    &geo_system_index,
                    NULL)) {
                if (geo_system_index != 0xFFFF)
                    g_print ("\t\tGeo system index: '%" G_GUINT16_FORMAT "'\n", geo_system_index);
            }

            if (qmi_message_nas_get_system_info_output_get_lte_voice_support (
                    output,
                    &voice_support,
                    NULL)) {
                g_print ("\t\tVoice support: '%s'\n", voice_support ? "yes" : "no");
            }

            if (qmi_message_nas_get_system_info_output_get_lte_embms_coverage_info_support (
                    output,
                    &embms_coverage_info_support,
                    NULL)) {
                g_print ("\t\teMBMS coverage info support: '%s'\n", embms_coverage_info_support ? "yes" : "no");
            }
        }
    }

    /* TD-SCDMA */
    {
        QmiNasServiceStatus service_status;
        QmiNasServiceStatus true_service_status;
        gboolean preferred_data_path;
        gboolean domain_valid;
        QmiNasNetworkServiceDomain domain;
        gboolean service_capability_valid;
        QmiNasNetworkServiceDomain service_capability;
        gboolean roaming_status_valid;
        QmiNasRoamingStatus roaming_status;
        gboolean forbidden_valid;
        gboolean forbidden;
        gboolean lac_valid;
        guint16 lac;
        gboolean cid_valid;
        guint32 cid;
        gboolean registration_reject_info_valid;
        QmiNasNetworkServiceDomain registration_reject_domain;
        guint8 registration_reject_cause;
        gboolean network_id_valid;
        const gchar *mcc;
        const gchar *mnc;
        gboolean hs_call_status_valid;
        QmiNasWcdmaHsService hs_call_status;
        gboolean hs_service_valid;
        QmiNasWcdmaHsService hs_service;
        gboolean cell_parameter_id_valid;
        guint16 cell_parameter_id;
        gboolean cell_broadcast_support_valid;
        QmiNasCellBroadcastCapability cell_broadcast_support;
        gboolean call_barring_status_cs_valid;
        QmiNasCallBarringStatus call_barring_status_cs;
        gboolean call_barring_status_ps_valid;
        QmiNasCallBarringStatus call_barring_status_ps;
        gboolean cipher_domain_valid;
        QmiNasNetworkServiceDomain cipher_domain;

        if (qmi_message_nas_get_system_info_output_get_td_scdma_service_status (
                output,
                &service_status,
                &true_service_status,
                &preferred_data_path,
                NULL)) {
            g_print ("\tTD-SCDMA service:\n"
                     "\t\tStatus: '%s'\n"
                     "\t\tTrue Status: '%s'\n"
                     "\t\tPreferred data path: '%s'\n",
                     qmi_nas_service_status_get_string (service_status),
                     qmi_nas_service_status_get_string (true_service_status),
                     preferred_data_path ? "yes" : "no");

            if (qmi_message_nas_get_system_info_output_get_td_scdma_system_info (
                    output,
                    &domain_valid, &domain,
                    &service_capability_valid, &service_capability,
                    &roaming_status_valid, &roaming_status,
                    &forbidden_valid, &forbidden,
                    &lac_valid, &lac,
                    &cid_valid, &cid,
                    &registration_reject_info_valid, &registration_reject_domain, &registration_reject_cause,
                    &network_id_valid, &mcc, &mnc,
                    &hs_call_status_valid, &hs_call_status,
                    &hs_service_valid, &hs_service,
                    &cell_parameter_id_valid, &cell_parameter_id,
                    &cell_broadcast_support_valid, &cell_broadcast_support,
                    &call_barring_status_cs_valid, &call_barring_status_cs,
                    &call_barring_status_ps_valid, &call_barring_status_ps,
                    &cipher_domain_valid, &cipher_domain,
                    NULL)) {
                if (domain_valid)
                    g_print ("\t\tDomain: '%s'\n", qmi_nas_network_service_domain_get_string (domain));
                if (service_capability_valid)
                    g_print ("\t\tService capability: '%s'\n", qmi_nas_network_service_domain_get_string (service_capability));
                if (roaming_status_valid)
                    g_print ("\t\tRoaming status: '%s'\n", qmi_nas_roaming_status_get_string (roaming_status));
                if (forbidden_valid)
                    g_print ("\t\tForbidden: '%s'\n", forbidden ? "yes" : "no");
                if (lac_valid)
                    g_print ("\t\tLocation Area Code: '%" G_GUINT16_FORMAT"'\n", lac);
                if (cid_valid)
                    g_print ("\t\tCell ID: '%" G_GUINT16_FORMAT"'\n", cid);
                if (registration_reject_info_valid)
                    g_print ("\t\tRegistration reject: '%s' (%u)\n",
                             qmi_nas_network_service_domain_get_string (registration_reject_domain),
                             registration_reject_cause);
                if (network_id_valid) {
                    g_print ("\t\tMCC: '%s'\n", mcc);
                    if ((guchar)mnc[2] == 0xFF)
                        g_print ("\t\tMNC: '%.2s'\n", mnc);
                    else
                        g_print ("\t\tMNC: '%.3s'\n", mnc);
                }
                if (hs_call_status_valid)
                    g_print ("\t\tHS call status: '%s'\n", qmi_nas_wcdma_hs_service_get_string (hs_call_status));
                if (hs_service_valid)
                    g_print ("\t\tHS service: '%s'\n", qmi_nas_wcdma_hs_service_get_string (hs_service));
                if (cell_parameter_id_valid)
                    g_print ("\t\tCell parameter ID: '%" G_GUINT16_FORMAT"'\n", cell_parameter_id);
                if (cell_broadcast_support_valid)
                    g_print ("\t\tCell broadcast support: '%s'\n", qmi_nas_cell_broadcast_capability_get_string (cell_broadcast_support));
                if (call_barring_status_cs_valid)
                    g_print ("\t\tCall barring status (CS): '%s'\n", qmi_nas_call_barring_status_get_string (call_barring_status_cs));
                if (call_barring_status_ps_valid)
                    g_print ("\t\tCall barring status (PS): '%s'\n", qmi_nas_call_barring_status_get_string (call_barring_status_ps));
                if (cipher_domain_valid)
                    g_print ("\t\tCipher Domain: '%s'\n", qmi_nas_network_service_domain_get_string (cipher_domain));
            }
        }

        /* Common */
        {
            QmiNasSimRejectState sim_reject_info;

            if (qmi_message_nas_get_system_info_output_get_sim_reject_info (
                    output,
                    &sim_reject_info,
                    NULL)) {
                g_print ("\tSIM reject info: '%s'\n", qmi_nas_sim_reject_state_get_string (sim_reject_info));
            }
        }
    }

    qmi_message_nas_get_system_info_output_unref (output);
    shutdown (TRUE);
}

static void
get_technology_preference_ready (QmiClientNas *client,
                                 GAsyncResult *res)
{
    QmiMessageNasGetTechnologyPreferenceOutput *output;
    GError *error = NULL;
    QmiNasRadioTechnologyPreference preference;
    QmiNasPreferenceDuration duration;
    gchar *preference_string;

    output = qmi_client_nas_get_technology_preference_finish (client, res, &error);
    if (!output) {
        g_printerr ("error: operation failed: %s\n", error->message);
        g_error_free (error);
        shutdown (FALSE);
        return;
    }

    if (!qmi_message_nas_get_technology_preference_output_get_result (output, &error)) {
        g_printerr ("error: couldn't get technology preference: %s\n", error->message);
        g_error_free (error);
        qmi_message_nas_get_technology_preference_output_unref (output);
        shutdown (FALSE);
        return;
    }

    qmi_message_nas_get_technology_preference_output_get_active (
        output,
        &preference,
        &duration,
        NULL);

    preference_string = qmi_nas_radio_technology_preference_build_string_from_mask (preference);
    g_print ("[%s] Successfully got technology preference\n"
             "\tActive: '%s', duration: '%s'\n",
             qmi_device_get_path_display (ctx->device),
             preference_string,
             qmi_nas_preference_duration_get_string (duration));
    g_free (preference_string);

    if (qmi_message_nas_get_technology_preference_output_get_persistent (
            output,
            &preference,
            NULL)) {
        preference_string = qmi_nas_radio_technology_preference_build_string_from_mask (preference);
        g_print ("\tPersistent: '%s', duration: '%s'\n",
                 qmi_device_get_path_display (ctx->device),
                 preference_string);
        g_free (preference_string);
    }

    qmi_message_nas_get_technology_preference_output_unref (output);
    shutdown (TRUE);
}

static void
get_system_selection_preference_ready (QmiClientNas *client,
                                 GAsyncResult *res)
{
    QmiMessageNasGetSystemSelectionPreferenceOutput *output;
    GError *error = NULL;
    gboolean emergency_mode;
    QmiNasRatModePreference mode_preference;
    QmiNasBandPreference band_preference;
    QmiNasLteBandPreference lte_band_preference;
    QmiNasTdScdmaBandPreference td_scdma_band_preference;
    QmiNasCdmaPrlPreference cdma_prl_preference;
    QmiNasRoamingPreference roaming_preference;
    QmiNasNetworkSelectionPreference network_selection_preference;
    QmiNasServiceDomainPreference service_domain_preference;
    QmiNasGsmWcdmaAcquisitionOrderPreference gsm_wcdma_acquisition_order_preference;
    guint16 mcc;
    guint16 mnc;
    gboolean has_pcs_digit;

    output = qmi_client_nas_get_system_selection_preference_finish (client, res, &error);
    if (!output) {
        g_printerr ("error: operation failed: %s\n", error->message);
        g_error_free (error);
        shutdown (FALSE);
        return;
    }

    if (!qmi_message_nas_get_system_selection_preference_output_get_result (output, &error)) {
        g_printerr ("error: couldn't get system_selection preference: %s\n", error->message);
        g_error_free (error);
        qmi_message_nas_get_system_selection_preference_output_unref (output);
        shutdown (FALSE);
        return;
    }

    g_print ("[%s] Successfully got system selection preference\n",
             qmi_device_get_path_display (ctx->device));

    if (qmi_message_nas_get_system_selection_preference_output_get_emergency_mode (
            output,
            &emergency_mode,
            NULL)) {
        g_print ("\tEmergency mode: '%s'\n",
                 emergency_mode ? "yes" : "no");
    }

    if (qmi_message_nas_get_system_selection_preference_output_get_mode_preference (
            output,
            &mode_preference,
            NULL)) {
        gchar *str;

        str = qmi_nas_rat_mode_preference_build_string_from_mask (mode_preference);
        g_print ("\tMode preference: '%s'\n", str);
        g_free (str);
    }

    if (qmi_message_nas_get_system_selection_preference_output_get_band_preference (
            output,
            &band_preference,
            NULL)) {
        gchar *str;

        str = qmi_nas_band_preference_build_string_from_mask (band_preference);
        g_print ("\tBand preference: '%s'\n", str);
        g_free (str);
    }

    if (qmi_message_nas_get_system_selection_preference_output_get_lte_band_preference (
            output,
            &lte_band_preference,
            NULL)) {
        gchar *str;

        str = qmi_nas_lte_band_preference_build_string_from_mask (lte_band_preference);
        g_print ("\tLTE band preference: '%s'\n", str);
        g_free (str);
    }

    if (qmi_message_nas_get_system_selection_preference_output_get_td_scdma_band_preference (
            output,
            &td_scdma_band_preference,
            NULL)) {
        gchar *str;

        str = qmi_nas_td_scdma_band_preference_build_string_from_mask (td_scdma_band_preference);
        g_print ("\tTD-SCDMA band preference: '%s'\n", str);
        g_free (str);
    }

    if (qmi_message_nas_get_system_selection_preference_output_get_cdma_prl_preference (
            output,
            &cdma_prl_preference,
            NULL)) {
        g_print ("\tCDMA PRL preference: '%s'\n",
                 qmi_nas_cdma_prl_preference_get_string (cdma_prl_preference));
    }

    if (qmi_message_nas_get_system_selection_preference_output_get_roaming_preference (
            output,
            &roaming_preference,
            NULL)) {
        g_print ("\tRoaming preference: '%s'\n",
                 qmi_nas_roaming_preference_get_string (roaming_preference));
    }

    if (qmi_message_nas_get_system_selection_preference_output_get_network_selection_preference (
            output,
            &network_selection_preference,
            NULL)) {
        g_print ("\tNetwork selection preference: '%s'\n",
                 qmi_nas_network_selection_preference_get_string (network_selection_preference));
    }


    if (qmi_message_nas_get_system_selection_preference_output_get_service_domain_preference (
            output,
            &service_domain_preference,
            NULL)) {
        g_print ("\tService domain preference: '%s'\n",
                 qmi_nas_service_domain_preference_get_string (service_domain_preference));
    }

    if (qmi_message_nas_get_system_selection_preference_output_get_gsm_wcdma_acquisition_order_preference (
            output,
            &gsm_wcdma_acquisition_order_preference,
            NULL)) {
        g_print ("\tService selection preference: '%s'\n",
                 qmi_nas_gsm_wcdma_acquisition_order_preference_get_string (gsm_wcdma_acquisition_order_preference));
    }

    if (qmi_message_nas_get_system_selection_preference_output_get_manual_network_selection (
            output,
            &mcc,
            &mnc,
            &has_pcs_digit,
            NULL)) {
        g_print ("\tManual network selection:\n"
                 "\t\tMCC: '%" G_GUINT16_FORMAT"'\n"
                 "\t\tMNC: '%" G_GUINT16_FORMAT"'\n"
                 "\t\tMCC with PCS digit: '%s'\n",
                 mcc,
                 mnc,
                 has_pcs_digit ? "yes" : "no");
    }

    qmi_message_nas_get_system_selection_preference_output_unref (output);
    shutdown (TRUE);
}

static void
network_scan_ready (QmiClientNas *client,
                    GAsyncResult *res)
{
    QmiMessageNasNetworkScanOutput *output;
    GError *error = NULL;
    GArray *array;

    output = qmi_client_nas_network_scan_finish (client, res, &error);
    if (!output) {
        g_printerr ("error: operation failed: %s\n", error->message);
        g_error_free (error);
        shutdown (FALSE);
        return;
    }

    if (!qmi_message_nas_network_scan_output_get_result (output, &error)) {
        g_printerr ("error: couldn't scan networks: %s\n", error->message);
        g_error_free (error);
        qmi_message_nas_network_scan_output_unref (output);
        shutdown (FALSE);
        return;
    }

    g_print ("[%s] Successfully scanned networks\n",
             qmi_device_get_path_display (ctx->device));

    array = NULL;
    if (qmi_message_nas_network_scan_output_get_network_information (output, &array, NULL)) {
        guint i;

        for (i = 0; i < array->len; i++) {
            QmiMessageNasNetworkScanOutputNetworkInformationElement *element;
            gchar *status_str;

            element = &g_array_index (array, QmiMessageNasNetworkScanOutputNetworkInformationElement, i);
            status_str = qmi_nas_network_status_build_string_from_mask (element->network_status);
            g_print ("Network [%u]:\n"
                     "\tMCC: '%" G_GUINT16_FORMAT"'\n"
                     "\tMNC: '%" G_GUINT16_FORMAT"'\n"
                     "\tStatus: '%s'\n"
                     "\tDescription: '%s'\n",
                     i,
                     element->mcc,
                     element->mnc,
                     status_str,
                     element->description);
            g_free (status_str);
        }
    }

    array = NULL;
    if (qmi_message_nas_network_scan_output_get_radio_access_technology (output, &array, NULL)) {
        guint i;

        for (i = 0; i < array->len; i++) {
            QmiMessageNasNetworkScanOutputRadioAccessTechnologyElement *element;

            element = &g_array_index (array, QmiMessageNasNetworkScanOutputRadioAccessTechnologyElement, i);
            g_print ("Network [%u]:\n"
                     "\tMCC: '%" G_GUINT16_FORMAT"'\n"
                     "\tMNC: '%" G_GUINT16_FORMAT"'\n"
                     "\tRAT: '%s'\n",
                     i,
                     element->mcc,
                     element->mnc,
                     qmi_nas_radio_interface_get_string (element->radio_interface));
        }
    }

    array = NULL;
    if (qmi_message_nas_network_scan_output_get_mnc_pcs_digit_include_status (output, &array, NULL)) {
        guint i;

        for (i = 0; i < array->len; i++) {
            QmiMessageNasNetworkScanOutputMncPcsDigitIncludeStatusElement *element;

            element = &g_array_index (array, QmiMessageNasNetworkScanOutputMncPcsDigitIncludeStatusElement, i);
            g_print ("Network [%u]:\n"
                     "\tMCC: '%" G_GUINT16_FORMAT"'\n"
                     "\tMNC: '%" G_GUINT16_FORMAT"'\n"
                     "\tMCC with PCS digit: '%s'\n",
                     i,
                     element->mcc,
                     element->mnc,
                     element->includes_pcs_digit ? "yes" : "no");
        }
    }

    qmi_message_nas_network_scan_output_unref (output);
    shutdown (TRUE);
}

static void
reset_ready (QmiClientNas *client,
             GAsyncResult *res)
{
    QmiMessageNasResetOutput *output;
    GError *error = NULL;

    output = qmi_client_nas_reset_finish (client, res, &error);
    if (!output) {
        g_printerr ("error: operation failed: %s\n", error->message);
        g_error_free (error);
        shutdown (FALSE);
        return;
    }

    if (!qmi_message_nas_reset_output_get_result (output, &error)) {
        g_printerr ("error: couldn't reset the NAS service: %s\n", error->message);
        g_error_free (error);
        qmi_message_nas_reset_output_unref (output);
        shutdown (FALSE);
        return;
    }

    g_print ("[%s] Successfully performed NAS service reset\n",
             qmi_device_get_path_display (ctx->device));

    qmi_message_nas_reset_output_unref (output);
    shutdown (TRUE);
}

static gboolean
noop_cb (gpointer unused)
{
    shutdown (TRUE);
    return FALSE;
}

void
qmicli_nas_run (QmiDevice *device,
                QmiClientNas *client,
                GCancellable *cancellable)
{
    /* Initialize context */
    ctx = g_slice_new (Context);
    ctx->device = g_object_ref (device);
    ctx->client = g_object_ref (client);
    if (cancellable)
        ctx->cancellable = g_object_ref (cancellable);

    /* Request to get signal strength? */
    if (get_signal_strength_flag) {
        QmiMessageNasGetSignalStrengthInput *input;

        input = get_signal_strength_input_create ();

        g_debug ("Asynchronously getting signal strength...");
        qmi_client_nas_get_signal_strength (ctx->client,
                                            input,
                                            10,
                                            ctx->cancellable,
                                            (GAsyncReadyCallback)get_signal_strength_ready,
                                            NULL);
        qmi_message_nas_get_signal_strength_input_unref (input);
        return;
    }

    /* Request to get signal info? */
    if (get_signal_info_flag) {
        g_debug ("Asynchronously getting signal info...");
        qmi_client_nas_get_signal_info (ctx->client,
                                        NULL,
                                        10,
                                        ctx->cancellable,
                                        (GAsyncReadyCallback)get_signal_info_ready,
                                        NULL);
        return;
    }

    /* Request to get home network? */
    if (get_home_network_flag) {
        g_debug ("Asynchronously getting home network...");
        qmi_client_nas_get_home_network (ctx->client,
                                         NULL,
                                         10,
                                         ctx->cancellable,
                                         (GAsyncReadyCallback)get_home_network_ready,
                                         NULL);
        return;
    }

    /* Set roaming mode */
    if (set_roaming_str) {
	QmiMessageNasSetSystemSelectionPreferenceInput *input = NULL;
	QmiNasRoamingPreference roamingpref = QMI_NAS_ROAMING_PREFERENCE_OFF;
        g_debug ("Asynchronously setting roaming mode...");
	if (g_ascii_strcasecmp(set_roaming_str, "OFF") == 0) {
		roamingpref = QMI_NAS_ROAMING_PREFERENCE_OFF;
	} else {
		roamingpref = QMI_NAS_ROAMING_PREFERENCE_ANY;
	}
	input = qmi_message_nas_set_system_selection_preference_input_new();
	qmi_message_nas_set_system_selection_preference_input_set_roaming_preference(input, roamingpref, NULL);
        qmi_client_nas_set_system_selection_preference(ctx->client,
                                         input,
                                         10,
                                         ctx->cancellable,
                                         (GAsyncReadyCallback)set_roaming_ready,
                                         NULL);
	qmi_message_nas_set_system_selection_preference_input_unref(input);
        return;
    }

    /* Set lte mode */
    if (set_lte_str) {
	QmiMessageNasSetSystemSelectionPreferenceInput *input = NULL;	
	QmiNasRatModePreference ltepref = 0;
	QmiNasGsmWcdmaAcquisitionOrderPreference order = QMI_NAS_GSM_WCDMA_ACQUISITION_ORDER_PREFERENCE_AUTOMATIC;


        g_debug ("Asynchronously setting lte mode...");
	input = qmi_message_nas_set_system_selection_preference_input_new();
	if (g_ascii_strcasecmp(set_lte_str, "LTE") == 0) {
		ltepref = QMI_NAS_RAT_MODE_PREFERENCE_LTE;
		qmi_message_nas_set_system_selection_preference_input_set_mode_preference(input, ltepref, NULL);
	} else if (g_ascii_strcasecmp(set_lte_str, "LTEUMTS") == 0) {
		ltepref = QMI_NAS_RAT_MODE_PREFERENCE_LTE|QMI_NAS_RAT_MODE_PREFERENCE_UMTS;	
		qmi_message_nas_set_system_selection_preference_input_set_mode_preference(input, ltepref, NULL);
	} else if (g_ascii_strcasecmp(set_lte_str, "GSMUMTS") == 0) {
		ltepref = QMI_NAS_RAT_MODE_PREFERENCE_GSM|QMI_NAS_RAT_MODE_PREFERENCE_UMTS;		
		order = QMI_NAS_GSM_WCDMA_ACQUISITION_ORDER_PREFERENCE_GSM;
		qmi_message_nas_set_system_selection_preference_input_set_mode_preference(input, ltepref, NULL);
    		qmi_message_nas_set_system_selection_preference_input_set_gsm_wcdma_acquisition_order_preference (input, order, NULL);
	} else if (g_ascii_strcasecmp(set_lte_str, "UMTSGSM") == 0) {
		ltepref = QMI_NAS_RAT_MODE_PREFERENCE_GSM|QMI_NAS_RAT_MODE_PREFERENCE_UMTS;		
		order = QMI_NAS_GSM_WCDMA_ACQUISITION_ORDER_PREFERENCE_WCDMA;
		qmi_message_nas_set_system_selection_preference_input_set_mode_preference(input, ltepref, NULL);
    		qmi_message_nas_set_system_selection_preference_input_set_gsm_wcdma_acquisition_order_preference (input, order, NULL);
	} else if (g_ascii_strcasecmp(set_lte_str, "GSM") == 0) {
		ltepref = QMI_NAS_RAT_MODE_PREFERENCE_GSM;	
		qmi_message_nas_set_system_selection_preference_input_set_mode_preference(input, ltepref, NULL);
	} else {
		ltepref = QMI_NAS_RAT_MODE_PREFERENCE_CDMA_1X|QMI_NAS_RAT_MODE_PREFERENCE_CDMA_1XEVDO|QMI_NAS_RAT_MODE_PREFERENCE_GSM|QMI_NAS_RAT_MODE_PREFERENCE_UMTS|QMI_NAS_RAT_MODE_PREFERENCE_LTE|QMI_NAS_RAT_MODE_PREFERENCE_TD_SCDMA;
		qmi_message_nas_set_system_selection_preference_input_set_mode_preference(input, ltepref, NULL);
	}
        qmi_message_nas_set_system_selection_preference_input_set_change_duration (input, QMI_NAS_CHANGE_DURATION_PERMANENT, NULL);
        qmi_client_nas_set_system_selection_preference(ctx->client,
                                         input,
                                         10,
                                         ctx->cancellable,
                                         (GAsyncReadyCallback)set_roaming_ready,
                                         NULL);
	qmi_message_nas_set_system_selection_preference_input_unref(input);
        return;
    }

    /* Request to get serving system? */
    if (get_serving_system_flag) {
        g_debug ("Asynchronously getting serving system...");
        qmi_client_nas_get_serving_system (ctx->client,
                                           NULL,
                                           10,
                                           ctx->cancellable,
                                           (GAsyncReadyCallback)get_serving_system_ready,
                                           NULL);
        return;
    }

    /* Request to get system info? */
    if (get_system_info_flag) {
        g_debug ("Asynchronously getting system info...");
        qmi_client_nas_get_system_info (ctx->client,
                                        NULL,
                                        10,
                                        ctx->cancellable,
                                        (GAsyncReadyCallback)get_system_info_ready,
                                        NULL);
        return;
    }

    /* Request to get technology preference? */
    if (get_technology_preference_flag) {
        g_debug ("Asynchronously getting technology preference...");
        qmi_client_nas_get_technology_preference (ctx->client,
                                                  NULL,
                                                  10,
                                                  ctx->cancellable,
                                                  (GAsyncReadyCallback)get_technology_preference_ready,
                                                  NULL);
        return;
    }

    /* Request to get system_selection preference? */
    if (get_system_selection_preference_flag) {
        g_debug ("Asynchronously getting system selection preference...");
        qmi_client_nas_get_system_selection_preference (ctx->client,
                                                        NULL,
                                                        10,
                                                        ctx->cancellable,
                                                        (GAsyncReadyCallback)get_system_selection_preference_ready,
                                                        NULL);
        return;
    }

    /* Request to scan networks? */
    if (network_scan_flag) {
        g_debug ("Asynchronously scanning networks...");
        qmi_client_nas_network_scan (ctx->client,
                                     NULL,
                                     300, /* this operation takes a lot of time! */
                                     ctx->cancellable,
                                     (GAsyncReadyCallback)network_scan_ready,
                                     NULL);
        return;
    }

    /* Request to reset NAS service? */
    if (reset_flag) {
        g_debug ("Asynchronously resetting NAS service...");
        qmi_client_nas_reset (ctx->client,
                              NULL,
                              10,
                              ctx->cancellable,
                              (GAsyncReadyCallback)reset_ready,
                              NULL);
        return;
    }

    /* Just client allocate/release? */
    if (noop_flag) {
        g_idle_add (noop_cb, NULL);
        return;
    }

    g_warn_if_reached ();
}
