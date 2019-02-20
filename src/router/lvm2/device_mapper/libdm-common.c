/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.
 * Copyright (C) 2004-2012 Red Hat, Inc. All rights reserved.
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

#include "misc/dmlib.h"
#include "libdm-common.h"
#include "ioctl/libdm-targets.h"
#include "misc/kdev_t.h"
#include "misc/dm-ioctl.h"
#include "base/memory/zalloc.h"

#include <stdarg.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>

#ifdef UDEV_SYNC_SUPPORT
#  include <sys/types.h>
#  include <sys/ipc.h>
#  include <sys/sem.h>
#  include <libudev.h>
#endif

#ifdef __linux__
#  include <linux/fs.h>
#endif

#ifdef HAVE_SELINUX
#  include <selinux/selinux.h>
#endif
#ifdef HAVE_SELINUX_LABEL_H
#  include <selinux/label.h>
#endif

#define DM_DEFAULT_NAME_MANGLING_MODE_ENV_VAR_NAME "DM_DEFAULT_NAME_MANGLING_MODE"

#define DEV_DIR "/dev/"

#ifdef UDEV_SYNC_SUPPORT
#ifdef _SEM_SEMUN_UNDEFINED
union semun
{
	int val;			/* value for SETVAL */
	struct semid_ds *buf;		/* buffer for IPC_STAT & IPC_SET */
	unsigned short int *array;	/* array for GETALL & SETALL */
	struct seminfo *__buf;		/* buffer for IPC_INFO */
};
#endif
#endif

static char _dm_dir[PATH_MAX] = DEV_DIR DM_DIR;
static char _sysfs_dir[PATH_MAX] = "/sys/";
static char _path0[PATH_MAX];           /* path buffer, safe 4kB on stack */
static const char _mountinfo[] = "/proc/self/mountinfo";

#define DM_MAX_UUID_PREFIX_LEN	15
static char _default_uuid_prefix[DM_MAX_UUID_PREFIX_LEN + 1] = "LVM-";

static int _verbose = 0;
static int _suspended_dev_counter = 0;
static dm_string_mangling_t _name_mangling_mode = DEFAULT_DM_NAME_MANGLING;

#ifdef HAVE_SELINUX_LABEL_H
static struct selabel_handle *_selabel_handle = NULL;
#endif

static int _udev_disabled = 0;

#ifdef UDEV_SYNC_SUPPORT
static int _semaphore_supported = -1;
static int _udev_running = -1;
static int _sync_with_udev = 1;
static int _udev_checking = 1;
#endif

void dm_lib_init(void)
{
	const char *env;

	if (getenv("DM_DISABLE_UDEV"))
		_udev_disabled = 1;

	_name_mangling_mode = DEFAULT_DM_NAME_MANGLING;
	if ((env = getenv(DM_DEFAULT_NAME_MANGLING_MODE_ENV_VAR_NAME))) {
		if (!strcasecmp(env, "none"))
			_name_mangling_mode = DM_STRING_MANGLING_NONE;
		else if (!strcasecmp(env, "auto"))
			_name_mangling_mode = DM_STRING_MANGLING_AUTO;
		else if (!strcasecmp(env, "hex"))
			_name_mangling_mode = DM_STRING_MANGLING_HEX;
	}
}

/*
 * Library users can provide their own logging
 * function.
 */

__attribute__((format(printf, 5, 0)))
static void _default_log_line(int level, const char *file,
			      int line, int dm_errno_or_class,
			      const char *f, va_list ap)
{
	static int _abort_on_internal_errors = -1;
	static int _debug_with_line_numbers = -1;
	FILE *out = log_stderr(level) ? stderr : stdout;

	level = log_level(level);

	if (level <= _LOG_WARN || _verbose) {
		if (level < _LOG_WARN)
			out = stderr;

		if (_debug_with_line_numbers < 0)
			/* Set when env DM_DEBUG_WITH_LINE_NUMBERS is not "0" */
			_debug_with_line_numbers =
				strcmp(getenv("DM_DEBUG_WITH_LINE_NUMBERS") ? : "0", "0");

		if (_debug_with_line_numbers)
			fprintf(out, "%s:%d     ", file, line);

		vfprintf(out, f, ap);
		fputc('\n', out);
	}

	if (_abort_on_internal_errors < 0)
		/* Set when env DM_ABORT_ON_INTERNAL_ERRORS is not "0" */
		_abort_on_internal_errors =
			strcmp(getenv("DM_ABORT_ON_INTERNAL_ERRORS") ? : "0", "0");

	if (_abort_on_internal_errors &&
	    !strncmp(f, INTERNAL_ERROR, sizeof(INTERNAL_ERROR) - 1))
		abort();
}

__attribute__((format(printf, 5, 6)))
static void _default_log_with_errno(int level,
	    const char *file, int line, int dm_errno_or_class,
	    const char *f, ...)
{
	va_list ap;

	va_start(ap, f);
	_default_log_line(level, file, line, dm_errno_or_class, f, ap);
	va_end(ap);
}

__attribute__((format(printf, 4, 5)))
static void _default_log(int level, const char *file,
			 int line, const char *f, ...)
{
	va_list ap;

	va_start(ap, f);
	_default_log_line(level, file, line, 0, f, ap);
	va_end(ap);
}

dm_log_fn dm_log = _default_log;
dm_log_with_errno_fn dm_log_with_errno = _default_log_with_errno;

/*
 * Wrapper function to reformat new messages to and
 * old style logging which had not used errno parameter
 *
 * As we cannot simply pass '...' to old function we
 * need to process arg list locally and just pass '%s' + buffer
 */
__attribute__((format(printf, 5, 6)))
static void _log_to_default_log(int level,
	    const char *file, int line, int dm_errno_or_class,
	    const char *f, ...)
{
	int n;
	va_list ap;
	char buf[2 * PATH_MAX + 256]; /* big enough for most messages */

	va_start(ap, f);
	n = vsnprintf(buf, sizeof(buf), f, ap);
	va_end(ap);

	if (n > 0) /* Could be truncated */
		dm_log(level, file, line, "%s", buf);
}

/*
 * Wrapper function take 'old' style message without errno
 * and log it via new logging function with errno arg
 *
 * This minor case may happen if new libdm is used with old
 * recompiled tool that would decided to use new logging,
 * but still would like to use old binary plugins.
 */
__attribute__((format(printf, 4, 5)))
static void _log_to_default_log_with_errno(int level,
	    const char *file, int line, const char *f, ...)
{
	int n;
	va_list ap;
	char buf[2 * PATH_MAX + 256]; /* big enough for most messages */

	va_start(ap, f);
	n = vsnprintf(buf, sizeof(buf), f, ap);
	va_end(ap);

	if (n > 0) /* Could be truncated */
		dm_log_with_errno(level, file, line, 0, "%s", buf);
}

void dm_log_init(dm_log_fn fn)
{
	if (fn)  {
		dm_log = fn;
		dm_log_with_errno = _log_to_default_log;
	} else {
		dm_log = _default_log;
		dm_log_with_errno = _default_log_with_errno;
	}
}

int dm_log_is_non_default(void)
{
	return (dm_log == _default_log && dm_log_with_errno == _default_log_with_errno) ? 0 : 1;
}

void dm_log_with_errno_init(dm_log_with_errno_fn fn)
{
	if (fn) {
		dm_log = _log_to_default_log_with_errno;
		dm_log_with_errno = fn;
	} else {
		dm_log = _default_log;
		dm_log_with_errno = _default_log_with_errno;
	}
}

void dm_log_init_verbose(int level)
{
	_verbose = level;
}

static int _build_dev_path(char *buffer, size_t len, const char *dev_name)
{
	int r;

	/* If there's a /, assume caller knows what they're doing */
	if (strchr(dev_name, '/'))
		r = dm_strncpy(buffer, dev_name, len);
	else
		r = (dm_snprintf(buffer, len, "%s/%s",
				 _dm_dir, dev_name) < 0) ? 0 : 1;
	if (!r)
		log_error("Failed to build dev path for \"%s\".", dev_name);

	return r;
}

int dm_get_library_version(char *version, size_t size)
{
	return dm_strncpy(version, DM_LIB_VERSION, size);
}

void inc_suspended(void)
{
	_suspended_dev_counter++;
	log_debug_activation("Suspended device counter increased to %d", _suspended_dev_counter);
}

void dec_suspended(void)
{
	if (!_suspended_dev_counter) {
		log_error("Attempted to decrement suspended device counter below zero.");
		return;
	}

	_suspended_dev_counter--;
	log_debug_activation("Suspended device counter reduced to %d", _suspended_dev_counter);
}

int dm_get_suspended_counter(void)
{
	return _suspended_dev_counter;
}

int dm_set_name_mangling_mode(dm_string_mangling_t name_mangling_mode)
{
	_name_mangling_mode = name_mangling_mode;

	return 1;
}

dm_string_mangling_t dm_get_name_mangling_mode(void)
{
	return _name_mangling_mode;
}

struct dm_task *dm_task_create(int type)
{
	struct dm_task *dmt = zalloc(sizeof(*dmt));

	if (!dmt) {
		log_error("dm_task_create: malloc(%" PRIsize_t ") failed",
			  sizeof(*dmt));
		return NULL;
	}

	if (!dm_check_version()) {
		free(dmt);
		return_NULL;
	}

	dmt->type = type;
	dmt->minor = -1;
	dmt->major = -1;
	dmt->allow_default_major_fallback = 1;
	dmt->uid = DM_DEVICE_UID;
	dmt->gid = DM_DEVICE_GID;
	dmt->mode = DM_DEVICE_MODE;
	dmt->no_open_count = 0;
	dmt->read_ahead = DM_READ_AHEAD_AUTO;
	dmt->read_ahead_flags = 0;
	dmt->event_nr = 0;
	dmt->cookie_set = 0;
	dmt->query_inactive_table = 0;
	dmt->new_uuid = 0;
	dmt->secure_data = 0;
	dmt->record_timestamp = 0;

	return dmt;
}

/*
 * Find the name associated with a given device number by scanning _dm_dir.
 */
