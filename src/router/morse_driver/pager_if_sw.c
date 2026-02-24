/*
 * Copyright 2021-2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include <linux/kfifo.h>
#include "pager_if_sw.h"
#include "bus.h"
#include "debug.h"
#include "hw.h"
#include "skb_header.h"
#include "chip_if.h"

/*
 * For upper layers MTU=1500, the maximum MPDU will be
 *    S_MPDU Delimiter           4 Octets
 *    MAC header                36 Octets
 *    CCMP header                8 Octets
 *    LLC/SNAP header            8 Octets
 *    Payload                 1500 Octets
 *    FCS                        4 Octets
 *    S_MPDU EOF Delimiter       4 Octets
 *    -----------------------------------
 *    Total                   1564 Octets
 */
#define MORSE_MAX_MPDU_LENGTH (1564)

#define MAX_PAGER_PAGE_LEN (32)

/* Hardcoded page size for sw pager */
#define MM_PAGER_PKT_SIZE \
	(MORSE_MAX_MPDU_LENGTH + sizeof(struct morse_buff_skb_header))

#define MORSE_RB_HEAD_ADDR(pager)	\
	((((struct morse_pager_sw_aux_data *)(pager)->aux_data)->entry_addr) + \
				  offsetof(struct morse_pager_sw_entry, head))
#define MORSE_RB_TAIL_ADDR(pager)	\
	((((struct morse_pager_sw_aux_data *)(pager)->aux_data)->entry_addr) + \
				  offsetof(struct morse_pager_sw_entry, tail))
#define MORSE_AUX_DATA_CACHE(pager) \
	(((struct morse_pager_sw_aux_data *)(pager)->aux_data)->cache)

struct morse_pager_sw_aux_data {
	u32 entry_addr;
	u16 size;

	/* Local copies of base, head, tail */
	u32 base;		/* Ring buffer base address */
	u32 head;		/* Ring buffer head address */
	u32 tail;		/* Ring buffer tail address */

	/**
	 * Used to identify when the above pointers get modified
	 * and need to be written back to the chip.
	 */
	bool head_is_dirty;
	bool tail_is_dirty;

	/** We use a cache to do bulk writes/reads of the pages. */
	 DECLARE_KFIFO(cache, u32, MAX_PAGER_PAGE_LEN);
	/**
	 *  Set to TRUE when there are pages in the cache that need to be
	 * written back.
	 **/
	bool pages_need_put;
};

int morse_pager_sw_read_table(struct morse *mors, struct morse_pager_sw_table *tbl_ptr)
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

static inline int morse_pager_sw_rb_write_tail(const struct morse_pager *pager)
{
	struct morse *mors = pager->mors;
	struct morse_pager_sw_aux_data *aux_data =
	    (struct morse_pager_sw_aux_data *)pager->aux_data;

	morse_reg32_write(mors, MORSE_RB_TAIL_ADDR(pager), aux_data->tail);
	morse_reg32_write(mors, MORSE_PAGER_TRGR_SET(mors), MORSE_PAGER_IRQ_MASK(pager->id));

	aux_data->tail_is_dirty = false;
	return 0;
}

static inline int morse_pager_sw_rb_write_head(const struct morse_pager *pager)
{
	struct morse *mors = pager->mors;
	struct morse_pager_sw_aux_data *aux_data =
	    (struct morse_pager_sw_aux_data *)pager->aux_data;

	morse_reg32_write(mors, MORSE_RB_HEAD_ADDR(pager), aux_data->head);
	morse_reg32_write(mors, MORSE_PAGER_TRGR_SET(mors), MORSE_PAGER_IRQ_MASK(pager->id));

	aux_data->head_is_dirty = false;
	return 0;
}

static inline int morse_pager_sw_rb_read_head(struct morse_pager *pager)
{
	struct morse *mors = pager->mors;
	struct morse_pager_sw_aux_data *aux_data =
	    (struct morse_pager_sw_aux_data *)pager->aux_data;

	morse_reg32_read(mors, MORSE_RB_HEAD_ADDR(pager), &aux_data->head);
	return 0;
}

static inline int morse_pager_sw_rb_read_tail(struct morse_pager *pager)
{
	struct morse *mors = pager->mors;
	struct morse_pager_sw_aux_data *aux_data =
	    (struct morse_pager_sw_aux_data *)pager->aux_data;

	morse_reg32_read(mors, MORSE_RB_TAIL_ADDR(pager), &aux_data->tail);
	return 0;
}

