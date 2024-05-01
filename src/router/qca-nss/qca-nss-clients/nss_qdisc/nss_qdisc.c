/*
 **************************************************************************
 * Copyright (c) 2014-2021 The Linux Foundation. All rights reserved.
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

#include <nss_api_if.h>
#include "nss_qdisc.h"
#include "nss_fifo.h"
#include "nss_codel.h"
#include "nss_tbl.h"
#include "nss_prio.h"
#include "nss_bf.h"
#include "nss_wrr.h"
#include "nss_wfq.h"
#include "nss_htb.h"
#include "nss_blackhole.h"
#include "nss_wred.h"

void *nss_qdisc_ctx;			/* Shaping context for nss_qdisc */

#define NSS_QDISC_COMMAND_TIMEOUT (10*HZ) /* We set 10sec to be the command */
					   /* timeout value for messages */

/*
 * Defines related to root hash maintenance
 */
#define NSS_QDISC_ROOT_HASH_SIZE 4
#define NSS_QDISC_ROOT_HASH_MASK (NSS_QDISC_ROOT_HASH_SIZE - 1)

/*
 * nss_qdisc_interface_is_virtual()
 *	Return true if it is redirect or bridge interface.
 */
bool nss_qdisc_interface_is_virtual(struct nss_ctx_instance *nss_ctx, int32_t if_num)
{
	/*
	 * If there is no bridge client, then bridge gets represented
	 * as a redirect interface. So this check is sufficient.
	 */
	bool is_virtual = nss_cmn_interface_is_redirect(nss_ctx, if_num) || nss_igs_verify_if_num(if_num);

#if defined(NSS_QDISC_BRIDGE_SUPPORT)
	is_virtual = is_virtual || nss_bridge_verify_if_num(if_num);
#endif
	return is_virtual;
}

#if defined(NSS_QDISC_PPE_SUPPORT)
/*
 * nss_qdisc_ppe_init()
 *	Initializes a shaper in PPE.
 */
static int nss_qdisc_ppe_init(struct Qdisc *sch, struct nss_qdisc *nq, nss_shaper_node_type_t type, uint32_t parent)
{
	int status = 0;

	/*
	 * Fallback to NSS Qdisc if PPE Qdisc configuration failed.
	 */
	if (nq->ppe_init_failed) {
		nss_qdisc_info("Qdisc %px (type %d) HW Qdisc initialization already tried, creating NSS Qdisc\n",
			nq->qdisc, nq->type);
		return 0;
	}

	/*
	 * Bridge and IFB needs PPE looback port shapers.
	 */
	if (nq->is_bridge || nss_igs_verify_if_num(nq->nss_interface_number)) {
		nss_qdisc_info("Qdisc %px (type %d) init qdisc: %px, needs PPE loopback port\n",
			nq->qdisc, nq->type, nq->qdisc);
		nq->needs_ppe_loopback = true;
	} else {
		nss_qdisc_info("Qdisc %px (type %d) init qdisc: %px, does not need PPE loopback port\n",
			nq->qdisc, nq->type, nq->qdisc);
		nq->needs_ppe_loopback = false;
	}

	/*
	 * Set the parent of PPE qdisc.
	 */
	status = nss_ppe_set_parent(sch, nq, parent);
	if (status == NSS_PPE_QDISC_PARENT_NOT_EXISTING) {
		nss_qdisc_info("HW qdisc/class %x cannot be attached to non-existing parent %x\n", nq->qos_tag, parent);
		return -1;
	} else if (status == NSS_PPE_QDISC_PARENT_NOT_PPE) {
		nss_qdisc_info("HW qdisc/class %x cannot be attached to NSS qdisc/class %x\n", nq->qos_tag, parent);
		return 0;
	}

	if (nss_ppe_init(sch, nq, type) < 0) {
			/*
			 * Class creation failures in PPE cannot use fallback to NSS-FW
			 * because classes under a qdisc usually share resources and that
			 * cannot happen across two data planes.
			 * Therefore fallback only applies to qdiscs.
			 */
			if (nq->is_class) {
			nss_qdisc_error("Qdisc %px (type %d) initializing HW class failed", nq->qdisc, nq->type);
			return -1;
		}
		nss_qdisc_info("Qdisc %px (type %d) initializing HW Qdisc failed, initializing NSS Qdisc \n",
			nq->qdisc, nq->type);
	} else {
		nss_qdisc_info("Qdisc %px (type %d) successfully created in PPE\n", nq->qdisc, nq->type);
	}

	return 0;
}
#endif

/*
 * nss_qdisc_msg_init()
 *      Initialize the qdisc specific message
 */
static void nss_qdisc_msg_init(struct nss_if_msg *nim, uint16_t if_num, uint32_t msg_type, uint32_t len,
				nss_if_msg_callback_t cb, void *app_data)
{
	nss_cmn_msg_init(&nim->cm, if_num, msg_type, len, (void*)cb, app_data);
}

/*
 * nss_qdisc_get_interface_msg()
 *	Returns the correct message that needs to be sent down to the NSS interface.
 */
static inline int nss_qdisc_get_interface_msg(bool is_bridge, uint32_t msg_type)
{
	/*
	 * We re-assign the message based on whether this is for the I shaper
	 * or the B shaper. The is_bridge flag tells if we are on a bridge interface.
	 */
	if (is_bridge) {
		switch(msg_type) {
		case NSS_QDISC_IF_SHAPER_ASSIGN:
			return NSS_IF_BSHAPER_ASSIGN;
		case NSS_QDISC_IF_SHAPER_UNASSIGN:
			return NSS_IF_BSHAPER_UNASSIGN;
		case NSS_QDISC_IF_SHAPER_CONFIG:
			return NSS_IF_BSHAPER_CONFIG;
		default:
			nss_qdisc_info("Unknown message type for a bridge - type %d", msg_type);
			return -1;
		}
	} else {
		switch(msg_type) {
		case NSS_QDISC_IF_SHAPER_ASSIGN:
			return NSS_IF_ISHAPER_ASSIGN;
		case NSS_QDISC_IF_SHAPER_UNASSIGN:
			return NSS_IF_ISHAPER_UNASSIGN;
		case NSS_QDISC_IF_SHAPER_CONFIG:
			return NSS_IF_ISHAPER_CONFIG;
		default:
			nss_qdisc_info("Unknown message type for an interface - type %d", msg_type);
			return -1;
		}
	}
}

/*
 * nss_qdisc_attach_bshaper_callback()
 *	Call back funtion for bridge shaper attach to an interface.
 */
static void nss_qdisc_attach_bshaper_callback(void *app_data, struct nss_if_msg *nim)
{
	struct Qdisc *sch = (struct Qdisc *)app_data;
	struct nss_qdisc *nq = qdisc_priv(sch);

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_warning("B-shaper attach FAILED - response: %d\n",
				nim->cm.error);
		atomic_set(&nq->state, NSS_QDISC_STATE_FAILED_RESPONSE);
		wake_up(&nq->wait_queue);
		return;
	}

	nss_qdisc_info("B-shaper attach SUCCESS\n");
	atomic_set(&nq->state, NSS_QDISC_STATE_READY);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_attach_bridge()
 *	Attaches a given bridge shaper to a given interface (Different from shaper_assign)
 */
static int nss_qdisc_attach_bshaper(struct Qdisc *sch, uint32_t if_num)
{
	struct nss_if_msg nim;
	struct nss_qdisc *nq = (struct nss_qdisc *)qdisc_priv(sch);
	int32_t state, rc;

	nss_qdisc_info("Attaching B-shaper %u to interface %u\n",
			nq->shaper_id, if_num);

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_warning("qdisc %px (type %d) is not ready: State - %d\n",
				sch, nq->type, state);
		return -1;
	}

	/*
	 * Set shaper node state to IDLE
	 */
	atomic_set(&nq->state, NSS_QDISC_STATE_IDLE);

	/*
	 * Populate the message and send it down
	 */
	nss_qdisc_msg_init(&nim, if_num, NSS_IF_BSHAPER_ASSIGN,
				sizeof(struct nss_if_shaper_assign),
				nss_qdisc_attach_bshaper_callback,
				sch);
	/*
	 * Assign the ID of the Bshaper that needs to be assigned to the interface recognized
	 * by if_num.
	 */
	nim.msg.shaper_assign.shaper_id = nq->shaper_id;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, &nim);

	if (rc != NSS_TX_SUCCESS) {
		nss_qdisc_warning("Failed to send bshaper (id: %u) attach for "
				"interface(if_num: %u)\n", nq->shaper_id, if_num);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	/*
	 * Wait until cleanup operation is complete at which point the state
	 * shall become non-idle.
	 */
	if (!wait_event_timeout(nq->wait_queue, atomic_read(&nq->state) != NSS_QDISC_STATE_IDLE,
				NSS_QDISC_COMMAND_TIMEOUT)) {
		nss_qdisc_error("bshaper attach command for %x on interface %u timedout!\n",
					nq->qos_tag, if_num);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_error("Failed to attach B-shaper %u to interface %u - state: %d\n",
				nq->shaper_id, if_num, state);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	nss_qdisc_info("Attach of B-shaper %u to interface %u is complete\n",
			nq->shaper_id, if_num);
	return 0;
}

/*
 * nss_qdisc_detach_bshaper_callback()
 *	Call back function for bridge shaper detach
 */
static void nss_qdisc_detach_bshaper_callback(void *app_data, struct nss_if_msg *nim)
{
	struct Qdisc *sch = (struct Qdisc *)app_data;
	struct nss_qdisc *nq = qdisc_priv(sch);

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_error("B-shaper detach FAILED - response: %d\n",
				nim->cm.error);
		atomic_set(&nq->state, NSS_QDISC_STATE_FAILED_RESPONSE);
		wake_up(&nq->wait_queue);
		return;
	}

	nss_qdisc_info("B-shaper detach SUCCESS\n");
	atomic_set(&nq->state, NSS_QDISC_STATE_READY);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_detach_bridge()
 *	Detaches a given bridge shaper from a given interface (different from shaper unassign)
 */
static int nss_qdisc_detach_bshaper(struct Qdisc *sch, uint32_t if_num)
{
	struct nss_if_msg nim;
	struct nss_qdisc *nq = (struct nss_qdisc *)qdisc_priv(sch);
	int32_t state, rc;

	nss_qdisc_info("Detaching B-shaper %u from interface %u\n",
			nq->shaper_id, if_num);

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_warning("qdisc %px (type %d) is not ready: %d\n",
				sch, nq->type, state);
		return -1;
	}

	/*
	 * Set shaper node state to IDLE
	 */
	atomic_set(&nq->state, NSS_QDISC_STATE_IDLE);

	/*
	 * Create and send shaper unassign message to the NSS interface
	 */
	nss_qdisc_msg_init(&nim, if_num, NSS_IF_BSHAPER_UNASSIGN,
				sizeof(struct nss_if_shaper_unassign),
				nss_qdisc_detach_bshaper_callback,
				sch);
	nim.msg.shaper_unassign.shaper_id = nq->shaper_id;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, &nim);

	if (rc != NSS_TX_SUCCESS) {
		nss_qdisc_warning("Failed to send B-shaper (id: %u) detach "
			"for interface(if_num: %u)\n", nq->shaper_id, if_num);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	/*
	 * Wait until cleanup operation is complete at which point the state
	 * shall become non-idle.
	 */
	if (!wait_event_timeout(nq->wait_queue, atomic_read(&nq->state) != NSS_QDISC_STATE_IDLE,
				NSS_QDISC_COMMAND_TIMEOUT)) {
		nss_qdisc_error("bshaper detach command for %x on interface %u timedout!\n",
					nq->qos_tag, if_num);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_error("Failed to detach B-shaper %u from interface %u - state %d\n",
				nq->shaper_id, if_num, state);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	nss_qdisc_info("Detach of B-shaper %u to interface %u is complete.",
			nq->shaper_id, if_num);

	return 0;
}

/*
 * nss_qdisc_refresh_bshaper_assignment()
 *	Performs assign on unassign of bshaper for interfaces on the bridge.
 */
static int nss_qdisc_refresh_bshaper_assignment(struct Qdisc *br_qdisc,
					enum nss_qdisc_bshaper_tasks task)
{
	struct net_device *dev;
	struct net_device *br_dev = qdisc_dev(br_qdisc);
	struct nss_qdisc *nq;
	struct nss_qdisc_bridge_update br_update;
	int i;

	if ((br_qdisc->parent != TC_H_ROOT) && (br_qdisc->parent != TC_H_UNSPEC)) {
		nss_qdisc_error("Qdisc not root qdisc for the bridge interface: "
				"Handle - %x", br_qdisc->parent);
		return -1;
	}

	nq = qdisc_priv(br_qdisc);

	/*
	 * Initialize the bridge update srtucture.
	 */
	br_update.port_list_count = 0;
	br_update.unassign_count = 0;

	read_lock(&dev_base_lock);
	dev = first_net_device(&init_net);

	while(dev) {
		struct net_bridge_port *br_port;
		int nss_if_num;

		nss_qdisc_info("Scanning device %s", dev->name);

		rcu_read_lock();
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 3, 0))
		br_port = br_port_get_rcu(dev);
#else
		br_port = rcu_dereference(dev->br_port);
#endif

		if (!br_port || !br_port->br) {
			rcu_read_unlock();
			goto nextdev;
		}

		/*
		 * Dont care if this device is not on the
		 * bridge that is of concern.
		 */
		if (br_port->br->dev != br_dev) {
			rcu_read_unlock();
			goto nextdev;
		}

		rcu_read_unlock();

		/*
		 * If the interface is known to NSS then we will have to shape it.
		 * Irrespective of whether it has an interface qdisc or not.
		 */
		nss_if_num = nss_cmn_get_interface_number(nq->nss_shaping_ctx, dev);
		if (nss_if_num < 0) {
			goto nextdev;
		}

		nss_qdisc_info("Will be linking/unlinking %s to/from bridge %s\n",
						dev->name, br_dev->name);
		br_update.port_list[br_update.port_list_count++] = nss_if_num;
nextdev:
		dev = next_net_device(dev);
	}
	read_unlock(&dev_base_lock);

	nss_qdisc_info("List count %d\n", br_update.port_list_count);

	if (task == NSS_QDISC_SCAN_AND_ASSIGN_BSHAPER) {
		/*
		 * Loop through the ports and assign them with B-shapers.
		 */
		for (i = 0; i < br_update.port_list_count; i++) {
			if (nss_qdisc_attach_bshaper(br_qdisc, br_update.port_list[i]) >= 0) {
				nss_qdisc_info("Interface %u added to bridge %s\n",
					br_update.port_list[i], br_dev->name);
				continue;
			}
			nss_qdisc_error("Unable to attach bshaper with shaper-id: %u, "
				"to interface if_num: %d\n", nq->shaper_id,
				br_update.port_list[i]);
			br_update.unassign_count = i;
			break;
		}
		nss_qdisc_info("Unassign count %d\n", br_update.unassign_count);
		if (br_update.unassign_count == 0) {
			return 0;
		}

		/*
		 * In case of a failure, unassign the B-shapers that were assigned above
		 */
		for (i = 0; i < br_update.unassign_count; i++) {
			if (nss_qdisc_detach_bshaper(br_qdisc, br_update.port_list[i]) >= 0) {
				continue;
			}
			nss_qdisc_error("Unable to detach bshaper with shaper-id: %u, "
				"from interface if_num: %d\n", nq->shaper_id,
				br_update.port_list[i]);
		}

		nss_qdisc_info("Failed to link interfaces to bridge\n");
		return -1;
	} else if (task == NSS_QDISC_SCAN_AND_UNASSIGN_BSHAPER) {
		/*
		 * Loop through the ports and assign them with B-shapers.
		 */
		for (i = 0; i < br_update.port_list_count; i++) {
			if (nss_qdisc_detach_bshaper(br_qdisc, br_update.port_list[i]) >= 0) {
				nss_qdisc_info("Interface %u removed from bridge %s\n",
					br_update.port_list[i], br_dev->name);
				continue;
			}
			nss_qdisc_error("Unable to detach bshaper with shaper-id: %u, "
				"from interface if_num: %d\n", nq->shaper_id,
				br_update.port_list[i]);
		}
	}

	return 0;
}

