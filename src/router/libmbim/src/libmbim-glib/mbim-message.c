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

#include <glib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <endian.h>

#include "mbim-message.h"
#include "mbim-message-private.h"
#include "mbim-error-types.h"
#include "mbim-enum-types.h"

/**
 * SECTION:mbim-message
 * @title: MbimMessage
 * @short_description: Generic MBIM message handling routines
 *
 * #MbimMessage is a generic type representing a MBIM message of any kind
 * (request, response, indication).
 **/

/*****************************************************************************/

static void
bytearray_apply_padding (GByteArray *buffer,
                         guint32    *len)
{
    static const guint8 padding = 0;

    g_assert (buffer);
    g_assert (len);

    /* Apply padding to the requested length until multiple of 4 */
    while (*len % 4 != 0) {
        g_byte_array_append (buffer, &padding, 1);
        (*len)++;
    }
}

/*****************************************************************************/

GType
mbim_message_get_type (void)
{
    static volatile gsize g_define_type_id__volatile = 0;

    if (g_once_init_enter (&g_define_type_id__volatile)) {
        GType g_define_type_id =
            g_boxed_type_register_static (g_intern_static_string ("MbimMessage"),
                                          (GBoxedCopyFunc) mbim_message_ref,
                                          (GBoxedFreeFunc) mbim_message_unref);

        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);
    }

    return g_define_type_id__volatile;
}

/*****************************************************************************/

GByteArray *
_mbim_message_allocate (MbimMessageType message_type,
                        guint32         transaction_id,
                        guint32         additional_size)
{
    GByteArray *self;
    guint32 len;

    /* Compute size of the basic empty message and allocate heap for it */
    len = sizeof (struct header) + additional_size;
    self = g_byte_array_sized_new (len);
    g_byte_array_set_size (self, len);

    /* Set MBIM header */
    ((struct header *)(self->data))->type           = GUINT32_TO_LE (message_type);
    ((struct header *)(self->data))->length         = GUINT32_TO_LE (len);
    ((struct header *)(self->data))->transaction_id = GUINT32_TO_LE (transaction_id);

    return self;
}

static guint32
_mbim_message_get_information_buffer_offset (const MbimMessage *self)
{
    g_return_val_if_fail (MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_COMMAND ||
                          MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_COMMAND_DONE ||
                          MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_INDICATE_STATUS, 0);

    switch (MBIM_MESSAGE_GET_MESSAGE_TYPE (self)) {
    case MBIM_MESSAGE_TYPE_COMMAND:
        return (sizeof (struct header) +
                G_STRUCT_OFFSET (struct command_message, buffer));

    case MBIM_MESSAGE_TYPE_COMMAND_DONE:
        return (sizeof (struct header) +
                G_STRUCT_OFFSET (struct command_done_message, buffer));
        break;

    case MBIM_MESSAGE_TYPE_INDICATE_STATUS:
        return (sizeof (struct header) +
                G_STRUCT_OFFSET (struct indicate_status_message, buffer));
        break;

    default:
        g_assert_not_reached ();
        return 0;
    }
}

guint32
_mbim_message_read_guint32 (const MbimMessage *self,
                            guint32            relative_offset)
{
    guint32 information_buffer_offset;

    information_buffer_offset = _mbim_message_get_information_buffer_offset (self);
    return GUINT32_FROM_LE (G_STRUCT_MEMBER (
                                guint32,
                                self->data,
                                (information_buffer_offset + relative_offset)));
}

guint32 *
_mbim_message_read_guint32_array (const MbimMessage *self,
                                  guint32            array_size,
                                  guint32            relative_offset_array_start)
{
    guint i;
    guint32 *out;
    guint32 information_buffer_offset;

    if (!array_size)
        return NULL;

    information_buffer_offset = _mbim_message_get_information_buffer_offset (self);

    out = g_new (guint32, array_size + 1);
    for (i = 0; i < array_size; i++) {
        out[i] = GUINT32_FROM_LE (G_STRUCT_MEMBER (
                                      guint32,
                                      self->data,
                                      (information_buffer_offset +
                                       relative_offset_array_start +
                                       (4 * i))));
    }
    out[array_size] = 0;

    return out;
}

guint64
_mbim_message_read_guint64 (const MbimMessage *self,
                            guint64            relative_offset)
{
    guint64 information_buffer_offset;

    information_buffer_offset = _mbim_message_get_information_buffer_offset (self);
    return GUINT64_FROM_LE (G_STRUCT_MEMBER (
                                guint64,
                                self->data,
                                (information_buffer_offset + relative_offset)));
}

gchar *
_mbim_message_read_string (const MbimMessage *self,
                           guint32            struct_start_offset,
                           guint32            relative_offset)
{
    guint32 offset;
    guint32 size;
    gchar *str;
    GError *error = NULL;
    guint32 information_buffer_offset;
    gunichar2 *utf16d = NULL;
    const gunichar2 *utf16 = NULL;

    information_buffer_offset = _mbim_message_get_information_buffer_offset (self);

    offset = GUINT32_FROM_LE (G_STRUCT_MEMBER (
                                  guint32,
                                  self->data,
                                  (information_buffer_offset + relative_offset)));
    size   = GUINT32_FROM_LE (G_STRUCT_MEMBER (
                                  guint32,
                                  self->data,
                                  (information_buffer_offset + relative_offset + 4)));
    if (!size)
        return NULL;

    utf16 = (const gunichar2 *) G_STRUCT_MEMBER_P (self->data, (information_buffer_offset + struct_start_offset + offset));

    /* For BE systems, convert from LE to BE */
    if (G_BYTE_ORDER == G_BIG_ENDIAN) {
        guint i;

        utf16d = (gunichar2 *) g_malloc (size);
        for (i = 0; i < (size / 2); i++)
            utf16d[i] = GUINT16_FROM_LE (utf16[i]);
    }

    str = g_utf16_to_utf8 (utf16d ? utf16d : utf16,
                           size / 2,
                           NULL,
                           NULL,
                           &error);
    if (error) {
        g_warning ("Error converting string: %s", error->message);
        g_error_free (error);
    }

    g_free (utf16d);

    return str;
}

gchar **
_mbim_message_read_string_array (const MbimMessage *self,
                                 guint32            array_size,
                                 guint32            struct_start_offset,
                                 guint32            relative_offset_array_start)
{
    gchar **array;
    guint32 offset;
    guint32 i;

    if (!array_size)
        return NULL;

    array = g_new (gchar *, array_size + 1);
    for (i = 0, offset = relative_offset_array_start;
         i < array_size;
         offset += 8, i++) {
        /* Read next string in the OL pair list */
        array[i] = _mbim_message_read_string (self, struct_start_offset, offset);
    }
    array[i] = NULL;

    return array;
}

/*
 * Byte arrays may be given in very different ways:
 *  - (a) Offset + Length pair in static buffer, data in variable buffer.
 *  - (b) Just length in static buffer, data just afterwards.
 *  - (c) Just offset in static buffer, length given in another variable, data in variable buffer.
 *  - (d) Fixed-sized array directly in the static buffer.
 *  - (e) Unsized array directly in the variable buffer, length is assumed until end of message.
 */
const guint8 *
_mbim_message_read_byte_array (const MbimMessage *self,
                               guint32            struct_start_offset,
                               guint32            relative_offset,
                               gboolean           has_offset,
                               gboolean           has_length,
                               guint32           *array_size)
{
    guint32 information_buffer_offset;

    information_buffer_offset = _mbim_message_get_information_buffer_offset (self);

    /* (a) Offset + Length pair in static buffer, data in variable buffer. */
    if (has_offset && has_length) {
        guint32 offset;

        g_assert (array_size != NULL);

        offset = GUINT32_FROM_LE (G_STRUCT_MEMBER (
                                      guint32,
                                      self->data,
                                      (information_buffer_offset + relative_offset)));
        *array_size = GUINT32_FROM_LE (G_STRUCT_MEMBER (
                                           guint32,
                                           self->data,
                                           (information_buffer_offset + relative_offset + 4)));
        return (const guint8 *) G_STRUCT_MEMBER_P (self->data,
                                                   (information_buffer_offset + struct_start_offset + offset));
    }

    /* (b) Just length in static buffer, data just afterwards. */
    if (!has_offset && has_length) {
        g_assert (array_size != NULL);

        *array_size = GUINT32_FROM_LE (G_STRUCT_MEMBER (
                                           guint32,
                                           self->data,
                                           (information_buffer_offset + relative_offset)));
        return (const guint8 *) G_STRUCT_MEMBER_P (self->data,
                                                   (information_buffer_offset + relative_offset + 4));
    }

    /* (c) Just offset in static buffer, length given in another variable, data in variable buffer. */
    if (has_offset && !has_length) {
        guint32 offset;

        g_assert (array_size == NULL);

        offset = GUINT32_FROM_LE (G_STRUCT_MEMBER (
                                      guint32,
                                      self->data,
                                      (information_buffer_offset + relative_offset)));
        return (const guint8 *) G_STRUCT_MEMBER_P (self->data,
                                                   (information_buffer_offset + struct_start_offset + offset));
    }

    /* (d) Fixed-sized array directly in the static buffer.
     * (e) Unsized array directly in the variable buffer, length is assumed until end of message. */
    if (!has_offset && !has_length) {
        /* If array size is requested, it's case (e) */
        if (array_size)
            *array_size = self->len - (information_buffer_offset + relative_offset);

        return (const guint8 *) G_STRUCT_MEMBER_P (self->data,
                                                   (information_buffer_offset + relative_offset));
    }

    g_assert_not_reached ();
}

