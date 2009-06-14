/*
 * Broadcom UPnP module GENA implementation
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: gena.c,v 1.9 2008/12/12 02:08:22 Exp $
 */
#include <upnp.h>

/*
 * Variables
 */
static  unsigned int    unique_id_count = 1;


/* Get the subscriber chain of the focus interface */
UPNP_SCBRCHAIN *
get_subscriber_chain(UPNP_CONTEXT *context, UPNP_SERVICE *service)
{
	UPNP_SCBRCHAIN	*scbrchain;

	/* Search the subscriber chain of the focused interface */
	for (scbrchain = service->scbrchain;
	     scbrchain;
	     scbrchain = scbrchain->next) {
		if (scbrchain->ifp == context->focus_ifp)
			break;
	}

	return scbrchain;
}

/* Get the evented state variable's value of the focus interface */
UPNP_EVALUE	*
get_evalue(UPNP_CONTEXT *context, UPNP_STATE_VAR *state_var)
{
	UPNP_EVALUE	*evalue;

	for (evalue = state_var->evalue;
	     evalue;
	     evalue = evalue->next) {
		if (evalue->ifp == context->focus_ifp)
			break;
	}

	return evalue;
}

/* Search GENA event lists for a target event */
UPNP_STATE_VAR	*
find_event_var(UPNP_CONTEXT *context, UPNP_SERVICE *service, char *name)
{
	UPNP_STATE_VAR *statevar;

	for (statevar = service->event_var_list;
	     statevar;
	     statevar = statevar->next) {
		if (strcmp(statevar->name, name) == 0)
			break;
	}

	return statevar;
}

/* Sarch GENA event lists for a target event */
UPNP_SERVICE *
find_event(UPNP_CONTEXT *context, char *event_url)
{
	UPNP_INTERFACE	*ifp = context->focus_ifp;
	UPNP_DEVCHAIN	*chain;
	UPNP_SERVICE	*service;

	/*
	 * Loop for all the UPnP device, and find out the
	 * UPnP service matches the event_url.
	 */
	for (chain = ifp->device_chain;
	     chain;
	     chain = chain->next) {
		for (service = chain->device->service_table;
		     service && service->event_url;
		     service++) {
			if (strcmp(service->event_url, event_url) == 0) {
				ifp->focus_devchain = chain;
				return service;
			}
		}
	}

	return 0;
}

/* Remove subscriber from list */
void
delete_subscriber(UPNP_SCBRCHAIN *scbrchain, UPNP_SUBSCRIBER *subscriber)
{
	/* Remove from queue */
	if (subscriber->prev) {
		subscriber->prev->next = subscriber->next;
	}
	else {
		/* first node, re-configure the list pointer */
		scbrchain->subscriberlist = subscriber->next;
	}

	if (subscriber->next)
		subscriber->next->prev = subscriber->prev;

	free(subscriber);
	return;
}

/* Parse the header, CALLBACK, to get host address (ip, port) and uri */
char *
parse_callback(char *callback, struct in_addr *ipaddr, unsigned short *port)
{
	char *p;
	int pos;
	char host[sizeof("255.255.255.255:65535")];
	char *uri;
	int host_port;
	struct in_addr host_ip;

	/* callback: "<http://192.168.2.13:5000/notify>" */
	if (memcmp(callback, "<http://", 8) != 0) {
		/* not a HTTP URL, return NULL pointer */
		return 0;
	}

	/* make <http:/'\0'192.168.2.13:5000/notify'\0' */
	pos = strcspn(callback, ">");
	callback[pos] = 0;

	callback[7] = 0;

	/* Locate uri */
	p = callback + 8;
	pos = strcspn(p, "/");
	if (pos > sizeof(host)-1)
		return 0;

	if (p[pos] != '/')
		uri = callback + 6;
	else
		uri = p + pos;

	strncpy(host, p, pos);
	host[pos] = 0;

	/* make port */
	host_port = 0;

	pos = strcspn(host, ":");
	if (host[pos] == ':')
		host_port = atoi(host + pos + 1);

	if (host_port > 65535)
		return 0;

	if (host_port == 0)
		host_port = 80;

	/* make ip */
	host[pos] = 0;
	if (inet_aton(host, &host_ip) == 0)
		return 0;

	*ipaddr = host_ip;
	*port = host_port;
	return uri;
}