/*
 * nss_qdisc_root_cleanup_final()
 *	Performs final cleanup of a root shaper node after all other
 *	shaper node cleanup is complete.
 */
static void nss_qdisc_root_cleanup_final(struct nss_qdisc *nq)
{
	nss_qdisc_info("Root qdisc %px (type %d) final cleanup\n",
				nq->qdisc, nq->type);

	/*
	 * If we are a bridge then we have to unregister for bridge bouncing
	 * AND destroy the virtual interface that provides bridge shaping.
	 */
	if (nq->is_bridge) {
		/*
		 * Unregister for bouncing to the NSS for bridge shaping
		 */
		nss_qdisc_info("Unregister for bridge bouncing: %px\n",
				nq->bounce_context);
		nss_shaper_unregister_shaper_bounce_bridge(nq->nss_interface_number);

		/*
		 * Unregister the virtual interface we use to act as shaper
		 * for bridge shaping.
		 */
		nss_qdisc_info("Release root bridge virtual interface: %px\n",
				nq->virt_if_ctx);
	}

	/*
	 * If we are a virual interface other than a bridge then we simply
	 * unregister for interface bouncing and not care about deleting the
	 * interface.
	 */
	if (nq->is_virtual && !nq->is_bridge) {
		/*
		 * Unregister for interface bouncing of packets
		 */
		nss_qdisc_info("Unregister for interface bouncing: %px\n",
				nq->bounce_context);
		nss_shaper_unregister_shaper_bounce_interface(nq->nss_interface_number);
	}

	/*
	 * Finally unregister for shaping
	 */
	nss_qdisc_info("Unregister for shaping\n");
	nss_shaper_unregister_shaping(nq->nss_shaping_ctx);

	/*
	 * Now set our final state and wake up the caller
	 */
	atomic_set(&nq->state, nq->pending_final_state);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_root_cleanup_shaper_unassign_callback()
 *	Invoked on the response to a shaper unassign config command issued
 */
static void nss_qdisc_root_cleanup_shaper_unassign_callback(void *app_data,
							struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;
	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_error("Root qdisc %px (type %d) shaper unsassign FAILED\n", nq->qdisc, nq->type);
		atomic_set(&nq->state, NSS_QDISC_STATE_UNASSIGN_SHAPER_FAIL);
		wake_up(&nq->wait_queue);
		return;
	}

	nss_qdisc_root_cleanup_final(nq);
}

/*
 * nss_qdisc_root_cleanup_shaper_unassign()
 *	Issue command to unassign the shaper
 */
static void nss_qdisc_root_cleanup_shaper_unassign(struct nss_qdisc *nq)
{
	struct nss_if_msg nim;
	nss_tx_status_t rc;
	int msg_type;

	nss_qdisc_info("Root qdisc %px (type %d): shaper unassign: %d\n",
			nq->qdisc, nq->type, nq->shaper_id);

	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_UNASSIGN);
	nss_qdisc_msg_init(&nim, nq->nss_interface_number, msg_type,
			sizeof(struct nss_if_shaper_unassign),
			nss_qdisc_root_cleanup_shaper_unassign_callback,
			nq);
	nim.msg.shaper_unassign.shaper_id = nq->shaper_id;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, &nim);

	if (rc == NSS_TX_SUCCESS) {
		/*
		 * Tx successful, simply return.
		 */
		return;
	}

	nss_qdisc_error("Root qdisc %px (type %d): unassign command send failed: "
		"%d, shaper id: %d\n", nq->qdisc, nq->type, rc, nq->shaper_id);

	atomic_set(&nq->state, NSS_QDISC_STATE_UNASSIGN_SHAPER_SEND_FAIL);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_root_cleanup_free_node_callback()
 *	Invoked on the response to freeing a shaper node
 */
static void nss_qdisc_root_cleanup_free_node_callback(void *app_data,
						struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;
	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_error("Root qdisc %px (type %d) free FAILED response "
					"type: %d\n", nq->qdisc, nq->type,
					nim->msg.shaper_configure.config.response_type);
		atomic_set(&nq->state, NSS_QDISC_STATE_NODE_FREE_FAIL);
		wake_up(&nq->wait_queue);
		return;
	}

	nss_qdisc_info("Root qdisc %px (type %d) free SUCCESS - response "
			"type: %d\n", nq->qdisc, nq->type,
			nim->msg.shaper_configure.config.response_type);

	nss_qdisc_root_cleanup_shaper_unassign(nq);
}

/*
 * nss_qdisc_root_cleanup_free_node()
 *	Free the shaper node, issue command to do so.
 */
static void nss_qdisc_root_cleanup_free_node(struct nss_qdisc *nq)
{
	struct nss_if_msg nim;
	nss_tx_status_t rc;
	int msg_type;

	nss_qdisc_info("Root qdisc %px (type %d): freeing shaper node\n",
			nq->qdisc, nq->type);

	/*
	 * Construct and send the shaper configure message down to the NSS interface
	 */
	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_CONFIG);
	nss_qdisc_msg_init(&nim, nq->nss_interface_number, msg_type,
				sizeof(struct nss_if_shaper_configure),
				nss_qdisc_root_cleanup_free_node_callback,
				nq);
	nim.msg.shaper_configure.config.request_type = NSS_SHAPER_CONFIG_TYPE_FREE_SHAPER_NODE;
	nim.msg.shaper_configure.config.msg.free_shaper_node.qos_tag = nq->qos_tag;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, &nim);

	if (rc == NSS_TX_SUCCESS) {
		/*
		 * Tx successful, simply return.
		 */
		return;
	}

	nss_qdisc_error("Qdisc %px (type %d): free command send "
		"failed: %d, qos tag: %x\n", nq->qdisc, nq->type,
		rc, nq->qos_tag);

	atomic_set(&nq->state, NSS_QDISC_STATE_NODE_FREE_SEND_FAIL);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_root_init_root_assign_callback()
 *	Invoked on the response to assigning shaper node as root
 */
static void nss_qdisc_root_init_root_assign_callback(void *app_data,
						struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_warning("Root assign FAILED for qdisc %px (type %d), "
			"response type: %d\n", nq->qdisc, nq->type,
			nim->msg.shaper_configure.config.response_type);
		nq->pending_final_state = NSS_QDISC_STATE_ROOT_SET_FAIL;
		nss_qdisc_root_cleanup_free_node(nq);
		return;
	}

	nss_qdisc_info("Qdisc %px (type %d): set as root is done. Response - %d"
			, nq->qdisc, nq->type, nim->msg.shaper_configure.config.response_type);
	atomic_set(&nq->state, NSS_QDISC_STATE_READY);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_root_init_alloc_node_callback()
 *	Invoked on the response to creating a shaper node as root
 */
static void nss_qdisc_root_init_alloc_node_callback(void *app_data,
						struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;
	nss_tx_status_t rc;
	int msg_type;

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_info("Qdisc %px (type %d) root alloc node FAILED "
			"response type: %d\n", nq->qdisc, nq->type,
			nim->msg.shaper_configure.config.response_type);

		nq->pending_final_state = NSS_QDISC_STATE_NODE_ALLOC_FAIL;

		/*
		 * No shaper node created, cleanup from unsassigning the shaper
		 */
		nss_qdisc_root_cleanup_shaper_unassign(nq);
		return;
	}

	nss_qdisc_info("Qdisc %px (type %d), shaper node alloc success: %u\n",
				nq->qdisc, nq->type, nq->shaper_id);

	/*
	 * Create and send shaper configure message to the NSS interface
	 */
	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_CONFIG);
	nss_qdisc_msg_init(nim, nq->nss_interface_number, msg_type,
				sizeof(struct nss_if_shaper_configure),
				nss_qdisc_root_init_root_assign_callback,
				nq);
	nim->msg.shaper_configure.config.request_type = NSS_SHAPER_CONFIG_TYPE_SET_ROOT;
	nim->msg.shaper_configure.config.msg.set_root_node.qos_tag = nq->qos_tag;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, nim);

	if (rc == NSS_TX_SUCCESS) {
		return;
	}

	nss_qdisc_warning("Root assign send command failed: %d\n",
			rc);

	nq->pending_final_state = NSS_QDISC_STATE_ROOT_SET_SEND_FAIL;
	nss_qdisc_root_cleanup_free_node(nq);
}

/*
 * nss_qdisc_root_init_shaper_assign_callback()
 *	Invoked on the response to a shaper assign config command issued
 */
