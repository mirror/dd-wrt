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

#include "mbim-compat.h"

/**
 * SECTION: mbim-compat
 * @title: Deprecated API
 * @short_description: Types and functions flagged as deprecated.
 *
 * This section defines types and functions that have been deprecated.
 */

/*****************************************************************************/
/* 'Service Subscriber List' rename to 'Service Subscribe List' */

/**
 * mbim_message_device_service_subscriber_list_set_new:
 * @events_count: the 'EventsCount' field, given as a #guint32.
 * @events: the 'Events' field, given as an array of #MbimEventEntrys.
 * @error: return location for error or %NULL.
 *
 * Create a new request for the 'Device Service Subscribe List' set command in the 'Basic Connect' service.
 *
 * Returns: a newly allocated #MbimMessage, which should be freed with mbim_message_unref().
 *
 * Deprecated:1.8.0: Use mbim_message_device_service_subscribe_list_set_new() instead.
 */
MbimMessage *
mbim_message_device_service_subscriber_list_set_new (
    guint32 events_count,
    const MbimEventEntry *const *events,
    GError **error)
{
    return (mbim_message_device_service_subscribe_list_set_new (
                events_count,
                events,
                error));
}

/**
 * mbim_message_device_service_subscriber_list_response_parse:
 * @message: the #MbimMessage.
 * @events_count: return location for a #guint32, or %NULL if the 'EventsCount' field is not needed.
 * @events: return location for a newly allocated array of #MbimEventEntrys, or %NULL if the 'Events' field is not needed. Free the returned value with mbim_event_entry_array_free().
 * @error: return location for error or %NULL.
 *
 * Create a new request for the 'Events' response command in the 'Basic Connect' service.
 *
 * Returns: %TRUE if the message was correctly parsed, %FALSE if @error is set.
 *
 * Deprecated:1.8.0: Use mbim_message_device_service_subscribe_list_response_parse() instead.
 */
gboolean
mbim_message_device_service_subscriber_list_response_parse (
    const MbimMessage *message,
    guint32 *events_count,
    MbimEventEntry ***events,
    GError **error)
{
    return (mbim_message_device_service_subscribe_list_response_parse (
                message,
                events_count,
                events,
                error));
}
