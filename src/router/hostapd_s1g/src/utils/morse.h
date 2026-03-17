/*
 * Copyright 2022 Morse Micro
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef MORSE_H
#define MORSE_H

#include "utils/includes.h"
#include "utils/common.h"
#include "ap/ap_config.h"
#include "ap/hostapd.h"

#define MORSE_S1G_RETURN_ERROR (-1)
#define MORSE_INVALID_CHANNEL (-2)
#define MORSE_SUCCESS (0)
/** The maximum number of country codes that can be assigned to an S1G class */
#define COUNTRY_CODE_MAX (2)
#define COUNTRY_CODE_LEN (2)
#define S1G_CHAN_ENABLED_FLAG(ch) (1LLU << (ch))
#define NUMBER_OF_BITS(x) (sizeof(x) * 8)

#define MIN_S1G_FREQ_KHZ	750000
#define MAX_S1G_FREQ_KHZ	950000

#define MORSE_JP_HT20_NON_OVERLAP_CHAN_OFFSET 12
#define MORSE_JP_HT20_NON_OVERLAP_CHAN_START 50
#define MORSE_JP_HT20_NON_OVERLAP_CHAN_END 60
#define MORSE_JP_S1G_NON_OVERLAP_CHAN 21

#define S1G_OP_CLASS_IE_LEN 3 /* eid + ie len + op class */
extern const unsigned int S1G_OP_CLASSES_LEN;

/* Define Maximum interfaces supported for MBSSID IE */
#define MBSSID_MAX_INTERFACES 2

#define MORSE_OUI	0x0CBF74

enum morse_vendor_events {
	MORSE_VENDOR_EVENT_BCN_VENDOR_IE_FOUND = 0, /* To be deprecated in a future version */
	MORSE_VENDOR_EVENT_OCS_DONE = 1,
	MORSE_VENDOR_EVENT_MGMT_VENDOR_IE_FOUND = 2,
	MORSE_VENDOR_EVENT_MESH_PEER_ADDR = 3
};

enum morse_vendor_attributes {
	MORSE_VENDOR_ATTR_DATA = 0,
	/* Bitmask of type @ref enum morse_vendor_ie_mgmt_type_flags */
	MORSE_VENDOR_ATTR_MGMT_FRAME_TYPE = 1,
};

struct ah_class {
	u32 s1g_freq_start;
	u8 s1g_op_class;
	/* Array index to select class based on regdomain */
	u8 s1g_op_class_idx;
	u8 global_op_class;
	u8 s1g_width;
	char cc_list[COUNTRY_CODE_MAX][COUNTRY_CODE_LEN];
	u64 chans;
};

struct morse_twt {
	u8 enable;
	u8 flow_id;
	u8 setup_command;
	u32 wake_duration_us;
	u64 wake_interval_us;
	u64 target_wake_time;
};

enum s1g_op_class_type {
	OP_CLASS_INVALID = -1,
	OP_CLASS_S1G_LOCAL = 1,
	OP_CLASS_S1G_GLOBAL = 0,
};

/* This table is also in the Morse Micro driver */
enum morse_dot11ah_region {
	MORSE_AU,
	MORSE_CA,
	MORSE_EU,
	MORSE_GB,
	MORSE_IN,
	MORSE_JP,
	MORSE_KR,
	MORSE_NZ,
	MORSE_SG,
	MORSE_US,
	REGION_UNSET = 0xFF,
};

/* Used to define buffer size when running a morse_cli command */
#define MORSE_CTRL_COMMAND_LENGTH (256)

/**
 * Return the pointer to the start of a container when a pointer within
 * the container is known
 */
#define container_of(ptr, type, member) ({\
	const typeof(((type *)0)->member)*__mptr = (const typeof(((type *)0)->member) *)(ptr); \
	(type *)((char *)__mptr - offsetof(type, member)); })

/**
 * morse_set_s1g_ht_chan_pairs - Set the s1g->ht channelisation pairs
 *
 * @cc: The country code for the required channelisation scheme.
 *	Passing NULL selects the default scheme (AU/US etc.)
 */
void morse_set_s1g_ht_chan_pairs(const char *cc);

/* Convert ht channel to s1g channel */
int morse_ht_chan_to_s1g_chan(int ht_chan);

/* Convert ht frequency to s1g channel */
int morse_ht_freq_to_s1g_chan(int ht_freq);

