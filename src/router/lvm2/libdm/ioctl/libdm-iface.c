/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2013 Red Hat, Inc. All rights reserved.
 *
 * This file is part of the device-mapper userspace tools.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libdm/misc/dmlib.h"
#include "libdm-targets.h"
#include "libdm-common.h"

#include <stddef.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/utsname.h>
#include <limits.h>

#ifdef __linux__
#  include "libdm/misc/kdev_t.h"
#  include <linux/limits.h>
#else
#  define MAJOR(x) major((x))
#  define MINOR(x) minor((x))
#  define MKDEV(x,y) makedev(((dev_t)x),((dev_t)y))
#endif

#include "libdm/misc/dm-ioctl.h"

/*
 * Ensure build compatibility.  
 * The hard-coded versions here are the highest present 
 * in the _cmd_data arrays.
 */

#if !((DM_VERSION_MAJOR == 4 && DM_VERSION_MINOR >= 6))
#error The version of dm-ioctl.h included is incompatible.
#endif

/* FIXME This should be exported in device-mapper.h */
#define DM_NAME "device-mapper"

#define PROC_MISC "/proc/misc"
#define PROC_DEVICES "/proc/devices"
#define MISC_NAME "misc"

#define NUMBER_OF_MAJORS 4096

/*
 * Static minor number assigned since kernel version 2.6.36.
 * The original definition is in kernel's include/linux/miscdevice.h.
 * This number is also visible in modules.devname exported by depmod
 * utility (support included in module-init-tools version >= 3.12).
 */
#define MAPPER_CTRL_MINOR 236
#define MISC_MAJOR 10

/* dm major version no for running kernel */
static unsigned _dm_version = DM_VERSION_MAJOR;
static unsigned _dm_version_minor = 0;
static unsigned _dm_version_patchlevel = 0;
static int _log_suppress = 0;
static struct dm_timestamp *_dm_ioctl_timestamp = NULL;

/*
 * If the kernel dm driver only supports one major number
 * we store it in _dm_device_major.  Otherwise we indicate
 * which major numbers have been claimed by device-mapper
 * in _dm_bitset.
 */
static unsigned _dm_multiple_major_support = 1;
static dm_bitset_t _dm_bitset = NULL;
static uint32_t _dm_device_major = 0;

static int _control_fd = -1;
static int _hold_control_fd_open = 0;
static int _version_checked = 0;
static int _version_ok = 1;
static unsigned _ioctl_buffer_double_factor = 0;

const int _dm_compat = 0;

/* *INDENT-OFF* */
static struct cmd_data _cmd_data_v4[] = {
	{"create",	DM_DEV_CREATE,		{4, 0, 0}},
	{"reload",	DM_TABLE_LOAD,		{4, 0, 0}},
	{"remove",	DM_DEV_REMOVE,		{4, 0, 0}},
	{"remove_all",	DM_REMOVE_ALL,		{4, 0, 0}},
	{"suspend",	DM_DEV_SUSPEND,		{4, 0, 0}},
	{"resume",	DM_DEV_SUSPEND,		{4, 0, 0}},
	{"info",	DM_DEV_STATUS,		{4, 0, 0}},
	{"deps",	DM_TABLE_DEPS,		{4, 0, 0}},
	{"rename",	DM_DEV_RENAME,		{4, 0, 0}},
	{"version",	DM_VERSION,		{4, 0, 0}},
	{"status",	DM_TABLE_STATUS,	{4, 0, 0}},
	{"table",	DM_TABLE_STATUS,	{4, 0, 0}},
	{"waitevent",	DM_DEV_WAIT,		{4, 0, 0}},
	{"names",	DM_LIST_DEVICES,	{4, 0, 0}},
	{"clear",	DM_TABLE_CLEAR,		{4, 0, 0}},
	{"mknodes",	DM_DEV_STATUS,		{4, 0, 0}},
#ifdef DM_LIST_VERSIONS
	{"versions",	DM_LIST_VERSIONS,	{4, 1, 0}},
#endif
#ifdef DM_TARGET_MSG
	{"message",	DM_TARGET_MSG,		{4, 2, 0}},
#endif
#ifdef DM_DEV_SET_GEOMETRY
	{"setgeometry",	DM_DEV_SET_GEOMETRY,	{4, 6, 0}},
#endif
#ifdef DM_DEV_ARM_POLL
	{"armpoll",	DM_DEV_ARM_POLL,	{4, 36, 0}},
#endif
};
/* *INDENT-ON* */

#define ALIGNMENT 8

/* FIXME Rejig library to record & use errno instead */
#ifndef DM_EXISTS_FLAG
#  define DM_EXISTS_FLAG 0x00000004
#endif

static char *_align(char *ptr, unsigned int a)
{
	register unsigned long agn = --a;

	return (char *) (((unsigned long) ptr + agn) & ~agn);
}

#ifdef DM_IOCTLS
static unsigned _kernel_major = 0;
static unsigned _kernel_minor = 0;
static unsigned _kernel_release = 0;

static int _uname(void)
{
	static int _uts_set = 0;
	struct utsname _uts;
	int parts;

	if (_uts_set)
		return 1;

	if (uname(&_uts)) {
		log_error("uname failed: %s", strerror(errno));
		return 0;
	}

	parts = sscanf(_uts.release, "%u.%u.%u",
		       &_kernel_major, &_kernel_minor, &_kernel_release);

	/* Kernels with a major number of 2 always had 3 parts. */
	if (parts < 1 || (_kernel_major < 3 && parts < 3)) {
		log_error("Could not determine kernel version used.");
		return 0;
	}

	_uts_set = 1;
	return 1;
}

int get_uname_version(unsigned *major, unsigned *minor, unsigned *release)
{
	if (!_uname())
		return_0;

	*major = _kernel_major;
	*minor = _kernel_minor;
	*release = _kernel_release;

	return 1;
}
/*
 * Set number to NULL to populate _dm_bitset - otherwise first
 * match is returned.
 * Returns:
 * 	0 - error
 * 	1 - success - number found
 * 	2 - success - number not found (only if require_module_loaded=0)
 */
static int _get_proc_number(const char *file, const char *name,
			    uint32_t *number, int require_module_loaded)
{
	FILE *fl;
	char nm[256];
	char *line = NULL;
	size_t len;
	uint32_t num;

	if (!(fl = fopen(file, "r"))) {
		log_sys_error("fopen", file);
		return 0;
	}

	while (getline(&line, &len, fl) != -1) {
		if (sscanf(line, "%d %255s\n", &num, &nm[0]) == 2) {
			if (!strcmp(name, nm)) {
				if (number) {
					*number = num;
					if (fclose(fl))
						log_sys_error("fclose", file);
					free(line);
					return 1;
				}
				dm_bit_set(_dm_bitset, num);
			}
		}
	}
	if (fclose(fl))
		log_sys_error("fclose", file);
	free(line);

	if (number) {
		if (require_module_loaded) {
			log_error("%s: No entry for %s found", file, name);
			return 0;
		}

		return 2;
	}

	return 1;
}

static int _control_device_number(uint32_t *major, uint32_t *minor)
{
	if (!_get_proc_number(PROC_DEVICES, MISC_NAME, major, 1) ||
	    !_get_proc_number(PROC_MISC, DM_NAME, minor, 1)) {
		*major = 0;
		return 0;
	}

	return 1;
}

/*
 * Returns 1 if it exists on returning; 0 if it doesn't; -1 if it's wrong.
 */
static int _control_exists(const char *control, uint32_t major, uint32_t minor)
{
	struct stat buf;

	if (stat(control, &buf) < 0) {
		if (errno != ENOENT)
			log_sys_error("stat", control);
		return 0;
	}

	if (!S_ISCHR(buf.st_mode)) {
		log_verbose("%s: Wrong inode type", control);
		if (!unlink(control))
			return 0;
		log_sys_error("unlink", control);
		return -1;
	}

	if (major && buf.st_rdev != MKDEV(major, minor)) {
		log_verbose("%s: Wrong device number: (%u, %u) instead of "
			    "(%u, %u)", control,
			    MAJOR(buf.st_mode), MINOR(buf.st_mode),
			    major, minor);
		if (!unlink(control))
			return 0;
		log_sys_error("unlink", control);
		return -1;
	}

	return 1;
}

