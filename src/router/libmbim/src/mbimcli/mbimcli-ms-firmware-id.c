/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 * Copyright (C) 2014 Google, Inc.
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>

#include <glib.h>
#include <gio/gio.h>

#include <libmbim-glib.h>

#include "mbimcli.h"

/* Context */
typedef struct {
    MbimDevice *device;
    GCancellable *cancellable;
} Context;
static Context *ctx;

/* Options */
static gboolean query_firmware_id_flag;

static GOptionEntry entries[] = {
    { "ms-query-firmware-id", 0, 0, G_OPTION_ARG_NONE, &query_firmware_id_flag,
      "Query firmware ID",
      NULL
    },
    { NULL }
};

GOptionGroup *
mbimcli_ms_firmware_id_get_option_group (void)
{
   GOptionGroup *group;

   group = g_option_group_new ("ms-firmware-id",
                               "Microsoft Firmware ID options",
                               "Show Microsoft Firmware ID Service options",
                               NULL,
                               NULL);
   g_option_group_add_entries (group, entries);

   return group;
}

gboolean
mbimcli_ms_firmware_id_options_enabled (void)
{
    static guint n_actions = 0;
    static gboolean checked = FALSE;

    if (checked)
        return !!n_actions;

    n_actions = query_firmware_id_flag;

    if (n_actions > 1) {
        g_printerr ("error: too many Microsoft Firmware ID actions requested\n");
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

static void
query_firmware_id_ready (MbimDevice   *device,
                         GAsyncResult *res)
{
    MbimMessage *response;
    GError *error = NULL;
    const MbimUuid *firmware_id;
    gchar *firmware_id_str;

    response = mbim_device_command_finish (device, res, &error);
    if (!response || !mbim_message_response_get_result (response, MBIM_MESSAGE_TYPE_COMMAND_DONE, &error)) {
        g_printerr ("error: operation failed: %s\n", error->message);
        g_error_free (error);
        if (response)
            mbim_message_unref (response);
        shutdown (FALSE);
        return;
    }

    if (!mbim_message_ms_firmware_id_get_response_parse (
            response,
            &firmware_id,
            &error)) {
        g_printerr ("error: couldn't parse response message: %s\n", error->message);
        g_error_free (error);
        shutdown (FALSE);
        return;
    }

    firmware_id_str = firmware_id ? mbim_uuid_get_printable (firmware_id) : NULL;

    g_print ("[%s] Firmware ID retrieved: '%s'\n\n",
             mbim_device_get_path_display (device),
             VALIDATE_UNKNOWN (firmware_id_str));
    g_free (firmware_id_str);

    mbim_message_unref (response);
    shutdown (TRUE);
}

void
mbimcli_ms_firmware_id_run (MbimDevice   *device,
                            GCancellable *cancellable)
{
    /* Initialize context */
    ctx = g_slice_new (Context);
    ctx->device = g_object_ref (device);
    if (cancellable)
        ctx->cancellable = g_object_ref (cancellable);

    /* Request to get firmware ID? */
    if (query_firmware_id_flag) {
        MbimMessage *request;

        g_debug ("Asynchronously querying firmware ID...");
        request = (mbim_message_ms_firmware_id_get_query_new (NULL));
        mbim_device_command (ctx->device,
                             request,
                             10,
                             ctx->cancellable,
                             (GAsyncReadyCallback)query_firmware_id_ready,
                             NULL);
        mbim_message_unref (request);
        return;
    }

    g_warn_if_reached ();
}
