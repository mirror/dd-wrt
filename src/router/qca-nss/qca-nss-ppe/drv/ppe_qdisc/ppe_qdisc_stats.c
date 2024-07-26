/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

#include "ppe_qdisc.h"

static LIST_HEAD(ppe_qdisc_stats_list);			/* List of stats request nodes */
static DEFINE_SPINLOCK(ppe_qdisc_stats_list_lock);	/* Lock for the stats list */

static struct workqueue_struct *ppe_qdisc_stats_workqueue;	/* Workqueue object */
static struct delayed_work ppe_qdisc_stats_dwork;		/* Delayed Work object */

/*
 * ppe_qdisc_stats_queue_delayed_work()
 *	Work function to add stats work to queue
 */
static inline void ppe_qdisc_stats_queue_delayed_work(struct ppe_qdisc *pq)
{
	struct ppe_qdisc_stats_wq *pqsw = pq->stats_wq;
	bool restart_work = false;

	if (!(pq->flags & PPE_QDISC_FLAG_NODE_ROOT)) {
		return;
	}

	/*
	 * If this qdisc's stats polling is stopped don't enqueue to work
	 */
	spin_lock_bh(&ppe_qdisc_stats_list_lock);
	if (pqsw->stats_polling_stopped) {
		ppe_qdisc_warning("%px Stats Polling has stopped", pq);
		spin_unlock_bh(&ppe_qdisc_stats_list_lock);
		return;
	}

	if (list_empty(&ppe_qdisc_stats_list)) {

		/*
		 * The list is empty, so we need to restart the delayed work queue
		 */
		ppe_qdisc_info("%px ppe_qdisc_stats_list List is empty, so we need to restart the delayed work queue", pq);
		restart_work = true;
	}

	/*
	 * Add the work for this root node to the tail of the queue
	 */
	list_add_tail(&pqsw->stats_list, &ppe_qdisc_stats_list);
	spin_unlock_bh(&ppe_qdisc_stats_list_lock);

	if (restart_work) {
		ppe_qdisc_info("%px Restarting delayed work queue", pq);
		queue_delayed_work(ppe_qdisc_stats_workqueue, &ppe_qdisc_stats_dwork, 0);
	}
}

/*
 * ppe_qdisc_stats_sync_restart()
 * 	Restarts stats sync work
 */
static void ppe_qdisc_stats_sync_restart(struct timer_list *tm)
{
	/*
	 * Invoked by timer to begin a new stats fetch
	 */
	struct ppe_qdisc_stats_wq *pqsw = from_timer(pqsw, tm, stats_get_timer);
	struct ppe_qdisc *pq = pqsw->pq;

	ppe_qdisc_stats_queue_delayed_work(pq);
}

/*
 * ppe_qdisc_stats_update_parent()
 *	Updates parent node's statistics of the given leaf node
 */
void ppe_qdisc_stats_update_parent(struct ppe_qdisc *pq, struct ppe_drv_qos_q_stat *cur_stats,
		struct ppe_drv_qos_q_stat *prev_stats)
{
	struct Qdisc *rqdisc, *qdisc;
	struct ppe_qdisc *leaf = pq;
	struct ppe_drv_qos_q_stat *delta = kzalloc(sizeof(struct ppe_drv_qos_q_stat), GFP_KERNEL);
	if (!delta) {
		ppe_qdisc_warning("%px Failed to allocate mem", pq);
		return;
	}

	delta->tx_pkts = (cur_stats->tx_pkts - prev_stats->tx_pkts);
	delta->tx_bytes = (cur_stats->tx_bytes - prev_stats->tx_bytes);
	delta->drop_pkts = (cur_stats->drop_pkts - prev_stats->drop_pkts);
	delta->drop_bytes = (cur_stats->drop_bytes - prev_stats->drop_bytes);

	/*
	 * If root, return, else start iterating from the qdisc's parent node.
	 */
	if (pq->flags & PPE_QDISC_FLAG_NODE_ROOT) {
		kfree(delta);
		return;
	} else {
		pq = pq->parent;
	}

