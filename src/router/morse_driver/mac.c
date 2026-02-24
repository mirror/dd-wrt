/*
 * Copyright 2017-2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/crc32.h>
#include <net/mac80211.h>
#include <asm/div64.h>
#include <linux/kernel.h>

#include "morse.h"
#include "mac.h"
#include "s1g_ies.h"
#include "bus.h"
#include "debug.h"
#include "command.h"
#include "vendor.h"
#include "vendor_ie.h"
#include "mac_config.h"
#include "dot11ah/dot11ah.h"
#include "skb_header.h"
#include "ps.h"
#include "raw.h"
#include "twt.h"
#include "mbssid.h"
#include "watchdog.h"
#include "firmware.h"
#include "offload.h"
#include "mesh.h"
#ifdef CONFIG_MORSE_RC
#include "rc.h"
#else
#include "minstrel_rc.h"
#endif
#include "ipmon.h"
#include "wiphy.h"
#include "mesh.h"
#include "pv1.h"
#include "peer.h"
#ifdef CONFIG_MORSE_HW_TRACE
#include "hw_trace.h"
#endif
#include "led.h"
#include "monitor.h"

#define RATE(rate100m, _flags) { \
	.bitrate = (rate100m), \
	.flags = (_flags), \
	.hw_value = 0, \
}

#define CHAN5GHZ(channel, chflags) { \
	.band = NL80211_BAND_5GHZ, \
	.center_freq = 5000 + 5 * (channel), \
	.hw_value = (channel), \
	.flags = chflags, \
	.max_antenna_gain = 0, \
	.max_power = 22, \
}

#define STA_PRIV_TIMEOUT_MSEC		(2000)

/* Supported TX/RX MCS mask: 0x03FF -> Each bit represents MCS0-9 */
#define DEFAULT_MCS_RATE_MASK		IEEE80211_HT_MCS_RX_HIGHEST_MASK

/* Mask for MCS0-7 */
#define MCS_RATE_MASK_0_TO_7		GENMASK(7, 0)

/* Offset for MCS 8 and 9 bits */
#define MCS_RATE_MASK_8_TO_9_OFFSET	(8)

/* Max 32 for legacy BA. 8 for 1MHZ NDP BA. 16 for 2+MHZ NDP BA */
#define DOT11AH_BA_MAX_MPDU_PER_AMPDU	(32)

/* Default alpha-2 code */
#define USER_ASSIGNED_ALPHA "ZZ"

/** When automatically trying MCS0 before MCS10, this is how many MCS0 attempts to make */
#define MCS0_BEFORE_MCS10_COUNT		(1)

/* Arbitrary size limit for the filter command address list, to ensure that the command
 * does not exceed page/MTU size. This will be far greater than the number of filters
 * supported by the firmware.
 */
#define MCAST_FILTER_COUNT_MAX (1024 / sizeof(filter->addr_list[0]))

/* Calculation of average RSSI */
#define CALC_AVG_RSSI(_avg, _sample) ((((_avg) * 9 + (_sample)) / 10))

#define MORSE_ECSA_DBG(_m, _f, _a...)		morse_dbg(FEATURE_ID_ECSA, _m, _f, ##_a)
#define MORSE_ECSA_INFO(_m, _f, _a...)		morse_info(FEATURE_ID_ECSA, _m, _f, ##_a)
#define MORSE_ECSA_WARN(_m, _f, _a...)		morse_warn(FEATURE_ID_ECSA, _m, _f, ##_a)
#define MORSE_ECSA_ERR(_m, _f, _a...)		morse_err(FEATURE_ID_ECSA, _m, _f, ##_a)

/**
 * Interval in milliseconds at which a work queue is scheduled to evaluate the rate
 * to be used for multicast packets transmission
 */
#define MULTICAST_TX_RATE_EVAL_WORK_PERIOD_MS	60000

#define MORSE_HEALTH_CHECK_RETRIES 1

enum dot11ah_powersave_mode {
	POWERSAVE_MODE_DISABLED = 0x00,
	POWERSAVE_MODE_PROTOCOL_ENABLED = 0x01,
	POWERSAVE_MODE_FULLY_ENABLED = 0x02,
	POWERSAVE_MODE_UNKNOWN = 0xFF
};

enum morse_mac_mcs10_mode {
	MCS10_MODE_DISABLED = 0x00,
	MCS10_MODE_FORCED = 0x01,
	MCS10_MODE_AUTO = 0x02
};

/* Custom Module parameters */
/* On chip hardware encryption can be disabled through modparam */
static bool no_hwcrypt;
module_param(no_hwcrypt, bool, 0644);
MODULE_PARM_DESC(no_hwcrypt, "Disable on-chip hardware encryption");

/* TX/RX MCS mask. Default 0x3FF limits max MCS to 9 for both Tx and Rx. */
static uint mcs_mask __read_mostly = DEFAULT_MCS_RATE_MASK;
module_param(mcs_mask, uint, 0644);
MODULE_PARM_DESC(mcs_mask, "Supported MCS mask, e.g. MCS0-2 use mask 0x07");

/**
 * Set the MCS10 configuration
 * 0 - MCS10 disabled
 * 1 - MCS10 replaces MCS0
 * 2 - Initally try MCS0 and then MCS10
 */
static enum morse_mac_mcs10_mode mcs10_mode __read_mostly = MCS10_MODE_DISABLED;
module_param(mcs10_mode, uint, 0644);
MODULE_PARM_DESC(mcs10_mode, "MCS10 mode");

/* Enable/Disable channel survey */
static bool enable_survey __read_mostly = ENABLE_SURVEY_DEFAULT;
module_param(enable_survey, bool, 0644);
MODULE_PARM_DESC(enable_survey, "Enable channel survey");

/* Enable/Disable Subband transmission */
static enum morse_mac_subbands_mode
enable_subbands __read_mostly = SUBBANDS_MODE_ENABLED;
module_param(enable_subbands, uint, 0644);
MODULE_PARM_DESC(enable_subbands, "Enable subband transmission");

/* Enable/Disable Powersave */
static enum dot11ah_powersave_mode enable_ps __read_mostly = CONFIG_MORSE_POWERSAVE_MODE;
module_param(enable_ps, uint, 0644);
MODULE_PARM_DESC(enable_ps, "Enable PS");

/* Enable/Disable Powersave */
static bool enable_dynamic_ps_offload __read_mostly = true;
module_param(enable_dynamic_ps_offload, bool, 0644);
MODULE_PARM_DESC(enable_dynamic_ps_offload, "Enable dynamic PS firmware offload");

/*
 * When set to a value greater than 0, Thin LMAC Mode is enabled.
 */
static bool thin_lmac __read_mostly;
module_param(thin_lmac, bool, 0644);
MODULE_PARM_DESC(thin_lmac, "Thin LMAC mode");

/*
 * When set to a value greater than 0, Virtual Station Test Mode is enabled, allowing up to
 * virtual_sta_max virtual interfaces to be configured in STA mode for emulating multiple physical
 * stations.
 */
static u32 virtual_sta_max __read_mostly;
module_param(virtual_sta_max, uint, 0644);
MODULE_PARM_DESC(virtual_sta_max, "Virtual STA test mode (max virtual STAs or 0 to disable)");

/* Enable/disable MBSSID IE addition in beacon/probe response */
static bool enable_mbssid_ie __read_mostly;
module_param(enable_mbssid_ie, bool, 0644);
MODULE_PARM_DESC(enable_mbssid_ie, "Enable MBSSID IE support in beacon and probe responses");

/* Allow/Disallow rate control to use SGI */
static bool enable_sgi_rc __read_mostly = true;
module_param(enable_sgi_rc, bool, 0644);
MODULE_PARM_DESC(enable_sgi_rc, "Enable SGI use in rate control");

/* Enable/Disable broadcasting travelling pilot support */
static bool enable_trav_pilot __read_mostly = true;
module_param(enable_trav_pilot, bool, 0644);
MODULE_PARM_DESC(enable_trav_pilot, "Enable travelling pilots");

/* Enable/Disable RTS/CTS for 8MHz (Disabled by default) */
static bool enable_rts_8mhz __read_mostly;
module_param(enable_rts_8mhz, bool, 0644);
MODULE_PARM_DESC(enable_rts_8mhz, "Enable RTS/CTS protection for 8MHz");

/* Use CTS-to-self in place of RTS/CTS */
static bool enable_cts_to_self __read_mostly;
module_param(enable_cts_to_self, bool, 0644);
MODULE_PARM_DESC(enable_cts_to_self, "Use CTS-to-self in place of RTS/CTS");

/* Parse the regulatory domain, 2 char ISO-Alpha2 */
static char country[MORSE_COUNTRY_LEN] = CONFIG_MORSE_COUNTRY;
module_param_string(country, country, sizeof(country), 0644);
MODULE_PARM_DESC(country,
"The ISO/IEC alpha2 country code for the country in which this device is currently operating.");

/* Enable/Disable watchdog support */
static bool enable_watchdog __read_mostly = ENABLE_WATCHDOG_DEFAULT;
module_param(enable_watchdog, bool, 0644);
MODULE_PARM_DESC(enable_watchdog, "Enable watchdog");

/* Set watchdog interval. User can update the watchdog interval in run time */
static uint watchdog_interval_secs __read_mostly = 30;
module_param(watchdog_interval_secs, uint, 0644);
MODULE_PARM_DESC(watchdog_interval_secs, "Watchdog interval in seconds");

/* Enable/Disable watchdog reset */
static bool enable_watchdog_reset __read_mostly;
module_param(enable_watchdog_reset, bool, 0644);
MODULE_PARM_DESC(enable_watchdog_reset, "Enable driver reset from watchdog");

/* Set limit on rate chain: could be 1, 2, 3 or 4 */
static uint max_rates __read_mostly = INIT_MAX_RATES_NUM;
module_param(max_rates, uint, 0644);
MODULE_PARM_DESC(max_rates, "Maximum number of rates to try");

/* Set maximum rate attempts, could be 1, 2, 3 or 4 */
static uint max_rate_tries __read_mostly = 1;
module_param(max_rate_tries, uint, 0644);
MODULE_PARM_DESC(max_rate_tries, "Maximum retries per rate");

/* Set maximum aggregation count */
static uint max_aggregation_count __read_mostly;
module_param(max_aggregation_count, uint, 0644);
MODULE_PARM_DESC(max_aggregation_count, "Maximum number of aggregated packets we can receive");

/* Enable/Disable RAW */
static bool enable_raw __read_mostly = true;
module_param(enable_raw, bool, 0644);
MODULE_PARM_DESC(enable_raw, "Enable RAW");

/* Enable/Disable mac80211 pull interface for airtime fairness */
static bool enable_airtime_fairness __read_mostly;
module_param(enable_airtime_fairness, bool, 0644);
MODULE_PARM_DESC(enable_airtime_fairness, "Enable mac80211 pull interface for airtime fairness");

/* Enable/disable the mac802.11 connection monitor */
static bool enable_mac80211_connection_monitor __read_mostly;
module_param(enable_mac80211_connection_monitor, bool, 0644);
MODULE_PARM_DESC(enable_mac80211_connection_monitor, "Enable mac80211 connection monitor");

/* Enable/disable the TWT feature */
static bool enable_twt __read_mostly = true;
module_param(enable_twt, bool, 0644);
MODULE_PARM_DESC(enable_twt, "Enable TWT support");

/* Maximum TX power (default) */
static int tx_max_power_mbm __read_mostly = 2200;
module_param(tx_max_power_mbm, int, 0644);
MODULE_PARM_DESC(tx_max_power_mbm, "Maximum transmitted power in mbm");

/* Set maximum multicast frames after DTIM (0 - Do not limit) */
static uint max_mc_frames __read_mostly = MORSE_MAX_MC_FRAMES_AFTER_DTIM;
module_param(max_mc_frames, uint, 0644);
MODULE_PARM_DESC(max_mc_frames, "Maximum multicast frames after DTIM (0 for unlimited)");

/* Enable CAC (Centralized Authentication Control) (AP mode only) */
static bool enable_cac __read_mostly;
module_param(enable_cac, bool, 0644);
MODULE_PARM_DESC(enable_cac, "Enable Centralized Authentication Control (CAC)");

/* Enable Monitoring of Beacon Change Seq (STA mode only) */
static bool enable_bcn_change_seq_monitor __read_mostly;
module_param(enable_bcn_change_seq_monitor, bool, 0644);
MODULE_PARM_DESC(enable_bcn_change_seq_monitor,
	"Enable monitoring of Change Sequence field in S1G Beacon");

/* Enable/Disable FW ARP response offloading */
static bool enable_arp_offload __read_mostly = ENABLE_ARP_OFFLOAD_DEFAULT;
module_param(enable_arp_offload, bool, 0644);
MODULE_PARM_DESC(enable_arp_offload, "Enable ARP offload");

static bool enable_dhcpc_offload __read_mostly = ENABLE_DHCP_OFFLOAD_DEFAULT;
module_param(enable_dhcpc_offload, bool, 0644);
MODULE_PARM_DESC(enable_dhcpc_offload, "Enable DHCP client offload");

/* Enable/Disable FW IBSS Probe Req Filtering */
bool enable_ibss_probe_filtering __read_mostly = true;
module_param(enable_ibss_probe_filtering, bool, 0644);
MODULE_PARM_DESC(enable_ibss_probe_filtering, "Enable IBSS probe request filtering in firmware");

char dhcpc_lease_update_script[DHCPC_LEASE_UPDATE_SCRIPT_NAME_SIZE_MAX] =
	"/morse/scripts/dhcpc_update.sh";
module_param_string(dhcpc_lease_update_script, dhcpc_lease_update_script,
		    sizeof(dhcpc_lease_update_script), 0644);
MODULE_PARM_DESC(dhcpc_lease_update_script, "Path to script called on DHCP lease updates");

/* Enable/Disable automatic duty cycle based on regulatory domain */
static bool enable_auto_duty_cycle __read_mostly = true;
module_param(enable_auto_duty_cycle, bool, 0644);
MODULE_PARM_DESC(enable_auto_duty_cycle, "Enable automatic duty cycling setting");

/**
 * Configure the duty cycle mode of operation when enabled.
 * 0 - SPREAD mode (default)
 * 1 - BURST mode
 */
static enum morse_cmd_duty_cycle_mode duty_cycle_mode
	__read_mostly = MORSE_CMD_DUTY_CYCLE_MODE_SPREAD;
module_param(duty_cycle_mode, uint, 0644);
MODULE_PARM_DESC(duty_cycle_mode, "Duty cycle mode when automatic duty cycling enabled");

/** Default Duty Cycle probe retry threshold */
#define MORSE_DUTY_CYCLE_PROBE_RETRY_THRESHOLD (2500)
/** Maximum Duty Cycle probe retry threshold */
#define MORSE_DUTY_CYCLE_PROBE_RETRY_THRESHOLD_MAX (10000)

/**
 * Duty Cycle threshold at or below which retries will be disabled for probe requests and responses.
 * The value is in 100ths of a percent, between 1 and 10000. E.g. 2500 is 25%.
 * A value of 10000 disables all probe request/response retries.
 */
static int duty_cycle_probe_retry_threshold __read_mostly = MORSE_DUTY_CYCLE_PROBE_RETRY_THRESHOLD;
module_param(duty_cycle_probe_retry_threshold, uint, 0644);
MODULE_PARM_DESC(duty_cycle_probe_retry_threshold,
	"Duty Cycle threshold at which to disable retries for probe requests and responses");

/* Enable/Disable automatic minimum packet spacing configuration based on regulatory domain */
static bool enable_auto_mpsw __read_mostly = true;
module_param(enable_auto_mpsw, bool, 0644);
MODULE_PARM_DESC(enable_auto_mpsw, "Enable automatic minimum packet spacing window setting");

/* Enable/disable FullMAC mode */
static bool enable_wiphy;
module_param(enable_wiphy, bool, 0644);
MODULE_PARM_DESC(enable_wiphy, "Enable FullMAC (Wiphy) interface");

/* OCS type */
uint ocs_type __read_mostly = MORSE_CMD_OCS_TYPE_RAW;
module_param(ocs_type, uint, 0644);
MODULE_PARM_DESC(ocs_type, "OCS Type (0: use QoS Null frames, 1: use RAW)");

/* Enable/Disable multicast whitelisting */
bool enable_mcast_whitelist __read_mostly = true;
module_param(enable_mcast_whitelist, bool, 0644);
MODULE_PARM_DESC(enable_mcast_whitelist, "Enable multicast whitelisting");

/* Enable multicast rate control */
static bool enable_mcast_rate_control __read_mostly;
module_param(enable_mcast_rate_control, bool, 0644);
MODULE_PARM_DESC(enable_mcast_rate_control,
	"Enable multicast rate control tracking to transmit at highest possible rate, bandwidth and GI");

/* Enable/Disable automatic logging of modparams on boot */
static bool log_modparams_on_boot __read_mostly = true;
module_param(log_modparams_on_boot, bool, 0644);
MODULE_PARM_DESC(log_modparams_on_boot, "Log all module parameters during boot");

/* Enable/Disable page slicing (dev only) */
static bool enable_page_slicing __read_mostly;
module_param(enable_page_slicing, bool, 0644);
MODULE_PARM_DESC(enable_page_slicing, "Enable page slicing");

/* Enable/disable the PV1 frame support */
static bool enable_pv1 __read_mostly;
module_param(enable_pv1, bool, 0644);
MODULE_PARM_DESC(enable_pv1, "Enable PV1 frame support (dev only)");

static bool enable_hw_scan __read_mostly = true;
module_param(enable_hw_scan, bool, 0644);
MODULE_PARM_DESC(enable_hw_scan, "Enable SoftMAC hardware scan support");

static bool enable_sched_scan __read_mostly = true;
module_param(enable_sched_scan, bool, 0644);
MODULE_PARM_DESC(enable_sched_scan, "Enable scheduled scanning");

/* Enable/Disable always sending probe requests at 1MHz */
static bool enable_1mhz_probes __read_mostly = true;
module_param(enable_1mhz_probes, bool, 0644);
MODULE_PARM_DESC(enable_1mhz_probes, "Enable sending all probe requests at 1MHz");

/* Set clock source selection mode */
static enum morse_cmd_slow_clock_mode
slow_clock_mode __read_mostly = MORSE_CMD_SLOW_CLOCK_MODE_AUTO;
module_param(slow_clock_mode, uint, 0644);
MODULE_PARM_DESC(slow_clock_mode, "Slow clock source selection mode");

static struct ieee80211_channel mors_5ghz_channels[] = {
	/* UNII-1 */
	CHAN5GHZ(36, 0),
	CHAN5GHZ(40, 0),
	CHAN5GHZ(44, 0),
	CHAN5GHZ(48, 0),
	/* UNII-2 */
	CHAN5GHZ(52, 0),
	CHAN5GHZ(56, 0),
	CHAN5GHZ(60, 0),
	CHAN5GHZ(64, 0),
	CHAN5GHZ(100, 0),
	CHAN5GHZ(104, 0),
	CHAN5GHZ(108, 0),
	CHAN5GHZ(112, 0),
	CHAN5GHZ(116, 0),
	CHAN5GHZ(120, 0),
	CHAN5GHZ(124, 0),
	CHAN5GHZ(128, 0),
	CHAN5GHZ(132, 0),
	CHAN5GHZ(136, 0),
	/* UNII-3 */
	CHAN5GHZ(149, 0),
	CHAN5GHZ(153, 0),
	CHAN5GHZ(157, 0),
	CHAN5GHZ(161, 0),
	CHAN5GHZ(165, 0),
	CHAN5GHZ(169, 0),
	CHAN5GHZ(173, 0),
	CHAN5GHZ(177, 0),

	/* 40MHz mapping */
	/* UNII-1 */
	CHAN5GHZ(38, 0),
	CHAN5GHZ(46, 0),
	/* UNII-2 */
	CHAN5GHZ(54, 0),
	CHAN5GHZ(62, 0),
	CHAN5GHZ(102, 0),
	CHAN5GHZ(110, 0),
	CHAN5GHZ(118, 0),
	CHAN5GHZ(126, 0),
	CHAN5GHZ(134, 0),
	CHAN5GHZ(151, 0),
	CHAN5GHZ(159, 0),
	CHAN5GHZ(167, 0),
	CHAN5GHZ(175, 0),

	/* 80MHz mapping */
	CHAN5GHZ(42, 0),
	CHAN5GHZ(58, 0),
	CHAN5GHZ(106, 0),
	CHAN5GHZ(122, 0),
	CHAN5GHZ(155, 0),
	CHAN5GHZ(171, 0),

	/* 160MHz mapping */
	CHAN5GHZ(50, 0),
	CHAN5GHZ(114, 0),
	CHAN5GHZ(163, 0),
};

static struct ieee80211_rate mors_2ghz_rates[] = {
	RATE(5, 0),
	RATE(10, 0),		/* 0x02 = 1.0 Mbps, basic rates for 2.4GHz */
	RATE(15, 0),
	RATE(20, 0),		/* 0x04 = 2.0 Mbps, basic rates for 2.4GHz */
	RATE(25, 0),
	RATE(30, 0),
	RATE(35, 0),
	RATE(40, 0),
	RATE(45, 0),
	RATE(55, 0),		/* 0x0B = 5.5 Mbps, basic rates for 2.4GHz */
	RATE(60, 0),		/* 0x0C = 6.0 Mbps, basic rates for 5GHz */
	RATE(65, 0),
	RATE(70, 0),
	RATE(85, 0),
	RATE(90, 0),
	RATE(110, 0),		/* 0x16 = 11.0 Mbps, basic rates for 2.4GHz */
	RATE(120, 0),		/* 0x18 = 12.0 Mbps, basic rates for 5GHz */
	RATE(125, 0),
	RATE(135, 0),
	RATE(150, 0),
	RATE(180, 0),
	RATE(240, 0)		/* 0x30 = 24.0 Mbps, basic rates for 5GHz */
};

struct ieee80211_supported_band mors_band_5ghz = {
	.band = NL80211_BAND_5GHZ,
	.channels = mors_5ghz_channels,
	.n_channels = ARRAY_SIZE(mors_5ghz_channels),
	.bitrates = mors_2ghz_rates,
	.n_bitrates = ARRAY_SIZE(mors_2ghz_rates),
	.ht_cap = {
		   .cap = IEEE80211_HT_CAP_GRN_FLD |
		   (1 << IEEE80211_HT_CAP_RX_STBC_SHIFT) | IEEE80211_HT_CAP_MAX_AMSDU,
		   .ht_supported = 1,
		   .ampdu_factor = IEEE80211_HT_MAX_AMPDU_64K,
		   .ampdu_density = IEEE80211_HT_MPDU_DENSITY_NONE,
		   .mcs = {
			   .rx_mask[0] = (DEFAULT_MCS_RATE_MASK & MCS_RATE_MASK_0_TO_7),
			   .rx_mask[1] = (DEFAULT_MCS_RATE_MASK >> MCS_RATE_MASK_8_TO_9_OFFSET),
			   .rx_highest = __cpu_to_le16(0x41),
			   .tx_params = IEEE80211_HT_MCS_TX_DEFINED,
			   },
		    },
	.vht_cap = {
		    .vht_mcs = {
				.rx_highest = __cpu_to_le16(0x41),
				},
		     },

};

bool is_fullmac_mode(void)
{
	return enable_wiphy;
}

/* Returns true if only STA mode is supported */
static bool is_sta_mode_only(void)
{
	if (is_fullmac_mode() || is_virtual_sta_test_mode())
		return true;

	return false;
}

bool is_thin_lmac_mode(void)
{
	return (thin_lmac > 0);
}

bool is_virtual_sta_test_mode(void)
{
	return (virtual_sta_max > 0);
}

bool is_sw_crypto_mode(void)
{
	return no_hwcrypt;
}

bool morse_mac_ps_enabled(struct morse *mors)
{
	if (mors->bus_type == MORSE_HOST_BUS_TYPE_USB)
		return (enable_ps != POWERSAVE_MODE_DISABLED);

	return (enable_ps != POWERSAVE_MODE_DISABLED) &&
			mors->cfg->mm_ps_gpios_supported;
}

enum morse_cmd_slow_clock_mode morse_mac_slow_clock_mode(void)
{
	return slow_clock_mode;
}

static inline int morse_vif_max_tx_bw(struct morse_vif *mors_vif)
{
	int max_bw_mhz;
	struct morse_caps *capabs = &mors_vif->capabilities;

	max_bw_mhz = MORSE_CAPAB_SUPPORTED(capabs, 8MHZ) ? 8 :
		     MORSE_CAPAB_SUPPORTED(capabs, 4MHZ) ? 4 :
		     MORSE_CAPAB_SUPPORTED(capabs, 2MHZ) ? 2 : 1;

	return max_bw_mhz;
}

struct ieee80211_vif *morse_get_vif_from_tx_status(struct morse *mors,
						   struct morse_skb_tx_status *hdr_tx_status)
{
	u8 vif_id = MORSE_TX_CONF_FLAGS_VIF_ID_GET(le32_to_cpu(hdr_tx_status->flags));

	return morse_get_vif_from_vif_id(mors, vif_id);
}

struct ieee80211_vif *__morse_get_vif_from_vif_id(struct morse *mors, int vif_id)
{
	struct ieee80211_vif *vif = NULL;

	vif_id = array_index_nospec(vif_id, mors->max_vifs);

	if (likely(vif_id < mors->max_vifs && mors->vif))
		vif = mors->vif[vif_id];

	return vif;
}

struct ieee80211_vif *morse_get_vif_from_vif_id(struct morse *mors, int vif_id)
{
	struct ieee80211_vif *vif = NULL;

	spin_lock_bh(&mors->vif_list_lock);
	vif = __morse_get_vif_from_vif_id(mors, vif_id);
	spin_unlock_bh(&mors->vif_list_lock);

	return vif;
}

static void morse_vif_remove(struct morse *mors, u8 idx)
{
	spin_lock_bh(&mors->vif_list_lock);
	mors->vif[idx] = NULL;
	spin_unlock_bh(&mors->vif_list_lock);
}

static void morse_vif_add(struct morse *mors, u8 idx, struct ieee80211_vif *vif)
{
	/* The vif array is indexed by the vif_id reported from the firmware.
	 * As such the driver must always obey the vif_id given. Warn if there is a stale entry,
	 * but overwrite it anyway.
	 * We do not need to free stale entries as the memory is managed by mac80211
	 */
	spin_lock_bh(&mors->vif_list_lock);
	WARN_ON(mors->vif[idx] && mors->vif[idx] != vif);
	mors->vif[idx] = vif;
	spin_unlock_bh(&mors->vif_list_lock);
}

struct ieee80211_vif *morse_get_vif(struct morse *mors)
{
	int vif_id = 0;
	struct ieee80211_vif *vif = NULL;

	spin_lock_bh(&mors->vif_list_lock);
	for (vif_id = 0; vif_id < mors->max_vifs; vif_id++) {
		vif = __morse_get_vif_from_vif_id(mors, vif_id);
		if (vif)
			break;
	}
	spin_unlock_bh(&mors->vif_list_lock);
	return vif;
}

struct ieee80211_vif *morse_get_vif_from_rx_status(struct morse *mors,
						   const struct morse_skb_rx_status *hdr_rx_status)
{
	u8 vif_id = MORSE_RX_STATUS_FLAGS_VIF_ID_GET(le32_to_cpu(hdr_rx_status->flags));

	return morse_get_vif_from_vif_id(mors, vif_id);
}

static struct ieee80211_vif *morse_get_first_vif_of_type(struct morse *mors,
							 enum nl80211_iftype type)
{
	int vif_id = 0;
	struct ieee80211_vif *vif = NULL;

	spin_lock_bh(&mors->vif_list_lock);
	for (vif_id = 0; vif_id < mors->max_vifs; vif_id++) {
		vif = __morse_get_vif_from_vif_id(mors, vif_id);
		if (vif && vif->type == type)
			break;

		vif = NULL;
	}
	spin_unlock_bh(&mors->vif_list_lock);
	return vif;
}

struct ieee80211_vif *morse_get_ap_vif(struct morse *mors)
{
	return morse_get_first_vif_of_type(mors, NL80211_IFTYPE_AP);
}

struct ieee80211_vif *morse_get_sta_vif(struct morse *mors)
{
	return morse_get_first_vif_of_type(mors, NL80211_IFTYPE_STATION);
}

struct ieee80211_vif *morse_get_ibss_vif(struct morse *mors)
{
	return morse_get_first_vif_of_type(mors, NL80211_IFTYPE_ADHOC);
}

struct ieee80211_vif *morse_get_mesh_vif(struct morse *mors)
{
	return morse_get_first_vif_of_type(mors, NL80211_IFTYPE_MESH_POINT);
}

char *morse_vif_name(struct ieee80211_vif *vif)
{
	struct wireless_dev *wdev = ieee80211_vif_to_wdev(vif);

	if (!wdev)
		return "Unknown";

	return wdev->netdev->name;
}

#if KERNEL_VERSION(4, 12, 0) <= MAC80211_VERSION_CODE
static u8 morse_mac_rx_bw_to_skb_vht(struct morse *mors, u8 rx_bw_mhz)
{
	struct ieee80211_conf *conf = &mors->hw->conf;
	u8 default_rate = RATE_INFO_BW_20;

	/* Can't do 3 x subbands for channel does not support VHT80 or VHT160 */
	if (conf->chandef.width != NL80211_CHAN_WIDTH_80 &&
	    conf->chandef.width != NL80211_CHAN_WIDTH_160)
		return (rx_bw_mhz == 1) ? RATE_INFO_BW_20 : RATE_INFO_BW_40;

	switch (rx_bw_mhz) {
	case 1:
		return RATE_INFO_BW_20;
	case 2:
		return RATE_INFO_BW_40;
	case 4:
		return RATE_INFO_BW_80;
	case 8:
		return conf->chandef.width !=
		    NL80211_CHAN_WIDTH_160 ? RATE_INFO_BW_80 : RATE_INFO_BW_160;
	default:
		MORSE_WARN_RATELIMITED(mors, "%s: Invalid S1G bandwidth\n", __func__);
		return default_rate;
	}
}
#endif

static void morse_mac_apply_mcs10_mode_auto(struct morse *mors, struct morse_skb_tx_info *tx_info)
{
	u8 i;
	u8 j;
	int mcs0_first_idx = -1;
	int mcs0_last_idx = -1;

	/* Find out where our first and last MCS0 entries are. */
	for (i = 0; i < IEEE80211_TX_MAX_RATES; i++) {
		enum dot11_bandwidth bw_idx =
		    morse_ratecode_bw_index_get(tx_info->rates[i].morse_ratecode);

		if (bw_idx == DOT11_BANDWIDTH_1MHZ) {
			mcs0_last_idx = i;
			if (mcs0_first_idx == -1)
				mcs0_first_idx = i;
		}
		/* If the count is 0 then we are at the end of the table. Break to
		 * allow us to reuse i indicating the end of the table.
		 */
		if (tx_info->rates[i].count == 0)
			break;
	}

	/* If there aren't any MCS0 (at 1MHz) entries we are done. */
	if (mcs0_first_idx < 0)
		return;

	/*
	 * If we are in MCS10_MODE_AUTO add MCS10 counts to the table if they
	 * will fit.
	 * There should be three cases:
	 * - There is one MSC0 entry and the table is full -> do nothing
	 * - There is one MSC0 entry and the table has space -> adjust MSC0 down and
	 *   add MCS 10
	 * - There are multiple MCS0 entries -> replace entries after the first
	 *   with MCS 10
	 */
	/* Case 3 - replace additional entries. */
	if (mcs0_last_idx > mcs0_first_idx) {
		mors->debug.mcs_stats_tbl.mcs0.tx_count += tx_info->rates[mcs0_first_idx].count;
		for (j = mcs0_first_idx + 1; j < i; j++) {
			enum dot11_bandwidth bw_idx =
			    morse_ratecode_bw_index_get(tx_info->rates[j].morse_ratecode);
			u8 mcs_index =
			    morse_ratecode_mcs_index_get(tx_info->rates[j].morse_ratecode);
			if (mcs_index == 0 && bw_idx == DOT11_BANDWIDTH_1MHZ) {
				morse_ratecode_mcs_index_set(&tx_info->rates[j].morse_ratecode, 10);
				mors->debug.mcs_stats_tbl.mcs10.tx_count += tx_info->rates[j].count;
			}
		}
		/* Case 2 - add additional MCS10 entry. */
	} else if (mcs0_last_idx == mcs0_first_idx &&
		   i < (IEEE80211_TX_MAX_RATES)) {
		int pre_mcs10_mcs0_count = min_t(u8, tx_info->rates[mcs0_last_idx].count,
						 MCS0_BEFORE_MCS10_COUNT);
		int mcs10_count = tx_info->rates[mcs0_last_idx].count - pre_mcs10_mcs0_count;

		/* If there were less retries than our desired minimum MCS0 we don't
		 * add MCS10 retries.
		 */
		if (mcs10_count > 0) {
			/* Use the same flags for MCS10 as MCS0. */
			tx_info->rates[i].morse_ratecode =
					    tx_info->rates[mcs0_last_idx].morse_ratecode;
			morse_ratecode_mcs_index_set(&tx_info->rates[i].morse_ratecode, 10);
			tx_info->rates[mcs0_last_idx].count = pre_mcs10_mcs0_count;
			tx_info->rates[i].count = mcs10_count;
		}
		/* Update our statistics. */
		mors->debug.mcs_stats_tbl.mcs10.tx_count += mcs10_count;
		mors->debug.mcs_stats_tbl.mcs0.tx_count += pre_mcs10_mcs0_count;
		/* Case 1 full table - increment MCS0 count. */
	} else {
		for (j = mcs0_first_idx; j < IEEE80211_TX_MAX_RATES; j++) {
			u8 mcs_index =
			    morse_ratecode_mcs_index_get(tx_info->rates[i].morse_ratecode);

			if (mcs_index == 0)
				mors->debug.mcs_stats_tbl.mcs0.tx_count += tx_info->rates[j].count;
		}
	}
}

static void morse_mac_apply_mcs10(struct morse *mors, struct morse_skb_tx_info *tx_info)
{
	u8 i;

	switch (mcs10_mode) {
	case MCS10_MODE_DISABLED:
		for (i = 0; i < IEEE80211_TX_MAX_RATES; i++) {
			enum dot11_bandwidth bw_idx =
			    morse_ratecode_bw_index_get(tx_info->rates[i].morse_ratecode);
			u8 mcs_index =
			    morse_ratecode_mcs_index_get(tx_info->rates[i].morse_ratecode);

			if (bw_idx == DOT11_BANDWIDTH_1MHZ && mcs_index == 0)
				mors->debug.mcs_stats_tbl.mcs0.tx_count += tx_info->rates[i].count;
		}
		return;
	case MCS10_MODE_FORCED:
		for (i = 0; i < IEEE80211_TX_MAX_RATES; i++) {
			enum dot11_bandwidth bw_idx =
			    morse_ratecode_bw_index_get(tx_info->rates[i].morse_ratecode);
			u8 mcs_index =
			    morse_ratecode_mcs_index_get(tx_info->rates[i].morse_ratecode);

			if (bw_idx == DOT11_BANDWIDTH_1MHZ && mcs_index == 0) {
				morse_ratecode_mcs_index_set(&tx_info->rates[i].morse_ratecode, 10);
				/* Update our statistics. */
				mors->debug.mcs_stats_tbl.mcs10.tx_count += tx_info->rates[i].count;
			}
		}
		return;
	case MCS10_MODE_AUTO:
		morse_mac_apply_mcs10_mode_auto(mors, tx_info);
	}
}

bool morse_mac_is_subband_enable(void)
{
	return (enable_subbands == SUBBANDS_MODE_ENABLED);
}

uint morse_mac_get_max_rate_tries(void)
{
	return max_rate_tries;
}

uint morse_mac_get_max_rate(void)
{
	return max_rates;
}

bool morse_mac_is_1mhz_probe_req_enabled(void)
{
	return enable_1mhz_probes;
}

bool morse_mac_is_rts_8mhz_enabled(void)
{
	return enable_rts_8mhz;
}

#ifdef CONFIG_MORSE_RC
static bool morse_mac_pkt_over_rts_threshold(struct morse *mors,
					     struct ieee80211_tx_info *info, struct sk_buff *skb)
{
	if (info->control.hw_key) {
		u8 ccmp_len;

		if (info->control.hw_key->keylen == 32)
			ccmp_len = IEEE80211_CCMP_256_HDR_LEN + IEEE80211_CCMP_256_MIC_LEN;
		else if (info->control.hw_key->keylen == 16)
			ccmp_len = IEEE80211_CCMP_HDR_LEN + IEEE80211_CCMP_MIC_LEN;
		else
			ccmp_len = 0;

		return ((skb->len + FCS_LEN + ccmp_len) > mors->rts_threshold);
	}
	return ((skb->len + FCS_LEN) > mors->rts_threshold);
}
#endif

void morse_mac_fill_tx_info(struct morse *mors,
				   struct morse_skb_tx_info *tx_info,
				   struct sk_buff *skb,
				   struct ieee80211_vif *vif,
				   int tx_bw_mhz, struct ieee80211_sta *sta)
{
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);
	struct morse_sta *mors_sta = NULL;
	__le16 fc = ((struct ieee80211_hdr *)skb->data)->frame_control;
	int op_bw_mhz = mors->custom_configs.channel_info.op_bw_mhz;
	int i;
	u8 ampdu_mmss = 0;
	u8 morse_mmss_offset = 0;
	bool rts_allowed = op_bw_mhz < 8 || enable_rts_8mhz;	/* Disable 8MHz RTS/CTS for now */

	if (sta)
		mors_sta = (struct morse_sta *)sta->drv_priv;

