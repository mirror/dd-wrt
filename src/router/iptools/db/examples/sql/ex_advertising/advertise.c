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
 * Advertise
 *
 * This program simulates a central server that sends personal advertisements
 * to smart phones based on the phone's location and owner's shopping
 * preferences.  When the server receives GPS coordinates from a phone it 
 * first looks up the shopping preferences of the phone owner in a Berkeley DB
 * btree database, then uses that information plus the GPS coordinates to find
 * ads for nearby stores in a Berkeley DB SQL database.
 */

#include "ad_common_utils.h"
#include "ad_bdb_filters.h"

#define DATA_LEN 64
#define SQL_LEN 256

/*
 * This function receives a person's location and id and uses that information 
 * to send personalized advertisements.  It first looks up the user's shopping
 * preferences in a Berkeley DB btree database.  Then uses the location and
 * shopping preferences to find ads for stores near the person's location that
 * match their tastes, through a query on a Berkeley DB SQL database.
 */
static int advertise_filter __P((BDB_EVT_FILTER *, DB_ENV *, DB *, DBT *));
static int advertise_filter(bdb_filter, evt_queue_env, evt_queue, evt)
    BDB_EVT_FILTER *bdb_filter;
    DB_ENV *evt_queue_env;
    DB *evt_queue;
    DBT *evt;
{
    sqlite3 *stores;
    sqlite3_stmt *stmt;
    DB_ENV *dbenv;
    DB *users;
    DBC *cursor;
    DB_SEQUENCE *ads_sent, *reqs;
    DB_TXN *txn;
    DBT key, data;
    db_seq_t num_ads, num_reqs;
    const unsigned char *ad;
    char data_buf[KEY_LEN], key_buf[KEY_LEN], sql_stmt[SQL_LEN];
    char *preferences[MAX_PREF];
    double lat, longi;
    int i, num_pref, ret, user_id;

    ret = 0;
    dbenv = bdb_filter->dbenv;
    stores = bdb_filter->sql;
    reqs = bdb_filter->seq1;
    ads_sent = bdb_filter->seq2;
    users = bdb_filter->dbs[0];
    stmt = NULL;
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));
    cursor = NULL;
    txn = NULL;
    memset(sql_stmt, 0, SQL_LEN);
    memset(preferences, 0, MAX_PREF);
    for (i = 0; i < MAX_PREF; i++) {
	preferences[i] = malloc(DATA_LEN);
	if (preferences[i] == NULL)
	    goto err;
    }

    /* Get the latitude, longitude, and user id from the event. */
    memcpy(&user_id, evt->data, sizeof(int));
    memcpy(&lat, (unsigned char *)evt->data + sizeof(int), sizeof(double));
    memcpy(&longi,
	(unsigned char *)evt->data + (sizeof(int) + sizeof(double)),
	sizeof(double));

    /* Increament the requests counter. */
    if ((ret = reqs->get(reqs, NULL, 1, &num_reqs, 0)) != 0)
	goto err;

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
    data.ulen = DATA_LEN;

    /*
     * Get the user's shopping preferences.  Since the database might have
     * multiple data entries associated with the given key, use a cursor
     * to iterate through all of them.
     */
    if ((ret = dbenv->txn_begin(dbenv, NULL, &txn, 0)) != 0)
	goto err;
    if ((ret = users->cursor(users, txn, &cursor, 0)) != 0)
	goto err_abort;
    key.size = sizeof(int);
    memcpy(key.data, &user_id, key.size);
    /* Get the first data record. */
    if ((ret = cursor->get(cursor, &key, &data, DB_SET)) != 0)
	goto err_abort;
    memcpy(preferences[0], (char *)data.data, data.size);
    /* Get the rest of the data records. */
    num_pref = 1;
    while ((ret = cursor->get(cursor, &key, &data, DB_NEXT_DUP)) == 0) {
	memcpy(preferences[num_pref], (char *)data.data, data.size);
	num_pref++;
    }
    if (ret != DB_NOTFOUND)
	goto err_abort;
    if ((ret = txn->commit(txn, 0)) != 0)
	goto err;
    txn = NULL;

    /* Get advertisements that match the user's preferences and location. */
    sprintf(sql_stmt,
	"select ad from STOREINFO where lat < %f AND lat > %f AND longi < %f AND longi > %f;",
	lat + 0.2, lat - 0.2, longi + 0.2, longi - 0.2);
    printf(sql_stmt + strlen(sql_stmt), " AND (type LIKE '%s'", preferences[0]);
    for (i = 1; i < num_pref; i++) {
	sprintf(sql_stmt + strlen(sql_stmt), " OR type LIKE '%s'", preferences[i]);
    }
    sprintf(sql_stmt + strlen(sql_stmt), ");");
    if ((ret = sqlite3_prepare_v2(
	stores, sql_stmt, (int)strlen(sql_stmt), &stmt, NULL)) != SQLITE_OK)
	goto err;
    
    ret = sqlite3_step(stmt);
    /*
     * Get each ad that fits the criteria, it is possible that no results are
     * returned.
     */
    while (ret == SQLITE_ROW) {
	/* Get the ad. */
	ad = sqlite3_column_text(stmt, 0);
	if (ad != NULL) {
	    /* Increament the requests counter. */
	    if ((ret = ads_sent->get(ads_sent, NULL, 1, &num_ads, 0)) != 0)
		goto err;
	    /* Send the ad to the phone. */
	    if ((num_ads % 1000) == 0 && num_ads != 0)
		printf("%d ads sent, including the ad: %s\n", num_ads, ad);
	}
	/* Get next result. */
	ret = sqlite3_step(stmt);
    }
    if (ret != SQLITE_DONE)
	goto err;

    if ((ret = sqlite3_finalize(stmt)) != SQLITE_OK)
	goto err;
    stmt = NULL;

