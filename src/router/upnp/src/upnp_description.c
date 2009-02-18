/*
 * Broadcom UPnP module XML description protocol implementation
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_description.c,v 1.9 2008/06/19 06:22:58 Exp $
 */

#include <upnp.h>

#include <stdlib.h>
#include <bcmnvram.h>

/*
 * Change PresentationURL to specific interface IP address.
 */
static int
set_url(UPNP_CONTEXT *context, UPNP_DESCRIPTION *descr, char *data_buf, int *data_len)
{
	char *s, *p;
	char buf[32];
	int buf_len;
	int tail_len;
	unsigned char myaddr[sizeof("255.255.255.255:65535")];

	UPNP_INTERFACE	*ifp = context->focus_ifp;
	char *root_device_xml = ifp->focus_devchain->device->root_device_xml;
	char *name = descr->name;

	/*
	 * Check if this is the root xml.
	 * If yes, replace the <presentationURL>,
	 * else return the data buffer length.
	 */
	if (strcmp(root_device_xml, name+1) != 0) {
		*data_len = strlen(data_buf);
		return 0;
	}

	/* Search for <presentationURL> */
	s = strstr(data_buf, URL_BTAG);
	if (s == 0) {
		/* This root xml does not contain the <presentationURL> */
		*data_len = strlen(data_buf);
		return 0;
	}
	s += strlen(URL_BTAG);

	/* Find the balanced </presentationURL> */
	p = strstr(s, URL_ETAG);
	if (p == 0) {
		upnp_syslog(LOG_ERR,
			"No balanced %s, should correct it and recomile the image.\n",
			URL_ETAG);
		return -1;
	}

	tail_len = strlen(p);

	/* Get new presentationURL length */
	
	if( nvram_match( "https_enable", "1" ) && !nvram_match( "http_enable", "1" ) )
		{
		upnp_host_addr(myaddr, ifp->ipaddr, 80);
		buf_len = sprintf(buf, "https://%s", myaddr );
		}
	else
		{
		upnp_host_addr(myaddr, ifp->ipaddr, atoi( nvram_safe_get( "http_lanport" ) ) );
		buf_len = sprintf(buf, "http://%s", myaddr);
		}

	/* Pull up tail, including the null end */
	memmove(s + buf_len, p, tail_len+1);

	/* Replace http://255.255.255.255 with interface address (http://192.168.1.1) */
	memcpy(s, buf, buf_len);

	*data_len = strlen(data_buf);
	return 0;
}

/* Send description XML file */
static int
description_send(UPNP_CONTEXT *context, UPNP_DESCRIPTION *descr)
{
	char *p;
	int len;
	char *data_buf = descr->xml;
	int data_len = 0;

	set_url(context, descr, data_buf, &data_len);
	len = sprintf(context->buf,
			"HTTP/1.1 200 OK\r\n"
			"Content-Type: text/xml\r\n"
			"Content-Length: %d\r\n"
			"Connection: close\r\n"
			"Pragma: no-cache\r\n"
			"\r\n",
			data_len);

	if (send(context->fd, context->buf, len, 0) == -1) {
		upnp_syslog(LOG_ERR,
			"description_process() send failed! fd=%d, buf=%s\n",
			context->fd, context->buf);
		return R_ERROR;
	}

	p = data_buf;

	while (data_len) {
		len = (data_len > UPNP_DESC_MAXLEN) ? UPNP_DESC_MAXLEN : data_len;
		if (send(context->fd, p, len, 0) == -1) {
			upnp_syslog(LOG_ERR, "description_send() failed");
			return -1;
		}

		p += len;
		data_len -= len;
	}

	return 0;
}

/* Description lookup and sending routine */
int
description_process(UPNP_CONTEXT *context)
{
	UPNP_INTERFACE *ifp = context->focus_ifp;
	UPNP_DEVCHAIN *chain;
	UPNP_DESCRIPTION *descr = 0;

	for (chain = ifp->device_chain;
	     chain;
	     chain = chain->next) {
		/* search the table for target url */
		for (descr = chain->device->description_table;
		     descr && descr->xml;
		     descr++) {
			/* Matched, set the focus device chain */
			if (strcmp(context->url, descr->name) == 0) {
				ifp->focus_devchain = chain;
				goto find;
			}
		}
	}

find:
	if (chain == 0) {
		return R_NOT_FOUND;
	}

	/* send description XML body */
	description_send(context, descr);
	return 0;
}
