/*
 * Copyright (C) 2010 Felix Fietkau <nbd@nbd.name>
 * Copyright (C) 2016 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <bcmnvram.h>
#include <utils.h>
#include <pthread.h>

#include "unl.h"
#include <nl80211.h>
#include "list.h"
#include "list_sort.h"

#include "wlutils.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

static struct mac80211_ac *add_to_mac80211_ac(struct mac80211_ac *list_root);
void free_mac80211_ac(struct mac80211_ac *acs);

struct frequency {
	struct list_head list;
	unsigned int freq;
	bool passive;
	int quality;
	int clear;
	int clear_count;
	int noise;
	int noise_count;
	int eirp;
};

static struct nla_policy survey_policy[NL80211_SURVEY_INFO_MAX + 1] = {
	[NL80211_SURVEY_INFO_FREQUENCY] = {.type = NLA_U32},
	[NL80211_SURVEY_INFO_NOISE] = {.type = NLA_U8},
	[NL80211_SURVEY_INFO_CHANNEL_TIME] = {.type = NLA_U64},
	[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY] = {.type = NLA_U64},
};

#ifdef HAVE_BUFFALO
static int bias_2g[] = { 80, 50, 70, 50, 100, 50, 70, 50, 80, 50, 70, 50, 100 };
static int bias_2g_ht40[] = { 50, 50, 100, 50, 50, 50, 50, 50, 50, 50, 100, 50, 50 };
#else
static int bias_2g[] = { 100, 50, 75, 50, 100, 50, 75, 50, 100, 50, 75, 50, 100 };
static int bias_2g_ht40[] = { 50, 50, 100, 50, 50, 50, 50, 50, 50, 50, 100, 50, 50 };
#endif

static bool in_range(unsigned long freq, const char *freq_range)
{
	const char *s = freq_range;
	unsigned long start, stop;
	char *end = NULL;

	if (!freq_range)
		return true;

	while (s && *s) {
		start = strtoul(s, &end, 10);
		s = end;
		switch (*s) {
		case '-':
			stop = strtoul(s + 1, &end, 10);
			if (freq >= start && freq <= stop)
				return true;

			if (*end != ',')
				return false;

			s++;
			break;
		case ',':
			s++;
			/* fall through */
		case '\0':
			if (start == freq)
				return true;
			break;
		}
	}
	return false;
}

static struct nla_policy freq_policy[NL80211_FREQUENCY_ATTR_MAX + 1] = {
	[NL80211_FREQUENCY_ATTR_FREQ] = {.type = NLA_U32},
};

static int freq_list(struct unl *unl, int phy, const char *freq_range, struct list_head *frequencies)
{
	struct nlattr *tb[NL80211_FREQUENCY_ATTR_MAX + 1];
	struct frequency *f;
	struct nl_msg *msg;
	struct nlattr *band, *bands, *freqlist, *freq;
	int rem, rem2, freq_mhz, chan;
	msg = unl_genl_msg(unl, NL80211_CMD_GET_WIPHY, false);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(unl, msg, &msg) < 0) {
		return NL_SKIP;
	}

	bands = unl_find_attr(unl, msg, NL80211_ATTR_WIPHY_BANDS);
	if (!bands)
		goto out;

	nla_for_each_nested(band, bands, rem) {
		freqlist = nla_find(nla_data(band), nla_len(band), NL80211_BAND_ATTR_FREQS);
		if (!freqlist)
			continue;

		nla_for_each_nested(freq, freqlist, rem2) {
			nla_parse_nested(tb, NL80211_FREQUENCY_ATTR_MAX, freq, freq_policy);
			if (!tb[NL80211_FREQUENCY_ATTR_FREQ])
				continue;

			if (tb[NL80211_FREQUENCY_ATTR_DISABLED])
				continue;

			freq_mhz = nla_get_u32(tb[NL80211_FREQUENCY_ATTR_FREQ]);
			if (!in_range(freq_mhz, freq_range))
				continue;
#if defined(HAVE_BUFFALO_SA) && defined(HAVE_ATH9K)
			if ((!strcmp(getUEnv("region"), "AP") || !strcmp(getUEnv("region"), "US"))
			    && ieee80211_mhz2ieee(freq_mhz) > 11 && ieee80211_mhz2ieee(freq_mhz) < 14 && nvram_default_match("region", "SA", ""))
				continue;
#endif
#if defined(HAVE_BUFFALO) && defined(HAVE_WZRHPAG300NH)
			if (tb[NL80211_FREQUENCY_ATTR_RADAR])
				continue;
#endif
#ifdef HAVE_IDEXX
			if (ieee80211_mhz2ieee(freq_mhz) > 48)
				continue;
#endif
			f = calloc(1, sizeof(*f));
			INIT_LIST_HEAD(&f->list);

			f->freq = freq_mhz;
			list_add_tail(&f->list, frequencies);
			if (tb[NL80211_FREQUENCY_ATTR_PASSIVE_SCAN])
				f->passive = true;
		}
	}

