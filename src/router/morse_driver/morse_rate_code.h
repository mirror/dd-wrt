/*
 * Copyright 2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#pragma once

/**
 * @file morse_rate_code.h
 * @brief Morse rate code definitions and utility functions.
 */
#include <linux/types.h>
#include "misc.h"

enum dot11_bandwidth {
	DOT11_BANDWIDTH_1MHZ = 0,
	DOT11_BANDWIDTH_2MHZ = 1,
	DOT11_BANDWIDTH_20MHZ = DOT11_BANDWIDTH_2MHZ,
	DOT11_BANDWIDTH_4MHZ = 2,
	DOT11_BANDWIDTH_40MHZ = DOT11_BANDWIDTH_4MHZ,
	DOT11_BANDWIDTH_8MHZ = 3,
	DOT11_BANDWIDTH_80MHZ = DOT11_BANDWIDTH_8MHZ,
	DOT11_BANDWIDTH_16MHZ = 4,
	DOT11_BANDWIDTH_160MHZ = DOT11_BANDWIDTH_16MHZ,

	DOT11_MAX_BANDWIDTH = DOT11_BANDWIDTH_16MHZ,
	DOT11_INVALID_BANDWIDTH = 5
};

enum morse_rate_preamble {
	/** S1G LONG format (with SIG-A and SIG-B) */
	MORSE_RATE_PREAMBLE_S1G_LONG = 0,
	/** This is the most common format used */
	MORSE_RATE_PREAMBLE_S1G_SHORT = 1,
	/** S1G 1M format */
	MORSE_RATE_PREAMBLE_S1G_1M = 2,
	/** 11b frames at 1Mbps and 2Mbps */
	MORSE_RATE_PREAMBLE_DSSS_LONG = 3,
	/** 11b (CCK) frames at 2Mbps, 5.5Mbps and 11Mbps */
	MORSE_RATE_PREAMBLE_DSSS_SHORT = 4,
	/** 11g frames upto 54Mbps */
	MORSE_RATE_PREAMBLE_ERP = 5,
	/** HT-Mixed field mode (no support for greenfield) */
	MORSE_RATE_PREAMBLE_HT = 6,

	MORSE_RATE_MAX_PREAMBLE = MORSE_RATE_PREAMBLE_HT,
	MORSE_RATE_INVALID_PREAMBLE = 7
};

enum dot11b_mcs {
	DOT11B_DSSS_1M = 0,
	DOT11B_DSSS_2M = 1,
	DOT11B_CCK_5_5_M = 2,
	DOT11B_CCK_11M = 3,

	DOT11B_MAX_MCS = DOT11B_CCK_11M
};

enum dot11g_mcs {
	DOT11G_OFDM_6M = 0,
	DOT11G_OFDM_9M = 1,
	DOT11G_OFDM_12M = 2,
	DOT11G_OFDM_18M = 3,
	DOT11G_OFDM_24M = 4,
	DOT11G_OFDM_36M = 5,
	DOT11G_OFDM_48M = 6,
	DOT11G_OFDM_54M = 7,

	DOT11G_MAX_MCS = DOT11G_OFDM_54M
};

