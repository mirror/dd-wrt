/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2008-2013 Sourcefire, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "util.h"
#include "sfActionQueue.h"
#include "mempool.h"
#include "active.h"
#include "pkt_tracer.h"

tSfActionQueueId sfActionQueueInit(
        int queueLength
        )
{
    tSfActionQueue *queue = SnortAlloc(sizeof(tSfActionQueue));
    if (queue)
    {
        if (mempool_init(&queue->mempool,
                queueLength, sizeof(tSfActionNode)) != 0)
        {
            FatalError("%s(%d) Could not initialize action queue memory pool.\n",
                    __FILE__, __LINE__);
        }
    }

    return queue;
}

int sfActionQueueAdd(
        tSfActionQueueId actionQ,
        void (*callback)(void *),
        void *data
        )
{
    MemBucket *bucket = mempool_alloc(&actionQ->mempool);

    if (bucket != NULL)
    {
        tSfActionNode *node = bucket->data;
        node->callback = callback;
        node->data = data;

        //Using used_list in mempool for tracking allocated MemBucket
        return 0;
    }

    return -1;
}

void sfActionQueueExecAll(
        tSfActionQueueId actionQ
        )
{
    //drain
    while (mempool_numUsedBuckets(&actionQ->mempool))
    {
        sfActionQueueExec(actionQ);
    }
    if (Active_PacketWasDropped() || Active_PacketWouldBeDropped())
    {
        if (pkt_trace_enabled)
            addPktTraceData(VERDICT_REASON_SNORT, snprintf(trace_line, MAX_TRACE_LINE,
                "Snort: processed decoder alerts or actions queue, %s\n", getPktTraceActMsg()));
        else addPktTraceData(VERDICT_REASON_SNORT, 0);
    }
}

void sfActionQueueExec(
        tSfActionQueueId actionQ
        )
{

    MemBucket *firstUsedBucket = mempool_oldestUsedBucket(&actionQ->mempool);

    if (firstUsedBucket)
    {
        tSfActionNode *node = (tSfActionNode *)firstUsedBucket->data;
        (node->callback)(node->data);
        mempool_free(&actionQ->mempool, firstUsedBucket);
    }
}

/**Destroys action queue. All memory allocated by the actionQueue module is
 * freed. Since the queued actions are not executed, any memory freed in the action
 * will be lost. User should do a execAll if there is a potential memory leak
 * or the actions must be completed.
 */
void sfActionQueueDestroy(
        tSfActionQueueId actionQ
        )
{
    mempool_destroy(&actionQ->mempool);
    free(actionQ);
}


