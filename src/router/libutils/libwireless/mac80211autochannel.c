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
#include <dd_list.h>
#include <list_sort.h>

#include "wlutils.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

#define CHANNEL_EOF -1
#define CHANNEL_DISABLED -2

static struct mac80211_ac *add_to_mac80211_ac(struct mac80211_ac *list_root);
void free_mac80211_ac(struct mac80211_ac *acs);

static struct nla_policy survey_policy[NL80211_SURVEY_INFO_MAX + 1] = {
	[NL80211_SURVEY_INFO_FREQUENCY] = { .type = NLA_U32 },
	[NL80211_SURVEY_INFO_NOISE] = { .type = NLA_U8 },
	[NL80211_SURVEY_INFO_CHANNEL_TIME] = { .type = NLA_U64 },
	[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY] = { .type = NLA_U64 },
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
	[NL80211_FREQUENCY_ATTR_FREQ] = { .type = NLA_U32 },
};

static int freq_list(struct unl *unl, int phy, const char *freq_range, struct dd_list_head *frequencies)
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

#if defined(HAVE_BUFFALO) && defined(HAVE_WZRHPAG300NH)
			if (tb[NL80211_FREQUENCY_ATTR_RADAR])
				continue;
#endif

			freq_mhz = nla_get_u32(tb[NL80211_FREQUENCY_ATTR_FREQ]);
			if (!in_range(freq_mhz, freq_range))
				continue;
			f = calloc(1, sizeof(*f));
			INIT_DD_LIST_HEAD(&f->list);

			f->freq = freq_mhz;
			dd_list_add(&f->list, frequencies);
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

static struct frequency *get_freq(int freq, struct dd_list_head *frequencies)
{
	struct frequency *f;

	dd_list_for_each_entry(f, frequencies, list)
	{
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
	struct dd_list_head *frequencies = data;
	int freq;
	unsigned long long time, busy;
	if (mac80211_parse_survey(msg, sinfo))
		goto out;

	freq = nla_get_u32(sinfo[NL80211_SURVEY_INFO_FREQUENCY]);
	f = get_freq(freq, frequencies);
	if (!f)
		goto out;
	if (sinfo[NL80211_SURVEY_INFO_IN_USE]) {
		f->in_use = true;
	}
	if (sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME]) {
		time = nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME]);
		f->active += time;
		f->active_count++;
	}
	if (sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY]) {
		time = nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY]);
		f->busy += time;
		f->busy_count++;
	}
	if (sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_RX]) {
		time = (unsigned long long)nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_RX]);
		f->rx_time += time;
		f->rx_time_count++;
	}
	if (sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_TX]) {
		time = (unsigned long long)nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_TX]);
		f->tx_time += time;
		f->tx_time_count++;
	}

	if (sinfo[NL80211_SURVEY_INFO_NOISE]) {
		int8_t noise = nla_get_u8(sinfo[NL80211_SURVEY_INFO_NOISE]);
		f->noise += noise;
		f->noise_count++;
	}

out:
	return NL_SKIP;
}

static void survey(struct unl *unl, int wdev, unl_cb cb, struct dd_list_head *frequencies)
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

static int scan(struct unl *unl, int wdev, struct dd_list_head *frequencies)
{
	struct frequency *f;
	struct nlattr *opts;
	struct nl_msg *msg;
	int i = 0;
	int ret = 0;
	msg = unl_genl_msg(unl, NL80211_CMD_TRIGGER_SCAN, false);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, wdev);

	opts = nla_nest_start(msg, NL80211_ATTR_SCAN_FREQUENCIES);
	dd_list_for_each_entry(f, frequencies, list)
	{
		NLA_PUT_U32(msg, ++i, f->freq);
	}
	nla_nest_end(msg, opts);

	unl_genl_subscribe(unl, "scan");
	if ((i = unl_genl_request(unl, msg, NULL, NULL)) < 0) {
		dd_loginfo("survey", "Scan request failed: %s\n", strerror(-i));
		ret = -1;
		goto out;
	}

	unl_genl_loop(unl, scan_event_cb, unl);
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
		if (chan->freq == CHANNEL_DISABLED)
			continue;
		if (chan->freq == CHANNEL_EOF)
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
		if (chan->freq == CHANNEL_DISABLED)
			continue;
		if (chan->freq == CHANNEL_EOF)
			break;
		if (chan->freq == freq) {
			return chan->hw_eirp;
		}
	}
	return 0;
}

