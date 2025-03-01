/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2005, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#include "db_config.h"

#include "db_int.h"
#include "dbinc/db_page.h"
#include "dbinc/btree.h"
#include "dbinc/txn.h"

#define	INITIAL_SITES_ALLOCATION	3	     /* Arbitrary guess. */

static int convert_gmdb(ENV *, DB_THREAD_INFO *, DB *, DB_TXN *);
static int get_eid __P((ENV *, const char *, u_int, int *));
static int read_gmdb __P((ENV *, DB_THREAD_INFO *, u_int8_t **, size_t *));
static int __repmgr_addrcmp __P((repmgr_netaddr_t *, repmgr_netaddr_t *));
static int __repmgr_find_commit __P((ENV *, DB_LSN *, DB_LSN *, int *));
static int __repmgr_remote_lsnhist(ENV *, int, u_int32_t,
    __repmgr_lsnhist_match_args *);

/*
 * Schedules a future attempt to re-establish a connection with the given site.
 * Usually, we wait the configured retry_wait period.  But if the "immediate"
 * parameter is given as TRUE, we'll make the wait time 0, and put the request
 * at the _beginning_ of the retry queue.
 *
 * PUBLIC: int __repmgr_schedule_connection_attempt __P((ENV *, int, int));
 *
 * !!!
 * Caller should hold mutex.
 *
 * Unless an error occurs, we always attempt to wake the main thread;
 * __repmgr_bust_connection relies on this behavior.
 */
int
__repmgr_schedule_connection_attempt(env, eid, immediate)
	ENV *env;
	int eid;
	int immediate;
{
	DB_REP *db_rep;
	REP *rep;
	REPMGR_RETRY *retry, *target;
	REPMGR_SITE *site;
	SITEINFO *sites;
	db_timeout_t timeout;
	db_timespec t;
	int ret;

	db_rep = env->rep_handle;
	rep = db_rep->region;
	if ((ret = __os_malloc(env, sizeof(*retry), &retry)) != 0)
		return (ret);

	DB_ASSERT(env, IS_VALID_EID(eid));
	site = SITE_FROM_EID(eid);
	__os_gettime(env, &t, 1);
	if (immediate)
		TAILQ_INSERT_HEAD(&db_rep->retries, retry, entries);
	else {
		/*
		 * Normally we retry a connection after connection retry
		 * timeout.  In a subordinate rep-aware process, we retry sooner
		 * when there is a listener candidate on the disconnected site.
		 * The listener process will be connected from the new listener,
		 * but subordinate rep-aware process can only wait for retry.
		 * It matters when the subordinate process becomes listener and
		 * the disconnected site is master.  The m_listener_wait is set
		 * to retry after enough time has passed for a takeover.  The
		 * number of listener candidates is maintained in the listener
		 * process as it has connections to all subordinate processes
		 * from other sites.
		*/
		timeout = rep->connection_retry_wait;
		CHECK_LISTENER_CAND(timeout, >0, db_rep->m_listener_wait,
		    timeout);
		TIMESPEC_ADD_DB_TIMEOUT(&t, timeout);

		/*
		 * Insert the new "retry" on the (time-ordered) list in its
		 * proper position.  To do so, find the list entry ("target")
		 * with a later time; insert the new entry just before that.
		 */
		TAILQ_FOREACH(target, &db_rep->retries, entries) {
			if (timespeccmp(&target->time, &t, >))
				break;
		}
		if (target == NULL)
			TAILQ_INSERT_TAIL(&db_rep->retries, retry, entries);
		else
			TAILQ_INSERT_BEFORE(target, retry, entries);

	}
	retry->eid = eid;
	retry->time = t;

	site->state = SITE_PAUSING;
	site->ref.retry = retry;

	return (__repmgr_wake_main_thread(env));
}

/*
 * Determines whether a remote site should be considered a "server" to us as a
 * "client" (in typical client/server terminology, not to be confused with our
 * usual use of the term "client" as in the master/client replication role), or
 * vice versa.
 *
 * PUBLIC: int __repmgr_is_server __P((ENV *, REPMGR_SITE *));
 */
int
__repmgr_is_server(env, site)
	ENV *env;
	REPMGR_SITE *site;
{
	DB_REP *db_rep;
	int cmp;

	db_rep = env->rep_handle;
	cmp = __repmgr_addrcmp(&site->net_addr,
	    &SITE_FROM_EID(db_rep->self_eid)->net_addr);
	DB_ASSERT(env, cmp != 0);

	/*
	 * The mnemonic here is that a server conventionally has a
	 * small well-known port number, while clients typically use a port
	 * number from the higher ephemeral range.  So, for the remote site to
	 * be considered a server, its address should have compared as lower
	 * than ours.
	 */
	return (cmp == -1);
}

/*
 * Compare two network addresses (lexicographically), and return -1, 0, or 1, as
 * the first is less than, equal to, or greater than the second.
 */
static int
__repmgr_addrcmp(addr1, addr2)
	repmgr_netaddr_t *addr1, *addr2;
{
	int cmp;

	cmp = strcmp(addr1->host, addr2->host);
	if (cmp != 0)
		return (cmp);

	if (addr1->port < addr2->port)
		return (-1);
	else if (addr1->port > addr2->port)
		return (1);
	return (0);
}

/*
 * Initialize the necessary control structures to begin reading a new input
 * message.
 *
 * PUBLIC: void __repmgr_reset_for_reading __P((REPMGR_CONNECTION *));
 */
void
__repmgr_reset_for_reading(con)
	REPMGR_CONNECTION *con;
{
	con->reading_phase = SIZES_PHASE;
	__repmgr_iovec_init(&con->iovecs);
	__repmgr_add_buffer(&con->iovecs,
	    con->msg_hdr_buf, __REPMGR_MSG_HDR_SIZE);
}

/*
 * Constructs a DB_REPMGR_CONNECTION structure.
 *
 * PUBLIC: int __repmgr_new_connection __P((ENV *,
 * PUBLIC:     REPMGR_CONNECTION **, socket_t, int));
 */
int
__repmgr_new_connection(env, connp, s, state)
	ENV *env;
	REPMGR_CONNECTION **connp;
	socket_t s;
	int state;
{
	REPMGR_CONNECTION *c;
	int ret;

	if ((ret = __os_calloc(env, 1, sizeof(REPMGR_CONNECTION), &c)) != 0)
		return (ret);
	if ((ret = __repmgr_alloc_cond(&c->drained)) != 0) {
		__os_free(env, c);
		return (ret);
	}
	if ((ret = __repmgr_init_waiters(env, &c->response_waiters)) != 0) {
		(void)__repmgr_free_cond(&c->drained);
		__os_free(env, c);
		return (ret);
	}

	c->fd = s;
	c->state = state;
	c->type = UNKNOWN_CONN_TYPE;
#ifdef DB_WIN32
	c->event_object = WSA_INVALID_EVENT;
#endif

	STAILQ_INIT(&c->outbound_queue);
	c->out_queue_length = 0;

	__repmgr_reset_for_reading(c);
	*connp = c;

	return (0);
}

/*
 * PUBLIC: int __repmgr_set_keepalive __P((ENV *, REPMGR_CONNECTION *));
 */
int
__repmgr_set_keepalive(env, conn)
	ENV *env;
	REPMGR_CONNECTION *conn;
{
	int ret, sockopt;

	ret = 0;
#ifdef SO_KEEPALIVE
	sockopt = 1;
	if (setsockopt(conn->fd, SOL_SOCKET,
	    SO_KEEPALIVE, (sockopt_t)&sockopt, sizeof(sockopt)) != 0) {
		ret = net_errno;
		__db_err(env, ret, DB_STR("3626",
			"can't set KEEPALIVE socket option"));
		(void)__repmgr_destroy_conn(env, conn);
	}
#endif
	return (ret);
}

/*
 * PUBLIC: int __repmgr_new_site __P((ENV *, REPMGR_SITE**,
 * PUBLIC:     const char *, u_int));
 *
 * Manipulates the process-local copy of the sites list.  So, callers should
 * hold the db_rep->mutex (except for single-threaded, pre-open configuration).
 */
int
__repmgr_new_site(env, sitep, host, port)
	ENV *env;
	REPMGR_SITE **sitep;
	const char *host;
	u_int port;
{
	DB_REP *db_rep;
	REPMGR_CONNECTION *conn;
	REPMGR_SITE *site, *sites;
	char *p;
	u_int i, new_site_max;
	int ret;

	db_rep = env->rep_handle;
	if (db_rep->site_cnt >= db_rep->site_max) {
		new_site_max = db_rep->site_max == 0 ?
		    INITIAL_SITES_ALLOCATION : db_rep->site_max * 2;
		if ((ret = __os_malloc(env,
		     sizeof(REPMGR_SITE) * new_site_max, &sites)) != 0)
			 return (ret);
		if (db_rep->site_max > 0) {
			/*
			 * For each site in the array, copy the old struct to
			 * the space allocated for the new struct.  But the
			 * sub_conns list header (and one of the conn structs on
			 * the list, if any) contain pointers to the address of
			 * the old list header; so we have to move them
			 * explicitly.  If not for that, we could use a simple
			 * __os_realloc() call.
			 */
			for (i = 0; i < db_rep->site_cnt; i++) {
				sites[i] = db_rep->sites[i];
				TAILQ_INIT(&sites[i].sub_conns);
				while (!TAILQ_EMPTY(
				    &db_rep->sites[i].sub_conns)) {
					conn = TAILQ_FIRST(
					    &db_rep->sites[i].sub_conns);
					TAILQ_REMOVE(
					    &db_rep->sites[i].sub_conns,
					    conn, entries);
					TAILQ_INSERT_TAIL(&sites[i].sub_conns,
					    conn, entries);
				}
			}
			__os_free(env, db_rep->sites);
		}
		db_rep->sites = sites;
		db_rep->site_max = new_site_max;
	}
	if ((ret = __os_strdup(env, host, &p)) != 0) {
		/* No harm in leaving the increased site_max intact. */
		return (ret);
	}
	site = &db_rep->sites[db_rep->site_cnt++];

	site->net_addr.host = p;
	site->net_addr.port = (u_int16_t)port;

	site->max_ack_gen = 0;
	ZERO_LSN(site->max_ack);
	site->ack_policy = 0;
	site->alignment = 0;
	site->flags = 0;
	timespecclear(&site->last_rcvd_timestamp);
	TAILQ_INIT(&site->sub_conns);
	site->connector = NULL;
	site->ref.conn.in = site->ref.conn.out = NULL;
	site->state = SITE_IDLE;

	site->membership = 0;
	site->gmdb_flags = 0;
	site->config = 0;

	*sitep = site;
	return (0);
}

/*
 * PUBLIC: int __repmgr_create_mutex __P((ENV *, mgr_mutex_t **));
 */
int
__repmgr_create_mutex(env, mtxp)
	ENV *env;
	mgr_mutex_t **mtxp;
{
	mgr_mutex_t *mtx;
	int ret;

	if ((ret = __os_malloc(env, sizeof(mgr_mutex_t), &mtx)) == 0 &&
	    (ret = __repmgr_create_mutex_pf(mtx)) != 0) {
		__os_free(env, mtx);
	}
	if (ret == 0)
		*mtxp = mtx;
	return (ret);
}

/*
 * PUBLIC: int __repmgr_destroy_mutex __P((ENV *, mgr_mutex_t *));
 */
int
__repmgr_destroy_mutex(env, mtx)
	ENV *env;
	mgr_mutex_t *mtx;
{
	int ret;

	ret = __repmgr_destroy_mutex_pf(mtx);
	__os_free(env, mtx);
	return (ret);
}

/*
 * Kind of like a destructor for a repmgr_netaddr_t: cleans up any subordinate
 * allocated memory pointed to by the addr, though it does not free the struct
 * itself.
 *
 * PUBLIC: void __repmgr_cleanup_netaddr __P((ENV *, repmgr_netaddr_t *));
 */
void
__repmgr_cleanup_netaddr(env, addr)
	ENV *env;
	repmgr_netaddr_t *addr;
{
	if (addr->host != NULL) {
		__os_free(env, addr->host);
		addr->host = NULL;
	}
}

/*
 * PUBLIC: void __repmgr_iovec_init __P((REPMGR_IOVECS *));
 */
void
__repmgr_iovec_init(v)
	REPMGR_IOVECS *v;
{
	v->offset = v->count = 0;
	v->total_bytes = 0;
}

/*
 * PUBLIC: void __repmgr_add_buffer __P((REPMGR_IOVECS *, void *, size_t));
 *
 * !!!
 * There is no checking for overflow of the vectors[5] array.
 */
void
__repmgr_add_buffer(v, address, length)
	REPMGR_IOVECS *v;
	void *address;
	size_t length;
{
	if (length > 0) {
		v->vectors[v->count].iov_base = address;
		v->vectors[v->count++].iov_len = (u_long)length;
		v->total_bytes += length;
	}
}

/*
 * PUBLIC: void __repmgr_add_dbt __P((REPMGR_IOVECS *, const DBT *));
 */
void
__repmgr_add_dbt(v, dbt)
	REPMGR_IOVECS *v;
	const DBT *dbt;
{
	if (dbt->size > 0) {
		v->vectors[v->count].iov_base = dbt->data;
		v->vectors[v->count++].iov_len = dbt->size;
		v->total_bytes += dbt->size;
	}
}

/*
 * Update a set of iovecs to reflect the number of bytes transferred in an I/O
 * operation, so that the iovecs can be used to continue transferring where we
 * left off.
 *     Returns TRUE if the set of buffers is now fully consumed, FALSE if more
 * remains.
 *
 * PUBLIC: int __repmgr_update_consumed __P((REPMGR_IOVECS *, size_t));
 */
int
__repmgr_update_consumed(v, byte_count)
	REPMGR_IOVECS *v;
	size_t byte_count;
{
	db_iovec_t *iov;
	int i;

	for (i = v->offset; ; i++) {
		DB_ASSERT(NULL, i < v->count && byte_count > 0);
		iov = &v->vectors[i];
		if (byte_count > iov->iov_len) {
			/*
			 * We've consumed (more than) this vector's worth.
			 * Adjust count and continue.
			 */
			byte_count -= iov->iov_len;
		} else {
			/*
			 * Adjust length of remaining portion of vector.
			 * byte_count can never be greater than iov_len, or we
			 * would not be in this section of the if clause.
			 */
			iov->iov_len -= (u_int32_t)byte_count;
			if (iov->iov_len > 0) {
				/*
				 * Still some left in this vector.  Adjust base
				 * address too, and leave offset pointing here.
				 */
				iov->iov_base = (void *)
				    ((u_int8_t *)iov->iov_base + byte_count);
				v->offset = i;
			} else {
				/*
				 * Consumed exactly to a vector boundary.
				 * Advance to next vector for next time.
				 */
				v->offset = i+1;
			}
			/*
			 * If offset has reached count, the entire thing is
			 * consumed.
			 */
			return (v->offset >= v->count);
		}
	}
}

