/*
 * Copyright 2017-2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _RAW_H_
#define _RAW_H_

#include <linux/types.h>
#include <linux/workqueue.h>

#define MAX_NUM_RAWS_USER_PRIO			(8)	/* Limited by QoS User Priority */
#define MAX_NUM_RAWS_INTERNAL			(1)	/* Internal (e.g. used by OCS) */
#define MAX_NUM_RAWS				(MAX_NUM_RAWS_USER_PRIO + MAX_NUM_RAWS_INTERNAL)
#define RAW_INTERNAL_ID_OFFSET			(0x8000)
#define MORSE_RAW_DEFAULT_START_AID		(1)

/* AID mask used for creating the RAW priority groups */
#define MORSE_RAW_AID_PRIO_MASK			GENMASK(10, 8)
#define MORSE_RAW_AID_PRIO_SHIFT		(8)
#define MORSE_RAW_GET_PRIO(x) \
	(((x) & MORSE_RAW_AID_PRIO_MASK) >> MORSE_RAW_AID_PRIO_SHIFT)
#define MORSE_RAW_GET_SUB_AID(x) \
	((x) & ~MORSE_RAW_AID_PRIO_MASK)
#define MORSE_RAW_AID_DEVICE_MASK		GENMASK(7, 0)

/* EMA smoothing factor: 1/8 */
#define EMA_SHIFT       3
#define EMA_ALPHA       BIT(EMA_SHIFT)
/* Calculate exponential moving average */
#define EMA_AVG_CALC(cur, new) (((cur) * (EMA_ALPHA - 1) + (new)) >> EMA_SHIFT)

/* Initialize EMA if previous is zero */
#define EMA_INIT(prev, new_val) \
	((prev) == 0 ? (new_val) : EMA_AVG_CALC(prev, new_val))

/* Update EMA for u32 values */
static inline u32 ema_update_u32(u32 prev, u32 sample)
{
	return EMA_INIT(prev, sample);
}

struct morse;
struct morse_vif;
struct morse_cmd_req_config_raw;

/**
 * enum ieee80211_s1g_rps_raw_type - types of RAW possible in the RPS IE
 *
 * @IEEE80211_S1G_RPS_RAW_TYPE_GENERIC: generic RAW type
 * @IEEE80211_S1G_RPS_RAW_TYPE_SOUNDING: sounding RAW type
 * @IEEE80211_S1G_RPS_RAW_TYPE_SIMPLEX: simplex RAW type
 * @IEEE80211_S1G_RPS_RAW_TYPE_TRIGGERING: triggering RAW type
 */
enum ieee80211_s1g_rps_raw_type {
	IEEE80211_S1G_RPS_RAW_TYPE_GENERIC = 0,
	IEEE80211_S1G_RPS_RAW_TYPE_SOUNDING = 1,
	IEEE80211_S1G_RPS_RAW_TYPE_SIMPLEX = 2,
	IEEE80211_S1G_RPS_RAW_TYPE_TRIGGERING = 3,
};

enum ieee80211_s1g_rps_raw_sounding_type {
	IEEE80211_S1G_RPS_RAW_TYPE_SST_SOUNDING = 0,
	IEEE80211_S1G_RPS_RAW_TYPE_SST_REPORT = 1,
	IEEE80211_S1G_RPS_RAW_TYPE_SECTOR_SOUNDING = 2,
	IEEE80211_S1G_RPS_RAW_TYPE_SECTOR_REPORT = 3,
};

enum ieee80211_s1g_rps_raw_simplex_type {
	IEEE80211_S1G_RPS_RAW_TYPE_AP_PM = 0,
	IEEE80211_S1G_RPS_RAW_TYPE_NON_TIM = 1,
	IEEE80211_S1G_RPS_RAW_TYPE_OMNI = 2,
};

/** Structure with configuration parameters specific to the Generic RAW */
struct morse_raw_config_generic_t {
	/** Whether or not only paged STAs may transmit. */
	bool paged_sta;

	/** Whether or not to transmit a resource allocation frame at the beginning of the raw. */
	bool ra_frame;

	/** Whether or not to use the last group definition or use a new one. */
	bool group_same_as_prev;
};

