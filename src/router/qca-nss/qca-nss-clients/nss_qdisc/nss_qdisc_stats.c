/*
 **************************************************************************
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
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
 **************************************************************************
 */

#include "nss_qdisc.h"

static LIST_HEAD(nss_qdisc_stats_list);			/* List of stats request nodes */
static DEFINE_SPINLOCK(nss_qdisc_stats_list_lock);	/* Lock for the stats list */
static struct workqueue_struct *nss_qdisc_stats_workqueue;
static struct delayed_work nss_qdisc_stats_dwork;

/*
 * nss_qdisc_stats_queue_delayed_work()
 *	Work function, adds stats struct to the stats work list, nq is a root node
 */
static inline void nss_qdisc_stats_queue_delayed_work(struct nss_qdisc *nq)
{
	struct nss_qdisc_stats_wq *nqsw = nq->stats_wq;
	bool restart_work = false;

	spin_lock_bh(&nss_qdisc_stats_list_lock);

	/*
	 * Stats message is processed
	 */
	atomic_sub(1, &nqsw->pending_stat_work);

	/*
	 * If this qdisc's stats polling is stopped don't enqueue to work
	 */
	if (nqsw->stats_polling_stopped) {
		spin_unlock_bh(&nss_qdisc_stats_list_lock);
		wake_up(&nqsw->stats_work_waitqueue);
		return;
	}

	if (list_empty(&nss_qdisc_stats_list)) {
		/*
		 * The list is empty, so we need to restart the delayed work queue
		 */
		restart_work = true;
	}

	/*
	 * Add the work for this root node to the tail of the queue
	 */
	list_add_tail(&nqsw->stats_list, &nss_qdisc_stats_list);
	spin_unlock_bh(&nss_qdisc_stats_list_lock);

	if (restart_work) {
		queue_delayed_work(nss_qdisc_stats_workqueue, &nss_qdisc_stats_dwork, 0);
	}

	nss_qdisc_info("qdisc:%p nq:%p,queued work ", nq->qdisc, nq);
}

/*
 * nss_qdisc_stats_sync_restart()
 *	Invoked by timer to begin a new stats fetch
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
static void nss_qdisc_stats_sync_restart(unsigned long int data)
#else
static void nss_qdisc_stats_sync_restart(struct timer_list *tm)
#endif
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
	struct nss_qdisc *nq = (struct nss_qdisc *)data;
#else
	struct nss_qdisc_stats_wq *nqsw = from_timer(nqsw, tm, stats_get_timer);
	struct nss_qdisc *nq = nqsw->nq;
#endif

	nss_qdisc_stats_queue_delayed_work(nq);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
/*
 * nss_qdisc_stats_put_class_nq()
 *	Call to put class, followed by get class
 */
static inline void nss_qdisc_stats_put_class_nq(struct nss_qdisc *nq)
{
	const struct Qdisc_class_ops *clops;
	clops = nq->qdisc->ops->cl_ops;
	clops->put(nq->qdisc, (unsigned long)nq);
}

/*
 * nss_qdisc_stats_get_class_nq()
 *	Get struct nss_qdisc of corresponding class
 */
static inline struct nss_qdisc *nss_qdisc_stats_get_class_nq(struct nss_qdisc *nqp,
		uint32_t classid)
{

	const struct Qdisc_class_ops *clops;
	struct nss_qdisc *nq;
	clops = nqp->qdisc->ops->cl_ops;
	if (!clops) {
		nss_qdisc_warning("Classid %u for unsupported qdisc %p\n", classid, nqp);
		return NULL;
	}

	nq = (struct nss_qdisc *)clops->get(nqp->qdisc, classid);
	if (!nq) {
		nss_qdisc_info("Class %u not found for qdisc %p\n", classid, nqp);
		return NULL;
	}

	return nq;
}
#else
/*
 * nss_qdisc_stats_find_class_nq()
 *	Get struct nss_qdisc of corresponding class
 */