/*
 * Builds a buffer containing our network address information, suitable for
 * publishing as cdata via a call to rep_start, and sets up the given DBT to
 * point to it.  The buffer is dynamically allocated memory, and the caller must
 * assume responsibility for it.
 *
 * PUBLIC: int __repmgr_prepare_my_addr __P((ENV *, DBT *));
 */
int
__repmgr_prepare_my_addr(env, dbt)
	ENV *env;
	DBT *dbt;
{
	DB_REP *db_rep;
	repmgr_netaddr_t addr;
	size_t size, hlen;
	u_int16_t port_buffer;
	u_int8_t *ptr;
	int ret;

	db_rep = env->rep_handle;
	LOCK_MUTEX(db_rep->mutex);
	addr = SITE_FROM_EID(db_rep->self_eid)->net_addr;
	UNLOCK_MUTEX(db_rep->mutex);
	/*
	 * The cdata message consists of the 2-byte port number, in network byte
	 * order, followed by the null-terminated host name string.
	 */
	port_buffer = htons(addr.port);
	size = sizeof(port_buffer) + (hlen = strlen(addr.host) + 1);
	if ((ret = __os_malloc(env, size, &ptr)) != 0)
		return (ret);

	DB_INIT_DBT(*dbt, ptr, size);

	memcpy(ptr, &port_buffer, sizeof(port_buffer));
	ptr = &ptr[sizeof(port_buffer)];
	memcpy(ptr, addr.host, hlen);

	return (0);
}

/*
 * !!!
 * This may only be called after threads have been started, because we don't
 * know the answer until we have established group membership (e.g., reading the
 * membership database).  That should be OK, because we only need this
 * for starting an election, or counting acks after sending a PERM message.
 *
 * PUBLIC: int __repmgr_get_nsites __P((ENV *, u_int32_t *));
 */
int
__repmgr_get_nsites(env, nsitesp)
	ENV *env;
	u_int32_t *nsitesp;
{
	DB_REP *db_rep;
	u_int32_t nsites;

	db_rep = env->rep_handle;

	if ((nsites = db_rep->region->config_nsites) == 0) {
		__db_errx(env, DB_STR("3672",
		    "Nsites unknown before repmgr_start()"));
		return (EINVAL);
	}
	*nsitesp = nsites;
	return (0);
}

/*
 * PUBLIC: int __repmgr_thread_failure __P((ENV *, int));
 */
int
__repmgr_thread_failure(env, why)
	ENV *env;
	int why;
{
	DB_REP *db_rep;
	DB_THREAD_INFO *ip;

	db_rep = env->rep_handle;
	ENV_ENTER(env, ip);
	LOCK_MUTEX(db_rep->mutex);
	(void)__repmgr_stop_threads(env);
	UNLOCK_MUTEX(db_rep->mutex);
	ENV_LEAVE(env, ip);
	return (__env_panic(env, why));
}

/*
 * Format a printable representation of a site location, suitable for inclusion
 * in an error message.  The buffer must be at least as big as
 * MAX_SITE_LOC_STRING.
 *
 * PUBLIC: char *__repmgr_format_eid_loc __P((DB_REP *,
 * PUBLIC:    REPMGR_CONNECTION *, char *));
 *
 * Caller must hold mutex.
 */
char *
__repmgr_format_eid_loc(db_rep, conn, buffer)
	DB_REP *db_rep;
	REPMGR_CONNECTION *conn;
	char *buffer;
{
	int eid;

	if (conn->type == APP_CONNECTION)
		snprintf(buffer,
		    MAX_SITE_LOC_STRING, "(application channel)");
	else if (conn->type == REP_CONNECTION &&
	    IS_VALID_EID(eid = conn->eid))
		(void)__repmgr_format_site_loc(SITE_FROM_EID(eid), buffer);
	else
		snprintf(buffer, MAX_SITE_LOC_STRING, "(unidentified site)");
	return (buffer);
}

/*
 * PUBLIC: char *__repmgr_format_site_loc __P((REPMGR_SITE *, char *));
 */
char *
__repmgr_format_site_loc(site, buffer)
	REPMGR_SITE *site;
	char *buffer;
{
	return (__repmgr_format_addr_loc(&site->net_addr, buffer));
}

/*
 * PUBLIC: char *__repmgr_format_addr_loc __P((repmgr_netaddr_t *, char *));
 */
char *
__repmgr_format_addr_loc(addr, buffer)
	repmgr_netaddr_t *addr;
	char *buffer;
{
	snprintf(buffer, MAX_SITE_LOC_STRING, "site %s:%lu",
	    addr->host, (u_long)addr->port);
	return (buffer);
}

/*
 * PUBLIC: int __repmgr_repstart __P((ENV *, u_int32_t, u_int32_t));
 */
int
__repmgr_repstart(env, flags, startopts)
	ENV *env;
	u_int32_t flags;
	u_int32_t startopts;
{
	DBT my_addr;
	int ret;

	/* Include "cdata" in case sending to old-version site. */
	if ((ret = __repmgr_prepare_my_addr(env, &my_addr)) != 0)
		return (ret);
	/*
	 * force_role_chg and hold_client_gen are used by preferred master
	 * mode to help control site startup.
	 */
	ret = __rep_start_int(env, &my_addr, flags, startopts);
	__os_free(env, my_addr.data);
	if (ret != 0)
		__db_err(env, ret, DB_STR("3673", "rep_start"));
	return (ret);
}

/*
 * PUBLIC: int __repmgr_become_master __P((ENV *, u_int32_t));
 */
int
__repmgr_become_master(env, startopts)
	ENV *env;
	u_int32_t startopts;
{
	DB_REP *db_rep;
	DB_THREAD_INFO *ip;
	DB *dbp;
	DB_TXN *txn;
	REPMGR_SITE *site;
	DBT key_dbt, data_dbt;
	__repmgr_membership_key_args key;
	__repmgr_membership_data_args member_data;
	repmgr_netaddr_t addr;
	u_int32_t status;
	u_int8_t data_buf[__REPMGR_MEMBERSHIP_DATA_SIZE];
	u_int8_t key_buf[MAX_MSG_BUF];
	size_t len;
	u_int i;
	int ret, t_ret;

	db_rep = env->rep_handle;
	dbp = NULL;
	txn = NULL;

	/* Examine membership list to see if we have a victim in limbo. */
	LOCK_MUTEX(db_rep->mutex);
	ZERO_LSN(db_rep->limbo_failure);
	ZERO_LSN(db_rep->durable_lsn);
	db_rep->limbo_victim = DB_EID_INVALID;
	db_rep->limbo_resolution_needed = FALSE;
	FOR_EACH_REMOTE_SITE_INDEX(i) {
		site = SITE_FROM_EID(i);
		if (site->membership == SITE_ADDING ||
		    site->membership == SITE_DELETING) {
			db_rep->limbo_victim = (int)i;
			db_rep->limbo_resolution_needed = TRUE;

			/*
			 * Since there can never be more than one limbo victim,
			 * when we find one we don't have to continue looking
			 * for others.
			 */
			break;
		}
	}
	db_rep->client_intent = FALSE;
	UNLOCK_MUTEX(db_rep->mutex);

	if ((ret = __repmgr_repstart(env, DB_REP_MASTER, startopts)) != 0)
		return (ret);

	/*
	 * Make sure member_version_gen is current so that this master
	 * can reject obsolete member lists from other sites.
	 */
	db_rep->member_version_gen = db_rep->region->gen;

	/* If there is already a gmdb, we are finished. */
	if (db_rep->have_gmdb)
		return (0);

	/* There isn't a gmdb.  Create one from the in-memory site list. */
	if ((ret = __repmgr_hold_master_role(env, NULL, 0)) != 0)
		goto leave;
	ENV_GET_THREAD_INFO(env, ip);
retry:
	if ((ret = __repmgr_setup_gmdb_op(env, ip, &txn, DB_CREATE)) != 0)
		goto err;

	DB_ASSERT(env, txn != NULL);
	dbp = db_rep->gmdb;
	DB_ASSERT(env, dbp != NULL);

	/* Write the meta-data record. */
	if ((ret = __repmgr_set_gm_version(env, ip, txn, 1)) != 0)
		goto err;

	/* Write a record representing each site in the group. */
	for (i = 0; i < db_rep->site_cnt; i++) {
		LOCK_MUTEX(db_rep->mutex);
		site = SITE_FROM_EID(i);
		addr = site->net_addr;
		status = site->membership;
		UNLOCK_MUTEX(db_rep->mutex);
		if (status == 0)
			continue;
		DB_INIT_DBT(key.host, addr.host, strlen(addr.host) + 1);
		key.port = addr.port;
		ret = __repmgr_membership_key_marshal(env,
		    &key, key_buf, sizeof(key_buf), &len);
		DB_ASSERT(env, ret == 0);
		DB_INIT_DBT(key_dbt, key_buf, len);
		member_data.status = status;
		member_data.flags = site->gmdb_flags;
		__repmgr_membership_data_marshal(env, &member_data, data_buf);
		DB_INIT_DBT(data_dbt, data_buf, __REPMGR_MEMBERSHIP_DATA_SIZE);
		if ((ret = __db_put(dbp, ip, txn, &key_dbt, &data_dbt, 0)) != 0)
			goto err;
	}

err:
	if (txn != NULL) {
		if ((t_ret = __db_txn_auto_resolve(env, txn, 0, ret)) != 0 &&
		    ret == 0)
			ret = t_ret;
		if ((t_ret = __repmgr_cleanup_gmdb_op(env, TRUE)) != 0 &&
		    ret == 0)
			ret = t_ret;
	}
	if (ret == DB_LOCK_DEADLOCK || ret == DB_LOCK_NOTGRANTED)
		goto retry;
	if ((t_ret = __repmgr_rlse_master_role(env)) != 0 && ret == 0)
		ret = t_ret;
leave:
	return (ret);
}

/*
 * Visits all the connections we know about, performing the desired action.
 * "err_quit" determines whether we give up, or soldier on, in case of an
 * error.
 *
 * PUBLIC: int __repmgr_each_connection __P((ENV *,
 * PUBLIC:     CONNECTION_ACTION, void *, int));
 *
 * !!!
 * Caller must hold mutex.
 */
int
__repmgr_each_connection(env, callback, info, err_quit)
	ENV *env;
	CONNECTION_ACTION callback;
	void *info;
	int err_quit;
{
	DB_REP *db_rep;
	REPMGR_CONNECTION *conn, *next;
	REPMGR_SITE *site;
	int eid, ret, t_ret;

#define	HANDLE_ERROR		        \
	do {			        \
		if (err_quit)	        \
			return (t_ret); \
		if (ret == 0)	        \
			ret = t_ret;    \
	} while (0)

	db_rep = env->rep_handle;
	ret = 0;

	/*
	 * We might have used TAILQ_FOREACH here, except that in some cases we
	 * need to unlink an element along the way.
	 */
	for (conn = TAILQ_FIRST(&db_rep->connections);
	     conn != NULL;
	     conn = next) {
		next = TAILQ_NEXT(conn, entries);

		if ((t_ret = (*callback)(env, conn, info)) != 0)
			HANDLE_ERROR;
	}

	FOR_EACH_REMOTE_SITE_INDEX(eid) {
		site = SITE_FROM_EID(eid);

		if (site->state == SITE_CONNECTED) {
			if ((conn = site->ref.conn.in) != NULL &&
			    (t_ret = (*callback)(env, conn, info)) != 0)
				HANDLE_ERROR;
			if ((conn = site->ref.conn.out) != NULL &&
			    (t_ret = (*callback)(env, conn, info)) != 0)
				HANDLE_ERROR;
		}

		for (conn = TAILQ_FIRST(&site->sub_conns);
		     conn != NULL;
		     conn = next) {
			next = TAILQ_NEXT(conn, entries);
			if ((t_ret = (*callback)(env, conn, info)) != 0)
				HANDLE_ERROR;
		}
	}

	return (0);
}

/*
 * Initialize repmgr's portion of the shared region area.  Note that we can't
 * simply get the REP* address from the env as we usually do, because at the
 * time of this call it hasn't been linked into there yet.
 *
 * This function is only called during creation of the region.  If anything
 * fails, our caller will panic and remove the region.  So, if we have any
 * failure, we don't have to clean up any partial allocation.
 *
 * PUBLIC: int __repmgr_open __P((ENV *, void *));
 */
int
__repmgr_open(env, rep_)
	ENV *env;
	void *rep_;
{
	DB_REP *db_rep;
	REP *rep;
	int ret;

	db_rep = env->rep_handle;
	rep = rep_;

	if ((ret = __mutex_alloc(env, MTX_REPMGR, 0, &rep->mtx_repmgr)) != 0)
		return (ret);

	DB_ASSERT(env, rep->siteinfo_seq == 0 && db_rep->siteinfo_seq == 0);
	rep->siteinfo_off = INVALID_ROFF;
	rep->siteinfo_seq = 0;
	if ((ret = __repmgr_share_netaddrs(env, rep, 0, db_rep->site_cnt)) != 0)
		return (ret);

	rep->self_eid = db_rep->self_eid;
	rep->perm_policy = db_rep->perm_policy;
	rep->ack_timeout = db_rep->ack_timeout;
	rep->connection_retry_wait = db_rep->connection_retry_wait;
	rep->election_retry_wait = db_rep->election_retry_wait;
	rep->heartbeat_monitor_timeout = db_rep->heartbeat_monitor_timeout;
	rep->heartbeat_frequency = db_rep->heartbeat_frequency;
	rep->inqueue_max_gbytes = db_rep->inqueue_max_gbytes;
	rep->inqueue_max_bytes = db_rep->inqueue_max_bytes;
	rep->write_forward_timeout = db_rep->write_forward_timeout;
	if (rep->inqueue_max_gbytes == 0 && rep->inqueue_max_bytes == 0) {
		rep->inqueue_max_bytes = DB_REPMGR_DEFAULT_INQUEUE_MAX;
	}
	__repmgr_set_incoming_queue_redzone(rep, rep->inqueue_max_gbytes,
	    rep->inqueue_max_bytes);

	return (ret);
}

