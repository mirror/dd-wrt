/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2014, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#include <db_config.h>
#include <db.h>

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

/* This BDB internal routine calls gettimeofday() or clock_gettime() or ...  */
void __os_gettime __P((const ENV *, struct timespec *, int));

/* This exit status says "stop, look at the last run. something didn't work." */
#define EXIT_TEST_ABORTED	101

/*
 * "Shut that bloody compiler up!"
 *
 * Unused, or not-used-yet variable.  We need to write and then read the
 * variable, some compilers are too bloody clever by half.
 */
#define	COMPQUIET(n, v)	do {					        \
	(n) = (v);						        \
	(n) = (n);						        \
} while (0)
#define	UTIL_ANNOTATE_STRLEN	64	/* Length of argument to util_annotate(). */

/*
 * NB: This application is written using POSIX 1003.1b-1993 pthreads
 * interfaces, which may not be portable to your system.
 */
extern int sched_yield __P((void));		/* Pthread yield function. */

extern pthread_key_t TxnCommitMutex;

int	db_init __P((DB_ENV **, u_int32_t));
void   *deadlock __P((void *));
void	fatal __P((const char *, int));
void	onint __P((int));
int	main __P((int, char *[]));
void	notice_event __P((DB_ENV *, u_int32_t, void *));
int	reader __P((int));
void	stats __P((void));
void   *failchk __P((void *));
int	say_is_alive __P((DB_ENV *, pid_t, db_threadid_t, unsigned));
void   *trickle __P((void *));
void   *tstart __P((void *));
int	usage __P((const char *));
const char   *util_annotate __P((const DB_ENV *, char *, size_t));
void	util_errcall __P((const DB_ENV *, const char *, const char *));
void	util_msgcall __P((const DB_ENV *, const char *, const char *));
void	word __P((void));
int	writer __P((int));

struct _statistics {
	int aborted;		/* Write. */
	int aborts;		/* Read/write. */
	int adds;		/* Write. */
	int deletes;		/* Write. */
	int txns;		/* Write. */
	int found;		/* Read. */
	int notfound;		/* Read. */
} *perf;

const char *Progname = "test_failchk";		/* Program name. */

#define	DATABASE	"access.db"		/* Database name. */
#define	WORDLIST	"../test/tcl/wordlist"	/* Dictionary. */

/*
 * We can seriously increase the number of collisions and transaction
 * aborts by yielding the scheduler after every DB call.  Specify the
 * -p option to do this.
 */
time_t	EndTime;
int	Duration = 60;		/* -d <#seconds to run> */
char	*Home = "TESTDIR";	/* -h */
int	Punish;			/* -p */
int	Nlist = 1000;		/* -n */
int	Nreaders = 4;		/* -r */
int	StatsInterval = 60;	/* -s #seconds between printout of statistics */
int	TxnInterval = 1000;	/* -t #txns between printout of txn progress */
int	Verbose;		/* -v */
int	Nwriters = 4;		/* -w */


int	ActiveThreads = 0;
DB     *Dbp;			/* Database handle. */
DB_ENV *DbEnv;			/* Database environment. */
int	EnvOpenFlags =  DB_CREATE | DB_THREAD | DB_REGISTER;
int	Failchk = 0;		/* Failchk found a dead process. */
char  **List;			/* Word list. */
int	MutexDied = 0;		/* #threads that tripped on a dead mutex */
int	Nthreads;		/* Total number of non-failchk threads. */
int	Quit = 0;		/* Interrupt handling flag. */
int	PrintStats = 0;		/* -S print all stats before exit. */

/*
 * test_failchk --
 *	Test failchk in a simple threaded application of some numbers of readers
 *	and writers competing to read and update a set of words.
 *	A typical test scenario runs this programs several times concurrently,
 *	with different options:
 *		first with the -I option to clear out any home directory
 *		one or more instances with -f to activate the failchk thread
 *		one or more instance with neither -I nor -f, as minimally
 *			involved workers.
 *	
 *
 *	
 *
 * Example UNIX shell script to run this program:
 *	test_failchk -I &	# recreates the home TESTDIR directory
 *	test_failchk &		# read & write in testdir, expecting w
 *	test_failchk -f &	# read & write & call faichk to notice crashes
 *	randomly kill a process, leaving at least one other to discover the crash
 *	with a DB_ENV->failchk() call, which then allows the other processes 
 */
