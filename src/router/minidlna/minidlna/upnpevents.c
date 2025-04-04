/* MiniDLNA project
 * http://minidlna.sourceforge.net/
 *
 * MiniDLNA media server
 * Copyright (C) 2008-2009  Justin Maggard
 *
 * This file is part of MiniDLNA.
 *
 * MiniDLNA is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * MiniDLNA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with MiniDLNA. If not, see <http://www.gnu.org/licenses/>.
 *
 * Portions of the code from the MiniUPnP project:
 *
 * Copyright (c) 2006-2007, Thomas Bernard
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * The name of the author may not be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "config.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "queue.h"
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>

#include "event.h"
#include "upnpevents.h"
#include "minidlnapath.h"
#include "upnpglobalvars.h"
#include "upnpdescgen.h"
#include "uuid.h"
#include "utils.h"
#include "log.h"

/* stuctures definitions */
struct subscriber {
	LIST_ENTRY(subscriber) entries;
	struct upnp_event_notify * notify;
	time_t timeout;
	uint32_t seq;
	enum subscriber_service_enum service;
	char uuid[42];
	char callback[];
};

struct upnp_event_notify {
	struct event ev;
	LIST_ENTRY(upnp_event_notify) entries;
	enum { EConnecting,
	       ESending,
	       EWaitingForResponse,
	       EFinished,
	       EError } state;
	struct subscriber * sub;
	char * buffer;
	int buffersize;
	int tosend;
	int sent;
	const char * path;
	char addrstr[16];
	char portstr[8];
};

/* prototypes */
static void upnp_event_create_notify(struct subscriber * sub);
static void upnp_event_process_notify(struct event *ev);

/* Subscriber list */
LIST_HEAD(listhead, subscriber) subscriberlist = { NULL };

/* notify list */
LIST_HEAD(listheadnotif, upnp_event_notify) notifylist = { NULL };

#define MAX_SUBSCRIBERS 500
static uint16_t nsubscribers = 0;

/* create a new subscriber */
static struct subscriber *
newSubscriber(const char * eventurl, const char * callback, int callbacklen)
{
	struct subscriber * tmp;
	if(!eventurl || !callback || !callbacklen)
		return NULL;
	tmp = calloc(1, sizeof(struct subscriber)+callbacklen+1);
	if(strcmp(eventurl, CONTENTDIRECTORY_EVENTURL)==0)
		tmp->service = EContentDirectory;
	else if(strcmp(eventurl, CONNECTIONMGR_EVENTURL)==0)
		tmp->service = EConnectionManager;
	else if(strcmp(eventurl, X_MS_MEDIARECEIVERREGISTRAR_EVENTURL)==0)
		tmp->service = EMSMediaReceiverRegistrar;
	else {
		free(tmp);
		return NULL;
	}
	memcpy(tmp->callback, callback, callbacklen);
	tmp->callback[callbacklen] = '\0';
	/* make a dummy uuid */
	strncpyt(tmp->uuid, uuidvalue, sizeof(tmp->uuid));
	if( get_uuid_string(tmp->uuid+5) != 0 )
	{
		tmp->uuid[sizeof(tmp->uuid)-1] = '\0';
		snprintf(tmp->uuid+37, 5, "%04lx", random() & 0xffff);
	}

	return tmp;
}

/* creates a new subscriber and adds it to the subscriber list
 * also initiate 1st notify */
const char *
upnpevents_addSubscriber(const char * eventurl,
                         const char * callback, int callbacklen,
                         int timeout)
{
	struct subscriber * tmp;
	DPRINTF(E_DEBUG, L_HTTP, "addSubscriber(%s, %.*s, %d)\n",
	       eventurl, callbacklen, callback, timeout);
	if (nsubscribers >= MAX_SUBSCRIBERS)
		return NULL;
	tmp = newSubscriber(eventurl, callback, callbacklen);
	if(!tmp)
		return NULL;
	if(timeout)
		tmp->timeout = time(NULL) + timeout;
	LIST_INSERT_HEAD(&subscriberlist, tmp, entries);
	nsubscribers++;
	upnp_event_create_notify(tmp);
	return tmp->uuid;
}

/* renew a subscription (update the timeout) */
int
renewSubscription(const char * sid, int sidlen, int timeout)
{
	struct subscriber * sub;
	for(sub = subscriberlist.lh_first; sub != NULL; sub = sub->entries.le_next) {
		if(memcmp(sid, sub->uuid, 41) == 0) {
			sub->timeout = (timeout ? time(NULL) + timeout : 0);
			return 0;
		}
	}
	return -1;
}

