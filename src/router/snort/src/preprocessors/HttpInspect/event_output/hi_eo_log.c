/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2003-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/

/**
**  @file       hi_eo_log.c
**
**  @author     Daniel Roelker <droelker@sourcefire.com>
**
**  @brief      This file contains the event output functionality that
**              HttpInspect uses to log events and data associated with
**              the events.
**
**  Log events, retrieve events, and select events that HttpInspect
**  generates.
**
**  Logging Events:
**    Since the object behind this is no memset()s, we have to rely on the
**    stack interface to make sure we don't log the same event twice.  So
**    if there are events in the stack we cycle through to make sure that
**    there are none available before we add a new event and increment the
**    stack count.  Then to reset the event queue, we just need to set the
**    stack count back to zero.
**
**  NOTES:
**    - Initial development.  DJR
*/
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "hi_si.h"
#include "hi_eo.h"
#include "hi_util_xmalloc.h"
#include "hi_return_codes.h"

/*
**  The client events and the priorities are listed here.
**  Any time that a new client event is added, we have to
**  add the event id and the priority here.  If you want to
**  change either of those characteristics, you have to change
**  them here.
*/
static HI_EVENT_INFO client_event_info[HI_EO_CLIENT_EVENT_NUM] = {
    { HI_EO_CLIENT_ASCII, HI_EO_LOW_PRIORITY, HI_EO_CLIENT_ASCII_STR },
    { HI_EO_CLIENT_DOUBLE_DECODE, HI_EO_HIGH_PRIORITY,
        HI_EO_CLIENT_DOUBLE_DECODE_STR },
    { HI_EO_CLIENT_U_ENCODE, HI_EO_MED_PRIORITY, HI_EO_CLIENT_U_ENCODE_STR },
    { HI_EO_CLIENT_BARE_BYTE, HI_EO_HIGH_PRIORITY, HI_EO_CLIENT_BARE_BYTE_STR},
    /* Base36 is deprecated - leave here so events keep the same number */
    { HI_EO_CLIENT_BASE36, HI_EO_HIGH_PRIORITY, HI_EO_CLIENT_BASE36_STR },
    { HI_EO_CLIENT_UTF_8, HI_EO_LOW_PRIORITY, HI_EO_CLIENT_UTF_8_STR },
    { HI_EO_CLIENT_IIS_UNICODE, HI_EO_LOW_PRIORITY,
        HI_EO_CLIENT_IIS_UNICODE_STR },
    { HI_EO_CLIENT_MULTI_SLASH, HI_EO_MED_PRIORITY,
        HI_EO_CLIENT_MULTI_SLASH_STR },
    { HI_EO_CLIENT_IIS_BACKSLASH, HI_EO_MED_PRIORITY,
        HI_EO_CLIENT_IIS_BACKSLASH_STR },
    { HI_EO_CLIENT_SELF_DIR_TRAV, HI_EO_HIGH_PRIORITY,
        HI_EO_CLIENT_SELF_DIR_TRAV_STR },
    { HI_EO_CLIENT_DIR_TRAV, HI_EO_LOW_PRIORITY, HI_EO_CLIENT_DIR_TRAV_STR },
    { HI_EO_CLIENT_APACHE_WS, HI_EO_MED_PRIORITY, HI_EO_CLIENT_APACHE_WS_STR },
    { HI_EO_CLIENT_IIS_DELIMITER, HI_EO_MED_PRIORITY,
        HI_EO_CLIENT_IIS_DELIMITER_STR },
    { HI_EO_CLIENT_NON_RFC_CHAR, HI_EO_HIGH_PRIORITY,
        HI_EO_CLIENT_NON_RFC_CHAR_STR },
    { HI_EO_CLIENT_OVERSIZE_DIR, HI_EO_HIGH_PRIORITY,
        HI_EO_CLIENT_OVERSIZE_DIR_STR },
    {HI_EO_CLIENT_LARGE_CHUNK, HI_EO_HIGH_PRIORITY,
        HI_EO_CLIENT_LARGE_CHUNK_STR },
    {HI_EO_CLIENT_PROXY_USE, HI_EO_LOW_PRIORITY,
        HI_EO_CLIENT_PROXY_USE_STR },
    {HI_EO_CLIENT_WEBROOT_DIR, HI_EO_HIGH_PRIORITY,
        HI_EO_CLIENT_WEBROOT_DIR_STR },
    {HI_EO_CLIENT_LONG_HDR, HI_EO_LOW_PRIORITY,
        HI_EO_CLIENT_LONG_HDR_STR},
    {HI_EO_CLIENT_MAX_HEADERS, HI_EO_LOW_PRIORITY,
        HI_EO_CLIENT_MAX_HEADERS_STR},
    {HI_EO_CLIENT_MULTIPLE_CONTLEN, HI_EO_HIGH_PRIORITY,
        HI_EO_CLIENT_MULTIPLE_CONTLEN_STR},
    {HI_EO_CLIENT_CHUNK_SIZE_MISMATCH, HI_EO_HIGH_PRIORITY,
        HI_EO_CLIENT_CHUNK_SIZE_MISMATCH_STR},
    {HI_EO_CLIENT_INVALID_TRUEIP, HI_EO_LOW_PRIORITY,
        HI_EO_CLIENT_INVALID_TRUEIP_STR},
    {HI_EO_CLIENT_MULTIPLE_HOST_HDRS, HI_EO_LOW_PRIORITY,
        HI_EO_CLIENT_MULTIPLE_HOST_HDRS_STR},
    {HI_EO_CLIENT_LONG_HOSTNAME, HI_EO_LOW_PRIORITY,
        HI_EO_CLIENT_LONG_HOSTNAME_STR},
    {HI_EO_CLIENT_EXCEEDS_SPACES, HI_EO_LOW_PRIORITY,
        HI_EO_CLIENT_EXCEEDS_SPACES_STR},
    {HI_EO_CLIENT_CONSECUTIVE_SMALL_CHUNKS, HI_EO_MED_PRIORITY,
        HI_EO_CLIENT_CONSECUTIVE_SMALL_CHUNKS_STR},
    {HI_EO_CLIENT_UNBOUNDED_POST, HI_EO_MED_PRIORITY,
        HI_EO_CLIENT_UNBOUNDED_POST_STR},
    {HI_EO_CLIENT_MULTIPLE_TRUEIP_IN_SESSION, HI_EO_MED_PRIORITY,
        HI_EO_CLIENT_MULTIPLE_TRUEIP_IN_SESSION_STR},
    {HI_EO_CLIENT_BOTH_TRUEIP_XFF_HDRS, HI_EO_LOW_PRIORITY,
        HI_EO_CLIENT_BOTH_TRUEIP_XFF_HDRS_STR},
    {HI_EO_CLIENT_UNKNOWN_METHOD, HI_EO_MED_PRIORITY,
        HI_EO_CLIENT_UNKNOWN_METHOD_STR},
    {HI_EO_CLIENT_SIMPLE_REQUEST, HI_EO_HIGH_PRIORITY,
        HI_EO_CLIENT_SIMPLE_REQUEST_STR},
    {HI_EO_CLIENT_UNESCAPED_SPACE_URI, HI_EO_MED_PRIORITY,
            HI_EO_CLIENT_UNESCAPED_SPACE_URI_STR},
    {HI_EO_CLIENT_PIPELINE_MAX, HI_EO_MED_PRIORITY,
        HI_EO_CLIENT_PIPELINE_MAX_STR},
    {HI_EO_CLIENT_MULTIPLE_COLON_BETN_KEY_VALUE, HI_EO_HIGH_PRIORITY,
        HI_EO_CLIENT_MULTIPLE_COLON_BETN_KEY_VALUE_STR},
    {HI_EO_CLIENT_INVALID_RANGE_UNIT_FMT, HI_EO_MED_PRIORITY,
        HI_EO_CLIENT_INVALID_RANGE_UNIT_FMT_STR},
    {HI_EO_CLIENT_RANGE_NON_GET_METHOD, HI_EO_MED_PRIORITY,
        HI_EO_CLIENT_RANGE_NON_GET_METHOD_STR},
    {HI_EO_CLIENT_RANGE_FIELD_ERROR, HI_EO_MED_PRIORITY,
        HI_EO_CLIENT_RANGE_FIELD_ERROR_STR}
};

