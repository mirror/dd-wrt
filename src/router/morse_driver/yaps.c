/*
 * Copyright 2022-2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include <linux/gpio.h>
#include <linux/random.h>
#include <linux/timer.h>

#include "morse.h"
#include "debug.h"
#include "yaps.h"
#include "skb_header.h"
#include "ps.h"
#include "bus.h"
#include "command.h"
#include "skbq.h"
#include "yaps-hw.h"

#define BENCHMARK_PKT_LEN		(1496)
#define BENCHMARK_WAIT_MS		(5000)

/* This is a fail safe timeout */
#define CHIP_FULL_RECOVERY_TIMEOUT_MS 30

/* Defined as the most number of MPDUs per AMPDU */
#ifndef MAX_PKTS_PER_TX_TXN
#define MAX_PKTS_PER_TX_TXN	16
#endif

/* 2 full AMPDUs (and also more than the number of RX pages in chip) */
#ifndef MAX_PKTS_PER_RX_TXN
#define MAX_PKTS_PER_RX_TXN	32
#endif

#define MORSE_YAPS_DBG(_m, _f, _a...)		morse_dbg(FEATURE_ID_YAPS, _m, _f, ##_a)
#define MORSE_YAPS_INFO(_m, _f, _a...)		morse_info(FEATURE_ID_YAPS, _m, _f, ##_a)
#define MORSE_YAPS_WARN(_m, _f, _a...)		morse_warn(FEATURE_ID_YAPS, _m, _f, ##_a)
#define MORSE_YAPS_ERR(_m, _f, _a...)		morse_err(FEATURE_ID_YAPS, _m, _f, ##_a)

