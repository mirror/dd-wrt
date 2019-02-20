/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2017 Red Hat, Inc. All rights reserved.
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
#include "lib/format_text/format-text.h"

#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/utsname.h>

#define report_log_ret_code(ret_code) report_current_object_cmdlog(REPORT_OBJECT_CMDLOG_NAME, \
					((ret_code) == ECMD_PROCESSED) ? REPORT_OBJECT_CMDLOG_SUCCESS \
								   : REPORT_OBJECT_CMDLOG_FAILURE, (ret_code))

struct device_id_list {
	struct dm_list list;
	struct device *dev;
	char pvid[ID_LEN + 1];
};

const char *command_name(struct cmd_context *cmd)
{
	return cmd->command->name;
}

static void _sigchld_handler(int sig __attribute__((unused)))
{
	while (wait4(-1, NULL, WNOHANG | WUNTRACED, NULL) > 0) ;
}

/*
 * returns:
 * -1 if the fork failed
 *  0 if the parent
 *  1 if the child
 */
int become_daemon(struct cmd_context *cmd, int skip_lvm)
{
	static const char devnull[] = "/dev/null";
	int null_fd;
	pid_t pid;
	struct sigaction act = {
		{_sigchld_handler},
		.sa_flags = SA_NOCLDSTOP,
	};

	log_verbose("Forking background process from command: %s", cmd->cmd_line);

	if (sigaction(SIGCHLD, &act, NULL))
		log_warn("WARNING: Failed to set SIGCHLD action.");

	if (!skip_lvm)
		if (!sync_local_dev_names(cmd)) { /* Flush ops and reset dm cookie */
			log_error("Failed to sync local devices before forking.");
			return -1;
		}

	if ((pid = fork()) == -1) {
		log_error("fork failed: %s", strerror(errno));
		return -1;
	}

	/* Parent */
	if (pid > 0)
		return 0;

	/* Child */
	if (setsid() == -1)
		log_error("Background process failed to setsid: %s",
			  strerror(errno));

/* Set this to avoid discarding output from background process */
// #define DEBUG_CHILD

#ifndef DEBUG_CHILD
	if ((null_fd = open(devnull, O_RDWR)) == -1) {
		log_sys_error("open", devnull);
		_exit(ECMD_FAILED);
	}

	if ((dup2(null_fd, STDIN_FILENO) < 0)  || /* reopen stdin */
	    (dup2(null_fd, STDOUT_FILENO) < 0) || /* reopen stdout */
	    (dup2(null_fd, STDERR_FILENO) < 0)) { /* reopen stderr */
		log_sys_error("dup2", "redirect");
		(void) close(null_fd);
		_exit(ECMD_FAILED);
	}

	if (null_fd > STDERR_FILENO)
		(void) close(null_fd);

	init_verbose(VERBOSE_BASE_LEVEL);
#endif	/* DEBUG_CHILD */

	strncpy(*cmd->argv, "(lvm2)", strlen(*cmd->argv));

	if (!skip_lvm) {
		reset_locking();
		lvmcache_destroy(cmd, 1, 1);
		if (!lvmcache_init(cmd))
			/* FIXME Clean up properly here */
			_exit(ECMD_FAILED);
	}

	return 1;
}

/*
 * Strip dev_dir if present
 */
const char *skip_dev_dir(struct cmd_context *cmd, const char *vg_name,
			 unsigned *dev_dir_found)
{
	size_t devdir_len = strlen(cmd->dev_dir);
	const char *dmdir = dm_dir() + devdir_len;
	size_t dmdir_len = strlen(dmdir), vglv_sz;
	char *vgname, *lvname, *layer, *vglv;

	/* FIXME Do this properly */
	if (*vg_name == '/')
		while (vg_name[1] == '/')
			vg_name++;

	if (strncmp(vg_name, cmd->dev_dir, devdir_len)) {
		if (dev_dir_found)
			*dev_dir_found = 0;
	} else {
		if (dev_dir_found)
			*dev_dir_found = 1;

		vg_name += devdir_len;
		while (*vg_name == '/')
			vg_name++;

		/* Reformat string if /dev/mapper found */
		if (!strncmp(vg_name, dmdir, dmdir_len) && vg_name[dmdir_len] == '/') {
			vg_name += dmdir_len + 1;
			while (*vg_name == '/')
				vg_name++;

			if (!dm_split_lvm_name(cmd->mem, vg_name, &vgname, &lvname, &layer) ||
			    *layer) {
				log_error("skip_dev_dir: Couldn't split up device name %s.",
					  vg_name);
				return vg_name;
			}
			vglv_sz = strlen(vgname) + strlen(lvname) + 2;
			if (!(vglv = dm_pool_alloc(cmd->mem, vglv_sz)) ||
			    dm_snprintf(vglv, vglv_sz, "%s%s%s", vgname,
					*lvname ? "/" : "",
					lvname) < 0) {
				log_error("vg/lv string alloc failed.");
				return vg_name;
			}
			return vglv;
		}
	}

	return vg_name;
}

static int _printed_clustered_vg_advice = 0;

/*
 * Three possible results:
 * a) return 0, skip 0: take the VG, and cmd will end in success
 * b) return 0, skip 1: skip the VG, and cmd will end in success
 * c) return 1, skip *: skip the VG, and cmd will end in failure
 *
 * Case b is the special case, and includes the following:
 * . The VG is inconsistent, and the command allows for inconsistent VGs.
 * . The VG is clustered, the host cannot access clustered VG's,
 *   and the command option has been used to ignore clustered vgs.
 *
 * Case c covers the other errors returned when reading the VG.
 *   If *skip is 1, it's OK for the caller to read the list of PVs in the VG.
 */
static int _ignore_vg(struct volume_group *vg, const char *vg_name,
		      struct dm_list *arg_vgnames, uint32_t read_flags,
		      int *skip, int *notfound)
{
	uint32_t read_error = vg_read_error(vg);

	*skip = 0;
	*notfound = 0;

	if ((read_error & FAILED_NOTFOUND) && (read_flags & READ_OK_NOTFOUND)) {
		*notfound = 1;
		return 0;
	}

	if ((read_error & FAILED_INCONSISTENT) && (read_flags & READ_ALLOW_INCONSISTENT))
		read_error &= ~FAILED_INCONSISTENT; /* Check for other errors */

	if (read_error & FAILED_CLUSTERED) {
		if (arg_vgnames && str_list_match_item(arg_vgnames, vg->name)) {
			log_error("Cannot access clustered VG %s.", vg->name);
			if (!_printed_clustered_vg_advice) {
				_printed_clustered_vg_advice = 1;
				log_error("See lvmlockd(8) for changing a clvm/clustered VG to a shared VG.");
			}
			return 1;
		} else {
			log_warn("Skipping clustered VG %s.", vg_name);
			if (!_printed_clustered_vg_advice) {
				_printed_clustered_vg_advice = 1;
				log_error("See lvmlockd(8) for changing a clvm/clustered VG to a shared VG.");
			}
			*skip = 1;
			return 0;
		}
	}

	/*
	 * Commands that operate on "all vgs" shouldn't be bothered by
	 * skipping a foreign VG, and the command shouldn't fail when
	 * one is skipped.  But, if the command explicitly asked to
	 * operate on a foreign VG and it's skipped, then the command
	 * would expect to fail.
	 */
	if (read_error & FAILED_SYSTEMID) {
		if (arg_vgnames && str_list_match_item(arg_vgnames, vg->name)) {
			log_error("Cannot access VG %s with system ID %s with %slocal system ID%s%s.",
				  vg->name, vg->system_id, vg->cmd->system_id ? "" : "unknown ",
				  vg->cmd->system_id ? " " : "", vg->cmd->system_id ? vg->cmd->system_id : "");
			return 1;
		} else {
			read_error &= ~FAILED_SYSTEMID; /* Check for other errors */
			log_verbose("Skipping foreign volume group %s", vg_name);
			*skip = 1;
		}
	}

	/*
	 * Accessing a lockd VG when lvmlockd is not used is similar
	 * to accessing a foreign VG.
	 * This is also the point where a command fails if it failed
	 * to acquire the necessary lock from lvmlockd.
	 * The two cases are distinguished by FAILED_LOCK_TYPE (the
	 * VG lock_type requires lvmlockd), and FAILED_LOCK_MODE (the
	 * command failed to acquire the necessary lock.)
	 */
	if (read_error & (FAILED_LOCK_TYPE | FAILED_LOCK_MODE)) {
		if (arg_vgnames && str_list_match_item(arg_vgnames, vg->name)) {
			if (read_error & FAILED_LOCK_TYPE)
				log_error("Cannot access VG %s with lock type %s that requires lvmlockd.",
					  vg->name, vg->lock_type);
			/* For FAILED_LOCK_MODE, the error is printed in vg_read. */
			return 1;
		} else {
			read_error &= ~FAILED_LOCK_TYPE; /* Check for other errors */
			read_error &= ~FAILED_LOCK_MODE;
			log_verbose("Skipping volume group %s", vg_name);
			*skip = 1;
		}
	}

	if (read_error != SUCCESS) {
		*skip = 0;
		if (is_orphan_vg(vg_name))
			log_error("Cannot process standalone physical volumes");
		else
			log_error("Cannot process volume group %s", vg_name);
		return 1;
	}

	return 0;
}

/*
 * This functiona updates the "selected" arg only if last item processed
 * is selected so this implements the "whole structure is selected if
 * at least one of its items is selected".
 */
static void _update_selection_result(struct processing_handle *handle, int *selected)
{
	if (!handle || !handle->selection_handle)
		return;

	if (handle->selection_handle->selected)
		*selected = 1;
}

static void _set_final_selection_result(struct processing_handle *handle, int selected)
{
	if (!handle || !handle->selection_handle)
		return;

	handle->selection_handle->selected = selected;
}

/*
 * Metadata iteration functions
 */
int process_each_segment_in_pv(struct cmd_context *cmd,
			       struct volume_group *vg,
			       struct physical_volume *pv,
			       struct processing_handle *handle,
			       process_single_pvseg_fn_t process_single_pvseg)
{
	struct pv_segment *pvseg;
	int whole_selected = 0;
	int ret_max = ECMD_PROCESSED;
	int ret;
	struct pv_segment _free_pv_segment = { .pv = pv };

	if (dm_list_empty(&pv->segments)) {
		ret = process_single_pvseg(cmd, NULL, &_free_pv_segment, handle);
		if (ret != ECMD_PROCESSED)
			stack;
		if (ret > ret_max)
			ret_max = ret;
	} else {
		dm_list_iterate_items(pvseg, &pv->segments) {
			if (sigint_caught())
				return_ECMD_FAILED;

			ret = process_single_pvseg(cmd, vg, pvseg, handle);
			_update_selection_result(handle, &whole_selected);
			if (ret != ECMD_PROCESSED)
				stack;
			if (ret > ret_max)
				ret_max = ret;
		}
	}

	/* the PV is selected if at least one PV segment is selected */
	_set_final_selection_result(handle, whole_selected);
	return ret_max;
}

int process_each_segment_in_lv(struct cmd_context *cmd,
			       struct logical_volume *lv,
			       struct processing_handle *handle,
			       process_single_seg_fn_t process_single_seg)
{
	struct lv_segment *seg;
	int whole_selected = 0;
	int ret_max = ECMD_PROCESSED;
	int ret;

	dm_list_iterate_items(seg, &lv->segments) {
		if (sigint_caught())
			return_ECMD_FAILED;

		ret = process_single_seg(cmd, seg, handle);
		_update_selection_result(handle, &whole_selected);
		if (ret != ECMD_PROCESSED)
			stack;
		if (ret > ret_max)
			ret_max = ret;
	}

	/* the LV is selected if at least one LV segment is selected */
	_set_final_selection_result(handle, whole_selected);
	return ret_max;
}

static const char *_extract_vgname(struct cmd_context *cmd, const char *lv_name,
				   const char **after)
{
	const char *vg_name = lv_name;
	char *st, *pos;

	/* Strip dev_dir (optional) */
	if (!(vg_name = skip_dev_dir(cmd, vg_name, NULL)))
		return_0;

	/* Require exactly one set of consecutive slashes */
	if ((st = pos = strchr(vg_name, '/')))
		while (*st == '/')
			st++;

	if (!st || strchr(st, '/')) {
		log_error("\"%s\": Invalid path for Logical Volume.",
			  lv_name);
		return 0;
	}

	if (!(vg_name = dm_pool_strndup(cmd->mem, vg_name, pos - vg_name))) {
		log_error("Allocation of vg_name failed.");
		return 0;
	}

	if (after)
		*after = st;

	return vg_name;
}

/*
 * Extract default volume group name from environment
 */
static const char *_default_vgname(struct cmd_context *cmd)
{
	const char *vg_path;

	/* Take default VG from environment? */
	vg_path = getenv("LVM_VG_NAME");
	if (!vg_path)
		return 0;

	vg_path = skip_dev_dir(cmd, vg_path, NULL);

	if (strchr(vg_path, '/')) {
		log_error("\"%s\": Invalid environment var LVM_VG_NAME set for Volume Group.",
			  vg_path);
		return 0;
	}

	return dm_pool_strdup(cmd->mem, vg_path);
}

/*
 * Determine volume group name from a logical volume name
 */
const char *extract_vgname(struct cmd_context *cmd, const char *lv_name)
{
	const char *vg_name = lv_name;

	/* Path supplied? */
	if (vg_name && strchr(vg_name, '/')) {
		if (!(vg_name = _extract_vgname(cmd, lv_name, NULL)))
			return_NULL;

		return vg_name;
	}

	if (!(vg_name = _default_vgname(cmd))) {
		if (lv_name)
			log_error("Path required for Logical Volume \"%s\".",
				  lv_name);
		return NULL;
	}

	return vg_name;
}

/*
 * Process physical extent range specifiers
 */
static int _add_pe_range(struct dm_pool *mem, const char *pvname,
			 struct dm_list *pe_ranges, uint32_t start, uint32_t count)
{
	struct pe_range *per;

	log_debug("Adding PE range: start PE " FMTu32 " length " FMTu32 " on %s.",
		  start, count, pvname);

	/* Ensure no overlap with existing areas */
	dm_list_iterate_items(per, pe_ranges) {
		if (((start < per->start) && (start + count - 1 >= per->start)) ||
		    ((start >= per->start) &&
			(per->start + per->count - 1) >= start)) {
			log_error("Overlapping PE ranges specified (" FMTu32
				  "-" FMTu32 ", " FMTu32 "-" FMTu32 ") on %s.",
				  start, start + count - 1, per->start,
				  per->start + per->count - 1, pvname);
			return 0;
		}
	}

	if (!(per = dm_pool_alloc(mem, sizeof(*per)))) {
		log_error("Allocation of list failed.");
		return 0;
	}

	per->start = start;
	per->count = count;
	dm_list_add(pe_ranges, &per->list);

	return 1;
}

static int _xstrtouint32(const char *s, char **p, int base, uint32_t *result)
{
	unsigned long ul;

	errno = 0;
	ul = strtoul(s, p, base);

	if (errno || *p == s || ul > UINT32_MAX)
		return 0;

	*result = ul;

	return 1;
}

static int _parse_pes(struct dm_pool *mem, char *c, struct dm_list *pe_ranges,
		      const char *pvname, uint32_t size)
{
	char *endptr;
	uint32_t start, end, len;

	/* Default to whole PV */
	if (!c) {
		if (!_add_pe_range(mem, pvname, pe_ranges, UINT32_C(0), size))
			return_0;
		return 1;
	}

	while (*c) {
		if (*c != ':')
			goto error;

		c++;

		/* Disallow :: and :\0 */
		if (*c == ':' || !*c)
			goto error;

		/* Default to whole range */
		start = UINT32_C(0);
		end = size - 1;

		/* Start extent given? */
		if (isdigit(*c)) {
			if (!_xstrtouint32(c, &endptr, 10, &start))
				goto error;
			c = endptr;
			/* Just one number given? */
			if (!*c || *c == ':')
				end = start;
		}
		/* Range? */
		if (*c == '-') {
			c++;
			if (isdigit(*c)) {
				if (!_xstrtouint32(c, &endptr, 10, &end))
					goto error;
				c = endptr;
			}
		} else if (*c == '+') {	/* Length? */
			c++;
			if (isdigit(*c)) {
				if (!_xstrtouint32(c, &endptr, 10, &len))
					goto error;
				c = endptr;
				end = start + (len ? (len - 1) : 0);
			}
		}

		if (*c && *c != ':')
			goto error;

		if ((start > end) || (end > size - 1)) {
			log_error("PE range error: start extent %" PRIu32 " to "
				  "end extent %" PRIu32 ".", start, end);
			return 0;
		}

		if (!_add_pe_range(mem, pvname, pe_ranges, start, end - start + 1))
			return_0;

	}

	return 1;

      error:
	log_error("Physical extent parsing error at %s.", c);
	return 0;
}

static int _create_pv_entry(struct dm_pool *mem, struct pv_list *pvl,
			     char *colon, int allocatable_only, struct dm_list *r)
{
	const char *pvname;
	struct pv_list *new_pvl = NULL, *pvl2;
	struct dm_list *pe_ranges;

	pvname = pv_dev_name(pvl->pv);
	if (allocatable_only && !(pvl->pv->status & ALLOCATABLE_PV)) {
		log_warn("WARNING: Physical volume %s not allocatable.", pvname);
		return 1;
	}

	if (allocatable_only && is_missing_pv(pvl->pv)) {
		log_warn("WARNING: Physical volume %s is missing.", pvname);
		return 1;
	}

	if (allocatable_only &&
	    (pvl->pv->pe_count == pvl->pv->pe_alloc_count)) {
		log_warn("WARNING: No free extents on physical volume \"%s\".", pvname);
		return 1;
	}

	dm_list_iterate_items(pvl2, r)
		if (pvl->pv->dev == pvl2->pv->dev) {
			new_pvl = pvl2;
			break;
		}

	if (!new_pvl) {
		if (!(new_pvl = dm_pool_alloc(mem, sizeof(*new_pvl)))) {
			log_error("Unable to allocate physical volume list.");
			return 0;
		}

		memcpy(new_pvl, pvl, sizeof(*new_pvl));

		if (!(pe_ranges = dm_pool_alloc(mem, sizeof(*pe_ranges)))) {
			log_error("Allocation of pe_ranges list failed.");
			return 0;
		}
		dm_list_init(pe_ranges);
		new_pvl->pe_ranges = pe_ranges;
		dm_list_add(r, &new_pvl->list);
	}

	/* Determine selected physical extents */
	if (!_parse_pes(mem, colon, new_pvl->pe_ranges, pv_dev_name(pvl->pv),
			pvl->pv->pe_count))
		return_0;

	return 1;
}

struct dm_list *create_pv_list(struct dm_pool *mem, struct volume_group *vg, int argc,
			    char **argv, int allocatable_only)
{
	struct dm_list *r;
	struct pv_list *pvl;
	struct dm_list tagsl, arg_pvnames;
	char *pvname = NULL;
	char *colon, *at_sign, *tagname;
	int i;

	/* Build up list of PVs */
	if (!(r = dm_pool_alloc(mem, sizeof(*r)))) {
		log_error("Allocation of list failed.");
		return NULL;
	}
	dm_list_init(r);

	dm_list_init(&tagsl);
	dm_list_init(&arg_pvnames);

	for (i = 0; i < argc; i++) {
		dm_unescape_colons_and_at_signs(argv[i], &colon, &at_sign);

		if (at_sign && (at_sign == argv[i])) {
			tagname = at_sign + 1;
			if (!validate_tag(tagname)) {
				log_error("Skipping invalid tag %s.", tagname);
				continue;
			}
			dm_list_iterate_items(pvl, &vg->pvs) {
				if (str_list_match_item(&pvl->pv->tags,
							tagname)) {
					if (!_create_pv_entry(mem, pvl, NULL,
							      allocatable_only,
							      r))
						return_NULL;
				}
			}
			continue;
		}

		pvname = argv[i];

		if (colon && !(pvname = dm_pool_strndup(mem, pvname,
					(unsigned) (colon - pvname)))) {
			log_error("Failed to clone PV name.");
			return NULL;
		}

		if (!(pvl = find_pv_in_vg(vg, pvname))) {
			log_error("Physical Volume \"%s\" not found in "
				  "Volume Group \"%s\".", pvname, vg->name);
			return NULL;
		}
		if (!_create_pv_entry(mem, pvl, colon, allocatable_only, r))
			return_NULL;
	}

	if (dm_list_empty(r))
		log_error("No specified PVs have space available.");

	return dm_list_empty(r) ? NULL : r;
}

struct dm_list *clone_pv_list(struct dm_pool *mem, struct dm_list *pvsl)
{
	struct dm_list *r;
	struct pv_list *pvl, *new_pvl;

	/* Build up list of PVs */
	if (!(r = dm_pool_alloc(mem, sizeof(*r)))) {
		log_error("Allocation of list failed.");
		return NULL;
	}
	dm_list_init(r);

	dm_list_iterate_items(pvl, pvsl) {
		if (!(new_pvl = dm_pool_zalloc(mem, sizeof(*new_pvl)))) {
			log_error("Unable to allocate physical volume list.");
			return NULL;
		}

		memcpy(new_pvl, pvl, sizeof(*new_pvl));
		dm_list_add(r, &new_pvl->list);
	}

	return r;
}

const char _pe_size_may_not_be_negative_msg[] = "Physical extent size may not be negative.";

int vgcreate_params_set_defaults(struct cmd_context *cmd,
				 struct vgcreate_params *vp_def,
				 struct volume_group *vg)
{
	int64_t extent_size;

	/* Only vgsplit sets vg */
	if (vg) {
		vp_def->vg_name = NULL;
		vp_def->extent_size = vg->extent_size;
		vp_def->max_pv = vg->max_pv;
		vp_def->max_lv = vg->max_lv;
		vp_def->alloc = vg->alloc;
		vp_def->vgmetadatacopies = vg->mda_copies;
		vp_def->system_id = vg->system_id;	/* No need to clone this */
	} else {
		vp_def->vg_name = NULL;
		extent_size = find_config_tree_int64(cmd,
				allocation_physical_extent_size_CFG, NULL) * 2;
		if (extent_size < 0) {
			log_error(_pe_size_may_not_be_negative_msg);
			return 0;
		}
		vp_def->extent_size = (uint32_t) extent_size;
		vp_def->max_pv = DEFAULT_MAX_PV;
		vp_def->max_lv = DEFAULT_MAX_LV;
		vp_def->alloc = DEFAULT_ALLOC_POLICY;
		vp_def->vgmetadatacopies = DEFAULT_VGMETADATACOPIES;
		vp_def->system_id = cmd->system_id;
	}

	return 1;
}

/*
 * Set members of struct vgcreate_params from cmdline arguments.
 * Do preliminary validation with arg_*() interface.
 * Further, more generic validation is done in validate_vgcreate_params().
 * This function is to remain in tools directory.
 */
int vgcreate_params_set_from_args(struct cmd_context *cmd,
				  struct vgcreate_params *vp_new,
				  struct vgcreate_params *vp_def)
{
	const char *system_id_arg_str;
	const char *lock_type = NULL;
	int use_lvmlockd;
	lock_type_t lock_type_num;

	if (arg_is_set(cmd, clustered_ARG)) {
		log_error("The clustered option is deprecated, see --shared.");
		return 0;
	}

	vp_new->vg_name = skip_dev_dir(cmd, vp_def->vg_name, NULL);
	vp_new->max_lv = arg_uint_value(cmd, maxlogicalvolumes_ARG,
					vp_def->max_lv);
	vp_new->max_pv = arg_uint_value(cmd, maxphysicalvolumes_ARG,
					vp_def->max_pv);
	vp_new->alloc = (alloc_policy_t) arg_uint_value(cmd, alloc_ARG, vp_def->alloc);

	/* Units of 512-byte sectors */
	vp_new->extent_size =
	    arg_uint_value(cmd, physicalextentsize_ARG, vp_def->extent_size);

	if (arg_sign_value(cmd, physicalextentsize_ARG, SIGN_NONE) == SIGN_MINUS) {
		log_error(_pe_size_may_not_be_negative_msg);
		return 0;
	}

	if (arg_uint64_value(cmd, physicalextentsize_ARG, 0) > MAX_EXTENT_SIZE) {
		log_error("Physical extent size must be smaller than %s.",
				  display_size(cmd, (uint64_t) MAX_EXTENT_SIZE));
		return 0;
	}

