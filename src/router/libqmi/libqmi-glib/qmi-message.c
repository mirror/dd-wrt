/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 * Copyright (c) 2012 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file of the libqmi library.
 */

/*
 * libqmi-glib -- GLib/GIO based library to control QMI devices
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
 * Copyright (C) 2012 Aleksander Morgado <aleksander@lanedo.com>
 */

#include <glib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <endian.h>

#include "qmi-message.h"
#include "qmi-utils.h"
#include "qmi-enums-private.h"
#include "qmi-enum-types-private.h"
#include "qmi-enum-types.h"
#include "qmi-error-types.h"

#include "qmi-ctl.h"
#include "qmi-dms.h"
#include "qmi-wds.h"
#include "qmi-nas.h"
#include "qmi-wms.h"
#include "qmi-pds.h"

/**
 * SECTION:qmi-message
 * @title: QmiMessage
 * @short_description: Generic QMI message handling routines
 *
 * #QmiMessage is a generic type representing a QMI message of any kind
 * (request, response, indication) or service (including #QMI_SERVICE_CTL).
 *
 * This set of generic routines help in handling these message types, and
 * allow creating any kind of message with any kind of TLV.
 **/

#define PACKED __attribute__((packed))

struct qmux {
  guint16 length;
  guint8 flags;
  guint8 service;
  guint8 client;
} PACKED;

struct control_header {
  guint8 flags;
  guint8 transaction;
  guint16 message;
  guint16 tlv_length;
} PACKED;

struct service_header {
  guint8 flags;
  guint16 transaction;
  guint16 message;
  guint16 tlv_length;
} PACKED;

struct tlv {
  guint8 type;
  guint16 length;
  guint8 value[];
} PACKED;

struct control_message {
  struct control_header header;
  struct tlv tlv[];
} PACKED;

struct service_message {
  struct service_header header;
  struct tlv tlv[];
} PACKED;

struct full_message {
    guint8 marker;
    struct qmux qmux;
    union {
        struct control_message control;
        struct service_message service;
    } qmi;
} PACKED;

static inline gboolean
message_is_control (QmiMessage *self)
{
    return ((struct full_message *)(self->data))->qmux.service == QMI_SERVICE_CTL;
}

static inline guint16
get_qmux_length (QmiMessage *self)
{
    return GUINT16_FROM_LE (((struct full_message *)(self->data))->qmux.length);
}

static inline void
set_qmux_length (QmiMessage *self,
                 guint16 length)
{
    ((struct full_message *)(self->data))->qmux.length = GUINT16_TO_LE (length);
}

static inline guint8
get_qmux_flags (QmiMessage *self)
{
    return ((struct full_message *)(self->data))->qmux.flags;
}

static inline guint8
get_qmi_flags (QmiMessage *self)
{
    if (message_is_control (self))
        return ((struct full_message *)(self->data))->qmi.control.header.flags;

    return ((struct full_message *)(self->data))->qmi.service.header.flags;
}

/**
 * qmi_message_is_response:
 * @self: a #QmiMessage.
 *
 * Checks whether the given #QmiMessage is a response.
 *
 * Returns: %TRUE if @self is a response message, %FALSE otherwise.
 */
gboolean
qmi_message_is_response (QmiMessage *self)
{
    if (message_is_control (self)) {
        if (((struct full_message *)(self->data))->qmi.control.header.flags & QMI_CTL_FLAG_RESPONSE)
            return TRUE;
    } else {
        if (((struct full_message *)(self->data))->qmi.service.header.flags & QMI_SERVICE_FLAG_RESPONSE)
            return TRUE;
    }

    return FALSE;
}

/**
 * qmi_message_is_indication:
 * @self: a #QmiMessage.
 *
 * Checks whether the given #QmiMessage is an indication.
 *
 * Returns: %TRUE if @self is an indication message, %FALSE otherwise.
 */
