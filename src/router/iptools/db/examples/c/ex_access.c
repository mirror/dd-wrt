/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * ex_access -- A basic Data Store example using B-tree indexing:
 *	Add lines of text to a database and then print them out.
 *
 *   This example demonstrates storage of text in a DB_BTREE. A B-tree index
 *   stores keys in sorted order.  A B-tree has two interesting properties:
 *   (1) any key can be retrieved with the same number of page accesses as any
 *   other key and (2) it is possible to perform range retrievals, obtaining all
 *   records from key1 to key2 in order. B-Trees are a very popular indexing
 *   mechanism for these reasons.
 *
 *   The program has an input phase which inserts records into the database and
 *   and output phase which displays the records.
 *
 *   The input phase reads lines of text from the standard input and puts them
 *   into to the database. The key of each record is the whole line; the data
 *   reverses the order of these characters.
 *
 *   The output phase the uses a cursor to iterate over all the records in the
 *   database, displaying each one.
 *   
 * Source File: ex_access.c
 * Database: access.db	- a DB_BTREE
 *
 * Options:
 *	-r	remove the database during startup; otherwise new records are
 *		added to any existing ones entered by previous executions.
 *
 *   [filename]	If specified, records will be stored in this file, rather than
 *		the default "access.db".
 *
 * $Id$
 */

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
extern int getopt(int, char * const *, const char *);
#else
#include <unistd.h>
#endif

#include <db.h>

#define	DATABASE	"access.db"
int main __P((int, char *[]));
int usage __P((void));