static int _find_dm_name_of_device(dev_t st_rdev, char *buf, size_t buf_len)
{
	const char *name;
	char path[PATH_MAX];
	struct dirent *dirent;
	DIR *d;
	struct stat st;
	int r = 0;

	if (!(d = opendir(_dm_dir))) {
		log_sys_error("opendir", _dm_dir);
		return 0;
	}

	while ((dirent = readdir(d))) {
		name = dirent->d_name;

		if (!strcmp(name, ".") || !strcmp(name, ".."))
			continue;

		if (dm_snprintf(path, sizeof(path), "%s/%s", _dm_dir,
				name) == -1) {
			log_error("Couldn't create path for %s", name);
			continue;
		}

		if (stat(path, &st))
			continue;

		if (st.st_rdev == st_rdev) {
			strncpy(buf, name, buf_len);
			r = 1;
			break;
		}
	}

	if (closedir(d))
		log_sys_error("closedir", _dm_dir);

	return r;
}

static int _is_whitelisted_char(char c)
{
	/*
	 * Actually, DM supports any character in a device name.
	 * This whitelist is just for proper integration with udev.
	 */
        if ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            strchr("#+-.:=@_", c) != NULL)
                return 1;

        return 0;
}

int check_multiple_mangled_string_allowed(const char *str, const char *str_name,
					 dm_string_mangling_t mode)
{
	if (mode == DM_STRING_MANGLING_AUTO && strstr(str, "\\x5cx")) {
		log_error("The %s \"%s\" seems to be mangled more than once. "
			  "This is not allowed in auto mode.", str_name, str);
		return 0;
	}

	return 1;
}

/*
 * Mangle all characters in the input string which are not on a whitelist
 * with '\xNN' format where NN is the hex value of the character.
 */
int mangle_string(const char *str, const char *str_name, size_t len,
		  char *buf, size_t buf_len, dm_string_mangling_t mode)
{
	int need_mangling = -1; /* -1 don't know yet, 0 no, 1 yes */
	size_t i, j;

	if (!str || !buf)
		return -1;

	/* Is there anything to do at all? */
	if (!*str || !len)
		return 0;

	if (buf_len < DM_NAME_LEN) {
		log_error(INTERNAL_ERROR "mangle_string: supplied buffer too small");
		return -1;
	}

	if (mode == DM_STRING_MANGLING_NONE)
		mode = DM_STRING_MANGLING_AUTO;

	for (i = 0, j = 0; str[i]; i++) {
		if (mode == DM_STRING_MANGLING_AUTO) {
			/*
			 * Detect already mangled part of the string and keep it.
			 * Return error on mixture of mangled/not mangled!
			 */
			if (str[i] == '\\' && str[i+1] == 'x') {
				if ((len - i < 4) || (need_mangling == 1))
					goto bad1;
				if (buf_len - j < 4)
					goto bad2;

				memcpy(&buf[j], &str[i], 4);
				i+=3; j+=4;

				need_mangling = 0;
				continue;
			}
		}

		if (_is_whitelisted_char(str[i])) {
			/* whitelisted, keep it. */
			if (buf_len - j < 1)
				goto bad2;
			buf[j] = str[i];
			j++;
		} else {
			/*
			 * Not on a whitelist, mangle it.
			 * Return error on mixture of mangled/not mangled
			 * unless a DM_STRING_MANGLING_HEX is used!.
			 */
			if ((mode != DM_STRING_MANGLING_HEX) && (need_mangling == 0))
				goto bad1;
			if (buf_len - j < 4)
				goto bad2;

			sprintf(&buf[j], "\\x%02x", (unsigned char) str[i]);
			j+=4;

			need_mangling = 1;
		}
	}

	if (buf_len - j < 1)
		goto bad2;
	buf[j] = '\0';

	/* All chars in the string whitelisted? */
	if (need_mangling == -1)
		need_mangling = 0;

	return need_mangling;

bad1:
	log_error("The %s \"%s\" contains mixed mangled and unmangled "
		  "characters or it's already mangled improperly.", str_name, str);
	return -1;
bad2:
	log_error("Mangled form of the %s too long for \"%s\".", str_name, str);
	return -1;
}

/*
 * Try to unmangle supplied string.
 * Return value: -1 on error, 0 when no unmangling needed, 1 when unmangling applied
 */
int unmangle_string(const char *str, const char *str_name, size_t len,
		    char *buf, size_t buf_len, dm_string_mangling_t mode)
{
	int strict = mode != DM_STRING_MANGLING_NONE;
	char str_rest[DM_NAME_LEN];
	size_t i, j;
	int code;
	int r = 0;

	if (!str || !buf)
		return -1;

	/* Is there anything to do at all? */
	if (!*str || !len)
		return 0;

	if (buf_len < DM_NAME_LEN) {
		log_error(INTERNAL_ERROR "unmangle_string: supplied buffer too small");
		return -1;
	}

	for (i = 0, j = 0; str[i]; i++, j++) {
		if (strict && !(_is_whitelisted_char(str[i]) || str[i]=='\\')) {
			log_error("The %s \"%s\" should be mangled but "
				  "it contains blacklisted characters.", str_name, str);
			j=0; r=-1;
			goto out;
		}

		if (str[i] == '\\' && str[i+1] == 'x') {
			if (!sscanf(&str[i+2], "%2x%s", &code, str_rest)) {
				log_debug_activation("Hex encoding mismatch detected in %s \"%s\" "
						     "while trying to unmangle it.", str_name, str);
				goto out;
			}
			buf[j] = (unsigned char) code;

			/* skip the encoded part we've just decoded! */
			i+= 3;

			/* unmangling applied */
			r = 1;
		} else
			buf[j] = str[i];
	}

out:
	buf[j] = '\0';
	return r;
}

static int _dm_task_set_name(struct dm_task *dmt, const char *name,
			     dm_string_mangling_t mangling_mode)
{
	char mangled_name[DM_NAME_LEN];
	int r = 0;

	free(dmt->dev_name);
	dmt->dev_name = NULL;
	free(dmt->mangled_dev_name);
	dmt->mangled_dev_name = NULL;

	if (strlen(name) >= DM_NAME_LEN) {
		log_error("Name \"%s\" too long.", name);
		return 0;
	}

	if (!check_multiple_mangled_string_allowed(name, "name", mangling_mode))
		return_0;

	if (mangling_mode != DM_STRING_MANGLING_NONE &&
	    (r = mangle_string(name, "name", strlen(name), mangled_name,
			       sizeof(mangled_name), mangling_mode)) < 0) {
		log_error("Failed to mangle device name \"%s\".", name);
		return 0;
	}

	/* Store mangled_dev_name only if it differs from dev_name! */
	if (r) {
		log_debug_activation("Device name mangled [%s]: %s --> %s",
				     mangling_mode == DM_STRING_MANGLING_AUTO ? "auto" : "hex",
				     name, mangled_name);
		if (!(dmt->mangled_dev_name = strdup(mangled_name))) {
			log_error("_dm_task_set_name: strdup(%s) failed", mangled_name);
			return 0;
		}
	}

	if (!(dmt->dev_name = strdup(name))) {
		log_error("_dm_task_set_name: strdup(%s) failed", name);
		return 0;
	}

	return 1;
}

static int _dm_task_set_name_from_path(struct dm_task *dmt, const char *path,
				       const char *name)
{
	char buf[PATH_MAX];
	struct stat st1, st2;
	const char *final_name = NULL;
	size_t len;

	if (dmt->type == DM_DEVICE_CREATE) {
		log_error("Name \"%s\" invalid. It contains \"/\".", path);
		return 0;
	}

	if (!stat(path, &st1)) {
		/*
		 * Found directly.
		 * If supplied path points to same device as last component
		 * under /dev/mapper, use that name directly.  
		 */
		if (dm_snprintf(buf, sizeof(buf), "%s/%s", _dm_dir, name) == -1) {
			log_error("Couldn't create path for %s", name);
			return 0;
		}

		if (!stat(buf, &st2) && (st1.st_rdev == st2.st_rdev))
			final_name = name;
	} else {
		/* Not found. */
		/* If there is exactly one '/' try a prefix of /dev */
		if ((len = strlen(path)) < 3 || path[0] == '/' ||
		    dm_count_chars(path, len, '/') != 1) {
			log_error("Device %s not found", path);
			return 0;
		}
		if (dm_snprintf(buf, sizeof(buf), "%s/../%s", _dm_dir, path) == -1) {
			log_error("Couldn't create /dev path for %s", path);
			return 0;
		}
		if (stat(buf, &st1)) {
			log_error("Device %s not found", path);
			return 0;
		}
		/* Found */
	}

	/*
	 * If we don't have the dm name yet, Call _find_dm_name_of_device() to
	 * scan _dm_dir for a match.
	 */
	if (!final_name) {
		if (_find_dm_name_of_device(st1.st_rdev, buf, sizeof(buf)))
			final_name = buf;
		else {
			log_error("Device %s not found", name);
			return 0;
		}
	}

	/* This is an already existing path - do not mangle! */
	return _dm_task_set_name(dmt, final_name, DM_STRING_MANGLING_NONE);
}

int dm_task_set_name(struct dm_task *dmt, const char *name)
{
	char *pos;

	/* Path supplied for existing device? */
	if ((pos = strrchr(name, '/')))
		return _dm_task_set_name_from_path(dmt, name, pos + 1);

	return _dm_task_set_name(dmt, name, dm_get_name_mangling_mode());
}

const char *dm_task_get_name(const struct dm_task *dmt)
{
	return (dmt->dmi.v4->name);
}

static char *_task_get_string_mangled(const char *str, const char *str_name,
				      char *buf, size_t buf_size,
				      dm_string_mangling_t mode)
{
	char *rs;
	int r;

	if ((r = mangle_string(str, str_name, strlen(str), buf, buf_size, mode)) < 0)
		return NULL;

	if (!(rs = r ? strdup(buf) : strdup(str)))
		log_error("_task_get_string_mangled: strdup failed");

	return rs;
}

