/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#include "db_config.h"

#include "db_int.h"
#include "dbinc/db_page.h"
#include "dbinc/db_am.h"
#include "dbinc/lock.h"
#include "dbinc/mp.h"
#include "dbinc/txn.h"

#if defined(HAVE_ERROR_HISTORY)
static void __db_thread_once_func __P((void));
static void __db_deferred_free __P((void *));
#endif

/*
 * __db_fchk --
 *	General flags checking routine.
 *
 * PUBLIC: int __db_fchk __P((ENV *, const char *, u_int32_t, u_int32_t));
 */
int
__db_fchk(env, name, flags, ok_flags)
	ENV *env;
	const char *name;
	u_int32_t flags, ok_flags;
{
	return (LF_ISSET(~ok_flags) ? __db_ferr(env, name, 0) : 0);
}

/*
 * __db_fcchk --
 *	General combination flags checking routine.
 *
 * PUBLIC: int __db_fcchk
 * PUBLIC:    __P((ENV *, const char *, u_int32_t, u_int32_t, u_int32_t));
 */
int
__db_fcchk(env, name, flags, flag1, flag2)
	ENV *env;
	const char *name;
	u_int32_t flags, flag1, flag2;
{
	return (LF_ISSET(flag1) &&
	    LF_ISSET(flag2) ? __db_ferr(env, name, 1) : 0);
}

/*
 * __db_ferr --
 *	Common flag errors.
 *
 * PUBLIC: int __db_ferr __P((const ENV *, const char *, int));
 */
int
__db_ferr(env, name, iscombo)
	const ENV *env;
	const char *name;
	int iscombo;
{
	int ret;

	ret = USR_ERR(env, EINVAL);
	if (iscombo)
		__db_errx(env, DB_STR_A("0054",
		    "illegal flag combination specified to %s", "%s"), name);
	else
		__db_errx(env, DB_STR_A("0055",
		    "illegal flag specified to %s", "%s"), name);

	return (ret);
}

/*
 * __db_fnl --
 *	Common flag-needs-locking message.
 *
 * PUBLIC: int __db_fnl __P((const ENV *, const char *));
 */
int
__db_fnl(env, name)
	const ENV *env;
	const char *name;
{
	int ret;

	ret = USR_ERR(env, EINVAL);
	__db_errx(env, DB_STR_A("0056",
    "%s: DB_READ_COMMITTED, DB_READ_UNCOMMITTED and DB_RMW require locking",
	    "%s"), name);
	return (ret);
}

/*
 * __db_pgerr --
 *	Error when unable to retrieve a specified page.
 *
 * PUBLIC: int __db_pgerr __P((DB *, db_pgno_t, int));
 */
int
__db_pgerr(dbp, pgno, errval)
	DB *dbp;
	db_pgno_t pgno;
	int errval;
{
	/*
	 * Three things are certain:
	 * Death, taxes, and lost data.
	 * Guess which has occurred.
	 */
	__db_errx(dbp->env, DB_STR_A("0057",
	    "unable to create/retrieve page %lu", "%lu"), (u_long)pgno);
	return (__env_panic(dbp->env, errval));
}

/*
 * __db_pgfmt --
 *	Error when a page has the wrong format.
 *
 * PUBLIC: int __db_pgfmt __P((ENV *, db_pgno_t));
 */
int
__db_pgfmt(env, pgno)
	ENV *env;
	db_pgno_t pgno;
{
	__db_errx(env, DB_STR_A("0058",
	    "page %lu: illegal page type or format", "%lu"), (u_long)pgno);
	return (__env_panic(env, EINVAL));
}

#ifdef DIAGNOSTIC
/*
 * __db_assert --
 *	Error when an assertion fails.  Only checked if #DIAGNOSTIC defined.
 *
 * PUBLIC: #ifdef DIAGNOSTIC
 * PUBLIC: void __db_assert __P((ENV *, const char *, const char *, int));
 * PUBLIC: #endif
 */
void
__db_assert(env, e, file, line)
	ENV *env;
	const char *e, *file;
	int line;
{
	if (DB_GLOBAL(j_assert) != NULL)
		DB_GLOBAL(j_assert)(e, file, line);
	else {
		/*
		 * If a panic has preceded this assertion failure, print that
		 * message as well -- it might be relevant.
		 */
#ifdef HAVE_FAILCHK_BROADCAST
		if (PANIC_ISSET(env)) {
			REGENV *renv;
			renv = (env == NULL || env->reginfo == NULL) ?
				NULL : env->reginfo->primary;
			__db_errx(env, DB_STR_A("0242",
			    "assert failure (%s/%d: %s) after panic %s",
			    "%s %d %s %s"), file, line, e,
			    renv == NULL ? "" : renv->failure_symptom);
		} else
#endif
			__db_errx(env, DB_STR_A("0059",
			    "assert failure: %s/%d: \"%s\"",
			    "%s %d %s"), file, line, e);

		__os_abort(env);
		/* NOTREACHED */
	}
}
#endif

/*
 * __env_panic_event -
 *	Notify the application of a db_register, failchk, or generic panic.
 *
 * PUBLIC: void __env_panic_event __P((ENV *, int));
 */
void
__env_panic_event(env, errval)
	ENV *env;
	int errval;
{
	DB_ENV *dbenv;
	REGENV *renv;
	u_int32_t event;
	void *info;
	DB_EVENT_FAILCHK_INFO failinfo;

	dbenv = env->dbenv;
	info = &errval;
	if (dbenv->db_paniccall != NULL)	/* Deprecated */
		dbenv->db_paniccall(dbenv, errval);
	/*
	 * We check for DB_EVENT_FAILCHK and DB_EVENT_REG_PANIC first because
	 * they are not set by themselves. If one of those is set, it means that
	 * this panic is somewhat an expected consequence of a previous failure.
	 */
	renv = (env->reginfo == NULL) ? NULL : env->reginfo->primary;
	if (renv != NULL && renv->failure_panic) {
		event = DB_EVENT_FAILCHK_PANIC;
		failinfo.error = errval;
		(void)strncpy(failinfo.symptom,
		    renv->failure_symptom, sizeof(failinfo.symptom));
		failinfo.symptom[sizeof(failinfo.symptom) - 1] = '\0';
		info = &failinfo;
	} else if (renv != NULL && renv->reg_panic)
		event = DB_EVENT_REG_PANIC;
	else
		event = DB_EVENT_PANIC;
	DB_EVENT(env, event, info);
}

