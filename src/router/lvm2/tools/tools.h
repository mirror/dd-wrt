/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.  
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

#ifndef _LVM_TOOLS_H
#define _LVM_TOOLS_H

#include "tools/tool.h"

#include "lib/log/lvm-logging.h"

#include "lib/activate/activate.h"
#include "lib/format_text/archiver.h"
#include "lib/cache/lvmcache.h"
#include "lib/locking/lvmlockd.h"
#include "lvm-version.h"
#include "lib/config/config.h"
#include "lib/config/defaults.h"
#include "lib/device/dev-cache.h"
#include "lib/device/device.h"
#include "lib/display/display.h"
#include "errors.h"
#include "lib/metadata/metadata-exported.h"
#include "lib/locking/locking.h"
#include "lib/misc/lvm-exec.h"
#include "lib/misc/lvm-file.h"
#include "lib/misc/lvm-signal.h"
#include "lib/misc/lvm-string.h"
#include "lib/metadata/segtype.h"
#include "lib/datastruct/str_list.h"
#include "lib/commands/toolcontext.h"
#include "toollib.h"
#include "lib/notify/lvmnotify.h"

#include <ctype.h>
#include <sys/types.h>

#define CMD_LEN 256
#define MAX_ARGS 64

/* define the enums for each unique ID in command defs in command-lines.in */
enum {
#define cmd(a, b) a ,
#include "cmds.h"
#undef cmd
};

/* define the enums for the values accepted by command line --options, foo_VAL */
enum {
#define val(a, b, c, d) a ,
#include "vals.h"
#undef val
};

/* define the enums for the command line --options, foo_ARG */
enum {
#define arg(a, b, c, d, e, f, g) a ,
#include "args.h"
#undef arg
};

/* command functions */
#define xx(a, b...) int a(struct cmd_context *cmd, int argc, char **argv);
#include "commands.h"
#undef xx

/* define enums for LV properties, foo_LVP */
enum {
#define lvp(a, b, c) a ,
#include "lv_props.h"
#undef lvp
};

/* define enums for LV types, foo_LVT */
enum {
#define lvt(a, b, c) a ,
#include "lv_types.h"
#undef lvt
};

#include "command.h"
#include "command-count.h"

#define ARG_COUNTABLE 0x00000001	/* E.g. -vvvv */
#define ARG_GROUPABLE 0x00000002	/* E.g. --addtag */

struct arg_values {
	unsigned count;
	char *value;
	int32_t i_value;
	uint32_t ui_value;
	int64_t i64_value;
	uint64_t ui64_value;
	sign_t sign;
	percent_type_t percent;
};

struct arg_value_group_list {
        struct dm_list list;
        struct arg_values arg_values[0];
	uint32_t prio;
};

#define PERMITTED_READ_ONLY 	0x00000002
/* Process all VGs if none specified on the command line. */
#define ALL_VGS_IS_DEFAULT	0x00000004
/* Process all devices with --all if none are specified on the command line. */
#define ENABLE_ALL_DEVS		0x00000008	
/* Command may try to interpret a vgname arg as a uuid. */
#define ALLOW_UUID_AS_NAME	0x00000010
/* Command needs a shared lock on a VG; it only reads the VG. */
#define LOCKD_VG_SH		0x00000020
/* Command does not process any metadata. */
#define NO_METADATA_PROCESSING	0x00000040
/* Command must use all specified arg names and fail if all cannot be used. */
#define MUST_USE_ALL_ARGS        0x00000100
/* Command should process unused duplicate devices. */
#define ENABLE_DUPLICATE_DEVS    0x00000400
/* Command does not accept tags as args. */
#define DISALLOW_TAG_ARGS        0x00000800
/* Command may need to find VG name in an option value. */
#define GET_VGNAME_FROM_OPTIONS  0x00001000
/* The data read from disk by label scan can be used for vg_read. */
#define CAN_USE_ONE_SCAN	 0x00002000


void usage(const char *name);

