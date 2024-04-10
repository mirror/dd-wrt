// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2020 LG Electronics
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include "tools.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>

#include <linux/ksmbd_server.h>
#include <management/spnego.h>
#include <asn1.h>
#include "spnego_mech.h"

static struct spnego_mech_ctx mech_ctxs[SPNEGO_MAX_MECHS];

static struct spnego_mech_ctx *get_mech(int mech_type)
{
	if (mech_type >= SPNEGO_MAX_MECHS)
		return NULL;
	return &mech_ctxs[mech_type];
}

void spnego_init(void)
{
	struct spnego_mech_ctx *mech_ctx;
	int i;

	mech_ctx = &mech_ctxs[SPNEGO_MECH_MSKRB5];
	if (!mech_ctx->ops)
		mech_ctx->ops = &spnego_mskrb5_operations;
	if (!mech_ctx->params.krb5.service_name)
		mech_ctx->params.krb5.service_name =
			g_strdup(global_conf.krb5_service_name);
	if (!mech_ctx->params.krb5.keytab_name)
		mech_ctx->params.krb5.keytab_name =
			g_strdup(global_conf.krb5_keytab_file);

	mech_ctx = &mech_ctxs[SPNEGO_MECH_KRB5];
	if (!mech_ctx->ops)
		mech_ctx->ops = &spnego_krb5_operations;
	if (!mech_ctx->params.krb5.service_name)
		mech_ctx->params.krb5.service_name =
			g_strdup(global_conf.krb5_service_name);
	if (!mech_ctx->params.krb5.keytab_name)
		mech_ctx->params.krb5.keytab_name =
			g_strdup(global_conf.krb5_keytab_file);

	for (i = 0; i < SPNEGO_MAX_MECHS; i++)
		if (mech_ctxs[i].ops->setup &&
		    mech_ctxs[i].ops->setup(&mech_ctxs[i]))
			abort();
}

void spnego_destroy(void)
{
	int i;

	for (i = 0; i < SPNEGO_MAX_MECHS; i++) {
		if (mech_ctxs[i].ops && mech_ctxs[i].ops->cleanup)
			mech_ctxs[i].ops->cleanup(&mech_ctxs[i]);
	}
}

static int compare_oid(unsigned long *oid1, unsigned int oid1len,
		    unsigned long *oid2, unsigned int oid2len)
{
	unsigned int i;

	if (oid1len != oid2len)
		return 1;

	for (i = 0; i < oid1len; i++) {
		if (oid1[i] != oid2[i])
			return 1;
	}
	return 0;
}

static bool is_supported_mech(unsigned long *oid, unsigned int len,
			int *mech_type)
{
	if (!compare_oid(oid, len, MSKRB5_OID, ARRAY_SIZE(MSKRB5_OID))) {
		*mech_type = SPNEGO_MECH_MSKRB5;
		return true;
	}

	if (!compare_oid(oid, len, KRB5_OID, ARRAY_SIZE(KRB5_OID))) {
		*mech_type = SPNEGO_MECH_KRB5;
		return true;
	}

	*mech_type = SPNEGO_MAX_MECHS;
	return false;
}

static int decode_asn1_header(struct asn1_ctx *ctx, unsigned char **end,
		unsigned int cls, unsigned int con, unsigned int tag)
{
	unsigned int d_cls, d_con, d_tag;

	if (asn1_header_decode(ctx, end, &d_cls, &d_con, &d_tag) == 0 ||
		(d_cls != cls || d_con != con || d_tag != tag))
		return -EINVAL;
	return 0;
}

