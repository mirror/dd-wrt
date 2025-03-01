/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2006, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef _WIN32
#include <windows.h>
#define	sleep(s)		Sleep(1000 * (s))
#else /* !_WIN32 */
#include <unistd.h>
#endif

#include <db.h>

#ifdef _WIN32
extern int getopt(int, char * const *, const char *);
#endif

#define	CACHESIZE   (10 * 1024 * 1024)
#define	DATABASE    "quote.db"
#define SLEEPTIME   3

const char *progname = "ex_rep_gsg_wrfor";

int create_env(const char *, DB_ENV **);
int env_init(DB_ENV *, const char *);
int doloop (DB_ENV *, int);
int print_stocks(DB *);

static void
usage()
{
    fprintf(stderr, "usage: %s ", progname);
    fprintf(stderr, "-h home -l|-L host:port [-r host:port]\n");
    fprintf(stderr, "where:\n");
    fprintf(stderr, "\t-h identifies the environment home directory ");
    fprintf(stderr, "(required).\n");
    fprintf(stderr, "\t-l identifies the host and port used by this ");
    fprintf(stderr, "site (required unless L is specified).\n");
    fprintf(stderr, "\t-L identifies the local site as group creator. \n");
    fprintf(stderr, "\t-r identifies another site participating in "); 
    fprintf(stderr, "this replication group\n");
    exit(EXIT_FAILURE);
}

int
main(int argc, char *argv[])
{
    extern char *optarg;
    DB_ENV *dbenv;
    DB_SITE *dbsite;
    const char *home;
    char ch, *host, *last_colon, *portstr;
    int is_group_creator, local_is_set, ret;
    u_int16_t port;

    dbenv = NULL;

    ret = is_group_creator = local_is_set = 0;
    home = NULL;

    /* Create and configure the environment handle. */
    if ((ret = create_env(progname, &dbenv)) != 0)
	goto err;

    /* Collect the command line options. */
    while ((ch = getopt(argc, argv, "h:l:L:r:")) != EOF)
	switch (ch) {
	case 'h':
	    home = optarg;
	    break;
	case 'L':
	    is_group_creator = 1; /* FALLTHROUGH */
	case 'l':
	    host = optarg;
	    /*
	     * The final colon in host:port string is the
	     * boundary between the host and the port portions
	     * of the string.
	     */
	    if ((last_colon = strrchr(host, ':')) == NULL ) {
		fprintf(stderr, "Bad local host specification.\n");
		goto err;
	    }
	    /*
	     * Separate the host and port portions of the
	     * string for further processing.
	     */
	    portstr = last_colon + 1;
	    *last_colon = '\0';
	    port = (unsigned short)atoi(portstr);
	    if ((ret =
	      dbenv->repmgr_site(dbenv, host, port, &dbsite, 0)) != 0){
		fprintf(stderr, "Could not set local address %s:%d.\n",
		  host, port);
		goto err;
	    }
	    dbsite->set_config(dbsite, DB_LOCAL_SITE, 1);
	    if (is_group_creator)
		dbsite->set_config(dbsite, DB_GROUP_CREATOR, 1);

	    if ((ret = dbsite->close(dbsite)) != 0) {
		dbenv->err(dbenv, ret, "DB_SITE->close");
		goto err;
            }
	    local_is_set = 1;
	    break;
	/* Identify another site in the replication group. */
	case 'r':
	    host = optarg;
	    /*
	     * The final colon in host:port string is the
	     * boundary between the host and the port portions
	     * of the string.
	     */
	    if ((last_colon = strrchr(host, ':')) == NULL ) {
		fprintf(stderr, "Bad remote host specification.\n");
		goto err;
	    }
	    /*
	     * Separate the host and port portions of the
	     * string for further processing.
	     */
	    portstr = last_colon + 1;
	    *last_colon = '\0';
	    port = (unsigned short)atoi(portstr);
	    if ((ret = dbenv->repmgr_site(dbenv, host, port, &dbsite, 0)) != 0) {
		dbenv->err(dbenv, ret, "DB_ENV->repmgr_site");
		goto err;
	    }
	    dbsite->set_config(dbsite, DB_BOOTSTRAP_HELPER, 1);
	    if ((ret = dbsite->close(dbsite)) != 0) {
		dbenv->err(dbenv, ret, "DB_SITE->close");
		goto err;
	    }
	    break;
	case '?':
	default:
	    usage();
	}

    /* Error check command line. */
    if (home == NULL || !local_is_set)
	usage();

    /* Open the environment. */
    if ((ret = env_init(dbenv, home)) != 0)
      goto err;

    /* Configure Replication Manager write forwarding. */
    dbenv->rep_set_config(dbenv, DB_REPMGR_CONF_FORWARD_WRITES, 1);

    /* Start Replication Manager. */
    if ((ret = dbenv->repmgr_start(dbenv, 3, DB_REP_ELECTION)) != 0)
	goto err;

    if ((ret = doloop(dbenv, is_group_creator)) != 0) {
	dbenv->err(dbenv, ret, "Application failed");
	goto err;
    }

err: if (dbenv != NULL)
	(void)dbenv->close(dbenv, 0);

    return (ret);
}

