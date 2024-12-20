/*
 * Broadcom UPnP module SSDP implementation
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: ssdp.c,v 1.11 2008/06/20 05:24:52 Exp $
 */

#include <upnp.h>

/*
 * method parsing lookup table
 */
static int ssdp_msearch(UPNP_CONTEXT *);

struct upnp_method ssdp_methods[] = { { "M-SEARCH ", sizeof("M-SEARCH ") - 1, METHOD_MSEARCH, ssdp_msearch }, { 0, 0, 0, 0 } };

static int read_header(UPNP_CONTEXT *, struct upnp_method *);

static struct upnp_state ssdp_fsm[] = { { upnp_http_fsm_init, 0 },
					{ read_header, 0 },
					{ parse_method, ssdp_methods },
					{ parse_uri, 0 },
					{ parse_msghdr, 0 },
					{ upnp_http_fsm_dispatch, ssdp_methods },
					{ 0, 0 } };

/* Send out SSDP packet */
void ssdp_send(UPNP_CONTEXT *context)
{
	struct sockaddr_in dst;

	char *buf = context->head_buffer;
	int len = strlen(buf);

	/*
	 * if caller does not specify a unicast address, send multicast
	 * (239.255.255.250)
	 */
	if (context->dst == 0) {
		/* Assign mutlicast interface to send */
		struct in_addr inaddr = context->focus_ifp->ipaddr;

		setsockopt(context->ssdp_sock, IPPROTO_IP, IP_MULTICAST_IF, (void *)&inaddr, sizeof(inaddr));

		/* Send to SSDP_ADDR:SSDP_PORT */
#if defined(__ECOS)
		dst.sin_len = sizeof(struct sockaddr_in);
#endif
		dst.sin_family = AF_INET;
		dst.sin_port = htons(SSDP_PORT);
		inet_aton(SSDP_ADDR, &dst.sin_addr);
	} else {
		dst = *context->dst;
	}

	/* Send packet */
	sendto(context->ssdp_sock, buf, len, 0, (struct sockaddr *)&dst, sizeof(dst));
	return;
}

/* Send SSDP response for a M-SEARCH request */
void ssdp_response(UPNP_CONTEXT *context, UPNP_ADVERTISE *advertise, int type)
{
	UPNP_INTERFACE *ifp = context->focus_ifp;
	UPNP_DEVICE *device = ifp->focus_devchain->device;

	unsigned char myaddr[sizeof("255.255.255.255:65535")];
	char time_buf[64];
	char *buf = context->head_buffer;
	char *p;
	int len;

	/* Get location URI */
	upnp_host_addr(myaddr, ifp->ipaddr, context->config.http_port);

	/* Should we use local time ? */
	gmt_time(time_buf);

	/* Build headers */
	len = sprintf(buf,
		      "HTTP/1.1 200 OK\r\n"
		      "Cache-Control: max-age=%d\r\n"
		      "Date: %s\r\n"
		      "Ext: \r\n"
		      "Location: http://%s/%s\r\n"
		      "Server: POSIX UPnP/1.0 %s/%s\r\n",
		      context->config.adv_time * 2, time_buf, myaddr, device->root_device_xml, context->config.os_name,
		      context->config.os_ver);

	p = buf + len;

	switch (type) {
	case MSEARCH_SERVICE:
		sprintf(p,
			"ST: %s:1\r\n"
			"USN: uuid:%s::%s:1\r\n"
			"\r\n",
			advertise->name, advertise->uuid, advertise->name);
		break;

	case MSEARCH_DEVICE:
		sprintf(p,
			"ST: %s:1\r\n"
			"USN: uuid:%s::%s:1\r\n"
			"\r\n",
			advertise->name, advertise->uuid, advertise->name);
		break;

	case MSEARCH_UUID:
		sprintf(p,
			"ST: uuid:%s\r\n"
			"USN: uuid:%s\r\n"
			"\r\n",
			advertise->uuid, advertise->uuid);
		break;

	case MSEARCH_ROOTDEVICE:
		sprintf(p,
			"ST: upnp:rootdevice\r\n"
			"USN: uuid:%s::upnp:rootdevice\r\n"
			"\r\n",
			advertise->uuid);
		break;
	}

	/* send packet out */
	ssdp_send(context);
	return;
}

