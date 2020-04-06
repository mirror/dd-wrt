// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2016 Namjae Jeon <linkinjeon@gmail.com>
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/backing-dev.h>
#include <linux/writeback.h>
#include <linux/uio.h>
#include <linux/xattr.h>
#include <crypto/hash.h>
#include <crypto/aead.h>
#include <linux/random.h>
#include <linux/scatterlist.h>

#include "auth.h"
#include "glob.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
#include <linux/fips.h>
#include <crypto/des.h>
#endif

#include "server.h"
#include "smb_common.h"
#include "connection.h"
#include "mgmt/user_session.h"
#include "mgmt/user_config.h"
#include "crypto_ctx.h"

/*
 * Fixed format data defining GSS header and fixed string
 * "not_defined_in_RFC4178@please_ignore".
 * So sec blob data in neg phase could be generated statically.
 */
static char NEGOTIATE_GSS_HEADER[AUTH_GSS_LENGTH] = {
	0x60, 0x48, 0x06, 0x06, 0x2b, 0x06, 0x01, 0x05,
	0x05, 0x02, 0xa0, 0x3e, 0x30, 0x3c, 0xa0, 0x0e,
	0x30, 0x0c, 0x06, 0x0a, 0x2b, 0x06, 0x01, 0x04,
	0x01, 0x82, 0x37, 0x02, 0x02, 0x0a, 0xa3, 0x2a,
	0x30, 0x28, 0xa0, 0x26, 0x1b, 0x24, 0x6e, 0x6f,
	0x74, 0x5f, 0x64, 0x65, 0x66, 0x69, 0x6e, 0x65,
	0x64, 0x5f, 0x69, 0x6e, 0x5f, 0x52, 0x46, 0x43,
	0x34, 0x31, 0x37, 0x38, 0x40, 0x70, 0x6c, 0x65,
	0x61, 0x73, 0x65, 0x5f, 0x69, 0x67, 0x6e, 0x6f,
	0x72, 0x65
};

void ksmbd_copy_gss_neg_header(void *buf)
{
	memcpy(buf, NEGOTIATE_GSS_HEADER, AUTH_GSS_LENGTH);
}