#ifdef CONFIG_MORSE_RC
	rts_allowed &= morse_mac_pkt_over_rts_threshold(mors, info, skb);
#else
	rts_allowed &= info->control.use_rts;
#endif

	morse_rc_sta_fill_tx_rates(mors, tx_info, skb, sta, tx_bw_mhz, rts_allowed);

	for (i = 0; i < IEEE80211_TX_MAX_RATES; i++) {
		/* SW-3200: WAR to prevent firmware crash when
		 * RTS/CTS is attempted to be sent at 4MHz
		 */
		if (rts_allowed) {
			/* || (info->control.flags & IEEE80211_TX_RC_USE_RTS_CTS)) */
			if (enable_cts_to_self)
				morse_ratecode_enable_cts2self(&tx_info->rates[i].morse_ratecode);
			else
				morse_ratecode_enable_rts(&tx_info->rates[i].morse_ratecode);
		}

		if (mors_vif->ctrl_resp_in_1mhz_en)
			morse_ratecode_enable_ctrl_resp_1mhz(&tx_info->rates[i].morse_ratecode);

		/* If travelling pilot reception is supported always use it */
		if (mors_sta && enable_trav_pilot &&
		    (mors_sta->trav_pilot_support == TRAV_PILOT_RX_1NSS ||
		     mors_sta->trav_pilot_support == TRAV_PILOT_RX_1_2_NSS))
			morse_ratecode_enable_trav_pilots(&tx_info->rates[i].morse_ratecode);

		if (info->control.rates[i].flags & IEEE80211_TX_RC_SHORT_GI)
			morse_ratecode_enable_sgi(&tx_info->rates[i].morse_ratecode);
	}

	/* Disable probe retries in constrained environments */
	if (mors->duty_cycle > 0 && mors->duty_cycle <= duty_cycle_probe_retry_threshold) {
		if (ieee80211_is_probe_req(fc) || ieee80211_is_probe_resp(fc)) {
			mors->debug.page_stats.tx_duty_cycle_retry_disabled++;
			tx_info->rates[0].count = 1;
			tx_info->rates[1].count = 0;
		}
	}

	/* Apply change of MCS0 to MCS10 if required. */
	morse_mac_apply_mcs10(mors, tx_info);

	tx_info->flags |= cpu_to_le32(MORSE_TX_CONF_FLAGS_VIF_ID_SET(mors_vif->id));

	if (info->flags & IEEE80211_TX_CTL_AMPDU)
		tx_info->flags |= cpu_to_le32(MORSE_TX_CONF_FLAGS_CTL_AMPDU);

	if (info->flags & IEEE80211_TX_CTL_SEND_AFTER_DTIM)
		tx_info->flags |= cpu_to_le32(MORSE_TX_CONF_FLAGS_SEND_AFTER_DTIM);

	if (info->flags & IEEE80211_TX_CTL_NO_PS_BUFFER) {
		tx_info->flags |= cpu_to_le32(MORSE_TX_CONF_NO_PS_BUFFER);

		if (info->flags & IEEE80211_TX_STATUS_EOSP)
			tx_info->flags |= cpu_to_le32(MORSE_TX_CONF_FLAGS_IMMEDIATE_REPORT);
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
	} else if (ieee80211_is_mgmt(fc) && !ieee80211_is_bufferable_mmpdu(fc)) {
#else
	} else if (ieee80211_is_mgmt(fc) && !ieee80211_is_bufferable_mmpdu(skb)) {
#endif
		/*
		 * Note, this does not handle the special IBSS case where probe responses
		 * to unicast probe requests should be buffered. That will need to be added
		 * if power save support for IBSS is added.
		 */
		info->flags |= IEEE80211_TX_CTL_NO_PS_BUFFER;
		tx_info->flags |= cpu_to_le32(MORSE_TX_CONF_NO_PS_BUFFER);
	}

	if (info->control.hw_key) {
		tx_info->flags |= cpu_to_le32(MORSE_TX_CONF_FLAGS_HW_ENCRYPT);

		tx_info->flags |=
		    cpu_to_le32(MORSE_TX_CONF_FLAGS_KEY_IDX_SET(info->control.hw_key->hw_key_idx));
	}

	tx_info->tid = skb->priority & IEEE80211_QOS_CTL_TID_MASK;
	if (ieee80211_is_data_qos(fc)) {
		u8 frame_tid = MORSE_IEEE80211_GET_TID((struct ieee80211_hdr *)skb->data);

		MORSE_WARN_ON_ONCE(FEATURE_ID_DEFAULT, tx_info->tid != frame_tid);
		tx_info->tid = frame_tid;
	}

	if (mors_sta) {
		tx_info->tid_params = mors_sta->tid_params[tx_info->tid];

		if (info->flags & IEEE80211_TX_CTL_CLEAR_PS_FILT) {
			if (mors_sta->tx_ps_filter_en)
				MORSE_DBG(mors, "TX ps filter cleared sta[%pM]\n", mors_sta->addr);
			mors_sta->tx_ps_filter_en = false;
		}
	}

	if (morse_is_pv1_protected_frame(skb))
		tx_info->flags |= cpu_to_le32(MORSE_TX_CONF_HAS_PV1_BPN_IN_BODY);

	/* Fill MMSS (Minimum MPDU start spacing) fields */
	if (morse_mac_is_iface_ap_type(vif)) {
		if (mors_sta) {
			ampdu_mmss = mors_sta->ampdu_mmss;
			morse_mmss_offset = mors_sta->vendor_info.morse_mmss_offset;
		}
	} else {
		ampdu_mmss = mors_vif->bss_ampdu_mmss;
		morse_mmss_offset = mors_vif->bss_vendor_info.morse_mmss_offset;
	}

	tx_info->mmss_params = (TX_INFO_MMSS_PARAMS_SET_MMSS(ampdu_mmss) |
				TX_INFO_MMSS_PARAMS_SET_MMSS_OFFSET(morse_mmss_offset));
}

static bool morse_mac_tx_ps_filtered_for_sta(struct morse *mors,
					     struct sk_buff *skb, struct ieee80211_sta *sta)
{
	struct morse_sta *mors_sta;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);

	if (!sta)
		return false;

	mors_sta = (struct morse_sta *)sta->drv_priv;

	if (!mors_sta->tx_ps_filter_en)
		return false;

	MORSE_DBG(mors, "Frame for sta[%pM] PS filtered\n", mors_sta->addr);
	mors->debug.page_stats.tx_ps_filtered++;

	info->flags |= IEEE80211_TX_STAT_TX_FILTERED;
	info->flags &= ~IEEE80211_TX_CTL_AMPDU;

	MORSE_IEEE80211_TX_STATUS(mors->hw, skb);
	return true;
}

void morse_mac_skb_free(struct morse *mors, struct sk_buff *skb)
{
	dev_kfree_skb_any(skb);
}

/**
 * morse_mac_set_s1g_capab() - Set S1G caps based on the chip capabilities.
 * @vif: The interface pointer to check and update.
 *
 * This functionality is required for both native S1G registration and for
 * translation layer. For translation it is required once when new interface
 * being added by morse_mac_ops_add_interface(). And, for native S1G it is
 * required for registration stage.
 *
 * Return: 0 on success or relevant error.
 */
static int morse_mac_set_s1g_capab(struct ieee80211_vif *vif)
{
	struct morse_vif *mors_vif;
	struct ieee80211_s1g_cap *s1g_capab;
	u8 s1g_mcs_map_entry;
	int i;

	if (!vif)
		return -ENOENT;

	mors_vif = (struct morse_vif *)vif->drv_priv;

	if (!mors_vif)
		return -ENOENT;

	s1g_capab = &mors_vif->s1g_cap_ie;

	memset(s1g_capab, 0, sizeof(*s1g_capab));

	/* Following the format given in Draft P802.11REVme_D3.0
	 * section 9.4.2.199.2 S1G Capabilities Information field
	 * Note these are 0 indexed in code, 1 indexed in the standard
	 */
	/* S1G Cap IE Octet 1 */
	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, S1G_LONG))
		s1g_capab->capab_info[0] |= S1G_CAP0_S1G_LONG;

	s1g_capab->capab_info[0] |= S1G_CAP0_SGI_1MHZ;
	s1g_capab->capab_info[0] |= MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, 2MHZ)  ?
					S1G_CAP0_SGI_2MHZ  : 0;
	s1g_capab->capab_info[0] |= MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, 4MHZ)  ?
					S1G_CAP0_SGI_4MHZ  : 0;
	s1g_capab->capab_info[0] |= MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, 8MHZ)  ?
					S1G_CAP0_SGI_8MHZ  : 0;
	s1g_capab->capab_info[0] |= MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, 16MHZ) ?
					S1G_CAP0_SGI_16MHZ : 0;

	/* SW-3993 - It is determined that for the current HaLow R1 test bed
	 * we have to signal 4MHz SGI support but not 4MHz width support.
	 * Hardcode it here, and TODO: Remove 4MHz SGI hard-coding
	 */
	s1g_capab->capab_info[0] |= S1G_CAP0_SGI_4MHZ;

	s1g_capab->capab_info[0] |=
		MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, 16MHZ) ? S1G_CAP0_SUPP_16MHZ :
		MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, 8MHZ) ? S1G_CAP0_SUPP_8MHZ :
		MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, 4MHZ) ? S1G_CAP0_SUPP_4MHZ : 0;

	/* S1G Cap IE Octet 3 */
	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, MU_BEAMFORMEE))
		s1g_capab->capab_info[2] |= S1G_CAP2_MU_BFEE;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, MU_BEAMFORMER))
		s1g_capab->capab_info[2] |= S1G_CAP2_MU_BFER;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, TRAVELING_PILOT_ONE_STREAM))
		s1g_capab->capab_info[2] |= S1G_CAP2_SET_TRAV_PILOT(TRAV_PILOT_RX_1NSS);
	else if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, TRAVELING_PILOT_TWO_STREAM))
		s1g_capab->capab_info[2] |= S1G_CAP2_SET_TRAV_PILOT(TRAV_PILOT_RX_1_2_NSS);

	/* S1G Cap IE Octet 4 */
	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, RD_RESPONDER))
		s1g_capab->capab_info[3] |= S1G_CAP3_RD_RESPONDER;

	s1g_capab->capab_info[3] |= S1G_CAP3_MPDU_MAX_LEN_3895;
	s1g_capab->capab_info[3] |=
	    S1G_CAP3_SET_MAX_AMPDU_LEN_EXP(mors_vif->capabilities.maximum_ampdu_length_exponent);
	s1g_capab->capab_info[3] |=
	    S1G_CAP3_SET_MIN_AMPDU_START_SPC(mors_vif->capabilities.ampdu_mss);

	/* S1G Cap IE Octet 5 */
	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, UPLINK_SYNC))
		s1g_capab->capab_info[4] |= S1G_CAP4_UPLINK_SYNC;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, DYNAMIC_AID))
		s1g_capab->capab_info[4] |= S1G_CAP4_DYNAMIC_AID;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, BAT))
		s1g_capab->capab_info[4] |= S1G_CAP4_BAT;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, TIM_ADE))
		s1g_capab->capab_info[4] |= S1G_CAP4_TIME_ADE;

	/* Enable non-TIM mode based on VIF config and capabilities */
	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, NON_TIM) &&
		mors_vif->enable_non_tim_mode)
		s1g_capab->capab_info[4] |= S1G_CAP4_NON_TIM;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, GROUP_AID))
		s1g_capab->capab_info[4] |= S1G_CAP4_GROUP_AID;

	if (vif->type == NL80211_IFTYPE_AP || vif->type == NL80211_IFTYPE_MESH_POINT) {
		/* In case of mixed sta support no need to set any bits */
		if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, STA_TYPE_SENSOR) &&
		    MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, STA_TYPE_NON_SENSOR))
			s1g_capab->capab_info[4] &= ~S1G_CAP4_STA_TYPE;
		else if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, STA_TYPE_SENSOR))
			s1g_capab->capab_info[4] |= S1G_CAP4_STA_TYPE_SENSOR;
		else if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, STA_TYPE_NON_SENSOR))
			s1g_capab->capab_info[4] |= S1G_CAP4_STA_TYPE_NON_SENSOR;
	} else if (vif->type == NL80211_IFTYPE_STATION) {
		if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, STA_TYPE_NON_SENSOR))
			s1g_capab->capab_info[4] |= S1G_CAP4_STA_TYPE_NON_SENSOR;
		else if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, STA_TYPE_SENSOR))
			s1g_capab->capab_info[4] |= S1G_CAP4_STA_TYPE_SENSOR;
	}

	/* S1G Cap IE Octet 6 */
	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, CAC))
		s1g_capab->capab_info[5] |= S1G_CAP5_CENT_AUTH_CONTROL;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, DAC))
		s1g_capab->capab_info[5] |= S1G_CAP5_DIST_AUTH_CONTROL;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, AMSDU))
		s1g_capab->capab_info[5] |= S1G_CAP5_AMSDU;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, AMPDU))
		s1g_capab->capab_info[5] |= S1G_CAP5_AMPDU;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, ASYMMETRIC_BA_SUPPORT))
		s1g_capab->capab_info[5] |= S1G_CAP5_ASYMMETRIC_BA;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, FLOW_CONTROL))
		s1g_capab->capab_info[5] |= S1G_CAP5_FLOW_CONTROL;

	/* TODO: Handle the following: */
	/* TXOP_SECTORIZATION */
	/* GROUP_SECTORIZATION */

	/* S1G Cap IE Octet 7 */
	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, OBSS_MITIGATION))
		s1g_capab->capab_info[6] |= S1G_CAP6_OBSS_MITIGATION;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, FRAGMENT_BA))
		s1g_capab->capab_info[6] |= S1G_CAP6_FRAGMENT_BA;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, NDP_PSPOLL))
		s1g_capab->capab_info[6] |= S1G_CAP6_NDP_PS_POLL;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, RAW))
		s1g_capab->capab_info[6] |= S1G_CAP6_RAW_OPERATION;

	/* Enable page slicing only for AP or STA */
	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, PAGE_SLICING))
		mors_vif->page_slicing_info.enabled = morse_mac_is_iface_infra_bss_type(vif) ?
			enable_page_slicing : false;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, TXOP_SHARING_IMPLICIT_ACK))
		s1g_capab->capab_info[6] |= S1G_CAP6_TXOP_SHARING_IMP_ACK;

	/* TODO: handle VHT Link Adaptation Capable field properly */
	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, HTC_VHT_MFB))
		s1g_capab->capab_info[6] |= S1G_CAP6_VHT_LINK_ADAPT;

	/* S1G Cap IE Octet 8 */
	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, TACK_AS_PSPOLL))
		s1g_capab->capab_info[7] |= S1G_CAP7_TACK_AS_PS_POLL;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, DUPLICATE_1MHZ))
		s1g_capab->capab_info[7] |= S1G_CAP7_DUP_1MHZ;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, MCS_NEGOTIATION))
		s1g_capab->capab_info[7] |= S1G_CAP7_DUP_1MHZ;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, 1MHZ_CONTROL_RESPONSE_PREAMBLE))
		s1g_capab->capab_info[7] |= S1G_CAP7_1MHZ_CTL_RESPONSE_PREAMBLE;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, NDP_BEAMFORMING_REPORT))
		s1g_capab->capab_info[7] |= S1G_CAP7_NDP_BFING_REPORT_POLL;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, UNSOLICIT_DYNAMIC_AID))
		s1g_capab->capab_info[7] |= S1G_CAP7_UNSOLICITED_DYN_AID;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, SECTOR_TRAINING))
		s1g_capab->capab_info[7] |= S1G_CAP7_SECTOR_TRAINING_OPERATION;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, TMP_PS_MODE_SWITCH))
		s1g_capab->capab_info[7] |= S1G_CAP7_TEMP_PS_MODE_SWITCH;

	/* S1G Cap IE Octet 9 */
	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, BDT))
		s1g_capab->capab_info[8] |= S1G_CAP8_BDT;

	if (vif->type == NL80211_IFTYPE_AP)
		s1g_capab->capab_info[8] |= S1G_CAP8_SET_COLOR(mors_vif->bss_color);

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, TWT_REQUESTER))
		s1g_capab->capab_info[8] |= S1G_CAP8_TWT_REQUEST;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, TWT_RESPONDER))
		s1g_capab->capab_info[8] |= S1G_CAP8_TWT_RESPOND;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, PV1))
		s1g_capab->capab_info[8] |= S1G_CAP8_PV1_FRAME;

	/* S1G Cap IE Octet 10 */
	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, LINK_ADAPTATION_WO_NDP_CMAC))
		s1g_capab->capab_info[9] |= S1G_CAP9_LINK_ADAPT_PER_CONTROL_RESPONSE;

	if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, MCS9) ||
	    MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, MCS8))
		s1g_mcs_map_entry = IEEE80211_VHT_MCS_SUPPORT_0_9;
	else
		s1g_mcs_map_entry = IEEE80211_VHT_MCS_SUPPORT_0_8;

	/* Draft P80211REVme_D3.0 section 9.4.2.199.3 Supported S1G-MCS and NSS Set field
	 * RX S1G-MCS MAP B0-B7
	 * Rx Highest Supported Long GI Data Rate B8-B16
	 * TX S1G-MCS MAP B17-B24
	 * TX Highest Supported Long GI Data Rate B25-B33
	 */
	for (i = 0; i < NL80211_S1G_NSS_MAX; i++) {
		if (i == 0 ||
		    (i == 1 && MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, 2SS)) ||
		    (i == 2 && MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, 3SS)) ||
		    (i == 3 && MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, 4SS))) {
			s1g_capab->supp_mcs_nss[0] |=
				(s1g_mcs_map_entry << (i * S1G_CAP_BITS_PER_MCS_NSS));

			continue;
		}

		s1g_capab->supp_mcs_nss[0] |= (IEEE80211_VHT_MCS_NOT_SUPPORTED <<
					       (i * S1G_CAP_BITS_PER_MCS_NSS));
	}
	s1g_capab->supp_mcs_nss[1] = 0x0;
	/* Assume TX MCS is the same as RX */
	s1g_capab->supp_mcs_nss[2] = ((s1g_capab->supp_mcs_nss[0] << 1) & 0xFE);
	s1g_capab->supp_mcs_nss[3] = ((s1g_capab->supp_mcs_nss[0] >> 7) & 0x01);
	s1g_capab->supp_mcs_nss[4] = 0x0;

	return 0;
}

/**
 * morse_mac_update_custom_s1g_capab() - Override chip S1G caps.
 * @mors_vif: The morse VIF struct.
 * @ies_mask: Contains array of information elements.
 * @vif_type: The (virtual) interface netlink types.
 *
 * Update a smaller set of S1G caps per management frame transmit operation.
 * These are runtime updates/configuration that can vary from the initial
 * chip caps such as user commands configurations.
 */
void morse_mac_update_custom_s1g_capab(struct morse_vif *mors_vif,
				       struct dot11ah_ies_mask *ies_mask,
				       enum nl80211_iftype vif_type)
{
	struct ieee80211_s1g_cap *s1g_capab;

	if (!ies_mask)
		return;

	s1g_capab = (struct ieee80211_s1g_cap *)ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr;

	if (!s1g_capab)
		return;

	if (!mors_vif->custom_configs->enable_sgi_rc)
		s1g_capab->capab_info[0] &= ~(S1G_CAP0_SGI_1MHZ | S1G_CAP0_SGI_2MHZ |
					      S1G_CAP0_SGI_4MHZ | S1G_CAP0_SGI_8MHZ |
					      S1G_CAP0_SGI_16MHZ);

	if (!mors_vif->custom_configs->enable_trav_pilot)
		s1g_capab->capab_info[2] &= ~S1G_CAP2_TRAVELING_PILOT;

	if (!mors_vif->custom_configs->enable_ampdu)
		s1g_capab->capab_info[3] &=
		    ~(S1G_CAP3_MAX_MPDU_LEN | S1G_CAP3_MAX_AMPDU_LEN_EXP | S1G_CAP3_MIN_MPDU_START);

	/* Determine user configured STA type */
	if (vif_type == NL80211_IFTYPE_AP || vif_type == NL80211_IFTYPE_MESH_POINT) {
		/* At this point s1g_capab was already updated according to FW
		 * capabilities. If sta_type is not STA_TYPE_MIXED (supporting both
		 * sensor and non-sensor) then the user cannot change this capability
		 */
		u8 sta_type = S1G_CAP4_GET_STA_TYPE(s1g_capab->capab_info[4]);

		if (sta_type == STA_TYPE_MIXED) {
			switch (mors_vif->custom_configs->sta_type) {
			case STA_TYPE_SENSOR:
				s1g_capab->capab_info[4] |= S1G_CAP4_STA_TYPE_SENSOR;
				break;
			case STA_TYPE_NON_SENSOR:
				s1g_capab->capab_info[4] |= S1G_CAP4_STA_TYPE_NON_SENSOR;
				break;
			}
		}
	} else if (vif_type == NL80211_IFTYPE_STATION) {
		if (mors_vif->custom_configs->sta_type == STA_TYPE_NON_SENSOR) {
			if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, STA_TYPE_NON_SENSOR))
				s1g_capab->capab_info[4] |=
				    S1G_CAP4_CLEAR_AND_SET_STA_TYPE(S1G_CAP4_STA_TYPE_NON_SENSOR);
		} else if (mors_vif->custom_configs->sta_type == STA_TYPE_SENSOR) {
			if (MORSE_CAPAB_SUPPORTED(&mors_vif->capabilities, STA_TYPE_SENSOR))
				s1g_capab->capab_info[4] |=
				    S1G_CAP4_CLEAR_AND_SET_STA_TYPE(S1G_CAP4_STA_TYPE_SENSOR);
		}
	}

	if (!morse_cac_is_enabled(mors_vif))
		s1g_capab->capab_info[5] &= ~S1G_CAP5_CENT_AUTH_CONTROL;

	if (!mors_vif->custom_configs->enable_ampdu)
		s1g_capab->capab_info[5] &= ~S1G_CAP5_AMPDU;

	if (!morse_raw_is_enabled(mors_vif))
		s1g_capab->capab_info[6] &= ~S1G_CAP6_RAW_OPERATION;

	if (!mors_vif->ctrl_resp_out_1mhz_en)
		s1g_capab->capab_info[7] &= ~S1G_CAP7_1MHZ_CTL_RESPONSE_PREAMBLE;

	if (!mors_vif->twt.requester)
		s1g_capab->capab_info[8] &= ~S1G_CAP8_TWT_REQUEST;

	if (!mors_vif->twt.responder)
		s1g_capab->capab_info[8] &= ~S1G_CAP8_TWT_RESPOND;
}

/**
 * morse_mac_capabilities_validate() - check caps and elements per frame type.
 * @mors: Morse chip struct
 * @ies_mask: array containing information elements of the input skb
 * @vif: pointer to the virtual interface
 * @skb: pointer to the inspected packet buffer
 *
 * Validate mandatory elements per frame type and compare custom user config
 * against incoming skb capabilities.
 *
 * return: true if checks pass or false if validation fail.
 */
static bool morse_mac_capabilities_validate(struct morse *mors,
					    struct dot11ah_ies_mask *ies_mask,
					    struct ieee80211_vif *vif, struct sk_buff *skb)
{
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;
	u8 s1g_caps_sta_type = 0;

	if (ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr)
		s1g_caps_sta_type =
		    ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr[4] & S1G_CAP4_STA_TYPE;

	if (ieee80211_is_probe_req(hdr->frame_control)) {
		if (!ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr)
			return false;

		/* If this isn't true, then this field is wrong, and we won't respond to it */
		/* Commented out for interop
		 *
		 if (ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr &&
		     s1g_caps_sta_type != S1G_CAP4_STA_TYPE_NON_SENSOR &&
		     s1g_caps_sta_type != S1G_CAP4_STA_TYPE_BOTH)
			return false;
		 */
	} else if (ieee80211_is_probe_resp(hdr->frame_control)) {
		/* Will have to discard this SSID'less probe response */
		if (!ies_mask->ies[WLAN_EID_SSID].ptr)
			return false;
	} else if (ieee80211_is_assoc_req(hdr->frame_control) ||
		   ieee80211_is_reassoc_req(hdr->frame_control)) {
		if (!ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr ||
		    !ies_mask->ies[WLAN_EID_AID_REQUEST].ptr)
			return false;

		/* If this isn't true, then this field is wrong, and we won't respond to it */
		/* Commented out for interop
		 *
		 if ((ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr) &&
		     s1g_caps_sta_type != S1G_CAP4_STA_TYPE_NON_SENSOR &&
		     s1g_caps_sta_type != S1G_CAP4_STA_TYPE_BOTH)
			 return false;
		 */

		if ((mors_vif->custom_configs->sta_type == STA_TYPE_NON_SENSOR &&
		     s1g_caps_sta_type != S1G_CAP4_STA_TYPE_NON_SENSOR) ||
		    (mors_vif->custom_configs->sta_type == STA_TYPE_SENSOR &&
		     s1g_caps_sta_type != S1G_CAP4_STA_TYPE_SENSOR))
			return false;

		/* Check that it's the length we are willing to accept */
		/* Commented out for interop:
		 *
		 * Another vendor is sending us an AID Request with optional fields filled.
		 *
		 if (ies_mask->ies[WLAN_EID_AID_REQUEST].ptr &&
		     ies_mask->ies[WLAN_EID_AID_REQUEST].ptr[1] != 1)
			 return false;
		 */
	} else if (ieee80211_is_assoc_resp(hdr->frame_control) ||
		   ieee80211_is_reassoc_resp(hdr->frame_control)) {
		if (!ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr ||
		    !ies_mask->ies[WLAN_EID_AID_RESPONSE].ptr)
			return false;

		/* If this isn't true, then this field is wrong, and we won't respond to it */
		if (s1g_caps_sta_type != S1G_CAP4_STA_TYPE_NON_SENSOR &&
		    s1g_caps_sta_type != S1G_CAP4_STA_TYPE_BOTH) {
			/* prevent this rejecting silently */
			MORSE_WARN(mors,
			   "Association rejected due to S1G Capabilities STA type mismatch\n");
			return false;
		}
	} else if (ieee80211_is_action(hdr->frame_control) &&
		   morse_dot11_is_mpm_frame((struct ieee80211_mgmt *)hdr) &&
		   morse_dot11_is_mpm_confirm_frame((struct ieee80211_mgmt *)hdr)) {
		if (!ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr)
			return false;
	}

	return true;
}

/**
 * morse_mac_process_s1g_caps() - Updates interface capabilities
 * @mors: Morse chip struct
 * @vif: pointer to the virtual interface
 * @skb: pointer to the incoming packet buffer
 * @ies_mask: array containing information elements of the input skb
 *
 * Process incoming skb capabilities and updates the vif interface and station.
 *
 * return: 0 for success or relevant error
 */
static int morse_mac_process_s1g_caps(struct morse *mors,
				       struct ieee80211_vif *vif,
				       struct sk_buff *skb, struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)skb->data;
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;
	struct morse_sta *mors_sta = NULL;
	struct ieee80211_sta *sta;
	int sta_max_bw;
	bool is_assoc_req = ieee80211_is_assoc_req(mgmt->frame_control) ||
		    ieee80211_is_reassoc_req(mgmt->frame_control);
	bool is_assoc_resp = ieee80211_is_assoc_resp(mgmt->frame_control) ||
		    ieee80211_is_reassoc_resp(mgmt->frame_control);
	u8 s1g_cap3 = 0;

	if (ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr)
		s1g_cap3 = ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr[3];

	if (is_assoc_resp) {
		mors_vif->bss_color =
		    S1G_CAP8_GET_COLOR(ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr[8]);
		mors_vif->bss_ampdu_mmss = S1G_CAP3_GET_MIN_AMPDU_START_SPC(s1g_cap3);
	}

	/* Only applicable to association request/response and if this is an
	 * Mesh Peering Management (MPM) action frame
	 */
	if (!(is_assoc_req || is_assoc_resp ||
	      (ieee80211_is_action(mgmt->frame_control) && morse_dot11_is_mpm_frame(mgmt) &&
	       morse_dot11_is_mpm_confirm_frame(mgmt))))
		return 0;

	if (ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr[7] & S1G_CAP7_1MHZ_CTL_RESPONSE_PREAMBLE)
		mors_vif->ctrl_resp_in_1mhz_en = true;

	/* Must be held while finding and dereferencing sta */
	rcu_read_lock();
	sta = ieee80211_find_sta(vif, mgmt->sa);
	if (!sta) {
		rcu_read_unlock();
		return -EEXIST;
	}

	mors_sta = (struct morse_sta *)sta->drv_priv;
	if (!mors_sta) {
		rcu_read_unlock();
		return -EEXIST;
	}

	/* Update non-TIM mode status based on the negotiated capabilities.
	 * In STA mode, check association request frame to see if AP has allowed non-TIM mode.
	 * In AP mode, check association response to see if STA is requesting for non-TIM mode
	 * and save the request status for later processing while sending association response.
	 */
	if (is_assoc_resp || is_assoc_req) {
		bool non_tim_mode_enabled = ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr[4] &
					    S1G_CAP4_NON_TIM;

		if (is_assoc_resp)
			mors_sta->non_tim_mode_status = non_tim_mode_enabled ?
						   NON_TIM_MODE_ENABLED : NON_TIM_MODE_DISABLED;
		else
			mors_sta->non_tim_mode_status = non_tim_mode_enabled ?
						   NON_TIM_MODE_REQUESTED : NON_TIM_MODE_DISABLED;
	}

	/* Common code to all accepted frame types goes here */
	if (ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr[5] & S1G_CAP5_AMPDU)
		mors_sta->ampdu_supported = true;

	/* Check partial PV1 support bit set in vendor IE. This is temporary. Replace PV1
	 * frame support check with S1G capabilities once PV1 is fully supported and advertised
	 * in S1G capabilities.
	 */
	if (vif->type == NL80211_IFTYPE_AP)
		mors_sta->pv1_frame_support =
			(mors_sta->vendor_info.pv1_data_frame_only_support && is_assoc_req);
	else if (vif->type == NL80211_IFTYPE_STATION)
		mors_sta->pv1_frame_support =
			(mors_vif->bss_vendor_info.pv1_data_frame_only_support && is_assoc_resp);
	else
		mors_sta->pv1_frame_support = false;

	mors_sta->trav_pilot_support =
	    S1G_CAP2_GET_TRAV_PILOT(ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr[2]);
	if (mors_sta->trav_pilot_support == TRAV_PILOT_RESERVED1)
		MORSE_WARN(mors, "trav_pilot_support == TRAV_PILOT_RESERVED1\n");

	/* Per type configurations goes here */
	if (!is_assoc_resp) {
		sta_max_bw =
		    ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr[0] & S1G_CAP0_SUPP_CH_WIDTH;
		mors_sta->max_bw_mhz = (sta_max_bw == S1G_CAP0_SUPP_16MHZ) ? 16 :
		    (sta_max_bw == S1G_CAP0_SUPP_8MHZ) ? 8 :
		    (sta_max_bw == S1G_CAP0_SUPP_4MHZ) ? 4 : 2;

		mors_sta->ampdu_mmss = S1G_CAP3_GET_MIN_AMPDU_START_SPC(s1g_cap3);
	}

	if (is_assoc_req)
		mors_sta->assoc_req_count++;

	/* Store 1st byte of S1G Caps to retrieve SGI and support channel width info later */
	mors_sta->s1g_cap0 = ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr[0];

	rcu_read_unlock();

	return 0;
}

/**
 * morse_mac_update_custom_listen_interval() - optional override for listen
 *                                             interval.
 * @mors_vif: The morse VIF struct.
 * @mgmt: the management frame to update
 *
 * Checks if listen_interval_ovr is set. If true, update the management frame.
 * The listen_interval is in the correct S1G format and specified in section
 * 9.4.1.6 Listen Interval field of IEEE802.11-2020.
 */
static void morse_mac_update_custom_listen_interval(struct morse_vif *mors_vif,
						    struct ieee80211_mgmt *mgmt)
{
	struct ieee80211_mgmt *s1g_assoc_req = mgmt;
	u16 s1g_li;

	if (!ieee80211_is_assoc_req(s1g_assoc_req->frame_control) &&
	    !ieee80211_is_reassoc_req(s1g_assoc_req->frame_control))
		return;

	if (mors_vif->custom_configs->listen_interval_ovr) {
		s1g_li = mors_vif->custom_configs->listen_interval;

		if (ieee80211_is_assoc_req(s1g_assoc_req->frame_control))
			s1g_assoc_req->u.assoc_req.listen_interval = cpu_to_le16(s1g_li);
		else
			s1g_assoc_req->u.reassoc_req.listen_interval = cpu_to_le16(s1g_li);
	}
}

/**
 * morse_mac_update_custom_max_idle_period() - optional override for mac idle
 *                                             period information element.
 * @mors_vif: The morse VIF struct.
 * @hdr: the frame MAC header
 * @ies_mask: Contains array of information elements.
 *
 * Checks if listen_interval is set. If non-zero, update the
 * WLAN_EID_BSS_MAX_IDLE_PERIOD IE in the ies_mask. The listen_interval is in
 * the correct S1G format and specified in section 9.4.1.6 Listen Interval field
 * of IEEE802.11-2020.
 */
static void morse_mac_update_custom_max_idle_period(struct morse_vif *mors_vif,
						    struct dot11ah_ies_mask *ies_mask,
						    struct ieee80211_hdr *hdr)
{
	struct ieee80211_bss_max_idle_period_ie *bss_max_idle_period;
	u16 s1g_max_idle_period;

	if (!ieee80211_is_assoc_resp(hdr->frame_control) &&
	    !ieee80211_is_reassoc_resp(hdr->frame_control))
		return;

	if (!ies_mask->ies[WLAN_EID_BSS_MAX_IDLE_PERIOD].ptr)
		return;

	if (mors_vif->custom_configs->listen_interval) {
		s1g_max_idle_period = mors_vif->custom_configs->listen_interval;
		bss_max_idle_period = (struct ieee80211_bss_max_idle_period_ie *)
		    ies_mask->ies[WLAN_EID_BSS_MAX_IDLE_PERIOD].ptr;

		/* Convert to S1G (USF/UI) format */
		bss_max_idle_period->max_idle_period = cpu_to_le16(s1g_max_idle_period);

		ies_mask->ies[WLAN_EID_BSS_MAX_IDLE_PERIOD].ptr = (u8 *)bss_max_idle_period;
		ies_mask->ies[WLAN_EID_BSS_MAX_IDLE_PERIOD].len = sizeof(*bss_max_idle_period);
	}
}

u8 *morse_mac_get_ie_pos(struct sk_buff *skb, int *ies_len, int *header_length, bool is_s1g_pkt)
{
	struct ieee80211_mgmt *mgmt = (struct ieee80211_mgmt *)skb->data;
	struct ieee80211_ext *s1g_beacon = (struct ieee80211_ext *)skb->data;
	bool is_protected = ieee80211_has_protected(mgmt->frame_control);
	struct morse_dot11ah_s1g_twt_action *twt_action =
		(struct morse_dot11ah_s1g_twt_action *)(skb->data +
			(is_protected ? IEEE80211_CCMP_HDR_LEN : 0));
	int additional_len = 0;
	u8 *ies_pos = NULL;
	*ies_len = 0;

	if (ieee80211_is_s1g_beacon(mgmt->frame_control)) {
		ies_pos = s1g_beacon->u.s1g_beacon.variable;

		if (le16_to_cpu(s1g_beacon->frame_control) & IEEE80211_FC_NEXT_TBTT)
			ies_pos += 3;

		if (le16_to_cpu(s1g_beacon->frame_control) & IEEE80211_FC_COMPRESS_SSID)
			ies_pos += 4;

		if (le16_to_cpu(s1g_beacon->frame_control) & IEEE80211_FC_ANO)
			ies_pos += 1;

	} else if (ieee80211_is_beacon(mgmt->frame_control)) {
		ies_pos = mgmt->u.beacon.variable;
	} else if (ieee80211_is_assoc_req(mgmt->frame_control) ||
		   ieee80211_is_reassoc_req(mgmt->frame_control)) {
		ies_pos = ieee80211_is_assoc_req(mgmt->frame_control) ?
		    mgmt->u.assoc_req.variable : mgmt->u.reassoc_req.variable;
	} else if (ieee80211_is_assoc_resp(mgmt->frame_control) ||
		   ieee80211_is_reassoc_resp(mgmt->frame_control)) {
		/* Support only S1G assoc response frame offset!!! */
		if (is_s1g_pkt) {
			struct morse_dot11ah_s1g_assoc_resp *s1g_assoc_resp =
			    (struct morse_dot11ah_s1g_assoc_resp *)skb->data;

			ies_pos = s1g_assoc_resp->variable;
		} else {
			ies_pos = mgmt->u.assoc_resp.variable;
		}
	} else if (ieee80211_is_probe_req(mgmt->frame_control)) {
		ies_pos = mgmt->u.probe_req.variable;
	} else if (ieee80211_is_probe_resp(mgmt->frame_control)) {
		ies_pos = mgmt->u.probe_resp.variable;
	} else if (ieee80211_is_action(mgmt->frame_control)) {
		if (morse_dot11_is_mpm_frame(mgmt)) {
			ies_pos = morse_dot11_mpm_frame_ies(mgmt);
			additional_len = morse_dot11_get_mpm_ampe_len(skb);
		} else if (morse_dot11_is_twt_setup_action_frame(twt_action)) {
			ies_pos = morse_dot11_twt_action_ie_pos(twt_action);
		}
	} else {
		return NULL;
	}

	if (ies_pos) {
		*header_length = (ies_pos - skb->data);
		*ies_len = skb->len - *header_length - additional_len;
	}
	return ies_pos;
}

