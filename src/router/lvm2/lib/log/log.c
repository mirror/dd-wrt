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

#include "lib/misc/lib.h"
#include "lib/device/device.h"
#include "lib/mm/memlock.h"
#include "lib/config/defaults.h"
#include "lib/report/report.h"
#include "lib/misc/lvm-file.h"

#include <stdio.h>
#include <stdarg.h>
#include <syslog.h>
#include <ctype.h>

static FILE *_log_file;
static char _log_file_path[PATH_MAX];
static struct device _log_dev;
static struct dm_str_list _log_dev_alias;

static int _syslog = 0;
static int _log_to_file = 0;
static uint64_t _log_file_max_lines = 0;
static uint64_t _log_file_lines = 0;
static int _log_direct = 0;
static int _log_while_suspended = 0;
static int _indent = 1;
static int _log_suppress = 0;
static char _msg_prefix[30] = "  ";
static int _already_logging = 0;
static int _abort_on_internal_errors_config = 0;

static lvm2_log_fn_t _lvm2_log_fn = NULL;

static int _lvm_errno = 0;
static int _store_errmsg = 0;
static char *_lvm_errmsg = NULL;
static size_t _lvm_errmsg_size = 0;
static size_t _lvm_errmsg_len = 0;
#define MAX_ERRMSG_LEN (512 * 1024)  /* Max size of error buffer 512KB */

static log_report_t _log_report = {
	.report = NULL,
	.context = LOG_REPORT_CONTEXT_NULL,
	.object_type = LOG_REPORT_OBJECT_TYPE_NULL,
	.object_id = NULL,
	.object_name = NULL,
	.object_group = NULL
};

#define LOG_STREAM_BUFFER_SIZE 4096

struct log_stream_item {
	FILE *stream;
	char *buffer;
};

static struct log_stream {
	struct log_stream_item out;
	struct log_stream_item err;
	struct log_stream_item report;
} _log_stream = {{NULL, NULL},
		 {NULL, NULL},
		 {NULL, NULL}};

#define out_stream (_log_stream.out.stream ? : stdout)
#define err_stream (_log_stream.err.stream ? : stderr)
#define report_stream (_log_stream.report.stream ? : stdout)

static int _set_custom_log_stream(struct log_stream_item *stream_item, int custom_fd)
{
	FILE *final_stream = NULL;
	int flags;
	int r = 1;

	if (custom_fd < 0)
		goto out;

	if (is_valid_fd(custom_fd)) {
		if ((flags = fcntl(custom_fd, F_GETFL)) > 0) {
			if ((flags & O_ACCMODE) == O_RDONLY) {
				log_error("File descriptor %d already open in read-only "
					  "mode, expected write-only or read-write mode.",
					   (int) custom_fd);
				r = 0;
				goto out;
			}
		}

		if (custom_fd == STDIN_FILENO) {
			log_error("Can't set standard input for log output.");
			r = 0;
			goto out;
		}

		if (custom_fd == STDOUT_FILENO) {
			final_stream = stdout;
			goto out;
		}

		if (custom_fd == STDERR_FILENO) {
			final_stream = stderr;
			goto out;
		}
	}

	if (!(final_stream = fdopen(custom_fd, "w"))) {
		log_error("Failed to open stream for file descriptor %d.",
			  (int) custom_fd);
		r = 0;
		goto out;
	}

	if (!(stream_item->buffer = malloc(LOG_STREAM_BUFFER_SIZE))) {
		log_error("Failed to allocate buffer for stream on file "
			  "descriptor %d.", (int) custom_fd);
	} else {
		if (setvbuf(final_stream, stream_item->buffer, _IOLBF, LOG_STREAM_BUFFER_SIZE)) {
			log_sys_error("setvbuf", "");
			free(stream_item->buffer);
			stream_item->buffer = NULL;
		}
	}
out:
	stream_item->stream = final_stream;
	return r;
}

