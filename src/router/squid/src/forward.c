
/*
 * $Id: forward.c,v 1.120 2007/01/19 00:21:01 hno Exp $
 *
 * DEBUG: section 17    Request Forwarding
 * AUTHOR: Duane Wessels
 *
 * SQUID Web Proxy Cache          http://www.squid-cache.org/
 * ----------------------------------------------------------
 *
 *  Squid is the result of efforts by numerous individuals from
 *  the Internet community; see the CONTRIBUTORS file for full
 *  details.   Many organizations have provided support for Squid's
 *  development; see the SPONSORS file for full details.  Squid is
 *  Copyrighted (C) 2001 by the Regents of the University of
 *  California; see the COPYRIGHT file for full details.  Squid
 *  incorporates software developed and/or copyrighted by other
 *  sources; see the CREDITS file for full details.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111, USA.
 *
 */


#include "squid.h"

#if LINUX_NETFILTER
#include <linux/netfilter_ipv4.h>
#endif
#if LINUX_TPROXY
#include <linux/netfilter_ipv4/ip_tproxy.h>
#endif

static PSC fwdStartComplete;
static void fwdDispatch(FwdState *);
static void fwdConnectStart(void *);	/* should be same as EVH */
static void fwdStateFree(FwdState * fwdState);
static PF fwdConnectTimeout;
static PF fwdServerClosed;
static PF fwdPeerClosed;
static CNCB fwdConnectDone;
static int fwdCheckRetry(FwdState * fwdState);
static int fwdReforward(FwdState *);
static void fwdStartFail(FwdState *);
static void fwdLogReplyStatus(int tries, http_status status);
static OBJH fwdStats;
static STABH fwdAbort;
static peer *fwdStateServerPeer(FwdState *);

#define MAX_FWD_STATS_IDX 9
static int FwdReplyCodes[MAX_FWD_STATS_IDX + 1][HTTP_INVALID_HEADER + 1];

#if WIP_FWD_LOG
static void fwdLog(FwdState * fwdState);
static Logfile *logfile = NULL;
#endif

static peer *
fwdStateServerPeer(FwdState * fwdState)
{
    if (NULL == fwdState)
	return NULL;
    if (NULL == fwdState->servers)
	return NULL;
    return fwdState->servers->peer;
}

static void
fwdServerFree(FwdServer * fs)
{
    if (fs->peer)
	cbdataUnlock(fs->peer);
    memFree(fs, MEM_FWD_SERVER);
}

static void
fwdStateFree(FwdState * fwdState)
{
    StoreEntry *e = fwdState->entry;
    int sfd;
    peer *p;
    debug(17, 3) ("fwdStateFree: %p\n", fwdState);
    assert(e->mem_obj);
#if URL_CHECKSUM_DEBUG
    assert(e->mem_obj->chksum == url_checksum(e->mem_obj->url));
#endif
#if WIP_FWD_LOG
    fwdLog(fwdState);
#endif
    if (e->store_status == STORE_PENDING) {
	if (e->mem_obj->inmem_hi == 0) {
	    assert(fwdState->err);
	    errorAppendEntry(e, fwdState->err);
	    fwdState->err = NULL;
	} else {
	    EBIT_CLR(e->flags, ENTRY_FWD_HDR_WAIT);
	    storeReleaseRequest(e);
	    storeComplete(e);
	}
    }
    if (EBIT_TEST(e->flags, ENTRY_DEFER_READ))
	storeResetDefer(e);
    if (storePendingNClients(e) > 0)
	assert(!EBIT_TEST(e->flags, ENTRY_FWD_HDR_WAIT));
    p = fwdStateServerPeer(fwdState);
    fwdServersFree(&fwdState->servers);
    requestUnlink(fwdState->request);
    fwdState->request = NULL;
    if (fwdState->err)
	errorStateFree(fwdState->err);
    storeClientUnregisterAbort(e);
    storeUnlockObject(e);
    fwdState->entry = NULL;
    sfd = fwdState->server_fd;
    if (sfd > -1) {
	comm_remove_close_handler(sfd, fwdServerClosed, fwdState);
	fwdState->server_fd = -1;
	debug(17, 3) ("fwdStateFree: closing FD %d\n", sfd);
	comm_close(sfd);
    }
    cbdataFree(fwdState);
}

static int
fwdCheckRetry(FwdState * fwdState)
{
    if (shutting_down)
	return 0;
    if (fwdState->entry->store_status != STORE_PENDING)
	return 0;
    if (fwdState->entry->mem_obj->inmem_hi > 0)
	return 0;
    if (fwdState->n_tries > 10)
	return 0;
    if (fwdState->origin_tries > 2)
	return 0;
    if (squid_curtime - fwdState->start >= Config.Timeout.forward)
	return 0;
    if (fwdState->flags.dont_retry)
	return 0;
    if (fwdState->request->flags.body_sent)
	return 0;
    return 1;
}

