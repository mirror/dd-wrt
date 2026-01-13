/*
 * Pound - the reverse-proxy load-balancer
 * Copyright (C) 2002-2010 Apsis GmbH
 * Copyright (C) 2018-2025 Sergey Poznyakoff
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
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <limits.h>

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

#define PORT_HTTP 80
#define PORT_HTTPS 443

#define TS(s) #s
#define TOSTR(s) TS(s)
#define PORT_HTTP_STR TOSTR(PORT_HTTP)
#define PORT_HTTPS_STR TOSTR(PORT_HTTPS)

#if HAVE_OPENSSL_SSL_H
# define OPENSSL_THREAD_DEFINES
# include <openssl/ssl.h>
# include <openssl/lhash.h>
# include <openssl/err.h>
# include <openssl/rand.h>
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

struct genpat
{
  struct genpat_defn const *vtab;
  void *data;
};

typedef struct genpat *GENPAT;

typedef struct {
  int rm_so;
  int rm_eo;
} POUND_REGMATCH;

#define GENPAT_DEFAULT   0
#define GENPAT_ICASE     0x1
#define GENPAT_MULTILINE 0x2

int genpat_compile (GENPAT *, int, const char *, int);
int genpat_match (GENPAT, const char *, size_t, POUND_REGMATCH *);
void genpat_free (GENPAT);
char const *genpat_error (GENPAT, size_t *);
size_t genpat_nsub (GENPAT);

enum
  {
    GENPAT_POSIX,
    GENPAT_PCRE,
    GENPAT_PREFIX,
    GENPAT_SUFFIX,
    GENPAT_CONTAIN,
    GENPAT_EXACT,
  };

struct genpat_defn
{
  int (*gp_init) (GENPAT);
  int (*gp_compile) (void *, const char *, int);
  char const *(*gp_error) (void *, size_t *);
  int (*gp_exec) (void *, const char *, size_t, POUND_REGMATCH *);
  size_t (*gp_nsub) (void *);
  void (*gp_free) (void *);
};

extern struct genpat_defn posix_genpat_defn;
extern struct genpat_defn prefix_genpat_defn;
extern struct genpat_defn suffix_genpat_defn;
extern struct genpat_defn contain_genpat_defn;
extern struct genpat_defn exact_genpat_defn;

#ifdef HAVE_LIBPCRE
extern struct genpat_defn pcre_genpat_defn;
# define PCRE_REGEX_DEFN &pcre_genpat_defn
#else
# define PCRE_REGEX_DEFN NULL
#endif

#ifdef  HAVE_LONG_LONG_INT
typedef long long CONTENT_LENGTH;
# define CONTENT_LENGTH_MAX LLONG_MAX
# define PRICLEN "lld"
#else
typedef long CONTENT_LENGTH;
# define CONTENT_LENGTH_MAX LONG_MAX
# define PRICLEN "ld"
#endif

#define NO_CONTENT_LENGTH ((CONTENT_LENGTH) -1)

int strtoclen (char const *arg, int base, CONTENT_LENGTH *retval, char **endptr);
unsigned long strhash_ci (const char *c, size_t len);


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

#ifndef SYSCONFDIR
# define SYSCONFDIR "/etc"
#endif
#ifndef LOCALSTATEDIR
# define LOCALSTATEDIR "/var"
#endif
#ifndef PKGDATADIR
# define PKGDATADIR "/usr/share/pound"
#endif

#define POUND_TMPL_PATH "~/.poundctl.tmpl:" PKGDATADIR

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
  };

char const *method_name (int meth);

/* HTTP errors */
enum
  {
    HTTP_STATUS_OK,                // 200
    HTTP_STATUS_BAD_REQUEST,       // 400
    HTTP_STATUS_UNAUTHORIZED,      // 401
    HTTP_STATUS_FORBIDDEN,         // 403
    HTTP_STATUS_NOT_FOUND,         // 404
    HTTP_STATUS_METHOD_NOT_ALLOWED,// 405
    HTTP_STATUS_PAYLOAD_TOO_LARGE, // 413
    HTTP_STATUS_URI_TOO_LONG,      // 414
    HTTP_STATUS_TOO_MANY_REQUESTS, // 429
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

enum { NANOSECOND = 1000000000L };

static inline struct timespec
timespec_sub (struct timespec const *a, struct timespec const *b)
{
  struct timespec d;

  d.tv_sec = a->tv_sec - b->tv_sec;
  d.tv_nsec = a->tv_nsec - b->tv_nsec;
  if (d.tv_nsec < 0)
    {
      --d.tv_sec;
      d.tv_nsec += NANOSECOND;
    }

  return d;
}

/* Memory allocation primitives. */
#include "mem.h"
/* List definitions. */
#include "list.h"
/* Configuration parser */
#include "cfgparser.h"
#include "cctype.h"

char *locus_point_str (struct locus_point const *loc);
char *locus_range_str (struct locus_range const *loc);

int field_list_filter (char const *subj, size_t len,
		       int (*pred) (char const *, size_t, void *),
		       void *data, char **endp);

/* Header types */
enum
  {
    HEADER_ILLEGAL = -1,
    HEADER_OTHER = 0,
    HEADER_TRANSFER_ENCODING,
    HEADER_CONTENT_LENGTH,
    HEADER_CONNECTION,
    HEADER_LOCATION,
    HEADER_CONTLOCATION,
    HEADER_HOST,
    HEADER_REFERER,
    HEADER_USER_AGENT,
    HEADER_URI,
    HEADER_DESTINATION,
    HEADER_EXPECT,
    HEADER_UPGRADE,
    HEADER_AUTHORIZATION,
  };

struct http_header
{
  char *header;
  int code;
  size_t name_start;
  size_t name_end;
  size_t val_start;
  size_t val_end;
  char *value;
  DLIST_ENTRY (http_header) link;
};

static inline char const *
http_header_name_ptr (struct http_header *hdr)
{
  return hdr->header + hdr->name_start;
}

static inline size_t
http_header_name_len (struct http_header *hdr)
{
  return hdr->name_end - hdr->name_start;
}

typedef DLIST_HEAD(,http_header) HTTP_HEADER_LIST;
#define HTTP_HEADER_LIST_INITIALIZER DLIST_HEAD_INITIALIZER

int http_header_list_parse (HTTP_HEADER_LIST *head, char const *text,
			    int replace, char **end);
void http_header_list_free (HTTP_HEADER_LIST *head);

/* Append modes: what to do if the header with that name already exist. */
enum
  {
    H_KEEP,     /* Keep old header, discard changes. */
    H_REPLACE,  /* Replace old header with the new one. */
    H_APPEND    /* Append to the value (assume #(values)) */
  };

int http_header_list_append (HTTP_HEADER_LIST *head, char const *text,
			     int replace);
int http_header_list_append_list (HTTP_HEADER_LIST *head,
				  HTTP_HEADER_LIST *add);
void http_header_list_remove_field (HTTP_HEADER_LIST *head, char const *field);

struct query_param
{
  char *name;
  char *value;
  DLIST_ENTRY (query_param) link;
};

typedef DLIST_HEAD (,query_param) QUERY_HEAD;
#define QUERY_EMPTY DLIST_EMPTY

struct http_request
{
  char *request;             /* Request line */
  HTTP_HEADER_LIST headers;  /* Request headers */
  int method;                /* Method code (see METH_* constants above) */
  int version;               /* HTTP minor version: 0 or 1 */
  char *url;                 /* URL part of the request */
  char *path;                /* URL Path */
  char *query;               /* URL query */
  QUERY_HEAD query_head;
  char *orig_request_line;   /* Original request line (for logging purposes) */
  int split;
  char *eval_result;         /* Array of return statuses from evaluation of
				detached conditions. */
  struct stringbuf *body;    /* Response body overridden by modification
				request. Deliberately not using any stream
				type here. */
};

static inline void http_request_init (struct http_request *http)
{
  memset (http, 0, sizeof (*http));
  DLIST_INIT (&http->headers);
  DLIST_INIT (&http->query_head);
}

void http_request_free (struct http_request *);
int http_request_eval_get (struct http_request *http, int n);
int http_request_eval_cache (struct http_request *http, int n, int res);

/*
 * Return codes for http_request_get_query_param,
 * http_request_get_query_param_value, and request accessors.
 */
enum
  {
    RETRIEVE_OK,
    RETRIEVE_NOT_FOUND,
    RETRIEVE_ERROR = -1
  };

int http_request_get_url (struct http_request *, char const **);
int http_request_get_query (struct http_request *, char const **);
int http_request_get_path (struct http_request *req, char const **retval);
int http_request_count_query_param (struct http_request *req);
int http_request_get_query_param_value (struct http_request *req,
					char const *name,
					char const **retval);
char const *http_request_orig_line (struct http_request *req);
int http_request_get_basic_auth (struct http_request *req,
				 char **u_name, char **u_pass);

int http_request_set_path (struct http_request *req, char const *path);
int http_request_set_url (struct http_request *req, char const *url);
int http_request_set_query (struct http_request *req, char const *rawquery);
int http_request_set_query_param (struct http_request *req, char const *name,
				  char const *raw_value);

#define POUND_TID() ((unsigned long)pthread_self ())
#define PRItid "lx"

typedef struct watcher WATCHER;

void watcher_lock (WATCHER *);
void watcher_unlock (WATCHER *);
WATCHER *watcher_register (void *obj, char
			   const *filename, struct locus_range const *loc,
			   int (*read) (void *, char const *, WORKDIR *),
			   void (*clear) (void *));
int watcher_setup (void);

char const *filename_split_wd (char const *filename, WORKDIR **wdp);

struct cidr;

typedef struct acl
{
  char *name;                 /* ACL name (optional) */
  SLIST_HEAD (,cidr) head;    /* List of CIDRs */
  SLIST_ENTRY (acl) next;
  WATCHER *watcher;
} ACL;

typedef SLIST_HEAD (,acl) ACL_HEAD;

static inline void
acl_lock (ACL *acl)
{
  watcher_lock (acl->watcher);
}

static inline void
acl_unlock (ACL *acl)
{
  watcher_unlock (acl->watcher);
}

int acl_match (ACL *acl, struct sockaddr *sa);
void acl_clear (ACL *acl);

enum job_ctl
  {
    job_ctl_run,
    job_ctl_cancel
  };
typedef void (*JOB_FUNC) (enum job_ctl, void *, const struct timespec *);
typedef unsigned long JOB_ID;

JOB_ID job_enqueue (struct timespec const *ts, JOB_FUNC func, void *data);
JOB_ID job_enqueue_after (unsigned t, JOB_FUNC func, void *data);
void job_cancel (JOB_ID id);
int job_get_timestamp (JOB_ID jid, struct timespec *ts);

enum
  {
    PNDLUA_CTX_GLOBAL,
    PNDLUA_CTX_THREAD
  };

struct pndlua_closure
{
  int ctx;
  char *func;
  int argc;
  char **argv;
  struct locus_range locus;
};

/* matcher chain */
typedef struct _matcher
{
  GENPAT pat;		/* pattern to match the request/header against */
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
    BE_REGULAR,
    BE_MATRIX,
    BE_REDIRECT,
    BE_ACME,
    BE_CONTROL,
    BE_ERROR,
    BE_METRICS,
    BE_BACKEND_REF,     /* See be_name in BACKEND */
    BE_FILE,
    BE_LUA
  }
  BACKEND_TYPE;

enum backend_resolve_mode
  {
    bres_immediate,
    bres_first,
    bres_all,
    bres_srv
  };

char const *resolve_mode_str (int mode);

typedef struct backend_table *BACKEND_TABLE;

struct be_matrix
{
  char *hostname;       /* Hostname or IP address. */
  int port;             /* Port number (network order). */
  int family;           /* Address family for resolving hostname. */
  int resolve_mode;     /* Mode for resolving hostname. */
  unsigned retry_interval; /* Retry interval for failed queries. */
  int ignore_srv_weight;   /* Ignore weight field in SRV RR. */
  unsigned override_ttl;   /* Use this TTL instead of one returned from DNS. */

  unsigned to;		/* read/write time-out */
  unsigned conn_to;	/* connection time-out */
  unsigned ws_to;	/* websocket time-out */
  SSL_CTX *ctx;		/* CTX for SSL connections */
  char *servername;     /* SNI */

  BACKEND_TABLE betab;  /* Table of regular backends generated from this
			   matrix. */
  JOB_ID jid;           /* ID of the periodic job scheduled to update this
			   matrix. */
  int weight;           /* Weight of the backend list where to allocate
			   regular backends. */
  struct _backend *parent;  /* Points to matrix backend, if this backend was
			       dynamically generated. */
};

struct http_errmsg
{
  char *text;
  HTTP_HEADER_LIST hdr;
};

#define HTTP_ERRMSG_INITIALIZER(m) \
  { NULL, HTTP_HEADER_LIST_INITIALIZER(m.hdr) }

struct be_regular
{
  struct addrinfo addr;	/* IPv4/6 address */
  int alive;		/* false if the back-end is dead */
  unsigned to;		/* read/write time-out */
  unsigned conn_to;	/* connection time-out */
  unsigned ws_to;	/* websocket time-out */
  SSL_CTX *ctx;		/* CTX for SSL connections */
  char *servername;     /* SNI */

  struct _backend *parent; /* Points to matrix backend, if this backend was
			      dynamically generated. */
};

struct be_redirect
{
  char *url;		 /* for redirectors */
  int status;            /* Redirection status (301, 302, 303, 307, or 308 ) */
  int has_uri;		 /* URL has path and/or query part. */
};

struct be_file           /* For ACME services and FILE backends. */
{
  int wd;                /* Working directory descriptor. */
};

struct be_error
{
  int status;            /* Pound HTTP status index */
  struct http_errmsg msg;
};

/* back-end definition */
typedef struct _backend
{
  struct _service *service;     /* Back pointer to the owning service */
  struct balancer *balancer;    /* Back pointer to the owning backend list. */
  struct locus_range locus;     /* Location in the config file */
  BACKEND_TYPE be_type;         /* Backend type */
  int priority;			/* priority */
  int disabled;			/* true if the back-end is disabled */
  DLIST_ENTRY (_backend) link;

  /* Auxiliary fields. */
  int mark;                     /* If set, this backend is a candidate for
				   deletion. */


  /* Statistics */
  pthread_mutex_t mut;		/* mutex for this back-end */
#ifdef ENABLE_DYNAMIC_BACKENDS
  unsigned long refcount;       /* reference counter */
#endif
  double numreq;		/* number of requests seen */
  double avgtime;		/* Avg. time per request */
  double avgsqtime;             /* Avg. squared time per request */

  /* Data specific for each backend type. */
  union
  {
    struct be_regular reg;
    struct be_matrix mtx;
    struct be_file acme;
    struct be_file file;
    struct be_redirect redirect;
    struct be_error error;
    struct pndlua_closure lua;
    char *be_name;              /* Name of the backend; Used during parsing. */
  } v;

} BACKEND;

typedef DLIST_HEAD (,_backend) BACKEND_HEAD;

static inline int backend_is_https (BACKEND *be)
{
  return be->be_type == BE_REGULAR && be->v.reg.ctx != NULL;
}

static inline int backend_is_alive (BACKEND *be)
{
  /* Matrix backends are special, they are never alive. */
  if (be->be_type == BE_MATRIX)
    return 0;
  /* Redirects, ACME, and control backends are always alive. */
  return be->be_type != BE_REGULAR || be->v.reg.alive;
}

static inline int backend_is_active (BACKEND *be)
{
  return !be->disabled && backend_is_alive (be);
}

BACKEND *backend_create (BACKEND_TYPE type, int prio,
			 struct locus_range const *loc);

typedef struct session
{
  char *key;
  BACKEND *backend;
  struct timespec expire;
  DLIST_ENTRY (session) link;
} SESSION;

#define KEY_SIZE    127

#define HT_TYPE SESSION
#define HT_NAME_FIELD key
#define HT_NO_FOREACH
#define HT_NO_HASH_FREE
#include "ht.h"

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
    COND_BOOL,  /* Boolean operation. */
    COND_ACL,   /* ACL match. */
    COND_URL,   /* URL match. */
    COND_PATH,  /* Path match. */
    COND_QUERY, /* Raw query match. */
    COND_QUERY_PARAM, /* Query parameter match */
    COND_HDR,   /* Header match. */
    COND_HOST,  /* Special case of COND_HDR: matches the value of the
		   Host: header */
    COND_BASIC_AUTH,  /* Check if request passes basic auth. */
    COND_STRING_MATCH,/* String match. */
    COND_CLIENT_CERT,
    COND_DYN,
    COND_TBF,
    COND_LUA,
    COND_REF    /* Reference to detached condition. */
  };