/* Send SSDP notifications out */
void ssdp_notify(UPNP_CONTEXT *context, UPNP_ADVERTISE *advertise, int adv_type, int ssdp_type)
{
	int len;
	char *buf;
	char *p;

	context->dst = 0;
	buf = context->head_buffer;

	if (ssdp_type == SSDP_ALIVE) {
		UPNP_INTERFACE *ifp = context->focus_ifp;
		unsigned char myaddr[sizeof("255.255.255.255:65535")];

		/* Get location URI */
		upnp_host_addr(myaddr, ifp->ipaddr, context->config.http_port);

		/* SSDP_ALIVE */
		len = sprintf(buf,
			      "NOTIFY * HTTP/1.1\r\n"
			      "Host: 239.255.255.250:1900\r\n"
			      "Cache-Control: max-age=%d\r\n"
			      "Location: http://%s/%s\r\n"
			      "NTS: ssdp:alive\r\n"
			      "Server: POSIX, UPnP/1.0 %s/%s\r\n",
			      context->config.adv_time * 2, myaddr, ifp->focus_devchain->device->root_device_xml,
			      context->config.os_name, context->config.os_ver);
	} else {
		/* SSDP_BYEBYE */
		len = sprintf(buf, "NOTIFY * HTTP/1.1\r\n"
				   "Host: 239.255.255.250:1900\r\n"
				   "NTS: ssdp:byebye\r\n");
	}

	p = buf + len;

	switch (adv_type) {
	case ADVERTISE_SERVICE:
		sprintf(p,
			"NT: %s:1\r\n"
			"USN: uuid:%s::%s:1\r\n"
			"\r\n",
			advertise->name, advertise->uuid, advertise->name);
		break;

	case ADVERTISE_DEVICE:
		sprintf(p,
			"NT: %s:1\r\n"
			"USN: uuid:%s::%s:1\r\n"
			"\r\n",
			advertise->name, advertise->uuid, advertise->name);
		break;

	case ADVERTISE_UUID:
		sprintf(p,
			"NT: uuid:%s\r\n"
			"USN: uuid:%s\r\n"
			"\r\n",
			advertise->uuid, advertise->uuid);
		break;

	case ADVERTISE_ROOTDEVICE:
		sprintf(p,
			"NT: upnp:rootdevice\r\n"
			"USN: uuid:%s::upnp:rootdevice\r\n"
			"\r\n",
			advertise->uuid);
		break;
	}

	ssdp_send(context);
	return;
}

/* Send SSDP advertisement messages */
void ssdp_adv_process(UPNP_CONTEXT *context, int ssdp_type)
{
	UPNP_ADVERTISE *advertise;
	UPNP_DEVCHAIN *chain = context->focus_ifp->focus_devchain;

	for (advertise = chain->device->advertise_table; advertise->name; advertise++) {
		switch (advertise->type) {
		case ADVERTISE_SERVICE:
			ssdp_notify(context, advertise, ADVERTISE_SERVICE, ssdp_type);
			break;

		case ADVERTISE_DEVICE:
			ssdp_notify(context, advertise, ADVERTISE_DEVICE, ssdp_type);
			ssdp_notify(context, advertise, ADVERTISE_UUID, ssdp_type);
			break;

		default:
			ssdp_notify(context, advertise, ADVERTISE_DEVICE, ssdp_type);
			ssdp_notify(context, advertise, ADVERTISE_UUID, ssdp_type);
			ssdp_notify(context, advertise, ADVERTISE_ROOTDEVICE, ssdp_type);
			break;
		}
	}

	return;
}

/* Send SSDP_BYEBYE message */
void ssdp_byebye(UPNP_CONTEXT *context)
{
	ssdp_adv_process(context, SSDP_BYEBYE);
}

/* Send SSDP_ALIVE message */
void ssdp_alive(UPNP_CONTEXT *context)
{
	ssdp_adv_process(context, SSDP_ALIVE);
}

