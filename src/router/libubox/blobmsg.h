/*
 * Copyright (C) 2010-2012 Felix Fietkau <nbd@openwrt.org>
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
#ifndef __BLOBMSG_H
#define __BLOBMSG_H

#include <stdarg.h>
#include "blob.h"

#define BLOBMSG_ALIGN	2
#define BLOBMSG_PADDING(len) (((len) + (1 << BLOBMSG_ALIGN) - 1) & ~((1 << BLOBMSG_ALIGN) - 1))

enum blobmsg_type {
	BLOBMSG_TYPE_UNSPEC,
	BLOBMSG_TYPE_ARRAY,
	BLOBMSG_TYPE_TABLE,
	BLOBMSG_TYPE_STRING,
	BLOBMSG_TYPE_INT64,
	BLOBMSG_TYPE_INT32,
	BLOBMSG_TYPE_INT16,
	BLOBMSG_TYPE_INT8,
	BLOBMSG_TYPE_BOOL = BLOBMSG_TYPE_INT8,
	BLOBMSG_TYPE_DOUBLE,
	__BLOBMSG_TYPE_LAST,
	BLOBMSG_TYPE_LAST = __BLOBMSG_TYPE_LAST - 1,
	BLOBMSG_CAST_INT64 = __BLOBMSG_TYPE_LAST,
};

struct blobmsg_hdr {
	uint16_t namelen;
	uint8_t name[];
} __packed;

struct blobmsg_policy {
	const char *name;
	enum blobmsg_type type;
};

static inline int blobmsg_hdrlen(unsigned int namelen)
{
	return BLOBMSG_PADDING(sizeof(struct blobmsg_hdr) + namelen + 1);
}

static inline void blobmsg_clear_name(struct blob_attr *attr)
{
	struct blobmsg_hdr *hdr = (struct blobmsg_hdr *) blob_data(attr);
	hdr->name[0] = 0;
}

static inline const char *blobmsg_name(const struct blob_attr *attr)
{
	struct blobmsg_hdr *hdr = (struct blobmsg_hdr *) blob_data(attr);
	return (const char *)(hdr + 1);
}

static inline int blobmsg_type(const struct blob_attr *attr)
{
	return blob_id(attr);
}

static uint16_t blobmsg_namelen(const struct blobmsg_hdr *hdr)
{
	return be16_to_cpu(hdr->namelen);
}

static inline void *blobmsg_data(const struct blob_attr *attr)
{
	struct blobmsg_hdr *hdr;
	char *data;

	if (!attr)
		return NULL;

	hdr = (struct blobmsg_hdr *) blob_data(attr);
	data = (char *) blob_data(attr);

	if (blob_is_extended(attr))
		data += blobmsg_hdrlen(blobmsg_namelen(hdr));

	return data;
}

static inline size_t blobmsg_data_len(const struct blob_attr *attr)
{
	uint8_t *start, *end;

	if (!attr)
		return 0;

	start = (uint8_t *) blob_data(attr);
	end = (uint8_t *) blobmsg_data(attr);

	return blob_len(attr) - (end - start);
}

static inline size_t blobmsg_len(const struct blob_attr *attr)
{
	return blobmsg_data_len(attr);
}

/*
 * blobmsg_check_attr: validate a list of attributes
 *
 * This method may be used with trusted data only. Providing
 * malformed blobs will cause out of bounds memory access.
 */
bool blobmsg_check_attr(const struct blob_attr *attr, bool name);

/*
 * blobmsg_check_attr_len: validate a list of attributes
 *
 * This method should be safer implementation of blobmsg_check_attr.
 * It will limit all memory access performed on the blob to the
 * range [attr, attr + len] (upper bound non inclusive) and is
 * thus suited for checking of untrusted blob attributes.
 */
bool blobmsg_check_attr_len(const struct blob_attr *attr, bool name, size_t len);

/*
 * blobmsg_check_attr_list: validate a list of attributes
 *
 * This method may be used with trusted data only. Providing
 * malformed blobs will cause out of bounds memory access.
 */
bool blobmsg_check_attr_list(const struct blob_attr *attr, int type);

/*
 * blobmsg_check_attr_list_len: validate a list of untrusted attributes
 *
 * This method should be safer implementation of blobmsg_check_attr_list.
 * It will limit all memory access performed on the blob to the
 * range [attr, attr + len] (upper bound non inclusive) and is
 * thus suited for checking of untrusted blob attributes.
 */
