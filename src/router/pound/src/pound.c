/*
 * Pound - the reverse-proxy load-balancer
 * Copyright (C) 2002-2010 Apsis GmbH
 * Copyright (C) 2018-2024 Sergey Poznyakoff
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
 * along with pound.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "pound.h"
#include "json.h"
#include "extern.h"

/* common variables */
char *user;			/* user to run as */
char *group;			/* group to run as */
char *root_jail;		/* directory to chroot to */
char *pid_name = POUND_PID;     /* file to record pid in */

int anonymise;			/* anonymise client address */
int daemonize = 1;		/* run as daemon */
int enable_supervisor = 1;      /* enable supervisor process */
int log_facility = -1;		/* log facility to use */
int print_log;                  /* print log messages to stdout/stderr during startup */
int enable_backend_stats;

unsigned alive_to = DEFAULT_ALIVE_TO; /* check interval for resurrection */
unsigned grace = DEFAULT_GRACE_TO;    /* grace period before shutdown */

SERVICE_HEAD services = SLIST_HEAD_INITIALIZER (services);
				/* global services (if any) */
LISTENER_HEAD listeners = SLIST_HEAD_INITIALIZER (listeners);
				/* all available listeners */
int n_listeners;                /* Number of listeners */

regex_t HEADER,			/* Allowed header */
  CONN_UPGRD,			/* upgrade in connection header */
  LOCATION;			/* the host we are redirected to */

char *forwarded_header;         /* "forwarded" header name */
ACL *trusted_ips;               /* Trusted IP addresses */

#ifndef  SOL_TCP
/* for systems without the definition */
int SOL_TCP;
#endif

/*
 * Error reporting.
 */

/* Serialize access to the log stream */
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Log an error to the syslog or to stderr
 */
void
vlogmsg (const int priority, const char *fmt, va_list ap)
{
  if (log_facility == -1 || print_log)
    {
      va_list aq;
      FILE *fp = (priority == LOG_INFO || priority == LOG_DEBUG)
		     ? stdout : stderr;
      pthread_mutex_lock (&log_mutex);
      if (progname)
	fprintf (fp, "%s: ", progname);
      va_copy (aq, ap);
      vfprintf (fp, fmt, aq);
      va_end (aq);
      fputc ('\n', fp);
      pthread_mutex_unlock (&log_mutex);
    }

  if (log_facility != -1)
    {
      struct stringbuf sb;
      xstringbuf_init (&sb);
      stringbuf_vprintf (&sb, fmt, ap);
      syslog (priority, "%s", stringbuf_value (&sb));
      stringbuf_free (&sb);
    }
  return;
}

void
logmsg (const int priority, const char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  vlogmsg (priority, fmt, ap);
  va_end (ap);
}

/*
 * This is used as exit point if memory allocation failures occur at program
 * startup (e.g. when parsing config or the like).
 */
void
xnomem (void)
{
  logmsg (LOG_CRIT, "out of memory");
  exit (1);
}

/*
 * This is used as json_memabrt hook.  The code in svc.c handles memory
 * allocation failures gracefully and returns 500 if any occurs.
 */
void
lognomem (void)
{
  logmsg (LOG_CRIT, "out of memory");
}

/*
 * Log a message at LOG_CRIT and terminate the program.
 */
void
abend (char const *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  vlogmsg (LOG_CRIT, fmt, ap);
  va_end (ap);
  logmsg (LOG_NOTICE, "pound terminated");
  _exit (1);
}

/*
 * OpenSSL thread support stuff
 */
#if OPENSSL_VERSION_NUMBER >= 0x10100000L
#define l_init()
#else
static pthread_mutex_t *l_array;

static void
l_init (void)
{
  int i, n_locks;

  n_locks = CRYPTO_num_locks ();
  l_array = xcalloc (n_locks, sizeof (pthread_mutex_t));

  for (i = 0; i < n_locks; i++)
    /* pthread_mutex_init() always returns 0 */
    pthread_mutex_init (&l_array[i], NULL);
  return;
}

