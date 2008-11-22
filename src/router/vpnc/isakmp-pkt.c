/* ISAKMP packing and unpacking routines.
   Copyright (C) 2002  Geoffrey Keating
   Copyright (C) 2003-2005 Maurice Massar

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   $Id: isakmp-pkt.c 312 2008-06-15 18:09:42Z Joerg Mayer $
*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "sysdep.h"
#include "config.h"
#include "isakmp-pkt.h"
#include "math_group.h"
#include "vpnc.h"

void *xallocc(size_t x)
{
	void *result;
	result = calloc(1, x);
	if (result == NULL)
		error(1, errno, "malloc of %lu bytes failed", (unsigned long)x);
	return result;
}

struct flow {
	size_t len;
	uint8_t *base;
	uint8_t *end;
};

static uint8_t *flow_reserve_p(struct flow *f, size_t sz)
{
	size_t l = f->end - f->base;
	if (l + sz > f->len) {
		size_t new_len = f->len == 0 ? 128 : f->len;
		while (l + sz >= new_len)
			new_len *= 2;

		if (f->base == NULL)
			f->base = malloc(new_len);
		else
			f->base = realloc(f->base, new_len);
		if (f->base == NULL)
			error(1, errno, "alloc of %lud bytes failed", (unsigned long)new_len);
		memset(f->base + f->len, 0, new_len - f->len);
		f->end = f->base + l;
		f->len = new_len;
	}
	f->end += sz;
	return f->end - sz;
}

static size_t flow_reserve(struct flow *f, size_t sz)
{
	uint8_t *p = flow_reserve_p(f, sz);
	return p - f->base;
}

static void flow_x(struct flow *f, uint8_t * data, size_t data_len)
{
	memcpy(flow_reserve_p(f, data_len), data, data_len);
}

static void flow_1(struct flow *f, uint8_t d)
{
	flow_reserve_p(f, 1)[0] = d;
}

static void flow_2(struct flow *f, uint16_t d)
{
	uint8_t dd[2];
	dd[0] = d >> 8;
	dd[1] = d;
	flow_x(f, dd, sizeof(dd));
}

static void flow_4(struct flow *f, uint32_t d)
{
	uint8_t dd[4];
	dd[0] = d >> 24;
	dd[1] = d >> 16;
	dd[2] = d >> 8;
	dd[3] = d;
	flow_x(f, dd, sizeof(dd));
}

static void init_flow(struct flow *f)
{
	memset(f, 0, sizeof(*f));
}

static void flow_attribute(struct flow *f, struct isakmp_attribute *p)
{
	for (; p; p = p->next)
		switch (p->af) {
		case isakmp_attr_lots:
			flow_2(f, p->type);
			flow_2(f, p->u.lots.length);
			flow_x(f, p->u.lots.data, p->u.lots.length);
			break;
		case isakmp_attr_16:
			flow_2(f, p->type | 0x8000);
			flow_2(f, p->u.attr_16);
			break;
		case isakmp_attr_2x8:
			flow_2(f, p->type | 0x8000);
			flow_x(f, p->u.attr_2x8, 2);
			break;
		default:
			abort();
		}
}

static void flow_payload(struct flow *f, struct isakmp_payload *p)
{
	size_t lpos;
	size_t baselen;

	if (p == NULL)
		return;

	baselen = f->end - f->base;
	if (p->next == NULL)
		flow_1(f, 0);
	else
		flow_1(f, p->next->type);
	flow_1(f, 0);
	lpos = flow_reserve(f, 2);
	switch (p->type) {
	case ISAKMP_PAYLOAD_SA:
		flow_4(f, p->u.sa.doi);
		flow_4(f, p->u.sa.situation);
		flow_payload(f, p->u.sa.proposals);
		break;
	case ISAKMP_PAYLOAD_P:
		flow_1(f, p->u.p.number);
		flow_1(f, p->u.p.prot_id);
		flow_1(f, p->u.p.spi_size);
		{
			uint8_t num_xform = 0;
			struct isakmp_payload *xform;
			for (xform = p->u.p.transforms; xform; xform = xform->next)
				num_xform++;
			flow_1(f, num_xform);
		}
		flow_x(f, p->u.p.spi, p->u.p.spi_size);
		flow_payload(f, p->u.p.transforms);
		break;
	case ISAKMP_PAYLOAD_T:
		flow_1(f, p->u.t.number);
		flow_1(f, p->u.t.id);
		flow_2(f, 0);
		flow_attribute(f, p->u.t.attributes);
		break;
	case ISAKMP_PAYLOAD_KE:
	case ISAKMP_PAYLOAD_HASH:
	case ISAKMP_PAYLOAD_SIG:
	case ISAKMP_PAYLOAD_NONCE:
	case ISAKMP_PAYLOAD_VID:
	case ISAKMP_PAYLOAD_NAT_D:
	case ISAKMP_PAYLOAD_NAT_D_OLD:
		flow_x(f, p->u.ke.data, p->u.ke.length);
		break;
	case ISAKMP_PAYLOAD_ID:
		flow_1(f, p->u.id.type);
		flow_1(f, p->u.id.protocol);
		flow_2(f, p->u.id.port);
		flow_x(f, p->u.id.data, p->u.id.length);
		break;
	case ISAKMP_PAYLOAD_CERT:
	case ISAKMP_PAYLOAD_CR:
		flow_1(f, p->u.cert.encoding);
		flow_x(f, p->u.cert.data, p->u.cert.length);
		break;
	case ISAKMP_PAYLOAD_N:
		flow_4(f, p->u.n.doi);
		flow_1(f, p->u.n.protocol);
		flow_1(f, p->u.n.spi_length);
		flow_2(f, p->u.n.type);
		flow_x(f, p->u.n.spi, p->u.n.spi_length);
		flow_x(f, p->u.n.data, p->u.n.data_length);
		break;
	case ISAKMP_PAYLOAD_D:
		flow_4(f, p->u.d.doi);
		flow_1(f, p->u.d.protocol);
		flow_1(f, p->u.d.spi_length);
		flow_2(f, p->u.d.num_spi);
		if (p->u.d.spi_length > 0) {
			int i;
			for (i = 0; i < p->u.d.num_spi; i++)
				flow_x(f, p->u.d.spi[i], p->u.d.spi_length);
		}
		break;
	case ISAKMP_PAYLOAD_MODECFG_ATTR:
		flow_1(f, p->u.modecfg.type);
		flow_1(f, 0);
		flow_2(f, p->u.modecfg.id);
		flow_attribute(f, p->u.modecfg.attributes);
		break;
	default:
		abort();
	}
	f->base[lpos] = (f->end - f->base - baselen) >> 8;
	f->base[lpos + 1] = (f->end - f->base - baselen);
	flow_payload(f, p->next);
}

void flatten_isakmp_payloads(struct isakmp_payload *p, uint8_t ** result, size_t * size)
{
	struct flow f;
	init_flow(&f);
	flow_payload(&f, p);
	*result = f.base;
	*size = f.end - f.base;
}

void flatten_isakmp_payload(struct isakmp_payload *p, uint8_t ** result, size_t * size)
{
	struct isakmp_payload *next;
	next = p->next;
	p->next = NULL;
	flatten_isakmp_payloads(p, result, size);
	p->next = next;
}

void flatten_isakmp_packet(struct isakmp_packet *p, uint8_t ** result, size_t * size, size_t blksz)
{
	struct flow f;
	size_t lpos, sz, padding;

	init_flow(&f);
	flow_x(&f, p->i_cookie, ISAKMP_COOKIE_LENGTH);
	flow_x(&f, p->r_cookie, ISAKMP_COOKIE_LENGTH);
	if (p->payload == NULL)
		flow_1(&f, 0);
	else
		flow_1(&f, p->payload->type);
	flow_1(&f, p->isakmp_version);
	flow_1(&f, p->exchange_type);
	flow_1(&f, p->flags);
	flow_4(&f, p->message_id);
	lpos = flow_reserve(&f, 4);
	flow_payload(&f, p->payload);
	if (p->flags & ISAKMP_FLAG_E) {
		assert(blksz != 0);
		sz = (f.end - f.base) - ISAKMP_PAYLOAD_O;
		padding = blksz - (sz % blksz);
		if (padding == blksz)
			padding = 0;
		DEBUG(3, printf("size = %ld, blksz = %ld, padding = %ld\n",
				(long)sz, (long)blksz, (long)padding));
		flow_reserve(&f, padding);
	}
	f.base[lpos] = (f.end - f.base) >> 24;
	f.base[lpos + 1] = (f.end - f.base) >> 16;
	f.base[lpos + 2] = (f.end - f.base) >> 8;
	f.base[lpos + 3] = (f.end - f.base);
	*result = f.base;
	*size = f.end - f.base;
	 /*DUMP*/ if (opt_debug >= 3) {
		printf("\n sending: ========================>\n");
		free_isakmp_packet(parse_isakmp_packet(f.base, f.end - f.base, NULL));
	}
}