static char *_task_get_string_unmangled(const char *str, const char *str_name,
					char *buf, size_t buf_size,
					dm_string_mangling_t mode)
{
	char *rs;
	int r = 0;

	/*
	 * Unless the mode used is 'none', the string
	 * is *already* unmangled on ioctl return!
	 */
	if (mode == DM_STRING_MANGLING_NONE &&
	    (r = unmangle_string(str, str_name, strlen(str), buf, buf_size, mode)) < 0)
		return NULL;

	if (!(rs = r ? strdup(buf) : strdup(str)))
		log_error("_task_get_string_unmangled: strdup failed");

	return rs;
}

char *dm_task_get_name_mangled(const struct dm_task *dmt)
{
	const char *s = dm_task_get_name(dmt);
	char buf[DM_NAME_LEN];
	char *rs;

	if (!(rs = _task_get_string_mangled(s, "name", buf, sizeof(buf), dm_get_name_mangling_mode())))
		log_error("Failed to mangle device name \"%s\".", s);

	return rs;
}

char *dm_task_get_name_unmangled(const struct dm_task *dmt)
{
	const char *s = dm_task_get_name(dmt);
	char buf[DM_NAME_LEN];
	char *rs;

	if (!(rs = _task_get_string_unmangled(s, "name", buf, sizeof(buf), dm_get_name_mangling_mode())))
		log_error("Failed to unmangle device name \"%s\".", s);

	return rs;
}

const char *dm_task_get_uuid(const struct dm_task *dmt)
{
	return (dmt->dmi.v4->uuid);
}

char *dm_task_get_uuid_mangled(const struct dm_task *dmt)
{
	const char *s = dm_task_get_uuid(dmt);
	char buf[DM_UUID_LEN];
	char *rs;

	if (!(rs = _task_get_string_mangled(s, "UUID", buf, sizeof(buf), dm_get_name_mangling_mode())))
		log_error("Failed to mangle device uuid \"%s\".", s);

	return rs;
}

char *dm_task_get_uuid_unmangled(const struct dm_task *dmt)
{
	const char *s = dm_task_get_uuid(dmt);
	char buf[DM_UUID_LEN];
	char *rs;

	if (!(rs = _task_get_string_unmangled(s, "UUID", buf, sizeof(buf), dm_get_name_mangling_mode())))
		log_error("Failed to unmangle device uuid \"%s\".", s);

	return rs;
}

int dm_task_set_newname(struct dm_task *dmt, const char *newname)
{
	dm_string_mangling_t mangling_mode = dm_get_name_mangling_mode();
	char mangled_name[DM_NAME_LEN];
	int r = 0;

	if (strchr(newname, '/')) {
		log_error("Name \"%s\" invalid. It contains \"/\".", newname);
		return 0;
	}

	if (strlen(newname) >= DM_NAME_LEN) {
		log_error("Name \"%s\" too long", newname);
		return 0;
	}

	if (!*newname) {
		log_error("Non empty new name is required.");
		return 0;
	}

	if (!check_multiple_mangled_string_allowed(newname, "new name", mangling_mode))
		return_0;

	if (mangling_mode != DM_STRING_MANGLING_NONE &&
	    (r = mangle_string(newname, "new name", strlen(newname), mangled_name,
			       sizeof(mangled_name), mangling_mode)) < 0) {
		log_error("Failed to mangle new device name \"%s\"", newname);
		return 0;
	}

	if (r) {
		log_debug_activation("New device name mangled [%s]: %s --> %s",
				     mangling_mode == DM_STRING_MANGLING_AUTO ? "auto" : "hex",
				     newname, mangled_name);
		newname = mangled_name;
	}

	free(dmt->newname);
	if (!(dmt->newname = strdup(newname))) {
		log_error("dm_task_set_newname: strdup(%s) failed", newname);
		return 0;
	}

	dmt->new_uuid = 0;

	return 1;
}

int dm_task_set_uuid(struct dm_task *dmt, const char *uuid)
{
	char mangled_uuid[DM_UUID_LEN];
	dm_string_mangling_t mangling_mode = dm_get_name_mangling_mode();
	int r = 0;

	free(dmt->uuid);
	dmt->uuid = NULL;
	free(dmt->mangled_uuid);
	dmt->mangled_uuid = NULL;

	if (!check_multiple_mangled_string_allowed(uuid, "UUID", mangling_mode))
		return_0;

	if (mangling_mode != DM_STRING_MANGLING_NONE &&
	    (r = mangle_string(uuid, "UUID", strlen(uuid), mangled_uuid,
			       sizeof(mangled_uuid), mangling_mode)) < 0) {
		log_error("Failed to mangle device uuid \"%s\".", uuid);
		return 0;
	}

	if (r) {
		log_debug_activation("Device uuid mangled [%s]: %s --> %s",
				     mangling_mode == DM_STRING_MANGLING_AUTO ? "auto" : "hex",
				     uuid, mangled_uuid);

		if (!(dmt->mangled_uuid = strdup(mangled_uuid))) {
			log_error("dm_task_set_uuid: strdup(%s) failed", mangled_uuid);
			return 0;
		}
	}

	if (!(dmt->uuid = strdup(uuid))) {
		log_error("dm_task_set_uuid: strdup(%s) failed", uuid);
		return 0;
	}

	return 1;
}

int dm_task_set_major(struct dm_task *dmt, int major)
{
	dmt->major = major;
	dmt->allow_default_major_fallback = 0;

	return 1;
}

int dm_task_set_minor(struct dm_task *dmt, int minor)
{
	dmt->minor = minor;

	return 1;
}

int dm_task_set_major_minor(struct dm_task *dmt, int major, int minor,
			    int allow_default_major_fallback)
{
	dmt->major = major;
	dmt->minor = minor;
	dmt->allow_default_major_fallback = allow_default_major_fallback;

	return 1;
}

int dm_task_set_uid(struct dm_task *dmt, uid_t uid)
{
	dmt->uid = uid;

	return 1;
}

int dm_task_set_gid(struct dm_task *dmt, gid_t gid)
{
	dmt->gid = gid;

	return 1;
}

int dm_task_set_mode(struct dm_task *dmt, mode_t mode)
{
	dmt->mode = mode;

	return 1;
}

int dm_task_enable_checks(struct dm_task *dmt)
{
	dmt->enable_checks = 1;

	return 1;
}

int dm_task_add_target(struct dm_task *dmt, uint64_t start, uint64_t size,
		       const char *ttype, const char *params)
{
	struct target *t = create_target(start, size, ttype, params);
	if (!t)
		return_0;

	if (!dmt->head)
		dmt->head = dmt->tail = t;
	else {
		dmt->tail->next = t;
		dmt->tail = t;
	}

	return 1;
}

#ifdef HAVE_SELINUX
static int _selabel_lookup(const char *path, mode_t mode,
			   security_context_t *scontext)
{
#ifdef HAVE_SELINUX_LABEL_H
	if (!_selabel_handle &&
	    !(_selabel_handle = selabel_open(SELABEL_CTX_FILE, NULL, 0))) {
		log_error("selabel_open failed: %s", strerror(errno));
		return 0;
	}

	if (selabel_lookup(_selabel_handle, scontext, path, mode)) {
		log_debug_activation("selabel_lookup failed for %s: %s",
				     path, strerror(errno));
		return 0;
	}
#else
	if (matchpathcon(path, mode, scontext)) {
		log_debug_activation("matchpathcon failed for %s: %s",
				     path, strerror(errno));
		return 0;
	}
#endif
	return 1;
}
#endif

#ifdef HAVE_SELINUX
static int _is_selinux_enabled(void)
{
	static int _tested = 0;
	static int _enabled;

	if (!_tested) {
		_tested = 1;
		_enabled = is_selinux_enabled();
	}

	return _enabled;
}
#endif

int dm_prepare_selinux_context(const char *path, mode_t mode)
{
#ifdef HAVE_SELINUX
	security_context_t scontext = NULL;

	if (_is_selinux_enabled() <= 0)
		return 1;

	if (path) {
		if (!_selabel_lookup(path, mode, &scontext))
			return_0;

		log_debug_activation("Preparing SELinux context for %s to %s.", path, scontext);
	}
	else
		log_debug_activation("Resetting SELinux context to default value.");

	if (setfscreatecon(scontext) < 0) {
		log_sys_error("setfscreatecon", (path ? : "SELinux context reset"));
		freecon(scontext);
		return 0;
	}

	freecon(scontext);
#endif
	return 1;
}

int dm_set_selinux_context(const char *path, mode_t mode)
{
#ifdef HAVE_SELINUX
	security_context_t scontext = NULL;

	if (_is_selinux_enabled() <= 0)
		return 1;

	if (!_selabel_lookup(path, mode, &scontext))
		return_0;

	log_debug_activation("Setting SELinux context for %s to %s.", path, scontext);

	if ((lsetfilecon(path, scontext) < 0) && (errno != ENOTSUP)) {
		log_sys_error("lsetfilecon", path);
		freecon(scontext);
		return 0;
	}

	freecon(scontext);
#endif
	return 1;
}

void selinux_release(void)
{
#ifdef HAVE_SELINUX_LABEL_H
	if (_selabel_handle)
		selabel_close(_selabel_handle);
	_selabel_handle = NULL;
#endif
}

static int _warn_if_op_needed(int warn_if_udev_failed)
{
    return warn_if_udev_failed && dm_udev_get_sync_support() && dm_udev_get_checking();
}

