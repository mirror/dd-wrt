/*-
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

/*
 * Location Based Personalized Advertizing Example
 *
 * This example shows a location based personalized advertising application.
 * It simulates a server receiving customer location's from their smart phones
 * and sends advertisements back to the phone based on the customer's location
 * and shopping preferences.  
 */

/*
 * Data Generator
 *
 * The program populates various databases that are used in the location based
 * personalized advertising example.  These databases consist of:
 *
 * Store Information: A SQL database that contains information describing the
 * stores that are advertised, and the ads that will be delivered to the
 * customers.
 * User Information: A btree database that supports duplicate data entries.
 * Contains the user's shopping preferences that are used to decide what ads
 * to send the customer.
 *
 */

#include <stdlib.h>
#include "ad_common_utils.h"
#include "db.h"
#include "sqlite3.h"

#define ARRAY_SIZE 12
static const char *shop_names[] = { "The Shop", "The Store", "Well's Goods",
    "Bob's Store", "Shop and Save", "Acme Goods", "Best Prices",
    "Friday Morning", "Whole Goods", "Best Store Ever", "Good Shop",
    "Deals and More" };

static const char *shop_types[] = { "sporting goods", "kitchen appliances",
    "books", "computer games", "comic books", "pet supplies",
    "shoes", "clothes", "coffee", "groceries",
    "beauty products", "furniture" };

#define KEY_SIZE 32

