/*
 * mstp.c      State machines from IEEE 802.1Q-2005
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version
 *  2 of the License, or (at your option) any later version.
 *
 * Authors: Vitalii Demianets <dvitasgs@gmail.com>
 */

/* NOTE: The standard messes up Hello_Time timer management.
 * The portTimes and msgTimes structures have it, while
 * designatedTimes, rootTimes and BridgeTimes do not!
 * And there are places, where standard says:
 * "portTimes = designatedTimes" (13.32)
 * "rootTimes = portTimes" (13.26.23)
 * ---- Bad IEEE! ----
 * For now I decide: All structures will hold Hello_Time,
 * because in 802.1D they do.
 * Besides, it is necessary for compatibility with old STP implementations.
 */

/* 802.1Q-2005 does not define but widely use variable name newInfoXst.
 * From the 802.1s I can guess that it means:
 *   - "newInfo" when tree is CIST;
 *   - "newInfoMsti" when tree is not CIST (is MSTI).
 * But that is only a guess and I could be wrong here ;)
 */

/* Important coding convention:
 *   All functions that have dry_run argument must follow the
 *   return value convention:
 *      They should return true if state change detected during dry run.
 *      Otherwise (!dry_run || !state_change) they return false.
 */

#include <config.h>

#include <string.h>
#include <netinet/in.h>
#include <linux/if_bridge.h>
#include <asm/byteorder.h>

#include "mstp.h"
#include "log.h"
#include "driver.h"
#include "clock_gettime.h"

static void PTSM_tick(port_t *prt);
static bool TCSM_run(per_tree_port_t *ptp, bool dry_run);
static void BDSM_begin(port_t *prt);
static void br_state_machines_begin(bridge_t *br);
static void prt_state_machines_begin(port_t *prt);
static void tree_state_machines_begin(tree_t *tree);
static void br_state_machines_run(bridge_t *br);
static void updtbrAssuRcvdInfoWhile(port_t *prt);

#define FOREACH_PORT_IN_BRIDGE(port, bridge) \
    list_for_each_entry((port), &(bridge)->ports, br_list)
#define FOREACH_TREE_IN_BRIDGE(tree, bridge) \
    list_for_each_entry((tree), &(bridge)->trees, bridge_list)
#define FOREACH_PTP_IN_TREE(ptp, tree) \
    list_for_each_entry((ptp), &(tree)->ports, tree_list)
#define FOREACH_PTP_IN_PORT(ptp, port) \
    list_for_each_entry((ptp), &(port)->trees, port_list)

/* 17.20.11 of 802.1D */
#define rstpVersion(br) ((br)->ForceProtocolVersion >= protoRSTP)
/* Bridge assurance is operational only when NetworkPort type is configured
 * and the operation status is pointToPoint and version is RSTP/MSTP
 */
#define assurancePort(prt) ((prt)->NetworkPort && (prt)->operPointToPointMAC \
                            && (prt)->sendRSTP)
/*
 * Recalculate configuration digest. (13.7)
 */
static void RecalcConfigDigest(bridge_t *br)
{
    __be16 vid2mstid[MAX_VID + 2];
    unsigned char mstp_key[] = HMAC_KEY;
    int vid;

    vid2mstid[0] = vid2mstid[MAX_VID + 1] = 0;
    for(vid = 1; vid <= MAX_VID; ++vid)
        vid2mstid[vid] = br->fid2mstid[br->vid2fid[vid]];

    hmac_md5((void *)vid2mstid, sizeof(vid2mstid), mstp_key, sizeof(mstp_key),
             (caddr_t)br->MstConfigId.s.configuration_digest);
}

/*
 * 13.37.1 - Table 13-3
 */
static __u32 compute_pcost(int speed)
{
  /* speed is in MB/s*/
  if(speed > 0)
    return (speed < 20000000) ? 20000000 / speed : 1;
  else
    return MAX_PATH_COST;
}

static void bridge_default_internal_vars(bridge_t *br)
{
    br->uptime = 0;
}

static void tree_default_internal_vars(tree_t *tree)
{
    /* 12.8.1.1.3.(b,c,d) */
    tree->time_since_topology_change = 0;
    tree->topology_change_count = 0;
    tree->topology_change = false; /* since all tcWhile are initialized to 0 */
    strncpy(tree->topology_change_port, "None", IFNAMSIZ);
    strncpy(tree->last_topology_change_port, "None", IFNAMSIZ);

    /* The following are initialized in BEGIN state:
     *  - rootPortId, rootPriority, rootTimes: in Port Role Selection SM
     */
}

static void port_default_internal_vars(port_t *prt)
{
    prt->infoInternal = false;
    prt->rcvdInternal = false;
    prt->rcvdTcAck = false;
    prt->rcvdTcn = false;
    assign(prt->rapidAgeingWhile, 0u);
    assign(prt->brAssuRcvdInfoWhile, 0u);
    prt->BaInconsistent = false;
    prt->num_rx_bpdu_filtered = 0;
    prt->num_rx_bpdu = 0;
    prt->num_rx_tcn = 0;
    prt->num_tx_bpdu = 0;
    prt->num_tx_tcn = 0;
    prt->num_trans_fwd = 0;
    prt->num_trans_blk = 0;

    /* The following are initialized in BEGIN state:
     * - mdelayWhile. mcheck, sendRSTP: in Port Protocol Migration SM
     * - helloWhen, newInfo, newInfoMsti, txCount: in Port Transmit SM
     * - edgeDelayWhile, rcvdBpdu, rcvdRSTP, rcvdSTP : in Port Receive SM
     * - operEdge: in Bridge Detection SM
     * - tcAck: in Topology Change SM
     */
}

static void ptp_default_internal_vars(per_tree_port_t *ptp)
{
    ptp->rcvdTc = false;
    ptp->tcProp = false;
    ptp->updtInfo = false;
    ptp->master = false; /* 13.24.5 */
    ptp->disputed = false;
    assign(ptp->rcvdInfo, (port_info_t)0);
    ptp->mastered = false;
    memset(&ptp->msgPriority, 0, sizeof(ptp->msgPriority));
    memset(&ptp->msgTimes, 0, sizeof(ptp->msgTimes));

    /* The following are initialized in BEGIN state:
     * - rcvdMsg: in Port Receive SM
     * - fdWhile, rrWhile, rbWhile, role, learn, forward,
     *   sync, synced, reRoot: in Port Role Transitions SM
     * - tcWhile, fdbFlush: Topology Change SM
     * - rcvdInfoWhile, proposed, proposing, agree, agreed,
     *   infoIs, reselect, selected: Port Information SM
     * - forwarding, learning: Port State Transition SM
     * - selectedRole, designatedPriority, designatedTimes: Port Role Selection SM
     */
}

static tree_t * create_tree(bridge_t *br, __u8 *macaddr, __be16 MSTID)
{
    /* Initialize all fields except anchor */
    tree_t *tree = calloc(1, sizeof(*tree));
    if(!tree)
    {
        ERROR_BRNAME(br, "Out of memory");
        return NULL;
    }
    tree->bridge = br;
    tree->MSTID = MSTID;
    INIT_LIST_HEAD(&tree->ports);

    memcpy(tree->BridgeIdentifier.s.mac_address, macaddr, ETH_ALEN);
    /* 0x8000 = default bridge priority (17.14 of 802.1D) */
    tree->BridgeIdentifier.s.priority = __constant_cpu_to_be16(0x8000) | MSTID;
    assign(tree->BridgePriority.RootID, tree->BridgeIdentifier);
    assign(tree->BridgePriority.RRootID, tree->BridgeIdentifier);
    assign(tree->BridgePriority.DesignatedBridgeID, tree->BridgeIdentifier);
    /* 13.23.4 */
    assign(tree->BridgeTimes.remainingHops, br->MaxHops);
    assign(tree->BridgeTimes.Forward_Delay, br->Forward_Delay);
    assign(tree->BridgeTimes.Max_Age, br->Max_Age);
    assign(tree->BridgeTimes.Message_Age, (__u8)0);
    assign(tree->BridgeTimes.Hello_Time, br->Hello_Time);

    tree_default_internal_vars(tree);

    return tree;
}

static per_tree_port_t * create_ptp(tree_t *tree, port_t *prt)
{
    /* Initialize all fields except anchors */
    per_tree_port_t *ptp = calloc(1, sizeof(*ptp));
    if(!ptp)
    {
        ERROR_PRTNAME(prt->bridge, prt, "Out of memory");
        return NULL;
    }
    ptp->port = prt;
    ptp->tree = tree;
    ptp->MSTID = tree->MSTID;

    ptp->state = BR_STATE_DISABLED;
    /* 0x80 = default port priority (17.14 of 802.1D) */
    ptp->portId = __constant_cpu_to_be16(0x8000) | prt->port_number;
    assign(ptp->AdminInternalPortPathCost, 0u);
    assign(ptp->InternalPortPathCost, compute_pcost(GET_PORT_SPEED(prt)));
    /* 802.1Q leaves portPriority and portTimes uninitialized */
    assign(ptp->portPriority, tree->BridgePriority);
    assign(ptp->portTimes, tree->BridgeTimes);

    ptp->calledFromFlushRoutine = false;

    ptp_default_internal_vars(ptp);

    return ptp;
}

/* External events */

bool MSTP_IN_bridge_create(bridge_t *br, __u8 *macaddr)
{
    tree_t *cist;

    if (!driver_create_bridge(br, macaddr))
        return false;

    /* Initialize all fields except sysdeps and anchor */
    INIT_LIST_HEAD(&br->ports);
    INIT_LIST_HEAD(&br->trees);
    br->bridgeEnabled = false;
    memset(br->vid2fid, 0, sizeof(br->vid2fid));
    memset(br->fid2mstid, 0, sizeof(br->fid2mstid));
    assign(br->MstConfigId.s.selector, (__u8)0);
    sprintf((char *)br->MstConfigId.s.configuration_name,
            "%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
            macaddr[0], macaddr[1], macaddr[2],
            macaddr[3], macaddr[4], macaddr[5]);
    assign(br->MstConfigId.s.revision_level, __constant_cpu_to_be16(0));
    RecalcConfigDigest(br); /* set br->MstConfigId.s.configuration_digest */
    br->ForceProtocolVersion = protoRSTP;
    assign(br->MaxHops, (__u8)20);       /* 13.37.3 */
    assign(br->Forward_Delay, (__u8)15); /* 17.14 of 802.1D */
    assign(br->Max_Age, (__u8)20);       /* 17.14 of 802.1D */
    assign(br->Transmit_Hold_Count, 6u); /* 17.14 of 802.1D */
    assign(br->Migrate_Time, 3u); /* 17.14 of 802.1D */
    assign(br->Ageing_Time, 300u);/* 8.8.3 Table 8-3 */
    assign(br->Hello_Time, (__u8)2);     /* 17.14 of 802.1D */

    bridge_default_internal_vars(br);

    /* Create CIST */
    if(!(cist = create_tree(br, macaddr, 0)))
        return false;
    list_add_tail(&cist->bridge_list, &br->trees);

    return true;
}

bool MSTP_IN_port_create_and_add_tail(port_t *prt, __u16 portno)
{
    tree_t *tree;
    per_tree_port_t *ptp, *nxt;
    bridge_t *br = prt->bridge;

    if (!driver_create_port(prt, portno))
        return false;

    /* Initialize all fields except sysdeps and bridge */
    INIT_LIST_HEAD(&prt->trees);
    prt->port_number = __cpu_to_be16(portno);

    assign(prt->AdminExternalPortPathCost, 0u);
    /* Default for operP2P is false because by default AdminP2P
     * says to auto-detect p2p state, and it is derived from duplex
     * and initially port is in down state and in this down state
     * duplex is set to false (half) */
    prt->AdminP2P = p2pAuto;
    prt->operPointToPointMAC = false;
    prt->portEnabled = false;
    prt->restrictedRole = false; /* 13.25.14 */
    prt->restrictedTcn = false; /* 13.25.15 */
    assign(prt->ExternalPortPathCost, MAX_PATH_COST); /* 13.37.1 */
    prt->AdminEdgePort = false; /* 13.25 */
    prt->AutoEdge = true;       /* 13.25 */
    prt->BpduGuardPort = false;
    prt->BpduGuardError = false;
    prt->NetworkPort = false;
    prt->dontTxmtBpdu = false;
    prt->bpduFilterPort = false;
    prt->deleted = false;

    port_default_internal_vars(prt);

    /* Create PerTreePort structures for all existing trees */
    FOREACH_TREE_IN_BRIDGE(tree, br)
    {
        if(!(ptp = create_ptp(tree, prt)))
        {
            /* Remove and free all previously created entries in port's list */
            list_for_each_entry_safe(ptp, nxt, &prt->trees, port_list)
            {
                list_del(&ptp->port_list);
                list_del(&ptp->tree_list);
                free(ptp);
            }
            return false;
        }
        list_add_tail(&ptp->port_list, &prt->trees);
        list_add_tail(&ptp->tree_list, &tree->ports);
    }

    /* Add new port to the tail of the list in the bridge */
    /* NOTE: if one wants add port NOT to the tail of the list of ports,
     * one should revise above loop (FOREACH_TREE_IN_BRIDGE)
     * because it heavily depends on the fact that port is added to the tail.
     */
    list_add_tail(&prt->br_list, &br->ports);

    prt_state_machines_begin(prt);
    return true;
}

void MSTP_IN_delete_port(port_t *prt)
{
    per_tree_port_t *ptp, *nxt;
    bridge_t *br = prt->bridge;

    driver_delete_port(prt);

    prt->deleted = true;
    if(prt->portEnabled)
    {
        prt->portEnabled = false;
        br_state_machines_run(br);
    }

    list_for_each_entry_safe(ptp, nxt, &prt->trees, port_list)
    {
        list_del(&ptp->port_list);
        list_del(&ptp->tree_list);
        free(ptp);
    }

    list_del(&prt->br_list);
    br_state_machines_run(br);
}

void MSTP_IN_delete_bridge(bridge_t *br)
{
    tree_t *tree, *nxt_tree;
    port_t *prt, *nxt_prt;

    driver_delete_bridge(br);

    br->bridgeEnabled = false;

    /* We SHOULD first delete all ports and only THEN delete all tree_t
     * structures as the tree_t structure contains the head for the per-port
     * list of tree data (tree_t.ports).
     * If this list_head will be deleted before all the per_tree_ports
     * bad things will happen ;)
     */

    list_for_each_entry_safe(prt, nxt_prt, &br->ports, br_list)
    {
        MSTP_IN_delete_port(prt);
        free(prt);
    }

    list_for_each_entry_safe(tree, nxt_tree, &br->trees, bridge_list)
    {
        list_del(&tree->bridge_list);
        free(tree);
    }
}

void MSTP_IN_set_bridge_address(bridge_t *br, __u8 *macaddr)
{
    tree_t *tree;
    bool changed = false;

    FOREACH_TREE_IN_BRIDGE(tree, br)
    {
        if(0 == memcmp(tree->BridgeIdentifier.s.mac_address, macaddr, ETH_ALEN))
            continue;
        changed = true;
        memcpy(tree->BridgeIdentifier.s.mac_address, macaddr, ETH_ALEN);
        tree->BridgePriority.RootID = tree->BridgePriority.RRootID =
            tree->BridgePriority.DesignatedBridgeID = tree->BridgeIdentifier;
    }

    if(changed)
        br_state_machines_begin(br);
}

void MSTP_IN_set_bridge_enable(bridge_t *br, bool up)
{
    port_t *prt;
    per_tree_port_t *ptp;
    tree_t *tree;

    if(br->bridgeEnabled == up)
        return;
    br->bridgeEnabled = up;

    /* Reset all internal states and variables,
     * except those which are user-configurable */
    bridge_default_internal_vars(br);
    FOREACH_TREE_IN_BRIDGE(tree, br)
    {
        tree_default_internal_vars(tree);
    }
    FOREACH_PORT_IN_BRIDGE(prt, br)
    {
        /* NOTE: Don't check prt->deleted here, as it is imposible condition */
        /* NOTE: In the port_default_internal_vars() rapidAgeingWhile will be
         *  reset, so we should stop rapid ageing procedure here.
         */
        if(prt->rapidAgeingWhile)
        {
            MSTP_OUT_set_ageing_time(prt, br->Ageing_Time);
        }
        port_default_internal_vars(prt);
        FOREACH_PTP_IN_PORT(ptp, prt)
        {
            if(BR_STATE_DISABLED != ptp->state)
            {
                MSTP_OUT_set_state(ptp, BR_STATE_DISABLED);
            }
            ptp_default_internal_vars(ptp);
        }
    }
    br_state_machines_begin(br);
}

void MSTP_IN_set_port_enable(port_t *prt, bool up, int speed, int duplex)
{
    __u32 computed_pcost, new_ExternalPathCost, new_InternalPathCost;
    per_tree_port_t *ptp;
    bool new_p2p;
    bool changed = false;

    if(up)
    {
        computed_pcost = compute_pcost(speed);
        new_ExternalPathCost = (0 == prt->AdminExternalPortPathCost) ?
                                 computed_pcost
                               : prt->AdminExternalPortPathCost;
        if(prt->ExternalPortPathCost != new_ExternalPathCost)
        {
            assign(prt->ExternalPortPathCost, new_ExternalPathCost);
            changed = true;
        }
        FOREACH_PTP_IN_PORT(ptp, prt)
        {
            new_InternalPathCost = (0 == ptp->AdminInternalPortPathCost) ?
                                    computed_pcost
                                   : ptp->AdminInternalPortPathCost;
            if(ptp->InternalPortPathCost != new_InternalPathCost)
            {
                assign(ptp->InternalPortPathCost, new_InternalPathCost);
                changed = true;
            }
        }

        switch(prt->AdminP2P)
        {
            case p2pForceTrue:
                new_p2p = true;
                break;
            case p2pForceFalse:
                new_p2p = false;
                break;
            case p2pAuto:
            default:
                new_p2p = !!duplex;
                break;
        }
        if(prt->operPointToPointMAC != new_p2p)
        {
            prt->operPointToPointMAC = new_p2p;
            changed = true;
        }

        if(!prt->portEnabled)
        {
            prt->portEnabled = true;
            prt->BpduGuardError = false;
            prt->BaInconsistent = false;
            prt->num_rx_bpdu_filtered = 0;
            prt->num_rx_bpdu = 0;
            prt->num_rx_tcn = 0;
            prt->num_tx_bpdu = 0;
            prt->num_tx_tcn = 0;
            changed = true;
            /* When port is enabled, initialize bridge assurance timer,
             * so that enough time is given before port is put in
             * inconsistent state.
             */
            updtbrAssuRcvdInfoWhile(prt);
        }
    }
    else
    {
        if(prt->portEnabled)
        {
            prt->portEnabled = false;
            changed = true;
        }
    }

    if(changed)
        br_state_machines_run(prt->bridge);
}

void MSTP_IN_one_second(bridge_t *br)
{
    port_t *prt;
    tree_t *tree;

    ++(br->uptime);

    if(!br->bridgeEnabled)
        return;

    FOREACH_TREE_IN_BRIDGE(tree, br)
        if(!(tree->topology_change))
            ++(tree->time_since_topology_change);

    FOREACH_PORT_IN_BRIDGE(prt, br)
    {
        PTSM_tick(prt);
        /* support for rapid ageing */
        if(prt->rapidAgeingWhile)
        {
            if((--(prt->rapidAgeingWhile)) == 0)
            {
                if(!prt->deleted)
                    MSTP_OUT_set_ageing_time(prt, br->Ageing_Time);
            }
        }
    }

    br_state_machines_run(br);
}

void MSTP_IN_all_fids_flushed(per_tree_port_t *ptp)
{
    bridge_t *br = ptp->port->bridge;
    ptp->fdbFlush = false;
    if(!br->bridgeEnabled)
        return;
    if(!ptp->calledFromFlushRoutine)
    {
        TCSM_run(ptp, false /* actual run */);
        br_state_machines_run(br);
    }
}

/* NOTE: bpdu pointer is unaligned, but it works because
 * bpdu_t is packed. Don't try to cast bpdu to non-packed type ;)
 */
