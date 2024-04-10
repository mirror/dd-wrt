// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include <memory.h>
#include <endian.h>
#include <glib.h>
#include <errno.h>
#include <linux/ksmbd_server.h>

#include <rpc.h>
#include <rpc_srvsvc.h>
#include <rpc_wkssvc.h>
#include <rpc_samr.h>
#include <rpc_lsarpc.h>
#include <tools.h>

static GHashTable	*pipes_table;
static GRWLock		pipes_table_lock;

/*
 * Version 2.0 data representation protocol
 *
 * UUID: 8a885d04-1ceb-11c9-9fe8-08002b104860
 * VERSION: 2
 *
 *
 * Transfer Syntax: Bind Time Feature Negotiation
 * UUID:6cb71c2c-9812-4540-0300-000000000000
 *
 * 6CB71C2C-9812-4540
 *
 * MUST BE BLOCKED
 * Interface: SRVSVC UUID: 4b324fc8-1670-01d3-1278-5a47bf6ee188
 */
struct dcerpc_syntax_table {
	struct dcerpc_syntax	syn;
	int			ack_result;
};

static struct dcerpc_syntax_table known_syntaxes[] = {
	{
		.syn.uuid.time_low = 0x8a885d04,
		.syn.uuid.time_mid = 0x1ceb,
		.syn.uuid.time_hi_and_version = 0x11c9,
		.syn.uuid.clock_seq = {0x9f, 0xe8},
		.syn.uuid.node = {0x8, 0x0, 0x2b, 0x10, 0x48, 0x60},
		.syn.ver_major = 0x2,
		.syn.ver_minor = 0x0,
		.ack_result = DCERPC_BIND_ACK_RES_ACCEPT,
	},
	{
		.syn.uuid.time_low = 0x6CB71C2C,
		.syn.uuid.time_mid = 0x9812,
		.syn.uuid.time_hi_and_version = 0x4540,
		.syn.uuid.clock_seq = {0x0, 0x0},
		.syn.uuid.node = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
		.syn.ver_major = 0x1,
		.syn.ver_minor = 0x0,
		.ack_result = DCERPC_BIND_ACK_RES_NEGOTIATE_ACK,
	},
};

/*
 * PNIO uuid
 * Transfer Syntax: PNIO (Implicit Ar)
 *
 * All zero-s.
 */
static struct dcerpc_syntax negotiate_ack_PNIO_uuid;

/*
 * We need a proper DCE RPC (ndr/ndr64) parser. And we also need a proper
 * IDL support...
 * Maybe someone smart and cool enough can do it for us. The one you can
 * find here is just a very simple implementation, which sort of works for
 * us, but we do realize that it sucks.
 *
 * Documentation:
 *
 * http://pubs.opengroup.org/onlinepubs/9629399/chap14.htm#tagfcjh_39
 * https://msdn.microsoft.com/en-us/library/cc243858.aspx
 */

#define PAYLOAD_HEAD(d)	((d)->payload + (d)->offset)

#define __ALIGN(x, a)							\
	({								\
		typeof(x) ret = (x);					\
		if (((x) & ((typeof(x))(a) - 1)) != 0)			\
			ret = __ALIGN_MASK(x, (typeof(x))(a) - 1);	\
		ret;							\
	})

#define __ALIGN_MASK(x, mask)	(((x) + (mask)) & ~(mask))

static struct ksmbd_rpc_pipe *rpc_pipe_lookup(unsigned int id)
{
	struct ksmbd_rpc_pipe *pipe;

	g_rw_lock_reader_lock(&pipes_table_lock);
	pipe = g_hash_table_lookup(pipes_table, &id);
	g_rw_lock_reader_unlock(&pipes_table_lock);

	return pipe;
}

static void dcerpc_free(struct ksmbd_dcerpc *dce)
{
	if (!(dce->flags & KSMBD_DCERPC_EXTERNAL_PAYLOAD))
		g_free(dce->payload);
	g_free(dce);
}

static struct ksmbd_dcerpc *dcerpc_ext_alloc(unsigned int flags,
					     void *payload,
					     int payload_sz)
{
	struct ksmbd_dcerpc *dce;

	dce = g_try_malloc0(sizeof(struct ksmbd_dcerpc));
	if (!dce)
		return NULL;

	dce->payload = payload;
	dce->payload_sz = payload_sz;

	dce->flags = flags;
	dce->flags |= KSMBD_DCERPC_EXTERNAL_PAYLOAD;
	dce->flags |= KSMBD_DCERPC_FIXED_PAYLOAD_SZ;
	return dce;
}

void dcerpc_set_ext_payload(struct ksmbd_dcerpc *dce, void *payload, size_t sz)
{
	dce->num_pointers = 1;
	dce->payload = payload;
	dce->payload_sz = sz;
	dce->offset = 0;
	dce->flags |= KSMBD_DCERPC_EXTERNAL_PAYLOAD;
	dce->flags |= KSMBD_DCERPC_FIXED_PAYLOAD_SZ;
}

void rpc_pipe_reset(struct ksmbd_rpc_pipe *pipe)
{
	if (pipe->entry_processed) {
		while (pipe->num_entries)
			pipe->entry_processed(pipe, 0);
	}
	pipe->num_entries = 0;
}

static void __rpc_pipe_free(struct ksmbd_rpc_pipe *pipe)
{
	rpc_pipe_reset(pipe);
	if (pipe->dce)
		dcerpc_free(pipe->dce);
	if (pipe->entries)
		g_ptr_array_free(pipe->entries, 1);
	g_free(pipe);
}