int init_custom_log_streams(struct custom_fds *custom_fds)
{
	return _set_custom_log_stream(&_log_stream.out, custom_fds->out) &&
	       _set_custom_log_stream(&_log_stream.err, custom_fds->err) &&
	       _set_custom_log_stream(&_log_stream.report, custom_fds->report);
}

static void _check_and_replace_standard_log_streams(FILE *old_stream, FILE *new_stream)
{
	if (_log_stream.out.stream == old_stream)
		_log_stream.out.stream = new_stream;

	if (_log_stream.err.stream == old_stream)
		_log_stream.err.stream = new_stream;

	if (_log_stream.report.stream == old_stream)
		_log_stream.report.stream = new_stream;
}

/*
 * Close and reopen standard stream on file descriptor fd.
 */
int reopen_standard_stream(FILE **stream, const char *mode)
{
	int fd, fd_copy, new_fd;
	const char *name;
	FILE *old_stream = *stream;
	FILE *new_stream;

	if (old_stream == stdin) {
		fd = STDIN_FILENO;
		name = "stdin";
	} else if (old_stream == stdout) {
		fd = STDOUT_FILENO;
		name = "stdout";
	} else if (old_stream == stderr) {
		fd = STDERR_FILENO;
		name = "stderr";
	} else {
		log_error(INTERNAL_ERROR "reopen_standard_stream called on non-standard stream");
		return 0;
	}

	if ((fd_copy = dup(fd)) < 0) {
		log_sys_error("dup", name);
		return 0;
	}

	if (fclose(old_stream))
		log_sys_error("fclose", name);

	if ((new_fd = dup2(fd_copy, fd)) < 0)
		log_sys_error("dup2", name);
	else if (new_fd != fd)
		log_error("dup2(%d, %d) returned %d", fd_copy, fd, new_fd);

	if (close(fd_copy) < 0)
		log_sys_error("close", name);

	if (!(new_stream = fdopen(fd, mode))) {
		log_sys_error("fdopen", name);
		return 0;
	}

	_check_and_replace_standard_log_streams(old_stream, new_stream);

	*stream = new_stream;
	return 1;
}

void init_log_fn(lvm2_log_fn_t log_fn)
{
	_lvm2_log_fn = log_fn;
}

/*
 * Support envvar LVM_LOG_FILE_EPOCH and allow to attach
 * extra keyword (consist of upto 32 alpha chars) to
 * opened log file. After this 'epoch' word pid and starttime
 * (in kernel units, read from /proc/self/stat)
 * is automatically attached.
 * If command/daemon forks multiple times, it could create multiple
 * log files ensuring, there are no overwrites.
 */
void init_log_file(const char *log_file, int append)
{
	static const char statfile[] = "/proc/self/stat";
	const char *env;
	int pid;
	unsigned long long starttime;
	FILE *st;
	int i = 0;

	_log_file_path[0] = '\0';
	if ((env = getenv("LVM_LOG_FILE_EPOCH"))) {
		while (isalpha(env[i]) && i < 32) /* Up to 32 alphas */
			i++;
		if (env[i]) {
			if (i)
				log_warn("WARNING: Ignoring invalid LVM_LOG_FILE_EPOCH envvar \"%s\".", env);
			goto no_epoch;
		}

		if (!(st = fopen(statfile, "r")))
			log_sys_error("fopen", statfile);
		else if (fscanf(st, "%d %*s %*c %*d %*d %*d %*d " /* tty_nr */
			   "%*d %*u %*u %*u %*u " /* mjflt */
			   "%*u %*u %*u %*d %*d " /* cstim */
			   "%*d %*d %*d %*d " /* itrealvalue */
			   "%llu", &pid, &starttime) != 2) {
			log_warn("WARNING: Cannot parse content of %s.", statfile);
		} else {
			if (dm_snprintf(_log_file_path, sizeof(_log_file_path),
					"%s_%s_%d_%llu", log_file, env, pid, starttime) < 0) {
				log_warn("WARNING: Debug log file path is too long for epoch.");
				_log_file_path[0] = '\0';
			} else {
				log_file = _log_file_path;
				append = 1; /* force */
			}
		}

		if (st && fclose(st))
			log_sys_debug("fclose", statfile);

		if ((env = getenv("LVM_LOG_FILE_MAX_LINES"))) {
			if (sscanf(env, FMTu64, &_log_file_max_lines) != 1) {
				log_warn("WARNING: Ignoring invalid LVM_LOG_MAX_LINES envvar \"%s\".", env);
				_log_file_max_lines = 0;
			}
			_log_file_lines = 0;
		}
	}

no_epoch:
	if (!(_log_file = fopen(log_file, append ? "a" : "w"))) {
		log_sys_error("fopen", log_file);
		return;
	}

	_log_to_file = 1;
}

