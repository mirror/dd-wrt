/*-
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

/*
 * Toll Booth Event Processing Example
 *
 * This application simulates an automated highway toll booth.  The application
 * receives a data stream of licence plates and timestamps of cars passing
 * through the toll booth.  The application uses this information to perform
 * billing, send traffic alerts, and check for stolen cars.
 */

/*
 * Event Generator
 *
 * This program simulates a day's worth of cars passing through the toll booth
 * by placing events in a Berkeley DB queue database that contain a licence
 * plate and a continuously increasing timestamp.
 */

#include <stdlib.h>
#include "toll_common_utils.h"
#include "db.h"

/*
 * Simulates a days worth of toll booth traffic by continuously creating
 * events and inserting them into a BDB queue from which the event processor
 * reads.
 */
int send_evts(dbenv, event_queue)
    DB_ENV *dbenv;
    DB *event_queue;
{
    DBT key, data;
    unsigned char evt[EVT_LEN], key_buf[25];
    int plate, ret, time;

    /*
     * Configure the key and data fields to use a user defined memory
     * buffer instead of having the database put function allocate one.
     */
    memset(evt, 0, EVT_LEN);
    memset(&data, 0, sizeof(DBT));
    memset(&key, 0, sizeof(DBT));
    data.flags |= DB_DBT_USERMEM;
    data.ulen = EVT_LEN;
    data.size = EVT_LEN;
    data.data = evt;
    key.flags |= DB_DBT_USERMEM;
    key.ulen = 25;
    key.data = key_buf;
    time = ret = 0;
    /* A predictible random set. */
    srand(0);

    /* Create a days worth of toll booth events. */
    while(time < (SEC_IN_A_DAY + (10 * NUM_THREADS))) {
	/* Pick a random car to pass through the toll booth. */
	plate = rand() % NUM_PLATES;

	/* A car passes through the toll booth at least every 5 seconds. */
	time += rand() % 5;

	/* Add the plate number and the time to the event. */
	memcpy(evt, &plate, sizeof(int));
	memcpy(evt + sizeof(int), &time, sizeof(int));

	/* Place the event at the end of the queue. */
	if ((ret = event_queue->put(
	    event_queue, NULL, &key, &data, DB_APPEND)) != 0)
	    return ret;
    }

    return ret;
}

/* Open the event queue and start adding events to it. */
int send_events()
{
    DB_ENV *dbenv;
    DB *event_queue;
    int ret;

    event_queue = NULL;
    dbenv = NULL;

    /*
     * Open the event queue as an in-memory Berkeley DB queue database
     * that uses in-memory logging.
     */
    if ((ret = open_env(&dbenv, HOME, 1, 1, 0)) != 0)
	goto err;
    if ((ret = open_queue(dbenv, &event_queue, DB_NAME, 1, 1, 0)) != 0)
	goto err;

    /* Add events to the queue. */
    ret = send_evts(dbenv, event_queue);

err:	if (event_queue != NULL)
	    (void)event_queue->close(event_queue, 0);
	if (dbenv != NULL)
	    (void)dbenv->close(dbenv, 0);

	return ret;
}

/* Send a day's worth of events to the event processor. */
int main()
{
    return send_events();
}
