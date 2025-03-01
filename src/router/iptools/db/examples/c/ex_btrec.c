/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * ex_btrec-- A basic example of using record numbers in a btree.
 *  
 *   This example program shows how to store automatically numbered records in a
 *   btree database, one of the kinds of access methods provided by Berkeley DB.
 *   Access methods determine how key-value pairs are stored in the file.
 *   B-tree is one of the most commonly used types because it supports sorted
 *   access to variable length records.
 *   
 *   The program first reads 1000 records from file "wordlist" and then stores
 *   the data in the "access.db" database. The key of each record is the record
 *   number concatenated with a word from the word list; the data is the same,
 *   but in reverse order. Then it opens a cursor to fetch key/data pairs.
 *   The user selects a record by entering its record number. Both the specified
 *   record and the one following it will be displayed.
 *   
 * Database: access.db 
 * Wordlist directory: ../test/tcl/wordlist
 *
 * $Id: ex_btrec.c,v 0f73af5ae3da 2010/05/10 05:38:40 alexander $
 */

#include <sys/types.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <db.h>


#define	DATABASE	"access.db"
#define	WORDLIST	"../test/tcl/wordlist"
int	main __P((void));

int	ex_btrec __P((void));
void	show __P((const char *, DBT *, DBT *));

int
main()
{
	return (ex_btrec() == 1 ? EXIT_FAILURE : EXIT_SUCCESS);
}

