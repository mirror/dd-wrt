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
 * Destination
 *
 * This program handles messages that are forwarded by the Firewall.
 * After every 10,000 messages handled it prints out a message 
 * stating how long it took to handle those messages.
 */

#include "firewall_common_utils.h"
#include "db.h"

static int
    usage()
{
    (void)fprintf(stderr,
	"usage: message_handler -d db_name -h env_home \n");
    return (EXIT_FAILURE);
}

#ifdef _WIN32
extern int getopt(int, char * const *, const char *);
#endif

/*
 * Start reading messages from the queue.  After 10,000 messages are received
 * print out how long it took to process those messages.
 */
int process_msgs(dbenv, msg_queue)
    DB_ENV *dbenv;
    DB *msg_queue;
{
    DBT key, data;
#ifdef _WIN32
    struct timeval2 end_time, start_time;
#else
    struct timeval end_time, start_time;
#endif
    double time_secs;
    unsigned char msg[MSG_LEN], key_buf[KEY_LEN];
    int count, first, msg_count, ret;

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
    first = 1;
    count = msg_count = 0;

    while(msg_count < NUM_MESSAGES) {
	/*
	 * Read and remove the message at the front of the queue.  If
	 * there is no message wait until one arrives.
	 */
	if ((ret = msg_queue->get(
	    msg_queue, NULL, &key, &data, DB_CONSUME_WAIT)) != 0)
	    return ret;

	/* After getting the first message start the timer. */
	if (first) {
	    (void)gettimeofday(&start_time, NULL);
	    first = 0;
	}

	/*
	 * Increment the message counter.  After 10,000 messages reset
	 * the counter and print out how long it took.
	 */
	count++;
	msg_count++;
	if (count == 10000) {
	    count = 0;
	    /*
	     * Calculate the time it took to process 
	     * 10,000 messages.
	     */
	    (void)gettimeofday(&end_time, NULL);
	    time_secs =
		(((double)end_time.tv_sec * NS_PER_MS + 
		end_time.tv_usec) -
		((double)start_time.tv_sec * NS_PER_MS + 
		start_time.tv_usec))/NS_PER_MS;
	    (void)gettimeofday(&start_time, NULL);
	    /*
	     * Print out the time it took to process
	     * 10,000 messages.
	     */
	    printf(
		"10,000 messages processed in %f seconds.\n", time_secs);
	}
    }

    return ret;
}

/* Open the message queue and start receiving messages. */
int process_messages(env_home, dbname)
    const char *env_home;
    const char *dbname;
{
    DB_ENV *dbenv;
    DB *msg_queue;
    int ret;

    msg_queue = NULL;
    dbenv = NULL;

    /* Open the environment and message queue. */
    if ((ret = open_env(&dbenv, env_home, 1, 1, 0)) != 0) {
	goto err;
    }
    if ((ret = open_queue(dbenv, &msg_queue, dbname, 1, 1, 0)) != 0)
	goto err;

    /* Start receiving and processing messages. */
    ret = process_msgs(dbenv, msg_queue);

err:	
    if (msg_queue != NULL)
	(void)msg_queue->close(msg_queue, 0);
    if (dbenv != NULL)
	(void)dbenv->close(dbenv, 0);

    return ret;
}

/* Receive and process messages. */
int main(argc, argv)
    int argc;
    char *argv[];
{
    extern char *optarg;
    extern int optind;
    const char *dbname, *env_home;
    char ch;

    /*
     * Get the name of the message queue and the path to the BDB
     * environment.
     */
    while ((ch = getopt(argc, argv, "d:h:")) != EOF) {
	switch (ch) {
	case 'd':
	    dbname = optarg;
	    break;
	case 'h':
	    env_home = optarg;
	    break;
	case '?':
	default:
	    return (usage());
	}
    }

    return process_messages(env_home, dbname);
}
