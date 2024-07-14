/*
 *  Copyright (c) 2010 Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * 2012 Qualcomm Atheros, Inc.
 */

/*-M- interface --
*/

#ifndef hybridge__h
#define hybridge__h

#include <sys/types.h>
#include "hyfi_api.h"
#include "mc_api2.h"

typedef enum
{
	HYFI_BRIDGE_HA_TABLE,
	HYFI_BRIDGE_HD_TABLE,
	HYFI_BRIDGE_FDB_TABLE,
	HYFI_BRIDGE_MDB_TABLE,
	HYFI_BRIDGE_ACL_TABLE,
	HYFI_BRIDGE_ENCAP_TABLE,
	HYFI_BRIDGE_FLOOD_TABLE,
	HYFI_BRIDGE_RTPORT_TABLE,

	HYFI_BRIDGE_TABLE_LAST

} hybridgeTable_e;

typedef enum
{
    HYFI_BRIDGE_ACTION_GET = 0,
    HYFI_BRIDGE_ACTION_SET

} hybridgeTableAction_e;

typedef struct
{
    const char *interfaceName[ HYFI_AGGR_MAX_IFACE ];
    u_int8_t quota[ HYFI_AGGR_MAX_IFACE ];

} hyfiBridgeAggrEntry_t;

#define MAX_WDSEXT_IFACE 15
struct WdsExt_ifaces {
    char ifname[IFNAMSIZ];
};

struct WdsExt_iflist {
    u_int32_t num_entries;
    struct WdsExt_ifaces iflist[MAX_WDSEXT_IFACE];
};

/* Hybrid message size overhead */
#define HYFI_BRIDGE_MESSAGE_SIZE( x )		( NLMSG_LENGTH(0) + HYFI_MSG_HDRLEN + x )

/* Hybrid bridge table action */
int32_t bridgeTableAction( const char* BridgeName, hybridgeTable_e TableType, int32_t* NumEntries, void* TableEntry, hybridgeTableAction_e TableAction );

/* Get a table from the hybrid bridge */
#define bridgeGetTable( _BridgeName, _TableType, _NumEntries, _TableEntry ) \
    bridgeTableAction( _BridgeName, _TableType, _NumEntries, _TableEntry, HYFI_BRIDGE_ACTION_GET )

/* Set a table in the hybrid bridge */
#define bridgeSetTable( _BridgeName, _TableType, _NumEntries, _TableEntry ) \
    bridgeTableAction( _BridgeName, _TableType, _NumEntries, _TableEntry, HYFI_BRIDGE_ACTION_SET )

/* Allocate table buffer, use with the bridgeGetTable function */
void *bridgeAllocTableBuf( int32_t Size, const char *BridgeName );

/* Free table buffer */
void bridgeFreeTableBuf( void *Buf );

/*-F- bridgeGetLANPortNumber --*/
int32_t bridgeGetLANPortNumber(const char* BridgeName, const char* MAC, u_int32_t vlanid );

/*-F- bridgeGetVersionCompatibility --*/
int32_t bridgeGetVersionCompatibility(const char* BridgeName, char* app_ver);

/*-F- bridgeAttach --
 */
int32_t bridgeAttach(const char* BridgeName);

/*-F- bridgeDetach --
 */
int32_t bridgeDetach(const char* BridgeName);

/*-F- getBridge --
 */
int32_t getBridge(const char* BridgeName);

/*-F- bridgeSetBridgeMode --
 */
int32_t bridgeSetBridgeMode(const char* BridgeName, int32_t Mode);

/*-F- bridgeSetBridgeForwardingMode --
 */
int32_t bridgeSetForwardingMode(const char* BridgeName, int32_t Mode);

/*-F- netlink_msg --
 */
int32_t netlink_msg(int32_t msg_type, u_int8_t *data, int32_t hymsgdatalen, int32_t netlink_key);

/*-F- bridgeSetIFGroup --
 */
int32_t bridgeSetIFGroup(const char* BridgeName, const char* InterfaceName, int32_t GroupID, int32_t GroupType);

#ifndef DISABLE_APS_HOOKS
/*-F- bridgeAddHATableEntries --
 */
int32_t bridgeAddHATableEntries(const char* BridgeName, int32_t Hash,
		u_int8_t* MAC, u_int8_t* ID, const char* InterfaceName, int32_t TrafficClass, u_int32_t Priority  );

/*-F- bridgeSetHATableEntries --
 */
int32_t bridgeSetHATableEntries(const char* BridgeName,int32_t Hash,
		u_int8_t* MAC, const char* InterfaceName, int32_t TrafficClass, u_int32_t Priority);

/*-F- bridgeSetHATableAggrEntry --
 */
