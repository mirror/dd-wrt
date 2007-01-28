
/*
 * $Id: auth_ntlm.c,v 1.37 2007/01/20 21:13:28 hno Exp $
 *
 * DEBUG: section 29    NTLM Authenticator
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
#include "auth_ntlm.h"

extern AUTHSSETUP authSchemeSetup_ntlm;

static void
authenticateStateFree(authenticateStateData * r)
{
    authenticateAuthUserRequestUnlock(r->auth_user_request);
    r->auth_user_request = NULL;
    cbdataFree(r);
}

/* NTLM Scheme */
static HLPSCB authenticateNTLMHandleReply;
static AUTHSACTIVE authenticateNTLMActive;
static AUTHSAUTHED authNTLMAuthenticated;
static AUTHSAUTHUSER authenticateNTLMAuthenticateUser;
static AUTHSCONFIGURED authNTLMConfigured;
static AUTHSFIXERR authenticateNTLMFixErrorHeader;
static AUTHSFREE authenticateNTLMFreeUser;
static AUTHSDIRECTION authenticateNTLMDirection;
static AUTHSDECODE authenticateDecodeNTLMAuth;
static AUTHSDUMP authNTLMCfgDump;
static AUTHSFREECONFIG authNTLMFreeConfig;
static AUTHSINIT authNTLMInit;
static AUTHSONCLOSEC authenticateNTLMOnCloseConnection;
static AUTHSUSERNAME authenticateNTLMUsername;
static AUTHSREQFREE authNTLMAURequestFree;
static AUTHSPARSE authNTLMParse;
static AUTHSCHECKCONFIG authNTLMCheckConfig;
static AUTHSSTART authenticateNTLMStart;
static AUTHSSTATS authenticateNTLMStats;
static AUTHSSHUTDOWN authNTLMDone;

static statefulhelper *ntlmauthenticators = NULL;

CBDATA_TYPE(authenticateStateData);

static int authntlm_initialised = 0;

static MemPool *ntlm_user_pool = NULL;
static MemPool *ntlm_request_pool = NULL;
static auth_ntlm_config *ntlmConfig = NULL;

static void authenticateNTLMReleaseServer(ntlm_request_t * ntlm_request);
/*
 *
 * Private Functions
 *
 */

static void
authNTLMDone(void)
{
    debug(29, 2) ("authNTLMDone: shutting down NTLM authentication.\n");
    if (ntlmauthenticators)
	helperStatefulShutdown(ntlmauthenticators);
    authntlm_initialised = 0;
    if (!shutting_down)
	return;
    if (ntlmauthenticators)
	helperStatefulFree(ntlmauthenticators);
    ntlmauthenticators = NULL;
    if (ntlm_request_pool) {
	memPoolDestroy(ntlm_request_pool);
	ntlm_request_pool = NULL;
    }
    if (ntlm_user_pool) {
	memPoolDestroy(ntlm_user_pool);
	ntlm_user_pool = NULL;
    }
    debug(29, 2) ("authNTLMDone: NTLM authentication Shutdown.\n");
}

/* free any allocated configuration details */
static void
authNTLMFreeConfig(authScheme * scheme)
{
    if (ntlmConfig == NULL)
	return;
    assert(ntlmConfig == scheme->scheme_data);
    if (ntlmConfig->authenticate)
	wordlistDestroy(&ntlmConfig->authenticate);
    safe_free(ntlmConfig);
    scheme->scheme_data = NULL;
}

static void
authNTLMCfgDump(StoreEntry * entry, const char *name, authScheme * scheme)
{
    auth_ntlm_config *config = scheme->scheme_data;
    wordlist *list = config->authenticate;
    storeAppendPrintf(entry, "%s %s program", name, "ntlm");
    while (list != NULL) {
	storeAppendPrintf(entry, " %s", list->key);
	list = list->next;
    }
    storeAppendPrintf(entry, "\n");
    storeAppendPrintf(entry, "%s %s children %d\n", name, "ntlm", config->authenticateChildren);
    storeAppendPrintf(entry, "%s %s keep_alive %s\n", name, "ntlm", config->keep_alive ? "on" : "off");
}

