/*-
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

/*
 * Firewall Example
 *
 * This application shows how Oracle Berkeley DB can be used to implement a
 * simple firewall using a queue and btree databases.
 */

/*
 * Message Generator
 *
 * This program is a message simulator that continuously generates messages
 * that are sent to the Destination through the Firewall.  The messages consist
 * of a point of origin which could be forbidden by the firewall.
 */

#include <stdlib.h>
#include "firewall_common_utils.h"
#include "db.h"

/*
 * Simulates a high message load to a server by continuously creating messages
 * and inserting them into a BDB queue that the server reads from.
 */
int send_msgs(dbenv, msg_queue)
    DB_ENV *dbenv;
    DB *msg_queue;
{
    DBT key, data;
    unsigned char msg[MSG_LEN], key_buf[25];
    int count, ret, site;

    /*
     * Configure the key and data fields to use a user defined memory
     * buffer instead of having the database put function allocate one.
     */
    memset(msg, 0, MSG_LEN);
    memset(&data, 0, sizeof(DBT));
    memset(&key, 0, sizeof(DBT));
    data.flags |= DB_DBT_USERMEM;
    data.ulen = MSG_LEN;
    data.size = MSG_LEN;
    data.data = msg;
    key.flags |= DB_DBT_USERMEM;
    key.ulen = 25;
    key.data = key_buf;
    count = ret = 0;
    /* A predictible random set. */
    srand(0);

    while(count < (NUM_MESSAGES * 1.4)) {
	/* Copy a site of origin into the message. */
	site = rand() % KNOWN_SITES_SIZE;
	memcpy(data.data,  KnownSites[site], strlen(KnownSites[site]) + 1);

	count++;

	/* Place the message at the end of the queue. */
	if ((ret = msg_queue->put(
	    msg_queue, NULL, &key, &data, DB_APPEND)) != 0)
	    return ret;
    }

    return ret;
}

/* Open the message queue and start adding messages to it. */
int send_messages()
{
    DB_ENV *dbenv;
    DB *msg_queue;
    int ret;

    msg_queue = NULL;
    dbenv = NULL;

    /* Open the message queue. */
    if ((ret = open_env(&dbenv, HOME, 1, 1, 0)) != 0)
	goto err;
    if ((ret = open_queue(dbenv, &msg_queue, DB_NAME, 1, 1, 0)) != 0)
	goto err;

    /* Add messages to the queue. */
    ret = send_msgs(dbenv, msg_queue);

err:	if (msg_queue != NULL)
	    (void)msg_queue->close(msg_queue, 0);
	if (dbenv != NULL)
	    (void)dbenv->close(dbenv, 0);

	return ret;
}

/* Send a heavy stream of messages to the server. */
int main()
{
    return send_messages();
}