/* Consume the input buffer after sending the notification */
static int
gena_read_sock(int s, char *buf, int len, int flags)
{
	long     rc;
	fd_set   ibits;
	struct   timeval tv = {1, 0};   /* wait for at most 1 seconds */
	int      n_read;

	FD_ZERO(&ibits);
	FD_SET(s, &ibits);

	rc = select(s+1, &ibits, 0, 0, &tv);
	if (rc <= 0)
		return -1;

	if (FD_ISSET(s, &ibits)) {
		n_read = recv(s, buf, len, flags);
		return n_read;
	}

	return -1;
}

/* Send out property event changes message */
void
notify_prop_change(UPNP_CONTEXT *context, UPNP_SUBSCRIBER *subscriber)
{
	struct sockaddr_in sin;

	int s;
	int len;
	int on;

	/* open socket */
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s < 0)
		goto error_out;

	/* set non-blocking IO to have connect() return without pending */
	on = 1;
	if ((ioctl(s, FIONBIO, (int)&on)) == -1) {
		upnp_syslog(LOG_ERR, "cannot set socket as non-blocking IO");
		goto error_out;
	}

	/* Fill socket address to connect */
	memset(&sin, 0, sizeof(sin));

#if defined(__ECOS)
	sin.sin_len = sizeof(sin);
#endif	
	sin.sin_family = AF_INET;
	sin.sin_port = htons(subscriber->port);
	sin.sin_addr = subscriber->ipaddr;

	/* connect */
	if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
		struct timeval tv;
		fd_set fds;
		int i;

		FD_ZERO(&fds);
		FD_SET(s, &fds);

#define	GENA_NOTIFY_SELECT_MAX	20
		/* timeout after 3 seconds */
		for (i = 0; i < GENA_NOTIFY_SELECT_MAX; i++) {
			tv.tv_sec = 0;
			tv.tv_usec = 500000;
			if (select(s+1, 0, &fds, 0, &tv) > 0) {
				if (FD_ISSET(s, &fds))
					break;
			}
		}

		/* Reach maximum waits, go out */
		if (i == GENA_NOTIFY_SELECT_MAX) {
			upnp_syslog(LOG_ERR, "Notify connect failed!");
			goto error_out;
		}
	}

	/* set non-blocking IO to have connect() return without pending */
	on = 0;
	if ((ioctl(s, FIONBIO, (int)&on)) == -1) {
		upnp_syslog(LOG_ERR, "cannot set socket as blocking IO");
		goto error_out;
	}

	/* Send message out */
	len = strlen(context->head_buffer);
	if (send(s, context->head_buffer, len, 0) == -1) {
		upnp_syslog(LOG_ERR, "Notify failed");
		goto error_out;
	}

	/* try to read response */
	gena_read_sock(s, context->head_buffer, GENA_MAX_BODY, 0);

error_out:
	if (s >= 0)
		close(s);

	return;
}

