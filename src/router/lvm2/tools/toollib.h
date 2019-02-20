/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved. 
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

#ifndef _LVM_TOOLLIB_H
#define _LVM_TOOLLIB_H

#include "lib/metadata/metadata-exported.h"
#include "lib/report/report.h"

int become_daemon(struct cmd_context *cmd, int skip_lvm);

/*
 * The "struct processing_handle" is used as a handle for processing
 * functions (process_each_* and related).
 *
 * The "custom_handle" is any handle used to pass custom data into
 * process_each_* and related functions.
 *
 * The "internal_report_for_select=0" makes processing function to
 * skip checking the report/selection criteria (if given on cmd line)
 * before executing the action on the item.
 *
 * The "selection_handle" is only used if "internal_report_for_select=1".
 *
 * Some important notes about selection:
 * =====================================
 * In case we're processing for display, the selection is directly
 * a part of reporting for the display on output so we don't need to
 * report the item in memory to get the selection result, then dropping
 * the report and then reporting the same thing again for it to be
 * displayed on output.
 * For example, compare these code paths:
 *
 *   - when reporting for display on output:
 *      _report -> process_each_* -> ... -> dm_report_object
 *      (Here the dm_report_object does both selection and
 *       reporting for display on output.)
 *
 *   - for any other processing and reporting for selection:
 *      process_each_* -> _select_match_* -> ... -> dm_report_object_is_selected
 *                                                            |
 *                                                            --> (selection result) --> ...
 *      (Here the dm_report_object_is_selected just gets
 *       the selection result and it drops reporting buffer
 *       immediately. Then based on the selection result,
 *       the process_each_* action on the item is executed
 *       or not...)
 *
 * Simply, we want to avoid this double reporting when reporting
 * for display on output:
 *     _report -> process_each_* -> _select_match_* -> ... -> dm_report_object_is_selected
 *                                                                      |
 *                                                                      --> (selection result) -> dm_report_object
 *
 * So whenever the processing action is "to display item on output", use
 * "internal_report_for_select=0" as report/selection is already
 * a part of that reporting for display (dm_report_object).
 */
struct processing_handle {
	struct processing_handle *parent;
	int internal_report_for_select;
	int include_historical_lvs;
	struct selection_handle *selection_handle;
	void *custom_handle;
};

typedef int (*process_single_vg_fn_t) (struct cmd_context * cmd,
				       const char *vg_name,
				       struct volume_group * vg,
				       struct processing_handle *handle);
typedef int (*process_single_pv_fn_t) (struct cmd_context *cmd,
				  struct volume_group *vg,
				  struct physical_volume *pv,
				  struct processing_handle *handle);
typedef int (*process_single_label_fn_t) (struct cmd_context *cmd,
					  struct label *label,
					  struct processing_handle *handle);
typedef int (*process_single_lv_fn_t) (struct cmd_context *cmd,
				  struct logical_volume *lv,
				  struct processing_handle *handle);
typedef int (*process_single_seg_fn_t) (struct cmd_context * cmd,
					struct lv_segment * seg,
					struct processing_handle *handle);
typedef int (*process_single_pvseg_fn_t) (struct cmd_context * cmd,
					  struct volume_group * vg,
					  struct pv_segment * pvseg,
					  struct processing_handle *handle);

/*
 * Called prior to process_single_lv() to decide if the LV should be
 * processed.  If this returns 0, the LV is not processed.
 *
 * This can evaluate the combination of command definition and
 * the LV object to decide if the combination is allowed.
 */
typedef int (*check_single_lv_fn_t) (struct cmd_context *cmd,
				     struct logical_volume *lv,
				     struct processing_handle *handle,
				     int lv_is_named_arg);

int process_each_vg(struct cmd_context *cmd,
	            int argc, char **argv,
		    const char *one_vgname,
		    struct dm_list *use_vgnames,
		    uint32_t flags,
		    int include_internal,
		    struct processing_handle *handle,
		    process_single_vg_fn_t process_single_vg);

int process_each_pv(struct cmd_context *cmd, int argc, char **argv, const char *vg_name,
		    int all_is_set, uint32_t read_flags,
		    struct processing_handle *handle,
		    process_single_pv_fn_t process_single_pv);

int process_each_label(struct cmd_context *cmd, int argc, char **argv,
		       struct processing_handle *handle,
		       process_single_label_fn_t process_single_label);

int process_each_segment_in_pv(struct cmd_context *cmd,
			       struct volume_group *vg,
			       struct physical_volume *pv,
			       struct processing_handle *handle,
			       process_single_pvseg_fn_t process_single_pvseg);

int process_each_lv(struct cmd_context *cmd, int argc, char **argv,
		    const char *one_vgname, const char *one_lvname,
		    uint32_t flags, struct processing_handle *handle,
		    check_single_lv_fn_t check_single_lv,
		    process_single_lv_fn_t process_single_lv);


