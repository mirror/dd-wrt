// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Oracle.  All Rights Reserved.
 * Author: Darrick J. Wong <darrick.wong@oracle.com>
 */
#include "xfs.h"
#include <pthread.h>
#include <sys/statvfs.h>
#include <syslog.h>
#include "platform_defs.h"
#include "libfrog/paths.h"
#include "xfs_scrub.h"
#include "common.h"
#include "progress.h"

extern char		*progname;

/*
 * Reporting Status to the Console
 *
 * We aim for a roughly standard reporting format -- the severity of the
 * status being reported, a textual description of the object being
 * reported, and whatever the status happens to be.
 *
 * Errors are the most severe and reflect filesystem corruption.
 * Warnings indicate that something is amiss and needs the attention of
 * the administrator, but does not constitute a corruption.  Information
 * is merely advisory.
 */

/* Too many errors? Bail out. */
bool
scrub_excessive_errors(
	struct scrub_ctx	*ctx)
{
	unsigned long long	errors_seen;

	/*
	 * We only set max_errors at the start of the program, so it's safe to
	 * access it locklessly.
	 */
	if (ctx->max_errors == 0)
		return false;

	pthread_mutex_lock(&ctx->lock);
	errors_seen = ctx->corruptions_found + ctx->unfixable_errors;
	pthread_mutex_unlock(&ctx->lock);

	return errors_seen >= ctx->max_errors;
}

static struct {
	const char *string;
	int loglevel;
} err_levels[] = {
	[S_ERROR]  = {
		.string = "Error",
		.loglevel = LOG_ERR,
	},
	[S_CORRUPT] = {
		.string = "Corruption",
		.loglevel = LOG_ERR,
	},
	[S_UNFIXABLE] = {
		.string = "Unfixable Error",
		.loglevel = LOG_ERR,
	},
	[S_WARN]   = {
		.string = "Warning",
		.loglevel = LOG_WARNING,
	},
	[S_INFO]   = {
		.string = "Info",
		.loglevel = LOG_INFO,
	},
	[S_REPAIR] = {
		.string = "Repaired",
		.loglevel = LOG_INFO,
	},
	[S_PREEN]  = {
		.string = "Optimized",
		.loglevel = LOG_INFO,
	},
};

/* If stream is a tty, clear to end of line to clean up progress bar. */
static inline const char *stream_start(FILE *stream)
{
	if (stream == stderr)
		return stderr_isatty ? CLEAR_EOL : "";
	return stdout_isatty ? CLEAR_EOL : "";
}

/* Print a warning string and some warning text. */
void
__str_out(
	struct scrub_ctx	*ctx,
	const char		*descr,
	enum error_level	level,
	int			error,
	const char		*file,
	int			line,
	const char		*format,
	...)
{
	FILE			*stream = stderr;
	va_list			args;
	char			buf[DESCR_BUFSZ];

	/* print strerror or format of choice but not both */
	assert(!(error && format));

	if (level >= S_INFO)
		stream = stdout;

	pthread_mutex_lock(&ctx->lock);

	/* We only want to hear about optimizing when in debug/verbose mode. */
	if (level == S_PREEN && !debug && !verbose)
		goto out_record;

	fprintf(stream, "%s%s: %s: ", stream_start(stream),
			_(err_levels[level].string), descr);
	if (error) {
		fprintf(stream, _("%s."), strerror_r(error, buf, DESCR_BUFSZ));
	} else {
		va_start(args, format);
		vfprintf(stream, format, args);
		va_end(args);
	}

	if (debug)
		fprintf(stream, _(" (%s line %d)"), file, line);
	fprintf(stream, "\n");
	if (stream == stdout)
		fflush(stream);

out_record:
	if (error || level == S_ERROR)      /* A syscall failed */
		ctx->runtime_errors++;
	else if (level == S_CORRUPT)
		ctx->corruptions_found++;
	else if (level == S_UNFIXABLE)
		ctx->unfixable_errors++;
	else if (level == S_WARN)
		ctx->warnings_found++;
	else if (level == S_REPAIR)
		ctx->repairs++;
	else if (level == S_PREEN)
		ctx->preens++;

	pthread_mutex_unlock(&ctx->lock);
}

/* Log a message to syslog. */
#define LOG_BUFSZ	4096
#define LOGNAME_BUFSZ	256
void
__str_log(
	struct scrub_ctx	*ctx,
	enum error_level	level,
	const char		*format,
	...)
{
	va_list			args;
	char			logname[LOGNAME_BUFSZ];
	char			buf[LOG_BUFSZ];
	int			sz;

	/* We only want to hear about optimizing when in debug/verbose mode. */
	if (level == S_PREEN && !debug && !verbose)
		return;

	/*
	 * Skip logging if we're being run as a service (presumably the
	 * service will log stdout/stderr); if we're being run in a non
	 * interactive manner (assume we're a service); or if we're in
	 * debug mode.
	 */
	if (is_service || !isatty(fileno(stdin)) || debug)
		return;

	snprintf(logname, LOGNAME_BUFSZ, "%s@%s", progname, ctx->mntpoint);
	openlog(logname, LOG_PID, LOG_DAEMON);

	sz = snprintf(buf, LOG_BUFSZ, "%s: ", _(err_levels[level].string));
	va_start(args, format);
	vsnprintf(buf + sz, LOG_BUFSZ - sz, format, args);
	va_end(args);
	syslog(err_levels[level].loglevel, "%s", buf);

	closelog();
}

double
timeval_subtract(
	struct timeval		*tv1,
	struct timeval		*tv2)
{
	return ((tv1->tv_sec - tv2->tv_sec) +
		((float) (tv1->tv_usec - tv2->tv_usec)) / 1000000);
}