gboolean
qmi_message_is_indication (QmiMessage *self)
{
    if (message_is_control (self)) {
        if (((struct full_message *)(self->data))->qmi.control.header.flags & QMI_CTL_FLAG_INDICATION)
            return TRUE;
    } else {
        if (((struct full_message *)(self->data))->qmi.service.header.flags & QMI_SERVICE_FLAG_INDICATION)
            return TRUE;
    }

    return FALSE;
}

/**
 * qmi_message_get_service:
 * @self: a #QmiMessage.
 *
 * Gets the service corresponding to the given #QmiMessage.
 *
 * Returns: a #QmiService.
 */
QmiService
qmi_message_get_service (QmiMessage *self)
{
    g_return_val_if_fail (self != NULL, QMI_SERVICE_UNKNOWN);

    return (QmiService)((struct full_message *)(self->data))->qmux.service;
}

/**
 * qmi_message_get_client_id:
 * @self: a #QmiMessage.
 *
 * Gets the client ID of the message.
 *
 * Returns: the client ID.
 */
guint8
qmi_message_get_client_id (QmiMessage *self)
{
    g_return_val_if_fail (self != NULL, 0);

    return ((struct full_message *)(self->data))->qmux.client;
}

/**
 * qmi_message_get_transaction_id:
 * @self: a #QmiMessage.
 *
 * Gets the transaction ID of the message.
 *
 * Returns: the transaction ID.
 */
guint16
qmi_message_get_transaction_id (QmiMessage *self)
{
    g_return_val_if_fail (self != NULL, 0);

    if (message_is_control (self))
        /* note: only 1 byte for transaction in CTL message */
        return (guint16)((struct full_message *)(self->data))->qmi.control.header.transaction;

    return GUINT16_FROM_LE (((struct full_message *)(self->data))->qmi.service.header.transaction);
}

/**
 * qmi_message_get_message_id:
 * @self: a #QmiMessage.
 *
 * Gets the ID of the message.
 *
 * Returns: the ID.
 */
guint16
qmi_message_get_message_id (QmiMessage *self)
{
    g_return_val_if_fail (self != NULL, 0);

    if (message_is_control (self))
        return GUINT16_FROM_LE (((struct full_message *)(self->data))->qmi.control.header.message);

    return GUINT16_FROM_LE (((struct full_message *)(self->data))->qmi.service.header.message);
}

/**
 * qmi_message_get_length:
 * @self: a #QmiMessage.
 *
 * Gets the length of the raw data corresponding to the given #QmiMessage.
 *
 * Returns: the length of the raw data.
 */
gsize
qmi_message_get_length (QmiMessage *self)
{
    g_return_val_if_fail (self != NULL, 0);

    return self->len;
}

static inline guint16
get_all_tlvs_length (QmiMessage *self)
{
    if (message_is_control (self))
        return GUINT16_FROM_LE (((struct full_message *)(self->data))->qmi.control.header.tlv_length);

    return GUINT16_FROM_LE (((struct full_message *)(self->data))->qmi.service.header.tlv_length);
}

static inline void
set_all_tlvs_length (QmiMessage *self,
                     guint16 length)
{
    if (message_is_control (self))
        ((struct full_message *)(self->data))->qmi.control.header.tlv_length = GUINT16_TO_LE (length);
    else
        ((struct full_message *)(self->data))->qmi.service.header.tlv_length = GUINT16_TO_LE (length);
}

static inline struct tlv *
qmi_tlv (QmiMessage *self)
{
    if (message_is_control (self))
        return ((struct full_message *)(self->data))->qmi.control.tlv;

    return ((struct full_message *)(self->data))->qmi.service.tlv;
}

static inline guint8 *
qmi_end (QmiMessage *self)
{
    return (guint8 *) self->data + self->len;
}

static inline struct tlv *
tlv_next (struct tlv *tlv)
{
    return (struct tlv *)((guint8 *)tlv + sizeof(struct tlv) + GUINT16_FROM_LE (tlv->length));
}

