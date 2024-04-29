/*
 **************************************************************************
 * Copyright (c) 2019-2020, The Linux Foundation.  All rights reserved.
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
#include <linux/etherdevice.h>

#include <ovsmgr.h>
#include "ecm_classifier_ovs_public.h"

/*
 * This is a test module for the ECM's ovs CLassifier.
 * It is extracting the VLAN information based on the flow information.
 */

/*
 * ecm_ovs_dump_flow()
 * 	Show OVS flow contents
 */
static void ecm_ovs_dump_flow(struct ovsmgr_dp_flow *flow)
{
	pr_debug("OVS DP Flow:\n");
	if (flow->indev) {
		pr_debug("\tindev: %s\n", flow->indev->name);
	} else {
		pr_debug("\tindev: NULL\n");
	}

	if (flow->outdev) {
		pr_debug("\toutdev: %s\n", flow->outdev->name);
	} else {
		pr_debug("\toutdev: NULL\n");
	}

	pr_debug("\tSMAC: %pM\n", &flow->smac);
	pr_debug("\tSMAC: %pM\n", &flow->dmac);
	pr_debug("\tingress_vlan: %d:%x\n", flow->ingress_vlan.h_vlan_TCI, flow->ingress_vlan.h_vlan_encapsulated_proto);
	pr_debug("\tTuple: V: %d, P: %d, SP: %d, DP: %d\n", flow->tuple.ip_version, flow->tuple.protocol,
			flow->tuple.src_port, flow->tuple.dst_port);

	if (flow->tuple.ip_version == 4) {
		pr_debug("\t\tSIP: %pI4, DIP: %pI4\n", &flow->tuple.ipv4.src, &flow->tuple.ipv4.dst);
	} else {
		pr_debug("\t\tSIP: %pI6, DIP: %pI6\n", &flow->tuple.ipv6.src, &flow->tuple.ipv6.dst);
	}
}

/*
 * ecm_ovs_get_ovs()
 *	OVS get callback function registered with ECM.
 */