static int decode_negTokenInit(unsigned char *negToken, int token_len,
			int *mech_type, unsigned char **krb5_ap_req,
			unsigned int *req_len)
{
	struct asn1_ctx ctx;
	unsigned char *end, *mech_types_end, *id;
	unsigned long *oid = NULL;
	unsigned int len;

	asn1_open(&ctx, negToken, token_len);

	/* GSSAPI header */
	if (decode_asn1_header(&ctx, &end, ASN1_APL, ASN1_CON, ASN1_EOC)) {
		pr_debug("Error decoding SPNEGO application tag\n");
		return -EINVAL;
	}

	/* SPNEGO oid */
	if (decode_asn1_header(&ctx, &end, ASN1_UNI, ASN1_PRI, ASN1_OJI) ||
			asn1_oid_decode(&ctx, end, &oid, &len) == 0 ||
			compare_oid(oid, len, SPNEGO_OID, SPNEGO_OID_LEN)) {
		pr_debug("Error decoding SPNEGO OID\n");
		g_free(oid);
		return -EINVAL;
	}
	g_free(oid);

	/* negoTokenInit */
	if (decode_asn1_header(&ctx, &end, ASN1_CTX, ASN1_CON, 0) ||
			decode_asn1_header(&ctx, &end,
				ASN1_UNI, ASN1_CON, ASN1_SEQ)) {
		pr_debug("Error decoding negTokenInit tag\n");
		return -EINVAL;
	}

	/* mechTypes */
	if (decode_asn1_header(&ctx, &end, ASN1_CTX, ASN1_CON, 0) ||
			decode_asn1_header(&ctx, &end,
				ASN1_UNI, ASN1_CON, ASN1_SEQ)) {
		pr_debug("Error decoding mechTypes tag\n");
		return -EINVAL;
	}

	mech_types_end = end;
	if (decode_asn1_header(&ctx, &end, ASN1_UNI, ASN1_PRI, ASN1_OJI) ||
			asn1_oid_decode(&ctx, end, &oid, &len) == 0) {
		pr_debug("Error decoding Kerberos 5 OIDs\n");
		return -EINVAL;
	}

	if (!is_supported_mech(oid, len, mech_type)) {
		g_free(oid);
		pr_debug("Not a supported mechanism\n");
		return -EINVAL;
	}
	g_free(oid);

	ctx.pointer = mech_types_end;
	/* mechToken */
	if (decode_asn1_header(&ctx, &end, ASN1_CTX, ASN1_CON, 2) ||
			decode_asn1_header(&ctx, &end,
				ASN1_UNI, ASN1_PRI, ASN1_OTS)) {
		pr_debug("Error decoding krb5_blob\n");
		return -EINVAL;
	}

	if (decode_asn1_header(&ctx, &end, ASN1_APL, ASN1_CON, ASN1_EOC)) {
		pr_debug("Error decoding GSSAPI application tag\n");
		return -EINVAL;
	}

	/* Kerberos 5 oid */
	if (decode_asn1_header(&ctx, &end, ASN1_UNI, ASN1_PRI, ASN1_OJI)) {
		pr_debug("Error decoding Kerberos 5 OID tag\n");
		return -EINVAL;
	}

	if (asn1_oid_decode(&ctx, end, &oid, &len) == 0 ||
			compare_oid(oid, len, KRB5_OID,
				ARRAY_SIZE(KRB5_OID))) {
		pr_debug("Not a Kerberos 5 OID\n");
		g_free(oid);
		return -EINVAL;
	}
	g_free(oid);

	/* AP_REQ id */
	if (asn1_read(&ctx, &id, 2) == 0 || id[0] != 1 || id[1] != 0) {
		g_free(id);
		pr_debug("Error decoding AP_REQ ID\n");
		return -EINVAL;
	}
	g_free(id);

	/* AP_REQ */
	*req_len = (unsigned int)(ctx.end - ctx.pointer);
	*krb5_ap_req = ctx.pointer;
	return 0;
}

