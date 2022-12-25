/*
 * Pound - the reverse-proxy load-balancer
 * Copyright (C) 2002-2010 Apsis GmbH
 *
 * This file is part of Pound.
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
 *
 * Contact information:
 * Apsis GmbH
 * P.O.Box
 * 8707 Uetikon am See
 * Switzerland
 * EMail: roseg@apsis.ch
 */

#include "config.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <pwd.h>
#include <grp.h>
#include <syslog.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fnmatch.h>

#if HAVE_GETOPT_H
# include <getopt.h>
#endif

#ifndef UNIX_PATH_MAX
/* on Linux this is defined in linux/un.h rather than sys/un.h - go figure */
# define UNIX_PATH_MAX   108
#endif

#ifndef NI_MAXHOST
# define NI_MAXHOST      1025
#endif

#ifndef NI_MAXSERV
# define NI_MAXSERV      32
#endif

#define MAX_ADDR_BUFSIZE (NI_MAXHOST + NI_MAXSERV + 4)

#if HAVE_OPENSSL_SSL_H
# define OPENSSL_THREAD_DEFINES
# include <openssl/ssl.h>
# include <openssl/lhash.h>
# include <openssl/err.h>
# if OPENSSL_VERSION_NUMBER >= 0x00907000L
#  ifndef OPENSSL_THREADS
#   error "Pound requires OpenSSL with thread support"
#  endif
# else
#  ifndef THREADS
#   error "Pound requires OpenSSL with thread support"
#  endif
# endif
#else
# error "Pound needs openssl/ssl.h"
#endif

#if HAVE_OPENSSL_ENGINE_H
# include <openssl/engine.h>
#endif

#if HAVE_LIBPCREPOSIX
# if HAVE_PCREPOSIX_H
#  include <pcreposix.h>
# elif HAVE_PCRE_PCREPOSIX_H
#  include <pcre/pcreposix.h>
# else
#  error "You have libpcreposix, but the header files are missing. Use --disable-pcreposix"
# endif
#else
# include <regex.h>
#endif

#ifdef  HAVE_LONG_LONG_INT
# define LONG    long long
# define L0      0LL
# define L_1     -1LL
# define STRTOL  strtoll
# define ATOL    atoll
# define PRILONG "lld"
#else
# define LONG    long
# define L0      0L
# define L_1     -1L
# define STRTOL  strtol
# define ATOL    atol
# define PRILONG "ld"
#endif

#ifndef DEFAULT_WORKER_MIN
# define DEFAULT_WORKER_MIN 5
#endif

#ifndef DEFAULT_WORKER_MAX
# define DEFAULT_WORKER_MAX 128
#endif

#ifndef DEFAULT_WORKER_IDLE_TIMEOUT
# define DEFAULT_WORKER_IDLE_TIMEOUT 30
#endif

#ifndef DEFAULT_GRACE_TO
# define DEFAULT_GRACE_TO 30
#endif

#ifndef DEFAULT_ALIVE_TO
# define DEFAULT_ALIVE_TO 30
#endif

#ifndef MAXBUF
# define MAXBUF      4096
#endif

#define MAXHEADERS  128

#ifndef SYSCONFDIR
# define SYSCONFDIR "/etc"
#endif
#ifndef LOCALSTATEDIR
# define LOCALSTATEDIR "/var"
#endif

#ifndef POUND_CONF
# define POUND_CONF SYSCONFDIR "/" "pound.cfg"
#endif

#ifndef POUND_PID
# define POUND_PID  LOCALSTATEDIR "/run/pound.pid"
#endif

#define ATTR_PRINTFLIKE(fmt,narg)                               \
    __attribute__ ((__format__ (__printf__, fmt, narg)))

/* HTTP methods */

