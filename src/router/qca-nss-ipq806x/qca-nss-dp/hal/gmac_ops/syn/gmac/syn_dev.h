/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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

#ifndef __SYN_DEV_H__
#define __SYN_DEV_H__

#include <nss_dp_dev.h>
#include <fal/fal_mib.h>
#include <fal/fal_port_ctrl.h>

/*
 * Synopsys GMAC supports a maximum frame size of 16383 bytes.
 * So, the max MTU value is maximum frame size excluding
 * Ethernet header size (14B), FCS (4B) and 2x VLANs (8B).
 */
#define SYN_HAL_MAX_MTU_SIZE		16357

/*
 * SSDK API internally takes care of adding size of
 * 2xVLANs (8B) when configuring the MTU value.
 * So, the max L2 overhead does not consider 2xVLANs.
 */
#define SYN_HAL_MTU_L2_OVERHEAD		(ETH_HLEN + ETH_FCS_LEN)

/*
 * GCC_SNOC_GMAC_AXI_CLOCK
 */
#define SYN_GMAC_SNOC_GMAC_AXI_CLK	"nss-snoc-gmac-axi-clk"

/*
 * Subclass for base nss_gmac_hal_dev
 */
struct syn_hal_dev {
	struct nss_gmac_hal_dev nghd;	/* Base class */
	struct nss_dp_gmac_stats stats;	/* Stats structure */
};

#endif /*__SYN_DEV_H__*/
