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

#include "nss_core.h"
#include <nss_qvpn.h>

/*
 * nss_qvpn_stats_read()
 *	Read qvpn node statiistics.
 */
static ssize_t nss_qvpn_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	struct nss_ctx_instance *nss_ctx = nss_qvpn_get_context();
	enum nss_dynamic_interface_type type;
	ssize_t bytes_read = 0;
	size_t len = 0, size;
	uint32_t if_num;
	unsigned long *ifmap;
	char *buf;

	ifmap = nss_qvpn_ifmap_get();
	size = NSS_QVPN_STATS_SIZE_PER_IF * bitmap_weight(ifmap, NSS_MAX_NET_INTERFACES);

	buf = kzalloc(size, GFP_KERNEL);
	if (!buf) {
		nss_warning("Could not allocate memory for local statistics buffer\n");
		return 0;
	}

	/*
	 * Common node stats for each QVPN dynamic interface.
	 */
	len += nss_stats_banner(buf, len, size, "qvpn", NSS_STATS_SINGLE_CORE);
	for_each_set_bit(if_num, ifmap, NSS_MAX_NET_INTERFACES) {
		type = nss_dynamic_interface_get_type(nss_ctx, if_num);

		switch (type) {
		case NSS_DYNAMIC_INTERFACE_TYPE_QVPN_INNER:
			len += scnprintf(buf + len, size - len, "\nInner if_num:%03u", if_num);
			break;

		case NSS_DYNAMIC_INTERFACE_TYPE_QVPN_OUTER:
			len += scnprintf(buf + len, size - len, "\nOuter if_num:%03u", if_num);
			break;

		default:
			len += scnprintf(buf + len, size - len, "\nUnknown(%d) if_num:%03u", type, if_num);
			break;
		}

		len += scnprintf(buf + len, size - len, "\n-------------------\n");
		len += nss_stats_fill_common_stats(if_num, NSS_STATS_SINGLE_INSTANCE, buf, len, size - len, "qvpn");
	}

	bytes_read = simple_read_from_buffer(ubuf, sz, ppos, buf, len);
	kfree(buf);

	return bytes_read;
}

/*
 * nss_qvpn_stats_ops
 */
NSS_STATS_DECLARE_FILE_OPERATIONS(qvpn)

/*
 * nss_qvpn_stats_dentry_create()
 *	Create QVPN statistics debug entry.
 */
void nss_qvpn_stats_dentry_create(void)
{
	nss_stats_create_dentry("qvpn", &nss_qvpn_stats_ops);
}
