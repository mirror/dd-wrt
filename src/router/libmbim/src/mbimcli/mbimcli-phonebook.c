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
 * Copyright (C) 2013 - 2014 Nagaraju Kadiri <kadiri.raju@gmail.com>
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
} Context;
static Context *ctx;

/* Options */
static gboolean  phonebook_configuration_flag;
static gint      phonebook_read_index;
static gboolean  phonebook_read_all_flag;
static gchar    *phonebook_write_str;
static gint      phonebook_delete_index;
static gboolean  phonebook_delete_all_flag;

static GOptionEntry entries[] = {
    { "phonebook-query-configuration", 0, 0, G_OPTION_ARG_NONE, &phonebook_configuration_flag,
      "Query the phonebook configuration",
      NULL
    },
    { "phonebook-read", 0, 0, G_OPTION_ARG_INT, &phonebook_read_index,
      "Read phonebook entry with given index",
      "[(Phonebook index)]"
    },
    { "phonebook-read-all", 0, 0, G_OPTION_ARG_NONE, &phonebook_read_all_flag,
      "Read all phonebook entries",
      NULL
    },
    { "phonebook-write", 0, 0, G_OPTION_ARG_STRING, &phonebook_write_str,
      "Add new phonebook entry or update an existing one",
      "[(Name),(Number)[,(Index)]]"
    },
    { "phonebook-delete", 0, 0, G_OPTION_ARG_INT, &phonebook_delete_index,
      "Delete phonebook entry with given index",
      "[(Phonebook index)]"
    },
    { "phonebook-delete-all", 0, 0, G_OPTION_ARG_NONE, &phonebook_delete_all_flag,
      "Delete all phonebook entries",
      NULL
    },
    { NULL }
};

GOptionGroup *
mbimcli_phonebook_get_option_group (void)
{
    GOptionGroup *group;

    group = g_option_group_new ("phonebook",
                                "Phonebook options",
                                "Show Phonebook Service options",
                                NULL,
                                NULL);
    g_option_group_add_entries (group, entries);

    return group;
}