/*
 * __env_panic_msg --
 *	Report that we noticed a panic which had been set somewhere else.
 *
 * PUBLIC: int __env_panic_msg __P((ENV *));
 */
int
__env_panic_msg(env)
	ENV *env;
{
	int ret;

	ret = DB_RUNRECOVERY;
	/* Make a note saying where this panic was detected. */
	(void)USR_ERR(env, ret);

	__db_errx(env, DB_STR("0060",
	    "PANIC: fatal region error detected; run recovery"));

	__env_panic_event(env, ret);

	return (ret);
}

/*
 * __env_panic --
 *	Lock out the database environment due to unrecoverable error.
 *
 * PUBLIC: int __env_panic __P((ENV *, int));
 */
int
__env_panic(env, errval)
	ENV *env;
	int errval;
{
	if (env != NULL) {
		__env_panic_set(env, 1);

		if (errval != DB_RUNRECOVERY)
			__db_err(env, errval, DB_STR("0061", "PANIC"));

		__env_panic_event(env, errval);
	}

#if defined(DIAGNOSTIC) && !defined(CONFIG_TEST)
	/*
	 * We want a stack trace of how this could possibly happen.
	 *
	 * Don't drop core if it's the test suite -- it's reasonable for the
	 * test suite to check to make sure that DB_RUNRECOVERY is returned
	 * under certain conditions.
	 */
	__os_abort(env);
	/* NOTREACHED */
#endif

	/*
	 * Chaos reigns within.
	 * Reflect, repent, and reboot.
	 * Order shall return.
	 */
	return (DB_RUNRECOVERY);
}

/*
 * db_strerror --
 *	ANSI C strerror(3) for DB.
 *
 * EXTERN: char *db_strerror __P((int));
 */
char *
db_strerror(error)
	int error;
{
	char *p;

	if (error == 0)
		return (DB_STR("0062", "Successful return: 0"));
	if (error > 0) {
		if ((p = strerror(error)) != NULL)
			return (p);
		return (__db_unknown_error(error));
	}

	/*
	 * !!!
	 * The Tcl API requires that some of these return strings be compared
	 * against strings stored in application scripts.  So, any of these
	 * errors that do not invariably result in a Tcl exception may not be
	 * altered.
	 */
	switch (error) {
	case DB_BUFFER_SMALL:
		return (DB_STR("0063",
		    "DB_BUFFER_SMALL: User memory too small for return value"));
	case DB_DONOTINDEX:
		return (DB_STR("0064",
		    "DB_DONOTINDEX: Secondary index callback returns null"));
	case DB_FOREIGN_CONFLICT:
		return (DB_STR("0065",
       "DB_FOREIGN_CONFLICT: A foreign database constraint has been violated"));
	case DB_HEAP_FULL:
		return (DB_STR("0208","DB_HEAP_FULL: no free space in db"));
	case DB_KEYEMPTY:
		return (DB_STR("0066",
		    "DB_KEYEMPTY: Non-existent key/data pair"));
	case DB_KEYEXIST:
		return (DB_STR("0067",
		    "DB_KEYEXIST: Key/data pair already exists"));
	case DB_LOCK_DEADLOCK:
		return (DB_STR("0068",
		    "DB_LOCK_DEADLOCK: Locker killed to resolve a deadlock"));
	case DB_LOCK_NOTGRANTED:
		return (DB_STR("0069", "DB_LOCK_NOTGRANTED: Lock not granted"));
	case DB_LOG_BUFFER_FULL:
		return (DB_STR("0070",
		    "DB_LOG_BUFFER_FULL: In-memory log buffer is full"));
	case DB_LOG_VERIFY_BAD:
		return (DB_STR("0071",
		    "DB_LOG_VERIFY_BAD: Log verification failed"));
	case DB_META_CHKSUM_FAIL:
		return (DB_STR("0247",
"DB_META_CHKSUM_FAIL: Checksum mismatch detected on a database metadata page"));
	case DB_NOSERVER:
		return (DB_STR("0072",
    "DB_NOSERVER: No message dispatch call-back function has been configured"));
	case DB_NOTFOUND:
		return (DB_STR("0073",
		    "DB_NOTFOUND: No matching key/data pair found"));
	case DB_OLD_VERSION:
		return (DB_STR("0074",
		    "DB_OLDVERSION: Database requires a version upgrade"));
	case DB_PAGE_NOTFOUND:
		return (DB_STR("0075",
		    "DB_PAGE_NOTFOUND: Requested page not found"));
	case DB_REP_DUPMASTER:
		return (DB_STR("0076",
		    "DB_REP_DUPMASTER: A second master site appeared"));
	case DB_REP_HANDLE_DEAD:
		return (DB_STR("0077",
		    "DB_REP_HANDLE_DEAD: Handle is no longer valid"));
	case DB_REP_HOLDELECTION:
		return (DB_STR("0078",
		    "DB_REP_HOLDELECTION: Need to hold an election"));
	case DB_REP_IGNORE:
		return (DB_STR("0079",
		    "DB_REP_IGNORE: Replication record/operation ignored"));
	case DB_REP_ISPERM:
		return (DB_STR("0080",
		    "DB_REP_ISPERM: Permanent record written"));
	case DB_REP_JOIN_FAILURE:
		return (DB_STR("0081",
		    "DB_REP_JOIN_FAILURE: Unable to join replication group"));
	case DB_REP_LEASE_EXPIRED:
		return (DB_STR("0082",
		    "DB_REP_LEASE_EXPIRED: Replication leases have expired"));
	case DB_REP_LOCKOUT:
		return (DB_STR("0083",
	    "DB_REP_LOCKOUT: Waiting for replication recovery to complete"));
	case DB_REP_NEWSITE:
		return (DB_STR("0084",
		    "DB_REP_NEWSITE: A new site has entered the system"));
	case DB_REP_NOTPERM:
		return (DB_STR("0085",
		    "DB_REP_NOTPERM: Permanent log record not written"));
	case DB_REP_UNAVAIL:
		return (DB_STR("0086",
	    "DB_REP_UNAVAIL: Too few remote sites to complete operation"));
	case DB_REP_WOULDROLLBACK:	/* Undocumented; C API only. */
		return (DB_STR("0207",
			"DB_REP_WOULDROLLBACK: Client data has diverged"));
	case DB_RUNRECOVERY:
		return (DB_STR("0087",
		    "DB_RUNRECOVERY: Fatal error, run database recovery"));
	case DB_SECONDARY_BAD:
		return (DB_STR("0088",
	    "DB_SECONDARY_BAD: Secondary index inconsistent with primary"));
	case DB_SLICE_CORRUPT:
		return (DB_STR("0251",
    "DB_SLICE_CORRUPT: One or more slices of this environment are malformed"));
	case DB_TIMEOUT:
		return (DB_STR("0089", "DB_TIMEOUT: Operation timed out"));
	case DB_VERIFY_BAD:
		return (DB_STR("0090",
		    "DB_VERIFY_BAD: Database verification failed"));
	case DB_VERSION_MISMATCH:
		return (DB_STR("0091",
	    "DB_VERSION_MISMATCH: Database environment version mismatch"));
	default:
		break;
	}

	return (__db_unknown_error(error));
}

