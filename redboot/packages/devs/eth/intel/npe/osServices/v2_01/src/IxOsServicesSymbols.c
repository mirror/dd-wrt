/**
 * @file IxOsServicesSymbols.c
 *
 * @author Intel Corporation
 * @date 04-Oct-2002
 *
 * @brief This file declares exported symbols for linux kernel module builds.
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
 */

#ifdef __linux

#include <linux/module.h>
#include "IxOsalBackward.h"

EXPORT_SYMBOL (ixOsServIntBind);
EXPORT_SYMBOL (ixOsServIntUnbind);
EXPORT_SYMBOL (ixOsServIntLock);
EXPORT_SYMBOL (ixOsServIntUnlock);
EXPORT_SYMBOL (ixOsServIntLevelSet);
EXPORT_SYMBOL (ixOsServMutexInit);
EXPORT_SYMBOL (ixOsServMutexLock);
EXPORT_SYMBOL (ixOsServMutexUnlock);
EXPORT_SYMBOL (ixOsServMutexDestroy);
EXPORT_SYMBOL (ixOsServFastMutexInit);
EXPORT_SYMBOL (ixOsServFastMutexTryLock);
EXPORT_SYMBOL (ixOsServFastMutexUnlock);
EXPORT_SYMBOL (ixOsServLog);
EXPORT_SYMBOL (ixOsServLogLevelSet);
EXPORT_SYMBOL (ixOsServSleep);
EXPORT_SYMBOL (ixOsServTaskSleep);
EXPORT_SYMBOL (ixOsServTimestampGet);
EXPORT_SYMBOL (ixOsServUnload);
EXPORT_SYMBOL (ixOsServYield);
EXPORT_SYMBOL (ixOsalOsIxp400BackwardPoolInit);
EXPORT_SYMBOL (ixOsalOsIxp400BackwardMbufPoolGet);

#endif /* __linux */

