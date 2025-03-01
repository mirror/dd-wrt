/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1996, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * ex_external_file --
 *	This program shows how to store large values as Berkeley DB external.
 *	files.  It creates a DB_BTREE database configured with external file
 *	support, inserts a few thousand key/value pairs, and then retrieves
 *	them. It also uses the DB_STREAM interface to stream an external file
 *	value.  This method is particularly useful for very large
 *	values, e.g., multimedia files or geological data sets.
 *
 *   The function db_ext_file_example() contains all the Berkeley DB API calls.
 *
 * Source File:
 *	ex_external_file.c
 * Environment:
 *	EX_EXTFILEDIR
 * Database:
 *	ex_ext_file_db1.db
 *
 * Options:
 *	-h	Set the environment home to this directory, not EX_EXTFILEDIR.
 *
 * $Id$
 */

#include <sys/types.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CMD_LEN 25
#ifdef _WIN32
extern int getopt(int, char * const *, const char *);
#else
#include <unistd.h>
#endif

#include <db.h>

int db_ext_file_example __P((const char *, FILE *, const char *));
int usage __P((void));

const char *progname = "ex_external_file";		/* Program name. */

/*
 * An example of a program that stores data items in Berkeley DB external
 * files.
 */
int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;
	extern int optind;
	char *cmd;
	const char *home;
	int ch;

	home = "EX_EXTFILEDIR";
	while ((ch = getopt(argc, argv, "b:h:")) != EOF)
		switch (ch) {
		case 'h':
			home = optarg;
			break;
		case '?':
		default:
			return (usage());
		}
	argc -= optind;
	argv += optind;

	if (argc != 0)
		return (usage());

	if ((cmd = malloc(strlen(home) + CMD_LEN)) == NULL)
		return (EXIT_FAILURE);

#ifdef _WIN32
	sprintf(cmd, "rmdir %s /q/s", home);
#else
	sprintf(cmd, "rm -rf %s", home);
#endif
	system(cmd);
	sprintf(cmd, "mkdir %s", home);
	system(cmd);
	free(cmd);

	if (db_ext_file_example(home, stderr, progname) != 0)
		return (EXIT_FAILURE);

	return (EXIT_SUCCESS);
}

/*
 * db_ext_file_example --
 *	
 *	Setup phase:
 *		Create an environment and a database; configure it with a
 *		1000 byte threshold for records to be stored as external files.
 *	Insert phase:
 *  		Insert records into the database. Most of the records have
 *		external file-sized values. A few of the values are small and
 *		are stored as regular, on-page values.
 *	Process phase:
 *		Retrieve records from the database  using regular DB->get() API.
 *		Next use the file i/o-like DB_STREAM API to read and write 
 *		an external file.
 *
 *	Delete phase:
 *		Delete every other record.
 *  
 *	Shutdown
 *		Close the database and environment.
 */
