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

#ifndef MISS_FACILITYNAMES
#define SYSLOG_NAMES    1
#endif

#include    "pound.h"

#ifdef MISS_FACILITYNAMES

/* This is lifted verbatim from the Linux sys/syslog.h */

typedef struct _code {
	char	*c_name;
	int	c_val;
} CODE;

static CODE facilitynames[] = {
    { "auth", LOG_AUTH },
#ifdef  LOG_AUTHPRIV
    { "authpriv", LOG_AUTHPRIV },
#endif
    { "cron", LOG_CRON },
    { "daemon", LOG_DAEMON },
#ifdef  LOG_FTP
    { "ftp", LOG_FTP },
#endif
    { "kern", LOG_KERN },
    { "lpr", LOG_LPR },
    { "mail", LOG_MAIL },
    { "mark", 0 },                  /* never used! */
    { "news", LOG_NEWS },
    { "security", LOG_AUTH },       /* DEPRECATED */
    { "syslog", LOG_SYSLOG },
    { "user", LOG_USER },
    { "uucp", LOG_UUCP },
    { "local0", LOG_LOCAL0 },
    { "local1", LOG_LOCAL1 },
    { "local2", LOG_LOCAL2 },
    { "local3", LOG_LOCAL3 },
    { "local4", LOG_LOCAL4 },
    { "local5", LOG_LOCAL5 },
    { "local6", LOG_LOCAL6 },
    { "local7", LOG_LOCAL7 },
    { NULL, -1 }
};
#endif

static regex_t  Empty, Comment, User, Group, RootJail, Daemon, LogFacility, LogLevel, Alive, SSLEngine, Control;
static regex_t  ListenHTTP, ListenHTTPS, End, Address, Port, Cert, xHTTP, Client, CheckURL;
static regex_t  Err414, Err500, Err501, Err503, MaxRequest, HeadRemove, RewriteLocation, RewriteDestination;
static regex_t  Service, ServiceName, URL, HeadRequire, HeadDeny, BackEnd, Emergency, Priority, HAport, HAportAddr;
static regex_t  Redirect, RedirectN, TimeOut, Session, Type, TTL, ID, DynScale;
static regex_t  ClientCert, AddHeader, Ciphers, CAlist, VerifyList, CRLlist, NoHTTPS11;
static regex_t  Grace;

static regmatch_t   matches[5];

static char *xhttp[] = {
    "^(GET|POST|HEAD) ([^ ]+) HTTP/1.[01]$",
    "^(GET|POST|HEAD|PUT|DELETE) ([^ ]+) HTTP/1.[01]$",
    "^(GET|POST|HEAD|PUT|DELETE|LOCK|UNLOCK|PROPFIND|PROPPATCH|SEARCH|MKCOL|MOVE|COPY|OPTIONS|TRACE|MKACTIVITY|CHECKOUT|MERGE|REPORT) ([^ ]+) HTTP/1.[01]$",
    "^(GET|POST|HEAD|PUT|DELETE|LOCK|UNLOCK|PROPFIND|PROPPATCH|SEARCH|MKCOL|MOVE|COPY|OPTIONS|TRACE|MKACTIVITY|CHECKOUT|MERGE|REPORT|SUBSCRIBE|BPROPPATCH|POLL|BMOVE|BCOPY|BDELETE|CONNECT) ([^ ]+) HTTP/1.[01]$",
    "^(GET|POST|HEAD|PUT|DELETE|LOCK|UNLOCK|PROPFIND|PROPPATCH|SEARCH|MKCOL|MOVE|COPY|OPTIONS|TRACE|MKACTIVITY|CHECKOUT|MERGE|REPORT|SUBSCRIBE|BPROPPATCH|POLL|BMOVE|BCOPY|BDELETE|CONNECT|RPC_IN_DATA|RPC_OUT_DATA) ([^ ]+) HTTP/1.[01]$",
};

static int  log_level = 1;
static int  def_facility = LOG_DAEMON;
static int  clnt_to = 10;
static int  be_to = 15;
static int  n_lin = 0;
static int  dynscale = 0;

/*
 * parse a back-end
 */