/* Adds data to the databases containing store and user information. */
int populate_databases()
{
    sqlite3 *store;
    sqlite3_stmt* stmt;
    DB_ENV *dbenv;
    DB *user;
    DB_TXN *txn;
    DBT key, data;
    double lat, longi;
    int i, j, index, num_types, prev, ret;
    char ad[128], data_buf[32], key_buf[KEY_SIZE], *sql;

    dbenv = NULL;
    store = NULL;
    user = NULL;
    stmt = NULL;
    txn = NULL;
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));
    key.data = key_buf;
    key.ulen = key.size = KEY_SIZE;
    key.flags = DB_DBT_USERMEM;
    data.data = data_buf;
    data.ulen = 32;
    data.flags = DB_DBT_USERMEM;

    /* Repeatable random set. */
    srand(NUM_STORES);

    /* Open the store information database as an on-disk SQL database. */
    if ((ret = open_sql(&store, STORES, 1)) != SQLITE_OK)
	goto err;

    /*
     * Add a STOREINFO table to the store database that contains the stores's
     * name, location by longitude and latitude, type, and the ad that will
     * be sent to the user's phone.
     */
    sql = "CREATE table STOREINFO(lat double, longi double, name varchar[20], type varchar[20], ad varchar[100])";
    if ((ret = sqlite3_exec(store, sql, NULL, NULL, NULL)) != SQLITE_OK)
	goto err;

    /* Create an index on the longitude, latitude, and type fields. */
    sql = "CREATE index TYPE on STOREINFO(type);";
    if ((ret = sqlite3_exec(store, sql, NULL, NULL, NULL)) != SQLITE_OK)
	goto err;
    sql = "CREATE index LONGI on STOREINFO(longi);";
    if ((ret = sqlite3_exec(store, sql, NULL, NULL, NULL)) != SQLITE_OK)
	goto err;
    sql = "CREATE index LAT on STOREINFO(lat);";
    if ((ret = sqlite3_exec(store, sql, NULL, NULL, NULL)) != SQLITE_OK)
	goto err;

    /* Add records to the STOREINFO table. */
    sql = "INSERT into STOREINFO values(?, ?, ?, ?, ?);";
    if ((ret = sqlite3_prepare_v2(
	store, sql, (int)strlen(sql), &stmt, NULL)) != SQLITE_OK) 
	    goto err;
    if ((ret = sqlite3_exec(store, "BEGIN;", NULL, NULL, NULL)) != SQLITE_OK)
	goto err;
    for (i = 0; i < NUM_STORES; i++) {
	/* Random latitude and longitude in the United States. */
	lat = (rand() % 2000)/100.00 + 30.0;
	longi = (rand() % 5000)/100.00 + 70.0;
	index = rand() % ARRAY_SIZE;

	/* Bind the latitude and longitude. */
	if ((ret = sqlite3_bind_double(stmt, 1, lat)) != SQLITE_OK)
	    goto err_abort;
	if ((ret = sqlite3_bind_double(stmt, 2, longi)) != SQLITE_OK)
	    goto err_abort;
	/* Bind the store name and type. */
	if ((ret = sqlite3_bind_text(stmt, 3, shop_names[index],
	    (int)strlen(shop_names[index]), SQLITE_STATIC)) != SQLITE_OK)
	    goto err_abort;
	if ((ret = sqlite3_bind_text(stmt, 4, shop_types[index],
	    (int)strlen(shop_types[index]), SQLITE_STATIC)) != SQLITE_OK)
	    goto err_abort;
	/* Bind the store ad. */
	sprintf(ad, "Get %d percent off of %s at %s.",
	    rand() % 80, shop_types[index], shop_names[index]);
	if ((ret = sqlite3_bind_text(
	    stmt, 5, ad, (int)strlen(ad), SQLITE_STATIC)) != SQLITE_OK)
	    goto err_abort;
	/* Insert the record. */
	if ((ret = sqlite3_step(stmt)) != SQLITE_DONE)
	    goto err_abort;
	if ((ret = sqlite3_reset(stmt)) != SQLITE_OK)
	    goto err;
	/* Create a transaction for every 100 entries. */
	if ((i % 100) == 99) {
	    if ((ret = sqlite3_exec(
		store, "COMMIT;", NULL, NULL, NULL)) != SQLITE_OK)
		goto err_abort;
	    if (i != (NUM_STORES - 1)) {
		if ((ret = sqlite3_exec(
		    store, "BEGIN;", NULL, NULL, NULL)) != SQLITE_OK)
		    goto err;
	    }
	}
    }
    if ((ret = sqlite3_finalize(stmt)) != SQLITE_OK)
	goto err;
    stmt = NULL;

    /*
     * Open the user database as an on-disk btree database that supports
     * duplicate data entries.
     */
    if ((ret = open_env(&dbenv, USERS_ENV, 0, 1, 0)) != 0)
	goto err;

    if ((ret = open_btree(dbenv, &user, USERS, 0, 1, 0, 1)) != 0)
	goto err;

    /*
     * Insert users along with random store types as their shopping
     * preference.
     */
    key.size = sizeof(int);
    if ((ret = dbenv->txn_begin(dbenv, NULL, &txn, 0)) != 0)
	goto err;
    for (i = 0; i < NUM_USERS; i++) {
	memcpy(key.data, &i, key.size);
	/* User will have a random number of preferences between 1 and 1. */
	num_types = (rand() % MAX_PREF) + 1;
	prev = index;
	for (j = 0; j < num_types; j++) {
	    /* Avoid any repeating shopping preferences for the same user. */
	    while (prev == index) {
		index = rand() % ARRAY_SIZE;
	    }
	    prev = index;
	    data.size = (u_int32_t)strlen(shop_types[index]) + 1;
	    memcpy(data.data, shop_types[index], data.size);
	    if ((ret = user->put(user, txn, &key, &data, 0)) != 0)
		goto err_abort;
	}
	/* Create a transaction for each 100 inserts. */
	if ((i % 100) == 99) {
	    if ((ret = txn->commit(txn, 0)) != 0)
		goto err;
	    txn = NULL;
	    if (i != (NUM_USERS - 1)) {
		if ((ret = dbenv->txn_begin(dbenv, NULL, &txn, 0)) != 0)
		    goto err;
	    }
	}
    }

if (0) {
err_abort:
	sqlite3_exec(store, "ROLLBACK;", NULL, NULL, NULL);
	if (txn != NULL)
	    txn->abort(txn);
err:
	printf("Error populating databases for example.");
}
    if (stmt != NULL)
	sqlite3_finalize(stmt);
    if (store != NULL)
	(void)sqlite3_close_v2(store);
    if (user != NULL)
	(void)user->close(user, 0);
    if (dbenv != NULL)
	(void)dbenv->close(dbenv, 0);

    return (ret);
}

/* Populate the databases. */
int main()
{
    return populate_databases();
}
