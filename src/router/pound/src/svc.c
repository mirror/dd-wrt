/*
 * Pound - the reverse-proxy load-balancer
 * Copyright (C) 2002-2010 Apsis GmbH
 * Copyright (C) 2022-2025 Sergey Poznyakoff
 *
 * Pound is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pound is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pound.h"
#include "extern.h"
#include "json.h"
#include <assert.h>

#if ! SET_DH_AUTO
static void init_rsa (void);
static void do_RSAgen (enum job_ctl, void *, const struct timespec *);
#endif

/* Periodic jobs */
typedef struct job
{
  JOB_ID id;
  struct timespec ts;
  JOB_FUNC func;
  void *data;
  DLIST_ENTRY (job) link;
} JOB;

typedef DLIST_HEAD (,job) JOB_HEAD;

typedef struct jobcancel
{
  JOB_ID id;
  DLIST_ENTRY (jobcancel) link;
} JOBCNCL;

typedef DLIST_HEAD (,jobcancel) JOBCNCL_HEAD;

static JOB_HEAD job_head = DLIST_HEAD_INITIALIZER (job_head);
static JOBCNCL_HEAD jobcncl_head = SLIST_HEAD_INITIALIZER (jobcncl_head);
static JOB_ID job_next_id;
static pthread_cond_t job_cond = PTHREAD_COND_INITIALIZER;

static pthread_once_t job_key_once = PTHREAD_ONCE_INIT;
static pthread_mutex_t _job_mutex = PTHREAD_MUTEX_INITIALIZER;

static void
job_mutex_init (void)
{
  pthread_mutex_init (&_job_mutex, &mutex_attr_recursive);
}

static pthread_mutex_t *
job_mutex (void)
{
  pthread_once (&job_key_once, job_mutex_init);
  return &_job_mutex;
}

static JOB *
job_alloc (struct timespec const *ts, JOB_FUNC func, void *data)
{
  JOB *job;

  XZALLOC (job);
  job->ts = *ts;
  job->func = func;
  job->data = data;
  return job;
}

static JOB_ID
job_arm_unlocked (JOB *job)
{
  JOB *t;
  JOB_ID jid = job_next_id++;

  job->id = jid;

  DLIST_FOREACH (t, &job_head, link)
    {
      if (timespec_cmp (&job->ts, &t->ts) < 0)
	break;
    }

  if (t == NULL)
    DLIST_INSERT_TAIL (&job_head, job, link);
  else
    DLIST_INSERT_BEFORE (&job_head, t, job, link);

  if (DLIST_PREV (job, link) == NULL)
    pthread_cond_broadcast (&job_cond);

  return jid;
}

static JOB_ID
job_enqueue_unlocked (struct timespec const *ts, JOB_FUNC func, void *data)
{
  return job_arm_unlocked (job_alloc (ts, func, data));
}

JOB_ID
job_enqueue (struct timespec const *ts, JOB_FUNC func, void *data)
{
  JOB_ID jid;

  pthread_mutex_lock (job_mutex ());
  jid = job_enqueue_unlocked (ts, func, data);
  pthread_mutex_unlock (job_mutex ());
  return jid;
}

static JOB_ID
job_enqueue_after_unlocked (unsigned t, JOB_FUNC func, void *data)
{
  struct timespec ts;
  clock_gettime (CLOCK_REALTIME, &ts);
  ts.tv_sec += t;
  return job_enqueue_unlocked (&ts, func, data);
}

JOB_ID
job_enqueue_after (unsigned t, JOB_FUNC func, void *data)
{
  JOB_ID jid;
  pthread_mutex_lock (job_mutex ());
  jid = job_enqueue_after_unlocked (t, func, data);
  pthread_mutex_unlock (job_mutex ());
  return jid;
}

static void
job_remove (JOB_ID jid)
{
  JOB *job, *tmp;
  DLIST_FOREACH_SAFE (job, tmp, &job_head, link)
    {
      if (job->id == jid)
	{
	  struct timespec ts;
	  clock_gettime (CLOCK_REALTIME, &ts);
	  job->func (job_ctl_cancel, job->data, &ts);
	  DLIST_REMOVE (&job_head, job, link);
	  free (job);
	  break;
	}
    }
}

int
job_get_timestamp (JOB_ID jid, struct timespec *ts)
{
  int rc = 1;
  JOB *job;
  pthread_mutex_lock (job_mutex ());
  DLIST_FOREACH (job, &job_head, link)
    {
      if (job->id == jid)
	{
	  *ts = job->ts;
	  rc = 0;
	  break;
	}
    }
  pthread_mutex_unlock (job_mutex ());
  return rc;
}

void
job_cancel (JOB_ID id)
{
  JOBCNCL *jc;
  XZALLOC (jc);
  jc->id = id;
  pthread_mutex_lock (job_mutex ());
  DLIST_PUSH (&jobcncl_head, jc, link);
  pthread_cond_broadcast (&job_cond);
  pthread_mutex_unlock (job_mutex ());
}

static void
timer_cleanup (void *ptr)
{
  pthread_mutex_unlock (job_mutex ());
}

void *
thr_job_runner (void *arg)
{
  JOB *job = arg;
  job->func (job_ctl_run, job->data, &job->ts);
  free (job);
  return NULL;
}

/*
 * run periodic functions:
 */
void *
thr_timer (void *arg)
{
  /* Seed the job queue */
#if ! SET_DH_AUTO
  init_rsa ();
  job_enqueue_after (T_RSA_KEYS, do_RSAgen, NULL);
#endif

  pthread_mutex_lock (job_mutex ());
  pthread_cleanup_push (timer_cleanup, NULL);

  for (;;)
    {
      int rc;
      JOB *job;

      while (!DLIST_EMPTY (&jobcncl_head))
	{
	  JOBCNCL *jc = DLIST_FIRST (&jobcncl_head);
	  job_remove (jc->id);
	  DLIST_SHIFT (&jobcncl_head, link);
	  free (jc);
	}

      if (DLIST_EMPTY (&job_head))
	{
	  pthread_cond_wait (&job_cond, job_mutex ());
	  continue;
	}

      job = DLIST_FIRST (&job_head);

      rc = pthread_cond_timedwait (&job_cond, job_mutex (), &job->ts);
      if (rc == 0)
	{
	  continue;
	}
      else if (rc == ETIMEDOUT)
	{
	  if (job != DLIST_FIRST (&job_head))
	    /* Job was removed or its time changed */
	    continue;
	  else
	    {
	      pthread_t tid;
	      DLIST_SHIFT (&job_head, link);
	      pthread_create (&tid, &thread_attr_detached, thr_job_runner,
			      job);
	    }
	}
      else
	abend (NULL, "unexpected error from pthread_cond_timedwait: %s",
	       strerror (rc));
    }
  pthread_cleanup_pop (1);
}

/* Session functions */

SESSION_TABLE *
session_table_new (void)
{
  SESSION_TABLE *st;
  XZALLOC (st);
  if ((st->hash = SESSION_HASH_NEW ()) == NULL)
    xnomem ();
  DLIST_INIT (&st->head);
  return st;
}

static inline void
session_link_at_tail (SESSION_TABLE *tab, SESSION *sess)
{
  DLIST_INSERT_TAIL (&tab->head, sess, link);
}

static inline void
session_unlink (SESSION_TABLE *tab, SESSION *sess)
{
  DLIST_REMOVE (&tab->head, sess, link);
}

static void
session_free (SESSION *sess)
{
  free (sess);
}

/*
 * Periodic job to remove expired sessions within the given service (passed
 * as the DATA argument),  Sessions within the session table are ordered
 * by their expiration time.  The expire_sessions cronjob is normally scheduled
 * to the expiration time of the first session in the list, i.e. the least
 * recently used one.
 *
 * Before returning, the function rearms the periodic job if necessary.
 */
static void
expire_sessions (enum job_ctl ctl, void *data, const struct timespec *now)
{
  SERVICE *svc = data;
  SESSION_TABLE *tab;
  SESSION *sess;

  if (ctl != job_ctl_run)
    return;

  pthread_mutex_lock (&svc->mut);

  tab = svc->sessions;
  while ((sess = DLIST_FIRST (&tab->head)) != NULL &&
	 timespec_cmp (&sess->expire, now) <= 0)
    {
      SESSION_DELETE (tab->hash, sess);
      session_unlink (tab, sess);
      session_free (sess);
    }

  if (!DLIST_EMPTY (&tab->head))
    job_enqueue (&DLIST_FIRST (&tab->head)->expire, expire_sessions, svc);
  pthread_mutex_unlock (&svc->mut);
}

/*
 * Promote the given session by updating its expiration time and
 * moving it to the end of the session list.
 *
 * The service mutex must be locked.
 */
static void
service_session_promote (SERVICE *svc, SESSION *sess)
{
  SESSION_TABLE *tab = svc->sessions;
  session_unlink (tab, sess);
  clock_gettime (CLOCK_REALTIME, &sess->expire);
  sess->expire.tv_sec += svc->sess_ttl;
  session_link_at_tail (tab, sess);
}

/*
 * Add a new key/backend pair to the session table of SVC.  If this is
 * going to be the the first session in the session list, schedule the
 * expiration job.
 *
 * The service mutex must be locked.
 */
static void
service_session_add (SERVICE *svc, const char *key, BACKEND *be)
{
  SESSION_TABLE *tab = svc->sessions;
  SESSION *t, *old;
  size_t keylen = strlen (key);

  if ((t = malloc (sizeof (SESSION) + keylen + 1)) == NULL)
    {
      logmsg (LOG_WARNING, "service_session_add() content malloc");
      return;
    }
  t->key = (char*)(t + 1);
  strcpy (t->key, key);
  t->backend = be;
  clock_gettime (CLOCK_REALTIME, &t->expire);
  t->expire.tv_sec += svc->sess_ttl;
  if ((old = SESSION_INSERT (tab->hash, t)) != NULL)
    {
      session_unlink (svc->sessions, old);
      session_free (old);
      logmsg (LOG_WARNING, "service_session_add() DUP");
    }
  session_link_at_tail (tab, t);
  if (DLIST_NEXT (DLIST_FIRST (&tab->head), link) == 0)
    job_enqueue (&t->expire, expire_sessions, svc);
}

/*
 * Look up the service session table for the given key.  If found,
 * return the corresponding backend and promote the session.
 *
 * The service mutex must be locked.
 */
static BACKEND *
service_session_find (SERVICE *svc, char *const key)
{
  SESSION_TABLE *tab = svc->sessions;
  SESSION t, *res;

  t.key = key;
  if ((res = SESSION_RETRIEVE (tab->hash, &t)) != NULL)
    {
      service_session_promote (svc, res);
      return res->backend;
    }
  return NULL;
}

