/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * ex_tpcb--
 *
 *   This example is an early transaction processing benchmark that simulates bank
 *   transfers from one account to another. The program is first run in an initialization
 *   mode which loads the data. Subsequent runs in one or more processes perform a 
 *   workload. This program implements a basic TPC/B driver program. To create the TPC/B 
 *   database, run with the -i(init) flag. The number of records with which to populate the 
 *   account, history, branch, and teller tables is specified by the a, s, b, and t flags 
 *   respectively. To run a TPC/B test, use the n flag to indicate a number of transactions 
 *   to run.
 *
 *   The program requires two invocations, both taking in integer identifier as an argument.
 *   This identifier allows for multiple sets of databases to be used within the same 
 *   environment. The first is to initialize the databases, the second is to run the program
 *   on those databases.
 *
 *   After the transaction message is printed, the program closes the environment and exits.
 *
 * Options:
 *    -a	set the number of accounts per teller
 *    -b	set the number of branches
 *    -c	set the cache size in bytes
 *    -f	fast I/O mode: no txn sync
 *    -h	set the home directory
 *    -i	initialize the environment and load databases
 *    -n	set the number of transactions
 *    -S	set the random number seed
 *    -s	set the number of history records
 *    -t	set the number of bank tellers
 *    -v	print verbose messages during processing
 *
 * $Id$
 */

#include <sys/types.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define	NS_PER_MS	1000000		/* Nanoseconds in a millisecond */
#define	NS_PER_US	1000		/* Nanoseconds in a microsecond */
#ifdef _WIN32
#include <sys/timeb.h>
#include <winsock2.h>
extern int getopt(int, char * const *, const char *);
/* Implement a basic high res timer with a POSIX interface for Windows. */
int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	struct _timeb now;
	_ftime(&now);
	tv->tv_sec = (long)now.time;
	tv->tv_usec = now.millitm * NS_PER_US;
	return (0);
}
#else
#include <unistd.h>
#include <sys/time.h>
#endif

#include <db.h>

typedef enum { ACCOUNT, BRANCH, TELLER } FTYPE;

DB_ENV	 *db_init __P((const char *, const char *, int, u_int32_t));
int	  hpopulate __P((DB *, int, int, int, int));
int	  populate __P((DB *, u_int32_t, u_int32_t, int, const char *));
u_int32_t random_id __P((FTYPE, int, int, int));
u_int32_t random_int __P((u_int32_t, u_int32_t));
int	  tp_populate __P((DB_ENV *, int, int, int, int, int));
int	  tp_run __P((DB_ENV *, int, int, int, int, int));
int	  tp_txn __P((DB_ENV *, DB *, DB *, DB *, DB *, int, int, int, int));

int	  invarg __P((const char *, int, const char *));
int	  main __P((int, char *[]));
int	  usage __P((const char *));

#define	TELLERS_PER_BRANCH	10
#define	ACCOUNTS_PER_TELLER	10000
#define	HISTORY_PER_BRANCH	2592000

/*
 * The default configuration that adheres to TPCB scaling rules requires
 * nearly 3 GB of space. To avoid requiring that much space for testing,
 * we set the parameters much lower. If you want to run a valid 10 TPS
 * configuration, define VALID_SCALING.
 */
#ifdef	VALID_SCALING
#define	ACCOUNTS	 1000000
#define	BRANCHES	      10
#define	TELLERS		     100
#define	HISTORY		25920000
#endif

#ifdef	TINY
#define	ACCOUNTS	    1000
#define	BRANCHES	      10
#define	TELLERS		     100
#define	HISTORY		   10000
#endif

#ifdef	VERY_TINY
#define	ACCOUNTS	     500
#define	BRANCHES	      10
#define	TELLERS		      50
#define	HISTORY		    5000
#endif

#if !defined(VALID_SCALING) && !defined(TINY) && !defined(VERY_TINY)
#define	ACCOUNTS	  100000
#define	BRANCHES	      10
#define	TELLERS		     100
#define	HISTORY		  259200
#endif