static void nss_qdisc_root_init_shaper_assign_callback(void *app_data,
						struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;
	nss_tx_status_t rc;
	int msg_type;

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_warning("Qdisc %x (type %d): shaper assign failed - phys_if response type: %d\n",
			nq->qos_tag, nq->type, nim->cm.error);
		/*
		 * Unable to assign a shaper, perform cleanup from final stage
		 */
		nq->pending_final_state = NSS_QDISC_STATE_SHAPER_ASSIGN_FAILED;
		nss_qdisc_root_cleanup_final(nq);
		return;
	}

	if (nim->cm.type != NSS_IF_ISHAPER_ASSIGN && nim->cm.type != NSS_IF_BSHAPER_ASSIGN) {
		nss_qdisc_error("Qdisc %x (type %d): shaper assign callback received garbage: %d\n",
			nq->qos_tag, nq->type, nim->cm.type);
		return;
	}

	nss_qdisc_info("Qdisc %x (type %d): shaper assign callback received sane message: %d\n",
		nq->qos_tag, nq->type, nim->cm.type);

	/*
	 * Shaper has been allocated and assigned
	 */
	nq->shaper_id = nim->msg.shaper_assign.new_shaper_id;
	nss_qdisc_info("Qdisc %px (type %d), shaper assigned: %u\n",
				nq->qdisc, nq->type, nq->shaper_id);

	/*
	 * Create and send the shaper configure message to the NSS interface
	 */
	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_CONFIG);
	nss_qdisc_msg_init(nim, nq->nss_interface_number, msg_type, sizeof(struct nss_if_shaper_configure),
				nss_qdisc_root_init_alloc_node_callback,
				nq);
	nim->msg.shaper_configure.config.request_type = NSS_SHAPER_CONFIG_TYPE_ALLOC_SHAPER_NODE;
	nim->msg.shaper_configure.config.msg.alloc_shaper_node.node_type = nq->type;
	nim->msg.shaper_configure.config.msg.alloc_shaper_node.qos_tag = nq->qos_tag;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, nim);

	if (rc == NSS_TX_SUCCESS) {
		return;
	}

	/*
	 * Unable to send alloc node command, cleanup from unassigning the shaper
	 */
	nss_qdisc_warning("Qdisc %px (type %d) create command failed: %d\n",
			nq->qdisc, nq->type, rc);

	nq->pending_final_state = NSS_QDISC_STATE_NODE_ALLOC_SEND_FAIL;
	nss_qdisc_root_cleanup_shaper_unassign(nq);
}

/*
 * nss_qdisc_child_cleanup_final()
 *	Perform final cleanup of a shaper node after all shaper node
 *	cleanup is complete.
 */
static void nss_qdisc_child_cleanup_final(struct nss_qdisc *nq)
{
	nss_qdisc_info("Final cleanup type %d: %px\n",
			nq->type, nq->qdisc);

	/*
	 * Finally unregister for shaping
	 */
	nss_qdisc_info("Unregister for shaping\n");
	nss_shaper_unregister_shaping(nq->nss_shaping_ctx);

	/*
	 * Now set our final state
	 */
	atomic_set(&nq->state, nq->pending_final_state);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_child_cleanup_free_node_callback()
 *	Invoked on the response to freeing a child shaper node
 */
static void nss_qdisc_child_cleanup_free_node_callback(void *app_data,
						struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_error("Qdisc %px (type %d qos_tag %x): child free FAILED response type: %d\n",
			nq->qdisc, nq->type, nq->qos_tag, nim->msg.shaper_configure.config.response_type);
		atomic_set(&nq->state, NSS_QDISC_STATE_NODE_FREE_FAIL);
		wake_up(&nq->wait_queue);
		return;
	}

	nss_qdisc_info("Qdisc %px (type %d): child shaper node "
			"free complete\n", nq->qdisc, nq->type);

	/*
	 * Perform final cleanup
	 */
	nss_qdisc_child_cleanup_final(nq);
}

/*
 * nss_qdisc_child_cleanup_free_node()
 *	Free the child shaper node, issue command to do so.
 */
static void nss_qdisc_child_cleanup_free_node(struct nss_qdisc *nq)
{
	struct nss_if_msg nim;
	nss_tx_status_t rc;
	int msg_type;

	nss_qdisc_info("Qdisc %px (type %d qos_tag %x): free shaper node command\n",
			nq->qdisc, nq->type, nq->qos_tag);

	/*
	 * Create and send the shaper configure message to the NSS interface
	 */
	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_CONFIG);
	nss_qdisc_msg_init(&nim, nq->nss_interface_number, msg_type, sizeof(struct nss_if_shaper_configure),
				nss_qdisc_child_cleanup_free_node_callback,
				nq);
	nim.msg.shaper_configure.config.request_type = NSS_SHAPER_CONFIG_TYPE_FREE_SHAPER_NODE;
	nim.msg.shaper_configure.config.msg.free_shaper_node.qos_tag = nq->qos_tag;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, &nim);

	if (rc == NSS_TX_SUCCESS) {
		return;
	}

	nss_qdisc_error("Qdisc %px (type %d): child free node command send "
			"failed: %d, qos tag: %x\n", nq->qdisc, nq->type,
			rc, nq->qos_tag);

	atomic_set(&nq->state, NSS_QDISC_STATE_NODE_FREE_SEND_FAIL);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_child_init_alloc_node_callback()
 *	Invoked on the response to creating a child shaper node
 */
static void nss_qdisc_child_init_alloc_node_callback(void *app_data, struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_error("Qdisc %px (type %d): child alloc node FAILED, response "
			"type: %d\n", nq->qdisc, nq->type, nim->msg.shaper_configure.config.response_type);
		/*
		 * Cleanup from final stage
		 */
		nq->pending_final_state = NSS_QDISC_STATE_NODE_ALLOC_FAIL_CHILD;
		nss_qdisc_child_cleanup_final(nq);
		return;
	}

	/*
	 * Shaper node has been allocated
	 */
	nss_qdisc_info("Qdisc %px (type %d): shaper node successfully "
			"created as a child node\n", nq->qdisc, nq->type);

	atomic_set(&nq->state, NSS_QDISC_STATE_READY);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_add_to_tail_protected()
 *	Adds to list while holding the qdisc lock.
 */