/*
 * Delete from the service session table a session corresponding to the
 * given key.
 *
 * The service mutex must be locked.
 */
static void
service_session_remove_by_key (SERVICE *svc, char const *key)
{
  SESSION_TABLE *tab = svc->sessions;
  SESSION t, *res;

  t.key = (char*) key;
  if ((res = SESSION_DELETE (tab->hash, &t)) != NULL)
    {
      session_unlink (tab, res);
      session_free (res);
    }
  return;
}

/*
 * Remove from the service session table all sessions with the given backend.
 *
 * The service mutex must be locked.
 */
void
service_session_remove_by_backend (SERVICE *svc, BACKEND *be)
{
  SESSION_TABLE *tab = svc->sessions;
  SESSION *sess, *tmp;

  DLIST_FOREACH_SAFE (sess, tmp, &tab->head, link)
    {
      if (sess->backend == be)
	{
	  SESSION_DELETE (tab->hash, sess);
	  session_unlink (tab, sess);
	  session_free (sess);
	}
    }
}

/*
 * Translate inet/inet6 address/port into a string
 */
char *
addr2str (char *res, int res_len, const struct addrinfo *addr, int no_port)
{
  int n;
  char *ptr = res;

  if (res == NULL || res_len <= 0 || addr == NULL)
    return NULL;

  ptr[res_len - 1] = 0;
  --res_len;

  if (addr->ai_family == AF_UNIX)
    {
      struct sockaddr_un *sun = (struct sockaddr_un *)addr->ai_addr;
      n = addr->ai_addrlen - offsetof (struct sockaddr_un, sun_path);
      if (n > res_len)
	n = res_len;
      strncpy (ptr, sun->sun_path, n);
      if (ptr[n-1] != 0 && n < res_len)
	ptr[n] = 0;
    }
  else
    {
      char hostbuf[NI_MAXHOST];
      char portbuf[NI_MAXSERV];

      int rc = getnameinfo (addr->ai_addr, addr->ai_addrlen,
			    hostbuf, sizeof (hostbuf),
			    portbuf, sizeof (portbuf),
			    NI_NUMERICHOST | NI_NUMERICSERV);

      if (rc)
	{
	  logmsg (LOG_ERR, "getnameinfo: %s", gai_strerror (rc));
	  strncpy (ptr, "(UNKNOWN)", res_len);
	}
      else
	{
	  if (addr->ai_family == AF_INET6)
	    snprintf (ptr, res_len+1, "[%s]", hostbuf);
	  else
	    strncpy (ptr, hostbuf, res_len);

	  n = strlen (ptr);
	  ptr += n;
	  res_len -= n;

	  if (!no_port)
	    {
	      if (res_len)
		{
		  *ptr++ = ':';
		  res_len--;
		  strncpy (ptr, portbuf, res_len);
		}
	    }
	}
    }
  return res;
}

/* Return a string representation for a back-end address */
char *
str_be (char *buf, size_t size, BACKEND *be)
{
  switch (be->be_type)
    {
    case BE_REGULAR:
      addr2str (buf, size, &be->v.reg.addr, 0);
      break;

    case BE_MATRIX:
      snprintf (buf, size, "matrix:%s", be->v.mtx.hostname);
      break;

    case BE_REDIRECT:
      snprintf (buf, size, "redirect:%s", be->v.redirect.url);
      break;

    case BE_ACME:
      snprintf (buf, size, "acme");
      break;

    case BE_CONTROL:
      strncpy (buf, "control", size);
      break;

    case BE_ERROR:
      if (be->v.error.msg.text)
	snprintf (buf, size, "error:%d:text",
		  pound_to_http_status (be->v.error.status));
      else
	snprintf (buf, size, "error:%d",
		  pound_to_http_status (be->v.error.status));
      break;

    case BE_METRICS:
      strncpy (buf, "metrics", size);
      break;

    case BE_FILE:
      strncpy (buf, "file", size);
      break;

    case BE_LUA:
      snprintf (buf, size, "lua:%s", be->v.lua.func);
      break;

    default:
      abort ();
    }
  return buf;
}

/*
 * Match the request obtained via PHTTP against conditions from the service
 * SVC.  Return value:
 *
 *  0 request doesn't match or an error occurred;
 *  1 request matches.
 */
static int
match_service (SERVICE *svc, POUND_HTTP *phttp)
{
  int rc;
  /*
   * Temporarily assume this service.  This is needed to properly
   * expand %[remoteip 1] constructs, since TrustedIP lists can be declared
   * inside a service.
   */
  phttp->svc = svc;
  rc = match_cond (&svc->cond, phttp, &phttp->request);
  phttp->svc = NULL;
  return rc == 1;
}

/*
 * Find the right service for a request
 */
SERVICE *
get_service (POUND_HTTP *phttp)
{
  SERVICE *svc;

  SLIST_FOREACH (svc, &phttp->lstn->services, next)
    {
      if (svc->disabled)
	continue;
      if (match_service (svc, phttp))
	return svc;
    }

  /* try global services */
  SLIST_FOREACH (svc, &services, next)
    {
      if (svc->disabled)
	continue;
      if (match_service (svc, phttp))
	return svc;
    }

  /* nothing matched */
  return NULL;
}

/*
 * Calculate a uniformly distributed random number less than max.
 * avoiding "modulo bias".
 *
 * The code is based on arc4random_uniform from OpenBSD, see
 * http://cvsweb.openbsd.org/cgi-bin/cvsweb/~checkout~/src/lib/libc/crypt/arc4r\
andom_uniform.c
 *
 * Uniformity is achieved by generating new random numbers until the one
 * returned is outside the range [0, 2**32 % max).  This
 * guarantees the selected random number will be inside
 * [2**32 % max, 2**32) which maps back to [0, max)
 * after reduction modulo max.
 */
static long
random_in_range (unsigned long max)
{
  long r;
  unsigned long min;

  if (max < 2)
    return 0;
  min = - max % max;
  do
    r = random ();
  while (r < min);
  return r % max;
}

static void
rand_pri_init (BALANCER *bl)
{
  bl->rand.sum_pri = 0;
}

static int
rand_pri_update (BALANCER *bl, BACKEND *be)
{
  if (LONG_MAX - bl->rand.sum_pri < be->priority)
    {
      conf_error_at_locus_range (&be->locus,
			    "this backend overflows the sum of priorities");
      return 1;
    }
  bl->rand.sum_pri += be->priority;
  return 0;
}

static BACKEND *
rand_select (BALANCER *balancer)
{
  BACKEND *be;
  int pri;

  if (DLIST_EMPTY (&balancer->backends))
    return NULL;

  pri = random_in_range (balancer->rand.sum_pri);
  DLIST_FOREACH (be, &balancer->backends, link)
    {
      if (backend_is_active (be) && (pri -= be->priority) < 0)
	break;
    }
  return be;
}

static void
iwrr_init (BALANCER *balancer)
{
  balancer->iwrr.round = 0;
  balancer->iwrr.cur = DLIST_FIRST (&balancer->backends);
}

static void
iwrr_reset (BALANCER *balancer, BACKEND *be)
{
  if (balancer->iwrr.cur == be)
    {
      iwrr_init (balancer);
    }
}

static BACKEND *
iwrr_select (BALANCER *bal)
{
  BACKEND *be = NULL;

  if (bal->iwrr.cur == NULL)
    {
      iwrr_init (bal);
      if (bal->iwrr.cur == NULL)
	return NULL;
    }

  do
    {
      if (backend_is_active (bal->iwrr.cur))
	{
	  if (bal->iwrr.round < bal->iwrr.cur->priority)
	    be = bal->iwrr.cur;
	}
      if ((bal->iwrr.cur = DLIST_NEXT (bal->iwrr.cur, link)) == NULL)
	{
	  bal->iwrr.cur = DLIST_FIRST (&bal->backends);
	  bal->iwrr.round = (bal->iwrr.round + 1) % (bal->iwrr.max_pri + 1);
	}
    }
  while (be == NULL);
  return be;
}

static void
iwrr_pri_init (BALANCER *bal)
{
  bal->iwrr.max_pri = 0;
}

static int
iwrr_pri_update (BALANCER *bal, BACKEND *be)
{
  if (be->priority == INT_MAX)
    {
      conf_error_at_locus_range (&be->locus, "priority value too big");
      return 1;
    }
  if (bal->iwrr.max_pri < be->priority)
    bal->iwrr.max_pri = be->priority;
  return 0;
}

struct balancer_def
{
  void (*init) (BALANCER *);
  void (*reset) (BALANCER *, BACKEND *be);
  BACKEND *(*select) (BALANCER *);
  void (*pri_init) (BALANCER *);
  int (*pri_update) (BALANCER *, BACKEND *);
};

static struct balancer_def balancer_tab[] = {
  [BALANCER_ALGO_RANDOM] = {
    .select = rand_select,
    .pri_init = rand_pri_init,
    .pri_update = rand_pri_update
  },
  [BALANCER_ALGO_IWRR] = {
    .init = iwrr_init,
    .reset = iwrr_reset,
    .select = iwrr_select,
    .pri_init = iwrr_pri_init,
    .pri_update = iwrr_pri_update
  }
};

static inline BACKEND *
balancer_select_backend (BALANCER *balancer)
{
  BACKEND *be;

  if (DLIST_NEXT (DLIST_FIRST (&balancer->backends), link) == NULL)
    {
      be = DLIST_FIRST (&balancer->backends);
      if (backend_is_active (be))
	return be;
      return NULL;
    }
  return balancer_tab[balancer->algo].select (balancer);
}

static inline void
balancer_init (BALANCER *balancer)
{
  if (balancer_tab[balancer->algo].init)
    balancer_tab[balancer->algo].init (balancer);
}

static inline void
balancer_reset (BALANCER *balancer, BACKEND *be)
{
  if (balancer_tab[balancer->algo].reset)
    balancer_tab[balancer->algo].reset (balancer, be);
}

static inline void
balancer_pri_init (BALANCER *balancer)
{
  if (balancer_tab[balancer->algo].pri_init)
    balancer_tab[balancer->algo].pri_init (balancer);
}

static inline int
balancer_pri_update (BALANCER *balancer, BACKEND *be)
{
  if (balancer_tab[balancer->algo].pri_update)
    return balancer_tab[balancer->algo].pri_update (balancer, be);
  return 0;
}

BACKEND *
service_lb_select_backend (SERVICE *svc)
{
  BALANCER *balancer;
  DLIST_FOREACH (balancer, &svc->balancers, link)
    {
      BACKEND *be = balancer_select_backend (balancer);
      if (be)
	return be;
    }
  return NULL;
}

void
service_lb_init (SERVICE *svc)
{
  BALANCER *balancer;
  DLIST_FOREACH (balancer, &svc->balancers, link)
    {
      balancer_init (balancer);
    }
}

