/**
 * @file IxAtmdSymbols.c
 *
 * @author Intel Corporation
 * @date 04-Oct-2002
 *
 * @brief This file declares exported symbols for linux kernel module builds.
 *
 * 
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
 */

#ifdef __linux

#include <linux/module.h>
#include <IxAtmdAcc.h>
#include <IxAtmdAccCtrl.h>

EXPORT_SYMBOL(ixAtmdAccRxDispatcherRegister);
EXPORT_SYMBOL(ixAtmdAccRxDispatch);
EXPORT_SYMBOL(ixAtmdAccRxLevelQuery);
EXPORT_SYMBOL(ixAtmdAccRxQueueSizeQuery);
EXPORT_SYMBOL(ixAtmdAccPortTxFreeEntriesQuery);
EXPORT_SYMBOL(ixAtmdAccPortTxCallbackRegister);
EXPORT_SYMBOL(ixAtmdAccPortTxCallbackUnregister);
EXPORT_SYMBOL(ixAtmdAccTxDoneDispatcherUnregister);
EXPORT_SYMBOL(ixAtmdAccUninit);
EXPORT_SYMBOL(ixAtmdAccPortTxScheduledModeEnable);
EXPORT_SYMBOL(ixAtmdAccPortTxProcess);
EXPORT_SYMBOL(ixAtmdAccTxDoneDispatch);
EXPORT_SYMBOL(ixAtmdAccTxDoneLevelQuery);
EXPORT_SYMBOL(ixAtmdAccTxDoneQueueSizeQuery);
EXPORT_SYMBOL(ixAtmdAccTxDoneDispatcherRegister);
EXPORT_SYMBOL(ixAtmdAccUtopiaConfigSet);
EXPORT_SYMBOL(ixAtmdAccUtopiaStatusGet);
EXPORT_SYMBOL(ixAtmdAccPortEnable);
EXPORT_SYMBOL(ixAtmdAccPortDisable);
EXPORT_SYMBOL(ixAtmdAccPortDisableComplete);
EXPORT_SYMBOL(ixAtmdAccInit);
EXPORT_SYMBOL(ixAtmdAccShow);
EXPORT_SYMBOL(ixAtmdAccStatsShow);
EXPORT_SYMBOL(ixAtmdAccStatsReset);
EXPORT_SYMBOL(ixAtmdAccRxVcConnect);
EXPORT_SYMBOL(ixAtmdAccRxVcFreeReplenish);
EXPORT_SYMBOL(ixAtmdAccRxVcFreeLowCallbackRegister);
EXPORT_SYMBOL(ixAtmdAccRxVcFreeEntriesQuery);
EXPORT_SYMBOL(ixAtmdAccRxVcEnable);
EXPORT_SYMBOL(ixAtmdAccRxVcDisable);
EXPORT_SYMBOL(ixAtmdAccRxVcTryDisconnect);
EXPORT_SYMBOL(ixAtmdAccTxVcConnect);
EXPORT_SYMBOL(ixAtmdAccTxVcPduSubmit);
EXPORT_SYMBOL(ixAtmdAccTxVcTryDisconnect);
EXPORT_SYMBOL(ixAtmdAccUtopiaConfigReset);
EXPORT_SYMBOL(ixAtmdAccRxDispatcherUnregister);
EXPORT_SYMBOL(ixAtmdAccPortTxScheduledModeDisable);

#endif /* __linux */
