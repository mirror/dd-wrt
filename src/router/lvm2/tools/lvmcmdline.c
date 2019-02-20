/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2012 Red Hat, Inc. All rights reserved.
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

#include "tools.h"

#include "lvm2cmdline.h"
#include "lib/label/label.h"
#include "lvm-version.h"
#include "lib/locking/lvmlockd.h"

#include "stub.h"
#include "lib/misc/last-path-component.h"

#include <signal.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/resource.h>
#include <dirent.h>
#include <paths.h>
#include <locale.h>
#include <langinfo.h>

#ifdef HAVE_VALGRIND
#include <valgrind.h>
#endif

#ifdef HAVE_GETOPTLONG
#  include <getopt.h>
#  define GETOPTLONG_FN(a, b, c, d, e) getopt_long((a), (b), (c), (d), (e))
#  define OPTIND_INIT 0
#else
struct option {
};
extern int optind;
extern char *optarg;
#  define GETOPTLONG_FN(a, b, c, d, e) getopt((a), (b), (c))
#  define OPTIND_INIT 1
#endif


/*
 * Table of valid --option values.
 */
extern struct val_name val_names[VAL_COUNT + 1];

/*
 * Table of valid --option's
 */
extern struct opt_name opt_names[ARG_COUNT + 1];

/*
 * Table of LV properties
 */
extern struct lv_prop lv_props[LVP_COUNT + 1];

/*
 * Table of LV types
 */
extern struct lv_type lv_types[LVT_COUNT + 1];

/*
 * Table of command names
 */
extern struct command_name command_names[MAX_COMMAND_NAMES];

/*
 * Table of commands (as defined in command-lines.in)
 */
struct command commands[COMMAND_COUNT];

static struct cmdline_context _cmdline;

/*
 * Table of command line functions
 *
 * This table could be auto-generated once all commands have been converted
 * to use these functions instead of the old per-command-name function.
 * For now, any command id not included here uses the old command fn.
 */
static const struct command_function _command_functions[CMD_COUNT] = {
	{ lvmconfig_general_CMD, lvmconfig },
	{ lvchange_properties_CMD, lvchange_properties_cmd },
	{ lvchange_resync_CMD, lvchange_resync_cmd },
	{ lvchange_syncaction_CMD, lvchange_syncaction_cmd },
	{ lvchange_rebuild_CMD, lvchange_rebuild_cmd },
	{ lvchange_activate_CMD, lvchange_activate_cmd },
	{ lvchange_refresh_CMD, lvchange_refresh_cmd },
	{ lvchange_monitor_CMD, lvchange_monitor_poll_cmd },
	{ lvchange_poll_CMD, lvchange_monitor_poll_cmd },
	{ lvchange_persistent_CMD, lvchange_persistent_cmd },

	{ vgchange_locktype_CMD, vgchange_locktype_cmd },
	{ vgchange_lockstart_CMD, vgchange_lock_start_stop_cmd },
	{ vgchange_lockstop_CMD, vgchange_lock_start_stop_cmd },
	{ vgchange_systemid_CMD, vgchange_systemid_cmd },

	/* lvconvert utilities related to repair. */
	{ lvconvert_repair_CMD,	lvconvert_repair_cmd },
	{ lvconvert_replace_pv_CMD, lvconvert_replace_pv_cmd },

	/* lvconvert utilities related to snapshots. */
	{ lvconvert_split_cow_snapshot_CMD, lvconvert_split_snapshot_cmd },
	{ lvconvert_merge_snapshot_CMD, lvconvert_merge_snapshot_cmd },
	{ lvconvert_combine_split_snapshot_CMD, lvconvert_combine_split_snapshot_cmd },

	/* lvconvert utility to trigger polling on an LV. */
	{ lvconvert_start_poll_CMD, lvconvert_start_poll_cmd },
	{ lvconvert_plain_CMD, lvconvert_start_poll_cmd },

	/* lvconvert utilities for creating/maintaining thin and cache objects. */
	{ lvconvert_to_thinpool_CMD,			lvconvert_to_pool_cmd },
	{ lvconvert_to_cachepool_CMD,			lvconvert_to_pool_cmd },
	{ lvconvert_to_thin_with_external_CMD,		lvconvert_to_thin_with_external_cmd },
	{ lvconvert_to_cache_vol_CMD,			lvconvert_to_cache_vol_cmd },
	{ lvconvert_swap_pool_metadata_CMD,		lvconvert_swap_pool_metadata_cmd },
	{ lvconvert_to_thinpool_or_swap_metadata_CMD,   lvconvert_to_pool_or_swap_metadata_cmd },
	{ lvconvert_to_cachepool_or_swap_metadata_CMD,  lvconvert_to_pool_or_swap_metadata_cmd },
	{ lvconvert_merge_thin_CMD,			lvconvert_merge_thin_cmd },
	{ lvconvert_split_and_keep_cachepool_CMD,	lvconvert_split_cachepool_cmd },
	{ lvconvert_split_and_remove_cachepool_CMD,	lvconvert_split_cachepool_cmd },

	/* lvconvert raid-related type conversions */
	{ lvconvert_raid_types_CMD,			lvconvert_raid_types_cmd },

	/* lvconvert utilities for raid/mirror */
	{ lvconvert_split_mirror_images_CMD,		lvconvert_split_mirror_images_cmd},
	{ lvconvert_change_mirrorlog_CMD,		lvconvert_change_mirrorlog_cmd },
	{ lvconvert_merge_mirror_images_CMD,		lvconvert_merge_mirror_images_cmd },
	{ lvconvert_change_region_size_CMD,		lvconvert_change_region_size_cmd },

	/* redirected to merge_snapshot/merge_thin/merge_mirrors */
	{ lvconvert_merge_CMD, lvconvert_merge_cmd },

	/* lvconvert VDO pool */
	{ lvconvert_to_vdopool_CMD, lvconvert_to_vdopool_cmd },
	{ lvconvert_to_vdopool_param_CMD, lvconvert_to_vdopool_param_cmd },

	{ pvscan_display_CMD, pvscan_display_cmd },
	{ pvscan_cache_CMD, pvscan_cache_cmd },
};


/* Command line args */
unsigned arg_count(const struct cmd_context *cmd, int a)
{
	return cmd->opt_arg_values ? cmd->opt_arg_values[a].count : 0;
}

unsigned grouped_arg_count(const struct arg_values *av, int a)
{
	return av ? av[a].count : 0;
}

unsigned arg_is_set(const struct cmd_context *cmd, int a)
{
	return arg_count(cmd, a) ? 1 : 0;
}

int arg_from_list_is_set(const struct cmd_context *cmd, const char *err_found, ...)
{
	int arg;
	va_list ap;

	va_start(ap, err_found);
	while ((arg = va_arg(ap, int)) != -1 && !arg_is_set(cmd, arg))
		/* empty */;
	va_end(ap);

	if (arg == -1)
		return 0;

	if (err_found)
		log_error("%s %s.", arg_long_option_name(arg), err_found);

	return 1;
}

int arg_outside_list_is_set(const struct cmd_context *cmd, const char *err_found, ...)
{
	int i, arg;
	va_list ap;

	for (i = 0; i < ARG_COUNT; ++i) {
		switch (i) {
		/* skip common options */
		case commandprofile_ARG:
		case config_ARG:
		case debug_ARG:
		case driverloaded_ARG:
		case help2_ARG:
		case help_ARG:
		case profile_ARG:
		case quiet_ARG:
		case verbose_ARG:
		case version_ARG:
		case yes_ARG:
			continue;
		}
		if (!arg_is_set(cmd, i))
			continue; /* unset */
		va_start(ap, err_found);
		while (((arg = va_arg(ap, int)) != -1) && (arg != i))
			/* empty */;
		va_end(ap);

		if (arg == i)
			continue; /* set and in list */

		if (err_found)
			log_error("Option %s %s.", arg_long_option_name(i), err_found);

		return 1;
	}

	return 0;
}

int arg_from_list_is_negative(const struct cmd_context *cmd, const char *err_found, ...)
{
	int arg, ret = 0;
	va_list ap;

	va_start(ap, err_found);
	while ((arg = va_arg(ap, int)) != -1)
		if (arg_sign_value(cmd, arg, SIGN_NONE) == SIGN_MINUS) {
			if (err_found)
				log_error("%s %s.", arg_long_option_name(arg), err_found);
			ret = 1;
		}
	va_end(ap);

	return ret;
}

int arg_from_list_is_zero(const struct cmd_context *cmd, const char *err_found, ...)
{
	int arg, ret = 0;
	va_list ap;

	va_start(ap, err_found);
	while ((arg = va_arg(ap, int)) != -1)
		if (arg_is_set(cmd, arg) &&
		    !arg_int_value(cmd, arg, 0)) {
			if (err_found)
				log_error("%s %s.", arg_long_option_name(arg), err_found);
			ret = 1;
		}
	va_end(ap);

	return ret;
}

unsigned grouped_arg_is_set(const struct arg_values *av, int a)
{
	return grouped_arg_count(av, a) ? 1 : 0;
}

const char *arg_long_option_name(int a)
{
	return _cmdline.opt_names[a].long_opt;
}

const char *arg_value(const struct cmd_context *cmd, int a)
{
	return cmd->opt_arg_values ? cmd->opt_arg_values[a].value : NULL;
}

const char *arg_str_value(const struct cmd_context *cmd, int a, const char *def)
{
	return arg_is_set(cmd, a) ? cmd->opt_arg_values[a].value : def;
}

const char *grouped_arg_str_value(const struct arg_values *av, int a, const char *def)
{
	return grouped_arg_count(av, a) ? av[a].value : def;
}

int32_t grouped_arg_int_value(const struct arg_values *av, int a, const int32_t def)
{
	return grouped_arg_count(av, a) ? av[a].i_value : def;
}

int32_t first_grouped_arg_int_value(const struct cmd_context *cmd, int a, const int32_t def)
{
	struct arg_value_group_list *current_group;
	struct arg_values *av;

	dm_list_iterate_items(current_group, &cmd->arg_value_groups) {
		av = current_group->arg_values;
		if (grouped_arg_count(av, a))
			return grouped_arg_int_value(av, a, def);
	}

	return def;
}

int32_t arg_int_value(const struct cmd_context *cmd, int a, const int32_t def)
{
	return (_cmdline.opt_names[a].flags & ARG_GROUPABLE) ?
		first_grouped_arg_int_value(cmd, a, def) : (arg_is_set(cmd, a) ? cmd->opt_arg_values[a].i_value : def);
}

uint32_t arg_uint_value(const struct cmd_context *cmd, int a, const uint32_t def)
{
	return arg_is_set(cmd, a) ? cmd->opt_arg_values[a].ui_value : def;
}

int64_t arg_int64_value(const struct cmd_context *cmd, int a, const int64_t def)
{
	return arg_is_set(cmd, a) ? cmd->opt_arg_values[a].i64_value : def;
}

uint64_t arg_uint64_value(const struct cmd_context *cmd, int a, const uint64_t def)
{
	return arg_is_set(cmd, a) ? cmd->opt_arg_values[a].ui64_value : def;
}

/* No longer used.
const void *arg_ptr_value(struct cmd_context *cmd, int a, const void *def)
{
	return arg_is_set(cmd, a) ? cmd->opt_arg_values[a].ptr : def;
}
*/

sign_t arg_sign_value(const struct cmd_context *cmd, int a, const sign_t def)
{
	return arg_is_set(cmd, a) ? cmd->opt_arg_values[a].sign : def;
}

percent_type_t arg_percent_value(const struct cmd_context *cmd, int a, const percent_type_t def)
{
	return arg_is_set(cmd, a) ? cmd->opt_arg_values[a].percent : def;
}

int arg_count_increment(struct cmd_context *cmd, int a)
{
	return cmd->opt_arg_values[a].count++;
}

int yes_no_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av)
{
	av->sign = SIGN_NONE;
	av->percent = PERCENT_NONE;

	if (!strcmp(av->value, "y")) {
		av->i_value = 1;
		av->ui_value = 1;
	}

	else if (!strcmp(av->value, "n")) {
		av->i_value = 0;
		av->ui_value = 0;
	}

	else
		return 0;

	return 1;
}

int activation_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av)
{
	av->sign = SIGN_NONE;
	av->percent = PERCENT_NONE;

	if (!strcmp(av->value, "e") || !strcmp(av->value, "ey") ||
	    !strcmp(av->value, "ye")) {
		av->i_value = CHANGE_AEY;
		av->ui_value = CHANGE_AEY;
	}

	else if (!strcmp(av->value, "s") || !strcmp(av->value, "sy") ||
		 !strcmp(av->value, "ys")) {
		av->i_value = CHANGE_ASY;
		av->ui_value = CHANGE_ASY;
	}

	else if (!strcmp(av->value, "y")) {
		av->i_value = CHANGE_AY;
		av->ui_value = CHANGE_AY;
	}

	else if (!strcmp(av->value, "a") || !strcmp(av->value, "ay") ||
		 !strcmp(av->value, "ya")) {
		av->i_value = CHANGE_AAY;
		av->ui_value = CHANGE_AAY;
	}

	else if (!strcmp(av->value, "n") || !strcmp(av->value, "en") ||
		 !strcmp(av->value, "ne")) {
		av->i_value = CHANGE_AN;
		av->ui_value = CHANGE_AN;
	}

	else if (!strcmp(av->value, "ln") || !strcmp(av->value, "nl")) {
		av->i_value = CHANGE_ALN;
		av->ui_value = CHANGE_ALN;
	}

	else if (!strcmp(av->value, "ly") || !strcmp(av->value, "yl")) {
		av->i_value = CHANGE_ALY;
		av->ui_value = CHANGE_ALY;
	}

	else
		return 0;

	return 1;
}

