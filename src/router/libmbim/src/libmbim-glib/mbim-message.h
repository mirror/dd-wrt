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

#ifndef _LIBMBIM_GLIB_MBIM_MESSAGE_H_
#define _LIBMBIM_GLIB_MBIM_MESSAGE_H_

#if !defined (__LIBMBIM_GLIB_H_INSIDE__) && !defined (LIBMBIM_GLIB_COMPILATION)
#error "Only <libmbim-glib.h> can be included directly."
#endif

#include <glib.h>
#include <glib-object.h>

#include "mbim-uuid.h"
#include "mbim-errors.h"

G_BEGIN_DECLS

/**
 * MbimMessage:
 *
 * An opaque type representing a MBIM message.
 */
typedef struct _MbimMessage MbimMessage;

GType mbim_message_get_type (void) G_GNUC_CONST;
#define MBIM_TYPE_MESSAGE (mbim_message_get_type ())

/**
 * MbimIPv4:
 * @addr: 4 bytes specifying the IPv4 address.
 *
 * An IPv4 address.
 */
typedef struct _MbimIPv4 MbimIPv4;
struct _MbimIPv4 {
    guint8 addr[4];
};

/**
 * MbimIPv6:
 * @addr: 16 bytes specifying the IPv6 address.
 *
 * An IPv6 address.
 */
typedef struct _MbimIPv6 MbimIPv6;
struct _MbimIPv6 {
    guint8 addr[16];
};

/**
 * MbimMessageType:
 * @MBIM_MESSAGE_TYPE_INVALID: Invalid MBIM message.
 * @MBIM_MESSAGE_TYPE_OPEN: Initialization request.
 * @MBIM_MESSAGE_TYPE_CLOSE: Close request.
 * @MBIM_MESSAGE_TYPE_COMMAND: Command request.
 * @MBIM_MESSAGE_TYPE_HOST_ERROR: Host-reported error in the communication.
 * @MBIM_MESSAGE_TYPE_OPEN_DONE: Response to initialization request.
 * @MBIM_MESSAGE_TYPE_CLOSE_DONE: Response to close request.
 * @MBIM_MESSAGE_TYPE_COMMAND_DONE: Response to command request.
 * @MBIM_MESSAGE_TYPE_FUNCTION_ERROR: Function-reported error in the communication.
 * @MBIM_MESSAGE_TYPE_INDICATE_STATUS: Unsolicited message from the function.
 *
 * Type of MBIM messages.
 */
typedef enum {
    MBIM_MESSAGE_TYPE_INVALID         = 0x00000000,
    /* From Host to Function */
    MBIM_MESSAGE_TYPE_OPEN            = 0x00000001,
    MBIM_MESSAGE_TYPE_CLOSE           = 0x00000002,
    MBIM_MESSAGE_TYPE_COMMAND         = 0x00000003,
    MBIM_MESSAGE_TYPE_HOST_ERROR      = 0x00000004,
    /* From Function to Host */
    MBIM_MESSAGE_TYPE_OPEN_DONE       = 0x80000001,
    MBIM_MESSAGE_TYPE_CLOSE_DONE      = 0x80000002,
    MBIM_MESSAGE_TYPE_COMMAND_DONE    = 0x80000003,
    MBIM_MESSAGE_TYPE_FUNCTION_ERROR  = 0x80000004,
    MBIM_MESSAGE_TYPE_INDICATE_STATUS = 0x80000007
} MbimMessageType;

/*****************************************************************************/
/* Generic message interface */

MbimMessage     *mbim_message_new                  (const guint8       *data,
                                                    guint32             data_length);
MbimMessage     *mbim_message_dup                  (const MbimMessage  *self);
MbimMessage     *mbim_message_ref                  (MbimMessage        *self);
void             mbim_message_unref                (MbimMessage        *self);

gchar           *mbim_message_get_printable        (const MbimMessage  *self,
                                                    const gchar        *line_prefix,
                                                    gboolean            headers_only);
const guint8    *mbim_message_get_raw              (const MbimMessage  *self,
                                                    guint32            *length,
                                                    GError            **error);

MbimMessageType  mbim_message_get_message_type     (const MbimMessage  *self);
guint32          mbim_message_get_message_length   (const MbimMessage  *self);
guint32          mbim_message_get_transaction_id   (const MbimMessage  *self);

void             mbim_message_set_transaction_id   (MbimMessage        *self,
                                                    guint32             transaction_id);

/*****************************************************************************/
/* 'Open' message interface */