static inline struct tlv *
qmi_tlv_first (QmiMessage *self)
{
    if (get_all_tlvs_length (self))
        return qmi_tlv (self);

    return NULL;
}

static inline struct tlv *
qmi_tlv_next (QmiMessage *self,
              struct tlv *tlv)
{
    struct tlv *end;
    struct tlv *next;

    end = (struct tlv *) qmi_end (self);
    next = tlv_next (tlv);

    return (next < end ? next : NULL);
}

/*
 * Checks the validity of a QMI message.
 *
 * In particular, checks:
 * 1. The message has space for all required headers.
 * 2. The length of the buffer, the qmux length field, and the QMI tlv_length
 *    field are all consistent.
 * 3. The TLVs in the message fit exactly in the payload size.
 *
 * Returns: %TRUE if the message is valid, %FALSE otherwise.
 */
static gboolean
message_check (QmiMessage *self,
               GError **error)
{
    gsize header_length;
    guint8 *end;
    struct tlv *tlv;

    if (((struct full_message *)(self->data))->marker != QMI_MESSAGE_QMUX_MARKER) {
        g_set_error (error,
                     QMI_CORE_ERROR,
                     QMI_CORE_ERROR_INVALID_MESSAGE,
                     "Marker is incorrect");
        return FALSE;
    }

    if (get_qmux_length (self) < sizeof (struct qmux)) {
        g_set_error (error,
                     QMI_CORE_ERROR,
                     QMI_CORE_ERROR_INVALID_MESSAGE,
                     "QMUX length too short for QMUX header (%u < %" G_GSIZE_FORMAT ")",
                     get_qmux_length (self), sizeof (struct qmux));
        return FALSE;
    }

    /*
     * qmux length is one byte shorter than buffer length because qmux
     * length does not include the qmux frame marker.
     */
    if (get_qmux_length (self) != self->len - 1) {
        g_set_error (error,
                     QMI_CORE_ERROR,
                     QMI_CORE_ERROR_INVALID_MESSAGE,
                     "QMUX length and buffer length don't match (%u != %u)",
                     get_qmux_length (self), self->len - 1);
        return FALSE;
    }

    header_length = sizeof (struct qmux) + (message_is_control (self) ?
                                            sizeof (struct control_header) :
                                            sizeof (struct service_header));

    if (get_qmux_length (self) < header_length) {
        g_set_error (error,
                     QMI_CORE_ERROR,
                     QMI_CORE_ERROR_INVALID_MESSAGE,
                     "QMUX length too short for QMI header (%u < %" G_GSIZE_FORMAT ")",
                     get_qmux_length (self), header_length);
        return FALSE;
    }

    if (get_qmux_length (self) - header_length != get_all_tlvs_length (self)) {
        g_set_error (error,
                     QMI_CORE_ERROR,
                     QMI_CORE_ERROR_INVALID_MESSAGE,
                     "QMUX length and QMI TLV lengths don't match (%u - %" G_GSIZE_FORMAT " != %u)",
                     get_qmux_length (self), header_length, get_all_tlvs_length (self));
        return FALSE;
    }

    end = qmi_end (self);
    for (tlv = qmi_tlv (self); tlv < (struct tlv *)end; tlv = tlv_next (tlv)) {
        if (tlv->value > end) {
            g_set_error (error,
                         QMI_CORE_ERROR,
                         QMI_CORE_ERROR_INVALID_MESSAGE,
                         "TLV header runs over buffer (%p > %p)",
                         tlv->value, end);
            return FALSE;
        }
        if (tlv->value + GUINT16_FROM_LE (tlv->length) > end) {
            g_set_error (error,
                         QMI_CORE_ERROR,
                         QMI_CORE_ERROR_INVALID_MESSAGE,
                         "TLV value runs over buffer (%p + %u  > %p)",
                         tlv->value, GUINT16_FROM_LE (tlv->length), end);
            return FALSE;
        }
    }

    /*
     * If this assert triggers, one of the if statements in the loop is wrong.
     * (It shouldn't be reached on malformed QMI messages.)
     */
    g_assert (tlv == (struct tlv *)end);

    return TRUE;
}

