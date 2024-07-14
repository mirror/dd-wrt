/*
 * Copyright (c) 2012, The Linux Foundation. All rights reserved.
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

#ifndef HYFI_FDB_H_
#define HYFI_FDB_H_

#include "hyfi_bridge.h"

static inline int hyfi_fdb_should_update(struct hyfi_net_bridge *hyfi_br,
		const struct net_bridge_port *src, const struct net_bridge_port *dst)
{
	return (hyfi_portgrp_relay(hyfi_bridge_get_port(src))
			|| !hyfi_portgrp_relay(hyfi_bridge_get_port(dst)));
}

int hyfi_fdb_init(void);
void hyfi_fdb_fini(void);
void hyfi_fdb_perport(struct hyfi_net_bridge *hyfi_br, struct __switchport_index *pid);
/*
 * Fill buffer with forwarding table records in
 * the API format.
 */
int hyfi_fdb_fillbuf(struct net_bridge *br, void *buf, u_int32_t buf_len,
		u_int32_t skip, u_int32_t *bytes_written, u_int32_t *bytes_needed);

#endif /* HYFI_FDB_H_ */
