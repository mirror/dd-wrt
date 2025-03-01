/*
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

#ifndef __TOLL_COMMON_UTILS_H
#define __TOLL_COMMON_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "db.h"
#include "sqlite3.h"

/* Environment directory for the event queue. */
#define HOME "TollBoothExample"
/* Database name of the evnt queue. */
#define DB_NAME "evt_queue"

/* Database names of the billing, stolen, and traffic databases. */
#define BILLING "Billing/billing.dbsql"
#define BILLING_ENV "Billing"
#define COUNTER "counter"
#define STOLEN "stolen.db"
#define STOLEN_ENV "Stolen"
#define TRAFFIC "traffic.db"
#define TRAFFIC_ENV "Traffic"
#define SEQUENCE "seq"

/* Seconds in a day. */
#define SEC_IN_A_DAY 86400

/* Number of license plates stored in the Billing database. */
#define NUM_PLATES 100000

/* Open a Berkeley DB queue database. */
int open_queue(DB_ENV *dbenv, DB **queue, const char *name, int in_memory, int create, int threaded);
/* Open a Berkeley DB btree database. */
int open_btree(DB_ENV *dbenv, DB **btree, const char *name, int in_memory, int create, int threaded, int dups);
/* Open a Berkeley DB SQL database. */
int open_sql(sqlite3 **dbsql, const char *name, int create);
/* Open a Berkeley DB environment. */
int open_env(DB_ENV **env, const char *home, int in_memory, int create, int threaded);
/* Open a Berkeley DB sequence. */
int open_seq(DB_ENV *dbenv, DB *dbp, DB_SEQUENCE **sequence, const char *name, int create, int threaded);

#define MAX_THREAD 1028
#define EVT_LEN 128
#define KEY_LEN 25
#define EVT_EXTENT_SIZE 5242880
#define	NS_PER_MS 1000000/* Nanoseconds in a millisecond*/
#define	NS_PER_US 1000   /* Nanoseconds in a microsecond*/
#define NUM_THREADS 3

/*
* Platform dependent functions and definitions for handling mutexes,
* threads, and recording time.
*/
#ifdef _WIN32
#include <windows.h>
#include <sys/timeb.h>
#include <time.h>

extern int getopt(int, char * const *, const char *);

/* Implement a basic high resource timer with a POSIX interface for Windows.*/
struct timeval2 {
    time_t tv_sec;
    long tv_usec;
};

int gettimeofday(struct timeval2 *tv, struct timezone *tz);

typedef HANDLE          os_thread_t;
#define os_thread_id()  GetCurrentThreadId()
#define os_thread_create(pid, func, arg)                    \
    ((*pid = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, \
    arg, 0, NULL)) != NULL)
#define S_ISDIR(m) ((m) & _S_IFDIR)
#define	usleep(s)		Sleep((s))

/* Mutexes. */
typedef HANDLE mutex_t;
#define	mutex_init(m, attr)						   \
    (((*(m) = CreateMutex(NULL, FALSE, NULL)) != NULL) ? 0 : -1)
#define	mutex_lock(m)							   \
    ((WaitForSingleObject(*(m), INFINITE) == WAIT_OBJECT_0) ? 0 : -1)
#define	mutex_unlock(m)		(ReleaseMutex(*(m)) ? 0 : -1)
#else
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

/* Thread functions. */
typedef pthread_t       os_thread_t;
#define os_thread_id()  pthread_self()
#define os_thread_create(pid, func, arg)                    \
    (0 == pthread_create(pid, NULL, func, arg))

/* Mutexes. */
typedef pthread_mutex_t mutex_t;
#define	mutex_init(m, attr)	pthread_mutex_init((m), (attr))
#define	mutex_lock(m)		pthread_mutex_lock(m)
#define	mutex_unlock(m)		pthread_mutex_unlock(m)
#endif

/* Program should register the thread id for following join_threads(). */
void register_thread_id(os_thread_t pid); 
/* Join all registered threads. */
int join_threads();

#endif