static HI_EVENT_INFO server_event_info[HI_EO_SERVER_EVENT_NUM] = {
    {HI_EO_ANOM_SERVER, HI_EO_HIGH_PRIORITY, HI_EO_ANOM_SERVER_STR },
    {HI_EO_SERVER_INVALID_STATCODE, HI_EO_MED_PRIORITY,
                    HI_EO_SERVER_INVALID_STATCODE_STR},
    {HI_EO_SERVER_NO_CONTLEN, HI_EO_MED_PRIORITY,
        HI_EO_SERVER_NO_CONTLEN_STR},
    {HI_EO_SERVER_UTF_NORM_FAIL, HI_EO_MED_PRIORITY,
        HI_EO_SERVER_UTF_NORM_FAIL_STR},
    {HI_EO_SERVER_UTF7, HI_EO_MED_PRIORITY,
        HI_EO_SERVER_UTF7_STR},
    {HI_EO_SERVER_DECOMPR_FAILED, HI_EO_MED_PRIORITY,
        HI_EO_SERVER_DECOMPR_FAILED_STR},
    {HI_EO_SERVER_CONSECUTIVE_SMALL_CHUNKS, HI_EO_MED_PRIORITY,
        HI_EO_SERVER_CONSECUTIVE_SMALL_CHUNKS_STR},
    {HI_EO_CLISRV_MSG_SIZE_EXCEPTION, HI_EO_MED_PRIORITY,
        HI_EO_CLISRV_MSG_SIZE_EXCEPTION_STR},
    {HI_EO_SERVER_JS_OBFUSCATION_EXCD, HI_EO_MED_PRIORITY,
        HI_EO_SERVER_JS_OBFUSCATION_EXCD_STR},
    {HI_EO_SERVER_JS_EXCESS_WS, HI_EO_MED_PRIORITY,
        HI_EO_SERVER_JS_EXCESS_WS_STR},
    {HI_EO_SERVER_MIXED_ENCODINGS, HI_EO_MED_PRIORITY,
        HI_EO_SERVER_MIXED_ENCODINGS_STR},
    {HI_EO_SERVER_SWF_ZLIB_FAILURE, HI_EO_MED_PRIORITY,
        HI_EO_SERVER_SWF_ZLIB_FAILURE_STR},
    {HI_EO_SERVER_SWF_LZMA_FAILURE, HI_EO_MED_PRIORITY,
        HI_EO_SERVER_SWF_LZMA_FAILURE_STR},
    {HI_EO_SERVER_PDF_DEFL_FAILURE, HI_EO_MED_PRIORITY,
        HI_EO_SERVER_PDF_DEFL_FAILURE_STR},
    {HI_EO_SERVER_PDF_UNSUP_COMP_TYPE, HI_EO_MED_PRIORITY,
        HI_EO_SERVER_PDF_UNSUP_COMP_TYPE_STR},
    {HI_EO_SERVER_PDF_CASC_COMP, HI_EO_MED_PRIORITY,
        HI_EO_SERVER_PDF_CASC_COMP_STR},
    {HI_EO_SERVER_PDF_PARSE_FAILURE, HI_EO_MED_PRIORITY,
        HI_EO_SERVER_PDF_PARSE_FAILURE_STR},
    {HI_EO_SERVER_PROTOCOL_OTHER, HI_EO_MED_PRIORITY,
        HI_EO_SERVER_PROTOCOL_OTHER_STR},
    {HI_EO_SERVER_MULTIPLE_CONTLEN, HI_EO_HIGH_PRIORITY,
        HI_EO_SERVER_MULTIPLE_CONTLEN_STR},
    {HI_EO_SERVER_MULTIPLE_CONTENT_ENCODING, HI_EO_HIGH_PRIORITY,
        HI_EO_SERVER_MULTIPLE_CONTENT_ENCODING_STR},
    {HI_EO_SERVER_MULTIPLE_COLON_BETN_KEY_VALUE, HI_EO_HIGH_PRIORITY,
        HI_EO_SERVER_MULTIPLE_COLON_BETN_KEY_VALUE_STR},
    {HI_EO_SERVER_INVALID_CHAR_BETN_KEY_VALUE, HI_EO_HIGH_PRIORITY,
        HI_EO_SERVER_INVALID_CHAR_BETN_KEY_VALUE_STR},
    {HI_EO_CLISRV_INVALID_CHUNKED_ENCODING, HI_EO_HIGH_PRIORITY,
        HI_EO_CLISRV_INVALID_CHUNKED_EXCEPTION_STR},
    {HI_EO_SERVER_PARTIAL_DECOMPRESSION_FAIL, HI_EO_HIGH_PRIORITY,
        HI_EO_SERVER_PARTIAL_DECOMPRESSION_FAIL_STR},
    {HI_EO_SERVER_INVALID_HEADER_FOLDING, HI_EO_HIGH_PRIORITY,
        HI_EO_SERVER_INVALID_HEADER_FOLDING_STR},
    {HI_EO_SERVER_JUNK_LINE_BEFORE_RESP_HEADER,HI_EO_HIGH_PRIORITY,
        HI_EO_SERVER_JUNK_LINE_BEFORE_RESP_HEADER_STR},
    {HI_EO_SERVER_NO_RESP_HEADER_END,HI_EO_HIGH_PRIORITY,
        HI_EO_SERVER_NO_RESP_HEADER_END_STR},
    {HI_EO_SERVER_INVALID_CHUNK_SIZE,HI_EO_HIGH_PRIORITY,
        HI_EO_SERVER_INVALID_CHUNK_SIZE_STR},
    {HI_EO_SERVER_INVALID_VERSION_RESP_HEADER,HI_EO_HIGH_PRIORITY,
        HI_EO_SERVER_INVALID_VERSION_RESP_HEADER_STR},
    {HI_EO_SERVER_INVALID_CONTENT_RANGE_UNIT_FMT, HI_EO_MED_PRIORITY,
        HI_EO_SERVER_INVALID_CONTENT_RANGE_UNIT_FMT_STR},
    {HI_EO_SERVER_RANGE_FIELD_ERROR, HI_EO_MED_PRIORITY,
        HI_EO_SERVER_RANGE_FIELD_ERROR_STR},
    {HI_EO_SERVER_NON_RANGE_GET_PARTIAL_METHOD, HI_EO_MED_PRIORITY,
        HI_EO_SERVER_NON_RANGE_GET_PARTIAL_METHOD_STR}
};

