/*****************************************************************************
  Copyright (c) 2006 EMC Corporation.
  Copyright (c) 2011 Factor-SPE

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License as published by the Free
  Software Foundation; either version 2 of the License, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  The full GNU General Public License is included in this distribution in the
  file called LICENSE.

  Authors: Srinivas Aji <Aji_Srinivas@emc.com>
  Authors: Vitalii Demianets <vitas@nppfactor.kiev.ua>

******************************************************************************/

#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/param.h>
#include <linux/if_bridge.h>
#include <asm/byteorder.h>

#include "bridge_ctl.h"
#include "ctl_functions.h"
#include "netif_utils.h"
#include "packet.h"
#include "log.h"
#include "mstp.h"
#include "driver.h"
#include "libnetlink.h"

static LIST_HEAD(bridges);

static bridge_t * create_br(int if_index)
{
    bridge_t *br;
    TST((br = calloc(1, sizeof(*br))) != NULL, NULL);

    /* Init system dependent info */
    br->sysdeps.if_index = if_index;
    if_indextoname(if_index, br->sysdeps.name);
    get_hwaddr(br->sysdeps.name, br->sysdeps.macaddr);

    INFO("Add bridge %s", br->sysdeps.name);
    if(!MSTP_IN_bridge_create(br, br->sysdeps.macaddr))
    {
        free(br);
        return NULL;
    }

    list_add_tail(&br->list, &bridges);
    return br;
}

static bridge_t * find_br(int if_index)
{
    bridge_t *br;
    list_for_each_entry(br, &bridges, list)
    {
        if(br->sysdeps.if_index == if_index)
            return br;
    }
    return NULL;
}

static port_t * create_if(bridge_t * br, int if_index)
{
    port_t *prt;
    TST((prt = calloc(1, sizeof(*prt))) != NULL, NULL);

    /* Init system dependent info */
    prt->sysdeps.if_index = if_index;
    if_indextoname(if_index, prt->sysdeps.name);
    get_hwaddr(prt->sysdeps.name, prt->sysdeps.macaddr);

    int portno;
    if(0 > (portno = get_bridge_portno(prt->sysdeps.name)))
    {
        ERROR("Couldn't get port number for %s", prt->sysdeps.name);
        free(prt);
        return NULL;
    }
    if((0 == portno) || (portno > MAX_PORT_NUMBER))
    {
        ERROR("Port number for %s is invalid (%d)", prt->sysdeps.name, portno);
        free(prt);
        return NULL;
    }

    INFO("Add iface %s as port#%d to bridge %s", prt->sysdeps.name,
         portno, br->sysdeps.name);
    prt->bridge = br;
    if(!MSTP_IN_port_create_and_add_tail(prt, portno))
    {
        free(prt);
        return NULL;
    }

    return prt;
}

static port_t * find_if(bridge_t * br, int if_index)
{
    port_t *prt;
    list_for_each_entry(prt, &br->ports, br_list)
    {
        if(prt->sysdeps.if_index == if_index)
            return prt;
    }
    return NULL;
}

static inline void delete_if(port_t *prt)
{
    MSTP_IN_delete_port(prt);
    free(prt);
}

static inline bool delete_if_byindex(bridge_t * br, int if_index)
{
    port_t *prt;
    if(!(prt = find_if(br, if_index)))
        return false;
    delete_if(prt);
    return true;
}

static bool delete_br_byindex(int if_index)
{
    bridge_t *br;
    if(!(br = find_br(if_index)))
        return false;
    list_del(&br->list);
    MSTP_IN_delete_bridge(br);
    free(br);
    return true;
}

void bridge_one_second(void)
{
    bridge_t *br;
    list_for_each_entry(br, &bridges, list)
        MSTP_IN_one_second(br);
}

/* New MAC address is stored in addr, which also holds the old value on entry.
   Return true if the address changed */
