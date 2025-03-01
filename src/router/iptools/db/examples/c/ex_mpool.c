/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1997, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * ex_mpool -- 
 *	Use the memory pool API to demonstrate random access to a file.
 *
 *	This example fills a plain file (not a database) and performs
 *	random reads of it through the MPOOL file interface. It displays
 *	the read throughput in blocks and megabytes per second.
 *   
 * Source File:
 *	ex_mpool.c
 *
 * Data File:
 *	mpool
 *
 * Environment directory:
 *	The current directory. It configured for memory pools only. It does
 *	not need the Transactional or Concurrent Data store, or locking, so
 *	those subsystems are not enabled.
 *
 * Options:								default
 *	-cN	set the database cache size to N Kbytes.	       [20 (KB)]
 *	-hN	set hits - the number of random accesses to make       [10000]
 *	-k	keep the environment around after the program finishes [remove]
 *	-nN	Set the size of the file to N pages		       [50]
 *	-pN	Set the pagesize: a power of two from 512 to 65536     [1024]
 *
 * $Id$
 */

#include <sys/types.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

#else
#include <sys/time.h>
#include <unistd.h>
#endif

#include <db.h>

int	init __P((const char *, u_int32_t, int, const char *));
int	run __P((int, long long, u_int32_t, int, const char *));
int	main __P((int, char *[]));
int	usage __P((const char *));
#define	MPOOL	"mpool"					/* The data file. */

void	timer_start(struct timeval *start);
double	timer_end(const struct timeval *start);

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
 *	Example of mpool file access
 */
int
main(argc, argv)
	int argc;
	char *argv[];
{
	DB_ENV *dbenv;
	extern char *optarg;	/* From getopt(): the current option's value. */
	extern int optind;	/* From getopt(): the # of argv's processed. */
	long long cachesize;	/* -c<N> option: environment cachesize */
	u_int32_t pagesize;	/* -p<N> option: database pagesize */
	int ch;			/* The current command line option char. */
	int hits;		/* -h<N> option: environment cachesize */
	int keep;		/* -k: keep the environment, don't remove it. */
	int npages;		/* -n<N> option: environment cachesize */
	int ret;		/* Return code from the Berkley DB API calls */
	char *progname;		/* The program name, used for error messages */

	progname = "ex_mpool";
	/* Initialize the command line options to their default values. */
	hits = 10000;		/* Fetch 10,000 pages from the file. */
	npages = 50;		/* Size of the file: 50 pages of 1024 bytes. */
	pagesize = 1024;
	cachesize = 50 * 1024;	/* The default cache holds the default file. */
	keep = 0;		/* The default is to remove environment. */
	while ((ch = getopt(argc, argv, "c:h:kn:p:")) != EOF)
		switch (ch) {
		case 'c':
			if ((cachesize = atoi(optarg)) < 20)
				return (usage(progname));
			cachesize *= 1024;
			break;
		case 'h':
			if ((hits = atoi(optarg)) <= 0)
				return (usage(progname));
			break;
		case 'k':
			keep = 1;
			break;
		case 'n':
			if ((npages = atoi(optarg)) <= 0)
				return (usage(progname));
			break;
		case 'p':
			if ((pagesize = atoi(optarg)) <= 0)
				return (usage(progname));
			break;
		default:
			return (usage(progname));
		}
	argc -= optind;
	argv += optind;

	/* Ensure that cache can hold at least 1 page. */
	if (cachesize < pagesize)
		cachesize = pagesize;

	/* Create and fill the file with data. */
	if ((ret = init(MPOOL, pagesize, npages, progname)) != 0)
		return (EXIT_FAILURE);

	/* Open the environment, file, and randomly access its pages. */
	ret = run(hits, cachesize, pagesize, npages, progname);

	/* After a successful run, the environment is usually removed. */
	if (ret == 0 && !keep) {
		/*
		 * Remove the environment so that the the next run of this
		 * program can start afresh; otherwise if you specified a
		 * different cache size on the next run it would have no effect;
		 * that setting is used only when the environment does not
		 * already exist.
		 */
		if ((ret = db_env_create(&dbenv, 0)) != 0) {
			fprintf(stderr,
			    "%s: final db_env_create: %s\n", progname, db_strerror(ret));
			return (1);
		}
		if ((ret = dbenv->remove(dbenv, NULL, 0)) != 0) {
			fprintf(stderr,
			    "%s:  DB_ENV->remove: %s\n", progname, db_strerror(ret));
			return (1);
		}
	}

	return (ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE);
}

/*
 * usage --
 *	Display the command line options.
 */
int
usage(progname)
	const char *progname;
{
	(void)fprintf(stderr,
	    "usage: %s [-c <cachesize in Kbytes>] [-h <hits>] "
	    "[-k<eep>] [-n <npages>] [-p <pagesize>]\n",
	    progname);
	return (EXIT_FAILURE);
}


/*
 * init --
 *	Create a file and fill it with enough data so that each page can be
 *	distinguished from other pages.
 */
int
init(file, pagesize, npages, progname)
	const char *file;
	u_int32_t pagesize;
	int npages;
	const char *progname;
{
	FILE *fp;
	int cnt;
	char *p;

	/*
	 * Create a file with the right number of pages, and store a page
	 * number on each page.
	 */
	if ((fp = fopen(file, "wb")) == NULL) {
		fprintf(stderr,
		    "%s: open %s: %s\n", progname, file, strerror(errno));
		return (1);
	}
	if ((p = (char *)malloc(pagesize)) == NULL) {
		fprintf(stderr, "%s: malloc: %s\n", progname, strerror(errno));
		return (1);
	}

	/*
	 * The pages are numbered from 0, not 1.
	 *
	 * Write the index of the page at the beginning of the page in order
	 * to verify the retrieved page (see run()).
	 */
	for (cnt = 0; cnt < npages; ++cnt) {
		*(db_pgno_t *)p = cnt;
		if (fwrite(p, pagesize, 1, fp) != 1) {
			fprintf(stderr, "%s: %s fwrite @ %u: %s\n",
			    progname, file, cnt * pagesize, strerror(errno));
			return (1);
		}
	}

	(void)fclose(fp);
	free(p);
	return (0);
}