/*
 * __db_unknown_error --
 *	Format an unknown error value into a static buffer.
 *
 * PUBLIC: char *__db_unknown_error __P((int));
 */
char *
__db_unknown_error(error)
	int error;
{
	/*
	 * !!!
	 * Room for a 64-bit number + slop.  This buffer is only used
	 * if we're given an unknown error number, which should never
	 * happen.
	 *
	 * We're no longer thread-safe if it does happen, but the worst
	 * result is a corrupted error string because there will always
	 * be a trailing nul byte since the error buffer is nul filled
	 * and longer than any error message.
	 */
	(void)snprintf(DB_GLOBAL(error_buf),
	    sizeof(DB_GLOBAL(error_buf)), DB_STR_A("0092",
	    "Unknown error: %d", "%d"), error);
	return (DB_GLOBAL(error_buf));
}

/*
 * __db_syserr --
 *	Standard error routine.
 *
 * PUBLIC: void __db_syserr __P((const ENV *, int, const char *, ...))
 * PUBLIC:    __attribute__ ((__format__ (__printf__, 3, 4)));
 */
void
__db_syserr(const ENV *env, int error, const char *fmt, ...)
{
	DB_ENV *dbenv;

	dbenv = env == NULL ? NULL : env->dbenv;
	if (env != NULL)
		(void)USR_ERR(env, error);

	/*
	 * The same as DB->err, except we don't default to writing to stderr
	 * after any output channel has been configured, and we use a system-
	 * specific function to translate errors to strings.
	 */
	DB_REAL_ERR(dbenv,
	    error, error == 0 ? DB_ERROR_NOT_SET : DB_ERROR_SYSTEM, 0, fmt);
}

/*
 * __db_err --
 *	Standard error routine with an error code.
 *
 * PUBLIC: void __db_err __P((const ENV *, int, const char *, ...))
 * PUBLIC:    __attribute__ ((__format__ (__printf__, 3, 4)));
 */
void
__db_err(const ENV *env, int error, const char *fmt, ...)
{
	DB_ENV *dbenv;

	dbenv = env == NULL ? NULL : env->dbenv;

	/* (If no deferred messages yet, at least?) add this calls' info.
	(void)USR_ERR(env, error);
	*/

	/*
	 * The same as DB->err, except we don't default to writing to stderr
	 * once an output channel has been configured.
	 */
	DB_REAL_ERR(dbenv, error, DB_ERROR_SET, 0, fmt);
}

/*
 * __db_errx --
 *	Standard error routine without any error code.
 *
 * PUBLIC: void __db_errx __P((const ENV *, const char *, ...))
 * PUBLIC:    __attribute__ ((__format__ (__printf__, 2, 3)));
 */
void
__db_errx(const ENV *env, const char *fmt, ...)
{
	DB_ENV *dbenv;

	dbenv = env == NULL ? NULL : env->dbenv;

	/*
	 * The same as DB->errx, except we don't default to writing to stderr
	 * once an output channel has been configured.
	 */
	DB_REAL_ERR(dbenv, 0, DB_ERROR_NOT_SET, 0, fmt);
}

/*
 * __db_errcall --
 *	Do the error message work for callback functions.
 *
 * PUBLIC: void __db_errcall
 * PUBLIC:    __P((const DB_ENV *, int, db_error_set_t, const char *, va_list));
 */
void
__db_errcall(dbenv, error, error_set, fmt, ap)
	const DB_ENV *dbenv;
	int error;
	db_error_set_t error_set;
	const char *fmt;
	va_list ap;
{
	char *end, *p;
	char buf[2048 + DB_ERROR_HISTORY_SIZE];
	char sysbuf[1024];
#ifdef HAVE_ERROR_HISTORY
	DB_MSGBUF *deferred_mb;
	ptrdiff_t len;
#endif

	p = buf;
	/* Reserve 1 byte at the end for '\0'. */
	end = buf + sizeof(buf) - 1;
	if (fmt != NULL)
		p += vsnprintf(buf, sizeof(buf), fmt, ap);

	if (error_set != DB_ERROR_NOT_SET)
		p += snprintf(p, (size_t)(end - p), ": %s",
		    error_set == DB_ERROR_SET ? db_strerror(error) :
		    __os_strerror(error, sysbuf, sizeof(sysbuf)));

#ifdef HAVE_ERROR_HISTORY
	/*
	 * Append any messages (e.g., diagnostics) stashed away in the deferred
	 * msgbuf. Strncpy() can't be trusted to append '\0', do it "manually".
	 */
	if ((deferred_mb = __db_deferred_get()) != NULL &&
	    (len = deferred_mb->cur - deferred_mb->buf) != 0) {
		p += snprintf(p,
		    (size_t)(end - p), "\nErrors during this API call:");
		if (len > (end - p))
			len = end - p;
		if (len != 0) {
			memmove(p, deferred_mb->buf, (size_t)len);
			p[len] = '\0';
		}
	}
#endif

	dbenv->db_errcall(dbenv, dbenv->db_errpfx, buf);
}

/*
 * __db_errfile --
 *	Do the error message work for FILE *s. Combine the messages into a
 *	single fprintf() call, to avoid interspersed output when there are
 *	multiple active threads.
 *
 *	Display a ": " after the dbenv prefix, if it has one.
 *	Display a ": " before the error message string, if it error was set.
 *
 * PUBLIC: void __db_errfile
 * PUBLIC:    __P((const DB_ENV *, int, db_error_set_t, const char *, va_list));
 */
