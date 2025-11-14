/*
 ***************************************************************************
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024, Qualcomm Innovation Center, Inc. All rights reserved.
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
 ***************************************************************************
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/debugfs.h>
#include <linux/string.h>
#include <linux/netfilter_bridge.h>
#include <net/netfilter/nf_conntrack_core.h>
#include <net/ip.h>
#include <linux/inet.h>

/*
 * Debug output levels
 * 0 = OFF
 * 1 = ASSERTS / ERRORS
 * 2 = 1 + WARN
 * 3 = 2 + INFO
 * 4 = 3 + TRACE
 */
#define DEBUG_LEVEL ECM_CLASSIFIER_WIFI_DEBUG_LEVEL

#include "ecm_types.h"
#include "ecm_db_types.h"
#include "ecm_state.h"
#include "ecm_tracker.h"
#include "ecm_classifier.h"
#include "ecm_front_end_types.h"
#include "ecm_db.h"
#include "ecm_interface.h"
#include "ecm_front_end_ipv4.h"
#include "ecm_front_end_ipv6.h"
#include "ecm_front_end_common.h"
#include "ecm_classifier_wifi.h"
#include "ecm_classifier_wifi_public.h"

/*
 * Magic numbers
 */
#define ECM_CLASSIFIER_WIFI_INSTANCE_MAGIC 0xFE35

/*
 * struct ecm_classifier_wifi_instance
 */
struct ecm_classifier_wifi_instance {
	struct ecm_classifier_instance base;			/* Base type */

	struct ecm_classifier_wifi_instance *next;		/* Next classifier state instance (for accouting and reporting purposes) */
	struct ecm_classifier_wifi_instance *prev;		/* Prev classifier state instance (for accouting and reporting purposes) */

	uint32_t ci_serial;					/* RO: Serial of the connection */
	struct ecm_classifier_process_response process_response;/* Last process response computed */

	int refs;						/* Integer to trap we never go negative */

#if (DEBUG_LEVEL > 0)
	uint16_t magic;
#endif
};

/*
 * Operational control
 */
static uint32_t ecm_classifier_wifi_enabled = 1;			/* Operational behaviour */

/*
 * Management thread control
 */
static bool ecm_classifier_wifi_terminate_pending = false;	/* True when the user wants us to terminate */

/*
 * Locking of the classifier structures
 */
static DEFINE_SPINLOCK(ecm_classifier_wifi_lock);		/* Protect SMP access. */

/*
 * List of our classifier instances
 */
static struct ecm_classifier_wifi_instance *ecm_classifier_wifi_instances = NULL;
								/* list of all active instances */
static int ecm_classifier_wifi_count = 0;			/* Tracks number of instances allocated */

/*
 * Callback structure to support fetching wifi metadata
 */
static struct ecm_classifier_wifi_callbacks ecm_wifi;

/*
 * Debugfs dentry object.
 */
static struct dentry *ecm_classifier_wifi_dentry;

/*
 * ecm_classifier_wifi_type_get()
 *	Get type of classifier this is
 */
static ecm_classifier_type_t ecm_classifier_wifi_type_get(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_wifi_instance *cwifii;
	cwifii = (struct ecm_classifier_wifi_instance *)ci;

	DEBUG_CHECK_MAGIC(cwifii, ECM_CLASSIFIER_WIFI_INSTANCE_MAGIC, "%px: magic failed\n", cwifii);
	return ECM_CLASSIFIER_TYPE_WIFI;
}

/*
 * ecm_classifier_wifi_reclassify_allowed()
 *	Indicate if reclassify is allowed
 */
static bool ecm_classifier_wifi_reclassify_allowed(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_wifi_instance *cwifii;
	cwifii = (struct ecm_classifier_wifi_instance *)ci;
	DEBUG_CHECK_MAGIC(cwifii, ECM_CLASSIFIER_WIFI_INSTANCE_MAGIC, "%px: magic failed\n", cwifii);

	return true;
}

/*
 * ecm_classifier_wifi_reclassify()
 *	Reclassify
 */
static void ecm_classifier_wifi_reclassify(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_wifi_instance *cwifii;
	cwifii = (struct ecm_classifier_wifi_instance *)ci;
	DEBUG_CHECK_MAGIC(cwifii, ECM_CLASSIFIER_WIFI_INSTANCE_MAGIC, "%px: magic failed\n", cwifii);

	/*
	 * Revert back to MAYBE relevant - we will evaluate when we get the next process() call.
	 */
	spin_lock_bh(&ecm_classifier_wifi_lock);
	cwifii->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_MAYBE;
	spin_unlock_bh(&ecm_classifier_wifi_lock);
}