static int _create_control(const char *control, uint32_t major, uint32_t minor)
{
	int ret;
	mode_t old_umask;

	/*
	 * Return if the control already exists with intended major/minor
	 * or there's an error unlinking an apparently incorrect one.
	 */
	ret = _control_exists(control, major, minor);
	if (ret == -1)
		return_0;	/* Failed to unlink existing incorrect node */
	if (ret)
		return 1;	/* Already exists and correct */

	(void) dm_prepare_selinux_context(dm_dir(), S_IFDIR);
	old_umask = umask(DM_DEV_DIR_UMASK);
	ret = dm_create_dir(dm_dir());
	umask(old_umask);
	(void) dm_prepare_selinux_context(NULL, 0);

	if (!ret)
		return_0;

	log_verbose("Creating device %s (%u, %u)", control, major, minor);

	(void) dm_prepare_selinux_context(control, S_IFCHR);
	old_umask = umask(DM_CONTROL_NODE_UMASK);
	if (mknod(control, S_IFCHR | S_IRUSR | S_IWUSR,
		  MKDEV(major, minor)) < 0)  {
		log_sys_error("mknod", control);
		ret = 0;
	}
	umask(old_umask);
	(void) dm_prepare_selinux_context(NULL, 0);

	return ret;
}
#endif

/*
 * FIXME Update bitset in long-running process if dm claims new major numbers.
 */
/*
 * If require_module_loaded=0, caller is responsible to check
 * whether _dm_device_major or _dm_bitset is really set. If
 * it's not, it means the module is not loaded.
 */
static int _create_dm_bitset(int require_module_loaded)
{
	int r;

#ifdef DM_IOCTLS
	if (_dm_bitset || _dm_device_major)
		return 1;

	if (!_uname())
		return 0;

	/*
	 * 2.6 kernels are limited to one major number.
	 * Assume 2.4 kernels are patched not to.
	 * FIXME Check _dm_version and _dm_version_minor if 2.6 changes this.
	 */
	if (KERNEL_VERSION(_kernel_major, _kernel_minor, _kernel_release) >=
	    KERNEL_VERSION(2, 6, 0))
		_dm_multiple_major_support = 0;

	if (!_dm_multiple_major_support) {
		if (!_get_proc_number(PROC_DEVICES, DM_NAME, &_dm_device_major,
				      require_module_loaded))
			return 0;
		return 1;
	}

	/* Multiple major numbers supported */
	if (!(_dm_bitset = dm_bitset_create(NULL, NUMBER_OF_MAJORS)))
		return 0;

	r = _get_proc_number(PROC_DEVICES, DM_NAME, NULL, require_module_loaded);
	if (!r || r == 2) {
		dm_bitset_destroy(_dm_bitset);
		_dm_bitset = NULL;
		/*
		 * It's not an error if we didn't find anything and we
		 * didn't require module to be loaded at the same time.
		 */
		return r == 2;
	}

	return 1;
#else
	return 0;
#endif
}

int dm_is_dm_major(uint32_t major)
{
	if (!_create_dm_bitset(0))
		return 0;

	if (_dm_multiple_major_support) {
		if (!_dm_bitset)
			return 0;
		return dm_bit(_dm_bitset, major) ? 1 : 0;
	}

	if (!_dm_device_major)
		return 0;

	return (major == _dm_device_major) ? 1 : 0;
}

static void _close_control_fd(void)
{
	if (_control_fd != -1) {
		if (close(_control_fd) < 0)
			log_sys_error("close", "_control_fd");
		_control_fd = -1;
	}
}

#ifdef DM_IOCTLS
static int _open_and_assign_control_fd(const char *control)
{
	if ((_control_fd = open(control, O_RDWR)) < 0) {
		log_sys_error("open", control);
		return 0;
	}

	return 1;
}
#endif

static int _open_control(void)
{
#ifdef DM_IOCTLS
	char control[PATH_MAX];
	uint32_t major = MISC_MAJOR;
	uint32_t minor = MAPPER_CTRL_MINOR;

	if (_control_fd != -1)
		return 1;

	if (!_uname())
		return 0;

	if (dm_snprintf(control, sizeof(control), "%s/%s", dm_dir(), DM_CONTROL_NODE) < 0)
		goto_bad;

	/*
	 * Prior to 2.6.36 the minor number should be looked up in /proc.
	 */
	if ((KERNEL_VERSION(_kernel_major, _kernel_minor, _kernel_release) <
	     KERNEL_VERSION(2, 6, 36)) &&
	    !_control_device_number(&major, &minor))
		goto_bad;

	/*
	 * Create the node with correct major and minor if not already done.
	 * Udev may already have created /dev/mapper/control
	 * from the modules.devname file generated by depmod.
	 */
	if (!_create_control(control, major, minor))
		goto_bad;

	/*
	 * As of 2.6.36 kernels, the open can trigger autoloading dm-mod.
	 */
	if (!_open_and_assign_control_fd(control))
		goto_bad;
	
	if (!_create_dm_bitset(1)) {
		log_error("Failed to set up list of device-mapper major numbers");
		return 0;
	}

	return 1;

bad:
	log_error("Failure to communicate with kernel device-mapper driver.");
	if (!geteuid())
		log_error("Check that device-mapper is available in the kernel.");
	return 0;
#else
	return 1;
#endif
}

static void _dm_zfree_string(char *string)
{
	if (string) {
		memset(string, 0, strlen(string));
		asm volatile ("" ::: "memory"); /* Compiler barrier. */
		dm_free(string);
	}
}

static void _dm_zfree_dmi(struct dm_ioctl *dmi)
{
	if (dmi) {
		memset(dmi, 0, dmi->data_size);
		asm volatile ("" ::: "memory"); /* Compiler barrier. */
		dm_free(dmi);
	}
}

static void _dm_task_free_targets(struct dm_task *dmt)
{
	struct target *t, *n;

	for (t = dmt->head; t; t = n) {
		n = t->next;
		_dm_zfree_string(t->params);
		dm_free(t->type);
		dm_free(t);
	}

	dmt->head = dmt->tail = NULL;
}

void dm_task_destroy(struct dm_task *dmt)
{
	_dm_task_free_targets(dmt);
	_dm_zfree_dmi(dmt->dmi.v4);
	dm_free(dmt->dev_name);
	dm_free(dmt->mangled_dev_name);
	dm_free(dmt->newname);
	dm_free(dmt->message);
	dm_free(dmt->geometry);
	dm_free(dmt->uuid);
	dm_free(dmt->mangled_uuid);
	dm_free(dmt);
}

/*
 * Protocol Version 4 functions.
 */

int dm_task_get_driver_version(struct dm_task *dmt, char *version, size_t size)
{
	unsigned *v;

	if (!dmt->dmi.v4) {
		if (version)
			version[0] = '\0';
		return 0;
	}

	v = dmt->dmi.v4->version;
	_dm_version_minor = v[1];
	_dm_version_patchlevel = v[2];
	if (version &&
	    (snprintf(version, size, "%u.%u.%u", v[0], v[1], v[2]) < 0)) {
		log_error("Buffer for version is to short.");
		if (size > 0)
			version[0] = '\0';
		return 0;
	}

	return 1;
}

static int _check_version(char *version, size_t size, int log_suppress)
{
	struct dm_task *task;
	int r;

	if (!(task = dm_task_create(DM_DEVICE_VERSION))) {
		log_error("Failed to get device-mapper version");
		version[0] = '\0';
		return 0;
	}

	if (log_suppress)
		_log_suppress = 1;

	r = dm_task_run(task);
	if (!dm_task_get_driver_version(task, version, size))
		stack;
	dm_task_destroy(task);
	_log_suppress = 0;

	return r;
}

/*
 * Find out device-mapper's major version number the first time 
 * this is called and whether or not we support it.
 */
int dm_check_version(void)
{
	char libversion[64] = "", dmversion[64] = "";
	const char *compat = "";

	if (_version_checked)
		return _version_ok;

	_version_checked = 1;

	if (_check_version(dmversion, sizeof(dmversion), _dm_compat))
		return 1;

	if (!_dm_compat)
		goto_bad;

	log_verbose("device-mapper ioctl protocol version %u failed. "
		    "Trying protocol version 1.", _dm_version);
	_dm_version = 1;
	if (_check_version(dmversion, sizeof(dmversion), 0)) {
		log_verbose("Using device-mapper ioctl protocol version 1");
		return 1;
	}

	compat = "(compat)";

      bad:
	dm_get_library_version(libversion, sizeof(libversion));

	log_error("Incompatible libdevmapper %s%s and kernel driver %s.",
		  *libversion ? libversion : "(unknown version)", compat,
		  *dmversion ? dmversion : "(unknown version)");

	_version_ok = 0;
	return 0;
}

