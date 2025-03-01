/*
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

#include "firewall_bdb_filters.h"
#include "firewall_common_utils.h"


/*
 * Reads messages from the queue and forwards or handles them based on
 * the given filters.
 */
int receive_msgs(dbenv, msg_queue, filters)
    DB_ENV *dbenv;
    DB *msg_queue;
    BDB_MSG_FILTERS *filters;
{
    DBT key, data;
    BDB_MSG_FILTER *filter;
    unsigned char msg[MSG_LEN], key_buf[KEY_LEN];
    int count, i, ret;

    /*
     * Configure the key and data fields to use a user defined memory
     * buffer instead of having the database get function allocate one.
     */
    memset(&data, 0, sizeof(DBT));
    memset(&key, 0, sizeof(DBT));
    data.flags |= DB_DBT_USERMEM;
    data.ulen = MSG_LEN;
    data.data = msg;
    key.flags |= DB_DBT_USERMEM;
    key.ulen = KEY_LEN;
    key.data = key_buf;
    count = ret = 0;

    while(count < (NUM_MESSAGES * 1.4)) {
	/*
	 * Read and remove a message from the front of the queue.  If
	 * the queue is empty wait until a message arrives.
	 */
	if ((ret = msg_queue->get(
	    msg_queue, NULL, &key, &data, DB_CONSUME_WAIT)) != 0)
	    return ret;

	/* Pass the message to the filters. */
	for (i = 0; i < filters->num_filters; i++) {
	    filter = filters->filters[i];
	    if ((ret = filter->filter(filter, dbenv, msg_queue, &data)) != 0)
		return ret;
	}
	count++;
    }

    return ret;
}

/*
 * Open the message queue and pass the messages to the various modules based
 * on the given filters.
 */
int receive_messages(filters)
    BDB_MSG_FILTERS *filters;
{
    DB_ENV *msg_env;
    DB *msg_queue;
    int ret;

    msg_queue = NULL;
    msg_env = NULL;

    /* Open the message queue. */
    if ((ret = open_env(&msg_env, HOME, 1, 1, 0)) != 0)
	goto err;
    if ((ret = open_queue(msg_env, &msg_queue, DB_NAME, 1, 1, 0)) != 0)
	goto err;

    /* Receive and filter the messages. */
    ret = receive_msgs(msg_env, msg_queue, filters);

err:	if (msg_queue != NULL)
	    (void)msg_queue->close(msg_queue, 0);
	if (msg_env != NULL)
	    (void)msg_env->close(msg_env, 0);

	return ret;
}
