/**
 * @file IxOsBuffPoolMgt.c (Now mapped to OSAL )
 *
 * @date 9 Oct 2002
 *
 * @brief This file contains the mbuf pool management implementation.
 *
 * 
 * @par
 * IXP400 SW Release version  2.0
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright 2002-2005 Intel Corporation All Rights Reserved.
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
 *
 * @sa IxOsBuffPoolMgt.h
 * @sa IxOsBuffMgt.h
 */

#include "IxOsal.h"
#include "IxOsBuffMgt.h"
 
IX_STATUS
ixOsalOsIxp400BackwardPoolInit (IX_OSAL_MBUF_POOL ** poolPtrPtr,
    UINT32 count, UINT32 size, const char *name)
{
    *poolPtrPtr = IX_OSAL_MBUF_POOL_INIT (count, size, name);
    if (*poolPtrPtr == NULL)
    {
        return IX_FAIL;
    }
    else
    {
        return IX_SUCCESS;
    }
}

IX_STATUS
ixOsalOsIxp400BackwardMbufPoolGet (IX_OSAL_MBUF_POOL * poolPtr,
    IX_OSAL_MBUF ** newBufPtrPtr)
{
    *newBufPtrPtr = IX_OSAL_MBUF_POOL_GET (poolPtr);
    if (*newBufPtrPtr == NULL)
    {
        return IX_FAIL;
    }
    else
    {
        return IX_SUCCESS;
    }
}

