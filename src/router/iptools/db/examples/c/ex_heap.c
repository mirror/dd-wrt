/* 
 * See the file LICENSE for redistribution information.
 * 
 * Copyright (c) 2011, 2017 Oracle and/or its affiliates.  All rights reserved.
 * 
 * $Id$
 *
 * ex_heap -- A basic example about usage of heap access method.
 *   
 *   This program demonstrates the usage of heap access method of BDB and differences 
 *   between the heap and btree access method. There are many kinds of access methods
 *   for BDB, access methods determine how pages are stored in the file and how records
 *   are stored on a page. Heap access method is one of the common used access method 
 *   for BDB. The records are unsorted and the key indicates the page location within 
 *   heap access method. It's feature is to reuse space easily and enable the database
 *   to stay a constant size, which is different from btree access method.
 * 
 *   The application initially populates a database, and then proceeds to move into
 *   a process of adding and removing data. The sample application using a btree 
 *   database will be run if the flag is specified. The system will keep a fairly 
 *   constant amount of data in the database. After the insert/delete operation
 *   is completed. The system will calculate the execution time and physical file
 *   size to demonstrate the difference between btree database and heap database.
 *   The outcome shows that heap access method will maintain a constant database 
 *   size if the heap size is configured properly, while the btree database will
 *   continue to grow. 
 *
 * Database: heap.db, btree.db
 * Program name: ex_heap
 *
 * Options:
 *   -b		run sample application using a btree database 
 *   -c		specify the cache size for the environment
 *   -d		test on variable-length data (default:fix-length)
 *   -h [dir]	specify the home directory for the environment
 *   -n		specify the number of records per repetition (default:10000)
 *   -p		specify the pgsize of database
 *   -r		set number of repetition (a pair of insertion and deletion, default:1)
 *   -s		specify the heap size (bytes) for the heap database
 *   -S		specify the heap size (gbytes) for the heap database
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "db.h"


#ifndef lint
static const char copyright[] =
    "Copyright (c) 2011, 2017 Oracle and/or its affiliates.  All rights reserved.\n";
#endif

#define	BUFFER_LEN		30     /* Buffer size to hold data */
#define	NS_PER_MS		1000000/* Nanoseconds in a millisecond*/
#define	NS_PER_US		1000   /* Nanoseconds in a microsecond*/
#define	DEF_INIT_RECS		10000  /* Default initial records */
#define	DEF_RECS_PER_REP	10000  /* Default records per repeat. */
#define	DEF_REPEATS		1      /* Default repetition value */

/*
 * Average space each record needs, based on the data generated.
 * The ideal heap size for this example should be set as (bytes):
 * AVG_SPACE_PER_RECORD * (DEF_INIT_RECS + records inserted each repetition)
 */
#define	AVG_SPACE_PER_RECORD	36

#ifdef _WIN32
#include <sys/timeb.h>
#include <time.h>
#include <winsock2.h>
extern int getopt(int, char * const *, const char *);

/* Implement a basic high-resolution timer with a POSIX interface for Windows.*/
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	struct _timeb now;
	_ftime(&now);
	tv->tv_sec = (long)now.time;
	tv->tv_usec = now.millitm * NS_PER_US;
	return (0);
}
#else
#include <sys/time.h>
#include <unistd.h>
#endif

int	compare_int(DB *, const DBT *, const DBT *, size_t *);
int	delete_recs __P((DB *, DB_ENV *, int));
int	file_size __P((DB *, DBTYPE, int *));
int	generate_data __P((char [], int, int));
int	insert_btree __P((DB *, DB_ENV *, int, int, int));
int	insert_heap __P((DB *, DB_ENV *, int, int, int));
int	open_db __P((
    DB **, DB_ENV *, DBTYPE, char *, u_int32_t, u_int32_t, u_int32_t));
int	open_env __P((DB_ENV **, char *, u_int32_t));
int	run_workload __P((DB *, int, int, int));
void	usage __P((void));