void MSTP_IN_rx_bpdu(port_t *prt, bpdu_t *bpdu, int size)
{
    int mstis_size;
    bridge_t *br = prt->bridge;

    ++(prt->num_rx_bpdu);

    if(prt->BpduGuardPort)
    {
        prt->BpduGuardError = true;
        ERROR_PRTNAME(br, prt,
                      "Received BPDU on BPDU Guarded Port - Port Down");
        MSTP_OUT_shutdown_port(prt);
        return;
    }

    if(prt->bpduFilterPort)
    {
        LOG_PRTNAME(br, prt,
                   "Received BPDU on BPDU Filtered Port - discarded");
        ++(prt->num_rx_bpdu_filtered);
        return;
    }

    if(!br->bridgeEnabled)
    {
        INFO_PRTNAME(br, prt, "Received BPDU while bridge is disabled");
        return;
    }

    if(prt->rcvdBpdu)
    {
        ERROR_PRTNAME(br, prt, "Port hasn't processed previous BPDU");
        return;
    }

    /* 14.4 Validation */
    if((TCN_BPDU_SIZE > size) || (0 != bpdu->protocolIdentifier))
    {
bpdu_validation_failed:
        INFO_PRTNAME(br, prt, "BPDU validation failed");
        return;
    }
    switch(bpdu->bpduType)
    {
        case bpduTypeTCN:
            /* 14.4.b) */
            /* Valid TCN BPDU */
            bpdu->protocolVersion = protoSTP;
            LOG_PRTNAME(br, prt, "received TCN BPDU");
            break;
        case bpduTypeConfig:
            /* 14.4.a) */
            if(CONFIG_BPDU_SIZE > size)
                goto bpdu_validation_failed;
            /* Valid Config BPDU */
            bpdu->protocolVersion = protoSTP;
            LOG_PRTNAME(br, prt, "received Config BPDU%s",
                        (bpdu->flags & (1 << offsetTc)) ? ", tcFlag" : ""
                       );
            break;
        case bpduTypeRST:
            if(protoRSTP == bpdu->protocolVersion)
            { /* 14.4.c) */
                if(RST_BPDU_SIZE > size)
                    goto bpdu_validation_failed;
                /* Valid RST BPDU */
                /* bpdu->protocolVersion = protoRSTP; */
                LOG_PRTNAME(br, prt, "received RST BPDU%s",
                            (bpdu->flags & (1 << offsetTc)) ? ", tcFlag" : ""
                           );
                break;
            }
            if(protoMSTP > bpdu->protocolVersion)
                goto bpdu_validation_failed;
            /* Yes, 802.1Q-2005 says here to check if it contains
             * "35 or more octets", not 36! (see 14.4.d).1) )
             * That's why I check size against CONFIG_BPDU_SIZE
             * and not RST_BPDU_SIZE.
             */
            if(CONFIG_BPDU_SIZE > size)
                goto bpdu_validation_failed;
            mstis_size = __be16_to_cpu(bpdu->version3_len)
                         - MST_BPDU_VER3LEN_WO_MSTI_MSGS;
            if((MST_BPDU_SIZE_WO_MSTI_MSGS > size) || (0 != bpdu->version1_len)
               || (0 > mstis_size)
               || ((MAX_STANDARD_MSTIS * sizeof(msti_configuration_message_t))
                   < mstis_size)
               || (0 != (mstis_size % sizeof(msti_configuration_message_t)))
              )
            { /* 14.4.d) */
                /* Valid RST BPDU */
                bpdu->protocolVersion = protoRSTP;
                LOG_PRTNAME(br, prt, "received RST BPDU");
                break;
            }
            /* 14.4.e) */
            /* Valid MST BPDU */
            bpdu->protocolVersion = protoMSTP;
            prt->rcvdBpduNumOfMstis = mstis_size
                                      / sizeof(msti_configuration_message_t);
            LOG_PRTNAME(br, prt, "received MST BPDU%s with %d MSTIs",
                        (bpdu->flags & (1 << offsetTc)) ? ", tcFlag" : "",
                        prt->rcvdBpduNumOfMstis
                       );
            break;
        default:
            goto bpdu_validation_failed;
    }

    if((protoSTP == bpdu->protocolVersion) && (bpduTypeTCN == bpdu->bpduType))
    {
        ++(prt->num_rx_tcn);
    }
    else
    {
        if(bpdu->flags & (1 << offsetTc))
            ++(prt->num_rx_tcn);
    }

    assign(prt->rcvdBpduData, *bpdu);
    prt->rcvdBpdu = true;

    /* Reset bridge assurance on receipt of valid BPDU */
    if(prt->BaInconsistent)
    {
        prt->BaInconsistent = false;
        INFO_PRTNAME(br, prt, "Clear Bridge assurance inconsistency");
    }
    updtbrAssuRcvdInfoWhile(prt);

    br_state_machines_run(br);
}

/* 12.8.1.1 Read CIST Bridge Protocol Parameters */
void MSTP_IN_get_cist_bridge_status(bridge_t *br, CIST_BridgeStatus *status)
{
    tree_t *cist = GET_CIST_TREE(br);
    assign(status->bridge_id, cist->BridgeIdentifier);
    assign(status->time_since_topology_change,
           cist->time_since_topology_change);
    assign(status->topology_change_count, cist->topology_change_count);
    status->topology_change = cist->topology_change;
    strncpy(status->topology_change_port, cist->topology_change_port,
            IFNAMSIZ);
    strncpy(status->last_topology_change_port, cist->last_topology_change_port,
            IFNAMSIZ);
    assign(status->designated_root, cist->rootPriority.RootID);
    assign(status->root_path_cost,
           __be32_to_cpu(cist->rootPriority.ExtRootPathCost));
    assign(status->regional_root, cist->rootPriority.RRootID);
    assign(status->internal_path_cost,
           __be32_to_cpu(cist->rootPriority.IntRootPathCost));
    assign(status->root_port_id, cist->rootPortId);
    assign(status->root_max_age, cist->rootTimes.Max_Age);
    assign(status->root_forward_delay, cist->rootTimes.Forward_Delay);
    assign(status->bridge_max_age, br->Max_Age);
    assign(status->bridge_forward_delay, br->Forward_Delay);
    assign(status->max_hops, br->MaxHops);
    assign(status->tx_hold_count, br->Transmit_Hold_Count);
    status->protocol_version = br->ForceProtocolVersion;
    status->enabled = br->bridgeEnabled;
    assign(status->bridge_hello_time, br->Hello_Time);
    assign(status->Ageing_Time, br->Ageing_Time);
}

/* 12.8.1.2 Read MSTI Bridge Protocol Parameters */
void MSTP_IN_get_msti_bridge_status(tree_t *tree, MSTI_BridgeStatus *status)
{
    assign(status->bridge_id, tree->BridgeIdentifier);
    assign(status->time_since_topology_change,
           tree->time_since_topology_change);
    assign(status->topology_change_count, tree->topology_change_count);
    status->topology_change = tree->topology_change;
    strncpy(status->topology_change_port, tree->topology_change_port,
            IFNAMSIZ);
    strncpy(status->last_topology_change_port, tree->last_topology_change_port,
            IFNAMSIZ);
    assign(status->regional_root, tree->rootPriority.RRootID);
    assign(status->internal_path_cost,
           __be32_to_cpu(tree->rootPriority.IntRootPathCost));
    assign(status->root_port_id, tree->rootPortId);
}

/* 12.8.1.3 Set CIST Bridge Protocol Parameters */
int MSTP_IN_set_cist_bridge_config(bridge_t *br, CIST_BridgeConfig *cfg)
{
    bool changed, changedBridgeTimes, init;
    int r = 0;
    __u8 new_forward_delay, new_max_age;
    tree_t *tree;
    port_t *prt;
    per_tree_port_t *ptp;

    /* Firstly, validation */
    if(cfg->set_bridge_max_age)
    {
        new_max_age = cfg->bridge_max_age;
        if((6 > new_max_age) || (40 < new_max_age))
        {
            ERROR_BRNAME(br,
                "Bridge Max Age must be between 6 and 40 seconds");
            r = -1;
        }
    }
    else
        new_max_age = br->Max_Age;

    if(cfg->set_bridge_forward_delay)
    {
        new_forward_delay = cfg->bridge_forward_delay;
        if((4 > new_forward_delay) || (30 < new_forward_delay))
        {
            ERROR_BRNAME(br,
                "Bridge Forward Delay must be between 4 and 30 seconds");
            r = -1;
        }
    }
    else
        new_forward_delay = br->Forward_Delay;

    if(cfg->set_bridge_max_age || cfg->set_bridge_forward_delay)
    {
        if((2 * (new_forward_delay - 1)) < new_max_age)
        {
            ERROR_BRNAME(br, "Configured Bridge Times don't meet "
                "2 * (Bridge Foward Delay - 1 second) >= Bridge Max Age");
            r = -1;
        }
    }

    if(cfg->set_protocol_version)
    {
        switch(cfg->protocol_version)
        {
            case protoSTP:
            case protoRSTP:
            case protoMSTP:
                break;
            default:
                ERROR_BRNAME(br, "Bad protocol version (%d)",
                             cfg->protocol_version);
                r = -1;
        }
    }

    if(cfg->set_tx_hold_count)
    {
        if((1 > cfg->tx_hold_count) || (10 < cfg->tx_hold_count))
        {
            ERROR_BRNAME(br,
                "Transmit Hold Count must be between 1 and 10 seconds");
            r = -1;
        }
    }

    if(cfg->set_max_hops)
    {
        if((6 > cfg->max_hops) || (40 < cfg->max_hops))
        {
            ERROR_BRNAME(br, "Bridge Max Hops must be between 6 and 40");
            r = -1;
        }
    }

    if(cfg->set_bridge_hello_time)
    {
        if((1 > cfg->bridge_hello_time) || (10 < cfg->bridge_hello_time))
        {
            ERROR_BRNAME(br, "Bridge Hello Time must be between 1 and 10");
            r = -1;
        }
    }

    if(cfg->set_bridge_ageing_time)
    {
        if((10 > cfg->bridge_ageing_time)||(1000000 < cfg->bridge_ageing_time))
        {
            ERROR_BRNAME(br,
                "Bridge Ageing Time must be between 10 and 1000000 seconds");
            r = -1;
        }
    }

    if(r)
        return r;

    /* Secondly, do set */
    changed = changedBridgeTimes = init = false;

    if(cfg->set_bridge_max_age || cfg->set_bridge_forward_delay)
    {
        if(cmp(new_max_age, !=, br->Max_Age)
           || cmp(new_forward_delay, !=, br->Forward_Delay)
          )
        {
            assign(br->Max_Age, new_max_age);
            assign(br->Forward_Delay, new_forward_delay);
            changed = changedBridgeTimes = true;
        }
    }

    if((cfg->set_protocol_version)
       && (cfg->protocol_version != br->ForceProtocolVersion)
      )
    {
        br->ForceProtocolVersion = cfg->protocol_version;
        changed = init = true;
    }

    if(cfg->set_tx_hold_count)
    {
        if(cfg->tx_hold_count != br->Transmit_Hold_Count)
        {
            assign(br->Transmit_Hold_Count, cfg->tx_hold_count);
            FOREACH_PORT_IN_BRIDGE(prt, br)
                assign(prt->txCount, 0u);
            changed = true;
        }
    }

    if(cfg->set_max_hops)
    {
        if(cfg->max_hops != br->MaxHops)
        {
            assign(br->MaxHops, cfg->max_hops);
            changed = changedBridgeTimes = true;
        }
    }

    if(cfg->set_bridge_hello_time)
    {
        if(cfg->bridge_hello_time != br->Hello_Time)
        {
            INFO_BRNAME(br, "bridge hello_time new=%hhu, old=%hhu",
                        cfg->bridge_hello_time, br->Hello_Time);
            assign(br->Hello_Time, cfg->bridge_hello_time);
            changed = changedBridgeTimes = true;
        }
    }

    if(cfg->set_bridge_ageing_time)
    {
        if(cfg->bridge_ageing_time != br->Ageing_Time)
        {
            INFO_BRNAME(br, "bridge ageing_time new=%u, old=%u",
                        cfg->bridge_ageing_time, br->Ageing_Time);
            assign(br->Ageing_Time, cfg->bridge_ageing_time);
        }
    }

    /* Thirdly, finalize changes */
    if(changedBridgeTimes)
    {
        FOREACH_TREE_IN_BRIDGE(tree, br)
        {
            assign(tree->BridgeTimes.remainingHops, br->MaxHops);
            assign(tree->BridgeTimes.Forward_Delay, br->Forward_Delay);
            assign(tree->BridgeTimes.Max_Age, br->Max_Age);
            assign(tree->BridgeTimes.Hello_Time, br->Hello_Time);
        /* Comment found in rstpd by Srinivas Aji:
         * Do this for any change in BridgeTimes.
         * Otherwise we fail UNH rstp.op_D test 3.2 since when administratively
         * setting BridgeForwardDelay, etc, the values don't propagate from
         * rootTimes to designatedTimes immediately without this change.
         */
            FOREACH_PTP_IN_TREE(ptp, tree)
            {
                ptp->selected = false;
                ptp->reselect = true;
                /* TODO: change this when Hello_Time will be configurable
                 *   per-port. For now, copy Bridge's Hello_Time
                 *   to the port's Hello_Time.
                 */
                assign(ptp->portTimes.Hello_Time, br->Hello_Time);
            }
        }
    }

    if(changed && br->bridgeEnabled)
    {
        if(init)
            br_state_machines_begin(br);
        else
            br_state_machines_run(br);
    }

    return 0;
}

/* 12.8.1.4 Set MSTI Bridge Protocol Parameters */
int MSTP_IN_set_msti_bridge_config(tree_t *tree, __u8 bridge_priority)
{
    per_tree_port_t *ptp;
    __u8 valuePri;

    if(15 < bridge_priority)
    {
        ERROR_BRNAME(tree->bridge,
                     "MSTI %hu: Bridge Priority must be between 0 and 15",
                     __be16_to_cpu(tree->MSTID));
        return -1;
    }

    valuePri = bridge_priority << 4;
    if(GET_PRIORITY_FROM_IDENTIFIER(tree->BridgeIdentifier) == valuePri)
        return 0;
    SET_PRIORITY_IN_IDENTIFIER(valuePri, tree->BridgeIdentifier);
    tree->BridgePriority.RootID = tree->BridgePriority.RRootID =
        tree->BridgePriority.DesignatedBridgeID = tree->BridgeIdentifier;
    /* 12.8.1.4.4 do not require reselect, but I think it is needed,
     *  because 12.8.1.3.4.c) requires it */
    FOREACH_PTP_IN_TREE(ptp, tree)
    {
        ptp->selected = false;
        ptp->reselect = true;
    }
    return 0;
}

/* 12.8.2.1 Read CIST Port Parameters */
void MSTP_IN_get_cist_port_status(port_t *prt, CIST_PortStatus *status)
{
    per_tree_port_t *cist = GET_CIST_PTP_FROM_PORT(prt);
    /* 12.8.2.2.3 b) */
    status->uptime = (signed int)((prt->bridge)->uptime)
                     - (signed int)(cist->start_time);
    status->state = cist->state;
    assign(status->port_id, cist->portId);
    assign(status->admin_external_port_path_cost,
           prt->AdminExternalPortPathCost);
    assign(status->external_port_path_cost, prt->ExternalPortPathCost);
    assign(status->designated_root, cist->portPriority.RootID);
    assign(status->designated_external_cost,
           __be32_to_cpu(cist->portPriority.ExtRootPathCost));
    assign(status->designated_bridge, cist->portPriority.DesignatedBridgeID);
    assign(status->designated_port, cist->portPriority.DesignatedPortID);
    assign(status->designated_regional_root, cist->portPriority.RRootID);
    assign(status->designated_internal_cost,
           __be32_to_cpu(cist->portPriority.IntRootPathCost));
    status->tc_ack = prt->tcAck;
    assign(status->port_hello_time, cist->portTimes.Hello_Time);
    status->admin_edge_port = prt->AdminEdgePort;
    status->auto_edge_port = prt->AutoEdge;
    status->oper_edge_port = prt->operEdge;
    status->enabled = prt->portEnabled;
    status->admin_p2p = prt->AdminP2P;
    status->oper_p2p = prt->operPointToPointMAC;
    status->restricted_role = prt->restrictedRole;
    status->restricted_tcn = prt->restrictedTcn;
    status->role = cist->role;
    status->disputed = cist->disputed;
    assign(status->admin_internal_port_path_cost,
           cist->AdminInternalPortPathCost);
    assign(status->internal_port_path_cost, cist->InternalPortPathCost);
    status->bpdu_guard_port = prt->BpduGuardPort;
    status->bpdu_guard_error = prt->BpduGuardError;
    status->network_port = prt->NetworkPort;
    status->ba_inconsistent = prt->BaInconsistent;
    status->bpdu_filter_port = prt->bpduFilterPort;
    status->num_rx_bpdu_filtered = prt->num_rx_bpdu_filtered;
    status->num_rx_bpdu = prt->num_rx_bpdu;
    status->num_rx_tcn = prt->num_rx_tcn;
    status->num_tx_bpdu = prt->num_tx_bpdu;
    status->num_tx_tcn = prt->num_tx_tcn;
    status->num_trans_fwd = prt->num_trans_fwd;
    status->num_trans_blk = prt->num_trans_blk;
    status->rcvdBpdu = prt->rcvdBpdu;
    status->rcvdRSTP = prt->rcvdRSTP;
    status->rcvdSTP = prt->rcvdSTP;
    status->rcvdTcAck = prt->rcvdTcAck;
    status->rcvdTcn = prt->rcvdTcn;
    status->sendRSTP = prt->sendRSTP;
}

/* 12.8.2.2 Read MSTI Port Parameters */
void MSTP_IN_get_msti_port_status(per_tree_port_t *ptp,
                                  MSTI_PortStatus *status)
{
    status->uptime = (signed int)((ptp->port->bridge)->uptime)
                     - (signed int)(ptp->start_time);
    status->state = ptp->state;
    assign(status->port_id, ptp->portId);
    assign(status->admin_internal_port_path_cost,
           ptp->AdminInternalPortPathCost);
    assign(status->internal_port_path_cost, ptp->InternalPortPathCost);
    assign(status->designated_regional_root, ptp->portPriority.RRootID);
    assign(status->designated_internal_cost,
           __be32_to_cpu(ptp->portPriority.IntRootPathCost));
    assign(status->designated_bridge, ptp->portPriority.DesignatedBridgeID);
    assign(status->designated_port, ptp->portPriority.DesignatedPortID);
    status->role = ptp->role;
    status->disputed = ptp->disputed;
}

