/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2011, 2017 Oracle and/or its affiliates.  All rights reserved.
 */

#include "../utilities/bdb_xa_util.h"

#include <time.h>
#include <atmi.h>
#include <fml1632.h>
#include <fml32.h>
#include <tx.h>
#include <db.h>
#include <stdlib.h>
#include <errno.h>

static char itoc(int num)
{
	char vals[] = {'0', '1', '2', '3', '4',
		'5', '6', '7', '8', '9'};
	if (num > 9 || num < 0)
		return '0';
	return vals[num];
}

/*
 * This function syncs all clients by having each one create
 * a file called [num]_[test_num]_[call], and attempting to read the files
 * the other clients create until it succeeds.
 */
int sync_clients(num, num_clients, call, test_num)
	int num;
	int num_clients;
	int call;
	int test_num;
{
	FILE *fp;
	int i, ret;
	char name[6];

	name[5] = 0;
	name[0] = itoc(num);
	name[1] = '_';
	name[2] = itoc(test_num);
	name[3] = '_';
	name[4] = itoc(call);

	//printf("client %d waiting in call %d\n", num, call);

	fp = fopen(name, "w");
	if (fp == NULL)
		return ENOENT;
	fprintf(fp, "success");
	if ((ret = fclose(fp)) != 0)
		return (ret);
	for (i = 1; i < (num_clients + 1); i++) {
		if (i == num)
			continue;
		name[0] = itoc(i);
		while ((fp = fopen(name, "r")) == NULL) {
			sleep(1);
		}
	}
	//printf ("client %d continuing in call %d\n", num, call);
	return (0);
}

/*
 * Print callback for __db_prdbt.
 */
int
pr_callback(handle, str_arg)
	void *handle;
	const void *str_arg;
{
	char *str;
	FILE *f;

	str = (char *)str_arg;
	f = (FILE *)handle;

	if (fprintf(f, "%s", str) != (int)strlen(str))
		return (-1);

	return (0);
}

/*
 * Initialize an XA server and allocate and open its database handles
 */
int
init_xa_server(num_db, progname, use_mvcc)
	int num_db;
	const char *progname;
	int use_mvcc;
{
	int ret, i, j;
	u_int32_t temp;
	u_int32_t flags = DB_AUTO_COMMIT|DB_CREATE|DB_THREAD;

	if (use_mvcc)
		 flags |= DB_MULTIVERSION;
	  
	if (verbose)
		printf("%s: called\n", progname);

	/* Open resource managers. */
	if (tx_open() == TX_ERROR) {
		fprintf(stderr, "tx_open: TX_ERROR\n");
		return (-1);
	}

	/* Seed random number generator. */
	srand((u_int)(time(NULL) | getpid()));

	for (i = 0; i < MAX_NUMDB; i++)
		dbs[i] = NULL;

	/* 
	 * Open XA database handles.  Databases must be opened 
	 * outside of a global transaction 
	 * because the open event is held in process local memory.
	 */
	for (i = 0; i < num_db; i++) {
		if ((ret = db_create(&dbs[i], NULL, DB_XA_CREATE)) != 0) {
			fprintf(stderr, "db_create: %s\n", db_strerror(ret));
			goto err;
		}

		dbs[i]->set_errfile(dbs[i], stderr);
		if ((ret = dbs[i]->open(dbs[i], NULL,
		    db_names[i], NULL, DB_BTREE, flags, 0664)) != 0) {
			fprintf(stderr, "DB open: %s: %s\n", db_names[i], 
			    db_strerror(ret));
			goto err;
		}
	}

	if (verbose)
		printf("%s: tpsvrinit: initialization done\n", progname);

	return (0);

	/* Clean up on error. */
err:	for (i = 0; i < num_db; i++) {
		if (dbs[i] != NULL)
			(void) dbs[i]->close(dbs[i], 0);
		dbs[i] = NULL;
	}
	return (-1);
}

/* 
 * Called when the servers are shutdown.  This closes all open 
 * database handles. 
 */
void
close_xa_server(num_db, progname)
	int num_db;
	const char *progname;
{
	int i;
  
	for (i = 0; i < num_db; i++) {
		if (dbs[i] != NULL)
			(void) dbs[i]->close(dbs[i], 0);
		dbs[i] = NULL;
	}

	tx_close();

	if (verbose)
		printf("%s: tpsvrdone: shutdown done\n", progname);
}