/*
 * Unlink the log file depeding on command's return value
 *
 * When envvar LVM_EXPECTED_EXIT_STATUS is set, compare
 * resulting status with this string.
 *
 * It's possible to specify 2 variants - having it equal to
 * a single number or having it different from a single number.
 *
 * i.e.  LVM_EXPECTED_EXIT_STATUS=">1"  # delete when ret > 1.
 */
void unlink_log_file(int ret)
{
	const char *env;

	if (_log_file_path[0] &&
	    (env = getenv("LVM_EXPECTED_EXIT_STATUS")) &&
	    ((env[0] == '>' && ret > atoi(env + 1)) ||
	     (atoi(env) == ret))) {
		if (unlink(_log_file_path))
			log_sys_error("unlink", _log_file_path);
		_log_file_path[0] = '\0';
	}
}

void init_log_direct(const char *log_file, int append)
{
	int open_flags = append ? 0 : O_TRUNC;

	dev_create_file(log_file, &_log_dev, &_log_dev_alias, 1);
	if (!dev_open_flags(&_log_dev, O_RDWR | O_CREAT | open_flags, 1, 0))
		return;

	_log_direct = 1;
}

void init_log_while_suspended(int log_while_suspended)
{
	_log_while_suspended = log_while_suspended;
}

void init_syslog(int facility)
{
	openlog("lvm", LOG_PID, facility);
	_syslog = 1;
}

int log_suppress(int suppress)
{
	int old_suppress = _log_suppress;

	_log_suppress = suppress;

	return old_suppress;
}

void release_log_memory(void)
{
	if (!_log_direct)
		return;

	free((char *) _log_dev_alias.str);
	_log_dev_alias.str = "activate_log file";
}

void fin_log(void)
{
	if (_log_direct) {
		(void) dev_close(&_log_dev);
		_log_direct = 0;
	}

	if (_log_to_file) {
		if (dm_fclose(_log_file)) {
			if (errno)
			      fprintf(err_stream, "failed to write log file: %s\n",
				      strerror(errno));
			else
			      fprintf(err_stream, "failed to write log file\n");

		}
		_log_to_file = 0;
	}
}

void fin_syslog(void)
{
	if (_syslog)
		closelog();
	_syslog = 0;
}

void init_msg_prefix(const char *prefix)
{
	if (prefix)
		/* Cut away too long prefix */
		(void) dm_strncpy(_msg_prefix, prefix, sizeof(_msg_prefix));
}

void init_indent(int indent)
{
	_indent = indent;
}

/* If present, environment setting will override this. */
void init_abort_on_internal_errors(int fatal)
{
	_abort_on_internal_errors_config = fatal;
}

void reset_lvm_errno(int store_errmsg)
{
	_lvm_errno = 0;

	if (_lvm_errmsg) {
		free(_lvm_errmsg);
		_lvm_errmsg = NULL;
		_lvm_errmsg_size = _lvm_errmsg_len = 0;
	}

	_store_errmsg = store_errmsg;
}

int stored_errno(void)
{
	return _lvm_errno;
}

const char *stored_errmsg(void)
{
	return _lvm_errmsg ? : "";
}

const char *stored_errmsg_with_clear(void)
{
	const char *rc = strdup(stored_errmsg());
	reset_lvm_errno(1);
	return rc;
}

static struct dm_hash_table *_duplicated = NULL;