/* 12.8.2.3 Set CIST port parameters */
int MSTP_IN_set_cist_port_config(port_t *prt, CIST_PortConfig *cfg)
{
    bool changed;
    __u32 new_ExternalPathCost;
    bool new_p2p;
    per_tree_port_t *cist;
    bridge_t *br = prt->bridge;

    /* Firstly, validation */
    if(cfg->set_admin_p2p)
    {
        switch(cfg->admin_p2p)
        {
            case p2pAuto:
            case p2pForceTrue:
            case p2pForceFalse:
                break;
            default:
                cfg->admin_p2p = p2pAuto;
        }
    }

    /* Secondly, do set */
    changed = false;

    if(cfg->set_admin_external_port_path_cost)
    {
        prt->AdminExternalPortPathCost = cfg->admin_external_port_path_cost;
        new_ExternalPathCost = (0 == prt->AdminExternalPortPathCost) ?
                                 compute_pcost(GET_PORT_SPEED(prt))
                               : prt->AdminExternalPortPathCost;
        if(prt->ExternalPortPathCost != new_ExternalPathCost)
        {
            assign(prt->ExternalPortPathCost, new_ExternalPathCost);
            changed = true;
            /* 12.8.2.3.4 */
            cist = GET_CIST_PTP_FROM_PORT(prt);
            cist->selected = false;
            cist->reselect = true;
        }
    }

    if(cfg->set_admin_p2p)
    {
        prt->AdminP2P = cfg->admin_p2p;
        switch(prt->AdminP2P)
        {
            case p2pForceTrue:
                new_p2p = true;
                break;
            case p2pForceFalse:
                new_p2p = false;
                break;
            case p2pAuto:
            default:
                new_p2p = !!GET_PORT_DUPLEX(prt);
                break;
        }
        if(prt->operPointToPointMAC != new_p2p)
        {
            prt->operPointToPointMAC = new_p2p;
            changed = true;
        }
    }

    if(cfg->set_admin_edge_port)
    {
        if(prt->AdminEdgePort != cfg->admin_edge_port)
        {
            prt->AdminEdgePort = cfg->admin_edge_port;
            BDSM_begin(prt);
            changed = true;
        }
    }

    if(cfg->set_auto_edge_port)
    {
        if(prt->AutoEdge != cfg->auto_edge_port)
        {
            prt->AutoEdge = cfg->auto_edge_port;
            changed = true;
        }
    }

    if(cfg->set_restricted_role)
    {
        if(prt->restrictedRole != cfg->restricted_role)
        {
            prt->restrictedRole = cfg->restricted_role;
            changed = true;
        }
    }

    if(cfg->set_restricted_tcn)
    {
        if(prt->restrictedTcn != cfg->restricted_tcn)
        {
            prt->restrictedTcn = cfg->restricted_tcn;
            changed = true;
        }
    }

    if(cfg->set_bpdu_guard_port)
    {
        if(prt->BpduGuardPort != cfg->bpdu_guard_port)
        {
            prt->BpduGuardPort = cfg->bpdu_guard_port;
            INFO_PRTNAME(br, prt,"BpduGuardPort new=%d", prt->BpduGuardPort);
        }
    }

    if(cfg->set_network_port)
    {
        if(prt->NetworkPort != cfg->network_port)
        {
            prt->NetworkPort = cfg->network_port;
            INFO_PRTNAME(br, prt, "NetworkPort new=%d", prt->NetworkPort);
            /* When Network port config is removed and bridge assurance
             * inconsistency is set, clear the inconsistency.
             */
            if(!prt->NetworkPort && prt->BaInconsistent)
            {
                prt->BaInconsistent = false;
                INFO_PRTNAME(br, prt, "Clear Bridge assurance inconsistency");
            }
            changed = true;
        }
    }

    if(cfg->set_dont_txmt)
    {
        if(prt->dontTxmtBpdu != cfg->dont_txmt)
        {
            prt->dontTxmtBpdu = cfg->dont_txmt;
            INFO_PRTNAME(br, prt, "donttxmt new=%d", prt->dontTxmtBpdu);
        }
    }

    if(cfg->set_bpdu_filter_port)
    {
        if (prt->bpduFilterPort != cfg->bpdu_filter_port)
        {
            prt->bpduFilterPort = cfg->bpdu_filter_port;
            prt->num_rx_bpdu_filtered = 0;
            INFO_PRTNAME(br, prt,"bpduFilterPort new=%d", prt->bpduFilterPort);
        }
    }

    if(changed && prt->portEnabled)
        br_state_machines_run(prt->bridge);

    return 0;
}

/* 12.8.2.4 Set MSTI port parameters */
int MSTP_IN_set_msti_port_config(per_tree_port_t *ptp, MSTI_PortConfig *cfg)
{
    __u8 valuePri;
    __u32 new_InternalPathCost;
    bool changed = false;
    port_t *prt = ptp->port;
    bridge_t *br = prt->bridge;

    if(cfg->set_port_priority)
    {
        if(15 < cfg->port_priority)
        {
            ERROR_MSTINAME(br, prt, ptp,
                           "Port Priority must be between 0 and 15");
            return -1;
        }
        valuePri = cfg->port_priority << 4;
        if(GET_PRIORITY_FROM_IDENTIFIER(ptp->portId) != valuePri)
        {
            SET_PRIORITY_IN_IDENTIFIER(valuePri, ptp->portId);
            changed = true;
        }
    }

    if(cfg->set_admin_internal_port_path_cost)
    {
        ptp->AdminInternalPortPathCost = cfg->admin_internal_port_path_cost;
        new_InternalPathCost = (0 == ptp->AdminInternalPortPathCost) ?
                                 compute_pcost(GET_PORT_SPEED(prt))
                               : ptp->AdminInternalPortPathCost;
        if(ptp->InternalPortPathCost != new_InternalPathCost)
        {
            assign(ptp->InternalPortPathCost, new_InternalPathCost);
            changed = true;
        }
    }

    if(changed && prt->portEnabled)
    {
        /* 12.8.2.4.4 */
        ptp->selected = false;
        ptp->reselect = true;

        br_state_machines_run(br);
    }

    return 0;
}

/* 12.8.2.5 Force BPDU Migration Check */
int MSTP_IN_port_mcheck(port_t *prt)
{
    bridge_t *br = prt->bridge;
    per_tree_port_t *cist = GET_CIST_PTP_FROM_PORT(prt);

    if(rstpVersion(br) && prt->portEnabled && br->bridgeEnabled)
    {
        prt->mcheck = true;
        cist->proposing = true;
        br_state_machines_run(br);
    }

    return 0;
}

/* 12.10.3.8 Set VID to FID allocation */
bool MSTP_IN_set_vid2fid(bridge_t *br, __u16 vid, __u16 fid)
{
    bool vid2mstid_changed;

    if((vid < 1) || (vid > MAX_VID) || (fid > MAX_FID))
    {
        ERROR_BRNAME(br, "Error allocating VID(%hu) to FID(%hu)", vid, fid);
        return false;
    }

    vid2mstid_changed =
        (br->fid2mstid[fid] != br->fid2mstid[br->vid2fid[vid]]);
    br->vid2fid[vid] = fid;
    if(vid2mstid_changed)
    {
        RecalcConfigDigest(br);
        br_state_machines_begin(br);
    }

    return true;
}

/* Set all VID-to-FID mappings at once */
bool MSTP_IN_set_all_vids2fids(bridge_t *br, __u16 *vids2fids)
{
    bool vid2mstid_changed;
    int vid;

    vid2mstid_changed = false;
    for(vid = 1; vid <= MAX_VID; ++vid)
    {
        if(vids2fids[vid] > MAX_FID)
        { /* Incorrect value == keep prev value */
            vids2fids[vid] = br->vid2fid[vid];
            continue;
        }
        if(br->fid2mstid[vids2fids[vid]] != br->fid2mstid[br->vid2fid[vid]])
            vid2mstid_changed = true;
    }
    memcpy(br->vid2fid, vids2fids, sizeof(br->vid2fid));
    if(vid2mstid_changed)
    {
        RecalcConfigDigest(br);
        br_state_machines_begin(br);
    }

    return true;
}

/* 12.12.2.2 Set FID to MSTID allocation */
bool MSTP_IN_set_fid2mstid(bridge_t *br, __u16 fid, __u16 mstid)
{
    tree_t *tree;
    __be16 MSTID;
    bool found;
    int vid;

    if(fid > MAX_FID)
    {
        ERROR_BRNAME(br, "Bad FID(%hu)", fid);
        return false;
    }

    MSTID = __cpu_to_be16(mstid);
    found = false;
    FOREACH_TREE_IN_BRIDGE(tree, br)
    {
        if(tree->MSTID == MSTID)
        {
            found = true;
            break;
        }
    }
    if(!found)
    {
        ERROR_BRNAME(br, "MSTID(%hu) not found", mstid);
        return false;
    }

    if(br->fid2mstid[fid] != MSTID)
    {
        br->fid2mstid[fid] = MSTID;
        /* check if there are VLANs using this FID */
        for(vid = 1; vid <= MAX_VID; ++vid)
        {
            if(br->vid2fid[vid] == fid)
            {
                RecalcConfigDigest(br);
                br_state_machines_begin(br);
                break;
            }
        }
    }

    return true;
}

/* Set all FID-to-MSTID mappings at once */
bool MSTP_IN_set_all_fids2mstids(bridge_t *br, __u16 *fids2mstids)
{
    tree_t *tree;
    __be16 MSTID[MAX_FID + 1];
    bool found, vid2mstid_changed;
    int fid, vid;
    __be16 prev_vid2mstid[MAX_VID + 2];

    for(fid = 0; fid <= MAX_FID; ++fid)
    {
        if(fids2mstids[fid] > MAX_MSTID)
        { /* Incorrect value == keep prev value */
            fids2mstids[fid] = __be16_to_cpu(MSTID[fid] = br->fid2mstid[fid]);
        }
        else
            MSTID[fid] = __cpu_to_be16(fids2mstids[fid]);
        found = false;
        FOREACH_TREE_IN_BRIDGE(tree, br)
        {
            if(tree->MSTID == MSTID[fid])
            {
                found = true;
                break;
            }
        }
        if(!found)
        {
            ERROR_BRNAME(br,
                "Error allocating FID(%hu) to MSTID(%hu): MSTID not found",
                fid, fids2mstids[fid]);
            return false;
        }
    }

    for(vid = 1; vid <= MAX_VID; ++vid)
        prev_vid2mstid[vid] = br->fid2mstid[br->vid2fid[vid]];
    memcpy(br->fid2mstid, MSTID, sizeof(br->fid2mstid));
    vid2mstid_changed = false;
    for(vid = 1; vid <= MAX_VID; ++vid)
    {
        if(prev_vid2mstid[vid] != br->fid2mstid[br->vid2fid[vid]])
        {
            vid2mstid_changed = true;
            break;
        }
    }
    if(vid2mstid_changed)
    {
        RecalcConfigDigest(br);
        br_state_machines_begin(br);
    }

    return true;
}

/* 12.12.1.1 Read MSTI List */
bool MSTP_IN_get_mstilist(bridge_t *br, int *num_mstis, __u16 *mstids)
{
    tree_t *tree;

    *num_mstis = 0;
    FOREACH_TREE_IN_BRIDGE(tree, br)
    {
        mstids[*num_mstis] = __be16_to_cpu(tree->MSTID);
        /* Check for "<", not for "<=", as num_mstis include CIST */
        if(MAX_IMPLEMENTATION_MSTIS < ++(*num_mstis))
            break;
    }

    return true;
}

/* 12.12.1.2 Create MSTI */
bool MSTP_IN_create_msti(bridge_t *br, __u16 mstid)
{
    tree_t *tree, *tree_after, *new_tree;
    per_tree_port_t *ptp, *nxt, *ptp_after, *new_ptp;
    int num_of_mstis;
    __be16 MSTID;

    if((mstid < 1) || (mstid > MAX_MSTID))
    {
        ERROR_BRNAME(br, "Bad MSTID(%hu)", mstid);
        return false;
    }

    MSTID = __cpu_to_be16(mstid);
    /* Find place where to insert new MSTID.
     * Also check if such MSTID is already in the list.
     * Also count existing mstis.
     */
    tree_after = NULL;
    num_of_mstis = 0;
    FOREACH_TREE_IN_BRIDGE(tree, br)
    {
        if(tree->MSTID == MSTID)
        {
            INFO_BRNAME(br, "MSTID(%hu) is already in the list", mstid);
            return true; /* yes, it is success */
        }
        if(cmp(tree->MSTID, <, MSTID))
            tree_after = tree;
        ++num_of_mstis;
    }
    /* Sanity check */
    if(NULL == tree_after)
    {
        ERROR_BRNAME(br, "Can't add MSTID(%hu): no CIST in the list", mstid);
        return false;
    }
    /* End of Sanity check */

    /* Check for "<", not for "<=", as num_of_mstis include CIST */
    if(MAX_IMPLEMENTATION_MSTIS < num_of_mstis)
    {
        ERROR_BRNAME(br, "Can't add MSTID(%hu): maximum count(%u) reached",
                     mstid, MAX_IMPLEMENTATION_MSTIS);
        return false;
    }

    /* Create new tree and its list of PerTreePort structures */
    tree = GET_CIST_TREE(br);
    if(!(new_tree=create_tree(br,tree->BridgeIdentifier.s.mac_address,MSTID)))
        return false;

    FOREACH_PTP_IN_TREE(ptp_after, tree_after)
    {
        if(!(new_ptp = create_ptp(new_tree, ptp_after->port)))
        {
            /* Remove and free all previously created entries in tree's list */
            list_for_each_entry_safe(ptp, nxt, &new_tree->ports, tree_list)
            {
                list_del(&ptp->port_list);
                list_del(&ptp->tree_list);
                free(ptp);
            }
            return false;
        }
        list_add(&new_ptp->port_list, &ptp_after->port_list);
        list_add_tail(&new_ptp->tree_list, &new_tree->ports);
    }

    list_add(&new_tree->bridge_list, &tree_after->bridge_list);
    /* There are no FIDs allocated to this MSTID, so VID-to-MSTID mapping
     *  did not change. So, no need in RecalcConfigDigest.
     * Just initialize state machines for this tree.
     */
    tree_state_machines_begin(new_tree);
    return true;
}

/* 12.12.1.3 Delete MSTI */
bool MSTP_IN_delete_msti(bridge_t *br, __u16 mstid)
{
    tree_t *tree;
    per_tree_port_t *ptp, *nxt;
    int fid;
    bool found;
    __be16 MSTID = __cpu_to_be16(mstid);

    if((mstid < 1) || (mstid > MAX_MSTID))
    {
        ERROR_BRNAME(br, "Bad MSTID(%hu)", mstid);
        return false;
    }

    /* Check if there are FIDs associated with this MSTID */
    for(fid = 0; fid <= MAX_FID; ++fid)
    {
        if(br->fid2mstid[fid] == MSTID)
        {
            ERROR_BRNAME(br,
                "Can't delete MSTID(%hu): there are FIDs allocated to it",
                mstid);
            return false;
        }
    }

    found = false;
    FOREACH_TREE_IN_BRIDGE(tree, br)
    {
        if(tree->MSTID == MSTID)
        {
            found = true;
            break;
        }
    }
    if(!found)
    {
        INFO_BRNAME(br, "MSTID(%hu) is not in the list", mstid);
        return true; /* yes, it is success */
    }

    list_del(&tree->bridge_list);
    list_for_each_entry_safe(ptp, nxt, &tree->ports, tree_list)
    {
        list_del(&ptp->port_list);
        list_del(&ptp->tree_list);
        free(ptp);
    }
    free(tree);

    /* There are no FIDs allocated to this MSTID, so VID-to-MSTID mapping
     *  did not change. So, no need in RecalcConfigDigest.
     * Give state machine a spare run, just for the case...
     */
    br_state_machines_run(br);
    return true;
}

/* 12.12.3.4 Set MST Configuration Identifier Elements */
void MSTP_IN_set_mst_config_id(bridge_t *br, __u16 revision, __u8 *name)
{
    __be16 valueRevision = __cpu_to_be16(revision);
    bool changed = (0 != strncmp((char *)name, (char *)br->MstConfigId.s.configuration_name,
                                 sizeof(br->MstConfigId.s.configuration_name))
                   )
                   || (valueRevision != br->MstConfigId.s.revision_level);

    if(changed)
    {
        assign(br->MstConfigId.s.revision_level, valueRevision);
        memset(br->MstConfigId.s.configuration_name, 0,
               sizeof(br->MstConfigId.s.configuration_name));
        strncpy((char *)br->MstConfigId.s.configuration_name, (char *)name,
                sizeof(br->MstConfigId.s.configuration_name) - 1);
        br_state_machines_begin(br);
    }
}

/*
 * If hint_SetToYes == true, some tcWhile in this tree has non-zero value.
 * If hint_SetToYes == false, some tcWhile in this tree has just became zero,
 *  so we should check all other tcWhile's in this tree.
 */
static void set_TopologyChange(tree_t *tree, bool hint_SetToYes, port_t *port)
{
    per_tree_port_t *ptp;
    bool prev_tc_not_set = !tree->topology_change;

    if(hint_SetToYes)
    {
        tree->topology_change = true;
        tree->time_since_topology_change = 0;
        if(prev_tc_not_set)
            ++(tree->topology_change_count);
        strncpy(tree->topology_change_port, tree->last_topology_change_port,
                IFNAMSIZ);
        strncpy(tree->last_topology_change_port, port->sysdeps.name, IFNAMSIZ);
        return;
    }

    /* Some tcWhile has just became zero. Check if we need reset
     * topology_change flag */
    if(prev_tc_not_set)
        return;

    tree->topology_change = false;
    FOREACH_PTP_IN_TREE(ptp, tree)
    {
        if(0 != ptp->tcWhile)
        {
            tree->topology_change = true;
            tree->time_since_topology_change = 0;
            return;
        }
    }
}

/* Helper functions, compare two priority vectors */
static bool samePriorityAndTimers(port_priority_vector_t *vec1,
                                  port_priority_vector_t *vec2,
                                  times_t *time1,
                                  times_t *time2,
                                  bool cist)
{
    if(cist)
    {
        if(cmp(time1->Forward_Delay, !=, time2->Forward_Delay))
            return false;
        if(cmp(time1->Max_Age, !=, time2->Max_Age))
            return false;
        if(cmp(time1->Message_Age, !=, time2->Message_Age))
            return false;
        if(cmp(time1->Hello_Time, !=, time2->Hello_Time))
            return false;

        if(cmp(vec1->RootID, !=, vec2->RootID))
            return false;
        if(cmp(vec1->ExtRootPathCost, !=, vec2->ExtRootPathCost))
            return false;
    }

    if(cmp(time1->remainingHops, !=, time2->remainingHops))
        return false;

    if(cmp(vec1->RRootID, !=, vec2->RRootID))
        return false;
    if(cmp(vec1->IntRootPathCost, !=, vec2->IntRootPathCost))
        return false;
    if(cmp(vec1->DesignatedBridgeID, !=, vec2->DesignatedBridgeID))
        return false;
    if(cmp(vec1->DesignatedPortID, !=, vec2->DesignatedPortID))
        return false;

    return true;
}

static bool betterorsamePriority(port_priority_vector_t *vec1,
                                 port_priority_vector_t *vec2,
                                 port_identifier_t pId1,
                                 port_identifier_t pId2,
                                 bool cist)
{
    int result;

    if(cist)
    {
        if(0 < (result = _ncmp(vec1->RootID, vec2->RootID)))
            return false; /* worse */
        else if(0 > result)
            return true; /* better */
        /* The same. Check further. */
        if(0 < (result = _ncmp(vec1->ExtRootPathCost, vec2->ExtRootPathCost)))
            return false; /* worse */
        else if(0 > result)
            return true; /* better */
        /* The same. Check further. */
    }

    if(0 < (result = _ncmp(vec1->RRootID, vec2->RRootID)))
        return false; /* worse */
    else if(0 > result)
        return true; /* better */
    /* The same. Check further. */

    if(0 < (result = _ncmp(vec1->IntRootPathCost, vec2->IntRootPathCost)))
        return false; /* worse */
    else if(0 > result)
        return true; /* better */
    /* The same. Check further. */

    if(0 < (result = _ncmp(vec1->DesignatedBridgeID, vec2->DesignatedBridgeID)))
        return false; /* worse */
    else if(0 > result)
        return true; /* better */
    /* The same. Check further. */

    if(0 < (result = _ncmp(vec1->DesignatedPortID, vec2->DesignatedPortID)))
        return false; /* worse */
    else if(0 > result)
        return true; /* better */
    /* The same. Check further. */

    /* Port ID is a tie-breaker */
    return cmp(pId1, <=, pId2);
}

/* 13.26.1 betterorsameInfo */
static bool betterorsameInfo(per_tree_port_t *ptp, port_info_origin_t newInfoIs)
{
    if((ioReceived == newInfoIs) && (ioReceived == ptp->infoIs))
        return betterorsamePriority(&ptp->msgPriority,
                                    &ptp->portPriority,
                                    0, 0, (0 == ptp->MSTID));
    else if((ioMine == newInfoIs) && (ioMine == ptp->infoIs))
        return betterorsamePriority(&ptp->designatedPriority,
                                    &ptp->portPriority,
                                    0, 0, (0 == ptp->MSTID));
    return false;
}

/* 13.26.2 clearAllRcvdMsgs */
static bool clearAllRcvdMsgs(port_t *prt, bool dry_run)
{
    per_tree_port_t *ptp;

    if(dry_run)
    {
        FOREACH_PTP_IN_PORT(ptp, prt)
            if(ptp->rcvdMsg)
                return true;
        return false;
    }

    FOREACH_PTP_IN_PORT(ptp, prt)
        ptp->rcvdMsg = false;

    return false;
}

/* 13.26.3 clearReselectTree */
static void clearReselectTree(tree_t *tree)
{
    per_tree_port_t *ptp;

    FOREACH_PTP_IN_TREE(ptp, tree)
        ptp->reselect = false;
}

/* 13.26.4 fromSameRegion */
static bool fromSameRegion(port_t *prt)
{
    /* Check for rcvdRSTP is superfluous here */
    if((protoMSTP > prt->rcvdBpduData.protocolVersion)/* || (!prt->rcvdRSTP)*/)
        return false;
    return cmp(prt->bridge->MstConfigId,
               ==, prt->rcvdBpduData.mstConfigurationIdentifier);
}

