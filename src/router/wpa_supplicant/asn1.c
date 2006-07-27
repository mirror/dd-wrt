/*
 * ASN.1 DER parsing
 * Copyright (c) 2006, Jouni Malinen <jkmaline@cc.hut.fi>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "includes.h"

#include "common.h"

#ifdef CONFIG_INTERNAL_X509

#include "asn1.h"

int asn1_get_next(const u8 *buf, size_t len, struct asn1_hdr *hdr)
{
	const u8 *pos, *end;
	u8 tmp;

	memset(hdr, 0, sizeof(*hdr));
	pos = buf;
	end = buf + len;

	hdr->identifier = *pos++;
	hdr->class = hdr->identifier >> 6;
	hdr->constructed = !!(hdr->identifier & (1 << 5));

	if ((hdr->identifier & 0x1f) == 0x1f) {
		hdr->tag = 0;
		do {
			if (pos >= end) {
				wpa_printf(MSG_DEBUG, "ASN.1: Identifier "
					   "underflow");
				return -1;
			}
			tmp = *pos++;
			wpa_printf(MSG_MSGDUMP, "ASN.1: Extended tag data: "
				   "0x%02x", tmp);
			hdr->tag = (hdr->tag << 7) | (tmp & 0x7f);
		} while (tmp & 0x80);
	} else
		hdr->tag = hdr->identifier & 0x1f;

	if (hdr->class == ASN1_CLASS_CONTEXT_SPECIFIC) {
		/* FIX: is this correct way of parsing
		 * Context-Specific? */
		wpa_printf(MSG_MSGDUMP, "ASN.1: Context-Specific %d", *pos);
		hdr->context_specific = *pos;
		pos++;
		hdr->payload = pos;
		return 0;
	}

	if (hdr->class == ASN1_CLASS_APPLICATION) {
		/* TODO: any special processing? */
	}

	tmp = *pos++;
	if (tmp & 0x80) {
		if (tmp == 0xff) {
			wpa_printf(MSG_DEBUG, "ASN.1: Reserved length "
				   "value 0xff used");
			return -1;
		}
		tmp &= 0x7f; /* number of subsequent octets */
		hdr->length = 0;
		while (tmp--) {
			if (pos >= end) {
				wpa_printf(MSG_DEBUG, "ASN.1: Length "
					   "underflow");
				return -1;
			}
			hdr->length = (hdr->length << 8) | *pos++;
		}
	} else {
		/* Short form - length 0..127 in one octet */
		hdr->length = tmp;
	}

	if (pos + hdr->length > end) {
		wpa_printf(MSG_DEBUG, "ASN.1: Contents underflow");
		return -1;
	}

	hdr->payload = pos;
	return 0;
}


int asn1_get_oid(const u8 *buf, size_t len, struct asn1_oid *oid,
		 const u8 **next)
{
	struct asn1_hdr hdr;
	const u8 *pos, *end;
	unsigned long val;
	u8 tmp;

	memset(oid, 0, sizeof(*oid));

	if (asn1_get_next(buf, len, &hdr) < 0 || hdr.length == 0)
		return -1;

	pos = hdr.payload;
	end = hdr.payload + hdr.length;
	*next = end;

	while (pos < end) {
		val = 0;

		do {
			if (pos >= end)
				return -1;
			tmp = *pos++;
			val = (val << 7) | (tmp & 0x7f);
		} while (tmp & 0x80);

		if (oid->len >= ASN1_MAX_OID_LEN) {
			wpa_printf(MSG_DEBUG, "ASN.1: Too long OID value");
			return -1;
		}
		if (oid->len == 0) {
			/*
			 * The first octet encodes the first two object
			 * identifier components in (X*40) + Y formula.
			 * X = 0..2.
			 */
			oid->oid[0] = val / 40;
			if (oid->oid[0] > 2)
				oid->oid[0] = 2;
			oid->oid[1] = val - oid->oid[0] * 40;
			oid->len = 2;
		} else
			oid->oid[oid->len++] = val;
	}

	return 0;
}

#endif /* CONFIG_INTERNAL_X509 */