/*
**  hi_eo_anom_server_event_log::
*/
/**
**  This routine logs anomalous server events to the event queue.
**
**  @param Session   pointer to the HttpInspect session
**  @param iEvent    the event id for the client
**  @param data      pointer to the user data of the event
**  @param free_data pointer to a function to free the user data
**
**  @return integer
**
**  @retval HI_SUCCESS function successful
**  @retval HI_INVALID_ARG invalid arguments
*/
int hi_eo_anom_server_event_log(HI_SESSION *Session, int iEvent, void *data,
        void (*free_data)(void *))
{
    HI_ANOM_SERVER_EVENTS *anom_server_events;
    HI_EVENT *event;
    int iCtr;

    /*
    **  Check the input variables for correctness
    */
    if(!Session || (iEvent >= HI_EO_SERVER_EVENT_NUM))
    {
        return HI_INVALID_ARG;
    }

    anom_server_events = &(Session->anom_server.event_list);

    /* this won't happen since iEvent < HI_EO_SERVER_EVENT_NUM and
     * stack_count can at most equal HI_EO_SERVER_EVENT_NUM */
    if (anom_server_events->stack_count > HI_EO_SERVER_EVENT_NUM)
        return HI_INVALID_ARG;

    /*
    **  This is where we cycle through the current event stack.  If the event
    **  to be logged is already in the queue, then we increment the event
    **  count, before returning.  Otherwise, we fall through the loop and
    **  set the event before adding it to the queue and incrementing the
    **  pointer.
    */
    for(iCtr = 0; iCtr < anom_server_events->stack_count; iCtr++)
    {
        if(anom_server_events->stack[iCtr] == iEvent)
        {
            anom_server_events->events[iEvent].count++;
            return HI_SUCCESS;
        }
    }

    /* this won't happen since iEvent will have been found above
     * before this happens */
    if (anom_server_events->stack_count >= HI_EO_SERVER_EVENT_NUM)
        return HI_INVALID_ARG;

    /*
    **  Initialize the event before putting it in the queue.
    */
    event = &(anom_server_events->events[iEvent]);
    event->event_info = &server_event_info[iEvent];
    event->count = 1;
    event->data = data;
    event->free_data = free_data;

    /*
    **  We now add the event to the stack.
    */
    anom_server_events->stack[anom_server_events->stack_count] = iEvent;
    anom_server_events->stack_count++;

    return HI_SUCCESS;
}