u64 morse_mac_generate_timestamp_for_frame(struct morse_vif *mors_vif)
{
	return jiffies_to_usecs(get_jiffies_64() - mors_vif->epoch);
}

/**
 * morse_mac_update_non_tim_mode() - Update non-TIM mode in S1G Caps based on the STA's request.
 *
 * @vif: Pointer to Virtual interface.
 * @mgmt: Pointer to header of management frame.
 * @s1g_capab: S1G Capabilities IE.
 */
static void morse_mac_update_non_tim_mode(struct ieee80211_vif *vif,
					   struct ieee80211_mgmt *s1g_mgmt,
					   struct ieee80211_s1g_cap *s1g_capab)
{
	struct ieee80211_sta *sta;
	struct morse_sta *mors_sta = NULL;

	rcu_read_lock();
	sta = ieee80211_find_sta(vif, s1g_mgmt->da);
	if (sta)
		mors_sta = (struct morse_sta *)sta->drv_priv;

	if (!mors_sta) {
		rcu_read_unlock();
		return;
	}

	if (mors_sta->non_tim_mode_status == NON_TIM_MODE_REQUESTED) {
		s1g_capab->capab_info[4] |= S1G_CAP4_NON_TIM;
		mors_sta->non_tim_mode_status = NON_TIM_MODE_ENABLED;
	} else {
		s1g_capab->capab_info[4] &= ~S1G_CAP4_NON_TIM;
		mors_sta->non_tim_mode_status = NON_TIM_MODE_DISABLED;
	}
	rcu_read_unlock();
}

static int morse_mac_mgmt_pkt_to_s1g(struct morse *mors, struct sk_buff **skb_orig)
{
	int s1g_ies_length;
	struct dot11ah_ies_mask *ies_mask = NULL;
	struct sk_buff *skb = *skb_orig;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	struct ieee80211_vif *vif = info->control.vif;
	struct morse_vif *mors_vif;
	struct morse_twt_event *twt_tx = NULL;
	enum morse_vendor_ie_mgmt_type_flags mgmt_type = MORSE_VENDOR_IE_TYPE_NONE;
	int twt_ie_size = 0;
	u8 *s1g_mgmt_ies = NULL;
	int s1g_hdr_length = 0;
	u8 *s1g_ordered_ies_buff = NULL;
	struct ieee80211_mgmt *s1g_mgmt;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	int ret = 0;
	bool is_assoc_resp = ieee80211_is_assoc_resp(hdr->frame_control) ||
		ieee80211_is_reassoc_resp(hdr->frame_control);

	mors_vif = ieee80211_vif_to_morse_vif(vif);

	ies_mask = morse_dot11ah_ies_mask_alloc();
	if (!ies_mask) {
		ret = -ENOMEM;
		goto exit;
	}

	if (ieee80211_is_probe_req(hdr->frame_control))
		mgmt_type = MORSE_VENDOR_IE_TYPE_PROBE_REQ;
	else if (ieee80211_is_probe_resp(hdr->frame_control))
		mgmt_type = MORSE_VENDOR_IE_TYPE_PROBE_RESP;
	else if (ieee80211_is_assoc_req(hdr->frame_control) ||
			ieee80211_is_reassoc_req(hdr->frame_control))
		mgmt_type = MORSE_VENDOR_IE_TYPE_ASSOC_REQ;
	else if (is_assoc_resp)
		mgmt_type = MORSE_VENDOR_IE_TYPE_ASSOC_RESP;

	s1g_mgmt_ies = morse_mac_get_ie_pos(skb, &s1g_ies_length, &s1g_hdr_length, false);

	/* Parse out the original IEs so we can mess with them */
	if (s1g_mgmt_ies)
		if (morse_dot11ah_parse_ies(s1g_mgmt_ies, s1g_ies_length, ies_mask) < 0) {
			ret = -EINVAL;
			MORSE_WARN_RATELIMITED(mors,
				"Failed parsing FC:0x%04x information element\n",
				le16_to_cpu(hdr->frame_control));
			goto exit;
		}

	if (ieee80211_vif_is_mesh(vif))
		morse_mac_process_mesh_tx_mgmt(mors_vif, skb, ies_mask);

	if (morse_twt_is_enabled(mors_vif) &&
	    mgmt_type == MORSE_VENDOR_IE_TYPE_ASSOC_RESP) {
		spin_lock_bh(&mors_vif->twt.lock);
		twt_tx = morse_twt_peek_tx(mors, mors_vif, hdr->addr1, NULL);
		if (twt_tx) {
			twt_ie_size = morse_twt_get_ie_size(mors, twt_tx);
			MORSE_DBG(mors, "TWT IE size: %d\n", twt_ie_size);
			morse_twt_dump_event(mors, twt_tx);
		} else {
			MORSE_DBG(mors, "No TWT IEs for TX available\n");
		}
		spin_unlock_bh(&mors_vif->twt.lock);
	}

	/* Send setup command TWT IE if available and an association request. */
	if (morse_twt_is_enabled(mors_vif) &&
	    mgmt_type == MORSE_VENDOR_IE_TYPE_ASSOC_REQ &&
	    mors_vif->twt.req_event_tx) {
		twt_tx = (struct morse_twt_event *)mors_vif->twt.req_event_tx;
		twt_ie_size = morse_twt_get_ie_size(mors, twt_tx);
		MORSE_DBG(mors, "TWT IE size: %d\n", twt_ie_size);
		morse_twt_dump_event(mors, twt_tx);
	}

	if (twt_ie_size > 0) {
		morse_twt_insert_ie(mors, twt_tx, ies_mask, twt_ie_size);
		if (mgmt_type == MORSE_VENDOR_IE_TYPE_ASSOC_RESP &&
		    morse_twt_dequeue_tx(mors, mors_vif, twt_tx))
			MORSE_WARN_RATELIMITED(mors, "%s: Unable to dequeue TWT tx\n", __func__);
	}

	morse_cac_insert_ie(ies_mask, vif, hdr->frame_control);
	morse_vendor_insert_caps_ops_ie(mors, vif, skb, ies_mask);

	spin_lock_bh(&mors_vif->vendor_ie.lock);
	morse_vendor_ie_add_ies(mors_vif, ies_mask, mgmt_type);

	morse_dot11ah_11n_to_s1g_tx_packet(vif, skb, s1g_hdr_length, false, ies_mask);

	s1g_mgmt = (struct ieee80211_mgmt *)skb->data;

	/* We have skb with translated header (if required) + ies_mask with all translated IEs */
	morse_mac_update_custom_listen_interval(mors_vif, s1g_mgmt);
	morse_mac_update_custom_max_idle_period(mors_vif, ies_mask, hdr);
	morse_mac_update_custom_s1g_capab(mors_vif, ies_mask, vif->type);

	/* Update non-TIM mode based on the STA's request */
	if (is_assoc_resp && mors_vif->enable_non_tim_mode) {
		struct ieee80211_s1g_cap *s1g_capab =
			   (struct ieee80211_s1g_cap *)ies_mask->ies[WLAN_EID_S1G_CAPABILITIES].ptr;

		spin_unlock_bh(&mors_vif->vendor_ie.lock);
		if (!s1g_capab) {
			ret = -EINVAL;
			goto exit;
		}
		morse_mac_update_non_tim_mode(vif, s1g_mgmt, s1g_capab);

		spin_lock_bh(&mors_vif->vendor_ie.lock);
	}

	if (ieee80211_is_probe_resp(s1g_mgmt->frame_control))
		s1g_mgmt->u.probe_resp.timestamp =
		    cpu_to_le64(morse_mac_generate_timestamp_for_frame(mors_vif));

	/* At this stage, the skb will contain the S1G header of the frame while
	 * the information elements will be referenced by ies_mask. Next steps
	 * will include re-ordering the information elements into a temporary buffer.
	 * Then expanding the skb if the required ies_mask + the new header are
	 * greater than the original skb room. In any case, the temporary buffer
	 * will be copied into the tail of skb (original/expanded)
	 */
	s1g_mgmt_ies = morse_mac_get_ie_pos(skb, &s1g_ies_length, &s1g_hdr_length, true);
	if (s1g_mgmt_ies) {
		s1g_ies_length = morse_dot11_insert_ordered_ies_from_ies_mask(skb,
			NULL, ies_mask, hdr->frame_control);

		s1g_ordered_ies_buff = kmalloc(s1g_ies_length, GFP_ATOMIC);
		morse_dot11_insert_ordered_ies_from_ies_mask(skb,
							s1g_ordered_ies_buff,
							ies_mask,
							hdr->frame_control);

		if ((skb->len + skb_tailroom(skb)) < (s1g_hdr_length + s1g_ies_length)) {
			struct sk_buff *skb2;
			/* Allocate new SKB according to total size of ies_mask plus header */
			skb2 = skb_copy_expand(skb,
				skb_headroom(skb),
				(s1g_hdr_length + s1g_ies_length) - skb->len,
				GFP_ATOMIC);
			if (!skb2) {
				ret = -ENOMEM;
				spin_unlock_bh(&mors_vif->vendor_ie.lock);
				kfree(s1g_ordered_ies_buff);
				goto exit;
			}
			/* Cannot clean original skb just yet, ies_mask is still pending */
			skb = skb2;
			hdr = (struct ieee80211_hdr *)skb->data;
		}

		skb_trim(skb, s1g_hdr_length);
		s1g_mgmt_ies = skb_put(skb, s1g_ies_length);
		memcpy(s1g_mgmt_ies, s1g_ordered_ies_buff, s1g_ies_length);
		kfree(s1g_ordered_ies_buff);

		if (*skb_orig != skb) {
			morse_mac_skb_free(mors, *skb_orig);
			*skb_orig = skb;
		}
	}
	spin_unlock_bh(&mors_vif->vendor_ie.lock);
exit:
	morse_dot11ah_ies_mask_free(ies_mask);
	return ret;
}

static int last_mgmt_rx_bw_mhz_from_sta(const struct ieee80211_sta *sta)
{
	morse_rate_code_t rc;
	const struct morse_sta *msta;

	msta = (struct morse_sta *)sta->drv_priv;
	if (msta->last_rx.is_mgmt_set)
		rc = msta->last_rx.mgmt_status.morse_ratecode;
	else
		return -1;

	return morse_ratecode_bw_index_to_s1g_bw_mhz(morse_ratecode_bw_index_get(rc));
}

/**
 * Given a mgmt frame to send from an AP vif, find the appropriate transmit bandwidth (MHz).

 * @mors: morse chip struct
 * @vif: iee80211 interface
 * @sta: sta information for directed mgmt frame (may be NULL)
 * @da: destination address of mgmt frame
 *
 * Return: bandwidth (MHz)
 */
static int find_tx_bw_for_mgmt_frame_ap(struct morse *mors, struct ieee80211_vif *vif,
					const struct ieee80211_sta *sta, const u8 *da)
{
	int tx_bw_mhz;

	if (is_broadcast_ether_addr(da) || is_multicast_ether_addr(da))
		return mors->custom_configs.channel_info.pri_bw_mhz;

	/* if destination sta is known, use last rx from it */
	if (sta)
		return last_mgmt_rx_bw_mhz_from_sta(sta);

	/* This destination may have been recorded in the pre-assoc list */
	tx_bw_mhz = morse_pre_assoc_peer_get_last_rx_bw_mhz(mors, da);

	/* Associated WDS / 4addr stations may not always have
	 * sta object assigned to the tx SKB control block.
	 */
	if (tx_bw_mhz <= 0) {
		rcu_read_lock();
		sta = ieee80211_find_sta(vif, da);
		if (sta)
			tx_bw_mhz = last_mgmt_rx_bw_mhz_from_sta(sta);
		rcu_read_unlock();
	}

	return tx_bw_mhz;
}

/**
 * Given a mgmt frame to send, find the appropriate transmit bandwidth (MHz).
 *
 * Return: The bandwidth (in MHz)
 */
static int find_tx_bw_for_mgmt_frame(struct morse *mors, struct ieee80211_vif *vif,
				     const struct ieee80211_sta *sta,
				     const struct ieee80211_mgmt *mgmt)
{
	int tx_bw_mhz = -1;
	__le16 fc = mgmt->frame_control;
	u8 *da = ieee80211_get_DA((struct ieee80211_hdr *)mgmt);
	struct morse_channel_info ch_info;

	if (morse_mac_is_iface_ap_type(vif)) {
		tx_bw_mhz = find_tx_bw_for_mgmt_frame_ap(mors, vif, sta, da);
	} else if (ieee80211_is_probe_req(fc) && morse_mac_is_1mhz_probe_req_enabled()) {
		tx_bw_mhz = 1;
	} else if (morse_mac_find_channel_info_for_bssid(mgmt->bssid, &ch_info)) {
		/* BSSID entry found (i.e. AP channel info), use that */
		tx_bw_mhz = ch_info.pri_bw_mhz;
	}

	/* If BW information could not be found for the current frame, fall back to using the
	 * currently configured primary bandwidth.
	 */
	if (tx_bw_mhz <= 0)
		tx_bw_mhz = mors->custom_configs.channel_info.pri_bw_mhz;

	/* Ensure selected bw does not exceed the currently configured primary bandwidth.*/
	tx_bw_mhz = min_t(int, tx_bw_mhz, mors->custom_configs.channel_info.pri_bw_mhz);
	MORSE_DBG(mors, "%s: considering %d MHz for mgmt frame to %pM", __func__, tx_bw_mhz, da);
	return tx_bw_mhz;
}

int morse_mac_pkt_to_s1g(struct morse *mors, const struct ieee80211_sta *sta,
			 struct sk_buff **skb_orig, int *tx_bw_mhz)
{
	int ret = 0;
	bool use_op_bw = false;
	__le16 fc = ((struct ieee80211_hdr *)(*skb_orig)->data)->frame_control;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(*skb_orig);

	if (morse_dot11ah_is_pv1_qos_data(le16_to_cpu(fc))) {
		/* Don't expect non-QoS PV1 frames */
		use_op_bw = true;
	} else if (ieee80211_is_qos_nullfunc(fc) || ieee80211_is_nullfunc(fc)) {
		/* NULL funcs generated by the UMAC (AP or STA), should be sent at the primary bw */
		*tx_bw_mhz = mors->custom_configs.channel_info.pri_bw_mhz;
	} else if (ieee80211_is_s1g_beacon(fc)) {
		ret = morse_mac_mgmt_pkt_to_s1g(mors, skb_orig);
		*tx_bw_mhz = mors->custom_configs.channel_info.pri_bw_mhz;
	} else if (ieee80211_is_mgmt(fc)) {
		ret = morse_mac_mgmt_pkt_to_s1g(mors, skb_orig);
		*tx_bw_mhz = find_tx_bw_for_mgmt_frame(mors, info->control.vif, sta,
						(const struct ieee80211_mgmt *)(*skb_orig)->data);
	} else if (unlikely((*skb_orig)->protocol == cpu_to_be16(ETH_P_PAE) ||
		   info->flags & IEEE80211_TX_CTL_USE_MINRATE)) {
		/* Send initial EAPOL handshake frames at the primary bandwidth */
		*tx_bw_mhz = mors->custom_configs.channel_info.pri_bw_mhz;
	} else {
		/* For almost all frames - default to sending at operating
		 * bandwidth. Rate control algorithms may later want to tweak this.
		 */
		use_op_bw = true;
	}

	if (use_op_bw || !morse_mac_is_subband_enable())
		*tx_bw_mhz = mors->custom_configs.channel_info.op_bw_mhz;

	return ret;
}

static void
morse_aggr_check(struct morse_vif *mors_vif, struct ieee80211_sta *pubsta, struct sk_buff *skb)
{
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	struct morse_sta *mors_sta = (struct morse_sta *)pubsta->drv_priv;
	u8 tid = MORSE_IEEE80211_GET_TID(hdr);

	/* we are already aggregating */
	if (mors_sta->tid_tx[tid] || mors_sta->tid_start_tx[tid])
		return;

	if (!mors_vif->custom_configs)
		return;

	if (!mors_vif->custom_configs->enable_ampdu)
		return;

	if (!mors_sta->ampdu_supported)
		return;

	if (mors_sta->state < IEEE80211_STA_AUTHORIZED)
		return;

	if (skb_get_queue_mapping(skb) == IEEE80211_AC_VO)
		return;

	if (unlikely(!ieee80211_is_data_qos(hdr->frame_control)))
		return;

	if (unlikely(skb->protocol == cpu_to_be16(ETH_P_PAE)))
		return;

	mors_sta->tid_start_tx[tid] = true;

	ieee80211_start_tx_ba_session(pubsta, tid, 0);
}

void morse_mac_schedule_probe_req(struct ieee80211_vif *vif)
{
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;
	struct morse *mors = morse_vif_to_morse(mors_vif);

	tasklet_schedule(&mors_vif->send_probe_req);
	MORSE_DBG(mors, "QoS NULL frame Tx completed! Scheduled to a send probe req\n");
	mors_vif->waiting_for_probe_req_sched = false;
}

static void morse_mac_ops_tx(struct ieee80211_hw *hw,
			     struct ieee80211_tx_control *control, struct sk_buff *skb)
{
	struct morse *mors = hw->priv;
	struct morse_skbq *mq;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	struct morse_vif *mors_vif = NULL;
	struct ieee80211_vif *vif = NULL;
	struct morse_skb_tx_info tx_info = { 0 };
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	bool is_mgmt = (hdr && ieee80211_is_mgmt(hdr->frame_control));
	const int op_bw_mhz = mors->custom_configs.channel_info.op_bw_mhz;
	int tx_bw_mhz = op_bw_mhz;
	struct ieee80211_sta *sta = NULL;
	struct morse_sta *mors_sta = NULL;
	int vif_max_bw_mhz;
	int sta_max_bw_mhz = 0;

	if (info && info->control.vif)
		vif = info->control.vif;

	if (!vif) {
		MORSE_ERR_RATELIMITED(mors, "%s: vif is null", __func__);
		return;
	}

	mors_vif = ieee80211_vif_to_morse_vif(vif);
	if (control)
		sta = control->sta;

	/* If we have a station, retrieve station-specific tx info */
	if (sta) {
#ifdef CONFIG_MORSE_IPMON
		static u64 time_start;

		morse_ipmon(&time_start, skb, skb->data, skb->len, IPMON_LOC_CLIENT_DRV1, 0);
#endif
		/* see if we should start aggregation */
		morse_aggr_check(mors_vif, sta, skb);
		/* Get the s1g bw limit */
		mors_sta = (struct morse_sta *)sta->drv_priv;
		sta_max_bw_mhz = mors_sta->max_bw_mhz;

		if (mors_vif->enable_pv1 && mors_sta->pv1_frame_support && !is_mgmt) {
			if (!morse_mac_convert_pv0_to_pv1(mors, mors_vif, sta, skb))
				hdr = (struct ieee80211_hdr *)skb->data;
		}
		mors_sta->tx_pkt_count++;
	}

	if (morse_mac_pkt_to_s1g(mors, sta, &skb, &tx_bw_mhz) < 0) {
		MORSE_DBG(mors, "Failed to convert packet to S1G. Dropping..\n");
		morse_mac_skb_free(mors, skb);
		return;
	}

	/* In AP mode, broadcast and multicast frames are sent using the primary bandwidth, to
	 * ensure reception by third-party stations that do not support the operating bandwidth and
	 * by stations operating at low bandwidth because of poor reception.
	 * Mesh and IBSS modes currently do not need to interoperate and prefer full bandwidth.
	 */
	if (hdr && vif->type == NL80211_IFTYPE_AP && morse_mac_is_subband_enable()) {
		u8 *da = ieee80211_get_DA(hdr);

		if (is_broadcast_ether_addr(da) ||
		    (is_multicast_ether_addr(da) && !mors_vif->enable_multicast_rate_control))
			tx_bw_mhz = mors->custom_configs.channel_info.pri_bw_mhz;
	}

	/* limit check the set tx_bw for the vif */
	vif_max_bw_mhz = morse_vif_max_tx_bw(mors_vif);
	tx_bw_mhz = min(vif_max_bw_mhz, tx_bw_mhz);

	/* This will be true if we are an AP and have parsed the STA's S1G
	 * capabilities when it associated - STAs use the s1g operation
	 * from the AP to determine max bw
	 */
	if (sta_max_bw_mhz > 0)
		tx_bw_mhz = min(tx_bw_mhz, sta_max_bw_mhz);

	morse_mac_fill_tx_info(mors, &tx_info, skb, vif, tx_bw_mhz, sta);

	/* Function will automatically call tx_status on
	 * skb if frame should be rescheduled by mac80211 for power save filtering.
	 */
	if (morse_mac_tx_ps_filtered_for_sta(mors, skb, sta))
		return;

	if (is_mgmt)
		mq = mors->cfg->ops->skbq_mgmt_tc_q(mors);
	else
		mq = mors->cfg->ops->skbq_tc_q_from_aci(mors, dot11_tid_to_ac(tx_info.tid));

	morse_skbq_skb_tx(mq, &skb, &tx_info,
			  (is_mgmt) ? MORSE_SKB_CHAN_MGMT : MORSE_SKB_CHAN_DATA);
}

#if KERNEL_VERSION(5, 9, 0) <= MAC80211_VERSION_CODE
/* The following functions are for airtime fairness */
static int morse_txq_send(struct morse *mors, struct ieee80211_txq *txq)
{
	struct ieee80211_tx_control control = { };

	control.sta = txq->sta;

	while (!test_bit(MORSE_STATE_FLAG_DATA_QS_STOPPED, &mors->state_flags)) {
		struct sk_buff *skb = ieee80211_tx_dequeue(mors->hw, txq);

		if (!skb)
			break;

		morse_mac_ops_tx(mors->hw, &control, skb);
	}

	return test_bit(MORSE_STATE_FLAG_DATA_QS_STOPPED, &mors->state_flags);
}

static bool morse_txq_schedule_list(struct morse *mors, enum morse_page_aci aci)
{
	struct ieee80211_txq *txq;
	bool tx_stopped = false;

	do {
		txq = ieee80211_next_txq(mors->hw, aci);
		if (!txq)
			break;

		tx_stopped = morse_txq_send(mors, txq);

		ieee80211_return_txq(mors->hw, txq, false);
	} while (!tx_stopped);

	return tx_stopped;
}

static bool morse_txq_schedule(struct morse *mors, enum morse_page_aci aci)
{
	bool tx_stopped = false;

	if (aci > MORSE_ACI_VO)
		return 0;

	rcu_read_lock();

	ieee80211_txq_schedule_start(mors->hw, aci);
	tx_stopped = morse_txq_schedule_list(mors, aci);
	ieee80211_txq_schedule_end(mors->hw, aci);

	rcu_read_unlock();

	return tx_stopped;
}

static void morse_txq_tasklet(struct tasklet_struct *t)
{
	s16 aci;
	bool tx_stopped;
	struct morse *mors = from_tasklet(mors, t, tasklet_txq);

	if (test_bit(MORSE_STATE_FLAG_DATA_QS_STOPPED, &mors->state_flags))
		return;

	for (aci = MORSE_ACI_VO; aci >= 0; aci--) {
		tx_stopped = morse_txq_schedule(mors, (enum morse_page_aci)aci);

		if (tx_stopped)
			/* Queues are stopped, probably filled */
			break;

		if (aci == MORSE_ACI_BE)
			break;
	}
}

static void morse_mac_ops_wake_tx_queue(struct ieee80211_hw *hw, struct ieee80211_txq *txq)
{
	struct morse *mors = hw->priv;

	tasklet_schedule(&mors->tasklet_txq);
}
#endif

static void morse_survey_destroy_usage_records(struct morse *mors)
{
	if (!mors->channel_survey)
		return;

	kfree(mors->channel_survey->records);
	kfree(mors->channel_survey);
	mors->channel_survey = NULL;
}

int morse_survey_init_usage_records(struct morse *mors)
{
	struct morse_channel_survey *survey;

	if (mors->channel_survey)
		morse_survey_destroy_usage_records(mors);

	survey = kmalloc(sizeof(*survey), GFP_KERNEL);
	if (!survey)
		return -ENOMEM;

	survey->num_records = morse_dot11ah_get_num_channels();
	survey->first_channel_in_scan = true;
	survey->records = kcalloc(survey->num_records, sizeof(*survey->records), GFP_KERNEL);
	if (!survey->records) {
		kfree(survey);
		return -ENOMEM;
	}

	mors->channel_survey = survey;
	return 0;
}

int morse_survey_add_channel_usage(struct morse *mors, struct morse_survey_rx_usage_record *record)
{
	struct morse_channel_survey *survey = mors->channel_survey;
	int ret = -1;
	int i;

	if (!survey)
		return -EEXIST;

	for (i = 0; i < survey->num_records; i++) {
		/* No record for this channel, so initialise it */
		if (survey->records[i].freq_hz == 0) {
			memcpy(&survey->records[i], record, sizeof(*record));
			ret = 0;
			break;
		} else if (record->freq_hz == survey->records[i].freq_hz &&
				record->bw_mhz == survey->records[i].bw_mhz) {
			/* Matched record, so update */
			survey->records[i].time_listen += record->time_listen;
			survey->records[i].time_rx += record->time_rx;
			/* Only take most recent noise figure */
			survey->records[i].noise = record->noise;
			ret = 0;
			break;
		}
	}

	MORSE_DBG(mors, "%s: [%d] freq: %u bw: %d tot: %llu rx: %llu noise: %d\n", __func__,
		ret ? -1 : i, record->freq_hz, record->bw_mhz, record->time_listen,
		record->time_rx, record->noise);
	return ret;
}

static int morse_survey_update_channel_usage(struct morse *mors)
{
	struct morse_channel_survey *survey = mors->channel_survey;
	struct morse_survey_rx_usage_record usage_record;
	int ret;

	if (!enable_survey || !survey)
		return -EEXIST;

	if (survey->first_channel_in_scan) {
		survey->first_channel_in_scan = false;
		return 0;
	}

	ret = morse_cmd_get_channel_usage(mors, &usage_record);
	if (ret)
		return ret;

	return morse_survey_add_channel_usage(mors, &usage_record);
}

static struct morse_survey_rx_usage_record *morse_survey_get_record(struct morse *mors, u32 freq_hz,
								    u8 bw_mhz)
{
	struct morse_channel_survey *survey = mors->channel_survey;
	int i;

	if (!survey || freq_hz == 0 || bw_mhz == 0)
		return NULL;

	for (i = 0; i < survey->num_records; i++) {
		if (freq_hz == survey->records[i].freq_hz && bw_mhz == survey->records[i].bw_mhz)
			return &survey->records[i];
		else if (survey->records[i].freq_hz == 0)
			break;
	}
	return NULL;
}

int morse_mac_traffic_control(struct morse *mors, int interface_id,
			      bool pause_data_traffic, int sources)
{
	int ret = -1;
	unsigned long *event_flags = &mors->chip_if->event_flags;
	struct morse_vif *mors_vif;
	bool sources_includes_twt = (sources & MORSE_CMD_UMAC_TRAFFIC_CONTROL_SOURCE_TWT);
	struct ieee80211_vif *vif = morse_get_vif_from_vif_id(mors, interface_id);

	if (!vif) {
		MORSE_WARN_ON(FEATURE_ID_DEFAULT, 1);
		goto exit;
	}

	mors_vif = ieee80211_vif_to_morse_vif(vif);

	if (!mors_vif->twt.requester && sources_includes_twt) {
		/* TWT not supported.. LMAC should not be signalling traffic control */
		WARN_ONCE(1, "TWT not supported on interface\n");
		goto exit;
	}

	if (pause_data_traffic) {
		set_bit(MORSE_DATA_TRAFFIC_PAUSE_PEND, event_flags);
		queue_work(mors->chip_wq, &mors->chip_if_work);
		if (sources_includes_twt)
			morse_watchdog_pause(mors);
	} else {
		set_bit(MORSE_DATA_TRAFFIC_RESUME_PEND, event_flags);
		queue_work(mors->chip_wq, &mors->chip_if_work);
		if (sources_includes_twt)
			morse_watchdog_resume(mors);
	}

	ret = 0;
exit:
	return ret;
}

int morse_cqm_rssi_notify_event(struct morse *mors, struct ieee80211_vif *vif,
		struct morse_cmd_evt_cqm_rssi_notify *cqm_notify)
{
	s16 rssi = le16_to_cpu(cqm_notify->rssi);
	u16 event = le16_to_cpu(cqm_notify->event);

	if (!vif)
		return -ENODEV;

	if (!(vif->driver_flags & IEEE80211_VIF_SUPPORTS_CQM_RSSI))
		return -EPERM;

	MORSE_DBG(mors, "%s: event:%d, rssi:%d dBm\n", __func__,
					cqm_notify->event, cqm_notify->rssi);
	if (event == MORSE_CMD_CQM_RSSI_THRESHOLD_EVENT_LOW) {
		morse_mac_cqm_rssi_notify(vif, NL80211_CQM_RSSI_THRESHOLD_EVENT_LOW,
					rssi, GFP_KERNEL);
	} else if (event == MORSE_CMD_CQM_RSSI_THRESHOLD_EVENT_HIGH) {
		morse_mac_cqm_rssi_notify(vif, NL80211_CQM_RSSI_THRESHOLD_EVENT_HIGH,
					rssi, GFP_KERNEL);
	}

	return 0;
}

static int morse_mac_driver_restart(struct morse *mors)
{
	schedule_work(&mors->driver_restart);
	MORSE_INFO(mors, "Scheduled a driver reset ...\n");
	set_bit(MORSE_STATE_FLAG_CHIP_UNRESPONSIVE, &mors->state_flags);

	return 0;
}

/**
 * ignore_s1g_channel() - Mark an S1G channel as 'ignored' in this regulatory domain.
 *
 * As cfg80211 will consider a channel to be unusable if any sub-channel is disabled, the custom
 * flag @ref IEEE80211_CHAN_IGNORE is used to mark this channel as unusable (propagated to
 * userspace).
 */
static void ignore_s1g_channel(struct ieee80211_hw *hw, u32 freq_khz, u32 bw_mhz)
{
	int ret;
	struct ieee80211_channel *ch;
	struct morse *mors = hw->priv;
	int op_freq_5g;
	int op_chan_s1g = morse_dot11ah_freq_khz_bw_mhz_to_chan(freq_khz, bw_mhz);
	int op_chan_5g = morse_dot11ah_s1g_chan_to_5g_chan(op_chan_s1g);

	if (op_chan_5g < 0) {
		MORSE_WARN(mors, "%s: Channel %dkHz %dMHz %d (S1G) not found", __func__,
			   freq_khz, bw_mhz, op_chan_s1g);
		return;
	}

	op_freq_5g = ieee80211_channel_to_frequency(op_chan_5g, NL80211_BAND_5GHZ);
	ch = ieee80211_get_channel(hw->wiphy, op_freq_5g);
	if (!ch) {
		MORSE_WARN(mors, "%s: Channel %dkHz %dMHz %d/%d (S1G/5G) not found in 5GHz band",
			  __func__, freq_khz, bw_mhz, op_chan_s1g, op_chan_5g);
		return;
	}

	ret = morse_dot11ah_ignore_channel(op_chan_s1g);
	if (ret) {
		MORSE_WARN(mors, "%s: Channel %dkHz %dMHz %d/%d (S1G/5G) not found in S1G band",
			   __func__, freq_khz, bw_mhz, op_chan_s1g, op_chan_5g);
		return;
	}

	if (ch->flags & IEEE80211_CHAN_IGNORE)
		return;

	ch->flags |= IEEE80211_CHAN_IGNORE;
	MORSE_INFO(mors, "%s: Channel %dkHz %dMHz %d/%d (S1G/5G) ignored", __func__, freq_khz,
		  bw_mhz, op_chan_s1g, op_chan_5g);
}

/**
 * set_hw_ignored_s1g_channels() - Retrieve channels that HW has disabled and update channel list.
 *
 * Dynamic regdom updates will trigger a restart and reload of the firmware. This function should
 * be called again to approriately update the new channel list.
 *
 * @hw: mac80211 hw object
 */
static void set_hw_ignored_s1g_channels(struct ieee80211_hw *hw)
{
	int ret;
	uint i;
	struct morse *mors = hw->priv;
	struct morse_cmd_resp_get_disabled_channels *resp = NULL;
	uint resp_len =
		sizeof(*resp) +
		sizeof(struct morse_cmd_disabled_channel_entry) * ARRAY_SIZE(mors_5ghz_channels);

	lockdep_assert_held(&mors->lock);

	resp = kmalloc(resp_len, GFP_KERNEL);
	if (!resp) {
		ret = -ENOSPC;
		goto exit;
	}

	ret = morse_cmd_get_disabled_channels(mors, resp, resp_len);
	if (ret)
		goto exit;

	for (i = 0; i < le32_to_cpu(resp->n_channels); i++) {
		struct morse_cmd_disabled_channel_entry *channel = &resp->channels[i];
		u32 freq_khz = le16_to_cpu(channel->freq_100khz) * 100;
		u32 bw_mhz = channel->bw_mhz;

		if (bw_mhz) {
			ignore_s1g_channel(hw, freq_khz, bw_mhz);
		} else {
			/* When bw_mhz is zero all frequency/bandwidth combinations
			 * should be disabled.
			 */
			ignore_s1g_channel(hw, freq_khz, 1);
			ignore_s1g_channel(hw, freq_khz, 2);
			ignore_s1g_channel(hw, freq_khz, 4);
			ignore_s1g_channel(hw, freq_khz, 8);
		}
	}

exit:
	if (ret)
		MORSE_ERR(mors, "%s: failed (ret:%d)", __func__, ret);
	else
		MORSE_DBG(mors, "%s: %d channel(s) disabled", __func__, resp->n_channels);

	kfree(resp);
}

static int morse_mac_ops_start(struct ieee80211_hw *hw)
{
	struct morse *mors = hw->priv;
	bool restart_requested;

	mutex_lock(&mors->lock);
	restart_requested =
	    test_and_clear_bit(MORSE_STATE_FLAG_RELOAD_FW_AFTER_START, &mors->state_flags);
	if (restart_requested) {
		MORSE_INFO(mors, "FW reload was requested during initialisation\n");
		morse_mac_driver_restart(mors);
		mutex_unlock(&mors->lock);
		return 0;
	}

	/* Read and print FW version */
	morse_cmd_get_version(mors);
	mors->started = true;

	/* Retrieve channels from the HW that are unusable */
	set_hw_ignored_s1g_channels(hw);

	if (mors->cfg->set_slow_clock_mode)
		mors->cfg->set_slow_clock_mode(mors, morse_mac_slow_clock_mode());

	mors->state_flags &= MORSE_STATE_FLAG_KEEP_ON_START_MASK;
	mutex_unlock(&mors->lock);

	return 0;
}

#if KERNEL_VERSION(6, 11, 0) > MAC80211_VERSION_CODE
static void morse_mac_ops_stop(struct ieee80211_hw *hw)
#else
static void morse_mac_ops_stop(struct ieee80211_hw *hw, bool suspend)
#endif
{
	struct morse *mors = hw->priv;
	struct morse_vif *mon_if = &mors->mon_if;

	mutex_lock(&mors->lock);
	/* Make sure we stop any monitor interfaces */
	if (mon_if->id != INVALID_VIF_ID) {
		morse_cmd_rm_if(mors, mon_if->id);
		mon_if->id = INVALID_VIF_ID;
		MORSE_INFO(mors, "monitor interfaced removed\n");
	}
	mors->started = false;
	mutex_unlock(&mors->lock);
}

/**
 * SW-7260: Resetting the beacon change sequence related parameters.
 */
static void
morse_mac_reset_s1g_bcn_change_seq_params(struct ieee80211_hw *hw, struct ieee80211_vif *vif)
{
	struct morse *mors = hw->priv;
	struct morse_vif *mors_vif;

	if (!vif) {
		MORSE_ERR(mors, "%s vif is NULL\n", __func__);
		return;
	}
	mors_vif = ieee80211_vif_to_morse_vif(vif);

	mors_vif->s1g_bcn_change_seq = 0;
	mors_vif->s1g_oper_param_crc = 0;
	mors_vif->edca_param_crc = 0;
	mors_vif->chan_switch_in_progress = false;
	mors_vif->waiting_for_probe_req_sched = false;
}

static void morse_mac_reset_sta_backup(struct morse *mors, struct morse_vif *mors_vif)
{
	memset(mors_vif->sta_backups, 0, sizeof(mors_vif->sta_backups));

	MORSE_DBG(mors, "STA backup entries cleared\n");
}

static void
morse_mac_save_sta_backup(struct morse *mors, struct morse_vif *mors_vif,
			  struct morse_sta *mors_sta)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(mors_vif->sta_backups); i++) {
		if (mors_vif->sta_backups[i].assoc_req_count == 0 ||
		    time_after(jiffies, mors_vif->sta_backups[i].timeout)) {
			MORSE_DBG(mors, "Storing STA backup (slot %d) for %pM\n",
				  i, mors_sta->addr);
			memcpy(&mors_vif->sta_backups[i], mors_sta, sizeof(*mors_sta));
			mors_vif->sta_backups[i].timeout =
			    jiffies + msecs_to_jiffies(STA_PRIV_TIMEOUT_MSEC);
			return;
		}
	}
	MORSE_WARN(mors, "No spare STA backup slot\n");
}