/* Convert s1g channel to ht channel */
int morse_s1g_chan_to_ht_chan(int s1g_chan);

/* Convert s1g channel to bandwidth */
int morse_s1g_chan_to_bw(int s1g_chan);

/* RAW limits */
#define MORSE_RAW_MAX_3BIT_SLOTS		(0b111)
#define MORSE_RAW_MIN_SLOT_DUR_US		(500)
#define MORSE_RAW_MAX_SLOT_DUR_US		(MORSE_RAW_MIN_SLOT_DUR_US + (200 * (1 << 11) - 1))
#define MORSE_RAW_MIN_RAW_DUR_US		MORSE_RAW_MIN_SLOT_DUR_US
#define MORSE_RAW_MAX_RAW_DUR_US		(MORSE_RAW_MAX_SLOT_DUR_US * \
							MORSE_RAW_MAX_3BIT_SLOTS)
#define MORSE_RAW_MAX_START_TIME_US		(UINT8_MAX * 2 * 1024)
#define MORSE_RAW_MAX_SLOTS			(63)
#define MORSE_RAW_MAX_PRIORITY			(7)
#define MORSE_RAW_MAX_BEACON_SPREAD		(UINT16_MAX)
#define MORSE_RAW_MAX_NOM_STA_PER_BEACON	(UINT16_MAX)
#define MORSE_RAW_DEFAULT_START_AID		(1)
#define MORSE_RAW_AID_PRIO_SHIFT		(8)
#define MORSE_RAW_AID_DEVICE_MASK		(0xFF)
#define MORSE_MAX_NUM_RAWS_USER_PRIO		(8)	/* Limited by QoS User Priority */
#define MORSE_RAW_ID_HOSTAPD_PRIO_OFFSET	(0x4000)
/* This is an existing limitation which can be removed with native s1g support. */
#define MAX_AID					(2007)

/* Convert an operating class to channel width */
int morse_s1g_op_class_to_ch_width(u8 s1g_op_class);

/* Convert an operating class to country code */
int morse_s1g_op_class_to_country(u8 s1g_op_class, char *cc);

/* Convert a country code to a global operating class */
int morse_s1g_country_to_global_op_class(char *cc);

/* Convert an operating class and s1g channel to frequency (kHz) */
int morse_s1g_op_class_chan_to_freq(u8 s1g_op_class, int s1g_chan);

/* Convert ht channel and s1g operating class to s1g frequency (kHz) */
int morse_s1g_op_class_ht_chan_to_s1g_freq(u8 s1g_op_class, int ht_chan);

/* Convert an operating class and ht frequency into a s1g frequency (kHz) */
int morse_s1g_op_class_ht_freq_to_s1g_freq(u8 s1g_op_class, int ht_freq);

/* Convert a country and ht frequency into a s1g frequency (kHz) */
int morse_cc_ht_freq_to_s1g_freq(char *cc, int ht_freq);

/* Convert a country and S1G frequency (kHz) into an HT frequency (MHz) */
int morse_s1g_freq_and_cc_to_ht_freq(int s1g_frequency, const char *cc);

/* Return the first valid channel from an s1g operating class */
int morse_s1g_op_class_first_chan(u8 s1g_op_class);

/* Returns the center channel, taking into account VHT channel offsets */
int morse_ht_chan_to_ht_chan_center(struct hostapd_config *conf, int ht_chan);

/* Returns the ht channel, taking into account VHT channel offsets */
int morse_ht_center_chan_to_ht_chan(struct hostapd_config *conf, int ht_chan);

/*
 * Verify operating class and country code (no channel).
 * Returns S1G local operating class if valid combination, negative if invalid.
 */
int morse_s1g_verify_op_class_country(u8 s1g_op_class, char *cc, u8 s1g_1mhz_prim_index);

/*
 * Verify operating class, country code and channel.
 * Returns S1G local operating class if valid combination, negative if invalid.
 */
int morse_s1g_verify_op_class_country_channel(u8 s1g_op_class, char *cc, int s1g_chan,
					u8 s1g_1mhz_prim_index);

/* Validate ht centre channel with index and returns corresponding ht channel */
int morse_validate_ht_channel_with_idx(u8 s1g_op_class, int ht_center_channel, int *oper_chwidth,
				int s1g_prim_1mhz_chan_index, struct hostapd_config *conf);

int morse_s1g_validate_csa_params(struct hostapd_iface *iface, struct csa_settings *settings);