struct dyn_service_cond
{
  struct bool_service_cond boolean;
  STRING *string;
  enum service_cond_type cond_type;
  int pat_type;
  int flags;
};

struct string_match
{
  STRING *string;
  GENPAT re;
};

struct user_pass
{
  SLIST_ENTRY (user_pass) link;
  char *pass;
  char user[1];
};

typedef SLIST_HEAD(,user_pass) USER_PASS_HEAD;

struct pass_file
{
  USER_PASS_HEAD head;
};

typedef struct tbf TBF;

struct tbf_cond
{
  STRING *key;
  TBF *tbf;
};

struct acl_cond
{
  ACL *acl;
  int forwarded;
};

typedef struct _service_cond
{
  enum service_cond_type type;
  STRING *tag;
  int decode;
  WATCHER *watcher;
  union
  {
    struct acl_cond acl;
    GENPAT re;
    struct bool_service_cond boolean;
    struct dyn_service_cond dyn;
    int ref;                 /* COND_REF */
    struct string_match sm;  /* COND_QUERY_PARAM and COND_STRING_MATCH */
    struct pass_file pwfile; /* COND_BASIC_AUTH */
    X509 *x509;              /* COND_CLIENT_CERT */
    struct pndlua_closure clua; /* COND_LUA */
    struct tbf_cond tbf;     /* COND_TBF */
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
      cond->boolean.op = BOOL_AND;
      SLIST_INIT (&cond->boolean.head);
      break;
    }
}

