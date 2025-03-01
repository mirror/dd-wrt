/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2011, 2017 Oracle and/or its affiliates.  All rights reserved.
 */

#include <pthread.h>
#include <db.h>

/* Maximum number of db handles that init/close_xa_server will handle. */
#define MAX_NUMDB	4

/* Names for the databases. */
static char *db_names[] = {"table1.db", "table2.db", "table3.db", "table4.db"};

/* Debugging output. */
#ifdef VERBOSE
static int verbose = 1;				
#else
static int verbose = 0;
#endif

/* Table handles. */				
DB *dbs[MAX_NUMDB]; 

/*
 * This function syncs all clients by having each one create
 * a file called [num]_[test_num]_[call], and attempting to read the files
 * the other clients create until it succeeds.
 */
int sync_clients(int num, int num_clients, int call, int test_num);

/*
 * Print callback for __db_prdbt.
 */
int pr_callback(void *handle, const void *str_arg);
/*
 * Initialize an XA server and allocates and opens database handles.  
 * The number of database handles it opens is the value of the argument
 *  num_db.
 */
int init_xa_server(int num_db, const char *progname, int use_mvcc);

/* 
 * Called when the servers are shutdown.  This closes all open 
 * database handles. num_db is the number of open database handles.
 */
void close_xa_server(int num_db, const char *progname);

/*
 * check_data --
 *	Compare data between two databases to ensure that they are identical.
 */
int check_data(DB_ENV *dbenv1, const char *name1, DB_ENV *dbenv2, 
	       const char *name2, const char *progname);