enum
  {
    METH_GET,
    METH_POST,
    METH_HEAD,
    METH_PUT,
    METH_PATCH,
    METH_DELETE,
    METH_LOCK,
    METH_UNLOCK,
    METH_PROPFIND,
    METH_PROPPATCH,
    METH_SEARCH,
    METH_MKCOL,
    METH_MOVE,
    METH_COPY,
    METH_OPTIONS,
    METH_TRACE,
    METH_MKACTIVITY,
    METH_CHECKOUT,
    METH_MERGE,
    METH_REPORT,
    METH_SUBSCRIBE,
    METH_UNSUBSCRIBE,
    METH_BPROPPATCH,
    METH_POLL,
    METH_BMOVE,
    METH_BCOPY,
    METH_BDELETE,
    METH_BPROPFIND,
    METH_NOTIFY,
    METH_CONNECT,
    METH_RPC_IN_DATA,
    METH_RPC_OUT_DATA,
  };

/* HTTP errors */
enum
  {
    HTTP_STATUS_OK,                // 200
    HTTP_STATUS_BAD_REQUEST,       // 400
    HTTP_STATUS_NOT_FOUND,         // 404
    HTTP_STATUS_PAYLOAD_TOO_LARGE, // 413
    HTTP_STATUS_URI_TOO_LONG,      // 414
    HTTP_STATUS_INTERNAL_SERVER_ERROR,          // 500
    HTTP_STATUS_NOT_IMPLEMENTED,   // 501
    HTTP_STATUS_SERVICE_UNAVAILABLE, // 503
    HTTP_STATUS_MAX
  };

/*
 * Operations on struct timespec
 */
/*
 * Compare two timespecs
 */
static inline int
timespec_cmp (struct timespec const *a, struct timespec const *b)
{
  if (a->tv_sec < b->tv_sec)
    return -1;
  if (a->tv_sec > b->tv_sec)
    return 1;
  if (a->tv_nsec < b->tv_nsec)
    return -1;
  if (a->tv_nsec > b->tv_nsec)
    return 1;
  return 0;
}

static inline struct timespec
timespec_sub (struct timespec const *a, struct timespec const *b)
{
  struct timespec d;

  d.tv_sec = a->tv_sec - b->tv_sec;
  d.tv_nsec = a->tv_nsec - b->tv_nsec;
  if (d.tv_nsec < 0)
    {
      --d.tv_sec;
      d.tv_nsec += 1e9;
    }

  return d;
}


/* List definitions. */
#include "list.h"

#define POUND_TID() ((unsigned long)pthread_self ())
#define PRItid "lx"

struct cidr;

typedef struct acl
{
  char *name;                 /* ACL name (optional) */
  SLIST_HEAD (,cidr) head;    /* List of CIDRs */
  SLIST_ENTRY (acl) next;
} ACL;

typedef SLIST_HEAD (,acl) ACL_HEAD;

int acl_match (ACL *acl, struct sockaddr *sa);

/* matcher chain */
typedef struct _matcher
{
  regex_t pat;		/* pattern to match the request/header against */
  SLIST_ENTRY (_matcher) next;
} MATCHER;

typedef SLIST_HEAD(,_matcher) MATCHER_HEAD;

/* back-end types */
typedef enum
  {
    SESS_NONE,
    SESS_IP,
    SESS_COOKIE,
    SESS_URL,
    SESS_PARM,
    SESS_HEADER,
    SESS_BASIC
  }
  SESS_TYPE;

typedef enum
  {
    BE_BACKEND,
    BE_REDIRECT,
    BE_ACME,
    BE_CONTROL
  }
  BACKEND_TYPE;

/* back-end definition */
typedef struct _backend
{
  struct _service *service;     /* Back pointer to the owning service */
  BACKEND_TYPE be_type;         /* Backend type */
  struct addrinfo addr;		/* IPv4/6 address */
  int priority;			/* priority */
  unsigned to;			/* read/write time-out */
  unsigned conn_to;		/* connection time-out */
  unsigned ws_to;		/* websocket time-out */
  char *url;			/* for redirectors */
  int redir_code;               /* Redirection code (301, 302, or 307) */
  int redir_req;		/* the redirect should include the request path */
  SSL_CTX *ctx;			/* CTX for SSL connections */

  /* FIXME: The following four fields are currently not used: */
  pthread_mutex_t mut;		/* mutex for this back-end */
  int n_requests;		/* number of requests seen */
  double t_requests;		/* time to answer these requests */
  double t_average;		/* average time to answer requests */

  int alive;			/* false if the back-end is dead */
  int disabled;			/* true if the back-end is disabled */
  SLIST_ENTRY (_backend) next;
} BACKEND;

