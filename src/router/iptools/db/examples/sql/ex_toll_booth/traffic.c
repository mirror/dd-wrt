/*-
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

/*
 * Automatic Toll Booth Event Processing Example
 *
 * This application simulates an automated highway toll booth.  The application
 * receives a data stream of licence plates and timestamps of cars passing
 * through the toll booth.  The application uses this information to perform
 * billing, send traffic alerts, and check for stolen cars.
 */

/*
 * Heavy Traffic Alert
 *
 * This is an aggregated event filter that sends traffic alerts if a certain
 * number of cars pass the toll booth in an hour.  It does this by using a
 * Berkeley DB sequence to record how many cars have passed the toll booth.
 *
 * A Berkeley DB sequence is a persistent object that returns an increasing or
 * decreasing series of integers.  It can be configured to have transactional
 * properties.
 */

#include "toll_common_utils.h"
#include "toll_bdb_filters.h"

#define SEC_IN_A_HOUR 3600
#define HEAVY_TRAFFIC_THRESHOLD 1800
static int traffic_filter __P((BDB_EVT_FILTER *, DB_ENV *, DB *, DBT *));

/*
 * This function is an aggregated event filter that sends traffic alerts when
 * a certain number of cars pass the toll booth in an hour.
 */
static int traffic_filter(bdb_filter, evt_queue_env, evt_queue, evt)
    BDB_EVT_FILTER *bdb_filter;
    DB_ENV *evt_queue_env;
    DB *evt_queue;
    DBT *evt;
{
    DB_ENV *env;
    DB *traffic;
    DB_SEQUENCE *seq;
    DB_TXN *txn;
    DBT key, data;
    db_seq_t total_cars, prev_cars;
    char key_buf[KEY_LEN], data_buf[KEY_LEN];
    int ret, hour;

    ret = 0;
    traffic = bdb_filter->dbp;
    seq = bdb_filter->seq;
    env = bdb_filter->dbenv;
    txn = NULL;

    /*
     * Configure the key and data so that it uses a user defined buffer instead
     * of having the get function allocated memory to store the values.
     */
    memset(&key, 0, sizeof(DBT));
    key.data = key_buf;
    key.flags |= DB_DBT_USERMEM;
    key.ulen = KEY_LEN;
    memset(&data, 0, sizeof(DBT));
    data.data = data_buf;
    data.flags |= DB_DBT_USERMEM;
    data.ulen = KEY_LEN;

    /* Get current hour from the timestamp in the event. */
    memcpy(&hour, (char *)evt->data + sizeof(int), sizeof(int));
    hour = hour / SEC_IN_A_HOUR;

    key.size = sizeof(int);
    memcpy(key.data, &hour, key.size);

    /*
     * To calculate how many cars have passed the toll booth this hour, first
     * get the number of cars that had passed the toll booth before the current
     * hour by reading it from a Berkeley DB btree database.  Then read the
     * total number of cars that have passed the toll booth by getting the
     * next value in the Berkeley DB sequence.  Then get the number of cars
     * that have passed the toll booth this hour by subtracting the total
     * number of cars from the number of cars before the current hour.
     */
retry:
    /* Begin a transaction. */
    if ((ret = env->txn_begin(env, NULL, &txn, 0)) != 0)
	goto err;
    ret = traffic->get(traffic, txn, &key, &data, 0);
    if (ret == 0) {
	/* Get the total number of cars from the sequence. */
	if ((ret = seq->get(seq, txn, 1, &total_cars, 0)) != 0)
	    goto err;
	/* Commit the transaction. */
	if ((ret = txn->commit(txn, 0)) != 0)
	    goto err;
	txn = NULL;
	data.size = sizeof(db_seq_t);
	memcpy(&prev_cars, data.data, data.size);
    /*
     * If the current hour has not been recorded yet, add hour
     * and the total number of cars to the database.
     */
    } else if (ret == DB_NOTFOUND) {
	/* Get the total number of cars from the sequence. */
	if ((ret = seq->get(seq, txn, 1, &total_cars, 0)) != 0)
	    goto err;
	data.size = sizeof(db_seq_t);
	memcpy(data.data, &total_cars, data.size);
	/* Insert the new hour and the number of cars into the database. */
	if ((ret = traffic->put(traffic, txn, &key, &data, 0)) != 0)
	    goto err;
	/* Commit the transaction. */
	if ((ret = txn->commit(txn, 0)) != 0)
	    goto err;
	txn = NULL;
	prev_cars = total_cars;
    } else
	goto err;

    /*
     * If the number of cars that have passed the toll booth in this hour
     * reaches the heave traffic threshold, send out the traffic alerts.
     */
    if ((total_cars - prev_cars) == HEAVY_TRAFFIC_THRESHOLD) {
	printf("Heavy traffic alert sent at hour %d.\n", hour + 1);
	/* Send Alerts. */
    }

    return (0);
err:
    if (txn != NULL)
	(void)txn->abort(txn);
    /*
     * If the error was deadlock, retry the operation after aborting the
     * transaction to solve the deadlock.
     */
    if (ret == DB_LOCK_DEADLOCK)
	goto retry;
    return (ret);
}

/*
 * Set up the filter by creating the in-memory environment, database, and
 * sequence.
 */
int traffic_setup(bdb_filter)
    BDB_EVT_FILTER *bdb_filter;
{
    int ret;
    bdb_filter->db_name = TRAFFIC;
    bdb_filter->filter = traffic_filter;
    bdb_filter->env_home = TRAFFIC_ENV;

    /*
     * Create the environment, database, and sequence, all as free threaded
     * and in-memory.
     */
    if ((ret = open_env(
	&(bdb_filter->dbenv), bdb_filter->env_home, 1, 1, 1)) != 0)
	goto err;
    if ((ret = open_btree(bdb_filter->dbenv, 
	&(bdb_filter->dbp), bdb_filter->db_name, 1, 1, 1, 0)) != 0)
	goto err;
    if ((ret = open_seq(
	bdb_filter->dbenv, bdb_filter->dbp,
	&(bdb_filter->seq), SEQUENCE, 1, 1)) != 0)
	goto err;

    return (0);
err:
    return (ret);
}

/*
 * Teardown the traffic filter by closing the environment, database, and
 * sequence.
 */
void traffic_teardown(bdb_filter)
    BDB_EVT_FILTER *bdb_filter;
{
    /* Close the sequence. */
    if (bdb_filter->seq != NULL)
	(void)bdb_filter->seq->close(bdb_filter->seq, 0);

    /* Close the database. */
    if (bdb_filter->dbp != NULL)
	(void)bdb_filter->dbp->close(bdb_filter->dbp, 0);

    /* Close the environment. */
    if (bdb_filter->dbenv != NULL)
	(void)bdb_filter->dbenv->close(bdb_filter->dbenv, 0);
}