	/*
	 * Update stats of pq up until root node is reached
	 */
	while (!(pq->flags & PPE_QDISC_FLAG_NODE_ROOT)) {
		spin_lock_bh(&pq->lock);
		if (pq->flags & PPE_QDISC_FLAG_NODE_CLASS) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
			pq->bstats.packets += delta->tx_pkts;
			pq->bstats.bytes += delta->tx_bytes;
#else
			u64_stats_add(&pq->bstats.packets, delta->tx_pkts);
			u64_stats_add(&pq->bstats.bytes, delta->tx_bytes);
#endif
			pq->qstats.drops += delta->drop_pkts;
		} else {
			qdisc = pq->qdisc;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
			qdisc->bstats.packets += delta->tx_pkts;
			qdisc->bstats.bytes += delta->tx_bytes;
#else
			u64_stats_add(&qdisc->bstats.packets, delta->tx_pkts);
			u64_stats_add(&qdisc->bstats.bytes, delta->tx_bytes);
#endif
			qdisc->qstats.drops += delta->drop_pkts;
		}
		spin_unlock_bh(&pq->lock);
		pq = pq->parent;
	}
	spin_lock_bh(&pq->lock);

	/*
	 * Update stats for root node
	 */
	rqdisc = pq->qdisc;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	rqdisc->bstats.packets += delta->tx_pkts;
	rqdisc->bstats.bytes += delta->tx_bytes;
#else
	u64_stats_add(&rqdisc->bstats.packets, delta->tx_pkts);
	u64_stats_add(&rqdisc->bstats.bytes, delta->tx_bytes);
#endif
	rqdisc->qstats.drops += delta->drop_pkts;
	spin_unlock_bh(&pq->lock);

	ppe_qdisc_info("%px:Root node stats is updated with stats of its leaf node %px", pq, leaf);
	kfree(delta);
}

/*
 * ppe_qdisc_stats_get_node()
 *      Iterates through the leaf nodes list and gets corresponding statistics from PPE
 */
static void ppe_qdisc_stats_get_node(struct ppe_qdisc *pqr)
{
	struct ppe_qdisc *cursor;
	struct ppe_drv_qos_q_stat *prev_stats, *cur_stats;
	bool is_red;

	prev_stats = kzalloc(sizeof(struct ppe_drv_qos_q_stat), GFP_KERNEL);
	if (!prev_stats) {
		ppe_qdisc_warning("%px Failed to allocate mem", pqr);
		return;
	}

	cur_stats = kzalloc(sizeof(struct ppe_drv_qos_q_stat), GFP_KERNEL);
	if (!cur_stats) {
		ppe_qdisc_warning("%px Failed to allocate mem", pqr);
		return;
	}

	/*
	 * Iterate through the list of leaf node list and update statistics
	 */
	list_for_each_entry(cursor, &pqr->stats_wq->q_list_head, q_list_element) {
		if (cursor != NULL) {
			spin_lock_bh(&cursor->lock);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
			prev_stats->tx_pkts = cursor->qdisc->bstats.packets;
			prev_stats->tx_bytes = cursor->qdisc->bstats.bytes;
#else
			prev_stats->tx_pkts =
				u64_stats_read(&cursor->qdisc->bstats.packets);
			prev_stats->tx_bytes =
				u64_stats_read(&cursor->qdisc->bstats.bytes);
#endif
			prev_stats->drop_pkts = cursor->qdisc->qstats.drops;
			is_red = (cursor->type == PPE_QDISC_NODE_TYPE_RED) ? true : false;
			/*
			 * Getting statistics from PPE
			 */
			ppe_drv_qos_queue_stats_get(cursor->res.q.ucast_qid, is_red, cur_stats);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
			cursor->qdisc->bstats.packets = cur_stats->tx_pkts;
			cursor->qdisc->bstats.bytes = cur_stats->tx_bytes;
#else
			u64_stats_set(&cursor->qdisc->bstats.packets, cur_stats->tx_pkts);
			u64_stats_set(&cursor->qdisc->bstats.bytes, cur_stats->tx_bytes);
#endif
			cursor->qdisc->qstats.drops = cur_stats->drop_pkts;
			spin_unlock_bh(&cursor->lock);

			ppe_qdisc_stats_update_parent(cursor, cur_stats, prev_stats);
		}
	}

	ppe_qdisc_info("%px Root node stats updated with stats all nodes under it", pqr);
	kfree(prev_stats);
	kfree(cur_stats);
}

/*
 * ppe_qdisc_stats_sync_process_work()
 *	Schedules delayed work to process connection stats and triggers next request.
 */
static void ppe_qdisc_stats_sync_process_work(struct work_struct *work)
{
	struct ppe_qdisc_stats_wq *pqsw;

	spin_lock_bh(&ppe_qdisc_stats_list_lock);
	if (list_empty(&ppe_qdisc_stats_list)) {
		spin_unlock_bh(&ppe_qdisc_stats_list_lock);
		return;
	}

	/*
	 * Dequeue the work from the head of the list and process the statistics sync for it
	 */
	pqsw = list_first_entry(&ppe_qdisc_stats_list, struct ppe_qdisc_stats_wq, stats_list);
	list_del(&pqsw->stats_list);
	spin_unlock_bh(&ppe_qdisc_stats_list_lock);

	ppe_qdisc_stats_get_node(pqsw->pq);
	pqsw->stats_get_timer.expires = jiffies + PPE_QDISC_STATS_SYNC_MANY_PERIOD;
	add_timer(&pqsw->stats_get_timer);
}

