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
 * Copyright (C) 2014 Aleksander Morgado <aleksander@aleksander.es>
 * Copyright (C) 2014 Smith Micro Software, Inc.
 */

#include <string.h>
#include "mbim-proxy-helpers.h"
#include "mbim-message-private.h"
#include "mbim-message.h"
#include "mbim-cid.h"
#include "mbim-uuid.h"

/*****************************************************************************/

static gboolean
cmp_event_entry_contents (const MbimEventEntry *in,
                          const MbimEventEntry *out)
{
    guint i, o;

    g_assert (mbim_uuid_cmp (&(in->device_service_id), &(out->device_service_id)));

    /* First, compare number of cids in the array */
    if (in->cids_count != out->cids_count)
        return FALSE;

    if (in->cids_count == 0)
        g_assert (in->cids == NULL);
    if (out->cids_count == 0)
        g_assert (out->cids == NULL);

    for (i = 0; i < in->cids_count; i++) {
        for (o = 0; o < out->cids_count; o++) {
            if (in->cids[i] == out->cids[o])
                break;
        }
        if (o == out->cids_count)
            return FALSE;
    }

    return TRUE;
}

gboolean
_mbim_proxy_helper_service_subscribe_list_cmp (const MbimEventEntry * const *a,
                                               gsize                         a_size,
                                               const MbimEventEntry * const *b,
                                               gsize                         b_size)
{
    gsize i, o;

    /* First, compare number of entries a the array */
    if (a_size != b_size)
        return FALSE;

    /* Now compare each service one by one */
    for (i = 0; i < a_size; i++) {
        /* Look for this same service a the other array */
        for (o = 0; o < b_size; o++) {
            /* When service found, compare contents */
            if (mbim_uuid_cmp (&(a[i]->device_service_id), &(b[o]->device_service_id))) {
                if (!cmp_event_entry_contents (a[i], b[o]))
                    return FALSE;
                break;
            }
        }
        /* Service not found! */
        if (!b[o])
            return FALSE;
    }

    return TRUE;
}

/*****************************************************************************/

void
_mbim_proxy_helper_service_subscribe_list_debug (const MbimEventEntry * const *list,
                                                 gsize                         list_size)
{
    gsize i;

    for (i = 0; i < list_size; i++) {
        const MbimEventEntry *entry = list[i];
        MbimService service;
        gchar *str;

        service = mbim_uuid_to_service (&entry->device_service_id);
        str = mbim_uuid_get_printable (&entry->device_service_id);
        g_debug ("[service %u] %s (%s)",
                 (guint)i, str, mbim_service_lookup_name (service));
        g_free (str);

        if (entry->cids_count == 0)
            g_debug ("[service %u] all CIDs enabled", (guint)i);
        else {
            guint j;

            g_debug ("[service %u] %u CIDs enabled", (guint)i, entry->cids_count);;
            for (j = 0; j < entry->cids_count; j++) {
                const gchar *cid_str;

                cid_str = mbim_cid_get_printable (service, entry->cids[j]);
                g_debug ("[service %u] [cid %u] %u (%s)",
                         (guint)i, j, entry->cids[j], cid_str ? cid_str : "unknown");
            }
        }
    }
}

/*****************************************************************************/

MbimEventEntry **
_mbim_proxy_helper_service_subscribe_standard_list_new (gsize *out_size)
{
    gsize i;
    MbimService service;
    MbimEventEntry **out;

    g_assert (out_size != NULL);

    out = g_new0 (MbimEventEntry *, 1 + (MBIM_SERVICE_DSS - MBIM_SERVICE_BASIC_CONNECT + 1));

    for (service = MBIM_SERVICE_BASIC_CONNECT, i = 0;
         service <= MBIM_SERVICE_DSS;
         service++, i++) {
         out[i] = g_new0 (MbimEventEntry, 1);
         memcpy (&out[i]->device_service_id, mbim_uuid_from_service (service), sizeof (MbimUuid));
    }

    *out_size = i;
    return out;
}

/*****************************************************************************/

MbimEventEntry **
_mbim_proxy_helper_service_subscribe_request_parse (MbimMessage *message,
                                                    gsize       *out_size)
{
    MbimEventEntry **array = NULL;
    guint32 i;
    guint32 element_count;
    guint32 offset = 0;
    guint32 array_offset;
    MbimEventEntry *event;

    g_assert (message != NULL);
    g_assert (out_size != NULL);

    element_count = _mbim_message_read_guint32 (message, offset);
    if (element_count) {
        array = g_new (MbimEventEntry *, element_count + 1);

        offset += 4;
        for (i = 0; i < element_count; i++) {
            array_offset = _mbim_message_read_guint32 (message, offset);

            event = g_new (MbimEventEntry, 1);

            memcpy (&(event->device_service_id), _mbim_message_read_uuid (message, array_offset), 16);
            array_offset += 16;

            event->cids_count = _mbim_message_read_guint32 (message, array_offset);
            array_offset += 4;

            if (event->cids_count)
                event->cids = _mbim_message_read_guint32_array (message, event->cids_count, array_offset);
            else
                event->cids = NULL;

            array[i] = event;
            offset += 8;
        }

        array[element_count] = NULL;
    }

    *out_size = element_count;
    return array;
}

/*****************************************************************************/

MbimEventEntry **
_mbim_proxy_helper_service_subscribe_list_merge (MbimEventEntry **in,
                                                 gsize            in_size,
                                                 MbimEventEntry **merge,
                                                 gsize            merge_size,
                                                 gsize           *out_size)
{
    gsize m;

    g_assert (out_size != NULL);

    *out_size = in_size;

    if (!merge || !merge_size)
        return in;

    for (m = 0; m < merge_size; m++) {
        MbimEventEntry *entry = NULL;

        /* look for matching uuid */
        if (in && in_size) {
            gsize i;

            for (i = 0; i < in_size; i++) {
                if (mbim_uuid_cmp (&merge[m]->device_service_id, &in[i]->device_service_id)) {
                    entry = in[i];
                    break;
                }
            }
        }

        /* matching uuid not found in merge array, add it */
        if (!entry) {
            gsize o;

            /* Index of the new element to add... */
            o = *out_size;

            /* Increase number of events in the output array */
            (*out_size)++;
            in = g_realloc (in, sizeof (MbimEventEntry *) * (*out_size + 1));
            in[o] = g_memdup (merge[m], sizeof (MbimEventEntry));
            if (merge[m]->cids_count)
                in[o]->cids = g_memdup (merge[m]->cids, sizeof (guint32) * merge[m]->cids_count);
            else
                in[o]->cids = NULL;
            in[*out_size] = NULL;
        } else {
            gsize cm, co;

            /* matching uuid found, add cids */
            if (!entry->cids_count)
                /* all cids already enabled for uuid */
                continue;

            /* If we're adding all enabled cids, directly apply that */
            if (merge[m]->cids_count == 0) {
                g_free (entry->cids);
                entry->cids = NULL;
                entry->cids_count = 0;
            }

            for (cm = 0; cm < merge[m]->cids_count; cm++) {
                for (co = 0; co < entry->cids_count; co++) {
                    if (merge[m]->cids[cm] == entry->cids[co]) {
                        break;
                    }
                }

                if (co == entry->cids_count) {
                    /* cid not found in merge array, add it */
                    entry->cids = g_realloc (entry->cids, sizeof (guint32) * (++entry->cids_count));
                    entry->cids[co] = merge[m]->cids[cm];
                }
            }
        }
    }

    return in;
}