static int
fwdCheckRetriable(FwdState * fwdState)
{
    /* If there is a request body then Squid can only try once
     * even if the method is indempotent
     */
    if (fwdState->request->body_reader)
	return 0;

    /* RFC2616 9.1 Safe and Idempotent Methods */
    switch (fwdState->request->method) {
	/* 9.1.1 Safe Methods */
    case METHOD_GET:
    case METHOD_HEAD:
	/* 9.1.2 Indepontent Methods */
    case METHOD_PUT:
    case METHOD_DELETE:
    case METHOD_OPTIONS:
    case METHOD_TRACE:
	break;
    default:
	return 0;
    }

    return 1;
}

static void
fwdServerClosed(int fd, void *data)
{
    FwdState *fwdState = data;
    debug(17, 2) ("fwdServerClosed: FD %d %s\n", fd, storeUrl(fwdState->entry));
    assert(fwdState->server_fd == fd);
    fwdState->server_fd = -1;
    if (EBIT_TEST(fwdState->entry->flags, ENTRY_DEFER_READ))
	storeResetDefer(fwdState->entry);
    if (fwdCheckRetry(fwdState)) {
	int originserver = (fwdState->servers->peer == NULL);
	debug(17, 3) ("fwdServerClosed: re-forwarding (%d tries, %d secs)\n",
	    fwdState->n_tries,
	    (int) (squid_curtime - fwdState->start));
	if (fwdState->servers->next) {
	    /* use next, or cycle if origin server isn't last */
	    FwdServer *fs = fwdState->servers;
	    FwdServer **T, *T2 = NULL;
	    fwdState->servers = fs->next;
	    for (T = &fwdState->servers; *T; T2 = *T, T = &(*T)->next);
	    if (T2 && T2->peer) {
		/* cycle */
		*T = fs;
		fs->next = NULL;
	    } else {
		/* Use next. The last "direct" entry is retried multiple times */
		fwdState->servers = fs->next;
		fwdServerFree(fs);
		originserver = 0;
	    }
	}
	/* use eventAdd to break potential call sequence loops and to slow things down a little */
	eventAdd("fwdConnectStart", fwdConnectStart, fwdState, originserver ? 0.05 : 0.005, 0);
	return;
    }
    if (!fwdState->err && shutting_down)
	fwdState->err = errorCon(ERR_SHUTTING_DOWN, HTTP_SERVICE_UNAVAILABLE, fwdState->request);
    fwdStateFree(fwdState);
}

#if USE_SSL
static void
fwdNegotiateSSL(int fd, void *data)
{
    FwdState *fwdState = data;
    FwdServer *fs = fwdState->servers;
    SSL *ssl = fd_table[fd].ssl;
    int ret;
    ErrorState *err;
    request_t *request = fwdState->request;

    errno = 0;
    ERR_clear_error();
    if ((ret = SSL_connect(ssl)) <= 0) {
	int ssl_error = SSL_get_error(ssl, ret);
	switch (ssl_error) {
	case SSL_ERROR_WANT_READ:
	    commSetSelect(fd, COMM_SELECT_READ, fwdNegotiateSSL, fwdState, 0);
	    return;
	case SSL_ERROR_WANT_WRITE:
	    commSetSelect(fd, COMM_SELECT_WRITE, fwdNegotiateSSL, fwdState, 0);
	    return;
	default:
	    debug(81, 1) ("fwdNegotiateSSL: Error negotiating SSL connection on FD %d: %s (%d/%d/%d)\n", fd, ERR_error_string(ERR_get_error(), NULL), ssl_error, ret, errno);
	    err = errorCon(ERR_CONNECT_FAIL, HTTP_SERVICE_UNAVAILABLE, request);
#ifdef EPROTO
	    err->xerrno = EPROTO;
#else
	    err->xerrno = EACCES;
#endif
	    fwdFail(fwdState, err);
	    if (fs->peer) {
		peerConnectFailed(fs->peer);
		fs->peer->stats.conn_open--;
	    }
	    comm_close(fd);
	    return;
	}
    }
    if (fs->peer && !SSL_session_reused(ssl)) {
	if (fs->peer->sslSession)
	    SSL_SESSION_free(fs->peer->sslSession);
	fs->peer->sslSession = SSL_get1_session(ssl);
    }
#if NOT_YET
    if (verify_domain) {
	char *host;
	STACK_OF(GENERAL_NAME) * altnames;
	if (fs->peer) {
	    if (fs->peer->ssldomain)
		host = fs->peer->ssldomain;
	    else
		host = fs->peer->host;
	} else {
	    host = fs->request->host;
	}
	if (!ssl_verify_domain(host, ssl)) {
	    debug(17, 1) ("Warning: SSL certificate does not match host name '%s'\n", host);
	}
    }
#endif
    fwdDispatch(fwdState);
}