	if (arg_sign_value(cmd, maxlogicalvolumes_ARG, SIGN_NONE) == SIGN_MINUS) {
		log_error("Max Logical Volumes may not be negative.");
		return 0;
	}

	if (arg_sign_value(cmd, maxphysicalvolumes_ARG, SIGN_NONE) == SIGN_MINUS) {
		log_error("Max Physical Volumes may not be negative.");
		return 0;
	}

	if (arg_is_set(cmd, vgmetadatacopies_ARG))
		vp_new->vgmetadatacopies = arg_int_value(cmd, vgmetadatacopies_ARG,
							DEFAULT_VGMETADATACOPIES);
	else
		vp_new->vgmetadatacopies = find_config_tree_int(cmd, metadata_vgmetadatacopies_CFG, NULL);

	if (!(system_id_arg_str = arg_str_value(cmd, systemid_ARG, NULL))) {
		vp_new->system_id = vp_def->system_id;
	} else {
		if (!(vp_new->system_id = system_id_from_string(cmd, system_id_arg_str)))
			return_0;

		/* FIXME Take local/extra_system_ids into account */
		if (vp_new->system_id && cmd->system_id &&
		    strcmp(vp_new->system_id, cmd->system_id)) {
			if (*vp_new->system_id)
				log_warn("VG with system ID %s might become inaccessible as local system ID is %s",
					 vp_new->system_id, cmd->system_id);
			else
				log_warn("WARNING: A VG without a system ID allows unsafe access from other hosts.");
		}
	}

	if ((system_id_arg_str = arg_str_value(cmd, systemid_ARG, NULL))) {
		vp_new->system_id = system_id_from_string(cmd, system_id_arg_str);
	} else {
		vp_new->system_id = vp_def->system_id;
	}

	if (system_id_arg_str) {
		if (!vp_new->system_id || !vp_new->system_id[0])
			log_warn("WARNING: A VG without a system ID allows unsafe access from other hosts.");

		if (vp_new->system_id && cmd->system_id &&
		    strcmp(vp_new->system_id, cmd->system_id)) {
			log_warn("VG with system ID %s might become inaccessible as local system ID is %s",
				 vp_new->system_id, cmd->system_id);
		}
	}

	/*
	 * Locking: what kind of locking should be used for the
	 * new VG, and is it compatible with current lvm.conf settings.
	 *
	 * The end result is to set vp_new->lock_type to:
	 * none | clvm | dlm | sanlock.
	 *
	 * If 'vgcreate --lock-type <arg>' is set, the answer is given
	 * directly by <arg> which is one of none|clvm|dlm|sanlock.
	 *
	 * 'vgcreate --clustered y' is the way to create clvm VGs.
	 *
	 * 'vgcreate --shared' is the way to create lockd VGs.
	 * lock_type of sanlock or dlm is selected based on
	 * which lock manager is running.
	 *
	 *
	 * 1. Using neither clvmd nor lvmlockd.
	 * ------------------------------------------------
	 * lvm.conf:
	 * global/use_lvmlockd = 0
	 * global/locking_type = 1
	 *
	 * - no locking is enabled
	 * - clvmd is not used
	 * - lvmlockd is not used
	 * - VGs with CLUSTERED set are ignored (requires clvmd)
	 * - VGs with lockd type are ignored (requires lvmlockd)
	 * - vgcreate can create new VGs with lock_type none
	 * - 'vgcreate --clustered y' fails
	 * - 'vgcreate --shared' fails
	 * - 'vgcreate' (neither option) creates a local VG
	 *
	 * 2. Using clvmd.
	 * ------------------------------------------------
	 * lvm.conf:
	 * global/use_lvmlockd = 0
	 * global/locking_type = 3
	 *
	 * - locking through clvmd is enabled (traditional clvm config)
	 * - clvmd is used
	 * - lvmlockd is not used
	 * - VGs with CLUSTERED set can be used
	 * - VGs with lockd type are ignored (requires lvmlockd)
	 * - vgcreate can create new VGs with CLUSTERED status flag
	 * - 'vgcreate --clustered y' works
	 * - 'vgcreate --shared' fails
	 * - 'vgcreate' (neither option) creates a clvm VG
	 *
	 * 3. Using lvmlockd.
	 * ------------------------------------------------
	 * lvm.conf:
	 * global/use_lvmlockd = 1
	 * global/locking_type = 1
	 *
	 * - locking through lvmlockd is enabled
	 * - clvmd is not used
	 * - lvmlockd is used
	 * - VGs with CLUSTERED set are ignored (requires clvmd)
	 * - VGs with lockd type can be used
	 * - vgcreate can create new VGs with lock_type sanlock or dlm
	 * - 'vgcreate --clustered y' fails
	 * - 'vgcreate --shared' works
	 * - 'vgcreate' (neither option) creates a local VG
	 */

	use_lvmlockd = find_config_tree_bool(cmd, global_use_lvmlockd_CFG, NULL);

	if (arg_is_set(cmd, locktype_ARG)) {
		lock_type = arg_str_value(cmd, locktype_ARG, "");

		if (arg_is_set(cmd, shared_ARG) && !is_lockd_type(lock_type)) {
			log_error("The --shared option requires lock type sanlock or dlm.");
			return 0;
		}

	} else if (arg_is_set(cmd, shared_ARG)) {
		int found_multiple = 0;

		if (use_lvmlockd) {
			if (!(lock_type = lockd_running_lock_type(cmd, &found_multiple))) {
				if (found_multiple)
					log_error("Found multiple lock managers, select one with --lock-type.");
				else
					log_error("Failed to detect a running lock manager to select lock type.");
				return 0;
			}

		} else {
			log_error("Using a shared lock type requires lvmlockd.");
			return 0;
		}

	} else {
		lock_type = "none";
	}

	/*
	 * Check that the lock_type is recognized, and is being
	 * used with the correct lvm.conf settings.
	 */
	lock_type_num = get_lock_type_from_string(lock_type);

	switch (lock_type_num) {
	case LOCK_TYPE_INVALID:
	case LOCK_TYPE_CLVM:
		log_error("lock_type %s is invalid", lock_type);
		return 0;

	case LOCK_TYPE_SANLOCK:
	case LOCK_TYPE_DLM:
		if (!use_lvmlockd) {
			log_error("Using a shared lock type requires lvmlockd.");
			return 0;
		}
		break;
	case LOCK_TYPE_NONE:
		break;
	};

	/*
	 * The vg is not owned by one host/system_id.
	 * Locking coordinates access from multiple hosts.
	 */
	if (lock_type_num == LOCK_TYPE_DLM || lock_type_num == LOCK_TYPE_SANLOCK)
		vp_new->system_id = NULL;

	vp_new->lock_type = lock_type;

	log_debug("Setting lock_type to %s", vp_new->lock_type);
	return 1;
}

/* Shared code for changing activation state for vgchange/lvchange */
int lv_change_activate(struct cmd_context *cmd, struct logical_volume *lv,
		       activation_change_t activate)
{
	int r = 1;
	struct logical_volume *snapshot_lv;

	if (lv_is_cache_pool(lv)) {
		if (is_change_activating(activate)) {
			log_verbose("Skipping activation of cache pool %s.",
				    display_lvname(lv));
			return 1;
		}
		if (!dm_list_empty(&lv->segs_using_this_lv)) {
			log_verbose("Skipping deactivation of used cache pool %s.",
				    display_lvname(lv));
			return 1;
		}
		/*
		 * Allow to pass only deactivation of unused cache pool.
		 * Useful only for recovery of failed zeroing of metadata LV.
		 */
	}

	if (lv_is_merging_origin(lv)) {
		/*
		 * For merging origin, its snapshot must be inactive.
		 * If it's still active and cannot be deactivated
		 * activation or deactivation of origin fails!
		 *
		 * When origin is deactivated and merging snapshot is thin
		 * it allows to deactivate origin, but still report error,
		 * since the thin snapshot remains active.
		 *
		 * User could retry to deactivate it with another
		 * deactivation of origin, which is the only visible LV
		 */
		snapshot_lv = find_snapshot(lv)->lv;
		if (lv_is_thin_type(snapshot_lv) && !deactivate_lv(cmd, snapshot_lv)) {
			if (is_change_activating(activate)) {
				log_error("Refusing to activate merging volume %s while "
					  "snapshot volume %s is still active.",
					  display_lvname(lv), display_lvname(snapshot_lv));
				return 0;
			}

			log_error("Cannot fully deactivate merging origin volume %s while "
				  "snapshot volume %s is still active.",
				  display_lvname(lv), display_lvname(snapshot_lv));
			r = 0; /* and continue to deactivate origin... */
		}
	}

	if (is_change_activating(activate) &&
	    lvmcache_found_duplicate_pvs() &&
	    vg_has_duplicate_pvs(lv->vg) &&
	    !find_config_tree_bool(cmd, devices_allow_changes_with_duplicate_pvs_CFG, NULL)) {
		log_error("Cannot activate LVs in VG %s while PVs appear on duplicate devices.",
			  lv->vg->name);
		return 0;
	}

	if (!lv_active_change(cmd, lv, activate))
		return_0;

	set_lv_notify(lv->vg->cmd);

	return r;
}

int lv_refresh(struct cmd_context *cmd, struct logical_volume *lv)
{
	struct logical_volume *snapshot_lv;

	if (lv_is_merging_origin(lv)) {
		snapshot_lv = find_snapshot(lv)->lv;
		if (lv_is_thin_type(snapshot_lv) && !deactivate_lv(cmd, snapshot_lv))
			log_print_unless_silent("Delaying merge for origin volume %s since "
						"snapshot volume %s is still active.",
						display_lvname(lv), display_lvname(snapshot_lv));
	}

	if (!lv_refresh_suspend_resume(lv))
		return_0;

	/*
	 * check if snapshot merge should be polled
	 * - unfortunately: even though the dev_manager will clear
	 *   the lv's merge attributes if a merge is not possible;
	 *   it is clearing a different instance of the lv (as
	 *   retrieved with lv_from_lvid)
	 * - fortunately: polldaemon will immediately shutdown if the
	 *   origin doesn't have a status with a snapshot percentage
	 */
	if (background_polling() && lv_is_merging_origin(lv) && lv_is_active(lv))
		lv_spawn_background_polling(cmd, lv);

	return 1;
}

int vg_refresh_visible(struct cmd_context *cmd, struct volume_group *vg)
{
	struct lv_list *lvl;
	int r = 1;

	sigint_allow();
	dm_list_iterate_items(lvl, &vg->lvs) {
		if (sigint_caught()) {
			r = 0;
			stack;
			break;
		}

		if (lv_is_visible(lvl->lv) && !lv_refresh(cmd, lvl->lv)) {
			r = 0;
			stack;
		}
	}

	sigint_restore();

	return r;
}

void lv_spawn_background_polling(struct cmd_context *cmd,
				 struct logical_volume *lv)
{
	const char *pvname;
	const struct logical_volume *lv_mirr = NULL;

	if (lv_is_pvmove(lv))
		lv_mirr = lv;
	else if (lv_is_locked(lv))
		lv_mirr = find_pvmove_lv_in_lv(lv);

	if (lv_mirr &&
	    (pvname = get_pvmove_pvname_from_lv_mirr(lv_mirr))) {
		log_verbose("Spawning background pvmove process for %s.",
			    pvname);
		pvmove_poll(cmd, pvname, lv_mirr->lvid.s, lv_mirr->vg->name, lv_mirr->name, 1);
	}

	if (lv_is_converting(lv) || lv_is_merging(lv)) {
		log_verbose("Spawning background lvconvert process for %s.",
			    lv->name);
		lvconvert_poll(cmd, lv, 1);
	}
}

int get_activation_monitoring_mode(struct cmd_context *cmd,
				   int *monitoring_mode)
{
	*monitoring_mode = DEFAULT_DMEVENTD_MONITOR;

	if (arg_is_set(cmd, monitor_ARG) &&
	    (arg_is_set(cmd, ignoremonitoring_ARG) ||
	     arg_is_set(cmd, sysinit_ARG))) {
		log_error("--ignoremonitoring or --sysinit option not allowed with --monitor option.");
		return 0;
	}

	if (arg_is_set(cmd, monitor_ARG))
		*monitoring_mode = arg_int_value(cmd, monitor_ARG,
						 DEFAULT_DMEVENTD_MONITOR);
	else if (is_static() || arg_is_set(cmd, ignoremonitoring_ARG) ||
		 arg_is_set(cmd, sysinit_ARG) ||
		 !find_config_tree_bool(cmd, activation_monitoring_CFG, NULL))
		*monitoring_mode = DMEVENTD_MONITOR_IGNORE;

	return 1;
}

/*
 * Read pool options from cmdline
 */
int get_pool_params(struct cmd_context *cmd,
		    const struct segment_type *segtype,
		    uint64_t *pool_metadata_size,
		    int *pool_metadata_spare,
		    uint32_t *chunk_size,
		    thin_discards_t *discards,
		    thin_zero_t *zero_new_blocks)
{
	if (segtype_is_thin_pool(segtype) || segtype_is_thin(segtype)) {
		if (arg_is_set(cmd, zero_ARG)) {
			*zero_new_blocks = arg_int_value(cmd, zero_ARG, 0) ? THIN_ZERO_YES : THIN_ZERO_NO;
			log_very_verbose("%s pool zeroing.",
					 (*zero_new_blocks == THIN_ZERO_YES) ? "Enabling" : "Disabling");
		} else
			*zero_new_blocks = THIN_ZERO_UNSELECTED;

		if (arg_is_set(cmd, discards_ARG)) {
			*discards = (thin_discards_t) arg_uint_value(cmd, discards_ARG, 0);
			log_very_verbose("Setting pool discards to %s.",
					 get_pool_discards_name(*discards));
		} else
			*discards = THIN_DISCARDS_UNSELECTED;
	}

	if (arg_from_list_is_negative(cmd, "may not be negative",
				      chunksize_ARG,
				      pooldatasize_ARG,
				      poolmetadatasize_ARG,
				      -1))
		return_0;

	if (arg_from_list_is_zero(cmd, "may not be zero",
				  chunksize_ARG,
				  pooldatasize_ARG,
				  poolmetadatasize_ARG,
				  -1))
		return_0;

	if (arg_is_set(cmd, chunksize_ARG)) {
		*chunk_size = arg_uint_value(cmd, chunksize_ARG, 0);

		if (!validate_pool_chunk_size(cmd, segtype, *chunk_size))
			return_0;

		log_very_verbose("Setting pool chunk size to %s.",
				 display_size(cmd, *chunk_size));
	} else
		*chunk_size = 0;

	if (arg_is_set(cmd, poolmetadatasize_ARG)) {
		if (arg_is_set(cmd, poolmetadata_ARG)) {
			log_error("Please specify either metadata logical volume or its size.");
			return 0;
		}

		*pool_metadata_size = arg_uint64_value(cmd, poolmetadatasize_ARG,
						       UINT64_C(0));
	} else
		*pool_metadata_size = 0;


	/* TODO: default in lvm.conf and metadata profile ? */
	*pool_metadata_spare = arg_int_value(cmd, poolmetadataspare_ARG,
					     DEFAULT_POOL_METADATA_SPARE);

	return 1;
}

/*
 * Generic stripe parameter checks.
 */
static int _validate_stripe_params(struct cmd_context *cmd, const struct segment_type *segtype,
				   uint32_t *stripes, uint32_t *stripe_size)
{
	if (*stripes < 1 || *stripes > MAX_STRIPES) {
		log_error("Number of stripes (%d) must be between %d and %d.",
			  *stripes, 1, MAX_STRIPES);
		return 0;
	}

	if (!segtype_supports_stripe_size(segtype)) {
		if (*stripe_size) {
			log_print_unless_silent("Ignoring stripesize argument for %s devices.",
						segtype->name);
			*stripe_size = 0;
		}
	} else if (*stripes == 1) {
		if (*stripe_size) {
			log_print_unless_silent("Ignoring stripesize argument with single stripe.");
			*stripe_size = 0;
		}
	} else {
		if (!*stripe_size) {
			*stripe_size = find_config_tree_int(cmd, metadata_stripesize_CFG, NULL) * 2;
			log_print_unless_silent("Using default stripesize %s.",
						display_size(cmd, (uint64_t) *stripe_size));
		}

		if (*stripe_size > STRIPE_SIZE_LIMIT * 2) {
			log_error("Stripe size cannot be larger than %s.",
				  display_size(cmd, (uint64_t) STRIPE_SIZE_LIMIT));
			return 0;
		} else if (*stripe_size < STRIPE_SIZE_MIN || !is_power_of_2(*stripe_size)) {
			log_error("Invalid stripe size %s.",
				  display_size(cmd, (uint64_t) *stripe_size));
			return 0;
		}
	}

	return 1;
}

/*
 * The stripe size is limited by the size of a uint32_t, but since the
 * value given by the user is doubled, and the final result must be a
 * power of 2, we must divide UINT_MAX by four and add 1 (to round it
 * up to the power of 2)
 */
int get_stripe_params(struct cmd_context *cmd, const struct segment_type *segtype,
		      uint32_t *stripes, uint32_t *stripe_size,
		      unsigned *stripes_supplied, unsigned *stripe_size_supplied)
{
	/* stripes_long_ARG takes precedence (for lvconvert) */
	/* FIXME Cope with relative +/- changes for lvconvert. */
	if (arg_is_set(cmd, stripes_long_ARG)) {
		*stripes = arg_uint_value(cmd, stripes_long_ARG, 0);
		*stripes_supplied = 1;
	} else if (arg_is_set(cmd, stripes_ARG)) {
		*stripes = arg_uint_value(cmd, stripes_ARG, 0);
		*stripes_supplied = 1;
	} else {
		/*
		 * FIXME add segtype parameter for min_stripes and remove logic for this
		 *       from all other places
		 */
		if (segtype_is_any_raid6(segtype))
			*stripes = 3;
		else if (segtype_is_striped_raid(segtype))
			*stripes = 2;
		else
			*stripes = 1;
		*stripes_supplied = 0;
	}

	if ((*stripe_size = arg_uint_value(cmd, stripesize_ARG, 0))) {
		if (arg_sign_value(cmd, stripesize_ARG, SIGN_NONE) == SIGN_MINUS) {
			log_error("Negative stripesize is invalid.");
			return 0;
		}
	}
	*stripe_size_supplied = arg_is_set(cmd, stripesize_ARG);

	return _validate_stripe_params(cmd, segtype, stripes, stripe_size);
}

static int _validate_cachepool_params(const char *policy_name, cache_mode_t cache_mode)
{
	/*
	 * FIXME: it might be nice if cmd def rules could check option values,
	 * then a rule could do this.
	 */
	if ((cache_mode == CACHE_MODE_WRITEBACK) && policy_name && !strcmp(policy_name, "cleaner")) {
		log_error("Cache mode \"writeback\" is not compatible with cache policy \"cleaner\".");
		return 0;
	}

	return 1;
}

int get_cache_params(struct cmd_context *cmd,
		     uint32_t *chunk_size,
		     cache_metadata_format_t *cache_metadata_format,
		     cache_mode_t *cache_mode,
		     const char **name,
		     struct dm_config_tree **settings)
{
	const char *str;
	struct arg_value_group_list *group;
	struct dm_config_tree *result = NULL, *prev = NULL, *current = NULL;
	struct dm_config_node *cn;
	int ok = 0;

	if (arg_is_set(cmd, chunksize_ARG)) {
		*chunk_size = arg_uint_value(cmd, chunksize_ARG, 0);

		if (!validate_cache_chunk_size(cmd, *chunk_size))
			return_0;

		log_very_verbose("Setting pool chunk size to %s.",
				 display_size(cmd, *chunk_size));
	}

	*cache_metadata_format = (cache_metadata_format_t)
		arg_uint_value(cmd, cachemetadataformat_ARG, CACHE_METADATA_FORMAT_UNSELECTED);

	*cache_mode = (cache_mode_t) arg_uint_value(cmd, cachemode_ARG, CACHE_MODE_UNSELECTED);

	*name = arg_str_value(cmd, cachepolicy_ARG, NULL);

	if (!_validate_cachepool_params(*name, *cache_mode))
		goto_out;

	dm_list_iterate_items(group, &cmd->arg_value_groups) {
		if (!grouped_arg_is_set(group->arg_values, cachesettings_ARG))
			continue;

		if (!(current = dm_config_create()))
			goto_out;
		if (prev)
			current->cascade = prev;
		prev = current;

		if (!(str = grouped_arg_str_value(group->arg_values,
						  cachesettings_ARG,
						  NULL)))
			goto_out;

		if (!dm_config_parse_without_dup_node_check(current, str, str + strlen(str)))
			goto_out;
	}

	if (current) {
		if (!(result = dm_config_flatten(current)))
			goto_out;

		if (result->root) {
			if (!(cn = dm_config_create_node(result, "policy_settings")))
				goto_out;

			cn->child = result->root;
			result->root = cn;
		}
	}

	ok = 1;
out:
	if (!ok && result) {
		dm_config_destroy(result);
		result = NULL;
	}
	while (prev) {
		current = prev->cascade;
		dm_config_destroy(prev);
		prev = current;
	}

	*settings = result;

	return ok;
}

/* FIXME move to lib */
static int _pv_change_tag(struct physical_volume *pv, const char *tag, int addtag)
{
	if (addtag) {
		if (!str_list_add(pv->fmt->cmd->mem, &pv->tags, tag)) {
			log_error("Failed to add tag %s to physical volume %s.",
				  tag, pv_dev_name(pv));
			return 0;
		}
	} else
		str_list_del(&pv->tags, tag);

	return 1;
}

/* Set exactly one of VG, LV or PV */
int change_tag(struct cmd_context *cmd, struct volume_group *vg,
	       struct logical_volume *lv, struct physical_volume *pv, int arg)
{
	const char *tag;
	struct arg_value_group_list *current_group;

	dm_list_iterate_items(current_group, &cmd->arg_value_groups) {
		if (!grouped_arg_is_set(current_group->arg_values, arg))
			continue;

		if (!(tag = grouped_arg_str_value(current_group->arg_values, arg, NULL))) {
			log_error("Failed to get tag.");
			return 0;
		}

		if (vg && !vg_change_tag(vg, tag, arg == addtag_ARG))
			return_0;
		else if (lv && !lv_change_tag(lv, tag, arg == addtag_ARG))
			return_0;
		else if (pv && !_pv_change_tag(pv, tag, arg == addtag_ARG))
			return_0;
	}

	return 1;
}

/*
 * FIXME: replace process_each_label() with process_each_vg() which is
 * based on performing vg_read(), which provides a correct representation
 * of VGs/PVs, that is not provided by lvmcache_label_scan().
 */

int process_each_label(struct cmd_context *cmd, int argc, char **argv,
		       struct processing_handle *handle,
		       process_single_label_fn_t process_single_label)
{
	log_report_t saved_log_report_state = log_get_report_state();
	struct label *label;
	struct dev_iter *iter;
	struct device *dev;
	struct lvmcache_info *info;
	struct dm_list process_duplicates;
	struct device_list *devl;
	int ret_max = ECMD_PROCESSED;
	int ret;
	int opt = 0;

	dm_list_init(&process_duplicates);

	log_set_report_object_type(LOG_REPORT_OBJECT_TYPE_LABEL);

	lvmcache_label_scan(cmd);

	if (argc) {
		for (; opt < argc; opt++) {
			if (!(dev = dev_cache_get(cmd, argv[opt], cmd->filter))) {
				log_error("Failed to find device "
					  "\"%s\".", argv[opt]);
				ret_max = ECMD_FAILED;
				continue;
			}

			if (!(label = lvmcache_get_dev_label(dev))) {
				if (!lvmcache_dev_is_unchosen_duplicate(dev)) {
					log_error("No physical volume label read from %s.", argv[opt]);
					ret_max = ECMD_FAILED;
				} else {
					if (!(devl = malloc(sizeof(*devl))))
						return_0;
					devl->dev = dev;
					dm_list_add(&process_duplicates, &devl->list);
				}
				continue;
			}

			log_set_report_object_name_and_id(dev_name(dev), NULL);

			ret = process_single_label(cmd, label, handle);
			report_log_ret_code(ret);

			if (ret > ret_max)
				ret_max = ret;

			log_set_report_object_name_and_id(NULL, NULL);

			if (sigint_caught())
				break;
		}

		dm_list_iterate_items(devl, &process_duplicates) {
			/* 
			 * remove the existing dev for this pvid from lvmcache
			 * so that the duplicate dev can replace it.
			 */
			if ((info = lvmcache_info_from_pvid(devl->dev->pvid, NULL, 0)))
				lvmcache_del(info);

			/*
			 * add info to lvmcache from the duplicate dev.
			 */
			label_read(devl->dev);

			/*
			 * the info/label should now be found because
			 * the label_read should have added it.
			 */
			if (!(label = lvmcache_get_dev_label(devl->dev)))
				continue;

			log_set_report_object_name_and_id(dev_name(devl->dev), NULL);

			ret = process_single_label(cmd, label, handle);
			report_log_ret_code(ret);

			if (ret > ret_max)
				ret_max = ret;

			log_set_report_object_name_and_id(NULL, NULL);

			if (sigint_caught())
				break;
		}

		goto out;
	}

	if (!(iter = dev_iter_create(cmd->filter, 1))) {
		log_error("dev_iter creation failed.");
		ret_max = ECMD_FAILED;
		goto out;
	}

	while ((dev = dev_iter_get(cmd, iter)))
	{
		if (!(label = lvmcache_get_dev_label(dev)))
			continue;

		log_set_report_object_name_and_id(dev_name(label->dev), NULL);

		ret = process_single_label(cmd, label, handle);
		report_log_ret_code(ret);

		if (ret > ret_max)
			ret_max = ret;

		log_set_report_object_name_and_id(NULL, NULL);

		if (sigint_caught())
			break;
	}

	dev_iter_destroy(iter);
out:
	log_restore_report_state(saved_log_report_state);
	return ret_max;
}

