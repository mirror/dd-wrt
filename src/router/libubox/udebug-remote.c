/*
 * udebug - debug ring buffer library
 *
 * Copyright (C) 2023 Felix Fietkau <nbd@nbd.name>
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
#include "udebug-priv.h"

static int
udebug_remote_get_handle(struct udebug *ctx)
{
	struct udebug_client_msg *msg;
	struct udebug_client_msg send_msg = {
		.type = CL_MSG_GET_HANDLE,
	};

	if (ctx->poll_handle >= 0 || !udebug_is_connected(ctx))
		return 0;

	msg = udebug_send_and_wait(ctx, &send_msg, NULL);
	if (!msg)
		return -1;

	ctx->poll_handle = msg->id;
	return 0;
}

struct udebug_remote_buf *udebug_remote_buf_get(struct udebug *ctx, uint32_t id)
{
	struct udebug_remote_buf *rb;
	void *key = (void *)(uintptr_t)id;

	return avl_find_element(&ctx->remote_rings, key, rb, node);
}

int udebug_remote_buf_map(struct udebug *ctx, struct udebug_remote_buf *rb, uint32_t id)
{
	void *key = (void *)(uintptr_t)id;
	struct udebug_client_msg *msg;
	struct udebug_client_msg send_msg = {
		.type = CL_MSG_RING_GET,
		.id = id,
	};
	int fd = -1;

	if (rb->buf.data || !udebug_is_connected(ctx))
		return -1;

	msg = udebug_send_and_wait(ctx, &send_msg, &fd);
	if (!msg || fd < 0)
		return -1;

	if (udebug_buf_open(&rb->buf, fd, msg->ring_size, msg->data_size)) {
		fprintf(stderr, "failed to open fd %d, ring_size=%d, data_size=%d\n", fd, msg->ring_size, msg->data_size);
		close(fd);
		return -1;
	}

	rb->pcap_iface = ~0;
	rb->node.key = key;
	avl_insert(&ctx->remote_rings, &rb->node);

	return 0;
}

void udebug_remote_buf_unmap(struct udebug *ctx, struct udebug_remote_buf *rb)
{
	if (!rb->buf.data)
		return;

	avl_delete(&ctx->remote_rings, &rb->node);
	udebug_buf_free(&rb->buf);
	rb->poll = 0;
	rb->node.key = NULL;
	rb->pcap_iface = ~0;
}

int udebug_remote_buf_set_poll(struct udebug *ctx, struct udebug_remote_buf *rb, bool val)
{
	int handle;

	if (!rb->buf.data)
		return -1;

	if (rb->poll == val)
		return 0;

	rb->poll = val;
	if (!val)
		return 0;

	handle = udebug_remote_get_handle(ctx);
	if (handle < 0)
		return -1;

	__atomic_fetch_or(&rb->buf.hdr->notify, 1UL << handle, __ATOMIC_RELAXED);
	return 0;
}

static void
rbuf_advance_read_head(struct udebug_remote_buf *rb, uint32_t head,
		       uint32_t *data_start)
{
	struct udebug_hdr *hdr = rb->buf.hdr;
	uint32_t min_head = head + 1 - rb->buf.ring_size;
	uint32_t min_data = u32_get(&hdr->data_used) - rb->buf.data_size;
	struct udebug_ptr *last_ptr = udebug_ring_ptr(hdr, head - 1);

	if (!u32_get(&hdr->head_hi) && u32_sub(0, min_head) > 0)
		min_head = 0;

	/* advance head to skip over any entries that are guaranteed
	 * to be overwritten now. final check will be performed after
	 * data copying */

	if (u32_sub(rb->head, min_head) < 0)
		rb->head = min_head;

	for (size_t i = 0; i < rb->buf.ring_size; i++) {
		struct udebug_ptr *ptr = udebug_ring_ptr(hdr, rb->head);

		if (data_start) {
			*data_start = u32_get(&ptr->start);
			__sync_synchronize();
		}

		if (ptr->timestamp > last_ptr->timestamp)
			continue;

		if (u32_sub(ptr->start, min_data) > 0)
			break;

		rb->head++;
	}
}

void udebug_remote_buf_set_start_time(struct udebug_remote_buf *rb, uint64_t ts)
{
	struct udebug_hdr *hdr = rb->buf.hdr;
	uint32_t head = u32_get(&hdr->head);
	uint32_t start = rb->head, end = head;
	uint32_t diff;

	if (!hdr)
		return;

	rbuf_advance_read_head(rb, head, NULL);
	while ((diff = u32_sub(end, start)) > 0) {
		uint32_t cur = start + diff / 2;
		struct udebug_ptr *ptr;

		ptr = udebug_ring_ptr(hdr, cur);
		if (ptr->timestamp > ts)
			end = cur - 1;
		else
			start = cur + 1;
	}

	rb->head = start;
}

void udebug_remote_buf_set_start_offset(struct udebug_remote_buf *rb, uint32_t idx)
{
	if (!rb->buf.hdr)
		return;

	rb->head = rb->buf.hdr->head - idx;
}