static void
authNTLMParse(authScheme * scheme, int n_configured, char *param_str)
{
    if (scheme->scheme_data == NULL) {
	assert(ntlmConfig == NULL);
	/* this is the first param to be found */
	scheme->scheme_data = xmalloc(sizeof(auth_ntlm_config));
	memset(scheme->scheme_data, 0, sizeof(auth_ntlm_config));
	ntlmConfig = scheme->scheme_data;
	ntlmConfig->authenticateChildren = 5;
	ntlmConfig->keep_alive = 1;
    }
    ntlmConfig = scheme->scheme_data;
    if (strcasecmp(param_str, "program") == 0) {
	if (ntlmConfig->authenticate)
	    wordlistDestroy(&ntlmConfig->authenticate);
	parse_wordlist(&ntlmConfig->authenticate);
    } else if (strcasecmp(param_str, "children") == 0) {
	parse_int(&ntlmConfig->authenticateChildren);
    } else if (strcasecmp(param_str, "keep_alive") == 0) {
	parse_onoff(&ntlmConfig->keep_alive);
    } else {
	debug(29, 0) ("unrecognised ntlm auth scheme parameter '%s'\n", param_str);
    }
}

static void
authNTLMCheckConfig(authScheme * scheme)
{
    auth_ntlm_config *config = scheme->scheme_data;
    requirePathnameExists("authparam ntlm program", config->authenticate->key);
}

void
authSchemeSetup_ntlm(authscheme_entry_t * authscheme)
{
    assert(!authntlm_initialised);
    authscheme->Active = authenticateNTLMActive;
    authscheme->configured = authNTLMConfigured;
    authscheme->parse = authNTLMParse;
    authscheme->checkconfig = authNTLMCheckConfig;
    authscheme->dump = authNTLMCfgDump;
    authscheme->requestFree = authNTLMAURequestFree;
    authscheme->freeconfig = authNTLMFreeConfig;
    authscheme->init = authNTLMInit;
    authscheme->authAuthenticate = authenticateNTLMAuthenticateUser;
    authscheme->authenticated = authNTLMAuthenticated;
    authscheme->authFixHeader = authenticateNTLMFixErrorHeader;
    authscheme->FreeUser = authenticateNTLMFreeUser;
    authscheme->authStart = authenticateNTLMStart;
    authscheme->authStats = authenticateNTLMStats;
    authscheme->authUserUsername = authenticateNTLMUsername;
    authscheme->getdirection = authenticateNTLMDirection;
    authscheme->decodeauth = authenticateDecodeNTLMAuth;
    authscheme->donefunc = authNTLMDone;
    authscheme->oncloseconnection = authenticateNTLMOnCloseConnection;
}

/* Initialize helpers and the like for this auth scheme. Called AFTER parsing the
 * config file */
static void
authNTLMInit(authScheme * scheme)
{
    static int ntlminit = 0;
    if (ntlmConfig->authenticate) {
	/*
	 * disable client side request pipelining. There is a race with
	 * NTLM when the client sends a second request on an NTLM
	 * connection before the authenticate challenge is sent. With
	 * this patch, the client may fail to authenticate, but squid's
	 * state will be preserved.
	 */
	if (ntlmConfig->authenticate && Config.onoff.pipeline_prefetch != 0) {
	    debug(29, 1) ("pipeline prefetching incompatile with NTLM authentication. Disabling pipeline_prefetch\n");
	    Config.onoff.pipeline_prefetch = 0;
	}
	if (!ntlm_user_pool)
	    ntlm_user_pool = memPoolCreate("NTLM Scheme User Data", sizeof(ntlm_user_t));
	if (!ntlm_request_pool)
	    ntlm_request_pool = memPoolCreate("NTLM Scheme Request Data", sizeof(ntlm_request_t));
	authntlm_initialised = 1;
	if (ntlmauthenticators == NULL)
	    ntlmauthenticators = helperStatefulCreate("ntlmauthenticator");
	ntlmauthenticators->cmdline = ntlmConfig->authenticate;
	ntlmauthenticators->n_to_start = ntlmConfig->authenticateChildren;
	ntlmauthenticators->ipc_type = IPC_STREAM;
	helperStatefulOpenServers(ntlmauthenticators);
	if (!ntlminit) {
	    cachemgrRegister("ntlmauthenticator",
		"NTLM User Authenticator Stats",
		authenticateNTLMStats, 0, 1);
	    ntlminit++;
	}
	CBDATA_INIT_TYPE(authenticateStateData);
    }
}