static void rpc_pipe_free(struct ksmbd_rpc_pipe *pipe)
{
	if (pipe->id != (unsigned int)-1) {
		g_rw_lock_writer_lock(&pipes_table_lock);
		g_hash_table_remove(pipes_table, &(pipe->id));
		g_rw_lock_writer_unlock(&pipes_table_lock);
	}

	__rpc_pipe_free(pipe);
}

static struct ksmbd_rpc_pipe *rpc_pipe_alloc(void)
{
	struct ksmbd_rpc_pipe *pipe;

	pipe = g_try_malloc0(sizeof(struct ksmbd_rpc_pipe));
	if (!pipe)
		return NULL;

	pipe->id = -1;
	pipe->entries = g_ptr_array_new();
	return pipe;
}

static struct ksmbd_rpc_pipe *rpc_pipe_alloc_bind(unsigned int id)
{
	struct ksmbd_rpc_pipe *pipe = rpc_pipe_alloc();
	int ret;

	if (!pipe)
		return NULL;

	pipe->id = id;
	g_rw_lock_writer_lock(&pipes_table_lock);
	ret = g_hash_table_insert(pipes_table, &(pipe->id), pipe);
	g_rw_lock_writer_unlock(&pipes_table_lock);

	if (!ret) {
		pipe->id = (unsigned int)-1;
		rpc_pipe_free(pipe);
		pipe = NULL;
	}
	return pipe;
}

static void __clear_pipes_table(void)
{
	struct ksmbd_rpc_pipe *pipe;
	GHashTableIter iter;

	g_rw_lock_writer_lock(&pipes_table_lock);
	ghash_for_each(pipe, pipes_table, iter)
		__rpc_pipe_free(pipe);
	g_rw_lock_writer_unlock(&pipes_table_lock);
}

static void align_offset(struct ksmbd_dcerpc *dce, size_t n)
{
	dce->offset = __ALIGN(dce->offset, n);
}

void auto_align_offset(struct ksmbd_dcerpc *dce)
{
	if (dce->flags & KSMBD_DCERPC_ALIGN8)
		dce->offset = __ALIGN(dce->offset, 8);
	else if (dce->flags & KSMBD_DCERPC_ALIGN4)
		dce->offset = __ALIGN(dce->offset, 4);
}

static int try_realloc_payload(struct ksmbd_dcerpc *dce, size_t data_sz)
{
	char *n;

	if (dce->offset + data_sz < dce->payload_sz)
		return 0;

	if (dce->flags & KSMBD_DCERPC_FIXED_PAYLOAD_SZ) {
		pr_err("DCE RPC: fixed payload buffer overflow\n");
		return -ENOMEM;
	}

	n = g_try_realloc(dce->payload, dce->payload_sz + 4096);
	if (!n)
		return -ENOMEM;

	dce->payload = n;
	dce->payload_sz += 4096;
	memset(dce->payload + dce->offset, 0, dce->payload_sz - dce->offset);
	return 0;
}

static __u8 noop_int8(__u8 v)
{
	return v;
}

#define htobe_n noop_int8
#define htole_n noop_int8
#define betoh_n noop_int8
#define letoh_n noop_int8

#define NDR_WRITE_INT(name, type, be, le)				\
int ndr_write_##name(struct ksmbd_dcerpc *dce, type value)		\
{									\
	align_offset(dce, sizeof(type));				\
	if (try_realloc_payload(dce, sizeof(value)))			\
		return -ENOMEM;						\
	if (dce->flags & KSMBD_DCERPC_LITTLE_ENDIAN)			\
		*(type *)PAYLOAD_HEAD(dce) = le(value);			\
	else								\
		*(type *)PAYLOAD_HEAD(dce) = be(value);			\
	dce->offset += sizeof(value);					\
	return 0;							\
}

NDR_WRITE_INT(int8,  __u8, htobe_n, htole_n);
NDR_WRITE_INT(int16, __u16, htobe16, htole16);
NDR_WRITE_INT(int32, __u32, htobe32, htole32);
NDR_WRITE_INT(int64, __u64, htobe64, htole64);

#define NDR_READ_INT(name, type, be, le)				\
int ndr_read_##name(struct ksmbd_dcerpc *dce, type *value)		\
{									\
	type ret;							\
									\
	align_offset(dce, sizeof(type));				\
	if (dce->offset + sizeof(type) > dce->payload_sz)		\
		return -EINVAL;						\
									\
	if (dce->flags & KSMBD_DCERPC_LITTLE_ENDIAN)			\
		ret = le(*(type *)PAYLOAD_HEAD(dce));			\
	else								\
		ret = be(*(type *)PAYLOAD_HEAD(dce));			\
	dce->offset += sizeof(type);					\
	if (value)							\
		*value = ret;						\
	return 0;							\
}

NDR_READ_INT(int8,  __u8, betoh_n, letoh_n);
NDR_READ_INT(int16, __u16, be16toh, le16toh);
NDR_READ_INT(int32, __u32, be32toh, le32toh);
NDR_READ_INT(int64, __u64, be64toh, le64toh);

/*
 * For a non-encapsulated union, the discriminant is marshalled into
 * the transmitted data stream twice: once as the field or parameter,
 * which is referenced by the switch_is construct, in the procedure
 * argument list; and once as the first part of the union
 * representation.
 */
#define NDR_WRITE_UNION(name, type)					\
int ndr_write_union_##name(struct ksmbd_dcerpc *dce, type value)	\
{									\
	int ret;							\
									\
	ret = ndr_write_##name(dce, value);				\
	if (ret)							\
		return ret;						\
	ret = ndr_write_##name(dce, value);				\
	return ret;							\
}

NDR_WRITE_UNION(int16, __u16);
NDR_WRITE_UNION(int32, __u32);