bool blobmsg_check_attr_list_len(const struct blob_attr *attr, int type, size_t len);

/*
 * blobmsg_check_array: validate array/table and return size
 *
 * Checks if all elements of an array or table are valid and have
 * the specified type. Returns the number of elements in the array
 *
 * This method may be used with trusted data only. Providing
 * malformed blobs will cause out of bounds memory access.
 */
int blobmsg_check_array(const struct blob_attr *attr, int type);

/*
 * blobmsg_check_array_len: validate untrusted array/table and return size
 *
 * Checks if all elements of an array or table are valid and have
 * the specified type. Returns the number of elements in the array.
 *
 * This method should be safer implementation of blobmsg_check_array.
 * It will limit all memory access performed on the blob to the
 * range [attr, attr + len] (upper bound non inclusive) and is
 * thus suited for checking of untrusted blob attributes.
 */
int blobmsg_check_array_len(const struct blob_attr *attr, int type, size_t len);

int blobmsg_parse(const struct blobmsg_policy *policy, int policy_len,
                  struct blob_attr **tb, void *data, unsigned int len);
int blobmsg_parse_array(const struct blobmsg_policy *policy, int policy_len,
			struct blob_attr **tb, void *data, unsigned int len);

int blobmsg_add_field(struct blob_buf *buf, int type, const char *name,
                      const void *data, unsigned int len);

static inline int
blobmsg_parse_attr(const struct blobmsg_policy *policy, int policy_len,
		   struct blob_attr **tb, struct blob_attr *data)
{
	return blobmsg_parse(policy, policy_len, tb, blobmsg_data(data), blobmsg_len(data));
}

static inline int
blobmsg_parse_array_attr(const struct blobmsg_policy *policy, int policy_len,
			 struct blob_attr **tb, struct blob_attr *data)
{
	return blobmsg_parse_array(policy, policy_len, tb, blobmsg_data(data), blobmsg_len(data));
}

static inline int
blobmsg_add_double(struct blob_buf *buf, const char *name, double val)
{
	union {
		double d;
		uint64_t u64;
	} v;
	v.d = val;
	v.u64 = cpu_to_be64(v.u64);
	return blobmsg_add_field(buf, BLOBMSG_TYPE_DOUBLE, name, &v.u64, 8);
}

static inline int
blobmsg_add_u8(struct blob_buf *buf, const char *name, uint8_t val)
{
	return blobmsg_add_field(buf, BLOBMSG_TYPE_INT8, name, &val, 1);
}

static inline int
blobmsg_add_u16(struct blob_buf *buf, const char *name, uint16_t val)
{
	val = cpu_to_be16(val);
	return blobmsg_add_field(buf, BLOBMSG_TYPE_INT16, name, &val, 2);
}

static inline int
blobmsg_add_u32(struct blob_buf *buf, const char *name, uint32_t val)
{
	val = cpu_to_be32(val);
	return blobmsg_add_field(buf, BLOBMSG_TYPE_INT32, name, &val, 4);
}

static inline int
blobmsg_add_u64(struct blob_buf *buf, const char *name, uint64_t val)
{
	val = cpu_to_be64(val);
	return blobmsg_add_field(buf, BLOBMSG_TYPE_INT64, name, &val, 8);
}

static inline int
blobmsg_add_string(struct blob_buf *buf, const char *name, const char *string)
{
	return blobmsg_add_field(buf, BLOBMSG_TYPE_STRING, name, string, strlen(string) + 1);
}

static inline int
blobmsg_add_blob(struct blob_buf *buf, struct blob_attr *attr)
{
	return blobmsg_add_field(buf, blobmsg_type(attr), blobmsg_name(attr),
				 blobmsg_data(attr), blobmsg_data_len(attr));
}

void *blobmsg_open_nested(struct blob_buf *buf, const char *name, bool array);

static inline void *
blobmsg_open_array(struct blob_buf *buf, const char *name)
{
	return blobmsg_open_nested(buf, name, true);
}

static inline void *
blobmsg_open_table(struct blob_buf *buf, const char *name)
{
	return blobmsg_open_nested(buf, name, false);
}

static inline void
blobmsg_close_array(struct blob_buf *buf, void *cookie)
{
	blob_nest_end(buf, cookie);
}

static inline void
blobmsg_close_table(struct blob_buf *buf, void *cookie)
{
	blob_nest_end(buf, cookie);
}