static void
morse_mac_restore_sta_backup(struct morse *mors, struct morse_vif *mors_vif,
			     struct morse_sta *mors_sta, u8 *addr)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(mors_vif->sta_backups); i++) {
		if (mors_vif->sta_backups[i].assoc_req_count > 0 &&
		    ether_addr_equal_unaligned(mors_vif->sta_backups[i].addr, addr)) {
			MORSE_INFO(mors, "Retrieving STA backup (slot %d) for %pM\n",
				   i, mors_sta->addr);
			memcpy(mors_sta, &mors_vif->sta_backups[i], sizeof(*mors_sta));
			memset(&mors_vif->sta_backups[i], 0, sizeof(mors_vif->sta_backups[i]));
			return;
		}
	}
	MORSE_DBG(mors, "No STA backup for %pM\n", mors_sta->addr);
}

/* Sets the requested channel. Also sets tx power to the channel's regulatory limit if requested */
static int morse_mac_set_channel(struct morse *mors, u32 op_chan_freq_hz, u8 pri_1mhz_chan_idx,
				 u8 op_bw_mhz, u8 pri_bw_mhz, bool s1g_chan_power,
				 const char *caller_func)
{
	int ret;
	const struct morse_dot11ah_channel *chan_s1g;

	ret = morse_cmd_set_channel(mors, op_chan_freq_hz, pri_1mhz_chan_idx, op_bw_mhz,
				    pri_bw_mhz, &mors->tx_power_mbm);
	if (ret) {
		MORSE_ERR(mors, "%s (%s): morse_cmd_set_channel() failed, ret %d\n",
			  __func__, caller_func, ret);
		return ret;
	}

	if (!s1g_chan_power)
		return 0;

	/* Update Tx power using S1G max values if possible */
	chan_s1g = morse_dot11ah_s1g_freq_to_s1g(op_chan_freq_hz, op_bw_mhz);
	if (chan_s1g)
		morse_mac_set_txpower(mors, chan_s1g->ch.max_reg_power);

	return 0;
}

#if KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE
static void morse_chswitch_timer(unsigned long addr)
#else
static void morse_chswitch_timer(struct timer_list *t)
#endif
{
#if KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE
	struct morse_vif *mors_vif = (struct morse_vif *)addr;
#else
	struct morse_vif *mors_vif = from_timer(mors_vif, t, chswitch_timer);
#endif
	struct ieee80211_vif *vif;
	struct morse *mors;

	if (!mors_vif) {
		MORSE_PR_ERR(FEATURE_ID_ECSA, "%s: mors_vif NULL\n", __func__);
		return;
	}
	vif = morse_vif_to_ieee80211_vif(mors_vif);
	mors = morse_vif_to_morse(mors_vif);

	MORSE_ECSA_INFO(mors, "%s: chswitch timer TS=%ld\n", __func__, jiffies);

	if (vif->type == NL80211_IFTYPE_AP)
		MORSE_IEEE80211_CSA_FINISH(vif);
}

static void morse_ecsa_chswitch_work(struct work_struct *work)
{
	struct morse_vif *mors_vif = container_of(work, struct morse_vif, ecsa_chswitch_work.work);
	struct morse_channel_info *ch;
	struct ieee80211_vif *vif = morse_vif_to_ieee80211_vif(mors_vif);
	struct morse *mors = morse_vif_to_morse(mors_vif);

	if (vif->type == NL80211_IFTYPE_AP)
		ch = &mors_vif->custom_configs->default_bw_info;
	else
		ch = &mors_vif->assoc_sta_channel_info;

	mutex_lock(&mors->lock);
	morse_mac_set_channel(mors,
			      ch->op_chan_freq_hz,
			      ch->pri_1mhz_chan_idx, ch->op_bw_mhz, ch->pri_bw_mhz, true, __func__);
	mutex_unlock(&mors->lock);
}

static bool morse_mac_ecsa_begin_channel_switch(struct morse *mors)
{
	int ret;

	mors->in_scan = true;
	ret = morse_cmd_cfg_scan(mors, true, false);
	if (ret) {
		MORSE_ECSA_ERR(mors, "%s: morse_cmd_cfg_scan failed %d", __func__, ret);
		return false;
	} else {
		return true;
	}
}

static bool morse_mac_ecsa_finish_channel_switch(struct morse *mors)
{
	int ret;

	mors->in_scan = false;
	ret = morse_cmd_cfg_scan(mors, false, false);
	if (ret) {
		MORSE_ECSA_ERR(mors, "%s: morse_cmd_cfg_scan failed %d", __func__, ret);
		return false;
	} else {
		return true;
	}
}

/* Update the ecsa channel config in mors_vif and mors channel info */
static void morse_mac_ecsa_update_bss_chan_info(struct morse_vif *mors_vif)
{
	struct ieee80211_vif *vif = morse_vif_to_ieee80211_vif(mors_vif);

	/* Update default bandwidth info used during channel change for 1mhz primary channel idx
	 * and bw.
	 */
	memcpy(&mors_vif->custom_configs->default_bw_info, &mors_vif->ecsa_channel_info,
	       sizeof(mors_vif->ecsa_channel_info));

	/* Update channel info used in AP mode for S1G Operation IE */
	memcpy(&mors_vif->custom_configs->channel_info, &mors_vif->ecsa_channel_info,
	       sizeof(mors_vif->ecsa_channel_info));

	/* Update assoc sta channel info used in STA mode to restore the primary channel config
	 * after scan.
	 */
	if (vif->type == NL80211_IFTYPE_STATION)
		memcpy(&mors_vif->assoc_sta_channel_info, &mors_vif->ecsa_channel_info,
		       sizeof(mors_vif->ecsa_channel_info));

	if (vif->type == NL80211_IFTYPE_AP)
		mors_vif->mask_ecsa_info_in_beacon = true;
}

/*
 * API to verify if we are switching to new channel as part of ECSA and update
 * the ECSA channel info in mors and mors_vif data structures.
 * It also configures scan state in the firmware to postpone the PHY calibration
 * so that AP can switch to new channel within beacon interval. Otherwise
 * channel change is taking 230 - 440msecs due to PHY DC calibration.
 * PHY calibration is not performed during scan.
 */
static bool morse_mac_ecsa_channel_switch_in_progress(struct morse *mors, u32 freq_hz, u8 op_bw_mhz,
						      u8 *pri_bw_mhz, u8 *pri_1mhz_chan_idx)
{
	struct ieee80211_vif *vif;
	struct morse_vif *mors_vif;
	bool scan_configured = false;
	u16 if_idx;

	for (if_idx = 0; if_idx < mors->max_vifs; if_idx++) {
		vif = morse_get_vif_from_vif_id(mors, if_idx);

		if (!vif || !morse_mac_is_csa_active(vif) ||
		    (vif->type != NL80211_IFTYPE_AP && vif->type != NL80211_IFTYPE_STATION))
			continue;

		mors_vif = ieee80211_vif_to_morse_vif(vif);

		if (freq_hz == mors_vif->ecsa_channel_info.op_chan_freq_hz &&
		    op_bw_mhz == mors_vif->ecsa_channel_info.op_bw_mhz) {
			/* Update the new ecsa channel config in mors_vif and mors channel info */
			morse_mac_ecsa_update_bss_chan_info(mors_vif);

			/*
			 * Update pri_bw_mhz and pri_1mhz_chan_idx which are used in
			 * morse_mac_ops_config to switch to the new channel. mac80211 doesn't pass
			 * this primary chan info to driver
			 */
			*pri_bw_mhz = mors_vif->ecsa_channel_info.pri_bw_mhz;
			*pri_1mhz_chan_idx = mors_vif->ecsa_channel_info.pri_1mhz_chan_idx;

			/* Clear the scan list in STA mode as cssid list contains AP with old S1G Op
			 * IE.
			 */
			if (vif->type == NL80211_IFTYPE_STATION) {
				morse_dot11ah_clear_list();
				/* Reset channel info */
				memset(&mors_vif->ecsa_channel_info, 0,
				       sizeof(mors_vif->ecsa_channel_info));

				/* Reset beacon change seq */
				mors_vif->s1g_bcn_change_seq = INVALID_BCN_CHANGE_SEQ_NUM;
			}

			/*
			 * SW-8055: Set ecsa_chan_configured to configure the channel again to
			 * perform DC calibration.
			 * This change is not required once the periodic PHY DC calibration is
			 * enabled in fw and it will be tracked through jira id SW-8055.
			 * We delay this until AP sends 1st beacon in new channel and on client side
			 * until it receives first beacon. On client side, 1st beacon is needed in
			 * mac80211 to unblock the traffic, if it has blocked during start of the
			 * ECSA.
			 */
			scan_configured = morse_mac_ecsa_begin_channel_switch(mors);

			mors_vif->ecsa_chan_configured = true;

			MORSE_ECSA_INFO(mors,
				   "%s: pri_bw_mhz=%d, pri_1mhz_chan_idx=%d, bcn_change_seq =%x",
				   __func__, *pri_bw_mhz, *pri_1mhz_chan_idx,
				   mors_vif->s1g_bcn_change_seq);
		}
	}
	return scan_configured;
}

static bool country_codes_are_equal(const char *cc1, const char *cc2)
{
	return (cc1[0] == cc2[0]) && (cc1[1] == cc2[1]);
}

static void morse_mcast_tx_rate_eval_work(struct work_struct *work)
{
	struct morse_vif *mors_vif = container_of(work, struct morse_vif, mcast_tx_rate_work.work);
	struct morse *mors;

	mors = morse_vif_to_morse(mors_vif);

	mutex_lock(&mors->lock);
	morse_rc_vif_update_mcast_rate(mors, mors_vif);
	mutex_unlock(&mors->lock);
	morse_dbg(FEATURE_ID_RATECONTROL, mors, "%s: Updated rate=%u, bw=%u, gi=%u\n", __func__,
		   mors_vif->mcast_tx_rate.rate, mors_vif->mcast_tx_rate.bw,
		   mors_vif->mcast_tx_rate.guard);

	schedule_delayed_work(&mors_vif->mcast_tx_rate_work,
		msecs_to_jiffies(MULTICAST_TX_RATE_EVAL_WORK_PERIOD_MS));
}

static int morse_mac_ops_add_interface(struct ieee80211_hw *hw, struct ieee80211_vif *vif)
{
	int ret = 0;
	int i;
	struct morse *mors = hw->priv;
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;
	bool is_ap = false;
	bool is_sta = false;

	if (!morse_mac_is_iface_type_supported(vif)) {
		MORSE_ERR(mors, "%s: Attempt to add type %d, not supported\n", __func__, vif->type);
		return -EOPNOTSUPP;
	}

	mutex_lock(&mors->lock);

	if (morse_mac_is_iface_ap_type(vif) && !country_codes_are_equal(mors->country, country)) {
		MORSE_ERR(mors,
			  "Regulatory domain %c%c is not consistent with loaded country code %c%c",
			  mors->country[0], mors->country[1], country[0], country[1]);
		ret = -EPERM;
		goto exit;
	}

	mors_vif->beaconing_enabled = false;
	ret = morse_cmd_add_if(mors, &mors_vif->id, vif->addr, vif->type);
	if (ret) {
		MORSE_ERR(mors, "morse_cmd_add_if failed %d", ret);
		goto exit;
	}

	morse_vendor_ie_init_interface(mors_vif);

	if (mors_vif->id >= mors->max_vifs) {
		MORSE_ERR(mors, "vif_id is too large %u\n", mors_vif->id);
		goto exit;
	}

	if (mors_vif->id != (mors_vif->id & MORSE_TX_CONF_FLAGS_VIF_ID_MASK)) {
		MORSE_ERR(mors, "%s invalid vif_id %u\n", __func__, mors_vif->id);
		ret = EOPNOTSUPP;
		goto exit;
	}
	vif->driver_flags |= IEEE80211_VIF_BEACON_FILTER | IEEE80211_VIF_SUPPORTS_CQM_RSSI;

	morse_vif_add(mors, mors_vif->id, vif);

	/* Set control response frame bandwidth for this interface.
	 * May have already been set using vendor commands but the chip would
	 * reject while interface is down. Send again after interface is up
	 */
	if (mors_vif->ctrl_resp_out_1mhz_en)
		morse_cmd_set_cr_bw(mors, mors_vif, 0, 1);
	if (mors_vif->ctrl_resp_in_1mhz_en)
		morse_cmd_set_cr_bw(mors, mors_vif, 1, 1);

	mors_vif->ap = NULL;
	mors_vif->custom_configs = &mors->custom_configs;

	/* Enable PV1 only for AP or STA */
	if (morse_mac_is_iface_infra_bss_type(vif)) {
		mors_vif->enable_pv1 = enable_pv1;
		mors_pv1_init_vif(mors_vif);
	}

	if (morse_mac_is_iface_ap_type(vif)) {
		mors_vif->dtim_count = 0;
		mors_vif->ap = kzalloc(sizeof(*mors_vif->ap), GFP_KERNEL);
		if (!mors_vif->ap) {
			ret = -ENOMEM;
			goto exit;
		}

		if (mors->cfg->enable_short_bcn_as_dtim && enable_page_slicing) {
			MORSE_ERR(mors,
				  "%s: short dtim beacon can't be enabled while page slicing is enabled, disabling short dtim beacon",
				  __func__);
			mors->cfg->enable_short_bcn_as_dtim = false;
		}
		ret = morse_beacon_init(mors_vif);
		if (ret) {
			MORSE_ERR(mors, "%s: morse_beacon_init failed for VIF %u",
				  __func__, mors_vif->id);
			goto exit;
		}

		ret = morse_ndp_probe_req_resp_init(mors_vif);
		if (ret)
			goto exit;

		mors_vif->ap->mors_vif = mors_vif;

		if (ieee80211_vif_is_mesh(vif)) {
			ret = morse_mesh_init(mors_vif);
			if (ret) {
				MORSE_ERR(mors, "%s: morse_mac_mesh_init failed for VIF %u",
					  __func__, mors_vif->id);
				goto exit;
			}
		}
		/* TODO: SW-9660 Handle the MMSS exchange in action frames for IBSS */
		if (vif->type == NL80211_IFTYPE_ADHOC)
			morse_cmd_ack_timeout_adjust(mors, mors_vif->id,
						     DEFAULT_MORSE_IBSS_ACK_TIMEOUT_ADJUST_US);

		if (vif->type == NL80211_IFTYPE_AP || ieee80211_vif_is_mesh(vif))
			mors_vif->enable_multicast_rate_control = enable_mcast_rate_control;
		else
			mors_vif->enable_multicast_rate_control = false;

		if (mors_vif->enable_multicast_rate_control) {
			INIT_DELAYED_WORK(&mors_vif->mcast_tx_rate_work,
				morse_mcast_tx_rate_eval_work);
			mors_vif->mcast_tx_rate_throughput = 0;
			schedule_delayed_work(&mors_vif->mcast_tx_rate_work,
				msecs_to_jiffies(MULTICAST_TX_RATE_EVAL_WORK_PERIOD_MS));
		}

		morse_pre_assoc_peer_list_vif_take(mors);
	}
	mors_vif->epoch = get_jiffies_64();

	/* Get and assign the interface's capabilities */
	ret = morse_cmd_get_capabilities(mors, mors_vif->id, &mors_vif->capabilities);
	if (ret) {
		/* If this command failed, We might cause a timeout for the callback */
		MORSE_ERR(mors, "%s: morse_cmd_get_capabilities failed for VIF %d",
			  __func__, mors_vif->id);
		goto exit;
	}

	/* Enable features */
	switch (vif->type) {
	case NL80211_IFTYPE_AP:
		is_ap = true;

		if (enable_cac) {
			/* STA mode CAC is enabled via wpa_supplicant */
			ret = morse_cac_init(mors, mors_vif);
			MORSE_WARN_ON(FEATURE_ID_DEFAULT, ret);
		}

		ret = morse_raw_init(mors_vif, enable_raw && (!enable_wiphy));
		MORSE_WARN_ON(FEATURE_ID_DEFAULT, ret);

		ret = morse_bss_stats_init(mors_vif);
		MORSE_WARN_ON(FEATURE_ID_DEFAULT, ret);
		break;
	case NL80211_IFTYPE_STATION:
		is_sta = true;

		if (enable_dynamic_ps_offload)
			vif->driver_flags |= IEEE80211_VIF_SUPPORTS_UAPSD;

		morse_send_probe_req_init(vif);

		if (enable_bcn_change_seq_monitor)
			morse_send_probe_req_enable(vif, true);
		break;
	default:
		break;
	}

#ifdef CONFIG_ANDROID
	mors_vif->apf.enabled = false;
#endif
	morse_twt_init_vif(mors, mors_vif, enable_twt, is_ap, is_sta,
		(enable_ps == POWERSAVE_MODE_FULLY_ENABLED),
		enable_dynamic_ps_offload,
		enable_mac80211_connection_monitor);

	/* Initialize the change seq to 0. Other parameters keeping track of IE changes */
	morse_mac_reset_s1g_bcn_change_seq_params(hw, vif);

	/* Reset all stored private data backups, if any */
	morse_mac_reset_sta_backup(mors, mors_vif);

	MORSE_DBG(mors, "FW Manifest Flags for VIF %d:", mors_vif->id);
	for (i = 0; i < ARRAY_SIZE(mors_vif->capabilities.flags); i++)
		MORSE_DBG(mors, "%d: 0x%x", i, mors_vif->capabilities.flags[i]);

	morse_mac_set_s1g_capab(vif);

	ieee80211_wake_queues(mors->hw);

	/* Only stations support PS filtering out-of-the-box
	 * (re-buffered internally to driver).
	 */
	mors_vif->supports_ps_filter = (vif->type == NL80211_IFTYPE_STATION);

#if KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE
	init_timer(&mors_vif->chswitch_timer);
	mors_vif->chswitch_timer.data = (unsigned long)mors_vif;
	mors_vif->chswitch_timer.function = morse_chswitch_timer;
	add_timer(&mors_vif->chswitch_timer);
#else
	timer_setup(&mors_vif->chswitch_timer, morse_chswitch_timer, 0);
#endif
	mors_vif->ecsa_chan_configured = false;
	mors_vif->mask_ecsa_info_in_beacon = false;

	/* Reset channel info */
	memset(&mors_vif->ecsa_channel_info, 0, sizeof(mors_vif->ecsa_channel_info));

	INIT_DELAYED_WORK(&mors_vif->ecsa_chswitch_work, morse_ecsa_chswitch_work);

	if (vif->type == NL80211_IFTYPE_AP)
		mors->num_of_ap_interfaces++;

	MORSE_INFO(mors, "%s: [id:%d %s]\n", __func__,
		   mors_vif->id, morse_iftype_to_str(vif->type));
exit:
	mutex_unlock(&mors->lock);

	return ret;
}

static void morse_mac_ops_remove_interface(struct ieee80211_hw *hw, struct ieee80211_vif *vif)
{
	int ret;
	struct morse *mors = hw->priv;
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;

	if (!mors_vif)
		return;

	if (mors_vif->enable_multicast_rate_control)
		cancel_delayed_work_sync(&mors_vif->mcast_tx_rate_work);

	mutex_lock(&mors->lock);

	if (vif->type == NL80211_IFTYPE_AP) {
		if (mors->num_of_ap_interfaces)
			mors->num_of_ap_interfaces--;
		else
			MORSE_ERR(mors, "%s: Invalid num_of_ap_interfaces=%d\n", __func__,
				mors->num_of_ap_interfaces);
	}

	/* Must be done before removing AP interface */
	morse_cac_deinit(mors_vif);

	/* Make sure no beacons are sent */
	if (morse_mac_is_iface_ap_type(vif)) {
		morse_ndp_probe_req_resp_finish(mors_vif);
		if (vif->type == NL80211_IFTYPE_AP && morse_mbssid_ie_enabled(mors)) {
			if (mors_vif->id == mors_vif->mbssid_info.transmitter_vif_id)
				morse_beacon_finish(mors_vif);
		} else {
			morse_beacon_finish(mors_vif);
		}
		if (ieee80211_vif_is_mesh(vif))
			morse_mesh_deinit(mors_vif);

		if (vif->type == NL80211_IFTYPE_AP) {
			morse_bss_stats_deinit(mors_vif);
			morse_raw_finish(mors_vif);
		}

		morse_pre_assoc_peer_list_vif_release(mors);
		kfree(mors_vif->ap);
		mors_vif->ap = NULL;
	}

	if (enable_bcn_change_seq_monitor && vif->type == NL80211_IFTYPE_STATION) {
		morse_send_probe_req_enable(vif, false);
		morse_send_probe_req_finish(vif);

		/* Free up probe req template buffer */
		if (mors_vif->probe_req_buf)
			dev_kfree_skb_any(mors_vif->probe_req_buf);

		mors_vif->probe_req_buf = NULL;
	}

	morse_twt_finish_vif(mors, mors_vif);

	if (vif->type == NL80211_IFTYPE_AP)
		morse_mbssid_ie_deinit_bss(mors, mors_vif);

	/* Clean up PV1 status only for AP & STA */
	if (morse_mac_is_iface_infra_bss_type(vif))
		morse_pv1_finish_vif(mors_vif);

	morse_vendor_ie_deinit_interface(mors_vif);

#ifdef CONFIG_ANDROID
	mors_vif->apf.enabled = false;
	MORSE_DBG(mors, "%s: Disabled APF\n", __func__);
#endif

	ret = morse_cmd_rm_if(mors, mors_vif->id);
	if (ret) {
		MORSE_ERR(mors, "morse_cmd_rm_if failed %d", ret);
		goto exit;
	}

	del_timer_sync(&mors_vif->chswitch_timer);
	flush_delayed_work(&mors_vif->ecsa_chswitch_work);

	/* If data TX is stopped, the LMAC will eventually send the
	 * TWT traffic event to unblock TX on reception of the iface teardown
	 * command. To be safe, however, explicitly unblock traffic here as well
	 * to prevent unintended consequences if the to-host unblock event is lost.
	 */
	if (vif->type == NL80211_IFTYPE_STATION &&
	    test_bit(MORSE_STATE_FLAG_DATA_TX_STOPPED, &mors->state_flags)) {
		set_bit(MORSE_DATA_TRAFFIC_RESUME_PEND, &mors->chip_if->event_flags);
		queue_work(mors->chip_wq, &mors->chip_if_work);
	}

	MORSE_INFO(mors, "%s: [id:%d %s]\n", __func__,
		   mors_vif->id, morse_iftype_to_str(vif->type));

exit:
	if (mors_vif)
		morse_vif_remove(mors, mors_vif->id);
	mutex_unlock(&mors->lock);
}

static s32 morse_mac_get_max_txpower(struct morse *mors)
{
	int ret;
	s32 power_mbm;

	/* Retrieve maximum TX power the chip can transmit */
	ret = morse_cmd_get_max_txpower(mors, &power_mbm);
	if (ret) {
		MORSE_ERR(mors, "%s failed (%d), using default tx max power %d mBm\n",
			  __func__, ret, tx_max_power_mbm);
		return tx_max_power_mbm;
	}

	MORSE_INFO(mors, "Max tx power detected %d mBm\n", power_mbm);

	return power_mbm;
}

s32 morse_mac_set_txpower(struct morse *mors, s32 power_mbm)
{
	int ret;
	s32 out_power_mbm;

	if (mors->tx_max_power_mbm == INT_MAX)
		mors->tx_max_power_mbm = morse_mac_get_max_txpower(mors);

	power_mbm = min(power_mbm, mors->tx_max_power_mbm);

	if (power_mbm == mors->tx_power_mbm)
		return mors->tx_power_mbm;

	ret = morse_cmd_set_txpower(mors, &out_power_mbm, power_mbm);
	if (ret) {
		MORSE_ERR(mors, "%s: failed, power %d mBm ret %d\n", __func__, power_mbm, ret);
		return mors->tx_power_mbm;
	}

	if (out_power_mbm != mors->tx_power_mbm) {
		MORSE_INFO(mors, "%s: %d -> %d mBm\n", __func__, mors->tx_power_mbm, out_power_mbm);
		mors->tx_power_mbm = out_power_mbm;
	}

	return mors->tx_power_mbm;
}

static int set_duty_cycle(struct morse *mors, const struct morse_reg_rule *mors_reg_rule,
			bool have_ap)
{
	int ret = 0;
	u32 duty_cycle;

	if (have_ap)
		duty_cycle = mors_reg_rule->duty_cycle.ap;
	else
		duty_cycle = mors_reg_rule->duty_cycle.sta;

	/* Burst mode duty cycling will not operate correctly if dynamic ps offload is switched off
	 * or if the mac80211 beacon loss monitor is switched on.
	 */
	if (duty_cycle_mode == MORSE_CMD_DUTY_CYCLE_MODE_BURST) {
		if (!enable_dynamic_ps_offload)
			MORSE_ERR(mors,
				"%s: Duty cycle burst configured but ps offload disabled\n",
				__func__);
		if (enable_mac80211_connection_monitor)
			MORSE_ERR(mors,
				"%s: Duty cycle burst configured but connection monitor enabled\n",
				__func__);
	}

	if (mors->custom_configs.duty_cycle)
		duty_cycle = min(duty_cycle, mors->custom_configs.duty_cycle);

	MORSE_DBG(mors, "Setting duty cycle to %d (omit_ctrl_resp %d)",
			duty_cycle,
			mors_reg_rule->duty_cycle.omit_ctrl_resp ? 1 : 0);

	ret = morse_cmd_set_duty_cycle(mors, duty_cycle_mode, duty_cycle,
				mors_reg_rule->duty_cycle.omit_ctrl_resp);
	if (ret)
		MORSE_ERR(mors, "morse_cmd_set_duty_cycle failed %d\n",
				ret);
	else
		mors->duty_cycle = duty_cycle;

	return ret;
}

static int morse_mac_change_channel(struct ieee80211_hw *hw)
{
	struct morse *mors = hw->priv;
	struct ieee80211_conf *conf = &hw->conf;
	struct morse_channel_info info;
	int ret = 0;
	bool scan_configured = false;
	const struct morse_dot11ah_channel *chan_s1g;
	const struct morse_reg_rule *mors_reg_rule;
	u32 freq_hz;
	u8 op_bw_mhz;
	u8 pri_1mhz_chan_idx = mors->custom_configs.default_bw_info.pri_1mhz_chan_idx;
	u8 pri_bw_mhz = mors->custom_configs.default_bw_info.pri_bw_mhz;
	u8 bssid[ETH_ALEN];
	const char *region = morse_dot11ah_get_region_str();
	int vif_id;
	bool have_ap = morse_get_ap_vif(mors) ? true : false;

	/* Convert 5G channel to S1G channel */
	chan_s1g = morse_dot11ah_channel_chandef_to_s1g(&conf->chandef);
	if (!chan_s1g) {
		if (!mors->in_scan)
			MORSE_DBG(mors, "%s: Set channel index %d failed: not in region map %s\n",
					__func__, conf->chandef.chan->hw_value, region);

		return -ENOENT;
	}

	freq_hz = KHZ_TO_HZ(morse_dot11ah_channel_to_freq_khz(chan_s1g->ch.hw_value));

	op_bw_mhz = (chan_s1g->ch.flags & IEEE80211_CHAN_8MHZ) ? 8 :
		(chan_s1g->ch.flags & IEEE80211_CHAN_4MHZ) ? 4 :
		(chan_s1g->ch.flags & IEEE80211_CHAN_2MHZ) ? 2 : 1;

	/* Update primary channel info based on BSS only if no AP interfaces */
	if (!have_ap) {
		for (vif_id = 0; vif_id < mors->max_vifs; vif_id++) {
			struct ieee80211_vif *vif = morse_get_vif_from_vif_id(mors, vif_id);

			if (mors->in_scan) {
				/* SW-2278 For interop:
				 * Other vendors appear to be responding to our
				 * 1Mhz probes requests with 2MHz probe responses -
				 *
				 * As a WAR, we will always configure our operating
				 * width to 2MHz to be able to receive these responses.
				 */
				pri_bw_mhz = op_bw_mhz > 1 ? 2 : 1;
			} else if (vif && vif->bss_conf.bssid) {
				/* If we are a STA and have a BSS/AP conf, try to use the AP's chan
				 * info.
				 */
				memcpy(bssid, vif->bss_conf.bssid, ETH_ALEN);
				if (morse_mac_find_channel_info_for_bssid(bssid, &info)) {
					if (freq_hz == info.op_chan_freq_hz) {
						pri_bw_mhz = info.pri_bw_mhz;
						pri_1mhz_chan_idx = info.pri_1mhz_chan_idx;
					}
				}
				break;
			} else if (!vif) {
				if (!morse_dot11_find_bssid_on_channel(freq_hz, bssid)) {
					/* If we don't have a VIF or aren't an AP, use
					 * channel info from the first bssid in the stored list.
					 * WARNING:
					 * When there are multiple APs, This can cause incorrect
					 * channel config leading to problems such as auth failure.
					 */
					MORSE_WARN(mors,
						"%s: Using first stored bssid info for channel config\n",
						__func__);
					morse_mac_find_channel_info_for_bssid(bssid, &info);
					pri_bw_mhz = info.pri_bw_mhz;
					pri_1mhz_chan_idx = info.pri_1mhz_chan_idx;
				}
				break;
			}
		}
	}
	scan_configured = morse_mac_ecsa_channel_switch_in_progress(mors, freq_hz, op_bw_mhz,
									&pri_bw_mhz,
									&pri_1mhz_chan_idx);

	/* Final sanity check:
	 * pri_bw_mhz is either 1MHZ or 2MHZ
	 * pri_bw_mhz shouldn't 2 if op_bw_mhz is 1
	 * pri_1mhz_index is based on op_bw_mhz
	 */
	pri_bw_mhz = min_t(u8, pri_bw_mhz, 2);
	pri_bw_mhz = min_t(u8, pri_bw_mhz, op_bw_mhz);
	pri_1mhz_chan_idx = op_bw_mhz == 8 ? min_t(u8, pri_1mhz_chan_idx, 7) :
		op_bw_mhz == 4 ? min_t(u8, pri_1mhz_chan_idx, 3) :
		op_bw_mhz == 2 ? min_t(u8, pri_1mhz_chan_idx, 1) : 0;

	mors->channel_num_80211n = conf->chandef.chan->hw_value;

	ret = morse_mac_set_channel(mors, freq_hz, pri_1mhz_chan_idx, op_bw_mhz, pri_bw_mhz,
				    false, __func__);

	if (ret == MORSE_RET_EPERM) {
		MORSE_ERR(mors, "%s: HW does not permit channel (f:%u kHz, bw:%u MHz)",
			  __func__, HZ_TO_KHZ(freq_hz), op_bw_mhz);

		/* This channel should have been marked as unusable on boot */
		MORSE_WARN_ON(FEATURE_ID_DEFAULT,
			((conf->chandef.chan->flags & IEEE80211_CHAN_IGNORE) == 0));
		return ret;
	}

	if (scan_configured)
		morse_mac_ecsa_finish_channel_switch(mors);

	if (mors->in_scan)
		morse_survey_update_channel_usage(mors);

	if (!ret) {
		struct morse_channel_info *stored_info = &mors->custom_configs.channel_info;

		if (freq_hz != MORSE_CMD_CHANNEL_FREQ_NOT_SET)
			stored_info->op_chan_freq_hz = freq_hz;

		if (pri_1mhz_chan_idx != MORSE_CMD_CHANNEL_IDX_NOT_SET)
			stored_info->pri_1mhz_chan_idx = pri_1mhz_chan_idx;

		if (op_bw_mhz != MORSE_CMD_CHANNEL_BW_NOT_SET)
			stored_info->op_bw_mhz = op_bw_mhz;

		if (pri_bw_mhz != MORSE_CMD_CHANNEL_BW_NOT_SET)
			stored_info->pri_bw_mhz = pri_bw_mhz;

		/* Validate that primary does not exceed operating */
		stored_info->pri_bw_mhz =
			(stored_info->op_bw_mhz == 1) ? 1 : stored_info->pri_bw_mhz;

		/* Update Tx power using S1G max values if possible */
		morse_mac_set_txpower(mors, chan_s1g->ch.max_reg_power);
	}

	mors_reg_rule = morse_regdom_get_rule_for_freq(region,
							ieee80211_channel_to_khz
							(&chan_s1g->ch));
	if (mors_reg_rule) {
		if (enable_auto_duty_cycle)
			ret = set_duty_cycle(mors, mors_reg_rule, have_ap);
		if (!ret && enable_auto_mpsw) {
			MORSE_DBG(mors,
					"Setting MPSW to min %d us max %d us, window %d us\n",
					mors_reg_rule->mpsw.airtime_min_us,
					mors_reg_rule->mpsw.airtime_max_us,
					mors_reg_rule->mpsw.window_length_us);
			ret = morse_cmd_set_mpsw(mors, mors_reg_rule->mpsw.airtime_min_us,
						mors_reg_rule->mpsw.airtime_max_us,
						mors_reg_rule->mpsw.window_length_us);
			if (ret)
				MORSE_ERR(mors, "morse_cmd_set_mpsw failed %d\n", ret);
		}
	} else {
		MORSE_WARN(mors, "No reg rule for %s freq %d - duty cycle and mpsw not set\n",
				region, HZ_TO_KHZ(freq_hz));
	}

	return ret;
}

static int morse_mac_ops_config(struct ieee80211_hw *hw, u32 changed)
{
	int err = 0;
	struct morse *mors = hw->priv;
	struct ieee80211_conf *conf = &hw->conf;
	bool channel_valid;

	mutex_lock(&mors->lock);
	if (!mors->started)
		goto exit;

	if (changed & IEEE80211_CONF_CHANGE_LISTEN_INTERVAL)
		MORSE_DBG(mors, "ieee80211_conf_change_listen_interval\n");

	if (changed & IEEE80211_CONF_CHANGE_MONITOR) {
		int ret = 0;
		struct morse_vif *mon_if = &mors->mon_if;

		MORSE_DBG(mors, "%s: change monitor mode: %s\n",
			  __func__, conf->flags & IEEE80211_CONF_MONITOR ? "true" : "false");
		if (conf->flags & IEEE80211_CONF_MONITOR) {
			ret = morse_cmd_add_if(mors,
					       &mon_if->id, mors->macaddr, NL80211_IFTYPE_MONITOR);
			if (ret)
				MORSE_ERR(mors, "monitor interface add failed %d\n", ret);
			else
				MORSE_INFO(mors, "monitor interfaced added %d\n", mon_if->id);
			mors->monitor_mode = true;
		} else {
			if (mon_if->id != INVALID_VIF_ID) {
				morse_cmd_rm_if(mors, mon_if->id);
				MORSE_INFO(mors, "monitor interfaced removed\n");
			}
			mon_if->id = INVALID_VIF_ID;
			mors->monitor_mode = false;
		}
	}

	channel_valid = conf->chandef.chan &&
	    ((conf->chandef.chan->flags & IEEE80211_CHAN_DISABLED) == 0) &&
	    ((conf->chandef.chan->flags & IEEE80211_CHAN_IGNORE) == 0);

	if ((changed & IEEE80211_CONF_CHANGE_CHANNEL) && channel_valid) {
		err = morse_mac_change_channel(hw);
		if (err)
			goto exit;
	}

	if ((changed & IEEE80211_CONF_CHANGE_POWER) && !(conf->flags & IEEE80211_CONF_MONITOR) &&
	    channel_valid) {
		s32 power_mbm = DBM_TO_MBM(conf->power_level);
		const struct morse_dot11ah_channel *chan_s1g;

		chan_s1g = morse_dot11ah_channel_chandef_to_s1g(&conf->chandef);
		if (chan_s1g)
			power_mbm = min(chan_s1g->ch.max_reg_power, power_mbm);

		power_mbm = morse_mac_set_txpower(mors, power_mbm);
		conf->power_level = MBM_TO_DBM(power_mbm);
	}

	if (changed & IEEE80211_CONF_CHANGE_RETRY_LIMITS)
		MORSE_DBG(mors, "ieee80211_conf_change_retry_limits\n");

exit:
	mutex_unlock(&mors->lock);
	return err;
}

/* Return Tx power only when channel is configured and is the same as one in hw->conf */
static int morse_mac_ops_get_txpower(struct ieee80211_hw *hw, struct ieee80211_vif *vif, int *dbm)
{
	int err;
	struct morse *mors = hw->priv;
	struct ieee80211_chanctx_conf *chanctx_conf;
	struct cfg80211_chan_def *chandef = MORSE_BSS_CONF_CHAN_DEF(vif);

	/* Refer ieee80211_cfg_get_channel() */
	chanctx_conf = morse_mac_get_chantx_conf(vif);

	err = !chanctx_conf;

	if (err || !cfg80211_chandef_identical(chandef, &hw->conf.chandef))
		return -ENODATA;

	mutex_lock(&mors->lock);
	*dbm = MBM_TO_DBM(mors->tx_power_mbm);
	mutex_unlock(&mors->lock);

	return err;
}