typedef SLIST_HEAD (,_backend) BACKEND_HEAD;

typedef struct session
{
  char *key;
  BACKEND *backend;
  struct timespec expire;
  DLIST_ENTRY (session) link;
} SESSION;

/* maximal session key size */
#define KEY_SIZE    127

#if OPENSSL_VERSION_NUMBER >= 0x10100000L
DEFINE_LHASH_OF (SESSION);
#elif OPENSSL_VERSION_NUMBER >= 0x10000000L
DECLARE_LHASH_OF (SESSION);
#endif

typedef LHASH_OF (SESSION) SESSION_HASH;

typedef struct
{
  SESSION_HASH *hash;
  DLIST_HEAD (,session) head;
} SESSION_TABLE;

SESSION_TABLE *session_table_new (void);

enum
  {
    BOOL_AND,
    BOOL_OR,
    BOOL_NOT
  };

struct bool_service_cond
{
  int op;
  SLIST_HEAD(,_service_cond) head;
};

enum service_cond_type
  {
    COND_BOOL,
    COND_ACL,
    COND_URL,
    COND_HDR,
  };

typedef struct _service_cond
{
  enum service_cond_type type;
  union
  {
    ACL *acl;
    regex_t re;
    struct bool_service_cond bool;
    struct _service_cond *cond;
  };
  SLIST_ENTRY (_service_cond) next;
} SERVICE_COND;

static inline void
service_cond_init (SERVICE_COND *cond, int type)
{
  cond->type = type;
  switch (type)
    {
    case COND_ACL:
    case COND_URL:
    case COND_HDR:
      break;

    case COND_BOOL:
      cond->bool.op = BOOL_AND;
      SLIST_INIT (&cond->bool.head);
      break;
    }
}

/* service definition */
typedef struct _service
{
  char name[KEY_SIZE + 1];	/* symbolic name */
  SERVICE_COND cond;
  BACKEND_HEAD backends;
  BACKEND *emergency;
  int abs_pri;			/* abs total priority for all back-ends */
  int tot_pri;			/* total priority for current back-ends */
  pthread_mutex_t mut;		/* mutex for this service */
  SESS_TYPE sess_type;
  unsigned sess_ttl;		/* session time-to-live */
  regex_t sess_start;		/* pattern to identify the session data */
  regex_t sess_pat;		/* pattern to match the session data */
  SESSION_TABLE *sessions;	/* currently active sessions */
  int disabled;			/* true if the service is disabled */
  SLIST_ENTRY (_service) next;
} SERVICE;

typedef SLIST_HEAD (,_service) SERVICE_HEAD;

typedef struct _pound_ctx
{
  SSL_CTX *ctx;
  char *server_name;
  unsigned char **subjectAltNames;
  unsigned int subjectAltNameCount;
  SLIST_ENTRY (_pound_ctx) next;
} POUND_CTX;

typedef SLIST_HEAD (,_pound_ctx) POUND_CTX_HEAD;