void udebug_remote_buf_set_flags(struct udebug_remote_buf *rb, uint64_t mask, uint64_t set)
{
	struct udebug_hdr *hdr = rb->buf.hdr;

	if (!hdr)
		return;

	if ((uintptr_t)mask)
		__atomic_and_fetch(&hdr->flags[0], (uintptr_t)~mask, __ATOMIC_RELAXED);
	if ((uintptr_t)set)
		__atomic_or_fetch(&hdr->flags[0], (uintptr_t)set, __ATOMIC_RELAXED);

	if (sizeof(mask) == sizeof(unsigned long))
		return;

	mask >>= 32;
	if ((uintptr_t)mask)
		__atomic_and_fetch(&hdr->flags[1], (uintptr_t)~mask, __ATOMIC_RELAXED);
	if ((uintptr_t)set)
		__atomic_or_fetch(&hdr->flags[1], (uintptr_t)set, __ATOMIC_RELAXED);
}

struct udebug_snapshot *
udebug_remote_buf_snapshot(struct udebug_remote_buf *rb)
{
	struct udebug_hdr *hdr = rb->buf.hdr;
	struct udebug_ptr *last_ptr;
	uint32_t data_start, data_end, data_used;
	struct udebug_snapshot *s = NULL;
	struct udebug_ptr *ptr_buf, *first_ptr;
	uint32_t data_size, ptr_size;
	uint32_t head, first_idx;
	uint32_t prev_read_head = rb->head;
	void *data_buf;

	if (!hdr)
		return NULL;

	head = u32_get(&hdr->head);
	rbuf_advance_read_head(rb, head, &data_start);
	if (rb->head == head)
		return NULL;

	first_idx = rb->head;
	first_ptr = udebug_ring_ptr(hdr, first_idx);
	last_ptr = udebug_ring_ptr(hdr, head - 1);
	data_end = last_ptr->start + last_ptr->len;

	data_size = data_end - data_start;
	ptr_size = head - rb->head;
	if (data_size > rb->buf.data_size || ptr_size > rb->buf.ring_size) {
		fprintf(stderr, "Invalid data size: %x > %x, %x > %x\n", data_size, (int)rb->buf.data_size, ptr_size, (int)rb->buf.ring_size);
		goto out;
	}

	s = calloc_a(sizeof(*s),
		     &ptr_buf, ptr_size * sizeof(*ptr_buf),
		     &data_buf, data_size);

	s->data = memcpy(data_buf, udebug_buf_ptr(&rb->buf, data_start), data_size);
	s->data_size = data_size;
	s->entries = ptr_buf;
	s->dropped = rb->head - prev_read_head;

	if (first_ptr > last_ptr) {
		struct udebug_ptr *start_ptr = udebug_ring_ptr(hdr, 0);
		struct udebug_ptr *end_ptr = udebug_ring_ptr(hdr, rb->buf.ring_size - 1) + 1;
		uint32_t size = end_ptr - first_ptr;
		memcpy(s->entries, first_ptr, size * sizeof(*s->entries));
		memcpy(s->entries + size, start_ptr, (last_ptr + 1 - start_ptr) * sizeof(*s->entries));
	} else {
		memcpy(s->entries, first_ptr, (last_ptr + 1 - first_ptr) * sizeof(*s->entries));
	}

	/* get a snapshot of the counter that indicates how much data has been
	 * clobbered by newly added entries */
	__sync_synchronize();
	data_used = u32_get(&hdr->data_used) - rb->buf.data_size;

	s->n_entries = head - first_idx;

	rbuf_advance_read_head(rb, head, NULL);
	if (s->n_entries < rb->head - first_idx) {
		free(s);
		s = NULL;
		goto out;
	}

	s->entries += rb->head - first_idx;
	s->n_entries -= rb->head - first_idx;
	while (s->n_entries > 0 &&
	       u32_sub(s->entries[0].start, data_used) < 0) {
		s->entries++;
		s->n_entries--;
		s->dropped++;
	}

	for (size_t i = 0; i < s->n_entries; i++)
		s->entries[i].start -= data_start;

	s->format = hdr->format;
	s->sub_format = hdr->sub_format;
	s->rbuf_idx = (uint32_t)(uintptr_t)rb->node.key;

out:
	rb->head = head;
	return s;
}

bool udebug_snapshot_get_entry(struct udebug_snapshot *s, struct udebug_iter *it, unsigned int entry)
{
	struct udebug_ptr *ptr;

	it->len = 0;
	if (entry >= s->n_entries)
		goto error;

	ptr = &s->entries[entry];
	if (ptr->start > s->data_size || ptr->len > s->data_size ||
	    ptr->start + ptr->len > s->data_size)
		goto error;

	it->s = s;
	it->data = s->data + ptr->start;
	it->len = ptr->len;
	it->timestamp = ptr->timestamp;
	return true;

error:
	it->data = NULL;
	return false;
}

void udebug_iter_start(struct udebug_iter *it, struct udebug_snapshot **s, size_t n)
{
	memset(it, 0, sizeof(*it));

	it->list = s;
	it->n = n;

	for (size_t i = 0; i < it->n; i++)
		it->list[i]->iter_idx = 0;
}

bool udebug_iter_next(struct udebug_iter *it)
{
	while (1) {
		struct udebug_snapshot *s;
		uint64_t cur_ts;
		int cur = -1;

		for (size_t i = 0; i < it->n; i++) {
			struct udebug_ptr *ptr;

			s = it->list[i];
			if (s->iter_idx >= s->n_entries)
				continue;

			ptr = &s->entries[s->iter_idx];
			if (cur >= 0 && ptr->timestamp > cur_ts)
				continue;

			cur = i;
			cur_ts = ptr->timestamp;
		}

		if (cur < 0)
			return false;

		s = it->list[cur];
		it->s_idx = cur;
		if (!udebug_snapshot_get_entry(s, it, s->iter_idx++))
			continue;

		return true;
	}
}
