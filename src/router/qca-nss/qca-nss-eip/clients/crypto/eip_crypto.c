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

#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/crypto.h>
#include <soc/qcom/socinfo.h>

#include "eip_crypto.h"

/*
 * eip_crypto_dt_ids
 *	Device ID for crypto client.
 */
static const struct of_device_id eip_crypto_dt_ids[] = {
	{ .compatible = "qcom,eip_crypto" },
	{}
};
MODULE_DEVICE_TABLE(of, eip_crypto_dt_ids);

/*
 * eip_crypto_get_summary_stats()
 *	Summarizes the stats.
 */
void eip_crypto_get_summary_stats(struct eip_crypto_tfm_stats *tfm_stats, struct eip_crypto_tfm_stats *stats)
{
	int words, cpu, i;

	words = (sizeof(*stats) / sizeof(uint64_t));
	memset(stats, 0, sizeof(*stats));

	/*
	 * Iterate over all CPUs to summarize stats.
	 */
	for_each_possible_cpu(cpu) {
		const struct eip_crypto_tfm_stats *sp = per_cpu_ptr(tfm_stats, cpu);
		uint64_t *stats_ptr = (uint64_t *)stats;
		uint64_t *sp_ptr = (uint64_t *)sp;

		for (i = 0; i < words; i++, stats_ptr++, sp_ptr++) {
			*stats_ptr += *sp_ptr;
		}
	}
}

/*
 * eip_crypto_probe()
 *	Probe the eip crypto driver
 */
static int eip_crypto_probe(struct platform_device *pdev)
{
	struct device_node *np;
	int ret;

	/*
	 * Get the node corresponding to pdev.
	 */
	np = of_node_get(pdev->dev.of_node);
	if (!np) {
		pr_warn("%px: Invalid driver.", pdev);
		return -EINVAL;
	}

	/*
	 * Initialize the crypto AEAD services.
	 */
	if ((ret = eip_crypto_aead_init(np))) {
		return ret;
	}

	/*
	 * Initialize the skcipher service.
	 */
	if ((ret = eip_crypto_skcipher_init(np))) {
		eip_crypto_aead_exit(np);
		return ret;
	}

	/*
	 * Initialize the ahash service.
	 */
	if ((ret = eip_crypto_ahash_init(np))) {
		eip_crypto_skcipher_exit(np);
		eip_crypto_aead_exit(np);
		return ret;
	}

	pr_info("%px: Crypto probe done.\n", pdev);
	return 0;
}

/*
 * eip_crypto_remove()
 *	Remove the eip crypto driver
 */
static void eip_crypto_remove(struct platform_device *pdev)
{
	struct device_node *np;

	/*
	 * Get the node corresponding to pdev.
	 */
	np = of_node_get(pdev->dev.of_node);
	if (!np) {
		pr_warn("%px: Invalid driver.\n", pdev);
		return;
	}

	/*
	 * De-initialize the crypto AHASH services.
	 */
	eip_crypto_ahash_exit(np);

	/*
	 * De-initialize the crypto SKCIPHER services.
	 */
	eip_crypto_skcipher_exit(np);

	/*
	 * De-initialize the crypto AEAD services.
	 */
	eip_crypto_aead_exit(np);

	pr_info("%px: Crypto remove done.\n", pdev);
}

/*
 * eip_crypto
 *	Platform device instance
 */
static struct platform_driver eip_crypto = {
	.probe = eip_crypto_probe,
	.remove = eip_crypto_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "eip-crypto",
		.of_match_table = of_match_ptr(eip_crypto_dt_ids),
	},
};

/*
 * eip_crypto_init()
 *	Initialize crypto client.
 */
int eip_crypto_init(void)
{
	int ret = 0;

	/*
	 * Check if crypto hardware is up.
	 */
	if (!eip_is_enabled()) {
		pr_info("Crypto hardware is not enabled.\n");
		return -1;
	}

	/*
	 * Register platform driver.
	 */
	ret = platform_driver_register(&eip_crypto);
	if (ret) {
		pr_warn("%px: Unable to register platform driver(%d).\n", &eip_crypto, ret);
		return ret;
	}

	pr_info("Crypto module loaded.\n");
	return 0;
}

/*
 * eip_crypto_exit()
 *	De-initialize crypto client.
 */
void eip_crypto_exit(void)
{
	platform_driver_unregister(&eip_crypto);
	pr_info("Crypto module unloaded.\n");
}

MODULE_LICENSE("Dual BSD/GPL");

module_init(eip_crypto_init);
module_exit(eip_crypto_exit);
