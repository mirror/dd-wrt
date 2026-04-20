/*
 **************************************************************************
 * Copyright (c) 2019, The Linux Foundation. All rights reserved.
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

/**
 * nss_eogremgr.h
 *	Hedaer file of eogre manager
 */
#ifndef __NSS_EOGREMGR_H
#define __NSS_EOGREMGR_H

#include <linux/if_vlan.h>
#include <net/gre.h>
#include <net/ipv6.h>
#include "nss_connmgr_gre_public.h"


/*
 * Maximum number of tunnels currently supported
 */
#define NSS_EOGREMGR_MAX_TUNNELS	64

/*
 * Maximum headroom required for an EoGRE tunnel. Include GRE key (4 bytes) in the calculation
 */
#define NSS_EOGREMGR_MAX_HEADROOM	sizeof(struct ethhdr) + sizeof(struct vlan_hdr) + sizeof(struct gre_base_hdr) + sizeof(uint32_t) + sizeof(struct ipv6hdr)

typedef enum nss_eogremgr_err_code {
	NSS_EOGRE_SUCCESS = 0,			/**< Eogre success */
	NSS_EOGRE_ERR_INVALID_TUNNEL_ID = 1,		/**< Invalid tunnel id */
	NSS_EOGRE_ERR_CREATE_IP_RULE_FAILED = 2,	/**< Create IP Rule failed */
	NSS_EOGRE_ERR_DESTROY_IP_RULE_FAILED = 3,	/**< Destroy IP Rule failed */
	NSS_EOGRE_ERR_BAD_NSS_CTX = 4,		/**< Failed to get NSS context */
	NSS_EOGRE_ERR_NETDEV_DISABLE_FAILED = 5,	/**< Failed to disable netdev */
	NSS_EOGRE_ERR_NETDEV_ENABLE_FAILED = 6,	/**< Failed to enable netdev */
	NSS_EOGRE_ERR_ALLOC_TUNNEL_FAILED = 7,	/**< Alloc tunnel memory failed */
	NSS_EOGRE_TUNNEL_DESTROY_FAILED = 8,	/**< Tunnel destroy failed */
	NSS_EOGRE_ERR_GRE_CREATE_FAILED = 9,	/**< GRE interface create failed */
	NSS_EOGRE_ERR_TUNNEL_ID_EXIST = 10,	/**< Tunnel id already exist */
	NSS_EOGRE_ERR_INVALID_GRE_IFNUM = 11,	/**< Failed to get valid GRE interface number */
	NSS_EOGRE_ERR_MAX
} nss_eogremgr_status_t;

/**
 *
 * @brief Get GRE inner NSS interface number associated with the tunnel_id
 *
 * @param[in] tunnel_id
 *
 * @return int
 */
extern int nss_eogremgr_get_if_num_inner(uint32_t tunnel_id);

/**
 *
 * @brief Disable a EoGRE tunnel
 *
 * @param[in] tunnel_id
 *
 * @return nss_eogremgr_status_t
 */
extern nss_eogremgr_status_t nss_eogremgr_tunnel_disable(uint32_t tunnel_id);

/**
 *
 * @brief Enable a EoGRE tunnel
 *
 * @param[in] tunnel_id
 *
 * @return nss_eogremgr_status_t
 */
extern nss_eogremgr_status_t nss_eogremgr_tunnel_enable(uint32_t tunnel_id);

/**
 *
 * @brief Destroy the EoGRE tunnel for the given tunnel_id
 *
 * @param[in] tunnel_id
 *
 * @return nss_eogremgr_status_t
 */
extern nss_eogremgr_status_t nss_eogremgr_tunnel_destroy(uint32_t tunnel_id);

/**
 *
 * @brief Create an IPv6 type EoGRE tunnel for the given tunnel_id
 *
 * @param[in] cfg        GRE configuration structure
 * @param[in] nircm      IPv6 rule create message
 * @param[in] tunnel_id  EoGRE tunnel_id
 *
 * @return nss_eogremgr_status_t
 */
extern nss_eogremgr_status_t nss_eogremgr_tunnel_ipv6_create(struct nss_connmgr_gre_cfg *cfg, struct nss_ipv6_rule_create_msg *nircm, uint32_t tunnel_id);

/**
 *
 * @brief Create an IPv4 type EoGRE tunnel for the given tunnel_id
 *
 * @param[in] cfg        GRE configuration structure
 * @param[in] nircm      IPv4 rule create message
 * @param[in] tunnel_id  EoGRE tunnel_id
 *
 * @return nss_eogremgr_status_t
 */
extern nss_eogremgr_status_t nss_eogremgr_tunnel_ipv4_create(struct nss_connmgr_gre_cfg *cfg, struct nss_ipv4_rule_create_msg *nircm, uint32_t tunnel_id);

#endif /* __NSS_EOGREMGR_H */