int dm_cookie_supported(void)
{
	return (dm_check_version() &&
		_dm_version >= 4 &&
		_dm_version_minor >= 15);
}

static int _dm_inactive_supported(void)
{
	int inactive_supported = 0;

	if (dm_check_version() && _dm_version >= 4) {
		if (_dm_version_minor >= 16)
			inactive_supported = 1; /* upstream */
		else if (_dm_version_minor == 11 &&
			 (_dm_version_patchlevel >= 6 &&
			  _dm_version_patchlevel <= 40)) {
			inactive_supported = 1; /* RHEL 5.7 */
		}
	}

	return inactive_supported;
}

int dm_message_supports_precise_timestamps(void)
{
	/*
	 * 4.32.0 supports "precise_timestamps" and "histogram:" options
	 * to @stats_create messages but lacks the ability to report
	 * these properties via a subsequent @stats_list: require at
	 * least 4.33.0 in order to use these features.
	 */
	if (dm_check_version() && _dm_version >= 4)
		if (_dm_version_minor >= 33)
			return 1;
	return 0;
}

void *dm_get_next_target(struct dm_task *dmt, void *next,
			 uint64_t *start, uint64_t *length,
			 char **target_type, char **params)
{
	struct target *t = (struct target *) next;

	if (!t)
		t = dmt->head;

	if (!t) {
		*start = 0;
		*length = 0;
		*target_type = 0;
		*params = 0;
		return NULL;
	}

	*start = t->start;
	*length = t->length;
	*target_type = t->type;
	*params = t->params;

	return t->next;
}

/* Unmarshall the target info returned from a status call */
static int _unmarshal_status(struct dm_task *dmt, struct dm_ioctl *dmi)
{
	char *outbuf = (char *) dmi + dmi->data_start;
	char *outptr = outbuf;
	uint32_t i;
	struct dm_target_spec *spec;

	_dm_task_free_targets(dmt);

	for (i = 0; i < dmi->target_count; i++) {
		spec = (struct dm_target_spec *) outptr;
		if (!dm_task_add_target(dmt, spec->sector_start,
					spec->length,
					spec->target_type,
					outptr + sizeof(*spec))) {
			return 0;
		}

		outptr = outbuf + spec->next;
	}

	return 1;
}

int dm_format_dev(char *buf, int bufsize, uint32_t dev_major,
		  uint32_t dev_minor)
{
	int r;

	if (bufsize < 8)
		return 0;

	r = snprintf(buf, (size_t) bufsize, "%u:%u", dev_major, dev_minor);
	if (r < 0 || r > bufsize - 1)
		return 0;

	return 1;
}

int dm_task_get_info(struct dm_task *dmt, struct dm_info *info)
{
	if (!dmt->dmi.v4)
		return 0;

	memset(info, 0, sizeof(*info));

	info->exists = dmt->dmi.v4->flags & DM_EXISTS_FLAG ? 1 : 0;
	if (!info->exists)
		return 1;

	info->suspended = dmt->dmi.v4->flags & DM_SUSPEND_FLAG ? 1 : 0;
	info->read_only = dmt->dmi.v4->flags & DM_READONLY_FLAG ? 1 : 0;
	info->live_table = dmt->dmi.v4->flags & DM_ACTIVE_PRESENT_FLAG ? 1 : 0;
	info->inactive_table = dmt->dmi.v4->flags & DM_INACTIVE_PRESENT_FLAG ?
	    1 : 0;
	info->deferred_remove = dmt->dmi.v4->flags & DM_DEFERRED_REMOVE;
	info->internal_suspend = (dmt->dmi.v4->flags & DM_INTERNAL_SUSPEND_FLAG) ? 1 : 0;
	info->target_count = dmt->dmi.v4->target_count;
	info->open_count = dmt->dmi.v4->open_count;
	info->event_nr = dmt->dmi.v4->event_nr;
	info->major = MAJOR(dmt->dmi.v4->dev);
	info->minor = MINOR(dmt->dmi.v4->dev);

	return 1;
}

uint32_t dm_task_get_read_ahead(const struct dm_task *dmt, uint32_t *read_ahead)
{
	const char *dev_name;

	*read_ahead = 0;

	if (!dmt->dmi.v4 || !(dmt->dmi.v4->flags & DM_EXISTS_FLAG))
		return 0;

	if (*dmt->dmi.v4->name)
		dev_name = dmt->dmi.v4->name;
	else if (!(dev_name = DEV_NAME(dmt))) {
		log_error("Get read ahead request failed: device name unrecorded.");
		return 0;
	}

	return get_dev_node_read_ahead(dev_name, MAJOR(dmt->dmi.v4->dev),
				       MINOR(dmt->dmi.v4->dev), read_ahead);
}

struct dm_deps *dm_task_get_deps(struct dm_task *dmt)
{
	return (struct dm_deps *) (((char *) dmt->dmi.v4) +
				   dmt->dmi.v4->data_start);
}

struct dm_names *dm_task_get_names(struct dm_task *dmt)
{
	return (struct dm_names *) (((char *) dmt->dmi.v4) +
				    dmt->dmi.v4->data_start);
}

struct dm_versions *dm_task_get_versions(struct dm_task *dmt)
{
	return (struct dm_versions *) (((char *) dmt->dmi.v4) +
				       dmt->dmi.v4->data_start);
}

const char *dm_task_get_message_response(struct dm_task *dmt)
{
	const char *start, *end;

	if (!(dmt->dmi.v4->flags & DM_DATA_OUT_FLAG))
		return NULL;

	start = (const char *) dmt->dmi.v4 + dmt->dmi.v4->data_start;
	end = (const char *) dmt->dmi.v4 + dmt->dmi.v4->data_size;

	if (end < start) {
		log_error(INTERNAL_ERROR "Corrupted message structure returned: start %d > end %d", (int)dmt->dmi.v4->data_start, (int)dmt->dmi.v4->data_size);
		return NULL;
	}

	if (!memchr(start, 0, end - start)) {
		log_error(INTERNAL_ERROR "Message response doesn't contain terminating NUL character");
		return NULL;
	}

	return start;
}

int dm_task_set_ro(struct dm_task *dmt)
{
	dmt->read_only = 1;
	return 1;
}

int dm_task_set_read_ahead(struct dm_task *dmt, uint32_t read_ahead,
			   uint32_t read_ahead_flags)
{
	dmt->read_ahead = read_ahead;
	dmt->read_ahead_flags = read_ahead_flags;

	return 1;
}

int dm_task_suppress_identical_reload(struct dm_task *dmt)
{
	dmt->suppress_identical_reload = 1;
	return 1;
}

int dm_task_set_add_node(struct dm_task *dmt, dm_add_node_t add_node)
{
	switch (add_node) {
	case DM_ADD_NODE_ON_RESUME:
	case DM_ADD_NODE_ON_CREATE:
		dmt->add_node = add_node;
		return 1;
	default:
		log_error("Unknown add node parameter");
		return 0;
	}
}

int dm_task_set_newuuid(struct dm_task *dmt, const char *newuuid)
{
	dm_string_mangling_t mangling_mode = dm_get_name_mangling_mode();
	char mangled_uuid[DM_UUID_LEN];
	int r = 0;

	if (strlen(newuuid) >= DM_UUID_LEN) {
		log_error("Uuid \"%s\" too long", newuuid);
		return 0;
	}

	if (!check_multiple_mangled_string_allowed(newuuid, "new UUID", mangling_mode))
		return_0;

	if (mangling_mode != DM_STRING_MANGLING_NONE &&
	    (r = mangle_string(newuuid, "new UUID", strlen(newuuid), mangled_uuid,
			       sizeof(mangled_uuid), mangling_mode)) < 0) {
		log_error("Failed to mangle new device UUID \"%s\"", newuuid);
		return 0;
	}

	if (r) {
		log_debug_activation("New device uuid mangled [%s]: %s --> %s",
				     mangling_mode == DM_STRING_MANGLING_AUTO ? "auto" : "hex",
				     newuuid, mangled_uuid);
		newuuid = mangled_uuid;
	}

	dm_free(dmt->newname);
	if (!(dmt->newname = dm_strdup(newuuid))) {
		log_error("dm_task_set_newuuid: strdup(%s) failed", newuuid);
		return 0;
	}
	dmt->new_uuid = 1;

	return 1;
}

