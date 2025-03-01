/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2011, 2017 Oracle and/or its affiliates.  All rights reserved.
 */

/*
 * This is the MVCC test for XA.  It runs 3 tests, which are as follows:
 * 1. No MVCC
 * 2. MVCC enabled by DB_CONFIG
 * 3. MVCC enabled by flags
 */
#include <sys/types.h>
#include <sys/time.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <tx.h>
#include <atmi.h>
#include <fml32.h>
#include <fml1632.h>

#include <db.h>

#include "../utilities/bdb_xa_util.h"

#define	HOME	"../data"
#define	TABLE1	"../data/table1.db"
#define	TABLE2	"../data/table2.db"
#define NUM_CLIENTS 2
#define NUM_TESTS 4
#define TIMEOUT 60

static int error = 1;

char *progname;					/* Client run-time name. */

int
usage()
{
	fprintf(stderr, "usage: %s [-v] -t[0|1|2] -n [name]\n", progname);
	return (EXIT_FAILURE);
}

enum test_type{NO_MVCC, MVCC_DBCONFIG, MVCC_FLAG};

void init_tests(char *ops[], int success[], int type, int client_num);

/*
 * 
 */
int call_server(char *client_name, int ttype)
{
	char *ops[NUM_TESTS];
	int commit, j, success[NUM_TESTS], client_num, ret;
	TPINIT *initBuf = NULL;
	FBFR *replyBuf = NULL;
	long replyLen = 0;

	client_num = atoi(client_name);
	init_tests(ops, success, ttype, client_num);

	if (verbose)
		printf("%s:%s: starting client %i\n", progname, client_name,
		    client_num);
	
	/* Allocate init buffer */
	if ((initBuf = (TPINIT *)tpalloc("TPINIT", NULL, TPINITNEED(0))) == 0)
		goto tuxedo_err;

	if (tpinit(initBuf) == -1)
		goto tuxedo_err;
	if (verbose)
		printf("%s:%s: tpinit() OK\n", progname, client_name);

	/* Allocate reply buffer. */
	replyLen = 1024;
	if ((replyBuf = (FBFR*)tpalloc("FML32", NULL, replyLen)) == NULL) 
		goto tuxedo_err;
	if (verbose)
		printf("%s:%s: tpalloc(\"FML32\"), reply buffer OK\n", 
		    progname, client_name);

	for (j = 0; j < NUM_TESTS; j++) {
		commit = 1;

		/* Sync Apps. */
		if ((ret = sync_clients(client_num, NUM_CLIENTS, 1, j)) != 0) {
			fprintf(stderr, 
			    "%s:%s: Error syncing clients: %i \n",
			    progname, client_name, ret);
			goto end;
		}

		/* Begin the XA transaction. */
		if (tpbegin(TIMEOUT, 0L) == -1)
			goto tuxedo_err;
		if (verbose)
			printf("%s:%s: tpbegin() OK\n", progname, client_name);

		/* Force client 2 to wait till client 1 does its operation.*/
		if (client_num == 2) {
		  if ((ret = sync_clients(client_num, NUM_CLIENTS, 2, j)) != 0) {
				fprintf(stderr, 
				    "%s:%s: Error syncing client 1: %i \n",
				    progname, client_name, ret);
				goto end;
			}
		}
		if (verbose)
			printf("%s:%s: calling server %s\n", progname, 
			    client_name, ops[j]);

		/* Read or insert into the database. */
		if (tpcall(ops[j], NULL, 0L, (char **)&replyBuf, 
		    &replyLen, 0) == -1) 
			goto tuxedo_err;

		/* Wake up client 2.*/
		if (client_num == 1) {
		  if ((ret = sync_clients(client_num, NUM_CLIENTS, 2, j)) != 0) {
				fprintf(stderr, 
				    "%s:%s: Error syncing client 1: %i \n",
				    progname, client_name, ret);
				goto end;
			}
		}

		/* Sync both clients. */
		if ((ret = sync_clients(client_num, NUM_CLIENTS, 3, j)) != 0) {
			fprintf(stderr, 
			    "%s:%s: Error syncing clients: %i \n",
			    progname, client_name, ret);
			goto end;
		}

		/* 
		 * Commit or abort the transaction depending the what the 
		 * server returns. Check that it matched expectation
		 * (We abort on LOCK_NOTGRANTED and DEADLOCK errors, and
		 * commit otherwise.  Other errors result in returning
		 * without committing or aborting.
		 */
		commit = !tpurcode;
		if (commit != success[j]) {
			fprintf(stderr, 
			    "%s:%s: Expected: %i Got: %i.\n",
			    progname, client_name, success[j], commit);
			if (verbose) {
				printf("%s:%s: Expected: %i Got: %i.\n",
				    progname, client_name, success[j], commit);
			}
		}
			
		if (commit) {
			if (tpcommit(0L) == -1) 
				goto tuxedo_err;
			if (verbose) {
				printf("%s:%s: tpcommit() OK\n", progname, 
				    client_name);
			}
		} else {
			if (tpabort(0L) == -1) {
				goto tuxedo_err;
			}
			if (verbose) {
				printf("%s:%s: tpabort() OK\n", progname, 
				    client_name);
			}
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

	if (initBuf != NULL)
		tpfree((char *)initBuf);

	if (replyBuf != NULL)
		tpfree((char *)replyBuf);

	return(ret);
}

/*
 * Call the servers, and check that data in the two
 * databases is identical.
 */
int
main(int argc, char* argv[])
{
	int ch, i, ret, ttype;
	char *name;

	progname = argv[0];
	i = 1;
	verbose = 0;

	while ((ch = getopt(argc, argv, "vt:n:")) != EOF) {
		switch (ch) {
		case 'n':
			name = argv[++i];
			break;
		case 't':
			ttype = atoi(argv[++i]);
			break;
		case 'v':
			verbose = 1;
			break;
		case '?':
		default:
			return (usage());
		}
		i++;
	}

	if (ttype > 2 || ttype < 0)
		return (usage());

	if (verbose)
		printf("%s: called with type %i\n", progname, ttype);

	if (call_server(name, ttype) != 0)
		goto err;

	if (0) {
err:		ret = EXIT_FAILURE;
	}

	return (ret);
}

static char *rdb1 = "read_db1";
static char *wdb1 = "write_db1";

/*
 *	    Operation		    Success
 * client 1	client 2	no MVCC		MVCC
 * write    	write		no		no
 * write    	read		no		yes
 * read		read		yes		yes
 * read		write		no		yes
 */
void init_tests(char *ops[], int success[], int type, int client_num) 
{
	if (client_num == 1) {
		ops[0] = wdb1;
		ops[1] = wdb1;
		ops[2] = rdb1;
		ops[3] = rdb1;
		/* client 1 is always successful. */
		success[0] = 1;
		success[1] = 1;
		success[2] = 1;
		success[3] = 1;
	} else {
		ops[0] = wdb1;
		ops[1] = rdb1;
		ops[2] = rdb1;
		ops[3] = wdb1;
		if (type == NO_MVCC) {
			success[0] = 0;
			success[1] = 0;
			success[2] = 1;
			success[3] = 0;
		} else {
			success[0] = 0;
			success[1] = 1;
			success[2] = 1;
			success[3] = 1;
		}
	}
}