/* Listener definition */
typedef struct _listener
{
  struct addrinfo addr;		/* IPv4/6 address */
  int sock;			/* listening socket */
  POUND_CTX_HEAD ctx_head;	/* CTX for SSL connections */
  int clnt_check;		/* client verification mode */
  int noHTTPS11;		/* HTTP 1.1 mode for SSL */
  char *add_head;		/* extra SSL header */
  int verb;			/* allowed HTTP verb group */
  unsigned to;			/* client time-out */
  int has_pat;			/* was a URL pattern defined? */
  regex_t url_pat;		/* pattern to match the request URL against */
  char *http_err[HTTP_STATUS_MAX];	/* error messages */
  LONG max_req;			/* max. request size */
  MATCHER_HEAD head_off;	/* headers to remove */
  int rewr_loc;			/* rewrite location response */
  int rewr_dest;		/* rewrite destination header */
  int disabled;			/* true if the listener is disabled */
  int log_level;		/* log level for this listener */
  int allow_client_reneg;	/* Allow Client SSL Renegotiation */
  SERVICE_HEAD services;
  SLIST_ENTRY (_listener) next;

  /* Used during configuration parsing */
  int ssl_op_enable;
  int ssl_op_disable;
  int has_other;
} LISTENER;

typedef SLIST_HEAD(,_listener) LISTENER_HEAD;

typedef struct _thr_arg
{
  int sock;
  LISTENER *lstn;
  struct addrinfo from_host;
  SLIST_ENTRY(_thr_arg) next;
} THR_ARG;		/* argument to processing threads: socket, origin */

typedef SLIST_HEAD(,_thr_arg) THR_ARG_HEAD;

/* Track SSL handshare/renegotiation so we can reject client-renegotiations. */
typedef enum
  {
    RENEG_INIT = 0,
    RENEG_REJECT,
    RENEG_ALLOW,
    RENEG_ABORT
  }
  RENEG_STATE;

/* Header types */
#define HEADER_ILLEGAL              -1
#define HEADER_OTHER                0
#define HEADER_TRANSFER_ENCODING    1
#define HEADER_CONTENT_LENGTH       2
#define HEADER_CONNECTION           3
#define HEADER_LOCATION             4
#define HEADER_CONTLOCATION         5
#define HEADER_HOST                 6
#define HEADER_REFERER              7
#define HEADER_USER_AGENT           8
#define HEADER_URI                  9
#define HEADER_DESTINATION          10
#define HEADER_EXPECT               11
#define HEADER_UPGRADE              13

/* control request stuff */
typedef enum
{
  CTRL_LST,
  CTRL_EN_LSTN, CTRL_DE_LSTN,
  CTRL_EN_SVC, CTRL_DE_SVC,
  CTRL_EN_BE, CTRL_DE_BE,
  CTRL_ADD_SESS, CTRL_DEL_SESS
} CTRL_CODE;

typedef struct
{
  CTRL_CODE cmd;
  int listener;
  int service;
  int backend;
  char key[KEY_SIZE + 1];
} CTRL_CMD;

/* add a request to the queue */
int put_thr_arg (THR_ARG *);
/* get a request from the queue */
THR_ARG *get_thr_arg (void);
/* get the current queue length */
int get_thr_qlen (void);
/* Decrement number of active threads. */
void active_threads_decr (void);

/* handle HTTP requests */
void *thr_http (void *);

/* Log an error to the syslog or to stderr */
void logmsg (const int, const char *, ...)
  ATTR_PRINTFLIKE(2,3);

/* Parse a URL, possibly decoding hexadecimal-encoded characters */
int cpURL (char *, char *, int);

/* Translate inet/inet6 address into a string */
char *addr2str (char *, int, const struct addrinfo *, int);

/* Return a string representation for a back-end address */
char *str_be (char *buf, size_t size, BACKEND *be);

struct submatch
{
  size_t matchn;
  size_t matchmax;
  regmatch_t *matchv;
};

#define SUBMATCH_INITIALIZER { 0, 0, NULL }

void submatch_free (struct submatch *sm);

/* Find the right service for a request */
SERVICE *get_service (const LISTENER *, struct sockaddr *,
		      const char *, char **const, struct submatch *);

/* Find the right back-end for a request */
BACKEND *get_backend (SERVICE * const, const struct addrinfo *,
		      const char *, char **const);

/* Search for a host name, return the addrinfo for it */
int get_host (char *const, struct addrinfo *, int);