void reset_log_duplicated(void) {
	if (_duplicated) {
		dm_hash_destroy(_duplicated);
		_duplicated = NULL;
	}
}

static const char *_get_log_level_name(int use_stderr, int level)
{
	static const char *log_level_names[] = {"",      /* unassigned */
						"",      /* unassigned */
						"fatal", /* _LOG_FATAL */
						"error", /* _LOG_ERROR */
						"warn",  /* _LOG_WARN */
						"notice",/* _LOG_NOTICE */
						"info",  /* _LOG_INFO */
						"debug"  /* _LOG_DEBUG */
						};
	if (level == _LOG_WARN && !use_stderr)
		return "print";

	return log_level_names[level];
}

const char *log_get_report_context_name(log_report_context_t context)
{
	static const char *log_context_names[LOG_REPORT_CONTEXT_COUNT] = {[LOG_REPORT_CONTEXT_NULL] = "",
									  [LOG_REPORT_CONTEXT_SHELL] = "shell",
									  [LOG_REPORT_CONTEXT_PROCESSING] = "processing"};
	return log_context_names[context];
}


const char *log_get_report_object_type_name(log_report_object_type_t object_type)
{
	static const char *log_object_type_names[LOG_REPORT_OBJECT_TYPE_COUNT] = {[LOG_REPORT_OBJECT_TYPE_NULL] = "",
										  [LOG_REPORT_OBJECT_TYPE_CMD] = "cmd",
										  [LOG_REPORT_OBJECT_TYPE_ORPHAN] = "orphan",
										  [LOG_REPORT_OBJECT_TYPE_PV] = "pv",
										  [LOG_REPORT_OBJECT_TYPE_LABEL] = "label",
										  [LOG_REPORT_OBJECT_TYPE_VG] = "vg",
										  [LOG_REPORT_OBJECT_TYPE_LV] = "lv"};
	return log_object_type_names[object_type];
}

