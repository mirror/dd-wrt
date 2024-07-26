/*
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <ppe_vp_public.h>

/*
 * ppe_tun_l2tp_info
 *	L2TP specific tunnel information
 */
struct ppe_tun_l2tp_info {
	uint16_t session_id;
	uint16_t tunnel_id;
};

/*
 * ppe_tun_data
 *	Tunnel specific information to be stored for a tunnel
 */
typedef union {
	struct ppe_tun_l2tp_info l2tp_info;
} ppe_tun_data;

typedef ppe_vp_hw_stats_t ppe_tun_hw_stats;
typedef bool(*ppe_tun_exception_method_t)(struct ppe_vp_cb_info *info, ppe_tun_data  *tun_data);
typedef bool (*ppe_tun_stats_method_t)(struct net_device *dev, ppe_tun_hw_stats *stats, ppe_tun_data *tun_data);

/*
 * ppe_tun_excp
 *	ppe_tun structure to hold callback data.
 */
struct ppe_tun_excp {
	ppe_tun_exception_method_t src_excp_method;	/**< callback for exception packets with src VP >**/
	ppe_tun_exception_method_t dest_excp_method;	/**< callback for exception packets with dest VP >**/
	ppe_tun_stats_method_t stats_update_method;	/**< callback for updating tunnel statistics >**/
	ppe_tun_data *tun_data;			/**< Tunnel specific data from client >**/
};