static bool check_mac_address(char *name, __u8 *addr)
{
    __u8 temp_addr[ETH_ALEN];
    if(get_hwaddr(name, temp_addr))
    {
        LOG("Error getting hw address: %s", name);
        /* Error. Ignore the new value */
        return false;
    }
    if(memcmp(addr, temp_addr, sizeof(temp_addr)) == 0)
        return false;
    else
    {
        memcpy(addr, temp_addr, sizeof(temp_addr));
        return true;
    }
}

static void set_br_up(bridge_t * br, bool up)
{
    INFO("%s was %s", br->sysdeps.name, br->sysdeps.up ? "up" : "down");
    INFO("Set bridge %s %s", br->sysdeps.name, up ? "up" : "down");

    bool changed = false;

    if(up != br->sysdeps.up)
    {
        br->sysdeps.up = up;
        changed = true;
    }

    if(check_mac_address(br->sysdeps.name, br->sysdeps.macaddr))
    {
        /* MAC address changed */
        /* Notify bridge address change */
        MSTP_IN_set_bridge_address(br, br->sysdeps.macaddr);
    }

    if(changed)
        MSTP_IN_set_bridge_enable(br, br->sysdeps.up);
}

static void set_if_up(port_t *prt, bool up)
{
    INFO("Port %s : %s", prt->sysdeps.name, (up ? "up" : "down"));
    int speed = -1;
    int duplex = -1;
    bool changed = false;

    if(check_mac_address(prt->sysdeps.name, prt->sysdeps.macaddr))
    {
        /* MAC address changed */
        if(check_mac_address(prt->bridge->sysdeps.name,
           prt->bridge->sysdeps.macaddr))
        {
            /* Notify bridge address change */
            MSTP_IN_set_bridge_address(prt->bridge,
                                       prt->bridge->sysdeps.macaddr);
        }
    }

    if(!up)
    { /* Down */
        if(prt->sysdeps.up)
        {
            prt->sysdeps.up = false;
            changed = true;
        }
    }
    else
    { /* Up */
        int r = ethtool_get_speed_duplex(prt->sysdeps.name, &speed, &duplex);
        if((r < 0) || (speed < 0))
            speed = 10;
        if((r < 0) || (duplex < 0))
            duplex = 0; /* Assume half duplex */

        if(speed != prt->sysdeps.speed)
        {
            prt->sysdeps.speed = speed;
            changed = true;
        }
        if(duplex != prt->sysdeps.duplex)
        {
            prt->sysdeps.duplex = duplex;
            changed = true;
        }
        if(!prt->sysdeps.up)
        {
            prt->sysdeps.up = true;
            changed = true;
        }
    }
    if(changed)
        MSTP_IN_set_port_enable(prt, prt->sysdeps.up, prt->sysdeps.speed,
                                prt->sysdeps.duplex);
}

/* br_index == if_index means: interface is bridge master */
int bridge_notify(int br_index, int if_index, bool newlink, unsigned flags)
{
    port_t *prt;
    bridge_t *br = NULL, *other_br;
    bool up = !!(flags & IFF_UP);
    bool running = up && (flags & IFF_RUNNING);

    LOG("br_index %d, if_index %d, newlink %d, up %d, running %d",
        br_index, if_index, newlink, up, running);

    if((br_index >= 0) && (br_index != if_index))
    {
        if(!(br = find_br(br_index)))
            return -2; /* bridge not in list */
        int br_flags = get_flags(br->sysdeps.name);
        if(br_flags >= 0)
            set_br_up(br, !!(br_flags & IFF_UP));
    }

    if(br)
    {
        if(!(prt = find_if(br, if_index)))
        {
            if(!newlink)
            {
                INFO("Got DELLINK for unknown port %d on "
                     "bridge %d", if_index, br_index);
                return -1;
            }
            /* Check if this interface is slave of another bridge */
            list_for_each_entry(other_br, &bridges, list)
            {
                if(other_br != br)
                    if(delete_if_byindex(other_br, if_index))
                    {
                        INFO("Device %d has come to bridge %d. "
                             "Missed notify for deletion from bridge %d",
                             if_index, br_index, other_br->sysdeps.if_index);
                        break;
                    }
            }
            prt = create_if(br, if_index);
        }
        if(!prt)
        {
            ERROR("Couldn't create data for interface %d (master %d)",
                  if_index, br_index);
            return -1;
        }
        if(!newlink)
        {
            delete_if(prt);
            return 0;
        }
        set_if_up(prt, running); /* And speed and duplex */
    }
    else
    { /* Interface is not a bridge slave */
        if(!newlink)
        {
            /* DELLINK not from bridge means interface unregistered. */
            /* Cleanup removed bridge or removed bridge slave */
            if(!delete_br_byindex(if_index))
                list_for_each_entry(br, &bridges, list)
                {
                    if(delete_if_byindex(br, if_index))
                        break;
                }
            return 0;
        }
        else
        { /* This may be a new link */
            if(br_index == if_index)
            {
                if(!(br = find_br(br_index)))
                    return -2; /* bridge not in list */
                set_br_up(br, up);
            }
        }
    }
    return 0;
}