#define MORSE_YAPS_DBG_RATELIMITED(_m, _f, _a...)		\
	morse_dbg_ratelimited(FEATURE_ID_YAPS, _m, _f, ##_a)

/* Used to communicate with lower yaps_hw layer */
static struct morse_yaps_pkt to_chip_pkts[MAX_PKTS_PER_TX_TXN];
static struct morse_yaps_pkt from_chip_pkts[MAX_PKTS_PER_RX_TXN];

/* Mappings between sk_buff, skbq and yaps */
static struct morse_skbq *skbq_yaps_tc_q_from_aci(struct morse *mors, int aci)
{
	struct morse_yaps *yaps = mors->chip_if->yaps;

	if (aci >= ARRAY_SIZE(yaps->data_tx_qs))
		return NULL;
	return &yaps->data_tx_qs[aci];
}

static void skbq_yaps_get_tx_qs(struct morse *mors, struct morse_skbq **qs, int *num_qs)
{
	*qs = mors->chip_if->yaps->data_tx_qs;
	*num_qs = YAPS_TX_SKBQ_MAX;
}

static struct morse_skbq *skbq_yaps_bcn_q(struct morse *mors)
{
	return &mors->chip_if->yaps->beacon_q;
}

static struct morse_skbq *skbq_yaps_mgmt_q(struct morse *mors)
{
	return &mors->chip_if->yaps->mgmt_q;
}

static struct morse_skbq *skbq_yaps_cmd_q(struct morse *mors)
{
	return &mors->chip_if->yaps->cmd_q;
}

static int yaps_irq_handler(struct morse *mors, u32 status)
{
	if (test_bit(MORSE_INT_YAPS_FC_PKT_WAITING_IRQN, (unsigned long *)&status))
		set_bit(MORSE_RX_PEND, &mors->chip_if->event_flags);

	if (test_bit(MORSE_INT_YAPS_FC_PACKET_FREED_UP_IRQN, (unsigned long *)&status)) {
		/* No need for the timer anymore */
		del_timer_sync(&mors->chip_if->yaps->chip_queue_full.timer);
		set_bit(MORSE_TX_PACKET_FREED_UP_PEND, &mors->chip_if->event_flags);
	}

	queue_work(mors->chip_wq, &mors->chip_if_work);
	return 0;
}

static int morse_hw_restarted(struct morse *mors)
{
	int ret = 0;
	int err = 0;

	ret = morse_hw_enable_stop_notifications(mors, true);
	if (ret) {
		MORSE_ERR(mors, "%s: morse_hw_enable_stop_notifications failed: %d\n",
			__func__, ret);
		err = ret;
	}

	return err;
}

/**
 * Flush data in tx queues
 *
 * @yaps: Pointer to yaps struct
 */
static void morse_yaps_flush_tx_data(struct morse *mors)
{
	int i;
	struct morse_yaps *yaps = mors->chip_if->yaps;

	if ((yaps->flags & MORSE_CHIP_IF_FLAGS_DIR_TO_CHIP) &&
	    (yaps->flags & (MORSE_CHIP_IF_FLAGS_DATA | MORSE_CHIP_IF_FLAGS_BEACON))) {
		morse_skbq_tx_flush(&yaps->beacon_q);
		morse_skbq_tx_flush(&yaps->mgmt_q);
		for (i = 0; i < ARRAY_SIZE(yaps->data_tx_qs); i++)
			morse_skbq_tx_flush(&yaps->data_tx_qs[i]);
	}
}

/**
 * Flush commands in cmd queues
 *
 * @yaps: Pointer to yaps struct
 */
static void morse_yaps_flush_cmds(struct morse *mors)
{
	struct morse_yaps *yaps = mors->chip_if->yaps;

	if (yaps->flags & MORSE_CHIP_IF_FLAGS_COMMAND) {
		morse_skbq_finish(&yaps->cmd_q);
		morse_skbq_finish(&yaps->cmd_resp_q);
	}
}

const struct chip_if_ops morse_yaps_ops = {
	.init = morse_yaps_hw_init,
	.hw_restarted = morse_hw_restarted,
	.flush_tx_data = morse_yaps_flush_tx_data,
	.flush_cmds = morse_yaps_flush_cmds,
	.skbq_get_tx_status_pending_count = morse_yaps_get_tx_status_pending_count,
	.skbq_get_tx_buffered_count = morse_yaps_get_tx_buffered_count,
	.finish = morse_yaps_hw_finish,
	.skbq_get_tx_qs = skbq_yaps_get_tx_qs,
	.skbq_bcn_tc_q = skbq_yaps_bcn_q,
	.skbq_mgmt_tc_q = skbq_yaps_mgmt_q,
	.skbq_cmd_tc_q = skbq_yaps_cmd_q,
	.skbq_tc_q_from_aci = skbq_yaps_tc_q_from_aci,
	.chip_if_handle_irq = yaps_irq_handler
};

static int morse_yaps_read_pkt(struct morse_yaps *yaps, struct sk_buff *skb)
{
	struct morse *mors = yaps->mors;
	struct sk_buff_head skbq;
	struct morse_skbq *mq = NULL;
	struct morse_buff_skb_header *hdr;
	int skb_bytes_remaining;
	int skb_len;
	int ret = 0;

	if (!skb) {
		ret = -EINVAL;
		goto exit_return_page;
	}

	__skb_queue_head_init(&skbq);

	hdr = (struct morse_buff_skb_header *)skb->data;

	/* Validate header */
	if (hdr->sync != 0xAA) {
		MORSE_YAPS_ERR(mors, "%s sync value error [0xAA:%d], hdr.len %d\n",
			       __func__, hdr->sync, hdr->len);
		ret = -EIO;
		goto exit_return_page;
	}

	if (yaps->mors->chip_if->validate_skb_checksum && !morse_validate_skb_checksum(skb->data)) {
		mors->debug.page_stats.invalid_checksum++;
		MORSE_YAPS_DBG(yaps->mors, "SKB checksum is invalid hdr:[c:%02X s:%02X len:%d]",
			       hdr->channel, hdr->sync, hdr->len);

		/* Pass the data to mac80211 if tx status. It has been observed the tput drops if
		 * a corrupted tx status is dropped.
		 */
		if (hdr->channel != MORSE_SKB_CHAN_TX_STATUS) {
			ret = -EIO;
			goto exit;
		}
		mors->debug.page_stats.invalid_tx_status_checksum++;
	}

	/* Get correct skbq for the data based on the declared channel */
	switch (hdr->channel) {
	case MORSE_SKB_CHAN_DATA:
	case MORSE_SKB_CHAN_NDP_FRAMES:
	case MORSE_SKB_CHAN_TX_STATUS:
	case MORSE_SKB_CHAN_DATA_NOACK:
	case MORSE_SKB_CHAN_BEACON:
	case MORSE_SKB_CHAN_MGMT:
	case MORSE_SKB_CHAN_WIPHY:
	case MORSE_SKB_CHAN_LOOPBACK:
		mq = &yaps->data_rx_q;
		break;
	case MORSE_SKB_CHAN_COMMAND:
		mq = &yaps->cmd_resp_q;
		break;
	default:
		MORSE_YAPS_ERR(mors, "channel value error [%d]\n", hdr->channel);
		ret = -EIO;
		goto exit_return_page;
	}

	/* Check there is room in the skbq */
	skb_len = sizeof(*hdr) + hdr->offset + le16_to_cpu(hdr->len);
	skb_bytes_remaining = morse_skbq_space(mq);

#ifdef MORSE_YAPS_SUPPORTS_BENCHMARK
	if (hdr->channel == MORSE_SKB_CHAN_LOOPBACK)
		atomic_inc(&yaps->benchmark_cnt_fc);
#endif

	/* if skb queue full, don't read */
	if (skb_len > skb_bytes_remaining) {
		MORSE_YAPS_ERR(mors, "Page will not fit in SKBQ, dropping - len %d remain %d\n",
			       skb_len, skb_bytes_remaining);
		ret = -ENOMEM;
		/* Queue work to clear backlog */
		queue_work(mors->net_wq, &mq->dispatch_work);
		goto exit_return_page;
	}

	skb_trim(skb, skb_len);
	__skb_queue_tail(&skbq, skb);

	if (skbq.qlen)
		morse_skbq_enq(mq, &skbq);

	/* push packets up in a different context */
	queue_work(mors->net_wq, &mq->dispatch_work);

	goto exit;

exit_return_page:
	if (ret && mq) {
		MORSE_YAPS_ERR(mors, "%s failed %d\n", __func__, ret);
		morse_skbq_purge(mq, &skbq);
		goto exit;
	}

exit:
	if (ret && skb)
		dev_kfree_skb(skb);

	return ret;
}

static int morse_yaps_tx(struct morse_yaps *yaps, struct morse_skbq *mq)
{
	int ret = 0;
	int num_items = 0;
	int tc_pkt_idx = 0;
	int num_pkts_sent = 0;
	int i;
	struct sk_buff *skb;
	struct sk_buff_head skbq_to_send;
	struct sk_buff_head skbq_sent;
	struct sk_buff_head skbq_failed;
	struct sk_buff *pfirst, *pnext;
	struct morse *mors = yaps->mors;
	struct morse_buff_skb_header *hdr;

	/* Check there is something on the queue */
	spin_lock_bh(&mq->lock);
	skb = skb_peek(&mq->skbq);
	spin_unlock_bh(&mq->lock);
	if (!skb)
		return 0;

	__skb_queue_head_init(&skbq_to_send);
	__skb_queue_head_init(&skbq_sent);
	__skb_queue_head_init(&skbq_failed);

	if (mq == &yaps->cmd_q)
		/* Purge timed-out commands (this should not happen) */
		morse_skbq_purge(mq, &mq->pending);
	else if (mq == &yaps->mgmt_q && mq->skbq.qlen > 0)
		/* Purge old mgmt frames that have not been sent due to congestion */
		morse_skbq_purge_aged(mors, mq);

	/* We should replace MAX_PKTS_PER_TX_TXN with some heuristic that takes
	 * into account free space in the queue and free pages in the pool
	 */
	num_items = morse_skbq_deq_num_items(mq, &skbq_to_send, MAX_PKTS_PER_TX_TXN);

	skb_queue_walk_safe(&skbq_to_send, pfirst, pnext) {
		enum morse_yaps_to_chip_q tc_queue;

		hdr = (struct morse_buff_skb_header *)pfirst->data;
		switch (hdr->channel) {
		case MORSE_SKB_CHAN_COMMAND:
			tc_queue = MORSE_YAPS_CMD_Q;
			break;
		case MORSE_SKB_CHAN_BEACON:
			tc_queue = MORSE_YAPS_BEACON_Q;
			break;
		case MORSE_SKB_CHAN_MGMT:
			tc_queue = MORSE_YAPS_MGMT_Q;
			break;
		default:
			tc_queue = MORSE_YAPS_TX_Q;
			break;
		}
		to_chip_pkts[tc_pkt_idx].tc_queue = tc_queue;
		to_chip_pkts[tc_pkt_idx].skb = pfirst;
		tc_pkt_idx++;
	}

	/* Send queued packets to chip */
	ret = yaps->ops->update_status(yaps);
	if (ret)
		return ret;

	ret = yaps->ops->write_pkts(yaps, to_chip_pkts, tc_pkt_idx, &num_pkts_sent);

	/* Move sent packets to done queue and update stats */
	for (i = 0; i < num_pkts_sent; ++i) {
		switch (to_chip_pkts[i].tc_queue) {
		case MORSE_YAPS_CMD_Q:
			mors->debug.page_stats.cmd_tx++;
			break;
		case MORSE_YAPS_BEACON_Q:
			mors->debug.page_stats.bcn_tx++;
			break;
		case MORSE_YAPS_MGMT_Q:
			mors->debug.page_stats.mgmt_tx++;
			break;
		default:
			mors->debug.page_stats.data_tx++;
			break;
		}
		pfirst = __skb_dequeue(&skbq_to_send);
#ifdef MORSE_YAPS_SUPPORTS_BENCHMARK
		hdr = (struct morse_buff_skb_header *)pfirst->data;
		if (hdr->channel == MORSE_SKB_CHAN_LOOPBACK)
			atomic_inc(&yaps->benchmark_cnt_tc);
#endif
		__skb_queue_tail(&skbq_sent, pfirst);
	}

	for (i = num_pkts_sent; i < num_items; ++i) {
		mors->debug.page_stats.no_page++;
		pfirst = __skb_dequeue(&skbq_to_send);
		__skb_queue_tail(&skbq_failed, pfirst);
	}

	if (skbq_failed.qlen > 0) {
		morse_skbq_enq_prepend(mq, &skbq_failed);

		/* queue full, cant requeue */
		mors->debug.page_stats.write_fail += skbq_failed.qlen;
		if (skbq_failed.qlen > 0) {
			MORSE_YAPS_WARN(mors, "cant requeue failed pkts, skbq full, purging\n");
			__skb_queue_purge(&skbq_failed);
		}
	}

	if (skbq_sent.qlen > 0)
		morse_skbq_tx_complete(mq, &skbq_sent);

	return ret;
}

/* Returns true if there are TX data pages waiting to be sent */
static bool morse_yaps_tx_data_handler(struct morse_yaps *yaps)
{
	s16 aci;
	u32 count = 0;
	struct morse *mors = yaps->mors;

	for (aci = MORSE_ACI_VO; aci >= 0; aci--) {
		struct morse_skbq *data_q = skbq_yaps_tc_q_from_aci(mors, aci);

		if (!morse_is_data_tx_allowed(mors))
			break;

		yaps->chip_queue_full.is_full = morse_yaps_tx(yaps, data_q);
		count += morse_skbq_count(data_q);

		if (yaps->chip_queue_full.is_full)
			break;

		if (aci == MORSE_ACI_BE)
			break;
	}

	/* Data has potentially been transmitted from the data SKBQs.
	 * If the mac80211 TX data Qs were previously stopped,
	 * now would be a good time to check if they can be started again.
	 */
	morse_skbq_may_wake_tx_queues(mors);
	if (mors->custom_configs.enable_airtime_fairness &&
	    !test_bit(MORSE_STATE_FLAG_DATA_QS_STOPPED, &mors->state_flags))
		tasklet_schedule(&mors->tasklet_txq);

	return (count > 0) && morse_is_data_tx_allowed(mors);
}

/* Returns true if there are commands waiting to be sent */
static bool morse_yaps_tx_cmd_handler(struct morse_yaps *yaps)
{
	struct morse_skbq *cmd_q = &yaps->cmd_q;

	morse_yaps_tx(yaps, cmd_q);

	return (morse_skbq_count(cmd_q) > 0);
}

static bool morse_yaps_tx_beacon_handler(struct morse_yaps *yaps)
{
	struct morse_skbq *beacon_q = &yaps->beacon_q;

	morse_yaps_tx(yaps, beacon_q);

	return (morse_skbq_count(beacon_q) > 0);
}

static bool morse_yaps_tx_mgmt_handler(struct morse_yaps *yaps)
{
	struct morse_skbq *mgmt_q = &yaps->mgmt_q;

	morse_yaps_tx(yaps, mgmt_q);

	return (morse_skbq_count(mgmt_q) > 0);
}

/* Returns true if there are populated RX pages left in the device */
static bool morse_yaps_rx_handler(struct morse_yaps *yaps)
{
	int ret = 0;
	int i;
	int num_pks_received;

	ret = yaps->ops->update_status(yaps);
	if (ret)
		goto exit;

	ret =
	    yaps->ops->read_pkts(yaps, from_chip_pkts, ARRAY_SIZE(from_chip_pkts),
				 &num_pks_received);
	if (ret && ret != -EAGAIN) {
		MORSE_YAPS_ERR(yaps->mors, "YAPS read_pkts fail: %d", ret);
		goto exit;
	}

	if (num_pks_received == 0)
		yaps->mors->debug.page_stats.rx_empty++;

	for (i = 0; i < num_pks_received; ++i) {
		morse_yaps_read_pkt(yaps, from_chip_pkts[i].skb);
		from_chip_pkts[i].skb = NULL;
	}

exit:
	if (ret == -ENOMEM || ret == -EAGAIN)
		return true;
	else
		return false;
}

void morse_yaps_stale_tx_work(struct work_struct *work)
{
	int i;
	int flushed = 0;
	struct morse *mors = container_of(work, struct morse, tx_stale_work);
	struct morse_yaps *yaps;

	if (!mors->chip_if || !mors->chip_if->yaps || !mors->stale_status.enabled)
		return;

	yaps = mors->chip_if->yaps;
	flushed += morse_skbq_check_for_stale_tx(mors, &yaps->beacon_q);
	flushed += morse_skbq_check_for_stale_tx(mors, &yaps->mgmt_q);

	for (i = 0; i < ARRAY_SIZE(yaps->data_tx_qs); i++)
		flushed += morse_skbq_check_for_stale_tx(mors, &yaps->data_tx_qs[i]);

	if (flushed) {
		MORSE_YAPS_DBG(mors, "%s: Flushed %d stale TX SKBs\n", __func__, flushed);

		if (mors->ps.enable &&
		    !mors->ps.suspended && (morse_yaps_get_tx_buffered_count(mors) == 0)) {
			/* Evaluate ps to check if it was gated on a stale tx status */
			queue_delayed_work(mors->chip_wq, &mors->ps.delayed_eval_work, 0);
		}
	}
}

void morse_yaps_work(struct work_struct *work)
{
	struct morse *mors = container_of(work,
					  struct morse, chip_if_work);
	int ps_bus_timeout_ms = 0;
	unsigned long *flags = &mors->chip_if->event_flags;
	struct morse_yaps *yaps = mors->chip_if->yaps;

	/* Don't attempt to interact with device once it becomes unresponsive */
	if (test_bit(MORSE_STATE_FLAG_CHIP_UNRESPONSIVE, &mors->state_flags))
		return;

	if (!*flags)
		return;

	/* Disable power save in case it is running */
	morse_ps_disable(mors);
	morse_claim_bus(mors);

	/* Handle any populated RX pages from chip first to
	 * avoid dropping pkts due to full on-chip buffers.
	 * Check if all pages were removed, set event flags if not.
	 */
	if (test_and_clear_bit(MORSE_RX_PEND, flags)) {
		int buffered = yaps->data_rx_q.skbq.qlen;

		if (morse_yaps_rx_handler(yaps))
			set_bit(MORSE_RX_PEND, flags);

		if (yaps->data_rx_q.skbq.qlen > buffered)
			ps_bus_timeout_ms = max(ps_bus_timeout_ms, NETWORK_BUS_TIMEOUT_MS);
	}

	/* TX any commands before considering data */
	if (test_and_clear_bit(MORSE_TX_COMMAND_PEND, flags)) {
		if (morse_yaps_tx_cmd_handler(yaps))
			set_bit(MORSE_TX_COMMAND_PEND, flags);
	}

	/* TX beacons before considering mgmt/data */
	if (test_and_clear_bit(MORSE_TX_BEACON_PEND, flags)) {
		if (morse_yaps_tx_beacon_handler(yaps))
			set_bit(MORSE_TX_BEACON_PEND, flags);
	}

	/* TX mgmt before considering data */
	if (test_and_clear_bit(MORSE_TX_MGMT_PEND, flags)) {
		ps_bus_timeout_ms = max(ps_bus_timeout_ms, NETWORK_BUS_TIMEOUT_MS);
		if (morse_yaps_tx_mgmt_handler(yaps))
			set_bit(MORSE_TX_MGMT_PEND, flags);
	}

	/* Pause TX data Qs */
	if (test_and_clear_bit(MORSE_DATA_TRAFFIC_PAUSE_PEND, flags)) {
		if (test_and_clear_bit(MORSE_DATA_TRAFFIC_RESUME_PEND, flags))
			MORSE_ERR_RATELIMITED(mors,
					      "Latency to handle twt traffic pause is too great\n");

		morse_skbq_data_traffic_pause(mors);
	}

	/* Resume TX data Qs  */
	if (test_and_clear_bit(MORSE_DATA_TRAFFIC_RESUME_PEND, flags)) {
		if (test_bit(MORSE_DATA_TRAFFIC_PAUSE_PEND, flags))
			MORSE_ERR_RATELIMITED(mors,
					      "Latency to handle twt traffic resume is too great\n");

		morse_skbq_data_traffic_resume(mors);
	}

	/* Handle chip queue status */
	if (test_and_clear_bit(MORSE_TX_PACKET_FREED_UP_PEND, flags))
		yaps->chip_queue_full.is_full = false;

	/* Check to see if the queue is full or
	 * long enough has past since the queue was full
	 */
	if (yaps->chip_queue_full.is_full &&
	    time_before(jiffies, yaps->chip_queue_full.retry_expiry))
		goto exit;

	/* Finally TX any data */
	if (test_and_clear_bit(MORSE_TX_DATA_PEND, flags)) {
		ps_bus_timeout_ms = max(ps_bus_timeout_ms, NETWORK_BUS_TIMEOUT_MS);
		if (morse_yaps_tx_data_handler(yaps))
			set_bit(MORSE_TX_DATA_PEND, flags);

		if (yaps->chip_queue_full.is_full) {
			yaps->chip_queue_full.retry_expiry =
			    jiffies + msecs_to_jiffies(CHIP_FULL_RECOVERY_TIMEOUT_MS);
			mod_timer(&yaps->chip_queue_full.timer, yaps->chip_queue_full.retry_expiry);
		}
	}

	if (test_and_clear_bit(MORSE_UPDATE_HW_CLOCK_REFERENCE, flags))
		morse_hw_clock_update(mors);

exit:
	if (ps_bus_timeout_ms)
		morse_ps_bus_activity(mors, ps_bus_timeout_ms);

	/* Disable power save in case it is running */
	morse_release_bus(mors);
	morse_ps_enable(mors);

	/* Don't requeue work if we are shutting down. */
	if (yaps->finish)
		return;
	/* Evaluate all events except MORSE_TX_DATA_PEND in case data tx queue is full */
	if ((*flags) & ~(1 << MORSE_TX_DATA_PEND))
		queue_work(mors->chip_wq, &mors->chip_if_work);
	/* if data tx queue is not full and the work hasn't been queued let's queue it */
	else if (!yaps->chip_queue_full.is_full && *flags)
		queue_work(mors->chip_wq, &mors->chip_if_work);
}

int morse_yaps_get_tx_status_pending_count(struct morse *mors)
{
	int i = 0;
	int count = 0;
	struct morse_yaps *yaps;

	if (!mors->chip_if || !mors->chip_if->yaps)
		return 0;

	yaps = mors->chip_if->yaps;
	count += yaps->beacon_q.pending.qlen;
	count += yaps->mgmt_q.pending.qlen;
	count += yaps->cmd_q.pending.qlen;

	for (i = 0; i < ARRAY_SIZE(yaps->data_tx_qs); i++)
		count += yaps->data_tx_qs[i].pending.qlen;

	return count;
}

int morse_yaps_get_tx_buffered_count(struct morse *mors)
{
	int i = 0;
	int count = 0;
	struct morse_yaps *yaps;

	if (!mors->chip_if || !mors->chip_if->yaps)
		return 0;

	yaps = mors->chip_if->yaps;
	count += yaps->beacon_q.skbq.qlen + yaps->beacon_q.pending.qlen;
	count += yaps->mgmt_q.skbq.qlen + yaps->mgmt_q.pending.qlen;
	count += yaps->cmd_q.skbq.qlen + yaps->cmd_q.pending.qlen;

	for (i = 0; i < ARRAY_SIZE(yaps->data_tx_qs); i++)
		count += morse_skbq_count_tx_ready(&yaps->data_tx_qs[i]) +
		    yaps->data_tx_qs[i].pending.qlen;

	return count;
}

#if KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE
static void morse_tx_chip_full_timer(unsigned long addr)
{
	struct morse_yaps *yaps = (struct morse_yaps *)addr;
#else
static void morse_tx_chip_full_timer(struct timer_list *t)
{
	struct morse_yaps *yaps = from_timer(yaps, t, chip_queue_full.timer);
#endif

	if (!yaps || !yaps->mors)
		return;

	/* Haven't received anything from the chip indicating the queue might have room */
	queue_work(yaps->mors->chip_wq, &yaps->mors->chip_if_work);
}

static int morse_tx_chip_full_timer_init(struct morse_yaps *yaps)
{
#if KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE
	init_timer(&yaps->chip_queue_full.timer);
	yaps->chip_queue_full.timer.data = (unsigned long)yaps;
	yaps->chip_queue_full.timer.function = morse_tx_chip_full_timer;
	add_timer(&yaps->chip_queue_full.timer);
#else
	timer_setup(&yaps->chip_queue_full.timer, morse_tx_chip_full_timer, 0);
#endif

	return 0;
}

static int morse_tx_chip_full_timer_finish(struct morse_yaps *yaps)
{
	del_timer_sync(&yaps->chip_queue_full.timer);

	return 0;
}

int morse_yaps_init(struct morse *mors, struct morse_yaps *yaps, u8 flags)
{
	int i;

	yaps->mors = mors;
	yaps->flags = flags;
	mors->chip_if->active_chip_if = MORSE_CHIP_IF_YAPS;

	if (yaps->flags & MORSE_CHIP_IF_FLAGS_DATA) {
		/* YAPS is bi-directional */
		morse_skbq_init(mors, &yaps->data_rx_q,
				MORSE_CHIP_IF_FLAGS_DATA | MORSE_CHIP_IF_FLAGS_DIR_TO_HOST);
		morse_skbq_init(mors, &yaps->beacon_q,
				MORSE_CHIP_IF_FLAGS_DATA | MORSE_CHIP_IF_FLAGS_DIR_TO_HOST);
		morse_skbq_init(mors, &yaps->mgmt_q,
				MORSE_CHIP_IF_FLAGS_DATA | MORSE_CHIP_IF_FLAGS_DIR_TO_HOST);
		for (i = 0; i < ARRAY_SIZE(yaps->data_tx_qs); i++)
			morse_skbq_init(mors, &yaps->data_tx_qs[i],
					MORSE_CHIP_IF_FLAGS_DATA | MORSE_CHIP_IF_FLAGS_DIR_TO_CHIP);
	}

	if (yaps->flags & MORSE_CHIP_IF_FLAGS_COMMAND) {
		/* YAPS is bi-directional */
		morse_skbq_init(mors, &yaps->cmd_q,
				MORSE_CHIP_IF_FLAGS_COMMAND | MORSE_CHIP_IF_FLAGS_DIR_TO_CHIP);
		morse_skbq_init(mors, &yaps->cmd_resp_q,
				MORSE_CHIP_IF_FLAGS_COMMAND | MORSE_CHIP_IF_FLAGS_DIR_TO_HOST);
	}

	morse_tx_chip_full_timer_init(yaps);

	return 0;
}

void morse_yaps_finish(struct morse_yaps *yaps)
{
	int i;

	yaps->finish = true;

	if (yaps->flags & MORSE_CHIP_IF_FLAGS_DATA) {
		morse_skbq_finish(&yaps->data_rx_q);
		morse_skbq_finish(&yaps->beacon_q);
		morse_skbq_finish(&yaps->mgmt_q);
		for (i = 0; i < ARRAY_SIZE(yaps->data_tx_qs); i++)
			morse_skbq_finish(&yaps->data_tx_qs[i]);
	}

	if (yaps->flags & MORSE_CHIP_IF_FLAGS_COMMAND) {
		morse_skbq_finish(&yaps->cmd_q);
		morse_skbq_finish(&yaps->cmd_resp_q);
	}

	morse_tx_chip_full_timer_finish(yaps);
}

void morse_yaps_show(struct morse_yaps *yaps, struct seq_file *file)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(yaps->data_tx_qs); i++)
		morse_skbq_show(&yaps->data_tx_qs[i], file);
	morse_skbq_show(&yaps->beacon_q, file);
	morse_skbq_show(&yaps->mgmt_q, file);
	morse_skbq_show(&yaps->data_rx_q, file);
	morse_skbq_show(&yaps->cmd_q, file);
	morse_skbq_show(&yaps->cmd_resp_q, file);

	yaps->ops->show(yaps, file);
}