const MbimUuid *
_mbim_message_read_uuid (const MbimMessage *self,
                         guint32            relative_offset)
{
    guint32 information_buffer_offset;

    information_buffer_offset = _mbim_message_get_information_buffer_offset (self);

    return (const MbimUuid *) G_STRUCT_MEMBER_P (self->data,
                                                 (information_buffer_offset + relative_offset));
}

const MbimIPv4 *
_mbim_message_read_ipv4 (const MbimMessage *self,
                         guint32            relative_offset,
                         gboolean           ref)
{
    guint32 information_buffer_offset;
    guint32 offset;

    information_buffer_offset = _mbim_message_get_information_buffer_offset (self);

    if (ref) {
        offset = GUINT32_FROM_LE (G_STRUCT_MEMBER (guint32,
                                                   self->data,
                                                   (information_buffer_offset + relative_offset)));
        if (!offset)
            return NULL;
    } else
        offset = relative_offset;

    return (const MbimIPv4 *) G_STRUCT_MEMBER_P (self->data,
                                                 (information_buffer_offset + offset));
}

MbimIPv4 *
_mbim_message_read_ipv4_array (const MbimMessage *self,
                               guint32            array_size,
                               guint32            relative_offset_array_start)
{
    MbimIPv4 *array;
    guint32 offset;
    guint32 i;
    guint32 information_buffer_offset;

    if (!array_size)
        return NULL;

    information_buffer_offset = _mbim_message_get_information_buffer_offset (self);

    array = g_new (MbimIPv4, array_size);
    offset = GUINT32_FROM_LE (G_STRUCT_MEMBER (
                                  guint32,
                                  self->data,
                                  (information_buffer_offset + relative_offset_array_start)));

    for (i = 0; i < array_size; i++, offset += 4) {
        memcpy (&array[i],
                G_STRUCT_MEMBER_P (self->data,
                                   (information_buffer_offset + offset)),
                4);
    }

    return array;
}

const MbimIPv6 *
_mbim_message_read_ipv6 (const MbimMessage *self,
                         guint32            relative_offset,
                         gboolean           ref)
{
    guint32 information_buffer_offset;
    guint32 offset;

    information_buffer_offset = _mbim_message_get_information_buffer_offset (self);

    if (ref) {
        offset = GUINT32_FROM_LE (G_STRUCT_MEMBER (guint32,
                                                   self->data,
                                                   (information_buffer_offset + relative_offset)));
        if (!offset)
            return NULL;
    } else
        offset = relative_offset;

    return (const MbimIPv6 *) G_STRUCT_MEMBER_P (self->data,
                                                 (information_buffer_offset + offset));
}

MbimIPv6 *
_mbim_message_read_ipv6_array (const MbimMessage *self,
                               guint32            array_size,
                               guint32            relative_offset_array_start)
{
    MbimIPv6 *array;
    guint32 offset;
    guint32 i;
    guint32 information_buffer_offset;

    if (!array_size)
        return NULL;

    information_buffer_offset = _mbim_message_get_information_buffer_offset (self);

    array = g_new (MbimIPv6, array_size);
    offset = GUINT32_FROM_LE (G_STRUCT_MEMBER (
                                  guint32,
                                  self->data,
                                  (information_buffer_offset + relative_offset_array_start)));
    for (i = 0; i < array_size; i++, offset += 16) {
        memcpy (&array[i],
                G_STRUCT_MEMBER_P (self->data,
                                   (information_buffer_offset + offset)),
                16);
    }

    return array;
}

/*****************************************************************************/
/* Struct builder interface
 *
 * Types like structs consist of a fixed sized prefix plus a variable length
 * data buffer. Items of variable size are usually given as an offset (with
 * respect to the start of the struct) plus a size field. */

MbimStructBuilder *
_mbim_struct_builder_new (void)
{
    MbimStructBuilder *builder;

    builder = g_slice_new (MbimStructBuilder);
    builder->fixed_buffer = g_byte_array_new ();
    builder->variable_buffer = g_byte_array_new ();
    builder->offsets = g_array_new (FALSE, FALSE, sizeof (guint32));
    return builder;
}

GByteArray *
_mbim_struct_builder_complete (MbimStructBuilder *builder)
{
    GByteArray *out;
    guint i;

    /* Update offsets with the length of the information buffer, and store them
     * in LE. */
    for (i = 0; i < builder->offsets->len; i++) {
        guint32 offset_offset;
        guint32 *offset_value;

        offset_offset = g_array_index (builder->offsets, guint32, i);
        offset_value = (guint32 *) &builder->fixed_buffer->data[offset_offset];
        *offset_value = GUINT32_TO_LE (*offset_value + builder->fixed_buffer->len);
    }

    /* Merge both buffers */
    g_byte_array_append (builder->fixed_buffer,
                         (const guint8 *)builder->variable_buffer->data,
                         (guint32)builder->variable_buffer->len);

    /* Steal the buffer to return */
    out = builder->fixed_buffer;

    /* Dispose the builder */
    g_array_unref (builder->offsets);
    g_byte_array_unref (builder->variable_buffer);
    g_slice_free (MbimStructBuilder, builder);

    return out;
}

/*
 * Byte arrays may be given in very different ways:
 *  - (a) Offset + Length pair in static buffer, data in variable buffer.
 *  - (b) Just length in static buffer, data just afterwards.
 *  - (c) Just offset in static buffer, length given in another variable, data in variable buffer.
 *  - (d) Fixed-sized array directly in the static buffer.
 *  - (e) Unsized array directly in the variable buffer, length is assumed until end of message.
 */
void
_mbim_struct_builder_append_byte_array (MbimStructBuilder *builder,
                                        gboolean           with_offset,
                                        gboolean           with_length,
                                        gboolean           pad_buffer,
                                        const guint8      *buffer,
                                        guint32            buffer_len)
{
    /*
     * (d) Fixed-sized array directly in the static buffer.
     * (e) Unsized array directly in the variable buffer (here end of static buffer is also beginning of variable)
     */
    if (!with_offset && !with_length) {
        g_byte_array_append (builder->fixed_buffer, buffer, buffer_len);
        if (pad_buffer)
            bytearray_apply_padding (builder->fixed_buffer, &buffer_len);
        return;
    }

    /* (a) Offset + Length pair in static buffer, data in variable buffer.
     * This case is the sum of cases b+c */

    /* (c) Just offset in static buffer, length given in another variable, data in variable buffer. */
    if (with_offset) {
        guint32 offset;

        /* If string length is greater than 0, add the offset to fix, otherwise set
         * the offset to 0 and don't configure the update */
        if (buffer_len == 0) {
            offset = 0;
            g_byte_array_append (builder->fixed_buffer, (guint8 *)&offset, sizeof (offset));
        } else {
            guint32 offset_offset;

            /* Offset of the offset */
            offset_offset = builder->fixed_buffer->len;

            /* Length *not* in LE yet */
            offset = builder->variable_buffer->len;
            /* Add the offset value */
            g_byte_array_append (builder->fixed_buffer, (guint8 *)&offset, sizeof (offset));
            /* Configure the value to get updated */
            g_array_append_val (builder->offsets, offset_offset);
        }
    }

    /* (b) Just length in static buffer, data just afterwards. */
    if (with_length) {
        guint32 length;

        /* Add the length value */
        length = GUINT32_TO_LE (buffer_len);
        g_byte_array_append (builder->fixed_buffer, (guint8 *)&length, sizeof (length));
    }

    /* And finally, the bytearray itself to the variable buffer */
    if (buffer_len) {
        g_byte_array_append (builder->variable_buffer, (const guint8 *)buffer, (guint)buffer_len);

        /* Note: adding zero padding causes trouble for QMI service */
        if (pad_buffer)
            bytearray_apply_padding (builder->variable_buffer, &buffer_len);
    }
}

void
_mbim_struct_builder_append_uuid (MbimStructBuilder *builder,
                                  const MbimUuid    *value)
{
    static const MbimUuid uuid_invalid = {
        .a = { 0x00, 0x00, 0x00, 0x00 },
        .b = { 0x00, 0x00 },
        .c = { 0x00, 0x00 },
        .d = { 0x00, 0x00 },
        .e = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
    };

    /* uuids are added in the static buffer only */
    g_byte_array_append (builder->fixed_buffer,
                         value ? (guint8 *)value : (guint8 *)&uuid_invalid,
                         sizeof (MbimUuid));
}

void
_mbim_struct_builder_append_guint32 (MbimStructBuilder *builder,
                                     guint32            value)
{
    guint32 tmp;

    /* guint32 values are added in the static buffer only */
    tmp = GUINT32_TO_LE (value);
    g_byte_array_append (builder->fixed_buffer, (guint8 *)&tmp, sizeof (tmp));
}

void
_mbim_struct_builder_append_guint32_array (MbimStructBuilder *builder,
                                           const guint32     *values,
                                           guint32            n_values)
{
    guint i;

    /* guint32 array added directly in the static buffer */
    for (i = 0; i < n_values; i++)
        _mbim_struct_builder_append_guint32 (builder, values[i]);
}