/**
 * qmi_message_new:
 * @service: a #QmiService
 * @client_id: client ID of the originating control point.
 * @transaction_id: transaction ID.
 * @message_id: message ID.
 *
 * Create a new #QmiMessage with the specified parameters.
 *
 * Note that @transaction_id must be less than #G_MAXUINT8 if @service is
 * #QMI_SERVICE_CTL.
 *
 * Returns: (transfer full): a newly created #QmiMessage. The returned value should be freed with qmi_message_unref().
 */
QmiMessage *
qmi_message_new (QmiService service,
                 guint8 client_id,
                 guint16 transaction_id,
                 guint16 message_id)
{
    GByteArray *self;
    struct full_message *buffer;
    gsize buffer_len;

    /* Transaction ID in the control service is 8bit only */
    g_return_val_if_fail ((service != QMI_SERVICE_CTL || transaction_id <= G_MAXUINT8),
                          NULL);

    /* Create array with enough size for the QMUX marker, the QMUX header and
     * the QMI header */
    buffer_len = (1 +
                  sizeof (struct qmux) +
                  (service == QMI_SERVICE_CTL ? sizeof (struct control_header) : sizeof (struct service_header)));
    buffer = g_malloc (buffer_len);

    buffer->marker = QMI_MESSAGE_QMUX_MARKER;
    buffer->qmux.flags = 0;
    buffer->qmux.service = service;
    buffer->qmux.client = client_id;

    if (service == QMI_SERVICE_CTL) {
        buffer->qmi.control.header.flags = 0;
        buffer->qmi.control.header.transaction = (guint8)transaction_id;
        buffer->qmi.control.header.message = GUINT16_TO_LE (message_id);
    } else {
        buffer->qmi.service.header.flags = 0;
        buffer->qmi.service.header.transaction = GUINT16_TO_LE (transaction_id);
        buffer->qmi.service.header.message = GUINT16_TO_LE (message_id);
    }

    /* Create the GByteArray */
    self = g_byte_array_new_take ((guint8 *)buffer, buffer_len);

    /* Update length fields. */
    set_qmux_length (self, buffer_len - 1); /* QMUX marker not included in length */
    set_all_tlvs_length (self, 0);

    /* We shouldn't create invalid empty messages */
    g_assert (message_check (self, NULL));

    return (QmiMessage *)self;
}

/**
 * qmi_message_ref:
 * @self: a #QmiMessage.
 *
 * Atomically increments the reference count of @self by one.
 *
 * Returns: (transfer full) the new reference to @self.
 */
QmiMessage *
qmi_message_ref (QmiMessage *self)
{
    g_return_val_if_fail (self != NULL, NULL);

    return (QmiMessage *)g_byte_array_ref (self);
}

/**
 * qmi_message_unref:
 * @self: a #QmiMessage.
 *
 * Atomically decrements the reference count of @self by one.
 * If the reference count drops to 0, @self is completely disposed.
 */
void
qmi_message_unref (QmiMessage *self)
{
    g_return_if_fail (self != NULL);

    g_byte_array_unref (self);
}

/**
 * qmi_message_get_raw:
 * @self: a #QmiMessage.
 * @length: (out): return location for the size of the output buffer.
 * @error: return location for error or %NULL.
 *
 * Gets the raw data buffer of the #QmiMessage.
 *
 * Returns: (transfer none): The raw data buffer, or #NULL if @error is set.
 */
const guint8 *
qmi_message_get_raw (QmiMessage *self,
                     gsize *length,
                     GError **error)
{
    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (length != NULL, NULL);

    *length = self->len;
    return self->data;
}