/*
 * Join an existing environment, by setting up our local site info structures
 * from shared network address configuration in the region.
 *
 * As __repmgr_open(), note that we can't simply get the REP* address from the
 * env as we usually do, because at the time of this call it hasn't been linked
 * into there yet.
 *
 * PUBLIC: int __repmgr_join __P((ENV *, void *));
 */
int
__repmgr_join(env, rep_)
	ENV *env;
	void *rep_;
{
	DB_REP *db_rep;
	REGINFO *infop;
	REP *rep;
	SITEINFO *p;
	REPMGR_SITE *site, temp;
	repmgr_netaddr_t *addrp;
	char *host;
	u_int i, j;
	int ret;

	db_rep = env->rep_handle;
	infop = env->reginfo;
	rep = rep_;
	ret = 0;

	MUTEX_LOCK(env, rep->mtx_repmgr);

	/*
	 * Merge local and shared lists of remote sites.  Note that the
	 * placement of entries in the shared array must not change.  To
	 * accomplish the merge, pull in entries from the shared list, into the
	 * proper position, shuffling not-yet-resolved local entries if
	 * necessary.  Then add any remaining locally known entries to the
	 * shared list.
	 */
	i = 0;
	if (rep->siteinfo_off != INVALID_ROFF) {
		p = R_ADDR(infop, rep->siteinfo_off);

		/* For each address in the shared list ... */
		for (; i < rep->site_cnt; i++) {
			host = R_ADDR(infop, p[i].addr.host);

			RPRINT(env, (env, DB_VERB_REPMGR_MISC,
			    "Site %s:%lu found at EID %u",
				host, (u_long)p[i].addr.port, i));
			/*
			 * Find it in the local list.  Everything before 'i'
			 * already matches the shared list, and is therefore in
			 * the right place.  So we only need to search starting
			 * from 'i'.  When found, local config values will be
			 * used because they are assumed to be "fresher".  But
			 * membership status is not, since this process hasn't
			 * been active (running) yet.
			 */
			for (j = i; j < db_rep->site_cnt; j++) {
				site = &db_rep->sites[j];
				addrp = &site->net_addr;
				if (strcmp(host, addrp->host) == 0 &&
				    p[i].addr.port == addrp->port) {
					p[i].config = site->config;
					site->membership = p[i].status;
					break;
				}
			}

			/*
			 * When not found in local list, copy peer values
			 * from shared list.
			 */
			if (j == db_rep->site_cnt) {
				if ((ret = __repmgr_new_site(env,
				    &site, host, p[i].addr.port)) != 0)
					goto unlock;
				site->config = p[i].config;
				site->membership = p[i].status;
			}
			DB_ASSERT(env, j < db_rep->site_cnt);

			/* Found or added at 'j', but belongs at 'i': swap. */
			if (i != j) {
				temp = db_rep->sites[j];
				db_rep->sites[j] = db_rep->sites[i];
				db_rep->sites[i] = temp;
				/*
				 * If we're moving the entry that self_eid
				 * points to, then adjust self_eid to match.
				 * For now this is still merely our original,
				 * in-process pointer; we have yet to make sure
				 * it matches the one from shared memory.
				 */
				if (db_rep->self_eid == (int)j)
					db_rep->self_eid = (int)i;
			}
		}
	}
	if ((ret = __repmgr_share_netaddrs(env, rep, i, db_rep->site_cnt)) != 0)
		goto unlock;
	if (db_rep->self_eid == DB_EID_INVALID)
		db_rep->self_eid = rep->self_eid;
	else if (rep->self_eid == DB_EID_INVALID)
		rep->self_eid = db_rep->self_eid;
	else if (db_rep->self_eid != rep->self_eid) {
		__db_errx(env, DB_STR("3674",
    "A mismatching local site address has been set in the environment"));
		ret = EINVAL;
		goto unlock;
	}

	db_rep->siteinfo_seq = rep->siteinfo_seq;
	/*
	 * Update the incoming queue limit settings if necessary.
	 */
	if ((db_rep->inqueue_max_gbytes != 0 ||
	    db_rep->inqueue_max_bytes != 0) &&
	    (db_rep->inqueue_max_gbytes != rep->inqueue_max_gbytes ||
	     db_rep->inqueue_max_bytes != rep->inqueue_max_gbytes)) {
		rep->inqueue_max_gbytes = db_rep->inqueue_max_gbytes;
		rep->inqueue_max_bytes = db_rep->inqueue_max_bytes;
		__repmgr_set_incoming_queue_redzone(rep,
		    rep->inqueue_max_gbytes, rep->inqueue_max_bytes);
	}
unlock:
	MUTEX_UNLOCK(env, rep->mtx_repmgr);
	return (ret);
}

/*
 * PUBLIC: int __repmgr_env_refresh __P((ENV *env));
 */
int
__repmgr_env_refresh(env)
	ENV *env;
{
	DB_REP *db_rep;
	REP *rep;
	REGINFO *infop;
	SITEINFO *shared_array;
	u_int i;
	int ret;

	db_rep = env->rep_handle;
	rep = db_rep->region;
	infop = env->reginfo;
	ret = 0;
	COMPQUIET(i, 0);

	if (F_ISSET(env, ENV_PRIVATE)) {
		ret = __mutex_free(env, &rep->mtx_repmgr);
		if (rep->siteinfo_off != INVALID_ROFF) {
			shared_array = R_ADDR(infop, rep->siteinfo_off);
			for (i = 0; i < db_rep->site_cnt; i++)
				__env_alloc_free(infop, R_ADDR(infop,
				    shared_array[i].addr.host));
			__env_alloc_free(infop, shared_array);
			rep->siteinfo_off = INVALID_ROFF;
		}
	}

	return (ret);
}

/*
 * Copies new remote site information from the indicated private array slots
 * into the shared region.  The corresponding shared array slots do not exist
 * yet; they must be allocated.
 *
 * PUBLIC: int __repmgr_share_netaddrs __P((ENV *, void *, u_int, u_int));
 *
 * !!! The rep pointer is passed, because it may not yet have been installed
 * into the env handle.
 *
 * !!! Assumes caller holds mtx_repmgr lock.
 */
int
__repmgr_share_netaddrs(env, rep_, start, limit)
	ENV *env;
	void *rep_;
	u_int start, limit;
{
	DB_REP *db_rep;
	REP *rep;
	REGINFO *infop;
	REGENV *renv;
	SITEINFO *orig, *shared_array;
	char *host, *hostbuf;
	size_t sz;
	u_int i, n;
	int eid, ret, touched;

	db_rep = env->rep_handle;
	infop = env->reginfo;
	renv = infop->primary;
	rep = rep_;
	ret = 0;
	touched = FALSE;

	MUTEX_LOCK(env, renv->mtx_regenv);

	for (i = start; i < limit; i++) {
		if (rep->site_cnt >= rep->site_max) {
			/* Table is full, we need more space. */
			if (rep->siteinfo_off == INVALID_ROFF) {
				n = INITIAL_SITES_ALLOCATION;
				sz = n * sizeof(SITEINFO);
				if ((ret = __env_alloc(infop,
				    sz, &shared_array)) != 0)
					goto out;
			} else {
				n = 2 * rep->site_max;
				sz = n * sizeof(SITEINFO);
				if ((ret = __env_alloc(infop,
				    sz, &shared_array)) != 0)
					goto out;
				orig = R_ADDR(infop, rep->siteinfo_off);
				memcpy(shared_array, orig,
				    sizeof(SITEINFO) * rep->site_cnt);
				__env_alloc_free(infop, orig);
			}
			rep->siteinfo_off = R_OFFSET(infop, shared_array);
			rep->site_max = n;
		} else
			shared_array = R_ADDR(infop, rep->siteinfo_off);

		DB_ASSERT(env, rep->site_cnt < rep->site_max &&
		    rep->siteinfo_off != INVALID_ROFF);

		host = db_rep->sites[i].net_addr.host;
		sz = strlen(host) + 1;
		if ((ret = __env_alloc(infop, sz, &hostbuf)) != 0)
			goto out;
		eid = (int)rep->site_cnt++;
		(void)strcpy(hostbuf, host);
		shared_array[eid].addr.host = R_OFFSET(infop, hostbuf);
		shared_array[eid].addr.port = db_rep->sites[i].net_addr.port;
		shared_array[eid].config = db_rep->sites[i].config;
		shared_array[eid].status = db_rep->sites[i].membership;
		shared_array[eid].flags = db_rep->sites[i].gmdb_flags;
		RPRINT(env, (env, DB_VERB_REPMGR_MISC,
		    "EID %d is assigned for site %s:%lu",
			eid, host, (u_long)shared_array[eid].addr.port));
		touched = TRUE;
	}

out:
	if (touched)
		db_rep->siteinfo_seq = ++rep->siteinfo_seq;
	MUTEX_UNLOCK(env, renv->mtx_regenv);
	return (ret);
}

/*
 * Copy into our local list any newly added/changed remote site
 * configuration information.
 *
 * !!! Caller must hold db_rep->mutex and mtx_repmgr locks.
 *
 * PUBLIC: int __repmgr_copy_in_added_sites __P((ENV *));
 */
int
__repmgr_copy_in_added_sites(env)
	ENV *env;
{
	DB_REP *db_rep;
	REP *rep;
	REGINFO *infop;
	SITEINFO *base, *p;
	REPMGR_SITE *site;
	char *host;
	int ret;
	u_int i;

	db_rep = env->rep_handle;
	rep = db_rep->region;

	if (rep->siteinfo_off == INVALID_ROFF)
		goto out;

	infop = env->reginfo;
	base = R_ADDR(infop, rep->siteinfo_off);

	/* Create private array slots for new sites. */
	for (i = db_rep->site_cnt; i < rep->site_cnt; i++) {
		p = &base[i];
		host = R_ADDR(infop, p->addr.host);
		if ((ret = __repmgr_new_site(env,
		    &site, host, p->addr.port)) != 0)
			return (ret);
		RPRINT(env, (env, DB_VERB_REPMGR_MISC,
		    "Site %s:%lu found at EID %u",
			host, (u_long)p->addr.port, i));
	}

	/* Make sure info is up to date for all sites, old and new. */
	for (i = 0; i < db_rep->site_cnt; i++) {
		p = &base[i];
		site = SITE_FROM_EID(i);
		site->config = p->config;
		site->membership = p->status;
		site->gmdb_flags = p->flags;
	}

out:
	/*
	 * We always make sure our local list has been brought up to date with
	 * the shared list before adding to the local list (except before env
	 * open of course).  So here there should be nothing on our local list
	 * not yet in shared memory.
	 */
	DB_ASSERT(env, db_rep->site_cnt == rep->site_cnt);
	db_rep->siteinfo_seq = rep->siteinfo_seq;
	return (0);
}

/*
 * Initialize a range of sites newly added to our site list array.  Process each
 * array entry in the range from <= x < limit.  Passing from >= limit is
 * allowed, and is effectively a no-op.
 *
 * PUBLIC: int __repmgr_init_new_sites __P((ENV *, int, int));
 *
 * !!! Assumes caller holds db_rep->mutex.
 */
int
__repmgr_init_new_sites(env, from, limit)
	ENV *env;
	int from, limit;
{
	DB_REP *db_rep;
	REPMGR_SITE *site;
	int i, ret;

	db_rep = env->rep_handle;

	if (db_rep->selector == NULL)
		return (0);

	DB_ASSERT(env, IS_VALID_EID(from) && IS_VALID_EID(limit) &&
	    from <= limit);
	for (i = from; i < limit; i++) {
		site = SITE_FROM_EID(i);
		if (site->membership == SITE_PRESENT &&
		    (ret = __repmgr_schedule_connection_attempt(env,
		    i, TRUE)) != 0)
			return (ret);
	}

	return (0);
}

/*
 * PUBLIC: int __repmgr_failchk __P((ENV *));
 */
int
__repmgr_failchk(env)
	ENV *env;
{
	DB_ENV *dbenv;
	DB_REP *db_rep;
	REP *rep;
	db_threadid_t unused;

	dbenv = env->dbenv;
	db_rep = env->rep_handle;
	rep = db_rep->region;

	DB_THREADID_INIT(unused);
	MUTEX_LOCK(env, rep->mtx_repmgr);

	/*
	 * Check to see if the main (listener) replication process may have died
	 * without cleaning up the flag.  If so, we only have to clear it, and
	 * another process should then be able to come along and become the
	 * listener.  So in either case we can return success.
	 */
	if (rep->listener != 0 && !dbenv->is_alive(dbenv,
	    rep->listener, unused, DB_MUTEX_PROCESS_ONLY))
		rep->listener = 0;
	MUTEX_UNLOCK(env, rep->mtx_repmgr);

	return (0);
}

/*
 * PUBLIC: int __repmgr_master_is_known __P((ENV *));
 */
int
__repmgr_master_is_known(env)
	ENV *env;
{
	DB_REP *db_rep;
	REPMGR_CONNECTION *conn;
	REPMGR_SITE *master;

	db_rep = env->rep_handle;

	/*
	 * We are the master, or we know of a master and have a healthy
	 * connection to it.
	 */
	if (db_rep->region->master_id == db_rep->self_eid)
		return (TRUE);
	if ((master = __repmgr_connected_master(env)) == NULL)
		return (FALSE);
	if ((conn = master->ref.conn.in) != NULL &&
	    IS_READY_STATE(conn->state))
		return (TRUE);
	if ((conn = master->ref.conn.out) != NULL &&
	    IS_READY_STATE(conn->state))
		return (TRUE);
	return (FALSE);
}

/*
 * PUBLIC: int __repmgr_stable_lsn __P((ENV *, DB_LSN *));
 *
 * This function may be called before any of repmgr's threads have
 * been started.  This code must not be called before env open.
 * Currently that is impossible since its only caller is log_archive
 * which itself cannot be called before env_open.
 */
int
__repmgr_stable_lsn(env, stable_lsn)
	ENV *env;
	DB_LSN *stable_lsn;
{
	DB_REP *db_rep;
	REP *rep;

	db_rep = env->rep_handle;
	rep = db_rep->region;

	LOCK_MUTEX(db_rep->mutex);
	if (rep->sites_avail != 0 && rep->min_log_file != 0 &&
	    rep->min_log_file < stable_lsn->file) {
		/*
		 * Returning an LSN to be consistent with the rest of the
		 * log archiving processing.  Construct LSN of format
		 * [filenum][0].
		 */
		stable_lsn->file = rep->min_log_file;
		stable_lsn->offset = 0;
	}
	RPRINT(env, (env, DB_VERB_REPMGR_MISC,
"Repmgr_stable_lsn: Returning stable_lsn[%lu][%lu] sites_avail %lu min_log %lu",
	    (u_long)stable_lsn->file, (u_long)stable_lsn->offset,
	    (u_long)rep->sites_avail, (u_long)rep->min_log_file));
	UNLOCK_MUTEX(db_rep->mutex);
	return (0);
}