int dm_task_set_message(struct dm_task *dmt, const char *message)
{
	dm_free(dmt->message);
	if (!(dmt->message = dm_strdup(message))) {
		log_error("dm_task_set_message: strdup failed");
		return 0;
	}

	return 1;
}

int dm_task_set_sector(struct dm_task *dmt, uint64_t sector)
{
	dmt->sector = sector;

	return 1;
}

int dm_task_set_geometry(struct dm_task *dmt, const char *cylinders, const char *heads,
			 const char *sectors, const char *start)
{
	dm_free(dmt->geometry);
	if (dm_asprintf(&(dmt->geometry), "%s %s %s %s",
			cylinders, heads, sectors, start) < 0) {
		log_error("dm_task_set_geometry: sprintf failed");
		return 0;
	}

	return 1;
}

int dm_task_no_flush(struct dm_task *dmt)
{
	dmt->no_flush = 1;

	return 1;
}

int dm_task_no_open_count(struct dm_task *dmt)
{
	dmt->no_open_count = 1;

	return 1;
}

int dm_task_skip_lockfs(struct dm_task *dmt)
{
	dmt->skip_lockfs = 1;

	return 1;
}

int dm_task_secure_data(struct dm_task *dmt)
{
	dmt->secure_data = 1;

	return 1;
}

int dm_task_retry_remove(struct dm_task *dmt)
{
	dmt->retry_remove = 1;

	return 1;
}

int dm_task_deferred_remove(struct dm_task *dmt)
{
	dmt->deferred_remove = 1;

	return 1;
}

int dm_task_query_inactive_table(struct dm_task *dmt)
{
	dmt->query_inactive_table = 1;

	return 1;
}

int dm_task_set_event_nr(struct dm_task *dmt, uint32_t event_nr)
{
	dmt->event_nr = event_nr;

	return 1;
}

int dm_task_set_record_timestamp(struct dm_task *dmt)
{
	if (!_dm_ioctl_timestamp)
		_dm_ioctl_timestamp = dm_timestamp_alloc();

	if (!_dm_ioctl_timestamp)
		return_0;

	dmt->record_timestamp = 1;

	return 1;
}

struct dm_timestamp *dm_task_get_ioctl_timestamp(struct dm_task *dmt)
{
	return dmt->record_timestamp ? _dm_ioctl_timestamp : NULL;
}

struct target *create_target(uint64_t start, uint64_t len, const char *type,
			     const char *params)
{
	struct target *t;

	if (strlen(type) >= DM_MAX_TYPE_NAME) {
		log_error("Target type name %s is too long.", type);
		return NULL;
	}

	if (!(t = dm_zalloc(sizeof(*t)))) {
		log_error("create_target: malloc(%" PRIsize_t ") failed",
			  sizeof(*t));
		return NULL;
	}

	if (!(t->params = dm_strdup(params))) {
		log_error("create_target: strdup(params) failed");
		goto bad;
	}

	if (!(t->type = dm_strdup(type))) {
		log_error("create_target: strdup(type) failed");
		goto bad;
	}

	t->start = start;
	t->length = len;
	return t;

      bad:
	_dm_zfree_string(t->params);
	dm_free(t->type);
	dm_free(t);
	return NULL;
}

static char *_add_target(struct target *t, char *out, char *end)
{
	char *out_sp = out;
	struct dm_target_spec sp;
	size_t sp_size = sizeof(struct dm_target_spec);
	unsigned int backslash_count = 0;
	int len;
	char *pt;

	if (strlen(t->type) >= sizeof(sp.target_type)) {
		log_error("Target type name %s is too long.", t->type);
		return NULL;
	}

	sp.status = 0;
	sp.sector_start = t->start;
	sp.length = t->length;
	strncpy(sp.target_type, t->type, sizeof(sp.target_type) - 1);
	sp.target_type[sizeof(sp.target_type) - 1] = '\0';

	out += sp_size;
	pt = t->params;

	while (*pt)
		if (*pt++ == '\\')
			backslash_count++;
	len = strlen(t->params) + backslash_count;

	if ((out >= end) || (out + len + 1) >= end) {
		log_error("Ran out of memory building ioctl parameter");
		return NULL;
	}

	if (backslash_count) {
		/* replace "\" with "\\" */
		pt = t->params;
		do {
			if (*pt == '\\')
				*out++ = '\\';
			*out++ = *pt++;
		} while (*pt);
		*out++ = '\0';
	}
	else {
		strcpy(out, t->params);
		out += len + 1;
	}

	/* align next block */
	out = _align(out, ALIGNMENT);

	sp.next = out - out_sp;
	memcpy(out_sp, &sp, sp_size);

	return out;
}

static int _lookup_dev_name(uint64_t dev, char *buf, size_t len)
{
	struct dm_names *names;
	unsigned next = 0;
	struct dm_task *dmt;
	int r = 0;
 
	if (!(dmt = dm_task_create(DM_DEVICE_LIST)))
		return 0;
 
	if (!dm_task_run(dmt))
		goto out;

	if (!(names = dm_task_get_names(dmt)))
		goto out;
 
	if (!names->dev)
		goto out;
 
	do {
		names = (struct dm_names *)((char *) names + next);
		if (names->dev == dev) {
			strncpy(buf, names->name, len);
			r = 1;
			break;
		}
		next = names->next;
	} while (next);

      out:
	dm_task_destroy(dmt);
	return r;
}

static int _add_params(int type)
{
	switch (type) {
	case DM_DEVICE_REMOVE_ALL:
	case DM_DEVICE_CREATE:
	case DM_DEVICE_REMOVE:
	case DM_DEVICE_SUSPEND:
	case DM_DEVICE_STATUS:
	case DM_DEVICE_CLEAR:
	case DM_DEVICE_ARM_POLL:
		return 0; /* IOCTL_FLAGS_NO_PARAMS in drivers/md/dm-ioctl.c */
	default:
		return 1;
	}
}

static struct dm_ioctl *_flatten(struct dm_task *dmt, unsigned repeat_count)
{
	const size_t min_size = 16 * 1024;
	const int (*version)[3];

	struct dm_ioctl *dmi;
	struct target *t;
	struct dm_target_msg *tmsg;
	size_t len = sizeof(struct dm_ioctl);
	char *b, *e;
	int count = 0;

	if (_add_params(dmt->type))
		for (t = dmt->head; t; t = t->next) {
			len += sizeof(struct dm_target_spec);
			len += strlen(t->params) + 1 + ALIGNMENT;
			count++;
		}
	else if (dmt->head)
		log_debug_activation(INTERNAL_ERROR "dm '%s' ioctl should not define parameters.",
				     _cmd_data_v4[dmt->type].name);

	if (count && (dmt->sector || dmt->message)) {
		log_error("targets and message are incompatible");
		return NULL;
	}

	if (count && dmt->newname) {
		log_error("targets and rename are incompatible");
		return NULL;
	}

	if (count && dmt->geometry) {
		log_error("targets and geometry are incompatible");
		return NULL;
	}

	if (dmt->newname && (dmt->sector || dmt->message)) {
		log_error("message and rename are incompatible");
		return NULL;
	}

	if (dmt->newname && dmt->geometry) {
		log_error("geometry and rename are incompatible");
		return NULL;
	}

	if (dmt->geometry && (dmt->sector || dmt->message)) {
		log_error("geometry and message are incompatible");
		return NULL;
	}

	if (dmt->sector && !dmt->message) {
		log_error("message is required with sector");
		return NULL;
	}

	if (dmt->newname)
		len += strlen(dmt->newname) + 1;

	if (dmt->message)
		len += sizeof(struct dm_target_msg) + strlen(dmt->message) + 1;

	if (dmt->geometry)
		len += strlen(dmt->geometry) + 1;

	/*
	 * Give len a minimum size so that we have space to store
	 * dependencies or status information.
	 */
	if (len < min_size)
		len = min_size;

	/* Increase buffer size if repeating because buffer was too small */
	while (repeat_count--)
		len *= 2;

	if (!(dmi = dm_zalloc(len)))
		return NULL;

	version = &_cmd_data_v4[dmt->type].version;

	dmi->version[0] = (*version)[0];
	dmi->version[1] = (*version)[1];
	dmi->version[2] = (*version)[2];

	dmi->data_size = len;
	dmi->data_start = sizeof(struct dm_ioctl);

