/*
 * Copyright (c) 2014, 2015, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "shell_config.h"
#include "shell_sw.h"


/*cmdline tree descript*/
struct cmd_des_t gcmd_des[] =
{
    /*port ctrl*/
#ifdef IN_PORTCONTROL
    {
        "port", "config port control",
        {
            {"duplex", "get", "get duplex mode of a port", "<port_id>" , SW_API_PT_DUPLEX_GET, NULL},
            {"duplex", "set", "set duplex mode of a port", "<port_id> <half|full>", SW_API_PT_DUPLEX_SET, NULL},
            {"speed", "get", "get speed mode of a port", "<port_id>",  SW_API_PT_SPEED_GET, NULL},
            {"speed", "set", "set speed mode of a port", "<port_id> <10|100|1000>", SW_API_PT_SPEED_SET, NULL},
            {"autoAdv", "get", "get auto-negotiation advertisement of a port", "<port_id>", SW_API_PT_AN_ADV_GET, NULL},
            {"autoAdv", "set", "set auto-negotiation advertisement of a port", "<port_id> <cap_bitmap>", SW_API_PT_AN_ADV_SET, NULL},
            {"autoNeg", "get", "get auto-negotiation status of a port", "<port_id>", SW_API_PT_AN_GET, NULL},
            {"autoNeg", "enable", "enable auto-negotiation of a port", "<port_id>", SW_API_PT_AN_ENABLE, NULL},
            {"autoNeg", "restart", "restart auto-negotiation process of a port", "<port_id>", SW_API_PT_AN_RESTART, NULL},
            {"header", "set", "set atheros header/tag status of a port", "<port_id> <enable|disable>", SW_API_PT_HDR_SET, NULL},
            {"header", "get", "get atheros header/tag status of a port", "<port_id>", SW_API_PT_HDR_GET, NULL},
            {"txhdr", "set", "set tx frame atheros header/tag status of a port", "<port_id> <noheader|onlymanagement|allframe>", SW_API_PT_TXHDR_SET, NULL},
            {"txhdr", "get", "get tx frame atheros header/tag status of a port", "<port_id>", SW_API_PT_TXHDR_GET, NULL},
            {"rxhdr", "set", "set rx frame atheros header/tag status of a port", "<port_id> <noheader|onlymanagement|allframe>", SW_API_PT_RXHDR_SET, NULL},
            {"rxhdr", "get", "get rx frame atheros header/tag status of a port", "<port_id>", SW_API_PT_RXHDR_GET, NULL},
            {"hdrtype", "set", "set atheros header/tag type", "<enable|disable> <type 0x-0xffff>", SW_API_HEADER_TYPE_SET, NULL},
            {"hdrtype", "get", "get atheros header/tag type", "", SW_API_HEADER_TYPE_GET, NULL},
            {"flowCtrl", "set", "set flow control status of a port", "<port_id> <enable|disable>", SW_API_PT_FLOWCTRL_SET, NULL},
            {"flowCtrl", "get", "get flow control status of a port", "<port_id>", SW_API_PT_FLOWCTRL_GET, NULL},
            {"flowCtrlforcemode", "set", "set flow control force mode of a port", "<port_id> <enable|disable>", SW_API_PT_FLOWCTRL_MODE_SET, NULL},
            {"flowCtrlforcemode", "get", "get flow control force mode of a port", "<port_id>", SW_API_PT_FLOWCTRL_MODE_GET, NULL},
            {"powersave", "set", "set powersave status of a port", "<port_id> <enable|disable>", SW_API_PT_POWERSAVE_SET, NULL},
            {"powersave", "get", "get powersave status of a port", "<port_id>", SW_API_PT_POWERSAVE_GET, NULL},
            {"hibernate", "set", "set hibernate status of a port", "<port_id> <enable|disable>", SW_API_PT_HIBERNATE_SET, NULL},
            {"hibernate", "get", "get hibernate status of a port", "<port_id>", SW_API_PT_HIBERNATE_GET, NULL},
            {"cdt", "run", "run cable diagnostic test of a port", "<port_id> <mdi_pair>", SW_API_PT_CDT, NULL},
            {"txmacstatus", "set", "set txmac status of a port", "<port_id> <enable|disable>", SW_API_TXMAC_STATUS_SET, NULL},
            {"txmacstatus", "get", "get txmac status of a port", "<port_id>", SW_API_TXMAC_STATUS_GET, NULL},
            {"rxmacstatus", "set", "set rxmac status of a port", "<port_id> <enable|disable>", SW_API_RXMAC_STATUS_SET, NULL},
            {"rxmacstatus", "get", "get rxmac status of a port", "<port_id>", SW_API_RXMAC_STATUS_GET, NULL},
            {"txfcstatus", "set", "set tx flow control status of a port", "<port_id> <enable|disable>", SW_API_TXFC_STATUS_SET, NULL},
            {"txfcstatus", "get", "get tx flow control status of a port", "<port_id>", SW_API_TXFC_STATUS_GET, NULL},
            {"rxfcstatus", "set", "set rx flow control status of a port", "<port_id> <enable|disable>", SW_API_RXFC_STATUS_SET, NULL},
            {"rxfcstatus", "get", "get rx flow control status of a port", "<port_id>", SW_API_RXFC_STATUS_GET, NULL},
            {"bpstatus", "set", "set back pressure status of a port", "<port_id> <enable|disable>", SW_API_BP_STATUS_SET, NULL},
            {"bpstatus", "get", "get back pressure status of a port", "<port_id>", SW_API_BP_STATUS_GET, NULL},
            {"linkforcemode", "set", "set link force mode of a port", "<port_id> <enable|disable>", SW_API_PT_LINK_MODE_SET, NULL},
            {"linkforcemode", "get", "get link force mode of a port", "<port_id>", SW_API_PT_LINK_MODE_GET, NULL},
            {"linkstatus", "get", "get link status of a port", "<port_id>", SW_API_PT_LINK_STATUS_GET, NULL},
            {"macLoopback", "set", "set mac level loop back mode of port", "<port_id> <enable|disable>", SW_API_PT_MAC_LOOPBACK_SET, NULL},
            {"macLoopback", "get", "get mac level loop back mode of port", "<port_id>", SW_API_PT_MAC_LOOPBACK_GET, NULL},
            {"ptslinkstatus", "get", "get link status of all ports", "", SW_API_PTS_LINK_STATUS_GET, NULL},
			{"congedrop", "set", "set congestion drop of port queue", "<port_id> <queue_id> <enable|disable>", SW_API_PT_CONGESTION_DROP_SET, NULL},
			{"congedrop", "get", "get congestion drop of port queue", "<port_id> <queue_id>", SW_API_PT_CONGESTION_DROP_GET, NULL},
			{"ringfcthres", "set", "set ring flow ctrl of ring", "<ring_id> <on_thres> <off_thres>", SW_API_PT_RING_FLOW_CTRL_THRES_SET, NULL},
			{"ringfcthres", "get", "get ring flow ctrl of ring", "<ring_id>", SW_API_PT_RING_FLOW_CTRL_THRES_GET, NULL},
            {"Ieee8023az", "set", "set 8023az status of a port", "<port_id> <enable|disable>", SW_API_PT_8023AZ_SET, NULL},
            {"Ieee8023az", "get", "get 8023az status of a port", "<port_id>", SW_API_PT_8023AZ_GET, NULL},
            {"crossover", "set", "set crossover mode of a port", "<port_id> <auto|mdi|mdix>", SW_API_PT_MDIX_SET, NULL},
            {"crossover", "get", "get crossover mode of a port", "<port_id>", SW_API_PT_MDIX_GET, NULL},
            {"crossover", "status", "get current crossover status of a port", "<port_id>", SW_API_PT_MDIX_STATUS_GET, NULL},
            {"preferMedium", "set", "set prefer medium of a combo port", "<port_id> <copper|fiber>", SW_API_PT_COMBO_PREFER_MEDIUM_SET, NULL},
            {"preferMedium", "get", "get prefer medium of a combo port", "<port_id>", SW_API_PT_COMBO_PREFER_MEDIUM_GET, NULL},
            {"mediumType", "get", "get current medium status of a combo port", "<port_id>", SW_API_PT_COMBO_MEDIUM_STATUS_GET, NULL},
            {"fiberMode", "set", "set fiber mode of a combo fiber port", "<port_id> <100fx|1000bx>", SW_API_PT_COMBO_FIBER_MODE_SET, NULL},
            {"fiberMode", "get", "get fiber mode of a combo fiber port", "<port_id>", SW_API_PT_COMBO_FIBER_MODE_GET, NULL},
            {"localLoopback", "set", "set local loopback of a port", "<port_id> <enable|disable>", SW_API_PT_LOCAL_LOOPBACK_SET, NULL},
            {"localLoopback", "get", "get local loopback of a port", "<port_id>", SW_API_PT_LOCAL_LOOPBACK_GET, NULL},
            {"remoteLoopback", "set", "set remote loopback of a port", "<port_id> <enable|disable>", SW_API_PT_REMOTE_LOOPBACK_SET, NULL},
            {"remoteLoopback", "get", "get remote loopback of a port", "<port_id>", SW_API_PT_REMOTE_LOOPBACK_GET, NULL},
            {"reset", "set", "reset phy of a port", "<port_id>", SW_API_PT_RESET, NULL},
            {"poweroff", "set", "power off phy of a port", "<port_id>", SW_API_PT_POWER_OFF, NULL},
            {"poweron", "set", "power on phy of a port", "<port_id>", SW_API_PT_POWER_ON, NULL},
            {"magicFrameMac", "set", "set magic frame mac address  of a port", "<port_id> <mac_address>", SW_API_PT_MAGIC_FRAME_MAC_SET, NULL},
            {"magicFrameMac", "get", "get magic frame mac address  of a port", "<port_id>", SW_API_PT_MAGIC_FRAME_MAC_GET, NULL},
            {"phyId", "get", "get phy id of a port", "<port_id>", SW_API_PT_PHY_ID_GET, NULL},
            {"wolstatus", "set", "set wol status of a port", "<port_id> <enable|disable>", SW_API_PT_WOL_STATUS_SET, NULL},
            {"wolstatus", "get", "get wol status of a port", "<port_id>", SW_API_PT_WOL_STATUS_GET, NULL},
            {"interfaceMode", "set", "set interface mode of phy", "<port_id> <psgmii_baset|psgmii_bx1000|psgmii_fx100|psgmii_amdet|sgmii_baset>", SW_API_PT_INTERFACE_MODE_SET, NULL},
            {"interfaceMode", "get", "get interface mode of phy", "<port_id>", SW_API_PT_INTERFACE_MODE_GET, NULL},
            {"interfaceMode", "status", "get current interface mode of phy", "<port_id>", SW_API_PT_INTERFACE_MODE_STATUS_GET, NULL},
	        {"counter", "set", "set counter status of a port", "<port_id> <enable|disable>", SW_API_PT_COUNTER_SET, NULL},
	        {"counter", "get", "get counter status of a port", "<port_id>", SW_API_PT_COUNTER_GET, NULL},
	        {"counter", "show", "show counter statistics of a port", "<port_id>", SW_API_PT_COUNTER_SHOW, NULL},
            {NULL, NULL, NULL, NULL, (int)NULL, NULL},/*end of desc*/
        },
    },
#endif

