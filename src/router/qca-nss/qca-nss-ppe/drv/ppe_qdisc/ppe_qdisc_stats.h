/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 */

struct ppe_qdisc;
struct ppe_qdisc_stats_wq;

void ppe_qdisc_stats_work_queue_exit(void);
bool ppe_qdisc_stats_work_queue_init(void);
void ppe_qdisc_stats_sync_many_exit(struct ppe_qdisc *pq);
bool ppe_qdisc_stats_sync_many_init(struct ppe_qdisc *pq);
void ppe_qdisc_stats_stop_polling(struct ppe_qdisc *pq);
void ppe_qdisc_stats_start_polling(struct ppe_qdisc *pq);
void ppe_qdisc_stats_qdisc_detach(struct ppe_qdisc *pq);
void ppe_qdisc_stats_qdisc_attach(struct ppe_qdisc *pq);