int
ex_btrec()
{
	DB *dbp;		/* Handle of the main database to store the content of wordlist. */
	DBC *dbcp;		/* Handle of database cursor used for putting or getting the word data. */
	DBT key;		/* The key to dbcp->put()/from dbcp->get(). */
	DBT data;		/* The data to dbcp->put()/from dbcp->get(). */
	DB_BTREE_STAT *statp;	/* The statistic pointer to record the total amount of record number. */
	FILE *fp;		/* File pointer that points to the wordlist. */
	db_recno_t recno;	/* Record number to retrieve a record in access.db database. */
	size_t len;		/* The size of buffer. */
	int cnt;		/* The count variable to read records from wordlist. */
	int ret;		/* Return code from call into Berkeley DB. */
	char *p;		/* Pointer to store buffer. */
	char *t;		/* Pointer to store reverse buffer. */
	char buf[1024];		/* Buffer to store key value. */
	char rbuf[1024];	/* Reverse buffer to store data value. */
	const char *progname = "ex_btrec";		/* Program name. */

	/* Open the text file containing the words to be inserted. */
	if ((fp = fopen(WORDLIST, "r")) == NULL) {
		fprintf(stderr, "%s: open %s: %s\n",
		    progname, WORDLIST, db_strerror(errno));
		return (1);
	}

	/* Remove the previous database. */
	(void)remove(DATABASE);

	/* Create the database handle. */
	if ((ret = db_create(&dbp, NULL, 0)) != 0) {
		fprintf(stderr,
		    "%s: db_create: %s\n", progname, db_strerror(ret));
		return (1);
	}

	/*
	 * Prefix any error messages with the name of this program and a ':'.
	 * Setting the errfile to stderr is not necessary, since that is the
	 * default; it is provided here as a placeholder showing where one
	 * could direct error messages to an application-specific log file.
	 */
	dbp->set_errfile(dbp, stderr);
	dbp->set_errpfx(dbp, progname);

	/* Configure the database to use 1KB page sizes and record numbering. */
	if ((ret = dbp->set_pagesize(dbp, 1024)) != 0) {
		dbp->err(dbp, ret, "set_pagesize");
		return (1);
	}
	if ((ret = dbp->set_flags(dbp, DB_RECNUM)) != 0) {
		dbp->err(dbp, ret, "set_flags: DB_RECNUM");
		return (1);
	}
	/* Open it with DB_CREATE, making it a DB_BTREE. */
	if ((ret = dbp->open(dbp,
	    NULL, DATABASE, NULL, DB_BTREE, DB_CREATE, 0664)) != 0) {
		dbp->err(dbp, ret, "open: %s", DATABASE);
		return (1);
	}

	/*
	 * Insert records in the wordlist into the database. The key is the
	 * word preceded by its record number, and the data contains the same
	 * characters in the key, but in reverse order.
	 */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));
	for (cnt = 1; cnt <= 1000; ++cnt) {
		(void)sprintf(buf, "%04d_", cnt);
		if (fgets(buf + 4, sizeof(buf) - 4, fp) == NULL)
			break;
		/* The values exclude the trailing newline, hence the -1. */
		len = strlen(buf) - 1;
		/*
		 * Fill the reverse buffer 'rbuf' with the characters from
		 * 'buf', but in reverse order.
		 */
		for (t = rbuf, p = buf + len; p > buf;)
			*t++ = *--p;
		*t++ = '\0';

		/*
		 * Now that we have generated the values for the key and data
		 * items, set the DBTs to point to them.
		 */
		key.data = buf;
		data.data = rbuf;
		data.size = key.size = (u_int32_t)len;

		if ((ret =
		    dbp->put(dbp, NULL, &key, &data, DB_NOOVERWRITE)) != 0) {
			dbp->err(dbp, ret, "DB->put");
			if (ret != DB_KEYEXIST)
				goto err1;
		}
	}

	/* We are done with the file of words. */
	(void)fclose(fp);

	/* Get the database statistics and print the total number of records. */
	if ((ret = dbp->stat(dbp, NULL, &statp, 0)) != 0) {
		dbp->err(dbp, ret, "DB->stat");
		goto err1;
	}
	printf("%s: database contains %lu records\n",
	    progname, (u_long)statp->bt_ndata);
	free(statp);

	/* Acquire a cursor for sequential access to the database. */
	if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
		dbp->err(dbp, ret, "DB->cursor");
		goto err1;
	}

	/*
	 * Repeatly prompt the user for a record number, then retrieve and
	 * display that record as well as the one after it. Quit on EOF or
	 * when a zero record number is entered.
	 */
	for (;;) {
		/* Get a record number. */
		printf("recno #> ");
		fflush(stdout);
		if (fgets(buf, sizeof(buf), stdin) == NULL)
			break;
		recno = atoi(buf);
		/*
		 * Zero is an invalid record number: exit when that (or a
		 * non-numeric string) is entered.
		 */
		if (recno == 0)
			break;

		/*
		 * Reset the key each time, the dbp->get() routine returns
		 * the key and data pair, not just the key!
		 */
		key.data = &recno;
		key.size = sizeof(recno);
		if ((ret = dbcp->get(dbcp, &key, &data, DB_SET_RECNO)) != 0)
			goto get_err;

		/* Display the key and data. */
		show("k/d\t", &key, &data);

		/* DB_NEXT moves the cursor to the next record. */
		if ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) != 0)
			goto get_err;
		
		/* Display the successor. */
		show("next\t", &key, &data);

		/*
		 * Retrieve the record number for the successor into "recno"
		 * and print it. Set data flags to DB_DBT_USERMEM so that the
		 * record number will be retrieved into a proper, aligned
		 * integer, which is needed by some hardware platforms. It
		 * also makes it easier to print the value: no u_int32_t
		 * casting is needed.
		 */
		data.data = &recno;
		data.size = sizeof(recno);
		data.ulen = sizeof(recno);
		data.flags |= DB_DBT_USERMEM;
		if ((ret = dbcp->get(dbcp, &key, &data, DB_GET_RECNO)) != 0) {
get_err:		dbp->err(dbp, ret, "DBcursor->get");
			if (ret != DB_NOTFOUND && ret != DB_KEYEMPTY)
				goto err2;
		} else
			printf("retrieved recno: %lu\n", (u_long)recno);

		/* Reset the data DBT. */
		memset(&data, 0, sizeof(data));
	}

	/* Close the cursor, then its database. */
	if ((ret = dbcp->close(dbcp)) != 0) {
		dbp->err(dbp, ret, "DBcursor->close");
		goto err1;
	}
	if ((ret = dbp->close(dbp, 0)) != 0) {
		/*
		 * This uses fprintf rather than dbp->err because the dbp has
		 * been deallocated by dbp->close() and may no longer be used.
		 */
		fprintf(stderr,
		    "%s: DB->close: %s\n", progname, db_strerror(ret));
		return (1);
	}

	return (0);

err2:	(void)dbcp->close(dbcp);
err1:	(void)dbp->close(dbp, 0);
	return (ret);

}

/*
 * show --
 *	Display a key/data pair.
 *
 * Parameters:
 *  msg		print message
 *  key		the target key to print
 *  data	the target data to print
 */
void
show(msg, key, data)
	const char *msg;
	DBT *key, *data;
{
	printf("%s%.*s : %.*s\n", msg,
	    (int)key->size, (char *)key->data,
	    (int)data->size, (char *)data->data);
}