/*
 * PUBLIC: int __repmgr_make_request_conn __P((ENV *,
 * PUBLIC:     repmgr_netaddr_t *, REPMGR_CONNECTION **));
 */
int
__repmgr_make_request_conn(env, addr, connp)
	ENV *env;
	repmgr_netaddr_t *addr;
	REPMGR_CONNECTION **connp;
{
	DBT vi;
	__repmgr_msg_hdr_args msg_hdr;
	__repmgr_version_confirmation_args conf;
	REPMGR_CONNECTION *conn;
	int alloc, ret, unused;

	alloc = FALSE;
	if ((ret = __repmgr_connect(env, addr, &conn, &unused)) != 0)
		return (ret);
	conn->type = APP_CONNECTION;

	/* Read a handshake msg, to get version confirmation and parameters. */
	if ((ret = __repmgr_read_conn(conn)) != 0)
		goto err;
	/*
	 * We can only get here after having read the full 9 bytes that we
	 * expect, so this can't fail.
	 */
	DB_ASSERT(env, conn->reading_phase == SIZES_PHASE);
	ret = __repmgr_msg_hdr_unmarshal(env, &msg_hdr,
	    conn->msg_hdr_buf, __REPMGR_MSG_HDR_SIZE, NULL);
	DB_ASSERT(env, ret == 0);
	__repmgr_iovec_init(&conn->iovecs);
	conn->reading_phase = DATA_PHASE;

	if ((ret = __repmgr_prepare_simple_input(env, conn, &msg_hdr)) != 0)
		goto err;
	alloc = TRUE;

	if ((ret = __repmgr_read_conn(conn)) != 0)
		goto err;

	/*
	 * Analyze the handshake msg, and stash relevant info.
	 */
	if ((ret = __repmgr_find_version_info(env, conn, &vi)) != 0)
		goto err;
	DB_ASSERT(env, vi.size > 0);
	if ((ret = __repmgr_version_confirmation_unmarshal(env,
	    &conf, vi.data, vi.size, NULL)) != 0)
		goto err;

	if (conf.version < GM_MIN_VERSION ||
	    (IS_VIEW_SITE(env) && conf.version < VIEW_MIN_VERSION) ||
	    (PREFMAS_IS_SET(env) && conf.version < PREFMAS_MIN_VERSION)) {
		ret = DB_REP_UNAVAIL;
		goto err;
	}
	conn->version = conf.version;

err:
	if (alloc) {
		DB_ASSERT(env, conn->input.repmgr_msg.cntrl.size > 0);
		__os_free(env, conn->input.repmgr_msg.cntrl.data);
		DB_ASSERT(env, conn->input.repmgr_msg.rec.size > 0);
		__os_free(env, conn->input.repmgr_msg.rec.data);
	}
	__repmgr_reset_for_reading(conn);
	if (ret == 0)
		*connp = conn;
	else {
		(void)__repmgr_close_connection(env, conn);
		(void)__repmgr_destroy_conn(env, conn);
	}
	return (ret);
}

/*
 * PUBLIC: int __repmgr_send_sync_msg __P((ENV *, REPMGR_CONNECTION *,
 * PUBLIC:     u_int32_t, u_int8_t *, u_int32_t));
 */
int
__repmgr_send_sync_msg(env, conn, type, buf, len)
	ENV *env;
	REPMGR_CONNECTION *conn;
	u_int8_t *buf;
	u_int32_t len, type;
{
	REPMGR_IOVECS iovecs;
	__repmgr_msg_hdr_args msg_hdr;
	u_int8_t hdr_buf[__REPMGR_MSG_HDR_SIZE];
	size_t unused;

	msg_hdr.type = REPMGR_OWN_MSG;
	REPMGR_OWN_BUF_SIZE(msg_hdr) = len;
	REPMGR_OWN_MSG_TYPE(msg_hdr) = type;
	__repmgr_msg_hdr_marshal(env, &msg_hdr, hdr_buf);

	__repmgr_iovec_init(&iovecs);
	__repmgr_add_buffer(&iovecs, hdr_buf, __REPMGR_MSG_HDR_SIZE);
	if (len > 0)
		__repmgr_add_buffer(&iovecs, buf, len);

	return (__repmgr_write_iovecs(env, conn, &iovecs, &unused));
}

/*
 * Reads a whole message, when we expect to get a REPMGR_OWN_MSG.
 */
/*
 * PUBLIC: int __repmgr_read_own_msg __P((ENV *, REPMGR_CONNECTION *,
 * PUBLIC:     u_int32_t *, u_int8_t **, size_t *));
 */
int
__repmgr_read_own_msg(env, conn, typep, bufp, lenp)
	ENV *env;
	REPMGR_CONNECTION *conn;
	u_int32_t *typep;
	u_int8_t **bufp;
	size_t *lenp;
{
	__repmgr_msg_hdr_args msg_hdr;
	u_int8_t *buf;
	u_int32_t type;
	size_t size;
	int ret;

	__repmgr_reset_for_reading(conn);
	if ((ret = __repmgr_read_conn(conn)) != 0)
		goto err;
	ret = __repmgr_msg_hdr_unmarshal(env, &msg_hdr,
	    conn->msg_hdr_buf, __REPMGR_MSG_HDR_SIZE, NULL);
	DB_ASSERT(env, ret == 0);

	if ((conn->msg_type = msg_hdr.type) != REPMGR_OWN_MSG) {
		ret = DB_REP_UNAVAIL; /* Protocol violation. */
		goto err;
	}
	type = REPMGR_OWN_MSG_TYPE(msg_hdr);
	if ((size = (size_t)REPMGR_OWN_BUF_SIZE(msg_hdr)) > 0) {
		conn->reading_phase = DATA_PHASE;
		__repmgr_iovec_init(&conn->iovecs);

		if ((ret = __os_malloc(env, size, &buf)) != 0)
			goto err;
		conn->input.rep_message = NULL;

		__repmgr_add_buffer(&conn->iovecs, buf, size);
		if ((ret = __repmgr_read_conn(conn)) != 0) {
			__os_free(env, buf);
			goto err;
		}
		*bufp = buf;
	}

	*typep = type;
	*lenp = size;

err:
	return (ret);
}

/*
 * Returns TRUE if we are connected to the other site in a preferred
 * master replication group, FALSE otherwise.
 *
 * PUBLIC: int __repmgr_prefmas_connected __P((ENV *));
 */
int
__repmgr_prefmas_connected(env)
	ENV *env;
{
	DB_REP *db_rep;
	REPMGR_CONNECTION *conn;
	REPMGR_SITE *other_site;

	db_rep = env->rep_handle;

	/*
	 * Preferred master mode only has 2 sites, so the other site is
	 * always EID 1.
	 */
	if (!IS_PREFMAS_MODE(env) || !IS_KNOWN_REMOTE_SITE(1))
		  return (FALSE);

	other_site = SITE_FROM_EID(1);
	if (other_site->state == SITE_CONNECTED)
		return (TRUE);

	if ((conn = other_site->ref.conn.in) != NULL &&
	    IS_READY_STATE(conn->state))
		return (TRUE);
	if ((conn = other_site->ref.conn.out) != NULL &&
	    IS_READY_STATE(conn->state))
		return (TRUE);

	return (FALSE);
}

/*
 * Used by a preferred master site to restart the remote temporary master
 * site as a client.  This is used to help guarantee that the preferred master
 * site's transactions are never rolled back.
 *
 * PUBLIC: int __repmgr_restart_site_as_client __P((ENV *, int));
 */
int
__repmgr_restart_site_as_client(env, eid)
	ENV *env;
	int eid;
{
	DB_REP *db_rep;
	REPMGR_CONNECTION *conn;
	repmgr_netaddr_t addr;
	u_int32_t type;
	size_t len;
	u_int8_t any_value, *response_buf;
	int ret, t_ret;

	COMPQUIET(any_value, 0);
	db_rep = env->rep_handle;
	conn = NULL;

	if (!IS_PREFMAS_MODE(env))
		return (0);

	LOCK_MUTEX(db_rep->mutex);
	addr = SITE_FROM_EID(eid)->net_addr;
	UNLOCK_MUTEX(db_rep->mutex);
	if ((ret = __repmgr_make_request_conn(env, &addr, &conn)) != 0)
		return (ret);

	/*
	 * No payload needed, but must send at least a dummy byte for the
	 * other side to recognize that a message has arrived.
	 */
	if ((ret = __repmgr_send_sync_msg(env, conn,
	    REPMGR_RESTART_CLIENT, VOID_STAR_CAST &any_value, 1)) != 0)
		goto err;

	if ((ret = __repmgr_read_own_msg(env,
	    conn, &type, &response_buf, &len)) != 0)
		goto err;
	if (type != REPMGR_PREFMAS_SUCCESS) {
		RPRINT(env, (env, DB_VERB_REPMGR_MISC,
		    "restart_site_as_client got unexpected message type %d",
		    type));
		ret = DB_REP_UNAVAIL; /* Invalid response: protocol violation */
	}
err:
	if (conn != NULL) {
		if ((t_ret = __repmgr_close_connection(env,
		    conn)) != 0 && ret != 0)
			ret = t_ret;
		if ((t_ret = __repmgr_destroy_conn(env,
		    conn)) != 0 && ret != 0)
			ret = t_ret;
	}
	return (ret);
}

/*
 * Used by a preferred master site to make the remote temporary master
 * site a readonly master.  This is used to help preserve all temporary
 * master transactions.
 *
 * PUBLIC: int __repmgr_make_site_readonly_master __P((ENV *, int,
 * PUBLIC:     u_int32_t *, DB_LSN *));
 */
int
__repmgr_make_site_readonly_master(env, eid, gen, sync_lsnp)
	ENV *env;
	int eid;
	u_int32_t *gen;
	DB_LSN *sync_lsnp;
{
	DB_REP *db_rep;
	REPMGR_CONNECTION *conn;
	repmgr_netaddr_t addr;
	__repmgr_permlsn_args permlsn;
	u_int32_t type;
	size_t len;
	u_int8_t any_value, *response_buf;
	int ret, t_ret;

	COMPQUIET(any_value, 0);
	db_rep = env->rep_handle;
	conn = NULL;
	response_buf = NULL;
	*gen = 0;
	ZERO_LSN(*sync_lsnp);

	if (!IS_PREFMAS_MODE(env))
		return (0);

	LOCK_MUTEX(db_rep->mutex);
	addr = SITE_FROM_EID(eid)->net_addr;
	UNLOCK_MUTEX(db_rep->mutex);
	if ((ret = __repmgr_make_request_conn(env, &addr, &conn)) != 0)
		return (ret);

	/*
	 * No payload needed, but must send at least a dummy byte for the
	 * other side to recognize that a message has arrived.
	 */
	if ((ret = __repmgr_send_sync_msg(env, conn,
	    REPMGR_READONLY_MASTER, VOID_STAR_CAST &any_value, 1)) != 0)
		goto err;

	if ((ret = __repmgr_read_own_msg(env,
	    conn, &type, &response_buf, &len)) != 0)
		goto err;

	if (type == REPMGR_READONLY_RESPONSE) {
		if ((ret = __repmgr_permlsn_unmarshal(env,
		    &permlsn, response_buf, len, NULL)) != 0)
			goto err;
		*gen = permlsn.generation;
		*sync_lsnp = permlsn.lsn;
	} else {
		RPRINT(env, (env, DB_VERB_REPMGR_MISC,
		    "make_site_readonly_master got unexpected message type %d",
		    type));
		ret = DB_REP_UNAVAIL; /* Invalid response: protocol violation */
	}

err:
	if (conn != NULL) {
		if ((t_ret = __repmgr_close_connection(env,
		    conn)) != 0 && ret != 0)
			ret = t_ret;
		if ((t_ret = __repmgr_destroy_conn(env,
		    conn)) != 0 && ret != 0)
			ret = t_ret;
	}
	if (response_buf != NULL)
		__os_free(env, response_buf);
	return (ret);
}

/*
 * Used by a preferred master site to perform the LSN history comparisons to
 * determine whether there is are continuous or conflicting sets of
 * transactions between this site and the remote temporary master.
 *
 * PUBLIC: int __repmgr_lsnhist_match __P((ENV *,
 * PUBLIC:     DB_THREAD_INFO *, int, int *));
 */
