/*
 **************************************************************************
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
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
 **************************************************************************
 */

/*
 * nss_portifmgr.h
 *	Port interface manager for NSS
 */
#ifndef __NSS_PORTIFMGR_H
#define __NSS_PORTIFMGR_H

/*
 * portid interface net_device private structure
 */
struct nss_portifmgr_priv {
	struct nss_ctx_instance *nss_ctx;	/**< Pointer to NSS context */
	struct net_device *real_dev;		/**< Pointer to the real gmac net_device */
	int if_num;				/**< Interface number */
	int port_id;				/**< Switch port id */
};

/**
 * @brief Create a net_device on a switch port
 *
 * @param port_id mapped port id
 * @param gmac_id mapped gmac id
 * @param name net_device name
 *
 * @return created net_device
 */
extern struct net_device *nss_portifmgr_create_if(int port_id, int gmac_id, const char *name);

/**
 * @brief Destroy a port interface net_device
 *
 * @param ndev net_device to be destroyed
 *
 * @return none
 */
extern void nss_portifmgr_destroy_if(struct net_device *ndev);
#endif /* __NSS_PORTIFMGR_H */