struct isakmp_attribute *new_isakmp_attribute(uint16_t type, struct isakmp_attribute *next)
{
	struct isakmp_attribute *r = xallocc(sizeof(struct isakmp_attribute));
	r->type = type;
	r->next = next;
	return r;
}

struct isakmp_attribute *new_isakmp_attribute_16(uint16_t type, uint16_t data,
	struct isakmp_attribute *next)
{
	struct isakmp_attribute *r = xallocc(sizeof(struct isakmp_attribute));
	r->next = next;
	r->type = type;
	r->af = isakmp_attr_16;
	r->u.attr_16 = data;
	return r;
}

struct isakmp_packet *new_isakmp_packet(void)
{
	return xallocc(sizeof(struct isakmp_packet));
}

struct isakmp_payload *new_isakmp_payload(uint8_t type)
{
	struct isakmp_payload *result = xallocc(sizeof(struct isakmp_payload));
	result->type = type;
	return result;
}

struct isakmp_payload *new_isakmp_data_payload(uint8_t type, const void *data, size_t data_length)
{
	struct isakmp_payload *result = xallocc(sizeof(struct isakmp_payload));

	if (type != ISAKMP_PAYLOAD_KE && type != ISAKMP_PAYLOAD_HASH
		&& type != ISAKMP_PAYLOAD_SIG && type != ISAKMP_PAYLOAD_NONCE
		&& type != ISAKMP_PAYLOAD_VID && type != ISAKMP_PAYLOAD_NAT_D
		&& type != ISAKMP_PAYLOAD_NAT_D_OLD)
		abort();
	if (data_length >= 16384)
		abort();