/** Structure with configuration parameters specific to the Sounding RAW */
struct morse_raw_config_sounding_t {
	/** Type of Sounding RAW. */
	enum ieee80211_s1g_rps_raw_sounding_type sounding_type;

	/** Whether or not to use the last group definition or use a new one. */
	bool group_same_as_prev;
};

/** Structure with configuration parameters specific to the Simplex RAW */
struct morse_raw_config_simplex_t {
	/** Type of Simplex RAW. */
	enum ieee80211_s1g_rps_raw_simplex_type simplex_type;

	/** Whether to exclude non-AP STAs. If not excluded then define group. */
	bool exclude_non_ap_sta;
};

/** Structure with configuration parameters specific to the Triggering RAW */
struct morse_raw_config_triggering_t {
	/** Whether or not to use the last group definition or use a new one. */
	bool group_same_as_prev;
};

/**
 * Contains AID information used to generate RPS IEs
 */
struct morse_aid_list {
	/** Number of AIDs */
	u16 num_aids;
	/** Array of AIDs of stations */
	u16 aids[];
};

/** Structure containing configuration information creating for RAW assignments in an RPS IE. */
struct morse_raw_config {
	/** List head for RAW config list */
	struct list_head list;

	/** List head for RAWs currently active */
	struct list_head active_list;

	/** RAW type */
	enum ieee80211_s1g_rps_raw_type type;

	/** ID of this RAW config */
	u16 id;

	/** Start time offset from the last RAW or beacon in microseconds. */
	u32 start_time_us;

	/*
	 * If the generic/sounding/triggering RAW isn't using the previous group definition then
	 * use these AID values.
	 */
	/** Starting AID for the RAW. */
	u16 start_aid;
	/** Ending AID for the RAW. */
	u16 end_aid;

	/**
	 * The index into the station data aid list for the first AID in the range of this
	 * config.
	 */
	s32 start_aid_idx;
	/**
	 * The index into the station data aid list for the last AID in the range of this
	 * config.
	 */
	s32 end_aid_idx;

	/**
	 * Slot definition for this RAW assignment.
	 */
	struct {
		/** Allow transmitting STAs to bleed into the next slot */
		bool cross_slot_boundary;
		/** Number of slots in the RAW. */
		u16 num_slots;
		/**
		 * Duration of the slot in the RAW in microseconds.
		 * Maximum duration is 246260us.
		 */
		u32 slot_duration_us;
	} slot_definition;

	/* Optional periodic configuration information. */
	struct {
		/**
		 * If true, refresh the PRAW when the validity expires. Otherwise, disable the
		 * RAW cfg.
		 */
		bool refresh_praw;
		/** Validity of the RAW in number of beacons. If non-0, config is a PRAW */
		u8 validity;
		/** Period of the RAW in number of beacons. */
		u8 periodicity;
		/** Start offset from the number of beacons. */
		u8 start_offset;
		/** The current validity of the PRAW */
		u8 cur_validity;
		/** The current period start offset of the PRAW */
		u8 cur_start_offset;
	} periodic;

	/**
	 * Struct for MM RAW beacon spreading information.
	 * Beacon spreading allows a RAW group to spread its stations over multiple assignments &
	 * beacons.
	 */
	struct {
		/**
		 * When spreading STAs over multiple beacons this is the maximum number of beacons
		 * to cycle over. If 0 then there is no maximum beacon spread.
		 */
		u16 max_spread;
		/**
		 * When spreading STAs over multiple beacons this is the number to place in a beacon
		 * before increasing the number of beacons to cycle over. If 0 then spreading over
		 * multiple beacons is disabled.
		 */
		u16 nominal_sta_per_beacon;
		/** Last AID that was used in a beacon with spreading. */
		u16 last_aid;
	} beacon_spreading;

	/**
	 * Struct for MM dynamic RAW configuration settings.
	 */
	struct {
		/** The insertion index of this configuration into the current raw cycle. */
		u16 insert_at_idx;
	} dynamic;

	/** RAW type specific configuration information. */
	union {
		/** Generic RAW specific configuration information. */
		struct morse_raw_config_generic_t generic;
		/** Sounding RAW specific configuration information. */
		struct morse_raw_config_sounding_t sounding;
		/** Simplex RAW specific configuration information. */
		struct morse_raw_config_simplex_t simplex;
		/** Triggering RAW specific configuration information. */
		struct morse_raw_config_triggering_t triggering;
	};
};