#define NDR_READ_UNION(name, type)					\
int ndr_read_union_##name(struct ksmbd_dcerpc *dce, type *value)	\
{									\
	type val1, val2;						\
									\
	if (ndr_read_##name(dce, &val1))				\
		return -EINVAL;						\
	if (ndr_read_##name(dce, &val2))				\
		return -EINVAL;						\
	if (val1 != val2) {						\
		pr_err("NDR: union representation mismatch %lu\n",	\
				(unsigned long)val1);			\
		return -EINVAL;						\
	}								\
	if (value)							\
		*value = val1;						\
	return 0;							\
}

NDR_READ_UNION(int32, __u32);

int ndr_write_bytes(struct ksmbd_dcerpc *dce, void *value, size_t sz)
{
	align_offset(dce, 2);
	if (try_realloc_payload(dce, sz))
		return -ENOMEM;

	memcpy(PAYLOAD_HEAD(dce), value, sz);
	dce->offset += sz;
	return 0;
}

int ndr_read_bytes(struct ksmbd_dcerpc *dce, void *value, size_t sz)
{
	align_offset(dce, 2);
	if (dce->offset + sz > dce->payload_sz)
		return -EINVAL;
	memcpy(value, PAYLOAD_HEAD(dce), sz);
	dce->offset += sz;
	return 0;
}

static gchar *ndr_convert_char_to_unicode(struct ksmbd_dcerpc *dce, char *str,
		size_t len, gsize *bytes_written)
{
	gchar *out;
	gsize bytes_read = 0;
	int charset = KSMBD_CHARSET_UTF16LE;

	if (!(dce->flags & KSMBD_DCERPC_LITTLE_ENDIAN))
		charset = KSMBD_CHARSET_UTF16BE;

	if (dce->flags & KSMBD_DCERPC_ASCII_STRING)
		charset = KSMBD_CHARSET_UTF8;

	out = ksmbd_gconvert(str,
			     len,
			     charset,
			     KSMBD_CHARSET_DEFAULT,
			     &bytes_read,
			     bytes_written);

	return out;
}

int ndr_write_vstring(struct ksmbd_dcerpc *dce, void *value)
{
	g_autofree char *out = NULL;
	gsize bytes_written = 0;

	size_t raw_len, str_len;
	char *raw_value = value;
	int ret;

	if (!value)
		raw_value = "";

	raw_len = strlen(raw_value) + 1;
	out = ndr_convert_char_to_unicode(dce, raw_value, raw_len,
			&bytes_written);
	if (!out)
		return -EINVAL;

	str_len = g_utf8_strlen(raw_value, -1) + 1;

	/*
	 * NDR represents a conformant and varying string as an ordered
	 * sequence of representations of the string elements, preceded
	 * by three unsigned long integers. The first integer gives the
	 * maximum number of elements in the string, including the terminator.
	 * The second integer gives the offset from the first index of the
	 * string to the first index of the actual subset being passed.
	 * The third integer gives the actual number of elements being
	 * passed, including the terminator.
	 */
	ret = ndr_write_int32(dce, str_len);
	if (ret)
		return ret;

	ret = ndr_write_int32(dce, 0);
	if (ret)
		return ret;

	ret = ndr_write_int32(dce, str_len);
	if (ret)
		return ret;

	ret = ndr_write_bytes(dce, out, bytes_written);
	if (ret)
		return ret;
	auto_align_offset(dce);

	return ret;
}

int ndr_write_string(struct ksmbd_dcerpc *dce, char *str)
{
	g_autofree char *out = NULL;
	gsize bytes_written = 0;

	size_t len;
	int ret;

	if (!str)
		str = "";

	len = strlen(str);
	out = ndr_convert_char_to_unicode(dce, str, len, &bytes_written);
	if (!out)
		return -EINVAL;

	ret = ndr_write_int32(dce, len); // max count
	if (ret)
		return ret;

	ret = ndr_write_int32(dce, 0);
	if (ret)
		return ret;

	ret = ndr_write_int32(dce, len); // actual count
	if (ret)
		return ret;

	ret = ndr_write_bytes(dce, out, bytes_written);
	auto_align_offset(dce);

	return ret;
}

int ndr_write_lsa_string(struct ksmbd_dcerpc *dce, char *str)
{
	g_autofree char *out = NULL;
	gsize bytes_written = 0;

	size_t len;
	int ret;

	if (!str)
		str = "";

	len = strlen(str);
	out = ndr_convert_char_to_unicode(dce, str, len, &bytes_written);
	if (!out)
		return -EINVAL;

	ret = ndr_write_int32(dce, len + 1); // max count
	if (ret)
		return ret;

	ret = ndr_write_int32(dce, 0);
	if (ret)
		return ret;

	ret = ndr_write_int32(dce, len); // actual count
	if (ret)
		return ret;

	ret = ndr_write_bytes(dce, out, bytes_written);
	auto_align_offset(dce);

	return ret;
}

char *ndr_read_vstring(struct ksmbd_dcerpc *dce)
{
	gchar *out;
	gsize bytes_read = 0;
	gsize bytes_written = 0;

	int raw_len;
	int charset = KSMBD_CHARSET_UTF16LE;

	if (ndr_read_int32(dce, &raw_len))
		return NULL;
	/* read in offset */
	if (ndr_read_int32(dce, NULL))
		return NULL;
	if (ndr_read_int32(dce, NULL))
		return NULL;

	if (!(dce->flags & KSMBD_DCERPC_LITTLE_ENDIAN))
		charset = KSMBD_CHARSET_UTF16BE;

	if (dce->flags & KSMBD_DCERPC_ASCII_STRING)
		charset = KSMBD_CHARSET_UTF8;

	if (raw_len == 0) {
		out = g_strdup("");
		return out;
	}

	if (dce->offset + 2 * raw_len > dce->payload_sz)
		return NULL;

	out = ksmbd_gconvert(PAYLOAD_HEAD(dce),
			     raw_len * 2,
			     KSMBD_CHARSET_DEFAULT,
			     charset,
			     &bytes_read,
			     &bytes_written);
	if (!out)
		return NULL;

	dce->offset += raw_len * 2;
	auto_align_offset(dce);
	return out;
}

int ndr_read_vstring_ptr(struct ksmbd_dcerpc *dce, struct ndr_char_ptr *ctr)
{
	ctr->ptr = ndr_read_vstring(dce);
	if (!ctr->ptr)
		return -EINVAL;
	return 0;
}

int ndr_read_uniq_vstring_ptr(struct ksmbd_dcerpc *dce,
			      struct ndr_uniq_char_ptr *ctr)
{
	if (ndr_read_int32(dce, &ctr->ref_id))
		return -EINVAL;

	if (ctr->ref_id == 0) {
		ctr->ptr = NULL;
		return 0;
	}
	ctr->ptr = ndr_read_vstring(dce);
	if (!ctr->ptr)
		return -EINVAL;
	return 0;
}

void ndr_free_vstring_ptr(struct ndr_char_ptr *ctr)
{
	g_free(ctr->ptr);
	ctr->ptr = NULL;
}

void ndr_free_uniq_vstring_ptr(struct ndr_uniq_char_ptr *ctr)
{
	ctr->ref_id = 0;
	g_free(ctr->ptr);
	ctr->ptr = NULL;
}

int ndr_read_ptr(struct ksmbd_dcerpc *dce, struct ndr_ptr *ctr)
{
	if (ndr_read_int32(dce, &ctr->ptr))
		return -EINVAL;
	return 0;
}

int ndr_read_uniq_ptr(struct ksmbd_dcerpc *dce, struct ndr_uniq_ptr *ctr)
{
	if (ndr_read_int32(dce, &ctr->ref_id))
		return -EINVAL;
	if (ctr->ref_id == 0) {
		ctr->ptr = 0;
		return 0;
	}
	if (ndr_read_int32(dce, &ctr->ptr))
		return -EINVAL;
	return 0;
}

static int __max_entries(struct ksmbd_dcerpc *dce, struct ksmbd_rpc_pipe *pipe)
{
	int current_size, i;

	if (!(dce->flags & KSMBD_DCERPC_FIXED_PAYLOAD_SZ))
		return pipe->num_entries;

	if (!dce->entry_size) {
		pr_err("No ->entry_size() callback was provided\n");
		return pipe->num_entries;
	}

	current_size = 0;
	for (i = 0; i < pipe->num_entries; i++) {
		void *entry;

		entry = g_ptr_array_index(pipe->entries, i);
		current_size += dce->entry_size(dce, entry);

		if (current_size < 4 * dce->payload_sz / 5)
			continue;
		return i;
	}

	return pipe->num_entries;
}

int __ndr_write_array_of_structs(struct ksmbd_rpc_pipe *pipe, int max_entry_nr)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	int i;

	for (i = 0; i < max_entry_nr; i++) {
		void *entry;

		entry = g_ptr_array_index(pipe->entries, i);
		if (dce->entry_rep(dce, entry))
			return KSMBD_RPC_EBAD_DATA;
	}

	for (i = 0; i < max_entry_nr; i++) {
		void *entry;

		entry = g_ptr_array_index(pipe->entries, i);
		if (dce->entry_data(dce, entry))
			return KSMBD_RPC_EBAD_DATA;
	}

	if (pipe->entry_processed) {
		for (i = 0; i < max_entry_nr; i++)
			pipe->entry_processed(pipe, 0);
	}

	return KSMBD_RPC_OK;
}

static int ndr_write_empty_array_of_struct(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;

	dce->num_pointers++;
	if (ndr_write_int32(dce, dce->num_pointers))
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, 0))
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, 0))
		return KSMBD_RPC_EBAD_DATA;

	return KSMBD_RPC_OK;
}