void
service_lb_reset (SERVICE *svc, BACKEND *be)
{
  BALANCER *balancer;
  DLIST_FOREACH (balancer, &svc->balancers, link)
    {
      balancer_reset (balancer, be);
    }
}

/*
 * Find backend by session key.  If no session with the given key is found,
 * create one and associate it with a randomly selected backend.
 */
static BACKEND *
find_backend_by_key (SERVICE *svc, char const *key)
{
  BACKEND *res;
  char keybuf[KEY_SIZE + 1];

  if (key == NULL)
    return NULL;

  strncpy (keybuf, key, sizeof (keybuf) - 1);
  keybuf[sizeof (keybuf) - 1] = 0;

  if ((res = service_session_find (svc, keybuf)) == NULL)
    {
      /* no session yet - create one */
      if ((res = service_lb_select_backend (svc)) != NULL)
	service_session_add (svc, keybuf, res);
    }

  return res;
}

/*
 * Find session key using the header name and key extractor function.
 *
 * The function looks up the header HNAME in the header list HEADERS.
 * If found, the key extractor function KEYFUN is invoked with the
 * header value, ID, and RET_KEY as its arguments.  The function shall
 * extract the key from the value and place it in the RET_KEY buffer,
 * assuming it is KEY_SIZE+1 bytes long and truncating the value as
 * necessary.  On success, KEYFUN shall return 0.
 *
 * If KEYFUN is NULL, the obtained header value (at most first KEY_SIZE
 * octets of it) is copied to RET_KEY verbatim.
 */
int
find_key_by_header (HTTP_HEADER_LIST *headers, char const *hname,
		    int (*keyfun) (char const *, char const *, char *),
		    char const *id,
		    char *ret_key)
{
  struct http_header *hdr;
  char const *val;

  if ((hdr = http_header_list_locate_name (headers,
					   hname,
					   strlen (hname))) != NULL
      && (val = http_header_get_value (hdr)) != NULL)
    {
      if (keyfun)
	return keyfun (val, id, ret_key);
      else
	{
	  strncpy (ret_key, val, KEY_SIZE);
	  ret_key[KEY_SIZE] = 0;
	  return 0;
	}
    }
  return 1;
}

/*
 * Look up for the backend in sessions using the header name, key
 * extraction function and session ID.  See the two functions above
 * for details.
 */
static BACKEND *
find_backend_by_header (SERVICE *svc,
			struct http_request *req, char const *hname,
			int (*keyfun) (char const *, char const *, char *),
			char const *id)
{
  char key[KEY_SIZE + 1];

  if (find_key_by_header (&req->headers, hname, keyfun, id, key) == 0)
    {
      return find_backend_by_key (svc, key);
    }

  return NULL;
}

/*
 * Key extractor function for Authorized header (basic authentication).
 */
static int
key_authbasic (char const *hval, char const *sid, char *ret_key)
{
  if (c_strncasecmp (hval, "Basic", 5) == 0 && c_isspace (hval[5]))
    {
      for (hval += 6; *hval && c_isspace (*hval); hval++)
	;
      strncpy (ret_key, hval, KEY_SIZE);
      ret_key[KEY_SIZE] = 0;
      return 0;
    }
  return 1;
}

/*
 * Key extractor function for Cookie header.  Extracts the value of
 * the parameter SID.
 */
static int
key_cookie (char const *hval, char const *sid, char *ret_key)
{
  size_t slen = strlen (sid);
  while (*hval)
    {
      size_t n;

      hval += strspn (hval, " \t");
      if (*hval == 0)
	break;

      n = strcspn (hval, ";");

      if (n > slen && hval[slen] == '=' && c_strncasecmp (hval, sid, slen) == 0)
	{
	  hval += slen + 1;
	  n -= slen + 1;
	  if (n > KEY_SIZE)
	    n = KEY_SIZE;
	  memcpy (ret_key, hval, n);
	  ret_key[n] = 0;
	  return 0;
	}

      hval += n;
      if (hval[0] == 0)
	break;
      hval++;
    }
  return 1;
}

static int
service_has_backends (SERVICE *svc)
{
  BALANCER *bl;
  DLIST_FOREACH (bl, &svc->balancers, link)
    {
      if (bl->act_num > 0)
	return 1;
    }
  return 0;
}

/*
 * Find the right back-end for a request
 */
BACKEND *
get_backend (POUND_HTTP *phttp)
{
  SERVICE *svc = phttp->svc;
  BACKEND *res = NULL;
  char keybuf[KEY_SIZE + 1];
  char const *key;

  pthread_mutex_lock (&svc->mut);

  if (service_has_backends (svc))
    {
      switch (svc->sess_type)
	{
	case SESS_NONE:
	  /* choose one back-end randomly */
	  break;

	case SESS_COOKIE:
	  res = find_backend_by_header (svc, &phttp->request,
					"Cookie", key_cookie, svc->sess_id);
	  break;

	case SESS_IP:
	  addr2str (keybuf, sizeof (keybuf), &phttp->from_host, 1);
	  res = find_backend_by_key (svc, keybuf);
	  break;

	case SESS_URL:
	  if (http_request_get_query_param_value (&phttp->request,
						  svc->sess_id, &key) == RETRIEVE_OK)
	    res = find_backend_by_key (svc, key);
	  break;

	case SESS_PARM:
	  if (http_request_get_path (&phttp->request, &key) == 0)
	    {
	      char *p = strrchr (key, ';');
	      if (p)
		res = find_backend_by_key (svc, p + 1);
	    }
	  break;

	case SESS_HEADER:
	  res = find_backend_by_header (svc, &phttp->request,
					svc->sess_id, NULL, NULL);
	  break;

	case SESS_BASIC:
	  res = find_backend_by_header (svc, &phttp->request,
					"Authorization", key_authbasic, NULL);
	}

      if (!res)
	res = service_lb_select_backend (svc);
      backend_ref (res);
    }

  pthread_mutex_unlock (&svc->mut);

  return res;
}

/*
 * Create session based on response headers.
 */
void
upd_session (SERVICE *svc, HTTP_HEADER_LIST *headers, BACKEND *be)
{
  char *hname;
  char key[KEY_SIZE + 1];
  int (*keyfun) (char const *, char const *, char *);

  switch (svc->sess_type)
    {
    case SESS_HEADER:
      hname = svc->sess_id;
      keyfun = NULL;
      break;

    case SESS_COOKIE:
      hname = "Set-Cookie";
      keyfun = key_cookie;
      break;

    case SESS_BASIC:
      hname = "Authorization";
      keyfun = key_authbasic;
      break;

    default:
      return;
    }

  pthread_mutex_lock (&svc->mut);
  if (find_key_by_header (headers, hname, keyfun, svc->sess_id, key) == 0)
    {
      if (service_session_find (svc, key) == NULL)
	service_session_add (svc, key, be);
    }
  pthread_mutex_unlock (&svc->mut);
}

/*
 * Recompute max. backend priority and sum of priorities for the backend list.
 * If call-back function cb is supplied, call it for each backend.
 *
 * NOTICE:
 *  1. The service should be locked prior to calling this function.
 *  2. Sum of priorities of all the backends should not overflow the type
 *     of svc->tot_pri.  For static backends, this is always true.  For
 *     dynamic backends, this should be ensured when allocating them.
 */
void
balancer_recompute_pri_unlocked (BALANCER *bl,
				 void (*cb) (BACKEND *, void *),
				 void *data)
{
  BACKEND *b;

  bl->act_num = 0;
  balancer_pri_init (bl);
  DLIST_FOREACH (b, &bl->backends, link)
    {
      if (cb)
	cb (b, data);
      if (backend_is_active (b))
	{
	  if (balancer_pri_update (bl, b))
	    b->disabled = 1;
	  else
	    bl->act_num++;
	}
    }
}

void
service_recompute_pri_unlocked (SERVICE *svc,
				void (*cb) (BACKEND *, void *),
				void *data)
{
  BALANCER *bl;
  DLIST_FOREACH (bl, &svc->balancers, link)
    {
      balancer_recompute_pri_unlocked (bl, cb, data);
    }
}

/*
 * Recompute max. backend priority and sum of priorities for the given
 * backend list of the service.  If call-back function cb is supplied,
 * call it for each backend.
 *
 * If bl == NULL, recompute all backend lists.
 *
 * Locks the svc mutex prior to use.  See second notice to
 * balancer_recompute_pri_unlocked, above.
 */
void
service_recompute_pri (SERVICE *svc, BALANCER *bl,
		       void (*cb) (BACKEND *, void *),
		       void *data)
{
  pthread_mutex_lock (&svc->mut);
  if (bl)
    balancer_recompute_pri_unlocked (bl, cb, data);
  else
    {
      DLIST_FOREACH (bl, &svc->balancers, link)
	{
	  balancer_recompute_pri_unlocked (bl, cb, data);
	}
    }
  pthread_mutex_unlock (&svc->mut);
}

struct disable_closure
{
  BACKEND *be;
  int disable_mode;
};

static void touch_be (enum job_ctl ctl, void *data, const struct timespec *ts);

/*
 * NOTE:
 * Changing the disabled state of a backend cannot be done separately from
 * the service where that backend is hosted, because service priorities
 * depend on it.  Updating priorities after state change would create a
 * race condition.  Therefore, state change is done as part of priority
 * recalculation, while service mutex is locked.  It is thus guaranteed
 * that the disabled field of any backend is safe to be accessed for
 * reading while the hosting service of that backend is locked.
 *
 * The convention of using service mutex for locking access to the
 * disabled field of a backend (instead of that of the backend itself) is
 * used throughout the code.  In particular, it is used in backend
 * serialization code below as well as in matrix backend manuipulations
 * (see dynbe.c).
 */
static void
cb_backend_disable (BACKEND *b, void *data)
{
  struct disable_closure *c = data;
  char buf[MAXBUF];

  if (b == c->be)
    {
      switch (c->disable_mode)
	{
	case BE_DISABLE:
	  b->disabled = 1;
	  str_be (buf, sizeof (buf), b);
	  logmsg (LOG_NOTICE, "(%"PRItid") Backend %s disabled",
		  POUND_TID (),
		  buf);
	  break;

	case BE_KILL:
	  b->v.reg.alive = 0;
	  str_be (buf, sizeof (buf), b);
	  logmsg (LOG_NOTICE, "(%"PRItid") Backend %s dead (killed)",
		  POUND_TID (), buf);
	  service_session_remove_by_backend (b->service, b);
	  backend_ref (b);
	  job_enqueue_after (alive_to, touch_be, b);
	  break;

	case BE_ENABLE:
	  str_be (buf, sizeof (buf), b);
	  logmsg (LOG_NOTICE, "(%"PRItid") Backend %s enabled",
		  POUND_TID (),
		  buf);
	  b->disabled = 0;
	  break;
	}
    }
}