/* Construct property event changes message */
void
submit_prop_event_message(UPNP_CONTEXT *context,
	UPNP_SERVICE *service, UPNP_SUBSCRIBER *subscriber)
{
	UPNP_STATE_VAR *statevar = service->event_var_list;
	UPNP_VALUE value;

	unsigned char host[sizeof("255.255.255.255:65535")];
	char *p;
	int len;

	/* construct body */
	p = context->body_buffer;
	len = sprintf(context->body_buffer,
		"<e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\">\r\n");
	p += len;

	for (statevar = service->event_var_list;
	     statevar;
	     statevar = statevar->next) {
		UPNP_EVALUE *evalue;

		evalue = get_evalue(context, statevar);
		if (evalue == 0)
			continue;

		/* If had not been retrieved, get the variable */
		if (evalue->init == FALSE) {
			if ((*statevar->func)(context, service, statevar, &evalue->value) == 0) {
				evalue->init = TRUE;
			}
		}

		/* Translate the duplicated value to output string */
		value = evalue->value;
		translate_value(context, &value);

		/* new subscription */
		if (subscriber->seq == 0) {
			len = sprintf(p,
				"<e:property><e:%s>%s</e:%s></e:property>\r\n",
				statevar->name,
				value.val.str,
				statevar->name);
			p += len;
		}
		else if (evalue->changed) {
			/* state changed */
			len = sprintf(p,
				"<e:property><e:%s>%s</e:%s></e:property>\r\n",
				statevar->name,
				value.val.str,
				statevar->name);
			p += len;
		}
	}

	strcat(context->body_buffer, "</e:propertyset>\r\n\r\n");

	/* construct header */
	/* make host */
	upnp_host_addr(host, subscriber->ipaddr, subscriber->port);

	/* Make notify string */
	sprintf(context->head_buffer,
		"NOTIFY %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Content-Type: text/xml\r\n"
		"Content-Length: %d\r\n"
		"NT: upnp:event\r\n"
		"NTS: upnp:propchange\r\n"
		"SID: %s\r\n"
		"SEQ: %d\r\n"
		"Connection: close\r\n\r\n"
		"%s",
		subscriber->uri,
		host,
		(int)strlen(context->body_buffer),
		subscriber->sid,
		subscriber->seq,
		context->body_buffer);

	/* Send out */
	notify_prop_change(context, subscriber);
	return;
}

/* Submit property events to subscribers */
void
gena_notify(UPNP_CONTEXT *context, UPNP_SERVICE *service, char *sid)
{
	UPNP_SCBRCHAIN	*scbrchain;
	UPNP_SUBSCRIBER	*subscriber;

	/* walk through the subscribers */
	scbrchain = get_subscriber_chain(context, service);
	if (scbrchain == 0)
		return;

	/* Loop for all the subscriber list to send notify */
	subscriber = scbrchain->subscriberlist;
	while (subscriber) {
		/* If the sid is given, only send this one out */
		if (sid) {
			/* for specific URL */
			if (strcmp(sid, subscriber->sid) == 0) {
				submit_prop_event_message(context, service, subscriber);
				subscriber->seq++;
				break;
			}
		}
		else {
			/* for all */
			submit_prop_event_message(context, service, subscriber);
			subscriber->seq++;
		}

		subscriber = subscriber->next;
	}

	return;
}

/* Completion function after gena_notify */
void
gena_notify_complete(UPNP_CONTEXT *context, UPNP_SERVICE *service)
{
	UPNP_STATE_VAR *statevar;
	UPNP_EVALUE *evalue;

	for (statevar = service->event_var_list;
	     statevar;
	     statevar = statevar->next) {
		evalue = get_evalue(context, statevar);

		/* 
		 * After notification complete,
		 * we should reset all the changed flag to false.
		 */
		if (evalue && evalue->changed == TRUE)
			evalue->changed = FALSE;
	}

	service->evented = FALSE;
	return;
}

/* Update the event value of an event variable */
int
gena_update_event_var(UPNP_CONTEXT *context,
	UPNP_SERVICE *service, UPNP_STATE_VAR *statevar, UPNP_VALUE *value)
{
	UPNP_EVALUE *evalue;

	if (service == 0 || statevar == 0)
		return FALSE;

	evalue = get_evalue(context, statevar);
	if (evalue == 0)
		return FALSE;

	/*
	 * The evalue will be updated, only when the old value is different from
	 * the new one, or it would cause unnessary notification sent out.
	 */
	if (evalue->init == FALSE ||
	    evalue->value.len != value->len ||
	    memcmp(evalue->value.val.str, value->val.str, value->len) != 0) {
		/* Update this value */
		evalue->value = *value;

		evalue->init = TRUE;
		evalue->changed = TRUE;

		/* Tell SOAP to send notification */
		service->evented = TRUE;
		return TRUE;
	}

	return FALSE;
}

