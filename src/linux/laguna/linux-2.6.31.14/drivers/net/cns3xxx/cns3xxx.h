/*******************************************************************************
 *
 *   Copyright (c) 2009 Cavium Networks 
 *
 *   This program is free software; you can redistribute it and/or modify it
 *   under the terms of the GNU General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *   more details.
 *
 *   You should have received a copy of the GNU General Public License along with
 *   this program; if not, write to the Free Software Foundation, Inc., 59
 *   Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *   The full GNU General Public License is included in this distribution in the
 *   file called LICENSE.
 *
 ********************************************************************************/

#ifndef CNS3XXX_H
#define CNS3XXX_H

#include "cns3xxx_symbol.h"
#include "cns3xxx_config.h"
#include <linux/cns3xxx/switch_api.h>

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/bootmem.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/reboot.h>
#include <asm/bitops.h>
#include <asm/irq.h>
#include <asm/io.h>
//#include <asm/hardware.h>
#include <linux/pci.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/if_ether.h>
#include <linux/icmp.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/if_arp.h>
#include <net/arp.h>


#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
#include <linux/if_vlan.h>
#endif

//#define VERSION "1.0"


typedef struct
{
	int32_t sdp; // segment data pointer

#ifdef CONFIG_SWITCH_BIG_ENDIAN
	u32 cown:1;
	u32 eor:1;
	u32 fsd:1;
	u32 lsd:1;
	u32 interrupt:1;
	u32 fr:1;
	u32 fp:1; // force priority
	u32 pri:3;
	u32 rsv_1:3; // reserve
	u32 ico:1;
	u32 uco:1;
	u32 tco:1;
	u32 sdl:16; // segment data length

#else
	u32 sdl:16; // segment data length
	u32 tco:1;
	u32 uco:1;
	u32 ico:1;
	u32 rsv_1:3; // reserve
	u32 pri:3;
	u32 fp:1; // force priority
	u32 fr:1;
	u32 interrupt:1;
	u32 lsd:1;
	u32 fsd:1;
	u32 eor:1;
	u32 cown:1;
#endif

#ifdef CONFIG_SWITCH_BIG_ENDIAN
	u32 rsv_3:5;
	u32 fewan:1;
	u32 ewan:1;
	u32 mark:3;
	u32 pmap:5;
	u32 rsv_2:9;
	u32 dels:1;
	u32 inss:1;
	u32 sid:4;
	u32 stv:1;
	u32 ctv:1;
#else
	u32 ctv:1;
	u32 stv:1;
	u32 sid:4;
	u32 inss:1;
	u32 dels:1;
	u32 rsv_2:9;
	u32 pmap:5;
	u32 mark:3;
	u32 ewan:1;
	u32 fewan:1;
	u32 rsv_3:5;
#endif

#ifdef CONFIG_SWITCH_BIG_ENDIAN
	u32 s_pri:3;
	u32 s_dei:1;
	u32 s_vid:12;
	u32 c_pri:3;
	u32 c_cfs:1;
	u32 c_vid:12;
#else
	u32 c_vid:12;
	u32 c_cfs:1;
	u32 c_pri:3;
	u32 s_vid:12;
	u32 s_dei:1;
	u32 s_pri:3;
#endif

	u8 alignment[16]; // for alignment 32 byte

} __attribute__((packed)) TXDesc;