int cachemode_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av)
{
	cache_mode_t mode;

	if (!set_cache_mode(&mode, av->value))
		return_0;

	av->i_value = mode;
	av->ui_value = mode;

	return 1;
}

int cachemetadataformat_arg(struct cmd_context *cmd, struct arg_values *av)
{
	if (!strcmp(av->value, "auto")) {
		av->i_value = CACHE_METADATA_FORMAT_UNSELECTED;
		av->ui_value = CACHE_METADATA_FORMAT_UNSELECTED;
	} else if (!int_arg(cmd, av))
		return_0;

	switch (av->i_value) {
	case CACHE_METADATA_FORMAT_UNSELECTED:
	case CACHE_METADATA_FORMAT_1:
	case CACHE_METADATA_FORMAT_2:
		return 1;
	}

	log_error("Selected cache metadata format %d is not supported.", av->i_value);
	return 0;
}

int discards_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av)
{
	thin_discards_t discards;

	if (!set_pool_discards(&discards, av->value))
		return_0;

	av->i_value = discards;
	av->ui_value = discards;

	return 1;
}

int mirrorlog_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av)
{
	int log_count;

	if (!set_mirror_log_count(&log_count, av->value))
		return_0;

	av->i_value = log_count;
	av->ui_value = log_count;

	return 1;
}

int metadatatype_arg(struct cmd_context *cmd, struct arg_values *av)
{
	return get_format_by_name(cmd, av->value) ? 1 : 0;
}

static int _get_int_arg(struct arg_values *av, char **ptr)
{
	char *val;
	unsigned long long v;

	av->percent = PERCENT_NONE;

	val = av->value;
	switch (*val) {
	case '+':
		av->sign = SIGN_PLUS;
		val++;
		break;
	case '-':
		av->sign = SIGN_MINUS;
		val++;
		break;
	default:
		av->sign = SIGN_NONE;
	}

	if (!isdigit(*val))
		return 0;

	errno = 0;
	v = strtoull(val, ptr, 10);

	if (*ptr == val || errno)
		return 0;

	av->i_value = (v < INT32_MAX) ? (int32_t) v : INT32_MAX;
	av->ui_value = (v < UINT32_MAX) ? (uint32_t) v : UINT32_MAX;
	av->i64_value = (v < INT64_MAX) ? (int64_t) v : INT64_MAX;
	av->ui64_value = (v < UINT64_MAX) ? (uint64_t) v : UINT64_MAX;

	return 1;
}

static int _get_percent_arg(struct arg_values *av, const char *ptr)
{
	if (!strcasecmp(ptr, "V") || !strcasecmp(ptr, "VG"))
		av->percent = PERCENT_VG;
	else if (!strcasecmp(ptr, "L") || !strcasecmp(ptr, "LV"))
		av->percent = PERCENT_LV;
	else if (!strcasecmp(ptr, "P") || !strcasecmp(ptr, "PV") ||
		 !strcasecmp(ptr, "PVS"))
		av->percent = PERCENT_PVS;
	else if (!strcasecmp(ptr, "F") || !strcasecmp(ptr, "FR") ||
		 !strcasecmp(ptr, "FREE"))
		av->percent = PERCENT_FREE;
	else if (!strcasecmp(ptr, "O") || !strcasecmp(ptr, "OR") ||
		 !strcasecmp(ptr, "ORIGIN"))
		av->percent = PERCENT_ORIGIN;
	else {
		log_error("Specified %%%s is unknown.", ptr);
		return 0;
	}

	return 1;
}

/* Size stored in sectors */
static int _size_arg(struct cmd_context *cmd __attribute__((unused)),
		     struct arg_values *av, int factor, int percent)
{
	char *ptr;
	int i;
	static const char *suffixes = "kmgtpebs";
	char *val;
	double v;
	uint64_t v_tmp, adjustment;
	const char *radixchar = nl_langinfo(RADIXCHAR) ? : ".";

	av->percent = PERCENT_NONE;

	val = av->value;
	switch (*val) {
	case '+':
		av->sign = SIGN_PLUS;
		val++;
		break;
	case '-':
		av->sign = SIGN_MINUS;
		val++;
		break;
	default:
		av->sign = SIGN_NONE;
	}

	if (*val == '+' || *val == '-') {
		log_error("Multiple sign symbols detected.");
		return 0;
	}

	if (!isdigit(*val) && (*val != '.') && (*val != radixchar[0])) {
		log_error("Size requires number argument.");
		return 0;
	}

	errno = 0;
	v = strtod(val, &ptr);

	if (*ptr == '.' && radixchar[0] != '.') {
		/*
		 * Maybe user has non-C locale with different decimal point ?
		 * Lets be tolerant and retry with standard C locales
		 */
		if (setlocale(LC_ALL, "C")) {
			errno = 0;
			v = strtod(val, &ptr);
			setlocale(LC_ALL, "");
		}
	}

	if (ptr == val || errno) {
		log_error("Can't parse size argument at '%c'.%s%s", ptr[0], (errno) ? " " :"", (errno) ? strerror(errno) : "");
		return 0;
	}

	if (percent && *ptr == '%') {
		if (!_get_percent_arg(av, ++ptr))
			return_0;
		if ((uint64_t) v >= UINT32_MAX) {
			log_error("Percentage is too big (>=%d%%).", UINT32_MAX);
			return 0;
		}
	} else if (*ptr) {
		for (i = strlen(suffixes) - 1; i >= 0; i--)
			if (suffixes[i] == tolower((int) *ptr))
				break;

		if (i < 0) {
			log_error("Can't parse size argument.");
			return 0;
		} else if (i == 7) {
			/* v is already in sectors */
			;
		} else if (i == 6) {
			/* bytes */
			v_tmp = (uint64_t) v;
			adjustment = v_tmp % 512;
			if (adjustment) {
				v_tmp += (512 - adjustment);
				log_error("Size is not a multiple of 512. "
					  "Try using %"PRIu64" or %"PRIu64".",
					  v_tmp - 512, v_tmp);
				return 0;
			}
			v /= 512;
		} else {
			/* all other units: kmgtpe */
			while (i-- > 0)
				v *= 1024;
			v *= 2;
		}
	} else
		v *= factor;

	/* Compare (double) */
	if (v >= (double) (UINT64_MAX >> SECTOR_SHIFT)) {
		log_error("Size is too big (>=16EiB).");
		return 0;
	}

	av->i_value = (v < INT32_MAX) ? (int32_t) v : INT32_MAX;
	av->ui_value = (v < UINT32_MAX) ? (uint32_t) v : UINT32_MAX;
	av->i64_value = (v < INT64_MAX) ? (int64_t) v : INT64_MAX;
	av->ui64_value = (v < UINT64_MAX) ? (uint64_t) v : UINT64_MAX;

	return 1;
}

/* negative not accepted */
int size_kb_arg(struct cmd_context *cmd, struct arg_values *av)
{
	if (!_size_arg(cmd, av, 2, 0))
		return 0;

	if (av->sign == SIGN_MINUS) {
		log_error("Size may not be negative.");
		return 0;
	}

	return 1;
}

int ssize_kb_arg(struct cmd_context *cmd, struct arg_values *av)
{
	return _size_arg(cmd, av, 2, 0);
}

int size_mb_arg(struct cmd_context *cmd, struct arg_values *av)
{
	if (!_size_arg(cmd, av, 2048, 0))
		return 0;

	if ((av->sign == SIGN_MINUS) || (av->sign == SIGN_PLUS)) {
		log_error("Size may not be relative/signed.");
		return 0;
	}

	return 1;
}

int ssize_mb_arg(struct cmd_context *cmd, struct arg_values *av)
{
	return _size_arg(cmd, av, 2048, 0);
}

int psize_mb_arg(struct cmd_context *cmd, struct arg_values *av)
{
	if (!_size_arg(cmd, av, 2048, 0))
		return 0;

	if (av->sign == SIGN_MINUS) {
		log_error("Size may not be negative.");
		return 0;
	}

	return 1;
}

int nsize_mb_arg(struct cmd_context *cmd, struct arg_values *av)
{
	if (!_size_arg(cmd, av, 2048, 0))
		return 0;

	if (av->sign == SIGN_PLUS) {
		log_error("Size may not be positive.");
		return 0;
	}

	return 1;
}

int int_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av)
{
	char *ptr;

	if (!_get_int_arg(av, &ptr) || (*ptr) || (av->sign == SIGN_MINUS))
		return 0;

	return 1;
}

int uint32_arg(struct cmd_context *cmd, struct arg_values *av)
{
	if (!int_arg(cmd, av) || (av->ui64_value > UINT32_MAX))
		return 0;

	return 1;
}

int int_arg_with_sign(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av)
{
	char *ptr;

	if (!_get_int_arg(av, &ptr) || (*ptr))
		return 0;

	return 1;
}

int int_arg_with_plus(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av)
{
	char *ptr;

	if (!_get_int_arg(av, &ptr) || (*ptr))
		return 0;

	if (av->sign == SIGN_MINUS) {
		log_error("Number may not be negative.");
		return 0;
	}

	return 1;
}

static int _extents_arg(struct cmd_context *cmd __attribute__((unused)),
			struct arg_values *av)
{
	char *ptr;

	if (!_get_int_arg(av, &ptr))
		return 0;

	if (!*ptr)
		return 1;

	if (*ptr++ != '%')
		return 0;

	if (!_get_percent_arg(av, ptr))
		return_0;

	if (av->ui64_value >= UINT32_MAX) {
		log_error("Percentage is too big (>=%d%%).", UINT32_MAX);
		return 0;
	}

	return 1;
}

int extents_arg(struct cmd_context *cmd __attribute__((unused)),
		struct arg_values *av)
{
	if (!_extents_arg(cmd, av))
		return 0;

	if ((av->sign == SIGN_MINUS) || (av->sign == SIGN_PLUS)) {
		log_error("Extents may not be relative/signed.");
		return 0;
	}

	return 1;
}

int sextents_arg(struct cmd_context *cmd __attribute__((unused)),
		 struct arg_values *av)
{
	return _extents_arg(cmd, av);
}

int pextents_arg(struct cmd_context *cmd __attribute__((unused)),
		 struct arg_values *av)
{
	if (!_extents_arg(cmd, av))
		return 0;

	if (av->sign == SIGN_MINUS) {
		log_error("Extents may not be negative.");
		return 0;
	}

	return 1;
}

int nextents_arg(struct cmd_context *cmd __attribute__((unused)),
		 struct arg_values *av)
{
	if (!_extents_arg(cmd, av))
		return 0;

	if (av->sign == SIGN_PLUS) {
		log_error("Extents may not be positive.");
		return 0;
	}

	return 1;
}

int string_arg(struct cmd_context *cmd __attribute__((unused)),
	       struct arg_values *av __attribute__((unused)))
{
	return 1;
}

int tag_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av)
{
	char *pos = av->value;

	if (*pos == '@')
		pos++;

	if (!validate_tag(pos))
		return 0;

	av->value = pos;

	return 1;
}

int permission_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av)
{
	av->sign = SIGN_NONE;

	if ((!strcmp(av->value, "rw")) || (!strcmp(av->value, "wr")))
		av->ui_value = LVM_READ | LVM_WRITE;

	else if (!strcmp(av->value, "r"))
		av->ui_value = LVM_READ;

	else
		return 0;

	return 1;
}

int alloc_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av)
{
	alloc_policy_t alloc;

	av->sign = SIGN_NONE;

	alloc = get_alloc_from_string(av->value);
	if (alloc == ALLOC_INVALID)
		return 0;

	av->ui_value = (uint32_t) alloc;

	return 1;
}

int locktype_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av)
{
	lock_type_t lock_type;

	av->sign = SIGN_NONE;

	lock_type = get_lock_type_from_string(av->value);
	if (lock_type == LOCK_TYPE_INVALID)
		return 0;

	return 1;
}

int segtype_arg(struct cmd_context *cmd, struct arg_values *av)
{
	struct segment_type *segtype;
	const char *str = (!strcmp(av->value, SEG_TYPE_NAME_LINEAR)) ? SEG_TYPE_NAME_STRIPED : av->value;

	if (!(segtype = get_segtype_from_string(cmd, str)))
		return_0;

	return (!segtype_is_unknown(segtype)) ? 1 : 0;
}

/*
 * Positive integer, zero or "auto".
 */
int readahead_arg(struct cmd_context *cmd __attribute__((unused)), struct arg_values *av)
{
	if (!strcasecmp(av->value, "auto")) {
		av->ui_value = DM_READ_AHEAD_AUTO;
		return 1;
	}

	if (!strcasecmp(av->value, "none")) {
		av->ui_value = DM_READ_AHEAD_NONE;
		return 1;
	}

	if (!_size_arg(cmd, av, 1, 0))
		return 0;

	if (av->sign == SIGN_MINUS)
		return 0;

	return 1;
}

int regionsize_mb_arg(struct cmd_context *cmd, struct arg_values *av)
{
	int pagesize = lvm_getpagesize();
	uint32_t num;

	if (!_size_arg(cmd, av, 2048, 0))
		return 0;

	if (av->sign == SIGN_MINUS) {
		log_error("Region size may not be negative.");
		return 0;
	}

	if (av->ui64_value > UINT32_MAX) {
		log_error("Region size is too big (max %u).", UINT32_MAX);
		return 0;
	}

	num = av->ui_value;

	if (!num) {
		log_error("Region size may not be zero.");
		return 0;
	}

	if (num % (pagesize >> SECTOR_SHIFT)) {
		log_error("Region size must be a multiple of machine memory page size (%d bytes).",
			  pagesize);
		return 0;
	}

	if (!is_power_of_2(num)) {
		log_error("Region size must be a power of 2.");
		return 0;
	}

	return 1;
}

