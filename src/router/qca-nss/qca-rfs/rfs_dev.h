/*
 * Copyright (c) 2014 - 2015, The Linux Foundation. All rights reserved.
 *
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
 */

/*
 * rfs_dev.h
 *	Receiving Flow Streering - device APIs
 */


/*
 * Load balance(ldb) and CPU ID
 * In QCA implementation, the value of load balance is equal to the CPU core ID.
 * EDMA implemented the mapping from a load balance to unique RX ring and user should
 * bind the IRQs to CPU cores through smp_affinity.
 */

typedef int (*rfs_dev_mac_rule_cb)(uint16_t vid, uint8_t *mac,
				   uint8_t ldb, int is_set);
typedef int (*rfs_dev_ip4_rule_cb)(uint16_t vid, uint32_t ipaddr,
				   uint8_t *mac, uint8_t ldb, int is_set);
typedef int (*rfs_dev_ip6_rule_cb)(uint16_t vid, uint8_t *ipaddr,
				   uint8_t *mac, uint8_t ldb, int is_set);

/*
 * struct rfs_device
 */
struct rfs_device {
	char *name; /*It must be a 'NULL' ended string*/
	rfs_dev_mac_rule_cb mac_rule_cb;
	rfs_dev_ip4_rule_cb ip4_rule_cb;
	rfs_dev_ip6_rule_cb ip6_rule_cb;
};


/*
 * rfs_ess_device_register
 * Return: 0 on success, others on failure
 */
int rfs_ess_device_register(struct rfs_device *dev);

/*
 * rfs_ess_device_unregister
 * Return: 0 on success, others on failure
 */
int rfs_ess_device_unregister(struct rfs_device *dev);
