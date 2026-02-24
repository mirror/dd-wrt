#ifndef _MORSE_DOT11AH_DEBUG_H_
#define _MORSE_DOT11AH_DEBUG_H_

/*
 * Copyright 2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */
#include <linux/types.h>

#define DOT11AH_MSG_ERR			0x00000008
#define DOT11AH_MSG_WARN		0x00000004
#define DOT11AH_MSG_INFO		0x00000002
#define DOT11AH_MSG_DEBUG		0x00000001

__printf(4, 5)
void __dot11ah_debug(u32 level, const char *func, int line, const char *fmt, ...);
__printf(4, 5)
void __dot11ah_info(u32 level, const char *func, int line, const char *fmt, ...);
__printf(4, 5)
void __dot11ah_warn(u32 level, const char *func, int line, const char *fmt, ...);
__printf(4, 5)
void __dot11ah_warn_ratelimited(u32 level, const char *func, int line, const char *fmt, ...);
__printf(4, 5)
void __dot11ah_err(u32 level, const char *func, int line, const char *fmt, ...);

#ifndef dot11ah_dbg
#define dot11ah_debug(f, a...)	\
	__dot11ah_debug((dot11ah_debug_mask & DOT11AH_MSG_DEBUG), __func__, __LINE__, f, ##a)
#endif
#ifndef dot11ah_info
#define dot11ah_info(f, a...)	\
		__dot11ah_info((dot11ah_debug_mask & DOT11AH_MSG_INFO), __func__, __LINE__, f, ##a)
#endif
#ifndef dot11ah_warn
#define dot11ah_warn(f, a...)	\
	__dot11ah_warn((dot11ah_debug_mask & DOT11AH_MSG_WARN) ? 0xFFFFFFFF : 0, \
		__func__, __LINE__, f, ##a)
#endif
#ifndef dot11ah_warn_ratelimited
#define dot11ah_warn_ratelimited(f, a...)	\
		__dot11ah_warn_ratelimited((dot11ah_debug_mask & DOT11AH_MSG_WARN) ? \
			0xFFFFFFFF : 0, __func__, __LINE__, f, ##a)
#endif
#ifndef dot11ah_err
#define dot11ah_err(f, a...)	\
		__dot11ah_err((dot11ah_debug_mask & DOT11AH_MSG_ERR) ? 0xFFFFFFFF : 0, \
				__func__, __LINE__, f, ##a)
#endif

#ifndef dot11ah_hexdump_warn
#define dot11ah_hexdump_warn(prefix, buf, len)	\
	do { \
		if ((dot11ah_debug_mask & DOT11AH_MSG_WARN) ? 0xFFFFFFFF : 0) \
			print_hex_dump_bytes(prefix, DUMP_PREFIX_OFFSET, buf, len); \
	} while (0)
#endif

#ifndef DOT11AH_WARN_ON
#define DOT11AH_WARN_ON(_condition) \
	do { \
		bool _assertion = (_condition); \
		if (dot11ah_debug_mask & DOT11AH_MSG_WARN) \
			WARN_ON(_assertion); \
		else if (dot11ah_debug_mask && (_assertion)) \
			pr_warn("%s:%d: WARN_ON ASSERTED\n", __func__, __LINE__); \
	} while (0)
#endif

extern u32 dot11ah_debug_mask;

void morse_dot11ah_debug_init(u32 mask);
void morse_dot11ah_debug_set_mask(u32 mask);

#endif  /* !_MORSE_DEBUG_H_ */
