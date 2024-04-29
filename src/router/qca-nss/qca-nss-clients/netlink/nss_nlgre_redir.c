/*
 ***************************************************************************
 * Copyright (c) 2015-2016,2018-2020, The Linux Foundation. All rights reserved.
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
 ***************************************************************************
 */

#include <linux/version.h>
#include <linux/etherdevice.h>
#include <linux/if_ether.h>
#include <net/genetlink.h>
#include <nss_api_if.h>
#include <nss_nl_if.h>
#include "nss_nlcmn_if.h"
#include "nss_nl.h"
#include "nss_nlgre_redir_if.h"
#include "nss_nlgre_redir_cmn.h"
#include "nss_nlgre_redir.h"

/*
 * nss_nlgre_redir_destroy_tun()
 * 	Destroys the gre_redir tunnel
 */
int nss_nlgre_redir_destroy_tun(struct net_device *dev)
{
	int ret;

	if (!dev) {
		nss_nl_error("Dev is NULL\n");
		return -EINVAL;
	}

	ret = nss_nlgre_redir_cmn_destroy_tun(dev);
	if (ret < 0) {
		nss_nl_error("Could not destroy the tunnel\n");
		return -EINVAL;
	}

	nss_nl_info("Successfully destroyed the tunnel\n");
	return 0;
}

/*
 * nss_nlgre_redir_create_tun()
 * 	Creates a gre_redir tunnel
 */
int nss_nlgre_redir_create_tun(struct nss_nlgre_redir_create_tun *create_params)
{
	struct net_device *dev;

	if (!create_params) {
		nss_nl_error("create_params is NULL\n");
		return -EINVAL;
	}

	dev = nss_nlgre_redir_cmn_create_tun(create_params->sip, create_params->dip, create_params->iptype);
	if (!dev) {
		nss_nl_error("Could not create tunnel\n");
		return -EINVAL;
	}

	nss_nl_info("Successfully created the tunnel = %s\n", dev->name);
	return 0;
}

/*
 * nss_nlgre_redir_map_interface()
 * 	Maps the nss interface to the tunnel ID
 */
int nss_nlgre_redir_map_interface(struct nss_nlgre_redir_map *map_params)
{
	struct nss_ctx_instance *nss_ctx;
	uint32_t nexthop_nssif, vap_nss_if;
	uint8_t tun_type;
	int ret;

	if (!map_params) {
		nss_nl_error("map params is NULL\n");
		return -EINVAL;
	}

	nss_ctx = nss_gre_redir_get_context();
	vap_nss_if = nss_nlgre_redir_cmn_get_dev_ifnum(map_params->vap_nss_if);

	/*
	 * Get tunnel type from tun_type string
	 */
	tun_type = nss_nlgre_redir_cmn_get_tun_type(map_params->tun_type);
	switch(tun_type) {
	case NSS_NLGRE_REDIR_TUN_TYPE_DTUN:
	case NSS_NLGRE_REDIR_TUN_TYPE_TUN:
		nexthop_nssif = vap_nss_if;
		break;
	case NSS_NLGRE_REDIR_TUN_TYPE_SPLIT:
		nexthop_nssif = NSS_ETH_RX_INTERFACE;
		break;
	default:
		nss_nl_error("%px: not a valid tunnel_type\n", nss_ctx);
		return -1;
	}

	/*
	 * Map the nss interface
	 */
	ret = nss_nlgre_redir_cmn_map_interface(nexthop_nssif, 0, map_params);
	if (ret == -1) {
		nss_nl_error("%px: Unable to map nss interface\n", nss_ctx);
		return -1;
	}

	nss_nl_info("Successfully mapped the nss interface to tunnel ID\n");
	return 0;
}

/*
 * nss_nlgre_redir_set_next_hop()
 * 	Sets the next hop of vap as wifi_offld_inner interface of gre_redir node
 */
int nss_nlgre_redir_set_next_hop(struct nss_nlgre_redir_set_next *set_next_params)
{
	enum nss_nlgre_redir_cmn_mode_type mode;
	struct nss_ctx_instance *nss_ctx;
	struct net_device *next_dev;
	uint32_t nexthop_ifnum;
	int ret;

	if (!set_next_params) {
		nss_nl_error("set next params is NULL\n");
		return -EINVAL;
	}

	nss_ctx = nss_gre_redir_get_context();
	next_dev = dev_get_by_name(&init_net, set_next_params->next_dev_name);
	if (!next_dev) {
		nss_nl_error("%px: Unable to get the reference to dev %s\n", nss_ctx, set_next_params->next_dev_name);
		return -1;
	}

	dev_put(next_dev);
	mode = nss_nlgre_redir_cmn_mode_str_to_enum(set_next_params->mode);
	switch(mode) {
	case NSS_NLGRE_REDIR_CMN_MODE_TYPE_WIFI:
		/*
		 * Gets the wifi_offl_inner_ifnum interface number of gretun[index]
		 */
		nexthop_ifnum = nss_nlgre_redir_cmn_get_tun_ifnum(NSS_NLGRE_REDIR_CMN_MODE_TYPE_WIFI, next_dev);
		break;
	case NSS_NLGRE_REDIR_CMN_MODE_TYPE_SPLIT:
		nexthop_ifnum = NSS_ETH_RX_INTERFACE;
		break;
	default:
		nss_nl_error("%px: Unknown set next mode\n", nss_ctx);
		return -1;
	}

	ret = nss_nlgre_redir_cmn_set_next_hop(nexthop_ifnum, set_next_params);
	if (ret == -1) {
		nss_nl_error("%px: Unable to set the next hop\n", nss_ctx);
		return -1;
	}

	nss_nl_info("Successfully set the next hop\n");
	return 0;
}