void
_mbim_struct_builder_append_guint64 (MbimStructBuilder *builder,
                                     guint64            value)
{
    guint64 tmp;

    /* guint64 values are added in the static buffer only */
    tmp = GUINT64_TO_LE (value);
    g_byte_array_append (builder->fixed_buffer, (guint8 *)&tmp, sizeof (tmp));
}

void
_mbim_struct_builder_append_guint64_array (MbimStructBuilder *builder,
                                           const guint64     *values,
                                           guint32            n_values)
{
    guint i;

    /* guint64 array added directly in the static buffer */
    for (i = 0; i < n_values; i++)
        _mbim_struct_builder_append_guint64 (builder, values[i]);
}

void
_mbim_struct_builder_append_string (MbimStructBuilder *builder,
                                    const gchar       *value)
{
    guint32 offset;
    guint32 length;
    gunichar2 *utf16 = NULL;
    guint32 utf16_bytes = 0;
    GError *error = NULL;

    /* A string consists of Offset+Size in the static buffer, plus the
     * string itself in the variable buffer */

    /* Convert the string from UTF-8 to UTF-16HE */
    if (value && value[0]) {
        glong items_written = 0;

        utf16 = g_utf8_to_utf16 (value,
                                 -1,
                                 NULL, /* bytes */
                                 &items_written, /* gunichar2 */
                                 &error);
        if (!utf16) {
            g_warning ("Error converting string: %s", error->message);
            g_error_free (error);
            return;
        }

        utf16_bytes = items_written * 2;
    }

    /* If string length is greater than 0, add the offset to fix, otherwise set
     * the offset to 0 and don't configure the update */
    if (utf16_bytes == 0) {
        offset = 0;
        g_byte_array_append (builder->fixed_buffer, (guint8 *)&offset, sizeof (offset));
    } else {
        guint32 offset_offset;

        /* Offset of the offset */
        offset_offset = builder->fixed_buffer->len;

        /* Length *not* in LE yet */
        offset = builder->variable_buffer->len;
        /* Add the offset value */
        g_byte_array_append (builder->fixed_buffer, (guint8 *)&offset, sizeof (offset));
        /* Configure the value to get updated */
        g_array_append_val (builder->offsets, offset_offset);
    }

    /* Add the length value */
    length = GUINT32_TO_LE (utf16_bytes);
    g_byte_array_append (builder->fixed_buffer, (guint8 *)&length, sizeof (length));

    /* And finally, the string itself to the variable buffer */
    if (utf16_bytes) {
        /* For BE systems, convert from BE to LE */
        if (G_BYTE_ORDER == G_BIG_ENDIAN) {
            guint i;

            for (i = 0; i < (utf16_bytes / 2); i++)
                utf16[i] = GUINT16_TO_LE (utf16[i]);
        }
        g_byte_array_append (builder->variable_buffer, (const guint8 *)utf16, (guint)utf16_bytes);
        bytearray_apply_padding (builder->variable_buffer, &utf16_bytes);
    }
    g_free (utf16);
}

void
_mbim_struct_builder_append_string_array (MbimStructBuilder  *builder,
                                          const gchar *const *values,
                                          guint32             n_values)
{
    /* TODO */
    g_assert_not_reached ();
}

void
_mbim_struct_builder_append_ipv4 (MbimStructBuilder *builder,
                                  const MbimIPv4    *value,
                                  gboolean           ref)
{
    if (ref)
        _mbim_struct_builder_append_ipv4_array (builder, value, value ? 1 : 0);
    else
        g_byte_array_append (builder->fixed_buffer, (guint8 *)value, sizeof (MbimIPv4));
}

void
_mbim_struct_builder_append_ipv4_array (MbimStructBuilder *builder,
                                        const MbimIPv4    *values,
                                        guint32            n_values)
{
    guint32 offset;

    if (!n_values) {
        offset = 0;
        g_byte_array_append (builder->fixed_buffer, (guint8 *)&offset, sizeof (offset));
    } else {
        guint32 offset_offset;

        /* Offset of the offset */
        offset_offset = builder->fixed_buffer->len;

        /* Length *not* in LE yet */
        offset = builder->variable_buffer->len;
        /* Add the offset value */
        g_byte_array_append (builder->fixed_buffer, (guint8 *)&offset, sizeof (offset));
        /* Configure the value to get updated */
        g_array_append_val (builder->offsets, offset_offset);

        /* NOTE: length of the array must be given in a separate variable */

        /* And finally, the array of IPs itself to the variable buffer */
        g_byte_array_append (builder->variable_buffer, (guint8 *)values, n_values * sizeof (MbimIPv4));
    }
}

void
_mbim_struct_builder_append_ipv6 (MbimStructBuilder *builder,
                                  const MbimIPv6    *value,
                                  gboolean           ref)
{
    if (ref)
        _mbim_struct_builder_append_ipv6_array (builder, value, value ? 1 : 0);
    else
        g_byte_array_append (builder->fixed_buffer, (guint8 *)value, sizeof (MbimIPv6));
}

void
_mbim_struct_builder_append_ipv6_array (MbimStructBuilder *builder,
                                        const MbimIPv6    *values,
                                        guint32            n_values)
{
    guint32 offset;

    if (!n_values) {
        offset = 0;
        g_byte_array_append (builder->fixed_buffer, (guint8 *)&offset, sizeof (offset));
    } else {
        guint32 offset_offset;

        /* Offset of the offset */
        offset_offset = builder->fixed_buffer->len;

        /* Length *not* in LE yet */
        offset = builder->variable_buffer->len;
        /* Add the offset value */
        g_byte_array_append (builder->fixed_buffer, (guint8 *)&offset, sizeof (offset));
        /* Configure the value to get updated */
        g_array_append_val (builder->offsets, offset_offset);

        /* NOTE: length of the array must be given in a separate variable */

        /* And finally, the array of IPs itself to the variable buffer */
        g_byte_array_append (builder->variable_buffer, (guint8 *)values, n_values * sizeof (MbimIPv6));
    }
}

/*****************************************************************************/
/* Command message builder interface */

MbimMessageCommandBuilder *
_mbim_message_command_builder_new (guint32                transaction_id,
                                   MbimService            service,
                                   guint32                cid,
                                   MbimMessageCommandType command_type)
{
    MbimMessageCommandBuilder *builder;

    builder = g_slice_new (MbimMessageCommandBuilder);
    builder->message = mbim_message_command_new (transaction_id, service, cid, command_type);
    builder->contents_builder = _mbim_struct_builder_new ();
    return builder;
}

MbimMessage *
_mbim_message_command_builder_complete (MbimMessageCommandBuilder *builder)
{
    MbimMessage *message;
    GByteArray *contents;

    /* Complete contents, which disposes the builder itself */
    contents = _mbim_struct_builder_complete (builder->contents_builder);

    /* Merge both buffers */
    mbim_message_command_append (builder->message,
                                 (const guint8 *)contents->data,
                                 (guint32)contents->len);
    g_byte_array_unref (contents);

    /* Steal the message to return */
    message = builder->message;

    /* Dispose the remaining stuff from the message builder */
    g_slice_free (MbimMessageCommandBuilder, builder);

    return message;
}

void
_mbim_message_command_builder_append_byte_array (MbimMessageCommandBuilder *builder,
                                                 gboolean                   with_offset,
                                                 gboolean                   with_length,
                                                 gboolean                   pad_buffer,
                                                 const guint8              *buffer,
                                                 guint32                    buffer_len)
{
    _mbim_struct_builder_append_byte_array (builder->contents_builder, with_offset, with_length, pad_buffer, buffer, buffer_len);
}

void
_mbim_message_command_builder_append_uuid (MbimMessageCommandBuilder *builder,
                                           const MbimUuid            *value)
{
    _mbim_struct_builder_append_uuid (builder->contents_builder, value);
}

void
_mbim_message_command_builder_append_guint32 (MbimMessageCommandBuilder *builder,
                                              guint32                    value)
{
    _mbim_struct_builder_append_guint32 (builder->contents_builder, value);
}

void
_mbim_message_command_builder_append_guint32_array (MbimMessageCommandBuilder *builder,
                                                    const guint32             *values,
                                                    guint32                    n_values)
{
    _mbim_struct_builder_append_guint32_array (builder->contents_builder, values, n_values);
}

void
_mbim_message_command_builder_append_guint64 (MbimMessageCommandBuilder *builder,
                                              guint64                    value)
{
    _mbim_struct_builder_append_guint64 (builder->contents_builder, value);
}

void
_mbim_message_command_builder_append_guint64_array (MbimMessageCommandBuilder *builder,
                                                    const guint64             *values,
                                                    guint32                    n_values)
{
    _mbim_struct_builder_append_guint64_array (builder->contents_builder, values, n_values);
}

void
_mbim_message_command_builder_append_string (MbimMessageCommandBuilder *builder,
                                             const gchar               *value)
{
    _mbim_struct_builder_append_string (builder->contents_builder, value);
}

void
_mbim_message_command_builder_append_string_array (MbimMessageCommandBuilder *builder,
                                                   const gchar *const        *values,
                                                   guint32                    n_values)
{
    _mbim_struct_builder_append_string_array (builder->contents_builder, values, n_values);
}

void
_mbim_message_command_builder_append_ipv4 (MbimMessageCommandBuilder *builder,
                                           const MbimIPv4            *value,
                                           gboolean                   ref)
{
    _mbim_struct_builder_append_ipv4 (builder->contents_builder, value, ref);
}

