#include <ctype.h>
#include <netlink/attr.h>
#include <errno.h>
#include <stdbool.h>
#include "iw.h"
#include "nl80211.h"

void mac_addr_n2a(char *mac_addr, const unsigned char *arg)
{
	int i, l;

	l = 0;
	for (i = 0; i < ETH_ALEN ; i++) {
		if (i == 0) {
			sprintf(mac_addr+l, "%02x", arg[i]);
			l += 2;
		} else {
			sprintf(mac_addr+l, ":%02x", arg[i]);
			l += 3;
		}
	}
}

int mac_addr_a2n(unsigned char *mac_addr, char *arg)
{
	int i;

	for (i = 0; i < ETH_ALEN ; i++) {
		int temp;
		char *cp = strchr(arg, ':');
		if (cp) {
			*cp = 0;
			cp++;
		}
		if (sscanf(arg, "%x", &temp) != 1)
			return -1;
		if (temp < 0 || temp > 255)
			return -1;

		mac_addr[i] = temp;
		if (!cp)
			break;
		arg = cp;
	}
	if (i < ETH_ALEN - 1)
		return -1;

	return 0;
}

int parse_hex_mask(char *hexmask, unsigned char **result, size_t *result_len,
		   unsigned char **mask)
{
	size_t len = strlen(hexmask) / 2;
	unsigned char *result_val;
	unsigned char *result_mask = NULL;

	int pos = 0;

	*result_len = 0;

	result_val = calloc(len + 2, 1);
	if (!result_val)
		goto error;
	*result = result_val;
	if (mask) {
		result_mask = calloc(DIV_ROUND_UP(len, 8) + 2, 1);
		if (!result_mask)
			goto error;
		*mask = result_mask;
	}

	while (1) {
		char *cp = strchr(hexmask, ':');
		if (cp) {
			*cp = 0;
			cp++;
		}

		if (result_mask && (strcmp(hexmask, "-") == 0 ||
				    strcmp(hexmask, "xx") == 0 ||
				    strcmp(hexmask, "--") == 0)) {
			/* skip this byte and leave mask bit unset */
		} else {
			int temp, mask_pos;
			char *end;

			temp = strtoul(hexmask, &end, 16);
			if (*end)
				goto error;
			if (temp < 0 || temp > 255)
				goto error;
			result_val[pos] = temp;

			mask_pos = pos / 8;
			if (result_mask)
				result_mask[mask_pos] |= 1 << (pos % 8);
		}

		(*result_len)++;
		pos++;

		if (!cp)
			break;
		hexmask = cp;
	}

	return 0;
 error:
	free(result_val);
	free(result_mask);
	return -1;
}

unsigned char *parse_hex(char *hex, size_t *outlen)
{
	unsigned char *result;

	if (parse_hex_mask(hex, &result, outlen, NULL))
		return NULL;
	return result;
}

static const char *ifmodes[NL80211_IFTYPE_MAX + 1] = {
	"unspecified",
	"IBSS",
	"managed",
	"AP",
	"AP/VLAN",
	"WDS",
	"monitor",
	"mesh point",
	"P2P-client",
	"P2P-GO",
	"P2P-device",
	"outside context of a BSS",
	"NAN",
	"TDMA",
};

static char modebuf[100];

const char *iftype_name(enum nl80211_iftype iftype)
{
	if (iftype <= NL80211_IFTYPE_MAX && ifmodes[iftype])
		return ifmodes[iftype];
	sprintf(modebuf, "(%d) - Unknown mode (%d)", NL80211_IFTYPE_MAX, iftype);
	return modebuf;
}

static const char *commands[NL80211_CMD_MAX + 1] = {
#include "nl80211-commands.inc"
};

static char cmdbuf[100];
#if 1
const char *command_name(enum nl80211_commands cmd)
{
	if (cmd <= NL80211_CMD_MAX && commands[cmd])
		return commands[cmd];
	sprintf(cmdbuf, "Unknown command (%d)", cmd);
	return cmdbuf;
}
#endif

int ieee80211_channel_to_frequency(int chan, enum nl80211_band band)
{
	/* see 802.11 17.3.8.3.2 and Annex J
	 * there are overlapping channel numbers in 5GHz and 2GHz bands */
	if (chan < 0)
		return 0; /* not supported */
	switch (band) {
	case NL80211_BAND_2GHZ:
		if (chan == 14)
			return 2484;
		else if (chan == 0 ) {
			return 2407;
		}
		else if (chan > 14 && chan < 65) {
			return 2414 +(chan * 5);
		}
		else if (chan > 211 && chan < 256) {
			return 2407 + (chan - 256) * 5;
		}
		else if (chan < 14)
			return 2407 + chan * 5;
		break;
	case NL80211_BAND_5GHZ:
		if (chan >= 182 && chan <= 196)
			return 4000 + chan * 5;
		else
			return 5000 + chan * 5;
		break;
	case NL80211_BAND_6GHZ:
		/* see 802.11ax D6.1 27.3.23.2 */
		if (chan == 2)
			return 5935;
		if (chan <= 253)
			return 5950 + chan * 5;
		break;
	case NL80211_BAND_60GHZ:
		if (chan < 7)
			return 56160 + chan * 2160;
		break;
	default:
		;
	}
	return 0; /* not supported */
}

int ieee80211_frequency_to_channel(int freq)
{
	/* see 802.11-2007 17.3.8.3.2 and Annex J */
	if (freq == 2484)
		return 14;
	/* see 802.11ax D6.1 27.3.23.2 and Annex E */
	else if (freq == 5935)
		return 2;
	// boes hack...
	else if (freq == 2407)
		return 0;
	else if (freq < 2412)
		return (freq - 2407) / 5 + 256;
	else if (freq < 2484)
		return (freq - 2407) / 5;
	else if (freq > 2484 && freq < 4000 )
		return (freq - 2414) / 5;
	else if (freq < 4990 && freq > 4940)
		return ((freq * 10) + (((freq % 5) == 2) ? 5 : 0) - 49400) / 5;
	else if (freq >= 4800 && freq < 5005)
		return (freq - 4000) / 5;
	else if (freq < 5945)
		return (freq - 5000) / 5;
	else if (freq <= 45000) /* DMG band lower limit */
		/* see 802.11ax D4.1 27.3.22.2 */
		return (freq - 5940) / 5;
	else if (freq >= 58320 && freq <= 70200)
		return (freq - 56160) / 2160;
	else
		return 0;
}

void print_ssid_escaped(const uint8_t len, const uint8_t *data)
{
	int i;

	for (i = 0; i < len; i++) {
		if (isprint(data[i]) && data[i] != ' ' && data[i] != '\\')
			printf("%c", data[i]);
		else if (data[i] == ' ' &&
			 (i != 0 && i != len -1))
			printf(" ");
		else
			printf("\\x%.2x", data[i]);
	}
}

static int hex2num(char digit)
{
	if (!isxdigit(digit))
		return -1;
	if (isdigit(digit))
		return digit - '0';
	return tolower(digit) - 'a' + 10;
}

static int hex2byte(const char *hex)
{
	int d1, d2;

	d1 = hex2num(hex[0]);
	if (d1 < 0)
		return -1;
	d2 = hex2num(hex[1]);
	if (d2 < 0)
		return -1;
	return (d1 << 4) | d2;
}

char *hex2bin(const char *hex, char *buf)
{
	char *result = buf;
	int d;

	while (hex[0]) {
		d = hex2byte(hex);
		if (d < 0)
			return NULL;
		buf[0] = d;
		buf++;
		hex += 2;
	}

	return result;
}

static int parse_akm_suite(const char *cipher_str)
{

	if (!strcmp(cipher_str, "PSK"))
		return 0x000FAC02;
	if (!strcmp(cipher_str, "FT/PSK"))
		return 0x000FAC03;
	if (!strcmp(cipher_str, "PSK/SHA-256"))
		return 0x000FAC06;
	return -EINVAL;
}

static int parse_cipher_suite(const char *cipher_str)
{

	if (!strcmp(cipher_str, "TKIP"))
		return WLAN_CIPHER_SUITE_TKIP;
	if (!strcmp(cipher_str, "CCMP") || !strcmp(cipher_str, "CCMP-128"))
		return WLAN_CIPHER_SUITE_CCMP;
	if (!strcmp(cipher_str, "GCMP") || !strcmp(cipher_str, "GCMP-128"))
		return WLAN_CIPHER_SUITE_GCMP;
	if (!strcmp(cipher_str, "GCMP-256"))
		return WLAN_CIPHER_SUITE_GCMP_256;
	if (!strcmp(cipher_str, "CCMP-256"))
		return WLAN_CIPHER_SUITE_CCMP_256;
	return -EINVAL;
}

