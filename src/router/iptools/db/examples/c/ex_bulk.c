/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2001, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * ex_bulk -- 
 *   This sample program shows how the DB_MULTIPLE_*() macros allow you to
 *   retrieve or update multiple records within single calls of DBC->get(),
 *   DB->put(), and DB->del(). 
 *   It also shows how to:
 *	 a) override the default btree key comparison function, so that records
 *		are ordered as natural integers, rather than byte streams,
 *	 b) store two databases in a single Berkeley DB file, and
 *	 c) construct a secondary "index" database for quick record access via a 
 *		component of the data field.
 *
 * Source File:
 *	ex_bulk.c
 *
 * Environment directory:
 *	EX_BULK_DIR: configured as a Transactional Data Store
 *
 * Database file:
 *	ex_bulk.db (with multiple sub-database support)
 *	
 *	subdbs:	primary:
 *		    key:  an 'int'-sized integer, which starts at 0 and is
 *			  incremented for each successive record.
 *			  Range: 0 to the number of records added (option: -nN)
 *		    data: a struct bulk_record: containing an id and a string.
 *
 *		secondary:	(only if the -S option was specified)
 *		    key:  a copy of the first byte of primary.data's string.
 *			  Since that field contains only the characters found
 *			  in DataString[], the secondary's key is always one of
 *			  the characters of DataString[]. This allows the
 *			  example to select a random character from DataString[]
 *			  as a key to use when scanning through the secondary
 *			  index.
 *			  Range: 0..9a..f
 *		    data: a copy of primary.key, as always is the case for a
 * 			  secondary index; it points to a primary record.
 *	
 *   This program has three main modes of operation:
 *
 *   Insert		(this is the default mode)
 *		Erase and create the environment and main database;
 *		Add key/data pairs (as described above) with bulk puts.
 *	 	If the -S option is specified then it also creates a secondary
 *		index, using get_first_str() to create 1-byte key.
 *
 *   Read		(command line option: -R)
 *		Read batches of records from the primary database. By running
 *		the program first in `insert' mode, and then this mode you can
 *		see how fast simple bulk retrievals can be. Read mode ignores
 *		the secondary database, if present.
 *
 *   Delete		(command line option: -D)
 *	 	This mode performs  all the steps of Insert mode, and then does
 *		bulk deletes of a randomly selected subset of the records, those
 *		with keys less than or equal to a certain value. Without the -S
 *		the maximum key is between 0 and the number of records inserted.
 *		With -S, the maximum key is selected from DataString[].
 *
 *   Each phase displays statistics of its elapsed time and throughput,
 *   in records and bulk batches per second.
 *
 * Options:							default value
 *	-bN  Set the number of records for each bulk batch	[100]
 *	-cN  Set the Berkeley DB mpool cache size		[1000*pagesize]
 *	-dN  Number of values for each key in the primary	[1] 
 *	-iN  Number of bulk iterations for Read and Delete Mode	[1000000] 
 *	-nN  Number of keys to insert, or the highest key to  	[1000000]
 *	     read or delete
 *	-pN  A power-of-two pagesize from 512 to 65536		[65536]
 *	-v   Print each value stored into or obtained from 	[not set]
 *	     the DB_MULTIPLE* DBTs.
 *	-D   Bulk delete mode: first insert -nN records, then	[not set]
 *	     delete a random subset of them
 *	-R   Perform bulk read: perform -iN bulk get requests 	[not set]
 *	     starting from a random key from 0 to -nN
 *	-S   Associate a secondary 'index' database with the	[not set]
 *	     primary. This association connects the databases together, so
 *	     that an insert into the primary also constructs and inserts a
 *	     record into the secondary; deleting a record from one deletes
 *	     the corresponding record of the other.
 *		When -S is set, the -dN option may not be specified.
 *	     This is a requirement of secondary databases, which store a copy
 *	     of the key of the primary as a logical pointer to the primary's
 *	     records. If the primary allowed duplicates then there would no 
 *	     longer be a one-to-one correspondence between records in the
 *	     primary and the secondary.
 *
 * $Id$
 */

#include <sys/types.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define	BULKDIR			"EX_BULK_DIR"	/* Environment directory name */
#define	DATABASE_FILE		"ex_bulk.db"	/* Database file name */
#define	DB_MAXIMUM_PAGESIZE	(64 * 1024)	/* Maximum database page size */
#define	PRIMARY_NAME		"primary"	/* Primary sub-database */
#define	SECONDARY_NAME		"secondary"	/* Secondary sub-database */

#define	DATALEN		20			/* The primary value's length */
#define	STRINGLEN	(DATALEN - sizeof(int))	/* The length of its string */

/*
 * The bulk_record structure descibes the value stored in the primary database.
 *
 * The 'id' indicates which 'duplicate' this record is. The first value of each
 * key has an id of 0. If duplicates are specified with the -dN option, there
 * will be N additional records; each successive one with a incremented id.
 * The 'str' value is derived from the characters in DataString[], below..
 *
 * For example, ex_bulk without any options generates:
 *	key	value
 *	0	{ 0, 0123456789abcdef }
 *	1	{ 0, 123456789abcdef0 }
 *	2, 	{ 0, 23456789abcdef01 }
 *	...
 *	15, 	{ 0, f0123456789abcde }
 *	16, 	{ 0, 0123456789abcdef }
 *	17, 	{ 0, 123456789abcdef0 }
 *	...
 *
 * ex_bulk -S (creating a secondary 'index' database) creates:
 *	primary db				secondary db
 *	key	value				key	value
 *	0	{ 0, 0123456789abcdef }		'0'	0
 *	1	{ 0, 123456789abcdef0 }		'0'	0
 *	2, 	{ 0, 23456789abcdef01 }		'0'	0
 *	3, 	{ 0, 3456789abcdef012 }		'0'	0
 *
 * ex_bulk -d2 generates these records:
 *	key	value
 *	0	{ 0, 0123456789abcdef }
 *	0	{ 1, 0123456789abcdef }
 *	0	{ 2, 0123456789abcdef }
 *	1	{ 0, 123456789abcdef0 }
 *	1	{ 1, 123456789abcdef0 }
 *	1	{ 2, 123456789abcdef0 }
 *	2	{ 0, 23456789abcdef01 }
 *	...
 *	
 */
typedef struct bulk_record {
	int	id;
	char	str[STRINGLEN];
} bulk_record_t;

/* The values put into the key's string value are constructed from this. */
const char	DataString[] = "0123456789abcdef";


#ifdef _WIN32
#include <sys/timeb.h>
#include <winsock2.h>
extern int getopt(int, char * const *, const char *);

/*
 * Timer support --
 * This implements a POSIX interface to Windows' millisecond resolution timers.
 */
int
gettimeofday(struct timeval *tv, struct timezone *tz)
{
	struct _timeb now;
	_ftime(&now);
	tv->tv_sec = (long)now.time;
	tv->tv_usec = now.millitm * 1000;
	return (0);
}

/*
 * Remove any environment directory remaining from a prior execution of this
 * example program: the Windows version.
 */
#define CLEANUP_CMD "rmdir " BULKDIR " /q/s"

#else

#include <sys/time.h>
#include <unistd.h>

/* Remove any environment directory remaining from a prior execution of this
 * example program: the POSIX/Solaris/Linux/Unix version.
 */
#define CLEANUP_CMD "rm -rf " BULKDIR

#endif

#include <db.h>

int	bulk_dbt_init(DBT *, int);
int	bulk_delete(DB_ENV *, DB *, int, int, int, long long *, int *, int);
int	bulk_delete_sec(DB_ENV *, DB *, int, int, int, long long *, int *, int);
int	bulk_insert(DB_ENV *, DB *, int, int, int, long long *, int *, int);
int	bulk_get(DB_ENV *, DB *, int, int, int, int, long long *, int);
int	compare_int(DB *, const DBT *, const DBT *, size_t *);
int	db_init(DB_ENV *, DB **, DB**, int, int, int);
DB_ENV	*env_init(const char *, const char *, u_int);
int	get_first_str(DB *, const DBT *, const DBT *, DBT *);
int	rotate_string(const char *, char *, int);
void	timer_start(struct timeval *start);
double	timer_end(const struct timeval *start);
int	main(int, char *[]);
void	usage(void);

/* This program name is used in error messages. */
const char	*Progname = "ex_bulk";


/*
 * timer_start --
 *	Remember the start of an interval, e.g., the beginning of a major
 *	loop of bulk operations. At the end of the interval, use timer_end().
 */
void
timer_start(struct timeval *timer)
{
	(void)gettimeofday(timer, NULL);
}

/*
 * timer_end --
 *	Return the length of an interval whose start was remembered by
 *	calling timer_start().
 *
 *	Returns:
 *		The elapsed time, in floating point seconds.
 *		It never returns 0, even when the interval was smaller
 *		than the precision of the timer, which is easily possible
 *		on systems with millisecond resolution. This means that the
 *		caller does not need to guard against divide by zero errors,
 *		as when computing the average number of records per second.
 */
double
timer_end(const struct timeval *start)
{
	struct timeval now;
	double elapsed;

	(void)gettimeofday(&now, NULL);
	elapsed = (now.tv_sec - start->tv_sec) +			
	    ((double)now.tv_usec - start->tv_usec) / 1000000;
	/* Return a minimum duration of 1 microsecond. */
	if (elapsed <= 0.0)
		elapsed = 0.000001;
	return (elapsed);
}


/*
 * main --
 *	The main function decides which mode to run, opens the environment and
 *	database(s), runs the requested modes while collecting execution
 *	statistics, prints the resulting times, and cleanly closes down.
 */
int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;	/* From getopt(): the current option's value */
	extern int optind;	/* From getopt(): the # of argv's processed */
	DB *dbp;		/* Handle of the main database to be updated */
	DB *secdbp;		/* Handle of secondary database, if -S */
	DB_ENV *dbenv;		/* Handle of the database environment */
	struct timeval start;	/* Stores the time when an interval started */
	double duration;	/* Length of the current interval, in seconds */
	u_int cache;		/* -c<N> option: environment cachesize */
	u_int pagesize;		/* -p<N> option: database pagesize */
	int batchsize;		/* -b<N> option: #items per bulk get/put/del */
	int bulkgets;		/* -i<N> option: the number of bulk get calls
				   to make during read mode. */
	int bulkchanges;	/* the number of bulk put or delete calls
				   needed by insert or delete mode */
	int ch;			/* The current command line option char */
	int deletemode;		/* -D option: if set, perform bulk deletes */
	int dups;		/* -d<N> option: if set, keys have > 1 value */
	int numkeys;		/* -n<N> option: the number of records to
				   insert, or the highest one to get|delete */
	int deletepairs;	/* For the -DS case: whether the bulk delete
				   through the secondary database shall specify
				   keys or key+data pairs in its DBT. */
	int readmode;		/* -R option: Perform reads when set. */
	long long resultcount;	/* The number of records processed */
	int ret;		/* Return code from the main API calls */
	int ret2;		/* This second return code avoids ovwritting
				   the main return code 'ret' when closing
				   down, so that the first error seen is
				   always propagated to the exit status. */ 
	int secondaryflag;	/* -S option: maintain a secondory index. */
	int verbose;		/* -v option: Print each key or key/data pair */
	char *desc;		/* For delete mode: whether the item counts are
				    for key groups or records. */

	/*
	 * Initialize the environment and database handles so that the cleanup 
	 * code can determine whether they need to be closed.
	 */
	dbenv = NULL;
	dbp = secdbp = NULL;

	/* Set the defaults for the command line options. */
	bulkgets = 1000000;
	numkeys = 1000000;
	batchsize = 100;
	deletemode = dups = readmode = secondaryflag = verbose = 0;
	pagesize = 65536;
	cache = 1000 * pagesize;

	while ((ch = getopt(argc, argv, "b:c:d:i:n:p:vDRS")) != EOF)
		switch (ch) {
		case 'b':
			if ((batchsize = atoi(optarg)) <= 0)
				usage();
			break;
		case 'c':
			if ((cache = (u_int)atoi(optarg)) <= 0)
				usage();
			break;
		case 'd':
			if ((dups = atoi(optarg)) <= 0)
				usage();
			break;
		case 'i':
			if ((bulkgets = atoi(optarg)) <= 0)
				usage();
			break;
		case 'n':
			if ((numkeys = atoi(optarg)) <= 0)
				usage();
			break;
		case 'p':
			if ((pagesize = (u_int)atoi(optarg)) < 512)
				usage();
			break;
		case 'v':
			verbose = 1;
			break;
		case 'D':
			deletemode = 1;
			break;
		case 'R':
			readmode = 1;
			break;
		case 'S':
			secondaryflag = 1;
			break;
		default:
			usage();
		}

	if (secondaryflag && dups) {
		fprintf(stderr,
		    "%s: the -S and -d%d options are not compatible:\n\t"
		    "Primary databases must be unique.\n", Progname, dups);
		exit(1);
	}

	/* Read mode uses the existing environment and database(s); the
	 * modification modes remove any existing files.
	 */
	if (!readmode) {
		system(CLEANUP_CMD);
		system("mkdir " BULKDIR);
	}

	/* Create the environment handle, configure, and open the it. */
	if ((dbenv = env_init(BULKDIR, Progname, cache)) == NULL)
		return (-1);

	/* Create the database handle(s), configure and open them. */
	if ((ret = db_init(dbenv,
	    &dbp, &secdbp, dups, secondaryflag, pagesize)) != 0)
		return (1);

	/* The read and delete modes use random numbers; initialize it. */
	srand((int)time(NULL));

	if (readmode) {
		timer_start(&start);
		/*
		 * Repeat getting one bulk batch of records from a randomly
		 * selected key. Set resultcount to the number of records fetched.
		 */
		if ((ret = bulk_get(dbenv, dbp, numkeys,
		    dups, batchsize, bulkgets, &resultcount, verbose)) != 0)
			goto cleanup;
		duration = timer_end(&start);
		printf(
		    "[STAT] Read %lld records using %d batches in %.2f seconds"
		    ": %.0f records/sec\n",
		    resultcount, bulkgets, duration,
		    resultcount / duration);
	} else {
		timer_start(&start);
		/*
		 * Insert 'numkeys' records, with multiple values for each
		 * key if 'dups' is set. The resultcount will be the number of
		 * records inserted, unless an error occurs. The number of bulk
		 * puts that were required is returned in 'bulkchanges'.
		 */
		if ((ret = bulk_insert(dbenv, dbp, numkeys, dups, batchsize,
		     &resultcount, &bulkchanges, verbose)) != 0)
			goto cleanup;
		duration = timer_end(&start);
		printf("[STAT] Insert %lld records using %d batches",
		    resultcount, bulkchanges);
		printf(" in %.2f seconds: ", duration);
		printf("%.0f records/second\n", resultcount / duration);

		if (deletemode) {
			if (secondaryflag) {
				/*
				 * Delete some of the data 'through' the
				 * secondary database. Randomly set deletepairs
				 * to delete either individual records specified
				 * by key-value pairs, or sets of records
				 * sharing the same key.
				 */
				deletepairs = rand() % 2;
				timer_start(&start);
				if ((ret = bulk_delete_sec(dbenv,
				    secdbp, numkeys, deletepairs, batchsize,
				    &resultcount, &bulkchanges, verbose)) != 0)
					goto cleanup;
				duration = timer_end(&start);
				desc = deletepairs ? "records" : "key groups";
				printf(
				    "[STAT] Delete %lld %s using %d batches "
				    "in %.2f seconds: %.0f %s/second\n",
				    resultcount, desc, bulkchanges, duration,
				    resultcount / duration, desc);
			} else {
				/*
				 * Delete a randomly selected subset of records
				 * from the primary database.
				 */
				timer_start(&start);
				if ((ret = bulk_delete(dbenv,
				    dbp, numkeys, dups, batchsize,
				    &resultcount, &bulkchanges, verbose)) != 0)
					goto cleanup;
				duration = timer_end(&start);
				desc = dups ? "records" : "key groups";
				printf(
				    "[STAT] Delete %lld %s using %d batches "
				    "in %.2f seconds: %.0f %s/second\n",  
				    resultcount, desc, bulkchanges, duration,
				    resultcount / duration, desc);
			}
		} 
	}

	/* Close any non-null handles: they have been opened. */
cleanup:
	/*
	 * First check whether the secondary database needs to be closed, then
	 * the primary, lastly the environment. 
	 * If this was in read mode, then there's no need to flush the database
	 * to stable storage, hence the DB_NOSYNC; it saves just a little
	 * time, avoiding an unecessary scan over the Berkeley DB cache.
	 */
	if (secdbp != NULL &&
	    (ret2 = secdbp->close(secdbp, readmode ? DB_NOSYNC : 0)) != 0) {
		dbenv->err(dbenv, ret, "DB(secondary)->close");
		/*
		 * Remember that there was a failure, but don't overwrite any
		 * prior error.
		 */
		if (ret == 0)
			ret = ret2;
	}

	if (dbp != NULL &&
	    (ret2 = dbp->close(dbp, readmode ? DB_NOSYNC : 0)) != 0) {
		dbenv->err(dbenv, ret, " DB(primary)->close");
		if (ret == 0)
			ret = ret2;
	}
	if (dbenv != NULL &&
	    (ret2 = dbenv->close(dbenv, 0)) != 0) {
		fprintf(stderr, "DB_ENV->close: %s", db_strerror(ret2));
		if (ret == 0)
			ret = ret2;
	}

	/* Return 0 from main if all succeeded; otherwise return 1. */
	return (ret == 0 ? 0 : 1);
}

/*
 * bulk_dbt_init --
 *	Initialize a bulk dbt suitable for a number of key/data pairs.
 *	The bulk buffer size is calculated to be more than enough to
 *	handle the largest bulk request that this program generates.
 *
 *	The buffer must be a multiple of 1024 bytes, at least as large as the
 *	page size of the underlying database, and aligned for unsigned integer
 *	(u_int32_t) access.
 *
 */
int
bulk_dbt_init(bulk, itemcount)
	DBT *bulk;
	int itemcount;
{
	memset(bulk, 0, sizeof(DBT));
	/*
	 * Allow each key/value pair to use twice the size of the data to be
	 * inserted. This gives space for the bookkeeping fields required by
	 * DB_MULTIPLE_WRITE() and DB_MULTIPLE_WRITE(). A simple one megabyte
	 * buffer is suitable for most applications.
	 */
	bulk->ulen = (u_int32_t)itemcount * 2 * (sizeof(u_int32_t) + DATALEN);
	/* Round the size up to be a multiple of 1024. */
	bulk->ulen += 1024 - (bulk->ulen % 1024);
	/*
	 * In order to use a bulk DBT for get() calls, it must be at least as
	 * large as the database's pages. Make sure that it as least 64KB, the
	 * maximum database page size.
	 */
	if (bulk->ulen < DB_MAXIMUM_PAGESIZE)
		bulk->ulen = DB_MAXIMUM_PAGESIZE;
	bulk->flags = DB_DBT_USERMEM | DB_DBT_BULK;
	if ((bulk->data = malloc(bulk->ulen)) == NULL) {
		printf("bulk_dbt_init: malloc(%u) failed: %s",
			(unsigned)bulk->ulen, strerror(errno));
		return (errno);
	}
	memset(bulk->data, 0, bulk->ulen);
	return (0);
}

/*
 * bulk_insert --
 *	Insert numkeys * (dups + 1) records into the database, using bulk DBTs.
 *	If the secondary database has been created and associated, automatically
 *	insert the corresponding records into the secondary.
 */
int
bulk_insert(dbenv, dbp, numkeys, dups, batchsize, itemcountp, batchcountp, verbose)
	DB_ENV *dbenv;		/* Environment handle */
	DB *dbp;		/* Database handle open to the secondary db. */
	int numkeys;		/* The number of distinct keys. */
	int dups;		/* The number of additional records per key. */
	int batchsize;		/* Store this many items in the bulk DBT. */
	long long *itemcountp;	/* out: the number of items added to the DBT. */
	int *batchcountp;	/* out: the number of DB->put() calls done. */
	int verbose;		/* If set, print each key to be deleted. */
{
	DBT key;		/* The bulk key parameter to DB->put(). */
	DBT data;		/* The bulk data parameter to DB->put(). */
	u_int32_t flag;		/* For DB->put: DB_MULTIPLE | DB_MULTIPLE_KEY */
	DB_TXN *txnp;
	bulk_record_t data_val;	/* The data value being put into a bulk dbt. */
	int batchcount;		/* The number of batches performed. */
	int pendingitems;	/* The number of items in the current batch. */
	int totalitems;		/* The total number of items put into batches. */
	int prikey;		/* The current primary key: 0..numkeys-1. */
	int ret;		/* Berkeley DB API return code. */
	void *poskey;		/* The current position in key bulk DBT. */
	void *posdata;		/* The current position in data bulk DBT. */

	txnp = NULL;
	batchcount = pendingitems = totalitems = ret = 0;
	poskey = posdata = NULL;
	memset(&data_val, 0, DATALEN);

	/* Initialize the bulk dbts to support at least batchsize items. */
	if ((ret = bulk_dbt_init(&key, batchsize)) != 0)
		return (ret);
	if ((ret = bulk_dbt_init(&data, batchsize)) != 0) {
		free(&key.data);
		return (ret);
	}

	/*
	 * Bulk insert with either DB_MULTIPLE in two buffers or
	 * DB_MULTIPLE_KEY in a single buffer. With DB_MULTIPLE, all keys are
	 * constructed in the key DBT, and all data is constructed in the data
	 * DBT. With DB_MULTIPLE_KEY, all key/data pairs are constructed in the
	 * key Dbt. We use DB_MULTIPLE mode when there are duplicate records.
	 */
	flag = dups ? DB_MULTIPLE : DB_MULTIPLE_KEY;
	DB_MULTIPLE_WRITE_INIT(poskey, &key);
	if (dups)
		DB_MULTIPLE_WRITE_INIT(posdata, &data);
	for (prikey = 0; prikey < numkeys; prikey++) {
		/*
		 * Add one key,value pair to the bulk DBT for each record to
		 * insert.
		 * The id is incremented for each duplicate. The str value doesn't change.
		 */
		data_val.id = 0;
		rotate_string(DataString, data_val.str, prikey);
		do {
			/*
			 * 
			 */
			if (dups) {
				DB_MULTIPLE_WRITE_NEXT(poskey, &key, 
				    &prikey, sizeof(prikey));
				assert(poskey != NULL);
				DB_MULTIPLE_WRITE_NEXT(posdata, &data, 
				    &data_val, DATALEN);
				assert(posdata != NULL);
			} else {
				DB_MULTIPLE_KEY_WRITE_NEXT(poskey, &key,
				    &prikey, sizeof(prikey), &data_val, DATALEN);
				assert(poskey != NULL);
			}
			/* Keep track of the number of items in the bulk DBT. */
			pendingitems++;
			if (verbose)
				printf("Insert key: %d, \t"
				    "data: (id %d, str %.*s)\n", prikey,
				    data_val.id, DATALEN, data_val.str);
		} while (data_val.id++ < dups);

		/*
		 * When the desired batch size has been reached (or slightly
		 * exceeded, by the duplicate count), then perform a
		 * transaction-protected DB->put().
		 */
		if (pendingitems >= batchsize) {
			if ((ret = 
			    dbenv->txn_begin(dbenv, NULL, &txnp, 0)) != 0) {
				dbenv->err(dbenv,
				    ret, "bulk_insert: txn_begin");
				goto err_cleanup;
			}

			/* Insert the batch. */
			if ((ret =
			    dbp->put(dbp, txnp, &key, &data, flag)) != 0) {
				dbp->err(dbp, ret, "Bulk DB->put");
				goto err_cleanup;
			} 

			/* Update statistics: batches, records. */
			batchcount++;
			totalitems += pendingitems;

			/* Prepare for the next bulk batch. */
			pendingitems = 0;
			DB_MULTIPLE_WRITE_INIT(poskey, &key);
			if (dups)
				DB_MULTIPLE_WRITE_INIT(posdata, &data);

			/* Commit the inserted batch of records. */
			ret = txnp->commit(txnp, 0);
			/*
			 * The transaction handle must not be referenced after
			 * a commit (or abort); null out the handle before
			 * checking the return code.
			 */
			txnp = NULL;
			if (ret != 0) {
				dbenv->err(dbenv, ret, "bulk_insert: commit");
				goto err_cleanup;
			}
		}
	} 

	/*
	 * Insert any remaining items stored in the bulk DBT which haven't
	 * been inserted.
	 */
	if (pendingitems != 0) {
		/* As above: begin transaction, put, commit transaction. */
		if ((ret = dbenv->txn_begin(dbenv, NULL, &txnp, 0)) != 0) {
			dbenv->err(dbenv, ret, "bulk_insert: final txn_begin");
			goto err_cleanup;
		}
		if ((ret = dbp->put(dbp, txnp, &key, &data, flag)) != 0) {
			dbp->err(dbp, ret, "Bulk DB->put");
			goto err_cleanup;
		}
		batchcount++;
		totalitems += pendingitems;
		ret = txnp->commit(txnp, 0);
		txnp = NULL;
		if (ret != 0) {
			dbenv->err(dbenv, ret, "bulk_insert: final commit");
			goto err_cleanup;
		}
	}

	/*
	 * Tell the caller how many records were inserted, and the number of
	 * bulk API calls were needed.
	 */
	*itemcountp = totalitems;
	*batchcountp = batchcount;

	/* Clean up any locally acquired resources. */
err_cleanup:
	if (txnp != NULL)
		(void)txnp->abort(txnp);
	free(key.data);
	free(data.data);
	return (ret);
}

/*
 * bulk_delete --
 *	Bulk delete a subset of records from the primary database, starting at
 *	the first one, and ending at a randomly selected key.
 */
int
bulk_delete(dbenv, dbp, numkeys, dups, batchsize, itemcountp, batchcountp, verbose)
	DB_ENV *dbenv;		/* Environment handle */
	DB *dbp;		/* Database handle open to the secondary db. */
	int numkeys;		/* The number of distinct keys. */
	int dups;		/* The number of additional records per key. */
	int batchsize;		/* Approximate number of items per DB->del() */
	long long *itemcountp;	/* out: the number of items added to the DBT. */
	int *batchcountp;	/* out: the number of DB->del() calls done. */
	int verbose;		/* If set, print each key to be deleted. */
{
	DBT bulk;		/* DBT containing keys or key/value pairs */ 
	bulk_record_t value;	/* This is the value of key/value pairs. */
	u_int32_t flag;		/* Flags to DB->del(): DB_MULITPLE[_KEY] */
	int batchcount;		/* The number of (bulk) DB->del() calls made. */
	int pendingitems;	/* The number of items written into the DBT, 
				   but not yet passwd to DB->del(). */
	int totalitems;		/* Total number of items written into the DBT. */
	int prikey;		/* The primary database's key. */
	int highest;		/* The highest, or only, key value to delete. */
	int ret;		/* Berkeley DB API return code. */
	void *writepos;		/* Location of the next write into bulk. */

	/* Suppress "warning: <var> not used" messages in many compilers. */
	dbenv = NULL;
	batchcount = pendingitems = totalitems = ret = 0;
	memset(&value, 0, DATALEN);

	/* Select the highest key to be deleted. */
	highest = rand() % numkeys;
	printf("Bulk delete of %s <= %d\n",
		dups ? "records" : "keys", highest);

	/* Initialize the bulk dbt to support at least batchsize items. */
	bulk_dbt_init(&bulk, batchsize);

	/*
	 * Prepare the bulk DBT for DB_MULTIPLE_WRITE_NEXT() or
	 * DB_MULTIPLE_KEY_WRITE_NEXT(); the initialization is the same.
	 */
	DB_MULTIPLE_WRITE_INIT(writepos, &bulk);

	/*
	 * If there are no duplicate data values, then we can completely
	 * specify records by the (unique) key, and so use the single-value
	 * version DB_MULTIPLE_WRITE_NEXT(). If there are duplicates we use the
	 * dual-value version, which specify the exact key+value pairs that we
	 * expect to find, since they were just inserted by bulk_insert().
	 */
	flag = dups ? DB_MULTIPLE_KEY : DB_MULTIPLE;
	for (prikey = 0; prikey < highest; prikey++) {
		if (dups) {
			/*
			 * Add one key,value pair to the bulk DBT for each
			 * record which was inserted. The id is incremented for
			 * each duplicate. The str value doesn't change.
			 */
			rotate_string(DataString, value.str, prikey);
			for (value.id = 0; value.id <= dups; value.id++) {
				DB_MULTIPLE_KEY_WRITE_NEXT(writepos, &bulk,
				    &prikey, sizeof(prikey), &value, DATALEN);
				if (writepos == NULL) {
					dbp->errx(dbp,
					    "bulk_delete: Duplicate DBT "
					    "overflow: key %d duplicate #%d",
					    prikey, value.id);
					ret = ENOMEM;
					goto err;
				}
				pendingitems++;
				if (verbose)
					printf("Delete key: %d, \t"
					    "data: (id %d, str %.*s)\n", prikey,
					    value.id, DATALEN, value.str);
			}
		} else {
			/*
			 * The non-dupicates case. Each addition to the bulk dbt
			 * is a single item: the next (incremented) primary key.
			 */
			DB_MULTIPLE_WRITE_NEXT(writepos,
			    &bulk, &prikey, sizeof(prikey));
			if (writepos == NULL) {
				dbp->errx(dbp,
				    "bulk_delete: DBT overflow @ key %d", prikey);
				ret = ENOMEM;
				goto err;
			}
			pendingitems++;
			if (verbose)
				printf("Delete key: %d\n", prikey);
		}

		/*
		 * If we have specified enough records in the bulk DBT, now
		 * delete them, update the delete statistics, and reset the
		 * bulk DBT for the next batch.
		 */
		if (pendingitems >= batchsize) {
			switch (ret = dbp->del(dbp, NULL, &bulk, flag)) {
			case 0:
				batchcount++;
				totalitems += pendingitems;
				pendingitems = 0;
				DB_MULTIPLE_WRITE_INIT(writepos, &bulk);
				break;
			default:
				dbp->err(dbp, ret, "delete");
				goto err;
			}
		}
	}

	/* Finish any accumulated items that have not been deleted. */
	if (pendingitems != 0) {
		switch (ret = dbp->del(dbp, NULL, &bulk, flag)) {
		case 0:
			batchcount++;
			totalitems += pendingitems;
			break;
		default:
			dbp->err(dbp, ret, "final delete");
			goto err;
		}
	}

	*itemcountp = totalitems;
	*batchcountp = batchcount;

err:
	free(bulk.data);
	return (ret);
}

/*
 * bulk_delete_sec --
 *	Delete records from both databases, accessing them through the
 *	secondary database.
 *
 *	The range of records to delete extends from the lowest key up through
 *	one of randomly selected char source for the secondary
 *	database's key: DataString[].
 *
 *	The 'pairs' parameter *only* determines *how* those records are
 *	specified, whether by a small number of keys or a much larger number
 *	of key/value pairs. In this example the same records are deleted in
 *	either case, since the key/value case takes care to specify all values
 *	which were inserted by bulk_insert().
 */
int
bulk_delete_sec(dbenv, dbp,
    numkeys, pairs, batchsize, itemcountp, batchcountp, verbose)
	DB_ENV *dbenv;		/* Environment handle */
	DB *dbp;		/* Database handle open to the secondary db. */
	int numkeys;		/* The number of distinct keys. */
	int pairs;		/* Delete by keys, or key+data pairs? */
	int batchsize;		/* Approximate number of items per DB->del() */
	long long *itemcountp;	/* out: the number of items added to the DBT. */
	int *batchcountp;	/* out: the number of DB->del() calls done. */
	int verbose;		/* If set, print each key to be deleted. */
{
	DBT bulk;		/* The bulk DBT. */
	DB_TXN *txnp;		/* Each DB->del() uses its own transaction. */
	u_int32_t flag;		/* DB->del() param: which kind of bulk DBT. */
	int deletes;		/* Counts the number of DB->del() calls. */
	int highest;		/* The highest secondary key to delete */
	int i;			/* loop iterator scanning over secondary keys. */
	int totalitems;		/* Count of keys or key/value pairs. */
	int j;			/* loop iterator for computing primary key */
	int pendingitems;	/* The number of items written into the DBT, 
				   but not yet passwd to DB->del(). */
	int prikey;		/* Add this primary key to bulk, if pairs on. */
	int ret;		/* Berkeley DB API return code. */
	void *writepos;		/* Location of the next write into bulk. */
	char seckey;		/* The secondary key to add to the bulk DBT.*/

	/*
	 * Initialize variables: there is no transaction yet, no keys or
	 * key/value pairs added, no deletes done, no error yet.
	 */
	txnp = NULL;
	deletes = pendingitems = totalitems = ret = 0;

	/*
	 * Initialize the get flags to match the style of DBT that will be
	 * constructed: DB_MULTIPLE_KEY_WRITE_NEXT() if pairs is set; or
	 * DB_MULTIPLE_WRITE_NEXT() if 
	 */
	flag = pairs ? DB_MULTIPLE_KEY : DB_MULTIPLE;

	/* Select the highest key to delete at random. */
	highest = rand() % STRINGLEN;
	printf("Bulk delete through secondary of %s <= '%c'\n",
		pairs ? "records" : "keys", DataString[highest]);

	/* Initialize the bulk dbt to support at least batchsize items. */
	bulk_dbt_init(&bulk, batchsize);
	DB_MULTIPLE_WRITE_INIT(writepos, &bulk);

	/*
	 * Bulk delete all records of a specific set of keys which includes all
	 * characters before the random key in the Datastring. The random key is
	 * one of the characters in the DataString.
	 * If DB_MULTIPLE, construct the key DBT by the DB_MULTIPLE_WRITE_NEXT
	 * with the specific set of keys. If DB_MULTIPLE_KEY, construct the key
	 * DBT by the DB_MULTIPLE_KEY_WRITE_NEXT with all key/data pairs of the
	 * specific set of keys.
	 */
	flag |= pairs ? DB_MULTIPLE_KEY : DB_MULTIPLE;
	for (i = 0; i <= highest; i++) {
		seckey = DataString[i];
		if (pairs) {
			/*
			 * When specifying the exact key-value pairs to delete,
			 * recompute the same pairs as were inserted; use the
			 * two-item version of the multiple write macro.
			 */
			j = 0;
			do {
				prikey = j * STRINGLEN + i;
				DB_MULTIPLE_KEY_WRITE_NEXT(writepos, &bulk,
				    &seckey, sizeof(seckey),
				    &prikey, sizeof(prikey));
				if (writepos == NULL) {
					/*
					 * The DBT should have been sized to
					 * prevent the it from overflowing,
					 * but check, just in case.
					 */
					dbp->errx(dbp,
					    "Bulk buffer overflow trying to add"
					    " ('%c', %d) batch %d",
					    seckey, prikey, deletes);
					ret = ENOMEM;
					goto err;
				}
				pendingitems++;
				if (verbose)
					printf("Delete secondary record: %c,\t"
					    "data: %d\n", seckey, prikey);
			} while (++j < (int)(numkeys / STRINGLEN));
		} else {
			DB_MULTIPLE_WRITE_NEXT(writepos,
			    &bulk, &seckey, sizeof(seckey));
			assert(writepos != NULL);
			pendingitems++;
			if (verbose)
				printf("Delete key: %c\n", seckey);
		}

		/* If this batch is large enough, delete its records. */
		if (pendingitems >= batchsize) {
			if ((ret =
			    dbenv->txn_begin(dbenv, NULL, &txnp, 0)) != 0) {
				dbenv->err(dbenv, ret, "txn_begin");
				goto err;
			}
			switch (ret = dbp->del(dbp, txnp, &bulk, flag)) {
			case 0:
				deletes++;
				totalitems += pendingitems;
				pendingitems = 0;
				DB_MULTIPLE_WRITE_INIT(writepos, &bulk);
				break;
			default:
				dbp->err(dbp, ret, "bulk_delete_sec DB->del");
				goto err;
			}
			ret = txnp->commit(txnp, 0);
			txnp = NULL;
			if (ret != 0) {
				dbenv->err(dbenv,
				    ret, "bulk_delete_sec txn_commit");
				goto err;
			}
		}
	}

	/* Delete the final batch of records. */
	if (pendingitems != 0) {
		if ((ret = dbenv->txn_begin(dbenv, NULL, &txnp, 0)) != 0) {
			dbenv->err(dbenv, ret, "txn_begin");
			goto err;
		}
		switch (ret = dbp->del(dbp, txnp, &bulk, flag)) {
		case 0:
			deletes++;
			totalitems += pendingitems;
			break;
		default:
			dbp->err(dbp, ret, "final bulk DB->del");
			goto err;
		}
		ret = txnp->commit(txnp, 0);
		txnp = NULL;
		if (ret != 0) {
			dbenv->err(dbenv, ret, "final txn_commit");
			goto err;
		}
	}

	*itemcountp = totalitems;
	*batchcountp = deletes;	

err:	if (txnp != NULL)
		(void)txnp->abort(txnp);
	free(bulk.data);
	return (ret);
}

/*
 * bulk_get --
 *	Repeatedy fetch one bulk batch of records starting from random keys.
 *	This shows how to use both the one-value DB_MULTPLE_NEXT() and the
 *	two-value DB_MUTLIPLE_KEY_NEXT() macros. When there are duplicates we
 *	get a relatively small number of records with DB_MULTPLE_NEXT(), just
 * 	the duplicate data values for a single random key, When there are no
 *	duplicates, we get all the key-value pairs from the starting key to the
 *	last one in the database, with DB_MULTIPLE_KEY_NEXT(). That allows the
 *	DB->get() to return both parts of key-value pairs.
 *
 *	Returns:
 *	0 on success, or
 *	a Berkeley DB error code
 */
int
bulk_get(dbenv, dbp, numkeys, dups, batchsize, iterations, countp, verbose)
	DB_ENV *dbenv;		/* Handle for the opened enivironment. */
	DB *dbp;		/* Handle for the opened primary database. */
	int numkeys;		/* The number of distinct keys. */
	int dups;		/* The number of duplicate values per key. */
	int batchsize;		/* Maximum number of items per DB->get(). */
	int iterations;		/* The number of random gets to perform. */
	long long *countp;	/* Returns the number of records retrieved */
	int verbose;		/* Print the data in each record. */
{
	DBC *dbcp;		/* DBC->get() 'seeks' to key, fetches records. */
	DBT key;		/* The starting point of DBC->get(). Not bulk. */
	DBT bulk;		/* Bulk DBT which receieves many data values. */
	DB_TXN *txnp;		/* The cursor's transaction. */
	bulk_record_t *record;	/* The record extracted from the bulk DBT. */
	u_int32_t flags;	/* DB_MULTIPLE* flags for DBC->get(). */
	long long count;	/* Count the number of records retrieved. */
	int i;			/* The main loop iterator. */
	int randkey;		/* The random key for the current batch. */
	int ret;		/* Berkeley DB API return code. */
	void *readpos;		/* The current location in the return bulk DBT. */
	void *retdata;		/* The data value extracted from bulk. */
	void *retkey;		/* The key value extracted from bulk. */
	u_int32_t retdlen;	/* The length of retdata's value. */
	u_int32_t retklen;	/* The length of retkey's value. */

	/*
	 * Null out the pointers which point to resources to be released
	 * when this function returns.
	 */
	dbcp = NULL;
	txnp = NULL;

	/* Initialize the count of the records retrieved and the error code. */
	count = ret = 0;

	/*
	 * Prepare the key DBT to pass the value in 'randkey' to the API. This
	 * does not set yet key.data and key.size to refer to randkey. Those
	 * fields need to be re-set before each DBC->get() call, which
	 * updates them to denote the key which was retrieved. 
	 */
	memset(&key, 0, sizeof(key));

	/* Initialize the bulk dbt to support at least batchsize items. */
	bulk_dbt_init(&bulk, batchsize);

	/*
	 * Determine the flags to pass to DBC->get(). DB_SET says to position at
	 * the key specified in the key parameter, or the next higher value.
	 */
	flags = DB_SET;
	flags |= dups ? DB_MULTIPLE: DB_MULTIPLE_KEY;
	for (i = 0; i < iterations; i++) {
		if ((ret =
		    dbenv->txn_begin(dbenv, NULL, &txnp, 0)) != 0)
			goto err_cleanup;
		if ((ret = dbp->cursor(dbp, txnp, &dbcp, 0)) != 0)
			goto err_cleanup;

		/*
		 * Get the random key for the start of this batch and set up
		 * the key to use it for the DBC->get(). This needs to be done
		 * each time through this loop, because DBC->get() overwrites
		 * key.data and key.size to the item actually found.
		 */
		randkey = rand() % numkeys;
		key.data = &randkey;
		key.size = sizeof(randkey);
		if (verbose)
			printf("Getting a batch starting at %d\n", randkey);

		/*
		 * If there are duplicates in the database, retrieve
		 * with DB_MULTIPLE and use the DB_MULTIPLE_NEXT
		 * to iterate the data of the random key in the data
		 * DBT. Otherwise retrieve with DB_MULTIPLE_KEY and use
		 * the DB_MULTIPLE_KEY_NEXT to iterate the
		 * key/data pairs of the specific set of keys which
		 * includes all integers >= the random key and < "numkeys".
		 */
		if ((ret = dbcp->get(dbcp, &key, &bulk, flags)) != 0)
			goto err_cleanup;

		/* Prepare to extract items from the bulk dbt. */
		DB_MULTIPLE_INIT(readpos, &bulk);
		if (flags & DB_MULTIPLE)
			while (readpos != NULL) {
				DB_MULTIPLE_NEXT(readpos,
				    &bulk, retdata, retdlen);
				if (retdata) {
					count++;
					record = (bulk_record_t *)retdata;
					if (verbose)
						printf(
						    "Retrieve key: %d, \tdata: "
						    "%d:%.*s\n",
						    *(int *)key.data,
						    record->id,
						    DATALEN, record->str);
				}
			}
		else
			while (readpos != NULL) {
				DB_MULTIPLE_KEY_NEXT(readpos,
				    &bulk, retkey, retklen, retdata, retdlen);
				if (retkey) {
					count++;
					record = (bulk_record_t *)retdata;
					if (verbose)
						printf(
						    "Retrieve key: %d, \tdata: "
						    "%d:%.*s\n", 
						    *((int *) retkey),
						    record->id,
						    DATALEN, record->str);
				}
			}

		ret = dbcp->close(dbcp);
		dbcp = NULL;
		if (ret != 0)
			goto err_cleanup;

		ret = txnp->commit(txnp, 0);
		txnp = NULL;
		if (ret != 0)
			goto err_cleanup;
	}

	/* Tell the caller how many records were retrieved. */
	*countp = count;

	/* Release any active resources obtained by this function: the cursor,
	 * transaction, and bulk dbt buffer.
	 */
err_cleanup:
	if (dbcp != NULL)
		(void)dbcp->close(dbcp);
	if (txnp != NULL)
		(void)txnp->abort(txnp);
	if (ret != 0)
		dbp->err(dbp, ret, "bulk get");
	if (bulk.data != NULL)
		free(bulk.data);
	return (ret);
}

/*
 * compare_int --
 *	Compare two values as the natural integers of this CPU.
 */
int
compare_int(dbp, a, b, locp)
	DB *dbp;
	const DBT *a, *b;
	size_t *locp;
{
	int ai, bi;

	/* Suppress "warning: <var> not used" messages in many compilers. */
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

/*
 * db_init --
 *	Create, configure, and open a database handle for the primary database
 *	inside the file DATABASE (ex_bulk.db).  If the secondary database flag
 * 	as was specified on the command line, create and associate the secondary
 *	datatbase as well.
 *
 * Returns:
 *	0 on success
 *	a Berkeley DB error code on failure, after printing a relevent message
 *	
 */
int
db_init(dbenv, dbpp, sdbpp, dups, secondaryflag, pagesize)
	DB_ENV *dbenv;		/* The already-opened database environemnt */
	DB **dbpp;		/* 'out': return the primary db handle. */
	DB **sdbpp;		/* 'out': return the secondary db handle. */
	int dups;		/* #duplicate per key in primary database. */
	int secondaryflag;	/* If true, create the secondary database. */
	int pagesize;		/* Create the database with this page size. */
{
	DB *dbp;		/* Local handle for the primary database. */
	DB *sdbp;		/* Local handle for the secondry database. */
	DB_TXN *txnp; 		/* Open/create the databases with this txn. */
	int ret;		/* Berkeley DB API return code. */
	char *dbname;		/* Database name: "primary" or "secondary". */

	/*
	 * Clear these Berkeley DB handles so we know that they do not be
	 * closed or aborted, otherwise cleaned up if an error occurs.
	 */
	dbp = NULL;
	sdbp = NULL;
	txnp = NULL;	

	/* Allocate and initialze the handle for the primary database. */
	if ((ret = db_create(&dbp, dbenv, 0)) != 0) {
		dbenv->err(dbenv, ret, "db_create main database handle");
		return (ret);
	}

	/*
	 * Configure the error handling for the database, similarly to what was
	 * done for the environment handle. Here we continue to set the error
	 * file to stderr; however, the prefix is set to the the database
	 * filename rather than the program name as was done for the DB_ENV. 
	 */
	dbp->set_errfile(dbp, stderr);
	dbname = PRIMARY_NAME;
	dbp->set_errpfx(dbp, dbname);

	/* 
	 * By setting the btree comparison function, the records in the primary
	 * database will be stored and retrieved in the numerical order. Without
	 * this the keys will be sorted as byte streams. On little-endian CPUs,
	 * the values returned would not be in numeric order.
	 */
	if ((ret = dbp->set_bt_compare(dbp, compare_int)) != 0) {
		dbp->err(dbp, ret, "set_bt_compare");
		goto err_cleanup;
	}
	/*
	 * Set the size of the Berkeley DB pages to use. This is a tuning
	 * paramter; it does not limit the length of keys or values.
	 */
	if ((ret = dbp->set_pagesize(dbp, pagesize)) != 0) {
		dbp->err(dbp, ret, "set_pagesize(%d)", pagesize);
		goto err_cleanup;
	}
	/*
	 * Permits duplicates if duplicates were requested. Without this it is
	 * not permitted to have two key-value pairs with the same key.
	 */
	if (dups && (ret = dbp->set_flags(dbp, DB_DUP)) != 0) {
		dbp->err(dbp, ret, "set_flags(DB_DUP)");
		goto err_cleanup;
	}

	/* Begin the transaction to use for creating the database(s). */
	if ((ret = dbenv->txn_begin(dbenv, NULL, &txnp, 0)) != 0)
		goto err_cleanup;
	/*
	 * This DB->open() call creates the database file in the file system,
	 * creates a sub-database dbname ("primary") inside that file, and 
	 * opens that sub-database into the handle 'dbp'.
	 */
	if ((ret = dbp->open(dbp, txnp,
	    DATABASE_FILE, dbname, DB_BTREE, DB_CREATE , 0664)) != 0) {
		dbp->err(dbp, ret, "DB->open(%s)", dbname);
		goto err_cleanup;
	}
	*dbpp = dbp;

	if (secondaryflag) {
		/* 
		 * Create a handle for a secondary database. Berkeley DB will
		 * maintain it as a secondary index of the primary database,
		 * using the first character of the primary's data value as the
		 * key.  After the secondary is configured and opened, the 
		 * DB->associate() further down ties the databases together.
		 */
		if ((ret = db_create(&sdbp, dbenv, 0)) != 0) {
			dbenv->err(dbenv, ret, "%s: secondary db_create");
			goto err_cleanup;
		}
		dbname = SECONDARY_NAME;
		dbp->set_errpfx(dbp, dbname);
		/*
		 * Enable support for sorted duplicate data values in the
		 * secondary database.  There are many key+value pairs in the
		 * primary database, by default 1 million.  Since the secondary
		 * key is only 1 character, not early enough to distinguish
		 * that many records, duplicate support is needed. Sorted
		 * duplicates are faster than unsorted (DB_DUP) duplicates.
		 */
		if ((ret = sdbp->set_flags(sdbp, DB_DUPSORT)) != 0) {
			sdbp->err(sdbp, ret, "set_flags(DB_DUPSORT)");
			goto err_cleanup;
		}
		if ((ret = sdbp->open(sdbp, txnp,
		     DATABASE_FILE, dbname, DB_BTREE, DB_CREATE, 0664)) != 0) {
			sdbp->err(sdbp,
			    ret, "%s: secondary open", DATABASE_FILE);
			goto err_cleanup;
		}

		/*
		 * The associate call connects the secondary database to the
		 * primary database, indicating that when a key+value pair is
		 * added to the primary, the function get_first_str() is to be
		 * called to generate the key for the pair to be added to the
		 * secondary. The data portion of the pair will be the primary's
		 * key. This allows the secondary to 'point' to the primary.
		 */
		if ((ret =
		    dbp->associate(dbp, txnp, sdbp, get_first_str, 0)) != 0) {
			dbp->err(dbp, ret, "associate");
			goto err_cleanup;
		}
	}
	*sdbpp = sdbp;

	/*
	 * Commit the transaction which opens and possible creates the on-disk
	 * database file.
	 */
	ret = txnp->commit(txnp, 0);
	txnp = NULL;
	if (ret != 0)
		goto err_cleanup;

	return (0);

	/* This label is used by any error in this function which requires
	 * releasing any locally acquired resources, such as allocated 
	 * database handles or active transactions.
	 */
err_cleanup:
	if (txnp != NULL)
		(void)txnp->abort(txnp);
	if (sdbp != NULL)
		(void)sdbp->close(sdbp, 0);
	if (dbp != NULL)
		(void)dbp->close(dbp, 0);
	return (ret);
}

/*
 * env_init --
 *	Create the environment handle, then configure and open it for the
 *	Transactional Data Store (TDS).
 *	
 */
DB_ENV *
env_init(home, progname, cachesize)
	const char *home;
	const char *progname;
	u_int cachesize;
{
	DB_ENV *dbenv;
	int ret;

	/* Allocate and initialize an empty environment handle. */
	if ((ret = db_env_create(&dbenv, 0)) != 0) {
		dbenv->err(dbenv, ret, "db_env_create");
		return (NULL);
	}

	/*
	 * Send error messages to the standard error stream. You could also
	 * open an application-specific log file to use here.
	 */
	dbenv->set_errfile(dbenv, stderr);

	/* Include the name of the program before each error message. */
	dbenv->set_errpfx(dbenv, progname);

	/* Set the size of the cache which holds database pages. */
	if ((ret = dbenv->set_cachesize(dbenv, 0, cachesize, 0)) != 0) {
		dbenv->err(dbenv, ret, "DB_ENV->set_cachesize(%u)", cachesize);
		return (NULL);
	}

	/*
	 * Open the now-configured environment handle, creating the support
	 * files required by the Berkeley DB Transactional Data Store and
	 * setting up the in-memory data structures of the dbenv for DB access.
	 */
	if ((ret = dbenv->open(dbenv, home, DB_CREATE | DB_INIT_MPOOL |
	    DB_INIT_TXN | DB_INIT_LOCK, 0)) != 0) {
		dbenv->err(dbenv, ret, "DB_ENV->open(%s, TDS)", home);
		(void)dbenv->close(dbenv, 0);
		return (NULL);
	}
	return (dbenv);
}

/*
 * get_first_str --
 *	Construct the key of this example's secondary index.
 *	The secondary's key is the first character of the string field
 *	in the primary database's value DBT.
 */
int
get_first_str(sdbp, key, data, skey)
	DB *sdbp;
	const DBT *key;
	const DBT *data;
	DBT *skey;
{
	/* Suppress "warning: <var> not used" messages in many compilers. */
	sdbp = NULL;
	key = NULL;

	memset(skey, 0, sizeof(DBT));
	skey->data = ((bulk_record_t *)(data->data))->str;
	skey->size = sizeof(char);
	return (0);
}

/*
 * rotate_string --
 *	Perform a left-circular-shift while copying a STRINGLEN-sized char array.
 *	The first character in the destination is the 'offset'th character
 *	of the source; the second is from 'offset'+1, etc. 
 *		src  = "abcde", offset = 3 results in -->
 *		dest = "deabc"
 *	The source and destination are required to be STRINGLEN bytes long.
 *	No null character is appended.
 */
int
rotate_string(source, dest, offset)
	const char *source;
	char *dest;
	int offset;
{
	unsigned i;

	for (i = 0; i < STRINGLEN; i++) 
		dest[i] = source[(offset + i) % STRINGLEN];
	return (0);
}
/*
 * usage --
 *	Describe the command line options.
 */
void
usage()
{
	(void)fprintf(stderr, 
	    "Usage: %s \n"
	    "    -bN	batch size: number of records per bulk operation [100]\n"
	    "    -cN	cachesize [1000 * pagesize]\n"
	    "    -dN	duplicates: number of values for each key [0]\n"
	    "    -iN	number of bulk get calls in read mode [1000000]\n"
	    "    -nN	number of keys [1000000]\n"
	    "    -pN	set pagesize: a power of 2 from 512 to 65536 [65536]\n"
	    "    -v	verbose output\n"
	    "    -D	set mode to perform bulk deletes\n"
	    "    -R	set mode to perform bulk reads\n"
	    "    -S	perform bulk operations in secondary database\n",
			Progname);
	exit(EXIT_FAILURE);
}
