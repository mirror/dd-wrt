/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * ex_sequence--
 *    This sample program shows using sequences to automatically generate item identifiers.
 *    Sequences provide an arbitrary number of persistent objects that return an increasing
 *    or decreasing sequence of integers. Opening a sequence handle associates it with a record
 *    in a database. The handle can maintain a cache of values from the database so that a
 *    database update is not needed as the application allocates a value.
 *
 *    The program first creates and opens a database object. Then a sequence is created and opened
 *    within the database. A sequence is stored as a record pair in a database. After the sequence 
 *    is opened, the program gets and stores the sequence elements. Finally it prints the sequence
 *    number.
 *
 *    At the end of the program, everything is closed including the database and sequence.
 *
 * Program name: ex_sequence
 * Database: sequence.db
 * Sequence: my_sequence
 *
 * Options:
 *   -r    remove existed database.
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

/* Forward declarations */

#define	DATABASE	"sequence.db"
#define	SEQUENCE	"my_sequence"
int main __P((int, char *[]));
int usage __P((void));

int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern int optind;
	DB *dbp;		/* The database handle. */
	DB_SEQUENCE *seq;	/* The sequence handle. */
	DBT key;		/* The key of the sequence. */
	int ch;			/* The current command line option char. */
	int i;			/* Temporary iteration number. */
	int ret;		/* Return code from call into Berkeley DB. */
	int rflag;		/* -r option: If set remove existing db. */
	db_seq_t seqnum;	/* The retrieved sequence number. */
	const char *database;	/* The database name. */
	const char *progname = "ex_sequence";	/* Program name. */

	dbp = NULL;
	seq = NULL;

	rflag = 0;

	/* Parse the command line arguments. */
	while ((ch = getopt(argc, argv, "r")) != EOF)
		switch (ch) {
		case 'r':		/* Remove existing database. */
			rflag = 1;
			break;
		case '?':		/* Display help messages and exit. */
		default:
			return (usage());
		}
	argc -= optind;
	argv += optind;

	/* Accept the optional database name. */
	database = *argv == NULL ? DATABASE : argv[0];

	/* Optionally discard the database. */
	if (rflag)
		(void)remove(database);

	/* Create and initialize database object, open the database. */
	if ((ret = db_create(&dbp, NULL, 0)) != 0) {
		fprintf(stderr,
		    "%s: db_create: %s\n", progname, db_strerror(ret));
		return (EXIT_FAILURE);
	}

	/* Define an error output stream. */
	dbp->set_errfile(dbp, stderr);

	/* Define an error message prefix. */
	dbp->set_errpfx(dbp, progname);

	/* 
	 * Now open the configured handle. In this example, we use a B-Tree
	 * database. You can experiment with other access methods in order to
	 * see how the behavior changes.
	 */
	if ((ret = dbp->open(dbp,
	    NULL, database, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
		dbp->err(dbp, ret, "%s: open", database);
		goto err;
	}

	/* Create sequence object within the database. */
	if ((ret = db_sequence_create(&seq, dbp, 0)) != 0) {
		dbp->err(dbp, ret, "db_sequence_create");
		goto err;
	}

	/* Zero out the DBTs before using them. */
	memset(&key, 0, sizeof(DBT));

	/* Set the sequence's key. */
	key.data = SEQUENCE;
	key.size = (u_int32_t)strlen(SEQUENCE);

	/* 
	 * Open a sequence,a sequence is stored as a record pair in a database.
	 * The sequence is referenced by the key used when the sequence is created.
	 */
	if ((ret = seq->open(seq, NULL, &key, DB_CREATE)) != 0) {
		dbp->err(dbp, ret, "%s: DB_SEQUENCE->open", SEQUENCE);
		goto err;
	}

	/* 
	 * Get the next 10 sequence numbers. Sequence numbers are stored in seqnum.
	 */
	for (i = 0; i < 10; i++) {
		if ((ret = seq->get(seq, NULL, 1, &seqnum, 0)) != 0) {
			dbp->err(dbp, ret, "DB_SEQUENCE->get");
			goto err;
		}

		/* 
		 * Print the sequence number. There's no portable way to print
		 * 64-bit numbers.
		 */
#ifdef _WIN32
		printf("Got sequence number %I64d\n", (int64_t)seqnum);
#else
		printf(
		    "Got sequence number %llu\n", (unsigned long long)seqnum);
#endif
	}

	/* Close the sequence and database. */
	if ((ret = seq->close(seq, 0)) != 0) {
		seq = NULL;
		dbp->err(dbp, ret, "DB_SEQUENCE->close");
		goto err;
	}
	if ((ret = dbp->close(dbp, 0)) != 0) {
		fprintf(stderr,
		    "%s: DB->close: %s\n", progname, db_strerror(ret));
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);

err:	if (seq != NULL)
		(void)seq->close(seq, 0);
	if (dbp != NULL)
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
	(void)fprintf(stderr, "usage: ex_sequence [-r] [database]\n");
	return (EXIT_FAILURE);
}