	result->type = type;
	result->u.ke.length = data_length;
	result->u.ke.data = xallocc(data_length);
	memcpy(result->u.ke.data, data, data_length);
	return result;
}

static void free_isakmp_payload(struct isakmp_payload *p)
{
	struct isakmp_payload *nxt;

	if (p == NULL)
		return;

	switch (p->type) {
	case ISAKMP_PAYLOAD_SA:
		free_isakmp_payload(p->u.sa.proposals);
		break;
	case ISAKMP_PAYLOAD_P:
		free(p->u.p.spi);
		free_isakmp_payload(p->u.p.transforms);
		break;
	case ISAKMP_PAYLOAD_T:
		{
			struct isakmp_attribute *att, *natt;
			for (att = p->u.t.attributes; att; att = natt) {
				natt = att->next;
				if (att->af == isakmp_attr_lots)
					free(att->u.lots.data);
				free(att);
			}
		}
		break;
	case ISAKMP_PAYLOAD_KE:
	case ISAKMP_PAYLOAD_HASH:
	case ISAKMP_PAYLOAD_SIG:
	case ISAKMP_PAYLOAD_NONCE:
	case ISAKMP_PAYLOAD_VID:
	case ISAKMP_PAYLOAD_NAT_D:
	case ISAKMP_PAYLOAD_NAT_D_OLD:
		free(p->u.ke.data);
		break;
	case ISAKMP_PAYLOAD_ID:
		free(p->u.id.data);
		break;
	case ISAKMP_PAYLOAD_CERT:
	case ISAKMP_PAYLOAD_CR:
		free(p->u.cert.data);
		break;
	case ISAKMP_PAYLOAD_N:
		free(p->u.n.spi);
		free(p->u.n.data);
		break;
	case ISAKMP_PAYLOAD_D:
		if (p->u.d.spi) {
			int i;
			for (i = 0; i < p->u.d.num_spi; i++)
				free(p->u.d.spi[i]);
			free(p->u.d.spi);
		}
		break;
	case ISAKMP_PAYLOAD_MODECFG_ATTR:
		{
			struct isakmp_attribute *att, *natt;
			for (att = p->u.modecfg.attributes; att; att = natt) {
				natt = att->next;
				if (att->af == isakmp_attr_lots)
					free(att->u.lots.data);
				if (att->af == isakmp_attr_acl)
					free(att->u.acl.acl_ent);
				free(att);
			}
		}
		break;
	default:
		abort();

	}
	nxt = p->next;
	free(p);
	free_isakmp_payload(nxt);
}

void free_isakmp_packet(struct isakmp_packet *p)
{
	if (p == NULL)
		return;
	free_isakmp_payload(p->payload);
	free(p);
}

static const struct debug_strings *transform_id_to_debug_strings(enum isakmp_ipsec_proto_enum decode_proto)
{
	switch (decode_proto) {
	case ISAKMP_IPSEC_PROTO_ISAKMP:
		return isakmp_ipsec_key_enum_array;
	case ISAKMP_IPSEC_PROTO_IPSEC_AH:
		return isakmp_ipsec_ah_enum_array;
	case ISAKMP_IPSEC_PROTO_IPSEC_ESP:
		return isakmp_ipsec_esp_enum_array;
	case ISAKMP_IPSEC_PROTO_IPCOMP:
		return isakmp_ipsec_ipcomp_enum_array;
	default:
		return NULL;
	}
}

static const struct debug_strings *attr_type_to_debug_strings(enum isakmp_ipsec_proto_enum decode_proto)
{
	switch (decode_proto) {
	case ISAKMP_IPSEC_PROTO_ISAKMP:
		return ike_attr_enum_array;
	case ISAKMP_IPSEC_PROTO_IPSEC_AH:
	case ISAKMP_IPSEC_PROTO_IPSEC_ESP:
		return isakmp_ipsec_attr_enum_array;
	case ISAKMP_IPSEC_PROTO_MODECFG:
		return isakmp_modecfg_attrib_enum_array;
	default:
		return NULL;
	}
}

static const struct debug_strings *attr_val_to_debug_strings(enum isakmp_ipsec_proto_enum decode_proto, uint16_t type)
{
	switch (decode_proto) {
	case ISAKMP_IPSEC_PROTO_ISAKMP:
		switch (type) {
		case IKE_ATTRIB_ENC:         return ike_enc_enum_array;
		case IKE_ATTRIB_HASH:        return ike_hash_enum_array;
		case IKE_ATTRIB_AUTH_METHOD: return ike_auth_enum_array;
		case IKE_ATTRIB_GROUP_DESC:  return ike_group_enum_array;
		case IKE_ATTRIB_GROUP_TYPE:  return ike_group_type_enum_array;
		case IKE_ATTRIB_LIFE_TYPE:   return ike_life_enum_array;
		default:  return NULL;
		}
	case ISAKMP_IPSEC_PROTO_IPSEC_AH:
	case ISAKMP_IPSEC_PROTO_IPSEC_ESP:
		switch (type) {
		case ISAKMP_IPSEC_ATTRIB_SA_LIFE_TYPE: return ipsec_life_enum_array;
		case ISAKMP_IPSEC_ATTRIB_ENCAP_MODE:   return ipsec_encap_enum_array;
		case ISAKMP_IPSEC_ATTRIB_AUTH_ALG:     return ipsec_auth_enum_array;
		default:  return NULL;
		}
	default:
		return NULL;
	}
}

