/*-
* See the file LICENSE for redistribution information.
*
* Copyright (c) 2015, 2017 Oracle and/or its affiliates.  All rights reserved.
*
*/

/*
 * Toll Booth Event Processing Example
 *
 * This application simulates an automated highway toll booth.  The application
 * receives a data stream of licence plates and timestamps of cars passing
 * through the toll booth.  The application uses this information to perform
 * billing, send traffic alerts, and check for stolen cars.
 */

/*
 * Data Generator
 *
 * The program populates various databases that are used in the toll booth
 * example.  These databases consist of:
 *
 * Billing: A SQL database that contains user information and toll booth funds.
 * Stolen: A btree database that contains license plate numbers of stolen cars.
 *
 */

#include <stdlib.h>
#include "toll_common_utils.h"
#include "db.h"
#include "sqlite3.h"

#define KEY_SIZE 32

/* Adds data to the databases containing billing and stolen car information. */
int populate_databases()
{
    sqlite3 *billing;
    sqlite3_stmt* stmt;
    DB_ENV *dbenv;
    DB *stolen;
    DB_TXN *txn;
    DBT key, data;
    double funds;
    int i, plate, ret;
    char key_buf[KEY_SIZE], *sql;

    dbenv = NULL;
    billing = NULL;
    stolen = NULL;
    stmt = NULL;
    txn = NULL;
    memset(&key, 0, sizeof(DBT));
    memset(&data, 0, sizeof(DBT));
    key.data = key_buf;
    key.ulen = key.size = KEY_SIZE;
    key.flags = DB_DBT_USERMEM;

    /* Repeatable random set. */
    srand(NUM_PLATES);

    /* Open the billing information database as an on-disk SQL database. */
    if ((ret = open_sql(&billing, BILLING, 1)) != SQLITE_OK)
	goto err;

    /*
     * Add a USERINFO table to the billing database that contains the car's
     * plate number, owner's name, owner's email, and avaiable funds for paying
     * tolls.
     */
    sql = "CREATE table USERINFO(plate int, name varchar[20], email varchar[20], funds double)";
    if ((ret = sqlite3_exec(billing, sql, NULL, NULL, NULL)) != SQLITE_OK)
	goto err;
    /* Create an index on the license plate field. */
    sql = "CREATE index PLATES on USERINFO(plate);";
    if ((ret = sqlite3_exec(billing, sql, NULL, NULL, NULL)) != SQLITE_OK)
	goto err;
    /* Add 100,000 records to the USERINFO table. */
    sql = "INSERT into USERINFO values(?, \"John Doe\", \"email@email.com\", ?);";
    if ((ret = sqlite3_prepare_v2(
	billing, sql, (int)strlen(sql), &stmt, NULL)) != SQLITE_OK) 
	    goto err;
    if ((ret = sqlite3_exec(billing, "BEGIN;", NULL, NULL, NULL)) != SQLITE_OK)
	goto err;
    for (plate = 0; plate < NUM_PLATES; plate++) {
	/* Bind the license plate number. */
	if ((ret = sqlite3_bind_int(stmt, 1, plate)) != SQLITE_OK)
	    goto err_abort;
	/* Randomly set the avaliable funds. */
	funds = rand()%100 + 8.0;
	if ((ret = sqlite3_bind_double(stmt, 2, funds)) != SQLITE_OK)
	    goto err_abort;
	if ((ret = sqlite3_step(stmt)) != SQLITE_DONE)
	    goto err_abort;
	if ((ret = sqlite3_reset(stmt)) != SQLITE_OK)
	    goto err;
	/* Create a transaction for every 100 entries. */
	if ((plate % 100) == 99) {
	    if ((ret = sqlite3_exec(
		billing, "COMMIT;", NULL, NULL, NULL)) != SQLITE_OK)
		goto err_abort;
	    if (plate != (NUM_PLATES - 1)) {
		if ((ret = sqlite3_exec(
		    billing, "BEGIN;", NULL, NULL, NULL)) != SQLITE_OK)
		    goto err;
	    }
	}
    }
    if ((ret = sqlite3_finalize(stmt)) != SQLITE_OK)
	goto err;
    stmt = NULL;

    /* Open the stolen car database as an on-disk btree database. */
    if ((ret = open_env(&dbenv, STOLEN_ENV, 0, 1, 0)) != 0)
	goto err;

    if ((ret = open_btree(dbenv, &stolen, STOLEN, 0, 1, 0, 0)) != 0)
	goto err;

    /* Insert 5 random licence plates into the stolen database. */
    key.size = sizeof(int);
    if ((ret = dbenv->txn_begin(dbenv, NULL, &txn, 0)) != 0)
	goto err;
    for (i = 0; i < 5; i++) {
	plate = rand() % NUM_PLATES;
	memcpy(key.data, &plate, key.size);
	if ((ret = stolen->put(stolen, txn, &key, &data, 0)) != 0)
	    goto err_abort;
    }
    if ((ret = txn->commit(txn, 0)) != 0)
	goto err;

if (0) {
err_abort:
	sqlite3_exec(billing, "ROLLBACK;", NULL, NULL, NULL);
	if (txn != NULL)
	    txn->abort(txn);
err:
	printf("Error populating databases for example.");
}
    if (stmt != NULL)
	sqlite3_finalize(stmt);
    if (billing != NULL)
	(void)sqlite3_close_v2(billing);
    if (stolen != NULL)
	(void)stolen->close(stolen, 0);
    if (dbenv != NULL)
	(void)dbenv->close(dbenv, 0);

    return (ret);
}

/* Populate the databases. */
int main()
{
    return populate_databases();
}