static void morse_mac_config_ps(struct morse *mors, struct ieee80211_vif *vif)
{
#if KERNEL_VERSION(6, 0, 0) > MAC80211_VERSION_CODE
	bool en_ps = vif->bss_conf.ps;
#else
	bool en_ps = vif->cfg.ps;
#endif

	if (vif->type == NL80211_IFTYPE_MONITOR || morse_mac_is_iface_ap_type(vif))
		return;

	MORSE_INFO(mors, "%s: change powersave mode: %s (current %s)\n",
		   __func__, en_ps ? "true" : "false", mors->config_ps ? "true" : "false");

	if (mors->config_ps == en_ps)
		return;

	mors->config_ps = en_ps;

	if (enable_ps != POWERSAVE_MODE_FULLY_ENABLED)
		return;

	/* If we have GPIO pins wired. Let's control host-to-chip PS
	 * mechanism. Otherwise, ignore the command altogether.
	 */
	if (en_ps) {
		morse_cmd_set_ps(mors, true, enable_dynamic_ps_offload);
		morse_ps_enable(mors);
	} else {
		morse_ps_disable(mors);
		morse_cmd_set_ps(mors, false, false);
	}
}

static void
morse_mac_ops_bss_info_changed(struct ieee80211_hw *hw,
			       struct ieee80211_vif *vif,
			       struct ieee80211_bss_conf *info,
#if KERNEL_VERSION(6, 0, 0) > MAC80211_VERSION_CODE
			       u32 changed)
#else
			       u64 changed)
#endif
{
	struct morse *mors = hw->priv;
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;
	struct ieee80211_bss_conf *bss_conf = &vif->bss_conf;
	bool csa_active = morse_mac_is_csa_active(vif);

	mutex_lock(&mors->lock);

	if (changed & BSS_CHANGED_PS)
		morse_mac_config_ps(mors, vif);

	if (changed & BSS_CHANGED_BEACON)
		MORSE_INFO(mors,
				"BSS Changed beacon data, reset flag=%d, csa_active=%d ecsa_chan_configured=%d\n",
				mors_vif->mask_ecsa_info_in_beacon, csa_active,
				mors_vif->ecsa_chan_configured);

	if (vif->type != NL80211_IFTYPE_ADHOC &&
		(changed & BSS_CHANGED_BEACON_ENABLED)) {
		/* start command is sent, only if it was previously stopped */
		if ((mors_vif->beaconing_enabled && bss_conf->enable_beacon) ||
			!bss_conf->enable_beacon) {
			MORSE_INFO(mors, "BSS [vif_id:%d %s] beaconing %s\n",
				mors_vif->id, morse_iftype_to_str(vif->type),
				bss_conf->enable_beacon ? "restarted" : "stopped");
			morse_cmd_config_beacon_timer(mors, mors_vif, bss_conf->enable_beacon);
		}
		mors_vif->beaconing_enabled = true;

		/* Handle only stop mesh. Start mesh will be handled, when supplicant
		 * configures mesh id and other params.
		 */
		if (ieee80211_vif_is_mesh(vif) && !vif->bss_conf.enable_beacon) {
			int ret = morse_cmd_cfg_mesh_bss(mors_vif, true);

			mors_vif->mesh->is_mesh_active = false;
			if (ret)
				MORSE_ERR(mors, "Stop mesh beaconing ret=%d\n", ret);
		}
	}

	if (changed & BSS_CHANGED_BANDWIDTH) {
		MORSE_INFO(mors, "BSS Changed BW, changed=0x%llx, jiffies=%ld, csa_active=%d\n",
			   (u64)changed, jiffies, csa_active);

#ifdef CONFIG_MORSE_RC
		if (csa_active && vif->type == NL80211_IFTYPE_AP && mors_vif->ap->num_stas)
			morse_rc_reinit_stas(mors, vif);
#endif
	}

	if (changed & BSS_CHANGED_BEACON_INT || changed & BSS_CHANGED_SSID) {
		int ret;
		u32 cssid = morse_vif_generate_cssid(vif);

		ret = morse_cmd_cfg_bss(mors, mors_vif->id,
					info->beacon_int, info->dtim_period, cssid);
		if (ret)
			MORSE_ERR(mors, "morse_cmd_cfg_bss fail %d\n", ret);
		else
			MORSE_INFO(mors, "Beacon interval set %d\n", info->beacon_int);

		morse_page_slicing_init(vif, info->dtim_period, enable_page_slicing);
	}

	/*
	 * SW-5031: Keep track of IBSS network notifications. These
	 * are invoked when
	 * a. node joins the IBSS
	 * b. creates new IBSS
	 * c. node leaves the IBSS or disconnects from IBSS
	 *
	 * For a & b, bss_conf.enable_beacon is set to TRUE and for only
	 * case b (creates new IBSS) bss_conf.ibss_creator is set to true.
	 *
	 * For c, bss_conf.enable_beacon is set to false
	 */
	if ((changed & BSS_CHANGED_IBSS) && vif->type == NL80211_IFTYPE_ADHOC) {
		int ret = 0;
		bool ibss_creator = morse_mac_is_ibss_creator(vif);

		/** If enable_beacon is set to false, stop the IBSS.
		 *  enable_beacon seems to be set false even for BSS.
		 *  Need to review later if it needs to be checked for
		 *  AP mode as well.
		 */
		bool stop_ibss = (!vif->bss_conf.enable_beacon);

		ret = morse_cmd_cfg_ibss(mors, mors_vif->id,
					 vif->bss_conf.bssid, ibss_creator,
					 stop_ibss);
		if (ret)
			MORSE_ERR(mors, "morse_cmd_cfg_ibss fail %d\n", ret);
		else
			MORSE_INFO(mors, "IBSS creator: %d stop_ibss:%d\n",
				   ibss_creator, stop_ibss);
	}

	/* SW-5445: Get the template probe request buffer populated in this
	 * event handler and use it on detection of beacon change seq number
	 */
	if (vif->type == NL80211_IFTYPE_STATION && changed & BSS_CHANGED_ASSOC && bss_conf) {
		mors_vif->is_sta_assoc = morse_mac_is_sta_vif_associated(vif);

		/* Request for new template buffer only on new association */
		if (enable_bcn_change_seq_monitor && mors_vif->is_sta_assoc) {
			/* Free up old template buffer */
			if (mors_vif->probe_req_buf)
				dev_kfree_skb_any(mors_vif->probe_req_buf);

			mors_vif->probe_req_buf = ieee80211_ap_probereq_get(mors->hw, vif);
			mors_vif->s1g_bcn_change_seq = INVALID_BCN_CHANGE_SEQ_NUM;

			if (!mors_vif->probe_req_buf)
				MORSE_ERR(mors, "%s: ieee80211_ap_probereq_get failed\n", __func__);
		}
	}

	/* SW-4817: Note that we are 'repurposing' this to configure ARP offload.
	 * Instead of arp_addr_list being used purely for ARP filtering (as mac80211 expects),
	 * the firmware will AUTOMATICALLY respond to ARP requests addressed to the first IP in this
	 * table. ARP requests addressed to the first IP of this table will NEVER make their way
	 * to Linux, instead having the response generated and transmitted in FW.
	 * The other IPs in this table will behave as mac80211 expects and will be allowed to pass.
	 */
	if (changed & BSS_CHANGED_ARP_FILTER &&
	    vif->type == NL80211_IFTYPE_STATION &&
	    mors_vif->custom_configs->enable_arp_offload)
		morse_cmd_arp_offload_update_ip_table(mors, mors_vif->id,
#if KERNEL_VERSION(6, 0, 0) > MAC80211_VERSION_CODE
						      info->arp_addr_cnt, info->arp_addr_list);
#else
						      vif->cfg.arp_addr_cnt,
						      vif->cfg.arp_addr_list);
#endif

	if ((changed & BSS_CHANGED_CQM) && vif->type == NL80211_IFTYPE_STATION) {
		morse_cmd_set_cqm_rssi(mors, mors_vif->id,
				info->cqm_rssi_thold, info->cqm_rssi_hyst);
	}

	mutex_unlock(&mors->lock);
}

static int morse_mac_ops_get_survey(struct ieee80211_hw *hw, int idx, struct survey_info *survey)
{
	struct morse *mors = hw->priv;
	struct ieee80211_supported_band *sband;
	const struct morse_dot11ah_channel *chan_s1g;
	struct morse_survey_rx_usage_record *record;
	u32 op_ch_bw;
	u32 freq_hz;
	int ret = 0;

	mutex_lock(&mors->lock);

	if (!enable_survey) {
		ret = -ENOENT;
		goto exit;
	}

	sband = hw->wiphy->bands[NL80211_BAND_5GHZ];
	if (idx >= sband->n_channels) {
		ret = -ENOENT;
		goto exit;
	}

	survey->channel = &sband->channels[idx];

	chan_s1g = morse_dot11ah_5g_chan_to_s1g(survey->channel);
	if (!chan_s1g) {
		/* SW-4684: Channel is not supported in regdom, but we will upset Linux wireless if
		 * we return ENOENT here (nl80211_dump_survey loop will break if any error is
		 * returned). Alternatively, return 0 and set channel to NULL instead (to skip
		 * channel)
		 * TODO: a better way is to loop over the supported regdom channels only instead of
		 * the comprehensive supported list sband->channels
		 */
		survey->channel = NULL;
		survey->filled = 0;
		goto exit;
	}

	freq_hz = KHZ_TO_HZ(ieee80211_channel_to_khz(&chan_s1g->ch));

	op_ch_bw = (chan_s1g->ch.flags & IEEE80211_CHAN_1MHZ) ? 1 :
	    (chan_s1g->ch.flags & IEEE80211_CHAN_2MHZ) ? 2 :
	    (chan_s1g->ch.flags & IEEE80211_CHAN_4MHZ) ? 4 : 8;

	MORSE_DBG(mors, "%s: halow channel %d", __func__, chan_s1g->ch.hw_value);

	record = morse_survey_get_record(mors, freq_hz, op_ch_bw);

	if (!record) {
		survey->channel = NULL;
		survey->filled = 0;
		goto exit;
	}

	survey->noise = record->noise;
	survey->time = record->time_listen;
	do_div(survey->time, USEC_PER_MSEC);
	if (!survey->time && record->time_listen)
		survey->time = 1; /* round up to 1ms */

	survey->time_rx = record->time_rx;
	do_div(survey->time_rx, USEC_PER_MSEC);
	if (!survey->time_rx && record->time_rx)
		survey->time_rx = 1; /* round up to 1ms */

	survey->filled = SURVEY_INFO_NOISE_DBM | SURVEY_INFO_TIME | SURVEY_INFO_TIME_RX;

exit:
	mutex_unlock(&mors->lock);

	return ret;
}

static void
morse_mac_ops_configure_filter(struct ieee80211_hw *hw,
			       unsigned int changed_flags, unsigned int *total_flags, u64 multicast)
{
	struct morse *mors = hw->priv;
	struct mcast_filter *cmd = (void *)(unsigned long)multicast;
	struct morse_vif *mors_vif;
	struct ieee80211_vif *vif;
	int vif_id = 0;
	int ret = 0;

	mutex_lock(&mors->lock);

	if (!cmd)
		goto out;

	if (is_virtual_sta_test_mode())
		goto out;

	kfree(mors->mcast_filter);
	mors->mcast_filter = cmd;

	for (vif_id = 0; vif_id < mors->max_vifs; vif_id++) {
		vif = morse_get_vif_from_vif_id(mors, vif_id);
		if (vif && vif->type != NL80211_IFTYPE_MONITOR) {
			mors_vif = ieee80211_vif_to_morse_vif(vif);

			ret = morse_cmd_cfg_multicast_filter(mors, mors_vif);

			if (ret) {
				if (ret == MORSE_RET_ENOMEM)
					MORSE_INFO(mors,
						   "Multicast filtering disabled - too many groups (%d)\n",
						   mors->mcast_filter->count);
				else
					MORSE_ERR(mors, "Multicast filtering failed - rc=%d\n",
						  ret);

				mors->mcast_filter = NULL;
				kfree(cmd);
				break;
			}
		}
	}

out:
	mutex_unlock(&mors->lock);
	*total_flags &= 0;
}

static inline __le32 mac2leuint32(const unsigned char *addr)
{
	return cpu_to_le32(((u32)(addr[2]) << 24) |
			   ((u32)(addr[3]) << 16) |
			   ((u32)(addr[4]) << 8) | ((u32)(addr[5])));
}

static u64
morse_mac_ops_prepare_multicast(struct ieee80211_hw *hw, struct netdev_hw_addr_list *mc_list)
{
	struct morse *mors = hw->priv;
	struct mcast_filter *filter;
	struct netdev_hw_addr *addr;
	u16 len;
	u16 addr_count;

	if (!enable_mcast_whitelist) {
		MORSE_INFO(mors, "Multicast filtering disabled - enable=%d",
			   enable_mcast_whitelist);
		return 0;
	}

	addr_count = netdev_hw_addr_list_count(mc_list);
	len = sizeof(*filter) + addr_count * sizeof(filter->addr_list[0]);
	filter = kzalloc(len, GFP_ATOMIC);

	if (!filter)
		return 0;

	if (addr_count > MCAST_FILTER_COUNT_MAX) {
		MORSE_INFO(mors, "Multicast filtering disabled - too many groups (%d) > %u\n",
			   addr_count, (u16)MCAST_FILTER_COUNT_MAX);
		filter->count = 0;
	} else {
		netdev_hw_addr_list_for_each(addr, mc_list) {
			MORSE_DBG(mors, "mcast whitelist (%d): %pM\n", filter->count, addr->addr);
			filter->addr_list[filter->count++] = mac2leuint32(addr->addr);
		}
	}

	return (u64)(unsigned long)filter;
}

static void morse_mac_ops_sw_scan_start(struct ieee80211_hw *hw,
					struct ieee80211_vif *vif, const u8 *mac_addr)
{
	int ret;
	struct morse *mors = hw->priv;

	mutex_lock(&mors->lock);
	if (!mors->started) {
		MORSE_INFO(mors, "%s: Not started. Aborting\n", __func__);
		goto exit;
	}

	mors->in_scan = true;
	morse_survey_init_usage_records(mors);

	/* Some APs may change their configurations, clear cached AP list */
	morse_dot11ah_clear_list();

	ret = morse_cmd_cfg_scan(mors, true, (vif->type == NL80211_IFTYPE_AP));
	if (ret)
		MORSE_ERR(mors, "%s: morse_cmd_cfg_scan failed %d", __func__, ret);

exit:
	mutex_unlock(&mors->lock);
}

static void morse_mac_save_ecsa_chan_info(struct morse *mors, struct morse_vif *mors_vif,
					  struct ieee80211_ext_chansw_ie *ecsa_ie_info,
					  const u8 *chswitch_wrapper_ie_data,
					  u8 chswitch_wrapper_ie_datalen)
{
	const u8 *ie = NULL;
	u8 prim_1mhz_chan_loc = 0;
	u32 ecsa_new_chan_freq = morse_dot11ah_s1g_chan_to_s1g_freq(ecsa_ie_info->new_ch_num);

	if (chswitch_wrapper_ie_data)
		ie = cfg80211_find_ie(WLAN_EID_WIDE_BW_CHANNEL_SWITCH,
				      chswitch_wrapper_ie_data, chswitch_wrapper_ie_datalen);

	mors_vif->ecsa_channel_info.s1g_operating_class = ecsa_ie_info->new_operating_class;

	/* If wide bw channel switch wrapper IE is null then it is 1MHz Operating channel */
	if (!ie) {
		u32 channel_flags = morse_dot11ah_channel_get_flags(ecsa_ie_info->new_ch_num);

		mors_vif->ecsa_channel_info.op_chan_freq_hz = ecsa_new_chan_freq;
		if (channel_flags) {
			mors_vif->ecsa_channel_info.pri_bw_mhz = ch_flag_to_chan_bw(channel_flags);
		} else {
			mors_vif->ecsa_channel_info.pri_bw_mhz = 1;
			MORSE_WARN_ON(FEATURE_ID_ECSA, 1);
		}

		mors_vif->ecsa_channel_info.op_bw_mhz = mors_vif->ecsa_channel_info.pri_bw_mhz;
		mors_vif->ecsa_channel_info.pri_1mhz_chan_idx = 0;
	} else {
		struct ieee80211_wide_bw_chansw_ie *wbcsie =
		    (struct ieee80211_wide_bw_chansw_ie *)(ie + 2);

		mors_vif->ecsa_channel_info.op_chan_freq_hz =
		    morse_dot11ah_s1g_chan_to_s1g_freq(wbcsie->new_center_freq_seg0);
		/*
		 * Assign the op bw by incrementing new_channel_width, as new_channel_width
		 * is defined as per standard i.e, actual bw-1.
		 */
		mors_vif->ecsa_channel_info.op_bw_mhz =
			IEEE80211AH_S1G_OPERATION_GET_OP_CHAN_BW(wbcsie->new_channel_width);
		mors_vif->ecsa_channel_info.pri_bw_mhz =
			IEEE80211AH_S1G_OPERATION_GET_PRIM_CHAN_BW(wbcsie->new_channel_width);
		prim_1mhz_chan_loc =
			IEEE80211AH_S1G_OPERATION_GET_PRIM_CHAN_LOC(wbcsie->new_channel_width);

		mors_vif->ecsa_channel_info.pri_1mhz_chan_idx =
			morse_dot11_calc_prim_s1g_chan_loc(HZ_TO_KHZ(ecsa_new_chan_freq),
				HZ_TO_KHZ(mors_vif->ecsa_channel_info.op_chan_freq_hz),
				mors_vif->ecsa_channel_info.op_bw_mhz);
		/*
		 * pri_1mhz_chan_idx points to lower channel now, update it based on the 1MHz
		 * primary channel location when primary channel bandwidth is 2MHz.
		 */
		if (mors_vif->ecsa_channel_info.pri_bw_mhz == 2 &&
		    prim_1mhz_chan_loc)
			mors_vif->ecsa_channel_info.pri_1mhz_chan_idx++;
	}

	MORSE_ECSA_INFO(mors,
		   "%s: Chan Info:Prim_ch=%d, Op_ch=%d [%d-%d-%d], op_class=%d, count=%d\n",
		   __func__,
		   morse_dot11ah_s1g_chan_to_s1g_freq(ecsa_ie_info->new_ch_num),
		   mors_vif->ecsa_channel_info.op_chan_freq_hz,
		   mors_vif->ecsa_channel_info.op_bw_mhz,
		   mors_vif->ecsa_channel_info.pri_bw_mhz,
		   mors_vif->ecsa_channel_info.pri_1mhz_chan_idx,
		   mors_vif->ecsa_channel_info.s1g_operating_class,
		   ecsa_ie_info->count);
}

void morse_mac_process_ecsa_ie(struct morse *mors, struct ieee80211_vif *vif, struct sk_buff *skb)
{
	struct morse_vif *mors_vif;
	struct ieee80211_ext *s1g_beacon = (struct ieee80211_ext *)skb->data;
	const u8 *ie;
	u8 *s1g_ies = s1g_beacon->u.s1g_beacon.variable;
	int header_length = s1g_ies - skb->data;
	int s1g_ies_len = skb->len - header_length;

	mors_vif = ieee80211_vif_to_morse_vif(vif);

	if (le16_to_cpu(s1g_beacon->frame_control) & IEEE80211_FC_ANO) {
		s1g_ies += 1;
		s1g_ies_len -= 1;
	}

	ie = cfg80211_find_ie(WLAN_EID_EXT_CHANSWITCH_ANN, s1g_ies, s1g_ies_len);

	/* Process ECSA Info only once by checking operating channel */
	if (ie && !mors_vif->ecsa_channel_info.op_chan_freq_hz) {
		struct ieee80211_ext_chansw_ie *ecsa_ie_info =
		    (struct ieee80211_ext_chansw_ie *)(ie + 2);

		ie = cfg80211_find_ie(WLAN_EID_CHANNEL_SWITCH_WRAPPER, s1g_ies, s1g_ies_len);

		if (ie)
			morse_mac_save_ecsa_chan_info(mors, mors_vif, ecsa_ie_info, (ie + 2),
						      ie[1]);
		else
			morse_mac_save_ecsa_chan_info(mors, mors_vif, ecsa_ie_info, NULL, 0);
	}
}

static bool morse_check_chan_info_after_scan(struct morse *mors, struct morse_vif *mors_vif)
{
	if (memcmp(&mors_vif->assoc_sta_channel_info, &mors->custom_configs.channel_info,
		   sizeof(mors->custom_configs.channel_info)) == 0)
		return true;
	return false;
}

static inline bool morse_check_sta_associated(struct ieee80211_vif *vif,
					      struct morse_vif *mors_vif)
{
	if (vif->type == NL80211_IFTYPE_STATION && mors_vif->is_sta_assoc)
		return true;
	else
		return false;
}

static inline bool morse_check_ibss_node_joined(struct ieee80211_vif *vif,
						struct morse_vif *mors_vif)
{
	if (vif->type == NL80211_IFTYPE_ADHOC && mors_vif->is_ibss_node_joined)
		return true;
	else
		return false;
}

static void morse_mac_ops_sw_scan_complete(struct ieee80211_hw *hw, struct ieee80211_vif *vif)
{
	int ret;
	struct morse *mors = hw->priv;
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;

	if (!mors->started) {
		MORSE_INFO(mors, "%s: Not started. Aborting\n", __func__);
		return;
	}

	mutex_lock(&mors->lock);

	if ((morse_check_sta_associated(vif, mors_vif) ||
	     morse_check_ibss_node_joined(vif, mors_vif)) &&
	    !morse_check_chan_info_after_scan(mors, mors_vif)) {
		morse_mac_set_channel(mors,
				      mors_vif->assoc_sta_channel_info.op_chan_freq_hz,
				      mors_vif->assoc_sta_channel_info.pri_1mhz_chan_idx,
				      mors_vif->assoc_sta_channel_info.op_bw_mhz,
				      mors_vif->assoc_sta_channel_info.pri_bw_mhz, true, __func__);
	}

	mors->in_scan = false;
	ret = morse_cmd_cfg_scan(mors, false, false);
	if (ret)
		MORSE_ERR(mors, "%s: morse_cmd_cfg_scan failed %d", __func__, ret);

	morse_survey_update_channel_usage(mors);

	mutex_unlock(&mors->lock);
}

static void morse_config_uapsd(struct morse *mors, struct morse_queue_params *mqp)
{
	if (!mors || !mqp || !mqp->uapsd)
		return;

	mors->uapsd_per_ac |= BIT(mqp->aci);
}

static int
morse_mac_ops_conf_tx(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
#if KERNEL_VERSION(6, 0, 0) <= MAC80211_VERSION_CODE
		      unsigned int link_id,
#endif
		      u16 queue, const struct ieee80211_tx_queue_params *params)
{
	struct morse *mors = hw->priv;
	struct morse_queue_params mqp;

	int ret;

	mutex_lock(&mors->lock);
	mqp.aci = map_mac80211q_2_morse_aci(queue);
	mqp.aifs = params->aifs;
	mqp.cw_max = params->cw_max;
	mqp.cw_min = params->cw_min;
	/* For mesh/IBSS interface, cfg80211 blocks configuring supplicant values and mac80211
	 * configures the TXOP values corresponding to the regular Wi-Fi values. Overwrite
	 * with S1G specific TXOP value of 15008 usecs, as per IEEE-802.11-2020 Table 9-155.
	 * FW needs TXOP in units of usecs. mac80211 sends in units of 32 usecs.
	 */
	if (vif->type == NL80211_IFTYPE_ADHOC || ieee80211_vif_is_mesh(vif))
		mqp.txop = S1G_WMM_DEFAULT_TXOP_USECS;
	else
		mqp.txop = params->txop << 5;
	mqp.uapsd = params->uapsd;

	MORSE_DBG(mors, "%s queue:%d txop:%d cw_min:%d cw_max:%d aifs:%d\n",
		  __func__, mqp.aci, mqp.txop, mqp.cw_min, mqp.cw_max, mqp.aifs);

	ret = morse_cmd_cfg_qos(mors, &mqp);

	if (ret)
		MORSE_ERR(mors, "%s: morse_cmd_cfg_qos failed %d", __func__, ret);

	morse_config_uapsd(mors, &mqp);

	mutex_unlock(&mors->lock);

	return ret;
}

/**
 * Update values derived from the AID bitmap.
 * This function should be called on an AP every time the AID bitmap is updated
 */
static inline void morse_aid_bitmap_update(struct morse_ap *mors_ap)
{
	u16 largest_aid = find_last_bit(mors_ap->aid_bitmap, MORSE_AP_AID_BITMAP_SIZE);

	if (largest_aid == MORSE_AP_AID_BITMAP_SIZE)
		largest_aid = 0;

	mors_ap->largest_aid = largest_aid;
}

/*
 * This function updates remote peer capabilities using the custom config
 * based on the assumption that all nodes in the IBSS network have similar capabilities.
 */
static
void morse_mac_update_ibss_node_capabilities_using_defaults(struct ieee80211_hw *hw,
							    struct ieee80211_vif *vif,
							    struct ieee80211_sta *sta)
{
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;
	struct morse_sta *mors_sta = (struct morse_sta *)sta->drv_priv;
	struct ieee80211_sta_ht_cap *ht_cap = morse_mac_sta_ht_cap(sta);
	struct ieee80211_sta_vht_cap *vht_cap = morse_mac_sta_vht_cap(sta);

	rcu_read_lock();

	/* defaults - vif is IBSS creator or if no entry found in cssid list
	 * Update the STA capabilities using mors_vif->custom_configs
	 */
	mors_sta->ampdu_supported = mors_vif->custom_configs->enable_ampdu;
	mors_sta->trav_pilot_support = mors_vif->custom_configs->enable_trav_pilot;
	mors_sta->max_bw_mhz = mors_vif->custom_configs->channel_info.op_bw_mhz;

	/* mmrc is enabling all rates (MCS0-9 & 10) by default, assign rates to defaults */
	ht_cap->mcs.rx_mask[0] = (mcs_mask & MCS_RATE_MASK_0_TO_7);
	ht_cap->mcs.rx_mask[1] = (mcs_mask >> MCS_RATE_MASK_8_TO_9_OFFSET);

	/* Update VHT & SGI Capabilities */
	if (mors_vif->custom_configs->enable_sgi_rc) {
		ht_cap->cap |= (IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40);

		if (mors_vif->custom_configs->channel_info.op_bw_mhz >= 4) {
			vht_cap->vht_supported = true;
			vht_cap->cap |= IEEE80211_VHT_CAP_SHORT_GI_80;

			if (mors_vif->custom_configs->channel_info.op_bw_mhz > 4) {
				vht_cap->cap |= IEEE80211_VHT_CAP_SHORT_GI_160;
				vht_cap->cap |= IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ;
			}
		}
	}

	rcu_read_unlock();
}

/*
 * This function updates remote peer capabilities using the beacon/probe response,
 * based on the assumption that all nodes in the network have similar capabilities.
 * The  main reason for this assumption is that every node in the IBSS network will
 * not have capabilities information of other nodes joining the network.
 * In IBSS network, nodes(ieee80211_sta/morse_sta) are created in mac80211 upon
 * reception of data frames with bssid same as this(receiving) node joined.
 */
static
void morse_mac_update_ibss_node_capabilities(struct ieee80211_hw *hw,
					     struct ieee80211_vif *vif,
					     struct ieee80211_sta *sta,
					     struct ieee80211_s1g_cap *s1g_caps,
					     struct morse_channel_info *info)
{
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;
	struct morse_sta *mors_sta = (struct morse_sta *)sta->drv_priv;
	struct ieee80211_sta_ht_cap *ht_cap = morse_mac_sta_ht_cap(sta);
	struct ieee80211_sta_vht_cap *vht_cap = morse_mac_sta_vht_cap(sta);
	bool sgi_enabled = 0;
	int sta_max_bw = 0;

	sgi_enabled = (s1g_caps->capab_info[0] & (S1G_CAP0_SGI_1MHZ | S1G_CAP0_SGI_2MHZ
						  | S1G_CAP0_SGI_4MHZ | S1G_CAP0_SGI_8MHZ));
	sta_max_bw = (s1g_caps->capab_info[0] & S1G_CAP0_SUPP_CH_WIDTH);

	rcu_read_lock();

	if (s1g_caps->capab_info[7] & S1G_CAP7_1MHZ_CTL_RESPONSE_PREAMBLE)
		mors_vif->ctrl_resp_in_1mhz_en = true;

	/* AMPDU params info */
	if (s1g_caps->capab_info[5] & IEEE80211AH_AMPDU_SUPPORTED)
		mors_sta->ampdu_supported = true;
	else
		mors_sta->ampdu_supported = false;

	ht_cap->ampdu_factor = (s1g_caps->capab_info[3] >> 3) & 0x3;
	ht_cap->ampdu_density = (s1g_caps->capab_info[3] >> 5) & 0x7;

	mors_sta->trav_pilot_support = S1G_CAP2_GET_TRAV_PILOT(s1g_caps->capab_info[2]);

	mors_sta->max_bw_mhz = (sta_max_bw == S1G_CAP0_SUPP_16MHZ) ? 16 :
	    (sta_max_bw == S1G_CAP0_SUPP_8MHZ) ? 8 : (sta_max_bw == S1G_CAP0_SUPP_4MHZ) ? 4 : 2;
	mors_vif->bss_color = S1G_CAP8_GET_COLOR(s1g_caps->capab_info[8]);

	/* mmrc is enabling all rates (MCS0-9 & 10) by default, assign rates to defaults */
	ht_cap->mcs.rx_mask[0] = (mcs_mask & MCS_RATE_MASK_0_TO_7);
	ht_cap->mcs.rx_mask[1] = (mcs_mask >> MCS_RATE_MASK_8_TO_9_OFFSET);

	if (sgi_enabled && mors_vif->custom_configs->enable_sgi_rc)
		ht_cap->cap |= (IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40);

	if (s1g_caps->capab_info[0] & S1G_CAP0_SGI_4MHZ)
		ht_cap->cap |= IEEE80211_HT_CAP_SUP_WIDTH_20_40;

	if (info->op_bw_mhz >= 4) {
		if (sgi_enabled && mors_vif->custom_configs->enable_sgi_rc) {
			vht_cap->cap |= IEEE80211_VHT_CAP_SHORT_GI_80;

			if (info->op_bw_mhz > 4)
				vht_cap->cap |= IEEE80211_VHT_CAP_SHORT_GI_160;
		}
		vht_cap->vht_supported = true;

		if (s1g_caps->capab_info[0] & S1G_CAP0_SGI_8MHZ)
			vht_cap->cap |= IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ;
	}

	rcu_read_unlock();
}

/* API to process the bandwidth change notification from mac80211 */
static void morse_mac_ops_sta_rc_update(struct ieee80211_hw *hw,
					struct ieee80211_vif *vif,
#if KERNEL_VERSION(6, 13, 0) > MAC80211_VERSION_CODE
					struct ieee80211_sta *sta,
#else
					struct ieee80211_link_sta *link_sta,
#endif
					u32 changed)
{
	struct morse *mors;
#ifdef CONFIG_MORSE_RC
	enum ieee80211_sta_state old_state;
	enum ieee80211_sta_state new_state;
#endif
#if KERNEL_VERSION(6, 13, 0) <= MAC80211_VERSION_CODE
	struct ieee80211_sta *sta = link_sta->sta;
#endif

	if (!hw || !vif || !sta)
		return;

	mors = hw->priv;

	MORSE_DBG(mors, "Rate control config updated (changed %u, peer address %pM)\n",
		  changed, sta->addr);

	if (!(changed & IEEE80211_RC_BW_CHANGED))
		return;

#ifdef CONFIG_MORSE_RC
	/* Simulate the disconnection and connection to reinitialize the sta in mmrc with new BW */
	old_state = IEEE80211_STA_ASSOC;
	new_state = IEEE80211_STA_NOTEXIST;

	MORSE_DBG(mors, "%s Remove sta, old_state=%d, new_state=%d, changed=0x%x, bw_changed=%d\n",
		  __func__, old_state, new_state, changed, (changed & IEEE80211_RC_BW_CHANGED));
	mutex_lock(&mors->lock);

	morse_rc_sta_state_check(mors, vif, sta, old_state, new_state);

	old_state = IEEE80211_STA_NOTEXIST;
	new_state = IEEE80211_STA_ASSOC;

	MORSE_DBG(mors, "%s Add sta, old_state=%d, new_state=%d\n",
		  __func__, old_state, new_state);

	morse_rc_sta_state_check(mors, vif, sta, old_state, new_state);

	mutex_unlock(&mors->lock);
#endif
}

static int
morse_mac_ops_sta_state(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
			struct ieee80211_sta *sta,
			enum ieee80211_sta_state old_state, enum ieee80211_sta_state new_state)
{
	struct morse *mors;
	struct morse_vif *mors_vif;
	struct morse_sta *mors_sta;
	u16 aid;
	int ret = 0;

	if (!hw || !vif || !sta)
		return -EINVAL;

	mors = hw->priv;
	mors_vif = (struct morse_vif *)vif->drv_priv;
	mors_sta = (struct morse_sta *)sta->drv_priv;

	/* Ignore both NOTEXIST to NONE and NONE to NOTEXIST */
	if ((old_state == IEEE80211_STA_NOTEXIST && new_state == IEEE80211_STA_NONE) ||
	    (old_state == IEEE80211_STA_NONE && new_state == IEEE80211_STA_NOTEXIST))
		return 0;

	/* SW-5033: in IBSS mode, ignore any state transition originated by the network creator.
	 * Note: mac80211 will create two entries/peers/sta's for the network generator,
	 * one of them using the BSSID and the other using the actual peer MAC address. We
	 * can safely ignore the BSSID entry as it does not present an actual peer (and it
	 * will not have an IP anyway)
	 */
	if (vif->type == NL80211_IFTYPE_ADHOC &&
	    ether_addr_equal_unaligned(sta->addr, vif->bss_conf.bssid)) {
		return 0;
	}

	mutex_lock(&mors->lock);

	if ((old_state > IEEE80211_STA_NONE &&
	     new_state <= IEEE80211_STA_NONE) &&
	    mors_sta->assoc_req_count > 1) {
		mors_sta->tx_ps_filter_en = false;
		morse_mac_save_sta_backup(mors, mors_vif, mors_sta);
		morse_vendor_reset_sta_transient_info(vif, mors_sta);
	}

	if (new_state > IEEE80211_STA_ASSOC)
		mors_sta->assoc_req_count = 0;

	/* Always use WME (or QoS) for 802.11ah */
	rcu_read_lock();
	if (sta) {
		sta->wme = true;
		morse_mac_sta_set_ht_support(sta, true);
	}
	rcu_read_unlock();

	if (vif->type == NL80211_IFTYPE_STATION)
		aid = morse_mac_sta_aid(vif);
	else if (vif->type == NL80211_IFTYPE_ADHOC)
		/* SW-4741: in IBSS mode, AID is always zero, and we can not use
		 * it as a unique ID. As a WAR, we overload the AID with the MAC
		 * address (lowest two octets) assuming those will always be unique
		 *
		 * TODO: make sure the AID passed to FW is never used as an index,
		 * but only used for lookup purposes (i.e., RAW will not work)
		 */
		aid = (((u16)sta->addr[4] << 8) | ((u16)sta->addr[5])) & 0x7FFF;
	else
		aid = sta->aid;

	if (vif->type == NL80211_IFTYPE_STATION && new_state > old_state &&
	    new_state == IEEE80211_STA_ASSOC)
		ret = morse_cmd_set_bss_color(mors, mors_vif, mors_vif->bss_color);

	if (!ret)
		ret = morse_cmd_sta_state(mors, mors_vif, aid, sta, new_state);

	if (old_state < new_state && new_state == IEEE80211_STA_ASSOC)
		morse_mac_restore_sta_backup(mors, mors_vif, mors_sta, sta->addr);

	if (new_state == IEEE80211_STA_ASSOC) {
		int i;

		for (i = 0; i < IEEE80211_NUM_TIDS; i++) {
			mors_sta->tid_start_tx[i] = false;
			mors_sta->tid_tx[i] = false;
		}
		/* Fetch beacon/probe resp using bssid for S1G caps and update the
		 * STA subbands (HT/VHT) Capabilities
		 */
		if (vif->type == NL80211_IFTYPE_ADHOC) {
			struct ieee80211_s1g_cap s1g_caps;
			u8 bssid[ETH_ALEN];

			memcpy(bssid, vif->bss_conf.bssid, ETH_ALEN);

			/* Apply sta capabilities using beacon/probe response */
			if (morse_dot11ah_find_s1g_caps_for_bssid(bssid, &s1g_caps)) {
				struct morse_channel_info info;

				MORSE_INFO(mors, "Update RC of associated peer %pM using beacon\n",
					   sta->addr);
				morse_mac_find_channel_info_for_bssid(bssid, &info);
				morse_mac_update_ibss_node_capabilities(hw, vif, sta, &s1g_caps,
									&info);
			} else {
				MORSE_INFO(mors,
					   "Set defaults and update RC of associated peer %pM\n",
					   sta->addr);
				morse_mac_update_ibss_node_capabilities_using_defaults(hw, vif,
										       sta);
			}
		}
	}

#ifdef CONFIG_MORSE_RC
	morse_rc_sta_state_check(mors, vif, sta, old_state, new_state);
#endif

	ether_addr_copy(mors_sta->addr, sta->addr);
	mors_sta->state = new_state;
	mors_sta->vif = vif;

	/* As per the mac80211 documentation, this callback must not fail
	 * for down transitions of state.
	 */
	if (new_state < old_state)
		ret = 0;

	morse_bss_stats_update_sta_state(mors, vif, sta, old_state, new_state);