static int _add_dev_node(const char *dev_name, uint32_t major, uint32_t minor,
			 uid_t uid, gid_t gid, mode_t mode, int warn_if_udev_failed)
{
	char path[PATH_MAX];
	struct stat info;
	dev_t dev = MKDEV(major, minor);
	mode_t old_mask;

	if (!_build_dev_path(path, sizeof(path), dev_name))
		return_0;

	if (stat(path, &info) >= 0) {
		if (!S_ISBLK(info.st_mode)) {
			log_error("A non-block device file at '%s' "
				  "is already present", path);
			return 0;
		}

		/* If right inode already exists we don't touch uid etc. */
		if (info.st_rdev == dev)
			return 1;

		if (unlink(path) < 0) {
			log_error("Unable to unlink device node for '%s'",
				  dev_name);
			return 0;
		}
	} else if (_warn_if_op_needed(warn_if_udev_failed))
		log_warn("%s not set up by udev: Falling back to direct "
			 "node creation.", path);

	(void) dm_prepare_selinux_context(path, S_IFBLK);
	old_mask = umask(0);

	/* The node may already have been created by udev. So ignore EEXIST. */
	if (mknod(path, S_IFBLK | mode, dev) < 0 && errno != EEXIST) {
		log_error("%s: mknod for %s failed: %s", path, dev_name, strerror(errno));
		umask(old_mask);
		(void) dm_prepare_selinux_context(NULL, 0);
		return 0;
	}
	umask(old_mask);
	(void) dm_prepare_selinux_context(NULL, 0);

	if (chown(path, uid, gid) < 0) {
		log_sys_error("chown", path);
		return 0;
	}

	log_debug_activation("Created %s", path);

	return 1;
}

static int _rm_dev_node(const char *dev_name, int warn_if_udev_failed)
{
	char path[PATH_MAX];
	struct stat info;

	if (!_build_dev_path(path, sizeof(path), dev_name))
		return_0;
	if (lstat(path, &info) < 0)
		return 1;
	else if (_warn_if_op_needed(warn_if_udev_failed))
		log_warn("Node %s was not removed by udev. "
			 "Falling back to direct node removal.", path);

	/* udev may already have deleted the node. Ignore ENOENT. */
	if (unlink(path) < 0 && errno != ENOENT) {
		log_error("Unable to unlink device node for '%s'", dev_name);
		return 0;
	}

	log_debug_activation("Removed %s", path);

	return 1;
}

static int _rename_dev_node(const char *old_name, const char *new_name,
			    int warn_if_udev_failed)
{
	char oldpath[PATH_MAX];
	char newpath[PATH_MAX];
	struct stat info, info2;
	struct stat *info_block_dev;

	if (!_build_dev_path(oldpath, sizeof(oldpath), old_name) ||
	    !_build_dev_path(newpath, sizeof(newpath), new_name))
		return_0;

	if (lstat(newpath, &info) == 0) {
		if (S_ISLNK(info.st_mode)) {
			if (stat(newpath, &info2) == 0)
				info_block_dev = &info2;
			else {
				log_sys_error("stat", newpath);
				return 0;
			}
		} else
			info_block_dev = &info;

		if (!S_ISBLK(info_block_dev->st_mode)) {
			log_error("A non-block device file at '%s' "
				  "is already present", newpath);
			return 0;
		}
		else if (_warn_if_op_needed(warn_if_udev_failed)) {
			if (lstat(oldpath, &info) < 0 &&
				 errno == ENOENT)
				/* assume udev already deleted this */
				return 1;

			log_warn("The node %s should have been renamed to %s "
				 "by udev but old node is still present. "
				 "Falling back to direct old node removal.",
				 oldpath, newpath);
			return _rm_dev_node(old_name, 0);
		}

		if (unlink(newpath) < 0) {
			if (errno == EPERM) {
				/* devfs, entry has already been renamed */
				return 1;
			}
			log_error("Unable to unlink device node for '%s'",
				  new_name);
			return 0;
		}
	}
	else if (_warn_if_op_needed(warn_if_udev_failed))
		log_warn("The node %s should have been renamed to %s "
			 "by udev but new node is not present. "
			 "Falling back to direct node rename.",
			 oldpath, newpath);

	/* udev may already have renamed the node. Ignore ENOENT. */
	/* FIXME: when renaming to target mangling mode "none" with udev
	 * while there are some blacklisted characters in the node name,
	 * udev will remove the old_node, but fails to properly rename
	 * to new_node. The libdevmapper code tries to call
	 * rename(old_node,new_node), but that won't do anything
	 * since the old node is already removed by udev.
	 * For example renaming 'a\x20b' to 'a b':
	 *   - udev removes 'a\x20b'
	 *   - udev creates 'a' and 'b' (since it considers the ' ' as a delimiter
	 *   - libdevmapper checks udev has done the rename properly
	 *   - libdevmapper calls stat(new_node) and it does not see it
	 *   - libdevmapper calls rename(old_node,new_node)
	 *   - the rename is a NOP since the old_node does not exist anymore
	 *
	 * However, this situation is very rare - why would anyone need
	 * to rename to an unsupported mode??? So a fix for this would be
	 * just for completeness.
	 */
	if (rename(oldpath, newpath) < 0 && errno != ENOENT) {
		log_error("Unable to rename device node from '%s' to '%s'",
			  old_name, new_name);
		return 0;
	}

	log_debug_activation("Renamed %s to %s", oldpath, newpath);

	return 1;
}

#ifdef __linux__
static int _open_dev_node(const char *dev_name)
{
	int fd = -1;
	char path[PATH_MAX];

	if (!_build_dev_path(path, sizeof(path), dev_name))
		return fd;

	if ((fd = open(path, O_RDONLY, 0)) < 0)
		log_sys_error("open", path);

	return fd;
}

int get_dev_node_read_ahead(const char *dev_name, uint32_t major, uint32_t minor,
			    uint32_t *read_ahead)
{
	char buf[24];
	int len;
	int r = 1;
	int fd;
	long read_ahead_long;

	/*
	 * If we know the device number, use sysfs if we can.
	 * Otherwise use BLKRAGET ioctl.
	 */
	if (*_sysfs_dir && major != 0) {
		if (dm_snprintf(_path0, sizeof(_path0), "%sdev/block/%" PRIu32
				":%" PRIu32 "/bdi/read_ahead_kb", _sysfs_dir,
				major, minor) < 0) {
			log_error("Failed to build sysfs_path.");
			return 0;
		}

		if ((fd = open(_path0, O_RDONLY, 0)) != -1) {
			/* Reading from sysfs, expecting number\n */
			if ((len = read(fd, buf, sizeof(buf) - 1)) < 1) {
				log_sys_error("read", _path0);
				r = 0;
			} else {
				buf[len] = 0; /* kill \n and ensure \0 */
				*read_ahead = atoi(buf) * 2;
				log_debug_activation("%s (%d:%d): read ahead is %" PRIu32,
						     dev_name, major, minor, *read_ahead);
			}

			if (close(fd))
				log_sys_debug("close", _path0);

			return r;
		}

		log_sys_debug("open", _path0);
		/* Fall back to use dev_name */
	}

	/*
	 * Open/close dev_name may block the process
	 * (i.e. overfilled thin pool volume)
	 */
	if (!*dev_name) {
		log_error("Empty device name passed to BLKRAGET");
		return 0;
	}

	if ((fd = _open_dev_node(dev_name)) < 0)
		return_0;

	if (ioctl(fd, BLKRAGET, &read_ahead_long)) {
		log_sys_error("BLKRAGET", dev_name);
		*read_ahead = 0;
		r = 0;
	} else {
		*read_ahead = (uint32_t) read_ahead_long;
		log_debug_activation("%s: read ahead is %" PRIu32, dev_name, *read_ahead);
	}

	if (close(fd))
		log_sys_debug("close", dev_name);

	return r;
}

static int _set_read_ahead(const char *dev_name, uint32_t major, uint32_t minor,
			   uint32_t read_ahead)
{
	char buf[24];
	int len;
	int r = 1;
	int fd;
	long read_ahead_long = (long) read_ahead;

	log_debug_activation("%s (%d:%d): Setting read ahead to %" PRIu32, dev_name,
			     major, minor, read_ahead);

	/*
	 * If we know the device number, use sysfs if we can.
	 * Otherwise use BLKRASET ioctl. RA is set after resume.
	 */
	if (*_sysfs_dir && major != 0) {
		if (dm_snprintf(_path0, sizeof(_path0), "%sdev/block/%" PRIu32
				":%" PRIu32 "/bdi/read_ahead_kb",
				_sysfs_dir, major, minor) < 0) {
			log_error("Failed to build sysfs_path.");
			return 0;
		}

		/* Sysfs is kB based, round up to kB */
		if ((len = dm_snprintf(buf, sizeof(buf), FMTu32,
				       (read_ahead + 1) / 2)) < 0) {
			log_error("Failed to build size in kB.");
			return 0;
		}

		if ((fd = open(_path0, O_WRONLY, 0)) != -1) {
			if (write(fd, buf, len) < len) {
				log_sys_error("write", _path0);
				r = 0;
			}

			if (close(fd))
				log_sys_debug("close", _path0);

			return r;
		}

		log_sys_debug("open", _path0);
		/* Fall back to use dev_name */
	}

	if (!*dev_name) {
		log_error("Empty device name passed to BLKRAGET");
		return 0;
	}

	if ((fd = _open_dev_node(dev_name)) < 0)
		return_0;

	if (ioctl(fd, BLKRASET, read_ahead_long)) {
		log_sys_error("BLKRASET", dev_name);
		r = 0;
	}

	if (close(fd))
		log_sys_debug("close", dev_name);

	return r;
}

static int _set_dev_node_read_ahead(const char *dev_name,
				    uint32_t major, uint32_t minor,
				    uint32_t read_ahead, uint32_t read_ahead_flags)
{
	uint32_t current_read_ahead;

	if (read_ahead == DM_READ_AHEAD_AUTO)
		return 1;

	if (read_ahead == DM_READ_AHEAD_NONE)
		read_ahead = 0;

	if (read_ahead_flags & DM_READ_AHEAD_MINIMUM_FLAG) {
		if (!get_dev_node_read_ahead(dev_name, major, minor, &current_read_ahead))
			return_0;

		if (current_read_ahead >= read_ahead) {
			log_debug_activation("%s: retaining kernel read ahead of %" PRIu32
				  " (requested %" PRIu32 ")",           
				  dev_name, current_read_ahead, read_ahead);
			return 1;
		}
	}

	return _set_read_ahead(dev_name, major, minor, read_ahead);
}

#else

int get_dev_node_read_ahead(const char *dev_name, uint32_t *read_ahead)
{
	*read_ahead = 0;

	return 1;
}

