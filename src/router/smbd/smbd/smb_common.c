// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 *   Copyright (C) 2018 Namjae Jeon <linkinjeon@gmail.com>
 */

#include "smb_common.h"
#ifdef CONFIG_SMB_INSECURE_SERVER
#include "smb1pdu.h"
#endif
#include "server.h"
#include "misc.h"
#include "smbstatus.h"
/* @FIXME */
#include "connection.h"
#include "smbd_work.h"

/*for shortname implementation */
static const char basechars[43] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ_-!@#$%";
#define MANGLE_BASE       (sizeof(basechars)/sizeof(char)-1)
#define MAGIC_CHAR '~'
#define PERIOD '.'
#define mangle(V) ((char)(basechars[(V) % MANGLE_BASE]))

#ifdef CONFIG_SMB_INSECURE_SERVER
#define SMBD_MIN_SUPPORTED_HEADER_SIZE	(sizeof(struct smb_hdr))
#else
#define SMBD_MIN_SUPPORTED_HEADER_SIZE	(sizeof(struct smb2_hdr))
#endif

LIST_HEAD(global_lock_list);

struct smb_protocol {
	int		index;
	char		*name;
	char		*prot;
	__u16		prot_id;
};

static struct smb_protocol smb_protos[] = {
#ifdef CONFIG_SMB_INSECURE_SERVER
	{
		SMB1_PROT,
		"\2NT LM 0.12",
		"NT1",
		SMB10_PROT_ID
	},
	{
		SMB2_PROT,
		"\2SMB 2.002",
		"SMB2_02",
		SMB20_PROT_ID
	},
#endif
	{
		SMB21_PROT,
		"\2SMB 2.1",
		"SMB2_10",
		SMB21_PROT_ID
	},
	{
		SMB2X_PROT,
		"\2SMB 2.???",
		"SMB2_22",
		SMB2X_PROT_ID
	},
	{
		SMB30_PROT,
		"\2SMB 3.0",
		"SMB3_00",
		SMB30_PROT_ID
	},
	{
		SMB302_PROT,
		"\2SMB 3.02",
		"SMB3_02",
		SMB302_PROT_ID
	},
	{
		SMB311_PROT,
		"\2SMB 3.1.1",
		"SMB3_11",
		SMB311_PROT_ID
	},
};

unsigned int smbd_small_buffer_size(void)
{
	return 448;
}

unsigned int smbd_server_side_copy_max_chunk_count(void)
{
	return 256;
}

unsigned int smbd_server_side_copy_max_chunk_size(void)
{
	return (2U << 30) - 1;
}

unsigned int smbd_server_side_copy_max_total_size(void)
{
	return (2U << 30) - 1;
}

inline int smbd_min_protocol(void)
{
#ifdef CONFIG_SMB_INSECURE_SERVER
	return SMB1_PROT;
#else
	return SMB2_PROT;
#endif
}

inline int smbd_max_protocol(void)
{
	return SMB311_PROT;
}

int smbd_lookup_protocol_idx(char *str)
{
	int offt = ARRAY_SIZE(smb_protos) - 1;
	int len = strlen(str);

	while (offt >= 0) {
		if (!strncmp(str, smb_protos[offt].prot, len)) {
			smbd_debug("selected %s dialect idx = %d\n",
					smb_protos[offt].prot, offt);
			return smb_protos[offt].index;
		}
		offt--;
	}
	return -1;
}

/**
 * check_message() - check for valid smb2 request header
 * @buf:       smb2 header to be checked
 *
 * check for valid smb signature and packet direction(request/response)
 *
 * Return:      0 on success, otherwise 1
 */
int smbd_verify_smb_message(struct smbd_work *work)
{
	struct smb2_hdr *smb2_hdr = REQUEST_BUF(work);

#ifdef CONFIG_SMB_INSECURE_SERVER
	if (smb2_hdr->ProtocolId == SMB2_PROTO_NUMBER) {
		smbd_debug("got SMB2 command\n");
		return smbd_smb2_check_message(work);
	}

	return smbd_smb1_check_message(work);
#else
	if (smb2_hdr->ProtocolId == SMB2_PROTO_NUMBER)
		return smbd_smb2_check_message(work);

	return 0;
#endif
}