static void
l_lock (const int mode, const int n, /* unused */ const char *file,
	/* unused */ int line)
{
  int ret_val;

  if (mode & CRYPTO_LOCK)
    {
      if ((ret_val = pthread_mutex_lock (&l_array[n])) != 0)
	logmsg (LOG_ERR, "l_lock lock(): %s", strerror (ret_val));
    }
  else
    {
      if ((ret_val = pthread_mutex_unlock (&l_array[n])) != 0)
	logmsg (LOG_ERR, "l_lock unlock(): %s", strerror (ret_val));
    }
  return;
}

static unsigned long
l_id (void)
{
  return (unsigned long) pthread_self ();
}
#endif

/*
 * work queue stuff
 */
static POUND_HTTP_HEAD thr_head = SLIST_HEAD_INITIALIZER (thr_head);
static pthread_cond_t arg_cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t active_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t arg_mut = PTHREAD_MUTEX_INITIALIZER;

static unsigned active_threads; /* Number of threads currently active,
				   i.e processing requests. */

static unsigned worker_count = 0;
static pthread_attr_t attr;

unsigned worker_min_count = DEFAULT_WORKER_MIN;
unsigned worker_max_count = DEFAULT_WORKER_MAX;
unsigned worker_idle_timeout = DEFAULT_WORKER_IDLE_TIMEOUT;

struct json_value *
workers_serialize (void)
{
  struct json_value *obj;
  int err = 0;

  obj = json_new_object ();
  if (obj)
    {
      err = json_object_set (obj, "min", json_new_number (worker_min_count))
	|| json_object_set (obj, "max", json_new_number (worker_max_count))
	|| json_object_set (obj, "timeout", json_new_number (worker_idle_timeout));
      if (err == 0)
	{
	  pthread_mutex_lock (&arg_mut);
	  err = json_object_set (obj, "count", json_new_number (worker_count))
	    || json_object_set (obj, "active", json_new_number (active_threads));
	  pthread_mutex_unlock (&arg_mut);
	}
    }
  if (err)
    {
      json_value_free (obj);
      obj = NULL;
    }
  return obj;
}

static void
worker_start (void)
{
  pthread_t thr;
  int rc;

  if ((rc = pthread_create (&thr, &attr, thr_http, NULL)) != 0)
    abend ("can't create worker thread: %s", strerror (rc));
  worker_count++;
}

/*
 * add a request to the queue
 */
int
pound_http_enqueue (int sock, LISTENER *lstn, struct sockaddr *sa, socklen_t salen)
{
  POUND_HTTP *res;

  if ((res = calloc (1, sizeof (res[0]))) == NULL)
    {
      lognomem ();
      return -1;
    }

  if ((res->from_host.ai_addr = malloc (salen)) == NULL)
    {
      lognomem ();
      free (res);
      return -1;
    }

  res->sock = sock;
  res->lstn = lstn;

  memcpy (res->from_host.ai_addr, sa, salen);
  res->from_host.ai_family = sa->sa_family;
  res->from_host.ai_addrlen = salen;

  http_request_init (&res->request);
  http_request_init (&res->response);
  /*
   * Note: submatch_queue_init is not called, because res is already
   * filled with zeros.  Revise this if submatch_queue stuff changes.
   */

  pthread_mutex_lock (&arg_mut);
  SLIST_PUSH (&thr_head, res, next);
  if (worker_count < worker_max_count && worker_count == active_threads)
    {
      worker_start ();
    }
  pthread_cond_signal (&arg_cond);
  pthread_mutex_unlock (&arg_mut);
  return 0;
}

/*
 * get a request from the queue
 */