/**
 * Morse RATE code is a 4-byte variable consisting of the following format
 * +-----+-----------+---+---+---+---+---+---+-------+-----------+---------+-----------+-----------+
 * | Rsvd|  DUP.     | D |1M.|T .| S | C | R |Reservd| Bandwidth | NSS     | MCS index |  PREAMBLE |
 * |     |Bandwidth  | U |Res|P  | G | T | T |       |           |         |           |           |
 * +-----|           | P.|   |   | I | S | S |-------|-----------|---------|-----------|-----------|
 * |b31|.|b24|b23|b22|b21|b20|b19|b18|b17|b16|b15|b14|b13|b12|b11|b10|b9|b8|b7|b6|b5|b4|b3|b2|b1|b0
 *
 * @param preamble represents the value of the enum enum morse_rate_preamble (supports 11bgn/ah)
 * @param mcs index represents the value of the MCS for 11n/ah. for 11b/g there is mapping of a
 *        pseudo MCS index to the RATE field defined by enum dot11b_mcs and dot11g_mcs.
 * @param nss index represents the number of spatial streams - 1. i.e. 0 => NSS 1, 1 => NSS 2 so on
 * @param bw index represents the value of the enum enum dot11_bandwidth. The PHY
 *				 throughput/duration depends on this bandwidth.
 * @param rts indicates if RTS protection is enabled on the packet using this rate code.
 * @param cts indicates if CTS2SELF protection is enabled on the packet using this rate code.
 * @param sgi indicates if short guard interval is used when this rate code is applied.
 * @param tp indicates if travelling pilots is used. (only for 802.11ah).
 * @param ctrl_resp_1mhz indicates if 1MHz control response is expected from the destination.
 * @param dup indicates if this packet is intended to be sent in duplicate mode.
 * @param dup_bw index representing the BW across which this packet is duplicated.
 *        valid only when duplicate bit is set above.
 * All reserved bits should be set to zero.
 *
 * To further illustrate the differences between operating bandwidth, (tx/rx) bandwidth index
 * and (tx/rx) duplicate bandwidth index or channel bandwidth consider the example below:
 *
 * A payload (RTS) rx/tx in 2MHZ over 4MHZ in duplicates. Our OBW is 8MHZ.
 * _______________________________________________________________________________
 * |                                                                              |
 * |                       Operating Bandwidth: 8MHz                              |
 * |______________________________________________________________________________|
 * |                                          |                                   |
 * |             DUP BW index: 4MHz           |            (EMPTY)                |
 * |____________________ _____________________|___________________________________|
 * |                    |                     |                |                  |
 * |  BW index: 2MHz    |    duplicated       |                |                  |
 * |  (payload signal)  |  (payload signal)   |            (EMPTY)                |
 * |____________________|_____________________|________________|__________________|
 */
typedef __le32 morse_rate_code_t;

#define MORSE_RATECODE_PREAMBLE                    (0x0000000F)
#define MORSE_RATECODE_MCS_INDEX                   (0x000000F0)
#define MORSE_RATECODE_NSS_INDEX                   (0x00000700)
#define MORSE_RATECODE_BW_INDEX                    (0x00003800)
#define MORSE_RATECODE_RTS_FLAG                    (0x00010000)
#define MORSE_RATECODE_CTS2SELF_FLAG               (0x00020000)
#define MORSE_RATECODE_SHORT_GI_FLAG               (0x00040000)
#define MORSE_RATECODE_TRAV_PILOTS_FLAG            (0x00080000)
#define MORSE_RATECODE_CTRL_RESP_1MHZ_FLAG         (0x00100000)
#define MORSE_RATECODE_DUP_FORMAT_FLAG             (0x00200000)
#define MORSE_RATECODE_DUP_BW_INDEX                (0x01C00000)

/**
 * @brief Get the preamble value from the morse rate code.
 *
 * @param[in] rc Morse rate code.
 * @return Morse rate preamble value.
 */
static inline enum morse_rate_preamble morse_ratecode_preamble_get(morse_rate_code_t rc)
{
	return (enum morse_rate_preamble)(BMGET(le32_to_cpu(rc), MORSE_RATECODE_PREAMBLE));
}

/**
 * @brief Get the MCS index value from the morse rate code.
 *
 * @note: For modes such as 11bg where MCS index strictly does not exist, a pseudo index is returned
 * which maps to the RATE defined in the standard.
 *
 * @param[in] rc Morse rate code.
 * @return MCS index value. (0 to maximum MCS supported).
 */
static inline u8 morse_ratecode_mcs_index_get(morse_rate_code_t rc)
{
	return BMGET(le32_to_cpu(rc), MORSE_RATECODE_MCS_INDEX);
}

/**
 * @brief Get the NSS index value from the morse rate code.
 *
 * @param[in] rc Morse rate code.
 * @return NSS index value.
 */
static inline u8 morse_ratecode_nss_index_get(morse_rate_code_t rc)
{
	return BMGET(le32_to_cpu(rc), MORSE_RATECODE_NSS_INDEX);
}

/**
 * @brief Get the bandwidth index value from the morse rate code.
 *
 * @param[in] rc Morse rate code.
 * @return Morse rate bandwidth index value.
 */