static int
authenticateNTLMActive()
{
    return (authntlm_initialised == 1) ? 1 : 0;
}


static int
authNTLMConfigured()
{
    if (ntlmConfig == NULL) {
	debug(29, 9) ("authNTLMConfigured: not configured\n");
	return 0;
    }
    if (ntlmConfig->authenticate == NULL) {
	debug(29, 9) ("authNTLMConfigured: no helper\n");
	return 0;
    }
    if (ntlmConfig->authenticateChildren == 0) {
	debug(29, 9) ("authNTLMConfigured: no helper children\n");
	return 0;
    }
    debug(29, 9) ("authNTLMConfigured: returning configured\n");
    return 1;
}

/* NTLM Scheme */

static int
authenticateNTLMDirection(auth_user_request_t * auth_user_request)
{
    ntlm_request_t *ntlm_request = auth_user_request->scheme_data;
    /* null auth_user is checked for by authenticateDirection */
    if (ntlm_request->waiting || ntlm_request->client_blob)
	return -1;		/* Need helper response to continue */
    switch (ntlm_request->auth_state) {
    case AUTHENTICATE_STATE_NONE:	/* no progress at all. */
	debug(29, 1) ("authenticateNTLMDirection: called before NTLM Authenticate!. Report a bug to squid-dev. au %p\n", auth_user_request);
	return -2;
    case AUTHENTICATE_STATE_NEGOTIATE:		/* send to client */
	assert(ntlm_request->server_blob);
	return 1;
    case AUTHENTICATE_STATE_FAILED:
	return -2;
    case AUTHENTICATE_STATE_FINISHED:	/* do nothing.. */
    case AUTHENTICATE_STATE_DONE:	/* do nothing.. */
	return 0;
    case AUTHENTICATE_STATE_INITIAL:
	debug(29, 1) ("authenticateNTLMDirection: Unexpected AUTHENTICATE_STATE_INITIAL\n");
	return -2;
    }
    return -2;
}

/*
 * Send the authenticate error header(s). Note: IE has a bug and the NTLM header
 * must be first. To ensure that, the configure use --enable-auth=ntlm, anything
 * else.
 */
