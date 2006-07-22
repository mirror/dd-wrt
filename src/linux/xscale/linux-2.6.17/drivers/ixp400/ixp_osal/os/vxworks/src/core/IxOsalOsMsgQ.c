/**
 * @file IxOsalOsMsgQ.c
 *
 * @brief vxWorks-specific message queue implementations 
 *
 * Design Notes:
 *
 * @par
 * IXP400 SW Release version  2.1
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
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