int
db_ext_file_example(home, errfp, progname)
	const char *home;
	FILE *errfp;
	const char *progname;
{
	DB_ENV *dbenv;
	DB_STREAM *dbs;
	DB_TXN *txn;
	DB *dbp;
	DBC *dbc;
	DBT data, key;
	int ret;
	db_off_t size;
	unsigned int i;
	u_int32_t threshold;

	dbc = NULL;
	dbenv = NULL;
	dbp = NULL;
	dbs = NULL;
	txn = NULL;
	/* 
	 * Values at least this size will be external filess. It can be any
	 * u_int32_t.  External file thresholds this low will generally be
	 * slower than somewhat larger values; 
	 */
	threshold = 1000;

	printf("Setup environment in %s\n", home);
	/*
	 * Create an environment object and begin to configure it. First
	 * specify the destination FILE * for error messages, then prefix
	 * each error message with the name of this program.
	 */
	if ((ret = db_env_create(&dbenv, 0)) != 0) {
		fprintf(errfp, "%s: %s\n", progname, db_strerror(ret));
		return (1);
	}
	dbenv->set_errfile(dbenv, errfp);
	dbenv->set_errpfx(dbenv, progname);

	/* Increase the cache size to 64MB, more than enough for this test. */
	if ((ret = dbenv->set_cachesize(dbenv, 0, 64 * 1024 *1024, 0)) != 0) {
		dbenv->err(dbenv, ret, "set_cachesize");
		goto err_cleanup;
	}

	/* Turn on external file support in the environment. */
	if ((ret = dbenv->set_ext_file_threshold(dbenv, threshold, 0)) != 0) {
		dbenv->err(dbenv, ret, "set_ext_file_threshold");
		goto err_cleanup;
	}

	/* Open the environment with full transactional support. */
	if ((ret = dbenv->open(dbenv, home,
	    DB_CREATE | DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL |
	    DB_INIT_TXN, 0644)) != 0) {
		dbenv->err(dbenv, ret, "environment open: %s", home);
		goto err_cleanup;
	}

	/*
	 * Create a database handle in the environment, inheriting its external
	 * file threshold. We could give the database an external file threshold
	 * different from that of the environment by calling
	 * dbp->set_ext_file_threshold() before the dbp->open().
	 */
	if ((ret = db_create(&dbp, dbenv, 0)) != 0) {
		dbenv->err(dbenv, ret, "database create");
		goto err_cleanup;
	}

	/*
	 * Open the database handle, creating the btree on disk. The DB_HASH
	 * and DB_HEAP database types also support external files.
	 */
	if ((ret = dbp->open(dbp, NULL, "ex_ext_file_db1.db", NULL,
	    DB_BTREE, DB_CREATE | DB_AUTO_COMMIT, 0644)) != 0) {
		dbenv->err(dbenv, ret, "database open");
		goto err_cleanup;
	}

	/*
	 * Construct a pair with an integer key and a large enough alphabetic
	 * value that it will be stored as an external file. The key is never
	 * stored as an external file; only the data may be.
	 */
	memset(&key, 0, sizeof(DBT));
	key.ulen = key.size = sizeof(int);
	key.flags = DB_DBT_USERMEM;
	memset(&data, 0, sizeof(DBT));
	data.size = data.ulen = threshold * 4;
	data.data = malloc(data.size);
	for (i = 0; i < data.size; i++)
		((char *)data.data)[i] = 'a' + (i % 26);
	data.flags = DB_DBT_USERMEM;

	/*
	 * Input phase:
	 *
	 * Insert 5000 key-value pairs, where most of them are large enough
	 * that they are stored as external files. One in 23 of them are
	 * smaller, regular values.
	 */
#define COUNT 5000
	for (i = 1; i <= COUNT; i++) {
		if (i % 1000 == 0)
			printf("Adding record #%d\n", i);
		key.data = &i;
		/*
		 * Most of the values are external files, but occasionally
		 * insert an on-page value.
		 */
		if (i % 23 == 0)
			data.size = 20;
		if ((ret = dbp->put(dbp, NULL, &key, &data, 0)) != 0) {
			dbenv->err(dbenv, ret, "external file put key %u", i);
			goto err_cleanup;
		}

		/*
		 * If a short value was just written, restore the size to the
		 * full length, causing the next values to be external files.
		 */
		if (i % 23 == 0)
			data.size = data.ulen;
	}

	/*
	 * Process phase, step 1: DB->get()
	 *
	 * Retrieve half of the records; verify that the inital part of the
	 * values has the bytes that were inserted. In this step external files
	 * and regular values are accessed in the same manner.
	 */
	for (i = 1; i <= COUNT; i += 2) {
		if (i % 1000 == 1)
			printf("Retrieving record #%d\n", i);
		key.data = &i;
		memset(data.data, 0, data.size);
		if ((ret = dbp->get(dbp, NULL, &key, &data, 0)) != 0) {
			dbenv->err(dbenv, ret, "external file get");
			goto err_cleanup;
		}
		if (strncmp((char *)data.data, "abcdefghijklmno", 15) != 0)
			printf("external file data mismatch: key %d; value %.15s\n",
				i, data.data);
	}
	
	/*
	 * Process phase, step 2: DB_STREAM->read(), DB_STREAM->write(),
	 *
	 * External files can also be accessed using the DB_STREAM API,
	 * accessing the value in smaller, manageable sections. This involves
	 * opening a cursor to get to a record, then opening a stream to access
	 * the data at the cursor's position.
	 */
	if ((ret = dbenv->txn_begin(dbenv, NULL, &txn, 0)) != 0){
		dbenv->err(dbenv, ret, "txn");
		goto err_cleanup;
	}

	if ((ret = dbp->cursor(dbp, txn, &dbc, 0)) != 0) {
		dbenv->err(dbenv, ret, "cursor");
		goto err_cleanup;
	}

	/*
	 * Set the cursor to the first value, which happens to be an external
	 * file because the first key-value pair written above had a large size.
	 * Use DB_DBT_PARTIAL with dlen == 0 to avoid returning any
	 * external file data during the initial dbc->get().
	 */
	data.flags = DB_DBT_USERMEM | DB_DBT_PARTIAL;
	data.dlen = 0;
	if ((ret = dbc->get(dbc, &key, &data, DB_FIRST)) != 0) {
		dbenv->err(dbenv, ret, "dbc->get(DB_FIRST)");
		goto err_cleanup;
	}
	data.flags = DB_DBT_USERMEM;

	/*
	 * Create a stream for the external file which the cursor points to. If
	 * the cursor is not positioned on an external file, this returns
	 * EINVAL.
	 */
	if ((ret = dbc->db_stream(dbc, &dbs, DB_STREAM_WRITE)) != 0) {
		dbenv->err(dbenv, 0, "Creating stream.");
		goto err_cleanup;
	}
	/* Get the size of the external file. */
	if ((ret = dbs->size(dbs, &size, 0)) != 0) {
		dbenv->err(dbenv, 0, "Stream size.");
		goto err_cleanup;
	}
	/* Read from the external file. */
	if ((ret = dbs->read(dbs, &data, 0, (u_int32_t)size, 0)) != 0) {
		dbenv->err(dbenv, 0, "Stream read.");
		goto err_cleanup;
	}
	/*
	 * Write that data back to the external file. This increases the
	 * external file's size by writing the entire value to a later position
	 * in the external file -- the middle of it.
	 */
	if ((ret = dbs->write(dbs, &data, size/2, 0)) != 0) {
		dbenv->err(dbenv, 0, "Stream write.");
		goto err_cleanup;
	}
	/*
	 * Close the stream, flushing the changes to the filesystem. The flags
	 * parameter does not currently accept any options, it must be zero.
	 */
	ret = dbs->close(dbs, 0);
	dbs = NULL;
	if (ret != 0) {
		dbenv->err(dbenv, ret, "Stream close");
		goto err_cleanup;
	}
	/* Close the cursor and commit the transaction. */
	ret = dbc->close(dbc);
	dbc = NULL;
	if (ret != 0) {
		dbenv->err(dbenv, ret, "Stream cursor close");
		goto err_cleanup;
	}
	ret = txn->commit(txn, 0);
	txn = NULL;
	if (ret != 0) {
		dbenv->err(dbenv, ret, "Stream transaction close");
		goto err_cleanup;
	}

	/* The data DBT is not used after this point; free its memory. */
	free(data.data);
	data.data = NULL;

	/*
	 * Delete phase:
	 *
	 * Delete every other record. Whenever a key-value pair contains an
	 * external file, the file is automatically deleted.
	 */
	for (i = 1; i <= COUNT; i += 2) {
		if (i % 1000 == 1)
			printf("Deleting record #%d\n", i);
		key.data = &i;
		if ((ret = dbp->del(dbp, NULL, &key, 0)) != 0) {
			dbenv->err(dbenv, ret, "external file del key %u", i);
			goto err_cleanup;
		}
	}

	/*
	 * Shutdown:
	 *
	 * Close the database, then the environment.
	 */
	ret = dbp->close(dbp, 0);
	dbp = NULL;
	if (ret != 0) {
		fprintf(stderr, "database close: %s\n", db_strerror(ret));
		goto err_cleanup;
	}
	if ((ret = dbenv->close(dbenv, 0)) != 0) {
		fprintf(stderr, "DB_ENV->close: %s\n", db_strerror(ret));
		return (1);
	}

	/* Success! */
	return (0);


	/* This label is used when the Berkeley DB API returns an error which
	 * requires releasing any locally acquired resources, such as allocated
	 * database handles or active transactions.
	 */
err_cleanup:
	if (data.data != NULL)
		free(data.data);
	if (dbs != NULL)
		dbs->close(dbs, 0);
	if (txn != NULL)
		txn->abort(txn);
	if (dbc != NULL)
		dbc->close(dbc);
	if (dbp != NULL)
		dbp->close(dbp, 0);
	if (dbenv != NULL)
		dbenv->close(dbenv, 0);

	return (ret);
}

/*
 * usage --
 *	Display the command line option.
 */
int
usage()
{
	fprintf(stderr, "usage: %s [-h <home>]\n", progname);
	return (EXIT_FAILURE);
}
