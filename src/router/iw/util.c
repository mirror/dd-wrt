#include <ctype.h>
#include <netlink/attr.h>
#include <errno.h>
#include <stdbool.h>
#include "iw.h"
#include "nl80211.h"

void mac_addr_n2a(char *mac_addr, unsigned char *arg)
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
/*
 * sed 's%^\tNL80211_CMD_%%;t n;d;:n s%^\([^=]*\),.*%\t[NL80211_CMD_\1] = \"\L\1\",%;t;d' nl80211.h | grep -v "reserved"
 */
	[NL80211_CMD_UNSPEC] = "unspec",
	[NL80211_CMD_GET_WIPHY] = "get_wiphy",
	[NL80211_CMD_SET_WIPHY] = "set_wiphy",
	[NL80211_CMD_NEW_WIPHY] = "new_wiphy",
	[NL80211_CMD_DEL_WIPHY] = "del_wiphy",
	[NL80211_CMD_GET_INTERFACE] = "get_interface",
	[NL80211_CMD_SET_INTERFACE] = "set_interface",
	[NL80211_CMD_NEW_INTERFACE] = "new_interface",
	[NL80211_CMD_DEL_INTERFACE] = "del_interface",
	[NL80211_CMD_GET_KEY] = "get_key",
	[NL80211_CMD_SET_KEY] = "set_key",
	[NL80211_CMD_NEW_KEY] = "new_key",
	[NL80211_CMD_DEL_KEY] = "del_key",
	[NL80211_CMD_GET_BEACON] = "get_beacon",
	[NL80211_CMD_SET_BEACON] = "set_beacon",
	[NL80211_CMD_START_AP] = "start_ap",
	[NL80211_CMD_STOP_AP] = "stop_ap",
	[NL80211_CMD_GET_STATION] = "get_station",
	[NL80211_CMD_SET_STATION] = "set_station",
	[NL80211_CMD_NEW_STATION] = "new_station",
	[NL80211_CMD_DEL_STATION] = "del_station",
	[NL80211_CMD_GET_MPATH] = "get_mpath",
	[NL80211_CMD_SET_MPATH] = "set_mpath",
	[NL80211_CMD_NEW_MPATH] = "new_mpath",
	[NL80211_CMD_DEL_MPATH] = "del_mpath",
	[NL80211_CMD_SET_BSS] = "set_bss",
	[NL80211_CMD_SET_REG] = "set_reg",
	[NL80211_CMD_REQ_SET_REG] = "req_set_reg",
	[NL80211_CMD_GET_MESH_CONFIG] = "get_mesh_config",
	[NL80211_CMD_SET_MESH_CONFIG] = "set_mesh_config",
	[NL80211_CMD_GET_REG] = "get_reg",
	[NL80211_CMD_GET_SCAN] = "get_scan",
	[NL80211_CMD_TRIGGER_SCAN] = "trigger_scan",
	[NL80211_CMD_NEW_SCAN_RESULTS] = "new_scan_results",
	[NL80211_CMD_SCAN_ABORTED] = "scan_aborted",
	[NL80211_CMD_REG_CHANGE] = "reg_change",
	[NL80211_CMD_AUTHENTICATE] = "authenticate",
	[NL80211_CMD_ASSOCIATE] = "associate",
	[NL80211_CMD_DEAUTHENTICATE] = "deauthenticate",
	[NL80211_CMD_DISASSOCIATE] = "disassociate",
	[NL80211_CMD_MICHAEL_MIC_FAILURE] = "michael_mic_failure",
	[NL80211_CMD_REG_BEACON_HINT] = "reg_beacon_hint",
	[NL80211_CMD_JOIN_IBSS] = "join_ibss",
	[NL80211_CMD_LEAVE_IBSS] = "leave_ibss",
	[NL80211_CMD_TESTMODE] = "testmode",
	[NL80211_CMD_CONNECT] = "connect",
	[NL80211_CMD_ROAM] = "roam",
	[NL80211_CMD_DISCONNECT] = "disconnect",
	[NL80211_CMD_SET_WIPHY_NETNS] = "set_wiphy_netns",
	[NL80211_CMD_GET_SURVEY] = "get_survey",
	[NL80211_CMD_NEW_SURVEY_RESULTS] = "new_survey_results",
	[NL80211_CMD_SET_PMKSA] = "set_pmksa",
	[NL80211_CMD_DEL_PMKSA] = "del_pmksa",
	[NL80211_CMD_FLUSH_PMKSA] = "flush_pmksa",
	[NL80211_CMD_REMAIN_ON_CHANNEL] = "remain_on_channel",
	[NL80211_CMD_CANCEL_REMAIN_ON_CHANNEL] = "cancel_remain_on_channel",
	[NL80211_CMD_SET_TX_BITRATE_MASK] = "set_tx_bitrate_mask",
	[NL80211_CMD_REGISTER_FRAME] = "register_frame",
	[NL80211_CMD_FRAME] = "frame",
	[NL80211_CMD_FRAME_TX_STATUS] = "frame_tx_status",
	[NL80211_CMD_SET_POWER_SAVE] = "set_power_save",
	[NL80211_CMD_GET_POWER_SAVE] = "get_power_save",
	[NL80211_CMD_SET_CQM] = "set_cqm",
	[NL80211_CMD_NOTIFY_CQM] = "notify_cqm",
	[NL80211_CMD_SET_CHANNEL] = "set_channel",
	[NL80211_CMD_SET_WDS_PEER] = "set_wds_peer",
	[NL80211_CMD_FRAME_WAIT_CANCEL] = "frame_wait_cancel",
	[NL80211_CMD_JOIN_MESH] = "join_mesh",
	[NL80211_CMD_LEAVE_MESH] = "leave_mesh",
	[NL80211_CMD_UNPROT_DEAUTHENTICATE] = "unprot_deauthenticate",
	[NL80211_CMD_UNPROT_DISASSOCIATE] = "unprot_disassociate",
	[NL80211_CMD_NEW_PEER_CANDIDATE] = "new_peer_candidate",
	[NL80211_CMD_GET_WOWLAN] = "get_wowlan",
	[NL80211_CMD_SET_WOWLAN] = "set_wowlan",
	[NL80211_CMD_START_SCHED_SCAN] = "start_sched_scan",
	[NL80211_CMD_STOP_SCHED_SCAN] = "stop_sched_scan",
	[NL80211_CMD_SCHED_SCAN_RESULTS] = "sched_scan_results",
	[NL80211_CMD_SCHED_SCAN_STOPPED] = "sched_scan_stopped",
	[NL80211_CMD_SET_REKEY_OFFLOAD] = "set_rekey_offload",
	[NL80211_CMD_PMKSA_CANDIDATE] = "pmksa_candidate",
	[NL80211_CMD_TDLS_OPER] = "tdls_oper",
	[NL80211_CMD_TDLS_MGMT] = "tdls_mgmt",
	[NL80211_CMD_UNEXPECTED_FRAME] = "unexpected_frame",
	[NL80211_CMD_PROBE_CLIENT] = "probe_client",
	[NL80211_CMD_REGISTER_BEACONS] = "register_beacons",
	[NL80211_CMD_UNEXPECTED_4ADDR_FRAME] = "unexpected_4addr_frame",
	[NL80211_CMD_SET_NOACK_MAP] = "set_noack_map",
	[NL80211_CMD_CH_SWITCH_NOTIFY] = "ch_switch_notify",
	[NL80211_CMD_START_P2P_DEVICE] = "start_p2p_device",
	[NL80211_CMD_STOP_P2P_DEVICE] = "stop_p2p_device",
	[NL80211_CMD_CONN_FAILED] = "conn_failed",
	[NL80211_CMD_SET_MCAST_RATE] = "set_mcast_rate",
	[NL80211_CMD_SET_MAC_ACL] = "set_mac_acl",
	[NL80211_CMD_RADAR_DETECT] = "radar_detect",
	[NL80211_CMD_GET_PROTOCOL_FEATURES] = "get_protocol_features",
	[NL80211_CMD_UPDATE_FT_IES] = "update_ft_ies",
	[NL80211_CMD_FT_EVENT] = "ft_event",
	[NL80211_CMD_CRIT_PROTOCOL_START] = "crit_protocol_start",
	[NL80211_CMD_CRIT_PROTOCOL_STOP] = "crit_protocol_stop",
	[NL80211_CMD_GET_COALESCE] = "get_coalesce",
	[NL80211_CMD_SET_COALESCE] = "set_coalesce",
	[NL80211_CMD_CHANNEL_SWITCH] = "channel_switch",
	[NL80211_CMD_VENDOR] = "vendor",
	[NL80211_CMD_SET_QOS_MAP] = "set_qos_map",
	[NL80211_CMD_ADD_TX_TS] = "add_tx_ts",
	[NL80211_CMD_DEL_TX_TS] = "del_tx_ts",
	[NL80211_CMD_GET_MPP] = "get_mpp",
	[NL80211_CMD_JOIN_OCB] = "join_ocb",
	[NL80211_CMD_LEAVE_OCB] = "leave_ocb",
	[NL80211_CMD_CH_SWITCH_STARTED_NOTIFY] = "ch_switch_started_notify",
	[NL80211_CMD_TDLS_CHANNEL_SWITCH] = "tdls_channel_switch",
	[NL80211_CMD_TDLS_CANCEL_CHANNEL_SWITCH] = "tdls_cancel_channel_switch",
	[NL80211_CMD_WIPHY_REG_CHANGE] = "wiphy_reg_change",
	[NL80211_CMD_ABORT_SCAN] = "abort_scan",
	[NL80211_CMD_START_NAN] = "start_nan",
	[NL80211_CMD_STOP_NAN] = "stop_nan",
	[NL80211_CMD_ADD_NAN_FUNCTION] = "add_nan_function",
	[NL80211_CMD_DEL_NAN_FUNCTION] = "del_nan_function",
	[NL80211_CMD_CHANGE_NAN_CONFIG] = "change_nan_config",
	[NL80211_CMD_NAN_MATCH] = "nan_match",
	[NL80211_CMD_SET_MULTICAST_TO_UNICAST] = "set_multicast_to_unicast",
	[NL80211_CMD_UPDATE_CONNECT_PARAMS] = "update_connect_params",
	[NL80211_CMD_SET_PMK] = "set_pmk",
	[NL80211_CMD_DEL_PMK] = "del_pmk",
	[NL80211_CMD_PORT_AUTHORIZED] = "port_authorized",
