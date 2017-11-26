/*
 * mbimcli -- Command line interface to control MBIM devices
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
 * Copyright (C) 2014 NVIDIA CORPORATION
 * Copyright (C) 2014 Aleksander Morgado <aleksander@aleksander.es>
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <errno.h>

#include <glib.h>
#include <gio/gio.h>

#include <libmbim-glib.h>

#include "mbimcli.h"
#include "mbimcli-helpers.h"

/* Context */
typedef struct {
    MbimDevice *device;
    GCancellable *cancellable;
    guint32 session_id;
} Context;
static Context *ctx;

/* Options */
static gchar    *connect_str;
static gchar    *disconnect_str;

static GOptionEntry entries[] = {
    { "dss-connect", 0, 0, G_OPTION_ARG_STRING, &connect_str,
      "Connect DSS session",
      "[(UUID),(Session ID)]"
    },
    { "dss-disconnect", 0, 0, G_OPTION_ARG_STRING, &disconnect_str,
      "Disconnect DSS session",
      "[(UUID),(Session ID)]"
    },
    { NULL }
};

GOptionGroup *
mbimcli_dss_get_option_group (void)
{
    GOptionGroup *group;

    group = g_option_group_new ("dss",
                                "Device Service Stream options",
                                "Show Device Service Stream options",
                                NULL,
                                NULL);
    g_option_group_add_entries (group, entries);

    return group;
}

