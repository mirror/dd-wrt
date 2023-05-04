/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2004-2013 Sourcefire, Inc.
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
 
#ifndef __EVENT_QUEUE_H__
#define __EVENT_QUEUE_H__

#include "decode.h"
#include "sfutil/sfeventq.h"
#include "treenodes.h"

#define SNORT_EVENTQ_PRIORITY    1
#define SNORT_EVENTQ_CONTENT_LEN 2

struct _OptTreeNode;

typedef struct _EventQueueConfig
{
    int max_events;
    int log_events;
    int order;
    int process_all_events;

} EventQueueConfig;

typedef struct s_SNORT_EVENTQ_USER
{
    char rule_alert;
    void *pkt;

} SNORT_EVENTQ_USER;

typedef struct _EventNode
{
    unsigned int gid;
    unsigned int sid;
    unsigned int rev;
    unsigned int classification;
    unsigned int priority;
    const char  *msg;
    OptTreeNode *rule_info;
} EventNode;

EventQueueConfig * EventQueueConfigNew(void);
void EventQueueConfigFree(EventQueueConfig *);

void SnortEventqNew(EventQueueConfig *, SF_EVENTQ*[]);
void SnortEventqFree(SF_EVENTQ *[]);

void SnortEventqReset(void);
void SnortEventqResetCounts(void);

int SnortEventqLog(SF_EVENTQ *[], Packet *);
int SnortEventqAdd(uint32_t, uint32_t, uint32_t,
                   uint32_t, uint32_t, const char *, void *);

void SnortEventqPush(void);
void SnortEventqPop(void);

#endif