enum rewrite_type
  {
    REWRITE_REWRITE_RULE,
    REWRITE_HDR_DEL,
    REWRITE_HDR_SET,
    REWRITE_URL_SET,
    REWRITE_PATH_SET,
    REWRITE_QUERY_SET,
    REWRITE_QUERY_PARAM_SET,
    REWRITE_QUERY_DELETE,
    REWRITE_LUA
  };

typedef SLIST_HEAD(,rewrite_op) REWRITE_OP_HEAD;
typedef SLIST_HEAD(,rewrite_rule) REWRITE_RULE_HEAD;

typedef struct rewrite_op
{
  SLIST_ENTRY (rewrite_op) next; /* Next op in the list. */
  enum rewrite_type type;        /* Rewrite operation type. */
  int encode;                    /* Percent-encode value before setting. */
  union
  {
    struct rewrite_rule *rule;   /* type == REWRITE_REWRITE_RULE */
    MATCHER *hdrdel;             /* type == REWRITE_HDR_DEL */
    struct
    {
      char *name;
      char *value;
    } qp;                        /* type == REWRITE_QUERY_PARAM_SET */
    char *str;                   /* type == REWRITE_*_SET */
    struct pndlua_closure lua;   /* type == REWRITE_LUA */
  } v;
} REWRITE_OP;

