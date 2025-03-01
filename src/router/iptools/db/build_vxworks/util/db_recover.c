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

void db_recover_feedback __P((DB_ENV *, int, int));
int  db_recover_main __P((int, char *[]));
int  db_recover_read_timestamp __P((char *, time_t *));
void db_recover_usage __P((void));

const char *progname;
int newline_needed;

int
db_recover(args)
	char *args;
{
	int argc;
	char **argv;

	__db_util_arg("db_recover", args, &argc, &argv);
	return (db_recover_main(argc, argv) ? EXIT_FAILURE : EXIT_SUCCESS);
}

#include <stdio.h>
#define	ERROR_RETURN	ERROR

int
db_recover_main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;
	extern int optind, __db_getopt_reset;
	DB_ENV	*dbenv;
	time_t timestamp;
	u_int32_t flags;
	int ch, exitval, fatal_recover, ret, retain_env, set_feedback, verbose;
	char *blob_dir, *home, *passwd, *region_dir;

	progname = __db_util_arg_progname(argv[0]);

	if ((ret = __db_util_version_check(progname)) != 0)
		return (ret);

	dbenv = NULL;
	blob_dir = home = passwd = region_dir = NULL;
	timestamp = 0;
	fatal_recover = retain_env = set_feedback = verbose = 0;
	exitval = EXIT_SUCCESS;
	__db_getopt_reset = 1;
	while ((ch = getopt(argc, argv, "b:cefh:P:r:t:Vv")) != EOF)
		switch (ch) {
		case 'b':
			blob_dir = optarg;
			break;
		case 'c':
			fatal_recover = 1;
			break;
		case 'e':
			retain_env = 1;
			break;
		case 'f':
			set_feedback = 1;
			break;
		case 'h':
			home = optarg;
			break;
		case 'P':
			if (__db_util_arg_password(progname,
 			    optarg, &passwd) != 0)
  				goto err;
			break;
		case 'r':
			region_dir = optarg;
			break;
		case 't':
			if ((ret = db_recover_read_timestamp(optarg, &timestamp)) != 0)
				goto err;
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

	if (__db_util_env_create(&dbenv, progname, passwd, NULL) != 0)
		goto err;

	if (set_feedback)
		(void)dbenv->set_feedback(dbenv, db_recover_feedback);
	if (verbose)
		(void)dbenv->set_verbose(dbenv, DB_VERB_RECOVERY, 1);
	if (timestamp &&
	    (ret = dbenv->set_tx_timestamp(dbenv, &timestamp)) != 0) {
		dbenv->err(dbenv, ret, "DB_ENV->set_timestamp");
		goto err;
	}

	if (blob_dir != NULL &&
	    (ret = dbenv->set_blob_dir(dbenv, blob_dir)) != 0) {
		dbenv->err(dbenv, ret, "set_blob_dir");
		goto err;
	}

	if (region_dir != NULL &&
	    (ret = dbenv->set_region_dir(dbenv, region_dir)) != 0) {
		dbenv->err(dbenv, ret, "set_region_dir");
		goto err;
	}

	/*
	 * Initialize the environment -- we don't actually do anything
	 * else, that all that's needed to run recovery.
	 *
	 * Note that unless the caller specified the -e option, we use a
	 * private environment, as we're about to create a region, and we
	 * don't want to to leave it around.  If we leave the region around,
	 * the application that should create it will simply join it instead,
	 * and will then be running with incorrectly sized (and probably
	 * terribly small) caches.  Applications that use -e should almost
	 * certainly use DB_CONFIG files in the directory.
	 */
	flags = 0;
	LF_SET(DB_CREATE | DB_INIT_LOG |
	    DB_INIT_MPOOL | DB_INIT_TXN | DB_USE_ENVIRON);
	LF_SET(fatal_recover ? DB_RECOVER_FATAL : DB_RECOVER);
	LF_SET(retain_env ? DB_INIT_LOCK : DB_PRIVATE);
	if ((ret = dbenv->open(dbenv, home, flags, 0)) != 0) {
		dbenv->err(dbenv, ret, "DB_ENV->open");
		goto err;
	}

	if (0) {
usage_err:	db_recover_usage();
err:		exitval = EXIT_FAILURE;
	}
done:
	/* Flush to the next line of the output device. */
	if (newline_needed)
		printf("\n");

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

/*
 * db_recover_feedback --
 *	Provide feedback on recovery progress.
 */
void
db_recover_feedback(dbenv, opcode, percent)
	DB_ENV *dbenv;
	int opcode;
	int percent;
{
	COMPQUIET(dbenv, NULL);

	if (opcode == DB_RECOVER) {
		printf(DB_STR_A("5022", "\rrecovery %d%% complete", "%d"),
		    percent);
		(void)fflush(stdout);
		newline_needed = 1;
	}
}

#define	ATOI2(ar)	((ar)[0] - '0') * 10 + ((ar)[1] - '0'); (ar) += 2;

/*
 * read_timestamp --
 *	Convert a time argument to Epoch seconds.
 *
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
int
db_recover_read_timestamp(arg, timep)
	char *arg;
	time_t *timep;
{
	struct tm *t;
	time_t now;
	int yearset;
	char *p;
					/* Start with the current time. */
	(void)time(&now);
	if ((t = localtime(&now)) == NULL) {
		fprintf(stderr, DB_STR_A("5023", "%s: localtime: %s\n",
		    "%s %s\n"), progname, strerror(errno));
		return (EXIT_FAILURE);
	}
					/* [[CC]YY]MMDDhhmm[.SS] */
	if ((p = strchr(arg, '.')) == NULL)
		t->tm_sec = 0;		/* Seconds defaults to 0. */
	else {
		if (strlen(p + 1) != 2)
			goto terr;
		*p++ = '\0';
		t->tm_sec = ATOI2(p);
	}

	yearset = 0;
	switch (strlen(arg)) {
	case 12:			/* CCYYMMDDhhmm */
		t->tm_year = ATOI2(arg);
		t->tm_year *= 100;
		yearset = 1;
		/* FALLTHROUGH */
	case 10:			/* YYMMDDhhmm */
		if (yearset) {
			yearset = ATOI2(arg);
			t->tm_year += yearset;
		} else {
			yearset = ATOI2(arg);
			if (yearset < 69)
				t->tm_year = yearset + 2000;
			else
				t->tm_year = yearset + 1900;
		}
		t->tm_year -= 1900;	/* Convert to UNIX time. */
		/* FALLTHROUGH */
	case 8:				/* MMDDhhmm */
		t->tm_mon = ATOI2(arg);
		--t->tm_mon;		/* Convert from 01-12 to 00-11 */
		t->tm_mday = ATOI2(arg);
		t->tm_hour = ATOI2(arg);
		t->tm_min = ATOI2(arg);
		break;
	default:
		goto terr;
	}

	t->tm_isdst = -1;		/* Figure out DST. */

	*timep = mktime(t);
	if (*timep == -1) {
terr:		fprintf(stderr, DB_STR_A("5024",
    "%s: out of range or illegal time specification: [[CC]YY]MMDDhhmm[.SS]",
		    "%s"), progname);
		return (EXIT_FAILURE);
	}
	return (0);
}

void
db_recover_usage()
{
	(void)fprintf(stderr, "usage: %s %s\n", progname,
"[-cefVv] [-h home] [-b blob_dir] [-P password]  [-r region_dir] [-t [[CC]YY]MMDDhhmm[.SS]]");
}