static inline void nss_qdisc_add_to_tail_protected(struct sk_buff *skb, struct Qdisc *sch)
{
	struct nss_qdisc *nq = qdisc_priv(sch);

	/*
	 * Since packets can come back from the NSS at any time (in case of bounce),
	 * enqueue's and dequeue's can cause corruption, if not done within locks.
	 */
	spin_lock_bh(&nq->bounce_protection_lock);

	/*
	 * We do not use the qdisc_enqueue_tail() API here in order
	 * to prevent stats from getting updated by the API.
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	__skb_queue_tail(&sch->q, skb);
#else
	__qdisc_enqueue_tail(skb, &sch->q);
#endif

	spin_unlock_bh(&nq->bounce_protection_lock);
};

/*
 * nss_qdisc_add_to_tail()
 *	Adds to list without holding any locks.
 */
static inline void nss_qdisc_add_to_tail(struct sk_buff *skb, struct Qdisc *sch)
{
	/*
	 * We do not use the qdisc_enqueue_tail() API here in order
	 * to prevent stats from getting updated by the API.
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	__skb_queue_tail(&sch->q, skb);
#else
	__qdisc_enqueue_tail(skb, &sch->q);
#endif
};

/*
 * nss_qdisc_remove_from_tail_protected()
 *	Removes from list while holding the qdisc lock.
 */
static inline struct sk_buff *nss_qdisc_remove_from_tail_protected(struct Qdisc *sch)
{
	struct nss_qdisc *nq = qdisc_priv(sch);
	struct sk_buff *skb;

	/*
	 * Since packets can come back from the NSS at any time (in case of bounce),
	 * enqueue's and dequeue's can cause corruption, if not done within locks.
	 */
	spin_lock_bh(&nq->bounce_protection_lock);

	/*
	 * We use __skb_dequeue() to ensure that
	 * stats don't get updated twice.
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0))
	skb = __skb_dequeue(&sch->q);
#else
	skb = __qdisc_dequeue_head(&sch->q);
#endif
	spin_unlock_bh(&nq->bounce_protection_lock);
	return skb;
};

/*
 * nss_qdisc_remove_to_tail_protected()
 *	Removes from list without holding any locks.
 */
static inline struct sk_buff *nss_qdisc_remove_from_tail(struct Qdisc *sch)
{
	/*
	 * We use __skb_dequeue() to ensure that
	 * stats don't get updated twice.
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0))
	return __skb_dequeue(&sch->q);
#else
	return __qdisc_dequeue_head(&sch->q);
#endif
};

/*
 * nss_qdisc_bounce_callback()
 *	Enqueues packets bounced back from NSS firmware.
 */
static void nss_qdisc_bounce_callback(void *app_data, struct sk_buff *skb)
{
	struct Qdisc *sch = (struct Qdisc *)app_data;

	/*
	 * Enqueue the packet for transmit and schedule a dequeue
	 * This enqueue has to be protected in order to avoid corruption.
	 */
	nss_qdisc_add_to_tail_protected(skb, sch);
	__netif_schedule(sch);
}

/*
 * nss_qdisc_mark_and_schedule()
 *	Mark the classid in packet and enqueue it.
 */
static void nss_qdisc_mark_and_schedule(void *app_data, struct sk_buff *skb)
{
	struct Qdisc *sch = (struct Qdisc *)app_data;
	uint16_t temp = TC_H_MAJ(skb->priority) >> 16;

	/*
	 * Restore the priority field of skb to its value before bouncing.
	 * Before bounce, we saved the priority value to tc_index field.
	 *
	 * Save the qostag of shaped packet to tc_index field.
	 */
	skb->priority = (skb->tc_index << 16) | TC_H_MIN(skb->priority);
	skb->tc_index = temp;

	/*
	 * Enqueue the packet for transmit and schedule a dequeue
	 * This enqueue has to be protected in order to avoid corruption.
	 */
	nss_qdisc_add_to_tail_protected(skb, sch);
	__netif_schedule(sch);
}

/*
 * nss_qdisc_replace()
 *	Used to replace old qdisc with a new qdisc.
 */
struct Qdisc *nss_qdisc_replace(struct Qdisc *sch, struct Qdisc *new,
					  struct Qdisc **pold)
{
	/*
	 * The qdisc_replace() API is originally introduced in kernel version 4.6,
	 * however this has been back ported to the 4.4. kernal used in QSDK.
	 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0))
	return qdisc_replace(sch, new, pold);
#else
	struct Qdisc *old;

	sch_tree_lock(sch);
	old = *pold;
	*pold = new;
	if (old != NULL) {
		qdisc_tree_decrease_qlen(old, old->q.qlen);
		qdisc_reset(old);
	}
	sch_tree_unlock(sch);

	return old;
#endif
}

/*
 * nss_qdisc_qopt_get()
 *	Extracts qopt from opt.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
void *nss_qdisc_qopt_get(struct nlattr *opt, struct nla_policy *policy,
				struct nlattr *tb[], uint32_t tca_max, uint32_t tca_params)
#else
void *nss_qdisc_qopt_get(struct nlattr *opt, struct nla_policy *policy,
				struct nlattr *tb[], uint32_t tca_max, uint32_t tca_params, struct netlink_ext_ack *extack)
#endif
{
	int err;

	if (!opt) {
		return NULL;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
	err = nla_parse_nested(tb, tca_max, opt, policy);
#else
	err = nla_parse_nested_deprecated(tb, tca_max, opt, policy, extack);
#endif

	if (err < 0)
		return NULL;

	if (tb[tca_params] == NULL)
		return NULL;

	return nla_data(tb[tca_params]);
}

/*
 * nss_qdisc_mode_get()
 *	Returns the operating mode of nss_qdisc, 0 = nss-fw, 1 = ppe.
 */
uint8_t nss_qdisc_accel_mode_get(struct nss_qdisc *nq)
{
	if (nq->mode == NSS_QDISC_MODE_PPE) {
		return TCA_NSS_ACCEL_MODE_PPE;
	}

	return TCA_NSS_ACCEL_MODE_NSS_FW;
}

/*
 * nss_qdisc_peek()
 *	Called to peek at the head of an nss qdisc
 */
struct sk_buff *nss_qdisc_peek(struct Qdisc *sch)
{
	struct nss_qdisc *nq = qdisc_priv(sch);
	struct sk_buff *skb;

	if (!nq->is_virtual) {
		skb = qdisc_peek_head(sch);
	} else {
		spin_lock_bh(&nq->bounce_protection_lock);
		skb = qdisc_peek_head(sch);
		spin_unlock_bh(&nq->bounce_protection_lock);
	}

	return skb;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
/*
 * nss_qdisc_drop()
 *	Called to drop the packet at the head of queue
 */
unsigned int nss_qdisc_drop(struct Qdisc *sch)
{
	struct nss_qdisc *nq = qdisc_priv(sch);
	unsigned int ret;

	if (!nq->is_virtual) {
		ret = __qdisc_queue_drop_head(sch, &sch->q);
	} else {
		spin_lock_bh(&nq->bounce_protection_lock);
		/*
		 * This function is safe to call within locks
		 */
		ret = __qdisc_queue_drop_head(sch, &sch->q);
		spin_unlock_bh(&nq->bounce_protection_lock);
	}

	return ret;
}
#endif

/*
 * nss_qdisc_reset()
 *	Called when a qdisc is reset
 */
void nss_qdisc_reset(struct Qdisc *sch)
{
	struct nss_qdisc *nq = qdisc_priv(sch);

	nss_qdisc_info("Qdisc %px (type %d) resetting\n",
			sch, nq->type);

	/*
	 * Delete all packets pending in the output queue and reset stats
	 */
	if (!nq->is_virtual) {
		qdisc_reset_queue(sch);
	} else {
		spin_lock_bh(&nq->bounce_protection_lock);
		/*
		 * This function is safe to call within locks
		 */
		qdisc_reset_queue(sch);
		spin_unlock_bh(&nq->bounce_protection_lock);
	}

	nss_qdisc_info("Qdisc %px (type %d) reset complete\n",
			sch, nq->type);
}

/*
 * nss_qdisc_iterate_fl()
 *	Iterate the filter list over the qdisc.
 *
 * Return 1, if the packet need not to be processed further, otherwise, return 0.
 */
static bool nss_qdisc_iterate_fl(struct sk_buff *skb, struct Qdisc *sch)
{
	struct nss_qdisc *nq = qdisc_priv(sch);
	struct tcf_proto *tcf;
	struct tcf_result res;
	int status;

	if (!(tcf = rcu_dereference_bh(nq->filter_list))) {
		return 0;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 15, 0))
	status = tcf_classify(skb, tcf, &res, false);
#else
	status = tcf_classify(skb, NULL, tcf, &res, false);
#endif
	if ((status == TC_ACT_STOLEN) || (status == TC_ACT_QUEUED)) {
		return 1;
	}

	/*
	 * Check the tc filter's action result.
	 * Save the higher 16-bits of skb's priority field in tc_index
	 * and update it with the class-id to be used for ingress shaping
	 * by NSS firmware.
	 */
	if (status != TC_ACT_UNSPEC) {
		skb->tc_index = TC_H_MAJ(skb->priority) >> 16;
		skb->priority = TC_H_MAKE(res.classid, skb->priority);
	}
	return 0;
}

/*
 * nss_qdisc_enqueue()
 *	Generic enqueue call for enqueuing packets into NSS for shaping
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
extern int nss_qdisc_enqueue(struct sk_buff *skb, struct Qdisc *sch)
#else
extern int nss_qdisc_enqueue(struct sk_buff *skb, struct Qdisc *sch, struct sk_buff **to_free)
#endif
{
	struct nss_qdisc *nq = qdisc_priv(sch);
	nss_tx_status_t status;

	/*
	 * If we are not the root qdisc then we should not be getting packets!!
	 */
	if (unlikely(!nq->is_root)) {
		nss_qdisc_error("Qdisc %px (type %d): unexpected packet "
			"for child qdisc - skb: %px\n", sch, nq->type, skb);
		nss_qdisc_add_to_tail(skb, sch);
		__netif_schedule(sch);
		return NET_XMIT_SUCCESS;
	}

	/*
	 * Packet enueued in linux for transmit.
	 *
	 * What we do here depends upon whether we are a bridge or not. If not a
	 * bridge then it depends on if we are a physical or virtual interface
	 * The decision we are trying to reach is whether to bounce a packet to
	 * the NSS to be shaped or not.
	 *
	 * is_bridge		is_virtual	Meaning
	 * ---------------------------------------------------------------------------
	 * false		false		Physical interface in NSS
	 *
	 * Action: Simply allow the packet to be dequeued. The packet will be
	 * shaped by the interface shaper in the NSS by the usual transmit path.
	 *
	 *
	 * false		true		Physical interface in Linux.
	 * 					NSS still responsible for shaping
	 *
	 * Action: Bounce the packet to the NSS virtual interface that represents
	 * this Linux physical interface for INTERFACE shaping. When the packet is
	 * returned from being shaped we allow it to be dequeued for transmit.
	 *
	 * true			n/a		Logical Linux interface.
	 *					Root qdisc created a virtual interface
	 *					to represent it in the NSS for shaping
	 *					purposes.
	 *
	 * Action: Bounce the packet to the NSS virtual interface (for BRIDGE shaping)
	 * the bridge root qdisc created for it. When the packet is returned from being
	 * shaped we allow it to be dequeued for transmit.
	 */

	/*
	 * Iterate over the filters attached to the qdisc.
	 */
	if (nss_qdisc_iterate_fl(skb, sch)) {
		kfree_skb(skb);
		return NET_XMIT_SUCCESS;
	}

	/*
	 * Skip the shaping of already shaped packets.
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	if (skb->tc_verd & TC_NCLS_NSS) {
		skb->tc_verd = CLR_TC_NCLS_NSS(skb->tc_verd);
		nss_qdisc_mark_and_schedule(nq->qdisc, skb);
		return NET_XMIT_SUCCESS;
	}
#else
	if (skb_skip_tc_classify_offload(skb)) {
		nss_qdisc_mark_and_schedule(nq->qdisc, skb);
		return NET_XMIT_SUCCESS;
	}
#endif

	if (!nq->is_virtual) {
		/*
		 * TX to an NSS physical - the shaping will occur as part of normal
		 * transmit path.
		 */
		nss_qdisc_add_to_tail(skb, sch);
		__netif_schedule(sch);
		return NET_XMIT_SUCCESS;
	}

	if (nq->is_bridge) {
		/*
		 * TX to a bridge, this is to be shaped by the b shaper on the virtual interface created
		 * to represent the bridge interface.
		 */
		status = nss_shaper_bounce_bridge_packet(nq->bounce_context, nq->nss_interface_number, skb);
		if (likely(status == NSS_TX_SUCCESS)) {
			return NET_XMIT_SUCCESS;
		}

		nss_qdisc_trace("Qdisc %px (type %d): failed to bounce for bridge %d, skb: %px\n",
					sch, nq->type, nq->nss_interface_number, skb);
		goto enqueue_drop;
	}

	/*
	 * TX to a physical Linux (NSS virtual).  Bounce packet to NSS for
	 * interface shaping.
	 */
	status = nss_shaper_bounce_interface_packet(nq->bounce_context,
							nq->nss_interface_number, skb);
	if (likely(status == NSS_TX_SUCCESS)) {
		return NET_XMIT_SUCCESS;
	}

	/*
	 * We failed to bounce the packet for shaping on a virtual interface
	 */
	nss_qdisc_trace("Qdisc %px (type %d): failed to bounce for "
		"interface: %d, skb: %px\n", sch, nq->type,
		nq->nss_interface_number, skb);

enqueue_drop:
	/*
	 * We were unable to transmit the packet for bridge shaping.
	 * We therefore drop it.
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
	kfree_skb(skb);

	spin_lock_bh(&nq->lock);
	sch->qstats.drops++;
	spin_unlock_bh(&nq->lock);
#else
	qdisc_drop(skb, sch, to_free);
#endif
	return NET_XMIT_DROP;
}

/*
 * nss_qdisc_dequeue()
 *	Generic dequeue call for dequeuing bounced packets.
 */
inline struct sk_buff *nss_qdisc_dequeue(struct Qdisc *sch)
{
	struct nss_qdisc *nq = qdisc_priv(sch);

	/*
	 * We use the protected dequeue API if the interface involves bounce.
	 * That is, a bridge or a virtual interface. Else, we use the unprotected
	 * API.
	 */
	if (nq->is_virtual) {
		return nss_qdisc_remove_from_tail_protected(sch);
	} else {
		return nss_qdisc_remove_from_tail(sch);
	}
}

/*
 * nss_qdisc_set_hybrid_mode_callback()
 *	The callback function for a shaper node set hybrid mode
 */
static void nss_qdisc_set_hybrid_mode_callback(void *app_data,
					struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_error("Qdisc %px (type %d): shaper node set default FAILED, response type: %d\n",
			nq->qdisc, nq->type, nim->msg.shaper_configure.config.response_type);
		atomic_set(&nq->state, NSS_QDISC_STATE_FAILED_RESPONSE);
		wake_up(&nq->wait_queue);
		return;
	}

	nss_qdisc_info("Qdisc %px (type %d): attach complete\n", nq->qdisc, nq->type);
	atomic_set(&nq->state, NSS_QDISC_STATE_READY);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_set_hybrid_mode()
 *	Configuration function that enables/disables hybrid mode
 */
int nss_qdisc_set_hybrid_mode(struct nss_qdisc *nq, enum nss_qdisc_hybrid_mode mode, uint32_t offset)
{
	int32_t state, rc;
	int msg_type;
	struct nss_if_msg nim;

	nss_qdisc_info("Setting qdisc %px (type %d) as hybrid mode\n",
			nq->qdisc, nq->type);

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_warning("Qdisc %px (type %d): qdisc state not ready: %d\n",
				nq->qdisc, nq->type, state);
		return -1;
	}

	/*
	 * Set shaper node state to IDLE
	 */
	atomic_set(&nq->state, NSS_QDISC_STATE_IDLE);

	/*
	 * Create the shaper configure message and send it down to the NSS interface
	 */
	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_CONFIG);
	nss_qdisc_msg_init(&nim, nq->nss_interface_number, msg_type, sizeof(struct nss_if_shaper_configure),
				nss_qdisc_set_hybrid_mode_callback,
				nq);

	if (mode == NSS_QDISC_HYBRID_MODE_ENABLE) {
		nim.msg.shaper_configure.config.request_type = NSS_SHAPER_CONFIG_TYPE_HYBRID_MODE_ENABLE;
		nim.msg.shaper_configure.config.msg.set_hybrid_mode.offset = offset;
	} else {
		nim.msg.shaper_configure.config.request_type = NSS_SHAPER_CONFIG_TYPE_HYBRID_MODE_DISABLE;
	}

	rc = nss_if_tx_msg(nq->nss_shaping_ctx, &nim);

	if (rc != NSS_TX_SUCCESS) {
		nss_qdisc_warning("Failed to send set hybrid mode message for "
					"qdisc type %d\n", nq->type);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	/*
	 * Wait until cleanup operation is complete at which point the state
	 * shall become non-idle.
	 */
	if (!wait_event_timeout(nq->wait_queue, atomic_read(&nq->state) != NSS_QDISC_STATE_IDLE,
				NSS_QDISC_COMMAND_TIMEOUT)) {
		nss_qdisc_error("set_hybrid_mode for qdisc %x timedout!\n", nq->qos_tag);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_error("Qdisc %px (type %d): failed to set hybrid mode "
			"State: %d\n", nq->qdisc, nq->type, state);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	nss_qdisc_info("Qdisc %px (type %d): shaper node set hybrid mode complete\n",
			nq->qdisc, nq->type);
	return 0;
}

/*
 * nss_qdisc_set_default_callback()
 *	The callback function for a shaper node set default
 */
static void nss_qdisc_set_default_callback(void *app_data,
					struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_error("Qdisc %px (type %d): shaper node set default FAILED, response type: %d\n",
			nq->qdisc, nq->type, nim->msg.shaper_configure.config.response_type);
		atomic_set(&nq->state, NSS_QDISC_STATE_FAILED_RESPONSE);
		wake_up(&nq->wait_queue);
		return;
	}

	nss_qdisc_info("Qdisc %px (type %d): attach complete\n", nq->qdisc, nq->type);
	atomic_set(&nq->state, NSS_QDISC_STATE_READY);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_node_set_default()
 *	Configuration function that sets shaper node as default for packet enqueue
 */
int nss_qdisc_set_default(struct nss_qdisc *nq)
{
	int32_t state, rc;
	int msg_type;
	struct nss_if_msg nim;

	nss_qdisc_info("Setting qdisc %px (type %d) as default\n",
			nq->qdisc, nq->type);

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_warning("Qdisc %px (type %d): qdisc state not ready: %d\n",
				nq->qdisc, nq->type, state);
		return -1;
	}

	/*
	 * Set shaper node state to IDLE
	 */
	atomic_set(&nq->state, NSS_QDISC_STATE_IDLE);

	/*
	 * Create the shaper configure message and send it down to the NSS interface
	 */
	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_CONFIG);
	nss_qdisc_msg_init(&nim, nq->nss_interface_number, msg_type, sizeof(struct nss_if_shaper_configure),
				nss_qdisc_set_default_callback,
				nq);
	nim.msg.shaper_configure.config.request_type = NSS_SHAPER_CONFIG_TYPE_SET_DEFAULT;
	nim.msg.shaper_configure.config.msg.set_default_node.qos_tag = nq->qos_tag;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, &nim);

	if (rc != NSS_TX_SUCCESS) {
		nss_qdisc_warning("Failed to send set default message for "
					"qdisc type %d\n", nq->type);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	/*
	 * Wait until cleanup operation is complete at which point the state
	 * shall become non-idle.
	 */
	if (!wait_event_timeout(nq->wait_queue, atomic_read(&nq->state) != NSS_QDISC_STATE_IDLE,
				NSS_QDISC_COMMAND_TIMEOUT)) {
		nss_qdisc_error("set_default for qdisc %x timedout!\n", nq->qos_tag);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_error("Qdisc %px (type %d): failed to default "
			"State: %d\n", nq->qdisc, nq->type, state);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	nss_qdisc_info("Qdisc %px (type %d): shaper node default complete\n",
			nq->qdisc, nq->type);
	return 0;
}

/*
 * nss_qdisc_node_attach_callback()
 *	The callback function for a shaper node attach message
 */
static void nss_qdisc_node_attach_callback(void *app_data,
					struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_info("Qdisc %px (type %d) shaper node attach FAILED - response "
			"type: %d\n", nq->qdisc, nq->type,
			nim->msg.shaper_configure.config.response_type);
		atomic_set(&nq->state, NSS_QDISC_STATE_FAILED_RESPONSE);
		wake_up(&nq->wait_queue);
		return;
	}

	nss_qdisc_info("qdisc type %d: %px, attach complete\n",
			nq->type, nq->qdisc);

	atomic_set(&nq->state, NSS_QDISC_STATE_READY);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_node_attach()
 *	Configuration function that helps attach a child shaper node to a parent.
 */
int nss_qdisc_node_attach(struct nss_qdisc *nq, struct nss_qdisc *nq_child,
			struct nss_if_msg *nim, int32_t attach_type)
{
	int32_t state, rc;
	int msg_type;

	nss_qdisc_info("Qdisc %px (type %d) attaching\n",
			nq->qdisc, nq->type);
#if defined(NSS_QDISC_PPE_SUPPORT)
	if (nq->mode == NSS_QDISC_MODE_PPE) {
		if (nss_ppe_node_attach(nq, nq_child) < 0) {
			nss_qdisc_warning("attach of new qdisc %px failed\n", nq_child->qdisc);
			return -EINVAL;

		}
		nim->msg.shaper_configure.config.msg.shaper_node_config.snc.ppe_sn_attach.child_qos_tag = nq_child->qos_tag;
	}
#endif

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_warning("Qdisc %px (type %d): not ready, state: %d\n",
				nq->qdisc, nq->type, state);
		return -1;
	}

	/*
	 * Set shaper node state to IDLE
	 */
	atomic_set(&nq->state, NSS_QDISC_STATE_IDLE);

	/*
	 * Create the shaper configure message and send it down to the NSS interface
	 */
	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_CONFIG);
	nss_qdisc_msg_init(nim, nq->nss_interface_number, msg_type, sizeof(struct nss_if_shaper_configure),
				nss_qdisc_node_attach_callback,
				nq);
	nim->msg.shaper_configure.config.request_type = attach_type;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, nim);

	if (rc != NSS_TX_SUCCESS) {
		nss_qdisc_warning("Failed to send configure message for "
					"qdisc type %d\n", nq->type);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	/*
	 * Wait until cleanup operation is complete at which point the state
	 * shall become non-idle.
	 */
	if (!wait_event_timeout(nq->wait_queue, atomic_read(&nq->state) != NSS_QDISC_STATE_IDLE,
				NSS_QDISC_COMMAND_TIMEOUT)) {
		nss_qdisc_error("attach for qdisc %x timedout!\n", nq->qos_tag);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_error("Qdisc %px (type %d) failed to attach child "
			"node, State: %d\n", nq->qdisc, nq->type, state);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	/*
	 * Save the parent node (helps in debugging)
	 */
	spin_lock_bh(&nq_child->lock);
	nq_child->parent = nq;
	spin_unlock_bh(&nq_child->lock);

#if defined(NSS_QDISC_PPE_SUPPORT)
	/*
	 * In case of hybrid mode, enable PPE queues when NSS queuing
	 * Qdiscs are attached in the hierarchy.
	 */
	nss_ppe_all_queue_enable_hybrid(nq_child);
#endif

	nss_qdisc_info("Qdisc %px (type %d): shaper node attach complete\n",
			nq->qdisc, nq->type);
	return 0;
}

/*
 * nss_qdisc_node_detach_callback()
 *	The callback function for a shaper node detach message
 */
static void nss_qdisc_node_detach_callback(void *app_data,
					struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_info("Qdisc %px (type %d): shaper node detach FAILED - response "
			"type: %d\n", nq->qdisc, nq->type,
			nim->msg.shaper_configure.config.response_type);
		atomic_set(&nq->state, NSS_QDISC_STATE_FAILED_RESPONSE);
		wake_up(&nq->wait_queue);
		return;
	}

	nss_qdisc_info("Qdisc %px (type %d): detach complete\n",
			nq->qdisc, nq->type);

	atomic_set(&nq->state, NSS_QDISC_STATE_READY);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_node_detach()
 *	Configuration function that helps detach a child shaper node to a parent.
 */
int nss_qdisc_node_detach(struct nss_qdisc *nq, struct nss_qdisc *nq_child,
	struct nss_if_msg *nim, int32_t detach_type)
{
	int32_t state, rc, msg_type;

	nss_qdisc_info("Qdisc %px (type %d) detaching\n",
			nq->qdisc, nq->type);

#if defined(NSS_QDISC_PPE_SUPPORT)
	if (nq->mode == NSS_QDISC_MODE_PPE) {
		if (nss_ppe_node_detach(nq, nq_child) < 0) {
			nss_qdisc_warning("detach of old qdisc %px failed\n", nq_child->qdisc);
			return -1;
		}
		nim->msg.shaper_configure.config.msg.shaper_node_config.snc.ppe_sn_detach.child_qos_tag = nq_child->qos_tag;
	}
#endif

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_warning("Qdisc %px (type %d): not ready, state: %d\n",
				nq->qdisc, nq->type, state);
		return -1;
	}

	/*
	 * Set shaper node state to IDLE
	 */
	atomic_set(&nq->state, NSS_QDISC_STATE_IDLE);

	/*
	 * Create and send the shaper configure message to the NSS interface
	 */
	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_CONFIG);
	nss_qdisc_msg_init(nim, nq->nss_interface_number, msg_type, sizeof(struct nss_if_shaper_configure),
				nss_qdisc_node_detach_callback,
				nq);
	nim->msg.shaper_configure.config.request_type = detach_type;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, nim);

	if (rc != NSS_TX_SUCCESS) {
		nss_qdisc_warning("Qdisc %px (type %d): Failed to send configure "
					"message.", nq->qdisc, nq->type);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	/*
	 * Wait until cleanup operation is complete at which point the state
	 * shall become non-idle.
	 */
	if (!wait_event_timeout(nq->wait_queue, atomic_read(&nq->state) != NSS_QDISC_STATE_IDLE,
				NSS_QDISC_COMMAND_TIMEOUT)) {
		nss_qdisc_error("detach for qdisc %x timedout!\n", nq->qos_tag);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_error("Qdisc %px (type %d): failed to detach child node, "
				"State: %d\n", nq->qdisc, nq->type, state);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	spin_lock_bh(&nq_child->lock);
	nq_child->parent = NULL;
	spin_unlock_bh(&nq_child->lock);

	nss_qdisc_info("Qdisc %px (type %d): shaper node detach complete\n",
			nq->qdisc, nq->type);
	return 0;
}

/*
 * nss_qdisc_configure_callback()
 *	The call back function for a shaper node configure message
 */
static void nss_qdisc_configure_callback(void *app_data,
				struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_info("Qdisc %px (type %d): shaper node configure FAILED "
			"response type: %d\n", nq->qdisc, nq->type,
			nim->msg.shaper_configure.config.response_type);
		atomic_set(&nq->state, NSS_QDISC_STATE_FAILED_RESPONSE);
		wake_up(&nq->wait_queue);
		return;
	}

	/*
	 * Shapers that need to look at responses for a configure message register
	 * for a configure callback. This needs to be invoked.
	 */
	if (nq->config_cb) {
		nq->config_cb(nq, &nim->msg.shaper_configure.config);
	}

	nss_qdisc_info("Qdisc %px (type %d): configuration complete\n",
			nq->qdisc, nq->type);
	atomic_set(&nq->state, NSS_QDISC_STATE_READY);
	wake_up(&nq->wait_queue);
}

