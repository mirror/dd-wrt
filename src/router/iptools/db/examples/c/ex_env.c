/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * ex_env -- A basic Environment example with transaction support:
 *	Create a transactional environment and a B-Tree database.
 *
 *   This example shows how to set up a transactional DB environment. A
 *   Berkeley DB environment is an encapsulation of one or more databases,
 *   log files and region files. Region files contain information about
 *   resources shared among all databases in the environment, such as locks
 *   and memory pool caches.
 *
 *   The program first creates an environment handle, configures its cache size
 *   and the directory used to store data files, and finally opens it for
 *   transactional programs.
 *
 *   After the environment is opened, a B-Tree database is opened within the
 *   environment. Once this is done, the database handle and the environment
 *   handle are closed.
 *
 *   As a cleanup step, at the end of the program, the environment is removed. 
 *
 * Source File: ex_env.c
 * Environment Home Directory: TESTDIR
 * Data Directory: TESTDIR/data
 * Database: exenv_db1.db
 *
 * Prerequisite: the home directory and the data directory must exist before
 * running the program.
 *
 * Options:
 *	-l		use the DB_LOCKDOWN flag when opening the environment. Lock
 *			shared BDB environment files and memory-mapped databases into
 *			memory.
 *	-h [dir]	specify the environment home directory
 *	-d [dir]	specify the data diretory
 *
 * $Id$
 */

#include <sys/types.h>

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
extern int getopt(int, char * const *, const char *);
#else
#include <unistd.h>
#endif

#include <db.h>


int db_setup __P((const char *, const char *, FILE *, const char *));
int db_teardown __P((const char *, const char *, FILE *, const char *));
static int usage __P((void));

const char *progname = "ex_env";		/* Program name. */

int EnvFlags = 
    DB_CREATE |		/* Create environment files if necessary. */
    DB_INIT_LOCK |	/* Initializes the locking subsystem. */
    DB_INIT_LOG |	/* Initializes the logging subsystem. */
    DB_INIT_MPOOL |	/* Initializes shared memory buffer pool subsystem. */
    DB_INIT_TXN;	/* Initializes the transaction subsystem. */

/*
 * An example of a program creating/configuring a Berkeley DB environment.
 */
int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;	/* Set by getopt(): the argv being processed. */
	extern int optind;	/* Set by getopt(): the # of argv's processed. */
	const char *data_dir, *home;
	int ch;

	/*
	 * All of the shared environment files live in home, but data files
	 * live in the data_dir under home. Shared environment files contain
	 * shared supporting infrastructures such as locking , logging and cache.
	 * Actual database files are stored in the data directory.
	 */
	home = "TESTDIR";
	data_dir = "data";

	/* Parse the command line arguments */
	while ((ch = getopt(argc, argv, "h:d:l")) != EOF)
		switch (ch) {
		case 'h':	/* Set custom environment home directory. */
			home = optarg;
			break;
		case 'd':	/* Set custom data directory. */
			data_dir = optarg;
			break;
		case 'l':	/* Lock environment files into memory. */
			EnvFlags |= DB_LOCKDOWN;
			break;
		case '?':	/* Display help messages and exit. */
		default:
			return (usage());
		}
	argc -= optind;
	argv += optind;

	if (argc != 0)
		return (usage());

	printf("Setup env\n");
	if (db_setup(home, data_dir, stderr, progname) != 0)
		return (EXIT_FAILURE);

	printf("Teardown env\n");
	if (db_teardown(home, data_dir, stderr, progname) != 0)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*
 * db_setup --
 * 	Set up a transactional environment by creating a new
 * environment handle, configuring its error report facility, cache size and
 * data file directory and finally opening the environment with appropriate
 * flags. It also shows how to open a database in the environment.
 *
 * Parameters:
 *	home		the environment home directory
 *	data_dir	the directory used to locate database files
 *	errfp		the pointer to a FILE where error messages are written to
 *	progname	the name of this program
 */