int ndr_write_array_of_structs(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	int max_entry_nr;

	/*
	 * In the NDR representation of a structure that contains a
	 * conformant and varying array, the maximum counts for dimensions
	 * of the array are moved to the beginning of the structure, but
	 * the offsets and actual counts remain in place at the end of the
	 * structure, immediately preceding the array elements.
	 */

	if (pipe->num_entries == 0)
		return ndr_write_empty_array_of_struct(pipe);

	max_entry_nr = __max_entries(dce, pipe);
	if (ndr_write_int32(dce, max_entry_nr))
		return KSMBD_RPC_EBAD_DATA;
	/*
	 * ARRAY representation [per dimension]
	 *    max_count
	 *    offset
	 *    actual_count
	 *    element representation [1..N]
	 *    actual elements [1..N]
	 */
	if (ndr_write_int32(dce, max_entry_nr))
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, 1))
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, max_entry_nr))
		return KSMBD_RPC_EBAD_DATA;

	if (max_entry_nr == 0) {
		pr_err("DCERPC: can't fit any data, buffer is too small\n");
		rpc_pipe_reset(pipe);
		return KSMBD_RPC_EBAD_DATA;
	}

	return __ndr_write_array_of_structs(pipe, max_entry_nr);
}

void rpc_init(void)
{
	if (!pipes_table)
		pipes_table = g_hash_table_new(g_int_hash, g_int_equal);

	rpc_samr_init();
	rpc_lsarpc_init();
}