/*
 * ecm_classifier_wifi_fill_metadata()
 *	Save the wifi metadata in the classifier instance.
 */
static void ecm_classifier_wifi_fill_metadata(struct ecm_classifier_wifi_instance *cwifii, uint32_t flow_wifi_metadata, uint32_t flow_wifi_ds_node_id,
							uint32_t return_wifi_metadata, uint32_t return_wifi_ds_node_id, ecm_tracker_sender_type_t sender)
{
	spin_lock_bh(&ecm_classifier_wifi_lock);
	if (sender == ECM_TRACKER_SENDER_TYPE_SRC)
	{
		cwifii->process_response.process_actions |= (ECM_CLASSIFIER_PROCESS_ACTION_MARK | ECM_CLASSIFIER_PROCESS_ACTION_WIFI_TAG);
		cwifii->process_response.flow_mark = flow_wifi_metadata;
		cwifii->process_response.flow_wifi_ds_node_id = flow_wifi_ds_node_id;
		cwifii->process_response.return_mark = return_wifi_metadata;
		cwifii->process_response.return_wifi_ds_node_id = return_wifi_ds_node_id;
	} else {
		cwifii->process_response.process_actions |= (ECM_CLASSIFIER_PROCESS_ACTION_MARK | ECM_CLASSIFIER_PROCESS_ACTION_WIFI_TAG);
		cwifii->process_response.flow_mark = return_wifi_metadata;
		cwifii->process_response.flow_wifi_ds_node_id = return_wifi_ds_node_id;
		cwifii->process_response.return_mark = flow_wifi_metadata;
		cwifii->process_response.return_wifi_ds_node_id = flow_wifi_ds_node_id;
	}

	spin_unlock_bh(&ecm_classifier_wifi_lock);
}

/*
 * ecm_classifier_wifi_process()
 *	Process new data for connection
 */
static void ecm_classifier_wifi_process(struct ecm_classifier_instance *aci, ecm_tracker_sender_type_t sender,
						struct ecm_tracker_ip_header *ip_hdr, struct sk_buff *skb,
						struct ecm_classifier_process_response *process_response)
{
	struct ecm_classifier_wifi_instance *cwifii;
	ecm_classifier_relevence_t relevance;
	struct ecm_db_connection_instance *ci = NULL;
	struct ecm_front_end_connection_instance *feci;
	ecm_front_end_acceleration_mode_t accel_mode;
	uint32_t became_relevant = 0;
	uint32_t wifi_flow_metadata = 0;
	uint32_t wifi_return_metadata = 0;
	uint32_t return_ds_metadata = ECM_CLASSIFIER_WIFI_INVALID_DS_NODE_ID;
	uint32_t flow_ds_metadata = ECM_CLASSIFIER_WIFI_INVALID_DS_NODE_ID;
	int protocol;
	uint8_t dmac[ETH_ALEN];
	uint8_t smac[ETH_ALEN];
	struct net_device *src_dev = NULL;
	struct net_device *dest_dev = NULL;
	struct ecm_classifier_wifi_metadata wifi_metadata_info = {0};

	cwifii = (struct ecm_classifier_wifi_instance *)aci;
	DEBUG_CHECK_MAGIC(cwifii, ECM_CLASSIFIER_WIFI_INSTANCE_MAGIC, "%px: magic failed\n", cwifii);

	spin_lock_bh(&ecm_classifier_wifi_lock);
	relevance = cwifii->process_response.relevance;

	/*
	 * Are we relevant?
	 * If the classifier is set as ir-relevant to the connection,
	 * the process response of the classifier instance was set from
	 * the earlier packets.
	 */
	if (relevance == ECM_CLASSIFIER_RELEVANCE_NO) {
		/*
		 * Lock still held
		 */
		goto process_wifi_classifier_out;
	}

	/*
	 * Yes or maybe relevant.
	 *
	 * Need to decide our relevance to this connection.
	 * We are only relevant to a connection if:
	 * We are enabled.
	 * Any other condition and we are not and will stop analysing this connection.
	 */
	if (!ecm_classifier_wifi_enabled) {
		/*
		 * Lock still held
		 */
		cwifii->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
		goto process_wifi_classifier_out;
	}

