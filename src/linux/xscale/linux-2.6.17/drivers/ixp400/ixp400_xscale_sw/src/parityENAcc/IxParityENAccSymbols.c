/**
 * @file IxParityENAccSymbols.c
 *
 * @author Intel Corporation
 * @date 27 Aug 2004
 *
 * @brief  This file declares exported/public symbols for linux kernel 
 * module builds.
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

#ifdef __ixp46X
#ifdef __linux

#include <linux/module.h>
#include "IxParityENAcc.h"

EXPORT_SYMBOL(ixParityENAccInit);
EXPORT_SYMBOL(ixParityENAccCallbackRegister);
EXPORT_SYMBOL(ixParityENAccParityDetectionConfigure);
EXPORT_SYMBOL(ixParityENAccParityDetectionQuery);
EXPORT_SYMBOL(ixParityENAccParityErrorContextGet);
EXPORT_SYMBOL(ixParityENAccParityErrorInterruptClear);
EXPORT_SYMBOL(ixParityENAccStatsGet);
EXPORT_SYMBOL(ixParityENAccStatsShow);
EXPORT_SYMBOL(ixParityENAccStatsReset);

#endif /* end of #ifdef __linux */
#endif /* __ixp46X */