void
_mbim_message_command_builder_append_ipv4_array (MbimMessageCommandBuilder *builder,
                                                 const MbimIPv4            *values,
                                                 guint32                    n_values)
{
    _mbim_struct_builder_append_ipv4_array (builder->contents_builder, values, n_values);
}

void
_mbim_message_command_builder_append_ipv6 (MbimMessageCommandBuilder *builder,
                                           const MbimIPv6            *value,
                                           gboolean                   ref)
{
    _mbim_struct_builder_append_ipv6 (builder->contents_builder, value, ref);
}

void
_mbim_message_command_builder_append_ipv6_array (MbimMessageCommandBuilder *builder,
                                                 const MbimIPv6            *values,
                                                 guint32                    n_values)
{
    _mbim_struct_builder_append_ipv6_array (builder->contents_builder, values, n_values);
}

/*****************************************************************************/
/* Generic message interface */

/**
 * mbim_message_ref:
 * @self: a #MbimMessage.
 *
 * Atomically increments the reference count of @self by one.
 *
 * Returns: (transfer full) the new reference to @self.
 */
MbimMessage *
mbim_message_ref (MbimMessage *self)
{
    g_return_val_if_fail (self != NULL, NULL);

    return (MbimMessage *) g_byte_array_ref ((GByteArray *)self);
}

/**
 * mbim_message_unref:
 * @self: a #MbimMessage.
 *
 * Atomically decrements the reference count of @self by one.
 * If the reference count drops to 0, @self is completely disposed.
 */
void
mbim_message_unref (MbimMessage *self)
{
    g_return_if_fail (self != NULL);

    g_byte_array_unref ((GByteArray *)self);
}

/**
 * mbim_message_get_message_type:
 * @self: a #MbimMessage.
 *
 * Gets the message type.
 *
 * Returns: a #MbimMessageType.
 */
MbimMessageType
mbim_message_get_message_type (const MbimMessage *self)
{
    g_return_val_if_fail (self != NULL, MBIM_MESSAGE_TYPE_INVALID);

    return MBIM_MESSAGE_GET_MESSAGE_TYPE (self);
}

/**
 * mbim_message_get_message_length:
 * @self: a #MbimMessage.
 *
 * Gets the whole message length.
 *
 * Returns: the length of the message.
 */
guint32
mbim_message_get_message_length (const MbimMessage *self)
{
    g_return_val_if_fail (self != NULL, 0);

    return MBIM_MESSAGE_GET_MESSAGE_LENGTH (self);
}

/**
 * mbim_message_get_transaction_id:
 * @self: a #MbimMessage.
 *
 * Gets the transaction ID of the message.
 *
 * Returns: the transaction ID.
 */
guint32
mbim_message_get_transaction_id (const MbimMessage *self)
{
    g_return_val_if_fail (self != NULL, 0);

    return MBIM_MESSAGE_GET_TRANSACTION_ID (self);
}

/**
 * mbim_message_set_transaction_id:
 * @self: a #MbimMessage.
 * @transaction_id: the transaction id.
 *
 * Sets the transaction ID of the message.
 */
void
mbim_message_set_transaction_id (MbimMessage *self,
                                 guint32      transaction_id)
{
    g_return_if_fail (self != NULL);

    ((struct header *)(self->data))->transaction_id = GUINT32_TO_LE (transaction_id);
}

/**
 * mbim_message_new:
 * @data: contents of the message.
 * @data_length: length of the message.
 *
 * Create a #MbimMessage with the given contents.
 *
 * Returns: (transfer full): a newly created #MbimMessage, which should be freed with mbim_message_unref().
 */
MbimMessage *
mbim_message_new (const guint8 *data,
                  guint32       data_length)
{
    GByteArray *out;

    /* Create output MbimMessage */
    out = g_byte_array_sized_new (data_length);
    g_byte_array_append (out, data, data_length);

    return (MbimMessage *)out;
}

/**
 * mbim_message_dup:
 * @self: a #MbimMessage to duplicate.
 *
 * Create a #MbimMessage with the same contents as @self.
 *
 * Returns: (transfer full): a newly created #MbimMessage, which should be freed with mbim_message_unref().
 */
MbimMessage *
mbim_message_dup (const MbimMessage *self)
{
    g_return_val_if_fail (self != NULL, NULL);

    return mbim_message_new (((GByteArray *)self)->data,
                             MBIM_MESSAGE_GET_MESSAGE_LENGTH (self));
}

/**
 * mbim_message_get_raw:
 * @self: a #MbimMessage.
 * @length: (out): return location for the size of the output buffer.
 * @error: return location for error or %NULL.
 *
 * Gets the whole raw data buffer of the #MbimMessage.
 *
 * Returns: (transfer none): The raw data buffer, or #NULL if @error is set.
 */
const guint8 *
mbim_message_get_raw (const MbimMessage  *self,
                      guint32            *length,
                      GError            **error)
{
    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (length != NULL, NULL);

    if (!self->data || !self->len) {
        g_set_error_literal (error,
                             MBIM_CORE_ERROR,
                             MBIM_CORE_ERROR_FAILED,
                             "Message is empty");
        return NULL;
    }

    *length = (guint32) self->len;
    return self->data;
}

/**
 * mbim_message_get_printable:
 * @self: a #MbimMessage.
 * @line_prefix: prefix string to use in each new generated line.
 * @headers_only: %TRUE if only basic headers should be printed.
 *
 * Gets a printable string with the contents of the whole MBIM message.
 *
 * Returns: (transfer full): a newly allocated string, which should be freed with g_free().
 */
gchar *
mbim_message_get_printable (const MbimMessage *self,
                            const gchar       *line_prefix,
                            gboolean           headers_only)
{
    GString *printable;

    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (line_prefix != NULL, NULL);

    if (!line_prefix)
        line_prefix = "";

    printable = g_string_new ("");
    g_string_append_printf (printable,
                            "%sHeader:\n"
                            "%s  length      = %u\n"
                            "%s  type        = %s (0x%08x)\n"
                            "%s  transaction = %u\n",
                            line_prefix,
                            line_prefix, MBIM_MESSAGE_GET_MESSAGE_LENGTH (self),
                            line_prefix, mbim_message_type_get_string (MBIM_MESSAGE_GET_MESSAGE_TYPE (self)), MBIM_MESSAGE_GET_MESSAGE_TYPE (self),
                            line_prefix, MBIM_MESSAGE_GET_TRANSACTION_ID (self));

    switch (MBIM_MESSAGE_GET_MESSAGE_TYPE (self)) {
    case MBIM_MESSAGE_TYPE_INVALID:
        g_warn_if_reached ();
        break;

    case MBIM_MESSAGE_TYPE_OPEN:
        if (!headers_only)
            g_string_append_printf (printable,
                                    "%sContents:\n"
                                    "%s  max_control_transfer = %u\n",
                                    line_prefix,
                                    line_prefix, mbim_message_open_get_max_control_transfer (self));
        break;

    case MBIM_MESSAGE_TYPE_CLOSE:
        break;

    case MBIM_MESSAGE_TYPE_OPEN_DONE:
        if (!headers_only) {
            MbimStatusError status;

            status = mbim_message_open_done_get_status_code (self);
            g_string_append_printf (printable,
                                    "%sContents:\n"
                                    "%s  status error = '%s' (0x%08x)\n",
                                    line_prefix,
                                    line_prefix, mbim_status_error_get_string (status), status);
        }
        break;

    case MBIM_MESSAGE_TYPE_CLOSE_DONE:
        if (!headers_only) {
            MbimStatusError status;

            status = mbim_message_close_done_get_status_code (self);
            g_string_append_printf (printable,
                                    "%sContents:\n"
                                    "%s  status error = '%s' (0x%08x)\n",
                                    line_prefix,
                                    line_prefix, mbim_status_error_get_string (status), status);
        }
        break;

    case MBIM_MESSAGE_TYPE_HOST_ERROR:
    case MBIM_MESSAGE_TYPE_FUNCTION_ERROR:
        if (!headers_only) {
            MbimProtocolError error;

            error = mbim_message_error_get_error_status_code (self);
            g_string_append_printf (printable,
                                    "%sContents:\n"
                                    "%s  error = '%s' (0x%08x)\n",
                                    line_prefix,
                                    line_prefix, mbim_protocol_error_get_string (error), error);
        }
        break;

    case MBIM_MESSAGE_TYPE_COMMAND:
        g_string_append_printf (printable,
                                "%sFragment header:\n"
                                "%s  total   = %u\n"
                                "%s  current = %u\n",
                                line_prefix,
                                line_prefix, _mbim_message_fragment_get_total (self),
                                line_prefix, _mbim_message_fragment_get_current (self));
        if (!headers_only) {
            gchar *uuid_printable;
            const gchar *cid_printable;

            uuid_printable = mbim_uuid_get_printable (mbim_message_command_get_service_id (self));
            cid_printable = mbim_cid_get_printable (mbim_message_command_get_service (self),
                                                    mbim_message_command_get_cid (self));
            g_string_append_printf (printable,
                                    "%sContents:\n"
                                    "%s  service = '%s' (%s)\n"
                                    "%s  cid     = '%s' (0x%08x)\n"
                                    "%s  type    = '%s' (0x%08x)\n",
                                    line_prefix,
                                    line_prefix, mbim_service_lookup_name (mbim_message_command_get_service (self)), uuid_printable,
                                    line_prefix, cid_printable, mbim_message_command_get_cid (self),
                                    line_prefix, mbim_message_command_type_get_string (mbim_message_command_get_command_type (self)), mbim_message_command_get_command_type (self));
            g_free (uuid_printable);
        }
        break;

    case MBIM_MESSAGE_TYPE_COMMAND_DONE:
        g_string_append_printf (printable,
                                "%sFragment header:\n"
                                "%s  total   = %u\n"
                                "%s  current = %u\n",
                                line_prefix,
                                line_prefix, _mbim_message_fragment_get_total (self),
                                line_prefix, _mbim_message_fragment_get_current (self));
        if (!headers_only) {
            gchar *uuid_printable;
            MbimStatusError status;
            const gchar *cid_printable;

            status = mbim_message_command_done_get_status_code (self);
            uuid_printable = mbim_uuid_get_printable (mbim_message_command_done_get_service_id (self));
            cid_printable = mbim_cid_get_printable (mbim_message_command_done_get_service (self),
                                                    mbim_message_command_done_get_cid (self));
            g_string_append_printf (printable,
                                    "%sContents:\n"
                                    "%s  status error = '%s' (0x%08x)\n"
                                    "%s  service      = '%s' (%s)\n"
                                    "%s  cid          = '%s' (0x%08x)\n",
                                    line_prefix,
                                    line_prefix, mbim_status_error_get_string (status), status,
                                    line_prefix, mbim_service_lookup_name (mbim_message_command_done_get_service (self)), uuid_printable,
                                    line_prefix, cid_printable, mbim_message_command_done_get_cid (self));
            g_free (uuid_printable);
        }
        break;

    case MBIM_MESSAGE_TYPE_INDICATE_STATUS:
        g_string_append_printf (printable,
                                "%sFragment header:\n"
                                "%s  total   = %u\n"
                                "%s  current = %u\n",
                                line_prefix,
                                line_prefix, _mbim_message_fragment_get_total (self),
                                line_prefix, _mbim_message_fragment_get_current (self));
        if (!headers_only) {
            gchar *uuid_printable;
            const gchar *cid_printable;

            uuid_printable = mbim_uuid_get_printable (mbim_message_indicate_status_get_service_id (self));
            cid_printable = mbim_cid_get_printable (mbim_message_indicate_status_get_service (self),
                                                    mbim_message_indicate_status_get_cid (self));
            g_string_append_printf (printable,
                                    "%sContents:\n"
                                    "%s  service = '%s' (%s)\n"
                                    "%s  cid     = '%s' (0x%08x)\n",
                                    line_prefix,
                                    line_prefix, mbim_service_lookup_name (mbim_message_indicate_status_get_service (self)), uuid_printable,
                                    line_prefix, cid_printable, mbim_message_indicate_status_get_cid (self));
            g_free (uuid_printable);
        }
        break;
    }

    return g_string_free (printable, FALSE);
}