/*
 * mark a backend host as dead/disabled; remove its sessions if necessary
 *  disable_only == BE_DISABLE:  mark as disabled
 *  disable_only == BE_KILL:     mark as dead, remove sessions
 *  disable_only == BE_ENABLE:   mark as enabled
 */
void
kill_be (SERVICE *svc, BACKEND *be, const int disable_mode)
{
  if (be->be_type == BE_REGULAR)
    {
      struct disable_closure clos;
      clos.be = be;
      clos.disable_mode = disable_mode;
      assert (be->balancer != NULL);
      service_recompute_pri (svc, be->balancer, cb_backend_disable, &clos);
    }
}

static void
backend_disable (SERVICE *svc, BACKEND *be, const int disable_mode)
{
  switch (be->be_type)
    {
    case BE_REGULAR:
      kill_be (svc, be, disable_mode);
      break;

#if ENABLE_DYNAMIC_BACKENDS
    case BE_MATRIX:
      backend_matrix_disable (be, disable_mode);
      break;
#endif

    default:
      break;
    }
}

/*
 * Search for a host name, return the addrinfo for it
 */
int
get_host (char const *name, struct addrinfo *res, int ai_family)
{
  struct addrinfo *chain;
  struct addrinfo hints;
  int ret_val;

  memset (&hints, 0, sizeof (hints));
  hints.ai_family = ai_family;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = (feature_is_set (FEATURE_DNS) ? 0 : AI_NUMERICHOST);
  if ((ret_val = getaddrinfo (name, NULL, &hints, &chain)) == 0)
    {
      *res = *chain;
      if ((res->ai_addr = malloc (chain->ai_addrlen)) == NULL)
	{
	  freeaddrinfo (chain);
	  return EAI_MEMORY;
	}
      memcpy (res->ai_addr, chain->ai_addr, chain->ai_addrlen);
      freeaddrinfo (chain);
    }
  return ret_val;
}

/* Return 1 if the protocol designation PROTO of length LEN equals PAT. */
static inline int
is_proto (char const *pat, char const *proto, size_t len)
{
  return len == strlen (pat) && c_strncasecmp (proto, pat, len) == 0;
}

union sockaddr_union
{
  struct sockaddr_in i;
  struct sockaddr_in6 i6;
  struct sockaddr_storage s;
};

/* Set port value in the sockaddr_union. */
static void
sun_set_port (union sockaddr_union *sun, int port)
{
  switch (sun->s.ss_family)
    {
    case AF_INET:
      sun->i.sin_port = port;
      break;

    case AF_INET6:
      sun->i6.sin6_port = port;
      break;
    }
}

/*
 * Return 1 if any of the addresses from ADDR with port changed to PORT
 * coincides with the address of the backend BE.
 */
static int
is_backend_address (struct addrinfo const *addr, int port, const BACKEND *be)
{
  union sockaddr_union sun_be, sun;

  memcpy (&sun_be, be->v.reg.addr.ai_addr, be->v.reg.addr.ai_addrlen);

  for (; addr; addr = addr->ai_next)
    {
      if (addr->ai_family == sun_be.s.ss_family)
	{
	  memcpy (&sun, addr->ai_addr, addr->ai_addrlen);
	  sun_set_port (&sun, port);
	  if (memcmp (&sun_be, &sun, addr->ai_addrlen) == 0)
	    return 1;
	}
    }
  return 0;
}

/*
 * Return 1 if any of the ADDR addresses matches the address of the given
 * listener.  Ignore port number in comparisons.
 */
static int
is_listener_address (struct addrinfo const *addr, const LISTENER *lstn)
{
  union sockaddr_union sun_lstn, sun;

  if (!addr)
    return 0;

  memcpy (&sun_lstn, lstn->addr.ai_addr, lstn->addr.ai_addrlen);
  sun_set_port (&sun_lstn, 0);

  for (; addr; addr = addr->ai_next)
    {
      if (addr->ai_family != sun_lstn.s.ss_family)
	{
	  memcpy (&sun, addr->ai_addr, addr->ai_addrlen);
	  sun_set_port (&sun, 0);
	  if (memcmp (&sun_lstn, &sun, addr->ai_addrlen) == 0)
	    return 1;
	}
    }
  return 0;
}

/* Return port number of the listener LSTN. */
static int
listener_port (const LISTENER *lstn)
{
  union sockaddr_union *p = (union sockaddr_union *)&lstn->addr.ai_addr;
  switch (p->s.ss_family)
    {
    case AF_INET:
      return p->i.sin_port;

    case AF_INET6:
      return p->i6.sin6_port;
    }
  return 0;
}

/*
 * Test if the redirection location value needs rewriting.  Return 1 if so.
 * Depending on the value of the RewriteLocation setting (lstn->rewr_loc):
 *
 *   when 0       - no rewrite occurs; the function returns 0;
 *   when 1       - rewrite is needed if the location points to the backend
 *                  or to the listener with wrong port number of protocol;
 *   when 2       - rewrite is needed if the location points to the backend.
 *
 * Input arguments:
 *   location     -  value of the Location: header;
 *   v_host       -  host part of the incoming request;
 *   lstn         -  selected listener;
 *   be           -  backend the response came from;
 * Output argument:
 *   ppath        -  return path part of the location.
 */