	spin_unlock_bh(&ecm_classifier_wifi_lock);

	if (!ecm_wifi.get_wifi_metadata) {
		spin_lock_bh(&ecm_classifier_wifi_lock);
		DEBUG_WARN("%px: No callback registered to get metadata \n", cwifii);
		cwifii->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
		goto process_wifi_classifier_out;
	}

	/*
	 * Can we accelerate?
	 */
	ci = ecm_db_connection_serial_find_and_ref(cwifii->ci_serial);
	if (!ci) {
		DEBUG_TRACE("%px: No ci found for %u\n", cwifii, cwifii->ci_serial);
		spin_lock_bh(&ecm_classifier_wifi_lock);
		cwifii->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
		goto process_wifi_classifier_out;
	}

	feci = ecm_db_connection_front_end_get_and_ref(ci);
	accel_mode = ecm_front_end_connection_accel_state_get(feci);
	ecm_front_end_connection_deref(feci);
	ecm_db_connection_deref(ci);

	if (ECM_FRONT_END_ACCELERATION_NOT_POSSIBLE(accel_mode)) {
		spin_lock_bh(&ecm_classifier_wifi_lock);
		cwifii->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
		goto process_wifi_classifier_out;
	}

	/*
	 * Based on direction store the smac and dmac
	 */
	if (sender == ECM_TRACKER_SENDER_TYPE_SRC) {
		DEBUG_TRACE("%px: sender is SRC\n", aci);
		ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM, smac);
		ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_TO, dmac);
	} else {
		DEBUG_TRACE("%px: sender is DEST\n", aci);
		ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_TO, smac);
		ecm_db_connection_node_address_get(ci, ECM_DB_OBJ_DIR_FROM, dmac);
	}

	protocol = ecm_db_connection_protocol_get(ci);
	if ((protocol != IPPROTO_UDP) && (protocol != IPPROTO_TCP)) {
		/*
		 * TODO: Support this for GRE over WLAN.
		 */
		spin_lock_bh(&ecm_classifier_wifi_lock);
		cwifii->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
		goto process_wifi_classifier_out;
	}

	/*
	 * Fetch the src and dest net devices to check their interfaces.
	 */
	ecm_db_netdevs_get_and_hold(ci, sender, &src_dev, &dest_dev);

	if (dest_dev) {
		wifi_metadata_info.valid_params_flag |= ECM_CLASSIFIER_WIFI_MLO_PARAM_VALID;
		wifi_metadata_info.wifi_mdata.dest_dev = dest_dev;
		wifi_metadata_info.wifi_mdata.dest_mac = dmac;

		wifi_flow_metadata = ecm_wifi.get_wifi_metadata(&wifi_metadata_info);
		flow_ds_metadata = wifi_metadata_info.wifi_mdata.out_ppe_ds_node_id;
	}

	if (src_dev) {
		wifi_metadata_info.valid_params_flag |= ECM_CLASSIFIER_WIFI_MLO_PARAM_VALID;
		wifi_metadata_info.wifi_mdata.dest_dev = src_dev;
		wifi_metadata_info.wifi_mdata.dest_mac = smac;

		wifi_return_metadata = ecm_wifi.get_wifi_metadata(&wifi_metadata_info);
		return_ds_metadata = wifi_metadata_info.wifi_mdata.out_ppe_ds_node_id;
	}

	/*
	 * Make this classifier not relevant if neither DS node id nor wifi metadata is
	 * valid in any of the direction.
	 */
	if (!wifi_flow_metadata && !wifi_return_metadata && (flow_ds_metadata == ECM_CLASSIFIER_WIFI_INVALID_DS_NODE_ID)
			&& (return_ds_metadata == ECM_CLASSIFIER_WIFI_INVALID_DS_NODE_ID)) {
		spin_lock_bh(&ecm_classifier_wifi_lock);
		cwifii->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_NO;
		goto process_wifi_classifier_out;
	}

	ecm_classifier_wifi_fill_metadata(cwifii, wifi_flow_metadata, flow_ds_metadata, wifi_return_metadata, return_ds_metadata, sender);

	/*
	 * We are relevant to the connection.
	 * Set the process response to its default value, that is, to
	 * allow the acceleration.
	 */
	became_relevant = ecm_db_time_get();

	spin_lock_bh(&ecm_classifier_wifi_lock);
	cwifii->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_YES;
	cwifii->process_response.became_relevant = became_relevant;

	cwifii->process_response.process_actions |= ECM_CLASSIFIER_PROCESS_ACTION_ACCEL_MODE;
	cwifii->process_response.accel_mode = ECM_CLASSIFIER_ACCELERATION_MODE_ACCEL;

	DEBUG_TRACE("%px: flow mark: %x, return mark: %x, flow DS node id %d, return DS node id %d, sender %d\n",
			cwifii, cwifii->process_response.flow_mark, cwifii->process_response.return_mark,
			cwifii->process_response.flow_wifi_ds_node_id, cwifii->process_response.return_wifi_ds_node_id,
			sender);