/*****************************************************************************/
/* Fragment interface */

#define MBIM_MESSAGE_IS_FRAGMENT(self)                                  \
    (MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_COMMAND || \
     MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_COMMAND_DONE || \
     MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_INDICATE_STATUS)

#define MBIM_MESSAGE_FRAGMENT_GET_TOTAL(self)                           \
    GUINT32_FROM_LE (((struct full_message *)(self->data))->message.fragment.fragment_header.total)

#define MBIM_MESSAGE_FRAGMENT_GET_CURRENT(self)                         \
    GUINT32_FROM_LE (((struct full_message *)(self->data))->message.fragment.fragment_header.current)


gboolean
_mbim_message_is_fragment (const MbimMessage *self)
{
    return MBIM_MESSAGE_IS_FRAGMENT (self);
}

guint32
_mbim_message_fragment_get_total (const MbimMessage  *self)
{
    g_assert (MBIM_MESSAGE_IS_FRAGMENT (self));

    return MBIM_MESSAGE_FRAGMENT_GET_TOTAL (self);
}

guint32
_mbim_message_fragment_get_current (const MbimMessage  *self)
{
    g_assert (MBIM_MESSAGE_IS_FRAGMENT (self));

    return MBIM_MESSAGE_FRAGMENT_GET_CURRENT (self);
}

const guint8 *
_mbim_message_fragment_get_payload (const MbimMessage *self,
                                    guint32           *length)
{
    g_assert (MBIM_MESSAGE_IS_FRAGMENT (self));

    *length = (MBIM_MESSAGE_GET_MESSAGE_LENGTH (self) - \
               sizeof (struct header) -                 \
               sizeof (struct fragment_header));
    return ((struct full_message *)(self->data))->message.fragment.buffer;
}

MbimMessage *
_mbim_message_fragment_collector_init (const MbimMessage  *fragment,
                                       GError            **error)
{
   g_assert (MBIM_MESSAGE_IS_FRAGMENT (fragment));

   /* Collector must start with fragment #0 */
   if (MBIM_MESSAGE_FRAGMENT_GET_CURRENT (fragment) != 0) {
       g_set_error (error,
                    MBIM_PROTOCOL_ERROR,
                    MBIM_PROTOCOL_ERROR_FRAGMENT_OUT_OF_SEQUENCE,
                    "Expecting fragment '0/%u', got '%u/%u'",
                    MBIM_MESSAGE_FRAGMENT_GET_TOTAL (fragment),
                    MBIM_MESSAGE_FRAGMENT_GET_CURRENT (fragment),
                    MBIM_MESSAGE_FRAGMENT_GET_TOTAL (fragment));
       return NULL;
   }

   return mbim_message_dup (fragment);
}

gboolean
_mbim_message_fragment_collector_add (MbimMessage        *self,
                                      const MbimMessage  *fragment,
                                      GError            **error)
{
    guint32 buffer_len;
    const guint8 *buffer;

    g_assert (MBIM_MESSAGE_IS_FRAGMENT (self));
    g_assert (MBIM_MESSAGE_IS_FRAGMENT (fragment));

    /* We can only add a fragment if it is the next one we're expecting.
     * Otherwise, we return an error. */
    if (MBIM_MESSAGE_FRAGMENT_GET_CURRENT (self) != (MBIM_MESSAGE_FRAGMENT_GET_CURRENT (fragment) - 1)) {
        g_set_error (error,
                     MBIM_PROTOCOL_ERROR,
                     MBIM_PROTOCOL_ERROR_FRAGMENT_OUT_OF_SEQUENCE,
                     "Expecting fragment '%u/%u', got '%u/%u'",
                     MBIM_MESSAGE_FRAGMENT_GET_CURRENT (self) + 1,
                     MBIM_MESSAGE_FRAGMENT_GET_TOTAL (self),
                     MBIM_MESSAGE_FRAGMENT_GET_CURRENT (fragment),
                     MBIM_MESSAGE_FRAGMENT_GET_TOTAL (fragment));
        return FALSE;
    }

    buffer = _mbim_message_fragment_get_payload (fragment, &buffer_len);
    if (buffer_len) {
        /* Concatenate information buffers */
        g_byte_array_append ((GByteArray *)self, buffer, buffer_len);
        /* Update the whole message length */
        ((struct header *)(self->data))->length =
            GUINT32_TO_LE (MBIM_MESSAGE_GET_MESSAGE_LENGTH (self) + buffer_len);
    }

    /* Update the current fragment info in the main message; skip endian changes */
    ((struct full_message *)(self->data))->message.fragment.fragment_header.current =
        ((struct full_message *)(fragment->data))->message.fragment.fragment_header.current;

    return TRUE;
}

gboolean
_mbim_message_fragment_collector_complete (MbimMessage *self)
{
    g_assert (MBIM_MESSAGE_IS_FRAGMENT (self));

    if (MBIM_MESSAGE_FRAGMENT_GET_CURRENT (self) != (MBIM_MESSAGE_FRAGMENT_GET_TOTAL (self) - 1))
        /* Not complete yet */
        return FALSE;

    /* Reset current & total */
    ((struct full_message *)(self->data))->message.fragment.fragment_header.current = 0;
    ((struct full_message *)(self->data))->message.fragment.fragment_header.total = GUINT32_TO_LE (1);
    return TRUE;
}