#define fetch4()  					\
  (data += 4, data_len -= 4,				\
   (uint32_t)(data[-4]) << 24 | (uint32_t)(data[-3]) << 16	\
   | (uint32_t)(data[-2]) << 8 | data[-1])
#define fetch2()				\
  (data += 2, data_len -= 2,			\
   (uint16_t)(data[-2]) << 8 | data[-1])
#define fetch1() (data_len--, *data++)
#define fetchn(d,n)  \
  (memcpy ((d), data, (n)), data += (n), data_len -= (n))

static struct isakmp_attribute *parse_isakmp_attributes(const uint8_t ** data_p,
	size_t data_len, int * reject, enum isakmp_ipsec_proto_enum decode_proto)
{
	const uint8_t *data = *data_p;
	struct isakmp_attribute *r;
	uint16_t type, length;
	int i;

	if (data_len < 4)
		return NULL;

	r = new_isakmp_attribute(0, NULL);
	type = fetch2();
	length = fetch2();
	if (type & 0x8000) {
		r->type = type & ~0x8000;
		hex_dump("t.attributes.type", &r->type, DUMP_UINT16, attr_type_to_debug_strings(decode_proto));
		r->af = isakmp_attr_16;
		r->u.attr_16 = length;
		if ((ISAKMP_XAUTH_06_ATTRIB_TYPE <= r->type)
			&& (r->type <= ISAKMP_XAUTH_06_ATTRIB_ANSWER)
			&& (r->type != ISAKMP_XAUTH_06_ATTRIB_STATUS)
			&& (length > 0)
			&& (opt_debug < 99))
			DEBUG(3, printf("(not dumping xauth data)\n"));
		else
			hex_dump("t.attributes.u.attr_16", &r->u.attr_16, DUMP_UINT16,
				attr_val_to_debug_strings(decode_proto, r->type));
	} else {
		r->type = type;
		hex_dump("t.attributes.type", &r->type, DUMP_UINT16, attr_type_to_debug_strings(decode_proto));
		r->af = isakmp_attr_lots;
		r->u.lots.length = length;
		if ((ISAKMP_XAUTH_06_ATTRIB_TYPE <= r->type)
			&& (r->type <= ISAKMP_XAUTH_06_ATTRIB_ANSWER)
			&& (r->type != ISAKMP_XAUTH_06_ATTRIB_STATUS)
			&& (length > 0)
			&& (opt_debug < 99))
			DEBUG(3, printf("(not dumping xauth data length)\n"));
		else
			hex_dump("t.attributes.u.lots.length", &r->u.lots.length, DUMP_UINT16, NULL);
		if (data_len < length) {
			*reject = ISAKMP_N_PAYLOAD_MALFORMED;
			return r;
		}
		if (r->type == ISAKMP_MODECFG_ATTRIB_CISCO_SPLIT_INC) {
			r->af = isakmp_attr_acl;
			r->u.acl.count = length / (4+4+2+2+2);
			if (r->u.acl.count * (4+4+2+2+2) != length) {
				*reject = ISAKMP_N_PAYLOAD_MALFORMED;
				return r;
			}
			r->u.acl.acl_ent = xallocc(r->u.acl.count * sizeof(struct acl_ent_s));
			
			for (i = 0; i < r->u.acl.count; i++) {
				fetchn(&r->u.acl.acl_ent[i].addr.s_addr, 4);
				fetchn(&r->u.acl.acl_ent[i].mask.s_addr, 4);
				r->u.acl.acl_ent[i].protocol = fetch2();
				r->u.acl.acl_ent[i].sport = fetch2();
				r->u.acl.acl_ent[i].dport = fetch2();
				hex_dump("t.attributes.u.acl.addr", &r->u.acl.acl_ent[i].addr.s_addr, 4, NULL);
				hex_dump("t.attributes.u.acl.mask", &r->u.acl.acl_ent[i].mask.s_addr, 4, NULL);
				hex_dump("t.attributes.u.acl.protocol", &r->u.acl.acl_ent[i].protocol, DUMP_UINT16, NULL);
				hex_dump("t.attributes.u.acl.sport", &r->u.acl.acl_ent[i].sport, DUMP_UINT16, NULL);
				hex_dump("t.attributes.u.acl.dport", &r->u.acl.acl_ent[i].dport, DUMP_UINT16, NULL);
			}
		} else {
			r->u.lots.data = xallocc(length);
			fetchn(r->u.lots.data, length);
			if ((ISAKMP_XAUTH_06_ATTRIB_TYPE <= type)
				&& (type <= ISAKMP_XAUTH_06_ATTRIB_ANSWER)
				&& (r->type != ISAKMP_XAUTH_06_ATTRIB_STATUS)
				&& (length > 0)
				&& (opt_debug < 99))
				DEBUG(3, printf("(not dumping xauth data)\n"));
			else
				hex_dump("t.attributes.u.lots.data", r->u.lots.data, r->u.lots.length, NULL);
		}
	}
	r->next = parse_isakmp_attributes(&data, data_len, reject, decode_proto);
	*data_p = data;
	return r;
}

