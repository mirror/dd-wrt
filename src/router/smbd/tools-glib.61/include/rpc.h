/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#ifndef __KSMBD_RPC_H__
#define __KSMBD_RPC_H__

#include <linux/types.h>
#include <glib.h>

#define KSMBD_DCERPC_LITTLE_ENDIAN	(1 << 0)
#define KSMBD_DCERPC_ALIGN2		(1 << 1)
#define KSMBD_DCERPC_ALIGN4		(1 << 2)
#define KSMBD_DCERPC_ALIGN8		(1 << 3)
#define KSMBD_DCERPC_ASCII_STRING	(1 << 4)
#define KSMBD_DCERPC_FIXED_PAYLOAD_SZ	(1 << 5)
#define KSMBD_DCERPC_EXTERNAL_PAYLOAD	(1 << 6)
#define KSMBD_DCERPC_RETURN_READY	(1 << 7)

#define KSMBD_DCERPC_MAX_PREFERRED_SIZE -1

#define DCERPC_PTYPE_RPC_REQUEST	0x00
#define DCERPC_PTYPE_RPC_PING		0x01
#define DCERPC_PTYPE_RPC_RESPONSE	0x02
#define DCERPC_PTYPE_RPC_FAULT		0x03
#define DCERPC_PTYPE_RPC_WORKING	0x04
#define DCERPC_PTYPE_RPC_NOCALL		0x05
#define DCERPC_PTYPE_RPC_REJECT		0x06
#define DCERPC_PTYPE_RPC_ACK		0x07
#define DCERPC_PTYPE_RPC_CL_CANCEL	0x08
#define DCERPC_PTYPE_RPC_FACK		0x09
#define DCERPC_PTYPE_RPC_CANCEL_ACK	0x0A
#define DCERPC_PTYPE_RPC_BIND		0x0B
#define DCERPC_PTYPE_RPC_BINDACK	0x0C
#define DCERPC_PTYPE_RPC_BINDNACK	0x0D
#define DCERPC_PTYPE_RPC_ALTCONT	0x0E
#define DCERPC_PTYPE_RPC_ALTCONTRESP	0x0F
#define DCERPC_PTYPE_RPC_AUTH3		0x10
#define DCERPC_PTYPE_RPC_SHUTDOWN	0x11
#define DCERPC_PTYPE_RPC_CO_CANCEL	0x12
#define DCERPC_PTYPE_RPC_ORPHANED	0x13

/* First fragment */
#define DCERPC_PFC_FIRST_FRAG		0x01
/* Last fragment */
#define DCERPC_PFC_LAST_FRAG		0x02
/* Cancel was pending at sender */
#define DCERPC_PFC_PENDING_CANCEL	0x04
#define DCERPC_PFC_RESERVED_1		0x08
/* Supports concurrent multiplexing of a single connection. */
#define DCERPC_PFC_CONC_MPX		0x10

/*
 * Only meaningful on `fault' packet; if true, guaranteed
 * call did not execute.
 */
#define DCERPC_PFC_DID_NOT_EXECUTE	0x20
/* `maybe' call semantics requested */
#define DCERPC_PFC_MAYBE		0x40
/*
 * If true, a non-nil object UUID was specified in the handle, and
 * is present in the optional object field. If false, the object field
 * is omitted.
 */
#define DCERPC_PFC_OBJECT_UUID		0x80

#define DCERPC_SERIALIZATION_TYPE1		1
#define DCERPC_SERIALIZATION_TYPE2		2
#define DCERPC_SERIALIZATION_LITTLE_ENDIAN	0x10
#define DCERPC_SERIALIZATION_BIG_ENDIAN		0x00

struct dcerpc_header {
	/* start 8-octet aligned */

	/* common fields */
	__u8	rpc_vers;            /* 00:01 RPC version */
	__u8	rpc_vers_minor;      /* 01:01 minor version */
	__u8	ptype;               /* 02:01 bind PDU */
	__u8	pfc_flags;           /* 03:01 flags */
	__s8	packed_drep[4];      /* 04:04 NDR data rep format label*/
	__u16	frag_length;         /* 08:02 total length of fragment */
	__u16	auth_length;         /* 10:02 length of auth_value */
	__u32	call_id;             /* 12:04 call identifier */

	/* end common fields */
};

struct dcerpc_request_header {
	__u32	alloc_hint;
	__u16	context_id;
	__u16	opnum;
	/*
	 * SWITCH dcerpc_object object;
	 * PAYLOAD_BLOB;
	 */
};

struct dcerpc_response_header {
	__u32	alloc_hint;
	__u16	context_id;
	__u8	cancel_count;
};