int parse_keys(struct nl_msg *msg, char **argv[], int *argc)
{
	struct nlattr *keys;
	int i = 0;
	bool have_default = false;
	char *arg = **argv;
	char keybuf[13];
	int pos = 0;

	if (!*argc)
		return 1;

	if (!memcmp(&arg[pos], "psk", 3)) {
		char psk_keybuf[32];
		int cipher_suite, akm_suite;

		if (*argc < 4)
			goto explain;

		pos+=3;
		if (arg[pos] != ':')
			goto explain;
		pos++;

		NLA_PUT_U32(msg, NL80211_ATTR_WPA_VERSIONS, NL80211_WPA_VERSION_2);

		if (strlen(&arg[pos]) != (sizeof(psk_keybuf) * 2) || !hex2bin(&arg[pos], psk_keybuf)) {
			printf("Bad PSK\n");
			return -EINVAL;
		}

		NLA_PUT(msg, NL80211_ATTR_PMK, 32, psk_keybuf);
		NLA_PUT_U32(msg, NL80211_ATTR_AUTH_TYPE, NL80211_AUTHTYPE_OPEN_SYSTEM);

		*argv += 1;
		*argc -= 1;
		arg = **argv;

		akm_suite = parse_akm_suite(arg);
		if (akm_suite < 0)
			goto explain;

		NLA_PUT_U32(msg, NL80211_ATTR_AKM_SUITES, akm_suite);

		*argv += 1;
		*argc -= 1;
		arg = **argv;

		cipher_suite = parse_cipher_suite(arg);
		if (cipher_suite < 0)
			goto explain;

		NLA_PUT_U32(msg, NL80211_ATTR_CIPHER_SUITES_PAIRWISE, cipher_suite);

		*argv += 1;
		*argc -= 1;
		arg = **argv;

		cipher_suite = parse_cipher_suite(arg);
		if (cipher_suite < 0)
			goto explain;

		NLA_PUT_U32(msg, NL80211_ATTR_CIPHER_SUITE_GROUP, cipher_suite);

		*argv += 1;
		*argc -= 1;
		return 0;
	}

	NLA_PUT_FLAG(msg, NL80211_ATTR_PRIVACY);

	keys = nla_nest_start(msg, NL80211_ATTR_KEYS);
	if (!keys)
		return -ENOBUFS;

	do {
		int keylen;
		struct nlattr *key = nla_nest_start(msg, ++i);
		char *keydata;

		arg = **argv;
		pos = 0;

		if (!key)
			return -ENOBUFS;

		if (arg[pos] == 'd') {
			NLA_PUT_FLAG(msg, NL80211_KEY_DEFAULT);
			pos++;
			if (arg[pos] == ':')
				pos++;
			have_default = true;
		}

		if (!isdigit(arg[pos]))
			goto explain;
		NLA_PUT_U8(msg, NL80211_KEY_IDX, arg[pos++] - '0');
		if (arg[pos++] != ':')
			goto explain;
		keydata = arg + pos;
		switch (strlen(keydata)) {
		case 10:
			keydata = hex2bin(keydata, keybuf);
			/* fall through */
		case 5:
			NLA_PUT_U32(msg, NL80211_KEY_CIPHER,
				    WLAN_CIPHER_SUITE_WEP40);
			keylen = 5;
			break;
		case 26:
			keydata = hex2bin(keydata, keybuf);
			/* fall through */
		case 13:
			NLA_PUT_U32(msg, NL80211_KEY_CIPHER,
				    WLAN_CIPHER_SUITE_WEP104);
			keylen = 13;
			break;
		default:
			goto explain;
		}

		if (!keydata)
			goto explain;

		NLA_PUT(msg, NL80211_KEY_DATA, keylen, keydata);

		*argv += 1;
		*argc -= 1;

		/* one key should be TX key */
		if (!have_default && !*argc)
			NLA_PUT_FLAG(msg, NL80211_KEY_DEFAULT);

		nla_nest_end(msg, key);
	} while (*argc);

	nla_nest_end(msg, keys);

	return 0;
 nla_put_failure:
	return -ENOBUFS;
 explain:
	fprintf(stderr, "key must be [d:]index:data where\n"
			"  'd:'     means default (transmit) key\n"
			"  'index:' is a single digit (0-3)\n"
			"  'data'   must be 5 or 13 ascii chars\n"
			"           or 10 or 26 hex digits\n"
			"for example: d:2:6162636465 is the same as d:2:abcde\n"
			"or psk:data <AKM Suite> <pairwise CIPHER> <groupwise CIPHER> where\n"
			"  'data' is the PSK (output of wpa_passphrase and the CIPHER can be CCMP or GCMP\n"
			"for example: psk:0123456789abcdef PSK CCMP CCMP\n"
			"The allowed AKM suites are PSK, FT/PSK, PSK/SHA-256\n"
			"The allowed Cipher suites are TKIP, CCMP, GCMP, GCMP-256, CCMP-256\n");
	return 2;
}

enum nl80211_chan_width str_to_bw(const char *str)
{
	static const struct {
		const char *name;
		unsigned int val;
	} bwmap[] = {
		{ .name = "2.5", .val = NL80211_CHAN_WIDTH_3, },
		{ .name = "5", .val = NL80211_CHAN_WIDTH_5, },
		{ .name = "10", .val = NL80211_CHAN_WIDTH_10, },
		{ .name = "20", .val = NL80211_CHAN_WIDTH_20, },
		{ .name = "40", .val = NL80211_CHAN_WIDTH_40, },
		{ .name = "80", .val = NL80211_CHAN_WIDTH_80, },
		{ .name = "80+80", .val = NL80211_CHAN_WIDTH_80P80, },
		{ .name = "160", .val = NL80211_CHAN_WIDTH_160, },
		{ .name = "320", .val = NL80211_CHAN_WIDTH_320, },
	};
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(bwmap); i++) {
		if (strcasecmp(bwmap[i].name, str) == 0)
			return bwmap[i].val;
	}

	return NL80211_CHAN_WIDTH_20_NOHT;
}

static int parse_freqs(struct chandef *chandef, int argc, char **argv,
		       int *parsed)
{
	uint32_t freq;
	char *end;
	bool need_cf1 = false, need_cf2 = false;

	if (argc < 1)
		return 0;

	chandef->width = str_to_bw(argv[0]);

	switch (chandef->width) {
	case NL80211_CHAN_WIDTH_20_NOHT:
		/* First argument was not understood, give up gracefully. */
		return 0;
	case NL80211_CHAN_WIDTH_20:
	case NL80211_CHAN_WIDTH_5:
	case NL80211_CHAN_WIDTH_10:
	case NL80211_CHAN_WIDTH_3:
		break;
	case NL80211_CHAN_WIDTH_80P80:
		need_cf2 = true;
		/* fall through */
	case NL80211_CHAN_WIDTH_40:
	case NL80211_CHAN_WIDTH_80:
	case NL80211_CHAN_WIDTH_160:
	case NL80211_CHAN_WIDTH_320:
		need_cf1 = true;
		break;
	case NL80211_CHAN_WIDTH_1:
	case NL80211_CHAN_WIDTH_2:
	case NL80211_CHAN_WIDTH_4:
	case NL80211_CHAN_WIDTH_8:
	case NL80211_CHAN_WIDTH_16:
		/* can't happen yet */
		break;
	}

	*parsed += 1;

	if (!need_cf1)
		return 0;

	if (argc < 2)
		return 1;

	/* center freq 1 */
	if (!*argv[1])
		return 1;
	freq = strtoul(argv[1], &end, 10);
	if (*end)
		return 1;
	*parsed += 1;

	chandef->center_freq1 = freq;

	if (!need_cf2)
		return 0;

	if (argc < 3)
		return 1;

	/* center freq 2 */
	if (!*argv[2])
		return 1;
	freq = strtoul(argv[2], &end, 10);
	if (*end)
		return 1;
	chandef->center_freq2 = freq;

	*parsed += 1;

	return 0;
}


/**
 * parse_freqchan - Parse frequency or channel definition
 *
 * @chandef: chandef structure to be filled in
 * @chan: Boolean whether to parse a channel or frequency based specifier
 * @argc: Number of arguments
 * @argv: Array of string arguments
 * @parsed: Pointer to return the number of used arguments, or NULL to error
 *          out if any argument is left unused.
 *
 * The given chandef structure will be filled in from the command line
 * arguments. argc/argv will be updated so that further arguments from the
 * command line can be parsed.
 *
 * Note that despite the fact that the function knows how many center freqs
 * are needed, there's an ambiguity if the next argument after this is an
 * integer argument, since the valid channel width values are interpreted
 * as such, rather than a following argument. This can be avoided by the
 * user by giving "NOHT" instead.
 *
 * The working specifier if chan is set are:
 *   <channel> [NOHT|HT20|HT40+|HT40-|5MHz|10MHz|80MHz|160MHz]
 *
 * And if frequency is set:
 *   <freq> [NOHT|HT20|HT40+|HT40-|5MHz|10MHz|80MHz|160MHz|320MHz]
 *   <control freq> [5|10|20|40|80|80+80|160] [<center1_freq> [<center2_freq>]]
 *
 * If the mode/channel width is not given the NOHT is assumed.
 *
 * Return: Number of used arguments, zero or negative error number otherwise
 */