/* GENA unusubscription process routine */
int
unsubscribe(UPNP_CONTEXT *context)
{
	UPNP_SERVICE *service;
	UPNP_SUBSCRIBER *subscriber;
	UPNP_SCBRCHAIN *scbrchain;
	UPNP_DEVICE *device;

	char *gena_sid = context->SID;

	/* find event */
	service = find_event(context, context->url);
	if (service == 0)
		return R_NOT_FOUND;

	/* find SID */
	scbrchain = get_subscriber_chain(context, service);
	if (scbrchain == 0)
		return R_NOT_FOUND;

	/* Search in the subscriber list to find the matched SID */
	subscriber = scbrchain->subscriberlist;
	while (subscriber) {
		if (strcmp(subscriber->sid, gena_sid) == 0)
			break;

		subscriber = subscriber->next;
	}

	if (subscriber == 0)
		return R_PRECONDITION_FAIL;

	/* Delete this subscriber */
	delete_subscriber(scbrchain, subscriber);

	/* send reply */
	strcpy(context->head_buffer,
		"HTTP/1.1 200 OK\r\n"
		"Connection: close\r\n"
		"\r\n");

	send(context->fd, context->head_buffer, strlen(context->head_buffer), 0);

	/* service dependent things */
	device = context->focus_ifp->focus_devchain->device;
	if (device->notify)
		(*device->notify)(context, service);

	return 0;
}

/* Get a unique id string */
int
get_unique_id(char *unique_id, unsigned int size)
{
	time_t curr_time;
	unsigned int	id = ++unique_id_count;
	unsigned long	pid = upnp_pid();

	if (size < 8)
		return FALSE;

	/* get current time */
	curr_time = time(0);

	/*
	 * We have three format of unique id:
	 * when size less than 17, the format will be %lux,
	 * between 17 and 25, we use %lux-%lux,
	 * others with the foramt %lux-%lux-%lux
	 */
	if (size < 17) {
		sprintf(unique_id, "%lux", (u_long)curr_time);
	}
	else if (size < 26) {
		sprintf(unique_id, "%lux-%lux", (u_long)curr_time, (u_long)pid);
	}
	else {
		sprintf(unique_id, "%lux-%lux-%lux", (u_long)curr_time, (u_long)pid, (u_long)id);
	}

	return TRUE;
}