int
main(argc, argv)
	int argc;
	char *argv[];
{
	extern char *optarg;
	extern int errno, optind;
	DB_TXN *txnp;
	pthread_t *tids;
	void (*sig)();
	int ch, do_failchk, i, init, pid, ret;
	char syscmd[100];
	void *retp;

	setlinebuf(stdout);
	setlinebuf(stderr);
	txnp = NULL;
	do_failchk = init = 0;
	pid = getpid();
	while ((ch = getopt(argc, argv, "d:fh:Ipn:Rr:Ss:t:vw:x")) != EOF)
		switch (ch) {
		case 'd':
			if ((Duration = atoi(optarg)) <= 0)
				return (usage("-d <duration> must be >= 0"));
			break;
		case 'f':
			do_failchk = 1;
			break;
		case 'h':
			Home = optarg;
			break;
		case 'I':
			init = 1;
			break;
		case 'n':
			Nlist = atoi(optarg);
			break;
		case 'p':
			Punish = 1;
			break;
		case 'R':
			EnvOpenFlags &= ~DB_REGISTER;
			break;
		case 'r':
			if ((Nreaders = atoi(optarg)) < 0)
				return (usage("-r <readers> may not be <0"));
			break;
		case 'S':
			PrintStats = 1;
			break;
		case 's':
			if ((StatsInterval = atoi(optarg)) <= 0)
				return (usage("-s <seconds> must be positive"));
			break;
		case 't':
			if ((TxnInterval = atoi(optarg)) <= 0)
				return (usage("-t <#txn> must be positive"));
			break;
		case 'v':
			Verbose = 1;
			break;
		case 'w':
			if ((Nwriters = atoi(optarg)) < 0)
				return (usage("-r <writers> may not be <0"));
			break;
		case 'x':
			EnvOpenFlags |= DB_RECOVER;
			break;
		case '?':
		default:
			return (usage("unknown option"));
		}
	printf("Running %d: %s ", pid, argv[0]);
	for (i = 1; i != argc; i++)
		printf("%s ", argv[i]);
	printf("\n");
	argc -= optind;
	argv += optind;

	if (init) {
		/* Prevent accidentally rm -rf of a full path, etc. */
		if (Home[0] == '/' || Home[0] == '.')
			return (usage("-I accepts only local path names (prevents rm -r /...)"));
		snprintf(syscmd, sizeof(syscmd),
		    "rm -rf %s ; mkdir %s", Home, Home);
		printf("Clearing out env with \"%s\"\n", syscmd);
		if ((ret = system(syscmd)) != 0) {
			fatal(syscmd, errno);
			/* NOTREACHED */
			return (EXIT_TEST_ABORTED);
		}
	}
	if (Nreaders + Nwriters == 0 && !do_failchk)
		usage("Nothing specified to do?");

	srand(pid | time(NULL));

	/*
	 * Close down the env cleanly on an interrupt, except when running in
	 * the background. Catch SIGTERM to exit with a distinctive status.
	 */
	if ((sig = signal(SIGINT, onint)) != SIG_DFL)
		(void)signal(SIGINT, sig);
	(void)signal(SIGTERM, onint);

	/* Build the key list. */
	word();

	/* Set when this run will end, if not interrupted. */
	if (StatsInterval > Duration)
		StatsInterval = Duration;
	time(&EndTime);
	EndTime += Duration;
	
	/* Initialize the database environment. */
	if ((ret = db_init(&DbEnv, EnvOpenFlags)) != 0)
		return (ret);
	EnvOpenFlags &= ~DB_RECOVER;

	/*
	 * Create thread ID structures.	It starts with the readers and writers,
	 * then the trickle, deadlock and possibly failchk threads.
	 */
	Nthreads = Nreaders + Nwriters + 2;
	if ((tids = malloc((Nthreads + do_failchk) * sizeof(pthread_t))) == NULL)
		fatal("malloc threads", errno);

	/*
	 * Create failchk thread first; it might be needed during db_create.
	 * Put it at the end of the threads array, so that in doesn't get in
	 * the way of the worker threads.
	 */
	if (do_failchk &&
	    (ret = pthread_create(&tids[Nthreads], NULL, failchk, &i)) != 0)
		fatal("pthread_create failchk", errno);

	/* Initialize the database. */
	if ((ret = db_create(&Dbp, DbEnv, 0)) != 0) {
		DbEnv->err(DbEnv, ret, "db_create");
		(void)DbEnv->close(DbEnv, 0);
		return (EXIT_TEST_ABORTED);
	}
	if ((ret = Dbp->set_pagesize(Dbp, 1024)) != 0) {
		Dbp->err(Dbp, ret, "set_pagesize");
		goto err;
	}

	if ((ret = DbEnv->txn_begin(DbEnv, NULL, &txnp, 0)) != 0)
		fatal("txn_begin", ret);
	if ((ret = Dbp->open(Dbp, txnp,
	     DATABASE, NULL, DB_BTREE, DB_CREATE | DB_THREAD, 0664)) != 0) {
		Dbp->err(Dbp, ret, "%s: open", DATABASE);
		goto err;
	} else {
		ret = txnp->commit(txnp, 0);
		txnp = NULL;
		if (ret != 0)
			goto err;
	}

	ActiveThreads = Nthreads;

	/* Create statistics structures, offset by 1. */
	if ((perf = calloc(Nreaders + Nwriters + 1, sizeof(*perf))) == NULL)
		fatal("calloc statistics", errno);

	/* Create reader/writer threads. */
	for (i = 0; i < Nreaders + Nwriters; ++i)
		if ((ret = pthread_create(
		    &tids[i], NULL, tstart, (void *)(uintptr_t)i)) != 0)
			fatal("pthread_create", ret > 0 ? ret : errno);

	/* Create buffer pool trickle thread. */
	if (pthread_create(&tids[i], NULL, trickle, &i))
		fatal("pthread_create trickle thread", errno);
	++i;

	/* Create deadlock detector thread. */
	if (pthread_create(&tids[i], NULL, deadlock, &i))
		fatal("pthread_create deadlock thread", errno);
	++i;

	/* Wait for the worker, trickle and deadlock threads. */
	for (i = 0; i < Nthreads; ++i) {
		printf("joining thread %d...\n", i);
		if ((ret = pthread_join(tids[i], &retp)) != 0)
			fatal("pthread_join", ret);
		ActiveThreads--;
		printf("join thread %d done, %d left\n", i, ActiveThreads);
	}

	printf("Exiting\n");
	stats();

	if (!Failchk) {
err:		if (txnp != NULL)
			ret = txnp->abort(txnp);
		if (ret == 0 && Dbp != NULL)
			ret = Dbp->close(Dbp, 0);
		if (PrintStats)
			DbEnv->stat_print(DbEnv,
			    DB_STAT_SUBSYSTEM | DB_STAT_ALL);
		if (ret == 0 && DbEnv != NULL)
			ret = DbEnv->close(DbEnv, 0);
	}

	return (ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}

int
reader(id)
	int id;
{
	DBT key, data;
	int n, ret;
	char buf[100];

	/*
	 * DBT's must use local memory or malloc'd memory if the DB handle
	 * is accessed in a threaded fashion.
	 */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));
	data.flags = DB_DBT_MALLOC;

	/*
	 * Read-only threads do not require transaction protection, unless
	 * there's a need for repeatable reads.
	 */
	while (!Quit) {
		/* Pick a key at random, and look it up. */
		n = rand() % Nlist;
		key.data = List[n];
		key.size = strlen(key.data);

		if (Verbose)
			DbEnv->errx(DbEnv, "reader: %d: list entry %d", id, n);

		switch (ret = Dbp->get(Dbp, NULL, &key, &data, 0)) {
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
			sprintf(buf, "reader %d: dbp->get of %s",
			    id, (char *)key.data);
			fatal(buf, ret);
		}
	}
	return (0);
}

int
writer(id)
	int id;
{
	DBT key, data;
	DB_TXN *tid;
	int n, ret;
	char buf[100], dbuf[10000];


	/*
	 * DBT's must use local memory or malloc'd memory if the DB handle
	 * is accessed in a threaded fashion.
	 */
	memset(&key, 0, sizeof(DBT));
	memset(&data, 0, sizeof(DBT));
	data.data = dbuf;
	data.ulen = sizeof(dbuf);
	data.flags = DB_DBT_USERMEM;

	while (!Quit) {
		/* Pick a random key. */
		n = rand() % Nlist;
		key.data = List[n];
		key.size = strlen(key.data);

		if (Verbose)
			DbEnv->errx(DbEnv, "writer: %d: list entry %d", id, n);

		/* Abort and retry. */
		if (0) {
retry:			if ((ret = tid->abort(tid)) != 0)
				fatal("DB_TXN->abort", ret);
			++perf[id].aborts;
			++perf[id].aborted;
		}

		/* Begin the transaction. */
		if ((ret = DbEnv->txn_begin(DbEnv, NULL, &tid, 0)) != 0)
			fatal("txn_begin", ret);

		/*
		 * Get the key.  If it doesn't exist, add it.  If it does
		 * exist, delete it.
		 */
		switch (ret = Dbp->get(Dbp, tid, &key, &data, 0)) {
		case DB_LOCK_DEADLOCK:
			goto retry;
		case 0:
			goto delete;
		case DB_NOTFOUND:
			goto add;
		default:
			snprintf(buf, sizeof(buf),
			    "writer %d: put %s", id, (char *)key.data);
			fatal(buf, ret);
			/* NOTREACHED */
		}

delete:		/* Delete the key. */
		switch (ret = Dbp->del(Dbp, tid, &key, 0)) {
		case DB_LOCK_DEADLOCK:
			goto retry;
		case 0:
			++perf[id].deletes;
			goto commit;
		}

		snprintf(buf, sizeof(buf), "writer: %d: dbp->del", id);
		fatal(buf, ret);
		/* NOTREACHED */

add:		/* Add the key.  1 data item in 30 is an overflow item. */
		data.size = 20 + rand() % 128;
		if (rand() % 30 == 0)
			data.size += 8192;

		switch (ret = Dbp->put(Dbp, tid, &key, &data, 0)) {
		case DB_LOCK_DEADLOCK:
			goto retry;
		case 0:
			++perf[id].adds;
			goto commit;
		default:
			snprintf(buf, sizeof(buf), "writer: %d: dbp->put", id);
			fatal(buf, ret);
		}

commit:		/* The transaction finished, commit it. */
		if ((ret = tid->commit(tid, 0)) != 0)
			fatal("DB_TXN->commit", ret);

		/*
		 * Every time the thread completes many transactions, show
		 * our progress.
		 */
		if (++perf[id].txns % TxnInterval == 0) {
			DbEnv->errx(DbEnv,
"writer: %2d: adds: %4d: deletes: %4d: aborts: %4d: txns: %4d",
			    id, perf[id].adds, perf[id].deletes,
			    perf[id].aborts, perf[id].txns);
		}

		/*
		 * If this thread was aborted more than 5 times before
		 * the transaction finished, complain.
		 */
		if (perf[id].aborted > 5) {
			DbEnv->errx(DbEnv,
"writer: %2d: adds: %4d: deletes: %4d: aborts: %4d: txns: %4d: ABORTED: %2d",
			    id, perf[id].adds, perf[id].deletes,
			    perf[id].aborts, perf[id].txns, perf[id].aborted);
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
	int id;
	char *p, buf[8192];

	p = buf + sprintf(buf, "-------------\n");
	for (id = 0; id < Nreaders + Nwriters;)
		if (id++ < Nwriters)
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

	printf("%s", buf);
}

/*
 * util_annotate -
 *	Obtain timestamp, thread id, etc; prepend to messages.
 */
const char *
util_annotate(dbenv, header, header_size)
    const DB_ENV *dbenv;
    char *header;
    size_t header_size;
{
	struct timespec now;
	db_threadid_t tid;
	pid_t pid;
#ifdef HAVE_STRFTIME
	struct tm *tm_p;
#ifdef HAVE_LOCALTIME_R
	struct tm tm;
#endif
#endif
	char idstr[DB_THREADID_STRLEN], tmstr[20];

	if (dbenv == NULL) {
		snprintf(idstr, sizeof(idstr),
		    "Pid/tid %d:%p", getpid(), (void *)pthread_self());
	}
	else {
		dbenv->thread_id((DB_ENV *)dbenv, &pid, &tid);
		(void)dbenv->thread_id_string((DB_ENV *)dbenv, pid, tid, idstr);
	}

	__os_gettime(dbenv == NULL ? NULL : dbenv->env, &now, 0);
	/* Print the time readably if possible; else print seconds. */
#ifdef HAVE_STRFTIME
#ifdef HAVE_LOCALTIME_R
	tm_p = localtime_r(&now.tv_sec, &tm);
#else
	tm_p = localtime(&now.tv_sec);
#endif
	if (tm_p != NULL)
		(void)strftime(tmstr, sizeof(tmstr), "%H:%M:%S", tm_p);
	else
#endif
		(void)snprintf(tmstr, sizeof(tmstr), "%lu", (u_long)now.tv_sec);
	(void)snprintf(header, header_size, "%s.%06lu[%s]: ",
	    tmstr, (u_long)(now.tv_nsec / 1000), idstr);

	return (header);
}

/*
 * util_errcall -
 *	Annotate error messages with timestamp and thread id, + ???
 */
void
util_errcall(dbenv, errpfx, msg)
	const DB_ENV *dbenv;
	const char *errpfx;
	const char *msg;
{
	char header[UTIL_ANNOTATE_STRLEN];

	util_annotate(dbenv, header, sizeof(header));
	if (errpfx == NULL)
		errpfx = "";
	fprintf(stderr, "%s%s%s\n", header, errpfx, msg);
	fflush(stderr);
}

/*
 * util_msgcall -
 *	Annotate messages with timestamp and thread id, + ???
 */
void
util_msgcall(dbenv, msgpfx, msg)
	const DB_ENV *dbenv;
	const char *msgpfx;
	const char *msg;
{
	char header[UTIL_ANNOTATE_STRLEN];

	util_annotate(dbenv, header, sizeof(header));
	if (msgpfx == NULL)
		msgpfx = "";
	fprintf(stderr, "%s%s%s\n", header, msgpfx, msg);
	fflush(stderr);
}

/*
 * db_init --
 *	Initialize a TDS environment with failchk, running recovery if needed.
 *	The caller specifies additional flags such as:
 *		DB_THREAD | DB_CREATE | DB_REGISTER
 */
int
db_init(dbenvp, flags)
	DB_ENV **dbenvp;
	u_int32_t flags;
{
	DB_ENV *dbenv;
	int ret;

	dbenv = *dbenvp;
retry:
	if (dbenv != NULL) {
		dbenv->errx(dbenv, "Closing existing environment");
		*dbenvp = NULL;
		(void)dbenv->close(dbenv, 0);
	}
	if ((ret = db_env_create(&dbenv, 0)) != 0) {
		fprintf(stderr,
		    "%s: db_env_create: %s\n", Progname, db_strerror(ret));
		return (EXIT_TEST_ABORTED);
	}
	(void)dbenv->set_event_notify(dbenv, notice_event);
	if (Punish)
		(void)dbenv->set_flags(dbenv, DB_YIELDCPU, 1);

	/* Use errcall and msgcall functions to include threadid, timestamp. */
	(void)dbenv->set_errcall(dbenv, util_errcall);
	(void)dbenv->set_msgcall(dbenv, util_msgcall);

	/* Set a tiny cache. */
	(void)dbenv->set_cachesize(dbenv, 0, 100 * 1024, 0);
	(void)dbenv->set_lg_max(dbenv, 200000);
	(void)dbenv->set_verbose(dbenv, DB_VERB_RECOVERY, 1);
	(void)dbenv->set_verbose(dbenv, DB_VERB_REGISTER, 1);
	(void)dbenv->set_verbose(dbenv, DB_VERB_FILEOPS, 1);
	(void)dbenv->set_verbose(dbenv, DB_VERB_FILEOPS_ALL, 1);
	(void)dbenv->set_isalive(dbenv, say_is_alive);
	(void)dbenv->set_thread_count(dbenv, 100);

	flags |= DB_INIT_LOCK | DB_INIT_LOG | DB_INIT_MPOOL | DB_INIT_TXN |
	    DB_FAILCHK;
	if ((ret = dbenv->open(dbenv, Home, flags, 0)) == DB_RUNRECOVERY &&
	    !(flags & DB_RECOVER)) {
		dbenv->errx(dbenv, "About to run recovery in %s", Home);
		flags |= DB_RECOVER;
		goto retry;
	}
	if (ret != 0) {
		dbenv->err(dbenv, ret, "Could not open environment in %s", Home);
		*dbenvp = NULL;
		(void)dbenv->close(dbenv, 0);
		return (EXIT_TEST_ABORTED);
	}
	if (flags & DB_RECOVER && !(flags & DB_REGISTER)) {
		DB_TXN_STAT *txn_stat;
		if ((ret = dbenv->txn_stat(dbenv, &txn_stat, 0)) != 0)
			fatal("txn_stat after recovery failed", ret);
		if (txn_stat->st_nbegins != 0)
			fatal("txn_stat found txns, did recovery run?", 0);
		free(txn_stat);
	}
	*dbenvp = dbenv;
	dbenv->errx(dbenv, "Opened environment in %s", Home);
	if (!Verbose) {
		(void)dbenv->set_verbose(dbenv, DB_VERB_RECOVERY, 0);
		(void)dbenv->set_verbose(dbenv, DB_VERB_REGISTER, 0);
		(void)dbenv->set_verbose(dbenv, DB_VERB_FILEOPS, 0);
		(void)dbenv->set_verbose(dbenv, DB_VERB_FILEOPS_ALL, 0);
	}
	return (0);
}

/*
 * tstart --
 *	Thread start function for readers and writers.
 */
void *
tstart(arg)
	void *arg;
{
	pthread_t tid;
	u_int id;

	id = (uintptr_t)arg + 1;

	tid = pthread_self();

	if (id <= (u_int)Nwriters) {
		printf("write thread %d starting: tid: %lx\n", id, (u_long)tid);
		fflush(stdout);
		writer(id);
	} else {
		printf("read thread %d starting: tid: %lx\n", id, (u_long)tid);
		fflush(stdout);
		reader(id);
	}

	/* NOTREACHED */
	return (NULL);
}

/*
 * deadlock --
 *	Thread start function for DB_ENV->lock_detect.
 */
void *
deadlock(arg)
	void *arg;
{
	struct timeval t;
	pthread_t tid;
	int err;

	tid = pthread_self();

	printf("deadlock thread starting: tid: %lx\n", (u_long)tid);
	fflush(stdout);

	t.tv_sec = 0;
	t.tv_usec = 100000;
	while (!Quit) {
		err = DbEnv->lock_detect(DbEnv, 0, DB_LOCK_YOUNGEST, NULL);
		if (err != 0) {
			DbEnv->err(DbEnv, err, "lock_detect failed");
			break;
		}

		/* Check every 100ms. */
		(void)select(0, NULL, NULL, NULL, &t);
	}

	printf("%d deadlock thread exiting\n", getpid());
	COMPQUIET(arg, NULL);
	return (NULL);
}

/*
 * trickle --
 *	Thread start function for memp_trickle.
 */
void *
trickle(arg)
	void *arg;
{
	pthread_t tid;
	time_t now, then;
	int err, wrote;

	time(&now);
	then = now;
	tid = pthread_self();

	printf("trickle thread starting: tid: %lx\n", (u_long)tid);
	fflush(stdout);

	while (!Quit) {
		err = DbEnv->memp_trickle(DbEnv, 10, &wrote);
		if (err != 0) {
			DbEnv->err(DbEnv, err, "trickle failed");
			break;
		}
		if (Verbose)
			fprintf(stderr, "trickle: wrote %d\n", wrote);

		/*
		 * The trickle thread prints statistics every few seconds.
		 * It also checks whether it is time to quit.
		 */
		time(&now);
		if (now - then >= StatsInterval) {
			stats();
			then = now;
			if (now > EndTime) {
				printf("trickle: ending time reached @ %s",
					ctime(&now));
				Quit = 1;
			}
		}
		if (wrote == 0) {
			sleep(1);
			sched_yield();
		}
	}
	printf("%d trickle thread exiting\n", getpid());

	COMPQUIET(arg, NULL);
	return (NULL);
}

/*
 * failchk --
 *	Thread start function for failchk.
 */
void *
failchk(arg)
	void *arg;
{
	DB_ENV *failenv;
	pthread_t tid;
	time_t now;
	int err;

	tid = pthread_self();
	failenv = NULL;

	if (db_init(&failenv, 0) != 0) {
		fprintf(stderr, "failchk: environment open failed!\n");
		exit(EXIT_TEST_ABORTED);
	}
	(void)failenv->set_errpfx(failenv, "(failchk) ");

	failenv->errx(failenv, "starting tid: %lx\n", (u_long)tid);

	while (!Quit) {
		if ((err = failenv->failchk(failenv, 0)) != 0) {
			Failchk = 1;
			failenv->err(failenv, err, "failchk() returned");
			system("db_stat -Neh TESTDIR|egrep 'Creation|Failure'");
			/*
			 * Tell all threads to quit, then check that
			 * the environment can be reopened.
			 */
			Quit = 1;
			if (0) {
				do {
					sleep(10);
					if ((err = failenv->failchk(failenv, 0)) != 0)
						failenv->err(failenv, err,
						    "redo failchk with %d left returns",
						    ActiveThreads);
				} while (ActiveThreads > 0);
				fprintf(stderr,
				    "failchk: reopening %s with recovery\n", Home);
				(void)db_init(&failenv, DB_RECOVER);
				fprintf(stderr, "failchk: reopened %s\n", Home);
			}
			system("db_stat -eh TESTDIR | egrep 'Creat|Failure'");
			fprintf(stderr, "failchk thread exiting\n");
			exit(0);
		}
		sleep(1);
	}

	(void)failenv->close(failenv, 0);
	now = time(NULL);
	printf("failchk() thread returning @ %s", ctime(&now));

	COMPQUIET(arg, NULL);
	return (NULL);
}

/*
 * word --
 *	Build the dictionary word list
 */
void
word()
{
	FILE *fp;
	int cnt;
	char buf[256], *nl;

	if ((fp = fopen(WORDLIST, "r")) == NULL)
		fatal(WORDLIST, errno);

	if ((List = malloc(Nlist * sizeof(char *))) == NULL)
		fatal("malloc word list", errno);

	for (cnt = 0; cnt < Nlist; ++cnt) {
		if (fgets(buf, sizeof(buf), fp) == NULL)
			break;
		/* Newlines in the data make for confusing messages. */
		if ((nl = strrchr(buf, '\n')) != NULL)
			*nl = '\0';
		if ((List[cnt] = strdup(buf)) == NULL)
			fatal("strdup word", errno);
	}
	Nlist = cnt;	/* In case nlist was larger than the word list. */
}

/*
 * fatal --
 *	Report a fatal error and quit.
 */
void
fatal(msg, err)
	const char *msg;
	int err;
{
	char buf[1000];
	char header[UTIL_ANNOTATE_STRLEN];
	int ret;

	snprintf(buf, sizeof(buf), "pid %d %s: %s", getpid(), Progname, msg);
	if (err != 0)
		snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), ": %s%s",
		    db_strerror(err), MutexDied > 0 ? 
			    " after seeing DB_EVENT_MUTEX_DIED" : 
			    (Failchk > 0 ? "after seeing a failchk panic" : ""));
	/* fatal errors are 'ok' if a failchk-detected panic has occurred. */
	ret = (Failchk == 0 && MutexDied == 0 ? EXIT_TEST_ABORTED : EXIT_FAILURE);
	util_annotate(NULL, header, sizeof(header));
	fprintf(stderr, "%s%s\n", header, buf);

	exit(ret);

	/* NOTREACHED */
}

/*
 * usage --
 *	Usage message.
 */
int
usage(msg)
	const char *msg;
{
	(void)fprintf(stderr,
	    "usage: %s "
	    "[-p<Punish>] [-v<Verbose>] [-R<avoid DB_REGISTER>] [-f<run a failchk thread>] [-I <initialize>] [-x <always recover>]\n\t"
	    "[-d <duration(%d seconds)]\n\t"
	    "[-h <home(%s)>]\n\t"
	    "[-n <words(%d)>]\n\t"
	    "[-r <#readers %d>]\n\t"
	    "[-s <statistics interval>]\n\t"
	    "[-t <txn progress interval>]\n\t"
	    "[-w <#writers %d>]\n\t%s\n",
	    Progname, Duration, Home, Nlist, Nreaders, Nwriters, msg);
	return (EXIT_TEST_ABORTED);
}

/*
 * onint --
 *	Interrupt signal handler.
 */
void
onint(signo)
	int signo;
{
	Quit = 1;
	fflush(stdout);
	printf("pid %d sees signal %d\n", getpid(), signo);
	if (signo == SIGTERM) {
		printf("pid %d exiting due to SIGTERM\n", getpid());
		exit(EXIT_FAILURE + 1);
	}	
}

/*
 * notice_event --
 *	Display the details of events.
 */