/* Respond to an SSDP M-SEARCH message */
void ssdp_msearch_response(UPNP_CONTEXT *context, char *name, int type)
{
	UPNP_INTERFACE *ifp = context->focus_ifp;
	UPNP_DEVCHAIN *chain;
	UPNP_ADVERTISE *advertise;

	for (chain = ifp->device_chain; chain; chain = chain->next) {
		ifp->focus_devchain = chain;

		/* Loop for all the advertise table of this device */
		for (advertise = chain->device->advertise_table; advertise->name; advertise++) {
			switch (type) {
			case MSEARCH_ROOTDEVICE:
				if (advertise->type == ADVERTISE_ROOTDEVICE) {
					ssdp_response(context, advertise, MSEARCH_ROOTDEVICE);
				}
				break;

			case MSEARCH_UUID:
				if (advertise->type != ADVERTISE_SERVICE && strcmp(name, advertise->uuid) == 0) {
					ssdp_response(context, advertise, MSEARCH_UUID);

					/* Return because the desired UUID matched */
					return;
				}
				break;

			case MSEARCH_DEVICE:
				if (strcmp(name, advertise->name) == 0) {
					ssdp_response(context, advertise, MSEARCH_DEVICE);
				}
				break;

			case MSEARCH_SERVICE:
				if (strcmp(name, advertise->name) == 0) {
					ssdp_response(context, advertise, MSEARCH_SERVICE);
				}
				break;

			case MSEARCH_ALL:
				if (advertise->type == ADVERTISE_SERVICE) {
					ssdp_response(context, advertise, MSEARCH_SERVICE);
				} else if (advertise->type == ADVERTISE_DEVICE) {
					ssdp_response(context, advertise, MSEARCH_DEVICE);
					ssdp_response(context, advertise, MSEARCH_UUID);
				} else if (advertise->type == ADVERTISE_ROOTDEVICE) {
					ssdp_response(context, advertise, MSEARCH_DEVICE);
					ssdp_response(context, advertise, MSEARCH_UUID);
					ssdp_response(context, advertise, MSEARCH_ROOTDEVICE);
				}
				break;
			}
		} /* advertise */
	} /* chain */

	return;
}

/* Parse the M-SEARCH message */
int ssdp_msearch(UPNP_CONTEXT *context)
{
	char name[128];
	int type;
	char *host;
	char *man;
	char *st;

	/* check HOST:239.255.255.250:1900 */
	host = context->HOST;
	if (!host || strcmp(host, "239.255.255.250:1900") != 0)
		return -1;

	/* check MAN:"ssdp:discover" */
	man = context->MAN;
	if (!man || strcmp(man, "\"ssdp:discover\"") != 0)
		return -1;

	/* process search target */
	st = context->ST;
	if (!st)
		return -1;

	if (strcmp(st, "ssdp:all") == 0) {
		type = MSEARCH_ALL;
	} else if (strcmp(st, "upnp:rootdevice") == 0) {
		type = MSEARCH_ROOTDEVICE;
	} else if (memcmp(st, "uuid:", 5) == 0) {
		/* uuid */
		type = MSEARCH_UUID;
		st += 5;
		strlcpy(name, st, sizeof(name));
	} else {
		/* check advertise_table for specify name. */
		UPNP_DEVCHAIN *chain;
		UPNP_ADVERTISE *advertise;
		int name_len;

		type = -1;
		for (chain = context->focus_ifp->device_chain; chain; chain = chain->next) {
			for (advertise = chain->device->advertise_table; advertise->name; advertise++) {
				name_len = strlen(advertise->name);
				if (advertise->type != ADVERTISE_UUID && memcmp(st, advertise->name, name_len) == 0) {
					/* Search target is a UPnP service */
					if (advertise->type == ADVERTISE_SERVICE) {
						type = MSEARCH_SERVICE;
						strncpy(name, st, name_len);
						name[name_len] = 0;
						goto find;
					} else if (advertise->type == ADVERTISE_DEVICE || advertise->type == ADVERTISE_ROOTDEVICE) {
						/* device and rootdevice are all devices */
						type = MSEARCH_DEVICE;
						strncpy(name, st, name_len);
						name[name_len] = 0;
						goto find;
					}
				}
			} /* advertise */
		} /* chain */
find:
		if (type == -1)
			return -1;
	}

	/* do M-SEARCH response */
	ssdp_msearch_response(context, name, type);
	return 0;
}