int
upnpevents_removeSubscriber(const char * sid, int sidlen)
{
	struct subscriber * sub;
	if(!sid)
		return -1;
	DPRINTF(E_DEBUG, L_HTTP, "removeSubscriber(%.*s)\n",
	       sidlen, sid);
	for(sub = subscriberlist.lh_first; sub != NULL; sub = sub->entries.le_next) {
		if(memcmp(sid, sub->uuid, 41) == 0) {
			if(sub->notify) {
				sub->notify->sub = NULL;
			}
			LIST_REMOVE(sub, entries);
			nsubscribers--;
			free(sub);
			return 0;
		}
	}
	return -1;
}

void
upnpevents_removeSubscribers(void)
{
	struct subscriber * sub;

	for(sub = subscriberlist.lh_first; sub != NULL; sub = subscriberlist.lh_first) {
		upnpevents_removeSubscriber(sub->uuid, sizeof(sub->uuid));
	}
}

/* notifies all subscribers of a SystemUpdateID change */
void
upnp_event_var_change_notify(enum subscriber_service_enum service)
{
	struct subscriber * sub;
	for(sub = subscriberlist.lh_first; sub != NULL; sub = sub->entries.le_next) {
		if(sub->service == service && sub->notify == NULL)
			upnp_event_create_notify(sub);
	}
}

/* create and add the notify object to the list, start connecting */
static void
upnp_event_create_notify(struct subscriber *sub)
{
	struct upnp_event_notify * obj;
	int flags, s, i;
	const char *p;
	unsigned short port;
	struct sockaddr_in addr;

	assert(sub);

	obj = calloc(1, sizeof(struct upnp_event_notify));
	if(!obj) {
		DPRINTF(E_ERROR, L_HTTP, "calloc(): %s\n", strerror(errno));
		return;
	}
	obj->sub = sub;
	s = socket(PF_INET, SOCK_STREAM, 0);
	if(s < 0) {
		DPRINTF(E_ERROR, L_HTTP, "socket(): %s\n", strerror(errno));
		goto error;
	}
	if((flags = fcntl(s, F_GETFL, 0)) < 0) {
		DPRINTF(E_ERROR, L_HTTP, "fcntl(..F_GETFL..): %s\n",
		       strerror(errno));
		goto error;
	}
	if(fcntl(s, F_SETFL, flags | O_NONBLOCK) < 0) {
		DPRINTF(E_ERROR, L_HTTP, "fcntl(..F_SETFL..): %s\n",
		       strerror(errno));
		goto error;
	}
	if(sub)
		sub->notify = obj;
	LIST_INSERT_HEAD(&notifylist, obj, entries);

	memset(&addr, 0, sizeof(addr));
	i = 0;
	p = obj->sub->callback;
	p += 7;	/* http:// */
	while(*p != '/' && *p != ':' && i < (sizeof(obj->addrstr)-1))
		obj->addrstr[i++] = *(p++);
	obj->addrstr[i] = '\0';
	if(*p == ':') {
		obj->portstr[0] = *p;
		i = 1;
		p++;
		port = (unsigned short)atoi(p);
		while(*p != '/' && *p != '\0') {
			if(i<7) obj->portstr[i++] = *p;
			p++;
		}
		obj->portstr[i] = 0;
	} else {
		port = 80;
		obj->portstr[0] = '\0';
	}
	if( *p )
		obj->path = p;
	else
		obj->path = "/";
	addr.sin_family = AF_INET;
	inet_aton(obj->addrstr, &addr.sin_addr);
	addr.sin_port = htons(port);
	DPRINTF(E_DEBUG, L_HTTP, "'%s' %hu '%s'\n",
	       obj->addrstr, port, obj->path);
	obj->state = EConnecting;
	obj->ev = (struct event ){ .fd = s, .rdwr = EVENT_WRITE,
		.process = upnp_event_process_notify, .data = obj };
	event_module.add(&obj->ev);
	if(connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		if(errno != EINPROGRESS && errno != EWOULDBLOCK) {
			DPRINTF(E_ERROR, L_HTTP, "connect(): %s\n", strerror(errno));
			obj->state = EError;
			event_module.del(&obj->ev, 0);
		}
	}

	return;

error:
	if(s >= 0)
		close(s);
	free(obj);
}