static struct isakmp_payload *parse_isakmp_payload(uint8_t type,
	const uint8_t ** data_p, size_t * data_len_p, int * reject, enum isakmp_ipsec_proto_enum decode_proto)
{
	const uint8_t *data = *data_p, *tmpdata;
	size_t data_len = *data_len_p;
	struct isakmp_payload *r;
	uint8_t next_type;
	size_t length, olength;

	static const uint16_t min_payload_len[ISAKMP_PAYLOAD_MODECFG_ATTR + 1] = {
		4, 12, 8, 8, 4, 8, 5, 5, 4, 4, 4, 12, 12, 4, 8
	};

	DEBUG(3, printf("\n"));
	hex_dump("PARSING PAYLOAD type", &type, DUMP_UINT8, isakmp_payload_enum_array);
	if (type == 0)
		return NULL;
	if (type <= ISAKMP_PAYLOAD_MODECFG_ATTR) {
		if (data_len < min_payload_len[type]) {
			*reject = ISAKMP_N_PAYLOAD_MALFORMED;
			return NULL;
		}
	} else if (data_len < 4) {
		*reject = ISAKMP_N_PAYLOAD_MALFORMED;
		return NULL;
	}

	r = new_isakmp_payload(type);
	next_type = fetch1();
	hex_dump("next_type", &next_type, DUMP_UINT8, isakmp_payload_enum_array);
	if (fetch1() != 0) {
		*reject = ISAKMP_N_PAYLOAD_MALFORMED;
		return r;
	}
	length = fetch2();
	hex_dump("length", &length, DUMP_UINT16, NULL);
	if (length > data_len + 4
		|| ((type <= ISAKMP_PAYLOAD_MODECFG_ATTR)&&(length < min_payload_len[type]))
		|| (length < 4)) {
		*reject = ISAKMP_N_PAYLOAD_MALFORMED;
		return r;
	}
	olength = length;
	switch (type) {
	case ISAKMP_PAYLOAD_SA:
		r->u.sa.doi = fetch4();
		hex_dump("sa.doi", &r->u.sa.doi, DUMP_UINT32, isakmp_doi_enum_array);
		if (r->u.sa.doi != ISAKMP_DOI_IPSEC) {
			*reject = ISAKMP_N_DOI_NOT_SUPPORTED;
			return r;
		}
		r->u.sa.situation = fetch4();
		hex_dump("sa.situation", &r->u.sa.situation, DUMP_UINT32, isakmp_ipsec_sit_enum_array);
		if (r->u.sa.situation != ISAKMP_IPSEC_SIT_IDENTITY_ONLY) {
			*reject = ISAKMP_N_SITUATION_NOT_SUPPORTED;
			return r;
		}
		*reject = 0;
		length -= 12;
		r->u.sa.proposals = parse_isakmp_payload(ISAKMP_PAYLOAD_P, &data, &length, reject, decode_proto);
		if (*reject != 0)
			return r;
		/* Allow trailing garbage at end of payload.  */
		data_len -= olength - 12;
		break;

	case ISAKMP_PAYLOAD_P:
		if (next_type != ISAKMP_PAYLOAD_P && next_type != 0) {
			*reject = ISAKMP_N_INVALID_PAYLOAD_TYPE;
			return r;
		}
		{
			uint8_t num_xform;
			struct isakmp_payload *xform;

			r->u.p.number = fetch1();
			hex_dump("p.number", &r->u.p.number, DUMP_UINT8, NULL);
			r->u.p.prot_id = fetch1();
			hex_dump("p.prot_id", &r->u.p.prot_id, DUMP_UINT8, isakmp_ipsec_proto_enum_array);
			r->u.p.spi_size = fetch1();
			hex_dump("p.spi_size", &r->u.p.spi_size, DUMP_UINT8, NULL);
			num_xform = fetch1();
			hex_dump("length", &num_xform, DUMP_UINT8, NULL);

			if (data_len < r->u.p.spi_size) {
				*reject = ISAKMP_N_PAYLOAD_MALFORMED;
				return r;
			}
			r->u.p.spi = xallocc(r->u.p.spi_size);
			fetchn(r->u.p.spi, r->u.p.spi_size);
			hex_dump("p.spi", r->u.p.spi, r->u.p.spi_size, NULL);
			length -= 8 + r->u.p.spi_size;
			r->u.p.transforms = parse_isakmp_payload(ISAKMP_PAYLOAD_T,
				&data, &length, reject, r->u.p.prot_id);
			for (xform = r->u.p.transforms; xform; xform = xform->next)
				if (num_xform-- == 0)
					break;
			if (num_xform != 0) {
				*reject = ISAKMP_N_BAD_PROPOSAL_SYNTAX;
				return r;
			}

			/* Allow trailing garbage at end of payload.  */
			data_len -= olength - 8 - r->u.p.spi_size;
		}
		break;

	case ISAKMP_PAYLOAD_T:
		if (next_type != ISAKMP_PAYLOAD_T && next_type != 0) {
			*reject = ISAKMP_N_INVALID_PAYLOAD_TYPE;
			return r;
		}
		r->u.t.number = fetch1();
		hex_dump("t.number", &r->u.t.number, DUMP_UINT8, NULL);
		r->u.t.id = fetch1();
		hex_dump("t.id", &r->u.t.id, DUMP_UINT8, transform_id_to_debug_strings(decode_proto));
		if (fetch2() != 0) {
			*reject = ISAKMP_N_BAD_PROPOSAL_SYNTAX;
			return r;
		}
		length -= 8;
		r->u.t.attributes = parse_isakmp_attributes(&data, length, reject, decode_proto);
		data_len -= olength - 8;
		break;

	case ISAKMP_PAYLOAD_KE:
	case ISAKMP_PAYLOAD_HASH:
	case ISAKMP_PAYLOAD_SIG:
	case ISAKMP_PAYLOAD_NONCE:
	case ISAKMP_PAYLOAD_VID:
	case ISAKMP_PAYLOAD_NAT_D:
	case ISAKMP_PAYLOAD_NAT_D_OLD:
		r->u.ke.length = length - 4;
		r->u.ke.data = xallocc(r->u.ke.length);
		fetchn(r->u.ke.data, r->u.ke.length);
		hex_dump("ke.data", r->u.ke.data, r->u.ke.length, NULL);
		if (type == ISAKMP_PAYLOAD_VID)
			print_vid(r->u.ke.data, r->u.ke.length);
		break;
	case ISAKMP_PAYLOAD_ID:
		r->u.id.type = fetch1();
		hex_dump("id.type", &r->u.id.type, DUMP_UINT8, isakmp_ipsec_id_enum_array);
		r->u.id.protocol = fetch1();
		hex_dump("id.protocol", &r->u.id.protocol, DUMP_UINT8, NULL); /* IP protocol nr */
		r->u.id.port = fetch2();
		hex_dump("id.port", &r->u.id.port, DUMP_UINT16, NULL);
		r->u.id.length = length - 8;
		r->u.id.data = xallocc(r->u.id.length);
		fetchn(r->u.id.data, r->u.id.length);
		hex_dump("id.data", r->u.id.data, r->u.id.length, NULL);
		break;
	case ISAKMP_PAYLOAD_CERT:
	case ISAKMP_PAYLOAD_CR:
		r->u.cert.encoding = fetch1();
		hex_dump("cert.encoding", &r->u.cert.encoding, DUMP_UINT8, NULL);
		r->u.cert.length = length - 5;
		r->u.cert.data = xallocc(r->u.cert.length);
		fetchn(r->u.cert.data, r->u.cert.length);
		hex_dump("cert.data", r->u.cert.data, r->u.cert.length, NULL);
		break;
	case ISAKMP_PAYLOAD_N:
		r->u.n.doi = fetch4();
		hex_dump("n.doi", &r->u.n.doi, DUMP_UINT32, isakmp_doi_enum_array);
		r->u.n.protocol = fetch1();
		hex_dump("n.protocol", &r->u.n.protocol, DUMP_UINT8, isakmp_ipsec_proto_enum_array);
		r->u.n.spi_length = fetch1();
		hex_dump("n.spi_length", &r->u.n.spi_length, DUMP_UINT8, NULL);
		r->u.n.type = fetch2();
		hex_dump("n.type", &r->u.n.type, DUMP_UINT16, isakmp_notify_enum_array);
		if (r->u.n.spi_length + 12u > length) {
			*reject = ISAKMP_N_PAYLOAD_MALFORMED;
			return r;
		}
		r->u.n.spi = xallocc(r->u.n.spi_length);
		fetchn(r->u.n.spi, r->u.n.spi_length);
		hex_dump("n.spi", r->u.n.spi, r->u.n.spi_length, NULL);
		r->u.n.data_length = length - 12 - r->u.n.spi_length;
		r->u.n.data = xallocc(r->u.n.data_length);
		fetchn(r->u.n.data, r->u.n.data_length);
		hex_dump("n.data", r->u.n.data, r->u.n.data_length, NULL);
		if ((r->u.n.doi == ISAKMP_DOI_IPSEC)&&(r->u.n.type == ISAKMP_N_IPSEC_RESPONDER_LIFETIME)) {
			tmpdata = r->u.n.data;
			r->u.n.attributes = parse_isakmp_attributes(&tmpdata, r->u.n.data_length, reject,
				r->u.n.protocol);
		}
		break;
	case ISAKMP_PAYLOAD_D:
		r->u.d.doi = fetch4();
		hex_dump("d.doi", &r->u.d.doi, DUMP_UINT32, isakmp_doi_enum_array);
		r->u.d.protocol = fetch1();
		hex_dump("d.protocol", &r->u.d.protocol, DUMP_UINT8, isakmp_ipsec_proto_enum_array);
		r->u.d.spi_length = fetch1();
		hex_dump("d.spi_length", &r->u.d.spi_length, DUMP_UINT8, NULL);
		r->u.d.num_spi = fetch2();
		hex_dump("d.num_spi", &r->u.d.num_spi, DUMP_UINT16, NULL);
		if (r->u.d.num_spi * r->u.d.spi_length + 12u != length) {
			*reject = ISAKMP_N_PAYLOAD_MALFORMED;
			return r;
		}
		r->u.d.spi = xallocc(sizeof(uint8_t *) * r->u.d.num_spi);
		{
			int i;
			for (i = 0; i < r->u.d.num_spi; i++) {
				r->u.d.spi[i] = xallocc(r->u.d.spi_length);
				fetchn(r->u.d.spi[i], r->u.d.spi_length);
				hex_dump("d.spi", r->u.d.spi[i], r->u.d.spi_length, NULL);
			}
		}
		break;
	case ISAKMP_PAYLOAD_MODECFG_ATTR:
		r->u.modecfg.type = fetch1();
		hex_dump("modecfg.type", &r->u.modecfg.type, DUMP_UINT8, isakmp_modecfg_cfg_enum_array);
		if (fetch1() != 0) {
			*reject = ISAKMP_N_PAYLOAD_MALFORMED;
			return r;
		}
		r->u.modecfg.id = fetch2();
		hex_dump("modecfg.id", &r->u.modecfg.id, DUMP_UINT16, NULL);
		length -= 8;
		r->u.modecfg.attributes = parse_isakmp_attributes(&data, length, reject,
			ISAKMP_IPSEC_PROTO_MODECFG); /* this "proto" is a hack for simplicity */
		data_len -= olength - 8;
		break;

	default:
		r->u.ke.length = length - 4;
		r->u.ke.data = xallocc(r->u.ke.length);
		fetchn(r->u.ke.data, r->u.ke.length);
		hex_dump("UNKNOWN.data", r->u.ke.data, r->u.ke.length, NULL);
		break;
	}
	*data_p = data;
	*data_len_p = data_len;
	hex_dump("DONE PARSING PAYLOAD type", &type, DUMP_UINT8, isakmp_payload_enum_array);
	r->next = parse_isakmp_payload(next_type, data_p, data_len_p, reject, decode_proto);
	return r;
}

