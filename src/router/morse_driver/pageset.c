/*
 * Copyright 2021-2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include "morse.h"
#include "debug.h"
#include "pageset.h"
#include "pager_if.h"
#include "skb_header.h"
#include "ps.h"
#include "hw.h"
#include "bus.h"
#include "ipmon.h"
#include <linux/gpio.h>
#include "pager_if_hw.h"
#include "pager_if_sw.h"

/* Defined as the most number of MPDUs per AMPDU */
#ifndef MAX_PAGES_PER_TX_TXN
#define MAX_PAGES_PER_TX_TXN	16
#endif

/* 2 full AMPDUs (and also more than the number of RX pages in chip) */
#ifndef MAX_PAGES_PER_RX_TXN
#define MAX_PAGES_PER_RX_TXN	32
#endif

/* How frequently to notify the chip when RX pages are returned */
#ifndef PAGE_RETURN_NOTIFY_INT
#define PAGE_RETURN_NOTIFY_INT	4
#endif

/* Time in milliseconds to wait for the beacon tasklet to queue the beacon to skbq */
#define BEACON_TASKLET_WAITQ_TIMEOUT 1

static int is_pageset_locked(struct morse_pageset *pageset)
{
	return test_bit(0, &pageset->access_lock);
}

static int pageset_lock(struct morse_pageset *pageset)
{
	if (test_and_set_bit_lock(0, &pageset->access_lock))
		return -1;
	return 0;
}

static void pageset_unlock(struct morse_pageset *pageset)
{
	clear_bit_unlock(0, &pageset->access_lock);
}

/* Mappings between sk_buff, skbq and pageset */
static inline struct morse_skbq *skbq_pageset_tc_q_from_aci(struct morse *mors, int aci)
{
	struct morse_pageset *pageset = mors->chip_if->to_chip_pageset;

	if (!pageset)
		return NULL;

	if (aci >= ARRAY_SIZE(pageset->data_qs))
		return NULL;

	return &pageset->data_qs[aci];
}

static inline struct morse_skbq *pageset2cmdq(struct morse_pageset *pageset)
{
	return &pageset->cmd_q;
}

static struct morse_skbq *skbq_pageset_cmd_tc_q(struct morse *mors)
{
	return (mors->chip_if->to_chip_pageset) ? &mors->chip_if->to_chip_pageset->cmd_q : NULL;
}

static struct morse_skbq *skbq_pageset_bcn_tc_q(struct morse *mors)
{
	return (mors->chip_if->to_chip_pageset) ? &mors->chip_if->to_chip_pageset->beacon_q : NULL;
}

static struct morse_skbq *skbq_pageset_mgmt_tc_q(struct morse *mors)
{
	return (mors->chip_if->to_chip_pageset) ? &mors->chip_if->to_chip_pageset->mgmt_q : NULL;
}

static void skbq_pageset_get_tx_qs(struct morse *mors, struct morse_skbq **qs, int *num_qs)
{
	struct morse_pageset *pageset = mors->chip_if->to_chip_pageset;
	*qs = pageset->data_qs;
	*num_qs = PAGESET_TX_SKBQ_MAX;
}

static struct morse_skbq *skbq_pageset_get_rx_data_q(struct morse *mors)
{
	/* On rx, all data frames go through data_q[0] */
	const int rx_data_queue = 0;

	return (mors->chip_if && mors->chip_if->from_chip_pageset) ?
	    &mors->chip_if->from_chip_pageset->data_qs[rx_data_queue] : NULL;
}

static int morse_pageset_get_rx_buffered_count(struct morse *mors)
{
	struct morse_skbq *skbq = skbq_pageset_get_rx_data_q(mors);

	if (!skbq)
		return 0;

	return skbq->skbq.qlen;
}

const struct chip_if_ops morse_pageset_hw_ops = {
	.init = morse_pager_hw_pagesets_init,
	.hw_restarted = morse_pager_hw_pagesets_init,
	.flush_tx_data = morse_pager_hw_pagesets_flush_tx_data,
	.skbq_get_tx_status_pending_count = morse_pagesets_get_tx_status_pending_count,
	.skbq_get_tx_buffered_count = morse_pagesets_get_tx_buffered_count,
	.finish = morse_pager_hw_pagesets_finish,
	.skbq_get_tx_qs = skbq_pageset_get_tx_qs,
	.skbq_bcn_tc_q = skbq_pageset_bcn_tc_q,
	.skbq_mgmt_tc_q = skbq_pageset_mgmt_tc_q,
	.skbq_cmd_tc_q = skbq_pageset_cmd_tc_q,
	.skbq_tc_q_from_aci = skbq_pageset_tc_q_from_aci,
	.chip_if_handle_irq = morse_pager_irq_handler
};

const struct chip_if_ops morse_pageset_sw_ops = {
	.init = morse_pager_sw_pagesets_init,
	.flush_tx_data = morse_pager_sw_pagesets_flush_tx_data,
	.skbq_get_tx_status_pending_count = morse_pagesets_get_tx_status_pending_count,
	.skbq_get_tx_buffered_count = morse_pagesets_get_tx_buffered_count,
	.finish = morse_pager_sw_pagesets_finish,
	.skbq_get_tx_qs = skbq_pageset_get_tx_qs,
	.skbq_bcn_tc_q = skbq_pageset_bcn_tc_q,
	.skbq_mgmt_tc_q = skbq_pageset_mgmt_tc_q,
	.skbq_cmd_tc_q = skbq_pageset_cmd_tc_q,
	.skbq_tc_q_from_aci = skbq_pageset_tc_q_from_aci,
	.chip_if_handle_irq = morse_pager_irq_handler
};

