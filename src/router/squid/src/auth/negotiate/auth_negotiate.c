
/*
 * $Id: auth_negotiate.c,v 1.7 2007/01/20 21:13:28 hno Exp $
 *
 * DEBUG: section 29    Negotiate Authenticator
 * AUTHOR: Robert Collins
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

/* The functions in this file handle authentication.
 * They DO NOT perform access control or auditing.
 * See acl.c for access control and client_side.c for auditing */


#include "squid.h"
#include "auth_negotiate.h"

extern AUTHSSETUP authSchemeSetup_negotiate;

static void
authenticateStateFree(authenticateStateData * r)
{
    authenticateAuthUserRequestUnlock(r->auth_user_request);
    r->auth_user_request = NULL;
    cbdataFree(r);
}

/* Negotiate Scheme */
static HLPSCB authenticateNegotiateHandleReply;
static AUTHSACTIVE authenticateNegotiateActive;
static AUTHSAUTHED authNegotiateAuthenticated;
static AUTHSAUTHUSER authenticateNegotiateAuthenticateUser;
static AUTHSCONFIGURED authNegotiateConfigured;
static AUTHSFIXERR authenticateNegotiateFixErrorHeader;
static AUTHSADDHEADER authNegotiateAddHeader;
static AUTHSFREE authenticateNegotiateFreeUser;
static AUTHSDIRECTION authenticateNegotiateDirection;
static AUTHSDECODE authenticateDecodeNegotiateAuth;
static AUTHSDUMP authNegotiateCfgDump;
static AUTHSFREECONFIG authNegotiateFreeConfig;
static AUTHSINIT authNegotiateInit;
static AUTHSONCLOSEC authenticateNegotiateOnCloseConnection;
static AUTHSUSERNAME authenticateNegotiateUsername;
static AUTHSREQFREE authNegotiateAURequestFree;
static AUTHSPARSE authNegotiateParse;
static AUTHSCHECKCONFIG authNegotiateCheckConfig;
static AUTHSSTART authenticateNegotiateStart;
static AUTHSSTATS authenticateNegotiateStats;
static AUTHSSHUTDOWN authNegotiateDone;

static statefulhelper *negotiateauthenticators = NULL;

CBDATA_TYPE(authenticateStateData);

static int authnegotiate_initialised = 0;

static MemPool *negotiate_user_pool = NULL;
static MemPool *negotiate_request_pool = NULL;
static auth_negotiate_config *negotiateConfig = NULL;

static void authenticateNegotiateReleaseServer(negotiate_request_t * negotiate_request);
/*
 *
 * Private Functions
 *
 */

static void
authNegotiateDone(void)
{
    debug(29, 2) ("authNegotiateDone: shutting down Negotiate authentication.\n");
    if (negotiateauthenticators)
	helperStatefulShutdown(negotiateauthenticators);
    authnegotiate_initialised = 0;
    if (!shutting_down)
	return;
    if (negotiateauthenticators)
	helperStatefulFree(negotiateauthenticators);
    negotiateauthenticators = NULL;
    if (negotiate_request_pool) {
	assert(memPoolInUseCount(negotiate_request_pool) == 0);
	memPoolDestroy(negotiate_request_pool);
	negotiate_request_pool = NULL;
    }
    if (negotiate_user_pool) {
	assert(memPoolInUseCount(negotiate_user_pool) == 0);
	memPoolDestroy(negotiate_user_pool);
	negotiate_user_pool = NULL;
    }
    debug(29, 2) ("authNegotiateDone: Negotiate authentication Shutdown.\n");
}

/* free any allocated configuration details */
static void
authNegotiateFreeConfig(authScheme * scheme)
{
    if (negotiateConfig == NULL)
	return;
    assert(negotiateConfig == scheme->scheme_data);
    if (negotiateConfig->authenticate)
	wordlistDestroy(&negotiateConfig->authenticate);
    safe_free(negotiateConfig);
    scheme->scheme_data = NULL;
}

