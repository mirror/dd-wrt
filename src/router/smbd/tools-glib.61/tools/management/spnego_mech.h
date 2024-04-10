/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2020 LG Electronics
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#ifndef _SPNEGO_MECH_H_
#define _SPNEGO_MECH_H_

enum {
	SPNEGO_MECH_MSKRB5	= 0,
	SPNEGO_MECH_KRB5,
	SPNEGO_MAX_MECHS,
};

struct spnego_mech_ctx;

typedef int (*spnego_encode_t)(char *in_blob, int in_len,
				const unsigned long *oid, int oid_len,
				char **out_blob, int *out_len);

struct spnego_mech_operations {
	int (*setup)(struct spnego_mech_ctx *mech_ctx);
	void (*cleanup)(struct spnego_mech_ctx *mech_ctx);
	int (*handle_authen)(struct spnego_mech_ctx *mech_ctx,
				char *in_blob, unsigned int in_len,
				struct ksmbd_spnego_auth_out *auth_out,
				spnego_encode_t encode);
};

struct spnego_mech_ctx {
	const unsigned long	*oid;
	int			oid_len;
	void			*private;
	union {
		struct {
			void *keytab_name;
			void *service_name;
		} krb5;
	} params;
	struct spnego_mech_operations *ops;
};

extern struct spnego_mech_operations spnego_krb5_operations;
extern struct spnego_mech_operations spnego_mskrb5_operations;

#endif