int32_t bridgeSetHATableAggrEntry(const char* BridgeName,int32_t Hash,
        u_int8_t* MAC, hyfiBridgeAggrEntry_t *aggrData, int32_t TrafficClass, u_int32_t Priority);

/*
 * bridgeToggleLocalBit
 */
int32_t bridgeToggleLocalBit( const char* BridgeName, const struct __hatbl_entry *entry );

/*-F- bridgeDelHATableEntries --
 */
int32_t bridgeDelHATableEntries(const char* BridgeName,int32_t Hash,
u_int8_t* MAC, int32_t TrafficClass, u_int32_t Priority  );

/*-F- bridgeAddHDTableEntries --
 */
int32_t bridgeAddHDTableEntries(const char* BridgeName, const u_int8_t* MAC,
		const u_int8_t* ID, const char* InterfaceUDP, const char* InterfaceOther, int32_t StaticEntry);

/*-F- bridgeSetHDTableEntries --
 */
int32_t bridgeSetHDTableEntries(const char* BridgeName, u_int8_t* MAC,
		u_int8_t* ID, const char* InterfaceUDP, const char* InterfaceOther);

/*-F- bridgeDelHDTableEntriesByMAC --
 */
int32_t bridgeDelHDTableEntriesByMAC(const char* BridgeName, const u_int8_t* MAC);

/*-F- bridgeDelHDTableEntriesByID --
 */
int32_t bridgeDelHDTableEntriesByID(const char* BridgeName, u_int8_t* ID);

/*-F- bridgeFlushHATable --
 */
int32_t bridgeFlushHATable(const char* BridgeName);

/*-F- bridgeFlushHDTable --
 */
int32_t bridgeFlushHDTable(const char* BridgeName);

/*-F- bridgeSetHATableAgingParams --
 */
int32_t bridgeSetHATableAgingParams(const char* BridgeName, u_int32_t AgingTime  );
#endif

/*-F- bridgeSetIFBroadcast --
 */
int32_t bridgeSetIFBroadcast(const char* BridgeName, const char* InterfaceName, u_int32_t BroadcastEnable );

/*-F- bridgeSetTcpSP --
 */
int32_t bridgeSetTcpSP(const char* BridgeName, u_int32_t TcpSPEnable );

/* Init buffer; to be used by the bridge int32_terface only
 */
void bridgeInitBuf( void *Buf, size_t Size, const char *BridgeName );

/*-F- bridgeSetEnablePathSwitchParams --
 */
int32_t bridgeSetPathSwitchParam(const char* BridgeName, struct __path_switch_param * PathswitchParam);

/*-F- bridgeSetPathSwitchSwitchEndMarkerTimeout --
 */
int32_t bridgeSetPathSwitchSwitchEndMarkerTimeout( const char* BridgeName, u_int32_t to );

/*-F- bridgeSetPathSwitchAdvancedParam --
 */
int32_t bridgeSetPathSwitchAdvancedParam(const char* BridgeName, u_int32_t type, u_int32_t data );

/*-F- bridgeSetIFType --
 */
int32_t bridgeSetIFType(const char* BridgeName, const char* InterfaceName, enum __hyInterfaceType InterfaceType );

/*-F- bridgeSetEventInfo --
 */
int32_t bridgeSetEventInfo(const char* BridgeName, u_int32_t Pid, u_int32_t Cmd, u_int32_t netlinkKey);

/*-F- bridgeSetSnoopingParam --
 */
int32_t bridgeSetSnoopingParam(const char* BridgeName, int Cmd, void *MCParam, u_int32_t ParamLen);

/*-F- bridgeSetSnoopingParam --
 */
int32_t bridgeGetSnoopingParam(const char* BridgeName, int Cmd, void *MCParam, u_int32_t ParamLen);

/*-F- bridgeSetBridgeMapTrafficSeparationMode --
 */
int32_t bridgeSetBridgeMapTrafficSeparationMode(const char* BridgeName, int32_t TSEnabled);

/*-F- bridgeSetServicePriortizationRuleSet
 */
int32_t bridgeSetServicePriortizationRuleSet(const char* BridgeName, struct __sp_rule *rule);

/*-F- bridgeFlushServicePriortizationRules
 */
int32_t bridgeFlushServicePriortizationRules(const char* BridgeName);

/*-F- bridgeSetMSCSRule --
 */
int32_t bridgeSetMSCSRule(const char* BridgeName, struct __mscs_rule* rule);

/*-F- bridgeGetFourAddressInterfaces --
 */
int32_t bridgeGetFourAddressIface(const char* BridgeName, void *iface_list);
#endif //hybridge__h
