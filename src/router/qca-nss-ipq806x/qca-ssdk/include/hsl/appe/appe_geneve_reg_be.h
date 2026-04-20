/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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


/**
 * @defgroup
 * @{
 */
#ifndef APPE_GENEVE_REG_H
#define APPE_GENEVE_REG_H

/*[register] TPR_GENEVE_CFG*/
#define TPR_GENEVE_CFG
#define TPR_GENEVE_CFG_ADDRESS 0x4a8
#define TPR_GENEVE_CFG_NUM     1
#define TPR_GENEVE_CFG_INC     0x4
#define TPR_GENEVE_CFG_TYPE    REG_TYPE_RW
#define TPR_GENEVE_CFG_DEFAULT 0x0
        /*[field] UDP_PORT_MAP*/
        #define TPR_GENEVE_CFG_UDP_PORT_MAP
        #define TPR_GENEVE_CFG_UDP_PORT_MAP_OFFSET  0
        #define TPR_GENEVE_CFG_UDP_PORT_MAP_LEN     6
        #define TPR_GENEVE_CFG_UDP_PORT_MAP_DEFAULT 0x0

struct tpr_geneve_cfg {
        a_uint32_t  _reserved0:26;
        a_uint32_t  udp_port_map:6;
};

union tpr_geneve_cfg_u {
        a_uint32_t val;
        struct tpr_geneve_cfg bf;
};
#endif