typedef struct rewrite_rule
{
  SERVICE_COND cond;               /* Optional condition. */
  SLIST_ENTRY (rewrite_rule) next; /* Next rule in the list. */
  struct rewrite_rule *iffalse;    /* Branch to go if cond yields false. */
  REWRITE_OP_HEAD ophead;          /* Do this if cond yields true. */
} REWRITE_RULE;

typedef enum
  {
    BALANCER_ALGO_RANDOM,
    BALANCER_ALGO_IWRR,
  } BALANCER_ALGO;

#define PRI_MAX   65535

enum
  {
    REWRITE_REQUEST,
    REWRITE_RESPONSE
  };

struct iwrr_balancer
{
  int round;
  int max_pri;                  /* maximum priority */
  BACKEND *cur;
};

struct rand_balancer
{
  unsigned long sum_pri;	/* sum of priorities of active backends */
};

typedef struct balancer
{
  BALANCER_ALGO algo;
  int weight;                   /* relative weight among other balancers */
  unsigned act_num;             /* number of active backends */
  BACKEND_HEAD backends;
  DLIST_ENTRY (balancer) link;
  union
  {
    struct iwrr_balancer iwrr;
    struct rand_balancer rand;
  };
} BALANCER;

typedef DLIST_HEAD (,balancer) BALANCER_LIST;

