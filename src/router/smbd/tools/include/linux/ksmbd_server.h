/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *
 *   linux-usmbd-devel@lists.sourceforge.net
 */

#ifndef _LINUX_USMBD_SERVER_H
#define _LINUX_USMBD_SERVER_H

#include <linux/types.h>

#define USMBD_GENL_NAME		"SMBD_GENL"
#define USMBD_GENL_VERSION		0x01

#ifndef ____usmbd_align
#define ____usmbd_align		__attribute__((__aligned__(4)))
#endif

#define USMBD_REQ_MAX_ACCOUNT_NAME_SZ	48
#define USMBD_REQ_MAX_HASH_SZ		18
#define USMBD_REQ_MAX_SHARE_NAME	64

struct usmbd_heartbeat {
	__u32	handle;
} ____usmbd_align;

/*
 * Global config flags.
 */
#define USMBD_GLOBAL_FLAG_INVALID		(0)
#define USMBD_GLOBAL_FLAG_SMB2_LEASES		(1 << 0)
#define USMBD_GLOBAL_FLAG_CACHE_TBUF		(1 << 1)
#define USMBD_GLOBAL_FLAG_CACHE_RBUF		(1 << 2)
#define USMBD_GLOBAL_FLAG_SMB3_ENCRYPTION	(1 << 3)
#define USMBD_GLOBAL_FLAG_DURABLE_HANDLE	(1 << 4)

struct usmbd_startup_request {
	__u32	flags;
	__s32	signing;
	__s8	min_prot[16];
	__s8	max_prot[16];
	__s8	netbios_name[16];
	__s8	work_group[64];
	__s8	server_string[64];
	__u16	tcp_port;
	__u16	ipc_timeout;
	__u32	deadtime;
	__u32	file_max;
	__u32	smb2_max_write;
	__u32	smb2_max_read;
	__u32	smb2_max_trans;
	__u32	ifc_list_sz;
	__s8	____payload[0];
} ____usmbd_align;

#define USMBD_STARTUP_CONFIG_INTERFACES(s)	((s)->____payload)

struct usmbd_shutdown_request {
	__s32	reserved;
} ____usmbd_align;

struct usmbd_login_request {
	__u32	handle;
	__s8	account[USMBD_REQ_MAX_ACCOUNT_NAME_SZ];
} ____usmbd_align;

struct usmbd_login_response {
	__u32	handle;
	__u32	gid;
	__u32	uid;
	__s8	account[USMBD_REQ_MAX_ACCOUNT_NAME_SZ];
	__u16	status;
	__u16	hash_sz;
	__s8	hash[USMBD_REQ_MAX_HASH_SZ];
} ____usmbd_align;

struct usmbd_share_config_request {
	__u32	handle;
	__s8	share_name[USMBD_REQ_MAX_SHARE_NAME];
} ____usmbd_align;

struct usmbd_share_config_response {
	__u32	handle;
	__u32	flags;
	__u16	create_mask;
	__u16	directory_mask;
	__u16	force_create_mode;
	__u16	force_directory_mode;
	__u16	force_uid;
	__u16	force_gid;
	__u32	veto_list_sz;
	__s8	____payload[0];
} ____usmbd_align;

#define USMBD_SHARE_CONFIG_VETO_LIST(s)	((s)->____payload)
#define USMBD_SHARE_CONFIG_PATH(s)				\
	({							\
		char *p = (s)->____payload;			\
		if ((s)->veto_list_sz)				\
			p += (s)->veto_list_sz + 1;		\
		p;						\
	 })

struct usmbd_tree_connect_request {
	__u32	handle;
	__u16	account_flags;
	__u16	flags;
	__u64	session_id;
	__u64	connect_id;
	__s8	account[USMBD_REQ_MAX_ACCOUNT_NAME_SZ];
	__s8	share[USMBD_REQ_MAX_SHARE_NAME];
	__s8	peer_addr[64];
} ____usmbd_align;

struct usmbd_tree_connect_response {
	__u32	handle;
	__u16	status;
	__u16	connection_flags;
} ____usmbd_align;

struct usmbd_tree_disconnect_request {
	__u64	session_id;
	__u64	connect_id;
} ____usmbd_align;

struct usmbd_logout_request {
	__s8	account[USMBD_REQ_MAX_ACCOUNT_NAME_SZ];
} ____usmbd_align;

struct usmbd_rpc_command {
	__u32	handle;
	__u32	flags;
	__u32	payload_sz;
	__u8	payload[0];
} ____usmbd_align;

/*
 * This also used as NETLINK attribute type value.
 *
 * NOTE:
 * Response message type value should be equal to
 * request message type value + 1.
 */
enum usmbd_event {
	USMBD_EVENT_UNSPEC			= 0,
	USMBD_EVENT_HEARTBEAT_REQUEST,

	USMBD_EVENT_STARTING_UP,
	USMBD_EVENT_SHUTTING_DOWN,

	USMBD_EVENT_LOGIN_REQUEST,
	USMBD_EVENT_LOGIN_RESPONSE		= 5,

	USMBD_EVENT_SHARE_CONFIG_REQUEST,
	USMBD_EVENT_SHARE_CONFIG_RESPONSE,

	USMBD_EVENT_TREE_CONNECT_REQUEST,
	USMBD_EVENT_TREE_CONNECT_RESPONSE,

	USMBD_EVENT_TREE_DISCONNECT_REQUEST	= 10,

	USMBD_EVENT_LOGOUT_REQUEST,

	USMBD_EVENT_RPC_REQUEST,
	USMBD_EVENT_RPC_RESPONSE,