/* Create and configure an environment handle. */
int
create_env(const char *progname, DB_ENV **dbenvp)
{
    DB_ENV *dbenv;
    int ret;

    if ((ret = db_env_create(&dbenv, 0)) != 0) {
	fprintf(stderr, "can't create env handle: %s\n",
	    db_strerror(ret));
	return (ret);
    }

    dbenv->set_errfile(dbenv, stderr);
    dbenv->set_errpfx(dbenv, progname);

    *dbenvp = dbenv;
    return (0);
}

/* Open and configure an environment. */
int
env_init(DB_ENV *dbenv, const char *home)
{
    u_int32_t flags;
    int ret;

    (void)dbenv->set_cachesize(dbenv, 0, CACHESIZE, 0);
    (void)dbenv->set_flags(dbenv, DB_TXN_NOSYNC, 1);

    /* DB_INIT_REP and DB_THREAD are required for Replication Manager. */
    flags = DB_CREATE |
	    DB_INIT_LOCK |
	    DB_INIT_LOG |
	    DB_INIT_MPOOL |
	    DB_INIT_REP |
	    DB_INIT_TXN |
	    DB_RECOVER |
	    DB_THREAD;
    if ((ret = dbenv->open(dbenv, home, flags, 0)) != 0)
	dbenv->err(dbenv, ret, "can't open environment");
    return (ret);
}

/*
 * Provides the main data processing function for our application.
 * This function provides a command line prompt to which the user
 * can provide a ticker string and a stock price.  Once a value is
 * entered to the application, the application writes the value to
 * the database and then displays the entire database.
 */
