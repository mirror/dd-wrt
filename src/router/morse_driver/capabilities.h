#ifndef _MORSE_CAPABILITIES_H_
#define _MORSE_CAPABILITIES_H_

/*
 * Copyright 2017-2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include <linux/bitmap.h>
#include "morse.h"
#include "misc.h"

#define MORSE_CAPS_MAX_FW_VAL  (128)

/**
 * enum morse_caps_flags - Capabilities of the morse device.
 *
 * A subset of flags are signalled from the hw as reported
 * by the fw table. These flags appear between:
 * MORSE_CAPS_FW_START <-> MORSE_CAPS_FW_END
 */
enum morse_caps_flags {
	MORSE_CAPS_FW_START = 0,
	MORSE_CAPS_2MHZ = MORSE_CAPS_FW_START,
	MORSE_CAPS_4MHZ,
	MORSE_CAPS_8MHZ,
	MORSE_CAPS_16MHZ,
	MORSE_CAPS_SGI,
	MORSE_CAPS_S1G_LONG,
	MORSE_CAPS_TRAVELING_PILOT_ONE_STREAM,
	MORSE_CAPS_TRAVELING_PILOT_TWO_STREAM,
	MORSE_CAPS_MU_BEAMFORMEE,
	MORSE_CAPS_MU_BEAMFORMER,
	MORSE_CAPS_RD_RESPONDER,
	MORSE_CAPS_STA_TYPE_SENSOR,
	MORSE_CAPS_STA_TYPE_NON_SENSOR,
	MORSE_CAPS_GROUP_AID,
	MORSE_CAPS_NON_TIM,
	MORSE_CAPS_TIM_ADE,
	MORSE_CAPS_BAT,
	MORSE_CAPS_DYNAMIC_AID,
	MORSE_CAPS_UPLINK_SYNC,
	MORSE_CAPS_FLOW_CONTROL,
	MORSE_CAPS_AMPDU,
	MORSE_CAPS_AMSDU,
	MORSE_CAPS_1MHZ_CONTROL_RESPONSE_PREAMBLE,
	MORSE_CAPS_PAGE_SLICING,
	MORSE_CAPS_RAW,
	MORSE_CAPS_MCS8,
	MORSE_CAPS_MCS9,
	MORSE_CAPS_ASYMMETRIC_BA_SUPPORT,
	MORSE_CAPS_DAC,
	MORSE_CAPS_CAC,
	MORSE_CAPS_TXOP_SHARING_IMPLICIT_ACK,
	MORSE_CAPS_NDP_PSPOLL,
	MORSE_CAPS_FRAGMENT_BA,
	MORSE_CAPS_OBSS_MITIGATION,
	MORSE_CAPS_TMP_PS_MODE_SWITCH,
	MORSE_CAPS_SECTOR_TRAINING,
	MORSE_CAPS_UNSOLICIT_DYNAMIC_AID,
	MORSE_CAPS_NDP_BEAMFORMING_REPORT,
	MORSE_CAPS_MCS_NEGOTIATION,
	MORSE_CAPS_DUPLICATE_1MHZ,
	MORSE_CAPS_TACK_AS_PSPOLL,
	MORSE_CAPS_PV1,
	MORSE_CAPS_TWT_RESPONDER,
	MORSE_CAPS_TWT_REQUESTER,
	MORSE_CAPS_BDT,
	MORSE_CAPS_TWT_GROUPING,
	MORSE_CAPS_LINK_ADAPTATION_WO_NDP_CMAC,
	MORSE_CAPS_LONG_MPDU,
	MORSE_CAPS_TXOP_SECTORIZATION,
	MORSE_CAPS_GROUP_SECTORIZATION,
	MORSE_CAPS_HTC_VHT,
	MORSE_CAPS_HTC_VHT_MFB,
	MORSE_CAPS_HTC_VHT_MRQ,
	MORSE_CAPS_2SS,
	MORSE_CAPS_3SS,
	MORSE_CAPS_4SS,
	MORSE_CAPS_SU_BEAMFORMEE,
	MORSE_CAPS_SU_BEAMFORMER,
	MORSE_CAPS_RX_STBC,
	MORSE_CAPS_TX_STBC,
	MORSE_CAPS_RX_LDPC,
	MORSE_CAPS_HW_FRAGMENT,
	MORSE_CAPS_FW_END = MORSE_CAPS_MAX_FW_VAL,

	/* Capabilities not filled by FW need to be inserted after
	 * MORSE_CAPS_FW_END. These capabilities are allowed to move
	 * around within the enum (for example if the CAPS_FW subset expands).
	 *
	 * As such, their internal integer representation should not be used
	 * directly when sending information on air.
	 */

	MORSE_CAPS_LAST = MORSE_CAPS_FW_END,
};

/* CAPABILITIES_FLAGS_WIDTH = ceil(MORSE_CAPS_LAST / 32) */
#define CAPABILITIES_FLAGS_WIDTH   (4)

struct morse_caps {
	u32 flags[CAPABILITIES_FLAGS_WIDTH];
	u8 ampdu_mss;
	u8 beamformee_sts_capability;
	u8 number_sounding_dimensions;
	u8 maximum_ampdu_length_exponent;
	u8 morse_mmss_offset;
};

/* Shorten the verbosity for referencing the firmware flags */
#define MORSE_CAPAB_SUPPORTED(MORSE_CAPS, CAPABILITY)\
	morse_caps_supported(MORSE_CAPS, \
			   MORSE_CAPS_##CAPABILITY)

/**
 * @brief Check if a capability is supported.
 *
 * @param mors The target morse capabilities
 * @param flag The capability flag to check
 * @return true if the capability is supported, false if otherwise
 */
static inline bool morse_caps_supported(struct morse_caps *caps, enum morse_caps_flags flag)
{
	const unsigned long *flags_ptr = (unsigned long *)caps->flags;

	return test_bit(flag, flags_ptr);
}

#endif /* !_MORSE_CAPABILITIES_H_ */