void rpc_destroy(void)
{
	rpc_lsarpc_destroy();
	rpc_samr_destroy();

	if (pipes_table) {
		__clear_pipes_table();
		g_hash_table_destroy(pipes_table);
		pipes_table = NULL;
	}
}

static int dcerpc_hdr_write(struct ksmbd_dcerpc *dce,
			    struct dcerpc_header *hdr)
{
	int ret;

	ret = ndr_write_int8(dce, hdr->rpc_vers);
	if (ret)
		return ret;

	ret = ndr_write_int8(dce, hdr->rpc_vers_minor);
	if (ret)
		return ret;

	ret = ndr_write_int8(dce, hdr->ptype);
	if (ret)
		return ret;

	ret = ndr_write_int8(dce, hdr->pfc_flags);
	if (ret)
		return ret;

	ret = ndr_write_bytes(dce, &hdr->packed_drep,
			      sizeof(hdr->packed_drep));
	if (ret)
		return ret;

	ret = ndr_write_int16(dce, hdr->frag_length);
	if (ret)
		return ret;

	ret = ndr_write_int16(dce, hdr->auth_length);
	if (ret)
		return ret;

	ret = ndr_write_int32(dce, hdr->call_id);

	return ret;
}

static int dcerpc_hdr_read(struct ksmbd_dcerpc *dce,
			   struct dcerpc_header *hdr)
{
	/* Common Type Header for the Serialization Stream */

	if (ndr_read_int8(dce, &hdr->rpc_vers))
		return -EINVAL;
	if (ndr_read_int8(dce, &hdr->rpc_vers_minor))
		return -EINVAL;
	if (ndr_read_int8(dce, &hdr->ptype))
		return -EINVAL;
	if (ndr_read_int8(dce, &hdr->pfc_flags))
		return -EINVAL;
	/*
	 * This common type header MUST be presented by using
	 * little-endian format in the octet stream. The first
	 * byte of the common type header MUST be equal to 1 to
	 * indicate level 1 of type serialization.
	 *
	 * Type serialization version 1 can use either a little-endian
	 * or big-endian integer and floating-pointer byte order but
	 * MUST use the IEEE floating-point format representation and
	 * ASCII character format.
	 */
	if (ndr_read_bytes(dce, &hdr->packed_drep, sizeof(hdr->packed_drep)))
		return -EINVAL;

	dce->flags |= KSMBD_DCERPC_ALIGN4;
	dce->flags |= KSMBD_DCERPC_LITTLE_ENDIAN;
	if (hdr->packed_drep[0] != DCERPC_SERIALIZATION_LITTLE_ENDIAN)
		dce->flags &= ~KSMBD_DCERPC_LITTLE_ENDIAN;

	if (ndr_read_int16(dce, &hdr->frag_length))
		return -EINVAL;
	if (ndr_read_int16(dce, &hdr->auth_length))
		return -EINVAL;
	if (ndr_read_int32(dce, &hdr->call_id))
		return -EINVAL;
	return 0;
}

static int dcerpc_response_hdr_write(struct ksmbd_dcerpc *dce,
				     struct dcerpc_response_header *hdr)
{
	int ret;

	ret = ndr_write_int32(dce, hdr->alloc_hint);
	if (ret)
		return ret;

	ret = ndr_write_int16(dce, hdr->context_id);
	if (ret)
		return ret;

	ret = ndr_write_int8(dce, hdr->cancel_count);
	auto_align_offset(dce);

	return ret;
}

static int dcerpc_request_hdr_read(struct ksmbd_dcerpc *dce,
				   struct dcerpc_request_header *hdr)
{
	if (ndr_read_int32(dce, &hdr->alloc_hint))
		return -EINVAL;
	if (ndr_read_int16(dce, &hdr->context_id))
		return -EINVAL;
	if (ndr_read_int16(dce, &hdr->opnum))
		return -EINVAL;
	return 0;
}

int dcerpc_write_headers(struct ksmbd_dcerpc *dce, int method_status)
{
	int payload_offset, ret;

	payload_offset = dce->offset;
	dce->offset = 0;

	dce->hdr.ptype = DCERPC_PTYPE_RPC_RESPONSE;
	dce->hdr.pfc_flags = DCERPC_PFC_FIRST_FRAG | DCERPC_PFC_LAST_FRAG;
	dce->hdr.frag_length = payload_offset;
	if (method_status == KSMBD_RPC_EMORE_DATA)
		dce->hdr.pfc_flags = 0;
	ret = dcerpc_hdr_write(dce, &dce->hdr);
	if (ret)
		return ret;

	/* cast req_hdr to resp_hdr and NULL out lower 2 bytes */
	dce->req_hdr.opnum = 0;
	dce->resp_hdr.cancel_count = 0;
	dce->resp_hdr.alloc_hint = payload_offset;
	ret = dcerpc_response_hdr_write(dce, &dce->resp_hdr);
	if (ret)
		return ret;

	dce->offset = payload_offset;
	return 0;
}

static int __dcerpc_read_syntax(struct ksmbd_dcerpc *dce,
				struct dcerpc_syntax *syn)
{
	if (ndr_read_int32(dce, &syn->uuid.time_low))
		return -EINVAL;
	if (ndr_read_int16(dce, &syn->uuid.time_mid))
		return -EINVAL;
	if (ndr_read_int16(dce, &syn->uuid.time_hi_and_version))
		return -EINVAL;
	if (ndr_read_bytes(dce, syn->uuid.clock_seq,
			   sizeof(syn->uuid.clock_seq)))
		return -EINVAL;
	if (ndr_read_bytes(dce, syn->uuid.node, sizeof(syn->uuid.node)))
		return -EINVAL;
	if (ndr_read_int16(dce, &syn->ver_major))
		return -EINVAL;
	if (ndr_read_int16(dce, &syn->ver_minor))
		return -EINVAL;
	return 0;
}