/*
 * Map the given S1G frequency in KHz onto the matching 5GHz one in MHz.
 */
int morse_convert_s1g_freq_to_ht_freq(int s1g_freq, const char *country);

/*
 * Get the lowest center frequency for a given country.
 */
int morse_s1g_get_first_center_freq_for_country(char *cc);

/* Return the configured 1 or 2MHz primary channel */
int morse_s1g_get_primary_channel(struct hostapd_config *conf, int bw);

/* Defined in driver/driver/h */
enum wnm_oper;

#ifdef CONFIG_MORSE_WNM
/**
 * Handle wnm_oper from driver.
 *
 * @param ifname	The name of the interface that this operation applies to.
 * @param oper		The operation type.
 *
 * @returns 0 on success, else an error code.
 *
 * @see wpa_driver_ops::wnm_oper
 */
int morse_wnm_oper(const char *ifname, enum wnm_oper oper);
#endif

/**
 * Execute a morse_cli command line.
 *
 * @param iface		Optional interface name. If specified then "-i <iface>" will be included in
 *			the command line.
 * @param args_fmt	printf format string for morse_cli arguments.
 * @param ...		Variadic arguments for format string.
 *
 * @returns 0 on success, else an error code.
 */
int morse_cli(const char *iface, const char *args_fmt, ...);

/**
 * Issue a Morse control command to enable or disable long sleep (i.e., sleep through DTIMs).
 *
 * @param iface		The name of the interface (e.g., wlan0)
 * @param enabled	Boolean indicating whether long sleep should be enabled or not.
 *
 * @returns 0 on success, else an error code.
 */
int morse_set_long_sleep_enabled(const char *iface, bool enabled);

int morse_s1g_op_class_first_chan(u8 s1g_op_class);

/**
 * Issue a morse_cli command to set the S1G operating class for the S1G operating element in
 * management frames
 *
 * @param ifname	The name of the interface (e.g., wlan0)
 * @param opclass	The S1G operating class
 *
 * @returns 0 on success, else an error code.
 */
int morse_set_s1g_op_class(const char* ifname, u8 opclass, u8 prim_opclass);

/**
 * Issue a morse_cli command to set the channel parameters
 *
 * @param ifname		The name of the interface (e.g., wlan0)
 * @param oper_freq		Operating center frequency in KHz
 * @param oper_chwidth		Operating bandwidth in MHz (1, 2, 4, 8)
 * @param prim_chwidth		Primary channel width in MHz (1, 2)
 * @param prim_1mhz_ch_idx	Primary 1MHz channel index (0-7 for 8MHz BW, 0-3 for 4MHz,
 *				0-1 for 2MHz, and 0 for 1MHz)
 *
 * @returns 0 on success, else an error code.
 */
int morse_set_channel(const char* ifname, int oper_freq, int oper_chwidth, u8 prim_chwidth,
			u8 prim_1mhz_ch_idx);

/**
 * Issue a morse_cli command to store session information after succesful association
 *
 * @param ifname	The name of the interface (e.g., wlan0)
 * @param bssid		The BSSID for the association
 * @param dir		The directory path for storing Standby session information
 */
void morse_standby_session_store(const char* ifname, const u8* bssid,
	const char* standby_session_dir);

/**
 * Classify an operating class number as S1G local, global or invalid.
 * Also returns a mapping for local operating classes (if ch_map NOT NULL).
 *
 * @param s1g_op_class	The S1G operating class
 * @param class		ah_class structure
 *
 * @returns OP_CLASS_INVALID, OP_CLASS_S1G_LOCAL or OP_CLASS_S1G_GLOBAL based on the the op_class
 */
enum s1g_op_class_type morse_s1g_op_class_valid(u8 s1g_op_class, const struct ah_class **class);

/**
 * Fill the current operating class and list of supported operating class
 * ie parameters
 *
 * @param buf				Pointer to the buffer for filling ies
 * @param cc				country code
 * @param s1g_chwidth			Operating bandwidth for operating channel
 * @param s1g_op_chan			S1G Operating Channel
 *
 * @returns MORSE_SUCCESS on success, MORSE_S1G_RETURN_ERROR on failure
 */
int morse_insert_supported_op_class(struct wpabuf *buf, char *cc, int s1g_chwidth, int s1g_op_chan);

#ifdef CONFIG_MORSE_KEEP_ALIVE_OFFLOAD