/**
 * qmi_message_get_raw_tlv:
 * @self: a #QmiMessage.
 * @type: specific ID of the TLV to get.
 * @length: (out): return location for the length of the TLV.
 *
 * Get the raw data buffer of a specific TLV within the #QmiMessage.
 *
 * Returns: (transfer none): The raw data buffer of the TLV, or #NULL if not found.
 */
const guint8 *
qmi_message_get_raw_tlv (QmiMessage *self,
                         guint8 type,
                         guint16 *length)
{
    struct tlv *tlv;

    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (length != NULL, NULL);

    for (tlv = qmi_tlv_first (self); tlv; tlv = qmi_tlv_next (self, tlv)) {
        if (tlv->type == type) {
            *length = GUINT16_FROM_LE (tlv->length);
            return (guint8 *)&(tlv->value[0]);
        }
    }

    return NULL;
}

/**
 * qmi_message_foreach_raw_tlv:
 * @self: a #QmiMessage.
 * @func: the function to call for each TLV.
 * @user_data: user data to pass to the function.
 *
 * Calls the given function for each TLV found within the #QmiMessage.
 */
void
qmi_message_foreach_raw_tlv (QmiMessage *self,
                             QmiMessageForeachRawTlvFn func,
                             gpointer user_data)
{
    struct tlv *tlv;

    g_return_if_fail (self != NULL);
    g_return_if_fail (func != NULL);

    for (tlv = qmi_tlv_first (self); tlv; tlv = qmi_tlv_next (self, tlv)) {
        func (tlv->type,
              (const guint8 *)tlv->value,
              (gsize)(GUINT16_FROM_LE (tlv->length)),
              user_data);
    }
}

/**
 * qmi_message_add_raw_tlv:
 * @self: a #QmiMessage.
 * @type: specific ID of the TLV to add.
 * @raw: raw data buffer with the value of the TLV.
 * @length: length of the raw data buffer.
 * @error: return location for error or %NULL.
 *
 * Creates a new @type TLV with the value given in @raw, and adds it to the #QmiMessage.
 *
 * Returns: %TRUE if the TLV as successfully added, otherwise %FALSE is returned and @error is set.
 */
gboolean
qmi_message_add_raw_tlv (QmiMessage *self,
                         guint8 type,
                         const guint8 *raw,
                         gsize length,
                         GError **error)
{
    size_t tlv_len;
    struct tlv *tlv;

    g_return_val_if_fail (self != NULL, FALSE);
    g_return_val_if_fail (raw != NULL, FALSE);
    g_return_val_if_fail (length > 0, FALSE);

    /* Find length of new TLV */
    tlv_len = sizeof (struct tlv) + length;

    /* Check for overflow of message size. */
    if (get_qmux_length (self) + tlv_len > G_MAXUINT16) {
        g_set_error (error,
                     QMI_CORE_ERROR,
                     QMI_CORE_ERROR_TLV_TOO_LONG,
                     "TLV to add is too long");
        return FALSE;
    }

    /* Resize buffer. */
    g_byte_array_set_size (self, self->len + tlv_len);

    /* Fill in new TLV. */
    tlv = (struct tlv *)(qmi_end (self) - tlv_len);
    tlv->type = type;
    tlv->length = GUINT16_TO_LE (length);
    memcpy (tlv->value, raw, length);

    /* Update length fields. */
    set_qmux_length (self, (guint16)(get_qmux_length (self) + tlv_len));
    set_all_tlvs_length (self, (guint16)(get_all_tlvs_length (self) + tlv_len));

    /* Make sure we didn't break anything. */
    g_assert (message_check (self, error));

    return TRUE;
}

/**
 * qmi_message_new_from_raw:
 * @raw: (inout): raw data buffer.
 * @error: return location for error or %NULL.
 *
 * Create a new #QmiMessage from the given raw data buffer.
 *
 * Whenever a complete QMI message is read, its raw data gets removed from the @raw buffer.
 *
 * Returns: (transfer full): a newly created #QmiMessage, which should be freed with qmi_message_unref(). If @raw doesn't contain a complete QMI message #NULL is returned. If there is a complete QMI message but it appears not to be valid, #NULL is returned and @error is set.
 */
