/*
 * Copyright (c) 2008, Atheros Communications Inc.
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

#ifndef _ATHRS_IOCTL_H
#define _ATHRS_IOCTL_H 1

#ifndef ETHREG_TOOL_BUILD
#include <linux/types.h>
#include <linux/spinlock_types.h>
#include <linux/workqueue.h>
//#include <asm/system.h>
#include <linux/netdevice.h>
#include <net/inet_ecn.h>                /* XXX for TOS */
#include <linux/if_ether.h>
#else
#include <stdint.h>
#endif
#define S26_RD_PHY       (SIOCDEVPRIVATE | 0x1)
#define S26_WR_PHY       (SIOCDEVPRIVATE | 0x2)
#define S26_FORCE_PHY    (SIOCDEVPRIVATE | 0x3)


struct eth_cfg_params {
    uint16_t cmd;
    char    ad_name[IFNAMSIZ];      /* if name, e.g. "eth0" */
    uint16_t vlanid;
    uint16_t portnum;           /* pack to fit, yech */
    uint32_t phy_reg;
    uint32_t tos;
    uint32_t val;
    uint8_t duplex;
    uint8_t  mac_addr[6];
};


#ifdef CONFIG_ATHRS_QOS

#define ETH_SOFT_CLASS   (SIOCDEVPRIVATE | 0x4)
#define ETH_PORT_QOS     (SIOCDEVPRIVATE | 0x5)
#define ETH_VLAN_QOS     (SIOCDEVPRIVATE | 0x6)
#define ETH_DA_QOS       (SIOCDEVPRIVATE | 0x7)
#define ETH_IP_QOS       (SIOCDEVPRIVATE | 0x8)
#define ETH_PORT_ILIMIT  (SIOCDEVPRIVATE | 0x9)
#define ETH_PORT_ELIMIT  (SIOCDEVPRIVATE | 0xa)
#define ETH_PORT_EQLIMIT (SIOCDEVPRIVATE | 0xb)
struct ath_qops {
    uint32_t  (*reg_read)(char *ad_name,uint32_t Reg);
    void      (*reg_write)(char *ad_name,uint32_t Reg, uint32_t Val);
    void      (*reg_rmw_set)(char *ad_name,uint32_t Reg, uint32_t Val);
    void      (*reg_rmw_clear)(char *ad_name,uint32_t Reg, uint32_t Val);
    int       (*enable_qos)(char *ad_name);
    void      (*disable_qos)(char *ad_name);
    int       qos_cap;
    int       qos_flag;
};

int athrs_config_qos(struct eth_cfg_params *ethcfg, int cmd);
int athr_register_qos(void *mac);

#endif

#ifdef CONFIG_MACH_AR7240
#define SW_ONLY_MODE     (SIOCDEVPRIVATE | 0x8)
#define SOFT_LED_BLINK   (SIOCDEVPRIVATE | 0x9)
#define ETH_DMA_CHECK    (SIOCDEVPRIVATE | 0xa)
#endif

#ifdef CONFIG_ATHRS_QOS
#define  S26_QOS_CTL			(SIOCDEVPRIVATE | 0x9)
#endif

#ifdef CONFIG_AR7240_S26_VLAN_IGMP
// Add or remove ports to the device
// bit0--->port0;bit1--->port1.
#define  S26_VLAN_ADDPORTS      (SIOCDEVPRIVATE | 0x4)
#define  S26_VLAN_DELPORTS      (SIOCDEVPRIVATE | 0x5)

// Set the tag mode to the port.
#define  S26_VLAN_SETTAGMODE    (SIOCDEVPRIVATE | 0x6)

// Set default vlan id to the port
#define  S26_VLAN_SETDEFAULTID  (SIOCDEVPRIVATE | 0x7)

// Enable or disable IGMP snooping based on a vlanid
#define  S26_IGMP_ON_OFF    (SIOCDEVPRIVATE | 0x8)
//#define  S26_IGMP_OFF         (SIOCDEVPRIVATE | 0x9)

// Get a link status from the specified port.
#define  S26_LINK_GETSTAT   (SIOCDEVPRIVATE | 0xA)

#define  S26_VLAN_ENABLE    (SIOCDEVPRIVATE | 0xB)
#define  S26_VLAN_DISABLE   (SIOCDEVPRIVATE | 0xC)

#define  S26_ARL_ADD            (SIOCDEVPRIVATE | 0xD)
#define  S26_ARL_DEL            (SIOCDEVPRIVATE | 0xE)

#define  S26_MCAST_CLR      (SIOCDEVPRIVATE | 0xF)
#define  S26_PACKET_FLAG    (SIOCDEVPRIVATE | 0x0)

#define  VLAN_DEV_INFO(x) ((struct eth_vlan_dev_info *)x->priv)

struct eth_vlan_dev_info {
    unsigned long inmap[8];
    char * outmap[16];
    unsigned short vlan_id;
};

#endif

typedef struct {
    u_int8_t uc[6];
} mac_addr_t;

struct arl_struct {
    mac_addr_t mac_addr;
    int port_map;
    int sa_drop; 
};



#endif //_ATHRS_IOCTL_H