/*
 * Parse persistent major minor parameters.
 *
 * --persistent is unspecified => state is deduced
 * from presence of options --minor or --major.
 *
 * -Mn => --minor or --major not allowed.
 *
 * -My => --minor is required (and also --major on <=2.4)
 */
int get_and_validate_major_minor(const struct cmd_context *cmd,
				 const struct format_type *fmt,
				 int32_t *major, int32_t *minor)
{
	if (arg_count(cmd, minor_ARG) > 1) {
		log_error("Option --minor may not be repeated.");
		return 0;
	}

	if (arg_count(cmd, major_ARG) > 1) {
		log_error("Option -j|--major may not be repeated.");
		return 0;
	}

	/* Check with default 'y' */
	if (!arg_int_value(cmd, persistent_ARG, 1)) { /* -Mn */
		if (arg_is_set(cmd, minor_ARG) || arg_is_set(cmd, major_ARG)) {
			log_error("Options --major and --minor are incompatible with -Mn.");
			return 0;
		}
		*major = *minor = -1;
		return 1;
	}

	/* -1 cannot be entered as an argument for --major, --minor */
	*major = arg_int_value(cmd, major_ARG, -1);
	*minor = arg_int_value(cmd, minor_ARG, -1);

	if (arg_is_set(cmd, persistent_ARG)) { /* -My */
		if (*minor == -1) {
			log_error("Please specify minor number with --minor when using -My.");
			return 0;
		}
	}

	if (!strncmp(cmd->kernel_vsn, "2.4.", 4)) {
		/* Major is required for 2.4 */
		if (arg_is_set(cmd, persistent_ARG) && *major < 0) {
			log_error("Please specify major number with --major when using -My.");
			return 0;
		}
	} else {
		if (*major != -1) {
			log_warn("WARNING: Ignoring supplied major number %d - "
				 "kernel assigns major numbers dynamically. "
				 "Using major number %d instead.",
				 *major, cmd->dev_types->device_mapper_major);
		}
		/* Stay with dynamic major:minor if minor is not specified. */
		*major = (*minor == -1) ? -1 : cmd->dev_types->device_mapper_major;
	}

	if ((*minor != -1) && !validate_major_minor(cmd, fmt, *major, *minor))
		return_0;

	return 1;
}

/*
 * Validate lvname parameter
 *
 * If it contains vgname, it is extracted from lvname.
 * If there is passed vgname, it is compared whether its the same name.
 */
int validate_lvname_param(struct cmd_context *cmd, const char **vg_name,
			  const char **lv_name)
{
	const char *vgname;
	const char *lvname;

	if (!lv_name || !*lv_name)
		return 1;  /* NULL lvname is ok */

	/* If contains VG name, extract it. */
	if (strchr(*lv_name, (int) '/')) {
		if (!(vgname = _extract_vgname(cmd, *lv_name, &lvname)))
			return_0;

		if (!*vg_name)
			*vg_name = vgname;
		else if (strcmp(vgname, *vg_name)) {
			log_error("Please use a single volume group name "
				  "(\"%s\" or \"%s\").", vgname, *vg_name);
			return 0;
		}

		*lv_name = lvname;
	}

	if (!validate_name(*lv_name)) {
		log_error("Logical volume name \"%s\" is invalid.",
			  *lv_name);
		return 0;
	}

	return 1;
}

/*
 * Validate lvname parameter
 * This name must follow restriction rules on prefixes and suffixes.
 *
 * If it contains vgname, it is extracted from lvname.
 * If there is passed vgname, it is compared whether its the same name.
 */
int validate_restricted_lvname_param(struct cmd_context *cmd, const char **vg_name,
				     const char **lv_name)
{
	if (!validate_lvname_param(cmd, vg_name, lv_name))
		return_0;

	if (lv_name && *lv_name && !apply_lvname_restrictions(*lv_name))
		return_0;

	return 1;
}

/*
 * Extract list of VG names and list of tags from command line arguments.
 */
static int _get_arg_vgnames(struct cmd_context *cmd,
			    int argc, char **argv,
			    const char *one_vgname,
			    struct dm_list *use_vgnames,
			    struct dm_list *arg_vgnames,
			    struct dm_list *arg_tags)
{
	int opt = 0;
	int ret_max = ECMD_PROCESSED;
	const char *vg_name;

	if (one_vgname) {
		if (!str_list_add(cmd->mem, arg_vgnames,
				  dm_pool_strdup(cmd->mem, one_vgname))) {
			log_error("strlist allocation failed.");
			return ECMD_FAILED;
		}
		return ret_max;
	}

	if (use_vgnames && !dm_list_empty(use_vgnames)) {
		dm_list_splice(arg_vgnames, use_vgnames);
		return ret_max;
	}

	for (; opt < argc; opt++) {
		vg_name = argv[opt];

		if (*vg_name == '@') {
			if (!validate_tag(vg_name + 1)) {
				log_error("Skipping invalid tag: %s", vg_name);
				if (ret_max < EINVALID_CMD_LINE)
					ret_max = EINVALID_CMD_LINE;
				continue;
			}

			if (!str_list_add(cmd->mem, arg_tags,
					  dm_pool_strdup(cmd->mem, vg_name + 1))) {
				log_error("strlist allocation failed.");
				return ECMD_FAILED;
			}

			continue;
		}

		vg_name = skip_dev_dir(cmd, vg_name, NULL);
		if (strchr(vg_name, '/')) {
			log_error("Invalid volume group name %s.", vg_name);
			if (ret_max < EINVALID_CMD_LINE)
				ret_max = EINVALID_CMD_LINE;
			continue;
		}

		if (!str_list_add(cmd->mem, arg_vgnames,
				  dm_pool_strdup(cmd->mem, vg_name))) {
			log_error("strlist allocation failed.");
			return ECMD_FAILED;
		}
	}

	return ret_max;
}

struct processing_handle *init_processing_handle(struct cmd_context *cmd, struct processing_handle *parent_handle)
{
	struct processing_handle *handle;

	if (!(handle = dm_pool_zalloc(cmd->mem, sizeof(struct processing_handle)))) {
		log_error("_init_processing_handle: failed to allocate memory for processing handle");
		return NULL;
	}

	handle->parent = parent_handle;

	/*
	 * For any reporting tool, the internal_report_for_select is reset to 0
	 * automatically because the internal reporting/selection is simply not
	 * needed - the reporting/selection is already a part of the code path
	 * used there.
	 *
	 * *The internal report for select is only needed for non-reporting tools!*
	 */
	handle->internal_report_for_select = arg_is_set(cmd, select_ARG);
	handle->include_historical_lvs = cmd->include_historical_lvs;

	if (!parent_handle && !cmd->cmd_report.report_group) {
		if (!report_format_init(cmd)) {
			dm_pool_free(cmd->mem, handle);
			return NULL;
		}
	} else
		cmd->cmd_report.saved_log_report_state = log_get_report_state();

	log_set_report_context(LOG_REPORT_CONTEXT_PROCESSING);
	return handle;
}

int init_selection_handle(struct cmd_context *cmd, struct processing_handle *handle,
			  report_type_t initial_report_type)
{
	struct selection_handle *sh;
	const char *selection;

	if (!(sh = dm_pool_zalloc(cmd->mem, sizeof(struct selection_handle)))) {
		log_error("_init_selection_handle: failed to allocate memory for selection handle");
		return 0;
	}

	if (!report_get_single_selection(cmd, initial_report_type, &selection))
		return_0;

	sh->report_type = initial_report_type;
	if (!(sh->selection_rh = report_init_for_selection(cmd, &sh->report_type, selection))) {
		dm_pool_free(cmd->mem, sh);
		return_0;
	}

	handle->selection_handle = sh;
	return 1;
}

void destroy_processing_handle(struct cmd_context *cmd, struct processing_handle *handle)
{
	if (handle) {
		if (handle->selection_handle && handle->selection_handle->selection_rh)
			dm_report_free(handle->selection_handle->selection_rh);

		log_restore_report_state(cmd->cmd_report.saved_log_report_state);

		if (!cmd->is_interactive) {
			if (!dm_report_group_destroy(cmd->cmd_report.report_group))
				stack;
			cmd->cmd_report.report_group = NULL;

			if (cmd->cmd_report.log_rh) {
				dm_report_free(cmd->cmd_report.log_rh);
				cmd->cmd_report.log_rh = NULL;
			}
		}
		/*
		 * TODO: think about better alternatives:
		 * handle mempool, dm_alloc for handle memory...
		 */
		memset(handle, 0, sizeof(*handle));
	}
}


int select_match_vg(struct cmd_context *cmd, struct processing_handle *handle,
		    struct volume_group *vg)
{
	int r;

	if (!handle->internal_report_for_select)
		return 1;

	handle->selection_handle->orig_report_type = VGS;
	if (!(r = report_for_selection(cmd, handle, NULL, vg, NULL)))
		log_error("Selection failed for VG %s.", vg->name);
	handle->selection_handle->orig_report_type = 0;

	return r;
}

int select_match_lv(struct cmd_context *cmd, struct processing_handle *handle,
		    struct volume_group *vg, struct logical_volume *lv)
{
	int r;

	if (!handle->internal_report_for_select)
		return 1;

	handle->selection_handle->orig_report_type = LVS;
	if (!(r = report_for_selection(cmd, handle, NULL, vg, lv)))
		log_error("Selection failed for LV %s.", lv->name);
	handle->selection_handle->orig_report_type = 0;

	return r;
}

int select_match_pv(struct cmd_context *cmd, struct processing_handle *handle,
		    struct volume_group *vg, struct physical_volume *pv)
{
	int r;

	if (!handle->internal_report_for_select)
		return 1;

	handle->selection_handle->orig_report_type = PVS;
	if (!(r = report_for_selection(cmd, handle, pv, vg, NULL)))
		log_error("Selection failed for PV %s.", dev_name(pv->dev));
	handle->selection_handle->orig_report_type = 0;

	return r;
}

static int _select_matches(struct processing_handle *handle)
{
	if (!handle->internal_report_for_select)
		return 1;

	return handle->selection_handle->selected;
}

static int _process_vgnameid_list(struct cmd_context *cmd, uint32_t read_flags,
				  struct dm_list *vgnameids_to_process,
				  struct dm_list *arg_vgnames,
				  struct dm_list *arg_tags,
				  struct processing_handle *handle,
				  process_single_vg_fn_t process_single_vg)
{
	log_report_t saved_log_report_state = log_get_report_state();
	char uuid[64] __attribute__((aligned(8)));
	struct volume_group *vg;
	struct vgnameid_list *vgnl;
	const char *vg_name;
	const char *vg_uuid;
	uint32_t lockd_state = 0;
	int whole_selected = 0;
	int ret_max = ECMD_PROCESSED;
	int ret;
	int skip;
	int notfound;
	int process_all = 0;
	int do_report_ret_code = 1;

	log_set_report_object_type(LOG_REPORT_OBJECT_TYPE_VG);

	/*
	 * If no VG names or tags were supplied, then process all VGs.
	 */
	if (dm_list_empty(arg_vgnames) && dm_list_empty(arg_tags))
		process_all = 1;

	/*
	 * FIXME If one_vgname, only proceed if exactly one VG matches tags or selection.
	 */
	dm_list_iterate_items(vgnl, vgnameids_to_process) {
		vg_name = vgnl->vg_name;
		vg_uuid = vgnl->vgid;
		skip = 0;
		notfound = 0;

		uuid[0] = '\0';
		if (is_orphan_vg(vg_name)) {
			log_set_report_object_type(LOG_REPORT_OBJECT_TYPE_ORPHAN);
			log_set_report_object_name_and_id(vg_name + sizeof(VG_ORPHANS), uuid);
		} else {
			if (vg_uuid && !id_write_format((const struct id*)vg_uuid, uuid, sizeof(uuid)))
				stack;
			log_set_report_object_name_and_id(vg_name, uuid);
		}

		if (sigint_caught()) {
			ret_max = ECMD_FAILED;
			goto_out;
		}

		log_very_verbose("Processing VG %s %s", vg_name, uuid);

		if (!lockd_vg(cmd, vg_name, NULL, 0, &lockd_state)) {
			stack;
			ret_max = ECMD_FAILED;
			report_log_ret_code(ret_max);
			continue;
		}

		vg = vg_read(cmd, vg_name, vg_uuid, read_flags, lockd_state);
		if (_ignore_vg(vg, vg_name, arg_vgnames, read_flags, &skip, &notfound)) {
			stack;
			ret_max = ECMD_FAILED;
			report_log_ret_code(ret_max);
			goto endvg;
		}
		if (skip || notfound)
			goto endvg;

		/* Process this VG? */
		if ((process_all ||
		    (!dm_list_empty(arg_vgnames) && str_list_match_item(arg_vgnames, vg_name)) ||
		    (!dm_list_empty(arg_tags) && str_list_match_list(arg_tags, &vg->tags, NULL))) &&
		    select_match_vg(cmd, handle, vg) && _select_matches(handle)) {

			log_very_verbose("Running command for VG %s %s", vg_name, vg_uuid ? uuid : "");

			ret = process_single_vg(cmd, vg_name, vg, handle);
			_update_selection_result(handle, &whole_selected);
			if (ret != ECMD_PROCESSED)
				stack;
			report_log_ret_code(ret);
			if (ret > ret_max)
				ret_max = ret;
		}

		if (!vg_read_error(vg))
			unlock_vg(cmd, vg, vg_name);
endvg:
		release_vg(vg);
		if (!lockd_vg(cmd, vg_name, "un", 0, &lockd_state))
			stack;

		log_set_report_object_name_and_id(NULL, NULL);
	}
	/* the VG is selected if at least one LV is selected */
	_set_final_selection_result(handle, whole_selected);
	do_report_ret_code = 0;
out:
	if (do_report_ret_code)
		report_log_ret_code(ret_max);
	log_restore_report_state(saved_log_report_state);
	return ret_max;
}

/*
 * Check if a command line VG name is ambiguous, i.e. there are multiple VGs on
 * the system that have the given name.  If *one* VG with the given name is
 * local and the rest are foreign, then use the local VG (removing foreign VGs
 * with the same name from the vgnameids_on_system list).  If multiple VGs with
 * the given name are local, we don't know which VG is intended, so remove the
 * ambiguous name from the list of args.
 */
static int _resolve_duplicate_vgnames(struct cmd_context *cmd,
				      struct dm_list *arg_vgnames,
				      struct dm_list *vgnameids_on_system)
{
	struct dm_str_list *sl, *sl2;
	struct vgnameid_list *vgnl, *vgnl2;
	char uuid[64] __attribute__((aligned(8)));
	int found;
	int ret = ECMD_PROCESSED;

	dm_list_iterate_items_safe(sl, sl2, arg_vgnames) {
		found = 0;
		dm_list_iterate_items(vgnl, vgnameids_on_system) {
			if (strcmp(sl->str, vgnl->vg_name))
				continue;
			found++;
		}

		if (found < 2)
			continue;

		/*
		 * More than one VG match the given name.
		 * If only one is local, use that one.
		 */

		found = 0;
		dm_list_iterate_items_safe(vgnl, vgnl2, vgnameids_on_system) {
			if (strcmp(sl->str, vgnl->vg_name))
				continue;

			/*
			 * label scan has already populated lvmcache vginfo with
			 * this information.
			 */
			if (lvmcache_vg_is_foreign(cmd, vgnl->vg_name, vgnl->vgid)) {
				if (!id_write_format((const struct id*)vgnl->vgid, uuid, sizeof(uuid)))
					stack;
				log_warn("WARNING: Ignoring foreign VG with matching name %s UUID %s.",
					 vgnl->vg_name, uuid);
				dm_list_del(&vgnl->list);
			} else {
				found++;
			}
		}

		if (found < 2)
			continue;

		/*
		 * More than one VG with this name is local so the intended VG
		 * is unknown.
		 */
		log_error("Multiple VGs found with the same name: skipping %s", sl->str);
		log_error("Use --select vg_uuid=<uuid> in place of the VG name.");
		dm_list_del(&sl->list);
		ret = ECMD_FAILED;
	}

	return ret;
}

/*
 * For each arg_vgname, move the corresponding entry from
 * vgnameids_on_system to vgnameids_to_process.  If an
 * item in arg_vgnames doesn't exist in vgnameids_on_system,
 * then add a new entry for it to vgnameids_to_process.
 */
static void _choose_vgs_to_process(struct cmd_context *cmd,
				   struct dm_list *arg_vgnames,
				   struct dm_list *vgnameids_on_system,
				   struct dm_list *vgnameids_to_process)
{
	char uuid[64] __attribute__((aligned(8)));
	struct dm_str_list *sl, *sl2;
	struct vgnameid_list *vgnl, *vgnl2;
	struct id id;
	int arg_is_uuid = 0;
	int found;

	dm_list_iterate_items_safe(sl, sl2, arg_vgnames) {
		found = 0;
		dm_list_iterate_items_safe(vgnl, vgnl2, vgnameids_on_system) {
			if (strcmp(sl->str, vgnl->vg_name))
				continue;

			dm_list_del(&vgnl->list);
			dm_list_add(vgnameids_to_process, &vgnl->list);
			found = 1;
			break;
		}

		/*
		 * If the VG name arg looks like a UUID, then check if it
		 * matches the UUID of a VG.  (--select should generally
		 * be used to select a VG by uuid instead.)
		 */
		if (!found && (cmd->cname->flags & ALLOW_UUID_AS_NAME))
			arg_is_uuid = id_read_format_try(&id, sl->str);

		if (!found && arg_is_uuid) {
			dm_list_iterate_items_safe(vgnl, vgnl2, vgnameids_on_system) {
				if (!(id_write_format((const struct id*)vgnl->vgid, uuid, sizeof(uuid))))
					continue;

				if (strcmp(sl->str, uuid))
					continue;

				log_print("Processing VG %s because of matching UUID %s",
					  vgnl->vg_name, uuid);

				dm_list_del(&vgnl->list);
				dm_list_add(vgnameids_to_process, &vgnl->list);

				/* Make the arg_vgnames entry use the actual VG name. */
				sl->str = dm_pool_strdup(cmd->mem, vgnl->vg_name);

				found = 1;
				break;
			}
		}
		
		/*
		 * If the name arg was not found in the list of all VGs, then
		 * it probably doesn't exist, but we want the "VG not found"
		 * failure to be handled by the existing vg_read() code for
		 * that error.  So, create an entry with just the VG name so
		 * that the processing loop will attempt to process it and use
		 * the vg_read() error path.
		 */
		if (!found) {
			log_verbose("VG name on command line not found in list of VGs: %s", sl->str);

			if (!(vgnl = dm_pool_alloc(cmd->mem, sizeof(*vgnl))))
				continue;

			vgnl->vgid = NULL;

			if (!(vgnl->vg_name = dm_pool_strdup(cmd->mem, sl->str)))
				continue;

			dm_list_add(vgnameids_to_process, &vgnl->list);
		}
	}
}

/*
 * Call process_single_vg() for each VG selected by the command line arguments.
 * If one_vgname is set, process only that VG and ignore argc/argv (which should be 0/NULL).
 * If one_vgname is not set, get VG names to process from argc/argv.
 */
int process_each_vg(struct cmd_context *cmd,
		    int argc, char **argv,
		    const char *one_vgname,
		    struct dm_list *use_vgnames,
		    uint32_t read_flags,
		    int include_internal,
		    struct processing_handle *handle,
		    process_single_vg_fn_t process_single_vg)
{
	log_report_t saved_log_report_state = log_get_report_state();
	int handle_supplied = handle != NULL;
	struct dm_list arg_tags;		/* str_list */
	struct dm_list arg_vgnames;		/* str_list */
	struct dm_list vgnameids_on_system;	/* vgnameid_list */
	struct dm_list vgnameids_to_process;	/* vgnameid_list */
	int enable_all_vgs = (cmd->cname->flags & ALL_VGS_IS_DEFAULT);
	int process_all_vgs_on_system = 0;
	int ret_max = ECMD_PROCESSED;
	int ret;

	log_set_report_object_type(LOG_REPORT_OBJECT_TYPE_VG);
	log_debug("Processing each VG");

	/* Disable error in vg_read so we can print it from ignore_vg. */
	cmd->vg_read_print_access_error = 0;

	dm_list_init(&arg_tags);
	dm_list_init(&arg_vgnames);
	dm_list_init(&vgnameids_on_system);
	dm_list_init(&vgnameids_to_process);

	/*
	 * Find any VGs or tags explicitly provided on the command line.
	 */
	if ((ret = _get_arg_vgnames(cmd, argc, argv, one_vgname, use_vgnames, &arg_vgnames, &arg_tags)) != ECMD_PROCESSED) {
		ret_max = ret;
		goto_out;
	}

	/*
	 * Process all VGs on the system when:
	 * . tags are specified and all VGs need to be read to
	 *   look for matching tags.
	 * . no VG names are specified and the command defaults
	 *   to processing all VGs when none are specified.
	 */
	if ((dm_list_empty(&arg_vgnames) && enable_all_vgs) || !dm_list_empty(&arg_tags))
		process_all_vgs_on_system = 1;

	/*
	 * Needed for a current listing of the global VG namespace.
	 */
	if (process_all_vgs_on_system && !lockd_gl(cmd, "sh", 0)) {
		ret_max = ECMD_FAILED;
		goto_out;
	}

	/*
	 * Scan all devices to populate lvmcache with initial
	 * list of PVs and VGs.
	 */
	lvmcache_label_scan(cmd);

	/*
	 * A list of all VGs on the system is needed when:
	 * . processing all VGs on the system
	 * . A VG name is specified which may refer to one
	 *   of multiple VGs on the system with that name.
	 */
	log_very_verbose("Obtaining the complete list of VGs to process");

	if (!get_vgnameids(cmd, &vgnameids_on_system, NULL, include_internal)) {
		ret_max = ECMD_FAILED;
		goto_out;
	}

	if (!dm_list_empty(&arg_vgnames)) {
		/* This may remove entries from arg_vgnames or vgnameids_on_system. */
		ret = _resolve_duplicate_vgnames(cmd, &arg_vgnames, &vgnameids_on_system);
		if (ret > ret_max)
			ret_max = ret;
		if (dm_list_empty(&arg_vgnames) && dm_list_empty(&arg_tags)) {
			ret_max = ECMD_FAILED;
			goto out;
		}
	}

	if (dm_list_empty(&arg_vgnames) && dm_list_empty(&vgnameids_on_system)) {
		/* FIXME Should be log_print, but suppressed for reporting cmds */
		log_verbose("No volume groups found.");
		ret_max = ECMD_PROCESSED;
		goto out;
	}

	if (dm_list_empty(&arg_vgnames))
		read_flags |= READ_OK_NOTFOUND;

	/*
	 * When processing all VGs, vgnameids_on_system simply becomes
	 * vgnameids_to_process.
	 * When processing only specified VGs, then for each item in
	 * arg_vgnames, move the corresponding entry from
	 * vgnameids_on_system to vgnameids_to_process.
	 */
	if (process_all_vgs_on_system)
		dm_list_splice(&vgnameids_to_process, &vgnameids_on_system);
	else
		_choose_vgs_to_process(cmd, &arg_vgnames, &vgnameids_on_system, &vgnameids_to_process);

	if (!handle && !(handle = init_processing_handle(cmd, NULL))) {
		ret_max = ECMD_FAILED;
		goto_out;
	}

	if (handle->internal_report_for_select && !handle->selection_handle &&
	    !init_selection_handle(cmd, handle, VGS)) {
		ret_max = ECMD_FAILED;
		goto_out;
	}

	ret = _process_vgnameid_list(cmd, read_flags, &vgnameids_to_process,
				     &arg_vgnames, &arg_tags, handle, process_single_vg);
	if (ret > ret_max)
		ret_max = ret;
out:
	if (!handle_supplied)
		destroy_processing_handle(cmd, handle);

	log_restore_report_state(saved_log_report_state);
	return ret_max;
}

