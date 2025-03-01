/*-
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

/*
 * Priority Message Handling Example
 *
 * This application is a simple priority message handler that uses Oracle
 * Berkeley DB queue databases to implement a high priority message queue and
 * a low priority message queue.
 */

/*
 * Priority Sorter
 *
 * This program receives messages sent by MessageGenerator and checks their
 * pritority.  High priority messages are put in a high priority queue, while
 * the rest are put in a low priority queue.  Both types of messages are
 * then processed at the Destination module.
 */

#include "priority_common_utils.h"
#include "priority_bdb_filters.h"

static int priority_filter __P((BDB_MSG_FILTER *, DB_ENV *, DB *, DBT *));

/*
 * This function is used to sort messages into a high priority queue
 * and a low priority queue.
 */
static int priority_filter(bdb_filter, msg_queue_env, msg_queue, msg)
    BDB_MSG_FILTER *bdb_filter;
    DB_ENV *msg_queue_env;
    DB *msg_queue;
    DBT *msg;
{
    DB *high, *low;
    DBT key;

    char key_buf[KEY_LEN];
    int priority, ret;

    ret = 0;
    high = bdb_filter->dbs[0];
    low = bdb_filter->dbs[1];

    /*
     * Configure the key so that it uses a user defined buffer instead of
     * having the put function allocated memory to store the key value.
     */
    memset(&key, 0, sizeof(DBT));
    key.data = key_buf;
    key.flags |= DB_DBT_USERMEM;
    key.ulen = KEY_LEN;

    /* Get the priority of the message. */
    memcpy(&priority, msg->data, sizeof(int));
    /*
     * Put messages with a priority greater than 0 in the high priority queue,
     * and the rest in the low priority queue.
     */
    if (priority > 0) {
	if ((ret = high->put(high, NULL, &key, msg, DB_APPEND)) != 0)
	    return (ret);
    } else {
	if ((ret = low->put(low, NULL, &key, msg, DB_APPEND)) != 0)
	    return (ret);
    }

    return (0);
}


/* Starts several threads to receive and sort incoming messages. */
int main()
{
    BDB_MSG_FILTERS *filters;
    BDB_MSG_FILTER filter;
    int i, ret;

    /*
     * Create the filters that sort the messages and forward them to
     * different modules based on the message value.
     */
    memset(&filter, 0, sizeof(BDB_MSG_FILTER));
    filters = malloc(sizeof(BDB_MSG_FILTERS));
    filters->filters = NULL;
    filters->num_filters = 1;
    filter.num_dbs = 2;
    if ((filter.db_names = malloc(sizeof(char *) * filter.num_dbs)) == NULL)
	goto err;
    filter.db_names[0] = HIGH_DB;
    filter.db_names[1] = LOW_DB;
    filter.env_home = DESTINATION_ENV;
    filter.filter = priority_filter;

    /* Open the environments and databases defined in the filters. */
    if ((ret = open_env(&(filter.dbenv), filter.env_home, 1, 1, 1)) != 0)
	goto err;
    filter.dbs = malloc(sizeof(DB *) * filter.num_dbs);
    for (i = 0; i < filter.num_dbs; i++) {
	if ((ret = open_queue(
	    filter.dbenv, &(filter.dbs[i]), filter.db_names[i], 1, 1, 1)) != 0)
	    goto err;
    }

    /* Start recieving and processing messages. */
    filters->filters = malloc(sizeof(BDB_MSG_FILTER *));
    if (filters->filters == NULL)
	goto err;
    filters->filters[0] = &(filter);
    if ((ret = receive_messages(filters)) != 0)
	goto err;

err:		free(filters->filters);
    if (filter.db_names != NULL)
	free(filter.db_names);
    if (filter.dbs != NULL) {
	for (i = 0; i < filter.num_dbs; i++) {
	    if (filter.dbs[i] != NULL) {
		(void)filter.dbs[i]->close(filter.dbs[i], 0);
	    }
	}
    }
    free(filter.dbs);
    if (filter.dbenv != NULL)
	(void)filter.dbenv->close(filter.dbenv, 0);

    free(filters);
    return 0;
}