/*
 * run --
 *	Fetch 'hits' pages at random from the file, using mpool accesses.
 *	The mpool is made by creating and configuring an environment with
 *	DB_INIT_MPOOL. It verifies that the correct page number was obtained,
 *	in order to show how one looks at the data.
 */
int
run(hits, cachesize, pagesize, npages, progname)
	int hits;
	long long cachesize;
	u_int32_t pagesize;
	int npages;
	const char *progname;
{
	DB_ENV *dbenv;
	DB_MPOOLFILE *mfp;
	struct timeval start;
	db_pgno_t pageno;
	int cnt;
	int ret;		/* Return code from the Berkley DB API calls */
	void *p;
	double duration;

	/*
	 * Null out handles of resources; any non-null ones are closed before
	 * returning.
	 */
	dbenv = NULL;
	mfp = NULL;

	printf("%s: cachesize: %lld; pagesize: %d; %d pages in the file\n",
	    progname, cachesize, pagesize, npages);

	/* Create an environment handle and configure it, before opening it. */
	if ((ret = db_env_create(&dbenv, 0)) != 0) {
		fprintf(stderr,
		    "%s: db_env_create: %s\n", progname, db_strerror(ret));
		return (1);
	}
	/* Send error messages to stderr, preceded by the program name. */
	dbenv->set_errfile(dbenv, stderr);
	dbenv->set_errpfx(dbenv, progname);

#ifdef HAVE_VXWORKS
	/* VXWORKS needs to have a shared memory key. */
	if ((ret = dbenv->set_shm_key(dbenv, VXSHM_KEY)) != 0) {
		dbenv->err(dbenv, ret, "set_shm_key");
		return (1);
	}
#endif

	/*
	 * Set the size of the mpool cache. Gigabytes and bytes are separate
	 * parameters.
	 */
	if ((ret = dbenv->set_cachesize(dbenv,
	    (int)(cachesize >> 30), cachesize % (1 << 30), 1)) != 0) {
		dbenv->err(dbenv, ret, "set_cachesize(%lld)", cachesize);
		goto err_cleanup;
	}
	/*
	 * Setting the average mpool pagesize calculates some mpool parameters
	 * to fit that size, such as the number of mutexes and hash table slots.
	 */
	if ((ret = dbenv->set_mp_pagesize(dbenv, pagesize)) != 0) {
		dbenv->err(dbenv, ret, "set_mp_pagesize(%d)", pagesize);
		goto err_cleanup;
	}

	/* Open the environment, creating it in the filesystem. */ 
	if ((ret = dbenv->open(
	    dbenv, NULL, DB_CREATE | DB_INIT_MPOOL, 0)) != 0) {
		dbenv->err(dbenv, ret, "DB_ENV->open");
		goto err_cleanup;
	}

	/* Create the file handle and open it in the environment. */
	if ((ret = dbenv->memp_fcreate(dbenv, &mfp, 0)) != 0) {
		dbenv->err(dbenv, ret, "DB_ENV->memp_fcreate: %s", MPOOL);
		goto err_cleanup;
	}
	if ((ret = mfp->open(mfp, MPOOL, 0, 0, pagesize)) != 0) {
		dbenv->err(dbenv, ret, "DB_MPOOLFILE->open: %s", MPOOL);
		goto err_cleanup;
	}

	printf("retrieving %d pages at random... ", hits);
	fflush(stdout);
	srand((u_int)time(NULL));

	timer_start(&start);
	for (cnt = 0; cnt < hits; ++cnt) {
		/*
		 * Select a page number at random, get it, check its data,
		 * then 'let go' of the page with MFP->put().
		 */
		pageno = rand() % npages;
		if ((ret = mfp->get(mfp, &pageno, NULL, 0, &p)) != 0) {
			dbenv->err(dbenv, ret,
			    "unable to retrieve page %lu", (u_long)pageno);
			goto err_cleanup;
		}
		/* Verify the page's number that was written in init(). */
		if (*(db_pgno_t *)p != pageno) {
			dbenv->errx(dbenv, "wrong page retrieved (%lu != %d)",
			    (u_long)pageno, *(int *)p);
			goto err_cleanup;
		}
		if ((ret = mfp->put(mfp, p, DB_PRIORITY_UNCHANGED, 0)) != 0) {
			dbenv->err(dbenv, ret,
			    "unable to return page %lu", (u_long)pageno);
			goto err_cleanup;
		}
	}
	duration = timer_end(&start);

	printf("%d pages/second; %.0f MB/second.\n",
	    (int)(cnt / duration), cnt * (pagesize / 1000000.0) / duration);

	/* Close the file. */
	if ((ret = mfp->close(mfp, 0)) != 0) {
		dbenv->err(dbenv, ret, "DB_MPOOLFILE->close");
		goto err_cleanup;
	}

	/* Close the environment which contains the pool. */
	if ((ret = dbenv->close(dbenv, 0)) != 0) {
		fprintf(stderr,
		    "%s: db_env_create: %s\n", progname, db_strerror(ret));
		return (1);
	}

	return (0);

err_cleanup:
	if (mfp != NULL)
		(void)mfp->close(mfp, 0);
	if (dbenv != NULL)
		(void)dbenv->close(dbenv, 0);
	return (1);
}
