/*
 * Pound - the reverse-proxy load-balancer
 * Copyright (C) 2002-2007 Apsis GmbH
 *
 * This file is part of Pound.
 *
 * Pound is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Foobar is distributed in the hope that it will be useful,
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
 * Tel: +41-44-920 4904
 * EMail: roseg@apsis.ch
 */

#include    "config.h"
#include    <stdio.h>
#include    <math.h>

#if HAVE_STDLIB_H
#include    <stdlib.h>
#else
#error "Pound needs stdlib.h"
#endif

#if HAVE_UNISTD_H
#include    <unistd.h>
#else
#error "Pound needs unistd.h"
#endif

#if HAVE_GETOPT_H
#include    <getopt.h>
#endif

#if HAVE_PTHREAD_H
#include    <pthread.h>
#else
#error "Pound needs pthread.h"
#endif

#if HAVE_STRING_H
#include    <string.h>
#else
#error "Pound needs string.h"
#endif

#if TIME_WITH_SYS_TIME
#if HAVE_SYS_TIME_H
#include    <sys/time.h>
#else
#error "Pound needs sys/time.h"
#endif
#if HAVE_TIME_H
#include    <time.h>
#else
#error "Pound needs time.h"
#endif
#else   /* may not mix sys/time.h and time.h */
#if HAVE_SYS_TIME_H
#include    <sys/time.h>
#elif   HAVE_TIME_H
#include    <time.h>
#else
#error "Pound needs time.h"
#endif
#endif  /* mix */

#if HAVE_SYS_TYPES_H
#include    <sys/types.h>
#else
#error "Pound needs sys/types.h"
#endif

#if HAVE_SYS_SOCKET_H
#include    <sys/socket.h>
#else
#error "Pound needs sys/socket.h"
#endif

#if HAVE_SYS_UN_H
#include    <sys/un.h>
#else
#error "Pound needs sys/un.h"
#endif

#if HAVE_NETINET_IN_H
#include    <netinet/in.h>
#else
#error "Pound needs netinet/in.h"
#endif

#if HAVE_NETINET_TCP_H
#include    <netinet/tcp.h>
#else
#error "Pound needs netinet/tcp.h"
#endif

#if HAVE_ARPA_INET_H
#include    <arpa/inet.h>
#else
#error "Pound needs arpa/inet.h"
#endif

#if HAVE_NETDB_H
#include    <netdb.h>
#else
#error "Pound needs netdb.h"
#endif

#if HAVE_SYS_POLL_H
#include    <sys/poll.h>
#else
#error "Pound needs sys/poll.h"
#endif

#if HAVE_OPENSSL_SSL_H
#define OPENSSL_THREAD_DEFINES
#include    <openssl/ssl.h>
#include    <openssl/lhash.h>
#include    <openssl/err.h>
#if OPENSSL_VERSION_NUMBER >= 0x00907000L
#ifndef OPENSSL_THREADS
#error  "Pound requires OpenSSL with thread support"
#endif
#else
#ifndef THREADS
#error  "Pound requires OpenSSL with thread support"
#endif
#endif
#else
#error "Pound needs openssl/ssl.h"
#endif

#if HAVE_OPENSSL_ENGINE_H
#include    <openssl/engine.h>
#endif

#if HAVE_PWD_H
#include    <pwd.h>
#else
#error "Pound needs pwd.h"
#endif

#if HAVE_GRP_H
#include    <grp.h>
#else
#error "Pound needs grp.h"
#endif

#if HAVE_SYSLOG_H
#include    <syslog.h>
#endif

#if HAVE_SYS_SYSLOG_H
#include    <sys/syslog.h>
#endif

#if HAVE_SIGNAL_H
#include    <signal.h>
#else
#error "Pound needs signal.h"
#endif

#if HAVE_PCREPOSIX_H
#include    <pcreposix.h>
#elif HAVE_PCRE_PCREPOSIX
#include    <pcre/pcreposix.h>
#elif HAVE_REGEX_H
#include    <regex.h>
#else
#error "Pound needs regex.h"
#endif

#if HAVE_CTYPE_H
#include    <ctype.h>
#else
#error "Pound needs ctype.h"
#endif

#if HAVE_ERRNO_H
#include    <errno.h>
#else
#error "Pound needs errno.h"
#endif

#if HAVE_WAIT_H
#include    <wait.h>
#elif   HAVE_SYS_WAIT_H
#include    <sys/wait.h>
#else
#error "Pound needs sys/wait.h"
#endif

#if HAVE_SYS_STAT_H
#include    <sys/stat.h>
#else
#error "Pound needs sys/stat.h"
#endif

#if HAVE_FCNTL_H
#include    <fcntl.h>
#else
#error "Pound needs fcntl.h"
#endif

#if HAVE_STDARG_H
#include    <stdarg.h>
#else
#include    <varargs.h>
#endif

#ifndef __STDC__
#define const
#endif

