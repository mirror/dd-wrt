/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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


#ifndef _PPE_DS_STATS_H_
#define _PPE_DS_STATS_H_

#include <linux/atomic.h>
#include "ppe_ds.h"

/*
 * PPE-DS stats per Node
 */
struct ppe_ds_stats {
	atomic64_t tx_pkts;
	atomic64_t rx_pkts;
};

extern struct ppe_ds_stats ppe_ds_node_stats[PPE_DS_MAX_NODE];

int ppe_ds_node_stats_debugfs_init(void);
void ppe_ds_node_stats_debugfs_exit(void);
#endif