const char *progname = "ex_heap"; /* Program name. */

int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;	/* Set by getopt(): the argv being processed. */
	DB_ENV *dbenv;		/* The environment handle. */
	DB *dbp;		/* The database handle. */
	u_int32_t cachesize;	/* The size of cache set for the environment. */
	u_int32_t ghpsize;	/* The heap size (gbytes) for the heap database. */
	u_int32_t hpsize;	/* The heap size (bytes) for the heap database. */
	u_int32_t pgsize;	/* The page size of database. */
	char *home;		/* The home directory for the program. */
	int ch;			/* The current command line option char. */
	int ret;		/* Return code from call into Berkeley DB. */
	int ret_t;		/* Return code from close operation. */
	int set_ghpsize;	/* Flag for setting gphsize. */
	int set_hpsize;		/* Flag for setting hpsize. */
	int test_btree;		/* Flag for running sample application using a btree database. */
	int test_var_data;	/* Flag for testing variable-length data. */

	int recs_per_rep;	/* Number of records per repetition. */
	int repeats;		/* Repetition value. */

	dbenv = NULL;
	dbp = NULL;
	cachesize = 0;
	ret = ret_t = set_ghpsize = set_hpsize = test_btree = 0;
	home = NULL;

	recs_per_rep = DEF_RECS_PER_REP;
	ghpsize = hpsize = pgsize = 0;
	repeats = DEF_REPEATS;
	test_var_data = 0; /* Default as fix-length data. */

	/* Parse the command line arguments */
	while ((ch = getopt(argc, argv, "bc:dh:n:p:r:S:s:")) != EOF)
		switch (ch) {
		case 'b':		/* Run sample application using a btree database. */
			test_btree = 1;
			break;
		case 'c':		/* Specify the cache size for the environment. */
			cachesize = atoi(optarg);
			break;
		case 'd':		/* Test on variable-length data. */
			test_var_data = 1;
			break;
		case 'h':		/* Specify the home directory for the environment. */
			home = optarg;
			break;
		case 'n':		/* Specify the number of records per repetition. */
			recs_per_rep = atoi(optarg);
			break;
		case 'p':		/* Specify the pgsize of database. */
			pgsize = atoi(optarg);
			break;
		case 'r':		/* Set number of repetition. */
			repeats = atoi(optarg);
			break;
		case 's':		/* Specify the heap size (bytes) for the heap database. */
			set_hpsize = 1;
			hpsize = atoi(optarg);
			break;
		case 'S':		/* Specify the heap size (gbytes) for the heap database. */
			set_ghpsize = 1;
			ghpsize = atoi(optarg);
			break;
		default:
			usage();
		}

	if (!home)
		usage();

	srand((int)time(NULL));

	/* 
	 * If heap size is not specified, then use our default configuration
	 * as follows.
	 */
	if (!set_hpsize && !set_ghpsize)
		hpsize = AVG_SPACE_PER_RECORD * (DEF_INIT_RECS + recs_per_rep);

	/* Open the evnironment for usage. */
	if ((ret = open_env(&dbenv, home, cachesize)) != 0) {
		fprintf(stderr, "%s: open_env: %s", progname, db_strerror(ret));
		goto err;
	}
	/* Open a heap database for insert/delete operations. */
	if ((ret = open_db(&dbp, dbenv, DB_HEAP, home,
	    ghpsize, hpsize, pgsize)) != 0) {
		dbenv->err(dbenv, ret, "Failed to open heap database.");
		goto err;
	}

	/*
	 * Perform requested rounds of insert/delete operations
	 * using heap database.
	 */
	if ((ret =
	    run_workload(dbp, repeats, recs_per_rep, test_var_data)) != 0) {
		dbenv->err(dbenv, ret,
		    "Failed to perform operations on heap database.");
		goto err;
	}

	if (test_btree) {
		/* Close the DB handle for heap. */
		if ((ret = dbp->close(dbp, 0)) != 0) {
			dbenv->err(dbenv, ret, "DB->close");
			goto err;
		}
		dbp = NULL;
		/* Open a b-tree database for insert/delete operations. */
		if ((ret =
		    open_db(&dbp, dbenv, DB_BTREE, home, 0, 0, pgsize)) != 0) {
			dbenv->err(dbenv, ret,
			    "Failed to open btree database.");
			goto err;
		}

		/* 
		 * Perform requested rounds of insert/delete operations
		 * using btree database.
		 */
		if ((ret = run_workload(dbp,
		    repeats, recs_per_rep, test_var_data)) != 0) {
			dbenv->err(dbenv, ret,
			    "Failed to perform operations on btree database.");
			goto err;
		}
	}