process_wifi_classifier_out:

	/*
	 * Return our process response
	 * Release the source and destination dev count
	 */
	dev_put(src_dev);
	dev_put(dest_dev);

	*process_response = cwifii->process_response;
	spin_unlock_bh(&ecm_classifier_wifi_lock);
}

#ifdef ECM_STATE_OUTPUT_ENABLE
/*
 * ecm_classifier_wifi_state_get()
 *	Return state
 */
static int ecm_classifier_wifi_state_get(struct ecm_classifier_instance *ci, struct ecm_state_file_instance *sfi)
{
	int result;
	struct ecm_classifier_wifi_instance *cwifii;
	struct ecm_classifier_process_response process_response;
	uint32_t flow_wifi_ds_node_id;
	uint32_t return_wifi_ds_node_id;

	cwifii = (struct ecm_classifier_wifi_instance *)ci;
	DEBUG_CHECK_MAGIC(cwifii, ECM_CLASSIFIER_WIFI_INSTANCE_MAGIC, "%px: magic failed", cwifii);

	if ((result = ecm_state_prefix_add(sfi, "wifi"))) {
		return result;
	}

	spin_lock_bh(&ecm_classifier_wifi_lock);
	process_response = cwifii->process_response;
	flow_wifi_ds_node_id = cwifii->process_response.flow_wifi_ds_node_id;
	return_wifi_ds_node_id = cwifii->process_response.return_wifi_ds_node_id;
	spin_unlock_bh(&ecm_classifier_wifi_lock);

	if ((result = ecm_classifier_process_response_state_get(sfi, &process_response))) {
		return result;
	}

	if ((result = ecm_state_write(sfi, "flow_wifi_ds_node_id", "0x%x", flow_wifi_ds_node_id))) {
		return result;
	}
	if ((result = ecm_state_write(sfi, "return_wifi_ds_node_id", "0x%x", return_wifi_ds_node_id))) {
		return result;
	}

	return ecm_state_prefix_remove(sfi);
}
#endif

/*
 * ecm_classifier_wifi_sync_to_v4()
 *	Front end is pushing accel engine state to us
 */
static void ecm_classifier_wifi_sync_to_v4(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_sync *sync)
{
}

/*
 * ecm_classifier_wifi_sync_from_v4()
 *	Front end is retrieving accel engine state from us
 */
static void ecm_classifier_wifi_sync_from_v4(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_create *ecrc)
{
}

/*
 * ecm_classifier_wifi_sync_to_v6()
 *	Front end is pushing accel engine state to us
 */
static void ecm_classifier_wifi_sync_to_v6(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_sync *sync)
{
}

/*
 * ecm_classifier_wifi_sync_from_v6()
 *	Front end is retrieving accel engine state from us
 */
static void ecm_classifier_wifi_sync_from_v6(struct ecm_classifier_instance *aci, struct ecm_classifier_rule_create *ecrc)
{
}

/*
 * ecm_classifier_wifi_last_process_response_get()
 *	Get result code returned by the last process call
 */
static void ecm_classifier_wifi_last_process_response_get(struct ecm_classifier_instance *ci,
							struct ecm_classifier_process_response *process_response)
{
	struct ecm_classifier_wifi_instance *cwifii;

	cwifii = (struct ecm_classifier_wifi_instance *)ci;
	DEBUG_CHECK_MAGIC(cwifii, ECM_CLASSIFIER_WIFI_INSTANCE_MAGIC, "%px: magic failed\n", cwifii);

	spin_lock_bh(&ecm_classifier_wifi_lock);
	*process_response = cwifii->process_response;
	spin_unlock_bh(&ecm_classifier_wifi_lock);
}