static int encode_negTokenTarg(char *in_blob, int in_len,
			const unsigned long *oid, int oid_len,
			char **out_blob, int *out_len)
{
	unsigned char *buf;
	unsigned char *sup_oid, *krb5_oid;
	int sup_oid_len, krb5_oid_len;
	unsigned int neg_result_len, sup_mech_len, rep_token_len, len;

	if (asn1_oid_encode(oid, oid_len, &sup_oid, &sup_oid_len))
		return -ENOMEM;
	if (asn1_oid_encode(KRB5_OID, ARRAY_SIZE(KRB5_OID),
			&krb5_oid, &krb5_oid_len)) {
		g_free(sup_oid);
		return -ENOMEM;
	}

	neg_result_len = asn1_header_len(1, 2);
	sup_mech_len = asn1_header_len(sup_oid_len, 2);
	rep_token_len = asn1_header_len(krb5_oid_len, 1);
	rep_token_len += 2 + in_len;
	rep_token_len = asn1_header_len(rep_token_len, 3);

	*out_len = asn1_header_len(
			neg_result_len + sup_mech_len + rep_token_len, 2);
	*out_blob = g_try_malloc0(*out_len);
	if (*out_blob == NULL)
		return -ENOMEM;
	buf = *out_blob;

	/* negTokenTarg */
	len = *out_len;
	asn1_header_encode(&buf,
			ASN1_CTX, ASN1_CON, 1,
			&len);
	asn1_header_encode(&buf,
			ASN1_UNI, ASN1_CON, ASN1_SEQ,
			&len);

	/* negTokenTarg/negResult */
	len = neg_result_len;
	asn1_header_encode(&buf,
			ASN1_CTX, ASN1_CON, 0,
			&len);
	asn1_header_encode(&buf,
			ASN1_UNI, ASN1_PRI, ASN1_ENUM,
			&len);
	*buf++ = 0;

	/* negTokenTarg/supportedMechType */
	len = sup_mech_len;
	asn1_header_encode(&buf,
			ASN1_CTX, ASN1_CON, 1,
			&len);
	asn1_header_encode(&buf,
			ASN1_UNI, ASN1_PRI, ASN1_OJI,
			&len);
	memcpy(buf, sup_oid, sup_oid_len);
	buf += len;

	/* negTokenTarg/responseToken */
	len = rep_token_len;
	asn1_header_encode(&buf,
			ASN1_CTX, ASN1_CON, 2,
			&len);
	asn1_header_encode(&buf,
			ASN1_UNI, ASN1_PRI, ASN1_OTS,
			&len);
	asn1_header_encode(&buf,
			ASN1_APL, ASN1_CON, 0,
			&len);
	/* negTokenTarg/responseToken/OID */
	len = asn1_header_len(krb5_oid_len, 1);
	asn1_header_encode(&buf,
			ASN1_UNI, ASN1_PRI, ASN1_OJI,
			&len);
	/* negTokenTarg/responseToken/mechToken*/
	memcpy(buf, krb5_oid, krb5_oid_len);
	buf += len;
	/* AP_REP id */
	*buf++ = 2;
	*buf++ = 0;
	memcpy(buf, in_blob, in_len);

	g_free(sup_oid);
	g_free(krb5_oid);
}

int spnego_handle_authen_request(struct ksmbd_spnego_authen_request *req,
			struct ksmbd_spnego_auth_out *auth_out)
{
	struct spnego_mech_ctx *mech_ctx;
	unsigned char *mech_token;
	int token_len, mech_type;
	int retval = 0;

	if (decode_negTokenInit(req->spnego_blob, (int)req->spnego_blob_len,
				&mech_type, &mech_token, &token_len)) {
		pr_info("Error decoding negTokenInit\n");
		return -EINVAL;
	}

	mech_ctx = get_mech(mech_type);
	if (!mech_ctx) {
		retval = -ENOTSUP;
		pr_info("No support for Kerberos 5\n");
		goto out;
	}

	if (mech_ctx->ops->handle_authen(mech_ctx,
				mech_token, token_len,
				auth_out, encode_negTokenTarg)) {
		retval = -EPERM;
		pr_info("Error authenticating\n");
		goto out;
	}
out:
	return retval;
}