static inline int blobmsg_buf_init(struct blob_buf *buf)
{
	return blob_buf_init(buf, BLOBMSG_TYPE_TABLE);
}

static inline uint8_t blobmsg_get_u8(struct blob_attr *attr)
{
	return *(uint8_t *) blobmsg_data(attr);
}

static inline bool blobmsg_get_bool(struct blob_attr *attr)
{
	return *(uint8_t *) blobmsg_data(attr);
}

static inline uint16_t blobmsg_get_u16(struct blob_attr *attr)
{
	return be16_to_cpu(*(uint16_t *) blobmsg_data(attr));
}

static inline uint32_t blobmsg_get_u32(struct blob_attr *attr)
{
	return be32_to_cpu(*(uint32_t *) blobmsg_data(attr));
}

static inline uint64_t blobmsg_get_u64(struct blob_attr *attr)
{
	uint32_t *ptr = (uint32_t *) blobmsg_data(attr);
	uint64_t tmp = ((uint64_t) be32_to_cpu(ptr[0])) << 32;
	tmp |= be32_to_cpu(ptr[1]);
	return tmp;
}

static inline uint64_t blobmsg_cast_u64(struct blob_attr *attr)
{
	uint64_t tmp = 0;

	if (blobmsg_type(attr) == BLOBMSG_TYPE_INT64)
		tmp = blobmsg_get_u64(attr);
	else if (blobmsg_type(attr) == BLOBMSG_TYPE_INT32)
		tmp = blobmsg_get_u32(attr);
	else if (blobmsg_type(attr) == BLOBMSG_TYPE_INT16)
		tmp = blobmsg_get_u16(attr);
	else if (blobmsg_type(attr) == BLOBMSG_TYPE_INT8)
		tmp = blobmsg_get_u8(attr);

	return tmp;
}

static inline int64_t blobmsg_cast_s64(struct blob_attr *attr)
{
	int64_t tmp = 0;

	if (blobmsg_type(attr) == BLOBMSG_TYPE_INT64)
		tmp = blobmsg_get_u64(attr);
	else if (blobmsg_type(attr) == BLOBMSG_TYPE_INT32)
		tmp = (int32_t)blobmsg_get_u32(attr);
	else if (blobmsg_type(attr) == BLOBMSG_TYPE_INT16)
		tmp = (int16_t)blobmsg_get_u16(attr);
	else if (blobmsg_type(attr) == BLOBMSG_TYPE_INT8)
		tmp = (int8_t)blobmsg_get_u8(attr);

	return tmp;
}

static inline double blobmsg_get_double(struct blob_attr *attr)
{
	union {
		double d;
		uint64_t u64;
	} v;
	v.u64 = blobmsg_get_u64(attr);
	return v.d;
}

static inline char *blobmsg_get_string(struct blob_attr *attr)
{
	if (!attr)
		return NULL;

	return (char *) blobmsg_data(attr);
}

void *blobmsg_alloc_string_buffer(struct blob_buf *buf, const char *name, unsigned int maxlen);
void *blobmsg_realloc_string_buffer(struct blob_buf *buf, unsigned int maxlen);
void blobmsg_add_string_buffer(struct blob_buf *buf);

int blobmsg_vprintf(struct blob_buf *buf, const char *name, const char *format, va_list arg);
int blobmsg_printf(struct blob_buf *buf, const char *name, const char *format, ...)
     __attribute__((format(printf, 3, 4)));

#define blobmsg_for_each_attr(pos, attr, rem) \
	for (rem = attr ? blobmsg_data_len(attr) : 0, \
	     pos = (struct blob_attr *) (attr ? blobmsg_data(attr) : NULL); \
	     rem >= sizeof(struct blob_attr) && (blob_pad_len(pos) <= rem) && \
	     (blob_pad_len(pos) >= sizeof(struct blob_attr)); \
	     rem -= blob_pad_len(pos), pos = blob_next(pos))

#define __blobmsg_for_each_attr(pos, attr, rem) \
	for (pos = (struct blob_attr *) (attr ? blobmsg_data(attr) : NULL); \
	     rem >= sizeof(struct blob_attr) && (blob_pad_len(pos) <= rem) && \
	     (blob_pad_len(pos) >= sizeof(struct blob_attr)); \
	     rem -= blob_pad_len(pos), pos = blob_next(pos))

#endif