static void
fwdInitiateSSL(FwdState * fwdState)
{
    FwdServer *fs = fwdState->servers;
    int fd = fwdState->server_fd;
    SSL *ssl;
    SSL_CTX *sslContext = NULL;
    peer *peer = fs->peer;
    if (peer) {
	assert(peer->use_ssl);
	sslContext = peer->sslContext;
    } else {
	sslContext = Config.ssl_client.sslContext;
    }
    assert(sslContext);
    if ((ssl = SSL_new(sslContext)) == NULL) {
	ErrorState *err;
	debug(83, 1) ("fwdInitiateSSL: Error allocating handle: %s\n",
	    ERR_error_string(ERR_get_error(), NULL));
	err = errorCon(ERR_SOCKET_FAILURE, HTTP_INTERNAL_SERVER_ERROR, fwdState->request);
	err->xerrno = errno;
	fwdFail(fwdState, err);
	fwdStateFree(fwdState);
	return;
    }
    SSL_set_fd(ssl, fd);
    if (peer) {
	if (peer->sslSession)
	    SSL_set_session(ssl, peer->sslSession);
    }
    fd_table[fd].ssl = ssl;
    fd_table[fd].read_method = &ssl_read_method;
    fd_table[fd].write_method = &ssl_write_method;
    fwdNegotiateSSL(fd, fwdState);
}
#endif

static void
fwdConnectDone(int server_fd, int status, void *data)
{
    FwdState *fwdState = data;
    FwdServer *fs = fwdState->servers;
    ErrorState *err;
    request_t *request = fwdState->request;
    assert(fwdState->server_fd == server_fd);
    if (Config.onoff.log_ip_on_direct && status != COMM_ERR_DNS && fs->code == HIER_DIRECT)
	hierarchyNote(&fwdState->request->hier, fs->code, fd_table[server_fd].ipaddr);
    if (status == COMM_ERR_DNS) {
	/*
	 * Only set the dont_retry flag if the DNS lookup fails on
	 * a direct connection.  If DNS lookup fails when trying
	 * a neighbor cache, we may want to retry another option.
	 */
	if (NULL == fs->peer)
	    fwdState->flags.dont_retry = 1;
	debug(17, 4) ("fwdConnectDone: Unknown host: %s\n",
	    request->host);
	err = errorCon(ERR_DNS_FAIL, HTTP_SERVICE_UNAVAILABLE, fwdState->request);
	err->dnsserver_msg = xstrdup(dns_error_message);
	fwdFail(fwdState, err);
	comm_close(server_fd);
    } else if (status != COMM_OK) {
	assert(fs);
	err = errorCon(ERR_CONNECT_FAIL, HTTP_SERVICE_UNAVAILABLE, fwdState->request);
	err->xerrno = errno;
	fwdFail(fwdState, err);
	if (fs->peer)
	    peerConnectFailed(fs->peer);
	comm_close(server_fd);
    } else {
	debug(17, 3) ("fwdConnectDone: FD %d: '%s'\n", server_fd, storeUrl(fwdState->entry));
	fd_note(server_fd, storeUrl(fwdState->entry));
	fd_table[server_fd].uses++;
	if (fd_table[server_fd].uses == 1 && fs->peer)
	    peerConnectSucceded(fs->peer);
#if USE_SSL
	if ((fs->peer && fs->peer->use_ssl) ||
	    (!fs->peer && request->protocol == PROTO_HTTPS)) {
	    fwdInitiateSSL(fwdState);
	    return;
	}
#endif
	fwdDispatch(fwdState);
    }
}

static void
fwdConnectTimeout(int fd, void *data)
{
    FwdState *fwdState = data;
    StoreEntry *entry = fwdState->entry;
    FwdServer *fs = fwdState->servers;
    ErrorState *err;
    debug(17, 2) ("fwdConnectTimeout: FD %d: '%s'\n", fd, storeUrl(entry));
    assert(fd == fwdState->server_fd);
    if (Config.onoff.log_ip_on_direct && fs->code == HIER_DIRECT && fd_table[fd].ipaddr[0])
	hierarchyNote(&fwdState->request->hier, fs->code, fd_table[fd].ipaddr);
    if (entry->mem_obj->inmem_hi == 0) {
	err = errorCon(ERR_CONNECT_FAIL, HTTP_GATEWAY_TIMEOUT, fwdState->request);
	err->xerrno = ETIMEDOUT;
	fwdFail(fwdState, err);
	/*
	 * This marks the peer DOWN ... 
	 */
	if (fwdState->servers)
	    if (fwdState->servers->peer)
		peerConnectFailed(fwdState->servers->peer);
    }
    comm_close(fd);
}

static struct in_addr
aclMapAddr(acl_address * head, aclCheck_t * ch)
{
    acl_address *l;
    struct in_addr addr;
    for (l = head; l; l = l->next) {
	if (aclMatchAclList(l->acl_list, ch))
	    return l->addr;
    }
    addr.s_addr = INADDR_ANY;
    return addr;
}

static int
aclMapTOS(acl_tos * head, aclCheck_t * ch)
{
    acl_tos *l;
    for (l = head; l; l = l->next) {
	if (aclMatchAclList(l->acl_list, ch))
	    return l->tos;
    }
    return 0;
}