/**
 * is_smb_request() - check for valid smb request type
 * @conn:	connection instance
 * @type:	smb request type
 *
 * Return:      true on success, otherwise false
 */
bool smbd_smb_request(struct smbd_conn *conn)
{
	int type = *(char *)conn->request_buf;

	switch (type) {
	case RFC1002_SESSION_MESSAGE:
		/* Regular SMB request */
		return true;
	case RFC1002_SESSION_KEEP_ALIVE:
		smbd_debug("RFC 1002 session keep alive\n");
		break;
	default:
		smbd_debug("RFC 1002 unknown request type 0x%x\n", type);
	}

	return false;
}

static bool supported_protocol(int idx)
{
	return (server_conf.min_protocol <= idx &&
			idx <= server_conf.max_protocol);
}

static char *next_dialect(char *dialect, int *next_off)
{
	dialect = dialect + *next_off;
	*next_off = strlen(dialect);
	return dialect;
}

static int smbd_lookup_dialect_by_name(char *cli_dialects, __le16 byte_count)
{
	int i, seq_num, bcount, next;
	char *dialect;

	for (i = ARRAY_SIZE(smb_protos) - 1; i >= 0; i--) {
		seq_num = 0;
		next = 0;
		dialect = cli_dialects;
		bcount = le16_to_cpu(byte_count);
		do {
			dialect = next_dialect(dialect, &next);
			smbd_debug("client requested dialect %s\n", dialect);
			if (!strcmp(dialect, smb_protos[i].name)) {
				if (supported_protocol(smb_protos[i].index)) {
					smbd_debug("selected %s dialect\n",
							smb_protos[i].name);
					if (smb_protos[i].index == SMB1_PROT)
						return seq_num;
					return smb_protos[i].prot_id;
				}
			}
			seq_num++;
			bcount -= (++next);
		} while (bcount > 0);
	}

	return BAD_PROT_ID;
}

int smbd_lookup_dialect_by_id(__le16 *cli_dialects, __le16 dialects_count)
{
	int i;
	int count;

	for (i = ARRAY_SIZE(smb_protos) - 1; i >= 0; i--) {
		count = le16_to_cpu(dialects_count);
		while (--count >= 0) {
			smbd_debug("client requested dialect 0x%x\n",
				le16_to_cpu(cli_dialects[count]));
			if (le16_to_cpu(cli_dialects[count]) !=
					smb_protos[i].prot_id)
				continue;

			if (supported_protocol(smb_protos[i].index)) {
				smbd_debug("selected %s dialect\n",
					smb_protos[i].name);
				return smb_protos[i].prot_id;
			}
		}
	}

	return BAD_PROT_ID;
}

int smbd_negotiate_smb_dialect(void *buf)
{
	__le32 proto;

	proto = ((struct smb2_hdr *)buf)->ProtocolId;
	if (proto == SMB2_PROTO_NUMBER) {
		struct smb2_negotiate_req *req;

		req = (struct smb2_negotiate_req *)buf;
		return smbd_lookup_dialect_by_id(req->Dialects,
						  req->DialectCount);
	}

	proto = *(__le32 *)((struct smb_hdr *)buf)->Protocol;
	if (proto == SMB1_PROTO_NUMBER) {
		struct smb_negotiate_req *req;

		req = (struct smb_negotiate_req *)buf;
		return smbd_lookup_dialect_by_name(req->DialectsArray,
						    req->ByteCount);
	}

	return BAD_PROT_ID;
}

void smbd_init_smb2_server_common(struct smbd_conn *conn)
{
	if (init_smb2_0_server(conn) == -ENOTSUPP)
		init_smb2_1_server(conn);
}

