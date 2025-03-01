/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2001, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <db.h>

#include "rep_base.h"

static int   connect_site __P((DB_ENV *, machtab_t *,
		 const char *, repsite_t *, int *, thread_t *));
static void *elect_thread __P((void *));
static void *hm_loop __P((void *));

typedef struct {
	DB_ENV *dbenv;
	machtab_t *machtab;
} elect_args;

typedef struct {
	DB_ENV *dbenv;
	const char *progname;
	const char *home;
	socket_t fd;
	u_int32_t eid;
	machtab_t *tab;
} hm_loop_args;

/*
 * This is a generic message handling loop that is used both by the
 * master to accept messages from a client as well as by clients
 * to communicate with other clients.
 */
static void *
hm_loop(args)
	void *args;
{
	DB_ENV *dbenv;
	DB_LSN permlsn;
	DBT rec, control;
	APP_DATA *app;
	const char *home, *progname;
	char *last_colon, *portstr;
	elect_args *ea;
	hm_loop_args *ha;
	machtab_t *tab;
	thread_t elect_thr, *site_thrs, *tmp, tid;
	repsite_t newsite;
	u_int32_t timeout;
	int eid, n, nsites, nsites_allocd;
	int already_open, r, ret, t_ret;
	socket_t fd;
	void *status;

	ea = NULL;
	site_thrs = NULL;
	nsites_allocd = 0;
	nsites = 0;

	ha = (hm_loop_args *)args;
	dbenv = ha->dbenv;
	fd = ha->fd;
	home = ha->home;
	eid = ha->eid;
	progname = ha->progname;
	tab = ha->tab;
	free(ha);
	app = dbenv->app_private;

	memset(&rec, 0, sizeof(DBT));
	memset(&control, 0, sizeof(DBT));

	for (ret = 0; ret == 0;) {
		if ((ret = get_next_message(fd, &rec, &control)) != 0) {
			if (app->shared_data.app_finished) {
				ret = 0;
				goto netclose;
			}
			/*
			 * Close this connection; if it's the master call
			 * for an election.
			 */
			closesocket(fd);
			if ((ret = machtab_rem(tab, eid, 1)) != 0)
				break;

			/*
			 * If I'm the master, I just lost a client and this
			 * thread is done.
			 */
			if (master_eid == SELF_EID)
				break;

			/*
			 * If I was talking with the master and the master
			 * went away, I need to call an election; else I'm
			 * done.
			 */
			if (master_eid != eid)
				break;

			master_eid = DB_EID_INVALID;
			machtab_parm(tab, &n, &timeout);
			(void)dbenv->rep_set_timeout(dbenv,
			    DB_REP_ELECTION_TIMEOUT, timeout);
			if ((ret = dbenv->rep_elect(dbenv,
			    n, (n/2+1), 0)) != 0)
				continue;

			/*
			 * Regardless of the results, the site I was talking
			 * to is gone, so I have nothing to do but exit.
			 */
			if (app->elected) {
				app->elected = 0;
				ret = dbenv->rep_start(dbenv,
				    NULL, DB_REP_MASTER);
			}
			break;
		}

		switch (r = dbenv->rep_process_message(dbenv,
		    &control, &rec, eid, &permlsn)) {
		case DB_REP_NEWSITE:
			/*
			 * Check if we got sent connect information and if we
			 * did, if this is me or if we already have a
			 * connection to this new site.  If we don't,
			 * establish a new one.
			 */

			/* No connect info. */
			if (rec.size == 0)
				break;

			newsite.host = (char *)rec.data;
			/*
			 * Parse for last colon in host:port string to
			 * allow for IPv6 addresses in which the host
			 * string itself contains colons.
			 */
			if ((last_colon = 
			    strrchr(newsite.host, ':')) == NULL ) {
				dbenv->errx(dbenv, "Bad host specification");
				goto out;
			}
			portstr = last_colon + 1;
			*last_colon = '\0';
			newsite.port = atoi(portstr);

			/* It's me, do nothing. */
			if (strcmp(myaddr, newsite.host) == 0)
				break;

			/*
			 * We try to connect to the new site.  If we can't,
			 * we treat it as an error since we know that the site
			 * should be up if we got a message from it (even
			 * indirectly).
			 */
			if (nsites == nsites_allocd) {
				/* Need to allocate more space. */
				if ((tmp = realloc(
				    site_thrs, (10 + nsites) *
				    sizeof(thread_t))) == NULL) {
					ret = errno;
					goto out;
				}
				site_thrs = tmp;
				nsites_allocd += 10;
			}
			if ((ret = connect_site(dbenv, tab, progname,
			    &newsite, &already_open, &tid)) != 0)
				goto out;
			if (!already_open)
				memcpy(&site_thrs
				    [nsites++], &tid, sizeof(thread_t));
			break;
		case DB_REP_HOLDELECTION:
			if (master_eid == SELF_EID)
				break;
			/* Make sure that previous election has finished. */
			if (ea != NULL) {
				if (thread_join(elect_thr, &status) != 0) {
					dbenv->errx(dbenv,
					    "thread join failure");
					goto out;
				}
				free(ea);
				ea = NULL;
			}
			if ((ea = calloc(sizeof(elect_args), 1)) == NULL) {
				dbenv->errx(dbenv, "can't allocate memory");
				ret = errno;
				goto out;
			}
			ea->dbenv = dbenv;
			ea->machtab = tab;
			if ((ret = thread_create(&elect_thr,
			     NULL, elect_thread, (void *)ea)) != 0) {
				dbenv->errx(dbenv,
				    "can't create election thread");
			}
			break;
		case DB_REP_ISPERM:
			break;
		case 0:
			if (app->elected) {
				app->elected = 0;
				if ((ret = dbenv->rep_start(dbenv,
				    NULL, DB_REP_MASTER)) != 0) {
					dbenv->err(dbenv, ret,
					    "can't start as master");
					goto out;
				}
			}
			break;
		default:
			dbenv->err(dbenv, r, "DB_ENV->rep_process_message");
			break;
		}
	}

out:	if ((t_ret = machtab_rem(tab, eid, 1)) != 0 && ret == 0)
		ret = t_ret;
netclose:
	/* Don't close the environment before any children exit. */
	if (ea != NULL) {
		if (thread_join(elect_thr, &status) != 0)
			dbenv->errx(dbenv, "can't join election thread");
		free(ea);
		ea = NULL;
	}

	if (site_thrs != NULL) {
		while (--nsites >= 0)
			if (thread_join(site_thrs[nsites], &status) != 0)
				dbenv->errx(dbenv, "can't join site thread");
		free(site_thrs);
	}

	return ((void *)(uintptr_t)ret);
}

