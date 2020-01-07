// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2016 Namjae Jeon <linkinjeon@gmail.com>
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#include "glob.h"
#include "nterr.h"
#include "smb2pdu.h"
#include "smb_common.h"
#include "smbstatus.h"
#include "mgmt/user_session.h"
#include "connection.h"

static int check_smb2_hdr(struct smb2_hdr *hdr)
{
	/*
	 * Make sure that this really is an SMB, that it is a response.
	 */
	if (hdr->Flags & SMB2_FLAGS_SERVER_TO_REDIR)
		return 1;
	return 0;
}

/*
 *  The following table defines the expected "StructureSize" of SMB2 requests
 *  in order by SMB2 command.  This is similar to "wct" in SMB/CIFS requests.
 *
 *  Note that commands are defined in smb2pdu.h in le16 but the array below is
 *  indexed by command in host byte order
 */
static const __le16 smb2_req_struct_sizes[NUMBER_OF_SMB2_COMMANDS] = {
	/* SMB2_NEGOTIATE */ cpu_to_le16(36),
	/* SMB2_SESSION_SETUP */ cpu_to_le16(25),
	/* SMB2_LOGOFF */ cpu_to_le16(4),
	/* SMB2_TREE_CONNECT */ cpu_to_le16(9),
	/* SMB2_TREE_DISCONNECT */ cpu_to_le16(4),
	/* SMB2_CREATE */ cpu_to_le16(57),
	/* SMB2_CLOSE */ cpu_to_le16(24),
	/* SMB2_FLUSH */ cpu_to_le16(24),
	/* SMB2_READ */ cpu_to_le16(49),
	/* SMB2_WRITE */ cpu_to_le16(49),
	/* SMB2_LOCK */ cpu_to_le16(48),
	/* SMB2_IOCTL */ cpu_to_le16(57),
	/* SMB2_CANCEL */ cpu_to_le16(4),
	/* SMB2_ECHO */ cpu_to_le16(4),
	/* SMB2_QUERY_DIRECTORY */ cpu_to_le16(33),
	/* SMB2_CHANGE_NOTIFY */ cpu_to_le16(32),
	/* SMB2_QUERY_INFO */ cpu_to_le16(41),
	/* SMB2_SET_INFO */ cpu_to_le16(33),
	/* use 44 for lease break */
	/* SMB2_OPLOCK_BREAK */ cpu_to_le16(36)
};

/*
 * The size of the variable area depends on the offset and length fields
 * located in different fields for various SMB2 requests. SMB2 requests
 * with no variable length info, show an offset of zero for the offset field.
 */
static const bool has_smb2_data_area[NUMBER_OF_SMB2_COMMANDS] = {
	/* SMB2_NEGOTIATE */ true,
	/* SMB2_SESSION_SETUP */ true,
	/* SMB2_LOGOFF */ false,
	/* SMB2_TREE_CONNECT */	true,
	/* SMB2_TREE_DISCONNECT */ false,
	/* SMB2_CREATE */ true,
	/* SMB2_CLOSE */ false,
	/* SMB2_FLUSH */ false,
	/* SMB2_READ */	true,
	/* SMB2_WRITE */ true,
	/* SMB2_LOCK */	true,
	/* SMB2_IOCTL */ true,
	/* SMB2_CANCEL */ false, /* BB CHECK this not listed in documentation */
	/* SMB2_ECHO */ false,
	/* SMB2_QUERY_DIRECTORY */ true,
	/* SMB2_CHANGE_NOTIFY */ false,
	/* SMB2_QUERY_INFO */ true,
	/* SMB2_SET_INFO */ true,
	/* SMB2_OPLOCK_BREAK */ false
};

static int get_neg_context_size(char *buf, int *off, int *len)
{
	int i = 0;
	struct smb2_negotiate_req *req = (struct smb2_negotiate_req *)buf;
	char *pneg_ctxt;
	__le16 *ContextType;
	int neg_ctxt_cnt = le16_to_cpu(req->NegotiateContextCount);

	*off = le32_to_cpu(req->NegotiateContextOffset);
	if (*off == 0 || neg_ctxt_cnt == 0)
		return 0;

	pneg_ctxt = buf + le32_to_cpu(req->NegotiateContextOffset) + 4;
	ContextType = (__le16 *)pneg_ctxt;
	while (i++ < neg_ctxt_cnt) {
		if (*ContextType == SMB2_PREAUTH_INTEGRITY_CAPABILITIES) {
			pneg_ctxt +=
				sizeof(struct smb2_preauth_neg_context) + 2;
			*len += sizeof(struct smb2_preauth_neg_context) + 2;
			ContextType = (__le16 *)pneg_ctxt;
		} else if (*ContextType == SMB2_ENCRYPTION_CAPABILITIES) {
			pneg_ctxt +=
				sizeof(struct smb2_encryption_neg_context) + 2;
			*len += sizeof(struct smb2_encryption_neg_context) + 2;
			ContextType = (__le16 *)pneg_ctxt;
		}
	}
	*len -= 2;

	if (*len <= 0)
		return 0;
	return 1;
}

