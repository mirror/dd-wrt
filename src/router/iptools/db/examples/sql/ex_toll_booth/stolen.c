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
 * Stolen Car Alert
 *
 * This event filter checks the license plate number of every car that passes
 * the toll booth against a Berkeley DB btree database of stolen cars.  If a
 * match is found then an alert is sent with the time and the plate number. It
 * also consumes the event if the car is stolen so that the car owner is not
 * billed.
 */

#include "toll_common_utils.h"
#include "toll_bdb_filters.h"

static int stolen_filter __P((BDB_EVT_FILTER *, DB_ENV *, DB *, DBT *));
/*
 * This function a filter that takes the license plate number of a car and
 * checks it against a Berkeley DB btree database of stolen cars.  If a match
 * is found it sends an alert and consumes the event.
 */
static int stolen_filter(bdb_filter, evt_queue_env, evt_queue, evt)
    BDB_EVT_FILTER *bdb_filter;
    DB_ENV *evt_queue_env;
    DB *evt_queue;
    DBT *evt;
{
    DB *stolen;
    DBT key, data;
    char key_buf[KEY_LEN], data_buf[KEY_LEN];
    int plate, time, ret;

    ret = 0;
    stolen = bdb_filter->dbp;

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

    /* Get current time from the timestamp in the event. */
    memcpy(&time, (char *)evt->data + sizeof(int), sizeof(int));

    /* Get the plate number from the event. */
    key.size = sizeof(int);
    memcpy(&plate, evt->data, key.size);
    memcpy(key.data, &plate, key.size);

    /*
     * Check the license plate number against the plate numbers stored in the
     * database.  If found send an alert and consume the event.  If not found
     * do nothing.
     */
    if ((ret = stolen->get(stolen, NULL, &key, &data, 0)) == 0) {
	printf("Stolen car with plate number %d passed the toll booth.\n",
	    plate);
	/* Send Alert. */
    } else if (ret != DB_NOTFOUND)
	goto err;

    return (0);
err:
    return (ret);
}

/* Set up the filter by opening the existing stolen car database. */
int stolen_setup(bdb_filter)
    BDB_EVT_FILTER *bdb_filter;
{
    int ret;
 
    bdb_filter->db_name = STOLEN;
    bdb_filter->filter = stolen_filter;
    bdb_filter->env_home = STOLEN_ENV;

    /*
     * Open the existing environment and database as on-disk and free threaded.
     */
    if ((ret = open_env(
	&(bdb_filter->dbenv), bdb_filter->env_home, 0, 0, 1)) != 0)
	goto err;
    if ((ret = open_btree(bdb_filter->dbenv, 
	&(bdb_filter->dbp), bdb_filter->db_name, 0, 0, 1, 0)) != 0)
	goto err;

    return (0);
err:
    return (ret);
}

/* Teardown the stolen filter by closing the environment and the database. */
void stolen_teardown(bdb_filter)
    BDB_EVT_FILTER *bdb_filter;
{
    /* Close the database. */
    if (bdb_filter->dbp != NULL)
	(void)bdb_filter->dbp->close(bdb_filter->dbp, 0);

    /* Close the environment. */
    if (bdb_filter->dbenv != NULL)
	(void)bdb_filter->dbenv->close(bdb_filter->dbenv, 0);
}