/*
**  NAME
**    hi_eo_client_event_log::
*/
/**
**  This function logs client events during HttpInspect processing.
**
**  The idea behind this event logging is modularity, but at the same time
**  performance.  We accomplish this utilizing an optimized stack as an
**  index into the client event array, instead of walking a list for
**  already logged events.  The problem here is that we can't just log
**  every event that we've already seen, because this opens us up to a
**  DOS.  So by using this method, we can quickly check if an event
**  has already been logged and deal appropriately.
**
**  @param Session   pointer to the HttpInspect session
**  @param iEvent    the event id for the client
**  @param data      pointer to the user data of the event
**  @param free_data pointer to a function to free the user data
**
**  @return integer
**
**  @retval HI_SUCCESS function successful
**  @retval HI_INVALID_ARG invalid arguments
*/
int hi_eo_client_event_log(HI_SESSION *Session, int iEvent, void *data,
        void (*free_data)(void *))
{
    HI_CLIENT_EVENTS *client_events;
    HI_EVENT *event;
    int iCtr;

    /*
    **  Check the input variables for correctness
    */
    if(!Session || (iEvent >= HI_EO_CLIENT_EVENT_NUM))
    {
        return HI_INVALID_ARG;
    }

    client_events = &(Session->client.event_list);

    /*
    **  This is where we cycle through the current event stack.  If the event
    **  to be logged is already in the queue, then we increment the event
    **  count, before returning.  Otherwise, we fall through the loop and
    **  set the event before adding it to the queue and incrementing the
    **  pointer.
    */
    for(iCtr = 0; iCtr < client_events->stack_count; iCtr++)
    {
        if(client_events->stack[iCtr] == iEvent)
        {
            client_events->events[iEvent].count++;
            return HI_SUCCESS;
        }
    }

    /*
    **  Initialize the event before putting it in the queue.
    */
    event = &(client_events->events[iEvent]);
    event->event_info = &client_event_info[iEvent];
    event->count = 1;
    event->data = data;
    event->free_data = free_data;

    /*
    **  We now add the event to the stack.
    */
    client_events->stack[client_events->stack_count] = iEvent;
    client_events->stack_count++;

    return HI_SUCCESS;
}

