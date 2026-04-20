/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _PPE_TUN_H_
#define _PPE_TUN_H_

#include <ppe_acl.h>
#include <ppe_drv_sc.h>
#include <ppe_tun.h>

/*
 * PPE Tunnel debug macros
 */
#if (PPE_TUN_DEBUG_LEVEL == 3)
#define ppe_tun_assert(c, s, ...)
#else
#define ppe_tun_assert(c, s, ...) if (!(c)) { printk(KERN_CRIT "%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__); BUG_ON(!(c)); }
#endif

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * If dynamic debug is enabled, use pr_debug.
 */
#define ppe_tun_warn(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define ppe_tun_info(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define ppe_tun_trace(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#else

/*
 * Statically compile messages at different levels, when dynamic debug is disabled.
 */
#if (PPE_TUN_DEBUG_LEVEL < 2)
#define ppe_tun_warn(s, ...)
#else
#define ppe_tun_warn(s, ...) pr_warn("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (PPE_TUN_DEBUG_LEVEL < 3)
#define ppe_tun_info(s, ...)
#else
#define ppe_tun_info(s, ...) pr_notice("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif

#if (PPE_TUN_DEBUG_LEVEL < 4)
#define ppe_tun_trace(s, ...)
#else
#define ppe_tun_trace(s, ...) pr_info("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#endif
#endif

/*
 * Maximum number of virtual port tunnels
 */
#define PPE_TUN_MAX	PPE_DRV_VIRTUAL_MAX

/*
 * Convert virtual port number to an idx
 */
#define PPE_TUN_VP_NUM_TO_IDX(vp_num)	((vp_num) - PPE_DRV_VIRTUAL_START)

#define PPE_TUN_DISABLE	0
#define PPE_TUN_ENABLE	1

enum ppe_tun_state {
	PPE_TUN_STATE_CONFIGURED	= (1 << 0),
};

enum xcpn_mode {PPE_TUN_XCPN_MODE_0, PPE_TUN_XCPN_MODE_1};

/*
 * ppe_tun_accel
 *	Enable / Disable acceleration for tunnel type
 */
struct ppe_tun_accel {
	bool ppe_tun_gretap_accel;	/* Controls gretap acceleration */
	bool ppe_tun_vxlan_accel;	/* Controls vxlan acceleration */
	bool ppe_tun_ipip6_accel;	/* Controls ipip6 acceleration */
	bool ppe_tun_mapt_accel;	/* Controls mapt acceleration */
	bool ppe_tun_l2tp_accel;	/* Controls l2tp acceleration */
};

/*
 * ppe_tun_xcpn_mode
 *	Enable / Disable xcpn_mode for tunnel type
 */
struct ppe_tun_xcpn_mode {
	uint8_t gretap;	/* Controls gretap exception mode */
	uint8_t ipip6;	/* Controls ipip6 exception mode */
	uint8_t l2tp;	/* Controls l2tp exception mode */
};

/*
 * ppe_tun
 *	Main ppe_tun structure to hold information about tunnel node.
 */
struct ppe_tun {
	struct kref ref;			/* Reference count */
	int32_t idx;				/* PPE tunne Index */
	ppe_vp_num_t vp_num;			/* Port number attached for VP */
	enum ppe_tun_state state;		/* PPE tunnel status flags */
	enum ppe_drv_tun_cmn_ctx_type type;	/* Tunnel type */
	struct net_device *dev;			/* Tunnel netdev */
	struct net_device *phys_dev;		/* Physical dev attached to VP */
	ppe_tun_exception_method_t src_excp;	/* Callback for exception packets with src VP */
	ppe_tun_exception_method_t dest_excp;	/* Callback for exception packets with dest VP */
	atomic64_t exception_packet;		/* Number of exception packets seen by tunnel */
	atomic64_t exception_bytes;		/* Total exception bytes total */
	ppe_tun_stats_method_t stats_excp;	/* Callback for updating tunnel statistics */
	ppe_tun_data *tun_data;		/* Tunnel specific data from client */
};

/*
 * ppe_tun_priv
 *	Private structure maintaining state of PPE tunnel driver.
 */
struct ppe_tun_priv {
	struct ppe_tun *tun[PPE_TUN_MAX];	/* Active tunnels index by vp_num PPE */
	spinlock_t lock;			/* Base lock */
	struct dentry *dentry;			/* Debugfs entry */
	struct ppe_tun_accel tun_accel;		/* Enable or disable acceleration per tunnel type */
	struct ppe_tun_xcpn_mode xcpn_mode;	/* Toggle exception mode */
	ppe_acl_rule_id_t ppe_tun_l2_tunnel_rule_id;	/* PPE ACL rule-id for L2 Tunnels */
	atomic_t total_free;			/* Number of available tunnel instance*/
	atomic_t free_pending;			/* Number of tunnel delete pending */
	atomic_t alloc_fail;			/* Number of tunnel alloc fails */
};
#endif /* _PPE_TUN_H_ */