static inline enum dot11_bandwidth morse_ratecode_bw_index_get(morse_rate_code_t rc)
{
	return (enum dot11_bandwidth)(BMGET(le32_to_cpu(rc), MORSE_RATECODE_BW_INDEX));
}

/**
 * @brief Get the RTS flag value from the morse rate code.
 *
 * @param[in] rc Morse rate code.
 * @return true if RTS is enabled, otherwise false.
 */
static inline bool morse_ratecode_rts_get(morse_rate_code_t rc)
{
	return BMGET(le32_to_cpu(rc), MORSE_RATECODE_RTS_FLAG);
}

/**
 * @brief Get the CTS-to-self flag value from the morse rate code.
 *
 * @param[in] rc Morse rate code.
 * @return true if CTS-to-self is enabled, otherwise false.
 */
static inline bool morse_ratecode_cts2self_get(morse_rate_code_t rc)
{
	return BMGET(le32_to_cpu(rc), MORSE_RATECODE_CTS2SELF_FLAG);
}

/**
 * @brief Get the short guard interval flag value from the morse rate code.
 *
 * @param[in] rc Morse rate code.
 * @return true if short guard interval is enabled, otherwise false.
 */
static inline bool morse_ratecode_sgi_get(morse_rate_code_t rc)
{
	return BMGET(le32_to_cpu(rc), MORSE_RATECODE_SHORT_GI_FLAG);
}

/**
 * @brief Get the travelling pilots flag value from the morse rate code.
 *
 * @param[in] rc Morse rate code.
 * @return true if travelling pilots are enabled, otherwise false.
 */
static inline bool morse_ratecode_trav_pilots_get(morse_rate_code_t rc)
{
	return BMGET(le32_to_cpu(rc), MORSE_RATECODE_TRAV_PILOTS_FLAG);
}

/**
 * @brief Get the control response 1MHz flag value from the morse rate code.
 *
 * @param[in] rc Morse rate code.
 * @return true if the control response 1MHz flag is enabled, otherwise false.
 */
static inline bool morse_ratecode_ctrl_resp_1mhz_get(morse_rate_code_t rc)
{
	return BMGET(le32_to_cpu(rc), MORSE_RATECODE_CTRL_RESP_1MHZ_FLAG);
}

/**
 * @brief Get the duplicate format flag value from the morse rate code.
 *
 * @param[in] rc Morse rate code.
 * @return true if need to be sent in duplicate format, otherwise false.
 */
static inline bool morse_ratecode_dup_format_get(morse_rate_code_t rc)
{
	return BMGET(le32_to_cpu(rc), MORSE_RATECODE_DUP_FORMAT_FLAG);
}

/**
 * @brief Get the duplicate bandwidth index value from the morse rate code.
 *
 * @param[in] rc Morse rate code.
 * @return enum dot11_bandwidth index corresponding to the duplicate bandwidth.
 */
static inline enum dot11_bandwidth morse_ratecode_dup_bw_index_get(morse_rate_code_t rc)
{
	return (enum dot11_bandwidth)(BMGET(le32_to_cpu(rc), MORSE_RATECODE_DUP_BW_INDEX));
}

/**
 * @brief Initialize a morse rate code with given parameters. Can be used for global initialization
 *
 * @param[in] bw_index Bandwidth index.
 * @param[in] nss_index Number of spatial streams index.
 * @param[in] mcs_index MCS index.
 * @param[in] preamble morse_rate_preamble type.
 *
 * @return morse_rate_code_t
 */
#define MORSE_RATECODE_INIT(bw_index, nss_index, mcs_index, preamble)           \
				((BMSET((bw_index), MORSE_RATECODE_BW_INDEX))     |  \
				(BMSET((nss_index), MORSE_RATECODE_NSS_INDEX))    |  \
				(BMSET((mcs_index), MORSE_RATECODE_MCS_INDEX))    |  \
				(preamble))

/**
 * @brief Initialize a morse rate code with given parameters.
 *
 * @param[in] bw_index Bandwidth index.
 * @param[in] nss_index Number of spatial streams index.
 * @param[in] mcs_index MCS index.
 * @param[in] preamble enum morse_rate_preamble type.
 *
 * @return morse_rate_code_t
 */
