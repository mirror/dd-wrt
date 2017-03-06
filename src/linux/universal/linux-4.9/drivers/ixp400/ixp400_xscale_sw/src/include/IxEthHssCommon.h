/**
 * @file IxEthHssCommon.h
 *
 * @date	25 January 2006 
 *
 * @brief   API of the IXP400 HSS Access component 
 *
 *
 * @par
 * IXP400 SW Release version  2.3
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

#ifndef IXETHHSSCOMMON_H
#define IXETHHSSCOMMON_H

#include "IxOsal.h"

/*
 * This variable is the common mutex that is shared between
 * Ethernet & HssAcc services when the co-exist
 */ 
extern IxOsalMutex ixEthHssCoexistLock;

/*
 * This variable is used to indicate that Mutex initialization
 * is done or not
 */
extern  BOOL ixEthHssComMutexInitDone;

/*
 * This variable is used to indicate presence of Ethernet & HssAcc
 * services co existence
 */
extern  BOOL ixEthHssAccCoexistEnable ;


/**
 * @def IX_NPEDL_IMAGEID_FUNCTIONID_OFFSET
 *
 * @brief Offset from LSB of Functionality ID field in Image ID
 */
#define IX_IMAGEID_FUNCTIONID_OFFSET 16

/**
 * @def IX_NPEIMAGE_FIELD_MASK
 *
 * @brief Mask for NPE Image ID's Field
 *
 */
#define IX_NPEIMAGE_FIELD_MASK  0xff

/* macros for common mutex control for ETH-HSS co-existence */
#define IX_ETH_HSS_COM_MUT_LOCK() do{\
    ixOsalMutexLock (&ixEthHssCoexistLock, IX_OSAL_WAIT_FOREVER);\
    } while(0)

#define IX_ETH_HSS_COM_MUT_UNLOCK() do{\
    ixOsalMutexUnlock (&ixEthHssCoexistLock);\
    } while(0)


/**
 * @def IX_FUNCTIONID_FROM_NPEIMAGEID_GET
 *
 * @brief Macro to extract Functionality ID field from Image ID
 */
#define IX_FUNCTIONID_FROM_NPEIMAGEID_GET(imageId) \
    (((imageId) >> IX_IMAGEID_FUNCTIONID_OFFSET) & \
     IX_NPEIMAGE_FIELD_MASK)


#endif  /* IXETHHSSCOMMON_H */