/* 13.26.5 newTcWhile */
static void newTcWhile(per_tree_port_t *ptp)
{
    if(0 != ptp->tcWhile)
        return;

    tree_t *tree = ptp->tree;
    port_t *prt = ptp->port;

    if(prt->sendRSTP)
    {
        per_tree_port_t *cist = GET_CIST_PTP_FROM_PORT(prt);

        ptp->tcWhile = cist->portTimes.Hello_Time + 1;
        set_TopologyChange(tree, true, prt);

        if(0 == ptp->MSTID)
            prt->newInfo = true;
        else
            prt->newInfoMsti = true;
        return;
    }

    times_t *times = &tree->rootTimes;

    ptp->tcWhile = times->Max_Age + times->Forward_Delay;
    set_TopologyChange(tree, true, prt);
}

/* 13.26.6 rcvInfo */
static port_info_t rcvInfo(per_tree_port_t *ptp)
{
    msti_configuration_message_t *msti_msg;
    per_tree_port_t *ptp_1;
    bool roleIsDesignated, cist;
    bool msg_Better_port, msg_SamePriorityAndTimers_port;
    port_priority_vector_t *mPri = &(ptp->msgPriority);
    times_t *mTimes = &(ptp->msgTimes);
    port_t *prt = ptp->port;
    bpdu_t *b = &(prt->rcvdBpduData);

    if(bpduTypeTCN == b->bpduType)
    {
        prt->rcvdTcn = true;
        FOREACH_PTP_IN_PORT(ptp_1, prt)
            ptp_1->rcvdTc = true;
        return OtherInfo;
    }

    if(0 == ptp->MSTID)
    { /* CIST */
        if(protoSTP != b->protocolVersion)
        {
            switch(BPDU_FLAGS_ROLE_GET(b->flags))
            {
                case encodedRoleAlternateBackup:
                case encodedRoleRoot:
                    roleIsDesignated = false;
                    break;
                case encodedRoleDesignated:
                    roleIsDesignated = true;
                    break;
                case encodedRoleMaster:
                    /* 802.1D-2004 S9.2.9 P61. The Unknown value of Port Role
                     * cannot be generated by a valid implementation; however,
                     * this value is accepted on receipt. roleMaster in MSTP is
                     * roleUnknown in RSTP.
                     * NOTE.If the Unknown value of the Port Role parameter is
                     * received, the state machines will effectively treat the RST
                     * BPDU as if it were a Configuration BPDU
                     */
                    if(protoRSTP == b->protocolVersion)
                    {
                        roleIsDesignated = true;
                        break;
                    }
                    else
                    {
                        return OtherInfo;
                    }
                    break;
                default:
                    return OtherInfo;
            }
        }
        else
        { /* 13.26.6.NOTE: A Configuration BPDU implicitly conveys a
           *   Designated Port Role */
            roleIsDesignated = true;
        }
        cist = true;

        assign(mPri->RRootID, b->cistRRootID);
        assign(mPri->DesignatedPortID, b->cistPortID);
        assign(mPri->RootID, b->cistRootID);
        assign(mPri->ExtRootPathCost, b->cistExtRootPathCost);
        /* messageTimes */
#define NEAREST_WHOLE_SECOND(msgTime)  \
    ((128 > msgTime[1]) ? msgTime[0] : msgTime[0] + 1)
        mTimes->Forward_Delay = NEAREST_WHOLE_SECOND(b->ForwardDelay);
        mTimes->Max_Age = NEAREST_WHOLE_SECOND(b->MaxAge);
        mTimes->Message_Age = NEAREST_WHOLE_SECOND(b->MessageAge);
        mTimes->Hello_Time = NEAREST_WHOLE_SECOND(b->HelloTime);
        if(protoMSTP > b->protocolVersion)
        { /* STP Configuration BPDU or RST BPDU */
            assign(mPri->IntRootPathCost, __constant_cpu_to_be32(0));
            assign(mPri->DesignatedBridgeID, b->cistRRootID);
            /* messageTimes.remainingHops */
            assign(mTimes->remainingHops, prt->bridge->MaxHops);
        }
        else
        { /* MST BPDU */
            assign(mPri->IntRootPathCost, b->cistIntRootPathCost);
            assign(mPri->DesignatedBridgeID, b->cistBridgeID);
            /* messageTimes.remainingHops */
            assign(mTimes->remainingHops, b->cistRemainingHops);
        }
    }
    else
    { /* MSTI */
        if(protoMSTP > b->protocolVersion)
            return OtherInfo;
        msti_msg = ptp->rcvdMstiConfig;
        switch(BPDU_FLAGS_ROLE_GET(msti_msg->flags))
        {
            case encodedRoleAlternateBackup:
            case encodedRoleRoot:
                roleIsDesignated = false;
                break;
            case encodedRoleDesignated:
                roleIsDesignated = true;
                break;
            default:
                return OtherInfo;
        }
        cist = false;

        assign(mPri->RRootID, msti_msg->mstiRRootID);
        assign(mPri->IntRootPathCost, msti_msg->mstiIntRootPathCost);
        /* Build MSTI DesignatedBridgeID */
        assign(mPri->DesignatedBridgeID, b->cistBridgeID);
        assign(mPri->DesignatedBridgeID.s.priority, ptp->MSTID);
        SET_PRIORITY_IN_IDENTIFIER(msti_msg->bridgeIdentifierPriority,
                                   mPri->DesignatedBridgeID);
        /* Build MSTI DesignatedPortID */
        assign(mPri->DesignatedPortID, b->cistPortID);
        SET_PRIORITY_IN_IDENTIFIER(msti_msg->portIdentifierPriority,
                                   mPri->DesignatedPortID);
        /* messageTimes */
        assign(mTimes->remainingHops, msti_msg->remainingHops);
    }

    msg_Better_port = !betterorsamePriority(&(ptp->portPriority), mPri,
                                            0, 0, cist);
    if(roleIsDesignated)
    {
        /* a).1) */
        if(msg_Better_port
           || ((0 == memcmp(mPri->DesignatedBridgeID.s.mac_address,
                            ptp->portPriority.DesignatedBridgeID.s.mac_address,
                            ETH_ALEN)
               )
               && (0 == ((mPri->DesignatedPortID
                          ^ ptp->portPriority.DesignatedPortID
                         ) & __constant_cpu_to_be16(0x0FFF)
                        )
                  )
              )
          )
            return SuperiorDesignatedInfo;

        /* a).2) */
        /* We already know that msgPriority _IS_NOT_BETTER_than portPriority.
         * So, if msgPriority _IS_SAME_OR_BETTER_than portPriority then
         *   msgPriority _IS_SAME_as portPriority.
        */
        msg_SamePriorityAndTimers_port =
            samePriorityAndTimers(mPri, &(ptp->portPriority),
                                  mTimes, &(ptp->portTimes),
                                  cist);
        if((!msg_SamePriorityAndTimers_port)
           && betterorsamePriority(mPri, &(ptp->portPriority), 0, 0, cist)
          )
            return SuperiorDesignatedInfo;

        /* b) */
        if(msg_SamePriorityAndTimers_port && (ioReceived == ptp->infoIs))
            return RepeatedDesignatedInfo;

        /* c) */
        return InferiorDesignatedInfo;
    }

    /* d) */
    if(!msg_Better_port)
        return InferiorRootAlternateInfo;

    return OtherInfo;
}

/* 13.26.7 recordAgreement */
static void recordAgreement(per_tree_port_t *ptp)
{
    bool cist_agreed, cist_proposing;
    per_tree_port_t *cist;
    port_t *prt = ptp->port;
    bpdu_t *b = &(prt->rcvdBpduData);

    if(0 == ptp->MSTID)
    { /* CIST */
        if(rstpVersion(prt->bridge) && prt->operPointToPointMAC
           && (b->flags & (1 << offsetAgreement))
          )
        {
            ptp->agreed = true;
            ptp->proposing = false;
        }
        else
            ptp->agreed = false;
        cist_agreed = ptp->agreed;
        cist_proposing = ptp->proposing;
        if(!prt->rcvdInternal)
            list_for_each_entry_continue(ptp, &prt->trees, port_list)
            {
                ptp->agreed = cist_agreed;
                ptp->proposing = cist_proposing;
            }
        return;
    }
    /* MSTI */
    cist = GET_CIST_PTP_FROM_PORT(prt);
    if(prt->operPointToPointMAC 
       && cmp(b->cistRootID, ==, cist->portPriority.RootID)
       && cmp(b->cistExtRootPathCost, ==, cist->portPriority.ExtRootPathCost)
       && cmp(b->cistRRootID, ==, cist->portPriority.RRootID)
       && (ptp->rcvdMstiConfig->flags & (1 << offsetAgreement))
      )
    {
        ptp->agreed = true;
        ptp->proposing = false;
    }
    else
        ptp->agreed = false;
}

/* 13.26.8 recordDispute */
static void recordDispute(per_tree_port_t *ptp)
{
    port_t *prt;

    if(0 == ptp->MSTID)
    { /* CIST */
        prt = ptp->port;
        /* 802.1Q-2005(-2011) is somewhat unclear for the case
         *  (!prt->rcvdInternal): if we should record dispute for all MSTIs
         *  unconditionally or only when CIST Learning flag is set in BPDU.
         * I guess that in this case MSTIs should be in sync with CIST
         * so record dispute for the MSTIs only when the same is done for CIST.
         * Additional supporting argument to this guess is that in
         *  setTcFlags() we do the same.
         * But that is only a guess and I could be wrong here ;)
         */
        if(prt->rcvdBpduData.flags & (1 << offsetLearnig))
        {
            ptp->disputed = true;
            ptp->agreed = false;
            if(!prt->rcvdInternal)
                list_for_each_entry_continue(ptp, &prt->trees, port_list)
                {
                    ptp->disputed = true;
                    ptp->agreed = false;
                }
        }
        return;
    }
    /* MSTI */
    if(ptp->rcvdMstiConfig->flags & (1 << offsetLearnig))
    {
        ptp->disputed = true;
        ptp->agreed = false;
    }
}

/* 13.26.9 recordMastered */
static void recordMastered(per_tree_port_t *ptp)
{
    port_t *prt = ptp->port;

    if(0 == ptp->MSTID)
    { /* CIST */
        if(!prt->rcvdInternal)
            list_for_each_entry_continue(ptp, &prt->trees, port_list)
                ptp->mastered = false;
        return;
    }
    /* MSTI */
    ptp->mastered = prt->operPointToPointMAC
                    && (ptp->rcvdMstiConfig->flags & (1 << offsetMaster));
}

/* 13.26.f) recordPriority */
static void recordPriority(per_tree_port_t *ptp)
{
    assign(ptp->portPriority, ptp->msgPriority);
}

/* 13.26.10 recordProposal */
static void recordProposal(per_tree_port_t *ptp)
{
    bool cist_proposed;
    port_t *prt;

    /* 802.1Q-2005 says to check if received message conveys
     *  a Designated Port Role. But there is no need in this check,
     *  as it is always true. This function is called only in two states:
     *  PISM_SUPERIOR_DESIGNATED and PISM_REPEATED_DESIGNATED, which
     *  can be entered only if rcvInfo returns
     *  SuperiorDesignatedInfo or RepeatedDesignatedInfo.
     *  Which in turn can only happen if message conveys designated role
     *   (see rcvInfo).
     */
    if(0 == ptp->MSTID)
    { /* CIST */
        prt = ptp->port;
        if(prt->rcvdBpduData.flags & (1 << offsetProposal))
            ptp->proposed = true;
        cist_proposed = ptp->proposed;
        if(!prt->rcvdInternal)
            list_for_each_entry_continue(ptp, &prt->trees, port_list)
                ptp->proposed = cist_proposed;
        return;
    }
    /* MSTI */
    if(ptp->rcvdMstiConfig->flags & (1 << offsetProposal))
        ptp->proposed = true;
}

/* 13.26.11 recordTimes */
static void recordTimes(per_tree_port_t *ptp)
{
    /* 802.1Q-2005 and 802.1D-2004 both say that we have to copy
     *   Hello_Time from msgTimes to portTimes.
     * 802.1Q-2011, on the other hand, says that Hello_Time should be set
     *   to the default here.
     * As we have configurable Hello_Time, I choose the third option:
     *   preserve the configured Hello_Time, It is in accordance with the
     *   spirit of 802.1Q-2011, if we allow Hello_Time to be configurable.
     */
    __u8 prev_Hello_Time = 0;
    assign(prev_Hello_Time, ptp->portTimes.Hello_Time);
    assign(ptp->portTimes, ptp->msgTimes);
    assign(ptp->portTimes.Hello_Time, prev_Hello_Time);
}

/* 13.24.s) + 17.19.7 of 802.1D : fdbFlush */
static void set_fdbFlush(per_tree_port_t *ptp)
{
    port_t *prt = ptp->port;

    if(prt->operEdge || prt->deleted)
    {
        ptp->fdbFlush = false;
        return;
    }

    bridge_t *br = prt->bridge;

    if(rstpVersion(br))
    {
        ptp->fdbFlush = true;
        ptp->calledFromFlushRoutine = true;
        MSTP_OUT_flush_all_fids(ptp);
        ptp->calledFromFlushRoutine = false;
    }
    else
    {
        per_tree_port_t *cist = GET_CIST_PTP_FROM_PORT(prt);
        unsigned int FwdDelay = cist->designatedTimes.Forward_Delay;
        /* Initiate rapid ageing */
        MSTP_OUT_set_ageing_time(prt, FwdDelay);
        assign(prt->rapidAgeingWhile, FwdDelay);
        ptp->fdbFlush = false;
    }
}

/* 13.26.12 setRcvdMsgs */
static void setRcvdMsgs(port_t *prt)
{
    msti_configuration_message_t *msti_msg;
    int i;
    __be16 msg_MSTID;
    bool found;
    per_tree_port_t *ptp = GET_CIST_PTP_FROM_PORT(prt);
    ptp->rcvdMsg = true;

    /* 802.1Q-2005 says:
     *   "Make the received CST or CIST message available to the CIST Port
     *    Information state machines"
     * No need to do something special here, we already have rcvdBpduData.
     */

    if(prt->rcvdInternal)
    {
        list_for_each_entry_continue(ptp, &prt->trees, port_list)
        {
            found = false;
            /* Find if message for this MSTI is conveyed in the BPDU */
            for(i = 0, msti_msg = prt->rcvdBpduData.mstConfiguration;
                i < prt->rcvdBpduNumOfMstis;
                ++i, ++msti_msg)
            {
                msg_MSTID = msti_msg->mstiRRootID.s.priority
                            & __constant_cpu_to_be16(0x0FFF);
                if(msg_MSTID == ptp->MSTID)
                {
                    found = true;
                    break;
                }
            }
            if(found)
            {
                ptp->rcvdMsg = true;
                /* 802.1Q-2005 says:
                 *   "Make available each MSTI message and the common parts of
                 *    the CIST message priority (the CIST Root Identifier,
                 *    External Root Path Cost and Regional Root Identifier)
                 *    to the Port Information state machine for that MSTI"
                 * We set pointer to the MSTI configuration message for
                 * fast access, while do not anything special for common
                 * parts of the message, as the whole message is available
                 * in rcvdBpduData.
                 */
                ptp->rcvdMstiConfig = msti_msg;
            }
        }
    }
}

/* 13.26.13 setReRootTree */
static void setReRootTree(tree_t *tree)
{
    per_tree_port_t *ptp;

    FOREACH_PTP_IN_TREE(ptp, tree)
        ptp->reRoot = true;
}

/* 13.26.14 setSelectedTree */
static void setSelectedTree(tree_t *tree)
{
    per_tree_port_t *ptp;

    /*
     * 802.1Q-2005 says that I should check "reselect" var
     * and take no action if it is "true" for any of the ports.
     * But there is no need in this check as setSelectedTree is called
     * only from PRSSM_to_ROLE_SELECTION, which is atomic, and it is called
     * in this sequence (13.33):
     *   clearReselectTree(tree);
     *   updtRolesTree(tree);
     *   setSelectedTree(tree);
     * And we know that clearReselectTree resets "reselect" for all ports
     * and updtRolesTree() does not change value of "reselect".
     */
    FOREACH_PTP_IN_TREE(ptp, tree)
        ptp->selected = true;
}

/* 13.26.15 setSyncTree */
static void setSyncTree(tree_t *tree)
{
    per_tree_port_t *ptp;

    FOREACH_PTP_IN_TREE(ptp, tree)
        ptp->sync = true;
}

/* 13.26.16 setTcFlags */
static void setTcFlags(per_tree_port_t *ptp)
{
    __u8 cistFlags;
    port_t *prt;

    if(0 == ptp->MSTID)
    { /* CIST */
        prt = ptp->port;
        cistFlags = prt->rcvdBpduData.flags;
        if(cistFlags & (1 << offsetTcAck))
            prt->rcvdTcAck = true;
        if(cistFlags & (1 << offsetTc))
        {
            ptp->rcvdTc = true;
            if(!prt->rcvdInternal)
                list_for_each_entry_continue(ptp, &prt->trees, port_list)
                    ptp->proposed = true;
        }
        return;
    }
    /* MSTI */
    if(ptp->rcvdMstiConfig->flags & (1 << offsetTc))
        ptp->rcvdTc = true;
}

/* 13.26.17 setTcPropTree */
static void setTcPropTree(per_tree_port_t *ptp)
{
    per_tree_port_t *ptp_1;

    if(ptp->port->restrictedTcn)
        return;

    FOREACH_PTP_IN_TREE(ptp_1, ptp->tree)
    {
        if(ptp != ptp_1)
            ptp_1->tcProp = true;
    }
}

/* 13.26.18 syncMaster */
static void syncMaster(bridge_t *br)
{
    per_tree_port_t *ptp;
    tree_t *tree = GET_CIST_TREE(br);

    /* For each MSTI */
    list_for_each_entry_continue(tree, &br->trees, bridge_list)
    {
        FOREACH_PTP_IN_TREE(ptp, tree)
        {
            /* for each Port that has infoInternal set */
            if(ptp->port->infoInternal)
            {
                ptp->agree = false;
                ptp->agreed = false;
                ptp->synced = false;
                ptp->sync = true;
            }
        }
    }
}

/* 13.26.19 txConfig */
static void txConfig(port_t *prt)
{
    bpdu_t b;
    per_tree_port_t *cist = GET_CIST_PTP_FROM_PORT(prt);

    if(prt->deleted || (roleDisabled == cist->role) || prt->dontTxmtBpdu)
        return;

    b.protocolIdentifier = 0;
    b.protocolVersion = protoSTP;
    b.bpduType = bpduTypeConfig;
    /* Standard says "tcWhile ... for the Port". Which one tcWhile?
     * I guess that this means tcWhile for the CIST.
     * But that is only a guess and I could be wrong here ;)
     */
    b.flags = (0 != cist->tcWhile) ? (1 << offsetTc) : 0;
    if(prt->tcAck)
        b.flags |= (1 << offsetTcAck);
    assign(b.cistRootID, cist->designatedPriority.RootID);
    assign(b.cistExtRootPathCost, cist->designatedPriority.ExtRootPathCost);
    assign(b.cistRRootID, cist->designatedPriority.DesignatedBridgeID);
    assign(b.cistPortID, cist->designatedPriority.DesignatedPortID);
    b.MessageAge[0] = cist->designatedTimes.Message_Age;
    b.MessageAge[1] = 0;
    b.MaxAge[0] = cist->designatedTimes.Max_Age;
    b.MaxAge[1] = 0;
    b.HelloTime[0] = cist->portTimes.Hello_Time; /* ! use portTimes ! */
    b.HelloTime[1] = 0;
    b.ForwardDelay[0] = cist->designatedTimes.Forward_Delay;
    b.ForwardDelay[1] = 0;

    MSTP_OUT_tx_bpdu(prt, &b, CONFIG_BPDU_SIZE);
}

static inline __u8 message_role_from_port_role(per_tree_port_t *ptp)
{
    switch(ptp->role)
    {
        case roleRoot:
            return encodedRoleRoot;
        case roleDesignated:
            return encodedRoleDesignated;
        case roleAlternate:
        case roleBackup:
            return encodedRoleAlternateBackup;
        case roleMaster:
            return encodedRoleMaster;
        default:
            ERROR_PRTNAME(ptp->port->bridge, ptp->port,
                          "Attempt to send from port with Disabled role");
            return encodedRoleAlternateBackup;
    }
}

/* 802.1Q-2005: 13.26.20 txMstp
 * 802.1Q-2011: 13.27.27 txRstp
 */
