/**
 * @file IxHssAccSymbols.c
 *
 * @author Intel Corporation
 * @date 04-Oct-2002
 *
 * @brief This file declares exported symbols for linux kernel module builds.
 *
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
#include <IxHssAcc.h>

EXPORT_SYMBOL(ixHssAccPortInit);
EXPORT_SYMBOL(ixHssAccLastErrorRetrievalInitiate);
EXPORT_SYMBOL(ixHssAccInit);
EXPORT_SYMBOL(ixHssAccPktPortConnect);
EXPORT_SYMBOL(ixHssAccPktPortEnable);
EXPORT_SYMBOL(ixHssAccPktPortDisable);
EXPORT_SYMBOL(ixHssAccPktPortDisconnect);
EXPORT_SYMBOL(ixHssAccPktPortIsDisconnectComplete);
EXPORT_SYMBOL(ixHssAccPktPortRxFreeReplenish);
EXPORT_SYMBOL(ixHssAccPktPortTx);
EXPORT_SYMBOL(ixHssAccChanConnect);
EXPORT_SYMBOL(ixHssAccChanPortEnable);
EXPORT_SYMBOL(ixHssAccChanPortDisable);
EXPORT_SYMBOL(ixHssAccChanDisconnect);
EXPORT_SYMBOL(ixHssAccChanStatusQuery);
EXPORT_SYMBOL(ixHssAccShow);
EXPORT_SYMBOL(ixHssAccStatsInit);
EXPORT_SYMBOL(ixHssAccChanTslotSwitchEnable);
EXPORT_SYMBOL(ixHssAccChanTslotSwitchDisable);
EXPORT_SYMBOL(ixHssAccChanTslotSwitchGctDownload);
EXPORT_SYMBOL(ixHssAccUninit);
#endif /* __linux */