/* service definition */
typedef struct _service
{
  char *name;			/* symbolic name */
  struct locus_range locus;     /* Location in the config file. */
  SERVICE_COND cond;
  REWRITE_RULE_HEAD rewrite[2];
  BALANCER_LIST balancers;
  BALANCER_ALGO balancer_algo;
  pthread_mutex_t mut;		/* mutex for this service */
  SESS_TYPE sess_type;
  unsigned sess_ttl;		/* session time-to-live */
  char *sess_id;                /* Session anchor ID */
  SESSION_TABLE *sessions;	/* currently active sessions */
  int disabled;			/* true if the service is disabled */
  int rewrite_errors;           /* Rewrite HTTP errors. */

  /* Logging */
  char *forwarded_header;       /* "forwarded" header name */
  ACL *trusted_ips;             /* Trusted IP addresses */
  int log_suppress_mask;        /* Suppress HTTP logging for these status
				   codes.  A bitmask. */

  /* Backend removal */
  BACKEND_HEAD be_rem_head;     /* List of backends scheduled for removal. */
  pthread_cond_t be_rem_cond;   /* Condition through which the removal thread
				   is notified that a backend from the list is
				   ready for removal.
				 */

  SLIST_ENTRY (_service) next;
} SERVICE;

typedef SLIST_HEAD (,_service) SERVICE_HEAD;

#define STATUS_MASK(s) (1 << ((s) / 100))

typedef struct _pound_ctx
{
  SSL_CTX *ctx;
  char *server_name;
  char **subjectAltNames;
  size_t subjectAltNameCount;
  SLIST_ENTRY (_pound_ctx) next;
} POUND_CTX;

typedef SLIST_HEAD (,_pound_ctx) POUND_CTX_HEAD;

/* HTTP logger */
#define MAX_HTTP_LOG_FORMATS 32

int http_log_format_compile (char const *name, char const *fmt,
			     void (*logfn) (void *, int, char const *, int),
			     void *logdata);
int http_log_format_find (char const *name);
int http_log_format_check (int n);

/* Additional listener options */
#define HDROPT_NONE              0   /* Nothing special */
#define HDROPT_FORWARDED_HEADERS 0x1 /* Add X-Forwarded headers */
#define HDROPT_SSL_HEADERS       0x2 /* Add X-SSL- headers */