#define SMB_COM_NEGOTIATE	0x72
int smbd_init_smb_server(struct smbd_work *work)
{
	struct smbd_conn *conn = work->conn;
	void *buf = REQUEST_BUF(work);
	__le32 proto;

	if (conn->need_neg == false)
		return 0;

	proto = *(__le32 *)((struct smb_hdr *)buf)->Protocol;
#ifdef CONFIG_SMB_INSECURE_SERVER
	if (proto == SMB1_PROTO_NUMBER)
		init_smb1_server(conn);
	else
		smbd_init_smb2_server_common(conn);
#else
	smbd_init_smb2_server_common(conn);
#endif

	if (conn->ops->get_cmd_val(work) != SMB_COM_NEGOTIATE)
		conn->need_neg = false;
	return 0;
}

bool smbd_pdu_size_has_room(unsigned int pdu)
{
	return (pdu >= SMBD_MIN_SUPPORTED_HEADER_SIZE - 4);
}

int smbd_populate_dot_dotdot_entries(struct smbd_conn *conn,
				      int info_level,
				      struct smbd_file *dir,
				      struct smbd_dir_info *d_info,
				      char *search_pattern,
				      int (*fn)(struct smbd_conn *,
						int,
						struct smbd_dir_info *,
						struct smbd_kstat *))
{
	int i, rc = 0;

	for (i = 0; i < 2; i++) {
		struct kstat kstat;
		struct smbd_kstat smbd_kstat;

		if (!dir->dot_dotdot[i]) { /* fill dot entry info */
			if (i == 0) {
				d_info->name = ".";
				d_info->name_len = 1;
			} else {
				d_info->name = "..";
				d_info->name_len = 2;
			}

			if (!match_pattern(d_info->name, search_pattern)) {
				dir->dot_dotdot[i] = 1;
				continue;
			}

			generic_fillattr(PARENT_INODE(dir), &kstat);
			smbd_kstat.file_attributes = ATTR_DIRECTORY_LE;
			smbd_kstat.kstat = &kstat;
			rc = fn(conn, info_level, d_info, &smbd_kstat);
			if (rc)
				break;
			if (d_info->out_buf_len <= 0)
				break;

			dir->dot_dotdot[i] = 1;
		}
	}

	return rc;
}

/**
 * smbd_extract_shortname() - get shortname from long filename
 * @conn:	connection instance
 * @longname:	source long filename
 * @shortname:	destination short filename
 *
 * Return:	shortname length or 0 when source long name is '.' or '..'
 * TODO: Though this function comforms the restriction of 8.3 Filename spec,
 * but the result is different with Windows 7's one. need to check.
 */
int smbd_extract_shortname(struct smbd_conn *conn,
			    const char *longname,
			    char *shortname)
{
	const char *p;
	char base[9], extension[4];
	char out[13] = {0};
	int baselen = 0;
	int extlen = 0, len = 0;
	unsigned int csum = 0;
	const unsigned char *ptr;
	bool dot_present = true;

	p = longname;
	if ((*p == '.') || (!(strcmp(p, "..")))) {
		/*no mangling required */
		return 0;
	}

	p = strrchr(longname, '.');
	if (p == longname) { /*name starts with a dot*/
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 3, 0)
		strcpy(extension, "___");
		extension[3] = '\0';
#else
		strscpy(extension, "___", strlen("___"));
#endif
	} else {
		if (p != NULL) {
			p++;
			while (*p && extlen < 3) {
				if (*p != '.')
					extension[extlen++] = toupper(*p);
				p++;
			}
			extension[extlen] = '\0';
		} else
			dot_present = false;
	}

	p = longname;
	if (*p == '.') {
		p++;
		longname++;
	}
	while (*p && (baselen < 5)) {
		if (*p != '.')
			base[baselen++] = toupper(*p);
		p++;
	}

	base[baselen] = MAGIC_CHAR;
	memcpy(out, base, baselen+1);

	ptr = longname;
	len = strlen(longname);
	for (; len > 0; len--, ptr++)
		csum += *ptr;

	csum = csum % (MANGLE_BASE * MANGLE_BASE);
	out[baselen+1] = mangle(csum/MANGLE_BASE);
	out[baselen+2] = mangle(csum);
	out[baselen+3] = PERIOD;

	if (dot_present)
		memcpy(&out[baselen+4], extension, 4);
	else
		out[baselen+4] = '\0';
	smbConvertToUTF16((__le16 *)shortname, out, PATH_MAX,
			conn->local_nls, 0);
	len = strlen(out) * 2;
	return len;
}