static void
str_to_key(unsigned char *str, unsigned char *key)
{
	int i;

	key[0] = str[0] >> 1;
	key[1] = ((str[0] & 0x01) << 6) | (str[1] >> 2);
	key[2] = ((str[1] & 0x03) << 5) | (str[2] >> 3);
	key[3] = ((str[2] & 0x07) << 4) | (str[3] >> 4);
	key[4] = ((str[3] & 0x0F) << 3) | (str[4] >> 5);
	key[5] = ((str[4] & 0x1F) << 2) | (str[5] >> 6);
	key[6] = ((str[5] & 0x3F) << 1) | (str[6] >> 7);
	key[7] = str[6] & 0x7F;
	for (i = 0; i < 8; i++)
		key[i] = (key[i] << 1);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
static int
smbhash(unsigned char *out, const unsigned char *in, unsigned char *key)
{
	unsigned char key2[8];
	struct des_ctx ctx;

	str_to_key(key, key2);

	if (fips_enabled) {
		ksmbd_debug(AUTH,
			"FIPS compliance enabled: DES not permitted\n");
		return -ENOENT;
	}

	des_expand_key(&ctx, key2, DES_KEY_SIZE);
	des_encrypt(&ctx, out, in);
	memzero_explicit(&ctx, sizeof(ctx));
	return 0;
}
#else
static int
smbhash(unsigned char *out, const unsigned char *in, unsigned char *key)
{
	int rc;
	unsigned char key2[8];
	struct scatterlist sgin, sgout;
	struct ksmbd_crypto_ctx *ctx;

	str_to_key(key, key2);

	ctx = ksmbd_crypto_ctx_find_ecbdes();
	if (!ctx) {
		ksmbd_debug(AUTH, "could not allocate des crypto API\n");
		return -EINVAL;
	}

	crypto_blkcipher_setkey(CRYPTO_ECBDES_TFM(ctx), key2, 8);

	sg_init_one(&sgin, in, 8);
	sg_init_one(&sgout, out, 8);

	rc = crypto_blkcipher_encrypt(CRYPTO_ECBDES(ctx), &sgout, &sgin, 8);
	if (rc)
		ksmbd_debug(AUTH, "could not encrypt crypt key rc: %d\n", rc);
	ksmbd_release_crypto_ctx(ctx);
	return rc;
}
#endif

static int ksmbd_enc_p24(unsigned char *p21,
			 const unsigned char *c8,
			 unsigned char *p24)
{
	int rc;

	rc = smbhash(p24, c8, p21);
	if (rc)
		return rc;
	rc = smbhash(p24 + 8, c8, p21 + 7);
	if (rc)
		return rc;
	rc = smbhash(p24 + 16, c8, p21 + 14);
	return rc;
}

/* produce a md4 message digest from data of length n bytes */
static int ksmbd_enc_md4(unsigned char *md4_hash,
			 unsigned char *link_str,
			 int link_len)
{
	int rc;
	struct ksmbd_crypto_ctx *ctx;

	ctx = ksmbd_crypto_ctx_find_md4();
	if (!ctx) {
		ksmbd_debug(AUTH, "Crypto md4 allocation error\n");
		return -EINVAL;
	}

	rc = crypto_shash_init(CRYPTO_MD4(ctx));
	if (rc) {
		ksmbd_debug(AUTH, "Could not init md4 shash\n");
		goto out;
	}

	rc = crypto_shash_update(CRYPTO_MD4(ctx), link_str, link_len);
	if (rc) {
		ksmbd_debug(AUTH, "Could not update with link_str\n");
		goto out;
	}

	rc = crypto_shash_final(CRYPTO_MD4(ctx), md4_hash);
	if (rc)
		ksmbd_debug(AUTH, "Could not generate md4 hash\n");
out:
	ksmbd_release_crypto_ctx(ctx);
	return rc;
}

static int ksmbd_enc_update_sess_key(unsigned char *md5_hash,
				     char *nonce,
				     char *server_challenge,
				     int len)
{
	int rc;
	struct ksmbd_crypto_ctx *ctx;

	ctx = ksmbd_crypto_ctx_find_md5();
	if (!ctx) {
		ksmbd_debug(AUTH, "Crypto md5 allocation error\n");
		return -EINVAL;
	}

	rc = crypto_shash_init(CRYPTO_MD5(ctx));
	if (rc) {
		ksmbd_debug(AUTH, "Could not init md5 shash\n");
		goto out;
	}

	rc = crypto_shash_update(CRYPTO_MD5(ctx), server_challenge, len);
	if (rc) {
		ksmbd_debug(AUTH, "Could not update with challenge\n");
		goto out;
	}

	rc = crypto_shash_update(CRYPTO_MD5(ctx), nonce, len);
	if (rc) {
		ksmbd_debug(AUTH, "Could not update with nonce\n");
		goto out;
	}

	rc = crypto_shash_final(CRYPTO_MD5(ctx), md5_hash);
	if (rc)
		ksmbd_debug(AUTH, "Could not generate md5 hash\n");
out:
	ksmbd_release_crypto_ctx(ctx);
	return rc;
}

/**
 * smbd_gen_sess_key() - function to generate session key
 * @sess:	session of connection
 * @hash:	source hash value to be used for find session key
 * @hmac:	source hmac value to be used for finding session key
 *
 */
static int ksmbd_gen_sess_key(struct ksmbd_session *sess,
			      char *hash,
			      char *hmac)
{
	struct ksmbd_crypto_ctx *ctx;
	int rc = -EINVAL;

	ksmbd_debug(ALL, "enter %s\n", __func__);
	ctx = ksmbd_crypto_ctx_find_hmacmd5();
	if (!ctx) {
		ksmbd_debug(ALL, "hmacmd5 crypto error\n");
		goto out;
	}

	rc = crypto_shash_setkey(CRYPTO_HMACMD5_TFM(ctx),
				 hash,
				 CIFS_HMAC_MD5_HASH_SIZE);
	if (rc) {
		ksmbd_debug(AUTH, "hmacmd5 set key fail error %d\n", rc);
		goto out;
	}

	rc = crypto_shash_init(CRYPTO_HMACMD5(ctx));
	if (rc) {
		ksmbd_debug(AUTH, "could not init hmacmd5 error %d\n", rc);
		goto out;
	}

	ksmbd_debug(ALL, "%s hmac  %*ph\n",__func__, 
			SMB2_NTLMV2_SESSKEY_SIZE, hmac);
	rc = crypto_shash_update(CRYPTO_HMACMD5(ctx),
				 hmac,
				 SMB2_NTLMV2_SESSKEY_SIZE);
	if (rc) {
		ksmbd_debug(AUTH, "Could not update with response error %d\n",
			rc);
		goto out;
	}

	rc = crypto_shash_final(CRYPTO_HMACMD5(ctx), sess->sess_key);
	if (rc) {
		ksmbd_debug(AUTH, "Could not generate hmacmd5 hash error %d\n",
			rc);
		goto out;
	}
	ksmbd_debug(ALL, "%s   %*ph\n",__func__, 
			SMB2_NTLMV2_SESSKEY_SIZE, sess->sess_key);

out:
	ksmbd_release_crypto_ctx(ctx);
	return rc;
}

static int calc_ntlmv2_hash(struct ksmbd_session *sess, char *ntlmv2_hash,
	char *dname)
{
	int ret = -EINVAL, len;
	wchar_t *domain = NULL;
	__le16 *uniname = NULL;
	struct ksmbd_crypto_ctx *ctx;

	ctx = ksmbd_crypto_ctx_find_hmacmd5();
	if (!ctx) {
		ksmbd_debug(AUTH, "can't generate ntlmv2 hash\n");
		goto out;
	}

	ret = crypto_shash_setkey(CRYPTO_HMACMD5_TFM(ctx),
				  user_passkey(sess->user),
				  CIFS_ENCPWD_SIZE);
	if (ret) {
		ksmbd_debug(AUTH, "Could not set NT Hash as a key\n");
		goto out;
	}

	ret = crypto_shash_init(CRYPTO_HMACMD5(ctx));
	if (ret) {
		ksmbd_debug(AUTH, "could not init hmacmd5\n");
		goto out;
	}

	/* convert user_name to unicode */
	len = strlen(user_name(sess->user));
	uniname = kzalloc(2 + UNICODE_LEN(len), GFP_KERNEL);
	if (!uniname) {
		ret = -ENOMEM;
		goto out;
	}
	ksmbd_debug(ALL, "convert username to unicode %s\n", user_name(sess->user));
	
	if (len) {
		len = smb_strtoUTF16(uniname, user_name(sess->user), len,
			sess->conn->local_nls);
		UniStrupr(uniname);
	}

	ret = crypto_shash_update(CRYPTO_HMACMD5(ctx),
				  (char *)uniname,
				  UNICODE_LEN(len));
	if (ret) {
		ksmbd_debug(AUTH, "Could not update with user\n");
		goto out;
	}

	/* Convert domain name or conn name to unicode and uppercase */
	len = strlen(dname);
	domain = kzalloc(2 + UNICODE_LEN(len), GFP_KERNEL);
	if (!domain) {
		ret = -ENOMEM;
		goto out;
	}

	ksmbd_debug(ALL, "convert dname to unicode %s\n", dname);
	len = smb_strtoUTF16((__le16 *)domain, dname, len,
			     sess->conn->local_nls);

	ret = crypto_shash_update(CRYPTO_HMACMD5(ctx),
				  (char *)domain,
				  UNICODE_LEN(len));
	if (ret) {
		ksmbd_debug(AUTH, "Could not update with domain\n");
		goto out;
	}

	ret = crypto_shash_final(CRYPTO_HMACMD5(ctx), ntlmv2_hash);
out:
	if (ret)
		ksmbd_debug(AUTH, "Could not generate md5 hash\n");
	kfree(uniname);
	kfree(domain);
	ksmbd_release_crypto_ctx(ctx);
	return ret;
}

/**
 * ksmbd_auth_ntlm() - NTLM authentication handler
 * @sess:	session of connection
 * @pw_buf:	NTLM challenge response
 * @passkey:	user password
 *
 * Return:	0 on success, error number on error
 */
int ksmbd_auth_ntlm(struct ksmbd_session *sess, char *pw_buf)
{
	int rc;
	unsigned char p21[21];
	char key[CIFS_AUTH_RESP_SIZE];

	memset(p21, '\0', 21);
	memcpy(p21, user_passkey(sess->user), CIFS_NTHASH_SIZE);
	rc = ksmbd_enc_p24(p21, sess->ntlmssp.cryptkey, key);
	if (rc) {
		ksmbd_err("password processing failed\n");
		return rc;
	}

	ksmbd_enc_md4(sess->sess_key,
			user_passkey(sess->user),
			CIFS_SMB1_SESSKEY_SIZE);
	memcpy(sess->sess_key + CIFS_SMB1_SESSKEY_SIZE, key,
		CIFS_AUTH_RESP_SIZE);
	sess->sequence_number = 1;

	if (strncmp(pw_buf, key, CIFS_AUTH_RESP_SIZE) != 0) {
		ksmbd_debug(AUTH, "ntlmv1 authentication failed\n");
		rc = -EINVAL;
	} else
		ksmbd_debug(AUTH, "ntlmv1 authentication pass\n");

	return rc;
}

/**
 * ksmbd_auth_ntlmv2() - NTLMv2 authentication handler
 * @sess:	session of connection
 * @ntlmv2:		NTLMv2 challenge response
 * @blen:		NTLMv2 blob length
 * @domain_name:	domain name
 *
 * Return:	0 on success, error number on error
 */
int ksmbd_auth_ntlmv2(struct ksmbd_session *sess,
		      struct ntlmv2_resp *ntlmv2,
		      int blen,
		      char *domain_name)
{
	char ntlmv2_hash[CIFS_ENCPWD_SIZE];
	char ntlmv2_rsp[CIFS_HMAC_MD5_HASH_SIZE];
	struct ksmbd_crypto_ctx *ctx;
	char *construct = NULL;
	int rc = -EINVAL, len;

	ctx = ksmbd_crypto_ctx_find_hmacmd5();
	if (!ctx) {
		ksmbd_debug(AUTH, "could not crypto alloc hmacmd5 rc %d\n", rc);
		goto out;
	}

	rc = calc_ntlmv2_hash(sess, ntlmv2_hash, domain_name);
	if (rc) {
		ksmbd_debug(AUTH, "could not get v2 hash rc %d\n", rc);
		goto out;
	}

	rc = crypto_shash_setkey(CRYPTO_HMACMD5_TFM(ctx),
				 ntlmv2_hash,
				 CIFS_HMAC_MD5_HASH_SIZE);
	if (rc) {
		ksmbd_debug(AUTH, "Could not set NTLMV2 Hash as a key\n");
		goto out;
	}

	rc = crypto_shash_init(CRYPTO_HMACMD5(ctx));
	if (rc) {
		ksmbd_debug(AUTH, "Could not init hmacmd5\n");
		goto out;
	}

	len = CIFS_CRYPTO_KEY_SIZE + blen;
	construct = kzalloc(len, GFP_KERNEL);
	if (!construct) {
		rc = -ENOMEM;
		goto out;
	}

	memcpy(construct, sess->ntlmssp.cryptkey, CIFS_CRYPTO_KEY_SIZE);
	memcpy(construct + CIFS_CRYPTO_KEY_SIZE,
		(char *)(&ntlmv2->blob_signature), blen);

	rc = crypto_shash_update(CRYPTO_HMACMD5(ctx), construct, len);
	if (rc) {
		ksmbd_debug(AUTH, "Could not update with response\n");
		goto out;
	}

	rc = crypto_shash_final(CRYPTO_HMACMD5(ctx), ntlmv2_rsp);
	if (rc) {
		ksmbd_debug(AUTH, "Could not generate md5 hash\n");
		goto out;
	}

	rc = ksmbd_gen_sess_key(sess, ntlmv2_hash, ntlmv2_rsp);
	if (rc) {
		ksmbd_debug(AUTH, "Could not generate sess key\n");
		goto out;
	}

	rc = memcmp(ntlmv2->ntlmv2_hash, ntlmv2_rsp, CIFS_HMAC_MD5_HASH_SIZE);
out:
	ksmbd_release_crypto_ctx(ctx);
	kfree(construct);
	return rc;
}

/**
 * __ksmbd_auth_ntlmv2() - NTLM2(extended security) authentication handler
 * @sess:	session of connection
 * @client_nonce:	client nonce from LM response.
 * @ntlm_resp:		ntlm response data from client.
 *
 * Return:	0 on success, error number on error
 */
static int __ksmbd_auth_ntlmv2(struct ksmbd_session *sess,
			       char *client_nonce,
			       char *ntlm_resp)
{
	char sess_key[CIFS_SMB1_SESSKEY_SIZE] = {0};
	int rc;
	unsigned char p21[21];
	char key[CIFS_AUTH_RESP_SIZE];

	rc = ksmbd_enc_update_sess_key(sess_key,
				       client_nonce,
				       (char *)sess->ntlmssp.cryptkey, 8);
	if (rc) {
		ksmbd_err("password processing failed\n");
		goto out;
	}

	memset(p21, '\0', 21);
	memcpy(p21, user_passkey(sess->user), CIFS_NTHASH_SIZE);
	rc = ksmbd_enc_p24(p21, sess_key, key);
	if (rc) {
		ksmbd_err("password processing failed\n");
		goto out;
	}

	rc = memcmp(ntlm_resp, key, CIFS_AUTH_RESP_SIZE);
out:
	return rc;
}

/**
 * ksmbd_decode_ntlmssp_auth_blob() - helper function to construct
 * authenticate blob
 * @authblob:	authenticate blob source pointer
 * @usr:	user details
 * @sess:	session of connection
 *
 * Return:	0 on success, error number on error
 */
int ksmbd_decode_ntlmssp_auth_blob(struct authenticate_message *authblob,
				   int blob_len,
				   struct ksmbd_session *sess)
{
	char *domain_name;
	unsigned int lm_off, nt_off;
	unsigned short nt_len;
	int ret;

	if (blob_len < sizeof(struct authenticate_message)) {
		ksmbd_debug(AUTH, "negotiate blob len %d too small\n",
			blob_len);
		return -EINVAL;
	}

	if (memcmp(authblob->Signature, "NTLMSSP", 8)) {
		ksmbd_debug(AUTH, "blob signature incorrect %s\n",
				authblob->Signature);
		return -EINVAL;
	}

	lm_off = le32_to_cpu(authblob->LmChallengeResponse.BufferOffset);
	nt_off = le32_to_cpu(authblob->NtChallengeResponse.BufferOffset);
	nt_len = le16_to_cpu(authblob->NtChallengeResponse.Length);

	/* process NTLM authentication */
	if (nt_len == CIFS_AUTH_RESP_SIZE) {
		if (le32_to_cpu(authblob->NegotiateFlags)
			& NTLMSSP_NEGOTIATE_EXTENDED_SEC)
			return __ksmbd_auth_ntlmv2(sess, (char *)authblob +
				lm_off, (char *)authblob + nt_off);
		else
			return ksmbd_auth_ntlm(sess, (char *)authblob +
				nt_off);
	}

	/* TODO : use domain name that imported from configuration file */
	domain_name = smb_strndup_from_utf16(
			(const char *)authblob +
			le32_to_cpu(authblob->DomainName.BufferOffset),
			le16_to_cpu(authblob->DomainName.Length), true,
			sess->conn->local_nls);
	if (IS_ERR(domain_name))
		return PTR_ERR(domain_name);

	/* process NTLMv2 authentication */
	ksmbd_debug(AUTH, "decode_ntlmssp_authenticate_blob dname%s\n",
			domain_name);
	ret = ksmbd_auth_ntlmv2(sess,
			(struct ntlmv2_resp *)((char *)authblob + nt_off),
			nt_len - CIFS_ENCPWD_SIZE,
			domain_name);
	kfree(domain_name);
	return ret;
}

/**
 * ksmbd_decode_ntlmssp_neg_blob() - helper function to construct
 * negotiate blob
 * @negblob: negotiate blob source pointer
 * @rsp:     response header pointer to be updated
 * @sess:    session of connection
 *
 */
int ksmbd_decode_ntlmssp_neg_blob(struct negotiate_message *negblob,
				  int blob_len,
				  struct ksmbd_session *sess)
{
	if (blob_len < sizeof(struct negotiate_message)) {
		ksmbd_debug(AUTH, "negotiate blob len %d too small\n",
			blob_len);
		return -EINVAL;
	}

	if (memcmp(negblob->Signature, "NTLMSSP", 8)) {
		ksmbd_debug(AUTH, "blob signature incorrect %s\n",
				negblob->Signature);
		return -EINVAL;
	}

	sess->ntlmssp.client_flags = le32_to_cpu(negblob->NegotiateFlags);
	return 0;
}

/**
 * ksmbd_build_ntlmssp_challenge_blob() - helper function to construct
 * challenge blob
 * @chgblob: challenge blob source pointer to initialize
 * @rsp:     response header pointer to be updated
 * @sess:    session of connection
 *
 */
unsigned int
ksmbd_build_ntlmssp_challenge_blob(struct challenge_message *chgblob,
		struct ksmbd_session *sess)
{
	struct target_info *tinfo;
	wchar_t *name;
	__u8 *target_name;
	unsigned int len, flags, blob_off, blob_len, type, target_info_len = 0;
	int cflags = sess->ntlmssp.client_flags;

	memcpy(chgblob->Signature, NTLMSSP_SIGNATURE, 8);
	chgblob->MessageType = NtLmChallenge;

	flags = NTLMSSP_NEGOTIATE_UNICODE |
		NTLMSSP_NEGOTIATE_NTLM | NTLMSSP_TARGET_TYPE_SERVER |
		NTLMSSP_NEGOTIATE_TARGET_INFO;

	if (cflags & NTLMSSP_NEGOTIATE_SIGN) {
		flags |= NTLMSSP_NEGOTIATE_SIGN;
		flags |= cflags & (NTLMSSP_NEGOTIATE_128 |
			NTLMSSP_NEGOTIATE_56);
	}

	if (cflags & NTLMSSP_NEGOTIATE_ALWAYS_SIGN)
		flags |= NTLMSSP_NEGOTIATE_ALWAYS_SIGN;

	if (cflags & NTLMSSP_REQUEST_TARGET)
		flags |= NTLMSSP_REQUEST_TARGET;

	if (sess->conn->use_spnego &&
		(cflags & NTLMSSP_NEGOTIATE_EXTENDED_SEC))
		flags |= NTLMSSP_NEGOTIATE_EXTENDED_SEC;

	chgblob->NegotiateFlags = cpu_to_le32(flags);
	len = strlen(ksmbd_netbios_name());
	name = kmalloc(2 + (len * 2), GFP_KERNEL);
	if (!name)
		return -ENOMEM;

	len = smb_strtoUTF16((__le16 *)name, ksmbd_netbios_name(), len,
			sess->conn->local_nls);
	len = UNICODE_LEN(len);

	blob_off = sizeof(struct challenge_message);
	blob_len = blob_off + len;

	chgblob->TargetName.Length = cpu_to_le16(len);
	chgblob->TargetName.MaximumLength = cpu_to_le16(len);
	chgblob->TargetName.BufferOffset = cpu_to_le32(blob_off);

	/* Initialize random conn challenge */
	get_random_bytes(sess->ntlmssp.cryptkey, sizeof(__u64));
	memcpy(chgblob->Challenge, sess->ntlmssp.cryptkey,
		CIFS_CRYPTO_KEY_SIZE);

	/* Add Target Information to security buffer */
	chgblob->TargetInfoArray.BufferOffset = cpu_to_le32(blob_len);

	target_name = (__u8 *)chgblob + blob_off;
	memcpy(target_name, name, len);
	tinfo = (struct target_info *)(target_name + len);

	chgblob->TargetInfoArray.Length = 0;
	/* Add target info list for NetBIOS/DNS settings */
	for (type = NTLMSSP_AV_NB_COMPUTER_NAME;
		type <= NTLMSSP_AV_DNS_DOMAIN_NAME; type++) {
		tinfo->Type = cpu_to_le16(type);
		tinfo->Length = cpu_to_le16(len);
		memcpy(tinfo->Content, name, len);
		tinfo = (struct target_info *)((char *)tinfo + 4 + len);
		target_info_len += 4 + len;
	}

	/* Add terminator subblock */
	tinfo->Type = 0;
	tinfo->Length = 0;
	target_info_len += 4;

	chgblob->TargetInfoArray.Length = cpu_to_le16(target_info_len);
	chgblob->TargetInfoArray.MaximumLength = cpu_to_le16(target_info_len);
	blob_len += target_info_len;
	kfree(name);
	ksmbd_debug(AUTH, "NTLMSSP SecurityBufferLength %d\n", blob_len);
	return blob_len;
}

#ifdef CONFIG_SMB_INSECURE_SERVER
/**
 * smbd_sign_smb1_pdu() - function to generate SMB1 packet signing
 * @sess:	session of connection
 * @iov:        buffer iov array
 * @n_vec:	number of iovecs
 * @sig:        signature value generated for client request packet
 *
 */
int ksmbd_sign_smb1_pdu(struct ksmbd_session *sess,
			struct kvec *iov,
			int n_vec,
			char *sig)
{
	struct ksmbd_crypto_ctx *ctx;
	int rc = -EINVAL;
	int i;

	ctx = ksmbd_crypto_ctx_find_md5();
	if (!ctx) {
		ksmbd_debug(AUTH, "could not crypto alloc md5 rc %d\n", rc);
		goto out;
	}

	rc = crypto_shash_init(CRYPTO_MD5(ctx));
	if (rc) {
		ksmbd_debug(AUTH, "md5 init error %d\n", rc);
		goto out;
	}

	ksmbd_debug(ALL, "%s   %*ph\n",__func__, 
			SMB2_NTLMV2_SESSKEY_SIZE, sess->sess_key);
	rc = crypto_shash_update(CRYPTO_MD5(ctx), sess->sess_key, 40);
	if (rc) {
		ksmbd_debug(AUTH, "md5 update error %d\n", rc);
		goto out;
	}

	for (i = 0; i < n_vec; i++) {
		rc = crypto_shash_update(CRYPTO_MD5(ctx),
					 iov[i].iov_base,
					 iov[i].iov_len);
		if (rc) {
			ksmbd_debug(AUTH, "md5 update error %d\n", rc);
			goto out;
		}
	}

	rc = crypto_shash_final(CRYPTO_MD5(ctx), sig);
	if (rc)
		ksmbd_debug(AUTH, "md5 generation error %d\n", rc);

out:
	ksmbd_release_crypto_ctx(ctx);
	return rc;
}
#endif

/**
 * ksmbd_sign_smb2_pdu() - function to generate packet signing
 * @conn:	connection
 * @key:	signing key
 * @iov:        buffer iov array
 * @n_vec:	number of iovecs
 * @sig:	signature value generated for client request packet
 *
 */
int ksmbd_sign_smb2_pdu(struct ksmbd_conn *conn,
			char *key,
			struct kvec *iov,
			int n_vec,
			char *sig)
{
	struct ksmbd_crypto_ctx *ctx;
	int rc = -EINVAL;
	int i;

	ctx = ksmbd_crypto_ctx_find_hmacsha256();
	if (!ctx) {
		ksmbd_debug(AUTH, "could not crypto alloc hmacmd5 rc %d\n", rc);
		goto out;
	}

	rc = crypto_shash_setkey(CRYPTO_HMACSHA256_TFM(ctx),
				 key,
				 SMB2_NTLMV2_SESSKEY_SIZE);
	if (rc)
		goto out;

	rc = crypto_shash_init(CRYPTO_HMACSHA256(ctx));
	if (rc) {
		ksmbd_debug(AUTH, "hmacsha256 init error %d\n", rc);
		goto out;
	}

	for (i = 0; i < n_vec; i++) {
		rc = crypto_shash_update(CRYPTO_HMACSHA256(ctx),
					 iov[i].iov_base,
					 iov[i].iov_len);
		if (rc) {
			ksmbd_debug(AUTH, "hmacsha256 update error %d\n", rc);
			goto out;
		}
	}

	rc = crypto_shash_final(CRYPTO_HMACSHA256(ctx), sig);
	if (rc)
		ksmbd_debug(AUTH, "hmacsha256 generation error %d\n", rc);
out:
	ksmbd_release_crypto_ctx(ctx);
	return rc;
}

/**
 * ksmbd_sign_smb3_pdu() - function to generate packet signing
 * @conn:	connection
 * @key:	signing key
 * @iov:        buffer iov array
 * @n_vec:	number of iovecs
 * @sig:	signature value generated for client request packet
 *
 */
int ksmbd_sign_smb3_pdu(struct ksmbd_conn *conn,
			char *key,
			struct kvec *iov,
			int n_vec,
			char *sig)
{
	struct ksmbd_crypto_ctx *ctx;
	int rc = -EINVAL;
	int i;

	ctx = ksmbd_crypto_ctx_find_cmacaes();
	if (!ctx) {
		ksmbd_debug(AUTH, "could not crypto alloc cmac rc %d\n", rc);
		goto out;
	}

	rc = crypto_shash_setkey(CRYPTO_CMACAES_TFM(ctx),
				 key,
				 SMB2_CMACAES_SIZE);
	if (rc)
		goto out;

	rc = crypto_shash_init(CRYPTO_CMACAES(ctx));
	if (rc) {
		ksmbd_debug(AUTH, "cmaces init error %d\n", rc);
		goto out;
	}

	for (i = 0; i < n_vec; i++) {
		rc = crypto_shash_update(CRYPTO_CMACAES(ctx),
					 iov[i].iov_base,
					 iov[i].iov_len);
		if (rc) {
			ksmbd_debug(AUTH, "cmaces update error %d\n", rc);
			goto out;
		}
	}

	rc = crypto_shash_final(CRYPTO_CMACAES(ctx), sig);
	if (rc)
		ksmbd_debug(AUTH, "cmaces generation error %d\n", rc);
out:
	ksmbd_release_crypto_ctx(ctx);
	return rc;
}

struct derivation {
	struct kvec label;
	struct kvec context;
	bool binding;
};

static int generate_key(struct ksmbd_session *sess, struct kvec label,
	struct kvec context, __u8 *key, unsigned int key_size)
{
	unsigned char zero = 0x0;
	__u8 i[4] = {0, 0, 0, 1};
	__u8 L[4] = {0, 0, 0, 128};
	int rc = -EINVAL;
	unsigned char prfhash[SMB2_HMACSHA256_SIZE];
	unsigned char *hashptr = prfhash;
	struct ksmbd_crypto_ctx *ctx;

	memset(prfhash, 0x0, SMB2_HMACSHA256_SIZE);
	memset(key, 0x0, key_size);

	ctx = ksmbd_crypto_ctx_find_hmacsha256();
	if (!ctx) {
		ksmbd_debug(AUTH, "could not crypto alloc hmacmd5 rc %d\n", rc);
		goto smb3signkey_ret;
	}

	ksmbd_debug(ALL, "%s   %*ph\n",__func__, 
			SMB2_NTLMV2_SESSKEY_SIZE, sess->sess_key);
	rc = crypto_shash_setkey(CRYPTO_HMACSHA256_TFM(ctx),
				 sess->sess_key,
				 SMB2_NTLMV2_SESSKEY_SIZE);
	if (rc)
		goto smb3signkey_ret;

	rc = crypto_shash_init(CRYPTO_HMACSHA256(ctx));
	if (rc) {
		ksmbd_debug(AUTH, "hmacsha256 init error %d\n", rc);
		goto smb3signkey_ret;
	}

	rc = crypto_shash_update(CRYPTO_HMACSHA256(ctx), i, 4);
	if (rc) {
		ksmbd_debug(AUTH, "could not update with n\n");
		goto smb3signkey_ret;
	}

	rc = crypto_shash_update(CRYPTO_HMACSHA256(ctx),
				 label.iov_base,
				 label.iov_len);
	if (rc) {
		ksmbd_debug(AUTH, "could not update with label\n");
		goto smb3signkey_ret;
	}

	rc = crypto_shash_update(CRYPTO_HMACSHA256(ctx), &zero, 1);
	if (rc) {
		ksmbd_debug(AUTH, "could not update with zero\n");
		goto smb3signkey_ret;
	}

	rc = crypto_shash_update(CRYPTO_HMACSHA256(ctx),
				 context.iov_base,
				 context.iov_len);
	if (rc) {
		ksmbd_debug(AUTH, "could not update with context\n");
		goto smb3signkey_ret;
	}

	rc = crypto_shash_update(CRYPTO_HMACSHA256(ctx), L, 4);
	if (rc) {
		ksmbd_debug(AUTH, "could not update with L\n");
		goto smb3signkey_ret;
	}

	rc = crypto_shash_final(CRYPTO_HMACSHA256(ctx), hashptr);
	if (rc) {
		ksmbd_debug(AUTH, "Could not generate hmacmd5 hash error %d\n",
			rc);
		goto smb3signkey_ret;
	}

	memcpy(key, hashptr, key_size);

smb3signkey_ret:
	ksmbd_release_crypto_ctx(ctx);
	return rc;
}

static int generate_smb3signingkey(struct ksmbd_session *sess,
				   const struct derivation *signing)
{
	int rc;
	struct channel *chann;
	char *key;

	chann = lookup_chann_list(sess);
	if (!chann)
		return 0;

	if (sess->conn->dialect >= SMB30_PROT_ID && signing->binding)
		key = chann->smb3signingkey;
	else
		key = sess->smb3signingkey;

	rc = generate_key(sess, signing->label, signing->context, key,
		SMB3_SIGN_KEY_SIZE);
	if (rc)
		return rc;

	if (!(sess->conn->dialect >= SMB30_PROT_ID && signing->binding))
		memcpy(chann->smb3signingkey, key, SMB3_SIGN_KEY_SIZE);

	ksmbd_debug(AUTH, "dumping generated AES signing keys\n");
	ksmbd_debug(AUTH, "Session Id    %llu\n", sess->id);
	ksmbd_debug(AUTH, "Session Key   %*ph\n",
			SMB2_NTLMV2_SESSKEY_SIZE, sess->sess_key);
	ksmbd_debug(AUTH, "Signing Key   %*ph\n",
			SMB3_SIGN_KEY_SIZE, key);
	return rc;
}

int ksmbd_gen_smb30_signingkey(struct ksmbd_session *sess)
{
	struct derivation d;

	d.label.iov_base = "SMB2AESCMAC";
	d.label.iov_len = 12;
	d.context.iov_base = "SmbSign";
	d.context.iov_len = 8;
	d.binding = false;

	return generate_smb3signingkey(sess, &d);
}

int ksmbd_gen_smb311_signingkey(struct ksmbd_session *sess)
{
	struct derivation d;

	d.label.iov_base = "SMBSigningKey";
	d.label.iov_len = 14;
	d.context.iov_base = sess->Preauth_HashValue;
	d.context.iov_len = 64;
	d.binding = false;

	return generate_smb3signingkey(sess, &d);
}

struct derivation_twin {
	struct derivation encryption;
	struct derivation decryption;
};

static int generate_smb3encryptionkey(struct ksmbd_session *sess,
	const struct derivation_twin *ptwin)
{
	int rc;

	rc = generate_key(sess, ptwin->encryption.label,
			ptwin->encryption.context, sess->smb3encryptionkey,
			SMB3_SIGN_KEY_SIZE);
	if (rc)
		return rc;

	rc = generate_key(sess, ptwin->decryption.label,
			ptwin->decryption.context,
			sess->smb3decryptionkey, SMB3_SIGN_KEY_SIZE);
	if (rc)
		return rc;

	ksmbd_debug(AUTH, "dumping generated AES encryption keys\n");
	ksmbd_debug(AUTH, "Session Id    %llu\n", sess->id);
	ksmbd_debug(AUTH, "Session Key   %*ph\n",
			SMB2_NTLMV2_SESSKEY_SIZE, sess->sess_key);
	ksmbd_debug(AUTH, "ServerIn Key  %*ph\n",
			SMB3_SIGN_KEY_SIZE, sess->smb3encryptionkey);
	ksmbd_debug(AUTH, "ServerOut Key %*ph\n",
			SMB3_SIGN_KEY_SIZE, sess->smb3decryptionkey);
	return rc;
}

int ksmbd_gen_smb30_encryptionkey(struct ksmbd_session *sess)
{
	struct derivation_twin twin;
	struct derivation *d;

	d = &twin.encryption;
	d->label.iov_base = "SMB2AESCCM";
	d->label.iov_len = 11;
	d->context.iov_base = "ServerOut";
	d->context.iov_len = 10;

	d = &twin.decryption;
	d->label.iov_base = "SMB2AESCCM";
	d->label.iov_len = 11;
	d->context.iov_base = "ServerIn ";
	d->context.iov_len = 10;

	return generate_smb3encryptionkey(sess, &twin);
}

int ksmbd_gen_smb311_encryptionkey(struct ksmbd_session *sess)
{
	struct derivation_twin twin;
	struct derivation *d;

	d = &twin.encryption;
	d->label.iov_base = "SMBS2CCipherKey";
	d->label.iov_len = 16;
	d->context.iov_base = sess->Preauth_HashValue;
	d->context.iov_len = 64;

	d = &twin.decryption;
	d->label.iov_base = "SMBC2SCipherKey";
	d->label.iov_len = 16;
	d->context.iov_base = sess->Preauth_HashValue;
	d->context.iov_len = 64;

	return generate_smb3encryptionkey(sess, &twin);
}

int ksmbd_gen_preauth_integrity_hash(struct ksmbd_conn *conn,
				     char *buf,
				     __u8 *pi_hash)
{
	int rc = -1;
	struct smb2_hdr *rcv_hdr = (struct smb2_hdr *)buf;
	char *all_bytes_msg = (char *)&rcv_hdr->ProtocolId;
	int msg_size = be32_to_cpu(rcv_hdr->smb2_buf_length);
	struct ksmbd_crypto_ctx *ctx = NULL;

	if (conn->preauth_info->Preauth_HashId ==
			SMB2_PREAUTH_INTEGRITY_SHA512) {
		ctx = ksmbd_crypto_ctx_find_sha512();
		if (!ctx) {
			ksmbd_debug(AUTH, "could not alloc sha512 rc %d\n", rc);
			goto out;
		}
	} else
		goto out;

	rc = crypto_shash_init(CRYPTO_SHA512(ctx));
	if (rc) {
		ksmbd_debug(AUTH, "could not init shashn");
		goto out;
	}

	rc = crypto_shash_update(CRYPTO_SHA512(ctx), pi_hash, 64);
	if (rc) {
		ksmbd_debug(AUTH, "could not update with n\n");
		goto out;
	}

	rc = crypto_shash_update(CRYPTO_SHA512(ctx), all_bytes_msg, msg_size);
	if (rc) {
		ksmbd_debug(AUTH, "could not update with n\n");
		goto out;
	}

	rc = crypto_shash_final(CRYPTO_SHA512(ctx), pi_hash);
	if (rc) {
		ksmbd_debug(AUTH, "Could not generate hash err : %d\n", rc);
		goto out;
	}
out:
	ksmbd_release_crypto_ctx(ctx);
	return rc;
}

static int ksmbd_get_encryption_key(struct ksmbd_conn *conn,
				    __u64 ses_id,
				    int enc,
				    u8 *key)
{
	struct ksmbd_session *sess;
	u8 *ses_enc_key;

	sess = ksmbd_session_lookup(conn, ses_id);
	if (!sess)
		return 1;

	ses_enc_key = enc ? sess->smb3encryptionkey :
		sess->smb3decryptionkey;
	memcpy(key, ses_enc_key, SMB3_SIGN_KEY_SIZE);

	return 0;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 2, 0)
static struct scatterlist *ksmbd_init_sg(struct kvec *iov,
					 unsigned int nvec,
					 u8 *sign)
{
	struct scatterlist *sg;
	unsigned int i = 0;

	sg = kmalloc_array(nvec, sizeof(struct scatterlist), GFP_KERNEL);
	if (!sg)
		return NULL;

	sg_init_table(sg, nvec);
	for (i = 0; i < nvec - 1; i++)
		sg_set_buf(&sg[i], iov[i + 1].iov_base, iov[i + 1].iov_len);
	sg_set_buf(&sg[nvec - 1], sign, SMB2_SIGNATURE_SIZE);
	return sg;
}
#else
static struct scatterlist *ksmbd_init_sg(struct kvec *iov,
					 unsigned int nvec,
					 u8 *sign)
{
	struct scatterlist *sg;
	unsigned int i = 0;
	unsigned int assoc_data_len = sizeof(struct smb2_transform_hdr) - 24;

	sg = kmalloc_array(nvec + 1, sizeof(struct scatterlist), GFP_KERNEL);
	if (!sg)
		return NULL;

	sg_init_table(sg, nvec + 1);
	sg_set_buf(&sg[0], iov[0].iov_base + 24, assoc_data_len);
	for (i = 1; i < nvec; i++)
		sg_set_buf(&sg[i], iov[i].iov_base, iov[i].iov_len);
	sg_set_buf(&sg[nvec], sign, SMB2_SIGNATURE_SIZE);
	return sg;
}
#endif

int ksmbd_crypt_message(struct ksmbd_conn *conn,
			struct kvec *iov,
			unsigned int nvec,
			int enc)
{
	struct smb2_transform_hdr *tr_hdr =
		(struct smb2_transform_hdr *)iov[0].iov_base;
	unsigned int assoc_data_len = sizeof(struct smb2_transform_hdr) - 24;
	int rc = 0;
	struct scatterlist *sg;
	u8 sign[SMB2_SIGNATURE_SIZE] = {};
	u8 key[SMB3_SIGN_KEY_SIZE];
	struct aead_request *req;
	char *iv;
	unsigned int iv_len;
	struct crypto_aead *tfm;
	unsigned int crypt_len = le32_to_cpu(tr_hdr->OriginalMessageSize);
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 2, 0)
	struct scatterlist assoc;
#endif
	struct ksmbd_crypto_ctx *ctx;

	rc = ksmbd_get_encryption_key(conn,
				      le64_to_cpu(tr_hdr->SessionId),
				      enc,
				      key);
	if (rc) {
		ksmbd_err("Could not get %scryption key\n", enc ? "en" : "de");
		return 0;
	}

	if (conn->cipher_type == SMB2_ENCRYPTION_AES128_GCM)
		ctx = ksmbd_crypto_ctx_find_gcm();
	else
		ctx = ksmbd_crypto_ctx_find_ccm();
	if (!ctx) {
		ksmbd_err("crypto alloc failed\n");
		return -EINVAL;
	}

	if (conn->cipher_type == SMB2_ENCRYPTION_AES128_GCM)
		tfm = CRYPTO_GCM(ctx);
	else
		tfm = CRYPTO_CCM(ctx);

	rc = crypto_aead_setkey(tfm, key, SMB3_SIGN_KEY_SIZE);
	if (rc) {
		ksmbd_err("Failed to set aead key %d\n", rc);
		return rc;
	}

	rc = crypto_aead_setauthsize(tfm, SMB2_SIGNATURE_SIZE);
	if (rc) {
		ksmbd_err("Failed to set authsize %d\n", rc);
		return rc;
	}

	req = aead_request_alloc(tfm, GFP_KERNEL);
	if (!req) {
		ksmbd_err("Failed to alloc aead request\n");
		return -ENOMEM;
	}

	if (!enc) {
		memcpy(sign, &tr_hdr->Signature, SMB2_SIGNATURE_SIZE);
		crypt_len += SMB2_SIGNATURE_SIZE;
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 2, 0)
	sg_init_one(&assoc, iov[0].iov_base + 24, assoc_data_len);
#endif

	sg = ksmbd_init_sg(iov, nvec, sign);
	if (!sg) {
		ksmbd_err("Failed to init sg\n");
		rc = -ENOMEM;
		goto free_req;
	}

	iv_len = crypto_aead_ivsize(tfm);
	iv = kzalloc(iv_len, GFP_KERNEL);
	if (!iv) {
		ksmbd_err("Failed to alloc IV\n");
		rc = -ENOMEM;
		goto free_sg;
	}

	if (conn->cipher_type == SMB2_ENCRYPTION_AES128_GCM)
		memcpy(iv, (char *)tr_hdr->Nonce, SMB3_AES128GCM_NONCE);
	else {
		iv[0] = 3;
		memcpy(iv + 1, (char *)tr_hdr->Nonce, SMB3_AES128CCM_NONCE);
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 2, 0)
	aead_request_set_assoc(req, &assoc, assoc_data_len);
	aead_request_set_crypt(req, sg, sg, crypt_len, iv);
#else
	aead_request_set_crypt(req, sg, sg, crypt_len, iv);
	aead_request_set_ad(req, assoc_data_len);
#endif
	aead_request_set_callback(req, CRYPTO_TFM_REQ_MAY_SLEEP, NULL, NULL);

	if (enc)
		rc = crypto_aead_encrypt(req);
	else
		rc = crypto_aead_decrypt(req);
	if (!rc && enc)
		memcpy(&tr_hdr->Signature, sign, SMB2_SIGNATURE_SIZE);

	kfree(iv);
free_sg:
	kfree(sg);
free_req:
	kfree(req);
	ksmbd_release_crypto_ctx(ctx);
	return rc;
}