/*
 * Returns the pointer to the beginning of the data area. Length of the data
 * area and the offset to it (from the beginning of the smb are also returned.
 */
static char *smb2_get_data_area_len(int *off, int *len, struct smb2_hdr *hdr)
{
	*off = 0;
	*len = 0;

	/* error reqeusts do not have data area */
	if (hdr->Status && hdr->Status != STATUS_MORE_PROCESSING_REQUIRED &&
			(((struct smb2_err_rsp *)hdr)->StructureSize) ==
			SMB2_ERROR_STRUCTURE_SIZE2_LE)
		return NULL;

	/*
	 * Following commands have data areas so we have to get the location
	 * of the data buffer offset and data buffer length for the particular
	 * command.
	 */
	switch (hdr->Command) {
	case SMB2_NEGOTIATE:
		if (!get_neg_context_size((char *)hdr, off, len)) {
			*off = __SMB2_HEADER_STRUCTURE_SIZE + 36;
			*len = le16_to_cpu(((struct smb2_negotiate_req *)
				hdr)->DialectCount) * 2;
		}
		break;
	case SMB2_SESSION_SETUP:
		*off = le16_to_cpu(
		     ((struct smb2_sess_setup_req *)hdr)->SecurityBufferOffset);
		*len = le16_to_cpu(
		     ((struct smb2_sess_setup_req *)hdr)->SecurityBufferLength);
		break;
	case SMB2_TREE_CONNECT:
		*off = le16_to_cpu(
		     ((struct smb2_tree_connect_req *)hdr)->PathOffset);
		*len = le16_to_cpu(
		     ((struct smb2_tree_connect_req *)hdr)->PathLength);
		break;
	case SMB2_CREATE:
	{
		if (((struct smb2_create_req *)hdr)->CreateContextsLength) {
			*off = le32_to_cpu(((struct smb2_create_req *)
				hdr)->CreateContextsOffset);
			*len = le32_to_cpu(((struct smb2_create_req *)
				hdr)->CreateContextsLength);
			break;
		}

		*off = le16_to_cpu(
		     ((struct smb2_create_req *)hdr)->NameOffset);
		*len = le16_to_cpu(
		     ((struct smb2_create_req *)hdr)->NameLength);
		break;
	}
	case SMB2_QUERY_INFO:
		*off = le16_to_cpu(
		     ((struct smb2_query_info_req *)hdr)->InputBufferOffset);
		*len = le32_to_cpu(
		     ((struct smb2_query_info_req *)hdr)->InputBufferLength);
		break;
	case SMB2_SET_INFO:
		*off = le16_to_cpu(
		     ((struct smb2_set_info_req *)hdr)->BufferOffset);
		*len = le32_to_cpu(
		     ((struct smb2_set_info_req *)hdr)->BufferLength);
		break;
	case SMB2_READ:
		*off = le16_to_cpu(
		     ((struct smb2_read_req *)hdr)->ReadChannelInfoOffset);
		*len = le16_to_cpu(
		     ((struct smb2_read_req *)hdr)->ReadChannelInfoLength);
		break;
	case SMB2_WRITE:
		if (((struct smb2_write_req *)hdr)->DataOffset) {
			*off = le16_to_cpu(
			     ((struct smb2_write_req *)hdr)->DataOffset);
			*len = le32_to_cpu(
				((struct smb2_write_req *)hdr)->Length);
			break;
		}

		*off = le16_to_cpu(
		     ((struct smb2_write_req *)hdr)->WriteChannelInfoOffset);
		*len = le16_to_cpu(
		     ((struct smb2_write_req *)hdr)->WriteChannelInfoLength);
		break;
	case SMB2_QUERY_DIRECTORY:
		*off = le16_to_cpu(
		     ((struct smb2_query_directory_req *)hdr)->FileNameOffset);
		*len = le16_to_cpu(
		     ((struct smb2_query_directory_req *)hdr)->FileNameLength);
		break;
	case SMB2_LOCK:
	{
		int lock_count;

		/*
		 * smb2_lock request size is 48 included single
		 * smb2_lock_element structure size.
		 */
		lock_count = le16_to_cpu(
			((struct smb2_lock_req *)hdr)->LockCount) - 1;
		if (lock_count > 0) {
			*off = __SMB2_HEADER_STRUCTURE_SIZE + 48;
			*len = sizeof(struct smb2_lock_element) * lock_count;
		}
		break;
	}
	case SMB2_IOCTL:
		*off = le32_to_cpu(
		     ((struct smb2_ioctl_req *)hdr)->InputOffset);
		*len = le32_to_cpu(((struct smb2_ioctl_req *)hdr)->InputCount);

		break;
	default:
		smbd_debug("no length check for command\n");
		break;
	}

	/*
	 * Invalid length or offset probably means data area is invalid, but
	 * we have little choice but to ignore the data area in this case.
	 */
	if (*off > 4096) {
		smbd_debug("offset %d too large, data area ignored\n", *off);
		*len = 0;
		*off = 0;
	} else if (*off < 0) {
		smbd_debug("negative offset %d to data invalid ignore data area\n",
			*off);
		*off = 0;
		*len = 0;
	} else if (*len < 0) {
		smbd_debug("negative data length %d invalid, data area ignored\n",
			*len);
		*len = 0;
	} else if (*len > 128 * 1024) {
		smbd_debug("data area larger than 128K: %d\n", *len);
		*len = 0;
	}

	/* return pointer to beginning of data area, ie offset from SMB start */
	if ((*off != 0) && (*len != 0))
		return (char *)hdr + *off;
	else
		return NULL;
}

