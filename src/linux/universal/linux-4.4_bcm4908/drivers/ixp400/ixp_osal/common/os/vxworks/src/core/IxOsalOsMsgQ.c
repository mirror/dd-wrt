/**
 * @file IxOsalOsMsgQ.c
 *
 * @brief vxWorks-specific message queue implementations 
 *
 * Design Notes:
 *
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
 */

#include <msgQLib.h>

#include "IxOsal.h"

IX_STATUS
ixOsalMessageQueueCreate (IxOsalMessageQueue * queue,
    UINT32 msgCount, UINT32 msgLen)
{
    if (queue == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalMessageQueueCreate():  NULL queue ptr \n", 0, 0, 0, 0, 0,
            0);
        return IX_FAIL;
    }

    if ((msgCount == 0) || (msgLen == 0))
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalMessageQueueCreate():  zero msgCount or msgLen encountered \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    /*
     * Use FIFO by default when creating a new MsgQ
     */
    queue->queueHandle = msgQCreate (msgCount, msgLen, MSG_Q_FIFO);
    queue->msgLen = msgLen;

    if ((queue->queueHandle) == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalMessageQueueCreate():  Fail \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }
    return IX_SUCCESS;
}

IX_STATUS
ixOsalMessageQueueDelete (IxOsalMessageQueue * queue)
{
    if (OK != msgQDelete ((MSG_Q_ID) (queue->queueHandle)))
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalMessageQueueDelete():  Fail \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }
    return IX_SUCCESS;
}

IX_STATUS
ixOsalMessageQueueSend (IxOsalMessageQueue * queue, UINT8 * message)
{
    int retVal = 0;
    retVal = msgQSend (((MSG_Q_ID) (queue->queueHandle)),
        (char *) message, queue->msgLen, NO_WAIT, MSG_PRI_NORMAL);

    if (OK != retVal)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalMessageQueueSend():  Fail \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }
    return IX_SUCCESS;
}

IX_STATUS
ixOsalMessageQueueReceive (IxOsalMessageQueue * queue, UINT8 * message)
{
    int retVal = 0;
    retVal = msgQReceive (((MSG_Q_ID) (queue->queueHandle)),
        (char *) message, (int) queue->msgLen, NO_WAIT);

    if (ERROR == retVal)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalMessageQueueReceive():  Fail \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }
    return IX_SUCCESS;
}
