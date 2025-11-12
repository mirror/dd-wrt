/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2013-2015 Pablo Neira Ayuso <pablo@netfilter.org>
 */

#include "internal.h"
#include <errno.h>
#include <libmnl/libmnl.h>
#include <libnftnl/batch.h>

struct nftnl_batch {
	uint32_t		num_pages;
	struct nftnl_batch_page	*current_page;
	uint32_t		page_size;
	uint32_t		page_overrun_size;
	struct list_head	page_list;
};

struct nftnl_batch_page {
	struct list_head	head;
	struct mnl_nlmsg_batch	*batch;
};

static struct nftnl_batch_page *nftnl_batch_page_alloc(struct nftnl_batch *batch)
{
	struct nftnl_batch_page *page;
	char *buf;

	page = malloc(sizeof(struct nftnl_batch_page));
	if (page == NULL)
		return NULL;

	buf = malloc(batch->page_size + batch->page_overrun_size);
	if (buf == NULL)
		goto err1;

	page->batch = mnl_nlmsg_batch_start(buf, batch->page_size);
	if (page->batch == NULL)
		goto err2;

	return page;
err2:
	free(buf);
err1:
	free(page);
	return NULL;
}

static void nftnl_batch_add_page(struct nftnl_batch_page *page,
			       struct nftnl_batch *batch)
{
	batch->current_page = page;
	batch->num_pages++;
	list_add_tail(&page->head, &batch->page_list);
}

EXPORT_SYMBOL(nftnl_batch_alloc);
struct nftnl_batch *nftnl_batch_alloc(uint32_t pg_size, uint32_t pg_overrun_size)
{
	struct nftnl_batch *batch;
	struct nftnl_batch_page *page;

	batch = calloc(1, sizeof(struct nftnl_batch));
	if (batch == NULL)
		return NULL;

	batch->page_size = pg_size;
	batch->page_overrun_size = pg_overrun_size;
	INIT_LIST_HEAD(&batch->page_list);

	page = nftnl_batch_page_alloc(batch);
	if (page == NULL)
		goto err1;

	nftnl_batch_add_page(page, batch);
	return batch;
err1:
	free(batch);
	return NULL;
}

EXPORT_SYMBOL(nftnl_batch_free);
void nftnl_batch_free(struct nftnl_batch *batch)
{
	struct nftnl_batch_page *page, *next;

	list_for_each_entry_safe(page, next, &batch->page_list, head) {
		free(mnl_nlmsg_batch_head(page->batch));
		mnl_nlmsg_batch_stop(page->batch);
		free(page);
	}

	free(batch);
}

EXPORT_SYMBOL(nftnl_batch_update);
int nftnl_batch_update(struct nftnl_batch *batch)
{
	struct nftnl_batch_page *page;
	struct nlmsghdr *last_nlh;

	if (mnl_nlmsg_batch_next(batch->current_page->batch))
		return 0;

	last_nlh = nftnl_batch_buffer(batch);

	page = nftnl_batch_page_alloc(batch);
	if (page == NULL)
		goto err1;

	nftnl_batch_add_page(page, batch);

	memcpy(nftnl_batch_buffer(batch), last_nlh, last_nlh->nlmsg_len);
	mnl_nlmsg_batch_next(batch->current_page->batch);

	return 0;
err1:
	return -1;
}

EXPORT_SYMBOL(nftnl_batch_buffer);
void *nftnl_batch_buffer(struct nftnl_batch *batch)
{
	return mnl_nlmsg_batch_current(batch->current_page->batch);
}

EXPORT_SYMBOL(nftnl_batch_buffer_len);
uint32_t nftnl_batch_buffer_len(struct nftnl_batch *batch)
{
	return mnl_nlmsg_batch_size(batch->current_page->batch);
}

EXPORT_SYMBOL(nftnl_batch_iovec_len);
int nftnl_batch_iovec_len(struct nftnl_batch *batch)
{
	int num_pages = batch->num_pages;

	/* Skip last page if it's empty */
	if (mnl_nlmsg_batch_is_empty(batch->current_page->batch))
		num_pages--;

	return num_pages;
}

EXPORT_SYMBOL(nftnl_batch_iovec);
void nftnl_batch_iovec(struct nftnl_batch *batch, struct iovec *iov,
		       uint32_t iovlen)
{
	struct nftnl_batch_page *page;
	int i = 0;

	list_for_each_entry(page, &batch->page_list, head) {
		if (i >= iovlen)
			break;

		iov[i].iov_base = mnl_nlmsg_batch_head(page->batch);
		iov[i].iov_len = mnl_nlmsg_batch_size(page->batch);
		i++;
	}
}