gboolean
mbimcli_dss_options_enabled (void)
{
    static guint n_actions = 0;
    static gboolean checked = FALSE;

    if (checked)
        return !!n_actions;

    n_actions = (!!connect_str +
                 !!disconnect_str );

    if (n_actions > 1) {
        g_printerr ("error: too many DSS actions requested\n");
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
    g_slice_free (Context, context);
}

static void
shutdown (gboolean operation_status)
{
    /* Cleanup context and finish async operation */
    context_free (ctx);
    mbimcli_async_operation_done (operation_status);
}

enum {
    CONNECT,
    DISCONNECT
};

static void
ip_configuration_query_ready (MbimDevice *device,
                              GAsyncResult *res,
                              gpointer unused)
{
    GError *error = NULL;
    MbimMessage *response;
    gboolean success = FALSE;

    response = mbim_device_command_finish (device, res, &error);
    if (!response ||
        !mbim_message_response_get_result (response, MBIM_MESSAGE_TYPE_COMMAND_DONE, &error)) {
        g_printerr ("error: couldn't get IP configuration response message: %s\n", error->message);
    } else {
        success = mbimcli_print_ip_config (device, response, &error);
        if (!success)
            g_printerr ("error: couldn't parse IP configuration response message: %s\n", error->message);
    }

    g_clear_error (&error);
    if (response)
        mbim_message_unref (response);
    shutdown (success);
}

static void
set_dss_ready (MbimDevice *device,
               GAsyncResult *res,
               gpointer user_data)
{
    MbimMessage *response, *message;
    GError *error = NULL;

    response = mbim_device_command_finish (device, res, &error);
    if (!response || !mbim_message_response_get_result (response, MBIM_MESSAGE_TYPE_COMMAND_DONE, &error)) {
        g_printerr ("error: operation failed: %s\n", error->message);
        g_error_free (error);
        shutdown (FALSE);
        return;
    }

    if (!mbim_message_dss_connect_response_parse (response, &error)) {
        g_printerr ("error: couldn't parse response message: %s\n", error->message);
        g_error_free (error);
        mbim_message_unref (response);
        shutdown (FALSE);
        return;
    }
    mbim_message_unref (response);

    if (GPOINTER_TO_UINT (user_data) == DISCONNECT) {
        g_print ("[%s] Successfully disconnected\n",
                 mbim_device_get_path_display (device));
        shutdown (TRUE);
        return;
    }

    g_assert (GPOINTER_TO_UINT (user_data) == CONNECT);

    g_print ("[%s] Successfully connected\n",
             mbim_device_get_path_display (device));

    message = (mbim_message_ip_configuration_query_new (
               ctx->session_id,
               MBIM_IP_CONFIGURATION_AVAILABLE_FLAG_NONE, /* ipv4configurationavailable */
               MBIM_IP_CONFIGURATION_AVAILABLE_FLAG_NONE, /* ipv6configurationavailable */
               0, /* ipv4addresscount */
               NULL, /* ipv4address */
               0, /* ipv6addresscount */
               NULL, /* ipv6address */
               NULL, /* ipv4gateway */
               NULL, /* ipv6gateway */
               0, /* ipv4dnsservercount */
               NULL, /* ipv4dnsserver */
               0, /* ipv6dnsservercount */
               NULL, /* ipv6dnsserver */
               0, /* ipv4mtu */
               0, /* ipv6mtu */
               &error));
    if (!message) {
        g_printerr ("error: couldn't create IP config request: %s\n", error->message);
        g_error_free (error);
        mbim_message_unref (message);
        shutdown (FALSE);
        return;
    }

    mbim_device_command (device,
                         message,
                         60,
                         NULL,
                         (GAsyncReadyCallback)ip_configuration_query_ready,
                         NULL);
    mbim_message_unref (message);
}

static gboolean
common_parse (const gchar *str,
              MbimUuid    *dsid,
              guint32     *ssid)
{
    gchar **split;
    gboolean status = FALSE;

    g_assert (dsid != NULL);
    g_assert (ssid != NULL);

    /* Format of the string is:
     * [(UUID),(Session ID)]
     */
    split = g_strsplit (str, ",", -1);

    if (g_strv_length (split) > 2)
        g_printerr ("error: couldn't parse input string, too many arguments\n");
    else if (g_strv_length (split) < 1)
        g_printerr ("error: couldn't parse input string, missing arguments\n");
    else if (!mbim_uuid_from_printable (split[0], dsid))
        g_printerr ("error: couldn't parse UUID, should be xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx\n");
    else if (!mbimcli_read_uint_from_string (split[1], ssid))
        g_printerr ("error: couldn't parse Session ID, should be a number\n");
    else
        status = TRUE;

    g_strfreev (split);
    return status;
}

void
mbimcli_dss_run (MbimDevice   *device,
                 GCancellable *cancellable)
{
    MbimMessage *request;
    GError *error = NULL;

    /* Initialize context */
    ctx = g_slice_new (Context);
    ctx->device = g_object_ref (device);
    if (cancellable)
        ctx->cancellable = g_object_ref (cancellable);

    /* Connect? */
    if (connect_str) {
        MbimUuid service_id;

        if (!common_parse (connect_str, &service_id, &ctx->session_id)) {
            shutdown (FALSE);
            return;
        }

        request = mbim_message_dss_connect_set_new (&service_id,
                                                    ctx->session_id,
                                                    MBIM_DSS_LINK_STATE_ACTIVATE,
                                                    &error);

        if (!request) {
            g_printerr ("error: couldn't create request: %s\n", error->message);
            g_error_free (error);
            shutdown (FALSE);
            return;
        }

        mbim_device_command (ctx->device,
                             request,
                             10,
                             ctx->cancellable,
                             (GAsyncReadyCallback)set_dss_ready,
                             GUINT_TO_POINTER (CONNECT));
        mbim_message_unref (request);
        return;
    }

    /* Disconnect? */
    if (disconnect_str) {
        MbimUuid service_id;
        guint32 session_id;

        if (!common_parse (disconnect_str, &service_id, &session_id)) {
            shutdown (FALSE);
            return;
        }

        request = mbim_message_dss_connect_set_new (&service_id,
                                                    session_id,
                                                    MBIM_DSS_LINK_STATE_DEACTIVATE,
                                                    &error);
        if (!request) {
            g_printerr ("error: couldn't create request: %s\n", error->message);
            g_error_free (error);
            shutdown (FALSE);
            return;
        }

        mbim_device_command (ctx->device,
                             request,
                             10,
                             ctx->cancellable,
                             (GAsyncReadyCallback)set_dss_ready,
                             GUINT_TO_POINTER (DISCONNECT));
        mbim_message_unref (request);
        return;
    }

    g_warn_if_reached ();
}
