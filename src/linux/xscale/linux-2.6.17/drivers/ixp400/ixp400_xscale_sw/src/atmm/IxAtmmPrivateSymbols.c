/**
 * @file IxAtmmPrivateSymbols.c
 *
 * @author Intel Corporation
 * @date 29-Jul-2003
 *
 * @brief This file declares additional exported symbols for linux kernel module builds.
 *
 * @par 
 *
 * The symbols exported by this file are internal functions, exported
 * only if the macro IX_PRIVATE_OFF is defined. They are exported for
 * Intel testing purposes only.
 *
 * @par
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

#ifdef __linux

#include <linux/module.h>

#ifdef IX_PRIVATE_OFF

/* Macro to provide dummy declarations for the symbols. */
#define DECLARE_AND_EXPORT_SYMBOL(s) extern void s(void);\
                                     EXPORT_SYMBOL(s)

/* These symbols are referenced by the atmd integration test */
DECLARE_AND_EXPORT_SYMBOL(ixAtmmTxLowHandle);
DECLARE_AND_EXPORT_SYMBOL(ixAtmmUtCfg);
DECLARE_AND_EXPORT_SYMBOL(ixAtmmUtopiaInitDone);
DECLARE_AND_EXPORT_SYMBOL(ixAtmmVcDemandUpdate);
DECLARE_AND_EXPORT_SYMBOL(ixAtmmVcIdGet);
DECLARE_AND_EXPORT_SYMBOL(ixAtmmVcQueueClear);

#endif /* IX_PRIVATE_OFF */

#endif /* __linux */