struct fragment_info *
_mbim_message_split_fragments (const MbimMessage *self,
                               guint32            max_fragment_size,
                               guint             *n_fragments)
{
    GArray *array;
    guint32 total_message_length;
    guint32 total_payload_length;
    guint32 fragment_header_length;
    guint32 fragment_payload_length;
    guint32 total_fragments;
    guint i;
    const guint8 *data;
    guint32 data_length;

    /* A message which is longer than the maximum fragment size needs to be
     * split in different fragments before sending it. */

    total_message_length = mbim_message_get_message_length (self);

    /* If a single fragment is enough, don't try to split */
    if (total_message_length <= max_fragment_size)
        return NULL;

    /* Total payload length is the total length minus the headers of the
     * input message */
    fragment_header_length = sizeof (struct header) + sizeof (struct fragment_header);
    total_payload_length = total_message_length - fragment_header_length;

    /* Fragment payload length is the maximum amount of data that can fit in a
     * single fragment */
    fragment_payload_length = max_fragment_size - fragment_header_length;

    /* We can now compute the number of fragments that we'll get */
    total_fragments = total_payload_length / fragment_payload_length;
    if (total_payload_length % fragment_payload_length)
        total_fragments++;

    array = g_array_sized_new (FALSE,
                               FALSE,
                               sizeof (struct fragment_info),
                               total_fragments);

    /* Initialize data walkers */
    data = ((struct full_message *)(((GByteArray *)self)->data))->message.fragment.buffer;
    data_length = total_payload_length;

    /* Create fragment infos */
    for (i = 0; i < total_fragments; i++) {
        struct fragment_info info;

        /* Set data info */
        info.data = data;
        info.data_length = (data_length > fragment_payload_length ? fragment_payload_length : data_length);

        /* Set header info */
        info.header.type             = GUINT32_TO_LE (MBIM_MESSAGE_GET_MESSAGE_TYPE (self));
        info.header.length           = GUINT32_TO_LE (fragment_header_length + info.data_length);
        info.header.transaction_id   = GUINT32_TO_LE (MBIM_MESSAGE_GET_TRANSACTION_ID (self));
        info.fragment_header.total   = GUINT32_TO_LE (total_fragments);
        info.fragment_header.current = GUINT32_TO_LE (i);

        g_array_insert_val (array, i, info);

        /* Update walkers */
        data = &data[info.data_length];
        data_length -= info.data_length;
    }

    g_warn_if_fail (data_length == 0);

    *n_fragments = total_fragments;
    return (struct fragment_info *) g_array_free (array, FALSE);
}

/*****************************************************************************/
/* 'Open' message interface */

/**
 * mbim_message_open_new:
 * @transaction_id: transaction ID.
 * @max_control_transfer: maximum control transfer.
 *
 * Create a new #MbimMessage of type %MBIM_MESSAGE_TYPE_OPEN with the specified
 * parameters.
 *
 * Returns: (transfer full): a newly created #MbimMessage. The returned value
 * should be freed with mbim_message_unref().
 */
MbimMessage *
mbim_message_open_new (guint32 transaction_id,
                       guint32 max_control_transfer)
{
    GByteArray *self;

    self = _mbim_message_allocate (MBIM_MESSAGE_TYPE_OPEN,
                                   transaction_id,
                                   sizeof (struct open_message));

    /* Open header */
    ((struct full_message *)(self->data))->message.open.max_control_transfer = GUINT32_TO_LE (max_control_transfer);

    return (MbimMessage *)self;
}

/**
 * mbim_message_open_get_max_control_transfer:
 * @self: a #MbimMessage.
 *
 * Get the maximum control transfer set to be used in the #MbimMessage of type
 * %MBIM_MESSAGE_TYPE_OPEN.
 *
 * Returns: the maximum control transfer.
 */
guint32
mbim_message_open_get_max_control_transfer (const MbimMessage *self)
{
    g_return_val_if_fail (self != NULL, 0);
    g_return_val_if_fail (MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_OPEN, 0);

    return GUINT32_FROM_LE (((struct full_message *)(self->data))->message.open.max_control_transfer);
}

/*****************************************************************************/
/* 'Open Done' message interface */

/**
 * mbim_message_open_done_new:
 * @transaction_id: transaction ID.
 * @error_status_code: a #MbimStatusError.
 *
 * Create a new #MbimMessage of type %MBIM_MESSAGE_TYPE_OPEN_DONE with the specified
 * parameters.
 *
 * Returns: (transfer full): a newly created #MbimMessage, which should be freed with mbim_message_unref().
 */
MbimMessage *
mbim_message_open_done_new (guint32         transaction_id,
                            MbimStatusError error_status_code)
{
    GByteArray *self;

    self = _mbim_message_allocate (MBIM_MESSAGE_TYPE_OPEN_DONE,
                                   transaction_id,
                                   sizeof (struct open_done_message));

    /* Open header */
    ((struct full_message *)(self->data))->message.open_done.status_code = GUINT32_TO_LE (error_status_code);

    return (MbimMessage *)self;
}

/**
 * mbim_message_open_done_get_status_code:
 * @self: a #MbimMessage.
 *
 * Get status code from the %MBIM_MESSAGE_TYPE_OPEN_DONE message.
 *
 * Returns: a #MbimStatusError.
 */
MbimStatusError
mbim_message_open_done_get_status_code (const MbimMessage *self)
{
    g_return_val_if_fail (self != NULL, MBIM_STATUS_ERROR_FAILURE);
    g_return_val_if_fail (MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_OPEN_DONE, MBIM_STATUS_ERROR_FAILURE);

    return (MbimStatusError) GUINT32_FROM_LE (((struct full_message *)(self->data))->message.open_done.status_code);
}

/**
 * mbim_message_open_done_get_result:
 * @self: a #MbimMessage.
 * @error: return location for error or %NULL.
 *
 * Gets the result of the 'Open' operation in the %MBIM_MESSAGE_TYPE_OPEN_DONE message.
 *
 * Returns: %TRUE if the operation succeeded, %FALSE if @error is set.
 */
gboolean
mbim_message_open_done_get_result (const MbimMessage  *self,
                                   GError            **error)
{
    MbimStatusError status;

    g_return_val_if_fail (self != NULL, FALSE);
    g_return_val_if_fail (MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_OPEN_DONE, FALSE);

    status = (MbimStatusError) GUINT32_FROM_LE (((struct full_message *)(self->data))->message.open_done.status_code);
    if (status == MBIM_STATUS_ERROR_NONE)
        return TRUE;

    g_set_error_literal (error,
                         MBIM_STATUS_ERROR,
                         status,
                         mbim_status_error_get_string (status));
    return FALSE;
}

/*****************************************************************************/
/* 'Close' message interface */

/**
 * mbim_message_close_new:
 * @transaction_id: transaction ID.
 *
 * Create a new #MbimMessage of type %MBIM_MESSAGE_TYPE_CLOSE with the specified
 * parameters.
 *
 * Returns: (transfer full): a newly created #MbimMessage. The returned value
 * should be freed with mbim_message_unref().
 */
MbimMessage *
mbim_message_close_new (guint32 transaction_id)
{
    return (MbimMessage *) _mbim_message_allocate (MBIM_MESSAGE_TYPE_CLOSE,
                                                   transaction_id,
                                                   0);
}

/*****************************************************************************/
/* 'Close Done' message interface */

/**
 * mbim_message_close_done_new:
 * @transaction_id: transaction ID.
 * @error_status_code: a #MbimStatusError.
 *
 * Create a new #MbimMessage of type %MBIM_MESSAGE_TYPE_CLOSE_DONE with the specified
 * parameters.
 *
 * Returns: (transfer full): a newly created #MbimMessage, which should be freed with mbim_message_unref().
 */
MbimMessage *
mbim_message_close_done_new (guint32         transaction_id,
                             MbimStatusError error_status_code)
{
    GByteArray *self;

    self = _mbim_message_allocate (MBIM_MESSAGE_TYPE_CLOSE_DONE,
                                   transaction_id,
                                   sizeof (struct close_done_message));

    /* Open header */
    ((struct full_message *)(self->data))->message.close_done.status_code = GUINT32_TO_LE (error_status_code);

    return (MbimMessage *)self;
}

/**
 * mbim_message_close_done_get_status_code:
 * @self: a #MbimMessage.
 *
 * Get status code from the %MBIM_MESSAGE_TYPE_CLOSE_DONE message.
 *
 * Returns: a #MbimStatusError.
 */
MbimStatusError
mbim_message_close_done_get_status_code (const MbimMessage *self)
{
    g_return_val_if_fail (self != NULL, MBIM_STATUS_ERROR_FAILURE);
    g_return_val_if_fail (MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_CLOSE_DONE, MBIM_STATUS_ERROR_FAILURE);

    return (MbimStatusError) GUINT32_FROM_LE (((struct full_message *)(self->data))->message.close_done.status_code);
}

/**
 * mbim_message_close_done_get_result:
 * @self: a #MbimMessage.
 * @error: return location for error or %NULL.
 *
 * Gets the result of the 'Close' operation in the %MBIM_MESSAGE_TYPE_CLOSE_DONE message.
 *
 * Returns: %TRUE if the operation succeeded, %FALSE if @error is set.
 */
gboolean
mbim_message_close_done_get_result (const MbimMessage  *self,
                                    GError            **error)
{
    MbimStatusError status;

    g_return_val_if_fail (self != NULL, FALSE);
    g_return_val_if_fail (MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_CLOSE_DONE, FALSE);

    status = (MbimStatusError) GUINT32_FROM_LE (((struct full_message *)(self->data))->message.close_done.status_code);
    if (status == MBIM_STATUS_ERROR_NONE)
        return TRUE;

    g_set_error_literal (error,
                         MBIM_STATUS_ERROR,
                         status,
                         mbim_status_error_get_string (status));
    return FALSE;
}

/*****************************************************************************/
/* 'Error' message interface */

/**
 * mbim_message_error_new:
 * @transaction_id: transaction ID.
 * @error_status_code: a #MbimProtocolError.
 *
 * Create a new #MbimMessage of type %MBIM_MESSAGE_TYPE_HOST_ERROR with the specified
 * parameters.
 *
 * Returns: (transfer full): a newly created #MbimMessage. The returned value
 * should be freed with mbim_message_unref().
 */
