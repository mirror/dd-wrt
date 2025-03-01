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
 * Destination
 *
 * This program handles messages that are forwarded by the PrioritySorter.
 * High priority messages are stored in the high priority queue and low
 * priority messages are stored in the low priority queue, with high
 * priority messages being handled before the low priority ones.  A message
 * is printed out after each 10,000 high priority messages are handled and
 * each 10,000 low priority messages.
 */

#include "priority_common_utils.h"
#include "db.h"

/*
 * Start reading messages from the queue.  After 10,000 messages are received
 * from either queue print out how long it took to process those messages.
 */
int process_msgs(dbenv, high, low)
    DB_ENV *dbenv;
    DB *high;
    DB *low;
{
    DBT key, data;
#ifdef _WIN32
    struct timeval2 high_end, high_start, low_end, low_start;
#else
    struct timeval high_end, high_start, low_end, low_start;
#endif
    double high_secs, low_secs;
    unsigned char msg[MSG_LEN], key_buf[KEY_LEN];
    int high_count, high_first, low_count, low_first, msg_count, ret;
    int starve;

    /*
     * Configure the key and data fields to use a user defined memory
     * buffer instead of having the database get function allocate one.
     */
    memset(msg, 0, MSG_LEN);
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));
    data.flags |= DB_DBT_USERMEM;
    data.ulen = MSG_LEN;
    data.data = msg;
    key.flags |= DB_DBT_USERMEM;
    key.ulen = KEY_LEN;
    key.data = key_buf;
    high_first = low_first = 1;
    high_count = low_count = msg_count = starve = 0;

    while(msg_count < NUM_MESSAGES) {
	/*
	 * Try to read from the high priority queue first, and if it is empty
	 * (returns DB_NOTFOUND), try the low priority queue.
	 */
	ret = high->get(high, NULL, &key, &data, DB_CONSUME);
	if (ret == 0) {
	    /* After getting the first message start the high priority timer.*/
	    if (high_first) {
		(void)gettimeofday(&high_start, NULL);
		high_first = 0;
	    }

	    /*
	     * Increment the high priority message counter.  After 10,000
	     * messages reset the counter and print out how long it took.
	     */
	    high_count++;
	    msg_count++;
	    starve++;
	    if (high_count == 10000) {
		high_count = 0;
		/*
		 * Calculate the time it took to process 
		 * 10,000 messages.
		 */
		(void)gettimeofday(&high_end, NULL);
		high_secs =
		    (((double)high_end.tv_sec * NS_PER_MS + 
		    high_end.tv_usec) -
		    ((double)high_start.tv_sec * NS_PER_MS + 
		    high_start.tv_usec))/NS_PER_MS;
		(void)gettimeofday(&high_start, NULL);
		/*
		 * Print out the time it took to process
		 * 10,000 high priority messages.
		 */
		printf(
		    "10,000 high priority messages processed in %f seconds.\n",
		    high_secs);
	    }
	    /*
	     * If 3 high priority messages are processed without checkin the
	     * low priority queue, then check it to prevent starvation.
	     */
	    if (starve == 3)
		goto low;
	} else if (ret == DB_NOTFOUND) {
low:	    starve = 0;
	    ret = low->get(low, NULL, &key, &data, DB_CONSUME);
	    /*
	     * If no low priority messages then try the high priority queue
	     * again.
	     */
	    if (ret == DB_NOTFOUND) {
		/*
		 * Sleep for a short amount of time if both queues are empty,
		 * so the databases are free of locks so new messages can be
		 * added.
		 */
		usleep(10);
		continue;
	    } else if (ret != 0)
		goto err;

	    /* After getting the first message start the low priority timer.*/
	    if (low_first) {
		(void)gettimeofday(&low_start, NULL);
		low_first = 0;
	    }

	    /*
	     * Increment the low priority message counter.  After 10,000
	     * messages reset the counter and print out how long it took.
	     */
	    low_count++;
	    msg_count++;
	    if (low_count == 10000) {
		low_count = 0;
		/*
		 * Calculate the time it took to process 
		 * 10,000 messages.
		 */
		(void)gettimeofday(&low_end, NULL);
		low_secs =
		    (((double)low_end.tv_sec * NS_PER_MS + 
		    low_end.tv_usec) -
		    ((double)low_start.tv_sec * NS_PER_MS + 
		    low_start.tv_usec))/NS_PER_MS;
		(void)gettimeofday(&low_start, NULL);
		/*
		 * Print out the time it took to process
		 * 10,000 low priority messages.
		 */
		printf(
		    "10,000 low priority messages processed in %f seconds.\n",
		    low_secs);
	    }
	} else
	    goto err;
    }

 err:   return (ret);
}

/*
 * Open the high and low priority message queues and start receiving
 * messages.
 */
int process_messages()
{
    DB_ENV *dbenv;
    DB *high, *low;
    int ret;

    high = low = NULL;
    dbenv = NULL;

    /* Open the environment and message queues. */
    if ((ret = open_env(&dbenv, DESTINATION_ENV, 1, 1, 0)) != 0) {
	goto err;
    }
    if ((ret = open_queue(dbenv, &high, HIGH_DB, 1, 1, 0)) != 0)
	goto err;

    if ((ret = open_queue(dbenv, &low, LOW_DB, 1, 1, 0)) != 0)
	goto err;

    /* Start receiving and processing messages. */
    ret = process_msgs(dbenv, high, low);

err:	
    if (high != NULL)
	(void)high->close(high, 0);
    if (low != NULL)
	(void)low->close(low, 0);
    if (dbenv != NULL)
	(void)dbenv->close(dbenv, 0);

    return ret;
}

/* Receive and process messages. */
int main()
{
    return process_messages();
}
