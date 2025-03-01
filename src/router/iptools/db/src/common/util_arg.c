/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2001, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#include "db_config.h"

#include "db_int.h"

#if DB_VERSION_MAJOR < 4 || DB_VERSION_MAJOR == 4 && DB_VERSION_MINOR < 5
/*
 * !!!
 * We build this file in old versions of Berkeley DB when we're doing test
 * runs using the test_micro tool.   Without a prototype in place, we get
 * warnings, and there's no simple workaround.
 */
char *strsep();
#endif

/*
 * __db_util_arg --
 *	Convert a string into an argc/argv pair.
 *
 * PUBLIC: int __db_util_arg __P((char *, char *, int *, char ***));
 */
int
__db_util_arg(arg0, str, argcp, argvp)
	char *arg0, *str, ***argvp;
	int *argcp;
{
	int n, ret;
	char **ap, **argv;

#define	MAXARGS	25
	if ((ret =
	    __os_malloc(NULL, (MAXARGS + 1) * sizeof(char **), &argv)) != 0)
		return (ret);

	ap = argv;
	*ap++ = arg0;
	for (n = 1; (*ap = strsep(&str, " \t")) != NULL;)
		if (**ap != '\0') {
			++ap;
			if (++n == MAXARGS)
				break;
		}
	*ap = NULL;

	*argcp = (int)(ap - argv);
	*argvp = argv;

	return (0);
}

/*
 * __db_util_arg_progname --
 *	Return the program name.
 *
 * PUBLIC: const char *__db_util_arg_progname __P((const char *));
 */
const char *
__db_util_arg_progname(arg0)
	const char *arg0;
{
	char *progname;

	if ((progname = __db_rpath(arg0)) == NULL)
		return (arg0);
	else
		return (progname + 1);
}

/*
 * __db_util_arg_password --
 *	Handle the password argument.
 *
 * PUBLIC: int __db_util_arg_password __P((const char *, char *, char **));
 */
int
__db_util_arg_password(progname, opt_arg, passwdp)
	const char *progname;
	char *opt_arg;
	char **passwdp;
{
	if (passwdp == NULL)
		return (EINVAL);

	if (*passwdp != NULL) {
		fprintf(stderr, DB_STR("5130",
		    "Password may not be specified twice"));
		return (EINVAL);
	}

	*passwdp = strdup(opt_arg);
	memset(opt_arg, 0, strlen(opt_arg));
	if (*passwdp == NULL) {
		fprintf(stderr, DB_STR_A("5005",
		    "%s: strdup: %s\n", "%s %s\n"),
		    progname, strerror(errno));
		return (errno);
	}

	return (0);
}
