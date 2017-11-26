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
 *
 * This is a private non-installed header
 */

#ifndef _LIBMBIM_GLIB_MBIM_MESSAGE_PRIVATE_H_
#define _LIBMBIM_GLIB_MBIM_MESSAGE_PRIVATE_H_

#if !defined (LIBMBIM_GLIB_COMPILATION)
#error "This is a private header!!"
#endif

#include <glib.h>

#include "mbim-message.h"

G_BEGIN_DECLS

/*****************************************************************************/
/* The MbimMessage */

/* Defined in the same way as GByteArray */
struct _MbimMessage {
  guint8 *data;
  guint   len;
};

/*****************************************************************************/
/* Basic message types */

struct header {
  guint32 type;
  guint32 length;
  guint32 transaction_id;
} __attribute__((packed));

#define MBIM_MESSAGE_GET_MESSAGE_TYPE(self)                             \
    (MbimMessageType) GUINT32_FROM_LE (((struct header *)(self->data))->type)
#define MBIM_MESSAGE_GET_MESSAGE_LENGTH(self)                           \
    GUINT32_FROM_LE (((struct header *)(self->data))->length)
#define MBIM_MESSAGE_GET_TRANSACTION_ID(self)                           \
    GUINT32_FROM_LE (((struct header *)(self->data))->transaction_id)

struct open_message {
    guint32 max_control_transfer;
} __attribute__((packed));

struct open_done_message {
    guint32 status_code;
} __attribute__((packed));

struct close_done_message {
    guint32 status_code;
} __attribute__((packed));

struct error_message {
    guint32 error_status_code;
} __attribute__((packed));

struct fragment_header {
  guint32 total;
  guint32 current;
} __attribute__((packed));

struct fragment_message {
    struct fragment_header fragment_header;
    guint8                 buffer[];
} __attribute__((packed));

struct command_message {
    struct fragment_header fragment_header;
    guint8                 service_id[16];
    guint32                command_id;
    guint32                command_type;
    guint32                buffer_length;
    guint8                 buffer[];
} __attribute__((packed));

struct command_done_message {
    struct fragment_header fragment_header;
    guint8                 service_id[16];
    guint32                command_id;
    guint32                status_code;
    guint32                buffer_length;
    guint8                 buffer[];
} __attribute__((packed));

struct indicate_status_message {
    struct fragment_header fragment_header;
    guint8                 service_id[16];
    guint32                command_id;
    guint32                buffer_length;
    guint8                 buffer[];
} __attribute__((packed));

struct full_message {
    struct header header;
    union {
        struct open_message            open;
        struct open_done_message       open_done;
        /* nothing needed for close_message */
        struct close_done_message      close_done;
        struct error_message           error;
        struct fragment_message        fragment;
        struct command_message         command;
        struct command_done_message    command_done;
        struct indicate_status_message indicate_status;
    } message;
} __attribute__((packed));

/*****************************************************************************/
/* Message creation */
GByteArray *_mbim_message_allocate (MbimMessageType message_type, guint32 transaction_id, guint32 additional_size);

/*****************************************************************************/
/* Fragment interface */

gboolean      _mbim_message_is_fragment          (const MbimMessage  *self);
guint32       _mbim_message_fragment_get_total   (const MbimMessage  *self);
guint32       _mbim_message_fragment_get_current (const MbimMessage  *self);
const guint8 *_mbim_message_fragment_get_payload (const MbimMessage  *self,
                                                  guint32            *length);

/* Merge fragments into a message... */

MbimMessage *_mbim_message_fragment_collector_init     (const MbimMessage  *fragment,
                                                        GError            **error);
gboolean     _mbim_message_fragment_collector_add      (MbimMessage        *self,
                                                        const MbimMessage  *fragment,
                                                        GError            **error);
gboolean     _mbim_message_fragment_collector_complete (MbimMessage        *self);

/* Split message into fragments... */

struct fragment_info {
    struct header           header;
    struct fragment_header  fragment_header;
    guint32                 data_length;
    const guint8           *data;
} __attribute__((packed));

struct fragment_info *_mbim_message_split_fragments (const MbimMessage *self,
                                                     guint32            max_fragment_size,
                                                     guint             *n_fragments);

/*****************************************************************************/
/* Struct builder */

typedef struct {
    GByteArray  *fixed_buffer;
    GByteArray  *variable_buffer;
    GArray      *offsets;
} MbimStructBuilder;

MbimStructBuilder *_mbim_struct_builder_new                  (void);
GByteArray        *_mbim_struct_builder_complete             (MbimStructBuilder *builder);
void               _mbim_struct_builder_append_byte_array    (MbimStructBuilder *builder,
                                                              gboolean           with_offset,
                                                              gboolean           with_length,
                                                              gboolean           pad_buffer,
                                                              const guint8      *buffer,
                                                              guint32            buffer_len);
void               _mbim_struct_builder_append_uuid          (MbimStructBuilder *builder,
                                                              const MbimUuid    *value);
void               _mbim_struct_builder_append_guint32       (MbimStructBuilder *builder,
                                                              guint32            value);
void               _mbim_struct_builder_append_guint32_array (MbimStructBuilder *builder,
                                                              const guint32     *values,
                                                              guint32            n_values);
void               _mbim_struct_builder_append_guint64       (MbimStructBuilder *builder,
                                                              guint64            value);
void               _mbim_struct_builder_append_guint64_array (MbimStructBuilder *builder,
                                                              const guint64     *values,
                                                              guint32            n_values);