static void
authNegotiateCfgDump(StoreEntry * entry, const char *name, authScheme * scheme)
{
    auth_negotiate_config *config = scheme->scheme_data;
    wordlist *list = config->authenticate;
    storeAppendPrintf(entry, "%s %s program", name, "negotiate");
    while (list != NULL) {
	storeAppendPrintf(entry, " %s", list->key);
	list = list->next;
    }
    storeAppendPrintf(entry, "\n");
    storeAppendPrintf(entry, "%s %s children %d\n", name, "negotiate", config->authenticateChildren);
    storeAppendPrintf(entry, "%s %s keep_alive %s\n", name, "negotiate", config->keep_alive ? "on" : "off");
}

static void
authNegotiateParse(authScheme * scheme, int n_configured, char *param_str)
{
    if (scheme->scheme_data == NULL) {
	assert(negotiateConfig == NULL);
	/* this is the first param to be found */
	scheme->scheme_data = xmalloc(sizeof(auth_negotiate_config));
	memset(scheme->scheme_data, 0, sizeof(auth_negotiate_config));
	negotiateConfig = scheme->scheme_data;
	negotiateConfig->authenticateChildren = 5;
	negotiateConfig->keep_alive = 1;
    }
    negotiateConfig = scheme->scheme_data;
    if (strcasecmp(param_str, "program") == 0) {
	if (negotiateConfig->authenticate)
	    wordlistDestroy(&negotiateConfig->authenticate);
	parse_wordlist(&negotiateConfig->authenticate);
    } else if (strcasecmp(param_str, "children") == 0) {
	parse_int(&negotiateConfig->authenticateChildren);
    } else if (strcasecmp(param_str, "keep_alive") == 0) {
	parse_onoff(&negotiateConfig->keep_alive);
    } else {
	debug(29, 0) ("unrecognised negotiate auth scheme parameter '%s'\n", param_str);
    }
}

static void
authNegotiateCheckConfig(authScheme * scheme)
{
    auth_negotiate_config *config = scheme->scheme_data;
    requirePathnameExists("authparam negotiate program", config->authenticate->key);
}

void
authSchemeSetup_negotiate(authscheme_entry_t * authscheme)
{
    assert(!authnegotiate_initialised);
    authscheme->Active = authenticateNegotiateActive;
    authscheme->configured = authNegotiateConfigured;
    authscheme->parse = authNegotiateParse;
    authscheme->checkconfig = authNegotiateCheckConfig;
    authscheme->dump = authNegotiateCfgDump;
    authscheme->requestFree = authNegotiateAURequestFree;
    authscheme->freeconfig = authNegotiateFreeConfig;
    authscheme->init = authNegotiateInit;
    authscheme->authAuthenticate = authenticateNegotiateAuthenticateUser;
    authscheme->authenticated = authNegotiateAuthenticated;
    authscheme->authFixHeader = authenticateNegotiateFixErrorHeader;
    authscheme->AddHeader = authNegotiateAddHeader;
    authscheme->FreeUser = authenticateNegotiateFreeUser;
    authscheme->authStart = authenticateNegotiateStart;
    authscheme->authStats = authenticateNegotiateStats;
    authscheme->authUserUsername = authenticateNegotiateUsername;
    authscheme->getdirection = authenticateNegotiateDirection;
    authscheme->decodeauth = authenticateDecodeNegotiateAuth;
    authscheme->donefunc = authNegotiateDone;
    authscheme->oncloseconnection = authenticateNegotiateOnCloseConnection;
}

/* Initialize helpers and the like for this auth scheme. Called AFTER parsing the
 * config file */