struct wifi_channels *get_chan(struct wifi_channels *wifi_channels, int freq, const char *interface)
{
	struct wifi_channels *chan = NULL;
	int i = 0;
	while (1) {
		chan = &wifi_channels[i++];
		if (chan->freq == CHANNEL_DISABLED)
			continue;
		if (chan->freq == CHANNEL_EOF)
			break;
		if (chan->freq == freq)
			break;
	}
	if (chan && chan->freq != CHANNEL_EOF) {
		if (freq >= 4000 &&
		    (nvram_nmatch("ng-only", "%s_net_mode", interface) || nvram_nmatch("n2-only", "%s_net_mode", interface) ||
		     nvram_nmatch("bg-mixed", "%s_net_mode", interface) || nvram_nmatch("ng-mixed", "%s_net_mode", interface) ||
		     nvram_nmatch("b-only", "%s_net_mode", interface) || nvram_nmatch("g-only", "%s_net_mode", interface))) {
			dd_loginfo("autochannel", "%s: %d not valid, ignore\n", interface, chan->freq);
			chan->freq = CHANNEL_DISABLED;
		}
		if (freq < 4000 &&
		    (nvram_nmatch("a-only", "%s_net_mode", interface) || nvram_nmatch("na-only", "%s_net_mode", interface) ||
		     nvram_nmatch("ac-only", "%s_net_mode", interface) || nvram_nmatch("acn-mixed", "%s_net_mode", interface) ||
		     nvram_nmatch("ax-only", "%s_net_mode", interface) || nvram_nmatch("xacn-mixed", "%s_net_mode", interface) ||
		     nvram_nmatch("n5-only", "%s_net_mode", interface))) {
			dd_loginfo("autochannel", "%s: %d not valid, ignore\n", interface, chan->freq);
			chan->freq = CHANNEL_DISABLED;
		}
#if defined(HAVE_BUFFALO_SA) && defined(HAVE_ATH9K)
		if ((!strcmp(getUEnv("region"), "AP") || !strcmp(getUEnv("region"), "US")) && ieee80211_mhz2ieee(freq) > 11 &&
		    ieee80211_mhz2ieee(freq) < 14 && nvram_default_match("region", "SA", ""))
			chan->freq = CHANNEL_DISABLED;
#endif
#ifdef HAVE_IDEXX
		if (ieee80211_mhz2ieee(freq) > 48)
			chan->freq = CHANNEL_DISABLED;
#endif
	}
	return chan;
}