/*
 * This is a generic thread that spawns a thread to listen for connections
 * on a socket and then spawns off child threads to handle each new
 * connection.
 */
void *
connect_thread(args)
	void *args;
{
	DB_ENV *dbenv;
	APP_DATA *app;
	const char *home, *host, *progname;
	hm_loop_args *ha;
	connect_args *cargs;
	machtab_t *machtab;
	thread_t hm_thrs[MAX_THREADS];
	void *status;
	int i, eid, port, ret;
	socket_t fd, ns;

	ha = NULL;
	i = 0;
	cargs = (connect_args *)args;
	dbenv = cargs->dbenv;
	home = cargs->home;
	progname = cargs->progname;
	machtab = cargs->machtab;
	host = cargs->host;
	port = cargs->port;
	app = dbenv->app_private;

	/*
	 * Loop forever, accepting connections from new machines,
	 * and forking off a thread to handle each.
	 */
	if ((fd = listen_socket_init(progname, host, port, machtab)) < 0) {
		ret = errno;
		goto err;
	}

	for (i = 0; i < MAX_THREADS; i++) {
		if ((ns = listen_socket_accept(machtab,
		    progname, fd, &eid)) == SOCKET_CREATION_FAILURE) {
			if (app->shared_data.app_finished)
				ret = 0;
			else
				ret = errno;
			goto err;
		}
		if ((ha = calloc(sizeof(hm_loop_args), 1)) == NULL) {
			dbenv->errx(dbenv, "can't allocate memory");
			ret = errno;
			goto err;
		}
		ha->progname = progname;
		ha->home = home;
		ha->fd = ns;
		ha->eid = eid;
		ha->tab = machtab;
		ha->dbenv = dbenv;
		if ((ret = thread_create(&hm_thrs[i], NULL,
		    hm_loop, (void *)ha)) != 0) {
			dbenv->errx(dbenv, "can't create thread for site");
			goto err;
		}
		ha = NULL;
	}

	/* If we fell out, we ended up with too many threads. */
	dbenv->errx(dbenv, "Too many threads");
	ret = ENOMEM;

err:
	/* Do not return until all threads have exited. */
	while (--i >= 0)
		if (thread_join(hm_thrs[i], &status) != 0)
			dbenv->errx(dbenv, "can't join site thread");

	return (ret == 0 ? (void *)EXIT_SUCCESS : (void *)EXIT_FAILURE);
}

/*
 * Open a connection to everyone that we've been told about.  If we
 * cannot open some connections, keep trying.
 */