/*
 * Non-zero, positive integer, "all", or "unmanaged"
 */
int vgmetadatacopies_arg(struct cmd_context *cmd, struct arg_values *av)
{
	if (!strcasecmp(av->value, "all")) {
		av->ui_value = VGMETADATACOPIES_ALL;
		return 1;
	}

	if (!strcasecmp(av->value, "unmanaged")) {
		av->ui_value = VGMETADATACOPIES_UNMANAGED;
		return 1;
	}

	return int_arg(cmd, av);
}

int pvmetadatacopies_arg(struct cmd_context *cmd, struct arg_values *av)
{
	int num;

	if (!int_arg(cmd, av))
		return 0;

	num = av->i_value;

	if ((num != 0) && (num != 1) && (num != 2))
		return 0;

	return 1;
}

int metadatacopies_arg(struct cmd_context *cmd, struct arg_values *av)
{
	if (!strncmp(cmd->name, "pv", 2))
		return pvmetadatacopies_arg(cmd, av);
	if (!strncmp(cmd->name, "vg", 2))
		return vgmetadatacopies_arg(cmd, av);
	return 0;
}

int polloperation_arg(struct cmd_context *cmd, struct arg_values *av)
{
	if (!strcmp(av->value, "pvmove") ||
	    !strcmp(av->value, "convert") ||
	    !strcmp(av->value, "merge") ||
	    !strcmp(av->value, "merge_thin"))
		return 1;
	return 0;
}

int writemostly_arg(struct cmd_context *cmd, struct arg_values *av)
{
	/* Could we verify that a PV arg looks like /dev/foo ? */
	return 1;
}

int syncaction_arg(struct cmd_context *cmd, struct arg_values *av)
{
	if (!strcmp(av->value, "check") ||
	    !strcmp(av->value, "repair"))
		return 1;
	return 0;
}

int reportformat_arg(struct cmd_context *cmd, struct arg_values *av)
{
	if (!strcmp(av->value, "basic") ||
	    !strcmp(av->value, "json"))
		return 1;
	return 0;
}

int configreport_arg(struct cmd_context *cmd, struct arg_values *av)
{
	if (!strcmp(av->value, "log") ||
	    !strcmp(av->value, "vg") ||
	    !strcmp(av->value, "lv") ||
	    !strcmp(av->value, "pv") ||
	    !strcmp(av->value, "pvseg") ||
	    !strcmp(av->value, "seg"))
		return 1;
	return 0;
}

int configtype_arg(struct cmd_context *cmd, struct arg_values *av)
{
	if (!strcmp(av->value, "current") ||
	    !strcmp(av->value, "default") ||
	    !strcmp(av->value, "diff") ||
	    !strcmp(av->value, "full") ||
	    !strcmp(av->value, "list") ||
	    !strcmp(av->value, "missing") ||
	    !strcmp(av->value, "new") ||
	    !strcmp(av->value, "profilable") ||
	    !strcmp(av->value, "profilable-command") ||
	    !strcmp(av->value, "profilable-metadata"))
		return 1;
	return 0;
}

/*
 * FIXME: there's been a confusing mixup among:
 * resizeable, resizable, allocatable, allocation.
 *
 * resizeable and allocatable are the preferred,
 * standard option names.
 *
 * The dispreferred "resizable" is always translated
 * to the preferred resizeable.
 *
 * But, the dispreferred "allocation" name seems
 * to translate to either or both resizeable
 * and allocatable, it's not clear which.
 */

static int _opt_standard_to_synonym(const char *cmd_name, int opt)
{
	switch (opt) {
	case mirrorlog_ARG:
		return corelog_ARG;
	case resizeable_ARG:
		return resizable_ARG;
	case allocatable_ARG:
		return allocation_ARG;
	case activate_ARG:
		return available_ARG;
	case rebuild_ARG:
		return raidrebuild_ARG;
	case syncaction_ARG:
		return raidsyncaction_ARG;
	case writemostly_ARG:
		return raidwritemostly_ARG;
	case minrecoveryrate_ARG:
		return raidminrecoveryrate_ARG;
	case maxrecoveryrate_ARG:
		return raidmaxrecoveryrate_ARG;
	case writebehind_ARG:
		return raidwritebehind_ARG;
	case virtualsize_ARG:
		return virtualoriginsize_ARG;
	case splitcache_ARG:
		return split_ARG;
	case pvmetadatacopies_ARG:
		if (!strncmp(cmd_name, "pv", 2))
			return metadatacopies_ARG;
		return 0;
	case vgmetadatacopies_ARG:
		if (!strncmp(cmd_name, "vg", 2))
			return metadatacopies_ARG;
		return 0;
	}
	return 0;
}

static int _opt_synonym_to_standard(const char *cmd_name, int opt)
{
	switch (opt) {
	case corelog_ARG:
		return mirrorlog_ARG;
	case resizable_ARG:
		return resizeable_ARG;
	case allocation_ARG:
		return allocatable_ARG;
	case available_ARG:
		return activate_ARG;
	case raidrebuild_ARG:
		return rebuild_ARG;
	case raidsyncaction_ARG:
		return syncaction_ARG;
	case raidwritemostly_ARG:
		return writemostly_ARG;
	case raidminrecoveryrate_ARG:
		return minrecoveryrate_ARG;
	case raidmaxrecoveryrate_ARG:
		return maxrecoveryrate_ARG;
	case raidwritebehind_ARG:
		return writebehind_ARG;
	case virtualoriginsize_ARG:
		return virtualsize_ARG;
	case split_ARG:
		return splitcache_ARG;
	case metadatacopies_ARG:
		if (!strncmp(cmd_name, "pv", 2))
			return pvmetadatacopies_ARG;
		if (!strncmp(cmd_name, "vg", 2))
			return vgmetadatacopies_ARG;
		return 0;
	}
	return 0;
}

static void _add_getopt_arg(int arg_enum, char **optstrp, struct option **longoptsp);

/*
 * The valid args for a command name in general is a union of
 * required_opt_args and optional_opt_args for all commands[]
 * with the given name.
 */

static void _set_valid_args_for_command_name(int ci)
{
	int all_args[ARG_COUNT] = { 0 };
	int num_args = 0;
	int opt_enum; /* foo_ARG from args.h */
	int opt_syn;
	int i, ro, oo, io;

	/*
	 * all_args is indexed by the foo_ARG enum vals
	 */

	for (i = 0; i < COMMAND_COUNT; i++) {
		if (strcmp(commands[i].name, command_names[ci].name))
			continue;

		for (ro = 0; ro < commands[i].ro_count; ro++) {
			opt_enum = commands[i].required_opt_args[ro].opt;
			all_args[opt_enum] = 1;

		}
		for (oo = 0; oo < commands[i].oo_count; oo++) {
			opt_enum = commands[i].optional_opt_args[oo].opt;
			all_args[opt_enum] = 1;
		}
		for (io = 0; io < commands[i].io_count; io++) {
			opt_enum = commands[i].ignore_opt_args[io].opt;
			all_args[opt_enum] = 1;
		}
	}

	for (i = 0; i < ARG_COUNT; i++) {
		if (all_args[i]) {
			opt_enum = _cmdline.opt_names[i].opt_enum;

			command_names[ci].valid_args[num_args] = opt_enum;
			num_args++;

			/* Automatically recognize --extents in addition to --size. */
			if (opt_enum == size_ARG) {
				command_names[ci].valid_args[num_args] = extents_ARG;
				num_args++;
			}

			/* Recognize synonyms */
			if ((opt_syn = _opt_standard_to_synonym(command_names[ci].name, opt_enum))) {
				command_names[ci].valid_args[num_args] = opt_syn;
				num_args++;
			}

			/*
			 * "--allocation" is a weird option that seems to be
			 * a synonym for either allocatable or resizeable,
			 * each which already have their own other synonyms,
			 * so just add allocation whenever either is seen.
			 */
			if ((opt_enum == allocatable_ARG) || (opt_enum == resizeable_ARG)) {
				command_names[ci].valid_args[num_args] = allocation_ARG;
				num_args++;
			}
		}
	}

	command_names[ci].num_args = num_args;
}

static struct command_name *_find_command_name(const char *name)
{
	int i;
	
	for (i = 0; i < MAX_COMMAND_NAMES; i++) {
		if (!command_names[i].name)
			break;
		if (!strcmp(command_names[i].name, name))
			return &command_names[i];
	}
	return NULL;
}

static const struct command_function *_find_command_id_function(int command_enum)
{
	int i;

	if (!command_enum)
		return NULL;

	for (i = 0; i < CMD_COUNT; i++) {
		if (_command_functions[i].command_enum == command_enum)
			return &_command_functions[i];
	}
	return NULL;
}

static void _unregister_commands(void)
{
	_cmdline.commands = NULL;
	_cmdline.num_commands = 0;
	_cmdline.command_names = NULL;
	_cmdline.num_command_names = 0;
	memset(&commands, 0, sizeof(commands));
}

int lvm_register_commands(struct cmd_context *cmd, const char *run_name)
{
	int i;

	/* already initialized */
	if (_cmdline.commands)
		return 1;

	memset(&commands, 0, sizeof(commands));

	/*
	 * populate commands[] array with command definitions
	 * by parsing command-lines.in/command-lines-input.h
	 */
	if (!define_commands(cmd, run_name)) {
		log_error(INTERNAL_ERROR "Failed to parse command definitions.");
		return 0;
	}

	_cmdline.commands = commands;
	_cmdline.num_commands = COMMAND_COUNT;

	for (i = 0; i < COMMAND_COUNT; i++) {
		commands[i].command_enum = command_id_to_enum(commands[i].command_id);

		if (!commands[i].command_enum) {
			log_error(INTERNAL_ERROR "Failed to find command id %s.", commands[i].command_id);
			_cmdline.commands = NULL;
			_cmdline.num_commands = 0;
			return 0;
		}

		/* new style */
		commands[i].functions = _find_command_id_function(commands[i].command_enum);

		/* old style */
		if (!commands[i].functions) {
			struct command_name *cname = _find_command_name(commands[i].name);
			if (cname)
				commands[i].fn = cname->fn;
		}
	}

	_cmdline.command_names = command_names;
	_cmdline.num_command_names = 0;

	for (i = 0; i < MAX_COMMAND_NAMES; i++) {
		if (!command_names[i].name)
			break;
		_cmdline.num_command_names++;
	}

	for (i = 0; i < _cmdline.num_command_names; i++)
		_set_valid_args_for_command_name(i);

	return 1;
}

struct lv_prop *get_lv_prop(int lvp_enum)
{
	if (!lvp_enum)
		return NULL;
	return &lv_props[lvp_enum];
}

struct lv_type *get_lv_type(int lvt_enum)
{
	if (!lvt_enum)
		return NULL;
	return &lv_types[lvt_enum];
}

struct command *get_command(int cmd_enum)
{
	int i;

	for (i = 0; i < COMMAND_COUNT; i++) {
		if (commands[i].command_enum == cmd_enum)
			return &commands[i];
	}

	return NULL;
}

/*
 * Also see merge_synonym().  The command definitions
 * are written using just one variation of the option
 * name (opt below).  This function checks if the user
 * entered a synonym (arg_is_set).
 */

static int _opt_synonym_is_set(struct cmd_context *cmd, int opt_std)
{
	int opt_syn = _opt_standard_to_synonym(cmd->name, opt_std);

	return opt_syn && arg_is_set(cmd, opt_syn);
}

static int _command_optional_opt_matches(struct cmd_context *cmd, int ci, int oo)
{
	int opt_enum = commands[ci].optional_opt_args[oo].opt;

	if (val_bit_is_set(commands[ci].optional_opt_args[oo].def.val_bits, conststr_VAL)) {
		if (!strcmp(commands[ci].optional_opt_args[oo].def.str, arg_str_value(cmd, opt_enum, "")))
			return 1;
		return 0;
	}

	if (val_bit_is_set(commands[ci].optional_opt_args[oo].def.val_bits, constnum_VAL)) {
		if (commands[ci].optional_opt_args[oo].def.num == arg_int_value(cmd, opt_enum, 0))
			return 1;
		return 0;
	}

	return 1;
}

static int _command_ignore_opt_matches(struct cmd_context *cmd, int ci, int io)
{
	int opt_enum = commands[ci].ignore_opt_args[io].opt;

	if (val_bit_is_set(commands[ci].ignore_opt_args[io].def.val_bits, conststr_VAL)) {
		if (!strcmp(commands[ci].ignore_opt_args[io].def.str, arg_str_value(cmd, opt_enum, "")))
			return 1;
		return 0;
	}

	if (val_bit_is_set(commands[ci].ignore_opt_args[io].def.val_bits, constnum_VAL)) {
		if (commands[ci].ignore_opt_args[io].def.num == arg_int_value(cmd, opt_enum, 0))
			return 1;
		return 0;
	}

	return 1;
}