out:
nla_put_failure:
	nlmsg_free(msg);
	return NL_SKIP;
}

int mac80211_parse_survey(struct nl_msg *msg, struct nlattr **sinfo);

static struct frequency *get_freq(int freq, struct list_head *frequencies)
{
	struct frequency *f;

	list_for_each_entry(f, frequencies, list) {
		if (f->freq != freq)
			continue;

		return f;
	}
	return NULL;
}

static int freq_add_stats(struct nl_msg *msg, void *data)
{
	struct nlattr *sinfo[NL80211_SURVEY_INFO_MAX + 1];
	struct frequency *f;
	struct list_head *frequencies = data;
	int freq;
	if (mac80211_parse_survey(msg, sinfo))
		goto out;

	freq = nla_get_u32(sinfo[NL80211_SURVEY_INFO_FREQUENCY]);
	f = get_freq(freq, frequencies);
	if (!f)
		goto out;

	if (sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME] && sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY]) {
		uint64_t time, busy;

		time = nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME]);
		busy = nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY]);
		if (!time)
			goto out;

		f->clear += 100 - (uint32_t) (busy * 100 / time);
		f->clear_count++;
	}

	if (sinfo[NL80211_SURVEY_INFO_NOISE]) {
		int8_t noise = nla_get_u8(sinfo[NL80211_SURVEY_INFO_NOISE]);
		f->noise += noise;
		f->noise_count++;
	}

out:
	return NL_SKIP;
}

static void survey(struct unl *unl, int wdev, unl_cb cb, struct list_head *frequencies)
{
	struct nl_msg *msg;

	msg = unl_genl_msg(unl, NL80211_CMD_GET_SURVEY, true);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, wdev);
	unl_genl_request(unl, msg, cb, frequencies);
	return;

nla_put_failure:
	nlmsg_free(msg);
}

static int scan_event_cb(struct nl_msg *msg, void *data)
{
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct unl *unl = data;

	switch (gnlh->cmd) {
	case NL80211_CMD_NEW_SCAN_RESULTS:
	case NL80211_CMD_SCAN_ABORTED:
		unl_loop_done(unl);
	default:
		return NL_SKIP;
	}
}

static int scan(struct unl *unl, int wdev, struct list_head *frequencies)
{
	struct frequency *f;
	struct nlattr *opts;
	struct nl_msg *msg;
	int i = 0;
	int ret = 0;
	msg = unl_genl_msg(unl, NL80211_CMD_TRIGGER_SCAN, false);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, wdev);

	opts = nla_nest_start(msg, NL80211_ATTR_SCAN_FREQUENCIES);
	list_for_each_entry(f, frequencies, list) {
		NLA_PUT_U32(msg, ++i, f->freq);
	}
	nla_nest_end(msg, opts);

	unl_genl_subscribe(unl, "scan");
	if ((i = unl_genl_request(unl, msg, NULL, NULL)) < 0) {
		fprintf(stderr, "Scan request failed: %s\n", strerror(-i));
		ret = -1;
		goto out;
	}

	unl_genl_loop(unl, scan_event_cb, unl);
	return ret;
