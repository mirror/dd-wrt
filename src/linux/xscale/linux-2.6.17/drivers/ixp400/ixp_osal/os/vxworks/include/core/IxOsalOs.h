/**
 * @file IxOsalOs.h
 *
 * @brief vxWorks-specific defines 
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

#ifndef IxOsalOs_H
#define IxOsalOs_H

#include <cacheLib.h>
#include <stdio.h>
#include <string.h>


/* vxWorks virt <--> phys address mapping */
#define IX_OSAL_OS_MMU_PHYS_TO_VIRT(addr) (addr)
#define IX_OSAL_OS_MMU_VIRT_TO_PHYS(addr) (addr)


#ifdef IX_OSAL_CACHED
#define IX_OSAL_OS_CACHE_INVALIDATE(addr,size) cacheInvalidate(DATA_CACHE, (void*)addr, size)

#define IX_OSAL_OS_CACHE_FLUSH(addr,size) \
    do { cacheFlush(DATA_CACHE, (void*)addr, size); cachePipeFlush(); } while(0)

/* Cache preload not available*/
#define IX_OSAL_OS_CACHE_PRELOAD(addr,size) {}

#else /* IX_OSAL_CACHED */

#define IX_OSAL_OS_CACHE_INVALIDATE(addr,size) {}
#define IX_OSAL_OS_CACHE_FLUSH(addr,size) {}
#define IX_OSAL_OS_CACHE_PRELOAD(addr,size) {}

#endif /* IX_OSAL_CACHED */

/* 
 * Only available for vxworks, not an OSAL top API yet. 
 */
PUBLIC IX_STATUS ixOsalThreadIdGet (IxOsalThread * ptrTid);

#endif /* IxOsalOs_H */
