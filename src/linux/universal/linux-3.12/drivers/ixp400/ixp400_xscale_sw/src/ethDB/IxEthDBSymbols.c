/**
 * @file IxEthDBSymbols.c
 *
 * @author Intel Corporation
 * @date 20-Nov-2002
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
#include <IxEthDB.h>
#include <include/IxEthDB_p.h>

/*
EXPORT_SYMBOL(ixEthDBFilteringStaticEntryProvision);
EXPORT_SYMBOL(ixEthDBFilteringDynamicEntryProvision);
EXPORT_SYMBOL(ixEthDBFilteringEntryDelete);
EXPORT_SYMBOL(ixEthDBFilteringPortSearch);
EXPORT_SYMBOL(ixEthDBFilteringDatabaseSearch);
EXPORT_SYMBOL(ixEthDBFilteringPortUpdatingSearch);*/
EXPORT_SYMBOL(ixEthDBPortAgingDisable);
EXPORT_SYMBOL(ixEthDBPortAgingEnable);
EXPORT_SYMBOL(ixEthDBDatabaseMaintenance);/*
EXPORT_SYMBOL(ixEthDBEventProcessorPauseModeSet);*/

EXPORT_SYMBOL(ixEthDBFilteringPortMaximumFrameSizeSet);/*
EXPORT_SYMBOL(ixEthDBFilteringPortMaximumTxFrameSizeSet);
EXPORT_SYMBOL(ixEthDBFilteringPortMaximumRxFrameSizeSet);

EXPORT_SYMBOL(ixEthDBInit);*/
EXPORT_SYMBOL(ixEthDBUnload);/*
EXPORT_SYMBOL(ixEthDBPortInit);
EXPORT_SYMBOL(ixEthDBPortEnable);
EXPORT_SYMBOL(ixEthDBPortDisable);
EXPORT_SYMBOL(ixEthDBPortAddressSet);

EXPORT_SYMBOL(ixEthDBFilteringDatabaseShowRecords);

EXPORT_SYMBOL(ixEthDBPortDependencyMapSet);
EXPORT_SYMBOL(ixEthDBPortDependencyMapGet);

EXPORT_SYMBOL(ixEthDBPortVlanTagSet);
EXPORT_SYMBOL(ixEthDBPortVlanTagGet);
EXPORT_SYMBOL(ixEthDBVlanTagSet);
EXPORT_SYMBOL(ixEthDBVlanTagGet);
EXPORT_SYMBOL(ixEthDBPortVlanMembershipAdd);
EXPORT_SYMBOL(ixEthDBPortVlanMembershipRangeAdd);
EXPORT_SYMBOL(ixEthDBPortVlanMembershipRemove);
EXPORT_SYMBOL(ixEthDBPortVlanMembershipRangeRemove);
EXPORT_SYMBOL(ixEthDBPortVlanMembershipSet);
EXPORT_SYMBOL(ixEthDBPortVlanMembershipGet);
EXPORT_SYMBOL(ixEthDBAcceptableFrameTypeSet);
EXPORT_SYMBOL(ixEthDBAcceptableFrameTypeGet);
EXPORT_SYMBOL(ixEthDBPriorityMappingTableSet);
EXPORT_SYMBOL(ixEthDBPriorityMappingTableGet);
EXPORT_SYMBOL(ixEthDBPriorityMappingClassSet);
EXPORT_SYMBOL(ixEthDBPriorityMappingClassGet);
EXPORT_SYMBOL(ixEthDBPriorityMappingTableUpdate);
EXPORT_SYMBOL(ixEthDBEgressVlanEntryTaggingEnabledSet);
EXPORT_SYMBOL(ixEthDBEgressVlanEntryTaggingEnabledGet);
EXPORT_SYMBOL(ixEthDBEgressVlanRangeTaggingEnabledSet);
EXPORT_SYMBOL(ixEthDBEgressVlanTaggingEnabledSet);
EXPORT_SYMBOL(ixEthDBEgressVlanTaggingEnabledGet);
EXPORT_SYMBOL(ixEthDBIngressVlanTaggingEnabledSet);
EXPORT_SYMBOL(ixEthDBIngressVlanTaggingEnabledGet);
EXPORT_SYMBOL(ixEthDBVlanPortExtractionEnable);

EXPORT_SYMBOL(ixEthDBFeatureCapabilityGet);
EXPORT_SYMBOL(ixEthDBFeatureEnable);
EXPORT_SYMBOL(ixEthDBFeatureStatusGet);
EXPORT_SYMBOL(ixEthDBFeaturePropertyGet);
EXPORT_SYMBOL(ixEthDBFeaturePropertySet);
EXPORT_SYMBOL(ixEthDBFeatureStatesRestore);

EXPORT_SYMBOL(ixEthDBDatabaseClear);

EXPORT_SYMBOL(ixEthDBWiFiStationEntryAdd);
EXPORT_SYMBOL(ixEthDBWiFiAccessPointEntryAdd);
EXPORT_SYMBOL(ixEthDBWiFiEntryRemove);
EXPORT_SYMBOL(ixEthDBWiFiConversionTableDownload);
EXPORT_SYMBOL(ixEthDBWiFiFrameControlSet);
EXPORT_SYMBOL(ixEthDBWiFiDurationIDSet);
EXPORT_SYMBOL(ixEthDBWiFiBSSIDSet);
EXPORT_SYMBOL(ixEthDBWiFiRecordEntryAdd);

EXPORT_SYMBOL(ixEthDBSpanningTreeBlockingStateSet);
EXPORT_SYMBOL(ixEthDBSpanningTreeBlockingStateGet);


EXPORT_SYMBOL(ixEthDBFirewallModeSet);
EXPORT_SYMBOL(ixEthDBFirewallInvalidAddressFilterEnable);
EXPORT_SYMBOL(ixEthDBFirewallEntryAdd);
EXPORT_SYMBOL(ixEthDBFirewallEntryRemove);
EXPORT_SYMBOL(ixEthDBFirewallMaskedEntryAdd);
EXPORT_SYMBOL(ixEthDBFirewallMaskedEntryRemove);
EXPORT_SYMBOL(ixEthDBFirewallTableDownload);

EXPORT_SYMBOL(ixEthDBUserFieldSet);
EXPORT_SYMBOL(ixEthDBUserFieldGet);

EXPORT_SYMBOL(mac2string);
EXPORT_SYMBOL(ixEthDBPortInfo);
*/
#endif /* __linux */