#define	HISTORY_LEN	    100
#define	RECLEN		    100
#define	BEGID		1000000

typedef struct _defrec {
	u_int32_t	id;
	u_int32_t	balance;
	u_int8_t	pad[RECLEN - sizeof(u_int32_t) - sizeof(u_int32_t)];
} defrec;

typedef struct _histrec {
	u_int32_t	aid;
	u_int32_t	bid;
	u_int32_t	tid;
	u_int32_t	amount;
	u_int8_t	pad[RECLEN - 4 * sizeof(u_int32_t)];
} histrec;

char *progname = "ex_tpcb";			/* Program name. */

int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;    /* argument associated with option */
	extern int optind;      /* index into parent argv vector */
	DB_ENV *dbenv;          /* The environment handle. */
	int accounts;           /* The account variable. */
	int branches;           /* The branch variable. */
	int seed;               /* The random number seed variable. */
	int tellers;            /* The teller variable. */
	int history;            /* The history record variable. */
	int ch;                 /* The current command line option char. */
	int iflag;              /* The initialization flag. */
	int mpool;              /* Memory pool variable. */
	int ntxns;              /* The number of transactions. */
	int ret;                /* Return code from call into Berkeley DB. */
	int txn_no_sync;        /* Transaction no synchronization flag. */
	int verbose;            /* Verbose message flag. */
	const char *home;       /* Home directory constant. */

	home = "TESTDIR";
	accounts = branches = history = tellers = 0;
	iflag = mpool = ntxns = txn_no_sync = verbose = 0;
	seed = (int)time(NULL);
	/* Parse the command line arguments */
	while ((ch = getopt(argc, argv, "a:b:c:fh:in:S:s:t:v")) != EOF)
		switch (ch) {
		case 'a':			/* Set the number of accounts per teller. */
			if ((accounts = atoi(optarg)) <= 0)
				return (invarg(progname, ch, optarg));
			break;
		case 'b':			/* Set the number of branches. */
			if ((branches = atoi(optarg)) <= 0)
				return (invarg(progname, ch, optarg));
			break;
		case 'c':			/* Set the cache size in bytes. */
			if ((mpool = atoi(optarg)) <= 0)
				return (invarg(progname, ch, optarg));
			break;
		case 'f':			/* Fast I/O mode: no txn sync. */
			txn_no_sync = 1;
			break;
		case 'h':			/* Set the home directory. */
			home = optarg;
			break;
		case 'i':			/* Initialize the environment and load databases. */
			iflag = 1;
			break;
		case 'n':			/* Set the number of transactions */
			if ((ntxns = atoi(optarg)) <= 0)
				return (invarg(progname, ch, optarg));
			break;
		case 'S':			/* Set the random number seed. */
			if ((seed = atoi(optarg)) <= 0)
				return (invarg(progname, ch, optarg));
			break;
		case 's':			/* Set the number of history records */
			if ((history = atoi(optarg)) <= 0)
				return (invarg(progname, ch, optarg));
			break;
		case 't':			/* Set the number of bank tellers. */
			if ((tellers = atoi(optarg)) <= 0)
				return (invarg(progname, ch, optarg));
			break;
		case 'v':			/* Print verbose messages during processing. */
			verbose = 1;
			break;
		case '?':
		default:
			return (usage(progname));
		}
	argc -= optind;
	argv += optind;

	srand((u_int)seed);

	/* Initialize the database environment. */
	if ((dbenv = db_init(home,
	    progname, mpool, txn_no_sync ? DB_TXN_NOSYNC : 0)) == NULL)
		return (EXIT_FAILURE);

	accounts = accounts == 0 ? ACCOUNTS : accounts;
	branches = branches == 0 ? BRANCHES : branches;
	tellers = tellers == 0 ? TELLERS : tellers;
	history = history == 0 ? HISTORY : history;

	if (verbose)
		printf("%ld Accounts, %ld Branches, %ld Tellers, %ld History\n",
		    (long)accounts, (long)branches,
		    (long)tellers, (long)history);

	if (iflag) {
		if (ntxns != 0)
			return (usage(progname));
		tp_populate(dbenv,
		    accounts, branches, history, tellers, verbose);
	} else {
		if (ntxns == 0)
			return (usage(progname));
		tp_run(dbenv, ntxns, accounts, branches, tellers, verbose);
	}

	if ((ret = dbenv->close(dbenv, 0)) != 0) {
		fprintf(stderr, "%s: dbenv->close failed: %s\n",
		    progname, db_strerror(ret));
		return (EXIT_FAILURE);
	}

	return (EXIT_SUCCESS);
}

