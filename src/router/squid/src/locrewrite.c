
/*
 * $Id: locrewrite.c,v 1.4 2006/05/16 04:43:30 hno Exp $
 *
 * DEBUG: section 29    Redirector
 * AUTHOR: Henrik Nordstrom
 * BASED ON: redirect.c by Duane Wessels
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

typedef struct {
    void *data;
    char *orig_url;
    RH *handler;
} rewriteStateData;

static HLPCB locationRewriteHandleReply;
static void locationRewriteStateFree(rewriteStateData * r);
static helper *locrewriters = NULL;
static OBJH locationRewriteStats;
static int n_bypassed = 0;
CBDATA_TYPE(rewriteStateData);

static void
locationRewriteHandleReply(void *data, char *reply)
{
    rewriteStateData *r = data;
    int valid;
    char *t;
    debug(29, 5) ("rewriteHandleRead: {%s}\n", reply ? reply : "<NULL>");
    if (reply) {
	if ((t = strchr(reply, ' ')))
	    *t = '\0';
	if (*reply == '\0')
	    reply = NULL;
    }
    valid = cbdataValid(r->data);
    cbdataUnlock(r->data);
    if (valid)
	r->handler(r->data, reply);
    locationRewriteStateFree(r);
}

static void
locationRewriteStateFree(rewriteStateData * r)
{
    safe_free(r->orig_url);
    cbdataFree(r);
}

static void
locationRewriteStats(StoreEntry * sentry)
{
    storeAppendPrintf(sentry, "Redirector Statistics:\n");
    helperStats(sentry, locrewriters);
    if (Config.onoff.redirector_bypass)
	storeAppendPrintf(sentry, "\nNumber of requests bypassed "
	    "because all rewriters were busy: %d\n", n_bypassed);
}

/**** PUBLIC FUNCTIONS ****/

void
locationRewriteStart(HttpReply * rep, clientHttpRequest * http, RH * handler, void *data)
{
    rewriteStateData *r = NULL;
    const char *location = httpHeaderGetStr(&rep->header, HDR_LOCATION);
    const char *urlgroup;
    char buf[8192];

    if (http->orig_request && http->orig_request->urlgroup)
	urlgroup = http->orig_request->urlgroup;
    else if (http->request && http->request->urlgroup)
	urlgroup = http->request->urlgroup;
    else
	urlgroup = NULL;

    assert(handler);
    if (!urlgroup || !*urlgroup)
	urlgroup = "-";
    debug(29, 5) ("locationRewriteStart: '%s'\n", location);
    if (Config.Program.location_rewrite.command == NULL) {
	handler(data, NULL);
	return;
    }
    if (Config.onoff.redirector_bypass && locrewriters->stats.queue_size) {
	/* Skip rewriter if there is one request queued */
	n_bypassed++;
	handler(data, NULL);
	return;
    }
    r = cbdataAlloc(rewriteStateData);
    r->orig_url = xstrdup(location);
    r->handler = handler;
    r->data = data;
    cbdataLock(r->data);
    snprintf(buf, 8192, "%s %s %s\n",
	r->orig_url, http->uri, urlgroup);
    helperSubmit(locrewriters, buf, locationRewriteHandleReply, r);
}

void
locationRewriteInit(void)
{
    static int init = 0;
    if (!Config.Program.location_rewrite.command)
	return;
    if (locrewriters == NULL)
	locrewriters = helperCreate("location_rewriter");
    locrewriters->cmdline = Config.Program.location_rewrite.command;
    locrewriters->n_to_start = Config.Program.location_rewrite.children;
    locrewriters->concurrency = Config.Program.location_rewrite.concurrency;
    locrewriters->ipc_type = IPC_STREAM;
    helperOpenServers(locrewriters);
    if (!init) {
	cachemgrRegister("location_rewriter",
	    "Location Rewriter Stats",
	    locationRewriteStats, 0, 1);
	init = 1;
	CBDATA_INIT_TYPE(rewriteStateData);
    }
}

void
locationRewriteShutdown(void)
{
    if (!locrewriters)
	return;
    helperShutdown(locrewriters);
    if (!shutting_down)
	return;
    helperFree(locrewriters);
    locrewriters = NULL;
}