struct llc_header
{
    __u8 dest_addr[ETH_ALEN];
    __u8 src_addr[ETH_ALEN];
    __be16 len8023;
    __u8 d_sap;
    __u8 s_sap;
    __u8 llc_ctrl;
} __attribute__((packed));

/* LLC_PDU_xxx defines snitched from linux/net/llc_pdu.h */
#define LLC_PDU_LEN_U   3   /* header and 1 control byte */
#define LLC_PDU_TYPE_U  3   /* first two bits */

/* 7.12.3 of 802.1D */
#define LLC_SAP_BSPAN   0x42
static const __u8 bridge_group_address[ETH_ALEN] =
{
    0x01, 0x80, 0xc2, 0x00, 0x00, 0x00
};

void bridge_bpdu_rcv(int if_index, const unsigned char *data, int len)
{
    port_t *prt = NULL;
    bridge_t *br;

    LOG("ifindex %d, len %d", if_index, len);

    list_for_each_entry(br, &bridges, list)
    {
        if((prt = find_if(br, if_index)))
            break;
    }
    if(!prt)
        return;

    /* sanity checks */
    TST(br == prt->bridge,);
    TST(prt->sysdeps.up,);

    /* Validate Ethernet and LLC header,
     * maybe we can skip this check thanks to Berkeley filter in packet socket?
     */
    struct llc_header *h;
    unsigned int l;
    TST(len > sizeof(struct llc_header),);
    h = (struct llc_header *)data;
    TST(0 == memcmp(h->dest_addr, bridge_group_address, ETH_ALEN),
             INFO("ifindex %d, len %d, %02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
                  if_index, len,
                  h->dest_addr[0], h->dest_addr[1], h->dest_addr[2],
                  h->dest_addr[3], h->dest_addr[4], h->dest_addr[5])
       );
    l = __be16_to_cpu(h->len8023);
    TST(l <= ETH_DATA_LEN && l <= len - ETH_HLEN && l >= LLC_PDU_LEN_U, );
    TST(h->d_sap == LLC_SAP_BSPAN && h->s_sap == LLC_SAP_BSPAN && (h->llc_ctrl & 0x3) == LLC_PDU_TYPE_U,);

    MSTP_IN_rx_bpdu(prt,
                    /* Don't include LLC header */
                    (bpdu_t *)(data + sizeof(*h)), l - LLC_PDU_LEN_U);
}

static int br_set_state(struct rtnl_handle *rth, unsigned ifindex, __u8 state)
{
    struct
    {
        struct nlmsghdr n;
        struct ifinfomsg ifi;
        char buf[256];
    } req;

    memset(&req, 0, sizeof(req));

    req.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg));
    req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_REPLACE;
    req.n.nlmsg_type = RTM_SETLINK;
    req.ifi.ifi_family = AF_BRIDGE;
    req.ifi.ifi_index = ifindex;

    addattr8(&req.n, sizeof(req.buf), IFLA_PROTINFO, state);

    return rtnl_talk(rth, &req.n, 0, 0, NULL, NULL, NULL);
}