/* Listener definition */
typedef struct _listener
{
  char *name;			/* symbolic name */
  struct locus_range locus;     /* Location in the config file. */
  struct addrinfo addr;		/* Socket address */
  int mode;                     /* File mode for AF_UNIX */
  int chowner;                  /* Change to effective owner, for AF_UNIX */
  int sock;			/* listening socket */
  POUND_CTX_HEAD ctx_head;	/* CTX for SSL connections */
  int clnt_check;		/* client verification mode */
  int noHTTPS11;		/* HTTP 1.1 mode for SSL */
  int header_options;           /* additional header options */
  REWRITE_RULE_HEAD rewrite[2];
  int rewrite_errors;           /* Rewrite HTTP errors. */
  int verb;			/* allowed HTTP verb group */
  unsigned to;			/* client time-out */
  GENPAT url_pat;	/* pattern to match the request URL against */
  struct http_errmsg *http_err[HTTP_STATUS_MAX];	/* error messages */
  unsigned linebufsize;         /* Line buffer size */
  CONTENT_LENGTH max_req_size;	/* max. request size */
  unsigned max_uri_length;      /* max. URI length */
  int rewr_loc;			/* rewrite location response */
  int rewr_dest;		/* rewrite destination header */
  int disabled;			/* true if the listener is disabled */
  int log_level;		/* log level for this listener */
  char *forwarded_header;       /* "forwarded" header name */
  ACL *trusted_ips;             /* Trusted IP addresses */
  int allow_client_reneg;	/* Allow Client SSL Renegotiation */
  SERVICE_HEAD services;
  SLIST_ENTRY (_listener) next;

  /* Used during configuration parsing */
  char *addr_str;
  char *port_str;
  int ssl_op_enable;
  int ssl_op_disable;
  int verify;
  int socket_from;
} LISTENER;

typedef SLIST_HEAD(,_listener) LISTENER_HEAD;

struct submatch
{
  size_t matchn;
  size_t matchmax;
  POUND_REGMATCH *matchv;
  char *subject;
  STRING *tag;
};

#define SUBMATCH_INITIALIZER { 0, 0, NULL, NULL, NULL }

#define SMQ_SIZE 8

struct submatch_queue
{
  int cur;
  struct submatch sm[SMQ_SIZE+1];
};

static inline void
submatch_init (struct submatch *sm)
{
  sm->matchn = 0;
  sm->matchmax = 0;
  sm->matchv = NULL;
  sm->subject = NULL;
}

static inline void submatch_queue_init (struct submatch_queue *smq) {
  memset (smq, 0, sizeof (*smq));
}
void submatch_queue_free (struct submatch_queue *smq);

enum
  {
    WSS_INIT = 0,
    WSS_REQ_GET = 0x01,
    WSS_REQ_HEADER_CONNECTION_UPGRADE = 0x02,
    WSS_REQ_HEADER_UPGRADE_WEBSOCKET = 0x04,

    WSS_RESP_101 = 0x08,
    WSS_RESP_HEADER_CONNECTION_UPGRADE = 0x10,
    WSS_RESP_HEADER_UPGRADE_WEBSOCKET = 0x20,
    WSS_COMPLETE = WSS_REQ_GET
    | WSS_REQ_HEADER_CONNECTION_UPGRADE
    | WSS_REQ_HEADER_UPGRADE_WEBSOCKET | WSS_RESP_101 |
    WSS_RESP_HEADER_CONNECTION_UPGRADE | WSS_RESP_HEADER_UPGRADE_WEBSOCKET
  };

/* Track SSL handshake/renegotiation so we can reject client-renegotiations. */
typedef enum
  {
    RENEG_INIT = 0,
    RENEG_REJECT,
    RENEG_ALLOW,
    RENEG_ABORT
  }
  RENEG_STATE;

typedef struct _pound_http
{
  /* Input parameters */
  int sock;
  LISTENER *lstn;
  struct addrinfo from_host;
  char *buffer; /* Line buffer, allocated after the structure. */

  /* Deduced information */
  SERVICE *svc;
  BACKEND *backend;

  /* Data used during http processing */
  BIO *cl;
  BIO *be;
  X509 *x509;
  SSL *ssl;
  struct submatch_queue smq;
  RENEG_STATE reneg_state;

  int ws_state;  /* Websocket state */
  int no_cont;   /* True if no content is expected */
  int conn_closed; /* True if the connection is closed */

  struct http_request request;
  struct http_request response;

  struct timespec start_req; /* Time when original request was received */
  struct timespec end_req;   /* Time after the response was sent */
  struct timespec be_start;  /* Time when the request was handed to the
				backend */

  int response_code;

  CONTENT_LENGTH res_bytes;

#if ENABLE_LUA
  char stash_init[2];
#endif

  SLIST_ENTRY(_pound_http) next;
} POUND_HTTP;

typedef SLIST_HEAD(,_pound_http) POUND_HTTP_HEAD;

SERVICE_COND *detached_cond (int n);

void stringbuf_store_ip (struct stringbuf *sb, POUND_HTTP *phttp, int fwd);
void http_log (POUND_HTTP *phttp);
struct addrinfo *get_remote_ip (POUND_HTTP *phttp, int forwarded,
				struct addrinfo **pres);
struct timespec pound_uptime (void);