static void
authNegotiateInit(authScheme * scheme)
{
    static int negotiateinit = 0;
    if (negotiateConfig->authenticate) {
	/*
	 * disable client side request pipelining. There is a race with
	 * Negotiate when the client sends a second request on an Negotiate
	 * connection before the authenticate challenge is sent. With
	 * this patch, the client may fail to authenticate, but squid's
	 * state will be preserved.  Caveats: this should be a post-parse
	 * test, but that can wait for the modular parser to be integrated.
	 */
	if (negotiateConfig->authenticate && Config.onoff.pipeline_prefetch != 0) {
	    debug(29, 1) ("pipeline prefetching incompatile with Negotiate authentication. Disabling pipeline_prefetch\n");
	    Config.onoff.pipeline_prefetch = 0;
	}
	if (!negotiate_user_pool)
	    negotiate_user_pool = memPoolCreate("Negotiate Scheme User Data", sizeof(negotiate_user_t));
	if (!negotiate_request_pool)
	    negotiate_request_pool = memPoolCreate("Negotiate Scheme Request Data", sizeof(negotiate_request_t));
	authnegotiate_initialised = 1;
	if (negotiateauthenticators == NULL)
	    negotiateauthenticators = helperStatefulCreate("negotiateauthenticator");
	negotiateauthenticators->cmdline = negotiateConfig->authenticate;
	negotiateauthenticators->n_to_start = negotiateConfig->authenticateChildren;
	negotiateauthenticators->ipc_type = IPC_STREAM;
	helperStatefulOpenServers(negotiateauthenticators);
	if (!negotiateinit) {
	    cachemgrRegister("negotiateauthenticator",
		"Negotiate User Authenticator Stats",
		authenticateNegotiateStats, 0, 1);
	    negotiateinit++;
	}
	CBDATA_INIT_TYPE(authenticateStateData);
    }
}

static int
authenticateNegotiateActive()
{
    return (authnegotiate_initialised == 1) ? 1 : 0;
}


static int
authNegotiateConfigured()
{
    if (negotiateConfig == NULL) {
	debug(29, 9) ("authNegotiateConfigured: not configured\n");
	return 0;
    }
    if (negotiateConfig->authenticate == NULL) {
	debug(29, 9) ("authNegotiateConfigured: no helper\n");
	return 0;
    }
    if (negotiateConfig->authenticateChildren == 0) {
	debug(29, 9) ("authNegotiateConfigured: no helper children\n");
	return 0;
    }
    debug(29, 9) ("authNegotiateConfigured: returning configured\n");
    return 1;
}

/* Negotiate Scheme */

static int
authenticateNegotiateDirection(auth_user_request_t * auth_user_request)
{
    negotiate_request_t *negotiate_request = auth_user_request->scheme_data;
    /* null auth_user is checked for by authenticateDirection */
    if (negotiate_request->waiting || negotiate_request->client_blob)
	return -1;		/* Need helper response to continue */
    switch (negotiate_request->auth_state) {
    case AUTHENTICATE_STATE_NONE:	/* no progress at all. */
	debug(29, 1) ("authenticateNegotiateDirection: called before Negotiate Authenticate!. Report a bug to squid-dev. au %p\n", auth_user_request);
	return -2;
    case AUTHENTICATE_STATE_NEGOTIATE:		/* send to client */
	assert(negotiate_request->server_blob);
	return 1;
    case AUTHENTICATE_STATE_FAILED:
	return -2;
    case AUTHENTICATE_STATE_FINISHED:	/* do nothing.. */
    case AUTHENTICATE_STATE_DONE:	/* do nothing.. */
	return 0;
    case AUTHENTICATE_STATE_INITIAL:
	debug(29, 1) ("authenticateNegotiateDirection: Unexpected AUTHENTICATE_STATE_INITIAL\n");
	return -2;
    }
    return -2;
}

/*
 * Send the authenticate error header(s). Note: IE has a bug and the Negotiate header
 * must be first. To ensure that, the configure use --enable-auth=negotiate, anything
 * else.
 */