static void txMstp(port_t *prt)
{
    bpdu_t b;
    bridge_t *br = prt->bridge;
    per_tree_port_t *cist = GET_CIST_PTP_FROM_PORT(prt);
    int msti_msgs_total_size;
    per_tree_port_t *ptp;
    msti_configuration_message_t *msti_msg;

    if(prt->deleted || (roleDisabled == cist->role) || prt->dontTxmtBpdu)
        return;

    b.protocolIdentifier = 0;
    b.bpduType = bpduTypeRST;
    /* Standard says "{tcWhile, agree, proposing} ... for the Port".
     * Which one {tcWhile, agree, proposing}?
     * I guess that this means {tcWhile, agree, proposing} for the CIST.
     * But that is only a guess and I could be wrong here ;)
     */
    b.flags = BPDU_FLAGS_ROLE_SET(message_role_from_port_role(cist));
    if(0 != cist->tcWhile)
        b.flags |= (1 << offsetTc);
    if(cist->proposing)
        b.flags |= (1 << offsetProposal);
    if(cist->learning)
        b.flags |= (1 << offsetLearnig);
    if(cist->forwarding)
        b.flags |= (1 << offsetForwarding);
    if(cist->agree)
        b.flags |= (1 << offsetAgreement);
    assign(b.cistRootID, cist->designatedPriority.RootID);
    assign(b.cistExtRootPathCost, cist->designatedPriority.ExtRootPathCost);
    assign(b.cistRRootID, cist->designatedPriority.RRootID);
    assign(b.cistPortID, cist->designatedPriority.DesignatedPortID);
    b.MessageAge[0] = cist->designatedTimes.Message_Age;
    b.MessageAge[1] = 0;
    b.MaxAge[0] = cist->designatedTimes.Max_Age;
    b.MaxAge[1] = 0;
    b.HelloTime[0] = cist->portTimes.Hello_Time; /* ! use portTimes ! */
    b.HelloTime[1] = 0;
    b.ForwardDelay[0] = cist->designatedTimes.Forward_Delay;
    b.ForwardDelay[1] = 0;

    b.version1_len = 0;

    if(br->ForceProtocolVersion < protoMSTP)
    {
        b.protocolVersion = protoRSTP;
        MSTP_OUT_tx_bpdu(prt, &b, RST_BPDU_SIZE);
        return;
    }

    b.protocolVersion = protoMSTP;

    /* MST specific fields */
    assign(b.mstConfigurationIdentifier, br->MstConfigId);
    assign(b.cistIntRootPathCost, cist->designatedPriority.IntRootPathCost);
    assign(b.cistBridgeID, cist->designatedPriority.DesignatedBridgeID);
    assign(b.cistRemainingHops, cist->designatedTimes.remainingHops);

    msti_msgs_total_size = 0;
    ptp = cist;
    msti_msg = b.mstConfiguration;
    /* 13.26.20.f) requires that msti configs should be inserted in
     * MSTID order. This is met by inserting trees in port's list of trees
     * in sorted (by MSTID) order (see MSTP_IN_create_msti) */
    list_for_each_entry_continue(ptp, &prt->trees, port_list)
    {
        msti_msg->flags =
            BPDU_FLAGS_ROLE_SET(message_role_from_port_role(ptp));
        if(0 != ptp->tcWhile)
            msti_msg->flags |= (1 << offsetTc);
        if(ptp->proposing)
            msti_msg->flags |= (1 << offsetProposal);
        if(ptp->learning)
            msti_msg->flags |= (1 << offsetLearnig);
        if(ptp->forwarding)
            msti_msg->flags |= (1 << offsetForwarding);
        if(ptp->agree)
            msti_msg->flags |= (1 << offsetAgreement);
        if(ptp->master)
            msti_msg->flags |= (1 << offsetMaster);
        assign(msti_msg->mstiRRootID, ptp->designatedPriority.RRootID);
        assign(msti_msg->mstiIntRootPathCost,
               ptp->designatedPriority.IntRootPathCost);
        msti_msg->bridgeIdentifierPriority =
            GET_PRIORITY_FROM_IDENTIFIER(ptp->designatedPriority.DesignatedBridgeID);
        msti_msg->portIdentifierPriority =
            GET_PRIORITY_FROM_IDENTIFIER(ptp->designatedPriority.DesignatedPortID);
        assign(msti_msg->remainingHops, ptp->designatedTimes.remainingHops);

        msti_msgs_total_size += sizeof(msti_configuration_message_t);
        ++msti_msg;
    }

    assign(b.version3_len, __cpu_to_be16(MST_BPDU_VER3LEN_WO_MSTI_MSGS
                                         + msti_msgs_total_size));
    MSTP_OUT_tx_bpdu(prt, &b, MST_BPDU_SIZE_WO_MSTI_MSGS
                              + msti_msgs_total_size);
}

/* 13.26.a) txTcn */
static void txTcn(port_t *prt)
{
    bpdu_t b;
    per_tree_port_t *cist = GET_CIST_PTP_FROM_PORT(prt);

    if(prt->deleted || (roleDisabled == cist->role) || prt->dontTxmtBpdu)
        return;

    b.protocolIdentifier = 0;
    b.protocolVersion = protoSTP;
    b.bpduType = bpduTypeTCN;

    MSTP_OUT_tx_bpdu(prt, &b, TCN_BPDU_SIZE);
}

/* 13.26.21 updtBPDUVersion */
static void updtBPDUVersion(port_t *prt)
{
    if(protoRSTP <= prt->rcvdBpduData.protocolVersion)
        prt->rcvdRSTP = true;
    else
        prt->rcvdSTP = true;
}

/* 13.26.22 updtRcvdInfoWhile */
static void updtRcvdInfoWhile(per_tree_port_t *ptp)
{
    port_t *prt = ptp->port;
    per_tree_port_t *cist = GET_CIST_PTP_FROM_PORT(prt);
    unsigned int Message_Age = cist->portTimes.Message_Age;
    unsigned int Max_Age = cist->portTimes.Max_Age;
    unsigned int Hello_Time = cist->portTimes.Hello_Time;

    /* NOTE: 802.1Q-2005(-2011) says that we should use
     *  "remainingHops ... from the CIST's portTimes parameter"
     *  As for me this is clear oversight in the standard,
     *  the remainingHops should be taken form the port's own portTimes,
     *  not from CIST's. After all, if we don't use port's own
     *  remainingHops here, they aren't used anywhere at all.
     *  Besides, there is a scenario which breaks if we use CIST's
     *  remainingHops here:
     *   1) Connect two switches (SW1,SW2) with two ports, thus forming a loop
     *   2) Configure them to be in the same region, with two trees:
     *      0 (CIST) and 1.
     *   3) at SW1# mstpctl settreeprio br0 1 4
     *      SW1 becomes regional root in tree 1
     *   4) at SW2# mstpctl settreeprio br0 1 14
     *   5) at SW1# mstpctl settreeprio br0 1 9
     *
     *  And now we have the classic "count-to-infinity" problem when the old
     *  info ("Regional Root is SW1 with priority 4") circulates in the loop,
     *  because it is better than current info ("Regional Root is SW1 with
     *  priority 9"). The only way to get rid of that old info is
     *  to age it out by the means of remainingHops counter.
     *  In this situation we certainly must use counter from tree 1,
     *  not CIST's.
     */
    if((!prt->rcvdInternal && ((Message_Age + 1) <= Max_Age))
       || (prt->rcvdInternal && (ptp->portTimes.remainingHops > 1))
      )
        ptp->rcvdInfoWhile = 3 * Hello_Time;
    else
        ptp->rcvdInfoWhile = 0;
}

static void updtbrAssuRcvdInfoWhile(port_t *prt)
{
    per_tree_port_t *cist = GET_CIST_PTP_FROM_PORT(prt);

    prt->brAssuRcvdInfoWhile = 3 * cist->portTimes.Hello_Time;
}

/* 13.26.24 updtRolesDisabledTree */
static void updtRolesDisabledTree(tree_t *tree)
{
    per_tree_port_t *ptp;

    FOREACH_PTP_IN_TREE(ptp, tree)
        ptp->selectedRole = roleDisabled;
}

/* Aux function, not in standard.
 * Sets reselect for all MSTIs in the case CIST state for the port changes
 */
static void reselectMSTIs(port_t *prt)
{
    per_tree_port_t *ptp = GET_CIST_PTP_FROM_PORT(prt);

    /* For each non-CIST ptp */
    list_for_each_entry_continue(ptp, &prt->trees, port_list)
        ptp->reselect = true;
}

/* 13.26.23 updtRolesTree */
static void updtRolesTree(tree_t *tree)
{
    per_tree_port_t *ptp, *root_ptp = NULL;
    port_priority_vector_t root_path_priority;
    bridge_identifier_t prevRRootID = tree->rootPriority.RRootID;
    __be32 prevExtRootPathCost = tree->rootPriority.ExtRootPathCost;
    bool cist = (0 == tree->MSTID);

    /* a), b) Select new root priority vector = {rootPriority, rootPortId} */
      /* Initial value = bridge priority vector = {BridgePriority, 0} */
    assign(tree->rootPriority, tree->BridgePriority);
    assign(tree->rootPortId, __constant_cpu_to_be16(0));
      /* Now check root path priority vectors of all ports in tree and see if
       * there is a better vector */
    FOREACH_PTP_IN_TREE(ptp, tree)
    {
        port_t *prt = ptp->port;
        /* 802.1Q says to calculate root priority vector only if port
         * is not Disabled, but check (infoIs == ioReceived) covers
         * the case (infoIs != ioDisabled).
         */
        if((ioReceived == ptp->infoIs) && !prt->restrictedRole
           && cmp(ptp->portPriority.DesignatedBridgeID, !=,
                  tree->BridgeIdentifier)
          )
        {
            root_path_priority = ptp->portPriority;
            if(prt->rcvdInternal)
            {
                assign(root_path_priority.IntRootPathCost,
                       __cpu_to_be32(__be32_to_cpu(root_path_priority.IntRootPathCost)
                                     + ptp->InternalPortPathCost)
                      );
            }
            else if(cist) /* Yes, this check might be superfluous,
                           * but I want to be on the safe side */
            {
                assign(root_path_priority.ExtRootPathCost,
                       __cpu_to_be32(__be32_to_cpu(root_path_priority.ExtRootPathCost)
                                     + prt->ExternalPortPathCost)
                      );
                assign(root_path_priority.RRootID, tree->BridgeIdentifier);
                assign(root_path_priority.IntRootPathCost,
                       __constant_cpu_to_be32(0));
            }
            if(betterorsamePriority(&root_path_priority, &tree->rootPriority,
                                    ptp->portId, tree->rootPortId, cist))
            {
                assign(tree->rootPriority, root_path_priority);
                assign(tree->rootPortId, ptp->portId);
                root_ptp = ptp;
            }
        }
    }

    /* 802.1q-2005 says, that at some point we need compare portTimes with
     * "... one for the Root Port ...". Bad IEEE! Why not mention explicit
     * var names??? (see 13.26.23.g) for instance)
     * These comparisons happen three times, twice in clause g)
     *   and once in clause i). Look for samePriorityAndTimers() calls.
     * So, now I should guess what will work for the "times for the Root Port".
     * Thanks to Rajani's experiments I know for sure that I should use
     *  designatedTimes here. Thank you, Rajani!
     * NOTE: Both Alex Rozin (author of rstplib) and Srinivas Aji (author
     *   of rstpd) also compare portTimes with designatedTimes.
     */

    /* c) Set new rootTimes */
    if(root_ptp)
    {
        assign(tree->rootTimes, root_ptp->portTimes);
        port_t *prt = root_ptp->port;
        if(prt->rcvdInternal)
        {
            if(tree->rootTimes.remainingHops)
                --(tree->rootTimes.remainingHops);
        }
        else
            ++(tree->rootTimes.Message_Age);
    }
    else
    {
        assign(tree->rootTimes, tree->BridgeTimes);
    }

    FOREACH_PTP_IN_TREE(ptp, tree)
    {
        port_t *prt = ptp->port;

        /* d) Set new designatedPriority */
        assign(ptp->designatedPriority, tree->rootPriority);
        assign(ptp->designatedPriority.DesignatedBridgeID,
               tree->BridgeIdentifier);
        assign(ptp->designatedPriority.DesignatedPortID, ptp->portId);
        /* I am not sure which condition to check here, as 802.1Q-2005 says:
         * "... If {Port} is attached to a LAN that has one or more STP Bridges
         *  attached (as determined by the Port Protocol Migration state
         * machine) ..." -- why not to mention explicit var name? Bad IEEE.
         * But I guess that sendSTP (i.e. !sendRSTP) var will do ;)
         */
        if(cist && !prt->sendRSTP)
            assign(ptp->designatedPriority.RRootID, tree->BridgeIdentifier);

        /* e) Set new designatedTimes */
        assign(ptp->designatedTimes, tree->rootTimes);
        /* Keep the configured Hello_Time for the port.
         * NOTE: this is in accordance with the spirit of 802.1D-2004.
         *    Also, this does not contradict 802.1Q-2005(-2011), as in these
         *    standards both designatedTimes and rootTimes structures
         *    don't have Hello_Time member.
         */
        assign(ptp->designatedTimes.Hello_Time, ptp->portTimes.Hello_Time);
    }

    /* syncMaster */
    if(cist && cmp(tree->rootPriority.RRootID, !=, prevRRootID)
       && ((0 != tree->rootPriority.ExtRootPathCost)
           || (0 != prevExtRootPathCost)
          )
      )
        syncMaster(tree->bridge);

    FOREACH_PTP_IN_TREE(ptp, tree)
    {
        port_t *prt = ptp->port;
        per_tree_port_t *cist_tree = GET_CIST_PTP_FROM_PORT(prt);

        /* f) Set Disabled role */
        if(ioDisabled == ptp->infoIs)
        {
            ptp->selectedRole = roleDisabled;
            continue;
        }

        if(!cist && (ioReceived == cist_tree->infoIs) && !prt->infoInternal)
        {
            /* g) Set role for the boundary port in MSTI */
            if(roleRoot == cist_tree->selectedRole)
            {
                ptp->selectedRole = roleMaster;
                if(!samePriorityAndTimers(&ptp->portPriority,
                                          &ptp->designatedPriority,
                                          &ptp->portTimes,
                                          &ptp->designatedTimes,
                                          /*cist*/ false))
                    ptp->updtInfo = true;
                continue;
            }
            /* Bad IEEE again! It says in 13.26.23 g) 2) that
             * MSTI state should follow CIST state only for the case of
             * Alternate port. This is obviously wrong!
             * In the descriptive clause 13.13 f) it says:
             *  "At a Boundary Port frames allocated to the CIST and
             *   all MSTIs are forwarded or not forwarded alike.
             *   This is because Port Role assignments are such that
             *   if the CIST Port Role is Root Port, the MSTI Port Role
             *   will be Master Port, and if the CIST Port Role is
             *   Designated Port, Alternate Port, Backup Port,
             *   or Disabled Port, each MSTI’s Port Role will be the same."
             * So, ignore wrong 13.26.23 g) 2) and do as stated in 13.13 f) !
             */
            /* if(roleAlternate == cist_tree->selectedRole) */
            {
                ptp->selectedRole = cist_tree->selectedRole;
                if(!samePriorityAndTimers(&ptp->portPriority,
                                          &ptp->designatedPriority,
                                          &ptp->portTimes,
                                          &ptp->designatedTimes,
                                          /*cist*/ false))
                    ptp->updtInfo = true;
                continue;
            }
        }
        else
     /* if(cist || (ioReceived != cist_tree->infoIs) || prt->infoInternal) */
        {
            /* h) Set role for the aged info */
            if(ioAged == ptp->infoIs)
            {
                ptp->selectedRole = roleDesignated;
                ptp->updtInfo = true;
                continue;
            }
            /* i) Set role for the mine info */
            if(ioMine == ptp->infoIs)
            {
                ptp->selectedRole = roleDesignated;
                if(!samePriorityAndTimers(&ptp->portPriority,
                                          &ptp->designatedPriority,
                                          &ptp->portTimes,
                                          &ptp->designatedTimes,
                                          cist))
                    ptp->updtInfo = true;
                continue;
            }
            if(ioReceived == ptp->infoIs)
            {
                /* j) Set Root role */
                if(root_ptp == ptp)
                {
                    ptp->selectedRole = roleRoot;
                    ptp->updtInfo = false;
                }
                else
                {
                    if(betterorsamePriority(&ptp->portPriority,
                                             &ptp->designatedPriority,
                                             0, 0, cist))
                    {
                        if(cmp(ptp->portPriority.DesignatedBridgeID, !=,
                               tree->BridgeIdentifier))
                        {
                            /* k) Set Alternate role */
                            ptp->selectedRole = roleAlternate;
                        }
                        else
                        {
                            /* l) Set Backup role */
                            ptp->selectedRole = roleBackup;
                        }
                        /* reset updtInfo for both k) and l) */
                        ptp->updtInfo = false;
                    }
                    else /* designatedPriority is better than portPriority */
                    {
                        /* m) Set Designated role */
                        ptp->selectedRole = roleDesignated;
                        ptp->updtInfo = true;
                    }
                }
                /* This is not in standard. But we really should set here
                 * reselect for all MSTIs so that updtRolesTree is called
                 * for each MSTI and due to above clause g) MSTI role is
                 * changed to Master or reflects CIST port role.
                 * Because in 802.1Q-2005 this will not happen when BPDU arrives
                 * at boundary port - the rcvdMsg is not set for the MSTIs and
                 * updtRolesTree is not called.
                 * Bad IEEE !!!
                 */
                if(cist && (ptp->selectedRole != ptp->role))
                    reselectMSTIs(prt);
                continue;
            }
        }
    }
}

/* 13.27  The Port Timers state machine */

static void PTSM_tick(port_t *prt)
{
    per_tree_port_t *ptp;

    if(prt->helloWhen)
        --(prt->helloWhen);
    if(prt->mdelayWhile)
        --(prt->mdelayWhile);
    if(prt->edgeDelayWhile)
        --(prt->edgeDelayWhile);
    if(prt->txCount)
        --(prt->txCount);
    if(prt->brAssuRcvdInfoWhile)
        --(prt->brAssuRcvdInfoWhile);

    FOREACH_PTP_IN_PORT(ptp, prt)
    {
        if(ptp->fdWhile)
            --(ptp->fdWhile);
        if(ptp->rrWhile)
            --(ptp->rrWhile);
        if(ptp->rbWhile)
            --(ptp->rbWhile);
        if(ptp->tcWhile)
        {
            if(0 == --(ptp->tcWhile))
                set_TopologyChange(ptp->tree, false, prt);
        }
        if(ptp->rcvdInfoWhile)
            --(ptp->rcvdInfoWhile);
    }
}

/* 13.28  Port Receive state machine */
#define PRSM_begin(prt) PRSM_to_DISCARD((prt), false)
static bool PRSM_to_DISCARD(port_t *prt, bool dry_run)
{
    if(dry_run)
    {
        return (prt->PRSM_state != PRSM_DISCARD)
               || prt->rcvdBpdu || prt->rcvdRSTP || prt->rcvdSTP
               || (prt->edgeDelayWhile != prt->bridge->Migrate_Time)
               || clearAllRcvdMsgs(prt, dry_run);
    }

    prt->PRSM_state = PRSM_DISCARD;

    prt->rcvdBpdu = false;
    prt->rcvdRSTP = false;
    prt->rcvdSTP = false;
    clearAllRcvdMsgs(prt, false /* actual run */);
    assign(prt->edgeDelayWhile, prt->bridge->Migrate_Time);

    /* No need to run, no one condition will be met
     * if(!begin)
     *     PRSM_run(prt, false); */
    return false;
}

static void PRSM_to_RECEIVE(port_t *prt)
{
    prt->PRSM_state = PRSM_RECEIVE;

    updtBPDUVersion(prt);
    prt->rcvdInternal = fromSameRegion(prt);
    setRcvdMsgs(prt);
    prt->operEdge = false;
    prt->rcvdBpdu = false;
    assign(prt->edgeDelayWhile, prt->bridge->Migrate_Time);

    /* No need to run, no one condition will be met
      PRSM_run(prt, false); */
}