/* GENA subscription process routine */
int
subscribe(UPNP_CONTEXT *context)
{
	UPNP_SERVICE *service;
	UPNP_SUBSCRIBER *subscriber;
	UPNP_SCBRCHAIN *scbrchain;
	UPNP_DEVICE *device;

	int infinite = FALSE;
	int interval = context->config.sub_time;
	time_t now;
	char time_buf[64];
	char timeout[64];

	char *gena_timeout  = context->TIMEOUT;
	char *gena_sid = context->SID;
	char *gena_callback = context->CALLBACK;

	/* find event */
	service = find_event(context, context->url);
	if (service == 0)
		return R_NOT_FOUND;

	/* process subscription time interval */
	if (gena_timeout) {
		char *ptr;

		/*
		 * If the header, TIMEOUT, is given,
		 * the value should begin with "Second-".
		 */
		if (memcmp(gena_timeout, "Second-", 7) != 0)
			return R_PRECONDITION_FAIL;

		/* "infinite" means always no timed-out */
		ptr = gena_timeout + 7;
		if (strcmp(ptr, "infinite") == 0) {
			infinite = TRUE;
		}
		else {
			/* Convert the value to subscriber time */
			interval = atoi(ptr);
			if (interval == 0)
				interval = context->config.sub_time;
		}
	}
	else {
		/* No TIMEOUT header, use the default value */
		sprintf(timeout, "Second-%d", context->config.sub_time);
		gena_timeout = timeout;
	}

	/*
	 * process SID and Callback
	 */
	scbrchain = get_subscriber_chain(context, service);
	if (scbrchain == 0)
		return R_NOT_FOUND;

	/* new subscription */
	if (gena_sid == 0) {
		struct in_addr ipaddr;
		unsigned short port;
		char *uri;
		int len;

		uri = parse_callback(gena_callback, &ipaddr, &port);
		if (uri == 0)
			return R_ERROR;

		/* Find exist subscriber and free it */
		subscriber = scbrchain->subscriberlist;
		while (subscriber) {
			if (subscriber->ipaddr.s_addr == ipaddr.s_addr &&
				subscriber->port == port &&
				strcmp(subscriber->uri, uri) == 0) {

				delete_subscriber(scbrchain, subscriber);
				break;
			}

			subscriber = subscriber->next;
		}

		/*
		 * There may be multiple subscribers for the same
		 * callback, create a new subscriber anyway.
		 */
		len = sizeof(*subscriber) + strlen(uri) + 1;
		subscriber = (UPNP_SUBSCRIBER *)calloc(1, len);
		if (subscriber == 0)
			return R_ERROR;

		/* Setup subscriber data */
		subscriber->ipaddr = ipaddr;
		subscriber->port = port;

		subscriber->uri = (char *)(subscriber + 1);
		strcpy(subscriber->uri, uri);

		strcpy(subscriber->sid, "uuid:");
		get_unique_id(subscriber->sid+5, sizeof(subscriber->sid)-5-1);

		/* insert queue */
		subscriber->next = scbrchain->subscriberlist;
		if (scbrchain->subscriberlist)
			scbrchain->subscriberlist->prev = subscriber;

		scbrchain->subscriberlist = subscriber;

		/* set sequence number */
		subscriber->seq = 0;
	}
	else {
		/*
		 * This is the case, the subscriber wants to
		 * extend the subscription time.
		 */
		subscriber = scbrchain->subscriberlist;
		while (subscriber) {
			if (strcmp(subscriber->sid, gena_sid) == 0)
				break;

			subscriber = subscriber->next;
		}

		if (subscriber == 0)
			return R_PRECONDITION_FAIL;
	}

	/* update expiration time */
	if (infinite) {
		subscriber->expire_time = 0;
	}
	else {
		now = time(0);

		subscriber->expire_time = now + interval;
		if (subscriber->expire_time == 0)
			subscriber->expire_time = 1;
	}

	/* send reply */
	gmt_time(time_buf);

	sprintf(context->head_buffer,
		"HTTP/1.1 200 OK\r\n"
		"Server: POSIX, UPnP/1.0 %s/%s\r\n"
		"Date: %s\r\n"
		"SID: %s\r\n"
		"Timeout: %s\r\n"
		"Connection: close\r\n"
		"\r\n",
		context->config.os_name,
		context->config.os_ver,
		time_buf,
		subscriber->sid,
		gena_timeout);

	send(context->fd, context->head_buffer, strlen(context->head_buffer), 0);

	/*
	 * send initial property change notifications,
	 * if it is the first subscription
	 */
	if (subscriber->seq == 0)
		gena_notify(context, service, subscriber->sid);

	/* service dependent things */
	device = context->focus_ifp->focus_devchain->device;
	if (device->notify)
		(*device->notify)(context, service);

	return 0;
}

