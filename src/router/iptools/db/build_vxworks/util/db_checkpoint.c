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

int	 db_checkpoint_main __P((int, char *[]));
void	 db_checkpoint_usage __P((void));

const char *progname;

int
db_checkpoint(args)
	char *args;
{
	int argc;
	char **argv;

	__db_util_arg("db_checkpoint", args, &argc, &argv);
	return (db_checkpoint_main(argc, argv) ? EXIT_FAILURE : EXIT_SUCCESS);
}

#include <stdio.h>
#define	ERROR_RETURN	ERROR

int
db_checkpoint_main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;
	extern int optind, __db_getopt_reset;
	DB_ENV	*dbenv;
	time_t now;
	long argval;
	u_int32_t flags, kbytes, minutes, seconds;
	int ch, exitval, once, ret, verbose;
	char *home, *logfile, *msgpfx, *passwd, time_buf[CTIME_BUFLEN];

	progname = __db_util_arg_progname(argv[0]);

	if ((ret = __db_util_version_check(progname)) != 0)
		return (ret);

	/*
	 * !!!
	 * Don't allow a fully unsigned 32-bit number, some compilers get
	 * upset and require it to be specified in hexadecimal and so on.
	 */
#define	MAX_UINT32_T	2147483647

	dbenv = NULL;
	kbytes = minutes = 0;
	once = verbose = 0;
	flags = 0;
	exitval = EXIT_SUCCESS;
	home = logfile = msgpfx = passwd = NULL;
	__db_getopt_reset = 1;
	while ((ch = getopt(argc, argv, "1h:k:L:m:P:p:Vv")) != EOF)
		switch (ch) {
		case '1':
			once = 1;
			flags = DB_FORCE;
			break;
		case 'h':
			home = optarg;
			break;
		case 'k':
			if (__db_getlong(NULL, progname,
			    optarg, 1, (long)MAX_UINT32_T, &argval))
				goto err;
			kbytes = (u_int32_t)argval;
			break;
		case 'L':
			logfile = optarg;
			break;
		case 'm':
			msgpfx = optarg;
			break;
		case 'P':
			if (__db_util_arg_password(progname,
 			    optarg, &passwd) != 0)
  				goto err;
			break;
		case 'p':
			if (__db_getlong(NULL, progname,
			    optarg, 1, (long)MAX_UINT32_T, &argval))
				goto err;
			minutes = (u_int32_t)argval;
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

	if (once == 0 && kbytes == 0 && minutes == 0) {
		(void)fprintf(stderr, DB_STR_A("5122",
		    "%s: at least one of -1, -k and -p must be specified\n",
		    "%s\n"), progname);
		goto usage_err;
	}

	/* Handle possible interruptions. */
	__db_util_siginit();

	/* Log our process ID. */
	if (logfile != NULL && __db_util_logset(progname, logfile))
		goto err;

	if (__db_util_env_create(&dbenv, progname, passwd, msgpfx) != 0)
		goto err;

	/*
	 * Turn on DB_THREAD in case a repmgr application wants to do
	 * checkpointing using this utility: repmgr requires DB_THREAD
	 * for all env handles.
	 */
#ifdef HAVE_REPLICATION_THREADS
#define	ENV_FLAGS DB_THREAD
#else
#define	ENV_FLAGS 0
#endif
	if (__db_util_env_open(dbenv, home, ENV_FLAGS,
	    once, DB_INIT_TXN, 0, NULL) != 0)
		goto err;

	/*
	 * If we have only a time delay, then we'll sleep the right amount
	 * to wake up when a checkpoint is necessary.  If we have a "kbytes"
	 * field set, then we'll check every 30 seconds.
	 */
	seconds = kbytes != 0 ? 30 : minutes * 60;
	while (!__db_util_interrupted()) {
		if (verbose) {
			(void)time(&now);
			dbenv->msg(dbenv, DB_STR_A("5123",
			    "checkpoint begin: %s", "%s"),
			    __os_ctime(&now, time_buf));
		}

		if ((ret = dbenv->txn_checkpoint(dbenv,
		    kbytes, minutes, flags)) != 0) {
			dbenv->err(dbenv, ret, "txn_checkpoint");
			goto err;
		}

		if (verbose) {
			(void)time(&now);
			dbenv->msg(dbenv, DB_STR_A("5124",
			    "checkpoint complete: %s", "%s"),
			    __os_ctime(&now, time_buf));
		}

		if (once)
			break;

		__os_yield(dbenv->env, seconds, 0);
	}

	if (0) {
usage_err:	db_checkpoint_usage();
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
db_checkpoint_usage()
{
	(void)fprintf(stderr, "usage: %s [-1Vv]\n\t%s %s\n", progname,
	    "[-h home] [-k kbytes] [-L file] [-m msg_pfx]",
	    "[-P password] [-p min]");
}