/*
 * nss_qdisc_configure()
 *	Configuration function that aids in tuning of queuing parameters.
 */
int nss_qdisc_configure(struct nss_qdisc *nq,
	struct nss_if_msg *nim, int32_t config_type)
{
	int32_t state, rc;
	int msg_type;

	nss_qdisc_info("Qdisc %px (type %d) configuring\n", nq->qdisc, nq->type);

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_warning("Qdisc %px (type %d): not ready for configure, "
				"state : %d\n", nq->qdisc, nq->type, state);
		return -1;
	}

	/*
	 * Set shaper node state to IDLE
	 */
	atomic_set(&nq->state, NSS_QDISC_STATE_IDLE);

	/*
	 * Create and send the shaper configure message to the NSS interface
	 */
	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_CONFIG);
	nss_qdisc_msg_init(nim, nq->nss_interface_number, msg_type, sizeof(struct nss_if_shaper_configure),
				nss_qdisc_configure_callback,
				nq);
	nim->msg.shaper_configure.config.request_type = config_type;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, nim);

	if (rc != NSS_TX_SUCCESS) {
		nss_qdisc_warning("Qdisc %px (type %d): Failed to send configure "
			"message\n", nq->qdisc, nq->type);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	/*
	 * Wait until cleanup operation is complete at which point the state
	 * shall become non-idle.
	 */
	if (!wait_event_timeout(nq->wait_queue, atomic_read(&nq->state) != NSS_QDISC_STATE_IDLE,
				NSS_QDISC_COMMAND_TIMEOUT)) {
		nss_qdisc_error("configure for qdisc %x timedout!\n", nq->qos_tag);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_error("Qdisc %px (type %d): failed to configure shaper "
			"node: State: %d\n", nq->qdisc, nq->type, state);
		atomic_set(&nq->state, NSS_QDISC_STATE_READY);
		return -1;
	}

	nss_qdisc_info("Qdisc %px (type %d): shaper node configure complete\n",
			nq->qdisc, nq->type);
	return 0;
}

/*
 * nss_qdisc_register_configure_callback()
 *	Register shaper configure callback, which gets invoked on receiving a response.
 */
void nss_qdisc_register_configure_callback(struct nss_qdisc *nq, nss_qdisc_configure_callback_t cb)
{
	nss_qdisc_assert(!nq->config_cb, "Qdisc %px: config callback already registered", nq);
	nq->config_cb = cb;
}

/*
 * nss_qdisc_register_stats_callback()
 *	Register shaper stats callback, which gets invoked on receiving a stats response.
 */
void nss_qdisc_register_stats_callback(struct nss_qdisc *nq, nss_qdisc_stats_callback_t cb)
{
	nss_qdisc_assert(!nq->stats_cb, "Qdisc %px: config callback already registered", nq);
	nq->stats_cb = cb;
}

/*
 * nss_qdisc_destroy()
 *	Destroys a shaper in NSS, and the sequence is based on the position of
 *	this qdisc (child or root) and the interface to which it is attached to.
 */
void nss_qdisc_destroy(struct nss_qdisc *nq)
{
	int32_t state;
	nss_tx_status_t cmd_status;

	nss_qdisc_info("Qdisc %px (type %d) destroy\n",
			nq->qdisc, nq->type);

	/*
	 * Destroy any attached filter over qdisc.
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	tcf_destroy_chain(&nq->filter_list);
#else
	tcf_block_put(nq->block);
#endif

#if defined(NSS_QDISC_PPE_SUPPORT)
	if (nq->mode == NSS_QDISC_MODE_PPE) {
		nss_ppe_destroy(nq);
	}
#endif

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_READY) {
		nss_qdisc_warning("Qdisc %px (type %d): destroy not ready, "
				"state: %d\n", nq->qdisc, nq->type, state);
		return;
	}

	/*
	 * How we begin to tidy up depends on whether we are root or child
	 */
	nq->pending_final_state = NSS_QDISC_STATE_IDLE;
	if (!nq->is_root) {
		nss_qdisc_child_cleanup_free_node(nq);
	} else {

		/*
		 * If this is root on a bridge interface, then unassign
		 * the bshaper from all the attached interfaces.
		 */
		if (nq->is_bridge) {
			nss_qdisc_info("Qdisc %px (type %d): is root on bridge. Need to "
				"unassign bshapers from its interfaces\n", nq->qdisc, nq->type);
			nss_qdisc_refresh_bshaper_assignment(nq->qdisc, NSS_QDISC_SCAN_AND_UNASSIGN_BSHAPER);
		}

		/*
		 * Begin by freeing the root shaper node
		 */
		nss_qdisc_root_cleanup_free_node(nq);

		/*
		 * In case of IGS interface, release the reference of the IGS module.
		 */
		if (nss_igs_verify_if_num(nq->nss_interface_number)) {
			nss_igs_module_put();
		}
	}

	/*
	 * Wait until cleanup operation is complete at which point the state
	 * shall become idle.
	 */
	if (!wait_event_timeout(nq->wait_queue, atomic_read(&nq->state) == NSS_QDISC_STATE_IDLE,
				NSS_QDISC_COMMAND_TIMEOUT)) {
		nss_qdisc_error("destroy command for %x timedout!\n", nq->qos_tag);
	}

	state = atomic_read(&nq->state);
	if (state != NSS_QDISC_STATE_IDLE) {
		nss_qdisc_error("clean up for nss qdisc %x failed with "
					"status %d\n", nq->qos_tag, state);
	}

	if (nq->destroy_virtual_interface) {
		/*
		 * We are using the sync API here since qdisc operations
		 * in Linux are expected to operate synchronously.
		 */
		cmd_status = nss_virt_if_destroy_sync(nq->virt_if_ctx);
		if (cmd_status != NSS_TX_SUCCESS) {
			nss_qdisc_error("Qdisc %px virtual interface %px destroy failed: %d\n",
						nq->qdisc, nq->virt_if_ctx, cmd_status);
		}
		nq->virt_if_ctx = NULL;
	}

	nss_qdisc_info("Qdisc %px (type %d): destroy complete\n",
			nq->qdisc, nq->type);
}