err:
	if (dbp != NULL && (ret_t = dbp->close(dbp, 0)) != 0) {
		dbenv->err(dbenv, ret_t, "DB->close");
		ret = (ret == 0 ? ret_t : ret);
	}

	if (dbenv != NULL && (ret_t = dbenv->close(dbenv, 0)) != 0) {
		fprintf(stderr, "%s: dbenv->close: %s", progname,
		    db_strerror(ret_t));
		ret = (ret == 0 ? ret_t : ret);
	}

	return (ret);
}

/*
 * run_workload --
 * 	Perform requested rounds of insert/delete operations.
 *
 * Parameters:
 *	dbp		the database handle
 *	repeats		the number of repetition (a pair of insertion and deletion)
 *	recs_per_rep	the number of records per repetition
 *	test_var	test on variable-length data flag
 */
int
run_workload(dbp, repeats, recs_per_rep, test_var)
	DB *dbp;
	int repeats, recs_per_rep, test_var;
{
	DB_ENV *dbenv;			/* The environment handle. */
	DBTYPE dbtype;			/* The access method used by the database. */
	u_int32_t ghpsize;		/* The heap size (gbytes). */
	u_int32_t hpsize;		/* The heap size (bytes). */
	struct timeval end_time;	/* The end time of each iteration. */
	struct timeval start_time;	/* The start time of each iteration. */
	double *time_secs;		/* The array of each iteration's run time. */
	int *db_file_sizes;		/* The array of file sizes after each iteration. */
	int fsize;			/* The file size after the current iteration. */
	int i;				/* Iteration number. */
	int ret;			/* The return value. */

	dbenv = dbp->dbenv;
	fsize = 0;
	time_secs = NULL;
	db_file_sizes = NULL;

	/* Get the access method of the database. */
	if ((ret = dbp->get_type(dbp, &dbtype)) != 0) {
		dbenv->err(dbenv, ret, "DB->get_type");
		goto err;
	}

	/* Get the heap size if the database uses the heap access method. */
	if (dbtype == DB_HEAP &&
	    (ret = dbp->get_heapsize(dbp, &ghpsize, &hpsize)) != 0) {
		dbenv->err(dbenv, ret, "DB->get_heapsize");
		goto err;
	}

	/* Initialize the array holding file sizes after each iteration. */
	if ((db_file_sizes =
	    (int *)malloc((repeats + 1) * sizeof(int))) == NULL) {
		fprintf(stderr,
		    "%s: Unable to allocate space for array db_file_sizes.\n",
		    progname);
		goto err;
	}
	memset(db_file_sizes, 0, (repeats + 1) * sizeof(int));

	/* Initialize the array holding each iteration's run time. */
	if ((time_secs =
	    (double *)malloc((repeats + 1) * sizeof(double))) == NULL) {
		fprintf(stderr,
		    "%s: Unable to allocate space for array time_secs.\n",
		    progname);
		goto err; 
	}
	memset(time_secs, 0, (repeats + 1) * sizeof(double));

	printf("\n\n======================================================");
	printf("\nAbout to enter the insert phase.");
	printf("\n\tDatabase type: %s \t",
	    dbtype == DB_HEAP ? "Heap" : "Btree");
	if (dbtype == DB_HEAP)
		printf("with configured heapsize = %d gbytes and %d bytes.",
		    ghpsize, hpsize);
	printf("\n\tPagesize: %d", dbp->pgsize);
	printf("\n\tInitial records number: %d", DEF_INIT_RECS);
	printf("\n\tNumber of repetitions: %d", repeats);
	printf("\n\tNumber of inserts per repetition: %d\n", recs_per_rep);

	/* 
	 * Insert records to the database and delete the same number from
	 * the database, then check the change of the physical database file.
	 * 
	 * Don't delete after the first insertion to leave some data
	 * in the tables for subsequent iterations.
	 */
	for (i = 0; i <= repeats; i++) {
		/* Get the start time. */
		(void)gettimeofday(&start_time, NULL);

		/* 
		 * Insert records into the database. Depending on the type
		 * of the database, different types of keys are used. Calling
		 * different functions to perform insertions.
		 */
		if ((dbtype == DB_HEAP) && (ret = insert_heap(dbp, dbenv,
		    i == 0 ? DEF_INIT_RECS : recs_per_rep,
		    i == 0 ? 0 : (DEF_INIT_RECS + (i - 1) * recs_per_rep),
		    test_var)) != 0) {
			dbenv->err(dbenv, ret,
			    "Failed to insert records to heap database.");
			goto err;
		}

		if ((dbtype == DB_BTREE) && (ret = insert_btree(dbp, dbenv,
		    i == 0 ? DEF_INIT_RECS : recs_per_rep,
		    i == 0 ? 0 : (DEF_INIT_RECS + (i - 1) * recs_per_rep),
		    test_var)) != 0) {
			dbenv->err(dbenv, ret,
			    "Failed to insert records to btree database.");
			goto err;
		}

		/* 
		 * Delete the same number of records except for the first
		 * iteration.
		 */
		if (i > 0 &&
		    (ret = delete_recs(dbp, dbenv, recs_per_rep)) != 0) {
			dbenv->err(dbenv, ret, "Failed to delete records.");
			goto err;
		}

		/* Get the end time. */
		(void)gettimeofday(&end_time, NULL);

		/* Calculate the runtime. */
		time_secs[i] =
		    (((double)end_time.tv_sec * NS_PER_MS + 
		    end_time.tv_usec) -
		    ((double)start_time.tv_sec * NS_PER_MS + 
		    start_time.tv_usec)) / NS_PER_MS;

		/* Calculate the physical file size. */
		if ((ret = file_size(dbp, dbtype, &fsize)) != 0) {
			dbenv->err(dbenv, ret, "Failed to calculate "
			    "the file size on repeat %d.\n", i);
			goto err;
		}
		db_file_sizes[i] = fsize;
	}
	printf("\n------------------------------------------------------\n");
	printf("%5s \t %10s \t %10s\n", "repetition", "physical file size",
	    "running time");
	for (i = 0; i <= repeats; i++)
		printf("%5d \t\t %10d \t\t %.2f seconds\n",
		    i, db_file_sizes[i], time_secs[i]);

err:
	if (db_file_sizes != NULL)
		free(db_file_sizes);
	if (time_secs != NULL)
		free(time_secs);

	return (ret);
}

