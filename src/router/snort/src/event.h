/* $Id$ */
/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License Version 2 as
** published by the Free Software Foundation.  You may not use, modify or
** distribute this program under any other version of the GNU General
** Public License.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/*  D E F I N E S  ************************************************************/
#ifndef __EVENT_H__
#define __EVENT_H__

#ifdef OSF1
#include <sys/bitypes.h>
#endif

#include <sys/types.h>

#include "pcap_pkthdr32.h"

#if defined(FEAT_OPEN_APPID)
#define MAX_EVENT_APPNAME_LEN  64
#endif /* defined(FEAT_OPEN_APPID) */
typedef struct _Event
{
    uint32_t sig_generator;   /* which part of snort generated the alert? */
    uint32_t sig_id;          /* sig id for this generator */
    uint32_t sig_rev;         /* sig revision for this id */
    uint32_t classification;  /* event classification */
    uint32_t priority;        /* event priority */
    uint32_t event_id;        /* event ID */
    uint32_t event_reference; /* reference to other events that have gone off,
                                * such as in the case of tagged packets...
                                */
    struct sf_timeval32 ref_time;   /* reference time for the event reference */

#if defined(FEAT_OPEN_APPID)
    char     app_name[MAX_EVENT_APPNAME_LEN];
#endif /* defined(FEAT_OPEN_APPID) */
    /* Don't add to this structure because this is the serialized data
     * struct for unified logging.
     */
} Event;

#if 0
typedef struct _EventID
{
    uint32_t sequence;
    uint32_t seconds;
} EventID;

typedef struct _Event
{
    EventID id;
    uint32_t uSeconds;
    SigInfo sigInfo;
} Event;

#endif


#endif /* __EVENT_H__ */