__attribute__ ((format(printf, 5, 0)))
static void _vprint_log(int level, const char *file, int line, int dm_errno_or_class,
			const char *format, va_list orig_ap)
{
	va_list ap;
	char buf[1024], message[4096];
	int bufused, n;
	const char *trformat;		/* Translated format string */
	char *newbuf;
	int use_stderr = log_stderr(level);
	int log_once = log_once(level);
	int log_bypass_report = log_bypass_report(level);
	int fatal_internal_error = 0;
	size_t msglen;
	const char *indent_spaces = "";
	FILE *stream;
	static int _abort_on_internal_errors_env_present = -1;
	static int _abort_on_internal_errors_env = 0;
	char *env_str;
	struct dm_report *orig_report;
	int logged_via_report = 0;

	level = log_level(level);

	if (_abort_on_internal_errors_env_present < 0) {
		if ((env_str = getenv("DM_ABORT_ON_INTERNAL_ERRORS"))) {
			_abort_on_internal_errors_env_present = 1;
			/* Set when env DM_ABORT_ON_INTERNAL_ERRORS is not "0" */
			_abort_on_internal_errors_env = strcmp(env_str, "0");
		} else
			_abort_on_internal_errors_env_present = 0;
	}

	/* Use value from environment if present, otherwise use value from config. */
	if (((_abort_on_internal_errors_env_present && _abort_on_internal_errors_env) ||
	     (!_abort_on_internal_errors_env_present && _abort_on_internal_errors_config)) &&
	    !strncmp(format, INTERNAL_ERROR, sizeof(INTERNAL_ERROR) - 1)) {
		fatal_internal_error = 1;
		/* Internal errors triggering abort cannot be suppressed. */
		_log_suppress = 0;
		level = _LOG_FATAL;
	}

	if (level <= _LOG_ERR)
		init_error_message_produced(1);

	trformat = _(format);

	if (level < _LOG_DEBUG && dm_errno_or_class && !_lvm_errno)
		_lvm_errno = dm_errno_or_class;

	if (_lvm2_log_fn ||
	    (_store_errmsg && (level <= _LOG_ERR)) ||
	    (_log_report.report && !log_bypass_report && (use_stderr || (level <=_LOG_WARN))) ||
	    log_once) {
		va_copy(ap, orig_ap);
		n = vsnprintf(message, sizeof(message), trformat, ap);
		va_end(ap);

		/* When newer glibc returns >= sizeof(locn), we will just log what
                 * has fit into buffer, it's '\0' terminated string */
		if (n < 0) {
			fprintf(err_stream, _("vsnprintf failed: skipping external "
					      "logging function"));
			goto log_it;
		}
	}

/* FIXME Avoid pointless use of message buffer when it'll never be read! */
	if (_store_errmsg && (level <= _LOG_ERR) &&
	    _lvm_errmsg_len < MAX_ERRMSG_LEN) {
		msglen = strlen(message);
		if ((_lvm_errmsg_len + msglen + 1) >= _lvm_errmsg_size) {
			_lvm_errmsg_size = 2 * (_lvm_errmsg_len + msglen + 1);
			if ((newbuf = realloc(_lvm_errmsg,
						 _lvm_errmsg_size)))
				_lvm_errmsg = newbuf;
			else
				_lvm_errmsg_size = _lvm_errmsg_len;
		}
		if (_lvm_errmsg &&
		    (_lvm_errmsg_len + msglen + 2) < _lvm_errmsg_size) {
			/* prepend '\n' and copy with '\0' but do not count in */
                        if (_lvm_errmsg_len)
				_lvm_errmsg[_lvm_errmsg_len++] = '\n';
			memcpy(_lvm_errmsg + _lvm_errmsg_len, message, msglen + 1);
			_lvm_errmsg_len += msglen;
		}
	}

	if (log_once) {
		if (!_duplicated)
			_duplicated = dm_hash_create(128);
		if (_duplicated) {
			if (dm_hash_lookup(_duplicated, message))
				level = _LOG_NOTICE;
			else
				(void) dm_hash_insert(_duplicated, message, (void*)1);
		}
	}

	if (_log_report.report && !log_bypass_report && (use_stderr || (level <= _LOG_WARN))) {
		orig_report = _log_report.report;
		_log_report.report = NULL;
		if (!report_cmdlog(orig_report, _get_log_level_name(use_stderr, level),
				   log_get_report_context_name(_log_report.context),
				   log_get_report_object_type_name(_log_report.object_type),
				   _log_report.object_name, _log_report.object_id,
				   _log_report.object_group, _log_report.object_group_id,
				   message, _lvm_errno, 0))
			fprintf(err_stream, _("failed to report cmdstatus"));
		else
			logged_via_report = 1;

		_log_report.report = orig_report;
	}

	if (_lvm2_log_fn) {
		_lvm2_log_fn(level, file, line, 0, message);
		if (fatal_internal_error)
			abort();
		return;
	}

      log_it:
	if (!logged_via_report && ((verbose_level() >= level) && !_log_suppress)) {
		if (verbose_level() > _LOG_DEBUG) {
			(void) dm_snprintf(buf, sizeof(buf), "#%s:%-5d ",
					   file, line);
		} else
			buf[0] = '\0';

		if (_indent)
			switch (level) {
			case _LOG_NOTICE: indent_spaces = "  "; break;
			case _LOG_INFO:   indent_spaces = "    "; break;
			case _LOG_DEBUG:  indent_spaces = "      "; break;
			default: /* nothing to do */;
			}

		va_copy(ap, orig_ap);
		switch (level) {
		case _LOG_DEBUG:
			if (verbose_level() < _LOG_DEBUG)
				break;
			if (!debug_class_is_logged(dm_errno_or_class))
				break;
			if ((verbose_level() == level) &&
			    (strcmp("<backtrace>", format) == 0))
				break;
			/* fall through */
		default:
			/* Typically only log_warn goes to out_stream */
			stream = (use_stderr || (level != _LOG_WARN)) ? err_stream : out_stream;
			if (stream == err_stream)
				fflush(out_stream);
			fprintf(stream, "%s%s%s%s", buf, log_command_name(),
				_msg_prefix, indent_spaces);
			vfprintf(stream, trformat, ap);
			fputc('\n', stream);
		}
		va_end(ap);
	}

	if ((level > debug_level()) ||
	    (level >= _LOG_DEBUG && !debug_class_is_logged(dm_errno_or_class))) {
		if (fatal_internal_error)
			abort();
		return;
	}

	if (_log_to_file && (_log_while_suspended || !critical_section())) {
		fprintf(_log_file, "%s:%-5d %s%s", file, line, log_command_name(),
			_msg_prefix);

		va_copy(ap, orig_ap);
		vfprintf(_log_file, trformat, ap);
		va_end(ap);

		if (_log_file_max_lines && ++_log_file_lines >= _log_file_max_lines) {
			fprintf(_log_file, "\n%s:%-5d %sAborting. Command has reached limit "
				"for logged lines (LVM_LOG_FILE_MAX_LINES=" FMTu64 ").",
				file, line, _msg_prefix,
				_log_file_max_lines);
			fatal_internal_error = 1;
		}

		fputc('\n', _log_file);
		fflush(_log_file);
	}

	if (_syslog && (_log_while_suspended || !critical_section())) {
		va_copy(ap, orig_ap);
		vsyslog(level, trformat, ap);
		va_end(ap);
	}

	if (fatal_internal_error)
		abort();

	/* FIXME This code is unfinished - pre-extend & condense. */
	if (!_already_logging && _log_direct && critical_section()) {
		_already_logging = 1;
		memset(&buf, ' ', sizeof(buf));
		bufused = 0;
		if ((n = dm_snprintf(buf, sizeof(buf),
				      "%s:%-5d %s%s", file, line, log_command_name(),
				      _msg_prefix)) == -1)
			goto done;

		bufused += n;		/* n does not include '\0' */

		va_copy(ap, orig_ap);
		n = vsnprintf(buf + bufused, sizeof(buf) - bufused,
			      trformat, ap);
		va_end(ap);

		if (n < 0)
			goto done;

		bufused += n;
		if (n >= (int) sizeof(buf))
			bufused = sizeof(buf) - 1;
	      done:
		buf[bufused] = '\n';
		buf[sizeof(buf) - 1] = '\n';
		/* FIXME real size bufused */
		dev_append(&_log_dev, sizeof(buf), DEV_IO_LOG, buf);
		_already_logging = 0;
	}
}