/*
 * Calculate the size of the SMB message based on the fixed header
 * portion, the number of word parameters and the data portion of the message.
 */
static unsigned int smb2_calc_size(void *buf)
{
	struct smb2_pdu *pdu = (struct smb2_pdu *)buf;
	struct smb2_hdr *hdr = &pdu->hdr;
	int offset; /* the offset from the beginning of SMB to data area */
	int data_length; /* the length of the variable length data area */
	/* Structure Size has already been checked to make sure it is 64 */
	int len = le16_to_cpu(hdr->StructureSize);

	/*
	 * StructureSize2, ie length of fixed parameter area has already
	 * been checked to make sure it is the correct length.
	 */
	len += le16_to_cpu(pdu->StructureSize2);

	if (has_smb2_data_area[le16_to_cpu(hdr->Command)] == false)
		goto calc_size_exit;

	smb2_get_data_area_len(&offset, &data_length, hdr);
	smbd_debug("SMB2 data length %d offset %d\n", data_length, offset);

	if (data_length > 0) {
		/*
		 * Check to make sure that data area begins after fixed area,
		 * Note that last byte of the fixed area is part of data area
		 * for some commands, typically those with odd StructureSize,
		 * so we must add one to the calculation.
		 */
		if (offset + 1 < len)
			smbd_debug("data area offset %d overlaps SMB2 header %d\n",
					offset + 1, len);
		else
			len = offset + data_length;
	}
calc_size_exit:
	smbd_debug("SMB2 len %d\n", len);
	return len;
}

static inline int smb2_query_info_req_len(struct smb2_query_info_req *h)
{
	return le32_to_cpu(h->InputBufferLength) +
		le32_to_cpu(h->OutputBufferLength);
}

static inline int smb2_set_info_req_len(struct smb2_set_info_req *h)
{
	return le32_to_cpu(h->BufferLength);
}

static inline int smb2_read_req_len(struct smb2_read_req *h)
{
	return le32_to_cpu(h->Length);
}

static inline int smb2_write_req_len(struct smb2_write_req *h)
{
	return le32_to_cpu(h->Length);
}

static inline int smb2_query_dir_req_len(struct smb2_query_directory_req *h)
{
	return le32_to_cpu(h->OutputBufferLength);
}

static inline int smb2_ioctl_req_len(struct smb2_ioctl_req *h)
{
	return le32_to_cpu(h->InputCount) +
		le32_to_cpu(h->OutputCount);
}

static inline int smb2_ioctl_resp_len(struct smb2_ioctl_req *h)
{
	return le32_to_cpu(h->MaxInputResponse) +
		le32_to_cpu(h->MaxOutputResponse);
}

static int smb2_validate_credit_charge(struct smb2_hdr *hdr)
{
	int req_len = 0, expect_resp_len = 0, calc_credit_num, max_len;
	int credit_charge = le16_to_cpu(hdr->CreditCharge);
	void *__hdr = hdr;

	switch (hdr->Command) {
	case SMB2_QUERY_INFO:
		req_len = smb2_query_info_req_len(__hdr);
		break;
	case SMB2_SET_INFO:
		req_len = smb2_set_info_req_len(__hdr);
		break;
	case SMB2_READ:
		req_len = smb2_read_req_len(__hdr);
		break;
	case SMB2_WRITE:
		req_len = smb2_write_req_len(__hdr);
		break;
	case SMB2_QUERY_DIRECTORY:
		req_len = smb2_query_dir_req_len(__hdr);
		break;
	case SMB2_IOCTL:
		req_len = smb2_ioctl_req_len(__hdr);
		expect_resp_len = smb2_ioctl_resp_len(__hdr);
		break;
	default:
		return 0;
	}

	max_len = max(req_len, expect_resp_len);
	calc_credit_num = (max_len - 1) / 65536 + 1;
	if (!credit_charge && max_len > 65536) {
		smbd_err("credit charge is zero and payload size(%d) is bigger than 64K\n",
			max_len);
		return 1;
	} else if (credit_charge < calc_credit_num) {
		smbd_err("credit charge : %d, calc_credit_num : %d\n",
			credit_charge, calc_credit_num);
		return 1;
	}

	return 0;
}

int smbd_smb2_check_message(struct smbd_work *work)
{
	struct smb2_pdu *pdu = REQUEST_BUF(work);
	struct smb2_hdr *hdr = &pdu->hdr;
	int command;
	__u32 clc_len;  /* calculated length */
	__u32 len = get_rfc1002_len(pdu);

	if (work->next_smb2_rcv_hdr_off) {
		pdu = REQUEST_BUF_NEXT(work);
		hdr = &pdu->hdr;
	}

	if (le32_to_cpu(hdr->NextCommand) > 0)
		len = le32_to_cpu(hdr->NextCommand);
	else if (work->next_smb2_rcv_hdr_off) {
		len -= work->next_smb2_rcv_hdr_off;
		len = round_up(len, 8);
	}

	if (check_smb2_hdr(hdr))
		return 1;

	if (hdr->StructureSize != SMB2_HEADER_STRUCTURE_SIZE) {
		smbd_debug("Illegal structure size %u\n",
			le16_to_cpu(hdr->StructureSize));
		return 1;
	}

	command = le16_to_cpu(hdr->Command);
	if (command >= NUMBER_OF_SMB2_COMMANDS) {
		smbd_debug("Illegal SMB2 command %d\n", command);
		return 1;
	}

	if (smb2_req_struct_sizes[command] != pdu->StructureSize2) {
		if (command != SMB2_OPLOCK_BREAK_HE && (hdr->Status == 0 ||
		    pdu->StructureSize2 != SMB2_ERROR_STRUCTURE_SIZE2_LE)) {
			/* error packets have 9 byte structure size */
			smbd_debug("Illegal request size %u for command %d\n",
				le16_to_cpu(pdu->StructureSize2), command);
			return 1;
		} else if (command == SMB2_OPLOCK_BREAK_HE
				&& (hdr->Status == 0)
				&& (le16_to_cpu(pdu->StructureSize2) !=
					OP_BREAK_STRUCT_SIZE_20)
				&& (le16_to_cpu(pdu->StructureSize2) !=
					OP_BREAK_STRUCT_SIZE_21)) {
			/* special case for SMB2.1 lease break message */
			smbd_debug("Illegal request size %d for oplock break\n",
				le16_to_cpu(pdu->StructureSize2));
			return 1;
		}
	}

	clc_len = smb2_calc_size(hdr);
	if (len != clc_len) {
		/* server can return one byte more due to implied bcc[0] */
		if (clc_len == len + 1)
			return 0;

		/*
		 * Some windows servers (win2016) will pad also the final
		 * PDU in a compound to 8 bytes.
		 */
		if (ALIGN(clc_len, 8) == len)
			return 0;

		/*
		 * windows client also pad up to 8 bytes when compounding.
		 * If pad is longer than eight bytes, log the server behavior
		 * (once), since may indicate a problem but allow it and
		 * continue since the frame is parseable.
		 */
		if (clc_len < len) {
			smbd_debug(
				"cli req padded more than expected. Length %d not %d for cmd:%d mid:%llu\n",
					len, clc_len, command,
					le64_to_cpu(hdr->MessageId));
			return 0;
		}
		smbd_debug(
			"cli req too short, len %d not %d. cmd:%d mid:%llu\n",
				len, clc_len, command,
				le64_to_cpu(hdr->MessageId));

		return 1;
	}

	return work->conn->vals->capabilities & SMB2_GLOBAL_CAP_LARGE_MTU ?
		smb2_validate_credit_charge(hdr) : 0;
}

int smb2_negotiate_request(struct smbd_work *work)
{
	return smbd_smb_negotiate_common(work, SMB2_NEGOTIATE_HE);
}