static u32 __morse_pager_sw_count(const struct morse_pager *pager)
{
	struct morse_pager_sw_aux_data *aux_data =
	    (struct morse_pager_sw_aux_data *)pager->aux_data;

	/* Current used in chip */
	u32 head = aux_data->head - aux_data->base;
	u32 tail = aux_data->tail - aux_data->base;
	u32 size = aux_data->size;
	u32 count = (((head) - (tail)) & ((size) - 1));

	return count;
}

static u32 __morse_pager_sw_space(const struct morse_pager *pager)
{
	struct morse_pager_sw_aux_data *aux_data =
	    (struct morse_pager_sw_aux_data *)pager->aux_data;

	return (aux_data->size - 1) - __morse_pager_sw_count(pager);
}

static u32 morse_pager_sw_space_to_end(const struct morse_pager *pager)
{
	struct morse_pager_sw_aux_data *aux_data =
	    (struct morse_pager_sw_aux_data *)pager->aux_data;

	u32 head = aux_data->head;
	u32 tail = aux_data->tail;
	u32 end = aux_data->base + aux_data->size - 1;

	return (head >= tail) ? (end - head + 1) : (tail - head);
}

static u32 morse_pager_sw_count_to_end(const struct morse_pager *pager)
{
	struct morse_pager_sw_aux_data *aux_data =
	    (struct morse_pager_sw_aux_data *)pager->aux_data;

	u32 head = aux_data->head;
	u32 tail = aux_data->tail;
	u32 end = aux_data->base + aux_data->size - 1;

	return (head >= tail) ? (head - tail) : (end - tail + 1);
}

static int morse_pager_sw_data_write(struct morse_pager *pager, u8 *data, u32 len)
{
	int ret = 0;
	u32 spc2end;
	struct morse *mors = pager->mors;
	struct morse_pager_sw_aux_data *aux_data =
	    (struct morse_pager_sw_aux_data *)pager->aux_data;

	morse_pager_sw_rb_read_tail(pager);

	spc2end = morse_pager_sw_space_to_end(pager);

	if (len > __morse_pager_sw_space(pager)) {
		ret = -EAGAIN;
		goto exit;
	}
	if (len <= spc2end) {
		morse_dm_write(mors, aux_data->head, data, len);
		aux_data->head += len;
	} else {
		/* wrap around */
		morse_dm_write(mors, aux_data->head, data, spc2end);
		data += spc2end;
		len -= spc2end;
		morse_dm_write(mors, aux_data->base, data, len);
		aux_data->head = aux_data->base + len;
	}

	aux_data->head_is_dirty = true;

exit:
	return ret;
}

static int morse_pager_sw_data_read(struct morse_pager *pager, u8 *data, u32 len)
{
	int ret = 0;
	int cnt2end;
	struct morse *mors = pager->mors;
	struct morse_pager_sw_aux_data *aux_data =
	    (struct morse_pager_sw_aux_data *)pager->aux_data;

	cnt2end = morse_pager_sw_count_to_end(pager);

	if (len > __morse_pager_sw_count(pager)) {
		ret = -EAGAIN;
		goto exit;
	}
	if (len <= cnt2end) {
		morse_dm_read(mors, aux_data->tail, data, len);
		aux_data->tail += len;
		if (aux_data->tail == (aux_data->base + aux_data->size))
			aux_data->tail = aux_data->base;
	} else {
		/* wrap around */
		morse_dm_read(mors, aux_data->tail, data, cnt2end);
		data += cnt2end;
		len -= cnt2end;
		morse_dm_read(mors, aux_data->base, data, len);
		aux_data->tail = aux_data->base + len;
	}

	aux_data->tail_is_dirty = true;

exit:
	return ret;
}

static int morse_pager_sw_notify_pager(const struct morse_pager *pager)
{
	int ret = -1;
	struct morse_pager_sw_aux_data *aux_data =
	    (struct morse_pager_sw_aux_data *)pager->aux_data;

	if (aux_data->pages_need_put) {
		u32 page_addr;
		size_t to_write = kfifo_len(&MORSE_AUX_DATA_CACHE(pager)) * sizeof(u32);

		if (to_write > 0) {
			int i = 0;
			u32 *buffer = kzalloc(to_write, GFP_KERNEL);

			WARN_ON(!buffer);
			while (kfifo_get(&MORSE_AUX_DATA_CACHE(pager), &page_addr)) {
				buffer[i] = page_addr;
				i++;
			}

			ret = morse_pager_sw_data_write((struct morse_pager *)pager, (u8 *)buffer,
							to_write);
			WARN_ON(ret);
			kfree(buffer);
			aux_data->pages_need_put = false;
		}
	}

	/**
	 * Depending on the type of pager and its direction (to/from chip),
	 * we only update either the cached head/tail through notifying the chip.
	 */
	if (pager->flags & MORSE_PAGER_FLAGS_DIR_TO_CHIP) {
		if (pager->flags & MORSE_PAGER_FLAGS_POPULATED && aux_data->head_is_dirty)
			ret = morse_pager_sw_rb_write_head(pager);
		else if (pager->flags & MORSE_PAGER_FLAGS_FREE && aux_data->tail_is_dirty)
			ret = morse_pager_sw_rb_write_tail(pager);
	} else if (pager->flags & MORSE_PAGER_FLAGS_DIR_TO_HOST) {
		if (pager->flags & MORSE_PAGER_FLAGS_POPULATED && aux_data->tail_is_dirty)
			ret = morse_pager_sw_rb_write_tail(pager);
		else if (pager->flags & MORSE_PAGER_FLAGS_FREE && aux_data->head_is_dirty)
			ret = morse_pager_sw_rb_write_head(pager);
	}

	MORSE_WARN_ON(FEATURE_ID_DEFAULT, aux_data->head_is_dirty);
	MORSE_WARN_ON(FEATURE_ID_DEFAULT, aux_data->tail_is_dirty);

	return 0;
}

