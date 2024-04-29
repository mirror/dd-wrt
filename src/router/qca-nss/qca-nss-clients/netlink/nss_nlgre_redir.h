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

#define NSS_NLGRE_REDIR_MODE_MAX_SZ 16		/**< Max size of mode value */

/*
 * nss_nlgre_redir_tun_type
 * 	Different tunnel types supported in gre_redir
 */
enum nss_nlgre_redir_tun_type {
	NSS_NLGRE_REDIR_TUN_TYPE_UNKNOWN,		/**< Unknown tunnel type */
	NSS_NLGRE_REDIR_TUN_TYPE_TUN,			/**< Raw mode 802.11 frames traffic*/
	NSS_NLGRE_REDIR_TUN_TYPE_DTUN,			/**< For 802.3 frames traffic */
	NSS_NLGRE_REDIR_TUN_TYPE_SPLIT,			/**< For split mode */
	NSS_NLGRE_REDIR_TUN_TYPE_MAX			/**< Max number of tun type supported */
};

/*
 * nss_nlgre_redir_map_interface()
 * 	Interface map message.
 */
int nss_nlgre_redir_map_interface(struct nss_nlgre_redir_map *map_params);

/*
 * nss_nlgre_redir_set_next_hop()
 * 	Sets next hop as gre-redir for wifi.
 */
int nss_nlgre_redir_set_next_hop(struct nss_nlgre_redir_set_next *setnext_params);

/*
 * nss_nlgre_redir_create_tun()
 * 	Unregisters and deallocs dynamic interfaces.
 */
int nss_nlgre_redir_create_tun(struct nss_nlgre_redir_create_tun *create_params);

/*
 * nss_nlgre_redir_destroy_tun()
 * 	Destroy tunnel in tunnel mode.
 */
int nss_nlgre_redir_destroy_tun(struct net_device *dev);

