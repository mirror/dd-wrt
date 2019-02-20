/*
 * Copyright (C) 2003-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2015 Red Hat, Inc. All rights reserved.
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

#ifndef _LVM_TOOL_POLLDAEMON_H
#define _LVM_TOOL_POLLDAEMON_H

#include "lib/metadata/metadata-exported.h"

typedef enum {
	PROGRESS_CHECK_FAILED = 0,
	PROGRESS_UNFINISHED = 1,
	PROGRESS_FINISHED_SEGMENT = 2,
	PROGRESS_FINISHED_ALL = 3
} progress_t;

struct daemon_parms;

struct poll_functions {
	const char *(*get_copy_name_from_lv) (const struct logical_volume *lv);
	progress_t (*poll_progress)(struct cmd_context *cmd,
				    struct logical_volume *lv,
				    const char *name,
				    struct daemon_parms *parms);
	int (*update_metadata) (struct cmd_context *cmd,
				struct volume_group *vg,
				struct logical_volume *lv,
				struct dm_list *lvs_changed, unsigned flags);
	int (*finish_copy) (struct cmd_context *cmd,
			    struct volume_group *vg,
			    struct logical_volume *lv,
			    struct dm_list *lvs_changed);
};

struct poll_operation_id {
	const char *vg_name;
	const char *lv_name;
	const char *display_name;
	const char *uuid;
};

struct daemon_parms {
	unsigned interval;
	unsigned wait_before_testing;
	unsigned aborting;
	unsigned background;
	unsigned outstanding_count;
	unsigned progress_display;
	const char *progress_title;
	uint64_t lv_type;
	struct poll_functions *poll_fns;
};

int poll_daemon(struct cmd_context *cmd, unsigned background,
		uint64_t lv_type, struct poll_functions *poll_fns,
		const char *progress_title, struct poll_operation_id *id);

progress_t poll_mirror_progress(struct cmd_context *cmd,
				struct logical_volume *lv, const char *name,
				struct daemon_parms *parms);

int wait_for_single_lv(struct cmd_context *cmd, struct poll_operation_id *id,
		       struct daemon_parms *parms);

#endif