err_abort:
    if (txn != NULL)
	(void)txn->abort(txn);
err:
    if (stmt != NULL)
	(void)sqlite3_finalize(stmt);
    for (i = 0; i < MAX_PREF; i++) {
	if (preferences[i] != NULL)
	    free(preferences[i]);
    }
    return (ret);
}

/*
 * Set up the filter by creating the on-disk environment, database, and
 * sequences.
 */
int advertise_setup(bdb_filter)
    BDB_EVT_FILTER *bdb_filter;
{
    int ret;
    
    bdb_filter->db_name = USERS;
    bdb_filter->filter = advertise_filter;
    bdb_filter->env_home = USERS_ENV;

    /*
     * Create the environment, sql database, databases, and sequences,
     * all as free threaded and on disk.
     */
    if ((ret = open_env(
	&(bdb_filter->dbenv), bdb_filter->env_home, 0, 1, 1)) != 0)
	goto err;
    if ((ret = open_sql(&bdb_filter->sql, STORES, 1)) != 0)
	goto err;
    if ((ret = open_btree(bdb_filter->dbenv,
	&(bdb_filter->dbs[0]), USERS, 0, 1, 1, 1)) != 0) 
	goto err;
    if ((ret = open_btree(bdb_filter->dbenv,
	&(bdb_filter->dbs[1]), COUNTERS, 0, 1, 1, 0)) != 0) 
	goto err;
    if ((ret = open_seq(bdb_filter->dbenv,
	bdb_filter->dbs[1], &(bdb_filter->seq1), REQUESTS, 1, 1)) != 0) 
	goto err;
    if ((ret = open_seq(bdb_filter->dbenv,
	bdb_filter->dbs[1] ,&(bdb_filter->seq2), ADS_SENT, 1, 1)) != 0) 
	goto err;

    return (0);
err:
    return (ret);
}

/*
 * Teardown the advertise filter by closing the environment, databases, and
 * sequences.
 */
void advertise_teardown(bdb_filter)
    BDB_EVT_FILTER *bdb_filter;
{
    int i;

    /* Close the sequences. */
    if (bdb_filter->seq1 != NULL)
	(void)bdb_filter->seq1->close(bdb_filter->seq1, 0);
    if (bdb_filter->seq2 != NULL)
	(void)bdb_filter->seq2->close(bdb_filter->seq2, 0);

    /* Close the databases. */
    if (bdb_filter->dbs != NULL) {
	for (i = 0; i < bdb_filter->num_dbs; i++) {
	    if (bdb_filter->dbs[i] != NULL)
		(void)bdb_filter->dbs[i]->close(bdb_filter->dbs[i], 0);
	}
	free(bdb_filter->dbs);
    }

    /* Close the SQL database. */
    if(bdb_filter->sql != NULL)
	(void)sqlite3_close_v2(bdb_filter->sql);

    /* Close the environment. */
    if (bdb_filter->dbenv != NULL)
	(void)bdb_filter->dbenv->close(bdb_filter->dbenv, 0);
}