static int _set_dev_node_read_ahead(const char *dev_name,
				    uint32_t major, uint32_t minor,
				    uint32_t read_ahead, uint32_t read_ahead_flags)
{
	return 1;
}
#endif

typedef enum {
	NODE_ADD,
	NODE_DEL,
	NODE_RENAME,
	NODE_READ_AHEAD,
	NUM_NODES
} node_op_t;

static int _do_node_op(node_op_t type, const char *dev_name, uint32_t major,
		       uint32_t minor, uid_t uid, gid_t gid, mode_t mode,
		       const char *old_name, uint32_t read_ahead,
		       uint32_t read_ahead_flags, int warn_if_udev_failed)
{
	switch (type) {
	case NODE_ADD:
		return _add_dev_node(dev_name, major, minor, uid, gid,
				     mode, warn_if_udev_failed);
	case NODE_DEL:
		return _rm_dev_node(dev_name, warn_if_udev_failed);
	case NODE_RENAME:
		return _rename_dev_node(old_name, dev_name, warn_if_udev_failed);
	case NODE_READ_AHEAD:
		return _set_dev_node_read_ahead(dev_name, major, minor,
						read_ahead, read_ahead_flags);
	default:
		; /* NOTREACHED */
	}

	return 1;
}

static DM_LIST_INIT(_node_ops);
static int _count_node_ops[NUM_NODES];

struct node_op_parms {
	struct dm_list list;
	node_op_t type;
	char *dev_name;
	uint32_t major;
	uint32_t minor;
	uid_t uid;
	gid_t gid;
	mode_t mode;
	uint32_t read_ahead;
	uint32_t read_ahead_flags;
	char *old_name;
	int warn_if_udev_failed;
	unsigned rely_on_udev;
	char names[0];
};

static void _store_str(char **pos, char **ptr, const char *str)
{
	strcpy(*pos, str);
	*ptr = *pos;
	*pos += strlen(*ptr) + 1;
}

static void _del_node_op(struct node_op_parms *nop)
{
	_count_node_ops[nop->type]--;
	dm_list_del(&nop->list);
	free(nop);

}

/* Check if there is other the type of node operation stacked */
static int _other_node_ops(node_op_t type)
{
	unsigned i;

	for (i = 0; i < NUM_NODES; i++)
		if (type != i && _count_node_ops[i])
			return 1;
	return 0;
}

static void _log_node_op(const char *action_str, struct node_op_parms *nop)
{
	const char *rely = nop->rely_on_udev ? " [trust_udev]" : "" ;
	const char *verify = nop->warn_if_udev_failed ? " [verify_udev]" : "";

	switch (nop->type) {
	case NODE_ADD:
		log_debug_activation("%s: %s NODE_ADD (%" PRIu32 ",%" PRIu32 ") %u:%u 0%o%s%s",
				     nop->dev_name, action_str, nop->major, nop->minor, nop->uid, nop->gid, nop->mode,
				     rely, verify);
		break;
	case NODE_DEL:
		log_debug_activation("%s: %s NODE_DEL%s%s", nop->dev_name, action_str, rely, verify);
		break;
	case NODE_RENAME:
		log_debug_activation("%s: %s NODE_RENAME to %s%s%s", nop->old_name, action_str, nop->dev_name, rely, verify);
		break;
	case NODE_READ_AHEAD:
		log_debug_activation("%s: %s NODE_READ_AHEAD %" PRIu32 " (flags=%" PRIu32 ")%s%s",
				     nop->dev_name, action_str, nop->read_ahead, nop->read_ahead_flags, rely, verify);
		break;
	default:
		; /* NOTREACHED */
	}
}

static int _stack_node_op(node_op_t type, const char *dev_name, uint32_t major,
			  uint32_t minor, uid_t uid, gid_t gid, mode_t mode,
			  const char *old_name, uint32_t read_ahead,
			  uint32_t read_ahead_flags, int warn_if_udev_failed,
			  unsigned rely_on_udev)
{
	struct node_op_parms *nop;
	struct dm_list *noph, *nopht;
	size_t len = strlen(dev_name) + strlen(old_name) + 2;
	char *pos;

	/*
	 * Note: warn_if_udev_failed must have valid content
	 */
	if ((type == NODE_DEL) && _other_node_ops(type))
		/*
		 * Ignore any outstanding operations on the node if deleting it.
		 */
		dm_list_iterate_safe(noph, nopht, &_node_ops) {
			nop = dm_list_item(noph, struct node_op_parms);
			if (!strcmp(dev_name, nop->dev_name)) {
				_log_node_op("Unstacking", nop);
				_del_node_op(nop);
				if (!_other_node_ops(type))
					break; /* no other non DEL ops */
			}
		}
	else if ((type == NODE_ADD) && _count_node_ops[NODE_DEL])
		/*
		 * Ignore previous DEL operation on added node.
		 * (No other operations for this device then DEL could be stacked here).
		 */
		dm_list_iterate_safe(noph, nopht, &_node_ops) {
			nop = dm_list_item(noph, struct node_op_parms);
			if ((nop->type == NODE_DEL) &&
			    !strcmp(dev_name, nop->dev_name)) {
				_log_node_op("Unstacking", nop);
				_del_node_op(nop);
				break; /* no other DEL ops */
			}
		}
	else if (type == NODE_RENAME)
		/*
		 * Ignore any outstanding operations if renaming it.
		 *
		 * Currently  RENAME operation happens through 'suspend -> resume'.
		 * On 'resume' device is added with read_ahead settings, so it is
		 * safe to remove any stacked ADD, RENAME, READ_AHEAD operation
		 * There cannot be any DEL operation on the renamed device.
		 */
		dm_list_iterate_safe(noph, nopht, &_node_ops) {
			nop = dm_list_item(noph, struct node_op_parms);
			if (!strcmp(old_name, nop->dev_name)) {
				_log_node_op("Unstacking", nop);
				_del_node_op(nop);
			}
		}
	else if (type == NODE_READ_AHEAD) {
		/* udev doesn't process readahead */
		rely_on_udev = 0;
		warn_if_udev_failed = 0;
	}

	if (!(nop = malloc(sizeof(*nop) + len))) {
		log_error("Insufficient memory to stack mknod operation");
		return 0;
	}

	pos = nop->names;
	nop->type = type;
	nop->major = major;
	nop->minor = minor;
	nop->uid = uid;
	nop->gid = gid;
	nop->mode = mode;
	nop->read_ahead = read_ahead;
	nop->read_ahead_flags = read_ahead_flags;
	nop->rely_on_udev = rely_on_udev;

	/*
	 * Clear warn_if_udev_failed if rely_on_udev is set.  It doesn't get
	 * checked in this case - this just removes the flag from log messages.
	 */
	nop->warn_if_udev_failed = rely_on_udev ? 0 : warn_if_udev_failed;

	_store_str(&pos, &nop->dev_name, dev_name);
	_store_str(&pos, &nop->old_name, old_name);

	_count_node_ops[type]++;
	dm_list_add(&_node_ops, &nop->list);

	_log_node_op("Stacking", nop);

	return 1;
}

static void _pop_node_ops(void)
{
	struct dm_list *noph, *nopht;
	struct node_op_parms *nop;

	dm_list_iterate_safe(noph, nopht, &_node_ops) {
		nop = dm_list_item(noph, struct node_op_parms);
		if (!nop->rely_on_udev) {
			_log_node_op("Processing", nop);
			_do_node_op(nop->type, nop->dev_name, nop->major, nop->minor,
				    nop->uid, nop->gid, nop->mode, nop->old_name,
				    nop->read_ahead, nop->read_ahead_flags,
				    nop->warn_if_udev_failed);
		} else
			_log_node_op("Skipping", nop);
		_del_node_op(nop);
	}
}

int add_dev_node(const char *dev_name, uint32_t major, uint32_t minor,
		 uid_t uid, gid_t gid, mode_t mode, int check_udev, unsigned rely_on_udev)
{
	return _stack_node_op(NODE_ADD, dev_name, major, minor, uid,
			      gid, mode, "", 0, 0, check_udev, rely_on_udev);
}

int rename_dev_node(const char *old_name, const char *new_name, int check_udev, unsigned rely_on_udev)
{
	return _stack_node_op(NODE_RENAME, new_name, 0, 0, 0,
			      0, 0, old_name, 0, 0, check_udev, rely_on_udev);
}

int rm_dev_node(const char *dev_name, int check_udev, unsigned rely_on_udev)
{
	return _stack_node_op(NODE_DEL, dev_name, 0, 0, 0,
			      0, 0, "", 0, 0, check_udev, rely_on_udev);
}

int set_dev_node_read_ahead(const char *dev_name,
                            uint32_t major, uint32_t minor,
			    uint32_t read_ahead, uint32_t read_ahead_flags)
{
	if (read_ahead == DM_READ_AHEAD_AUTO)
		return 1;

	return _stack_node_op(NODE_READ_AHEAD, dev_name, major, minor, 0, 0,
                              0, "", read_ahead, read_ahead_flags, 0, 0);
}

void update_devs(void)
{
	_pop_node_ops();
}

static int _canonicalize_and_set_dir(const char *src, const char *suffix, size_t max_len, char *dir)
{
	size_t len;
	const char *slash;

	if (*src != '/') {
		log_debug_activation("Invalid directory value, %s: "
				     "not an absolute name.", src);
		return 0;
	}

	len = strlen(src);
	slash = src[len-1] == '/' ? "" : "/";

	if (dm_snprintf(dir, max_len, "%s%s%s", src, slash, suffix ? suffix : "") < 0) {
		log_debug_activation("Invalid directory value, %s: name too long.", src);
		return 0;
	}

	return 1;
}

int dm_set_dev_dir(const char *dev_dir)
{
	return _canonicalize_and_set_dir(dev_dir, DM_DIR, sizeof _dm_dir, _dm_dir);
}

const char *dm_dir(void)
{
	return _dm_dir;
}