static bool morse_pageset_page_is_cached(struct morse_pageset *pageset, struct morse_page *page)
{
	int i;
	int n_pages;
	struct morse_page pages[CACHED_PAGES_MAX];

	BUILD_BUG_ON(CMD_RSVED_PAGES_MAX > CACHED_PAGES_MAX);
	MORSE_WARN_ON(FEATURE_ID_PAGER, !pageset || !page);
	if (!pageset || !page)
		return false;

	n_pages = kfifo_out_peek(&pageset->reserved_pages, pages, CACHED_PAGES_MAX);
	for (i = 0; i < n_pages; i++) {
		if (pages[i].addr == page->addr)
			return true;
	}

	n_pages = kfifo_out_peek(&pageset->cached_pages, pages, CACHED_PAGES_MAX);
	for (i = 0; i < n_pages; i++) {
		if (pages[i].addr == page->addr)
			return true;
	}

	return false;
}

static void morse_pageset_to_chip_return_handler_no_lock(struct morse *mors)
{
	struct morse_pageset *pageset = mors->chip_if->to_chip_pageset;
	struct morse_pager *pager = pageset->return_pager;
	struct morse_page page;
	int ret;
	bool pager_empty = false;
	unsigned int popped = 0;
	/* Continue to pop until either the pager is exhausted or more than
	 * double the amount of possible cache entries have been popped.
	 * (to max bound this loop in the event of hardware failure)
	 */
	unsigned int max_expected_pops = kfifo_size(&pageset->cached_pages) * 2;

	MORSE_WARN_ON(FEATURE_ID_PAGER, !is_pageset_locked(pageset));

	while (kfifo_len(&pageset->reserved_pages) < CMD_RSVED_PAGES_MAX) {
		if (pager->ops->pop(pager, &page)) {
			pager_empty = true;
			break;
		}

		popped++;
		if (morse_pageset_page_is_cached(pageset, &page))
			continue;

		ret = kfifo_put(&pageset->reserved_pages, page);
		MORSE_WARN_ON(FEATURE_ID_PAGER, !ret);
	}

	if (pager_empty)
		goto exit;

	while (popped < max_expected_pops) {
		if (pager->ops->pop(pager, &page))
			break;

		popped++;
		if (morse_pageset_page_is_cached(pageset, &page))
			continue;

		ret = kfifo_put(&pageset->cached_pages, page);
		MORSE_WARN_ON(FEATURE_ID_PAGER, !ret);
	}

	MORSE_WARN_ON(FEATURE_ID_PAGER, popped >= max_expected_pops);

exit:
	if (popped)
		pager->ops->notify(pager);
}

static void morse_pageset_to_chip_return_handler(struct morse *mors, bool have_lock)
{
	struct morse_pageset *pageset = mors->chip_if->to_chip_pageset;

	if (!have_lock) {
		int ret = 0;

		ret = pageset_lock(pageset);
		if (ret) {
			MORSE_DBG(mors, "%s pageset lock failed %d\n", __func__, ret);
			return;
		}
	}

	morse_pageset_to_chip_return_handler_no_lock(mors);

	if (!have_lock)
		pageset_unlock(pageset);
}

#define BCN_LOSS_CHECK		(500)
#define BCN_LOSS_THRESHOLD	(50)

static unsigned int bcn_page_get;
static unsigned int bcn_page_fail;

/*
 * Some beacons may be lost by design. Report excessive beacon loss.
 */
static void morse_pageset_bcn_loss_monitor(struct morse *mors)
{
	if (bcn_page_fail > BCN_LOSS_THRESHOLD) {
		mors->debug.page_stats.excessive_bcn_loss++;
		MORSE_WARN(mors, "%s failed to send %d of %d beacons\n",
			   __func__, bcn_page_fail, bcn_page_get);
	}
	bcn_page_get = 0;
	bcn_page_fail = 0;
}

static bool morse_pageset_rsved_page_is_avail(struct morse_pageset *pageset, u8 channel,
					      bool have_lock)
{
	struct morse *mors = pageset->mors;

	switch (channel) {
	case MORSE_SKB_CHAN_BEACON:
		bcn_page_get++;
		if (bcn_page_get == BCN_LOSS_CHECK)
			morse_pageset_bcn_loss_monitor(mors);
		/* Always hold at least one reserved page for commands */
		if (kfifo_len(&pageset->reserved_pages) <= 1) {
			bcn_page_fail++;
			mors->debug.page_stats.bcn_no_page++;
			MORSE_DBG(mors, "%s no page available for beacon\n", __func__);
			return false;
		}
		return true;
	case MORSE_SKB_CHAN_COMMAND:
		/*
		 * Always try to write a command. There should only ever be one and there should
		 * always be a reserved page available. It may have been returned after the command
		 * response, so check again if it's not already available.
		 */
		if (kfifo_is_empty(&pageset->reserved_pages)) {
			morse_pageset_to_chip_return_handler(mors, have_lock);
			if (kfifo_is_empty(&pageset->reserved_pages)) {
				mors->debug.page_stats.cmd_no_page++;
				MORSE_ERR(mors, "%s unexpected command page exhaustion\n",
					  __func__);
			} else {
				mors->debug.page_stats.cmd_rsv_page_retry++;
				MORSE_DBG(mors, "%s got command page on second attempt\n",
					  __func__);
			}
		}
		return (kfifo_len(&pageset->reserved_pages) > 0);
	}

	return 0;
}