struct isakmp_packet *parse_isakmp_packet(const uint8_t * data, size_t data_len, int * reject)
{
	int reason = 0;
	uint8_t payload;
	struct isakmp_packet *r = new_isakmp_packet();
	size_t o_data_len = data_len;
	size_t isakmp_data_len;

	if (data_len < ISAKMP_PAYLOAD_O) {
		DEBUG(2, printf("packet to short: len = %lld < min = %lld\n", (long long) data_len, (long long)ISAKMP_PAYLOAD_O));
		reason = ISAKMP_N_UNEQUAL_PAYLOAD_LENGTHS;
		goto error;
	}

	DEBUG(3, printf("BEGIN_PARSE\n"));
	DEBUG(3, printf("Recieved Packet Len: %d\n", data_len));
	fetchn(r->i_cookie, ISAKMP_COOKIE_LENGTH);
	hex_dump("i_cookie", r->i_cookie, ISAKMP_COOKIE_LENGTH, NULL);
	fetchn(r->r_cookie, ISAKMP_COOKIE_LENGTH);
	hex_dump("r_cookie", r->r_cookie, ISAKMP_COOKIE_LENGTH, NULL);
	payload = fetch1();
	hex_dump("payload", &payload, DUMP_UINT8, isakmp_payload_enum_array);