static int _command_required_opt_matches(struct cmd_context *cmd, int ci, int ro)
{
	int opt_enum = commands[ci].required_opt_args[ro].opt;

	if (arg_is_set(cmd, opt_enum) || _opt_synonym_is_set(cmd, opt_enum))
		goto check_val;

	/*
	 * For some commands, --size and --extents are interchangable,
	 * but command[] definitions use only --size.
	 */
	if ((opt_enum == size_ARG) && arg_is_set(cmd, extents_ARG) &&
	    command_has_alternate_extents(commands[ci].name))
		goto check_val;

	return 0;

	/*
	 * If the definition requires a literal string or number, check
	 * that the arg value matches.
	 */

check_val:
	if (val_bit_is_set(commands[ci].required_opt_args[ro].def.val_bits, conststr_VAL)) {
		if (!strcmp(commands[ci].required_opt_args[ro].def.str, arg_str_value(cmd, opt_enum, "")))
			return 1;

		/* Special case: "raid0" (any raid<N>), matches command def "raid" */
		if (!strcmp(commands[ci].required_opt_args[ro].def.str, "raid") &&
		    !strncmp(arg_str_value(cmd, opt_enum, ""), "raid", 4))
			return 1;

		return 0;
	}

	if (val_bit_is_set(commands[ci].required_opt_args[ro].def.val_bits, constnum_VAL)) {
		if (commands[ci].required_opt_args[ro].def.num == arg_int_value(cmd, opt_enum, 0))
			return 1;
		return 0;
	}

	return 1;
}

static int _command_required_pos_matches(struct cmd_context *cmd, int ci, int rp, char **argv)
{
	const char *name;

	/*
	 * rp is the index in required_pos_args[] of the required positional arg.
	 * The pos values begin with 1, so the first positional arg has
	 * pos 1, rp 0.
	 */
	if (argv[rp]) {
		/* FIXME: can we match object type better than just checking something exists? */
		/* Some cases could be validated by looking at defs.types and at the value. */
		return 1;
	}

	/*
	 * If Select is specified as a pos arg, then that pos arg can be
	 * empty if --select is used.
	 */
	if ((val_bit_is_set(commands[ci].required_pos_args[rp].def.val_bits, select_VAL)) &&
	    arg_is_set(cmd, select_ARG))
		return 1;

	/*
	 * For an lvcreate command with VG as the first required positional arg,
	 * the VG position is allowed to be empty if --name VG/LV is used, or if the
	 * LVM_VG_NAME env var is set.
	 *
	 * --thinpool VG/LV and --cachepool VG/LV can also function like --name 
	 * to provide the VG name in place of the positional arg.
	 */
	if (!strcmp(cmd->name, "lvcreate") &&
	    (rp == 0) &&
	    val_bit_is_set(commands[ci].required_pos_args[rp].def.val_bits, vg_VAL) &&
	    (arg_is_set(cmd, name_ARG) ||
	     arg_is_set(cmd, thinpool_ARG) ||
	     arg_is_set(cmd, cachepool_ARG) ||
	     getenv("LVM_VG_NAME"))) {

		if (getenv("LVM_VG_NAME"))
			return 1;

		if ((name = arg_str_value(cmd, name_ARG, NULL))) {
			if (strstr(name, "/"))
				return 1;
		}

		if ((name = arg_str_value(cmd, thinpool_ARG, NULL))) {
			if (strstr(name, "/"))
				return 1;
		}

		if ((name = arg_str_value(cmd, cachepool_ARG, NULL))) {
			if (strstr(name, "/"))
				return 1;
		}
	}

	return 0;
}

/*
 * Match what the user typed with a one specific command definition/prototype
 * from commands[].  If nothing matches, it's not a valid command.  The match
 * is based on command name, required opt args and required pos args.
 *
 * Find an entry in the commands array that matches based the arg values.
 *
 * If the cmd has opt or pos args set that are not accepted by command,
 * we can: silently ignore them, warn they are not being used, or fail.
 * Default should probably be to warn and continue.
 *
 * For each command[i], check how many required opt/pos args cmd matches.
 * Save the command[i] that matches the most.
 *
 * commands[i].cmd_flags & CMD_FLAG_ONE_REQUIRED_OPT means
 * any one item from commands[i].required_opt_args needs to be
 * set to match.
 *
 * required_pos_args[0].types & select_VAL means
 * argv[] in that pos can be NULL if arg_is_set(select_ARG)
 */

/* The max number of unused options we keep track of to warn about */
#define MAX_UNUSED_COUNT 8

#define MAX_OPTS_MSG 64

static struct command *_find_command(struct cmd_context *cmd, const char *path, int *argc, char **argv)
{
	const char *name;
	char opts_msg[MAX_OPTS_MSG];
	char check_opts_msg[MAX_OPTS_MSG];
	int match_required, match_ro, match_rp, match_type, match_unused, mismatch_required;
	int best_i = 0, best_required = 0, best_type = 0, best_unused = 0;
	int close_i = 0, close_ro = 0, close_type = 0;
	int only_i = 0;
	int temp_unused_options[MAX_UNUSED_COUNT];
	int temp_unused_count;
	int best_unused_options[MAX_UNUSED_COUNT] = { 0 };
	int best_unused_count = 0;
	int opts_match_count, opts_unmatch_count;
	int ro, rp;
	int i, j;
	int opt_enum, opt_i;
	int accepted, count;
	int variants = 0;

	name = last_path_component(path);

	/* factor_common_options() is only for usage, so cname->variants is not set. */
	for (i = 0; i < COMMAND_COUNT; i++) {
		if (strcmp(name, commands[i].name))
			continue;
		variants++;
	}

	for (i = 0; i < COMMAND_COUNT; i++) {
		if (strcmp(name, commands[i].name))
			continue;

		if (variants == 1)
			only_i = i;

		/* For help and version just return the first entry with matching name. */
		if (arg_is_set(cmd, help_ARG) || arg_is_set(cmd, help2_ARG) || arg_is_set(cmd, longhelp_ARG) || arg_is_set(cmd, version_ARG))
			return &commands[i];

		/*
		 * The 'lvconvert LV' cmd def matches any lvconvert cmd which throws off
		 * nearest-command partial-match suggestions.  Make it a special case so
		 * that it won't be used as a close match.  If the command has any option
		 * set (other than -v), don't attempt to match it to 'lvconvert LV'.
		 */
		if (commands[i].command_enum == lvconvert_plain_CMD) {
			if (cmd->opt_count - cmd->opt_arg_values[verbose_ARG].count)
				continue;
		}

		match_required = 0;	/* required parameters that match */
		match_ro = 0;		/* required opt_args that match */
		match_rp = 0;		/* required pos_args that match */
		match_type = 0;		/* type arg matches */
		match_unused = 0;	/* options set that are not accepted by command */
		mismatch_required = 0;	/* required parameters that do not match */
		temp_unused_count = 0;
		memset(&temp_unused_options, 0, sizeof(temp_unused_options));

		/* if the command name alone is enough, then that's a match */

		if (!commands[i].ro_count && !commands[i].rp_count)
			match_required = 1;

		/* match required_opt_args */

		for (ro = 0; ro < commands[i].ro_count; ro++) {
			if (_command_required_opt_matches(cmd, i, ro)) {
				/* log_warn("match %d ro opt %d", i, commands[i].required_opt_args[ro].opt); */
				match_required++;
				match_ro++;

				if (commands[i].required_opt_args[ro].opt == type_ARG)
					match_type = 1;
			} else {
				/* cmd is missing a required opt arg */
				/* log_warn("mismatch %d ro opt %d", i, commands[i].required_opt_args[ro].opt); */
				mismatch_required++;
			}
		}

		/*
		 * Special case where missing required_opt_arg's does not matter
		 * if one required_opt_arg did match.
		 */
		if (commands[i].cmd_flags & CMD_FLAG_ONE_REQUIRED_OPT) {
			if (match_ro) {
				/* one or more of the required_opt_args is used */
				mismatch_required = 0;
			} else {
				/* not even one of the required_opt_args is used */
				mismatch_required = 1;
			}
		}

		/* match required_pos_args */

		for (rp = 0; rp < commands[i].rp_count; rp++) {
			if (_command_required_pos_matches(cmd, i, rp, argv)) {
				/* log_warn("match %d rp %d", i, commands[i].required_pos_args[rp].pos); */
				match_required++;
				match_rp++;
			} else {
				/* cmd is missing a required pos arg */
				/* log_warn("mismatch %d rp %d", i, commands[i].required_pos_args[rp].pos); */
				mismatch_required++;
			}
		}

		/* if cmd is missing any required opt/pos args, it can't be this command. */

		if (mismatch_required) {
			/* save "closest" command that doesn't match */
			if ((match_type && !close_type) ||
			    ((match_type == close_type) && (match_ro > close_ro))) {
				close_i = i;
				close_ro = match_ro;
				close_type = match_type;
			}
			continue;
		}

		if (!match_required)
			continue;

		/* Count the command name as a match if all the required opt/pos args match. */

		if ((commands[i].ro_count || commands[i].rp_count) && (match_ro || match_rp))
			match_required++;

		/* log_warn("command %d has match_required %d match_ro %d match_rp %d",
			 i, match_required, match_ro, match_rp); */

		/* Count how many options cmd has set that are not accepted by commands[i]. */
		/* FIXME: also count unused positional args? */

		for (opt_i = 0; opt_i < ARG_COUNT; opt_i++) {
			if (!arg_is_set(cmd, opt_i))
				continue;

			if (!(opt_enum = _opt_synonym_to_standard(cmd->name, opt_i)))
				opt_enum = opt_i;

			/* extents are not used in command definitions */
			if (opt_enum == extents_ARG)
				continue;

			accepted = 0;

			/* NB in some cases required_opt_args are optional */
			for (j = 0; j < commands[i].ro_count; j++) {
				if (commands[i].required_opt_args[j].opt == opt_enum) {
					accepted = 1;
					break;
				}
			}

			if (accepted)
				continue;

			for (j = 0; j < commands[i].oo_count; j++) {
				if ((commands[i].optional_opt_args[j].opt == opt_enum) &&
				    _command_optional_opt_matches(cmd, i, j)) {
					accepted = 1;
					break;
				}
			}

			for (j = 0; j < commands[i].io_count; j++) {
				if ((commands[i].ignore_opt_args[j].opt == opt_enum) &&
				    _command_ignore_opt_matches(cmd, i, j)) {
					accepted = 1;
					break;
				}
			}

			if (!accepted) {
				match_unused++;
				if (temp_unused_count < MAX_UNUSED_COUNT)
					temp_unused_options[temp_unused_count++] = opt_enum;
			}
		}

		/*
		 * Choose the best match, which in general is the command with
		 * the most matching required_{opt,pos}, but it could be a
		 * command with fewer required_{opt,pos} matches in the case
		 * where cmddef1 has more required matches, but a match_unused
		 * and cmddef2 has fewer required matches, but zero match_unused.
		 *
		 * A match is better if:
		 * . more required opt/pos args match
		 * . type arg matches when other doesn't
		 * . less unused options
		 */

		if (!best_required ||
		    ((match_required > best_required) && !match_unused) ||
		    (match_unused < best_unused) ||
		    (match_type > best_type) ||
		    ((match_required == best_required) && (match_type == best_type) && (match_unused < best_unused))) {
			/* log_warn("best %d has match_required %d match_ro %d match_rp %d",
				 i, match_required, match_ro, match_rp); */
			best_i = i;
			best_required = match_required;
			best_type = match_type;
			best_unused = match_unused;
			best_unused_count = temp_unused_count;
			memcpy(&best_unused_options, &temp_unused_options, sizeof(best_unused_options));
		}
	}

	if (!best_required) {
		/* cmd did not have all the required opt/pos args of any command */
		log_error("No command with matching syntax recognised.  Run '%s --help' for more information.", name);

		if (only_i) {
			log_warn("Correct command syntax is:");
			print_usage(&_cmdline.commands[only_i], 0, 0);
		} else if (close_ro) {
			log_warn("Nearest similar command has syntax:");
			print_usage(&_cmdline.commands[close_i], 0, 0);
		}
		return NULL;
	}

	/*
	 * If the user passed an option that is not accepted by the matched
	 * command, then fail.
	 *
	 * FIXME: it might be nice to have a config setting that would turn
	 * these into warnings, and just ignore the unused options.
	 */

	if (best_unused_count) {
		for (i = 0; i < best_unused_count; i++) {
			const char *opt_val = NULL;
			opt_enum = best_unused_options[i];
			opt_val = arg_value(cmd, opt_enum);

			log_error("Command does not accept option: %s%s%s.",
				  arg_long_option_name(opt_enum),
				  opt_val ? " " : "", opt_val ?: "");
		}
		return NULL;
	}

	/*
	 * If the user provided a positional arg that is not accepted by
	 * the mached command, then fail.
	 *
	 * If the last required_pos_arg or the last optional_pos_arg may repeat,
	 * then there won't be unused positional args.
	 *
	 * FIXME: same question as above, should there be a config setting
	 * to just warn/ignore about unused positional args?
	 */

	count = commands[best_i].rp_count;
	if (count && (commands[best_i].required_pos_args[count - 1].def.flags & ARG_DEF_FLAG_MAY_REPEAT))
		goto out;

	count = commands[best_i].op_count;
	if (count && (commands[best_i].optional_pos_args[count - 1].def.flags & ARG_DEF_FLAG_MAY_REPEAT))
		goto out;

	for (count = 0; ; count++) {
		if (!argv[count])
			break;

		if (count >= (commands[best_i].rp_count + commands[best_i].op_count)) {
			log_error("Command does not accept argument: %s.", argv[count]);

			/* FIXME: to warn/ignore, clear so it can't be used when processing. */
			/*
			argv[count] = NULL;
			(*argc)--;
			*/
			return NULL;
		}
	}

out:
	/*
	 * Check any rules related to option combinations.
	 * Other rules are checked after VG is read.
	 */

