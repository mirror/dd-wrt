/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * ex_lock---
 *  This example demonstrates how to use the lock manager for non-Berkeley DB data.
 *
 *   This example shows how to use the lock manager to handle non-Berkeley DB data.
 *   Lock mechanism is a typical implementation technology in database management
 *   systems. Locks can protect shared data by granting exclusive accesses to them.
 *
 *   The program first initializes a database environment. In the database environment,
 *   the user inputs a operation type, release or get a lock. The default operation type is to 
 *   get a lock. After the operation type is entered, if the operation type is to get a lock,
 *   the user inputs object to lock and then finally specifies the lock type. The input object 
 *   will be store in lock DBT. If the operation type is to release a lock,the user need to 
 *   input the lock id,which lies in the range between a and lockcount.
 *
 *   At the end of the program, the lock is freed and the environment is closed.
 *
 * Environment directory:
 *	TESTDIR: configured as a Transactional Data Store
 *
 * program name: ex_lock
 *
 * Options:
 *   -h     specify the environment home directory.
 *   -m     set maximum number of locks.
 *   -u     remove existing environment
 *
 * $Id$
 */

#include <sys/types.h>

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
extern int getopt(int, char * const *, const char *);
#else
#include <unistd.h>
#endif

#include <db.h>



int db_init __P((const char *, u_int32_t, int));
int main __P((int, char *[]));
int usage __P((void));

DB_ENV	 *dbenv;	/*handle of the database environment*/
const char
	*progname = "ex_lock";				/* Program name. */