static void
authenticateNegotiateFixErrorHeader(auth_user_request_t * auth_user_request, HttpReply * rep, http_hdr_type type, request_t * request)
{
    negotiate_request_t *negotiate_request;
    if (!request->flags.proxy_keepalive)
	return;
    if (!negotiateConfig->authenticate)
	return;
    /* New request, no user details */
    if (auth_user_request == NULL) {
	debug(29, 9) ("authenticateNegotiateFixErrorHeader: Sending type:%d header: 'Negotiate'\n", type);
	httpHeaderPutStrf(&rep->header, type, "Negotiate");
	if (!negotiateConfig->keep_alive) {
	    /* drop the connection */
	    httpHeaderDelByName(&rep->header, "keep-alive");
	    request->flags.proxy_keepalive = 0;
	}
	return;
    }
    negotiate_request = auth_user_request->scheme_data;
    switch (negotiate_request->auth_state) {
    case AUTHENTICATE_STATE_NONE:
    case AUTHENTICATE_STATE_FAILED:
	debug(29, 9) ("authenticateNegotiateFixErrorHeader: Sending type:%d header: 'Negotiate'\n", type);
	httpHeaderPutStrf(&rep->header, type, "Negotiate");
	/* drop the connection */
	httpHeaderDelByName(&rep->header, "keep-alive");
	request->flags.proxy_keepalive = 0;
	break;
    case AUTHENTICATE_STATE_NEGOTIATE:
	/* we are 'waiting' for a response from the client */
	/* pass the blob to the client */
	debug(29, 9) ("authenticateNegotiateFixErrorHeader: Sending type:%d header: 'Negotiate %s'\n", type, negotiate_request->server_blob);
	httpHeaderPutStrf(&rep->header, type, "Negotiate %s", negotiate_request->server_blob);
	safe_free(negotiate_request->server_blob);
	request->flags.must_keepalive = 1;
	break;
    case AUTHENTICATE_STATE_FINISHED:
    case AUTHENTICATE_STATE_DONE:
	/* Special case when authentication finished, but not allowed by ACL */
	if (negotiate_request->server_blob) {
	    debug(29, 9) ("authenticateNegotiateFixErrorHeader: Sending type:%d header: 'Negotiate %s'\n", type, negotiate_request->server_blob);
	    httpHeaderPutStrf(&rep->header, type, "Negotiate %s", negotiate_request->server_blob);
	    safe_free(negotiate_request->server_blob);
	} else {
	    debug(29, 9) ("authenticateNegotiateFixErrorHeader: Connection authenticated\n");
	    httpHeaderPutStrf(&rep->header, type, "Negotiate");
	}
	break;
    default:
	debug(29, 0) ("authenticateNegotiateFixErrorHeader: state %d.\n", negotiate_request->auth_state);
	fatal("unexpected state in AuthenticateNegotiateFixErrorHeader.\n");
    }
}

/* add the [proxy]authorisation header */
static void
authNegotiateAddHeader(auth_user_request_t * auth_user_request, HttpReply * rep, int accel)
{
    int type;
    negotiate_request_t *negotiate_request;
    if (!auth_user_request)
	return;
    negotiate_request = auth_user_request->scheme_data;
    if (!negotiate_request->server_blob)
	return;

    type = accel ? HDR_WWW_AUTHENTICATE : HDR_PROXY_AUTHENTICATE;

    debug(29, 9) ("authenticateNegotiateAddHeader: Sending type:%d header: 'Negotiate %s'\n", type, negotiate_request->server_blob);
    httpHeaderPutStrf(&rep->header, type, "Negotiate %s", negotiate_request->server_blob);
    safe_free(negotiate_request->server_blob);
}

static void
authNegotiateRequestFree(negotiate_request_t * negotiate_request)
{
    if (!negotiate_request)
	return;
    safe_free(negotiate_request->server_blob);
    safe_free(negotiate_request->client_blob);
    if (negotiate_request->authserver != NULL) {
	debug(29, 9) ("authenticateNegotiateRequestFree: releasing server '%p'\n", negotiate_request->authserver);
	authenticateNegotiateReleaseServer(negotiate_request);
    }
    memPoolFree(negotiate_request_pool, negotiate_request);
}

static void
authNegotiateAURequestFree(auth_user_request_t * auth_user_request)
{
    if (auth_user_request->scheme_data)
	authNegotiateRequestFree((negotiate_request_t *) auth_user_request->scheme_data);
    auth_user_request->scheme_data = NULL;
}

