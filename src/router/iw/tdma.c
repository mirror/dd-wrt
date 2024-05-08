#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif
#ifdef CONFIG_TDMA
#include <errno.h>
#include <string.h>
#include <strings.h>

#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "nl80211.h"
#include "iw.h"

SECTION(tdma);



static int join_tdma(struct nl80211_state *state,
		     struct nl_msg *msg,
		     int argc, char **argv,
		     enum id_input id)
{
	char *end;
	static const struct {
		const char *name;
		unsigned int val;
		unsigned int bw;
	} htmap[] = {
		{ .name = "HT", .val = NL80211_CHAN_HT20, .bw = NL80211_CHAN_WIDTH_20, },
		{ .name = "HT20", .val = NL80211_CHAN_HT20, .bw = NL80211_CHAN_WIDTH_20, },
		{ .name = "HT40+", .val = NL80211_CHAN_HT40PLUS, .bw = NL80211_CHAN_WIDTH_40, },
		{ .name = "HT40-", .val = NL80211_CHAN_HT40MINUS, .bw = NL80211_CHAN_WIDTH_40, },
		{ .name = "NOHT", .val = NL80211_CHAN_NO_HT, .bw = NL80211_CHAN_WIDTH_20_NOHT, },
		{ .name = "NOHT20", .val = NL80211_CHAN_NO_HT, .bw = NL80211_CHAN_WIDTH_20_NOHT, },
		{ .name = "NOHT3", .val = NL80211_CHAN_NO_HT, .bw = NL80211_CHAN_WIDTH_3, },
		{ .name = "NOHT5", .val = NL80211_CHAN_NO_HT, .bw = NL80211_CHAN_WIDTH_5, },
		{ .name = "NOHT10", .val = NL80211_CHAN_NO_HT, .bw = NL80211_CHAN_WIDTH_10, },
		{ .name = "HT3", .val = NL80211_CHAN_HT20, .bw = NL80211_CHAN_WIDTH_3, },
		{ .name = "HT5", .val = NL80211_CHAN_HT20, .bw = NL80211_CHAN_WIDTH_5, },
		{ .name = "HT10", .val = NL80211_CHAN_HT20, .bw = NL80211_CHAN_WIDTH_10, },
//		{ .name = "NOHT40", .val = NL80211_CHAN_NO_HT, .bw = NL80211_CHAN_WIDTH_40_NOHT, },
	};
	unsigned int htval = NL80211_CHAN_NO_HT;
	unsigned int bwval = NL80211_CHAN_WIDTH_20_NOHT;

	struct nlattr *nl_rates, *nl_band;
	int i;
	bool have_legacy_24 = false, have_legacy_5 = false;
	uint8_t legacy_24[32], legacy_5[32];
	int n_legacy_24 = 0, n_legacy_5 = 0;
	uint8_t *legacy = NULL;
	int *n_legacy = NULL;
	bool have_mcs_24 = false, have_mcs_5 = false;
	uint8_t mcs_24[77], mcs_5[77];
	int n_mcs_24 = 0, n_mcs_5 = 0;
	uint8_t *mcs = NULL;
	int *n_mcs = NULL;
	unsigned long freq;
	enum {
		S_NONE,
		S_LEGACY,
		S_MCS,
	} parser_state = S_NONE;
	unsigned char mac_addr[ETH_ALEN];


	if (argc < 2)
		return 1;