static int morse_pager_sw_pop(struct morse_pager *pager, struct morse_page *page)
{
	int ret = 0;
	__le32 page_addr = 0;

	if (kfifo_is_empty(&MORSE_AUX_DATA_CACHE(pager))) {
		int i;
		u32 to_read;
		u32 *buffer;

		/**
		 * If the cache is empty, time to fill it again,
		 * read the head pointer to see how many pages might be available.
		 */
		morse_pager_sw_rb_read_head(pager);
		to_read = __morse_pager_sw_count(pager);
		if (!to_read)
			return -EAGAIN;

		buffer = kzalloc(to_read, GFP_KERNEL);
		if (!buffer)
			return -ENOMEM;

		ret = morse_pager_sw_data_read(pager, (u8 *)buffer, to_read);
		if (ret) {
			kfree(buffer);
			return ret;
		}

		for (i = 0; i < (to_read / sizeof(u32)); i++) {
			ret = kfifo_put(&MORSE_AUX_DATA_CACHE(pager), buffer[i]);
			WARN_ON(!ret);
		}

		kfree(buffer);
	}

	ret = kfifo_get(&MORSE_AUX_DATA_CACHE(pager), (__force u32 *)&page_addr);
	WARN_ON(ret == 0);
	ret = 0;

	if (!ret) {
		page->size_bytes = pager->page_size_bytes;
		page->addr = le32_to_cpu(page_addr);
	}
	return ret;
}

static int morse_pager_sw_put(struct morse_pager *pager, struct morse_page *page)
{
	int ret = 0;
	__le32 page_addr = cpu_to_le32(page->addr);
	struct morse_pager_sw_aux_data *aux_data =
	    (struct morse_pager_sw_aux_data *)pager->aux_data;

	ret = kfifo_put(&MORSE_AUX_DATA_CACHE(pager), (__force u32)page_addr);
	WARN_ON(ret == 0);

	aux_data->pages_need_put = true;
	ret = 0;

	if (!ret) {
		page->addr = 0;
		page->size_bytes = 0;
	}

	return ret;
}

static int morse_pager_sw_page_write(struct morse_pager *pager,
				     struct morse_page *page, int offset,
				     const char *buff, int num_bytes)
{
	int ret;

	if (offset < 0)
		return -EINVAL;

	if (num_bytes > page->size_bytes)
		return -EMSGSIZE;

	if (page->addr == 0)
		return -EFAULT;

	ret = morse_dm_write(pager->mors, page->addr + offset, buff, num_bytes);
	return ret;
}

static int morse_pager_sw_page_read(struct morse_pager *pager,
				    struct morse_page *page, int offset, char *buff, int num_bytes)
{
	int ret;

	if (offset < 0)
		return -EINVAL;

	if (num_bytes > page->size_bytes)
		return -EMSGSIZE;

	if (page->addr == 0)
		return -EFAULT;

	ret = morse_dm_read(pager->mors, page->addr + offset, buff, num_bytes);
	return ret;
}

const struct morse_pager_ops morse_pager_sw_ops = {
	.put = morse_pager_sw_put,
	.pop = morse_pager_sw_pop,
	.write_page = morse_pager_sw_page_write,
	.read_page = morse_pager_sw_page_read,
	.notify = morse_pager_sw_notify_pager,
};