int dm_set_sysfs_dir(const char *sysfs_dir)
{
	if (!sysfs_dir || !*sysfs_dir) {
		_sysfs_dir[0] = '\0';
		return 1;
	}

	return _canonicalize_and_set_dir(sysfs_dir, NULL, sizeof _sysfs_dir, _sysfs_dir);
}

const char *dm_sysfs_dir(void)
{
	return _sysfs_dir;
}

/*
 * Replace existing uuid_prefix provided it isn't too long.
 */
int dm_set_uuid_prefix(const char *uuid_prefix)
{
	if (!uuid_prefix)
		return_0;

	if (strlen(uuid_prefix) > DM_MAX_UUID_PREFIX_LEN) {
		log_error("New uuid prefix %s too long.", uuid_prefix);
		return 0;
	}

	strcpy(_default_uuid_prefix, uuid_prefix);

	return 1;
}

const char *dm_uuid_prefix(void)
{
	return _default_uuid_prefix;
}

static int _is_octal(int a)
{
	return (((a) & ~7) == '0');
}

/* Convert mangled mountinfo into normal ASCII string */
static void _unmangle_mountinfo_string(const char *src, char *buf)
{
	while (*src) {
		if ((*src == '\\') &&
		    _is_octal(src[1]) && _is_octal(src[2]) && _is_octal(src[3])) {
			*buf++ = 64 * (src[1] & 7) + 8 * (src[2] & 7) + (src[3] & 7);
			src += 4;
		} else
			*buf++ = *src++;
	}
	*buf = '\0';
}

/* Parse one line of mountinfo and unmangled target line */
static int _mountinfo_parse_line(const char *line, unsigned *maj, unsigned *min, char *buf)
{
	char root[PATH_MAX + 1]; /* sscanf needs extra '\0' */
	char target[PATH_MAX + 1];
	char *devmapper;
	struct dm_task *dmt;
	struct dm_info info;
	unsigned i;

	/* TODO: maybe detect availability of  %ms  glib support ? */
	if (sscanf(line, "%*u %*u %u:%u %" DM_TO_STRING(PATH_MAX)
		   "s %" DM_TO_STRING(PATH_MAX) "s",
		   maj, min, root, target) < 4) {
		log_error("Failed to parse mountinfo line.");
		return 0;
	}

	/* btrfs fakes device numbers, but there is still /dev/mapper name
	 * placed in mountinfo, so try to detect proper major:minor via this */
	if (*maj == 0 && (devmapper = strstr(line, "/dev/mapper/"))) {
		if (!(dmt = dm_task_create(DM_DEVICE_INFO))) {
			log_error("Mount info task creation failed.");
			return 0;
		}
		devmapper += 12; /* skip fixed prefix */
		for (i = 0; devmapper[i] && devmapper[i] != ' ' && i < sizeof(root)-1; ++i)
			root[i] = devmapper[i];
		root[i] = 0;
		_unmangle_mountinfo_string(root, buf);
		buf[DM_NAME_LEN] = 0; /* cut away */

		if (dm_task_set_name(dmt, buf) &&
		    dm_task_no_open_count(dmt) &&
		    dm_task_run(dmt) &&
		    dm_task_get_info(dmt, &info)) {
			log_debug("Replacing mountinfo device (%u:%u) with matching DM device %s (%u:%u).",
				  *maj, *min, buf, info.major, info.minor);
			*maj = info.major;
			*min = info.minor;
		}
		dm_task_destroy(dmt);
	}

	_unmangle_mountinfo_string(target, buf);

	return 1;
}

/*
 * Function to operate on individal mountinfo line,
 * minor, major and mount target are parsed and unmangled
 */
int dm_mountinfo_read(dm_mountinfo_line_callback_fn read_fn, void *cb_data)
{
	FILE *minfo;
	char buffer[2 * PATH_MAX];
	char target[PATH_MAX];
	unsigned maj, min;
	int r = 1;

	if (!(minfo = fopen(_mountinfo, "r"))) {
		if (errno != ENOENT)
			log_sys_error("fopen", _mountinfo);
		else
			log_sys_debug("fopen", _mountinfo);
		return 0;
	}

	while (!feof(minfo) && fgets(buffer, sizeof(buffer), minfo))
		if (!_mountinfo_parse_line(buffer, &maj, &min, target) ||
		    !read_fn(buffer, maj, min, target, cb_data)) {
			stack;
			r = 0;
			break;
		}

	if (fclose(minfo))
		log_sys_error("fclose", _mountinfo);

	return r;
}

static int _sysfs_get_dm_name(uint32_t major, uint32_t minor, char *buf, size_t buf_size)
{
	char *sysfs_path, *temp_buf = NULL;
	FILE *fp = NULL;
	int r = 0;
	size_t len;

	if (!(sysfs_path = malloc(PATH_MAX)) ||
	    !(temp_buf = malloc(PATH_MAX))) {
		log_error("_sysfs_get_dm_name: failed to allocate temporary buffers");
		goto bad;
	}

	if (dm_snprintf(sysfs_path, PATH_MAX, "%sdev/block/%" PRIu32 ":%" PRIu32
			"/dm/name", _sysfs_dir, major, minor) < 0) {
		log_error("_sysfs_get_dm_name: dm_snprintf failed");
		goto bad;
	}

	if (!(fp = fopen(sysfs_path, "r"))) {
		if (errno != ENOENT)
			log_sys_error("fopen", sysfs_path);
		else
			log_sys_debug("fopen", sysfs_path);
		goto bad;
	}

	if (!fgets(temp_buf, PATH_MAX, fp)) {
		log_sys_error("fgets", sysfs_path);
		goto bad;
	}

	len = strlen(temp_buf);

	if (len > buf_size) {
		log_error("_sysfs_get_dm_name: supplied buffer too small");
		goto bad;
	}

	temp_buf[len ? len - 1 : 0] = '\0'; /* \n */
	strcpy(buf, temp_buf);
	r = 1;
bad:
	if (fp && fclose(fp))
		log_sys_error("fclose", sysfs_path);

	free(temp_buf);
	free(sysfs_path);

	return r;
}

static int _sysfs_get_kernel_name(uint32_t major, uint32_t minor, char *buf, size_t buf_size)
{
	char *name, *sysfs_path, *temp_buf = NULL;
	ssize_t size;
	size_t len;
	int r = 0;

	if (!(sysfs_path = malloc(PATH_MAX)) ||
	    !(temp_buf = malloc(PATH_MAX))) {
		log_error("_sysfs_get_kernel_name: failed to allocate temporary buffers");
		goto bad;
	}

	if (dm_snprintf(sysfs_path, PATH_MAX, "%sdev/block/%" PRIu32 ":%" PRIu32,
			_sysfs_dir, major, minor) < 0) {
		log_error("_sysfs_get_kernel_name: dm_snprintf failed");
		goto bad;
	}

	if ((size = readlink(sysfs_path, temp_buf, PATH_MAX - 1)) < 0) {
		if (errno != ENOENT)
			log_sys_error("readlink", sysfs_path);
		else
			log_sys_debug("readlink", sysfs_path);
		goto bad;
	}
	temp_buf[size] = '\0';

	if (!(name = strrchr(temp_buf, '/'))) {
		log_error("Could not locate device kernel name in sysfs path %s", temp_buf);
		goto bad;
	}
	name += 1;
	len = size - (name - temp_buf) + 1;

	if (len > buf_size) {
		log_error("_sysfs_get_kernel_name: output buffer too small");
		goto bad;
	}

	strcpy(buf, name);
	r = 1;
bad:
	free(temp_buf);
	free(sysfs_path);

	return r;
}

int dm_device_get_name(uint32_t major, uint32_t minor, int prefer_kernel_name,
		       char *buf, size_t buf_size)
{
	if (!*_sysfs_dir)
		return 0;

	/*
	 * device-mapper devices and prefer_kernel_name = 0
	 * get dm name by reading /sys/dev/block/major:minor/dm/name,
	 * fallback to _sysfs_get_kernel_name if not successful
	 */
	if (dm_is_dm_major(major) && !prefer_kernel_name) {
		if (_sysfs_get_dm_name(major, minor, buf, buf_size))
			return 1;
		else
			stack;
	}

	/*
	 * non-device-mapper devices or prefer_kernel_name = 1
	 * get kernel name using readlink /sys/dev/block/major:minor -> .../dm-X
	 */
	return _sysfs_get_kernel_name(major, minor, buf, buf_size);
}

int dm_device_has_holders(uint32_t major, uint32_t minor)
{
	char sysfs_path[PATH_MAX];
	struct stat st;

	if (!*_sysfs_dir)
		return 0;

	if (dm_snprintf(sysfs_path, PATH_MAX, "%sdev/block/%" PRIu32
			":%" PRIu32 "/holders", _sysfs_dir, major, minor) < 0) {
		log_warn("WARNING: sysfs_path dm_snprintf failed.");
		return 0;
	}

	if (stat(sysfs_path, &st)) {
		if (errno != ENOENT)
			log_sys_debug("stat", sysfs_path);
		return 0;
	}

	return !dm_is_empty_dir(sysfs_path);
}

static int _mounted_fs_on_device(const char *kernel_dev_name)
{
	char sysfs_path[PATH_MAX];
	struct dirent *dirent;
	DIR *d;
	struct stat st;
	int r = 0;

	if (dm_snprintf(sysfs_path, PATH_MAX, "%sfs", _sysfs_dir) < 0) {
		log_warn("WARNING: sysfs_path dm_snprintf failed.");
		return 0;
	}

	if (!(d = opendir(sysfs_path))) {
		if (errno != ENOENT)
			log_sys_debug("opendir", sysfs_path);
		return 0;
	}

	while ((dirent = readdir(d))) {
		if (!strcmp(dirent->d_name, ".") || !strcmp(dirent->d_name, ".."))
			continue;

		if (dm_snprintf(sysfs_path, PATH_MAX, "%sfs/%s/%s",
				_sysfs_dir, dirent->d_name, kernel_dev_name) < 0) {
			log_warn("WARNING: sysfs_path dm_snprintf failed.");
			break;
		}

		if (!stat(sysfs_path, &st)) {
			/* found! */
			r = 1;
			break;
		}
		else if (errno != ENOENT) {
			log_sys_debug("stat", sysfs_path);
			break;
		}
	}

	if (closedir(d))
		log_sys_debug("closedir", kernel_dev_name);

	return r;
}