static int morse_pageset_write(struct morse_pageset *pageset, struct sk_buff *skb)
{
	int ret = 0;
	bool from_rsvd = false;
	struct morse *mors = pageset->mors;
	struct morse_pager *populated_pager = pageset->populated_pager;
	struct morse_page page;
	struct morse_buff_skb_header *hdr = (struct morse_buff_skb_header *)skb->data;
	size_t end_of_skb_pad = (skb->len & 0x03) ? (4 - (unsigned long)(skb->len & 3)) : 0;
	size_t write_len = skb->len + end_of_skb_pad;

	ret = pageset_lock(pageset);
	if (ret) {
		MORSE_DBG(mors, "%s pageset lock failed %d\n", __func__, ret);
		return ret;
	}

	if (morse_pageset_rsved_page_is_avail(pageset, hdr->channel, true)) {
		ret = kfifo_get(&pageset->reserved_pages, &page);
		from_rsvd = true;
	} else {
		ret = kfifo_get(&pageset->cached_pages, &page);
	}

	if (ret <= 0) {
		MORSE_ERR(mors, "%s no pages available\n", __func__);
		ret = -ENOSPC;
		goto exit;
	}

	if (write_len > page.size_bytes) {
		MORSE_ERR(mors, "%s Data larger than pagesize: [%d:%d]\n",
			  __func__, skb->len, page.size_bytes);
		ret = -ENOSPC;
		goto exit;
	}

	if (write_len > (skb->len + skb_tailroom(skb))) {
		/* SKB should be big enough to copy from */
		MORSE_WARN_ON(FEATURE_ID_SKB, 1);
		ret = -ENOSPC;
		goto exit;
	}

	morse_debug_fw_hostif_log_record(mors, true, skb, hdr);

	ret = populated_pager->ops->write_page(populated_pager, &page, 0, skb->data, write_len);
	if (ret) {
		MORSE_ERR(mors, "%s failed to write page: %d\n", __func__, ret);
		/* Put the page back into the cache */
		if (from_rsvd)
			kfifo_put(&pageset->reserved_pages, page);
		else
			kfifo_put(&pageset->cached_pages, page);

		goto exit;
	}

	/* Put the filled page to send it to the chip */
	ret = populated_pager->ops->put(populated_pager, &page);
	if (ret) {
		MORSE_ERR(mors, "%s failed to return page: %d\n", __func__, ret);
		/* Return page to avoid page leak.
		 * Write sync word as 0 so the chip discards it.
		 * Don't not putting this in the return pager to avoid
		 * reading and writing from the same pager, as this would require
		 * additional synchronisation.
		 */
		hdr->sync = 0;
		populated_pager->ops->write_page(populated_pager, &page, 0,
						 (const char *)hdr, sizeof(*hdr));
		populated_pager->ops->put(populated_pager, &page);
		goto exit;
	}

exit:
	pageset_unlock(pageset);

	return ret;
}

