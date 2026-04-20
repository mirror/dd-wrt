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

enum ppe_drv_ip_type;

/*
 * ppe_drv_host_lan_wan
 *	PPE host LAN/WAN info
 */
enum ppe_drv_host_lan_wan {
	PPE_DRV_HOST_LAN,	/* Host entry stores LAN side IP */
	PPE_DRV_HOST_WAN,	/* Host entry stores WAN side IP */
};

/*
 * ppe_drv_host
 *	Host information
 */
struct ppe_drv_host {
	enum ppe_drv_ip_type type;	/* What type of IP address this host entry holds */
	struct kref ref;		/* Reference count object */
	uint16_t index;			/* Index pointed to in PPE table */
};

void ppe_drv_host_dump(struct ppe_drv_host *host);
bool ppe_drv_host_deref(struct ppe_drv_host *host);
struct ppe_drv_host *ppe_drv_host_ref(struct ppe_drv_host *host);
struct ppe_drv_host *ppe_drv_host_v4_add(struct ppe_drv_v4_conn_flow *pcf);
struct ppe_drv_host *ppe_drv_host_v6_add(struct ppe_drv_v6_conn_flow *pcf);

void ppe_drv_host_entries_free(struct ppe_drv_host *host);
struct ppe_drv_host *ppe_drv_host_entries_alloc(void);