/*
 * file_size --
 * 	Calculate the physical file size of the given database.
 *
 * Parameters:
 *	dbp	the database handle
 *	dbtype	the database type
 *	fsize	the physical file size of database
 */
int
file_size(dbp, dbtype, fsize)
	DB *dbp;
	DBTYPE dbtype;
	int *fsize;
{
	DB_ENV *dbenv;		/* The environment handle. */
	u_int32_t pgcnt;	/* The number of pages in the database. */
	u_int32_t pgsize;	/* The page size. */
	int ret;		/* The return value. */
	int size;		/* The file size. */
	void *statp;		/* The statistic pointer for database size. */

	dbenv = dbp->dbenv;
	pgsize = dbp->pgsize;
	ret = size = 0;

	/* Retrieve the database's statistics. */
	if ((ret = dbp->stat(dbp, NULL, &statp, DB_FAST_STAT)) != 0) {
		dbenv->err(dbenv, ret, "DB->stat");
		return (ret);
	}

	/* Get the page count. */
	pgcnt = (dbtype == DB_HEAP ? ((DB_HEAP_STAT *)statp)->heap_pagecnt :
	    ((DB_BTREE_STAT *)statp)->bt_pagecnt);

	/* the file size is the value of page count multiplied by the page size. */
	size = pgcnt * pgsize;
	*fsize = size;

	free(statp);

	return (ret);
}

