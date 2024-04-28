/*
 * ********************************************************************************
 * Copyright (c) 2016-2019, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 **********************************************************************************
 */

#ifndef __NSS_IPSECMGR_REF_H
#define __NSS_IPSECMGR_REF_H

struct nss_ipsecmgr_ref;

typedef void (*nss_ipsecmgr_ref_method_t)(struct nss_ipsecmgr_ref *ref);
typedef ssize_t (*nss_ipsecmgr_ref_get_method_t)(struct nss_ipsecmgr_ref *ref);
typedef ssize_t (*nss_ipsecmgr_ref_print_method_t)(struct nss_ipsecmgr_ref *ref, char *buf);

/*
 * IPsec manager reference object
 */
struct nss_ipsecmgr_ref {
	struct list_head head;			/* parent "ref" */
	struct list_head node;			/* child "ref" */

	uint32_t id;				/* identifier */
	struct nss_ipsecmgr_ref *parent;	/* reference to parent */

	nss_ipsecmgr_ref_get_method_t print_len;	/* returns the statistics size for ref */
	nss_ipsecmgr_ref_print_method_t print;		/* dumps the statistics in buffer */

	nss_ipsecmgr_ref_method_t free;		/* free function */
};

/* functions to operate on reference object */
extern ssize_t nss_ipsecmgr_ref_print_len(struct nss_ipsecmgr_ref *ref);
extern ssize_t nss_ipsecmgr_ref_print(struct nss_ipsecmgr_ref *ref, char *buf);
extern void nss_ipsecmgr_ref_del(struct nss_ipsecmgr_ref *child);
extern void nss_ipsecmgr_ref_add(struct nss_ipsecmgr_ref *child, struct nss_ipsecmgr_ref *parent);
extern void nss_ipsecmgr_ref_free(struct nss_ipsecmgr_ref *ref);
extern void nss_ipsecmgr_ref_init(struct nss_ipsecmgr_ref *ref, nss_ipsecmgr_ref_method_t free);
extern void nss_ipsecmgr_ref_init_print(struct nss_ipsecmgr_ref *ref, nss_ipsecmgr_ref_get_method_t print_len,
					nss_ipsecmgr_ref_print_method_t print);

#endif