#define	BUFSIZE 1024
int
doloop(DB_ENV *dbenv, int is_group_creator)
{
    DB *dbp;
    DBT key, data;
    char buf[BUFSIZE], *rbuf;
    int ret;
    u_int32_t db_flags;

    dbp = NULL;
    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));
    ret = 0;

    for (;;) {
	if (dbp == NULL) {
	    if ((ret = db_create(&dbp, dbenv, 0)) != 0)
		return (ret);

	    db_flags = DB_AUTO_COMMIT;
	    /*
	     * Only need to create the database for the initial group creator
	     * startup.  The database will be replicated to the other sites
	     * when they first start up.  The database will already exist on
	     * each site for subsequent startups.
	     */
	    if (is_group_creator)
		db_flags |= DB_CREATE;

	    if ((ret = dbp->open(dbp,
		NULL, DATABASE, NULL, DB_BTREE, db_flags, 0)) != 0) {
		/* Retry in case site needs time to synchronize with master. */
		if (ret == ENOENT) {
		    printf(
		      "No stock database yet available.\n");
		    if ((ret = dbp->close(dbp, 0)) != 0) {
			dbenv->err(dbenv, ret, "DB->close");
			goto err;
		    }
		    dbp = NULL;
		    sleep(SLEEPTIME);
		    continue;
		}
		dbenv->err(dbenv, ret, "DB->open");
		goto err;
	    }
	}

	printf("QUOTESERVER> ");
	fflush(stdout);

	if (fgets(buf, sizeof(buf), stdin) == NULL)
	    break;
	if (strtok(&buf[0], " \t\n") == NULL) {
	    switch ((ret = print_stocks(dbp))) {
	    case 0:
		continue;
	    case DB_REP_HANDLE_DEAD:
		/* Must close and reopen the handle, then can retry. */
		(void)dbp->close(dbp, 0);
		dbp = NULL;
		dbenv->errx(dbenv, "Could not traverse data, retry operation");
		continue;
	    default:
		dbp->err(dbp, ret, "Error traversing data");
		goto err;
	    }
	}
	rbuf = strtok(NULL, " \t\n");
	if (rbuf == NULL || rbuf[0] == '\0') {
	    if (strncmp(buf, "exit", 4) == 0 ||
		strncmp(buf, "quit", 4) == 0)
		break;
	    dbenv->errx(dbenv, "Format: TICKER VALUE");
	    continue;
	}

	key.data = buf;
	key.size = (u_int32_t)strlen(buf);

	data.data = rbuf;
	data.size = (u_int32_t)strlen(rbuf);

	if ((ret = dbp->put(dbp, NULL, &key, &data, 0)) != 0)
	{
	    dbp->err(dbp, ret, "DB->put");
	    switch (ret) {
	    case DB_REP_HANDLE_DEAD:
		/* Must close and reopen the handle, then can retry. */
		(void)dbp->close(dbp, 0);
		dbp = NULL;
		/* FALLTHROUGH */
	    case DB_LOCK_DEADLOCK:
	    case DB_TIMEOUT:
	    case EACCES:
		/*
		 * Simply retry after a deadlock, timeout or permission error.
		 * A forwarded put operation can return a timeout error if
		 * the operation takes too long.  A forwarded put operation
		 * can return a permission error if there is currently no
		 * master.
		 */
		dbenv->errx(dbenv, "Could not update data, retry operation");
		continue;
	    default:
		dbp->err(dbp, ret, "Error updating data");
		goto err;
	    }
	}
    }
err: if (dbp != NULL)
	(void)dbp->close(dbp, 0);

    return (ret);
}

/* Display all the stock quote information in the database. */
int
print_stocks(DB *dbp)
{
    DBC *dbc;
    DBT key, data;
#define	MAXKEYSIZE  10
#define	MAXDATASIZE 20
    char keybuf[MAXKEYSIZE + 1], databuf[MAXDATASIZE + 1];
    int ret, t_ret;
    u_int32_t keysize, datasize;

    if ((ret = dbp->cursor(dbp, NULL, &dbc, 0)) != 0) {
	dbp->err(dbp, ret, "can't open cursor");
	return (ret);
    }

    memset(&key, 0, sizeof(key));
    memset(&data, 0, sizeof(data));

    printf("\tSymbol\tPrice\n");
    printf("\t======\t=====\n");

    for (ret = dbc->c_get(dbc, &key, &data, DB_FIRST);
	ret == 0;
	ret = dbc->c_get(dbc, &key, &data, DB_NEXT)) {
	keysize = key.size > MAXKEYSIZE ? MAXKEYSIZE : key.size;
	memcpy(keybuf, key.data, keysize);
	keybuf[keysize] = '\0';

	datasize = data.size >= MAXDATASIZE ? MAXDATASIZE : data.size;
	memcpy(databuf, data.data, datasize);
	databuf[datasize] = '\0';

	printf("\t%s\t%s\n", keybuf, databuf);
    }
    printf("\n");
    fflush(stdout);

    if ((t_ret = dbc->c_close(dbc)) != 0 && ret == 0)
	ret = t_ret;

    switch (ret) {
    case 0:
    case DB_NOTFOUND:
	return (0);
    default:
	return (ret);
    }
}