/*
 * insert_heap --
 * 	Insert a specified number of records to a heap database,
 *  with keys beginning with a specified value.
 *
 * Parameters:
 *	dbp		the database handle
 *	dbenv		the database environment
 *	numrecs		the number of records to insert
 *	start		the start position to insert records
 *	test_var	test on variable-length data flag
 */

int
insert_heap(dbp, dbenv, numrecs, start, test_var)
	DB *dbp;
	DB_ENV *dbenv;
	int numrecs, start, test_var;
{
	DB_HEAP_RID rid;	/* The returned record id. */
	DBT key, data;		/* The key/data pair. */
	char buf[BUFFER_LEN];	/* The data buffer. */
	int cnt, ret;

	memset(&rid, 0, sizeof(DB_HEAP_RID));
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	ret = 0;

	/* For a heap database, the key must be an empty DB_HEAP_RID. */
	key.data = &rid;
	key.size = key.ulen = sizeof(DB_HEAP_RID);
	key.flags = DB_DBT_USERMEM;
	data.data = buf;
	data.flags = DB_DBT_USERMEM;

	/* 
	 * Insert a certain number of records into btree database,
	 * the data to insert is generated randomly.
	 */
	for (cnt = start; cnt < (numrecs + start) &&
	    (ret = generate_data(buf, cnt, test_var)) == 0; ++cnt) {
		data.size = data.ulen = (u_int32_t)strlen(buf) + 1;

		/* Require DB_APPEND flag to add new data to the database.*/
		if ((ret = dbp->put(dbp, NULL, &key, &data, DB_APPEND)) != 0) {
			dbenv->err(dbenv, ret, "insert_heap:DB->put");
			break;
		}
	}

	return (ret);
}

/*
 * insert_btree --
 * 	Insert a specified number of records to a btree database,
 *  with keys beginning with a specified value.
 *
 * Parameters:
 *	dbp		the database handle
 *	dbenv		the database environment
 *	numrecs		the number of records to insert
 *	start		the start position to insert records
 *	test_var	test on variable-length data flag
 */

int
insert_btree(dbp, dbenv, numrecs, start, test_var)
	DB *dbp;
	DB_ENV *dbenv;
	int numrecs, start, test_var;
{
	DBT key, data;
	char buf[BUFFER_LEN];
	int cnt, ret;

	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	ret = 0;

	key.data = &cnt;
	key.size = key.ulen = sizeof(int);
	key.flags = DB_DBT_USERMEM;
	data.data = buf;
	data.flags = DB_DBT_USERMEM;

	/* 
	 * Insert a certain number of records into btree database,
	 * the data to insert is generated by randomly.
	 */
	for (cnt = start; cnt < (numrecs + start) &&
	    (ret = generate_data(buf, cnt, test_var)) == 0; ++cnt) {
		data.size = data.ulen = (u_int32_t)strlen(buf) + 1;

		if ((ret = dbp->put(dbp, NULL, &key, &data, 0)) != 0) {
			dbenv->err(dbenv, ret, "insert_btree:DB->put");
			break;
		}
	}

	return (ret);
}