void
__db_errfile(dbenv, error, error_set, fmt, ap)
	const DB_ENV *dbenv;
	int error;
	db_error_set_t error_set;
	const char *fmt;
	va_list ap;
{
	FILE *fp;
	char *defintro, *defmsgs, *error_str, *prefix, *sep1, *sep2;
	char sysbuf[200];
	char prefix_buf[200];
	char full_fmt[4096];
#ifdef HAVE_ERROR_HISTORY
	DB_MSGBUF *deferred_mb;
	size_t room;
#endif

	prefix = sep1 = sep2 = error_str = "";
	fp = dbenv == NULL ||
	    dbenv->db_errfile == NULL ? stderr : dbenv->db_errfile;
	if (fmt == NULL)
		fmt = "";

	if (dbenv != NULL && dbenv->db_errpfx != NULL) {
		/*
		 * Double each '%' in user-defined prefix so that
		 * we can get the right output with printf.
		 */
		prefix = __db_fmt_quote(prefix_buf,
		    sizeof(prefix_buf), dbenv->db_errpfx);
		sep1 = ": ";
	}
	switch (error_set) {
	case DB_ERROR_NOT_SET:
		break;
	case DB_ERROR_SET:
		error_str = db_strerror(error);
		sep2 = ": ";
		break;
	case DB_ERROR_SYSTEM:
		error_str = __os_strerror(error, sysbuf, sizeof(sysbuf));
		sep2 = ": ";
		break;
	}
#ifdef HAVE_ERROR_HISTORY
	if ((deferred_mb = __db_deferred_get()) != NULL &&
	    deferred_mb->cur != deferred_mb->buf) {
		defmsgs =
		    __db_fmt_quote(deferred_mb->buf, deferred_mb->len, NULL);
		defintro = "\nErrors during this API call:";
		/*
		 * If there are more deferred messages than will be displayed
		 * change the introductory message to warn of the truncation.
		 */
		room = sizeof(full_fmt) - (strlen(sep1) +
		    strlen(fmt) + strlen(sep2) + strlen(error_str));
		if (deferred_mb->len + strlen(defintro) > room) {
			defintro =
			    "\nFirst recorded errors during this API call:";
			memmove(defmsgs + room - 4, "...\n", 4);
		}

	} else
#endif
		defmsgs = defintro = "";
	(void)snprintf(full_fmt, sizeof(full_fmt), "%s%s%s%s%s%s%s\n", prefix,
	    sep1, fmt, sep2, error_str, defintro, defmsgs);
	(void)vfprintf(fp, full_fmt, ap);
	(void)fflush(fp);
}

/*
 * __db_msgadd --
 *	Aggregate a set of strings into a buffer for the callback API.
 *
 * PUBLIC: void __db_msgadd __P((const ENV *, DB_MSGBUF *, const char *, ...))
 * PUBLIC:    __attribute__ ((__format__ (__printf__, 3, 4)));
 */
void
__db_msgadd(const ENV *env, DB_MSGBUF *mbp, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	__db_msgadd_ap(env, mbp, fmt, ap);
	va_end(ap);
}

/*
 * __db_msgadd_ap --
 *	Aggregate a set of strings into a buffer for the callback API.
 *
 * PUBLIC: void __db_msgadd_ap
 * PUBLIC:     __P((const ENV *, DB_MSGBUF *, const char *, va_list));
 */
void
__db_msgadd_ap(env, mbp, fmt, ap)
	const ENV *env;
	DB_MSGBUF *mbp;
	const char *fmt;
	va_list ap;
{
	size_t len, nlen, olen;
	char buf[2048];

	len = (size_t)vsnprintf(buf, sizeof(buf), fmt, ap);

	/*
	 * There's a heap buffer in the ENV handle we use to aggregate the
	 * message chunks.  We maintain a pointer to the buffer, the next slot
	 * to be filled in in the buffer, and a total buffer length.
	 */
	olen = (size_t)(mbp->cur - mbp->buf);
	if (olen + len >= mbp->len) {
		/* Don't write too much for preallocated DB_MSGBUFs. */
		if (F_ISSET(mbp, DB_MSGBUF_PREALLOCATED)) {
			memset(mbp->cur, '*', mbp->len - olen);
			mbp->cur = mbp->buf + mbp->len;
			return;
		}
		nlen = mbp->len + len + (env == NULL ? 8192 : 256);
		if (__os_realloc(env, nlen, &mbp->buf))
			return;
		mbp->len = nlen;
		mbp->cur = mbp->buf + olen;
	}

	memcpy(mbp->cur, buf, len + 1);
	mbp->cur += len;
}

/*
 * __db_msg --
 *	Standard DB stat message routine.
 *
 * PUBLIC: void __db_msg __P((const ENV *, const char *, ...))
 * PUBLIC:    __attribute__ ((__format__ (__printf__, 2, 3)));
 */
void
__db_msg(const ENV *env, const char *fmt, ...)
{
	DB_ENV *dbenv;

	dbenv = env == NULL ? NULL : env->dbenv;

	DB_REAL_MSG(dbenv, fmt);
}

/*
 * __db_debug_msg --
 *	Save a message to be displayed only if this API call returns an error.
 *	The message is discarded if this API call succeeds.
 *
 * PUBLIC: void __db_debug_msg __P((const ENV *, const char *, ...));
 */
void
__db_debug_msg(const ENV *env, const char *fmt, ...)
{
#ifdef HAVE_ERROR_HISTORY
	DB_MSGBUF *mb;
	va_list ap;

	if (env == NULL || (mb = __db_deferred_get()) == NULL)
		return;

	va_start(ap, fmt);
	__db_msgadd_ap(env, mb, fmt, ap);
	va_end(ap);
#endif
	COMPQUIET(env, NULL);
	COMPQUIET(fmt, NULL);
}

/*
 * __db_repmsg --
 *	Replication system message routine.
 *
 * PUBLIC: void __db_repmsg __P((const ENV *, const char *, ...))
 * PUBLIC:    __attribute__ ((__format__ (__printf__, 2, 3)));
 */
void
__db_repmsg(const ENV *env, const char *fmt, ...)
{
	va_list ap;
	char buf[2048];

	va_start(ap, fmt);
	(void)vsnprintf(buf, sizeof(buf), fmt, ap);
	__rep_msg(env, buf);
	va_end(ap);
}

/*
 * __db_msgcall --
 *	Do the non-error message work for callback functions.
 *
 * PUBLIC: void __db_msgcall
 * PUBLIC:    __P((const DB_ENV *, const char *, va_list));
 */
