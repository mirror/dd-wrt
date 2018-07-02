/*
   Copyright (c) 2013 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2013:DUAL/GPL:standard
    
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

#ifdef CONFIG_DHD_RUNNER
#ifdef USE_BDMF_SHELL

#include "bdmf_shell.h"
#include "rdd.h"
#include "dhd_defs.h"
#include "rdpa_dhd_helper_basic.h"

#define MAKE_BDMF_SHELL_CMD_NOPARM(dir, cmd, help, cb) \
    bdmfmon_cmd_add(dir, cmd, cb, help, BDMF_ACCESS_ADMIN, NULL, NULL)

#define MAKE_BDMF_SHELL_CMD(dir, cmd, help, cb, parms...)   \
{                                                           \
    static bdmfmon_cmd_parm_t cmd_parms[]={                 \
        parms,                                              \
        BDMFMON_PARM_LIST_TERMINATOR                        \
    };                                                      \
    bdmfmon_cmd_add(dir, cmd, cb, help, BDMF_ACCESS_ADMIN, NULL, cmd_parms); \
}

static int _rdd_print_dhd_tx_post_desc(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_DTS *radio_instance_table_ptr;
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_DTS *radio_instance_entry_ptr;
    RDD_DHD_TX_POST_DESCRIPTOR_DTS *desc_ptr;
    RDD_DHD_TX_POST_DESCRIPTOR_DTS desc;
#if defined DSL_63138 || defined DSL_63148 || defined WL4908 || defined CM3390
    uint8_t *base_ptr;
#else
    uint32_t base_ptr;
#endif
    uint32_t radio_idx, ring_id, desc_num;
    rdpa_dhd_flring_cache_t *entry_ptr;

    radio_idx = (uint32_t)parm[0].value.unumber;
    ring_id = (uint32_t)parm[1].value.unumber;
    desc_num = (uint32_t)parm[2].value.unumber;

    radio_instance_table_ptr = RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_PTR();
    radio_instance_entry_ptr = &radio_instance_table_ptr->entry[radio_idx];

#if defined DSL_63138 || defined DSL_63148 || defined WL4908 || defined CM3390
    base_ptr = (uint8_t *)(uintptr_t)(((uint64_t)radio_instance_entry_ptr->tx_post_mgmt_fr_base_ptr_rdd_only_high << 32) + radio_instance_entry_ptr->tx_post_mgmt_fr_base_ptr_rdd_only_low);
    entry_ptr = (rdpa_dhd_flring_cache_t *)base_ptr + ring_id;
    desc_ptr = (RDD_DHD_TX_POST_DESCRIPTOR_DTS *)PHYS_TO_VIRT((uintptr_t)(((uint64_t)__swap4bytes(entry_ptr->base_addr_high) << 32) + __swap4bytes(entry_ptr->base_addr_low))) + desc_num;
    bdmf_session_print(session, "flow ring %d cache address = 0x%8.8lx, descriptor %d address = 0x%8.8lx\n", ring_id, (unsigned long)entry_ptr,
        desc_num, (unsigned long)desc_ptr);
#else
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_TX_POST_MGMT_FR_BASE_PTR_READ(base_ptr, radio_instance_entry_ptr);

    entry_ptr = (rdpa_dhd_flring_cache_t *)(0xa0000000 | base_ptr) + ring_id;
    desc_ptr = (RDD_DHD_TX_POST_DESCRIPTOR_DTS *)(0xa0000000 | entry_ptr->base_addr_low) + desc_num;
#endif

    memcpy(&desc, desc_ptr, sizeof(RDD_DHD_TX_POST_DESCRIPTOR_DTS));

    bdmf_session_print(session, "TX POST descriptor\n");
    bdmf_session_print(session, "msg_type                 = 0x%4.4x\n", desc.msg_type);
    bdmf_session_print(session, "if_id                    = 0x%4.4x\n", desc.if_id);
    bdmf_session_print(session, "common_hdr_flags         = 0x%4.4x\n", desc.common_hdr_flags);
    bdmf_session_print(session, "epoch                    = 0x%4.4x\n", desc.epoch);
    bdmf_session_print(session, "request_id               = 0x%8.8x\n", desc.request_id);
    bdmf_session_print(session, "tx_eth_hdr_0             = 0x%8.8x\n", desc.tx_eth_hdr_0);
    bdmf_session_print(session, "tx_eth_hdr_1             = 0x%8.8x\n", desc.tx_eth_hdr_1);
    bdmf_session_print(session, "tx_eth_hdr_2             = 0x%8.8x\n", desc.tx_eth_hdr_2);
    bdmf_session_print(session, "tx_eth_hdr_3             = 0x%4.4x\n", desc.tx_eth_hdr_3);
    bdmf_session_print(session, "flags                    = 0x%4.4x\n", desc.flags);
    bdmf_session_print(session, "seg_cnt                  = 0x%4.4x\n", desc.seg_cnt);
    bdmf_session_print(session, "metadata_buf_addr_low	= 0x%8.8x\n", desc.metadata_buf_addr_low);
    bdmf_session_print(session, "metadata_buf_addr_hi 	= 0x%8.8x\n", desc.metadata_buf_addr_hi);
    bdmf_session_print(session, "data_buf_addr_low    	= 0x%8.8x\n", cpu_to_le32(desc.data_buf_addr_low));
    bdmf_session_print(session, "data_buf_addr_hi         = 0x%8.8x\n", desc.data_buf_addr_hi);
    bdmf_session_print(session, "meta_buf_len             = 0x%4.4x\n", desc.meta_buf_len);
    bdmf_session_print(session, "data_len                 = 0x%4.4x\n", cpu_to_le16(desc.data_len));

    return 0;
}

static int _rdd_print_dhd_tx_complete_desc(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_DTS *radio_instance_table_ptr;
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_DTS *radio_instance_entry_ptr;
    RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS *desc_ptr;
    RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS desc;
    uint32_t desc_num, radio_idx, base_ptr;

    radio_idx = (uint32_t)parm[0].value.unumber;
    desc_num = (uint32_t)parm[1].value.unumber;

    radio_instance_table_ptr = RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_PTR();
    radio_instance_entry_ptr = &radio_instance_table_ptr->entry[radio_idx];

    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_TX_COMPLETE_FR_BASE_PTR_READ(base_ptr, radio_instance_entry_ptr);

#if !defined(FIRMWARE_INIT)
#if (defined DSL_63138 || defined DSL_63148 || defined WL4908)
    desc_ptr = (RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS *)PHYS_TO_CACHED((uint32_t)(base_ptr + desc_num * sizeof(RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS)));
#elif defined CM3390
    desc_ptr = (RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS *)phys_to_virt((phys_addr_t)((uint32_t)(base_ptr + desc_num * sizeof(RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS))));
#else
    desc_ptr = (RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS *)(0xa0000000 | (uint32_t)(base_ptr + desc_num * sizeof(RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS)));
#endif
#else  /* FIRMWARE_INIT */
    desc_ptr = (RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS *)(uint32_t)(base_ptr + desc_num * sizeof(RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS));