/*
 * __nss_qdisc_init()
 *	Initializes a shaper in NSS, based on the position of this qdisc (child or root)
 *	and if its a normal interface or a bridge interface.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
int __nss_qdisc_init(struct Qdisc *sch, struct nss_qdisc *nq, nss_shaper_node_type_t type, uint32_t classid, uint32_t accel_mode)
{
#else
int __nss_qdisc_init(struct Qdisc *sch, struct nss_qdisc *nq, nss_shaper_node_type_t type, uint32_t classid, uint32_t accel_mode,
		struct netlink_ext_ack *extack)
{
//	int err;
#endif
	struct Qdisc *root;
	u32 parent;
	nss_tx_status_t rc;
	struct net_device *dev;
	int32_t state;
	struct nss_if_msg nim;
	int msg_type;
	nss_tx_status_t cmd_status;
#if defined(NSS_QDISC_PPE_SUPPORT)
	bool mode_ppe = false;
#endif
	bool igs_put = false;
	if (accel_mode >= TCA_NSS_ACCEL_MODE_MAX) {
		nss_qdisc_warning("Qdisc %px (type %d) accel_mode:%u should be < %u\n",
					sch, nq->type, accel_mode, TCA_NSS_ACCEL_MODE_MAX);
		return -1;
	}

	/*
	 * Initialize locks
	 */
	spin_lock_init(&nq->bounce_protection_lock);
	spin_lock_init(&nq->lock);

	/*
	 * Initialize the wait queue
	 */
	init_waitqueue_head(&nq->wait_queue);

	/*
	 * Add NSS flag to the qdisc
	 */
	sch->flags |= TCQ_F_NSS;

	/*
	 * Record our qdisc, mode and type in the private region for handy use
	 */
	nq->qdisc = sch;
	nq->type = type;

#if defined(NSS_QDISC_PPE_SUPPORT)
	/*
	 * Record user's prefered mode input.
	 */
	if (accel_mode == TCA_NSS_ACCEL_MODE_PPE) {
		mode_ppe = true;
	}
#endif

	/*
	 * We set mode to NSS as default, but if we are successful in creating
	 * qdisc in PPE, then we will get changed to NSS_QDISC_MODE_PPE.
	 */
	nq->mode = NSS_QDISC_MODE_NSS;

	/*
	 * We dont have to destroy a virtual interface unless
	 * we are the ones who created it. So set it to false
	 * by default.
	 */
	nq->destroy_virtual_interface = false;

	/*
	 * Set shaper node state to IDLE
	 */
	atomic_set(&nq->state, NSS_QDISC_STATE_IDLE);

	/*
	 * Initialize filter list.
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	RCU_INIT_POINTER(nq->filter_list, NULL);
#endif
	/*
	 * If we are a class, then classid is used as the qos tag.
	 * Else the qdisc handle will be used as the qos tag.
	 */
	if (classid) {
		nq->qos_tag = classid;
		nq->is_class = true;
	} else {
		nq->qos_tag = (uint32_t)sch->handle;
		nq->is_class = false;
	}

	parent = sch->parent;

	/*
	 * If our parent is TC_H_ROOT and we are not a class, then we are the root qdisc.
	 * Note, classes might have its qdisc as root, however we should not set is_root to
	 * true for classes. This is the reason why we check for classid.
	 */
	if ((sch->parent == TC_H_ROOT) && (!nq->is_class)) {
		nss_qdisc_info("Qdisc %px (type %d) is root\n", nq->qdisc, nq->type);
		nq->is_root = true;
		root = sch;
	} else {
		nss_qdisc_info("Qdisc %px (type %d) not root\n", nq->qdisc, nq->type);
		nq->is_root = false;
		root = qdisc_root(sch);
	}

	/*
	 * Get the net device as it will tell us if we are on a bridge,
	 * or on a net device that is represented by a virtual NSS interface (e.g. WIFI)
	 */
	dev = qdisc_dev(sch);

#if 0 // (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	/*
	 * Currently filter addition is only supported over IFB interfaces.
	 * Therefore, perform tcf block allocation (which is used for storing
	 * filter list) only if the input net device is an IFB device.
	 */
	if (netif_is_ifb_dev(dev)) {
		err = tcf_block_get(&nq->block, &nq->filter_list, sch, extack);
		if (err) {
			nss_qdisc_error("%px: Unable to initialize tcf_block\n", &nq->block);
			return -1;
		}
	} else {
		RCU_INIT_POINTER(nq->filter_list, NULL);
		nq->block = NULL;
	}
#endif

	nss_qdisc_info("Qdisc %px (type %d) init dev: %px\n", nq->qdisc, nq->type, dev);

	/*
	 * Determine if dev is a bridge or not as this determines if we
	 * interract with an I or B shaper.
	 */
	if (dev->priv_flags & IFF_EBRIDGE) {
		nss_qdisc_info("Qdisc %px (type %d) init qdisc: %px, is bridge\n",
			nq->qdisc, nq->type, nq->qdisc);
		nq->is_bridge = true;
	} else {
		nss_qdisc_info("Qdisc %px (type %d) init qdisc: %px, not bridge\n",
			nq->qdisc, nq->type, nq->qdisc);
		nq->is_bridge = false;
	}

	nss_qdisc_info("Qdisc %px (type %d) init root: %px, qos tag: %x, "
		"parent: %x rootid: %s owner: %px\n", nq->qdisc, nq->type, root,
		nq->qos_tag, parent, root->ops->id, root->ops->owner);

	/*
	 * The root must be of PPE or nss type.
	 * This is to prevent mixing NSS and PPE qdisc with linux qdisc.
	 */
	if ((parent != TC_H_ROOT) && (root->ops->owner != THIS_MODULE)) {
		nss_qdisc_warning("parent (%d) and TC_H_ROOT (%d))", parent, TC_H_ROOT);
		nss_qdisc_warning("root->ops->owner (%px) and THIS_MODULE (%px))", root->ops->owner , THIS_MODULE);
		nss_qdisc_warning("NSS qdisc %px (type %d) used along with non-nss qdiscs,"
			" or the interface is currently down", nq->qdisc, nq->type);
	}

	/*
	 * Register for NSS shaping
	 */
	nq->nss_shaping_ctx = nss_shaper_register_shaping();
	if (!nq->nss_shaping_ctx) {
		nss_qdisc_error("no shaping context returned for type %d\n",
				nq->type);
		atomic_set(&nq->state, NSS_QDISC_STATE_INIT_FAILED);
		goto init_fail;
	}

	/*
	 * If we are not the root qdisc then we have a simple enough job to do
	 */
	if (!nq->is_root) {
		struct nss_if_msg nim_alloc;

		nss_qdisc_info("Qdisc %px (type %d) initializing non-root qdisc\n",
				nq->qdisc, nq->type);

		/*
		 * The device we are operational on MUST be recognised as an NSS interface.
		 * NOTE: We do NOT support non-NSS known interfaces in this implementation.
		 * NOTE: This will still work where the dev is registered as virtual, in which case
		 * nss_interface_number shall indicate a virtual NSS interface.
		 */
		nq->nss_interface_number = nss_cmn_get_interface_number(nq->nss_shaping_ctx, dev);
		if (nq->nss_interface_number < 0) {
			nss_qdisc_error("Qdisc %px (type %d) net device unknown to "
				"nss driver %s\n", nq->qdisc, nq->type, dev->name);
			nss_shaper_unregister_shaping(nq->nss_shaping_ctx);
			atomic_set(&nq->state, NSS_QDISC_STATE_INIT_FAILED);
			goto init_fail;
		}

		/*
		 * Set the virtual flag
		 */
		nq->is_virtual = nss_qdisc_interface_is_virtual(nq->nss_shaping_ctx, nq->nss_interface_number);

#if defined(NSS_QDISC_PPE_SUPPORT)
		/*
		 * Try initializing PPE Qdisc first.
		 */
		if (mode_ppe && nss_qdisc_ppe_init(sch, nq, type, parent) < 0) {
			nss_qdisc_error("Qdisc %px (type %d) init failed", nq->qdisc, nq->type);
			nss_shaper_unregister_shaping(nq->nss_shaping_ctx);
			atomic_set(&nq->state, NSS_QDISC_STATE_INIT_FAILED);
			goto init_fail;
		}
#endif

		/*
		 * Create a shaper node for requested type.
		 * Essentially all we need to do is create the shaper node.
		 */
		nss_qdisc_info("Qdisc %px (type %d) non-root (child) create\n",
				nq->qdisc, nq->type);

		/*
		 * Create and send the shaper configure message to the interface
		 */
		msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_CONFIG);
		nss_qdisc_msg_init(&nim_alloc, nq->nss_interface_number, msg_type, sizeof(struct nss_if_shaper_configure),
					nss_qdisc_child_init_alloc_node_callback,
					nq);
		nim_alloc.msg.shaper_configure.config.request_type = NSS_SHAPER_CONFIG_TYPE_ALLOC_SHAPER_NODE;
		nim_alloc.msg.shaper_configure.config.msg.alloc_shaper_node.node_type = nq->type;
		nim_alloc.msg.shaper_configure.config.msg.alloc_shaper_node.qos_tag = nq->qos_tag;
		rc = nss_if_tx_msg(nq->nss_shaping_ctx, &nim_alloc);

		if (rc != NSS_TX_SUCCESS) {
			nss_qdisc_error("Qdisc %px (type %d) create command "
				"failed: %d\n", nq->qdisc, nq->type, rc);
			nq->pending_final_state = NSS_QDISC_STATE_CHILD_ALLOC_SEND_FAIL;
			nss_qdisc_child_cleanup_final(nq);
			goto init_fail;
		}

		/*
		 * Wait until init operation is complete.
		 */
		if (!wait_event_timeout(nq->wait_queue, atomic_read(&nq->state) != NSS_QDISC_STATE_IDLE,
					NSS_QDISC_COMMAND_TIMEOUT)) {
			nss_qdisc_error("init for qdisc %x timedout!\n", nq->qos_tag);
			return -1;
		}

		state = atomic_read(&nq->state);
		nss_qdisc_info("Qdisc %px (type %d): initialised with state: %d\n",
					nq->qdisc, nq->type, state);

		/*
		 * If state is positive, return success
		 */
		if (state > 0) {
			return 0;
		}

		goto init_fail;
	}

	/*
	 * Root qdisc has a lot of work to do. It is responsible for setting up
	 * the shaper and creating the root and default shaper nodes. Also, when
	 * operating on a bridge, a virtual NSS interface is created to represent
	 * bridge shaping. Further, when operating on a bridge, we monitor for
	 * bridge port changes and assign B shapers to the interfaces of the ports.
	 */
	nss_qdisc_info("init qdisc type %d : %px, ROOT\n", nq->type, nq->qdisc);

	/*
	 * Detect if we are operating on a bridge or interface
	 */
	if (nq->is_bridge) {
		nss_qdisc_info("Qdisc %px (type %d): initializing root qdisc on bridge\n",
			nq->qdisc, nq->type);

		/*
		 * Since we are a root qdisc on this bridge, we have to create a
		 * virtual interface to represent this bridge in the NSS (if is does
		 * not already exist). This will allow us to bounce packets to the
		 * NSS for bridge shaping.
		 */
		nq->nss_interface_number = nss_cmn_get_interface_number(nq->nss_shaping_ctx, dev);
		if (nq->nss_interface_number < 0) {

			/*
			 * Case where bridge interface is not already represented
			 * in the firmware (by clients such as bridge_mgr).
			 */
			nq->virt_if_ctx = nss_virt_if_create_sync(dev);
			if (!nq->virt_if_ctx) {
				nss_qdisc_error("Qdisc %px (type %d): cannot create virtual interface\n",
					nq->qdisc, nq->type);
				nss_shaper_unregister_shaping(nq->nss_shaping_ctx);
				atomic_set(&nq->state, NSS_QDISC_STATE_INIT_FAILED);
				goto init_fail;
			}
			nss_qdisc_info("Qdisc %px (type %d): virtual interface registered in NSS: %px\n",
				nq->qdisc, nq->type, nq->virt_if_ctx);

			/*
			 * We are the one who have created the virtual interface, so we
			 * must ensure it is destroyed whenever we are done.
			 */
			nq->destroy_virtual_interface = true;

			/*
			 * Save the virtual interface number
			 */
			nq->nss_interface_number = nss_virt_if_get_interface_num(nq->virt_if_ctx);
			nss_qdisc_info("Qdisc %px (type %d) virtual interface number: %d\n",
					nq->qdisc, nq->type, nq->nss_interface_number);
		}

		nq->is_virtual = nss_qdisc_interface_is_virtual(nq->nss_shaping_ctx, nq->nss_interface_number);

		/*
		 * The root qdisc will get packets enqueued to it, so it must
		 * register for bridge bouncing as it will be responsible for
		 * bouncing packets to the NSS for bridge shaping.
		 */
		if (!nss_igs_verify_if_num(nq->nss_interface_number)) {
			nq->bounce_context = nss_shaper_register_shaper_bounce_bridge(nq->nss_interface_number,
					nss_qdisc_bounce_callback, nq->qdisc, THIS_MODULE);
		} else {
			nss_qdisc_error("Since %d is an IFB device, it cannot"
					" register for bridge bouncing\n", nq->nss_interface_number);
			nss_shaper_unregister_shaping(nq->nss_shaping_ctx);
			atomic_set(&nq->state, NSS_QDISC_STATE_INIT_FAILED);
			goto init_fail;
		}

		if (!nq->bounce_context) {
			nss_qdisc_error("Qdisc %px (type %d): is root but cannot register "
					"for bridge bouncing\n", nq->qdisc, nq->type);
			nss_shaper_unregister_shaping(nq->nss_shaping_ctx);
			atomic_set(&nq->state, NSS_QDISC_STATE_INIT_FAILED);
			goto init_fail;
		}

	} else {
		nss_qdisc_info("Qdisc %px (type %d): is interface\n", nq->qdisc, nq->type);

		/*
		 * The device we are operational on MUST be recognised as an NSS interface.
		 * NOTE: We do NOT support non-NSS known interfaces in this basic implementation.
		 * NOTE: This will still work where the dev is registered as virtual, in which case
		 * nss_interface_number shall indicate a virtual NSS interface.
		 */
		nq->nss_interface_number = nss_cmn_get_interface_number(nq->nss_shaping_ctx, dev);
		if (nq->nss_interface_number < 0) {
			nss_qdisc_error("Qdisc %px (type %d): interface unknown to nss driver %s\n",
					nq->qdisc, nq->type, dev->name);
			nss_shaper_unregister_shaping(nq->nss_shaping_ctx);
			atomic_set(&nq->state, NSS_QDISC_STATE_INIT_FAILED);
			goto init_fail;
		}

		/*
		 * Is the interface virtual or not?
		 * NOTE: If this interface is virtual then we have to bounce packets to it for shaping
		 */
		nq->is_virtual = nss_qdisc_interface_is_virtual(nq->nss_shaping_ctx, nq->nss_interface_number);
		if (!nq->is_virtual) {
			nss_qdisc_info("Qdisc %px (type %d): interface %u is physical\n",
					nq->qdisc, nq->type, nq->nss_interface_number);
		} else {
			nss_qdisc_info("Qdisc %px (type %d): interface %u is virtual\n",
					nq->qdisc, nq->type, nq->nss_interface_number);

			/*
			 * Register for interface bounce shaping.
			 */
			if (!nss_igs_verify_if_num(nq->nss_interface_number)) {
				nq->bounce_context = nss_shaper_register_shaper_bounce_interface(nq->nss_interface_number,
						nss_qdisc_bounce_callback, nq->qdisc, THIS_MODULE);
			} else {

				/*
				 * In case of IGS interface, take the reference of IGS module.
				 */
				if (!nss_igs_module_get()) {
					nss_qdisc_error("Module reference failed for IGS interface %d"
							" , Qdisc %px (type %d)\n", nq->nss_interface_number,
							nq->qdisc, nq->type);
					nss_shaper_unregister_shaping(nq->nss_shaping_ctx);
					atomic_set(&nq->state, NSS_QDISC_STATE_INIT_FAILED);
					goto init_fail;
				}

				/*
				 * Set the flag to indicate the IGS module reference get is successful.
				 * This flag will be used to decrement the IGS module reference in case
				 * of any error conditions.
				 */
				igs_put = true;
				nq->bounce_context = nss_shaper_register_shaper_bounce_interface(nq->nss_interface_number,
						nss_qdisc_mark_and_schedule, nq->qdisc, THIS_MODULE);
			}

			if (!nq->bounce_context) {
				nss_qdisc_error("Qdisc %px (type %d): is root but failed "
				"to register for interface bouncing\n", nq->qdisc, nq->type);
				nss_shaper_unregister_shaping(nq->nss_shaping_ctx);
				atomic_set(&nq->state, NSS_QDISC_STATE_INIT_FAILED);
				goto init_fail;
			}
		}
	}