/*
 * invarg --
 *	Describe invalid command line argument, then exit.
 *
 * Parameters:
 *   progname        program name
 *   arg             argument variable
 *   str             invalid input string
 */
int
invarg(progname, arg, str)
	const char *progname;
	int arg;
	const char *str;
{
	(void)fprintf(stderr,
	    "%s: invalid argument for -%c: %s\n", progname, arg, str);
	return (EXIT_FAILURE);
}

/*
 * usage --
 *	Describe this program's command line options, then exit.
 *
 * Parameters:
 *  progname        program name
 */
int
usage(progname)
	const char *progname;
{
	const char *a1, *a2;

	a1 = "[-fv] [-a accounts] [-b branches]\n";
	a2 = "\t[-c cache_size] [-h home] [-S seed] [-s history] [-t tellers]";
	(void)fprintf(stderr, "usage: %s -i %s %s\n", progname, a1, a2);
	(void)fprintf(stderr,
	    "       %s -n transactions %s %s\n", progname, a1, a2);
	return (EXIT_FAILURE);
}

/*
 * db_init --
 *	Initialize the environment.
 *
 * Parameters:
 *    home        home directory
 *    prefix      an error output stream 
 *    cachesize   cache size of the database environment
 *    flags       environment open flag
 */
DB_ENV *
db_init(home, prefix, cachesize, flags)
	const char *home, *prefix;
	int cachesize;
	u_int32_t flags;
{
	DB_ENV *dbenv;            /* Database environment handle. */
	u_int32_t local_flags;    /* Local flags to open environment. */
	int ret;                  /* Return code. */

	/* Create the environment object. */
	if ((ret = db_env_create(&dbenv, 0)) != 0) {
		fprintf(stderr,
		    "%s: db_env_create: %s\n", progname, db_strerror(ret));
		return (NULL);
	}

	/*
	 * Prefix any error messages with the name of this program and a ':'.
	 * Setting the errfile to stderr is not necessary, since that is the
	 * default; it is provided here as a placeholder showing where one
	 * could direct error messages to an application-specific log file.
	 */
	dbenv->set_errfile(dbenv, stderr);
	dbenv->set_errpfx(dbenv, prefix);

	/* Use the default deadlock detector. */
	(void)dbenv->set_lk_detect(dbenv, DB_LOCK_DEFAULT);
	/* Set the cache size. */
	(void)dbenv->set_cachesize(dbenv, 0,
	    cachesize == 0 ? 4 * 1024 * 1024 : (u_int32_t)cachesize, 0);

	/* 
	 * If -f option is specified, do not synchronize on transaction
	 * commit. This reduces the durability of the database but improves
	 * transaction throughput.
	 */
	if (flags & (DB_TXN_NOSYNC))
		(void)dbenv->set_flags(dbenv, DB_TXN_NOSYNC, 1);
	flags &= ~(DB_TXN_NOSYNC);

	/* Open a transactional environment. */
	local_flags = flags | DB_CREATE | DB_INIT_LOCK | DB_INIT_LOG |
	    DB_INIT_MPOOL | DB_INIT_TXN;
	/* Open environment with local flags. */
	if ((ret = dbenv->open(dbenv, home, local_flags, 0)) != 0) {
		dbenv->err(dbenv, ret, "DB_ENV->open: %s", home);
		(void)dbenv->close(dbenv, 0);
		return (NULL);
	}
	return (dbenv);
}

