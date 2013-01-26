/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * qmicli -- Command line interface to control QMI devices
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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
    QmiClientWds *client;
    GCancellable *cancellable;

    /* Helpers for the wds-start-network command */
    gulong network_started_id;
    guint packet_status_timeout_id;
    guint32 packet_data_handle;
} Context;
static Context *ctx;

/* Options */
static gchar *start_network_str;
static gboolean follow_network_flag;
static gchar *stop_network_str;
static gboolean get_packet_service_status_flag;
static gboolean get_data_bearer_technology_flag;
static gboolean get_current_data_bearer_technology_flag;
static gboolean reset_flag;
static gboolean noop_flag;

static GOptionEntry entries[] = {
    { "wds-start-network", 0, 0, G_OPTION_ARG_STRING, &start_network_str,
      "Start network",
      "[APN]"
    },
    { "wds-follow-network", 0, 0, G_OPTION_ARG_NONE, &follow_network_flag,
      "Follow the network status until disconnected. Use with `--wds-start-network'",
      NULL
    },
    { "wds-stop-network", 0, 0, G_OPTION_ARG_STRING, &stop_network_str,
      "Stop network",
      "[Packet data handle]"
    },
    { "wds-get-packet-service-status", 0, 0, G_OPTION_ARG_NONE, &get_packet_service_status_flag,
      "Get packet service status",
      NULL
    },
    { "wds-get-data-bearer-technology", 0, 0, G_OPTION_ARG_NONE, &get_data_bearer_technology_flag,
      "Get data bearer technology",
      NULL
    },
    { "wds-get-current-data-bearer-technology", 0, 0, G_OPTION_ARG_NONE, &get_current_data_bearer_technology_flag,
      "Get current data bearer technology",
      NULL
    },
    { "wds-reset", 0, 0, G_OPTION_ARG_NONE, &reset_flag,
      "Reset the service state",
      NULL
    },
    { "wds-noop", 0, 0, G_OPTION_ARG_NONE, &noop_flag,
      "Just allocate or release a WDS client. Use with `--client-no-release-cid' and/or `--client-cid'",
      NULL
    },
    { NULL }
};

GOptionGroup *
qmicli_wds_get_option_group (void)
{
	GOptionGroup *group;

	group = g_option_group_new ("wds",
	                            "WDS options",
	                            "Show Wireless Data Service options",
	                            NULL,
	                            NULL);
	g_option_group_add_entries (group, entries);

	return group;
}

