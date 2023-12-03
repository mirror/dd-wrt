/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2020 embedd.ch 
 *   Copyright (C) 2020 Felix Fietkau <nbd@nbd.name> 
 *   Copyright (C) 2020 John Crispin <john@phrozen.org> 
 */

#define _GNU_SOURCE
#include <linux/if_ether.h>
#include <net/if.h>


#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#include <linux/nl80211.h>
#include <unl.h>

#include "usteer.h"
#include "node.h"

static struct unl unl;
static struct nlattr *tb[NL80211_ATTR_MAX + 1];

struct nl80211_survey_req {
	void (*cb)(void *priv, struct usteer_survey_data *d);
	void *priv;
	int nosurvey;
};

struct nl80211_scan_req {
	void (*cb)(void *priv, struct usteer_scan_result *r);
	void *priv;
};

struct nl80211_freqlist_req {
	void (*cb)(void *priv, struct usteer_freq_data *f);
	void *priv;
};

static int nl80211_survey_result(struct nl_msg *msg, void *arg)
{
	static struct nla_policy survey_policy[NL80211_SURVEY_INFO_MAX + 1] = {
		[NL80211_SURVEY_INFO_FREQUENCY] = { .type = NLA_U32 },
		[NL80211_SURVEY_INFO_NOISE] = { .type = NLA_U8 },
		[NL80211_SURVEY_INFO_CHANNEL_TIME] = { .type = NLA_U64 },
		[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY] = { .type = NLA_U64 },
	};
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *tb_s[NL80211_SURVEY_INFO_MAX + 1];
	struct nl80211_survey_req *req = arg;
	struct usteer_survey_data data = {};
	struct genlmsghdr *gnlh;

	gnlh = nlmsg_data(nlmsg_hdr(msg));
	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[NL80211_ATTR_SURVEY_INFO])
		return NL_SKIP;

	if (nla_parse_nested(tb_s, NL80211_SURVEY_INFO_MAX,
			     tb[NL80211_ATTR_SURVEY_INFO], survey_policy))
		return NL_SKIP;

	if (!tb_s[NL80211_SURVEY_INFO_FREQUENCY])
		return NL_SKIP;

	data.freq = nla_get_u32(tb_s[NL80211_SURVEY_INFO_FREQUENCY]);

	if (tb_s[NL80211_SURVEY_INFO_NOISE])
		data.noise = (int8_t) nla_get_u8(tb_s[NL80211_SURVEY_INFO_NOISE]);

	if (tb_s[NL80211_SURVEY_INFO_CHANNEL_TIME] &&
	    tb_s[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY]) {
		data.time = nla_get_u64(tb_s[NL80211_SURVEY_INFO_CHANNEL_TIME]);
		data.time_busy = nla_get_u64(tb_s[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY]);
	}
	req->cb(req->priv, &data);
	return NL_SKIP;
}

static void nl80211_get_survey(struct usteer_node *node, void *priv,
			       void (*cb)(void *priv, struct usteer_survey_data *d))
{
	struct usteer_local_node *ln = container_of(node, struct usteer_local_node, node);
	struct nl80211_survey_req req = {
		.priv = priv,
		.cb = cb,
	};
	struct nl_msg *msg;

	if (!ln->nl80211.present)
		return;

	msg = unl_genl_msg(&unl, NL80211_CMD_GET_SURVEY, true);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, ln->ifindex);
	unl_genl_request(&unl, msg, nl80211_survey_result, &req);

nla_put_failure:
	return;
}