static inline struct nss_qdisc *nss_qdisc_stats_find_class_nq(struct nss_qdisc *nqp,
		uint32_t classid)
{

	const struct Qdisc_class_ops *clops;
	struct nss_qdisc *nq;
	clops = nqp->qdisc->ops->cl_ops;
	if (!clops) {
		nss_qdisc_warning("Classid %u for unsupported qdisc %p\n", classid, nqp);
		return NULL;
	}

	nq = (struct nss_qdisc *)clops->find(nqp->qdisc, classid);
	if (!nq) {
		nss_qdisc_info("Class %u not found for qdisc %p\n", classid, nqp);
		return NULL;
	}

	return nq;
}
#endif

/*
 * nss_qdisc_stats_process_node_stats()
 *	Look up the hash table for the Qdisc using the qos_tag and update the statistics
 */
static void nss_qdisc_stats_process_node_stats(struct nss_qdisc *nqr,
		struct nss_shaper_node_stats_response *response)
{
	struct Qdisc *qdisc;
	struct nss_qdisc *nq;
	struct gnet_stats_basic_sync *bstats;
	struct gnet_stats_queue *qstats;
	uint32_t qos_tag = response->qos_tag;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	atomic_t *refcnt;
#else
	refcount_t *refcnt;
#endif
	/*
	 * Find the Qdisc for this qos_tag from the hash table
	 */
	nq = nss_qdisc_htable_entry_lookup(&nqr->stats_wq->nqt, TC_H_MAJ(qos_tag));
	if (!nq) {
		nss_qdisc_info("qdisc %p (qos_tag %x): node not found - \n", nq, qos_tag);
		return;
	}

	qdisc = nq->qdisc;
	if (TC_H_MIN(qos_tag)) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
		nq = nss_qdisc_stats_get_class_nq(nq, qos_tag);
#else
		spinlock_t *root_lock = qdisc_lock(nqr->qdisc);
		spin_lock(root_lock);
		nq = nss_qdisc_stats_find_class_nq(nq, qos_tag);
#endif
		if (!nq) {
			nss_qdisc_info("Qdisc %p (qos_tag %x): class node not found - \n", nq, qos_tag);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
			spin_unlock(root_lock);
#endif
			return;
		}
	}

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
	 * NOTE: Please ensure the callback donot take qdisc root_lock.
	 */
	if (nq->stats_cb) {
		nq->stats_cb(nq, response);
	}

	if (TC_H_MIN(qos_tag)) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
		nss_qdisc_stats_put_class_nq(nq);
#else
		spinlock_t *root_lock = qdisc_lock(nqr->qdisc);
		spin_unlock(root_lock);
#endif
	}
}

/*
 * nss_qdisc_stats_sync_many_callback()
 *	Command response callback. Update the statistics using the hash table and
 *	schedule the next delayed sync for this root node as necessary
 */