int parse_freqchan(struct chandef *chandef, bool chan, int argc, char **argv,
		   int *parsed)
{
	char *end;
	static const struct chanmode chanmode[] = {
		{ .name = "HT20",
		  .width = NL80211_CHAN_WIDTH_20,
		  .freq1_diff = 0,
		  .chantype = NL80211_CHAN_HT20 },
		{ .name = "HT40+",
		  .width = NL80211_CHAN_WIDTH_40,
		  .freq1_diff = 10,
		  .chantype = NL80211_CHAN_HT40PLUS },
		{ .name = "HT40-",
		  .width = NL80211_CHAN_WIDTH_40,
		  .freq1_diff = -10,
		  .chantype = NL80211_CHAN_HT40MINUS },
		{ .name = "NOHT",
		  .width = NL80211_CHAN_WIDTH_20_NOHT,
		  .freq1_diff = 0,
		  .chantype = NL80211_CHAN_NO_HT },
		{ .name = "2.5MHz",
		  .width = NL80211_CHAN_WIDTH_3,
		  .freq1_diff = 0,
		  .chantype = -1 },
		{ .name = "5MHz",
		  .width = NL80211_CHAN_WIDTH_5,
		  .freq1_diff = 0,
		  .chantype = -1 },
		{ .name = "10MHz",
		  .width = NL80211_CHAN_WIDTH_10,
		  .freq1_diff = 0,
		  .chantype = -1 },
		{ .name = "80MHz",
		  .width = NL80211_CHAN_WIDTH_80,
		  .freq1_diff = 0,
		  .chantype = -1 },
		{ .name = "160MHz",
		  .width = NL80211_CHAN_WIDTH_160,
		  .freq1_diff = 0,
		  .chantype = -1 },
		{ .name = "320MHz",
		  .width = NL80211_CHAN_WIDTH_320,
		  .freq1_diff = 0,
		  .chantype = -1 },
		{ .name = "160MHz",
		  .width = NL80211_CHAN_WIDTH_160,
		  .freq1_diff = 0,
		  .chantype = -1 },
		{ .name = "80+80MHz",
		  .width = NL80211_CHAN_WIDTH_80P80,
		  .freq1_diff = 0,
		  .chantype = -1 },
	};
	const struct chanmode *chanmode_selected = NULL;
	unsigned int freq;
	unsigned int i;
	int _parsed = 0;
	int res = 0;

	if (argc < 1)
		return 1;

	if (!argv[0])
		goto out;
	freq = strtoul(argv[0], &end, 10);
	if (*end) {
		res = 1;
		goto out;
	}

	_parsed += 1;

	memset(chandef, 0, sizeof(struct chandef));

	if (chan) {
		enum nl80211_band band;

		band = freq <= 14 ? NL80211_BAND_2GHZ : NL80211_BAND_5GHZ;
		freq = ieee80211_channel_to_frequency(freq, band);
	}
	chandef->control_freq = freq;
	/* Assume 20MHz NOHT channel for now. */
	chandef->center_freq1 = freq;

	/* Try to parse HT mode definitions */
	if (argc > 1) {
		for (i = 0; i < ARRAY_SIZE(chanmode); i++) {
			if (strcasecmp(chanmode[i].name, argv[1]) == 0) {
				chanmode_selected = &chanmode[i];
				_parsed += 1;
				break;
			}
		}
	}

	/* channel mode given, use it and return. */
	if (chanmode_selected) {
		chandef->center_freq1 = get_cf1(chanmode_selected, freq);
		chandef->width = chanmode_selected->width;
		goto out;
	}

	/* This was a only a channel definition, nothing further may follow. */
	if (chan)
		goto out;

	res = parse_freqs(chandef, argc - 1, argv + 1, &_parsed);

 out:
	/* Error out if parsed is NULL. */
	if (!parsed && _parsed != argc)
		return 1;

	if (parsed)
		*parsed = _parsed;

	return res;
}

int put_chandef(struct nl_msg *msg, struct chandef *chandef)
{
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_FREQ, chandef->control_freq);
	NLA_PUT_U32(msg, NL80211_ATTR_CHANNEL_WIDTH, chandef->width);

	switch (chandef->width) {
	case NL80211_CHAN_WIDTH_20_NOHT:
		NLA_PUT_U32(msg,
			    NL80211_ATTR_WIPHY_CHANNEL_TYPE,
			    NL80211_CHAN_NO_HT);
		break;
	case NL80211_CHAN_WIDTH_20:
		NLA_PUT_U32(msg,
			    NL80211_ATTR_WIPHY_CHANNEL_TYPE,
			    NL80211_CHAN_HT20);
		break;
	case NL80211_CHAN_WIDTH_40:
		if (chandef->control_freq > chandef->center_freq1)
			NLA_PUT_U32(msg,
				    NL80211_ATTR_WIPHY_CHANNEL_TYPE,
				    NL80211_CHAN_HT40MINUS);
		else
			NLA_PUT_U32(msg,
				    NL80211_ATTR_WIPHY_CHANNEL_TYPE,
				    NL80211_CHAN_HT40PLUS);
		break;
	default:
		break;
	}

	if (chandef->center_freq1)
		NLA_PUT_U32(msg,
			    NL80211_ATTR_CENTER_FREQ1,
			    chandef->center_freq1);

	if (chandef->center_freq2)
		NLA_PUT_U32(msg,
			    NL80211_ATTR_CENTER_FREQ2,
			    chandef->center_freq2);

	return 0;

 nla_put_failure:
	return -ENOBUFS;
}

static void print_mcs_index(const __u8 *mcs)
{
	int mcs_bit, prev_bit = -2, prev_cont = 0;

	for (mcs_bit = 0; mcs_bit <= 76; mcs_bit++) {
		unsigned int mcs_octet = mcs_bit/8;
		unsigned int MCS_RATE_BIT = 1 << mcs_bit % 8;
		bool mcs_rate_idx_set;

		mcs_rate_idx_set = !!(mcs[mcs_octet] & MCS_RATE_BIT);

		if (!mcs_rate_idx_set)
			continue;

		if (prev_bit != mcs_bit - 1) {
			if (prev_bit != -2)
				printf("%d, ", prev_bit);
			else
				printf(" ");
			printf("%d", mcs_bit);
			prev_cont = 0;
		} else if (!prev_cont) {
			printf("-");
			prev_cont = 1;
		}

		prev_bit = mcs_bit;
	}

	if (prev_cont)
		printf("%d", prev_bit);
	printf("\n");
}

/*
 * There are only 4 possible values, we just use a case instead of computing it,
 * but technically this can also be computed through the formula:
 *
 * Max AMPDU length = (2 ^ (13 + exponent)) - 1 bytes
 */
static __u32 compute_ampdu_length(__u8 exponent)
{
	switch (exponent) {
	case 0: return 8191;  /* (2 ^(13 + 0)) -1 */
	case 1: return 16383; /* (2 ^(13 + 1)) -1 */
	case 2: return 32767; /* (2 ^(13 + 2)) -1 */
	case 3: return 65535; /* (2 ^(13 + 3)) -1 */
	default: return 0;
	}
}

static const char *print_ampdu_space(__u8 space)
{
	switch (space) {
	case 0: return "No restriction";
	case 1: return "1/4 usec";
	case 2: return "1/2 usec";
	case 3: return "1 usec";
	case 4: return "2 usec";
	case 5: return "4 usec";
	case 6: return "8 usec";
	case 7: return "16 usec";
	default:
		return "BUG (spacing more than 3 bits!)";
	}
}

void print_ampdu_length(__u8 exponent)
{
	__u32 max_ampdu_length;

	max_ampdu_length = compute_ampdu_length(exponent);

	if (max_ampdu_length) {
		printf("\t\tMaximum RX AMPDU length %d bytes (exponent: 0x0%02x)\n",
		       max_ampdu_length, exponent);
	} else {
		printf("\t\tMaximum RX AMPDU length: unrecognized bytes "
		       "(exponent: %d)\n", exponent);
	}
}

void print_ampdu_spacing(__u8 spacing)
{
	printf("\t\tMinimum RX AMPDU time spacing: %s (0x%02x)\n",
	       print_ampdu_space(spacing), spacing);
}