static void nl80211_update_node_result(void *priv, struct usteer_survey_data *d)
{
	struct usteer_local_node *ln = priv;
	uint32_t delta = 0, delta_busy = 0;

	if (d->freq != ln->node.freq)
		return;

	if (d->noise)
		ln->node.noise = d->noise;

	if (ln->time) {
		delta = d->time - ln->time;
		delta_busy = d->time_busy - ln->time_busy;
	}

	ln->time = d->time;
	ln->time_busy = d->time_busy;

	if (delta) {
		float cur = (100 * delta_busy) / delta;

		if (ln->load_ewma < 0)
			ln->load_ewma = cur;
		else
			ln->load_ewma = 0.85 * ln->load_ewma + 0.15 * cur;
	}
	if (ln->load_ewma <= 0.0)
		ln->load_ewma_total = 100.0  * 286.0;
	else
		ln->load_ewma_total = ln->load_ewma;
	// to make better loda decisions we should also consider the performance of the ap

	if (ln->node.he == 1)
		ln->load_ewma_total = ln->load_ewma_total / 286.0;
	else if (ln->node.vht == 1)
		ln->load_ewma_total = ln->load_ewma_total / 200.0;
	else if (ln->node.n == 1)
		ln->load_ewma_total = ln->load_ewma_total / 150.0;
	else
		ln->load_ewma_total = ln->load_ewma_total / 108.0;
	if (ln->node.freq > 4000)
		ln->load_ewma_total = ln->load_ewma_total * 0.5;

	if (ln->node.cw == 160) // since 160 mhz often operates just with 2 chains we treat it like 80 mhz
		ln->load_ewma_total = ln->load_ewma_total * 0.25;
	if (ln->node.cw == 80)
		ln->load_ewma_total = ln->load_ewma_total * 0.25;
	if (ln->node.cw == 40)
		ln->load_ewma_total = ln->load_ewma_total * 0.5;

	ln->node.load = ln->load_ewma_total;
	ln->node.nosurvey = 0;
}

static void nl80211_update_node(struct uloop_timeout *t)
{
	struct usteer_local_node *ln = container_of(t, struct usteer_local_node, nl80211.update);

	uloop_timeout_set(t, 1000);
	ln->ifindex = if_nametoindex(ln->iface);
	ln->node.nosurvey = 1;
	nl80211_get_survey(&ln->node, ln, nl80211_update_node_result);
}

static void nl80211_init_node(struct usteer_node *node)
{
	struct usteer_local_node *ln = container_of(node, struct usteer_local_node, node);
	struct genlmsghdr *gnlh;
	static bool _init = false;
	struct nl_msg *msg;

	if (node->type != NODE_TYPE_LOCAL)
		return;

	ln->nl80211.present = false;
	ln->wiphy = -1;

	if (!ln->ifindex) {
		MSG(INFO, "No ifindex found for node %s\n", usteer_node_name(node));
		return;
	}

	if (!_init) {
		if (unl_genl_init(&unl, "nl80211") < 0) {
			unl_free(&unl);
			MSG(INFO, "nl80211 init failed\n");
			return;
		}

		_init = true;
	}

	msg = unl_genl_msg(&unl, NL80211_CMD_GET_INTERFACE, false);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, ln->ifindex);
	unl_genl_request_single(&unl, msg, &msg);
	if (!msg)
		return;

	gnlh = nlmsg_data(nlmsg_hdr(msg));
	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[NL80211_ATTR_WIPHY])
		goto nla_put_failure;

	if (!tb[NL80211_ATTR_MAC])
		goto nla_put_failure;

	ln->wiphy = nla_get_u32(tb[NL80211_ATTR_WIPHY]);

	memcpy(node->bssid, nla_data(tb[NL80211_ATTR_MAC]), ETH_ALEN);

	if (tb[NL80211_ATTR_SSID]) {
		int len = nla_len(tb[NL80211_ATTR_SSID]);

		if (len >= sizeof(node->ssid))
			len = sizeof(node->ssid) - 1;

		memcpy(node->ssid, nla_data(tb[NL80211_ATTR_SSID]), len);
		node->ssid[len] = 0;
	}

	MSG(INFO, "Found nl80211 phy on wdev %s, ssid=%s\n", usteer_node_name(node), node->ssid);
	ln->load_ewma = -1;
	ln->nl80211.present = true;
	ln->nl80211.update.cb = nl80211_update_node;
	nl80211_update_node(&ln->nl80211.update);

nla_put_failure:
	nlmsg_free(msg);
	return;
}

static void nl80211_free_node(struct usteer_node *node)
{
	struct usteer_local_node *ln = container_of(node, struct usteer_local_node, node);

	if (!ln->nl80211.present)
		return;

	uloop_timeout_cancel(&ln->nl80211.update);
}

