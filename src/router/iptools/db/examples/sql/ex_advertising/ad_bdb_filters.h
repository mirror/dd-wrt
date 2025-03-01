/*
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

#ifndef __AD_BDB_FILTERS_H
#define __AD_BDB_FILTERS_H

#include "db.h"
#include "sqlite3.h"

#define NUM_FILTERS 1

struct __bdb_evt_filter;	typedef struct __bdb_evt_filter BDB_EVT_FILTER;
struct __bdb_evt_filters;	typedef struct __bdb_evt_filters BDB_EVT_FILTERS;

/*
 * Filter used by the program the receives the events from the
 * event generator.
 */
struct __bdb_evt_filter {
    char *db_name; /* Name of the database. */
    const char *env_home;/* Path to the environment. */
    DB_ENV *dbenv; /* Environment that holds the databases. */
    int num_dbs; /* Number of Berkeley DB databases. */
    DB **dbs; /* Berkeley DB databases. */
    DB_SEQUENCE *seq1; /* A Berkeley DB sequence. */
    DB_SEQUENCE *seq2; /* A Berkeley DB sequence. */
    sqlite3 *sql; /* A Berkeley DB SQL database. */
    /*
     * Filter that the event receiving module uses to decide whether and
     * where to forward the events.
     */
    int (*filter) (BDB_EVT_FILTER *bdb_filter,
	DB_ENV *evt_queue_env, DB *evt_queue, DBT *evt);
};

/* List of filters to pass to the sorting threads. */
struct __bdb_evt_filters {
    int num_filters;
    BDB_EVT_FILTER **filters;
};

/*
 * Receives events and forwards them to different modules based on the passed
 * filters.
 */
int receive_events(BDB_EVT_FILTERS *filters);

/* Setup functions. */
int advertise_setup(BDB_EVT_FILTER *bdb_filter);
/* Teardown functions. */
void advertise_teardown(BDB_EVT_FILTER *bdb_filter);

#endif