void
__db_msgcall(dbenv, fmt, ap)
	const DB_ENV *dbenv;
	const char *fmt;
	va_list ap;
{
	char buf[2048];

	(void)vsnprintf(buf, sizeof(buf), fmt, ap);
	dbenv->db_msgcall(dbenv, dbenv->db_msgpfx, buf);
}

/*
 * __db_msgfile --
 *	Do the non-error message work for FILE *s. Combine the messages into
 *	a single fprintf() call, to avoid interspersed output when there are
 *	multiple active threads.
 *
 *	Display a ": " after the dbenv prefix, if it has one.
 *
 * PUBLIC: void __db_msgfile
 * PUBLIC:    __P((const DB_ENV *, const char *, va_list));
 */
void
__db_msgfile(dbenv, fmt, ap)
	const DB_ENV *dbenv;
	const char *fmt;
	va_list ap;
{
	FILE *fp;
	char *prefix, *sep;
	char prefix_buf[200];
	char full_fmt[4096];

	prefix = sep = "";
	fp = dbenv == NULL ||
	    dbenv->db_msgfile == NULL ? stdout : dbenv->db_msgfile;

	if (dbenv != NULL && dbenv->db_msgpfx != NULL) {
		/*
		 * Double each '%' in user-defined prefix so that
		 * we can get the right output with printf.
		 */
		prefix = __db_fmt_quote(prefix_buf,
		    sizeof(prefix_buf), dbenv->db_msgpfx);
		sep = ": ";
	}

	(void)snprintf(full_fmt, sizeof(full_fmt), "%s%s%s\n",
	    prefix, sep, fmt);
	(void)vfprintf(fp, full_fmt, ap);
	(void)fflush(fp);
}

/*
 * __db_unknown_flag -- report internal error
 *
 * PUBLIC: int __db_unknown_flag __P((ENV *, char *, u_int32_t));
 */
int
__db_unknown_flag(env, routine, flag)
	ENV *env;
	char *routine;
	u_int32_t flag;
{
	int ret;

	ret = USR_ERR(env, EINVAL);
	__db_errx(env, DB_STR_A("0093", "%s: Unknown flag: %#x", "%s %#x"),
	    routine, (u_int)flag);

#ifdef DIAGNOSTIC
	__os_abort(env);
	/* NOTREACHED */
#endif
	return (ret);
}

/*
 * __db_unknown_type -- report internal database type error
 *
 * PUBLIC: int __db_unknown_type __P((ENV *, char *, DBTYPE));
 */
int
__db_unknown_type(env, routine, type)
	ENV *env;
	char *routine;
	DBTYPE type;
{
	int ret;

	ret = USR_ERR(env, EINVAL);
	__db_errx(env, DB_STR_A("0094", "%s: Unexpected database type: %s",
	    "%s %s"), routine, __db_dbtype_to_string(type));

#ifdef DIAGNOSTIC
	__os_abort(env);
	/* NOTREACHED */
#endif
	return (ret);
}

/*
 * __db_unknown_path -- report unexpected database code path error.
 *
 * PUBLIC: int __db_unknown_path __P((ENV *, char *));
 */
int
__db_unknown_path(env, routine)
	ENV *env;
	char *routine;
{
	int ret;

	ret = USR_ERR(env, EINVAL);
	__db_errx(env, DB_STR_A("0095", "%s: Unexpected code path error",
	    "%s"), routine);

#ifdef DIAGNOSTIC
	__os_abort(env);
	/* NOTREACHED */
#endif
	return (ret);
}

/*
 * __db_check_txn --
 *	Check for common transaction errors.
 *
 * PUBLIC: int __db_check_txn __P((DB *, DB_TXN *, DB_LOCKER *, int));
 */
int
__db_check_txn(dbp, txn, assoc_locker, read_op)
	DB *dbp;
	DB_TXN *txn;
	DB_LOCKER *assoc_locker;
	int read_op;
{
	ENV *env;
	int related, ret;

	env = dbp->env;

	/*
	 * If we are in recovery or aborting a transaction, then we
	 * don't need to enforce the rules about dbp's not allowing
	 * transactional operations in non-transactional dbps and
	 * vica-versa.  This happens all the time as the dbp during
	 * an abort may be transactional, but we undo operations
	 * outside a transaction since we're aborting.
	 */
	if (IS_RECOVERING(env) || F_ISSET(dbp, DB_AM_RECOVER))
		return (0);

	if (txn != NULL && dbp->blob_threshold &&
	    F_ISSET(txn, (TXN_READ_UNCOMMITTED | TXN_SNAPSHOT))) {
		ret = USR_ERR(env, EINVAL);
		__db_errx(env, DB_STR("0237",
"External file databases do not support DB_READ_UNCOMMITTED and TXN_SNAPSHOT")
		    );
		return (ret);
	}

	/*
	 * Check for common transaction errors:
	 *	an operation on a handle whose open commit hasn't completed.
	 *	a transaction handle in a non-transactional environment
	 *	a transaction handle for a non-transactional database
	 */
	if (!read_op && txn != NULL && F_ISSET(txn, TXN_READONLY)) {
		ret = USR_ERR(env, EINVAL);
		__db_errx(env, DB_STR("0096",
		    "Read-only transaction cannot be used for an update"));
	} else if (txn == NULL || F_ISSET(txn, TXN_PRIVATE)) {
		if (dbp->cur_locker != NULL &&
		    dbp->cur_locker->id >= TXN_MINIMUM)
			goto open_err;

		if (!read_op && F_ISSET(dbp, DB_AM_TXN)) {
			ret = USR_ERR(env, EINVAL);
			__db_errx(env, DB_STR("0097",
		    "Transaction not specified for a transactional database"));
			return (ret);
		}
	} else if (F_ISSET(txn, TXN_FAMILY)) {
		/*
		 * Family transaction handles can be passed to any method,
		 * since they only determine locker IDs.
		 */
		return (0);
	} else {
		if (!TXN_ON(env))
			 return (__db_not_txn_env(env));

		if (!F_ISSET(dbp, DB_AM_TXN)) {
			ret = USR_ERR(env, EINVAL);
			__db_errx(env, DB_STR("0098",
		    "Transaction specified for a non-transactional database"));
			return (ret);
		}

		if (F_ISSET(txn, TXN_DEADLOCK))
			return (__db_txn_deadlock_err(env, txn));

		if (dbp->cur_locker != NULL &&
		    dbp->cur_locker->id >= TXN_MINIMUM &&
		     dbp->cur_locker->id != txn->txnid) {
			if ((ret = __lock_locker_same_family(env,
			     dbp->cur_locker, txn->locker, &related)) != 0)
				return (ret);
			if (!related)
				goto open_err;
		}
	}

	/*
	 * If dbp->associate_locker is not NULL, that means we're in
	 * the middle of a DB->associate with DB_CREATE (i.e., a secondary index
	 * creation).
	 *
	 * In addition to the usual transaction rules, we need to lock out
	 * non-transactional updates that aren't part of the associate (and
	 * thus are using some other locker ID).
	 *
	 * Transactional updates should simply block;  from the time we
	 * decide to build the secondary until commit, we'll hold a write
	 * lock on all of its pages, so it should be safe to attempt to update
	 * the secondary in another transaction (presumably by updating the
	 * primary).
	 */
	if (!read_op && dbp->associate_locker != NULL &&
	    txn != NULL && dbp->associate_locker != assoc_locker) {
		ret = USR_ERR(env, EINVAL);
		__db_errx(env, DB_STR("0099",
	    "Operation forbidden while secondary index is being created"));
		return (ret);
	}

	/*
	 * Check the txn and dbp are from the same env.
	 */
	if (txn != NULL && env != txn->mgrp->env) {
		ret = USR_ERR(env, EINVAL);
		__db_errx(env, DB_STR("0100",
		    "Transaction and database from different environments"));
		return (ret);
	}

	return (0);
open_err:
	ret = USR_ERR(env, EINVAL);
	if (F2_ISSET(dbp, DB2_AM_EXCL))
	    __db_errx(env, DB_STR("0209",
"Exclusive database handles can only have one active transaction at a time."));
	else
		__db_errx(env, DB_STR("0101",
		    "Transaction that opened the DB handle is still active"));
	return (ret);
}