static inline morse_rate_code_t morse_ratecode_init(enum dot11_bandwidth bw_index,
						    u32 nss_index,
						    u32 mcs_index,
						    enum morse_rate_preamble preamble)
{
	return cpu_to_le32(MORSE_RATECODE_INIT(bw_index, nss_index, mcs_index, preamble));
}

/**
 * @brief Set the preamble type in a morse rate code.
 *
 * @param[in,out] rc Pointer to Morse rate code.
 * @param[in] preamble enum morse_rate_preamble type.
 */
static inline void morse_ratecode_preamble_set(morse_rate_code_t *rc,
					       enum morse_rate_preamble preamble)
{
	*rc = cpu_to_le32((le32_to_cpu(*rc) &
		~(MORSE_RATECODE_PREAMBLE)) | BMSET(preamble, MORSE_RATECODE_PREAMBLE));
}

/**
 * @brief Set the mcs_index in a morse rate code.
 *
 * @param[in,out] rc Pointer to Morse rate code.
 * @param[in] mcs_index based on preamble type/phy mode.
 */
static inline void morse_ratecode_mcs_index_set(morse_rate_code_t *rc, u32 mcs_index)
{
	*rc = cpu_to_le32((le32_to_cpu(*rc) &
		~(MORSE_RATECODE_MCS_INDEX)) | BMSET(mcs_index, MORSE_RATECODE_MCS_INDEX));
}

/**
 * @brief Set the nss in a morse rate code.
 *
 * @param[in,out] rc Pointer to Morse rate code.
 * @param[in] nss_index based on preamble type/phy mode.
 */
static inline void morse_ratecode_nss_index_set(morse_rate_code_t *rc, u32 nss_index)
{
	*rc = cpu_to_le32((le32_to_cpu(*rc) &
		~(MORSE_RATECODE_NSS_INDEX)) | BMSET(nss_index, MORSE_RATECODE_NSS_INDEX));
}

/**
 * @brief Set the BW index in a morse rate code.
 *
 * @param[in,out] rc Pointer to Morse rate code.
 * @param[in] bw_index used for modulation. See also DUP BW index for channel bandwidth.
 */
static inline void morse_ratecode_bw_index_set(morse_rate_code_t *rc,
					       enum dot11_bandwidth bw_index)
{
	*rc = cpu_to_le32((le32_to_cpu(*rc) &
		~(MORSE_RATECODE_BW_INDEX)) | BMSET(bw_index, MORSE_RATECODE_BW_INDEX));
}

/**
 * @brief Set the bw index for a S1G rate code and update the preamble based on s1g rules.
 *
 * @param[in,out] rc Pointer to Morse rate code.
 * @param[in] bw_index used for modulation. See also DUP bw index for channel bandwidth.
 */
static inline void morse_ratecode_update_s1g_bw_preamble(morse_rate_code_t *rc,
							 enum dot11_bandwidth bw_index)
{
	/* S1G_LONG is used only for MU transmissions and currently not supported */
	enum morse_rate_preamble pream = MORSE_RATE_PREAMBLE_S1G_SHORT;

	if (bw_index == DOT11_BANDWIDTH_1MHZ)
		pream = MORSE_RATE_PREAMBLE_S1G_1M;

	morse_ratecode_preamble_set(rc, pream);
	morse_ratecode_bw_index_set(rc, bw_index);
}

/**
 * @brief Set the duplicate bw index in a morse rate code.
 *
 * @param[in,out] rc Pointer to Morse rate code.
 * @param[in] dup_bw_index over which the signal is spread. See also bw index for modulation
 *					 bandwidth.
 */
static inline void morse_ratecode_dup_bw_index_set(morse_rate_code_t *rc,
						   enum dot11_bandwidth dup_bw_index)
{
	*rc = ((*rc & ~(MORSE_RATECODE_DUP_BW_INDEX)) |
	       BMSET(dup_bw_index, MORSE_RATECODE_DUP_BW_INDEX));
}

/**
 * @brief Enable RTS protection when using this morse rate code.
 *
 * @param[in,out] rc Pointer to Morse rate code.
 */
static inline void morse_ratecode_enable_rts(morse_rate_code_t *rc)
{
	*rc |= cpu_to_le32(MORSE_RATECODE_RTS_FLAG);
}

