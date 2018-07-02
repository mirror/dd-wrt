/*
   Copyright (c) 2014 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2014:DUAL/GPL:standard
    
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

#ifndef _RDD_DHD_HELPER_H
#define _RDD_DHD_HELPER_H

#include "rdd.h"
#include "rdd_data_structures_auto.h"
#include "dhd_defs.h"
#include "rdpa_types.h"
#include "rdpa_dhd_helper_basic.h"
#include "bdmf_shell.h"
#if defined(DSL_63138) || defined(DSL_63148) || defined(WL4908)
#include "rdd_cpu.h"
#endif

#define FLOW_RING_RX_COMPLETE_IDX 4

typedef struct {
    uint8_t* ring_ptr;
    uint32_t ring_size;
    uint8_t* ring_base;
    uint8_t* ring_end;
} rdd_dhd_complete_ring_descriptor_t;

typedef struct {
    uint16_t wr_idx;       // locally maintained wr_idx
    uint16_t rd_idx;       // locally maintained rd_idx
    uint8_t *ring_base;
    uint16_t *wr_idx_addr; // address for shared wr_idx (between dongle and runner)
    uint16_t *rd_idx_addr; // address for shared rd_idx (between dongle and runner)
    uint32_t *mb_int_mapped;  // IO MAP address for mailbox to dongle
} rdd_dhd_rx_post_ring_t;

extern rdd_dhd_complete_ring_descriptor_t g_dhd_complete_ring_desc[];

#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908)
extern bdmf_fastlock int_lock_irq;
extern uint32_t g_cpu_tx_skb_rdd_free_indexes_release_ptr;
extern uint32_t g_cpu_tx_skb_free_indexes_counter;
extern uint32_t g_cpu_tx_released_skb_counter;
extern uint32_t *g_cpu_tx_skb_pointers_reference_array;
#endif
extern bdmf_boolean  tx_complete_host_send2dhd_flag;

/* Init Flow Rings array, including feeding the RX_post array with BPMs */
int rdd_dhd_hlp_cfg(uint32_t radio_idx, rdpa_dhd_init_cfg_t *init_cfg, int enable);
bdmf_boolean rdd_dhd_helper_flow_ring_is_enabled(uint32_t flow_ring_idx);
int rdd_dhd_rx_post_init(uint32_t radio_idx, rdpa_dhd_init_cfg_t *init_cfg);
int rdd_dhd_rx_post_uninit(uint32_t radio_idx, rdpa_dhd_init_cfg_t *init_cfg);

void rdd_dhd_helper_flow_ring_flush(uint32_t radio_idx, uint32_t flow_ring_idx);
void rdd_dhd_helper_flow_ring_disable(uint32_t radio_idx, uint32_t flow_ring_idx);
void rdd_dhd_helper_shell_cmds_init(bdmfmon_handle_t rdd_dir);
void rdd_dhd_helper_wakeup_information_get(rdpa_dhd_wakeup_info_t *wakeup_info);
int rdd_dhd_helper_dhd_complete_ring_create(uint32_t radio_idx, uint32_t ring_size);
int rdd_dhd_helper_dhd_complete_ring_destroy(uint32_t radio_idx, uint32_t ring_size);
uint16_t rdd_dhd_helper_ssid_tx_dropped_packets_get(uint32_t radio_idx, uint32_t ssid);
#if defined(DSL_63138) || defined(DSL_63148) ||  defined(WL4908)
extern bdmf_sysb rdpa_cpu_return_free_index(uint16_t free_index);
#endif   

static inline void rdd_dhd_helper_wakeup(uint32_t radio_idx, bdmf_boolean is_tx_complete)
{
    RUNNER_REGS_CFG_CPU_WAKEUP cpu_wakeup_reg = {};
#ifdef CM3390
    uint32_t dhd_tx_complete_thread = DHD0_TX_COMPLETE_THREAD_NUMBER + radio_idx;
#else
    uint32_t dhd_tx_complete_thread = DHD_TX_COMPLETE_FAST_A_THREAD_NUMBER + radio_idx;
#endif
    uint32_t dhd_rx_complete_thread = DHD_RX_THREAD_NUMBER + radio_idx;

    /* send asynchronous wakeup command to the CPU-TX thread in the Runner */
    cpu_wakeup_reg.req_trgt = (is_tx_complete ? dhd_tx_complete_thread : dhd_rx_complete_thread) / 32;
    cpu_wakeup_reg.thread_num = (is_tx_complete ? dhd_tx_complete_thread : dhd_rx_complete_thread) % 32;
    cpu_wakeup_reg.urgent_req = 0;
   
    if (is_tx_complete)
        RUNNER_REGS_0_CFG_CPU_WAKEUP_WRITE(cpu_wakeup_reg);
    else
        RUNNER_REGS_1_CFG_CPU_WAKEUP_WRITE(cpu_wakeup_reg);
}