/* Read SSDP header */
static int read_header(UPNP_CONTEXT *context, struct upnp_method *methods)
{
	UPNP_INTERFACE *ifp = context->focus_ifp;
	socklen_t size = sizeof(struct sockaddr_in);
	int len;

	memset(&context->dst_addr, 0, size);
	len = recvfrom(context->fd, context->buf, sizeof(context->buf), 0, (struct sockaddr *)&context->dst_addr, &size);

	/* Read done */
	if (len > 0) {
		/* locate UPnP interface for the multicast socket */
		while (ifp) {
			if ((ifp->ipaddr.s_addr & ifp->netmask.s_addr) ==
			    (context->dst_addr.sin_addr.s_addr & ifp->netmask.s_addr)) {
				/* Set focus interface */
				context->focus_ifp = ifp;
				break;
			}

			ifp = ifp->next;
		}
		if (ifp == NULL)
			return -1;

		context->buf[len] = 0;
		context->end = len;

		context->dst = &context->dst_addr;

		/* Get message header */
		get_msghdr(context);
		if (context->status != 0)
			return -1;

		return 0;
	}

	return -1;
}

/* SSDP process entry */
void ssdp_process(UPNP_CONTEXT *context, int s)
{
	context->fd = s;

	upnp_http_fsm_engine(context, ssdp_fsm);
	return;
}

/* SSDP advertising timer */
void ssdp_timeout(UPNP_CONTEXT *context)
{
	ssdp_alive(context);
}

/* Add the interface IP to the SSDP multicast networking */
int ssdp_add_multi(UPNP_CONTEXT *context)
{
	UPNP_INTERFACE *ifp = context->focus_ifp;
	int ret;
	struct ip_mreq mreq;

	/*
	 * Join the interface ip to the SSDP multicast group
	 */
	inet_aton(SSDP_ADDR, &mreq.imr_multiaddr);
	mreq.imr_interface = ifp->ipaddr;
	ret = setsockopt(context->ssdp_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq));
	if (ret) {
		upnp_syslog(LOG_ERR, "IP_ADD_MEMBERSHIP errno = 0x%x", errno);
		return -1;
	}

	/* Multicast group joined successfully */
	ifp->flag |= IFF_MJOINED;
	return 0;
}

/* 
 * Delete mutlicast membership of a specific IP address
 * with regard to the SSDP socket.
 */
void ssdp_del_multi(UPNP_CONTEXT *context)
{
	UPNP_INTERFACE *ifp = context->focus_ifp;
	struct ip_mreq mreq;

	/* For ip changed or shutdown, we have to drop membership of multicast */
	if (ifp->flag & IFF_MJOINED) {
		inet_aton(SSDP_ADDR, &mreq.imr_multiaddr);
		mreq.imr_interface = ifp->ipaddr;

		setsockopt(context->ssdp_sock, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&mreq, sizeof(mreq));
		ifp->flag &= ~IFF_MJOINED;
	}

	return;
}

/* Shutdown SSDP module */
void ssdp_shutdown(UPNP_CONTEXT *context)
{
	if (context->ssdp_sock != -1) {
		close(context->ssdp_sock);
		context->ssdp_sock = -1;
	}

	return;
}

/* Initialize SSDP advertisement interval and open ssdp_sock */
int ssdp_init(UPNP_CONTEXT *context)
{
	struct in_addr addr = { INADDR_ANY };

	/* save ssdp socket */
	context->ssdp_sock = oslib_udp_socket(addr, SSDP_PORT);
	if (context->ssdp_sock < 0) {
		upnp_syslog(LOG_ERR, "Cannot open ssdp_multi_sock");
		return -1;
	}

	return 0;
}