/**
 * Issue a morse_cli command to set/offload the bss keep-alive frames.
 *
 * @param iface			The name of the interface (e.g., wlan0)
 * @param bss_max_idle_period	The BSS max idle period as derived directly
 *				from the WLAN_EID_BSS_MAX_IDLE_PERIOD
 * @param as_11ah		Intepret BSS max idle period as per the 11ah spec.
 *
 * @returns 0 on success, else an error code.
 */
int morse_set_keep_alive(const char* ifname, u16 bss_max_idle_period, bool as_11ah);

#endif /* CONFIG_MORSE_KEEP_ALIVE_OFFLOAD */

int morse_twt_conf(const char* ifname, struct morse_twt *twt_config);

/**
 * Issue a morse_cli command to set ecsa channel parameters
 *
 * @param ifname		The name of the interface (e.g., wlan0)
 * @param global_oper_class	Global operating class for the operating country
 *				and operating bandwidth (eg: for AU 68, 69, 70, 71)
 * @param prim_chwidth		Primary channel width in MHz (1, 2)
 * @param oper_chwidth		Operating channel width in MHz (1, 2, 4, 8)
 * @param oper_freq		Frequency of operating channel in kHz
 * @param prim_1mhz_ch_idx	1MHz channel index of primary channel
 * @param prim_global_op_class	Global operating class for primary channel
 * @param s1g_capab  User configured S1G capabilities.
 *
 * @returns 0 on success, else an error code.
 */
int morse_set_ecsa_params(const char* ifname, u8 global_oper_class, u8 prim_chwidth,
			int oper_chwidth, int oper_freq, u8 prim_1mhz_ch_idx,
			u8 prim_global_op_class, u32 s1g_capab);

int morse_set_mbssid_info(const char *ifname, const char *tx_iface_idx,
					u8 max_bss_index);
/**
 * Issue a morse_cli command to enable or disable CAC
 *
 * @param ifname		The name of the interface (e.g., wlan0)
 * @param enable		True to enable CAC, false to disable CAC
 *
 * @returns 0 on success, else an error code.
 */
int morse_cac_conf(const char* ifname, bool enable);

/**
 * Derives primary s1g channel
 *
 * @param op_bw_mhz		Operating bandwidth in MHz
 * @param pr_bw_mhz		Primary bandwidth in MHz
 * @param s1g_op_chan		S1G channel for operating center frequency
 * @param prim_1mhz_ch_idx	1MHz channel index of primary channel
 *
 * @returns derived channel on success, else error code.
 */
int morse_calculate_primary_s1g_channel(int op_bw_mhz, int pr_bw_mhz, int s1g_op_chan,
					int pr_1mhz_chan_idx);

/**
 * Derives class for s1g channel based on channel width
 *
 * @param s1g_bw		Primary/Operating band in MHz
 * @param cc			Pointer to country code
 * @param s1g_chan		S1G primary/operating channel
 */
const struct ah_class *morse_s1g_ch_to_op_class(u8 s1g_bw, char *cc, int s1g_chan);

/**
 * For "JP" get offset value to derive HT channel
 *
 * @param chan			S1G/HT primary channel
 * @param primary_1MHz_chan	S1G primary 1MHz channel for s1g channel, invalid otherwise
 * @param ht			Flag to select HT/S1G channel as input
 */
int morse_ht_chan_offset_jp(int chan, int primary_1MHz_chan, int ht);

/**
 * Derive primary channel for configured channel and bw in a country
 *
 * @param op_bw_mhz		Operating band in MHz
 * @param pr_bw_mhz		Primary band in MHz
 * @param pr_1mhz_chan_idx	Primary 1MHZ channel index
 * @param cc			Pointer to country code
 */
int morse_cc_get_primary_s1g_channel(int op_bw_mhz, int pr_bw_mhz,
		int s1g_op_chan, int pr_1mhz_chan_idx, char *cc);

/**
 * Derive primary 1MHz channel for "JP"
 *
 * @param op_bw_mhz		Operating band in MHz
 * @param pr_bw_mhz		Primary band in MHz
 * @param s1g_op_chan		S1G Operating channel
 * @param pr_1mhz_chan_idx	Primary 1MHz channel index
 */
int morse_calculate_primary_s1g_channel_jp(int op_bw_mhz, int pr_bw_mhz, int s1g_op_chan,
						int pr_1mhz_chan_idx);