	if (dmt->minor >= 0) {
		if (!_dm_multiple_major_support && dmt->allow_default_major_fallback &&
		    dmt->major != (int) _dm_device_major) {
			log_verbose("Overriding major number of %d "
				    "with %u for persistent device.",
				    dmt->major, _dm_device_major);
			dmt->major = _dm_device_major;
		}

		if (dmt->major <= 0) {
			log_error("Missing major number for persistent device.");
			goto bad;
		}

		dmi->flags |= DM_PERSISTENT_DEV_FLAG;
		dmi->dev = MKDEV(dmt->major, dmt->minor);
	}

	/* Does driver support device number referencing? */
	if (_dm_version_minor < 3 && !DEV_NAME(dmt) && !DEV_UUID(dmt) && dmi->dev) {
		if (!_lookup_dev_name(dmi->dev, dmi->name, sizeof(dmi->name))) {
			log_error("Unable to find name for device (%" PRIu32
				  ":%" PRIu32 ")", dmt->major, dmt->minor);
			goto bad;
		}
		log_verbose("device (%" PRIu32 ":%" PRIu32 ") is %s "
			    "for compatibility with old kernel",
			    dmt->major, dmt->minor, dmi->name);
	}

	/* FIXME Until resume ioctl supplies name, use dev_name for readahead */
	if (DEV_NAME(dmt) && (dmt->type != DM_DEVICE_RESUME || dmt->minor < 0 ||
			      dmt->major < 0))
		strncpy(dmi->name, DEV_NAME(dmt), sizeof(dmi->name));

	if (DEV_UUID(dmt))
		strncpy(dmi->uuid, DEV_UUID(dmt), sizeof(dmi->uuid));

	if (dmt->type == DM_DEVICE_SUSPEND)
		dmi->flags |= DM_SUSPEND_FLAG;
	if (dmt->no_flush) {
		if (_dm_version_minor < 12)
			log_verbose("No flush flag unsupported by kernel. "
				    "Buffers will be flushed.");
		else
			dmi->flags |= DM_NOFLUSH_FLAG;
	}
	if (dmt->read_only)
		dmi->flags |= DM_READONLY_FLAG;
	if (dmt->skip_lockfs)
		dmi->flags |= DM_SKIP_LOCKFS_FLAG;
	if (dmt->deferred_remove && (dmt->type == DM_DEVICE_REMOVE || dmt->type == DM_DEVICE_REMOVE_ALL))
		dmi->flags |= DM_DEFERRED_REMOVE;

	if (dmt->secure_data) {
		if (_dm_version_minor < 20)
			log_verbose("Secure data flag unsupported by kernel. "
				    "Buffers will not be wiped after use.");
		dmi->flags |= DM_SECURE_DATA_FLAG;
	}
	if (dmt->query_inactive_table) {
		if (!_dm_inactive_supported())
			log_warn("WARNING: Inactive table query unsupported "
				 "by kernel.  It will use live table.");
		dmi->flags |= DM_QUERY_INACTIVE_TABLE_FLAG;
	}
	if (dmt->new_uuid) {
		if (_dm_version_minor < 19) {
			log_error("WARNING: Setting UUID unsupported by "
				  "kernel.  Aborting operation.");
			goto bad;
		}
		dmi->flags |= DM_UUID_FLAG;
	}

	dmi->target_count = count;
	dmi->event_nr = dmt->event_nr;

	b = (char *) (dmi + 1);
	e = (char *) dmi + len;

	if (_add_params(dmt->type))
		for (t = dmt->head; t; t = t->next)
			if (!(b = _add_target(t, b, e)))
				goto_bad;

	if (dmt->newname)
		strcpy(b, dmt->newname);

	if (dmt->message) {
		tmsg = (struct dm_target_msg *) b;
		tmsg->sector = dmt->sector;
		strcpy(tmsg->message, dmt->message);
	}

	if (dmt->geometry)
		strcpy(b, dmt->geometry);

	return dmi;

      bad:
	_dm_zfree_dmi(dmi);
	return NULL;
}

static int _process_mapper_dir(struct dm_task *dmt)
{
	struct dirent *dirent;
	DIR *d;
	const char *dir;
	int r = 1;

	dir = dm_dir();
	if (!(d = opendir(dir))) {
		log_sys_error("opendir", dir);
		return 0;
	}

	while ((dirent = readdir(d))) {
		if (!strcmp(dirent->d_name, ".") ||
		    !strcmp(dirent->d_name, "..") ||
		    !strcmp(dirent->d_name, "control"))
			continue;
		if (!dm_task_set_name(dmt, dirent->d_name)) {
			r = 0;
			stack;
			continue; /* try next name */
		}
		if (!dm_task_run(dmt)) {
			r = 0;
			stack;  /* keep going */
		}
	}

	if (closedir(d))
		log_sys_error("closedir", dir);

	return r;
}

static int _process_all_v4(struct dm_task *dmt)
{
	struct dm_task *task;
	struct dm_names *names;
	unsigned next = 0;
	int r = 1;

	if (!(task = dm_task_create(DM_DEVICE_LIST)))
		return 0;

	if (!dm_task_run(task)) {
		r = 0;
		goto out;
	}

	if (!(names = dm_task_get_names(task))) {
		r = 0;
		goto out;
	}

	if (!names->dev)
		goto out;

	do {
		names = (struct dm_names *)((char *) names + next);
		if (!dm_task_set_name(dmt, names->name)) {
			r = 0;
			goto out;
		}
		if (!dm_task_run(dmt))
			r = 0;
		next = names->next;
	} while (next);

      out:
	dm_task_destroy(task);
	return r;
}

static int _mknodes_v4(struct dm_task *dmt)
{
	(void) _process_mapper_dir(dmt);

	return _process_all_v4(dmt);
}

/*
 * If an operation that uses a cookie fails, decrement the
 * semaphore instead of udev.
 */
static int _udev_complete(struct dm_task *dmt)
{
	uint16_t base;

	if (dmt->cookie_set &&
	    (base = dmt->event_nr & ~DM_UDEV_FLAGS_MASK))
		/* strip flags from the cookie and use cookie magic instead */
		return dm_udev_complete(base | (DM_COOKIE_MAGIC <<
						DM_UDEV_FLAGS_SHIFT));

	return 1;
}

#ifdef DM_IOCTLS
static int _check_uevent_generated(struct dm_ioctl *dmi)
{
	if (!dm_check_version() ||
	    _dm_version < 4 ||
	    _dm_version_minor < 17)
		/* can't check, assume uevent is generated */
		return 1;

	return dmi->flags & DM_UEVENT_GENERATED_FLAG;
}
#endif

static int _create_and_load_v4(struct dm_task *dmt)
{
	struct dm_task *task;
	int r;
	uint32_t cookie;

	/* Use new task struct to create the device */
	if (!(task = dm_task_create(DM_DEVICE_CREATE))) {
		_udev_complete(dmt);
		return_0;
	}

	/* Copy across relevant fields */
	if (dmt->dev_name && !dm_task_set_name(task, dmt->dev_name))
		goto_bad;

	if (dmt->uuid && !dm_task_set_uuid(task, dmt->uuid))
		goto_bad;

	task->major = dmt->major;
	task->minor = dmt->minor;
	task->uid = dmt->uid;
	task->gid = dmt->gid;
	task->mode = dmt->mode;
	/* FIXME: Just for udev_check in dm_task_run. Can we avoid this? */
	task->event_nr = dmt->event_nr & DM_UDEV_FLAGS_MASK;
	task->cookie_set = dmt->cookie_set;
	task->add_node = dmt->add_node;

	if (!dm_task_run(task))
		goto_bad;

	dm_task_destroy(task);

	/* Next load the table */
	if (!(task = dm_task_create(DM_DEVICE_RELOAD))) {
		stack;
		_udev_complete(dmt);
		goto revert;
	}

	/* Copy across relevant fields */
	if (dmt->dev_name && !dm_task_set_name(task, dmt->dev_name)) {
		stack;
		dm_task_destroy(task);
		_udev_complete(dmt);
		goto revert;
	}

	task->read_only = dmt->read_only;
	task->head = dmt->head;
	task->tail = dmt->tail;
	task->secure_data = dmt->secure_data;

	r = dm_task_run(task);

	task->head = NULL;
	task->tail = NULL;
	dm_task_destroy(task);

	if (!r) {
		stack;
		_udev_complete(dmt);
		goto revert;
	}

	/* Use the original structure last so the info will be correct */
	dmt->type = DM_DEVICE_RESUME;
	dm_free(dmt->uuid);
	dmt->uuid = NULL;
	dm_free(dmt->mangled_uuid);
	dmt->mangled_uuid = NULL;
	_dm_task_free_targets(dmt);

	if (dm_task_run(dmt))
		return 1;

      revert:
	dmt->type = DM_DEVICE_REMOVE;
	dm_free(dmt->uuid);
	dmt->uuid = NULL;
	dm_free(dmt->mangled_uuid);
	dmt->mangled_uuid = NULL;
	_dm_task_free_targets(dmt);

	/*
	 * Also udev-synchronize "remove" dm task that is a part of this revert!
	 * But only if the original dm task was supposed to be synchronized.
	 */
	if (dmt->cookie_set) {
		cookie = (dmt->event_nr & ~DM_UDEV_FLAGS_MASK) |
			 (DM_COOKIE_MAGIC << DM_UDEV_FLAGS_SHIFT);
		if (!dm_task_set_cookie(dmt, &cookie,
					(dmt->event_nr & DM_UDEV_FLAGS_MASK) >>
					DM_UDEV_FLAGS_SHIFT))
			stack; /* keep going */
	}

	if (!dm_task_run(dmt))
		log_error("Failed to revert device creation.");

	return 0;

      bad:
	dm_task_destroy(task);
	_udev_complete(dmt);

	return 0;
}

