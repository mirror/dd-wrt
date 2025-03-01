/*
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

#ifndef __FIREWALL_COMMON_UTILS_H
#define __FIREWALL_COMMON_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "db.h"

/* Environment directory for the message queue. */
#define HOME "FirewallExample"
/* Database name of the message queue. */
#define DB_NAME "msg_queue"

#define KNOWN_SITES_SIZE 7
#define BLOCKED_SITES_SIZE 2
static const char * const KnownSites[] = {"EvilIP", "PhishingRUS", "GoodSite", "GoodSite1",
					    "GoodSite2", "GoodSite3", "GoodSite4"};

/* Open a Berkeley DB queue database. */
int open_queue(DB_ENV *dbenv, DB **queue, const char *name, int in_memory, int create, int threaded);
/* Open a Berkeley DB environment. */
int open_env(DB_ENV **env, const char *home, int in_memory, int create, int threaded);

#define MSG_LEN 128
#define KEY_LEN 25
#define MSG_EXTENT_SIZE 5242880
#define	NS_PER_MS 1000000/* Nanoseconds in a millisecond*/
#define	NS_PER_US 1000   /* Nanoseconds in a microsecond*/
#define NUM_MESSAGES	100000

/* Platform dependent functions and definitions. */
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

#define	usleep(s)		Sleep((s))
#else
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>
#endif

#endif
