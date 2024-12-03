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
#endif /* __cplusplus */

#include <upnp_type.h>

#define SOAP_MAX_ERRMSG 256
#define SOAP_MAX_BUF 2048

enum SOAP_ERROR_E {
	SOAP_INVALID_ACTION = 401,
	SOAP_INVALID_ARGS,
	SOAP_OUT_OF_SYNC,
	SOAP_INVALID_VARIABLE,
	SOAP_DEVICE_INTERNAL_ERROR = 501,
	SOAP_ARGUMENT_VALUE_INVALID = 600,
	SOAP_ARGUMENT_VALUE_OUT_OF_RANGE,
	SOAP_OPTIONAL_ACTION_NOT_IMPLEMENTED,
	SOAP_OUT_OF_MEMORY,
	SOAP_HUMAN_INTERVENTION_REQUIRED,
	SOAP_STRING_ARGUMENT_TOO_LONG,
	SOAP_ACTION_NOT_AUTHORIZED,
	SOAP_SIGNATURE_FAILURE,
	SOAP_SIGNATURE_MISSING,
	SOAP_NOT_ENCRYPTED,
	SOAP_INVALID_SEQUENCE,
	SOAP_INVALID_CONTROL_URL,
	SOAP_NO_SUCH_SESSION
};

/*
 * Functions
 */
UPNP_SERVICE *get_service(UPNP_CONTEXT *context, char *purl);
UPNP_ADVERTISE *get_advertise(UPNP_CONTEXT *context, char *name);

int soap_process(UPNP_CONTEXT *context);
int soap_get_state_var(UPNP_CONTEXT *context, char *url, char *name, UPNP_VALUE *value);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __SOAP_H__ */
