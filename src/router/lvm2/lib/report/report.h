/*
 * Copyright (C) 2002-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2009 Red Hat, Inc. All rights reserved.
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

#ifndef _LVM_REPORT_H
#define _LVM_REPORT_H

#include "lib/metadata/metadata-exported.h"
#include "lib/label/label.h"
#include "lib/activate/activate.h"

typedef enum {
	CMDLOG		= 1,
	FULL		= 2,
	LVS		= 4,
	LVSINFO		= 8,
	LVSSTATUS	= 16,
	LVSINFOSTATUS   = 32,
	PVS		= 64,
	VGS		= 128,
	SEGS		= 256,
	PVSEGS		= 512,
	LABEL		= 1024,
	DEVTYPES	= 2048
} report_type_t;

/*
 * The "struct selection_handle" is used only for selection
 * of items that should be processed further (not for display!).
 *
 * It consists of selection reporting handle "selection_rh"
 * used for the selection itself (not for display on output!).
 * The items are reported directly in memory to a buffer and
 * then compared against selection criteria. Once we know the
 * result of the selection, the buffer is dropped!
 *
 * The "orig_report_type" is the original requested report type.
 * The "report_type" is the reporting type actually used which
 * also counts with report types of the fields used in selection
 * criteria.
 *
 * The "selected" variable is used for propagating the result
 * of the selection.
 */
struct selection_handle {
	struct dm_report *selection_rh;
	report_type_t orig_report_type;
	report_type_t report_type;
	int selected;
};

struct cmd_log_item {
	uint32_t seq_num;
	const char *type;
	const char *context;
	const char *object_type_name;
	const char *object_name;
	const char *object_id;
	const char *object_group;
	const char *object_group_id;
	const char *msg;
	int current_errno;
	int ret_code;
};

struct field;
struct report_handle;
struct processing_handle;

typedef int (*field_report_fn) (struct report_handle * dh, struct field * field,
				const void *data);

int report_format_init(struct cmd_context *cmd);

void *report_init(struct cmd_context *cmd, const char *format, const char *keys,
		  report_type_t *report_type, const char *separator,
		  int aligned, int buffered, int headings, int field_prefixes,
		  int quoted, int columns_as_rows, const char *selection,
		  int multiple_output);
int report_get_single_selection(struct cmd_context *cmd, report_type_t report_type, const char **selection);
void *report_init_for_selection(struct cmd_context *cmd, report_type_t *report_type,
				const char *selection);
int report_get_prefix_and_desc(report_type_t report_type_id,
			       const char **report_prefix,
			       const char **report_desc);
int report_for_selection(struct cmd_context *cmd,
			 struct processing_handle *parent_handle,
			 struct physical_volume *pv,
			 struct volume_group *vg,
			 struct logical_volume *lv);
void report_free(void *handle);
int report_object(void *handle, int selection_only, const struct volume_group *vg,
		  const struct logical_volume *lv, const struct physical_volume *pv,
		  const struct lv_segment *seg, const struct pv_segment *pvseg,
		  const struct lv_with_info_and_seg_status *lvdm,
		  const struct label *label);
int report_devtypes(void *handle);
int report_cmdlog(void *handle, const char *type, const char *context,
		  const char *object_type_name, const char *object_name,
		  const char *object_id, const char *object_group,
		  const char *object_group_id, const char *msg,
		  int current_errno, int ret_code);
void report_reset_cmdlog_seqnum(void);
#define REPORT_OBJECT_CMDLOG_NAME "status"
#define REPORT_OBJECT_CMDLOG_SUCCESS "success"
#define REPORT_OBJECT_CMDLOG_FAILURE "failure"
int report_current_object_cmdlog(const char *type, const char *msg, int32_t ret_code);
int report_output(void *handle);

#endif