static int br_flush_port(char *ifname)
{
    char fname[128];
    snprintf(fname, sizeof(fname), "/sys/class/net/%s/brport/flush", ifname);
    int fd = open(fname, O_WRONLY);
    TSTM(0 <= fd, -1, "Couldn't open flush file %s for write: %m", fname);
    int write_result = write(fd, "1", 1);
    close(fd);
    TST(1 == write_result, -1);
    return 0;
}

static int br_set_ageing_time(char *brname, unsigned int ageing_time)
{
    char fname[128], str_time[32];
    snprintf(fname, sizeof(fname), "/sys/class/net/%s/bridge/ageing_time",
             brname);
    int fd = open(fname, O_WRONLY);
    TSTM(0 <= fd, -1, "Couldn't open file %s for write: %m", fname);
    int len = sprintf(str_time, "%u", ageing_time * HZ);
    int write_result = write(fd, str_time, len);
    close(fd);
    TST(len == write_result, -1);
    return 0;
}

/* External actions for MSTP protocol */

void MSTP_OUT_set_state(per_tree_port_t *ptp, int new_state)
{
    char * state_name;
    port_t *prt = ptp->port;
    bridge_t *br = prt->bridge;

    if(ptp->state == new_state)
        return;
    ptp->state = driver_set_new_state(ptp, new_state);

    switch(ptp->state)
    {
        case BR_STATE_LISTENING:
            state_name = "listening";
            break;
        case BR_STATE_LEARNING:
            state_name = "learning";
            break;
        case BR_STATE_FORWARDING:
            state_name = "forwarding";
            ++(prt->num_trans_fwd);
            break;
        case BR_STATE_BLOCKING:
            state_name = "blocking";
            ++(prt->num_trans_blk);
            break;
        default:
        case BR_STATE_DISABLED:
            state_name = "disabled";
            break;
    }
    INFO_MSTINAME(br, prt, ptp, "entering %s state", state_name);

    /* Translate new CIST state to the kernel bridge code */
    if(0 == ptp->MSTID)
    { /* CIST */
        if(prt->sysdeps.up)
        {
            if(0 > br_set_state(&rth_state, prt->sysdeps.if_index, ptp->state))
                ERROR_PRTNAME(br, prt, "Couldn't set kernel bridge state %s",
                              state_name);
        }
    }
}

/* This function initiates process of flushing
 * all entries for the given port in all FIDs for the
 * given tree.
 * When this process finishes, implementation should signal
 * this by calling MSTP_IN_all_fids_flushed(per_tree_port_t *ptp)
 */
void MSTP_OUT_flush_all_fids(per_tree_port_t * ptp)
{
    port_t *prt = ptp->port;
    bridge_t *br = prt->bridge;

    /* Translate CIST flushing to the kernel bridge code */
    if(0 == ptp->MSTID)
    { /* CIST */
        if(0 > br_flush_port(prt->sysdeps.name))
            ERROR_PRTNAME(br, prt,
                          "Couldn't flush kernel bridge forwarding database");
    }
    /* Completion signal MSTP_IN_all_fids_flushed will be called by driver */
    INFO_MSTINAME(br, prt, ptp, "Flushing forwarding database");
    driver_flush_all_fids(ptp);
}

void MSTP_OUT_set_ageing_time(port_t *prt, unsigned int ageingTime)
{
    unsigned int actual_ageing_time;
    bridge_t *br = prt->bridge;

    actual_ageing_time = driver_set_ageing_time(prt, ageingTime);
    INFO_PRTNAME(br, prt, "Setting new ageing time to %u", actual_ageing_time);

    /*
     * Translate new ageing time to the kernel bridge code.
     * Kernel bridging code does not support per-port ageing time,
     * so set ageing time for the whole bridge.
     */
    if(0 > br_set_ageing_time(br->sysdeps.name, actual_ageing_time))
        ERROR_BRNAME(br, "Couldn't set new ageing time in kernel bridge");
}

