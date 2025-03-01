/*
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

#ifndef __TOLL_BDB_FILTERS_H
#define __TOLL_BDB_FILTERS_H

#include "db.h"
#include "sqlite3.h"

#define NUM_FILTERS 3

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
    DB *dbp; /* A Berkeley DB database. */
    DB_SEQUENCE *seq; /* A Berkeley DB sequence. */
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

/* Filter implementations. */
int stolen(BDB_EVT_FILTER *bdb_filter, DB_ENV *evt_queue_env, DB *evt_queue, DBT *evt);
int billing(BDB_EVT_FILTER *bdb_filter, DB_ENV *evt_queue_env, DB *evt_queue, DBT *evt);
int traffic(BDB_EVT_FILTER *bdb_filter, DB_ENV *evt_queue_env, DB *evt_queue, DBT *evt);
/* Setup functions. */
int stolen_setup(BDB_EVT_FILTER *bdb_filter);
int billing_setup(BDB_EVT_FILTER *bdb_filter);
int traffic_setup(BDB_EVT_FILTER *bdb_filter);

/* Teardown functions. */
void stolen_teardown(BDB_EVT_FILTER *bdb_filter);
void billing_teardown(BDB_EVT_FILTER *bdb_filter);
void traffic_teardown(BDB_EVT_FILTER *bdb_filter);

#endif