QmiMessage *
qmi_message_new_from_raw (GByteArray *raw,
                          GError **error)
{
    GByteArray *self;
    gsize message_len;

    g_return_val_if_fail (raw != NULL, NULL);

    /* If we didn't even read the QMUX header (comes after the 1-byte marker),
     * leave */
    if (raw->len < (sizeof (struct qmux) + 1))
        return NULL;

    /* We need to have read the length reported by the QMUX header (plus the
     * initial 1-byte marker) */
    message_len = GUINT16_FROM_LE (((struct full_message *)raw->data)->qmux.length);
    if (raw->len < (message_len + 1)) {
        g_printerr ("\ngot '%u' bytes, need '%u' bytes\n",
                    (guint)raw->len,
                    (guint)(message_len + 1));
        return NULL;
    }

    /* Ok, so we should have all the data available already */
    self = g_byte_array_sized_new (message_len + 1);
    g_byte_array_prepend (self, raw->data, message_len + 1);

    /* We got a complete QMI message, remove from input buffer */
    g_byte_array_remove_range (raw, 0, self->len);

    /* Check input message validity as soon as we create the QmiMessage */
    if (!message_check (self, error)) {
        /* Yes, we lose the whole message here */
        qmi_message_unref (self);
        return NULL;
    }

    return (QmiMessage *)self;
}

/**
 * qmi_message_get_tlv_printable:
 * @self: a #QmiMessage.
 * @line_prefix: prefix string to use in each new generated line.
 * @type: type of the TLV.
 * @raw: raw data buffer with the value of the TLV.
 * @raw_length: length of the raw data buffer.
 *
 * Gets a printable string with the contents of the TLV.
 *
 * This method is the most generic one and doesn't try to translate the TLV contents.
 *
 * Returns: (transfer full): a newly allocated string, which should be freed with g_free().
 */
gchar *
qmi_message_get_tlv_printable (QmiMessage *self,
                               const gchar *line_prefix,
                               guint8 type,
                               const guint8 *raw,
                               gsize raw_length)
{
    gchar *printable;
    gchar *value_hex;

    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (line_prefix != NULL, NULL);
    g_return_val_if_fail (raw != NULL, NULL);
    g_return_val_if_fail (raw_length > 0, NULL);

    value_hex = __qmi_utils_str_hex (raw, raw_length, ':');
    printable = g_strdup_printf ("%sTLV:\n"
                                 "%s  type   = 0x%02x\n"
                                 "%s  length = %" G_GSIZE_FORMAT "\n"
                                 "%s  value  = %s\n",
                                 line_prefix,
                                 line_prefix, type,
                                 line_prefix, raw_length,
                                 line_prefix, value_hex);
    g_free (value_hex);
    return printable;
}

static gchar *
get_generic_printable (QmiMessage *self,
                       const gchar *line_prefix)
{
    GString *printable;
    struct tlv *tlv;

    printable = g_string_new ("");

    g_string_append_printf (printable,
                            "%s  message     = (0x%04x)\n",
                            line_prefix, qmi_message_get_message_id (self));

    for (tlv = qmi_tlv_first (self); tlv; tlv = qmi_tlv_next (self, tlv)) {
        gchar *printable_tlv;

        printable_tlv = qmi_message_get_tlv_printable (self,
                                                       line_prefix,
                                                       tlv->type,
                                                       tlv->value,
                                                       tlv->length);
        g_string_append (printable, printable_tlv);
        g_free (printable_tlv);
    }

    return g_string_free (printable, FALSE);
}

/**
 * qmi_message_get_printable:
 * @self: a #QmiMessage.
 * @line_prefix: prefix string to use in each new generated line.
 *
 * Gets a printable string with the contents of the whole QMI message.
 *
 * If known, the printable string will contain translated TLV values as well as the raw
 * data buffer contents.
 *
 * Returns: (transfer full): a newly allocated string, which should be freed with g_free().
 */