static BACKEND *
parse_be(FILE *const f_conf, const int is_emergency)
{
    char        lin[MAXBUF];
    BACKEND     *res;
    int         has_addr, has_port;
    struct hostent      *host;
    struct sockaddr_in  in;
    struct sockaddr_in6 in6;

    if((res = (BACKEND *)malloc(sizeof(BACKEND))) == NULL) {
        logmsg(LOG_ERR, "line %d: BackEnd config: out of memory - aborted", n_lin);
        exit(1);
    }
    memset(res, 0, sizeof(BACKEND));
    res->be_type = 0;
    res->addr.ai_socktype = SOCK_STREAM;
    res->to = is_emergency? 120: be_to;
    res->alive = 1;
    memset(&res->addr, 0, sizeof(res->addr));
    res->priority = 5;
    memset(&res->ha_addr, 0, sizeof(res->ha_addr));
    res->url = NULL;
    res->next = NULL;
    has_addr = has_port = 0;
    pthread_mutex_init(&res->mut, NULL);
    while(fgets(lin, MAXBUF, f_conf)) {
        n_lin++;
        if(strlen(lin) > 0 && lin[strlen(lin) - 1] == '\n')
            lin[strlen(lin) - 1] = '\0';
        if(!regexec(&Empty, lin, 4, matches, 0) || !regexec(&Comment, lin, 4, matches, 0)) {
            /* comment or empty line */
            continue;
        } else if(!regexec(&Address, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            if(get_host(lin + matches[1].rm_so, &res->addr)) {
                /* if we can't resolve it assume this is a UNIX domain socket */
                res->addr.ai_socktype = SOCK_STREAM;
                res->addr.ai_family = AF_UNIX;
                res->addr.ai_protocol = 0;
                if((res->addr.ai_addr = (struct sockaddr *)strdup(lin + matches[1].rm_so)) == NULL) {
                    logmsg(LOG_ERR, "line %d: out of memory", n_lin);
                    exit(1);
                }
                res->addr.ai_addrlen = strlen(lin + matches[1].rm_so) + 1;
            }
            has_addr = 1;
        } else if(!regexec(&Port, lin, 4, matches, 0)) {
            switch(res->addr.ai_family) {
            case AF_INET:
                memcpy(&in, res->addr.ai_addr, sizeof(in));
                in.sin_port = (in_port_t)htons(atoi(lin + matches[1].rm_so));
                memcpy(res->addr.ai_addr, &in, sizeof(in));
                break;
            case AF_INET6:
                memcpy(&in6, res->addr.ai_addr, sizeof(in6));
                in6.sin6_port = (in_port_t)htons(atoi(lin + matches[1].rm_so));
                memcpy(res->addr.ai_addr, &in6, sizeof(in6));
                break;
            default:
                logmsg(LOG_ERR, "line %d: Port is supported only for INET/INET6 back-ends", n_lin);
                exit(1);
            }
            has_port = 1;
        } else if(!regexec(&Priority, lin, 4, matches, 0)) {
            if(is_emergency) {
                logmsg(LOG_ERR, "line %d: Priority is not supported for Emergency back-ends", n_lin);
                exit(1);
            }
            res->priority = atoi(lin + matches[1].rm_so);
        } else if(!regexec(&TimeOut, lin, 4, matches, 0)) {
            res->to = atoi(lin + matches[1].rm_so);
        } else if(!regexec(&HAport, lin, 4, matches, 0)) {
            if(is_emergency) {
                logmsg(LOG_ERR, "line %d: HAport is not supported for Emergency back-ends", n_lin);
                exit(1);
            }
            res->ha_addr = res->addr;
            if((res->ha_addr.ai_addr = (struct sockaddr *)malloc(res->addr.ai_addrlen)) == NULL) {
                logmsg(LOG_ERR, "line %d: out of memory", n_lin);
                exit(1);
            }
            memcpy(res->ha_addr.ai_addr, res->addr.ai_addr, res->addr.ai_addrlen);
            switch(res->addr.ai_family) {
            case AF_INET:
                memcpy(&in, res->ha_addr.ai_addr, sizeof(in));
                in.sin_port = (in_port_t)htons(atoi(lin + matches[1].rm_so));
                memcpy(res->ha_addr.ai_addr, &in, sizeof(in));
                break;
            case AF_INET6:
                memcpy(&in6, res->addr.ai_addr, sizeof(in6));
                in6.sin6_port = (in_port_t)htons(atoi(lin + matches[1].rm_so));
                memcpy(res->addr.ai_addr, &in6, sizeof(in6));
                break;
            default:
                logmsg(LOG_ERR, "line %d: HAport is supported only for INET/INET6 back-ends", n_lin);
                exit(1);
            }
        } else if(!regexec(&HAportAddr, lin, 4, matches, 0)) {
            if(is_emergency) {
                logmsg(LOG_ERR, "line %d: HAportAddr is not supported for Emergency back-ends", n_lin);
                exit(1);
            }
            lin[matches[1].rm_eo] = '\0';
            if(get_host(lin + matches[1].rm_so, &res->ha_addr)) {
                /* if we can't resolve it assume this is a UNIX domain socket */
                res->addr.ai_socktype = SOCK_STREAM;
                res->ha_addr.ai_family = AF_UNIX;
                res->ha_addr.ai_protocol = 0;
                if((res->ha_addr.ai_addr = (struct sockaddr *)strdup(lin + matches[1].rm_so)) == NULL) {
                    logmsg(LOG_ERR, "line %d: out of memory", n_lin);
                    exit(1);
                }
                res->addr.ai_addrlen = strlen(lin + matches[1].rm_so) + 1;
            } else switch(res->ha_addr.ai_family) {
            case AF_INET:
                memcpy(&in, res->ha_addr.ai_addr, sizeof(in));
                in.sin_port = (in_port_t)htons(atoi(lin + matches[2].rm_so));
                memcpy(res->ha_addr.ai_addr, &in, sizeof(in));
                break;
            case AF_INET6:
                memcpy(&in6, res->ha_addr.ai_addr, sizeof(in6));
                in6.sin6_port = (in_port_t)htons(atoi(lin + matches[2].rm_so));
                memcpy(res->ha_addr.ai_addr, &in6, sizeof(in6));
                break;
            default:
                logmsg(LOG_ERR, "line %d: Unknown HA address type", n_lin);
                exit(1);
            }
        } else if(!regexec(&End, lin, 4, matches, 0)) {
            if(!has_addr) {
                logmsg(LOG_ERR, "line %d: BackEnd missing Address - aborted", n_lin);
                exit(1);
            }
            if((res->addr.ai_family == AF_INET || res->addr.ai_family == AF_INET6) && !has_port) {
                logmsg(LOG_ERR, "line %d: BackEnd missing Port - aborted", n_lin);
                exit(1);
            }
            return res;
        } else {
            logmsg(LOG_ERR, "line %d: unknown directive \"%s\" - aborted", n_lin, lin);
            exit(1);
        }
    }

    logmsg(LOG_ERR, "line %d: BackEnd premature EOF", n_lin);
    exit(1);
    return NULL;
}

/*
 * parse a session
 */
static void
parse_sess(FILE *const f_conf, SERVICE *const svc)
{
    char        lin[MAXBUF], *cp;

    while(fgets(lin, MAXBUF, f_conf)) {
        n_lin++;
        if(strlen(lin) > 0 && lin[strlen(lin) - 1] == '\n')
            lin[strlen(lin) - 1] = '\0';
        if(!regexec(&Empty, lin, 4, matches, 0) || !regexec(&Comment, lin, 4, matches, 0)) {
            /* comment or empty line */
            continue;
        } else if(!regexec(&Type, lin, 4, matches, 0)) {
            if(svc->sess_type != SESS_NONE) {
                logmsg(LOG_ERR, "line %d: Multiple Session types in one Service - aborted", n_lin);
                exit(1);
            }
            lin[matches[1].rm_eo] = '\0';
            cp = lin + matches[1].rm_so;
            if(!strcasecmp(cp, "IP"))
                svc->sess_type = SESS_IP;
            else if(!strcasecmp(cp, "COOKIE"))
                svc->sess_type = SESS_COOKIE;
            else if(!strcasecmp(cp, "URL"))
                svc->sess_type = SESS_URL;
            else if(!strcasecmp(cp, "PARM"))
                svc->sess_type = SESS_PARM;
            else if(!strcasecmp(cp, "BASIC"))
                svc->sess_type = SESS_BASIC;
            else if(!strcasecmp(cp, "HEADER"))
                svc->sess_type = SESS_HEADER;
            else {
                logmsg(LOG_ERR, "line %d: Unknown Session type \"%s\" - aborted", n_lin, cp);
                exit(1);
            }
        } else if(!regexec(&TTL, lin, 4, matches, 0)) {
            svc->sess_ttl = atoi(lin + matches[1].rm_so);
        } else if(!regexec(&ID, lin, 4, matches, 0)) {
            if(svc->sess_type != SESS_COOKIE && svc->sess_type != SESS_URL && svc->sess_type != SESS_HEADER) {
                logmsg(LOG_ERR, "line %d: no ID permitted unless COOKIE/URL/HEADER Session - aborted", n_lin);
                exit(1);
            }
            lin[matches[1].rm_eo] = '\0';
            if((svc->sess_parm = strdup(lin + matches[1].rm_so)) == NULL) {
                logmsg(LOG_ERR, "line %d: ID config: out of memory - aborted", n_lin);
                exit(1);
            }
        } else if(!regexec(&End, lin, 4, matches, 0)) {
            if(svc->sess_type == SESS_NONE) {
                logmsg(LOG_ERR, "line %d: Session type not defined - aborted", n_lin);
                exit(1);
            }
            if(svc->sess_ttl == 0) {
                logmsg(LOG_ERR, "line %d: Session TTL not defined - aborted", n_lin);
                exit(1);
            }
            if((svc->sess_type == SESS_COOKIE || svc->sess_type == SESS_URL || svc->sess_type == SESS_HEADER)
            && svc->sess_parm == NULL) {
                logmsg(LOG_ERR, "line %d: Session ID not defined - aborted", n_lin);
                exit(1);
            }
            if(svc->sess_type == SESS_COOKIE) {
                snprintf(lin, MAXBUF - 1, "Cookie[^:]*:.*[ \t]%s=([^;]*)", svc->sess_parm);
                if(regcomp(&svc->sess_pat, lin, REG_ICASE | REG_NEWLINE | REG_EXTENDED)) {
                    logmsg(LOG_ERR, "line %d: COOKIE pattern \"%s\" failed - aborted", n_lin, lin);
                    exit(1);
                }
            } else if(svc->sess_type == SESS_URL) {
                snprintf(lin, MAXBUF - 1, "[?&]%s=([^&;#]*)", svc->sess_parm);
                if(regcomp(&svc->sess_pat, lin, REG_ICASE | REG_NEWLINE | REG_EXTENDED)) {
                    logmsg(LOG_ERR, "line %d: URL pattern \"%s\" failed - aborted", n_lin, lin);
                    exit(1);
                }
            } else if(svc->sess_type == SESS_PARM) {
                snprintf(lin, MAXBUF - 1, ";([^?]*)");
                if(regcomp(&svc->sess_pat, lin, REG_ICASE | REG_NEWLINE | REG_EXTENDED)) {
                    logmsg(LOG_ERR, "line %d: PARM pattern \"%s\" failed - aborted", n_lin, lin);
                    exit(1);
                }
            } else if(svc->sess_type == SESS_BASIC) {
                snprintf(lin, MAXBUF - 1, "Authorization:[ \t]*Basic[ \t]*([^ \t]*)[ \t]*");
                if(regcomp(&svc->sess_pat, lin, REG_ICASE | REG_NEWLINE | REG_EXTENDED)) {
                    logmsg(LOG_ERR, "line %d: BASIC pattern \"%s\" failed - aborted", n_lin, lin);
                    exit(1);
                }
            } else if(svc->sess_type == SESS_HEADER) {
                snprintf(lin, MAXBUF - 1, "%s:[ \t]*([^ \t]*)[ \t]*", svc->sess_parm);
                if(regcomp(&svc->sess_pat, lin, REG_ICASE | REG_NEWLINE | REG_EXTENDED)) {
                    logmsg(LOG_ERR, "line %d: HEADER pattern \"%s\" failed - aborted", n_lin, lin);
                    exit(1);
                }
            }
            return;
        } else {
            logmsg(LOG_ERR, "line %d: unknown directive \"%s\" - aborted", n_lin, lin);
            exit(1);
        }
    }

    logmsg(LOG_ERR, "line %d: Session premature EOF", n_lin);
    exit(1);
    return;
}

/*
 * basic hashing function, based on fmv
 */
static unsigned long
t_hash(const TABNODE *e)
{
    unsigned long   res;
    char            *k;

    k = e->key;
    res = 2166136261;
    while(*k)
        res = (res ^ *k++) * 16777619;
    return res;
}
static IMPLEMENT_LHASH_HASH_FN(t_hash, const TABNODE *)

static int
t_cmp(const TABNODE *d1, const TABNODE *d2)
{
    return strcmp(d1->key, d2->key);
}
static IMPLEMENT_LHASH_COMP_FN(t_cmp, const TABNODE *)

/*
 * parse a service
 */
static SERVICE *
parse_service(FILE *const f_conf, const char *svc_name)
{
    char        lin[MAXBUF];
    SERVICE     *res;
    BACKEND     *be;
    MATCHER     *m;

    if((res = (SERVICE *)malloc(sizeof(SERVICE))) == NULL) {
        logmsg(LOG_ERR, "line %d: Service config: out of memory - aborted", n_lin);
        exit(1);
    }
    memset(res, 0, sizeof(SERVICE));
    res->sess_type = SESS_NONE;
    res->dynscale = dynscale;
    pthread_mutex_init(&res->mut, NULL);
    if(svc_name)
        strncpy(res->name, svc_name, KEY_SIZE);
    if((res->sessions = lh_new(LHASH_HASH_FN(t_hash), LHASH_COMP_FN(t_cmp))) == NULL) {
        logmsg(LOG_ERR, "line %d: lh_new failed - aborted", n_lin);
        exit(1);
    }
    while(fgets(lin, MAXBUF, f_conf)) {
        n_lin++;
        if(strlen(lin) > 0 && lin[strlen(lin) - 1] == '\n')
            lin[strlen(lin) - 1] = '\0';
        if(!regexec(&Empty, lin, 4, matches, 0) || !regexec(&Comment, lin, 4, matches, 0)) {
            /* comment or empty line */
            continue;
        } else if(!regexec(&URL, lin, 4, matches, 0)) {
            if(res->url) {
                for(m = res->url; m->next; m = m->next)
                    ;
                if((m->next = (MATCHER *)malloc(sizeof(MATCHER))) == NULL) {
                    logmsg(LOG_ERR, "line %d: URL config: out of memory - aborted", n_lin);
                    exit(1);
                }
                m = m->next;
            } else {
                if((res->url = (MATCHER *)malloc(sizeof(MATCHER))) == NULL) {
                    logmsg(LOG_ERR, "line %d: URL config: out of memory - aborted", n_lin);
                    exit(1);
                }
                m = res->url;
            }
            memset(m, 0, sizeof(MATCHER));
            lin[matches[1].rm_eo] = '\0';
            if(regcomp(&m->pat, lin + matches[1].rm_so, REG_NEWLINE | REG_EXTENDED)) {
                logmsg(LOG_ERR, "line %d: URL bad pattern \"%s\" - aborted", n_lin, lin + matches[1].rm_so);
                exit(1);
            }
        } else if(!regexec(&HeadRequire, lin, 4, matches, 0)) {
            if(res->req_head) {
                for(m = res->req_head; m->next; m = m->next)
                    ;
                if((m->next = (MATCHER *)malloc(sizeof(MATCHER))) == NULL) {
                    logmsg(LOG_ERR, "line %d: HeadRequire config: out of memory - aborted", n_lin);
                    exit(1);
                }
                m = m->next;
            } else {
                if((res->req_head = (MATCHER *)malloc(sizeof(MATCHER))) == NULL) {
                    logmsg(LOG_ERR, "line %d: HeadRequire config: out of memory - aborted", n_lin);
                    exit(1);
                }
                m = res->req_head;
            }
            memset(m, 0, sizeof(MATCHER));
            lin[matches[1].rm_eo] = '\0';
            if(regcomp(&m->pat, lin + matches[1].rm_so, REG_ICASE | REG_NEWLINE | REG_EXTENDED)) {
                logmsg(LOG_ERR, "line %d: HeadRequire bad pattern \"%s\" - aborted", n_lin, lin + matches[1].rm_so);
                exit(1);
            }
        } else if(!regexec(&HeadDeny, lin, 4, matches, 0)) {
            if(res->deny_head) {
                for(m = res->deny_head; m->next; m = m->next)
                    ;
                if((m->next = (MATCHER *)malloc(sizeof(MATCHER))) == NULL) {
                    logmsg(LOG_ERR, "line %d: HeadDeny config: out of memory - aborted", n_lin);
                    exit(1);
                }
                m = m->next;
            } else {
                if((res->deny_head = (MATCHER *)malloc(sizeof(MATCHER))) == NULL) {
                    logmsg(LOG_ERR, "line %d: HeadDeny config: out of memory - aborted", n_lin);
                    exit(1);
                }
                m = res->deny_head;
            }
            memset(m, 0, sizeof(MATCHER));
            lin[matches[1].rm_eo] = '\0';
            if(regcomp(&m->pat, lin + matches[1].rm_so, REG_ICASE | REG_NEWLINE | REG_EXTENDED)) {
                logmsg(LOG_ERR, "line %d: HeadDeny bad pattern \"%s\" - aborted", n_lin, lin + matches[1].rm_so);
                exit(1);
            }
        } else if(!regexec(&Redirect, lin, 4, matches, 0)) {
            if(res->backends) {
                for(be = res->backends; be->next; be = be->next)
                    ;
                if((be->next = (BACKEND *)malloc(sizeof(BACKEND))) == NULL) {
                    logmsg(LOG_ERR, "line %d: Redirect config: out of memory - aborted", n_lin);
                    exit(1);
                }
                be = be->next;
            } else {
                if((res->backends = (BACKEND *)malloc(sizeof(BACKEND))) == NULL) {
                    logmsg(LOG_ERR, "line %d: Redirect config: out of memory - aborted", n_lin);
                    exit(1);
                }
                be = res->backends;
            }
            memset(be, 0, sizeof(BACKEND));
            be->be_type = 302;
            be->priority = 1;
            be->alive = 1;
            pthread_mutex_init(&res->mut, NULL);
            lin[matches[1].rm_eo] = '\0';
            if((be->url = strdup(lin + matches[1].rm_so)) == NULL) {
                logmsg(LOG_ERR, "line %d: Redirector config: out of memory - aborted", n_lin);
                exit(1);
            }
            /* split the URL into its fields */
            if(regexec(&LOCATION, be->url, 4, matches, 0)) {
                logmsg(LOG_ERR, "line %d: Redirect bad URL \"%s\" - aborted", n_lin, be->url);
                exit(1);
            }
            if(be->url[matches[3].rm_so] == '/')
                matches[3].rm_so++;
            /* if the path component is empty or a sigle slash */
            be->redir_req = ((matches[3].rm_eo - matches[3].rm_so) < 1);
        } else if(!regexec(&RedirectN, lin, 4, matches, 0)) {
            if(res->backends) {
                for(be = res->backends; be->next; be = be->next)
                    ;
                if((be->next = (BACKEND *)malloc(sizeof(BACKEND))) == NULL) {
                    logmsg(LOG_ERR, "line %d: Redirect config: out of memory - aborted", n_lin);
                    exit(1);
                }
                be = be->next;
            } else {
                if((res->backends = (BACKEND *)malloc(sizeof(BACKEND))) == NULL) {
                    logmsg(LOG_ERR, "line %d: Redirect config: out of memory - aborted", n_lin);
                    exit(1);
                }
                be = res->backends;
            }
            memset(be, 0, sizeof(BACKEND));
            be->be_type = atoi(lin + matches[1].rm_so);
            be->priority = 1;
            be->alive = 1;
            pthread_mutex_init(&res->mut, NULL);
            lin[matches[2].rm_eo] = '\0';
            if((be->url = strdup(lin + matches[2].rm_so)) == NULL) {
                logmsg(LOG_ERR, "line %d: Redirector config: out of memory - aborted", n_lin);
                exit(1);
            }
            /* split the URL into its fields */
            if(regexec(&LOCATION, be->url, 4, matches, 0)) {
                logmsg(LOG_ERR, "line %d: Redirect bad URL \"%s\" - aborted", n_lin, be->url);
                exit(1);
            }
            if(be->url[matches[3].rm_so] == '/')
                matches[3].rm_so++;
            /* if the path component is empty or a sigle slash */
            be->redir_req = ((matches[3].rm_eo - matches[3].rm_so) < 1);
        } else if(!regexec(&BackEnd, lin, 4, matches, 0)) {
            if(res->backends) {
                for(be = res->backends; be->next; be = be->next)
                    ;
                be->next = parse_be(f_conf, 0);
            } else
                res->backends = parse_be(f_conf, 0);
        } else if(!regexec(&Emergency, lin, 4, matches, 0)) {
            res->emergency = parse_be(f_conf, 1);
        } else if(!regexec(&Session, lin, 4, matches, 0)) {
            parse_sess(f_conf, res);
        } else if(!regexec(&End, lin, 4, matches, 0)) {
            for(be = res->backends; be; be = be->next)
                res->tot_pri += be->priority;
            res->abs_pri = res->tot_pri;
            return res;
        } else if(!regexec(&DynScale, lin, 4, matches, 0)) {
            res->dynscale = atoi(lin + matches[1].rm_so);
        } else {
            logmsg(LOG_ERR, "line %d: unknown directive \"%s\" - aborted", n_lin, lin);
            exit(1);
        }
    }

    logmsg(LOG_ERR, "line %d: Service premature EOF", n_lin);
    exit(1);
    return NULL;
}

/*
 * return the file contents as a string
 */
static char *
file2str(const char *fname)
{
    char    *res;
    struct stat st;
    int     fin;

    if(stat(fname, &st)) {
        logmsg(LOG_ERR, "line %d: can't stat Err file \"%s\" (%s) - aborted", n_lin, fname, strerror(errno));
        exit(1);
    }
    if((fin = open(fname, O_RDONLY)) < 0) {
        logmsg(LOG_ERR, "line %d: can't open Err file \"%s\" (%s) - aborted", n_lin, fname, strerror(errno));
        exit(1);
    }
    if((res = malloc(st.st_size + 1)) == NULL) {
        logmsg(LOG_ERR, "line %d: can't alloc Err file \"%s\" (out of memory) - aborted", n_lin, fname);
        exit(1);
    }
    if(read(fin, res, st.st_size) != st.st_size) {
        logmsg(LOG_ERR, "line %d: can't read Err file \"%s\" (%s) - aborted", n_lin, fname, strerror(errno));
        exit(1);
    }
    res[st.st_size] = '\0';
    close(fin);
    return res;
}

/*
 * parse an HTTP listener
 */
static LISTENER *
parse_HTTP(FILE *const f_conf)
{
    char        lin[MAXBUF];
    LISTENER    *res;
    SERVICE     *svc;
    MATCHER     *m;
    int         has_addr, has_port;
    struct sockaddr_in  in;
    struct sockaddr_in6 in6;

    if((res = (LISTENER *)malloc(sizeof(LISTENER))) == NULL) {
        logmsg(LOG_ERR, "line %d: ListenHTTP config: out of memory - aborted", n_lin);
        exit(1);
    }
    memset(res, 0, sizeof(LISTENER));
    res->to = clnt_to;
    res->rewr_loc = 1;
    res->err414 = "Request URI is too long";
    res->err500 = "An internal server error occurred. Please try again later.";
    res->err501 = "This method may not be used.";
    res->err503 = "The service is not available. Please try again later.";
    res->log_level = log_level;
    if(regcomp(&res->verb, xhttp[0], REG_ICASE | REG_NEWLINE | REG_EXTENDED)) {
        logmsg(LOG_ERR, "line %d: xHTTP bad default pattern - aborted", n_lin);
        exit(1);
    }
    has_addr = has_port = 0;
    while(fgets(lin, MAXBUF, f_conf)) {
        n_lin++;
        if(strlen(lin) > 0 && lin[strlen(lin) - 1] == '\n')
            lin[strlen(lin) - 1] = '\0';
        if(!regexec(&Empty, lin, 4, matches, 0) || !regexec(&Comment, lin, 4, matches, 0)) {
            /* comment or empty line */
            continue;
        } else if(!regexec(&Address, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            if(get_host(lin + matches[1].rm_so, &res->addr)) {
                logmsg(LOG_ERR, "line %d: Unknown Listener address \"%s\"", n_lin, lin + matches[1].rm_so);
                exit(1);
            }
            if(res->addr.ai_family != AF_INET && res->addr.ai_family != AF_INET6) {
                logmsg(LOG_ERR, "line %d: Unknown Listener address family %d", n_lin, res->addr.ai_family);
                exit(1);
            }
            has_addr = 1;
        } else if(!regexec(&Port, lin, 4, matches, 0)) {
            switch(res->addr.ai_family) {
            case AF_INET:
                memcpy(&in, res->addr.ai_addr, sizeof(in));
                in.sin_port = (in_port_t)htons(atoi(lin + matches[1].rm_so));
                memcpy(res->addr.ai_addr, &in, sizeof(in));
                break;
            case AF_INET6:
                memcpy(&in6, res->addr.ai_addr, sizeof(in6));
                in6.sin6_port = htons(atoi(lin + matches[1].rm_so));
                memcpy(res->addr.ai_addr, &in6, sizeof(in6));
                break;
            default:
                logmsg(LOG_ERR, "line %d: Unknown Listener address family %d", n_lin, res->addr.ai_family);
                exit(1);
            }
            has_port = 1;
        } else if(!regexec(&xHTTP, lin, 4, matches, 0)) {
            int n;

            n = atoi(lin + matches[1].rm_so);
            regfree(&res->verb);
            if(regcomp(&res->verb, xhttp[n], REG_ICASE | REG_NEWLINE | REG_EXTENDED)) {
                logmsg(LOG_ERR, "line %d: xHTTP bad pattern %d - aborted", n_lin, n);
                exit(1);
            }
        } else if(!regexec(&Client, lin, 4, matches, 0)) {
            res->to = atoi(lin + matches[1].rm_so);
        } else if(!regexec(&CheckURL, lin, 4, matches, 0)) {
            if(res->has_pat) {
                logmsg(LOG_ERR, "line %d: CheckURL multiple pattern - aborted", n_lin);
                exit(1);
            }
            lin[matches[1].rm_eo] = '\0';
            if(regcomp(&res->url_pat, lin + matches[1].rm_so, REG_NEWLINE | REG_EXTENDED)) {
                logmsg(LOG_ERR, "line %d: CheckURL bad pattern \"%s\" - aborted", n_lin, lin + matches[1].rm_so);
                exit(1);
            }
            res->has_pat = 1;
        } else if(!regexec(&Err414, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            res->err414 = file2str(lin + matches[1].rm_so);
        } else if(!regexec(&Err500, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            res->err500 = file2str(lin + matches[1].rm_so);
        } else if(!regexec(&Err501, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            res->err501 = file2str(lin + matches[1].rm_so);
        } else if(!regexec(&Err503, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            res->err503 = file2str(lin + matches[1].rm_so);
        } else if(!regexec(&MaxRequest, lin, 4, matches, 0)) {
            res->max_req = atol(lin + matches[1].rm_so);
        } else if(!regexec(&HeadRemove, lin, 4, matches, 0)) {
            if(res->head_off) {
                for(m = res->head_off; m->next; m = m->next)
                    ;
                if((m->next = (MATCHER *)malloc(sizeof(MATCHER))) == NULL) {
                    logmsg(LOG_ERR, "line %d: HeadRemove config: out of memory - aborted", n_lin);
                    exit(1);
                }
                m = m->next;
            } else {
                if((res->head_off = (MATCHER *)malloc(sizeof(MATCHER))) == NULL) {
                    logmsg(LOG_ERR, "line %d: HeadRemove config: out of memory - aborted", n_lin);
                    exit(1);
                }
                m = res->head_off;
            }
            memset(m, 0, sizeof(MATCHER));
            lin[matches[1].rm_eo] = '\0';
            if(regcomp(&m->pat, lin + matches[1].rm_so, REG_ICASE | REG_NEWLINE | REG_EXTENDED)) {
                logmsg(LOG_ERR, "line %d: HeadRemove bad pattern \"%s\" - aborted", n_lin, lin + matches[1].rm_so);
                exit(1);
            }
        } else if(!regexec(&AddHeader, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            if((res->add_head = strdup(lin + matches[1].rm_so)) == NULL) {
                logmsg(LOG_ERR, "line %d: AddHeader config: out of memory - aborted", n_lin);
                exit(1);
            }
        } else if(!regexec(&RewriteLocation, lin, 4, matches, 0)) {
            res->rewr_loc = atoi(lin + matches[1].rm_so);
        } else if(!regexec(&RewriteDestination, lin, 4, matches, 0)) {
            res->rewr_dest = atoi(lin + matches[1].rm_so);
        } else if(!regexec(&LogLevel, lin, 4, matches, 0)) {
            res->log_level = atoi(lin + matches[1].rm_so);
        } else if(!regexec(&Service, lin, 4, matches, 0)) {
            if(res->services == NULL)
                res->services = parse_service(f_conf, NULL);
            else {
                for(svc = res->services; svc->next; svc = svc->next)
                    ;
                svc->next = parse_service(f_conf, NULL);
            }
        } else if(!regexec(&ServiceName, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            if(res->services == NULL)
                res->services = parse_service(f_conf, lin + matches[1].rm_so);
            else {
                for(svc = res->services; svc->next; svc = svc->next)
                    ;
                svc->next = parse_service(f_conf, lin + matches[1].rm_so);
            }
        } else if(!regexec(&End, lin, 4, matches, 0)) {
            if(!has_addr || !has_port) {
                logmsg(LOG_ERR, "line %d: ListenHTTP missing Address or Port - aborted", n_lin);
                exit(1);
            }
            return res;
        } else {
            logmsg(LOG_ERR, "line %d: unknown directive \"%s\" - aborted", n_lin, lin);
            exit(1);
        }
    }

    logmsg(LOG_ERR, "line %d: ListenHTTP premature EOF", n_lin);
    exit(1);
    return NULL;
}
/*
 * Dummy certificate verification - always OK
 */
static int
verify_OK(int pre_ok, X509_STORE_CTX *ctx)
{
    return 1;
}

/*
 * parse an HTTPS listener
 */
static LISTENER *
parse_HTTPS(FILE *const f_conf)
{
    char        lin[MAXBUF];
    LISTENER    *res;
    SERVICE     *svc;
    MATCHER     *m;
    int         has_addr, has_port, has_cert;
    struct hostent      *host;
    struct sockaddr_in  in;
    struct sockaddr_in6 in6;

    if((res = (LISTENER *)malloc(sizeof(LISTENER))) == NULL) {
        logmsg(LOG_ERR, "line %d: ListenHTTPS config: out of memory - aborted", n_lin);
        exit(1);
    }
    memset(res, 0, sizeof(LISTENER));
    if((res->ctx = SSL_CTX_new(SSLv23_server_method())) == NULL) {
        logmsg(LOG_ERR, "line %d: SSL_CTX_new failed - aborted", n_lin);
        exit(1);
    }

    res->to = clnt_to;
    res->rewr_loc = 1;
    res->err414 = "Request URI is too long";
    res->err500 = "An internal server error occurred. Please try again later.";
    res->err501 = "This method may not be used.";
    res->err503 = "The service is not available. Please try again later.";
    res->log_level = log_level;
    if(regcomp(&res->verb, xhttp[0], REG_ICASE | REG_NEWLINE | REG_EXTENDED)) {
        logmsg(LOG_ERR, "line %d: xHTTP bad default pattern - aborted", n_lin);
        exit(1);
    }
    has_addr = has_port = has_cert = 0;
    while(fgets(lin, MAXBUF, f_conf)) {
        n_lin++;
        if(strlen(lin) > 0 && lin[strlen(lin) - 1] == '\n')
            lin[strlen(lin) - 1] = '\0';
        if(!regexec(&Empty, lin, 4, matches, 0) || !regexec(&Comment, lin, 4, matches, 0)) {
            /* comment or empty line */
            continue;
        } else if(!regexec(&Address, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            if(get_host(lin + matches[1].rm_so, &res->addr)) {
                logmsg(LOG_ERR, "line %d: Unknown Listener address \"%s\"", n_lin, lin + matches[1].rm_so);
                exit(1);
            }
            if(res->addr.ai_family != AF_INET && res->addr.ai_family != AF_INET6) {
                logmsg(LOG_ERR, "line %d: Unknown Listener address family %d", n_lin, res->addr.ai_family);
                exit(1);
            }
            has_addr = 1;
        } else if(!regexec(&Port, lin, 4, matches, 0)) {
            if(res->addr.ai_family == AF_INET) {
                memcpy(&in, res->addr.ai_addr, sizeof(in));
                in.sin_port = (in_port_t)htons(atoi(lin + matches[1].rm_so));
                memcpy(res->addr.ai_addr, &in, sizeof(in));
            } else {
                memcpy(&in6, res->addr.ai_addr, sizeof(in6));
                in6.sin6_port = htons(atoi(lin + matches[1].rm_so));
                memcpy(res->addr.ai_addr, &in6, sizeof(in6));
            }
            has_port = 1;
        } else if(!regexec(&xHTTP, lin, 4, matches, 0)) {
            int n;

            n = atoi(lin + matches[1].rm_so);
            regfree(&res->verb);
            if(regcomp(&res->verb, xhttp[n], REG_ICASE | REG_NEWLINE | REG_EXTENDED)) {
                logmsg(LOG_ERR, "line %d: xHTTP bad pattern %d - aborted", n_lin, n);
                exit(1);
            }
        } else if(!regexec(&Client, lin, 4, matches, 0)) {
            res->to = atoi(lin + matches[1].rm_so);
        } else if(!regexec(&CheckURL, lin, 4, matches, 0)) {
            if(res->has_pat) {
                logmsg(LOG_ERR, "line %d: CheckURL multiple pattern - aborted", n_lin);
                exit(1);
            }
            lin[matches[1].rm_eo] = '\0';
            if(regcomp(&res->url_pat, lin + matches[1].rm_so, REG_NEWLINE | REG_EXTENDED)) {
                logmsg(LOG_ERR, "line %d: CheckURL bad pattern \"%s\" - aborted", n_lin, lin + matches[1].rm_so);
                exit(1);
            }
            res->has_pat = 1;
        } else if(!regexec(&Err414, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            res->err414 = file2str(lin + matches[1].rm_so);
        } else if(!regexec(&Err500, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            res->err500 = file2str(lin + matches[1].rm_so);
        } else if(!regexec(&Err501, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            res->err501 = file2str(lin + matches[1].rm_so);
        } else if(!regexec(&Err503, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            res->err503 = file2str(lin + matches[1].rm_so);
        } else if(!regexec(&MaxRequest, lin, 4, matches, 0)) {
            res->max_req = atol(lin + matches[1].rm_so);
        } else if(!regexec(&HeadRemove, lin, 4, matches, 0)) {
            if(res->head_off) {
                for(m = res->head_off; m->next; m = m->next)
                    ;
                if((m->next = (MATCHER *)malloc(sizeof(MATCHER))) == NULL) {
                    logmsg(LOG_ERR, "line %d: HeadRemove config: out of memory - aborted", n_lin);
                    exit(1);
                }
                m = m->next;
            } else {
                if((res->head_off = (MATCHER *)malloc(sizeof(MATCHER))) == NULL) {
                    logmsg(LOG_ERR, "line %d: HeadRemove config: out of memory - aborted", n_lin);
                    exit(1);
                }
                m = res->head_off;
            }
            memset(m, 0, sizeof(MATCHER));
            lin[matches[1].rm_eo] = '\0';
            if(regcomp(&m->pat, lin + matches[1].rm_so, REG_ICASE | REG_NEWLINE | REG_EXTENDED)) {
                logmsg(LOG_ERR, "line %d: HeadRemove bad pattern \"%s\" - aborted", n_lin, lin + matches[1].rm_so);
                exit(1);
            }
        } else if(!regexec(&RewriteLocation, lin, 4, matches, 0)) {
            res->rewr_loc = atoi(lin + matches[1].rm_so);
        } else if(!regexec(&RewriteDestination, lin, 4, matches, 0)) {
            res->rewr_dest = atoi(lin + matches[1].rm_so);
        } else if(!regexec(&LogLevel, lin, 4, matches, 0)) {
            res->log_level = atoi(lin + matches[1].rm_so);
        } else if(!regexec(&Cert, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            if(SSL_CTX_use_certificate_chain_file(res->ctx, lin + matches[1].rm_so) != 1) {
                logmsg(LOG_ERR, "line %d: SSL_CTX_use_certificate_chain_file \"%s\" failed - aborted", n_lin,
                    lin + matches[1].rm_so);
                logmsg(LOG_ERR, "%s", ERR_error_string(ERR_get_error(), NULL));
                exit(1);
            }
            if(SSL_CTX_use_PrivateKey_file(res->ctx, lin + matches[1].rm_so, SSL_FILETYPE_PEM) != 1) {
                logmsg(LOG_ERR, "line %d: SSL_CTX_use_PrivateKey_file \"%s\" failed - aborted", n_lin,
                    lin + matches[1].rm_so);
                logmsg(LOG_ERR, "%s", ERR_error_string(ERR_get_error(), NULL));
                exit(1);
            }
            if(SSL_CTX_check_private_key(res->ctx) != 1) {
                logmsg(LOG_ERR, "line %d: SSL_CTX_check_private_key \"%s\" failed - aborted", n_lin,
                    lin + matches[1].rm_so);
                logmsg(LOG_ERR, "%s", ERR_error_string(ERR_get_error(), NULL));
                exit(1);
            }
            has_cert = 1;
        } else if(!regexec(&ClientCert, lin, 4, matches, 0)) {
            switch(res->clnt_check = atoi(lin + matches[1].rm_so)) {
            case 0:
                /* don't ask */
                SSL_CTX_set_verify(res->ctx, SSL_VERIFY_NONE, NULL);
                break;
            case 1:
                /* ask but OK if no client certificate */
                SSL_CTX_set_verify(res->ctx, SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE, NULL);
                SSL_CTX_set_verify_depth(res->ctx, atoi(lin + matches[2].rm_so));
                break;
            case 2:
                /* ask and fail if no client certificate */
                SSL_CTX_set_verify(res->ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL);
                SSL_CTX_set_verify_depth(res->ctx, atoi(lin + matches[2].rm_so));
                break;
            case 3:
                /* ask but do not verify client certificate */
                SSL_CTX_set_verify(res->ctx, SSL_VERIFY_PEER | SSL_VERIFY_CLIENT_ONCE, verify_OK);
                SSL_CTX_set_verify_depth(res->ctx, atoi(lin + matches[2].rm_so));
                break;
            }
        } else if(!regexec(&AddHeader, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            if((res->add_head = strdup(lin + matches[1].rm_so)) == NULL) {
                logmsg(LOG_ERR, "line %d: AddHeader config: out of memory - aborted", n_lin);
                exit(1);
            }
        } else if(!regexec(&Ciphers, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            SSL_CTX_set_cipher_list(res->ctx, lin + matches[1].rm_so);
        } else if(!regexec(&CAlist, lin, 4, matches, 0)) {
            STACK_OF(X509_NAME) *cert_names;

            lin[matches[1].rm_eo] = '\0';
            if((cert_names = SSL_load_client_CA_file(lin + matches[1].rm_so)) == NULL) {
                logmsg(LOG_ERR, "line %d: SSL_load_client_CA_file \"%s\" failed - aborted", n_lin,
                    lin + matches[1].rm_so);
                logmsg(LOG_ERR, "%s", ERR_error_string(ERR_get_error(), NULL));
                exit(1);
            }
            SSL_CTX_set_client_CA_list(res->ctx, cert_names);
        } else if(!regexec(&VerifyList, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            if(SSL_CTX_load_verify_locations(res->ctx, lin + matches[1].rm_so, NULL) != 1) {
                logmsg(LOG_ERR, "line %d: SSL_CTX_load_verify_locations \"%s\" failed - aborted", n_lin,
                    lin + matches[1].rm_so);
                logmsg(LOG_ERR, "%s", ERR_error_string(ERR_get_error(), NULL));
                exit(1);
            }
        } else if(!regexec(&CRLlist, lin, 4, matches, 0)) {
#if HAVE_X509_STORE_SET_FLAGS
            X509_STORE *store;
            X509_LOOKUP *lookup;

            lin[matches[1].rm_eo] = '\0';
            store = SSL_CTX_get_cert_store(res->ctx);
            if((lookup = X509_STORE_add_lookup(store, X509_LOOKUP_file())) == NULL) {
                logmsg(LOG_ERR, "line %d: X509_STORE_add_lookup \"%s\" failed - aborted", n_lin,
                    lin + matches[1].rm_so);
                logmsg(LOG_ERR, "%s", ERR_error_string(ERR_get_error(), NULL));
                exit(1);
            }
            if(X509_load_crl_file(lookup, lin + matches[1].rm_so, X509_FILETYPE_PEM) != 1) {
                logmsg(LOG_ERR, "line %d: X509_load_crl_file \"%s\" failed - aborted", n_lin, lin + matches[1].rm_so);
                logmsg(LOG_ERR, "%s", ERR_error_string(ERR_get_error(), NULL));
                exit(1);
            }
            X509_STORE_set_flags(store, X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL);
#else
            logmsg(LOG_ERR, "line %d: your version of OpenSSL does not support CRL checking", n_lin);
#endif
        } else if(!regexec(&NoHTTPS11, lin, 4, matches, 0)) {
            res->noHTTPS11 = atoi(lin + matches[1].rm_so);
        } else if(!regexec(&Service, lin, 4, matches, 0)) {
            if(res->services == NULL)
                res->services = parse_service(f_conf, NULL);
            else {
                for(svc = res->services; svc->next; svc = svc->next)
                    ;
                svc->next = parse_service(f_conf, NULL);
            }
        } else if(!regexec(&ServiceName, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            if(res->services == NULL)
                res->services = parse_service(f_conf, lin + matches[1].rm_so);
            else {
                for(svc = res->services; svc->next; svc = svc->next)
                    ;
                svc->next = parse_service(f_conf, lin + matches[1].rm_so);
            }
        } else if(!regexec(&End, lin, 4, matches, 0)) {
            X509_STORE  *store;

            if(!has_addr || !has_port || !has_cert) {
                logmsg(LOG_ERR, "line %d: ListenHTTPS missing Address, Port or Certificate - aborted", n_lin);
                exit(1);
            }
            SSL_CTX_set_mode(res->ctx, SSL_MODE_AUTO_RETRY);
            SSL_CTX_set_options(res->ctx, SSL_OP_ALL);
            sprintf(lin, "%d-Pound-%ld", getpid(), random());
            SSL_CTX_set_session_id_context(res->ctx, (unsigned char *)lin, strlen(lin));
            SSL_CTX_set_tmp_rsa_callback(res->ctx, RSA_tmp_callback);
            return res;
        } else {
            logmsg(LOG_ERR, "line %d: unknown directive \"%s\" - aborted", n_lin, lin);
            exit(1);
        }
    }

    logmsg(LOG_ERR, "line %d: ListenHTTPS premature EOF", n_lin);
    exit(1);
    return NULL;
}

/*
 * parse the config file
 */
static void
parse_file(FILE *const f_conf)
{
    char        lin[MAXBUF];
    SERVICE     *svc;
    LISTENER    *lstn;
    int         i;
#if HAVE_OPENSSL_ENGINE_H
    ENGINE      *e;
#endif

    while(fgets(lin, MAXBUF, f_conf)) {
        n_lin++;
        if(strlen(lin) > 0 && lin[strlen(lin) - 1] == '\n')
            lin[strlen(lin) - 1] = '\0';
        if(!regexec(&Empty, lin, 4, matches, 0) || !regexec(&Comment, lin, 4, matches, 0)) {
            /* comment or empty line */
            continue;
        } else if(!regexec(&User, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            if((user = strdup(lin + matches[1].rm_so)) == NULL) {
                logmsg(LOG_ERR, "line %d: User config: out of memory - aborted", n_lin);
                exit(1);
            }
        } else if(!regexec(&Group, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            if((group = strdup(lin + matches[1].rm_so)) == NULL) {
                logmsg(LOG_ERR, "line %d: Group config: out of memory - aborted", n_lin);
                exit(1);
            }
        } else if(!regexec(&RootJail, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            if((root_jail = strdup(lin + matches[1].rm_so)) == NULL) {
                logmsg(LOG_ERR, "line %d: RootJail config: out of memory - aborted", n_lin);
                exit(1);
            }
        } else if(!regexec(&Daemon, lin, 4, matches, 0)) {
            daemonize = atoi(lin + matches[1].rm_so);
        } else if(!regexec(&LogFacility, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            if(lin[matches[1].rm_so] == '-')
                def_facility = -1;
            else
                for(i = 0; facilitynames[i].c_name; i++)
                    if(!strcmp(facilitynames[i].c_name, lin + matches[1].rm_so)) {
                        def_facility = facilitynames[i].c_val;
                        break;
                    }
        } else if(!regexec(&Grace, lin, 4, matches, 0)) {
            grace = atoi(lin + matches[1].rm_so);
        } else if(!regexec(&LogLevel, lin, 4, matches, 0)) {
            log_level = atoi(lin + matches[1].rm_so);
        } else if(!regexec(&Client, lin, 4, matches, 0)) {
            clnt_to = atoi(lin + matches[1].rm_so);
        } else if(!regexec(&Alive, lin, 4, matches, 0)) {
            alive_to = atoi(lin + matches[1].rm_so);
        } else if(!regexec(&DynScale, lin, 4, matches, 0)) {
            dynscale = atoi(lin + matches[1].rm_so);
        } else if(!regexec(&TimeOut, lin, 4, matches, 0)) {
            be_to = atoi(lin + matches[1].rm_so);
#if HAVE_OPENSSL_ENGINE_H
        } else if(!regexec(&SSLEngine, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
#if OPENSSL_VERSION_NUMBER >= 0x00907000L
            ENGINE_load_builtin_engines();
#endif
            if (!(e = ENGINE_by_id(lin + matches[1].rm_so))) {
                logmsg(LOG_ERR, "line %d: could not find %s engine", n_lin, lin + matches[1].rm_so);
                exit(1);
            }
            if(!ENGINE_init(e)) {
                ENGINE_free(e);
                logmsg(LOG_ERR, "line %d: could not init %s engine", n_lin, lin + matches[1].rm_so);
                exit(1);
            }
            if(!ENGINE_set_default(e, ENGINE_METHOD_ALL)) {
                ENGINE_free(e);
                logmsg(LOG_ERR, "line %d: could not set all defaults", n_lin);
                exit(1);
            }
            ENGINE_finish(e);
            ENGINE_free(e);
#endif
        } else if(!regexec(&Control, lin, 4, matches, 0)) {
            if(ctrl_name != NULL) {
                logmsg(LOG_ERR, "line %d: Control multiply defined - aborted", n_lin);
                exit(1);
            }
            lin[matches[1].rm_eo] = '\0';
            ctrl_name = strdup(lin + matches[1].rm_so);
        } else if(!regexec(&ListenHTTP, lin, 4, matches, 0)) {
            if(listeners == NULL)
                listeners = parse_HTTP(f_conf);
            else {
                for(lstn = listeners; lstn->next; lstn = lstn->next)
                    ;
                lstn->next = parse_HTTP(f_conf);
            }
        } else if(!regexec(&ListenHTTPS, lin, 4, matches, 0)) {
            if(listeners == NULL)
                listeners = parse_HTTPS(f_conf);
            else {
                for(lstn = listeners; lstn->next; lstn = lstn->next)
                    ;
                lstn->next = parse_HTTPS(f_conf);
            }
        } else if(!regexec(&Service, lin, 4, matches, 0)) {
            if(services == NULL)
                services = parse_service(f_conf, NULL);
            else {
                for(svc = services; svc->next; svc = svc->next)
                    ;
                svc->next = parse_service(f_conf, NULL);
            }
        } else if(!regexec(&ServiceName, lin, 4, matches, 0)) {
            lin[matches[1].rm_eo] = '\0';
            if(services == NULL)
                services = parse_service(f_conf, lin + matches[1].rm_so);
            else {
                for(svc = services; svc->next; svc = svc->next)
                    ;
                svc->next = parse_service(f_conf, lin + matches[1].rm_so);
            }
        } else {
            logmsg(LOG_ERR, "line %d: unknown directive \"%s\" - aborted", n_lin, lin);
            exit(1);
        }
    }
    return;
}

/*
 * prepare to parse the arguments/config file
 */
void
config_parse(const int argc, char **const argv)
{
    char    *conf_name;
    FILE    *f_conf;
    int     c_opt, check_only;

    if(regcomp(&Empty, "^[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&Comment, "^[ \t]*#.*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&User, "^[ \t]*User[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&Group, "^[ \t]*Group[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&RootJail, "^[ \t]*RootJail[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&Daemon, "^[ \t]*Daemon[ \t]+([01])[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&LogFacility, "^[ \t]*LogFacility[ \t]+([a-z0-9-]+)[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&LogLevel, "^[ \t]*LogLevel[ \t]+([0-5])[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&Grace, "^[ \t]*Grace[ \t]+([0-9]+)[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&Alive, "^[ \t]*Alive[ \t]+([1-9][0-9]*)[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&SSLEngine, "^[ \t]*SSLEngine[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&Control, "^[ \t]*Control[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&ListenHTTP, "^[ \t]*ListenHTTP[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&ListenHTTPS, "^[ \t]*ListenHTTPS[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&End, "^[ \t]*End[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&Address, "^[ \t]*Address[ \t]+([^ \t]+)[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&Port, "^[ \t]*Port[ \t]+([1-9][0-9]*)[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&Cert, "^[ \t]*Cert[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&xHTTP, "^[ \t]*xHTTP[ \t]+([01234])[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&Client, "^[ \t]*Client[ \t]+([1-9][0-9]*)[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&CheckURL, "^[ \t]*CheckURL[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&Err414, "^[ \t]*Err414[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&Err500, "^[ \t]*Err500[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&Err501, "^[ \t]*Err501[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&Err503, "^[ \t]*Err503[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&MaxRequest, "^[ \t]*MaxRequest[ \t]+([1-9][0-9]*)[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&HeadRemove, "^[ \t]*HeadRemove[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&RewriteLocation, "^[ \t]*RewriteLocation[ \t]+([012])[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&RewriteDestination, "^[ \t]*RewriteDestination[ \t]+([01])[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&Service, "^[ \t]*Service[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&ServiceName, "^[ \t]*Service[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&URL, "^[ \t]*URL[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&HeadRequire, "^[ \t]*HeadRequire[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&HeadDeny, "^[ \t]*HeadDeny[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&BackEnd, "^[ \t]*BackEnd[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&Emergency, "^[ \t]*Emergency[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&Priority, "^[ \t]*Priority[ \t]+([1-9])[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&TimeOut, "^[ \t]*TimeOut[ \t]+([1-9][0-9]*)[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&HAport, "^[ \t]*HAport[ \t]+([1-9][0-9]*)[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&HAportAddr, "^[ \t]*HAport[ \t]+([^ \t]+)[ \t]+([1-9][0-9]*)[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&Redirect, "^[ \t]*Redirect[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&RedirectN, "^[ \t]*Redirect[ \t]+(30[127])[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&Session, "^[ \t]*Session[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&Type, "^[ \t]*Type[ \t]+([^ \t]+)[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&TTL, "^[ \t]*TTL[ \t]+([1-9-][0-9]*)[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&ID, "^[ \t]*ID[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&DynScale, "^[ \t]*DynScale[ \t]+([01])[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&ClientCert, "^[ \t]*ClientCert[ \t]+([0-3])[ \t]+([1-9])[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&AddHeader, "^[ \t]*AddHeader[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&Ciphers, "^[ \t]*Ciphers[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&CAlist, "^[ \t]*CAlist[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&VerifyList, "^[ \t]*VerifyList[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&CRLlist, "^[ \t]*CRLlist[ \t]+\"(.+)\"[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    || regcomp(&NoHTTPS11, "^[ \t]*NoHTTPS11[ \t]+([0-2])[ \t]*$", REG_ICASE | REG_NEWLINE | REG_EXTENDED)
    ) {
        logmsg(LOG_ERR, "bad config Regex - aborted");
        exit(1);
    }

    opterr = 0;
    check_only = 0;
    conf_name = F_CONF;
    pid_name = F_PID;

    while((c_opt = getopt(argc, argv, "f:cvVp:")) > 0)
        switch(c_opt) {
        case 'f':
            conf_name = optarg;
            break;
        case 'p':
            pid_name = optarg;
            break;
        case 'c':
            check_only = 1;
            break;
        case 'v':
            print_log = 1;
            break;
        case 'V':
            print_log = 1;
            logmsg(LOG_DEBUG, "Version %s", VERSION);
            logmsg(LOG_DEBUG, "  Configuration switches:");
#ifdef  C_SUPER
            if(strcmp(C_SUPER, "0"))
                logmsg(LOG_DEBUG, "    --disable-super");
#endif
#ifdef  C_CERT1L
            if(strcmp(C_CERT1L, "1"))
                logmsg(LOG_DEBUG, "    --enable-cert1l");
#endif
#ifdef  C_SSL
            if(strcmp(C_SSL, ""))
                logmsg(LOG_DEBUG, "    --with-ssl=%s", C_SSL);
#endif
#ifdef  C_T_RSA
            if(strcmp(C_T_RSA, "0"))
                logmsg(LOG_DEBUG, "    --with-t_rsa=%s", C_T_RSA);
#endif
#ifdef  C_MAXBUF
            if(strcmp(C_MAXBUF, "0"))
                logmsg(LOG_DEBUG, "    --with-maxbuf=%s", C_MAXBUF);
#endif
#ifdef  C_OWNER
            if(strcmp(C_OWNER, ""))
                logmsg(LOG_DEBUG, "    --with-owner=%s", C_OWNER);
#endif
#ifdef  C_GROUP
            if(strcmp(C_GROUP, ""))
                logmsg(LOG_DEBUG, "    --with-group=%s", C_GROUP);
#endif
            logmsg(LOG_DEBUG, "Exiting...");
            exit(0);
            break;
        default:
            logmsg(LOG_ERR, "bad flag -%c", optopt);
            exit(1);
            break;
        }
    if(optind < argc) {
        logmsg(LOG_ERR, "unknown extra arguments (%s...)", argv[optind]);
        exit(1);
    }

    if((f_conf = fopen(conf_name, "rt")) == NULL) {
        logmsg(LOG_ERR, "can't open configuration file \"%s\" (%s) - aborted", conf_name, strerror(errno));
        exit(1);
    }

    user = NULL;
    group = NULL;
    root_jail = NULL;
    ctrl_name = NULL;

    alive_to = 30;
    daemonize = 1;
    grace = 30;

    services = NULL;
    listeners = NULL;

    parse_file(f_conf);

    fclose(f_conf);

    if(check_only) {
        logmsg(LOG_INFO, "Config file %s is OK", conf_name);
        exit(0);
    }

    if(listeners == NULL) {
        logmsg(LOG_ERR, "no listeners defined - aborted");
        exit(1);
    }

    regfree(&Empty);
    regfree(&Comment);
    regfree(&User);
    regfree(&Group);
    regfree(&RootJail);
    regfree(&Daemon);
    regfree(&LogFacility);
    regfree(&LogLevel);
    regfree(&Grace);
    regfree(&Alive);
    regfree(&SSLEngine);
    regfree(&Control);
    regfree(&ListenHTTP);
    regfree(&ListenHTTPS);
    regfree(&End);
    regfree(&Address);
    regfree(&Port);
    regfree(&Cert);
    regfree(&xHTTP);
    regfree(&Client);
    regfree(&CheckURL);
    regfree(&Err414);
    regfree(&Err500);
    regfree(&Err501);
    regfree(&Err503);
    regfree(&MaxRequest);
    regfree(&HeadRemove);
    regfree(&RewriteLocation);
    regfree(&RewriteDestination);
    regfree(&Service);
    regfree(&ServiceName);
    regfree(&URL);
    regfree(&HeadRequire);
    regfree(&HeadDeny);
    regfree(&BackEnd);
    regfree(&Emergency);
    regfree(&Priority);
    regfree(&TimeOut);
    regfree(&HAport);
    regfree(&HAportAddr);
    regfree(&Redirect);
    regfree(&RedirectN);
    regfree(&Session);
    regfree(&Type);
    regfree(&TTL);
    regfree(&ID);
    regfree(&DynScale);
    regfree(&ClientCert);
    regfree(&AddHeader);
    regfree(&Ciphers);
    regfree(&CAlist);
    regfree(&VerifyList);
    regfree(&CRLlist);
    regfree(&NoHTTPS11);

    /* set the facility only here to ensure the syslog gets opened if necessary */
    log_facility = def_facility;

    return;
}
