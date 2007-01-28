
/*
 * $Id: errormap.c,v 1.2 2006/06/05 22:47:01 hno Exp $
 *
 * DEBUG: section ??    Error Beautifier
 * AUTHOR: Henrik Nordstrom
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

static const char *
getErrorMap(const errormap * map, const http_status status, const char *squid_error, const char *aclname)
{
    while (map) {
	struct error_map_entry *m;
	for (m = map->map; m; m = m->next) {
	    if (m->status == status)
		return map->url;
	    if (squid_error && strcmp(m->value, squid_error) == 0)
		return map->url;
	    if (aclname && strcmp(m->value, aclname) == 0)
		return map->url;
	}
	map = map->next;
    }
    return NULL;
}


static int client_header_identities[] =
{
    HDR_USER_AGENT,
    HDR_COOKIE,
    HDR_X_FORWARDED_FOR,
    HDR_VIA,
    HDR_AUTHORIZATION,
    HDR_ACCEPT,
    HDR_REFERER
};
HttpHeaderMask client_headers;

static int server_header_identities[] =
{
    HDR_VIA,
    HDR_SERVER,
    HDR_LOCATION,
    HDR_CONTENT_LOCATION
};
HttpHeaderMask server_headers;

typedef struct {
    request_t *req;
    StoreEntry *e;
    store_client *sc;
    ERRMAPCB *callback;
    void *callback_data;
    char *buf;
} ErrorMapState;

CBDATA_TYPE(ErrorMapState);

static void
errorMapFetchComplete(ErrorMapState * state)
{
    storeClientUnregister(state->sc, state->e, state);
    state->sc = NULL;
    storeUnlockObject(state->e);
    state->e = NULL;
    requestUnlink(state->req);
    state->req = NULL;
    memFree(state->buf, MEM_4K_BUF);
    cbdataUnlock(state->callback_data);
    state->callback_data = NULL;
    cbdataFree(state);
}

static void
errorMapFetchAbort(ErrorMapState * state)
{
    if (cbdataValid(state->callback_data))
	state->callback(NULL, -1, -1, state->callback_data);
    errorMapFetchComplete(state);
}

static void
errorMapFetchHeaders(void *data, char *buf, ssize_t size)
{
    ErrorMapState *state = data;
    size_t hdr_size;

    if (EBIT_TEST(state->e->flags, ENTRY_ABORTED))
	goto abort;
    if (size == 0)
	goto abort;
    if (!cbdataValid(state->callback_data))
	goto abort;

    if ((hdr_size = headersEnd(buf, size))) {
	http_status status;
	/* httpReplyParse(reply, buf, hdr_size); */
	HttpReply *reply = state->e->mem_obj->reply;
	assert(reply);
	status = reply->sline.status;
	if (status != HTTP_OK)
	    goto abort;
	/* Send object to caller (cbdataValid verified above) */
	state->callback(state->e, hdr_size, httpHeaderGetSize(&reply->header, HDR_CONTENT_LENGTH), state->callback_data);
	errorMapFetchComplete(state);
	goto done;
    }
    if (size >= 4096) {
	/* Not enought space for reply headers */
	goto abort;
    }
    /* Need more data */
    storeClientCopy(state->sc, state->e, size, 0, 4096, state->buf, errorMapFetchHeaders, state);
  done:
    return;
  abort:
    errorMapFetchAbort(state);
    return;
}

int
errorMapStart(const errormap * map, request_t * client_req, HttpReply * reply, const char *aclname, ERRMAPCB * callback, void *callback_data)
{
    char squid_error[100];
    int len = 0;
    const char *errorUrl;
    ErrorMapState *state;
    const char *tmp;
    http_status status;
    request_t *req;
    HttpHeaderPos hdrpos;
    HttpHeaderEntry *hdr;

    if (!client_req || !reply)
	return 0;

    status = reply->sline.status;

    tmp = httpHeaderGetStr(&reply->header, HDR_X_SQUID_ERROR);
    squid_error[0] = '\0';
    if (tmp) {
	xstrncpy(squid_error, tmp, sizeof(squid_error));
	len = strcspn(squid_error, " ");
    }
    squid_error[len] = '\0';
    errorUrl = getErrorMap(map, status, squid_error, aclname);
    if (!errorUrl)
	return 0;
    req = urlParse(METHOD_GET, (char *) errorUrl);
    if (!req) {
	debug(0, 0) ("error_map: Invalid error URL '%s'\n", errorUrl);
	return 0;
    }
    req->urlgroup = xstrdup("error");

    state = cbdataAlloc(ErrorMapState);
    state->req = requestLink(req);
    state->e = storeCreateEntry(errorUrl, errorUrl, req->flags, req->method);
    state->sc = storeClientRegister(state->e, state);
    state->callback = callback;
    state->callback_data = callback_data;
    cbdataLock(callback_data);

    hdrpos = HttpHeaderInitPos;
    while ((hdr = httpHeaderGetEntry(&client_req->header, &hdrpos)) != NULL) {
	if (CBIT_TEST(client_headers, hdr->id))
	    httpHeaderAddEntry(&req->header, httpHeaderEntryClone(hdr));
    }
    hdrpos = HttpHeaderInitPos;
    while ((hdr = httpHeaderGetEntry(&reply->header, &hdrpos)) != NULL) {
	if (CBIT_TEST(server_headers, hdr->id))
	    httpHeaderAddEntry(&req->header, httpHeaderEntryClone(hdr));
    }
    httpHeaderPutInt(&req->header, HDR_X_ERROR_STATUS, (int) reply->sline.status);
    httpHeaderPutStr(&req->header, HDR_X_REQUEST_URI, urlCanonical(client_req));

    state->buf = memAllocate(MEM_4K_BUF);
    fwdStart(-1, state->e, req);
    storeClientCopy(state->sc, state->e, 0, 0, 4096, state->buf, errorMapFetchHeaders, state);
    return 1;
}

void
errorMapInit(void)
{
    CBDATA_INIT_TYPE(ErrorMapState);

    httpHeaderCalcMask(&client_headers, client_header_identities, sizeof(client_header_identities) / sizeof(*client_header_identities));
    httpHeaderCalcMask(&server_headers, server_header_identities, sizeof(server_header_identities) / sizeof(*server_header_identities));
}