static bool PRSM_run(port_t *prt, bool dry_run)
{
    per_tree_port_t *ptp;
    bool rcvdAnyMsg;

    if((prt->rcvdBpdu || (prt->edgeDelayWhile != prt->bridge->Migrate_Time))
       && !prt->portEnabled)
    {
        return PRSM_to_DISCARD(prt, dry_run);
    }

    switch(prt->PRSM_state)
    {
        case PRSM_DISCARD:
            if(prt->rcvdBpdu && prt->portEnabled)
            {
                if(dry_run) /* state change */
                    return true;
                PRSM_to_RECEIVE(prt);
            }
            return false;
        case PRSM_RECEIVE:
            rcvdAnyMsg = false;
            FOREACH_PTP_IN_PORT(ptp, prt)
            {
                if(ptp->rcvdMsg)
                {
                    rcvdAnyMsg = true;
                    break;
                }
            }
            if(prt->rcvdBpdu && prt->portEnabled && !rcvdAnyMsg)
            {
                if(dry_run) /* at least rcvdBpdu will change */
                    return true;
                PRSM_to_RECEIVE(prt);
            }
        default:
            return false;
    }
}

/* 13.29  Port Protocol Migration state machine */

static bool PPMSM_run(port_t *prt, bool dry_run);
#define PPMSM_begin(prt) PPMSM_to_CHECKING_RSTP(prt)

static void PPMSM_to_CHECKING_RSTP(port_t *prt/*, bool begin*/)
{
    prt->PPMSM_state = PPMSM_CHECKING_RSTP;

    bridge_t *br = prt->bridge;
    prt->mcheck = false;
    prt->sendRSTP = rstpVersion(br);
    assign(prt->mdelayWhile, br->Migrate_Time);

    /* No need to run, no one condition will be met
     * if(!begin)
     *     PPMSM_run(prt, false); */
}

static void PPMSM_to_SELECTING_STP(port_t *prt)
{
    prt->PPMSM_state = PPMSM_SELECTING_STP;

    prt->sendRSTP = false;
    assign(prt->mdelayWhile, prt->bridge->Migrate_Time);

    PPMSM_run(prt, false /* actual run */);
}

static void PPMSM_to_SENSING(port_t *prt)
{
    prt->PPMSM_state = PPMSM_SENSING;

    prt->rcvdRSTP = false;
    prt->rcvdSTP = false;

    PPMSM_run(prt, false /* actual run */);
}

static bool PPMSM_run(port_t *prt, bool dry_run)
{
    bridge_t *br = prt->bridge;

    switch(prt->PPMSM_state)
    {
        case PPMSM_CHECKING_RSTP:
            if((prt->mdelayWhile != br->Migrate_Time)
               && !prt->portEnabled)
            {
                if(dry_run) /* at least mdelayWhile will change */
                    return true;
                PPMSM_to_CHECKING_RSTP(prt);
                return false;
            }
            if(0 == prt->mdelayWhile)
            {
                if(dry_run) /* state change */
                    return true;
                PPMSM_to_SENSING(prt);
            }
            return false;
        case PPMSM_SELECTING_STP:
            if(0 == prt->mdelayWhile || !prt->portEnabled || prt->mcheck)
            {
                if(dry_run) /* state change */
                    return true;
                PPMSM_to_SENSING(prt);
            }
            return false;
        case PPMSM_SENSING:
            if(!prt->portEnabled || prt->mcheck
               || (rstpVersion(br) && !prt->sendRSTP && prt->rcvdRSTP))
            {
                if(dry_run) /* state change */
                    return true;
                PPMSM_to_CHECKING_RSTP(prt);
                return false;
            }
            if(prt->sendRSTP && prt->rcvdSTP)
            {
                if(dry_run) /* state change */
                    return true;
                PPMSM_to_SELECTING_STP(prt);
            }
            return false;
    }

    return false;
}

/* 13.30  Bridge Detection state machine */
static void BDSM_to_EDGE(port_t *prt/*, bool begin*/)
{
    prt->BDSM_state = BDSM_EDGE;

    prt->operEdge = true;

    /* No need to run, no one condition will be met
     * if(!begin)
     *     BDSM_run(prt, false); */
}

static void BDSM_to_NOT_EDGE(port_t *prt/*, bool begin*/)
{
    prt->BDSM_state = BDSM_NOT_EDGE;

    prt->operEdge = false;

    /* No need to run, no one condition will be met
     * if(!begin)
     *     BDSM_run(prt, false); */
}

static void BDSM_begin(port_t *prt/*, bool begin*/)
{
    if(prt->AdminEdgePort)
        BDSM_to_EDGE(prt/*, begin*/);
    else
        BDSM_to_NOT_EDGE(prt/*, begin*/);
}

static bool BDSM_run(port_t *prt, bool dry_run)
{
    per_tree_port_t *cist;

    switch(prt->BDSM_state)
    {
        case BDSM_EDGE:
            if(((!prt->portEnabled || !prt->AutoEdge) && !prt->AdminEdgePort)
               || !prt->operEdge
              )
            {
                if(dry_run) /* state change */
                    return true;
                BDSM_to_NOT_EDGE(prt);
            }
            return false;
        case BDSM_NOT_EDGE:
             cist = GET_CIST_PTP_FROM_PORT(prt);
            /* NOTE: 802.1Q-2005(-2011) is not clear, which of the per-tree
             *  "proposing" flags to use here, or one should combine
             *  them all for all trees?
             * So, I decide that it will be the "proposing" flag
             *  from CIST tree - it seems like a good bet.
             */
            if((!prt->portEnabled && prt->AdminEdgePort)
               || ((0 == prt->edgeDelayWhile) && prt->AutoEdge && prt->sendRSTP
                   && cist->proposing)
              )
            {
                if(dry_run) /* state change */
                    return true;
                BDSM_to_EDGE(prt);
            }
        default:
            return false;
    }
}

/* 13.31  Port Transmit state machine */

static bool PTSM_run(port_t *prt, bool dry_run);
#define PTSM_begin(prt) PTSM_to_TRANSMIT_INIT((prt), true, false)

static bool PTSM_to_TRANSMIT_INIT(port_t *prt, bool begin, bool dry_run)
{
    if(dry_run)
    {
        return (prt->PTSM_state != PTSM_TRANSMIT_INIT)
               || (!prt->newInfo) || (!prt->newInfoMsti)
               || (0 != prt->txCount);
    }

    prt->PTSM_state = PTSM_TRANSMIT_INIT;

    prt->newInfo = true;
    prt->newInfoMsti = true;
    assign(prt->txCount, 0u);

    if(!begin && prt->portEnabled) /* prevent infinite loop */
        PTSM_run(prt, false /* actual run */);
    return false;
}

static void PTSM_to_TRANSMIT_CONFIG(port_t *prt)
{
    prt->PTSM_state = PTSM_TRANSMIT_CONFIG;

    prt->newInfo = false;
    txConfig(prt);
    ++(prt->txCount);
    prt->tcAck = false;

    PTSM_run(prt, false /* actual run */);
}

static void PTSM_to_TRANSMIT_TCN(port_t *prt)
{
    prt->PTSM_state = PTSM_TRANSMIT_TCN;

    prt->newInfo = false;
    txTcn(prt);
    ++(prt->txCount);

    PTSM_run(prt, false /* actual run */);
}

static void PTSM_to_TRANSMIT_RSTP(port_t *prt)
{
    prt->PTSM_state = PTSM_TRANSMIT_RSTP;

    prt->newInfo = false;
    prt->newInfoMsti = false;
    txMstp(prt);
    ++(prt->txCount);
    prt->tcAck = false;

    PTSM_run(prt, false /* actual run */);
}

static void PTSM_to_TRANSMIT_PERIODIC(port_t *prt)
{
    prt->PTSM_state = PTSM_TRANSMIT_PERIODIC;

    per_tree_port_t *ptp = GET_CIST_PTP_FROM_PORT(prt);
    bool cistDesignatedOrTCpropagatingRootPort =
        (roleDesignated == ptp->role)
        || ((roleRoot == ptp->role) && (0 != ptp->tcWhile));
    bool mstiDesignatedOrTCpropagatingRootPort;

    mstiDesignatedOrTCpropagatingRootPort = false;
    list_for_each_entry_continue(ptp, &prt->trees, port_list)
    {
        if((roleDesignated == ptp->role)
           || ((roleRoot == ptp->role) && (0 != ptp->tcWhile))
          )
        {
            mstiDesignatedOrTCpropagatingRootPort = true;
            break;
        }
    }

    prt->newInfo = prt->newInfo || cistDesignatedOrTCpropagatingRootPort;
    prt->newInfoMsti = prt->newInfoMsti
                       || mstiDesignatedOrTCpropagatingRootPort;

    PTSM_run(prt, false /* actual run */);
}

static void PTSM_to_IDLE(port_t *prt)
{
    prt->PTSM_state = PTSM_IDLE;

    per_tree_port_t *cist = GET_CIST_PTP_FROM_PORT(prt);
    prt->helloWhen = cist->portTimes.Hello_Time;

    PTSM_run(prt, false /* actual run */);
}

static bool PTSM_run(port_t *prt, bool dry_run)
{
   /* bool allTransmitReady; */
    per_tree_port_t *ptp;
    port_role_t cistRole;
    bool mstiMasterPort;

    if(!prt->portEnabled)
    {
        return PTSM_to_TRANSMIT_INIT(prt, false, dry_run);
    }

    switch(prt->PTSM_state)
    {
        case PTSM_TRANSMIT_INIT:
           /* return; */
        case PTSM_TRANSMIT_CONFIG:
           /* return; */
        case PTSM_TRANSMIT_TCN:
           /* return; */
        case PTSM_TRANSMIT_RSTP:
           /* return; */
        case PTSM_TRANSMIT_PERIODIC:
            if(dry_run) /* state change */
                return true;
            PTSM_to_IDLE(prt); /* UnConditional Transition */
            return false;
        case PTSM_IDLE:
            /* allTransmitReady = true; */
            ptp = GET_CIST_PTP_FROM_PORT(prt);
            if(!ptp->selected || ptp->updtInfo)
            {
                /* allTransmitReady = false; */
                return false;
            }
            cistRole = ptp->role;
            mstiMasterPort = false;
            list_for_each_entry_continue(ptp, &prt->trees, port_list)
            {
                if(!ptp->selected || ptp->updtInfo)
                {
                    /* allTransmitReady = false; */
                    return false;
                }
                if(roleMaster == ptp->role)
                    mstiMasterPort = true;
            }
            if(0 == prt->helloWhen)
            {
                if(dry_run) /* state change */
                    return true;
                PTSM_to_TRANSMIT_PERIODIC(prt);
                return false;
            }
            if(!(prt->txCount < prt->bridge->Transmit_Hold_Count))
                return false;

            if(prt->bpduFilterPort)
                return false;

            if(prt->sendRSTP)
            { /* implement MSTP */
                if(prt->newInfo || (prt->newInfoMsti && !mstiMasterPort)
                   || assurancePort(prt)
                  )
                {
                    if(dry_run) /* state change */
                        return true;
                    PTSM_to_TRANSMIT_RSTP(prt);
                    return false;
                }
            }
            else
            { /* fallback to STP */
                if(prt->newInfo && (roleDesignated == cistRole))
                {
                    if(dry_run) /* state change */
                        return true;
                    PTSM_to_TRANSMIT_CONFIG(prt);
                    return false;
                }
                if(prt->newInfo && (roleRoot == cistRole))
                {
                    if(dry_run) /* state change */
                        return true;
                    PTSM_to_TRANSMIT_TCN(prt);
                    return false;
                }
            }
            return false;
    }

    return false;
}

/* 13.32  Port Information state machine */

#ifdef PISM_ENABLE_LOG
#define PISM_LOG(_fmt, _args...) SMLOG_MSTINAME(ptp, _fmt, ##_args)
#else
#define PISM_LOG(_fmt, _args...) {}
#endif /* PISM_ENABLE_LOG */

static bool PISM_run(per_tree_port_t *ptp, bool dry_run);
#define PISM_begin(ptp) PISM_to_DISABLED((ptp), true)

static void PISM_to_DISABLED(per_tree_port_t *ptp, bool begin)
{
    PISM_LOG("");
    ptp->PISM_state = PISM_DISABLED;

    ptp->rcvdMsg = false;
    ptp->proposing = false;
    ptp->proposed = false;
    ptp->agree = false;
    ptp->agreed = false;
    assign(ptp->rcvdInfoWhile, 0u);
    ptp->infoIs = ioDisabled;
    ptp->reselect = true;
    ptp->selected = false;

    if(!begin)
        PISM_run(ptp, false /* actual run */);
}

static void PISM_to_AGED(per_tree_port_t *ptp)
{
    PISM_LOG("");
    ptp->PISM_state = PISM_AGED;

    ptp->infoIs = ioAged;
    ptp->reselect = true;
    ptp->selected = false;

    PISM_run(ptp, false /* actual run */);
}

static void PISM_to_UPDATE(per_tree_port_t *ptp)
{
    PISM_LOG("");
    ptp->PISM_state = PISM_UPDATE;

    ptp->proposing = false;
    ptp->proposed = false;
    ptp->agreed = ptp->agreed && betterorsameInfo(ptp, ioMine);
    ptp->synced = ptp->synced && ptp->agreed;
    assign(ptp->portPriority, ptp->designatedPriority);
    assign(ptp->portTimes, ptp->designatedTimes);
    ptp->updtInfo = false;
    ptp->infoIs = ioMine;
    /* newInfoXst = TRUE; */
    port_t *prt = ptp->port;
    if(0 == ptp->MSTID)
        prt->newInfo = true;
    else
        prt->newInfoMsti = true;

    PISM_run(ptp, false /* actual run */);
}

static void PISM_to_SUPERIOR_DESIGNATED(per_tree_port_t *ptp)
{
    PISM_LOG("");
    ptp->PISM_state = PISM_SUPERIOR_DESIGNATED;

    port_t *prt = ptp->port;

    prt->infoInternal = prt->rcvdInternal;
    ptp->agreed = false;
    ptp->proposing = false;
    recordProposal(ptp);
    setTcFlags(ptp);
    ptp->agree = ptp->agree && betterorsameInfo(ptp, ioReceived);
    recordAgreement(ptp);
    ptp->synced = ptp->synced && ptp->agreed;
    recordPriority(ptp);
    recordTimes(ptp);
    updtRcvdInfoWhile(ptp);
    ptp->infoIs = ioReceived;
    ptp->reselect = true;
    ptp->selected = false;
    ptp->rcvdMsg = false;

    PISM_run(ptp, false /* actual run */);
}

static void PISM_to_REPEATED_DESIGNATED(per_tree_port_t *ptp)
{
    PISM_LOG("");
    ptp->PISM_state = PISM_REPEATED_DESIGNATED;

    port_t *prt = ptp->port;

    prt->infoInternal = prt->rcvdInternal;
    recordProposal(ptp);
    setTcFlags(ptp);
    recordAgreement(ptp);
    updtRcvdInfoWhile(ptp);
    ptp->rcvdMsg = false;

    PISM_run(ptp, false /* actual run */);
}

static void PISM_to_INFERIOR_DESIGNATED(per_tree_port_t *ptp)
{
    PISM_LOG("");
    ptp->PISM_state = PISM_INFERIOR_DESIGNATED;

    recordDispute(ptp);
    ptp->rcvdMsg = false;

    PISM_run(ptp, false /* actual run */);
}

static void PISM_to_NOT_DESIGNATED(per_tree_port_t *ptp)
{
    PISM_LOG("");
    ptp->PISM_state = PISM_NOT_DESIGNATED;

    recordAgreement(ptp);
    setTcFlags(ptp);
    ptp->rcvdMsg = false;

    PISM_run(ptp, false /* actual run */);
}

static void PISM_to_OTHER(per_tree_port_t *ptp)
{
    PISM_LOG("");
    ptp->PISM_state = PISM_OTHER;

    ptp->rcvdMsg = false;

    PISM_run(ptp, false /* actual run */);
}

static void PISM_to_CURRENT(per_tree_port_t *ptp)
{
    PISM_LOG("");
    ptp->PISM_state = PISM_CURRENT;

    PISM_run(ptp, false /* actual run */);
}

static void PISM_to_RECEIVE(per_tree_port_t *ptp)
{
    PISM_LOG("");
    ptp->PISM_state = PISM_RECEIVE;

    ptp->rcvdInfo = rcvInfo(ptp);
    recordMastered(ptp);

    PISM_run(ptp, false /* actual run */);
}

static bool PISM_run(per_tree_port_t *ptp, bool dry_run)
{
    bool rcvdXstMsg, updtXstInfo;
    port_t *prt = ptp->port;

    if((!prt->portEnabled) && (ioDisabled != ptp->infoIs))
    {
        if(dry_run) /* at least infoIs will change */
            return true;
        PISM_to_DISABLED(ptp, false);
        return false;
    }

    switch(ptp->PISM_state)
    {
        case PISM_DISABLED:
            if(prt->portEnabled)
            {
                if(dry_run) /* state change */
                    return true;
                PISM_to_AGED(ptp);
                return false;
            }
            if(ptp->rcvdMsg)
            {
                if(dry_run) /* at least rcvdMsg will change */
                    return true;
                PISM_to_DISABLED(ptp, false);
            }
            return false;
        case PISM_AGED:
            if(ptp->selected && ptp->updtInfo)
            {
                if(dry_run) /* state change */
                    return true;
                PISM_to_UPDATE(ptp);
            }
            return false;
        case PISM_UPDATE:
            /* return; */
        case PISM_SUPERIOR_DESIGNATED:
            /* return; */
        case PISM_REPEATED_DESIGNATED:
            /* return; */
        case PISM_INFERIOR_DESIGNATED:
            /* return; */
        case PISM_NOT_DESIGNATED:
            /* return; */
        case PISM_OTHER:
            if(dry_run) /* state change */
                return true;
            PISM_to_CURRENT(ptp);
            return false;
        case PISM_CURRENT:
            /*
             * Although 802.1Q-2005 does not define rcvdXstMsg and updtXstInfo
             *  from 802.1s we can conclude that they are:
             *  - rcvdXstMsg = rcvdCistMsg, if tree is CIST
             *                 rcvdMstiMsg, if tree is MSTI.
             *  - updtXstInfo = updtCistInfo, if tree is CIST
             *                  updtMstiInfo, if tree is MSTI.
             */
            if(0 == ptp->MSTID)
            { /* CIST */
                rcvdXstMsg = ptp->rcvdMsg; /* 13.25.12 */
                updtXstInfo = ptp->updtInfo; /* 13.25.16 */
            }
            else
            { /* MSTI */
                per_tree_port_t *cist = GET_CIST_PTP_FROM_PORT(prt);
                rcvdXstMsg = !cist->rcvdMsg && ptp->rcvdMsg; /* 13.25.13 */
                updtXstInfo = ptp->updtInfo || cist->updtInfo; /* 13.25.17 */
            }
            if(rcvdXstMsg && !updtXstInfo)
            {
                if(dry_run) /* state change */
                    return true;
                PISM_to_RECEIVE(ptp);
                return false;
            }
            if((ioReceived == ptp->infoIs) && (0 == ptp->rcvdInfoWhile)
               && !ptp->updtInfo && !rcvdXstMsg)
            {
                if(dry_run) /* state change */
                    return true;
                PISM_to_AGED(ptp);
                return false;
            }
            if(ptp->selected && ptp->updtInfo)
            {
                if(dry_run) /* state change */
                    return true;
                PISM_to_UPDATE(ptp);
            }
            return false;
        case PISM_RECEIVE:
            switch(ptp->rcvdInfo)
            {
                case SuperiorDesignatedInfo:
                    if(dry_run) /* state change */
                        return true;
                    PISM_to_SUPERIOR_DESIGNATED(ptp);
                    return false;
                case RepeatedDesignatedInfo:
                    if(dry_run) /* state change */
                        return true;
                    PISM_to_REPEATED_DESIGNATED(ptp);
                    return false;
                case InferiorDesignatedInfo:
                    if(dry_run) /* state change */
                        return true;
                    PISM_to_INFERIOR_DESIGNATED(ptp);
                    return false;
                case InferiorRootAlternateInfo:
                    if(dry_run) /* state change */
                        return true;
                    PISM_to_NOT_DESIGNATED(ptp);
                    return false;
                case OtherInfo:
                    if(dry_run) /* state change */
                        return true;
                    PISM_to_OTHER(ptp);
                    return false;
            }
            return false;
    }

    return false;
}

/* 13.33  Port Role Selection state machine */

static bool PRSSM_run(tree_t *tree, bool dry_run);
#define PRSSM_begin(tree) PRSSM_to_INIT_TREE(tree)

static void PRSSM_to_INIT_TREE(tree_t *tree/*, bool begin*/)
{
    tree->PRSSM_state = PRSSM_INIT_TREE;

    updtRolesDisabledTree(tree);

    /* No need to check, as we assume begin = true here
     * because transition to this state can be initiated only by BEGIN var.
     * In other words, this function is called via xxx_begin macro only.
     * if(!begin)
     *     PRSSM_run(prt, false); */
}

