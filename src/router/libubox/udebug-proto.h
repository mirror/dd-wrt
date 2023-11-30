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
#ifndef __UDEBUG_PROTO_H
#define __UDEBUG_PROTO_H

#include "udebug.h"

struct udebug_hdr {
	uint32_t ring_size;
	uint32_t data_size;

	uint32_t format;
	uint32_t sub_format;

	uintptr_t flags[8 / sizeof(uintptr_t)];
	uintptr_t notify;

	uint32_t head_hi;
	uint32_t head;
	uint32_t data_head;
	uint32_t data_used;
};

enum udebug_client_msg_type {
	CL_MSG_RING_ADD,
	CL_MSG_RING_REMOVE,
	CL_MSG_RING_NOTIFY,
	CL_MSG_GET_HANDLE,
	CL_MSG_RING_GET,
	CL_MSG_ERROR,
};

struct udebug_client_msg {
	uint8_t type;
	uint8_t _pad[3];
	uint32_t id;
	union {
		struct {
			uint32_t ring_size, data_size;
		};
		uint32_t notify_mask;
	};
} __attribute__((packed, aligned(4)));

static inline struct udebug_ptr *
udebug_ring_ptr(struct udebug_hdr *hdr, uint32_t idx)
{
	struct udebug_ptr *ring = (struct udebug_ptr *)&hdr[1];
	return &ring[idx & (hdr->ring_size - 1)];
}

static inline void *udebug_buf_ptr(struct udebug_buf *buf, uint32_t ofs)
{
	return buf->data + (ofs & (buf->data_size - 1));
}

#endif