int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern int optind;	/* Set by getopt(): the # of argv's processed. */
	DB *dbp;		/* Handle of the database to be updated. */
	DBC *dbcp;		/* Cursor used for scanning over the database. */
	DBT key;		/* The key to dbp->put()/from dbcp->get(). */
	DBT data;		/* The data to dbp->put()/from dbcp->get(). */
	size_t len;		/* Length of the input line, discarding \n. */
	int ch;			/* The current command line option char. */
	int ret;		/* Return code from call into Berkeley DB. */
	int removeflag;		/* -r option: If set remove existing db. */
	char *database;		/* The name of the database to use. */
	char *p;		/* These two variables are used in reversing */
	char *t;		/* the input line, making the data to isnert. */
	char buf[BUFSIZ];	/* Buffer for reading from standard input. */
	char revbuf[BUFSIZ];	/* Buffer to hold the reversed 'buf[]' */
	const char *progname = "ex_access";		/* Program name. */

	removeflag = 0;		/* The default is not to remove the db. */

	/* Process any command line options. */
	while ((ch = getopt(argc, argv, "r")) != EOF)
		switch (ch) {
		case 'r':	/* Remove any existing database. */
			removeflag = 1;
			break;
		default:	/* Display terse help messages and exit. */
			return (usage());
		}
	argc -= optind;
	argv += optind;

	/* Accept optional database name. */
	database = *argv == NULL ? DATABASE : argv[0];

	/* Optionally discard any existing database. */
	if (removeflag)
		(void)remove(database);

	/* Create a database handle, to be configured, then opened. */
	if ((ret = db_create(&dbp, NULL, 0)) != 0) {
		fprintf(stderr,
		    "%s: db_create: %s\n", progname, db_strerror(ret));
		return (EXIT_FAILURE);
	}

	/*
	 * Prefix any error messages with the name of this program and a ':'.
	 * Setting the errfile to stderr is not necessary, since that is the
	 * default; it is provided here as a placeholder showing where one
	 * could direct error messages to an application-specific log file.
	 */
	dbp->set_errpfx(dbp, progname);
	dbp->set_errfile(dbp, stderr);

	/*
	 * Configure the database to use:
	 *	A database page size of 1024 bytes. It could be at most 64KB.
	 *	A database cache size of 0 GB and 32KB, in a single region.
	 */
	if ((ret = dbp->set_pagesize(dbp, 1024)) != 0) {
		dbp->err(dbp, ret, "set_pagesize");
		goto err_close_db;
	}
	if ((ret = dbp->set_cachesize(dbp, 0, 32 * 1024, 1)) != 0) {
		dbp->err(dbp, ret, "set_cachesize");
		goto err_close_db;
	}

	/*
	 * Now open the configured handle. The DB_BTREE specifies that it
	 * it will be a btree. You could experiment by changing that to DB_HASH
	 * in order to see how the behavior changes.
	 */
	if ((ret = dbp->open(dbp,
	    NULL, database, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
		dbp->err(dbp, ret, "%s: open", database);
		goto err_close_db;
	}

	/*
	 * Insert records into the database, where the key is the user input
	 * and the data is the user input in reverse order.
	 * Zeroing the DBTs prepares them for the dbp->put() calls below.
	 */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	/* Read and insert data until the user terminates input. */
	for (;;) {
		printf("input> ");
		fflush(stdout);

		/* Stop the input phase if EOF. */
		if (fgets(buf, sizeof(buf), stdin) == NULL)
			break;

		/*
		 * Find the length of the line; trim off any trailing newline
		 * so that doesn't get stored in the database.
		 */
		len = strlen(buf);
		if (len >= 1 && buf[len - 1] == '\n') {
			buf[len - 1] = '\0';
			len--;
		}
		/*
		 * Stop inserting on "exit" or "quit". Skip empty lines.
		 */
		if (strcmp(buf, "exit") == 0 || strcmp(buf, "quit") == 0)
			break;
		if (len == 0)
			continue;
		
		/*
		 * Put a reversed version of the line into the buffer of the
		 * data DBT.
		 */
		for (t = revbuf, p = buf + len; p >= buf;)
			*t++ = *--p;
		*t = '\0';

		key.data = buf;
		data.data = revbuf;

		/*
		 * A size_t (such as len) usually is bigger that a DBT size. The
		 * explicit cast prevents a warning message which is generated by
		 * several compilers.
		 */
		data.size = key.size = (u_int32_t)len;

		/*
		 * Add the record to the database. The DB_NOOVERWRITE flag
		 * causes DB_KEYEXIST to be returned if the key had already
		 * been put, either by a previous dbp->put() of this process,
		 * or because a prior execution of this program had inserted
		 * that key. Without DB_NOOVERWRITE, a dbp->put() of an already
		 * existing key replaces the old record with the new one.
		 */
		ret = dbp->put(dbp, NULL, &key, &data, DB_NOOVERWRITE);
		if (ret != 0) {
			/*
			 * Some kind of error was detected during the attempt to
			 * insert the record. The err() function is printf-like.
			 */
			dbp->err(dbp, ret, "DB(%s)->put(%s, %s)",
			    database, key.data, data.data);
			if (ret != DB_KEYEXIST)
				goto err_close_db;
		}
	}
	printf("\n");

	/*
	 * The user has finished inputting the data; now start printing it. The
	 * results will be sorted because this example uses a DB_BTREE database.
	 * 
	 * Create the cursor to use to iterate over the records.
	 */
	if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
		dbp->err(dbp, ret, "DB->cursor");
		goto err_close_db;
	}

	/*
	 * Initialize the key/data pair so that Berkeley DB manages the memory
	 * for the key and data DBTs, allocating and freeing them as needed.
	 */
	memset(&key, 0, sizeof(key));
	memset(&data, 0, sizeof(data));

	/* Walk through the database and print out the key/data pairs. */
	while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0)
		printf("%.*s : %.*s\n",
		    (int)key.size, (char *)key.data,
		    (int)data.size, (char *)data.data);

	/*
	 * The DB_NOTFOUND return code is expected when all the records have
	 * been retrieved. Any other return code is an error.
	 */
	if (ret != DB_NOTFOUND) {
		dbp->err(dbp, ret, "DBcursor->get");
		goto err_close_cursor;
	}

	/*
	 * A cursor should be closed before its underlying database is closed,
	 * even though the database close will automatically close its cursors.
	 * Closing now frees up resources sooner. Any errors detected now can be
	 * distinguished from any which might occur while closing the database.
	 * It also discourages a common mistake made with transactional cursors:
	 * forgetting to close them before committing their transaction.
	 */
	if ((ret = dbcp->close(dbcp)) != 0) {
		dbp->err(dbp, ret, "DBcursor->close");
		goto err_close_db;
	}
	if ((ret = dbp->close(dbp, 0)) != 0) {
		fprintf(stderr,
		    "%s: DB->close: %s\n", progname, db_strerror(ret));
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);

	/*
	 * An error has occurred while the cursor is open. First close it, then
	 * fall through to close the database handle.
	 */
err_close_cursor:
	(void)dbcp->close(dbcp);

err_close_db:
	/*
	 * Close the database. Inside the close() call Berkeley DB flushes the
	 * database cache out to the filesystem. The DB_NOSYNC option is ignored
	 * in cases such as this one, which do not use an explicitly created
	 * environment (DB_ENV).
	 */
	(void)dbp->close(dbp, 0);
	return (EXIT_FAILURE);
}

/*
 * usage --
 *	Describe this program's command line options, then exit.
 */
int
usage()
{
	(void)fprintf(stderr, "usage: ex_access [-r] [database]\n");
	return (EXIT_FAILURE);
}