int
need_rewrite (const char *location, const char *v_host,
	      const LISTENER *lstn, const BACKEND *be, const char **ppath)
{
  POUND_REGMATCH matches[4];
  char const *proto;
  char const *path;
  int port;
  char *cp;
  size_t len;
  char *host, *vhost;
  int result = 0;
  struct addrinfo hints, *addr = NULL;

  if (lstn->rewr_loc == 0)
    return 0;

  /* applies only to INET/INET6 back-ends */
  if (be->v.reg.addr.ai_family != AF_INET && be->v.reg.addr.ai_family != AF_INET6)
    return 0;

  /* split the location into its fields */
  if (genpat_match (LOCATION, location, 4, matches))
    return 0;
  proto = location + matches[1].rm_so;

  if (location[matches[3].rm_so] == '/')
    matches[3].rm_so++;
  path = location + matches[3].rm_so;

  len = matches[2].rm_eo - matches[2].rm_so;
  if ((host = malloc (len + 1)) == NULL)
    {
      lognomem ();
      return 0;
    }
  memcpy (host, location + matches[2].rm_so, len);
  host[len] = 0;

  if ((cp = strchr (host, ':')) != NULL)
    {
      *cp++ = '\0';
      port = htons (atoi (cp));
    }
  else if (is_proto ("https", proto, matches[1].rm_eo - matches[1].rm_so))
    port = htons (PORT_HTTPS);
  else
    port = htons (PORT_HTTP);

  if ((vhost = strdup (v_host)) == NULL)
    {
      lognomem ();
      free (host);
      return 0;
    }

  if ((cp = strchr (vhost, ':')) != NULL)
    *cp = '\0';

  memset (&hints, 0, sizeof (hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_CANONNAME |
		    (feature_is_set (FEATURE_DNS) ? 0 : AI_NUMERICHOST);
  if (getaddrinfo (host, NULL, &hints, &addr) == 0)
    {
      result = is_backend_address (addr, port, be);
    }
  else
    result = (be->v.reg.servername &&
	      c_strcasecmp (host, be->v.reg.servername) == 0 &&
	      is_proto (be->v.reg.ctx ? "https" : "http",
			proto,
			matches[1].rm_eo - matches[1].rm_so));

  if (result == 0 && lstn->rewr_loc == 1)
    {
      result = ((is_listener_address (addr, lstn) ||
		 c_strcasecmp (host, vhost) == 0) ||
		((port != listener_port (lstn) ||
		  !is_proto (SLIST_EMPTY (&lstn->ctx_head) ? "http" : "https",
			     proto,
			     matches[1].rm_eo - matches[1].rm_so))));
    }

  if (addr)
    freeaddrinfo (addr);
  free (vhost);
  free (host);

  if (result)
    *ppath = path;
  return result;
}

/*
 * Non-blocking connect(). Does the same as connect(2) but ensures
 * it will time-out after a much shorter time period SERVER_TO
 */
int
connect_nb (const int sockfd, const struct addrinfo *serv_addr, const int to)
{
  int flags;
  struct pollfd p;
  char caddr[MAX_ADDR_BUFSIZE];

  if ((flags = fcntl (sockfd, F_GETFL, 0)) < 0)
    {
      logmsg (LOG_WARNING, "(%"PRItid") connect_nb: %s: fcntl GETFL failed: %s",
	      POUND_TID (),
	      addr2str (caddr, sizeof (caddr), serv_addr, 0),
	      strerror (errno));
      return -1;
    }
  if (fcntl (sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
      logmsg (LOG_WARNING, "(%"PRItid") connect_nb: %s: fcntl SETFL failed: %s",
	      POUND_TID (),
	      addr2str (caddr, sizeof (caddr), serv_addr, 0),
	      strerror (errno));
      return -1;
    }

  if (connect (sockfd, serv_addr->ai_addr, serv_addr->ai_addrlen) < 0)
    {
      if (errno != EINPROGRESS)
	{
	  logmsg (LOG_WARNING, "(%"PRItid") connect_nb: %s: connect failed: %s",
		  POUND_TID (),
		  addr2str (caddr, sizeof (caddr), serv_addr, 0),
		  strerror (errno));
	  return -1;
	}

      memset (&p, 0, sizeof (p));
      p.fd = sockfd;
      p.events = POLLOUT;
      switch (poll (&p, 1, to * 1000))
	{
	case 1:
	  break;

	case 0:
	  /* timeout */
	  logmsg (LOG_WARNING, "(%"PRItid") connect_nb: %s: poll timed out",
		  POUND_TID (),
		  addr2str (caddr, sizeof (caddr), serv_addr, 0));
	  errno = ETIMEDOUT;
	  return -1;

	default:
	  logmsg (LOG_WARNING, "(%"PRItid") connect_nb: %s: poll failed: %s",
		  POUND_TID (),
		  addr2str (caddr, sizeof (caddr), serv_addr, 0),
		  strerror (errno));
	  return -1;
	}

      if ((p.revents & (POLLOUT | POLLERR | POLLHUP)) != POLLOUT)
	{
	  int error;
	  socklen_t len = sizeof (error);
	  if (getsockopt (sockfd, SOL_SOCKET, SO_ERROR, &error, &len) == 0)
	    {
	      logmsg (LOG_WARNING,
		      "(%"PRItid") connect_nb: %s: connection failed: %s",
		      POUND_TID (),
		      addr2str (caddr, sizeof (caddr), serv_addr, 0),
		      strerror (error));
	      errno = error;
	    }
	  else
	    logmsg (LOG_WARNING,
		    "(%"PRItid") connect_nb: %s: getsockopt failed: %s",
		    POUND_TID (),
		    addr2str (caddr, sizeof (caddr), serv_addr, 0),
		    strerror (errno));
	  return -1;
	}
    }

  /* restore file status flags */
  if (fcntl (sockfd, F_SETFL, flags) < 0)
    {
      logmsg (LOG_WARNING, "(%"PRItid") connect_nb: %s: fcntl restore SETFL failed: %s",
	      POUND_TID (),
	      addr2str (caddr, sizeof (caddr), serv_addr, 0),
	      strerror (errno));
      return -1;
    }

  /* really connected */
  return 0;
}

enum
  {
    BACKEND_OK,
    BACKEND_DEAD,
    BACKEND_ERR,
  };

static int
backend_probe (BACKEND *be)
{
  int sock;
  int family;
  int rc;

  switch (be->v.reg.addr.ai_family)
    {
    case AF_INET:
      family = PF_INET;
      break;

    case AF_INET6:
      family = PF_INET6;
      break;

    case AF_UNIX:
      family = PF_UNIX;
      break;

    default:
      errno = EINVAL;
      return BACKEND_ERR;
    }

  if ((sock = socket (family, SOCK_STREAM, 0)) < 0)
    return BACKEND_ERR;

  rc = connect_nb (sock, &be->v.reg.addr, be->v.reg.conn_to);
  shutdown (sock, 2);
  close (sock);

  return rc == 0 ? BACKEND_OK : BACKEND_DEAD;
}

/*
 * Periodic job: check if the backend passed as argument is alive.
 * If so, return it to the pool of backends.  Otherwise, reschedule
 * itself for alive_to seconds later.
 */
static void
touch_be (enum job_ctl ctl, void *data, const struct timespec *ts)
{
  BACKEND *be = data;
  char buf[MAXBUF];

  if (ctl == job_ctl_run && be->be_type == BE_REGULAR && backend_referenced(be))
    {
      if (!be->v.reg.alive)
	{
	  if (backend_probe (be) == BACKEND_OK)
	    {
	      be->v.reg.alive = 1;
	      str_be (buf, sizeof (buf), be);
	      logmsg (LOG_NOTICE, "Backend %s resurrected", buf);
	      if (!be->disabled)
		{
		  pthread_mutex_lock (&be->service->mut);
		  balancer_pri_update (be->balancer, be);
		  be->balancer->act_num++;
		  pthread_mutex_unlock (&be->service->mut);
		}
	    }
	  else
	    {
	      job_enqueue_after (alive_to, touch_be, be);
	      return;
	    }
	}
    }
  backend_unref (be);
}

#if ! SET_DH_AUTO
static pthread_mutex_t RSA_mut;	/* mutex for RSA keygen */
static RSA *RSA512_keys[N_RSA_KEYS];	/* ephemeral RSA keys */
static RSA *RSA1024_keys[N_RSA_KEYS];	/* ephemeral RSA keys */

/*
 * return a pre-generated RSA key
 */
static RSA *
RSA_tmp_callback ( /* not used */ SSL * ssl, /* not used */ int is_export,
		  int keylength)
{
  RSA *res;
  int ret_val;

  if ((ret_val = pthread_mutex_lock (&RSA_mut)) != 0)
    logmsg (LOG_WARNING, "RSA_tmp_callback() lock: %s", strerror (ret_val));
  res = (keylength <= 512) ? RSA512_keys[rand () % N_RSA_KEYS]
			   : RSA1024_keys[rand () % N_RSA_KEYS];
  if ((ret_val = pthread_mutex_unlock (&RSA_mut)) != 0)
    logmsg (LOG_WARNING, "RSA_tmp_callback() unlock: %s", strerror (ret_val));
  return res;
}

static int
generate_key (RSA ** ret_rsa, unsigned long bits)
{
  int rc = 0;
  RSA *rsa;

  rsa = RSA_new ();
  if (rsa)
    {
      BIGNUM *bne = BN_new ();
      if (BN_set_word (bne, RSA_F4))
	rc = RSA_generate_key_ex (rsa, bits, bne, NULL);
      BN_free (bne);
      if (rc)
	*ret_rsa = rsa;
      else
	RSA_free (rsa);
    }
  return rc;
}

/*
 * Periodically regenerate ephemeral RSA keys
 * runs every T_RSA_KEYS seconds
 */
static void
do_RSAgen (enum job_ctl ctl, void *unused1, const struct timespec *unused2)
{
  int n, ret_val;
  RSA *t_RSA512_keys[N_RSA_KEYS];
  RSA *t_RSA1024_keys[N_RSA_KEYS];

  if (ctl != job_ctl_run)
    return;

  /* Re-arm the job */
  job_enqueue_after (T_RSA_KEYS, do_RSAgen, NULL);

  for (n = 0; n < N_RSA_KEYS; n++)
    {
      /* FIXME: Error handling */
      generate_key (&t_RSA512_keys[n], 512);
      generate_key (&t_RSA1024_keys[n], 1024);
    }
  if ((ret_val = pthread_mutex_lock (&RSA_mut)) != 0)
    logmsg (LOG_WARNING, "thr_RSAgen() lock: %s", strerror (ret_val));
  for (n = 0; n < N_RSA_KEYS; n++)
    {
      RSA_free (RSA512_keys[n]);
      RSA512_keys[n] = t_RSA512_keys[n];
      RSA_free (RSA1024_keys[n]);
      RSA1024_keys[n] = t_RSA1024_keys[n];
    }
  if ((ret_val = pthread_mutex_unlock (&RSA_mut)) != 0)
    logmsg (LOG_WARNING, "thr_RSAgen() unlock: %s", strerror (ret_val));
}

#include    "dh.h"
static DH *DH512_params, *DHALT_params;

static DH *
DH_tmp_callback ( /* not used */ SSL * s, /* not used */ int is_export,
		 int keylength)
{
  return keylength == 512 ? DH512_params : DHALT_params;
}

/*
 * initialise RSA_mut and keys
 */
void
init_rsa (void)
{
  int n;

  /*
   * Pre-generate ephemeral RSA keys
   */
  for (n = 0; n < N_RSA_KEYS; n++)
    {
      if (!generate_key (&RSA512_keys[n], 512))
	{
	  logmsg (LOG_WARNING, "RSA_generate(%d, 512) failed", n);
	  return;
	}
      if (!generate_key (&RSA1024_keys[n], 1024))
	{
	  logmsg (LOG_WARNING, "RSA_generate(%d, 1024) failed", n);
	  return;
	}
    }
  /* pthread_mutex_init() always returns 0 */
  pthread_mutex_init (&RSA_mut, NULL);

  DH512_params = get_dh512 ();
#if DH_LEN == 1024
  DHALT_params = get_dh1024 ();
#else
  DHALT_params = get_dh2048 ();
#endif
  return;
}

#ifndef OPENSSL_NO_ECDH
static int EC_nid = NID_X9_62_prime256v1;
#endif

int
set_ECDHCurve (char *name)
{
  int n;

  if ((n = OBJ_sn2nid (name)) == 0)
    return -1;
  EC_nid = n;
  return 0;
}

void
POUND_SSL_CTX_init (SSL_CTX *ctx)
{
  SSL_CTX_set_tmp_rsa_callback (ctx, RSA_tmp_callback);
  SSL_CTX_set_tmp_dh_callback (ctx, DH_tmp_callback);
#ifndef OPENSSL_NO_ECDH
  /* This generates a EC_KEY structure with no key, but a group defined */
  EC_KEY *ecdh;
  if ((ecdh = EC_KEY_new_by_curve_name (EC_nid)) == NULL)
    abend (NULL, "Unable to generate temp ECDH key");

  SSL_CTX_set_tmp_ecdh (ctx, ecdh);
  SSL_CTX_set_options (ctx, SSL_OP_SINGLE_ECDH_USE);
  EC_KEY_free (ecdh);
#endif
}
#else /* SET_DH_AUTO == 1 */
void
POUND_SSL_CTX_init (SSL_CTX *ctx)
{
  SSL_CTX_set_dh_auto (ctx, 1);
}
#endif

static char *
get_param (char const *url, char const *param, size_t *ret_len)
{
  char *p;
  size_t paramlen = strlen (param);

  for (p = strchr (url, '?'); p && *++p; p = strchr (p, '&'))
    {
      if (strncmp (p, param, paramlen) == 0 && p[paramlen] == '=')
	{
	  p += paramlen + 1;
	  *ret_len = strcspn (p, "&");
	  break;
	}
    }
  return p;
}

static int
session_backend_index (SESSION *sess)
{
  int n = 0;
  BALANCER *balancer;

  DLIST_FOREACH (balancer, &sess->backend->service->balancers, link)
    {
      BACKEND *be;
      DLIST_FOREACH (be, &balancer->backends, link)
	{
	  if (be == sess->backend)
	    break;
	  n++;
	}
    }
  return n;
}

static struct json_value *
timespec_serialize (struct timespec const *ts)
{
  char buf[sizeof("1970-01-01T00:00:00.000000")];
  struct tm tm;
  size_t n;

  n = strftime (buf, sizeof (buf), "%Y-%m-%dT%H:%M:%S",
		localtime_r (&ts->tv_sec, &tm));
  snprintf (buf + n, sizeof (buf) - n, ".%06ld", ts->tv_nsec / 1000);
  return json_new_string (buf);
}

/* Duration is represented in milliseconds. */
static struct json_value *
duration_serialize (struct timespec const *ts)
{
  return json_new_number ((double)ts->tv_sec * 1000000 + ts->tv_nsec / 1000);
}

static struct json_value *
service_session_serialize (SERVICE *svc)
{
  struct json_value *obj;
  int err = 0;

  if (svc->sess_type == SESS_NONE)
    obj = json_new_null ();
  else
    {
      obj = json_new_array ();
      if (obj && svc->sessions)
	{
	  SESSION *sess;

	  DLIST_FOREACH (sess, &svc->sessions->head, link)
	    {
	      struct json_value *s = json_new_object ();

	      if (!s)
		{
		  err = 1;
		  break;
		}
	      else if (json_array_append (obj, s))
		{
		  json_value_free (s);
		  err = 1;
		  break;
		}
	      else if (json_object_set (s, "key", json_new_string (sess->key))
		       || json_object_set (s, "backend",
					   json_new_integer (session_backend_index (sess)))
		       || json_object_set (s, "expire", timespec_serialize (&sess->expire)))
		{
		  err = 1;
		  break;
		}
	    }
	}
    }

  if (err)
    {
      json_value_free (obj);
      obj = NULL;
    }
  return obj;
}

static char const *
backend_type_str (BACKEND_TYPE t)
{
  switch (t)
    {
    case BE_MATRIX:
      return "matrix";

    case BE_REGULAR:
      return "backend";

    case BE_REDIRECT:
      return "redirect";

    case BE_ACME:
      return "acme";

    case BE_CONTROL:
      return "control";

    case BE_ERROR:
      return "error";

    case BE_METRICS:
      return "metrics";

    case BE_FILE:
      return "file";

    default: /* BE_REGULAR_REF can't happen at this stage. */
      break;
    }

  return "UNKNOWN";
}

static struct json_value *
addrinfo_serialize (struct addrinfo *addr)
{
  char buf[MAX_ADDR_BUFSIZE];

  addr2str (buf, sizeof (buf), addr, 0);
  return json_new_string (buf);
}

static double
nabs (double a)
{
  return (a < 0) ? -a : a;
}

double
nsqrt (double a, double prec)
{
  double x0, x1;

  if (a < 0)
    return 0;
  if (a < prec)
    return 0;
  x1 = a / 2;
  do
    {
      x0 = x1;
      x1 = (x0 + a / x0) / 2;
    }
  while (nabs (x1 - x0) > prec);

  return x1;
}

static struct json_value *
backend_stats_serialize (BACKEND *be)
{
  struct json_value *obj;

  if ((obj = json_new_object ()) != NULL)
    {
      int err = 0;

      pthread_mutex_lock (&be->mut);
      err |= json_object_set (obj, "request_count", json_new_number (be->numreq));
      if (be->numreq > 0)
	{
	  err |= json_object_set (obj, "request_time_avg",
				  json_new_number (be->avgtime))
	    || json_object_set (obj, "request_time_stddev",
				json_new_number (nsqrt (be->avgsqtime - be->avgtime * be->avgtime, 0.5)));
	}
      pthread_mutex_unlock (&be->mut);
    }
  return obj;
}

/*
 * Find index of the backend in the list.
 * FIXME: Grossly ineffective.  Think how to cache this info.
 */
static int
find_backend_index (BACKEND *be)
{
  int i = 0;
  BALANCER *balancer;
  DLIST_FOREACH (balancer, &be->service->balancers, link)
    {
      BACKEND *b;
      DLIST_FOREACH (b, &balancer->backends, link)
	{
	  if (b == be)
	    return i;
	  i++;
	}
    }
  return -1;
}

/*
 * Add information about dynamic backend generation:
 *  If be is a matrix backend, its expiration time is stored in
 *  attribute "expire".
 *  If it is a regular dynamically created backend, the index of
 *  the matrix backend that created it is stored in attribute
 *  "parent".
 *  For any other backend types, do nothing.
 */
static int
backend_serialize_dyninfo (struct json_value *obj, BACKEND *be)
{
  BACKEND *up = be, *parent;
  int err = 0;

  while (up)
    {
      parent = up;
      switch (up->be_type)
	{
	case BE_MATRIX:
	  up = up->v.mtx.parent;
	  break;

	case BE_REGULAR:
	  up = up->v.reg.parent;
	  break;

	default:
	  return 0;
	}
    }
  if (parent == NULL)
    return 0;
  if (parent != be)
    {
      err = json_object_set (obj, "parent",
			     json_new_number (find_backend_index (parent)));
    }
  else if (err == 0 && be->be_type == BE_MATRIX)
    {
      struct timespec ts;

      if (job_get_timestamp (parent->v.mtx.jid, &ts) == 0)
	err = json_object_set (obj, "expire", timespec_serialize (&ts));
    }
  return err;
}

static inline struct json_value *
new_typed_object (char const *type)
{
  struct json_value *obj = json_new_object ();
  if (type && json_object_set (obj, "kind", json_new_string (type)))
    {
      json_value_free (obj);
      obj = NULL;
    }
  return obj;
}

static struct json_value *
backend_serialize (BACKEND *be)
{
  struct json_value *obj;
  int err = 0;

  if (!be)
    obj = json_new_null ();
  else
    {
      obj = new_typed_object ("backend");

      if (obj)
	{
	  if (json_object_set (obj, "type",
			       json_new_string (backend_type_str (be->be_type)))
	      || json_object_set (obj, "priority",
				  json_new_integer (be->priority))
	      || json_object_set (obj, "alive", json_new_bool (backend_is_alive (be)))
	      || json_object_set (obj, "enabled", json_new_bool (!be->disabled)))
	    {
	      err = 1;
	    }
	  else
	    {
	      if (be->balancer)
		err = json_object_set (obj, "weight",
				       json_new_integer (be->balancer->weight));
	      if (err == 0)
		switch (be->be_type)
		  {
		  case BE_MATRIX:
		    err = json_object_set (obj, "hostname",
					   json_new_string (be->v.mtx.hostname))
		      || json_object_set (obj, "resolve_mode",
					  json_new_string (resolve_mode_str (be->v.mtx.resolve_mode)))
		      || json_object_set (obj, "family",
					  json_new_integer (be->v.mtx.family))
		      || json_object_set (obj, "servername",
					  be->v.mtx.servername
					  ? json_new_string (be->v.mtx.servername)
					  : json_new_null ())
		      || backend_serialize_dyninfo (obj, be);
		    break;

		  case BE_REGULAR:
		    err = json_object_set (obj, "address", addrinfo_serialize (&be->v.reg.addr))
		      || json_object_set (obj, "io_to", json_new_integer (be->v.reg.to))
		      || json_object_set (obj, "conn_to", json_new_integer (be->v.reg.conn_to))
		      || json_object_set (obj, "ws_to", json_new_integer (be->v.reg.ws_to))
		      || json_object_set (obj, "protocol", json_new_string (backend_is_https (be) ? "https" : "http"))
		      || json_object_set (obj, "servername",
					  be->v.reg.servername
					  ? json_new_string (be->v.reg.servername)
					  : json_new_null ())
		      || backend_serialize_dyninfo (obj, be);
		    break;

		  case BE_REDIRECT:
		    err = json_object_set (obj, "url", json_new_string (be->v.redirect.url))
		      || json_object_set (obj, "code", json_new_integer (be->v.redirect.status))
		      || json_object_set (obj, "has_uri", json_new_bool (be->v.redirect.has_uri));
		    break;

		  case BE_ACME:
		  case BE_CONTROL:
		  case BE_METRICS:
		  case BE_FILE:
		    /* FIXME */
		    break;

		  case BE_BACKEND_REF:
		    /* Can't happen */
		    break;

		  case BE_ERROR:
		    err = json_object_set (obj, "status",
					   json_new_integer (pound_to_http_status (be->v.error.status)))
		      || json_object_set (obj, "text",
					  be->v.error.msg.text
					   ? json_new_string (be->v.error.msg.text)
					   : json_new_null ());
		    break;

		  case BE_LUA:
		    err = json_object_set (obj, "function",
					   json_new_string (be->v.lua.func));
		    break;
		  }
	      if (enable_backend_stats && be->be_type != BE_MATRIX)
		err |= json_object_set (obj, "stats", backend_stats_serialize (be));
	    }
	}
    }
  if (err)
    {
      json_value_free (obj);
      obj = NULL;
    }
  return obj;
}

static struct json_value *
backends_serialize (BALANCER_LIST *meta)
{
  struct json_value *obj;

  if (DLIST_EMPTY (meta))
    obj = json_new_null ();
  else
    {
      BALANCER *balancer;
      obj = json_new_array ();
      DLIST_FOREACH (balancer, meta, link)
	{
	  BACKEND *be;

	  DLIST_FOREACH (be, &balancer->backends, link)
	    {
	      if (json_array_append (obj, backend_serialize (be)))
		{
		  json_value_free (obj);
		  obj = NULL;
		  break;
		}
	    }
	}
    }
  return obj;
}

static struct json_value *
service_serialize (SERVICE *svc)
{
  struct json_value *obj = new_typed_object ("service");
  char const *typename;

  if (obj)
    {
      pthread_mutex_lock (&svc->mut);

      typename = sess_type_to_str (svc->sess_type);
      if (json_object_set (obj, "name",
			   svc->name ? json_new_string (svc->name) : json_new_null ())
	  || json_object_set (obj, "enabled", json_new_bool (!svc->disabled))
	  || json_object_set (obj, "session_type", json_new_string (typename ? typename : "UNKNOWN"))
	  || json_object_set (obj, "sessions", service_session_serialize (svc))
	  || json_object_set (obj, "backends", backends_serialize (&svc->balancers)))
	{
	  json_value_free (obj);
	  obj = NULL;
	}
      pthread_mutex_unlock (&svc->mut);
    }

  return obj;
}

static struct json_value *
listener_serialize (LISTENER *lstn)
{
  struct json_value *obj = new_typed_object ("listener"), *p;
  int is_https;
  SERVICE *svc;

  if (obj)
    {
      int err = 0;

      err |= json_object_set (obj, "name",
			      lstn->name ? json_new_string (lstn->name) : json_new_null ());
      err |= json_object_set (obj, "address", addrinfo_serialize (&lstn->addr));

      is_https = !SLIST_EMPTY (&lstn->ctx_head);
      err |= json_object_set (obj, "protocol",
			      json_new_string (is_https ? "https" : "http"));
      if (is_https)
	err |= json_object_set (obj, "nohttps11", json_new_integer (lstn->noHTTPS11));
      err |= json_object_set (obj, "enabled", json_new_bool (!lstn->disabled));

      if ((p = json_new_array ()) == NULL)
	err = 1;
      else
	{
	  if (json_object_set (obj, "services", p))
	    {
	      json_value_free (p);
	      err = 1;
	    }
	  SLIST_FOREACH (svc, &lstn->services, next)
	    {
	      if ((err = json_array_append (p, service_serialize (svc))) != 0)
		break;
	    }
	}

      if (err)
	{
	  json_value_free (obj);
	  obj = NULL;
	}
    }

  return obj;
}

static struct json_value *
pound_core_serialize (void)
{
  struct json_value *obj;

  if ((obj = new_typed_object ("core")) != NULL)
    {
      int err = 0;
      struct timespec ts, uptime = pound_uptime ();

      clock_gettime (CLOCK_REALTIME, &ts);
      err = json_object_set (obj, "version", json_new_string (PACKAGE_VERSION))
	|| json_object_set (obj, "pid", json_new_integer (getpid ()))
	|| json_object_set (obj, "timestamp", timespec_serialize (&ts))
	|| json_object_set (obj, "uptime", duration_serialize (&uptime))
	|| json_object_set (obj, "queue_len", json_new_integer (get_thr_qlen ()))
	|| json_object_set (obj, "workers", workers_serialize ());
      if (err)
	{
	  json_value_free (obj);
	  obj = NULL;
	}
    }
  return obj;
}

struct json_value *
pound_serialize (void)
{
  struct json_value *obj, *p;
  LISTENER *lstn;
  SERVICE *svc;

  obj = new_typed_object ("pound");

  if (obj)
    {
      int err = 0;

      if (json_object_set (obj, "core", pound_core_serialize ()))
	err = 1;
      else if ((p = json_new_array ()) == NULL)
	err = 1;
      else if (json_object_set (obj, "listeners", p))
	{
	  json_value_free (p);
	  err = 1;
	}
      else
	{
	  SLIST_FOREACH (lstn, &listeners, next)
	    if ((err = json_array_append (p, listener_serialize (lstn))) != 0)
	      break;
	}

      if (err == 0)
	{
	  if ((p = json_new_array ()) == NULL)
	    err = 1;
	  else if (json_object_set (obj, "services", p))
	    {
	      json_value_free (p);
	      err = 1;
	    }
	  else
	    {
	      SLIST_FOREACH (svc, &services, next)
		if ((err = json_array_append (p, service_serialize (svc))) != 0)
		  break;
	    }
	}

      if (err)
	{
	  json_value_free (obj);
	  obj = NULL;
	}
    }

  return obj;
}

static void
write_string (void *data, char const *str, size_t len)
{
  struct stringbuf *sb = data;
  stringbuf_add (sb, str, len);
}

static int
send_json_reply (BIO *c, struct json_value *val, char const *url)
{
  struct json_format format = {
    .indent = 0,
    .precision = 0,
    .write = write_string
  };
  struct stringbuf sb;
  char *str;
  char *indent;
  size_t len;

  if ((indent = get_param (url, "indent", &len)) != NULL)
    {
      long n;
      char *end;

      errno = 0;
      n = strtol (indent, &end, 10);
      if (errno || end != indent + len || n < 0 || n > 80)
	return HTTP_STATUS_BAD_REQUEST;
      format.indent = n;
    }

  stringbuf_init_log (&sb);
  format.data = &sb;
  json_value_format (val, &format, 0);
  if ((str = stringbuf_finish (&sb)) == NULL)
    {
      stringbuf_free (&sb);
      return HTTP_STATUS_INTERNAL_SERVER_ERROR;
    }

  BIO_printf (c,
	      "HTTP/1.1 %d %s\r\n"
	      "Content-Type: application/json\r\n"
	      "Content-Length: %"PRICLEN"\r\n"
	      "Connection: close\r\n\r\n"
	      "%s",
	      200, "OK", (CONTENT_LENGTH) strlen (str), str);
  BIO_flush (c);
  stringbuf_free (&sb);

  return HTTP_STATUS_OK;
}

static inline int
url_is_root (char const *url)
{
  size_t n = strcspn (url, "?");
  return n == 0 || (n == 1 && *url == '/');
}

static int
control_list_core (BIO *c, char const *url)
{
  struct json_value *val;
  int rc;

  if (!url_is_root (url))
    rc = HTTP_STATUS_NOT_FOUND;
  else if ((val = pound_core_serialize ()) == NULL)
    rc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
  else
    {
      rc = send_json_reply (c, val, url);
      json_value_free (val);
    }
  return rc;
}

static int
control_list_all (BIO *c, char const *url)
{
  struct json_value *val;
  int rc;

  if (!url_is_root (url))
    rc = HTTP_STATUS_NOT_FOUND;
  else if ((val = pound_serialize ()) == NULL)
    rc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
  else
    {
      rc = send_json_reply (c, val, url);
      json_value_free (val);
    }
  return rc;
}

/*
 * Type of object identifier.
 */
enum
  {
    IDTYPE_ERR = -1,
    IDTYPE_NUM,         /* Numeric identifier (index). */
    IDTYPE_STR          /* String identifier (name). */
  };

/*
 * Object identifier.
 */
typedef struct
{
  int type;              /* Identifier type. */
  union
  {
    int n;               /* Index value (IDTYPE_NUM). */
    struct               /* String value (IDTYPE_STR). */
    {
      int len;           /* Name length. */
      char const *name;  /* Pointer to the name. */
    } s;
  };
} IDENT;

/*
 * Get single identifier from the URL (must point to a /, on entry).
 * On success, advance URL to the point past the end of the identifier
 * (normally next / or end of URL).
 * On error, return ident with its type set to IDTYPE_ERR.
 */
static IDENT
ctl_getident (char const **url)
{
  IDENT retval = { IDTYPE_ERR };
  long n;
  char *end;

  if (**url == '/')
    {
      ++*url;
      errno = 0;
      n = strtol (*url, &end, 10);
      if (*end != 0 && *end != '/' && *end != '?')
	{
	  end = (char*) *url + strcspn (*url, "/?");
	  retval.s.name = *url;
	  retval.s.len = end - *url;
	  retval.type = IDTYPE_STR;
	  *url = end;
	}
      else if (end > *url && errno == 0 && n <= INT_MAX)
	{
	  retval.type = IDTYPE_NUM;
	  retval.n = n;
	  *url = end;
	}
    }
  return retval;
}

#define LOCATE(obj, head, id)						\
  do									\
    {									\
      long __n = 0;							\
      SLIST_FOREACH (obj, head, next)					\
	{								\
	  switch (id.type)						\
	    {								\
	    case IDTYPE_NUM:						\
	      if (__n == id.n)						\
		return obj;						\
	      break;							\
									\
	    case IDTYPE_STR:						\
	      if (obj->name && strlen (obj->name) == id.s.len &&	\
		  memcmp (obj->name, id.s.name, id.s.len) == 0)		\
		return obj;						\
	      break;							\
	    }								\
	  __n++;							\
	}								\
      }									\
    while (0)


/*
 * Locate listener identified by ID.
 */
static LISTENER *
locate_listener (IDENT id)
{
  if (id.type != IDTYPE_ERR)
    {
      LISTENER *lstn;
      LOCATE (lstn, &listeners, id);
    }
  return NULL;
}

/*
 * Locate service identified by ID.  If LSTN is null, look it up in the
 * global service list.
 */
static SERVICE *
locate_service (LISTENER *lstn, IDENT id)
{
  if (id.type != IDTYPE_ERR)
    {
      SERVICE_HEAD *head = lstn ? &lstn->services : &services;
      SERVICE *svc;
      LOCATE (svc, head, id);
    }
  return NULL;
}

/*
 * Locate backend identified by ID.  Only IDTYPE_NUM is supported.
 */
static BACKEND *
locate_backend (SERVICE *svc, IDENT id)
{
  if (id.type == IDTYPE_NUM)
    {
      long n = 0;
      BALANCER *balancer;
      DLIST_FOREACH (balancer, &svc->balancers, link)
	{
	  BACKEND *be;
	  DLIST_FOREACH (be, &balancer->backends, link)
	    {
	      if (n == id.n)
		return be;
	      n++;
	    }
	}
    }
  return NULL;
}

enum object_type
  {
    OBJ_LISTENER,
    OBJ_SERVICE,
    OBJ_BACKEND,
  };

typedef struct
{
  enum object_type type;
  union
  {
    LISTENER *lstn;
    SERVICE *svc;
    BACKEND *be;
  };
} OBJECT;

typedef int (*OBJHANDLER) (BIO *, OBJECT *, char const *, void *);

static int
ctl_backend (OBJHANDLER func, void *data, BIO *c, char const *url, SERVICE *svc)
{
  OBJECT obj = { .type = OBJ_BACKEND };

  if ((obj.be = locate_backend (svc, ctl_getident (&url))) == NULL)
    return HTTP_STATUS_NOT_FOUND;
  return func (c, &obj, url, data);
}

static int
ctl_service (OBJHANDLER func, void *data, BIO *c, char const *url, LISTENER *lstn)
{
  OBJECT obj = { .type = OBJ_SERVICE };

  if ((obj.svc = locate_service (lstn, ctl_getident (&url))) == NULL)
    return HTTP_STATUS_NOT_FOUND;

  if (*url && *url != '?')
    return ctl_backend (func, data, c, url, obj.svc);
  return func (c, &obj, url, data);
}

static int
ctl_listener (OBJHANDLER func, void *data, BIO *c, char const *url)
{
  OBJECT obj = { .type = OBJ_LISTENER };
  size_t len = strcspn (url, "?");

  if (len == 0)
    obj.lstn = NULL;
  else if (url[0] == '/' && len == 1)
    {
      obj.lstn = NULL;
      url++;
    }
  else if (url[0] == '/' && url[1] == '-' && (len == 2 || url[2] == '/'))
    {
      obj.lstn = NULL;
      url += 2;
    }
  else if ((obj.lstn = locate_listener (ctl_getident (&url))) == NULL)
    return HTTP_STATUS_NOT_FOUND;

  if (*url && *url != '?')
    return ctl_service (func, data, c, url, obj.lstn);

  return func (c, &obj, url, data);
}

static int
list_handler (BIO *c, OBJECT *obj, char const *url, void *data)
{
  struct json_value *val;
  int rc;

  if (*url && *url != '?')
    return HTTP_STATUS_NOT_FOUND;

  switch (obj->type)
    {
    case OBJ_BACKEND:
      /*
       * backend_serialize needs to access the backend disabled field.
       * To do it safely, the holdiing service must be locked.  See the
       * NOTE to cb_backend_disable, above.
       */
      pthread_mutex_lock (&obj->be->service->mut);
      val = backend_serialize (obj->be);
      pthread_mutex_unlock (&obj->be->service->mut);
      break;

    case OBJ_SERVICE:
      val = service_serialize (obj->svc);
      break;

    case OBJ_LISTENER:
      if (obj->lstn)
	val = listener_serialize (obj->lstn);
      else
	val = pound_serialize ();
      break;
    }

  if (!val)
    rc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
  else
    {
      rc = send_json_reply (c, val, url);
      json_value_free (val);
    }
  return rc;
}

static int
control_list_listener (BIO *c, char const *url)
{
  return ctl_listener (list_handler, NULL, c, url);
}

static int
control_list_service (BIO *c, char const *url)
{
  return ctl_service (list_handler, NULL, c, url, NULL);
}

static int
service_has_control (SERVICE *svc)
{
  BALANCER *balancer;
  DLIST_FOREACH (balancer, &svc->balancers, link)
    {
      BACKEND *be;
      DLIST_FOREACH (be, &balancer->backends, link)
	{
	  if (be->be_type == BE_CONTROL)
	    return 1;
	}
    }
  return 0;
}

static int
listener_has_control (LISTENER *lstn)
{
  SERVICE *svc;
  SLIST_FOREACH (svc, &lstn->services, next)
    if (service_has_control (svc))
      return 1;
  return 0;
}

static int
disable_handler (BIO *c, OBJECT *obj, char const *url, void *data)
{
  int *dis = data;
  int rc;
  struct json_value *val;

  if (*url && *url != '?')
    return HTTP_STATUS_NOT_FOUND;

  switch (obj->type)
    {
    case OBJ_BACKEND:
      if (obj->be->be_type == BE_CONTROL)
	return HTTP_STATUS_BAD_REQUEST;
      backend_disable (obj->be->service, obj->be, *dis ? BE_DISABLE : BE_ENABLE);
      break;

    case OBJ_SERVICE:
      if (service_has_control (obj->svc))
	return HTTP_STATUS_BAD_REQUEST;
      obj->svc->disabled = *dis;
      break;

    case OBJ_LISTENER:
      if (listener_has_control (obj->lstn))
	return HTTP_STATUS_BAD_REQUEST;
      obj->lstn->disabled = *dis;
      break;
    }

  if ((val = json_new_bool (1)) == NULL)
    rc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
  else
    {
      rc = send_json_reply (c, val, url);
      json_value_free (val);
    }
  return rc;
}

static int
control_disable_listener (BIO *c, char const *url)
{
  int dis = 1;
  if (*url == '/')
    return ctl_listener (disable_handler, &dis, c, url);
  return HTTP_STATUS_NOT_FOUND;
}

static int
control_enable_listener (BIO *c, char const *url)
{
  int dis = 0;
  if (*url == '/')
    return ctl_listener (disable_handler, &dis, c, url);
  return HTTP_STATUS_NOT_FOUND;
}

static int
control_disable_service (BIO *c, char const *url)
{
  int dis = 1;
  if (*url == '/')
    return ctl_service (disable_handler, &dis, c, url, NULL);
  return HTTP_STATUS_NOT_FOUND;
}

static int
control_enable_service (BIO *c, char const *url)
{
  int dis = 0;
  if (*url == '/')
    return ctl_service (disable_handler, &dis, c, url, NULL);
  return HTTP_STATUS_NOT_FOUND;
}

static int
session_remove_handler (BIO *c, OBJECT *obj, char const *url, void *data)
{
  SERVICE *svc;
  int rc;
  struct json_value *val;
  char keybuf[KEY_SIZE + 1];
  char *key;
  size_t keylen;

  switch (obj->type)
    {
    case OBJ_BACKEND:
    case OBJ_LISTENER:
      return HTTP_STATUS_BAD_REQUEST;

    case OBJ_SERVICE:
      svc = obj->svc;
      break;
    }

  if ((key = get_param (url, "key", &keylen)) == NULL)
    return HTTP_STATUS_BAD_REQUEST;
  if (keylen > sizeof (keybuf) - 1)
    return HTTP_STATUS_BAD_REQUEST;
  strncpy (keybuf, key, keylen);
  keybuf[keylen] = 0;

  pthread_mutex_lock (&svc->mut);
  service_session_remove_by_key (svc, keybuf);
  pthread_mutex_unlock (&svc->mut);

  if ((val = service_serialize (svc)) != NULL)
    {
      rc = send_json_reply (c, val, url);
      json_value_free (val);
    }
  else
    rc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
  return rc;
}

static int
session_add_handler (BIO *c, OBJECT *obj, char const *url, void *data)
{
  BACKEND *be;
  SERVICE *svc;
  int rc;
  struct json_value *val;
  char keybuf[KEY_SIZE + 1];
  char *key;
  size_t keylen;

  switch (obj->type)
    {
    case OBJ_LISTENER:
    case OBJ_SERVICE:
      return HTTP_STATUS_BAD_REQUEST;

    case OBJ_BACKEND:
      be = obj->be;
      svc = be->service;
      break;
    }

  if ((key = get_param (url, "key", &keylen)) == NULL)
    return HTTP_STATUS_BAD_REQUEST;
  if (keylen > sizeof (keybuf) - 1)
    return HTTP_STATUS_BAD_REQUEST;
  strncpy (keybuf, key, keylen);
  keybuf[keylen] = 0;

  pthread_mutex_lock (&svc->mut);
  service_session_add (svc, keybuf, be);
  pthread_mutex_unlock (&svc->mut);

  if ((val = service_serialize (svc)) != NULL)
    {
      rc = send_json_reply (c, val, url);
      json_value_free (val);
    }
  else
    rc = HTTP_STATUS_INTERNAL_SERVER_ERROR;
  return rc;
}

static int
control_delete_session (BIO *c, char const *url)
{
  if (*url == '/')
    return ctl_listener (session_remove_handler, NULL, c, url);
  return HTTP_STATUS_NOT_FOUND;
}

static int
control_add_session (BIO *c, char const *url)
{
  if (*url == '/')
    return ctl_listener (session_add_handler, NULL, c, url);
  return HTTP_STATUS_NOT_FOUND;
}

struct endpoint
{
  char *uri;
  size_t uri_len;
  int method;
  int (*endfn) (BIO *, char const *);
};

static struct endpoint control_endpoint[] = {
#define S(s) s, sizeof(s)-1
  { S(""), METH_GET, control_list_all },
  { S("/core"), METH_GET, control_list_core },
  { S("/listener"), METH_GET, control_list_listener },
  { S("/listener"), METH_DELETE, control_disable_listener },
  { S("/listener"), METH_PUT, control_enable_listener },
  { S("/service"), METH_GET, control_list_service },
  { S("/service"), METH_DELETE, control_disable_service },
  { S("/service"), METH_PUT, control_enable_service },
  { S("/session"), METH_DELETE, control_delete_session },
  { S("/session"), METH_PUT, control_add_session },
#undef S
  { NULL }
};

static struct endpoint *
find_endpoint (int method, const char *uri, int *errcode)
{
  struct endpoint *p, *cp = NULL;
  int uri_match = 0;
  size_t len = strcspn (uri, "?");

  if (len == 0)
    return NULL;

  if (uri[len-1] == '/')
    --len;

  for (p = control_endpoint; p->uri; p++)
    if (len >= p->uri_len && memcmp (p->uri, uri, p->uri_len) == 0
	&& (uri[p->uri_len] == 0
	    || uri[p->uri_len] == '/'
	    || uri[p->uri_len] == '?'))
      {
	if (p->method == method)
	  {
	    if (cp == NULL || cp->uri_len < p->uri_len)
	      cp = p;
	  }
	else
	  uri_match = 1;
      }

  if (cp == NULL)
    *errcode = uri_match ? HTTP_STATUS_METHOD_NOT_ALLOWED
			 : HTTP_STATUS_NOT_FOUND;
  return cp;
}

int
control_response_basic (POUND_HTTP *arg)
{
  struct endpoint *ep;
  int code;

  ep = find_endpoint (arg->request.method, arg->request.url, &code);
  if (ep != NULL)
    code = ep->endfn (arg->cl, arg->request.url + ep->uri_len);
  arg->response_code = 0;
  return code;
}

#ifndef SSL3_ST_SR_CLNT_HELLO_A
# define SSL3_ST_SR_CLNT_HELLO_A (0x110|SSL_ST_ACCEPT)
#endif
#ifndef SSL23_ST_SR_CLNT_HELLO_A
# define SSL23_ST_SR_CLNT_HELLO_A (0x210|SSL_ST_ACCEPT)
#endif

void
SSLINFO_callback (const SSL * ssl, int where, int rc)
{
  RENEG_STATE *reneg_state;

  /* Get our thr_arg where we're tracking this connection info */
  if ((reneg_state = (RENEG_STATE *) SSL_get_app_data (ssl)) == NULL)
    return;

  /*
   * If we're rejecting renegotiations, move to ABORT if Client Hello
   * is being read.
   */
  if ((where & SSL_CB_ACCEPT_LOOP) && *reneg_state == RENEG_REJECT)
    {
      int state;

      state = SSL_get_state (ssl);
      if (state == SSL3_ST_SR_CLNT_HELLO_A
	  || state == SSL23_ST_SR_CLNT_HELLO_A)
	{
	  *reneg_state = RENEG_ABORT;
	  logmsg (LOG_WARNING, "rejecting client initiated renegotiation");
	}
    }
  else if (where & SSL_CB_HANDSHAKE_DONE && *reneg_state == RENEG_INIT)
    {
      // Reject any followup renegotiations
      *reneg_state = RENEG_REJECT;
    }
}

int
foreach_listener (LISTENER_ITERATOR itr, void *data)
{
  LISTENER *lstn;
  int rc = 0;
  SLIST_FOREACH (lstn, &listeners, next)
    {
      if ((rc = itr (lstn, data)) != 0)
	break;
    }
  return rc;
}

int
foreach_service (SERVICE_ITERATOR itr, void *data)
{
  LISTENER *lstn;
  SERVICE *svc;

  SLIST_FOREACH (lstn, &listeners, next)
    {
      SLIST_FOREACH (svc, &lstn->services, next)
	{
	  int rc;
	  if ((rc = itr (svc, data)) != 0)
	    return rc;
	}
    }

  SLIST_FOREACH (svc, &services, next)
    {
      int rc;
      if ((rc = itr (svc, data)) != 0)
	return rc;
    }

  return 0;
}

struct service_backends_iterator
{
  BACKEND_ITERATOR itr;
  void *data;
};

static int
itr_service_backends (SERVICE *svc, void *data)
{
  struct service_backends_iterator *itp = data;
  BALANCER *balancer;
  DLIST_FOREACH (balancer, &svc->balancers, link)
    {
      BACKEND *be;
      DLIST_FOREACH (be, &balancer->backends, link)
	{
	  int rc;
	  if ((rc = itp->itr (be, itp->data)) != 0)
	    return rc;
	}
    }
  return 0;
}

int
foreach_backend (BACKEND_ITERATOR itr, void *data)
{
  struct service_backends_iterator bitr;

  bitr.itr = itr;
  bitr.data = data;
  return foreach_service (itr_service_backends, &bitr);
}

BALANCER *
balancer_list_get (BALANCER_LIST *ml, int weight, BALANCER_ALGO algo)
{
  BALANCER *bl, *new_bl;
  DLIST_FOREACH (bl, ml, link)
    {
      if (weight == bl->weight)
	return bl;
      if (weight < bl->weight)
	break;
    }

  if ((new_bl = calloc (1, sizeof (*new_bl))) == NULL)
    return NULL;
  new_bl->weight = weight;
  new_bl->algo = algo;
  DLIST_INIT (&new_bl->backends);

  if (bl)
    DLIST_INSERT_BEFORE (ml, bl, new_bl, link);
  else
    DLIST_INSERT_TAIL (ml, new_bl, link);

  return new_bl;
}

void
balancer_list_remove (BALANCER_LIST *ml, BALANCER *bl)
{
  DLIST_REMOVE (ml, bl, link);
}