/*
 * check_data --
 *	Compare data between two databases to ensure that they are identical.
 */
int check_data(dbenv1, name1, dbenv2, name2, progname)
     DB_ENV *dbenv1;
     const char *name1;
     DB_ENV *dbenv2;
     const char *name2;
     const char * progname;
{
	DB *dbp1, *dbp2;
	DBC *dbc1, *dbc2;
	DBT key1, data1, key2, data2;
	int ret, ret1, ret2;
	u_int32_t flags = DB_INIT_MPOOL | DB_INIT_LOG | DB_INIT_TXN |
	  DB_INIT_LOCK | DB_THREAD;

	dbp1 = dbp2 = NULL;
	dbc1 = dbc2 = NULL;
	
	/* Open table #1. */
	if ((ret = db_create(&dbp1, dbenv1, 0)) != 0 ||
	    (ret = dbp1->open(
	    dbp1, NULL, name1, NULL, DB_UNKNOWN, DB_RDONLY, 0)) != 0) {
		fprintf(stderr,
		    "%s: %s: %s\n", progname, name1, db_strerror(ret));
		goto err;
	}
	if (verbose)
		printf("%s: opened %s OK\n", progname, name1);

	/* Open table #2. */
	if ((ret = db_create(&dbp2, dbenv2, 0)) != 0 ||
	    (ret = dbp2->open(
	    dbp2, NULL, name2, NULL, DB_UNKNOWN, DB_RDONLY, 0)) != 0) {
		fprintf(stderr,
		    "%s: %s: %s\n", progname, name2, db_strerror(ret));
		goto err;
	}
	if (verbose)
		printf("%s: opened %s OK\n", progname, name2);

	/* Open cursors. */
	if ((ret = dbp1->cursor(dbp1, NULL, &dbc1, 0)) != 0 ||
	    (ret = dbp2->cursor(dbp2, NULL, &dbc2, 0)) != 0) {
		fprintf(stderr,
		    "%s: DB->cursor: %s\n", progname, db_strerror(ret));
		goto err;
	}
	if (verbose)
		printf("%s: opened cursors OK\n", progname);

	/* Compare the two databases. */
	memset(&key1, 0, sizeof(key1));
	memset(&data1, 0, sizeof(data1));
	memset(&key2, 0, sizeof(key2));
	memset(&data2, 0, sizeof(data2));
	for (;;) {
		ret1 = dbc1->get(dbc1, &key1, &data1, DB_NEXT);
		ret2 = dbc2->get(dbc2, &key2, &data2, DB_NEXT);
		if (ret1 != 0 || ret2 != 0)
			break;
		if (verbose) {
			__db_prdbt(&key1, 0, "get: key1: %s\n", stdout, 
			   pr_callback , 0, 0);
			__db_prdbt(&key2, 0, "get: key2: %s\n", stdout, 
			    pr_callback, 0, 0);
			__db_prdbt(&data1, 0, "get: data1: %s\n", stdout, 
			   pr_callback, 0, 0);
			__db_prdbt(&data2, 0, "get: data2: %s\n", stdout, 
			    pr_callback, 0, 0);
		}
		/* Compare the key and data.  */
		if (key1.size != key2.size ||
		    memcmp(key1.data, key2.data, key1.size) != 0 ||
		    data1.size != data2.size ||
		    memcmp(data1.data, data2.data,
		    data1.size) != 0)
			goto mismatch;
	}
	if (ret1 != ret2) {
mismatch:	fprintf(stderr, 
		    "%s: DB_ERROR: databases 1 and 2 weren't identical\n", 
		    progname);
		ret = 1;
	}

err:	if (dbc1 != NULL)
		(void)dbc1->close(dbc1);
	if (dbc2 != NULL)
		(void)dbc2->close(dbc2);
	if (dbp1 != NULL)
		(void)dbp1->close(dbp1, 0);
	if (dbp2 != NULL)
		(void)dbp2->close(dbp2, 0);

	if (verbose) {
		if (ret == 0)
			printf("Data check succeeded.\n", progname);
		else
			printf("Data check failed.\n", progname);
	}

	return (ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}
