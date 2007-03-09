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
#include <IxEthDB.h>
#include <include/IxEthDB_p.h>

EXPORT_SYMBOL(ixEthDBFilteringStaticEntryProvision);
EXPORT_SYMBOL(ixEthDBFilteringDynamicEntryProvision);
EXPORT_SYMBOL(ixEthDBFilteringEntryDelete);
EXPORT_SYMBOL(ixEthDBFilteringPortSearch);
EXPORT_SYMBOL(ixEthDBFilteringDatabaseSearch);
EXPORT_SYMBOL(ixEthDBFilteringPortUpdatingSearch);
EXPORT_SYMBOL(ixEthDBPortAgingDisable);
EXPORT_SYMBOL(ixEthDBPortAgingEnable);
EXPORT_SYMBOL(ixEthDBDatabaseMaintenance);

EXPORT_SYMBOL(ixEthDBFilteringPortMaximumFrameSizeSet);
EXPORT_SYMBOL(ixEthDBFilteringPortMaximumTxFrameSizeSet);
EXPORT_SYMBOL(ixEthDBFilteringPortMaximumRxFrameSizeSet);

EXPORT_SYMBOL(ixEthDBInit);
EXPORT_SYMBOL(ixEthDBUnload);
EXPORT_SYMBOL(ixEthDBPortInit);
EXPORT_SYMBOL(ixEthDBPortEnable);
EXPORT_SYMBOL(ixEthDBPortDisable);
EXPORT_SYMBOL(ixEthDBPortAddressSet);

EXPORT_SYMBOL(ixEthDBFilteringDatabaseShowAll);
EXPORT_SYMBOL(ixEthDBFilteringDatabaseShow);
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

EXPORT_SYMBOL(ixEthDBDatabaseClear);

EXPORT_SYMBOL(ixEthDBWiFiStationEntryAdd);
EXPORT_SYMBOL(ixEthDBWiFiAccessPointEntryAdd);
EXPORT_SYMBOL(ixEthDBWiFiEntryRemove);
EXPORT_SYMBOL(ixEthDBWiFiConversionTableDownload);
EXPORT_SYMBOL(ixEthDBWiFiFrameControlSet);
EXPORT_SYMBOL(ixEthDBWiFiDurationIDSet);
EXPORT_SYMBOL(ixEthDBWiFiBBSIDSet);

EXPORT_SYMBOL(ixEthDBSpanningTreeBlockingStateSet);
EXPORT_SYMBOL(ixEthDBSpanningTreeBlockingStateGet);


EXPORT_SYMBOL(ixEthDBFirewallModeSet);
EXPORT_SYMBOL(ixEthDBFirewallInvalidAddressFilterEnable);
EXPORT_SYMBOL(ixEthDBFirewallEntryAdd);
EXPORT_SYMBOL(ixEthDBFirewallEntryRemove);
EXPORT_SYMBOL(ixEthDBFirewallTableDownload);

EXPORT_SYMBOL(ixEthDBUserFieldSet);
EXPORT_SYMBOL(ixEthDBUserFieldGet);

EXPORT_SYMBOL(mac2string); /* used by codelet display */
EXPORT_SYMBOL(ixEthDBPortInfo); /* used by the integration code */

#endif /* __linux */
