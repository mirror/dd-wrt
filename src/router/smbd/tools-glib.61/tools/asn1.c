// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * The ASB.1/BER parsing code is derived from ip_nat_snmp_basic.c which was in
 * turn derived from the gxsnmp package by Gregory McLean & Jochen Friedrich
 *
 * Copyright (c) 2000 RP Internet (www.rpi.net.au).
 */

/*****************************************************************************
 *
 * Basic ASN.1 decoding routines (gxsnmp author Dirk Wisse)
 *
 *****************************************************************************/
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <glib.h>

#include "asn1.h"

void
asn1_open(struct asn1_ctx *ctx, unsigned char *buf, unsigned int len)
{
	ctx->begin = buf;
	ctx->end = buf + len;
	ctx->pointer = buf;
	ctx->error = ASN1_ERR_NOERROR;
}

static unsigned char
asn1_octet_decode(struct asn1_ctx *ctx, unsigned char *ch)
{
	if (ctx->pointer >= ctx->end) {
		ctx->error = ASN1_ERR_DEC_EMPTY;
		return 0;
	}
	*ch = *(ctx->pointer)++;
	return 1;
}

static unsigned char
asn1_tag_decode(struct asn1_ctx *ctx, unsigned int *tag)
{
	unsigned char ch;

	*tag = 0;

	do {
		if (!asn1_octet_decode(ctx, &ch))
			return 0;
		*tag <<= 7;
		*tag |= ch & 0x7F;
	} while ((ch & 0x80) == 0x80);
	return 1;
}

static unsigned char
asn1_id_decode(struct asn1_ctx *ctx,
	       unsigned int *cls, unsigned int *con, unsigned int *tag)
{
	unsigned char ch;

	if (!asn1_octet_decode(ctx, &ch))
		return 0;

	*cls = (ch & 0xC0) >> 6;
	*con = (ch & 0x20) >> 5;
	*tag = (ch & 0x1F);

	if (*tag == 0x1F) {
		if (!asn1_tag_decode(ctx, tag))
			return 0;
	}
	return 1;
}

static unsigned char
asn1_length_decode(struct asn1_ctx *ctx, unsigned int *def, unsigned int *len)
{
	unsigned char ch, cnt;

	if (!asn1_octet_decode(ctx, &ch))
		return 0;

	if (ch == 0x80)
		*def = 0;
	else {
		*def = 1;

		if (ch < 0x80)
			*len = ch;
		else {
			cnt = (unsigned char) (ch & 0x7F);
			*len = 0;

			while (cnt > 0) {
				if (!asn1_octet_decode(ctx, &ch))
					return 0;
				*len <<= 8;
				*len |= ch;
				cnt--;
			}
		}
	}

	/* don't trust len bigger than ctx buffer */
	if (*len > ctx->end - ctx->pointer)
		return 0;

	return 1;
}

unsigned char
asn1_header_decode(struct asn1_ctx *ctx,
		   unsigned char **eoc,
		   unsigned int *cls, unsigned int *con, unsigned int *tag)
{
	unsigned int def = 0;
	unsigned int len = 0;

	if (!asn1_id_decode(ctx, cls, con, tag))
		return 0;

	if (!asn1_length_decode(ctx, &def, &len))
		return 0;

	/* primitive shall be definite, indefinite shall be constructed */
	if (*con == ASN1_PRI && !def)
		return 0;

	if (def)
		*eoc = ctx->pointer + len;
	else
		*eoc = NULL;
	return 1;
}

static unsigned char
asn1_eoc_decode(struct asn1_ctx *ctx, unsigned char *eoc)
{
	unsigned char ch;

	if (eoc == NULL) {
		if (!asn1_octet_decode(ctx, &ch))
			return 0;

		if (ch != 0x00) {
			ctx->error = ASN1_ERR_DEC_EOC_MISMATCH;
			return 0;
		}

		if (!asn1_octet_decode(ctx, &ch))
			return 0;

		if (ch != 0x00) {
			ctx->error = ASN1_ERR_DEC_EOC_MISMATCH;
			return 0;
		}
		return 1;
	}

	if (ctx->pointer != eoc) {
		ctx->error = ASN1_ERR_DEC_LENGTH_MISMATCH;
		return 0;
	}
	return 1;
}

unsigned char
asn1_octets_decode(struct asn1_ctx *ctx,
		   unsigned char *eoc,
		   unsigned char **octets, unsigned int *len)
{
	unsigned char *ptr;

	*len = 0;

	*octets = g_try_malloc(eoc - ctx->pointer);
	if (!*octets)
		return 0;

	ptr = *octets;
	while (ctx->pointer < eoc) {
		if (!asn1_octet_decode(ctx, (unsigned char *) ptr++)) {
			g_free(*octets);
			*octets = NULL;
			return 0;
		}
		(*len)++;
	}
	return 1;
}

unsigned char asn1_read(struct asn1_ctx *ctx,
		unsigned char **buf, unsigned int len)
{
	*buf = NULL;
	if (ctx->end - ctx->pointer < len) {
		ctx->error = ASN1_ERR_DEC_EMPTY;
		return 0;
	}

	*buf = g_try_malloc(len);
	if (!*buf)
		return 0;
	memcpy(*buf, ctx->pointer, len);
	ctx->pointer += len;
	return 1;
}