#ifndef NO_EXTERNALS
/*
 * Global variables needed by everybody
 */

extern char *user,              /* user to run as */
            *group,             /* group to run as */
            *root_jail,         /* directory to chroot to */
            *pid_name,          /* file to record pid in */
            *ctrl_name;         /* control socket name */

extern int  alive_to,           /* check interval for resurrection */
            daemonize,          /* run as daemon */
            log_facility,       /* log facility to use */
            print_log,          /* print log messages to stdout/stderr */
            grace,              /* grace period before shutdown */
            control_sock;       /* control socket */

extern regex_t  HEADER,     /* Allowed header */
                CHUNK_HEAD, /* chunk header line */
                RESP_SKIP,  /* responses for which we skip response */
                RESP_IGN,   /* responses for which we ignore content */
                LOCATION,   /* the host we are redirected to */
                AUTHORIZATION;  /* the Authorisation header */

#ifndef  SOL_TCP
/* for systems without the definition */
extern int  SOL_TCP;
#endif

#endif /* NO_EXTERNALS */

#ifndef MAXBUF
#define MAXBUF      1024
#endif

#define MAXHEADERS  128

#ifndef F_CONF
#define F_CONF  "/usr/local/etc/pound.cfg"
#endif

#ifndef F_PID
#define F_PID  "/var/run/pound.pid"
#endif

/* matcher chain */
typedef struct _matcher {
    regex_t             pat;        /* pattern to match the request/header against */
    struct _matcher     *next;
}   MATCHER;

/* back-end types */
typedef enum    { SESS_NONE, SESS_IP, SESS_COOKIE, SESS_URL, SESS_PARM, SESS_HEADER, SESS_BASIC }   SESS_TYPE;

/* back-end definition */
typedef struct _backend {
    int                 be_type;    /* 0 if real back-end, otherwise code (301, 302/default, 307) */
    struct addrinfo     addr;       /* IPv4/6 address */
    int                 priority;   /* priority */
    int                 to;
    struct addrinfo     ha_addr;    /* HA address/port */
    char                *url;       /* for redirectors */
    int                 redir_req;  /* the redirect should include the request path */
    pthread_mutex_t     mut;        /* mutex for this back-end */
    int                 n_requests; /* number of requests seen */
    double              t_requests; /* time to answer these requests */
    double              t_average;  /* average time to answer requests */
    int                 alive;      /* false if the back-end is dead */
    int                 resurrect;  /* this back-end is to be resurrected */
    int                 disabled;   /* true if the back-end is disabled */
    struct _backend     *next;
}   BACKEND;

typedef struct _tn {
    char        *key;
    void        *content;
    time_t      last_acc;
}   TABNODE;

#define n_children(N)   ((N)? (N)->children: 0)

/* maximal session key size */
#define KEY_SIZE    127

/* service definition */
typedef struct _service {
    char                name[KEY_SIZE + 1]; /* symbolic name */
    MATCHER             *url,       /* request matcher */
                        *req_head,  /* required headers */
                        *deny_head; /* forbidden headers */
    BACKEND             *backends;
    BACKEND             *emergency;
    int                 abs_pri;    /* abs total priority for all back-ends */
    int                 tot_pri;    /* total priority for current back-ends */
    pthread_mutex_t     mut;        /* mutex for this service */
    SESS_TYPE           sess_type;
    int                 sess_ttl;   /* session time-to-live */
    regex_t             sess_pat;   /* pattern to match the session data */
    char                *sess_parm; /* session cookie or parameter */
    LHASH               *sessions;  /* currently active sessions */
    int                 dynscale;   /* true if the back-ends should be dynamically rescaled */
    int                 disabled;   /* true if the service is disabled */
    struct _service     *next;
}   SERVICE;

#ifndef NO_EXTERNALS
extern SERVICE          *services;  /* global services (if any) */
#endif /* NO_EXTERNALS */

/* Listener definition */
typedef struct _listener {
    struct addrinfo     addr;       /* IPv4/6 address */
    int                 sock;       /* listening socket */
    SSL_CTX             *ctx;       /* CTX for SSL connections */
    int                 clnt_check; /* client verification mode */
    int                 noHTTPS11;  /* HTTP 1.1 mode for SSL */
    char                *add_head;  /* extra SSL header */
    regex_t             verb;       /* pattern to match the request verb against */
    int                 to;         /* client time-out */
    int                 has_pat;    /* was a URL pattern defined? */
    regex_t             url_pat;    /* pattern to match the request URL against */
    char                *err414,    /* error messages */
                        *err500,
                        *err501,
                        *err503;
    long                max_req;    /* max. request size */
    MATCHER             *head_off;  /* headers to remove */
    int                 rewr_loc;   /* rewrite location response */
    int                 rewr_dest;  /* rewrite destination header */
    int                 disabled;   /* true if the listener is disabled */
    int                 log_level;  /* log level for this listener */
    SERVICE             *services;
    struct _listener    *next;
}   LISTENER;