/*
 * tp_populate --
 *        Initialize the database to the specified number of accounts, branches,
 * history records, and tellers.
 *
 * Parameters:
 *    env        environment handle
 *    accounts   account variable to specify
 *    branches   branch variable to specify
 *    history    history record
 *    tellers    teller variable to specify
 *    verbose    verbose message 
 */
int
tp_populate(env, accounts, branches, history, tellers, verbose)
	DB_ENV *env;
	int accounts, branches, history, tellers, verbose;
{
	DB *dbp;                  /* The database handle. */
	u_int32_t balance;        /* Balance of account. */
	u_int32_t idnum;          /* Number of transaction identifier. */
	u_int32_t oflags;         /* Open flag for the database. */
	u_int32_t end_anum;       /* End number of accounts. */
	u_int32_t end_bnum;       /* End number of branches. */
	u_int32_t end_tnum;       /* End number of tellers. */
	u_int32_t start_anum;     /* Start number of accounts. */
	u_int32_t start_bnum;     /* Start number of branches. */
	u_int32_t start_tnum;     /* Start number of tellers. */
	int ret;                  /* Return code. */

	idnum = BEGID;
	balance = 500000;
	oflags = DB_CREATE;

	/* Create the database object for accounts. */
	if ((ret = db_create(&dbp, env, 0)) != 0) {
		env->err(env, ret, "db_create");
		return (1);
	}
	/* Set the estimated final size of the account database. */
	(void)dbp->set_h_nelem(dbp, (u_int32_t)accounts);

	/* Create the account database with the hash access method. */
	if ((ret = dbp->open(dbp, NULL, "account", NULL,
	    DB_HASH, oflags, 0644)) != 0) {
		env->err(env, ret, "DB->open: account");
		return (1);
	}

	/* Populate the account database. */
	start_anum = idnum;
	populate(dbp, idnum, balance, accounts, "account");
	idnum += accounts;
	end_anum = idnum - 1;
	/* Close the account database. */
	if ((ret = dbp->close(dbp, 0)) != 0) {
		env->err(env, ret, "DB->close: account");
		return (1);
	}
	if (verbose)
		printf("Populated accounts: %ld - %ld\n",
		    (long)start_anum, (long)end_anum);

	/*
	 * Since the number of branches is very small, we want to use very
	 * small pages and only 1 key per page, i.e., key-locking instead
	 * of page locking.
	 */
	if ((ret = db_create(&dbp, env, 0)) != 0) {
		env->err(env, ret, "db_create");
		return (1);
	}
	/* Set the fill factor to be 1, allowing approximately 1 key per bucket. */
	(void)dbp->set_h_ffactor(dbp, 1);
	/* Set the estimated final size of the branch database. */
	(void)dbp->set_h_nelem(dbp, (u_int32_t)branches);
	/* Use a small page size to reduce the number of keys per page. */
	(void)dbp->set_pagesize(dbp, 512);
	/* Open the branch database with the hash access method. */
	if ((ret = dbp->open(dbp, NULL, "branch", NULL,
	    DB_HASH, oflags, 0644)) != 0) {
		env->err(env, ret, "DB->open: branch");
		return (1);
	}

	/* Populate the branch database. */
	start_bnum = idnum;
	populate(dbp, idnum, balance, branches, "branch");
	idnum += branches;
	end_bnum = idnum - 1;
	if ((ret = dbp->close(dbp, 0)) != 0) {
		env->err(env, ret, "DB->close: branch");
		return (1);
	}
	if (verbose)
		printf("Populated branches: %ld - %ld\n",
		    (long)start_bnum, (long)end_bnum);

	/*
	 * In the case of tellers, we also want small pages, but we'll let
	 * the fill factor dynamically adjust itself.
	 */
	if ((ret = db_create(&dbp, env, 0)) != 0) {
		env->err(env, ret, "db_create");
		return (1);
	}
	/* Set the fill factor to 0 to allow it dynamically adjust itself. */
	(void)dbp->set_h_ffactor(dbp, 0);
	/* Set the estimated final size of the teller database. */
	(void)dbp->set_h_nelem(dbp, (u_int32_t)tellers);
	/* Use a small page size. */
	(void)dbp->set_pagesize(dbp, 512);
	/* Open the teller database with the hash access method. */
	if ((ret = dbp->open(dbp, NULL, "teller", NULL,
	    DB_HASH, oflags, 0644)) != 0) {
		env->err(env, ret, "DB->open: teller");
		return (1);
	}

	/* Populate the teller database. */
	start_tnum = idnum;
	populate(dbp, idnum, balance, tellers, "teller");
	idnum += tellers;
	end_tnum = idnum - 1;
	if ((ret = dbp->close(dbp, 0)) != 0) {
		env->err(env, ret, "DB->close: teller");
		return (1);
	}
	if (verbose)
		printf("Populated tellers: %ld - %ld\n",
		    (long)start_tnum, (long)end_tnum);

	if ((ret = db_create(&dbp, env, 0)) != 0) {
		env->err(env, ret, "db_create");
		return (1);
	}
	/* Set the record length of history records. */
	(void)dbp->set_re_len(dbp, HISTORY_LEN);
	/* Open the history database with the recno access method. */
	if ((ret = dbp->open(dbp, NULL, "history", NULL,
	    DB_RECNO, oflags, 0644)) != 0) {
		env->err(env, ret, "DB->open: history");
		return (1);
	}

	/* Populate the history records. */
	hpopulate(dbp, history, accounts, branches, tellers);
	if ((ret = dbp->close(dbp, 0)) != 0) {
		env->err(env, ret, "DB->close: history");
		return (1);
	}
	return (0);
}