static struct dm_str_list *_str_list_match_item_with_prefix(const struct dm_list *sll, const char *prefix, const char *str)
{
	struct dm_str_list *sl;
	size_t prefix_len = strlen(prefix);

	dm_list_iterate_items(sl, sll) {
		if (!strncmp(prefix, sl->str, prefix_len) &&
		    !strcmp(sl->str + prefix_len, str))
			return sl;
	}

	return NULL;
}

/*
 * Dummy LV, segment type and segment to represent all historical LVs.
 */
static struct logical_volume _historical_lv = {
	.name = "",
	.major = -1,
	.minor = -1,
	.snapshot_segs = DM_LIST_HEAD_INIT(_historical_lv.snapshot_segs),
	.segments = DM_LIST_HEAD_INIT(_historical_lv.segments),
	.tags = DM_LIST_HEAD_INIT(_historical_lv.tags),
	.segs_using_this_lv = DM_LIST_HEAD_INIT(_historical_lv.segs_using_this_lv),
	.indirect_glvs = DM_LIST_HEAD_INIT(_historical_lv.indirect_glvs),
	.hostname = "",
};

static struct segment_type _historical_segment_type = {
	.name = "historical",
	.flags = SEG_VIRTUAL | SEG_CANNOT_BE_ZEROED,
};

static struct lv_segment _historical_lv_segment = {
	.lv = &_historical_lv,
	.segtype = &_historical_segment_type,
	.len = 0,
	.tags = DM_LIST_HEAD_INIT(_historical_lv_segment.tags),
	.origin_list = DM_LIST_HEAD_INIT(_historical_lv_segment.origin_list),
};

int opt_in_list_is_set(struct cmd_context *cmd, int *opts, int count,
		       int *match_count, int *unmatch_count)
{
	int match = 0;
	int unmatch = 0;
	int i;

	for (i = 0; i < count; i++) {
		if (arg_is_set(cmd, opts[i]))
			match++;
		else
			unmatch++;
	}

	if (match_count)
		*match_count = match;
	if (unmatch_count)
		*unmatch_count = unmatch;

	return match ? 1 : 0;
}
      
void opt_array_to_str(struct cmd_context *cmd, int *opts, int count,
		      char *buf, int len)
{
	int pos = 0;
	int ret;
	int i;

	for (i = 0; i < count; i++) {
		ret = snprintf(buf + pos, len - pos, "%s ", arg_long_option_name(opts[i]));
		if (ret >= len - pos)
			break;
		pos += ret;
	}

	buf[len - 1] = '\0';
}

static void _lvp_bits_to_str(uint64_t bits, char *buf, int len)
{
	struct lv_prop *prop;
	int lvp_enum;
	int pos = 0;
	int ret;

	for (lvp_enum = 0; lvp_enum < LVP_COUNT; lvp_enum++) {
		if (!(prop = get_lv_prop(lvp_enum)))
			continue;

		if (lvp_bit_is_set(bits, lvp_enum)) {
			ret = snprintf(buf + pos, len - pos, "%s ", prop->name);
			if (ret >= len - pos)
				break;
			pos += ret;
		}
	}
	buf[len - 1] = '\0';
}

static void _lvt_bits_to_str(uint64_t bits, char *buf, int len)
{
	struct lv_type *type;
	int lvt_enum;
	int pos = 0;
	int ret;

	for (lvt_enum = 0; lvt_enum < LVT_COUNT; lvt_enum++) {
		if (!(type = get_lv_type(lvt_enum)))
			continue;

		if (lvt_bit_is_set(bits, lvt_enum)) {
			ret = snprintf(buf + pos, len - pos, "%s ", type->name);
			if (ret >= len - pos)
				break;
			pos += ret;
		}
	}
	buf[len - 1] = '\0';
}

/*
 * This is the lv_prop function pointer used for lv_is_foo() #defines.
 * Alternatively, lv_is_foo() could all be turned into functions.
 */

static int _lv_is_prop(struct cmd_context *cmd, struct logical_volume *lv, int lvp_enum)
{
	switch (lvp_enum) {
	case is_locked_LVP:
		return lv_is_locked(lv);
	case is_partial_LVP:
		return lv_is_partial(lv);
	case is_virtual_LVP:
		return lv_is_virtual(lv);
	case is_merging_LVP:
		return lv_is_merging(lv);
	case is_merging_origin_LVP:
		return lv_is_merging_origin(lv);
	case is_converting_LVP:
		return lv_is_converting(lv);
	case is_external_origin_LVP:
		return lv_is_external_origin(lv);
	case is_virtual_origin_LVP:
		return lv_is_virtual_origin(lv);
	case is_not_synced_LVP:
		return lv_is_not_synced(lv);
	case is_pending_delete_LVP:
		return lv_is_pending_delete(lv);
	case is_error_when_full_LVP:
		return lv_is_error_when_full(lv);
	case is_pvmove_LVP:
		return lv_is_pvmove(lv);
	case is_removed_LVP:
		return lv_is_removed(lv);
	case is_vg_writable_LVP:
		return (lv->vg->status & LVM_WRITE) ? 1 : 0;
	case is_thinpool_data_LVP:
		return lv_is_thin_pool_data(lv);
	case is_thinpool_metadata_LVP:
		return lv_is_thin_pool_metadata(lv);
	case is_cachepool_data_LVP:
		return lv_is_cache_pool_data(lv);
	case is_cachepool_metadata_LVP:
		return lv_is_cache_pool_metadata(lv);
	case is_mirror_image_LVP:
		return lv_is_mirror_image(lv);
	case is_mirror_log_LVP:
		return lv_is_mirror_log(lv);
	case is_raid_image_LVP:
		return lv_is_raid_image(lv);
	case is_raid_metadata_LVP:
		return lv_is_raid_metadata(lv);
	case is_origin_LVP: /* use lv_is_thick_origin */
		return lv_is_origin(lv);
	case is_thick_origin_LVP:
		return lv_is_thick_origin(lv);
	case is_thick_snapshot_LVP:
		return lv_is_thick_snapshot(lv);
	case is_thin_origin_LVP:
		return lv_is_thin_origin(lv, NULL);
	case is_thin_snapshot_LVP:
		return lv_is_thin_snapshot(lv);
	case is_cache_origin_LVP:
		return lv_is_cache_origin(lv);
	case is_merging_cow_LVP:
		return lv_is_merging_cow(lv);
	case is_cow_covering_origin_LVP:
		return lv_is_cow_covering_origin(lv);
	case is_visible_LVP:
		return lv_is_visible(lv);
	case is_historical_LVP:
		return lv_is_historical(lv);
	case is_raid_with_tracking_LVP:
		return lv_is_raid_with_tracking(lv);
	default:
		log_error(INTERNAL_ERROR "unknown lv property value lvp_enum %d", lvp_enum);
	}

	return 0;
}

/*
 * Check if an LV matches a given LV type enum.
 */

static int _lv_is_type(struct cmd_context *cmd, struct logical_volume *lv, int lvt_enum)
{
	struct lv_segment *seg = first_seg(lv);

	switch (lvt_enum) {
	case striped_LVT:
		return seg_is_striped(seg) && !lv_is_cow(lv);
	case linear_LVT:
		return seg_is_linear(seg) && !lv_is_cow(lv);
	case snapshot_LVT:
		return lv_is_cow(lv);
	case thin_LVT:
		return lv_is_thin_volume(lv);
	case thinpool_LVT:
		return lv_is_thin_pool(lv);
	case cache_LVT:
		return lv_is_cache(lv);
	case cachepool_LVT:
		return lv_is_cache_pool(lv);
	case vdo_LVT:
		return lv_is_vdo(lv);
	case vdopool_LVT:
		return lv_is_vdo_pool(lv);
	case vdopooldata_LVT:
		return lv_is_vdo_pool_data(lv);
	case mirror_LVT:
		return lv_is_mirror(lv);
	case raid_LVT:
		return lv_is_raid(lv);
	case raid0_LVT:
		return seg_is_any_raid0(seg);
	case raid1_LVT:
		return seg_is_raid1(seg);
	case raid4_LVT:
		return seg_is_raid4(seg);
	case raid5_LVT:
		return seg_is_any_raid5(seg);
	case raid6_LVT:
		return seg_is_any_raid6(seg);
	case raid10_LVT:
		return seg_is_raid10(seg);
	case error_LVT:
		return !strcmp(seg->segtype->name, SEG_TYPE_NAME_ERROR);
	case zero_LVT:
		return !strcmp(seg->segtype->name, SEG_TYPE_NAME_ZERO);
	default:
		log_error(INTERNAL_ERROR "unknown lv type value lvt_enum %d", lvt_enum);
	}

	return 0;
}

int get_lvt_enum(struct logical_volume *lv)
{
	struct lv_segment *seg = first_seg(lv);

	/*
	 * The order these are checked is important, because a snapshot LV has
	 * a linear seg type.
	 */

	if (lv_is_cow(lv))
		return snapshot_LVT;
	if (seg_is_linear(seg))
		return linear_LVT;
	if (seg_is_striped(seg))
		return striped_LVT;
	if (lv_is_thin_volume(lv))
		return thin_LVT;
	if (lv_is_thin_pool(lv))
		return thinpool_LVT;
	if (lv_is_cache(lv))
		return cache_LVT;
	if (lv_is_cache_pool(lv))
		return cachepool_LVT;
	if (lv_is_vdo(lv))
		return vdo_LVT;
	if (lv_is_vdo_pool(lv))
		return vdopool_LVT;
	if (lv_is_vdo_pool_data(lv))
		return vdopooldata_LVT;
	if (lv_is_mirror(lv))
		return mirror_LVT;
	if (lv_is_raid(lv))
		return raid_LVT;
	if (seg_is_any_raid0(seg))
		return raid0_LVT;
	if (seg_is_raid1(seg))
		return raid1_LVT;
	if (seg_is_raid4(seg))
		return raid4_LVT;
	if (seg_is_any_raid5(seg))
		return raid5_LVT;
	if (seg_is_any_raid6(seg))
		return raid6_LVT;
	if (seg_is_raid10(seg))
		return raid10_LVT;

	if (!strcmp(seg->segtype->name, SEG_TYPE_NAME_ERROR))
		return error_LVT;
	if (!strcmp(seg->segtype->name, SEG_TYPE_NAME_ZERO))
		return zero_LVT;

	return 0;
}

/*
 * Call lv_is_<type> for each <type>_LVT bit set in lvt_bits.
 * If lv matches one of the specified lv types, then return 1.
 */

static int _lv_types_match(struct cmd_context *cmd, struct logical_volume *lv, uint64_t lvt_bits,
			   uint64_t *match_bits, uint64_t *unmatch_bits)
{
	struct lv_type *type;
	int lvt_enum;
	int found_a_match = 0;
	int match;

	if (match_bits)
		*match_bits = 0;
	if (unmatch_bits)
		*unmatch_bits = 0;

	for (lvt_enum = 1; lvt_enum < LVT_COUNT; lvt_enum++) {
		if (!lvt_bit_is_set(lvt_bits, lvt_enum))
			continue;

		if (!(type = get_lv_type(lvt_enum)))
			continue;

		/*
		 * All types are currently handled by _lv_is_type()
		 * because lv_is_type() are #defines and not exposed
		 * in tools.h
		 */

		if (!type->fn)
			match = _lv_is_type(cmd, lv, lvt_enum);
		else
			match = type->fn(cmd, lv);

		if (match)
			found_a_match = 1;

		if (match_bits && match)
			*match_bits |= lvt_enum_to_bit(lvt_enum);

		if (unmatch_bits && !match)
			*unmatch_bits |= lvt_enum_to_bit(lvt_enum);
	}

	return found_a_match;
}

/*
 * Call lv_is_<prop> for each <prop>_LVP bit set in lvp_bits.
 * If lv matches all of the specified lv properties, then return 1.
 */

static int _lv_props_match(struct cmd_context *cmd, struct logical_volume *lv, uint64_t lvp_bits,
			   uint64_t *match_bits, uint64_t *unmatch_bits)
{
	struct lv_prop *prop;
	int lvp_enum;
	int found_a_mismatch = 0;
	int match;

	if (match_bits)
		*match_bits = 0;
	if (unmatch_bits)
		*unmatch_bits = 0;

	for (lvp_enum = 1; lvp_enum < LVP_COUNT; lvp_enum++) {
		if (!lvp_bit_is_set(lvp_bits, lvp_enum))
			continue;

		if (!(prop = get_lv_prop(lvp_enum)))
			continue;

		if (!prop->fn)
			match = _lv_is_prop(cmd, lv, lvp_enum);
		else
			match = prop->fn(cmd, lv);

		if (!match)
			found_a_mismatch = 1;

		if (match_bits && match)
			*match_bits |= lvp_enum_to_bit(lvp_enum);

		if (unmatch_bits && !match)
			*unmatch_bits |= lvp_enum_to_bit(lvp_enum);
	}

	return !found_a_mismatch;
}

static int _check_lv_types(struct cmd_context *cmd, struct logical_volume *lv, int pos)
{
	int ret;

	if (!pos)
		return 1;

	if (!cmd->command->required_pos_args[pos-1].def.lvt_bits)
		return 1;

	if (!val_bit_is_set(cmd->command->required_pos_args[pos-1].def.val_bits, lv_VAL)) {
		log_error(INTERNAL_ERROR "Command %d:%s arg position %d does not permit an LV (%llx)",
			  cmd->command->command_index, cmd->command->command_id,
			  pos, (unsigned long long)cmd->command->required_pos_args[pos-1].def.val_bits);
		return 0;
	}

	ret = _lv_types_match(cmd, lv, cmd->command->required_pos_args[pos-1].def.lvt_bits, NULL, NULL);
	if (!ret) {
		int lvt_enum = get_lvt_enum(lv);
		struct lv_type *type = get_lv_type(lvt_enum);
		log_warn("Command on LV %s does not accept LV type %s.",
			 display_lvname(lv), type ? type->name : "unknown");
	}

	return ret;
}

/* Check if LV passes each rule specified in command definition. */

static int _check_lv_rules(struct cmd_context *cmd, struct logical_volume *lv)
{
	char buf[64];
	struct cmd_rule *rule;
	struct lv_type *lvtype = NULL;
	uint64_t lv_props_match_bits = 0, lv_props_unmatch_bits = 0;
	uint64_t lv_types_match_bits = 0, lv_types_unmatch_bits = 0;
	int opts_match_count = 0, opts_unmatch_count = 0;
	int lvt_enum;
	int ret = 1;
	int i;

	lvt_enum = get_lvt_enum(lv);
	if (lvt_enum)
		lvtype = get_lv_type(lvt_enum);

	for (i = 0; i < cmd->command->rule_count; i++) {
		rule = &cmd->command->rules[i];

		/*
		 * RULE: <conditions> INVALID|REQUIRE <checks>
		 *
		 * If all the conditions apply to the command+LV, then
		 * the checks are performed.  If all conditions are zero
		 * (!opts_count, !lvt_bits, !lvp_bits), then the check
		 * is always performed.
		 *
		 * Conditions:
		 *
		 * 1. options (opts): if any of the specified options are set,
		 *    then the checks may apply.
		 *
		 * 2. LV types (lvt_bits): if any of the specified LV types
		 *    match the LV, then the checks may apply.
		 *
		 * 3. LV properties (lvp_bits): if all of the specified
		 *    LV properties match the LV, then the checks may apply.
		 *
		 * If conditions 1, 2, 3 all pass, then the checks apply.
		 *
		 * Checks:
		 *
		 * 1. options (check_opts):
		 *    INVALID: if any of the specified options are set,
		 *    then the command fails.
		 *    REQUIRE: if any of the specified options are not set,
		 *    then the command fails.
		 *
		 * 2. LV types (check_lvt_bits):
		 *    INVALID: if any of the specified LV types match the LV,
		 *    then the command fails.
		 *    REQUIRE: if none of the specified LV types match the LV,
		 *    then the command fails.
		 *
		 * 3. LV properties (check_lvp_bits):
		 *    INVALID: if any of the specified LV properties match
		 *    the LV, then the command fails.
		 *    REQUIRE: if any of the specified LV properties do not match
		 *    the LV, then the command fails.
		 */

		if (rule->opts_count && !opt_in_list_is_set(cmd, rule->opts, rule->opts_count, NULL, NULL))
			continue;

		/* If LV matches one type in lvt_bits, this returns 1. */
		if (rule->lvt_bits && !_lv_types_match(cmd, lv, rule->lvt_bits, NULL, NULL))
			continue;

		/* If LV matches all properties in lvp_bits, this returns 1. */
		if (rule->lvp_bits && !_lv_props_match(cmd, lv, rule->lvp_bits, NULL, NULL))
			continue;

		/*
		 * Check the options, LV types, LV properties.
		 */

		if (rule->check_opts)
			opt_in_list_is_set(cmd, rule->check_opts, rule->check_opts_count,
					   &opts_match_count, &opts_unmatch_count);

		if (rule->check_lvt_bits)
			_lv_types_match(cmd, lv, rule->check_lvt_bits,
					&lv_types_match_bits, &lv_types_unmatch_bits);

		if (rule->check_lvp_bits)
			_lv_props_match(cmd, lv, rule->check_lvp_bits,
					&lv_props_match_bits, &lv_props_unmatch_bits);
		
		/*
		 * Evaluate if the check results pass based on the rule.
		 * The options are checked again here because the previous
		 * option validation (during command matching) does not cover
		 * cases where the option is combined with conditions of LV types
		 * or properties.
		 */

		/* Fail if any invalid options are set. */

		if (rule->check_opts && (rule->rule == RULE_INVALID) && opts_match_count) {
			memset(buf, 0, sizeof(buf));
			opt_array_to_str(cmd, rule->check_opts, rule->check_opts_count, buf, sizeof(buf));
			log_warn("Command on LV %s has invalid use of option %s.",
				 display_lvname(lv), buf);
			ret = 0;
		}

		/* Fail if any required options are not set. */

		if (rule->check_opts && (rule->rule == RULE_REQUIRE) && opts_unmatch_count)  {
			memset(buf, 0, sizeof(buf));
			opt_array_to_str(cmd, rule->check_opts, rule->check_opts_count, buf, sizeof(buf));
			log_warn("Command on LV %s requires option %s.",
				 display_lvname(lv), buf);
			ret = 0;
		}

		/* Fail if the LV matches any of the invalid LV types. */

		if (rule->check_lvt_bits && (rule->rule == RULE_INVALID) && lv_types_match_bits) {
			if (rule->opts_count)
				log_warn("Command on LV %s uses options invalid with LV type %s.",
				 	 display_lvname(lv), lvtype ? lvtype->name : "unknown");
			else
				log_warn("Command on LV %s with invalid LV type %s.",
				 	 display_lvname(lv), lvtype ? lvtype->name : "unknown");
			ret = 0;
		}

		/* Fail if the LV does not match any of the required LV types. */

		if (rule->check_lvt_bits && (rule->rule == RULE_REQUIRE) && !lv_types_match_bits) {
			memset(buf, 0, sizeof(buf));
			_lvt_bits_to_str(rule->check_lvt_bits, buf, sizeof(buf));
			if (rule->opts_count)
				log_warn("Command on LV %s uses options that require LV types %s.",
					 display_lvname(lv), buf);
			else
				log_warn("Command on LV %s does not accept LV type %s. Required LV types are %s.",
					 display_lvname(lv), lvtype ? lvtype->name : "unknown", buf);
			ret = 0;
		}

		/* Fail if the LV matches any of the invalid LV properties. */

		if (rule->check_lvp_bits && (rule->rule == RULE_INVALID) && lv_props_match_bits) {
			memset(buf, 0, sizeof(buf));
			_lvp_bits_to_str(lv_props_match_bits, buf, sizeof(buf));
			if (rule->opts_count)
				log_warn("Command on LV %s uses options that are invalid with LV properties: %s.",
				 	 display_lvname(lv), buf);
			else
				log_warn("Command on LV %s is invalid on LV with properties: %s.",
				 	 display_lvname(lv), buf);
			ret = 0;
		}

		/* Fail if the LV does not match any of the required LV properties. */

		if (rule->check_lvp_bits && (rule->rule == RULE_REQUIRE) && lv_props_unmatch_bits) {
			memset(buf, 0, sizeof(buf));
			_lvp_bits_to_str(lv_props_unmatch_bits, buf, sizeof(buf));
			if (rule->opts_count)
				log_warn("Command on LV %s uses options that require LV properties: %s.",
				 	 display_lvname(lv), buf);
			else
				log_warn("Command on LV %s requires LV with properties: %s.",
				 	 display_lvname(lv), buf);
			ret = 0;
		}
	}

	return ret;
}

/*
 * Return which arg position the given LV is at,
 * where 1 represents the first position arg.
 * When the first position arg is repeatable,
 * return 1 for all.
 *
 * Return 0 when the command has no required
 * position args. (optional position args are
 * not considered.)
 */

static int _find_lv_arg_position(struct cmd_context *cmd, struct logical_volume *lv)
{
	const char *sep, *lvname;
	int i;

	if (cmd->command->rp_count == 0)
		return 0;

	if (cmd->command->rp_count == 1)
		return 1;

	for (i = 0; i < cmd->position_argc; i++) {
		if (i == cmd->command->rp_count)
			break;

		if (!val_bit_is_set(cmd->command->required_pos_args[i].def.val_bits, lv_VAL))
			continue;

		if ((sep = strstr(cmd->position_argv[i], "/")))
			lvname = sep + 1;
		else
			lvname = cmd->position_argv[i];

		if (!strcmp(lvname, lv->name))
			return i + 1;
	}

	/*
	 * If the last position arg is an LV and this
	 * arg is beyond that position, then the last
	 * LV position arg is repeatable, so return
	 * that position.
	 */
	if (i == cmd->command->rp_count) {
		int last_pos = cmd->command->rp_count;
		if (val_bit_is_set(cmd->command->required_pos_args[last_pos-1].def.val_bits, lv_VAL))
			return last_pos;
	}

	return 0;
}