void notice_event(dbenv, event, info)
	DB_ENV *dbenv;
	u_int32_t event;
	void *info;
{
#ifdef DB_EVENT_MUTEX_DIED
	DB_EVENT_MUTEX_DIED_INFO *mtxdied;
#endif
#ifdef DB_EVENT_FAILCHK_PANIC
	DB_EVENT_FAILCHK_INFO *crashed;
#endif
	switch (event) {
	case DB_EVENT_PANIC:
		dbenv->err(dbenv, *(int *)info, "Notification: panic");
		break;
	case DB_EVENT_REG_ALIVE:
		dbenv->errx(dbenv, "DB_EVENT_REG_ALIVE pid %lu is still alive.",
		    (u_long)(*(pid_t *)info));
		break;
	case DB_EVENT_REG_PANIC:
		dbenv->err(dbenv, *(int *)info, "Notification: register panic");
		break;
#ifdef DB_EVENT_MUTEX_DIED
	case DB_EVENT_MUTEX_DIED:
		mtxdied = info;
		dbenv->errx(dbenv, "Notification: dead mutex:  %.*s",
		    sizeof(mtxdied->desc), mtxdied->desc);
		MutexDied++;
		break;
#endif
#ifdef DB_EVENT_FAILCHK_PANIC
	case DB_EVENT_FAILCHK_PANIC:
		crashed = info;
		dbenv->errx(dbenv, "Notification: panic \"%s\" after: %s",
		    db_strerror(crashed->error), crashed->symptom);
		Failchk++;
		break;
#endif
	default:
		dbenv->errx(dbenv, "Event %u info %p", event, info);
		break;
	}
}