void print_ht_capability(__u16 cap)
{
#define PRINT_HT_CAP(_cond, _str) \
	do { \
		if (_cond) \
			printf("\t\t\t" _str "\n"); \
	} while (0)

	printf("\t\tCapabilities: 0x%02x\n", cap);

	PRINT_HT_CAP((cap & BIT(0)), "RX LDPC");
	PRINT_HT_CAP((cap & BIT(1)), "HT20/HT40");
	PRINT_HT_CAP(!(cap & BIT(1)), "HT20");

	PRINT_HT_CAP(((cap >> 2) & 0x3) == 0, "Static SM Power Save");
	PRINT_HT_CAP(((cap >> 2) & 0x3) == 1, "Dynamic SM Power Save");
	PRINT_HT_CAP(((cap >> 2) & 0x3) == 3, "SM Power Save disabled");

	PRINT_HT_CAP((cap & BIT(4)), "RX Greenfield");
	PRINT_HT_CAP((cap & BIT(5)), "RX HT20 SGI");
	PRINT_HT_CAP((cap & BIT(6)), "RX HT40 SGI");
	PRINT_HT_CAP((cap & BIT(7)), "TX STBC");

	PRINT_HT_CAP(((cap >> 8) & 0x3) == 0, "No RX STBC");
	PRINT_HT_CAP(((cap >> 8) & 0x3) == 1, "RX STBC 1-stream");
	PRINT_HT_CAP(((cap >> 8) & 0x3) == 2, "RX STBC 2-streams");
	PRINT_HT_CAP(((cap >> 8) & 0x3) == 3, "RX STBC 3-streams");

	PRINT_HT_CAP((cap & BIT(10)), "HT Delayed Block Ack");

	PRINT_HT_CAP(!(cap & BIT(11)), "Max AMSDU length: 3839 bytes");
	PRINT_HT_CAP((cap & BIT(11)), "Max AMSDU length: 7935 bytes");

	/*
	 * For beacons and probe response this would mean the BSS
	 * does or does not allow the usage of DSSS/CCK HT40.
	 * Otherwise it means the STA does or does not use
	 * DSSS/CCK HT40.
	 */
	PRINT_HT_CAP((cap & BIT(12)), "DSSS/CCK HT40");
	PRINT_HT_CAP(!(cap & BIT(12)), "No DSSS/CCK HT40");

	/* BIT(13) is reserved */

	PRINT_HT_CAP((cap & BIT(14)), "40 MHz Intolerant");

	PRINT_HT_CAP((cap & BIT(15)), "L-SIG TXOP protection");
#undef PRINT_HT_CAP
}

void print_ht_mcs(const __u8 *mcs)
{
	/* As defined in 7.3.2.57.4 Supported MCS Set field */
	unsigned int tx_max_num_spatial_streams, max_rx_supp_data_rate;
	bool tx_mcs_set_defined, tx_mcs_set_equal, tx_unequal_modulation;

	max_rx_supp_data_rate = (mcs[10] | ((mcs[11] & 0x3) << 8));
	tx_mcs_set_defined = !!(mcs[12] & (1 << 0));
	tx_mcs_set_equal = !(mcs[12] & (1 << 1));
	tx_max_num_spatial_streams = ((mcs[12] >> 2) & 3) + 1;
	tx_unequal_modulation = !!(mcs[12] & (1 << 4));

	if (max_rx_supp_data_rate)
		printf("\t\tHT Max RX data rate: %d Mbps\n", max_rx_supp_data_rate);
	/* XXX: else see 9.6.0e.5.3 how to get this I think */

	if (tx_mcs_set_defined) {
		if (tx_mcs_set_equal) {
			printf("\t\tHT TX/RX MCS rate indexes supported:");
			print_mcs_index(mcs);
		} else {
			printf("\t\tHT RX MCS rate indexes supported:");
			print_mcs_index(mcs);

			if (tx_unequal_modulation)
				printf("\t\tTX unequal modulation supported\n");
			else
				printf("\t\tTX unequal modulation not supported\n");

			printf("\t\tHT TX Max spatial streams: %d\n",
				tx_max_num_spatial_streams);

			printf("\t\tHT TX MCS rate indexes supported may differ\n");
		}
	} else {
		printf("\t\tHT RX MCS rate indexes supported:");
		print_mcs_index(mcs);
		printf("\t\tHT TX MCS rate indexes are undefined\n");
	}
}

struct vht_nss_ratio {
	bool valid;
	int bw_20;
	int bw_40;
	int bw_80;
	int bw_160;
	int bw_80_80;
};

/*
 * indexed by [chan_width][ext_nss_bw], ratio in 1/4 unit
 */
static const struct vht_nss_ratio nss_ratio_tbl[3][4] = {
	{
		/* chan_width == 0, ext_nss_bw == 0 */
		{
			.valid = true,
			.bw_20 = 4,
			.bw_40 = 4,
			.bw_80 = 4,
		},
		/* chan_width == 0, ext_nss_bw == 1 */
		{
			.valid = true,
			.bw_20 = 4,
			.bw_40 = 4,
			.bw_80 = 4,
			.bw_160 = 2,
		},
		/* chan_width == 0, ext_nss_bw == 2 */
		{
			.valid = true,
			.bw_20 = 4,
			.bw_40 = 4,
			.bw_80 = 4,
			.bw_160 = 2,
			.bw_80_80 = 2,
		},
		/* chan_width == 0, ext_nss_bw == 3 */
		{
			.valid = true,
			.bw_20 = 4,
			.bw_40 = 4,
			.bw_80 = 4,
			.bw_160 = 3,
			.bw_80_80 = 3,
		},
	},
	{
		/* chan_width == 1, ext_nss_bw == 0 */
		{
			.valid = true,
			.bw_20 = 4,
			.bw_40 = 4,
			.bw_80 = 4,
			.bw_160 = 4,
		},
		/* chan_width == 1, ext_nss_bw == 1 */
		{
			.valid = true,
			.bw_20 = 4,
			.bw_40 = 4,
			.bw_80 = 4,
			.bw_160 = 4,
			.bw_80_80 = 2,
		},
		/* chan_width == 1, ext_nss_bw == 2 */
		{
			.valid = true,
			.bw_20 = 4,
			.bw_40 = 4,
			.bw_80 = 4,
			.bw_160 = 4,
			.bw_80_80 = 3,
		},
		/* chan_width == 1, ext_nss_bw == 3 */
		{
			.valid = true,
			.bw_20 = 8,
			.bw_40 = 8,
			.bw_80 = 8,
			.bw_160 = 8,
			.bw_80_80 = 1,
		},
	},
	{
		/* chan_width == 2, ext_nss_bw == 0 */
		{
			.valid = true,
			.bw_20 = 4,
			.bw_40 = 4,
			.bw_80 = 4,
			.bw_160 = 4,
			.bw_80_80 = 4,
		},
		/* chan_width == 2, ext_nss_bw == 1 */
		{},
		/* chan_width == 2, ext_nss_bw == 2 */
		{},
		/* chan_width == 2, ext_nss_bw == 3 */
		{
			.valid = true,
			.bw_20 = 8,
			.bw_40 = 8,
			.bw_80 = 8,
			.bw_160 = 4,
			.bw_80_80 = 4,
		},
	},
};

static void print_nss_ratio_value(int ratio)
{
	const char *rstr;

	switch (ratio) {
	case 4:
		return;
	case 3:
		rstr = "3/4";
		break;
	case 2:
		rstr = "1/2";
		break;
	case 8:
		rstr = "x2";
		break;
	default:
		rstr = "undef";
		break;
	}

	printf("(%s NSS) ", rstr);
}

static void print_nss_ratio(const char *str, bool force_show, int ratio)
{
	if (!ratio)
		return;
	if (ratio == 4) {
		if (force_show)
			printf("%s ", str);
	} else {
		printf("%s ", str);
		print_nss_ratio_value(ratio);
	}
}