int process_each_lv_in_vg(struct cmd_context *cmd, struct volume_group *vg,
			  struct dm_list *arg_lvnames, const struct dm_list *tags_in,
			  int stop_on_error,
			  struct processing_handle *handle,
			  check_single_lv_fn_t check_single_lv,
			  process_single_lv_fn_t process_single_lv)
{
	log_report_t saved_log_report_state = log_get_report_state();
	char lv_uuid[64] __attribute__((aligned(8)));
	char vg_uuid[64] __attribute__((aligned(8)));
	int ret_max = ECMD_PROCESSED;
	int ret = 0;
	int whole_selected = 0;
	int handle_supplied = handle != NULL;
	unsigned process_lv;
	unsigned process_all = 0;
	unsigned tags_supplied = 0;
	unsigned lvargs_supplied = 0;
	int lv_is_named_arg;
	int lv_arg_pos;
	struct lv_list *lvl;
	struct dm_str_list *sl;
	struct dm_list final_lvs;
	struct lv_list *final_lvl;
	struct dm_list found_arg_lvnames;
	struct glv_list *glvl, *tglvl;
	int do_report_ret_code = 1;

	log_set_report_object_type(LOG_REPORT_OBJECT_TYPE_LV);

	vg_uuid[0] = '\0';
	if (!id_write_format(&vg->id, vg_uuid, sizeof(vg_uuid)))
		stack;

	dm_list_init(&final_lvs);
	dm_list_init(&found_arg_lvnames);

	if (!vg_check_status(vg, EXPORTED_VG)) {
		ret_max = ECMD_FAILED;
		goto_out;
	}

	if (tags_in && !dm_list_empty(tags_in))
		tags_supplied = 1;

	if (arg_lvnames && !dm_list_empty(arg_lvnames))
		lvargs_supplied = 1;

	if (!handle && !(handle = init_processing_handle(cmd, NULL))) {
		ret_max = ECMD_FAILED;
		goto_out;
	}

	if (handle->internal_report_for_select && !handle->selection_handle &&
	    !init_selection_handle(cmd, handle, LVS)) {
		ret_max = ECMD_FAILED;
		goto_out;
	}

	/* Process all LVs in this VG if no restrictions given 
	 * or if VG tags match. */
	if ((!tags_supplied && !lvargs_supplied) ||
	    (tags_supplied && str_list_match_list(tags_in, &vg->tags, NULL)))
		process_all = 1;

	log_set_report_object_group_and_group_id(vg->name, vg_uuid);

	dm_list_iterate_items(lvl, &vg->lvs) {
		lv_uuid[0] = '\0';
		if (!id_write_format(&lvl->lv->lvid.id[1], lv_uuid, sizeof(lv_uuid)))
			stack;

		log_set_report_object_name_and_id(lvl->lv->name, lv_uuid);

		if (sigint_caught()) {
			ret_max = ECMD_FAILED;
			goto_out;
		}

		if (lv_is_snapshot(lvl->lv))
			continue;

		/* Skip availability change for non-virt snaps when processing all LVs */
		/* FIXME: pass process_all to process_single_lv() */
		if (process_all && arg_is_set(cmd, activate_ARG) &&
		    lv_is_cow(lvl->lv) && !lv_is_virtual_origin(origin_from_cow(lvl->lv)))
			continue;

		if (lv_is_virtual_origin(lvl->lv) && !arg_is_set(cmd, all_ARG)) {
			if (lvargs_supplied &&
			    str_list_match_item(arg_lvnames, lvl->lv->name))
				log_print_unless_silent("Ignoring virtual origin logical volume %s.",
							display_lvname(lvl->lv));
			continue;
		}

		/*
		 * Only let hidden LVs through if --all was used or the LVs 
		 * were specifically named on the command line.
		 */
		if (!lvargs_supplied && !lv_is_visible(lvl->lv) && !arg_is_set(cmd, all_ARG) &&
		    (!cmd->process_component_lvs || !lv_is_component(lvl->lv)))
			continue;

		/*
		 * Only let sanlock LV through if --all was used or if
		 * it is named on the command line.
		 */
		if (lv_is_lockd_sanlock_lv(lvl->lv)) {
			if (arg_is_set(cmd, all_ARG) ||
			    (lvargs_supplied && str_list_match_item(arg_lvnames, lvl->lv->name))) {
				log_very_verbose("Processing lockd_sanlock_lv %s/%s.", vg->name, lvl->lv->name);
			} else {
				continue;
			}
		}

		/*
		 * process the LV if one of the following:
		 * - process_all is set
		 * - LV name matches a supplied LV name
		 * - LV tag matches a supplied LV tag
		 * - LV matches the selection
		 */

		process_lv = process_all;

		if (lvargs_supplied && str_list_match_item(arg_lvnames, lvl->lv->name)) {
			/* Remove LV from list of unprocessed LV names */
			str_list_del(arg_lvnames, lvl->lv->name);
			if (!str_list_add(cmd->mem, &found_arg_lvnames, lvl->lv->name)) {
				log_error("strlist allocation failed.");
				ret_max = ECMD_FAILED;
				goto out;
			}
			process_lv = 1;
		}

		if (!process_lv && tags_supplied && str_list_match_list(tags_in, &lvl->lv->tags, NULL))
			process_lv = 1;

		process_lv = process_lv && select_match_lv(cmd, handle, vg, lvl->lv) && _select_matches(handle);

		if (sigint_caught()) {
			ret_max = ECMD_FAILED;
			goto_out;
		}

		if (!process_lv)
			continue;

		log_very_verbose("Adding %s/%s to the list of LVs to be processed.", vg->name, lvl->lv->name);

		if (!(final_lvl = dm_pool_zalloc(cmd->mem, sizeof(struct lv_list)))) {
			log_error("Failed to allocate final LV list item.");
			ret_max = ECMD_FAILED;
			goto out;
		}
		final_lvl->lv = lvl->lv;
		dm_list_add(&final_lvs, &final_lvl->list);
	}
	log_set_report_object_name_and_id(NULL, NULL);

	/*
	 * If a PV is stacked on an LV, then the LV is kept open
	 * in bcache, and needs to be closed so the open fd doesn't
	 * interfere with processing the LV.
	 */
	dm_list_iterate_items(lvl, &final_lvs)
		label_scan_invalidate_lv(cmd, lvl->lv);

	dm_list_iterate_items(lvl, &final_lvs) {
		lv_uuid[0] = '\0';
		if (!id_write_format(&lvl->lv->lvid.id[1], lv_uuid, sizeof(lv_uuid)))
			stack;

		log_set_report_object_name_and_id(lvl->lv->name, lv_uuid);

		if (sigint_caught()) {
			ret_max = ECMD_FAILED;
			goto_out;
		}
		/*
		 *  FIXME: Once we have index over vg->removed_lvs, check directly
		 *         LV presence there and remove LV_REMOVE flag/lv_is_removed fn
		 *         as they won't be needed anymore.
		 */
		if (lv_is_removed(lvl->lv))
			continue;

		lv_is_named_arg = str_list_match_item(&found_arg_lvnames, lvl->lv->name);

		lv_arg_pos = _find_lv_arg_position(cmd, lvl->lv);

		/*
		 * The command definition may include restrictions on the
		 * types and properties of LVs that can be processed.
		 */

		if (!_check_lv_types(cmd, lvl->lv, lv_arg_pos)) {
			/* FIXME: include this result in report log? */
			if (lv_is_named_arg) {
				log_error("Command not permitted on LV %s.", display_lvname(lvl->lv));
				ret_max = ECMD_FAILED;
			}
			continue;
		}

		if (!_check_lv_rules(cmd, lvl->lv)) {
			/* FIXME: include this result in report log? */
			if (lv_is_named_arg) {
				log_error("Command not permitted on LV %s.", display_lvname(lvl->lv));
				ret_max = ECMD_FAILED;
			} 
			continue;
		}

		if (check_single_lv && !check_single_lv(cmd, lvl->lv, handle, lv_is_named_arg)) {
			if (lv_is_named_arg)
				ret_max = ECMD_FAILED;
			continue;
		}

		log_very_verbose("Processing LV %s in VG %s.", lvl->lv->name, vg->name);

		ret = process_single_lv(cmd, lvl->lv, handle);
		if (handle_supplied)
			_update_selection_result(handle, &whole_selected);
		if (ret != ECMD_PROCESSED)
			stack;
		report_log_ret_code(ret);
		if (ret > ret_max)
			ret_max = ret;

		if (stop_on_error && ret != ECMD_PROCESSED) {
			do_report_ret_code = 0;
			goto_out;
		}
	}
	log_set_report_object_name_and_id(NULL, NULL);

	if (handle->include_historical_lvs && !tags_supplied) {
		if (!dm_list_size(&_historical_lv.segments))
			dm_list_add(&_historical_lv.segments, &_historical_lv_segment.list);
		_historical_lv.vg = vg;

		dm_list_iterate_items_safe(glvl, tglvl, &vg->historical_lvs) {
			lv_uuid[0] = '\0';
			if (!id_write_format(&glvl->glv->historical->lvid.id[1], lv_uuid, sizeof(lv_uuid)))
				stack;

			log_set_report_object_name_and_id(glvl->glv->historical->name, lv_uuid);

			process_lv = process_all;

			if (lvargs_supplied &&
			    (sl = _str_list_match_item_with_prefix(arg_lvnames, HISTORICAL_LV_PREFIX, glvl->glv->historical->name))) {
				str_list_del(arg_lvnames, glvl->glv->historical->name);
				dm_list_del(&sl->list);
				process_lv = 1;
			}

			process_lv = process_lv && select_match_lv(cmd, handle, vg, lvl->lv) && _select_matches(handle);

			if (sigint_caught()) {
				ret_max = ECMD_FAILED;
				goto_out;
			}

			if (!process_lv)
				continue;

			_historical_lv.this_glv = glvl->glv;
			_historical_lv.name = glvl->glv->historical->name;
			log_very_verbose("Processing historical LV %s in VG %s.", glvl->glv->historical->name, vg->name);

			ret = process_single_lv(cmd, &_historical_lv, handle);
			if (handle_supplied)
				_update_selection_result(handle, &whole_selected);
			if (ret != ECMD_PROCESSED)
				stack;
			report_log_ret_code(ret);
			if (ret > ret_max)
				ret_max = ret;

			if (stop_on_error && ret != ECMD_PROCESSED) {
				do_report_ret_code = 0;
				goto_out;
			}
		}
		log_set_report_object_name_and_id(NULL, NULL);
	}

	if (lvargs_supplied) {
		/*
		 * FIXME: lvm supports removal of LV with all its dependencies
		 * this leads to miscalculation that depends on the order of args.
		 */
		dm_list_iterate_items(sl, arg_lvnames) {
			log_set_report_object_name_and_id(sl->str, NULL);
			log_error("Failed to find logical volume \"%s/%s\"",
				  vg->name, sl->str);
			if (ret_max < ECMD_FAILED)
				ret_max = ECMD_FAILED;
			report_log_ret_code(ret_max);
		}
	}
	do_report_ret_code = 0;
out:
	if (do_report_ret_code)
		report_log_ret_code(ret_max);
	log_set_report_object_name_and_id(NULL, NULL);
	log_set_report_object_group_and_group_id(NULL, NULL);
	if (!handle_supplied)
		destroy_processing_handle(cmd, handle);
	else
		_set_final_selection_result(handle, whole_selected);
	log_restore_report_state(saved_log_report_state);

	return ret_max;
}

/*
 * If arg is tag, add it to arg_tags
 * else the arg is either vgname or vgname/lvname:
 * - add the vgname of each arg to arg_vgnames
 * - if arg has no lvname, add just vgname arg_lvnames,
 *   it represents all lvs in the vg
 * - if arg has lvname, add vgname/lvname to arg_lvnames
 */
static int _get_arg_lvnames(struct cmd_context *cmd,
			    int argc, char **argv,
			    const char *one_vgname, const char *one_lvname,
			    struct dm_list *arg_vgnames,
			    struct dm_list *arg_lvnames,
			    struct dm_list *arg_tags)
{
	int opt = 0;
	int ret_max = ECMD_PROCESSED;
	char *vglv;
	size_t vglv_sz;
	const char *vgname;
	const char *lv_name;
	const char *tmp_lv_name;
	const char *vgname_def;
	unsigned dev_dir_found;

	if (one_vgname) {
		if (!str_list_add(cmd->mem, arg_vgnames,
				  dm_pool_strdup(cmd->mem, one_vgname))) {
			log_error("strlist allocation failed.");
			return ECMD_FAILED;
		}

		if (!one_lvname) {
			if (!str_list_add(cmd->mem, arg_lvnames,
					  dm_pool_strdup(cmd->mem, one_vgname))) {
				log_error("strlist allocation failed.");
				return ECMD_FAILED;
			}
		} else {
			vglv_sz = strlen(one_vgname) + strlen(one_lvname) + 2;
			if (!(vglv = dm_pool_alloc(cmd->mem, vglv_sz)) ||
			    dm_snprintf(vglv, vglv_sz, "%s/%s", one_vgname, one_lvname) < 0) {
				log_error("vg/lv string alloc failed.");
				return ECMD_FAILED;
			}
			if (!str_list_add(cmd->mem, arg_lvnames, vglv)) {
				log_error("strlist allocation failed.");
				return ECMD_FAILED;
			}
		}
		return ret_max;
	}

	for (; opt < argc; opt++) {
		lv_name = argv[opt];
		dev_dir_found = 0;

		/* Do we have a tag or vgname or lvname? */
		vgname = lv_name;

		if (*vgname == '@') {
			if (!validate_tag(vgname + 1)) {
				log_error("Skipping invalid tag %s.", vgname);
				continue;
			}
			if (!str_list_add(cmd->mem, arg_tags,
					  dm_pool_strdup(cmd->mem, vgname + 1))) {
				log_error("strlist allocation failed.");
				return ECMD_FAILED;
			}
			continue;
		}

		/* FIXME Jumbled parsing */
		vgname = skip_dev_dir(cmd, vgname, &dev_dir_found);

		if (*vgname == '/') {
			log_error("\"%s\": Invalid path for Logical Volume.",
				  argv[opt]);
			if (ret_max < ECMD_FAILED)
				ret_max = ECMD_FAILED;
			continue;
		}
		lv_name = vgname;
		if ((tmp_lv_name = strchr(vgname, '/'))) {
			/* Must be an LV */
			lv_name = tmp_lv_name;
			while (*lv_name == '/')
				lv_name++;
			if (!(vgname = extract_vgname(cmd, vgname))) {
				if (ret_max < ECMD_FAILED) {
					stack;
					ret_max = ECMD_FAILED;
				}
				continue;
			}
		} else if (!dev_dir_found &&
			   (vgname_def = _default_vgname(cmd)))
			vgname = vgname_def;
		else
			lv_name = NULL;

		if (!str_list_add(cmd->mem, arg_vgnames,
				  dm_pool_strdup(cmd->mem, vgname))) {
			log_error("strlist allocation failed.");
			return ECMD_FAILED;
		}

		if (!lv_name) {
			if (!str_list_add(cmd->mem, arg_lvnames,
					  dm_pool_strdup(cmd->mem, vgname))) {
				log_error("strlist allocation failed.");
				return ECMD_FAILED;
			}
		} else {
			vglv_sz = strlen(vgname) + strlen(lv_name) + 2;
			if (!(vglv = dm_pool_alloc(cmd->mem, vglv_sz)) ||
			    dm_snprintf(vglv, vglv_sz, "%s/%s", vgname, lv_name) < 0) {
				log_error("vg/lv string alloc failed.");
				return ECMD_FAILED;
			}
			if (!str_list_add(cmd->mem, arg_lvnames, vglv)) {
				log_error("strlist allocation failed.");
				return ECMD_FAILED;
			}
		}
	}

	return ret_max;
}

/*
 * Finding vgname/lvname to process.
 *
 * When the position arg is a single name without any '/'
 * it is treated as an LV name (leaving the VG unknown).
 * Other option values, or env var, must be searched for a VG name.
 * If one of the option values contains a vgname/lvname value,
 * then the VG name is extracted and used for the LV position arg.
 * Or, if the env var has the VG name, that is used.
 *
 * Other option values that are searched for a VG name are:
 * --thinpool, --cachepool, --poolmetadata.
 *
 *  . command vg/lv1
 *  . add vg to arg_vgnames
 *  . add vg/lv1 to arg_lvnames
 *
 *  command lv1
 *  . error: no vg name (unless LVM_VG_NAME)
 *
 *  command --option=vg/lv1 vg/lv2
 *  . verify both vg names match
 *  . add vg to arg_vgnames
 *  . add vg/lv2 to arg_lvnames
 *
 *  command --option=lv1 lv2
 *  . error: no vg name (unless LVM_VG_NAME)
 *
 *  command --option=vg/lv1 lv2
 *  . add vg to arg_vgnames
 *  . add vg/lv2 to arg_lvnames
 *
 *  command --option=lv1 vg/lv2
 *  . add vg to arg_vgnames
 *  . add vg/lv2 to arg_lvnames
 */
static int _get_arg_lvnames_using_options(struct cmd_context *cmd,
			    		  int argc, char **argv,
					  struct dm_list *arg_vgnames,
					  struct dm_list *arg_lvnames,
					  struct dm_list *arg_tags)
{
	/* Array with args which may provide vgname */
	static const unsigned _opts_with_vgname[] = {
		cachepool_ARG, poolmetadata_ARG, thinpool_ARG
	};
	unsigned i;
	const char *pos_name = NULL;
	const char *arg_name = NULL;
	const char *pos_vgname = NULL;
	const char *opt_vgname = NULL;
	const char *pos_lvname = NULL;
	const char *use_vgname = NULL;
	char *vglv;
	size_t vglv_sz;

	if (argc != 1) {
		log_error("One LV position arg is required.");
		return ECMD_FAILED;
	}

	if (!(pos_name = dm_pool_strdup(cmd->mem, argv[0]))) {
		log_error("string alloc failed.");
		return ECMD_FAILED;
	}

	if (*pos_name == '@') {
		if (!validate_tag(pos_name + 1)) {
			log_error("Skipping invalid tag %s.", pos_name);
			return ECMD_FAILED;
		}
		if (!str_list_add(cmd->mem, arg_tags,
				  dm_pool_strdup(cmd->mem, pos_name + 1))) {
			log_error("strlist allocation failed.");
			return ECMD_FAILED;
		}
		return ECMD_PROCESSED;
	}

	if (strchr(pos_name, '/')) {
		/*
		 * This splits pos_name 'x/y' into pos_vgname 'x' and pos_lvname 'y'
		 * It skips repeated '/', e.g. x//y
		 * It also checks and fails for extra '/', e.g. x/y/z
		 */
		if (!(pos_vgname = _extract_vgname(cmd, pos_name, &pos_lvname)))
			return_0;
		use_vgname = pos_vgname;
	} else
		pos_lvname = pos_name;

	/* Go through the list of options which can provide vgname */
	for (i = 0; i < DM_ARRAY_SIZE(_opts_with_vgname); ++i) {
		if ((arg_name = arg_str_value(cmd, _opts_with_vgname[i], NULL)) &&
		    strchr(arg_name, '/')) {
			/* Combined VG/LV */
			/* Don't care about opt lvname, only extract vgname. */
			if (!(opt_vgname = _extract_vgname(cmd, arg_name, NULL)))
				return_0;
			/* Compare with already known vgname */
			if (use_vgname) {
				if (strcmp(use_vgname, opt_vgname)) {
					log_error("VG name mismatch from %s arg (%s) and option arg (%s).",
						  pos_vgname ? "position" : "option",
						  use_vgname, opt_vgname);
					return ECMD_FAILED;
				}
			} else
				use_vgname = opt_vgname;
		}
	}

	/* VG not specified as position nor as optional arg, so check for default VG */
	if (!use_vgname && !(use_vgname = _default_vgname(cmd)))  {
		log_error("Cannot find VG name for LV %s.", pos_lvname);
		return ECMD_FAILED;
	}

	if (!str_list_add(cmd->mem, arg_vgnames, dm_pool_strdup(cmd->mem, use_vgname))) {
		log_error("strlist allocation failed.");
		return ECMD_FAILED;
	}

	vglv_sz = strlen(use_vgname) + strlen(pos_lvname) + 2;

	if (!(vglv = dm_pool_alloc(cmd->mem, vglv_sz)) ||
	    dm_snprintf(vglv, vglv_sz, "%s/%s", use_vgname, pos_lvname) < 0) {
		log_error("vg/lv string alloc failed.");
		return ECMD_FAILED;
	}
	if (!str_list_add(cmd->mem, arg_lvnames, vglv)) {
		log_error("strlist allocation failed.");
		return ECMD_FAILED;
	}

	return ECMD_PROCESSED;
}

static int _process_lv_vgnameid_list(struct cmd_context *cmd, uint32_t read_flags,
				     struct dm_list *vgnameids_to_process,
				     struct dm_list *arg_vgnames,
				     struct dm_list *arg_lvnames,
				     struct dm_list *arg_tags,
				     struct processing_handle *handle,
				     check_single_lv_fn_t check_single_lv,
				     process_single_lv_fn_t process_single_lv)
{
	log_report_t saved_log_report_state = log_get_report_state();
	char uuid[64] __attribute__((aligned(8)));
	struct volume_group *vg;
	struct vgnameid_list *vgnl;
	struct dm_str_list *sl;
	struct dm_list *tags_arg;
	struct dm_list lvnames;
	uint32_t lockd_state = 0;
	const char *vg_name;
	const char *vg_uuid;
	const char *vgn;
	const char *lvn;
	int ret_max = ECMD_PROCESSED;
	int ret;
	int skip;
	int notfound;
	int do_report_ret_code = 1;

	log_set_report_object_type(LOG_REPORT_OBJECT_TYPE_VG);

	dm_list_iterate_items(vgnl, vgnameids_to_process) {
		vg_name = vgnl->vg_name;
		vg_uuid = vgnl->vgid;
		skip = 0;
		notfound = 0;

		uuid[0] = '\0';
		if (vg_uuid && !id_write_format((const struct id*)vg_uuid, uuid, sizeof(uuid)))
			stack;

		log_set_report_object_name_and_id(vg_name, uuid);

		if (sigint_caught()) {
			ret_max = ECMD_FAILED;
			goto_out;
		}

		/*
		 * arg_lvnames contains some elements that are just "vgname"
		 * which means process all lvs in the vg.  Other elements
		 * are "vgname/lvname" which means process only the select
		 * lvs in the vg.
		 */
		tags_arg = arg_tags;
		dm_list_init(&lvnames);	/* LVs to be processed in this VG */

		dm_list_iterate_items(sl, arg_lvnames) {
			vgn = sl->str;
			lvn = strchr(vgn, '/');

			if (!lvn && !strcmp(vgn, vg_name)) {
				/* Process all LVs in this VG */
				tags_arg = NULL;
				dm_list_init(&lvnames);
				break;
			}
			
			if (lvn && !strncmp(vgn, vg_name, strlen(vg_name)) &&
			    strlen(vg_name) == (size_t) (lvn - vgn)) {
				if (!str_list_add(cmd->mem, &lvnames,
						  dm_pool_strdup(cmd->mem, lvn + 1))) {
					log_error("strlist allocation failed.");
					ret_max = ECMD_FAILED;
					goto out;
				}
			}
		}

		log_very_verbose("Processing VG %s %s", vg_name, vg_uuid ? uuid : "");

		if (!lockd_vg(cmd, vg_name, NULL, 0, &lockd_state)) {
			ret_max = ECMD_FAILED;
			report_log_ret_code(ret_max);
			continue;
		}

		vg = vg_read(cmd, vg_name, vg_uuid, read_flags, lockd_state);
		if (_ignore_vg(vg, vg_name, arg_vgnames, read_flags, &skip, &notfound)) {
			stack;
			ret_max = ECMD_FAILED;
			report_log_ret_code(ret_max);
			goto endvg;
		}
		if (skip || notfound)
			goto endvg;

		ret = process_each_lv_in_vg(cmd, vg, &lvnames, tags_arg, 0,
					    handle, check_single_lv, process_single_lv);
		if (ret != ECMD_PROCESSED)
			stack;
		report_log_ret_code(ret);
		if (ret > ret_max)
			ret_max = ret;

		unlock_vg(cmd, vg, vg_name);
endvg:
		release_vg(vg);
		if (!lockd_vg(cmd, vg_name, "un", 0, &lockd_state))
			stack;
		log_set_report_object_name_and_id(NULL, NULL);
	}
	do_report_ret_code = 0;
out:
	if (do_report_ret_code)
		report_log_ret_code(ret_max);
	log_restore_report_state(saved_log_report_state);
	return ret_max;
}

/*
 * Call process_single_lv() for each LV selected by the command line arguments.
 */