enum raw_state_flags {
	/** RAW is globally enabled */
	RAW_STATE_ENABLED,
	/** RPS IE must be regenerated every beacon */
	RAW_STATE_UPDATE_EACH_BEACON,
	/** An AID refresh is required (a STA may have joined the BSS) */
	RAW_STATE_REFRESH_AIDS,
	/** A beacon has been sent since the last update */
	RAW_STATE_BEACON_SENT,
};

/**
 * struct morse_raw - contains RAW state and configuration information
 */
struct morse_raw {
	/** Bitmask of \ref enum raw_state_flags, mainly to signal conditions to the update work */
	unsigned long flags;
	/** Number of beacons left to send PRAWs */
	u8 praw_tx_count;
	/** The currently generated RPS IE */
	u8 *rps_ie;
	/** The size of the currently generated RPS IE */
	u8 rps_ie_len;
	struct {
		/**
		 * Number of static configurations active across active_raws and active_praws.
		 * Static configurations are configured and set once on UMAC start.
		 */
		int num_static;
		/**
		 * Number of dynamic configurations active in active_raws.
		 * Dynamic configurations are removed and added over the lifetime of the interface.
		 */
		int num_dynamic;
	} configs;

	/**
	 * Dynamic RAW configurations are installed and removed once they reach
	 * their lifetime expiry.
	 */
	struct {
		/* The number of beacons passed in the current configuration cycle */
		int current_num;
		/* Maximum number of beacons in the current configuration cycle */
		u8 num_beacons;
	} dynamic;

	/** An ordered list of AIDs, for use in generating the RPS IE */
	struct morse_aid_list *aid_list;
	/** All RAW configs */
	struct list_head raw_config_list;
	/** List for active PRAWs */
	struct list_head active_praws;
	/** List for active non-PRAWs */
	struct list_head active_raws;
	/** Work struct for updating RAW state / RPS IE / refreshing AIDs */
	struct work_struct update_work;
	/* Serialise RAW command and timer functions */
	struct mutex lock;
};

/**
 * morse_raw_cfg_is_periodic() - Returns true if the RAW config is a PRAW
 *
 * @cfg: config to test
 * Returns true if periodic RAW, else false
 */
static inline bool morse_raw_cfg_is_periodic(const struct morse_raw_config * const cfg)
{
	return cfg->periodic.validity != 0;
}

/**
 * morse_raw_has_dynamic_config() - Returns true if the dynamic RAW config is active.
 *
 * @cfg: RAW context
 * Returns true if dynamic RAW, else false
 */
static inline bool morse_raw_has_dynamic_config(struct morse_raw *raw)
{
	return raw->configs.num_dynamic > 0;
}

/**
 * morse_raw_has_static_config() - Returns true if the static RAW config is active.
 *
 * @raw: RAW context
 * Returns true if static RAW, else false
 */
static inline bool morse_raw_has_static_config(struct morse_raw *raw)
{
	return raw->configs.num_static > 0;
}

/**
 * morse_raw_cfg_has_bcn_idx() - Returns true if the RAW configuration has a beacon index.
 *
 * @cfg: config to test
 * Returns true if RAW config has index, else false
 */
static inline bool morse_raw_cfg_has_bcn_idx(const struct morse_raw_config *cfg)
{
	return cfg->dynamic.insert_at_idx != U16_MAX;
}

/**
 * morse_raw_process_rx_mgmt() - Process management frames in RAW module
 *
 * @mors:      Morse device
 * @vif:       VIF struct
 * @sta:       pointer to ieee80211_sta
 * @skb:       Rx management frame SKB
 * @ies_mask:  IEs Mask
 *
 * Return: 0 if frame was processed successfully, otherwise error code
 */
int morse_raw_process_rx_mgmt(struct morse *mors, struct ieee80211_vif *vif,
			struct ieee80211_sta *sta, const struct sk_buff *skb,
			struct dot11ah_ies_mask *ies_mask);

