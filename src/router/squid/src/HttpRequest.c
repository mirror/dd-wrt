
/*
 * $Id: HttpRequest.c,v 1.41 2006/07/17 02:31:59 hno Exp $
 *
 * DEBUG: section 73    HTTP Request
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

request_t *
requestCreate(method_t method, protocol_t protocol, const char *urlpath)
{
    request_t *req = memAllocate(MEM_REQUEST_T);
    req->method = method;
    req->protocol = protocol;
    if (urlpath)
	stringReset(&req->urlpath, urlpath);
    req->max_forwards = -1;
    req->lastmod = -1;
    req->client_addr = no_addr;
    req->my_addr = no_addr;
    httpHeaderInit(&req->header, hoRequest);
    return req;
}

void
requestDestroy(request_t * req)
{
    assert(req);
    if (req->body_reader)
	requestAbortBody(req);
    if (req->auth_user_request)
	authenticateAuthUserRequestUnlock(req->auth_user_request);
    safe_free(req->canonical);
    safe_free(req->vary_hdr);
    safe_free(req->vary_headers);
    stringClean(&req->vary_encoding);
    safe_free(req->urlgroup);
    safe_free(req->extacl_user);
    safe_free(req->extacl_passwd);
    stringClean(&req->urlpath);
    httpHeaderClean(&req->header);
    if (req->cache_control)
	httpHdrCcDestroy(req->cache_control);
    if (req->range)
	httpHdrRangeDestroy(req->range);
    stringClean(&req->extacl_log);
    if (req->vary) {
	if (req->etags == &req->vary->etags)
	    req->etags = NULL;
	storeLocateVaryDone(req->vary);
	req->vary = NULL;
    }
    assert(req->etags == NULL);
    safe_free(req->etag);
    if (req->pinned_connection)
	cbdataUnlock(req->pinned_connection);
    req->pinned_connection = NULL;
    memFree(req, MEM_REQUEST_T);
}

request_t *
requestLink(request_t * request)
{
    assert(request);
    request->link_count++;
    return request;
}

void
requestUnlink(request_t * request)
{
    if (!request)
	return;
    assert(request->link_count > 0);
    if (--request->link_count > 0)
	return;
    requestDestroy(request);
}

int
httpRequestParseHeader(request_t * req, const char *parse_start)
{
    const char *blk_start, *blk_end;
    if (!httpMsgIsolateHeaders(&parse_start, &blk_start, &blk_end))
	return 0;
    return httpHeaderParse(&req->header, blk_start, blk_end);
}

/* packs request-line and headers, appends <crlf> terminator */
static void
httpRequestPack(const request_t * req, Packer * p)
{
    assert(req && p);
    /* pack request-line */
    packerPrintf(p, "%s %s HTTP/1.0\r\n",
	RequestMethodStr[req->method], strBuf(req->urlpath));
    /* headers */
    httpHeaderPackInto(&req->header, p);
    /* trailer */
    packerAppend(p, "\r\n", 2);
}

/* packs debug info, canonical request-line and headers, appends <crlf> terminator */
void
httpRequestPackDebug(request_t * req, Packer * p)
{
    assert(req && p);
    /* Client info */
    packerPrintf(p, "Client: %s ", inet_ntoa(req->client_addr));
    packerPrintf(p, "http_port: %s:%d", inet_ntoa(req->my_addr), req->my_port);
    if (req->auth_user_request && authenticateUserRequestUsername(req->auth_user_request))
	packerPrintf(p, "user: %s", authenticateUserRequestUsername(req->auth_user_request));
    packerPrintf(p, "\n");
    /* pack request-line */
    packerPrintf(p, "%s %s HTTP/%d.%d\r\n",
	RequestMethodStr[req->method], urlCanonical(req), req->http_ver.major, req->http_ver.minor);
    /* headers */
    httpHeaderPackInto(&req->header, p);
    /* trailer */
    packerAppend(p, "\r\n", 2);
}

/* swaps out request using httpRequestPack */
void
httpRequestSwapOut(const request_t * req, StoreEntry * e)
{
    Packer p;
    assert(req && e);
    packerToStoreInit(&p, e);
    httpRequestPack(req, &p);
    packerClean(&p);
}

#if UNUSED_CODE
void
httpRequestSetHeaders(request_t * req, method_t method, const char *uri, const char *header_str)
{
    assert(req && uri && header_str);
    assert(!req->header.len);
    httpHeaderParse(&req->header, header_str, header_str + strlen(header_str));
}

#endif

/* returns the length of request line + headers + crlf */
int
httpRequestPrefixLen(const request_t * req)
{
    assert(req);
    return strlen(RequestMethodStr[req->method]) + 1 +
	strLen(req->urlpath) + 1 +
	4 + 1 + 3 + 2 +
	req->header.len + 2;
}

/*
 * Returns true if HTTP allows us to pass this header on.  Does not
 * check anonymizer (aka header_access) configuration.
 */
int
httpRequestHdrAllowed(const HttpHeaderEntry * e, String * strConn)
{
    assert(e);
    /* check connection header */
    if (strConn && strListIsMember(strConn, strBuf(e->name), ','))
	return 0;
    return 1;
}

/*
 * Read request body contents
 */
void
requestReadBody(request_t * request, char *buf, size_t size, CBCB * callback, void *cbdata)
{
    if (request->body_reader) {
	if (cbdataValid(request->body_reader_data)) {
	    request->body_reader(request, buf, size, callback, cbdata);
	} else {
	    debug(73, 1) ("requestReadBody: Aborted\n");
	    request->body_reader = NULL;
	    cbdataUnlock(request->body_reader_data);
	    request->body_reader_data = NULL;
	    callback(buf, 0, cbdata);	/* Signal end of body */
	}
    } else {
	callback(buf, 0, cbdata);	/* Signal end of body */
    }
}

void
requestAbortBody(request_t * request)
{
    if (!request)
	return;
    if (request->body_reader) {
	void *cbdata = request->body_reader_data;
	BODY_HANDLER *handler = request->body_reader;
	debug(73, 2) ("requestAbortBody: Aborted\n");
	request->body_reader = NULL;
	request->body_reader_data = NULL;
	if (cbdataValid(cbdata))
	    handler(request, NULL, -1, NULL, cbdata);
	cbdataUnlock(cbdata);
    }
}