MbimMessage *mbim_message_open_new                      (guint32            transaction_id,
                                                         guint32            max_control_transfer);
guint32      mbim_message_open_get_max_control_transfer (const MbimMessage *self);

/*****************************************************************************/
/* 'Open Done' message interface */

MbimMessage     *mbim_message_open_done_new (guint32         transaction_id,
                                             MbimStatusError error_status_code);
MbimStatusError  mbim_message_open_done_get_status_code (const MbimMessage  *self);
gboolean         mbim_message_open_done_get_result      (const MbimMessage  *self,
                                                         GError            **error);

/*****************************************************************************/
/* 'Close' message interface */

MbimMessage *mbim_message_close_new (guint32 transaction_id);

/*****************************************************************************/
/* 'Close Done' message interface */

MbimMessage     *mbim_message_close_done_new (guint32         transaction_id,
                                              MbimStatusError error_status_code);
MbimStatusError  mbim_message_close_done_get_status_code (const MbimMessage  *self);
gboolean         mbim_message_close_done_get_result      (const MbimMessage  *self,
                                                          GError            **error);

/*****************************************************************************/
/* 'Error' message interface */

MbimMessage       *mbim_message_error_new                   (guint32            transaction_id,
                                                             MbimProtocolError  error_status_code);
MbimMessage       *mbim_message_function_error_new          (guint32            transaction_id,
                                                             MbimProtocolError  error_status_code);
MbimProtocolError  mbim_message_error_get_error_status_code (const MbimMessage *self);
GError            *mbim_message_error_get_error             (const MbimMessage *self);

/*****************************************************************************/
/* 'Command' message interface */

/**
 * MbimMessageCommandType:
 * @MBIM_MESSAGE_COMMAND_TYPE_UNKNOWN: Unknown type.
 * @MBIM_MESSAGE_COMMAND_TYPE_QUERY: Query command.
 * @MBIM_MESSAGE_COMMAND_TYPE_SET: Set command.
 *
 * Type of command message.
 */
typedef enum {
    MBIM_MESSAGE_COMMAND_TYPE_UNKNOWN = -1,
    MBIM_MESSAGE_COMMAND_TYPE_QUERY   = 0,
    MBIM_MESSAGE_COMMAND_TYPE_SET     = 1
} MbimMessageCommandType;

MbimMessage *mbim_message_command_new    (guint32                transaction_id,
                                          MbimService            service,
                                          guint32                cid,
                                          MbimMessageCommandType command_type);
void         mbim_message_command_append (MbimMessage            *self,
                                          const guint8           *buffer,
                                          guint32                 buffer_size);

MbimService             mbim_message_command_get_service                (const MbimMessage *self);
const MbimUuid         *mbim_message_command_get_service_id             (const MbimMessage *self);
guint32                 mbim_message_command_get_cid                    (const MbimMessage *self);
MbimMessageCommandType  mbim_message_command_get_command_type           (const MbimMessage *self);
const guint8           *mbim_message_command_get_raw_information_buffer (const MbimMessage *self,
                                                                         guint32           *length);

/*****************************************************************************/
/* 'Command Done' message interface */

MbimService      mbim_message_command_done_get_service                (const MbimMessage  *self);
const MbimUuid  *mbim_message_command_done_get_service_id             (const MbimMessage  *self);
guint32          mbim_message_command_done_get_cid                    (const MbimMessage  *self);
MbimStatusError  mbim_message_command_done_get_status_code            (const MbimMessage  *self);
gboolean         mbim_message_command_done_get_result                 (const MbimMessage  *self,
                                                                       GError            **error);
const guint8    *mbim_message_command_done_get_raw_information_buffer (const MbimMessage  *self,
                                                                       guint32            *length);

/*****************************************************************************/
/* 'Indicate Status' message interface */

MbimService      mbim_message_indicate_status_get_service                (const MbimMessage  *self);
const MbimUuid  *mbim_message_indicate_status_get_service_id             (const MbimMessage  *self);
guint32          mbim_message_indicate_status_get_cid                    (const MbimMessage  *self);
const guint8    *mbim_message_indicate_status_get_raw_information_buffer (const MbimMessage  *self,
                                                                          guint32            *length);

/*****************************************************************************/
/* Other helpers */

gboolean mbim_message_response_get_result (const MbimMessage  *self,
                                           MbimMessageType     expected,
                                           GError            **error);

G_END_DECLS

#endif /* _LIBMBIM_GLIB_MBIM_MESSAGE_H_ */