static unsigned char
asn1_subid_decode(struct asn1_ctx *ctx, unsigned long *subid)
{
	unsigned char ch;

	*subid = 0;

	do {
		if (!asn1_octet_decode(ctx, &ch))
			return 0;

		*subid <<= 7;
		*subid |= ch & 0x7F;
	} while ((ch & 0x80) == 0x80);
	return 1;
}

int
asn1_oid_decode(struct asn1_ctx *ctx,
		unsigned char *eoc, unsigned long **oid, unsigned int *len)
{
	unsigned long subid;
	unsigned int size;
	unsigned long *optr;

	size = eoc - ctx->pointer + 1;

	/* first subid actually encodes first two subids */
	if (size < 2 || size > UINT_MAX/sizeof(unsigned long))
		return 0;

	*oid = g_try_malloc0_n(size, sizeof(unsigned long));
	if (*oid == NULL)
		return 0;

	optr = *oid;

	if (!asn1_subid_decode(ctx, &subid)) {
		g_free(*oid);
		*oid = NULL;
		return 0;
	}

	if (subid < 40) {
		optr[0] = 0;
		optr[1] = subid;
	} else if (subid < 80) {
		optr[0] = 1;
		optr[1] = subid - 40;
	} else {
		optr[0] = 2;
		optr[1] = subid - 80;
	}

	*len = 2;
	optr += 2;

	while (ctx->pointer < eoc) {
		if (++(*len) > size) {
			ctx->error = ASN1_ERR_DEC_BADVALUE;
			g_free(*oid);
			*oid = NULL;
			return 0;
		}

		if (!asn1_subid_decode(ctx, optr++)) {
			g_free(*oid);
			*oid = NULL;
			return 0;
		}
	}
	return 1;
}

/* return the size of @depth-nested headers + payload */
int asn1_header_len(unsigned int payload_len, int depth)
{
	unsigned int len;
	int i;

	len = payload_len;
	for (i = 0; i < depth; i++) {
		/* length */
		if (len >= (1 << 24))
			len += 5;
		else if (len >= (1 << 16))
			len += 4;
		else if (len >= (1 << 8))
			len += 3;
		else if (len >= (1 << 7))
			len += 2;
		else
			len += 1;
		/* 1-byte header */
		len += 1;
	}
	return len;
}

int asn1_oid_encode(const unsigned long *in_oid, int in_len,
			unsigned char **out_oid, int *out_len)
{
	unsigned char *oid;
	unsigned long id;
	int i;

	*out_oid = g_try_malloc0_n(in_len, 5);
	if (*out_oid == NULL)
		return -ENOMEM;

	oid = *out_oid;
	*oid++ = (unsigned char)(40 * in_oid[0] + in_oid[1]);
	for (i = 2; i < in_len; i++) {
		id = in_oid[i];
		if (id >= (1 << 28))
			*oid++ = (0x80 | ((id>>28) & 0x7F));
		if (id >= (1 << 21))
			*oid++ = (0x80 | ((id>>21) & 0x7F));
		if (id >= (1 << 14))
			*oid++ = (0x80 | ((id>>14) & 0x7F));
		if (id >= (1 << 7))
			*oid++ = (0x80 | ((id>>7) & 0x7F));
		*oid++ = id & 0x7F;
	}
	*out_len = (int)(oid - *out_oid);
	return 0;
}

/*
 * @len is the sum of all sizes of header, length and payload.
 * it will be decreased by the sum of sizes of header and length.
 */
int asn1_header_encode(unsigned char **buf,
			unsigned int cls, unsigned int con, unsigned int tag,
			unsigned int *len)
{
	unsigned char *loc;
	unsigned int r_len;

	/* at least, 1-byte header + 1-byte length is needed. */
	if (*len < 2)
		return -EINVAL;

	loc = *buf;
	r_len = *len;

	*loc++ = ((cls & 0x3) << 6) | ((con & 0x1) << 5) | (tag & 0x1F);
	r_len -= 1;

	if (r_len - 1 < (1 << 7)) {
		r_len -= 1;
		*loc++ = (unsigned char)(r_len & 0x7F);
	} else if (r_len - 2 < (1 << 8)) {
		r_len -= 2;
		*loc++ = 0x81;
		*loc++ = (unsigned char)(r_len & 0xFF);
	} else if (r_len - 3 < (1 << 16)) {
		r_len -= 3;
		*loc++ = 0x82;
		*loc++ = (unsigned char)((r_len>>8) & 0xFF);
		*loc++ = (unsigned char)(r_len & 0xFF);
	} else if (r_len - 4 < (1 << 24)) {
		r_len -= 4;
		*loc++ = 0x83;
		*loc++ = (unsigned char)((r_len>>16) & 0xFF);
		*loc++ = (unsigned char)((r_len>>8) & 0xFF);
		*loc++ = (unsigned char)(r_len & 0xFF);
	} else {
		r_len -= 5;
		*loc++ = 0x84;
		*loc++ = (unsigned char)((r_len>>24) & 0xFF);
		*loc++ = (unsigned char)((r_len>>16) & 0xFF);
		*loc++ = (unsigned char)((r_len>>8) & 0xFF);
		*loc++ = (unsigned char)(r_len & 0xFF);
	}

	*buf = loc;
	*len = r_len;
	return 0;
}