static int __dcerpc_write_syntax(struct ksmbd_dcerpc *dce,
				 struct dcerpc_syntax *syn)
{
	int ret;

	ret = ndr_write_int32(dce, syn->uuid.time_low);
	if (ret)
		return ret;

	ret = ndr_write_int16(dce, syn->uuid.time_mid);
	if (ret)
		return ret;

	ret = ndr_write_int16(dce, syn->uuid.time_hi_and_version);
	if (ret)
		return ret;

	ret = ndr_write_bytes(dce, syn->uuid.clock_seq,
			      sizeof(syn->uuid.clock_seq));
	if (ret)
		return ret;

	ret = ndr_write_bytes(dce, syn->uuid.node, sizeof(syn->uuid.node));
	if (ret)
		return ret;

	ret = ndr_write_int16(dce, syn->ver_major);
	if (ret)
		return ret;

	ret = ndr_write_int16(dce, syn->ver_minor);

	return ret;
}

static void dcerpc_bind_req_free(struct dcerpc_bind_request *hdr)
{
	int i;

	for (i = 0; i < hdr->num_contexts; i++)
		g_free(hdr->list[i].transfer_syntaxes);
	g_free(hdr->list);
	hdr->list = NULL;
	hdr->num_contexts = 0;
}

static int dcerpc_parse_bind_req(struct ksmbd_dcerpc *dce,
				 struct dcerpc_bind_request *hdr)
{
	int i, j;
	int ret = -EINVAL;

	hdr->flags = dce->rpc_req->flags;
	if (ndr_read_int16(dce, &hdr->max_xmit_frag_sz))
		return -EINVAL;
	if (ndr_read_int16(dce, &hdr->max_recv_frag_sz))
		return -EINVAL;
	if (ndr_read_int32(dce, &hdr->assoc_group_id))
		return -EINVAL;
	if (ndr_read_int8(dce, &hdr->num_contexts))
		return -EINVAL;
	hdr->list = NULL;
	auto_align_offset(dce);

	if (!hdr->num_contexts)
		return 0;

	hdr->list = g_try_malloc0_n(hdr->num_contexts, sizeof(struct dcerpc_context));
	if (!hdr->list)
		return -ENOMEM;

	for (i = 0; i < hdr->num_contexts; i++) {
		struct dcerpc_context *ctx = &hdr->list[i];

		if (ndr_read_int16(dce, &ctx->id))
			goto fail;
		if (ndr_read_int8(dce, &ctx->num_syntaxes))
			goto fail;
		if (!ctx->num_syntaxes) {
			pr_err("BIND: zero syntaxes provided\n");
			goto fail;
		}

		__dcerpc_read_syntax(dce, &ctx->abstract_syntax);

		ctx->transfer_syntaxes = g_try_malloc0_n(ctx->num_syntaxes,
						sizeof(struct dcerpc_syntax));
		if (!ctx->transfer_syntaxes) {
			ret = -ENOMEM;
			goto fail;
		}

		for (j = 0; j < ctx->num_syntaxes; j++)
			__dcerpc_read_syntax(dce, &ctx->transfer_syntaxes[j]);
	}
	return KSMBD_RPC_OK;

fail:
	g_free(hdr->list);
	return ret;
}

static int dcerpc_bind_invoke(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce;

	dce = pipe->dce;
	if (dcerpc_parse_bind_req(dce, &dce->bi_req))
		return KSMBD_RPC_EBAD_DATA;

	pipe->entry_processed = NULL;
	return KSMBD_RPC_OK;
}

static int dcerpc_syntax_cmp(struct dcerpc_syntax *a, struct dcerpc_syntax *b)
{
	if (a->uuid.time_low != b->uuid.time_low)
		return -1;
	if (a->uuid.time_mid != b->uuid.time_mid)
		return -1;
	if (a->uuid.time_hi_and_version != b->uuid.time_hi_and_version)
		return -1;
	if (a->ver_major != b->ver_major)
		return -1;
	return 0;
}

static int dcerpc_syntax_supported(struct dcerpc_syntax *a)
{
	int k;

	for (k = 0; k < ARRAY_SIZE(known_syntaxes); k++) {
		struct dcerpc_syntax *b = &known_syntaxes[k].syn;

		if (!dcerpc_syntax_cmp(a, b))
			return known_syntaxes[k].ack_result;
	}
	return -1;
}

static int dcerpc_bind_nack_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	int i, payload_offset;

	dce->offset = sizeof(struct dcerpc_header);

	if (ndr_write_int16(dce,
			    DCERPC_BIND_NAK_RSN_PROTOCOL_VERSION_NOT_SUPPORTED))
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int8(dce, ARRAY_SIZE(known_syntaxes)))
		return KSMBD_RPC_EBAD_DATA;

	auto_align_offset(dce);

	for (i = 0; i < ARRAY_SIZE(known_syntaxes); i++) {
		if (ndr_write_int8(dce, known_syntaxes[i].syn.ver_major))
			return KSMBD_RPC_EBAD_DATA;

		if (ndr_write_int8(dce, known_syntaxes[i].syn.ver_minor))
			return KSMBD_RPC_EBAD_DATA;
	}

	payload_offset = dce->offset;
	dce->offset = 0;

	dce->hdr.ptype = DCERPC_PTYPE_RPC_BINDNACK;
	dce->hdr.pfc_flags = DCERPC_PFC_FIRST_FRAG | DCERPC_PFC_LAST_FRAG;
	dce->hdr.frag_length = payload_offset;
	if (dcerpc_hdr_write(dce, &dce->hdr))
		return KSMBD_RPC_EBAD_DATA;

	dce->offset = payload_offset;
	dce->rpc_resp->payload_sz = dce->offset;
	return KSMBD_RPC_OK;
}