	if (new_state > old_state && new_state == IEEE80211_STA_ASSOC) {
		MORSE_INFO(mors, "Station associated %pM, aid=%d\n", sta->addr, sta->aid);

		if (vif->type == NL80211_IFTYPE_AP || vif->type == NL80211_IFTYPE_MESH_POINT) {
			if (test_and_set_bit(aid, mors_vif->ap->aid_bitmap)) {
				MORSE_WARN(mors, "Station associated with duplicate AID %d\n",
					   aid);
			} else {
				mors_vif->ap->num_stas++;
				morse_pre_assoc_peer_delete(mors, sta->addr);
			}

			morse_aid_bitmap_update(mors_vif->ap);
		}

		if (vif->type == NL80211_IFTYPE_STATION) {
			memcpy(&mors_vif->assoc_sta_channel_info,
			       &mors->custom_configs.channel_info,
			       sizeof(mors->custom_configs.channel_info));

			/* Reset channel info */
			memset(&mors_vif->ecsa_channel_info, 0,
			       sizeof(mors_vif->ecsa_channel_info));

			mors_vif->ecsa_chan_configured = false;

			if (morse_cmd_configure_page_slicing(mors_vif,
					mors_vif->bss_vendor_info.page_slicing_exclusive_support))
				mors_vif->bss_vendor_info.page_slicing_exclusive_support = false;
		}
	}

	if (new_state < old_state && new_state == IEEE80211_STA_NONE) {
		MORSE_INFO(mors, "Station disassociated %pM, aid=%d\n", sta->addr, sta->aid);

		/* Reset channel info */
		if (vif->type == NL80211_IFTYPE_STATION) {
			memset(&mors_vif->ecsa_channel_info, 0,
			       sizeof(mors_vif->ecsa_channel_info));
			mors_vif->ecsa_chan_configured = false;
		}

		if (vif->type == NL80211_IFTYPE_AP || vif->type == NL80211_IFTYPE_MESH_POINT) {
			if (test_and_clear_bit(aid, mors_vif->ap->aid_bitmap)) {
				mors_vif->ap->num_stas--;
			} else {
				MORSE_WARN(mors,
					   "Non-existent station disassociated with AID %d\n",
					   aid);
			}

			morse_aid_bitmap_update(mors_vif->ap);

			/* delete mesh peer from CSSID list */
			if (ieee80211_vif_is_mesh(vif) && mors_vif->mesh->mesh_beaconless_mode)
				morse_dot11ah_del_mesh_peer(sta->addr);
;
			/* Update the mulitcast tx rate */
			if (mors_vif->enable_multicast_rate_control)
				morse_rc_vif_update_mcast_rate(mors, mors_vif);
		}
	}

	if (vif->type == NL80211_IFTYPE_STATION &&
	    new_state > old_state &&
	    new_state == IEEE80211_STA_ASSOC) {
		bool enable_non_tim_mode = mors_sta->non_tim_mode_status == NON_TIM_MODE_ENABLED;

		if (enable_dhcpc_offload && (morse_cmd_dhcpc_enable(mors, mors_vif->id) < 0))
			MORSE_WARN(mors, "Failed to enable in-chip DHCP client\n");

		if (morse_cmd_config_non_tim_mode(mors, enable_non_tim_mode, mors_vif->id) < 0)
			MORSE_ERR(mors, "Failed to configure non-TIM mode\n");

		if (mors_vif->custom_configs->listen_interval_ovr &&
		    morse_cmd_enable_li_sleep(mors,
				 mors_vif->custom_configs->listen_interval,
				 mors_vif->id) < 0)
			MORSE_WARN(mors, "Failed to enable listen interval sleep\n");
	}

	mutex_unlock(&mors->lock);

	if (morse_mac_is_iface_infra_bss_type(vif)) {
		if (new_state > old_state && new_state >= IEEE80211_STA_ASSOC)
			morse_twt_handle_event(mors_vif, sta->addr);

		/* Install agreements after handling events in case there is an accept event. */
		if (new_state > old_state && new_state == IEEE80211_STA_AUTHORIZED) {
			morse_twt_process_pending_cmds(mors, mors_vif);
			morse_vendor_update_ack_timeout_on_assoc(mors, vif, sta);
		}

		/* Since agreements are negotiated in the (re)assoc frames, remove sta data if we
		 * become disassociated.
		 */
		if (old_state >= IEEE80211_STA_ASSOC && new_state < IEEE80211_STA_ASSOC)
			morse_twt_sta_remove_addr(mors, mors_vif, sta->addr);

		/* If a STA disconnects remove pending TWT events. In the case where an association
		 * attempt fails, mac80211 on the next attempt will set the STA state to
		 * IEEE80211_STA_NONE before immediately setting it back to IEEE80211_STA_ASSOC. In
		 * this case we don't purge events from the queue.
		 */
		if (new_state < old_state &&
		    new_state == IEEE80211_STA_NONE && mors_sta->assoc_req_count <= 1)
			morse_twt_event_queue_purge(mors, mors_vif, sta->addr);

		/* If a STA is added or removed from the AP, update the RAW assignments. */
		if (vif->type == NL80211_IFTYPE_AP) {
			if ((new_state > old_state && new_state == IEEE80211_STA_ASSOC) ||
			    (new_state < old_state && new_state == IEEE80211_STA_NONE)) {
				morse_raw_trigger_update(mors_vif, true);
			}
		}
	}

	return ret;
}

static int
morse_mac_ops_ampdu_action(struct ieee80211_hw *hw,
			   struct ieee80211_vif *vif, struct ieee80211_ampdu_params *params)
{
	struct morse *mors = hw->priv;
	struct ieee80211_sta *sta = params->sta;
	struct morse_sta *mors_sta = (struct morse_sta *)sta->drv_priv;
	enum ieee80211_ampdu_mlme_action action = params->action;
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);
	u16 tid = params->tid;
	bool amsdu_supported = params->amsdu;
	u16 buf_size = min_t(u16, params->buf_size,
			     DOT11AH_BA_MAX_MPDU_PER_AMPDU);
	int ret = 0;

	if (!mors->custom_configs.enable_ampdu) {
		MORSE_DBG(mors, "%s %pM.%d Denying AMPDU because not enabled\n",
			  __func__, mors_sta->addr, tid);
		return -EINVAL;
	}

	if (!mors_sta->ampdu_supported) {
		MORSE_DBG(mors, "%s %pM.%d Denying AMPDU because STA doesn't support it\n",
			  __func__, mors_sta->addr, tid);
		return -EINVAL;
	}

#if defined(MORSE_MAC80211_S1G_FEATURE_NDP_BLOCKACK)
	if (!params->ndp) {
		MORSE_WARN(mors, "%s %pM.%d requesting non-NDP block ack session\n",
				__func__, sta->addr, tid);
		return -EINVAL;
	}
#endif

	mutex_lock(&mors->lock);
	switch (action) {
	case IEEE80211_AMPDU_RX_START:
		MORSE_INFO(mors, "%s %pM.%d A-MPDU RX start\n", __func__, mors_sta->addr, tid);
		if (mors_vif->enable_pv1 && mors_sta->pv1_frame_support)
			morse_cmd_pv1_set_rx_ampdu_state(mors_vif, sta->addr, tid, buf_size, true);
		break;
	case IEEE80211_AMPDU_RX_STOP:
		MORSE_INFO(mors, "%s %pM.%d A-MPDU RX stop\n", __func__, mors_sta->addr, tid);
		if (mors_vif->enable_pv1 && mors_sta->pv1_frame_support)
			morse_cmd_pv1_set_rx_ampdu_state(mors_vif, sta->addr, tid, buf_size, false);
		break;
	case IEEE80211_AMPDU_TX_START:
		MORSE_INFO(mors, "%s %pM.%d A-MPDU TX start\n", __func__, mors_sta->addr, tid);
		ieee80211_start_tx_ba_cb_irqsafe(vif, sta->addr, tid);
		break;
	case IEEE80211_AMPDU_TX_STOP_CONT:
	case IEEE80211_AMPDU_TX_STOP_FLUSH:
	case IEEE80211_AMPDU_TX_STOP_FLUSH_CONT:
		MORSE_INFO(mors, "%s %pM.%d A-MPDU TX flush\n", __func__, mors_sta->addr, tid);
		mors_sta->tid_start_tx[tid] = false;
		mors_sta->tid_tx[tid] = false;
		mors_sta->tid_params[tid] = 0;
		ieee80211_stop_tx_ba_cb_irqsafe(vif, sta->addr, tid);
		break;
	case IEEE80211_AMPDU_TX_OPERATIONAL:
		MORSE_INFO(mors, "%s %pM.%d A-MPDU TX oper\n", __func__, mors_sta->addr, tid);
		mors_sta->tid_tx[tid] = true;
		/* Max reorder buffer is stored as little-Endian and 0-indexed */
		if (buf_size == 0) {
			MORSE_ERR(mors, "%s %pM.%d A-MPDU Invalid buf size\n",
				  __func__, mors_sta->addr, tid);
			break;
		}
		mors_sta->tid_params[tid] =
		    BMSET(buf_size - 1, TX_INFO_TID_PARAMS_MAX_REORDER_BUF) |
		    BMSET(1, TX_INFO_TID_PARAMS_AMPDU_ENABLED) |
		    BMSET(amsdu_supported, TX_INFO_TID_PARAMS_AMSDU_SUPPORTED);
		break;
	default:
		MORSE_ERR(mors,
			  "%s %pM.%d Invalid command %d, ignoring\n",
			  __func__, mors_sta->addr, tid, action);
	}

	mutex_unlock(&mors->lock);
	return ret;
}

static int
morse_mac_ops_set_key(struct ieee80211_hw *hw,
		      enum set_key_cmd cmd,
		      struct ieee80211_vif *vif,
		      struct ieee80211_sta *sta, struct ieee80211_key_conf *key)
{
	u16 aid;
	int ret = -EOPNOTSUPP;
	struct morse *mors = hw->priv;
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;
	struct morse_sta *mors_sta;

	mutex_lock(&mors->lock);

	if (vif->type == NL80211_IFTYPE_STATION) {
		aid = morse_mac_sta_aid(vif);
	} else if (vif->type == NL80211_IFTYPE_ADHOC) {
		/* SW-4741: in IBSS mode, AID is always zero, and we can not use
		 * it as a unique ID. As a WAR, we overload the AID with the MAC
		 * address (lowest two octets) assuming those will always be unique
		 *
		 * TODO: make sure the AID passed to FW is never used as an index,
		 * but only used for lookup purposes (i.e., RAW will not work)
		 */
		if (sta)
			aid = (((u16)sta->addr[4] << 8) | ((u16)sta->addr[5])) & 0x7FFF;
		else
			aid = 0;
	} else if (sta) {
		aid = sta->aid;
	} else {
		/* Is a group key - AID is unused */
		MORSE_WARN_ON(FEATURE_ID_DEFAULT, (key->flags & IEEE80211_KEY_FLAG_PAIRWISE));
		aid = 0;
	}

	switch (cmd) {
	case SET_KEY: {
		enum morse_cmd_key_cipher cipher;
		enum morse_cmd_aes_key_len length;

		switch (key->cipher) {
		case WLAN_CIPHER_SUITE_CCMP:
		case WLAN_CIPHER_SUITE_CCMP_256:
			cipher = MORSE_CMD_KEY_CIPHER_AES_CCM;
			break;
		case WLAN_CIPHER_SUITE_GCMP:
		case WLAN_CIPHER_SUITE_GCMP_256:
			cipher = MORSE_CMD_KEY_CIPHER_AES_GCM;
			break;
		case WLAN_CIPHER_SUITE_AES_CMAC:
			/* DEAD CODE, to latter be enabled */
			cipher = MORSE_CMD_KEY_CIPHER_AES_CMAC;
			/* CMAC is currently not supported */
			ret = -EOPNOTSUPP;
			goto exit;
		default:
			/* Cipher suite currently not supported */
			ret = -EOPNOTSUPP;
			goto exit;
		}

		switch (key->keylen) {
		case 16:
			length = MORSE_CMD_AES_KEY_LEN_LENGTH_128;
			break;
		case 32:
			length = MORSE_CMD_AES_KEY_LEN_LENGTH_256;
			break;
		default:
			/* Key length not supported */
			ret = -EOPNOTSUPP;
			goto exit;
		}

		ret = morse_cmd_install_key(mors, mors_vif, aid, key, cipher, length);
		break;
	}
	case DISABLE_KEY:
		ret = morse_cmd_disable_key(mors, mors_vif, aid, key);
		if (ret) {
			/* Must return 0 */
			MORSE_WARN(mors, "Failed to remove key\n");
			ret = 0;
		}
		break;
	default:
		MORSE_WARN_ON(FEATURE_ID_DEFAULT, 1);
	}
	if (ret) {
		MORSE_DBG(mors, "%s Falling back to software crypto\n", __func__);
		ret = 1;
	} else if (sta && key->flags & IEEE80211_KEY_FLAG_PAIRWISE) {
		mors_sta = (struct morse_sta *)sta->drv_priv;
		mors_sta->last_rx_mgmt_pn = 0;
	}

exit:
	mutex_unlock(&mors->lock);
	return ret;
}

static void morse_mac_ops_rfkill_poll(struct ieee80211_hw *hw)
{
	struct morse *mors = hw->priv;
	(void)mors;
}

static void morse_mac_ops_flush(struct ieee80211_hw *hw,
				struct ieee80211_vif *vif, u32 queues, bool drop)
{
	struct morse *mors = hw->priv;
	(void)mors;
}

static u64 morse_mac_ops_get_tsf(struct ieee80211_hw *hw, struct ieee80211_vif *vif)
{
	struct morse *mors = hw->priv;
	(void)mors;
	return 0;
}

static void morse_mac_ops_set_tsf(struct ieee80211_hw *hw, struct ieee80211_vif *vif, u64 tsf)
{
	struct morse *mors = hw->priv;
	(void)mors;
}

static void morse_mac_ops_offset_tsf(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
				     s64 offset)
{
#ifdef NOT_YET
	struct morse *mors = hw->priv;
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;
	int ret;
#endif

	/* Used only in Mesh STA Mode */
	if (!ieee80211_vif_is_mesh(vif))
		return;

	/* TODO: To be reviewed as part of SW-8015.
	 * (IEEE802.11-2020: section 14.3.2.2 Neighbor offset synchronization method).
	 * MAC Timestamp difference of two beacons and TSF Timestamp difference of
	 * two beacons is not same.
	 */
#ifdef NOT_YET
	mutex_lock(&mors->lock);
	/* Configure tsf offset in firmware */
	ret = morse_cmd_cfg_offset_tsf(mors, mors_vif->id, offset);
	if (ret)
		MORSE_ERR(mors, "%s: failed ret=%d, offset=%lld\n", __func__, ret, offset);
	mutex_unlock(&mors->lock);
#endif
}

static int morse_mac_ops_tx_last_beacon(struct ieee80211_hw *hw)
{
	/* SW-4741: in IBSS mode, this should return TRUE only if this
	 * node is the one that generates beacons (for the current beacon
	 * interval). This will help host to decide if this node should
	 * reply probe requests or not. For now, as all nodes are acting
	 * as AP (sending beacons), then we can force this to TRUE
	 * TODO: decide when should we cancel beacon and return FALSE here
	 */
	return 1;
}

static int morse_mac_join_ibss(struct ieee80211_hw *hw, struct ieee80211_vif *vif)
{
	struct morse *mors = hw->priv;
	struct cfg80211_chan_def *chandef = MORSE_BSS_CONF_CHAN_DEF(vif);
	const struct morse_dot11ah_channel *chan_s1g =
	    morse_dot11ah_channel_chandef_to_s1g(chandef);
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;
	u8 bssid[ETH_ALEN], fc_bss_bw_subfield = 0;
	bool ibss_creator = morse_mac_is_ibss_creator(vif);

	int op_bw_mhz =
	    (chan_s1g && (chan_s1g->ch.flags & IEEE80211_CHAN_8MHZ)) ? 8 :
	    (chan_s1g && (chan_s1g->ch.flags & IEEE80211_CHAN_4MHZ)) ? 4 :
	    (chan_s1g && (chan_s1g->ch.flags & IEEE80211_CHAN_2MHZ)) ? 2 :
	    (chan_s1g && (chan_s1g->ch.flags & IEEE80211_CHAN_1MHZ)) ? 1 : -1;

	MORSE_INFO(mors, "Joined IBSS:\n"
		   " * SSID           : %s\n"
		   " * BSSID          : %pM\n"
		   " * Address        : %pM\n"
		   " * 5G Channel     : Ch %u, Freq %uKHz\n"
		   " * S1G Channel    : Ch %d, Freq %dKHz, Width %dMHz\n"
		   " * Regulatory     : %s\n"
		   " * IBSS Creator?  : %s\n",
#if KERNEL_VERSION(6, 0, 0) > MAC80211_VERSION_CODE
		   vif->bss_conf.ssid,
#else
		   vif->cfg.ssid,
#endif
		   vif->bss_conf.bssid,
		   vif->addr,
		   chandef->chan->hw_value,
		   chandef->chan->center_freq,
		   chan_s1g ? chan_s1g->ch.hw_value : -1,
		   chan_s1g ? ieee80211_channel_to_khz(&chan_s1g->ch) : -1,
		   op_bw_mhz,
		   morse_dot11ah_get_region_str(), ibss_creator ? "Yes" : "No");

	/* Update channel only if it is not ibss creator */
	if (!ibss_creator) {
		u32 changed = 0;

		/* mac80211 updating bssid after configuring the channel(morse_mac_ops_config) to
		 * driver. we have now bssid updated in vif->bss_conf,
		 * update(mors->custom_configs.channel_info) operating bw, prim chan bw and idx.
		 * This is required for selecting right sub band in transmission of mgmt and data
		 * packets.
		 */
		changed |= IEEE80211_CONF_CHANGE_CHANNEL;
		morse_mac_ops_config(hw, changed);
	}

	memcpy(bssid, vif->bss_conf.bssid, ETH_ALEN);

	mutex_lock(&mors->lock);
	mors_vif->is_ibss_node_joined = true;
	if (morse_dot11ah_find_bss_bw(bssid, &fc_bss_bw_subfield) &&
	    MORSE_IS_FC_BSS_BW_SUBFIELD_VALID(fc_bss_bw_subfield)) {
		mors_vif->custom_configs->channel_info.pri_bw_mhz =
		    s1g_fc_bss_bw_lookup_min[fc_bss_bw_subfield];
	} else {
		struct morse_channel_info info;

		if (morse_mac_find_channel_info_for_bssid(bssid, &info))
			mors_vif->custom_configs->channel_info.pri_bw_mhz = info.pri_bw_mhz;
	}
	memcpy(&mors_vif->assoc_sta_channel_info, &mors->custom_configs.channel_info,
	       sizeof(mors->custom_configs.channel_info));

	mutex_unlock(&mors->lock);
	return 0;
}

static void morse_mac_leave_ibss(struct ieee80211_hw *hw, struct ieee80211_vif *vif)
{
	struct morse *mors = hw->priv;
	struct morse_vif *mors_vif = (struct morse_vif *)vif->drv_priv;

	MORSE_INFO(mors, "Leaving IBSS:bssid=%pM\n", vif->bss_conf.bssid);

	mutex_lock(&mors->lock);
	mors_vif->is_ibss_node_joined = false;
	mutex_unlock(&mors->lock);
}

static int morse_mac_set_frag_threshold(struct ieee80211_hw *hw, u32 value)
{
	int ret = -EINVAL;
	struct morse *mors = hw->priv;

	mutex_lock(&mors->lock);

	ret = morse_cmd_set_frag_threshold(mors, value);

	mutex_unlock(&mors->lock);

	return ret;
}

static int morse_mac_set_rts_threshold(struct ieee80211_hw *hw, u32 value)
{
	/* When Minstrel is not used, Linux checks if .set_rts_threshold is registered.
	 * MMRC follows Minstrel to apply RTS on retry rates so does not use this function.
	 * So create this function to pass the check and may apply different algorithm later.
	 */
#ifdef CONFIG_MORSE_RC
	struct morse *mors = hw->priv;

	mors->rts_threshold = value;
#endif
	return 0;
}

static void morse_mac_reconfig_complete(struct ieee80211_hw *hw,
				enum ieee80211_reconfig_type reconfig_type)
{
	struct morse *mors = hw->priv;
	struct ieee80211_vif *sta_vif = morse_get_sta_vif(mors);
	struct ieee80211_vif *mesh_vif = morse_get_mesh_vif(mors);

	if (reconfig_type != IEEE80211_RECONFIG_TYPE_RESTART)
		return;

	if (mesh_vif) {
		int ret;
		struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(mesh_vif);

		ret = morse_restore_mesh_config(mors_vif);
		MORSE_MESH_DBG(mors, "morse_restore_mesh_config %s:%d\n",
			      ret ? "fail" : "done", ret);
	}

	/* Triggers a re-association after a watchdog reset. Without this, the Packet Numbers
	 * in a WPA3 network will no longer be synchronised between the AP and STA following
	 * a watchdog reset and data transmission will not continue.
	 */
	if (sta_vif)
		ieee80211_connection_loss(sta_vif);
}

#ifdef CONFIG_MORSE_RC
static void
morse_sta_tx_rate_stats(struct ieee80211_hw *hw, struct ieee80211_vif *vif,
			struct ieee80211_sta *sta, struct station_info *sinfo)
{
	struct morse_sta *msta = (struct morse_sta *)sta->drv_priv;
	struct morse *mors = hw->priv;
	const struct mmrc_table *tb = msta->rc.tb;
	struct mmrc_rate rate;

	if (!tb || tb->best_tp.rate == MMRC_MCS_UNUSED) {
		sinfo->filled &= ~BIT_ULL(NL80211_STA_INFO_TX_BITRATE);
		return;
	}

	rate = tb->best_tp;
	sinfo->txrate.mcs = rate.rate;
	sinfo->txrate.nss = NSS_IDX_TO_NSS(rate.ss);
	sinfo->txrate.flags = (RATE_INFO_FLAGS_VHT_MCS);
	switch (rate.bw) {
	case MMRC_BW_1MHZ:
		sinfo->txrate.bw = RATE_INFO_BW_20;
		break;
	case MMRC_BW_2MHZ:
		sinfo->txrate.bw = RATE_INFO_BW_40;
		break;
	case MMRC_BW_4MHZ:
		sinfo->txrate.bw = RATE_INFO_BW_80;
		break;
	case MMRC_BW_8MHZ:
		sinfo->txrate.bw = RATE_INFO_BW_160;
		break;
	default:
		break;
	}

	if (rate.guard == MMRC_GUARD_SHORT)
		sinfo->txrate.flags |= (RATE_INFO_FLAGS_SHORT_GI);

	morse_dbg(FEATURE_ID_RATECONTROL, mors, "mcs: %d, bw: %d, flag: 0x%x\n",
			rate.rate, rate.bw, sinfo->txrate.flags);
	sinfo->filled |= BIT_ULL(NL80211_STA_INFO_TX_BITRATE);
}

static u32 morse_get_expected_throughput(struct ieee80211_hw *hw, struct ieee80211_sta *sta)
{
	struct morse_sta *msta = (struct morse_sta *)sta->drv_priv;
	struct morse *mors = hw->priv;
	const struct mmrc_table *tb = msta->rc.tb;
	struct mmrc_rate rate;
	u32 tput;

	if (!tb || tb->best_tp.rate == MMRC_MCS_UNUSED)
		return 0;

	rate = tb->best_tp;
	tput = BPS_TO_KBPS(mmrc_calculate_theoretical_throughput(rate));
	morse_dbg(FEATURE_ID_RATECONTROL, mors,
		"%s: Throughput: MCS: %d, BW: %d, GI: %d -> %u\n",
		__func__, rate.rate, 1 << rate.bw, rate.guard, tput);

	return tput;
}
#endif

static struct ieee80211_ops mors_ops = {
	.tx = morse_mac_ops_tx,
	.start = morse_mac_ops_start,
	.stop = morse_mac_ops_stop,
	.add_interface = morse_mac_ops_add_interface,
	.remove_interface = morse_mac_ops_remove_interface,
	.config = morse_mac_ops_config,
	.get_txpower = morse_mac_ops_get_txpower,
	.bss_info_changed = morse_mac_ops_bss_info_changed,
	.prepare_multicast = morse_mac_ops_prepare_multicast,
	.configure_filter = morse_mac_ops_configure_filter,
	.sw_scan_start = morse_mac_ops_sw_scan_start,
	.sw_scan_complete = morse_mac_ops_sw_scan_complete,
	.conf_tx = morse_mac_ops_conf_tx,
	.sta_state = morse_mac_ops_sta_state,
	.ampdu_action = morse_mac_ops_ampdu_action,
	.rfkill_poll = morse_mac_ops_rfkill_poll,
	.flush = morse_mac_ops_flush,
	.get_tsf = morse_mac_ops_get_tsf,
	.set_tsf = morse_mac_ops_set_tsf,
	.offset_tsf = morse_mac_ops_offset_tsf,
	.get_survey = morse_mac_ops_get_survey,
	.set_key = morse_mac_ops_set_key,
	.tx_last_beacon = morse_mac_ops_tx_last_beacon,
	.join_ibss = morse_mac_join_ibss,
	.leave_ibss = morse_mac_leave_ibss,
#if KERNEL_VERSION(6, 13, 0) > MAC80211_VERSION_CODE
	.sta_rc_update = morse_mac_ops_sta_rc_update,
#else
	.link_sta_rc_update = morse_mac_ops_sta_rc_update,
#endif
	.set_frag_threshold = morse_mac_set_frag_threshold,
	.set_rts_threshold = morse_mac_set_rts_threshold,
	.reconfig_complete = morse_mac_reconfig_complete,
	.hw_scan = morse_ops_hw_scan,
	.cancel_hw_scan = morse_ops_cancel_hw_scan,
	.sched_scan_start = morse_ops_sched_scan_start,
	.sched_scan_stop = morse_ops_sched_scan_stop,
#ifdef CONFIG_MORSE_RC
	.sta_statistics = morse_sta_tx_rate_stats,
	.get_expected_throughput = morse_get_expected_throughput,
#endif
#if KERNEL_VERSION(6, 9, 0) <= MAC80211_VERSION_CODE
	.add_chanctx = ieee80211_emulate_add_chanctx,
	.remove_chanctx = ieee80211_emulate_remove_chanctx,
	.change_chanctx = ieee80211_emulate_change_chanctx,
	.switch_vif_chanctx = ieee80211_emulate_switch_vif_chanctx,
#endif

};

int morse_mac_send_vendor_wake_action_frame(struct morse *mors, const u8 *dest_addr,
					    const u8 *payload, int payload_len)
{
	struct sk_buff *skb;
	struct ieee80211_tx_info *info;
	struct ieee80211_mgmt *action;
	struct ieee80211_sta *sta;
	struct ieee80211_vif *vif = morse_get_vif(mors);
	const u8 subcategory = MORSE_VENDOR_SPECIFIC_FRAME_SUBCAT_WAKE;
	u8 *pos;

	int frame_len = IEEE80211_MIN_ACTION_SIZE + sizeof(morse_oui) +
	    sizeof(subcategory) + payload_len;

	skb = dev_alloc_skb(frame_len + mors->hw->extra_tx_headroom);
	if (!skb)
		return -ENOMEM;

	skb_reserve(skb, mors->hw->extra_tx_headroom);
	action = (struct ieee80211_mgmt *)skb_put(skb, IEEE80211_MIN_ACTION_SIZE);
	memset(action, 0, IEEE80211_MIN_ACTION_SIZE);

	/* It has been agreed that MM action frames get sent out at VO aci */
	skb_set_queue_mapping(skb, IEEE80211_AC_VO);

	/* Must be held while finding and dereferencing sta */
	rcu_read_lock();

	sta = ieee80211_find_sta_by_ifaddr(mors->hw, dest_addr, vif->addr);

	if (sta && sta->mfp) {
		if (no_hwcrypt) {
			rcu_read_unlock();
			MORSE_WARN(mors,
				   "Can't send protected action frame with soft encryption\n");
			goto error_free_skb;
		}

		action->u.action.category = WLAN_CATEGORY_VENDOR_SPECIFIC_PROTECTED;
		action->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT |
						    IEEE80211_STYPE_ACTION |
						    IEEE80211_FCTL_PROTECTED);
	} else {
		action->u.action.category = WLAN_CATEGORY_VENDOR_SPECIFIC;
		action->frame_control = cpu_to_le16(IEEE80211_FTYPE_MGMT | IEEE80211_STYPE_ACTION);
	}

	rcu_read_unlock();

	memcpy(action->da, dest_addr, ETH_ALEN);
	memcpy(action->sa, vif->addr, ETH_ALEN);
	memcpy(action->bssid, vif->bss_conf.bssid, ETH_ALEN);

	pos = skb_put(skb, sizeof(morse_oui));
	memcpy(pos, morse_oui, sizeof(morse_oui));

	pos = skb_put(skb, sizeof(subcategory));
	memcpy(pos, &subcategory, sizeof(subcategory));

	pos = skb_put(skb, payload_len);
	memcpy(pos, payload, payload_len);

	/* Marking the packet as 'TX_FILTERED' will cause it
	 * to be rescheduled internal to mac80211. After this,
	 * the skb will go through the normal tx path.
	 */
	info = IEEE80211_SKB_CB(skb);
	info->control.vif = vif;
	info->flags |= IEEE80211_TX_STAT_TX_FILTERED;
	MORSE_IEEE80211_TX_STATUS(mors->hw, skb);

	return 0;

error_free_skb:
	morse_mac_skb_free(mors, skb);
	return -1;
}

void morse_mac_send_buffered_bc(struct ieee80211_vif *vif)
{
	int count = max_mc_frames;
	struct sk_buff *bc_frame;
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);
	struct morse *mors = morse_vif_to_morse(mors_vif);

	bc_frame = ieee80211_get_buffered_bc(mors->hw, vif);

	while (bc_frame) {
		morse_mac_ops_tx(mors->hw, NULL, bc_frame);

		if (count > 0)
			count--;
		if (max_mc_frames > 0 && count <= 0)
			break;
		bc_frame = ieee80211_get_buffered_bc(mors->hw, vif);
	}
}

/**
 * morse_mac_rx_center_freq_s1g_to_5g - Convert an Rx S1G channel center frequency to its
 * 5G equivalent
 *
 * @mors: Morse structure
 * @rx_s1g_freq_khz: Received frame center frequency in kHz
 * @rx_bw_mhz: S1G BW of the received frame in MHz
 *
 * Return: 5G equivalent channel center frequency of received frame in MHz
 */
static int morse_mac_rx_center_freq_s1g_to_5g(struct morse *mors, u32 rx_s1g_freq_khz,
	u8 rx_bw_mhz)
{
	int rx_freq;
	u32 s1g_chan;
	int chan_5g;

	s1g_chan = morse_dot11ah_freq_khz_bw_mhz_to_chan(rx_s1g_freq_khz, rx_bw_mhz);
	chan_5g = morse_dot11ah_s1g_chan_to_5g_chan(s1g_chan);
	rx_freq = ieee80211_channel_to_frequency(chan_5g, NL80211_BAND_5GHZ);

	if (rx_freq == 0)
		MORSE_WARN_RATELIMITED(mors, "Unable to determine 5G frequency of RX frame\n");

	return rx_freq;
}

void
morse_mac_rx_status(struct morse *mors,
		    const struct morse_skb_rx_status *hdr_rx_status,
		    struct ieee80211_rx_status *rx_status, struct sk_buff *skb)
{
	struct ieee80211_vif *vif = morse_get_vif_from_rx_status(mors, hdr_rx_status);
	__le16 fc = ((struct ieee80211_hdr *)skb->data)->frame_control;
	u8 mcs_index;
	u8 nss_index;
	enum dot11_bandwidth bw_idx;
	u32 bw_mhz;
#if KERNEL_VERSION(4, 12, 0) > MAC80211_VERSION_CODE
	enum nl80211_chan_width chan_width = mors->hw->conf.chandef.width;
#endif
	u32 flags = le32_to_cpu(hdr_rx_status->flags);

	rx_status->signal = le16_to_cpu(hdr_rx_status->rssi);

	if (vif) {
		struct ieee80211_sta *sta = NULL;
		const struct ieee80211_hdr *hdr = (const struct ieee80211_hdr *)skb->data;
		const u8 *sta_lookup_addr = (ieee80211_is_s1g_beacon(fc)) ?
			hdr->addr1 : hdr->addr2;
		bool is_null_data = (ieee80211_is_qos_nullfunc(fc) || ieee80211_is_nullfunc(fc));

		/* Must be held while finding and dereferencing sta */
		rcu_read_lock();
		sta = ieee80211_find_sta(vif, sta_lookup_addr);
		if (sta) {
			struct morse_sta *msta = (struct morse_sta *)sta->drv_priv;
			struct morse_skb_rx_status *status = ieee80211_is_mgmt(fc) ?
				&msta->last_rx.mgmt_status : &msta->last_rx.data_status;

			if (ieee80211_is_mgmt(fc)) {
				msta->last_rx.is_mgmt_set = true;
				memcpy(status, hdr_rx_status, sizeof(*status));
			} else if (!is_null_data) {
				msta->last_rx.is_data_set = true;
				morse_bss_stats_update_rx(vif, skb, sta, status);
				memcpy(status, hdr_rx_status, sizeof(*status));
			}

			msta->avg_rssi = msta->avg_rssi ?
			    CALC_AVG_RSSI(msta->avg_rssi, rx_status->signal) : rx_status->signal;
		}
		rcu_read_unlock();

		if (ieee80211_vif_is_mesh(vif)) {
			/* fill in TSF and flag its presence. Currently Enabling max rx timestamp
			 * only for mesh operation mode as this information is required for Neighbor
			 * offset synchronization method (IEEE802.11-2020: section 14.3.2.2 Neighbor
			 * offset synchronization method) in the mac80211.
			 */
			rx_status->mactime = le64_to_cpu(hdr_rx_status->rx_timestamp_us);
			rx_status->flag |= RX_FLAG_MACTIME_START;
		}

		if (!sta && morse_mac_is_iface_ap_type(vif))
			morse_pre_assoc_peer_update_rx_info(mors, sta_lookup_addr,
							    hdr_rx_status->morse_ratecode);
	}

	if (flags & MORSE_RX_STATUS_FLAGS_DECRYPTED)
		rx_status->flag |= RX_FLAG_DECRYPTED;

	rx_status->band = NL80211_BAND_5GHZ;
	/* Calculate bandwidth of rx status object in MHz */
	bw_idx = morse_ratecode_bw_index_get(hdr_rx_status->morse_ratecode);
	bw_mhz = morse_ratecode_bw_index_to_s1g_bw_mhz(bw_idx);

	rx_status->freq = morse_mac_rx_center_freq_s1g_to_5g(mors,
					KHZ100_TO_KHZ(le16_to_cpu(hdr_rx_status->freq_100khz)),
					bw_mhz);

	nss_index = morse_ratecode_nss_index_get(hdr_rx_status->morse_ratecode);
#if KERNEL_VERSION(4, 12, 0) <= MAC80211_VERSION_CODE
	rx_status->nss = NSS_IDX_TO_NSS(nss_index);
#else
	rx_status->vht_nss = NSS_IDX_TO_NSS(nss_index);
#endif
	rx_status->antenna = 1;

	mcs_index = morse_ratecode_mcs_index_get(hdr_rx_status->morse_ratecode);
	/* If MCS10, convert to MCS0 to keep rate control happy. */
	if (mcs_index == 10) {
		rx_status->rate_idx = 0;
		mors->debug.mcs_stats_tbl.mcs10.rx_count++;
	} else {
		rx_status->rate_idx = mcs_index;
		if (mcs_index == 0)
			mors->debug.mcs_stats_tbl.mcs0.rx_count++;
	}

	if (morse_ratecode_sgi_get(hdr_rx_status->morse_ratecode))
#if KERNEL_VERSION(4, 12, 0) <= MAC80211_VERSION_CODE
		rx_status->enc_flags |= RX_ENC_FLAG_SHORT_GI;
#else
		rx_status->flag |= RX_FLAG_SHORT_GI;
#endif

#if KERNEL_VERSION(4, 12, 0) <= MAC80211_VERSION_CODE
	bw_idx = morse_ratecode_bw_index_get(hdr_rx_status->morse_ratecode);
	bw_mhz = morse_ratecode_bw_index_to_s1g_bw_mhz(bw_idx);

	rx_status->encoding = RX_ENC_VHT;
	rx_status->bw = morse_mac_rx_bw_to_skb_vht(mors, bw_mhz);
#else
	if (chan_width == NL80211_CHAN_WIDTH_160)
		rx_status->vht_flag |= RX_VHT_FLAG_160MHZ;
	else if (chan_width == NL80211_CHAN_WIDTH_80)
		rx_status->vht_flag |= RX_VHT_FLAG_80MHZ;
#endif
}

/* Utility func to transmit driver generated management frames */
int morse_mac_tx_mgmt_frame(struct ieee80211_vif *vif, struct sk_buff *skb)
{
	struct morse_vif *mors_vif;
	struct morse_skb_tx_info tx_info = { 0 };
	struct morse_skbq *mq;
	struct ieee80211_tx_info *info;
	int tx_bw_mhz = 1;
	struct morse *mors;

	mors_vif = (struct morse_vif *)vif->drv_priv;
	mors = morse_vif_to_morse(mors_vif);

	info = IEEE80211_SKB_CB(skb);
	info->control.vif = vif;

	mq = mors->cfg->ops->skbq_mgmt_tc_q(mors);
	if (!mq)
		return -1;

	/* Convert the packet to s1g format */
	if (morse_mac_pkt_to_s1g(mors, NULL, &skb, &tx_bw_mhz) < 0)
		return -1;

	morse_fill_tx_info(mors, &tx_info, skb, mors_vif, tx_bw_mhz);

	if (morse_skbq_skb_tx(mq, &skb, &tx_info, MORSE_SKB_CHAN_MGMT))
		return -1;

	return 0;
}