POUND_HTTP *
pound_http_dequeue (void)
{
  POUND_HTTP *res;
  struct timespec ts;

  pthread_mutex_lock (&arg_mut);

  clock_gettime (CLOCK_REALTIME, &ts);
  ts.tv_sec += worker_idle_timeout;

  /*
   * Wait until something becomes available in the queue.  Spurious wakeups
   * from pthread_cond_wait can occur, hence the need for a loop.
   */
  while (SLIST_EMPTY (&thr_head))
    {
      int rc;

      if (worker_count == worker_min_count)
	pthread_cond_wait (&arg_cond, &arg_mut);
      else if ((rc = pthread_cond_timedwait (&arg_cond, &arg_mut, &ts)) != 0)
	{
	  if (rc == ETIMEDOUT)
	    {
	      /*
	       * worker_count might have changed while we were waiting,
	       * so check again if the minimal worker count is reached.
	       */
	      if (worker_count == worker_min_count)
		continue;
	      /*
	       * If there are more workers than the predefined minimum,
	       * decrease worker_count and return NULL.  The calling thread
	       * will then terminate.
	       */
	      worker_count--;
	      res = NULL;
	      goto end;
	    }
	  else
	    {
	      logmsg (LOG_ERR, "pthread_cond_timedwait: %s", strerror (rc));
	      exit (1);
	    }
	}
    }

  /* Dequeue the head element */
  res = SLIST_FIRST (&thr_head);
  SLIST_SHIFT (&thr_head, next);
  if (!SLIST_EMPTY (&thr_head))
    /*
     * If there's still more in the queue, signal other threads, so they
     * can have their share.
     * Notice, that pthread_cond_signal() may be called whether or not the
     * current thread owns the mutex associated with the condition.  However,
     * if predictable scheduling behavior is required, the mutex should be
     * locked.
     */
    pthread_cond_signal (&arg_cond);

  active_threads++;
 end:
  pthread_mutex_unlock (&arg_mut);
  return res;
}

void
pound_http_destroy (POUND_HTTP *arg)
{
  free (arg->from_host.ai_addr);

  free (arg->orig_forwarded_header);
  http_request_free (&arg->request);
  http_request_free (&arg->response);

  if (arg->ssl != NULL)
    {
      SSL_set_shutdown (arg->ssl, SSL_SENT_SHUTDOWN | SSL_RECEIVED_SHUTDOWN);
      BIO_ssl_shutdown (arg->cl);
    }

  if (arg->be != NULL)
    {
      BIO_flush (arg->be);
      BIO_reset (arg->be);
      BIO_free_all (arg->be);
    }

  if (arg->cl != NULL)
    {
      BIO_flush (arg->cl);
      BIO_reset (arg->cl);
      BIO_free_all (arg->cl);
    }

  if (arg->x509 != NULL)
    {
      X509_free (arg->x509);
    }

  submatch_queue_free (&arg->smq);

  free (arg);
}

/*
 * get the current queue length
 */
int
get_thr_qlen (void)
{
  int res = 0;
  POUND_HTTP *tap;

  pthread_mutex_lock (&arg_mut);
  SLIST_FOREACH (tap, &thr_head, next)
    res++;
  pthread_mutex_unlock (&arg_mut);
  return res;
}

void
active_threads_decr (void)
{
  pthread_mutex_lock (&arg_mut);
  active_threads--;
  if (active_threads == 0)
    pthread_cond_broadcast (&active_cond);
  pthread_mutex_unlock (&arg_mut);
}

void
active_threads_wait (void)
{
  struct timespec ts;

  pthread_mutex_lock (&arg_mut);
  if (active_threads > 0)
    {
      logmsg (LOG_NOTICE, "waiting for %u active threads to terminate",
	      active_threads);
      clock_gettime (CLOCK_REALTIME, &ts);
      ts.tv_sec += grace;
      while (active_threads > 0 &&
	     pthread_cond_timedwait (&active_cond, &arg_mut, &ts) == 0)
	;
    }
  pthread_mutex_unlock (&arg_mut);
}

static void
listener_cleanup (void *ptr)
{
  LISTENER *lstn;
  SLIST_FOREACH (lstn, &listeners, next)
    close (lstn->sock);
}