struct in_addr
getOutgoingAddr(request_t * request)
{
    aclCheck_t ch;
    memset(&ch, '\0', sizeof(aclCheck_t));
    if (request) {
	ch.src_addr = request->client_addr;
	ch.my_addr = request->my_addr;
	ch.my_port = request->my_port;
	ch.request = request;
    }
    return aclMapAddr(Config.accessList.outgoing_address, &ch);
}

unsigned long
getOutgoingTOS(request_t * request)
{
    aclCheck_t ch;
    memset(&ch, '\0', sizeof(aclCheck_t));
    if (request) {
	ch.src_addr = request->client_addr;
	ch.my_addr = request->my_addr;
	ch.my_port = request->my_port;
	ch.request = request;
    }
    return aclMapTOS(Config.accessList.outgoing_tos, &ch);
}

static void
fwdConnectStart(void *data)
{
    FwdState *fwdState = data;
    const char *url = storeUrl(fwdState->entry);
    int fd = -1;
    ErrorState *err;
    FwdServer *fs = fwdState->servers;
    const char *host;
    const char *name;
    unsigned short port;
    const char *domain = NULL;
    int ctimeout;
    int ftimeout = Config.Timeout.forward - (squid_curtime - fwdState->start);
    struct in_addr outgoing;
    unsigned short tos;
#if LINUX_TPROXY
    struct in_tproxy itp;
#endif

    assert(fs);
    assert(fwdState->server_fd == -1);
    debug(17, 3) ("fwdConnectStart: %s\n", url);
    if (fs->peer) {
	host = fs->peer->host;
	name = fs->peer->name;
	port = fs->peer->http_port;
	if (fs->peer->options.originserver)
	    domain = fwdState->request->host;
	ctimeout = fs->peer->connect_timeout > 0 ? fs->peer->connect_timeout
	    : Config.Timeout.peer_connect;
    } else {
	host = name = fwdState->request->host;
	port = fwdState->request->port;
	ctimeout = Config.Timeout.connect;
    }
    if (ftimeout < 0)
	ftimeout = 5;
    if (ftimeout < ctimeout)
	ctimeout = ftimeout;
    fwdState->request->flags.pinned = 0;
    if (fs->code == PINNED) {
	int auth;
	fd = clientGetPinnedConnection(fwdState->request->pinned_connection, fwdState->request, fs->peer, &auth);
	if (fd >= 0) {
#if 0
	    if (!fs->peer)
		fs->code = HIER_DIRECT;
#endif
	    fwdState->server_fd = fd;
	    fwdState->n_tries++;
	    fwdState->request->flags.pinned = 1;
	    if (auth)
		fwdState->request->flags.auth = 1;
	    comm_add_close_handler(fd, fwdServerClosed, fwdState);
	    fwdConnectDone(fd, COMM_OK, fwdState);
	    return;
	}
	/* Failure. Fall back on next path */
	cbdataUnlock(fwdState->request->pinned_connection);
	fwdState->request->pinned_connection = NULL;
	fwdState->servers = fs->next;
	fwdServerFree(fs);
	fwdConnectStart(fwdState);
	return;
    }
#if LINUX_TPROXY
    if (fd == -1 && fwdState->request->flags.tproxy)
	fd = pconnPop(name, port, domain, &fwdState->request->client_addr, 0);
#endif
    if (fd == -1)
	fd = pconnPop(name, port, domain, NULL, 0);
    if (fd != -1) {
	if (fwdCheckRetriable(fwdState)) {
	    debug(17, 3) ("fwdConnectStart: reusing pconn FD %d\n", fd);
	    fwdState->server_fd = fd;
	    fwdState->n_tries++;
	    if (!fs->peer)
		fwdState->origin_tries++;
	    comm_add_close_handler(fd, fwdServerClosed, fwdState);
	    if (fs->peer)
		hierarchyNote(&fwdState->request->hier, fs->code, fs->peer->host);
	    else if (Config.onoff.log_ip_on_direct && fs->code == HIER_DIRECT)
		hierarchyNote(&fwdState->request->hier, fs->code, fd_table[fd].ipaddr);
	    else
		hierarchyNote(&fwdState->request->hier, fs->code, name);
	    fwdConnectDone(fd, COMM_OK, fwdState);
	    return;
	} else {
	    /* Discard the persistent connection to not cause
	     * a imbalance in number of conenctions open if there
	     * is a lot of POST requests
	     */
	    comm_close(fd);
	}
    }
#if URL_CHECKSUM_DEBUG
    assert(fwdState->entry->mem_obj->chksum == url_checksum(url));
#endif
    outgoing = getOutgoingAddr(fwdState->request);
    tos = getOutgoingTOS(fwdState->request);

    debug(17, 3) ("fwdConnectStart: got addr %s, tos %d\n",
	inet_ntoa(outgoing), tos);
    fd = comm_openex(SOCK_STREAM,
	IPPROTO_TCP,
	outgoing,
	0,
	COMM_NONBLOCKING,
	tos,
	url);
    if (fd < 0) {
	debug(50, 4) ("fwdConnectStart: %s\n", xstrerror());
	err = errorCon(ERR_SOCKET_FAILURE, HTTP_INTERNAL_SERVER_ERROR, fwdState->request);
	err->xerrno = errno;
	fwdFail(fwdState, err);
	fwdStateFree(fwdState);
	return;
    }
    fwdState->server_fd = fd;
    fwdState->n_tries++;
    if (!fs->peer)
	fwdState->origin_tries++;
    /*
     * stats.conn_open is used to account for the number of
     * connections that we have open to the peer, so we can limit
     * based on the max-conn option.  We need to increment here,
     * even if the connection may fail.
     */
    if (fs->peer) {
	fs->peer->stats.conn_open++;
	comm_add_close_handler(fd, fwdPeerClosed, fs->peer);
    }
    comm_add_close_handler(fd, fwdServerClosed, fwdState);
    commSetTimeout(fd,
	ctimeout,
	fwdConnectTimeout,
	fwdState);
    if (fs->peer) {
	hierarchyNote(&fwdState->request->hier, fs->code, fs->peer->host);
    } else {
#if LINUX_TPROXY
	if (fwdState->request->flags.tproxy) {

	    itp.v.addr.faddr.s_addr = fwdState->src.sin_addr.s_addr;
	    itp.v.addr.fport = 0;

	    /* If these syscalls fail then we just fallback to connecting
	     * normally by simply ignoring the errors...
	     */
	    itp.op = TPROXY_ASSIGN;
	    if (setsockopt(fd, SOL_IP, IP_TPROXY, &itp, sizeof(itp)) == -1) {
		debug(20, 1) ("tproxy ip=%s,0x%x,port=%d ERROR ASSIGN\n",
		    inet_ntoa(itp.v.addr.faddr),
		    itp.v.addr.faddr.s_addr,
		    itp.v.addr.fport);
	    } else {
		itp.op = TPROXY_FLAGS;
		itp.v.flags = ITP_CONNECT;
		if (setsockopt(fd, SOL_IP, IP_TPROXY, &itp, sizeof(itp)) == -1) {
		    debug(20, 1) ("tproxy ip=%x,port=%d ERROR CONNECT\n",
			itp.v.addr.faddr.s_addr,
			itp.v.addr.fport);
		}
	    }
	}
#endif
	hierarchyNote(&fwdState->request->hier, fs->code, fwdState->request->host);
    }
    commConnectStart(fd, host, port, fwdConnectDone, fwdState);
}