int process_each_lv(struct cmd_context *cmd,
		    int argc, char **argv,
		    const char *one_vgname, const char *one_lvname,
		    uint32_t read_flags,
		    struct processing_handle *handle,
		    check_single_lv_fn_t check_single_lv,
		    process_single_lv_fn_t process_single_lv)
{
	log_report_t saved_log_report_state = log_get_report_state();
	int handle_supplied = handle != NULL;
	struct dm_list arg_tags;		/* str_list */
	struct dm_list arg_vgnames;		/* str_list */
	struct dm_list arg_lvnames;		/* str_list */
	struct dm_list vgnameids_on_system;	/* vgnameid_list */
	struct dm_list vgnameids_to_process;	/* vgnameid_list */
	int enable_all_vgs = (cmd->cname->flags & ALL_VGS_IS_DEFAULT);
	int process_all_vgs_on_system = 0;
	int ret_max = ECMD_PROCESSED;
	int ret;

	log_set_report_object_type(LOG_REPORT_OBJECT_TYPE_LV);

	/* Disable error in vg_read so we can print it from ignore_vg. */
	cmd->vg_read_print_access_error = 0;

	dm_list_init(&arg_tags);
	dm_list_init(&arg_vgnames);
	dm_list_init(&arg_lvnames);
	dm_list_init(&vgnameids_on_system);
	dm_list_init(&vgnameids_to_process);

	/*
	 * Find any LVs, VGs or tags explicitly provided on the command line.
	 */
	if (cmd->cname->flags & GET_VGNAME_FROM_OPTIONS)
		ret = _get_arg_lvnames_using_options(cmd, argc, argv, &arg_vgnames, &arg_lvnames, &arg_tags);
	else
		ret = _get_arg_lvnames(cmd, argc, argv, one_vgname, one_lvname, &arg_vgnames, &arg_lvnames, &arg_tags);

	if (ret != ECMD_PROCESSED) {
		ret_max = ret;
		goto_out;
	}

	if (!handle && !(handle = init_processing_handle(cmd, NULL))) {
		ret_max = ECMD_FAILED;
		goto_out;
	}

	if (handle->internal_report_for_select && !handle->selection_handle &&
	    !init_selection_handle(cmd, handle, LVS)) {
		ret_max = ECMD_FAILED;
		goto_out;
	}

	/*
	 * Process all VGs on the system when:
	 * . tags are specified and all VGs need to be read to
	 *   look for matching tags.
	 * . no VG names are specified and the command defaults
	 *   to processing all VGs when none are specified.
	 * . no VG names are specified and the select option needs
	 *   resolving.
	 */
	if (!dm_list_empty(&arg_tags))
		process_all_vgs_on_system = 1;
	else if (dm_list_empty(&arg_vgnames) && enable_all_vgs)
		process_all_vgs_on_system = 1;
	else if (dm_list_empty(&arg_vgnames) && handle->internal_report_for_select)
		process_all_vgs_on_system = 1;

	/*
	 * Needed for a current listing of the global VG namespace.
	 */
	if (process_all_vgs_on_system && !lockd_gl(cmd, "sh", 0)) {
		ret_max = ECMD_FAILED;
		goto_out;
	}

	/*
	 * Scan all devices to populate lvmcache with initial
	 * list of PVs and VGs.
	 */
	lvmcache_label_scan(cmd);

	/*
	 * A list of all VGs on the system is needed when:
	 * . processing all VGs on the system
	 * . A VG name is specified which may refer to one
	 *   of multiple VGs on the system with that name.
	 */
	log_very_verbose("Obtaining the complete list of VGs before processing their LVs");

	if (!get_vgnameids(cmd, &vgnameids_on_system, NULL, 0)) {
		ret_max = ECMD_FAILED;
		goto_out;
	}

	if (!dm_list_empty(&arg_vgnames)) {
		/* This may remove entries from arg_vgnames or vgnameids_on_system. */
		ret = _resolve_duplicate_vgnames(cmd, &arg_vgnames, &vgnameids_on_system);
		if (ret > ret_max)
			ret_max = ret;
		if (dm_list_empty(&arg_vgnames) && dm_list_empty(&arg_tags)) {
			ret_max = ECMD_FAILED;
			goto_out;
		}
	}

	if (dm_list_empty(&arg_vgnames) && dm_list_empty(&vgnameids_on_system)) {
		/* FIXME Should be log_print, but suppressed for reporting cmds */
		log_verbose("No volume groups found.");
		ret_max = ECMD_PROCESSED;
		goto out;
	}

	if (dm_list_empty(&arg_vgnames))
		read_flags |= READ_OK_NOTFOUND;

	/*
	 * When processing all VGs, vgnameids_on_system simply becomes
	 * vgnameids_to_process.
	 * When processing only specified VGs, then for each item in
	 * arg_vgnames, move the corresponding entry from
	 * vgnameids_on_system to vgnameids_to_process.
	 */
	if (process_all_vgs_on_system)
		dm_list_splice(&vgnameids_to_process, &vgnameids_on_system);
	else
		_choose_vgs_to_process(cmd, &arg_vgnames, &vgnameids_on_system, &vgnameids_to_process);

	ret = _process_lv_vgnameid_list(cmd, read_flags, &vgnameids_to_process, &arg_vgnames, &arg_lvnames,
					&arg_tags, handle, check_single_lv, process_single_lv);

	if (ret > ret_max)
		ret_max = ret;
out:
	if (!handle_supplied)
		destroy_processing_handle(cmd, handle);

	log_restore_report_state(saved_log_report_state);
	return ret_max;
}

static int _get_arg_pvnames(struct cmd_context *cmd,
			    int argc, char **argv,
			    struct dm_list *arg_pvnames,
			    struct dm_list *arg_tags)
{
	int opt = 0;
	char *at_sign, *tagname;
	char *arg_name;
	int ret_max = ECMD_PROCESSED;

	for (; opt < argc; opt++) {
		arg_name = argv[opt];

		dm_unescape_colons_and_at_signs(arg_name, NULL, &at_sign);
		if (at_sign && (at_sign == arg_name)) {
			tagname = at_sign + 1;

			if (!validate_tag(tagname)) {
				log_error("Skipping invalid tag %s.", tagname);
				if (ret_max < EINVALID_CMD_LINE)
					ret_max = EINVALID_CMD_LINE;
				continue;
			}
			if (!str_list_add(cmd->mem, arg_tags,
					  dm_pool_strdup(cmd->mem, tagname))) {
				log_error("strlist allocation failed.");
				return ECMD_FAILED;
			}
			continue;
		}

		if (!str_list_add(cmd->mem, arg_pvnames,
				  dm_pool_strdup(cmd->mem, arg_name))) {
			log_error("strlist allocation failed.");
			return ECMD_FAILED;
		}
	}

	return ret_max;
}

static int _get_arg_devices(struct cmd_context *cmd,
			    struct dm_list *arg_pvnames,
			    struct dm_list *arg_devices)
{
	struct dm_str_list *sl;
	struct device_id_list *dil;
	int ret_max = ECMD_PROCESSED;

	dm_list_iterate_items(sl, arg_pvnames) {
		if (!(dil = dm_pool_alloc(cmd->mem, sizeof(*dil)))) {
			log_error("device_id_list alloc failed.");
			return ECMD_FAILED;
		}

		if (!(dil->dev = dev_cache_get(cmd, sl->str, cmd->filter))) {
			log_error("Failed to find device for physical volume \"%s\".", sl->str);
			ret_max = ECMD_FAILED;
		} else {
			strncpy(dil->pvid, dil->dev->pvid, ID_LEN);
			dm_list_add(arg_devices, &dil->list);
		}
	}

	return ret_max;
}

static int _get_all_devices(struct cmd_context *cmd, struct dm_list *all_devices)
{
	struct dev_iter *iter;
	struct device *dev;
	struct device_id_list *dil;
	int r = ECMD_FAILED;

	log_debug("Getting list of all devices");

	if (!(iter = dev_iter_create(cmd->filter, 1))) {
		log_error("dev_iter creation failed.");
		return ECMD_FAILED;
	}

	while ((dev = dev_iter_get(cmd, iter))) {
		if (!(dil = dm_pool_alloc(cmd->mem, sizeof(*dil)))) {
			log_error("device_id_list alloc failed.");
			goto out;
		}

		strncpy(dil->pvid, dev->pvid, ID_LEN);
		dil->dev = dev;
		dm_list_add(all_devices, &dil->list);
	}

	r = ECMD_PROCESSED;
out:
	dev_iter_destroy(iter);
	return r;
}

static int _device_list_remove(struct dm_list *devices, struct device *dev)
{
	struct device_id_list *dil;

	dm_list_iterate_items(dil, devices) {
		if (dil->dev == dev) {
			dm_list_del(&dil->list);
			return 1;
		}
	}

	return 0;
}

static struct device_id_list *_device_list_find_dev(struct dm_list *devices, struct device *dev)
{
	struct device_id_list *dil;

	dm_list_iterate_items(dil, devices) {
		if (dil->dev == dev)
			return dil;
	}

	return NULL;
}

static int _device_list_copy(struct cmd_context *cmd, struct dm_list *src, struct dm_list *dst)
{
	struct device_id_list *dil;
	struct device_id_list *dil_new;

	dm_list_iterate_items(dil, src) {
		if (!(dil_new = dm_pool_alloc(cmd->mem, sizeof(*dil_new)))) {
			log_error("device_id_list alloc failed.");
			return ECMD_FAILED;
		}

		dil_new->dev = dil->dev;
		strncpy(dil_new->pvid, dil->pvid, ID_LEN);
		dm_list_add(dst, &dil_new->list);
	}

	return ECMD_PROCESSED;
}

/*
 * For each device in arg_devices or all_devices that has a pvid, add a copy of
 * that device to arg_missed.  All PVs (devices with a pvid) should have been
 * found while processing all VGs (including orphan VGs).  But, some may have
 * been missed if VGs were changing at the same time.  This function creates a
 * list of PVs that still remain in the given list, i.e. were missed the first
 * time.  A second iteration through VGs can look for these explicitly.
 * (arg_devices is used if specific PVs are being processed; all_devices is
 * used if all devs are being processed)
 */
static int _get_missed_pvs(struct cmd_context *cmd,
			   struct dm_list *devices,
			   struct dm_list *arg_missed)
{
	struct device_id_list *dil;
	struct device_id_list *dil_missed;

	dm_list_iterate_items(dil, devices) {
		if (!dil->pvid[0])
			continue;

		if (!(dil_missed = dm_pool_alloc(cmd->mem, sizeof(*dil_missed)))) {
			log_error("device_id_list alloc failed.");
			return ECMD_FAILED;
		}

		dil_missed->dev = dil->dev;
		strncpy(dil_missed->pvid, dil->pvid, ID_LEN);
		dm_list_add(arg_missed, &dil_missed->list);
	}

	return ECMD_PROCESSED;
}

static int _process_device_list(struct cmd_context *cmd, struct dm_list *all_devices,
				struct processing_handle *handle,
				process_single_pv_fn_t process_single_pv)
{
	struct physical_volume pv_dummy;
	struct physical_volume *pv;
	struct device_id_list *dil;
	int ret_max = ECMD_PROCESSED;
	int ret = 0;

	log_debug("Processing devices that are not PVs");

	/*
	 * Pretend that each device is a PV with dummy values.
	 * FIXME Formalise this extension or find an alternative.
	 */
	dm_list_iterate_items(dil, all_devices) {
		if (sigint_caught())
			return_ECMD_FAILED;

		memset(&pv_dummy, 0, sizeof(pv_dummy));
		dm_list_init(&pv_dummy.tags);
		dm_list_init(&pv_dummy.segments);
		pv_dummy.dev = dil->dev;
		pv = &pv_dummy;

		log_very_verbose("Processing device %s.", dev_name(dil->dev));

		ret = process_single_pv(cmd, NULL, pv, handle);

		if (ret > ret_max)
			ret_max = ret;
	}

	return ECMD_PROCESSED;
}

static int _process_duplicate_pvs(struct cmd_context *cmd,
				  struct dm_list *all_devices,
				  struct dm_list *arg_devices,
				  int process_all_devices,
				  struct processing_handle *handle,
				  process_single_pv_fn_t process_single_pv)
{
	struct device_id_list *dil;
	struct device_list *devl;
	struct dm_list unused_duplicate_devs;
	struct lvmcache_info *info;
	const char *vgname;
	const char *vgid;
	int ret_max = ECMD_PROCESSED;
	int ret = 0;

	struct physical_volume dummy_pv = {
		.tags = DM_LIST_HEAD_INIT(dummy_pv.tags),
		.segments= DM_LIST_HEAD_INIT(dummy_pv.segments),
	};

	struct format_instance dummy_fid = {
		.metadata_areas_in_use = DM_LIST_HEAD_INIT(dummy_fid.metadata_areas_in_use),
		.metadata_areas_ignored = DM_LIST_HEAD_INIT(dummy_fid.metadata_areas_ignored),
	};

	struct volume_group dummy_vg = {
		.fid = &dummy_fid,
		.name = "",
		.system_id = (char *) "",
		.pvs = DM_LIST_HEAD_INIT(dummy_vg.pvs),
		.lvs = DM_LIST_HEAD_INIT(dummy_vg.lvs),
		.historical_lvs = DM_LIST_HEAD_INIT(dummy_vg.historical_lvs),
		.tags = DM_LIST_HEAD_INIT(dummy_vg.tags),
	};

	dm_list_init(&unused_duplicate_devs);

	if (!lvmcache_get_unused_duplicate_devs(cmd, &unused_duplicate_devs))
		return_ECMD_FAILED;

	dm_list_iterate_items(devl, &unused_duplicate_devs) {
		/* Duplicates are displayed if -a is used or the dev is named as an arg. */

		_device_list_remove(all_devices, devl->dev);

		if (!process_all_devices && dm_list_empty(arg_devices))
			continue;

		if ((dil = _device_list_find_dev(arg_devices, devl->dev)))
			_device_list_remove(arg_devices, devl->dev);

		if (!process_all_devices && !dil)
			continue;

		if (!(cmd->cname->flags & ENABLE_DUPLICATE_DEVS))
			continue;

		/*
		 * Use the cached VG from the preferred device for the PV,
		 * the vg is only used to display the VG name.
		 *
		 * This VG from lvmcache was not read from the duplicate
		 * dev being processed here, but from the preferred dev
		 * in lvmcache.
		 *
		 * When a duplicate PV is displayed, the reporting fields
		 * that come from the VG metadata are not shown, because
		 * the dev is not a part of the VG, the dev for the
		 * preferred PV is (also the VG metadata in lvmcache is
		 * not from the duplicate dev, but from the preferred dev).
		 */

		log_very_verbose("Processing duplicate device %s.", dev_name(devl->dev));

		/*
		 * Don't pass dev to lvmcache_info_from_pvid because we looking
		 * for the chosen/preferred dev for this pvid.
		 */
		if (!(info = lvmcache_info_from_pvid(devl->dev->pvid, NULL, 0))) {
			log_error(INTERNAL_ERROR "No info for pvid");
			return_ECMD_FAILED;
		}

		vgname = lvmcache_vgname_from_info(info);
		vgid = vgname ? lvmcache_vgid_from_vgname(cmd, vgname) : NULL;

		dummy_pv.dev = devl->dev;
		dummy_pv.fmt = lvmcache_fmt_from_info(info);
		dummy_vg.name = vgname ?: "";

		if (vgid)
			memcpy(&dummy_vg.id, vgid, ID_LEN);
		else
			memset(&dummy_vg.id, 0, sizeof(dummy_vg.id));

		ret = process_single_pv(cmd, &dummy_vg, &dummy_pv, handle);

		if (ret > ret_max)
			ret_max = ret;

		if (sigint_caught())
			return_ECMD_FAILED;
	}

	return ECMD_PROCESSED;
}

static int _process_pvs_in_vg(struct cmd_context *cmd,
			      struct volume_group *vg,
			      struct dm_list *all_devices,
			      struct dm_list *arg_devices,
			      struct dm_list *arg_tags,
			      int process_all_pvs,
			      int process_all_devices,
			      int skip,
			      struct processing_handle *handle,
			      process_single_pv_fn_t process_single_pv)
{
	log_report_t saved_log_report_state = log_get_report_state();
	char pv_uuid[64] __attribute__((aligned(8)));
	char vg_uuid[64] __attribute__((aligned(8)));
	int handle_supplied = handle != NULL;
	struct physical_volume *pv;
	struct pv_list *pvl;
	struct device_id_list *dil;
	const char *pv_name;
	int process_pv;
	int do_report_ret_code = 1;
	int ret_max = ECMD_PROCESSED;
	int ret = 0;

	log_set_report_object_type(LOG_REPORT_OBJECT_TYPE_PV);

	vg_uuid[0] = '\0';
	if (!id_write_format(&vg->id, vg_uuid, sizeof(vg_uuid)))
		stack;

	if (!handle && (!(handle = init_processing_handle(cmd, NULL)))) {
		ret_max = ECMD_FAILED;
		goto_out;
	}

	if (handle->internal_report_for_select && !handle->selection_handle &&
	    !init_selection_handle(cmd, handle, PVS)) {
		ret_max = ECMD_FAILED;
		goto_out;
	}

	if (!is_orphan_vg(vg->name))
		log_set_report_object_group_and_group_id(vg->name, vg_uuid);

	dm_list_iterate_items(pvl, &vg->pvs) {
		pv = pvl->pv;
		pv_name = pv_dev_name(pv);
		pv_uuid[0]='\0';
		if (!id_write_format(&pv->id, pv_uuid, sizeof(pv_uuid)))
			stack;

		log_set_report_object_name_and_id(pv_name, pv_uuid);

		if (sigint_caught()) {
			ret_max = ECMD_FAILED;
			goto_out;
		}

		process_pv = process_all_pvs;

		/* Remove each arg_devices entry as it is processed. */

		if (!process_pv && !dm_list_empty(arg_devices) &&
		    (dil = _device_list_find_dev(arg_devices, pv->dev))) {
			process_pv = 1;
			_device_list_remove(arg_devices, dil->dev);
		}

		if (!process_pv && !dm_list_empty(arg_tags) &&
		    str_list_match_list(arg_tags, &pv->tags, NULL))
			process_pv = 1;

		process_pv = process_pv && select_match_pv(cmd, handle, vg, pv) && _select_matches(handle);

		if (process_pv) {
			if (skip)
				log_verbose("Skipping PV %s in VG %s.", pv_name, vg->name);
			else
				log_very_verbose("Processing PV %s in VG %s.", pv_name, vg->name);

			_device_list_remove(all_devices, pv->dev);

			/*
			 * pv->dev should be found in all_devices unless it's a
			 * case of a "missing device".  Previously there have
			 * been cases where we needed to skip processing the PV
			 * if pv->dev was not found in all_devices to avoid
			 * processing a PV twice, i.e. when the PV had no MDAs
			 * it would be seen once in its real VG and again
			 * wrongly in the orphan VG.  This no longer happens.
			 */

			if (!skip) {
				ret = process_single_pv(cmd, vg, pv, handle);
				if (ret != ECMD_PROCESSED)
					stack;
				report_log_ret_code(ret);
				if (ret > ret_max)
					ret_max = ret;
			}
		}

		/*
		 * When processing only specific PVs, we can quit once they've all been found.
	 	 */
		if (!process_all_pvs && dm_list_empty(arg_tags) && dm_list_empty(arg_devices))
			break;
		log_set_report_object_name_and_id(NULL, NULL);
	}
	do_report_ret_code = 0;
out:
	if (do_report_ret_code)
		report_log_ret_code(ret_max);
	log_set_report_object_name_and_id(NULL, NULL);
	log_set_report_object_group_and_group_id(NULL, NULL);
	if (!handle_supplied)
		destroy_processing_handle(cmd, handle);
	log_restore_report_state(saved_log_report_state);

	return ret_max;
}

/*
 * Iterate through all PVs in each listed VG.  Process a PV if
 * its dev or tag matches arg_devices or arg_tags.  If both
 * arg_devices and arg_tags are empty, then process all PVs.
 * No PV should be processed more than once.
 *
 * Each PV is removed from arg_devices and all_devices when it is
 * processed.  Any names remaining in arg_devices were not found, and
 * should produce an error.  Any devices remaining in all_devices were
 * not found and should be processed by process_device_list().
 */
static int _process_pvs_in_vgs(struct cmd_context *cmd, uint32_t read_flags,
			       struct dm_list *all_vgnameids,
			       struct dm_list *all_devices,
			       struct dm_list *arg_devices,
			       struct dm_list *arg_tags,
			       int process_all_pvs,
			       int process_all_devices,
			       struct processing_handle *handle,
			       process_single_pv_fn_t process_single_pv)
{
	log_report_t saved_log_report_state = log_get_report_state();
	char uuid[64] __attribute__((aligned(8)));
	struct volume_group *vg;
	struct vgnameid_list *vgnl;
	const char *vg_name;
	const char *vg_uuid;
	uint32_t lockd_state = 0;
	int ret_max = ECMD_PROCESSED;
	int ret;
	int skip;
	int notfound;
	int skip_lock;
	int do_report_ret_code = 1;

	log_set_report_object_type(LOG_REPORT_OBJECT_TYPE_VG);

	dm_list_iterate_items(vgnl, all_vgnameids) {
		vg_name = vgnl->vg_name;
		vg_uuid = vgnl->vgid;
		skip = 0;
		notfound = 0;

		uuid[0] = '\0';
		if (is_orphan_vg(vg_name)) {
			log_set_report_object_type(LOG_REPORT_OBJECT_TYPE_ORPHAN);
			log_set_report_object_name_and_id(vg_name + sizeof(VG_ORPHANS), uuid);
		} else {
			if (vg_uuid && !id_write_format((const struct id*)vg_uuid, uuid, sizeof(uuid)))
				stack;
			log_set_report_object_name_and_id(vg_name, uuid);
		}

		if (sigint_caught()) {
			ret_max = ECMD_FAILED;
			goto_out;
		}

		if (!lockd_vg(cmd, vg_name, NULL, 0, &lockd_state)) {
			ret_max = ECMD_FAILED;
			report_log_ret_code(ret_max);
			continue;
		}

		log_debug("Processing PVs in VG %s", vg_name);

		skip_lock = is_orphan_vg(vg_name) && (read_flags & PROCESS_SKIP_ORPHAN_LOCK);

		vg = vg_read(cmd, vg_name, vg_uuid, read_flags, lockd_state);
		if (_ignore_vg(vg, vg_name, NULL, read_flags, &skip, &notfound)) {
			stack;
			ret_max = ECMD_FAILED;
			report_log_ret_code(ret_max);
			if (!skip)
				goto endvg;
			/* Drop through to eliminate a clustered VG's PVs from the devices list */
		}
		if (notfound)
			goto endvg;
		
		/*
		 * Don't continue when skip is set, because we need to remove
		 * vg->pvs entries from devices list.
		 */
		
		ret = _process_pvs_in_vg(cmd, vg, all_devices, arg_devices, arg_tags,
					 process_all_pvs, process_all_devices, skip,
					 handle, process_single_pv);
		if (ret != ECMD_PROCESSED)
			stack;
		report_log_ret_code(ret);
		if (ret > ret_max)
			ret_max = ret;

		if (!skip && !skip_lock)
			unlock_vg(cmd, vg, vg->name);
endvg:
		release_vg(vg);
		if (!lockd_vg(cmd, vg_name, "un", 0, &lockd_state))
			stack;

		/* Quit early when possible. */
		if (!process_all_pvs && dm_list_empty(arg_tags) && dm_list_empty(arg_devices)) {
			do_report_ret_code = 0;
			goto out;
		}

		log_set_report_object_name_and_id(NULL, NULL);
	}
	do_report_ret_code = 0;
out:
	if (do_report_ret_code)
		report_log_ret_code(ret_max);
	log_restore_report_state(saved_log_report_state);
	return ret_max;
}