/*
 * generate_data --
 * 	Generate data for the specified record.
 *
 * Parameters:
 *	buf		the buffer variable to hold data
 *	rec_no		the record number
 *	test_var	test on variable-length data flag
 */

int
generate_data(buf, rec_no, test_var)
	char *buf;
	int rec_no, test_var;
{
	const char *str = "abcdefghijklmnopqrst";
	int len = (int)strlen(str);

	/*
	 * Default use the fix-length data,
	 * if required then use variable-length data.
	 */
	if (test_var == 1)
		len = rand() % (len - 2) + 1;

	(void)sprintf(buf, "%04d_%*s", rec_no, len, str); 

	return (0);
}

/*
 * delete_recs --
 * 	Delete a specified number of records.
 *
 * Parameters:
 *	dbp	the database handle
 *	dbenv	the database environment handle
 *	numrecs	number of records to delete
 */

int
delete_recs(dbp, dbenv, numrecs)
	DB *dbp;
	DB_ENV *dbenv;
	int numrecs;
{
	DBC *dbcp;
	DBT key, data;
	int cnt, ret;

	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));

	dbcp = NULL;
	cnt = ret = 0;

	/* Create a cursor to delete records. */
	if ((ret = dbp->cursor(dbp, NULL, &dbcp, 0)) != 0) {
		dbenv->err(dbenv, ret, "delete_recs:DB->cursor");
		goto err;
	}

	/* Delete the first numrecs records. */
	while ((ret = dbcp->get(dbcp, &key, &data, DB_NEXT)) == 0 &&
	    cnt < numrecs) {
		if ((ret = dbcp->del(dbcp, 0)) != 0) {
			dbenv->err(dbenv, ret, "delete_recs:DBCursor->del");
			break;
		} else
			++cnt;
	}

err:
	if (dbcp != NULL && (ret = dbcp->close(dbcp)) != 0)
		dbenv->err(dbenv, ret, "delete_recs:DBCursor->close");

	return (ret);
}

/*
 * usage --
 *	Describe this program's command line options, then exit.
 */
void
usage()
{
	fprintf(stderr, "usage: %s:\n%s \n %s\n", progname,
	    "\t[-b][-c cachesize][-d] -h home [-n recs_per_rep]",
	    "\t[-p pgsize][-r repeats][-S ghpsize][-s hpsize]");

	fprintf(stderr, "-b: run sample application using a btree database.\n");
	fprintf(stderr, "-c: specify the cache size for the environment.\n");
	fprintf(stderr, "-d: test on variable-length data "
	    "(default: fix-length).\n");
	fprintf(stderr, "-h: specify the home directory for "
	    "the environment (required).\n");
	fprintf(stderr, "-n: specify the num. of records "
	    "per repetition (default: %d).\n", DEF_RECS_PER_REP);
	fprintf(stderr, "-p: specify the pgsize of database.\n");
	fprintf(stderr, "-r: number of repetition (a pair of "
	    "insertion and deletion (default: %d)).\n", DEF_REPEATS);
	fprintf(stderr,
	    "-S: specify the heap size (gbytes) for the heap database.\n");
	fprintf(stderr,
	    "-s: specify the heap size (bytes) for the heap database.\n");

	exit(EXIT_FAILURE);
}

/*
 * open_env --
 * 	Open the environment before insert/delete operation
 *
 * Parameters:
 *	dbenvp		the database environment handle
 *	home		the home directory for the environment
 *	cachesize	the cache size for the environment
 */

