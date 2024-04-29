/*
 ***************************************************************************
 * Copyright (c) 2014-2015,2019, The Linux Foundation. All rights reserved.
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

#define NSS_NLGRE_REDIR_LAG_SLAVES 2		/**< Maximum number of slaves for lag tunnel */

/*
 * nss_nlgre_redir_lag_pvt_data
 * 	Context need to be maintained for LAG deployment.
 */
struct nss_nlgre_redir_lag_pvt_data {
	uint32_t inner_ifnum;			/**< Inner ifnum of lag netdev */
	uint32_t outer_ifnum;			/**< Outer ifnum of lag netdev */
	struct net_device *slaves[2];		/**< Slave's dev reference*/
	struct net_device *dev;			/**< Pointer to netdev for lag */
};

/*
 * nss_nlgre_redir_lag_create_tun()
 *	Creates a lag node and configures LAG US/DS dynamic nodes.
 */
int nss_nlgre_redir_lag_create_tun(struct nss_nlgre_redir_create_tun *create_params);

/*
 * nss_nlgre_redir_lag_destroy_tun()
 * 	Destroys the lag tunnel.
 */
int nss_nlgre_redir_lag_destroy_tun(struct net_device *dev);

/*
 * nss_nlgre_redir_lag_set_next_hop()
 * 	Sets the next hop as lag US node
 */
int nss_nlgre_redir_lag_set_next_hop(struct nss_nlgre_redir_set_next *set_next_params);

/*
 * nss_nlgre_redir_lag_map_interface()
 * 	Maps the vap interface to tunnel ID
 */
int nss_nlgre_redir_lag_map_interface(struct nss_nlgre_redir_map *map_params);

/*
 * nss_nlgre_redir_lag_add_hash()
 * 	Add hash entry.
 */
int nss_nlgre_redir_lag_add_hash(struct nss_nlgre_redir_hash_ops *hash_ops);

/*
 * nss_nlgre_redir_lag_del_hash()
 * 	Delete hash entry.
 */
int nss_nlgre_redir_lag_del_hash(struct nss_nlgre_redir_hash_ops *hash_ops);