int process_each_pv(struct cmd_context *cmd,
		    int argc, char **argv, const char *only_this_vgname,
		    int all_is_set, uint32_t read_flags,
		    struct processing_handle *handle,
		    process_single_pv_fn_t process_single_pv)
{
	log_report_t saved_log_report_state = log_get_report_state();
	struct dm_list arg_tags;	/* str_list */
	struct dm_list arg_pvnames;	/* str_list */
	struct dm_list arg_devices;	/* device_id_list */
	struct dm_list arg_missed;	/* device_id_list */
	struct dm_list all_vgnameids;	/* vgnameid_list */
	struct dm_list all_devices;	/* device_id_list */
	struct device_id_list *dil;
	int process_all_pvs;
	int process_all_devices;
	int ret_max = ECMD_PROCESSED;
	int ret;

	log_set_report_object_type(LOG_REPORT_OBJECT_TYPE_PV);
	log_debug("Processing each PV");

	/*
	 * When processing a specific VG name, warn if it's inconsistent and
	 * print an error if it's not found.  Otherwise we're processing all
	 * VGs, in which case the command doesn't care if the VG is inconsisent
	 * or not found; it just wants to skip that VG.  (It may be not found
	 * if it was removed between creating the list of all VGs and then
	 * processing each VG.
	 */
	if (only_this_vgname)
		read_flags |= READ_WARN_INCONSISTENT;
	else
		read_flags |= READ_OK_NOTFOUND;

	/* Disable error in vg_read so we can print it from ignore_vg. */
	cmd->vg_read_print_access_error = 0;

	dm_list_init(&arg_tags);
	dm_list_init(&arg_pvnames);
	dm_list_init(&arg_devices);
	dm_list_init(&arg_missed);
	dm_list_init(&all_vgnameids);
	dm_list_init(&all_devices);

	/*
	 * Create two lists from argv:
	 * arg_pvnames: pvs explicitly named in argv
	 * arg_tags: tags explicitly named in argv
	 *
	 * Then convert arg_pvnames, which are free-form, user-specified,
	 * names/paths into arg_devices which can be used to match below.
	 */
	if ((ret = _get_arg_pvnames(cmd, argc, argv, &arg_pvnames, &arg_tags)) != ECMD_PROCESSED) {
		ret_max = ret;
		goto_out;
	}

	if ((cmd->cname->flags & DISALLOW_TAG_ARGS) && !dm_list_empty(&arg_tags)) {
		log_error("Tags cannot be used with this command.");
		return ECMD_FAILED;
	}

	process_all_pvs = dm_list_empty(&arg_pvnames) && dm_list_empty(&arg_tags);

	process_all_devices = process_all_pvs && (cmd->cname->flags & ENABLE_ALL_DEVS) && all_is_set;

	/* Needed for a current listing of the global VG namespace. */
	if (!only_this_vgname && !lockd_gl(cmd, "sh", 0)) {
		ret_max = ECMD_FAILED;
		goto_out;
	}

	if (!(read_flags & PROCESS_SKIP_SCAN))
		lvmcache_label_scan(cmd);

	if (!get_vgnameids(cmd, &all_vgnameids, only_this_vgname, 1)) {
		ret_max = ret;
		goto_out;
	}

	/*
	 * If the caller wants to process all devices (not just PVs), then all PVs
	 * from all VGs are processed first, removing them from all_devices.  Then
	 * any devs remaining in all_devices are processed.
	 */
	if ((ret = _get_all_devices(cmd, &all_devices)) != ECMD_PROCESSED) {
		ret_max = ret;
		goto_out;
	}

	if ((ret = _get_arg_devices(cmd, &arg_pvnames, &arg_devices)) != ECMD_PROCESSED)
		/* get_arg_devices reports the error for any PV names not found. */
		ret_max = ECMD_FAILED;

	ret = _process_pvs_in_vgs(cmd, read_flags, &all_vgnameids, &all_devices,
				  &arg_devices, &arg_tags,
				  process_all_pvs, process_all_devices,
				  handle, process_single_pv);
	if (ret != ECMD_PROCESSED)
		stack;
	if (ret > ret_max)
		ret_max = ret;

	/*
	 * Process the list of unused duplicate devs so they can be shown by
	 * report/display commands.  These are the devices that were not chosen
	 * to be used in lvmcache because another device with the same PVID was
	 * preferred.  The unused duplicate devs are not seen by
	 * _process_pvs_in_vgs, which only sees the preferred device for the
	 * PVID.
	 *
	 * The main purpose in reporting/displaying the unused duplicate PVs
	 * here is so that they do not appear to be unused/free devices or
	 * orphans.
	 *
	 * We do not allow modifying the unused duplicate PVs.  To modify a
	 * non-preferred duplicate PV, e.g. pvchange -u, a filter needs to be
	 * used with the command to exclude the other devices with the same
	 * PVID.  This results in the command seeing only the one device with
	 * the PVID and allows it to be changed.  (If the duplicates actually
	 * represent the same underlying storage, these precautions are
	 * unnecessary, but lvm can't tell when the duplicates are different
	 * paths to the same storage or different underlying storage.)
	 *
	 * Even the preferred duplicate PV in lvmcache is limitted from being
	 * modified (by allow_changes_with_duplicate_pvs setting), because lvm
	 * cannot be sure that the preferred duplicate device is the correct one,
	 * e.g. if a VG has two PVs, and both PVs are cloned, lvm might prefer
	 * one of the original PVs and one of the cloned PVs, pairing them
	 * together as the VG.  Any changes on the VG or PVs in that state would
	 * end up changing one of the original PVs and one of the cloned PVs.
	 *
	 * vgimportclone of the two cloned PVs changes their PV UUIDs and gives
	 * them a new VG name.
	 */

	ret = _process_duplicate_pvs(cmd, &all_devices, &arg_devices, process_all_devices,
				     handle, process_single_pv);
	if (ret != ECMD_PROCESSED)
		stack;
	if (ret > ret_max)
		ret_max = ret;

	/*
	 * If the orphans lock was held, there shouldn't be missed devices.
	 */
	if (read_flags & PROCESS_SKIP_ORPHAN_LOCK)
		goto skip_missed;

	/*
	 * Some PVs may have been missed by the first search if another command
	 * moved them at the same time.  Repeat the search for only the
	 * specific PVs missed.  lvmcache needs clearing for a fresh search.
	 *
	 * If missed PVs are found in this repeated search, they are removed
	 * from the arg_missed list, but they also need to be removed from the
	 * arg_devices list, otherwise the check at the end will produce an
	 * error, thinking they weren't found.  This is the reason for saving
	 * and comparing the original arg_missed list.
	 */
	if (!process_all_pvs)
		_get_missed_pvs(cmd, &arg_devices, &arg_missed);
	else
		_get_missed_pvs(cmd, &all_devices, &arg_missed);

	if (!dm_list_empty(&arg_missed)) {
		struct dm_list arg_missed_orig;

		dm_list_init(&arg_missed_orig);
		_device_list_copy(cmd, &arg_missed, &arg_missed_orig);

		log_verbose("Some PVs were not found in first search, retrying.");

		lvmcache_label_scan(cmd);

		ret = _process_pvs_in_vgs(cmd, read_flags, &all_vgnameids, &all_devices,
					  &arg_missed, &arg_tags, 0, 0,
					  handle, process_single_pv);
		if (ret != ECMD_PROCESSED)
			stack;
		if (ret > ret_max)
			ret_max = ret;

		/* Devices removed from arg_missed are removed from arg_devices. */
		dm_list_iterate_items(dil, &arg_missed_orig) {
			if (!_device_list_find_dev(&arg_missed, dil->dev))
				_device_list_remove(&arg_devices, dil->dev);
		}
	}

skip_missed:
	dm_list_iterate_items(dil, &arg_devices) {
		log_error("Failed to find physical volume \"%s\".", dev_name(dil->dev));
		ret_max = ECMD_FAILED;
	}

	if (!process_all_devices)
		goto out;

	ret = _process_device_list(cmd, &all_devices, handle, process_single_pv);
	if (ret != ECMD_PROCESSED)
		stack;
	if (ret > ret_max)
		ret_max = ret;
out:
	log_restore_report_state(saved_log_report_state);
	return ret_max;
}

int process_each_pv_in_vg(struct cmd_context *cmd, struct volume_group *vg,
			  struct processing_handle *handle,
			  process_single_pv_fn_t process_single_pv)
{
	log_report_t saved_log_report_state = log_get_report_state();
	char pv_uuid[64] __attribute__((aligned(8)));
	char vg_uuid[64] __attribute__((aligned(8)));
	int whole_selected = 0;
	int ret_max = ECMD_PROCESSED;
	int ret;
	int do_report_ret_code = 1;
	struct pv_list *pvl;

	log_set_report_object_type(LOG_REPORT_OBJECT_TYPE_PV);

	vg_uuid[0] = '\0';
	if (!id_write_format(&vg->id, vg_uuid, sizeof(vg_uuid)))
		stack;

	if (!is_orphan_vg(vg->name))
		log_set_report_object_group_and_group_id(vg->name, vg_uuid);

	dm_list_iterate_items(pvl, &vg->pvs) {
		pv_uuid[0] = '\0';
		if (!id_write_format(&pvl->pv->id, pv_uuid, sizeof(pv_uuid)))
			stack;

		log_set_report_object_name_and_id(pv_dev_name(pvl->pv), pv_uuid);

		if (sigint_caught()) {
			ret_max = ECMD_FAILED;
			goto_out;
		}

		ret = process_single_pv(cmd, vg, pvl->pv, handle);
		_update_selection_result(handle, &whole_selected);
		if (ret != ECMD_PROCESSED)
			stack;
		report_log_ret_code(ret);
		if (ret > ret_max)
			ret_max = ret;

		log_set_report_object_name_and_id(NULL, NULL);
	}

	_set_final_selection_result(handle, whole_selected);
	do_report_ret_code = 0;
out:
	if (do_report_ret_code)
		report_log_ret_code(ret_max);
	log_restore_report_state(saved_log_report_state);
	return ret_max;
}

int lvremove_single(struct cmd_context *cmd, struct logical_volume *lv,
		    struct processing_handle *handle __attribute__((unused)))
{
	/*
	 * Single force is equivalent to single --yes
	 * Even multiple --yes are equivalent to single --force
	 * When we require -ff it cannot be replaced with -f -y
	 */
	force_t force = (force_t) arg_count(cmd, force_ARG)
		? : (arg_is_set(cmd, yes_ARG) ? DONT_PROMPT : PROMPT);

	if (!lv_remove_with_dependencies(cmd, lv, force, 0))
		return_ECMD_FAILED;

	return ECMD_PROCESSED;
}

int pvcreate_params_from_args(struct cmd_context *cmd, struct pvcreate_params *pp)
{
	pp->yes = arg_count(cmd, yes_ARG);
	pp->force = (force_t) arg_count(cmd, force_ARG);

	if (arg_int_value(cmd, labelsector_ARG, 0) >= LABEL_SCAN_SECTORS) {
		log_error("labelsector must be less than %lu.",
			  LABEL_SCAN_SECTORS);
		return 0;
	}

	pp->pva.label_sector = arg_int64_value(cmd, labelsector_ARG,
					       DEFAULT_LABELSECTOR);

	if (arg_is_set(cmd, metadataignore_ARG))
		pp->pva.metadataignore = arg_int_value(cmd, metadataignore_ARG,
						   DEFAULT_PVMETADATAIGNORE);
	else
		pp->pva.metadataignore = find_config_tree_bool(cmd, metadata_pvmetadataignore_CFG, NULL);

	if (arg_is_set(cmd, pvmetadatacopies_ARG) &&
	    !arg_int_value(cmd, pvmetadatacopies_ARG, -1) &&
	    pp->pva.metadataignore) {
		log_error("metadataignore only applies to metadatacopies > 0.");
		return 0;
	}

	pp->zero = arg_int_value(cmd, zero_ARG, 1);

	if (arg_sign_value(cmd, dataalignment_ARG, SIGN_NONE) == SIGN_MINUS) {
		log_error("Physical volume data alignment may not be negative.");
		return 0;
	}
	pp->pva.data_alignment = arg_uint64_value(cmd, dataalignment_ARG, UINT64_C(0));

	if (pp->pva.data_alignment > UINT32_MAX) {
		log_error("Physical volume data alignment is too big.");
		return 0;
	}

	if (arg_sign_value(cmd, dataalignmentoffset_ARG, SIGN_NONE) == SIGN_MINUS) {
		log_error("Physical volume data alignment offset may not be negative.");
		return 0;
	}
	pp->pva.data_alignment_offset = arg_uint64_value(cmd, dataalignmentoffset_ARG, UINT64_C(0));

	if (pp->pva.data_alignment_offset > UINT32_MAX) {
		log_error("Physical volume data alignment offset is too big.");
		return 0;
	}

	if ((pp->pva.data_alignment + pp->pva.data_alignment_offset) &&
	    (pp->pva.pe_start != PV_PE_START_CALC)) {
		if ((pp->pva.data_alignment ? pp->pva.pe_start % pp->pva.data_alignment : pp->pva.pe_start) != pp->pva.data_alignment_offset) {
			log_warn("WARNING: Ignoring data alignment %s"
				 " incompatible with restored pe_start value %s.",
				 display_size(cmd, pp->pva.data_alignment + pp->pva.data_alignment_offset),
				 display_size(cmd, pp->pva.pe_start));
			pp->pva.data_alignment = 0;
			pp->pva.data_alignment_offset = 0;
		}
	}

	if (arg_sign_value(cmd, metadatasize_ARG, SIGN_NONE) == SIGN_MINUS) {
		log_error("Metadata size may not be negative.");
		return 0;
	}

	if (arg_sign_value(cmd, bootloaderareasize_ARG, SIGN_NONE) == SIGN_MINUS) {
		log_error("Bootloader area size may not be negative.");
		return 0;
	}

	pp->pva.pvmetadatasize = arg_uint64_value(cmd, metadatasize_ARG, UINT64_C(0));
	if (!pp->pva.pvmetadatasize)
		pp->pva.pvmetadatasize = find_config_tree_int(cmd, metadata_pvmetadatasize_CFG, NULL);

	pp->pva.pvmetadatacopies = arg_int_value(cmd, pvmetadatacopies_ARG, -1);
	if (pp->pva.pvmetadatacopies < 0)
		pp->pva.pvmetadatacopies = find_config_tree_int(cmd, metadata_pvmetadatacopies_CFG, NULL);

	pp->pva.ba_size = arg_uint64_value(cmd, bootloaderareasize_ARG, pp->pva.ba_size);

	return 1;
}

enum {
	PROMPT_PVCREATE_PV_IN_VG = 1,
	PROMPT_PVREMOVE_PV_IN_VG = 2,
	PROMPT_PVCREATE_DEV_SIZE = 4,
};

enum {
	PROMPT_ANSWER_NO = 1,
	PROMPT_ANSWER_YES = 2
};

/*
 * When a prompt entry is created, save any strings or info
 * in this struct that are needed for the prompt messages.
 * The VG/PV structs are not be available when the prompt
 * is run.
 */
struct pvcreate_prompt {
	struct dm_list list;
	uint32_t type;
	uint64_t size;
	uint64_t new_size;
	const char *pv_name;
	const char *vg_name;
	struct device *dev;
	int answer;
	unsigned abort_command : 1;
	unsigned vg_name_unknown : 1;
};

struct pvcreate_device {
	struct dm_list list;
	const char *name;
	struct device *dev;
	char pvid[ID_LEN + 1];
	const char *vg_name;
	int wiped;
	unsigned is_not_pv : 1;     /* device is not a PV */
	unsigned is_orphan_pv : 1;  /* device is an orphan PV */
	unsigned is_vg_pv : 1;      /* device is a PV used in a VG */
	unsigned is_used_unknown_pv : 1; /* device is a PV used in an unknown VG */
};

/*
 * If a PV is in a VG, and pvcreate or pvremove is run on it:
 *
 * pvcreate|pvremove -f      : fails
 * pvcreate|pvremove -y      : fails
 * pvcreate|pvremove -f -y   : fails
 * pvcreate|pvremove -ff     : get y/n prompt
 * pvcreate|pvremove -ff -y  : succeeds
 *
 * FIXME: there are a lot of various phrasings used depending on the
 * command and specific case.  Find some similar way to phrase these.
 */

static void _check_pvcreate_prompt(struct cmd_context *cmd,
				   struct pvcreate_params *pp,
				   struct pvcreate_prompt *prompt,
				   int ask)
{
	const char *vgname = prompt->vg_name ? prompt->vg_name : "<unknown>";
	const char *pvname = prompt->pv_name;
	int answer_yes = 0;
	int answer_no = 0;

	/* The VG name can be unknown when the PV is used but metadata is not available */

	if (prompt->type & PROMPT_PVCREATE_PV_IN_VG) {
		if (pp->force != DONT_PROMPT_OVERRIDE) {
			answer_no = 1;

			if (prompt->vg_name_unknown) {
				log_error("PV %s is used by a VG but its metadata is missing.", pvname);
				log_error("Can't initialize PV '%s' without -ff.", pvname);
			} else if (!strcmp(command_name(cmd), "pvcreate")) {
				log_error("Can't initialize physical volume \"%s\" of volume group \"%s\" without -ff", pvname, vgname);
			} else {
				log_error("Physical volume '%s' is already in volume group '%s'", pvname, vgname);
				log_error("Unable to add physical volume '%s' to volume group '%s'", pvname, vgname);
			}
		} else if (pp->yes) {
			answer_yes = 1;
		} else if (ask) {
			if (yes_no_prompt("Really INITIALIZE physical volume \"%s\" of volume group \"%s\" [y/n]? ", pvname, vgname) == 'n') {
				answer_no = 1;
			} else {
				answer_yes = 1;
				log_warn("WARNING: Forcing physical volume creation on %s of volume group \"%s\"", pvname, vgname);
			}
		}

	}

	if (prompt->type & PROMPT_PVCREATE_DEV_SIZE) {
		if (pp->yes) {
			log_warn("WARNING: Faking size of PV %s. Don't write outside real device.", pvname);
			answer_yes = 1;
		} else if (ask) {
			if (prompt->new_size != prompt->size) {
				if (yes_no_prompt("WARNING: %s: device size %s does not match requested size %s. Proceed? [y/n]: ", pvname,
						  display_size(cmd, prompt->size),
						  display_size(cmd, prompt->new_size)) == 'n') {
					answer_no = 1;
				} else {
					answer_yes = 1;
					log_warn("WARNING: Faking size of PV %s. Don't write outside real device.", pvname);
				}
			}
		}
	}
	
	if (prompt->type & PROMPT_PVREMOVE_PV_IN_VG) {
		if (pp->force != DONT_PROMPT_OVERRIDE) {
			answer_no = 1;

			if (prompt->vg_name_unknown)
				log_error("PV %s is used by a VG but its metadata is missing.", pvname);
			else
				log_error("PV %s is used by VG %s so please use vgreduce first.", pvname, vgname);
			log_error("(If you are certain you need pvremove, then confirm by using --force twice.)");
		} else if (pp->yes) {
			log_warn("WARNING: PV %s is used by VG %s.", pvname, vgname);
			answer_yes = 1;
		} else if (ask) {
			log_warn("WARNING: PV %s is used by VG %s.", pvname, vgname);
			if (yes_no_prompt("Really WIPE LABELS from physical volume \"%s\" of volume group \"%s\" [y/n]? ", pvname, vgname) == 'n')
				answer_no = 1;
			else
				answer_yes = 1;
		}
	}

	if (answer_yes && answer_no) {
		log_warn("WARNING: prompt answer yes is overriden by prompt answer no.");
		answer_yes = 0;
	}

	/*
	 * no answer is valid when not asking the user.
	 * the caller uses this to check if all the prompts
	 * can be answered automatically without prompts.
	 */
	if (!ask && !answer_yes && !answer_no)
		return;

	if (answer_no)
		prompt->answer = PROMPT_ANSWER_NO;
	else if (answer_yes)
		prompt->answer = PROMPT_ANSWER_YES;

	/*
	 * Mostly historical messages.  Other messages above could be moved
	 * here to separate the answer logic from the messages.
	 */

	if ((prompt->type & (PROMPT_PVCREATE_DEV_SIZE | PROMPT_PVCREATE_PV_IN_VG)) &&
	    (prompt->answer == PROMPT_ANSWER_NO))
		log_error("%s: physical volume not initialized.", pvname);

	if ((prompt->type & PROMPT_PVREMOVE_PV_IN_VG) &&
	    (prompt->answer == PROMPT_ANSWER_NO))
		log_error("%s: physical volume label not removed.", pvname);

	if ((prompt->type & PROMPT_PVREMOVE_PV_IN_VG) &&
	    (prompt->answer == PROMPT_ANSWER_YES) &&
	    (pp->force == DONT_PROMPT_OVERRIDE))
		log_warn("WARNING: Wiping physical volume label from %s of volume group \"%s\".", pvname, vgname);
}

static struct pvcreate_device *_pvcreate_list_find_dev(struct dm_list *devices, struct device *dev)
{
	struct pvcreate_device *pd;

	dm_list_iterate_items(pd, devices) {
		if (pd->dev == dev)
			return pd;
	}

	return NULL;
}

static struct pvcreate_device *_pvcreate_list_find_name(struct dm_list *devices, const char *name)
{
	struct pvcreate_device *pd;

	dm_list_iterate_items(pd, devices) {
		if (!strcmp(pd->name, name))
			return pd;
	}

	return NULL;
}

/*
 * If this function decides that a arg_devices entry cannot be used, but the
 * command might be able to continue without it, then it moves that entry from
 * arg_devices to arg_fail.
 *
 * If this function decides that an arg_devices entry could be used (possibly
 * requiring a prompt), then it moves the entry from arg_devices to arg_process.
 *
 * Any arg_devices entries that are not moved to arg_fail or arg_process were
 * not found.  The caller will decide if the command can continue if any
 * arg_devices entries were not found, or if any were moved to arg_fail.
 *
 * This check does not need to look at PVs in foreign, shared or clustered VGs.
 * If pvcreate/vgcreate/vgextend specifies a device in a
 * foreign/shared/clustered VG, that VG will not be processed by this function,
 * and the arg will be reported as not found.
 */
static int _pvcreate_check_single(struct cmd_context *cmd,
				  struct volume_group *vg,
				  struct physical_volume *pv,
				  struct processing_handle *handle)
{
	struct pvcreate_params *pp = (struct pvcreate_params *) handle->custom_handle;
	struct pvcreate_device *pd;
	struct pvcreate_prompt *prompt;
	uint64_t size = 0;
	uint64_t new_size = 0;
	int need_size_prompt = 0;
	int need_vg_prompt = 0;
	int found = 0;

	if (!pv->dev)
		return 1;

	/*
	 * Check if one of the command args in arg_devices
	 * matches this device.
	 */
	dm_list_iterate_items(pd, &pp->arg_devices) {
		if (pd->dev != pv->dev)
			continue;

		if (pv->dev->pvid[0])
			strncpy(pd->pvid, pv->dev->pvid, ID_LEN);
		found = 1;
		break;
	}

	/*
	 * Check if the uuid specified for the new PV is used by another PV.
	 */
	if (!found && pv->dev && pp->uuid_str && id_equal(&pv->id, &pp->pva.id)) {
		log_error("UUID %s already in use on \"%s\".", pp->uuid_str, pv_dev_name(pv));
		pp->check_failed = 1;
		return 0;
	}

	if (!found)
		return 1;

	log_debug("Checking pvcreate arg %s which has existing PVID: %.32s.",
		  pv_dev_name(pv), pv->dev->pvid[0] ? pv->dev->pvid : "<none>");


	/*
	 * Don't allow using a device with duplicates.
	 */
	if (lvmcache_pvid_in_unchosen_duplicates(pd->dev->pvid)) {
		log_error("Cannot use device %s with duplicates.", pd->name);
		dm_list_move(&pp->arg_fail, &pd->list);
		return 1;
	}

	/*
	 * What kind of device is this: an orphan PV, an uninitialized/unused
	 * device, a PV used in a VG.
	 */
	if (vg && !is_orphan_vg(vg->name)) {
		/* Device is a PV used in a VG. */
		log_debug("Found pvcreate arg %s: pv is used in %s.", pd->name, vg->name);
		pd->is_vg_pv = 1;
		pd->vg_name = dm_pool_strdup(cmd->mem, vg->name);
	} else if (vg && is_orphan_vg(vg->name)) {
		if (is_used_pv(pv)) {
			/* Device is used in an unknown VG. */
			log_debug("Found pvcreate arg %s: PV is used in unknown VG.", pd->name);
			pd->is_used_unknown_pv = 1;
		} else {
			/* Device is an orphan PV. */
			log_debug("Found pvcreate arg %s: PV is orphan in %s.", pd->name, vg->name);
			pd->is_orphan_pv = 1;
		}

		pp->orphan_vg_name = FMT_TEXT_ORPHAN_VG_NAME;
	} else {
		log_debug("Found pvcreate arg %s: device is not a PV.", pd->name);
		/* Device is not a PV. */
		pd->is_not_pv = 1;
	}

	if (arg_is_set(cmd, setphysicalvolumesize_ARG)) {
		new_size = arg_uint64_value(cmd, setphysicalvolumesize_ARG, UINT64_C(0));

		if (!dev_get_size(pv->dev, &size)) {
			log_error("Can't get device size of %s.", pv_dev_name(pv));
			dm_list_move(&pp->arg_fail, &pd->list);
			return 1;
		}

		if (new_size != size)
			need_size_prompt = 1;
	}