static void
authenticateNTLMFixErrorHeader(auth_user_request_t * auth_user_request, HttpReply * rep, http_hdr_type type, request_t * request)
{
    ntlm_request_t *ntlm_request;
    if (!ntlmConfig->authenticate)
	return;
    /* New request, no user details */
    if (auth_user_request == NULL) {
	debug(29, 9) ("authenticateNTLMFixErrorHeader: Sending type:%d header: 'NTLM'\n", type);
	httpHeaderPutStrf(&rep->header, type, "NTLM");
	if (!ntlmConfig->keep_alive) {
	    /* drop the connection */
	    httpHeaderDelByName(&rep->header, "keep-alive");
	    request->flags.proxy_keepalive = 0;
	}
	return;
    }
    ntlm_request = auth_user_request->scheme_data;
    switch (ntlm_request->auth_state) {
    case AUTHENTICATE_STATE_NONE:
    case AUTHENTICATE_STATE_FAILED:
	debug(29, 9) ("authenticateNTLMFixErrorHeader: Sending type:%d header: 'NTLM'\n", type);
	httpHeaderPutStrf(&rep->header, type, "NTLM");
	/* drop the connection */
	httpHeaderDelByName(&rep->header, "keep-alive");
	request->flags.proxy_keepalive = 0;
	break;
    case AUTHENTICATE_STATE_NEGOTIATE:
	/* we are 'waiting' for a response from the client */
	/* pass the blob to the client */
	debug(29, 9) ("authenticateNTLMFixErrorHeader: Sending type:%d header: 'NTLM %s'\n", type, ntlm_request->server_blob);
	httpHeaderPutStrf(&rep->header, type, "NTLM %s", ntlm_request->server_blob);
	safe_free(ntlm_request->server_blob);
	request->flags.must_keepalive = 1;
	break;
    case AUTHENTICATE_STATE_FINISHED:
    case AUTHENTICATE_STATE_DONE:
	/* Special case when authentication finished, but not allowed by ACL */
	debug(29, 9) ("authenticateNTLMFixErrorHeader: Sending type:%d header: 'NTLM'\n", type);
	httpHeaderPutStrf(&rep->header, type, "NTLM");
	break;
    default:
	debug(29, 0) ("authenticateNTLMFixErrorHeader: state %d.\n", ntlm_request->auth_state);
	fatal("unexpected state in AuthenticateNTLMFixErrorHeader.\n");
    }
}

static void
authNTLMRequestFree(ntlm_request_t * ntlm_request)
{
    if (!ntlm_request)
	return;
    safe_free(ntlm_request->server_blob);
    safe_free(ntlm_request->client_blob);
    if (ntlm_request->authserver != NULL) {
	debug(29, 9) ("authenticateNTLMRequestFree: releasing server '%p'\n", ntlm_request->authserver);
	authenticateNTLMReleaseServer(ntlm_request);
    }
    memPoolFree(ntlm_request_pool, ntlm_request);
}

static void
authNTLMAURequestFree(auth_user_request_t * auth_user_request)
{
    if (auth_user_request->scheme_data)
	authNTLMRequestFree((ntlm_request_t *) auth_user_request->scheme_data);
    auth_user_request->scheme_data = NULL;
}

static void
authenticateNTLMFreeUser(auth_user_t * auth_user)
{
    ntlm_user_t *ntlm_user = auth_user->scheme_data;

    debug(29, 5) ("authenticateNTLMFreeUser: Clearing NTLM scheme data\n");
    safe_free(ntlm_user->username);
    memPoolFree(ntlm_user_pool, ntlm_user);
    auth_user->scheme_data = NULL;
}

/* clear the NTLM helper of being reserved for future requests */
static void
authenticateNTLMReleaseServer(ntlm_request_t * ntlm_request)
{
    helper_stateful_server *server = ntlm_request->authserver;
    if (!server)
	return;
    debug(29, 9) ("authenticateNTLMReleaseServer: releasing server '%p'\n", server);
    ntlm_request->authserver = NULL;
    helperStatefulReleaseServer(server);
}

