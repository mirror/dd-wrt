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
 * Copyright (C) 2013 - 2014 Aleksander Morgado <aleksander@aleksander.es>
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <errno.h>

#include <glib.h>
#include <glib/gprintf.h>
#include <gio/gio.h>
#include <glib-unix.h>

#include <libmbim-glib.h>

#include "mbimcli.h"
#include "mbimcli-helpers.h"

#define PROGRAM_NAME    "mbimcli"
#define PROGRAM_VERSION PACKAGE_VERSION

/* Globals */
static GMainLoop *loop;
static GCancellable *cancellable;
static MbimDevice *device;
static MbimService service;
static gboolean operation_status;

/* Main options */
static gchar *device_str;
static gboolean device_open_proxy_flag;
static gchar *no_open_str;
static gboolean no_close_flag;
static gboolean noop_flag;
static gboolean verbose_flag;
static gboolean silent_flag;
static gboolean version_flag;

static GOptionEntry main_entries[] = {
    { "device", 'd', 0, G_OPTION_ARG_STRING, &device_str,
      "Specify device path",
      "[PATH]"
    },
    { "device-open-proxy", 'p', 0, G_OPTION_ARG_NONE, &device_open_proxy_flag,
      "Request to use the 'mbim-proxy' proxy",
      NULL
    },
    { "no-open", 0, 0, G_OPTION_ARG_STRING, &no_open_str,
      "Do not explicitly open the MBIM device before running the command",
      "[Transaction ID]"
    },
    { "no-close", 0, 0, G_OPTION_ARG_NONE, &no_close_flag,
      "Do not close the MBIM device after running the command",
      NULL
    },
    { "noop", 0, 0, G_OPTION_ARG_NONE, &noop_flag,
      "Don't run any command",
      NULL
    },
    { "verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose_flag,
      "Run action with verbose logs, including the debug ones",
      NULL
    },
    { "silent", 0, 0, G_OPTION_ARG_NONE, &silent_flag,
      "Run action with no logs; not even the error/warning ones",
      NULL
    },
    { "version", 'V', 0, G_OPTION_ARG_NONE, &version_flag,
      "Print version",
      NULL
    },
    { NULL }
};

static gboolean
signals_handler (gpointer psignum)
{
    if (cancellable) {
        /* Ignore consecutive requests of cancellation */
        if (!g_cancellable_is_cancelled (cancellable)) {
            g_printerr ("cancelling the operation...\n");
            g_cancellable_cancel (cancellable);
            /* Re-set the signal handler to allow main loop cancellation on
             * second signal */
            g_unix_signal_add (GPOINTER_TO_INT (psignum),  (GSourceFunc) signals_handler, psignum);
            return FALSE;
        }
    }

    if (loop && g_main_loop_is_running (loop)) {
        g_printerr ("cancelling the main loop...\n");
        g_idle_add ((GSourceFunc) g_main_loop_quit, loop);
    }
    return FALSE;
}

static void
log_handler (const gchar *log_domain,
             GLogLevelFlags log_level,
             const gchar *message,
             gpointer user_data)
{
    const gchar *log_level_str;
    time_t now;
    gchar time_str[64];
    struct tm *local_time;
    gboolean err;

    /* Nothing to do if we're silent */
    if (silent_flag)
        return;

    now = time ((time_t *) NULL);
    local_time = localtime (&now);
    strftime (time_str, 64, "%d %b %Y, %H:%M:%S", local_time);
    err = FALSE;

    switch (log_level) {
    case G_LOG_LEVEL_WARNING:
        log_level_str = "-Warning **";
        err = TRUE;
        break;

    case G_LOG_LEVEL_CRITICAL:
    case G_LOG_FLAG_FATAL:
    case G_LOG_LEVEL_ERROR:
        log_level_str = "-Error **";
        err = TRUE;
        break;

    case G_LOG_LEVEL_DEBUG:
        log_level_str = "[Debug]";
        break;

    default:
        log_level_str = "";
        break;
    }

    if (!verbose_flag && !err)
        return;

    g_fprintf (err ? stderr : stdout,
               "[%s] %s %s\n",
               time_str,
               log_level_str,
               message);
}