#endif

#if (defined DSL_63138 || defined DSL_63148 || defined CM3390)
    bdmf_session_print(session, "descriptor %d address = 0x%8.8lx\n", desc_num, (unsigned long)desc_ptr);
#endif

    memcpy(&desc, desc_ptr, sizeof(RDD_DHD_TX_COMPLETE_DESCRIPTOR_DTS));

    bdmf_session_print(session, "TX COMPLETE descriptor\n");
    bdmf_session_print(session, "msg_type                 = 0x%4.4x\n", desc.msg_type);
    bdmf_session_print(session, "if_id                    = 0x%4.4x\n", desc.if_id);
    bdmf_session_print(session, "common_hdr_flags         = 0x%4.4x\n", desc.common_hdr_flags);
    bdmf_session_print(session, "epoch                    = 0x%4.4x\n", desc.epoch);
    bdmf_session_print(session, "request_id               = 0x%8.8x\n", desc.request_id);
    bdmf_session_print(session, "status                   = 0x%4.4x\n", desc.status);
    bdmf_session_print(session, "flow_ring_id             = 0x%4.4x\n", desc.flow_ring_id);
    bdmf_session_print(session, "dma_done_mark 	        = 0x%8.8x\n", desc.dma_done_mark);

    return 0;
}

static int _rdd_print_dhd_rx_complete_desc(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_DTS *radio_instance_table_ptr;
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_DTS *radio_instance_entry_ptr;
    RDD_DHD_RX_COMPLETE_DESCRIPTOR_DTS *desc_ptr;
    RDD_DHD_RX_COMPLETE_DESCRIPTOR_DTS desc;
    uint32_t desc_num, radio_idx, base_ptr;

    radio_idx = (uint32_t)parm[0].value.unumber;
    desc_num = (uint32_t)parm[1].value.unumber;

    radio_instance_table_ptr = RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_PTR();
    radio_instance_entry_ptr = &radio_instance_table_ptr->entry[radio_idx];

    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_RX_COMPLETE_FR_BASE_PTR_READ(base_ptr, radio_instance_entry_ptr);

#if !defined(FIRMWARE_INIT)
#if (defined DSL_63138 || defined DSL_63148 || defined WL4908)
    desc_ptr = (RDD_DHD_RX_COMPLETE_DESCRIPTOR_DTS *)PHYS_TO_CACHED((uint32_t)(base_ptr + desc_num * sizeof(RDD_DHD_RX_COMPLETE_DESCRIPTOR_DTS)));
#elif defined CM3390
    desc_ptr = (RDD_DHD_RX_COMPLETE_DESCRIPTOR_DTS *)phys_to_virt((phys_addr_t)((uint32_t)(base_ptr + desc_num * sizeof(RDD_DHD_RX_COMPLETE_DESCRIPTOR_DTS))));
#else
    desc_ptr = (RDD_DHD_RX_COMPLETE_DESCRIPTOR_DTS *)(0xa0000000 | (uint32_t)(base_ptr + desc_num * sizeof(RDD_DHD_RX_COMPLETE_DESCRIPTOR_DTS)));
#endif
#else  /* FIRMWARE_INIT */
    desc_ptr = (RDD_DHD_RX_COMPLETE_DESCRIPTOR_DTS *)(uint32_t)(base_ptr + desc_num * sizeof(RDD_DHD_RX_COMPLETE_DESCRIPTOR_DTS));
#endif

#if (defined DSL_63138 || defined DSL_63148 || defined CM3390)
    bdmf_session_print(session, "descriptor %d address = 0x%8.8lx\n", desc_num, (unsigned long)desc_ptr);
#endif

    memcpy(&desc, desc_ptr, sizeof(RDD_DHD_RX_COMPLETE_DESCRIPTOR_DTS));

    bdmf_session_print(session, "RX COMPLETE descriptor\n");
    bdmf_session_print(session, "msg_type                 = 0x%4.4x\n", desc.msg_type);
    bdmf_session_print(session, "if_id                    = 0x%4.4x\n", desc.if_id);
    bdmf_session_print(session, "common_hdr_flags         = 0x%4.4x\n", desc.common_hdr_flags);
    bdmf_session_print(session, "epoch                    = 0x%4.4x\n", desc.epoch);
    bdmf_session_print(session, "request_id               = 0x%8.8x\n", desc.request_id);
    bdmf_session_print(session, "compl_msg_hdr_status     = 0x%4.4x\n", desc.compl_msg_hdr_status);
    bdmf_session_print(session, "flow_ring_id             = 0x%4.4x\n", desc.flow_ring_id);
    bdmf_session_print(session, "data_len                 = 0x%4.4x\n", cpu_to_le16(desc.data_len));
    bdmf_session_print(session, "data_offset              = 0x%4.4x\n", cpu_to_le16(desc.data_offset));
    bdmf_session_print(session, "flags                    = 0x%4.4x\n", desc.flags);
    bdmf_session_print(session, "rx_status_0              = 0x%8.8x\n", desc.rx_status_0);
    bdmf_session_print(session, "rx_status_1	            = 0x%8.8x\n", desc.rx_status_1);
    bdmf_session_print(session, "dma_done_mark 	        = 0x%8.8x\n", desc.dma_done_mark);

    return 0;
}

