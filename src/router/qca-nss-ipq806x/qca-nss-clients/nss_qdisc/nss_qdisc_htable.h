/*
 **************************************************************************
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

struct hlist_head;
struct nss_qdisc;

/*
 * struct nss_qdisc_htable
 */
struct nss_qdisc_htable {
	struct hlist_head *htable;
	uint32_t count;
	uint32_t htsize;
	spinlock_t lock;
};

void nss_qdisc_htable_resize(struct Qdisc *sch, struct nss_qdisc_htable *nqt);
bool nss_qdisc_htable_init(struct nss_qdisc_htable *nqt);
void nss_qdisc_htable_dealloc(struct nss_qdisc_htable *nqt);
void nss_qdisc_htable_entry_add(struct nss_qdisc_htable *nqt, struct nss_qdisc *nq);
void nss_qdisc_htable_entry_del(struct nss_qdisc_htable *nqt, struct nss_qdisc *nq);
struct nss_qdisc *nss_qdisc_htable_entry_lookup(struct nss_qdisc_htable *nqt, u32 qos_tag);