	/* freq */
	freq = strtoul(argv[0], &end, 10);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_FREQ, freq);
	if (*end != '\0')
		return 1;
	argv++;
	argc--;

	if (argc) {
		for (i = 0; i < ARRAY_SIZE(htmap); i++) {
			if (strcasecmp(htmap[i].name, argv[0]) == 0) {
				htval = htmap[i].val;
				bwval = htmap[i].bw;
				argv++;
				argc--;
				break;
			}
		}

	}
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_CHANNEL_TYPE, htval);
	NLA_PUT_U32(msg, NL80211_ATTR_CHANNEL_WIDTH, bwval);

	if (argc) {
		if (strcasecmp("ts", argv[0]) == 0) {
			argv++;
			argc--;
			if ( argc) {
				char *end;
				long tmpl;

				tmpl = strtol(argv[0], &end, 0);

				if (*end != '\0')
					return 1;
				if (tmpl < 0 || tmpl > 255)
					return 1;
				NLA_PUT_U8(msg, NL80211_ATTR_TDMA_NODES, (unsigned char)tmpl);
				argv++;
				argc--;
			} else return 1;
		}
	}

	if (argc) {
		if (strcasecmp("ver", argv[0]) == 0) {
			argv++;
			argc--;
			if ( argc) {
				char *end;
				long tmpl;

				tmpl = strtol(argv[0], &end, 0);

				if (*end != '\0')
					return 1;
				if (tmpl < 0 || tmpl > 255)
					return 1;
				NLA_PUT_U8(msg, NL80211_ATTR_TDMA_VERSION, (unsigned char)tmpl);
				argv++;
				argc--;
			} else return 1;
		}
	}

	if (argc) {
		if (strcasecmp("gack", argv[0]) == 0) {
			argv++;
			argc--;
			NLA_PUT_FLAG(msg, NL80211_ATTR_TDMA_GACK);
		}
	}

	if (argc) {
		if (strcasecmp("nor", argv[0]) == 0) {
			argv++;
			argc--;
			NLA_PUT_FLAG(msg, NL80211_ATTR_TDMA_REORDER);
		}
	}

	if (argc) {
		if (strcasecmp("poll", argv[0]) == 0) {
			argv++;
			argc--;
			NLA_PUT_FLAG(msg, NL80211_ATTR_TDMA_POLLING);
		}
	}

	if (argc) {
		if (strcasecmp("conn", argv[0]) == 0) {
			argv++;
			argc--;
			if ( argc) {
				if (mac_addr_a2n(mac_addr, argv[0])) {
					fprintf(stderr, "invalid mac address\n");
					return 2;
				}
				NLA_PUT(msg, NL80211_ATTR_MAC, ETH_ALEN, mac_addr);
				argc--;
				argv++;
			} else return 1;
		}
	}

	if (argc) {
		if (strcasecmp("ssid", argv[0]) == 0) {
			argv++;
			argc--;
			if ( argc) {
				/* SSID */
				NLA_PUT(msg, NL80211_ATTR_SSID, strlen(argv[0]), argv[0]);
				argc--;
				argv++;
			} else return 1;
		}
	}

	if (argc) {
		if (strcasecmp("ss", argv[0]) == 0) {
			argv++;
			argc--;
			if ( argc) {
				char *end;
				long tmpl;

				tmpl = strtol(argv[0], &end, 0);

				if (*end != '\0')
					return 1;
				if (tmpl < 0 || tmpl > 255)
					return 1;
				NLA_PUT_U8(msg, NL80211_ATTR_TDMA_SLOT_SIZE, (unsigned char)tmpl);
				argv++;
				argc--;
			} else return 1;
		}
	}

	if (argc) {
		if (strcasecmp("txratio", argv[0]) == 0) {
			argv++;
			argc--;
			if ( argc) {
				char *end;
				long tmpl;

				tmpl = strtol(argv[0], &end, 0);

				if (*end != '\0')
					return 1;
				if (tmpl < 0 || tmpl > 255)
					return 1;
				NLA_PUT_U8(msg, NL80211_ATTR_TDMA_TX_RATIO, (unsigned char)tmpl);
				argv++;
				argc--;
			} else return 1;
		}
	}

	if (argc) {
		if (strcasecmp("rxratio", argv[0]) == 0) {
			argv++;
			argc--;
			if ( argc) {
				char *end;
				long tmpl;

				tmpl = strtol(argv[0], &end, 0);

				if (*end != '\0')
					return 1;
				if (tmpl < 0 || tmpl > 255)
					return 1;
				NLA_PUT_U8(msg, NL80211_ATTR_TDMA_RX_RATIO, (unsigned char)tmpl);
				argv++;
				argc--;
			} else return 1;
		}
	}

	if (argc) {
		if (strcasecmp("aggr", argv[0]) == 0) {
			argv++;
			argc--;
			if ( argc) {
				char *end;
				long tmpl;

				tmpl = strtol(argv[0], &end, 0);

				if (*end != '\0')
					return 1;
				if (tmpl < 0 || tmpl > 255)
					return 1;
				NLA_PUT_U8(msg, NL80211_ATTR_TDMA_AGGREGATION, (unsigned char)tmpl);
				argv++;
				argc--;
			} else return 1;
		}
	}

	/* multicast rate */
	if (argc > 1 && strcmp(argv[0], "mcast-rate") == 0) {
		double rate;
		argv++;
		argc--;

		rate = strtod(argv[0], &end);
		if (*end != '\0')
			return 1;

		NLA_PUT_U32(msg, NL80211_ATTR_MCAST_RATE, (int)(rate * 10));
		argv++;
		argc--;
	}

	freq /= 1000;
	for (i = 0; i < argc; i++) {
		char *end;
		double tmpd;
		long tmpl;
		if (strcmp(argv[i], "legacy") == 0) {
		    if (freq == 2) {
			if (have_legacy_24)
				break;
			parser_state = S_LEGACY;
			legacy = legacy_24;
			n_legacy = &n_legacy_24;
			have_legacy_24 = true;
		    } else {
			if (have_legacy_5)
				break;
			parser_state = S_LEGACY;
			legacy = legacy_5;
			n_legacy = &n_legacy_5;
			have_legacy_5 = true;
		    }
		}
		else if (strcmp(argv[i], "mcs") == 0) {
		    if (freq == 2) {
			if (have_mcs_24)
				break;
			parser_state = S_MCS;
			mcs = mcs_24;
			n_mcs = &n_mcs_24;
			have_mcs_24 = true;
		    } else {
			if (have_mcs_5)
				break;
			parser_state = S_MCS;
			mcs = mcs_5;
			n_mcs = &n_mcs_5;
			have_mcs_5 = true;
		    }
		}
		else switch (parser_state) {
		case S_LEGACY:
			tmpd = strtod(argv[i], &end);
			if (*end != '\0')
				goto ooc;
			if (tmpd < 1 || tmpd > 255 * 2)
				goto ooc;
			legacy[(*n_legacy)++] = tmpd * 2;
			break;
		case S_MCS:
			tmpl = strtol(argv[i], &end, 0);
			if (*end != '\0')
				goto ooc;
			if (tmpl < 0 || tmpl > 255)
				goto ooc;
			mcs[(*n_mcs)++] = tmpl;
			break;
		default:
			goto ooc;
			break;
		}
	}