static int _rdd_print_dhd_rx_post_desc(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_DTS *radio_instance_table_ptr;
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_DTS *radio_instance_entry_ptr;
    RDD_DHD_RX_POST_DESCRIPTOR_DTS *desc_ptr;
    RDD_DHD_RX_POST_DESCRIPTOR_DTS desc;
    uint32_t desc_num, radio_idx, base_ptr;

    radio_idx = (uint32_t)parm[0].value.unumber;
    desc_num = (uint32_t)parm[1].value.unumber;

    radio_instance_table_ptr = RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_PTR();
    radio_instance_entry_ptr = &radio_instance_table_ptr->entry[radio_idx];

    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_RX_POST_FR_BASE_PTR_READ(base_ptr, radio_instance_entry_ptr);

#if !defined(FIRMWARE_INIT)
#if (defined DSL_63138 || defined DSL_63148 || defined WL4908)
    desc_ptr = (RDD_DHD_RX_POST_DESCRIPTOR_DTS *)PHYS_TO_CACHED((uint32_t)(base_ptr + desc_num * sizeof(RDD_DHD_RX_POST_DESCRIPTOR_DTS)));
#elif defined CM3390
    desc_ptr = (RDD_DHD_RX_POST_DESCRIPTOR_DTS *)phys_to_virt((phys_addr_t)((uint32_t)(base_ptr + desc_num * sizeof(RDD_DHD_RX_POST_DESCRIPTOR_DTS))));
#else
    desc_ptr = (RDD_DHD_RX_POST_DESCRIPTOR_DTS*)(0xa0000000 | (uint32_t)(base_ptr + desc_num * sizeof(RDD_DHD_RX_POST_DESCRIPTOR_DTS)));
#endif
#else  /* FIRMWARE_INIT */
    desc_ptr = (RDD_DHD_RX_POST_DESCRIPTOR_DTS *)(uint32_t)(base_ptr + desc_num * sizeof(RDD_DHD_RX_POST_DESCRIPTOR_DTS));
#endif

#if (defined DSL_63138 || defined DSL_63148 || defined CM3390)
    bdmf_session_print(session, "descriptor %d address = 0x%8.8lx\n", desc_num, (unsigned long)desc_ptr);
#endif

    memcpy(&desc, desc_ptr, sizeof(RDD_DHD_RX_POST_DESCRIPTOR_DTS));

    bdmf_session_print(session, "RX POST descriptor\n");
    bdmf_session_print(session, "msg_type                 = 0x%4.4x\n", desc.msg_type);
    bdmf_session_print(session, "if_id                    = 0x%4.4x\n", desc.if_id);
    bdmf_session_print(session, "common_hdr_flags         = 0x%4.4x\n", desc.common_hdr_flags);
    bdmf_session_print(session, "epoch                    = 0x%4.4x\n", desc.epoch);
    bdmf_session_print(session, "request_id               = 0x%8.8x\n", desc.request_id);
    bdmf_session_print(session, "meta_buf_len             = 0x%4.4x\n", desc.meta_buf_len);
    bdmf_session_print(session, "data_len                 = 0x%4.4x\n", cpu_to_le16(desc.data_len));
    bdmf_session_print(session, "metadata_buf_addr_low    = 0x%8.8x\n", desc.metadata_buf_addr_low);
    bdmf_session_print(session, "metadata_buf_addr_hi     = 0x%8.8x\n", desc.metadata_buf_addr_hi);
    bdmf_session_print(session, "data_buf_addr_low        = 0x%8.8x\n", cpu_to_le32(desc.data_buf_addr_low));
    bdmf_session_print(session, "data_buf_addr_hi	        = 0x%8.8x\n", desc.data_buf_addr_hi);

    return 0;
}

