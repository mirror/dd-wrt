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

#ifndef _LVM_LOGGING_H
#define _LVM_LOGGING_H

#include "lib/misc/lvm-file.h"

__attribute__ ((format(printf, 5, 6)))
void print_log(int level, const char *file, int line, int dm_errno_or_class,
	       const char *format, ...);

__attribute__ ((format(printf, 5, 6)))
void print_log_libdm(int level, const char *file, int line, int dm_errno_or_class,
		     const char *format, ...);

#define LOG_LINE(l, x...) \
    print_log(l, __FILE__, __LINE__ , 0, ## x)

#define LOG_LINE_WITH_ERRNO(l, e, x...) \
    print_log(l, __FILE__, __LINE__ , e, ## x)

#define LOG_LINE_WITH_CLASS(l, c, x...) \
    print_log(l, __FILE__, __LINE__ , c, ## x)

#include "lib/log/log.h"

int init_custom_log_streams(struct custom_fds *custom_fds);
int reopen_standard_stream(FILE **stream, const char *mode);

typedef void (*lvm2_log_fn_t) (int level, const char *file, int line,
			       int dm_errno_or_class, const char *message);

void init_log_fn(lvm2_log_fn_t log_fn);

void init_indent(int indent);
void init_msg_prefix(const char *prefix);

void init_log_file(const char *log_file, int append);
void unlink_log_file(int ret);
void init_log_direct(const char *log_file, int append);
void init_log_while_suspended(int log_while_suspended);
void init_abort_on_internal_errors(int fatal);

void fin_log(void);
void release_log_memory(void);
void reset_log_duplicated(void);

void init_syslog(int facility);
void fin_syslog(void);

int error_message_produced(void);
void reset_lvm_errno(int store_errmsg);
int stored_errno(void);
const char *stored_errmsg(void);
const char *stored_errmsg_with_clear(void);

/* Suppress messages to stdout/stderr (1) or everywhere (2) */
/* Returns previous setting */
int log_suppress(int suppress);

/* Suppress messages to syslog */
void syslog_suppress(int suppress);

/* Hooks to handle logging through report. */
typedef enum {
	LOG_REPORT_CONTEXT_NULL,
	LOG_REPORT_CONTEXT_SHELL,
	LOG_REPORT_CONTEXT_PROCESSING,
	LOG_REPORT_CONTEXT_COUNT
} log_report_context_t;

typedef enum {
	LOG_REPORT_OBJECT_TYPE_NULL,
	LOG_REPORT_OBJECT_TYPE_CMD,
	LOG_REPORT_OBJECT_TYPE_ORPHAN,
	LOG_REPORT_OBJECT_TYPE_PV,
	LOG_REPORT_OBJECT_TYPE_LABEL,
	LOG_REPORT_OBJECT_TYPE_VG,
	LOG_REPORT_OBJECT_TYPE_LV,
	LOG_REPORT_OBJECT_TYPE_COUNT
} log_report_object_type_t;

typedef struct log_report {
	struct dm_report *report;
	log_report_context_t context;
	log_report_object_type_t object_type;
	const char *object_name;
	const char *object_id;
	const char *object_group;
	const char *object_group_id;
} log_report_t;

#define LOG_STATUS_NAME "status"
#define LOG_STATUS_SUCCESS "success"
#define LOG_STATUS_FAILURE "failure"

log_report_t log_get_report_state(void);
void log_restore_report_state(log_report_t log_report);

void log_set_report(struct dm_report *report);
void log_set_report_context(log_report_context_t context);
void log_set_report_object_type(log_report_object_type_t object_type);
void log_set_report_object_group_and_group_id(const char *group, const char *group_id);
void log_set_report_object_name_and_id(const char *name, const char *id);

const char *log_get_report_context_name(log_report_context_t context);
const char *log_get_report_object_type_name(log_report_object_type_t object_type);

#endif