ooc:
	nl_rates = nla_nest_start(msg, NL80211_ATTR_TX_RATES);
	if (!nl_rates)
		goto nla_put_failure;

	if (have_legacy_24 || have_mcs_24) {
		nl_band = nla_nest_start(msg, NL80211_BAND_2GHZ);
		if (!nl_band)
			goto nla_put_failure;
		if (have_legacy_24)
			nla_put(msg, NL80211_TXRATE_LEGACY, n_legacy_24, legacy_24);
		if (have_mcs_24)
			nla_put(msg, NL80211_TXRATE_MCS, n_mcs_24, mcs_24);
		nla_nest_end(msg, nl_band);
	}

	if (have_legacy_5 || have_mcs_5) {
		nl_band = nla_nest_start(msg, NL80211_BAND_5GHZ);
		if (!nl_band)
			goto nla_put_failure;
		if (have_legacy_5)
			nla_put(msg, NL80211_TXRATE_LEGACY, n_legacy_5, legacy_5);
		if (have_mcs_5)
			nla_put(msg, NL80211_TXRATE_MCS, n_mcs_5, mcs_5);
		nla_nest_end(msg, nl_band);
	}

	nla_nest_end(msg, nl_rates);

	if (i < argc) {
	    if (strcmp(argv[i], "key") == 0 || strcmp(argv[i], "keys") == 0) {
		argv = &argv[i+1];
		argc -= (i + 1);
		if ( parse_keys(msg, &argv, &argc)!= 0)
		    return 1;
	    }
	}

	return 0;
 nla_put_failure:
	return -ENOSPC;
}

static int leave_tdma(struct nl80211_state *state,
		      struct nl_msg *msg,
		      int argc, char **argv,
		      enum id_input id)
{
	return 0;
}
COMMAND(tdma, leave, NULL,
	NL80211_CMD_LEAVE_TDMA, 0, CIB_NETDEV, leave_tdma,
	"Leave the current TDMA network.");
COMMAND(tdma, join,
	"<freq in MHz> [HT3|HT5|HT10|HT20|HT40+|HT40-|NOHT|NOHT3|NOHT5|NOHT10|NOHT40] [ts <node_num>] [ver <version>] [gack] [nor] [poll] [conn MAC] [ssid SSID] [ss <slot size. 0 - Large, 1 - Medium, 2 - Small>] [txratio <NUM>] [rxratio <NUM>] [aggr 0|1] [mcast-rate <rate in Mbps>] [legacy <legacy rate in Mbps>*] [mcs <MCS index>*] [key d:0:abcde]",
	NL80211_CMD_JOIN_TDMA, 0, CIB_NETDEV, join_tdma,
	"Join the TDMA network with the given frequency, version, slots, bitrate and keys");
#endif
