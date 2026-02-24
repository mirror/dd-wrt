/*
 * Copyright 2021-2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include <linux/bitops.h>
#include "pager_if_hw.h"
#include "bus.h"
#include "debug.h"
#include "chip_if.h"
#include "pageset_trace.h"

/**
 * Set this #define to control whether or not the pager hardware IRQ
 * is used instead of the HOSTSYNC interrupt.
 */
#define ENABLE_PAGER_HW_IRQ 1

/* Split 32-bits into 2 parts: block + bitmap */
#define MORSE_PAGER_BITS_BLOCK_LEN	(1)
#define MORSE_PAGER_BITS_BITMAP_LEN	(32 - MORSE_PAGER_BITS_BLOCK_LEN)

#define MORSE_PAGER_NUM_BLOCKS		BIT(MORSE_PAGER_BITS_BLOCK_LEN)

struct morse_pager_cache {
	/* Encoded bitmap of pages in pkt memory */
	u32 bitmap[MORSE_PAGER_NUM_BLOCKS];
};

struct morse_pager_hw_aux_data {
	/* These addresses are be the put and pop
	 * for the same instance of the pager hw.
	 */
	u32 put_addr;
	u32 pop_addr;

	struct morse_pager_cache cache;
};

int morse_pager_hw_read_table(struct morse *mors, struct morse_pager_hw_table *tbl_ptr)
{
	int ret;
	const u32 pager_count_addr = mors->cfg->host_table_ptr +
	    offsetof(struct host_table, chip_if) +
	    offsetof(struct morse_chip_if_host_table, pager_count);

	tbl_ptr->addr = mors->cfg->host_table_ptr +
	    offsetof(struct host_table, chip_if) +
	    offsetof(struct morse_chip_if_host_table, pager_table);

	ret = morse_reg32_read(mors, pager_count_addr, &tbl_ptr->count);

	if (ret != 0 && (tbl_ptr->count == 0 || tbl_ptr->addr == 0))
		ret = -EIO;

	return ret;
}

static int morse_pager_hw_get_index_from_page(struct morse_pager *pager,
					      struct morse_page *page,
					      u8 *index)
{
	struct morse_pager_pkt_memory *pkt_memory = &pager->mors->chip_if->pkt_memory;

	if (page->addr < pkt_memory->base_addr ||
			page->addr > pkt_memory->base_addr + pkt_memory->page_len * pkt_memory->num)
		return -EINVAL;

	*index = (page->addr - pkt_memory->page_len_reserved - pkt_memory->base_addr) /
			pkt_memory->page_len;

	return 0;
}

static int morse_pager_hw_get_page_from_index(struct morse_pager *pager,
					      u8 index,
					      struct morse_page *page)
{
	struct morse_pager_pkt_memory *pkt_memory = &pager->mors->chip_if->pkt_memory;

	if (index >= pkt_memory->num)
		return -EINVAL;

	page->addr = pkt_memory->base_addr + (pkt_memory->page_len * index) +
			pkt_memory->page_len_reserved;
	page->size_bytes = pkt_memory->page_len - pkt_memory->page_len_reserved;

	return 0;
}

static void morse_pager_hw_cache_pages(struct morse_pager *pager,
			      struct morse_page *page)
{
	struct morse_pager_hw_aux_data *aux_data =
		(struct morse_pager_hw_aux_data *)pager->aux_data;
	u32 block = page->addr >> MORSE_PAGER_BITS_BITMAP_LEN;

	aux_data->cache.bitmap[block] = page->addr & ~BIT(MORSE_PAGER_BITS_BITMAP_LEN);

	pageset_trace_log(pager, PAGESET_TRACE_EVENT_ID_CACHE_PUT_PAGES,
		hweight32(aux_data->cache.bitmap[block]));
}

static int morse_pager_hw_get_page_from_cache(struct morse_pager *pager,
			      struct morse_page *page)
{
	int ret;
	struct morse_pager_hw_aux_data *aux_data =
		(struct morse_pager_hw_aux_data *)pager->aux_data;
	int block;
	int index;

	for (block = 0; block < ARRAY_SIZE(aux_data->cache.bitmap); ++block) {
		if (aux_data->cache.bitmap[block])
			break;
	}

