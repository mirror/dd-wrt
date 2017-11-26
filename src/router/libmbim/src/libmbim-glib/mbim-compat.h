/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
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
 */

#ifndef _LIBMBIM_GLIB_MBIM_COMPAT_H_
#define _LIBMBIM_GLIB_MBIM_COMPAT_H_

#if !defined (__LIBMBIM_GLIB_H_INSIDE__) && !defined (LIBMBIM_GLIB_COMPILATION)
#error "Only <libmbim-glib.h> can be included directly."
#endif

#include <glib.h>

#include "mbim-basic-connect.h"
#include "mbim-cid.h"

G_BEGIN_DECLS

/*****************************************************************************/
/* Registration flags name fixup */

/**
 * MBIM_REGISTRATION_FLAG_MANUAL_PACKET_SERVICE_AUTOMATIC_ATTACH:
 *
 * Modem should auto-attach to the network after registration.
 *
 * Deprecated:1.8.0: Use MBIM_REGISTRATION_FLAG_PACKET_SERVICE_AUTOMATIC_ATTACH instead.
 */
G_DEPRECATED_FOR (MBIM_REGISTRATION_FLAG_PACKET_SERVICE_AUTOMATIC_ATTACH)
static const int MBIM_DEPRECATED_REGISTRATION_FLAG_MANUAL_PACKET_SERVICE_AUTOMATIC_ATTACH =
    MBIM_REGISTRATION_FLAG_PACKET_SERVICE_AUTOMATIC_ATTACH;
#define MBIM_REGISTRATION_FLAG_MANUAL_PACKET_SERVICE_AUTOMATIC_ATTACH MBIM_DEPRECATED_REGISTRATION_FLAG_MANUAL_PACKET_SERVICE_AUTOMATIC_ATTACH

/*****************************************************************************/
/* 'Service Subscriber List' rename to 'Service Subscribe List' */

/**
 * MBIM_CID_BASIC_CONNECT_DEVICE_SERVICE_SUBSCRIBER_LIST:
 *
 * Device service subscribe list.
 *
 * Deprecated:1.8.0: Use MBIM_CID_BASIC_CONNECT_DEVICE_SERVICE_SUBSCRIBE_LIST instead.
 */
G_DEPRECATED_FOR (MBIM_CID_BASIC_CONNECT_DEVICE_SERVICE_SUBSCRIBE_LIST)
static const int MBIM_DEPRECATED_CID_BASIC_CONNECT_DEVICE_SERVICE_SUBSCRIBER_LIST =
    MBIM_CID_BASIC_CONNECT_DEVICE_SERVICE_SUBSCRIBE_LIST;
#define MBIM_CID_BASIC_CONNECT_DEVICE_SERVICE_SUBSCRIBER_LIST MBIM_DEPRECATED_CID_BASIC_CONNECT_DEVICE_SERVICE_SUBSCRIBER_LIST

G_DEPRECATED_FOR (mbim_message_device_service_subscribe_list_set_new)
MbimMessage *mbim_message_device_service_subscriber_list_set_new (
    guint32 events_count,
    const MbimEventEntry *const *events,
    GError **error);

G_DEPRECATED_FOR (mbim_message_device_service_subscribe_list_response_parse)
gboolean mbim_message_device_service_subscriber_list_response_parse (
    const MbimMessage *message,
    guint32 *events_count,
    MbimEventEntry ***events,
    GError **error);

G_END_DECLS

#endif /* _LIBMBIM_GLIB_MBIM_COMPAT_H_ */