static void
authenticateNegotiateFreeUser(auth_user_t * auth_user)
{
    negotiate_user_t *negotiate_user = auth_user->scheme_data;

    debug(29, 5) ("authenticateNegotiateFreeUser: Clearing Negotiate scheme data\n");
    safe_free(negotiate_user->username);
    memPoolFree(negotiate_user_pool, negotiate_user);
    auth_user->scheme_data = NULL;
}

/* clear the Negotiate helper of being reserved for future requests */
static void
authenticateNegotiateReleaseServer(negotiate_request_t * negotiate_request)
{
    helper_stateful_server *server = negotiate_request->authserver;
    if (!server)
	return;
    debug(29, 9) ("authenticateNegotiateReleaseServer: releasing server '%p'\n", server);
    negotiate_request->authserver = NULL;
    helperStatefulReleaseServer(server);
}

static void
authenticateNegotiateHandleReply(void *data, void *srv, char *reply)
{
    authenticateStateData *r = data;
    int valid;
    auth_user_request_t *auth_user_request;
    auth_user_t *auth_user;
    negotiate_user_t *negotiate_user;
    negotiate_request_t *negotiate_request;
    char *blob, *arg;
    debug(29, 9) ("authenticateNegotiateHandleReply: Helper: '%p' {%s}\n", srv, reply ? reply : "<NULL>");
    valid = cbdataValid(r->data);
    if (!valid) {
	debug(29, 2) ("AuthenticateNegotiateHandleReply: invalid callback data. Releasing helper '%p'.\n", srv);
	negotiate_request = r->auth_user_request->scheme_data;
	if (negotiate_request != NULL) {
	    if (negotiate_request->authserver == NULL)
		negotiate_request->authserver = srv;
	    authenticateNegotiateReleaseServer(negotiate_request);
	}
	cbdataUnlock(r->data);
	authenticateStateFree(r);
	return;
    }
    if (!reply) {
	debug(29, 1) ("AuthenticateNegotiateHandleReply: Helper '%p' crashed!.\n", srv);
	reply = (char *) "BH Internal error";
    }
    auth_user_request = r->auth_user_request;

    negotiate_request = auth_user_request->scheme_data;
    assert(negotiate_request != NULL);

    assert(negotiate_request->waiting);
    negotiate_request->waiting = 0;
    safe_free(negotiate_request->client_blob);

    auth_user = auth_user_request->auth_user;
    assert(auth_user != NULL);
    assert(auth_user->auth_type == AUTH_NEGOTIATE);
    negotiate_user = auth_user_request->auth_user->scheme_data;

    if (negotiate_request->authserver == NULL)
	negotiate_request->authserver = srv;
    else
	assert(negotiate_request->authserver == srv);

    /* seperate out the useful data */
    blob = strchr(reply, ' ');
    if (blob) {
	blob++;
	arg = strchr(blob + 1, ' ');
    } else {
	arg = NULL;
    }

    if (strncasecmp(reply, "TT ", 3) == 0) {
	/* we have been given a blob to send to the client */
	if (arg)
	    *arg++ = '\0';
	safe_free(negotiate_request->server_blob);
	negotiate_request->server_blob = xstrdup(blob);
	negotiate_request->auth_state = AUTHENTICATE_STATE_NEGOTIATE;
	safe_free(auth_user_request->message);
	auth_user_request->message = xstrdup("Authentication in progress");
	debug(29, 4) ("authenticateNegotiateHandleReply: Need to challenge the client with a server blob '%s'\n", blob);
    } else if (strncasecmp(reply, "AF ", 3) == 0 && arg != NULL) {
	/* we're finished, release the helper */
	if (arg)
	    *arg++ = '\0';
	safe_free(negotiate_user->username);
	negotiate_user->username = xstrdup(arg);
	safe_free(auth_user_request->message);
	auth_user_request->message = xstrdup("Login successful");
	safe_free(negotiate_request->server_blob);
	negotiate_request->server_blob = xstrdup(blob);
	debug(29, 4) ("authenticateNegotiateHandleReply: Successfully validated user via Negotiate. Username '%s'\n", arg);
	authenticateNegotiateReleaseServer(negotiate_request);
	negotiate_request->auth_state = AUTHENTICATE_STATE_FINISHED;
    } else if (strncasecmp(reply, "NA ", 3) == 0 && arg != NULL) {
	if (arg)
	    *arg++ = '\0';
	safe_free(auth_user_request->message);
	auth_user_request->message = xstrdup(arg);
	negotiate_request->auth_state = AUTHENTICATE_STATE_FAILED;
	safe_free(negotiate_request->server_blob);
	negotiate_request->server_blob = xstrdup(blob);
	authenticateNegotiateReleaseServer(negotiate_request);
	debug(29, 4) ("authenticateNegotiateHandleReply: Failed validating user via Negotiate. Error returned '%s'\n", arg);
    } else if (strncasecmp(reply, "BH ", 3) == 0) {
	/* TODO kick off a refresh process. This can occur after a YR or after
	 * a KK. If after a YR release the helper and resubmit the request via 
	 * Authenticate Negotiate start. 
	 * If after a KK deny the user's request w/ 407 and mark the helper as 
	 * Needing YR. */
	auth_user_request->message = xstrdup(blob);
	negotiate_request->auth_state = AUTHENTICATE_STATE_FAILED;
	safe_free(negotiate_request->server_blob);
	authenticateNegotiateReleaseServer(negotiate_request);
	debug(29, 1) ("authenticateNegotiateHandleReply: Error validating user via Negotiate. Error returned '%s'\n", reply);
    } else {
	fatalf("authenticateNegotiateHandleReply: *** Unsupported helper response ***, '%s'\n", reply);
    }
    r->handler(r->data, NULL);
    cbdataUnlock(r->data);
    authenticateStateFree(r);
}