static void
authenticateNTLMHandleReply(void *data, void *srv, char *reply)
{
    authenticateStateData *r = data;
    int valid;
    auth_user_request_t *auth_user_request;
    auth_user_t *auth_user;
    ntlm_user_t *ntlm_user;
    ntlm_request_t *ntlm_request;
    char *blob;
    debug(29, 9) ("authenticateNTLMHandleReply: Helper: '%p' {%s}\n", srv, reply ? reply : "<NULL>");
    valid = cbdataValid(r->data);
    if (!valid) {
	debug(29, 2) ("AuthenticateNTLMHandleReply: invalid callback data. Releasing helper '%p'.\n", srv);
	ntlm_request = r->auth_user_request->scheme_data;
	if (ntlm_request != NULL) {
	    if (ntlm_request->authserver == NULL)
		ntlm_request->authserver = srv;
	    authenticateNTLMReleaseServer(ntlm_request);
	}
	cbdataUnlock(r->data);
	authenticateStateFree(r);
	return;
    }
    if (!reply) {
	debug(29, 1) ("AuthenticateNTLMHandleReply: Helper '%p' crashed!.\n", srv);
	reply = (char *) "BH Internal error";
    }
    auth_user_request = r->auth_user_request;

    ntlm_request = auth_user_request->scheme_data;
    assert(ntlm_request != NULL);

    assert(ntlm_request->waiting);
    ntlm_request->waiting = 0;
    safe_free(ntlm_request->client_blob);

    auth_user = auth_user_request->auth_user;
    assert(auth_user != NULL);
    assert(auth_user->auth_type == AUTH_NTLM);
    ntlm_user = auth_user_request->auth_user->scheme_data;

    if (ntlm_request->authserver == NULL)
	ntlm_request->authserver = srv;
    else
	assert(ntlm_request->authserver == srv);

    /* seperate out the useful data */
    blob = strchr(reply, ' ');
    if (blob) {
	blob++;
    }
    if (strncasecmp(reply, "TT ", 3) == 0) {
	/* we have been given a blob to send to the client */
	safe_free(ntlm_request->server_blob);
	ntlm_request->server_blob = xstrdup(blob);
	ntlm_request->auth_state = AUTHENTICATE_STATE_NEGOTIATE;
	safe_free(auth_user_request->message);
	auth_user_request->message = xstrdup("Authentication in progress");
	debug(29, 4) ("authenticateNTLMHandleReply: Need to challenge the client with a server blob '%s'\n", blob);
    } else if (strncasecmp(reply, "AF ", 3) == 0) {
	/* we're finished, release the helper */
	safe_free(ntlm_user->username);
	ntlm_user->username = xstrdup(blob);
	safe_free(auth_user_request->message);
	auth_user_request->message = xstrdup("Login successful");
	debug(29, 4) ("authenticateNTLMHandleReply: Successfully validated user via NTLM. Username '%s'\n", blob);
	authenticateNTLMReleaseServer(ntlm_request);
	ntlm_request->auth_state = AUTHENTICATE_STATE_FINISHED;
    } else if (strncasecmp(reply, "NA ", 3) == 0) {
	safe_free(auth_user_request->message);
	auth_user_request->message = xstrdup(blob);
	ntlm_request->auth_state = AUTHENTICATE_STATE_FAILED;
	authenticateNTLMReleaseServer(ntlm_request);
	debug(29, 4) ("authenticateNTLMHandleReply: Failed validating user via NTLM. Error returned '%s'\n", blob);
    } else if (strncasecmp(reply, "BH ", 3) == 0) {
	/* TODO kick off a refresh process. This can occur after a YR or after
	 * a KK. If after a YR release the helper and resubmit the request via 
	 * Authenticate NTLM start. 
	 * If after a KK deny the user's request w/ 407 and mark the helper as 
	 * Needing YR. */
	auth_user_request->message = xstrdup(blob);
	ntlm_request->auth_state = AUTHENTICATE_STATE_FAILED;
	safe_free(ntlm_request->server_blob);
	authenticateNTLMReleaseServer(ntlm_request);
	debug(29, 1) ("authenticateNTLMHandleReply: Error validating user via NTLM. Error returned '%s'\n", reply);
    } else {
	fatalf("authenticateNTLMHandleReply: *** Unsupported helper response ***, '%s'\n", reply);
    }
    r->handler(r->data, NULL);
    cbdataUnlock(r->data);
    authenticateStateFree(r);
}

static void
authenticateNTLMStats(StoreEntry * sentry)
{
    storeAppendPrintf(sentry, "NTLM Authenticator Statistics:\n");
    helperStatefulStats(sentry, ntlmauthenticators);
}