/*
 * ppe_qdisc_stats_work_queue_exit()
 *	Stops work and releases the work queue thread
 */
void ppe_qdisc_stats_work_queue_exit(void)
{
	cancel_delayed_work_sync(&ppe_qdisc_stats_dwork);
	destroy_workqueue(ppe_qdisc_stats_workqueue);
}

/*
 * ppe_qdisc_stats_work_queue_init()
 *	Initializes the work queue resources
 */
bool ppe_qdisc_stats_work_queue_init(void)
{
	ppe_qdisc_stats_workqueue = create_singlethread_workqueue("ppe_qdisc_stats_workqueue");
	if (!ppe_qdisc_stats_workqueue) {
		return false;
	}

	INIT_DELAYED_WORK(&ppe_qdisc_stats_dwork, ppe_qdisc_stats_sync_process_work);

	ppe_qdisc_info("%px ppe_qdisc_stats_workqueue thread created", ppe_qdisc_stats_workqueue);
	return true;
}

/*
 * ppe_qdisc_stats_sync_many_exit()
 *	Stops and cleanups the stats sync framework for this root node
 */
void ppe_qdisc_stats_sync_many_exit(struct ppe_qdisc *pq)
{
	struct ppe_qdisc_stats_wq *pqsw = pq->stats_wq;
	struct ppe_qdisc_stats_wq *cursor, *next;

	/*
	 * Stop further queuing to work function
	 */
	spin_lock_bh(&ppe_qdisc_stats_list_lock);
	if (!pqsw) {
		spin_unlock_bh(&ppe_qdisc_stats_list_lock);
		return;
	}
	pqsw->stats_polling_stopped = true;

	/*
	 * if list is already empty, cleanup
	 */
	if (list_empty(&ppe_qdisc_stats_list)) {
		spin_unlock_bh(&ppe_qdisc_stats_list_lock);
		ppe_qdisc_info("Qdisc %px pq:%px list is empty", pq->qdisc, pq);
		goto cleanup;
	}

	/*
	 * Check if stats sync for this root node is queued in the list. If so,
	 * remove it from the list.
	 */
	list_for_each_entry_safe(cursor, next, &ppe_qdisc_stats_list, stats_list) {
		if (pqsw == cursor) {
			list_del(&pqsw->stats_list);
			spin_unlock_bh(&ppe_qdisc_stats_list_lock);
			ppe_qdisc_info("Qdisc %px pq:%px found work queue %px", pq->qdisc, pq, pqsw);
			goto free_mem;
		}
	}

	spin_unlock_bh(&ppe_qdisc_stats_list_lock);

cleanup:
	del_timer(&pqsw->stats_get_timer);

free_mem:
	/*
	 * Free the qdisc stats list for this root node
	 */
	pq->stats_wq = NULL;
	kfree(pqsw);
	ppe_qdisc_info("%px Qdisc Stats list object is freed", pq);
}

/*
 * ppe_qdisc_stats_sync_many_init()
 * 	Initialize stats sync framework for this root node
 */
bool ppe_qdisc_stats_sync_many_init(struct ppe_qdisc *pq)
{
	if (!pq) {
		return false;
	}

	if (!(pq->flags & PPE_QDISC_FLAG_NODE_ROOT)) {
		return false;
	}

	pq->stats_wq = kzalloc(sizeof(struct ppe_qdisc_stats_wq), GFP_KERNEL);
	if (!pq->stats_wq) {
		ppe_qdisc_warning("%px Failed to allocate mem for stats struct ppe_qdisc_stats_wq", pq);
		return false;
	}

	pq->stats_wq->pq = pq;

	/*
	 * Initialize stats list head
	 */
	INIT_LIST_HEAD(&pq->stats_wq->q_list_head);

	/*
	 * Initialize the timer that restarts the polling loop
	 */
	timer_setup(&pq->stats_wq->stats_get_timer, ppe_qdisc_stats_sync_restart, 0);
	ppe_qdisc_info("%px Qdisc stats sync message initialized for pq:%p", pq->qdisc, pq);

	return true;
}

/*
 * ppe_qdisc_stats_stop_polling()
 *	Stops polling on the root node
 */