static void nl80211_update_sta(struct usteer_node *node, struct sta_info *si)
{
	struct nlattr *tb_sta[NL80211_STA_INFO_MAX + 1];
	struct usteer_local_node *ln = container_of(node, struct usteer_local_node, node);
	struct genlmsghdr *gnlh;
	struct nl_msg *msg;
	int signal = NO_SIGNAL;

	if (!ln->nl80211.present)
		return;

	msg = unl_genl_msg(&unl, NL80211_CMD_GET_STATION, false);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, ln->ifindex);
	NLA_PUT(msg, NL80211_ATTR_MAC, ETH_ALEN, si->sta->addr);
	unl_genl_request_single(&unl, msg, &msg);
	if (!msg)
		return;

	gnlh = nlmsg_data(nlmsg_hdr(msg));
	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[NL80211_ATTR_STA_INFO])
		goto nla_put_failure;

	if (nla_parse_nested(tb_sta, NL80211_STA_INFO_MAX,
			     tb[NL80211_ATTR_STA_INFO], NULL))
		goto nla_put_failure;

	if (tb_sta[NL80211_STA_INFO_SIGNAL_AVG])
		signal = (int8_t) nla_get_u8(tb_sta[NL80211_STA_INFO_SIGNAL_AVG]);
	
	if (tb_sta[NL80211_STA_INFO_CONNECTED_TIME])
		si->connected_since = current_time - (nla_get_u32(tb_sta[NL80211_STA_INFO_CONNECTED_TIME]) * 1000);

	usteer_sta_info_update(si, signal, true);

nla_put_failure:
	nlmsg_free(msg);
	return;
}

static int nl80211_scan_result(struct nl_msg *msg, void *arg)
{
	static struct nla_policy bss_policy[NL80211_BSS_MAX + 1] = {
		[NL80211_BSS_FREQUENCY] = { .type = NLA_U32 },
		[NL80211_BSS_CAPABILITY] = { .type = NLA_U16 },
		[NL80211_BSS_SIGNAL_MBM] = { .type = NLA_U32 },
	};
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *bss[NL80211_BSS_MAX + 1];
	struct nl80211_scan_req *req = arg;
	struct usteer_scan_result data = {
		.signal = -127,
	};
	struct genlmsghdr *gnlh;
	struct nlattr *ie_attr;
	int ielen = 0;
	uint8_t *ie;

	gnlh = nlmsg_data(nlmsg_hdr(msg));
	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[NL80211_ATTR_BSS])
		return NL_SKIP;

	if (nla_parse_nested(bss, NL80211_BSS_MAX, tb[NL80211_ATTR_BSS],
			     bss_policy))
		return NL_SKIP;

	if (!bss[NL80211_BSS_BSSID] ||
	    !bss[NL80211_BSS_FREQUENCY])
		return NL_SKIP;

	data.freq = nla_get_u32(bss[NL80211_BSS_FREQUENCY]);
	memcpy(data.bssid, nla_data(bss[NL80211_BSS_BSSID]), sizeof(data.bssid));

	if (bss[NL80211_BSS_SIGNAL_MBM]) {
		int32_t signal = nla_get_u32(bss[NL80211_BSS_SIGNAL_MBM]);
		data.signal = signal / 100;
	}

	ie_attr = bss[NL80211_BSS_INFORMATION_ELEMENTS];
	if (!ie_attr)
		ie_attr = bss[NL80211_BSS_BEACON_IES];

	if (!ie_attr)
		goto skip_ie;

	ie = (uint8_t *) nla_data(ie_attr);
	ielen = nla_len(ie_attr);
	for (; ielen >= 2 && ielen >= ie[1];
	     ielen -= ie[1] + 2, ie += ie[1] + 2) {
		if (ie[0] == 0) { /* SSID */
			if (ie[1] > 32)
				continue;

			memcpy(data.ssid, ie + 2, ie[1]);
		}
	}

skip_ie:
	req->cb(req->priv, &data);

	return NL_SKIP;
}

static int nl80211_scan_event_cb(struct nl_msg *msg, void *data)
{
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));

	switch (gnlh->cmd) {
	case NL80211_CMD_NEW_SCAN_RESULTS:
	case NL80211_CMD_SCAN_ABORTED:
		unl_loop_done(&unl);
		break;
	}

	return NL_SKIP;
}