/*
 * populate --
 *        Initialize the database to the specified value according to the file type.
 *
 * Parameters:
 *    dbp        database handle
 *    start_id   start position
 *    balance    the balance of account
 *    nrecs      number of records
 *    msg        error message
 */
int
populate(dbp, start_id, balance, nrecs, msg)
	DB *dbp;
	u_int32_t start_id, balance;
	int nrecs;
	const char *msg;
{
	DBT kdbt;         /* The key value of the database. */
	DBT ddbt;         /* The data value of the database. */
	defrec drec;      /* database record. */
	int i;            /* Variable to indicate the position of the record. */
	int ret;          /* Return code. */

	/* Set the key to be record's id, and data to be the entire record. */
	kdbt.flags = 0;
	kdbt.data = &drec.id;
	kdbt.size = sizeof(u_int32_t);
	ddbt.flags = 0;
	ddbt.data = &drec;
	ddbt.size = sizeof(drec);
	memset(&drec.pad[0], 1, sizeof(drec.pad));

	/* Insert records. */
	for (i = 0; i < nrecs; i++) {
		drec.id = start_id + (u_int32_t)i;
		drec.balance = balance;
		if ((ret =
		    (dbp->put)(dbp, NULL, &kdbt, &ddbt, DB_NOOVERWRITE)) != 0) {
			dbp->err(dbp,
			    ret, "Failure initializing %s file\n", msg);
			return (1);
		}
	}
	return (0);
}

/*
 * hpopulate --
 *        Initialize the database to the specified number of history records.
 *
 * Parameters:
 *    dbp        database handle
 *    history    history record variable
 *    accounts   account variable to specify
 *    branches   branch variable to specify
 *    tellers    teller variable to specify
 */