struct mountinfo_s {
	unsigned maj;
	unsigned min;
	int mounted;
};

static int _device_has_mounted_fs(char *buffer, unsigned major, unsigned minor,
				  char *target, void *cb_data)
{
	struct mountinfo_s *data = cb_data;
	char kernel_dev_name[PATH_MAX];

	if ((major == data->maj) && (minor == data->min)) {
		if (!dm_device_get_name(major, minor, 1, kernel_dev_name,
					sizeof(kernel_dev_name))) {
			stack;
			*kernel_dev_name = '\0';
		}
		log_verbose("Device %s (%u:%u) appears to be mounted on %s.",
			    kernel_dev_name, major, minor, target);
		data->mounted = 1;
	}

	return 1;
}

int dm_device_has_mounted_fs(uint32_t major, uint32_t minor)
{
	char kernel_dev_name[PATH_MAX];
	struct mountinfo_s data = {
		.maj = major,
		.min = minor,
	};

	if (!dm_mountinfo_read(_device_has_mounted_fs, &data))
		stack;

	if (data.mounted)
		return 1;
	/*
	 * TODO: Verify dm_mountinfo_read() is superset
	 * and remove sysfs check (namespaces)
	 */
	/* Get kernel device name first */
	if (!dm_device_get_name(major, minor, 1, kernel_dev_name, PATH_MAX))
		return 0;

	/* Check /sys/fs/<fs_name>/<kernel_dev_name> presence */
	return _mounted_fs_on_device(kernel_dev_name);
}

int dm_mknodes(const char *name)
{
	struct dm_task *dmt;
	int r = 0;

	if (!(dmt = dm_task_create(DM_DEVICE_MKNODES)))
		return_0;

	if (name && !dm_task_set_name(dmt, name))
		goto out;

	if (!dm_task_no_open_count(dmt))
		goto out;

	r = dm_task_run(dmt);

out:
	dm_task_destroy(dmt);
	return r;
}

int dm_driver_version(char *version, size_t size)
{
	struct dm_task *dmt;
	int r = 0;

	if (!(dmt = dm_task_create(DM_DEVICE_VERSION)))
		return_0;

	if (!dm_task_run(dmt))
		log_error("Failed to get driver version");

	if (!dm_task_get_driver_version(dmt, version, size))
		goto out;

	r = 1;

out:
	dm_task_destroy(dmt);
	return r;
}

static void _set_cookie_flags(struct dm_task *dmt, uint16_t flags)
{
	if (!dm_cookie_supported())
		return;

	if (_udev_disabled) {
		/*
		 * If udev is disabled, hardcode this functionality:
		 *   - we want libdm to create the nodes
		 *   - we don't want the /dev/mapper and any subsystem
		 *     related content to be created by udev if udev
		 *     rules are installed
		 */
		flags &= ~DM_UDEV_DISABLE_LIBRARY_FALLBACK;
		flags |= DM_UDEV_DISABLE_DM_RULES_FLAG | DM_UDEV_DISABLE_SUBSYSTEM_RULES_FLAG;
	}

	dmt->event_nr = flags << DM_UDEV_FLAGS_SHIFT;
}

#ifndef UDEV_SYNC_SUPPORT
void dm_udev_set_sync_support(int sync_with_udev)
{
}

int dm_udev_get_sync_support(void)
{
	return 0;
}

void dm_udev_set_checking(int checking)
{
}

int dm_udev_get_checking(void)
{
	return 0;
}

int dm_task_set_cookie(struct dm_task *dmt, uint32_t *cookie, uint16_t flags)
{
	_set_cookie_flags(dmt, flags);

	*cookie = 0;
	dmt->cookie_set = 1;

	return 1;
}

int dm_udev_complete(uint32_t cookie)
{
	return 1;
}

int dm_udev_wait(uint32_t cookie)
{
	update_devs();

	return 1;
}

int dm_udev_wait_immediate(uint32_t cookie, int *ready)
{
	update_devs();
	*ready = 1;

	return 1;
}

#else		/* UDEV_SYNC_SUPPORT */

static int _check_semaphore_is_supported(void)
{
	int maxid;
	union semun arg;
	struct seminfo seminfo;

	arg.__buf = &seminfo;
	maxid = semctl(0, 0, SEM_INFO, arg);

	if (maxid < 0) {
		log_warn("Kernel not configured for semaphores (System V IPC). "
			 "Not using udev synchronisation code.");
		return 0;
	}

	return 1;
}

static int _check_udev_is_running(void)
{
	struct udev *udev;
	struct udev_queue *udev_queue;
	int r;

	if (!(udev = udev_new()))
		goto_bad;

	if (!(udev_queue = udev_queue_new(udev))) {
		udev_unref(udev);
		goto_bad;
	}

	if (!(r = udev_queue_get_udev_is_active(udev_queue)))
		log_debug_activation("Udev is not running. "
				     "Not using udev synchronisation code.");

	udev_queue_unref(udev_queue);
	udev_unref(udev);

	return r;

bad:
	log_error("Could not get udev state. Assuming udev is not running.");
	return 0;
}

static void _check_udev_sync_requirements_once(void)
{
	if (_semaphore_supported < 0)
		_semaphore_supported = _check_semaphore_is_supported();

	if (_udev_running < 0) {
		_udev_running = _check_udev_is_running();
		if (_udev_disabled && _udev_running)
			log_warn("Udev is running and DM_DISABLE_UDEV environment variable is set. "
				 "Bypassing udev, device-mapper library will manage device "
				 "nodes in device directory.");
	}
}

void dm_udev_set_sync_support(int sync_with_udev)
{
	_check_udev_sync_requirements_once();
	_sync_with_udev = sync_with_udev;
}

int dm_udev_get_sync_support(void)
{
	_check_udev_sync_requirements_once();

	return !_udev_disabled && _semaphore_supported &&
		dm_cookie_supported() &&_udev_running && _sync_with_udev;
}

void dm_udev_set_checking(int checking)
{
	if ((_udev_checking = checking))
		log_debug_activation("DM udev checking enabled");
	else
		log_debug_activation("DM udev checking disabled");
}

int dm_udev_get_checking(void)
{
	return _udev_checking;
}

static int _get_cookie_sem(uint32_t cookie, int *semid)
{
	if (cookie >> 16 != DM_COOKIE_MAGIC) {
		log_error("Could not continue to access notification "
			  "semaphore identified by cookie value %"
			  PRIu32 " (0x%x). Incorrect cookie prefix.",
			  cookie, cookie);
		return 0;
	}

	if ((*semid = semget((key_t) cookie, 1, 0)) >= 0)
		return 1;

	switch (errno) {
		case ENOENT:
			log_error("Could not find notification "
				  "semaphore identified by cookie "
				  "value %" PRIu32 " (0x%x)",
				  cookie, cookie);
			break;
		case EACCES:
			log_error("No permission to access "
				  "notificaton semaphore identified "
				  "by cookie value %" PRIu32 " (0x%x)",
				  cookie, cookie);
			break;
		default:
			log_error("Failed to access notification "
				   "semaphore identified by cookie "
				   "value %" PRIu32 " (0x%x): %s",
				  cookie, cookie, strerror(errno));
			break;
	}

	return 0;
}

static int _udev_notify_sem_inc(uint32_t cookie, int semid)
{
	struct sembuf sb = {0, 1, 0};
	int val;

	if (semop(semid, &sb, 1) < 0) {
		log_error("semid %d: semop failed for cookie 0x%" PRIx32 ": %s",
			  semid, cookie, strerror(errno));
		return 0;
	}

 	if ((val = semctl(semid, 0, GETVAL)) < 0) {
		log_error("semid %d: sem_ctl GETVAL failed for "
			  "cookie 0x%" PRIx32 ": %s",
			  semid, cookie, strerror(errno));
		return 0;		
	}

	log_debug_activation("Udev cookie 0x%" PRIx32 " (semid %d) incremented to %d",
		  cookie, semid, val);

	return 1;
}

static int _udev_notify_sem_dec(uint32_t cookie, int semid)
{
	struct sembuf sb = {0, -1, IPC_NOWAIT};
	int val;

 	if ((val = semctl(semid, 0, GETVAL)) < 0) {
		log_error("semid %d: sem_ctl GETVAL failed for "
			  "cookie 0x%" PRIx32 ": %s",
			  semid, cookie, strerror(errno));
		return 0;
	}

	if (semop(semid, &sb, 1) < 0) {
		switch (errno) {
			case EAGAIN:
				log_error("semid %d: semop failed for cookie "
					  "0x%" PRIx32 ": "
					  "incorrect semaphore state",
					  semid, cookie);
				break;
			default:
				log_error("semid %d: semop failed for cookie "
					  "0x%" PRIx32 ": %s",
					  semid, cookie, strerror(errno));
				break;
		}
		return 0;
	}

	log_debug_activation("Udev cookie 0x%" PRIx32 " (semid %d) decremented to %d",
			     cookie, semid, val - 1);

	return 1;
}

static int _udev_notify_sem_destroy(uint32_t cookie, int semid)
{
	if (semctl(semid, 0, IPC_RMID, 0) < 0) {
		log_error("Could not cleanup notification semaphore "
			  "identified by cookie value %" PRIu32 " (0x%x): %s",
			  cookie, cookie, strerror(errno));
		return 0;
	}

	log_debug_activation("Udev cookie 0x%" PRIx32 " (semid %d) destroyed", cookie,
			     semid);

	return 1;
}