static int morse_pageset_read(struct morse_pageset *pageset)
{
	int ret = 0;
	struct morse *mors = pageset->mors;
	struct sk_buff *skb = NULL;
	struct morse_skbq *mq = NULL;
	struct morse_pager *return_pager = pageset->return_pager;
	struct morse_pager *populated_pager = pageset->populated_pager;
	struct morse_chip_if_state *chip_if = mors->chip_if;
	struct morse_page page = { .addr = 0, .size_bytes = 0 };
	struct morse_buff_skb_header *hdr;
	int skb_len;
	int max_checksum_rounds = 2;
	int count = 0;
	bool checksum_valid = !(mors->chip_if->validate_skb_checksum);

	/* Preference CMD responses, then TX statuses from the pager bypass
	 * locations before processing other RX. We treat these page types as
	 * 'first-class' citizens.
	 */
	if (kfifo_len(&chip_if->bypass.cmd_resp.to_process) > 0) {
		ret = kfifo_get(&chip_if->bypass.cmd_resp.to_process, &page.addr);
		WARN_ON(ret == 0);
		page.size_bytes = populated_pager->page_size_bytes;
	} else if (kfifo_len(&chip_if->bypass.tx_sts.to_process) > 0) {
		ret = kfifo_get(&chip_if->bypass.tx_sts.to_process, &page.addr);
		WARN_ON(ret == 0);
		page.size_bytes = populated_pager->page_size_bytes;
	} else {
		/* Pop one page from pager */
		ret = populated_pager->ops->pop(populated_pager, &page);
		if (ret) {
			/* No pages left */
			page.addr = 0;
			goto exit;
		}
	}

	skb_len = round_up(page.addr >> 20, 4);
	page.addr = ((page.addr & 0xFFFFF) | mors->cfg->regs->pager_base_address);

	/* Allocate an skb for the page data, copy header to it */
	skb = dev_alloc_skb(skb_len);
	if (!skb) {
		ret = -ENOMEM;
		goto exit;
	}
	skb_put(skb, skb_len);

	/* Read page data */
	ret = populated_pager->ops->read_page(populated_pager, &page, 0, skb->data, skb_len);

	if (ret) {
		MORSE_ERR(mors, "%s failed to read page: %d\n", __func__, ret);
		/* Error is considered catastrophic, pass up error to stop
		 * more page pops.
		 */
		goto exit;
	}

	hdr = (struct morse_buff_skb_header *)skb->data;

	morse_debug_fw_hostif_log_record(mors, false, skb, hdr);

	/* Validate header */
	if (hdr->sync != MORSE_SKB_HEADER_SYNC) {
		bool chip_owned = (hdr->sync == MORSE_SKB_HEADER_CHIP_OWNED_SYNC);

		MORSE_DBG(mors, "%s sync error:0x%02X page[addr:0x%08x len:%d]\n",
			  __func__, hdr->sync, page.addr, hdr->len);

		if (chip_owned) {
			/* Chip already owns the page, clear page address
			 * to indicate that it should not be returned
			 */
			mors->debug.page_stats.page_owned_by_chip++;
			page.addr = 0;
		}

		/* Not considered catastrophic, continue to read pages out of the
		 * pager.
		 */
		ret = 0;
		goto exit;
	}

	while (!checksum_valid && count < max_checksum_rounds) {
		checksum_valid = morse_validate_skb_checksum(skb->data);
		if (checksum_valid)
			break;

		mors->debug.page_stats.invalid_checksum++;
		/* Read tx status again if the first read is corrupted. There is a tput degradation
		 * if continue to read pages from the pager.
		 */
		if (hdr->channel != MORSE_SKB_CHAN_TX_STATUS)
			break;
		ret =
		    populated_pager->ops->read_page(populated_pager, &page, 0, skb->data, skb_len);
		if (ret)
			break;
		count++;
	}

	if (!checksum_valid) {
		MORSE_DBG(mors,
			  "%s: SKB checksum is invalid, page:[a:0x%08x len:%d] hdr:[c:%02X s:%02X]",
			  __func__, page.addr, skb_len, hdr->channel, hdr->sync);
		if (hdr->channel == MORSE_SKB_CHAN_TX_STATUS)
			mors->debug.page_stats.invalid_tx_status_checksum++;
		goto exit;
	}

	/* SW-3875: seems like sdio read can sometimes go wrong and read first 4-bytes word twice,
	 * overwriting second word (hence, offset will be overwritten with 'sync' byte). Anyway, we
	 * should not expect offset value to be larger than word alignment (max 3 bytes)
	 */
	if (hdr->offset > 3) {
		MORSE_ERR(mors,
			  "%s: corrupted skb header offset [offset=%u], hdr.len %d, page addr: 0x%08x\n",
			  __func__, hdr->offset, hdr->len, page.addr);

		/* Should we actually do that, or just fail the page and go to exit_return_page? */
		hdr->offset = (le16_to_cpu(hdr->len) & 0x03) ?
			(4 - (unsigned long)(le16_to_cpu(hdr->len) & 3)) : 0;
	}

	/* Get correct skbq for the data based on the declared channel */
	switch (hdr->channel) {
	case MORSE_SKB_CHAN_DATA:
	case MORSE_SKB_CHAN_NDP_FRAMES:
	case MORSE_SKB_CHAN_TX_STATUS:
	case MORSE_SKB_CHAN_DATA_NOACK:
	case MORSE_SKB_CHAN_BEACON:
	case MORSE_SKB_CHAN_MGMT:
	case MORSE_SKB_CHAN_LOOPBACK:
		mq = skbq_pageset_get_rx_data_q(mors);
		break;
	case MORSE_SKB_CHAN_COMMAND:
		mq = &pageset->cmd_q;
		break;
	default:
		MORSE_ERR(mors, "%s: unknown channel %d\n", __func__, hdr->channel);
		/* Not considered catastrophic, continue to read pages out of the
		 * pager.
		 */
		ret = 0;
		goto exit;
	}

	if (!mq) {
		MORSE_WARN_ON_ONCE(FEATURE_ID_PAGER, 1);
		goto exit;
	}

	/* Read of page can be greater than actual size of data - so trim */
	skb_len = sizeof(*hdr) + hdr->offset + le16_to_cpu(hdr->len);
	skb_trim(skb, skb_len);

#ifdef CONFIG_MORSE_IPMON
	if (hdr->channel == MORSE_SKB_CHAN_DATA) {
		static u64 time_start;

		morse_ipmon(&time_start, skb, skb->data + sizeof(*hdr),
			    le16_to_cpu(hdr->len), IPMON_LOC_SERVER_DRV, 0);
	}
#endif

	ret = morse_skbq_put(mq, skb);

	/* Unconditionally queue network work to process RX page. Either
	 * insertion into the mq was successful, or the mq is currently full
	 * and requires processing anyway.
	 */
	queue_work(mors->net_wq, &mq->dispatch_work);

	if (ret) {
		MORSE_ERR(mors, "%s: Failed to insert skb into mq[channel:%d]\n", __func__,
			  hdr->channel);

		/* Considered catastrophic, return error code to stop page pop
		 * operations and more data getting lost.
		 */
		ret = -ENOMEM;
		goto exit;
	}

	/* Successful in receiving page/skb. Do not free the page as it now
	 * is the responsibility of mq.
	 */
	skb = NULL;

exit:
	/* If the SKB did not successfully make it into an MQ, it must be freed */
	dev_kfree_skb(skb);

	if (page.addr) {
		/* Put the emptied page to send it back to the chip */
		ret = return_pager->ops->put(return_pager, &page);
		if (ret)
			MORSE_ERR(mors, "%s: return page failed: %d\n", __func__, ret);
	}

	return ret;
}