static void nss_qdisc_stats_sync_many_callback(void *app_data,
		struct nss_if_msg *nim)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)app_data;
	struct nss_qdisc_stats_wq *nqsw = nq->stats_wq;
	struct Qdisc *qdisc = nq->qdisc;
	struct nss_shaper_node_stats_sync_many *nsnsm = &nim->msg.shaper_configure.config.msg.stats_get;
	struct nss_shaper_node_stats_sync_many *nqsw_nsnsm = &nqsw->stats_sync_req_msg.msg.shaper_configure.config.msg.stats_get;
	unsigned long int current_jiffies;
	int i;

	/*
	 * FW stats response received
	 */
	atomic_set(&nqsw->pending_stat_resp, 0);
	wake_up(&nqsw->stats_resp_waitqueue);

	if (nim->cm.response == NSS_CMN_RESPONSE_ACK) {
		for (i = 0; i < nsnsm->count; i++) {
			nss_qdisc_stats_process_node_stats(nq, &nsnsm->stats_sync[i]);
		}
		nss_qdisc_info("%p, last_qos_tag :%x, update last_qos_tag:%x, count:%d \n", nq, nqsw_nsnsm->last_qos_tag, nsnsm->last_qos_tag, nsnsm->count);
		nqsw_nsnsm->last_qos_tag = nsnsm->last_qos_tag;

		/*
		 * If stats sync for all nodes for this root node is done, we delay the next
		 * sync by one second for this root node
		 */
		if (nqsw_nsnsm->last_qos_tag == 0) {
			current_jiffies = jiffies;
			if (time_after(nqsw->next_req_time, current_jiffies)) {
				/*
				 * If one second has not expired yet from the beginning of the
				 * previous sync iteration for this root node, then program the
				 * timer accordingly.
				 */
				nqsw->stats_get_timer.expires = jiffies + nqsw->next_req_time - current_jiffies;
				mod_timer(&nqsw->stats_get_timer, nqsw->stats_get_timer.expires);
				nqsw->next_req_time += NSS_QDISC_STATS_SYNC_MANY_PERIOD;
				nss_qdisc_info("qdisc:%p nq:%p,nqsw->next_req_time :%lu\n", nq->qdisc, nq, nqsw->next_req_time);
				return;
			}
			/*
			 * Program the timer for one second from now for the next sync iteration
			 * for this root node
			 */
			nqsw->next_req_time = jiffies + NSS_QDISC_STATS_SYNC_MANY_PERIOD;
			nss_qdisc_info("qdisc:%p nq:%p,nqsw->next_req_time :%lu\n", nq->qdisc, nq, nqsw->next_req_time);
		}
		/*
		 * We have more nodes to sync stats for in this root node hierarchy. Continue
		 * with the next sync with a 5 ms delay
		 */
		nss_qdisc_stats_queue_delayed_work(nq);
		return;
	}
	/*
	 * We get a NACK from FW, which should not happen, restart the request
	 */
	nss_qdisc_warning("Qdisc %px (type %d): Receive stats FAILED - "
			"response: type: %d\n", qdisc, nq->type,
			nim->msg.shaper_configure.config.response_type);
	nqsw->stats_request_fail++;
	nqsw_nsnsm->last_qos_tag = 0;
	nss_qdisc_stats_queue_delayed_work(nq);
}

/*
 * nss_qdisc_stats_sync_msg_tx()
 *	Send the statistics sync request message to NSS. The command response handler
 *	will schedule the next delayed work
 */
static void nss_qdisc_stats_sync_msg_tx(struct nss_qdisc_stats_wq *nqsw)
{

	struct nss_qdisc *nq = nqsw->nq;
	struct nss_if_msg *nim = &nqsw->stats_sync_req_msg;

	/*
	 * Prepare a SYNC_MANY request
	 */
	struct nss_shaper_node_stats_sync_many *nsnsm = &nim->msg.shaper_configure.config.msg.stats_get;
	nss_tx_status_t nss_tx_status;
	int retry = 3;

	while (retry) {
		atomic_set(&nqsw->pending_stat_resp, 1);
		nss_tx_status = nss_if_tx_msg_with_size(nq->nss_shaping_ctx, nim, PAGE_SIZE);
		if (nss_tx_status != NSS_TX_SUCCESS) {
			atomic_set(&nqsw->pending_stat_resp, 0);
			nqsw->stats_request_fail++;
			retry--;
			nss_qdisc_info("TX_NOT_OKAY, try again later\n");
			usleep_range(100, 200);
			continue;
		}

		nqsw->stats_request_success++;

		/*
		 * Wait on this thread until response is received by fw.
		 */
		if (!wait_event_timeout(nqsw->stats_resp_waitqueue, atomic_read(&nqsw->pending_stat_resp) == 0,
					NSS_QDISC_COMMAND_TIMEOUT)) {
			nss_qdisc_error("Stats request command for %x timedout!\n", nq->qos_tag);
		} else {

			nss_qdisc_info("Qdisc %px nq:%p no more pending stat response  %d", nq->qdisc,
					nq, atomic_read(&nqsw->pending_stat_resp));
			return;
		}
	}

	/*
	 * TX failed after retries, reschedule ourselves with fresh start
	 */
	nsnsm->count = 0;
	nsnsm->last_qos_tag = 0;
	nss_qdisc_stats_queue_delayed_work(nq);
}