int
__repmgr_lsnhist_match(env, ip, eid, match)
	ENV *env;
	DB_THREAD_INFO *ip;
	int eid;
	int *match;
{
	DB_REP *db_rep;
	REP *rep;
	__rep_lsn_hist_data_args my_lsnhist;
	__repmgr_lsnhist_match_args remote_lsnhist;
	u_int32_t my_gen;
	int found_commit, ret;

	db_rep = env->rep_handle;
	rep = db_rep->region;
	*match = FALSE;
	my_gen = rep->gen;
	found_commit = FALSE;

	if (!IS_PREFMAS_MODE(env))
		  return (0);

	/* Get local LSN history information for comparison. */
	if ((ret = __rep_get_lsnhist_data(env, ip, my_gen, &my_lsnhist)) != 0)
		return (ret);

	/* Get remote LSN history information for comparison. */
	ret = __repmgr_remote_lsnhist(env, eid, my_gen, &remote_lsnhist);

	/*
	 * If the current gen doesn't exist at the remote site, the match
	 * fails.
	 *
	 * If the remote LSN or timestamp at the current gen doesn't match
	 * ours, we probably had a whack-a-mole situation where each site
	 * as up and down in isolation one or more times and the match fails.
	 *
	 * If the remote LSN for the next generation is lower than this
	 * site's startup LSN and there are any commit operations between
	 * these LSNs, there are conflicting sets of transactions and the
	 * match fails.
	 */
	RPRINT(env, (env, DB_VERB_REPMGR_MISC,
	    "lsnhist_match my_lsn [%lu][%lu] remote_lsn [%lu][%lu]",
	    (u_long)my_lsnhist.lsn.file, (u_long)my_lsnhist.lsn.offset,
	    (u_long)remote_lsnhist.lsn.file,
	    (u_long)remote_lsnhist.lsn.offset));
	RPRINT(env, (env, DB_VERB_REPMGR_MISC,
	    "lsnhist_match my_time %lu:%lu remote_time %lu:%lu",
	    (u_long)my_lsnhist.hist_sec, (u_long)my_lsnhist.hist_nsec,
	    (u_long)remote_lsnhist.hist_sec, (u_long)remote_lsnhist.hist_nsec));
	RPRINT(env, (env, DB_VERB_REPMGR_MISC,
	    "lsnhist_match pminit_lsn [%lu][%lu] next_gen_lsn [%lu][%lu]",
	    (u_long)db_rep->prefmas_init_lsn.file,
	    (u_long)db_rep->prefmas_init_lsn.offset,
	    (u_long)remote_lsnhist.next_gen_lsn.file,
	    (u_long)remote_lsnhist.next_gen_lsn.offset));
	if (ret != DB_REP_UNAVAIL &&
	    LOG_COMPARE(&my_lsnhist.lsn, &remote_lsnhist.lsn) == 0 &&
	    my_lsnhist.hist_sec == remote_lsnhist.hist_sec &&
	    my_lsnhist.hist_nsec == remote_lsnhist.hist_nsec) {
		/*
		 * If the remote site doesn't yet have the next gen or if
		 * our startup LSN is <= than the remote next gen LSN, we
		 * have a match.
		 *
		 * Otherwise, our startup LSN is higher than the remote
		 * next gen LSN.  If we have any commit operations between
		 * these two LSNs, we have preferred master operations we
		 * must preserve and there is not a match.  But if we just
		 * have uncommitted operations between these LSNs it doesn't
		 * matter if they are rolled back, so we call it a match and
		 * try to retain temporary master transactions if possible.
		 */
		if (IS_ZERO_LSN(remote_lsnhist.next_gen_lsn) ||
		    LOG_COMPARE(&db_rep->prefmas_init_lsn,
		    &remote_lsnhist.next_gen_lsn) <= 0)
			*match = TRUE;
		else if ((ret = __repmgr_find_commit(env,
		    &remote_lsnhist.next_gen_lsn,
		    &db_rep->prefmas_init_lsn, &found_commit)) == 0 &&
		    !found_commit) {
			RPRINT(env, (env, DB_VERB_REPMGR_MISC,
			    "lsnhist_match !found_commit set match TRUE"));
			*match = TRUE;
		}
	}

	/* Don't return an error if current gen didn't exist at remote site. */
	if (ret == DB_REP_UNAVAIL)
		ret = 0;
	RPRINT(env, (env, DB_VERB_REPMGR_MISC,
	    "lsnhist_match match %d returning %d", *match, ret));
	return (ret);
}

/*
 * Checks a range of log records from low_lsn to high_lsn for any
 * commit operations.  Sets found_commit to TRUE if a commit is
 * found.
 */
static int
__repmgr_find_commit(env, low_lsn, high_lsn, found_commit)
	ENV *env;
	DB_LSN *low_lsn;
	DB_LSN *high_lsn;
	int *found_commit;
{
	DB_LOGC *logc;
	DB_LSN lsn;
	DBT rec;
	__txn_regop_args *txn_args;
	u_int32_t rectype;
	int ret, t_ret;

	*found_commit = FALSE;
	ret = 0;

	lsn = *low_lsn;
	if ((ret = __log_cursor(env, &logc)) != 0)
		return (ret);
	memset(&rec, 0, sizeof(rec));
	if (__logc_get(logc, &lsn, &rec, DB_SET) == 0) {
		do {
			LOGCOPY_32(env, &rectype, rec.data);
			if (rectype == DB___txn_regop) {
				if ((ret = __txn_regop_read(
				    env, rec.data, &txn_args)) != 0)
					goto close_cursor;
				if (txn_args->opcode == TXN_COMMIT) {
					*found_commit = TRUE;
					__os_free(env, txn_args);
					break;
				}
				__os_free(env, txn_args);
			}
		} while ((ret = __logc_get(logc, &lsn, &rec, DB_NEXT)) == 0 &&
		    LOG_COMPARE(&lsn, high_lsn) <= 0);
	}
close_cursor:
	if ((t_ret = __logc_close(logc)) != 0 && ret == 0)
		ret = t_ret;
	return (ret);
}

/*
 * Used by a preferred master site to get remote LSN history information
 * from the other site in the replication group.
 */
static int
__repmgr_remote_lsnhist(env, eid, gen, lsnhist_match)
	ENV *env;
	int eid;
	u_int32_t gen;
	__repmgr_lsnhist_match_args *lsnhist_match;
{
	DB_REP *db_rep;
	REPMGR_CONNECTION *conn;
	repmgr_netaddr_t addr;
	__rep_lsn_hist_key_args lsnhist_key;
	u_int8_t lsnhist_key_buf[__REP_LSN_HIST_KEY_SIZE];
	u_int32_t type;
	size_t len;
	u_int8_t *response_buf;
	int ret, t_ret;

	db_rep = env->rep_handle;
	conn = NULL;
	response_buf = NULL;

	if (!IS_KNOWN_REMOTE_SITE(eid))
		  return (0);

	LOCK_MUTEX(db_rep->mutex);
	addr = SITE_FROM_EID(eid)->net_addr;
	UNLOCK_MUTEX(db_rep->mutex);
	if ((ret = __repmgr_make_request_conn(env, &addr, &conn)) != 0)
		return (ret);

	/* Marshal generation for which to request remote lsnhist data. */
	lsnhist_key.version = REP_LSN_HISTORY_FMT_VERSION;
	lsnhist_key.gen = gen;
	__rep_lsn_hist_key_marshal(env, &lsnhist_key, lsnhist_key_buf);
	if ((ret = __repmgr_send_sync_msg(env, conn, REPMGR_LSNHIST_REQUEST,
	    lsnhist_key_buf, sizeof(lsnhist_key_buf))) != 0)
		goto err;

	if ((ret = __repmgr_read_own_msg(env,
	    conn, &type, &response_buf, &len)) != 0)
		goto err;

	/* Unmarshal remote lsnhist time and LSNs for comparison. */
	if (type == REPMGR_LSNHIST_RESPONSE) {
		if ((ret = __repmgr_lsnhist_match_unmarshal(env, lsnhist_match,
		    response_buf, __REPMGR_LSNHIST_MATCH_SIZE, NULL)) != 0)
			goto err;
	} else {
		/*
		 * If the other site sent back REPMGR_PREFMAS_FAILURE, it means
		 * no lsnhist record for the requested gen was found on other
		 * site.
		 */
		if (type != REPMGR_PREFMAS_FAILURE)
			RPRINT(env, (env, DB_VERB_REPMGR_MISC,
			    "remote_lsnhist got unexpected message type %d",
			    type));
		ret = DB_REP_UNAVAIL;
	}

err:
	if (conn != NULL) {
		if ((t_ret = __repmgr_close_connection(env,
		    conn)) != 0 && ret != 0)
			ret = t_ret;
		if ((t_ret = __repmgr_destroy_conn(env,
		    conn)) != 0 && ret != 0)
			ret = t_ret;
	}
	if (response_buf != NULL)
		__os_free(env, response_buf);
	return (ret);
}

/*
 * Returns the number of tries and the amount of time to yield the
 * processor for preferred master waits.  The total wait is the larger
 * of 2 seconds or 3 * ack_timeout.
 *
 * PUBLIC: int __repmgr_prefmas_get_wait __P((ENV *, u_int32_t *, u_long *));
 */
int
__repmgr_prefmas_get_wait(env, tries, yield_usecs)
	ENV *env;
	u_int32_t *tries;
	u_long *yield_usecs;
{
	DB_REP *db_rep;
	REP *rep;
	db_timeout_t max_wait;

	db_rep = env->rep_handle;
	rep = db_rep->region;

	*yield_usecs = 250000;
	max_wait = DB_REPMGR_DEFAULT_ACK_TIMEOUT * 2;
	if ((rep->ack_timeout * 3) > max_wait)
		max_wait = rep->ack_timeout * 3;
	*tries = max_wait / (u_int32_t)*yield_usecs;
	return (0);
}

/*
 * Produce a membership list from the known info currently in memory.
 *
 * PUBLIC: int __repmgr_marshal_member_list __P((ENV *, u_int32_t,
 * PUBLIC:     u_int8_t **, size_t *));
 *
 * Caller must hold mutex.
 */
int
__repmgr_marshal_member_list(env, msg_version, bufp, lenp)
	ENV *env;
	u_int32_t msg_version;
	u_int8_t **bufp;
	size_t *lenp;
{
	DB_REP *db_rep;
	REP *rep;
	REPMGR_SITE *site;
	__repmgr_membr_vers_args membr_vers;
	__repmgr_site_info_args site_info;
	__repmgr_v4site_info_args v4site_info;
	u_int8_t *buf, *p;
	size_t bufsize, len;
	u_int i;
	int ret;

	db_rep = env->rep_handle;
	rep = db_rep->region;

	/* Compute a (generous) upper bound on needed buffer size. */
	bufsize = __REPMGR_MEMBR_VERS_SIZE +
	    db_rep->site_cnt * (__REPMGR_SITE_INFO_SIZE + MAXHOSTNAMELEN + 1);
	if ((ret = __os_malloc(env, bufsize, &buf)) != 0)
		return (ret);
	p = buf;

	membr_vers.version = db_rep->membership_version;
	membr_vers.gen = rep->gen;
	__repmgr_membr_vers_marshal(env, &membr_vers, p);
	p += __REPMGR_MEMBR_VERS_SIZE;

	for (i = 0; i < db_rep->site_cnt; i++) {
		site = SITE_FROM_EID(i);
		if (site->membership == 0)
			continue;

		if (msg_version < 5) {
			v4site_info.host.data = site->net_addr.host;
			v4site_info.host.size =
				(u_int32_t)strlen(site->net_addr.host) + 1;
			v4site_info.port = site->net_addr.port;
			v4site_info.flags = site->membership;
			ret = __repmgr_v4site_info_marshal(env,
			    &v4site_info, p, (size_t)(&buf[bufsize]-p), &len);
		} else {
			site_info.host.data = site->net_addr.host;
			site_info.host.size =
				(u_int32_t)strlen(site->net_addr.host) + 1;
			site_info.port = site->net_addr.port;
			site_info.status = site->membership;
			site_info.flags = site->gmdb_flags;
			ret = __repmgr_site_info_marshal(env,
			    &site_info, p, (size_t)(&buf[bufsize]-p), &len);
		}
		DB_ASSERT(env, ret == 0);
		p += len;
	}
	len = (size_t)(p - buf);

	*bufp = buf;
	*lenp = len;
	DB_ASSERT(env, ret == 0);
	return (0);
}

/*
 * Produce a membership list by reading the database.
 */
static int
read_gmdb(env, ip, bufp, lenp)
	ENV *env;
	DB_THREAD_INFO *ip;
	u_int8_t **bufp;
	size_t *lenp;
{
	DB_TXN *txn;
	DB *dbp;
	DBC *dbc;
	DBT key_dbt, data_dbt;
	__repmgr_membership_key_args key;
	__repmgr_membership_data_args member_data;
	__repmgr_member_metadata_args metadata;
	__repmgr_membr_vers_args membr_vers;
	__repmgr_site_info_args site_info;
	u_int8_t data_buf[__REPMGR_MEMBERSHIP_DATA_SIZE];
	u_int8_t key_buf[MAX_MSG_BUF];
	u_int8_t metadata_buf[__REPMGR_MEMBER_METADATA_SIZE];
	char *host;
	size_t bufsize, len;
	u_int8_t *buf, *p;
	u_int32_t gen;
	int ret, t_ret;

	txn = NULL;
	dbp = NULL;
	dbc = NULL;
	buf = NULL;
	COMPQUIET(len, 0);

	if ((ret = __rep_get_datagen(env, &gen)) != 0)
		return (ret);
	if ((ret = __txn_begin(env, ip, NULL, &txn, DB_IGNORE_LEASE)) != 0)
		goto err;
	if ((ret = __rep_open_sysdb(env, ip, txn, REPMEMBERSHIP, 0, &dbp)) != 0)
		goto err;
	if ((ret = __db_cursor(dbp, ip, txn, &dbc, 0)) != 0)
		goto err;

	memset(&key_dbt, 0, sizeof(key_dbt));
	key_dbt.data = key_buf;
	key_dbt.ulen = sizeof(key_buf);
	F_SET(&key_dbt, DB_DBT_USERMEM);
	memset(&data_dbt, 0, sizeof(data_dbt));
	data_dbt.data = metadata_buf;
	data_dbt.ulen = sizeof(metadata_buf);
	F_SET(&data_dbt, DB_DBT_USERMEM);

	/* Get metadata record, make sure key looks right. */
	if ((ret = __dbc_get(dbc, &key_dbt, &data_dbt, DB_NEXT)) != 0)
		goto err;
	ret = __repmgr_membership_key_unmarshal(env,
	    &key, key_buf, key_dbt.size, NULL);
	DB_ASSERT(env, ret == 0);
	DB_ASSERT(env, key.host.size == 0);
	DB_ASSERT(env, key.port == 0);
	ret = __repmgr_member_metadata_unmarshal(env,
	    &metadata, metadata_buf, data_dbt.size, NULL);
	DB_ASSERT(env, ret == 0);
	DB_ASSERT(env, metadata.format >= REPMGR_GMDB_FMT_MIN_VERSION &&
	    metadata.format <= REPMGR_GMDB_FMT_VERSION);
	DB_ASSERT(env, metadata.version > 0);
	/* Automatic conversion of old format gmdb if needed. */
	if (metadata.format < REPMGR_GMDB_FMT_VERSION &&
	    (ret = convert_gmdb(env, ip, dbp, txn)) != 0)
		goto err;

	bufsize = 1000;		/* Initial guess. */
	if ((ret = __os_malloc(env, bufsize, &buf)) != 0)
		goto err;
	membr_vers.version = metadata.version;
	membr_vers.gen = gen;
	__repmgr_membr_vers_marshal(env, &membr_vers, buf);
	p = &buf[__REPMGR_MEMBR_VERS_SIZE];

	data_dbt.data = data_buf;
	data_dbt.ulen = sizeof(data_buf);
	while ((ret = __dbc_get(dbc, &key_dbt, &data_dbt, DB_NEXT)) == 0) {
		ret = __repmgr_membership_key_unmarshal(env,
		    &key, key_buf, key_dbt.size, NULL);
		DB_ASSERT(env, ret == 0);
		DB_ASSERT(env, key.host.size <= MAXHOSTNAMELEN + 1 &&
		    key.host.size > 1);
		host = (char*)key.host.data;
		DB_ASSERT(env, host[key.host.size-1] == '\0');
		DB_ASSERT(env, key.port > 0);

		ret = __repmgr_membership_data_unmarshal(env,
		    &member_data, data_buf, data_dbt.size, NULL);
		DB_ASSERT(env, ret == 0);
		DB_ASSERT(env, member_data.status != 0);

		site_info.host = key.host;
		site_info.port = key.port;
		site_info.status = member_data.status;
		site_info.flags = member_data.flags;
		if ((ret = __repmgr_site_info_marshal(env, &site_info,
		    p, (size_t)(&buf[bufsize]-p), &len)) == ENOMEM) {
			bufsize *= 2;
			len = (size_t)(p - buf);
			if ((ret = __os_realloc(env, bufsize, &buf)) != 0)
				goto err;
			p = &buf[len];
			ret = __repmgr_site_info_marshal(env,
			    &site_info, p, (size_t)(&buf[bufsize]-p), &len);
			DB_ASSERT(env, ret == 0);
		}
		p += len;
	}
	len = (size_t)(p - buf);
	if (ret == DB_NOTFOUND)
		ret = 0;

err:
	if (dbc != NULL && (t_ret = __dbc_close(dbc)) != 0 && ret == 0)
		ret = t_ret;
	if (dbp != NULL &&
	    (t_ret = __db_close(dbp, txn, DB_NOSYNC)) != 0 && ret == 0)
		ret = t_ret;
	if (txn != NULL &&
	    (t_ret = __db_txn_auto_resolve(env, txn, 0, ret)) != 0 && ret == 0)
		ret = t_ret;
	if (ret == 0) {
		*bufp = buf;
		*lenp = len;
	} else if (buf != NULL)
		__os_free(env, buf);
	return (ret);
}