	for (i = 0; i < commands[best_i].rule_count; i++) {
		struct cmd_rule *rule;
		rule = &commands[best_i].rules[i];

		/*
		 * The rule wants to validate options (check_opts). That can be
		 * done here if the only qualification for the validation is
		 * other options (and not specific LV type or LV property which
		 * are not known here.)
		 */

		if (rule->check_opts_count && !rule->lvt_bits && !rule->lvp_bits) {
			/*
			 * When no opt is specified for applying the rule, then
			 * the rule is always applied, otherwise the rule is
			 * applied when the specific option is set.
			 */
			if (rule->opts_count &&
			    !opt_in_list_is_set(cmd, rule->opts, rule->opts_count, NULL, NULL))
				continue;

			opt_in_list_is_set(cmd, rule->check_opts, rule->check_opts_count,
					   &opts_match_count, &opts_unmatch_count);

			if (opts_match_count && (rule->rule == RULE_INVALID)) {
				memset(opts_msg, 0, sizeof(opts_msg));
				memset(check_opts_msg, 0, sizeof(check_opts_msg));

				if (rule->opts_count)
					opt_array_to_str(cmd, rule->opts, rule->opts_count, opts_msg, sizeof(opts_msg));
				opt_array_to_str(cmd, rule->check_opts, rule->check_opts_count, check_opts_msg, sizeof(check_opts_msg));

				if (rule->opts_count)
					log_error("Command does not accept option combination: %s with %s", opts_msg, check_opts_msg);
				else
					log_error("Command does not accept options: %s", check_opts_msg);
				return NULL;
			}

			if (opts_unmatch_count && (rule->rule == RULE_REQUIRE)) {
				memset(check_opts_msg, 0, sizeof(check_opts_msg));
				opt_array_to_str(cmd, rule->check_opts, rule->check_opts_count, check_opts_msg, sizeof(check_opts_msg));
				log_error("Command requires options: %s", check_opts_msg);
				return NULL;
			}
		}
	}

	log_debug("Recognised command %s (id %d / enum %d).",
		  commands[best_i].command_id, best_i, commands[best_i].command_enum);

	return &commands[best_i];
}

static void _short_usage(const char *name)
{
	log_error("Run `%s --help' for more information.", name);
}

static int _usage(const char *name, int longhelp, int skip_notes)
{
	struct command_name *cname = _find_command_name(name);
	struct command *cmd = NULL;
	int show_full = longhelp;
	int i;

	if (!cname) {
		log_print("%s: no such command.", name);
		return 0;
	}

	configure_command_option_values(name);

	/*
	 * Looks at all variants of each command name and figures out
	 * which options are common to all variants (for compact output)
	 */
	factor_common_options();

	log_print("%s - %s\n", name, cname->desc);

	/* Reduce the default output when there are several variants. */

	if (cname->variants < 3)
		show_full = 1;

	for (i = 0; i < COMMAND_COUNT; i++) {
		if (strcmp(_cmdline.commands[i].name, name))
			continue;

		if (_cmdline.commands[i].cmd_flags & CMD_FLAG_PREVIOUS_SYNTAX)
			continue;

		if ((_cmdline.commands[i].cmd_flags & CMD_FLAG_SECONDARY_SYNTAX) && !show_full)
			continue;

		log_very_verbose("Command definition index %d enum %d id %s",
			         _cmdline.commands[i].command_index,
			         _cmdline.commands[i].command_enum,
			         _cmdline.commands[i].command_id);

		print_usage(&_cmdline.commands[i], 1, 1);
		cmd = &_cmdline.commands[i];
	}

	/* Common options are printed once for all variants of a command name. */
	if (!cmd) {
		log_error(INTERNAL_ERROR "Command %s not found.", name);
		return 0;
	}

	print_usage_common_cmd(cname, cmd);
	print_usage_common_lvm(cname, cmd);

	if (skip_notes)
		return 1;

	if (longhelp)
		print_usage_notes(cname);
	else
		log_print("Use --longhelp to show all options and advanced commands.");

	return 1;
}

static void _usage_all(void)
{
	int i;

	for (i = 0; i < MAX_COMMAND_NAMES; i++) {
		if (!command_names[i].name)
			break;
		_usage(command_names[i].name, 1, 1);
	}

	print_usage_notes(NULL);
}

/*
 * Sets up the arguments to pass to getopt_long().
 *
 * getopt_long() takes a string of short option characters
 * where the char is followed by ":" if the option takes an arg,
 * e.g. "abc:d:"  This string is created in optstrp.
 *
 * getopt_long() also takes an array of struct option which
 * has the name of the long option, if it takes an arg, etc,
 * e.g.
 *
 * option long_options[] = {
 * 	{ "foo", required_argument, 0,  0  },
 * 	{ "bar", no_argument,       0, 'b' }
 * };
 *
 * this array is created in longoptsp.
 *
 * Original comment:
 * Sets up the short and long argument.  If there
 * is no short argument then the index of the
 * argument in the the_args array is set as the
 * long opt value.  Yuck.  Of course this means we
 * can't have more than 'a' long arguments.
 */

static void _add_getopt_arg(int opt_enum, char **optstrp, struct option **longoptsp)
{
	struct opt_name *a = _cmdline.opt_names + opt_enum;

	if (a->short_opt) {
		*(*optstrp)++ = a->short_opt;

		if (a->val_enum)
			*(*optstrp)++ = ':';
	}
#ifdef HAVE_GETOPTLONG
	/* long_arg is "--foo", so +2 is the offset of the name after "--" */

	if (*(a->long_opt + 2)) {
		(*longoptsp)->name = a->long_opt + 2;
		(*longoptsp)->has_arg = a->val_enum ? 1 : 0;
		(*longoptsp)->flag = NULL;

		/*
		 * When getopt_long() sees an option that has an associated
		 * single letter, it returns the ascii value of that letter.
		 * e.g. getopt_long() returns 100 for '-d' or '--debug'
		 * (100 is the ascii value of 'd').
		 *
		 * When getopt_long() sees an option that does not have an
		 * associated single letter, it returns the value of the
		 * the enum for that long option name plus 128.
		 * e.g. getopt_long() returns 139 for --cachepool
		 * (11 is the enum value for --cachepool, so 11+128)
		 */

		if (a->short_opt)
			(*longoptsp)->val = a->short_opt;
		else
			(*longoptsp)->val = opt_enum + 128;
		(*longoptsp)++;
	}
#endif
}

/*
 * getopt_long() has returned goval which indicates which option it's found.
 * We need to translate that goval to an enum value from the args array.
 * 
 * For options with both long and short forms, goval is the character value
 * of the short option.  For options with only a long form, goval is the
 * corresponding enum value plus 128.
 *
 * The trick with character values is that different long options share the
 * same single-letter short form.  So, we have to translate goval to an
 * enum using only the set of valid options for the given command.  And,
 * a command name is not allowed to use two different long options that
 * have the same single-letter short form.
 */

static int _find_arg(const char *cmd_name, int goval)
{
	struct command_name *cname;
	int arg_enum;
	int i;

	if (!(cname = _find_command_name(cmd_name)))
		return -1;

	for (i = 0; i < cname->num_args; i++) {
		arg_enum = cname->valid_args[i];

		/* assert arg_enum == _cmdline.opt_names[arg_enum].arg_enum */

		/* the value returned by getopt matches the ascii value of single letter option */
		if (_cmdline.opt_names[arg_enum].short_opt && (goval == _cmdline.opt_names[arg_enum].short_opt))
			return arg_enum;

		/* the value returned by getopt matches the enum value plus 128 */
		if (!_cmdline.opt_names[arg_enum].short_opt && (goval == (arg_enum + 128)))
			return arg_enum;
	}

	return -1;
}

static int _process_command_line(struct cmd_context *cmd, int *argc, char ***argv)
{
	char str[((ARG_COUNT + 1) * 2) + 1], *ptr = str;
	struct option opts[ARG_COUNT + 1], *o = opts;
	struct opt_name *a;
	struct arg_values *av;
	struct arg_value_group_list *current_group = NULL;
	int arg_enum; /* e.g. foo_ARG */
	int goval;    /* the number returned from getopt_long identifying what it found */
	int i;

	if (!(cmd->opt_arg_values = dm_pool_zalloc(cmd->mem, sizeof(*cmd->opt_arg_values) * ARG_COUNT))) {
		log_fatal("Unable to allocate memory for command line arguments.");
		return 0;
	}

	/*
	 * create the short-form character array (str) and the long-form option
	 * array (opts) to pass to the getopt_long() function.  IOW we generate
	 * the arguments to pass to getopt_long() from the opt_names data.
	 */
	if (cmd->cname)
		for (i = 0; i < cmd->cname->num_args; i++)
			_add_getopt_arg(cmd->cname->valid_args[i], &ptr, &o);

	*ptr = '\0';
	memset(o, 0, sizeof(*o));

	optarg = 0;
	optind = OPTIND_INIT;
	while ((goval = GETOPTLONG_FN(*argc, *argv, str, opts, NULL)) >= 0) {

		if (goval == '?')
			return 0;

		cmd->opt_count++;

		/*
		 * translate the option value used by getopt into the enum
		 * value (e.g. foo_ARG) from the args array.
		 */
		if ((arg_enum = _find_arg(cmd->name, goval)) < 0) {
			log_fatal("Unrecognised option.");
			return 0;
		}

		a = _cmdline.opt_names + arg_enum;

		av = &cmd->opt_arg_values[arg_enum];

		if (a->flags & ARG_GROUPABLE) {
			/*
			 * Start a new group of arguments:
			 *   - the first time,
			 *   - or if a non-countable argument is repeated,
			 *   - or if argument has higher priority than current group.
			 */
			if (!current_group ||
			    (current_group->arg_values[arg_enum].count && !(a->flags & ARG_COUNTABLE)) ||
			    (current_group->prio < a->prio)) {
				/* FIXME Reduce size including only groupable args */
				if (!(current_group = dm_pool_zalloc(cmd->mem, sizeof(struct arg_value_group_list) + sizeof(*cmd->opt_arg_values) * ARG_COUNT))) {
					log_fatal("Unable to allocate memory for command line arguments.");
					return 0;
				}

				current_group->prio = a->prio;
				dm_list_add(&cmd->arg_value_groups, &current_group->list);
			}
			/* Maintain total argument count as well as count within each group */
			av->count++;
			av = &current_group->arg_values[arg_enum];
		}

		if (av->count && !(a->flags & ARG_COUNTABLE)) {
			log_error("Option%s%c%s%s may not be repeated.",
				  a->short_opt ? " -" : "",
				  a->short_opt ? : ' ',
				  (a->short_opt && a->long_opt) ?
				  "/" : "", a->long_opt ? : "");
			return 0;
		}

		if (a->val_enum) {
			if (!optarg) {
				log_error("Option requires argument.");
				return 0;
			}

			av->value = optarg;

			if (!val_names[a->val_enum].fn(cmd, av)) {
				log_error("Invalid argument for %s: %s", a->long_opt, optarg);
				return 0;
			}
		}

		av->count++;
	}

	*argc -= optind;
	*argv += optind;
	return 1;
}

static void _copy_arg_values(struct arg_values *av, int oldarg, int newarg)
{
	const struct arg_values *old = av + oldarg;
	struct arg_values *new = av + newarg;

	new->count = old->count;
	new->value = old->value;
	new->i_value = old->i_value;
	new->ui_value = old->ui_value;
	new->i64_value = old->i64_value;
	new->ui64_value = old->ui64_value;
	new->sign = old->sign;
}

static int _merge_synonym(struct cmd_context *cmd, int oldarg, int newarg)
{
	struct arg_values *av;
	struct arg_value_group_list *current_group;

	if (arg_is_set(cmd, oldarg) && arg_is_set(cmd, newarg)) {
		log_error("%s and %s are synonyms.  Please only supply one.",
			  _cmdline.opt_names[oldarg].long_opt, _cmdline.opt_names[newarg].long_opt);
		return 0;
	}

	/* Not groupable? */
	if (!(_cmdline.opt_names[oldarg].flags & ARG_GROUPABLE)) {
		if (arg_is_set(cmd, oldarg))
			_copy_arg_values(cmd->opt_arg_values, oldarg, newarg);
		return 1;
	}

	if (arg_is_set(cmd, oldarg))
		cmd->opt_arg_values[newarg].count = cmd->opt_arg_values[oldarg].count;

	/* Groupable */
	dm_list_iterate_items(current_group, &cmd->arg_value_groups) {
		av = current_group->arg_values;
		if (!grouped_arg_count(av, oldarg))
			continue;
		_copy_arg_values(av, oldarg, newarg);
	}

	return 1;
}

int systemid(struct cmd_context *cmd __attribute__((unused)),
	     int argc __attribute__((unused)),
	     char **argv __attribute__((unused)))
{
	log_print("system ID: %s", cmd->system_id ? : "");

	return ECMD_PROCESSED;
}

int version(struct cmd_context *cmd __attribute__((unused)),
	    int argc __attribute__((unused)),
	    char **argv __attribute__((unused)))
{
	char vsn[80];

	log_print("LVM version:     %s", LVM_VERSION);
	if (library_version(vsn, sizeof(vsn)))
		log_print("Library version: %s", vsn);
	if (driver_version(vsn, sizeof(vsn)))
		log_print("Driver version:  %s", vsn);
	log_print("Configuration:   %s", LVM_CONFIGURE_LINE);

	return ECMD_PROCESSED;
}

static void _reset_current_settings_to_default(struct cmd_context *cmd)
{
	cmd->current_settings = cmd->default_settings;
}

