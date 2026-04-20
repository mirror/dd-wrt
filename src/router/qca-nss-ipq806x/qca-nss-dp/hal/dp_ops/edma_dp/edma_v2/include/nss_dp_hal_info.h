/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

#ifndef __NSS_DP_HAL_INFO_H__
#define __NSS_DP_HAL_INFO_H__

#include <edma.h>

/*
 * nss_dp_hal_info
 *	Data plane specific information wrapper
 */
struct nss_dp_hal_info {
	struct edma_txdesc_ring *txr_map[EDMA_TX_MAX_PRIORITY_LEVEL][NR_CPUS];
				/* Per CPU Tx descriptor ring map */
#ifdef NSS_DP_MHT_SW_PORT_MAP
	struct edma_txdesc_ring *txr_sw_port_map[NSS_DP_HAL_MHT_SWT_MAX_PORTS][NR_CPUS];
				/* Per CPU MHT switch ports Tx descriptor ring map */
#endif
	struct edma_pcpu_stats pcpu_stats;
				/* Per CPU netdev statistics */
};

#endif	/* __NSS_DP_HAL_INFO_H__ */