static int _rdd_print_dhd_drop_counters(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    uint32_t i, j;
    uint16_t counter, group;

    bdmf_session_print(session, "SSID drop counters\n");

#if defined DSL_63138 || defined DSL_63148 || defined CM3390 || defined WL4908
    for (j = 0; j < 3; j++)
#else
    for (j = 0; j < 2; j++)
#endif
    {
        bdmf_session_print(session, "Radio %d\n", j);
        group = DHD_SSID_DROP_PACKET_GROUP + j;
        for (i = 0; i < 16; i++)
        {
#if defined DSL_63138 || defined DSL_63148 || defined WL4908 || defined CM3390
            rdd_2_bytes_counter_get(group, i, &counter);
#else
            rdd_2_bytes_counter_get(group, i, 0, &counter);
#endif
            bdmf_session_print(session, "\tSSID%d = %d\n", i, counter);
        }
    }
#if defined DSL_63138 || defined DSL_63148 || defined WL4908 || defined CM3390
    rdd_2_bytes_counter_get(UPSTREAM_VARIOUS_PACKETS_GROUP, DHD_MALLOC_FAILED_OFFSET, &counter);
#else
    rdd_2_bytes_counter_get(UPSTREAM_VARIOUS_PACKETS_GROUP, DHD_MALLOC_FAILED_OFFSET, 0, &counter);
#endif
    bdmf_session_print(session, "Malloc failed = %d\n", counter);

#if defined DSL_63138 || defined DSL_63148 || defined WL4908 || defined CM3390
    rdd_2_bytes_counter_get(UPSTREAM_VARIOUS_PACKETS_GROUP, DHD_IH_CONGESTION_OFFSET, &counter);
#else
    rdd_2_bytes_counter_get(UPSTREAM_VARIOUS_PACKETS_GROUP, DHD_IH_CONGESTION_OFFSET, 0, &counter);
#endif
    bdmf_session_print(session, "IH congestion = %d\n", counter);

    return 0;
}

