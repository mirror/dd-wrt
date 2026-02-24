/*
 * Copyright 2017-2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#if !defined(__TRACE_MORSE_H) || defined(TRACE_HEADER_MULTI_READ)
#define __TRACE_MORSE_H

#include <linux/tracepoint.h>
#include "morse.h"
#include "debug.h"

#undef TRACE_SYSTEM
#define TRACE_SYSTEM morse

#define MORSE_MSG_MAX 200

DECLARE_EVENT_CLASS(morse_log_event,
	TP_PROTO(const struct morse *mors, struct va_format *vaf),
	TP_ARGS(mors, vaf),
	TP_STRUCT__entry(__string(device,
			 dev_name(mors->dev))
			 __string(driver, dev_driver_string(mors->dev))
			 __dynamic_array(char, msg, MORSE_MSG_MAX)),
#if KERNEL_VERSION(6, 10, 0) > LINUX_VERSION_CODE
	TP_fast_assign(__assign_str(device, dev_name(mors->dev));
		       __assign_str(driver, dev_driver_string(mors->dev));
#else
	TP_fast_assign(__assign_str(device);
		       __assign_str(driver);
#endif
		       WARN_ON_ONCE(vsnprintf(__get_dynamic_array(msg),
					      MORSE_MSG_MAX,
					      vaf->fmt, *vaf->va) >= MORSE_MSG_MAX);),
	TP_printk("%s %s %s", __get_str(driver), __get_str(device), __get_str(msg))
);

DEFINE_EVENT(morse_log_event, morse_err,
	TP_PROTO(const struct morse *mors, struct va_format *vaf), TP_ARGS(mors, vaf)
);

DEFINE_EVENT(morse_log_event, morse_warn,
	TP_PROTO(const struct morse *mors, struct va_format *vaf), TP_ARGS(mors, vaf)
);

DEFINE_EVENT(morse_log_event, morse_info,
	TP_PROTO(const struct morse *mors, struct va_format *vaf), TP_ARGS(mors, vaf)
);

DEFINE_EVENT(morse_log_event, morse_dbg,
	TP_PROTO(const struct morse *mors, struct va_format *vaf), TP_ARGS(mors, vaf)
);

DEFINE_EVENT(morse_log_event, morse_err_ratelimited,
	TP_PROTO(const struct morse *mors, struct va_format *vaf), TP_ARGS(mors, vaf)
);

DEFINE_EVENT(morse_log_event, morse_warn_ratelimited,
	TP_PROTO(const struct morse *mors, struct va_format *vaf), TP_ARGS(mors, vaf)
);

DEFINE_EVENT(morse_log_event, morse_info_ratelimited,
	TP_PROTO(const struct morse *mors, struct va_format *vaf), TP_ARGS(mors, vaf)
);

DEFINE_EVENT(morse_log_event, morse_dbg_ratelimited,
	TP_PROTO(const struct morse *mors, struct va_format *vaf), TP_ARGS(mors, vaf)
);

#endif

/* we don't want to use include/trace/events */
#undef TRACE_INCLUDE_PATH
#ifndef MORSE_TRACE_PATH
#error "MORSE_TRACE_PATH must be defined"
#endif
#define TRACE_INCLUDE_PATH	MORSE_TRACE_PATH
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_FILE	trace

/* This part must be outside protection */
#include <trace/define_trace.h>
