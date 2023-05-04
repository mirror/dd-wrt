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
**  @file       hi_eo.h
**
**  @author     Daniel Roelker <droelker@sourcefire.com>
**
**  @brief      Contains the data structures, event types, specific events,
**              and function prototypes for the Event Output Module.
**
**  This file is key to alerting with HttpInspect.  It contains the header
**  file with all the individual alerts.
**
**  The Event Output Module provides a mechanism to queue HttpInspect events
**  and prioritize them.  The Event Output Module does not actually log the
**  events, but tracks them per session/packet.  The user program needs to 
**  do the actual logging of events.
**
**  Each event contains the type of event, the priority of the event, and
**  any data that is associated with the event.
**
**  NOTES:
**    - 3.3.03:  Initial development.  DJR
*/
#ifndef __HI_EO_H__
#define __HI_EO_H__

#include "hi_include.h"
#include "hi_eo_events.h"

/**
**  We hold the type of alert, the priority of the alert
**  and any data associated with this alert.
*/
typedef struct s_HI_EVENT_INFO
{
    int alert_id;               /** the alert id */
    int priority;               /** the alert priority, 0 = highest */
    char *alert_str;            /** the alert string */

} HI_EVENT_INFO;

typedef struct s_HI_EVENT
{
    HI_EVENT_INFO *event_info;
    int  count;                 /** number of times event occurred in session */
    void *data;                 /** generic ptr to data */
    void (*free_data)(void *);  /** function to free data */

} HI_EVENT;

/**
**  This is a generic structure to translate different event types to
**  the same structure.  This helps when logging the different types
**  of events.
*/
typedef struct s_HI_GEN_EVENTS
{
    int *stack;
    int *stack_count;
    HI_EVENT *events;

} HI_GEN_EVENTS;

/**
**  The idea behind this event storage structure is that we use a
**  simple stack to tell us which events we have set, so we don't
**  set an event twice and can access the events very easily.
*/
typedef struct s_HI_CLIENT_EVENTS
{
    int stack[HI_EO_CLIENT_EVENT_NUM];
    int stack_count;
    HI_EVENT events[HI_EO_CLIENT_EVENT_NUM];

} HI_CLIENT_EVENTS;

typedef struct s_HI_ANOM_SERVER_EVENTS
{
    int stack[HI_EO_SERVER_EVENT_NUM];
    int stack_count;
    HI_EVENT events[HI_EO_SERVER_EVENT_NUM];

} HI_ANOM_SERVER_EVENTS;

typedef struct s_HI_SERVER_EVENTS
{
    int stack[HI_EO_SERVER_EVENT_NUM];
    int stack_count;
    HI_EVENT events[HI_EO_SERVER_EVENT_NUM];

} HI_SERVER_EVENTS;

#endif