static int _rdd_print_dhd_flow_rings_cache(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    int i;
    RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY_DTS *cache_lkp_ptr, cache_lkp;
    RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY_DTS *cache_ctx_ptr, ctx_entry;

    bdmf_session_print(session, "FlowRings Cache\n");

    cache_lkp_ptr = (RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY_DTS *)RDD_US_DHD_FLOW_RING_CACHE_LKP_TABLE_PTR();
    cache_ctx_ptr = (RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY_DTS *)RDD_DHD_FLOW_RING_CACHE_CTX_TABLE_PTR();

    bdmf_session_print(session, "Ring ID  Radio  Inv  Base         Size   Flags     RD_Idx  WR_Idx\n");
    bdmf_session_print(session, "=================================================================\n");

    for (i = 0; i < 16; i++, cache_lkp_ptr++, cache_ctx_ptr++)
    {
        /* Read Flowring cache entry */
        RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY_FLOW_RING_ID_READ(cache_lkp.flow_ring_id, cache_lkp_ptr);
        RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY_RADIO_IDX_READ(cache_lkp.radio_idx, cache_lkp_ptr);
        RDD_DHD_FLOW_RING_CACHE_LKP_ENTRY_INVALID_READ(cache_lkp.invalid, cache_lkp_ptr);
        bdmf_session_print(session, "%7d  %5d  %3d  ", cache_lkp.flow_ring_id, cache_lkp.radio_idx, cache_lkp.invalid);

        /* Read FlowRing context entry */
        RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY_FLOW_RING_BASE_LOW_READ(ctx_entry.flow_ring_base_low, cache_ctx_ptr);
        RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY_SIZE_READ(ctx_entry.size, cache_ctx_ptr);
        RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY_FLAGS_READ(ctx_entry.flags, cache_ctx_ptr);
        RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY_RD_IDX_READ(ctx_entry.rd_idx, cache_ctx_ptr);
        RDD_DHD_FLOW_RING_CACHE_CTX_ENTRY_WR_IDX_READ(ctx_entry.wr_idx, cache_ctx_ptr);
        bdmf_session_print(session, "0x%8x  %5d 0x%8x  %5d  %5d\n",
            ctx_entry.flow_ring_base_low, ctx_entry.size, ctx_entry.flags, ctx_entry.rd_idx, ctx_entry.wr_idx);
    }

    return 0;
}