/*
 * say_is_alive - failchk is_alive function
 *
 * Return 1 if the pid is alive, else 0 (dead).
 *
 * We are alive, so is our parent and any other process to which we can
 * send a null signal (*IX) or get info about (Win32). Posix doesn't provide
 * a true way to detect whether another process' threads are active.
 */
int
say_is_alive(dbenv, pid, tid, flags)
	DB_ENV *dbenv;
	pid_t pid;
	db_threadid_t tid;
	u_int32_t flags;
{
#ifdef DB_WIN32
	HANDLE proc;
	LONG exitCode;
	int still_active;
#else
	int ret;
#endif

#ifdef DB_WIN32
	/* OpenProcess() may return a handle to a dead process, so check
	 * whether the process exists as well as whether it has just
	 * recently exited. This fails to detect processes that
	 * explicitly return STILL_ACTIVE as its exit status.
	 */
	if ((proc = OpenProcess(PROCESS_QUERY_INFORMATION, 0, pid)) != 0) {
		still_active = GetExitCodeProcess(proc, &exitCode) != 0 &&
		    exitCode == STILL_ACTIVE;
		CloseHandle(proc);
		if (still_active)
			return (1);
	}
#else
	/* Self, parent, and processes findable by kill are alive. */
	if (pid == getpid() || pid == getppid() ||
	    kill(pid, 0) == 0 || (ret = errno) != ESRCH)
		return (1);
	ret = errno;
#endif
	dbenv->err(dbenv, ret, "is-alive probe for pid %d", pid);
	COMPQUIET(dbenv, NULL);
	COMPQUIET(tid, 0);
	COMPQUIET(flags, 0);

	return (0);
}