void MSTP_OUT_tx_bpdu(port_t *prt, bpdu_t * bpdu, int size)
{
    char *bpdu_type, *tcflag;
    bridge_t *br = prt->bridge;

    switch(bpdu->protocolVersion)
    {
        case protoSTP:
            switch(bpdu->bpduType)
            {
                case bpduTypeConfig:
                    bpdu_type = "STP-Config";
                    break;
                case bpduTypeTCN:
                    bpdu_type = "STP-TCN";
                    break;
                default:
                    bpdu_type = "STP-UnknownType";
            }
            break;
        case protoRSTP:
            bpdu_type = "RST";
            break;
        case protoMSTP:
            bpdu_type = "MST";
            break;
        default:
            bpdu_type = "UnknownProto";
    }

    ++(prt->num_tx_bpdu);
    if((protoSTP == bpdu->protocolVersion) && (bpduTypeTCN == bpdu->bpduType))
    {
        ++(prt->num_tx_tcn);
        LOG_PRTNAME(br, prt, "sending %s BPDU", bpdu_type);
    }
    else
    {
        tcflag = "";
        if(bpdu->flags & (1 << offsetTc))
        {
            ++(prt->num_tx_tcn);
            tcflag = ", tcFlag";
        }
        LOG_PRTNAME(br, prt, "sending %s BPDU%s", bpdu_type, tcflag);
    }

    struct llc_header h;
    memcpy(h.dest_addr, bridge_group_address, ETH_ALEN);
    memcpy(h.src_addr, prt->sysdeps.macaddr, ETH_ALEN);
    h.len8023 = __cpu_to_be16(size + LLC_PDU_LEN_U);
    h.d_sap = h.s_sap = LLC_SAP_BSPAN;
    h.llc_ctrl = LLC_PDU_TYPE_U;

    struct iovec iov[2] =
    {
        { .iov_base = &h, .iov_len = sizeof(h) },
        { .iov_base = bpdu, .iov_len = size }
    };

    packet_send(prt->sysdeps.if_index, iov, 2, sizeof(h) + size);
}

void MSTP_OUT_shutdown_port(port_t *prt)
{
    if(0 > if_shutdown(prt->sysdeps.name))
        ERROR_PRTNAME(prt->bridge, prt, "Couldn't shutdown port");
}

/* User interface commands */

#define CTL_CHECK_BRIDGE                                       \
    bridge_t *br = find_br(br_index);                          \
    if(NULL == br)                                             \
    {                                                          \
        ERROR("Couldn't find bridge with index %d", br_index); \
        return -1;                                             \
    }

#define CTL_CHECK_BRIDGE_PORT                                              \
    CTL_CHECK_BRIDGE;                                                      \
    port_t *prt = find_if(br, port_index);                                 \
    if(NULL == prt)                                                        \
    {                                                                      \
        ERROR_BRNAME(br, "Couldn't find port with index %d", port_index);  \
        return -1;                                                         \
    }

#define CTL_CHECK_BRIDGE_TREE                                      \
    CTL_CHECK_BRIDGE;                                              \
    tree_t *tree;                                                  \
    bool found = false;                                            \
    __be16 MSTID = __cpu_to_be16(mstid);                           \
    list_for_each_entry(tree, &br->trees, bridge_list)             \
        if(tree->MSTID == MSTID)                                   \
        {                                                          \
            found = true;                                          \
            break;                                                 \
        }                                                          \
    if(!found)                                                     \
    {                                                              \
        ERROR_BRNAME(br, "Couldn't find MSTI with ID %hu", mstid); \
        return -1;                                                 \
    }

#define CTL_CHECK_BRIDGE_PERTREEPORT                                     \
    CTL_CHECK_BRIDGE_PORT;                                               \
    per_tree_port_t *ptp;                                                \
    bool found = false;                                                  \
    __be16 MSTID = __cpu_to_be16(mstid);                                 \
    list_for_each_entry(ptp, &prt->trees, port_list)                     \
        if(ptp->MSTID == MSTID)                                          \
        {                                                                \
            found = true;                                                \
            break;                                                       \
        }                                                                \
    if(!found)                                                           \
    {                                                                    \
        ERROR_PRTNAME(br, prt, "Couldn't find MSTI with ID %hu", mstid); \
        return -1;                                                       \
    }