/*
 * __db_txn_deadlock_err --
 *	Transaction has allready been deadlocked.
 *
 * PUBLIC: int __db_txn_deadlock_err __P((ENV *, DB_TXN *));
 */
int
__db_txn_deadlock_err(env, txn)
	ENV *env;
	DB_TXN *txn;
{
	const char *name;
	int ret;

	ret = USR_ERR(env, EINVAL);
	name = NULL;
	(void)__txn_get_name(txn, &name);

	__db_errx(env, DB_STR_A("0102",
	    "%s%sprevious transaction deadlock return not resolved",
	    "%s %s"), name == NULL ? "" : name, name == NULL ? "" : ": ");

	return (ret);
}

/*
 * __db_not_txn_env --
 *	DB handle must be in an environment that supports transactions.
 *
 * PUBLIC: int __db_not_txn_env __P((ENV *));
 */
int
__db_not_txn_env(env)
	ENV *env;
{
	int ret;

	ret = USR_ERR(env, EINVAL);
	__db_errx(env, DB_STR("0103",
	    "DB environment not configured for transactions"));
	return (ret);
}

/*
 * __db_rec_toobig --
 *	Fixed record length exceeded error message.
 *
 * PUBLIC: int __db_rec_toobig __P((ENV *, u_int32_t, u_int32_t));
 */
int
__db_rec_toobig(env, data_len, fixed_rec_len)
	ENV *env;
	u_int32_t data_len, fixed_rec_len;
{
	int ret;

	ret = USR_ERR(env, EINVAL);
	__db_errx(env, DB_STR_A("0104",
	    "%lu larger than database's maximum record length %lu",
	    "%lu %lu"), (u_long)data_len, (u_long)fixed_rec_len);
	return (ret);
}

/*
 * __db_rec_repl --
 *	Fixed record replacement length error message.
 *
 * PUBLIC: int __db_rec_repl __P((ENV *, u_int32_t, u_int32_t));
 */
int
__db_rec_repl(env, data_size, data_dlen)
	ENV *env;
	u_int32_t data_size, data_dlen;
{
	int ret;

	ret = USR_ERR(env, EINVAL);
	__db_errx(env, DB_STR_A("0105",
	    "Record length error: "
	    "replacement length %lu differs from replaced length %lu",
	    "%lu %lu"), (u_long)data_size, (u_long)data_dlen);
	return (ret);
}

#if defined(DIAGNOSTIC) || defined(DEBUG_ROP)  || defined(DEBUG_WOP)
/*
 * __dbc_logging --
 *	In DIAGNOSTIC mode, check for bad replication combinations.
 *
 * PUBLIC: int __dbc_logging __P((DBC *));
 */
int
__dbc_logging(dbc)
	DBC *dbc;
{
	DB_REP *db_rep;
	ENV *env;
	int ret;

	env = dbc->env;
	db_rep = env->rep_handle;

	ret = LOGGING_ON(env) &&
	    !F_ISSET(dbc, DBC_RECOVER) && !IS_REP_CLIENT(env);

	/*
	 * If we're not using replication or running recovery, return.
	 */
	if (db_rep == NULL || F_ISSET(dbc, DBC_RECOVER))
		return (ret);

#ifndef	DEBUG_ROP
	/*
	 *  Only check when DEBUG_ROP is not configured.  People often do
	 * non-transactional reads, and debug_rop is going to write
	 * a log record.
	 */
	{
	REP *rep;

	rep = db_rep->region;

	/*
	 * If we're a client and not running recovery or non durably, error.
	 */
	if (IS_REP_CLIENT(env) && !F_ISSET(dbc->dbp, DB_AM_NOT_DURABLE)) {
		__db_errx(env, DB_STR("0106",
		    "dbc_logging: Client update"));
		goto err;
	}

#ifndef DEBUG_WOP
	/*
	 * If DEBUG_WOP is enabled, then we'll generate debugging log records
	 * that are non-transactional.  This is OK.
	 */
	if (IS_REP_MASTER(env) &&
	    dbc->txn == NULL && !F_ISSET(dbc->dbp, DB_AM_NOT_DURABLE)) {
		__db_errx(env, DB_STR("0107",
		    "Dbc_logging: Master non-txn update"));
		goto err;
	}
#endif

	if (0) {
err:		__db_errx(env, DB_STR_A("0108", "Rep: flags 0x%lx msg_th %lu",
		    "%lx %lu"), (u_long)rep->flags, (u_long)rep->msg_th);
		__db_errx(env, DB_STR_A("0109", "Rep: handle %lu, opcnt %lu",
		    "%lu %lu"), (u_long)rep->handle_cnt, (u_long)rep->op_cnt);
		__os_abort(env);
		/* NOTREACHED */
	}
	}
#endif
	return (ret);
}
#endif