/**
 * Determine how many pages are available for sending packets to the firmware.
 * - Always use 1 for commands. There should only ever be one command in progress at a
 *   time and there is a reserved page for it. If anything goes wrong the command will
 *   be dropped.
 */
static int morse_pageset_num_pages(struct morse_pageset *pageset, struct sk_buff *skb)
{
	struct morse_buff_skb_header *hdr = (struct morse_buff_skb_header *)skb->data;
	int num_pages = 0;

	if (hdr->channel == MORSE_SKB_CHAN_COMMAND) {
		num_pages = min(CMD_RSVED_CMD_PAGES_MAX,
				(int)kfifo_len(&pageset->reserved_pages) +
				(int)kfifo_len(&pageset->cached_pages));
	} else {
		if (morse_pageset_rsved_page_is_avail(pageset, hdr->channel, false))
			num_pages = (CMD_RSVED_PAGES_MAX - CMD_RSVED_CMD_PAGES_MAX);

		num_pages = min(MAX_PAGES_PER_TX_TXN,
				num_pages + (int)kfifo_len(&pageset->cached_pages));
	}

	return num_pages;
}

static void morse_pageset_tx(struct morse_pageset *pageset, struct morse_skbq *mq)
{
	int ret = 0;
	int num_pages;
	int num_items = 0;
	struct sk_buff *skb;
	struct sk_buff_head skbq_to_send;
	struct sk_buff_head skbq_sent;
	struct sk_buff_head skbq_failed;
	struct sk_buff *pfirst, *pnext;
	struct morse *mors = pageset->mors;
	struct morse_buff_skb_header *hdr;

	spin_lock_bh(&mq->lock);
	skb = skb_peek(&mq->skbq);
	if (skb)
		num_pages = morse_pageset_num_pages(pageset, skb);
	spin_unlock_bh(&mq->lock);

	if (!skb)
		return;

	__skb_queue_head_init(&skbq_to_send);
	__skb_queue_head_init(&skbq_sent);
	__skb_queue_head_init(&skbq_failed);

	if (mq == &pageset->cmd_q)
		/* Purge timed-out commands (this should not happen) */
		morse_skbq_purge(mq, &mq->pending);
	else if (mq == &pageset->mgmt_q && mq->skbq.qlen > 0)
		/* Purge old mgmt frames that have not been sent due to congestion */
		morse_skbq_purge_aged(mors, mq);

	if (num_pages > 0)
		num_items = morse_skbq_deq_num_items(mq, &skbq_to_send, num_pages);

	skb_queue_walk_safe(&skbq_to_send, pfirst, pnext) {
		if (num_pages) {
			ret = morse_pageset_write(pageset, pfirst);
		} else {
			mors->debug.page_stats.no_page++;
			MORSE_ERR(mors, "%s no pages available\n", __func__);
			ret = -ENOSPC;
		}
		if (ret == 0) {
			hdr = (struct morse_buff_skb_header *)pfirst->data;
			switch (hdr->channel) {
			case MORSE_SKB_CHAN_COMMAND:
				mors->debug.page_stats.cmd_tx++;
				break;
			case MORSE_SKB_CHAN_BEACON:
				mors->debug.page_stats.bcn_tx++;
				break;
			case MORSE_SKB_CHAN_MGMT:
				mors->debug.page_stats.mgmt_tx++;
				break;
			default:
				mors->debug.page_stats.data_tx++;
				break;
			}
			num_pages--;
			__skb_unlink(pfirst, &skbq_to_send);
			__skb_queue_tail(&skbq_sent, pfirst);
		} else {
			__skb_unlink(pfirst, &skbq_to_send);
			__skb_queue_tail(&skbq_failed, pfirst);
		}
	}

	if (skbq_failed.qlen > 0) {
		mors->debug.page_stats.write_fail += skbq_failed.qlen;
		MORSE_ERR(mors, "%s could not write %d pkts - rc=%d items=%d pages=%d",
			  __func__, skbq_failed.qlen, ret, num_items, num_pages);
		morse_skbq_purge(NULL, &skbq_failed);
	}

	if (skbq_sent.qlen > 0) {
		morse_skbq_tx_complete(mq, &skbq_sent);
		pageset->populated_pager->ops->notify(pageset->populated_pager);
	}
}