/*
 * nss_qdisc_stats_sync_process_work()
 *	Schedule delayed work to process connection stats and request next sync
 */
static void nss_qdisc_stats_sync_process_work(struct work_struct *work)
{
	struct nss_qdisc_stats_wq *nqsw;

	/*
	 * Delay the next sync request by the configured delay. This will help
	 * to space the sync command requests to NSS and also ease the workload
	 * on the CPU
	 */
	usleep_range(NSS_QDISC_STATS_SYNC_MANY_UDELAY - 100, NSS_QDISC_STATS_SYNC_MANY_UDELAY);

	spin_lock_bh(&nss_qdisc_stats_list_lock);
	if (list_empty(&nss_qdisc_stats_list)) {
		spin_unlock_bh(&nss_qdisc_stats_list_lock);
		return;
	}
	/*
	 * Dequeue the work from the head of the list and process the statistics sync for it
	 */
	nqsw = list_first_entry(&nss_qdisc_stats_list, struct nss_qdisc_stats_wq, stats_list);
	list_del(&nqsw->stats_list);
	atomic_add(1, &nqsw->pending_stat_work);
	spin_unlock_bh(&nss_qdisc_stats_list_lock);

	nss_qdisc_stats_sync_msg_tx(nqsw);

	/*
	 * Schedule work again for processing next stats request
	 */
	queue_delayed_work(nss_qdisc_stats_workqueue, &nss_qdisc_stats_dwork, 0);
}

/*
 * nss_qdisc_stats_work_queue_exit()
 * 	Stop work and release the work queue thread
 */
void nss_qdisc_stats_work_queue_exit(void)
{
	cancel_delayed_work_sync(&nss_qdisc_stats_dwork);
	destroy_workqueue(nss_qdisc_stats_workqueue);
}

/*
 * nss_qdisc_stats_work_queue_init()
 * 	Initialize the work queue resources
 */
bool nss_qdisc_stats_work_queue_init(void)
{

	nss_qdisc_stats_workqueue = create_singlethread_workqueue("nss_qdisc_stats_workqueue");
	if (!nss_qdisc_stats_workqueue) {
		return false;
	}

	INIT_DELAYED_WORK(&nss_qdisc_stats_dwork, nss_qdisc_stats_sync_process_work);
	nss_qdisc_info("nss_qdisc_stats_workqueue thread created :%p", nss_qdisc_stats_workqueue);

	return true;
}

/*
 * nss_qdisc_stats_sync_many_exit()
 * 	Stop and cleanup the stats sync framework for this root node
 */