void print_vht_info(__u32 capa, const __u8 *mcs)
{
	__u16 tmp;
	__u32 supp_chan_width, ext_nss_bw;
	const struct vht_nss_ratio *nss_tbl;
	int i;

	printf("\t\tVHT Capabilities (0x%.8x):\n", capa);

#define PRINT_VHT_CAPA(_bit, _str) \
	do { \
		if (capa & BIT(_bit)) \
			printf("\t\t\t" _str "\n"); \
	} while (0)

	printf("\t\t\tMax MPDU length: ");
	switch (capa & 3) {
	case 0: printf("3895\n"); break;
	case 1: printf("7991\n"); break;
	case 2: printf("11454\n"); break;
	case 3: printf("(reserved)\n");
	}

	printf("\t\t\tSupported Channel Width: ");
	supp_chan_width = (capa >> 2) & 3;
	ext_nss_bw = (capa >> 30) & 3;
	nss_tbl = &nss_ratio_tbl[supp_chan_width][ext_nss_bw];

	if (!nss_tbl->valid)
		printf("(reserved)\n");
	else if (nss_tbl->bw_20 == 4 &&
		 nss_tbl->bw_40 == 4 &&
		 nss_tbl->bw_80 == 4 &&
		 (!nss_tbl->bw_160 || nss_tbl->bw_160 == 4) &&
		 (!nss_tbl->bw_80_80 || nss_tbl->bw_80_80 == 4)) {
		/* old style print format */
		switch (supp_chan_width) {
		case 0: printf("neither 160 nor 80+80\n"); break;
		case 1: printf("160 MHz\n"); break;
		case 2: printf("160 MHz, 80+80 MHz\n"); break;
		}
	} else {
		print_nss_ratio("20Mhz", false, nss_tbl->bw_20);
		print_nss_ratio("40Mhz", false, nss_tbl->bw_40);
		print_nss_ratio("80Mhz", false, nss_tbl->bw_80);
		print_nss_ratio("160Mhz", false, nss_tbl->bw_160);
		print_nss_ratio("80+80Mhz", false, nss_tbl->bw_80_80);
		printf("\n");
	}

	PRINT_VHT_CAPA(4, "RX LDPC");
	PRINT_VHT_CAPA(5, "short GI (80 MHz)");
	PRINT_VHT_CAPA(6, "short GI (160/80+80 MHz)");
	PRINT_VHT_CAPA(7, "TX STBC");
	/* RX STBC */
	PRINT_VHT_CAPA(11, "SU Beamformer");
	PRINT_VHT_CAPA(12, "SU Beamformee");
	/* compressed steering */
	/* # of sounding dimensions */
	PRINT_VHT_CAPA(19, "MU Beamformer");
	PRINT_VHT_CAPA(20, "MU Beamformee");
	PRINT_VHT_CAPA(21, "VHT TXOP PS");
	PRINT_VHT_CAPA(22, "+HTC-VHT");
	/* max A-MPDU */
	/* VHT link adaptation */
	PRINT_VHT_CAPA(28, "RX antenna pattern consistency");
	PRINT_VHT_CAPA(29, "TX antenna pattern consistency");

	printf("\t\tVHT RX MCS set:\n");
	tmp = mcs[0] | (mcs[1] << 8);
	for (i = 1; i <= 8; i++) {
		printf("\t\t\t%d streams: ", i);
		switch ((tmp >> ((i-1)*2) ) & 3) {
		case 0: printf("MCS 0-7\n"); break;
		case 1: printf("MCS 0-8\n"); break;
		case 2: printf("MCS 0-9\n"); break;
		case 3: printf("not supported\n"); break;
		}
	}
	tmp = mcs[2] | (mcs[3] << 8);
	printf("\t\tVHT RX highest supported: %d Mbps\n", tmp & 0x1fff);

	printf("\t\tVHT TX MCS set:\n");
	tmp = mcs[4] | (mcs[5] << 8);
	for (i = 1; i <= 8; i++) {
		printf("\t\t\t%d streams: ", i);
		switch ((tmp >> ((i-1)*2) ) & 3) {
		case 0: printf("MCS 0-7\n"); break;
		case 1: printf("MCS 0-8\n"); break;
		case 2: printf("MCS 0-9\n"); break;
		case 3: printf("not supported\n"); break;
		}
	}
	tmp = mcs[6] | (mcs[7] << 8);
	printf("\t\tVHT TX highest supported: %d Mbps\n", tmp & 0x1fff);

	printf("\t\tVHT extended NSS: %ssupported\n",
	       (tmp & (1 << 13)) ? "" : "not ");
}