static void PRSSM_to_ROLE_SELECTION(tree_t *tree)
{
    tree->PRSSM_state = PRSSM_ROLE_SELECTION;

    clearReselectTree(tree);
    updtRolesTree(tree);
    setSelectedTree(tree);

    /* No need to run, no one condition will be met
      PRSSM_run(tree, false); */
}

static bool PRSSM_run(tree_t *tree, bool dry_run)
{
    per_tree_port_t *ptp;

    switch(tree->PRSSM_state)
    {
        case PRSSM_INIT_TREE:
            if(dry_run) /* state change */
                return true;
            PRSSM_to_ROLE_SELECTION(tree);
            return false;
        case PRSSM_ROLE_SELECTION:
            FOREACH_PTP_IN_TREE(ptp, tree)
                if(ptp->reselect)
                {
                    if(dry_run) /* at least reselect will change */
                        return true;
                    PRSSM_to_ROLE_SELECTION(tree);
                    return false;
                }
            return false;
    }

    return false;
}

/* 13.34  Port Role Transitions state machine */

#ifdef PRTSM_ENABLE_LOG
#define PRTSM_LOG(_fmt, _args...) SMLOG_MSTINAME(ptp, _fmt, ##_args)
#else
#define PRTSM_LOG(_fmt, _args...) {}
#endif /* PRTSM_ENABLE_LOG */

static bool PRTSM_runr(per_tree_port_t *ptp, bool recursive_call, bool dry_run);
#define PRTSM_run(ptp, dry_run) PRTSM_runr((ptp), false, (dry_run))
#define PRTSM_begin(ptp) PRTSM_to_INIT_PORT(ptp)

 /* Disabled Port role transitions */

static void PRTSM_to_INIT_PORT(per_tree_port_t *ptp/*, bool begin*/)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_INIT_PORT;

    unsigned int MaxAge, FwdDelay;
    per_tree_port_t *cist = GET_CIST_PTP_FROM_PORT(ptp->port);

    ptp->role = roleDisabled;
    ptp->learn = false;
    ptp->forward = false;
    ptp->synced = false;
    ptp->sync = true;
    ptp->reRoot = true;
    /* 13.25.6 */
    FwdDelay = cist->designatedTimes.Forward_Delay;
    assign(ptp->rrWhile, FwdDelay);
    /* 13.25.8 */
    MaxAge = cist->designatedTimes.Max_Age;
    assign(ptp->fdWhile, MaxAge);
    assign(ptp->rbWhile, 0u);

    /* No need to check, as we assume begin = true here
     * because transition to this state can be initiated only by BEGIN var.
     * In other words, this function is called via xxx_begin macro only.
     * if(!begin)
     *     PRTSM_runr(ptp, false, false); */
}

static void PRTSM_to_DISABLE_PORT(per_tree_port_t *ptp)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_DISABLE_PORT;

    /* Although 802.1Q-2005 says here to do role = selectedRole
     * I have difficulties with it in the next scenario:
     *  1) port was designated (role == selectedRole == roleDesignated)
     *  2) some config event occurs, e.g. MAC address changes and
     *     br_state_machines_begin is called
     *  3) role == selectedRole == roleDisabled, PRTSM_state = PRTSM_INIT_PORT
     * Because port was not actually down, on the next run
     *  Port Role Selection state machine sets selectedRole = roleDesignated
     *  and updtInfo = true:
     *  4) we have unconditional transition to DISABLE_PORT, and because
     *     updtInfo = true we can not follow transition to DESIGNATED_PORT
     *  5) if we follow standard, role = selectedRole = roleDesignated and
     *     on the next run we have transition to the DISABLED_PORT
     * And there we stuck. role == selectedRole, so we can not transit to
     *  DESIGNATED_PORT (it requires role != selectedRole ).
     *
     * Solution: do not follow the standard, and do role = roleDisabled
     *  instead of role = selectedRole.
     */
    ptp->role = roleDisabled;
    ptp->learn = false;
    ptp->forward = false;

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_DISABLED_PORT(per_tree_port_t *ptp, unsigned int MaxAge)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_DISABLED_PORT;

    assign(ptp->fdWhile, MaxAge);
    ptp->synced = true;
    assign(ptp->rrWhile, 0u);
    ptp->sync = false;
    ptp->reRoot = false;

    PRTSM_runr(ptp, true, false /* actual run */);
}

 /* MasterPort role transitions */

static void PRTSM_to_MASTER_PROPOSED(per_tree_port_t *ptp)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_MASTER_PROPOSED;

    setSyncTree(ptp->tree);
    ptp->proposed = false;

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_MASTER_AGREED(per_tree_port_t *ptp)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_MASTER_AGREED;

    ptp->proposed = false;
    ptp->sync = false;
    ptp->agree = true;

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_MASTER_SYNCED(per_tree_port_t *ptp)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_MASTER_SYNCED;

    assign(ptp->rrWhile, 0u);
    ptp->synced = true;
    ptp->sync = false;

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_MASTER_RETIRED(per_tree_port_t *ptp)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_MASTER_RETIRED;

    ptp->reRoot = false;

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_MASTER_FORWARD(per_tree_port_t *ptp)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_MASTER_FORWARD;

    ptp->forward = true;
    assign(ptp->fdWhile, 0u);
    ptp->agreed = ptp->port->sendRSTP;

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_MASTER_LEARN(per_tree_port_t *ptp, unsigned int forwardDelay)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_MASTER_LEARN;

    ptp->learn = true;
    assign(ptp->fdWhile, forwardDelay);

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_MASTER_DISCARD(per_tree_port_t *ptp, unsigned int forwardDelay)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_MASTER_DISCARD;

    ptp->learn = false;
    ptp->forward = false;
    ptp->disputed = false;
    assign(ptp->fdWhile, forwardDelay);

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_MASTER_PORT(per_tree_port_t *ptp)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_MASTER_PORT;

    ptp->role = roleMaster;

    PRTSM_runr(ptp, true, false /* actual run */);
}

 /* RootPort role transitions */

static void PRTSM_to_ROOT_PROPOSED(per_tree_port_t *ptp)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_ROOT_PROPOSED;

    setSyncTree(ptp->tree);
    ptp->proposed = false;

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_ROOT_AGREED(per_tree_port_t *ptp)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_ROOT_AGREED;

    ptp->proposed = false;
    ptp->sync = false;
    ptp->agree = true;
    /* newInfoXst = TRUE; */
    port_t *prt = ptp->port;
    if(0 == ptp->MSTID)
        prt->newInfo = true;
    else
        prt->newInfoMsti = true;

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_ROOT_SYNCED(per_tree_port_t *ptp)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_ROOT_SYNCED;

    ptp->synced = true;
    ptp->sync = false;

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_REROOT(per_tree_port_t *ptp)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_REROOT;

    setReRootTree(ptp->tree);

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_ROOT_FORWARD(per_tree_port_t *ptp)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_ROOT_FORWARD;

    assign(ptp->fdWhile, 0u);
    ptp->forward = true;

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_ROOT_LEARN(per_tree_port_t *ptp, unsigned int forwardDelay)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_ROOT_LEARN;

    assign(ptp->fdWhile, forwardDelay);
    ptp->learn = true;

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_REROOTED(per_tree_port_t *ptp)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_REROOTED;

    ptp->reRoot = false;

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_ROOT_PORT(per_tree_port_t *ptp, unsigned int FwdDelay)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_ROOT_PORT;

    ptp->role = roleRoot;
    assign(ptp->rrWhile, FwdDelay);

    PRTSM_runr(ptp, true, false /* actual run */);
}

 /* DesignatedPort role transitions */

static void PRTSM_to_DESIGNATED_PROPOSE(per_tree_port_t *ptp)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_DESIGNATED_PROPOSE;

    port_t *prt = ptp->port;

    ptp->proposing = true;
    /* newInfoXst = TRUE; */
    if(0 == ptp->MSTID)
    { /* CIST */
        /* 13.25.8. This tree is CIST. */
        unsigned int MaxAge = ptp->designatedTimes.Max_Age;
        /* 13.25.c) -> 17.20.4 of 802.1D : EdgeDelay */
        unsigned int EdgeDelay = prt->operPointToPointMAC ?
                                   prt->bridge->Migrate_Time
                                 : MaxAge;
        assign(prt->edgeDelayWhile, EdgeDelay);
        prt->newInfo = true;
    }
    else
        prt->newInfoMsti = true;

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_DESIGNATED_AGREED(per_tree_port_t *ptp)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_DESIGNATED_AGREED;

    ptp->proposed = false;
    ptp->sync = false;
    ptp->agree = true;
    /* newInfoXst = TRUE; */
    port_t *prt = ptp->port;
    if(0 == ptp->MSTID)
        prt->newInfo = true;
    else
        prt->newInfoMsti = true;

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_DESIGNATED_SYNCED(per_tree_port_t *ptp)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_DESIGNATED_SYNCED;

    assign(ptp->rrWhile, 0u);
    ptp->synced = true;
    ptp->sync = false;

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_DESIGNATED_RETIRED(per_tree_port_t *ptp)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_DESIGNATED_RETIRED;

    ptp->reRoot = false;

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_DESIGNATED_FORWARD(per_tree_port_t *ptp)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_DESIGNATED_FORWARD;

    ptp->forward = true;
    assign(ptp->fdWhile, 0u);
    ptp->agreed = ptp->port->sendRSTP;

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_DESIGNATED_LEARN(per_tree_port_t *ptp, unsigned int forwardDelay)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_DESIGNATED_LEARN;

    ptp->learn = true;
    assign(ptp->fdWhile, forwardDelay);

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_DESIGNATED_DISCARD(per_tree_port_t *ptp, unsigned int forwardDelay)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_DESIGNATED_DISCARD;

    ptp->learn = false;
    ptp->forward = false;
    ptp->disputed = false;
    assign(ptp->fdWhile, forwardDelay);

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_DESIGNATED_PORT(per_tree_port_t *ptp)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_DESIGNATED_PORT;

    ptp->role = roleDesignated;

    PRTSM_runr(ptp, true, false /* actual run */);
}

 /* AlternatePort and BackupPort role transitions */

static void PRTSM_to_BLOCK_PORT(per_tree_port_t *ptp)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_BLOCK_PORT;

    ptp->role = ptp->selectedRole;
    ptp->learn = false;
    ptp->forward = false;

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_BACKUP_PORT(per_tree_port_t *ptp, unsigned int HelloTime)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_BACKUP_PORT;

    assign(ptp->rbWhile, 2 * HelloTime);

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_ALTERNATE_PROPOSED(per_tree_port_t *ptp)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_ALTERNATE_PROPOSED;

    setSyncTree(ptp->tree);
    ptp->proposed = false;

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_ALTERNATE_AGREED(per_tree_port_t *ptp)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_ALTERNATE_AGREED;

    ptp->proposed = false;
    ptp->agree = true;
    /* newInfoXst = TRUE; */
    port_t *prt = ptp->port;
    if(0 == ptp->MSTID)
        prt->newInfo = true;
    else
        prt->newInfoMsti = true;

    PRTSM_runr(ptp, true, false /* actual run */);
}

static void PRTSM_to_ALTERNATE_PORT(per_tree_port_t *ptp, unsigned int forwardDelay)
{
    PRTSM_LOG("");
    ptp->PRTSM_state = PRTSM_ALTERNATE_PORT;

    assign(ptp->fdWhile, forwardDelay);
    ptp->synced = true;
    assign(ptp->rrWhile, 0u);
    ptp->sync = false;
    ptp->reRoot = false;

    PRTSM_runr(ptp, true, false /* actual run */);
}

static bool PRTSM_runr(per_tree_port_t *ptp, bool recursive_call, bool dry_run)
{
    /* Following vars do not need recalculating on recursive calls */
    static unsigned int MaxAge, FwdDelay, forwardDelay, HelloTime;
    static port_t *prt;
    static tree_t *tree;
    static per_tree_port_t *cist;
    /* Following vars are recalculated on each state transition */
    bool allSynced, reRooted;
    /* Following vars are auxiliary and don't depend on recursive_call */
    per_tree_port_t *ptp_1;

    if(!recursive_call)
    { /* calculate these intermediate vars only first time in chain of
       * recursive calls */
        prt = ptp->port;
        tree = ptp->tree;

        cist = GET_CIST_PTP_FROM_PORT(prt);

        /* 13.25.6 */
        FwdDelay = cist->designatedTimes.Forward_Delay;

        /* 13.25.7 */
        HelloTime = cist->portTimes.Hello_Time;

        /* 13.25.d) -> 17.20.5 of 802.1D */
        forwardDelay = prt->sendRSTP ? HelloTime : FwdDelay;

        /* 13.25.8 */
        MaxAge = cist->designatedTimes.Max_Age;
    }

    PRTSM_LOG("role = %d, selectedRole = %d, selected = %d, updtInfo = %d",
              ptp->role, ptp->selectedRole, ptp->selected, ptp->updtInfo);
    if((ptp->role != ptp->selectedRole) && ptp->selected && !ptp->updtInfo)
    {
        switch(ptp->selectedRole)
        {
            case roleDisabled:
                if(dry_run) /* at least role will change */
                    return true;
                PRTSM_to_DISABLE_PORT(ptp);
                return false;
            case roleMaster:
                if(dry_run) /* at least role will change */
                    return true;
                PRTSM_to_MASTER_PORT(ptp);
                return false;
            case roleRoot:
                if(dry_run) /* at least role will change */
                    return true;
                PRTSM_to_ROOT_PORT(ptp, FwdDelay);
                return false;
            case roleDesignated:
                if(dry_run) /* at least role will change */
                    return true;
                PRTSM_to_DESIGNATED_PORT(ptp);
                return false;
            case roleAlternate:
            case roleBackup:
                if(dry_run) /* at least role will change */
                    return true;
                PRTSM_to_BLOCK_PORT(ptp);
                return false;
        }
    }

    /* 13.25.1 */
    allSynced = true;
    FOREACH_PTP_IN_TREE(ptp_1, tree)
    {
        /* a) */
        if(!ptp_1->selected
           || (ptp_1->role != ptp_1->selectedRole)
           || ptp_1->updtInfo
          )
        {
            allSynced = false;
            break;
        }

        /* b) */
        switch(ptp->role)
        {
            case roleRoot:
            case roleAlternate:
                if((roleRoot != ptp_1->role) && !ptp_1->synced)
                    allSynced = false;
                break;
            case roleDesignated:
            case roleMaster:
                if((ptp != ptp_1) && !ptp_1->synced)
                    allSynced = false;
                break;
            default:
                allSynced = false;
        }
        if(!allSynced)
            break;
    }

    switch(ptp->PRTSM_state)
    {
     /* Disabled Port role transitions */
        case PRTSM_INIT_PORT:
            if(dry_run) /* state change */
                return true;
            PRTSM_to_DISABLE_PORT(ptp);
            return false;
        case PRTSM_DISABLE_PORT:
            if(ptp->selected && !ptp->updtInfo
               && !ptp->learning && !ptp->forwarding
              )
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_DISABLED_PORT(ptp, MaxAge);
            }
            return false;
        case PRTSM_DISABLED_PORT:
            if(ptp->selected && !ptp->updtInfo
               && (ptp->sync || ptp->reRoot || !ptp->synced
                   || (ptp->fdWhile != MaxAge))
              )
            {
                if(dry_run) /* one of (sync,reRoot,synced,fdWhile) will change */
                    return true;
                PRTSM_to_DISABLED_PORT(ptp, MaxAge);
            }
            return false;
     /* MasterPort role transitions */
        case PRTSM_MASTER_PROPOSED:
            /* return; */
        case PRTSM_MASTER_AGREED:
            /* return; */
        case PRTSM_MASTER_SYNCED:
            /* return; */
        case PRTSM_MASTER_RETIRED:
            /* return; */
        case PRTSM_MASTER_FORWARD:
            /* return; */
        case PRTSM_MASTER_LEARN:
            /* return; */
        case PRTSM_MASTER_DISCARD:
            if(dry_run) /* state change */
                return true;
            PRTSM_to_MASTER_PORT(ptp);
            return false;
        case PRTSM_MASTER_PORT:
            if(!(ptp->selected && !ptp->updtInfo))
                return false;
            if(ptp->reRoot && (0 == ptp->rrWhile))
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_MASTER_RETIRED(ptp);
                return false;
            }
            if((!ptp->learning && !ptp->forwarding && !ptp->synced)
               || (ptp->agreed && !ptp->synced)
               || (prt->operEdge && !ptp->synced)
               || (ptp->sync && ptp->synced)
              )
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_MASTER_SYNCED(ptp);
                return false;
            }
            if((allSynced && !ptp->agree)
               || (ptp->proposed && ptp->agree)
              )
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_MASTER_AGREED(ptp);
                return false;
            }
            if(ptp->proposed && !ptp->agree)
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_MASTER_PROPOSED(ptp);
                return false;
            }
            if(((0 == ptp->fdWhile) || allSynced)
               && ptp->learn && !ptp->forward
              )
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_MASTER_FORWARD(ptp);
                return false;
            }
            if(((0 == ptp->fdWhile) || allSynced)
               && !ptp->learn
              )
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_MASTER_LEARN(ptp, forwardDelay);
                return false;
            }
            if(((ptp->sync && !ptp->synced)
                || (ptp->reRoot && (0 != ptp->rrWhile))
                || ptp->disputed
               )
               && !prt->operEdge && (ptp->learn || ptp->forward)
              )
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_MASTER_DISCARD(ptp, forwardDelay);
                return false;
            }
            return false;
     /* RootPort role transitions */
        case PRTSM_ROOT_PROPOSED:
            /* return; */
        case PRTSM_ROOT_AGREED:
            /* return; */
        case PRTSM_ROOT_SYNCED:
            /* return; */
        case PRTSM_REROOT:
            /* return; */
        case PRTSM_ROOT_FORWARD:
            /* return; */
        case PRTSM_ROOT_LEARN:
            /* return; */
        case PRTSM_REROOTED:
            if(dry_run) /* state change */
                return true;
            PRTSM_to_ROOT_PORT(ptp, FwdDelay);
            return false;
        case PRTSM_ROOT_PORT:
            if(!(ptp->selected && !ptp->updtInfo))
                return false;
            if(!ptp->forward && !ptp->reRoot)
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_REROOT(ptp);
                return false;
            }
            if((ptp->agreed && !ptp->synced) || (ptp->sync && ptp->synced))
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_ROOT_SYNCED(ptp);
                return false;
            }
            if((allSynced && !ptp->agree) || (ptp->proposed && ptp->agree))
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_ROOT_AGREED(ptp);
                return false;
            }
            if(ptp->proposed && !ptp->agree)
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_ROOT_PROPOSED(ptp);
                return false;
            }
            /* 17.20.10 of 802.1D : reRooted */
            reRooted = true;
            FOREACH_PTP_IN_TREE(ptp_1, tree)
            {
                if((ptp != ptp_1) && (0 != ptp_1->rrWhile))
                {
                    reRooted = false;
                    break;
                }
            }
            if((0 == ptp->fdWhile)
               || (reRooted && (0 == ptp->rbWhile) && rstpVersion(prt->bridge))
              )
            {
                if(!ptp->learn)
                {
                    if(dry_run) /* state change */
                        return true;
                    PRTSM_to_ROOT_LEARN(ptp, forwardDelay);
                    return false;
                }
                else if(!ptp->forward)
                {
                    if(dry_run) /* state change */
                        return true;
                    PRTSM_to_ROOT_FORWARD(ptp);
                    return false;
                }
            }
            if(ptp->reRoot && ptp->forward)
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_REROOTED(ptp);
                return false;
            }
            if(ptp->rrWhile != FwdDelay)
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_ROOT_PORT(ptp, FwdDelay);
                return false;
            }
            return false;
     /* DesignatedPort role transitions */
        case PRTSM_DESIGNATED_PROPOSE:
            /* return; */
        case PRTSM_DESIGNATED_AGREED:
            /* return; */
        case PRTSM_DESIGNATED_SYNCED:
            /* return; */
        case PRTSM_DESIGNATED_RETIRED:
            /* return; */
        case PRTSM_DESIGNATED_FORWARD:
            /* return; */
        case PRTSM_DESIGNATED_LEARN:
            /* return; */
        case PRTSM_DESIGNATED_DISCARD:
            if(dry_run) /* state change */
                return true;
            PRTSM_to_DESIGNATED_PORT(ptp);
            return false;
        case PRTSM_DESIGNATED_PORT:
            if(!(ptp->selected && !ptp->updtInfo))
                return false;
            if(ptp->reRoot && (0 == ptp->rrWhile))
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_DESIGNATED_RETIRED(ptp);
                return false;
            }
            if((!ptp->learning && !ptp->forwarding && !ptp->synced)
               || (ptp->agreed && !ptp->synced)
               || (prt->operEdge && !ptp->synced)
               || (ptp->sync && ptp->synced)
              )
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_DESIGNATED_SYNCED(ptp);
                return false;
            }
            if(allSynced && (ptp->proposed || !ptp->agree))
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_DESIGNATED_AGREED(ptp);
                return false;
            }
            if(!ptp->forward && !ptp->agreed && !ptp->proposing
               && !prt->operEdge)
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_DESIGNATED_PROPOSE(ptp);
                return false;
            }
            /* Dont transition to learn/forward when BA inconsistent */
            if(((0 == ptp->fdWhile) || ptp->agreed || prt->operEdge)
               && ((0 == ptp->rrWhile) || !ptp->reRoot) && !ptp->sync
               && !ptp->port->BaInconsistent
              )
            {
                if(!ptp->learn)
                {
                    if(dry_run) /* state change */
                        return true;
                    PRTSM_to_DESIGNATED_LEARN(ptp, forwardDelay);
                    return false;
                }
                else if(!ptp->forward)
                {
                    if(dry_run) /* state change */
                        return true;
                    PRTSM_to_DESIGNATED_FORWARD(ptp);
                    return false;
                }
            }
            /* Transition to discarding when BA inconsistent */
            if(((ptp->sync && !ptp->synced)
                || (ptp->reRoot && (0 != ptp->rrWhile))
                || ptp->disputed
                || ptp->port->BaInconsistent
               )
               && !prt->operEdge && (ptp->learn || ptp->forward)
              )
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_DESIGNATED_DISCARD(ptp, forwardDelay);
                return false;
            }
            return false;
     /* AlternatePort and BackupPort role transitions */
        case PRTSM_BLOCK_PORT:
            if(ptp->selected && !ptp->updtInfo
               && !ptp->learning && !ptp->forwarding
              )
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_ALTERNATE_PORT(ptp, forwardDelay);
            }
            return false;
        case PRTSM_BACKUP_PORT:
            /* return; */
        case PRTSM_ALTERNATE_PROPOSED:
            /* return; */
        case PRTSM_ALTERNATE_AGREED:
            if(dry_run) /* state change */
                return true;
            PRTSM_to_ALTERNATE_PORT(ptp, forwardDelay);
            return false;
        case PRTSM_ALTERNATE_PORT:
            if(!(ptp->selected && !ptp->updtInfo))
                return false;
            if((allSynced && !ptp->agree) || (ptp->proposed && ptp->agree))
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_ALTERNATE_AGREED(ptp);
                return false;
            }
            if(ptp->proposed && !ptp->agree)
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_ALTERNATE_PROPOSED(ptp);
                return false;
            }
            if((ptp->rbWhile != 2 * HelloTime) && (roleBackup == ptp->role))
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_BACKUP_PORT(ptp, HelloTime);
                return false;
            }
            if((ptp->fdWhile != forwardDelay) || ptp->sync || ptp->reRoot
               || !ptp->synced)
            {
                if(dry_run) /* state change */
                    return true;
                PRTSM_to_ALTERNATE_PORT(ptp, forwardDelay);
                return false;
            }
            return false;
    }

    return false;
}

