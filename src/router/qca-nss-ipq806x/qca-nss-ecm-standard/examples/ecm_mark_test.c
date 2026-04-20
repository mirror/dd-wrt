/*
 **************************************************************************
 * Copyright (c) 2018, The Linux Foundation.  All rights reserved.
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
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/ctype.h>
#include <linux/if_pppox.h>

#include "ecm_classifier_mark_public.h"

/*
 * This is a test module for the ECM's mark CLassifier.
 * It is just extracting the PPPoE session ID from the skb and returns
 * it to the caller. The test module also receives IPv4 and IPv6 sync messages
 * from the classifer which includes the PPPoE session ID.
 */

/*
 * ecm_mark_test_sync_to_ipv6()
 *	IPv6 sync callback function registered with ECM.
 */
static void ecm_mark_test_sync_to_ipv6(uint32_t mark,
				       struct in6_addr *src_ip, int src_port,
				       struct in6_addr *dest_ip, int dest_port,
				       int protocol)
{
	pr_info("ecm_mark_test_sync_to_ipv6, mark: %d\n", (uint16_t)mark);

	/*
	 * Do whatever you wnat with this PPPoE session ID.
	 */
}

/*
 * ecm_mark_test_sync_to_ipv4()
 *	IPv4 sync callback function registered with ECM.
 */
static void ecm_mark_test_sync_to_ipv4(uint32_t mark,
				       __be32 src_ip, int src_port,
				       __be32 dest_ip, int dest_port,
				       int protocol)
{
	pr_info("ecm_mark_test_sync_to_ipv4, mark: %d\n", (uint16_t)mark);

	/*
	 * Do whatever you wnat with this PPPoE session ID.
	 */
}

/*
 * ecm_mark_test_get_mark()
 *	Mark get callback function registered with ECM.
 */
static ecm_classifier_mark_result_t ecm_mark_test_get_mark(struct sk_buff *skb, uint32_t *mark)
{
	struct pppoe_hdr *ph;

	/*
	 * If the protocol is not something we care about, we are not relevant to this packet.
	 * In this test module we inspect only the PPPoE session packets.
	 */
	if (ntohs(skb->protocol) != ETH_P_PPP_SES) {
		pr_info("ETH_P_PPP_SES: %d skb->protocol: %d\n", ETH_P_PPP_SES, ntohs(skb->protocol));
		return ECM_CLASSIFIER_MARK_RESULT_NOT_RELEVANT;
	}

	ph = pppoe_hdr(skb);
	pr_info("PPPoE session ID: %x\n", ntohs(ph->sid));
	*mark = ntohs(ph->sid);

	/*
	 * Inspection is success.
	 */
	return ECM_CLASSIFIER_MARK_RESULT_SUCCESS;
}

/*
 * ecm_mark_test_init()
 */
static int __init ecm_mark_test_init(void)
{
	int res;

	pr_info("ECM MARK Test INIT\n");

	/*
	 * Register the callbacks with the ECM mark classifier.
	 */
	res = ecm_classifier_mark_register_callbacks(ECM_CLASSIFIER_MARK_TYPE_L2_ENCAP,
					       ecm_mark_test_get_mark,
					       ecm_mark_test_sync_to_ipv4,
					       ecm_mark_test_sync_to_ipv6);
	if (res < 0) {
		pr_warn("Failed to register callbacks for L2_ENCAP type\n");
		return res;
	}

	return 0;
}

/*
 * ecm_mark_test_exit()
 */
static void __exit ecm_mark_test_exit(void)
{
	pr_info("ECM MARK Test EXIT\n");

	/*
	 * Unregister the callbacks.
	 */
	ecm_classifier_mark_unregister_callbacks(ECM_CLASSIFIER_MARK_TYPE_L2_ENCAP);
}

module_init(ecm_mark_test_init)
module_exit(ecm_mark_test_exit)

MODULE_DESCRIPTION("ECM MARK Test");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif

