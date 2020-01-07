/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#ifndef __AUTH_H__
#define __AUTH_H__

#include "ntlmssp.h"

#define AUTH_GSS_LENGTH		74
#define AUTH_GSS_PADDING	6

#define CIFS_HMAC_MD5_HASH_SIZE	(16)
#define CIFS_NTHASH_SIZE	(16)

/*
 * Size of the ntlm client response
 */
#define CIFS_AUTH_RESP_SIZE		24
#define CIFS_SMB1_SIGNATURE_SIZE	8
#define CIFS_SMB1_SESSKEY_SIZE		16

struct smbd_session;
struct smbd_conn;
struct kvec;

int smbd_crypt_message(struct smbd_conn *conn,
			struct kvec *iov,
			unsigned int nvec,
			int enc);

void smbd_copy_gss_neg_header(void *buf);

int smbd_auth_ntlm(struct smbd_session *sess,
		    char *pw_buf);

int smbd_auth_ntlmv2(struct smbd_session *sess,
		      struct ntlmv2_resp *ntlmv2,
		      int blen,
		      char *domain_name);

int smbd_decode_ntlmssp_auth_blob(struct authenticate_message *authblob,
				   int blob_len,
				   struct smbd_session *sess);

int smbd_decode_ntlmssp_neg_blob(struct negotiate_message *negblob,
				  int blob_len,
				  struct smbd_session *sess);

unsigned int
smbd_build_ntlmssp_challenge_blob(struct challenge_message *chgblob,
		struct smbd_session *sess);

int smbd_sign_smb1_pdu(struct smbd_session *sess,
			struct kvec *iov,
			int n_vec,
			char *sig);
int smbd_sign_smb2_pdu(struct smbd_conn *conn,
			char *key,
			struct kvec *iov,
			int n_vec,
			char *sig);
int smbd_sign_smb3_pdu(struct smbd_conn *conn,
			char *key,
			struct kvec *iov,
			int n_vec,
			char *sig);

int smbd_gen_smb30_signingkey(struct smbd_session *sess);
int smbd_gen_smb311_signingkey(struct smbd_session *sess);
int smbd_gen_smb30_encryptionkey(struct smbd_session *sess);
int smbd_gen_smb311_encryptionkey(struct smbd_session *sess);

int smbd_gen_preauth_integrity_hash(struct smbd_conn *conn,
				     char *buf,
				     __u8 *pi_hash);
#endif