static void upnp_event_prepare(struct upnp_event_notify * obj)
{
	static const char notifymsg[] = 
		"NOTIFY %s HTTP/1.1\r\n"
		"Host: %s%s\r\n"
		"Content-Type: text/xml; charset=\"utf-8\"\r\n"
		"Content-Length: %d\r\n"
		"NT: upnp:event\r\n"
		"NTS: upnp:propchange\r\n"
		"SID: %s\r\n"
		"SEQ: %u\r\n"
		"Connection: close\r\n"
		"Cache-Control: no-cache\r\n"
		"\r\n"
		"%.*s\r\n";
	char * xml;
	int l;

	assert(obj->sub);

	switch(obj->sub->service) {
	case EContentDirectory:
		xml = getVarsContentDirectory(&l);
		break;
	case EConnectionManager:
		xml = getVarsConnectionManager(&l);
		break;
	case EMSMediaReceiverRegistrar:
		xml = getVarsX_MS_MediaReceiverRegistrar(&l);
		break;
	default:
		xml = NULL;
		l = 0;
	}
	obj->tosend = asprintf(&(obj->buffer), notifymsg,
	                       obj->path, obj->addrstr, obj->portstr, l+2,
	                       obj->sub->uuid, obj->sub->seq,
	                       l, xml);
	obj->buffersize = obj->tosend;
	free(xml);
	DPRINTF(E_DEBUG, L_HTTP, "Sending UPnP Event response:\n%s\n", obj->buffer);
	obj->state = ESending;
}

static void upnp_event_send(struct upnp_event_notify * obj)
{
	int i;
	//DEBUG DPRINTF(E_DEBUG, L_HTTP, "Sending UPnP Event:\n%s", obj->buffer+obj->sent);
	while( obj->sent < obj->tosend ) {
		i = send(obj->ev.fd, obj->buffer + obj->sent, obj->tosend - obj->sent, 0);
		if(i<0) {
			DPRINTF(E_WARN, L_HTTP, "%s: send(): %s\n", "upnp_event_send", strerror(errno));
			obj->state = EError;
			event_module.del(&obj->ev, 0);
			return;
		}
		obj->sent += i;
	}
	if(obj->sent == obj->tosend) {
		obj->state = EWaitingForResponse;
		event_module.del(&obj->ev, 0);
		obj->ev.rdwr = EVENT_READ;
		event_module.add(&obj->ev);
	}
}

static void upnp_event_recv(struct upnp_event_notify * obj)
{
	int n;
	n = recv(obj->ev.fd, obj->buffer, obj->buffersize, 0);
	if(n<0) {
		DPRINTF(E_ERROR, L_HTTP, "%s: recv(): %s\n", "upnp_event_recv", strerror(errno));
		obj->state = EError;
		event_module.del(&obj->ev, 0);
		return;
	}
	DPRINTF(E_DEBUG, L_HTTP, "%s: (%dbytes) %.*s\n", "upnp_event_recv",
	       n, n, obj->buffer);
	obj->state = EFinished;
	event_module.del(&obj->ev, EV_FLAG_CLOSING);
	if(obj->sub)
	{
		obj->sub->seq++;
		if (!obj->sub->seq)
			obj->sub->seq++;
	}
}

static void
upnp_event_process_notify(struct event *ev)
{
	struct upnp_event_notify *obj = ev->data;

	switch(obj->state) {
	case EConnecting:
		/* now connected or failed to connect */
		upnp_event_prepare(obj);
		upnp_event_send(obj);
		break;
	case ESending:
		upnp_event_send(obj);
		break;
	case EWaitingForResponse:
		upnp_event_recv(obj);
		break;
	case EFinished:
		close(obj->ev.fd);
		obj->ev.fd = -1;
		break;
	default:
		DPRINTF(E_ERROR, L_HTTP, "upnp_event_process_notify: unknown state\n");
	}
}

void upnpevents_gc(void)
{
	struct upnp_event_notify * obj;
	struct upnp_event_notify * next;
	struct subscriber * sub;
	struct subscriber * subnext;
	time_t curtime;

	obj = notifylist.lh_first;
	while(obj != NULL) {
		next = obj->entries.le_next;
		if(obj->state == EError || obj->state == EFinished) {
			if(obj->ev.fd >= 0) {
				close(obj->ev.fd);
			}
			if(obj->sub)
				obj->sub->notify = NULL;
			/* remove also the subscriber from the list if there was an error */
			if(obj->state == EError && obj->sub) {
				LIST_REMOVE(obj->sub, entries);
				nsubscribers--;
				free(obj->sub);
			}
			free(obj->buffer);
			LIST_REMOVE(obj, entries);
			free(obj);
		}
		obj = next;
	}
	/* remove timed-out subscribers */
	curtime = time(NULL);
	for(sub = subscriberlist.lh_first; sub != NULL; ) {
		subnext = sub->entries.le_next;
		if(sub->timeout && curtime > sub->timeout && sub->notify == NULL) {
			LIST_REMOVE(sub, entries);
			nsubscribers--;
			free(sub);
		}
		sub = subnext;
	}
}
