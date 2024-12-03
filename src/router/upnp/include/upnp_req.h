/*
 * Broadcom UPnP module request message include file
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_req.h,v 1.11 2008/08/25 08:22:28 Exp $
 */

#ifndef __UPNP_REQ_H__
#define __UPNP_REQ_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* UPnP Request */
#define UPNP_REQ_PORT 40100
#define UPNP_REQ_ADDR "127.0.0.1"

#define UPNP_REQ_IF_ADD 1
#define UPNP_REQ_IF_DEL 2
#define UPNP_REQ_DEV_ADD 3
#define UPNP_REQ_DEV_DEL 4
#define UPNP_REQ_GET_STATE_VAR 5
#define UPNP_REQ_SET_EVENT_VAR 6
#define UPNP_REQ_GENA_NOTIFY 7
#define UPNP_REQ_RELOAD_PORTMAP 8
#define UPNP_REQ_SET_DEVINFO 9

typedef struct upnp_request {
	int cmd;
	int status;
	char url[128];
	int num;
	UPNP_EVAR var[1];
} UPNP_REQUEST;

/* UPnP message handler */
int upnp_request_init(UPNP_CONTEXT *context);
void upnp_request_shutdown(UPNP_CONTEXT *context);
void upnp_request_handler(UPNP_CONTEXT *context);
int upnp_request_response(UPNP_CONTEXT *context, UPNP_REQUEST *request);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __UPNP_REQ_H__ */