static int __smb2_negotiate(struct smbd_conn *conn)
{
	return (conn->dialect >= SMB20_PROT_ID &&
			conn->dialect <= SMB311_PROT_ID);
}

#ifndef CONFIG_SMB_INSECURE_SERVER
int smb_handle_negotiate(struct smbd_work *work)
{
	struct smb_negotiate_rsp *neg_rsp = RESPONSE_BUF(work);

	smbd_err("Unsupported SMB protocol\n");
	neg_rsp->hdr.Status.CifsError = STATUS_INVALID_LOGON_TYPE;
	return -EINVAL;
}
#endif

int smbd_smb_negotiate_common(struct smbd_work *work, unsigned int command)
{
	struct smbd_conn *conn = work->conn;
	int ret;

	conn->dialect = smbd_negotiate_smb_dialect(REQUEST_BUF(work));
	smbd_debug("conn->dialect 0x%x\n", conn->dialect);

	if (command == SMB2_NEGOTIATE_HE) {
		struct smb2_hdr *smb2_hdr = REQUEST_BUF(work);

		if (smb2_hdr->ProtocolId != SMB2_PROTO_NUMBER) {
			smbd_debug("Downgrade to SMB1 negotiation\n");
			command = SMB_COM_NEGOTIATE;
		}
	}

	if (command == SMB2_NEGOTIATE_HE) {
		ret = smb2_handle_negotiate(work);
		init_smb2_neg_rsp(work);
		return ret;
	}

	if (command == SMB_COM_NEGOTIATE) {
		if (__smb2_negotiate(conn)) {
			conn->need_neg = true;
			smbd_init_smb2_server_common(conn);
			init_smb2_neg_rsp(work);
			smbd_debug("Upgrade to SMB2 negotiation\n");
			return 0;
		}
		return smb_handle_negotiate(work);
	}

	smbd_err("Unknown SMB negotiation command: %u\n", command);
	return -EINVAL;
}

enum SHARED_MODE_ERRORS {
	SHARE_DELETE_ERROR,
	SHARE_READ_ERROR,
	SHARE_WRITE_ERROR,
	FILE_READ_ERROR,
	FILE_WRITE_ERROR,
	FILE_DELETE_ERROR,
};

static const char * const shared_mode_errors[] = {
	"Current access mode does not permit SHARE_DELETE",
	"Current access mode does not permit SHARE_READ",
	"Current access mode does not permit SHARE_WRITE",
	"Desired access mode does not permit FILE_READ",
	"Desired access mode does not permit FILE_WRITE",
	"Desired access mode does not permit FILE_DELETE",
};

static void smb_shared_mode_error(int error,
				  struct smbd_file *prev_fp,
				  struct smbd_file *curr_fp)
{
	smbd_debug("%s\n", shared_mode_errors[error]);
	smbd_debug("Current mode: 0x%x Desired mode: 0x%x\n",
		  prev_fp->saccess, curr_fp->daccess);
}