void               _mbim_struct_builder_append_string        (MbimStructBuilder *builder,
                                                              const gchar       *value);
void               _mbim_struct_builder_append_string_array  (MbimStructBuilder  *builder,
                                                              const gchar *const *values,
                                                              guint32             n_values);
void               _mbim_struct_builder_append_ipv4          (MbimStructBuilder *builder,
                                                              const MbimIPv4    *value,
                                                              gboolean           ref);
void               _mbim_struct_builder_append_ipv4_array    (MbimStructBuilder *builder,
                                                              const MbimIPv4    *values,
                                                              guint32            n_values);
void               _mbim_struct_builder_append_ipv6          (MbimStructBuilder *builder,
                                                              const MbimIPv6    *value,
                                                              gboolean           ref);
void               _mbim_struct_builder_append_ipv6_array    (MbimStructBuilder *builder,
                                                              const MbimIPv6    *values,
                                                              guint32            n_values);

/*****************************************************************************/
/* Message builder */

typedef struct {
    MbimMessage *message;
    MbimStructBuilder *contents_builder;
} MbimMessageCommandBuilder;

MbimMessageCommandBuilder *_mbim_message_command_builder_new                  (guint32                    transaction_id,
                                                                               MbimService                service,
                                                                               guint32                    cid,
                                                                               MbimMessageCommandType     command_type);
MbimMessage               *_mbim_message_command_builder_complete             (MbimMessageCommandBuilder *builder);
void                       _mbim_message_command_builder_append_byte_array    (MbimMessageCommandBuilder *builder,
                                                                               gboolean                   with_offset,
                                                                               gboolean                   with_length,
                                                                               gboolean                   pad_buffer,
                                                                               const guint8              *buffer,
                                                                               guint32                    buffer_len);
void                       _mbim_message_command_builder_append_uuid          (MbimMessageCommandBuilder *builder,
                                                                               const MbimUuid            *value);
void                       _mbim_message_command_builder_append_guint32       (MbimMessageCommandBuilder *builder,
                                                                               guint32                    value);
void                       _mbim_message_command_builder_append_guint32_array (MbimMessageCommandBuilder *builder,
                                                                               const guint32             *values,
                                                                               guint32                    n_values);
void                       _mbim_message_command_builder_append_guint64       (MbimMessageCommandBuilder *builder,
                                                                               guint64                    value);
void                       _mbim_message_command_builder_append_guint64_array (MbimMessageCommandBuilder *builder,
                                                                               const guint64             *values,
                                                                               guint32                    n_values);
void                       _mbim_message_command_builder_append_string        (MbimMessageCommandBuilder *builder,
                                                                               const gchar               *value);
void                       _mbim_message_command_builder_append_string_array  (MbimMessageCommandBuilder *builder,
                                                                               const gchar *const        *values,
                                                                               guint32                    n_values);
void                       _mbim_message_command_builder_append_ipv4          (MbimMessageCommandBuilder *builder,
                                                                               const MbimIPv4            *value,
                                                                               gboolean                   ref);
void                       _mbim_message_command_builder_append_ipv4_array    (MbimMessageCommandBuilder *builder,
                                                                               const MbimIPv4            *values,
                                                                               guint32                    n_values);
void                       _mbim_message_command_builder_append_ipv6          (MbimMessageCommandBuilder *builder,
                                                                               const MbimIPv6            *value,
                                                                               gboolean                   ref);
void                       _mbim_message_command_builder_append_ipv6_array    (MbimMessageCommandBuilder *builder,
                                                                               const MbimIPv6            *values,
                                                                               guint32                    n_values);

/*****************************************************************************/
/* Message parser */

const guint8    *_mbim_message_read_byte_array    (const MbimMessage *self,
                                                   guint32            struct_start_offset,
                                                   guint32            relative_offset,
                                                   gboolean           has_offset,
                                                   gboolean           has_length,
                                                   guint32           *array_size);
const MbimUuid  *_mbim_message_read_uuid          (const MbimMessage *self,
                                                   guint32            relative_offset);
guint32          _mbim_message_read_guint32       (const MbimMessage *self,
                                                   guint32            relative_offset);
guint32         *_mbim_message_read_guint32_array (const MbimMessage *self,
                                                   guint32            array_size,
                                                   guint32            relative_offset_array_start);
guint64          _mbim_message_read_guint64       (const MbimMessage *self,
                                                   guint64            relative_offset);
gchar           *_mbim_message_read_string        (const MbimMessage *self,
                                                   guint32            struct_start_offset,
                                                   guint32            relative_offset);
gchar          **_mbim_message_read_string_array  (const MbimMessage *self,
                                                   guint32            array_size,
                                                   guint32            struct_start_offset,
                                                   guint32            relative_offset_array_start);
const MbimIPv4  *_mbim_message_read_ipv4          (const MbimMessage *self,
                                                   guint32            relative_offset,
                                                   gboolean           ref);
MbimIPv4        *_mbim_message_read_ipv4_array    (const MbimMessage *self,
                                                   guint32            array_size,
                                                   guint32            relative_offset_array_start);
const MbimIPv6  *_mbim_message_read_ipv6          (const MbimMessage *self,
                                                   guint32            relative_offset,
                                                   gboolean           ref);
MbimIPv6        *_mbim_message_read_ipv6_array    (const MbimMessage *self,
                                                   guint32            array_size,
                                                   guint32            relative_offset_array_start);

G_END_DECLS

#endif /* _LIBMBIM_GLIB_MBIM_MESSAGE_PRIVATE_H_ */