/* Returns true if there are TX data pages waiting to be sent */
static bool morse_pageset_tx_data_handler(struct morse_pageset *pageset)
{
	s16 aci;
	u32 count = 0;
	struct morse *mors = pageset->mors;

	for (aci = MORSE_ACI_VO; aci >= 0; aci--) {
		struct morse_skbq *data_q = skbq_pageset_tc_q_from_aci(mors, aci);

		if (!morse_is_data_tx_allowed(mors))
			break;

		morse_pageset_tx(pageset, data_q);

		count += morse_skbq_count(data_q);

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
static bool morse_pageset_tx_cmd_handler(struct morse_pageset *pageset)
{
	struct morse_skbq *cmd_q = pageset2cmdq(pageset);

	morse_pageset_tx(pageset, cmd_q);

	return (morse_skbq_count(cmd_q) > 0);
}

static bool morse_pageset_tx_beacon_handler(struct morse_pageset *pageset)
{
	struct morse_skbq *beacon_q = &pageset->beacon_q;

	morse_pageset_tx(pageset, beacon_q);

	return (morse_skbq_count(beacon_q) > 0);
}

static bool morse_pageset_tx_mgmt_handler(struct morse_pageset *pageset)
{
	struct morse_skbq *mgmt_q = &pageset->mgmt_q;

	morse_pageset_tx(pageset, mgmt_q);

	return (morse_skbq_count(mgmt_q) > 0);
}

/**
 * morse_is_beacon_request_interrupt_set() -  Checks if beacon request interrupt
 * bit is set.
 *
 * @mors: morse chip struct
 *
 * Return: true if beacon request interrupt bit is set.
 */
static inline bool morse_is_beacon_request_interrupt_set(struct morse *mors)
{
	u32 status1 = 0;

	morse_reg32_read(mors, MORSE_REG_INT1_STS(mors), &status1);
	if (status1 & MORSE_INT_BEACON_VIF_MASK_ALL)
		return true;

	return false;
}

/* Returns true if there are populated RX pages left in the device */
static bool morse_pageset_rx_handler(struct morse_pageset *pageset,
	bool *is_beacon_pending)
{
	int ret = 0;
	int count = 0;
	bool return_notify_req = false;
	bool do_beacon_irq_check = is_beacon_pending;

	MORSE_WARN_ON(FEATURE_ID_PAGER, is_pageset_locked(pageset));

	/* Read as many pages out as are available up to the RX limit */
	do {
		ret = morse_pageset_read(pageset);
		count++;
		return_notify_req = true;
		if ((count % PAGE_RETURN_NOTIFY_INT) == 0) {
			pageset->return_pager->ops->notify(pageset->return_pager);
			return_notify_req = false;

			if (do_beacon_irq_check &&
			    (morse_is_beacon_request_interrupt_set(pageset->mors))) {
				*is_beacon_pending = true;
				break;
			}
		}
	} while ((count < MAX_PAGES_PER_RX_TXN) && (ret == 0));

	MORSE_WARN_ON(FEATURE_ID_PAGER,
		      kfifo_len(&pageset->mors->chip_if->bypass.tx_sts.to_process) > 0);
	MORSE_WARN_ON(FEATURE_ID_PAGER,
		      kfifo_len(&pageset->mors->chip_if->bypass.cmd_resp.to_process) > 0);

	if (return_notify_req)
		pageset->return_pager->ops->notify(pageset->return_pager);

	pageset->populated_pager->ops->notify(pageset->populated_pager);

	if (ret == -ENOMEM || count == MAX_PAGES_PER_RX_TXN ||
	    (do_beacon_irq_check && *is_beacon_pending))
		return true;
	else
		return false;
}

void morse_pagesets_stale_tx_work(struct work_struct *work)
{
	int i;
	int flushed = 0;
	struct morse *mors = container_of(work, struct morse, tx_stale_work);
	struct morse_pageset *tx_pageset;

	if (!mors->chip_if || !mors->chip_if->to_chip_pageset || !mors->stale_status.enabled)
		return;

	tx_pageset = mors->chip_if->to_chip_pageset;
	flushed += morse_skbq_check_for_stale_tx(mors, &tx_pageset->beacon_q);
	flushed += morse_skbq_check_for_stale_tx(mors, &tx_pageset->mgmt_q);

	for (i = 0; i < ARRAY_SIZE(tx_pageset->data_qs); i++)
		flushed += morse_skbq_check_for_stale_tx(mors, &tx_pageset->data_qs[i]);

	if (flushed) {
		MORSE_DBG(mors, "%s: Flushed %d stale TX SKBs\n", __func__, flushed);

		if (mors->ps.enable &&
		    !mors->ps.suspended && (morse_pagesets_get_tx_buffered_count(mors) == 0)) {
			/* Evaluate ps to check if it was gated on a stale tx status */
			queue_delayed_work(mors->chip_wq, &mors->ps.delayed_eval_work, 0);
		}
	}
}

void morse_pagesets_work(struct work_struct *work)
{
	struct morse *mors = container_of(work,
					  struct morse, chip_if_work);
	int ps_bus_timeout_ms = 0;
	unsigned long flags_on_entry = mors->chip_if->event_flags;
	unsigned long *flags = &mors->chip_if->event_flags;
	int rx_buffered_on_entry = morse_pageset_get_rx_buffered_count(mors);
	bool is_beacon_pending = false;

	if (!flags_on_entry)
		return;

	/* Don't attempt to interact with device once it becomes unresponsive */
	if (test_bit(MORSE_STATE_FLAG_CHIP_UNRESPONSIVE, &mors->state_flags))
		return;

	/* Disable power save in case it is running */
	morse_ps_disable(mors);
	morse_claim_bus(mors);

	/* Tx beacons first */
	if (test_and_clear_bit(MORSE_TX_BEACON_PEND, flags)) {
		if (morse_pageset_tx_beacon_handler(mors->chip_if->to_chip_pageset))
			set_bit(MORSE_TX_BEACON_PEND, flags);
	}

	/* Handle any populated RX pages from chip first to
	 * avoid dropping pkts due to full on-chip buffers.
	 * Check if all pages were removed, set event flags if not.
	 */
	if (test_and_clear_bit(MORSE_RX_PEND, flags)) {
		/* Check for beacon requests from target if AP interface exists */
		if (morse_pageset_rx_handler(mors->chip_if->from_chip_pageset,
				mors->num_of_ap_interfaces > 0 ? &is_beacon_pending : NULL))
			set_bit(MORSE_RX_PEND, flags);

		if (is_beacon_pending) {
			mors->beacon_queued = false;
			morse_release_bus(mors);
			wait_event_interruptible_timeout(mors->beacon_tasklet_waitq,
				mors->beacon_queued,
				msecs_to_jiffies(BEACON_TASKLET_WAITQ_TIMEOUT));
			morse_claim_bus(mors);
		}
	}

	/* Handle any free TX pages being returned so caches are refilled */
	if (test_and_clear_bit(MORSE_PAGE_RETURN_PEND, flags))
		morse_pageset_to_chip_return_handler(mors, false);

	/* TX any commands before anything else */
	if (test_and_clear_bit(MORSE_TX_COMMAND_PEND, flags)) {
		if (morse_pageset_tx_cmd_handler(mors->chip_if->to_chip_pageset))
			set_bit(MORSE_TX_COMMAND_PEND, flags);
	}

	/* TX beacons before considering mgmt/data */
	if (test_and_clear_bit(MORSE_TX_BEACON_PEND, flags)) {
		if (morse_pageset_tx_beacon_handler(mors->chip_if->to_chip_pageset))
			set_bit(MORSE_TX_BEACON_PEND, flags);
	}

	/* Process Rx buffers again if rx pages processing is stopped for any pending beacon */
	if (test_and_clear_bit(MORSE_RX_PEND, flags) && is_beacon_pending) {
		if (morse_pageset_rx_handler(mors->chip_if->from_chip_pageset, NULL))
			set_bit(MORSE_RX_PEND, flags);
	}

	/* TX mgmt before considering data */
	if (test_and_clear_bit(MORSE_TX_MGMT_PEND, flags)) {
		if (morse_pageset_tx_mgmt_handler(mors->chip_if->to_chip_pageset))
			set_bit(MORSE_TX_MGMT_PEND, flags);
	}

	/* Pause TX data Qs */
	if (test_and_clear_bit(MORSE_DATA_TRAFFIC_PAUSE_PEND, flags)) {
		if (test_and_clear_bit(MORSE_DATA_TRAFFIC_RESUME_PEND, flags))
			MORSE_ERR_RATELIMITED(mors,
					      "Latency to handle traffic pause is too great\n");
		else
			morse_skbq_data_traffic_pause(mors);
	}

	/* Resume TX data Qs  */
	if (test_and_clear_bit(MORSE_DATA_TRAFFIC_RESUME_PEND, flags)) {
		if (test_bit(MORSE_DATA_TRAFFIC_PAUSE_PEND, flags))
			MORSE_ERR_RATELIMITED(mors,
					      "Latency to handle traffic resume is too great\n");

		morse_skbq_data_traffic_resume(mors);
	}

	/* Finally TX any data */
	if (test_and_clear_bit(MORSE_TX_DATA_PEND, flags)) {
		if (morse_pageset_tx_data_handler(mors->chip_if->to_chip_pageset))
			set_bit(MORSE_TX_DATA_PEND, flags);
	}

	/* Calculate timeout to disable the shared bus with the chip. For any TX that was
	 * pushed down, or non-CMD RX that came up from the chip - increase the timeout as
	 * 'network activity' was seen.
	 */
	if (test_bit(MORSE_TX_DATA_PEND, &flags_on_entry) ||
	    test_bit(MORSE_TX_MGMT_PEND, &flags_on_entry) ||
	    morse_pageset_get_rx_buffered_count(mors) > rx_buffered_on_entry)
		ps_bus_timeout_ms = max(ps_bus_timeout_ms, morse_network_bus_timeout(mors));

	if (test_and_clear_bit(MORSE_UPDATE_HW_CLOCK_REFERENCE, flags))
		morse_hw_clock_update(mors);

	if (ps_bus_timeout_ms)
		morse_ps_bus_activity(mors, ps_bus_timeout_ms);

	/* Disable power save in case it is running */
	morse_release_bus(mors);
	morse_ps_enable(mors);

	/* A single RX event may represent the reception
	 * of many pages. We might not be able to process all these pages
	 * immediately. As such, manually requeue a chip work item - the firmware
	 * will not do this again.
	 *
	 * This is not required for TX events, as each single transmission will
	 * schedule a work event.
	 */
	if (test_bit(MORSE_RX_PEND, flags))
		queue_work(mors->chip_wq, &mors->chip_if_work);
}

void morse_pageset_show(struct morse *mors, struct morse_pageset *pageset, struct seq_file *file)
{
	int i;

	seq_printf(file, "flags:0x%01x reserved=%d cached=%d\n",
		   pageset->flags,
		   kfifo_len(&pageset->reserved_pages), kfifo_len(&pageset->cached_pages));

	morse_pager_show(pageset->mors, pageset->populated_pager, file);
	morse_pager_show(pageset->mors, pageset->return_pager, file);
	for (i = 0; i < ARRAY_SIZE(pageset->data_qs); i++)
		morse_skbq_show(&pageset->data_qs[i], file);
	morse_skbq_show(&pageset->mgmt_q, file);
	morse_skbq_show(&pageset->beacon_q, file);
	morse_skbq_show(&pageset->cmd_q, file);
}

int morse_pageset_init(struct morse *mors, struct morse_pageset *pageset,
		       u8 flags,
		       struct morse_pager *populated_pager, struct morse_pager *return_pager)
{
	int i;
	u16 chip_if_direction_flag;

	pageset->mors = mors;
	pageset->flags = flags;
	pageset->populated_pager = populated_pager;
	pageset->return_pager = return_pager;
	mors->chip_if->active_chip_if = MORSE_CHIP_IF_PAGESET;

	INIT_KFIFO(pageset->reserved_pages);
	INIT_KFIFO(pageset->cached_pages);

	chip_if_direction_flag = pageset->flags & MORSE_PAGER_FLAGS_DIR_TO_HOST ?
				 MORSE_CHIP_IF_FLAGS_DIR_TO_HOST :
				 MORSE_CHIP_IF_FLAGS_DIR_TO_CHIP;

	if (pageset->flags & MORSE_CHIP_IF_FLAGS_DATA) {
		morse_skbq_init(mors,
				&pageset->beacon_q,
				MORSE_CHIP_IF_FLAGS_DATA | chip_if_direction_flag);

		morse_skbq_init(mors,
				&pageset->mgmt_q,
				MORSE_CHIP_IF_FLAGS_DATA | chip_if_direction_flag);

		for (i = 0; i < ARRAY_SIZE(pageset->data_qs); i++)
			morse_skbq_init(mors,
					&pageset->data_qs[i],
					MORSE_CHIP_IF_FLAGS_DATA | chip_if_direction_flag);
	}

	if (pageset->flags & MORSE_CHIP_IF_FLAGS_COMMAND)
		morse_skbq_init(mors,
				&pageset->cmd_q,
				MORSE_CHIP_IF_FLAGS_COMMAND | chip_if_direction_flag);

	populated_pager->parent = pageset;
	return_pager->parent = pageset;

	pageset_trace_init(pageset);
	pageset_trace_log(populated_pager, PAGESET_TRACE_EVENT_ID_INIT, 0);
	pageset_trace_log(return_pager, PAGESET_TRACE_EVENT_ID_INIT, 0);
	return 0;
}

void morse_pageset_finish(struct morse_pageset *pageset)
{
	int i;

	pageset->return_pager = NULL;
	pageset->populated_pager = NULL;

	if (pageset->flags & MORSE_CHIP_IF_FLAGS_DATA) {
		morse_skbq_finish(&pageset->beacon_q);
		morse_skbq_finish(&pageset->mgmt_q);
		for (i = 0; i < ARRAY_SIZE(pageset->data_qs); i++)
			morse_skbq_finish(&pageset->data_qs[i]);
	}

	if (pageset->flags & MORSE_CHIP_IF_FLAGS_COMMAND)
		morse_skbq_finish(&pageset->cmd_q);
}

void morse_pageset_flush_tx_data(struct morse_pageset *pageset)
{
	int i;

	if (!(pageset->flags & (MORSE_CHIP_IF_FLAGS_DATA | MORSE_CHIP_IF_FLAGS_BEACON)) ||
			!(pageset->flags & MORSE_CHIP_IF_FLAGS_DIR_TO_CHIP)) {
		MORSE_WARN_ON_ONCE(FEATURE_ID_PAGER, 1);
		return;
	}

	morse_skbq_tx_flush(&pageset->beacon_q);
	morse_skbq_tx_flush(&pageset->mgmt_q);
	for (i = 0; i < ARRAY_SIZE(pageset->data_qs); i++)
		morse_skbq_tx_flush(&pageset->data_qs[i]);
}

int morse_pagesets_get_tx_status_pending_count(struct morse *mors)
{
	int i = 0;
	int count = 0;
	struct morse_pageset *tx_pageset;

	if (!mors->chip_if || !mors->chip_if->to_chip_pageset)
		return 0;

	tx_pageset = mors->chip_if->to_chip_pageset;
	if (!(tx_pageset->flags & MORSE_CHIP_IF_FLAGS_DIR_TO_CHIP)) {
		MORSE_WARN_ON_ONCE(FEATURE_ID_PAGER, 1);
		return 0;
	}

	count += tx_pageset->beacon_q.pending.qlen;
	count += tx_pageset->mgmt_q.pending.qlen;
	count += tx_pageset->cmd_q.pending.qlen;

	for (i = 0; i < ARRAY_SIZE(tx_pageset->data_qs); i++)
		count += tx_pageset->data_qs[i].pending.qlen;

	return count;
}

int morse_pagesets_get_tx_buffered_count(struct morse *mors)
{
	int i = 0;
	int count = 0;
	struct morse_pageset *tx_pageset;

	if (!mors->chip_if || !mors->chip_if->to_chip_pageset)
		return 0;

	tx_pageset = mors->chip_if->to_chip_pageset;
	if (!(tx_pageset->flags & MORSE_CHIP_IF_FLAGS_DIR_TO_CHIP)) {
		MORSE_WARN_ON_ONCE(FEATURE_ID_PAGER, 1);
		return 0;
	}

	count += tx_pageset->beacon_q.skbq.qlen + tx_pageset->beacon_q.pending.qlen;
	count += tx_pageset->mgmt_q.skbq.qlen + tx_pageset->mgmt_q.pending.qlen;
	count += tx_pageset->cmd_q.skbq.qlen + tx_pageset->cmd_q.pending.qlen;

	for (i = 0; i < ARRAY_SIZE(tx_pageset->data_qs); i++)
		count += morse_skbq_count_tx_ready(&tx_pageset->data_qs[i]) +
		    tx_pageset->data_qs[i].pending.qlen;

	return count;
}