	if (block >= ARRAY_SIZE(aux_data->cache.bitmap)) {
		ret = -ENOENT;
		goto exit;
	}

	index = ffs(aux_data->cache.bitmap[block]);
	if (!index) {
		ret = -ENOENT;
		goto exit;
	}
	index -= 1;

	aux_data->cache.bitmap[block] &= ~BIT(index);

	ret = morse_pager_hw_get_page_from_index(pager,
		(block * MORSE_PAGER_BITS_BITMAP_LEN) + index, page);

exit:
	pageset_trace_log(pager, PAGESET_TRACE_EVENT_ID_CACHE_GET, (ret) ? 0 : page->addr);
	return ret;
}

static int _morse_pager_hw_pop(struct morse_pager *pager,
			      struct morse_page *page)
{
	struct morse_pager_hw_aux_data *aux_data =
	    (struct morse_pager_hw_aux_data *)pager->aux_data;
	int ret = 0;
	u32 pop_val;

	ret = morse_reg32_read(pager->mors, aux_data->pop_addr, &pop_val);

	if (!ret) {
		/* Pager has no pages left */
		if (pop_val == 0) {
			ret = -EAGAIN;
			goto exit;
		}

		page->addr = pop_val;
		page->size_bytes = pager->page_size_bytes;
	}

exit:
	pageset_trace_log(pager, PAGESET_TRACE_EVENT_ID_POP_PAGE, (ret) ? 0 : page->addr);
	return ret;
}

static int morse_pager_hw_pop(struct morse_pager *pager,
			      struct morse_page *page)
{
	int ret;

	if (!(pager->flags & MORSE_PAGER_FLAGS_FREE) ||
			!pager->parent->mors->chip_if->pkt_memory.num)
		return _morse_pager_hw_pop(pager, page);

	/* Try to return a page from the cache. If unavailable, pop from the HW pager, cache the
	 * pages and return a page from it
	 */
	ret = morse_pager_hw_get_page_from_cache(pager, page);
	if (!ret)
		return ret;

	ret = _morse_pager_hw_pop(pager, page);
	if (ret)
		return ret;

	morse_pager_hw_cache_pages(pager, page);

	ret = morse_pager_hw_get_page_from_cache(pager, page);

	return ret;
}

static int _morse_pager_hw_put(struct morse_pager *pager,
			      struct morse_page *page)
{
	struct morse_pager_hw_aux_data *aux_data =
	    (struct morse_pager_hw_aux_data *)pager->aux_data;
	int ret;

	pageset_trace_log(pager, PAGESET_TRACE_EVENT_ID_PUT_PAGE, page->addr);
	ret = morse_reg32_write(pager->mors, aux_data->put_addr, page->addr);

	if (!ret) {
		page->addr = 0;
		page->size_bytes = 0;
	}
	return ret;
}

static int morse_pager_hw_put(struct morse_pager *pager,
			      struct morse_page *page)
{
	struct morse_pager_hw_aux_data *aux_data =
		(struct morse_pager_hw_aux_data *)pager->aux_data;
	int ret;
	u8 index;
	u32 block;

	if (!(pager->flags & MORSE_PAGER_FLAGS_FREE) ||
			!pager->parent->mors->chip_if->pkt_memory.num)
		return _morse_pager_hw_put(pager, page);

	/* Cache the pages for now. Actual "put" to the pager HW is delayed to notify() */
	ret = morse_pager_hw_get_index_from_page(pager, page, &index);
	if (ret)
		return ret;

	block = index / MORSE_PAGER_BITS_BITMAP_LEN;
	aux_data->cache.bitmap[block] |= BIT(index - (block * MORSE_PAGER_BITS_BITMAP_LEN));
	pageset_trace_log(pager, PAGESET_TRACE_EVENT_ID_STORE_PAGE_BULK,
			  BIT(index - (block * MORSE_PAGER_BITS_BITMAP_LEN)));
	return 0;
}

