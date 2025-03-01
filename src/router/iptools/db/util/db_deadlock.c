/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#include "db_config.h"

#include "db_int.h"

#ifndef lint
static const char copyright[] =
    "Copyright (c) 1996, 2017 Oracle and/or its affiliates.  All rights reserved.\n";
#endif

int main __P((int, char *[]));
void usage __P((void));

const char *progname;

int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;
	extern int optind;
	DB_ENV *dbenv;
	u_int32_t atype;
	time_t now;
	u_long secs, usecs;
	int rejected, ch, exitval, ret, verbose;
	char *home, *logfile, *passwd, *str, time_buf[CTIME_BUFLEN];

	progname = __db_util_arg_progname(argv[0]);

	if ((ret = __db_util_version_check(progname)) != 0)
		return (ret);

	dbenv = NULL;
	atype = DB_LOCK_DEFAULT;
	home = logfile = passwd = NULL;
	secs = usecs = 0;
	verbose = 0;
	exitval = EXIT_SUCCESS;
	while ((ch = getopt(argc, argv, "a:h:L:P:t:Vv")) != EOF)
		switch (ch) {
		case 'a':
			switch (optarg[0]) {
			case 'e':
				atype = DB_LOCK_EXPIRE;
				break;
			case 'm':
				atype = DB_LOCK_MAXLOCKS;
				break;
			case 'n':
				atype = DB_LOCK_MINLOCKS;
				break;
			case 'o':
				atype = DB_LOCK_OLDEST;
				break;
			case 'W':
				atype = DB_LOCK_MAXWRITE;
				break;
			case 'w':
				atype = DB_LOCK_MINWRITE;
				break;
			case 'y':
				atype = DB_LOCK_YOUNGEST;
				break;
			default:
				goto usage_err;
				/* NOTREACHED */
			}
			if (optarg[1] != '\0')
				goto usage_err;
			break;
		case 'h':
			home = optarg;
			break;
		case 'L':
			logfile = optarg;
			break;
		case 'P':
			if (__db_util_arg_password(progname, 
 			    optarg, &passwd) != 0)
  				goto err;
  			break;
		case 't':
			if ((str = strchr(optarg, '.')) != NULL) {
				*str++ = '\0';
				if (*str != '\0' && __db_getulong(
				    NULL, progname, str, 0, LONG_MAX, &usecs))
					goto err;
			}
			if (*optarg != '\0' && __db_getulong(
			    NULL, progname, optarg, 0, LONG_MAX, &secs))
				goto err;
			if (secs == 0 && usecs == 0)
				goto usage_err;
			break;
		case 'V':
			printf("%s\n", db_version(NULL, NULL, NULL));
			goto done;
		case 'v':
			verbose = 1;
			break;
		case '?':
		default:
			goto usage_err;
		}
	argc -= optind;
	argv += optind;

	if (argc != 0)
		goto usage_err;

	/* Handle possible interruptions. */
	__db_util_siginit();

	/* Log our process ID. */
	if (logfile != NULL && __db_util_logset(progname, logfile))
		goto err;

	if (__db_util_env_create(&dbenv, progname, passwd, NULL) != 0)
		goto err;

	if (verbose) {
		(void)dbenv->set_verbose(dbenv, DB_VERB_DEADLOCK, 1);
		(void)dbenv->set_verbose(dbenv, DB_VERB_WAITSFOR, 1);
	}

	if (__db_util_env_open(dbenv, home, 0, 0, 0, 0, NULL) != 0)
		goto err;

	while (!__db_util_interrupted()) {
		if (verbose) {
			(void)time(&now);
			dbenv->msg(dbenv, DB_STR_A("5102",
			    "running at %.24s", "%.24s"),
			     __os_ctime(&now, time_buf));
		}

		if ((ret =
		    dbenv->lock_detect(dbenv, 0, atype, &rejected)) != 0) {
			dbenv->err(dbenv, ret, "DB_ENV->lock_detect");
			goto err;
		}
		if (verbose)
			dbenv->msg(dbenv, DB_STR_A("5103",
			    "rejected %d locks", "%d"), rejected);

		/* Make a pass every "secs" secs and "usecs" usecs. */
		if (secs == 0 && usecs == 0)
			break;
		__os_yield(dbenv->env, secs, usecs);
	}

	if (0) {
usage_err:	usage();
err:		exitval = EXIT_FAILURE;
	}
done:
	/* Clean up the logfile. */
	if (logfile != NULL)
		(void)remove(logfile);

	/* Clean up the environment. */
	if (dbenv != NULL && (ret = dbenv->close(dbenv, 0)) != 0) {
		exitval = EXIT_FAILURE;
		fprintf(stderr,
		    "%s: dbenv->close: %s\n", progname, db_strerror(ret));
	}

	if (passwd != NULL)
		free(passwd);

	/* Resend any caught signal. */
	__db_util_sigresend();

	return (exitval);
}

void
usage()
{
	(void)fprintf(stderr,
	    "usage: %s [-Vv] [-a e | m | n | o | W | w | y]\n\t%s\n", progname,
	    "[-h home] [-L file] [-P password] [-t sec.usec]");
}