static ecm_classifier_ovs_result_t ecm_ovs_process(struct ovsmgr_dp_flow *flow, struct sk_buff *skb, struct ecm_classifier_ovs_process_response *resp)
{
	struct ovsmgr_flow_info ofi;
	enum ovsmgr_flow_status status;

	pr_debug("ecm_ovs_process\n");

	memset((void *)&ofi, 0, sizeof(ofi));
	ecm_ovs_dump_flow(flow);

	status = ovsmgr_flow_info_get(flow, skb, &ofi);
	if ((status == OVSMGR_FLOW_STATUS_DENY_ACCEL) || (status == OVSMGR_FLOW_STATUS_UNKNOWN)) {
		pr_debug("%px: Deny accelerating the flow\n", flow);
		return ECM_CLASSIFIER_OVS_RESULT_DENY_ACCEL;
	}

	if (status == OVSMGR_FLOW_STATUS_DENY_ACCEL_EGRESS) {
		pr_debug("%px: Deny accelerating the flow, egress %s is not allowed\n", flow, flow->outdev->name);
		return ECM_CLASSIFIER_OVS_RESULT_DENY_ACCEL_EGRESS;
	}

	/*
	 * get return DSCP value for bridge flow
	 */
	if (!flow->is_routed) {
		struct ovsmgr_flow_info return_ofi;
		struct ovsmgr_dp_flow return_flow;
		enum ovsmgr_flow_status return_status;

		memset((void *)&return_ofi, 0, sizeof(return_ofi));

		/*
		 * Create return flow
		 */
		return_flow.indev = flow->outdev;
		return_flow.outdev = flow->indev;
		return_flow.tuple.ip_version = flow->tuple.ip_version;
		return_flow.tuple.protocol = flow->tuple.protocol;

		ether_addr_copy(return_flow.smac, flow->dmac);
		ether_addr_copy(return_flow.dmac, flow->smac);
		return_flow.ingress_vlan.h_vlan_TCI = ofi.egress[0].h_vlan_TCI;
		return_flow.ingress_vlan.h_vlan_encapsulated_proto = ofi.egress[0].h_vlan_encapsulated_proto;
		return_flow.tuple.src_port = flow->tuple.dst_port;
		return_flow.tuple.dst_port = flow->tuple.src_port;

		if (return_flow.tuple.ip_version == 4) {
			return_flow.tuple.ipv4.src = flow->tuple.ipv4.dst;
			return_flow.tuple.ipv4.dst = flow->tuple.ipv4.src;
		} else {
			memcpy(&return_flow.tuple.ipv6.src, &flow->tuple.ipv6.dst, sizeof(return_flow.tuple.ipv6.src));
			memcpy(&return_flow.tuple.ipv6.dst, &flow->tuple.ipv6.src, sizeof(return_flow.tuple.ipv6.dst));
		}

		resp->flow_dscp = ofi.dscp;
		ecm_ovs_dump_flow(&return_flow);

		return_status = ovsmgr_flow_info_get(&return_flow, skb, &return_ofi);
		if ((return_status == OVSMGR_FLOW_STATUS_DENY_ACCEL) || (return_status == OVSMGR_FLOW_STATUS_UNKNOWN)) {
			pr_debug("%px: Deny accelerating the return flow\n", &return_flow);
			goto process_flow;
		}

		resp->return_dscp = return_ofi.dscp;
	}

process_flow:
	resp->dscp = ofi.dscp;
	switch (status) {
	case OVSMGR_FLOW_STATUS_ALLOW_VLAN_ACCEL:
	case OVSMGR_FLOW_STATUS_ALLOW_VLAN_QINQ_ACCEL:
		pr_debug("%px: Accelerate, VLAN data is valid\n", flow);
		/*
		 * Outer ingress VLAN
		 */
		resp->ingress_vlan[0].h_vlan_TCI = ofi.ingress[0].h_vlan_TCI;
		resp->ingress_vlan[0].h_vlan_encapsulated_proto = ofi.ingress[0].h_vlan_encapsulated_proto;

		/*
		 * Outer egress VLAN
		 */
		resp->egress_vlan[0].h_vlan_TCI = ofi.egress[0].h_vlan_TCI;
		resp->egress_vlan[0].h_vlan_encapsulated_proto = ofi.egress[0].h_vlan_encapsulated_proto;

		if (status == OVSMGR_FLOW_STATUS_ALLOW_VLAN_QINQ_ACCEL) {
			/*
			 * Inner ingress VLAN
			 */
			resp->ingress_vlan[1].h_vlan_TCI = ofi.ingress[1].h_vlan_TCI;
			resp->ingress_vlan[1].h_vlan_encapsulated_proto = ofi.ingress[1].h_vlan_encapsulated_proto;

			/*
			 * Inner egress VLAN
			 */
			resp->egress_vlan[1].h_vlan_TCI = ofi.egress[1].h_vlan_TCI;
			resp->egress_vlan[1].h_vlan_encapsulated_proto = ofi.egress[1].h_vlan_encapsulated_proto;

			return ECM_CLASSIFIER_OVS_RESULT_ALLOW_VLAN_QINQ_ACCEL;
		}
		return ECM_CLASSIFIER_OVS_RESULT_ALLOW_VLAN_ACCEL;
	case OVSMGR_FLOW_STATUS_ALLOW_ACCEL:
		return ECM_CLASSIFIER_OVS_RESULT_ALLOW_ACCEL;
	default:
		return ECM_CLASSIFIER_OVS_RESULT_DENY_ACCEL;
	}
}

static struct ecm_classifier_ovs_callbacks callbacks = {
	.ovs_process = ecm_ovs_process,
};

/*
 * ecm_ovs_init()
 */
static int __init ecm_ovs_init(void)
{
	int res;

	pr_info("ECM OVS Test INIT\n");

	/*
	 * Register the callbacks with the ECM ovs classifier.
	 */
	res = ecm_classifier_ovs_register_callbacks(&callbacks);
	if (res < 0) {
		pr_warn("Failed to register callbacks for OVS classifier\n");
		return res;
	}

	return 0;
}

/*
 * ecm_ovs_exit()
 */
static void __exit ecm_ovs_exit(void)
{
	pr_info("ECM OVS Test EXIT\n");

	/*
	 * Unregister the callbacks.
	 */
	ecm_classifier_ovs_unregister_callbacks();
}

module_init(ecm_ovs_init)
module_exit(ecm_ovs_exit)

MODULE_DESCRIPTION("ECM OVS Test");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif
