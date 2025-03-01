/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * ex_thread-- A basic example with multithreaded access.
 *   
 *   This sample program demonstrates a simple threaded application of some numbers 
 *   of readers and writers to compete for a set of words. It shows how to prepare and   
 *   open the environment and database handles so that they can be safely shared by freely  
 *   running threads. The application use two kinds of threads: reader and writer to compete 
 *   for the word list. During the time, a statistics function is used to record the number of 
 *   reader or writer threads. In addition, deadlock detection is included in the application.
 *
 *   The program first receives argument options from command line tool to determine the
 *   number of reading threads, writing threads and so on. Then the program builds the key
 *   list by extracting word from woldlist file. After the initialization of environment 
 *   and database, the program creates a new transaction in the environment. Some numbers 
 *   of reading threads and writing threads are created to access words concurently. Every 
 *   thread has one unique identifier for reader or writer.
 *   
 *   At last, the program displays reader/writer thread statistics and exits.
 *
 * program name: ex_thread
 * database: access.db
 *
 * Options:
 *    -d    specify the duration that the program shall run, in seconds
 *    -h    specify the home directory for the environment
 *    -p    set punish flag: switch threads frequently
 *    -n    specify the number of records
 *    -r    specify the number of reading threads
 *    -v    print verbose messages during processing
 *    -w    specify the number of writing threads
 *
 * $Id: ex_thread.c,v 8c6aae1f7250 2015/10/27 17:49:51 charles $
 */

#include <sys/types.h>
#include <sys/time.h>

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
extern int getopt(int, char * const *, const char *);
#else
#include <unistd.h>
#endif

#include <db.h>

/* Suppress unused variable warnings. */
#define	COMPQUIET(n, v)	do {	\
	(n) = (v);		\
	(n) = (n);		\
} while (0)

/*
 * NB: This application is written using POSIX 1003.1b-1993 pthreads
 * interfaces, which may not be portable to your system.
 */
/* Forward declarations */
extern int sched_yield __P((void));		/* Pthread yield function. */

int	db_init __P((const char *));
void   *deadlock __P((void *));
void	fatal __P((const char *, int, int));
void	onint __P((int));
int	main __P((int, char *[]));
int	reader __P((int));
void	stats __P((void));
void   *trickle __P((void *));
void   *tstart __P((void *));
int	usage __P((void));
void	word __P((void));
int	writer __P((int));

time_t	EndTime;		/* -d <#seconds> set the end time of the run. */
int	Quit = 0;		/* Interrupt handling flag. */

struct _statistics {
	int aborted;			/* Write. */
	int aborts;			/* Read/write. */
	int adds;			/* Write. */
	int deletes;			/* Write. */
	int txns;			/* Write. */
	int found;			/* Read. */
	int notfound;			/* Read. */
} *perf;

const char
	*progname = "ex_thread";		/* Program name. */

#define	DATABASE	"access.db"		/* Database name. */
#define	WORDLIST	"../test/tcl/wordlist"	/* Dictionary. */

/*
 * We can seriously increase the number of collisions and transaction
 * aborts by yielding the scheduler after every DB call.  Specify the
 * -p option to do this.
 */
int	punish;					/* -p */
int	nlist;					/* -n */
int	nreaders;				/* -r */
int	verbose;				/* -v */
int	nwriters;				/* -w */

DB     *dbp;					/* Database handle. */
DB_ENV *dbenv;					/* Database environment. */
int	nthreads;				/* Total threads. */
char  **list;					/* Word list. */

/*
 * ex_thread --
 *	Run a simple threaded application of some numbers of readers and
 *	writers competing for a set of words.
 *
 * Example UNIX shell script to run this program:
 *	% rm -rf TESTDIR
 *	% mkdir TESTDIR
 *	% ex_thread -h TESTDIR
 */
int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;		/* argument associated with option */
	extern int errno, optind;       /* index into parent argv vector */
	DB_TXN *txnp;   		/* Transaction handle. */
	pthread_t *tids;		/* Thread ID structure. */
	int ch;				/* The current command line option char. */
	int i;				/* Variable to record thread number. */
	int ret;			/* Return code from call into Berkeley DB. */
	const char *home;		/* The environment home directory. */
	void *retp;			/* Return pointer of the thread. */

	txnp = NULL;
	nlist = 1000;
	nreaders = nwriters = 4;
	home = "TESTDIR";
	/* Parse the command line arguments */
	while ((ch = getopt(argc, argv, "d:h:n:pr:vw:")) != EOF)
		switch (ch) {
		case 'd':
			EndTime = atoi(optarg) + time(NULL);
			break;
		case 'h':	/* Specify the home directory for the environment. */
			home = optarg;
			break;
		case 'n':	/* Specify the number of records. */
			nlist = atoi(optarg);
			break;
		case 'p':	/* Set punish flag. */
			punish = 1;
			break;
		case 'r':	/* Specify the number of reading threads. */
			nreaders = atoi(optarg);
			break;
		case 'v':	/* Print verbose messages during processing. */
			verbose = 1;
			break;
		case 'w':	/* Specify the number of writing threads. */
			nwriters = atoi(optarg);
			break;
		case '?':
		default:
			return (usage());
		}
	argc -= optind;
	argv += optind;

	/* Initialize the random number generator. */
	srand(getpid() | time(NULL));

	/* Register the signal handler. */
	(void)signal(SIGINT, onint);

	/* Build the key list. */
	word();

	/* Remove the previous database. */
	(void)remove(DATABASE);

	/* Initialize the database environment. */
	if ((ret = db_init(home)) != 0)
		return (ret);

	/* Initialize the database. */
	if ((ret = db_create(&dbp, dbenv, 0)) != 0) {
		dbenv->err(dbenv, ret, "db_create");
		(void)dbenv->close(dbenv, 0);
		return (EXIT_FAILURE);
	}
	/* Set page size for the database. */
	if ((ret = dbp->set_pagesize(dbp, 1024)) != 0) {
		dbp->err(dbp, ret, "set_pagesize");
		goto err;
	}
	/* Create a new transaction in the environment. */
	if ((ret = dbenv->txn_begin(dbenv, NULL, &txnp, 0)) != 0)
		fatal("txn_begin", ret, 1);
	/* Trasactionally open database with DB_BTREE access method. */
	if ((ret = dbp->open(dbp, txnp,
	     DATABASE, NULL, DB_BTREE, DB_CREATE | DB_THREAD, 0664)) != 0) {
		dbp->err(dbp, ret, "%s: open", DATABASE);
		goto err;
	} else {
		ret = txnp->commit(txnp, 0);
		txnp = NULL;
		if (ret != 0)
			goto err;
	}

	nthreads = nreaders + nwriters + 2;
	printf("Running: readers %d, writers %d\n", nreaders, nwriters);
	fflush(stdout);

	/* Create statistics structures, offset by 1. */
	if ((perf = calloc(nreaders + nwriters + 1, sizeof(*perf))) == NULL)
		fatal(NULL, errno, 1);

	/* Create thread ID structures. */
	if ((tids = malloc(nthreads * sizeof(pthread_t))) == NULL)
		fatal(NULL, errno, 1);

	/* Create reader/writer threads. */
	for (i = 0; i < nreaders + nwriters; ++i)
		if ((ret = pthread_create(
		    &tids[i], NULL, tstart, (void *)(uintptr_t)i)) != 0)
			fatal("pthread_create", ret > 0 ? ret : errno, 1);

	/* Create buffer pool trickle thread. */
	if (pthread_create(&tids[i], NULL, trickle, &i))
		fatal("pthread_create", errno, 1);
	++i;

	/* Create deadlock detector thread. */
	if (pthread_create(&tids[i], NULL, deadlock, &i))
		fatal("pthread_create", errno, 1);

	/* Wait for the threads. */
	for (i = 0; i < nthreads; ++i)
		(void)pthread_join(tids[i], &retp);

	printf("Exiting\n");
	stats();

err:	if (txnp != NULL)
		(void)txnp->abort(txnp);
	(void)dbp->close(dbp, 0);
	(void)dbenv->close(dbenv, 0);

	return (EXIT_SUCCESS);
}

/*
 * reader --
 * 	Reading thread to read data from buffer, the buffer 
 * stores data from wordlist.
 *
 * Parameter:
 *	id		thread identifier
 */

int
reader(id)
	int id;
{
	DBT key, data;    /* The key/data pair. */
	int n;            /* Number of key. */
	int ret;          /* The return value. */
	char buf[64];     /* Data buffer to hold the data from wordlist. */

	/*
	 * DBT's must use local memory or malloc'd memory if the DB handle
	 * is accessed in a threaded fashion.
	 */
	/*Zero out the DBTs before using them*/
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));
	data.flags = DB_DBT_MALLOC;

	/*
	 * Read-only threads do not require transaction protection, unless
	 * there's a need for repeatable reads.
	 */
	while (!Quit) {
		/* Pick a key at random, and look it up. */
		n = rand() % nlist;
		key.data = list[n];
		key.size = strlen(key.data);

		if (verbose) {
			sprintf(buf, "reader: %d: list entry %d\n", id, n);
			write(STDOUT_FILENO, buf, strlen(buf));
		}

		switch (ret = dbp->get(dbp, NULL, &key, &data, 0)) {
		case DB_LOCK_DEADLOCK:		/* Deadlock. */
			++perf[id].aborts;
			break;
		case 0:				/* Success. */
			++perf[id].found;
			free(data.data);
			break;
		case DB_NOTFOUND:		/* Not found. */
			++perf[id].notfound;
			break;
		default:
			sprintf(buf,
			    "reader %d: dbp->get: %s", id, (char *)key.data);
			fatal(buf, ret, 0);
		}
	}
	return (0);
}

/*
 * writer --
 * 	Writing thread to write data to data buffer.
 *
 * Parameter:
 *	id		thread identifier
 */

int
writer(id)
	int id;
{
	DBT key, data;               /* The key/data pair. */
	DB_TXN *tid;                 /* Transaction identifier. */
	time_t now;                  /* The start time.*/
	time_t then;                 /* The end time. */
	int n;                       /* Number of key. */
	int ret;                     /* The return value. */
	char buf[256];               /* Buffer to store read data. */
	char dbuf[10000];            /* Buffer to store write data. */

	time(&now);
	then = now;

	/*
	 * DBT's must use local memory or malloc'd memory if the DB handle
	 * is accessed in a threaded fashion.
	 */
	/*Zero out the DBTs before using them*/
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));
	data.data = dbuf;
	data.ulen = sizeof(dbuf);
	data.flags = DB_DBT_USERMEM;

	while (!Quit) {
		/* Pick a random key. */
		n = rand() % nlist;
		key.data = list[n];
		key.size = strlen(key.data);

		if (verbose) {
			sprintf(buf, "writer: %d: list entry %d\n", id, n);
			write(STDOUT_FILENO, buf, strlen(buf));
		}

		/* Abort and retry. */
		if (0) {
retry:			if ((ret = tid->abort(tid)) != 0)
				fatal("DB_TXN->abort", ret, 1);
			++perf[id].aborts;
			++perf[id].aborted;
			if (Quit)
				break;
		}

		/* Thread #1 prints out the stats every 20 seconds. */
		if (id == 1) {
			time(&now);
			if (now - then >= 20) {
				stats();
				then = now;
			}
		}

		/* Begin the transaction. */
		if ((ret = dbenv->txn_begin(dbenv, NULL, &tid, 0)) != 0)
			fatal("txn_begin", ret, 1);

		/*
		 * Get the key.  If it doesn't exist, add it.  If it does
		 * exist, delete it.
		 */
		switch (ret = dbp->get(dbp, tid, &key, &data, 0)) {
		case DB_LOCK_DEADLOCK:
			goto retry;
		case 0:
			goto delete;
		case DB_NOTFOUND:
			goto add;
		}

		sprintf(buf, "writer: %d: dbp->get", id);
		fatal(buf, ret, 1);
		/* NOTREACHED */

delete:		/* Delete the key. */
		switch (ret = dbp->del(dbp, tid, &key, 0)) {
		case DB_LOCK_DEADLOCK:
			goto retry;
		case 0:
			++perf[id].deletes;
			goto commit;
		}

		sprintf(buf, "writer: %d: dbp->del", id);
		fatal(buf, ret, 1);
		/* NOTREACHED */

add:		/* Add the key.  1 data item in 30 is an overflow item. */
		data.size = 20 + rand() % 128;
		if (rand() % 30 == 0)
			data.size += 8192;

		switch (ret = dbp->put(dbp, tid, &key, &data, 0)) {
		case DB_LOCK_DEADLOCK:
			goto retry;
		case 0:
			++perf[id].adds;
			goto commit;
		default:
			sprintf(buf, "writer: %d: dbp->put", id);
			fatal(buf, ret, 1);
		}

commit:		/* The transaction finished, commit it. */
		if ((ret = tid->commit(tid, 0)) != 0)
			fatal("DB_TXN->commit", ret, 1);

		/*
		 * Every time the thread completes 10000 transactions, show
		 * our progress.
		 */
		if (++perf[id].txns % 10000 == 0) {
			sprintf(buf,
"writer: %2d: adds: %4d: deletes: %4d: aborts: %4d: txns: %4d\n",
			    id, perf[id].adds, perf[id].deletes,
			    perf[id].aborts, perf[id].txns);
			write(STDOUT_FILENO, buf, strlen(buf));
		}

		/*
		 * If this thread was aborted more than 5 times before
		 * the transaction finished, complain.
		 */
		if (perf[id].aborted > 5) {
			sprintf(buf,
"writer: %2d: adds: %4d: deletes: %4d: aborts: %4d: txns: %4d: ABORTED: %2d\n",
			    id, perf[id].adds, perf[id].deletes,
			    perf[id].aborts, perf[id].txns, perf[id].aborted);
			write(STDOUT_FILENO, buf, strlen(buf));
		}
		perf[id].aborted = 0;
	}
	return (0);
}

/*
 * stats --
 *	Display reader/writer thread statistics.  To display the statistics
 *	for the mpool trickle or deadlock threads, use db_stat(1).
 */
void
stats()
{
	int id;		/* Thread identifier. */
	char *p;	/* Statistics pointer. */
	char buf[8192]; /* Buffer to store statistics data. */

	p = buf + sprintf(buf, "-------------\n");
	for (id = 0; id < nreaders + nwriters;)
		if (id++ < nwriters)
			p += sprintf(p,
	"writer: %2d: adds: %4d: deletes: %4d: aborts: %4d: txns: %4d\n",
			    id, perf[id].adds,
			    perf[id].deletes, perf[id].aborts, perf[id].txns);
		else
			p += sprintf(p,
	"reader: %2d: found: %5d: notfound: %5d: aborts: %4d\n",
			    id, perf[id].found,
			    perf[id].notfound, perf[id].aborts);
	p += sprintf(p, "-------------\n");

	write(STDOUT_FILENO, buf, p - buf);
}

/*
 * db_init --
 *	Initialize the environment.
 *
 * Parameter:
 *      home	environment home directory
 */
int
db_init(home)
	const char *home;
{
	int ret;	/* Return code. */

	/* Create the environment object. */
	if ((ret = db_env_create(&dbenv, 0)) != 0) {
		fprintf(stderr,
		    "%s: db_env_create: %s\n", progname, db_strerror(ret));
		return (EXIT_FAILURE);
	}
	/* If punish is set, tell DB to yield CPU often. */
	if (punish)
		(void)dbenv->set_flags(dbenv, DB_YIELDCPU, 1);

	dbenv->set_errfile(dbenv, stderr);
	dbenv->set_errpfx(dbenv, progname);
	(void)dbenv->set_cachesize(dbenv, 0, 10 * 1024 * 1024, 0);
	(void)dbenv->set_lg_max(dbenv, 10 * 1024 * 1024);
	(void)dbenv->set_flags(dbenv, DB_TXN_NOSYNC, 1);
	(void)dbenv->set_lk_detect(dbenv, DB_LOCK_YOUNGEST);

	/* 
	 * Open a transactional thread-safe environment. Note the use of
	 * the DB_THREAD flag which makes the handle thread-safe.
	 */
	if ((ret = dbenv->open(dbenv, home,
	    DB_CREATE | DB_INIT_LOCK | DB_INIT_LOG |
	    DB_INIT_MPOOL | DB_INIT_TXN | DB_THREAD, 0)) != 0) {
		dbenv->err(dbenv, ret, NULL);
		(void)dbenv->close(dbenv, 0);
		return (EXIT_FAILURE);
	}

	return (0);
}

/*
 * tstart --
 *	Thread start function for readers and writers.
 *
 * Parameter:
 *      arg	thread identifer argument
 */
void *
tstart(arg)
	void *arg;
{
	pthread_t tid;	/* Transaction identifier. */
	u_int id;	/* Thread identifier. */

	id = (uintptr_t)arg + 1;

	tid = pthread_self();

	if (id <= (u_int)nwriters) {
		printf("write thread %d starting: tid: %lu\n", id, (u_long)tid);
		fflush(stdout);
		writer(id);
	} else {
		printf("read thread %d starting: tid: %lu\n", id, (u_long)tid);
		fflush(stdout);
		reader(id);
	}

	/* NOTREACHED */
	return (NULL);
}

/*
 * deadlock --
 *	Thread start function for DB_ENV->lock_detect.
 *
 * Parameter:
 *      arg	argument
 */
void *
deadlock(arg)
	void *arg;
{
	struct timeval t;	/* Time variable. */
	pthread_t tid;		/* Transaction identifier. */

	COMPQUIET(arg, NULL);
	tid = pthread_self();

	printf("deadlock thread starting: tid: %lu\n", (u_long)tid);
	fflush(stdout);

	t.tv_sec = 0;
	t.tv_usec = 100000;
	while (!Quit) {
		/* Run deadlock dectection. */
		(void)dbenv->lock_detect(dbenv, 0, DB_LOCK_YOUNGEST, NULL);

		/* Check every 100ms. */
		(void)select(0, NULL, NULL, NULL, &t);
		if (time(NULL) > EndTime)
			Quit = 1;
	}

	return (NULL);
}

/*
 * trickle --
 *	Thread start function for memp_trickle.
 *
 * Parameter:
 *      arg	argument
 */
void *
trickle(arg)
	void *arg;
{
	pthread_t tid;		/* Transaction identifier. */
	int wrote;		/* Write flag. */
	char buf[64];		/* Data buffer. */

	COMPQUIET(arg, NULL);
	tid = pthread_self();

	printf("trickle thread starting: tid: %lu\n", (u_long)tid);
	fflush(stdout);

	while (!Quit) {
		/*
		 * Make sure there are some clean pages, possibly flushing
		 * some dirty pages.
		 */
		(void)dbenv->memp_trickle(dbenv, 10, &wrote);
		if (verbose) {
			sprintf(buf, "trickle: wrote %d\n", wrote);
			write(STDOUT_FILENO, buf, strlen(buf));
		}
		if (wrote == 0) {
			sleep(1);
			sched_yield();
		}
	}

	return (NULL);
}

/*
 * word --
 *	Build the dictionary word list.
 */
void
word()
{
	FILE *fp;	/* File pointer. */
	int cnt;	/* Count number of words. */
	char buf[256];	/* Buffer data. */
	char *eol;

	if ((fp = fopen(WORDLIST, "r")) == NULL)
		fatal(WORDLIST, errno, 1);

	if ((list = malloc(nlist * sizeof(char *))) == NULL)
		fatal(NULL, errno, 1);

	for (cnt = 0; cnt < nlist; ++cnt) {
		if (fgets(buf, sizeof(buf), fp) == NULL)
			break;
		/* Remove trailing newline */
		if ((eol = strrchr(buf, '\n')) != NULL)
			*eol = '\0';
		if ((list[cnt] = strdup(buf)) == NULL)
			fatal(NULL, errno, 1);
	}
	nlist = cnt;		/* In case nlist was larger than possible. */
}

/*
 * fatal --
 *	Report a fatal error and quit.
 *
 * Parameters:
 *      msg	variable to store error message
 *      err	error variable
 *      syserr	system error variable
 */
void
fatal(msg, err, syserr)
	const char *msg;
	int err, syserr;
{
	fprintf(stderr, "%s: ", progname);
	if (msg != NULL) {
		fprintf(stderr, "%s", msg);
		if (syserr)
			fprintf(stderr, ": ");
	}
	if (syserr)
		fprintf(stderr, "%s", strerror(err));
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);

	/* NOTREACHED */
}

/*
 * usage --
 *	Describe this program's command line options, then exit.
 */
int
usage()
{
	(void)fprintf(stderr,
    "usage: %s [-pv] [-h home] [-n words] [-r readers] [-w writers]\n",
	    progname);
	return (EXIT_FAILURE);
}

/*
 * onint --
 *	Interrupt signal handler.
 *
 * Parameter:
 *      signo	number of signal
 */
void
onint(signo)
	int signo;
{
	signo = 0;		/* Quiet compiler. */
	Quit = 1;
}