#ifndef NO_EXTERNALS
extern LISTENER         *listeners; /* all available listeners */
#endif /* NO_EXTERNALS */

typedef struct  {
    int             sock;
    LISTENER        *lstn;
    struct addrinfo from_host;
}   thr_arg;                        /* argument to processing threads: socket, origin */

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

/* control request stuff */
typedef enum    {
    CTRL_LST,
    CTRL_EN_LSTN, CTRL_DE_LSTN,
    CTRL_EN_SVC, CTRL_DE_SVC,
    CTRL_EN_BE, CTRL_DE_BE,
    CTRL_ADD_SESS, CTRL_DEL_SESS
}   CTRL_CODE;

typedef struct  {
    CTRL_CODE   cmd;
    int         listener;
    int         service;
    int         backend;
    char        key[KEY_SIZE + 1];
}   CTRL_CMD;

#ifdef  NEED_INADDRT
/* for oldish Unices - normally this is in /usr/include/netinet/in.h */
typedef u_int32_t   in_addr_t;
#endif

#ifdef  NEED_INPORTT
/* for oldish Unices - normally this is in /usr/include/netinet/in.h */
typedef u_int16_t   in_port_t;
#endif

#ifdef  NEED_TIMET
/* for oldish Unices - normally this is in /usr/include/time.h */
typedef u_int32_t   time_t;
#endif

/*
 * handle an HTTP request
 */
extern void *thr_http(void *);

/*
 * Log an error to the syslog or to stderr
 */
extern void logmsg(const int, const char *, ...);

/*
 * Translate inet/inet6 address into a string
 */
extern void addr2str(char *, const int, const struct addrinfo *, const int);

/*
 * Return a string representation for a back-end address
 */
#define str_be(BUF, LEN, BE)    addr2str((BUF), (LEN), &(BE)->addr, 0)

/*
 * Find the right service for a request
 */
extern SERVICE  *get_service(const LISTENER *, const char *, char **const);

/*
 * Find the right back-end for a request
 */
extern BACKEND  *get_backend(SERVICE *const, const struct addrinfo *, const char *, char **const);

/*
 * Search for a host name, return the addrinfo for it
 */
extern int  get_host(char *const, struct addrinfo *);

/*
 * Find if a redirect needs rewriting
 * In general we have two possibilities that require it:
 * (1) if the redirect was done to the correct location with the wrong protocol
 * (2) if the redirect was done to the back-end rather than the listener
 */
extern int  need_rewrite(const int, char *const, char *const, const LISTENER *, const BACKEND *);
/*
 * (for cookies only) possibly create session based on response headers
 */
extern void upd_session(SERVICE *const, char **const, BACKEND *const);

/*
 * Parse a header
 */
extern int  check_header(const char *, char *);

#define BE_DISABLE  -1
#define BE_KILL     1
#define BE_ENABLE   0
/*
 * mark a backend host as dead;
 * do nothing if no resurection code is active
 */
extern void kill_be(SERVICE *const, const BACKEND *, const int);

/*
 * Rescale back-end priorities if needed
 * runs every 5 minutes
 */
#ifndef RESCALE_TO
#define RESCALE_TO  300
#endif

/*
 * Dynamic rescaling constants
 */
#define RESCALE_MAX 32000
#define RESCALE_MIN 8000
#define RESCALE_BOT 4000

/*
 * Update the number of requests and time to answer for a given back-end
 */
extern void upd_be(SERVICE *const svc, BACKEND *const be, const double);

/*
 * Non-blocking version of connect(2). Does the same as connect(2) but
 * ensures it will time-out after a much shorter time period CONN_TO.
 */
extern int  connect_nb(const int, const struct addrinfo *, const int);

/*
 * Parse arguments/config file
 */
extern void config_parse(const int, char **const);

/*
 * RSA ephemeral keys: how many and how often
 */
#define N_RSA_KEYS  11
#ifndef T_RSA_KEYS
#define T_RSA_KEYS  300
#endif

/*
 * return a pre-generated RSA key
 */
extern RSA  *RSA_tmp_callback(SSL *, int, int);

/*
 * expiration stuff
 */
#ifndef EXPIRE_TO
#define EXPIRE_TO   60
#endif

#ifndef HOST_TO
#define HOST_TO     300
#endif

/*
 * initialise the timer functions:
 *  - host_mut
 *  - RSA_mut and keys
 */
extern void init_timer(void);

/*
 * run timed functions:
 *  - RSAgen every T_RSA_KEYS seconds
 *  - rescale every RESCALE_TO seconds
 *  - resurrect every alive_to seconds
 *  - expire every EXPIRE_TO seconds
 */
extern void *thr_timer(void *);

/*
 * The controlling thread
 * listens to client requests and calls the appropriate functions
 */
extern void *thr_control(void *);