void *
connect_all(args)
	void *args;
{
	DB_ENV *dbenv;
	APP_DATA *app;
	all_args *aa;
	const char *home, *progname;
	int failed, i, nsites, open, ret, *success;
	machtab_t *machtab;
	thread_t empty_id, *hm_thr;
	repsite_t *sites;
	void *status;

	aa = (all_args *)args;
	dbenv = aa->dbenv;
	progname = aa->progname;
	home = aa->home;
	machtab = aa->machtab;
	nsites = aa->nsites;
	sites = aa->sites;
	status = NULL;
	app = dbenv->app_private;
	memset(&empty_id, 0, sizeof(thread_t));

	ret = 0;
	hm_thr = NULL;
	success = NULL;

	/* Some implementations of calloc are sad about allocating 0 things. */
	if ((success = calloc(nsites > 0 ? nsites : 1, sizeof(int))) == NULL) {
		dbenv->err(dbenv, errno, "connect_all");
		ret = 1;
		goto err;
	}

	if (nsites > 0 && (hm_thr = calloc(nsites, sizeof(thread_t))) == NULL) {
		dbenv->err(dbenv, errno, "connect_all");
		ret = 1;
		goto err;
	}

	for (failed = nsites; failed > 0;) {
		for (i = 0; i < nsites; i++) {
			if (success[i])
				continue;

			if (app->shared_data.app_finished)
				goto err;

			ret = connect_site(dbenv, machtab,
			    progname, &sites[i], &open, &hm_thr[i]);

			/*
			 * If we couldn't make the connection, this isn't
			 * fatal to the loop, but we have nothing further
			 * to do on this machine at the moment.
			 */
			if (ret == DB_REP_UNAVAIL)
				continue;

			if (ret != 0)
				goto err;

			failed--;
			success[i] = 1;

			/* If the connection is already open, we're done. */
			if (ret == 0 && open == 1)
				continue;

		}
		sleep(1);
	}

err:	
	for (i = 0; i < nsites; i++) {
		if (!success[i] || 
		    memcmp(&hm_thr[i], &empty_id, sizeof(thread_t)) == 0)
			continue;
		if (thread_join(hm_thr[i], &status) != 0)
			dbenv->errx(dbenv, "can't join site thread");
	}
	if (success != NULL)
		free(success);
	if (hm_thr != NULL)
		free(hm_thr);
	return (ret ? (void *)EXIT_FAILURE : (void *)EXIT_SUCCESS);
}

static int
connect_site(dbenv, machtab, progname, site, is_open, hm_thrp)
	DB_ENV *dbenv;
	machtab_t *machtab;
	const char *progname;
	repsite_t *site;
	int *is_open;
	thread_t *hm_thrp;
{
	int eid, ret;
	socket_t s;
	hm_loop_args *ha;

	if ((s = get_connected_socket(machtab, progname,
	    site->host, site->port, is_open, &eid)) < 0)
		return (DB_REP_UNAVAIL);

	if (*is_open)
		return (0);

	if ((ha = calloc(sizeof(hm_loop_args), 1)) == NULL) {
		dbenv->errx(dbenv, "can't allocate memory");
		ret = errno;
		goto err;
	}

	ha->progname = progname;
	ha->fd = s;
	ha->eid = eid;
	ha->tab = machtab;
	ha->dbenv = dbenv;

	if ((ret = thread_create(hm_thrp, NULL,
	    hm_loop, (void *)ha)) != 0) {
		dbenv->errx(dbenv, "can't create thread for connected site");
		goto err1;
	}

	return (0);

err1:	free(ha);
err:
	return (ret);
}

/*
 * We need to spawn off a new thread in which to hold an election in
 * case we are the only thread listening on for messages.
 */
static void *
elect_thread(args)
	void *args;
{
	DB_ENV *dbenv;
	elect_args *eargs;
	machtab_t *machtab;
	u_int32_t timeout;
	int n, ret;
	APP_DATA *app;

	eargs = (elect_args *)args;
	dbenv = eargs->dbenv;
	machtab = eargs->machtab;
	free(eargs);
	app = dbenv->app_private;

	machtab_parm(machtab, &n, &timeout);
	(void)dbenv->rep_set_timeout(dbenv, DB_REP_ELECTION_TIMEOUT, timeout);
	while ((ret = dbenv->rep_elect(dbenv, n, (n/2+1), 0)) != 0) {
		if (app->shared_data.app_finished)
			return (NULL);
		sleep(2);
	}

	if (app->elected) {
		app->elected = 0;
		if ((ret = dbenv->rep_start(dbenv, NULL,
		    DB_REP_MASTER)) != 0 && !app->shared_data.app_finished)
			dbenv->err(dbenv, ret,
			    "can't start as master in election thread");
	}

	return (NULL);
}