out:
	unl_genl_unsubscribe(unl, "scan");
	return ret;

nla_put_failure:
	nlmsg_free(msg);
}

struct sort_data {
	int lowest_noise;
};
static int get_max_eirp(struct wifi_channels *wifi_channels)
{
	int eirp = 0;
	int i = 0;
	struct wifi_channels *chan = NULL;
	while (1) {
		chan = &wifi_channels[i++];
		if (chan->freq == -1)
			break;
		int max_eirp = chan->hw_eirp;
		if (max_eirp > eirp) {
			eirp = max_eirp;
		}
	}
	return eirp;
}

static int get_eirp(struct wifi_channels *wifi_channels, int freq)
{
	int i = 0;
	struct wifi_channels *chan = NULL;
	while (1) {
		chan = &wifi_channels[i++];
		if (chan->freq == -1)
			break;
		if (chan->freq == freq) {
			return chan->hw_eirp;
		}
	}
	return 0;
}

static int freq_quality(struct wifi_channels *wifi_channels, int _max_eirp, int _htflags, struct frequency *f, struct sort_data *s)
{
	int c;
	int idx;

	if (f->clear_count) {

		c = f->clear;

		idx = (f->freq - 2412) / 5;
		int *bias = bias_2g;
		if (_htflags % AUTO_FORCEHT40)
			bias = bias_2g_ht40;

		/* strongly discourage the use of channels other than 1,6,11 */
		if (f->freq >= 2412 && f->freq <= 2484 && idx < ARRAY_SIZE(bias_2g))
			c = (c * bias[idx]) / 100;
	} else {
		c = 100;
	}

	struct wifi_channels *chan = NULL;
	int i = 0;
	while (1) {
		chan = &wifi_channels[i++];
		if (chan->freq == -1)
			break;
		if (chan->freq == f->freq)
			break;
	}
	if (chan->freq == -1 || chan->freq == 2472) {
		return 0;
	}

	/* if HT40, VHT80 or VHT160 auto channel is requested, check if desired channel is capabile of that operation mode, if not, move it to the bottom of the list */
	if ((_htflags & AUTO_FORCEHT40) && !chan->luu && !chan->ull) {
		fprintf(stderr, "channel %d is not ht capable, set set quality to zero\n", chan->freq);
		return 0;
	}
	if ((_htflags & AUTO_FORCEVHT80) && !chan->ulu && !chan->lul) {
		fprintf(stderr, "channel %d is not vht80 capable, set set quality to zero\n", chan->freq);
		return 0;
	}
	if ((_htflags & AUTO_FORCEVHT160) && !chan->uuu && !chan->lll) {
		fprintf(stderr, "channel %d is not vht160 capable, set set quality to zero\n", chan->freq);
		return 0;
	}

	int eirp = get_eirp(wifi_channels, f->freq);
	/* subtract noise delta to lowest noise. */
	c -= (f->noise - s->lowest_noise);
	/* subtract max capable output power (regulatory limited by hw caps) delta from maximum eirp possible */
	c -= (_max_eirp - eirp);
	f->eirp = eirp;
	if (c < 0)
		c = 0;

	return c;
}

static int sort_cmp(void *priv, struct list_head *a, struct list_head *b)
{
	struct frequency *f1 = container_of(a, struct frequency, list);
	struct frequency *f2 = container_of(b, struct frequency, list);
	if (f1->quality > f2->quality)
		return -1;
	else
		return (f1->quality < f2->quality);
}