static void _get_current_output_settings_from_args(struct cmd_context *cmd)
{
	if (arg_is_set(cmd, debug_ARG))
		cmd->current_settings.debug = _LOG_FATAL + (arg_count(cmd, debug_ARG) - 1);

	if (arg_is_set(cmd, verbose_ARG))
		cmd->current_settings.verbose = arg_count(cmd, verbose_ARG);

	if (arg_is_set(cmd, quiet_ARG)) {
		cmd->current_settings.debug = 0;
		cmd->current_settings.verbose = 0;
		cmd->current_settings.silent = (arg_count(cmd, quiet_ARG) > 1) ? 1 : 0;
	}
}

static void _apply_current_output_settings(struct cmd_context *cmd)
{
	init_debug(cmd->current_settings.debug);
	init_debug_classes_logged(cmd->default_settings.debug_classes);
	init_verbose(cmd->current_settings.verbose + VERBOSE_BASE_LEVEL);
	init_silent(cmd->current_settings.silent);
}

static int _get_current_settings(struct cmd_context *cmd)
{
	const char *activation_mode;

	_get_current_output_settings_from_args(cmd);

	if (arg_is_set(cmd, test_ARG))
		cmd->current_settings.test = arg_is_set(cmd, test_ARG);

	if (arg_is_set(cmd, driverloaded_ARG)) {
		cmd->current_settings.activation =
		    arg_int_value(cmd, driverloaded_ARG,
				  cmd->default_settings.activation);
	}

	cmd->current_settings.archive = arg_int_value(cmd, autobackup_ARG, cmd->current_settings.archive);
	cmd->current_settings.backup = arg_int_value(cmd, autobackup_ARG, cmd->current_settings.backup);

	if (arg_is_set(cmd, readonly_ARG)) {
		cmd->current_settings.activation = 0;
		cmd->current_settings.archive = 0;
		cmd->current_settings.backup = 0;
	}

	if (cmd->cname->flags & LOCKD_VG_SH)
		cmd->lockd_vg_default_sh = 1;

	if (cmd->cname->flags & CAN_USE_ONE_SCAN)
		cmd->can_use_one_scan = 1;

	cmd->partial_activation = 0;
	cmd->degraded_activation = 0;
	activation_mode = find_config_tree_str(cmd, activation_mode_CFG, NULL);
	if (!activation_mode)
		activation_mode = DEFAULT_ACTIVATION_MODE;

	if (arg_is_set(cmd, activationmode_ARG)) {
		activation_mode = arg_str_value(cmd, activationmode_ARG,
						activation_mode);

		/* complain only if the two arguments conflict */
		if (arg_is_set(cmd, partial_ARG) &&
		    strcmp(activation_mode, "partial")) {
			log_error("--partial and --activationmode are mutually"
				  " exclusive arguments");
			return EINVALID_CMD_LINE;
		}
	} else if (arg_is_set(cmd, partial_ARG))
		activation_mode = "partial";

	if (!strcmp(activation_mode, "partial")) {
		cmd->partial_activation = 1;
		log_warn("PARTIAL MODE. Incomplete logical volumes will be processed.");
	} else if (!strcmp(activation_mode, "degraded"))
		cmd->degraded_activation = 1;
	else if (strcmp(activation_mode, "complete")) {
		log_error("Invalid activation mode given.");
		return EINVALID_CMD_LINE;
	}

	cmd->include_foreign_vgs = arg_is_set(cmd, foreign_ARG) ? 1 : 0;
	cmd->include_shared_vgs = arg_is_set(cmd, shared_ARG) ? 1 : 0;
	cmd->include_historical_lvs = arg_is_set(cmd, history_ARG) ? 1 : 0;
	cmd->record_historical_lvs = find_config_tree_bool(cmd, metadata_record_lvs_history_CFG, NULL) ?
							  (arg_is_set(cmd, nohistory_ARG) ? 0 : 1) : 0;

	/*
	 * This is set to zero by process_each which wants to print errors
	 * itself rather than having them printed in vg_read.
	 */
	cmd->vg_read_print_access_error = 1;
		
	if (arg_is_set(cmd, nosuffix_ARG))
		cmd->current_settings.suffix = 0;

	if (arg_is_set(cmd, units_ARG))
		if (!(cmd->current_settings.unit_factor =
		      dm_units_to_factor(arg_str_value(cmd, units_ARG, ""),
					 &cmd->current_settings.unit_type, 1, NULL))) {
			log_error("Invalid units specification");
			return EINVALID_CMD_LINE;
		}

	if (arg_is_set(cmd, binary_ARG))
		cmd->report_binary_values_as_numeric = 1;

	if (arg_is_set(cmd, trustcache_ARG)) {
		if (arg_is_set(cmd, all_ARG)) {
			log_error("--trustcache is incompatible with --all");
			return EINVALID_CMD_LINE;
		}
		init_trust_cache(1);
		log_warn("WARNING: Cache file of PVs will be trusted.  "
			  "New devices holding PVs may get ignored.");
	} else
		init_trust_cache(0);

	if (arg_is_set(cmd, noudevsync_ARG))
		cmd->current_settings.udev_sync = 0;

	/* Handle synonyms */
	if (!_merge_synonym(cmd, resizable_ARG, resizeable_ARG) ||
	    !_merge_synonym(cmd, allocation_ARG, allocatable_ARG) ||
	    !_merge_synonym(cmd, allocation_ARG, resizeable_ARG) ||
	    !_merge_synonym(cmd, virtualoriginsize_ARG, virtualsize_ARG) ||
	    !_merge_synonym(cmd, available_ARG, activate_ARG) ||
	    !_merge_synonym(cmd, raidrebuild_ARG, rebuild_ARG) ||
	    !_merge_synonym(cmd, raidsyncaction_ARG, syncaction_ARG) ||
	    !_merge_synonym(cmd, raidwritemostly_ARG, writemostly_ARG) ||
	    !_merge_synonym(cmd, raidminrecoveryrate_ARG, minrecoveryrate_ARG) ||
	    !_merge_synonym(cmd, raidmaxrecoveryrate_ARG, maxrecoveryrate_ARG) ||
	    !_merge_synonym(cmd, raidwritebehind_ARG, writebehind_ARG))
		return EINVALID_CMD_LINE;

	if ((!strncmp(cmd->name, "pv", 2) &&
	    !_merge_synonym(cmd, metadatacopies_ARG, pvmetadatacopies_ARG)) ||
	    (!strncmp(cmd->name, "vg", 2) &&
	     !_merge_synonym(cmd, metadatacopies_ARG, vgmetadatacopies_ARG)))
		return EINVALID_CMD_LINE;

	/* Zero indicates success */
	return 0;
}

static int _process_common_commands(struct cmd_context *cmd)
{
	if (arg_is_set(cmd, help_ARG) ||
	    arg_is_set(cmd, longhelp_ARG) ||
	    arg_is_set(cmd, help2_ARG)) {
		_usage(cmd->name, arg_is_set(cmd, longhelp_ARG), 0);
		return ECMD_PROCESSED;
	}

	if (arg_is_set(cmd, version_ARG)) {
		return version(cmd, 0, (char **) NULL);
	}

	/* Zero indicates it's OK to continue processing this command */
	return 0;
}

static void _display_help(void)
{
	int i;

	log_error("Available lvm commands:");
	log_error("Use 'lvm help <command>' for more information");
	log_error(" ");

	for (i = 0; i < _cmdline.num_command_names; i++) {
		struct command_name *cname = _cmdline.command_names + i;

		log_error("%-16.16s%s", cname->name, cname->desc);
	}
}

int help(struct cmd_context *cmd __attribute__((unused)), int argc, char **argv)
{
	int ret = ECMD_PROCESSED;

	if (!argc)
		_display_help();
	else if (argc == 1 && !strcmp(argv[0], "all"))
		_usage_all();
	else {
		int i;
		for (i = 0; i < argc; i++)
			if (!_usage(argv[i], 0, 0))
				ret = EINVALID_CMD_LINE;
	}

	return ret;
}

static void _apply_current_settings(struct cmd_context *cmd)
{
	_apply_current_output_settings(cmd);

	init_test(cmd->current_settings.test);
	init_mirror_in_sync(0);
	init_dmeventd_monitor(DEFAULT_DMEVENTD_MONITOR);

	init_msg_prefix(cmd->default_settings.msg_prefix);
	init_cmd_name(cmd->default_settings.cmd_name);

	archive_enable(cmd, cmd->current_settings.archive);
	backup_enable(cmd, cmd->current_settings.backup);

	set_activation(cmd->current_settings.activation, cmd->metadata_read_only);

	cmd->fmt = get_format_by_name(cmd, arg_str_value(cmd, metadatatype_ARG,
				      cmd->current_settings.fmt_name));

	cmd->handles_missing_pvs = 0;
}

static const char *_copy_command_line(struct cmd_context *cmd, int argc, char **argv)
{
	int i, space;

	/*
	 * Build up the complete command line, used as a
	 * description for backups.
	 */
	if (!dm_pool_begin_object(cmd->mem, 128))
		goto_bad;

	for (i = 0; i < argc; i++) {
		space = strchr(argv[i], ' ') ? 1 : 0;

		if (space && !dm_pool_grow_object(cmd->mem, "'", 1))
			goto_bad;

		if (!dm_pool_grow_object(cmd->mem, argv[i], strlen(argv[i])))
			goto_bad;

		if (space && !dm_pool_grow_object(cmd->mem, "'", 1))
			goto_bad;

		if (i < (argc - 1))
			if (!dm_pool_grow_object(cmd->mem, " ", 1))
				goto_bad;
	}

	/*
	 * Terminate.
	 */
	if (!dm_pool_grow_object(cmd->mem, "\0", 1))
		goto_bad;

	return dm_pool_end_object(cmd->mem);

      bad:
	log_error("Couldn't copy command line.");
	dm_pool_abandon_object(cmd->mem);
	return NULL;
}

static int _prepare_profiles(struct cmd_context *cmd)
{
	static const char COMMAND_PROFILE_ENV_VAR_NAME[] = "LVM_COMMAND_PROFILE";
	static const char _cmd_profile_arg_preferred_over_env_var_msg[] = "Giving "
				"preference to command profile specified on command "
				"line over the one specified via environment variable.";
	static const char _failed_to_add_profile_msg[] = "Failed to add %s %s.";
	static const char _failed_to_apply_profile_msg[] = "Failed to apply %s %s.";
	static const char _command_profile_source_name[] = "command profile";
	static const char _metadata_profile_source_name[] = "metadata profile";
	static const char _setting_global_profile_msg[] = "Setting global %s \"%s\".";

	const char *env_cmd_profile_name = NULL;
	const char *name;
	struct profile *profile;
	config_source_t source;
	const char *source_name;

	/* Check whether default global command profile is set via env. var. */
	if ((env_cmd_profile_name = getenv(COMMAND_PROFILE_ENV_VAR_NAME))) {
		if (!*env_cmd_profile_name)
			env_cmd_profile_name = NULL;
		else
			log_debug("Command profile '%s' requested via "
				  "environment variable.",
				   env_cmd_profile_name);
	}

	if (!arg_is_set(cmd, profile_ARG) &&
	    !arg_is_set(cmd, commandprofile_ARG) &&
	    !arg_is_set(cmd, metadataprofile_ARG) &&
	    !env_cmd_profile_name)
		/* nothing to do */
		return 1;

	if (arg_is_set(cmd, profile_ARG)) {
		/*
		 * If --profile is used with dumpconfig, it's used
		 * to dump the profile without the profile being applied.
		 */
		if (!strcmp(cmd->command->name, "dumpconfig") ||
		    !strcmp(cmd->command->name, "lvmconfig") ||
		    !strcmp(cmd->command->name, "config"))
			return 1;

		/*
		 * If --profile is used with lvcreate/lvchange/vgchange,
		 * it's recognized as shortcut to --metadataprofile.
		 * The --commandprofile is assumed otherwise.
		 */
		if (!strcmp(cmd->command->name, "lvcreate") ||
		    !strcmp(cmd->command->name, "vgcreate") ||
		    !strcmp(cmd->command->name, "lvchange") ||
		    !strcmp(cmd->command->name, "vgchange")) {
			if (arg_is_set(cmd, metadataprofile_ARG)) {
				log_error("Only one of --profile or "
					  " --metadataprofile allowed.");
				return 0;
			}
			source = CONFIG_PROFILE_METADATA;
			source_name = _metadata_profile_source_name;
		}
		else {
			if (arg_is_set(cmd, commandprofile_ARG)) {
				log_error("Only one of --profile or "
					  "--commandprofile allowed.");
				return 0;
			}
			/*
			 * Prefer command profile specified on command
			 * line over the profile specified via
			 * COMMAND_PROFILE_ENV_VAR_NAME env. var.
			 */
			if (env_cmd_profile_name) {
				log_debug(_cmd_profile_arg_preferred_over_env_var_msg);
				env_cmd_profile_name = NULL;
			}
			source = CONFIG_PROFILE_COMMAND;
			source_name = _command_profile_source_name;
		}

		name = arg_str_value(cmd, profile_ARG, NULL);

		if (!(profile = add_profile(cmd, name, source))) {
			log_error(_failed_to_add_profile_msg, source_name, name);
			return 0;
		}

		if (source == CONFIG_PROFILE_COMMAND) {
			log_debug(_setting_global_profile_msg, _command_profile_source_name, profile->name);
			cmd->profile_params->global_command_profile = profile;
		} else if (source == CONFIG_PROFILE_METADATA) {
			log_debug(_setting_global_profile_msg, _metadata_profile_source_name, profile->name);
			/* This profile will override any VG/LV-based profile if present */
			cmd->profile_params->global_metadata_profile = profile;
		}

		remove_config_tree_by_source(cmd, source);
		if (!override_config_tree_from_profile(cmd, profile)) {
			log_error(_failed_to_apply_profile_msg, source_name, name);
			return 0;
		}

	}

	if (arg_is_set(cmd, commandprofile_ARG) || env_cmd_profile_name) {
		if (arg_is_set(cmd, commandprofile_ARG)) {
			/*
			 * Prefer command profile specified on command
			 * line over the profile specified via
			 * COMMAND_PROFILE_ENV_VAR_NAME env. var.
			 */
			if (env_cmd_profile_name)
				log_debug(_cmd_profile_arg_preferred_over_env_var_msg);
			name = arg_str_value(cmd, commandprofile_ARG, NULL);
		} else
			name = env_cmd_profile_name;
		source_name = _command_profile_source_name;

		if (!(profile = add_profile(cmd, name, CONFIG_PROFILE_COMMAND))) {
			log_error(_failed_to_add_profile_msg, source_name, name);
			return 0;
		}

		remove_config_tree_by_source(cmd, CONFIG_PROFILE_COMMAND);
		if (!override_config_tree_from_profile(cmd, profile)) {
			log_error(_failed_to_apply_profile_msg, source_name, name);
			return 0;
		}

		log_debug(_setting_global_profile_msg, _command_profile_source_name, profile->name);
		cmd->profile_params->global_command_profile = profile;

		if (!cmd->opt_arg_values)
			cmd->profile_params->shell_profile = profile;
	}


	if (arg_is_set(cmd, metadataprofile_ARG)) {
		name = arg_str_value(cmd, metadataprofile_ARG, NULL);
		source_name = _metadata_profile_source_name;

		if (!(profile = add_profile(cmd, name, CONFIG_PROFILE_METADATA))) {
			log_error(_failed_to_add_profile_msg, source_name, name);
			return 0;
		}
		remove_config_tree_by_source(cmd, CONFIG_PROFILE_METADATA);
		if (!override_config_tree_from_profile(cmd, profile)) {
			log_error(_failed_to_apply_profile_msg, source_name, name);
			return 0;
		}

		log_debug(_setting_global_profile_msg, _metadata_profile_source_name, profile->name);
		cmd->profile_params->global_metadata_profile = profile;
	}

	if (!process_profilable_config(cmd))
		return_0;

	return 1;
}