/* add a request to the queue */
int pound_http_enqueue (int sock, LISTENER *lstn, struct sockaddr *sa, socklen_t salen);
/* get a request from the queue */
POUND_HTTP *pound_http_dequeue (void);
/* Free the argument */
void pound_http_destroy (POUND_HTTP *arg);
/* get the current queue length */
int get_thr_qlen (void);
/* Decrement number of active threads. */
void active_threads_decr (void);

/* handle HTTP requests */
void *thr_http (void *);

/* Log an error to the syslog or to stderr */
void logmsg (const int, const char *, ...)
  ATTR_PRINTFLIKE(2,3);
void abend (struct locus_range const *range, char const *fmt, ...)
  ATTR_PRINTFLIKE(2,3);

/* Translate inet/inet6 address into a string */
char *addr2str (char *, int, const struct addrinfo *, int);

/* Return a string representation for a back-end address */
char *str_be (char *buf, size_t size, BACKEND *be);

/* Find the right service for a request */
SERVICE *get_service (POUND_HTTP *);

/* Find the right back-end for a request */
BACKEND *get_backend (POUND_HTTP *phttp);

#ifdef ENABLE_DYNAMIC_BACKENDS
void backend_ref (BACKEND *be);
void backend_unref (BACKEND *be);
static inline void
backend_refcount_init (BACKEND *be)
{
  be->refcount = 1;
}
static inline int backend_referenced (BACKEND *be)
{
  return be->refcount > 1;
}
#else
# define backend_ref(be)
# define backend_unref(be)
# define backend_refcount_init(be)
# define backend_referenced(be) 1
#endif

void backend_matrix_to_regular (struct be_matrix *mtx, struct addrinfo *addr,
				struct be_regular *reg);
void backend_matrix_init (BACKEND *be);
void backend_matrix_disable (BACKEND *be, int disable_mode);

/* Search for a host name, return the addrinfo for it */
int get_host (char const *, struct addrinfo *, int);

/*
 * Find if a redirect needs rewriting
 * In general we have two possibilities that require it:
 * (1) if the redirect was done to the correct location with the wrong protocol
 * (2) if the redirect was done to the back-end rather than the listener
 */
int need_rewrite (const char *, const char *,
		  const LISTENER *, const BACKEND *, const char **);
/*
 * (for cookies only) possibly create session based on response headers
 */
void upd_session (SERVICE *, HTTP_HEADER_LIST *, BACKEND *);

#define BE_DISABLE  -1
#define BE_KILL     1
#define BE_ENABLE   0

/*
 * mark a backend host as dead;
 * do nothing if no resurrection code is active
 */
void kill_be (SERVICE *, BACKEND *, const int);

void service_session_remove_by_backend (SERVICE *svc, BACKEND *be);
void service_recompute_pri (SERVICE *svc,
			    BALANCER *bl,
			    void (*cb) (BACKEND *, void *),
			    void *data);
void service_recompute_pri_unlocked (SERVICE *svc,
				     void (*cb) (BACKEND *, void *),
				     void *data);
void balancer_recompute_pri_unlocked (BALANCER *bl,
					  void (*cb) (BACKEND *, void *),
					  void *data);

static inline void balancer_add_backend (BALANCER *bl, BACKEND *be)
{
  be->balancer = bl;
  DLIST_INSERT_TAIL (&bl->backends, be, link);
}

static inline void balancer_remove_backend (BALANCER *bl, BACKEND *be)
{
  be->balancer = NULL;
  DLIST_REMOVE (&bl->backends, be, link);
}

BALANCER *balancer_list_alloc (BALANCER_LIST *ml);
BALANCER *balancer_list_get (BALANCER_LIST *ml, int n, BALANCER_ALGO algo);

#define BALANCER_WEIGTH_MAX  65535

static inline BALANCER *
balancer_list_get_normal (BALANCER_LIST *ml)
{
  return balancer_list_get (ml, 0, BALANCER_ALGO_RANDOM);
}

static inline BALANCER *
balancer_list_get_emerg (BALANCER_LIST *ml)
{
  return balancer_list_get (ml, BALANCER_WEIGTH_MAX, BALANCER_ALGO_RANDOM);
}

/*
 * Non-blocking version of connect(2). Does the same as connect(2) but
 * ensures it will time-out after a much shorter time period CONN_TO.
 */
int connect_nb (const int, const struct addrinfo *, const int);

/*
 * Parse arguments/config file
 */
void config_parse (int, char **);
int config_parse_acl_file (ACL *acl, char const *filename, WORKDIR *wd);

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

char const *sess_type_to_str (int type);
int control_response_basic (POUND_HTTP *arg);
void pound_atexit (void (*func) (void *), void *arg);
int unlink_at_exit (char const *file_name);

extern char const *progname;

enum string_value_type
  {
    STRING_CONSTANT,
    STRING_INT,
    STRING_VARIABLE,
    STRING_FUNCTION,
    STRING_PRINTER
  };