static void __print_he_capa(const __u16 *mac_cap,
			    const __u16 *phy_cap,
			    const __u16 *mcs_set, size_t mcs_len,
			    const __u8 *ppet, int ppet_len,
			    bool indent)
{
	size_t mcs_used;
	int i;
	const char *pre = indent ? "\t" : "";

	#define PRINT_HE_CAP(_var, _idx, _bit, _str) \
	do { \
		if (_var[_idx] & BIT(_bit)) \
			printf("%s\t\t\t" _str "\n", pre); \
	} while (0)

	#define PRINT_HE_CAP_MASK(_var, _idx, _shift, _mask, _str) \
	do { \
		if ((_var[_idx] >> _shift) & _mask) \
			printf("%s\t\t\t" _str ": %d\n", pre, (_var[_idx] >> _shift) & _mask); \
	} while (0)

	#define PRINT_HE_MAC_CAP(...) PRINT_HE_CAP(mac_cap, __VA_ARGS__)
	#define PRINT_HE_MAC_CAP_MASK(...) PRINT_HE_CAP_MASK(mac_cap, __VA_ARGS__)
	#define PRINT_HE_PHY_CAP(...) PRINT_HE_CAP(phy_cap, __VA_ARGS__)
	#define PRINT_HE_PHY_CAP0(_idx, _bit, ...) PRINT_HE_CAP(phy_cap, _idx, _bit + 8, __VA_ARGS__)
	#define PRINT_HE_PHY_CAP_MASK(...) PRINT_HE_CAP_MASK(phy_cap, __VA_ARGS__)

	printf("%s\t\tHE MAC Capabilities (0x", pre);
	for (i = 0; i < 3; i++)
		printf("%04x", mac_cap[i]);
	printf("):\n");

	PRINT_HE_MAC_CAP(0, 0, "+HTC HE Supported");
	PRINT_HE_MAC_CAP(0, 1, "TWT Requester");
	PRINT_HE_MAC_CAP(0, 2, "TWT Responder");
	PRINT_HE_MAC_CAP_MASK(0, 3, 0x3, "Dynamic BA Fragementation Level");
	PRINT_HE_MAC_CAP_MASK(0, 5, 0x7, "Maximum number of MSDUS Fragments");
	PRINT_HE_MAC_CAP_MASK(0, 8, 0x3, "Minimum Payload size of 128 bytes");
	PRINT_HE_MAC_CAP_MASK(0, 10, 0x3, "Trigger Frame MAC Padding Duration");
	PRINT_HE_MAC_CAP_MASK(0, 12, 0x7, "Multi-TID Aggregation Support");

	PRINT_HE_MAC_CAP(1, 1, "All Ack");
	PRINT_HE_MAC_CAP(1, 2, "TRS");
	PRINT_HE_MAC_CAP(1, 3, "BSR");
	PRINT_HE_MAC_CAP(1, 4, "Broadcast TWT");
	PRINT_HE_MAC_CAP(1, 5, "32-bit BA Bitmap");
	PRINT_HE_MAC_CAP(1, 6, "MU Cascading");
	PRINT_HE_MAC_CAP(1, 7, "Ack-Enabled Aggregation");
	PRINT_HE_MAC_CAP(1, 9, "OM Control");
	PRINT_HE_MAC_CAP(1, 10, "OFDMA RA");
	PRINT_HE_MAC_CAP_MASK(1, 11, 0x3, "Maximum A-MPDU Length Exponent");
	PRINT_HE_MAC_CAP(1, 13, "A-MSDU Fragmentation");
	PRINT_HE_MAC_CAP(1, 14, "Flexible TWT Scheduling");
	PRINT_HE_MAC_CAP(1, 15, "RX Control Frame to MultiBSS");

	PRINT_HE_MAC_CAP(2, 0, "BSRP BQRP A-MPDU Aggregation");
	PRINT_HE_MAC_CAP(2, 1, "QTP");
	PRINT_HE_MAC_CAP(2, 2, "BQR");
	PRINT_HE_MAC_CAP(2, 3, "SRP Responder Role");
	PRINT_HE_MAC_CAP(2, 4, "NDP Feedback Report");
	PRINT_HE_MAC_CAP(2, 5, "OPS");
	PRINT_HE_MAC_CAP(2, 6, "A-MSDU in A-MPDU");
	PRINT_HE_MAC_CAP_MASK(2, 7, 7, "Multi-TID Aggregation TX");
	PRINT_HE_MAC_CAP(2, 10, "HE Subchannel Selective Transmission");
	PRINT_HE_MAC_CAP(2, 11, "UL 2x996-Tone RU");
	PRINT_HE_MAC_CAP(2, 12, "OM Control UL MU Data Disable RX");

	printf("%s\t\tHE PHY Capabilities: (0x", pre);
	for (i = 0; i < 11; i++)
		printf("%02x", ((__u8 *)phy_cap)[i + 1]);
	printf("):\n");

	PRINT_HE_PHY_CAP0(0, 1, "HE40/2.4GHz");
	PRINT_HE_PHY_CAP0(0, 2, "HE40/HE80/5GHz");
	PRINT_HE_PHY_CAP0(0, 3, "HE160/5GHz");
	PRINT_HE_PHY_CAP0(0, 4, "HE160/HE80+80/5GHz");
	PRINT_HE_PHY_CAP0(0, 5, "242 tone RUs/2.4GHz");
	PRINT_HE_PHY_CAP0(0, 6, "242 tone RUs/5GHz");

	PRINT_HE_PHY_CAP_MASK(1, 0, 0xf, "Punctured Preamble RX");
	PRINT_HE_PHY_CAP_MASK(1, 4, 0x1, "Device Class");
	PRINT_HE_PHY_CAP(1, 5, "LDPC Coding in Payload");
	PRINT_HE_PHY_CAP(1, 6, "HE SU PPDU with 1x HE-LTF and 0.8us GI");
	PRINT_HE_PHY_CAP_MASK(1, 7, 0x3, "Midamble Rx Max NSTS");
	PRINT_HE_PHY_CAP(1, 9, "NDP with 4x HE-LTF and 3.2us GI");
	PRINT_HE_PHY_CAP(1, 10, "STBC Tx <= 80MHz");
	PRINT_HE_PHY_CAP(1, 11, "STBC Rx <= 80MHz");
	PRINT_HE_PHY_CAP(1, 12, "Doppler Tx");
	PRINT_HE_PHY_CAP(1, 13, "Doppler Rx");
	PRINT_HE_PHY_CAP(1, 14, "Full Bandwidth UL MU-MIMO");
	PRINT_HE_PHY_CAP(1, 15, "Partial Bandwidth UL MU-MIMO");

	PRINT_HE_PHY_CAP_MASK(2, 0, 0x3, "DCM Max Constellation");
	PRINT_HE_PHY_CAP_MASK(2, 2, 0x1, "DCM Max NSS Tx");
	PRINT_HE_PHY_CAP_MASK(2, 3, 0x3, "DCM Max Constellation Rx");
	PRINT_HE_PHY_CAP_MASK(2, 5, 0x1, "DCM Max NSS Rx");
	PRINT_HE_PHY_CAP(2, 6, "Rx HE MU PPDU from Non-AP STA");
	PRINT_HE_PHY_CAP(2, 7, "SU Beamformer");
	PRINT_HE_PHY_CAP(2, 8, "SU Beamformee");
	PRINT_HE_PHY_CAP(2, 9, "MU Beamformer");
	PRINT_HE_PHY_CAP_MASK(2, 10, 0x7, "Beamformee STS <= 80Mhz");
	PRINT_HE_PHY_CAP_MASK(2, 13, 0x7, "Beamformee STS > 80Mhz");

	PRINT_HE_PHY_CAP_MASK(3, 0, 0x7, "Sounding Dimensions <= 80Mhz");
	PRINT_HE_PHY_CAP_MASK(3, 3, 0x7, "Sounding Dimensions > 80Mhz");
	PRINT_HE_PHY_CAP(3, 6, "Ng = 16 SU Feedback");
	PRINT_HE_PHY_CAP(3, 7, "Ng = 16 MU Feedback");
	PRINT_HE_PHY_CAP(3, 8, "Codebook Size SU Feedback");
	PRINT_HE_PHY_CAP(3, 9, "Codebook Size MU Feedback");
	PRINT_HE_PHY_CAP(3, 10, "Triggered SU Beamforming Feedback");
	PRINT_HE_PHY_CAP(3, 11, "Triggered MU Beamforming Feedback");
	PRINT_HE_PHY_CAP(3, 12, "Triggered CQI Feedback");
	PRINT_HE_PHY_CAP(3, 13, "Partial Bandwidth Extended Range");
	PRINT_HE_PHY_CAP(3, 14, "Partial Bandwidth DL MU-MIMO");
	PRINT_HE_PHY_CAP(3, 15, "PPE Threshold Present");

	PRINT_HE_PHY_CAP(4, 0, "SRP-based SR");
	PRINT_HE_PHY_CAP(4, 1, "Power Boost Factor ar");
	PRINT_HE_PHY_CAP(4, 2, "HE SU PPDU & HE PPDU 4x HE-LTF 0.8us GI");
	PRINT_HE_PHY_CAP_MASK(4, 3, 0x7, "Max NC");
	PRINT_HE_PHY_CAP(4, 6, "STBC Tx > 80MHz");
	PRINT_HE_PHY_CAP(4, 7, "STBC Rx > 80MHz");
	PRINT_HE_PHY_CAP(4, 8, "HE ER SU PPDU 4x HE-LTF 0.8us GI");
	PRINT_HE_PHY_CAP(4, 9, "20MHz in 40MHz HE PPDU 2.4GHz");
	PRINT_HE_PHY_CAP(4, 10, "20MHz in 160/80+80MHz HE PPDU");
	PRINT_HE_PHY_CAP(4, 11, "80MHz in 160/80+80MHz HE PPDU");
	PRINT_HE_PHY_CAP(4, 12, "HE ER SU PPDU 1x HE-LTF 0.8us GI");
	PRINT_HE_PHY_CAP(4, 13, "Midamble Rx 2x & 1x HE-LTF");
	PRINT_HE_PHY_CAP_MASK(4, 14, 0x3, "DCM Max BW");

	PRINT_HE_PHY_CAP(5, 0, "Longer Than 16HE SIG-B OFDM Symbols");
	PRINT_HE_PHY_CAP(5, 1, "Non-Triggered CQI Feedback");
	PRINT_HE_PHY_CAP(5, 2, "TX 1024-QAM");
	PRINT_HE_PHY_CAP(5, 3, "RX 1024-QAM");
	PRINT_HE_PHY_CAP(5, 4, "RX Full BW SU Using HE MU PPDU with Compression SIGB");
	PRINT_HE_PHY_CAP(5, 5, "RX Full BW SU Using HE MU PPDU with Non-Compression SIGB");

	mcs_used = 0;
	for (i = 0; i < 3; i++) {
		__u8 phy_cap_support[] = { BIT(1) | BIT(2), BIT(3), BIT(4) };
		char *bw[] = { "<= 80", "160", "80+80" };
		int j;

		if ((phy_cap[0] & (phy_cap_support[i] << 8)) == 0)
			continue;

		/* Supports more, but overflow? Abort. */
		if ((i * 2 + 2) * sizeof(mcs_set[0]) >= mcs_len)
			return;

		for (j = 0; j < 2; j++) {
			int k;
			printf("%s\t\tHE %s MCS and NSS set %s MHz\n", pre, j ? "TX" : "RX", bw[i]);
			for (k = 0; k < 8; k++) {
				__u16 mcs = mcs_set[(i * 2) + j];
				mcs >>= k * 2;
				mcs &= 0x3;
				printf("%s\t\t\t%d streams: ", pre, k + 1);
				if (mcs == 3)
					printf("not supported\n");
				else
					printf("MCS 0-%d\n", 7 + (mcs * 2));
			}

		}
		mcs_used += 2 * sizeof(mcs_set[0]);
	}

	/* Caller didn't provide ppet; infer it, if there's trailing space. */
	if (!ppet) {
		ppet = (const void *)((const __u8 *)mcs_set + mcs_used);
		if (mcs_used < mcs_len)
			ppet_len = mcs_len - mcs_used;
		else
			ppet_len = 0;
	}

	if (ppet_len && (phy_cap[3] & BIT(15))) {
		printf("%s\t\tPPE Threshold ", pre);
		for (i = 0; i < ppet_len; i++)
			if (ppet[i])
				printf("0x%02x ", ppet[i]);
		printf("\n");
	}
}

void print_iftype_list(const char *name, const char *pfx, struct nlattr *attr)
{
	struct nlattr *ift;
	int rem;

	printf("%s:\n", name);
	nla_for_each_nested(ift, attr, rem)
		printf("%s * %s\n", pfx, iftype_name(nla_type(ift)));
}

void print_iftype_line(struct nlattr *attr)
{
	struct nlattr *ift;
	bool first = true;
	int rem;

	nla_for_each_nested(ift, attr, rem) {
		if (first)
			first = false;
		else
			printf(", ");
		printf("%s", iftype_name(nla_type(ift)));
	}
}