#if defined CM3390
static int _rdd_print_dhd_d2r_wr_idx(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_DTS *radio_instance_table_ptr;
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_DTS *radio_instance_entry_ptr;
    uint16_t *ring_write_idx_ptr;
    uint16_t ring_write_idx;
    uint32_t radio_idx, ring_id, base_ptr;

    radio_idx = (uint32_t)parm[0].value.unumber;
    ring_id = (uint32_t)parm[1].value.unumber;

    radio_instance_table_ptr = RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_PTR();
    radio_instance_entry_ptr = &radio_instance_table_ptr->entry[radio_idx];

    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_D2R_WR_FR_DESC_BASE_PTR_READ(base_ptr, radio_instance_entry_ptr);

#ifndef FIRMWARE_INIT
    ring_write_idx_ptr = (uint16_t *)phys_to_virt((phys_addr_t)((uint32_t)(base_ptr + ring_id * sizeof(uint16_t))));
#else
    ring_write_idx_ptr = (uint16_t *)((uint32_t)(base_ptr + ring_id * sizeof(uint16_t)));
#endif

    memcpy(&ring_write_idx, ring_write_idx_ptr, sizeof(uint16_t));

    bdmf_session_print(session, "D2R Write Index ring %d: %d\n", ring_id, ring_write_idx);
    bdmf_session_print(session, "ring 0: tx complete, ring 1: rx complete, 2-127: not used\n"); 

    return 0;
}

static int _rdd_print_dhd_d2r_rd_idx(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_DTS *radio_instance_table_ptr;
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_DTS *radio_instance_entry_ptr;
    uint16_t *ring_read_idx_ptr;
    uint16_t ring_read_idx;
    uint32_t radio_idx, ring_id, base_ptr;

    radio_idx = (uint32_t)parm[0].value.unumber;
    ring_id = (uint32_t)parm[1].value.unumber;

    radio_instance_table_ptr = RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_PTR();
    radio_instance_entry_ptr = &radio_instance_table_ptr->entry[radio_idx];

    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_D2R_RD_FR_DESC_BASE_PTR_READ(base_ptr, radio_instance_entry_ptr);

#ifndef FIRMWARE_INIT
    ring_read_idx_ptr = (uint16_t *)phys_to_virt((phys_addr_t)((uint32_t)(base_ptr + ring_id * sizeof(uint16_t))));
#else
    ring_read_idx_ptr = (uint16_t *)((uint32_t)(base_ptr + ring_id * sizeof(uint16_t)));
#endif

    memcpy(&ring_read_idx, ring_read_idx_ptr, sizeof(uint16_t));

    bdmf_session_print(session, "D2R Read Index ring %d: %d\n", ring_id, ring_read_idx);
    bdmf_session_print(session, "ring 0: tx complete, ring 1: rx complete, 2-127: not used\n"); 

    return 0;
}

static int _rdd_print_dhd_r2d_wr_idx(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_DTS *radio_instance_table_ptr;
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_DTS *radio_instance_entry_ptr;
    uint16_t *ring_write_idx_ptr;
    uint16_t ring_write_idx;
    uint32_t radio_idx, ring_id, base_ptr;

    radio_idx = (uint32_t)parm[0].value.unumber;
    ring_id = (uint32_t)parm[1].value.unumber;

    radio_instance_table_ptr = RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_PTR();
    radio_instance_entry_ptr = &radio_instance_table_ptr->entry[radio_idx];

    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_R2D_WR_FR_DESC_BASE_PTR_READ(base_ptr, radio_instance_entry_ptr);

#ifndef FIRMWARE_INIT
    ring_write_idx_ptr = (uint16_t *)phys_to_virt((phys_addr_t)((uint32_t)(base_ptr + ring_id * sizeof(uint16_t))));
#else
    ring_write_idx_ptr = (uint16_t *)((uint32_t)(base_ptr + ring_id * sizeof(uint16_t)));
#endif

    memcpy(&ring_write_idx, ring_write_idx_ptr, sizeof(uint16_t));

    bdmf_session_print(session, "R2D Write Index ring %d: %d\n", ring_id, ring_write_idx);
    bdmf_session_print(session, "ring 0: not used, ring 1: rx post, 2-127: tx post\n"); 

    return 0;
}