struct string_value
{
  char const *kw;
  enum string_value_type type;
  union
  {
    char *s_const;
    char **s_var;
    int s_int;
    char const *(*s_func) (void);
    void (*s_print) (FILE *);
  } data;
};

void set_progname (char const *arg);
void print_version (struct string_value *settings);

typedef struct template *TEMPLATE;
struct json_value;

enum
  {
    TMPL_ERR_OK,    /* No error */
    TMPL_ERR_EOF,   /* Unexpected end of file */
    TMPL_ERR_RANGE, /* Number out of range */
    TMPL_ERR_NOFUNC,/* No such function */
    TMPL_ERR_BADTOK,/* Unexpected token */
    TMPL_ERR_NOTMPL,/* No such template */
  };

TEMPLATE template_lookup (const char *name);
int template_parse (char *text, TEMPLATE *ret_tmpl, size_t *end);
void template_run (TEMPLATE tmpl, struct json_value *val, FILE *outfile);
char const *template_strerror (int ec);
void template_free (TEMPLATE tmpl);

void errormsg (int ex, int ec, char const *fmt, ...)
  ATTR_PRINTFLIKE(3,4);
void json_error (struct json_value *val, char const *fmt, ...)
  ATTR_PRINTFLIKE(2,3);

int http_status_to_pound (int status);
int pound_to_http_status (int err);
char const *http_status_reason (int code);

struct json_value *workers_serialize (void);
struct json_value *pound_serialize (void);
int metrics_response (POUND_HTTP *phttp);

int match_cond (SERVICE_COND *cond, POUND_HTTP *phttp,
		struct http_request *req);

struct http_header *http_header_list_locate_name (HTTP_HEADER_LIST *head, char const *name, size_t len);
struct http_header *http_header_list_next (struct http_header *hdr);
char *http_header_get_value (struct http_header *hdr);


void service_lb_init (SERVICE *svc);
void service_lb_reset (SERVICE *svc, BACKEND *be);

FILE *fopen_wd (WORKDIR *wd, const char *filename);
FILE *fopen_include (const char *filename);
void fopen_error (int pri, int ec, WORKDIR *wd, const char *filename,
		  struct locus_range const *loc);
char *filename_resolve (const char *filename);

typedef int (*LISTENER_ITERATOR) (LISTENER *, void *);
int foreach_listener (LISTENER_ITERATOR itr, void *data);

typedef int (*SERVICE_ITERATOR) (SERVICE *, void *);
int foreach_service (SERVICE_ITERATOR itr, void *data);

typedef int (*BACKEND_ITERATOR) (BACKEND *, void *);
int foreach_backend (BACKEND_ITERATOR itr, void *data);

int basic_auth_read (void *obj, char const *filename, WORKDIR *wd);
void basic_auth_clear (void *obj);
int basic_auth (struct pass_file *pwf, struct http_request *req);

void combinable_header_add (char const *name);
int is_combinable_header (struct http_header *hdr);

TBF *tbf_alloc (uint64_t rate, unsigned burst);
int tbf_eval (TBF *env, char const *keyid);

#if ENABLE_LUA
static inline void phttp_lua_stash_reset (POUND_HTTP *p)
{
  p->stash_init[PNDLUA_CTX_GLOBAL] = p->stash_init[PNDLUA_CTX_THREAD] = 0;
}
int pndlua_init (void);
int pndlua_match (POUND_HTTP *phttp, struct pndlua_closure const *cond,
		  char **argv, void *data);
int pndlua_modify (POUND_HTTP *phttp, struct pndlua_closure const *cond,
		   char **argv, void *data);
int pndlua_backend (POUND_HTTP *phttp, struct pndlua_closure const *cond,
		    char **argv, void *data);
int pndlua_parse_config (void *call_data, void *section_data);
int pndlua_parse_closure (struct pndlua_closure *cond);
#else
# define phttp_lua_stash_reset(p)
static inline int pndlua_init (void) { return 0; }
static inline int
cfg_no_lua (void)
{
  conf_error ("%s", "this pound is compiled without support for Lua");
  return CFGPARSER_FAIL;
}
static inline int
pndlua_parse_config (void *call_data, void *section_data)
{
  return cfg_no_lua ();
}
static inline int
pndlua_parse_closure (struct pndlua_closure *cond)
{
  return cfg_no_lua ();
}
static inline int
pndlua_match (POUND_HTTP *phttp, struct pndlua_closure const *cond,
	      char **argv, void *data)
{
  return -1;
}
static inline int
pndlua_modify (POUND_HTTP *phttp, struct pndlua_closure const *cond,
	       char **argv, void *data)
{
  return -1;
}
static inline int
pndlua_backend (POUND_HTTP *phttp, struct pndlua_closure const *cond,
		char **argv, void *data)
{
  return -1;
}
#endif
