/*-
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

#include "priority_common_utils.h"
#include "db.h"

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
    if ((ret = dbenv->set_cachesize(dbenv, 0, NUM_MESSAGES * MSG_LEN, 1)) != 0)
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
    DB *msg_queue;
    int ret;
    u_int32_t flags;
    const char *db_name, *file_name;

    msg_queue = NULL;
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

    if ((ret = db_create(&msg_queue, dbenv, 0)) != 0)
	goto err;

    /* The record size needs to be set when using a queue. */
    if ((ret = msg_queue->set_re_len(msg_queue, MSG_LEN)) != 0)
	goto err;

    /* 
     * Optionally, unfilled space at the end of the record can be filled
     * with the given character.
     */
    if ((ret = msg_queue->set_re_pad(msg_queue, 0)) != 0)
	goto err;

    /*
     * If the queue is not in memory then it will be allocated on disk
     * as needed in files equal to the given extent size.  The files will
     * be deleted when they become empty.  If no extent size is given then
     * the queue will be a single file that will always be equal to the
     * largest size the queue has ever been in its lifetime.
     */
    if (!in_memory) {
	if ((ret = msg_queue->set_q_extentsize(
	    msg_queue, MSG_EXTENT_SIZE)) != 0)
	    goto err;
    }

    /*
     * Open the queue.  In this program, if DB_CREATE is not set and the
     * database does not exist, then the program sleeps for half a second
     * before trying again to open the database.
     */
    do {
	ret = msg_queue->open(msg_queue,
	    NULL, file_name, db_name, DB_QUEUE, flags, 0);
	if (ret == ENOENT && !create)
	    usleep(500);
	else if (ret != 0)
	    goto err;
    } while (ret == ENOENT);

    *queue = msg_queue;
    return 0;
err:
    dbenv->err(dbenv, ret, "Error opening database %s: ", name);
    if (msg_queue != NULL)
	(void)msg_queue->close(msg_queue, 0);
    return ret;
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


