/*
 * Copyright 2017-2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "debug.h"
#include "raw.h"
#include "command.h"

#define INVALID_AID_VALUE				(-1)
#define INVALID_AID_IDX_VALUE				(-1)

/* Masks for RAW assignment */
#define IEEE80211_S1G_RPS_RAW_CONTROL_TYPE_SHIFT	(0)
#define IEEE80211_S1G_RPS_RAW_CONTROL_TYPE		GENMASK(1, 0)

/**
 * enum ieee80211_s1g_rps_raw_control_generic_flags - generic RAW flags
 *
 * @IEEE80211_S1G_RPS_RAW_CONTROL_GENERIC_PSTA: paged STA flag
 * @IEEE80211_S1G_RPS_RAW_CONTROL_GENERIC_RAFRAME: RA frame flag
 */
enum ieee80211_s1g_rps_raw_control_generic_flags {
	IEEE80211_S1G_RPS_RAW_CONTROL_GENERIC_PSTA = BIT(0),
	IEEE80211_S1G_RPS_RAW_CONTROL_GENERIC_RAFRAME = BIT(1),
};

#define IEEE80211_S1G_RPS_RAW_CONTROL_TYPE_OPTION_SHIFT	(2)
#define IEEE80211_S1G_RPS_RAW_CONTROL_TYPE_OPTION	GENMASK(3, 2)

enum ieee80211_s1g_rps_raw_control_ind_flags {
	IEEE80211_S1G_RPS_RAW_CONTROL_START_IND = BIT(4),
	IEEE80211_S1G_RPS_RAW_CONTROL_GROUP_IND = BIT(5),
	IEEE80211_S1G_RPS_RAW_CONTROL_CHAN_IND = BIT(6),
	IEEE80211_S1G_RPS_RAW_CONTROL_PERIODIC_IND = BIT(7),
};

/**
 * enum ieee80211_s1g_rps_raw_slot_flags - RAW slot flags
 *
 * @IEEE80211_S1G_RPS_RAW_SLOT_DEF_FORMAT: slot format determining bits for duration
 * @IEEE80211_S1G_RPS_RAW_SLOT_CROSS_BOUNDARY: cross slot boundary bleed over
 */
enum ieee80211_s1g_rps_raw_slot_flags {
	IEEE80211_S1G_RPS_RAW_SLOT_DEF_FORMAT = BIT(0),
	IEEE80211_S1G_RPS_RAW_SLOT_CROSS_BOUNDARY = BIT(1),
};

#define IEEE80211_S1G_RPS_RAW_SLOT_NUM_3BITS		(3)
#define IEEE80211_S1G_RPS_RAW_SLOT_NUM_6BITS		(6)

#define IEEE80211_S1G_RPS_RAW_SLOT_DUR_8BITS		(8)
#define IEEE80211_S1G_RPS_RAW_SLOT_DUR_11BITS		(11)

#define IEEE80211_S1G_RPS_RAW_SLOT_DCOUNT_SHIFT		(2)
#define IEEE80211_S1G_RPS_RAW_SLOT_DCOUNT_8		GENMASK(9, 2)
#define IEEE80211_S1G_RPS_RAW_SLOT_DCOUNT_11		GENMASK(12, 2)

#define IEEE80211_S1G_RPS_RAW_SLOT_NUM_6_SHIFT		(10)
#define IEEE80211_S1G_RPS_RAW_SLOT_NUM_3_SHIFT		(13)
#define IEEE80211_S1G_RPS_RAW_SLOT_NUM_6		GENMASK(16, 10)
#define IEEE80211_S1G_RPS_RAW_SLOT_NUM_3		GENMASK(15, 13)

/* Masks for RAW groups */
#define IEEE80211_S1G_RPS_RAW_GROUP_PAGE_IDX_SHIFT	(0)
#define IEEE80211_S1G_RPS_RAW_GROUP_PAGE_IDX		GENMASK(1, 0)

#define AID_BITS					(11)
#define AID_END_BITS_SHIFT				(16 - 2 - AID_BITS)

#define IEEE80211_S1G_RPS_RAW_GROUP_START_AID_SHIFT	(2)
#define IEEE80211_S1G_RPS_RAW_GROUP_START_AID		GENMASK(AID_BITS + 1, 2)

#define IEEE80211_S1G_RPS_RAW_GROUP_END_AID_SHIFT	(13)
#define IEEE80211_S1G_RPS_RAW_GROUP_END_AID		GENMASK(AID_END_BITS_SHIFT + 12, 13)

/* Masks for RAW channel indication */
#define IEEE80211_S1G_RPS_RAW_CHAN_MAX_TRAN_WIDTH_SHIFT	(0)
#define IEEE80211_S1G_RPS_RAW_CHAN_MAX_TRAN_WIDTH_BITS	(2)
#define IEEE80211_S1G_RPS_RAW_CHAN_MAX_TRAN_WIDTH	GENMASK(1, 0)

/**
 * enum ieee80211_s1g_rps_raw_chan_activity_flags - RAW SST activity flags
 *
 * @IEEE80211_S1G_RPS_RAW_CHAN_UL_ACTIVITY: UL Activity subfield flag
 * @IEEE80211_S1G_RPS_RAW_CHAN_DL_ACTIVITY: DL Activity subfield flag
 */
enum ieee80211_s1g_rps_raw_chan_activity_flags {
	IEEE80211_S1G_RPS_RAW_CHAN_UL_ACTIVITY = BIT(3),
	IEEE80211_S1G_RPS_RAW_CHAN_DL_ACTIVITY = BIT(4),
};

/** Minimum slot duration in us. (Corresponds to a cslot value of 0) */
#define MORSE_RAW_MIN_SLOT_DURATION_US	(500)

/**
 * CSLOT_TO_US() - Convert a cslot value to a microseconds (us) duration.
 * @x: cslot value to convert
 *
 * Returns: a duration in microseconds (us)
 */
#define CSLOT_TO_US(x) (MORSE_RAW_MIN_SLOT_DURATION_US + ((x) * 120))

/**
 * US_TO_CSLOT() - Convert a microseconds (us) duration to a cslot value.
 * @x: microseconds duration to convert
 *
 * Returns: a cslot value
 *
 * It is worth noting that the cslot is not value checked here. Checking for
 * overflow or negative values should be done elsewhere.
 */
#define US_TO_CSLOT(x)	(((x) - MORSE_RAW_MIN_SLOT_DURATION_US) / 120)

/**
 * US_TO_TWO_TU() - Convert a microseconds (us) duration to a 2TU (2.048ms units) value.
 * @x: microseconds duration to convert
 *
 * Returns: a 2TU value
 */
#define US_TO_TWO_TU(x)	((x) / (1024 * 2))

/**
 * TWO_TU_TO_US() - Convert a 2TU (2.048ms units) value to a microseconds (us) duration.
 * @x: 2TU value to convert
 *
 * Returns: a duration in microseconds (us)
 */
#define TWO_TU_TO_US(x)	((x) * (1024 * 2))

/**
 * Transmit PRAWs for 10 DTIM beacons, to ensure the whole BSS has seen the PRAW assignments
 */
#define MORSE_RAW_DTIMS_FOR_PRAW_TX		(10)