/**
 * morse_raw_get_rps_ie_size() - Gets the size of the RPS IE for current RAW settings.
 * @mors_vif: Morse VIF structure
 *
 * Return: Size of the RPS IE or 0 on error / RAW disabled.
 */
u8 morse_raw_get_rps_ie_size(struct morse_vif *mors_vif);

/**
 * morse_raw_get_rps_ie() - Gets the current RPS IE for current RAW settings.
 * @mors_vif: Morse VIF structure
 *
 * Use morse_raw_get_rps_ie_size() to calculate the size to allocate for &rps_ie.
 *
 * Return: a pointer to the RPS IE or NULL on error.
 */
u8 *morse_raw_get_rps_ie(struct morse_vif *mors_vif);

/**
 * morse_raw_process_cmd() - Execute command to enable/disable/configure RAW
 *
 * @mors_vif:	Morse VIF structure
 * @req:	Command from morsectrl or hostapd or smart manager
 * Return: 0 if command was processed successfully, otherwise error code
 */
int morse_raw_process_cmd(struct morse_vif *mors_vif, struct morse_cmd_req_config_raw *req);

/**
 * morse_raw_create_or_find_by_id() - Create a RAW config with the specified ID if it does not
 * already exist. If it does exist, returns the existing config.
 * Callers should hold the RAW lock
 *
 * @raw: RAW context
 * @id: ID of RAW config to find or create
 * Return: RAW config
 */
struct morse_raw_config *morse_raw_create_or_find_by_id(struct morse_raw *raw, u16 id);

/**
 * morse_raw_find_config_by_id() - Find a RAW config with the specified ID
 *
 * @raw: RAW context
 * @id: ID to find
 * Return: RAW config, or NULL if not found
 */
struct morse_raw_config *morse_raw_find_config_by_id(struct morse_raw *raw, u16 id);

/**
 * morse_raw_is_enabled() - Is raw enabled on a particular virtual interface
 *
 * @mors_vif: Morse VIF structure
 *
 * Return: true if enabled, otherwise false
 */
bool morse_raw_is_enabled(struct morse_vif *mors_vif);

/**
 * morse_raw_enable() - Enable RAW globally
 *
 * @raw: RAW context
 */
void morse_raw_enable(struct morse_raw *raw);

/**
 * morse_raw_trigger_update() - Trigger an update of RAW information. Call this function when BSS
 * or RAW parameters change to regenerate the RPS IE.
 *
 * @mors_vif: Morse VIF structure
 * @refresh_aids: An AID refresh is required (a STA may have joined the network)
 */
void morse_raw_trigger_update(struct morse_vif *mors_vif, bool refresh_aids);

/**
 * morse_raw_beacon_sent() - Call this function after a beacon has been transmitted to update
 * RPS IE, if required.
 *
 * @mors_vif: Morse VIF structure
 */
void morse_raw_beacon_sent(struct morse_vif *mors_vif);

/**
 * morse_raw_init() - Initialise RAW.
 * @mors_vif: Morse VIF structure
 *
 * Return: 0 - OK
 */
int morse_raw_init(struct morse_vif *mors_vif, bool enable);

/**
 * morse_raw_finish() - Clean up RAW on finish.
 * @mors_vif: Morse VIF structure
 */
void morse_raw_finish(struct morse_vif *mors_vif);

/**
 * morse_dynamic_raw_init() - Clean up dynamic RAW and reinitialize.
 * @mors_vif: Morse VIF structure
 */
void morse_dynamic_raw_init(struct morse_vif *mors_vif);

/**
 * morse_raw_activate_config() - Activate and schedule a RAW config
 *
 * @raw: RAW context
 * @cfg: The config to activate
 */
void morse_raw_activate_config(struct morse_raw *raw, struct morse_raw_config *cfg);

/**
 * morse_raw_deactivate_config() - Deactivate and unschedule a RAW config
 *
 * @raw: RAW context
 * @cfg: The config to deactivate
 */
void morse_raw_deactivate_config(struct morse_raw *raw, struct morse_raw_config *cfg);

/**
 * morse_raw_is_config_active() - Test if a config is active
 *
 * @cfg: The config to test
 * Return: true if the config is active, otherwise false
 */
bool morse_raw_is_config_active(struct morse_raw_config *cfg);

#endif