/*
 * ecm_classifier_wifi_ref()
 *	Ref
 */
static void ecm_classifier_wifi_ref(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_wifi_instance *cwifii;
	cwifii = (struct ecm_classifier_wifi_instance *)ci;

	DEBUG_CHECK_MAGIC(cwifii, ECM_CLASSIFIER_WIFI_INSTANCE_MAGIC, "%px: magic failed\n", cwifii);
	spin_lock_bh(&ecm_classifier_wifi_lock);
	cwifii->refs++;
	DEBUG_ASSERT(cwifii->refs > 0, "%px: ref wrap\n", cwifii);
	DEBUG_TRACE("%px: cwifii ref %d\n", cwifii, cwifii->refs);
	spin_unlock_bh(&ecm_classifier_wifi_lock);
}

/*
 * ecm_classifier_wifi_deref()
 *	Deref
 */
static int ecm_classifier_wifi_deref(struct ecm_classifier_instance *ci)
{
	struct ecm_classifier_wifi_instance *cwifii;
	cwifii = (struct ecm_classifier_wifi_instance *)ci;
	DEBUG_CHECK_MAGIC(cwifii, ECM_CLASSIFIER_WIFI_INSTANCE_MAGIC, "%px: magic failed\n", cwifii);
	spin_lock_bh(&ecm_classifier_wifi_lock);
	cwifii->refs--;
	DEBUG_ASSERT(cwifii->refs >= 0, "%px: refs wrapped\n", cwifii);
	DEBUG_TRACE("%px: Wi-Fi classifier deref %d\n", cwifii, cwifii->refs);
	if (cwifii->refs) {
		int refs = cwifii->refs;
		spin_unlock_bh(&ecm_classifier_wifi_lock);
		return refs;
	}

	/*
	 * Object to be destroyed
	 */
	ecm_classifier_wifi_count--;
	DEBUG_ASSERT(ecm_classifier_wifi_count >= 0, "%px: ecm_classifier_wifi_count wrap\n", cwifii);

	/*
	 * UnLink the instance from our list
	 */
	if (cwifii->next) {
		cwifii->next->prev = cwifii->prev;
	}

	if (cwifii->prev) {
		cwifii->prev->next = cwifii->next;
	} else {
		DEBUG_ASSERT(ecm_classifier_wifi_instances == cwifii, "%px: list bad %px\n", cwifii, ecm_classifier_wifi_instances);
		ecm_classifier_wifi_instances = cwifii->next;
	}
	cwifii->prev = NULL;
	cwifii->next = NULL;
	spin_unlock_bh(&ecm_classifier_wifi_lock);

	/*
	 * Final
	 */
	DEBUG_INFO("%px: Final Wi-Fi classifier instance\n", cwifii);
	kfree(cwifii);

	return 0;
}

/*
 * ecm_classifier_wifi_instance_alloc()
 *	Allocate an instance of the wifi classifier
 */
struct ecm_classifier_wifi_instance *ecm_classifier_wifi_instance_alloc(struct ecm_db_connection_instance *ci)
{
	struct ecm_classifier_wifi_instance *cwifii;

	/*
	 * Allocate the instance
	 */
	cwifii = (struct ecm_classifier_wifi_instance *)kzalloc(sizeof(struct ecm_classifier_wifi_instance), GFP_ATOMIC | __GFP_NOWARN);
	if (!cwifii) {
		DEBUG_WARN("Failed to allocate wifi classifier instance\n");
		return NULL;
	}

	DEBUG_SET_MAGIC(cwifii, ECM_CLASSIFIER_WIFI_INSTANCE_MAGIC);
	cwifii->refs = 1;
	cwifii->base.process = ecm_classifier_wifi_process;
	cwifii->base.sync_from_v4 = ecm_classifier_wifi_sync_from_v4;
	cwifii->base.sync_to_v4 = ecm_classifier_wifi_sync_to_v4;
	cwifii->base.sync_from_v6 = ecm_classifier_wifi_sync_from_v6;
	cwifii->base.sync_to_v6 = ecm_classifier_wifi_sync_to_v6;
	cwifii->base.type_get = ecm_classifier_wifi_type_get;
	cwifii->base.last_process_response_get = ecm_classifier_wifi_last_process_response_get;
	cwifii->base.reclassify_allowed = ecm_classifier_wifi_reclassify_allowed;
	cwifii->base.reclassify = ecm_classifier_wifi_reclassify;
#ifdef ECM_STATE_OUTPUT_ENABLE
	cwifii->base.state_get = ecm_classifier_wifi_state_get;
#endif
	cwifii->base.ref = ecm_classifier_wifi_ref;
	cwifii->base.deref = ecm_classifier_wifi_deref;
	cwifii->ci_serial = ecm_db_connection_serial_get(ci);
	cwifii->process_response.process_actions = 0;
	cwifii->process_response.relevance = ECM_CLASSIFIER_RELEVANCE_MAYBE;