MbimMessage *
mbim_message_error_new (guint32           transaction_id,
                        MbimProtocolError error_status_code)
{
    GByteArray *self;

    self = _mbim_message_allocate (MBIM_MESSAGE_TYPE_HOST_ERROR,
                                   transaction_id,
                                   sizeof (struct error_message));

    /* Open header */
    ((struct full_message *)(self->data))->message.error.error_status_code = GUINT32_TO_LE (error_status_code);

    return (MbimMessage *)self;
}

/**
 * mbim_message_function_error_new:
 * @transaction_id: transaction ID.
 * @error_status_code: a #MbimProtocolError.
 *
 * Create a new #MbimMessage of type %MBIM_MESSAGE_TYPE_FUNCTION_ERROR with the specified
 * parameters.
 *
 * Returns: (transfer full): a newly created #MbimMessage. The returned value
 * should be freed with mbim_message_unref().
 */
MbimMessage *
mbim_message_function_error_new (guint32           transaction_id,
                                 MbimProtocolError error_status_code)
{
    GByteArray *self;

    self = _mbim_message_allocate (MBIM_MESSAGE_TYPE_FUNCTION_ERROR,
                                   transaction_id,
                                   sizeof (struct error_message));

    /* Open header */
    ((struct full_message *)(self->data))->message.error.error_status_code = GUINT32_TO_LE (error_status_code);

    return (MbimMessage *)self;
}

/**
 * mbim_message_error_get_error_status_code:
 * @self: a #MbimMessage.
 *
 * Get the error code in a %MBIM_MESSAGE_TYPE_HOST_ERROR or
 * %MBIM_MESSAGE_TYPE_FUNCTION_ERROR message.
 *
 * Returns: a #MbimProtocolError.
 */
MbimProtocolError
mbim_message_error_get_error_status_code (const MbimMessage *self)
{
    g_return_val_if_fail (self != NULL, MBIM_PROTOCOL_ERROR_INVALID);
    g_return_val_if_fail ((MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_HOST_ERROR ||
                           MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_FUNCTION_ERROR),
                          MBIM_PROTOCOL_ERROR_INVALID);

    return (MbimProtocolError) GUINT32_FROM_LE (((struct full_message *)(self->data))->message.error.error_status_code);
}

/**
 * mbim_message_error_get_error:
 * @self: a #MbimMessage.
 *
 * Get the error in a %MBIM_MESSAGE_TYPE_HOST_ERROR or
 * %MBIM_MESSAGE_TYPE_FUNCTION_ERROR message.
 *
 * Returns: a newly allocated #GError, which should be freed with g_error_free().
 */
GError *
mbim_message_error_get_error (const MbimMessage *self)
{
    MbimProtocolError error_status_code;


    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail ((MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_HOST_ERROR ||
                           MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_FUNCTION_ERROR),
                          NULL);

    error_status_code = (MbimProtocolError) GUINT32_FROM_LE (((struct full_message *)(self->data))->message.error.error_status_code);

    return g_error_new (MBIM_PROTOCOL_ERROR,
                        error_status_code,
                        "MBIM protocol error: %s",
                        mbim_protocol_error_get_string (error_status_code));
}

/*****************************************************************************/
/* 'Command' message interface */

/**
 * mbim_message_command_new:
 * @transaction_id: transaction ID.
 * @service: a #MbimService.
 * @cid: the command ID.
 * @command_type: the command type.
 *
 * Create a new #MbimMessage of type %MBIM_MESSAGE_TYPE_COMMAND with the
 * specified parameters and an empty information buffer.
 *
 * Returns: (transfer full): a newly created #MbimMessage. The returned value
 * should be freed with mbim_message_unref().
 */
MbimMessage *
mbim_message_command_new (guint32                transaction_id,
                          MbimService            service,
                          guint32                cid,
                          MbimMessageCommandType command_type)
{
    GByteArray *self;
    const MbimUuid *service_id;

    /* Known service required */
    service_id = mbim_uuid_from_service (service);
    g_return_val_if_fail (service_id != NULL, NULL);

    self = _mbim_message_allocate (MBIM_MESSAGE_TYPE_COMMAND,
                                   transaction_id,
                                   sizeof (struct command_message));

    /* Fragment header */
    ((struct full_message *)(self->data))->message.command.fragment_header.total   = GUINT32_TO_LE (1);
    ((struct full_message *)(self->data))->message.command.fragment_header.current = 0;

    /* Command header */
    memcpy (((struct full_message *)(self->data))->message.command.service_id, service_id, sizeof (*service_id));
    ((struct full_message *)(self->data))->message.command.command_id    = GUINT32_TO_LE (cid);
    ((struct full_message *)(self->data))->message.command.command_type  = GUINT32_TO_LE (command_type);
    ((struct full_message *)(self->data))->message.command.buffer_length = 0;

    return (MbimMessage *)self;
}

/**
 * mbim_message_command_append:
 * @self: a #MbimMessage.
 * @buffer: raw buffer to append to the message.
 * @buffer_size: length of the data in @buffer.
 *
 * Appends the contents of @buffer to @self.
 */
void
mbim_message_command_append (MbimMessage  *self,
                             const guint8 *buffer,
                             guint32       buffer_size)
{
    g_byte_array_append ((GByteArray *)self, buffer, buffer_size);

    /* Update message and buffer length */
    ((struct header *)(self->data))->length =
        GUINT32_TO_LE (MBIM_MESSAGE_GET_MESSAGE_LENGTH (self) + buffer_size);
    ((struct full_message *)(self->data))->message.command.buffer_length =
        GUINT32_TO_LE (GUINT32_FROM_LE (((struct full_message *)(self->data))->message.command.buffer_length) + buffer_size);
}

/**
 * mbim_message_command_get_service:
 * @self: a #MbimMessage.
 *
 * Get the service of a %MBIM_MESSAGE_TYPE_COMMAND message.
 *
 * Returns: a #MbimService.
 */
MbimService
mbim_message_command_get_service (const MbimMessage *self)
{
    g_return_val_if_fail (self != NULL, MBIM_SERVICE_INVALID);
    g_return_val_if_fail (MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_COMMAND, MBIM_SERVICE_INVALID);

    return mbim_uuid_to_service ((const MbimUuid *)&(((struct full_message *)(self->data))->message.command.service_id));
}

/**
 * mbim_message_command_get_service_id:
 * @self: a #MbimMessage.
 *
 * Get the service UUID of a %MBIM_MESSAGE_TYPE_COMMAND message.
 *
 * Returns: a #MbimUuid.
 */
const MbimUuid *
mbim_message_command_get_service_id (const MbimMessage *self)
{
    g_return_val_if_fail (self != NULL, MBIM_UUID_INVALID);
    g_return_val_if_fail (MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_COMMAND, MBIM_UUID_INVALID);

    return (const MbimUuid *)&(((struct full_message *)(self->data))->message.command.service_id);
}

/**
 * mbim_message_command_get_cid:
 * @self: a #MbimMessage.
 *
 * Get the command id of a %MBIM_MESSAGE_TYPE_COMMAND message.
 *
 * Returns: a CID.
 */
guint32
mbim_message_command_get_cid (const MbimMessage *self)
{
    g_return_val_if_fail (self != NULL, 0);
    g_return_val_if_fail (MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_COMMAND, 0);

    return GUINT32_FROM_LE (((struct full_message *)(self->data))->message.command.command_id);
}

/**
 * mbim_message_command_get_command_type:
 * @self: a #MbimMessage.
 *
 * Get the command type of a %MBIM_MESSAGE_TYPE_COMMAND message.
 *
 * Returns: a #MbimMessageCommandType.
 */
MbimMessageCommandType
mbim_message_command_get_command_type (const MbimMessage *self)
{
    g_return_val_if_fail (self != NULL, MBIM_MESSAGE_COMMAND_TYPE_UNKNOWN);
    g_return_val_if_fail (MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_COMMAND, MBIM_MESSAGE_COMMAND_TYPE_UNKNOWN);

    return (MbimMessageCommandType) GUINT32_FROM_LE (((struct full_message *)(self->data))->message.command.command_type);
}

/**
 * mbim_message_command_get_raw_information_buffer:
 * @self: a #MbimMessage.
 * @length: (out): return location for the size of the output buffer.
 *
 * Gets the information buffer of the %MBIM_MESSAGE_TYPE_COMMAND message.
 *
 * Returns: (transfer none): The raw data buffer, or #NULL if empty.
 */
const guint8 *
mbim_message_command_get_raw_information_buffer (const MbimMessage *self,
                                                 guint32           *length)
{
    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (length != NULL, NULL);
    g_return_val_if_fail (MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_COMMAND, NULL);

    *length = GUINT32_FROM_LE (((struct full_message *)(self->data))->message.command.buffer_length);

    return (*length > 0 ?
            ((struct full_message *)(self->data))->message.command.buffer :
            NULL);
}

/*****************************************************************************/
/* 'Command Done' message interface */

/**
 * mbim_message_command_done_get_service:
 * @self: a #MbimMessage.
 *
 * Get the service of a %MBIM_MESSAGE_TYPE_COMMAND_DONE message.
 *
 * Returns: a #MbimService.
 */
MbimService
mbim_message_command_done_get_service (const MbimMessage *self)
{
    g_return_val_if_fail (self != NULL, MBIM_SERVICE_INVALID);
    g_return_val_if_fail (MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_COMMAND_DONE, MBIM_SERVICE_INVALID);

    return mbim_uuid_to_service ((const MbimUuid *)&(((struct full_message *)(self->data))->message.command_done.service_id));
}