int process_each_segment_in_lv(struct cmd_context *cmd,
			       struct logical_volume *lv,
			       struct processing_handle *handle,
			       process_single_seg_fn_t process_single_seg);

int process_each_pv_in_vg(struct cmd_context *cmd, struct volume_group *vg,
			  struct processing_handle *handle,
			  process_single_pv_fn_t process_single_pv);


int process_each_lv_in_vg(struct cmd_context *cmd, struct volume_group *vg,
			  struct dm_list *arg_lvnames, const struct dm_list *tagsl,
			  int stop_on_error, struct processing_handle *handle,
			  check_single_lv_fn_t check_single_lv,
			  process_single_lv_fn_t process_single_lv);

struct processing_handle *init_processing_handle(struct cmd_context *cmd, struct processing_handle *parent_handle);
int init_selection_handle(struct cmd_context *cmd, struct processing_handle *handle,
			  report_type_t initial_report_type);
void destroy_processing_handle(struct cmd_context *cmd, struct processing_handle *handle);

int select_match_vg(struct cmd_context *cmd, struct processing_handle *handle,
		    struct volume_group *vg);
int select_match_lv(struct cmd_context *cmd, struct processing_handle *handle,
		    struct volume_group *vg, struct logical_volume *lv);
int select_match_pv(struct cmd_context *cmd, struct processing_handle *handle,
		    struct volume_group *vg, struct physical_volume *pv);

const char *extract_vgname(struct cmd_context *cmd, const char *lv_name);
const char *skip_dev_dir(struct cmd_context *cmd, const char *vg_name,
			 unsigned *dev_dir_found);

int opt_in_list_is_set(struct cmd_context *cmd, int *opts, int count,
		       int *match_count, int *unmatch_count);

void opt_array_to_str(struct cmd_context *cmd, int *opts, int count,
		      char *buf, int len);

int pvcreate_params_from_args(struct cmd_context *cmd, struct pvcreate_params *pp);
int pvcreate_each_device(struct cmd_context *cmd, struct processing_handle *handle, struct pvcreate_params *pp);

/*
 * Builds a list of pv's from the names in argv.  Used in
 * lvcreate/extend.
 */
struct dm_list *create_pv_list(struct dm_pool *mem, struct volume_group *vg, int argc,
			    char **argv, int allocatable_only);

struct dm_list *clone_pv_list(struct dm_pool *mem, struct dm_list *pvs);

int vgcreate_params_set_defaults(struct cmd_context *cmd,
				 struct vgcreate_params *vp_def,
				 struct volume_group *vg);
int vgcreate_params_set_from_args(struct cmd_context *cmd,
				  struct vgcreate_params *vp_new,
				  struct vgcreate_params *vp_def);
int lv_change_activate(struct cmd_context *cmd, struct logical_volume *lv,
		       activation_change_t activate);
int lv_refresh(struct cmd_context *cmd, struct logical_volume *lv);
int vg_refresh_visible(struct cmd_context *cmd, struct volume_group *vg);
void lv_spawn_background_polling(struct cmd_context *cmd,
				 struct logical_volume *lv);

int get_activation_monitoring_mode(struct cmd_context *cmd,
				   int *monitoring_mode);

int get_pool_params(struct cmd_context *cmd,
		    const struct segment_type *segtype,
		    uint64_t *pool_metadata_size,
		    int *pool_metadata_spare,
		    uint32_t *chunk_size,
		    thin_discards_t *discards,
		    thin_zero_t *zero_new_blocks);

int get_stripe_params(struct cmd_context *cmd, const struct segment_type *segtype,
		      uint32_t *stripes, uint32_t *stripe_size,
		      unsigned *stripes_supplied, unsigned *stripe_size_supplied);

int get_cache_params(struct cmd_context *cmd,
		     uint32_t *chunk_size,
		     cache_metadata_format_t *format,
		     cache_mode_t *cache_mode,
		     const char **name,
		     struct dm_config_tree **settings);

int change_tag(struct cmd_context *cmd, struct volume_group *vg,
	       struct logical_volume *lv, struct physical_volume *pv, int arg);

int get_and_validate_major_minor(const struct cmd_context *cmd,
				 const struct format_type *fmt,
				 int32_t *major, int32_t *minor);

int validate_lvname_param(struct cmd_context *cmd, const char **vg_name,
			  const char **lv_name);
int validate_restricted_lvname_param(struct cmd_context *cmd, const char **vg_name,
				     const char **lv_name);

int lvremove_single(struct cmd_context *cmd, struct logical_volume *lv,
                    struct processing_handle *handle __attribute__((unused)));

int get_lvt_enum(struct logical_volume *lv);

#endif