	/*
	 * pvcreate is being run on this device, and it's not a PV,
	 * or is an orphan PV.  Neither case requires a prompt.
	 * Or, pvcreate is being run on this device, but the device
	 * is already a PV in a VG.  A prompt or force option is required
	 * to use it.
	 */
	if (pd->is_orphan_pv || pd->is_not_pv)
		need_vg_prompt = 0;
	else
		need_vg_prompt = 1;

	if (!need_size_prompt && !need_vg_prompt) {
		pd->dev = pv->dev;
		dm_list_move(&pp->arg_process, &pd->list);
		return 1;
	}

	if (!(prompt = dm_pool_zalloc(cmd->mem, sizeof(*prompt)))) {
		log_error("prompt alloc failed.");
		pp->check_failed = 1;
		return 0;
	}
	prompt->dev = pd->dev;
	prompt->pv_name = dm_pool_strdup(cmd->mem, pd->name);
	prompt->size = size;
	prompt->new_size = new_size;

	if (pd->is_used_unknown_pv)
		prompt->vg_name_unknown = 1;
	else if (need_vg_prompt)
		prompt->vg_name = dm_pool_strdup(cmd->mem, vg->name);

	if (need_size_prompt)
		prompt->type |= PROMPT_PVCREATE_DEV_SIZE;

	if (need_vg_prompt)
		prompt->type |= PROMPT_PVCREATE_PV_IN_VG;

	dm_list_add(&pp->prompts, &prompt->list);
	pd->dev = pv->dev;
	dm_list_move(&pp->arg_process, &pd->list);

	return 1;
}

/*
 * This repeats the first check -- devices should be found, and should not have
 * changed since the first check.  If they were changed/used while the orphans
 * lock was not held (during prompting), then they can't be used any more and
 * are moved to arg_fail.  If they are not found by this loop, that also
 * disqualifies them from being used.  Each arg_confirm entry that's found and
 * is ok, is moved to arg_process.  Those not found will remain in arg_confirm.
 *
 * This check does not need to look in foreign/shared/clustered VGs.  If a
 * device from arg_confirm was used in a foreign/shared/clustered VG during the
 * prompts, then it will not be found during this check.
 */

static int _pv_confirm_single(struct cmd_context *cmd,
			      struct volume_group *vg,
			      struct physical_volume *pv,
			      struct processing_handle *handle)
{
	struct pvcreate_params *pp = (struct pvcreate_params *) handle->custom_handle;
	struct pvcreate_device *pd;
	int found = 0;

	dm_list_iterate_items(pd, &pp->arg_confirm) {
		if (pd->dev != pv->dev)
			continue;
		found = 1;
		break;
	}

	if (!found)
		return 1;

	/*
	 * What kind of device is this: an orphan PV, an uninitialized/unused
	 * device, a PV used in a VG.
	 */
	if (vg && !is_orphan_vg(vg->name)) {
		/* Device is a PV used in a VG. */

		if (pd->is_orphan_pv || pd->is_not_pv || pd->is_used_unknown_pv) {
			/* In check_single it was an orphan or unused. */
			goto fail;
		}

		if (pd->is_vg_pv && pd->vg_name && strcmp(pd->vg_name, vg->name)) {
			/* In check_single it was in a different VG. */
			goto fail;
		}
	} else if (is_orphan(pv)) {
		/* Device is an orphan PV. */

		if (pd->is_not_pv) {
			/* In check_single it was not a PV. */
			goto fail;
		}

		if (pd->is_vg_pv) {
			/* In check_single it was in a VG. */
			goto fail;
		}

		if (is_used_pv(pv) != pd->is_used_unknown_pv) {
			/* In check_single it was different. */
			goto fail;
		}
	} else {
		/* Device is not a PV. */
		if (pd->is_orphan_pv || pd->is_used_unknown_pv) {
			/* In check_single it was an orphan PV. */
			goto fail;
		}

		if (pd->is_vg_pv) {
			/* In check_single it was in a VG. */
			goto fail;
		}
	}

	/* Device is unchanged from check_single. */
	dm_list_move(&pp->arg_process, &pd->list);

	return 1;

fail:
	log_error("Cannot use device %s: it changed during prompt.", pd->name);
	dm_list_move(&pp->arg_fail, &pd->list);

	return 1;
}

static int _pvremove_check_single(struct cmd_context *cmd,
				  struct volume_group *vg,
				  struct physical_volume *pv,
				  struct processing_handle *handle)
{
	struct pvcreate_params *pp = (struct pvcreate_params *) handle->custom_handle;
	struct pvcreate_device *pd;
	struct pvcreate_prompt *prompt;
	int found = 0;

	if (!pv->dev)
		return 1;

	/*
	 * Check if one of the command args in arg_devices
	 * matches this device.
	 */
	dm_list_iterate_items(pd, &pp->arg_devices) {
		if (pd->dev != pv->dev)
			continue;

		if (pv->dev->pvid[0])
			strncpy(pd->pvid, pv->dev->pvid, ID_LEN);
		found = 1;
		break;
	}

	if (!found)
		return 1;

	log_debug("Checking device %s for pvremove %.32s.",
		  pv_dev_name(pv), pv->dev->pvid[0] ? pv->dev->pvid : "");


	/*
	 * Is there a pv here already?
	 * If not, this is an error unless you used -f.
	 */
	if (!lvmcache_has_dev_info(pv->dev)) {
		if (pp->force) {
			dm_list_move(&pp->arg_process, &pd->list);
			return 1;
		} else {
			pd->is_not_pv = 1;
		}
	}

	/*
	 * What kind of device is this: an orphan PV, an uninitialized/unused
	 * device, a PV used in a VG.
	 */

	if (pd->is_not_pv) {
		/* Device is not a PV. */
		log_debug("Found pvremove arg %s: device is not a PV.", pd->name);

	} else if (vg && !is_orphan_vg(vg->name)) {
		/* Device is a PV used in a VG. */
		log_debug("Found pvremove arg %s: pv is used in %s.", pd->name, vg->name);
		pd->is_vg_pv = 1;
		pd->vg_name = dm_pool_strdup(cmd->mem, vg->name);

	} else if (vg && is_orphan_vg(vg->name)) {
		if (is_used_pv(pv)) {
			/* Device is used in an unknown VG. */
			log_debug("Found pvremove arg %s: pv is used in unknown VG.", pd->name);
			pd->is_used_unknown_pv = 1;
		} else {
			/* Device is an orphan PV. */
			log_debug("Found pvremove arg %s: pv is orphan in %s.", pd->name, vg->name);
			pd->is_orphan_pv = 1;
		}

		pp->orphan_vg_name = FMT_TEXT_ORPHAN_VG_NAME;
	} else {
		/* FIXME: is it possible to reach here? */
		log_debug("Found pvremove arg %s: device is not a PV.", pd->name);
		/* Device is not a PV. */
		pd->is_not_pv = 1;
	}

	if (pd->is_not_pv) {
		pd->dev = pv->dev;
		log_error("No PV found on device %s.", pd->name);
		dm_list_move(&pp->arg_fail, &pd->list);
		return 1;
	}

	/*
	 * pvremove is being run on this device, and it's not a PV,
	 * or is an orphan PV.  Neither case requires a prompt.
	 */
	if (pd->is_orphan_pv) {
		pd->dev = pv->dev;
		dm_list_move(&pp->arg_process, &pd->list);
		return 1;
	}

	/*
	 * pvremove is being run on this device, but the device is in a VG.
	 * A prompt or force option is required to use it.
	 */

	if (!(prompt = dm_pool_zalloc(cmd->mem, sizeof(*prompt)))) {
		log_error("prompt alloc failed.");
		pp->check_failed = 1;
		return 0;
	}
	prompt->dev = pd->dev;
	prompt->pv_name = dm_pool_strdup(cmd->mem, pd->name);
	if (pd->is_used_unknown_pv)
		prompt->vg_name_unknown = 1;
	else
		prompt->vg_name = dm_pool_strdup(cmd->mem, vg->name);
	prompt->type |= PROMPT_PVREMOVE_PV_IN_VG;
	dm_list_add(&pp->prompts, &prompt->list);

	pd->dev = pv->dev;
	dm_list_move(&pp->arg_process, &pd->list);

	return 1;
}

/*
 * This can be used by pvcreate, vgcreate and vgextend to create PVs.  The
 * callers need to set up the pvcreate_each_params structure based on command
 * line args.  This includes the pv_names field which specifies the devices to
 * create PVs on.
 *
 * This uses process_each_pv() and should be called from a high level in the
 * command -- the same level at which other instances of process_each are
 * called.
 *
 * This function returns 0 (failed) if the caller requires all specified
 * devices to be created, and any of those devices are not found, or any of
 * them cannot be created.
 *
 * This function returns 1 (success) if the caller requires all specified
 * devices to be created, and all are created, or if the caller does not
 * require all specified devices to be created and one or more were created.
 *
 * When this function returns 1 (success), it returns to the caller with the
 * VG_ORPHANS write lock held.
 */

int pvcreate_each_device(struct cmd_context *cmd,
			 struct processing_handle *handle,
			 struct pvcreate_params *pp)
{
	struct pvcreate_device *pd, *pd2;
	struct pvcreate_prompt *prompt, *prompt2;
	struct physical_volume *pv;
	struct volume_group *orphan_vg;
	struct dm_list remove_duplicates;
	struct dm_list arg_sort;
	struct dm_list rescan_devs;
	struct pv_list *pvl;
	struct pv_list *vgpvl;
	struct device_list *devl;
	const char *pv_name;
	int must_use_all = (cmd->cname->flags & MUST_USE_ALL_ARGS);
	int found;
	unsigned i;

	set_pv_notify(cmd);

	dm_list_init(&remove_duplicates);
	dm_list_init(&arg_sort);
	dm_list_init(&rescan_devs);

	handle->custom_handle = pp;

	/*
	 * Create a list entry for each name arg.
	 */
	for (i = 0; i < pp->pv_count; i++) {
		dm_unescape_colons_and_at_signs(pp->pv_names[i], NULL, NULL);

		pv_name = pp->pv_names[i];

		if (!(pd = dm_pool_zalloc(cmd->mem, sizeof(*pd)))) {
			log_error("alloc failed.");
			return 0;
		}

		if (!(pd->name = dm_pool_strdup(cmd->mem, pv_name))) {
			log_error("strdup failed.");
			return 0;
		}

		dm_list_add(&pp->arg_devices, &pd->list);
	}

	if (!lock_vol(cmd, VG_ORPHANS, LCK_VG_WRITE, NULL)) {
		log_error("Can't get lock for orphan PVs.");
		return 0;
	}

	/*
	 * Scan before calling process_each_pv so we can set up the PV args
	 * first.  We can then skip the scan that would normally occur at the
	 * beginning of process_each_pv.
	 */
	lvmcache_label_scan(cmd);

	/*
	 * Translate arg names into struct device's.
	 */
	dm_list_iterate_items(pd, &pp->arg_devices)
		pd->dev = dev_cache_get(cmd, pd->name, cmd->filter);

	/*
	 * Use process_each_pv to search all existing PVs and devices.
	 *
	 * This is a slightly different way to use process_each_pv, because the
	 * command args (arg_devices) are not being processed directly by
	 * process_each_pv (argc and argv are not passed).  Instead,
	 * process_each_pv is processing all existing PVs and devices, and the
	 * single function is matching each of those against the command args
	 * (arg_devices).
	 *
	 * If an arg_devices entry is found during process_each_pv, it's moved
	 * to arg_process if it can be used, or arg_fail if it cannot be used.
	 * If it's added to arg_process but needs a prompt or force option, then
	 * a corresponding prompt entry is added to pp->prompts.
	 */
	process_each_pv(cmd, 0, NULL, NULL, 1, PROCESS_SKIP_SCAN | PROCESS_SKIP_ORPHAN_LOCK,
			handle, pp->is_remove ? _pvremove_check_single : _pvcreate_check_single);

	/*
	 * A fatal error was found while checking.
	 */
	if (pp->check_failed)
		goto_bad;

	/*
	 * Special case: pvremove -ff is allowed to clear a duplicate device in
	 * the unchosen duplicates list.  PVs in the unchosen duplicates list
	 * won't be found by normal process_each searches -- they are not in
	 * lvmcache and can't be processed normally.  We save them here and
	 * erase them below without going through the normal processing code.
	 */
	if (pp->is_remove && (pp->force == DONT_PROMPT_OVERRIDE) &&
	   !dm_list_empty(&pp->arg_devices) && lvmcache_found_duplicate_pvs()) {
		dm_list_iterate_items_safe(pd, pd2, &pp->arg_devices) {
			if (lvmcache_dev_is_unchosen_duplicate(pd->dev)) {
				log_debug("Found pvremove arg %s: device is a duplicate.", pd->name);
				dm_list_move(&remove_duplicates, &pd->list);
			}
		}
	}

	/*
	 * Check if all arg_devices were found by process_each_pv.
	 */
	dm_list_iterate_items(pd, &pp->arg_devices)
		log_error("Device %s %s.", pd->name, dev_cache_filtered_reason(pd->name));

	/*
	 * Can the command continue if some specified devices were not found?
	 */
	if (!dm_list_empty(&pp->arg_devices) && must_use_all)
		goto_bad;

	/*
	 * Can the command continue if some specified devices cannot be used?
	 */
	if (!dm_list_empty(&pp->arg_fail) && must_use_all)
		goto_bad;

	/*
	 * The command cannot continue if there are no devices to process.
	 */
	if (dm_list_empty(&pp->arg_process) && dm_list_empty(&remove_duplicates)) {
		log_debug("No devices to process.");
		goto bad;
	}

	/*
	 * Clear any prompts that have answers without asking the user. 
	 */
	dm_list_iterate_items_safe(prompt, prompt2, &pp->prompts) {
		_check_pvcreate_prompt(cmd, pp, prompt, 0);

		switch (prompt->answer) {
		case PROMPT_ANSWER_YES:
			/* The PV can be used, leave it on arg_process. */
			dm_list_del(&prompt->list);
			break;
		case PROMPT_ANSWER_NO:
			/* The PV cannot be used, remove it from arg_process. */
			if ((pd = _pvcreate_list_find_dev(&pp->arg_process, prompt->dev)))
				dm_list_move(&pp->arg_fail, &pd->list);
			dm_list_del(&prompt->list);
			break;
		}
	}

	if (!dm_list_empty(&pp->arg_fail) && must_use_all)
		goto_bad;

	/*
	 * If no remaining prompts need a user response, then keep orphans
	 * locked and go directly to the create steps. 
	 */
	if (dm_list_empty(&pp->prompts))
		goto do_command;

	/*
	 * Prompts require asking the user and make take some time, during
	 * which we don't want to block other commands.  So, release the lock
	 * to prevent blocking other commands while we wait.  After a response
	 * from the user, reacquire the lock, verify that the PVs were not used
	 * during the wait, then do the create steps.
	 */
	unlock_vg(cmd, NULL, VG_ORPHANS);

	/*
	 * Process prompts that require asking the user.  The orphans lock is
	 * not held, so there's no harm in waiting for a user to respond.
	 */
	dm_list_iterate_items_safe(prompt, prompt2, &pp->prompts) {
		_check_pvcreate_prompt(cmd, pp, prompt, 1);

		switch (prompt->answer) {
		case PROMPT_ANSWER_YES:
			/* The PV can be used, leave it on arg_process. */
			dm_list_del(&prompt->list);
			break;
		case PROMPT_ANSWER_NO:
			/* The PV cannot be used, remove it from arg_process. */
			if ((pd = _pvcreate_list_find_dev(&pp->arg_process, prompt->dev)))
				dm_list_move(&pp->arg_fail, &pd->list);
			dm_list_del(&prompt->list);
			break;
		}

		if (!dm_list_empty(&pp->arg_fail) && must_use_all)
			goto_out;

		if (sigint_caught())
			goto_out;

		if (prompt->abort_command)
			goto_out;
	}

	/*
	 * Reacquire the lock that was released above before waiting, then
	 * check again that the devices can still be used.  If the second loop
	 * finds them changed, or can't find them any more, then they aren't
	 * used.
	 */
	if (!lock_vol(cmd, VG_ORPHANS, LCK_VG_WRITE, NULL)) {
		log_error("Can't get lock for orphan PVs.");
		goto out;
	}

	lvmcache_label_scan(cmd);

	/*
	 * The device args began on the arg_devices list, then the first check
	 * loop moved those entries to arg_process as they were found.  Devices
	 * not found during the first loop are not being used, and remain on
	 * arg_devices.
	 * 
	 * Now, the arg_process entries are moved to arg_confirm, and the second
	 * check loop moves them back to arg_process as they are found and are
	 * unchanged.  Like the first loop, the second loop moves an entry to
	 * arg_fail if it cannot be used.  After the second loop, any devices
	 * remaining on arg_confirm were not found and are not used.
	 */
	dm_list_splice(&pp->arg_confirm, &pp->arg_process);

	process_each_pv(cmd, 0, NULL, NULL, 1, PROCESS_SKIP_SCAN | PROCESS_SKIP_ORPHAN_LOCK,
			handle, _pv_confirm_single);

	dm_list_iterate_items(pd, &pp->arg_confirm)
		log_error("Device %s %s.", pd->name, dev_cache_filtered_reason(pd->name));

	/* Some devices were not found during the second check. */
	if (!dm_list_empty(&pp->arg_confirm) && must_use_all)
		goto_bad;

	/* Some devices changed during the second check. */
	if (!dm_list_empty(&pp->arg_fail) && must_use_all)
		goto_bad;

	if (dm_list_empty(&pp->arg_process)) {
		log_debug("No devices to process.");
		goto bad;
	}

do_command:

	dm_list_iterate_items(pd, &pp->arg_process) {
		if (!(devl = dm_pool_zalloc(cmd->mem, sizeof(*devl))))
			goto bad;
		devl->dev = pd->dev;
		dm_list_add(&rescan_devs, &devl->list);
	}

	log_debug("Rescanning devices with exclusive open");
	if (!label_scan_devs_excl(&rescan_devs)) {
		log_debug("Failed to rescan devs excl");
		goto bad;
	}

	/*
	 * Reorder arg_process entries to match the original order of args.
	 */
	dm_list_splice(&arg_sort, &pp->arg_process);
	for (i = 0; i < pp->pv_count; i++) {
		if ((pd = _pvcreate_list_find_name(&arg_sort, pp->pv_names[i])))
			dm_list_move(&pp->arg_process, &pd->list);
	}

	if (pp->is_remove)
		dm_list_splice(&pp->arg_remove, &pp->arg_process);
	else
		dm_list_splice(&pp->arg_create, &pp->arg_process);

	/*
	 * Wipe signatures on devices being created.
	 */
	dm_list_iterate_items_safe(pd, pd2, &pp->arg_create) {
		label_scan_open(pd->dev);

		log_verbose("Wiping signatures on new PV %s.", pd->name);

		if (!wipe_known_signatures(cmd, pd->dev, pd->name, TYPE_LVM1_MEMBER | TYPE_LVM2_MEMBER,
					    0, pp->yes, pp->force, &pd->wiped)) {
			dm_list_move(&pp->arg_fail, &pd->list);
		}

		if (sigint_caught())
			goto_bad;
	}

	if (!dm_list_empty(&pp->arg_fail) && must_use_all)
		goto_bad;

	/*
	 * Find existing orphan PVs that vgcreate or vgextend want to use.
	 * "preserve_existing" means that the command wants to use existing PVs
	 * and not recreate a new PV on top of an existing PV.
	 */
	if (pp->preserve_existing && pp->orphan_vg_name) {
		log_debug("Using existing orphan PVs in %s.", pp->orphan_vg_name);

		if (!(orphan_vg = vg_read_orphans(cmd, 0, pp->orphan_vg_name))) {
			log_error("Cannot read orphans VG %s.", pp->orphan_vg_name);
			goto bad;
		}

		dm_list_iterate_items_safe(pd, pd2, &pp->arg_create) {
			if (!pd->is_orphan_pv)
				continue;

			if (!(pvl = dm_pool_alloc(cmd->mem, sizeof(*pvl)))) {
				log_error("alloc pvl failed.");
				dm_list_move(&pp->arg_fail, &pd->list);
				continue;
			}

			found = 0;
			dm_list_iterate_items(vgpvl, &orphan_vg->pvs) {
				if (vgpvl->pv->dev == pd->dev) {
					found = 1;
					break;
				}
			}

			if (found) {
				log_debug("Using existing orphan PV %s.", pv_dev_name(vgpvl->pv));
				pvl->pv = vgpvl->pv;
				dm_list_add(&pp->pvs, &pvl->list);
			} else {
				log_error("Failed to find PV %s", pd->name);
				dm_list_move(&pp->arg_fail, &pd->list);
			}
		}
	}

	/*
	 * Create PVs on devices.  Either create a new PV on top of an existing
	 * one (e.g. for pvcreate), or create a new PV on a device that is not
	 * a PV.
	 */
	dm_list_iterate_items_safe(pd, pd2, &pp->arg_create) {
		/* Using existing orphan PVs is covered above. */
		if (pp->preserve_existing && pd->is_orphan_pv)
			continue;

		if (!dm_list_empty(&pp->arg_fail) && must_use_all)
			break;

		if (!(pvl = dm_pool_alloc(cmd->mem, sizeof(*pvl)))) {
			log_error("alloc pvl failed.");
			dm_list_move(&pp->arg_fail, &pd->list);
			continue;
		}

		pv_name = pd->name;

		label_scan_open_excl(pd->dev);

		log_debug("Creating a new PV on %s.", pv_name);

		if (!(pv = pv_create(cmd, pd->dev, &pp->pva))) {
			log_error("Failed to setup physical volume \"%s\".", pv_name);
			dm_list_move(&pp->arg_fail, &pd->list);
			continue;
		}

		log_verbose("Set up physical volume for \"%s\" with %" PRIu64
			    " available sectors.", pv_name, pv_size(pv));


		if (!label_remove(pv->dev)) {
			log_error("Failed to wipe existing label on %s.", pv_name);
			dm_list_move(&pp->arg_fail, &pd->list);
			continue;
		}

		if (pp->zero) {
			log_verbose("Zeroing start of device %s.", pv_name);

			if (!dev_write_zeros(pv->dev, 0, 2048)) {
				log_error("%s not wiped: aborting.", pv_name);
				dm_list_move(&pp->arg_fail, &pd->list);
				continue;
			}
		}

		log_verbose("Writing physical volume data to disk \"%s\".", pv_name);

		if (!pv_write(cmd, pv, 0)) {
			log_error("Failed to write physical volume \"%s\".", pv_name);
			dm_list_move(&pp->arg_fail, &pd->list);
			continue;
		}

		log_print_unless_silent("Physical volume \"%s\" successfully created.",
					pv_name);

		pvl->pv = pv;
		dm_list_add(&pp->pvs, &pvl->list);
	}

	/*
	 * Remove PVs from devices for pvremove.
	 */
	dm_list_iterate_items_safe(pd, pd2, &pp->arg_remove) {
		if (!label_remove(pd->dev)) {
			log_error("Failed to wipe existing label(s) on %s.", pd->name);
			dm_list_move(&pp->arg_fail, &pd->list);
			continue;
		}

		log_print_unless_silent("Labels on physical volume \"%s\" successfully wiped.",
					pd->name);
	}

	/*
	 * Special case: pvremove duplicate PVs (also see above).
	 */
	dm_list_iterate_items_safe(pd, pd2, &remove_duplicates) {
		if (!label_remove(pd->dev)) {
			log_error("Failed to wipe existing label(s) on %s.", pd->name);
			dm_list_move(&pp->arg_fail, &pd->list);
			continue;
		}

		lvmcache_remove_unchosen_duplicate(pd->dev);

		log_print_unless_silent("Labels on physical volume \"%s\" successfully wiped.",
					pd->name);
	}

	/*
	 * Don't keep devs open excl in bcache because the excl will prevent
	 * using that dev elsewhere.
	 */
	dm_list_iterate_items(devl, &rescan_devs)
		label_scan_invalidate(devl->dev);

	dm_list_iterate_items(pd, &pp->arg_fail)
		log_debug("%s: command failed for %s.",
			  cmd->command->name, pd->name);

	if (!dm_list_empty(&pp->arg_fail))
		goto_bad;

	/*
	 * Returns with VG_ORPHANS write lock held because vgcreate and
	 * vgextend want to use the newly created PVs.
	 */
	return 1;

bad:
	unlock_vg(cmd, NULL, VG_ORPHANS);
out:
	return 0;
}
