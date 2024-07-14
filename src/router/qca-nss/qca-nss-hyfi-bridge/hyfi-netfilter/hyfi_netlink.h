/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
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

#ifndef HYFI_NETLINK_H_
#define HYFI_NETLINK_H_

#include "hyfi_bridge.h"

void hyfi_netlink_event_send(struct hyfi_net_bridge *hyfi_br, u32 event_type,
							 u32 event_len, void *event_data);

int hyfi_netlink_init(void);

void hyfi_netlink_fini(void);

/**********************/
/* Versioning Support */
/**********************/

/* Macro to convert the macro into string literal */
#define STRINGIZE(x) #x
#define CONVERT_2_STRING(x) STRINGIZE(x)

/* ===============================================================
 * Macro definition of SON Version Information
 * Note: Do not change the values in below macro definition,
 * these values are set based on the backward compatibility
 * ==============================================================*/
#define SON_PKG_NAME "son"
#define SON_MAJOR_VERION    10
#define SON_MINOR_VERION    0
#define SON_COMPATIBILITY   0
#define SON_SU_BUILD        0

/* ===============================================================
 * Macro definition of MAP Version Information
 * Note: Do not change the values in below macro definition,
 * these values are set based on the backward compatibility
 * ==============================================================*/
#define MAP_PKG_NAME "easymesh"
#define MAP_MAJOR_VERION    12
#define MAP_MINOR_VERION    5
#define MAP_COMPATIBILITY   2
#define MAP_SU_BUILD        0

/* character length assumption :
 * 16+5+5+5+5 = 32 char + 4 sym = 36 + 1 nul character*/
#define MESH_VERSION_STR_MAX_LEN 36+1

/* =====================================================*/
/* Macro to generate SON Version string Ex. son-1.0.0.0 */
/* =====================================================*/
#define SON_VERSION SON_PKG_NAME "-" \
        CONVERT_2_STRING(SON_MAJOR_VERION) "." \
        CONVERT_2_STRING(SON_MINOR_VERION) "." \
        CONVERT_2_STRING(SON_COMPATIBILITY) "." \
        CONVERT_2_STRING(SON_SU_BUILD)

/* ==========================================================*/
/* Macro to generate MAP Version string Ex. easymesh-1.0.0.0 */
/* ==========================================================*/

#define MAP_VERSION MAP_PKG_NAME "-" \
        CONVERT_2_STRING(MAP_MAJOR_VERION) "." \
        CONVERT_2_STRING(MAP_MINOR_VERION) "." \
        CONVERT_2_STRING(MAP_COMPATIBILITY) "." \
        CONVERT_2_STRING(MAP_SU_BUILD)

/* Hyfi Bridge Driver version */
#define HYFI_BRIDGE_SON_DRIVER_VERSION SON_VERSION
#define HYFI_BRIDGE_MAP_DRIVER_VERSION MAP_VERSION

#endif /* HYFI_NETLINK_H_ */