	USMBD_EVENT_MAX
};

enum USMBD_TREE_CONN_STATUS {
	USMBD_TREE_CONN_STATUS_OK		= 0,
	USMBD_TREE_CONN_STATUS_NOMEM,
	USMBD_TREE_CONN_STATUS_NO_SHARE,
	USMBD_TREE_CONN_STATUS_NO_USER,
	USMBD_TREE_CONN_STATUS_INVALID_USER,
	USMBD_TREE_CONN_STATUS_HOST_DENIED	= 5,
	USMBD_TREE_CONN_STATUS_CONN_EXIST,
	USMBD_TREE_CONN_STATUS_TOO_MANY_CONNS,
	USMBD_TREE_CONN_STATUS_TOO_MANY_SESSIONS,
	USMBD_TREE_CONN_STATUS_ERROR,
};

/*
 * User config flags.
 */
#define USMBD_USER_FLAG_INVALID		(0)
#define USMBD_USER_FLAG_OK		(1 << 0)
#define USMBD_USER_FLAG_BAD_PASSWORD	(1 << 1)
#define USMBD_USER_FLAG_BAD_UID		(1 << 2)
#define USMBD_USER_FLAG_BAD_USER	(1 << 3)
#define USMBD_USER_FLAG_GUEST_ACCOUNT	(1 << 4)

/*
 * Share config flags.
 */
#define USMBD_SHARE_FLAG_INVALID		(0)
#define USMBD_SHARE_FLAG_AVAILABLE		(1 << 0)
#define USMBD_SHARE_FLAG_BROWSEABLE		(1 << 1)
#define USMBD_SHARE_FLAG_WRITEABLE		(1 << 2)
#define USMBD_SHARE_FLAG_READONLY		(1 << 3)
#define USMBD_SHARE_FLAG_GUEST_OK		(1 << 4)
#define USMBD_SHARE_FLAG_GUEST_ONLY		(1 << 5)
#define USMBD_SHARE_FLAG_STORE_DOS_ATTRS	(1 << 6)
#define USMBD_SHARE_FLAG_OPLOCKS		(1 << 7)
#define USMBD_SHARE_FLAG_PIPE			(1 << 8)
#define USMBD_SHARE_FLAG_HIDE_DOT_FILES		(1 << 9)
#define USMBD_SHARE_FLAG_INHERIT_SMACK		(1 << 10)
#define USMBD_SHARE_FLAG_INHERIT_OWNER		(1 << 11)
#define USMBD_SHARE_FLAG_STREAMS		(1 << 12)

/*
 * Tree connect request flags.
 */
#define USMBD_TREE_CONN_FLAG_REQUEST_SMB1	(0)
#define USMBD_TREE_CONN_FLAG_REQUEST_IPV6	(1 << 0)
#define USMBD_TREE_CONN_FLAG_REQUEST_SMB2	(1 << 1)

/*
 * Tree connect flags.
 */
#define USMBD_TREE_CONN_FLAG_GUEST_ACCOUNT	(1 << 0)
#define USMBD_TREE_CONN_FLAG_READ_ONLY		(1 << 1)
#define USMBD_TREE_CONN_FLAG_WRITABLE		(1 << 2)
#define USMBD_TREE_CONN_FLAG_ADMIN_ACCOUNT	(1 << 3)

/*
 * RPC over IPC.
 */
#define USMBD_RPC_METHOD_RETURN		(1 << 0)
#define USMBD_RPC_SRVSVC_METHOD_INVOKE	(1 << 1)
#define USMBD_RPC_SRVSVC_METHOD_RETURN	((1 << 1) | USMBD_RPC_METHOD_RETURN)
#define USMBD_RPC_WKSSVC_METHOD_INVOKE	(1 << 2)
#define USMBD_RPC_WKSSVC_METHOD_RETURN	((1 << 2) | USMBD_RPC_METHOD_RETURN)
#define USMBD_RPC_IOCTL_METHOD		((1 << 3) | USMBD_RPC_METHOD_RETURN)
#define USMBD_RPC_OPEN_METHOD		(1 << 4)
#define USMBD_RPC_WRITE_METHOD		(1 << 5)
#define USMBD_RPC_READ_METHOD		((1 << 6) | USMBD_RPC_METHOD_RETURN)
#define USMBD_RPC_CLOSE_METHOD		(1 << 7)
#define USMBD_RPC_RAP_METHOD		((1 << 8) | USMBD_RPC_METHOD_RETURN)
#define USMBD_RPC_RESTRICTED_CONTEXT	(1 << 9)

#define USMBD_RPC_OK			0
#define USMBD_RPC_EBAD_FUNC		0x00000001
#define USMBD_RPC_EACCESS_DENIED	0x00000005
#define USMBD_RPC_EBAD_FID		0x00000006
#define USMBD_RPC_ENOMEM		0x00000008
#define USMBD_RPC_EBAD_DATA		0x0000000D
#define USMBD_RPC_ENOTIMPLEMENTED	0x00000040
#define USMBD_RPC_EINVALID_PARAMETER	0x00000057
#define USMBD_RPC_EMORE_DATA		0x000000EA
#define USMBD_RPC_EINVALID_LEVEL	0x0000007C

#define USMBD_CONFIG_OPT_DISABLED	0
#define USMBD_CONFIG_OPT_ENABLED	1
#define USMBD_CONFIG_OPT_AUTO		2
#define USMBD_CONFIG_OPT_MANDATORY	3

#endif /* _LINUX_USMBD_SERVER_H */