/**
 * Get the secondary channel offset to derive primary 2MHz channel
 * from 1MHz primary channel for primary bw 2MHz
 *
 * @param sec_chan_offset	Secondary channel offset flag
 * @param cc			Pointer to country code
 */
int morse_cc_get_sec_channel_offset(int sec_chan_offset, char *cc);

/**
 * Remove duplicate entries and sort buffer
 *
 * @param buf			Pointer to buffer to be sorted
 * @param buf_offset		Offset at which buffer should start sorting
 */
int morse_remove_duplicates_and_sort_buf(struct wpabuf *buf, int buf_offset);

/**
 * Pass the Mesh configuration parameters to driver
 *
 * @param ifname	The name of the interface (e.g., mesh0)
 * @param mesh_id	Mesh ID for the mesh interface
 * @param mesh_id_len	Length of Mesh ID
 * @param beaconless_mode Beaconless mode
 * @param max_plinks	maximum number of peer links
 */
int morse_set_mesh_config(const char *ifname, u8 *mesh_id, u8 mesh_id_len, u8 beaconless_mode,
	u8 max_plinks);

/**
 * Configure MBCA parameters
 *
 * @param ifname	The name of the interface (e.g., mesh0)
 * @param mbca_config	MBCA Configuration
 * @param min_beacon_gap	Minimum gap between our's and neighbor beacon.
 * @param tbtt_adj_interval	TBTT adjustment interval.
 * @param beacon_timing_report_int	Beacon Timing report interval.
 * @param mbss_start_scan_duration	Initial scan duration to find other peers in MBSS.
 *
 * @returns 0 on success, else an error code.
 */
int morse_mbca_conf(const char *ifname, u8 mbca_config, u8 min_beacon_gap, u8 tbtt_adj_interval,
	u8 beacon_timing_report_interval, u16 mbss_start_scan_duration);

/**
 * Configure Mesh Dynamic Peering parameters
 *
 * @param ifname	The name of the interface (e.g., mesh0)
 * @param enabled	True to enable dynamic peering, false to disable dynamic peering
 * @param rssi_margin	RSSI margin to consider while selecting a peer to kickout.
 * @param blacklist_timeout	Duration in seconds, a kicked out peer is blacklisted.
 *
 * @returns 0 on success, else an error code.
 */
int morse_set_mesh_dynamic_peering(const char *ifname, bool enabled, u8 rssi_margin,
	u32 blacklist_timeout);


/**
 * Globally enable / disable RAW
 *
 * @param ifname the name of the interface
 * @param enable true to enable RAW, false to disable RAW
 *
 * @returns 0 on success, else error code
 */
int morse_raw_global_enable(const char *ifname, bool enable);

/**
 * Enable or disable RAW priorities.
 *
 * @param ifname the name of the interface
 * @param enable true to enable the priority, false to disable
 * @param prio index of the priority
 * @param start_time_us Start time from last beacon or RAW
 * @param duration_us RAW duration time
 * @param num_slots number of slots
 * @param cross_slot Cross slot boundary bleed allowed
 * @param max_bcn_spread maximum beacons to spread over (0 for no limit)
 * @param nom_stas_per_bcn Nominal number of STAs per beacon (0 for no spreading)
 * @param praw_period the period of the PRAW in beacons (0 for PRAW disabled)
 * @param praw_start_offset the beacon offset of the PRAW within the period
 *
 * @return 0 on success, else error code
 */
int morse_raw_priority_enable(const char *ifname, bool enable, u8 prio, u32 start_time_us,
	u32 duration_us, u8 num_slots, bool cross_slot, u16 max_bcn_spread, u16 nom_stas_per_bcn,
	u8 praw_period, u8 praw_start_offset);

/**
 * Determine if a channel has a disabled primary channel confiuration
 *
 * This function checks if a HT-mapped operating channel has a disabled HT primary or HT secondary
 * channel. The function is only valid to be called on a channel that has the same bandwidth as the
 * configs operating class, as it uses the primary channel index from the config.
 *
 * @param conf Hostapd config, including configuration containing primary channel parameters
 * @param mode Supported HW mode information, including a list of enabled/disabled channels
 * @param s1g_op_chan S1G operating channel
 *
 * @return True if HT primary or HT secondary is disabled
 */
bool morse_s1g_is_chan_conf_primary_disabled(struct hostapd_config *conf,
					     struct hostapd_hw_modes *mode, int s1g_op_chan);

#endif /* MORSE_H */