static int nl80211_scan(struct usteer_node *node, struct usteer_scan_request *req,
			void *priv, void (*cb)(void *priv, struct usteer_scan_result *r))
{
	struct usteer_local_node *ln = container_of(node, struct usteer_local_node, node);
	struct nl80211_scan_req reqdata = {
		.priv = priv,
		.cb = cb,
	};
	struct nl_msg *msg;
	struct nlattr *cur;
	int i, ret;

	if (!ln->nl80211.present)
		return -ENODEV;

	msg = unl_genl_msg(&unl, NL80211_CMD_TRIGGER_SCAN, false);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, ln->ifindex);

	if (!req->passive) {
		cur = nla_nest_start(msg, NL80211_ATTR_SCAN_SSIDS);
		NLA_PUT(msg, 1, 0, "");
		nla_nest_end(msg, cur);
	}

	NLA_PUT_U32(msg, NL80211_ATTR_SCAN_FLAGS, NL80211_SCAN_FLAG_AP);

	if (req->n_freq) {
		cur = nla_nest_start(msg, NL80211_ATTR_SCAN_FREQUENCIES);
		for (i = 0; i < req->n_freq; i++)
			NLA_PUT_U32(msg, i, req->freq[i]);
		nla_nest_end(msg, cur);
	}

	unl_genl_subscribe(&unl, "scan");
	ret = unl_genl_request(&unl, msg, NULL, NULL);
	if (ret < 0)
		goto done;

	unl_genl_loop(&unl, nl80211_scan_event_cb, NULL);

done:
	unl_genl_unsubscribe(&unl, "scan");
	if (ret < 0)
		return ret;

	if (!cb)
		return 0;

	msg = unl_genl_msg(&unl, NL80211_CMD_GET_SCAN, true);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, ln->ifindex);
	unl_genl_request(&unl, msg, nl80211_scan_result, &reqdata);

	return 0;

nla_put_failure:
	nlmsg_free(msg);
	return -ENOMEM;
}

static int nl80211_wiphy_result(struct nl_msg *msg, void *arg)
{
	struct nl80211_freqlist_req *req = arg;
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nlattr *tb_band[NL80211_BAND_ATTR_MAX + 1];
	struct nlattr *tb_freq[NL80211_FREQUENCY_ATTR_MAX + 1];
	struct nlattr *nl_band;
	struct nlattr *nl_freq;
	struct nlattr *cur;
	struct genlmsghdr *gnlh;
	int rem_band;
	int rem_freq;

	gnlh = nlmsg_data(nlmsg_hdr(msg));
	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[NL80211_ATTR_WIPHY_BANDS])
		return NL_SKIP;

	nla_for_each_nested(nl_band, tb[NL80211_ATTR_WIPHY_BANDS], rem_band) {
		nla_parse(tb_band, NL80211_BAND_ATTR_MAX, nla_data(nl_band),
			  nla_len(nl_band), NULL);

		if (!tb_band[NL80211_BAND_ATTR_FREQS])
			continue;

		nla_for_each_nested(nl_freq, tb_band[NL80211_BAND_ATTR_FREQS],
				    rem_freq) {
			struct usteer_freq_data f = {};

			nla_parse(tb_freq, NL80211_FREQUENCY_ATTR_MAX,
				  nla_data(nl_freq), nla_len(nl_freq), NULL);

			if (tb_freq[NL80211_FREQUENCY_ATTR_DISABLED])
				continue;

			if (tb_freq[NL80211_FREQUENCY_ATTR_NO_IR])
				continue;

			cur = tb_freq[NL80211_FREQUENCY_ATTR_FREQ];
			if (!cur)
				continue;

			f.freq = nla_get_u32(cur);
			f.dfs = !!tb_freq[NL80211_FREQUENCY_ATTR_RADAR];

			cur = tb_freq[NL80211_FREQUENCY_ATTR_MAX_TX_POWER];
			if (cur)
				f.txpower = nla_get_u32(cur) / 100;

			req->cb(req->priv, &f);
		}
	}

	return NL_SKIP;
}

static void nl80211_get_freqlist(struct usteer_node *node, void *priv,
				 void (*cb)(void *priv, struct usteer_freq_data *f))
{
	struct usteer_local_node *ln = container_of(node, struct usteer_local_node, node);
	struct nl80211_freqlist_req req = {
		.priv = priv,
		.cb = cb
	};
	struct nl_msg *msg;

	if (!ln->nl80211.present)
		return;

	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);

	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, ln->wiphy);
	NLA_PUT_FLAG(msg, NL80211_ATTR_SPLIT_WIPHY_DUMP);

	unl_genl_request(&unl, msg, nl80211_wiphy_result, &req);

	return;

nla_put_failure:
	nlmsg_free(msg);
}

static struct usteer_node_handler nl80211_handler = {
	.init_node = nl80211_init_node,
	.free_node = nl80211_free_node,
	.update_sta = nl80211_update_sta,
	.get_survey = nl80211_get_survey,
	.get_freqlist = nl80211_get_freqlist,
	.scan = nl80211_scan,
};

static void __usteer_init usteer_nl80211_init(void)
{
	list_add(&nl80211_handler.list, &node_handlers);
}