	r->isakmp_version = fetch1();
	hex_dump("isakmp_version", &r->isakmp_version, DUMP_UINT8, NULL);
	if (r->isakmp_version > ISAKMP_VERSION) {
		if ((r->isakmp_version & 0xF0) >= (ISAKMP_VERSION & 0xF0))
			reason = ISAKMP_N_INVALID_MAJOR_VERSION;
		else
			reason = ISAKMP_N_INVALID_MINOR_VERSION;
		goto error;
	}

	r->exchange_type = fetch1();
	hex_dump("exchange_type", &r->exchange_type, DUMP_UINT8, isakmp_exchange_enum_array);
	r->flags = fetch1();
	hex_dump("flags", &r->flags, DUMP_UINT8, NULL);
	r->message_id = fetch4();
	hex_dump("message_id", &r->message_id, sizeof(r->message_id), NULL);

	isakmp_data_len = fetch4();
	hex_dump("len", &isakmp_data_len, DUMP_UINT32, NULL);
	if (o_data_len != isakmp_data_len) {
		DEBUG(2, printf("isakmp length does not match packet length: isakmp = %lld != datalen = %lld\n",
			(long long)isakmp_data_len, (long long)o_data_len));
		reason = ISAKMP_N_UNEQUAL_PAYLOAD_LENGTHS;
		goto error;
	}

	r->payload = parse_isakmp_payload(payload, &data, &data_len, &reason, 0);
	if (reason != 0)
		goto error;