/*
 * __db_check_lsn --
 *	Display the log sequence error message.
 *
 * PUBLIC: int __db_check_lsn __P((ENV *, DB_LSN *, DB_LSN *));
 */
int
__db_check_lsn(env, lsn, prev)
	ENV *env;
	DB_LSN *lsn, *prev;
{
	int ret;

	ret = USR_ERR(env, EINVAL);
	__db_errx(env, DB_STR_A("0110",
	    "Log sequence error: page LSN %lu %lu; previous LSN %lu %lu",
	    "%lu %lu %lu %lu"), (u_long)(lsn)->file,
	    (u_long)(lsn)->offset, (u_long)(prev)->file,
	    (u_long)(prev)->offset);
	return (ret);
}

/*
 * __db_rdonly --
 *	Common readonly message.
 * PUBLIC: int __db_rdonly __P((const ENV *, const char *));
 */
int
__db_rdonly(env, name)
	const ENV *env;
	const char *name;
{
	int ret;

	ret = USR_ERR(env, EACCES);
	__db_errx(env, DB_STR_A("0111",
	    "%s: attempt to modify a read-only database", "%s"), name);
	return (ret);
}

/*
 * __db_space_err --
 *	Common out of space message.
 * PUBLIC: int __db_space_err __P((const DB *));
 */
int
__db_space_err(dbp)
	const DB *dbp;
{
	int ret;

	ret = USR_ERR(dbp->env, ENOSPC);
	__db_errx(dbp->env, DB_STR_A("0112",
	    "%s: file limited to %lu pages", "%s %lu"),
	    dbp->fname, (u_long)dbp->mpf->mfp->maxpgno);
	return (ret);
}

/*
 * __db_failed --
 *	Common failed thread message, e.g., after it is seen to have crashed.
 *
  PUBLIC: int __db_failed __P((const ENV *,
 * PUBLIC:      const char *, pid_t, db_threadid_t));
 */
int
__db_failed(env, msg, pid, tid)
	const ENV *env;
	const char *msg;
	pid_t pid;
	db_threadid_t tid;
{
	DB_ENV *dbenv;
	int ret;
	char tidstr[DB_THREADID_STRLEN], failmsg[DB_FAILURE_SYMPTOM_SIZE];

	dbenv = env->dbenv;
	(void)dbenv->thread_id_string(dbenv, pid, tid, tidstr);
	ret = USR_ERR(env, DB_RUNRECOVERY);
	snprintf(failmsg, sizeof(failmsg), DB_STR_A("0113",
	    "Thread/process %s failed: %s", "%s %s"), tidstr, msg);
	(void)__env_failure_remember(env, failmsg);
	__db_errx(env, "%s", failmsg);
	return (ret);
}

/*
 * __env_failure_remember --
 *	If this failure of a process in the environment is about to set panic
 *	for the first time, record that a crashed thread was thw culprit.
 *	Do nothing if panic has already been set. There are no mutexes here;
 *	in order to avoid hanging on any crashed threads.
 *
 * PUBLIC: int __env_failure_remember __P((const ENV *, const char *));
 */
int
__env_failure_remember(env, reason)
	const ENV *env;
	const char *reason;
{
	REGENV *renv;

	renv = env->reginfo->primary;
	if (renv == NULL || renv->panic || renv->failure_panic)
		return (0);
	renv->failure_panic = 1;
	if (renv->failure_symptom[0] == '\0') {
		(void)strncpy(renv->failure_symptom,
		    reason, sizeof(renv->failure_symptom));
		renv->failure_symptom[sizeof(renv->failure_symptom) - 1] = '\0';
	}
	return (0);
}

#if defined(HAVE_ERROR_HISTORY)
/*
 * __db_deferred_free --
 *	Pthread_exit() calls this to release DB_GLOBAL(msgs_key)'s
 *	thread-local storage.
 */
static void
__db_deferred_free(void *p)
{
	DB_MSGBUF *mb;

	if ((mb = p) != NULL) {
		(void)pthread_setspecific(DB_GLOBAL(msgs_key), NULL);
		if (mb->buf != NULL)
			__os_free(NULL, mb->buf);
		free(mb);
	}
}

/*
 * __db_thread_once_func --
 *	The pthread_once() functions to initialize thread local storage.
 */
static void
__db_thread_once_func()
{
	(void)pthread_key_create(&DB_GLOBAL(msgs_key), __db_deferred_free);
}

/*
 * __db_thread_init --
 *	Initialization hook to be called at least once per process, before
 *	deferring any messages.
 *
 * PUBLIC: #ifdef HAVE_ERROR_HISTORY
 * PUBLIC: void __db_thread_init __P((void));
 * PUBLIC: #endif
 */
void
__db_thread_init()
{
	/*
	 * Assign the thread-local storage identifier. Tell thread exit to clean
	 * up withl __db_deferred_free().
	 */
	(void)pthread_once(&DB_GLOBAL(thread_once), __db_thread_once_func);
}

/*
 * __db_diags --
 *
 *	Save the context which triggers the "first notice" of an error code;
 *	i.e., its creation. It doesn't touch anything when err == 0.
 *
 * PUBLIC: #ifdef HAVE_ERROR_HISTORY
 * PUBLIC: int __db_diags __P((const ENV *, int));
 * PUBLIC: #endif
 */
 int
__db_diags(env, err)
	const ENV *env;
	int err;
{
	DB_MSGBUF *mb;

	if (err != 0 && (mb = __db_deferred_get()) != NULL)
		(void)__db_remember_context(env, mb, err);
	return (err);
}

/*
 * __db_deferred_get --
 *	Get this thread's deferred DB_MSGBUF, possibly allocating it.
 *
 * PUBLIC: #ifdef HAVE_ERROR_HISTORY
 * PUBLIC: DB_MSGBUF *__db_deferred_get __P((void));
 * PUBLIC: #endif
 */
DB_MSGBUF *
__db_deferred_get()
{
	DB_MSGBUF *mb;

	if ((mb = pthread_getspecific(DB_GLOBAL(msgs_key))) == NULL) {
		if ((mb = calloc(1, sizeof(*mb))) != NULL)
			if (pthread_setspecific(DB_GLOBAL(msgs_key), mb) != 0) {
				/* Nothing else is safe do on an error. */
				free(mb);
				mb = NULL;
			}
	}
	return (mb);
}

