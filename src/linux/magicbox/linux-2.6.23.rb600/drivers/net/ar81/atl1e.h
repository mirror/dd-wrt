/*
 * Copyright(c) 2007 Atheros Corporation. All rights reserved.
 * Copyright(c) 2007 xiong huang <xiong.huang@atheros.com>
 *
 * Derived from Intel e1000 driver
 * Copyright(c) 1999 - 2005 Intel Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * There are a lot of defines in here that are unused and/or have cryptic
 * names.  Please leave them alone, as they're the closest thing we have
 * to a spec from Atheros at present. *ahem* -- CHS
 */

#ifndef _ATL1E_H_
#define _ATL1E_H_

#include "kcompat.h"
#include "at_common.h"
#include "atl1e_hw.h"

#define BAR_0   0
#define BAR_1   1
#define BAR_5   5

#define LINK_POWER_ASPM_L0S_EN   0x1
#define LINK_POWER_ASPM_L1_EN    0x2

/* wrapper around a pointer to a socket buffer,
 * so a DMA handle can be stored along with the buffer */
struct atl1e_buffer {
	struct sk_buff *skb;
	u16 length;
	dma_addr_t dma;
};

struct atl1e_page {
    dma_addr_t				dma;
    u8*						addr;
    dma_addr_t			    WptrPhyAddr;
    u32*					pWptr;
    u32		                Rptr;
};

struct atl1e_ring_header {
    /* pointer to the descriptor ring memory */
    void *desc;
    /* physical adress of the descriptor ring */
    dma_addr_t dma;
    /* length of descriptor ring in bytes */
    unsigned int size;
};

/* board specific private data structure */
          
struct atl1e_adapter {
    /* OS defined structs */
    struct net_device *netdev;
    struct pci_dev *pdev;
    struct net_device_stats net_stats;
    
#ifdef NETIF_F_HW_VLAN_TX
    struct vlan_group *vlgrp;//
#endif
    u32 wol;
    u16 link_speed;
    u16 link_duplex;
    spinlock_t stats_lock;
    spinlock_t tx_lock;
    atomic_t irq_sem;//
    struct work_struct reset_task;//
    struct work_struct link_chg_task;//
    struct timer_list watchdog_timer;
    struct timer_list phy_config_timer;
    unsigned long cfg_phy;

    // All Descriptor memory
    dma_addr_t  	ring_dma;
    void*           ring_vir_addr;
    int             ring_size;
    
    TpdDescr*		tpd_ring;
    dma_addr_t		tpd_ring_dma;
	u16 			tpd_ring_size;	// number of TPDs within the TPD ring
	u16 			tpd_next_use;
	atomic_t		tpd_next_clean;
	struct atl1e_buffer *tx_buffer_info;
	dma_addr_t		tpd_cmb_dma;
	u32*			tpd_cmb;
	
	
	struct atl1e_page	rxf_page[4][2];
	u8				rxf_using[4];
	u16				rxf_nxseq[4];	// next sequence number
	u32				rxf_length;		// bytes length of rxf page
	
	int	num_rx_queues;
	
#ifdef CONFIG_AT_NAPI
	spinlock_t tx_queue_lock;
	struct net_device *polling_netdev;  /* One per active queue */
	
#ifdef CONFIG_AT_MQ 
	struct net_device **cpu_netdev;     /* per-cpu */
	struct call_async_data_struct rx_sched_call_data;
	cpumask_t cpumask;
	int rx_cpu[4];
#endif

#endif
 
    
    /* Interrupt Moderator timer ( 2us resolution) */
    u16 imt;
    /* Interrupt Clear timer (2us resolution) */
    u16 ict;
    
	u32 hw_csum_err;
    
	unsigned long flags;
    /* structs defined in atl1e_hw.h */
    u32 bd_number;     // board number;
    boolean_t pci_using_64;
    boolean_t have_msi;
    struct atl1e_hw hw;

    u32 usr_cmd;
//    u32 regs_buff[AT_REGS_LEN];
    u32 pci_state[16];

    u32* config_space;
};

enum atl1e_state_t {
	__AT_TESTING,
	__AT_RESETTING,
	__AT_DOWN
};

#define AT_WRITE_REG(a, reg, value) ( \
    writel((value), ((a)->hw_addr + reg)))

#define AT_WRITE_FLUSH(a) (\
    readl((a)->hw_addr))

#define AT_READ_REG(a, reg) ( \
    readl((a)->hw_addr + reg ))
    
    
#define AT_WRITE_REGB(a, reg, value) (\
    writeb((value), ((a)->hw_addr + reg)))

#define AT_READ_REGB(a, reg) (\
    readb((a)->hw_addr + reg))

#define AT_WRITE_REGW(a, reg, value) (\
    writew((value), ((a)->hw_addr + reg)))
    
#define AT_READ_REGW(a, reg) (\
    readw((a)->hw_addr + reg))

#define AT_WRITE_REG_ARRAY(a, reg, offset, value) ( \
    writel((value), (((a)->hw_addr + reg) + ((offset) << 2))))

#define AT_READ_REG_ARRAY(a, reg, offset) ( \
    readl(((a)->hw_addr + reg) + ((offset) << 2)))

#endif// _ATL1E_H_