static void
fwdStartComplete(FwdServer * servers, void *data)
{
    FwdState *fwdState = data;
    debug(17, 3) ("fwdStartComplete: %s\n", storeUrl(fwdState->entry));
    if (servers != NULL) {
	fwdState->servers = servers;
	fwdConnectStart(fwdState);
    } else {
	fwdStartFail(fwdState);
    }
}

static void
fwdStartFail(FwdState * fwdState)
{
    ErrorState *err;
    debug(17, 3) ("fwdStartFail: %s\n", storeUrl(fwdState->entry));
    err = errorCon(ERR_CANNOT_FORWARD, HTTP_SERVICE_UNAVAILABLE, fwdState->request);
    err->xerrno = errno;
    fwdFail(fwdState, err);
    fwdStateFree(fwdState);
}

static void
fwdDispatch(FwdState * fwdState)
{
    peer *p = NULL;
    request_t *request = fwdState->request;
    StoreEntry *entry = fwdState->entry;
    ErrorState *err;
    int server_fd = fwdState->server_fd;
    debug(17, 3) ("fwdDispatch: FD %d: Fetching '%s %s'\n",
	fwdState->client_fd,
	RequestMethodStr[request->method],
	storeUrl(entry));
    /*
     * Assert that server_fd is set.  This is to guarantee that fwdState
     * is attached to something and will be deallocated when server_fd
     * is closed.
     */
    assert(server_fd > -1);
    /*assert(!EBIT_TEST(entry->flags, ENTRY_DISPATCHED)); */
    assert(entry->ping_status != PING_WAITING);
    assert(entry->lock_count);
    EBIT_SET(entry->flags, ENTRY_DISPATCHED);
    netdbPingSite(request->host);
    entry->mem_obj->refresh_timestamp = squid_curtime;
    if (fwdState->servers && (p = fwdState->servers->peer)) {
	p->stats.fetches++;
	fwdState->request->peer_login = p->login;
	fwdState->request->peer_domain = p->domain;
	httpStart(fwdState);
    } else {
	fwdState->request->peer_login = NULL;
	fwdState->request->peer_domain = NULL;
	switch (request->protocol) {
#if USE_SSL
	case PROTO_HTTPS:
	    httpStart(fwdState);
	    break;
#endif
	case PROTO_HTTP:
	    httpStart(fwdState);
	    break;
	case PROTO_GOPHER:
	    gopherStart(fwdState);
	    break;
	case PROTO_FTP:
	    ftpStart(fwdState);
	    break;
	case PROTO_WAIS:
	    waisStart(fwdState);
	    break;
	case PROTO_CACHEOBJ:
	case PROTO_INTERNAL:
	case PROTO_URN:
	    fatal_dump("Should never get here");
	    break;
	case PROTO_WHOIS:
	    whoisStart(fwdState);
	    break;
	default:
	    debug(17, 1) ("fwdDispatch: Cannot retrieve '%s'\n",
		storeUrl(entry));
	    err = errorCon(ERR_UNSUP_REQ, HTTP_BAD_REQUEST, fwdState->request);
	    fwdFail(fwdState, err);
	    /*
	     * Force a persistent connection to be closed because
	     * some Netscape browsers have a bug that sends CONNECT
	     * requests as GET's over persistent connections.
	     */
	    request->flags.proxy_keepalive = 0;
	    /*
	     * Set the dont_retry flag becuase this is not a
	     * transient (network) error; its a bug.
	     */
	    fwdState->flags.dont_retry = 1;
	    comm_close(fwdState->server_fd);
	    break;
	}
    }
}