int CTL_get_cist_bridge_status(int br_index, CIST_BridgeStatus *status,
                               char *root_port_name)
{
    tree_t *cist;
    per_tree_port_t *ptp;

    CTL_CHECK_BRIDGE;
    MSTP_IN_get_cist_bridge_status(br, status);

    /* find root port name by root_port_id */
    cist = GET_CIST_TREE(br);
    *root_port_name = '\0';
    list_for_each_entry(ptp, &cist->ports, tree_list)
        if(ptp->portId == status->root_port_id)
        {
            strncpy(root_port_name, ptp->port->sysdeps.name, IFNAMSIZ);
            break;
        }
    return 0;
}

int CTL_get_msti_bridge_status(int br_index, __u16 mstid,
                               MSTI_BridgeStatus *status, char *root_port_name)
{
    per_tree_port_t *ptp;

    CTL_CHECK_BRIDGE_TREE;
    MSTP_IN_get_msti_bridge_status(tree, status);

    /* find root port name by root_port_id */
    *root_port_name = '\0';
    list_for_each_entry(ptp, &tree->ports, tree_list)
        if(ptp->portId == status->root_port_id)
        {
            strncpy(root_port_name, ptp->port->sysdeps.name, IFNAMSIZ);
            break;
        }
    return 0;
}

int CTL_set_cist_bridge_config(int br_index, CIST_BridgeConfig *cfg)
{
    CTL_CHECK_BRIDGE;
    return MSTP_IN_set_cist_bridge_config(br, cfg);
}

int CTL_set_msti_bridge_config(int br_index, __u16 mstid, __u8 bridge_priority)
{
    CTL_CHECK_BRIDGE_TREE;
    return MSTP_IN_set_msti_bridge_config(tree, bridge_priority);
}

int CTL_get_cist_port_status(int br_index, int port_index,
                             CIST_PortStatus *status)
{
    CTL_CHECK_BRIDGE_PORT;
    MSTP_IN_get_cist_port_status(prt, status);
    return 0;
}

int CTL_get_msti_port_status(int br_index, int port_index, __u16 mstid,
                             MSTI_PortStatus *status)
{
    CTL_CHECK_BRIDGE_PERTREEPORT;
    MSTP_IN_get_msti_port_status(ptp, status);
    return 0;
}

int CTL_set_cist_port_config(int br_index, int port_index,
                             CIST_PortConfig *cfg)
{
    CTL_CHECK_BRIDGE_PORT;
    return MSTP_IN_set_cist_port_config(prt, cfg);
}

int CTL_set_msti_port_config(int br_index, int port_index, __u16 mstid,
                             MSTI_PortConfig *cfg)
{
    CTL_CHECK_BRIDGE_PERTREEPORT;
    return MSTP_IN_set_msti_port_config(ptp, cfg);
}

int CTL_port_mcheck(int br_index, int port_index)
{
    CTL_CHECK_BRIDGE_PORT;
    return MSTP_IN_port_mcheck(prt);
}

int CTL_set_debug_level(int level)
{
    INFO("level %d", level);
    log_level = level;
    return 0;
}

int CTL_get_mstilist(int br_index, int *num_mstis, __u16 *mstids)
{
    CTL_CHECK_BRIDGE;
    return MSTP_IN_get_mstilist(br, num_mstis, mstids) ? 0 : -1;
}

int CTL_create_msti(int br_index, __u16 mstid)
{
    CTL_CHECK_BRIDGE;
    if((!driver_create_msti(br, mstid)) || (!MSTP_IN_create_msti(br, mstid)))
        return -1;
    return 0;
}

int CTL_delete_msti(int br_index, __u16 mstid)
{
    CTL_CHECK_BRIDGE;
    if((!driver_delete_msti(br, mstid)) || (!MSTP_IN_delete_msti(br, mstid)))
        return -1;
    return 0;
}

int CTL_get_mstconfid(int br_index, mst_configuration_identifier_t *cfg)
{
    CTL_CHECK_BRIDGE;
    *cfg = br->MstConfigId;
    return 0;
}