    /*vlan*/
#ifdef IN_VLAN
    {
        "vlan", "config VLAN table",
        {
            {"entry", "create", "create a VLAN entry", "<vlan_id>", SW_API_VLAN_ADD, NULL},
            {"entry", "del", "delete a VLAN entryn", "<vlan_id>", SW_API_VLAN_DEL, NULL},
            {"entry", "update", "update port member of a VLAN entry", "<vlan_id> <member_bitmap> <0>", SW_API_VLAN_MEM_UPDATE, NULL},
            {"entry", "find", "find a VLAN entry by VLAN id", "<vlan_id>", SW_API_VLAN_FIND, NULL},
            {"entry", "next", "find next VLAN entry by VLAN id", "<vlan_id>",SW_API_VLAN_NEXT, NULL},
            {"entry", "append", "append a VLAN entry", "", SW_API_VLAN_APPEND, NULL},
            {"entry", "flush", "flush all VLAN entries", "",SW_API_VLAN_FLUSH, NULL},
            {"entry", "show", "show whole VLAN entries", "", SW_CMD_VLAN_SHOW, cmd_show_vlan},
            {"fid", "set", "set VLAN entry fid", "<vlan_id> <fid>",SW_API_VLAN_FID_SET, NULL},
            {"fid", "get", "get VLAN entry fid", "<vlan_id>",SW_API_VLAN_FID_GET, NULL},
            {"member", "add", "add VLAN entry member", "<vlan_id> <port_id> <unmodified|untagged|tagged>",SW_API_VLAN_MEMBER_ADD, NULL},
            {"member", "del", "del VLAN entry member", "<vlan_id> <port_id>",SW_API_VLAN_MEMBER_DEL, NULL},
            {"learnsts", "set", "set VLAN entry learn status", "<vlan_id> <enable|disable>",SW_API_VLAN_LEARN_STATE_SET, NULL},
            {"learnsts", "get", "get VLAN entry learn status", "<vlan_id>",SW_API_VLAN_LEARN_STATE_GET, NULL},
            {NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/
        },
    },
#endif

    /*portvlan*/
#ifdef IN_PORTVLAN
    {
        "portVlan", "config port base VLAN",
        {
            {"ingress", "get", "get ingress VLAN mode of a port", "<port_id>", SW_API_PT_ING_MODE_GET, NULL},
            {"ingress", "set", "set ingress VLAN mode of a port", "<port_id> <disable|secure|check|fallback>", SW_API_PT_ING_MODE_SET, NULL},
            {"egress", "get", "get egress VLAN mode of a port", "<port_id>", SW_API_PT_EG_MODE_GET, NULL},
            {"egress", "set", "set egress VLAN mode of a port", "<port_id> <unmodified|untagged|tagged|hybrid|untouched>", SW_API_PT_EG_MODE_SET, NULL},
            {"member", "add", "add a member to the port based VLAN of a port", "<port_id> <memport_id>", SW_API_PT_VLAN_MEM_ADD, NULL},
            {"member", "del", "delete a member from the port based VLAN of a port", "<port_id> <memport_id>", SW_API_PT_VLAN_MEM_DEL, NULL},
            {"member", "update", "update members of the port based VLAN of a port", "<port_id> <port_bitmap>", SW_API_PT_VLAN_MEM_UPDATE, NULL},
            {"member", "get", "get members of the port based VLAN of a port", "<port_id>", SW_API_PT_VLAN_MEM_GET, NULL},
            {"defaultVid", "get", "get default VLAN id of a port", "<port_id>", SW_API_PT_DEF_VID_GET, NULL},
            {"defaultVid", "set", "set default VLAN id of a port", "<port_id> <vid>", SW_API_PT_DEF_VID_SET, NULL},
            {"forceVid", "set", "set VLAN id enforcement status of a port", "<port_id> <enable|disable>", SW_API_PT_FORCE_DEF_VID_SET, NULL},
            {"forceVid", "get", "get VLAN id enforcement status of a port", "<port_id>", SW_API_PT_FORCE_DEF_VID_GET, NULL},
            {"forceMode", "set", "set port based VLAN enforcement status of a port", "<port_id> <enable|disable>", SW_API_PT_FORCE_PORTVLAN_SET, NULL},
            {"forceMode", "get", "get port based VLAN enforcement status of a port", "<port_id>", SW_API_PT_FORCE_PORTVLAN_GET, NULL},
            {"nestVlan", "set", "set nest VLAN status of a port", "<port_id> <enable|disable>", SW_API_PT_NESTVLAN_SET, NULL},
            {"nestVlan", "get", "get nest VLAN status of a port", "<port_id>", SW_API_PT_NESTVLAN_GET, NULL},
            {"sVlanTPID", "set", "set service VLAN tpid", "<tpid>", SW_API_NESTVLAN_TPID_SET, NULL},
            {"sVlanTPID", "get", "get service VLAN tpid", "", SW_API_NESTVLAN_TPID_GET, NULL},
            /*shiva*/
            {"invlan", "set", "set port invlan mode", "<port_id> <admit_all|admit_tagged|admit_untagged>", SW_API_PT_IN_VLAN_MODE_SET, NULL},
            {"invlan", "get", "get port invlan mode", "<port_id>", SW_API_PT_IN_VLAN_MODE_GET, NULL},
            {"tlsMode", "set", "set TLS mode", "<port_id> <enable|disable>", SW_API_PT_TLS_SET, NULL},
            {"tlsMode", "get", "get TLS mode", "<port_id>", SW_API_PT_TLS_GET, NULL},
            {"priPropagation", "set", "set priority propagation", "<port_id> <enable|disable>", SW_API_PT_PRI_PROPAGATION_SET, NULL},
            {"priPropagation", "get", "get priority propagation", "<port_id>", SW_API_PT_PRI_PROPAGATION_GET, NULL},
            {"defaultSVid", "set", "set default SVID", "<port_id> <vlan_id>", SW_API_PT_DEF_SVID_SET, NULL},
            {"defaultSVid", "get", "get default SVID", "<port_id>", SW_API_PT_DEF_SVID_GET, NULL},
            {"defaultCVid", "set", "set default CVID", "<port_id> <vlan_id>", SW_API_PT_DEF_CVID_SET, NULL},
            {"defaultCVid", "get", "get default CVID", "<port_id>", SW_API_PT_DEF_CVID_GET, NULL},
            {"vlanPropagation", "set", "set vlan propagation", "<port_id> <disable|clone|replace>", SW_API_PT_VLAN_PROPAGATION_SET, NULL},
            {"vlanPropagation", "get", "get vlan propagation", "<port_id>", SW_API_PT_VLAN_PROPAGATION_GET, NULL},
            {"translation", "add", "add vlan translation", "<port_id>", SW_API_PT_VLAN_TRANS_ADD, NULL},
            {"translation", "del", "del vlan translation", "<port_id>", SW_API_PT_VLAN_TRANS_DEL, NULL},
            {"translation", "get", "get vlan translation", "<port_id>", SW_API_PT_VLAN_TRANS_GET, NULL},
            {"translation", "iterate", "iterate vlan translation tables", "<port_id> <iterator>", SW_API_PT_VLAN_TRANS_ITERATE, NULL},
            {"qinqMode", "set", "set qinq mode", "<ctag|stag>", SW_API_QINQ_MODE_SET, NULL},
            {"qinqMode", "get", "get qinq mode", "", SW_API_QINQ_MODE_GET, NULL},
            {"qinqRole", "set", "set qinq role", "<port_id> <edge|core>", SW_API_PT_QINQ_ROLE_SET, NULL},
            {"qinqRole", "get", "get qinq role", "<port_id>", SW_API_PT_QINQ_ROLE_GET, NULL},
            {"macvlanxlt", "set", "set mac vlan xlt status", "<port_id> <enable|disable>", SW_API_PT_MAC_VLAN_XLT_SET, NULL},
            {"macvlanxlt", "get", "set mac vlan xlt status", "<port_id>", SW_API_PT_MAC_VLAN_XLT_GET, NULL},
            {"netiso", "set", "enable public/private net isolate", "<enable|disable>", SW_API_NETISOLATE_SET, NULL},
            {"netiso", "get", "get public/private net isolate status", "", SW_API_NETISOLATE_GET, NULL},
            {"egbypass", "set", "enable egress translation filter bypass", "<enable|disable>", SW_API_EG_FLTR_BYPASS_EN_SET, NULL},
            {"egbypass", "get", "get the status of egress translation filter bypass", "", SW_API_EG_FLTR_BYPASS_EN_GET, NULL},
            {"ptvrfid", "set", "set port VRF ID", "<port_id> <vrf_id>", SW_API_PT_VRF_ID_SET, NULL},
            {"ptvrfid", "get", "get port VRF ID", "<port_id>", SW_API_PT_VRF_ID_GET, NULL},
            {NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/
        },
    },
#endif

    /*fdb*/
#ifdef IN_FDB
    {
        "fdb", "config FDB table",
        {
            {"entry", "add", "add a FDB entry", "", SW_API_FDB_ADD, NULL},
            {"entry", "del", "delete a FDB entry", "", SW_API_FDB_DELMAC, NULL},
            {"entry", "flush", "flush all FDB entries", "<0:dynamic only|1:dynamic and static>", SW_API_FDB_DELALL, NULL},
            {"entry", "show", "show whole FDB entries", "", SW_CMD_FDB_SHOW, cmd_show_fdb},
            {"entry", "find", "find a FDB entry", "", SW_API_FDB_FIND, NULL},
            {"entry", "iterate", "iterate all FDB entries", "<iterator>", SW_API_FDB_ITERATE, NULL},
            {"entry", "extendnext", "find next FDB entry in extend mode", "", SW_API_FDB_EXTEND_NEXT, NULL},
            {"entry", "extendfirst", "find first FDB entry in extend mode", "", SW_API_FDB_EXTEND_FIRST, NULL},
            {"entry", "transfer", "transfer port info in FDB entry", "<old port_id> <new port_id> <fid>", SW_API_FDB_TRANSFER, NULL},
            {"portEntry", "flush", "flush all FDB entries by a port", "<port_id> <0:dynamic only|1:dynamic and static>", SW_API_FDB_DELPORT, NULL},
            {"firstEntry", "find", "find the first FDB entry", "", SW_API_FDB_FIRST, NULL},
            {"nextEntry", "find", "find next FDB entry", "", SW_API_FDB_NEXT, NULL},
            {"portLearn", "set", "set FDB entry learning status of a port", "<port_id> <enable|disable>", SW_API_FDB_PT_LEARN_SET, NULL},
            {"portLearn", "get", "get FDB entry learning status of a port", "<port_id>", SW_API_FDB_PT_LEARN_GET, NULL},
            {"ageCtrl", "set", "set FDB entry aging status", "<enable|disable>", SW_API_FDB_AGE_CTRL_SET, NULL},
            {"ageCtrl", "get", "get FDB entry aging status", "", SW_API_FDB_AGE_CTRL_GET, NULL},
            {"vlansmode", "set", "set FDB vlan search mode", "<ivl|svl>", SW_API_FDB_VLAN_IVL_SVL_SET, NULL},
            {"vlansmode", "get", "get FDB vlan search mode", "", SW_API_FDB_VLAN_IVL_SVL_GET, NULL},
            {"ageTime", "set", "set FDB entry aging time", "<time:s>", SW_API_FDB_AGE_TIME_SET, NULL},
            {"ageTime", "get", "get FDB entry aging time", "", SW_API_FDB_AGE_TIME_GET, NULL},
            {"ptlearnlimit", "set", "set port FDB entry learn limit", "<port_id> <enable|disable> <limitcounter>", SW_API_PT_FDB_LEARN_LIMIT_SET, NULL},
            {"ptlearnlimit", "get", "get port FDB entry learn limit", "<port_id>", SW_API_PT_FDB_LEARN_LIMIT_GET, NULL},
            {"ptlearnexceedcmd", "set", "set port forwarding cmd when exceed learn limit", "<port_id> <forward|drop|cpycpu|rdtcpu>", SW_API_PT_FDB_LEARN_EXCEED_CMD_SET, NULL},
            {"ptlearnexceedcmd", "get", "get port forwarding cmd when exceed learn limit", "<port_id>", SW_API_PT_FDB_LEARN_EXCEED_CMD_GET, NULL},
            {"learnlimit", "set", "set FDB entry learn limit", "<enable|disable> <limitcounter>", SW_API_FDB_LEARN_LIMIT_SET, NULL},
            {"learnlimit", "get", "get FDB entry learn limit", "", SW_API_FDB_LEARN_LIMIT_GET, NULL},
            {"learnexceedcmd", "set", "set forwarding cmd when exceed learn limit", "<forward|drop|cpycpu|rdtcpu>", SW_API_FDB_LEARN_EXCEED_CMD_SET, NULL},
            {"learnexceedcmd", "get", "get forwarding cmd when exceed learn limit", "", SW_API_FDB_LEARN_EXCEED_CMD_GET, NULL},
            {"resventry", "add", "add a reserve FDB entry", "", SW_API_FDB_RESV_ADD, NULL},
            {"resventry", "del", "delete reserve a FDB entry", "", SW_API_FDB_RESV_DEL, NULL},
            {"resventry", "find", "find a reserve FDB entry", "", SW_API_FDB_RESV_FIND, NULL},
            {"resventry", "iterate", "iterate all reserve FDB entries", "<iterator>", SW_API_FDB_RESV_ITERATE, NULL},
            {"resventry", "show", "show whole resv FDB entries", "", SW_CMD_RESV_FDB_SHOW, cmd_show_resv_fdb},
            {"ptLearnstatic", "set", "set FDB entry learning static status of a port", "<port_id> <enable|disable>", SW_API_FDB_PT_LEARN_STATIC_SET, NULL},
            {"ptLearnStatic", "get", "get FDB entry learning static status of a port", "<port_id>", SW_API_FDB_PT_LEARN_STATIC_GET, NULL},
            {"port", "add", "add one port to a FDB entry", "<fid> <macaddr> <port_id>", SW_API_FDB_PORT_ADD, NULL},
            {"port", "del", "del one port from a FDB entry", "<fid> <macaddr> <port_id>", SW_API_FDB_PORT_DEL, NULL},
			{"fdbrfs", "set", "add a FDB rfs", "", SW_API_FDB_RFS_SET, NULL},
            {"fdbrfs", "del", "delete a FDB rfs", "", SW_API_FDB_RFS_DEL, NULL},
            {NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/
        },
    },
#endif

    /*acl*/
#ifdef IN_ACL
    {
        "acl", "config ACL",
        {
            {"list", "create", "create an ACL list", "<list_id> <priority>", SW_API_ACL_LIST_CREAT, NULL},
            {"list", "destroy", "destroy an ACL list", "<list_id>", SW_API_ACL_LIST_DESTROY, NULL},
            {"list", "bind", "bind an ACL list to a port", "<list_id> <0-0:direction> <0-0:objtype> <objindex>", SW_API_ACL_LIST_BIND, NULL},
            {"list", "unbind", "unbind an ACL list from a port", "<list_id> <0-0:direction> <0-0:objtype> <objindex>", SW_API_ACL_LIST_UNBIND, NULL},
            {"rule", "add", "add ACL rules to an ACL list", "<list_id> <rule_id> <rule_nr>", SW_API_ACL_RULE_ADD, NULL},
            {"rule", "del", "delete ACL rules from an ACL list", "<list_id> <rule_id> <rule_nr>", SW_API_ACL_RULE_DELETE, NULL},
            {"rule", "query", "query a ACL rule", "<list_id> <rule_id>", SW_API_ACL_RULE_QUERY, NULL},
            {"rule", "active", "active ACL rules in an ACL list", "<list_id> <rule_id> <rule_nr>", SW_API_ACL_RULE_ACTIVE, NULL},
            {"rule", "deactive", "deactive ACL rules in an ACL list", "<list_id> <rule_id> <rule_nr>", SW_API_ACL_RULE_DEACTIVE, NULL},
            {"srcfiltersts", "set", "set status of ACL rules source filter", "<rule_id> <enable|disable>", SW_API_ACL_RULE_SRC_FILTER_STS_SET, NULL},
            {"srcfiltersts", "get", "get status of ACL rules source filter", "<rule_id>", SW_API_ACL_RULE_SRC_FILTER_STS_GET, NULL},
            {"status", "set", "set status of ACL engine", "<enable|disable>", SW_API_ACL_STATUS_SET, NULL},
            {"status", "get", "get status of ACL engine", "", SW_API_ACL_STATUS_GET, NULL},
            {"udfprofile", "set", "set port udf profile", "<port_id> <l2/l2snap/l3/l3plus/l4> <offset> <length>", SW_API_ACL_PT_UDF_PROFILE_SET, NULL},
            {"udfprofile", "get", "get port udf profile", "<port_id> <l2/l2snap/l3/l3plus/l4>", SW_API_ACL_PT_UDF_PROFILE_GET, NULL},
            {NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/
        },
    },
#endif

    /*qos*/
#ifdef IN_QOS
    {
        "qos", "config Qos",
        {
            {"schMode", "set", "set traffic scheduling mode", "<sp|wrr|mix|mix_plus> <q0,q1,q3,q4>", SW_API_QOS_SCH_MODE_SET, NULL},
            {"schMode", "get", "get traffic scheduling mode", "", SW_API_QOS_SCH_MODE_GET, NULL},
            {"qTxBufSts", "set", "set queue tx buffer counting status of a port", "<port_id> <enable|disable>", SW_API_QOS_QU_TX_BUF_ST_SET, NULL},
            {"qTxBufSts", "get", "get queue tx buffer counting status of a port", "<port_id>", SW_API_QOS_QU_TX_BUF_ST_GET, NULL},
            {"qTxBufNr", "set", "set queue tx buffer number", "<port_id> <queueid:0-3> <number:0-120>", SW_API_QOS_QU_TX_BUF_NR_SET, NULL},
            {"qTxBufNr", "get", "get queue tx buffer number", "<port_id> <queueid:0-3>", SW_API_QOS_QU_TX_BUF_NR_GET, NULL},
            {"ptTxBufSts", "set", "set port tx buffer counting status of a port", "<port_id> <enable|disable>", SW_API_QOS_PT_TX_BUF_ST_SET, NULL},
            {"ptTxBufSts", "get", "get port tx buffer counting status of a port", "<port_id>", SW_API_QOS_PT_TX_BUF_ST_GET, NULL},
            {"ptRedEn", "set", "set status of port wred of a port", "<port_id> <enable|disable>", SW_API_QOS_PT_RED_EN_SET, NULL},
            {"ptRedEn", "get", "get status of port wred of a port", "<port_id>", SW_API_QOS_PT_RED_EN_GET, NULL},
            {"ptTxBufNr", "set", "set port tx buffer number", "<port_id> <number:0-504>", SW_API_QOS_PT_TX_BUF_NR_SET, NULL},
            {"ptTxBufNr", "get", "get port tx buffer number", "<port_id>", SW_API_QOS_PT_TX_BUF_NR_GET, NULL},
            {"ptRxBufNr", "set", "set port rx buffer number", "<port_id> <number:0-120>", SW_API_QOS_PT_RX_BUF_NR_SET, NULL},
            {"ptRxBufNr", "get", "get port rx buffer number", "<port_id>", SW_API_QOS_PT_RX_BUF_NR_GET, NULL},
            {"up2q", "set", "set user priority to queue mapping", "<up:0-7> <queueid:0-3>", SW_API_COSMAP_UP_QU_SET, NULL},
            {"up2q", "get", "get user priority to queue mapping", "<up:0-7>", SW_API_COSMAP_UP_QU_GET, NULL},
            {"dscp2q", "set", "set dscp to queue mapping", "<dscp:0-63> <queueid:0-3>", SW_API_COSMAP_DSCP_QU_SET, NULL},
            {"dscp2q", "get", "get dscp to queue mapping", "<dscp:0-63>", SW_API_COSMAP_DSCP_QU_GET, NULL},
            {"ptMode", "set", "set Qos mode of a port", "<port_id> <da|up|dscp| flow> <enable|disable>", SW_API_QOS_PT_MODE_SET, NULL},
            {"ptMode", "get", "get Qos mode of a port", "<port_id> <da|up|dscp| flow>", SW_API_QOS_PT_MODE_GET, NULL},
            {"ptModePri", "set", "set the priority of Qos modes of a port", "<port_id> <da|up|dscp| flow> <priority:0-3>", SW_API_QOS_PT_MODE_PRI_SET, NULL},
            {"ptModePri", "get", "get the priority of Qos modes of a port", "<port_id> <da|up|dscp| flow>", SW_API_QOS_PT_MODE_PRI_GET, NULL},
            {"ptDefaultUp", "set", "set default user priority for received frames of a port", "<port_id> <up:0-7>", SW_API_QOS_PORT_DEF_UP_SET, NULL},
            {"ptDefaultUp", "get", "get default user priority for received frames of a port", "<port_id>", SW_API_QOS_PORT_DEF_UP_GET, NULL},
            {"ptschMode", "set", "set port traffic scheduling mode", "<port_id> <sp|wrr|mix|mixplus> <q0,q1,q2,q3,q4,q5>", SW_API_QOS_PORT_SCH_MODE_SET, NULL},
            {"ptschMode", "get", "get port traffic scheduling mode", "<port_id>", SW_API_QOS_PORT_SCH_MODE_GET, NULL},
            {"ptDefaultSpri", "set", "set default stag priority for received frames of a port", "<port_id> <spri:0-7>", SW_API_QOS_PT_DEF_SPRI_SET, NULL},
            {"ptDefaultSpri", "get", "get default stag priority for received frames of a port", "<port_id>", SW_API_QOS_PT_DEF_SPRI_GET, NULL},
            {"ptDefaultCpri", "set", "set default ctag priority for received frames of a port", "<port_id> <cpri:0-7>", SW_API_QOS_PT_DEF_CPRI_SET, NULL},
            {"ptDefaultCpri", "get", "get default ctag priority for received frames of a port", "<port_id>", SW_API_QOS_PT_DEF_CPRI_GET, NULL},
            {"ptFSpriSts", "set", "set port force Stag priority status for received frames of a port", "<port_id> <enable|disable>", SW_API_QOS_PT_FORCE_SPRI_ST_SET, NULL},
            {"ptFSpriSts", "get", "get port force Stag priority status for received frames of a port", "<port_id>", SW_API_QOS_PT_FORCE_SPRI_ST_GET, NULL},
            {"ptFCpriSts", "set", "set port force Ctag priority status for received frames of a port", "<port_id> <enable|disable>", SW_API_QOS_PT_FORCE_CPRI_ST_SET, NULL},
            {"ptFCpriSts", "get", "get port force Ctag priority status for received frames of a port", "<port_id>", SW_API_QOS_PT_FORCE_CPRI_ST_GET, NULL},
            {"ptQuRemark", "set", "set egress queue based remark", "<port_id> <queue_id> <table_id> <enable|disable>", SW_API_QOS_QUEUE_REMARK_SET, NULL},
            {"ptQuRemark", "get", "get egress queue based remark", "<port_id> <queue_id>", SW_API_QOS_QUEUE_REMARK_GET, NULL},
            {NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/
        },
    },
#endif

    /*igmp*/
#ifdef IN_IGMP
    {
        "igmp", "config IGMP/MLD",
        {
            {"mode", "set", "set IGMP/MLD snooping status of a port", "<port_id> <enable|disable>", SW_API_PT_IGMPS_MODE_SET, NULL},
            {"mode", "get", "get port IGMP/MLD snooping status", "<port_id>", SW_API_PT_IGMPS_MODE_GET, NULL},
            {"cmd", "set", "set IGMP/MLD frames forwarding command", "<forward|drop|cpycpu|rdtcpu>", SW_API_IGMP_MLD_CMD_SET, NULL},
            {"cmd", "get", "get IGMP/MLD frames forwarding command", "", SW_API_IGMP_MLD_CMD_GET, NULL},
            {"portJoin", "set", "set IGMP/MLD hardware joining status", "<port_id> <enable|disable>", SW_API_IGMP_PT_JOIN_SET, NULL},
            {"portJoin", "get", "get IGMP/MLD hardware joining status", "<port_id>", SW_API_IGMP_PT_JOIN_GET, NULL},
            {"portLeave", "set", "set IGMP/MLD hardware leaving status", "<port_id> <enable|disable>", SW_API_IGMP_PT_LEAVE_SET, NULL},
            {"portLeave", "get", "get IGMP/MLD hardware leaving status", "<port_id>", SW_API_IGMP_PT_LEAVE_GET, NULL},
            {"rp", "set", "set IGMP/MLD router ports", "<port_bit_map>", SW_API_IGMP_RP_SET, NULL},
            {"rp", "get", "get IGMP/MLD router ports", "", SW_API_IGMP_RP_GET, NULL},
            {"createStatus", "set", "set IGMP/MLD ability for creating entry", "<enable|disable>", SW_API_IGMP_ENTRY_CREAT_SET, NULL},
            {"createStatus", "get", "get IGMP/MLD ability for creating entry", "", SW_API_IGMP_ENTRY_CREAT_GET, NULL},
            {"static", "set", "set IGMP/MLD static status for creating entry", "<enable|disable>", SW_API_IGMP_ENTRY_STATIC_SET, NULL},
            {"static", "get", "get IGMP/MLD static status for creating entry", "", SW_API_IGMP_ENTRY_STATIC_GET, NULL},
            {"leaky", "set", "set IGMP/MLD leaky status for creating entry", "<enable|disable>", SW_API_IGMP_ENTRY_LEAKY_SET, NULL},
            {"leaky", "get", "get IGMP/MLD leaky status for creating entry", "", SW_API_IGMP_ENTRY_LEAKY_GET, NULL},
            {"version3", "set", "set IGMP v3/MLD v2 status for creating entry", "<enable|disable>", SW_API_IGMP_ENTRY_V3_SET, NULL},
            {"version3", "get", "get IGMP v3/MLD v2 status for creating entry", "", SW_API_IGMP_ENTRY_V3_GET, NULL},
            {"queue", "set", "set IGMP/MLD queue status for creating entry", "<enable|disable> <queue_id>", SW_API_IGMP_ENTRY_QUEUE_SET, NULL},
            {"queue", "get", "get IGMP/MLD queue status for creating entry", "", SW_API_IGMP_ENTRY_QUEUE_GET, NULL},
            {"ptlearnlimit", "set", "set port Multicast entry learn limit", "<port_id> <enable|disable> <limitcounter>", SW_API_PT_IGMP_LEARN_LIMIT_SET, NULL},
            {"ptlearnlimit", "get", "get port Multicast entry learn limit", "<port_id>", SW_API_PT_IGMP_LEARN_LIMIT_GET, NULL},
            {"ptlearnexceedcmd", "set", "set port forwarding cmd when exceed multicast learn limit", "<port_id> <forward|drop|cpycpu|rdtcpu>", SW_API_PT_IGMP_LEARN_EXCEED_CMD_SET, NULL},
            {"ptlearnexceedcmd", "get", "get port forwarding cmd when exceed multicast learn limit", "<port_id>", SW_API_PT_IGMP_LEARN_EXCEED_CMD_GET, NULL},
            {"multi", "set", "set igmp/mld entry", "<entry>", SW_API_IGMP_SG_ENTRY_SET, NULL},
            {"multi", "clear", "clear igmp/mld entry", "<entry>", SW_API_IGMP_SG_ENTRY_CLEAR, NULL},
            {"multi", "show", "show all igmp/mld entry", "", SW_API_IGMP_SG_ENTRY_SHOW, NULL},
            {NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/
        },
    },
#endif

    /*leaky*/
#ifdef IN_LEAKY
    {
        "leaky", "config leaky",
        {
            {"ucMode", "set", "set unicast packets leaky mode", "<port|fdb>", SW_API_UC_LEAKY_MODE_SET, NULL},
            {"ucMode", "get", "get unicast packets leaky mode", "", SW_API_UC_LEAKY_MODE_GET, NULL},
            {"mcMode", "set", "set multicast packets leaky mode", "<port|fdb>", SW_API_MC_LEAKY_MODE_SET, NULL},
            {"mcMode", "get", "get multicast packets leaky mode", "", SW_API_MC_LEAKY_MODE_GET, NULL},
            {"arpMode", "set", "set arp packets leaky mode", "<port_id> <enable|disable>", SW_API_ARP_LEAKY_MODE_SET, NULL},
            {"arpMode", "get", "get arp packets leaky mode", "<port_id>", SW_API_ARP_LEAKY_MODE_GET, NULL},
            {"ptUcMode", "set", "set unicast packets leaky status of a port", "<port_id> <enable|disable>", SW_API_PT_UC_LEAKY_MODE_SET, NULL},
            {"ptUcMode", "get", "get unicast packets leaky status of a port", "<port_id>", SW_API_PT_UC_LEAKY_MODE_GET, NULL},
            {"ptMcMode", "set", "set multicast packets leaky status of a port", "<port_id> <enable|disable>", SW_API_PT_MC_LEAKY_MODE_SET, NULL},
            {"ptMcMode", "get", "get multicast packets leaky status of a port", "<port_id>", SW_API_PT_MC_LEAKY_MODE_GET, NULL},
            {NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/
        },
    },
#endif

    /*mirror*/
#ifdef IN_MIRROR
    {
        "mirror", "config mirror",
        {
            {"analyPt", "set", "set mirror analysis port", "<port_id>", SW_API_MIRROR_ANALY_PT_SET, NULL},
            {"analyPt", "get", "get mirror analysis port", "", SW_API_MIRROR_ANALY_PT_GET, NULL},
            {"ptIngress", "set", "set ingress mirror status of a port", "<port_id> <enable|disable>", SW_API_MIRROR_IN_PT_SET, NULL},
            {"ptIngress", "get", "get ingress mirror status of a port", "<port_id>", SW_API_MIRROR_IN_PT_GET, NULL},
            {"ptEgress", "set", "set egress mirror status of a port", "<port_id> <enable|disable>", SW_API_MIRROR_EG_PT_SET, NULL},
            {"ptEgress", "get", "get egress mirror status of a port", "<port_id>", SW_API_MIRROR_EG_PT_GET, NULL},
            {NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/
        },
    },
#endif

    /*rate*/
#ifdef IN_RATE
    {
        "rate", "config rate limit",
        {
            {"qEgress", "set", "set egress rate limit of a queue", "<port_id> <queueid:0-3> <speed:(kbps)> <enable|disable>", SW_API_RATE_QU_EGRL_SET, NULL},
            {"qEgress", "get", "get egress rate limit of a queue", "<port_id> <queueid:0-3>", SW_API_RATE_QU_EGRL_GET, NULL},
            {"ptEgress", "set", "set egress rate limit of a port", "<port_id> <speed:(kbps)> <enable|disable>", SW_API_RATE_PT_EGRL_SET, NULL},
            {"ptEgress", "get", "get egress rate limit of a port", "<port_id>", SW_API_RATE_PT_EGRL_GET, NULL},
            {"ptIngress", "set", "set ingress rate limit of a port", "<port_id> <speed:(kbps)> <enable|disable>", SW_API_RATE_PT_INRL_SET, NULL},
            {"ptIngress", "get", "get ingress rate limit of a port", "<port_id>", SW_API_RATE_PT_INRL_GET, NULL},
            {"stormCtrl", "set", "set storm control status of a particular frame type", "<port_id> <unicast|multicast|broadcast> <enable|disable>", SW_API_STORM_CTRL_FRAME_SET, NULL},
            {"stormCtrl", "get", "get storm control status of a particular frame type", "<port_id> <unicast|multicast|broadcast>", SW_API_STORM_CTRL_FRAME_GET, NULL},
            {"stormCtrlRate", "set", "set storm ctrl rate", "<port_id> <rate:(packets/s)>", SW_API_STORM_CTRL_RATE_SET, NULL},
            {"stormCtrlRate", "get", "get storm ctrl rate", "<port_id>", SW_API_STORM_CTRL_RATE_GET, NULL},
            {"portpolicer", "set", "set port policer", "<port_id>", SW_API_RATE_PORT_POLICER_SET, NULL},
            {"portpolicer", "get", "get port policer", "<port_id>", SW_API_RATE_PORT_POLICER_GET, NULL},
            {"portshaper", "set", "set port egress shaper", "<port_id> <enable|disable>", SW_API_RATE_PORT_SHAPER_SET, NULL},
            {"portshaper", "get", "get port egress shaper", "<port_id>", SW_API_RATE_PORT_SHAPER_GET, NULL},
            {"queueshaper", "set", "set queue egress shaper", "<port_id> <queue_id> <enable|disable>", SW_API_RATE_QUEUE_SHAPER_SET, NULL},
            {"queueshaper", "get", "get queue egress shaper", "<port_id> <queue_id>", SW_API_RATE_QUEUE_SHAPER_GET, NULL},
            {"aclpolicer", "set", "set acl policer", "<policer_id>", SW_API_RATE_ACL_POLICER_SET, NULL},
            {"aclpolicer", "get", "get acl policer", "<policer_id>", SW_API_RATE_ACL_POLICER_GET, NULL},
            {"ptAddRateByte", "set", "set add_rate_byte when cal rate ", "<port_id> <number:0-255>", SW_API_RATE_PT_ADDRATEBYTE_SET, NULL},
            {"ptAddRateByte", "get", "get add_rate_byte when cal rate ", "<port_id>", SW_API_RATE_PT_ADDRATEBYTE_GET, NULL},
            {"ptgolflowen", "set", "set status of port globle flow control", "<port_id> <enable|disable>", SW_API_RATE_PT_GOL_FLOW_EN_SET, NULL},
            {"ptgolflowen", "get", "get status of port globle flow control", "<port_id>", SW_API_RATE_PT_GOL_FLOW_EN_GET, NULL},
            {NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/
        },
    },
#endif

#ifdef IN_SEC
    {
        "sec", "config security",
        {
            {"mac", "set", "set MAC layer related security", "<resv_vid/invalid_src_addr> <value>", SW_API_SEC_MAC_SET, NULL},
            {"mac", "get", "get MAC layer related security", "<resv_vid/invalid_src_addr>", SW_API_SEC_MAC_GET, NULL},
            {"ip", "set", "set IP layer related security", "<invalid_ver/same_addr/ttl_change_status/ttl_val> <value>", SW_API_SEC_IP_SET, NULL},
            {"ip", "get", "get IP layer related security", "<invalid_ver/same_addr/ttl_change_status/ttl_val>", SW_API_SEC_IP_GET, NULL},
            {"ip4", "set", "set IP4 related security", "<invalid_hl/hdr_opts/invalid_df/frag_offset_min_len/frag_offset_min_size/frag_offset_max_len/invalid_frag_offset/invalid_sip/invalid_dip/invalid_chksum/invalid_pl/df_clear_status/ipid_random_status> <value>", SW_API_SEC_IP4_SET, NULL},
            {"ip4", "get", "get IP4 related security", "<invalid_hl/hdr_opts/invalid_df/frag_offset_min_len/frag_offset_min_size/frag_offset_max_len/invalid_frag_offset/invalid_sip/invalid_dip/invalid_chksum/invalid_pl/df_clear_status/ipid_random_status>", SW_API_SEC_IP4_GET, NULL},
            {"ip6", "set", "set IP6 related security", "<invalid_dip/invalid_sip/invalid_pl> <value>", SW_API_SEC_IP6_SET, NULL},
            {"ip6", "get", "get IP6 related security", "<invalid_dip/invalid_sip/invalid_pl>", SW_API_SEC_IP6_GET, NULL},
            {"tcp", "set", "set TCP related security", "<blat/invalid_hl/min_hdr_size/invalid_syn/su_block/sp_block/sap_block/xmas_scan/null_scan/sr_block/sf_block/sar_block/rst_scan/rst_with_data/fa_block/pa_block/ua_block/invalid_chksum/invalid_urgptr/invalid_opts> <value>", SW_API_SEC_TCP_SET, NULL},
            {"tcp", "get", "get TCP related security", "<blat/invalid_hl/min_hdr_size/invalid_syn/su_block/sp_block/sap_block/xmas_scan/null_scan/sr_block/sf_block/sar_block/rst_scan/rst_with_data/fa_block/pa_block/ua_block/invalid_chksum/invalid_urgptr/invalid_opts>", SW_API_SEC_TCP_GET, NULL},
            {"udp", "set", "set UDP related security", "<blat/invalid_len/invalid_chksum> <value>", SW_API_SEC_UDP_SET, NULL},
            {"udp", "get", "get UDP related security", "<blat/invalid_len/invalid_chksum>", SW_API_SEC_UDP_GET, NULL},
            {"icmp4", "set", "set ICMP4 related security", "<ping_pl_exceed/ping_frag/ping_max_pl> <value>", SW_API_SEC_ICMP4_SET, NULL},
            {"icmp4", "get", "get ICMP4 related security", "<ping_pl_exceed/ping_frag/ping_max_pl>", SW_API_SEC_ICMP4_GET, NULL},
            {"icmp6", "set", "set ICMP6 related security", "<ping_pl_exceed/ping_frag/ping_max_pl> <value>", SW_API_SEC_ICMP6_SET, NULL},
            {"icmp6", "get", "get ICMP6 related security", "<ping_pl_exceed/ping_frag/ping_max_pl>", SW_API_SEC_ICMP6_GET, NULL},
            {NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/
        },
    },
#endif

    /*stp*/
#ifdef IN_STP
    {
        "stp", "config STP",
        {
            {"portState", "set", "set STP state of a port", "<st_id> <port_id> <disable|block|listen|learn|forward>", SW_API_STP_PT_STATE_SET, NULL},
            {"portState", "get", "get STP state of a port", "<st_id> <port_id>", SW_API_STP_PT_STATE_GET, NULL},
            {NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/
        },
    },
#endif

    /*mib*/
#ifdef IN_MIB
    {
        "mib", "show MIB statistics information",
        {
            {"statistics", "get", "get statistics information of a port", "<port_id>",  SW_API_PT_MIB_GET, NULL},
            {"status", "set", "set mib status", "<enable|disable>",  SW_API_MIB_STATUS_SET, NULL},
            {"status", "get", "get mib status", "",  SW_API_MIB_STATUS_GET, NULL},
            {"counters",  "flush", "flush counters of a port", "<port_id>", SW_API_PT_MIB_FLUSH_COUNTERS, NULL},
            {"cpuKeep", "set", "set cpu keep bit", "<enable|disable>",  SW_API_MIB_CPU_KEEP_SET, NULL},
            {"cpuKeep", "get", "get cpu keep bit", "",  SW_API_MIB_CPU_KEEP_GET, NULL},
            {NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/
        },
    },
#endif

    /* led */
#ifdef IN_LED
    {
        "led", "set/get led control pattern",
        {
            {"ctrlpattern", "set", "set led control pattern", "<group_id> <led_id>", SW_API_LED_PATTERN_SET, NULL},
            {"ctrlpattern", "get", "get led control pattern", "<group_id> <led_id>", SW_API_LED_PATTERN_GET, NULL},
            {NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/
        },
    },
#endif

    /* cosmap */
#ifdef IN_COSMAP
    {
        "cosmap", "set/get cosmap table",
        {
            {"dscp2pri", "set", "set dscp to priority map table", "<dscp> <priority>", SW_API_COSMAP_DSCP_TO_PRI_SET, NULL},
            {"dscp2pri", "get", "get dscp to priority map table", "<dscp>", SW_API_COSMAP_DSCP_TO_PRI_GET, NULL},
            {"dscp2dp", "set", "set dscp to dp map table", "<dscp> <dp>", SW_API_COSMAP_DSCP_TO_DP_SET, NULL},
            {"dscp2dp", "get", "get dscp to dp map table", "<dscp>", SW_API_COSMAP_DSCP_TO_DP_GET, NULL},
            {"up2pri", "set", "set dot1p to priority map table", "<up> <priority>", SW_API_COSMAP_UP_TO_PRI_SET, NULL},
            {"up2pri", "get", "get dot1p to priority map table", "<up>", SW_API_COSMAP_UP_TO_PRI_GET, NULL},
            {"up2dp", "set", "set dot1p to dp map table", "<up> <dp>", SW_API_COSMAP_UP_TO_DP_SET, NULL},
            {"up2dp", "get", "get dot1p to dp map table", "<up>", SW_API_COSMAP_UP_TO_DP_GET, NULL},
            {"dscp2ehpri", "set", "set dscp to priority map table for WAN port", "<dscp> <priority>", SW_API_COSMAP_DSCP_TO_EHPRI_SET, NULL},
            {"dscp2ehpri", "get", "get dscp to priority map table for WAN port", "<dscp>", SW_API_COSMAP_DSCP_TO_EHPRI_GET, NULL},
            {"dscp2ehdp", "set", "set dscp to dp map table for WAN port", "<dscp> <dp>", SW_API_COSMAP_DSCP_TO_EHDP_SET, NULL},
            {"dscp2ehdp", "get", "get dscp to dp map table for WAN port", "<dscp>", SW_API_COSMAP_DSCP_TO_EHDP_GET, NULL},
            {"up2ehpri", "set", "set dot1p to priority map table for WAN port", "<up> <priority>", SW_API_COSMAP_UP_TO_EHPRI_SET, NULL},
            {"up2ehpri", "get", "get dot1p to priority map table for WAN port", "<up>", SW_API_COSMAP_UP_TO_EHPRI_GET, NULL},
            {"up2ehdp", "set", "set dot1p to dp map table for WAN port", "<up> <dp>", SW_API_COSMAP_UP_TO_EHDP_SET, NULL},
            {"up2ehdp", "get", "get dot1p to dp map table for WAN port", "<up>", SW_API_COSMAP_UP_TO_EHDP_GET, NULL},
            {"pri2q", "set", "set priority to queue mapping", "<priority> <queueid>", SW_API_COSMAP_PRI_TO_QU_SET, NULL},
            {"pri2q", "get", "get priority to queue mapping", "<priority>", SW_API_COSMAP_PRI_TO_QU_GET, NULL},
            {"pri2ehq", "set", "set priority to enhanced queue mapping", "<priority> <queueid>", SW_API_COSMAP_PRI_TO_EHQU_SET, NULL},
            {"pri2ehq", "get", "get priority to enhanced queue mapping", "<priority>", SW_API_COSMAP_PRI_TO_EHQU_GET, NULL},
            {"egRemark", "set", "set egress remark table", "<tableid>", SW_API_COSMAP_EG_REMARK_SET, NULL},
            {"egRemark", "get", "get egress remark table", "<tableid>", SW_API_COSMAP_EG_REMARK_GET, NULL},
            {NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/
        },
    },
#endif

    /*misc*/
#ifdef IN_MISC
    {
        "misc", "config miscellaneous",
        {
            {"arp", "set", "set arp packets hardware identification status", "<enable|disable>", SW_API_ARP_STATUS_SET, NULL},
            {"arp", "get", "get arp packets hardware identification status", "", SW_API_ARP_STATUS_GET, NULL},
            {"frameMaxSize", "set", "set the maximal received frame size of the device", "<size:byte>", SW_API_FRAME_MAX_SIZE_SET, NULL},
            {"frameMaxSize", "get", "get the maximal received frame size of the device", "", SW_API_FRAME_MAX_SIZE_GET, NULL},
            {"ptUnkSaCmd", "set", "set forwarding command for frames with unknown source address", "<port_id> <forward|drop|cpycpu|rdtcpu>", SW_API_PT_UNK_SA_CMD_SET, NULL},
            {"ptUnkSaCmd", "get", "get forwarding command for frames with unknown source address", "<port_id>", SW_API_PT_UNK_SA_CMD_GET, NULL},
            {"ptUnkUcFilter", "set", "set flooding status of unknown unicast frames", "<port_id> <enable|disable>", SW_API_PT_UNK_UC_FILTER_SET, NULL},
            {"ptUnkUcFilter", "get", "get flooding status of unknown unicast frames", "<port_id>", SW_API_PT_UNK_UC_FILTER_GET, NULL},
            {"ptUnkMcFilter", "set", "set flooding status of unknown multicast frames", "<port_id> <enable|disable>", SW_API_PT_UNK_MC_FILTER_SET, NULL},
            {"ptUnkMcFilter", "get", "get flooding status of unknown multicast frames", "<port_id>", SW_API_PT_UNK_MC_FILTER_GET, NULL},
            {"ptBcFilter", "set", "set flooding status of broadcast frames", "<port_id> <enable|disable>", SW_API_PT_BC_FILTER_SET, NULL},
            {"ptBcFilter", "get", "get flooding status of broadcast frames", "<port_id>", SW_API_PT_BC_FILTER_GET, NULL},
            {"cpuPort", "set", "set cpu port status", "<enable|disable>", SW_API_CPU_PORT_STATUS_SET, NULL},
            {"cpuPort", "get", "get cpu port status", "", SW_API_CPU_PORT_STATUS_GET, NULL},
            {"bctoCpu", "set", "set broadcast frames to Cpu port status", "<enable|disable>", SW_API_BC_TO_CPU_PORT_SET, NULL},
            {"bctoCpu", "get", "get broadcast frames to Cpu port status", "", SW_API_BC_TO_CPU_PORT_GET, NULL},
            {"PppoeCmd", "set", "set pppoe frames forwarding command", "<forward|rdtcpu>", SW_API_PPPOE_CMD_SET, NULL},
            {"PppoeCmd", "get", "get pppoe frames forwarding command", "", SW_API_PPPOE_CMD_GET, NULL},
            {"Pppoe", "set", "set pppoe frames hardware identification status", "<enable|disable>", SW_API_PPPOE_STATUS_SET, NULL},
            {"Pppoe", "get", "get pppoe frames hardware identification status", "", SW_API_PPPOE_STATUS_GET, NULL},
            {"ptDhcp", "set", "set dhcp frames hardware identification status", "<port_id> <enable|disable>", SW_API_PT_DHCP_SET, NULL},
            {"ptDhcp", "get", "get dhcp frames hardware identification status", "<port_id>", SW_API_PT_DHCP_GET, NULL},
            {"arpcmd", "set", "set arp packets forwarding command", "<forward|cpycpu|rdtcpu>", SW_API_ARP_CMD_SET, NULL},
            {"arpcmd", "get", "get arp packets forwarding command", "", SW_API_ARP_CMD_GET, NULL},
            {"eapolcmd", "set", "set eapol packets forwarding command", "<cpycpu|rdtcpu>", SW_API_EAPOL_CMD_SET, NULL},
            {"eapolcmd", "get", "get eapol packets forwarding command", "", SW_API_EAPOL_CMD_GET, NULL},
            {"pppoesession", "add", "add a pppoe session entry", "<session_id> <enable|disable>", SW_API_PPPOE_SESSION_ADD, NULL},
            {"pppoesession", "del", "del a pppoe session entry", "<session_id>", SW_API_PPPOE_SESSION_DEL, NULL},
            {"pppoesession", "get", "get a pppoe session entry", "<session_id>", SW_API_PPPOE_SESSION_GET, NULL},
            {"eapolstatus", "set", "set eapol frames hardware identification status", "<port_id> <enable|disable>", SW_API_EAPOL_STATUS_SET, NULL},
            {"eapolstatus", "get", "get eapol frames hardware identification status", "<port_id>", SW_API_EAPOL_STATUS_GET, NULL},
            {"rip", "set", "set rip packets hardware identification status", "<enable|disable>", SW_API_RIPV1_STATUS_SET, NULL},
            {"rip", "get", "get rip packets hardware identification status", "", SW_API_RIPV1_STATUS_GET, NULL},
            {"ptarpreq", "set", "set arp request packets hardware identification status", "<port_id> <enable|disable>", SW_API_PT_ARP_REQ_STATUS_SET, NULL},
            {"ptarpreq", "get", "get arp request packets hardware identification status", "<port_id>", SW_API_PT_ARP_REQ_STATUS_GET, NULL},
            {"ptarpack", "set", "set arp ack packets hardware identification status", "<port_id> <enable|disable>", SW_API_PT_ARP_ACK_STATUS_SET, NULL},
            {"ptarpack", "get", "get arp ack packets hardware identification status", "<port_id>", SW_API_PT_ARP_ACK_STATUS_GET, NULL},
            {"extendpppoe", "add", "add a pppoe session entry", "", SW_API_PPPOE_SESSION_TABLE_ADD, NULL},
            {"extendpppoe", "del", "del a pppoe session entry", "", SW_API_PPPOE_SESSION_TABLE_DEL, NULL},
            {"extendpppoe", "get", "get a pppoe session entry", "", SW_API_PPPOE_SESSION_TABLE_GET, NULL},
            {"pppoeid", "set", "set a pppoe session id entry", "<index> <id>", SW_API_PPPOE_SESSION_ID_SET, NULL},
            {"pppoeid", "get", "get a pppoe session id entry", "<index>", SW_API_PPPOE_SESSION_ID_GET, NULL},
            {"intrmask", "set", "set switch interrupt mask", "<intr_mask>", SW_API_INTR_MASK_SET, NULL},
            {"intrmask", "get", "get switch interrupt mask", "", SW_API_INTR_MASK_GET, NULL},
            {"intrstatus", "get", "get switch interrupt status", "", SW_API_INTR_STATUS_GET, NULL},
            {"intrstatus", "clear", "clear switch interrupt status", "<intr_mask>", SW_API_INTR_STATUS_CLEAR, NULL},
            {"intrportlinkmask", "set", "set link interrupt mask of a port", "<port_id> <intr_mask>", SW_API_INTR_PORT_LINK_MASK_SET, NULL},
            {"intrportlinkmask", "get", "get link interrupt mask of a port", "<port_id>", SW_API_INTR_PORT_LINK_MASK_GET, NULL},
            {"intrportlinkstatus", "get", "get link interrupt status of a port", "<port_id>", SW_API_INTR_PORT_LINK_STATUS_GET, NULL},
            {"intrmaskmaclinkchg", "set", "set switch interrupt mask for mac link change", "<port_id> <enable | disable>", SW_API_INTR_MASK_MAC_LINKCHG_SET, NULL},
            {"intrmaskmaclinkchg", "get", "get switch interrupt mask for mac link change", "<port_id>", SW_API_INTR_MASK_MAC_LINKCHG_GET, NULL},
            {"intrstatusmaclinkchg", "get", "get switch interrupt status for mac link change", "", SW_API_INTR_STATUS_MAC_LINKCHG_GET, NULL},
            {"intrstatusmaclinkchg", "clear", "clear switch interrupt status for mac link change", "", SW_API_INTR_STATUS_MAC_LINKCHG_CLEAR, NULL},
            {"cpuVid", "set", "set to_cpu vid status", "<enable|disable>", SW_API_CPU_VID_EN_SET, NULL},
            {"cpuVid", "get", "get to_cpu vid status", "", SW_API_CPU_VID_EN_GET, NULL},
            {"rtdPppoe", "set", "set RM_RTD_PPPOE_EN status", "<enable|disable>", SW_API_RTD_PPPOE_EN_SET, NULL},
            {"rtdPppoe", "get", "get RM_RTD_PPPOE_EN status", "", SW_API_RTD_PPPOE_EN_GET, NULL},
			{"glomacaddr", "set", "set global macaddr", "<macaddr>", SW_API_GLOBAL_MACADDR_SET, NULL},
			{"glomacaddr", "get", "get global macaddr", "", SW_API_GLOBAL_MACADDR_GET, NULL},	
			{"lldp", "set", "set lldp frames hardware identification status", "<enable|disable>", SW_API_LLDP_STATUS_SET, NULL},
			{"lldp", "get", "get lldp frames hardware identification status", "", SW_API_LLDP_STATUS_GET, NULL},
			{"framecrc", "set", "set frame crc reserve enable", "<enable|disable>", SW_API_FRAME_CRC_RESERVE_SET, NULL},
			{"framecrc", "get", "get frame crc reserve enable", "", SW_API_FRAME_CRC_RESERVE_GET, NULL},
            {NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/
        },
    },
#endif

    /* IP */
#ifdef IN_IP
    {
        "ip", "config ip",
        {
            {"hostentry", "add", "add host entry", "", SW_API_IP_HOST_ADD, NULL},
            {"hostentry", "del", "del host entry", "<del_mode>", SW_API_IP_HOST_DEL, NULL},
            {"hostentry", "get", "get host entry", "<get_mode>", SW_API_IP_HOST_GET, NULL},
            {"hostentry", "next", "next host entry", "<next_mode>", SW_API_IP_HOST_NEXT, NULL},
            {"hostentry", "show", "show whole host entries", "", SW_CMD_HOST_SHOW, cmd_show_host},
            {"hostentry", "bindcnt", "bind counter to host entry", "<host entry id> <cnt id> <enable|disable>", SW_API_IP_HOST_COUNTER_BIND, NULL},
            {"hostentry", "bindpppoe", "bind pppoe to host entry", "<host entry id> <pppoe id> <enable|disable>", SW_API_IP_HOST_PPPOE_BIND, NULL},
            {"ptarplearn", "set", "set port arp learn flag, bit0 req bit1 ack", "<port_id> <flag>", SW_API_IP_PT_ARP_LEARN_SET, NULL},
            {"ptarplearn", "get", "get port arp learn flag, bit0 req bit1 ack", "<port_id>", SW_API_IP_PT_ARP_LEARN_GET, NULL},
            {"arplearn",   "set", "set arp learn mode", "<learnlocal|learnall>", SW_API_IP_ARP_LEARN_SET, NULL},
            {"arplearn",   "get", "get arp learn mode", "", SW_API_IP_ARP_LEARN_GET, NULL},
            {"ptipsrcguard",   "set", "set ip source guard mode", "<port_id> <mac_ip|mac_ip_port|mac_ip_vlan|mac_ip_port_vlan|no_guard>", SW_API_IP_SOURCE_GUARD_SET, NULL},
            {"ptipsrcguard",   "get", "get ip source guard mode", "", SW_API_IP_SOURCE_GUARD_GET, NULL},
            {"ptarpsrcguard",  "set", "set arp source guard mode", "<port_id> <mac_ip|mac_ip_port|mac_ip_vlan|mac_ip_port_vlan|no_guard>", SW_API_IP_ARP_GUARD_SET, NULL},
            {"ptarpsrcguard",  "get", "get arp source guard mode", "", SW_API_IP_ARP_GUARD_GET, NULL},
            {"routestatus", "set", "set ip route status", "<enable|disable>", SW_API_IP_ROUTE_STATUS_SET, NULL},
            {"routestatus", "get", "get ip route status", "", SW_API_IP_ROUTE_STATUS_GET, NULL},
            {"intfentry", "add", "add interface mac address", "", SW_API_IP_INTF_ENTRY_ADD, NULL},
            {"intfentry", "del", "del interface mac address", "", SW_API_IP_INTF_ENTRY_DEL, NULL},
            {"intfentry", "show", "show whole interface mac entries", "", SW_CMD_INTFMAC_SHOW, cmd_show_intfmac},
            {"ipunksrc", "set", "set ip unkown source command", "<forward|drop|cpycpu|rdtcpu>", SW_API_IP_UNK_SOURCE_CMD_SET, NULL},
            {"ipunksrc", "get", "get ip unkown source command", "", SW_API_IP_UNK_SOURCE_CMD_GET, NULL},
            {"arpunksrc", "set", "set arp unkown source command", "<forward|drop|cpycpu|rdtcpu>", SW_API_ARP_UNK_SOURCE_CMD_SET, NULL},
            {"arpunksrc", "get", "get arp unkown source command", "", SW_API_ARP_UNK_SOURCE_CMD_GET, NULL},
            {"ipagetime", "set", "set dynamic ip entry age time", "<time>", SW_API_IP_AGE_TIME_SET, NULL},
            {"ipagetime", "get", "get dynamic ip entry age time", "", SW_API_IP_AGE_TIME_GET, NULL},
            {"wcmphashmode", "set", "set wcmp hash mode", "<hashmode>", SW_API_WCMP_HASH_MODE_SET, NULL},
            {"wcmphashmode", "get", "get wcmp hash mode", "", SW_API_WCMP_HASH_MODE_GET, NULL},
            {"vrfbaseaddr", "set", "set vrf prv base address", "<ip4 addr>", SW_API_IP_VRF_BASE_ADDR_SET, NULL},
            {"vrfbaseaddr", "get", "get vrf prv base address", "", SW_API_IP_VRF_BASE_ADDR_GET, NULL},
            {"vrfbasemask", "set", "set vrf prv base mask", "<ip4 addr>", SW_API_IP_VRF_BASE_MASK_SET, NULL},
            {"vrfbasemask", "get", "get vrf prv base mask", "", SW_API_IP_VRF_BASE_MASK_GET, NULL},
            {"defaultroute", "set", "set default route entry", "", SW_API_IP_DEFAULT_ROUTE_SET, NULL},
            {"defaultroute", "get", "get default route entry", "", SW_API_IP_DEFAULT_ROUTE_GET, NULL},
            {"hostroute", "set", "set host route entry", "", SW_API_IP_HOST_ROUTE_SET, NULL},
            {"hostroute", "get", "get host route entry", "", SW_API_IP_HOST_ROUTE_GET, NULL},
			{"wcmpentry", "set", "set wcmp entry", "", SW_API_IP_WCMP_ENTRY_SET, NULL},
            {"wcmpentry", "get", "get wcmp entry", "", SW_API_IP_WCMP_ENTRY_GET, NULL},
			{"rfsip4", "set", "set rfs ip4", "", SW_API_IP_RFS_IP4_SET, NULL},
			{"rfsip4", "del", "del rfs ip4", "", SW_API_IP_RFS_IP4_DEL, NULL},
			{"rfsip6", "set", "set rfs ip6", "", SW_API_IP_RFS_IP6_SET, NULL},
			{"rfsip6", "del", "del rfs ip6", "", SW_API_IP_RFS_IP6_DEL, NULL},
            {"defaultflowcmd", "set", "set default flow command", "<vrf id> <lan2lan|wan2lan|lan2wan|wan2wan> <forward|drop|rdtcpu|admit_all>", SW_API_IP_DEFAULT_FLOW_CMD_SET, NULL},
            {"defaultflowcmd", "get", "get default flow command", "<vrf id> <lan2lan|wan2lan|lan2wan|wan2wan>", SW_API_IP_DEFAULT_FLOW_CMD_GET, NULL},
            {"defaultrtflowcmd", "set", "set default route flow command", "<vrf id> <lan2lan|wan2lan|lan2wan|wan2wan> <forward|drop|rdtcpu|admit_all>", SW_API_IP_DEFAULT_RT_FLOW_CMD_SET, NULL},
            {"defaultrtflowcmd", "get", "get default route flow command", "<vrf id> <lan2lan|wan2lan|lan2wan|wan2wan>", SW_API_IP_DEFAULT_RT_FLOW_CMD_GET, NULL},
            {NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/
        },
    },
#endif

    /* NAT */
#ifdef IN_NAT
    {
        "nat", "config nat",
        {
            {"natentry", "add", "add nat entry", "", SW_API_NAT_ADD, NULL},
            {"natentry", "del", "del nat entry", "<del_mode>", SW_API_NAT_DEL, NULL},
            {"natentry", "get", "get nat entry", "<get_mode>", SW_API_NAT_GET, NULL},
            {"natentry", "next", "next nat entry", "<next_mode>", SW_API_NAT_NEXT, NULL},
            {"natentry", "show", "show whole nat entries", "", SW_CMD_NAT_SHOW, cmd_show_nat},
            {"natentry", "bindcnt", "bind counter to nat entry", "<nat entry id> <cnt id> <enable|disable>", SW_API_NAT_COUNTER_BIND, NULL},
            {"naptentry", "add", "add napt entry", "", SW_API_NAPT_ADD, NULL},
            {"naptentry", "del", "del napt entry", "<del_mode>", SW_API_NAPT_DEL, NULL},
            {"naptentry", "get", "get napt entry", "<get_mode>", SW_API_NAPT_GET, NULL},
            {"naptentry", "next", "next napt entry", "<next_mode>", SW_API_NAPT_NEXT, NULL},
            {"naptentry", "show", "show whole napt entries", "", SW_CMD_NAPT_SHOW, cmd_show_napt},
            {"naptentry", "bindcnt", "bind counter to napt entry", "<napt entry id> <cnt id> <enable|disable>", SW_API_NAPT_COUNTER_BIND, NULL},
            {"flowentry", "add", "add flow entry", "", SW_API_FLOW_ADD, NULL},
            {"flowentry", "del", "del flow entry", "<del_mode>", SW_API_FLOW_DEL, NULL},
            {"flowentry", "get", "get flow entry", "<get_mode>", SW_API_FLOW_GET, NULL},
            {"flowentry", "next", "next flow entry", "<next_mode>", SW_API_FLOW_NEXT, NULL},
            {"flowentry", "show", "show whole flow entries", "", SW_CMD_FLOW_SHOW, cmd_show_flow},
            {"flowentry", "bindcnt", "bind counter to flow entry", "<flow entry id> <cnt id> <enable|disable>", SW_API_FLOW_COUNTER_BIND, NULL},
            {"natstatus", "set", "set nat status", "<enable|disable>", SW_API_NAT_STATUS_SET, NULL},
            {"natstatus", "get", "get nat status", "", SW_API_NAT_STATUS_GET, NULL},
            {"naptstatus", "set", "set napt status", "<enable|disable>", SW_API_NAPT_STATUS_SET, NULL},
            {"naptstatus", "get", "get napt status", "", SW_API_NAPT_STATUS_GET, NULL},
            {"nathash", "set", "set nat hash mode", "<flag>", SW_API_NAT_HASH_MODE_SET, NULL},
            {"nathash", "get", "get nat hash mode", "", SW_API_NAT_HASH_MODE_GET, NULL},
            {"naptmode", "set", "set napt mode", "<fullcone|strictcone|portstrict|synmatric>", SW_API_NAPT_MODE_SET, NULL},
            {"naptmode", "get", "get napt mode", "", SW_API_NAPT_MODE_GET, NULL},
            {"prvbaseaddr", "set", "set nat prv base address", "<ip4 addr>", SW_API_PRV_BASE_ADDR_SET, NULL},
            {"prvbaseaddr", "get", "get nat prv base address", "", SW_API_PRV_BASE_ADDR_GET, NULL},
            {"prvaddrmode", "set", "set nat prv address map mode", "<enable|disable>", SW_API_PRV_ADDR_MODE_SET, NULL},
            {"prvaddrmode", "get", "get nat prv address map mode", "", SW_API_PRV_ADDR_MODE_GET, NULL},
            {"pubaddr", "add", "add pub address", "", SW_API_PUB_ADDR_ENTRY_ADD, NULL},
            {"pubaddr", "del", "del pub address", "<del_mode>", SW_API_PUB_ADDR_ENTRY_DEL, NULL},
            {"pubaddr", "show", "show whole pub address entries", "", SW_CMD_PUBADDR_SHOW, cmd_show_pubaddr},
            {"natunksess", "set", "set nat unkown session command", "<forward|drop|cpycpu|rdtcpu>", SW_API_NAT_UNK_SESSION_CMD_SET, NULL},
            {"natunksess", "get", "get nat unkown session command", "", SW_API_NAT_UNK_SESSION_CMD_GET, NULL},
            {"prvbasemask", "set", "set nat prv base mask", "<ip4 mask>", SW_API_PRV_BASE_MASK_SET, NULL},
            {"prvbasemask", "get", "get nat prv base mask", "", SW_API_PRV_BASE_MASK_GET, NULL},
            {"global", "set", "set global nat function", "<enable|disable> <enable:sync counter|disable:unsync counter>", SW_API_NAT_GLOBAL_SET, NULL},
			{"flowcookie", "set", "set flow cookie", "", SW_API_FLOW_COOKIE_SET, NULL},
			{"flowrfs", "set", "set flow rfs", "<action>", SW_API_FLOW_RFS_SET, NULL},
            {NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/
        },
    },
#endif

    /*Trunk*/
#ifdef IN_TRUNK
    {
        "trunk", "config trunk",
        {
            {"group", "set", "set trunk group member info", "<trunk_id> <disable|enable> <port_bitmap>", SW_API_TRUNK_GROUP_SET, NULL},
            {"group", "get", "get trunk group member info", "<trunk_id>", SW_API_TRUNK_GROUP_GET, NULL},
            {"hashmode", "set", "set trunk hash mode", "<hash_mode>", SW_API_TRUNK_HASH_SET, NULL},
            {"hashmode", "get", "get trunk hash mode", "", SW_API_TRUNK_HASH_GET, NULL},
            {"mansa", "set", "set trunk manipulable sa", "<macaddr>", SW_API_TRUNK_MAN_SA_SET, NULL},
            {"mansa", "get", "get trunk manipulable sa", "", SW_API_TRUNK_MAN_SA_GET, NULL},
            {NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/
        },
    },
#endif

    /*Interface Control*/
#ifdef IN_INTERFACECONTROL
    {
        "interface", "config interface",
        {
            {"macmode", "set", "set mac mode info", "<port_id>", SW_API_MAC_MODE_SET, NULL},
            {"macmode", "get", "get mac mode info", "<port_id>", SW_API_MAC_MODE_GET, NULL},
            {"pt3azstatus", "set", "get mac mode info", "<port_id> <enable/disable>", SW_API_PORT_3AZ_STATUS_SET, NULL},
            {"pt3azstatus", "get", "get mac mode info", "<port_id>", SW_API_PORT_3AZ_STATUS_GET, NULL},
            {"phymode", "set", "set phy mode info", "<phy_id>", SW_API_PHY_MODE_SET, NULL},
            {"phymode", "get", "get phy mode info", "<phy_id>", SW_API_PHY_MODE_GET, NULL},
            {"fx100ctrl", "set", "set fx100 config", "", SW_API_FX100_CTRL_SET, NULL},
            {"fx100ctrl", "get", "get fx100 config", "", SW_API_FX100_CTRL_GET, NULL},
            {"fx100status", "get", "get fx100 status", "", SW_API_FX100_STATUS_GET, NULL},
            {"mac06exch", "set", "set mac0 and mac6 exchange status", "<enable/disable>", SW_API_MAC06_EXCH_SET, NULL},
            {"mac06exch", "get", "get mac0 and mac6 exchange status", "", SW_API_MAC06_EXCH_GET, NULL},
            {NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/
        },
    },
#endif

    /* debug */
    {
        "debug", "read/write register",
        {
            {"phy", "get", "read phy register", "<ph_id> <reg_addr>", SW_API_PHY_GET, NULL},
            {"phy", "set", "write phy register", "<ph_id> <reg_addr> <value>", SW_API_PHY_SET, NULL},
            {"reg", "get", "read switch register", "<reg_addr> <4>", SW_API_REG_GET, NULL},
            {"reg", "set", "write switch register", "<reg_addr> <value> <4>", SW_API_REG_SET, NULL},
			{"reg", "dump", "dump switch register group", "<group id> <0-6>", SW_API_REG_DUMP, NULL},
			{"dbgreg", "dump", "dump switch dbg register group", "", SW_API_DBG_REG_DUMP, NULL},
            {"preg", "get", "read psgmii register", "<reg_addr> <4>", SW_API_PSGMII_REG_GET, NULL},
            {"preg", "set", "write psgmii register", "<reg_addr> <value> <4>", SW_API_PSGMII_REG_SET, NULL},
            {"field", "get", "read switch register field", "<reg_addr> <offset> <len> <4>", SW_API_REG_FIELD_GET, NULL},
            {"field", "set", "write switch register field", "<reg_addr> <offset> <len> <value> <4>", SW_API_REG_FIELD_SET, NULL},
            {"aclList", "dump", "dump all acl list", "", SW_API_ACL_LIST_DUMP, NULL},
            {"aclRule", "dump", "dump all acl rule", "", SW_API_ACL_RULE_DUMP, NULL},
            {"device",  "reset", "reset device",     "", SW_API_SWITCH_RESET, NULL},
            {"ssdk",  "config", "show ssdk configuration",     "", SW_API_SSDK_CFG, NULL},
            {NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/
        },
    },

    /*debug*/
    {
        "device", "set device id",
        {
            {"id", "set", "set device id", "<dev_id>", SW_CMD_SET_DEVID, cmd_set_devid},
            {NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/
        },
    },

    {"help", "type ? get help", {{NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/}},

    {"quit", "type quit/q quit shell", {{NULL, NULL, NULL, NULL, (int)NULL, NULL}/*end of desc*/}},

    {NULL, NULL, {{NULL, NULL, NULL, NULL, (int)NULL, NULL}}} /*end of desc*/
};

