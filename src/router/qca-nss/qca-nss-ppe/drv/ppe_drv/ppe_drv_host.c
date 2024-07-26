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

#include <linux/netdevice.h>
#include <linux/if_ether.h>
#include <fal/fal_ip.h>
#include "ppe_drv.h"

#if (PPE_DRV_DEBUG_LEVEL == 3)
/*
 * ppe_drv_host_dump()
 *	Read and print the host table content.
 */
void ppe_drv_host_dump(struct ppe_drv_host *host)
{
	fal_host_entry_t host_cfg = {0};
	sw_error_t err;

	host_cfg.entry_id = host->index;
	err = fal_ip_host_get(PPE_DRV_SWITCH_ID, FAL_IP_ENTRY_ID_EN, &host_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p, failed to get host entry configuration", host);
		return;
	}

	ppe_drv_trace("%p: ipv4_addr: %pI4", host, &host_cfg.ip4_addr);
	ppe_drv_trace("%p: entry type: %d", host, host_cfg.flags);
	ppe_drv_trace("%p: action: %d", host, host_cfg.action);
	ppe_drv_trace("%p: lan_wan direction: %d", host, host_cfg.lan_wan);
	ppe_drv_trace("%p: valid: %d", host, host_cfg.status);
}
#else
void ppe_drv_host_dump(struct ppe_drv_host *host)
{
}
#endif

/*
 * ppe_drv_host_delete()
 *	Destroy host tbl entry.
 */
static void ppe_drv_host_delete(struct kref *kref)
{
	struct ppe_drv_host *host = container_of(kref, struct ppe_drv_host, ref);
	fal_host_entry_t host_cfg = {0};
	sw_error_t err;

	host_cfg.entry_id = host->index;

	if (host->type == PPE_DRV_IP_TYPE_V4) {
		host_cfg.flags |= FAL_IP_IP4_ADDR;
	} else if (host->type == PPE_DRV_IP_TYPE_V6) {
		host_cfg.flags |= FAL_IP_IP6_ADDR;
	} else if (host->type == PPE_DRV_IP_TYPE_MC_V4) {
		host_cfg.flags |= FAL_IP_IP4_ADDR_MCAST;
	} else {
		host_cfg.flags |= FAL_IP_IP6_ADDR_MCAST;
	}

	err = fal_ip_host_del(PPE_DRV_SWITCH_ID, FAL_IP_ENTRY_ID_EN, &host_cfg);
	if (err != SW_OK) {
		/*
		 * TODO: add stats
		 */
		ppe_drv_warn("%p: host entry deletion failed for index: %d", host, host->index);
		return;
	}

	host->type = 0;
	ppe_drv_trace("%p: Destroy request for host entry idx: %u", host, host->index);
}

/*
 * ppe_drv_host_v6_add()
 *	Add host tbl entry.
 */
struct ppe_drv_host *ppe_drv_host_v6_add(struct ppe_drv_v6_conn_flow *pcf)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_host_entry_t host_cfg = {0};
	struct ppe_drv_host *host;
	uint32_t addr[4];
	sw_error_t err;

	ppe_drv_v6_conn_flow_match_src_ip_get(pcf, addr);
	memcpy(host_cfg.ip6_addr.ul, addr, sizeof(addr));
	host_cfg.flags = FAL_IP_IP6_ADDR;
	host_cfg.action = FAL_MAC_FRWRD;
	host_cfg.lan_wan = PPE_DRV_HOST_LAN;
	host_cfg.status = PPE_DRV_ENTRY_VALID;
	err = fal_ip_host_add(PPE_DRV_SWITCH_ID, &host_cfg);
	if (err != SW_OK) {
		ppe_drv_stats_inc(&p->stats.gen_stats.v6_host_add_fail);
		ppe_drv_warn("%p, host entry addition failed for IP: %pI6", pcf, &host_cfg.ip6_addr);
		return NULL;
	}

	/*
	 * retrieve host index
	 */
	host = &p->host[host_cfg.entry_id];

	if (!kref_read(&host->ref)) {
		kref_init(&host->ref);
		host->type = PPE_DRV_IP_TYPE_V6;
	} else {
		ppe_drv_host_ref(host);
	}

	return host;
}

/*
 * ppe_drv_host_v4_add()
 *	Add host tbl entry.
 */
struct ppe_drv_host *ppe_drv_host_v4_add(struct ppe_drv_v4_conn_flow *pcf)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_host_entry_t host_cfg = {0};
	struct ppe_drv_host *host;
	sw_error_t err;

	host_cfg.ip4_addr = ppe_drv_v4_conn_flow_match_src_ip_get(pcf);
	host_cfg.flags = FAL_IP_IP4_ADDR;
	host_cfg.action = FAL_MAC_FRWRD;
	host_cfg.lan_wan = PPE_DRV_HOST_LAN;
	host_cfg.status = PPE_DRV_ENTRY_VALID;

	err = fal_ip_host_add(PPE_DRV_SWITCH_ID, &host_cfg);
	if (err != SW_OK) {
		ppe_drv_stats_inc(&p->stats.gen_stats.v4_host_add_fail);
		ppe_drv_warn("%p, host entry addition failed for IP: %pI4", pcf, &host_cfg.ip4_addr);
		return NULL;
	}

	/*
	 * retrieve host index
	 */
	host = &p->host[host_cfg.entry_id];

	if (!kref_read(&host->ref)) {
		kref_init(&host->ref);
		host->type = PPE_DRV_IP_TYPE_V4;
	} else {
		ppe_drv_host_ref(host);
	}

	return host;
}

/*
 * ppe_drv_host_deref()
 *	Decrements ref count for host table entry and destroys it if refs = 0.
 */
bool ppe_drv_host_deref(struct ppe_drv_host *host)
{
	if (kref_put(&host->ref, ppe_drv_host_delete)) {
		ppe_drv_trace("reference goes down to 0 for host: %p\n", host);
		return true;
	}

	return false;
}

/*
 * ppe_drv_host_ref()
 *	Increments ref count for host table.
 */
struct ppe_drv_host *ppe_drv_host_ref(struct ppe_drv_host *host)
{
	kref_get(&host->ref);
	return host;
}

/*
 * ppe_drv_host_entries_free()
 *	Free host table entries if it was allocated.
 */
void ppe_drv_host_entries_free(struct ppe_drv_host *host)
{
	vfree(host);
}

/*
 * ppe_drv_host_entries_alloc()
 *	Allocated and initialize the host entries.
 */
struct ppe_drv_host *ppe_drv_host_entries_alloc()
{
	uint16_t i;
	struct ppe_drv_host *host;
	struct ppe_drv *p = &ppe_drv_gbl;

	host = vzalloc(sizeof(struct ppe_drv_host) * p->host_num);
	if (!host) {
		ppe_drv_warn("%p: failed to allocate host entries", p);
		return NULL;
	}

	for (i = 0; i < p->host_num; i++) {
		host[i].index = i;
	}

	return host;
}