/**
 * @brief Enable CTS2SELF protection when using this morse rate code.
 *
 * @param[in,out] rc Pointer to Morse rate code.
 */
static inline void morse_ratecode_enable_cts2self(morse_rate_code_t *rc)
{
	*rc |= cpu_to_le32(MORSE_RATECODE_CTS2SELF_FLAG);
}

/**
 * @brief Enable RTS protection when using a morse rate code.
 *
 * @param[in,out] rc Pointer to Morse rate code.
 */
static inline void morse_ratecode_enable_ctrl_resp_1mhz(morse_rate_code_t *rc)
{
	*rc |= cpu_to_le32(MORSE_RATECODE_CTRL_RESP_1MHZ_FLAG);
}

/**
 * @brief Enable short gurard interval when using a morse rate code.
 *
 * @param[in,out] rc Pointer to Morse rate code.
 */
static inline void morse_ratecode_enable_sgi(morse_rate_code_t *rc)
{
	*rc |= cpu_to_le32(MORSE_RATECODE_SHORT_GI_FLAG);
}

/**
 * @brief Disable short guard interval when using a morse rate code.
 *
 * @param[in,out] rc Pointer to Morse rate code.
 */
static inline void morse_ratecode_disable_sgi(morse_rate_code_t *rc)
{
	*rc &= ~cpu_to_le32(MORSE_RATECODE_SHORT_GI_FLAG);
}

/**
 * @brief Enable duplicates of bw_idx tx/rx over dup_bw_index.
 *
 * @param[in,out] rc Pointer to Morse rate code.
 */
static inline void morse_ratecode_enable_dup_format(morse_rate_code_t *rc)
{
	*rc |= cpu_to_le32(MORSE_RATECODE_DUP_FORMAT_FLAG);
}

/**
 * @brief Disable duplicates of bw_idx tx/rx.
 *
 * @param[in,out] rc Pointer to Morse rate code.
 */
static inline void morse_ratecode_disable_dup_format(morse_rate_code_t *rc)
{
	*rc &= ~cpu_to_le32(MORSE_RATECODE_DUP_FORMAT_FLAG);
}

/**
 * @brief Enable travelling pilots
 *
 * @param[in,out] rc Pointer to Morse rate code.
 */
static inline void morse_ratecode_enable_trav_pilots(morse_rate_code_t *rc)
{
	*rc |= cpu_to_le32(MORSE_RATECODE_TRAV_PILOTS_FLAG);
}

/**
 * @brief Disable travelling pilots
 *
 * @param[in,out] rc Pointer to Morse rate code.
 */
static inline void morse_ratecode_disable_trav_pilots(morse_rate_code_t *rc)
{
	*rc &= ~cpu_to_le32(MORSE_RATECODE_TRAV_PILOTS_FLAG);
}

/**
 * @brief Convert from bw in MHz to BW index
 *
 * @param[in] bw_mhz 2/4/6/8MHz
 * @param[out] bw_index in enum dot11_bandwidth.
 */
static inline enum dot11_bandwidth morse_ratecode_bw_mhz_to_bw_index(u8 bw_mhz)
{
	return ((bw_mhz == 1) ? DOT11_BANDWIDTH_1MHZ :
		(bw_mhz == 2) ? DOT11_BANDWIDTH_2MHZ :
		(bw_mhz == 4) ? DOT11_BANDWIDTH_4MHZ :
		(bw_mhz == 8) ? DOT11_BANDWIDTH_8MHZ : DOT11_BANDWIDTH_2MHZ);
}

/**
 * @brief Convert a enum dot11_bandwidth index into S1G BW in MHz.
 *
 * @param[in] bw_index
 * @return bw_mhz 2/4/6/8MHz
 */
static inline u8 morse_ratecode_bw_index_to_s1g_bw_mhz(enum dot11_bandwidth bw_idx)
{
	return ((bw_idx == DOT11_BANDWIDTH_1MHZ) ? 1 :
		(bw_idx == DOT11_BANDWIDTH_2MHZ) ? 2 :
		(bw_idx == DOT11_BANDWIDTH_4MHZ) ? 4 : (bw_idx == DOT11_BANDWIDTH_8MHZ) ? 8 : 2);
}