static int freq_quality(struct wifi_channels *wifi_channels, int _max_eirp, int _htflags, struct frequency *f, struct sort_data *s,
			const char *interface)
{
	int c;
	int idx;
	if (!f)
		return 0;

	struct wifi_channels *chan = get_chan(wifi_channels, f->freq, interface);
	if (!chan || chan->freq < 0 || chan->freq == 2472) {
		return -1;
	}

	if (f->active && f->active_count && f->busy_count) {
		c = 100 - (uint32_t)(f->busy * 100 / f->active);
		//              fprintf(stderr, "base quality %d\n", c);
		if (c < 0)
			c = 0;
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

	/* if HT40, VHT80 or VHT160 auto channel is requested, check if desired channel is capabile of that operation mode, if not, move it to the bottom of the list */
	/*	if (!(_htflags & 8)) {
		if ((_htflags & AUTO_FORCEHT40) && !chan->luu && !chan->ull) {
			fprintf(stderr, "channel %d is not ht capable, set quality to zero %d\n", chan->freq, _htflags);
			return 0;
		}
		if ((_htflags & AUTO_FORCEVHT80) && !chan->ulu && !chan->lul) {
			fprintf(stderr, "channel %d is not vht80 capable, set quality to zero\n", chan->freq);
			return 0;
		}
		if ((_htflags & AUTO_FORCEVHT160) && !chan->uuu && !chan->lll) {
			fprintf(stderr, "channel %d is not vht160 capable, set quality to zero\n", chan->freq);
			return 0;
		}
	}*/
	int eirp = get_eirp(wifi_channels, f->freq);
	//      fprintf(stderr, "eirp %d\n", eirp);
	//      fprintf(stderr, "lowest noise %d\n", s->lowest_noise);
	//      fprintf(stderr, "noise %d\n", f->noise);
	//      fprintf(stderr, "max_eirp %d\n", _max_eirp);
	/* subtract noise delta to lowest noise. */
	c -= (f->noise - s->lowest_noise);
	/* add max capable delta output power */
	c += (_max_eirp - eirp);

	f->eirp = eirp;
	if (c < 0)
		c = 0;

	return c;
}

static int sort_cmp(void *priv, struct dd_list_head *a, struct dd_list_head *b)
{
	struct frequency *f1 = container_of(a, struct frequency, list);
	struct frequency *f2 = container_of(b, struct frequency, list);
	if (f1->quality > f2->quality)
		return -1;
	else
		return (f1->quality < f2->quality);
}

int getsurveystats(struct dd_list_head *frequencies, struct wifi_channels **channels, const char *interface, char *freq_range,
		   int scans, int bw)
{
	struct frequency *f;
	int verbose = 0;
	struct unl unl;
	int wdev, phy;
	int i, ch;
	struct wifi_channels *wifi_channels;
	sysprintf("iw dev %s scan > /dev/null", interface);
	mac80211_lock();
	int ret = unl_genl_init(&unl, "nl80211");
	wdev = if_nametoindex(interface);
	if (wdev < 0) {
		ret = -1;
		goto out;
	}
	const char *country = getIsoName(nvram_default_get("wlan0_regdomain", "UNITED_STATES"));
	if (!country)
		country = "DE";
	wifi_channels = mac80211_get_channels(&unl, interface, country, bw, 0xff, 1);
	if (channels)
		*channels = wifi_channels;
	if (scans == 0)
		scans = 2;
	phy = unl_nl80211_wdev_to_phy(&unl, wdev);
	if (phy < 0) {
		ret = -1;
		goto out;
	}
	freq_list(&unl, phy, freq_range, frequencies);
	scan(&unl, wdev, frequencies);
	survey(&unl, wdev, freq_add_stats, frequencies);

out:
	unl_free(&unl);
	mac80211_unlock();
	return ret;
}

// leave space for enhencements with more cards and already chosen channels...
struct mac80211_ac *mac80211autochannel(const char *interface, char *freq_range, int scans, int enable_passive, int htflags)
{
	struct mac80211_ac *acs = NULL;
	struct frequency *f, *ftmp;
	int verbose = 0;
	int i, ch;
	struct sort_data sdata;
	int wdev, phy;
	int _htflags = htflags;
	int bw = 20;
	int _max_eirp;
	struct wifi_channels *wifi_channels;
	DD_LIST_HEAD(frequencies);
	if (is_ath5k(interface))
		_htflags |= 8;
	if (htflags & AUTO_FORCEVHT80)
		bw = 80;
	else if (htflags & AUTO_FORCEVHT160)
		bw = 160;
	else if (htflags & AUTO_FORCEHT40)
		bw = 40;
	/* get maximum eirp possible in channel list */
	if (getsurveystats(&frequencies, &wifi_channels, interface, freq_range, scans, bw))
		goto out;
	_max_eirp = get_max_eirp(wifi_channels);
	bzero(&sdata, sizeof(sdata));
	dd_list_for_each_entry(f, &frequencies, list)
	{
		if (f->noise_count) {
			f->noise /= f->noise_count;
			f->noise_count = 1;
			if (f->noise && f->noise < sdata.lowest_noise)
				sdata.lowest_noise = f->noise;
		}
	}

	dd_list_for_each_entry(f, &frequencies, list)
	{
		/* in case noise calibration fails, we assume -95 as default here */
		if (!f->noise)
			f->noise = -95;
		f->quality = freq_quality(wifi_channels, _max_eirp, _htflags, f, &sdata, interface);
	}

	dd_list_sort(&sdata, &frequencies, sort_cmp);

	int c = getdevicecount();
	for (i = 0; i < c; i++) {
		char dev[32];
		int freq = 0;
		sprintf(dev, "wlan%d", i);
		if (nvram_nmatch("0", "%s_channel", dev) && !strcmp(dev, interface))
			break;

		if (strcmp(dev, interface)) {
			if (!nvram_nmatch("sta", "%s_mode", dev) && !nvram_nmatch("wdssta", "%s_mode", dev) &&
			    !nvram_nmatch("wdssta_mtik", "%s_mode", dev)) {
				if (nvram_nmatch("0", "%s_channel", dev)) {
					struct wifi_interface *intf = wifi_getfreq(dev);
					if (intf)
						freq = intf->freq;
				} else {
					freq = atoi(nvram_nget("%s_channel", dev));
				}
			}
			if (freq) {
				dd_list_for_each_entry(f, &frequencies, list)
				{
					if (f->freq == freq) {
						dd_loginfo("autochannel", "%s: %d already in use by %s, reduce quality\n",
							   interface, freq, dev);
						f->quality /= 2;
					}
				}
			}
		}
	}

	dd_list_for_each_entry(f, &frequencies, list)
	{
		dd_loginfo("autochannel", "%s: freq:%d qual:%d noise:%d eirp: %d\n", interface, f->freq, f->quality, f->noise,
			   f->eirp);
	}

	dd_list_for_each_entry(f, &frequencies, list)
	{
		if (f->passive && !enable_passive)
			continue;
		/* todo, implement algorithm for 80+80 */
		struct wifi_channels *chan = get_chan(wifi_channels, f->freq, interface);
		if (!chan) {
			dd_loginfo("autochannel", "chan %d not found, curious\n", f->freq);
			continue;
		}
		if (chan->freq < 0)
			continue;
		switch (bw) {
		case 40:
			if (chan->luu) {
				acs = add_to_mac80211_ac(acs);
				acs->freq = f->freq;
				acs->quality = f->quality;
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 20, &frequencies), &sdata, interface);
				acs->quality /= 2;
				acs->noise = f->noise;
				acs->luu = 1;
				dd_loginfo("autochannel", "%s: freq: %d HT40 [lower] quality %d\n", interface, f->freq,
					   acs->quality);
			}
			if (chan->ull) {
				acs = add_to_mac80211_ac(acs);
				acs->freq = f->freq;
				acs->quality = f->quality;
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 20, &frequencies), &sdata, interface);
				acs->quality /= 2;

				acs->noise = f->noise;
				acs->ull = 1;
				dd_loginfo("autochannel", "%s: freq: %d HT40 [upper] quality %d\n", interface, f->freq,
					   acs->quality);
			}
			break;
		case 80:
			if (chan->ull) {
				acs = add_to_mac80211_ac(acs);
				acs->freq = f->freq;
				acs->quality = f->quality;
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 20, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 20, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 40, &frequencies), &sdata, interface);
				acs->quality /= 4;
				acs->noise = f->noise;
				acs->ull = 1;
				dd_loginfo("autochannel", "%s: freq: %d VHT80 [UL] quality %d\n", interface, f->freq, acs->quality);
			}
			if (chan->luu) {
				acs = add_to_mac80211_ac(acs);
				acs->freq = f->freq;
				acs->quality = f->quality;
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 20, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 20, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 40, &frequencies), &sdata, interface);
				acs->quality /= 4;
				acs->noise = f->noise;
				acs->luu = 1;
				dd_loginfo("autochannel", "%s: freq: %d VHT80 [LU] quality %d\n", interface, f->freq, acs->quality);
			}
			if (chan->ulu) {
				acs = add_to_mac80211_ac(acs);
				acs->freq = f->freq;
				acs->quality = f->quality;
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 20, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 40, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 60, &frequencies), &sdata, interface);
				acs->quality /= 4;
				acs->noise = f->noise;
				acs->ulu = 1;
				dd_loginfo("autochannel", "%s: freq: %d VHT80 [UU] quality %d\n", interface, f->freq, acs->quality);
			}
			if (chan->lul) {
				acs = add_to_mac80211_ac(acs);
				acs->freq = f->freq;
				acs->quality = f->quality;
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 20, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 40, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 60, &frequencies), &sdata, interface);
				acs->quality /= 4;
				acs->noise = f->noise;
				acs->lul = 1;
				dd_loginfo("autochannel", "%s: freq: %d VHT80 [LL] quality %d\n", interface, f->freq, acs->quality);
			}
			break;
		case 160:
			if (chan->luu) {
				acs = add_to_mac80211_ac(acs);
				acs->freq = f->freq;
				acs->quality = f->quality;
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 20, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 20, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 40, &frequencies), &sdata, interface);
				//                              acs->quality +=freq_quality(wifi_channels, _max_eirp, _htflags, get_freq(f->freq - 60, &frequencies), &sdata);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 80, &frequencies), &sdata, interface);
				acs->quality /= 5;
				acs->noise = f->noise;
				acs->luu = 1;
				dd_loginfo("autochannel", "%s: freq: %d VHT160 [LUU] quality %d\n", interface, f->freq,
					   acs->quality);
			}
			if (chan->ull) {
				acs = add_to_mac80211_ac(acs);
				acs->freq = f->freq;
				acs->quality = f->quality;
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 60, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 20, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 20, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 40, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 60, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 80, &frequencies), &sdata, interface);
				acs->quality /= 7;
				acs->noise = f->noise;
				acs->ull = 1;
				dd_loginfo("autochannel", "%s: freq: %d VHT160 [ULL] quality %d\n", interface, f->freq,
					   acs->quality);
			}
			if (chan->lll) {
				acs = add_to_mac80211_ac(acs);
				acs->freq = f->freq;
				acs->quality = f->quality;
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 140, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 20, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 40, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 60, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 80, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 120, &frequencies), &sdata, interface);
				acs->quality /= 7;
				acs->noise = f->noise;
				acs->lll = 1;
				dd_loginfo("autochannel", "%s: freq: %d VHT160 [LLL] quality %d\n", interface, f->freq,
					   acs->quality);
			}
			if (chan->uuu) {
				acs = add_to_mac80211_ac(acs);
				acs->freq = f->freq;
				acs->quality = f->quality;
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 20, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 40, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 60, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 80, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 100, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 120, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 140, &frequencies), &sdata, interface);
				acs->quality /= 8;
				acs->noise = f->noise;
				acs->uuu = 1;
				dd_loginfo("autochannel", "%s: freq: %d VHT160 [UUU] quality %d\n", interface, f->freq,
					   acs->quality);
			}
			if (chan->llu) {
				acs = add_to_mac80211_ac(acs);
				acs->freq = f->freq;
				acs->quality = f->quality;
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 120, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 20, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 40, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 60, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 80, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 100, &frequencies), &sdata, interface);
				acs->quality /= 7;
				acs->noise = f->noise;
				acs->llu = 1;
				dd_loginfo("autochannel", "%s: freq: %d VHT160 [LLU] quality %d\n", interface, f->freq,
					   acs->quality);
			}
			if (chan->uul) {
				acs = add_to_mac80211_ac(acs);
				acs->freq = f->freq;
				acs->quality = f->quality;
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 20, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 20, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 40, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 60, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 80, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 100, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 120, &frequencies), &sdata, interface);
				acs->quality /= 8;
				acs->noise = f->noise;
				acs->uul = 1;
				dd_loginfo("autochannel", "%s: freq: %d VHT160 [UUL] quality %d\n", interface, f->freq,
					   acs->quality);
			}
			if (chan->lul) {
				acs = add_to_mac80211_ac(acs);
				acs->freq = f->freq;
				acs->quality = f->quality;
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 100, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 20, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 40, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 60, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 80, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 100, &frequencies), &sdata, interface);
				acs->quality /= 7;
				acs->noise = f->noise;
				acs->lul = 1;
				dd_loginfo("autochannel", "%s: freq: %d VHT160 [LUL] quality %d\n", interface, f->freq,
					   acs->quality);
			}
			if (chan->ulu) {
				acs = add_to_mac80211_ac(acs);
				acs->freq = f->freq;
				acs->quality = f->quality;
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq - 40, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 20, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 40, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 60, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 80, &frequencies), &sdata, interface);
				acs->quality += freq_quality(wifi_channels, _max_eirp, _htflags,
							     get_freq(f->freq + 100, &frequencies), &sdata, interface);
				acs->quality /= 7;
				acs->noise = f->noise;
				acs->ulu = 1;
				dd_loginfo("autochannel", "%s: freq: %d VHT160 [ULU] quality %d\n", interface, f->freq,
					   acs->quality);
			}

			break;
		default:
			acs = add_to_mac80211_ac(acs);
			acs->freq = f->freq;
			acs->quality = f->quality;
			acs->noise = f->noise;
			dd_loginfo("autochannel", "%s: freq: %d default quality %d\n", interface, f->freq, acs->quality);
			break;
		}
	}

	dd_list_for_each_entry_safe(f, ftmp, &frequencies, list)
	{
		//		dd_loginfo("autochannel", "%s: free %d\n", interface, f->freq);
		dd_list_del(&f->list);
		free(f);
	}
	int lastq = -1;
	struct mac80211_ac *racs = NULL;
	struct mac80211_ac *head = acs;
	while (acs) {
		struct mac80211_ac *next = acs->next;
		if (acs->quality > lastq) {
			if (bw > 20) {
				if (!acs->ulu && !acs->lul && !acs->uul && !acs->llu && !acs->uuu && !acs->lll && !acs->ull &&
				    !acs->luu)
					continue; // ignore since entry is no valid combination
			}
			racs = acs;
			lastq = acs->quality;
		}
		acs = next;
	}
	acs = head;
	while (acs) {
		struct mac80211_ac *next = acs->next;
		if (acs != racs)
			free(acs);
		acs = next;
	}
	if (racs)
		racs->next = NULL;
out:
	if (wifi_channels)
		free(wifi_channels);
	if (racs)
		dd_loginfo("autochannel", "%s: selected: %d\n", interface, racs->freq);

	return racs;
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