uint64_t dm_task_get_existing_table_size(struct dm_task *dmt)
{
	return dmt->existing_table_size;
}

static int _reload_with_suppression_v4(struct dm_task *dmt)
{
	struct dm_task *task;
	struct target *t1, *t2;
	size_t len;
	int r;

	/* New task to get existing table information */
	if (!(task = dm_task_create(DM_DEVICE_TABLE))) {
		log_error("Failed to create device-mapper task struct");
		return 0;
	}

	/* Copy across relevant fields */
	if (dmt->dev_name && !dm_task_set_name(task, dmt->dev_name)) {
		dm_task_destroy(task);
		return 0;
	}

	if (dmt->uuid && !dm_task_set_uuid(task, dmt->uuid)) {
		dm_task_destroy(task);
		return 0;
	}

	task->major = dmt->major;
	task->minor = dmt->minor;

	r = dm_task_run(task);

	if (!r) {
		dm_task_destroy(task);
		return r;
	}

	/* Store existing table size */
	t2 = task->head;
	while (t2 && t2->next)
		t2 = t2->next;
	dmt->existing_table_size = t2 ? t2->start + t2->length : 0;

	if (((task->dmi.v4->flags & DM_READONLY_FLAG) ? 1 : 0) != dmt->read_only)
		goto no_match;

	t1 = dmt->head;
	t2 = task->head;

	while (t1 && t2) {
		len = strlen(t2->params);
		while (len-- > 0 && t2->params[len] == ' ')
			t2->params[len] = '\0';
		if ((t1->start != t2->start) ||
		    (t1->length != t2->length) ||
		    (strcmp(t1->type, t2->type)) ||
		    (strcmp(t1->params, t2->params)))
			goto no_match;
		t1 = t1->next;
		t2 = t2->next;
	}
	
	if (!t1 && !t2) {
		dmt->dmi.v4 = task->dmi.v4;
		task->dmi.v4 = NULL;
		dm_task_destroy(task);
		return 1;
	}

no_match:
	dm_task_destroy(task);

	/* Now do the original reload */
	dmt->suppress_identical_reload = 0;
	r = dm_task_run(dmt);

	return r;
}

static int _check_children_not_suspended_v4(struct dm_task *dmt, uint64_t device)
{
	struct dm_task *task;
	struct dm_info info;
	struct dm_deps *deps;
	int r = 0;
	uint32_t i;

	/* Find dependencies */
	if (!(task = dm_task_create(DM_DEVICE_DEPS)))
		return 0;

	/* Copy across or set relevant fields */
	if (device) {
		task->major = MAJOR(device);
		task->minor = MINOR(device);
	} else {
		if (dmt->dev_name && !dm_task_set_name(task, dmt->dev_name))
			goto out;

		if (dmt->uuid && !dm_task_set_uuid(task, dmt->uuid))
			goto out;

		task->major = dmt->major;
		task->minor = dmt->minor;
	}

	task->uid = dmt->uid;
	task->gid = dmt->gid;
	task->mode = dmt->mode;
	/* FIXME: Just for udev_check in dm_task_run. Can we avoid this? */
	task->event_nr = dmt->event_nr & DM_UDEV_FLAGS_MASK;
	task->cookie_set = dmt->cookie_set;
	task->add_node = dmt->add_node;
	
	if (!(r = dm_task_run(task)))
		goto out;

	if (!dm_task_get_info(task, &info) || !info.exists)
		goto out;

	/*
	 * Warn if any of the devices this device depends upon are already
	 * suspended: I/O could become trapped between the two devices.
	 */
	if (info.suspended) {
		if (!device)
			log_debug_activation("Attempting to suspend a device that is already suspended "
					     "(%u:%u)", info.major, info.minor);
		else
			log_error(INTERNAL_ERROR "Attempt to suspend device %s%s%s%.0d%s%.0d%s%s"
				  "that uses already-suspended device (%u:%u)", 
				  DEV_NAME(dmt) ? : "", DEV_UUID(dmt) ? : "",
				  dmt->major > 0 ? "(" : "",
				  dmt->major > 0 ? dmt->major : 0,
				  dmt->major > 0 ? ":" : "",
				  dmt->minor > 0 ? dmt->minor : 0,
				  dmt->major > 0 && dmt->minor == 0 ? "0" : "",
				  dmt->major > 0 ? ") " : "",
				  info.major, info.minor);

		/* No need for further recursion */
		r = 1;
		goto out;
	}

	if (!(deps = dm_task_get_deps(task)))
		goto out;

	for (i = 0; i < deps->count; i++) {
		/* Only recurse with dm devices */
		if (MAJOR(deps->device[i]) != _dm_device_major)
			continue;

		if (!_check_children_not_suspended_v4(task, deps->device[i]))
			goto out;
	}

	r = 1;

out:
	dm_task_destroy(task);

	return r;
}

static int _suspend_with_validation_v4(struct dm_task *dmt)
{
	/* Avoid recursion */
	dmt->enable_checks = 0;

	/*
	 * Ensure we can't leave any I/O trapped between suspended devices.
	 */
	if (!_check_children_not_suspended_v4(dmt, 0))
		return 0;

	/* Finally, perform the original suspend. */
	return dm_task_run(dmt);
}

static const char *_sanitise_message(char *message)
{
	const char *sanitised_message = message ?: "";

	/* FIXME: Check for whitespace variations. */
	/* This traps what cryptsetup sends us. */
	if (message && !strncasecmp(message, "key set", 7))
		sanitised_message = "key set";

	return sanitised_message;
}

#ifdef DM_IOCTLS
static int _do_dm_ioctl_unmangle_string(char *str, const char *str_name,
					char *buf, size_t buf_size,
					dm_string_mangling_t mode)
{
	int r;

	if (mode == DM_STRING_MANGLING_NONE)
		return 1;

	if (!check_multiple_mangled_string_allowed(str, str_name, mode))
		return_0;

	if ((r = unmangle_string(str, str_name, strlen(str), buf, buf_size, mode)) < 0) {
		log_debug_activation("_do_dm_ioctl_unmangle_string: failed to "
				     "unmangle %s \"%s\"", str_name, str);
		return 0;
	}

	if (r)
		memcpy(str, buf, strlen(buf) + 1);

	return 1;
}

static int _dm_ioctl_unmangle_names(int type, struct dm_ioctl *dmi)
{
	char buf[DM_NAME_LEN];
	struct dm_names *names;
	unsigned next = 0;
	char *name;
	int r = 1;

	if ((name = dmi->name))
		r = _do_dm_ioctl_unmangle_string(name, "name", buf, sizeof(buf),
						 dm_get_name_mangling_mode());

	if (type == DM_DEVICE_LIST &&
	    ((names = ((struct dm_names *) ((char *)dmi + dmi->data_start)))) &&
	    names->dev) {
		do {
			names = (struct dm_names *)((char *) names + next);
			r = _do_dm_ioctl_unmangle_string(names->name, "name",
							 buf, sizeof(buf),
							 dm_get_name_mangling_mode());
			next = names->next;
		} while (next);
	}

	return r;
}

static int _dm_ioctl_unmangle_uuids(int type, struct dm_ioctl *dmi)
{
	char buf[DM_UUID_LEN];
	char *uuid = dmi->uuid;

	if (uuid)
		return _do_dm_ioctl_unmangle_string(uuid, "UUID", buf, sizeof(buf),
						    dm_get_name_mangling_mode());

	return 1;
}
#endif