static int
fwdReforward(FwdState * fwdState)
{
    StoreEntry *e = fwdState->entry;
    FwdServer *fs = fwdState->servers;
    http_status s;
    assert(e->store_status == STORE_PENDING);
    assert(e->mem_obj);
#if URL_CHECKSUM_DEBUG
    assert(e->mem_obj->chksum == url_checksum(e->mem_obj->url));
#endif
    debug(17, 3) ("fwdReforward: %s?\n", storeUrl(e));
    if (!EBIT_TEST(e->flags, ENTRY_FWD_HDR_WAIT)) {
	debug(17, 3) ("fwdReforward: No, ENTRY_FWD_HDR_WAIT isn't set\n");
	return 0;
    }
    if (fwdState->request->flags.pinned) {
	debug(17, 3) ("fwdReforward: No, Connection oriented authentication is used\n");
	return 0;
    }
    if (fwdState->n_tries > 9)
	return 0;
    if (fwdState->origin_tries > 1)
	return 0;
    if (fwdState->request->flags.body_sent)
	return 0;
    assert(fs);
    fwdState->servers = fs->next;
    fwdServerFree(fs);
    if (fwdState->servers == NULL) {
	debug(17, 3) ("fwdReforward: No forward-servers left\n");
	return 0;
    }
    s = e->mem_obj->reply->sline.status;
    debug(17, 3) ("fwdReforward: status %d\n", (int) s);
    return fwdReforwardableStatus(s);
}

/* PUBLIC FUNCTIONS */

void
fwdServersFree(FwdServer ** FS)
{
    FwdServer *fs;
    while ((fs = *FS)) {
	*FS = fs->next;
	fwdServerFree(fs);
    }
}

void
fwdStartPeer(peer * p, StoreEntry * e, request_t * r)
{
    FwdState *fwdState;
    FwdServer *peer = NULL;
    debug(17, 3) ("fwdStartPeer: '%s'\n", storeUrl(e));
    e->mem_obj->request = requestLink(r);
#if URL_CHECKSUM_DEBUG
    assert(e->mem_obj->chksum == url_checksum(e->mem_obj->url));
#endif
    fwdState = cbdataAlloc(FwdState);
    fwdState->entry = e;
    fwdState->client_fd = -1;
    fwdState->server_fd = -1;
    fwdState->request = requestLink(r);
    fwdState->start = squid_curtime;
    storeLockObject(e);
    if (!fwdState->request->flags.pinned)
	EBIT_SET(e->flags, ENTRY_FWD_HDR_WAIT);
    storeRegisterAbort(e, fwdAbort, fwdState);
    peerAddFwdServer(&peer, p, HIER_DIRECT);
    fwdStartComplete(peer, fwdState);
}