static void morse_mac_tx_probe_req_change_seq(struct ieee80211_vif *vif)
{
	struct sk_buff *skb = NULL;
	struct morse_vif *mors_vif;
	struct morse *mors;

	if (!enable_bcn_change_seq_monitor || !vif)
		return;

	mors_vif = (struct morse_vif *)vif->drv_priv;
	mors = morse_vif_to_morse(mors_vif);

	if (!mors_vif->is_sta_assoc)
		return;

	/**
	 * Template probe request buffer is expected to be populated in
	 * morse_mac_ops_bss_info_changed event handler and use it here. Below portion
	 * is required only for any handling any corner cases  like update of beacon
	 * change seq is detected, just immediately after the station is associated
	 * and bss_info handler is not invoked. If it's null, request from here & use it.
	 */
	if (!mors_vif->probe_req_buf) {
		mors_vif->probe_req_buf = ieee80211_ap_probereq_get(mors->hw, vif);

		if (!mors_vif->probe_req_buf) {
			MORSE_ERR(mors, "%s: ieee80211_ap_probereq_get failed\n", __func__);
			goto exit;
		}
	}

	skb = skb_copy(mors_vif->probe_req_buf, GFP_ATOMIC);

	if (!skb) {
		MORSE_ERR(mors, "%s: SKB for probereq failed\n", __func__);
		goto exit;
	}
	if (morse_mac_tx_mgmt_frame(vif, skb)) {
		MORSE_ERR(mors, "Failed to send Probe Req for Beacon change sequence\n");
		goto exit;
	}
	MORSE_DBG(mors, "Generated Probe Req for Beacon change sequence\n");
	return;
exit:
	if (skb)
		dev_kfree_skb_any(skb);
}

static void morse_mac_send_probe_req_tasklet(unsigned long data)
{
	struct ieee80211_vif *vif = (struct ieee80211_vif *)data;

	morse_mac_tx_probe_req_change_seq(vif);
}

int morse_send_probe_req_enable(struct ieee80211_vif *vif, bool enable)
{
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);

	if (enable)
		tasklet_enable(&mors_vif->send_probe_req);
	else
		tasklet_disable(&mors_vif->send_probe_req);
	return 0;
}

int morse_send_probe_req_init(struct ieee80211_vif *vif)
{
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);

	tasklet_init(&mors_vif->send_probe_req, morse_mac_send_probe_req_tasklet,
		     (unsigned long)vif);
	tasklet_disable(&mors_vif->send_probe_req);
	return 0;
}

void morse_send_probe_req_finish(struct ieee80211_vif *vif)
{
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);

	tasklet_kill(&mors_vif->send_probe_req);
}

void morse_mac_ecsa_beacon_tx_done(struct morse *mors, struct sk_buff *skb)
{
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	struct ieee80211_vif *vif = info->control.vif ? info->control.vif : morse_get_ap_vif(mors);
	struct morse_vif *mors_vif;
	unsigned long timeout;

	if (!vif || vif->type != NL80211_IFTYPE_AP)
		return;

	mors_vif = (struct morse_vif *)vif->drv_priv;

	if (morse_mac_is_csa_active(vif) && morse_mac_is_s1g_long_beacon(mors, skb)) {
		if (MORSE_IEEE80211_CSA_COMPLETE(vif)) {
			/* If fw doesn't report beacon tx completion to host, trigger the channel
			 * switch after timeout of grace period + 1ms, as beacon buffer is requested
			 * ahead of TBTT. Otherwise trigger after 1ms delay.
			 */
			if (mors->firmware_flags & MORSE_FW_FLAGS_REPORTS_TX_BEACON_COMPLETION)
				timeout = jiffies + msecs_to_jiffies(1);
			else
				timeout = jiffies +
					   msecs_to_jiffies(BEACON_REQUEST_GRACE_PERIOD_MS + 1);

			MORSE_INFO(mors,
				   "%s: Countdown is comp, Trigger Chan Switch, ts=%ld, to=%ld\n",
				   __func__, jiffies, timeout);
			mod_timer(&mors_vif->chswitch_timer, timeout);
		}
	} else if (mors_vif->ecsa_chan_configured) {
		/* Trigger channel configuration immediately if fw reports beacon tx completion
		 * otherwise trigger after grace period + 1ms to make sure that the beacon is
		 * sent out.
		 */
		timeout = (mors->firmware_flags & MORSE_FW_FLAGS_REPORTS_TX_BEACON_COMPLETION) ?
			 0 : msecs_to_jiffies(BEACON_REQUEST_GRACE_PERIOD_MS + 1);

		/*
		 * We will configure channel again after sending beacon in new channel
		 * to perform PHY calibration.
		 */
		MORSE_INFO(mors, "%s: Configure ECSA Chan ts=%ld, to=%ld\n",
			   __func__, jiffies, timeout);
		schedule_delayed_work(&mors_vif->ecsa_chswitch_work, timeout);
		mors_vif->ecsa_chan_configured = false;
		/* Reset channel info */
		memset(&mors_vif->ecsa_channel_info, 0, sizeof(mors_vif->ecsa_channel_info));
		mors_vif->mask_ecsa_info_in_beacon = false;
	}
}

static void morse_mac_process_bcn_change_seq_tx_finish(struct morse *mors,
		struct sk_buff *skb,
		struct morse_vif *mors_vif)
{
	struct ieee80211_vif *vif = morse_vif_to_ieee80211_vif(mors_vif);
	/*
	 * Check if probe req frame to be sent after STA detected
	 * update in beacon change sequence number and notified mac80211.
	 * mac80211 will send out QoS NULL with PM clear and on completion
	 * of QoS NULL data, here schedule to send unicast/directed probe req
	 */
	if (mors_vif->waiting_for_probe_req_sched &&
	    mors_vif->is_sta_assoc) {
		MORSE_INFO(mors, "%s: Send probe req for updated beacon\n", __func__);
		morse_mac_schedule_probe_req(vif);
	}
}

void morse_mac_process_tx_finish(struct morse *mors, struct sk_buff *skb)
{
	struct morse_vif *mors_vif = NULL;
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	struct ieee80211_tx_info *info = IEEE80211_SKB_CB(skb);
	struct ieee80211_vif *vif = info->control.vif;

	if (!vif) {
		MORSE_WARN_ON_ONCE(FEATURE_ID_DEFAULT, !vif);
		return;
	}

	mors_vif = (struct morse_vif *)vif->drv_priv;
	if (vif->type == NL80211_IFTYPE_STATION) {
		if (ieee80211_is_nullfunc(hdr->frame_control) ||
			ieee80211_is_qos_nullfunc(hdr->frame_control))
			morse_mac_process_bcn_change_seq_tx_finish(mors, skb, mors_vif);
		else if (ieee80211_is_action(hdr->frame_control))
			morse_mac_process_twt_action_tx_finish(mors, vif, skb);
	} else if (vif->type == NL80211_IFTYPE_AP &&
			ieee80211_is_s1g_beacon(hdr->frame_control))
		morse_mac_ecsa_beacon_tx_done(mors, skb);
}

/**
 * morse_mac_process_beacon: Process S1G beacon
 *
 * @mors: Morse chip struct.
 * @vif: pointer to interface.
 * @skb: pointer to beacon skb.
 * @ies_mask: Contains array of information elements.
 */
static void morse_mac_process_s1g_beacon(struct morse *mors, struct ieee80211_vif *vif,
					 struct sk_buff *skb, struct dot11ah_ies_mask *ies_mask)
{
	const struct ieee80211_ext *s1g_beacon = (struct ieee80211_ext *)skb->data;
	struct morse_vif *mors_vif;
	u16 short_beacon;
	const struct dot11ah_s1g_tim_ie *s1g_tim =
				(const struct dot11ah_s1g_tim_ie *)ies_mask->ies[WLAN_EID_TIM].ptr;
	size_t total_len = ies_mask->ies[WLAN_EID_TIM].len;

	if (!vif)
		return;

	mors_vif = ieee80211_vif_to_morse_vif(vif);

	/* S1G beacons are not management frames, but are processed the same way */
	morse_vendor_ie_process_rx_mgmt(vif, skb);

	/* Past here we only care if we are an associated station and the beacon is from our BSS */
	if (vif->type != NL80211_IFTYPE_STATION || !mors_vif->is_sta_assoc ||
	    memcmp(vif->bss_conf.bssid, s1g_beacon->u.s1g_beacon.sa, ETH_ALEN) != 0)
		return;

	/*
	 * Check for Change Sequence number update in beacon and
	 * generate the probe request to get probe resp or wait for the full beacon.
	 * ECSA: Check for ECSA IE and save the channel info.
	 */
	if (mors_vif->s1g_bcn_change_seq == INVALID_BCN_CHANGE_SEQ_NUM) {
		/* Initialize the change seq number to track for STA */
		mors_vif->s1g_bcn_change_seq = s1g_beacon->u.s1g_beacon.change_seq;
	} else if (mors_vif->s1g_bcn_change_seq != s1g_beacon->u.s1g_beacon.change_seq) {
		/* Generate the probe Req */
		mors_vif->s1g_bcn_change_seq = s1g_beacon->u.s1g_beacon.change_seq;

		/* Check if the feature is enabled to generate probe req on
		 * detection of update in beacon change seq number
		 */
		if (enable_bcn_change_seq_monitor) {
			/* Notify mac80211 to wakeup from power save to send probe req */
			MORSE_DBG(mors,
				  "Beacon changed! Report Bcn loss,ps=%d, short_bcn=%d,seq_cnt=%d\n",
				  mors->config_ps,
				  (le16_to_cpu(s1g_beacon->frame_control) &
				    IEEE80211_FC_COMPRESS_SSID),
				  s1g_beacon->u.s1g_beacon.change_seq);
			ieee80211_beacon_loss(vif);

			if (!mors->config_ps) {
				/* Schedule the probe req, as we are already awake */
				tasklet_schedule(&mors_vif->send_probe_req);
				MORSE_DBG(mors, "Scheduled to a send probe req\n");
			} else {
				/* Set a flag. With beacon loss notification,
				 * mac80211 will send a QoS NULL. On Tx complete
				 * of NULL data, probe req will be scheduled to be sent.
				 */
				mors_vif->waiting_for_probe_req_sched = true;
			}
		}
	}

	/* Check for ECSA IE and process it */
	short_beacon = (le16_to_cpu(s1g_beacon->frame_control) & IEEE80211_FC_COMPRESS_SSID);
	if (!short_beacon && ies_mask->ies[WLAN_EID_EXT_CHANSWITCH_ANN].ptr)
		morse_mac_process_ecsa_ie(mors, vif, skb);

	if (morse_mac_is_csa_active(vif) && mors_vif->ecsa_chan_configured) {
		/*
		 * We will configure channel again after receiving beacon in new channel
		 * to perform PHY calibration. This change is not required once the periodic
		 * PHY DC calibration is enabled in firmware. This first beacon in new channel
		 * is required in mac80211 to unblock traffic if it is blocked.
		 */
		MORSE_ECSA_INFO(mors, "%s: Configure ECSA Chan ts=%ld,short_beacon=%d\n",
			   __func__, jiffies, short_beacon);
		/* Schedule immediately */
		schedule_delayed_work(&mors_vif->ecsa_chswitch_work, 0);
		mors_vif->ecsa_chan_configured = false;
	}

	/* Verify TIM IE */
	if (s1g_tim && total_len >= offsetof(struct ieee80211_tim_ie, virtual_map)) {
		u8 page_slice = (s1g_tim->bitmap_control & IEEE80211_S1G_TIM_BITMAP_PAGE_SLICE) >>
			IEEE80211_S1G_TIM_BITMAP_PAGE_SLICE_SHIFT;

		/*
		 * The Page Slice Number subfield indicates which page slice is encoded in the
		 * Partial Virtual Bitmap field when the subfield is in the range of 0 to 30.
		 * If the Page Slice Number subfield is 31, then the entire page indicated by the
		 * Page Index subfield value is encoded in the Partial Virtual Bitmap field of the
		 * TIM elements with the same page index.
		 * Allow page slice number 31 alone when page slicing is disabled.
		 */
		if (!mors_vif->bss_vendor_info.page_slicing_exclusive_support && page_slice > 0 &&
						page_slice != S1G_TIM_PAGE_SLICE_ENTIRE_PAGE) {
			/* Set length to the size of TIM IE */
			MORSE_WARN_RATELIMITED(mors, "PageSlice %d doesn't indicate entire page\n",
										page_slice);
			ies_mask->ies[WLAN_EID_TIM].len = sizeof(struct ieee80211_tim_ie) - 1;
		}
	}
}

/**
 * morse_mac_find_vif_for_bcast_mcast: Searches for suitable interface for given skb.
 *
 * @mors: Morse chip struct.
 * @skb: RX skbuff
 * @vif: Pointer to valid vif
 *
 * @return:
 * * beacon in STA or Mesh mode      - STA VIF
 * * NDP probe response in STA mode  - STA VIF
 * * Probe request in AP mode        - AP VIF
 */
static bool morse_mac_find_vif_for_bcast_mcast(struct morse *mors, struct sk_buff *skb,
					       struct ieee80211_vif **vif)
{
	int idx;
	int dest_vif_id = INVALID_VIF_INDEX;
	struct ieee80211_vif *vif_tmp;
	const struct ieee80211_hdr *hdr = NULL;

	if (skb->len > 0)
		hdr = (const struct ieee80211_hdr *)skb->data;

	if (!hdr)
		return false;

	for (idx = 0; idx < mors->max_vifs; idx++) {
		vif_tmp = morse_get_vif_from_vif_id(mors, idx);
		if (!vif_tmp)
			continue;

		if (!ieee80211_is_mgmt(hdr->frame_control)) {
			/* bool bcast = is_multicast_ether_addr(hdr->addr1);
			 *
			 * MORSE_WARN_RATELIMITED(mors,
			 *    "Unexpected rx data skb %s fc:%04x\n",
			 *     bcast ? "bcast" : "ucast", fc);
			 */
			dest_vif_id = idx;
			break;
		}

		switch (le16_to_cpu(hdr->frame_control) & IEEE80211_FCTL_STYPE) {
		case IEEE80211_STYPE_BEACON:
			if (vif_tmp->type == NL80211_IFTYPE_STATION ||
			    ieee80211_vif_is_mesh(vif_tmp))
				dest_vif_id = idx;
			break;
		case IEEE80211_STYPE_PROBE_RESP:
			if (vif_tmp->type == NL80211_IFTYPE_STATION)
				dest_vif_id = idx;
			break;
		case IEEE80211_STYPE_PROBE_REQ:
			if (morse_mac_is_iface_ap_type(vif_tmp))
				dest_vif_id = idx;
			break;
		default:
			/* MORSE_WARN_RATELIMITED(mors,
			 *	"Unexpected mgmt rx skb ftype %02x stype %02x\n",
			 *	fc & IEEE80211_FCTL_FTYPE, fc & IEEE80211_FCTL_STYPE);
			 * MORSE_HEXDUMP_WARN_ONCE(FEATURE_ID_DEFAULT,"RX MGMT:",
			 *	skb->data, skb->len);
			 */
			dest_vif_id = idx;
			break;
		}

		if (dest_vif_id != INVALID_VIF_INDEX)
			break;
	}

	if (dest_vif_id != INVALID_VIF_INDEX) {
		*vif = morse_get_vif_from_vif_id(mors, dest_vif_id);
		return true;
	}

	return false;
}

/**
 * morse_rx_mgmt_cmmp_replay_check: CCMP replay detection for Rx protected mgmt frames.
 *
 * @note: S1G features implemented in the driver, which rely on processing of S1G protected
 * action frames, would need to perform PN replay check here before mac80211 does, to protect
 * from replay attacks. Other protected action frames, which rely on mac80211 processing
 * doesn't need to do this check.
 *
 * @sta: Peer STA from which action frame is received
 * @skb: RX skbuff
 *
 * @return: true if replay detected or false otherwise
 */
static int morse_rx_mgmt_ccmp_replay_check(struct ieee80211_sta *sta, const struct sk_buff *skb)
{
	struct morse_sta *mors_sta = (struct morse_sta *)sta->drv_priv;
	u8 hdr_len = ieee80211_get_hdrlen_from_skb(skb);
	u8 *ccmp_hdr = skb->data + hdr_len;
	u8 pn[IEEE80211_CCMP_PN_LEN];
	u64 rpn64;
	struct morse_dot11ah_s1g_twt_action *twt_action =
		(struct morse_dot11ah_s1g_twt_action *)(skb->data + IEEE80211_CCMP_HDR_LEN);
	u8 action_code = twt_action->u.twt_action_setup.action_code;

	/* Check for protected TWT action frames */
	if (twt_action->category != WLAN_CATEGORY_S1G_PROTECTED &&
	    (action_code != WLAN_S1G_PROTECTED_TWT_SETUP &&
	     action_code != WLAN_S1G_PROTECTED_TWT_TEARDOWN))
		return false;

	pn[0] = ccmp_hdr[0];
	pn[1] = ccmp_hdr[1];
	pn[2] = ccmp_hdr[4];
	pn[3] = ccmp_hdr[5];
	pn[4] = ccmp_hdr[6];
	pn[5] = ccmp_hdr[7];

    /* Convert the 6-byte PN to a 64-bit integer */
	rpn64 = ((uint64_t)pn[5] << 40) |
		  ((uint64_t)pn[4] << 32) |
		  ((uint64_t)pn[3] << 24) |
		  ((uint64_t)pn[2] << 16) |
		  ((uint64_t)pn[1] << 8)  |
		  (uint64_t)pn[0];

    /* Draft P802.11REVme_D4.0 section 12.5.2.4.4:
     * If management frame protection is negotiated, the receiver shall set
     * the MFPC bit on a given link to 1, it shall discard any individually
     * addressed robust management frame that is received with its PN less
     * than or equal to the value of the replay counter
     */
	if (rpn64 <= mors_sta->last_rx_mgmt_pn)
		return true; /* Replay detected */

	/* Update the last seen PN */
	mors_sta->last_rx_mgmt_pn = rpn64;

	return false; /* No replay detected */
}

int morse_mac_get_tx_attempts(struct morse *mors, struct morse_skb_tx_status *tx_sts)
{
	int attempts = 0;
	int i;
	int count = min_t(int, MORSE_SKB_MAX_RATES, IEEE80211_TX_MAX_RATES);

	for (i = 0; i < count; i++) {
		if (tx_sts->rates[i].count > 0)
			attempts += tx_sts->rates[i].count;
		else
			break;
	}

	return attempts;
}

static int morse_mac_process_s1g_mgmt(struct morse *mors, struct ieee80211_vif *vif,
				      const struct sk_buff *skb,
				      struct dot11ah_ies_mask *ies_mask)
{
	const struct ieee80211_mgmt *hdr = (const struct ieee80211_mgmt *)skb->data;
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);
	struct ieee80211_sta *sta;
	__le16 fc = hdr->frame_control;
	bool is_assoc_req = (ieee80211_is_assoc_req(fc) || ieee80211_is_reassoc_req(fc));
	int ret = 0;

	/* Must be held while finding and dereferencing sta */
	rcu_read_lock();
	sta = ieee80211_find_sta_by_ifaddr(mors->hw, hdr->sa, vif->addr);
	/* PN replay check only for protected action frames */
	if (sta && ieee80211_is_action(fc) && ieee80211_has_protected(fc)) {
		if (sta->mfp && morse_rx_mgmt_ccmp_replay_check(sta, skb)) {
			rcu_read_unlock();
			MORSE_ERR(mors, "%s Mgmt replay detected, dropping\n", __func__);
			ret = -EINVAL;
			goto exit;
		}
	}
	/* Process Assoc Req in RAW module */
	if (vif->type == NL80211_IFTYPE_AP && is_assoc_req && sta)
		ret = morse_raw_process_rx_mgmt(mors, vif, sta, skb, ies_mask);
	rcu_read_unlock();

	morse_vendor_rx_caps_ops_ie(mors_vif, hdr, ies_mask);

	if (mors_vif->cac.enabled && vif->type == NL80211_IFTYPE_AP && ieee80211_is_auth(fc))
		morse_cac_count_auth(vif, hdr);

	morse_vendor_ie_process_rx_mgmt(vif, skb);

	/* Deal with TWT messages. */
	if (morse_mac_is_iface_infra_bss_type(vif))
		ret = morse_mac_process_rx_twt_mgmt(mors, vif, skb, ies_mask);

	if (mors_vif->enable_pv1 && ieee80211_is_action(fc)) {
		struct morse_dot11ah_s1g_action *s1g_mgmt =
			(struct morse_dot11ah_s1g_action *)skb->data;

		if (s1g_mgmt->category == WLAN_CATEGORY_S1G_PROTECTED &&
			s1g_mgmt->u.pv1_action.action_code == WLAN_S1G_HEADER_COMPRESSION) {
			morse_mac_process_pv1_action_frame(s1g_mgmt, mors, vif);
			/* Do not forward to mac80211 */
			ret = -EINVAL;
		}
	}
exit:
	return ret;
}

static int morse_mac_preprocess_s1g_action(struct morse *mors,
		struct ieee80211_vif *vif,
		struct sk_buff *skb)
{
	const struct ieee80211_mgmt *hdr = (const struct ieee80211_mgmt *)skb->data;
	__le16 fc = hdr->frame_control;
	bool is_protected = ieee80211_has_protected(fc);
	struct morse_dot11ah_s1g_twt_action *twt_action =
			(struct morse_dot11ah_s1g_twt_action *)(skb->data +
				(is_protected ? IEEE80211_CCMP_HDR_LEN : 0));
	u8 action_code = twt_action->u.twt_action_setup.action_code;

	/* Trim the MIC field for internally processed action frames */
	if (twt_action->category == WLAN_CATEGORY_S1G_PROTECTED &&
		 (action_code == WLAN_S1G_PROTECTED_TWT_SETUP ||
		  action_code == WLAN_S1G_PROTECTED_TWT_TEARDOWN))
		skb_trim(skb, skb->len - IEEE80211_CCMP_MIC_LEN);
	return 0;
}

static int morse_mac_process_s1g_mgmt_or_beacon(struct morse *mors,
						struct ieee80211_vif *vif,
						struct sk_buff *skb,
						const struct morse_skb_rx_status *hdr_rx_status,
						const struct ieee80211_rx_status *rx_status,
						struct dot11ah_ies_mask *ies_mask)
{
	int s1g_ies_length = 0;
	int s1g_hdr_length = 0;
	u8 *s1g_mgmt_ies = NULL;
	__le16 fc = ((struct ieee80211_hdr *)skb->data)->frame_control;
	bool is_mgmt = ieee80211_is_mgmt(fc);
	bool is_s1g_beacon = ieee80211_is_s1g_beacon(fc);
	bool is_assoc_req = ieee80211_is_assoc_req(fc) || ieee80211_is_reassoc_req(fc);
	struct morse_vif *mors_vif = ieee80211_vif_to_morse_vif(vif);
	int ret = 0;

	if (!is_mgmt && !is_s1g_beacon)
		return 0;

	/* Pre-process protected S1G management frames before parsing */
	if (ieee80211_is_action(fc) && ieee80211_has_protected(fc))
		morse_mac_preprocess_s1g_action(mors, vif, skb);

	/* Parse S1G IEs into IES mask */
	s1g_mgmt_ies = morse_mac_get_ie_pos(skb, &s1g_ies_length, &s1g_hdr_length, true);
	if (s1g_mgmt_ies) {
		if (morse_dot11ah_parse_ies(s1g_mgmt_ies, s1g_ies_length, ies_mask) < 0) {
			MORSE_INFO_RATELIMITED(mors, "%pM Failed to parse ies - fc 0x%04x\n",
					       ieee80211_get_SA((struct ieee80211_hdr *)skb->data),
					       fc);
			return -EINVAL;
		}

		/* Validate S1G capabilities in mgmt / beacon frame is supported by us */
		if (!morse_mac_capabilities_validate(mors, ies_mask, vif, skb)) {
			MORSE_INFO_RATELIMITED(mors, "%pM S1G capabilities mismatch - fc 0x%04x",
					       ieee80211_get_SA((struct ieee80211_hdr *)skb->data),
					       fc);

			if (is_assoc_req) {
				/* Respond to an S1G invalid assoc request by removing
				 * the IEs from the frame prior to passing up to mac80211. The
				 * higher layers will see this frame as invalid and generate
				 * the required assoc response with a failure code.
				 */
				skb_trim(skb, skb->len - s1g_ies_length);
				morse_dot11ah_ies_mask_clear(ies_mask);
				return 0;
			}

			return -EINVAL;
		}
	}

	/* MGMT or beacon specific processing */
	if (is_mgmt) {
		ret = morse_mac_process_s1g_mgmt(mors, vif, skb, ies_mask);
		if (ret)
			return ret;
	} else if (is_s1g_beacon) {
		morse_mac_process_s1g_beacon(mors, vif, skb, ies_mask);
#if KERNEL_VERSION(5, 1, 0) < LINUX_VERSION_CODE
		if (morse_mbssid_ie_enabled(mors)) {
			morse_process_beacon_from_mbssid_ie(mors, skb, ies_mask, vif,
							    hdr_rx_status);
		}
#endif
	}

	/* Mesh vif processing */
	if (ieee80211_vif_is_mesh(vif)) {
		int ret = morse_mac_process_mesh_rx_mgmt(mors_vif, skb, ies_mask, rx_status);
		/* drop mgmt frame, if mesh module indicates so */
		if (ret == -EACCES)
			return ret;
	}

	/* Finally - process S1G Capability field */
	return morse_mac_process_s1g_caps(mors, vif, skb, ies_mask);
}

void morse_mac_skb_recv(struct morse *mors,
			struct sk_buff *skb,
			struct morse_skb_rx_status *hdr_rx_status)
{
	struct ieee80211_hw *hw = mors->hw;
	struct dot11ah_ies_mask *ies_mask = NULL;
	struct ieee80211_vif *vif;
	struct ieee80211_rx_status rx_status = {0};
	int length_11n;
	struct morse_vif *mors_vif;
	bool skb_needs_free = true;

	if (!mors->started)
		goto exit;

	if (!skb->data || skb->len == 0)
		goto exit;

#ifdef CONFIG_MORSE_MONITOR
	if (mors->monitor_mode) {
		morse_mon_rx(mors, skb, hdr_rx_status);
		/* If we have a monitor interface, don't bother doing any
		 * other work on the SKB as we only support a single interface
		 */
		goto exit;
	}
#endif

	vif = morse_get_vif_from_rx_status(mors, hdr_rx_status);

	ies_mask = morse_dot11ah_ies_mask_alloc();
	if (!ies_mask)
		goto exit;

	/* The firmware passes up broadcast mgmt frames such as beacons with a NULL VIF.
	 * Assign the correct VIF. If no matching VIF was found, the VIF is not yet up.
	 */
	if (!vif && !morse_mac_find_vif_for_bcast_mcast(mors, skb, &vif))
		goto exit;

	mors_vif = ieee80211_vif_to_morse_vif(vif);

	/* Update vif index in rx status if it had invalid index */
	if (MORSE_RX_STATUS_FLAGS_VIF_ID_GET(le32_to_cpu(hdr_rx_status->flags)) ==
			INVALID_VIF_INDEX) {
		hdr_rx_status->flags =
			cpu_to_le32(MORSE_RX_STATUS_FLAGS_VIF_ID_CLEAR(le32_to_cpu(hdr_rx_status
			->flags)));
		hdr_rx_status->flags |=
			cpu_to_le32(MORSE_RX_STATUS_FLAGS_VIF_ID_SET(mors_vif->id));
	}

	/* Attempt to convert S1G PV1 to S1G PV0 */
	if (morse_dot11ah_is_pv1_qos_data(le16_to_cpu(((struct ieee80211_hdr *)skb->data)->
		frame_control))) {
		if (morse_mac_convert_pv1_to_pv0(mors, mors_vif, skb, hdr_rx_status,
						(struct dot11ah_mac_pv1_hdr *)skb->data))
			goto exit;
	}

	/* Fill iee80211 rx_status flags from morse RX status object */
	morse_mac_rx_status(mors, hdr_rx_status, &rx_status, skb);
	memcpy(IEEE80211_SKB_RXCB(skb), &rx_status, sizeof(rx_status));

	/* MGMT and beacon frames need to be inspected by the driver.
	 * Logic in the following function may dictate that the frame must be
	 * dropped (ignored) or modified prior to passing through
	 * the S1G to 11n conversion layer and then subsequently, mac80211.
	 */
	if (morse_mac_process_s1g_mgmt_or_beacon(mors, vif, skb, hdr_rx_status, &rx_status,
						 ies_mask))
		goto exit;

	/* Expand SKB to make room for 11n coversion (if required) */
	length_11n = morse_dot11ah_s1g_to_11n_rx_packet_size(vif, skb, ies_mask);
	if (length_11n < 0)
		goto exit;

	if (skb->len + skb_tailroom(skb) < length_11n) {
		struct sk_buff *skb2;
		u8 *s1g_ies;
		int s1g_ies_length;
		int s1g_hdr_length;

		skb2 = skb_copy_expand(skb, skb_headroom(skb), length_11n - skb->len, GFP_KERNEL);
		morse_mac_skb_free(mors, skb);
		skb = skb2;
		if (!skb)
			goto exit;

		/* Since we have freed the old skb, we must also clear the mask
		 * because now it will have references to invalid memory
		 */
		morse_dot11ah_ies_mask_clear(ies_mask);
		s1g_ies = morse_mac_get_ie_pos(skb, &s1g_ies_length, &s1g_hdr_length, true);
		if (s1g_ies) {
			if (morse_dot11ah_parse_ies(s1g_ies, s1g_ies_length, ies_mask) < 0) {
				MORSE_WARN_RATELIMITED(mors, "Failed to Parse IEs: %d\n",
						       s1g_ies_length);
				goto exit;
			}
		}
	}

	/* Perform S1G to 11n conversion prior to passing to mac80211 */
	morse_dot11ah_s1g_to_11n_rx_packet(vif, skb, length_11n, ies_mask);

	if (skb->len > 0) {
		ieee80211_rx_irqsafe(hw, skb);
		skb_needs_free = false;
	}

exit:
	if (skb_needs_free)
		morse_mac_skb_free(mors, skb);

	morse_dot11ah_ies_mask_free(ies_mask);
}

static void morse_mac_config_ht_cap(struct morse *mors)
{
	struct ieee80211_sta_ht_cap *morse_ht_cap = &mors_band_5ghz.ht_cap;

	if (mors->custom_configs.enable_sgi_rc)
		morse_ht_cap->cap |= (IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40);

	morse_ht_cap->cap |= IEEE80211_HT_CAP_SUP_WIDTH_20_40;
}

static void morse_mac_config_vht_base_cap(struct morse *mors)
{
	struct ieee80211_sta_vht_cap *morse_vht_cap = &mors_band_5ghz.vht_cap;
	struct morse_caps *s1g_caps = &mors->capabilities;
	u16 mcs_map = 0;
	int i;

	morse_vht_cap->vht_supported = true;

	if (MORSE_CAPAB_SUPPORTED(s1g_caps, RX_LDPC))
		morse_vht_cap->cap |= IEEE80211_VHT_CAP_RXLDPC;
	if (MORSE_CAPAB_SUPPORTED(s1g_caps, RX_STBC))
		morse_vht_cap->cap |= IEEE80211_VHT_CAP_RXSTBC_1;
	if (MORSE_CAPAB_SUPPORTED(s1g_caps, TX_STBC))
		morse_vht_cap->cap |= IEEE80211_VHT_CAP_TXSTBC;
	if (MORSE_CAPAB_SUPPORTED(s1g_caps, SU_BEAMFORMER))
		morse_vht_cap->cap |= IEEE80211_VHT_CAP_SU_BEAMFORMER_CAPABLE;
	if (MORSE_CAPAB_SUPPORTED(s1g_caps, SU_BEAMFORMEE))
		morse_vht_cap->cap |= IEEE80211_VHT_CAP_SU_BEAMFORMEE_CAPABLE;
	if (MORSE_CAPAB_SUPPORTED(s1g_caps, MU_BEAMFORMER))
		morse_vht_cap->cap |= IEEE80211_VHT_CAP_MU_BEAMFORMER_CAPABLE;
	if (MORSE_CAPAB_SUPPORTED(s1g_caps, MU_BEAMFORMEE))
		morse_vht_cap->cap |= IEEE80211_VHT_CAP_MU_BEAMFORMEE_CAPABLE;

	morse_vht_cap->cap |=
	    (s1g_caps->beamformee_sts_capability << IEEE80211_VHT_CAP_BEAMFORMEE_STS_SHIFT);

	morse_vht_cap->cap |=
	    (s1g_caps->number_sounding_dimensions << IEEE80211_VHT_CAP_SOUNDING_DIMENSIONS_SHIFT);

	/* Each 2 bits in mcs_map corresponds to a spatial stream. */
	for (i = 0; i < NL80211_VHT_NSS_MAX; i++) {
		if (i == 0 ||
		    (i == 1 && MORSE_CAPAB_SUPPORTED(s1g_caps, 2SS)) ||
		    (i == 2 && MORSE_CAPAB_SUPPORTED(s1g_caps, 3SS)) ||
		    (i == 3 && MORSE_CAPAB_SUPPORTED(s1g_caps, 4SS))) {
			/* We are mapping the max S1G MCS to VHT MCS as 2->7, 7->8, 9->9.
			 * Assume max MCS supported is the same for all spatial streams.
			 */
			if (MORSE_CAPAB_SUPPORTED(s1g_caps, MCS9) ||
			    MORSE_CAPAB_SUPPORTED(s1g_caps, MCS8))
				mcs_map |= (IEEE80211_VHT_MCS_SUPPORT_0_9 <<
					    (i * S1G_CAP_BITS_PER_MCS_NSS));
			else
				mcs_map |= (IEEE80211_VHT_MCS_SUPPORT_0_8 <<
					    (i * S1G_CAP_BITS_PER_MCS_NSS));

			continue;
		}
		mcs_map |= (IEEE80211_VHT_MCS_NOT_SUPPORTED << (i * S1G_CAP_BITS_PER_MCS_NSS));
	}

	MORSE_DBG(mors, "%s: vht rx_mcs_map 0x%04x", __func__, mcs_map);
	MORSE_DBG(mors, "%s: vht tx_mcs_map 0x%04x", __func__, mcs_map);
	morse_vht_cap->vht_mcs.rx_mcs_map = cpu_to_le16(mcs_map);
	morse_vht_cap->vht_mcs.tx_mcs_map = cpu_to_le16(mcs_map);
}

static void morse_mac_config_vht_80_cap(struct morse *mors)
{
	struct ieee80211_sta_vht_cap *morse_vht_cap = &mors_band_5ghz.vht_cap;

	morse_vht_cap->cap = IEEE80211_VHT_CAP_MAX_MPDU_LENGTH_11454 |
	    IEEE80211_VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT_MASK;

	if (mors->custom_configs.enable_sgi_rc)
		morse_vht_cap->cap |= IEEE80211_VHT_CAP_SHORT_GI_80;
}

static void morse_mac_config_vht_160_cap(struct morse *mors)
{
	struct ieee80211_sta_vht_cap *morse_vht_cap = &mors_band_5ghz.vht_cap;

	morse_vht_cap->cap |= IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ;

	if (mors->custom_configs.enable_sgi_rc)
		morse_vht_cap->cap |= IEEE80211_VHT_CAP_SHORT_GI_160;
}

static void morse_mac_config_wiphy_flags(struct morse *mors)
{
	struct wiphy *wiphy = mors->wiphy;

	wiphy->flags |= WIPHY_FLAG_AP_PROBE_RESP_OFFLOAD;
	wiphy->flags |= WIPHY_FLAG_HAS_CHANNEL_SWITCH;
	wiphy->flags |= WIPHY_FLAG_IBSS_RSN;
	wiphy->flags |= WIPHY_FLAG_AP_UAPSD;

	/* If the driver was loaded with enable_ps=0, ensure cfg80211 is aware that
	 * we can't enable powersave.
	 */
	if (!morse_mac_ps_enabled(mors))
		wiphy->flags &= ~WIPHY_FLAG_PS_ON_BY_DEFAULT;
}

