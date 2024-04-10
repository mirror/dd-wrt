/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 *   Copyright (C) 2020 LG Electronics
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#ifndef _MANAGEMENT_SPNEGO_H_
#define _MANAGEMENT_SPNEGO_H_

struct ksmbd_spnego_auth_out {
	char		*spnego_blob;
	unsigned int	blob_len;
	char		*sess_key;
	unsigned int	key_len;
	char		*user_name;
};

struct ksmbd_spnego_authen_request;

#ifdef CONFIG_KRB5
void spnego_init(void);
void spnego_destroy(void);
int spnego_handle_authen_request(struct ksmbd_spnego_authen_request *req,
				struct ksmbd_spnego_auth_out *auth_out);
#else
static inline void spnego_init(void) {}
static inline void spnego_destroy(void) {}
static inline int spnego_handle_authen_request(struct ksmbd_spnego_authen_request *req,
				struct ksmbd_spnego_auth_out *auth_out)
{
	return -ENOTSUP;
}
#endif

#endif