int
hpopulate(dbp, history, accounts, branches, tellers)
	DB *dbp;
	int history, accounts, branches, tellers;
{
	DBT kdbt;         /* The key value of the database. */
	DBT ddbt;         /* The data value of the database. */
	histrec hrec;     /* History record. */
	db_recno_t key;   /* The record number variable of the transaction database. */
	int i;            /* variable to indicate the position of history record. */
	int ret;          /* Return code. */

	/* 
	 * The key must be a record number. Set the data to be the entire
	 * history record.
	 */
	memset(&kdbt, 0, sizeof(kdbt));
	memset(&ddbt, 0, sizeof(ddbt));
	ddbt.data = &hrec;
	ddbt.size = sizeof(hrec);
	kdbt.data = &key;
	kdbt.size = sizeof(key);
	memset(&hrec.pad[0], 1, sizeof(hrec.pad));
	hrec.amount = 10;

	/* Insert history records. */
	for (i = 1; i <= history; i++) {
		hrec.aid = random_id(ACCOUNT, accounts, branches, tellers);
		hrec.bid = random_id(BRANCH, accounts, branches, tellers);
		hrec.tid = random_id(TELLER, accounts, branches, tellers);
		if ((ret = dbp->put(dbp, NULL, &kdbt, &ddbt, DB_APPEND)) != 0) {
			dbp->err(dbp, ret, "dbp->put");
			return (1);
		}
	}
	return (0);
}

/*
 * random_int --
 *        function to generate random integer within the range
 *
 * Parameters:
 *    lo    the low bound of the range
 *    hi    the high bound of the range
 */
u_int32_t
random_int(lo, hi)
	u_int32_t lo, hi;
{
	u_int32_t ret;          /* Return code. */
	int t;                  /* random integer. */

#ifndef RAND_MAX
#define	RAND_MAX	0x7fffffff
#endif
	t = rand();
	ret = (u_int32_t)(((double)t / ((double)(RAND_MAX) + 1)) *
	    (hi - lo + 1));
	ret += lo;
	return (ret);
}

/*
 * random_int --
 *        function to generate transaction identifier.
 *
 * Parameters:
 *    type        the file type
 *    accounts    the account variable
 *    branches    the branch variable
 *    tellers      the teller variable
 */
u_int32_t
random_id(type, accounts, branches, tellers)
	FTYPE type;
	int accounts, branches, tellers;
{
	u_int32_t min;        /* The minimum value of the transaction identifier. */
	u_int32_t max;        /* The maximum value of the transaction identifier. */
	u_int32_t num;        /* The range between the minimum value and maximum value. */

	max = min = BEGID;
	num = accounts;
	switch (type) {
	case TELLER:
		min += branches;
		num = tellers;
		/* FALLTHROUGH */
	case BRANCH:
		if (type == BRANCH)
			num = branches;
		min += accounts;
		/* FALLTHROUGH */
	case ACCOUNT:
		max = min + num - 1;
	}
	return (random_int(min, max));
}

/*
 * tp_run --
 *        function to run the transaction.
 *
 * Parameters:
 *    dbenv        the database environment handle
 *    n            the number of transactions
 *    accounts     the account variable
 *    branches     the branch variable
 *    tellers      the teller variable
 *    verbose      verbose message
 */