typedef struct
{
	u32 sdp;

#ifdef CONFIG_SWITCH_BIG_ENDIAN
	u32 cown:1;
	u32 eor:1;
	u32 fsd:1;
	u32 lsd:1;
	u32 hr :6;
	u32 prot:4;
	u32 ipf:1;
	u32 l4f:1;
	u32 sdl:16;
#else
	u32 sdl:16;
	u32 l4f:1;
	u32 ipf:1;
	u32 prot:4;
	u32 hr :6;
	u32 lsd:1;
	u32 fsd:1;
	u32 eor:1;
	u32 cown:1;
#endif

#ifdef CONFIG_SWITCH_BIG_ENDIAN
	u32 rsv_3:11;
	u32 ip_offset:5;
	u32 rsv_2:1;
	u32 tc:2;
	u32 un_eth:1;
	u32 crc_err:1;
	u32 sp:3;
	u32 rsv_1:2;
	u32 e_wan:1;
	u32 exdv:1;
	u32 iwan:1;
	u32 unv:1;
	u32 stv:1;
	u32 ctv:1;
#else
	u32 ctv:1;
	u32 stv:1;
	u32 unv:1;
	u32 iwan:1;
	u32 exdv:1;
	u32 e_wan:1;
	u32 rsv_1:2;
	u32 sp:3;
	u32 crc_err:1;
	u32 un_eth:1;
	u32 tc:2;
	u32 rsv_2:1;
	u32 ip_offset:5;
	u32 rsv_3:11;
#endif

#ifdef CONFIG_SWITCH_BIG_ENDIAN
	u32 s_pri:3;
	u32 s_dei:1;
	u32 s_vid:12;
	u32 c_pri:3;
	u32 c_cfs:1;
	u32 c_vid:12;
#else
	u32 c_vid:12;
	u32 c_cfs:1;
	u32 c_pri:3;
	u32 s_vid:12;
	u32 s_dei:1;
	u32 s_pri:3;
#endif

	u8 alignment[16]; // for alignment 32 byte

} __attribute__((packed)) RXDesc;

typedef struct {
	TXDesc *tx_desc;
	struct sk_buff *skb; // for free skb
	u32 pri;
	unsigned long j;
	unsigned long tx_index;
}TXBuffer;

typedef struct {
	RXDesc *rx_desc;
	struct sk_buff *skb; // rx path need to fill some skb field, ex: length ...
#ifdef NCNB_TEST
	u32 ncnb_index;
#endif
}RXBuffer;


typedef struct {
	TXBuffer *head; 
	TXDesc *tx_desc_head_vir_addr;
	dma_addr_t tx_desc_head_phy_addr;
	u32 cur_index; // for put send packet
	spinlock_t tx_lock;
	u32 non_free_tx_skb;
	u32 free_tx_skb_index;
	u32 ring_size;
	u32 max_ring_size;
}TXRing;


typedef struct {
	RXBuffer *head;
	RXDesc *rx_desc_head_vir_addr;
	dma_addr_t rx_desc_head_phy_addr;
	u32 cur_index;
	u32 ring_size;
	u32 max_ring_size;
}RXRing;

#if 0
typedef struct 
{
        CNS3XXXIoctlCmd cmd;
        TXRing *tx_ring;
        RXRing *rx_ring;
}CNS3XXXRingStatus;
#endif


#define RX_RING0(priv) (priv->rx_ring[0])
#define TX_RING0(priv) (priv->tx_ring[0])


static inline u32 get_rx_ring_size(const RXRing *ring)
{
	//printk("rx ring->ring_size: %d\n", ring->ring_size);
	return ring->ring_size;
}

static inline u32 get_tx_ring_size(TXRing *ring)
{
	//printk("tx ring->ring_size: %d\n", ring->ring_size);
	return ring->ring_size;
}

static inline RXBuffer *get_rx_ring_head(const RXRing *rx_ring)
{
	return rx_ring->head;
}

static inline TXBuffer *get_tx_ring_head(TXRing *tx_ring)
{
	return tx_ring->head;
}

static inline RXBuffer *get_cur_rx_buffer(RXRing *rx_ring)
{
	return rx_ring->head + rx_ring->cur_index;
}

static inline TXBuffer *get_cur_tx_buffer(TXRing *tx_ring)
{
	return tx_ring->head + tx_ring->cur_index;
}

static inline u32 get_rx_head_phy_addr(RXRing *rx_ring)
{
	return rx_ring->rx_desc_head_phy_addr;
}

static inline u32 get_tx_ring_head_phy_addr(TXRing *tx_ring)
{
	return tx_ring->tx_desc_head_phy_addr;
}


static inline u32 get_rx_cur_index(RXRing *rx_ring)
{
	return rx_ring->cur_index;
}

static inline u32 get_tx_cur_index(TXRing *tx_ring)
{
	return tx_ring->cur_index;
}

