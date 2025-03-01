/*-
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

#include "toll_common_utils.h"
#include "db.h"

#define CACHESIZE 200 * 1024 * 1024;

/* Configure and open a Berkeley DB environment. */
int open_env(env, home, in_memory, create, threaded)
    DB_ENV **env;
    const char *home;
    int in_memory;
    int create;
    int threaded;
{
    DB_ENV *dbenv;
    u_int32_t flags;
    int ret;

    *env = NULL;
    dbenv = NULL;

    /*
     * Initialize the transactional, logging, locking systems
     * and the cache.
     */
    flags = DB_INIT_TXN|DB_INIT_LOG|DB_INIT_LOCK|DB_INIT_MPOOL;
    if (create)
	flags |= DB_CREATE;
    if (threaded)
	flags |= DB_THREAD;

    if ((ret = db_env_create(&dbenv, 0)) != 0) {
	printf("Error creating the BDB environment");
	return ret;
    }

    /*
     * Logging in memory will improve performance, but the system will not
     * be recoverable in case of a crash.
     */
    if (in_memory) {
	if ((ret = dbenv->log_set_config(
	    dbenv, DB_LOG_IN_MEMORY, 1)) != 0)
	    goto err;

	/*
	 * Logging in memory usually requires a log buffer that is
	 * larger than the 1 MB default.  If the log buffer is too
	 * small then the application could run out of room for new
	 * logs.
	 */
	if ((ret = dbenv->set_lg_bsize(dbenv, 20 * 1024 * 1024)) != 0)
	    goto err;
    } else {
	/*
	 * Configure the environment to automatically delete logs
	 * that are no longer needed for standard recovery.
	 */
	if ((ret = dbenv->log_set_config(
	    dbenv, DB_LOG_AUTO_REMOVE, 1)) != 0)
	    goto err;
    }

    /*
     * The cache should be large enough to hold the working data set of
     * the application.  When using in-memory databases the cache must be
     * large enough to hold the entire database, or else an out of memory
     * error can occur.
     */
    if ((ret = dbenv->set_cachesize(dbenv, 0, 100 * 1024 * 1024, 1)) != 0)
	goto err;

    /* Open the environment. */
    if ((ret = dbenv->open(dbenv, home, flags, 0)) != 0)
	goto err;

    *env = dbenv;

    return 0;

err:	printf("Error creating the BDB environment");
    if (dbenv != NULL)
	(void)dbenv->close(dbenv, 0);
    return ret;
}

/* Configure and open a Berkeley DB queue database. */
int open_queue(dbenv, queue, name, in_memory, create, threaded)
    DB_ENV *dbenv;
    DB **queue;
    const char *name;
    int in_memory;
    int create;
    int threaded;
{
    DB *evt_queue;
    int ret;
    u_int32_t flags;
    const char *db_name, *file_name;

    evt_queue = NULL;
    *queue = NULL;
    flags = DB_AUTO_COMMIT;
    if (create)
	flags |= DB_CREATE;
    if (threaded)
	flags |= DB_THREAD;
    db_name = file_name = NULL;

    /* In memory databases are faster but not persistent. */
    if (in_memory)
	db_name = name;
    else
	file_name = name;

    if ((ret = db_create(&evt_queue, dbenv, 0)) != 0)
	goto err;

    /* The record size needs to be set when using a queue. */
    if ((ret = evt_queue->set_re_len(evt_queue, EVT_LEN)) != 0)
	goto err;

    /* 
     * Optionally, unfilled space at the end of the record can be filled
     * with the given character.
     */
    if ((ret = evt_queue->set_re_pad(evt_queue, 0)) != 0)
	goto err;

    /*
     * If the queue is not in memory then it will be allocated on disk
     * as needed in files equal to the given extent size.  The files will
     * be deleted when they become empty.  If no extent size is given then
     * the queue will be a single file that will always be equal to the
     * largest size the queue has ever been in its lifetime.
     */
    if (!in_memory) {
	if ((ret = evt_queue->set_q_extentsize(
	    evt_queue, EVT_EXTENT_SIZE)) != 0)
	    goto err;
    }

    /*
     * Open the queue.  In this program, if DB_CREATE is not set and the
     * database does not exist, then the program sleeps for half a second
     * before trying again to open the database.
     */
    do {
	ret = evt_queue->open(evt_queue,
	    NULL, file_name, db_name, DB_QUEUE, flags, 0);
	if (ret == ENOENT && !create)
	    usleep(500);
	else if (ret != 0)
	    goto err;
    } while (ret == ENOENT);

    *queue = evt_queue;
    return 0;
err:
    dbenv->err(dbenv, ret, "Error opening database %s: ", name);
    if (evt_queue != NULL)
	(void)evt_queue->close(evt_queue, 0);
    return ret;
}

