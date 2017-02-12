/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


/**
 * @defgroup fal_sec FAL_SEC
 * @{
 */
#ifndef _FAL_SEC_H_
#define _FAL_SEC_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#include "common/sw.h"
#include "fal/fal_type.h"


    typedef enum {
        /* define MAC layer related normalization items */
        FAL_NORM_MAC_RESV_VID_CMD = 0,
        FAL_NORM_MAC_INVALID_SRC_ADDR_CMD,

        /* define IP layer related normalization items */
        FAL_NORM_IP_INVALID_VER_CMD,
        FAL_NROM_IP_SAME_ADDR_CMD,
        FAL_NROM_IP_TTL_CHANGE_STATUS,
        FAL_NROM_IP_TTL_VALUE,

        /* define IP4 related normalization items */
        FAL_NROM_IP4_INVALID_HL_CMD,
        FAL_NROM_IP4_HDR_OPTIONS_CMD,
        FAL_NROM_IP4_INVALID_DF_CMD,
        FAL_NROM_IP4_FRAG_OFFSET_MIN_LEN_CMD,
        FAL_NROM_IP4_FRAG_OFFSET_MAX_LEN_CMD,
        FAL_NROM_IP4_INVALID_FRAG_OFFSET_CMD,
        FAL_NROM_IP4_INVALID_SIP_CMD,
        FAL_NROM_IP4_INVALID_DIP_CMD,
        FAL_NROM_IP4_INVALID_CHKSUM_CMD,
        FAL_NROM_IP4_INVALID_PL_CMD,
        FAL_NROM_IP4_DF_CLEAR_STATUS,
        FAL_NROM_IP4_IPID_RANDOM_STATUS,
        FAL_NROM_IP4_FRAG_OFFSET_MIN_SIZE,

        /* define IP4 related normalization items */
        FAL_NROM_IP6_INVALID_PL_CMD,
        FAL_NROM_IP6_INVALID_SIP_CMD,
        FAL_NROM_IP6_INVALID_DIP_CMD,

        /* define TCP related normalization items */
        FAL_NROM_TCP_BLAT_CMD,
        FAL_NROM_TCP_INVALID_HL_CMD,
        FAL_NROM_TCP_INVALID_SYN_CMD,
        FAL_NROM_TCP_SU_BLOCK_CMD,
        FAL_NROM_TCP_SP_BLOCK_CMD,
        FAL_NROM_TCP_SAP_BLOCK_CMD,
        FAL_NROM_TCP_XMAS_SCAN_CMD,
        FAL_NROM_TCP_NULL_SCAN_CMD,
        FAL_NROM_TCP_SR_BLOCK_CMD,
        FAL_NROM_TCP_SF_BLOCK_CMD,
        FAL_NROM_TCP_SAR_BLOCK_CMD,
        FAL_NROM_TCP_RST_SCAN_CMD,
        FAL_NROM_TCP_SYN_WITH_DATA_CMD,
        FAL_NROM_TCP_RST_WITH_DATA_CMD,
        FAL_NROM_TCP_FA_BLOCK_CMD,
        FAL_NROM_TCP_PA_BLOCK_CMD,
        FAL_NROM_TCP_UA_BLOCK_CMD,
        FAL_NROM_TCP_INVALID_CHKSUM_CMD,
        FAL_NROM_TCP_INVALID_URGPTR_CMD,
        FAL_NROM_TCP_INVALID_OPTIONS_CMD,
        FAL_NROM_TCP_MIN_HDR_SIZE,

        /* define UDP related normalization items */
        FAL_NROM_UDP_BLAT_CMD,
        FAL_NROM_UDP_INVALID_LEN_CMD,
        FAL_NROM_UDP_INVALID_CHKSUM_CMD,

        /* define ICMP related normalization items */
        FAL_NROM_ICMP4_PING_PL_EXCEED_CMD,
        FAL_NROM_ICMP6_PING_PL_EXCEED_CMD,
        FAL_NROM_ICMP4_PING_FRAG_CMD,
        FAL_NROM_ICMP6_PING_FRAG_CMD,
        FAL_NROM_ICMP4_PING_MAX_PL_VALUE,
        FAL_NROM_ICMP6_PING_MAX_PL_VALUE,
    }
    fal_norm_item_t;

    sw_error_t
    fal_sec_norm_item_set(a_uint32_t dev_id, fal_norm_item_t item, void *value);

    sw_error_t
    fal_sec_norm_item_get(a_uint32_t dev_id, fal_norm_item_t item, void *value);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _FAL_SEC_H_ */

/**
 * @}
 */