#ifdef MORSE_YAPS_SUPPORTS_BENCHMARK
int morse_yaps_benchmark(struct morse *mors, struct seq_file *file)
{
	int rc = 0;
	const int pkt_len = BENCHMARK_PKT_LEN + sizeof(struct morse_buff_skb_header);
	struct sk_buff *skb;
	struct morse_yaps *yaps = mors->chip_if->yaps;
	struct morse_skb_tx_info tx_info = { 0 };
	struct morse_skbq *mq = skbq_yaps_tc_q_from_aci(mors, MORSE_ACI_VO);
	unsigned long start_time, end_time, max_time;
	unsigned long time_taken_sec, time_taken_msec;
	char *body = kmalloc(BENCHMARK_PKT_LEN, GFP_KERNEL);
	int fc_cnt, tc_cnt;

	if (!body)
		return -EFAULT;

#if KERNEL_VERSION(4, 10, 0) <= LINUX_VERSION_CODE
	get_random_bytes_wait(body, BENCHMARK_PKT_LEN);
#endif

	/* Kick off sending benchmark results */
	atomic_set(&yaps->benchmark_cnt_tc, 0);
	atomic_set(&yaps->benchmark_cnt_fc, 0);
	start_time = jiffies;
	max_time = start_time + msecs_to_jiffies(BENCHMARK_WAIT_MS);
	while (time_before(jiffies, max_time)) {
		skb = dev_alloc_skb(BENCHMARK_PKT_LEN);
		if (!skb)
			goto exit;
		skb_put(skb, BENCHMARK_PKT_LEN);
		memcpy(skb->data, body, BENCHMARK_PKT_LEN);
		skb_set_queue_mapping(skb, IEEE80211_AC_VO);

		/* Wait for space and don't hold the spinlock too much */
		while (morse_skbq_space(mq) < (2 * pkt_len) && time_before(jiffies, max_time))
			usleep_range(5000, 6000);

		rc = morse_skbq_skb_tx(mq, &skb, &tx_info, MORSE_SKB_CHAN_LOOPBACK);
	}

	end_time = jiffies;
	fc_cnt = atomic_read(&yaps->benchmark_cnt_fc);
	tc_cnt = atomic_read(&yaps->benchmark_cnt_tc);

	if (tc_cnt == 0) {
		seq_printf(file, "error %d running benchmark\n", rc);
		seq_printf(file, "bytes sent %d\n", tc_cnt);
		seq_printf(file, "bytes received %d\n", fc_cnt);
		goto exit;
	}

	time_taken_msec = jiffies_to_msecs(end_time - start_time);
	time_taken_sec = time_taken_msec / 1000;
	seq_printf(file, "time taken (ms): %ld\n", time_taken_msec);
	seq_puts(file, "to chip:\n");
	seq_printf(file, "\tpackets per sec: %ld\n", tc_cnt / time_taken_sec);
	seq_printf(file, "\tgoodput (kbit): %ld\n",
		   (tc_cnt * BENCHMARK_PKT_LEN * 8) / time_taken_msec);

	seq_puts(file, "from chip:\n");
	seq_printf(file, "\tpackets per sec: %ld\n", fc_cnt / time_taken_sec);
	seq_printf(file, "\tgoodput (kbit): %ld\n",
		   (fc_cnt * BENCHMARK_PKT_LEN * 8) / time_taken_msec);

	seq_puts(file, "combined:\n");
	seq_printf(file, "\tpackets per sec: %ld\n", (tc_cnt + fc_cnt) / time_taken_sec);
	seq_printf(file, "\tgoodput (kbit): %ld\n",
		   ((tc_cnt + fc_cnt) * BENCHMARK_PKT_LEN * 8) / time_taken_msec);

exit:
	kfree(body);
	return 0;
}
#endif
