/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2011, 2017 Oracle and/or its affiliates.  All rights reserved.
 */

/*
 * This is the multi-process test for XA.  The application creates several
 * client processes and uses each process to send requests to the servers.
 * There are two tests.  The first one runs two client processes
 * that send requests to the servers then exit.  In the second
 * test there are 3 client processes.  The first 2 execute the same as in
 * the first test, but the third process calls the servers with a command
 * to kill that server.  This is done to test that the environment and
 * servers can recover from a failure.
 */
#include <sys/types.h>
#include <sys/time.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <tx.h>
#include <atmi.h>
#include <fml32.h>
#include <fml1632.h>

#include <db.h>

#include "../utilities/bdb_xa_util.h"

#define	HOME	"../data"
#define	TABLE1	"../data/table1.db"
#define	TABLE2	"../data/table2.db"
#define NUM_SERVERS 2

char *progname;					/* Client run-time name. */

int
usage()
{
	fprintf(stderr, "usage: %s [-v] [-k]\n", progname);
	return (EXIT_FAILURE);
} 

/*
 * In this function the clients call the servers.  There are three possible
 * clients, two both called "lives", and one called "dies". The clients named
 * "lives" send requests to both servers.  The client named "dies", if it
 * exists, calls one of the servers with a command that causes it to fail
 * and exit.
 */
int call_server(char *client_name, int expected_error)
{
	FBFR *replyBuf;
	long replyLen;
	char *server_names[NUM_SERVERS];
	char *name, *kill_server;
	int commit, j, iterations, ret;
	void *result = NULL;
	TPINIT *initBuf = NULL;
	kill_server = NULL;
	iterations = 100;
	replyBuf = NULL;
	ret = 0;

	/* Names of the function to call in the servers. */
	server_names[0] = "TestThread1";
	server_names[1] = "TestThread2";
	
	/* Allocate init buffer */
	if ((initBuf = (TPINIT *)tpalloc("TPINIT", NULL, TPINITNEED(0))) == 0)
		goto tuxedo_err;

	if (tpinit(initBuf) == -1)
		goto tuxedo_err;
	if (verbose)
	        printf("%s:%s: tpinit() OK\n", progname, client_name);

	/* Create the command to kill the server. */
	if (strcmp(client_name, "dies") == 0) {
		kill_server = (char *)tpalloc("STRING", NULL, 1);
		if (kill_server == NULL)
		  	goto tuxedo_err;
		iterations = 1;
	} else if (expected_error)
		sleep(30);   

	for (j = 0; j < iterations; j++) {
	  	commit = 1;
		if (replyBuf != NULL)
			tpfree((char *)replyBuf);

		/* Randomly select a server. */
		name = server_names[j % 2];

		/* Allocate reply buffer. */
		replyLen = 1024;
		replyBuf = NULL;
		if ((replyBuf = (FBFR*)tpalloc("FML32", NULL, replyLen)) 
		    == NULL) 
			goto tuxedo_err;
		if (verbose)
		        printf("%s:%s: tpalloc(\"FML32\"), reply buffer OK\n", 
			    progname, client_name);

		/* Begin the XA transaction. */
		if (tpbegin(60L, 0L) == -1)
			goto tuxedo_err;
		if (verbose)
		        printf("%s:%s: tpbegin() OK\n", progname, client_name);
		/* Call the server to kill it. */
		if (kill_server != NULL) {
		  	tpcall(name, kill_server,  1L, (char **)&replyBuf, 
			    &replyLen, 0L);
			goto abort;
		} else {
		        if (tpcall(name, NULL, 0L, (char **)&replyBuf, 
			    &replyLen, TPSIGRSTRT) == -1) 
			  
			        /* 
				 * When one of the servers is killed TPNOENT or 
				 * TPESVCERR is an expected error.
				 */
			  if (expected_error && (tperrno == TPESVCERR || tperrno == TPENOENT || tperrno == TPETIME)) 
			                goto abort;
				else
			                goto tuxedo_err;
		}

		/* 
		 * Commit or abort the transaction depending the what the 
		 * server returns. 
		 */
		commit = !tpurcode;
		if (commit) {
commit:			if (verbose) {
			        printf("%s:%s: txn success\n", progname, 
				    client_name);
			}
			if (tpcommit(0L) == -1) {
			  	if (expected_error && tperrno == TPETIME) 
			      	  	continue;
			  	else if (tperrno == TPEABORT)
			  	  	continue;
				else
				    	goto tuxedo_err;
			}
			if (verbose) {
				printf("%s:%s: tpcommit() OK\n", progname, 
				    client_name);
			}
		} else {
abort:			if (verbose) {
				printf("%s:%s: txn failure\n", progname, 
				    client_name);
			}
		  	if (tpabort(0L) == -1) {
			  	if (expected_error && tperrno == TPETIME) 
			    		continue;
			  	else
			  		goto tuxedo_err;
		  	}
			if (verbose) {
				printf("%s:%s: tpabort() OK\n", progname, 
				    client_name);
			}
			if (strcmp(client_name, "dies") == 0) 
			  	break;
		}
	}

	if (0) {
tuxedo_err:	fprintf(stderr, "%s:%s: TUXEDO ERROR: %s (code %d)\n",
			progname, client_name, tpstrerror(tperrno), tperrno);
		ret = -1;
	}
end:	tpterm();
	if (verbose)
		printf("%s:%s: tpterm() OK\n", progname, client_name);	

	if (replyBuf != NULL)
		tpfree((char *)replyBuf);
	if (initBuf != NULL)
		tpfree((char *)initBuf);
	if(kill_server != NULL)
		tpfree((char *)kill_server);	  

	return(ret);
}

/*
 * Create the threads to call the servers, and check that data in the two
 * databases is identical.
 */
int
main(int argc, char* argv[])
{
	int ch, expected_error, i, ret;
	char *name;
	DB_ENV *dbenv;
	u_int32_t flags = DB_INIT_MPOOL | DB_INIT_LOG | DB_INIT_TXN |
	  DB_INIT_LOCK | DB_CREATE | DB_THREAD | DB_RECOVER | DB_REGISTER;
	int check_results;

	name = "lives";
	progname = argv[0];
	dbenv = NULL;
	check_results = 1;
	ret = 0;

	while ((ch = getopt(argc, argv, "n:vkd")) != EOF)
		switch (ch) {
		case 'd':
			expected_error = 1;
			check_results = 0;
			break;
		case 'k':
			name = "dies";
			expected_error = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		case '?':
		default:
			return (usage());
		}
	argc -= optind;
	argv += optind;

	if (verbose)
		printf("%s: called\n", progname);

	if (call_server(name, expected_error) != 0)
	    goto err;

	/* If a server is not killed, check the data in the two tables.*/
	if (check_results) {
	    /* Join the DB environment. */
		if ((ret = db_env_create(&dbenv, 0)) != 0 ||
		    (ret = dbenv->open(dbenv, HOME, flags, 0)) != 0) {
			fprintf(stderr,
			    "%s: %s: %s\n", progname, HOME, db_strerror(ret));
			goto err;
		}
		ret = check_data(dbenv, TABLE1, dbenv, TABLE2, progname);
	}

	if (0) {
err:		ret = EXIT_FAILURE;
	}

	if (dbenv != NULL)
		dbenv->close(dbenv, 0);
	return (ret);
}

