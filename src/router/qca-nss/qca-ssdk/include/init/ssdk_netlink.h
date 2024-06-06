/*
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _SSDK_NETLINK_H_
#define _SSDK_NETLINK_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#define SSDK_GENL_FAMILY_NAME "ssdk_nl_family"
#define SSDK_GENL_MCAST_GRP_NAME "ssdk_nl_mcast"
#define SSDK_GENL_VERSION 1

enum ssdk_attrs {
	SSDK_ATTR_UNSPEC,
	SSDK_ATTR_IFNAME,	/* interface name */
	SSDK_ATTR_MACADDR,	/* mac address */
	SSDK_ATTR_MACPOLL,	/* macpoll: 1 enable, 0 disable */
	SSDK_ATTR_MAX,
};

enum ssdk_nl_commands {
	SSDK_COMMAND_NEW_MAC,
	SSDK_COMMAND_EXPIRE_MAC,
	SSDK_COMMAND_POLL_MAC,
	SSDK_COMMAND_MAX,
};

int ssdk_genl_init(void);
int ssdk_genl_deinit(void);
int ssdk_genl_notify_mac_info(char cmd, char *ifname, unsigned char *addr);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _SSDK_NETLINK_H_ */