void ppe_qdisc_stats_stop_polling(struct ppe_qdisc *pq)
{
	if (!(pq->flags & PPE_QDISC_FLAG_NODE_ROOT)) {
		ppe_qdisc_info("Stats polling stop request received on non-root qdisc:%px with qos_tag %x", pq, pq->qos_tag);
	}

	ppe_qdisc_stats_sync_many_exit(pq);
	ppe_qdisc_info("%px stopped delayed work queue for root %x", pq, pq->qos_tag);
}

/*
 * ppe_qdisc_stats_start_polling()
 *	Starts polling on root node
 */
void ppe_qdisc_stats_start_polling(struct ppe_qdisc *pq)
{
	if (!(pq->flags & PPE_QDISC_FLAG_NODE_ROOT)) {
		ppe_qdisc_info("%px Stats polling started on non-root qdisc with qos_tag %x", pq, pq->qos_tag);
	}

	ppe_qdisc_stats_queue_delayed_work(pq);
	ppe_qdisc_info("%px Started delayed work queue for root %x", pq, pq->qos_tag);
}

/*
 * ppe_qdisc_stats_get_root_qdisc()
 *	Gets root scheduler
 */
static inline struct Qdisc *ppe_qdisc_stats_get_root_qdisc(struct ppe_qdisc *pq)
{
	return (pq->flags & PPE_QDISC_FLAG_NODE_ROOT) ? pq->qdisc : qdisc_dev(pq->qdisc)->qdisc;
}

/*
 * ppe_qdisc_stats_qdisc_attach()
 *	Attaches node to stats list of leaf nodes
 */
void ppe_qdisc_stats_qdisc_attach(struct ppe_qdisc *pq)
{
	struct Qdisc *rqdisc;
	struct ppe_qdisc *rpq;

	if (pq->type < PPE_QDISC_NODE_SCH_MAX) {
		ppe_qdisc_info("%px is not a leaf node", pq);
		return;
	}

	/*
	 * Get root qdisc of this hierarchy, and get the stats management struct
	 */
	rqdisc = ppe_qdisc_stats_get_root_qdisc(pq);
	if (!(rqdisc)) {
		ppe_qdisc_warning("%px Root qdisc not found for ppe queue with qos_tag:%xn", pq, pq->qos_tag);
		return;
	}

	rpq = qdisc_priv(rqdisc);

	/*
	 * This is safety check. Ideally this would never happen
	 */
	if (!rpq || !rpq->stats_wq) {
		ppe_qdisc_warning("%px Error, stats wq should be initialized by now for root qdisc:%x", pq, rpq?rpq->qos_tag:0);
		return;
	}

	if (rpq == list_first_entry(&rpq->stats_wq->q_list_head, struct ppe_qdisc, q_list_element)) {
		ppe_qdisc_info("%px Deleting root node %px from leaf list", pq, rpq);
		list_del(&rpq->q_list_element);
	}

	list_add_tail(&pq->q_list_element, &rpq->stats_wq->q_list_head);
	spin_lock_bh(&pq->lock);
	ppe_drv_qos_queue_stats_reset(pq->res.q.ucast_qid);
	spin_unlock_bh(&pq->lock);
	ppe_qdisc_info("%px Node added into the leaf list", pq);
}

/*
 * ppe_qdisc_stats_qdisc_detach()
 *	Detaches node from the stats list of leaf nodes.
 */
void ppe_qdisc_stats_qdisc_detach(struct ppe_qdisc *pq)
{
	struct Qdisc *rqdisc;
	struct ppe_qdisc *rpq;

	if (pq->type < PPE_QDISC_NODE_SCH_MAX) {
		return;
	}

	rqdisc = ppe_qdisc_stats_get_root_qdisc(pq);
	if (!rqdisc) {
		ppe_qdisc_warning("%px Root qdisc not found for ppe qdisc with qos_tag:%x", pq, pq->qos_tag);
		return;
	}

	rpq = qdisc_priv(rqdisc);

	/*
	 * This is safety check. Ideally this would never happen
	 */
	if (!rpq || !rpq->stats_wq) {
		ppe_qdisc_warning("%px Error, stats wq should be initialized by now for root qdisc:%x", pq, rpq?rpq->qos_tag:0);
		return;
	}

	spin_lock_bh(&pq->lock);
	ppe_drv_qos_queue_stats_reset(pq->res.q.ucast_qid);
	spin_unlock_bh(&pq->lock);
	list_del(&pq->q_list_element);
	ppe_qdisc_info("%px Node detached from list of leaf nodes under the root %px", pq, rpq);
}