/*
 * http://pubs.opengroup.org/onlinepubs/9629399/chap14.htm
 *
 * We refer to pointers that are parameters in remote procedure calls as
 * top-level pointers and we refer to pointers that are elements of arrays,
 * members of structures, or members of unions as embedded pointers.
 *
 *  NDR represents a null full pointer as an unsigned long integer with the
 *  value 0 (zero).
 *  NDR represents the first instance in a octet stream of a non-null full
 *  pointer in two parts: the first part is a non-zero unsigned long integer
 *  that identifies the referent; the second part is the representation of
 *  the referent. NDR represents subsequent instances in the same octet
 *  stream of the same pointer only by the referent identifier.
 */
struct ndr_ptr {
	__u32	ptr;
};

struct ndr_uniq_ptr {
	__u32	ref_id;
	__u32	ptr;
};

struct ndr_char_ptr {
	char	*ptr;
};

struct ndr_uniq_char_ptr {
	__u32	ref_id;
	char	*ptr;
};

#define STR_VAL(x)	((x).ptr)

struct srvsvc_share_info_request {
	int				level;
	size_t				max_size;

	struct ndr_uniq_char_ptr	server_name;
	struct ndr_char_ptr		share_name;

	struct ndr_uniq_ptr		payload_handle;
};

struct wkssvc_netwksta_info_request {
	struct ndr_uniq_char_ptr	server_name;
	int				level;
};

struct samr_info_request {
	int				level;
	int				client_version;
	struct ndr_uniq_char_ptr	name;
	unsigned char handle[20];
	unsigned int rid;
};

struct lsarpc_info_request {
	unsigned char handle[20];
	unsigned int level;
};

struct dcerpc_guid {
	__u32		time_low;
	__u16		time_mid;
	__u16		time_hi_and_version;
	__u8		clock_seq[2];
	__u8		node[6];
};

struct dcerpc_syntax {
	struct dcerpc_guid	uuid;
	__u16			ver_major;
	__u16			ver_minor;
};

struct dcerpc_context {
	__u16			id;
	__u8			num_syntaxes;
	struct dcerpc_syntax	abstract_syntax;
	struct dcerpc_syntax    *transfer_syntaxes;
};

struct dcerpc_bind_request {
	__u32			flags;
	__u16			max_xmit_frag_sz;
	__u16			max_recv_frag_sz;
	__u32			assoc_group_id;
	__u8			num_contexts;
	struct dcerpc_context	*list;
};

enum DCERPC_BIND_ACK_RESULT {
	DCERPC_BIND_ACK_RES_ACCEPT				= 0,
	DCERPC_BIND_ACK_RES_USER_REJECT,
	DCERPC_BIND_ACK_RES_PROVIDER_REJECT,
	DCERPC_BIND_ACK_RES_NEGOTIATE_ACK
};

enum DCERPC_BIND_ACK_REASON {
	DCERPC_BIND_ACK_RSN_NOT_SPECIFIED			= 0,
	DCERPC_BIND_ACK_RSN_ABSTRACT_SYNTAX_NOT_SUPPORTED,
	DCERPC_BIND_ACK_RSN_TRANSFER_SYNTAXES_NOT_SUPPORTED,
	DCERPC_BIND_ACK_RSN_LOCAL_LIMIT_EXCEEDED
};

enum DCERPC_BIND_NAK_REASON {
	DCERPC_BIND_NAK_RSN_NOT_SPECIFIED			= 0,
	DCERPC_BIND_NAK_RSN_TEMPORARY_CONGESTION		= 1,
	DCERPC_BIND_NAK_RSN_LOCAL_LIMIT_EXCEEDED		= 2,
	DCERPC_BIND_NAK_RSN_PROTOCOL_VERSION_NOT_SUPPORTED	= 4,
	DCERPC_BIND_NAK_RSN_INVALID_AUTH_TYPE		= 8,
	DCERPC_BIND_NAK_RSN_INVALID_CHECKSUM			= 9
};

enum DCERPC_BIND_TIME_OPTIONS {
	DCERPC_BIND_TIME_OPT_SEC_CONTEXT_MULTIPLEXING	= 0x0001,
	DCERPC_BIND_TIME_OPT_KEEP_CONNECTION_ON_ORPHAN	= 0x0002,
};

/*
 * So how this is expected to work. First, you need to obtain a snapshot
 * of the data that you want to push to the wire. The data snapshot goes
 * to ksmbd_rpc_pipe. Then you perform a protocol specific transformation
 * of the data snapshot. The transformed data goes to a specific protocol
 * dependent structure, e.g. ksmbd_dcerpc for DCERPC (ndr/ndr64). Then you
 * write the transformed data snapshot to the wire.
 */

struct ksmbd_rpc_command;

struct ksmbd_dcerpc {
	unsigned int		flags;
	size_t			offset;
	size_t			payload_sz;
	char			*payload;
	int			num_pointers;

