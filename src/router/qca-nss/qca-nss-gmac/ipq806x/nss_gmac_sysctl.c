/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
 * USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 * @file
 * This is the network independent layer to handle sysctls exposed by driver.
 */

#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/sysctl.h>
#include <nss_gmac_dev.h>

/* nss_gmac_per_prec_stats_enable_handler()
 *	Enable/Disable per-precedence statistics
 */
int nss_gmac_per_prec_stats_enable_handler(struct ctl_table *table, int write,
					   void __user *buffer, size_t *lenp,
					   loff_t *ppos)
{
	int ret;

	ret = proc_dointvec(table, write, buffer, lenp, ppos);

	if (!write)
		return ret;

	if (ret) {
		pr_err("Errno: -%d.\n", ret);
		return ret;
	}

	switch (ctx.nss_gmac_per_prec_stats_enable) {
	case 0:
	case 1:
		break;
	default:
		pr_err("Invalid input. Valid input values: <0|1>\n");
		ret = -1;
		break;
	}

	return ret;
};

/* nss_gmac_per_prec_stats_reset_handler()
 *	Reset per-precedence statistics
 */
int nss_gmac_per_prec_stats_reset_handler(struct ctl_table *table, int write,
				   void __user *buffer, size_t *lenp,
				   loff_t *ppos)
{
	int ret, i;

	ret = proc_dointvec(table, write, buffer, lenp, ppos);

	if (!write)
		return ret;

	if (ret) {
		pr_err("Errno: -%d.\n", ret);
		return ret;
	}

	switch (ctx.nss_gmac_per_prec_stats_reset) {
	case 0:
		break;
	case 1: {
		for (i = 0; i < NSS_MAX_GMACS; i++) {
			if (ctx.nss_gmac[i]) {
				spin_lock_bh(&ctx.nss_gmac[i]->stats_lock);
				memset(ctx.nss_gmac[i]->nss_host_stats.tx_per_prec, 0, sizeof(u64) * NSS_GMAC_PRECEDENCE_MAX);
				memset(ctx.nss_gmac[i]->nss_host_stats.rx_per_prec, 0, sizeof(u64) * NSS_GMAC_PRECEDENCE_MAX);
				spin_unlock_bh(&ctx.nss_gmac[i]->stats_lock);
			}
		}
		break;
	}
	default:
		pr_err("Invalid input. Valid input value: 1\n");
		ret = -1;
		break;
	}

	return ret;
}