static void
print_version_and_exit (void)
{
    g_print ("\n"
             PROGRAM_NAME " " PROGRAM_VERSION "\n"
             "Copyright (2013-2014) Aleksander Morgado\n"
             "License GPLv2+: GNU GPL version 2 or later <http://gnu.org/licenses/gpl-2.0.html>\n"
             "This is free software: you are free to change and redistribute it.\n"
             "There is NO WARRANTY, to the extent permitted by law.\n"
             "\n");
    exit (EXIT_SUCCESS);
}

/*****************************************************************************/
/* Running asynchronously */

static void
device_close_ready (MbimDevice   *dev,
                    GAsyncResult *res)
{
    GError *error = NULL;

    if (!mbim_device_close_finish (dev, res, &error)) {
        g_printerr ("error: couldn't close device: %s", error->message);
        g_error_free (error);
    } else
        g_debug ("Device closed");

    /* If we left the device open, dump next transaction id */
    if (no_close_flag) {
        guint transaction_id;

        g_object_get (dev,
                      MBIM_DEVICE_TRANSACTION_ID, &transaction_id,
                      NULL);

        g_print ("[%s] Session not closed:\n"
                 "\t    TRID: '%u'\n",
                 mbim_device_get_path_display (dev),
                 transaction_id);
    }

    g_main_loop_quit (loop);
}

void
mbimcli_async_operation_done (gboolean reported_operation_status)
{
    /* Keep the result of the operation */
    operation_status = reported_operation_status;

    /* Cleanup cancellation */
    g_clear_object (&cancellable);

    /* Set the in-session setup */
    g_object_set (device,
                  MBIM_DEVICE_IN_SESSION, no_close_flag,
                  NULL);

    /* Close the device */
    mbim_device_close (device,
                       15,
                       cancellable,
                       (GAsyncReadyCallback) device_close_ready,
                       NULL);
}

static void
device_open_ready (MbimDevice   *dev,
                   GAsyncResult *res)
{
    GError *error = NULL;

    if (!mbim_device_open_finish (dev, res, &error)) {
        g_printerr ("error: couldn't open the MbimDevice: %s\n",
                    error->message);
        exit (EXIT_FAILURE);
    }

    g_debug ("MBIM Device at '%s' ready",
             mbim_device_get_path_display (dev));

    /* If no operation requested, finish */
    if (noop_flag) {
        mbimcli_async_operation_done (TRUE);
        return;
    }

    /* Run the service-specific action */
    switch (service) {
    case MBIM_SERVICE_BASIC_CONNECT:
        mbimcli_basic_connect_run (dev, cancellable);
        return;
    case MBIM_SERVICE_PHONEBOOK:
        mbimcli_phonebook_run (dev, cancellable);
        return;
    case MBIM_SERVICE_DSS:
        mbimcli_dss_run (dev, cancellable);
        return;
    case MBIM_SERVICE_MS_FIRMWARE_ID:
        mbimcli_ms_firmware_id_run (dev, cancellable);
        return;
    case MBIM_SERVICE_MS_HOST_SHUTDOWN:
        mbimcli_ms_host_shutdown_run (dev, cancellable);
        return;
    case MBIM_SERVICE_ATDS:
        mbimcli_atds_run (dev, cancellable);
        return;
    default:
        g_assert_not_reached ();
    }
}

static void
device_new_ready (GObject      *unused,
                  GAsyncResult *res)
{
    GError *error = NULL;
    MbimDeviceOpenFlags open_flags = MBIM_DEVICE_OPEN_FLAGS_NONE;

    device = mbim_device_new_finish (res, &error);
    if (!device) {
        g_printerr ("error: couldn't create MbimDevice: %s\n",
                    error->message);
        exit (EXIT_FAILURE);
    }

    /* Set the in-session setup */
    if (no_open_str) {
        guint transaction_id;

        if (!mbimcli_read_uint_from_string (no_open_str, &transaction_id)) {
            g_printerr ("error: invalid transaction ID specified: %s\n",
                        no_open_str);
            exit (EXIT_FAILURE);
        }

        g_object_set (device,
                      MBIM_DEVICE_IN_SESSION,     TRUE,
                      MBIM_DEVICE_TRANSACTION_ID, transaction_id,
                      NULL);
    }

    /* Setup device open flags */
    if (device_open_proxy_flag)
        open_flags |= MBIM_DEVICE_OPEN_FLAGS_PROXY;

    /* Open the device */
    mbim_device_open_full (device,
                           open_flags,
                           30,
                           cancellable,
                           (GAsyncReadyCallback) device_open_ready,
                           NULL);
}