/* 13.35  Port State Transition state machine */

static bool PSTSM_run(per_tree_port_t *ptp, bool dry_run);
#define PSTSM_begin(ptp) PSTSM_to_DISCARDING((ptp), true)

static void PSTSM_to_DISCARDING(per_tree_port_t *ptp, bool begin)
{
    ptp->PSTSM_state = PSTSM_DISCARDING;

    /* This effectively sets BLOCKING state:
    disableLearning();
    disableForwarding();
    */
    if(BR_STATE_BLOCKING != ptp->state)
    {
        if(!ptp->port->deleted)
            MSTP_OUT_set_state(ptp, BR_STATE_BLOCKING);
    }
    ptp->learning = false;
    ptp->forwarding = false;

    if(!begin)
        PSTSM_run(ptp, false /* actual run */);
}

static void PSTSM_to_LEARNING(per_tree_port_t *ptp)
{
    ptp->PSTSM_state = PSTSM_LEARNING;

    /* enableLearning(); */
    if(BR_STATE_LEARNING != ptp->state)
    {
        if(!ptp->port->deleted)
            MSTP_OUT_set_state(ptp, BR_STATE_LEARNING);
    }
    ptp->learning = true;

    PSTSM_run(ptp, false /* actual run */);
}

static void PSTSM_to_FORWARDING(per_tree_port_t *ptp)
{
    ptp->PSTSM_state = PSTSM_FORWARDING;

    /* enableForwarding(); */
    if(BR_STATE_FORWARDING != ptp->state)
    {
        if(!ptp->port->deleted)
            MSTP_OUT_set_state(ptp, BR_STATE_FORWARDING);
    }
    ptp->forwarding = true;

    /* No need to run, no one condition will be met
      PSTSM_run(ptp, false); */
}

static bool PSTSM_run(per_tree_port_t *ptp, bool dry_run)
{
    switch(ptp->PSTSM_state)
    {
        case PSTSM_DISCARDING:
            if(ptp->learn)
            {
                if(dry_run) /* state change */
                    return true;
                PSTSM_to_LEARNING(ptp);
            }
            return false;
        case PSTSM_LEARNING:
            if(!ptp->learn)
            {
                if(dry_run) /* state change */
                    return true;
                PSTSM_to_DISCARDING(ptp, false);
            }
            else if(ptp->forward)
            {
                if(dry_run) /* state change */
                    return true;
                PSTSM_to_FORWARDING(ptp);
            }
            return false;
        case PSTSM_FORWARDING:
            if(!ptp->forward)
            {
                if(dry_run) /* state change */
                    return true;
                PSTSM_to_DISCARDING(ptp, false);
            }
            return false;
    }

    return false;
}

/* 13.36  Topology Change state machine */

#define TCSM_begin(ptp) TCSM_to_INACTIVE((ptp), true)

static void TCSM_to_INACTIVE(per_tree_port_t *ptp, bool begin)
{
    ptp->TCSM_state = TCSM_INACTIVE;

    set_fdbFlush(ptp);
    assign(ptp->tcWhile, 0u);
    set_TopologyChange(ptp->tree, false, ptp->port);
    if(0 == ptp->MSTID) /* CIST */
        ptp->port->tcAck = false;

    if(!begin)
        TCSM_run(ptp, false /* actual run */);
}

static bool TCSM_to_LEARNING(per_tree_port_t *ptp, bool dry_run)
{
    if(dry_run)
    {
        if((ptp->TCSM_state != TCSM_LEARNING) || ptp->rcvdTc || ptp->tcProp)
            return true;
        if(0 == ptp->MSTID) /* CIST */
        {
            port_t *prt = ptp->port;
            if(prt->rcvdTcn || prt->rcvdTcAck)
                return true;
        }
        return false;
    }

    ptp->TCSM_state = TCSM_LEARNING;

    if(0 == ptp->MSTID) /* CIST */
    {
        port_t *prt = ptp->port;
        prt->rcvdTcn = false;
        prt->rcvdTcAck = false;
    }
    ptp->rcvdTc = false;
    ptp->tcProp = false;

    TCSM_run(ptp, false /* actual run */);
    return false;
}

static void TCSM_to_DETECTED(per_tree_port_t *ptp)
{
    ptp->TCSM_state = TCSM_DETECTED;

    newTcWhile(ptp);
    setTcPropTree(ptp);
    /* newInfoXst = TRUE; */
    port_t *prt = ptp->port;
    if(0 == ptp->MSTID)
        prt->newInfo = true;
    else
        prt->newInfoMsti = true;

    TCSM_run(ptp, false /* actual run */);
}

static void TCSM_to_NOTIFIED_TCN(per_tree_port_t *ptp)
{
    ptp->TCSM_state = TCSM_NOTIFIED_TCN;

    newTcWhile(ptp);

    TCSM_run(ptp, false /* actual run */);
}

static void TCSM_to_NOTIFIED_TC(per_tree_port_t *ptp)
{
    ptp->TCSM_state = TCSM_NOTIFIED_TC;

    ptp->rcvdTc = false;
    if(0 == ptp->MSTID) /* CIST */
    {
        port_t *prt = ptp->port;
        prt->rcvdTcn = false;
        if(roleDesignated == ptp->role)
            prt->tcAck = true;
    }
    setTcPropTree(ptp);

    TCSM_run(ptp, false /* actual run */);
}

static void TCSM_to_PROPAGATING(per_tree_port_t *ptp)
{
    ptp->TCSM_state = TCSM_PROPAGATING;

    newTcWhile(ptp);
    set_fdbFlush(ptp);
    ptp->tcProp = false;

    TCSM_run(ptp, false /* actual run */);
}

static void TCSM_to_ACKNOWLEDGED(per_tree_port_t *ptp)
{
    ptp->TCSM_state = TCSM_ACKNOWLEDGED;

    assign(ptp->tcWhile, 0u);
    set_TopologyChange(ptp->tree, false, ptp->port);
    ptp->port->rcvdTcAck = false;

    TCSM_run(ptp, false /* actual run */);
}

static void TCSM_to_ACTIVE(per_tree_port_t *ptp)
{
    ptp->TCSM_state = TCSM_ACTIVE;

    TCSM_run(ptp, false /* actual run */);
}

static bool TCSM_run(per_tree_port_t *ptp, bool dry_run)
{
    bool active_port;
    port_t *prt = ptp->port;

    switch(ptp->TCSM_state)
    {
        case TCSM_INACTIVE:
            if(ptp->learn && !ptp->fdbFlush)
            {
                if(dry_run) /* state change */
                    return true;
                TCSM_to_LEARNING(ptp, false /* actual run */);
            }
            return false;
        case TCSM_LEARNING:
            active_port = (roleRoot == ptp->role)
                          || (roleDesignated == ptp->role)
                          || (roleMaster == ptp->role);
            if(active_port && ptp->forward && !prt->operEdge)
            {
                if(dry_run) /* state change */
                    return true;
                TCSM_to_DETECTED(ptp);
                return false;
            }
            if(ptp->rcvdTc || prt->rcvdTcn || prt->rcvdTcAck || ptp->tcProp)
            {
                return TCSM_to_LEARNING(ptp, dry_run);
            }
            else if(!active_port && !(ptp->learn || ptp->learning))
            {
                if(dry_run) /* state change */
                    return true;
                TCSM_to_INACTIVE(ptp, false);
            }
            return false;
        case TCSM_NOTIFIED_TCN:
            if(dry_run) /* state change */
                return true;
            TCSM_to_NOTIFIED_TC(ptp);
            return false;
        case TCSM_DETECTED:
            /* return; */
        case TCSM_NOTIFIED_TC:
            /* return; */
        case TCSM_PROPAGATING:
            /* return; */
        case TCSM_ACKNOWLEDGED:
            if(dry_run) /* state change */
                return true;
            TCSM_to_ACTIVE(ptp);
            return false;
        case TCSM_ACTIVE:
            active_port = (roleRoot == ptp->role)
                          || (roleDesignated == ptp->role)
                          || (roleMaster == ptp->role);
            if(!active_port || prt->operEdge)
            {
                if(dry_run) /* state change */
                    return true;
                TCSM_to_LEARNING(ptp, false /* actual run */);
                return false;
            }
            if(prt->rcvdTcn)
            {
                if(dry_run) /* state change */
                    return true;
                TCSM_to_NOTIFIED_TCN(ptp);
                return false;
            }
            if(ptp->rcvdTc)
            {
                if(dry_run) /* state change */
                    return true;
                TCSM_to_NOTIFIED_TC(ptp);
                return false;
            }
            if(ptp->tcProp/* && !prt->operEdge */)
            {
                if(dry_run) /* state change */
                    return true;
                TCSM_to_PROPAGATING(ptp);
                return false;
            }
            if(prt->rcvdTcAck)
            {
                if(dry_run) /* state change */
                    return true;
                TCSM_to_ACKNOWLEDGED(ptp);
                return false;
            }
            return false;
    }

    return false;
}

/* Execute BEGIN state. We do not define BEGIN variable
 * but instead xxx_state_machines_begin execute begin state
 * abd do one step out of it
 */

static void tree_state_machines_begin(tree_t *tree)
{
    bridge_t *br = tree->bridge;
    per_tree_port_t *ptp;

    if(!br->bridgeEnabled)
        return;

    /* 13.32  Port Information state machine */
    FOREACH_PTP_IN_TREE(ptp, tree)
    {
        ptp->start_time = br->uptime; /* 12.8.2.2.3 b) */
        PISM_begin(ptp);
    }

    /* 13.33  Port Role Selection state machine */
    PRSSM_begin(tree);

    /* 13.34  Port Role Transitions state machine */
    FOREACH_PTP_IN_TREE(ptp, tree)
        PRTSM_begin(ptp);
    /* 13.35  Port State Transition state machine */
    FOREACH_PTP_IN_TREE(ptp, tree)
        PSTSM_begin(ptp);
    /* 13.36  Topology Change state machine */
    FOREACH_PTP_IN_TREE(ptp, tree)
        TCSM_begin(ptp);

    br_state_machines_run(br);
}

static void prt_state_machines_begin(port_t *prt)
{
    bridge_t *br = prt->bridge;
    tree_t *tree;
    per_tree_port_t *ptp;

    if(!br->bridgeEnabled)
        return;

    /* 13.28  Port Receive state machine */
    PRSM_begin(prt);
    /* 13.29  Port Protocol Migration state machine */
    PPMSM_begin(prt);
    /* 13.30  Bridge Detection state machine */
    BDSM_begin(prt);
    /* 13.31  Port Transmit state machine */
    PTSM_begin(prt);

    /* 13.32  Port Information state machine */
    FOREACH_PTP_IN_PORT(ptp, prt)
    {
        ptp->start_time = br->uptime; /* 12.8.2.2.3 b) */
        PISM_begin(ptp);
    }

    /* 13.33  Port Role Selection state machine */
    FOREACH_TREE_IN_BRIDGE(tree, br)
        PRSSM_run(tree, false /* actual run */);

    /* 13.34  Port Role Transitions state machine */
    FOREACH_PTP_IN_PORT(ptp, prt)
        PRTSM_begin(ptp);
    /* 13.35  Port State Transition state machine */
    FOREACH_PTP_IN_PORT(ptp, prt)
        PSTSM_begin(ptp);
    /* 13.36  Topology Change state machine */
    FOREACH_PTP_IN_PORT(ptp, prt)
        TCSM_begin(ptp);

    br_state_machines_run(br);
}

static void br_state_machines_begin(bridge_t *br)
{
    port_t *prt;
    per_tree_port_t *ptp;
    tree_t *tree;

    if(!br->bridgeEnabled)
        return;

    /* 13.28  Port Receive state machine */
    FOREACH_PORT_IN_BRIDGE(prt, br)
        PRSM_begin(prt);
    /* 13.29  Port Protocol Migration state machine */
    FOREACH_PORT_IN_BRIDGE(prt, br)
        PPMSM_begin(prt);
    /* 13.30  Bridge Detection state machine */
    FOREACH_PORT_IN_BRIDGE(prt, br)
        BDSM_begin(prt);
    /* 13.31  Port Transmit state machine */
    FOREACH_PORT_IN_BRIDGE(prt, br)
        PTSM_begin(prt);

    /* 13.32  Port Information state machine */
    FOREACH_PORT_IN_BRIDGE(prt, br)
    {
        FOREACH_PTP_IN_PORT(ptp, prt)
        {
            ptp->start_time = br->uptime; /* 12.8.2.2.3 b) */
            PISM_begin(ptp);
        }
    }

    /* 13.33  Port Role Selection state machine */
    FOREACH_TREE_IN_BRIDGE(tree, br)
        PRSSM_begin(tree);

    /* 13.34  Port Role Transitions state machine */
    FOREACH_PORT_IN_BRIDGE(prt, br)
    {
        FOREACH_PTP_IN_PORT(ptp, prt)
            PRTSM_begin(ptp);
    }
    /* 13.35  Port State Transition state machine */
    FOREACH_PORT_IN_BRIDGE(prt, br)
    {
        FOREACH_PTP_IN_PORT(ptp, prt)
            PSTSM_begin(ptp);
    }
    /* 13.36  Topology Change state machine */
    FOREACH_PORT_IN_BRIDGE(prt, br)
    {
        FOREACH_PTP_IN_PORT(ptp, prt)
            TCSM_begin(ptp);
    }

    br_state_machines_run(br);
}

/* Run each state machine.
 * Return false iff all state machines in dry run indicate that
 * state will not be changed. Otherwise return true.
 */
static bool __br_state_machines_run(bridge_t *br, bool dry_run)
{
    port_t *prt;
    per_tree_port_t *ptp;
    tree_t *tree;

    /* Check if bridge assurance timer expires */
    FOREACH_PORT_IN_BRIDGE(prt, br)
    {
        if(prt->portEnabled && assurancePort(prt)
           && (0 == prt->brAssuRcvdInfoWhile) && !prt->BaInconsistent
          )
        {
            if(dry_run) /* state change */
                return true;
            prt->BaInconsistent = true;
            ERROR_PRTNAME(prt->bridge, prt, "Bridge assurance inconsistent");
        }
    }

    /* 13.28  Port Receive state machine */
    FOREACH_PORT_IN_BRIDGE(prt, br)
    {
        if(PRSM_run(prt, dry_run) && dry_run)
            return true;
    }
    /* 13.29  Port Protocol Migration state machine */
    FOREACH_PORT_IN_BRIDGE(prt, br)
    {
        if(PPMSM_run(prt, dry_run) && dry_run)
            return true;
    }
    /* 13.30  Bridge Detection state machine */
    FOREACH_PORT_IN_BRIDGE(prt, br)
    {
        if(BDSM_run(prt, dry_run) && dry_run)
            return true;
    }
    /* 13.31  Port Transmit state machine */
    FOREACH_PORT_IN_BRIDGE(prt, br)
    {
        if(PTSM_run(prt, dry_run) && dry_run)
            return true;
    }

    /* 13.32  Port Information state machine */
    FOREACH_PORT_IN_BRIDGE(prt, br)
    {
        FOREACH_PTP_IN_PORT(ptp, prt)
        {
            if(PISM_run(ptp, dry_run) && dry_run)
                return true;
        }
    }

    /* 13.33  Port Role Selection state machine */
    FOREACH_TREE_IN_BRIDGE(tree, br)
    {
        if(PRSSM_run(tree, dry_run) && dry_run)
            return true;
    }

    /* 13.34  Port Role Transitions state machine */
    FOREACH_PORT_IN_BRIDGE(prt, br)
    {
        FOREACH_PTP_IN_PORT(ptp, prt)
        {
            if(PRTSM_run(ptp, dry_run) && dry_run)
                return true;
        }
    }
    /* 13.35  Port State Transition state machine */
    FOREACH_PORT_IN_BRIDGE(prt, br)
    {
        FOREACH_PTP_IN_PORT(ptp, prt)
        {
            if(PSTSM_run(ptp, dry_run) && dry_run)
                return true;
        }
    }
    /* 13.36  Topology Change state machine */
    FOREACH_PORT_IN_BRIDGE(prt, br)
    {
        FOREACH_PTP_IN_PORT(ptp, prt)
        {
            if(TCSM_run(ptp, dry_run) && dry_run)
                return true;
        }
    }

    return false;
}

/* Run state machines until their state stabilizes.
 * Do not consume more than 1 second.
 */
static void br_state_machines_run(bridge_t *br)
{
    struct timespec tv, tv_end;
    signed long delta;

    if(!br->bridgeEnabled)
        return;

    clock_gettime(CLOCK_MONOTONIC, &tv_end);
    ++(tv_end.tv_sec);

    do {
        if(!__br_state_machines_run(br, true /* dry run */))
            return;
        __br_state_machines_run(br, false /* actual run */);

        /* Check for the timeout */
        clock_gettime(CLOCK_MONOTONIC, &tv);
        if(0 < (delta = tv.tv_sec - tv_end.tv_sec))
            return;
        if(0 == delta)
        {
            delta = tv.tv_nsec - tv_end.tv_nsec;
            if(0 < delta)
                return;
        }
    } while(true);
}
