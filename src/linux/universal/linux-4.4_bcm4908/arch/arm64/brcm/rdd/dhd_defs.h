/*
   Copyright (c) 2015 Broadcom Corporation
   All Rights Reserved

    <:label-BRCM:2015:DUAL/GPL:standard
    
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

#ifndef _DHD_DEFS_H_
#define _DHD_DEFS_H_

#define DHD_DOORBELL_IRQ_NUM                        2
#define DHD_RXPOST_IRQ_NUM                          5
#define DHD_MSG_TYPE_TX_POST                        0xF
#define DHD_MSG_TYPE_RX_POST                        0x11
#define DHD_TX_POST_FLOW_RING_DESCRIPTOR_SIZE       48
#define DHD_RX_POST_FLOW_RING_SIZE                  1024
#define DHD_TX_COMPLETE_FLOW_RING_SIZE              1024
#define DHD_RX_COMPLETE_FLOW_RING_SIZE              1024
#define DHD_DATA_LEN                                2048
#define DHD_RX_POST_RING_NUMBER                     1  
#define DHD_TX_COMPLETE_RING_NUMBER                 3
#define DHD_RX_COMPLETE_RING_NUMBER                 4
#if !defined(DSL_63138) && !defined(DSL_63148) && !defined(WL4908)
#define DHD_DATA_OFFSET                             20
#endif
#define DHD_RX_POST_INT_COUNT                       32

/* The host DHD driver sets the Tx Post descriptor request_id field with the 32 bit
 * aligned SKB/FKB address.  Therefore, the least significant two bits are always 0.
 * These bits are used to indicate packet type.  Value 0 is SKB and value 3 is FKB.
 * The address and packet type are stored in little endian byte ordering.  This
 * causes the packet type to be in bits [25:24] instead of [1:0] on the big endian
 * Runner processor.  DHD offload firmware will use the packet type field to indicate
 * RDD host buffer (1) and Runner BPM buffer (2).
 */
#define DHD_TX_POST_BUFFER_TYPE_WIDTH               2
#define DHD_TX_POST_BUFFER_TYPE_OFFSET              24
#define DHD_TX_POST_HOST_BUFFER_BIT_OFFSET          DHD_TX_POST_BUFFER_TYPE_OFFSET
#define DHD_TX_POST_SKB_BUFFER_VALUE                0   /* 00: possible value in tx complete only */
#define DHD_TX_POST_HOST_BUFFER_VALUE               1   /* 01: possible value in tx post and tx complete */
#define DHD_TX_POST_BPM_BUFFER_VALUE                2   /* 10: possible value in tx post and tx complete */
#define DHD_TX_POST_FKB_BUFFER_VALUE                3   /* 11: possible value in tx complete only */
#define DHD_COMPLETE_OWNERSHIP_RUNNER               2   /* 02: (00/01/11 are reserved for SKB/FKB/HOST cases) queued on DHD Complete CPU ring */

#define DHD_TX_BPM_REF_COUNTER_TAIL_OFFSET          16
#define DHD_TX_POST_EXCLUSIVE_OFFSET                27

#define DHD_MSG_TYPE_FLOW_RING_FLUSH                0
#define DHD_MSG_TYPE_FLOW_RING_SET_DISABLED         1

#define DHD_TX_POST_FLOW_RING_CACHE_SIZE            16
#define DHD_FLOW_RING_CACHE_LKP_DEPTH               CAM_SEARCH_DEPTH_16

#define DHD_FLOW_RING_DISABLED_BIT                  1 /* (1 << 1) */

#define DHD_RADIO_OFFSET_COMMON_A(index)            (DHD_RADIO_INSTANCE_COMMON_A_DATA_ADDRESS + (index * sizeof(RDD_DHD_RADIO_INSTANCE_COMMON_A_ENTRY_DTS)))
#define DHD_RADIO_OFFSET_COMMON_B(index)            (DHD_RADIO_INSTANCE_COMMON_B_DATA_ADDRESS + (index * sizeof(RDD_DHD_RADIO_INSTANCE_COMMON_B_ENTRY_DTS)))

/* LLCSNAP definitions */
#define DHD_LLCSNAP_HEADER_SIZE                     8
#define DHD_ETH_LENGTH_TYPE_OFFSET                  12
#define DHD_ETH_FCS_SIZE                            4
#define DHD_ETH_L2_HEADER_SIZE                      14
#define DHD_LLCSNAP_CONTROL_OFFSET                  19
#define DHD_LLCSNAP_PROTOCOL_OFFSET                 20
#define DHD_LLCSNAP_END_OFFSET                      d.22
#define DHD_ETH_TYPE_MAX_DATA_LEN                   h.05dc
#define DHD_ETH_TYPE_APPLE_ARP                      h.80f3
#define DHD_ETH_TYPE_NOVELL_IPX                     h.8137
#define DHD_LLCSNAP_DSAP_SSAP_VALUE                 h.aaaa
#define DHD_LLCSNAP_CONTROL_VALUE                   h.0300
#define DHD_LLCSNAP_OUI_BRIDGE_TUNNEL_VALUE         h.f8

#endif