static void
authenticateNegotiateStats(StoreEntry * sentry)
{
    storeAppendPrintf(sentry, "Negotiate Authenticator Statistics:\n");
    helperStatefulStats(sentry, negotiateauthenticators);
}

/* send the initial data to a stateful negotiate authenticator module */
static void
authenticateNegotiateStart(auth_user_request_t * auth_user_request, RH * handler, void *data)
{
    authenticateStateData *r = NULL;
    char buf[8192];
    char *sent_string = NULL;
    negotiate_user_t *negotiate_user;
    negotiate_request_t *negotiate_request;
    auth_user_t *auth_user;

    assert(auth_user_request);
    auth_user = auth_user_request->auth_user;
    negotiate_user = auth_user->scheme_data;
    negotiate_request = auth_user_request->scheme_data;
    assert(negotiate_user);
    assert(negotiate_request);
    assert(handler);
    assert(data);
    assert(auth_user->auth_type == AUTH_NEGOTIATE);
    debug(29, 9) ("authenticateNegotiateStart: auth state '%d'\n", negotiate_request->auth_state);
    sent_string = negotiate_request->client_blob;

    debug(29, 9) ("authenticateNegotiateStart: state '%d'\n", negotiate_request->auth_state);
    debug(29, 9) ("authenticateNegotiateStart: '%s'\n", sent_string);
    if (negotiateConfig->authenticate == NULL) {
	debug(29, 0) ("authenticateNegotiateStart: no Negotiate program specified:'%s'\n", sent_string);
	handler(data, NULL);
	return;
    }
    /* Send blob to helper */
    r = cbdataAlloc(authenticateStateData);
    r->handler = handler;
    cbdataLock(data);
    r->data = data;
    r->auth_user_request = auth_user_request;
    authenticateAuthUserRequestLock(r->auth_user_request);
    if (negotiate_request->auth_state == AUTHENTICATE_STATE_INITIAL)
	snprintf(buf, 8192, "YR %s\n", sent_string);
    else
	snprintf(buf, 8192, "KK %s\n", sent_string);
    negotiate_request->waiting = 1;
    safe_free(negotiate_request->client_blob);
    helperStatefulSubmit(negotiateauthenticators, buf, authenticateNegotiateHandleReply, r, negotiate_request->authserver);
}