static struct dm_ioctl *_do_dm_ioctl(struct dm_task *dmt, unsigned command,
				     unsigned buffer_repeat_count,
				     unsigned retry_repeat_count,
				     int *retryable)
{
	struct dm_ioctl *dmi;
	int ioctl_with_uevent;
	int r;

	dmt->ioctl_errno = 0;

	dmi = _flatten(dmt, buffer_repeat_count);
	if (!dmi) {
		log_error("Couldn't create ioctl argument.");
		return NULL;
	}

	if (dmt->type == DM_DEVICE_TABLE)
		dmi->flags |= DM_STATUS_TABLE_FLAG;

	dmi->flags |= DM_EXISTS_FLAG;	/* FIXME */

	if (dmt->no_open_count)
		dmi->flags |= DM_SKIP_BDGET_FLAG;

	ioctl_with_uevent = dmt->type == DM_DEVICE_RESUME ||
			    dmt->type == DM_DEVICE_REMOVE ||
			    dmt->type == DM_DEVICE_RENAME;

	if (ioctl_with_uevent && dm_cookie_supported()) {
		/*
		 * Always mark events coming from libdevmapper as
		 * "primary sourced". This is needed to distinguish
		 * any spurious events so we can act appropriately.
		 * This needs to be applied even when udev_sync is
		 * not used because udev flags could be used alone.
		 */
		dmi->event_nr |= DM_UDEV_PRIMARY_SOURCE_FLAG <<
				 DM_UDEV_FLAGS_SHIFT;

		/*
		 * Prevent udev vs. libdevmapper race when processing nodes
		 * and symlinks. This can happen when the udev rules are
		 * installed and udev synchronisation code is enabled in
		 * libdevmapper but the software using libdevmapper does not
		 * make use of it (by not calling dm_task_set_cookie before).
		 * We need to instruct the udev rules not to be applied at
		 * all in this situation so we can gracefully fallback to
		 * libdevmapper's node and symlink creation code.
		 */
		if (!dmt->cookie_set && dm_udev_get_sync_support()) {
			log_debug_activation("Cookie value is not set while trying to call %s "
					     "ioctl. Please, consider using libdevmapper's udev "
					     "synchronisation interface or disable it explicitly "
					     "by calling dm_udev_set_sync_support(0).",
					     dmt->type == DM_DEVICE_RESUME ? "DM_DEVICE_RESUME" :
					     dmt->type == DM_DEVICE_REMOVE ? "DM_DEVICE_REMOVE" :
									     "DM_DEVICE_RENAME");
			log_debug_activation("Switching off device-mapper and all subsystem related "
					     "udev rules. Falling back to libdevmapper node creation.");
			/*
			 * Disable general dm and subsystem rules but keep
			 * dm disk rules if not flagged out explicitly before.
			 * We need /dev/disk content for the software that expects it.
			*/
			dmi->event_nr |= (DM_UDEV_DISABLE_DM_RULES_FLAG |
					  DM_UDEV_DISABLE_SUBSYSTEM_RULES_FLAG) <<
					 DM_UDEV_FLAGS_SHIFT;
		}
	}

	log_debug_activation("dm %s %s%s %s%s%s %s%.0d%s%.0d%s"
			     "%s[ %s%s%s%s%s%s%s%s%s] %.0" PRIu64 " %s [%u] (*%u)",
			     _cmd_data_v4[dmt->type].name,
			     dmt->new_uuid ? "UUID " : "",
			     dmi->name, dmi->uuid, dmt->newname ? " " : "",
			     dmt->newname ? dmt->newname : "",
			     dmt->major > 0 ? "(" : "",
			     dmt->major > 0 ? dmt->major : 0,
			     dmt->major > 0 ? ":" : "",
			     dmt->minor > 0 ? dmt->minor : 0,
			     dmt->major > 0 && dmt->minor == 0 ? "0" : "",
			     dmt->major > 0 ? ") " : "",
			     dmt->no_open_count ? "noopencount " : "opencount ",
			     dmt->no_flush ? "noflush " : "flush ",
			     dmt->read_only ? "readonly " : "",
			     dmt->skip_lockfs ? "skiplockfs " : "",
			     dmt->retry_remove ? "retryremove " : "",
			     dmt->deferred_remove ? "deferredremove " : "",
			     dmt->secure_data ? "securedata " : "",
			     dmt->query_inactive_table ? "inactive " : "",
			     dmt->enable_checks ? "enablechecks " : "",
			     dmt->sector, _sanitise_message(dmt->message),
			     dmi->data_size, retry_repeat_count);
#ifdef DM_IOCTLS
	r = ioctl(_control_fd, command, dmi);

	if (dmt->record_timestamp)
		if (!dm_timestamp_get(_dm_ioctl_timestamp))
			stack;

	if (r < 0 && dmt->expected_errno != errno) {
		dmt->ioctl_errno = errno;
		if (dmt->ioctl_errno == ENXIO && ((dmt->type == DM_DEVICE_INFO) ||
						  (dmt->type == DM_DEVICE_MKNODES) ||
						  (dmt->type == DM_DEVICE_STATUS)))
			dmi->flags &= ~DM_EXISTS_FLAG;	/* FIXME */
		else {
			if (_log_suppress || dmt->ioctl_errno == EINTR)
				log_verbose("device-mapper: %s ioctl on %s %s%s%.0d%s%.0d%s%s "
					    "failed: %s",
					    _cmd_data_v4[dmt->type].name,
					    dmi->name, dmi->uuid,
					    dmt->major > 0 ? "(" : "",
					    dmt->major > 0 ? dmt->major : 0,
					    dmt->major > 0 ? ":" : "",
					    dmt->minor > 0 ? dmt->minor : 0,
					    dmt->major > 0 && dmt->minor == 0 ? "0" : "",
					    dmt->major > 0 ? ")" : "",
					    strerror(dmt->ioctl_errno));
			else
				log_error("device-mapper: %s ioctl on %s %s%s%.0d%s%.0d%s%s "
					  "failed: %s",
					  _cmd_data_v4[dmt->type].name,
					  dmi->name, dmi->uuid,
					  dmt->major > 0 ? "(" : "",
					  dmt->major > 0 ? dmt->major : 0,
					  dmt->major > 0 ? ":" : "",
					  dmt->minor > 0 ? dmt->minor : 0,
					  dmt->major > 0 && dmt->minor == 0 ? "0" : "",
					  dmt->major > 0 ? ")" : "",
					  strerror(dmt->ioctl_errno));

			/*
			 * It's sometimes worth retrying after EBUSY in case
			 * it's a transient failure caused by an asynchronous
			 * process quickly scanning the device.
			 */
			*retryable = dmt->ioctl_errno == EBUSY;

			goto error;
		}
	}

	if (ioctl_with_uevent && dm_udev_get_sync_support() &&
	    !_check_uevent_generated(dmi)) {
		log_debug_activation("Uevent not generated! Calling udev_complete "
				     "internally to avoid process lock-up.");
		_udev_complete(dmt);
	}

	if (!_dm_ioctl_unmangle_names(dmt->type, dmi))
		goto error;

	if (dmt->type != DM_DEVICE_REMOVE &&
	    !_dm_ioctl_unmangle_uuids(dmt->type, dmi))
		goto error;

#else /* Userspace alternative for testing */
	goto error;
#endif
	return dmi;

error:
	_dm_zfree_dmi(dmi);
	return NULL;
}

void dm_task_update_nodes(void)
{
	update_devs();
}

#define DM_IOCTL_RETRIES 25
#define DM_RETRY_USLEEP_DELAY 200000

int dm_task_get_errno(struct dm_task *dmt)
{
	return dmt->ioctl_errno;
}

