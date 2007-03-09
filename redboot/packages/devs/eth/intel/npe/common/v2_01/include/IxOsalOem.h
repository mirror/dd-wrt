/**
 * @file IxOsalIxpOem.h
 *
 * @brief this file contains platform-specific defines.
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

#ifndef IxOsalOem_H
#define IxOsalOem_H

#include "IxOsalTypes.h"

/* OS-specific header for Platform package */
#include "IxOsalOsIxp400.h"

/*
 * Platform Name
 */
#define IX_OSAL_PLATFORM_NAME ixp400

/*
 * Cache line size
 */
#define IX_OSAL_CACHE_LINE_SIZE (32)


/* Platform-specific fastmutex implementation */
PUBLIC IX_STATUS ixOsalOemFastMutexTryLock (IxOsalFastMutex * mutex);

/* Platform-specific init (MemMap) */
PUBLIC IX_STATUS
ixOsalOemInit (void);

/* Platform-specific unload (MemMap) */
PUBLIC void
ixOsalOemUnload (void);

/* Default implementations */

PUBLIC UINT32
ixOsalIxp400SharedTimestampGet (void);


UINT32
ixOsalIxp400SharedTimestampRateGet (void);

UINT32
ixOsalIxp400SharedSysClockRateGet (void);

void
ixOsalIxp400SharedTimeGet (IxOsalTimeval * tv);


INT32
ixOsalIxp400SharedLog (UINT32 level, UINT32 device, char *format, 
                       int arg1, int arg2, int arg3, int arg4, 
                       int arg5, int arg6);

#endif /* IxOsal_Oem_H */