/* clear any connection related authentication details */
static void
authenticateNegotiateOnCloseConnection(ConnStateData * conn)
{
    negotiate_request_t *negotiate_request;
    assert(conn != NULL);
    if (conn->auth_user_request != NULL) {
	assert(conn->auth_user_request->scheme_data != NULL);
	negotiate_request = conn->auth_user_request->scheme_data;
	assert(negotiate_request->conn == conn);
	if (negotiate_request->authserver != NULL)
	    authenticateNegotiateReleaseServer(negotiate_request);
	/* unlock the connection based lock */
	debug(29, 9) ("authenticateNegotiateOnCloseConnection: Unlocking auth_user from the connection.\n");
	/* minor abstraction break here: FIXME */
	/* Ensure that the auth user request will be getting closed */
	/* IFF we start persisting the struct after the conn closes - say for logging
	 * then this test may become invalid
	 */
	assert(conn->auth_user_request->references == 1);
	authenticateAuthUserRequestUnlock(conn->auth_user_request);
	conn->auth_user_request = NULL;
    }
}

/* authenticateUserUsername: return a pointer to the username in the */
static char *
authenticateNegotiateUsername(auth_user_t * auth_user)
{
    negotiate_user_t *negotiate_user = auth_user->scheme_data;
    if (negotiate_user)
	return negotiate_user->username;
    return NULL;
}

/*
 * Called on the initial request only, to set things up for later processing
 */

static void
authenticateDecodeNegotiateAuth(auth_user_request_t * auth_user_request, const char *proxy_auth)
{
    dlink_node *node;
    assert(auth_user_request->auth_user == NULL);
    auth_user_request->auth_user = authenticateAuthUserNew("negotiate");
    auth_user_request->auth_user->auth_type = AUTH_NEGOTIATE;
    auth_user_request->auth_user->scheme_data = memPoolAlloc(negotiate_user_pool);
    auth_user_request->scheme_data = memPoolAlloc(negotiate_request_pool);
    memset(auth_user_request->scheme_data, '\0', sizeof(negotiate_request_t));
    /* lock for the auth_user_request link */
    authenticateAuthUserLock(auth_user_request->auth_user);
    node = dlinkNodeNew();
    dlinkAdd(auth_user_request, node, &auth_user_request->auth_user->requests);

    /* the helper does the rest, with data collected in
     * authenticateNegotiateAuthenticateUser */
    debug(29, 9) ("authenticateDecodeNegotiateAuth: Negotiate authentication\n");
    return;
}

static int
authenticateNegotiatecmpUsername(negotiate_user_t * u1, negotiate_user_t * u2)
{
    return strcmp(u1->username, u2->username);
}


static int
authNegotiateAuthenticated(auth_user_request_t * auth_user_request)
{
    negotiate_request_t *negotiate_request = auth_user_request->scheme_data;
    if (negotiate_request->auth_state == AUTHENTICATE_STATE_DONE)
	return 1;
    debug(29, 9) ("User not fully authenticated.\n");
    return 0;
}

