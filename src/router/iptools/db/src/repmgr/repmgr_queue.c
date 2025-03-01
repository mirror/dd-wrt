/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2006, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */

#include "db_config.h"

#include "db_int.h"

static REPMGR_MESSAGE *available_work __P((ENV *));

/*
 * Deallocates memory used by all messages on the queue.
 *
 * PUBLIC: int __repmgr_queue_destroy __P((ENV *));
 */
int
__repmgr_queue_destroy(env)
	ENV *env;
{
	DB_REP *db_rep;
	REP *rep;
	REPMGR_MESSAGE *m;
	REPMGR_CONNECTION *conn;
	u_int32_t mtype;
	int ret, t_ret;

	COMPQUIET(mtype, 0);

	db_rep = env->rep_handle;
	rep = db_rep->region;

	ret = 0;

	/*
	 * Turn on the DB_EVENT_REP_INQUEUE_FULL event firing.  We only do
	 * this for the main listener process.  For a subordinate process,
	 * it is always turned on.
	 */
	if (!STAILQ_EMPTY(&db_rep->input_queue.header) &&
	    !IS_SUBORDINATE(db_rep))
		rep->inqueue_full_event_on = 1;

	while (!STAILQ_EMPTY(&db_rep->input_queue.header)) {
		m = STAILQ_FIRST(&db_rep->input_queue.header);
		STAILQ_REMOVE_HEAD(&db_rep->input_queue.header, entries);
		if (m->msg_hdr.type == REPMGR_APP_MESSAGE) {
			if ((conn = m->v.appmsg.conn) != NULL &&
			    (t_ret = __repmgr_decr_conn_ref(env, conn)) != 0 &&
			    ret == 0)
				ret = t_ret;
		}
		if (m->msg_hdr.type == REPMGR_OWN_MSG) {
			mtype = REPMGR_OWN_MSG_TYPE(m->msg_hdr);
			if ((conn = m->v.gmdb_msg.conn) != NULL) {
				/*
				 * A site that removed itself may have already
				 * closed its connections.
				 */
				if ((t_ret = __repmgr_close_connection(env,
				    conn)) != 0 && ret == 0 &&
				    mtype != REPMGR_REMOVE_REQUEST)
					ret = t_ret;
				if ((t_ret = __repmgr_decr_conn_ref(env,
				    conn)) != 0 && ret == 0)
					ret = t_ret;
			}
		}
		__os_free(env, m);
	}

	return (ret);
}

/*
 * PUBLIC: int __repmgr_queue_get __P((ENV *,
 * PUBLIC:     REPMGR_MESSAGE **, REPMGR_RUNNABLE *));
 *
 * Get the first input message from the queue and return it to the caller.  The
 * caller hereby takes responsibility for the entire message buffer, and should
 * free it when done.
 *
 * Caller must hold mutex.
 */
int
__repmgr_queue_get(env, msgp, th)
	ENV *env;
	REPMGR_MESSAGE **msgp;
	REPMGR_RUNNABLE *th;
{
	DB_REP *db_rep;
	REP *rep;
	REPMGR_MESSAGE *m;
#ifdef DB_WIN32
	HANDLE wait_events[2];
#endif
	u_int32_t msgsize;
	int ret;

	ret = 0;
	db_rep = env->rep_handle;
	rep = db_rep->region;

	while ((m = available_work(env)) == NULL &&
	    db_rep->repmgr_status == running && !th->quit_requested) {
#ifdef DB_WIN32
		/*
		 * On Windows, msg_avail means either there's something in the
		 * queue, or we're all finished.  So, reset the event if that is
		 * not true.
		 */
		if (STAILQ_EMPTY(&db_rep->input_queue.header) &&
		    db_rep->repmgr_status == running &&
		    !ResetEvent(db_rep->msg_avail)) {
			ret = GetLastError();
			goto err;
		}
		wait_events[0] = db_rep->msg_avail;
		wait_events[1] = th->quit_event;
		UNLOCK_MUTEX(db_rep->mutex);
		ret = WaitForMultipleObjects(2, wait_events, FALSE, INFINITE);
		LOCK_MUTEX(db_rep->mutex);
		if (ret == WAIT_FAILED) {
			ret = GetLastError();
			goto err;
		}

#else
		if ((ret = pthread_cond_wait(&db_rep->msg_avail,
		    db_rep->mutex)) != 0)
			goto err;
#endif
	}
	if (db_rep->repmgr_status == stopped || th->quit_requested)
		ret = DB_REP_UNAVAIL;
	else {
		STAILQ_REMOVE(&db_rep->input_queue.header,
		    m, __repmgr_message, entries);
		msgsize = (u_int32_t)m->size;
		while (msgsize >= GIGABYTE) {
			DB_ASSERT(env, db_rep->input_queue.gbytes > 0);
			db_rep->input_queue.gbytes--;
			msgsize -= GIGABYTE;
		}
		if (db_rep->input_queue.bytes < msgsize) {
			DB_ASSERT(env, db_rep->input_queue.gbytes > 0);
			db_rep->input_queue.gbytes--;
			db_rep->input_queue.bytes += GIGABYTE;
		}
		db_rep->input_queue.bytes -= msgsize;

		/*
		 * Check if current size is out of the red zone.
		 * If it is, we will turn on the DB_EVENT_REP_INQUEUE_FULL
		 * event firing.
		 *
		 * We only have the redzone machanism for the main listener
		 * process.
		 */
		if (!IS_SUBORDINATE(db_rep) &&
		    rep->inqueue_full_event_on == 0) {
			MUTEX_LOCK(env, rep->mtx_repmgr);
			if (db_rep->input_queue.gbytes <
			    rep->inqueue_rz_gbytes ||
			    (db_rep->input_queue.gbytes ==
			    rep->inqueue_rz_gbytes &&
			    db_rep->input_queue.bytes <
			    rep->inqueue_rz_bytes))
				rep->inqueue_full_event_on = 1;
			MUTEX_UNLOCK(env, rep->mtx_repmgr);
		}

		*msgp = m;
	}
err:
	return (ret);
}