// leave space for enhencements with more cards and already chosen channels...
struct mac80211_ac *mac80211autochannel(char *interface, char *freq_range, int scans, int ammount, int enable_passive, int htflags)
{
	struct mac80211_ac *acs = NULL;
	struct frequency *f, *ftmp;
	int verbose = 0;
	int i, ch;
	struct sort_data sdata;
	int wdev, phy;
	struct unl unl;
	int ret = unl_genl_init(&unl, "nl80211");
	unsigned int count = ammount;
	int _htflags = htflags;
	int bw = 20;
	struct wifi_channels *wifi_channels;
	int _max_eirp;
	LIST_HEAD(frequencies);

	if (htflags & AUTO_FORCEVHT80)
		bw = 80;
	else if (htflags & AUTO_FORCEVHT160)
		bw = 160;
	else if (htflags & AUTO_FORCEHT40)
		bw = 40;
	const char *country = getIsoName(nvram_default_get("ath0_regdomain", "UNITED_STATES"));
	if (!country)
		country = "DE";

	wifi_channels = mac80211_get_channels(&unl, interface, country, bw, 0xff);
	if (scans == 0)
		scans = 2;
	/* get maximum eirp possible in channel list */
	_max_eirp = get_max_eirp(wifi_channels);
	wdev = if_nametoindex(interface);
	if (wdev < 0) {
		fprintf(stderr, "mac80211autochannel Interface not found\n");
		goto out;
	}

	phy = unl_nl80211_wdev_to_phy(&unl, wdev);
	if (phy < 0) {
		fprintf(stderr, "mac80211autochannel PHY not found\n");
		goto out;
	}

	freq_list(&unl, phy, freq_range, &frequencies);
	for (i = 0; i < scans; i++) {
		int x = 0;
		while (x++ < 10) {
			if (!scan(&unl, wdev, &frequencies))
				break;
			sleep(1);	// try again
		}
		survey(&unl, wdev, freq_add_stats, &frequencies);
	}
	bzero(&sdata, sizeof(sdata));
	list_for_each_entry(f, &frequencies, list) {
		if (f->clear_count) {
			f->clear /= f->clear_count;
			f->clear_count = 1;
		}

		if (f->noise_count) {
			f->noise /= f->noise_count;
			f->noise_count = 1;
			if (f->noise && f->noise < sdata.lowest_noise)
				sdata.lowest_noise = f->noise;
		}
	}

	list_for_each_entry(f, &frequencies, list) {
		/* in case noise calibration fails, we assume -95 as default here */
		if (!f->noise)
			f->noise = -95;
		f->quality = freq_quality(wifi_channels, _max_eirp, _htflags, f, &sdata);
	}

	list_sort(&sdata, &frequencies, sort_cmp);

	list_for_each_entry(f, &frequencies, list) {
		fprintf(stderr, "%s: freq:%d qual:%d noise:%d eirp: %d\n", interface, f->freq, f->quality, f->noise, f->eirp);
	}

	list_for_each_entry(f, &frequencies, list) {
		if (f->passive && !enable_passive)
			continue;

		if (count-- == 0)
			break;
		acs = add_to_mac80211_ac(acs);
		acs->freq = f->freq;
		acs->quality = f->quality;
		acs->noise = f->noise;
	}

	list_for_each_entry_safe(f, ftmp, &frequencies, list) {
		list_del(&f->list);
		free(f);
	}

out:
	unl_free(&unl);
	return acs;
}

// thats wrong order
static struct mac80211_ac *add_to_mac80211_ac(struct mac80211_ac *list_root)
{
	struct mac80211_ac *new = calloc(1, sizeof(struct mac80211_ac));
	if (new == NULL) {
		fprintf(stderr, "mac80211_autochannel add_to_mac80211_ac: Out of memory!\n");
		return (NULL);
	} else {
		new->next = list_root;
		return (new);
	}
}

void free_mac80211_ac(struct mac80211_ac *acs)
{
	while (acs) {
		struct mac80211_ac *next = acs->next;
		free(acs);
		acs = next;
	}
}