	union {
		struct dcerpc_header			hdr;
	};
	union {
		struct dcerpc_request_header		req_hdr;
		struct dcerpc_response_header		resp_hdr;
	};
	union {
		struct srvsvc_share_info_request	si_req;
		struct dcerpc_bind_request		bi_req;
		struct wkssvc_netwksta_info_request	wi_req;
		struct samr_info_request		sm_req;
		struct lsarpc_info_request		lr_req;
	};

	struct ksmbd_rpc_command	*rpc_req;
	struct ksmbd_rpc_command	*rpc_resp;

	/*
	 * Find out the estimated entry size under the given container level
	 * restriction
	 */
	int			(*entry_size)(struct ksmbd_dcerpc *dce,
					      gpointer entry);
	/*
	 * Entry representation under the given container level
	 * restriction for array representation
	 */
	int			(*entry_rep)(struct ksmbd_dcerpc *dce,
					      gpointer entry);
	/*
	 * Entry data under the given container level restriction
	 * for array representation
	 */
	int			(*entry_data)(struct ksmbd_dcerpc *dce,
					      gpointer entry);
};

struct ksmbd_rpc_pipe {
	unsigned int		id;

	int			num_entries;
	int			num_processed;
	GPtrArray		*entries;

	struct ksmbd_dcerpc	*dce;

	/*
	 * Tell pipe that we processed the entry and won't need it
	 * anymore so it can remove/drop it.
	 */
	int			(*entry_processed)(struct ksmbd_rpc_pipe *pipe,
						   int i);
};

int ndr_read_int8(struct ksmbd_dcerpc *dce, __u8 *value);
int ndr_read_int16(struct ksmbd_dcerpc *dce, __u16 *value);
int ndr_read_int32(struct ksmbd_dcerpc *dce, __u32 *value);
int ndr_read_int64(struct ksmbd_dcerpc *dce, __u64 *value);

int ndr_write_int8(struct ksmbd_dcerpc *dce, __u8 value);
int ndr_write_int16(struct ksmbd_dcerpc *dce, __u16 value);
int ndr_write_int32(struct ksmbd_dcerpc *dce, __u32 value);
int ndr_write_int64(struct ksmbd_dcerpc *dce, __u64 value);

int ndr_write_union_int16(struct ksmbd_dcerpc *dce, __u16 value);
int ndr_write_union_int32(struct ksmbd_dcerpc *dce, __u32 value);
int ndr_read_union_int32(struct ksmbd_dcerpc *dce, __u32 *value);

int ndr_write_bytes(struct ksmbd_dcerpc *dce, void *value, size_t sz);
int ndr_read_bytes(struct ksmbd_dcerpc *dce, void *value, size_t sz);
int ndr_write_vstring(struct ksmbd_dcerpc *dce, void *value);
int ndr_write_string(struct ksmbd_dcerpc *dce, char *str);
int ndr_write_lsa_string(struct ksmbd_dcerpc *dce, char *str);
char *ndr_read_vstring(struct ksmbd_dcerpc *dce);
int ndr_read_vstring_ptr(struct ksmbd_dcerpc *dce, struct ndr_char_ptr *ctr);
int ndr_read_uniq_vstring_ptr(struct ksmbd_dcerpc *dce,
			      struct ndr_uniq_char_ptr *ctr);
void ndr_free_vstring_ptr(struct ndr_char_ptr *ctr);
void ndr_free_uniq_vstring_ptr(struct ndr_uniq_char_ptr *ctr);
int ndr_read_ptr(struct ksmbd_dcerpc *dce, struct ndr_ptr *ctr);
int ndr_read_uniq_ptr(struct ksmbd_dcerpc *dce, struct ndr_uniq_ptr *ctr);
int __ndr_write_array_of_structs(struct ksmbd_rpc_pipe *pipe, int max_entry_nr);
int ndr_write_array_of_structs(struct ksmbd_rpc_pipe *pipe);

int dcerpc_write_headers(struct ksmbd_dcerpc *dce, int method_status);

void dcerpc_set_ext_payload(struct ksmbd_dcerpc *dce,
			    void *payload,
			    size_t sz);
void rpc_pipe_reset(struct ksmbd_rpc_pipe *pipe);

void rpc_init(void);
void rpc_destroy(void);

int rpc_restricted_context(struct ksmbd_rpc_command *req);

int rpc_ioctl_request(struct ksmbd_rpc_command *req,
		      struct ksmbd_rpc_command *resp,
		      int max_resp_sz);
int rpc_read_request(struct ksmbd_rpc_command *req,
		     struct ksmbd_rpc_command *resp,
		     int max_resp_sz);
int rpc_write_request(struct ksmbd_rpc_command *req,
		      struct ksmbd_rpc_command *resp);
int rpc_open_request(struct ksmbd_rpc_command *req,
		     struct ksmbd_rpc_command *resp);
int rpc_close_request(struct ksmbd_rpc_command *req,
		      struct ksmbd_rpc_command *resp);
void auto_align_offset(struct ksmbd_dcerpc *dce);
#endif /* __KSMBD_RPC_H__ */
