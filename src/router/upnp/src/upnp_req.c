/*
 * Broadcom UPnP module message passing by using loopback socket
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_req.c,v 1.17 2008/10/27 08:55:21 Exp $
 */

#include <upnp.h>
//#ifdef __CONFIG_NAT__
#ifdef __BCMIGD__
#include <InternetGatewayDevice.h>
#endif /* __BCMIGD__ */
//#endif /* __CONFIG_NAT__ */

/* Close the UPnP request socket */
void
upnp_request_shutdown(UPNP_CONTEXT *context)
{
	UPNP_INTERFACE	*ifp = context->focus_ifp;

	if (ifp->req_sock != -1) {
		close(ifp->req_sock);
		ifp->req_sock = -1;
	}

	return;
}

/* Open the UPnP request socket */
int
upnp_request_init(UPNP_CONTEXT *context)
{
	UPNP_INTERFACE	*ifp = context->focus_ifp;
	struct in_addr addr;

	addr.s_addr = inet_addr(UPNP_REQ_ADDR);
	ifp->req_sock = oslib_udp_socket(addr, UPNP_REQ_PORT + ifp->if_instance);
	if (ifp->req_sock < 0)
		return -1;

	return 0;
}

/* Send request response to peer */
int
upnp_request_response(UPNP_CONTEXT *context, UPNP_REQUEST *request)
{
	/* Send the response out */
	sendto(context->focus_ifp->req_sock, (void *)request, sizeof(*request), 0,
		(struct sockaddr *)&context->dst_addr, sizeof(context->dst_addr));

	return 0;
}

/* Read request message */
static int
read_request(UPNP_CONTEXT *context)
{
	socklen_t size = sizeof(struct sockaddr);
	int len;

	len = recvfrom(context->focus_ifp->req_sock, context->buf, sizeof(context->buf),
		0, (struct sockaddr *)&context->dst_addr, &size);

	/* sizeof message */
	context->end = len;
	return len;
}

static int
get_device_from_reqcmd(int cmd)
{
	switch (cmd)
	{
	case UPNP_REQ_GET_STATE_VAR:
	case UPNP_REQ_SET_EVENT_VAR:
	case UPNP_REQ_GENA_NOTIFY:
		break;

	case UPNP_REQ_DEV_ADD:
	case UPNP_REQ_DEV_DEL:
	default:
		return 1;
	}

	return 0;
}

/* Process the UPnP request message */
void
upnp_request_handler(UPNP_CONTEXT *context)
{
	int len;
	UPNP_REQUEST *request;
	UPNP_DEVICE *device = 0;
	UPNP_INTERFACE *ifp = context->focus_ifp;
	UPNP_DEVCHAIN *chain;

	/* Read message */
	len = read_request(context);
	if (len <= 0)
		return;

	request = (UPNP_REQUEST *)context->buf;
	if (get_device_from_reqcmd(request->cmd)) {
		int i;

		for (i = 0; (device = upnp_device_table[i]) != 0; i++) {
			if (strcmp(device->root_device_xml, request->url) == 0)
				break;
		}
		if (device == 0)
			return;
	}

	/* Switch the operation */
	switch (request->cmd)
	{
	case UPNP_REQ_DEV_ADD:
		upnp_syslog(LOG_INFO, "UPNP_CMD_DEV_ADD");

		upnp_device_attach(context, device);
		break;

	case UPNP_REQ_DEV_DEL:
		upnp_syslog(LOG_INFO, "UPNP_CMD_DEV_DEL");

		upnp_device_detach(context, device);
		break;

	case UPNP_REQ_GET_STATE_VAR:
		request->status = soap_get_state_var(
				context,
				request->url,
				request->var[0].statevar,
				&request->var[0].value);
		upnp_request_response(context, request);
		break;

	case UPNP_REQ_SET_EVENT_VAR:
		gena_event_alarm(context, request->url, request->num, request->var);
		break;

	case UPNP_REQ_GENA_NOTIFY:
		gena_event_alarm(context, request->url, 0, 0);
		break;

	default:
		upnp_syslog(LOG_INFO, "Device command.");
		for (chain = ifp->device_chain;
			chain;
			chain = chain->next) {
			if (chain->device == device) {
				ifp->focus_devchain = chain;
				device->request(context, request);
				break;
			}
		}
		break;
	}

	return;
}