	spin_lock_bh(&ecm_classifier_wifi_lock);

	/*
	 * Final check if we are pending termination
	 */
	if (ecm_classifier_wifi_terminate_pending) {
		spin_unlock_bh(&ecm_classifier_wifi_lock);
		DEBUG_INFO("%px: Terminating\n", ci);
		kfree(cwifii);
		return NULL;
	}

	/*
	 * Link the new instance into our list at the head
	 */
	cwifii->next = ecm_classifier_wifi_instances;
	if (ecm_classifier_wifi_instances) {
		ecm_classifier_wifi_instances->prev = cwifii;
	}
	ecm_classifier_wifi_instances = cwifii;

	/*
	 * Increment stats
	 */
	ecm_classifier_wifi_count++;
	DEBUG_ASSERT(ecm_classifier_wifi_count > 0, "%px: ecm_classifier_wifi_count wrap\n", cwifii);
	spin_unlock_bh(&ecm_classifier_wifi_lock);

	DEBUG_INFO("Wi-Fi instance alloc: %px\n", cwifii);
	return cwifii;
}

/*
 * ecm_classifier_wifi_callback_register()
 */
int ecm_classifier_wifi_callback_register(struct ecm_classifier_wifi_callbacks *wifi_cb)
{
	spin_lock_bh(&ecm_classifier_wifi_lock);
	if (ecm_wifi.get_wifi_metadata) {
		spin_unlock_bh(&ecm_classifier_wifi_lock);
		DEBUG_ERROR("Wifi classifier callbacks are registered\n");
		return -1;
	}

	ecm_wifi.get_wifi_metadata = wifi_cb->get_wifi_metadata;
	spin_unlock_bh(&ecm_classifier_wifi_lock);
	return 0;
}
EXPORT_SYMBOL(ecm_classifier_wifi_callback_register);

/*
 * ecm_classifier_wifi_callback_unregister()
 */
void ecm_classifier_wifi_callback_unregister(void)
{
	spin_lock_bh(&ecm_classifier_wifi_lock);
	ecm_wifi.get_wifi_metadata = NULL;
	spin_unlock_bh(&ecm_classifier_wifi_lock);
}
EXPORT_SYMBOL(ecm_classifier_wifi_callback_unregister);

/*
 * ecm_classifier_wifi_init()
 */
int ecm_classifier_wifi_init(struct dentry *dentry)
{
	DEBUG_INFO("Wi-fi classifier Module init\n");
	ecm_classifier_wifi_dentry = debugfs_create_dir("ecm_classifier_wifi", dentry);

	if (!ecm_classifier_wifi_dentry) {
		DEBUG_ERROR("Failed to create ecm wifi directory in debugfs\n");
		return -1;
	}

	if (!ecm_debugfs_create_u32("enabled", S_IRUGO | S_IWUSR, ecm_classifier_wifi_dentry,
					(u32 *)&ecm_classifier_wifi_enabled)) {
		DEBUG_ERROR("Failed to create ecm wifi classifier enabled file in debugfs\n");
		debugfs_remove_recursive(ecm_classifier_wifi_dentry);
		return -1;
	}

	return 0;
}

/*
 * ecm_classifier_wifi_exit()
 */
void ecm_classifier_wifi_exit(void)
{
	DEBUG_INFO("Wi-Fi classifier Module exit\n");

	spin_lock_bh(&ecm_classifier_wifi_lock);
	ecm_classifier_wifi_terminate_pending = true;
	spin_unlock_bh(&ecm_classifier_wifi_lock);

	/*
	 * Remove the debugfs files recursively.
	 */
	if (ecm_classifier_wifi_dentry) {
		debugfs_remove_recursive(ecm_classifier_wifi_dentry);
	}
}