/*
**  NAME
**    hi_eo_server_event_log::
*/
/**
**  This function logs server events during HttpInspect processing.
**
**  The idea behind this event logging is modularity, but at the same time
**  performance.  We accomplish this utilizing an optimized stack as an
**  index into the server event array, instead of walking a list for
**  already logged events.  The problem here is that we can't just log
**  every event that we've already seen, because this opens us up to a
**  DOS.  So by using this method, we can quickly check if an event
**  has already been logged and deal appropriately.
**
**  @param Session   pointer to the HttpInspect session
**  @param iEvent    the event id for the server
**  @param data      pointer to the user data of the event
**  @param free_data pointer to a function to free the user data
**
**  @return integer
**
**  @retval HI_SUCCESS function successful
**  @retval HI_INVALID_ARG invalid arguments
*/
int hi_eo_server_event_log(HI_SESSION *Session, int iEvent, void *data,
        void (*free_data)(void *))
{
    HI_SERVER_EVENTS *server_events;
    HI_EVENT *event;
    int iCtr;

    /*
    **  Check the input variables for correctness
    */
    if(!Session || (iEvent >= HI_EO_SERVER_EVENT_NUM))
    {
        return HI_INVALID_ARG;
    }

    server_events = &(Session->server.event_list);

    /*
    **  This is where we cycle through the current event stack.  If the event
    **  to be logged is already in the queue, then we increment the event
    **  count, before returning.  Otherwise, we fall through the loop and
    **  set the event before adding it to the queue and incrementing the
    **  pointer.
    */
    for(iCtr = 0; iCtr < server_events->stack_count; iCtr++)
    {
        if(server_events->stack[iCtr] == iEvent)
        {
            server_events->events[iEvent].count++;
            return HI_SUCCESS;
        }
    }

    /*
    **  Initialize the event before putting it in the queue.
    */
    event = &(server_events->events[iEvent]);
    event->event_info = &server_event_info[iEvent];
    event->count = 1;
    event->data = data;
    event->free_data = free_data;

    /*
    **  We now add the event to the stack.
    */
    server_events->stack[server_events->stack_count] = iEvent;
    server_events->stack_count++;

    return HI_SUCCESS;
}