/* Configure and open a Berkeley DB btree database. */
int open_btree(dbenv, btree, name, in_memory, create, threaded, dups)
    DB_ENV *dbenv;
    DB **btree;
    const char *name;
    int in_memory;
    int create;
    int threaded;
    int dups;
{
    DB *dbp;
    int ret;
    u_int32_t flags;
    const char *db_name, *file_name;

    dbp = NULL;
    *btree = NULL;
    flags = DB_AUTO_COMMIT;
    if (create)
	flags |= DB_CREATE;
    if (threaded)
	flags |= DB_THREAD;
    db_name = file_name = NULL;

    /* In memory databases are faster but not persistent. */
    if (in_memory)
	db_name = name;
    else
	file_name = name;

    if ((ret = db_create(&dbp, dbenv, 0)) != 0)
	goto err;

    /*
     * Btree databases can be configured to allow multiple data items
     * for each unique key.  These duplicate data entries can be sorted
     * or unsorted.
     */
    if (dups) {
	if ((ret = dbp->set_flags(dbp, DB_DUPSORT)) != 0)
	    goto err;
    }

    /*
     * Open the btree.  In this program, if DB_CREATE is not set and the
     * database does not exist, then the program sleeps for half a second
     * before trying again to open the database.
     */
    do {
	ret = dbp->open(dbp,
	    NULL, file_name, db_name, DB_BTREE, flags, 0);
	if (ret == ENOENT && !create)
	    usleep(500);
	else if (ret != 0)
	    goto err;
    } while (ret == ENOENT);

    *btree = dbp;
    return 0;
err:
    dbenv->err(dbenv, ret, "Error opening database %s: ", name);
    if (dbp != NULL)
	(void)dbp->close(dbp, 0);
    return ret;
}

int open_sql(dbsql, name, create)
    sqlite3 **dbsql;
    const char *name;
    int create;
{
    sqlite3 *sql;
    int flags, ret;

    flags = SQLITE_OPEN_READWRITE;
    sql = NULL;
    *dbsql = NULL;

    if (create)
	flags |= SQLITE_OPEN_CREATE;

    if ((ret = sqlite3_open_v2(name, &sql, flags, NULL)) != SQLITE_OK)
	goto err;

    if ((ret = sqlite3_exec(
	sql, "PRAGMA cache_size=8000;", NULL, NULL, NULL)) != SQLITE_OK)
	goto err;

    *dbsql = sql;
    return (0);

err:if (sql != NULL)
	sqlite3_close_v2(sql);

    return (ret);
}

/*
 * Configure and open a Berkeley DB sequence.
 * A Berkeley DB sequence is a persistent object that returns an ever
 * increasing or decreasing series of integers.
 */
int open_seq(dbenv, dbp, sequence, name, create, threaded)
    DB_ENV *dbenv;
    DB *dbp;
    DB_SEQUENCE **sequence;
    const char *name;
    int create;
    int threaded;
{
    DB_SEQUENCE *seq;
    DBT key;
    int ret;
    u_int32_t flags;

    flags = 0;
    seq = NULL;
    *sequence = NULL;
    memset(&key, 0, sizeof(DBT));

    /* A Berkeley DB sequence is stored in an existing Berkeley DB database. */
    if ((ret = db_sequence_create(&seq, dbp, flags)) != 0)
	goto err;

    /*
     * Open or create the sequence.  The key can be used to find the sequence
     * again after closing the sequence handle.
     */
    key.flags = DB_DBT_USERMEM;
    key.data = (char *)name;
    key.size = (u_int32_t)strlen(name) + 1;
    key.ulen = key.size;
    /* Make the sequence thread safe. */
    if (threaded)
	flags |= DB_THREAD;
    /* Create the sequence. */
    if (create)
	flags |= DB_CREATE;
    if ((ret = seq->open(seq, NULL, &key, flags)) != 0)
	goto err;

    *sequence = seq;
    return (0);
err:
    if (seq != NULL)
	(void)seq->close(seq, 0);
    return (ret);
}


/* Define gettimeofday and snprintf for Windows. */
#ifdef _WIN32
#define snprintf sprintf_s

int gettimeofday(struct timeval2 *tv, struct timezone *tz)
{
    struct _timeb now;
    _ftime(&now);
    tv->tv_sec = now.time;
    tv->tv_usec = now.millitm * NS_PER_US;
    return (0);
}
#endif


/*
 * A very simple multi-threaded manager for concurrent examples.
 */
static os_thread_t thread_stack[MAX_THREAD];
static int pstack = 0;

/* All created thread ids will be pushed into stack for managing. */
void
register_thread_id(pid)
		os_thread_t pid;
{
	/* Push pid into stack. */
	if (pstack < MAX_THREAD) {
		thread_stack[pstack++] = pid;
	} else {
		fprintf(stderr, "Error: Too many threads!\n");
	}
}

/* Join for all threads in stack when finished. */
int
join_threads()
{ 
	int i;
	int status = 0;
#if defined(WIN32)
#else
	void *retp;
#endif

	for (i = 0; i < pstack ; i++) {
#if defined(WIN32)
		if (WaitForSingleObject(thread_stack[i], INFINITE) == WAIT_FAILED) {
			status = 1;
			printf("join_threads: child %ld exited with error\n", (long)i);
		}
#else
		pthread_join(thread_stack[i], &retp);
		if (retp != NULL) {
			status = 1;
			printf("join_threads: child %ld exited with error\n",
			(long)i);
		}
#endif
	}

	pstack = 0;
	return status;
}