static int _init_lvmlockd(struct cmd_context *cmd)
{
	const char *lvmlockd_socket;
	int use_lvmlockd = find_config_tree_bool(cmd, global_use_lvmlockd_CFG, NULL);

	if (use_lvmlockd && arg_is_set(cmd, nolocking_ARG)) {
		/* --nolocking is only allowed with vgs/lvs/pvs commands */
		cmd->lockd_gl_disable = 1;
		cmd->lockd_vg_disable = 1;
		cmd->lockd_lv_disable = 1;
		return 1;
	}

	if (use_lvmlockd && arg_is_set(cmd, lockopt_ARG)) {
		const char *opts = arg_str_value(cmd, lockopt_ARG, "");
		if (strstr(opts, "skiplv")) {
			log_warn("WARNING: skipping LV lock in lvmlockd.");
			cmd->lockd_lv_disable = 1;
		}
		if (strstr(opts, "skipvg")) {
			log_warn("WARNING: skipping VG lock in lvmlockd.");
			cmd->lockd_vg_disable = 1;
		}
		if (strstr(opts, "skipgl")) {
			log_warn("WARNING: skipping global lock in lvmlockd.");
			cmd->lockd_gl_disable = 1;
		}
	}

	lvmlockd_disconnect(); /* start over when tool context is refreshed */
	lvmlockd_socket = getenv("LVM_LVMLOCKD_SOCKET");
	if (!lvmlockd_socket)
		lvmlockd_socket = DEFAULT_RUN_DIR "/lvmlockd.socket";

	lvmlockd_set_socket(lvmlockd_socket);
	lvmlockd_set_use(use_lvmlockd);
	if (use_lvmlockd) {
		lvmlockd_init(cmd);
		lvmlockd_connect();
	}

	return 1;
}

static int _cmd_no_meta_proc(struct cmd_context *cmd)
{
	return cmd->cname->flags & NO_METADATA_PROCESSING;
}

int lvm_run_command(struct cmd_context *cmd, int argc, char **argv)
{
	struct dm_config_tree *config_string_cft, *config_profile_command_cft, *config_profile_metadata_cft;
	int ret = 0;
	int locking_type;
	int nolocking = 0;
	int readonly = 0;
	int sysinit = 0;
	int monitoring;
	char *arg_new, *arg;
	int i;
	int skip_hyphens;
	int refresh_done = 0;

	init_error_message_produced(0);

	/* each command should start out with sigint flag cleared */
	sigint_clear();

	if (!(cmd->name = dm_pool_strdup(cmd->mem, dm_basename(argv[0])))) {
		log_error("Failed to strdup command basename.");
		return ECMD_FAILED;
	}

	configure_command_option_values(cmd->name);

	/* eliminate '-' from all options starting with -- */
	for (i = 1; i < argc; i++) {

		arg = argv[i];

		if (*arg++ != '-' || *arg++ != '-')
			continue;

		/* If we reach "--" then stop. */
		if (!*arg)
			break;

		arg_new = arg;
		skip_hyphens = 1;
		while (*arg) {
			/* If we encounter '=', stop any further hyphen removal. */
			if (*arg == '=')
				skip_hyphens = 0;

			/* Do we need to keep the next character? */
			if (*arg != '-' || !skip_hyphens) {
				if (arg_new != arg)
					*arg_new = *arg;
				++arg_new;
			}
			arg++;
		}

		/* Terminate a shortened arg */
		if (arg_new != arg)
			*arg_new = '\0';
	}

	/* The cmd_line string is only used for logging, not processing. */
	if (!(cmd->cmd_line = _copy_command_line(cmd, argc, argv)))
		return_ECMD_FAILED;

	/* Look up command - will be NULL if not recognised */
	if (!(cmd->cname = _find_command_name(cmd->name)))
		return ENO_SUCH_CMD;

	if (!_process_command_line(cmd, &argc, &argv)) {
		log_error("Error during parsing of command line.");
		return EINVALID_CMD_LINE;
	}

	/*
	 * Now we have the command line args, set up any known output logging
	 * options immediately.
	 */
	_reset_current_settings_to_default(cmd);
	_get_current_output_settings_from_args(cmd);
	_apply_current_output_settings(cmd);

	log_debug("Parsing: %s", cmd->cmd_line);

	if (!(cmd->command = _find_command(cmd, cmd->name, &argc, argv)))
		return EINVALID_CMD_LINE;

	/*
	 * Remaining position args after command name and --options are removed.
	 */
	cmd->position_argc = argc;
	cmd->position_argv = argv;

	set_cmd_name(cmd->name);

	if (arg_is_set(cmd, config_ARG))
		if (!override_config_tree_from_string(cmd, arg_str_value(cmd, config_ARG, ""))) {
			ret = EINVALID_CMD_LINE;
			goto_out;
		}

	if (arg_is_set(cmd, config_ARG) || !cmd->initialized.config || config_files_changed(cmd)) {
		/* Reinitialise various settings inc. logging, filters */
		if (!refresh_toolcontext(cmd)) {
			if ((config_string_cft = remove_config_tree_by_source(cmd, CONFIG_STRING)))
				dm_config_destroy(config_string_cft);
			log_error("Updated config file invalid. Aborting.");
			return ECMD_FAILED;
		}
		refresh_done = 1;
	}

	if (!_prepare_profiles(cmd))
		return_ECMD_FAILED;

	if (!cmd->initialized.connections && !_cmd_no_meta_proc(cmd) && !init_connections(cmd))
		return_ECMD_FAILED;

	if (!cmd->initialized.filters && !_cmd_no_meta_proc(cmd) &&
	    !init_filters(cmd, !refresh_done))
		return_ECMD_FAILED;

	if (arg_is_set(cmd, readonly_ARG))
		cmd->metadata_read_only = 1;

	if ((cmd->command->command_enum == vgchange_activate_CMD) ||
	    (cmd->command->command_enum == lvchange_activate_CMD))
		cmd->is_activating = 1;

	/*
	 * Now that all configs, profiles and command lines args are available,
	 * freshly calculate and apply all settings.  Specific command line
	 * options take precedence over config files (which include --config as
	 * that is treated like a config file).
	 */
	_reset_current_settings_to_default(cmd);
	if ((ret = _get_current_settings(cmd)))
		goto_out;
	_apply_current_settings(cmd);

	if (cmd->degraded_activation)
		log_debug("DEGRADED MODE. Incomplete RAID LVs will be processed.");

	if (!get_activation_monitoring_mode(cmd, &monitoring))
		goto_out;
	init_dmeventd_monitor(monitoring);

	log_debug("Processing command: %s", cmd->cmd_line);
	log_debug("Command pid: %d", getpid());
	log_debug("System ID: %s", cmd->system_id ? : "");

#ifdef O_DIRECT_SUPPORT
	log_debug("O_DIRECT will be used");
#endif

	if ((ret = _process_common_commands(cmd))) {
		if (ret != ECMD_PROCESSED)
			stack;
		goto out;
	}

	if (cmd->metadata_read_only &&
	    !(cmd->cname->flags & PERMITTED_READ_ONLY)) {
		log_error("%s: Command not permitted while global/metadata_read_only "
			  "is set.", cmd->cmd_line);
		goto out;
	}

	/* Defaults to 1 if not set. */
	locking_type = find_config_tree_int(cmd, global_locking_type_CFG, NULL);

	if (locking_type == 3)
		log_warn("WARNING: see lvmlockd(8) for information on using cluster/clvm VGs.");

	if ((locking_type == 0) || (locking_type == 5)) {
		log_warn("WARNING: locking_type (%d) is deprecated, using --nolocking.", locking_type);
		nolocking = 1;

	} else if (locking_type == 4) {
		log_warn("WARNING: locking_type (%d) is deprecated, using --sysinit --readonly.", locking_type);
		sysinit = 1;
		readonly = 1;

	} else if (locking_type != 1) {
		log_warn("WARNING: locking_type (%d) is deprecated, using file locking.", locking_type);
	}

	if (arg_is_set(cmd, nolocking_ARG) || _cmd_no_meta_proc(cmd))
		nolocking = 1;

	if (arg_is_set(cmd, sysinit_ARG))
		sysinit = 1;

	if (arg_is_set(cmd, readonly_ARG))
		readonly = 1;

	if (nolocking) {
		if (!_cmd_no_meta_proc(cmd))
			log_warn("WARNING: File locking is disabled.");
	} else {
		if (!init_locking(cmd, sysinit, readonly, arg_is_set(cmd, ignorelockingfailure_ARG))) {
			ret = ECMD_FAILED;
			goto_out;
		}
	}

	if (!_cmd_no_meta_proc(cmd) && !_init_lvmlockd(cmd)) {
		ret = ECMD_FAILED;
		goto_out;
	}

	if (cmd->command->functions)
		/* A command-line-specific function is used */
		ret = cmd->command->functions->fn(cmd, argc, argv);
	else
		/* The old style command-name function is used */
		ret = cmd->command->fn(cmd, argc, argv);

	lvmlockd_disconnect();
	fin_locking();

	if (!_cmd_no_meta_proc(cmd) && find_config_tree_bool(cmd, global_notify_dbus_CFG, NULL))
		lvmnotify_send(cmd);

      out:

	lvmcache_destroy(cmd, 1, 1);
	label_scan_destroy(cmd);

	if ((config_string_cft = remove_config_tree_by_source(cmd, CONFIG_STRING)))
		dm_config_destroy(config_string_cft);

	config_profile_command_cft = remove_config_tree_by_source(cmd, CONFIG_PROFILE_COMMAND);
	config_profile_metadata_cft = remove_config_tree_by_source(cmd, CONFIG_PROFILE_METADATA);
	cmd->profile_params->global_metadata_profile = NULL;

	if (config_string_cft) {
		/* Move this? */
		if (!refresh_toolcontext(cmd))
			stack;
	} else if (config_profile_command_cft || config_profile_metadata_cft) {
		if (!process_profilable_config(cmd))
			stack;
	}

	if (ret == EINVALID_CMD_LINE && !cmd->is_interactive)
		_short_usage(cmd->command->name);

	log_debug("Completed: %s", cmd->cmd_line);

	/*
	 * Reset all settings back to the persistent defaults that
	 * ignore everything supplied on the command line of the
	 * completed command.
	 */
	_reset_current_settings_to_default(cmd);
	_apply_current_settings(cmd);

	/*
	 * free off any memory the command used.
	 */
	dm_list_init(&cmd->arg_value_groups);
	dm_pool_empty(cmd->mem);

	reset_lvm_errno(1);
	reset_log_duplicated();

	return ret;
}

int lvm_return_code(int ret)
{
	unlink_log_file(ret);

	return (ret == ECMD_PROCESSED ? 0 : ret);
}

int lvm_split(char *str, int *argc, char **argv, int max)
{
	char *b = str, *e;
	char quote = 0;
	*argc = 0;

	while (*b) {
		while (*b && isspace(*b))
			b++;

		if ((!*b) || (*b == '#'))
			break;

		if (*b == '\'' || *b == '"') {
			quote = *b;
			b++;
		}

		e = b;
		while (*e && (quote ? *e != quote : !isspace(*e)))
			e++;

		argv[(*argc)++] = b;
		if (!*e)
			break;
		*e++ = '\0';
		quote = 0;
		b = e;
		if (*argc == max)
			break;
	}

	if (*argc < max)
		argv[*argc] = NULL;

	return *argc;
}