void print_log(int level, const char *file, int line, int dm_errno_or_class,
	       const char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	_vprint_log(level, file, line, dm_errno_or_class, format, ap);
	va_end(ap);
}

void print_log_libdm(int level, const char *file, int line, int dm_errno_or_class,
		     const char *format, ...)
{
	FILE *orig_out_stream = out_stream;
	va_list ap;

	/*
	 * Bypass report if printing output from libdm and if we have
	 * LOG_WARN level and it's not going to stderr (so we're
	 * printing common message that is not an error/warning).
	*/
	if (!log_stderr(level) &&
	    (log_level(level) == _LOG_WARN))
		level |= _LOG_BYPASS_REPORT;

	_log_stream.out.stream = report_stream;

	va_start(ap, format);
	_vprint_log(level, file, line, dm_errno_or_class, format, ap);
	va_end(ap);

	_log_stream.out.stream = orig_out_stream;
}

log_report_t log_get_report_state(void)
{
	return _log_report;
}

void log_restore_report_state(log_report_t log_report)
{
	_log_report = log_report;
}

void log_set_report(struct dm_report *report)
{
	_log_report.report = report;
}

void log_set_report_context(log_report_context_t context)
{
	_log_report.context = context;
}

void log_set_report_object_type(log_report_object_type_t object_type)
{
	_log_report.object_type = object_type;
}

void log_set_report_object_group_and_group_id(const char *group, const char *id)
{
	_log_report.object_group = group;
	_log_report.object_group_id = id;
}

void log_set_report_object_name_and_id(const char *name, const char *id)
{
	_log_report.object_name = name;
	_log_report.object_id = id;
}
