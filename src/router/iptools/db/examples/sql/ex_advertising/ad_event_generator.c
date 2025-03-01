/*-
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

/*
 * Location Based Personalized Advertising Example
 *
 * This example shows a location based personalized advertising application.
 * It simulates a server receiving customer location's from their smart phones
 * and sends advertisements back to the phone based on the customer's location
 * and shopping preferences.  
 */

/*
 * Event Generator
 *
 * This program simulates GPS coordinates coming from customer's smart phones
 * by placing events in a Berkeley DB queue database that contain the user id
 * of the phone's owner and the owner's current location.
 */

#include <stdlib.h>
#include "ad_common_utils.h"
#include "db.h"

/*
 * Simulates GPS and customer ids arriving at a central server by continuously
 * creating events and inserting them into a BDB queue from which the event
 * processor reads.
 */
int send_evts(dbenv, event_queue)
    DB_ENV *dbenv;
    DB *event_queue;
{
    DBT key, data;
    unsigned char evt[EVT_LEN], key_buf[25];
    double lat, longi;
    int count, ret, user_id;

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
    count = ret = 0;
    /* A predictible random set. */
    srand(0);

    /* Create a stream of events. */
    while(count < (NUM_EVENTS * NUM_THREADS)) {
	/* Pick a random user to push a location to the server. */
	user_id = rand() % NUM_USERS;

	/* Random latitude and longitude in the United States. */
	lat = (rand() % 2000)/100.00 + 30.0;
	longi = (rand() % 5000)/100.00 + 70.0;

	/* Add the user id and location to the event. */
	memcpy(evt, &user_id, sizeof(int));
	memcpy(evt + sizeof(int), &lat, sizeof(double));
	memcpy(evt + (sizeof(int) + sizeof(double)), &longi, sizeof(double));

	/* Place the event at the end of the queue. */
	if ((ret = event_queue->put(
	    event_queue, NULL, &key, &data, DB_APPEND)) != 0)
	    return ret;
	count++;
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

/* Send events to the event processor. */
int main()
{
    return send_events();
}