void nss_qdisc_stats_sync_many_exit(struct nss_qdisc *nq)
{
	struct nss_qdisc_stats_wq *nqsw = nq->stats_wq;
	struct nss_qdisc_stats_wq *cursor, *next;

	spin_lock_bh(&nss_qdisc_stats_list_lock);

	/*
	 * Stop further queuing to work function
	 */
	nqsw->stats_polling_stopped = true;

	/*
	 * if list is already empty then stats sync message may be in progress,
	 * wait for pending stats processing.
	 */
	if (list_empty(&nss_qdisc_stats_list)) {
		spin_unlock_bh(&nss_qdisc_stats_list_lock);
		nss_qdisc_info("Qdisc %px nq:%p list is empty", nq->qdisc, nq);
		goto cleanup;
	}

	/*
	 * Check if stats sync for this root node is queued in the list. If so,
	 * remove it from the list.
	 */
	list_for_each_entry_safe(cursor, next, &nss_qdisc_stats_list, stats_list) {
		if (nqsw == cursor) {
			list_del(&nqsw->stats_list);
			spin_unlock_bh(&nss_qdisc_stats_list_lock);
			nss_qdisc_info("Qdisc %px nq:%p found work queue %p", nq->qdisc, nq, nqsw);
			goto free_mem;
		}
	}

	spin_unlock_bh(&nss_qdisc_stats_list_lock);

	/*
	 * The timer has already fired, which means we have a pending stat response.
	 * We will have to wait until we have received the pending response.
	 */
cleanup:
	if (del_timer(&nqsw->stats_get_timer) > 0) {
		/*
		 * The timer was still active (counting down) when it was deleted.
		 * Therefore we are sure that there are no pending stats request
		 * for which we need to wait for. We can therefore return.
		 */
		goto free_mem;
	}

	nss_qdisc_info("Qdisc %px nq:%p waiting for nqsw->pending_stat_work to be zero %d", nq->qdisc, nq, atomic_read(&nqsw->pending_stat_work));

	/*
	 * A sync is in progress. Make sure we wait for the completion before cleaning
	 * up the stats sync framework
	 */
	if (!wait_event_timeout(nqsw->stats_work_waitqueue, atomic_read(&nqsw->pending_stat_work) == 0,
				NSS_QDISC_COMMAND_TIMEOUT)) {
		nss_qdisc_error("Stats request command for %x timedout!\n", nq->qos_tag);
	}

	nss_qdisc_info("Qdisc %px nq:%p no more pending_stat_work  %d", nq->qdisc, nq, atomic_read(&nqsw->pending_stat_work));

free_mem:
	/*
	 * Free the qdisc stats hash table for this root node
	 */
	nss_qdisc_htable_dealloc(&nqsw->nqt);
	nq->stats_wq = NULL;
	kfree(nqsw);
}

/*
 * nss_qdisc_stats_sync_many_init()
 * 	Initialize stats sync framework for this root node
 */