int
open_env(dbenvp, home, cachesize)
	DB_ENV **dbenvp;
	char *home;
	u_int32_t cachesize;
{
	DB_ENV *dbenv;
	int ret = 0;

	/* Create an environment handle and open an environment. */
	if ((ret = db_env_create(&dbenv, 0)) != 0) {
		fprintf(stderr, "%s: db_env_create: %s\n",
		    progname, db_strerror(ret));
		return (ret);
	}

	*dbenvp = dbenv;

	/*
	 * Prefix any error messages with the name of this program and a ':'.
	 * Setting the errfile to stderr is not necessary, since that is the
	 * default; it is provided here as a placeholder showing where one
	 * could direct error messages to an application-specific log file.
	 */
	dbenv->set_errfile(dbenv, stderr);
	dbenv->set_errpfx(dbenv, progname);

	/* Set cachesize for the database environment. */
	if ((cachesize > 0) && (ret =
	    dbenv->set_cachesize(dbenv, (u_int32_t)0, cachesize, 1)) != 0) {
		dbenv->err(dbenv, ret, "DB_ENV->set_cachesize");
		return (ret);
	}

	if ((ret = dbenv->open(dbenv, home, DB_CREATE | DB_INIT_MPOOL, 0)) != 0)
		dbenv->err(dbenv, ret, "DB_ENV->open");

	return (ret);
}

/*
 * open_db --
 * 	Open the database before insert/delete operation.
 *
 * Parameters:
 *	dbpp	the database handle
 *	dbenv	the database environment
 *	dbtype	the database type
 *	home	the home directory for the environment
 *	gphsize	the heap size (gbytes) for the heap database
 *	hpsize	the heap size (bytes) for the heap database
 *	pgsize	the page size for the heap database
 */
int
open_db(dbpp, dbenv, dbtype, home, ghpsize, hpsize, pgsize)
	DB **dbpp;
	DB_ENV *dbenv;
	DBTYPE dbtype;
	char *home;
	u_int32_t ghpsize, hpsize, pgsize;
{
	DB *dbp;
	u_int32_t dbflags = 0;
	char *dbname;
	int ret = 0;

	dbname = (dbtype == DB_HEAP) ? "heap.db" : "btree.db";

	/* Create a database handle and open a database. */
	if ((ret = db_create(&dbp, dbenv, 0)) != 0) {
		dbenv->err(dbenv, ret, "db_create : %s", dbname);
		goto err;
	}

	*dbpp = dbp;

	/* Set the record compare function for the btree database. */
	if ((dbtype == DB_BTREE) &&
	    (ret = dbp->set_bt_compare(dbp, compare_int)) != 0) {
		dbp->err(dbp, ret, "DB->set_bt_compare");
		goto err;
	}

	/* Set heap size for the heap database. */
	if ((dbtype == DB_HEAP) && (ghpsize > 0 || hpsize > 0) &&
	    (ret = dbp->set_heapsize(dbp, ghpsize, hpsize, 0)) != 0) {
		dbenv->err(dbenv, ret, "DB->set_heapsize");
		return (ret);
	}
	/* Set page size for the database. */
	if ((pgsize > 0) && (ret = dbp->set_pagesize(dbp, pgsize)) != 0) {
		dbenv->err(dbenv, ret, "DB->set_pagesize");
		return (ret);
	}

	if ((ret =
	    dbp->open(dbp, NULL, dbname, NULL, dbtype, DB_CREATE, 0)) != 0)
		dbenv->err(dbenv, ret, "DB->open");
err:

	return (ret);
}

/*
 * compare_int --
 * 	Compare two data value for btree database.
 *
 * Parameters:
 *	dbp	the database handle
 *	a	one of the data value use to compare
 *	b	another data value use to compare
 *	locp	lock handle
 */
int
compare_int(dbp, a, b, locp)
	DB *dbp;
	const DBT *a, *b;
	size_t *locp;
{
	int ai, bi;

	dbp = NULL;
	locp = NULL;

	/*
	 * Returns:
	 *	< 0 if a < b
	 *	= 0 if a = b
	 *	> 0 if a > b
	 */
	memcpy(&ai, a->data, sizeof(int));
	memcpy(&bi, b->data, sizeof(int));
	return (ai - bi);
}