void print_he_info(struct nlattr *nl_iftype)
{
	struct nlattr *tb[NL80211_BAND_IFTYPE_ATTR_MAX + 1];
	__u16 mac_cap[3] = { 0 };
	__u16 phy_cap[6] = { 0 };
	__u16 mcs_set[6] = { 0 };
	__u8 ppet[25] = { 0 };
	size_t len;
	int mcs_len = 0, ppet_len = 0;

	nla_parse(tb, NL80211_BAND_IFTYPE_ATTR_MAX,
		  nla_data(nl_iftype), nla_len(nl_iftype), NULL);

	if (!tb[NL80211_BAND_IFTYPE_ATTR_IFTYPES])
		return;

	printf("\t\tHE Iftypes: ");
	print_iftype_line(tb[NL80211_BAND_IFTYPE_ATTR_IFTYPES]);
	printf("\n");

	if (tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_MAC]) {
		len = nla_len(tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_MAC]);
		if (len > sizeof(mac_cap))
			len = sizeof(mac_cap);
		memcpy(mac_cap,
		       nla_data(tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_MAC]),
		       len);
	}

	if (tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_PHY]) {
		len = nla_len(tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_PHY]);

		if (len > sizeof(phy_cap) - 1)
			len = sizeof(phy_cap) - 1;
		memcpy(&((__u8 *)phy_cap)[1],
		       nla_data(tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_PHY]),
		       len);
	}

	if (tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_MCS_SET]) {
		len = nla_len(tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_MCS_SET]);
		if (len > sizeof(mcs_set))
			len = sizeof(mcs_set);
		memcpy(mcs_set,
		       nla_data(tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_MCS_SET]),
		       len);
		mcs_len = len;
	}

	if (tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_PPE]) {
		len = nla_len(tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_PPE]);
		if (len > sizeof(ppet))
			len = sizeof(ppet);
		memcpy(ppet,
		       nla_data(tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_PPE]),
		       len);
		ppet_len = len;
	}

	__print_he_capa(mac_cap, phy_cap, mcs_set, mcs_len, ppet, ppet_len,
			true);
}

static void __print_eht_capa(int band,
			     const __u8 *mac_cap,
			     const __u32 *phy_cap,
			     const __u8 *mcs_set, size_t mcs_len,
			     const __u8 *ppet, size_t ppet_len,
			     const __u16 *he_phy_cap,
			     bool indent)
{
	unsigned int i;
	const char *pre = indent ? "\t" : "";
	const char *mcs[] = { "0-7", "8-9", "10-11", "12-13"};

	#define PRINT_EHT_CAP(_var, _idx, _bit, _str) \
	do { \
		if (_var[_idx] & BIT(_bit)) \
			printf("%s\t\t\t" _str "\n", pre); \
	} while (0)

	#define PRINT_EHT_CAP_MASK(_var, _idx, _shift, _mask, _str) \
	do { \
		if ((_var[_idx] >> _shift) & _mask) \
			printf("%s\t\t\t" _str ": %d\n", pre, (_var[_idx] >> _shift) & _mask); \
	} while (0)

	#define PRINT_EHT_MAC_CAP(...) PRINT_EHT_CAP(mac_cap, __VA_ARGS__)
	#define PRINT_EHT_PHY_CAP(...) PRINT_EHT_CAP(phy_cap, __VA_ARGS__)
	#define PRINT_EHT_PHY_CAP_MASK(...) PRINT_EHT_CAP_MASK(phy_cap, __VA_ARGS__)

	printf("%s\t\tEHT MAC Capabilities (0x", pre);
	for (i = 0; i < 2; i++)
		printf("%02x", mac_cap[i]);
	printf("):\n");

	PRINT_EHT_MAC_CAP(0, 0, "NSEP priority access Supported");
	PRINT_EHT_MAC_CAP(0, 1, "EHT OM Control Supported");
	PRINT_EHT_MAC_CAP(0, 2, "Triggered TXOP Sharing Supported");
	PRINT_EHT_MAC_CAP(0, 3, "ARR Supported");

	printf("%s\t\tEHT PHY Capabilities: (0x", pre);
	for (i = 0; i < 8; i++)
		printf("%02x", ((__u8 *)phy_cap)[i]);
	printf("):\n");

	PRINT_EHT_PHY_CAP(0, 1, "320MHz in 6GHz Supported");
	PRINT_EHT_PHY_CAP(0, 2, "242-tone RU in BW wider than 20MHz Supported");
	PRINT_EHT_PHY_CAP(0, 3, "NDP With  EHT-LTF And 3.2 µs GI");
	PRINT_EHT_PHY_CAP(0, 4, "Partial Bandwidth UL MU-MIMO");
	PRINT_EHT_PHY_CAP(0, 5, "SU Beamformer");
	PRINT_EHT_PHY_CAP(0, 6, "SU Beamformee");
	PRINT_EHT_PHY_CAP_MASK(0, 7, 0x7, "Beamformee SS (80MHz)");
	PRINT_EHT_PHY_CAP_MASK(0, 10, 0x7, "Beamformee SS (160MHz)");
	PRINT_EHT_PHY_CAP_MASK(0, 13, 0x7, "Beamformee SS (320MHz)");

	PRINT_EHT_PHY_CAP_MASK(0, 16, 0x7, "Number Of Sounding Dimensions (80MHz)");
	PRINT_EHT_PHY_CAP_MASK(0, 19, 0x7, "Number Of Sounding Dimensions (160MHz)");
	PRINT_EHT_PHY_CAP_MASK(0, 22, 0x7, "Number Of Sounding Dimensions (320MHz)");
	PRINT_EHT_PHY_CAP(0, 25, "Ng = 16 SU Feedback");
	PRINT_EHT_PHY_CAP(0, 26, "Ng = 16 MU Feedback");
	PRINT_EHT_PHY_CAP(0, 27, "Codebook size (4, 2) SU Feedback");
	PRINT_EHT_PHY_CAP(0, 28, "Codebook size (7, 5) MU Feedback");
	PRINT_EHT_PHY_CAP(0, 29, "Triggered SU Beamforming Feedback");
	PRINT_EHT_PHY_CAP(0, 30, "Triggered MU Beamforming Partial BW Feedback");
	PRINT_EHT_PHY_CAP(0, 31, "Triggered CQI Feedback");

	PRINT_EHT_PHY_CAP(1, 0, "Partial Bandwidth DL MU-MIMO");
	PRINT_EHT_PHY_CAP(1, 1, "PSR-Based SR Support");
	PRINT_EHT_PHY_CAP(1, 2, "Power Boost Factor Support");
	PRINT_EHT_PHY_CAP(1, 3, "EHT MU PPDU With 4 EHT-LTF And 0.8 µs GI");
	PRINT_EHT_PHY_CAP_MASK(1, 4, 0xf, "Max Nc");
	PRINT_EHT_PHY_CAP(1, 8, "Non-Triggered CQI Feedback");

	PRINT_EHT_PHY_CAP(1, 9, "Tx 1024-QAM And 4096-QAM < 242-tone RU");
	PRINT_EHT_PHY_CAP(1, 10, "Rx 1024-QAM And 4096-QAM < 242-tone RU");
	PRINT_EHT_PHY_CAP(1, 11, "PPE Thresholds Present");
	PRINT_EHT_PHY_CAP_MASK(1, 12, 0x3, "Common Nominal Packet Padding");
	PRINT_EHT_PHY_CAP_MASK(1, 14, 0x1f, "Maximum Number Of Supported EHT-LTFs");
	PRINT_EHT_PHY_CAP_MASK(1, 19, 0xf, "Support of MCS 15");
	PRINT_EHT_PHY_CAP(1, 23, "Support Of EHT DUP In 6 GHz");
	PRINT_EHT_PHY_CAP(1, 24, "Support For 20MHz Rx NDP With Wider Bandwidth");
	PRINT_EHT_PHY_CAP(1, 25, "Non-OFDMA UL MU-MIMO (80MHz)");
	PRINT_EHT_PHY_CAP(1, 26, "Non-OFDMA UL MU-MIMO (160MHz)");
	PRINT_EHT_PHY_CAP(1, 27, "Non-OFDMA UL MU-MIMO (320MHz)");
	PRINT_EHT_PHY_CAP(1, 28, "MU Beamformer (80MHz)");
	PRINT_EHT_PHY_CAP(1, 29, "MU Beamformer (160MHz)");
	PRINT_EHT_PHY_CAP(1, 30, "MU Beamformer (320MHz)");

	printf("%s\t\tEHT MCS/NSS: (0x", pre);
	for (i = 0; i < mcs_len; i++)
		printf("%02x", ((__u8 *)mcs_set)[i]);
	printf("):\n");

	if (!(he_phy_cap[0] & ((BIT(2) | BIT(3) | BIT(4)) << 8))){
		for (i = 0; i < 4; i++)
			printf("%s\t\tEHT bw=20 MHz, max NSS for MCS %s: Rx=%u, Tx=%u\n",
			       pre, mcs[i],
			       mcs_set[i] & 0xf, mcs_set[i] >> 4);
	} else {
		if (he_phy_cap[0] & (BIT(2) << 8)) {
			for (i = 0; i < 3; i++)
				printf("%s\t\tEHT bw <= 80 MHz, max NSS for MCS %s: Rx=%u, Tx=%u\n",
				       pre, mcs[i + 1],
				       mcs_set[i] & 0xf, mcs_set[i] >> 4);
		}
		mcs_set += 3;

		if (he_phy_cap[0] & (BIT(3) << 8)) {
			for (i = 0; i < 3; i++)
				printf("%s\t\tEHT bw=160 MHz, max NSS for MCS %s: Rx=%u, Tx=%u\n",
				       pre, mcs[i + 1],
				       mcs_set[i] & 0xf, mcs_set[i] >> 4);
		}

		mcs_set += 3;
		if (band == NL80211_BAND_6GHZ && (phy_cap[0] & BIT(1))) {
			for (i = 0; i < 3; i++)
				printf("%s\t\tEHT bw=320 MHz, max NSS for MCS %s: Rx=%u, Tx=%u\n",
				       pre, mcs[i + 1],
				       mcs_set[i] & 0xf, mcs_set[i] >> 4);
		}
	}

	if (ppet && ppet_len && (phy_cap[1] & BIT(11))) {
		printf("%s\t\tEHT PPE Thresholds ", pre);
		for (i = 0; i < ppet_len; i++)
			if (ppet[i])
				printf("0x%02x ", ppet[i]);
		printf("\n");
	}
}

