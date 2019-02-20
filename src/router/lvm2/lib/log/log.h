/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2007 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef _LVM_LOG_H
#define _LVM_LOG_H

/*
 * printf()-style macros to use for messages:
 *
 *   log_error   - always print to stderr.
 *   log_print   - always print to stdout.  Use this instead of printf.
 *   log_verbose - print to stdout if verbose is set (-v)
 *   log_very_verbose - print to stdout if verbose is set twice (-vv)
 *   log_debug   - print to stdout if verbose is set three times (-vvv)
 *
 * In addition, messages will be logged to file or syslog if they
 * are more serious than the log level specified with the log/debug_level
 * parameter in the configuration file.  These messages get the file
 * and line number prepended.  'stack' (without arguments) can be used 
 * to log this information at debug level.
 *
 * log_sys_error and log_sys_very_verbose are for errors from system calls
 * e.g. log_sys_error("stat", filename);
 *      /dev/fd/7: stat failed: No such file or directory
 *
 */

#include <errno.h>

#define EUNCLASSIFIED -1	/* Generic error code */

#define _LOG_FATAL         0x0002
#define _LOG_ERR           0x0003
#define _LOG_WARN          0x0004
#define _LOG_NOTICE        0x0005
#define _LOG_INFO          0x0006
#define _LOG_DEBUG         0x0007
#define _LOG_STDERR        0x0080 /* force things to go to stderr, even if loglevel would make them go to stdout */
#define _LOG_ONCE          0x0100 /* downgrade to NOTICE if this has been already logged */
#define _LOG_BYPASS_REPORT 0x0200 /* do not log through report even if report available */
#define log_level(x)  ((x) & 0x0f)			/* obtain message level */
#define log_stderr(x)  ((x) & _LOG_STDERR)		/* obtain stderr bit */
#define log_once(x)  ((x) & _LOG_ONCE)			/* obtain once bit */
#define log_bypass_report(x)  ((x) & _LOG_BYPASS_REPORT)/* obtain bypass bit */

#define INTERNAL_ERROR "Internal error: "

/*
 * Classes available for debug log messages.
 * These are also listed in doc/example.conf
 * and lib/commands/toolcontext.c:_parse_debug_classes()
 */
#define LOG_CLASS_MEM		0x0001	/* "memory" */
#define LOG_CLASS_DEVS		0x0002	/* "devices" */
#define LOG_CLASS_ACTIVATION	0x0004	/* "activation" */
#define LOG_CLASS_ALLOC		0x0008	/* "allocation" */
#define LOG_CLASS_LVMETAD	0x0010	/* "lvmetad" */
#define LOG_CLASS_METADATA	0x0020	/* "metadata" */
#define LOG_CLASS_CACHE		0x0040	/* "cache" */
#define LOG_CLASS_LOCKING	0x0080	/* "locking" */
#define LOG_CLASS_LVMPOLLD	0x0100	/* "lvmpolld" */
#define LOG_CLASS_DBUS		0x0200	/* "dbus" */
#define LOG_CLASS_IO		0x0400	/* "io" */

#define log_debug(x...) LOG_LINE(_LOG_DEBUG, x)
#define log_debug_mem(x...) LOG_LINE_WITH_CLASS(_LOG_DEBUG, LOG_CLASS_MEM, x)
#define log_debug_devs(x...) LOG_LINE_WITH_CLASS(_LOG_DEBUG, LOG_CLASS_DEVS, x)
#define log_debug_activation(x...) LOG_LINE_WITH_CLASS(_LOG_DEBUG, LOG_CLASS_ACTIVATION, x)
#define log_debug_alloc(x...) LOG_LINE_WITH_CLASS(_LOG_DEBUG, LOG_CLASS_ALLOC, x)
#define log_debug_lvmetad(x...) LOG_LINE_WITH_CLASS(_LOG_DEBUG, LOG_CLASS_LVMETAD, x)
#define log_debug_metadata(x...) LOG_LINE_WITH_CLASS(_LOG_DEBUG, LOG_CLASS_METADATA, x)
#define log_debug_cache(x...) LOG_LINE_WITH_CLASS(_LOG_DEBUG, LOG_CLASS_CACHE, x)
#define log_debug_locking(x...) LOG_LINE_WITH_CLASS(_LOG_DEBUG, LOG_CLASS_LOCKING, x)
#define log_debug_lvmpolld(x...) LOG_LINE_WITH_CLASS(_LOG_DEBUG, LOG_CLASS_LVMPOLLD, x)
#define log_debug_dbus(x...) LOG_LINE_WITH_CLASS(_LOG_DEBUG, LOG_CLASS_DBUS, x)
#define log_debug_io(x...) LOG_LINE_WITH_CLASS(_LOG_DEBUG, LOG_CLASS_IO, x)

#define log_info(x...) LOG_LINE(_LOG_INFO, x)
#define log_notice(x...) LOG_LINE(_LOG_NOTICE, x)
#define log_warn(x...) LOG_LINE(_LOG_WARN | _LOG_STDERR, x)
#define log_warn_suppress(s, x...) LOG_LINE(s ? _LOG_NOTICE : _LOG_WARN | _LOG_STDERR, x)
#define log_err(x...) LOG_LINE_WITH_ERRNO(_LOG_ERR, EUNCLASSIFIED, x)
#define log_err_suppress(s, x...) LOG_LINE_WITH_ERRNO(s ? _LOG_NOTICE : _LOG_ERR, EUNCLASSIFIED, x)
#define log_err_once(x...) LOG_LINE_WITH_ERRNO(_LOG_ERR | _LOG_ONCE, EUNCLASSIFIED, x)
#define log_fatal(x...) LOG_LINE_WITH_ERRNO(_LOG_FATAL, EUNCLASSIFIED, x)

#define stack log_debug("<backtrace>")	/* Backtrace on error */
#define log_very_verbose(args...) log_info(args)
#define log_verbose(args...) log_notice(args)
#define log_print(args...) LOG_LINE(_LOG_WARN, args)
#define log_print_unless_silent(args...) LOG_LINE(silent_mode() ? _LOG_NOTICE : _LOG_WARN, args)
#define log_error(args...) log_err(args)
#define log_error_suppress(s, args...) log_err_suppress(s, args)
#define log_error_once(args...) log_err_once(args)
#define log_errno(args...) LOG_LINE_WITH_ERRNO(_LOG_ERR, args)

/* System call equivalents */
#define log_sys_error(x, y) \
		log_err("%s%s%s failed: %s", y, *y ? ": " : "", x, strerror(errno))
#define log_sys_error_suppress(s, x, y) \
		log_err_suppress(s, "%s%s%s failed: %s", y, *y ? ": " : "", x, strerror(errno))
#define log_sys_very_verbose(x, y) \
		log_info("%s: %s failed: %s", y, x, strerror(errno))
#define log_sys_debug(x, y) \
		log_debug("%s: %s failed: %s", y, x, strerror(errno))

#define return_0	do { stack; return 0; } while (0)
#define return_NULL	do { stack; return NULL; } while (0)
#define return_EINVALID_CMD_LINE \
			do { stack; return EINVALID_CMD_LINE; } while (0)
#define return_ECMD_FAILED do { stack; return ECMD_FAILED; } while (0)
#define goto_out	do { stack; goto out; } while (0)
#define goto_bad	do { stack; goto bad; } while (0)

#endif