static void _morse_pager_hw_notify_pager(const struct morse_pager *pager)
{
	struct morse_pager_hw_aux_data *aux_data =
		(struct morse_pager_hw_aux_data *)pager->aux_data;
	u32 block;
	struct morse_page page;

	page.size_bytes = 0;

	for (block = 0; block < ARRAY_SIZE(aux_data->cache.bitmap); ++block) {
		if (!aux_data->cache.bitmap[block])
			continue;

		page.addr = (block << MORSE_PAGER_BITS_BITMAP_LEN) | aux_data->cache.bitmap[block];

		pageset_trace_log(pager, PAGESET_TRACE_EVENT_ID_NOTIFY,
				  hweight32(aux_data->cache.bitmap[block]));
		_morse_pager_hw_put((struct morse_pager *)pager, &page);
		aux_data->cache.bitmap[block] = 0;
	}
}

static int morse_pager_hw_notify_pager(const struct morse_pager *pager)
{
	/* Put the cached pages to to_host free HW pager */
	if ((pager->flags & (MORSE_PAGER_FLAGS_DIR_TO_HOST | MORSE_PAGER_FLAGS_FREE)) &&
			pager->parent->mors->chip_if->pkt_memory.num)
		_morse_pager_hw_notify_pager(pager);

#if ENABLE_PAGER_HW_IRQ
	/**
	 * Popping and putting from pager will generate interrupt on chip,
	 * notify not required.
	 */
	return 0;
#else
	/* For hardware pager interrupts may be generated internally when
	 * a page is pushed/pulled from the pager. This feature is currently
	 * disabled in favor of a hostsync interrupt to make it easier
	 * to batch pages together for AMPDUs.
	 */
	return morse_reg32_write(pager->mors, MORSE_PAGER_TRGR_SET(pager->mors),
					MORSE_PAGER_IRQ_MASK(pager->id));
#endif
}

static int morse_pager_hw_page_write(struct morse_pager *pager,
				     struct morse_page *page, int offset,
				     const char *buff, int num_bytes)
{
	if (offset < 0)
		return -EINVAL;

	if (num_bytes > page->size_bytes)
		return -EMSGSIZE;

	if (page->addr == 0)
		return -EFAULT;

	pageset_trace_log(pager, PAGESET_TRACE_EVENT_ID_WRITE_PAGE, page->addr + offset);
	return morse_dm_write(pager->mors, page->addr + offset, buff, num_bytes);
}

static int morse_pager_hw_page_read(struct morse_pager *pager,
				    struct morse_page *page, int offset, char *buff, int num_bytes)
{
	if (offset < 0)
		return -EINVAL;

	if (num_bytes > page->size_bytes)
		return -EMSGSIZE;

	if (page->addr == 0)
		return -EFAULT;

	pageset_trace_log(pager, PAGESET_TRACE_EVENT_ID_READ_PAGE, page->addr + offset);
	return morse_dm_read(pager->mors, page->addr + offset, buff, num_bytes);
}

const struct morse_pager_ops morse_pager_hw_ops = {
	.put = morse_pager_hw_put,
	.pop = morse_pager_hw_pop,
	.write_page = morse_pager_hw_page_write,
	.read_page = morse_pager_hw_page_read,
	.notify = morse_pager_hw_notify_pager
};

int morse_pager_hw_init(struct morse *mors, struct morse_pager *pager, u32 put_addr, u32 pop_addr)
{
	struct morse_pager_hw_aux_data *aux_data;

	pager->ops = &morse_pager_hw_ops;
	pager->aux_data = kzalloc(sizeof(*aux_data), GFP_KERNEL);
	if (!pager->aux_data)
		return -ENOMEM;

	aux_data = (struct morse_pager_hw_aux_data *)pager->aux_data;
	aux_data->put_addr = put_addr;
	aux_data->pop_addr = pop_addr;

	return 0;
}

void morse_pager_hw_finish(struct morse *mors, struct morse_pager *pager)
{
	kfree(pager->aux_data);
	pager->aux_data = NULL;
	pager->ops = NULL;
}