int smbd_smb_check_shared_mode(struct file *filp, struct smbd_file *curr_fp)
{
	int rc = 0;
	struct smbd_file *prev_fp;
	struct list_head *cur;

	/*
	 * Lookup fp in master fp list, and check desired access and
	 * shared mode between previous open and current open.
	 */
	read_lock(&curr_fp->f_ci->m_lock);
	list_for_each(cur, &curr_fp->f_ci->m_fp_list) {
		prev_fp = list_entry(cur, struct smbd_file, node);
		if (file_inode(filp) != FP_INODE(prev_fp))
			continue;

		if (filp == prev_fp->filp)
			continue;

		if (smbd_stream_fd(prev_fp) && smbd_stream_fd(curr_fp))
			if (strcmp(prev_fp->stream.name, curr_fp->stream.name))
				continue;

		if (prev_fp->is_durable) {
			prev_fp->is_durable = 0;
			continue;
		}

		if (prev_fp->attrib_only != curr_fp->attrib_only)
			continue;

		if (!(prev_fp->saccess & (FILE_SHARE_DELETE_LE)) &&
				curr_fp->daccess & (FILE_DELETE_LE |
					FILE_GENERIC_ALL_LE |
					FILE_MAXIMAL_ACCESS_LE)) {
			smb_shared_mode_error(SHARE_DELETE_ERROR,
					      prev_fp,
					      curr_fp);
			rc = -EPERM;
			break;
		}

		/*
		 * Only check FILE_SHARE_DELETE if stream opened and
		 * normal file opened.
		 */
		if (smbd_stream_fd(prev_fp) && !smbd_stream_fd(curr_fp))
			continue;

		if (!(prev_fp->saccess & (FILE_SHARE_READ_LE)) &&
				curr_fp->daccess & (FILE_EXECUTE_LE |
					FILE_READ_DATA_LE |
					FILE_GENERIC_READ_LE |
					FILE_GENERIC_ALL_LE |
					FILE_MAXIMAL_ACCESS_LE)) {
			smb_shared_mode_error(SHARE_READ_ERROR,
					      prev_fp,
					      curr_fp);
			rc = -EPERM;
			break;
		}

		if (!(prev_fp->saccess & (FILE_SHARE_WRITE_LE)) &&
				curr_fp->daccess & (FILE_WRITE_DATA_LE |
					FILE_APPEND_DATA_LE |
					FILE_GENERIC_WRITE_LE |
					FILE_GENERIC_ALL_LE |
					FILE_MAXIMAL_ACCESS_LE)) {
			smb_shared_mode_error(SHARE_WRITE_ERROR,
					      prev_fp,
					      curr_fp);
			rc = -EPERM;
			break;
		}

		if (prev_fp->daccess & (FILE_EXECUTE_LE |
					FILE_READ_DATA_LE |
					FILE_GENERIC_READ_LE |
					FILE_GENERIC_ALL_LE |
					FILE_MAXIMAL_ACCESS_LE) &&
				!(curr_fp->saccess & FILE_SHARE_READ_LE)) {
			smb_shared_mode_error(FILE_READ_ERROR,
					      prev_fp,
					      curr_fp);
			rc = -EPERM;
			break;
		}

		if (prev_fp->daccess & (FILE_WRITE_DATA_LE |
					FILE_APPEND_DATA_LE |
					FILE_GENERIC_WRITE_LE |
					FILE_GENERIC_ALL_LE |
					FILE_MAXIMAL_ACCESS_LE) &&
				!(curr_fp->saccess & FILE_SHARE_WRITE_LE)) {
			smb_shared_mode_error(FILE_WRITE_ERROR,
					      prev_fp,
					      curr_fp);
			rc = -EPERM;
			break;
		}

		if (prev_fp->daccess & (FILE_DELETE_LE |
					FILE_GENERIC_ALL_LE |
					FILE_MAXIMAL_ACCESS_LE) &&
				!(curr_fp->saccess & FILE_SHARE_DELETE_LE)) {
			smb_shared_mode_error(FILE_DELETE_ERROR,
					      prev_fp,
					      curr_fp);
			rc = -EPERM;
			break;
		}
	}
	read_unlock(&curr_fp->f_ci->m_lock);

	return rc;
}

bool is_asterisk(char *p)
{
	return p && p[0] == '*';
}
