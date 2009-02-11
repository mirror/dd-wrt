/*
 * Broadcom UPnP module SOAP include file
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: soap.h,v 1.7 2008/06/19 06:22:26 Exp $
 */

#ifndef __SOAP_H__
#define __SOAP_H__

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

#include <upnp_type.h>

#define SOAP_MAX_ERRMSG		256
#define SOAP_MAX_BUF		2048

enum SOAP_ERROR_E {
	SOAP_INVALID_ACTION = 401,
	SOAP_INVALID_ARGS,
	SOAP_OUT_OF_SYNC,
	SOAP_INVALID_VARIABLE,
	SOAP_DEVICE_INTERNAL_ERROR = 501
};

/*
 * Functions
 */
UPNP_SERVICE	*get_service(UPNP_CONTEXT *context, char *purl);
UPNP_ADVERTISE	*get_advertise(UPNP_CONTEXT *context, char *name);

int soap_process(UPNP_CONTEXT *context);
int soap_get_state_var(UPNP_CONTEXT *context, char *url, char *name, UPNP_VALUE *value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SOAP_H__ */