/* GENA process entry */
int
gena_process(UPNP_CONTEXT *context)
{
	char *nt = context->NT;
	char *sid = context->SID;
	char *callback = context->CALLBACK;

	/* Process subscribe request */
	if (context->method == METHOD_SUBSCRIBE) {
		/*
		 * CALLBACK+NT and SID are mutual exclusive,
		 * when the SID is not given, NT and CALLBACK should
		 * be both existent.
		 */
		if (sid == 0) {
			if (nt == 0 || strcmp(nt, "upnp:event") != 0 || callback == 0)
				return R_BAD_REQUEST;
		}
		else {
			/* Got SID, NT and CALLBACK must be null */
			if (nt || callback)
				return R_BAD_REQUEST;
		}

		return subscribe(context);
	}
	else {
		/*
		 * Process unsubscribe request.
		 * We must have SID, meanwhile the NT and CALLBACK
		 * must be null.
		 */
		if (sid == 0) {
			return R_PRECONDITION_FAIL;
		}
		else {
			if (nt || callback)
				return R_BAD_REQUEST;
		}

		return unsubscribe(context);
	}
}

/* Check and remove expired subscribers */
void
gena_timeout(UPNP_CONTEXT *context)
{
	UPNP_INTERFACE	*ifp = context->focus_ifp;
	UPNP_SERVICE	*service;
	UPNP_SUBSCRIBER *subscriber;
	UPNP_SUBSCRIBER *temp;
	UPNP_SCBRCHAIN	*scbrchain;
	time_t now;

	now = time(0);

	/*
	 * Check all services of the focus device
	 * with the give interface.
	 */
	for (service = ifp->focus_devchain->device->service_table;
	     service && service->event_url;
	     service++) {
		/* Find the subscriber list of this service */
		scbrchain = get_subscriber_chain(context, service);
		if (scbrchain == 0)
			continue;

		/* Check expiration */
		subscriber = scbrchain->subscriberlist;
		while (subscriber) {
			temp = subscriber->next;

			/* If timed-out, remove this subscriber */
			if (subscriber->expire_time &&
				(now > subscriber->expire_time)) {
				delete_subscriber(scbrchain, subscriber);
			}

			subscriber = temp;
		}
	}
}

/* Alarm function called by UPnP request functions */
void
gena_event_alarm(UPNP_CONTEXT *context, char *event_url, int num, UPNP_EVAR *evar)
{
	UPNP_SERVICE *service;
	UPNP_STATE_VAR *statevar;
	UPNP_EVALUE *evalue;
	int i;

	service = find_event(context, event_url);
	if (service == 0)
		return;

	/* Update the specific state variable */
	if (num != 0) {
		for (i = 0; i < num; i++) {
			statevar = find_event_var(context, service, evar[i].statevar);
			if (statevar == 0)
				continue;

			/* Update event variable */
			evalue = get_evalue(context, statevar);
			if (evalue == 0)
				continue;

			evalue->value = evar[i].value;
			evalue->init = TRUE;
			evalue->changed = TRUE;
		}
	}
	else {
		/* Reset the state variables to unread state */
		for (statevar = service->event_var_list;
		     statevar;
		     statevar = statevar->next) {

			evalue = get_evalue(context, statevar);
			if (evalue == 0)
				continue;

			/* Reset to unread state */
			evalue->init = FALSE;
		}
	}

	/* Nofify the subscriber that state variable changed */
	gena_notify(context, service, 0);
	gena_notify_complete(context, service);

	return;
}

/* Initialize the subscriber chain */
void
subscriber_init(UPNP_CONTEXT *context, UPNP_SERVICE *service)
{
	UPNP_SCBRCHAIN	*scbrchain;

	/* Check if the interface subscriber initialized */
	scbrchain = get_subscriber_chain(context, service);
	if (scbrchain) {
		upnp_syslog(LOG_INFO, "service %s - sschain not clean", service->name);
		return;
	}

	/* Make a new one */
	scbrchain = (UPNP_SCBRCHAIN *)malloc(sizeof(*scbrchain));
	if (scbrchain == 0)
		return;

	scbrchain->ifp = context->focus_ifp;
	scbrchain->subscriberlist = 0;

	/* Prepend to the original chain */
	scbrchain->next = service->scbrchain;
	service->scbrchain = scbrchain;
	return;
}

