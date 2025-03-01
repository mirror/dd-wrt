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
	DB_ENV	*dbenv;
	u_int32_t flags;
	int ch, exitval, ret, verbose;
	char **file, *home, **list, *msgpfx, *passwd;

	progname = __db_util_arg_progname(argv[0]);

	if ((ret = __db_util_version_check(progname)) != 0)
		return (ret);

	dbenv = NULL;
	flags = 0;
	verbose = 0;
	exitval = EXIT_SUCCESS;
	home = msgpfx = passwd = NULL;
	file = list = NULL;
	while ((ch = getopt(argc, argv, "adh:lm:P:sVv")) != EOF)
		switch (ch) {
		case 'a':
			LF_SET(DB_ARCH_ABS);
			break;
		case 'd':
			LF_SET(DB_ARCH_REMOVE);
			break;
		case 'h':
			home = optarg;
			break;
		case 'l':
			LF_SET(DB_ARCH_LOG);
			break;
		case 'm':
			msgpfx = optarg;
			break;
		case 'P':
			if (__db_util_arg_password(progname,
 			    optarg, &passwd) != 0)
				goto err;
			break;
		case 's':
			LF_SET(DB_ARCH_DATA);
			break;
		case 'V':
			printf("%s\n", db_version(NULL, NULL, NULL));
			goto done;
		case 'v':
			/*
			 * !!!
			 * The verbose flag no longer actually does anything,
			 * but it's left rather than adding it back at some
			 * future date.
			 */
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

	if (__db_util_env_create(&dbenv, progname, passwd, msgpfx) != 0)
		goto err;

	if (__db_util_env_open(dbenv, home, 0, 1, DB_INIT_LOG, 0, NULL) != 0)
		goto err;

	/* Get the list of names. */
	if ((ret = dbenv->log_archive(dbenv, &list, flags)) != 0) {
		dbenv->err(dbenv, ret, "DB_ENV->log_archive");
		goto err;
	}

	/* Print the list of names. */
	if (list != NULL) {
		for (file = list; *file != NULL; ++file)
			printf("%s\n", *file);
		free(list);
	}

	if (0) {
usage_err:	usage();
err:		exitval = EXIT_FAILURE;
	}
done:	if (dbenv != NULL && (ret = dbenv->close(dbenv, 0)) != 0) {
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
	(void)fprintf(stderr, "usage: %s %s\n", progname,
	    "[-adlsVv] [-h home] [-m msg_pfx] [-P password]");
}