int morse_pager_hw_pagesets_init(struct morse *mors)
{
	int i = 0, j, ret = 0;
	struct morse_pager *pager;
	struct morse_pager_hw_table tbl_ptr;
	struct morse_pager_hw_entry *pager_entry = NULL;
	struct morse_pager *rx_data = NULL;
	struct morse_pager *rx_return = NULL;
	struct morse_pager *tx_data = NULL;
	struct morse_pager *tx_return = NULL;

	morse_claim_bus(mors);

	ret = morse_pager_hw_read_table(mors, &tbl_ptr);
	if (ret) {
		MORSE_ERR(mors, "morse_pager_hw_read_table failed %d\n", ret);
		goto exit;
	}

	mors->chip_if = kzalloc(sizeof(*mors->chip_if), GFP_KERNEL);
	if (!mors->chip_if) {
		ret = -ENOMEM;
		goto exit;
	}

	mors->chip_if->pagers = kcalloc(tbl_ptr.count, sizeof(struct morse_pager), GFP_KERNEL);
	if (!mors->chip_if->pagers) {
		ret = -ENOMEM;
		goto err_exit;
	}

	mors->chip_if->pager_count = tbl_ptr.count;
	MORSE_INFO(mors, "morse pagers detected %d\n", tbl_ptr.count);

	/* Read entry from chip */
	pager_entry = devm_kcalloc(mors->dev, tbl_ptr.count, sizeof(*pager_entry), GFP_KERNEL);

	ret = morse_dm_read(mors, tbl_ptr.addr, (u8 *)pager_entry,
				sizeof(*pager_entry) * tbl_ptr.count);

	/* First initialise implementation specific data */
	for (pager = mors->chip_if->pagers, i = 0; i < tbl_ptr.count; pager++, i++) {
		if (ret) {
			MORSE_ERR(mors, "%s failed to read table %d\n", __func__, ret);
			goto err_exit;
		}

		ret = morse_pager_hw_init(mors, pager,
					  __le32_to_cpu(pager_entry[i].push_addr),
					  __le32_to_cpu(pager_entry[i].pop_addr));
		if (ret) {
			MORSE_ERR(mors, "morse_pager_hw_init failed %d\n", ret);
			goto err_exit;
		}

		ret = morse_pager_init(mors, pager,
				       __le16_to_cpu(pager_entry[i].page_size),
				       pager_entry[i].flags, i);
		if (ret) {
			MORSE_ERR(mors, "morse_pager_init failed %d\n", ret);
			/* Cleanup for this instance */
			morse_pager_hw_finish(mors, pager);
			/* Set i to clean up other instances */
			i--;
			goto err_exit;
		}
	}

	/* Tie pagers to pageset */
	for (pager = mors->chip_if->pagers, i = 0; i < tbl_ptr.count; pager++, i++) {
		if ((pager->flags & MORSE_PAGER_FLAGS_DIR_TO_HOST) &&
		    (pager->flags & MORSE_PAGER_FLAGS_POPULATED)) {
			rx_data = pager;
		} else if ((pager->flags & MORSE_PAGER_FLAGS_DIR_TO_HOST) &&
			   (pager->flags & MORSE_PAGER_FLAGS_FREE)) {
			rx_return = pager;
		} else if ((pager->flags & MORSE_PAGER_FLAGS_DIR_TO_CHIP) &&
			   (pager->flags & MORSE_PAGER_FLAGS_POPULATED)) {
			tx_data = pager;
		} else if ((pager->flags & MORSE_PAGER_FLAGS_DIR_TO_CHIP) &&
			   (pager->flags & MORSE_PAGER_FLAGS_FREE)) {
			tx_return = pager;
			/* Preload pages */
			set_bit(MORSE_PAGE_RETURN_PEND, &mors->chip_if->event_flags);
		} else {
			MORSE_ERR(mors, "%s Invalid pager flags [0x%x]\n", __func__, pager->flags);
		}
	}
	if (!rx_data || !rx_return || !tx_data || !tx_return) {
		MORSE_ERR(mors, "%s Not all required pagers found\n", __func__);
		ret = -EFAULT;
		goto err_exit;
	}

	/* Setup pagesets */
	mors->chip_if->pagesets = kcalloc(2, sizeof(struct morse_pageset), GFP_KERNEL);
	mors->chip_if->pageset_count = 2;

	ret = morse_pageset_init(mors, &mors->chip_if->pagesets[0],
				 (MORSE_CHIP_IF_FLAGS_DIR_TO_CHIP | MORSE_CHIP_IF_FLAGS_COMMAND |
				  MORSE_CHIP_IF_FLAGS_DATA), tx_data, tx_return);

	if (ret)
		goto err_exit;

	ret = morse_pageset_init(mors, &mors->chip_if->pagesets[1],
				 (MORSE_CHIP_IF_FLAGS_DIR_TO_HOST | MORSE_CHIP_IF_FLAGS_COMMAND |
				  MORSE_CHIP_IF_FLAGS_DATA), rx_data, rx_return);

	if (ret) {
		morse_pageset_finish(&mors->chip_if->pagesets[0]);
		goto err_exit;
	}

	/* Only valid while we only have 2 pagesets */
	mors->chip_if->to_chip_pageset = &mors->chip_if->pagesets[0];
	mors->chip_if->from_chip_pageset = &mors->chip_if->pagesets[1];
	INIT_WORK(&mors->chip_if_work, morse_pagesets_work);
	INIT_WORK(&mors->tx_stale_work, morse_pagesets_stale_tx_work);
	INIT_KFIFO(mors->chip_if->bypass.tx_sts.to_process);
	INIT_KFIFO(mors->chip_if->bypass.cmd_resp.to_process);

	/* pager irq will claim and release the bus */
	morse_release_bus(mors);

	/* Enable interrupts */
	morse_pager_irq_enable(tx_return, true);
	morse_pager_irq_enable(rx_data, true);
	morse_pager_tx_status_irq_enable(mors, true);
	morse_pager_cmd_resp_irq_enable(mors, true);
	morse_hw_enable_stop_notifications(mors, true);
	devm_kfree(mors->dev, pager_entry);

	return ret;

err_exit:
	/* i contains index of pager where initialisation failed */
	for (pager = mors->chip_if->pagers, j = 0; j < i; pager++, j++) {
		morse_pager_finish(pager);
		morse_pager_hw_finish(mors, pager);
	}
	kfree(mors->chip_if->pagers);
	kfree(mors->chip_if->pagesets);
	mors->chip_if->pagers = NULL;
	mors->chip_if->pagesets = NULL;
	kfree(mors->chip_if);
	mors->chip_if = NULL;
	devm_kfree(mors->dev, pager_entry);

exit:
	morse_release_bus(mors);
	return ret;
}

