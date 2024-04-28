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

/*
 * nss_ovpnmgr_sk_priv.h
 */
#ifndef __NSS_OVPNMGR_SOCKET__H
#define __NSS_OVPNMGR_SOCKET__H

/*
 * Compile messages for dynamic enable/disable
 */
#if defined(CONFIG_DYNAMIC_DEBUG)
#define nss_ovpn_sk_warn(s, ...) \
		pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_ovpn_sk_info(s, ...) \
		pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_ovpn_sk_trace(s, ...) \
		pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#else /* CONFIG_DYNAMIC_DEBUG */
/*
 * Statically compile messages at different levels
 */
#if (NSS_OVPNMGR_DEBUG_LEVEL < 2)
#define nss_ovpn_sk_warn(s, ...)
#else
#define nss_ovpn_sk_warn(s, ...) \
		pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_OVPNMGR_DEBUG_LEVEL < 3)
#define nss_ovpn_sk_info(s, ...)
#else
#define nss_ovpn_sk_info(s, ...) \
		pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif
#endif /* CONFIG_DYNAMIC_DEBUG */

/*
 * nss_ovpn_sk_pinfo
 */
struct nss_ovpn_sk_pinfo {
	struct sock sk;		/* Sock instance (struct sock must be the first member). */
	pid_t pid;		/* PID of application which has created socket. */
	int32_t tun_fd;		/* TUN/TAP character device file descriptor. */
	int32_t udp_fd;		/* UDP socket file descriptor. */
	struct socket *tun_sock;/* Socket instance. */
	struct net_device *dev;	/* TUN/TAP netdevice. */
};

int nss_ovpn_sk_init(void);
int nss_ovpn_sk_send(struct sk_buff *skb, void *app_data);
void nss_ovpn_sk_cleanup(void);

#endif /* __NSS_OVPNMGR_SOCKET__H */