/*****************************************************************************/

static void
parse_actions (void)
{
    guint actions_enabled = 0;

    /* Basic Connect options? */
    if (mbimcli_basic_connect_options_enabled ()) {
        service = MBIM_SERVICE_BASIC_CONNECT;
        actions_enabled++;
    } else if (mbimcli_phonebook_options_enabled ()) {
        service = MBIM_SERVICE_PHONEBOOK;
        actions_enabled++;
    } else if (mbimcli_dss_options_enabled ()) {
        service = MBIM_SERVICE_DSS;
        actions_enabled++;
    } else if (mbimcli_ms_firmware_id_options_enabled ()) {
        service = MBIM_SERVICE_MS_FIRMWARE_ID;
        actions_enabled++;
    } else if (mbimcli_ms_host_shutdown_options_enabled ()) {
        service = MBIM_SERVICE_MS_HOST_SHUTDOWN;
        actions_enabled++;
    } else if (mbimcli_atds_options_enabled ()) {
        service = MBIM_SERVICE_ATDS;
        actions_enabled++;
    }

    /* Noop */
    if (noop_flag)
        actions_enabled++;

    /* Cannot mix actions from different services */
    if (actions_enabled > 1) {
        g_printerr ("error: cannot execute multiple actions of different services\n");
        exit (EXIT_FAILURE);
    }

    /* No options? */
    if (actions_enabled == 0) {
        g_printerr ("error: no actions specified\n");
        exit (EXIT_FAILURE);
    }

    /* Go on! */
}

int main (int argc, char **argv)
{
    GError *error = NULL;
    GFile *file;
    GOptionContext *context;

    setlocale (LC_ALL, "");

    /* Setup option context, process it and destroy it */
    context = g_option_context_new ("- Control MBIM devices");
    g_option_context_add_group (context,
                                mbimcli_basic_connect_get_option_group ());
    g_option_context_add_group (context,
                                mbimcli_phonebook_get_option_group ());
    g_option_context_add_group (context,
                                mbimcli_dss_get_option_group ());
    g_option_context_add_group (context,
                                mbimcli_ms_firmware_id_get_option_group ());
    g_option_context_add_group (context,
                                mbimcli_ms_host_shutdown_get_option_group ());
    g_option_context_add_group (context,
                                mbimcli_atds_get_option_group ());
    g_option_context_add_main_entries (context, main_entries, NULL);
    if (!g_option_context_parse (context, &argc, &argv, &error)) {
        g_printerr ("error: %s\n",
                    error->message);
        exit (EXIT_FAILURE);
    }
    g_option_context_free (context);

    if (version_flag)
        print_version_and_exit ();

    g_log_set_handler (NULL, G_LOG_LEVEL_MASK, log_handler, NULL);
    g_log_set_handler ("Mbim", G_LOG_LEVEL_MASK, log_handler, NULL);
    if (verbose_flag)
        mbim_utils_set_traces_enabled (TRUE);

    /* No device path given? */
    if (!device_str) {
        g_printerr ("error: no device path specified\n");
        exit (EXIT_FAILURE);
    }

    /* Build new GFile from the commandline arg */
    file = g_file_new_for_commandline_arg (device_str);

    parse_actions ();

    /* Create requirements for async options */
    cancellable = g_cancellable_new ();
    loop = g_main_loop_new (NULL, FALSE);

    /* Setup signals */
    g_unix_signal_add (SIGINT,  (GSourceFunc)signals_handler, GUINT_TO_POINTER (SIGINT));
    g_unix_signal_add (SIGHUP,  (GSourceFunc)signals_handler, GUINT_TO_POINTER (SIGHUP));
    g_unix_signal_add (SIGTERM, (GSourceFunc)signals_handler, GUINT_TO_POINTER (SIGTERM));

    /* Launch MbimDevice creation */
    mbim_device_new (file,
                     cancellable,
                     (GAsyncReadyCallback)device_new_ready,
                     NULL);
    g_main_loop_run (loop);

    if (cancellable)
        g_object_unref (cancellable);
    if (device)
        g_object_unref (device);
    g_main_loop_unref (loop);
    g_object_unref (file);

    return (operation_status ? EXIT_SUCCESS : EXIT_FAILURE);
}
