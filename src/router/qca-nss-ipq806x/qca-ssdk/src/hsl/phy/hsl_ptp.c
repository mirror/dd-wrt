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

#include "fal_ptp.h"
#include "qca808x.h"

void hsl_ptp_event_stat_update(hsl_ptp_event_pkt_stat_t *pkt_stat,
		a_int32_t msg_type, a_int32_t seqid_matched)
{
	if (pkt_stat == NULL ||
			(seqid_matched < PTP_PKT_SEQID_CHECKED) ||
			(seqid_matched >= PTP_PKT_SEQID_MATCH_MAX)) {
		return;
	}

	switch (msg_type) {
		case PTP_MSG_SYNC:
			pkt_stat->sync_cnt[seqid_matched]++;
			break;
		case PTP_MSG_DREQ:
			pkt_stat->delay_req_cnt[seqid_matched]++;
			break;
		case PTP_MSG_PREQ:
			pkt_stat->pdelay_req_cnt[seqid_matched]++;
			break;
		case PTP_MSG_PRESP:
			pkt_stat->pdelay_resp_cnt[seqid_matched]++;
			break;
		default:
			SSDK_DEBUG("%s: msg %x is not event frame\n",
					__func__, msg_type);
	}
}

static void hsl_ptp_event_stat_get(hsl_ptp_event_pkt_stat_t *pkt_stat,
		char *buf, ssize_t *count)
{
	if (!pkt_stat) {
		return;
	}

	/* stat for the ptp event packet handled by phc driver */
	*count += snprintf(buf + *count, PAGE_SIZE, "%-30s = %lld\n", "Sync_check_pkts",
			pkt_stat->sync_cnt[PTP_PKT_SEQID_CHECKED]);
	*count += snprintf(buf + *count, PAGE_SIZE, "%-30s = %lld\n", "DelayReq_check_pkts",
			pkt_stat->delay_req_cnt[PTP_PKT_SEQID_CHECKED]);
	*count += snprintf(buf + *count, PAGE_SIZE, "%-30s = %lld\n", "PdelayReq_check_pkts",
			pkt_stat->pdelay_req_cnt[PTP_PKT_SEQID_CHECKED]);
	*count += snprintf(buf + *count, PAGE_SIZE, "%-30s = %lld\n\n", "PdelayResp_check_pkts",
			pkt_stat->pdelay_resp_cnt[PTP_PKT_SEQID_CHECKED]);

	/* stat for the ptp event packet with timestamp matched by phc driver */
	*count += snprintf(buf + *count, PAGE_SIZE, "%-30s = %lld\n", "Sync_match_pkts",
			pkt_stat->sync_cnt[PTP_PKT_SEQID_MATCHED]);
	*count += snprintf(buf + *count, PAGE_SIZE, "%-30s = %lld\n", "DelayReq_match_pkts",
			pkt_stat->delay_req_cnt[PTP_PKT_SEQID_MATCHED]);
	*count += snprintf(buf + *count, PAGE_SIZE, "%-30s = %lld\n", "PdelayReq_match_pkts",
			pkt_stat->pdelay_req_cnt[PTP_PKT_SEQID_MATCHED]);
	*count += snprintf(buf + *count, PAGE_SIZE, "%-30s = %lld\n\n", "PdelayResp_match_pkts",
			pkt_stat->pdelay_resp_cnt[PTP_PKT_SEQID_MATCHED]);

	/* stat for the ptp event packet with timestamp unmatched by phc driver */
	*count += snprintf(buf + *count, PAGE_SIZE, "%-30s = %lld\n", "Sync_unmatch_pkts",
			pkt_stat->sync_cnt[PTP_PKT_SEQID_UNMATCHED]);
	*count += snprintf(buf + *count, PAGE_SIZE, "%-30s = %lld\n", "DelayReq_unmatch_pkts",
			pkt_stat->delay_req_cnt[PTP_PKT_SEQID_UNMATCHED]);
	*count += snprintf(buf + *count, PAGE_SIZE, "%-30s = %lld\n", "PdelayReq_unmatch_pkts",
			pkt_stat->pdelay_req_cnt[PTP_PKT_SEQID_UNMATCHED]);
	*count += snprintf(buf + *count, PAGE_SIZE, "%-30s = %lld\n\n", "PdelayResp_unmatch_pkts",
			pkt_stat->pdelay_resp_cnt[PTP_PKT_SEQID_UNMATCHED]);
}

static void hsl_ptp_event_stat_set(hsl_ptp_event_pkt_stat_t *pkt_stat)
{
	if (!pkt_stat) {
		return;
	}
	memset(pkt_stat, 0, sizeof(hsl_ptp_event_pkt_stat_t));
}

static int hsl_ptp_event_stat_operation_callback(struct device *dev, void *data)
{
	void *priv;
	u32 phy_id = 0, i = 1;
	ssize_t count = 0;
	int phy_addr = -1;
	char *buf = (char *)data;
	hsl_ptp_event_pkt_stat_t *pkt_stat = NULL;
	struct phy_device *phydev = to_phy_device(dev);

	priv = phydev->priv;

	if (!priv) {
		return 0;
	}

	if (phydev->is_c45) {
		while (i < ARRAY_SIZE(phydev->c45_ids.device_ids)) {
			if (phydev->c45_ids.devices_in_package & (1 << i)) {
				phy_id = phydev->c45_ids.device_ids[i];
				break;
			}
			i++;
		}
	} else {
		phy_id = phydev->phy_id;
	}

	switch (phy_id) {
		case QCA8081_PHY_V1_1:
		case QCA8084_PHY:
			pkt_stat = ((qca808x_priv *)priv)->ptp_event_stat;
			break;
		default:
			SSDK_ERROR("PHY ID 0x%.8x not supported", phydev->phy_id);
			return 0;
	}

	if (!pkt_stat) {
		return 0;
	}

	if (!strncmp(buf, "set", 3)) {
		hsl_ptp_event_stat_set(&pkt_stat[FAL_RX_DIRECTION]);
		hsl_ptp_event_stat_set(&pkt_stat[FAL_TX_DIRECTION]);
	} else {
		sscanf(buf + PAGE_SIZE - 5, "%zd", &count);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 0))
		phy_addr = phydev->addr;
#else
		phy_addr = phydev->mdio.addr;
#endif
		count += snprintf(buf + count, PAGE_SIZE,
				"PHY [%#x] PTP event packet statistics:\n", phy_addr);
		count += snprintf(buf + count, PAGE_SIZE,
				"****************** RX direction ******************\n");
		hsl_ptp_event_stat_get(&pkt_stat[FAL_RX_DIRECTION], buf, &count);

		count += snprintf(buf + count, PAGE_SIZE,
				"****************** TX direction ******************\n");
		hsl_ptp_event_stat_get(&pkt_stat[FAL_TX_DIRECTION], buf, &count);

		if (count > PAGE_SIZE - 5) {
			count = -ENOMEM;
		}
		snprintf(buf + PAGE_SIZE - 5, 5, "%zd", count);
	}

	return count < 0 ? count: 0;
}

int hsl_ptp_event_stat_operation(char *phy_driver_name, char *buf)
{
	struct device_driver *drv = driver_find(phy_driver_name, &mdio_bus_type);

	if (!drv) {
		SSDK_DEBUG("phy driver %s is not registered", phy_driver_name);
		return 0;
	}

	return driver_for_each_device(drv, NULL, buf, hsl_ptp_event_stat_operation_callback);
}