static void
authenticateNegotiateAuthenticateUser(auth_user_request_t * auth_user_request, request_t * request, ConnStateData * conn, http_hdr_type type)
{
    const char *proxy_auth, *blob;
    auth_user_hash_pointer *usernamehash;
    auth_user_t *auth_user;
    negotiate_request_t *negotiate_request;
    negotiate_user_t *negotiate_user;

    auth_user = auth_user_request->auth_user;
    assert(auth_user);
    assert(auth_user->auth_type == AUTH_NEGOTIATE);
    assert(auth_user->scheme_data != NULL);
    assert(auth_user_request->scheme_data != NULL);
    negotiate_user = auth_user->scheme_data;
    negotiate_request = auth_user_request->scheme_data;
    /* Check that we are in the client side, where we can generate
     * auth challenges */
    if (!conn) {
	negotiate_request->auth_state = AUTHENTICATE_STATE_FAILED;
	debug(29, 1) ("authenticateNegotiateAuthenticateUser: attempt to perform authentication without a connection!\n");
	return;
    }
    if (negotiate_request->waiting) {
	debug(29, 1) ("authenticateNegotiateAuthenticateUser: waiting for helper reply!\n");
	return;
    }
    if (negotiate_request->server_blob) {
	debug(29, 2) ("authenticateNegotiateAuthenticateUser: need to challenge client '%s'!\n", negotiate_request->server_blob);
	return;
    }
    /* get header */
    proxy_auth = httpHeaderGetStr(&request->header, type);
    blob = proxy_auth;
    while (xisspace(*blob) && *blob)
	blob++;
    while (!xisspace(*blob) && *blob)
	blob++;
    while (xisspace(*blob) && *blob)
	blob++;

    switch (negotiate_request->auth_state) {
    case AUTHENTICATE_STATE_NONE:
	/* we've recieved a negotiate request. pass to a helper */
	debug(29, 9) ("authenticateNegotiateAuthenticateUser: auth state negotiate none. %s\n", proxy_auth);
	negotiate_request->auth_state = AUTHENTICATE_STATE_INITIAL;
	safe_free(negotiate_request->client_blob);
	negotiate_request->client_blob = xstrdup(blob);
	conn->auth_type = AUTH_NEGOTIATE;
	conn->auth_user_request = auth_user_request;
	negotiate_request->conn = conn;
	/* and lock for the connection duration */
	debug(29, 9) ("authenticateNegotiateAuthenticateUser: Locking auth_user from the connection.\n");
	authenticateAuthUserRequestLock(auth_user_request);
	return;
	break;
    case AUTHENTICATE_STATE_INITIAL:
	debug(29, 1) ("authenticateNegotiateAuthenticateUser: need to ask helper!\n");
	return;
	break;
    case AUTHENTICATE_STATE_NEGOTIATE:
	/* we should have recieved a blob from the clien. pass it to the same 
	 * helper process */
	debug(29, 9) ("authenticateNegotiateAuthenticateUser: auth state challenge with header %s.\n", proxy_auth);
	/* do a cache lookup here. If it matches it's a successful negotiate 
	 * challenge - release the helper and use the existing auth_user 
	 * details. */
	safe_free(negotiate_request->client_blob);
	negotiate_request->client_blob = xstrdup(blob);
	return;
	break;
    case AUTHENTICATE_STATE_FINISHED:
	/* this connection is authenticated */
	debug(29, 4) ("authenticated user %s\n", negotiate_user->username);
	/* see if this is an existing user with a different proxy_auth 
	 * string */
	usernamehash = hash_lookup(proxy_auth_username_cache, negotiate_user->username);
	if (usernamehash) {
	    while (usernamehash && (usernamehash->auth_user->auth_type != auth_user->auth_type || authenticateNegotiatecmpUsername(usernamehash->auth_user->scheme_data, negotiate_user) != 0))
		usernamehash = usernamehash->next;
	}
	if (usernamehash) {
	    /* we can't seamlessly recheck the username due to the 
	     * challenge nature of the protocol. Just free the 
	     * temporary auth_user */
	    authenticateAuthUserMerge(auth_user, usernamehash->auth_user);
	    auth_user = usernamehash->auth_user;
	    auth_user_request->auth_user = auth_user;
	} else {
	    /* store user in hash's */
	    authenticateUserNameCacheAdd(auth_user);
	}
	/* set these to now because this is either a new login from an 
	 * existing user or a new user */
	auth_user->expiretime = current_time.tv_sec;
	authenticateNegotiateReleaseServer(negotiate_request);
	negotiate_request->auth_state = AUTHENTICATE_STATE_DONE;
	return;
    case AUTHENTICATE_STATE_DONE:
	fatal("authenticateNegotiateAuthenticateUser: unexpect auth state DONE! Report a bug to the squid developers.\n");
	break;
    case AUTHENTICATE_STATE_FAILED:
	/* we've failed somewhere in authentication */
	debug(29, 9) ("authenticateNegotiateAuthenticateUser: auth state negotiate failed. %s\n", proxy_auth);
	return;
    }
    return;
}
