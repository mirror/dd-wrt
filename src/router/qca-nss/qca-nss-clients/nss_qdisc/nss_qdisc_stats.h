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
struct nss_qdisc;
struct nss_qdisc_stats_wq;

void nss_qdisc_stats_work_queue_exit(void);
bool nss_qdisc_stats_work_queue_init(void);
void nss_qdisc_stats_sync_many_exit(struct nss_qdisc *nq);
bool nss_qdisc_stats_sync_many_init(struct nss_qdisc *nq);
void nss_qdisc_stats_stop_polling(struct nss_qdisc *nq);
void nss_qdisc_stats_start_polling(struct nss_qdisc *nq);
void nss_qdisc_stats_qdisc_detach(struct nss_qdisc *nq);
void nss_qdisc_stats_qdisc_attach(struct nss_qdisc *nq);