/* the argument verify/normalise functions */
int yes_no_arg(struct cmd_context *cmd, struct arg_values *av);
int activation_arg(struct cmd_context *cmd, struct arg_values *av);
int cachemetadataformat_arg(struct cmd_context *cmd, struct arg_values *av);
int cachemode_arg(struct cmd_context *cmd, struct arg_values *av);
int discards_arg(struct cmd_context *cmd, struct arg_values *av);
int mirrorlog_arg(struct cmd_context *cmd, struct arg_values *av);
int size_kb_arg(struct cmd_context *cmd, struct arg_values *av);
int ssize_kb_arg(struct cmd_context *cmd, struct arg_values *av);
int size_mb_arg(struct cmd_context *cmd, struct arg_values *av);
int ssize_mb_arg(struct cmd_context *cmd, struct arg_values *av);
int psize_mb_arg(struct cmd_context *cmd, struct arg_values *av);
int nsize_mb_arg(struct cmd_context *cmd, struct arg_values *av);
int int_arg(struct cmd_context *cmd, struct arg_values *av);
int uint32_arg(struct cmd_context *cmd, struct arg_values *av);
int int_arg_with_sign(struct cmd_context *cmd, struct arg_values *av);
int int_arg_with_plus(struct cmd_context *cmd, struct arg_values *av);
int extents_arg(struct cmd_context *cmd, struct arg_values *av);
int sextents_arg(struct cmd_context *cmd, struct arg_values *av);
int pextents_arg(struct cmd_context *cmd, struct arg_values *av);
int nextents_arg(struct cmd_context *cmd, struct arg_values *av);
int major_arg(struct cmd_context *cmd, struct arg_values *av);
int minor_arg(struct cmd_context *cmd, struct arg_values *av);
int string_arg(struct cmd_context *cmd, struct arg_values *av);
int tag_arg(struct cmd_context *cmd, struct arg_values *av);
int permission_arg(struct cmd_context *cmd, struct arg_values *av);
int metadatatype_arg(struct cmd_context *cmd, struct arg_values *av);
int units_arg(struct cmd_context *cmd, struct arg_values *av);
int segtype_arg(struct cmd_context *cmd, struct arg_values *av);
int alloc_arg(struct cmd_context *cmd, struct arg_values *av);
int locktype_arg(struct cmd_context *cmd, struct arg_values *av);
int readahead_arg(struct cmd_context *cmd, struct arg_values *av);
int regionsize_mb_arg(struct cmd_context *cmd, struct arg_values *av);
int vgmetadatacopies_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av);
int pvmetadatacopies_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av);
int metadatacopies_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av);
int polloperation_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av);
int writemostly_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av);
int syncaction_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av);
int reportformat_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av);
int configreport_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av);
int configtype_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av);

/* we use the enums to access the switches */
unsigned arg_count(const struct cmd_context *cmd, int a);
unsigned arg_is_set(const struct cmd_context *cmd, int a);
int arg_from_list_is_set(const struct cmd_context *cmd, const char *err_found, ...);
int arg_outside_list_is_set(const struct cmd_context *cmd, const char *err_found, ...);
int arg_from_list_is_negative(const struct cmd_context *cmd, const char *err_found, ...);
int arg_from_list_is_zero(const struct cmd_context *cmd, const char *err_found, ...);
const char *arg_long_option_name(int a);
const char *arg_value(const struct cmd_context *cmd, int a);
const char *arg_str_value(const struct cmd_context *cmd, int a, const char *def);
int32_t arg_int_value(const struct cmd_context *cmd, int a, const int32_t def);
int32_t first_grouped_arg_int_value(const struct cmd_context *cmd, int a, const int32_t def);
uint32_t arg_uint_value(const struct cmd_context *cmd, int a, const uint32_t def);
int64_t arg_int64_value(const struct cmd_context *cmd, int a, const int64_t def);
uint64_t arg_uint64_value(const struct cmd_context *cmd, int a, const uint64_t def);
const void *arg_ptr_value(const struct cmd_context *cmd, int a, const void *def);
sign_t arg_sign_value(const struct cmd_context *cmd, int a, const sign_t def);
percent_type_t arg_percent_value(const struct cmd_context *cmd, int a, const percent_type_t def);
int arg_count_increment(struct cmd_context *cmd, int a);

unsigned grouped_arg_count(const struct arg_values *av, int a);
unsigned grouped_arg_is_set(const struct arg_values *av, int a);
const char *grouped_arg_str_value(const struct arg_values *av, int a, const char *def);
int32_t grouped_arg_int_value(const struct arg_values *av, int a, const int32_t def); 