static inline u32 get_tx_cur_phy_addr(u8 ring_num)
{
	if (ring_num == 0)
		return TS_DESC_PTR0_REG;	
	if (ring_num == 1)
		return TS_DESC_PTR1_REG;	
	return 0; // fail
}

static inline void rx_index_next(RXRing *ring)
{ 
        ring->cur_index = ((ring->cur_index + 1) % ring->ring_size);
}  
static inline void tx_index_next(TXRing *ring)
{ 
        ring->cur_index = ((ring->cur_index + 1) % ring->ring_size);
}



struct CNS3XXXPrivate_;

typedef int (*RXFuncPtr)(struct sk_buff *skb, RXDesc*tx_desc_ptr, const struct CNS3XXXPrivate_* );
typedef int (*TXFuncPtr)(TXDesc*tx_desc_ptr, const struct CNS3XXXPrivate_*, struct sk_buff *);
typedef void (*OpenPtr)(void);
typedef void (*ClosePtr)(void);


// for ethtool set operate
typedef struct{

}NICSetting;

typedef struct{
	int pmap; // for port base, force route
	int is_wan; // mean the net device is WAN side.
	//u16 gid;
	u16 s_tag;
	//u8 mac_type; // VLAN base, or port base;
	u16 vlan_tag;

	// do port base mode and vlan base mode work
	RXFuncPtr rx_func;
	TXFuncPtr tx_func;
	OpenPtr open;
	ClosePtr close;
	u8 which_port;
	//NICSetting *nic_setting;
	u8 *mac; // point to a mac address array
	VLANTableEntry *vlan_table_entry;
	ARLTableEntry *arl_table_entry;
        NICSetting *nic_setting;
	const char *name; // 16 bytes, reference include/linux/netdevice.h IFNAMSIZ
}NetDevicePriv;

typedef struct
{
        u8 num_rx_queues;
        u8 num_tx_queues;
        TXRing *tx_ring;
        RXRing *rx_ring;
}RingInfo;


/* store this information for the driver.. */
typedef struct CNS3XXXPrivate_
{
	u8 num_rx_queues;
	u8 num_tx_queues;
	TXRing *tx_ring;
	RXRing *rx_ring;
	struct net_device_stats stats;
	spinlock_t lock;
	int pmap;
	int is_wan; // mean the net device is WAN side.
	u16 gid;
	u8 mac_type; // VLAN base, or port base;
	u16 vlan_tag;
	struct napi_struct napi;
        struct work_struct reset_task;

	u8 which_port;
	//NICSetting *nic_setting;
	char name[IFNAMSIZ]; // 16 bytes, reference include/linux/netdevice.h IFNAMSIZ
	

	NetDevicePriv *net_device_priv;
	u8 ring_index;

	u32 rx_s_vid[4096]; // record receive s vid (0x9100 ...)
	u32 rx_c_vid[4096]; // record receive c vid (0x8100 ...)
#ifdef CONFIG_CNS3XXX_NAPI
	volatile unsigned long is_qf; // determine rx ring queue full state
#endif
	
#if defined(CONFIG_VLAN_8021Q) || defined(CONFIG_VLAN_8021Q_MODULE)
        struct vlan_group               *vlgrp;
#endif
}CNS3XXXPrivate;




int rx_port_base(struct sk_buff *skb, RXDesc *rx_desc_ptr, const struct CNS3XXXPrivate_ *priv);

int rx_vlan_base(struct sk_buff *skb, RXDesc *rx_desc_ptr, const struct CNS3XXXPrivate_ *priv);

int tx_port_base(TXDesc *tx_desc_ptr, const struct CNS3XXXPrivate_ *priv, struct sk_buff *skb);


int tx_vlan_base(TXDesc *tx_desc_ptr, const struct CNS3XXXPrivate_ *priv, struct sk_buff *skb);
#if defined (CONFIG_CNS3XXX_SPPE)
int fp_port_base(TXDesc *tx_desc_ptr, const struct CNS3XXXPrivate_ *priv, struct sk_buff *skb);
#endif
#endif

