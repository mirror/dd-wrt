#ifndef _MORSE_DEBUG_H_
#define _MORSE_DEBUG_H_
/*
 * Copyright 2017-2023 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "morse.h"
#include "skb_header.h"

#include <linux/kern_levels.h>

/*
 * Map onto standard kernel loglevels, see
 * https://elixir.bootlin.com/linux/latest/source/include/linux/kern_levels.h.
 */
#define MORSE_MSG_NONE		LOGLEVEL_EMERG
#define MORSE_MSG_ALERT		LOGLEVEL_ALERT
#define MORSE_MSG_CRIT		LOGLEVEL_CRIT
#define MORSE_MSG_ERR		LOGLEVEL_ERR
#define MORSE_MSG_WARN		LOGLEVEL_WARNING
#define MORSE_MSG_NOTICE	LOGLEVEL_NOTICE
#define MORSE_MSG_INFO		LOGLEVEL_INFO
#define MORSE_MSG_DEBUG		LOGLEVEL_DEBUG

/**
 * List of features whose debug verbosity can be individually controlled. The special
 * DEFAULT value is used, when no specific feature corresponds to the logged functionality.
 *
 * When adding new IDs, make sure to update the matching array initialiser in debug.c!
 */
enum morse_feature_id {
	FEATURE_ID_DEFAULT,	/* General, non specific */

	FEATURE_ID_TWT,
	FEATURE_ID_RAW,
	FEATURE_ID_RATECONTROL,
	FEATURE_ID_SKB,
	FEATURE_ID_SDIO,
	FEATURE_ID_PAGER,
	FEATURE_ID_POWERSAVE,
	FEATURE_ID_MESH,
	FEATURE_ID_ECSA,
	FEATURE_ID_CAC,
	FEATURE_ID_SPI,
	FEATURE_ID_MGMT_FRAMES,
	FEATURE_ID_HWSCAN,
	FEATURE_ID_COREDUMP,
	FEATURE_ID_BEACON,
	FEATURE_ID_YAPS,
	FEATURE_ID_USB,
	FEATURE_ID_HWCLOCK,
	FEATURE_ID_APF,
	NUM_FEATURE_IDS
};

/*
 * Generator macro to produce the prototype for the various logging functions.
 * Note that "__printf(3, 4)"" is a shorthand for the format(printf) attribute.
 */
#define __generate_log_fn_prototype(fn)					\
__printf(3, 4)								\
void morse_ ## fn(u32 id, const struct morse *mors, const char *fmt, ...)

__generate_log_fn_prototype(dbg);
__generate_log_fn_prototype(dbg_ratelimited);
__generate_log_fn_prototype(info);
__generate_log_fn_prototype(info_ratelimited);
__generate_log_fn_prototype(warn);
__generate_log_fn_prototype(warn_ratelimited);
__generate_log_fn_prototype(err);
__generate_log_fn_prototype(err_ratelimited);

#undef __generate_log_fn_prototype

/*
 * Helper macros to avoid having to pass FEATURE_ID_DEFAULT all the time.
 */
#define MORSE_DBG(_m, _f, _a...)			\
	morse_dbg(FEATURE_ID_DEFAULT, _m, _f, ##_a)

#define MORSE_DBG_RATELIMITED(_m, _f, _a...)		\
	morse_dbg_ratelimited(FEATURE_ID_DEFAULT, _m, _f, ##_a)

#define MORSE_INFO(_m, _f, _a...)			\
	morse_info(FEATURE_ID_DEFAULT, _m, _f, ##_a)

#define MORSE_INFO_RATELIMITED(_m, _f, _a...)		\
	morse_info_ratelimited(FEATURE_ID_DEFAULT, _m, _f, ##_a)

#define MORSE_WARN(_m, _f, _a...)			\
	morse_warn(FEATURE_ID_DEFAULT, _m, _f, ##_a)

#define MORSE_WARN_RATELIMITED(_m, _f, _a...)		\
	morse_warn_ratelimited(FEATURE_ID_DEFAULT, _m, _f, ##_a)

#define MORSE_ERR(_m, _f, _a...)			\
	morse_err(FEATURE_ID_DEFAULT, _m, _f, ##_a)

#define MORSE_ERR_RATELIMITED(_m, _f, _a...)		\
	morse_err_ratelimited(FEATURE_ID_DEFAULT, _m, _f, ##_a)

#define MORSE_WARN_ON(id, condition)								\
	do {											\
		if (morse_log_is_enabled(id, MORSE_MSG_WARN))					\
			WARN_ON(condition);							\
	} while (0)

#define MORSE_WARN_ON_ONCE(id, condition)							\
	do {											\
		if (morse_log_is_enabled(id, MORSE_MSG_WARN))					\
			WARN_ON_ONCE(condition);						\
	} while (0)

#define MORSE_PR_WARN(id, f, a...)								\
	do {											\
		if (morse_log_is_enabled(id, MORSE_MSG_WARN))					\
			pr_warn(f, ##a);							\
	} while (0)

#define MORSE_PR_ERR(id, f, a...)								\
	do {											\
		if (morse_log_is_enabled(id, MORSE_MSG_ERR))					\
			pr_err(f, ##a);								\
	} while (0)

#define MORSE_HEXDUMP_WARN(id, prefix, buf, len)						\
	do {											\
		if (morse_log_is_enabled(id, MORSE_MSG_WARN))					\
			print_hex_dump_bytes(prefix, DUMP_PREFIX_OFFSET, buf, len);		\
	} while (0)

#define MORSE_HEXDUMP_WARN_ONCE(prefix, buf, len)						\
	do {											\
		static int warned;								\
		if (!warned) {									\
			warned = 1;								\
			MORSE_HEXDUMP_WARN(prefix, buf, len);					\
		}										\
	} while (0)

/**
 * Reference to the debug_mask module parameter defined in init.c used to configure
 * dot11ah_debug
 */
extern uint debug_mask;

/**
 * Check if output would be generated for the given feature and severity level. This can be
 * used to check if output would be generated prior to performing computationally expensive
 * operations (such as malloc or conversion to hex).
 *
 * @param id	Feature id to check
 * @param level	Log severity to check
 *
 * @returns	True if output would be generated and false otherwise.
 */
bool morse_log_is_enabled(u32 id, u8 level);

/**
 * Set the default logging level for all features.
 *
 * @param level	Log severity to set as the default
 */
void morse_init_log_levels(u8 level);

int morse_init_debug(struct morse *mors);

void morse_deinit_debug(struct morse *mors);

int morse_debug_log_tx_status(struct morse *mors, struct morse_skb_tx_status *tx_sts);

enum morse_fw_hostif_log_channel_enable {
	MORSE_HOSTIF_LOG_DATA = BIT(0),
	MORSE_HOSTIF_LOG_COMMAND = BIT(1),
	MORSE_HOSTIF_LOG_TX_STATUS = BIT(2)
};

void morse_debug_fw_hostif_log_record(struct morse *morse, int to_chip,
				      struct sk_buff *skb, struct morse_buff_skb_header *hdr);

const char *morse_iftype_to_str(enum nl80211_iftype type);

void morse_log_modparams(struct morse *mors);

#ifdef CONFIG_MORSE_RC
void mmrc_s1g_add_sta_debugfs(struct morse *mors);
void mmrc_s1g_add_mesh_debugfs(struct morse *mors);
#endif

#endif	/* !_MORSE_DEBUG_H_ */