int
db_setup(home, data_dir, errfp, progname)
	const char *home, *data_dir, *progname;
	FILE *errfp;
{
	DB_ENV *dbenv;	/* The environment handle. */
	DB *dbp;	/* The database handle. */
	int ret;	/* The return value. */

	/* Create an environment object and configure it. */
	if ((ret = db_env_create(&dbenv, 0)) != 0) {
		fprintf(errfp, "%s: %s\n", progname, db_strerror(ret));
		return (1);
	}

	/*
	 * Prefix any error messages with the name of this program and a ':'.
	 * Setting the errfile to stderr is not necessary, since that is the
	 * default; it is provided here as a placeholder showing where one
	 * could direct error messages to an application-specific log file.
	 */
	dbenv->set_errfile(dbenv, errfp);
	dbenv->set_errpfx(dbenv, progname);

	/*
	 * Configure the database to use:
	 *	A database cache size of 0 GB and 64KB, in a single region.
	 *
	 * This cache size is large enough for this trivial program. On a real
	 * production system, the cache size should be set large enough to
	 * hold the working set.
	 */
	if ((ret = dbenv->set_cachesize(dbenv, 0, 64 * 1024, 0)) != 0) {
		dbenv->err(dbenv, ret, "set_cachesize");
		dbenv->close(dbenv, 0);
		return (1);
	}

	/* 
	 * Configure the environment to store and search database files from
	 * the given directory.
	 */
	(void)dbenv->set_data_dir(dbenv, data_dir);

	/*
	 * Now open the environment. The DB_INIT_TXN flag and other flags
	 * required by it specify that the environment will have transactional
	 * support.
	 */
	if ((ret = dbenv->open(dbenv, home, EnvFlags, 0644)) != 0) {
		dbenv->err(dbenv, ret, "environment open: %s", home);
		if (ret == ENOENT) {
		    printf("Please check whether home dir \"%s\" existed.\n",
		        home);
		}
		dbenv->close(dbenv, 0);
		return (1);
	}

	/*
	 * Open a database in the environment to verify the data file directory
	 * has been set correctly.
	 *
	 * Create a database object using the above environment. 
	 */
	if ((ret = db_create(&dbp, dbenv, 0)) != 0) {
		fprintf(errfp, "%s: %s\n", progname, db_strerror(ret));
		return (1);
	}

	/* Open the database with DB_BTREE access method. */
	if ((ret = dbp->open(dbp, NULL, "exenv_db1.db", NULL,
	    DB_BTREE, DB_CREATE, 0644)) != 0) {
		fprintf(stderr, "database open: %s\n", db_strerror(ret));
		if (ret == ENOENT) {
			printf("Please check whether data dir \"%s\" "
			    "exists under \"%s\".\n", data_dir, home);
		}
		return (1);
	}

	/* Close the database object. */
	if ((ret = dbp->close(dbp, 0)) != 0) {
		fprintf(stderr, "database close: %s\n", db_strerror(ret));
		return (1);
	}

	/* Close the environment object. */
	if ((ret = dbenv->close(dbenv, 0)) != 0) {
		fprintf(stderr, "DB_ENV->close: %s\n", db_strerror(ret));
		return (1);
	}
	return (0);
}

/*
 * db_teardown --
 * 	Remove the environment and the database created in the above function.
 *
 * Parameters:
 *	home		the environment home directory
 *	data_dir	the directory used to locate database files
 *	errfp		the pointer to a FILE where error messages are written to
 *	progname	the name of this program
 */
int
db_teardown(home, data_dir, errfp, progname)
	const char *home, *data_dir, *progname;
	FILE *errfp;
{
	DB_ENV *dbenv;
	int ret;

	/* Create an environment object. */
	if ((ret = db_env_create(&dbenv, 0)) != 0) {
		fprintf(errfp, "%s: %s\n", progname, db_strerror(ret));
		return (1);
	}

	/* Configure the error reporting facility. */
	dbenv->set_errfile(dbenv, errfp);
	dbenv->set_errpfx(dbenv, progname);

	/* Set the data file search directory. */
	(void)dbenv->set_data_dir(dbenv, data_dir);

	/* 
	 * Remove the environment. All files in the home directory and data
	 * directory are removed.
	 */
	if ((ret = dbenv->remove(dbenv, home, 0)) != 0) {
		fprintf(stderr, "DB_ENV->remove: %s\n", db_strerror(ret));
		return (1);
	}
	return (0);
}

/*
 * usage --
 *	Describe this program's command line options, then exit.
 */

static int
usage()
{
	(void)fprintf(stderr,
	    "usage: %s [-l] [-h home] [-d data_dir]\n", progname);
	return (EXIT_FAILURE);
}