int
tp_run(dbenv, n, accounts, branches, tellers, verbose)
	DB_ENV *dbenv;
	int n, accounts, branches, tellers, verbose;
{
	DB *adb;          /* The accounts database. */
	DB *bdb;          /* The branches database. */
	DB *hdb;          /* The history record database. */
	DB *tdb;          /* The tellers database. */
	int failed;       /* Transaction running fail flag. */
	int ret;          /* Return code. */
	int txns;         /* Number of transactions. */
	struct timeval start_tv;        /* Time variable to store the start time. */   
	struct timeval end_tv;          /* Time variable to store the end time. */
	double start_time;              /* Start time variable. */
	double end_time;                /* End time varialbe. */

	adb = bdb = hdb = tdb = NULL;

	/*
	 * Open the database files.
	 */
	if ((ret = db_create(&adb, dbenv, 0)) != 0) {
		dbenv->err(dbenv, ret, "db_create");
		goto err;
	}
	if ((ret = adb->open(adb, NULL, "account", NULL, DB_UNKNOWN,
	    DB_AUTO_COMMIT, 0)) != 0) {
		dbenv->err(dbenv, ret, "DB->open: account");
		goto err;
	}
	if ((ret = db_create(&bdb, dbenv, 0)) != 0) {
		dbenv->err(dbenv, ret, "db_create");
		goto err;
	}
	if ((ret = bdb->open(bdb, NULL, "branch", NULL, DB_UNKNOWN,
	    DB_AUTO_COMMIT, 0)) != 0) {
		dbenv->err(dbenv, ret, "DB->open: branch");
		goto err;
	}
	if ((ret = db_create(&hdb, dbenv, 0)) != 0) {
		dbenv->err(dbenv, ret, "db_create");
		goto err;
	}
	if ((ret = hdb->open(hdb, NULL, "history", NULL, DB_UNKNOWN,
	    DB_AUTO_COMMIT, 0)) != 0) {
		dbenv->err(dbenv, ret, "DB->open: history");
		goto err;
	}
	if ((ret = db_create(&tdb, dbenv, 0)) != 0) {
		dbenv->err(dbenv, ret, "db_create");
		goto err;
	}
	if ((ret = tdb->open(tdb, NULL, "teller", NULL, DB_UNKNOWN,
	    DB_AUTO_COMMIT, 0)) != 0) {
		dbenv->err(dbenv, ret, "DB->open: teller");
		goto err;
	}

	/* Get the start time. */
	(void)gettimeofday(&start_tv, NULL);

	/* Run n transactions. */
	for (txns = n, failed = 0; n-- > 0;)
		if ((ret = tp_txn(dbenv, adb, bdb, tdb, hdb,
		    accounts, branches, tellers, verbose)) != 0)
			++failed;

	/* Get the end time. */
	(void)gettimeofday(&end_tv, NULL);

	start_time = start_tv.tv_sec + ((start_tv.tv_usec + 0.0)/NS_PER_MS);
	end_time = end_tv.tv_sec + ((end_tv.tv_usec + 0.0)/NS_PER_MS);
	if (end_time == start_time)
		end_time += 1/NS_PER_MS;

	/* Print the result. */
	printf("%s: %d txns: %d failed, %.3f sec, %.2f TPS\n", progname,
	    txns, failed, (end_time - start_time),
	    (txns - failed) / (double)(end_time - start_time));

	/* Close databases. */
err:	if (adb != NULL)
		(void)adb->close(adb, 0);
	if (bdb != NULL)
		(void)bdb->close(bdb, 0);
	if (tdb != NULL)
		(void)tdb->close(tdb, 0);
	if (hdb != NULL)
		(void)hdb->close(hdb, 0);
	return (ret == 0 ? 0 : 1);
}

/*
 * tp_txn --
 *        function to run one single specified transaction.
 *
 * Parameters:
 *    dbenv        the database environment handle
 *    adb          the accounts database
 *    bdb          the branches database
 *    tdb          the tellers database
 *    hdb          the history record database
 *    accounts     the account variable
 *    branches     the branch variable
 *    tellers      the teller variable
 *    verbose      verbose message
 */
