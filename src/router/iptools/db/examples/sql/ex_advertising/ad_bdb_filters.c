/*
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

#include "ad_bdb_filters.h"
#include "ad_common_utils.h"


/*
 * Reads events from the queue and forwards or handles them based on
 * the given filters.
 */
int receive_evts(dbenv, evt_queue, filters)
    DB_ENV *dbenv;
    DB *evt_queue;
    BDB_EVT_FILTERS *filters;
{
    DBT key, data;
    BDB_EVT_FILTER *filter;
    unsigned char evt[EVT_LEN], key_buf[KEY_LEN];
    int count, i, ret;

    /*
     * Configure the key and data fields to use a user defined memory
     * buffer instead of having the database get function allocate one.
     */
    memset(&data, 0, sizeof(DBT));
    memset(&key, 0, sizeof(DBT));
    data.flags |= DB_DBT_USERMEM;
    data.ulen = EVT_LEN;
    data.data = evt;
    key.flags |= DB_DBT_USERMEM;
    key.ulen = KEY_LEN;
    key.data = key_buf;
    count = ret = 0;

    /* Process the events. */
    while(count < NUM_EVENTS) {
	/*
	 * Read and remove a event from the front of the queue.  If
	 * the queue is empty wait until an event arrives.
	 */
	if ((ret = evt_queue->get(
	    evt_queue, NULL, &key, &data, DB_CONSUME_WAIT)) != 0)
	    return ret;

	/* Pass the event to the filters. */
	for (i = 0; i < filters->num_filters; i++) {
	    filter = filters->filters[i];
	    if ((ret = filter->filter(filter, dbenv, evt_queue, &data)) != 0)
		return ret;
	}
	count++;
    }

    return ret;
}

/*
 * Open the event queue and pass the events to the various modules based
 * on the given filters.
 */
int receive_events(filters)
    BDB_EVT_FILTERS *filters;
{
    DB_ENV *evt_env;
    DB *evt_queue;
    int ret;

    evt_queue = NULL;
    evt_env = NULL;

    /* Open the event queue. */
    if ((ret = open_env(&evt_env, HOME, 1, 1, 0)) != 0)
	goto err;
    if ((ret = open_queue(evt_env, &evt_queue, DB_NAME, 1, 1, 0)) != 0)
	goto err;

    /* Receive and filter the events. */
    ret = receive_evts(evt_env, evt_queue, filters);

err:	if (evt_queue != NULL)
	    (void)evt_queue->close(evt_queue, 0);
	if (evt_env != NULL)
	    (void)evt_env->close(evt_env, 0);

	return ret;
}