static int _rdd_print_dhd_r2d_rd_idx(bdmf_session_handle session, const bdmfmon_cmd_parm_t parm[], uint16_t n_parms)
{
    RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_DTS *radio_instance_table_ptr;
    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_DTS *radio_instance_entry_ptr;
    uint16_t *ring_read_idx_ptr;
    uint16_t ring_read_idx;
    uint32_t radio_idx, ring_id, base_ptr;

    radio_idx = (uint32_t)parm[0].value.unumber;
    ring_id = (uint32_t)parm[1].value.unumber;

    radio_instance_table_ptr = RDD_DHD_RADIO_INSTANCE_COMMON_B_DATA_PTR();
    radio_instance_entry_ptr = &radio_instance_table_ptr->entry[radio_idx];

    RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_R2D_RD_FR_DESC_BASE_PTR_READ(base_ptr, radio_instance_entry_ptr);

#ifndef FIRMWARE_INIT
    ring_read_idx_ptr = (uint16_t *)phys_to_virt((phys_addr_t)((uint32_t)(base_ptr + ring_id * sizeof(uint16_t))));
#else  /* FIRMWARE_INIT */
    ring_read_idx_ptr = (uint16_t *)((uint32_t)(base_ptr + ring_id * sizeof(uint16_t)));
#endif

    memcpy(&ring_read_idx, ring_read_idx_ptr, sizeof(uint16_t));

    bdmf_session_print(session, "R2D Read Index ring %d: %d\n", ring_id, ring_read_idx);
    bdmf_session_print(session, "ring 0: not used, ring 1: rx post, 2-127: tx post\n");

    return 0;
}
#endif

void rdd_dhd_helper_shell_cmds_init(bdmfmon_handle_t rdd_dir)
{
    MAKE_BDMF_SHELL_CMD(rdd_dir, "pdtxp", "print DHD TX POST descriptor", _rdd_print_dhd_tx_post_desc,
        BDMFMON_MAKE_PARM("radio_idx", "radio index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("ring_id", "ring id", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("desc_id", "desc id", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_dir, "pdrxp",   "print DHD RX POST descriptor", _rdd_print_dhd_rx_post_desc,
        BDMFMON_MAKE_PARM("radio_idx", "radio index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("desc_id", "desc id", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_dir, "pdtxc",   "print DHD TX COMPLETE descriptor", _rdd_print_dhd_tx_complete_desc,
        BDMFMON_MAKE_PARM("radio_idx", "radio index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("desc_id", "desc id", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_dir, "pdrxc",   "print DHD RX COMPLETE descriptor", _rdd_print_dhd_rx_complete_desc,
        BDMFMON_MAKE_PARM("radio_idx", "radio index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("desc_id", "desc id", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD_NOPARM(rdd_dir, "pddc",  "print DHD drop counters", _rdd_print_dhd_drop_counters);
    MAKE_BDMF_SHELL_CMD_NOPARM(rdd_dir, "pdfrc",  "print DHD Flow Rings cache", _rdd_print_dhd_flow_rings_cache);
#if defined CM3390
    MAKE_BDMF_SHELL_CMD(rdd_dir, "pd2rwi", "print DHD d2r write indexes", _rdd_print_dhd_d2r_wr_idx,
        BDMFMON_MAKE_PARM("radio_idx", "radio index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("ring_id", "ring id", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_dir, "pd2rri", "print DHD d2r read indexes", _rdd_print_dhd_d2r_rd_idx,
        BDMFMON_MAKE_PARM("radio_idx", "radio index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("ring_id", "ring id", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_dir, "pr2dwi", "print DHD r2d write indexes", _rdd_print_dhd_r2d_wr_idx,
        BDMFMON_MAKE_PARM("radio_idx", "radio index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("ring_id", "ring id", BDMFMON_PARM_NUMBER, 0));
    MAKE_BDMF_SHELL_CMD(rdd_dir, "pr2dri", "print DHD r2d write indexes", _rdd_print_dhd_r2d_rd_idx,
        BDMFMON_MAKE_PARM("radio_idx", "radio index", BDMFMON_PARM_NUMBER, 0),
        BDMFMON_MAKE_PARM("ring_id", "ring id", BDMFMON_PARM_NUMBER, 0));
#endif
}

#endif /* USE_BDMF_SHELL */
#endif /* CONFIG_DHD_RUNNER */