bool nss_qdisc_stats_sync_many_init(struct nss_qdisc *nq)
{
	struct nss_shaper_node_stats_sync_many *nsnsm = NULL;
	struct nss_if_msg *nim = NULL;
	struct nss_qdisc_stats_wq *nqsw = NULL;
	int msg_type;

	nqsw = kzalloc(sizeof(struct nss_qdisc_stats_wq), GFP_KERNEL);
	if (!nqsw) {
		nss_qdisc_error("%p Failed to allocate mem for stats struct nss_qdisc_stats_wq \n", nq);
		return false;
	}

	/*
	 * Allocate the memory for the hash table for the qdisc stats. The hash table helps to
	 * optimize the look up time based on qos_tag during statistics update
	 */
	if (!nss_qdisc_htable_init(&nqsw->nqt)) {
		nss_qdisc_error("%p Failed to allocate mem for stats hash \n", nq);
		kfree(nqsw);
		return false;
	}
	nq->stats_wq = nqsw;
	nqsw->nq = nq;
	init_waitqueue_head(&nqsw->stats_work_waitqueue);
	init_waitqueue_head(&nqsw->stats_resp_waitqueue);

	/*
	 * Create the shaper configure message to the NSS interface
	 */
	nim = &nqsw->stats_sync_req_msg;
	msg_type = nss_qdisc_get_interface_msg(nq->is_bridge, NSS_QDISC_IF_SHAPER_CONFIG);
	nss_qdisc_msg_init(nim, nq->nss_interface_number, msg_type, sizeof(struct nss_if_shaper_configure),
			nss_qdisc_stats_sync_many_callback,
			nq);
	nim->msg.shaper_configure.config.request_type = NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_STATS_SYNC_MANY;

	nsnsm = &nim->msg.shaper_configure.config.msg.stats_get;

	/*
	 * Start with last_qos_tag 0
	 */
	nsnsm->last_qos_tag = 0;
	nsnsm->size = PAGE_SIZE;

	/*
	 * Initialize the timer that restarts the polling loop
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0))
	init_timer(&nqsw->stats_get_timer);
	nqsw->stats_get_timer.function = nss_qdisc_stats_sync_restart;
	nqsw->stats_get_timer.data = (unsigned long)nq;
#else
	timer_setup(&nqsw->stats_get_timer, nss_qdisc_stats_sync_restart, 0);
#endif

	nss_qdisc_info("Qdisc %px stats sync message initialized for nq:%p", nq->qdisc, nq);
	return true;
}

/*
 * nss_qdisc_stats_stop_polling()
 *	Stop polling on the root node
 */
void nss_qdisc_stats_stop_polling(struct nss_qdisc *nq)
{
	nss_qdisc_assert(nq->is_root, "Stats polling stop request received on non-root qdisc:%p with qos_tag%x", nq, nq->qos_tag);

	nss_qdisc_stats_sync_many_exit(nq);
	nss_qdisc_info(" %p stopped delayed work queue for root %x\n", nq, nq->qos_tag);
}

/*
 * nss_qdisc_stats_start_polling()
 *	Initiate polling on root node
 */
void nss_qdisc_stats_start_polling(struct nss_qdisc *nq)
{
	nss_qdisc_assert(nq->is_root, "Stats polling started on non-root qdisc:%p with qos_tag%x", nq, nq->qos_tag);

	atomic_set(&nq->stats_wq->pending_stat_work, 1);
	nss_qdisc_stats_queue_delayed_work(nq);
	nss_qdisc_info("%p Started delayed work queue for root %x\n", nq, nq->qos_tag);
}

/*
 * nss_qdisc_stats_get_root_qdisc()
 *	Get root scheduler
 */
static inline struct Qdisc *nss_qdisc_stats_get_root_qdisc(struct nss_qdisc *nq)
{
	return nq->is_root ? nq->qdisc : qdisc_dev(nq->qdisc)->qdisc;
}

/*
 * nss_qdisc_stats_qdisc_attach()
 *	Attach node to qdisc stats hash table in the root node
 */
void nss_qdisc_stats_qdisc_attach(struct nss_qdisc *nq)
{
	struct Qdisc *rqdisc;
	struct nss_qdisc *rnq;

	if (nq->is_class) {
		/* List is needed only for qdiscs and not classes,
		 * kernel already has list of classes of given parent.
		 */
		return;
	}

	/*
	 * Get root qdisc of this hierarchy, and get the stats management struct
	 */
	rqdisc = nss_qdisc_stats_get_root_qdisc(nq);
	if (!rqdisc) {
		nss_qdisc_warning("Root qdisc not found for nq:%p with qos_tag:%x\n", nq, nq->qos_tag);
		return;
	}

	rnq = qdisc_priv(rqdisc);
	if (!rnq || !rnq->stats_wq) {
		nss_qdisc_warning(" Error, stats wq should be initialized by now for root qdisc:%x\n", rnq?rnq->qos_tag:0);
		return;
	}

	nss_qdisc_htable_entry_add(&rnq->stats_wq->nqt, nq);

	/*
	 * Resize the hash table if needed
	 */
	nss_qdisc_htable_resize(rqdisc, &rnq->stats_wq->nqt);
}

/*
 * nss_qdisc_stats_qdisc_detach()
 *	Detach node from qdisc stats hash table in the root node
 */
void nss_qdisc_stats_qdisc_detach(struct nss_qdisc *nq)
{
	struct Qdisc *rqdisc;
	struct nss_qdisc *rnq;

	if (nq->is_class) {
		return;
	}

	rqdisc = nss_qdisc_stats_get_root_qdisc(nq);
	if (!rqdisc) {
		nss_qdisc_warning("Root qdisc not found for nq:%p with qos_tag:%x\n", nq, nq->qos_tag);
		return;
	}

	rnq = qdisc_priv(rqdisc);
	if (!rnq || !rnq->stats_wq) {
		nss_qdisc_warning(" Error, stats wq should be initialized by now for root qdisc:%x\n", rnq?rnq->qos_tag:0);
		return;
	}

	nss_qdisc_htable_entry_del(&rnq->stats_wq->nqt, nq);
}