/*
 * Convert an older-format group membership database into the current format.
 */
static int
convert_gmdb(env, ip, dbp, txn)
	ENV *env;
	DB_THREAD_INFO *ip;
	DB *dbp;
	DB_TXN *txn;
{
	DBC *dbc;
	DBT key_dbt, data_dbt, v4data_dbt;
	__repmgr_membership_key_args key;
	__repmgr_membership_data_args member_data;
	__repmgr_v4membership_data_args v4member_data;
	__repmgr_member_metadata_args metadata;
	u_int8_t data_buf[__REPMGR_MEMBERSHIP_DATA_SIZE];
	u_int8_t key_buf[MAX_MSG_BUF];
	u_int8_t metadata_buf[__REPMGR_MEMBER_METADATA_SIZE];
	u_int8_t v4data_buf[__REPMGR_V4MEMBERSHIP_DATA_SIZE];
	int ret, t_ret;

	dbc = NULL;

	if ((ret = __db_cursor(dbp, ip, txn, &dbc, 0)) != 0)
		goto err;

	memset(&key_dbt, 0, sizeof(key_dbt));
	key_dbt.data = key_buf;
	key_dbt.ulen = sizeof(key_buf);
	F_SET(&key_dbt, DB_DBT_USERMEM);
	memset(&data_dbt, 0, sizeof(data_dbt));
	data_dbt.data = metadata_buf;
	data_dbt.ulen = sizeof(metadata_buf);
	F_SET(&data_dbt, DB_DBT_USERMEM);
	memset(&v4data_dbt, 0, sizeof(v4data_dbt));
	v4data_dbt.data = v4data_buf;
	v4data_dbt.ulen = sizeof(v4data_buf);
	F_SET(&v4data_dbt, DB_DBT_USERMEM);

	/*
	 * The first gmdb record is a special metadata record that contains
	 * an empty key and gmdb metadata (format and version) and has already
	 * been validated by the caller.  We need to update its format value
	 * for this conversion but leave the version alone.
	 */
	if ((ret = __dbc_get(dbc, &key_dbt, &data_dbt, DB_NEXT)) != 0)
		goto err;
	ret = __repmgr_membership_key_unmarshal(env,
	    &key, key_buf, key_dbt.size, NULL);
	DB_ASSERT(env, ret == 0);
	DB_ASSERT(env, key.host.size == 0);
	DB_ASSERT(env, key.port == 0);
	ret = __repmgr_member_metadata_unmarshal(env,
	    &metadata, metadata_buf, data_dbt.size, NULL);
	DB_ASSERT(env, ret == 0);
	DB_ASSERT(env, metadata.version > 0);
	metadata.format = REPMGR_GMDB_FMT_VERSION;
	__repmgr_member_metadata_marshal(env, &metadata, metadata_buf);
	DB_INIT_DBT(data_dbt, metadata_buf, __REPMGR_MEMBER_METADATA_SIZE);
	if ((ret = __dbc_put(dbc, &key_dbt, &data_dbt, DB_CURRENT)) != 0)
		goto err;

	/*
	 * The rest of the gmdb records contain a key (host and port) and
	 * membership data (status and now flags).  But the old format was
	 * using flags for the status value, so we need to transfer the
	 * old flags value to status and provide an empty flags value for
	 * this conversion.
	 */
	data_dbt.data = data_buf;
	data_dbt.ulen = sizeof(data_buf);
	while ((ret = __dbc_get(dbc, &key_dbt, &v4data_dbt, DB_NEXT)) == 0) {
		/* Get membership data in old format. */
		ret = __repmgr_v4membership_data_unmarshal(env,
		    &v4member_data, v4data_buf, v4data_dbt.size, NULL);
		DB_ASSERT(env, ret == 0);
		DB_ASSERT(env, v4member_data.flags != 0);

		/* Convert membership data into current format and update. */
		member_data.status = v4member_data.flags;
		member_data.flags = 0;
		__repmgr_membership_data_marshal(env, &member_data, data_buf);
		DB_INIT_DBT(data_dbt, data_buf, __REPMGR_MEMBERSHIP_DATA_SIZE);
		if ((ret = __dbc_put(dbc,
		    &key_dbt, &data_dbt, DB_CURRENT)) != 0)
			goto err;
	}
	if (ret == DB_NOTFOUND)
		ret = 0;

err:
	if (dbc != NULL && (t_ret = __dbc_close(dbc)) != 0 && ret == 0)
		ret = t_ret;
	return (ret);
}

/*
 * Refresh our sites array from the given membership list.
 *
 * PUBLIC: int __repmgr_refresh_membership __P((ENV *,
 * PUBLIC:     u_int8_t *, size_t, u_int32_t));
 */
int
__repmgr_refresh_membership(env, buf, len, version)
	ENV *env;
	u_int8_t *buf;
	size_t len;
	u_int32_t version;
{
	DB_REP *db_rep;
	REP *rep;
	REPMGR_SITE *site;
	__repmgr_membr_vers_args membr_vers;
	__repmgr_site_info_args site_info;
	__repmgr_v4site_info_args v4site_info;
	char *host;
	u_int8_t *p;
	u_int16_t port;
	u_int32_t i, participants;
	int eid, ret;

	db_rep = env->rep_handle;
	rep = db_rep->region;

	/*
	 * Membership list consists of membr_vers followed by a number of
	 * site_info structs.
	 */
	ret = __repmgr_membr_vers_unmarshal(env, &membr_vers, buf, len, &p);
	DB_ASSERT(env, ret == 0);

	if (db_rep->repmgr_status == stopped)
		return (0);
	/* Ignore obsolete versions. */
	if (__repmgr_gmdb_version_cmp(env,
	    membr_vers.gen, membr_vers.version) <= 0)
		return (0);

	LOCK_MUTEX(db_rep->mutex);

	db_rep->membership_version = membr_vers.version;
	db_rep->member_version_gen = membr_vers.gen;

	for (i = 0; i < db_rep->site_cnt; i++)
		F_CLR(SITE_FROM_EID(i), SITE_TOUCHED);

	for (participants = 0; p < &buf[len]; ) {
		if (version < 5) {
			ret = __repmgr_v4site_info_unmarshal(env,
			    &v4site_info, p, (size_t)(&buf[len] - p), &p);
			site_info.host = v4site_info.host;
			site_info.port = v4site_info.port;
			site_info.status = v4site_info.flags;
			site_info.flags = 0;
		} else
			ret = __repmgr_site_info_unmarshal(env,
			    &site_info, p, (size_t)(&buf[len] - p), &p);
		DB_ASSERT(env, ret == 0);

		host = site_info.host.data;
		DB_ASSERT(env,
		    (u_int8_t*)site_info.host.data + site_info.host.size <= p);
		host[site_info.host.size-1] = '\0';
		port = site_info.port;
		if (!FLD_ISSET(site_info.flags, SITE_VIEW))
			participants++;

		if ((ret = __repmgr_set_membership(env,
		    host, port, site_info.status, site_info.flags)) != 0)
			goto err;

		if ((ret = __repmgr_find_site(env, host, port, &eid)) != 0)
			goto err;
		DB_ASSERT(env, IS_VALID_EID(eid));
		F_SET(SITE_FROM_EID(eid), SITE_TOUCHED);
	}
	ret = __rep_set_nsites_int(env, participants);
	DB_ASSERT(env, ret == 0);
	if (FLD_ISSET(rep->config,
	    REP_C_PREFMAS_MASTER | REP_C_PREFMAS_CLIENT) &&
	    rep->config_nsites > 2)
		__db_errx(env, DB_STR("3703",
	    "More than two sites in preferred master replication group"));

	/* Scan "touched" flags so as to notice sites that have been removed. */
	for (i = 0; i < db_rep->site_cnt; i++) {
		site = SITE_FROM_EID(i);
		if (F_ISSET(site, SITE_TOUCHED))
			continue;
		host = site->net_addr.host;
		port = site->net_addr.port;
		if ((ret = __repmgr_set_membership(env, host, port,
		    0, site->gmdb_flags)) != 0)
			goto err;
	}

err:
	UNLOCK_MUTEX(db_rep->mutex);
	return (ret);
}

/*
 * PUBLIC: int __repmgr_reload_gmdb __P((ENV *));
 */
int
__repmgr_reload_gmdb(env)
	ENV *env;
{
	DB_THREAD_INFO *ip;
	u_int8_t *buf;
	size_t len;
	int ret;

	ENV_GET_THREAD_INFO(env, ip);
	if ((ret = read_gmdb(env, ip, &buf, &len)) == 0) {
		env->rep_handle->have_gmdb = TRUE;
		ret = __repmgr_refresh_membership(env, buf, len,
			DB_REPMGR_VERSION);
		__os_free(env, buf);
	}
	return (ret);
}

/*
 * Return 1, 0, or -1, as the given gen/version combination is >, =, or < our
 * currently known version.
 *
 * PUBLIC: int __repmgr_gmdb_version_cmp __P((ENV *, u_int32_t, u_int32_t));
 */
int
__repmgr_gmdb_version_cmp(env, gen, version)
	ENV *env;
	u_int32_t gen, version;
{
	DB_REP *db_rep;
	u_int32_t g, v;

	db_rep = env->rep_handle;
	g = db_rep->member_version_gen;
	v = db_rep->membership_version;

	if (gen == g)
		return (version == v ? 0 :
		    (version < v ? -1 : 1));
	return (gen < g ? -1 : 1);
}

/*
 * PUBLIC: int __repmgr_init_save __P((ENV *, DBT *));
 */
int
__repmgr_init_save(env, dbt)
	ENV *env;
	DBT *dbt;
{
	DB_REP *db_rep;
	u_int8_t *buf;
	size_t len;
	int ret;

	db_rep = env->rep_handle;
	LOCK_MUTEX(db_rep->mutex);
	if (db_rep->site_cnt == 0) {
		dbt->data = NULL;
		dbt->size = 0;
		ret = 0;
	} else if ((ret = __repmgr_marshal_member_list(env,
	    DB_REPMGR_VERSION, &buf, &len)) == 0) {
		dbt->data = buf;
		dbt->size = (u_int32_t)len;
	}
	UNLOCK_MUTEX(db_rep->mutex);

	return (ret);
}

/*
 * PUBLIC: int __repmgr_init_restore __P((ENV *, DBT *));
 */
int
__repmgr_init_restore(env, dbt)
	ENV *env;
	DBT *dbt;
{
	DB_REP *db_rep;

	db_rep = env->rep_handle;
	db_rep->restored_list = dbt->data;
	db_rep->restored_list_length = dbt->size;
	return (0);
}

/*
 * Generates an internal request for a deferred operation, to be performed on a
 * separate thread (conveniently, a message-processing thread).
 *
 * PUBLIC: int __repmgr_defer_op __P((ENV *, u_int32_t));
 *
 * Caller should hold mutex.
 */
int
__repmgr_defer_op(env, op)
	ENV *env;
	u_int32_t op;
{
	REPMGR_MESSAGE *msg;
	int ret;

	/*
	 * Overload REPMGR_MESSAGE to convey the type of operation being
	 * requested.  For now "op" is all we need; plenty of room for expansion
	 * if needed in the future.
	 *
	 * Leave msg->v.gmdb_msg.conn NULL to show no conn to be cleaned up.
	 */
	if ((ret = __os_calloc(env, 1, sizeof(*msg), &msg)) != 0)
		return (ret);
	msg->size = sizeof(*msg);
	msg->msg_hdr.type = REPMGR_OWN_MSG;
	REPMGR_OWN_MSG_TYPE(msg->msg_hdr) = op;
	ret = __repmgr_queue_put(env, msg);
	return (ret);
}

/*
 * PUBLIC: void __repmgr_fire_conn_err_event __P((ENV *,
 * PUBLIC:     REPMGR_CONNECTION *, int));
 */
void
__repmgr_fire_conn_err_event(env, conn, err)
	ENV *env;
	REPMGR_CONNECTION *conn;
	int err;
{
	DB_REP *db_rep;
	DB_REPMGR_CONN_ERR info;

	db_rep = env->rep_handle;
	if (conn->type == REP_CONNECTION && IS_VALID_EID(conn->eid)) {
		__repmgr_print_conn_err(env,
		    &SITE_FROM_EID(conn->eid)->net_addr, err);
		info.eid = conn->eid;
		info.error = err;
		DB_EVENT(env, DB_EVENT_REP_CONNECT_BROKEN, &info);
	}
}

/*
 * PUBLIC: void __repmgr_print_conn_err __P((ENV *, repmgr_netaddr_t *, int));
 */
void
__repmgr_print_conn_err(env, netaddr, err)
	ENV *env;
	repmgr_netaddr_t *netaddr;
	int err;
{
	SITE_STRING_BUFFER site_loc_buf;
	char msgbuf[200];	/* Arbitrary size. */

	(void)__repmgr_format_addr_loc(netaddr, site_loc_buf);
	/* TCP/IP sockets API convention: 0 indicates "end-of-file". */
	if (err == 0)
		RPRINT(env, (env, DB_VERB_REPMGR_MISC,
			"EOF on connection to %s", site_loc_buf));
	else
		RPRINT(env, (env, DB_VERB_REPMGR_MISC,
			"`%s' (%d) on connection to %s",
			__os_strerror(err, msgbuf, sizeof(msgbuf)),
			err, site_loc_buf));
}

/*
 * Change role from master to client, but if a GMDB operation is in progress,
 * wait for it to finish first.
 *
 * PUBLIC: int __repmgr_become_client __P((ENV *));
 */
int
__repmgr_become_client(env)
	ENV *env;
{
	DB_REP *db_rep;
	int ret;

	db_rep = env->rep_handle;
	LOCK_MUTEX(db_rep->mutex);
	if ((ret = __repmgr_await_gmdbop(env)) == 0)
		db_rep->client_intent = TRUE;
	UNLOCK_MUTEX(db_rep->mutex);
	return (ret == 0 ? __repmgr_repstart(env, DB_REP_CLIENT, 0) : ret);
}

/*
 * Looks up a site from our local (in-process) list, or returns NULL if not
 * found.
 *
 * PUBLIC: REPMGR_SITE *__repmgr_lookup_site __P((ENV *, const char *, u_int));
 */
REPMGR_SITE *
__repmgr_lookup_site(env, host, port)
	ENV *env;
	const char *host;
	u_int port;
{
	DB_REP *db_rep;
	REPMGR_SITE *site;
	u_int i;

	db_rep = env->rep_handle;
	for (i = 0; i < db_rep->site_cnt; i++) {
		site = &db_rep->sites[i];

		if (strcmp(site->net_addr.host, host) == 0 &&
		    site->net_addr.port == port)
			return (site);
	}

	return (NULL);
}

/*
 * Look up a site, or add it if it doesn't already exist.
 *
 * Caller must hold db_rep mutex and be within ENV_ENTER context, unless this is
 * a pre-open call.
 *
 * PUBLIC: int __repmgr_find_site __P((ENV *, const char *, u_int, int *));
 */
int
__repmgr_find_site(env, host, port, eidp)
	ENV *env;
	const char *host;
	u_int port;
	int *eidp;
{
	DB_REP *db_rep;
	REP *rep;
	REPMGR_SITE *site;
	int eid, ret;

	db_rep = env->rep_handle;
	ret = 0;
	if (REP_ON(env)) {
		rep = db_rep->region;
		MUTEX_LOCK(env, rep->mtx_repmgr);
		ret = get_eid(env, host, port, &eid);
		MUTEX_UNLOCK(env, rep->mtx_repmgr);
	} else {
		if ((site = __repmgr_lookup_site(env, host, port)) == NULL &&
		    (ret = __repmgr_new_site(env, &site, host, port)) != 0)
			return (ret);
		eid = EID_FROM_SITE(site);
	}
	if (ret == 0)
		*eidp = eid;
	return (ret);
}

/*
 * Get the EID of the named remote site, even if it means creating a new entry
 * in our table if it doesn't already exist.
 *
 * Caller must hold both db_rep mutex and mtx_repmgr.
 */
static int
get_eid(env, host, port, eidp)
	ENV *env;
	const char *host;
	u_int port;
	int *eidp;
{
	DB_REP *db_rep;
	REP *rep;
	REPMGR_SITE *site;
	int eid, ret;

	db_rep = env->rep_handle;
	rep = db_rep->region;

	if ((ret = __repmgr_copy_in_added_sites(env)) != 0)
		return (ret);
	if ((site = __repmgr_lookup_site(env, host, port)) == NULL) {
		/*
		 * Store both locally and in shared region.
		 */
		if ((ret = __repmgr_new_site(env, &site, host, port)) != 0)
			return (ret);

		eid = EID_FROM_SITE(site);
		DB_ASSERT(env, (u_int)eid == db_rep->site_cnt - 1);
		if ((ret = __repmgr_share_netaddrs(env,
		    rep, (u_int)eid, db_rep->site_cnt)) == 0) {
			/* Show that a change was made. */
			db_rep->siteinfo_seq = ++rep->siteinfo_seq;
		} else {
			/*
			 * Rescind the local slot we just added, so that we at
			 * least keep the two lists in sync.
			 */
			db_rep->site_cnt--;
			__repmgr_cleanup_netaddr(env, &site->net_addr);
		}
	} else
		eid = EID_FROM_SITE(site);
	if (ret == 0)
		*eidp = eid;
	return (ret);
}

/*
 * Sets the named remote site's group membership status to the given value,
 * creating it first if it doesn't already exist.  Adjusts connections
 * accordingly.
 *
 * PUBLIC: int __repmgr_set_membership __P((ENV *,
 * PUBLIC:     const char *, u_int, u_int32_t, u_int32_t));
 *
 * Caller must host db_rep mutex, and be in ENV_ENTER context.
 */
int
__repmgr_set_membership(env, host, port, status, flags)
	ENV *env;
	const char *host;
	u_int port;
	u_int32_t status;
	u_int32_t flags;
{
	DB_REP *db_rep;
	REP *rep;
	REGINFO *infop;
	REPMGR_SITE *site;
	SITEINFO *sites;
	u_int32_t orig;
	int eid, ret;

	db_rep = env->rep_handle;
	rep = db_rep->region;
	infop = env->reginfo;

	COMPQUIET(orig, 0);
	COMPQUIET(site, NULL);
	DB_ASSERT(env, REP_ON(env));

	MUTEX_LOCK(env, rep->mtx_repmgr);
	if ((ret = get_eid(env, host, port, &eid)) == 0) {
		DB_ASSERT(env, IS_VALID_EID(eid));
		site = SITE_FROM_EID(eid);
		orig = site->membership;
		sites = R_ADDR(infop, rep->siteinfo_off);

		RPRINT(env, (env, DB_VERB_REPMGR_MISC,
		    "set membership for %s:%lu %lu (was %lu)",
		    host, (u_long)port, (u_long)status, (u_long)orig));
		if (status != sites[eid].status) {
			/*
			 * Show that a change is occurring.
			 *
			 * The call to get_eid() might have also bumped the
			 * sequence number, and since this is all happening
			 * within a single critical section it would be possible
			 * to avoid "wasting" a sequence number.  But it's
			 * hardly worth the trouble and mental complexity: the
			 * sequence number counts changes that occur within an
			 * env region lifetime, so there should be plenty.
			 * We'll run out of membership DB version numbers long
			 * before this becomes a problem.
			 */
			db_rep->siteinfo_seq = ++rep->siteinfo_seq;
		}

		/* Set both private and shared copies of the info. */
		site->membership = status;
		site->gmdb_flags = flags;
		sites[eid].status = status;
		sites[eid].flags = flags;
	}
	MUTEX_UNLOCK(env, rep->mtx_repmgr);

	/*
	 * If our notion of the site's membership changed, we may need to create
	 * or kill a connection.
	 */
	if (ret == 0 && db_rep->repmgr_status == running &&
	    SELECTOR_RUNNING(db_rep)) {

		if (eid == db_rep->self_eid && status != SITE_PRESENT)
			ret = (status == SITE_ADDING) ?
			    __repmgr_defer_op(env, REPMGR_REJOIN) : DB_DELETED;
		else if (orig != SITE_PRESENT && status == SITE_PRESENT &&
		    site->state == SITE_IDLE) {
			/*
			 * Here we might have just joined a group, or we might
			 * be an existing site and we've just learned of another
			 * site joining the group.  In the former case, we
			 * certainly want to connect right away; in the later
			 * case it might be better to wait, because the new site
			 * probably isn't quite ready to accept our connection.
			 * But deciding which case we're in here would be messy,
			 * so for now we just keep it simple and always try
			 * connecting immediately.  The resulting connection
			 * failure shouldn't hurt anything, because we'll just
			 * naturally try again later.
			 */
			if (eid != db_rep->self_eid) {
				ret = __repmgr_schedule_connection_attempt(env,
				    eid, TRUE);
				DB_EVENT(env, DB_EVENT_REP_SITE_ADDED, &eid);
			}
		} else if (orig != 0 && status == 0)
			DB_EVENT(env, DB_EVENT_REP_SITE_REMOVED, &eid);

		/*
		 * Callers are responsible for adjusting nsites, even though in
		 * a way it would make sense to do it here.  It's awkward to do
		 * it here at start-up/join time, when we load up starting from
		 * an empty array.  Then we would get rep_set_nsites()
		 * repeatedly, and when leases were in use that would thrash the
		 * lease table adjustment.
		 */
	}
	return (ret);
}

/*
 * PUBLIC: int __repmgr_bcast_parm_refresh __P((ENV *));
 */
int
__repmgr_bcast_parm_refresh(env)
	ENV *env;
{
	DB_REP *db_rep;
	REP *rep;
	__repmgr_parm_refresh_args parms;
	u_int8_t buf[__REPMGR_PARM_REFRESH_SIZE];
	int ret;

	DB_ASSERT(env, REP_ON(env));
	db_rep = env->rep_handle;
	rep = db_rep->region;
	LOCK_MUTEX(db_rep->mutex);
	parms.ack_policy = (u_int32_t)rep->perm_policy;
	if (rep->priority == 0)
		parms.flags = 0;
	else
		parms.flags = SITE_ELECTABLE;
	__repmgr_parm_refresh_marshal(env, &parms, buf);
	ret = __repmgr_bcast_own_msg(env,
	    REPMGR_PARM_REFRESH, buf, __REPMGR_PARM_REFRESH_SIZE);
	UNLOCK_MUTEX(db_rep->mutex);
	return (ret);
}

/*
 * PUBLIC: int __repmgr_chg_prio __P((ENV *, u_int32_t, u_int32_t));
 */
int
__repmgr_chg_prio(env, prev, cur)
	ENV *env;
	u_int32_t prev, cur;
{
	if ((prev == 0 && cur != 0) ||
	    (prev != 0 && cur == 0))
		return (__repmgr_bcast_parm_refresh(env));
	return (0);
}

/*
 * PUBLIC: int __repmgr_bcast_own_msg __P((ENV *,
 * PUBLIC:     u_int32_t, u_int8_t *, size_t));
 *
 * Caller must hold mutex.
 */
int
__repmgr_bcast_own_msg(env, type, buf, len)
	ENV *env;
	u_int32_t type;
	u_int8_t *buf;
	size_t len;
{
	DB_REP *db_rep;
	REPMGR_CONNECTION *conn;
	REPMGR_SITE *site;
	int ret;
	u_int i;

	db_rep = env->rep_handle;
	if (!SELECTOR_RUNNING(db_rep))
		return (0);
	FOR_EACH_REMOTE_SITE_INDEX(i) {
		site = SITE_FROM_EID(i);
		if (site->state != SITE_CONNECTED)
			continue;
		if ((conn = site->ref.conn.in) != NULL &&
		    conn->state == CONN_READY &&
		    (ret = __repmgr_send_own_msg(env,
		    conn, type, buf, (u_int32_t)len)) != 0 &&
		    (ret = __repmgr_bust_connection(env, conn)) != 0)
			return (ret);
		if ((conn = site->ref.conn.out) != NULL &&
		    conn->state == CONN_READY &&
		    (ret = __repmgr_send_own_msg(env,
		    conn, type, buf, (u_int32_t)len)) != 0 &&
		    (ret = __repmgr_bust_connection(env, conn)) != 0)
			return (ret);
	}
	return (0);
}

/*
 * PUBLIC: int __repmgr_bcast_member_list __P((ENV *));
 *
 * Broadcast membership list to all other sites in the replication group.
 *
 * Caller must hold mutex.
 */
int
__repmgr_bcast_member_list(env)
	ENV *env;
{
	DB_REP *db_rep;
	REPMGR_CONNECTION *conn;
	REPMGR_SITE *site;
	u_int8_t *buf, *v4buf;
	size_t len, v4len;
	int ret;
	u_int i;

	db_rep = env->rep_handle;
	if (!SELECTOR_RUNNING(db_rep))
		return (0);
	buf = NULL;
	v4buf = NULL;
	LOCK_MUTEX(db_rep->mutex);
	/*
	 * Some of the other sites in the replication group might be at
	 * an older version, so we need to be able to send the membership
	 * list in the current or older format.
	 */
	if ((ret = __repmgr_marshal_member_list(env,
	    DB_REPMGR_VERSION, &buf, &len)) != 0 ||
	    (ret = __repmgr_marshal_member_list(env,
	    4, &v4buf, &v4len)) != 0) {
		UNLOCK_MUTEX(db_rep->mutex);
		goto out;
	}
	UNLOCK_MUTEX(db_rep->mutex);

	RPRINT(env, (env, DB_VERB_REPMGR_MISC,
	    "Broadcast latest membership list"));
	FOR_EACH_REMOTE_SITE_INDEX(i) {
		site = SITE_FROM_EID(i);
		if (site->state != SITE_CONNECTED)
			continue;
		if ((conn = site->ref.conn.in) != NULL &&
		    conn->state == CONN_READY &&
		    (ret = __repmgr_send_own_msg(env, conn, REPMGR_SHARING,
		    (conn->version < 5 ? v4buf : buf),
		    (conn->version < 5 ? (u_int32_t) v4len : (u_int32_t)len)))
		    != 0 &&
		    (ret = __repmgr_bust_connection(env, conn)) != 0)
			goto out;
		if ((conn = site->ref.conn.out) != NULL &&
		    conn->state == CONN_READY &&
		    (ret = __repmgr_send_own_msg(env, conn, REPMGR_SHARING,
		    (conn->version < 5 ? v4buf : buf),
		    (conn->version < 5 ? (u_int32_t)v4len : (u_int32_t)len)))
		    != 0 &&
		    (ret = __repmgr_bust_connection(env, conn)) != 0)
			goto out;
	}
out:
	if (buf != NULL)
		__os_free(env, buf);
	if (v4buf != NULL)
		__os_free(env, v4buf);
	return (ret);
}

/*
 * PUBLIC: int __repmgr_forward_single_write __P((u_int32_t,
 * PUBLIC:     DB *, DBT *, DBT *, u_int32_t));
 *
 * Forwards a replication manager client write operation to the master
 * for processing.  It uses repmgr channels to implement the internal
 * messages needed to do this.  This routine contains the client-side
 * processing.
 *
 * The only supported operations are a single put or a single del
 * without a transaction.  Each forwarded operation uses its own
 * implicit transaction on the master.  The master must have an open
 * database handle for the database on which the operation is being
 * performed.
 *
 * This routine only returns errors expected by a put or del operation
 * with one exception: it can also return DB_TIMEOUT to reflect
 * communications issues between the client and master.
 */