/**
 * mbim_message_command_done_get_service_id:
 * @self: a #MbimMessage.
 *
 * Get the service UUID of a %MBIM_MESSAGE_TYPE_COMMAND_DONE message.
 *
 * Returns: a #MbimUuid.
 */
const MbimUuid *
mbim_message_command_done_get_service_id (const MbimMessage *self)
{
    g_return_val_if_fail (self != NULL, MBIM_UUID_INVALID);
    g_return_val_if_fail (MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_COMMAND_DONE, MBIM_UUID_INVALID);

    return (const MbimUuid *)&(((struct full_message *)(self->data))->message.command_done.service_id);
}

/**
 * mbim_message_command_done_get_cid:
 * @self: a #MbimMessage.
 *
 * Get the command id of a %MBIM_MESSAGE_TYPE_COMMAND_DONE message.
 *
 * Returns: a CID.
 */
guint32
mbim_message_command_done_get_cid (const MbimMessage *self)
{
    g_return_val_if_fail (self != NULL, 0);
    g_return_val_if_fail (MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_COMMAND_DONE, 0);

    return GUINT32_FROM_LE (((struct full_message *)(self->data))->message.command_done.command_id);
}

/**
 * mbim_message_command_done_get_status_code:
 * @self: a #MbimMessage.
 *
 * Get status code from the %MBIM_MESSAGE_TYPE_COMMAND_DONE message.
 *
 * Returns: a #MbimStatusError.
 */
MbimStatusError
mbim_message_command_done_get_status_code (const MbimMessage *self)
{
    g_return_val_if_fail (self != NULL, MBIM_STATUS_ERROR_FAILURE);
    g_return_val_if_fail (MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_COMMAND_DONE, MBIM_STATUS_ERROR_FAILURE);

    return (MbimStatusError) GUINT32_FROM_LE (((struct full_message *)(self->data))->message.command_done.status_code);
}

/**
 * mbim_message_command_done_get_result:
 * @self: a #MbimMessage.
 * @error: return location for error or %NULL.
 *
 * Gets the result of the 'Command' operation in the %MBIM_MESSAGE_TYPE_COMMAND_DONE message.
 *
 * Returns: %TRUE if the operation succeeded, %FALSE if @error is set.
 */
gboolean
mbim_message_command_done_get_result (const MbimMessage  *self,
                                      GError            **error)
{
    MbimStatusError status;

    g_return_val_if_fail (self != NULL, FALSE);
    g_return_val_if_fail (MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_COMMAND_DONE, FALSE);

    status = (MbimStatusError) GUINT32_FROM_LE (((struct full_message *)(self->data))->message.command_done.status_code);
    if (status == MBIM_STATUS_ERROR_NONE)
        return TRUE;

    g_set_error_literal (error,
                         MBIM_STATUS_ERROR,
                         status,
                         mbim_status_error_get_string (status));
    return FALSE;
}

/**
 * mbim_message_command_done_get_raw_information_buffer:
 * @self: a #MbimMessage.
 * @length: (out): return location for the size of the output buffer.
 *
 * Gets the information buffer of the %MBIM_MESSAGE_TYPE_COMMAND_DONE message.
 *
 * Returns: (transfer none): The raw data buffer, or #NULL if empty.
 */
const guint8 *
mbim_message_command_done_get_raw_information_buffer (const MbimMessage *self,
                                                      guint32           *length)
{
    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_COMMAND_DONE, NULL);
    g_return_val_if_fail (length != NULL, NULL);

    *length = GUINT32_FROM_LE (((struct full_message *)(self->data))->message.command_done.buffer_length);

    return (*length > 0 ?
            ((struct full_message *)(self->data))->message.command_done.buffer :
            NULL);
}

/*****************************************************************************/
/* 'Indicate Status' message interface */

/**
 * mbim_message_indicate_status_get_service:
 * @self: a #MbimMessage.
 *
 * Get the service of a %MBIM_MESSAGE_TYPE_INDICATE_STATUS message.
 *
 * Returns: a #MbimService.
 */
MbimService
mbim_message_indicate_status_get_service (const MbimMessage *self)
{
    g_return_val_if_fail (self != NULL, MBIM_SERVICE_INVALID);
    g_return_val_if_fail (MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_INDICATE_STATUS, MBIM_SERVICE_INVALID);

    return mbim_uuid_to_service ((const MbimUuid *)&(((struct full_message *)(self->data))->message.indicate_status.service_id));
}

/**
 * mbim_message_indicate_status_get_service_id:
 * @self: a #MbimMessage.
 *
 * Get the service UUID of a %MBIM_MESSAGE_TYPE_INDICATE_STATUS message.
 *
 * Returns: a #MbimUuid.
 */
const MbimUuid *
mbim_message_indicate_status_get_service_id (const MbimMessage *self)
{
    g_return_val_if_fail (self != NULL, MBIM_UUID_INVALID);
    g_return_val_if_fail (MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_INDICATE_STATUS, MBIM_UUID_INVALID);

    return (const MbimUuid *)&(((struct full_message *)(self->data))->message.indicate_status.service_id);
}

/**
 * mbim_message_indicate_status_get_cid:
 * @self: a #MbimMessage.
 *
 * Get the command id of a %MBIM_MESSAGE_TYPE_INDICATE_STATUS message.
 *
 * Returns: a CID.
 */
guint32
mbim_message_indicate_status_get_cid (const MbimMessage *self)
{
    g_return_val_if_fail (self != NULL, 0);
    g_return_val_if_fail (MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_INDICATE_STATUS, 0);

    return GUINT32_FROM_LE (((struct full_message *)(self->data))->message.indicate_status.command_id);
}

/**
 * mbim_message_indicate_status_get_raw_information_buffer:
 * @self: a #MbimMessage.
 * @length: (out): return location for the size of the output buffer.
 *
 * Gets the information buffer of the %MBIM_MESSAGE_TYPE_INDICATE_STATUS message.
 *
 * Returns: (transfer none): The raw data buffer, or #NULL if empty.
 */
const guint8 *
mbim_message_indicate_status_get_raw_information_buffer (const MbimMessage *self,
                                                         guint32           *length)
{
    g_return_val_if_fail (self != NULL, NULL);
    g_return_val_if_fail (MBIM_MESSAGE_GET_MESSAGE_TYPE (self) == MBIM_MESSAGE_TYPE_INDICATE_STATUS, NULL);
    g_return_val_if_fail (length != NULL, NULL);

    *length = GUINT32_FROM_LE (((struct full_message *)(self->data))->message.indicate_status.buffer_length);

    return (*length > 0 ?
            ((struct full_message *)(self->data))->message.indicate_status.buffer :
            NULL);
}

/*****************************************************************************/
/* Other helpers */

/**
 * mbim_message_response_get_result:
 * @self: a #MbimMessage response message.
 * @expected: expected #MbimMessageType if there isn't any error in the operation.
 * @error: return location for error or %NULL.
 *
 * Gets the result of the operation from the response message, which
 * can be either a %MBIM_MESSAGE_TYPE_FUNCTION_ERROR message or a message of the
 * specified @expected type.
 *
 * Returns: %TRUE if the operation succeeded, %FALSE if @error is set.
 */
gboolean
mbim_message_response_get_result (const MbimMessage  *self,
                                  MbimMessageType     expected,
                                  GError            **error)
{
    MbimStatusError status = MBIM_STATUS_ERROR_NONE;
    MbimMessageType type;

    g_return_val_if_fail (self != NULL, FALSE);
    g_return_val_if_fail (expected == MBIM_MESSAGE_TYPE_OPEN_DONE  ||
                          expected == MBIM_MESSAGE_TYPE_CLOSE_DONE ||
                          expected == MBIM_MESSAGE_TYPE_COMMAND_DONE, FALSE);

    type = MBIM_MESSAGE_GET_MESSAGE_TYPE (self);
    if (type != MBIM_MESSAGE_TYPE_FUNCTION_ERROR && type != expected) {
        g_set_error (error,
                     MBIM_CORE_ERROR,
                     MBIM_CORE_ERROR_INVALID_MESSAGE,
                     "Unexpected response message type: 0x%04X", (guint32) type);
        return FALSE;
    }

    switch (type) {
    case MBIM_MESSAGE_TYPE_OPEN_DONE:
        status = (MbimStatusError) GUINT32_FROM_LE (((struct full_message *)(self->data))->message.open_done.status_code);
        break;

    case MBIM_MESSAGE_TYPE_CLOSE_DONE:
        status = (MbimStatusError) GUINT32_FROM_LE (((struct full_message *)(self->data))->message.close_done.status_code);
        break;

    case MBIM_MESSAGE_TYPE_COMMAND_DONE:
        status = (MbimStatusError) GUINT32_FROM_LE (((struct full_message *)(self->data))->message.command_done.status_code);
        break;

    case MBIM_MESSAGE_TYPE_FUNCTION_ERROR:
        if (error)
            *error = mbim_message_error_get_error (self);
        return FALSE;

    default:
        g_assert_not_reached ();
    }

    if (status == MBIM_STATUS_ERROR_NONE)
        return TRUE;

    /* Build error */
    g_set_error_literal (error,
                         MBIM_STATUS_ERROR,
                         status,
                         mbim_status_error_get_string (status));
    return FALSE;
}