/* Make sure we have always valid filedescriptors 0,1,2 */
static int _check_standard_fds(void)
{
	int err = is_valid_fd(STDERR_FILENO);

	if (!is_valid_fd(STDIN_FILENO) &&
	    !(stdin = fopen(_PATH_DEVNULL, "r"))) {
		if (err)
			perror("stdin stream open");
		else
			printf("stdin stream open: %s\n",
			       strerror(errno));
		return 0;
	}

	if (!is_valid_fd(STDOUT_FILENO) &&
	    !(stdout = fopen(_PATH_DEVNULL, "w"))) {
		if (err)
			perror("stdout stream open");
		/* else no stdout */
		return 0;
	}

	if (!is_valid_fd(STDERR_FILENO) &&
	    !(stderr = fopen(_PATH_DEVNULL, "w"))) {
		printf("stderr stream open: %s\n",
		       strerror(errno));
		return 0;
	}

	return 1;
}

#define LVM_OUT_FD_ENV_VAR_NAME    "LVM_OUT_FD"
#define LVM_ERR_FD_ENV_VAR_NAME    "LVM_ERR_FD"
#define LVM_REPORT_FD_ENV_VAR_NAME "LVM_REPORT_FD"

static int _do_get_custom_fd(const char *env_var_name, int *fd)
{
	const char *str;
	char *endptr;
	long int tmp_fd;

	*fd = -1;

	if (!(str = getenv(env_var_name)))
		return 1;

	errno = 0;
	tmp_fd = strtol(str, &endptr, 10);
	if (errno || *endptr || (tmp_fd < 0) || (tmp_fd > INT_MAX)) {
		log_error("%s: invalid file descriptor.", env_var_name);
		return 0;
	}

	*fd = tmp_fd;
	return 1;
}

static int _get_custom_fds(struct custom_fds *custom_fds)
{
	return _do_get_custom_fd(LVM_OUT_FD_ENV_VAR_NAME, &custom_fds->out) &&
	       _do_get_custom_fd(LVM_ERR_FD_ENV_VAR_NAME, &custom_fds->err) &&
	       _do_get_custom_fd(LVM_REPORT_FD_ENV_VAR_NAME, &custom_fds->report);
}

static const char *_get_cmdline(pid_t pid)
{
	static char _proc_cmdline[32];
	char buf[256];
	int fd, n = 0;

	snprintf(buf, sizeof(buf), DEFAULT_PROC_DIR "/%u/cmdline", pid);
	/* FIXME Use generic read code. */
	if ((fd = open(buf, O_RDONLY)) >= 0) {
		if ((n = read(fd, _proc_cmdline, sizeof(_proc_cmdline) - 1)) < 0) {
			log_sys_error("read", buf);
			n = 0;
		}
		if (close(fd))
			log_sys_error("close", buf);
	}
	_proc_cmdline[n] = '\0';

	return _proc_cmdline;
}

static const char *_get_filename(int fd)
{
	static char filename[PATH_MAX];
	char buf[32];	/* Assumes short DEFAULT_PROC_DIR */
	int size;

	snprintf(buf, sizeof(buf), DEFAULT_PROC_DIR "/self/fd/%u", fd);

	if ((size = readlink(buf, filename, sizeof(filename) - 1)) == -1)
		filename[0] = '\0';
	else
		filename[size] = '\0';

	return filename;
}

static void _close_descriptor(int fd, unsigned suppress_warnings,
			      const char *command, pid_t ppid,
			      const char *parent_cmdline)
{
	int r;
	const char *filename;

	/* Ignore bad file descriptors */
	if (!is_valid_fd(fd))
		return;

	if (!suppress_warnings)
		filename = _get_filename(fd);

	r = close(fd);
	if (suppress_warnings)
		return;

	if (!r)
		fprintf(stderr, "File descriptor %d (%s) leaked on "
			"%s invocation.", fd, filename, command);
	else if (errno == EBADF)
		return;
	else
		fprintf(stderr, "Close failed on stray file descriptor "
			"%d (%s): %s", fd, filename, strerror(errno));

	fprintf(stderr, " Parent PID %" PRIpid_t ": %s\n", ppid, parent_cmdline);
}

static int _close_stray_fds(const char *command, struct custom_fds *custom_fds)
{
#ifndef VALGRIND_POOL
	struct rlimit rlim;
	int fd;
	unsigned suppress_warnings = 0;
	pid_t ppid = getppid();
	const char *parent_cmdline = _get_cmdline(ppid);
	static const char _fd_dir[] = DEFAULT_PROC_DIR "/self/fd";
	struct dirent *dirent;
	DIR *d;

#ifdef HAVE_VALGRIND
	if (RUNNING_ON_VALGRIND) {
		log_debug("Skipping close of descriptors within valgrind execution.");
		return 1;
	}
#endif

	if (getenv("LVM_SUPPRESS_FD_WARNINGS"))
		suppress_warnings = 1;

	if (!(d = opendir(_fd_dir))) {
		if (errno != ENOENT) {
			log_sys_error("opendir", _fd_dir);
			return 0; /* broken system */
		}

		/* Path does not exist, use the old way */
		if (getrlimit(RLIMIT_NOFILE, &rlim) < 0) {
			log_sys_error("getrlimit", "RLIMIT_NOFILE");
			return 1;
		}

		for (fd = 3; fd < (int)rlim.rlim_cur; fd++) {
			if ((fd != custom_fds->out) &&
			    (fd != custom_fds->err) &&
			    (fd != custom_fds->report)) {
				_close_descriptor(fd, suppress_warnings, command, ppid,
						  parent_cmdline);
			}
		}
		return 1;
	}

	while ((dirent = readdir(d))) {
		fd = atoi(dirent->d_name);
		if ((fd > 2) &&
		    (fd != dirfd(d)) &&
		    (fd != custom_fds->out) &&
		    (fd != custom_fds->err) &&
		    (fd != custom_fds->report)) {
			_close_descriptor(fd, suppress_warnings,
					  command, ppid, parent_cmdline);
		}
	}

	if (closedir(d))
		log_sys_error("closedir", _fd_dir);
#endif

	return 1;
}

struct cmd_context *init_lvm(unsigned set_connections, unsigned set_filters)
{
	struct cmd_context *cmd;

	if (!udev_init_library_context())
		stack;

	/*
	 * It's not necessary to use name mangling for LVM:
	 *   - the character set used for LV names is subset of udev character set
	 *   - when we check other devices (e.g. device_is_usable fn), we use major:minor, not dm names
	 */
	dm_set_name_mangling_mode(DM_STRING_MANGLING_NONE);

	if (!(cmd = create_toolcontext(0, NULL, 1, 0,
			set_connections, set_filters))) {
		udev_fin_library_context();
		return_NULL;
	}

	_cmdline.opt_names = &opt_names[0];

	if (stored_errno()) {
		destroy_toolcontext(cmd);
		udev_fin_library_context();
		return_NULL;
	}

	return cmd;
}

void lvm_fin(struct cmd_context *cmd)
{
	_unregister_commands();
	destroy_toolcontext(cmd);
	udev_fin_library_context();
}

static int _run_script(struct cmd_context *cmd, int argc, char **argv)
{
	FILE *script;
	char buffer[CMD_LEN];
	int ret = ENO_SUCH_CMD;
	int magic_number = 0;
	char *script_file = argv[0];

	if ((script = fopen(script_file, "r")) == NULL)
		return ENO_SUCH_CMD;

	while (fgets(buffer, sizeof(buffer), script) != NULL) {
		if (!magic_number) {
			if (buffer[0] == '#' && buffer[1] == '!')
				magic_number = 1;
			else {
				ret = ENO_SUCH_CMD;
				break;
			}
		}
		if ((strlen(buffer) == sizeof(buffer) - 1)
		    && (buffer[sizeof(buffer) - 1] - 2 != '\n')) {
			buffer[50] = '\0';
			log_error("Line too long (max 255) beginning: %s",
				  buffer);
			ret = EINVALID_CMD_LINE;
			break;
		}
		if (lvm_split(buffer, &argc, argv, MAX_ARGS) == MAX_ARGS) {
			buffer[50] = '\0';
			log_error("Too many arguments: %s", buffer);
			ret = EINVALID_CMD_LINE;
			break;
		}
		if (!argc)
			continue;
		if (!strcmp(argv[0], "quit") || !strcmp(argv[0], "exit"))
			break;
		ret = lvm_run_command(cmd, argc, argv);
		/*
		 * FIXME: handling scripts with invalid or failing commands
		 * could use some cleaning up, e.g. error_message_produced
		 * check and error are repeated again in the caller.
		 */
		if (ret == ENO_SUCH_CMD)
			break;
		if (ret != ECMD_PROCESSED) {
			if (!error_message_produced()) {
				log_debug(INTERNAL_ERROR "Failed command did not use log_error");
				log_error("Command failed with status code %d.", ret);
			}
			break;
		}
	}

	if (fclose(script))
		log_sys_error("fclose", script_file);

	return ret;
}

static void _nonroot_warning(void)
{
	if (getuid() || geteuid())
		log_warn("WARNING: Running as a non-root user. Functionality may be unavailable.");
}

int lvm2_main(int argc, char **argv)
{
	const char *base;
	int ret, alias = 0;
	struct custom_fds custom_fds;
	struct cmd_context *cmd;
	int run_shell = 0;
	int run_script = 0;
	const char *run_name;
	const char *run_command_name = NULL;

	if (!argv)
		return EINIT_FAILED;

	base = last_path_component(argv[0]);
	if (strcmp(base, "lvm") && strcmp(base, "lvm.static") &&
	    strcmp(base, "initrd-lvm"))
		alias = 1;

	if (!_check_standard_fds())
		return EINIT_FAILED;

	if (!_get_custom_fds(&custom_fds))
		return EINIT_FAILED;

	if (!_close_stray_fds(base, &custom_fds))
		return EINIT_FAILED;

	if (!init_custom_log_streams(&custom_fds))
		return EINIT_FAILED;

	if (is_static() && strcmp(base, "lvm.static") &&
	    path_exists(LVM_PATH) &&
	    !getenv("LVM_DID_EXEC")) {
		if (setenv("LVM_DID_EXEC", base, 1))
			log_sys_error("setenv", "LVM_DID_EXEC");
		if (execvp(LVM_PATH, argv) == -1)
			log_sys_error("execvp", LVM_PATH);
		if (unsetenv("LVM_DID_EXEC"))
			log_sys_error("unsetenv", "LVM_DID_EXEC");
	}

	if (!alias && argc > 1) {
		/* "version" command is simple enough so it doesn't need any complex init */
		if (!strcmp(argv[1], "version"))
			return lvm_return_code(version(NULL, argc, argv));

		/* turn 'lvm -h', 'lvm --help', 'lvm -?' into 'lvm help' */
		if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help") || !strcmp(argv[1], "-?"))
			argv[1] = (char *)"help";

		if (*argv[1] == '-') {
			log_error("Specify options after a command: lvm [command] [options].");
			return EINVALID_CMD_LINE;
		}
	}

	/* turn command -? into command -h and lvm command -? into lvm command -h */
	if (alias && (argc > 1) && !strcmp(argv[1], "-?"))
		argv[1] = (char *)"-h";
	if (!alias && (argc > 2) && !strcmp(argv[2], "-?"))
		argv[2] = (char *)"-h";

	if (!(cmd = init_lvm(0, 0)))
		return EINIT_FAILED;

	/* Store original argv location so we may customise it if we become a daemon */
	cmd->argv = argv;

	/*
	 * If the invocation command name wasn't itself an alias, shift to the
	 * first arg.  After this point, run_name holds one of:
	 *   the LVM command name we want to run;
	 *   the LVM script name (handled through ENO_SUCH_CMD below);
	 *   NULL for a shell (if readline is enabled).
	 */
	if (!alias) {
		argc--;
		argv++;
		run_name = argv[0];
	} else
		run_name = dm_basename(argv[0]);

	/*
	 * Decide if we are running a shell or a command or a script.  When
	 * there is no run_name, it's a shell, when run_name is a recognized
	 * lvm command it's that command, when run_name is not a recognized
	 * command name, try it as an lvm script.
	 */
	if (!run_name)
		run_shell = 1;
	else if (!_find_command_name(run_name))
		run_script = 1;
	else
		run_command_name = run_name;

	/*
	 * NULL run_command_name means register all command defs because
	 * a script or shell needs to access any command name, while a
	 * single command needs to access only defs for the named command.
	 */
	if (!lvm_register_commands(cmd, run_command_name)) {
		ret = ECMD_FAILED;
		goto out;
	}

	if (run_shell) {
#ifdef READLINE_SUPPORT
		_nonroot_warning();
		if (!_prepare_profiles(cmd)) {
			ret = ECMD_FAILED;
			goto out;
		}
		ret = lvm_shell(cmd, &_cmdline);
		goto out;
#else
		log_fatal("Please supply an LVM command.");
		_display_help();
		ret = EINVALID_CMD_LINE;
		goto out;
#endif
	}

	_nonroot_warning();

	if (run_script)
		ret = _run_script(cmd, argc, argv);
	else
		ret = lvm_run_command(cmd, argc, argv);

	if (ret == ENO_SUCH_CMD) {
		log_error("No such command.  Try 'lvm help'.");
		goto out;
	}

	if ((ret != ECMD_PROCESSED) && !error_message_produced()) {
		log_debug(INTERNAL_ERROR "Failed command did not use log_error");
		log_error("Command failed with status code %d.", ret);
	}

      out:
	lvm_fin(cmd);

	return lvm_return_code(ret);
}
