// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2020 LG Electronics
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include "ksmbdtools.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <krb5.h>

#include <management/spnego.h>
#include <asn1.h>
#include "spnego_mech.h"

struct spnego_krb5_ctx {
	krb5_context	context;
	krb5_keytab	keytab;
	krb5_creds	creds;
};

#define SERVICE_NAME	"cifs"

#define pr_krb5_err(_context, _retval, _fmt, ...)		\
	do {							\
		const char *msg = krb5_get_error_message(_context, _retval);	\
		pr_err("%s: " _fmt, msg, ##__VA_ARGS__);	\
		krb5_free_error_message(_context, msg);		\
	} while (0)

static char *get_service_name(void)
{
	return strdup(SERVICE_NAME);
}

static char *get_host_name(void)
{
	struct addrinfo hint, *ai;
	char *host_name;
	char hostname[NI_MAXHOST];

	if (gethostname(hostname, sizeof(hostname)))
		return NULL;

	memset(&hint, 0, sizeof(hint));
	hint.ai_family = AF_UNSPEC;
	hint.ai_flags = AI_CANONNAME | AI_ADDRCONFIG;
	if (getaddrinfo(hostname, NULL, &hint, &ai))
		return NULL;

	host_name = strdup(ai->ai_canonname);
	freeaddrinfo(ai);
	return host_name;
}

/* Service full name is <service name>[/<host FQDN>[@REALM>]] */
static int parse_service_full_name(char *service_full_name,
			char **service_name,
			char **host_name)
{
	char *name, *delim;

	*service_name = NULL;
	*host_name = NULL;

	if (!service_full_name) {
		*service_name = get_service_name();
		*host_name = get_host_name();
		goto out;
	}

	name = service_full_name;
	delim = strchr(name, '/');
	if (!delim) {
		*service_name = strdup(name);
		*host_name = get_host_name();
		goto out;
	}
	*service_name = strndup(name, delim - name);
	if (*service_name == NULL)
		return -ENOMEM;

	name = delim + 1;
	delim = strchr(name, '@');
	if (!delim) {
		*host_name = strdup(name);
		goto out;
	}
	*host_name = strndup(name, delim - name);
	if (*host_name == NULL) {
		free(*service_name);
		return -ENOMEM;
	}
out:
	/* we assume the host name is FQDN if it has "." */
	if (strchr(*host_name, '.'))
		return 0;

	free(*service_name);
	free(*host_name);
	*service_name = NULL;
	*host_name = NULL;
	return -EINVAL;
}

static krb5_error_code acquire_creds_from_keytab(krb5_context context,
		char *service_full_name, char *keytab_name,
		krb5_creds *out_creds, krb5_keytab *keytab)
{
	krb5_error_code retval;
	krb5_principal sprinc = NULL;
	char *host_name = NULL, *service_name = NULL;

	if (keytab_name)
		retval = krb5_kt_resolve(context, keytab_name, keytab);
	else
		retval = krb5_kt_default(context, keytab);
	if (retval) {
		pr_krb5_err(context, retval, "while resolving keytab\n");
		return retval;
	}

	if (parse_service_full_name(service_full_name,
				&service_name, &host_name)) {
		retval = KRB5_ERR_HOST_REALM_UNKNOWN;
		pr_krb5_err(context, retval, "while getting host name\n");
		goto out_err;
	}

	retval = krb5_sname_to_principal(context, host_name, service_name,
			KRB5_NT_UNKNOWN, &sprinc);
	if (retval) {
		pr_krb5_err(context, retval, "while generating service name\n");
		goto out_err;
	}

	retval = krb5_get_init_creds_keytab(context, out_creds, sprinc,
			*keytab, 0, NULL, NULL);
	if (retval) {
		char *name;

		krb5_unparse_name(context, sprinc, &name);
		pr_krb5_err(context, retval,
			"while getting credentails for %s\n", name);
		krb5_free_unparsed_name(context, name);
		goto out_err;
	}

	free(host_name);
	free(service_name);
	return 0;
out_err:
	if (sprinc)
		krb5_free_principal(context, sprinc);
	if (service_name)
		free(service_name);
	if (host_name)
		free(host_name);
	if (*keytab)
		krb5_kt_close(context, *keytab);
	return retval;
}

static int handle_krb5_authen(struct spnego_mech_ctx *mech_ctx,
			char *in_blob, unsigned int in_len,
			struct ksmbd_spnego_auth_out *auth_out,
			spnego_encode_t spnego_encode)
{
	struct spnego_krb5_ctx *krb5_ctx;
	char *client_name;
	krb5_auth_context auth_context;
	krb5_data packet, ap_rep;
	krb5_ticket *ticket = NULL;
	krb5_keyblock *session_key;
	krb5_authenticator *authenti;
	int retval = -EINVAL;
	krb5_error_code krb_retval;

	krb5_ctx = (struct spnego_krb5_ctx *)mech_ctx->private;
	if (!krb5_ctx)
		return -EINVAL;

	krb_retval = krb5_auth_con_init(krb5_ctx->context, &auth_context);
	if (krb_retval) {
		pr_krb5_err(krb5_ctx->context, krb_retval,
				"while initailzing auth context\n");
		return -EINVAL;
	}

	packet.length = in_len;
	packet.data = (krb5_pointer)in_blob;
	krb_retval = krb5_rd_req(krb5_ctx->context, &auth_context, &packet,
				krb5_ctx->creds.client, krb5_ctx->keytab,
				NULL, &ticket);
	if (krb_retval) {
		char *name;

		krb5_unparse_name(krb5_ctx->context, krb5_ctx->creds.client,
				&name);
		krb5_auth_con_free(krb5_ctx->context, auth_context);
		pr_krb5_err(krb5_ctx->context, krb_retval,
			"while decoding AP_REQ with %s creds\n", name);
		krb5_free_unparsed_name(krb5_ctx->context, name);
		return -EINVAL;
	}

	krb_retval = krb5_auth_con_getsendsubkey(krb5_ctx->context,
				auth_context, &session_key);
	if (krb_retval) {
		pr_krb5_err(krb5_ctx->context, krb_retval,
				"while reading session key\n");
		goto out_free_con_auth;
	}

	krb_retval = krb5_mk_rep(krb5_ctx->context, auth_context, &ap_rep);
	if (krb_retval) {
		pr_krb5_err(krb5_ctx->context, krb_retval,
				"while making AP_REP\n");
		goto out_free_key;
	}

	krb_retval = krb5_auth_con_getauthenticator(krb5_ctx->context,
				auth_context, &authenti);
	if (krb_retval) {
		pr_krb5_err(krb5_ctx->context, krb_retval,
				"while getting authenticator\n");
		goto out_free_rep;
	}

	krb_retval = krb5_unparse_name_flags(krb5_ctx->context,
				authenti->client,
				KRB5_PRINCIPAL_UNPARSE_NO_REALM, &client_name);
	if (krb_retval) {
		pr_krb5_err(krb5_ctx->context, krb_retval,
				"while unparsing client name\n");
		goto out_free_auth;
	}

	memset(auth_out, 0, sizeof(*auth_out));
	auth_out->user_name = strdup(client_name);
	if (!auth_out->user_name) {
		krb5_free_unparsed_name(krb5_ctx->context, client_name);
		retval = -ENOMEM;
		goto out_free_auth;
	}
	krb5_free_unparsed_name(krb5_ctx->context, client_name);

	auth_out->sess_key = malloc(session_key->length);
	if (!auth_out->sess_key) {
		free(auth_out->user_name);
		retval = -ENOMEM;
		goto out_free_auth;
	}
	memcpy(auth_out->sess_key, session_key->contents, session_key->length);
	auth_out->key_len = session_key->length;

	if (spnego_encode(ap_rep.data, ap_rep.length,
			mech_ctx->oid, mech_ctx->oid_len,
			&auth_out->spnego_blob, &auth_out->blob_len)) {
		free(auth_out->user_name);
		free(auth_out->sess_key);
		goto out_free_auth;
	}

	pr_info("Succeeded to authenticate %s\n", auth_out->user_name);
	retval = 0;

out_free_auth:
	krb5_free_authenticator(krb5_ctx->context, authenti);
out_free_rep:
	krb5_free_data_contents(krb5_ctx->context, &ap_rep);
out_free_key:
	krb5_free_keyblock(krb5_ctx->context, session_key);
out_free_con_auth:
	krb5_free_ticket(krb5_ctx->context, ticket);
	krb5_auth_con_free(krb5_ctx->context, auth_context);
	return retval;
}

static int setup_krb5_ctx(struct spnego_mech_ctx *mech_ctx)
{
	struct spnego_krb5_ctx *krb5_ctx;
	krb5_error_code krb_retval;

	krb5_ctx = calloc(1, sizeof(*krb5_ctx));
	if (!krb5_ctx)
		return -ENOMEM;

	krb_retval = krb5_init_context(&krb5_ctx->context);
	if (krb_retval) {
		free(krb5_ctx);
		pr_err("while initializing krb5 context");
		return -EINVAL;
	}

	krb_retval = acquire_creds_from_keytab(krb5_ctx->context,
			mech_ctx->params.krb5.service_name,
			mech_ctx->params.krb5.keytab_name,
			&krb5_ctx->creds, &krb5_ctx->keytab);
	if (krb_retval) {
		krb5_free_context(krb5_ctx->context);
		free(krb5_ctx);
		return -EINVAL;
	}

	mech_ctx->private = krb5_ctx;
	return 0;
}

static int setup_krb5(struct spnego_mech_ctx *mech_ctx)
{
	mech_ctx->oid = KRB5_OID;
	mech_ctx->oid_len = ARRAY_SIZE(KRB5_OID);
	return setup_krb5_ctx(mech_ctx);
}

static int setup_mskrb5(struct spnego_mech_ctx *mech_ctx)
{
	mech_ctx->oid = MSKRB5_OID;
	mech_ctx->oid_len = ARRAY_SIZE(MSKRB5_OID);
	return setup_krb5_ctx(mech_ctx);
}

static void cleanup_krb5(struct spnego_mech_ctx *mech_ctx)
{
	if (mech_ctx->private) {
		struct spnego_krb5_ctx *krb5_ctx;

		krb5_ctx = (struct spnego_krb5_ctx *)mech_ctx->private;
		krb5_free_cred_contents(krb5_ctx->context, &krb5_ctx->creds);
		krb5_kt_close(krb5_ctx->context, krb5_ctx->keytab);
		krb5_free_context(krb5_ctx->context);
		free(krb5_ctx);
		mech_ctx->private = NULL;
	}
	if (mech_ctx->params.krb5.service_name)
		free(mech_ctx->params.krb5.service_name);
	if (mech_ctx->params.krb5.keytab_name)
		free(mech_ctx->params.krb5.keytab_name);
}

struct spnego_mech_operations spnego_krb5_operations = {
	.setup		= setup_krb5,
	.cleanup	= cleanup_krb5,
	.handle_authen	= handle_krb5_authen,
};

struct spnego_mech_operations spnego_mskrb5_operations = {
	.setup		= setup_mskrb5,
	.cleanup	= cleanup_krb5,
	.handle_authen	= handle_krb5_authen,
};
