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
#ifndef __UDEBUG_UTIL_H
#define __UDEBUG_UTIL_H

#include "blobmsg.h"
#include "udebug.h"
#include "udebug-proto.h"

#define UDEBUG_TIMEOUT	1000

__hidden int udebug_buf_open(struct udebug_buf *buf, int fd, uint32_t ring_size, uint32_t data_size);
__hidden struct udebug_client_msg *__udebug_poll(struct udebug *ctx, int *fd, bool wait);
__hidden void udebug_send_msg(struct udebug *ctx, struct udebug_client_msg *msg,
		     struct blob_attr *meta, int fd);

static inline int32_t u32_sub(uint32_t a, uint32_t b)
{
	return a - b;
}

static inline int32_t u32_max(uint32_t a, uint32_t b)
{
	return u32_sub(a, b) > 0 ? a : b;
}

static inline void u32_set(void *ptr, uint32_t val)
{
	volatile uint32_t *v = ptr;
	*v = val;
}

static inline uint32_t u32_get(void *ptr)
{
	volatile uint32_t *v = ptr;
	return *v;
}

#endif
