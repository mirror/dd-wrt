/*
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

#ifndef __PRIORITY_BDB_FILTERS_H
#define __PRIORITY_BDB_FILTERS_H

#include "db.h"

struct __bdb_msg_filter;	typedef struct __bdb_msg_filter BDB_MSG_FILTER;
struct __bdb_msg_filters;	typedef struct __bdb_msg_filters BDB_MSG_FILTERS;

/*
 * Filter used by the program the receives the messages from the
 * Message Generator.
 */
struct __bdb_msg_filter {
    int num_dbs; /* Number of entires in dbs and db_names. */
    char **db_names; /* Names of the databases. */
    const char *env_home;/* Path to the environment. */
    DB_ENV *dbenv; /* Environment that holds the databases. */
    DB **dbs; /* List of databases and message queues. */
    /*
     * Filter that the message receiving module uses to decide whether and
     * where to forward the messages.
     */
    int (*filter) (BDB_MSG_FILTER *bdb_filter,
	DB_ENV *msg_queue_env, DB *msg_queue, DBT *msg);
};

/* List of filters to pass to the sorting threads. */
struct __bdb_msg_filters {
    int num_filters;
    BDB_MSG_FILTER **filters;
};

/*
 * Receives messages and forwards them to different modules based on the passed
 * filters.
 */
int receive_messages(BDB_MSG_FILTERS *filters);

#endif