static void morse_mac_config_wiphy(struct morse *mors)
{
	struct wiphy *wiphy = mors->wiphy;
	struct ieee80211_iface_combination *comb;
	struct ieee80211_iface_limit *if_limits;

	/* Must be set before coming here */
	if (mors->max_vifs == 0) {
		MORSE_ERR(mors, "%s: Max vifs not set\n", __func__);
		return;
	}

	morse_mac_config_wiphy_flags(mors);

	wiphy->features |= NL80211_FEATURE_AP_MODE_CHAN_WIDTH_CHANGE;

	wiphy->probe_resp_offload |=
	    NL80211_PROBE_RESP_OFFLOAD_SUPPORT_WPS |
	    NL80211_PROBE_RESP_OFFLOAD_SUPPORT_WPS2 | NL80211_PROBE_RESP_OFFLOAD_SUPPORT_P2P;

	wiphy->features |= NL80211_FEATURE_TX_POWER_INSERTION;

	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_SET_SCAN_DWELL);
	wiphy_ext_feature_set(wiphy, NL80211_EXT_FEATURE_VHT_IBSS);

	comb = kcalloc(1, sizeof(*comb), GFP_KERNEL);
	if_limits = kcalloc(1, sizeof(*if_limits), GFP_KERNEL);

	wiphy->iface_combinations = comb;
	wiphy->n_iface_combinations = 1;

	comb->max_interfaces = mors->max_vifs;
	comb->limits = if_limits;
	comb->n_limits = 1;
	comb->num_different_channels = 1;

	if_limits->max = mors->max_vifs;
	if_limits->types = BIT(NL80211_IFTYPE_STATION);

	if (!is_sta_mode_only()) {
		if_limits->types |= BIT(NL80211_IFTYPE_AP);

		if (morse_mac_mesh_enabled(wiphy))
			if_limits->types |= BIT(NL80211_IFTYPE_MESH_POINT);
		MORSE_INFO(mors, "Mesh types supported: %s\n",
			   morse_mac_mesh_enabled(wiphy) ? "true" : "false");

#if KERNEL_VERSION(4, 10, 0) <= MAC80211_VERSION_CODE
		/**
		 * Allow different beacon intervals for AP and mesh interfaces. The gcd of all
		 * beacon intervals from beaconing interfaces of above interface group combination
		 * must be greater than or equal to beacon_int_min_gcd. This condition is validated
		 * in mac80211 while bringing up interfaces.
		 */
		comb->beacon_int_min_gcd = 1;
#endif
	}

	if (mors_ops.hw_scan) {
		wiphy->max_scan_ie_len = 512;
		wiphy->max_scan_ssids = 1;
		wiphy->signal_type = CFG80211_SIGNAL_TYPE_MBM;
		wiphy->max_remain_on_channel_duration = 10000;
	}

	if (mors_ops.sched_scan_start) {
#if KERNEL_VERSION(4, 12, 0) < MAC80211_VERSION_CODE
		wiphy->max_sched_scan_reqs = 1;
#endif
		wiphy->max_sched_scan_ssids = 1;
		wiphy->max_match_sets = 6;
		wiphy->max_sched_scan_ie_len = 512;
		wiphy->max_sched_scan_plans = 12;
		wiphy->max_sched_scan_plan_interval = U32_MAX / 1000;
		wiphy->max_sched_scan_plan_iterations = U32_MAX;
	}
}

static void morse_mac_config_ieee80211_hw(struct morse *mors, struct ieee80211_hw *hw)
{
	ieee80211_hw_set(hw, SIGNAL_DBM);
	ieee80211_hw_set(hw, MFP_CAPABLE);
	ieee80211_hw_set(hw, REPORTS_TX_ACK_STATUS);

	ieee80211_hw_set(hw, AMPDU_AGGREGATION);

#if KERNEL_VERSION(4, 10, 0) < MAC80211_VERSION_CODE
	if (MORSE_CAPAB_SUPPORTED(&mors->capabilities, HW_FRAGMENT))
		ieee80211_hw_set(hw, SUPPORTS_TX_FRAG);
#endif

	if (!enable_mac80211_connection_monitor)
		ieee80211_hw_set(hw, CONNECTION_MONITOR);

	ieee80211_hw_set(hw, HOST_BROADCAST_PS_BUFFERING);

	if (morse_mac_ps_enabled(mors)) {
		ieee80211_hw_set(hw, SUPPORTS_PS);
		/* Wait for a DTIM beacon - i.e in 802.11ah the long beacon, before associating */
		ieee80211_hw_set(hw, NEED_DTIM_BEFORE_ASSOC);
		if (enable_dynamic_ps_offload)
			ieee80211_hw_set(hw, SUPPORTS_DYNAMIC_PS);
		else
			ieee80211_hw_set(hw, PS_NULLFUNC_STACK);
	}

#ifdef CONFIG_MORSE_RC
	ieee80211_hw_set(hw, HAS_RATE_CONTROL);
#endif

	/* 802.11s requires per mesh STA GTK support */
	ieee80211_hw_set(hw, SUPPORTS_PER_STA_GTK);
#if KERNEL_VERSION(5, 1, 0) < LINUX_VERSION_CODE
	ieee80211_hw_set(hw, SUPPORTS_MULTI_BSSID);
#endif

#ifdef MORSE_MAC80211_S1G_FEATURE_NDP_BLOCKACK
	ieee80211_hw_set(hw, SUPPORTS_NDP_BLOCKACK);
#endif

#ifdef MORSE_MAC_CONFIG_IEEE80211_HW
	ieee80211_hw_set(hw, AP_LINK_PS);
	ieee80211_hw_set(hw, SPECTRUM_MGMT);
	ieee80211_hw_set(hw, SUPPORT_FAST_XMIT);
	ieee80211_hw_set(hw, WANT_MONITOR_VIF);
	ieee80211_hw_set(hw, CHANCTX_STA_CSA);
	ieee80211_hw_set(hw, QUEUE_CONTROL);

	ieee80211_hw_set(hw, SW_CRYPTO_CONTROL);
	ieee80211_hw_set(hw, TX_AMPDU_SETUP_IN_HW);
#endif
}

static void morse_reset_work(struct work_struct *work)
{
	struct morse *mors = container_of(work, struct morse,
					  reset);

	MORSE_INFO(mors, "Resetting Bus...\n");
	morse_bus_reset(mors);
}

static void morse_mac_quiesce_before_restart(struct morse *mors)
{
	if (is_fullmac_mode()) {
		morse_wiphy_stop(mors);
		return;
	}

	ieee80211_stop_queues(mors->hw);
}

static void morse_mac_cleanup_during_restart(struct morse *mors)
{
	u16 if_idx;

	for (if_idx = 0; if_idx < mors->max_vifs; if_idx++) {
		struct ieee80211_vif *vif = morse_get_vif_from_vif_id(mors, if_idx);
		struct morse_vif *mors_vif;
		bool deinit_beacon = false;

		if (!vif)
			continue;

		mors_vif = ieee80211_vif_to_morse_vif(vif);
		switch (vif->type) {
		case NL80211_IFTYPE_AP:
			deinit_beacon = morse_mbssid_ie_enabled(mors) ?
			    (mors_vif->id == mors_vif->mbssid_info.transmitter_vif_id) : true;
			if (deinit_beacon)
				morse_beacon_finish(mors_vif);
			morse_ndp_probe_req_resp_finish(mors_vif);
			morse_bss_stats_deinit(mors_vif);
			morse_twt_finish_vif(mors, mors_vif);
			morse_raw_finish(mors_vif);
			break;
		case NL80211_IFTYPE_ADHOC:
		case NL80211_IFTYPE_MESH_POINT:
			morse_beacon_finish(mors_vif);
			break;
		case NL80211_IFTYPE_STATION:
			morse_send_probe_req_finish(vif);
			morse_twt_finish_vif(mors, mors_vif);
			break;
		default:
			MORSE_WARN_ON(FEATURE_ID_DEFAULT, 1);
			break;
		}
		/* Restart HW will re-add all the interfaces, so clear out all the old references */
		morse_vif_remove(mors, if_idx);
	}

	/* Stop scanning (if running). mac80211 will complain
	 * if this is not done prior to calling `ieee80211_restart_hw()`
	 */
	morse_hw_sched_scan_finish(mors);
	morse_hw_scan_finish(mors);
}

static void morse_mac_restore_after_restart(struct morse *mors)
{
	if (is_fullmac_mode()) {
		morse_wiphy_restarted(mors);
		return;
	}

	ieee80211_restart_hw(mors->hw);
}

static int morse_mac_restart(struct morse *mors)
{
	int ret;
	u32 chip_id;
	const bool reset_hw = true;
	const bool reattach_hw = false;

	dev_warn(mors->dev, "%s: Restarting HW", __func__);
	lockdep_assert_held(&mors->lock);

	/* Clear started flag to prevent already queued work items (internal/mac80211)
	 * from accessing the chip during restart.
	 */
	mors->started = false;

	/* Disable powersave before starting restart work */
	morse_ps_disable(mors);
	/* Stop rx by disabling chip to driver interrupts and clearing
	 * irq & pending event flags.
	 */
	morse_bus_set_irq(mors, false);
	morse_hw_irq_clear(mors);
	/* Stop Tx */
	morse_mac_quiesce_before_restart(mors);

	set_bit(MORSE_STATE_FLAG_DATA_TX_STOPPED, &mors->state_flags);
	set_bit(MORSE_STATE_FLAG_DATA_QS_STOPPED, &mors->state_flags);

#if KERNEL_VERSION(5, 9, 0) <= MAC80211_VERSION_CODE
	if (enable_airtime_fairness)
		tasklet_kill(&mors->tasklet_txq);
#endif

	/* Allow time for in-transit tx/rx packets to settle */
	mdelay(20);
	cancel_work_sync(&mors->chip_if_work);
	cancel_work_sync(&mors->tx_stale_work);
	mors->chip_if->event_flags = 0;
	mors->cfg->ops->flush_tx_data(mors);
	if (mors->cfg->ops->flush_cmds)
		mors->cfg->ops->flush_cmds(mors);

	if (test_and_clear_bit(MORSE_STATE_FLAG_DO_COREDUMP, &mors->state_flags)) {
		dev_warn(mors->dev, "%s: Generating core-dump (reason:%s)", __func__,
			morse_coredump_reason_to_str(mors->coredump.crash.reason));

		ret = morse_coredump(mors);
		if (ret)
			MORSE_ERR(mors, "%s: Core-dump failed (errno:%d)", __func__, ret);

		if (mors->user_coredump_comp)
			complete(mors->user_coredump_comp);

		MORSE_INFO(mors, "%s: Core-dump created", __func__);
	}

	morse_claim_bus(mors);
	ret = morse_reg32_read(mors, MORSE_REG_CHIP_ID(mors), &chip_id);
	morse_release_bus(mors);

	if (ret < 0) {
		MORSE_ERR(mors, "%s: Failed to access HW (errno:%d)", __func__, ret);
		goto exit;
	}

	if (is_fullmac_mode())
		morse_wiphy_cleanup(mors);
	else
		morse_mac_cleanup_during_restart(mors);

	/* reload the firmware */
	ret = morse_firmware_prepare_and_init(mors, reset_hw, reattach_hw);
	if (ret < 0) {
		MORSE_ERR(mors, "%s: Failed to execute NDR (errno:%d)", __func__, ret);
		goto exit;
	}

	morse_bus_set_irq(mors, true);
	/* Allow TX again before exiting */
	clear_bit(MORSE_STATE_FLAG_DATA_TX_STOPPED, &mors->state_flags);
	clear_bit(MORSE_STATE_FLAG_DATA_QS_STOPPED, &mors->state_flags);
	clear_bit(MORSE_STATE_FLAG_CHIP_UNRESPONSIVE, &mors->state_flags);

	morse_mac_restore_after_restart(mors);

exit:
	morse_ps_enable(mors);
	return ret;
}

#if KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE
static void morse_stale_tx_status_timer(unsigned long addr)
{
	struct morse *mors = (struct morse *)addr;
#else
static void morse_stale_tx_status_timer(struct timer_list *t)
{
	struct morse *mors = from_timer(mors, t, stale_status.timer);
#endif

	if (!mors || !mors->stale_status.enabled)
		return;

	spin_lock_bh(&mors->stale_status.lock);

	if (mors->cfg->ops->skbq_get_tx_status_pending_count(mors))
		queue_work(mors->net_wq, &mors->tx_stale_work);

	spin_unlock_bh(&mors->stale_status.lock);
}

static int morse_stale_tx_status_timer_init(struct morse *mors)
{
	MORSE_WARN_ON(FEATURE_ID_DEFAULT, mors->stale_status.enabled);

	spin_lock_init(&mors->stale_status.lock);

	mors->stale_status.enabled = 1;

#if KERNEL_VERSION(4, 14, 0) > LINUX_VERSION_CODE
	init_timer(&mors->stale_status.timer);
	mors->stale_status.timer.data = (unsigned long)mors;
	mors->stale_status.timer.function = morse_stale_tx_status_timer;
	add_timer(&mors->stale_status.timer);
#else
	timer_setup(&mors->stale_status.timer, morse_stale_tx_status_timer, 0);
#endif

	return 0;
}

static int morse_stale_tx_status_timer_finish(struct morse *mors)
{
	if (!mors->stale_status.enabled)
		return 0;

	mors->stale_status.enabled = 0;

	del_timer_sync(&mors->stale_status.timer);

	return 0;
}

/* Schedule the restart work from wherever a code restart
 * is deemed necessary. This can be triggered directly from the debugfs
 * or will be scheduled indirectly from a watchdog timeout.
 */
static void morse_mac_restart_work(struct work_struct *work)
{
	int ret;
	struct morse *mors = container_of(work, struct morse,
					  driver_restart);

	mors->restart_counter++;

	mutex_lock(&mors->lock);
	morse_watchdog_pause(mors);
	ret = morse_mac_restart(mors);

	if (!ret) {
		morse_watchdog_resume(mors);
		MORSE_INFO(mors, "%s: HW restart success (count:%d)",
				   __func__, mors->restart_counter);
	} else {
		MORSE_ERR(mors, "%s: HW restart failed (errno:%d, count:%d)",
				  __func__,
				  ret,
				  mors->restart_counter);

		/* FW restart failed, will need a reset */
		if (enable_watchdog_reset) {
			/* Driver will request to reset the bus.
			 * This should remove/re-install the driver
			 */
			schedule_work(&mors->reset);
		} else {
			/* Offload removing driver to user space */
			mors->reset_required = 1;
			morse_watchdog_cleanup(mors);
		}

		/* Flag to lingering/racy mac80211 callbacks that we are no longer started and they
		 * need to abort.
		 */
		mors->started = false;

		/* Stopping sched scan */
		morse_hw_sched_scan_finish(mors);
	}

	mutex_unlock(&mors->lock);
}

static void morse_health_check_work(struct work_struct *work)
{
	int ret;
	int retries = 0;
	struct morse *mors = container_of(work, struct morse, health_check);

	mutex_lock(&mors->lock);

	if (!mors->started)
		goto out;

	if (test_bit(MORSE_STATE_FLAG_HOST_TO_CHIP_CMD_BLOCKED, &mors->state_flags))
		goto out;

	do {
		ret = morse_cmd_health_check(mors);
	} while (ret && retries++ < MORSE_HEALTH_CHECK_RETRIES);

	if (ret) {
		MORSE_ERR(mors, "%s: Failed health check (errno=%d)\n", __func__, ret);

		if (!morse_coredump_new(mors, MORSE_COREDUMP_REASON_HEALTH_CHECK_FAILED))
			set_bit(MORSE_STATE_FLAG_DO_COREDUMP, &mors->state_flags);

		morse_mac_driver_restart(mors);
	} else {
		MORSE_DBG(mors, "Health check complete\n");
	}

out:
	mutex_unlock(&mors->lock);
}

static int morse_mac_ping_health_check(struct morse *mors)
{
	schedule_work(&mors->health_check);
	MORSE_DBG(mors, "Scheduled a health check\n");

	return 0;
}

int morse_mac_watchdog_create(struct morse *mors)
{
	return morse_watchdog_init(mors, watchdog_interval_secs,
							   morse_mac_ping_health_check);
}

static int morse_ieee80211_init(struct morse *mors)
{
	int i;
	struct ieee80211_hw *hw = mors->hw;

	/* Initialise supported bands */
	for (i = 0; i < NUM_NL80211_BANDS; i++)
		hw->wiphy->bands[i] = NULL;

	hw->wiphy->bands[NL80211_BAND_5GHZ] = &mors_band_5ghz;

	hw->wiphy->interface_modes =
	    BIT(NL80211_IFTYPE_AP) |
	    BIT(NL80211_IFTYPE_STATION) |
#if MESH_CONFIG_ENABLED(MAC80211_MESH)
	    BIT(NL80211_IFTYPE_MESH_POINT) |
#endif
	    BIT(NL80211_IFTYPE_ADHOC);

	hw->extra_tx_headroom = sizeof(struct morse_buff_skb_header) +
			mors->bus_ops->bulk_alignment +
			mors->extra_tx_offset;
	hw->queues = 4;
	/* Limit the number of aggregations for SPI. May get overwhelmed by SDIO */
	if (max_aggregation_count)
		hw->max_rx_aggregation_subframes = max_aggregation_count;
	hw->max_rates = max_rates;	/* We support 4 rates */
	hw->max_report_rates = max_rates;	/* We support 4 rates */
	hw->max_rate_tries = max_rate_tries;
	hw->vif_data_size = sizeof(struct morse_vif);
	hw->sta_data_size = sizeof(struct morse_sta);

	mors->wiphy = hw->wiphy;

	/*
	 * Avoid adding kernel version check for hw->tx_sk_pacing_shift
	 * for kernel < linux-4.20.0. tx_sk_pacing_shift with tcp smaller
	 * queues is required to achieve sufficient throughput in TCP. For
	 * kernels < 4.20.0 apply TCP small queue patches to kernel and add
	 * "tx_sk_pacing_shift" variable to "struct ieee80211_hw".
	 * Ref: https://lwn.net/Articles/507065/
	 *      https://lwn.net/Articles/757643/
	 */
	hw->tx_sk_pacing_shift = SK_PACING_SHIFT;

	SET_IEEE80211_PERM_ADDR(hw, mors->macaddr);
	morse_mac_config_ieee80211_hw(mors, hw);

	return 0;
}

static int morse_set_regdomain(struct morse *mors, const char *country_code)
{
	const struct morse_regdomain *morse_regdom;
	struct ieee80211_regdomain *regdom;
	int ret;

	/* Set regulatory rules to support channels for the country=alpha */
	morse_regdom = morse_reg_set_alpha(country_code);
	if (!morse_regdom) {
		MORSE_ERR(mors, "Country code %s is not supported\n", country_code);
		ret = -EINVAL;
		goto err;
	}

	MORSE_INFO(mors, "Setting regulatory domain to %s", morse_regdom->alpha2);

	regdom = morse_regdom_to_ieee80211(morse_regdom);
	ret = regulatory_set_wiphy_regd(mors->wiphy, regdom);
	kfree(regdom);

	if (ret)
		goto err;

	/* Update the country code */
	memcpy(mors->country, morse_regdom->alpha2, sizeof(mors->country));

	/* give the regulatory workqueue a chance to run */
	schedule_timeout_interruptible(1);
err:
	return ret;
}

static void morse_reg_notifier(struct wiphy *wiphy, struct regulatory_request *request)
{
	struct morse *mors = morse_wiphy_to_morse(wiphy);
	char *req_cc;
	int i;

	if (test_bit(MORSE_STATE_FLAG_REGDOM_SET_BY_OTP, &mors->state_flags)) {
		MORSE_WARN(mors, "Regulatory change ignored, hardware restricted to %c%c\n",
			   mors->country[0], mors->country[1]);
		return;
	}

	MORSE_INFO(mors, "Regulatory notification received on %s\n", wiphy_name(wiphy));

	if (request->initiator != NL80211_REGDOM_SET_BY_USER ||
	    request->user_reg_hint_type != NL80211_USER_REG_HINT_USER) {
		MORSE_INFO(mors, "Unsupported regulatory notification from %s\n",
			   reg_initiator_name(request->initiator));
		return;
	}

	for (i = 0; i < mors->max_vifs; i++) {
		if (morse_mac_is_iface_ap_type(morse_get_vif_from_vif_id(mors, i))) {
			/*
			 * Do not support changing regulatory whilst running as an AP type,
			 * as userspace will require a config change
			 */
			MORSE_WARN(mors,
				   "Ignoring regulatory domain change whilst running as an AP type\n");
			return;
		}
	}

	/* If unspecified (ZZ) or world regdom (00), fall back to using the country specified
	 * by the module parameter
	 */
	if (country_codes_are_equal(request->alpha2, "ZZ") ||
	    country_codes_are_equal(request->alpha2, "00"))
		req_cc = country;
	else
		req_cc = request->alpha2;

	/* regdom has not changed, do nothing */
	if (country_codes_are_equal(req_cc, mors->country))
		return;

	if (morse_set_regdomain(mors, req_cc)) {
		MORSE_ERR(mors, "Failed to set regulatory to country %c%c, staying in %c%c\n",
			  request->alpha2[0], request->alpha2[1],
			  mors->country[0], mors->country[1]);
		return;
	}

	set_bit(MORSE_STATE_FLAG_REGDOM_SET_BY_USER, &mors->state_flags);

	if (mors->started) {
		MORSE_INFO(mors, "Scheduling chip restart to apply regulatory changes\n");
		morse_mac_driver_restart(mors);
	} else {
		/* Driver has not started yet. Set a flag to trigger a reload after everything
		 * has been properly initialised.
		 */
		set_bit(MORSE_STATE_FLAG_RELOAD_FW_AFTER_START, &mors->state_flags);
	}
}

static int morse_mac_init(struct morse *mors)
{
	int ret;

	if (enable_ps != POWERSAVE_MODE_FULLY_ENABLED) {
		MORSE_ERR(mors,
			  "%s: WARNING enable_ps modparam must only be used for testing - use iw set power_save\n",
			  __func__);
	}

	mors->max_vifs = MORSE_MAX_IF;

	if (is_thin_lmac_mode()) {
		MORSE_INFO(mors, "%s: Enabling thin LMAC mode\n", __func__);
		if (is_virtual_sta_test_mode()) {
			MORSE_ERR(mors, "%s: Virtual STA test mode is set but ignored\n",
				  __func__);
			virtual_sta_max = 0;
		}
	} else if (is_virtual_sta_test_mode()) {
		MORSE_INFO(mors, "%s: Enabling virtual STA test mode - max %d STAs\n",
			   __func__, virtual_sta_max);
		mors->max_vifs = virtual_sta_max;

		if (enable_ps != POWERSAVE_MODE_DISABLED) {
			MORSE_ERR(mors, "%s: Disabling power save in virtual STA test mode\n",
				  __func__);
			enable_ps = POWERSAVE_MODE_DISABLED;
		}
	}

	if (enable_wiphy)
		ret = morse_wiphy_init(mors);
	else
		ret = morse_ieee80211_init(mors);

	MORSE_WARN_ON(FEATURE_ID_DEFAULT, ret);

	mors->enable_subbands = enable_subbands;
	mors->enable_mbssid_ie = enable_mbssid_ie;
	mors->enable_hw_scan = enable_hw_scan;
	mors->enable_sched_scan = enable_sched_scan;

	if (enable_sgi_rc) {
		if (MORSE_CAPAB_SUPPORTED(&mors->capabilities, SGI)) {
			mors->custom_configs.enable_sgi_rc = true;
		} else {
			enable_sgi_rc = false;
			mors->custom_configs.enable_sgi_rc = false;
			MORSE_ERR(mors,
				  "%s: SGI has been configured but is not supported by this device. Ignoring.\n",
				  __func__);
		}
	} else {
		mors->custom_configs.enable_sgi_rc = false;
	}

	if (enable_trav_pilot) {
		if (MORSE_CAPAB_SUPPORTED(&mors->capabilities, TRAVELING_PILOT_ONE_STREAM) ||
		    MORSE_CAPAB_SUPPORTED(&mors->capabilities, TRAVELING_PILOT_TWO_STREAM)) {
			mors->custom_configs.enable_trav_pilot = true;
		} else {
			enable_trav_pilot = false;
			mors->custom_configs.enable_trav_pilot = false;
			MORSE_ERR(mors,
				  "%s: Travelling pilots configured but not supported - ignoring\n",
				  __func__);
		}
	} else {
		mors->custom_configs.enable_trav_pilot = false;
	}

	if (duty_cycle_probe_retry_threshold > MORSE_DUTY_CYCLE_PROBE_RETRY_THRESHOLD_MAX) {
		MORSE_ERR(mors,
			  "%s: Duty Cycle probe retry threshold (%d) is invalid",
			  __func__, duty_cycle_probe_retry_threshold);
		duty_cycle_probe_retry_threshold = MORSE_DUTY_CYCLE_PROBE_RETRY_THRESHOLD;
	}

#ifdef CONFIG_MORSE_RC
	/* Initial value for RTS threshold */
	mors->rts_threshold = IEEE80211_MAX_RTS_THRESHOLD;
#endif

#if KERNEL_VERSION(4, 10, 0) > MAC80211_VERSION_CODE
	/* Older kernels decide whether to do fragmentation based on the existence
	 * of this callback
	 */
	if (!MORSE_CAPAB_SUPPORTED(&mors->capabilities, HW_FRAGMENT))
		mors_ops.set_frag_threshold = NULL;
#endif

	/* Initialise hardware scan */
	morse_hw_scan_init(mors);

	/* Remove ops callbacks if user / chip does not support hw scan */
	if (!hw_scan_is_supported(mors)) {
		MORSE_INFO(mors, "Disabling HW scan\n");
		mors_ops.hw_scan = NULL;
		mors_ops.cancel_hw_scan = NULL;
	}

	/* Remove ops callbacks if user / chip does not support scheduled scan */
	if (!hw_scan_is_supported(mors) || !sched_scan_is_supported(mors)) {
		MORSE_INFO(mors, "Disabling scheduled scan\n");
		mors_ops.sched_scan_start = NULL;
		mors_ops.sched_scan_stop = NULL;
	}

	/* Initial channel information when chip first boots */
	mors->custom_configs.default_bw_info.pri_bw_mhz = 2;
	mors->custom_configs.default_bw_info.pri_1mhz_chan_idx = 0;
	mors->custom_configs.default_bw_info.op_bw_mhz = 2;
	/* Frequency is special - we don't necessarily know what freq will be */
	/* Initial values for sta_type and enc_mode */
	mors->custom_configs.sta_type = STA_TYPE_NON_SENSOR;
	mors->custom_configs.enc_mode = ENC_MODE_BLOCK;

	/* Get supported MCS rates (TX/RX) from modparam */
	mors_band_5ghz.ht_cap.mcs.rx_mask[0] = mcs_mask;

	mors->vif = kcalloc(mors->max_vifs, sizeof(*mors->vif), GFP_KERNEL);

	mors->mon_if.id = INVALID_VIF_ID;

	/* Fullmac configures its wiphy structure in morse_wiphy_init(), called above.
	 * In softmac we need to do it here.
	 */
	if (!is_fullmac_mode())
		morse_mac_config_wiphy(mors);

	morse_mac_config_ht_cap(mors);
	morse_mac_config_vht_base_cap(mors);

	/* 4 and 8MHz parts use VHT 80 and 160 respectively */
	if (MORSE_CAPAB_SUPPORTED(&mors->capabilities, 4MHZ))
		morse_mac_config_vht_80_cap(mors);

	if (MORSE_CAPAB_SUPPORTED(&mors->capabilities, 8MHZ))
		morse_mac_config_vht_160_cap(mors);

	morse_stale_tx_status_timer_init(mors);

	ret = morse_ps_init(mors, (enable_ps != POWERSAVE_MODE_DISABLED),
			    enable_dynamic_ps_offload);
	if (enable_ps != POWERSAVE_MODE_FULLY_ENABLED) {
		/* SW-2638:
		 * We do not have GPIO pins connected, let's disable the host-to-chip PS mechanism,
		 * that is by incrementing the number of wakers by one
		 */
		morse_ps_disable(mors);
	}

	MORSE_WARN_ON(FEATURE_ID_DEFAULT, ret);

#if KERNEL_VERSION(5, 9, 0) <= MAC80211_VERSION_CODE
	if (enable_airtime_fairness)
		tasklet_setup(&mors->tasklet_txq, morse_txq_tasklet);
#endif

	mors->tx_power_mbm = INT_MAX;
	mors->tx_max_power_mbm = INT_MAX;

	morse_mesh_config_list_init(mors);

#ifdef CONFIG_MORSE_HW_TRACE
	morse_hw_trace_init();
#endif

#ifdef CONFIG_MORSE_VENDOR_COMMAND
	/* register vendor commands and events */
	morse_set_vendor_commands_and_events(mors->wiphy);
#endif

	mors->num_of_ap_interfaces = 0;
	init_waitqueue_head(&mors->beacon_tasklet_waitq);
	mors->beacon_queued = false;

	return 0;
}

int morse_mac_register(struct morse *mors)
{
	int ret;
	struct ieee80211_hw *hw = mors->hw;

	/* Pass debug_mask modparam to dot11ah module */
	morse_dot11ah_debug_init(debug_mask);

	ret = morse_mac_init(mors);
	if (ret) {
		MORSE_ERR(mors, "morse_mac_init failed %d\n", ret);
		goto err;
	}

	morse_led_init(mors);

	/* We manage our own regdb, as Linux has no S1G support yet */
	mors->wiphy->regulatory_flags = REGULATORY_WIPHY_SELF_MANAGED;
	mors->wiphy->reg_notifier = morse_reg_notifier;

	/* Register with mac80211 */
	if (enable_wiphy)
		ret = morse_wiphy_register(mors);
	else
		ret = ieee80211_register_hw(hw);
	if (ret) {
		MORSE_ERR(mors, "ieee80211_register_hw failed %d\n", ret);
		morse_led_exit(mors);
		goto err;
	}

	/* Set the initial regdomain from the country code, if it has not been set by the regdb yet.
	 * If it has already been set by the regdb, the notifier will have been called
	 */
	if (test_bit(MORSE_STATE_FLAG_REGDOM_SET_BY_OTP, &mors->state_flags)) {
		MORSE_WARN(mors, "Country modparam (%c%c) ignored, hardware restricted to %c%c\n",
			country[0], country[1], mors->country[0], mors->country[1]);
		ret = morse_set_regdomain(mors, mors->country);
		memcpy(country, mors->country, sizeof(country));
	} else if (!test_bit(MORSE_STATE_FLAG_REGDOM_SET_BY_USER, &mors->state_flags)) {
		ret = morse_set_regdomain(mors, country);
	}
	if (ret) {
		MORSE_ERR(mors, "Failed to set regdomain on register %d\n", ret);
		goto err_unregister_hw;
	}

	INIT_WORK(&mors->reset, morse_reset_work);
	INIT_WORK(&mors->driver_restart, morse_mac_restart_work);
	INIT_WORK(&mors->health_check, morse_health_check_work);
	INIT_WORK(&mors->hw_stop, morse_hw_stop_work);

	ret = morse_init_debug(mors);
	if (ret)
		MORSE_ERR(mors, "Unable to create debugfs files\n");

	ret = morse_mac_watchdog_create(mors);
	if (ret) {
		MORSE_ERR(mors, "Failed to create watchdog %d\n", ret);
		goto err_unregister_hw;
	}

	if (enable_watchdog) {
		ret = morse_watchdog_start(mors);
		if (ret) {
			MORSE_ERR(mors, "morse_watchdog_start failed %d\n", ret);
			goto err_unregister_hw;
		}
	}

#ifdef CONFIG_MORSE_MONITOR
	ret = morse_mon_init(mors);
	if (ret) {
		MORSE_ERR(mors, "morse_mon_init failed %d\n", ret);
		goto err_unregister_hw;
	}
#endif

#ifdef CONFIG_MORSE_RC
	ret = morse_rc_init(mors);
	if (ret) {
		MORSE_ERR(mors, "morse_rc_init failed %d\n", ret);
		goto err_free_mon;
	}
	mors->rc_method = MORSE_RC_METHOD_MMRC;
#else
	mors->rc_method = MORSE_RC_METHOD_MINSTREL;
#endif

#ifdef MODULE
	if (log_modparams_on_boot)
		morse_log_modparams(mors);
#endif

	return ret;

#ifdef CONFIG_MORSE_RC
err_free_mon:
#ifdef CONFIG_MORSE_MONITOR
	morse_mon_free(mors);
#endif
#endif

err_unregister_hw:
	morse_led_exit(mors);
	ieee80211_unregister_hw(hw);
err:
	return ret;
}

static struct morse *morse_ieee80211_create(size_t priv_size, struct device *dev)
{
	struct ieee80211_hw *hw;
	struct morse *mors;

#if KERNEL_VERSION(5, 9, 0) <= MAC80211_VERSION_CODE
	if (enable_airtime_fairness)
		mors_ops.wake_tx_queue = morse_mac_ops_wake_tx_queue;
#endif
#if KERNEL_VERSION(6, 2, 0) <= MAC80211_VERSION_CODE
	/* mac80211 has dropped support for TX push path and has fully switched over to the
	 * internal TX queue (iTXQ) implementation, hence wake_tx_queue() callback is now
	 * mandatory
	 */
	else
		mors_ops.wake_tx_queue = ieee80211_handle_wake_tx_queue;
#endif

	/* User disabled HW-crypto - fallback to software crypto */
	/* Encryption and decryption must be done on the host in Thin LMAC mode */
	if (no_hwcrypt || is_thin_lmac_mode())
		mors_ops.set_key = NULL;

	hw = ieee80211_alloc_hw(sizeof(*mors) + priv_size, &mors_ops);
	if (!hw) {
		dev_err(dev, "ieee80211_alloc_hw failed\r\n");
		return NULL;
	}

	SET_IEEE80211_DEV(hw, dev);
	memset(hw->priv, 0, sizeof(*mors));

	mors = hw->priv;
	mors->hw = hw;

	return mors;
}

struct morse *morse_mac_create(size_t priv_size, struct device *dev)
{
	struct morse *mors;

	if (enable_wiphy)
		mors = morse_wiphy_create(priv_size, dev);
	else
		mors = morse_ieee80211_create(priv_size, dev);

	if (!mors)
		return NULL;

	mors->dev = dev;
	mutex_init(&mors->lock);
	mutex_init(&mors->cmd_lock);
	mutex_init(&mors->cmd_wait);
	spin_lock_init(&mors->vif_list_lock);

	/* Initialise coredump structures */
	mutex_init(&mors->coredump.lock);
	INIT_LIST_HEAD(&mors->coredump.crash.memory.regions);

	/* Initialise hw_clock structure */
	mutex_init(&mors->hw_clock.update_wait_lock);

	mors->custom_configs.enable_ampdu = true;
	mors->custom_configs.enable_subbands = enable_subbands;
	mors->custom_configs.enable_arp_offload = enable_arp_offload;
	mors->custom_configs.enable_dhcpc_offload = enable_dhcpc_offload;
	mors->custom_configs.dhcpc_lease_update_script = dhcpc_lease_update_script;

	country[sizeof(country) - 1] = '\0';
	memcpy(mors->country, country, sizeof(mors->country));

#if KERNEL_VERSION(5, 9, 0) <= MAC80211_VERSION_CODE
	mors->custom_configs.enable_airtime_fairness = enable_airtime_fairness;
#else
	mors->custom_configs.enable_airtime_fairness = false;
#endif
	/* TODO: Placeholder for legacy amsdu support */
	mors->custom_configs.enable_legacy_amsdu = (enable_airtime_fairness && false);

	mors->watchdog.paused = 0;
	mors->watchdog.consumers = 0;
	mors->watchdog.ping = NULL;

	/* Initialise pre-association station structure (shared between VIFs) */
	morse_pre_assoc_peer_list_init(mors);

	return mors;
}

static void morse_ieee80211_deinit(struct morse *mors)
{
	ieee80211_stop_queues(mors->hw);
	ieee80211_unregister_hw(mors->hw);
}

static void morse_mac_deinit(struct morse *mors)
{
	struct wiphy *wiphy = mors->wiphy;

	if (enable_watchdog) {
		morse_watchdog_stop(mors);
		cancel_work_sync(&mors->health_check);
	}

	if (enable_wiphy)
		morse_wiphy_deinit(mors);
	else
		morse_ieee80211_deinit(mors);

	mors->cfg->ops->flush_tx_data(mors);
	if (mors->cfg->ops->flush_cmds)
		mors->cfg->ops->flush_cmds(mors);

	morse_mac_clear_mesh_list(mors);
#if KERNEL_VERSION(5, 9, 0) <= MAC80211_VERSION_CODE
	if (enable_airtime_fairness)
		tasklet_kill(&mors->tasklet_txq);
#endif

	if (!is_fullmac_mode() && wiphy->iface_combinations) {
		kfree(wiphy->iface_combinations->limits);
		kfree(wiphy->iface_combinations);
		wiphy->iface_combinations = NULL;
		wiphy->n_iface_combinations = 0;
	}
	kfree(mors->vif);
	mors->vif = NULL;
}

void morse_mac_unregister(struct morse *mors)
{
	morse_led_exit(mors);
	morse_deinit_debug(mors);
	morse_ps_disable(mors);

#ifdef CONFIG_MORSE_RC
	morse_rc_deinit(mors);
#endif
	morse_mac_deinit(mors);

	morse_hw_scan_destroy(mors);
	morse_survey_destroy_usage_records(mors);
	morse_stale_tx_status_timer_finish(mors);
#ifdef CONFIG_MORSE_MONITOR
	morse_mon_free(mors);
#endif
	morse_ps_finish(mors);

#ifdef CONFIG_MORSE_HW_TRACE
	morse_hw_trace_deinit();
#endif
}

static void morse_ieee80211_destroy(struct morse *mors)
{
	ieee80211_free_hw(mors->hw);
}

void morse_mac_destroy(struct morse *mors)
{
	if (enable_watchdog)
		morse_watchdog_cleanup(mors);

	morse_coredump_destroy(mors);

	if (enable_wiphy)
		morse_wiphy_destroy(mors);
	else
		morse_ieee80211_destroy(mors);
}

int morse_mac_get_watchdog_interval_secs(void)
{
	return watchdog_interval_secs;
}

u8 morse_mac_get_mcs10_mode(void)
{
	return mcs10_mode;
}

u16 morse_mac_get_mcs_mask(void)
{
	return mcs_mask;
}