gboolean
mbimcli_phonebook_options_enabled (void)
{
    static guint n_actions = 0;
    static gboolean checked = FALSE;

    if (checked)
        return !!n_actions;

    n_actions = (phonebook_configuration_flag +
                 !!phonebook_read_index +
                 phonebook_read_all_flag +
                 !!phonebook_write_str +
                 !!phonebook_delete_index +
                 phonebook_delete_all_flag);

    if (n_actions > 1) {
        g_printerr ("error: too many phonebook actions requested\n");
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

static gboolean
phonebook_write_input_parse (const gchar  *str,
                             gchar       **name,
                             gchar       **number,
                             guint        *idx)
{
    gchar **split;

    g_assert (name != NULL);
    g_assert (number != NULL);
    g_assert (idx != NULL);

    /* Format of the string is:
     *    "[(Name),(Number)(,[Index])]"
     * i.e. index is optional
     */
    split = g_strsplit (str, ",", -1);

    if (g_strv_length (split) > 3) {
        g_printerr ("error: couldn't parse input string, too many arguments\n");
        g_strfreev (split);
        return FALSE;
    }

    if (g_strv_length (split) < 2) {
        g_printerr ("error: couldn't parse input string, missing arguments\n");
        g_strfreev (split);
        return FALSE;
    }

    /* Check whether we have the optional Index item */
    if (split[2]) {
        if (!mbimcli_read_uint_from_string (split[2], idx)) {
            g_printerr ("error: couldn't parse input string, invalid index '%s'\n", split[2]);
            g_strfreev (split);
            return FALSE;
        }
    } else {
        /* Default to index 0, which is an invalid one */
        *idx = 0;
    }

    /* First two items will always be available */
    *name = g_strdup (split[0]);
    *number = g_strdup (split[1]);

    g_strfreev (split);
    return TRUE;
}

static void
set_phonebook_write_ready (MbimDevice   *device,
                           GAsyncResult *res)
{
    MbimMessage *response;
    GError *error = NULL;

    response = mbim_device_command_finish (device, res, &error);
    if (!response || !mbim_message_response_get_result (response, MBIM_MESSAGE_TYPE_COMMAND_DONE, &error)) {
        g_printerr ("error: operation failed: %s\n", error->message);
        g_error_free (error);
        if (response)
            mbim_message_unref (response);
        shutdown (FALSE);
        return;
    }

    if (!mbim_message_phonebook_write_response_parse (response, &error)) {
        g_printerr ("error: couldn't parse response message: %s\n", error->message);
        g_error_free (error);
        shutdown (FALSE);
        return;
    }

    g_print ("Phonebook entry successfully written\n");

    mbim_message_unref (response);
    shutdown (TRUE);
}

static void
set_phonebook_delete_ready (MbimDevice   *device,
                            GAsyncResult *res)
{
    MbimMessage *response;
    GError *error = NULL;

    response = mbim_device_command_finish (device, res, &error);
    if (!response || !mbim_message_response_get_result (response, MBIM_MESSAGE_TYPE_COMMAND_DONE, &error)) {
        g_printerr ("error: operation failed: %s\n", error->message);
        g_error_free (error);
        if (response)
            mbim_message_unref (response);
        shutdown (FALSE);
        return;
    }

    if (!mbim_message_phonebook_delete_response_parse (response, &error)) {
        g_printerr ("error: couldn't parse response message: %s\n", error->message);
        g_error_free (error);
        shutdown (FALSE);
    }

    g_print ("Phonebook entry/entries successfully deleted");

    mbim_message_unref (response);
    shutdown (TRUE);
}

static void
query_phonebook_read_ready (MbimDevice   *device,
                            GAsyncResult *res)
{
    MbimMessage *response;
    GError *error = NULL;
    guint32 entry_count;
    MbimPhonebookEntry **phonebook_entries;
    gint i = 0;

    response = mbim_device_command_finish (device, res, &error);
    if (!response || !mbim_message_response_get_result (response, MBIM_MESSAGE_TYPE_COMMAND_DONE, &error)) {
        g_printerr ("error: operation failed: %s\n", error->message);
        g_error_free (error);
        if (response)
            mbim_message_unref (response);
        shutdown (FALSE);
        return;
    }

    if (!mbim_message_phonebook_read_response_parse (response,
                                                     &entry_count,
                                                     &phonebook_entries,
                                                     &error)) {
        g_printerr ("error: couldn't parse response message: %s\n", error->message);
        g_error_free (error);
        shutdown (FALSE);
        return;
    }

    g_print ("Successfully read phonebook entries (%d)\n", entry_count);
    for (i = 0; i < entry_count; i++) {
        g_print ("\tEntry index : %d \n"
                 "\t      Number: %s \n"
                 "\t        Name: %s \n",
                 phonebook_entries[i]->entry_index,
                 phonebook_entries[i]->number,
                 phonebook_entries[i]->name);
    }
    mbim_phonebook_entry_array_free (phonebook_entries);

    mbim_message_unref (response);
    shutdown (TRUE);
}

static void
query_phonebook_configuration_ready (MbimDevice   *device,
                                     GAsyncResult *res)
{
    MbimMessage *response;
    GError *error = NULL;
    MbimPhonebookState state;
    const gchar *state_str;
    guint32 number_of_entries;
    guint32 used_entries;
    guint32 max_number_length;
    guint32 max_name;

    response = mbim_device_command_finish (device, res, &error);
    if (!response || !mbim_message_response_get_result (response, MBIM_MESSAGE_TYPE_COMMAND_DONE, &error)) {
        g_printerr ("error: operation failed: %s\n", error->message);
        g_error_free (error);
        if (response)
            mbim_message_unref (response);
        shutdown (FALSE);
        return;
    }

    if (!mbim_message_phonebook_configuration_response_parse (response,
                                                              &state,
                                                              &number_of_entries,
                                                              &used_entries,
                                                              &max_number_length,
                                                              &max_name,
                                                              &error)) {
        g_printerr ("error: couldn't parse response message: %s\n", error->message);
        g_error_free (error);
        shutdown (FALSE);
        return;
    }

    state_str = mbim_phonebook_state_get_string (state);

    g_print ("\n Phonebook configuration retrieved... \n"
             "\t   Phonebook state: %s \n"
             "\t Number of entries: %d \n"
             "\t      used entries: %d \n"
             "\t max number length: %d \n"
             "\t         max name : %d \n",
             VALIDATE_UNKNOWN (state_str),
             number_of_entries,
             used_entries,
             max_number_length,
             max_name);

    mbim_message_unref (response);
    shutdown (TRUE);
}

void
mbimcli_phonebook_run (MbimDevice   *device,
                       GCancellable *cancellable)
{
    /* Initialize context */
    ctx = g_slice_new (Context);
    ctx->device = g_object_ref (device);
    if (cancellable)
        ctx->cancellable = g_object_ref (cancellable);

    /* Request to get configuration? */
    if (phonebook_configuration_flag) {
        MbimMessage *request;

        g_debug ("Asynchronously querying phonebook configurations...");
        request = mbim_message_phonebook_configuration_query_new (NULL);
        mbim_device_command (ctx->device,
                             request,
                             10,
                             ctx->cancellable,
                             (GAsyncReadyCallback)query_phonebook_configuration_ready,
                             NULL);
        mbim_message_unref (request);
        return;
    }

    /* Phonebook read */
    if (phonebook_read_index) {
        MbimMessage *request;

        g_debug ("Asynchronously querying phonebook read...");
        request = mbim_message_phonebook_read_query_new (MBIM_PHONEBOOK_FLAG_INDEX,
                                                         phonebook_read_index,
                                                         NULL);
        mbim_device_command (ctx->device,
                             request,
                             10,
                             ctx->cancellable,
                             (GAsyncReadyCallback)query_phonebook_read_ready,
                             NULL);
        mbim_message_unref (request);
        return;
    }

    /* Phonebook read all */
    if (phonebook_read_all_flag) {
        MbimMessage *request;

        g_debug ("Asynchronously querying phonebook read all...");
        request = mbim_message_phonebook_read_query_new (MBIM_PHONEBOOK_FLAG_ALL, 0, NULL);
        mbim_device_command (ctx->device,
                             request,
                             10,
                             ctx->cancellable,
                             (GAsyncReadyCallback)query_phonebook_read_ready,
                             NULL);
        mbim_message_unref (request);
        return;
    }

    /* Phonebook delete */
    if (phonebook_delete_index) {
        MbimMessage *request;

        g_debug ("Asynchronously phonebook delete...");
        request = mbim_message_phonebook_delete_set_new (MBIM_PHONEBOOK_FLAG_INDEX,
                                                         phonebook_delete_index,
                                                         NULL);
        mbim_device_command (ctx->device,
                             request,
                             10,
                             ctx->cancellable,
                             (GAsyncReadyCallback)set_phonebook_delete_ready,
                             NULL);
        mbim_message_unref (request);
        return;
    }

    /* Phonebook delete all */
    if (phonebook_delete_all_flag) {
        MbimMessage *request;

        g_debug ("Asynchronously phonebook delete all...");
        request = mbim_message_phonebook_delete_set_new (MBIM_PHONEBOOK_FLAG_ALL, 0, NULL);
        mbim_device_command (ctx->device,
                             request,
                             10,
                             ctx->cancellable,
                             (GAsyncReadyCallback)set_phonebook_delete_ready,
                             NULL);
        mbim_message_unref (request);
        return;
    }

    /* Phonebook write */
    if (phonebook_write_str) {
        MbimMessage *request;
        gchar *name;
        gchar *number;
        guint  idx;

        g_debug ("Asynchronously writing phonebook...");
        if (!phonebook_write_input_parse (phonebook_write_str, &name, &number, &idx)) {
            shutdown (FALSE);
            return;
        }

        request = mbim_message_phonebook_write_set_new ((idx ?
                                                         MBIM_PHONEBOOK_WRITE_FLAG_SAVE_INDEX :
                                                         MBIM_PHONEBOOK_WRITE_FLAG_SAVE_UNUSED),
                                                        idx,
                                                        number,
                                                        name,
                                                        NULL);
        mbim_device_command (ctx->device,
                             request,
                             10,
                             ctx->cancellable,
                             (GAsyncReadyCallback)set_phonebook_write_ready,
                             NULL);
        mbim_message_unref (request);
        return;
    }

    g_warn_if_reached ();
}
