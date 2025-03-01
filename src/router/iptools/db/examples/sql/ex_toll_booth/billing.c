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
 * Billing
 *
 * This is an event filter that handles the billing of cars that pass through
 * an automated toll booth.  When a car passes through the toll booth, this
 * filter looks up the car owner's information in a Berkeley DB SQL database.
 * It subtracts the toll from the owner's funds recorded in the database, and
 * sends out an alert to the owner if his or her funds are low.
 *
 */

#include "toll_common_utils.h"
#include "toll_bdb_filters.h"

#define TOLL 1.00
#define LOW_FUNDS 5.00

static int billing_filter __P((BDB_EVT_FILTER *, DB_ENV *, DB *, DBT *));

/*
 * This function bills the car owner when their car passes through the toll
 * booth, and sends out low funds alerts.  It also records how many cars
 * pass through the toll booth.
 */
static int billing_filter(bdb_filter, evt_queue_env, evt_queue, evt)
    BDB_EVT_FILTER *bdb_filter;
    DB_ENV *evt_queue_env;
    DB *evt_queue;
    DBT *evt;
{
    sqlite3 *sql;
    sqlite3_stmt *stmt;
    DB_SEQUENCE *seq;
    db_seq_t num_cars;
    const unsigned char *name, *email;
    char sql_stmt[128];
    double funds;
    int plate, ret;

    ret = 0;
    sql = bdb_filter->sql;
    seq = bdb_filter->seq;
    stmt = NULL;

    /* Get the plate number from the event. */
    memcpy(&plate, evt->data, sizeof(int));

retry:
    /* Start a transaction. */
    if ((ret = sqlite3_exec(sql, "BEGIN TRANSACTION", NULL, NULL, NULL)) != SQLITE_OK)
	goto err;

    /* Get the user's information from the database. */
    sprintf(sql_stmt, "select * from USERINFO where plate=%d;", plate);
    sqlite3_prepare_v2(sql, sql_stmt, (int)strlen(sql_stmt), &stmt, NULL);

    /* This will only return one result. */
    if ((ret = sqlite3_step(stmt)) != SQLITE_ROW)
	goto err_abort;

    name = sqlite3_column_text(stmt, 1);
    email = sqlite3_column_text(stmt, 2);
    funds = sqlite3_column_double(stmt, 3);

    if ((ret = sqlite3_finalize(stmt)) != SQLITE_OK)
	goto err_abort;
    stmt = NULL;

    /* Update the funds. */
    funds -= TOLL;
    sprintf(sql_stmt, "update USERINFO set funds=%f where plate=%d;", funds, plate);
    if ((ret = sqlite3_exec(sql, sql_stmt, NULL, NULL, NULL)) != SQLITE_OK)
	goto err_abort;

    /* Commit the transaction. */
    if ((ret = sqlite3_exec(sql, "COMMIT", NULL, NULL, NULL)) != SQLITE_OK)
	goto err;

    if (funds < LOW_FUNDS) {
	printf("Owner of plate number %d has low funds: $%4.2f.\n", plate, funds);
	/* Send Alert. */
    }

    /* Increament the car counter. */
    if ((ret = seq->get(seq, NULL, 1, &num_cars, 0)) != 0)
	goto err;

    return (0);
err_abort:
    /* Abort the transaction. */
    if (sqlite3_exec(sql, "ROLLBACK", NULL, NULL, NULL) != SQLITE_OK)
	goto err;
    /*
     * If the error was deadlock, retry the operation after aborting the
     * transaction to solve the deadlock.
     */
    if (ret == SQLITE_BUSY)
	goto retry;
err:
    return (ret);
}

/*
 * Set up the filter by creating the on-disk environment, database, and
 * sequence.
 */
int billing_setup(bdb_filter)
    BDB_EVT_FILTER *bdb_filter;
{
    int ret;
    
    bdb_filter->db_name = COUNTER;
    bdb_filter->filter = billing_filter;
    bdb_filter->env_home = BILLING_ENV;

    /*
     * Create the environment, sql database, database, and sequence, 
     * all as free threaded and on disk.
     */
    if ((ret = open_env(
	&(bdb_filter->dbenv), bdb_filter->env_home, 0, 1, 1)) != 0)
	goto err;
    if ((ret = open_sql(&bdb_filter->sql, BILLING, 1)) != 0)
	goto err;
    if ((ret = open_btree(bdb_filter->dbenv,
	&(bdb_filter->dbp), COUNTER, 0, 1, 1, 0)) != 0) 
	goto err;
    if ((ret = open_seq(bdb_filter->dbenv,
	bdb_filter->dbp,&(bdb_filter->seq), SEQUENCE, 1, 1)) != 0) 
	goto err;

    return (0);
err:
    return (ret);
}


/*
 * Teardown the billing filter by closing the environment, database, and
 * sequence.
 */
void billing_teardown(bdb_filter)
    BDB_EVT_FILTER *bdb_filter;
{
    /* Close the sequence. */
    if (bdb_filter->seq != NULL)
	(void)bdb_filter->seq->close(bdb_filter->seq, 0);

    /* Close the database. */
    if (bdb_filter->dbp != NULL)
	(void)bdb_filter->dbp->close(bdb_filter->dbp, 0);

    /* Close the SQL database. */
    if(bdb_filter->sql != NULL)
	(void)sqlite3_close_v2(bdb_filter->sql);

    /* Close the environment. */
    if (bdb_filter->dbenv != NULL)
	(void)bdb_filter->dbenv->close(bdb_filter->dbenv, 0);
}