void *
thr_dispatch (void *unused)
{
  int i;
  LISTENER *lstn;
  struct pollfd *polls;

  /* alloc the poll structures */
  polls = xcalloc (n_listeners, sizeof (struct pollfd));

  i = 0;
  SLIST_FOREACH (lstn, &listeners, next)
    polls[i++].fd = lstn->sock;

  pthread_cleanup_push (listener_cleanup, NULL);
  for (;;)
    {
      i = 0;
      SLIST_FOREACH (lstn, &listeners, next)
	{
	  polls[i].events = POLLIN | POLLPRI;
	  polls[i].revents = 0;
	  i++;
	}

      if (poll (polls, n_listeners, -1) < 0)
	{
	  logmsg (LOG_WARNING, "poll: %s", strerror (errno));
	}
      else
	{
	  i = 0;
	  SLIST_FOREACH (lstn, &listeners, next)
	    {
	      if (polls[i].revents & (POLLIN | POLLPRI))
		{
		  struct sockaddr_storage clnt_addr;
		  socklen_t clnt_length;
		  int clnt;

		  memset (&clnt_addr, 0, sizeof (clnt_addr));
		  clnt_length = sizeof (clnt_addr);
		  if ((clnt = accept (lstn->sock,
				      (struct sockaddr *) &clnt_addr,
				      &clnt_length)) < 0)
		    {
		      logmsg (LOG_WARNING, "HTTP accept: %s",
			      strerror (errno));
		    }
		  else
		    {
		      if (lstn->disabled)
			{
			  close (clnt);
			  continue;
			}

		      if (pound_http_enqueue (clnt, lstn,
					   (struct sockaddr *) &clnt_addr,
					   clnt_length))
			close (clnt);
		    }
		}
	      i++;
	    }
	}
    }
  pthread_cleanup_pop (1);
}

struct file_location
{
  int fd;            /* Directory descriptor. */
  char *basename;    /* Name relative to fd. */
  char pathname[1];  /* Full pathname (for error reporting). */
};

static void
unlink_file_location (void *arg)
{
  struct file_location *fil = arg;
  if (unlinkat (fil->fd, fil->basename, 0))
    logmsg (LOG_NOTICE, "can't remove file %s: %s", fil->pathname,
	    strerror (errno));
  close (fil->fd);
  free (fil);
}

static void
unlink_file (void *arg)
{
  char *file_name = arg;
  if (unlink (file_name))
    logmsg (LOG_NOTICE, "can't remove file %s: %s", file_name, strerror (errno));
  free (file_name);
}

int
unlink_at_exit (char const *file_name)
{
  if (root_jail)
    {
      struct file_location *fl;
      char const *p;
      size_t dirlen;
      int fd;

      if ((p = strrchr (file_name, '/')) == NULL)
	{
	  dirlen = 0;
	  fd = open (".", O_RDONLY | O_NONBLOCK | O_DIRECTORY);
	  if (fd == -1)
	    {
	      logmsg (LOG_NOTICE, "can't open CWD: %s", strerror (errno));
	      return -1;
	    }
	}
      else
	{
	  char *dir;

	  dirlen = p - file_name;
	  dir = xmalloc (dirlen + 1);
	  memcpy (dir, file_name, dirlen);
	  dir[dirlen] = 0;
	  fd = open (dir, O_RDONLY | O_NONBLOCK | O_DIRECTORY);
	  if (fd == -1)
	    {
	      logmsg (LOG_NOTICE, "can't open directory %s: %s",
		      dir, strerror (errno));
	      free (dir);
	      return -1;
	    }
	  free (dir);
	  dirlen++;
	}

      fl = xmalloc (sizeof (*fl) + strlen (file_name));
      fl->fd = fd;
      strcpy (fl->pathname, file_name);
      fl->basename = fl->pathname + dirlen;

      pound_atexit (unlink_file_location, fl);
    }
  else
    pound_atexit (unlink_file, xstrdup (file_name));
  return 0;
}

static void
pidfile_create (void)
{
  FILE *fp;

  if (!pid_name)
    return;
  if ((fp = fopen (pid_name, "wt")) != NULL)
    {
      fprintf (fp, "%d\n", getpid ());
      fclose (fp);
      unlink_at_exit (pid_name);
    }
  else
    logmsg (LOG_NOTICE, "Create \"%s\": %s", pid_name, strerror (errno));
}

/*
 * Clean-up function queue.
 */
struct cleanup_routine
{
  void (*func) (void *);
  void *arg;
  SLIST_ENTRY (cleanup_routine) next;
};

static SLIST_HEAD (,cleanup_routine) cleanup_head = SLIST_HEAD_INITIALIZER (cleanup_head);

void
pound_atexit (void (*func) (void *), void *arg)
{
  struct cleanup_routine *cr;
  XZALLOC (cr);
  cr->func = func;
  cr->arg = arg;
  SLIST_PUSH (&cleanup_head, cr, next);
}