gchar *
qmi_message_get_printable (QmiMessage *self,
                           const gchar *line_prefix)
{
    GString *printable;
    gchar *qmi_flags_str;
    gchar *contents;

    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (line_prefix != NULL, NULL);

    if (!line_prefix)
        line_prefix = "";

    printable = g_string_new ("");
    g_string_append_printf (printable,
                            "%sQMUX:\n"
                            "%s  length  = %u\n"
                            "%s  flags   = 0x%02x\n"
                            "%s  service = \"%s\"\n"
                            "%s  client  = %u\n",
                            line_prefix,
                            line_prefix, get_qmux_length (self),
                            line_prefix, get_qmux_flags (self),
                            line_prefix, qmi_service_get_string (qmi_message_get_service (self)),
                            line_prefix, qmi_message_get_client_id (self));

    if (qmi_message_get_service (self) == QMI_SERVICE_CTL)
        qmi_flags_str = qmi_ctl_flag_build_string_from_mask (get_qmi_flags (self));
    else
        qmi_flags_str = qmi_service_flag_build_string_from_mask (get_qmi_flags (self));

    g_string_append_printf (printable,
                            "%sQMI:\n"
                            "%s  flags       = \"%s\"\n"
                            "%s  transaction = %u\n"
                            "%s  tlv_length  = %u\n",
                            line_prefix,
                            line_prefix, qmi_flags_str,
                            line_prefix, qmi_message_get_transaction_id (self),
                            line_prefix, get_all_tlvs_length (self));
    g_free (qmi_flags_str);

    contents = NULL;
    switch (qmi_message_get_service (self)) {
    case QMI_SERVICE_CTL:
        contents = __qmi_message_ctl_get_printable (self, line_prefix);
        break;
    case QMI_SERVICE_DMS:
        contents = __qmi_message_dms_get_printable (self, line_prefix);
        break;
    case QMI_SERVICE_WDS:
        contents = __qmi_message_wds_get_printable (self, line_prefix);
        break;
    case QMI_SERVICE_NAS:
        contents = __qmi_message_nas_get_printable (self, line_prefix);
        break;
    case QMI_SERVICE_WMS:
        contents = __qmi_message_wms_get_printable (self, line_prefix);
        break;
    case QMI_SERVICE_PDS:
        contents = __qmi_message_pds_get_printable (self, line_prefix);
        break;
    default:
        break;
    }

    if (!contents)
        contents = get_generic_printable (self, line_prefix);
    g_string_append (printable, contents);
    g_free (contents);

    return g_string_free (printable, FALSE);
}

/**
 * qmi_message_get_version_introduced:
 * @self: a #QmiMessage.
 * @major: (out) return location for the major version.
 * @minor: (out) return location for the minor version.
 *
 * Gets, if known, the service version in which the given message was first introduced.
 *
 * Returns: %TRUE if @major and @minor are set, %FALSE otherwise.
 */
gboolean
qmi_message_get_version_introduced (QmiMessage *self,
                                    guint *major,
                                    guint *minor)
{
    switch (qmi_message_get_service (self)) {
    case QMI_SERVICE_CTL:
        /* For CTL service, we'll assume the minimum one */
        *major = 0;
        *minor = 0;
        return TRUE;

    case QMI_SERVICE_DMS:
        return __qmi_message_dms_get_version_introduced (self, major, minor);

    case QMI_SERVICE_WDS:
        return __qmi_message_wds_get_version_introduced (self, major, minor);

    case QMI_SERVICE_NAS:
        return __qmi_message_nas_get_version_introduced (self, major, minor);

    case QMI_SERVICE_WMS:
        return __qmi_message_wms_get_version_introduced (self, major, minor);

    case QMI_SERVICE_PDS:
        return __qmi_message_pds_get_version_introduced (self, major, minor);

    default:
        /* For the still unsupported services, cannot do anything */
        return FALSE;
    }
}