int
__repmgr_forward_single_write(optype, dbp, key, data, opflags)
	u_int32_t optype;
	DB *dbp;
	DBT *key;
	DBT *data;
	u_int32_t opflags;
{
	DB_CHANNEL *wfchannel;
	DB_ENV *dbenv;
	DB_REP *db_rep;
	ENV *env;
	REP *rep;
	DBT response;
	DBT msgdbts[REPMGR_WF_MAX_DBTS];
	char fidstr[REPMGR_WF_FILEID_STRLEN];
	wf_uint32_pair opmeta, twoflags, wfidvers;
	u_int32_t nsegs;
	int i, ret, ret2;

	env = dbp->env;
	dbenv = env->dbenv;
	db_rep = env->rep_handle;
	rep = db_rep->region;
	ret = 0;
	ret2 = 0;
	nsegs = 0;

	/*
	 * If we are a subordinate client process that didn't call
	 * repmgr_start(), we might not have set the write forwarding
	 * repmgr channels callback yet, so do it here.
	 */
	if (db_rep->msg_dispatch == NULL &&
	    (ret = __repmgr_set_write_forwarding(env, 1)) != 0) {
		__db_err(env, ret, "forward_single set_wf subordinate");
		return (ret);
	}

	/*
	 * Cannot support a bulk put or del because repmgr channels does
	 * not support DBTs formatted as bulk buffers.
	 */
	if (FLD_ISSET(opflags, DB_MULTIPLE_KEY | DB_MULTIPLE))
		return (EACCES);

	for (i = 0; i < REPMGR_WF_MAX_DBTS; i++)
		memset(&msgdbts[i], 0, sizeof(DBT));
	memset(&response, 0, sizeof(DBT));
	response.flags = DB_DBT_MALLOC;

	/*
	 * A note about numeric value-sharing in the write forwarding
	 * protocol: the repmgr channels protocol uses scatter-gather
	 * IO which depends on the iovecs array.  The maximum number of
	 * iovec segments is governed on each platform by IOV_MAX.  On
	 * some platforms IOV_MAX is very small (e.g. 16 on Solaris.)
	 * The repmgr channels protocol uses 3 iovec slots for overhead
	 * information, 1 slot for each DBT passed in, and possibly
	 * 1 additional slot for each DBT if padding is needed.  This
	 * means that write forwarding can pass in a maximum of 6 DBTs
	 * to operate on systems like Solaris.
	 *
	 * Write forwarding uses more than 6 separate pieces of information,
	 * so the write forwarding protocol packs two smaller numbers into
	 * a larger number value in a few cases to help conserve DBT slots.
	 */

	/* Pack write forwarding identifier and protocol version. */
	wfidvers.unum64 = 0;
	wfidvers.unum32[0] = REPMGR_WF_IDENTIFIER;
	wfidvers.unum32[1] = REPMGR_WF_VERSION;
	(&msgdbts[0])->data = &wfidvers;
	(&msgdbts[0])->size = sizeof(wfidvers);

	/* Pack write operation type and metapgno. */
	if (optype == 0 || optype > REPMGR_WF_MAX_V1_MSG_TYPE) {
		__db_err(env, ret, "forward_single invalid optype %u", optype);
		return (EINVAL);
	}
	opmeta.unum64 = 0;
	opmeta.unum32[0] = optype;
	opmeta.unum32[1] = dbp->meta_pgno;
	(&msgdbts[1])->data = &opmeta;
	(&msgdbts[1])->size = sizeof(opmeta);

	/* Pack database flags and operation flags. */
	twoflags.unum64 = 0;
	twoflags.unum32[0] = dbp->flags;
	twoflags.unum32[1] = opflags;
	(&msgdbts[2])->data = &twoflags;
	(&msgdbts[2])->size = sizeof(twoflags);

	/*
	 * Pack database fileid.  There is no need to specify a
	 * directory because there can only be one database file with
	 * a given name across all of the data directories.  The code
	 * in __db_appname() that finds a database file first searches
	 * all data directories for an existing file of that name and
	 * will only create a new file in the create directory if it
	 * doesn't find an existing one.
	 */
	(&msgdbts[3])->data = (void *)(dbp->fileid);
	(&msgdbts[3])->size = DB_FILE_ID_LEN;

	VPRINT(env, (env, DB_VERB_REPMGR_MISC,
	    "repmgr_forward_single_write: optype %d opflags %u",
	    optype, opflags));
	REPMGR_WF_DUMP_FILEID(dbp->fileid, i, fidstr);
	VPRINT(env, (env, DB_VERB_REPMGR_MISC,
	    "repmgr_forward_single_write: dbflags %u fileid %s meta_pgno %u",
	    dbp->flags, fidstr, dbp->meta_pgno));

	/* Pack write operation information. */
	msgdbts[4] = *key;
	switch (optype) {
	case REPMGR_WF_SINGLE_DEL:
		nsegs = 5;
		break;
	case REPMGR_WF_SINGLE_PUT:
		/*
		 * Lint complains about a possible NULL data value from
		 * the SINGLE_DEL call even though this switch never allows
		 * the NULL value to be used here.
		 */
		if (data != NULL) {
			msgdbts[5] = *data;
			nsegs = 6;
		}
		break;
	default:
		return (EINVAL);
	}

	if ((ret = __repmgr_channel(dbenv,
	    DB_EID_MASTER, &wfchannel, 0)) != 0) {
		/* Translate repmgr channels error into put/del error. */
		if (ret == DB_REP_UNAVAIL)
			ret = EACCES;
		__db_err(env, ret, "forward_single repmgr_channel");
		return (ret);
	}

	if ((ret = __repmgr_send_request(wfchannel, msgdbts,
	    nsegs, &response, rep->write_forward_timeout, 0)) != 0) {
		/* Translate repmgr channels error into put/del error. */
		if (ret == DB_NOSERVER)
			ret = EACCES;
		__db_err(env, ret, "forward_single channel->send_request");
		goto close_channel;
	}
	STAT(rep->mstat.st_write_ops_forwarded++);

	if (response.size > 0) {
		ret = *(int *)response.data;
		free(response.data);
		if (ret != 0)
			__db_err(env, ret, "forward_single response");
	}

close_channel:
	if ((ret2 = __repmgr_channel_close(wfchannel, 0)) != 0) {
		__db_err(env, ret2, "forward_single channel->close");
		if (ret == 0)
			ret = ret2;
	}
	VPRINT(env, (env, DB_VERB_REPMGR_MISC,
	    "repmgr_forward_single_write: returning %d", ret));
	return (ret);
}

/*
 * PUBLIC: void __repmgr_msgdispatch __P((DB_ENV *, DB_CHANNEL *, DBT *,
 * PUBLIC:     u_int32_t, u_int32_t));
 *
 * This is the repmgr channels message dispatch callback for repmgr write
 * forwarding.  This is the master-side processing that gets invoked when
 * the master receives a forwarded write operation via repmgr channels.
 * The supported write operations are a single put or a single del.  Each
 * operation uses its own implicit master transaction.
 *
 * The master must already have an open database handle for the database
 * on which the operation is being performed.
 */
void
__repmgr_msgdispatch(dbenv, ch, request, nseg, flags)
	DB_ENV *dbenv;
	DB_CHANNEL *ch;
	DBT *request;
	u_int32_t nseg;
	u_int32_t flags;
{
	DB *ldbp;
	DB_REP *db_rep;
	ENV *env;
	REP *rep;
	DBT data, key, response;
	char fidstr[REPMGR_WF_FILEID_STRLEN];
	wf_uint32_pair opmeta, twoflags, wfidvers;
	u_int32_t dbflags, opflags, optype, wfprotid, wfprotvers;
	db_pgno_t metapgno;
	int got_runrecovery, i, ret, ret2;
	u_int8_t *fileid;

	env = dbenv->env;
	db_rep = env->rep_handle;
	rep = db_rep->region;
	ret = 0;
	ret2 = 0;
	got_runrecovery = 0;

	/* Make sure we received an expected number of DBTs. */
	if (nseg < REPMGR_WF_MIN_DBTS || nseg > REPMGR_WF_MAX_DBTS) {
		ret = EACCES;
		__db_err(env, ret, "repmgr_msgdispatch wrong # DBTs");
		goto send_response;
	}

	/*
	 * Unpack write forwarding identifier and version.  Verify that
	 * caller supplied expected write forwarding identifier to make
	 * sure this isn't some other type of repmgr channels message or
	 * random noise.
	 */
	wfidvers.unum64 = *(u_int64_t *)request[0].data;
	wfprotid = wfidvers.unum32[0];
	wfprotvers = wfidvers.unum32[1];
	if (wfprotid != REPMGR_WF_IDENTIFIER) {
		ret = EACCES;
		__db_err(env, ret, "repmgr_msgdispatch bad id");
		goto send_response;
	}

	/* Unpack operation type and metapgno. */
	opmeta.unum64 = *(u_int64_t *)request[1].data;
	optype = opmeta.unum32[0];
	metapgno = opmeta.unum32[1];
	if (optype == 0 || optype > REPMGR_WF_MAX_V1_MSG_TYPE) {
		ret = EACCES;
		__db_err(env, ret, "repmgr_msgdispatch invalid optype");
		goto send_response;
	}
	VPRINT(env, (env, DB_VERB_REPMGR_MISC,
	    "repmgr_msgdispatch: protid %u protvers %u optype %u",
	    wfprotid, wfprotvers, optype));
	STAT(rep->mstat.st_write_ops_received++);

	/* Unpack database and operation flags. */
	twoflags.unum64 = *(u_int64_t *)request[2].data;
	dbflags = twoflags.unum32[0];
	opflags = twoflags.unum32[1];

	/* Unpack database fileid. */
	fileid = (u_int8_t *)request[3].data;
	REPMGR_WF_DUMP_FILEID(fileid, i, fidstr);
	VPRINT(env, (env, DB_VERB_REPMGR_MISC,
	    "repmgr_msgdispatch: dbflags %u fileid %s metapgno %u",
	    dbflags, fidstr, metapgno));

	/*
	 * Search the environment's dblist for an open handle for this
	 * database.  The dblist needs mutex protection.  We use the
	 * metapgno to identify the correct subdatabase.
	 *
	 * We cannot use the MUTEX_[UN]LOCK() macros because they can
	 * return DB_RUNRECOVERY and this message dispatch callback is void,
	 * so use the alternative MUTEX_[UN]LOCK_RET() macros instead.
	 * We perform special handling for DB_RUNRECOVERY below.
	 */
	if ((ret = MUTEX_LOCK_RET(env, env->mtx_dblist)) != 0) {
		__db_err(env, ret, "repmgr_msgdispatch mutex_lock");
		goto send_response;
	}
	TAILQ_FOREACH(ldbp, &env->dblist, dblistlinks) {
		if (memcmp(ldbp->fileid, fileid, DB_FILE_ID_LEN) == 0 &&
		    ldbp->meta_pgno == metapgno)
			break;
	}
	if ((ret = MUTEX_UNLOCK_RET(env, env->mtx_dblist)) != 0) {
		__db_err(env, ret, "repmgr_msgdispatch mutex_unlock");
		goto send_response;
	}
	/* If there isn't an open database handle, we can't do anything. */
	if (ldbp == NULL) {
		ret = EACCES;
		__db_err(env, ret, "repmgr_msgdispatch no open dbp");
		goto send_response;
	}
	VPRINT(env, (env, DB_VERB_REPMGR_MISC,
	    "repmgr_msgdispatch: db filename %s dbname %s opflags %u",
	    ldbp->fname, ldbp->dname, opflags));

	/* Unpack write operation information and perform write operation. */
	key = request[4];
	switch (optype) {
	case REPMGR_WF_SINGLE_DEL:
		if ((ret = __db_del_pp(ldbp, NULL, &key, opflags)) != 0) {
			/* Send client access error if database file is gone. */
			if (ret == ENOENT)
				ret = EACCES;
			/* Send success if deleted data is already gone. */
			if (ret == DB_NOTFOUND || ret == DB_KEYEMPTY)
				ret = 0;
			if (ret != 0)
				__db_err(env, ret,
				    "repmgr_msgdispatch del error");
			goto send_response;
		}
		break;
	case REPMGR_WF_SINGLE_PUT:
		data = request[5];
		if ((ret =
		    __db_put_pp(ldbp, NULL, &key, &data, opflags)) != 0) {
			/* Send client access error if database file is gone. */
			if (ret == ENOENT)
				ret = EACCES;
			if (ret != 0)
				__db_err(env, ret,
				    "repmgr_msgdispatch put error");
			goto send_response;
		}
		break;
	default:
		ret = EACCES;
		__db_err(env, ret, "repmgr_msgdispatch invalid optype");
		goto send_response;
	}

send_response:
	/*
	 * It is possible that the mutex routines returned DB_RUNRECOVERY.
	 * If they or anything else returned DB_RUNRECOERY, we don't want to
	 * pass that back to the client and cause a panic there.  Send EACCES
	 * to the client and then panic this master environment.
	 */
	if (ret == DB_RUNRECOVERY) {
		got_runrecovery = 1;
		__db_err(env, ret, "repmgr_msgdispatch RUNRECOVERY panic env");
		ret = EACCES;
	}
	/* Send either success or an error back to the client. */
	if (flags & DB_REPMGR_NEED_RESPONSE) {
		memset(&response, 0, sizeof(response));
		response.data = &ret;
		response.size = sizeof(ret);
		if ((ret2 = __repmgr_send_response(ch, &response, 1, 0)) == 0)
		    VPRINT(env, (env, DB_VERB_REPMGR_MISC,
			"repmgr_msgdispatch: sent response %d", ret));
		else
			__db_err(env, ret2, "repmgr_msgdispatch send_msg");
	}
	/* Now we can panic this master environment if necessary. */
	if (got_runrecovery)
		(void)__env_panic(env, DB_RUNRECOVERY);
}

/*
 * PUBLIC: int __repmgr_set_write_forwarding __P((ENV *, int));
 *
 * Enable or disable write forwarding as specified by the value turn_on.
 * This consists of setting or disabling the repmgr write forwarding
 * message dispatch callback for use by repmgr channels.
 */
int
__repmgr_set_write_forwarding(env, turn_on)
	ENV *env;
	int turn_on;
{
	DB_ENV *dbenv;
	int ret;

	dbenv = env->dbenv;

	if (turn_on)
		ret = __repmgr_set_msg_dispatch(dbenv, __repmgr_msgdispatch, 0);
	else
		ret = __repmgr_set_msg_dispatch(dbenv, NULL, 0);
	return (ret);

}
