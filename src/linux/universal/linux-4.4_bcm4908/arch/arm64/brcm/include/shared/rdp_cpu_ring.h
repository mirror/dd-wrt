/*
   
    <:copyright-BRCM:2013:DUAL/GPL:standard
    
       Copyright (c) 2013 Broadcom 
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

/******************************************************************************/
/*                                                                            */
/* File Description:                                                          */
/*                                                                            */
/* This file contains the implementation of the Runner CPU ring interface     */
/*                                                                            */
/******************************************************************************/

#ifndef _RDP_CPU_RING_H_
#define _RDP_CPU_RING_H_

#ifndef RDP_SIM

#if defined(__KERNEL__) || defined(_CFE_)

/*****************************************************************************/
/*                                                                           */
/* Include files                                                             */
/*                                                                           */
/*****************************************************************************/
#include<bcm_pkt_lengths.h>

#ifdef _CFE_
#include "bl_os_wraper.h"
#endif
#include "rdpa_types.h"
#include "rdpa_cpu_basic.h"
#include "rdd.h"
#include <bcm_mm.h>
#include "rdp_cpu_ring_defs.h"

/*****************************************************************************/
/*                                                                           */
/* defines and structures                                                    */
/*                                                                           */
/*****************************************************************************/


#ifdef _CFE_

#include "lib_malloc.h"
#include "cfe_iocb.h"
#define RDP_CPU_RING_MAX_QUEUES    1
#define RDP_WLAN_MAX_QUEUES        0
#define rdpa_cpu_rx_info_t int

#elif defined(__KERNEL__)

#include "rdpa_cpu.h"
#include "bdmf_system.h"
#include "bdmf_shell.h"
#include "bdmf_dev.h"

#define RDP_CPU_RING_MAX_QUEUES        RDPA_CPU_MAX_QUEUES
#define RDP_WLAN_MAX_QUEUES        RDPA_WLAN_MAX_QUEUES

//extern const bdmf_attr_enum_table_t rdpa_cpu_reason_enum_table;

#endif

typedef struct
{
   uint8_t* data_ptr;
   uint16_t packet_size;
   uint16_t flow_id;
   uint16_t reason;
   uint16_t src_bridge_port;
   uint16_t dst_ssid;
   uint16_t wl_metadata; 
   uint16_t ptp_index;
   uint16_t free_index;
   uint8_t  is_rx_offload;
   uint8_t  is_ipsec_upstream;
}
CPU_RX_PARAMS;

#define MAX_BUFS_IN_CACHE 32 
typedef struct RING_DESCTIPTOR RING_DESCTIPTOR;
typedef void *(*databuf_alloc_func)(RING_DESCTIPTOR *pDescriptor);
typedef void (*databuf_free_func)(void *pBuf, uint32_t context, RING_DESCTIPTOR *pDescriptor);
typedef void (*data_dump_func)(uint32_t rindId, rdpa_cpu_rx_info_t *info);
typedef void *(*memory_create_func)(RING_DESCTIPTOR *pDescriptor);
typedef void (*memory_delete_func)(void *buffMem);

struct RING_DESCTIPTOR
{
    uint32_t ring_id;
    uint32_t admin_status;
    uint32_t num_of_entries;
    uint32_t size_of_entry;
    uint32_t packet_size;
    CPU_RX_DESCRIPTOR* head;
    CPU_RX_DESCRIPTOR* base;
    CPU_RX_DESCRIPTOR* end;
    uint32_t buff_cache_cnt;
    uint8_t **buff_cache;
    void *buff_mem_context;
    databuf_alloc_func databuf_alloc;
    databuf_free_func databuf_free;
    data_dump_func data_dump;
    memory_create_func memory_create;
    memory_delete_func memory_delete;
    int stats_received; /* for every queue */
    int stats_dropped;  /* for every queue */
    int dump_enable;
};

typedef struct
{
    databuf_alloc_func databuf_alloc;
    databuf_free_func databuf_free;
    data_dump_func data_dump;
    void *buff_mem_context;
} RING_CB_FUNC;

#ifdef __KERNEL__

int rdp_cpu_ring_get_packet(uint32_t ringId, rdpa_cpu_rx_info_t *rxParams);

void rdp_cpu_reason_stat_cb(uint32_t *stat, rdpa_cpu_reason_index_t *rindex);

void rdp_cpu_rxq_stat_cb(int qid, extern_rxq_stat_t *stat, bdmf_boolean clear);

void rdp_cpu_dump_data_cb(bdmf_index queue, bdmf_boolean enabled);

int rdp_cpu_ring_read_packet_refill(uint32_t ring_id, CPU_RX_PARAMS *rxParams);

#endif

int rdp_cpu_ring_create_ring(uint32_t ring_id, uint8_t ring_type, uint32_t entries,
                             bdmf_phys_addr_t *ring_head, uint32_t packetSize,
                             RING_CB_FUNC *cbFunc);

int rdp_cpu_ring_delete_ring(uint32_t ringId);

void rdp_cpu_ring_free_mem(uint32_t ringId, void *pBuf);

int rdp_cpu_ring_get_queue_size(uint32_t ringId);

int rdp_cpu_ring_get_queued(uint32_t ringId);

int rdp_cpu_ring_flush(uint32_t ringId);

int rdp_cpu_ring_not_empty(uint32_t ringId);

int rdp_cpu_ring_is_full(uint32_t ringId);

/* Callback Functions */

void rdp_packet_dump(uint32_t ringId, rdpa_cpu_rx_info_t *info);

/* BPM (or CFE)*/

void *rdp_databuf_alloc(RING_DESCTIPTOR *pDescriptor);

void rdp_databuf_free(void *pBuf, uint32_t context, RING_DESCTIPTOR *pDescriptor);

/* Kmem_Cache */

void *rdp_databuf_alloc_cache(RING_DESCTIPTOR *pDescriptor);

void rdp_databuf_free_cache(void *pBuf, uint32_t context, RING_DESCTIPTOR *pDescriptor);

#endif /* if defined(__KERNEL__) || defined(_CFE_) */

#else
#include "rdp_cpu_ring_sim.h"
#define RDP_CPU_RING_MAX_QUEUES        RDPA_CPU_MAX_QUEUES
#define RDP_WLAN_MAX_QUEUES        RDPA_WLAN_MAX_QUEUES
#endif /* RDP_SIM */

/*array of possible rings private data*/
#define RING_ID_NUM_OF (RDP_CPU_RING_MAX_QUEUES + RDP_WLAN_MAX_QUEUES)
#define D_NUM_OF_RING_DESCRIPTORS RING_ID_NUM_OF

#endif /* _RDP_CPU_RING_H_ */