/* send the initial data to a stateful ntlm authenticator module */
static void
authenticateNTLMStart(auth_user_request_t * auth_user_request, RH * handler, void *data)
{
    authenticateStateData *r = NULL;
    char buf[8192];
    char *sent_string = NULL;
    ntlm_user_t *ntlm_user;
    ntlm_request_t *ntlm_request;
    auth_user_t *auth_user;

    assert(auth_user_request);
    auth_user = auth_user_request->auth_user;
    ntlm_user = auth_user->scheme_data;
    ntlm_request = auth_user_request->scheme_data;
    assert(ntlm_user);
    assert(ntlm_request);
    assert(handler);
    assert(data);
    assert(auth_user->auth_type == AUTH_NTLM);
    debug(29, 9) ("authenticateNTLMStart: auth state '%d'\n", ntlm_request->auth_state);
    sent_string = ntlm_request->client_blob;

    debug(29, 9) ("authenticateNTLMStart: state '%d'\n", ntlm_request->auth_state);
    debug(29, 9) ("authenticateNTLMStart: '%s'\n", sent_string);
    if (ntlmConfig->authenticate == NULL) {
	debug(29, 0) ("authenticateNTLMStart: no NTLM program specified:'%s'\n", sent_string);
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
    if (ntlm_request->auth_state == AUTHENTICATE_STATE_INITIAL) {
	snprintf(buf, 8192, "YR %s\n", sent_string);
    } else {
	snprintf(buf, 8192, "KK %s\n", sent_string);
    }
    ntlm_request->waiting = 1;
    safe_free(ntlm_request->client_blob);
    helperStatefulSubmit(ntlmauthenticators, buf, authenticateNTLMHandleReply, r, ntlm_request->authserver);
}

/* clear any connection related authentication details */
static void
authenticateNTLMOnCloseConnection(ConnStateData * conn)
{
    ntlm_request_t *ntlm_request;
    assert(conn != NULL);
    if (conn->auth_user_request != NULL) {
	assert(conn->auth_user_request->scheme_data != NULL);
	ntlm_request = conn->auth_user_request->scheme_data;
	assert(ntlm_request->conn == conn);
	if (ntlm_request->authserver != NULL)
	    authenticateNTLMReleaseServer(ntlm_request);
	/* unlock the connection based lock */
	debug(29, 9) ("authenticateNTLMOnCloseConnection: Unlocking auth_user from the connection.\n");
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
authenticateNTLMUsername(auth_user_t * auth_user)
{
    ntlm_user_t *ntlm_user = auth_user->scheme_data;
    if (ntlm_user)
	return ntlm_user->username;
    return NULL;
}

/*
 * Called on the initial request only, to set things up for later processing
 */

static void
authenticateDecodeNTLMAuth(auth_user_request_t * auth_user_request, const char *proxy_auth)
{
    dlink_node *node;
    assert(auth_user_request->auth_user == NULL);
    auth_user_request->auth_user = authenticateAuthUserNew("ntlm");
    auth_user_request->auth_user->auth_type = AUTH_NTLM;
    auth_user_request->auth_user->scheme_data = memPoolAlloc(ntlm_user_pool);
    auth_user_request->scheme_data = memPoolAlloc(ntlm_request_pool);
    memset(auth_user_request->scheme_data, '\0', sizeof(ntlm_request_t));
    /* lock for the auth_user_request link */
    authenticateAuthUserLock(auth_user_request->auth_user);
    node = dlinkNodeNew();
    dlinkAdd(auth_user_request, node, &auth_user_request->auth_user->requests);

    /* the helper does the rest, with data collected in
     * authenticateNTLMAuthenticateUser */
    debug(29, 9) ("authenticateDecodeNTLMAuth: NTLM authentication\n");
    return;
}

static int
authenticateNTLMcmpUsername(ntlm_user_t * u1, ntlm_user_t * u2)
{
    return strcmp(u1->username, u2->username);
}


static int
authNTLMAuthenticated(auth_user_request_t * auth_user_request)
{
    ntlm_request_t *ntlm_request = auth_user_request->scheme_data;
    if (ntlm_request->auth_state == AUTHENTICATE_STATE_DONE)
	return 1;
    debug(29, 9) ("User not fully authenticated.\n");
    return 0;
}

static void
authenticateNTLMAuthenticateUser(auth_user_request_t * auth_user_request, request_t * request, ConnStateData * conn, http_hdr_type type)
{
    const char *proxy_auth, *blob;
    auth_user_hash_pointer *usernamehash;
    auth_user_t *auth_user;
    ntlm_request_t *ntlm_request;
    ntlm_user_t *ntlm_user;

    auth_user = auth_user_request->auth_user;
    assert(auth_user);
    assert(auth_user->auth_type == AUTH_NTLM);
    assert(auth_user->scheme_data != NULL);
    assert(auth_user_request->scheme_data != NULL);
    ntlm_user = auth_user->scheme_data;
    ntlm_request = auth_user_request->scheme_data;
    /* Check that we are in the client side, where we can generate
     * auth challenges */
    if (!conn) {
	ntlm_request->auth_state = AUTHENTICATE_STATE_FAILED;
	debug(29, 1) ("authenticateNTLMAuthenticateUser: attempt to perform authentication without a connection!\n");
	return;
    }
    if (ntlm_request->waiting) {
	debug(29, 1) ("authenticateNTLMAuthenticateUser: waiting for helper reply!\n");
	return;
    }
    if (ntlm_request->server_blob) {
	debug(29, 2) ("authenticateNTLMAuthenticateUser: need to challenge client '%s'!\n", ntlm_request->server_blob);
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

    switch (ntlm_request->auth_state) {
    case AUTHENTICATE_STATE_NONE:
	/* we've recieved a ntlm request. pass to a helper */
	debug(29, 9) ("authenticateNTLMAuthenticateUser: auth state ntlm none. %s\n", proxy_auth);
	ntlm_request->auth_state = AUTHENTICATE_STATE_INITIAL;
	safe_free(ntlm_request->client_blob);
	ntlm_request->client_blob = xstrdup(blob);
	conn->auth_type = AUTH_NTLM;
	conn->auth_user_request = auth_user_request;
	ntlm_request->conn = conn;
	/* and lock for the connection duration */
	debug(29, 9) ("authenticateNTLMAuthenticateUser: Locking auth_user from the connection.\n");
	authenticateAuthUserRequestLock(auth_user_request);
	return;
	break;
    case AUTHENTICATE_STATE_INITIAL:
	debug(29, 1) ("authenticateNTLMAuthenticateUser: need to ask helper!\n");
	return;
	break;
    case AUTHENTICATE_STATE_NEGOTIATE:
	/* we should have recieved a blob from the clien. pass it to the same 
	 * helper process */
	debug(29, 9) ("authenticateNTLMAuthenticateUser: auth state challenge with header %s.\n", proxy_auth);
	/* do a cache lookup here. If it matches it's a successful ntlm 
	 * challenge - release the helper and use the existing auth_user 
	 * details. */
	safe_free(ntlm_request->client_blob);
	ntlm_request->client_blob = xstrdup(blob);
	return;
	break;
    case AUTHENTICATE_STATE_FINISHED:
	/* this connection is authenticated */
	debug(29, 4) ("authenticated user %s\n", ntlm_user->username);
	/* see if this is an existing user with a different proxy_auth 
	 * string */
	usernamehash = hash_lookup(proxy_auth_username_cache, ntlm_user->username);
	if (usernamehash) {
	    while (usernamehash && (usernamehash->auth_user->auth_type != auth_user->auth_type || authenticateNTLMcmpUsername(usernamehash->auth_user->scheme_data, ntlm_user) != 0))
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
	authenticateNTLMReleaseServer(ntlm_request);
	ntlm_request->auth_state = AUTHENTICATE_STATE_DONE;
	return;
    case AUTHENTICATE_STATE_DONE:
	fatal("authenticateNTLMAuthenticateUser: unexpect auth state DONE! Report a bug to the squid developers.\n");
	break;
    case AUTHENTICATE_STATE_FAILED:
	/* we've failed somewhere in authentication */
	debug(29, 9) ("authenticateNTLMAuthenticateUser: auth state ntlm failed. %s\n", proxy_auth);
	return;
    }
    return;
}