static void
cleanup (void)
{
  struct cleanup_routine *cr;
  while (!SLIST_EMPTY (&cleanup_head))
    {
      cr = SLIST_FIRST (&cleanup_head);
      SLIST_SHIFT (&cleanup_head, next);
      cr->func (cr->arg);
      free (cr);
    }
}

struct signal_handler
{
  int signo;
  void (*handler) (int);
};

static void
signull (int arg)
{
  /* nothing */
}

static struct signal_handler fatal_signals[] = {
  { SIGHUP, signull },
  { SIGINT, signull },
  { SIGTERM, signull },
  { SIGQUIT, signull },
  { SIGPIPE, SIG_IGN },
  { 0, NULL }
};

static void
server (void)
{
  int i;
  pthread_t thr;
  struct sigaction act;
  sigset_t sigs;
  void *res;

  sigemptyset (&sigs);

  act.sa_flags = 0;
  sigemptyset (&act.sa_mask);

  for (i = 0; fatal_signals[i].signo; i++)
    {
      sigaddset (&sigs, fatal_signals[i].signo);
      act.sa_handler = fatal_signals[i].handler;
      sigaction (fatal_signals[i].signo, &act, NULL);
    }
  pthread_sigmask (SIG_BLOCK, &sigs, NULL);

  /* thread stuff */
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

  /* set new stack size  */
  if (pthread_attr_setstacksize (&attr, 1 << 18))
    abend ("can't set stack size");

  /* start timer */
  if (pthread_create (&thr, &attr, thr_timer, NULL))
    abend ("can't create timer thread: %s", strerror (errno));

  /*
   * Create the worker threads
   */

  /*
   * Initialize worker_count to minimum to prevent pound_http_dequeue from
   * counting idle timeout.
   */
  worker_count = worker_min_count;
  for (i = 0; i < worker_min_count; i++)
    {
      worker_start ();
      /* Adjust worker_count */
      worker_count--;
    }

  pthread_create (&thr, NULL, thr_dispatch, NULL);

  /* Wait for a signal to arrive */
  sigwait (&sigs, &i);

  logmsg (LOG_NOTICE, "shutting down...");

  /* Stop main dispatcher thread */
  pthread_cancel (thr);
  pthread_join (thr, &res);

  switch (i)
    {
    case SIGHUP:
    case SIGINT:
      active_threads_wait ();
      break;

    default:
      /* Exit immediately */
      break;
    }

  cleanup ();

  exit (0);
}

static void
supervisor (void)
{
  pid_t pid = 0;
  int i;
  struct sigaction act;
  sigset_t sigs;
  int status;

  enum supervisor_state
  {
    S_RUNNING,         /* Program is running */
    S_TERMINATING,     /* Program is terminating */
  } state = S_RUNNING;

  sigemptyset (&sigs);

  act.sa_flags = 0;
  sigemptyset (&act.sa_mask);

  for (i = 0; fatal_signals[i].signo; i++)
    {
      sigaddset (&sigs, fatal_signals[i].signo);
      act.sa_handler = fatal_signals[i].handler;
      sigaction (fatal_signals[i].signo, &act, NULL);
    }

  act.sa_handler = signull;
  sigaction (SIGCHLD, &act, NULL);
  sigaddset (&sigs, SIGCHLD);

  act.sa_handler = signull;
  sigaction (SIGALRM, &act, NULL);
  sigaddset (&sigs, SIGALRM);

  for (;;)
    {
      if (pid == 0)
	{
	  if (state != S_RUNNING)
	    break;
	  pid = fork ();
	  if (pid == -1)
	    {
	      logmsg (LOG_ERR, "fork: %s", strerror (errno));
	      break;
	    }
	  if (pid == 0)
	    {
	      server ();
	      exit (0);
	    }
	}

      sigwait (&sigs, &i);
      logmsg (LOG_NOTICE, "got signal %d", i);

      if (i == SIGCHLD)
	{
	  if (wait (&status) != pid)
	    {
	      logmsg (LOG_ERR, "wait: %s", strerror (errno));
	    }
	  else if (WIFEXITED (status))
	    {
	      int code = WEXITSTATUS (status);
	      if (code == 0)
		return;
	      else
		logmsg (LOG_NOTICE, "child exited with status %d", code);
	    }
	  else if (WIFSIGNALED (status))
	    {
	      char const *coremsg = "";
#ifdef WCOREDUMP
	      if (WCOREDUMP (status))
		coremsg = " (core dumped)";
#endif
	      logmsg (LOG_NOTICE, "child terminated on signal %d%s",
		      WTERMSIG (status), coremsg);
	    }
	  else if (WIFSTOPPED (status))
	    {
	      logmsg (LOG_NOTICE, "child stopped on signal %d",
		      WSTOPSIG (status));
	    }
	  else
	    {
	      logmsg (LOG_NOTICE, "child terminated with unrecognized status %d", status);
	    }
	  if (state == S_RUNNING)
	    {
	      /* restart the child */
	      pid = 0;
	      continue;
	    }
	  else
	    break;
	}
      else if (i == SIGALRM)
	{
	  kill (pid, SIGKILL);
	  break;
	}
      else if (state == S_RUNNING)
	{
	  /* Termination signal received.  Send it to child. */
	  if (pid)
	    kill (pid, i);
	  state = S_TERMINATING;
	  alarm (grace + 1);
	}
    }
}