#if defined(NSS_QDISC_PPE_SUPPORT)
	/*
	 * Try initializing PPE Qdisc first.
	 */
	if (mode_ppe && nss_qdisc_ppe_init(sch, nq, type, parent) < 0) {
		nss_qdisc_error("Qdisc %px (type %d) init failed", nq->qdisc, nq->type);
		nss_shaper_unregister_shaping(nq->nss_shaping_ctx);
		atomic_set(&nq->state, NSS_QDISC_STATE_INIT_FAILED);
		goto init_fail;
	}
#endif

	/*
	 * Create and send the shaper assign message to the NSS interface
	 */
	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_ASSIGN);
	nss_qdisc_msg_init(&nim, nq->nss_interface_number, msg_type, sizeof(struct nss_if_shaper_assign),
				nss_qdisc_root_init_shaper_assign_callback,
				nq);
	nim.msg.shaper_assign.shaper_id = 0;	/* Any free shaper will do */
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, &nim);

	if (rc != NSS_TX_SUCCESS) {
		nss_qdisc_error("shaper assign command failed: %d\n", rc);
		nq->pending_final_state = NSS_QDISC_STATE_ASSIGN_SHAPER_SEND_FAIL;
		nss_qdisc_root_cleanup_final(nq);
		goto init_fail;
	}

	/*
	 * Wait until init operation is complete.
	 */
	if (!wait_event_timeout(nq->wait_queue, atomic_read(&nq->state) != NSS_QDISC_STATE_IDLE,
				NSS_QDISC_COMMAND_TIMEOUT)) {
		/*
		 * Decrement the IGS module reference.
		 */
		if (igs_put) {
			nss_igs_module_put();
		}
	nss_qdisc_error("init for qdisc %x timedout!\n", nq->qos_tag);
		return -1;
	}

	state = atomic_read(&nq->state);
	nss_qdisc_info("Qdisc %px (type %d): is initialised with state: %d\n",
			nq->qdisc, nq->type, state);

	if (state > 0) {

		/*
		 * Return if this is not a root qdisc on a bridge interface.
		 */
		if (!nq->is_root || !nq->is_bridge) {
			return 0;
		}

		nss_qdisc_info("This is a bridge interface. Linking bridge ...\n");
		/*
		 * This is a root qdisc added to a bridge interface. Now we go ahead
		 * and add this B-shaper to interfaces known to the NSS
		 */
		if (nss_qdisc_refresh_bshaper_assignment(nq->qdisc, NSS_QDISC_SCAN_AND_ASSIGN_BSHAPER) < 0) {
			nss_qdisc_destroy(nq);
			nss_qdisc_error("bridge linking failed\n");

			/*
			 * We do not go to init_fail since nss_qdisc_destroy()
			 * will take care of deleting the virtual interface.
			 */
			return -1;
		}
		nss_qdisc_info("Bridge linking complete\n");
		return 0;
	}

init_fail:

	/*
	 * Decrement the IGS module reference.
	 */
	if (igs_put) {
		nss_igs_module_put();
	}

#if defined(NSS_QDISC_PPE_SUPPORT)
	if (nq->mode == NSS_QDISC_MODE_PPE) {
		nss_ppe_destroy(nq);
	}
#endif

	/*
	 * Destroy any virtual interfaces created by us before returning a failure.
	 */
	if (nq->destroy_virtual_interface) {
		/*
		 * We are using the sync API here since qdisc operations
		 * in Linux are expected to operate synchronously.
		 */
		cmd_status = nss_virt_if_destroy_sync(nq->virt_if_ctx);
		if (cmd_status != NSS_TX_SUCCESS) {
			nss_qdisc_error("Qdisc %px virtual interface %px destroy failed: %d\n",
						nq->qdisc, nq->virt_if_ctx, cmd_status);
		}
		nq->virt_if_ctx = NULL;
	}

	return -1;
}

/*
 * nss_qdisc_init()
 *	Initialize nss qdisc based on position of the qdisc
 */
int nss_qdisc_init(struct Qdisc *sch, struct nss_qdisc *nq, nss_shaper_node_type_t type, uint32_t classid,
		uint32_t accel_mode, void *extack)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	return __nss_qdisc_init(sch, nq, type, classid, accel_mode);
#else
	return __nss_qdisc_init(sch, nq, type, classid, accel_mode, extack);
#endif
}

/*
 * nss_qdisc_basic_stats_callback()
 *	Invoked after getting basic stats
 */
static void nss_qdisc_basic_stats_callback(void *app_data,
				struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;
	struct Qdisc *qdisc = nq->qdisc;
	struct gnet_stats_basic_sync *bstats;
	struct gnet_stats_queue *qstats;
	struct nss_shaper_node_stats_response *response;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	atomic_t *refcnt;
#else
	refcount_t *refcnt;
#endif

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		nss_qdisc_warning("Qdisc %px (type %d): Receive stats FAILED - "
			"response: type: %d\n", qdisc, nq->type,
			nim->msg.shaper_configure.config.response_type);
		atomic_sub(1, &nq->pending_stat_requests);
		wake_up(&nq->wait_queue);
		return;
	}

	response = &nim->msg.shaper_configure.config.msg.shaper_node_stats_get.response;

	/*
	 * Get the right stats pointers based on whether it is a class
	 * or a qdisc.
	 */
	if (nq->is_class) {
		bstats = &nq->bstats;
		qstats = &nq->qstats;
		refcnt = &nq->refcnt;
	} else {
		bstats = &qdisc->bstats;
		qstats = &qdisc->qstats;
		refcnt = &qdisc->refcnt;
		qdisc->q.qlen = response->sn_stats.qlen_packets;
	}

	/*
	 * Update qdisc->bstats
	 */
	spin_lock_bh(&nq->lock);
	u64_stats_add(&bstats->bytes, (__u64)response->sn_stats.delta.dequeued_bytes);
	u64_stats_add(&bstats->packets, response->sn_stats.delta.dequeued_packets);

	/*
	 * Update qdisc->qstats
	 */
	qstats->backlog = response->sn_stats.qlen_bytes;

	qstats->drops += (response->sn_stats.delta.enqueued_packets_dropped +
				response->sn_stats.delta.dequeued_packets_dropped);

	/*
	 * Update qdisc->qstats
	 */
	qstats->qlen = response->sn_stats.qlen_packets;
	qstats->requeues = 0;
	qstats->overlimits += response->sn_stats.delta.queue_overrun;
	spin_unlock_bh(&nq->lock);

	/*
	 * Shapers that maintain additional unique statistics will process them
	 * via a registered callback. So invoke if its been registered.
	 */
	if (nq->stats_cb) {
		nq->stats_cb(nq, response);
	}

	/*
	 * All access to nq fields below do not need lock protection. They
	 * do not get manipulated on different thread contexts.
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	if (atomic_read(refcnt) == 0) {
#else
	if (refcount_read(refcnt) == 0) {
#endif
		atomic_sub(1, &nq->pending_stat_requests);
		wake_up(&nq->wait_queue);
		return;
	}

	/*
	 * Requests for stats again, after 1 sec.
	 */
	nq->stats_get_timer.expires += HZ;
	if (nq->stats_get_timer.expires <= jiffies) {
		nss_qdisc_info("losing time %lu, jiffies = %lu\n",
				nq->stats_get_timer.expires, jiffies);
		nq->stats_get_timer.expires = jiffies + HZ;
	}
	add_timer(&nq->stats_get_timer);
}

