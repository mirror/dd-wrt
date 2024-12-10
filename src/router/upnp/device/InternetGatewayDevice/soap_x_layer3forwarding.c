/*
 * Broadcom UPnP module, soap_x_layer3forwarding.c
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: soap_x_layer3forwarding.c,v 1.4 2007/11/23 09:51:55 Exp $
 */
#include <upnp.h>
#include <InternetGatewayDevice.h>

/*
 * WARNNING: PLEASE IMPLEMENT YOUR CODES AFTER 
 *          "<< USER CODE START >>"
 * AND DON'T REMOVE TAG :
 *          "<< AUTO GENERATED FUNCTION: "
 *          ">> AUTO GENERATED FUNCTION"
 *          "<< USER CODE START >>"
 */

/* << AUTO GENERATED FUNCTION: statevar_DefaultConnectionService() */
static int statevar_DefaultConnectionService(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar,
					     UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_STR(value))

	/* << USER CODE START >> */
	UPNP_SERVICE *wan_service = 0;
	UPNP_ADVERTISE *wan_advertise = 0;

	wan_service = get_service(context, "/control?WANIPConnection");
	if (!wan_service) {
		return SOAP_DEVICE_INTERNAL_ERROR;
	}
	wan_advertise = get_advertise(context, wan_service->name);
	if (!wan_advertise) {
		return SOAP_DEVICE_INTERNAL_ERROR;
	}
	/* Construct out string */
	sprintf(UPNP_STR(value), "uuid:%s:WANConnectionDevice:1,urn:upnp-org:serviceId:%s", wan_advertise->uuid,
		wan_service->service_id);

	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_SetDefaultConnectionService() */
static int action_SetDefaultConnectionService(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument,
					      OUT_ARGUMENT *out_argument)
{
	UPNP_IN_HINT(IN_ARGUMENT *in_NewDefaultConnectionService = UPNP_IN_ARG("NewDefaultConnectionService");)

	UPNP_USE_HINT(ARG_STR(in_NewDefaultConnectionService))

	/* << USER CODE START >> */
	/* modify default connection service */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetDefaultConnectionService() */
static int action_GetDefaultConnectionService(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument,
					      OUT_ARGUMENT *out_argument)
{
	UPNP_OUT_HINT(OUT_ARGUMENT *out_NewDefaultConnectionService = UPNP_OUT_ARG("NewDefaultConnectionService");)

	UPNP_USE_HINT(ARG_STR(out_NewDefaultConnectionService))

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewDefaultConnectionService = UPNP_OUT_ARG("NewDefaultConnectionService");
	if (!out_NewDefaultConnectionService)
		return SOAP_DEVICE_INTERNAL_ERROR;

	return statevar_DefaultConnectionService(context, service, out_NewDefaultConnectionService->statevar,
						 ARG_VALUE(out_NewDefaultConnectionService));
}
/* >> AUTO GENERATED FUNCTION */

/* << TABLE BEGIN */
/*
 * WARNNING: DON'T MODIFY THE FOLLOWING TABLES
 * AND DON'T REMOVE TAG :
 *          "<< TABLE BEGIN"
 *          ">> TABLE END"
 */

#define STATEVAR_DEFAULTCONNECTIONSERVICE 0

/* State Variable Table */
UPNP_STATE_VAR statevar_x_layer3forwarding[] = { { 0, "DefaultConnectionService", UPNP_TYPE_STR, &statevar_DefaultConnectionService,
						   1 },
						 { 0, 0, 0, 0, 0 } };

/* Action Table */
static ACTION_ARGUMENT arg_in_SetDefaultConnectionService[] = { { "NewDefaultConnectionService", UPNP_TYPE_STR,
								  STATEVAR_DEFAULTCONNECTIONSERVICE } };

static ACTION_ARGUMENT arg_out_GetDefaultConnectionService[] = { { "NewDefaultConnectionService", UPNP_TYPE_STR,
								   STATEVAR_DEFAULTCONNECTIONSERVICE } };

UPNP_ACTION action_x_layer3forwarding[] = {
	{ "GetDefaultConnectionService", 0, 0, 1, arg_out_GetDefaultConnectionService, &action_GetDefaultConnectionService },
	{ "SetDefaultConnectionService", 1, arg_in_SetDefaultConnectionService, 0, 0, &action_SetDefaultConnectionService },
	{ 0, 0, 0, 0, 0, 0 }
};
/* >> TABLE END */