int CTL_set_mstconfid(int br_index, __u16 revision, char *name)
{
    CTL_CHECK_BRIDGE;
    MSTP_IN_set_mst_config_id(br, revision, name);
    return 0;
}

int CTL_get_vids2fids(int br_index, __u16 *vids2fids)
{
    CTL_CHECK_BRIDGE;
    memcpy(vids2fids, br->vid2fid, sizeof(br->vid2fid));
    return 0;
}

int CTL_get_fids2mstids(int br_index, __u16 *fids2mstids)
{
    CTL_CHECK_BRIDGE;
    int i;
    for(i = 0; i < COUNT_OF(br->fid2mstid); ++i)
        fids2mstids[i] = __be16_to_cpu(br->fid2mstid[i]);
    return 0;
}

int CTL_set_vid2fid(int br_index, __u16 vid, __u16 fid)
{
    CTL_CHECK_BRIDGE;
    return MSTP_IN_set_vid2fid(br, vid, fid) ? 0 : -1;
}

int CTL_set_fid2mstid(int br_index, __u16 fid, __u16 mstid)
{
    CTL_CHECK_BRIDGE;
    return MSTP_IN_set_fid2mstid(br, fid, mstid) ? 0 : -1;
}

int CTL_set_vids2fids(int br_index, __u16 *vids2fids)
{
    CTL_CHECK_BRIDGE;
    return MSTP_IN_set_all_vids2fids(br, vids2fids) ? 0 : -1;
}

int CTL_set_fids2mstids(int br_index, __u16 *fids2mstids)
{
    CTL_CHECK_BRIDGE;
    return MSTP_IN_set_all_fids2mstids(br, fids2mstids) ? 0 : -1;
}

int CTL_add_bridges(int *br_array, int* *ifaces_lists)
{
    int i, j, ifcount, brcount = br_array[0];
    bridge_t *br, *other_br;
    port_t *prt, *nxt;
    int br_flags, if_flags;
    int *if_array;
    bool found;

    for(i = 1; i <= brcount; ++i)
    {
        if(NULL == (br = find_br(br_array[i])))
        {
            if(NULL == (br = create_br(br_array[i])))
            {
                ERROR("Couldn't create data for bridge interface %d",
                      br_array[i]);
                return -1;
            }
            if(0 <= (br_flags = get_flags(br->sysdeps.name)))
                set_br_up(br, !!(br_flags & IFF_UP));
        }
        if_array = ifaces_lists[i - 1];
        ifcount = if_array[0];
        /* delete all interfaces which are not in list */
        list_for_each_entry_safe(prt, nxt, &br->ports, br_list)
        {
            found = false;
            for(j = 1; j <= ifcount; ++j)
            {
                if(prt->sysdeps.if_index == if_array[j])
                {
                    found = true;
                    break;
                }
            }
            if(!found)
                delete_if(prt);
        }
        /* add all new interfaces from the list */
        for(j = 1; j <= ifcount; ++j)
        {
            if(NULL != find_if(br, if_array[j]))
                continue;
            /* Check if this interface is slave of another bridge */
            list_for_each_entry(other_br, &bridges, list)
            {
                if(other_br != br)
                    if(delete_if_byindex(other_br, if_array[j]))
                    {
                        INFO("Device %d has come to bridge %s. "
                             "Missed notify for deletion from bridge %s",
                             if_array[j], br->sysdeps.name,
                             other_br->sysdeps.name);
                        break;
                    }
            }
            if(NULL == (prt = create_if(br, if_array[j])))
            {
                INFO("Couldn't create data for interface %d (master %s)",
                     if_array[j], br->sysdeps.name);
                continue;
            }
            if(0 <= (if_flags = get_flags(prt->sysdeps.name)))
                set_if_up(prt, (IFF_UP | IFF_RUNNING) ==
                               (if_flags & (IFF_UP | IFF_RUNNING))
                         );
        }
    }

    return 0;
}

int CTL_del_bridges(int *br_array)
{
    int i, brcount = br_array[0];

    for(i = 1; i <= brcount; ++i)
        delete_br_byindex(br_array[i]);

    return 0;
}