int
tp_txn(dbenv, adb, bdb, tdb, hdb, accounts, branches, tellers, verbose)
	DB_ENV *dbenv;
	DB *adb, *bdb, *tdb, *hdb;
	int accounts, branches, tellers, verbose;
{
	DBC *acurs;        /* The accounts database cursor. */
	DBC *bcurs;        /* The branches database cursor. */
	DBC *tcurs;        /* The tellers database cursor. */
	DBT d_dbt;         /* The data value of the database. */
	DBT d_histdbt;     /* The data value of the history record database. */
	DBT k_dbt;         /* The key value of the database. */
	DBT k_histdbt;     /* The key value of the history record database. */
	DB_TXN *t;         /* The transaction handle. */
	db_recno_t key;    /* The record number variable of the transaction database. */
	defrec rec;        /* The record variable. */
	histrec hrec;      /* The history record. */
	int account;       /* The account variable. */
	int branch;        /* The branch variable. */
	int teller;        /* The teller variable. */
	int ret;           /* Return code. */

	t = NULL;
	acurs = bcurs = tcurs = NULL;

	/*
	 * !!!
	 * This is sample code -- we could move a lot of this into the driver
	 * to make it faster.
	 */
	account = random_id(ACCOUNT, accounts, branches, tellers);
	branch = random_id(BRANCH, accounts, branches, tellers);
	teller = random_id(TELLER, accounts, branches, tellers);

	/* Initialize history key/data pairs. */
	memset(&d_histdbt, 0, sizeof(d_histdbt));

	memset(&k_histdbt, 0, sizeof(k_histdbt));
	k_histdbt.data = &key;
	k_histdbt.size = sizeof(key);

	/* Initialize database key/data pairs. */
	memset(&k_dbt, 0, sizeof(k_dbt));
	k_dbt.size = sizeof(int);

	memset(&d_dbt, 0, sizeof(d_dbt));
	d_dbt.flags = DB_DBT_USERMEM;
	d_dbt.data = &rec;
	d_dbt.ulen = sizeof(rec);

	hrec.aid = account;
	hrec.bid = branch;
	hrec.tid = teller;
	hrec.amount = 10;
	/* Request 0 bytes since we're just positioning. */
	d_histdbt.flags = DB_DBT_PARTIAL;

	/*
	 * START PER-TRANSACTION TIMING.
	 *
	 * Technically, TPCB requires a limit on response time, you only get
	 * to count transactions that complete within 2 seconds.  That's not
	 * an issue for this sample application -- regardless, here's where
	 * the transaction begins.
	 */
	if (dbenv->txn_begin(dbenv, NULL, &t, 0) != 0)
		goto err;

	/* Create transactional cursors for account, branch and teller databases. */
	if (adb->cursor(adb, t, &acurs, 0) != 0 ||
	    bdb->cursor(bdb, t, &bcurs, 0) != 0 ||
	    tdb->cursor(tdb, t, &tcurs, 0) != 0)
		goto err;

	/* Read and update one account record */
	k_dbt.data = &account;
	if (acurs->get(acurs, &k_dbt, &d_dbt, DB_SET) != 0)
		goto err;
	rec.balance += 10;
	if (acurs->put(acurs, &k_dbt, &d_dbt, DB_CURRENT) != 0)
		goto err;

	/* Read and update one branch record */
	k_dbt.data = &branch;
	if (bcurs->get(bcurs, &k_dbt, &d_dbt, DB_SET) != 0)
		goto err;
	rec.balance += 10;
	if (bcurs->put(bcurs, &k_dbt, &d_dbt, DB_CURRENT) != 0)
		goto err;

	/* Read and update one teller record */
	k_dbt.data = &teller;
	if (tcurs->get(tcurs, &k_dbt, &d_dbt, DB_SET) != 0)
		goto err;
	rec.balance += 10;
	if (tcurs->put(tcurs, &k_dbt, &d_dbt, DB_CURRENT) != 0)
		goto err;

	/* Add one history record */
	d_histdbt.flags = 0;
	d_histdbt.data = &hrec;
	d_histdbt.ulen = sizeof(hrec);
	if (hdb->put(hdb, t, &k_histdbt, &d_histdbt, DB_APPEND) != 0)
		goto err;

	/* Close all cursors. */
	if (acurs->close(acurs) != 0 || bcurs->close(bcurs) != 0 ||
	    tcurs->close(tcurs) != 0)
		goto err;
	/* Commit the transaction. */
	ret = t->commit(t, 0);
	t = NULL;
	if (ret != 0)
		goto err;
	/* END PER-TRANSACTION TIMING. */

	return (0);

err:	if (acurs != NULL)
		(void)acurs->close(acurs);
	if (bcurs != NULL)
		(void)bcurs->close(bcurs);
	if (tcurs != NULL)
		(void)tcurs->close(tcurs);
	if (t != NULL)
		(void)t->abort(t);

	if (verbose)
		printf("Transaction A=%ld B=%ld T=%ld failed\n",
		    (long)account, (long)branch, (long)teller);
	return (-1);
}
