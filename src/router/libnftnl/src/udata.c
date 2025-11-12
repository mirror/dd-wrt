/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2012-2016 by Pablo Neira Ayuso <pablo@netfilter.org>
 * (C) 2016 by Carlos Falgueras García <carlosfg@riseup.net>
 */

#include <libnftnl/udata.h>
#include <udata.h>
#include <utils.h>

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

EXPORT_SYMBOL(nftnl_udata_buf_alloc);
struct nftnl_udata_buf *nftnl_udata_buf_alloc(uint32_t data_size)
{
	struct nftnl_udata_buf *buf;

	buf = malloc(sizeof(struct nftnl_udata_buf) + data_size);
	if (!buf)
		return NULL;
	buf->size = data_size;
	buf->end = buf->data;

	return buf;
}

EXPORT_SYMBOL(nftnl_udata_buf_free);
void nftnl_udata_buf_free(const struct nftnl_udata_buf *buf)
{
	xfree(buf);
}

EXPORT_SYMBOL(nftnl_udata_buf_len);
uint32_t nftnl_udata_buf_len(const struct nftnl_udata_buf *buf)
{
	return (uint32_t)(buf->end - buf->data);
}

static uint32_t nftnl_udata_buf_space(const struct nftnl_udata_buf *buf)
{
	return buf->size - nftnl_udata_buf_len(buf);
}

EXPORT_SYMBOL(nftnl_udata_buf_data);
void *nftnl_udata_buf_data(const struct nftnl_udata_buf *buf)
{
	return (void *)buf->data;
}

EXPORT_SYMBOL(nftnl_udata_buf_put);
void nftnl_udata_buf_put(struct nftnl_udata_buf *buf, const void *data,
			 uint32_t len)
{
	memcpy(buf->data, data, len <= buf->size ? len : buf->size);
	buf->end = buf->data + len;
}

EXPORT_SYMBOL(nftnl_udata_start);
struct nftnl_udata *nftnl_udata_start(const struct nftnl_udata_buf *buf)
{
	return (struct nftnl_udata *)buf->data;
}

EXPORT_SYMBOL(nftnl_udata_end);
struct nftnl_udata *nftnl_udata_end(const struct nftnl_udata_buf *buf)
{
	return (struct nftnl_udata *)buf->end;
}

EXPORT_SYMBOL(nftnl_udata_put);
bool nftnl_udata_put(struct nftnl_udata_buf *buf, uint8_t type, uint32_t len,
		     const void *value)
{
	struct nftnl_udata *attr;

	if (len > UINT8_MAX ||
	    nftnl_udata_buf_space(buf) < len + sizeof(struct nftnl_udata))
		return false;

	attr = (struct nftnl_udata *)buf->end;
	attr->len  = len;
	attr->type = type;
	memcpy(attr->value, value, len);

	buf->end = (char *)nftnl_udata_next(attr);

	return true;
}

EXPORT_SYMBOL(nftnl_udata_put_strz);
bool nftnl_udata_put_strz(struct nftnl_udata_buf *buf, uint8_t type,
			  const char *strz)
{
	return nftnl_udata_put(buf, type, strlen(strz) + 1, strz);
}

EXPORT_SYMBOL(nftnl_udata_put_u32);
bool nftnl_udata_put_u32(struct nftnl_udata_buf *buf, uint8_t type,
			 uint32_t data)
{
	return nftnl_udata_put(buf, type, sizeof(data), &data);
}

EXPORT_SYMBOL(nftnl_udata_type);
uint8_t nftnl_udata_type(const struct nftnl_udata *attr)
{
	return attr->type;
}

EXPORT_SYMBOL(nftnl_udata_len);
uint8_t nftnl_udata_len(const struct nftnl_udata *attr)
{
	return attr->len;
}

EXPORT_SYMBOL(nftnl_udata_get);
void *nftnl_udata_get(const struct nftnl_udata *attr)
{
	return (void *)attr->value;
}

EXPORT_SYMBOL(nftnl_udata_get_u32);
uint32_t nftnl_udata_get_u32(const struct nftnl_udata *attr)
{
	uint32_t data;

	memcpy(&data, attr->value, sizeof(data));

	return data;
}

EXPORT_SYMBOL(nftnl_udata_next);
struct nftnl_udata *nftnl_udata_next(const struct nftnl_udata *attr)
{
	return (struct nftnl_udata *)&attr->value[attr->len];
}

EXPORT_SYMBOL(nftnl_udata_parse);
int nftnl_udata_parse(const void *data, uint32_t data_len, nftnl_udata_cb_t cb,
		      void *cb_data)
{
	int ret = 0;
	const struct nftnl_udata *attr;

	nftnl_udata_for_each_data(data, data_len, attr) {
		ret = cb(attr, cb_data);
		if (ret < 0)
			return ret;
	}

	return ret;
}

EXPORT_SYMBOL(nftnl_udata_nest_start);
struct nftnl_udata *nftnl_udata_nest_start(struct nftnl_udata_buf *buf,
                                           uint8_t type)
{
	struct nftnl_udata *ud = nftnl_udata_end(buf);

	nftnl_udata_put(buf, type, 0, NULL);

	return ud;
}

EXPORT_SYMBOL(nftnl_udata_nest_end);
void nftnl_udata_nest_end(struct nftnl_udata_buf *buf, struct nftnl_udata *ud)
{
	ud->len = buf->end - (char *)ud->value;
}