gboolean
qmicli_wds_options_enabled (void)
{
    static guint n_actions = 0;
    static gboolean checked = FALSE;

    if (checked)
        return !!n_actions;

    n_actions = (!!start_network_str +
                 !!stop_network_str +
                 get_packet_service_status_flag +
                 get_data_bearer_technology_flag +
                 get_current_data_bearer_technology_flag +
                 reset_flag +
                 noop_flag);

    if (n_actions > 1) {
        g_printerr ("error: too many WDS actions requested\n");
        exit (EXIT_FAILURE);
    } else if (n_actions == 0 &&
               follow_network_flag) {
        g_printerr ("error: `--wds-follow-network' must be used with `--wds-start-network'\n");
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

    if (context->client)
        g_object_unref (context->client);
    if (context->network_started_id)
        g_cancellable_disconnect (context->cancellable, context->network_started_id);
    if (context->packet_status_timeout_id)
        g_source_remove (context->packet_status_timeout_id);
    g_object_unref (context->cancellable);
    g_object_unref (context->device);
    g_slice_free (Context, context);
}

static void
shutdown (gboolean operation_status)
{
    /* Cleanup context and finish async operation */
    context_free (ctx);
    qmicli_async_operation_done (operation_status);
}

static void
stop_network_ready (QmiClientWds *client,
                    GAsyncResult *res)
{
    GError *error = NULL;
    QmiMessageWdsStopNetworkOutput *output;

    output = qmi_client_wds_stop_network_finish (client, res, &error);
    if (!output) {
        g_printerr ("error: operation failed: %s\n",
                    error->message);
        g_error_free (error);
        shutdown (FALSE);
        return;
    }

    if (!qmi_message_wds_stop_network_output_get_result (output, &error)) {
        g_printerr ("error: couldn't stop network: %s\n", error->message);
        g_error_free (error);
        qmi_message_wds_stop_network_output_unref (output);
        shutdown (FALSE);
        return;
    }

#undef VALIDATE_UNKNOWN
#define VALIDATE_UNKNOWN(str) (str ? str : "unknown")

    g_print ("[%s] Network stopped\n",
             qmi_device_get_path_display (ctx->device));
    qmi_message_wds_stop_network_output_unref (output);
    shutdown (TRUE);
}

static void
internal_stop_network (GCancellable *cancellable,
                       guint32 packet_data_handle)
{
    QmiMessageWdsStopNetworkInput *input;

    input = qmi_message_wds_stop_network_input_new ();
    qmi_message_wds_stop_network_input_set_packet_data_handle (input, packet_data_handle, NULL);

    g_print ("Network cancelled... releasing resources\n");
    qmi_client_wds_stop_network (ctx->client,
                                 input,
                                 10,
                                 ctx->cancellable,
                                 (GAsyncReadyCallback)stop_network_ready,
                                 NULL);
    qmi_message_wds_stop_network_input_unref (input);
}

static void
network_cancelled (GCancellable *cancellable)
{
    ctx->network_started_id = 0;

    /* Remove the timeout right away */
    if (ctx->packet_status_timeout_id) {
        g_source_remove (ctx->packet_status_timeout_id);
        ctx->packet_status_timeout_id = 0;
    }

    g_print ("Network cancelled... releasing resources\n");
    internal_stop_network (cancellable, ctx->packet_data_handle);
}

static void
timeout_get_packet_service_status_ready (QmiClientWds *client,
                                         GAsyncResult *res)
{
    GError *error = NULL;
    QmiMessageWdsGetPacketServiceStatusOutput *output;
    QmiWdsConnectionStatus status;

    output = qmi_client_wds_get_packet_service_status_finish (client, res, &error);
    if (!output) {
        g_printerr ("error: operation failed: %s\n",
                    error->message);
        g_error_free (error);
        shutdown (FALSE);
        return;
    }

    if (!qmi_message_wds_get_packet_service_status_output_get_result (output, &error)) {
        g_printerr ("error: couldn't get packet service status: %s\n", error->message);
        g_error_free (error);
        qmi_message_wds_get_packet_service_status_output_unref (output);
        shutdown (FALSE);
        return;
    }

    qmi_message_wds_get_packet_service_status_output_get_connection_status (
        output,
        &status,
        NULL);

    g_print ("[%s] Connection status: '%s'\n",
             qmi_device_get_path_display (ctx->device),
             qmi_wds_connection_status_get_string (status));
    qmi_message_wds_get_packet_service_status_output_unref (output);
    shutdown (TRUE);
}

static gboolean
packet_status_timeout (void)
{
    qmi_client_wds_get_packet_service_status (ctx->client,
                                              NULL,
                                              10,
                                              ctx->cancellable,
                                              (GAsyncReadyCallback)timeout_get_packet_service_status_ready,
                                              NULL);

    return TRUE;
}

static void
start_network_ready (QmiClientWds *client,
                     GAsyncResult *res)
{
    GError *error = NULL;
    QmiMessageWdsStartNetworkOutput *output;

    output = qmi_client_wds_start_network_finish (client, res, &error);
    if (!output) {
        g_printerr ("error: operation failed: %s\n",
                    error->message);
        g_error_free (error);
        shutdown (FALSE);
        return;
    }

    if (!qmi_message_wds_start_network_output_get_result (output, &error)) {
        g_printerr ("error: couldn't start network: %s\n", error->message);
        if (g_error_matches (error,
                             QMI_PROTOCOL_ERROR,
                             QMI_PROTOCOL_ERROR_CALL_FAILED)) {
            QmiWdsCallEndReason cer;
            QmiWdsVerboseCallEndReasonType verbose_cer_type;
            gint16 verbose_cer_reason;

            if (qmi_message_wds_start_network_output_get_call_end_reason (
                    output,
                    &cer,
                    NULL))
                g_printerr ("call end reason (%u): %s\n",
                            cer,
                            qmi_wds_call_end_reason_get_string (cer));

            if (qmi_message_wds_start_network_output_get_verbose_call_end_reason (
                    output,
                    &verbose_cer_type,
                    &verbose_cer_reason,
                    NULL))
                g_printerr ("verbose call end reason (%u,%d): [%s] %s\n",
                            verbose_cer_type,
                            verbose_cer_reason,
                            qmi_wds_verbose_call_end_reason_type_get_string (verbose_cer_type),
                            qmi_wds_verbose_call_end_reason_get_string (verbose_cer_type, verbose_cer_reason));
        }

        g_error_free (error);
        qmi_message_wds_start_network_output_unref (output);
        shutdown (FALSE);
        return;
    }

    qmi_message_wds_start_network_output_get_packet_data_handle (output, &ctx->packet_data_handle, NULL);
    qmi_message_wds_start_network_output_unref (output);

#undef VALIDATE_UNKNOWN
#define VALIDATE_UNKNOWN(str) (str ? str : "unknown")

    g_print ("[%s] Network started\n"
             "\tPacket data handle: %u\n",
             qmi_device_get_path_display (ctx->device),
             (guint)ctx->packet_data_handle);

    if (follow_network_flag) {
        g_print ("\nCtrl+C will stop the network\n");
        ctx->network_started_id = g_cancellable_connect (ctx->cancellable,
                                                         G_CALLBACK (network_cancelled),
                                                         NULL,
                                                         NULL);

        ctx->packet_status_timeout_id = g_timeout_add_seconds (20,
                                                               (GSourceFunc)packet_status_timeout,
                                                               NULL);
        return;
    }

    /* Nothing else to do */
    shutdown (TRUE);
}

static void
get_packet_service_status_ready (QmiClientWds *client,
                                 GAsyncResult *res)
{
    GError *error = NULL;
    QmiMessageWdsGetPacketServiceStatusOutput *output;
    QmiWdsConnectionStatus status;

    output = qmi_client_wds_get_packet_service_status_finish (client, res, &error);
    if (!output) {
        g_printerr ("error: operation failed: %s\n",
                    error->message);
        g_error_free (error);
        shutdown (FALSE);
        return;
    }

    if (!qmi_message_wds_get_packet_service_status_output_get_result (output, &error)) {
        g_printerr ("error: couldn't get packet service status: %s\n", error->message);
        g_error_free (error);
        qmi_message_wds_get_packet_service_status_output_unref (output);
        shutdown (FALSE);
        return;
    }

    qmi_message_wds_get_packet_service_status_output_get_connection_status (
        output,
        &status,
        NULL);

    g_print ("[%s] Connection status: '%s'\n",
             qmi_device_get_path_display (ctx->device),
             qmi_wds_connection_status_get_string (status));

    qmi_message_wds_get_packet_service_status_output_unref (output);
    shutdown (TRUE);
}

static void
get_data_bearer_technology_ready (QmiClientWds *client,
                                  GAsyncResult *res)
{
    GError *error = NULL;
    QmiMessageWdsGetDataBearerTechnologyOutput *output;
    QmiWdsDataBearerTechnology current;

    output = qmi_client_wds_get_data_bearer_technology_finish (client, res, &error);
    if (!output) {
        g_printerr ("error: operation failed: %s\n",
                    error->message);
        g_error_free (error);
        shutdown (FALSE);
        return;
    }

    if (!qmi_message_wds_get_data_bearer_technology_output_get_result (output, &error)) {
        g_printerr ("error: couldn't get data bearer technology: %s\n", error->message);

        if (g_error_matches (error,
                             QMI_PROTOCOL_ERROR,
                             QMI_PROTOCOL_ERROR_OUT_OF_CALL)) {
            QmiWdsDataBearerTechnology last = QMI_WDS_DATA_BEARER_TECHNOLOGY_UNKNOWN;

            if (qmi_message_wds_get_data_bearer_technology_output_get_last (
                    output,
                    &last,
                    NULL))
                g_print ("[%s] Data bearer technology (last): '%s'(%d)\n",
                         qmi_device_get_path_display (ctx->device),
                         qmi_wds_data_bearer_technology_get_string (last), last);
        }

        g_error_free (error);
        qmi_message_wds_get_data_bearer_technology_output_unref (output);
        shutdown (FALSE);
        return;
    }

    qmi_message_wds_get_data_bearer_technology_output_get_last (
        output,
        &current,
        NULL);
    g_print ("[%s] Data bearer technology (current): '%s'\n",
             qmi_device_get_path_display (ctx->device),
             qmi_wds_data_bearer_technology_get_string (current));
    qmi_message_wds_get_data_bearer_technology_output_unref (output);
    shutdown (TRUE);
}

static void
print_current_data_bearer_technology_results (const gchar *which,
                                              QmiWdsNetworkType network_type,
                                              guint32 rat_mask,
                                              guint32 so_mask)
{
    gchar *rat_string = NULL;
    gchar *so_string = NULL;

    if (network_type == QMI_WDS_NETWORK_TYPE_3GPP2) {
        rat_string = qmi_wds_rat_3gpp2_build_string_from_mask (rat_mask);
        if (rat_mask & QMI_WDS_RAT_3GPP2_CDMA1X)
            so_string = qmi_wds_so_cdma1x_build_string_from_mask (so_mask);
        else if (rat_mask & QMI_WDS_RAT_3GPP2_EVDO_REVA)
            so_string = qmi_wds_so_evdo_reva_build_string_from_mask (so_mask);
    } else if (network_type == QMI_WDS_NETWORK_TYPE_3GPP) {
        rat_string = qmi_wds_rat_3gpp_build_string_from_mask (rat_mask);
    }

    g_print ("[%s] Data bearer technology (%s):\n"
             "              Network type: '%s'\n"
             "   Radio Access Technology: '%s'\n"
             "            Service Option: '%s'\n",
             qmi_device_get_path_display (ctx->device),
             which,
             qmi_wds_network_type_get_string (network_type),
             VALIDATE_UNKNOWN (rat_string),
             VALIDATE_UNKNOWN (so_string));
    g_free (rat_string);
    g_free (so_string);
}

static void
get_current_data_bearer_technology_ready (QmiClientWds *client,
                                          GAsyncResult *res)
{
    GError *error = NULL;
    QmiMessageWdsGetCurrentDataBearerTechnologyOutput *output;
    QmiWdsNetworkType network_type;
    guint32 rat_mask;
    guint32 so_mask;

    output = qmi_client_wds_get_current_data_bearer_technology_finish (client, res, &error);
    if (!output) {
        g_printerr ("error: operation failed: %s\n",
                    error->message);
        g_error_free (error);
        shutdown (FALSE);
        return;
    }

#undef VALIDATE_UNKNOWN
#define VALIDATE_UNKNOWN(str) (str ? str : "unknown")

    if (!qmi_message_wds_get_current_data_bearer_technology_output_get_result (output, &error)) {
        g_printerr ("error: couldn't get current data bearer technology: %s\n", error->message);

        if (qmi_message_wds_get_current_data_bearer_technology_output_get_last (
                output,
                &network_type,
                &rat_mask,
                &so_mask,
                NULL)) {
            print_current_data_bearer_technology_results (
                "last",
                network_type,
                rat_mask,
                so_mask);
        }

        g_error_free (error);
        qmi_message_wds_get_current_data_bearer_technology_output_unref (output);
        shutdown (FALSE);
        return;
    }

    /* Retrieve CURRENT */
    if (qmi_message_wds_get_current_data_bearer_technology_output_get_current (
            output,
            &network_type,
            &rat_mask,
            &so_mask,
            NULL)) {
        print_current_data_bearer_technology_results (
            "current",
            network_type,
            rat_mask,
            so_mask);
    }

    qmi_message_wds_get_current_data_bearer_technology_output_unref (output);
    shutdown (TRUE);
}

static void
reset_ready (QmiClientWds *client,
             GAsyncResult *res)
{
    QmiMessageWdsResetOutput *output;
    GError *error = NULL;

    output = qmi_client_wds_reset_finish (client, res, &error);
    if (!output) {
        g_printerr ("error: operation failed: %s\n", error->message);
        g_error_free (error);
        shutdown (FALSE);
        return;
    }

    if (!qmi_message_wds_reset_output_get_result (output, &error)) {
        g_printerr ("error: couldn't reset the WDS service: %s\n", error->message);
        g_error_free (error);
        qmi_message_wds_reset_output_unref (output);
        shutdown (FALSE);
        return;
    }

    g_print ("[%s] Successfully performed WDS service reset\n",
             qmi_device_get_path_display (ctx->device));

    qmi_message_wds_reset_output_unref (output);
    shutdown (TRUE);
}

static gboolean
noop_cb (gpointer unused)
{
    shutdown (TRUE);
    return FALSE;
}

void
qmicli_wds_run (QmiDevice *device,
                QmiClientWds *client,
                GCancellable *cancellable)
{
    /* Initialize context */
    ctx = g_slice_new (Context);
    ctx->device = g_object_ref (device);
    ctx->client = g_object_ref (client);
    ctx->cancellable = g_object_ref (cancellable);
    ctx->network_started_id = 0;
    ctx->packet_status_timeout_id = 0;

    /* Request to start network? */
    if (start_network_str) {
        QmiMessageWdsStartNetworkInput *input = NULL;

        /* Use the input string as APN */
        if (start_network_str[0]) {
            input = qmi_message_wds_start_network_input_new ();
            qmi_message_wds_start_network_input_set_apn (input, start_network_str, NULL);
        }

        g_debug ("Asynchronously starting network...");
        qmi_client_wds_start_network (ctx->client,
                                      input,
                                      10,
                                      ctx->cancellable,
                                      (GAsyncReadyCallback)start_network_ready,
                                      NULL);
        if (input)
            qmi_message_wds_start_network_input_unref (input);
        return;
    }

    /* Request to stop network? */
    if (stop_network_str) {
        gulong packet_data_handle;

        packet_data_handle = strtoul (stop_network_str, NULL, 10);
        if (!packet_data_handle ||
            packet_data_handle > G_MAXUINT32) {
            g_printerr ("error: invalid packet data handle given '%s'\n",
                        stop_network_str);
            shutdown (FALSE);
            return;
        }

        g_debug ("Asynchronously stopping network...");
        internal_stop_network (ctx->cancellable, (guint32)packet_data_handle);
        return;
    }

    /* Request to get packet service status? */
    if (get_packet_service_status_flag) {
        g_debug ("Asynchronously getting packet service status...");
        qmi_client_wds_get_packet_service_status (ctx->client,
                                                  NULL,
                                                  10,
                                                  ctx->cancellable,
                                                  (GAsyncReadyCallback)get_packet_service_status_ready,
                                                  NULL);
        return;
    }

    /* Request to get data bearer technology? */
    if (get_data_bearer_technology_flag) {
        g_debug ("Asynchronously getting data bearer technology...");
        qmi_client_wds_get_data_bearer_technology (ctx->client,
                                                   NULL,
                                                   10,
                                                   ctx->cancellable,
                                                   (GAsyncReadyCallback)get_data_bearer_technology_ready,
                                                   NULL);
        return;
    }

    /* Request to get current data bearer technology? */
    if (get_current_data_bearer_technology_flag) {
        g_debug ("Asynchronously getting current data bearer technology...");
        qmi_client_wds_get_current_data_bearer_technology (ctx->client,
                                                           NULL,
                                                           10,
                                                           ctx->cancellable,
                                                           (GAsyncReadyCallback)get_current_data_bearer_technology_ready,
                                                           NULL);
        return;
    }

    /* Request to reset WDS service? */
    if (reset_flag) {
        g_debug ("Asynchronously resetting WDS service...");
        qmi_client_wds_reset (ctx->client,
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