/*
 * Find if a redirect needs rewriting
 * In general we have two possibilities that require it:
 * (1) if the redirect was done to the correct location with the wrong protocol
 * (2) if the redirect was done to the back-end rather than the listener
 */
int need_rewrite (const int, char *const, char *const, const char *,
		  const LISTENER *, const BACKEND *);
/*
 * (for cookies only) possibly create session based on response headers
 */
void upd_session (SERVICE * const, char **const, BACKEND * const);

/*
 * Parse a header
 */
int check_header (const char *, char *);

#define BE_DISABLE  -1
#define BE_KILL     1
#define BE_ENABLE   0

/*
 * mark a backend host as dead;
 * do nothing if no resurection code is active
 */
void kill_be (SERVICE *, BACKEND *, const int);

/*
 * Update the number of requests and time to answer for a given back-end
 */
void upd_be (SERVICE * const svc, BACKEND * const be, const double);

/*
 * Non-blocking version of connect(2). Does the same as connect(2) but
 * ensures it will time-out after a much shorter time period CONN_TO.
 */
int connect_nb (const int, const struct addrinfo *, const int);

/*
 * Parse arguments/config file
 */
void config_parse (int, char **);

/*
 * RSA ephemeral keys: how many and how often
 */
#define N_RSA_KEYS  11
#ifndef T_RSA_KEYS
# define T_RSA_KEYS  7200
#endif

/*
 * Renegotiation callback
 */
void SSLINFO_callback (const SSL * s, int where, int rc);

/*
 * run periodic functions:
 *  - RSAgen every T_RSA_KEYS seconds (on older OpenSSL)
 *  - probe dead backends every alive_to seconds
 *  - clean up expire sessions as needed
 */
void *thr_timer (void *);

void POUND_SSL_CTX_init (SSL_CTX *ctx);
int set_ECDHCurve (char *name);

void *mem2nrealloc (void *p, size_t *pn, size_t s);
void xnomem (void);
void *xmalloc (size_t s);
void *xcalloc (size_t nmemb, size_t size);
#define xzalloc(s) xcalloc(1, s)
#define XZALLOC(v) (v = xzalloc (sizeof ((v)[0])))

void *xrealloc (void *p, size_t s);
void *x2nrealloc (void *p, size_t *pn, size_t s);
char *xstrdup (char const *s);
char *xstrndup (const char *s, size_t n);

struct stringbuf
{
  char *base;                     /* Buffer storage. */
  size_t size;                    /* Size of buf. */
  size_t len;                     /* Actually used length in buf. */
};

void stringbuf_init (struct stringbuf *sb);
void stringbuf_reset (struct stringbuf *sb);
char *stringbuf_finish (struct stringbuf *sb);
void stringbuf_free (struct stringbuf *sb);
void stringbuf_add (struct stringbuf *sb, char const *str, size_t len);
void stringbuf_add_char (struct stringbuf *sb, int c);
void stringbuf_add_string (struct stringbuf *sb, char const *str);
void stringbuf_vprintf (struct stringbuf *sb, char const *fmt, va_list ap);
void stringbuf_printf (struct stringbuf *sb, char const *fmt, ...)
  ATTR_PRINTFLIKE(2,3);

static inline char *stringbuf_value (struct stringbuf *sb)
{
  return sb->base;
}

void job_enqueue_unlocked (struct timespec const *ts, void (*func) (void *), void *data);
void job_enqueue_after_unlocked (unsigned t, void (*func) (void *), void *data);

void job_enqueue (struct timespec const *ts, void (*func) (void *), void *data);
void job_enqueue_after (unsigned t, void (*func) (void *), void *data);

void job_rearm_unlocked (struct timespec *ts, void (*func) (void *), void *data);
void job_rearm (struct timespec *ts, void (*func) (void *), void *data);

char const *sess_type_to_str (int type);
int control_reply (BIO *c, int method, const char *url, BACKEND *be);
void pound_atexit (void (*func) (void *), void *arg);
void unlink_file (void *arg);
