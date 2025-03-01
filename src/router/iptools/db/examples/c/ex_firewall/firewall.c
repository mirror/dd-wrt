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
 * Firewall
 *
 * This program receives messages sent by MessageGenerator and checks their
 * point of origin against a database containing blocked sites.  If the
 * message is from a blocked site it is dropped, otherwise it is forwarded
 * to its destination.
 */

#include "firewall_common_utils.h"
#include "firewall_bdb_filters.h"

static int firewall_filter __P((BDB_MSG_FILTER *, DB_ENV *, DB *, DBT *));
static int blocked_count;

/* This function is used to filter messages from blocked sites. */
static int firewall_filter(bdb_filter, msg_queue_env, msg_queue, msg)
    BDB_MSG_FILTER *bdb_filter;
    DB_ENV *msg_queue_env;
    DB *msg_queue;
    DBT *msg;
{
    DB *destination, *blocked_sites;
    DBT key, data;

    char key_buf[KEY_LEN], data_buf[KEY_LEN];
    int ret;

    ret = 0;
    destination = bdb_filter->dbs[0];
    blocked_sites = bdb_filter->dbs[1];

    /*
     * Configure the key so that it uses a user defined buffer instead of
     * having the put function allocated memory to store the key value.
     */
    memset(&key, 0, sizeof(DBT));
    key.data = key_buf;
    key.flags |= DB_DBT_USERMEM;
    key.ulen = KEY_LEN;
    memset(&data, 0, sizeof(DBT));
    data.data = data_buf;
    data.flags |= DB_DBT_USERMEM;
    data.ulen = KEY_LEN;

    /*
     * Check that the message does not come from one of the blocked sites by
     * searching for the site of origin in the blocked site list.
     */
    key.size = (u_int32_t)strlen((const char *)msg->data) + 1;
    strncpy((char *)key.data, (const char *)msg->data, key.size);
    ret = blocked_sites->get(blocked_sites, NULL, &key, &data, 0);
    /*
     * A return value of 0 means the site was found on the blocked list, so
     * the message should be blocked.
     */
    if (ret == 0) {
	blocked_count++;
	/* Print out a message after blocking 10,000 messages. */
	if ((blocked_count % 10000) == 0)
	    printf("The firewall has blocked %i messages.\n", blocked_count);
	return (0);
    /*
     * A return value of DB_NOTFOUND means no record was found in the btree
     * that matched the given key.  So the message should be forwarded
     * to its destination.  Any other non-zero return value is an error.
     */
    } else if (ret != DB_NOTFOUND)
	return (ret);

    /*
     * Forward the message to its destination by adding it to a queue from
     * which the destination module will read.
     */
    if ((ret = destination->put(destination, NULL, &key, msg, DB_APPEND)) != 0)
	return (ret);

    return (0);
}

/* Open and populate a database with the sites that should be blocked. */
static int open_blocked_sites_db(dbenv, db, db_name)
    DB_ENV *dbenv;
    DB **db;
    const char *db_name;
{
    DB *btree;
    DBT key, data;
    char buf[KEY_LEN];
    int i, ret;
    u_int32_t flags = DB_AUTO_COMMIT|DB_CREATE|DB_THREAD;

    *db = NULL;
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));
    key.flags |= DB_DBT_USERMEM;
    key.data = buf;
    key.ulen = KEY_LEN;

    if ((ret = db_create(&btree, dbenv, 0)) != 0)
	return (ret);
    *db = btree;
    /* Create an in-memory btree database. */
    if ((ret = btree->open(btree, NULL, NULL, db_name, DB_BTREE, flags, 0)) != 0)
	return (ret);
    /* Add the blocked sites to the database. */
    for (i = 0; i < BLOCKED_SITES_SIZE; i++) {
	strcpy((char *)key.data, KnownSites[i]);
	key.size = (u_int32_t)strlen(KnownSites[i]) + 1;
	if ((ret = btree->put(btree, NULL, &key, &data, 0)) != 0)
	    return (ret);
    }

    return (0);
}

/* Start recieving and sorting incoming messages. */
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
    filter.db_names[0] = "destination";
    filter.db_names[1] = "blocked_sites";
    filter.env_home = "Destination_Env";
    filter.filter = firewall_filter;
    blocked_count = 0;

    /* Open the environments and databases defined in the filters. */
    if ((ret = open_env(&(filter.dbenv), filter.env_home, 1, 1, 1)) != 0)
	goto err;
    filter.dbs = malloc(sizeof(DB *) * filter.num_dbs);
    if ((ret = open_queue(
	filter.dbenv, &(filter.dbs[0]), filter.db_names[0], 1, 1, 1)) != 0)
	goto err;
    /*
     * Open an in-memory btree database and populate it with the 
     * blocked sites.
     */
    if ((ret = open_blocked_sites_db(
	filter.dbenv, &(filter.dbs[1]), filter.db_names[1])) != 0)
	goto err;

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