/*
 * nss_qdisc_get_stats_timer_callback()
 *	Invoked periodically to get updated stats
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
static void nss_qdisc_get_stats_timer_callback(unsigned long int data)
#else
static void nss_qdisc_get_stats_timer_callback(struct timer_list *tm)
#endif
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
	struct nss_qdisc *nq = (struct nss_qdisc *)data;
#else
	struct nss_qdisc *nq = from_timer(nq, tm, stats_get_timer);
#endif

	nss_tx_status_t rc;
	struct nss_if_msg nim;
	int msg_type;

	/*
	 * Create and send the shaper configure message to the NSS interface
	 */
	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_CONFIG);
	nss_qdisc_msg_init(&nim, nq->nss_interface_number, msg_type, sizeof(struct nss_if_shaper_configure),
				nss_qdisc_basic_stats_callback,
				nq);
	nim.msg.shaper_configure.config.request_type = NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_BASIC_STATS_GET;
	nim.msg.shaper_configure.config.msg.shaper_node_stats_get.qos_tag = nq->qos_tag;
	rc = nss_if_tx_msg(nq->nss_shaping_ctx, &nim);

	/*
	 * Check if we failed to send the stats request to NSS.
	 */
	if (rc != NSS_TX_SUCCESS) {
		nss_qdisc_info("%px: stats fetch request dropped, causing "
				"delay in stats fetch\n", nq->qdisc);

		/*
		 * Schedule the timer once again for re-trying. Since this is a
		 * re-try we schedule it 100ms from now, instead of a whole second.
		 */
		nq->stats_get_timer.expires = jiffies + HZ/10;
		add_timer(&nq->stats_get_timer);
	}
}

/*
 * nss_qdisc_start_basic_stats_polling()
 *	Call to initiate the stats polling timer
 */
void nss_qdisc_start_basic_stats_polling(struct nss_qdisc *nq)
{
	/*
	 * In case the stats polling timer is already
	 * initiated, return. This can happen only when
	 * there is a fallback from PPE to NSS qdisc.
	 */
	if (atomic_read(&nq->pending_stat_requests)) {
		return;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
	init_timer(&nq->stats_get_timer);
	nq->stats_get_timer.function = nss_qdisc_get_stats_timer_callback;
	nq->stats_get_timer.data = (unsigned long)nq;
#else
	timer_setup(&nq->stats_get_timer, nss_qdisc_get_stats_timer_callback, 0);
#endif

	nq->stats_get_timer.expires = jiffies + HZ;
	atomic_set(&nq->pending_stat_requests, 1);
	add_timer(&nq->stats_get_timer);
}

/*
 * nss_qdisc_stop_basic_stats_polling()
 *	Call to stop polling of basic stats
 */
void nss_qdisc_stop_basic_stats_polling(struct nss_qdisc *nq)
{
	/*
	 * If the timer was active, then delete timer and return.
	 */
	if (del_timer(&nq->stats_get_timer) > 0) {
		/*
		 * The timer was still active (counting down) when it was deleted.
		 * Therefore we are sure that there are no pending stats request
		 * for which we need to wait for. We can therefore return.
		 */
		return;
	}

	/*
	 * The timer has already fired, which means we have a pending stat response.
	 * We will have to wait until we have received the pending response.
	 */
	if (!wait_event_timeout(nq->wait_queue, atomic_read(&nq->pending_stat_requests) == 0,
				NSS_QDISC_COMMAND_TIMEOUT)) {
		nss_qdisc_error("Stats request command for %x timedout!\n", nq->qos_tag);
	}
}

/*
 * nss_qdisc_gnet_stats_copy_basic()
 *  Wrapper around gnet_stats_copy_basic()
 */
int nss_qdisc_gnet_stats_copy_basic(struct Qdisc *sch, struct gnet_dump *d,
				struct gnet_stats_basic_sync *b)
{
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 18, 0))
	return gnet_stats_copy_basic(d, b);
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
	return gnet_stats_copy_basic(d, NULL, b);
#else
	return gnet_stats_copy_basic(d, NULL, b, false);
#endif
}

/*
 * nss_qdisc_gnet_stats_copy_queue()
 *  Wrapper around gnet_stats_copy_queue()
 */
int nss_qdisc_gnet_stats_copy_queue(struct gnet_dump *d,
					struct gnet_stats_queue *q)
{
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3, 18, 0))
	return gnet_stats_copy_queue(d, q);
#else
	return gnet_stats_copy_queue(d, NULL, q, q->qlen);
#endif
}

/*
 * nss_qdisc_if_event_cb()
 *	Callback function that is registered to listen to events on net_device.
 */
static int nss_qdisc_if_event_cb(struct notifier_block *unused,
					unsigned long event, void *ptr)
{
	struct net_device *dev;
	struct net_device *br;
	struct Qdisc *br_qdisc;
	int if_num, br_num;
	struct nss_qdisc *nq;

	dev = nss_qdisc_get_dev(ptr);
	if (!dev) {
		nss_qdisc_warning("Received event %lu on NULL interface\n", event);
		return NOTIFY_DONE;
	}

	switch (event) {
	case NETDEV_BR_JOIN:
	case NETDEV_BR_LEAVE:
		nss_qdisc_info("Received NETDEV_BR_JOIN/NETDEV_BR_LEAVE on interface %s\n",
				dev->name);
		br = nss_qdisc_get_dev_master(dev);
		if_num = nss_cmn_get_interface_number(nss_qdisc_ctx, dev);

		if (br == NULL || !(br->priv_flags & IFF_EBRIDGE)) {
			nss_qdisc_error("Sensed bridge activity on interface %s "
				"that is not on any bridge\n", dev->name);
			break;
		}

		br_num = nss_cmn_get_interface_number(nss_qdisc_ctx, br);
		br_qdisc = br->qdisc;
		/*
		 * Ensure the interfaces involved are known to NSS.
		 */
		if (if_num < 0 || br_num < 0) {
			nss_qdisc_info("No action taken since if_num is %d for %s "
					"and br_num is %d for bridge %s\n", if_num,
					dev->name, br_num, br->name);
			break;
		}

		/*
		 * Ensure we have nss qdisc configured on the bridge
		 */
		if (!(br_qdisc->flags & TCQ_F_NSS)) {
			nss_qdisc_info("NSS qdisc is not configured on %s interface, "
					"qdisc id: %s", br->name, br_qdisc->ops->id);
			break;
		}

		nq = (struct nss_qdisc *)qdisc_priv(br_qdisc);

		/*
		 * Call attach or detach according as per event type.
		 */
		if (event == NETDEV_BR_JOIN) {
			nss_qdisc_info("Instructing interface %s to attach to bridge(%s) "
					"shaping\n", dev->name, br->name);
			nss_qdisc_attach_bshaper(br_qdisc, if_num);
		} else if (event == NETDEV_BR_LEAVE) {
			nss_qdisc_info("Instructing interface %s to detach from bridge(%s) "
					"shaping\n",dev->name, br->name);
			nss_qdisc_detach_bshaper(br_qdisc, if_num);
		}

		break;
	default:
		nss_qdisc_info("Received NETDEV_DEFAULT on interface %s\n", dev->name);
		break;
	}

	return NOTIFY_DONE;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
/*
 * nss_qdisc_tcf_chain()
 *	Return the filter list of qdisc.
 */
struct tcf_proto __rcu **nss_qdisc_tcf_chain(struct Qdisc *sch, unsigned long arg)
{
	struct nss_qdisc *nq = qdisc_priv(sch);

	/*
	 * Currently filter addition is only supported over IFB interfaces.
	 */
	if (!nss_igs_verify_if_num(nq->nss_interface_number)) {
		nss_qdisc_error("%d is not an ifb interface. Filter addition is only"
				" supported for IFB interfaces.", nq->nss_interface_number);
		return NULL;
	}

	/*
	 * Currently, support is available only for tc filter iterations
	 * at root qdisc.
	 */
	if (nq->is_root) {
		return &(nq->filter_list);
	}

	return NULL;
}
#else
/*
 * nss_qdisc_tcf_block()
 *	Return the block containing chain of qdisc.
 */
struct tcf_block *nss_qdisc_tcf_block(struct Qdisc *sch, unsigned long cl, struct netlink_ext_ack *extack)
{
	struct nss_qdisc *nq = qdisc_priv(sch);

	/*
	 * Currently, support is available only for tc filter iterations
	 * at root qdisc.
	 */
	if (nq->is_root) {
		return nq->block;
	}

	return NULL;
}
#endif

/*
 * nss_qdisc_tcf_bind()
 *	Bind the filter to the qdisc.
 *
 * This is an empty callback, because, currently, tc filter iteration support
 * is not present at class of a qdisc.
 */
unsigned long nss_qdisc_tcf_bind(struct Qdisc *sch, unsigned long parent, u32 classid)
{
	return (unsigned long)NULL;
}

/*
 * nss_qdisc_tcf_unbind()
 *	Unbind the filter from the qdisc.
 *
 * This is an empty callback, because, currently, tc filter iteration support
 * is not present at class of a qdisc.
 */
void nss_qdisc_tcf_unbind(struct Qdisc *sch, unsigned long arg)
{
	return;
}

static struct notifier_block nss_qdisc_device_notifier = {
		.notifier_call = nss_qdisc_if_event_cb };

/*
 * TODO: Get the bridge related code out into nss_qdisc_bridge.c
 * Get the stats into nss_qdisc_stats.c
 *
 */

/* ================== Module registration ================= */

static int __init nss_qdisc_module_init(void)
{
	int ret;
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return 0;
	}
#endif
	nss_qdisc_info("Module initializing\n");
	nss_qdisc_ctx = nss_shaper_register_shaping();

	ret = register_qdisc(&nss_pfifo_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nsspfifo registered\n");

	ret = register_qdisc(&nss_bfifo_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nssbfifo registered\n");

	ret = register_qdisc(&nss_codel_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nsscodel registered\n");

	ret = register_qdisc(&nss_fq_codel_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nssfq_codel registered\n");

	ret = register_qdisc(&nss_tbl_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nsstbl registered\n");

	ret = register_qdisc(&nss_prio_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nssprio registered\n");

	ret = register_qdisc(&nss_bf_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nssbf registered\n");

	ret = register_qdisc(&nss_wrr_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nsswrr registered\n");

	ret = register_qdisc(&nss_wfq_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nsswfq registered\n");

	ret = register_qdisc(&nss_htb_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nsshtb registered\n");

	ret = register_qdisc(&nss_blackhole_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nssblackhole registered\n");

	ret = register_qdisc(&nss_red_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nssred registered\n");

	ret = register_qdisc(&nss_wred_qdisc_ops);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nsswred registered\n");

	ret = register_netdevice_notifier(&nss_qdisc_device_notifier);
	if (ret != 0)
		return ret;
	nss_qdisc_info("nss qdisc device notifiers registered\n");

#if defined(NSS_QDISC_PPE_SUPPORT)
	nss_ppe_port_res_alloc();
	nss_qdisc_info("nss ppe qdsic configured");
#endif

	return 0;
}

static void __exit nss_qdisc_module_exit(void)
{
#ifdef CONFIG_OF
	/*
	 * If the node is not compatible, don't do anything.
	 */
	if (!of_find_node_by_name(NULL, "nss-common")) {
		return;
	}
#endif

	unregister_qdisc(&nss_pfifo_qdisc_ops);
	nss_qdisc_info("nsspfifo unregistered\n");

	unregister_qdisc(&nss_bfifo_qdisc_ops);
	nss_qdisc_info("nssbfifo unregistered\n");

	unregister_qdisc(&nss_codel_qdisc_ops);
	nss_qdisc_info("nsscodel unregistered\n");

	unregister_qdisc(&nss_fq_codel_qdisc_ops);
	nss_qdisc_info("nssfq_codel unregistered\n");

	unregister_qdisc(&nss_tbl_qdisc_ops);
	nss_qdisc_info("nsstbl unregistered\n");

	unregister_qdisc(&nss_prio_qdisc_ops);
	nss_qdisc_info("nssprio unregistered\n");

	unregister_qdisc(&nss_bf_qdisc_ops);
	nss_qdisc_info("nssbf unregistered\n");

	unregister_qdisc(&nss_wrr_qdisc_ops);
	nss_qdisc_info("nsswrr unregistered\n");

	unregister_qdisc(&nss_wfq_qdisc_ops);
	nss_qdisc_info("nsswfq unregistered\n");

	unregister_qdisc(&nss_htb_qdisc_ops);
	nss_qdisc_info("nsshtb unregistered\n");

	unregister_qdisc(&nss_blackhole_qdisc_ops);
	nss_qdisc_info("nssblackhole unregistered\n");

	unregister_qdisc(&nss_red_qdisc_ops);
	nss_qdisc_info("nssred unregistered\n");

	unregister_qdisc(&nss_wred_qdisc_ops);
	nss_qdisc_info("nsswred unregistered\n");

	unregister_netdevice_notifier(&nss_qdisc_device_notifier);

#if defined(NSS_QDISC_PPE_SUPPORT)
	nss_ppe_port_res_free();
	nss_qdisc_info("nss_ppe_port_res_free\n");
#endif
}

module_init(nss_qdisc_module_init)
module_exit(nss_qdisc_module_exit)

MODULE_LICENSE("Dual BSD/GPL");
