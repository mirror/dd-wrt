/*
 **************************************************************************
 * Copyright (c) 2014-2015,2019-2020, The Linux Foundation. All rights reserved.
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
 * nss_nlgre_redir_cmd.h
 *	NSS Netlink gre_redir API definitions
 */
#ifndef __NSS_NLGRE_REDIR_CMD_H
#define __NSS_NLGRE_REDIR_CMD_H
#define NSS_NLGRE_REDIR_CMD_MAX 7
/*
 * nss_nlgre_redir_cmd_deploy_mode
 * 	Gre_redir deployment mode types
 */
enum nss_nlgre_redir_cmd_deploy_mode {
	NSS_NLGRE_REDIR_CMD_DEPLOY_MODE_UNKNOWN,			/**< Invalid mode */
	NSS_NLGRE_REDIR_CMD_DEPLOY_MODE_NON_LAG,			/**< Basic tunnel mode */
	NSS_NLGRE_REDIR_CMD_DEPLOY_MODE_LAG,				/**< Lag mode */
	NSS_NLGRE_REDIR_CMD_DEPLOY_MODE_MAX				/**< Maximum deploy mode */
};

/*
 * To keep track of family operations
 */
extern struct genl_family nss_nlgre_redir_cmd_family;

/*
 * nss_nlgre_redir_cmd_get_ifnum()
 * 	Get the interface number corresponding to netdev
 */
int nss_nlgre_redir_cmd_get_ifnum(struct net_device *dev, uint8_t proto);

/*
 * nss_nlgre_redir_cmd_get_mtu()
 * 	Returns the mtu based on the device passed
 */
int nss_nlgre_redir_cmd_get_mtu(struct net_device *dev, uint8_t iptype, int ifnum);

#endif /* __NSS_NLGRE_REDIR_CMD_H */
