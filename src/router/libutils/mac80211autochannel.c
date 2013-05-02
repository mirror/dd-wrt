/*
 * Copyright (C) 2010 Felix Fietkau <nbd@openwrt.org>
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

#include "unl.h"
#include "linux/nl80211.h"
#include "list.h"
#include "list_sort.h"

#include "wlutils.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

static struct unl unl;
static void __attribute__((constructor)) mac80211_init(void)
{
	static bool bunl;
	if (!bunl) {
		unl_genl_init(&unl, "nl80211");
		bunl = 1;
	}
}

static struct mac80211_ac *add_to_mac80211_ac(struct mac80211_ac *list_root);
void free_mac80211_ac(struct mac80211_ac *acs);

static const char *freq_range;

struct frequency {
	struct list_head list;
	unsigned int freq;
	bool passive;
	int quality;
	int clear;
	int clear_count;
	int noise;
	int noise_count;
};

static LIST_HEAD(frequencies);

static struct nla_policy survey_policy[NL80211_SURVEY_INFO_MAX + 1] = {
	[NL80211_SURVEY_INFO_FREQUENCY] = {.type = NLA_U32},
	[NL80211_SURVEY_INFO_NOISE] = {.type = NLA_U8},
	[NL80211_SURVEY_INFO_CHANNEL_TIME] = {.type = NLA_U64},
	[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY] = {.type = NLA_U64},
};

#ifdef HAVE_BUFFALO
static const int bias_2g[] = { 80, 50, 70, 70, 50, 100, 50, 70, 70, 50, 80, 50, 70 };
#else
static const int bias_2g[] = { 100, 50, 75, 75, 50, 100, 50, 75, 75, 50, 100, 50, 75 };
#endif

static bool in_range(unsigned long freq)
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

static int freq_list(struct unl *unl, int phy)
{
	struct nlattr *tb[NL80211_FREQUENCY_ATTR_MAX + 1];
	struct frequency *f;
	struct nl_msg *msg;
	struct nlattr *band, *bands, *freqlist, *freq;
	int rem, rem2, freq_mhz, chan;

	msg = unl_genl_msg(unl, NL80211_CMD_GET_WIPHY, false);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(unl, msg, &msg) < 0)
		return NL_SKIP;

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
			if (!in_range(freq_mhz))
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
			f = calloc(1, sizeof(*f));
			INIT_LIST_HEAD(&f->list);

			f->freq = freq_mhz;
			list_add_tail(&f->list, &frequencies);
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

static struct frequency *get_freq(int freq)
{
	struct frequency *f;

	list_for_each_entry(f, &frequencies, list) {
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
	int freq;

	if (mac80211_parse_survey(msg, sinfo))
		goto out;

	freq = nla_get_u32(sinfo[NL80211_SURVEY_INFO_FREQUENCY]);
	f = get_freq(freq);
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

static void survey(struct unl *unl, int wdev, unl_cb cb)
{
	struct nl_msg *msg;

	msg = unl_genl_msg(unl, NL80211_CMD_GET_SURVEY, true);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, wdev);
	unl_genl_request(unl, msg, cb, NULL);
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

static void scan(struct unl *unl, int wdev)
{
	struct frequency *f;
	struct nlattr *opts;
	struct nl_msg *msg;
	int i = 0;

	msg = unl_genl_msg(unl, NL80211_CMD_TRIGGER_SCAN, false);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, wdev);

	opts = nla_nest_start(msg, NL80211_ATTR_SCAN_FREQUENCIES);
	list_for_each_entry(f, &frequencies, list) {
		NLA_PUT_U32(msg, ++i, f->freq);
	}
	nla_nest_end(msg, opts);

	unl_genl_subscribe(unl, "scan");
	if ((i = unl_genl_request(unl, msg, NULL, NULL)) < 0) {
		fprintf(stderr, "Scan request failed: %s\n", strerror(-i));
		goto out;
	}

	unl_genl_loop(unl, scan_event_cb, unl);

out:
	unl_genl_unsubscribe(unl, "scan");
	return;

nla_put_failure:
	nlmsg_free(msg);
}

struct sort_data {
	int lowest_noise;
};

static int freq_quality(struct frequency *f, struct sort_data *s)
{
	int c;
	int idx;

	if (!f->clear_count)
		return 0;

	c = f->clear;

	idx = (f->freq - 2412) / 5;

	/* strongly discourage the use of channels other than 1,6,11 */
	if (f->freq >= 2412 && f->freq <= 2484 && idx < ARRAY_SIZE(bias_2g))
		c = (c * bias_2g[idx]) / 100;

	/* subtract 2 * the number of db that the noise value is over the
	 * lowest that was found to discourage noisy channels */
	c -= 2 * (f->noise - s->lowest_noise);

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
struct mac80211_ac *mac80211autochannel(char *interface, char *freq_range, int scans, int ammount, int enable_passive)
{
	struct mac80211_ac *acs = NULL;
	struct frequency *f, *ftmp;
	int verbose = 0;
	int i, ch;
	struct sort_data sdata;
	int wdev, phy;

	unsigned int count = ammount;

	if (scans == 0)
		scans = 2;

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

	freq_list(&unl, phy);
	for (i = 0; i < scans; i++) {
		scan(&unl, wdev);
		survey(&unl, wdev, freq_add_stats);
	}

	memset(&sdata, 0, sizeof(sdata));
	list_for_each_entry(f, &frequencies, list) {
		if (f->clear_count) {
			f->clear /= f->clear_count;
			f->clear_count = 1;
		}

		if (f->noise_count) {
			f->noise /= f->noise_count;
			f->noise_count = 1;
			if (f->noise < sdata.lowest_noise)
				sdata.lowest_noise = f->noise;
		}
	}

	list_for_each_entry(f, &frequencies, list) {
		f->quality = freq_quality(f, &sdata);
		fprintf(stderr, "freq:%d qual:%d noise:%d\n", f->freq, f->quality, f->noise);
	}

	list_sort(&sdata, &frequencies, sort_cmp);

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
