/*
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _PPE_QDISC_RES_H_
#define _PPE_QDISC_RES_H_

struct ppe_qdisc;

/*
 * ppe_qdisc_res_mcast_queue_set()
 *	Allocates and configures a multicast queue in SSDK.
 */
extern int ppe_qdisc_res_mcast_queue_set(struct ppe_qdisc *pq);

/*
 * ppe_qdisc_res_queue_limit_set()
 *	Sets queue size in SSDK.
 */
extern int ppe_qdisc_res_queue_limit_set(struct ppe_qdisc *pq);

/*
 * ppe_qdisc_res_shaper_set()
 *	Configures a shaper in SSDK.
 */
extern int ppe_qdisc_res_shaper_set(struct ppe_qdisc *pq);

/*
 * ppe_qdisc_res_scheduler_set()
 *	Configures scheduler resources in SSDK.
 */
extern int ppe_qdisc_res_scheduler_set(struct ppe_qdisc *pq);

/*
 * ppe_qdisc_res_free()
 *	Frees all the scheduler and shaper resources.
 */
extern void ppe_qdisc_res_free(struct ppe_qdisc *pq);

/*
 * ppe_qdisc_res_init()
 *	Allocates and initializes PPE Qdisc resources.
 */
extern int ppe_qdisc_res_init(struct ppe_qdisc *pq);

#endif