/*
 * __db_deferred_discard --
 *	Discard any saved-up deferred messages, at e.g. the end of the command.
 *
 * PUBLIC: #ifdef HAVE_ERROR_HISTORY
 * PUBLIC: void __db_deferred_discard __P((void));
 * PUBLIC: #endif
 */
void
__db_deferred_discard()
{
	DB_MSGBUF *mb;

	if ((mb = pthread_getspecific(DB_GLOBAL(msgs_key))) != NULL)
		mb->cur = mb->buf;
}

/*
 * __db_remember_context
 *	Save the context which triggers the "first notice" of an error code;
 *	i.e., its creation. Include the time, thread, recent portion of the
 *	stack, and the error number. Add replication info too?
 *
 *	Return the error number passed in, or 0?
 *
 * PUBLIC: #ifdef HAVE_ERROR_HISTORY
 * PUBLIC: int __db_remember_context __P((const ENV *, DB_MSGBUF *, int));
 * PUBLIC: #endif
 */
 int
 __db_remember_context(env, mb, err)
	const ENV *env;
	DB_MSGBUF *mb;
	int err;
{
	DB_ENV *dbenv;
	LOG *lp;
	db_timespec now;
	pid_t pid;
	db_threadid_t tid;
	char threadid[DB_THREADID_STRLEN], timestr[CTIME_BUFLEN];

	/* Limit the amount of context messges which are remembered. */
	if (mb->len >= DB_ERROR_HISTORY_SIZE)
		return (0);

	lp = NULL;
	if (env == NULL) {
		dbenv = NULL;
		threadid[0] = '\0';
	} else {
		dbenv = env->dbenv;
		dbenv->thread_id(dbenv, &pid, &tid);
		(void)dbenv->thread_id_string(dbenv, pid, tid, threadid);
		if (LOGGING_ON(env) && !IS_RECOVERING(env))
			lp = env->lg_handle->reginfo.primary;
	}

	__os_gettime(env, &now, 0);
	(void)__db_ctimespec(&now, timestr);
	__db_msgadd(env, mb, "\n[%s][%s] %s",
	    timestr, threadid, db_strerror(err));
	if (lp != NULL)
		__db_msgadd(env, mb, " lsn [%lu][%lu]",
		    (u_long)lp->lsn.file, (u_long)lp->lsn.offset);

#if defined(HAVE_BACKTRACE) && defined(HAVE_BACKTRACE_SYMBOLS)
	/*
	 * Add many frames of stack trace to the record, skipping the first two
	 * frames: __os_stack_msgadd() and __db_remember_context().
	 */
	__db_msgadd(env, mb, " from\n");
	__os_stack_msgadd(env, mb, 15, 2, NULL);
#endif

	return (0);
}
#endif

/*
 * __db_ctimespec --
 *	Format a timespec in microseconds, similar to a terse __os_ctime(),
 *	storing the results into a CTIME_BUFLEN sized buffer.
 *	The result format depends on the availability of localtime, etc
 *		MM/DD HH:MM:SS.uuuuuu	if strftime is available, or
 *		Jan DD HH:MM:SS.uuuuuu	if only __os_ctime() is available.
 *	Both are small enough to use __os_ctime() sized buffer, e.g. 26.
 *	The other fields (year, day-of-week, ...) are intentionally removed.
 *
 * PUBLIC: char * __db_ctimespec __P((const db_timespec *, char *));
 */
char *
__db_ctimespec(timespec, buf)
	const db_timespec *timespec;
	char *buf;
{
	char *d, date[CTIME_BUFLEN];
#ifdef HAVE_STRFTIME
	struct tm *tm_p;
#ifdef HAVE_LOCALTIME_R
	struct tm tm;
#endif
#endif

	/* Print the time readably if possible; else print seconds. */
#ifdef HAVE_STRFTIME
#ifdef HAVE_LOCALTIME_R
	tm_p = localtime_r(&timespec->tv_sec, &tm);
#else
	tm_p = localtime(&timespec->tv_sec);
#endif
	if (tm_p != NULL) {
		d = date;
		(void)strftime(d, sizeof(date), DB_GLOBAL(time_format), tm_p);
	}
	else
#endif
	{
		/* Trim off the leading day-of-week; then the trailing year. */
		d = __os_ctime(&timespec->tv_sec, date) + 4;
		d[sizeof("Jan 01 00:00:00")] = '\0';
	}
	(void)snprintf(buf, CTIME_BUFLEN,
	    "%s.%06lu", d, (u_long)(timespec->tv_nsec / NS_PER_US));
	buf[CTIME_BUFLEN - 1] = '\0';	/* In case of buggy snprintf. */
	return (buf);
}

/*
 * __db_fmt_quote --
 *	Copy a printf format string, quoting (doubling) each '%' along the way.
 *	Use this when inserting a user-defined string into a *printf format.
 *	If the src parameter is NULL, then quote in-place, shifting the
 *	rest of the string down by one character for each quote.
 *
 * PUBLIC: char *__db_fmt_quote __P((char *, size_t, const char *));
 */
char *
__db_fmt_quote(dest, destsize, src)
	char *dest;
	size_t destsize;
	const char *src;
{
	char *d, *end;
	const char *s;
	size_t len;

	/* Stop early enough so that dest always has room for a '\0'. */
	end = dest + destsize - 1;
	if (src == NULL) {
		d = dest;
		while ((d = strchr(d, '%')) != NULL && d[1] != '\0') {
			/*
			 * Shift the rest of the string by one byte to make
			 * space for another '%'. By starting at d and adding 1
			 * to the length, we double the '%' while copying the
			 * string and its terminating '\0'.
			 */
			len = strlen(d) + 1;
			memmove(d + 1, d, len);
			/*
			 * We're done if the string now is larger than the
			 * reserved size; else advance over both '%'s.
			 */
			if (d + len >= end) {
				DB_ASSERT(NULL, d + len == end);
				*end = '\0';
				break;
			}
			d += 2;
		}
	} else {
		for (s = src, d = dest; *s != '\0' && d < end; d++, s++)
			if ((*d = *s) == '%') {
				/* Discard a % at the end of the string. */
				if (s[1] == '\0')
					break;
				*++d = '%';
			}
		*d = '\0';
	}
	return (dest);
}
