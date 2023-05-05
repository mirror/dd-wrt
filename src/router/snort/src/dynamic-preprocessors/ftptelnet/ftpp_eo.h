/*
 * ftpp_eo.h
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2004-2013 Sourcefire, Inc.
 * Steven A. Sturges <ssturges@sourcefire.com>
 * Daniel J. Roelker <droelker@sourcefire.com>
 * Marc A. Norton <mnorton@sourcefire.com>
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
 * Description:
 *
 * Contains the data structures, event types, specific events,
 * and function prototypes for the Event Output Module.
 *
 * This file is key to alerting with FTPTelnet.  It contains the header
 * file with all the individual alerts.
 *
 * The Event Output Module provides a mechanism to queue HttpInspect events
 * and prioritize them.  The Event Output Module does not actually log the
 * events, but tracks them per session/packet.  The user program needs to 
 * do the actual logging of events.
 *
 * Each event contains the type of event, the priority of the event, and
 * any data that is associated with the event.
 *
 * NOTES:
 * - 20.09.04:  Initial Development.  SAS
 *
 */
#ifndef __FTPP_EO_H__
#define __FTPP_EO_H__

#include "ftpp_include.h"
#include "ftpp_eo_events.h"

/*
 * We hold the type of alert, the priority of the alert
 * and any data associated with this alert.
 */
typedef struct s_FTPP_EVENT_INFO
{
    int alert_id;               /* the alert id */
    int alert_sid;              /* the unique sid */
    int classification;         /* classification */
    int priority;               /* the alert priority, 0 = highest */
    char *alert_str;            /* the alert string */

} FTPP_EVENT_INFO;

typedef struct s_FTPP_EVENT
{
    FTPP_EVENT_INFO *event_info;
    int  count;                 /* number of times event occurred in session */
    void *data;                 /* generic ptr to data */
    void (*free_data)(void *);  /* function to free data */

} FTPP_EVENT;

/*
 * This is a generic structure to translate different event types to
 * the same structure.  This helps when logging the different types
 * of events.
 */
typedef struct s_FTPP_GEN_EVENTS
{
    int *stack;
    int stack_count;
    FTPP_EVENT *events;

} FTPP_GEN_EVENTS;

/*
 * The idea behind this event storage structure is that we use a
 * simple stack to tell us which events we have set, so we don't
 * set an event twice and can access the events very easily.
 */
typedef struct s_FTP_EVENTS
{
    int stack[FTP_EO_EVENT_NUM];
    int stack_count;
    FTPP_EVENT events[FTP_EO_EVENT_NUM];

} FTP_EVENTS;

/*
 * The idea behind this event storage structure is that we use a
 * simple stack to tell us which events we have set, so we don't
 * set an event twice and can access the events very easily.
 */
typedef struct s_TELNET_EVENTS
{
    int stack[TELNET_EO_EVENT_NUM];
    int stack_count;
    FTPP_EVENT events[TELNET_EO_EVENT_NUM];

} TELNET_EVENTS;
#endif