static int _udev_notify_sem_create(uint32_t *cookie, int *semid)
{
	int fd;
	int gen_semid;
	int val;
	uint16_t base_cookie;
	uint32_t gen_cookie;
	union semun sem_arg;

	if ((fd = open("/dev/urandom", O_RDONLY)) < 0) {
		log_error("Failed to open /dev/urandom "
			  "to create random cookie value");
		*cookie = 0;
		return 0;
	}

	/* Generate random cookie value. Be sure it is unique and non-zero. */
	do {
		/* FIXME Handle non-error returns from read(). Move _io() into libdm? */
		if (read(fd, &base_cookie, sizeof(base_cookie)) != sizeof(base_cookie)) {
			log_error("Failed to initialize notification cookie");
			goto bad;
		}

		gen_cookie = DM_COOKIE_MAGIC << 16 | base_cookie;

		if (base_cookie && (gen_semid = semget((key_t) gen_cookie,
				    1, 0600 | IPC_CREAT | IPC_EXCL)) < 0) {
			switch (errno) {
				case EEXIST:
					/* if the semaphore key exists, we
					 * simply generate another random one */
					base_cookie = 0;
					break;
				case ENOMEM:
					log_error("Not enough memory to create "
						  "notification semaphore");
					goto bad;
				case ENOSPC:
					log_error("Limit for the maximum number "
						  "of semaphores reached. You can "
						  "check and set the limits in "
						  "/proc/sys/kernel/sem.");
					goto bad;
				default:
					log_error("Failed to create notification "
						  "semaphore: %s", strerror(errno));
					goto bad;
			}
		}
	} while (!base_cookie);

	log_debug_activation("Udev cookie 0x%" PRIx32 " (semid %d) created",
			     gen_cookie, gen_semid);

	sem_arg.val = 1;

	if (semctl(gen_semid, 0, SETVAL, sem_arg) < 0) {
		log_error("semid %d: semctl failed: %s", gen_semid, strerror(errno));
		/* We have to destroy just created semaphore
		 * so it won't stay in the system. */
		(void) _udev_notify_sem_destroy(gen_cookie, gen_semid);
		goto bad;
	}

 	if ((val = semctl(gen_semid, 0, GETVAL)) < 0) {
		log_error("semid %d: sem_ctl GETVAL failed for "
			  "cookie 0x%" PRIx32 ": %s",
			  gen_semid, gen_cookie, strerror(errno));
		goto bad;
	}

	log_debug_activation("Udev cookie 0x%" PRIx32 " (semid %d) incremented to %d",
			     gen_cookie, gen_semid, val);

	if (close(fd))
		stack;

	*semid = gen_semid;
	*cookie = gen_cookie;

	return 1;

bad:
	if (close(fd))
		stack;

	*cookie = 0;

	return 0;
}

int dm_udev_create_cookie(uint32_t *cookie)
{
	int semid;

	if (!dm_udev_get_sync_support()) {
		*cookie = 0;
		return 1;
	}

	return _udev_notify_sem_create(cookie, &semid);
}

static const char *_task_type_disp(int type)
{
	switch(type) {
	case DM_DEVICE_CREATE:
		return "CREATE";
        case DM_DEVICE_RELOAD:
		return "RELOAD";
        case DM_DEVICE_REMOVE:
		return "REMOVE";
        case DM_DEVICE_REMOVE_ALL:
		return "REMOVE_ALL";
        case DM_DEVICE_SUSPEND:
		return "SUSPEND";
        case DM_DEVICE_RESUME:
		return "RESUME";
        case DM_DEVICE_INFO:
		return "INFO";
        case DM_DEVICE_DEPS:
		return "DEPS";
        case DM_DEVICE_RENAME:
		return "RENAME";
        case DM_DEVICE_VERSION:
		return "VERSION";
        case DM_DEVICE_STATUS:
		return "STATUS";
        case DM_DEVICE_TABLE:
		return "TABLE";
        case DM_DEVICE_WAITEVENT:
		return "WAITEVENT";
        case DM_DEVICE_LIST:
		return "LIST";
        case DM_DEVICE_CLEAR:
		return "CLEAR";
        case DM_DEVICE_MKNODES:
		return "MKNODES";
        case DM_DEVICE_LIST_VERSIONS:
		return "LIST_VERSIONS";
        case DM_DEVICE_TARGET_MSG:
		return "TARGET_MSG";
        case DM_DEVICE_SET_GEOMETRY:
		return "SET_GEOMETRY";
	}
	return "unknown";
}

int dm_task_set_cookie(struct dm_task *dmt, uint32_t *cookie, uint16_t flags)
{
	int semid;

	_set_cookie_flags(dmt, flags);

	if (!dm_udev_get_sync_support()) {
		*cookie = 0;
		dmt->cookie_set = 1;
		return 1;
	}

	if (*cookie) {
		if (!_get_cookie_sem(*cookie, &semid))
			goto_bad;
	} else if (!_udev_notify_sem_create(cookie, &semid))
		goto_bad;

	if (!_udev_notify_sem_inc(*cookie, semid)) {
		log_error("Could not set notification semaphore "
			  "identified by cookie value %" PRIu32 " (0x%x)",
			  *cookie, *cookie);
		goto bad;
	}

	dmt->event_nr |= ~DM_UDEV_FLAGS_MASK & *cookie;
	dmt->cookie_set = 1;

	log_debug_activation("Udev cookie 0x%" PRIx32 " (semid %d) assigned to "
			     "%s task(%d) with flags%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s (0x%" PRIx16 ")",
			     *cookie, semid, _task_type_disp(dmt->type), dmt->type,
			     (flags & DM_UDEV_DISABLE_DM_RULES_FLAG) ? " DISABLE_DM_RULES" : "",
			     (flags & DM_UDEV_DISABLE_SUBSYSTEM_RULES_FLAG) ? " DISABLE_SUBSYSTEM_RULES" : "",
			     (flags & DM_UDEV_DISABLE_DISK_RULES_FLAG) ? " DISABLE_DISK_RULES" : "",
			     (flags & DM_UDEV_DISABLE_OTHER_RULES_FLAG) ? " DISABLE_OTHER_RULES" : "",
			     (flags & DM_UDEV_LOW_PRIORITY_FLAG) ? " LOW_PRIORITY" : "",
			     (flags & DM_UDEV_DISABLE_LIBRARY_FALLBACK) ? " DISABLE_LIBRARY_FALLBACK" : "",
			     (flags & DM_UDEV_PRIMARY_SOURCE_FLAG) ? " PRIMARY_SOURCE" : "",
			     (flags & DM_SUBSYSTEM_UDEV_FLAG0) ? " SUBSYSTEM_0" : " ",
			     (flags & DM_SUBSYSTEM_UDEV_FLAG1) ? " SUBSYSTEM_1" : " ",
			     (flags & DM_SUBSYSTEM_UDEV_FLAG2) ? " SUBSYSTEM_2" : " ",
			     (flags & DM_SUBSYSTEM_UDEV_FLAG3) ? " SUBSYSTEM_3" : " ",
			     (flags & DM_SUBSYSTEM_UDEV_FLAG4) ? " SUBSYSTEM_4" : " ",
			     (flags & DM_SUBSYSTEM_UDEV_FLAG5) ? " SUBSYSTEM_5" : " ",
			     (flags & DM_SUBSYSTEM_UDEV_FLAG6) ? " SUBSYSTEM_6" : " ",
			     (flags & DM_SUBSYSTEM_UDEV_FLAG7) ? " SUBSYSTEM_7" : " ",
			     flags);

	return 1;

bad:
	dmt->event_nr = 0;
	return 0;
}

int dm_udev_complete(uint32_t cookie)
{
	int semid;

	if (!cookie || !dm_udev_get_sync_support())
		return 1;

	if (!_get_cookie_sem(cookie, &semid))
		return_0;

	if (!_udev_notify_sem_dec(cookie, semid)) {
		log_error("Could not signal waiting process using notification "
			  "semaphore identified by cookie value %" PRIu32 " (0x%x)",
			  cookie, cookie);
		return 0;
	}

	return 1;
}

/*
 * If *nowait is set, return immediately leaving it set if the semaphore
 * is not ready to be decremented to 0.  *nowait is cleared if the wait
 * succeeds.
 */
static int _udev_wait(uint32_t cookie, int *nowait)
{
	int semid;
	struct sembuf sb = {0, 0, 0};
	int val;

	if (!cookie || !dm_udev_get_sync_support())
		return 1;

	if (!_get_cookie_sem(cookie, &semid))
		return_0;

	/* Return immediately if the semaphore value exceeds 1? */
	if (*nowait) {
		if ((val = semctl(semid, 0, GETVAL)) < 0) {
			log_error("semid %d: sem_ctl GETVAL failed for "
				  "cookie 0x%" PRIx32 ": %s",
				  semid, cookie, strerror(errno));
			return 0;		
		}

		if (val > 1)
			return 1;

		*nowait = 0;
	}

	if (!_udev_notify_sem_dec(cookie, semid)) {
		log_error("Failed to set a proper state for notification "
			  "semaphore identified by cookie value %" PRIu32 " (0x%x) "
			  "to initialize waiting for incoming notifications.",
			  cookie, cookie);
		(void) _udev_notify_sem_destroy(cookie, semid);
		return 0;
	}

	log_debug_activation("Udev cookie 0x%" PRIx32 " (semid %d) waiting for zero",
			     cookie, semid);

repeat_wait:
	if (semop(semid, &sb, 1) < 0) {
		if (errno == EINTR)
			goto repeat_wait;
		else if (errno == EIDRM)
			return 1;

		log_error("Could not set wait state for notification semaphore "
			  "identified by cookie value %" PRIu32 " (0x%x): %s",
			  cookie, cookie, strerror(errno));
		(void) _udev_notify_sem_destroy(cookie, semid);
		return 0;
	}

	return _udev_notify_sem_destroy(cookie, semid);
}

int dm_udev_wait(uint32_t cookie)
{
	int nowait = 0;
	int r = _udev_wait(cookie, &nowait);

	update_devs();

	return r;
}

int dm_udev_wait_immediate(uint32_t cookie, int *ready)
{
	int nowait = 1;
	int r = _udev_wait(cookie, &nowait);

	if (r && nowait) {
		*ready = 0;
		return 1;
	}

	update_devs();
	*ready = 1;

	return r;
}
#endif		/* UDEV_SYNC_SUPPORT */