#define MORSE_RAW_DBG(_m, _f, _a...)		morse_dbg(FEATURE_ID_RAW, _m, _f, ##_a)
#define MORSE_RAW_INFO(_m, _f, _a...)		morse_info(FEATURE_ID_RAW, _m, _f, ##_a)
#define MORSE_RAW_WARN(_m, _f, _a...)		morse_warn(FEATURE_ID_RAW, _m, _f, ##_a)
#define MORSE_RAW_WARN_RATELIMITED(_m, _f, _a...) \
						morse_warn_ratelimited(FEATURE_ID_RAW, _m, _f, ##_a)
#define MORSE_RAW_ERR(_m, _f, _a...)		morse_err(FEATURE_ID_RAW, _m, _f, ##_a)

#define MORSE_RAW_WARN_ON(_val)			MORSE_WARN_ON(FEATURE_ID_RAW, _val)

enum morse_cmd_raw_enable_type {
	RAW_CMD_ENABLE_TYPE_GLOBAL = 0,
	RAW_CMD_ENABLE_TYPE_SINGLE = 1,
};

/**
 * struct ieee80211_s1g_rps_raw_assignment - RAW assignment subfield, of which multiple can be
 *	present in an RPS IE. RAW assignments define a RAW configuration on a BSS.
 * @raw_control:	RAW Control subfield - see Figure 9-670 (RAW Assignment
 *			subfield format(11ah)).
 * @slot_definition:	RAW Slot Definition - see Figure 9-671 (RAW Slot
 *			Definition Subfield format(11ah)).
 * @optional:		Contains various options defined in raw_control. These
 *			can include RAW Start Time, RAW Group, Channel
 *			Indication, Periodic Operation Parameters. Additional
 *			RAWs can also be specified which are added to the end.
 *
 * For details see 9.4.2.191 RPS element(11ah).
 */
struct ieee80211_s1g_rps_raw_assignment {
	u8 raw_control;
	__le16 slot_definition;
	/* Optional 0,1,2,3,5,6,7,8 or 9 bytes: depends on @raw_control */
	u8 optional[];
} __packed;

/**
 * struct morse_raw_start_time_t - RAW start time definition
 * @start_time_2tu:	RAW start time from either the current beacon or the end of
 *			the last RAW.
 *
 * The RAW Start Time subfield indicates the duration, in units of 2 TUs, from the
 * end of the S1G Beacon, the Probe Response, or the PV1 Probe Response frame transmission
 * that includes the RPS element to the start time of the RAW.
 */
struct morse_raw_start_time_t {
	u8 start_time_2tu;
} __packed;

/**
 * struct morse_raw_group_t - RAW Group definition
 * @raw_group12:	First two octets of the group definition, includes Page
 *			Index, RAW Start AID, and 3 bits of RAW End AID
 * @raw_group3:		Contains the third octet of the group definition, includes
 *			8 bits of the RAW End AID
 *
 * The RAW Group subfield indicates the STA AIDs that are allowed restricted access within
 * the RAW period. The RAW Group subfield contains Page Index, RAW Start AID, and RAW End AID
 * subfields according to the hierarchical addressing method of AIDs
 * (see Figure 9-152 (Hierarchical structure of traffic-indication virtual bitmap carried in an
 * S1G PPDU(#2001)(11ah))).
 */
struct morse_raw_group_t {
	__le16 raw_group12;
	u8 raw_group3;
} __packed;

/**
 * struct morse_raw_channel_t - RAW Channel definition
 * @channel_activity_bitmap:	bitmap of allowed operating channels
 * @channel2:			contains the Max Transmission width, UL and DL
 *				activity indications
 *
 * The Channel Activity Bitmap subfield shows the allowed operating channels for the
 * STAs indicated in the RAW, as defined in 10.23.5.1 (General). Each bit in the bitmap
 * corresponds to one minimum width channel within the current BSS operating channels, with the
 * least significant bit corresponding to the lowest numbered operating channel of the BSS.
 */
struct morse_raw_channel_t {
	u8 channel_activity_bitmap;
	u8 channel2;
} __packed;

/**
 * morse_raw_periodic_t - Periodic RAW definition
 * @periodicity:	period of the current PRAW occurrance
 * @validity:		number of periods the PRAW repeats
 * @start_offset:	number of beacons until PRAW starts
 *
 * 9.4.2.191 RPS element(11ah)
 *
 * The PRAW Periodicity subfield indicates the period of current PRAW occurrance in the unit of
 * beacon interval if dot11ShortBeaconInterval is false and in the unit of short beacon interval if
 * dot11ShortBeaconInterval is true (see 11.1.3.10.2 (Generation of S1G Beacon frames)).
 */
struct morse_raw_periodic_t {
	u8 periodicity;
	u8 validity;
	u8 start_offset;
} __packed;

/**
 * morse_raw_build_aid_list() - Generate an ordered AID array from a station bitmap.
 *
 * @aid_bitmap: Pointer to bitmap where bit position indicates an AID
 * @num_aids: The number of AIDs
 * @max_aid: the largest AID in the bitmap
 * @gfp: allocation flags
 *
 * @return: allocated AID list
 */
static struct morse_aid_list *morse_generate_aid_list(unsigned long *aid_bitmap, int num_aids,
	int max_aid, gfp_t gfp)
{
	int aid;
	int idx = 0;
	struct morse_aid_list *list;

	/* Bitops search up to but not including 'size'. +1 so we include the 'last' AID in our
	 * search.
	 */
	const int num_bits = max_aid + 1;

	list = kmalloc(sizeof(list) + (num_aids * sizeof(list->aids[0])), gfp);
	if (!list)
		return NULL;

	list->num_aids = num_aids;

	for_each_set_bit(aid, aid_bitmap, num_bits) {
		list->aids[idx++] = aid;
		if (unlikely(idx >= num_aids))
			break;
	}

	return list;
}

/**
 * morse_raw_generate_slot_definition() - Generate the slot definition for the RAW assignment of a
 *	RAW config.
 * @mors:		Morse chip struct
 * @config:		RAW config
 *
 * Returns: Slot definition field for the RAW assignment
 *
 * Because the number of bits allocated to duration and the number of slots is
 * variable, preference is given to the duration. I.e. a long duration is more
 * likely to result in the number of slots being capped.
 *
 */
static __le16 morse_raw_generate_slot_definition(struct morse *mors,
		struct morse_raw_config *config)
{
	__le16 slot_def = 0;
	u8 max_slots = 0;
	u16 cslot_max = 0;
	u32 cslot;

	if (config->slot_definition.slot_duration_us < MORSE_RAW_MIN_SLOT_DURATION_US) {
		cslot = US_TO_CSLOT(MORSE_RAW_MIN_SLOT_DURATION_US);
		MORSE_RAW_WARN(mors, "RAW Slot duration too short, setting to %u\n",
			       MORSE_RAW_MIN_SLOT_DURATION_US);
	} else {
		cslot = US_TO_CSLOT(config->slot_definition.slot_duration_us);
	}

	MORSE_RAW_DBG(mors, "Slot duration us, cslot: %u, %u\n",
			config->slot_definition.slot_duration_us, cslot);

	if (config->slot_definition.cross_slot_boundary)
		slot_def |= cpu_to_le16(IEEE80211_S1G_RPS_RAW_SLOT_CROSS_BOUNDARY);

	switch (config->type) {
	case IEEE80211_S1G_RPS_RAW_TYPE_SOUNDING:
		cslot_max = (1 << IEEE80211_S1G_RPS_RAW_SLOT_DUR_8BITS) - 1;
		max_slots = IEEE80211_S1G_RPS_RAW_SLOT_NUM_6BITS;
		break;
	case IEEE80211_S1G_RPS_RAW_TYPE_GENERIC:
	case IEEE80211_S1G_RPS_RAW_TYPE_SIMPLEX:
	case IEEE80211_S1G_RPS_RAW_TYPE_TRIGGERING:
		/* Give longer durations preference over greater number of slots. */
		if (cslot > __UINT8_MAX__) {
			slot_def |= cpu_to_le16(IEEE80211_S1G_RPS_RAW_SLOT_DEF_FORMAT);
			cslot_max = (1 << IEEE80211_S1G_RPS_RAW_SLOT_DUR_11BITS) - 1;
			max_slots = IEEE80211_S1G_RPS_RAW_SLOT_NUM_3BITS;
		} else {
			cslot_max = (1 << IEEE80211_S1G_RPS_RAW_SLOT_DUR_8BITS) - 1;
			max_slots = IEEE80211_S1G_RPS_RAW_SLOT_NUM_6BITS;
		}
		break;
	}

	if (config->slot_definition.num_slots > max_slots) {
		MORSE_RAW_WARN(mors, "Too many slots: %u, capping to %u\n",
				config->slot_definition.num_slots, max_slots);
		config->slot_definition.num_slots = max_slots;
	}

	if (cslot > cslot_max) {
		MORSE_RAW_WARN(mors, "Slot duration too long: %u (%uus), capping to %u (%uus)\n",
			       cslot, CSLOT_TO_US(cslot), cslot_max, CSLOT_TO_US(cslot_max));
		cslot = cslot_max;
	}

	if (le16_to_cpu(slot_def) & IEEE80211_S1G_RPS_RAW_SLOT_DEF_FORMAT) {
		slot_def |= cpu_to_le16((cslot << IEEE80211_S1G_RPS_RAW_SLOT_DCOUNT_SHIFT) &
						IEEE80211_S1G_RPS_RAW_SLOT_DCOUNT_11);

		slot_def |= cpu_to_le16((config->slot_definition.num_slots <<
						 IEEE80211_S1G_RPS_RAW_SLOT_NUM_3_SHIFT) &
						IEEE80211_S1G_RPS_RAW_SLOT_NUM_3);
	} else {
		slot_def |= cpu_to_le16((cslot << IEEE80211_S1G_RPS_RAW_SLOT_DCOUNT_SHIFT) &
						IEEE80211_S1G_RPS_RAW_SLOT_DCOUNT_8);

		slot_def |= cpu_to_le16((config->slot_definition.num_slots <<
						 IEEE80211_S1G_RPS_RAW_SLOT_NUM_6_SHIFT) &
						IEEE80211_S1G_RPS_RAW_SLOT_NUM_6);
	}
	return slot_def;
}

u8 morse_raw_get_rps_ie_size(struct morse_vif *mors_vif)
{
	struct morse_ap *ap = mors_vif->ap;

	if (!ap || !ap->raw.rps_ie || !morse_raw_is_enabled(mors_vif))
		return 0;

	return ap->raw.rps_ie_len;
}

/**
 * morse_raw_calc_rps_ie_size() -	Calculates the RPS IE size required for the provided RAW
 *									configurations
 *
 * @config_list		List of RAW configurations
 * @num_configs		Number of RAW configurations in the list.
 *
 * Return size (>0) on success otherwise -EINVAL if a RAW configuration is invalid.
 */
static int morse_raw_calc_rps_ie_size(const struct morse_raw_config *const *config_list,
				      u8 num_configs)
{
	u8 i;
	u16 size = 0;

	if (!num_configs) {
		MORSE_WARN_ON(FEATURE_ID_RAW, true);
		return 0;
	}

	WARN_ON(!config_list);

	for (i = 0; i < num_configs; i++) {
		/* Check for unsupported types. */
		switch (config_list[i]->type) {
		case IEEE80211_S1G_RPS_RAW_TYPE_SOUNDING:
		case IEEE80211_S1G_RPS_RAW_TYPE_SIMPLEX:
		case IEEE80211_S1G_RPS_RAW_TYPE_TRIGGERING:
			WARN_ON(true);
			break;
		case IEEE80211_S1G_RPS_RAW_TYPE_GENERIC:
			/* If the start time is 0 we can omit the start time field. */
			if (config_list[i]->start_time_us != 0)
				size += sizeof(struct morse_raw_start_time_t);

			/*
			 * While we could omit the RAW group configuration if the same as the last
			 * RAW we will include it for simplicity.
			 */
			size += sizeof(struct morse_raw_group_t);

			if (morse_raw_cfg_is_periodic(config_list[i]))
				size += sizeof(struct morse_raw_periodic_t);
		}

		size += sizeof(struct ieee80211_s1g_rps_raw_assignment);
	}

	return size;
}

u8 *morse_raw_get_rps_ie(struct morse_vif *mors_vif)
{
	return mors_vif->ap->raw.rps_ie;
}

static u8 *morse_raw_generate_assignment_with_aid_range(struct morse_vif *mors_vif,
		struct morse_raw_config *config, u8 *rps_ie, u16 start_aid, u16 end_aid)
{
	struct morse *mors = morse_vif_to_morse(mors_vif);
	struct ieee80211_s1g_rps_raw_assignment *rps_ptr =
			(struct ieee80211_s1g_rps_raw_assignment *)rps_ie;
	struct morse_raw_start_time_t *start_time_ptr;
	struct morse_raw_group_t *raw_group_ptr;
	struct morse_raw_channel_t *raw_channel_ptr;
	struct morse_raw_periodic_t *raw_periodic_ptr;
	u8 *end_ptr;
	/* Pages aren't used yet so always use zero. */
	const u8 page = 0;

	/* Create a basic configuration (Generic RAW) with all devices in a single RAW */
	rps_ptr->raw_control =
	    ((config->type << IEEE80211_S1G_RPS_RAW_CONTROL_TYPE_SHIFT) &
	     IEEE80211_S1G_RPS_RAW_CONTROL_TYPE);

	rps_ptr->slot_definition = morse_raw_generate_slot_definition(mors, config);

	start_time_ptr = (struct morse_raw_start_time_t *)(rps_ptr + 1);

	if (config->start_time_us != 0) {
		rps_ptr->raw_control |= IEEE80211_S1G_RPS_RAW_CONTROL_START_IND;
		start_time_ptr->start_time_2tu = US_TO_TWO_TU(config->start_time_us);
		raw_group_ptr = (struct morse_raw_group_t *)(start_time_ptr + 1);
	} else {
		raw_group_ptr = (struct morse_raw_group_t *)start_time_ptr;
	}

	rps_ptr->raw_control |= IEEE80211_S1G_RPS_RAW_CONTROL_GROUP_IND;
	raw_group_ptr->raw_group12 = 0;
	raw_group_ptr->raw_group3 = 0;

	raw_group_ptr->raw_group12 |= cpu_to_le16(page & IEEE80211_S1G_RPS_RAW_GROUP_PAGE_IDX);
	raw_group_ptr->raw_group12 |= cpu_to_le16((start_aid <<
						   IEEE80211_S1G_RPS_RAW_GROUP_START_AID_SHIFT) &
						  IEEE80211_S1G_RPS_RAW_GROUP_START_AID);

	raw_group_ptr->raw_group12 |= cpu_to_le16((end_aid <<
						   IEEE80211_S1G_RPS_RAW_GROUP_END_AID_SHIFT) &
						  IEEE80211_S1G_RPS_RAW_GROUP_END_AID);

	raw_group_ptr->raw_group3 = end_aid >> (AID_END_BITS_SHIFT);
	raw_channel_ptr = (struct morse_raw_channel_t *)(raw_group_ptr + 1);

	/* channel indication subfield not supported */

	raw_periodic_ptr = (struct morse_raw_periodic_t *)raw_channel_ptr;

	if (morse_raw_cfg_is_periodic(config)) {
		rps_ptr->raw_control |= IEEE80211_S1G_RPS_RAW_CONTROL_PERIODIC_IND;
		raw_periodic_ptr->periodicity = config->periodic.periodicity;
		raw_periodic_ptr->validity = config->periodic.cur_validity;
		raw_periodic_ptr->start_offset = config->periodic.cur_start_offset;

		end_ptr = (u8 *)(raw_periodic_ptr + 1);
	} else {
		end_ptr = (u8 *)raw_periodic_ptr;
	}

	return end_ptr;
}

/**
 * morse_raw_generate_assignment() - Generate a single RAW assignment.
 *
 * @mors		Morse chip struct
 * @config		RAW configuration
 * @rps_ie_start	A pointer to the start of this assignment in the RPS IE
 *
 * Return: the end of the RAW assignment in the RPS IE that was just written
 */
static u8 *morse_raw_generate_assignment(struct morse_vif *mors_vif,
					 struct morse_raw_config *config, u8 *rps_ie_start)
{
	struct morse *mors = morse_vif_to_morse(mors_vif);
	/* Pages aren't used yet so always use zero. */
	struct morse_aid_list *aid_list = mors_vif->ap->raw.aid_list;
	u16 num_stas;
	int current_beacon_start_aid_idx = INVALID_AID_IDX_VALUE;
	int current_beacon_end_aid_idx = INVALID_AID_IDX_VALUE;
	u16 current_beacon_start_aid;
	u16 current_beacon_end_aid;
	u32 i;

	u16 sta_per_beacon;
	u16 sta_per_beacon_mod;

	/* If beacon spreading is enabled and there are connected STAs find the subgroup of STAs for
	 * this beacon.
	 */
	if (config->beacon_spreading.nominal_sta_per_beacon &&
	    !(config->start_aid_idx < 0) && !(config->end_aid_idx < 0)) {
		/* Calculate how many STAs in each RAW. */
		num_stas = config->end_aid_idx - config->start_aid_idx + 1;

		MORSE_RAW_DBG(mors, "num_stas: %d, nspb:%d max_spread:%d\n",
			      num_stas, config->beacon_spreading.nominal_sta_per_beacon,
				  config->beacon_spreading.max_spread);

		/* Increase the number of stations per RAW to avoid spreading over
		 * too many beacons if necessary.
		 */
		if (config->beacon_spreading.max_spread &&
		    ((num_stas / config->beacon_spreading.nominal_sta_per_beacon)
				> config->beacon_spreading.max_spread)) {
			sta_per_beacon = num_stas / config->beacon_spreading.max_spread;
			sta_per_beacon_mod = num_stas % config->beacon_spreading.max_spread;
		} else {
			u16 beacon_count = num_stas /
				config->beacon_spreading.nominal_sta_per_beacon;

			if (num_stas % config->beacon_spreading.nominal_sta_per_beacon)
				beacon_count++;

			sta_per_beacon = num_stas / beacon_count;
			sta_per_beacon_mod = num_stas % beacon_count;
			MORSE_RAW_DBG(mors, "beacon_cnt:%d\n", beacon_count);
		}

		MORSE_RAW_DBG(mors, "sta_per_beacon, mod: %u, %u\n",
			      sta_per_beacon, sta_per_beacon_mod);

		/* Find where we should start the AID range for this beacon from. */
		MORSE_RAW_DBG(mors, "Last spread AID: %u\n", config->beacon_spreading.last_aid);
		for (i = config->start_aid_idx;
		     (i <= config->end_aid_idx) && (i < aid_list->num_aids); i++) {
			if (aid_list->aids[i] > config->beacon_spreading.last_aid) {
				current_beacon_start_aid_idx = i;
				break;
			}
		}

		/* If the last end AID was the last of the connected STAs then start the cycle from
		 * the beginning.
		 */
		if (current_beacon_start_aid_idx == INVALID_AID_IDX_VALUE)
			current_beacon_start_aid_idx = config->start_aid_idx;

		/* If we are in one of the earlier RAWs add an additional STA to deal with the
		 * modulus calculated earlier.
		 */
		if (((current_beacon_start_aid_idx - config->start_aid_idx) / sta_per_beacon) <
		    sta_per_beacon_mod)
			sta_per_beacon++;

		/* Find the end AID for this beacon. */
		for (i = current_beacon_start_aid_idx;
		     (i <= config->end_aid_idx) &&
		     (i < (current_beacon_start_aid_idx + sta_per_beacon)) &&
		     (i < aid_list->num_aids); i++) {
			current_beacon_end_aid_idx = i;
		}

		if (current_beacon_end_aid_idx < current_beacon_start_aid_idx) {
			/* This should never happen */
			MORSE_WARN_ON_ONCE(FEATURE_ID_RAW, 1);
			current_beacon_end_aid_idx = current_beacon_start_aid_idx;
		}

		current_beacon_start_aid = aid_list->aids[current_beacon_start_aid_idx];
		current_beacon_end_aid = aid_list->aids[current_beacon_end_aid_idx];
		config->beacon_spreading.last_aid = aid_list->aids[current_beacon_end_aid_idx];
		MORSE_RAW_DBG(mors, "Start, End AID idx: %u, %u\n",
			      current_beacon_start_aid_idx, current_beacon_end_aid_idx);
		MORSE_RAW_DBG(mors, "Start, End AID: %u, %u\n",
			      current_beacon_start_aid, current_beacon_end_aid);

		/* If not using beacon spreading or no connected STAs use the full AID range. */
	} else {
		current_beacon_start_aid = config->start_aid;
		current_beacon_end_aid = config->end_aid;
		config->beacon_spreading.last_aid = config->end_aid;
	}

	return morse_raw_generate_assignment_with_aid_range(mors_vif, config, rps_ie_start,
		current_beacon_start_aid, current_beacon_end_aid);
}

/**
 * morse_raw_generate_rps_ie() - Generate and update the RPS IE depending on RAW configurations.
 * Note: Caller should hold the RAW lock
 *
 * @mors:		Morse chip struct
 * @config_list		List of RAW configurations
 * @num_configs		Number of RAW configurations in the list.
 *
 * Return 0 on success otherwise -EINVAL if a RAW configuration is invalid.
 */
static int morse_raw_generate_rps_ie(struct morse_vif *mors_vif,
					struct morse_raw_config *const *config_list,
					u8 num_configs)
{
	int i;
	u8 *head;
	u8 old_rps_ie_len;
	struct morse *mors = morse_vif_to_morse(mors_vif);
	struct morse_raw *raw = &mors_vif->ap->raw;

	/* Calculate the size so we can allocate memory */
	int size =
	    morse_raw_calc_rps_ie_size((const struct morse_raw_config * const *)config_list,
				       num_configs);

	MORSE_RAW_DBG(mors, "Number of RAWs: %u\n", num_configs);
	MORSE_RAW_DBG(mors, "RPS IE size: %d\n", size);

	WARN_ON((size <= 0) || (size > __UINT8_MAX__));

	/* Invalidate current raw until we are finished by setting to 0. */
	old_rps_ie_len = raw->rps_ie_len;
	raw->rps_ie_len = 0;

	if (raw->rps_ie) {
		/* Adjust size of allocated memory if necessary. */
		if (old_rps_ie_len != size) {
			kfree(raw->rps_ie);
			raw->rps_ie = kmalloc(size, GFP_KERNEL);
		}
	} else {
		/* Allocate memory for the RPS IE. */
		MORSE_RAW_DBG(mors, "Allocate RAW RPS IE\n");
		raw->rps_ie = kmalloc(size, GFP_KERNEL);
	}
	/* Check we got our allocated memory. */
	if (!raw->rps_ie) {
		MORSE_RAW_DBG(mors, "Failed to allocate RAW RPS IE\n");
		return -ENOMEM;
	}

	/* Keep everything neat and zero the memory. */
	memset(raw->rps_ie, 0, size);
	head = raw->rps_ie;

	/* Populate RPS IE using config settings. */
	for (i = 0; i < num_configs; i++) {
		head = morse_raw_generate_assignment(mors_vif, config_list[i], head);
		WARN_ON(head > (raw->rps_ie + size));
	}

	/* Validate RPS IE by giving its size. */
	WARN_ON(head != (raw->rps_ie + size));
	raw->rps_ie_len = size;

	return 0;
}

/**
 * morse_raw_debug_print_aid_idx() - Print the end AID indices and values for RAWs
 *
 * @mors:	Morse chip struct
 * @aid_list:	Current AID list
 */
static void morse_raw_debug_print_aid_idx(struct morse_vif *mors_vif,
					  struct morse_aid_list *aid_list)
{
	struct morse *mors = morse_vif_to_morse(mors_vif);
	struct morse_raw *raw = &mors_vif->ap->raw;
	struct morse_raw_config *config_ptr;

	list_for_each_entry(config_ptr, &raw->raw_config_list, list) {
		if (morse_raw_is_config_active(config_ptr)) {
			MORSE_RAW_DBG(mors,
				      "Final Start/End AID indices (%d): %d, %d\n",
				      config_ptr->id, config_ptr->start_aid_idx,
				      config_ptr->end_aid_idx);

			if (config_ptr->start_aid_idx >= 0 && config_ptr->end_aid_idx >= 0) {
				MORSE_RAW_DBG(mors,
						"Final Start/End AID values (%d): %d, %d\n",
						config_ptr->id,
						aid_list->aids[config_ptr->start_aid_idx],
						aid_list->aids[config_ptr->end_aid_idx]);
			}
		}
	}
}

/**
 * morse_raw_start_praw_transmission() - Start transmitting PRAW assignments to the BSS.
 * Call this when configs are changed, or STAs join the network.
 *
 * @raw: RAW context
 * @new_config: A new config has been added
 */
static void morse_raw_start_praw_transmission(struct morse_raw *raw, bool new_config)
{
	struct morse_raw_config *cfg;

	struct morse_ap *ap = container_of(raw, struct morse_ap, raw);
	struct ieee80211_vif *vif = morse_vif_to_ieee80211_vif(ap->mors_vif);
	const u8 num_bcns_to_send_praw = vif->bss_conf.dtim_period * MORSE_RAW_DTIMS_FOR_PRAW_TX;

	lockdep_assert_held(&raw->lock);

	/*
	 * If there is a new PRAW config, we must reset the start offsets so the relative offsets
	 * are preserved, and not affected by PRAW config sequencing
	 */
	if (new_config)
		list_for_each_entry(cfg, &raw->active_praws, active_list)
			cfg->periodic.cur_start_offset = cfg->periodic.start_offset;

	raw->praw_tx_count = num_bcns_to_send_praw;
}

/**
 * morse_raw_update_praw_after_bcn() - Update active PRAWs after a beacon has been transmitted
 *
 * @raw: RAW context
 */
static void morse_raw_update_praw_after_bcn(struct morse_raw *raw)
{
	struct morse_raw_config *cfg, *tmp;
	bool kick_tx = false;

	lockdep_assert_held(&raw->lock);

	list_for_each_entry_safe(cfg, tmp, &raw->active_praws, active_list) {
		if (cfg->periodic.cur_start_offset == 0)
			cfg->periodic.cur_start_offset = cfg->periodic.periodicity - 1;
		else
			cfg->periodic.cur_start_offset--;

		/* When we wrap back to our start offset, we have gone through a period */
		if (cfg->periodic.cur_start_offset == cfg->periodic.start_offset)
			cfg->periodic.cur_validity--;

		/* PRAW has expired */
		if (cfg->periodic.cur_validity == 0) {
			/* reset values in case we want to start again */
			cfg->periodic.cur_validity = cfg->periodic.validity;
			cfg->periodic.cur_start_offset = cfg->periodic.start_offset;

			if (cfg->periodic.refresh_praw)
				kick_tx = true;
			else
				/* PRAW has expired */
				morse_raw_deactivate_config(raw, cfg);
		}
	}

	if (kick_tx)
		morse_raw_start_praw_transmission(raw, false);
}

/**
 * raw_bsearch_aid_indexes() - Binary search the AID list to find the index of the requested AID
 * or the closest index, rounding towards 0
 *
 * @aid_list: AID list to search
 * @aid: AID to find
 * Return: Index into aid_list for AID
 */
static int raw_bsearch_aid_indexes(const struct morse_aid_list *aid_list, u16 aid)
{
	int start_idx = 0;
	int end_idx = aid_list->num_aids - 1;
	int mid_idx = 0;

	MORSE_WARN_ON(FEATURE_ID_RAW, aid_list->num_aids == 0);
	if (aid_list->num_aids == 1)
		return 0;

	while (start_idx <= end_idx) {
		mid_idx = start_idx + ((end_idx - start_idx) / 2);

		if (aid_list->aids[mid_idx] == aid)
			return mid_idx;
		else if (aid_list->aids[mid_idx] > aid)
			end_idx = mid_idx - 1;
		else if (aid_list->aids[mid_idx] < aid)
			start_idx = mid_idx + 1;
		else
			break;
	}

	/* could not find the AID, so return the rounded down index */
	if (aid_list->aids[mid_idx] > aid)
		mid_idx = max(mid_idx - 1, 0);

	return mid_idx;
}

/**
 * raw_update_aid_indexes() - update the AID indexes for a RAW config
 *
 * @cfg: RAW config to update
 * @aid_list: Current AID list
 */
static void raw_update_aid_indexes(struct morse_raw_config *cfg,
		const struct morse_aid_list *aid_list)
{
	if (aid_list->num_aids) {
		cfg->start_aid_idx = raw_bsearch_aid_indexes(aid_list, cfg->start_aid);
		cfg->end_aid_idx = raw_bsearch_aid_indexes(aid_list, cfg->end_aid);

		/* Range couldnt be found, so invalidate */
		if (aid_list->aids[cfg->start_aid_idx] < cfg->start_aid) {
			cfg->start_aid_idx = INVALID_AID_VALUE;
			cfg->end_aid_idx = INVALID_AID_VALUE;
		}
	}
}

/**
 * morse_raw_refresh_aids() - Refresh AID list used for beacon spreading
 *
 * @ap: AP context
 * @raw: RAW context
 */
static void morse_raw_refresh_aids(struct morse_ap *ap, struct morse_raw *raw)
{
	struct morse_aid_list *aid_list;
	struct morse_raw_config *config_ptr;

	lockdep_assert_held(&raw->lock);

	aid_list =
		morse_generate_aid_list(ap->aid_bitmap, ap->num_stas, ap->largest_aid, GFP_KERNEL);

	if (!aid_list)
		return;

	/* reset aid list */
	kfree(raw->aid_list);

	raw->aid_list = aid_list;

	WARN_ON(aid_list->num_aids > 0 && !(u16 *)aid_list->aids);

	list_for_each_entry(config_ptr, &raw->active_raws, active_list) {
		/* only care about AID indexes for active beacon spreading RAWs */
		if (config_ptr->beacon_spreading.nominal_sta_per_beacon) {
			/* Reset indices */
			config_ptr->start_aid_idx = INVALID_AID_VALUE;
			config_ptr->end_aid_idx = INVALID_AID_VALUE;

			raw_update_aid_indexes(config_ptr, aid_list);
		}
	}
}

int morse_raw_process_rx_mgmt(struct morse *mors, struct ieee80211_vif *vif,
			struct ieee80211_sta *sta, const struct sk_buff *skb,
			struct dot11ah_ies_mask *ies_mask)
{
	struct ieee80211_mgmt *mgmt;
	struct morse_vif *mors_vif;
	struct morse_sta *msta;
	const u8 *qos_tc_ie;
	size_t qos_tc_len;
	u8 raw_priority;
	bool is_assoc_req;

	if (!mors || !vif || !sta)
		return -EINVAL;

	mors_vif = ieee80211_vif_to_morse_vif(vif);

	if (!morse_raw_is_enabled(mors_vif))
		return 0;

	mgmt = (struct ieee80211_mgmt *)skb->data;
	is_assoc_req = (ieee80211_is_assoc_req(mgmt->frame_control) ||
					ieee80211_is_reassoc_req(mgmt->frame_control));

	if (is_assoc_req && ies_mask->ies[WLAN_EID_QOS_TRAFFIC_CAPA].ptr) {
		qos_tc_ie = ies_mask->ies[WLAN_EID_QOS_TRAFFIC_CAPA].ptr;
		qos_tc_len = ies_mask->ies[WLAN_EID_QOS_TRAFFIC_CAPA].len;

		if (qos_tc_len == 0) {
			raw_priority = 0;
			MORSE_RAW_ERR(mors, "No QoS Traffic Cap UP using default: %u",
				raw_priority);
		} else {
			raw_priority =
				(*qos_tc_ie & QOS_TRAFFIC_UP_MASK) >> QOS_TRAFFIC_UP_SHIFT;
		}

		msta = (struct morse_sta *)sta->drv_priv;
		msta->raw_priority = raw_priority;
	}

	return 0;
}

/**
 * morse_raw_do_update() - Update the RAW state and regenerate the RPS IE based on AP state
 *
 * @mors_vif: Morse interface
 */
static void morse_raw_do_update(struct morse_vif *mors_vif)
{
	struct morse_raw_config *configs_list[MAX_NUM_RAWS];
	struct morse_raw_config *config_ptr;
	u8 count = 0;
	struct morse_ap *ap = mors_vif->ap;
	struct morse_raw *raw = &ap->raw;
	struct morse *mors = morse_vif_to_morse(mors_vif);
	bool include_praws = false;

	/* RPS IE should only be regenerated if RAW is enabled. */
	if (!test_bit(RAW_STATE_ENABLED, &raw->flags)) {
		MORSE_WARN_ON(FEATURE_ID_RAW, true);
		goto cleanup;
	}

	mutex_lock(&raw->lock);

	/* STAs have been added or removed, update AID list */
	if (test_and_clear_bit(RAW_STATE_REFRESH_AIDS, &raw->flags)) {
		morse_raw_refresh_aids(ap, raw);
		/* Print the AID indices and values if debug logging is enabled. */
		if (debug_mask & MORSE_MSG_DEBUG)
			morse_raw_debug_print_aid_idx(mors_vif, raw->aid_list);

		/* Start broadcasting PRAWs for the new STAs */
		morse_raw_start_praw_transmission(raw, false);
	}

	/* A beacon has been sent.
	 * 1. Update PRAWs (if any). If a PRAW is about to expire, we must
	 *    also include all active PRAWs
	 * 2. Update dynamic beacon count and reset if the dynamic RAW cycle is
	 *    completed, ie. all beacons has been sent with dynamic configs.
	 */
	if (test_and_clear_bit(RAW_STATE_BEACON_SENT, &raw->flags)) {
		if (morse_raw_has_dynamic_config(raw)) {
			raw->dynamic.current_num++;
			if (raw->dynamic.current_num >= raw->dynamic.num_beacons) {
				/* Done sending the configured RAWs. Reset the flag to skip updating
				 * next beacons with RPS IEs.
				 */
				raw->dynamic.current_num = -1;
				clear_bit(RAW_STATE_UPDATE_EACH_BEACON, &raw->flags);
			}
		}
		if (morse_raw_has_static_config(raw)) {
			morse_raw_update_praw_after_bcn(raw);
			/* Always include PRAWs if we are still transmitting them after an update
			 * (to make sure all STAs see them)
			 */
			if (raw->praw_tx_count) {
				include_praws = true;
				raw->praw_tx_count--;
			}
		}
	}

	/*
	 * Count how many RAWs exist and are enabled, starting with PRAWs
	 *
	 * Configs should already be sorted by ID
	 */

	/* Include RAWs */
	list_for_each_entry_reverse(config_ptr, &raw->active_raws, active_list) {
		if (count >= ARRAY_SIZE(configs_list)) {
			MORSE_RAW_WARN_RATELIMITED(mors,
				"Too many active RAW assignments, ID %u not included\n",
				config_ptr->id);
			continue;
		}

		if ((morse_raw_has_dynamic_config(raw) &&
		     config_ptr->dynamic.insert_at_idx == raw->dynamic.current_num) ||
		    (morse_raw_has_static_config(raw) && !morse_raw_cfg_has_bcn_idx(config_ptr)))
			configs_list[count++] = config_ptr;

		/* If including regular RAWs, must include PRAWs too */
		include_praws = true;
	}

	/* Include PRAWs if required */
	if (include_praws) {
		list_for_each_entry_reverse(config_ptr, &raw->active_praws, active_list) {
			if (count >= ARRAY_SIZE(configs_list)) {
				MORSE_RAW_WARN_RATELIMITED(mors,
					"Too many active RAW assignments, ID %u not included\n",
					config_ptr->id);
				continue;
			}

			configs_list[count++] = config_ptr;
		}
	}

	if (count) {
		/* This cast looks strange but adds some protection. */
		morse_raw_generate_rps_ie(mors_vif,
			(struct morse_raw_config * const *)configs_list, count);
		mutex_unlock(&raw->lock);

		return;
	}
cleanup:
	raw->rps_ie_len = 0;
	kfree(raw->rps_ie);
	raw->rps_ie = NULL;
	mutex_unlock(&raw->lock);
}

static void morse_raw_update_work(struct work_struct *work)
{
	struct morse_ap *ap = container_of(work, struct morse_ap, raw.update_work);

	morse_raw_do_update(ap->mors_vif);
}

/**
 * morse_raw_is_config_valid() - Check if a RAW config contains the minimum required fields
 *
 * @cfg: Config to check
 * Returns: true if valid, otherwise false
 */
static bool morse_raw_is_config_valid(struct morse_raw_config *cfg)
{
	/* Note that AID ranges are not required for the spec, but we do
	 * require it for now.
	 */
	return (cfg->slot_definition.slot_duration_us && cfg->start_aid && cfg->end_aid);
}

/**
 * morse_raw_cmd_to_config() - Convert a RAW command into a RAW configuration
 *
 * @head:		Pointer to the config TLV head
 * @len:		Length of the current TLV
 * @cfg:		A pointer to the configuration to populate
 * Returns:		True if config is valid, otherwise false
 */
static bool morse_raw_cmd_to_config(u8 *head, int len, struct morse_raw_config *cfg)
{
	union morse_cmd_raw_tlvs *tlv;
	u8 *tail;

	WARN_ON(!cfg);

	/* only support generic RAWs at the moment */
	cfg->type = IEEE80211_S1G_RPS_RAW_TYPE_GENERIC;

	tail = head + len;

	while (head < tail) {
		tlv = (union morse_cmd_raw_tlvs *)head;

		switch (tlv->tag) {
		case MORSE_CMD_RAW_TLV_TAG_SLOT_DEF:
			cfg->slot_definition.num_slots = tlv->slot_def.num_slots;
			cfg->slot_definition.slot_duration_us =
				le32_to_cpu(tlv->slot_def.raw_duration_us) /
						tlv->slot_def.num_slots;
			cfg->slot_definition.cross_slot_boundary =
				!!tlv->slot_def.cross_slot_bleed;

			head += sizeof(tlv->slot_def);
			break;

		case MORSE_CMD_RAW_TLV_TAG_GROUP:
			cfg->start_aid = le16_to_cpu(tlv->group.aid_start);
			cfg->end_aid = le16_to_cpu(tlv->group.aid_end);

			head += sizeof(tlv->group);
			break;

		case MORSE_CMD_RAW_TLV_TAG_START_TIME:
			cfg->start_time_us = le32_to_cpu(tlv->start_time.start_time_us);

			head += sizeof(tlv->start_time);
			break;

		case MORSE_CMD_RAW_TLV_TAG_PRAW:
			cfg->periodic.periodicity = tlv->praw.periodicity;
			cfg->periodic.validity = tlv->praw.validity;
			cfg->periodic.start_offset = tlv->praw.start_offset;

			cfg->periodic.cur_validity = cfg->periodic.validity;
			cfg->periodic.cur_start_offset = cfg->periodic.start_offset;

			cfg->periodic.refresh_praw = tlv->praw.refresh_on_expiry;

			head += sizeof(tlv->praw);
			break;

		case MORSE_CMD_RAW_TLV_TAG_BCN_SPREAD:
			cfg->beacon_spreading.max_spread = le16_to_cpu(tlv->bcn_spread.max_spread);
			cfg->beacon_spreading.nominal_sta_per_beacon =
					le16_to_cpu(tlv->bcn_spread.nominal_sta_per_bcn);
			cfg->beacon_spreading.last_aid = 0;

			head += sizeof(tlv->bcn_spread);
			break;
		default:
			/* unrecognised TLV */
			WARN_ON(true);
			return false;
		}
	}

	return morse_raw_is_config_valid(cfg);
}

/**
 * morse_raw_enable() - Enable RAW.
 * @mors_vif: Morse vif
 */
void morse_raw_enable(struct morse_raw *raw)
{
	set_bit(RAW_STATE_ENABLED, &raw->flags);
	schedule_work(&raw->update_work);
}

/**
 * morse_raw_disable() - Disable RAW.
 * @mors_vif: Morse vif
 */
static void morse_raw_disable(struct morse_raw *raw)
{
	clear_bit(RAW_STATE_ENABLED, &raw->flags);
	cancel_work_sync(&raw->update_work);
}

struct morse_raw_config *morse_raw_find_config_by_id(struct morse_raw *raw, u16 id)
{
	struct morse_raw_config *config;

	list_for_each_entry(config, &raw->raw_config_list, list)
		if (config->id == id)
			return config;

	return NULL;
}

struct morse_raw_config *morse_raw_create_or_find_by_id(struct morse_raw *raw, u16 id)
{
	struct morse_raw_config *config, *prev = NULL;

	lockdep_assert_held(&raw->lock);

	/* ID 0 is reserved */
	if (id == 0) {
		MORSE_WARN_ON(FEATURE_ID_RAW, true);
		return NULL;
	}

	list_for_each_entry(config, &raw->raw_config_list, list) {
		/* already exists, just return */
		if (config->id == id)
			return config;
		else if (config->id > id)
			prev = config; /* track to keep some level of sorting in master list */
	}

	config = kzalloc(sizeof(*config), GFP_KERNEL);
	if (!config)
		return NULL;

	config->id = id;

	INIT_LIST_HEAD(&config->active_list);

	if (prev)
		list_add(&config->list, &prev->list);
	else
		list_add_tail(&config->list, &raw->raw_config_list);

	return config;
}

/**
 * morse_dump_raw_config() - Dump RAW config
 *
 * @mors: Morse chip struct
 * @cfg: The RAW config to print
 */
static void morse_dump_raw_config(struct morse *mors, struct morse_raw_config *cfg)
{
	MORSE_RAW_DBG(mors, "New RAW Config: bcn_sprd:%d sta_per_bcn:%d start_time:%d",
		cfg->beacon_spreading.max_spread,
		cfg->beacon_spreading.nominal_sta_per_beacon,
		cfg->start_time_us);
	MORSE_RAW_DBG(mors, "start_aid:%d end_aid:%d num_slots:%d slot_dur:%d\n",
		cfg->start_aid,
		cfg->end_aid,
		cfg->slot_definition.num_slots,
		cfg->slot_definition.slot_duration_us);
}

/**
 * morse_raw_delete_config() - Delete a RAW config
 *
 * @raw: The global RAW context
 * @cfg: The RAW config to delete
 */
static void morse_raw_delete_config(struct morse_raw *raw, struct morse_raw_config *config)
{
	if (morse_raw_is_config_active(config))
		morse_raw_deactivate_config(raw, config);

	list_del(&config->list);
	kfree(config);
}

/**
 * morse_raw_list_add_sorted() - Add a RAW config to the specified config list, sorted by ID
 *
 * @cfg_list: The list to add the config to
 * @cfg: The RAW config to add
 */
static void morse_raw_list_add_sorted(struct list_head *cfg_list, struct morse_raw_config *cfg)
{
	struct morse_raw_config *iter;

	/* add assignment sorted by ID, to have ordering in the RAW assignments */
	list_for_each_entry(iter, cfg_list, active_list) {
		MORSE_RAW_WARN_ON(iter->id == cfg->id);

		/* iter is larger, so add before iter */
		if (iter->id > cfg->id) {
			list_add_tail(&cfg->active_list, &iter->active_list);
			return;
		}
	}

	/* Empty list, or no cfg larger. Add to end */
	list_add_tail(&cfg->active_list, cfg_list);
}

void morse_raw_activate_config(struct morse_raw *raw, struct morse_raw_config *cfg)
{
	if (morse_raw_is_config_active(cfg))
		return;

	if (morse_raw_cfg_is_periodic(cfg))
		morse_raw_list_add_sorted(&raw->active_praws, cfg);
	else
		morse_raw_list_add_sorted(&raw->active_raws, cfg);

	if (morse_raw_cfg_has_bcn_idx(cfg))
		raw->configs.num_dynamic++;
	else
		raw->configs.num_static++;
}

void morse_raw_deactivate_config(struct morse_raw *raw, struct morse_raw_config *cfg)
{
	if (morse_raw_cfg_has_bcn_idx(cfg)) {
		MORSE_WARN_ON(FEATURE_ID_RAW, raw->configs.num_dynamic == 0);
		raw->configs.num_dynamic--;
	} else {
		MORSE_WARN_ON(FEATURE_ID_RAW, raw->configs.num_static == 0);
		raw->configs.num_static--;
	}

	list_del_init(&cfg->active_list);
}

bool morse_raw_is_config_active(struct morse_raw_config *cfg)
{
	return !list_empty(&cfg->active_list);
}

/**
 * morse_raw_process_config_group() - Process the priority group configurations from the command
 *
 * @mors_vif:	Morse VIF structure
 * @dynamic_conf:	Config group entry for dynamic RAW
 * @static_conf:	Config group entry for static RAW
 * Return: 0 if command was processed successfully, otherwise error code
 */
static int morse_raw_process_config_group(struct morse_vif *mors_vif, struct morse_raw *raw,
				   struct morse_cmd_raw_tlv_dyn_config *dynamic_conf,
				   struct morse_cmd_req_config_raw *static_conf)
{
	struct morse_raw_config *config, *tmp;
	u8 *tlv_head;
	u16 flags;
	u16 id;
	int len;
	int ret = 0;
	bool enable = false;
	u16 beacon_idx = U16_MAX;
	struct morse *mors = morse_vif_to_morse(mors_vif);

	if (!dynamic_conf && !static_conf)
		return -ENOENT;

	if (dynamic_conf) {
		beacon_idx = le16_to_cpu(dynamic_conf->index);
		len = le16_to_cpu(dynamic_conf->len);
		flags = MORSE_CMD_CFG_RAW_FLAG_ENABLE | MORSE_CMD_CFG_RAW_FLAG_UPDATE;
		id = le16_to_cpu(dynamic_conf->id);
		tlv_head = dynamic_conf->variable;

		MORSE_RAW_DBG(mors, "Dynamic RAW CMD: %d %x %d\n", id, flags, beacon_idx);
	} else {
		len = le16_to_cpu(static_conf->hdr.len) -
			((sizeof(*static_conf) - sizeof(static_conf->hdr)));
		flags = le32_to_cpu(static_conf->flags);
		id = le16_to_cpu(static_conf->id);
		tlv_head = static_conf->variable;

		MORSE_RAW_DBG(mors, "RAW CMD: %d %x\n", id, flags);
	}

	if (id >= RAW_INTERNAL_ID_OFFSET)
		return -EPERM;

	enable = ((flags & MORSE_CMD_CFG_RAW_FLAG_ENABLE) != 0);

	/* Pause and unpause the BSS stats when static RAW is configured. Pause will stop the BSS
	 * stats events and the dynamic RAW commands to and fro of smart manager. Resume will
	 * start the BSS stats events to smart manager and dynamic RAW commands will resume.
	 */
	if (enable && static_conf)
		morse_bss_stats_pause(mors_vif);
	else if (!enable && static_conf)
		morse_bss_stats_resume(mors_vif);

	if (id == 0) {
		if (enable)
			morse_raw_enable(raw);
		else
			morse_raw_disable(raw);

		if (flags & MORSE_CMD_CFG_RAW_FLAG_DELETE) {
			MORSE_RAW_DBG(mors, "RAW DELETE CMD: %d %x\n", id, flags);
			list_for_each_entry_safe(config, tmp, &raw->raw_config_list, list)
				morse_raw_delete_config(raw, config);
		}
		return 0;
	}

	if (flags & MORSE_CMD_CFG_RAW_FLAG_UPDATE) {
		config = morse_raw_create_or_find_by_id(raw, id);
		if (!config)
			return -ENOMEM;

		if (!morse_raw_cmd_to_config(tlv_head, len, config)) {
			/* Allowed to have invalid configs provided they are disabled */
			if (enable)
				ret = -EINVAL;

			/* config is not valid, disable it */
			enable = false;
		} else {
			morse_dump_raw_config(mors, config);
		}
	} else {
		config = morse_raw_find_config_by_id(raw, id);
		if (!config) {
			if (enable) {
				MORSE_RAW_WARN(mors,
					"Trying to enable a RAW without configuration\n");
				ret = -ENOENT;
			}
			return ret;
		}

		if (flags & MORSE_CMD_CFG_RAW_FLAG_DELETE) {
			morse_raw_delete_config(raw, config);
			config = NULL;
		}
	}

	if (config) {
		config->dynamic.insert_at_idx = beacon_idx;

		if (enable)
			morse_raw_activate_config(raw, config);
		else
			morse_raw_deactivate_config(raw, config);
	}

	return ret;
}

/**
 * morse_raw_process_dynamic_cmd() - Execute command to enable/disable/configure dynamic RAW
 *
 * @mors_vif:	Morse VIF structure
 * @raw:	RAW context
 * @req:	Request from smart manager
 * Return: 0 if command was processed successfully, otherwise error code
 */
static int morse_raw_process_dynamic_cmd(struct morse_vif *mors_vif, struct morse_raw *raw,
				struct morse_cmd_req_config_raw *req)
{
	union morse_cmd_raw_tlvs *tlv;
	int num_configs;
	u8 *head;
	u8 *tail;
	int ret = 0;

	mutex_lock(&raw->lock);

	if (morse_raw_has_static_config(raw)) {
		mutex_unlock(&raw->lock);
		return 0;
	}

	/* Re-initialize to remove any previously configured RAW from config list */
	morse_dynamic_raw_init(mors_vif);

	head = req->variable;
	tail = ((uint8_t *)req) + le16_to_cpu(req->hdr.len) + sizeof(req->hdr);

	while (head < tail) {
		struct morse_cmd_raw_tlv_dyn_config *config;

		tlv = (union morse_cmd_raw_tlvs *)head;

		switch (tlv->tag) {
		case MORSE_CMD_RAW_TLV_TAG_DYN_GLOBAL:
			raw->dynamic.num_beacons =
					le16_to_cpu(tlv->dyn_global.num_bcn_indexes);
			num_configs = le16_to_cpu(tlv->dyn_global.num_configs);
			if (!num_configs) {
				mutex_unlock(&raw->lock);
				return 0;
			}
			head += sizeof(tlv->dyn_global);
			break;
		case MORSE_CMD_RAW_TLV_TAG_DYN_CONFIG:
			config = (struct morse_cmd_raw_tlv_dyn_config *)tlv;
			if (config) {
				ret = morse_raw_process_config_group(mors_vif, raw,
								     config, NULL);
				head += le16_to_cpu(config->len) + sizeof(tlv->dyn_config);
			}
			break;
		case MORSE_CMD_RAW_TLV_TAG_SLOT_DEF:
		case MORSE_CMD_RAW_TLV_TAG_GROUP:
		case MORSE_CMD_RAW_TLV_TAG_START_TIME:
		case MORSE_CMD_RAW_TLV_TAG_PRAW:
		case MORSE_CMD_RAW_TLV_TAG_BCN_SPREAD:
			/* Config TLVs to be processed with config group */
			break;
		default:
			/* unrecognised TLV */
			WARN_ON(true);
			ret = -EINVAL;
			break;
		}
		if (ret < 0)
			break;
	}

	/* By default, RPS IE / state is the same for consecutive beacons */
	clear_bit(RAW_STATE_UPDATE_EACH_BEACON, &raw->flags);
	if (!list_empty(&raw->active_raws))
		set_bit(RAW_STATE_UPDATE_EACH_BEACON, &raw->flags);

	if (ret == 0 && !morse_raw_is_enabled(mors_vif))
		morse_raw_enable(raw);

	mutex_unlock(&raw->lock);

	morse_raw_trigger_update(mors_vif, false);

	return ret;
}

/**
 * morse_raw_process_static_cmd() - Execute command to enable/disable/configure static RAW
 *
 * @mors_vif:	Morse VIF structure
 * @raw:	RAW context
 * @req:	Request from hostapd or morsectrl
 * Return: 0 if command was processed successfully, otherwise error code
 */
static int morse_raw_process_static_cmd(struct morse_vif *mors_vif, struct morse_raw *raw,
		struct morse_cmd_req_config_raw *req)
{
	int ret = 0;
	struct morse_raw_config *config;

	mutex_lock(&raw->lock);

	/* Re-initialize to remove any previously configured dynamic RAW from config list */
	morse_dynamic_raw_init(mors_vif);

	ret = morse_raw_process_config_group(mors_vif, raw, NULL, req);

	/* By default, RPS IE / state is the same for consecutive beacons */
	clear_bit(RAW_STATE_UPDATE_EACH_BEACON, &raw->flags);
	if (!list_empty(&raw->active_praws)) {
		/* RAW config updates require PRAWs to be retransmitted */
		morse_raw_start_praw_transmission(raw, true);
		/* PRAWs require updates on each beacon */
		set_bit(RAW_STATE_UPDATE_EACH_BEACON, &raw->flags);
	} else {
		list_for_each_entry(config, &raw->active_raws, list) {
			/* Beacon spreading requires updates to the RPS IE on every beacon */
			if (config->beacon_spreading.nominal_sta_per_beacon) {
				set_bit(RAW_STATE_UPDATE_EACH_BEACON, &raw->flags);
				break;
			}
		}
	}

	mutex_unlock(&raw->lock);

	/* Update RPS IE with new configuration. */
	morse_raw_trigger_update(mors_vif, false);

	return ret;
}

int morse_raw_process_cmd(struct morse_vif *mors_vif, struct morse_cmd_req_config_raw *req)
{
	struct morse_raw *raw;
	int ret = 0;
	struct ieee80211_vif *vif = morse_vif_to_ieee80211_vif(mors_vif);
	struct morse *mors = morse_vif_to_morse(mors_vif);
	bool is_dynamic_req = ((le32_to_cpu(req->flags) & MORSE_CMD_CFG_RAW_FLAG_DYNAMIC) != 0);

	if (vif->type != NL80211_IFTYPE_AP) {
		MORSE_RAW_INFO(mors, "RAW not supported on non-AP interfaces\n");
		return -ENOTSUPP;
	}

	raw = &mors_vif->ap->raw;

	if (is_dynamic_req)
		ret = morse_raw_process_dynamic_cmd(mors_vif, raw, req);
	else
		ret = morse_raw_process_static_cmd(mors_vif, raw, req);

	return ret;
}

void morse_raw_beacon_sent(struct morse_vif *mors_vif)
{
	/* Ok to assume this is an AP interface, considering its sending beacons */
	struct morse_raw *raw = &mors_vif->ap->raw;

	if (morse_raw_is_enabled(mors_vif) && test_bit(RAW_STATE_UPDATE_EACH_BEACON, &raw->flags)) {
		/* If we were too slow updating, validity may be out of sync for PRAWs */
		MORSE_RAW_WARN_ON(test_bit(RAW_STATE_BEACON_SENT, &raw->flags));
		set_bit(RAW_STATE_BEACON_SENT, &raw->flags);

		schedule_work(&raw->update_work);
	}
}

void morse_raw_trigger_update(struct morse_vif *mors_vif, bool refresh_aids)
{
	struct morse_raw *raw;

	if (!morse_raw_is_enabled(mors_vif))
		return;

	raw = &mors_vif->ap->raw;

	if (!raw->aid_list || refresh_aids)
		set_bit(RAW_STATE_REFRESH_AIDS, &raw->flags);

	schedule_work(&raw->update_work);
}

bool morse_raw_is_enabled(struct morse_vif *mors_vif)
{
	return mors_vif && mors_vif->ap && test_bit(RAW_STATE_ENABLED, &mors_vif->ap->raw.flags);
}

int morse_raw_init(struct morse_vif *mors_vif, bool enable)
{
	struct morse_raw *raw;
	struct morse *mors;

	if (!mors_vif || !mors_vif->ap)
		return -ENOTSUPP;

	mors = morse_vif_to_morse(mors_vif);
	raw = &mors_vif->ap->raw;

	memset(raw, 0, sizeof(*raw));

	mutex_init(&raw->lock);
	INIT_WORK(&raw->update_work, morse_raw_update_work);
	INIT_LIST_HEAD(&raw->raw_config_list);
	INIT_LIST_HEAD(&raw->active_raws);
	INIT_LIST_HEAD(&raw->active_praws);

	if (enable)
		morse_raw_enable(raw);
	else
		morse_raw_disable(raw);

	MORSE_RAW_INFO(mors, "RAW %s\n", enable ? "enabled" : "disabled");

	return 0;
}

void morse_dynamic_raw_init(struct morse_vif *mors_vif)
{
	struct morse_raw *raw;
	struct morse_raw_config *config, *tmp;

	if (!mors_vif || !mors_vif->ap)
		return;

	raw = &mors_vif->ap->raw;

	if (!morse_raw_has_dynamic_config(raw))
		return;

	/* Cleanup dynamic RAW configurations */
	raw->dynamic.num_beacons = 0;
	raw->dynamic.current_num = 0;

	/* Delete any previous active dynamic RAW config */
	list_for_each_entry_safe(config, tmp, &raw->raw_config_list, list) {
		if (morse_raw_cfg_has_bcn_idx(config))
			morse_raw_delete_config(raw, config);
	}
}

void morse_raw_finish(struct morse_vif *mors_vif)
{
	struct morse_raw *raw;
	struct morse_raw_config *config, *tmp;

	if (!mors_vif || !mors_vif->ap)
		return;

	raw = &mors_vif->ap->raw;

	morse_raw_disable(raw);

	/* Free RAW and clean up */
	raw->rps_ie_len = 0;
	kfree(raw->rps_ie);
	raw->rps_ie = NULL;

	list_for_each_entry_safe(config, tmp, &raw->raw_config_list, list)
		morse_raw_delete_config(raw, config);
}