void
detach (void)
{
  struct sigaction oldsa, sa;
  pid_t pid;
  int ec;

  sigemptyset (&sa.sa_mask);
  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;

  if (sigaction (SIGHUP, &sa, &oldsa))
    abend ("sigaction: %s", strerror (errno));

  switch (fork ())
    {
    case -1:
      abend ("fork: %s", strerror (errno));
      _exit (1);

    case 0:
      break;

    default:
      _exit (0);
    }

  pid = setsid ();
  ec = errno;

  sigaction (SIGHUP, &oldsa, NULL);

  if (pid == -1)
    abend ("setsid: %s", strerror (ec));

  if (chdir ("/"))
    abend ("can't change to /: %s", strerror (errno));

  close (0);
  close (1);
  close (2);
  if (open ("/dev/null", O_RDONLY) == -1)
    abend ("can't open /dev/null: %s", strerror (errno));
  if (open ("/dev/null", O_WRONLY) == -1)
    abend ("can't open /dev/null: %s", strerror (errno));
  if (dup (1) == -1)
    abend ("dup failed: %s", strerror (errno));
}

#if EARLY_PTHREAD_CANCEL_PROBE
/*
 * In GNU libc, a call to pthread_cancel involves loading the libgcc_s.so.1
 * shared library.  If pound is running in a chroot, this fails with the
 * diagnostics
 *
 *    libgcc_s.so.1 must be installed for pthread_cancel to work
 *
 * after which the program aborts.  That means that normal pound shutdown
 * sequence is not performed properly.  To avoid this, the following kludge
 * is implemented: a dummy thread is created and immediately cancelled before
 * doing chroot.  This should load libgcc_s.so.1 early, so that it remains
 * loaded after chroot.
 *
 * In case other libc flavours exhibit similar behaviour, this hack can
 * be enabled at compile time by giving the --enable-pthread-cancel-probe
 * option to configure.
 */
static void *
thr_cancel (void *dummy)
{
  pause ();
  return NULL;
}

static void
probe_pthread_cancel (void)
{
  pthread_t t;
  void *p;
  pthread_create (&t, NULL, thr_cancel, NULL);
  pthread_cancel (t);
  pthread_join (t, &p);
}
#else
# define probe_pthread_cancel()
#endif

