/** 
 * This file is intended to provide backward 
 * compatibility for main osService/OSSL 
 * APIs. 
 *
 * It shall be phased out gradually and users
 * are strongly recommended to use IX_OSAL API.
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
 */

#ifndef IX_OSAL_BACKWARD_CACHE_MMU_H
#define IX_OSAL_BACKWARD_CACHE_MMU_H

#ifdef IX_OSAL_CACHED
#define IX_ACC_CACHE_ENABLED
#endif

#define IX_XSCALE_CACHE_LINE_SIZE IX_OSAL_CACHE_LINE_SIZE

#define IX_ACC_DRV_DMA_MALLOC(size) IX_OSAL_CACHE_DMA_MALLOC(size)

#define IX_ACC_DRV_DMA_FREE(ptr,size) IX_OSAL_CACHE_DMA_FREE(ptr)

#define IX_MMU_VIRTUAL_TO_PHYSICAL_TRANSLATION(addr) IX_OSAL_MMU_VIRT_TO_PHYS(addr)

#define IX_MMU_PHYSICAL_TO_VIRTUAL_TRANSLATION(addr) IX_OSAL_MMU_PHYS_TO_VIRT(addr)

#define IX_ACC_DATA_CACHE_INVALIDATE(addr,size) IX_OSAL_CACHE_INVALIDATE(addr, size)

#define IX_ACC_DATA_CACHE_FLUSH(addr,size) IX_OSAL_CACHE_FLUSH(addr,size)

#endif /* IX_OSAL_BACKWARD_CACHE_MMU_H */
