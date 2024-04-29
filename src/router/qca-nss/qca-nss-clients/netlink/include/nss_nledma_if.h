/*
 **************************************************************************
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/*
 * @file nss_nledma_if.h
 *      NSS Netlink Edma headers
 */
#ifndef __NSS_NLEDMA_IF_H
#define __NSS_NLEDMA_IF_H
#include "nss_edma.h"

/**
 * Edma forwarding Family
 */
#define NSS_NLEDMA_FAMILY "nss_nledma"
#define NSS_NLEDMA_MCAST_GRP "nss_nledma_mc"

/**
 * @brief Edma stats
 */
struct nss_nledma_stats {
	uint32_t core_id;
	int port_id;
	uint64_t cmn_node_stats[NSS_STATS_NODE_MAX];
};

#endif /* __NSS_NLEDMA_IF_H */