//	[NL80211_CMD_RELOAD_REGDB] = "reload_regdb",
#ifdef CONFIG_TDMA
	[NL80211_CMD_JOIN_TDMA] = "join_tdma",
	[NL80211_CMD_LEAVE_TDMA] = "leave_tdma",
#endif
};

static char cmdbuf[100];

#if 0
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
	case NL80211_BAND_60GHZ:
		if (chan < 5)
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
	else if (freq > 4800 && freq < 5005)
		return (freq - 4000) / 5;
	else if (freq <= 45000) /* DMG band lower limit */
		return (freq - 5000) / 5;
	else if (freq >= 58320 && freq <= 64800)
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
		return 0x000FAC02;
	if (!strcmp(cipher_str, "CCMP") || !strcmp(cipher_str, "CCMP-128"))
		return 0x000FAC04;
	if (!strcmp(cipher_str, "GCMP") || !strcmp(cipher_str, "GCMP-128"))
		return 0x000FAC08;
	if (!strcmp(cipher_str, "GCMP-256"))
		return 0x000FAC09;
	if (!strcmp(cipher_str, "CCMP-256"))
		return 0x000FAC0A;
	return -EINVAL;
}

int parse_keys(struct nl_msg *msg, char **argv, int argc)
{
	struct nlattr *keys;
	int i = 0;
	bool have_default = false;
	char *arg = *argv;
	char keybuf[13];
	int pos = 0;

	if (!argc)
		return 1;

	if (!memcmp(&arg[pos], "psk", 3)) {
		char psk_keybuf[32];
		int cipher_suite, akm_suite;

		if (argc < 4)
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

		argv++;
		argc--;
		arg = *argv;

		akm_suite = parse_akm_suite(arg);
		if (akm_suite < 0)
			goto explain;

		NLA_PUT_U32(msg, NL80211_ATTR_AKM_SUITES, akm_suite);

		argv++;
		argc--;
		arg = *argv;

		cipher_suite = parse_cipher_suite(arg);
		if (cipher_suite < 0)
			goto explain;

		NLA_PUT_U32(msg, NL80211_ATTR_CIPHER_SUITES_PAIRWISE, cipher_suite);

		argv++;
		argc--;
		arg = *argv;

		cipher_suite = parse_cipher_suite(arg);
		if (cipher_suite < 0)
			goto explain;

		NLA_PUT_U32(msg, NL80211_ATTR_CIPHER_SUITE_GROUP, cipher_suite);

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

		arg = *argv;
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
		case 5:
			NLA_PUT_U32(msg, NL80211_KEY_CIPHER, 0x000FAC01);
			keylen = 5;
			break;
		case 26:
			keydata = hex2bin(keydata, keybuf);
		case 13:
			NLA_PUT_U32(msg, NL80211_KEY_CIPHER, 0x000FAC05);
			keylen = 13;
			break;
		default:
			goto explain;
		}

		if (!keydata)
			goto explain;

		NLA_PUT(msg, NL80211_KEY_DATA, keylen, keydata);

		argv++;
		argc--;

		/* one key should be TX key */
		if (!have_default && !argc)
			NLA_PUT_FLAG(msg, NL80211_KEY_DEFAULT);

		nla_nest_end(msg, key);
	} while (argc);

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
		need_cf1 = true;
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
 *   <channel> [NOHT|HT20|HT40+|HT40-|5MHz|10MHz|80MHz]
 *
 * And if frequency is set:
 *   <freq> [NOHT|HT20|HT40+|HT40-|5MHz|10MHz|80MHz]
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

