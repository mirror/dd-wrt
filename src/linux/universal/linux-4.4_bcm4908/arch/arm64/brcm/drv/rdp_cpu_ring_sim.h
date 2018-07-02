/*
    <:copyright-BRCM:2013-2016:DUAL/GPL:standard
    
       Copyright (c) 2013-2016 Broadcom 
       All Rights Reserved
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License, version 2, as published by
    the Free Software Foundation (the "GPL").
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    
    A copy of the GPL is available at http://www.broadcom.com/licenses/GPLv2.php, or by
    writing to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
    
    :>
*/

#ifndef _RDP_CPU_RING_SIM_H
#define _RDP_CPU_RING_SIM_H

#include "rdpa_types.h"
#include "rdd.h"
#include "rdd_defs.h"
#ifdef LEGACY_RDP
#include "rdd_legacy_conv.h"
#endif

#define BCM_PKTBUF_SIZE 1532

typedef enum
{
	sysb_type_skb,
	sysb_type_fkb,
	sysb_type_raw,
} cpu_ring_sysb_type;

typedef enum
{
	type_cpu_rx,
	type_pci_tx
} cpu_ring_type;

typedef struct
{
	uint8_t *sysb_ptr;
	uint8_t *data_ptr;
	uint32_t packet_size;
#ifdef XRDP
	rdd_vport_id_t src_bridge_port;
    uint8_t data_offset;
#else
	rdd_bridge_port_t src_bridge_port;
#endif
	uint32_t flow_id;
	rdpa_cpu_reason reason;
	uint16_t dst_ssid;
    uint16_t wl_metadata;
    uint16_t ptp_index;    
    uint16_t free_index;
    uint8_t  is_rx_offload;
    uint8_t  is_ipsec_upstream;
    uint8_t  is_ucast;
    uint8_t  is_exception;
} CPU_RX_PARAMS;

typedef struct
{
    void *buff_mem_context;
} RING_CB_FUNC;

static inline int rdp_cpu_ring_create_ring( uint32_t ringId, uint8_t type,
        uint32_t entries,  void *ic_cfg_p, uint32_t packetSize, RING_CB_FUNC *cbFunc )
{
    return 0;
}

static inline int rdp_cpu_ring_delete_ring( uint32_t ringId )
{
	return 0;
}

static inline int rdp_cpu_ring_read_packet_refill(
		uint32_t ringId, CPU_RX_PARAMS* rxParams)
{
	return 0;
}

#if defined(CONFIG_BCM963138) || defined(_BCM963138_) || defined(CONFIG_BCM963148) || defined(_BCM963148_)
typedef struct
{
    uint32_t packet_length;
    uint32_t source_port;
    uint8_t *data_ptr;
    rdpa_cpu_reason reason;
} CPU_RX_DESCRIPTOR;

static inline int rdp_cpu_ring_read_packet_refill2(uint32_t ring_id, CPU_RX_DESCRIPTOR* rxDesc)
{
    return 0;
}
#endif

static inline int rdp_cpu_ring_bulk_skb_get(
   uint32_t ring_id, unsigned int budget, void ** rx_pkts)
{
    return 0;
}

static inline int rdp_cpu_ring_bulk_fkb_get(
   uint32_t ring_id, unsigned int budget, void ** rx_pkts)
{
    return 0;
}

static inline int rdp_cpu_ring_read_packet_copy( uint32_t ringId,CPU_RX_PARAMS* rxParams)
{
	return 0;
}

static inline int	rdp_cpu_ring_get_queue_size( uint32_t ringId)
{
	return 0;
}

static inline int	rdp_cpu_ring_get_queued( uint32_t ringId)
{
	return 0;
}

static inline int rdp_cpu_ring_flush(uint32_t ringId)
{
   return 0;
}
static inline int rdp_cpu_ring_not_empty(uint32_t ringId)
{
	return 0;
}
static inline int rdp_cpu_ring_is_full(uint32_t ringId)
{
	return 0;
}
#endif
