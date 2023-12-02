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
#ifndef __UDEBUG_RINGBUF_H
#define __UDEBUG_RINGBUF_H

#include <sys/types.h>
#include <stdint.h>
#include <stdarg.h>

#include "list.h"
#include "uloop.h"
#include "avl.h"

#define UDEBUG_SOCK_NAME	"/var/run/udebug.sock"

enum udebug_format {
	UDEBUG_FORMAT_PACKET,
	UDEBUG_FORMAT_STRING,
	UDEBUG_FORMAT_BLOBMSG,
};

enum {
	UDEBUG_DLT_ETHERNET = 1,
	UDEBUG_DLT_PPP = 50,
	UDEBUG_DLT_RAW_IP = 101,
	UDEBUG_DLT_IEEE_802_11 = 105,
	UDEBUG_DLT_IEEE_802_11_RADIOTAP = 127,
	UDEBUG_DLT_NETLINK = 253,
};

enum udebug_meta_type {
	UDEBUG_META_IFACE_NAME,
	UDEBUG_META_IFACE_DESC,
	__UDEBUG_META_MAX
};

#define UDEBUG_TS_MSEC  1000ULL
#define UDEBUG_TS_SEC	(1000ULL * UDEBUG_TS_MSEC)

struct udebug;
struct udebug_hdr;

struct udebug_buf_flag {
	const char *name;
	uint64_t mask;
};

struct udebug_buf_meta {
	const char *name;
	enum udebug_format format;
	uint32_t sub_format; /* linktype for UDEBUG_FORMAT_PACKET */
	const struct udebug_buf_flag *flags;
	unsigned int n_flags;
};

struct udebug_buf {
	struct udebug *ctx;
	const struct udebug_buf_meta *meta;
	uint32_t id;

	struct list_head list;

	struct udebug_hdr *hdr;
	void *data;
	size_t data_size;
	size_t head_size;
	size_t ring_size;
	int fd;
};

struct udebug_packet_info {
	const char *attr[__UDEBUG_META_MAX];
};

struct udebug_remote_buf {
	struct avl_node node;
	struct udebug_buf buf;
	bool poll;
	uint32_t head;

	/* provided by user */
	uint32_t pcap_iface;
	void *priv;
	const struct udebug_packet_info *meta;
};

struct udebug {
	struct list_head local_rings;
	struct avl_tree remote_rings;
	uint32_t next_id;
	struct uloop_fd fd;
	int poll_handle;
	char *socket_path;
	struct uloop_timeout reconnect;

	/* filled by user */
	void (*notify_cb)(struct udebug *ctx, struct udebug_remote_buf *rb);
};

struct udebug_ptr {
	uint32_t start;
	uint32_t len;
	uint64_t timestamp;
};

struct udebug_snapshot {
	struct udebug_ptr *entries;
	unsigned int n_entries;
	unsigned int dropped;
	void *data;
	size_t data_size;

	uint32_t iter_idx;

	enum udebug_format format;
	uint32_t sub_format;

	uint32_t rbuf_idx;
};

struct udebug_iter {
	struct udebug_snapshot **list;
	size_t n;

	struct udebug_snapshot *s;
	unsigned int s_idx;

	uint64_t timestamp;
	void *data;
	size_t len;
};

uint64_t udebug_timestamp(void);

void udebug_entry_init_ts(struct udebug_buf *buf, uint64_t timestamp);
static inline void udebug_entry_init(struct udebug_buf *buf)
{
	udebug_entry_init_ts(buf, udebug_timestamp());
}
void *udebug_entry_append(struct udebug_buf *buf, const void *data, uint32_t len);
int udebug_entry_printf(struct udebug_buf *buf, const char *fmt, ...)
	__attribute__ ((format (printf, 2, 3)));
int udebug_entry_vprintf(struct udebug_buf *buf, const char *fmt, va_list ap)
	__attribute__ ((format (printf, 2, 0)));
uint16_t udebug_entry_trim(struct udebug_buf *buf, uint16_t len);
void udebug_entry_set_length(struct udebug_buf *buf, uint16_t len);
void udebug_entry_add(struct udebug_buf *buf);

int udebug_buf_init(struct udebug_buf *buf, size_t entries, size_t size);
int udebug_buf_add(struct udebug *ctx, struct udebug_buf *buf,
		   const struct udebug_buf_meta *meta);
uint64_t udebug_buf_flags(struct udebug_buf *buf);
void udebug_buf_free(struct udebug_buf *buf);
static inline bool udebug_buf_valid(struct udebug_buf *buf)
{
	return buf->hdr;
}

struct udebug_remote_buf *udebug_remote_buf_get(struct udebug *ctx, uint32_t id);
int udebug_remote_buf_map(struct udebug *ctx, struct udebug_remote_buf *rb, uint32_t id);
void udebug_remote_buf_unmap(struct udebug *ctx, struct udebug_remote_buf *rb);
int udebug_remote_buf_set_poll(struct udebug *ctx, struct udebug_remote_buf *rb, bool val);
void udebug_remote_buf_set_flags(struct udebug_remote_buf *rb, uint64_t mask, uint64_t set);
struct udebug_snapshot *udebug_remote_buf_snapshot(struct udebug_remote_buf *rb);
bool udebug_snapshot_get_entry(struct udebug_snapshot *s, struct udebug_iter *it, unsigned int entry);

void udebug_remote_buf_set_start_time(struct udebug_remote_buf *rb, uint64_t ts);
void udebug_remote_buf_set_start_offset(struct udebug_remote_buf *rb, uint32_t idx);

void udebug_iter_start(struct udebug_iter *it, struct udebug_snapshot **s, size_t n);
bool udebug_iter_next(struct udebug_iter *it);

void udebug_init(struct udebug *ctx);
int udebug_connect(struct udebug *ctx, const char *path);
void udebug_auto_connect(struct udebug *ctx, const char *path);
void udebug_add_uloop(struct udebug *ctx);
void udebug_poll(struct udebug *ctx);
void udebug_free(struct udebug *ctx);

static inline bool udebug_is_connected(struct udebug *ctx)
{
	return ctx->fd.fd >= 0;
}

int udebug_id_cmp(const void *k1, const void *k2, void *ptr);

#endif