/* Clear all the the subscribers */
void
subscriber_shutdown(UPNP_CONTEXT *context, UPNP_SERVICE *service)
{
	UPNP_SCBRCHAIN	*scbrchain, *prev;

	/* Find the sscbr_chain */
	for (prev = 0, scbrchain = service->scbrchain;
	     scbrchain;
	     prev = scbrchain, scbrchain = scbrchain->next) {

		if (scbrchain->ifp == context->focus_ifp)
			break;
	}

	if (scbrchain == 0)
		return;

	/* Free subscribers */
	while (scbrchain->subscriberlist)
		delete_subscriber(scbrchain, scbrchain->subscriberlist);

	/* Free this chain */
	if (prev == 0)
		service->scbrchain = scbrchain->next;
	else
		prev->next = scbrchain->next;

	free(scbrchain);
	return;
}

/* Initialize the Gena event list */
void
event_vars_init(UPNP_CONTEXT *context, UPNP_SERVICE *service)
{
	UPNP_STATE_VAR *statevar, *tail;

	/* Chain all the evented variable together */
	if (service->event_var_list == 0) {
		for (tail = 0, statevar = service->statevar_table;
		     statevar->name;
		     statevar++) {
			/* Do event variable prepend */
			if (statevar->eflag) {
				statevar->next = 0;

				/* link to tail */
				if (tail == 0)
					service->event_var_list = statevar;
				else
					tail->next = statevar;

				tail = statevar;
			}
		}

		if (tail == 0)
			return;
	}

	/* Intialize evalue for the focus interface */
	for (statevar = service->event_var_list;
	     statevar;
	     statevar = statevar->next) {

		UPNP_EVALUE *evalue;

		evalue = get_evalue(context, statevar);
		if (evalue) {
			upnp_syslog(LOG_INFO, "service %s - statevar %s is not clean",
				service->name, statevar->name);
			continue;
		}

		evalue = (struct upnp_evalue *)malloc(sizeof(*evalue));
		if (evalue == 0)
			return;

		memset(evalue, 0, sizeof(*evalue));

		evalue->ifp = context->focus_ifp;
		evalue->init = FALSE;
		evalue->changed = FALSE;
		evalue->value.type = statevar->type;

		/* Prepend this value */
		evalue->next = statevar->evalue;
		statevar->evalue = evalue;
	}

	return;
}

/* Clear the Gena event list */
void
event_vars_shutdown(UPNP_CONTEXT *context, UPNP_SERVICE *service)
{
	UPNP_STATE_VAR *statevar;

	for (statevar = service->event_var_list;
		 statevar;
		 statevar = statevar->next)
	{
		UPNP_EVALUE *evalue;
		UPNP_EVALUE *prev;

		for (prev = 0, evalue = statevar->evalue;
		     evalue;
		     prev = evalue, evalue = evalue->next) {
			/* If this interface had been initialized, do nothing */
			if (evalue->ifp == context->focus_ifp) {
				if (prev == 0)
					statevar->evalue = evalue->next;
				else
					prev->next = evalue->next;
			}
			free(evalue); //not sure if this is correct
		}
	}

	return;
}

/* Initialize GENA subscribers and event sources */
int
gena_init(UPNP_CONTEXT *context)
{
	UPNP_INTERFACE	*ifp = context->focus_ifp;
	UPNP_SERVICE	*service;

	for (service = ifp->focus_devchain->device->service_table;
	     service && service->event_url;
	     service++) {
		event_vars_init(context, service);
		subscriber_init(context, service);
	}

	return 0;
}

/* Do GEAN subscribers and event variables clean up */
int
gena_shutdown(UPNP_CONTEXT *context)
{
	UPNP_INTERFACE	*ifp = context->focus_ifp;
	UPNP_SERVICE	*service;

	for (service = ifp->focus_devchain->device->service_table;
	     service && service->event_url;
	     service++) {
		subscriber_shutdown(context, service);
		event_vars_shutdown(context, service);
	}

	return 0;
}