void morse_pager_hw_pagesets_flush_tx_data(struct morse *mors)
{
	int count;
	struct morse_pageset *pageset;

	for (pageset = mors->chip_if->pagesets, count = 0;
	     count < mors->chip_if->pageset_count; pageset++, count++) {
		if ((pageset->flags & MORSE_CHIP_IF_FLAGS_DIR_TO_CHIP) &&
		    (pageset->flags & (MORSE_CHIP_IF_FLAGS_DATA | MORSE_CHIP_IF_FLAGS_BEACON)))
			morse_pageset_flush_tx_data(pageset);
	}
}

void morse_pager_hw_pagesets_finish(struct morse *mors)
{
	int count;
	struct morse_pager *pager;
	struct morse_pageset *pageset;

	cancel_work_sync(&mors->chip_if_work);
	for (pageset = mors->chip_if->pagesets, count = 0;
	     count < mors->chip_if->pageset_count; pageset++, count++) {
		morse_pageset_finish(pageset);
	}
	cancel_work_sync(&mors->tx_stale_work);

	morse_pager_tx_status_irq_enable(mors, false);
	morse_pager_cmd_resp_irq_enable(mors, false);
	for (pager = mors->chip_if->pagers, count = 0;
	     count < mors->chip_if->pager_count; pager++, count++) {
		morse_pager_irq_enable(pager, false);
		morse_pager_finish(pager);
		morse_pager_hw_finish(mors, pager);
	}
	mors->chip_if->pager_count = 0;
	kfree(mors->chip_if->pagers);
	kfree(mors->chip_if->pagesets);
	mors->chip_if->pagers = NULL;
	mors->chip_if->pagesets = NULL;
	mors->chip_if->from_chip_pageset = NULL;
	mors->chip_if->to_chip_pageset = NULL;
	kfree(mors->chip_if);
	mors->chip_if = NULL;
}