static int dcerpc_bind_ack_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	int num_trans, i, payload_offset;
	char *addr;

	dce->offset = sizeof(struct dcerpc_header);

	/*
	 * Preserve bind assoc group, if was specified.
	 */
	if (dce->bi_req.assoc_group_id == 0)
		dce->bi_req.assoc_group_id = 0x53f0;

	if (ndr_write_int16(dce, dce->bi_req.max_xmit_frag_sz))
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int16(dce, dce->bi_req.max_recv_frag_sz))
		return KSMBD_RPC_EBAD_DATA;

	if (ndr_write_int32(dce, dce->bi_req.assoc_group_id))
		return KSMBD_RPC_EBAD_DATA;

	if (dce->bi_req.flags & KSMBD_RPC_SRVSVC_METHOD_INVOKE)
		addr = "\\PIPE\\srvsvc";
	else if (dce->bi_req.flags & KSMBD_RPC_WKSSVC_METHOD_INVOKE)
		addr = "\\PIPE\\wkssvc";
	else if (dce->bi_req.flags & KSMBD_RPC_SAMR_METHOD_INVOKE)
		addr = "\\PIPE\\samr";
	else if (dce->bi_req.flags & KSMBD_RPC_LSARPC_METHOD_INVOKE)
		addr = "\\PIPE\\lsarpc";
	else
		return KSMBD_RPC_EBAD_FUNC;

	if (dce->hdr.ptype == DCERPC_PTYPE_RPC_ALTCONT) {
		if (ndr_write_int16(dce, 0))
			return KSMBD_RPC_EBAD_DATA;

		if (ndr_write_int16(dce, 0))
			return KSMBD_RPC_EBAD_DATA;
	} else {
		if (ndr_write_int16(dce, strlen(addr)))
			return KSMBD_RPC_EBAD_DATA;

		if (ndr_write_bytes(dce, addr, strlen(addr)))
			return KSMBD_RPC_EBAD_DATA;
	}
	align_offset(dce, 4); /* [flag(NDR_ALIGN4)]    DATA_BLOB _pad1; */

	num_trans = dce->bi_req.num_contexts;
	if (ndr_write_int8(dce, num_trans))
		return KSMBD_RPC_EBAD_DATA;

	align_offset(dce, 2);

	for (i = 0; i < num_trans; i++) {
		struct dcerpc_syntax *s;
		__s16 result;

		s = &dce->bi_req.list[i].transfer_syntaxes[0];
		result = dcerpc_syntax_supported(s);

		if (result == -1) {
			result = DCERPC_BIND_ACK_RES_PROVIDER_REJECT;
			if (ndr_write_union_int16(dce, result))
				return KSMBD_RPC_EBAD_DATA;
		} else {
			if (result == DCERPC_BIND_ACK_RES_ACCEPT) {
				if (ndr_write_union_int16(dce, result))
					return KSMBD_RPC_EBAD_DATA;
			}
			if (result == DCERPC_BIND_ACK_RES_NEGOTIATE_ACK) {
				if (ndr_write_int16(dce, result))
					return KSMBD_RPC_EBAD_DATA;

				if (ndr_write_int16(dce, 0x00))
					return KSMBD_RPC_EBAD_DATA;
				s = &negotiate_ack_PNIO_uuid;
			}
		}
		if (__dcerpc_write_syntax(dce, s))
			return KSMBD_RPC_EBAD_DATA;
	}

	payload_offset = dce->offset;
	dce->offset = 0;

	if (dce->hdr.ptype == DCERPC_PTYPE_RPC_ALTCONT)
		dce->hdr.ptype = DCERPC_PTYPE_RPC_ALTCONTRESP;
	else
		dce->hdr.ptype = DCERPC_PTYPE_RPC_BINDACK;
	dce->hdr.pfc_flags = DCERPC_PFC_FIRST_FRAG | DCERPC_PFC_LAST_FRAG;
	dce->hdr.frag_length = payload_offset;
	if (dcerpc_hdr_write(dce, &dce->hdr))
		return KSMBD_RPC_EBAD_DATA;

	dce->offset = payload_offset;
	dce->rpc_resp->payload_sz = dce->offset;
	return KSMBD_RPC_OK;
}

static int dcerpc_bind_return(struct ksmbd_rpc_pipe *pipe)
{
	struct ksmbd_dcerpc *dce = pipe->dce;
	int i, j, ack = 0, ret;

	for (i = 0; i < dce->bi_req.num_contexts; i++) {
		for (j = 0; j < dce->bi_req.list[i].num_syntaxes; j++) {
			static struct dcerpc_syntax *a;

			a = &dce->bi_req.list[i].transfer_syntaxes[j];
			if (dcerpc_syntax_supported(a) != -1) {
				ack = 1;
				break;
			}
		}
	}

	if (!ack) {
		pr_err("Unsupported transfer syntax\n");
		ret =  dcerpc_bind_nack_return(pipe);
	} else {
		ret = dcerpc_bind_ack_return(pipe);
	}

	dcerpc_bind_req_free(&dce->bi_req);
	return ret;
}

int rpc_restricted_context(struct ksmbd_rpc_command *req)
{
	if (global_conf.restrict_anon == 0)
		return 0;

	return req->flags & KSMBD_RPC_RESTRICTED_CONTEXT;
}