void
fwdStart(int fd, StoreEntry * e, request_t * r)
{
    FwdState *fwdState;
    aclCheck_t ch;
    int answer;
    ErrorState *err;
    /*
     * client_addr == no_addr indicates this is an "internal" request
     * from peer_digest.c, asn.c, netdb.c, etc and should always
     * be allowed.  yuck, I know.
     */
    if (r->client_addr.s_addr != no_addr.s_addr && r->protocol != PROTO_INTERNAL && r->protocol != PROTO_CACHEOBJ) {
	/*      
	 * Check if this host is allowed to fetch MISSES from us (miss_access)
	 */
	memset(&ch, '\0', sizeof(aclCheck_t));
	ch.src_addr = r->client_addr;
	ch.my_addr = r->my_addr;
	ch.my_port = r->my_port;
	ch.request = r;
	answer = aclCheckFast(Config.accessList.miss, &ch);
	if (answer == 0) {
	    err_type page_id;
	    page_id = aclGetDenyInfoPage(&Config.denyInfoList, AclMatchedName, 1);
	    if (page_id == ERR_NONE)
		page_id = ERR_FORWARDING_DENIED;
	    err = errorCon(page_id, HTTP_FORBIDDEN, r);
	    errorAppendEntry(e, err);
	    return;
	}
    }
    debug(17, 3) ("fwdStart: '%s'\n", storeUrl(e));
    if (!e->mem_obj->request)
	e->mem_obj->request = requestLink(r);
#if URL_CHECKSUM_DEBUG
    assert(e->mem_obj->chksum == url_checksum(e->mem_obj->url));
#endif
    if (shutting_down) {
	/* more yuck */
	err = errorCon(ERR_SHUTTING_DOWN, HTTP_SERVICE_UNAVAILABLE, r);
	errorAppendEntry(e, err);
	return;
    }
    switch (r->protocol) {
	/*
	 * Note, don't create fwdState for these requests
	 */
    case PROTO_INTERNAL:
	internalStart(r, e);
	return;
    case PROTO_CACHEOBJ:
	cachemgrStart(fd, r, e);
	return;
    case PROTO_URN:
	urnStart(r, e);
	return;
    default:
	break;
    }
    fwdState = cbdataAlloc(FwdState);
    fwdState->entry = e;
    fwdState->client_fd = fd;
    fwdState->server_fd = -1;
    fwdState->request = requestLink(r);
    fwdState->start = squid_curtime;

#if LINUX_TPROXY
    /* If we need to transparently proxy the request
     * then we need the client source address and port */
    fwdState->src.sin_family = AF_INET;
    fwdState->src.sin_addr = r->client_addr;
    fwdState->src.sin_port = r->client_port;
#endif

    storeLockObject(e);
    if (!fwdState->request->flags.pinned)
	EBIT_SET(e->flags, ENTRY_FWD_HDR_WAIT);
    storeRegisterAbort(e, fwdAbort, fwdState);
    peerSelect(r, e, fwdStartComplete, fwdState);
}

int
fwdCheckDeferRead(int fd, void *data)
{
    StoreEntry *e = data;
    MemObject *mem = e->mem_obj;
    int rc = 0;
    if (mem == NULL)
	return 0;
#if URL_CHECKSUM_DEBUG
    assert(e->mem_obj->chksum == url_checksum(e->mem_obj->url));
#endif
#if DELAY_POOLS
    if (fd < 0)
	(void) 0;
    else if (delayIsNoDelay(fd))
	(void) 0;
    else {
	int i = delayMostBytesWanted(mem, INT_MAX);
	if (0 == i) {
	    /* No storeDeferRead here as it's not store dependent.
	     * Will get changed the day delay pools interact nicely
	     * with the store..
	     */
	    commDeferFD(fd);
	    return 1;
	}
	/* was: rc = -(rc != INT_MAX); */
	else if (INT_MAX == i)
	    rc = 0;
	else
	    rc = -1;
    }
#endif
    if (EBIT_TEST(e->flags, ENTRY_DEFER_READ)) {
	commDeferFD(mem->serverfd);
	return 1;
    }
    if (EBIT_TEST(e->flags, ENTRY_FWD_HDR_WAIT))
	return rc;
    if (EBIT_TEST(e->flags, RELEASE_REQUEST)) {
	/* Just a small safety cap to defer storing more data into the object
	 * if there already is way too much. This handles the case when there
	 * is disk clients pending on a too large object being fetched and a
	 * few other corner cases.
	 */
	if (fd >= 0 && mem->inmem_hi - mem->inmem_lo > SM_PAGE_SIZE + Config.Store.maxInMemObjSize + Config.readAheadGap) {
	    storeDeferRead(e, fd);
	    return 1;
	}
    }
    if (fd >= 0 && mem->inmem_hi - storeLowestMemReaderOffset(e) > Config.readAheadGap) {
	storeDeferRead(e, fd);
	return 1;
    }
    return rc;
}

void
fwdFail(FwdState * fwdState, ErrorState * errorState)
{
    debug(17, 3) ("fwdFail: %s \"%s\"\n\t%s\n",
	err_type_str[errorState->type],
	httpStatusString(errorState->http_status),
	storeUrl(fwdState->entry));
    if (fwdState->err)
	errorStateFree(fwdState->err);
    fwdState->err = errorState;
    if (!errorState->request)
	errorState->request = requestLink(fwdState->request);
}

/*
 * Called when someone else calls StoreAbort() on this entry
 */
static void
fwdAbort(void *data)
{
    FwdState *fwdState = data;
    debug(17, 2) ("fwdAbort: %s\n", storeUrl(fwdState->entry));
    fwdStateFree(fwdState);
}

/*
 * Accounts for closed persistent connections
 */
static void
fwdPeerClosed(int fd, void *data)
{
    peer *p = data;
    p->stats.conn_open--;
}