void print_vht_info(__u32 capa, const __u8 *mcs)
{
	__u16 tmp;
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
	switch ((capa >> 2) & 3) {
	case 0: printf("neither 160 nor 80+80\n"); break;
	case 1: printf("160 MHz\n"); break;
	case 2: printf("160 MHz, 80+80 MHz\n"); break;
	case 3: printf("(reserved)\n");
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
	unsigned int vht80[] = { 5180, 5260, 5500, 5580, 5660, 5745, 5825 };

	switch (chanmode->width) {
	case NL80211_CHAN_WIDTH_80:
	case NL80211_CHAN_WIDTH_80P80:
	        /* setup center_freq1 */
		for (j = 0; j < ARRAY_SIZE(vht80); j++) {
			if (freq >= vht80[j] && freq < vht80[j] + 80)
				break;
		}

		if (j == ARRAY_SIZE(vht80))
			break;

		cf1 = vht80[j] + 30;
		break;
	case NL80211_CHAN_WIDTH_160:
	        /* setup center_freq1 */
		for (j = 0; j < ARRAY_SIZE(vht80); j++) {
			if (freq >= vht80[j] && freq < vht80[j] + 160)
				break;
		}

		if (j == ARRAY_SIZE(vht80))
			break;

		cf1 = vht80[j] + 70;
		break;
	default:
		cf1 = freq + chanmode->freq1_diff;
		break;
	}

	return cf1;
}