static inline int rdd_dhd_helper_dhd_complete_message_get(rdpa_dhd_complete_data_t *dhd_complete_info)
{
    rdd_dhd_complete_ring_descriptor_t *pdesc = &g_dhd_complete_ring_desc[dhd_complete_info->radio_idx];
    uint32_t                              request_id_buffer_type;
    int                                   rc = 0;
    void                                  *txp = 0; 
    uint8_t                               buf_type = 0;                            
#if !defined(DSL_63138) && !defined(DSL_63148)  && !defined(WL4908)
    unsigned long                         flags;
    uint16_t                              index_to_free = 0;
#endif    
    

    RDD_DHD_COMPLETE_RING_ENTRY_REQUEST_ID_READ(request_id_buffer_type, pdesc->ring_ptr);

    if (RDD_DHD_COMPLETE_RING_ENTRY_OWNERSHIP_L_READ(request_id_buffer_type) != DHD_COMPLETE_OWNERSHIP_RUNNER)
    {                
        buf_type = RDD_DHD_COMPLETE_RING_ENTRY_BUFFER_TYPE_L_READ(request_id_buffer_type);
        
        if (buf_type == DHD_TX_POST_HOST_BUFFER_VALUE)
        {
            /* It is a buffer from offloaded ring - release an index and pass the ptr to DHD */            
            
#if (defined(DSL_63138) || defined(DSL_63148)  || defined(WL4908))
#ifndef BDMF_SYSTEM_SIM
            txp = (void *)rdd_cpu_return_free_index(request_id_buffer_type & LILAC_RDD_CPU_TX_SKB_INDEX_MASK);
            if (!tx_complete_host_send2dhd_flag)
            {
              bdmf_sysb_free((bdmf_sysb)txp);                                
              request_id_buffer_type = 0;
              txp = 0;
              rc = BDMF_ERR_ALREADY;
            }
#endif
#else            
            f_rdd_lock_irq ( &int_lock_irq, &flags );
              
            index_to_free = request_id_buffer_type & LILAC_RDD_CPU_TX_SKB_INDEX_MASK;
    
            /* free index in SW ring*/            
            g_rdd_free_skb_indexes_fifo_table[ g_cpu_tx_skb_rdd_free_indexes_release_ptr ] = index_to_free;            
    
            /* Debug feature - pointer validity check */
            if (likely(g_cpu_tx_skb_pointers_reference_array[index_to_free] != ( uint32_t )( -1 )) )
            {
                /* set ptr to SKB buffer ptr + indication of HOST_BUFFER value */                
                txp = (void *)(g_cpu_tx_skb_pointers_reference_array[index_to_free]);
                g_cpu_tx_skb_pointers_reference_array[index_to_free] = ( uint32_t )( -1 );
                                                                
                if (!tx_complete_host_send2dhd_flag)
                {
                    bdmf_sysb_free ( ( bdmf_sysb )txp );                                    
                    txp = 0;
                    rc = BDMF_ERR_ALREADY;
                }                                                
            }
            else
            {
                txp = 0;
                bdmf_trace("ERROR: rdd dhd helper: release of not allocated SKB: idx=%d, ptr=%d\n", index_to_free,
                    g_cpu_tx_skb_rdd_free_indexes_release_ptr);  
            }
        
            /* increment counters */
            g_cpu_tx_released_skb_counter++;
            g_cpu_tx_skb_free_indexes_counter++;
            g_cpu_tx_skb_rdd_free_indexes_release_ptr++;
            g_cpu_tx_skb_rdd_free_indexes_release_ptr &= (LILAC_RDD_CPU_TX_SKB_LIMIT_DEFAULT - 1);            
                
            f_rdd_unlock_irq ( &int_lock_irq, flags );
#endif            
        }
        else
        {
          txp = 0;
        }
        
        /* Set the return parameters. */
        dhd_complete_info->request_id = request_id_buffer_type;
        dhd_complete_info->buf_type = buf_type;
        dhd_complete_info->txp = txp;
        RDD_DHD_COMPLETE_RING_ENTRY_STATUS_READ(dhd_complete_info->status, pdesc->ring_ptr);
        RDD_DHD_COMPLETE_RING_ENTRY_FLOW_RING_ID_READ(dhd_complete_info->flow_ring_id, pdesc->ring_ptr);

        /* Set the ring element to be owned by Runner */
        RDD_DHD_COMPLETE_RING_ENTRY_REQUEST_ID_WRITE(0, pdesc->ring_ptr);
        RDD_DHD_COMPLETE_RING_ENTRY_OWNERSHIP_WRITE(DHD_COMPLETE_OWNERSHIP_RUNNER, pdesc->ring_ptr);

        /* Update the ring pointer to the next element. */
        if (pdesc->ring_ptr == pdesc->ring_end)
            pdesc->ring_ptr = pdesc->ring_base;
        else
            pdesc->ring_ptr += sizeof(RDD_DHD_COMPLETE_RING_ENTRY_DTS);
    }
    else
        rc = BDMF_ERR_ALREADY;

    return rc;
}

/* Definitions taken from DHD driver (cannot include it's header) */
typedef union {
    struct {
        uint32_t low;
        uint32_t high;
    };
    struct {
        uint32_t low_addr;
        uint32_t high_addr;
    };
    uint64_t u64;
} addr64_t;

typedef struct 
{
    /* message type */
    uint8_t msg_type;
    /* interface index this is valid for */
    uint8_t if_id;
    /* flags */
    uint8_t flags;
    /* alignment */
    uint8_t reserved;
    /* packet Identifier for the associated host buffer */
    uint32_t request_id;
} cmn_msg_hdr_t;

typedef struct 
{
    /* common message header */
    cmn_msg_hdr_t cmn_hdr;
    /* provided meta data buffer len */
    uint16_t metadata_buf_len;
    /* provided data buffer len to receive data */
    uint16_t data_buf_len;
    /* alignment to make the host buffers start on 8 byte boundary */
    uint32_t rsvd;
    /* provided meta data buffer */
    addr64_t metadata_buf_addr;
    /* provided data buffer to receive data */
    addr64_t data_buf_addr;
} host_rxbuf_post_t;

#endif /* _RDD_DHD_HELPER_H */