int
main (const int argc, char **argv)
{
  LISTENER *lstn;
  uid_t user_id = -1;
  gid_t group_id = -1;
#ifndef SOL_TCP
  struct protoent *pe;
#endif

  umask (077);
  srandom (getpid ());

  /* SSL stuff */
  SSL_load_error_strings ();
  SSL_library_init ();
  OpenSSL_add_all_algorithms ();
  l_init ();
  CRYPTO_set_id_callback (l_id);
  CRYPTO_set_locking_callback (l_lock);

  /* prepare regular expressions */
  if (regcomp (&HEADER, "^([a-z0-9!#$%&'*+.^_`|~-]+):[ \t]*(.*)[ \t]*$",
	       REG_ICASE | REG_NEWLINE | REG_EXTENDED)
      || regcomp (&CONN_UPGRD, "(^|[ \t,])upgrade([ \t,]|$)",
		  REG_ICASE | REG_NEWLINE | REG_EXTENDED)
      || regcomp (&LOCATION, "(http|https)://([^/]+)(.*)",
		  REG_ICASE | REG_NEWLINE | REG_EXTENDED))
    abend ("bad essential Regex");

#ifndef SOL_TCP
  /* for systems without the definition */
  if ((pe = getprotobyname ("tcp")) == NULL)
    {
      logmsg (LOG_ERR, "missing TCP protocol");
      exit (1);
    }
  SOL_TCP = pe->p_proto;
#endif

  json_memabrt = lognomem;

  /* read config */
  config_parse (argc, argv);

  if (log_facility != -1)
    openlog (progname, LOG_CONS | LOG_NDELAY | LOG_PID, log_facility);

  /* set uid if necessary */
  if (user)
    {
      struct passwd *pw;

      if ((pw = getpwnam (user)) == NULL)
	abend ("no such user %s", user);
      user_id = pw->pw_uid;
    }

  /* set gid if necessary */
  if (group)
    {
      struct group *gr;

      if ((gr = getgrnam (group)) == NULL)
	abend ("no such group %s", group);
      group_id = gr->gr_gid;
    }

  logmsg (LOG_NOTICE, "starting...");

  /* open listeners */
  n_listeners = 0;
  SLIST_FOREACH (lstn, &listeners, next)
    {
      if (lstn->sock == -1)
	{
	  /* prepare the socket */
	  int opt;
	  int domain;
	  char abuf[MAX_ADDR_BUFSIZE];
	  mode_t oldmask;

	  switch (lstn->addr.ai_family)
	    {
	    case AF_INET:
	      domain = PF_INET;
	      break;
	    case AF_INET6:
	      domain = PF_INET6;
	      break;
	    case AF_UNIX:
	      domain = PF_UNIX;
	      unlink (((struct sockaddr_un*)lstn->addr.ai_addr)->sun_path);
	      oldmask = umask (0777 & ~lstn->mode);
	      break;
	    default:
	      abort ();
	    }
	  if ((lstn->sock = socket (domain, SOCK_STREAM, 0)) < 0)
	    abend ("can't create HTTP socket %s: %s",
		   addr2str (abuf, sizeof (abuf), &lstn->addr, 0),
		   strerror (errno));

	  opt = 1;
	  setsockopt (lstn->sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof (opt));
	  if (bind (lstn->sock, lstn->addr.ai_addr,
		    (socklen_t) lstn->addr.ai_addrlen) < 0)
	    abend ("can't bind HTTP socket to %s: %s",
		   addr2str (abuf, sizeof (abuf), &lstn->addr, 0),
		   strerror (errno));

	  if (domain == PF_UNIX)
	    {
	      umask (oldmask);
	      if (lstn->chowner)
		{
		  char *sname = ((struct sockaddr_un*)lstn->addr.ai_addr)->sun_path;
		  if (chown (sname, user_id, group_id))
		    logmsg (LOG_ERR, "can't chown socket %s to %d:%d: %s",
			    sname, user_id, group_id, strerror (errno));
		}
	    }

	  if (listen (lstn->sock, 512))
	    abend ("can't listen on %s: %s",
                   addr2str (abuf, sizeof (abuf), &lstn->addr, 0),
                   strerror (errno));
	}
      n_listeners++;
    }

  print_log = 0;
  if (daemonize)
    detach ();

  pidfile_create ();

  /* chroot if necessary */
  if (root_jail)
    {
      unsigned char random;

      /* Make sure openssl opens /dev/urandom before the chroot. */
      RAND_bytes (&random, 1);

      /*
       * Give libc a chance to load any libraries necessary for pthread_cancel
       * to work.  See the comment above.
       */
      probe_pthread_cancel ();

      if (chroot (root_jail))
	abend ("chroot: %s", strerror (errno));

      if (chdir ("/"))
	abend ("chdir /: %s", strerror (errno));
    }

  if (group)
    if (setgid (group_id) || setegid (group_id))
      abend ("setgid: %s", strerror (errno));

  if (user)
    if (setuid (user_id) || seteuid (user_id))
      abend ("setuid: %s", strerror (errno));

  if (enable_supervisor && daemonize)
    supervisor ();
  else
    server ();

  return 0;
}