int morse_pager_sw_init(struct morse *mors, struct morse_pager *pager,
			u32 entry_addr, u32 size, u32 base, u32 head, u32 tail)
{
	struct morse_pager_sw_aux_data *aux_data;

	pager->ops = &morse_pager_sw_ops;
	pager->aux_data = kzalloc(sizeof(*aux_data), GFP_KERNEL);
	if (!pager->aux_data)
		return -ENOMEM;

	aux_data = (struct morse_pager_sw_aux_data *)pager->aux_data;
	aux_data->size = size;
	aux_data->base = base;
	aux_data->head = head;
	aux_data->tail = tail;
	aux_data->entry_addr = entry_addr;
	aux_data->head_is_dirty = false;
	aux_data->tail_is_dirty = false;

	INIT_KFIFO(aux_data->cache);
	aux_data->pages_need_put = false;

	return 0;
}

void morse_pager_sw_finish(struct morse *mors, struct morse_pager *pager)
{
	kfree(pager->aux_data);
	pager->aux_data = NULL;
	pager->ops = NULL;
}

int morse_pager_sw_pagesets_init(struct morse *mors)
{
	int i = 0, j, ret = 0;
	struct morse_pager *pager;
	struct morse_pager_sw_table tbl_ptr;
	struct morse_pager_sw_entry pager_entry;
	struct morse_pager *rx_data = NULL;
	struct morse_pager *rx_return = NULL;
	struct morse_pager *tx_data = NULL;
	struct morse_pager *tx_return = NULL;

	morse_claim_bus(mors);
	ret = morse_pager_sw_read_table(mors, &tbl_ptr);
	if (ret) {
		MORSE_ERR(mors, "morse_pager_sw_read_table failed %d\n", ret);
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

	/* First initialise implementation specific data */
	for (pager = mors->chip_if->pagers, i = 0; i < tbl_ptr.count; pager++, i++) {
		/* Read entry from chip */
		const u32 addr = tbl_ptr.addr + i * sizeof(struct morse_pager_sw_entry);

		ret = morse_dm_read(mors, addr, (u8 *)&pager_entry,
				    sizeof(struct morse_pager_sw_entry));
		if (ret) {
			MORSE_ERR(mors, "%s failed to read table %d\n", __func__, ret);
			goto err_exit;
		}

		ret = morse_pager_sw_init(mors, pager, addr,
					  __le16_to_cpu(pager_entry.size),
					  __le32_to_cpu(pager_entry.base),
					  __le32_to_cpu(pager_entry.head),
					  __le32_to_cpu(pager_entry.tail));
		if (ret) {
			MORSE_ERR(mors, "morse_pager_sw_init failed %d\n", ret);
			goto err_exit;
		}

		ret = morse_pager_init(mors, pager, MM_PAGER_PKT_SIZE, pager_entry.flags, i);
		if (ret) {
			MORSE_ERR(mors, "morse_pager_init failed %d\n", ret);
			/* Cleanup for this instance */
			morse_pager_sw_finish(mors, pager);
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

	/* Enable interrupts */
	morse_pager_irq_enable(tx_return, true);
	morse_pager_irq_enable(rx_data, true);
	morse_pager_tx_status_irq_enable(mors, true);
	morse_pager_cmd_resp_irq_enable(mors, true);
	morse_hw_enable_stop_notifications(mors, true);

	morse_release_bus(mors);
	return ret;

err_exit:
	/* i contains index of pager where initialisation failed */
	for (pager = mors->chip_if->pagers, j = 0; j < i; pager++, j++) {
		morse_pager_finish(pager);
		morse_pager_sw_finish(mors, pager);
	}
	kfree(mors->chip_if->pagers);
	kfree(mors->chip_if->pagesets);
	mors->chip_if->pagers = NULL;
	mors->chip_if->pagesets = NULL;
exit:
	morse_release_bus(mors);
	return ret;
}

void morse_pager_sw_pagesets_flush_tx_data(struct morse *mors)
{
	int count;
	struct morse_pageset *pageset;

	for (pageset = mors->chip_if->pagesets, count = 0;
	     count < mors->chip_if->pageset_count; pageset++, count++) {
		if (pageset->flags & MORSE_CHIP_IF_FLAGS_DIR_TO_CHIP &&
		    (pageset->flags & (MORSE_CHIP_IF_FLAGS_DATA | MORSE_CHIP_IF_FLAGS_BEACON)))
			morse_pageset_flush_tx_data(pageset);
	}
}

void morse_pager_sw_pagesets_finish(struct morse *mors)
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
		morse_pager_sw_finish(mors, pager);
	}
	mors->chip_if->pager_count = 0;
	kfree(mors->chip_if->pagers);
	kfree(mors->chip_if->pagesets);
	mors->chip_if->pagers = NULL;
	mors->chip_if->pagesets = NULL;
	mors->chip_if->from_chip_pageset = NULL;
	mors->chip_if->to_chip_pageset = NULL;
}