int dm_task_run(struct dm_task *dmt)
{
	struct dm_ioctl *dmi;
	unsigned command;
	int check_udev;
	int rely_on_udev;
	int suspended_counter;
	unsigned ioctl_retry = 1;
	int retryable = 0;
	const char *dev_name = DEV_NAME(dmt);
	const char *dev_uuid = DEV_UUID(dmt);

	if ((unsigned) dmt->type >= DM_ARRAY_SIZE(_cmd_data_v4)) {
		log_error(INTERNAL_ERROR "unknown device-mapper task %d",
			  dmt->type);
		return 0;
	}

	command = _cmd_data_v4[dmt->type].cmd;

	/* Old-style creation had a table supplied */
	if (dmt->type == DM_DEVICE_CREATE && dmt->head)
		return _create_and_load_v4(dmt);

	if (dmt->type == DM_DEVICE_MKNODES && !dev_name &&
	    !dev_uuid && dmt->major <= 0)
		return _mknodes_v4(dmt);

	if ((dmt->type == DM_DEVICE_RELOAD) && dmt->suppress_identical_reload)
		return _reload_with_suppression_v4(dmt);

	if ((dmt->type == DM_DEVICE_SUSPEND) && dmt->enable_checks)
		return _suspend_with_validation_v4(dmt);

	if (!_open_control()) {
		_udev_complete(dmt);
		return_0;
	}

	if ((suspended_counter = dm_get_suspended_counter()) &&
	    dmt->type == DM_DEVICE_RELOAD)
		log_error(INTERNAL_ERROR "Performing unsafe table load while %d device(s) "
			  "are known to be suspended: "
			  "%s%s%s %s%.0d%s%.0d%s%s",
			  suspended_counter,
			  dev_name ? : "",
			  dev_uuid ? " UUID " : "",
			  dev_uuid ? : "",
			  dmt->major > 0 ? "(" : "",
			  dmt->major > 0 ? dmt->major : 0,
			  dmt->major > 0 ? ":" : "",
			  dmt->minor > 0 ? dmt->minor : 0,
			  dmt->major > 0 && dmt->minor == 0 ? "0" : "",
			  dmt->major > 0 ? ") " : "");

	/* FIXME Detect and warn if cookie set but should not be. */
repeat_ioctl:
	if (!(dmi = _do_dm_ioctl(dmt, command, _ioctl_buffer_double_factor,
				 ioctl_retry, &retryable))) {
		/*
		 * Async udev rules that scan devices commonly cause transient
		 * failures.  Normally you'd expect the user to have made sure
		 * nothing was using the device before issuing REMOVE, so it's
		 * worth retrying in case the failure is indeed transient.
		 */
		if (retryable && dmt->type == DM_DEVICE_REMOVE &&
		    dmt->retry_remove && ++ioctl_retry <= DM_IOCTL_RETRIES) {
			usleep(DM_RETRY_USLEEP_DELAY);
			goto repeat_ioctl;
		}

		_udev_complete(dmt);
		return 0;
	}

	if (dmi->flags & DM_BUFFER_FULL_FLAG) {
		switch (dmt->type) {
		case DM_DEVICE_LIST_VERSIONS:
		case DM_DEVICE_LIST:
		case DM_DEVICE_DEPS:
		case DM_DEVICE_STATUS:
		case DM_DEVICE_TABLE:
		case DM_DEVICE_WAITEVENT:
		case DM_DEVICE_TARGET_MSG:
			_ioctl_buffer_double_factor++;
			_dm_zfree_dmi(dmi);
			goto repeat_ioctl;
		default:
			log_error("WARNING: libdevmapper buffer too small for data");
		}
	}

	/*
	 * Are we expecting a udev operation to occur that we need to check for?
	 */
	check_udev = dmt->cookie_set &&
		     !(dmt->event_nr >> DM_UDEV_FLAGS_SHIFT &
		       DM_UDEV_DISABLE_DM_RULES_FLAG);

	rely_on_udev = dmt->cookie_set ? (dmt->event_nr >> DM_UDEV_FLAGS_SHIFT &
					  DM_UDEV_DISABLE_LIBRARY_FALLBACK) : 0;

	switch (dmt->type) {
	case DM_DEVICE_CREATE:
		if ((dmt->add_node == DM_ADD_NODE_ON_CREATE) &&
		    dev_name && *dev_name && !rely_on_udev)
			add_dev_node(dev_name, MAJOR(dmi->dev),
				     MINOR(dmi->dev), dmt->uid, dmt->gid,
				     dmt->mode, check_udev, rely_on_udev);
		break;
	case DM_DEVICE_REMOVE:
		/* FIXME Kernel needs to fill in dmi->name */
		if (dev_name && !rely_on_udev)
			rm_dev_node(dev_name, check_udev, rely_on_udev);
		break;

	case DM_DEVICE_RENAME:
		/* FIXME Kernel needs to fill in dmi->name */
		if (!dmt->new_uuid && dev_name)
			rename_dev_node(dev_name, dmt->newname,
					check_udev, rely_on_udev);
		break;

	case DM_DEVICE_RESUME:
		if ((dmt->add_node == DM_ADD_NODE_ON_RESUME) &&
		    dev_name && *dev_name)
			add_dev_node(dev_name, MAJOR(dmi->dev),
				     MINOR(dmi->dev), dmt->uid, dmt->gid,
				     dmt->mode, check_udev, rely_on_udev);
		/* FIXME Kernel needs to fill in dmi->name */
		set_dev_node_read_ahead(dev_name,
					MAJOR(dmi->dev), MINOR(dmi->dev),
					dmt->read_ahead, dmt->read_ahead_flags);
		break;
	
	case DM_DEVICE_MKNODES:
		if (dmi->flags & DM_EXISTS_FLAG)
			add_dev_node(dmi->name, MAJOR(dmi->dev),
				     MINOR(dmi->dev), dmt->uid,
				     dmt->gid, dmt->mode, 0, rely_on_udev);
		else if (dev_name)
			rm_dev_node(dev_name, 0, rely_on_udev);
		break;

	case DM_DEVICE_STATUS:
	case DM_DEVICE_TABLE:
	case DM_DEVICE_WAITEVENT:
		if (!_unmarshal_status(dmt, dmi))
			goto bad;
		break;
	}

	/* Was structure reused? */
	_dm_zfree_dmi(dmt->dmi.v4);
	dmt->dmi.v4 = dmi;
	return 1;

      bad:
	_dm_zfree_dmi(dmi);
	return 0;
}

void dm_hold_control_dev(int hold_open)
{
	_hold_control_fd_open = hold_open ? 1 : 0;

	log_debug("Hold of control device is now %sset.",
		  _hold_control_fd_open ? "" : "un");
}

void dm_lib_release(void)
{
	if (!_hold_control_fd_open)
		_close_control_fd();
	dm_timestamp_destroy(_dm_ioctl_timestamp);
	_dm_ioctl_timestamp = NULL;
	update_devs();
}

void dm_pools_check_leaks(void);

void dm_lib_exit(void)
{
	int suspended_counter;
	static unsigned _exited = 0;

	if (_exited++)
		return;

	if ((suspended_counter = dm_get_suspended_counter()))
		log_error("libdevmapper exiting with %d device(s) still suspended.", suspended_counter);

	dm_lib_release();
	selinux_release();
	if (_dm_bitset)
		dm_bitset_destroy(_dm_bitset);
	_dm_bitset = NULL;
	dm_pools_check_leaks();
	dm_dump_memory();
	_version_ok = 1;
	_version_checked = 0;
}

#if defined(__GNUC__)
/*
 * Maintain binary backward compatibility.
 * Version script mechanism works with 'gcc' compatible compilers only.
 */

/*
 * This following code is here to retain ABI compatibility after adding
 * the field deferred_remove to struct dm_info in version 1.02.89.
 *
 * Binaries linked against version 1.02.88 of libdevmapper or earlier
 * will use this function that returns dm_info without the
 * deferred_remove field.
 *
 * Binaries compiled against version 1.02.89 onwards will use
 * the new function dm_task_get_info_with_deferred_remove due to the
 * #define.
 *
 * N.B. Keep this function at the end of the file to make sure that
 * no code in this file accidentally calls it.
 */

int dm_task_get_info_base(struct dm_task *dmt, struct dm_info *info);
DM_EXPORT_SYMBOL_BASE(dm_task_get_info);
int dm_task_get_info_base(struct dm_task *dmt, struct dm_info *info)
{
	struct dm_info new_info;

	if (!dm_task_get_info(dmt, &new_info))
		return 0;

	memcpy(info, &new_info, offsetof(struct dm_info, deferred_remove));

	return 1;
}

int dm_task_get_info_with_deferred_remove(struct dm_task *dmt, struct dm_info *info);
int dm_task_get_info_with_deferred_remove(struct dm_task *dmt, struct dm_info *info)
{
	struct dm_info new_info;

	if (!dm_task_get_info(dmt, &new_info))
		return 0;

	memcpy(info, &new_info, offsetof(struct dm_info, internal_suspend));

	return 1;
}
#endif
