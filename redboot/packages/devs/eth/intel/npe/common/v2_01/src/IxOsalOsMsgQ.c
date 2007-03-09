/**
 * @file IxOsalOsMsgQ.c (eCos)
 *
 * @brief OS-specific Message Queue implementation.
 * 
 * 
 * @par
 * IXP400 SW Release version 1.5
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright 2002-2004 Intel Corporation All Rights Reserved.
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

#include "IxOsal.h"

/*******************************
 * Public functions
 *******************************/
PUBLIC IX_STATUS
ixOsalMessageQueueCreate (IxOsalMessageQueue * queue,
    UINT32 msgCount, UINT32 msgLen)
{
    diag_printf("%s called\n", __FUNCTION__);
    return IX_FAIL;
}

PUBLIC IX_STATUS
ixOsalMessageQueueDelete (IxOsalMessageQueue * queue)
{
    diag_printf("%s called\n", __FUNCTION__);
    return IX_FAIL;
}

PUBLIC IX_STATUS
ixOsalMessageQueueSend (IxOsalMessageQueue * queue, UINT8 * message)
{
    diag_printf("%s called\n", __FUNCTION__);
    return IX_FAIL;
}

PUBLIC IX_STATUS
ixOsalMessageQueueReceive (IxOsalMessageQueue * queue, UINT8 * message)
{
    diag_printf("%s called\n", __FUNCTION__);
    return IX_FAIL;
}