/*
 * Gets an "available" item of work (i.e., a message) from the input queue.  If
 * there are plenty of message threads currently available, then we simply
 * return the first thing on the queue, regardless of what type of message it
 * is.  But otherwise skip over any message type that may possibly turn out to
 * be "long-running", so that we avoid starving out the important rep message
 * processing.
 */
static REPMGR_MESSAGE *
available_work(env)
	ENV *env;
{
	DB_REP *db_rep;
	REPMGR_MESSAGE *m;

	db_rep = env->rep_handle;
	if (STAILQ_EMPTY(&db_rep->input_queue.header))
		return (NULL);
	/*
	 * The "non_rep_th" field is the dynamically varying count of threads
	 * currently processing non-replication messages (a.k.a. possibly
	 * long-running messages, a.k.a. "deferrable").  We always ensure that
	 * db_rep->nthreads > reserved.
	 */
	if (db_rep->nthreads > db_rep->non_rep_th + RESERVED_MSG_TH(env))
		return (STAILQ_FIRST(&db_rep->input_queue.header));
	STAILQ_FOREACH(m, &db_rep->input_queue.header, entries) {
		if (!IS_DEFERRABLE(m->msg_hdr.type))
			return (m);
	}
	return (NULL);
}

/*
 * PUBLIC: int __repmgr_queue_put __P((ENV *, REPMGR_MESSAGE *));
 *
 * !!!
 * Caller must hold repmgr->mutex.
 */
int
__repmgr_queue_put(env, msg)
	ENV *env;
	REPMGR_MESSAGE *msg;
{
	DB_REP *db_rep;
	REP *rep;
	u_int32_t msgsize;

	db_rep = env->rep_handle;
	rep = db_rep->region;

	/*
	 * Drop message if incoming queue contains more messages than the
	 * limit.  See dbenv->repmgr_set_incoming_queue_max() for more
	 * information.
	 */
	MUTEX_LOCK(env, rep->mtx_repmgr);
	if (db_rep->input_queue.gbytes > rep->inqueue_max_gbytes ||
	    (db_rep->input_queue.gbytes == rep->inqueue_max_gbytes &&
	    db_rep->input_queue.bytes >= rep->inqueue_max_bytes)) {
		RPRINT(env, (env, DB_VERB_REPMGR_MISC,
		    "incoming queue limit exceeded"));
		STAT(rep->mstat.st_incoming_msgs_dropped++);
		if (IS_SUBORDINATE(db_rep) || rep->inqueue_full_event_on) {
			DB_EVENT(env, DB_EVENT_REP_INQUEUE_FULL, NULL);
			/*
			 * We will always disable the event firing after
			 * the queue is full.  It will be enabled again
			 * after the incoming queue size is out of the
			 * redzone.
			 *
			 * We only have the redzone machanism for the main
			 * listener process.
			 */
			if (!IS_SUBORDINATE(db_rep))
				rep->inqueue_full_event_on = 0;
		}
		MUTEX_UNLOCK(env, rep->mtx_repmgr);
		__os_free(env, msg);
		return (0);
	}
	MUTEX_UNLOCK(env, rep->mtx_repmgr);

	STAILQ_INSERT_TAIL(&db_rep->input_queue.header, msg, entries);
	msgsize = (u_int32_t)msg->size;
	while (msgsize >= GIGABYTE) {
		msgsize -= GIGABYTE;
		db_rep->input_queue.gbytes++;
	}
	db_rep->input_queue.bytes += msgsize;
	if (db_rep->input_queue.bytes >= GIGABYTE) {
		db_rep->input_queue.gbytes++;
		db_rep->input_queue.bytes -= GIGABYTE;
	}

	return (__repmgr_signal(&db_rep->msg_avail));
}
