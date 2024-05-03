/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

/*
 * nss_data_plane_register()
 *	Register data plane
 */
extern void nss_data_plane_lite_register(struct nss_ctx_instance *nss_ctx);

/*
 * nss_data_plane_unregister()
 *	Unregister data plane.
 */
extern void nss_data_plane_lite_unregister(void);

/*
 * nss_data_plane_lite_schedule_registration()
 *	Called from nss_init to schedule a work to do data_plane_lite register to data plane host driver
 */
extern bool nss_data_plane_lite_schedule_registration(void);

/*
 * nss_data_plane_lite_init_delay_work()
 *	Initialize data_plane_lite workqueue
 */
extern int nss_data_plane_lite_init_delay_work(void);

/*
 * nss_data_plane_lite_destroy_delay_work()
 *	Destroy data_plane_lite workqueue
 */
extern void nss_data_plane_lite_destroy_delay_work(void);