/* Produce human readable disk space output. */
double
auto_space_units(
	unsigned long long	bytes,
	char			**units)
{
	if (debug > 1)
		goto no_prefix;
	if (bytes > (1ULL << 40)) {
		*units = "TiB";
		return (double)bytes / (1ULL << 40);
	} else if (bytes > (1ULL << 30)) {
		*units = "GiB";
		return (double)bytes / (1ULL << 30);
	} else if (bytes > (1ULL << 20)) {
		*units = "MiB";
		return (double)bytes / (1ULL << 20);
	} else if (bytes > (1ULL << 10)) {
		*units = "KiB";
		return (double)bytes / (1ULL << 10);
	}

no_prefix:
	*units = "B";
	return bytes;
}

/* Produce human readable discrete number output. */
double
auto_units(
	unsigned long long	number,
	char			**units,
	int			*precision)
{
	if (debug > 1)
		goto no_prefix;
	*precision = 1;
	if (number > 1000000000000ULL) {
		*units = "T";
		return number / 1000000000000.0;
	} else if (number > 1000000000ULL) {
		*units = "G";
		return number / 1000000000.0;
	} else if (number > 1000000ULL) {
		*units = "M";
		return number / 1000000.0;
	} else if (number > 1000ULL) {
		*units = "K";
		return number / 1000.0;
	}

no_prefix:
	*units = "";
	*precision = 0;
	return number;
}

/* How many threads to kick off? */
unsigned int
scrub_nproc(
	struct scrub_ctx	*ctx)
{
	if (force_nr_threads)
		return force_nr_threads;
	return ctx->nr_io_threads;
}

/*
 * How many threads to kick off for a workqueue?  If we only want one
 * thread, save ourselves the overhead and just run it in the main thread.
 */
unsigned int
scrub_nproc_workqueue(
	struct scrub_ctx	*ctx)
{
	unsigned int		x;

	x = scrub_nproc(ctx);
	if (x == 1)
		x = 0;
	return x;
}

/*
 * Sleep for 100us * however many -b we got past the initial one.
 * This is an (albeit clumsy) way to throttle scrub activity.
 */
void
background_sleep(void)
{
	unsigned long long	time_ns;
	struct timespec		tv;

	if (bg_mode < 2)
		return;

	time_ns =  100 * NSEC_PER_USEC * (bg_mode - 1);
	tv.tv_sec = time_ns / NSEC_PER_SEC;
	tv.tv_nsec = time_ns % NSEC_PER_SEC;
	nanosleep(&tv, NULL);
}

/*
 * Return the input string with non-printing bytes escaped.
 * Caller must free the buffer.
 */
char *
string_escape(
	const char		*in)
{
	char			*str;
	const char		*p;
	char			*q;
	int			x;

	str = malloc(strlen(in) * 4);
	if (!str)
		return NULL;
	for (p = in, q = str; *p != '\0'; p++) {
		if (isprint(*p)) {
			*q = *p;
			q++;
		} else {
			x = sprintf(q, "\\x%02x", *p);
			q += x;
		}
	}
	*q = '\0';
	return str;
}

/*
 * Record another naming warning, and decide if it's worth
 * complaining about.
 */
bool
should_warn_about_name(
	struct scrub_ctx	*ctx)
{
	bool			whine;
	bool			res;

	pthread_mutex_lock(&ctx->lock);
	ctx->naming_warnings++;
	whine = ctx->naming_warnings == TOO_MANY_NAME_WARNINGS;
	res = ctx->naming_warnings < TOO_MANY_NAME_WARNINGS;
	pthread_mutex_unlock(&ctx->lock);

	if (whine && !(debug || verbose))
		str_info(ctx, ctx->mntpoint,
_("More than %u naming warnings, shutting up."),
				TOO_MANY_NAME_WARNINGS);

	return debug || verbose || res;
}

/* Decide if a value is within +/- (n/d) of a desired value. */
bool
within_range(
	struct scrub_ctx	*ctx,
	unsigned long long	value,
	unsigned long long	desired,
	unsigned long long	abs_threshold,
	unsigned int		n,
	unsigned int		d,
	const char		*descr)
{
	assert(n < d);

	/* Don't complain if difference does not exceed an absolute value. */
	if (value < desired && desired - value < abs_threshold)
		return true;
	if (value > desired && value - desired < abs_threshold)
		return true;

	/* Complain if the difference exceeds a certain percentage. */
	if (value < desired * (d - n) / d)
		return false;
	if (value > desired * (d + n) / d)
		return false;

	return true;
}

/*
 * Render an inode number into a buffer in a format suitable for use in
 * log messages. The buffer will be filled with:
 * 	"inode <inode number> (<ag number>/<ag inode number>)"
 * If the @format argument is non-NULL, it will be rendered into the buffer
 * after the inode representation and a single space.
 */
int
scrub_render_ino_descr(
	const struct scrub_ctx	*ctx,
	char			*buf,
	size_t			buflen,
	uint64_t		ino,
	uint32_t		gen,
	const char		*format,
	...)
{
	va_list			args;
	uint32_t		agno;
	uint32_t		agino;
	int			ret;

	agno = cvt_ino_to_agno(&ctx->mnt, ino);
	agino = cvt_ino_to_agino(&ctx->mnt, ino);
	ret = snprintf(buf, buflen, _("inode %"PRIu64" (%"PRIu32"/%"PRIu32")%s"),
			ino, agno, agino, format ? " " : "");
	if (ret < 0 || ret >= buflen || format == NULL)
		return ret;

	va_start(args, format);
	ret += vsnprintf(buf + ret, buflen - ret, format, args);
	va_end(args);
	return ret;
}