	DEBUG(3, printf("PARSE_OK\n"));
	return r;

      error:
	free_isakmp_packet(r);
	if (reject)
		*reject = reason;
	return NULL;
}

void test_pack_unpack(void)
{
	static const uint8_t pack[] = {
		0x7f, 0xba, 0x51, 0x29, 0x11, 0x9e, 0x76, 0xf7, 0x9a, 0x71, 0xee, 0x70,
		0xaa, 0x82, 0xb9, 0x7f, 0x01, 0x10, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x01, 0x4c, 0x04, 0x00, 0x00, 0x2c, 0x00, 0x00, 0x00, 0x01,
		0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x20, 0x01, 0x01, 0x00, 0x01,
		0x00, 0x00, 0x00, 0x18, 0x00, 0x01, 0x00, 0x00, 0x80, 0x01, 0x00, 0x05,
		0x80, 0x02, 0x00, 0x01, 0x80, 0x04, 0x00, 0x02, 0x80, 0x03, 0x00, 0x01,
		0x0a, 0x00, 0x00, 0x84, 0x1b, 0x1d, 0x4b, 0x29, 0x0e, 0x29, 0xb9, 0x6f,
		0x18, 0x34, 0xd1, 0x2d, 0xba, 0x92, 0x7c, 0x53, 0x35, 0x76, 0x0e, 0x3b,
		0x25, 0x92, 0x4f, 0x7c, 0x1e, 0x31, 0x41, 0x8c, 0xb9, 0xe3, 0xda, 0xf7,
		0x53, 0xd3, 0x22, 0x8e, 0xff, 0xeb, 0xed, 0x5b, 0x95, 0x56, 0x8d, 0xba,
		0xa8, 0xe3, 0x2a, 0x9b, 0xb4, 0x04, 0x5c, 0x90, 0xf0, 0xfe, 0x92, 0xc8,
		0x57, 0xa2, 0xc6, 0x0c, 0x85, 0xbb, 0x56, 0xe8, 0x1c, 0xa7, 0x2c, 0x57,
		0x04, 0xb6, 0xe0, 0x43, 0x82, 0xe1, 0x9f, 0x0b, 0xa6, 0x8b, 0xce, 0x7f,
		0x9b, 0x75, 0xbb, 0xd3, 0xff, 0x0e, 0x89, 0x19, 0xaf, 0xc6, 0x2e, 0xf6,
		0x92, 0x06, 0x46, 0x4f, 0xc7, 0x97, 0x22, 0xf4, 0xa6, 0xf9, 0x26, 0x34,
		0x04, 0x33, 0x89, 0x34, 0xa9, 0x2f, 0x81, 0x92, 0xd3, 0x21, 0x4f, 0x45,
		0xbe, 0x38, 0x12, 0x26, 0xec, 0x87, 0x45, 0xdd, 0x10, 0x1c, 0xd6, 0x16,
		0x05, 0x00, 0x00, 0x18, 0x77, 0xdf, 0x37, 0x3c, 0x03, 0x02, 0xe2, 0xc8,
		0xe1, 0x2f, 0x92, 0xf0, 0x2e, 0xa2, 0xa6, 0x00, 0x17, 0x8f, 0xdf, 0xb4,
		0x08, 0x00, 0x00, 0x0c, 0x01, 0x11, 0x01, 0xf4, 0xcd, 0xb4, 0x53, 0x6d,
		0x0d, 0x00, 0x00, 0x14, 0x07, 0x47, 0x8d, 0xa7, 0x0b, 0xd6, 0xd1, 0x66,
		0x7a, 0xaf, 0x2e, 0x61, 0x2a, 0x91, 0x80, 0x94, 0x0d, 0x00, 0x00, 0x14,
		0x12, 0xf5, 0xf2, 0x8c, 0x45, 0x71, 0x68, 0xa9, 0x70, 0x2d, 0x9f, 0xe2,
		0x74, 0xcc, 0x01, 0x00, 0x0d, 0x00, 0x00, 0x0c, 0x09, 0x00, 0x26, 0x89,
		0xdf, 0xd6, 0xb7, 0x12, 0x0d, 0x00, 0x00, 0x14, 0xaf, 0xca, 0xd7, 0x13,
		0x68, 0xa1, 0xf1, 0xc9, 0x6b, 0x86, 0x96, 0xfc, 0x77, 0x57, 0x01, 0x00,
		0x00, 0x00, 0x00, 0x14, 0x1f, 0x07, 0xf7, 0x0e, 0xaa, 0x65, 0x14, 0xd3,
		0xb0, 0xfa, 0x96, 0x54, 0x2a, 0x50, 0x03, 0x05
	};
	uint8_t *unpack;
	size_t unpack_len;
	struct isakmp_packet *p;
	int reject;

	p = parse_isakmp_packet(pack, sizeof(pack), &reject);
	flatten_isakmp_packet(p, &unpack, &unpack_len, 8);
	if (unpack_len != sizeof(pack)
		|| memcmp(unpack, pack, sizeof(pack)) != 0)
		abort();
	free(unpack);
	free_isakmp_packet(p);
}