int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;	/* Set by getopt(): the argv being processed. */
	extern int optind;	/* Set by getopt(): the # of argv's processed. */
	DBT lock_dbt;		/* The object to be locked. */
	DB_LOCK lock;		/* The lock object. */
	DB_LOCK *locks;		/* The array of all acquired locks. */
	db_lockmode_t lock_type;/* The lock type. */
	long held;		/* The number of locks still held. */
	size_t len;		/* The length of user inputs. */
	u_int32_t locker;	/* The locker id. */
	u_int32_t maxlocks;	/* The maxinum number of locks. */
	int ch;			/* The current command line option char. */
	int do_unlink;		/* -u option: If set remove existing environment. */
	int did_get;		/* If the lock for the current user-typed object is acquired. */
	int i;			/* -m option: The maximum number of locks supported. */
	int lockid;		/* The index to the array of all acquired locks. */
	int lockcount;		/* The total number of acquired locks. */
	int ret;		/* Return code from call into Berkeley DB. */
	const char *home;	/* The environment home directory. */

	char opbuf[16];		/* Operation type input buffer. */
	char objbuf[1024];	/* Object input buffer. */
	char lockbuf[16];	/* Lock type input buffer. */

	home = "TESTDIR";
	maxlocks = 0;
	do_unlink = 0;

	/* Parse the command line arguments */
	while ((ch = getopt(argc, argv, "h:m:u")) != EOF)
		switch (ch) {
		case 'h':		/* Set custom environment home directory. */
			home = optarg;
			break;
		case 'm':		/* Set the maximum number of locks. */
			if ((i = atoi(optarg)) <= 0)
				return (usage());
			maxlocks = (u_int32_t)i;
			break;
		case 'u':		/* Set do_unlink. */
			do_unlink = 1;
			break;
		case '?':		/* Display help messages and exit. */
		default:
			return (usage());
		}
	argc -= optind;
	argv += optind;

	if (argc != 0)
		return (usage());

	/* Initialize the database environment. */
	if ((ret = db_init(home, maxlocks, do_unlink)) != 0)
		return (ret);

	locks = 0;
	lockcount = 0;

	/* Acquire a new locker id. */
	if ((ret = dbenv->lock_id(dbenv, &locker)) != 0) {
		dbenv->err(dbenv, ret, "unable to get locker id");
		(void)dbenv->close(dbenv, 0);
		return (EXIT_FAILURE);
	}
	lockid = -1;

	/* Zero out the object to be locked. */
	memset(&lock_dbt, 0, sizeof(lock_dbt));
	for (held = 0, did_get = 0;;) {
		printf("Operation get/release [get]> ");
		fflush(stdout);

		/* Type in operation type. */
		if (fgets(opbuf, sizeof(opbuf), stdin) == NULL)
			break;
		if ((len = strlen(opbuf)) <= 1 || strcmp(opbuf, "get\n") == 0) {
			/* Acquire a lock. */
			printf("input object (text string) to lock> ");
			fflush(stdout);

			/* Type in the object to be locked. */
			if (fgets(objbuf, sizeof(objbuf), stdin) == NULL)
				break;
			if ((len = strlen(objbuf)) <= 1)
				continue;

			/* Type in the lock type. */
			do {
				printf("lock type read/write [read]> ");
				fflush(stdout);
				if (fgets(lockbuf,
				    sizeof(lockbuf), stdin) == NULL)
					break;
				len = strlen(lockbuf);
			} while (len > 1 &&
			    strcmp(lockbuf, "read\n") != 0 &&
			    strcmp(lockbuf, "write\n") != 0);

			/* Parse lock type. */
			if (len == 1 || strcmp(lockbuf, "read\n") == 0)
				lock_type = DB_LOCK_READ;
			else
				lock_type = DB_LOCK_WRITE;

			/* Set the object. */
			lock_dbt.data = objbuf;
			lock_dbt.size = (u_int32_t)strlen(objbuf);

			/* Acquire the lock for the object. */
			ret = dbenv->lock_get(dbenv, locker,
			    DB_LOCK_NOWAIT, &lock_dbt, lock_type, &lock);

			/* Add the lock to the end of the acquired lock array. */
			if (ret == 0) {
				did_get = 1;
				lockid = lockcount++;
				if (locks == NULL)
					locks =
					    (DB_LOCK *)malloc(sizeof(DB_LOCK));
				else
					locks = (DB_LOCK *)realloc(locks,
					    lockcount * sizeof(DB_LOCK));
				locks[lockid] = lock;
			}
		} else {
			/* Release a lock. */
			/* Type in the index of the lock to be released. */
			do {
				printf("input lock to release> ");
				fflush(stdout);
				if (fgets(objbuf,
				    sizeof(objbuf), stdin) == NULL)
					break;
			} while ((len = strlen(objbuf)) <= 1);
			lockid = strtol(objbuf, NULL, 16);
			if (lockid < 0 || lockid >= lockcount) {
				printf("Lock #%d out of range\n", lockid);
				continue;
			}

			/* Release the lock. */
			lock = locks[lockid];
			ret = dbenv->lock_put(dbenv, &lock);
			did_get = 0;
		}
		/* Print information about the operation. */
		switch (ret) {
		case 0:
			printf("Lock #%d %s\n", lockid,
			    did_get ? "granted" : "released");
			held += did_get ? 1 : -1;
			break;
		case DB_LOCK_NOTGRANTED:
			dbenv->err(dbenv, ret, NULL);
			break;
		case DB_LOCK_DEADLOCK:
			dbenv->err(dbenv, ret,
			    "lock_%s", did_get ? "get" : "put");
			break;
		default:
			dbenv->err(dbenv, ret,
			    "lock_%s", did_get ? "get" : "put");
			(void)dbenv->close(dbenv, 0);
			return (EXIT_FAILURE);
		}
	}

	printf("\nClosing lock region %ld locks held\n", held);

	if (locks != NULL)
		free(locks);

	/* Close the environment handle. */

	if ((ret = dbenv->close(dbenv, 0)) != 0) {
		fprintf(stderr,
		    "%s: dbenv->close: %s\n", progname, db_strerror(ret));
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

/*
 * db_init --
 *	Initialize the environment.
 *
 * Parameters:
 *  home       the environment home directory
 *  maxlocks   the maximum permitted lock number in the environment
 *  do_unlink  the unlink flag, if set remove existing environment
 */
int
db_init(home, maxlocks, do_unlink)
	const char *home;
	u_int32_t maxlocks;
	int do_unlink;
{
	int ret;

	/* Create the environment object. */
	if ((ret = db_env_create(&dbenv, 0)) != 0) {
		fprintf(stderr, "%s: db_env_create: %s\n",
		    progname, db_strerror(ret));
		return (EXIT_FAILURE);
	}

	if (do_unlink) {
		/* Remove the existing environment. */
		if ((ret = dbenv->remove(dbenv, home, DB_FORCE)) != 0) {
			fprintf(stderr, "%s: dbenv->remove: %s\n",
			    progname, db_strerror(ret));
			return (EXIT_FAILURE);
		}

		/* 
		 * The old handle cannot be used after calling remove.
		 * Create a new one.
		 */
		if ((ret = db_env_create(&dbenv, 0)) != 0) {
			fprintf(stderr, "%s: db_env_create: %s\n",
			    progname, db_strerror(ret));
			return (EXIT_FAILURE);
		}
	}

	/*
	 * Prefix any error messages with the name of this program and a ':'.
	 * Setting the errfile to stderr is not necessary, since that is the
	 * default; it is provided here as a placeholder showing where one
	 * could direct error messages to an application-specific log file.
	 */
	dbenv->set_errfile(dbenv, stderr);
	dbenv->set_errpfx(dbenv, progname);

	/* Set the maximum number of locks supported by the environment. */
	if (maxlocks != 0)
		dbenv->set_lk_max_locks(dbenv, maxlocks);

	/* 
	 * Open the environemnt. The DB_INIT_LOCK specifies that the lock
	 * subsystem is initialized.
	 */
	if ((ret =
	    dbenv->open(dbenv, home, DB_CREATE | DB_INIT_LOCK, 0)) != 0) {
		dbenv->err(dbenv, ret, NULL);
		(void)dbenv->close(dbenv, 0);
		return (EXIT_FAILURE);
	}
	return (0);
}

/*
 * usage --
 *	Describe this program's command line options, then exit.
 */
int
usage()
{
	(void)fprintf(stderr,
	    "usage: %s [-u] [-h home] [-m maxlocks]\n", progname);
	return (EXIT_FAILURE);
}