/*
 * Frees fwdState without closing FD or generating an abort
 */
void
fwdUnregister(int fd, FwdState * fwdState)
{
    debug(17, 3) ("fwdUnregister: %s\n", storeUrl(fwdState->entry));
    assert(fd == fwdState->server_fd);
    assert(fd > -1);
    commSetDefer(fd, NULL, NULL);
    if (EBIT_TEST(fwdState->entry->flags, ENTRY_DEFER_READ))
	storeResumeRead(fwdState->entry);
    comm_remove_close_handler(fd, fwdServerClosed, fwdState);
    fwdState->server_fd = -1;
}

/*
 * server-side modules call fwdComplete() when they are done
 * downloading an object.  Then, we either 1) re-forward the
 * request somewhere else if needed, or 2) call storeComplete()
 * to finish it off
 */
void
fwdComplete(FwdState * fwdState)
{
    StoreEntry *e = fwdState->entry;
    assert(e->store_status == STORE_PENDING);
    debug(17, 3) ("fwdComplete: %s\n\tstatus %d\n", storeUrl(e),
	e->mem_obj->reply->sline.status);
#if URL_CHECKSUM_DEBUG
    assert(e->mem_obj->chksum == url_checksum(e->mem_obj->url));
#endif
    fwdLogReplyStatus(fwdState->n_tries, e->mem_obj->reply->sline.status);
    if (fwdReforward(fwdState)) {
	debug(17, 3) ("fwdComplete: re-forwarding %d %s\n",
	    e->mem_obj->reply->sline.status,
	    storeUrl(e));
	if (fwdState->server_fd > -1)
	    fwdUnregister(fwdState->server_fd, fwdState);
	storeEntryReset(e);
	fwdStartComplete(fwdState->servers, fwdState);
    } else {
	debug(17, 3) ("fwdComplete: not re-forwarding status %d\n",
	    e->mem_obj->reply->sline.status);
	EBIT_CLR(e->flags, ENTRY_FWD_HDR_WAIT);
	storeComplete(e);
	/*
	 * If fwdState isn't associated with a server FD, it
	 * won't get freed unless we do it here.
	 */
	if (fwdState->server_fd < 0)
	    fwdStateFree(fwdState);
    }
}

void
fwdInit(void)
{
    cachemgrRegister("forward",
	"Request Forwarding Statistics",
	fwdStats, 0, 1);
#if WIP_FWD_LOG
    if (logfile)
	(void) 0;
    else if (NULL == Config.Log.forward)
	(void) 0;
    else
	logfile = logfileOpen(Config.Log.forward, 0, 1);
#endif
}

static void
fwdLogReplyStatus(int tries, http_status status)
{
    if (status > HTTP_INVALID_HEADER)
	return;
    assert(tries);
    tries--;
    if (tries > MAX_FWD_STATS_IDX)
	tries = MAX_FWD_STATS_IDX;
    FwdReplyCodes[tries][status]++;
}

static void
fwdStats(StoreEntry * s)
{
    int i;
    int j;
    storeAppendPrintf(s, "Status");
    for (j = 0; j <= MAX_FWD_STATS_IDX; j++) {
	storeAppendPrintf(s, "\ttry#%d", j + 1);
    }
    storeAppendPrintf(s, "\n");
    for (i = 0; i <= (int) HTTP_INVALID_HEADER; i++) {
	if (FwdReplyCodes[0][i] == 0)
	    continue;
	storeAppendPrintf(s, "%3d", i);
	for (j = 0; j <= MAX_FWD_STATS_IDX; j++) {
	    storeAppendPrintf(s, "\t%d", FwdReplyCodes[j][i]);
	}
	storeAppendPrintf(s, "\n");
    }
}

int
fwdReforwardableStatus(http_status s)
{
    switch (s) {
    case HTTP_BAD_GATEWAY:
    case HTTP_GATEWAY_TIMEOUT:
	return 1;
    case HTTP_FORBIDDEN:
    case HTTP_INTERNAL_SERVER_ERROR:
    case HTTP_NOT_IMPLEMENTED:
    case HTTP_SERVICE_UNAVAILABLE:
	return Config.retry.onerror;
    default:
	return 0;
    }
    /* NOTREACHED */
}

#if WIP_FWD_LOG
void
fwdUninit(void)
{
    if (NULL == logfile)
	return;
    logfileClose(logfile);
    logfile = NULL;
}

void
fwdLogRotate(void)
{
    if (logfile)
	logfileRotate(logfile);
}

static void
fwdLog(FwdState * fwdState)
{
    if (NULL == logfile)
	return;
    logfilePrintf(logfile, "%9d.%03d %03d %s %s\n",
	(int) current_time.tv_sec,
	(int) current_time.tv_usec / 1000,
	fwdState->last_status,
	RequestMethodStr[fwdState->request->method],
	fwdState->request->canonical);
}

void
fwdStatus(FwdState * fwdState, http_status s)
{
    fwdState->last_status = s;
}

#endif