int rpc_ioctl_request(struct ksmbd_rpc_command *req,
		      struct ksmbd_rpc_command *resp,
		      int max_resp_sz)
{
	int ret;

	ret = rpc_write_request(req, resp);
	if (ret == KSMBD_RPC_OK)
		return rpc_read_request(req, resp, max_resp_sz);
	return ret;
}

int rpc_read_request(struct ksmbd_rpc_command *req,
		     struct ksmbd_rpc_command *resp,
		     int max_resp_sz)
{
	int ret = KSMBD_RPC_ENOTIMPLEMENTED;
	struct ksmbd_rpc_pipe *pipe;
	struct ksmbd_dcerpc *dce;

	pipe = rpc_pipe_lookup(req->handle);
	if (!pipe || !pipe->dce) {
		pr_err("RPC: no pipe or pipe has no associated DCE [%d]\n",
			req->handle);
		return KSMBD_RPC_EBAD_FID;
	}

	dce = pipe->dce;
	dce->flags &= ~KSMBD_DCERPC_RETURN_READY;
	dce->rpc_req = req;
	dce->rpc_resp = resp;
	dcerpc_set_ext_payload(dce, resp->payload, max_resp_sz);

	if (dce->hdr.ptype == DCERPC_PTYPE_RPC_BIND ||
	    dce->hdr.ptype == DCERPC_PTYPE_RPC_ALTCONT)
		return dcerpc_bind_return(pipe);

	if (dce->hdr.ptype != DCERPC_PTYPE_RPC_REQUEST)
		return KSMBD_RPC_ENOTIMPLEMENTED;

	if (req->flags & KSMBD_RPC_SRVSVC_METHOD_INVOKE)
		return rpc_srvsvc_read_request(pipe, resp, max_resp_sz);

	if (req->flags & KSMBD_RPC_WKSSVC_METHOD_INVOKE)
		return rpc_wkssvc_read_request(pipe, resp, max_resp_sz);

	if (req->flags & KSMBD_RPC_SAMR_METHOD_INVOKE)
		return rpc_samr_read_request(pipe, resp, max_resp_sz);

	if (req->flags & KSMBD_RPC_LSARPC_METHOD_INVOKE)
		return rpc_lsarpc_read_request(pipe, resp, max_resp_sz);
	return ret;
}

int rpc_write_request(struct ksmbd_rpc_command *req,
		      struct ksmbd_rpc_command *resp)
{
	struct ksmbd_rpc_pipe *pipe;
	struct ksmbd_dcerpc *dce;

	pipe = rpc_pipe_lookup(req->handle);
	if (!pipe)
		return KSMBD_RPC_ENOMEM;

	if (pipe->dce->flags & KSMBD_DCERPC_RETURN_READY)
		return KSMBD_RPC_OK;

	if (pipe->num_entries)
		pr_err("RPC: A call on unflushed pipe. Pending %d\n",
			pipe->num_entries);

	dce = pipe->dce;
	dce->rpc_req = req;
	dce->rpc_resp = resp;
	dcerpc_set_ext_payload(dce, req->payload, req->payload_sz);
	dce->flags |= KSMBD_DCERPC_RETURN_READY;

	if (dcerpc_hdr_read(dce, &dce->hdr))
		return KSMBD_RPC_EBAD_DATA;

	if (dce->hdr.ptype == DCERPC_PTYPE_RPC_BIND ||
	    dce->hdr.ptype == DCERPC_PTYPE_RPC_ALTCONT)
		return dcerpc_bind_invoke(pipe);

	if (dce->hdr.ptype != DCERPC_PTYPE_RPC_REQUEST)
		return KSMBD_RPC_ENOTIMPLEMENTED;

	if (dcerpc_request_hdr_read(dce, &dce->req_hdr))
		return KSMBD_RPC_EBAD_DATA;

	if (req->flags & KSMBD_RPC_SRVSVC_METHOD_INVOKE)
		return rpc_srvsvc_write_request(pipe);

	if (req->flags & KSMBD_RPC_WKSSVC_METHOD_INVOKE)
		return rpc_wkssvc_write_request(pipe);

	if (req->flags & KSMBD_RPC_SAMR_METHOD_INVOKE)
		return rpc_samr_write_request(pipe);

	if (req->flags & KSMBD_RPC_LSARPC_METHOD_INVOKE)
		return rpc_lsarpc_write_request(pipe);
	return KSMBD_RPC_ENOTIMPLEMENTED;
}

int rpc_open_request(struct ksmbd_rpc_command *req,
		     struct ksmbd_rpc_command *resp)
{
	struct ksmbd_rpc_pipe *pipe;

	pipe = rpc_pipe_lookup(req->handle);
	if (pipe) {
		pr_err("RPC: pipe ID collision: %d\n", req->handle);
		return -EEXIST;
	}

	pipe = rpc_pipe_alloc_bind(req->handle);
	if (!pipe)
		return -ENOMEM;

	pipe->dce = dcerpc_ext_alloc(KSMBD_DCERPC_LITTLE_ENDIAN |
				     KSMBD_DCERPC_ALIGN4,
				     req->payload,
				     req->payload_sz);
	if (!pipe->dce) {
		rpc_pipe_free(pipe);
		return KSMBD_RPC_ENOMEM;
	}
	return KSMBD_RPC_OK;
}

int rpc_close_request(struct ksmbd_rpc_command *req,
		      struct ksmbd_rpc_command *resp)
{
	struct ksmbd_rpc_pipe *pipe;

	pipe = rpc_pipe_lookup(req->handle);
	if (pipe) {
		rpc_pipe_free(pipe);
		return 0;
	}

	pr_err("RPC: unknown pipe ID: %d\n", req->handle);
	return KSMBD_RPC_OK;
}