const char *command_name(struct cmd_context *cmd);

int pvmove_poll(struct cmd_context *cmd, const char *pv_name, const char *uuid,
		const char *vg_name, const char *lv_name, unsigned background);
int lvconvert_poll(struct cmd_context *cmd, struct logical_volume *lv, unsigned background);

int mirror_remove_missing(struct cmd_context *cmd,
			  struct logical_volume *lv, int force);


int vgchange_activate(struct cmd_context *cmd, struct volume_group *vg,
		       activation_change_t activate);

int vgchange_background_polling(struct cmd_context *cmd, struct volume_group *vg);

int vgchange_locktype_cmd(struct cmd_context *cmd, int argc, char **argv);
int vgchange_lock_start_stop_cmd(struct cmd_context *cmd, int argc, char **argv);
int vgchange_systemid_cmd(struct cmd_context *cmd, int argc, char **argv);

struct lv_prop *get_lv_prop(int lvp_enum);
struct lv_type *get_lv_type(int lvt_enum);
struct command *get_command(int cmd_enum);

int lvchange_properties_cmd(struct cmd_context *cmd, int argc, char **argv);
int lvchange_activate_cmd(struct cmd_context *cmd, int argc, char **argv);
int lvchange_refresh_cmd(struct cmd_context *cmd, int argc, char **argv);
int lvchange_resync_cmd(struct cmd_context *cmd, int argc, char **argv);
int lvchange_syncaction_cmd(struct cmd_context *cmd, int argc, char **argv);
int lvchange_rebuild_cmd(struct cmd_context *cmd, int argc, char **argv);
int lvchange_monitor_poll_cmd(struct cmd_context *cmd, int argc, char **argv);
int lvchange_persistent_cmd(struct cmd_context *cmd, int argc, char **argv);

int lvconvert_repair_cmd(struct cmd_context *cmd, int argc, char **argv);
int lvconvert_replace_pv_cmd(struct cmd_context *cmd, int argc, char **argv);

int lvconvert_merge_snapshot_cmd(struct cmd_context *cmd, int argc, char **argv);
int lvconvert_split_snapshot_cmd(struct cmd_context *cmd, int argc, char **argv);
int lvconvert_combine_split_snapshot_cmd(struct cmd_context *cmd, int argc, char **argv);

int lvconvert_start_poll_cmd(struct cmd_context *cmd, int argc, char **argv);

int lvconvert_to_pool_cmd(struct cmd_context *cmd, int argc, char **argv);
int lvconvert_to_cache_vol_cmd(struct cmd_context *cmd, int argc, char **argv);
int lvconvert_to_thin_with_external_cmd(struct cmd_context *cmd, int argc, char **argv);
int lvconvert_swap_pool_metadata_cmd(struct cmd_context *cmd, int argc, char **argv);
int lvconvert_to_pool_or_swap_metadata_cmd(struct cmd_context *cmd, int argc, char **argv);
int lvconvert_merge_thin_cmd(struct cmd_context *cmd, int argc, char **argv);
int lvconvert_split_cachepool_cmd(struct cmd_context *cmd, int argc, char **argv);

int lvconvert_raid_types_cmd(struct cmd_context * cmd, int argc, char **argv);
int lvconvert_split_mirror_images_cmd(struct cmd_context * cmd, int argc, char **argv);
int lvconvert_merge_mirror_images_cmd(struct cmd_context *cmd, int argc, char **argv);
int lvconvert_change_mirrorlog_cmd(struct cmd_context * cmd, int argc, char **argv);
int lvconvert_change_region_size_cmd(struct cmd_context * cmd, int argc, char **argv);

int lvconvert_merge_cmd(struct cmd_context *cmd, int argc, char **argv);

int lvconvert_to_vdopool_cmd(struct cmd_context *cmd, int argc, char **argv);
int lvconvert_to_vdopool_param_cmd(struct cmd_context *cmd, int argc, char **argv);

int pvscan_display_cmd(struct cmd_context *cmd, int argc, char **argv);
int pvscan_cache_cmd(struct cmd_context *cmd, int argc, char **argv);

#endif