void print_eht_info(struct nlattr *nl_iftype, int band)
{
	struct nlattr *tb[NL80211_BAND_IFTYPE_ATTR_MAX + 1];
	__u8 mac_cap[2] = { 0 };
	__u32 phy_cap[2] = { 0 };
	__u8 mcs_set[13] = { 0 };
	__u8 ppet[31] = { 0 };
	__u16 he_phy_cap[6] = { 0 };
	size_t len, mcs_len = 0, ppet_len = 0;

	nla_parse(tb, NL80211_BAND_IFTYPE_ATTR_MAX,
		  nla_data(nl_iftype), nla_len(nl_iftype), NULL);

	if (!tb[NL80211_BAND_IFTYPE_ATTR_IFTYPES])
		return;

	printf("\t\tEHT Iftypes: ");
	print_iftype_line(tb[NL80211_BAND_IFTYPE_ATTR_IFTYPES]);
	printf("\n");

	if (tb[NL80211_BAND_IFTYPE_ATTR_EHT_CAP_MAC]) {
		len = nla_len(tb[NL80211_BAND_IFTYPE_ATTR_EHT_CAP_MAC]);
		if (len > sizeof(mac_cap))
			len = sizeof(mac_cap);
		memcpy(mac_cap,
		       nla_data(tb[NL80211_BAND_IFTYPE_ATTR_EHT_CAP_MAC]),
		       len);
	}

	if (tb[NL80211_BAND_IFTYPE_ATTR_EHT_CAP_PHY]) {
		len = nla_len(tb[NL80211_BAND_IFTYPE_ATTR_EHT_CAP_PHY]);

		if (len > sizeof(phy_cap))
			len = sizeof(phy_cap);

		memcpy(phy_cap,
		       nla_data(tb[NL80211_BAND_IFTYPE_ATTR_EHT_CAP_PHY]),
		       len);
	}

	if (tb[NL80211_BAND_IFTYPE_ATTR_EHT_CAP_MCS_SET]) {
		len = nla_len(tb[NL80211_BAND_IFTYPE_ATTR_EHT_CAP_MCS_SET]);
		if (len > sizeof(mcs_set))
			len = sizeof(mcs_set);
		memcpy(mcs_set,
		       nla_data(tb[NL80211_BAND_IFTYPE_ATTR_EHT_CAP_MCS_SET]),
		       len);

		// Assume that all parts of the MCS set are present
		mcs_len = sizeof(mcs_set);
	}

	if (tb[NL80211_BAND_IFTYPE_ATTR_EHT_CAP_PPE]) {
		len = nla_len(tb[NL80211_BAND_IFTYPE_ATTR_EHT_CAP_PPE]);
		if (len > sizeof(ppet))
			len = sizeof(ppet);
		memcpy(ppet,
		       nla_data(tb[NL80211_BAND_IFTYPE_ATTR_EHT_CAP_PPE]),
		       len);
		ppet_len = len;
	}

	if (tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_PHY]) {
		len = nla_len(tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_PHY]);

		if (len > sizeof(he_phy_cap) - 1)
			len = sizeof(he_phy_cap) - 1;
		memcpy(&((__u8 *)he_phy_cap)[1],
		       nla_data(tb[NL80211_BAND_IFTYPE_ATTR_HE_CAP_PHY]),
		       len);
	}

	__print_eht_capa(band, mac_cap, phy_cap, mcs_set, mcs_len, ppet, ppet_len,
			 he_phy_cap, true);
}

void print_he_capability(const uint8_t *ie, int len)
{
	const void *mac_cap, *phy_cap, *mcs_set;
	int mcs_len;
	int i = 0;

	mac_cap = &ie[i];
	i += 6;

	phy_cap = &ie[i];
	i += 11;

	mcs_set = &ie[i];
	mcs_len = len - i;

	__print_he_capa(mac_cap, phy_cap - 1, mcs_set, mcs_len, NULL, 0, false);
}

void iw_hexdump(const char *prefix, const __u8 *buf, size_t size)
{
	size_t i;

	printf("%s: ", prefix);
	for (i = 0; i < size; i++) {
		if (i && i % 16 == 0)
			printf("\n%s: ", prefix);
		printf("%02x ", buf[i]);
	}
	printf("\n\n");
}

int get_cf1(const struct chanmode *chanmode, unsigned long freq)
{
	unsigned int cf1 = freq, j;
	unsigned int bw80[] = { 5180, 5260, 5500, 5580, 5660, 5745, 5825,
				5955, 6035, 6115, 6195, 6275, 6355,
				6435, 6515, 6595, 6675, 6755, 6835,
				6195, 6995 };
	unsigned int bw160[] = { 5180, 5500, 5955, 6115, 6275, 6435,
				  6595, 6755, 6915 };
	/* based on 11be D2 E.1 Country information and operating classes */
	unsigned int bw320[] = {5955, 6115, 6275, 6435, 6595, 6755};

	switch (chanmode->width) {
	case NL80211_CHAN_WIDTH_80:
	case NL80211_CHAN_WIDTH_80P80:
	        /* setup center_freq1 */
		for (j = 0; j < ARRAY_SIZE(bw80); j++) {
			if (freq >= bw80[j] && freq < bw80[j] + 80)
				break;
		}

		if (j == ARRAY_SIZE(bw80))
			break;

		cf1 = bw80[j] + 30;
		break;
	case NL80211_CHAN_WIDTH_160:
		/* setup center_freq1 */
		for (j = 0; j < ARRAY_SIZE(bw160); j++) {
			if (freq >= bw160[j] && freq < bw160[j] + 160)
				break;
		}

		if (j == ARRAY_SIZE(bw160))
			break;

		cf1 = bw160[j] + 70;
		break;
	case NL80211_CHAN_WIDTH_320:
		/* setup center_freq1 */
		for (j = 0; j < ARRAY_SIZE(bw320); j++) {
			if (freq >= bw320[j] && freq < bw320[j] + 160)
				break;
		}

		if (j == ARRAY_SIZE(bw320))
			break;

		cf1 = bw320[j] + 150;
		break;
	default:
		cf1 = freq + chanmode->freq1_diff;
		break;
	}

	return cf1;
}

int parse_random_mac_addr(struct nl_msg *msg, char *addrs)
{
	char *a_addr, *a_mask, *sep;
	unsigned char addr[ETH_ALEN], mask[ETH_ALEN];

	if (!*addrs) {
		/* randomise all but the multicast bit */
		NLA_PUT(msg, NL80211_ATTR_MAC, ETH_ALEN,
			"\x00\x00\x00\x00\x00\x00");
		NLA_PUT(msg, NL80211_ATTR_MAC_MASK, ETH_ALEN,
			"\x01\x00\x00\x00\x00\x00");
		return 0;
	}

	if (*addrs != '=')
		return 1;

	addrs++;
	sep = strchr(addrs, '/');
	a_addr = addrs;

	if (!sep)
		return 1;

	*sep = 0;
	a_mask = sep + 1;
	if (mac_addr_a2n(addr, a_addr) || mac_addr_a2n(mask, a_mask))
		return 1;

	NLA_PUT(msg, NL80211_ATTR_MAC, ETH_ALEN, addr);
	NLA_PUT(msg, NL80211_ATTR_MAC_MASK, ETH_ALEN, mask);

	return 0;
 nla_put_failure:
	return -ENOBUFS;
}
