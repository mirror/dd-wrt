/*
 * Broadcom UPnP module, soap_x_wanipconnection.c
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: soap_x_wanipconnection.c,v 1.7 2008/06/23 06:35:54 Exp $
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

/* << AUTO GENERATED FUNCTION: statevar_ConnectionType() */
static int
statevar_ConnectionType
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_STATE_VAR * statevar,
	UPNP_VALUE * value
)
{
	UPNP_USE_HINT( UPNP_STR(value) )
	
	/* << USER CODE START >> */
	strcpy(UPNP_STR(value), "IP_Routed");
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_PossibleConnectionTypes() */
static int
statevar_PossibleConnectionTypes
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_STATE_VAR * statevar,
	UPNP_VALUE * value
)
{
	UPNP_USE_HINT( UPNP_STR(value) )
	
	UPNP_CONST_HINT(char *pUnconfigured = "Unconfigured";)
	UPNP_CONST_HINT(char *pIP_Routed = "IP_Routed";)
	UPNP_CONST_HINT(char *pIP_Bridged = "IP_Bridged";)

	/* << USER CODE START >> */
	strcpy(UPNP_STR(value), "IP_Routed");
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_ConnectionStatus() */
static int
statevar_ConnectionStatus
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_STATE_VAR * statevar,
	UPNP_VALUE * value
)
{
	UPNP_USE_HINT( UPNP_STR(value) )
	
	UPNP_CONST_HINT( char *pUnconfigured = "Unconfigured"; )
	UPNP_CONST_HINT( char *pConnected = "Connected"; )
	UPNP_CONST_HINT( char *pDisconnected = "Disconnected"; )

	/* << USER CODE START >> */
	strcpy(UPNP_STR(value), upnp_osl_wan_isup() ? "Connected" : "Disconnected");
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_Uptime() */
static int
statevar_Uptime
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_STATE_VAR * statevar,
	UPNP_VALUE * value
)
{
	UPNP_USE_HINT( UPNP_UI4(value) )
	
	/* << USER CODE START >> */
	UPNP_UI4(value) = upnp_osl_wan_uptime();
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_LastConnectionError() */
static int
statevar_LastConnectionError
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_STATE_VAR * statevar,
	UPNP_VALUE * value
)
{
	UPNP_USE_HINT( UPNP_STR(value) )
	
	UPNP_CONST_HINT( char *pERROR_NONE = "ERROR_NONE"; )
	UPNP_CONST_HINT( char *pERROR_UNKNOWN = "ERROR_UNKNOWN"; )

	/* << USER CODE START >> */
	strcpy(UPNP_STR(value), "ERROR_NONE");
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_RSIPAvailable() */
static int
statevar_RSIPAvailable
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_STATE_VAR * statevar,
	UPNP_VALUE * value
)
{
	UPNP_USE_HINT( UPNP_BOOL(value) )
	
	/* << USER CODE START >> */
    /* FALSE */
	UPNP_BOOL(value) = 0;
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_NATEnabled() */
static int
statevar_NATEnabled
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_STATE_VAR * statevar,
	UPNP_VALUE * value
)
{
	UPNP_USE_HINT( UPNP_BOOL(value) )
	
	/* << USER CODE START >> */
	/* We have to check router mode settings */
	UPNP_BOOL(value) = 1;
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_PortMappingNumberOfEntries() */
static int
statevar_PortMappingNumberOfEntries
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_STATE_VAR * statevar,
	UPNP_VALUE * value
)
{
	UPNP_USE_HINT( UPNP_UI2(value) )
	
	/* << USER CODE START >> */
	UPNP_UI2(value) = upnp_portmap_num(context);
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_ExternalIPAddress() */
static int
statevar_ExternalIPAddress
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE * service,
	UPNP_STATE_VAR * statevar,
	UPNP_VALUE * value
)
{
	UPNP_USE_HINT( UPNP_STR(value) )
	
	/* << USER CODE START >> */
	struct in_addr inaddr = {0} ;

	upnp_osl_wan_ip(&inaddr);
	strcpy(UPNP_STR(value), inet_ntoa(inaddr));
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_SetConnectionType() */
static int
action_SetConnectionType
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE *service,
	IN_ARGUMENT *in_argument,
	OUT_ARGUMENT *out_argument
)
{
	UPNP_IN_HINT( IN_ARGUMENT *in_NewConnectionType = UPNP_IN_ARG("NewConnectionType"); )
	
	UPNP_USE_HINT( ARG_STR(in_NewConnectionType) )
	
	/* << USER CODE START >> */
	IN_ARGUMENT *in_NewConnectionType = UPNP_IN_ARG("NewConnectionType");
	
	if (strcmp(ARG_STR(in_NewConnectionType), "IP_Routed") != 0)
		return SOAP_INVALID_ARGS;
	
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetConnectionTypeInfo() */
static int
action_GetConnectionTypeInfo
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE *service,
	IN_ARGUMENT *in_argument,
	OUT_ARGUMENT *out_argument
)
{
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewConnectionType = UPNP_OUT_ARG("NewConnectionType"); )
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewPossibleConnectionTypes = UPNP_OUT_ARG("NewPossibleConnectionTypes"); )
	
	UPNP_USE_HINT( ARG_STR(out_NewConnectionType) )
	UPNP_USE_HINT( ARG_STR(out_NewPossibleConnectionTypes) )
	
	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewConnectionType = UPNP_OUT_ARG("NewConnectionType");
	OUT_ARGUMENT *out_NewPossibleConnectionTypes = UPNP_OUT_ARG("NewPossibleConnectionTypes");
	
    strcpy(ARG_STR(out_NewConnectionType), "IP_Routed");
    strcpy(ARG_STR(out_NewPossibleConnectionTypes), "IP_Routed");
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_RequestConnection() */
static int
action_RequestConnection
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE *service,
	IN_ARGUMENT *in_argument,
	OUT_ARGUMENT *out_argument
)
{
	/* << USER CODE START >> */
	/* 
	 * For security consideration,
	 * we don't implement it.
	 */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_ForceTermination() */
static int
action_ForceTermination
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE *service,
	IN_ARGUMENT *in_argument,
	OUT_ARGUMENT *out_argument
)
{
	/* << USER CODE START >> */
	/* 
	 * For security consideration,
	 * we don't implement it.
	 */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetStatusInfo() */
static int
action_GetStatusInfo
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE *service,
	IN_ARGUMENT *in_argument,
	OUT_ARGUMENT *out_argument
)
{
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewConnectionStatus = UPNP_OUT_ARG("NewConnectionStatus"); )
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewLastConnectionError = UPNP_OUT_ARG("NewLastConnectionError"); )
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewUptime = UPNP_OUT_ARG("NewUptime"); )
	
	UPNP_USE_HINT( ARG_STR(out_NewConnectionStatus) )
	UPNP_USE_HINT( ARG_STR(out_NewLastConnectionError) )
	UPNP_USE_HINT( ARG_UI4(out_NewUptime) )
	
	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewConnectionStatus = UPNP_OUT_ARG("NewConnectionStatus");
	OUT_ARGUMENT *out_NewLastConnectionError = UPNP_OUT_ARG("NewLastConnectionError");
	OUT_ARGUMENT *out_NewUptime = UPNP_OUT_ARG("NewUptime");
	
	int ret;
	
    ret = statevar_ConnectionStatus(context, service, out_NewConnectionStatus->statevar, ARG_VALUE(out_NewConnectionStatus));
    if (ret != OK)
        return ret;

    ret = statevar_LastConnectionError(context, service, out_NewLastConnectionError->statevar, ARG_VALUE(out_NewLastConnectionError));
    if (ret != OK)
        return ret;
    
    return statevar_Uptime(context, service, out_NewUptime->statevar, ARG_VALUE(out_NewUptime));
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetNATRSIPStatus() */
static int
action_GetNATRSIPStatus
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE *service,
	IN_ARGUMENT *in_argument,
	OUT_ARGUMENT *out_argument
)
{
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewRSIPAvailable = UPNP_OUT_ARG("NewRSIPAvailable"); )
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewNATEnabled = UPNP_OUT_ARG("NewNATEnabled"); )
	
	UPNP_USE_HINT( ARG_BOOL(out_NewConnectionStatus) )
	UPNP_USE_HINT( ARG_BOOL(out_NewLastConnectionError) )

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewRSIPAvailable = UPNP_OUT_ARG("NewRSIPAvailable");
	OUT_ARGUMENT *out_NewNATEnabled = UPNP_OUT_ARG("NewNATEnabled");
	
	ARG_BOOL(out_NewRSIPAvailable) = 0; /* FALSE */
	ARG_BOOL(out_NewNATEnabled) = 1;	/* ??? TRUE */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetGenericPortMappingEntry() */
static int
action_GetGenericPortMappingEntry
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE *service,
	IN_ARGUMENT *in_argument,
	OUT_ARGUMENT *out_argument
)
{
	UPNP_IN_HINT( IN_ARGUMENT *in_NewPortMappingIndex = UPNP_IN_ARG("NewPortMappingIndex"); )
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewRemoteHost = UPNP_OUT_ARG("NewRemoteHost"); )
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewExternalPort = UPNP_OUT_ARG("NewExternalPort"); )
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewProtocol = UPNP_OUT_ARG("NewProtocol"); )
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewInternalPort = UPNP_OUT_ARG("NewInternalPort"); )
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewInternalClient = UPNP_OUT_ARG("NewInternalClient"); )
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewEnabled = UPNP_OUT_ARG("NewEnabled"); )
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewPortMappingDescription = UPNP_OUT_ARG("NewPortMappingDescription"); )
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewLeaseDuration = UPNP_OUT_ARG("NewLeaseDuration"); )
	
	UPNP_USE_HINT( ARG_UI2(in_NewPortMappingIndex) )
	UPNP_USE_HINT( ARG_STR(out_NewRemoteHost) )
	UPNP_USE_HINT( ARG_UI2(out_NewExternalPort) )
	UPNP_USE_HINT( ARG_STR(out_NewProtocol) )
	UPNP_USE_HINT( ARG_UI2(out_NewInternalPort) )
	UPNP_USE_HINT( ARG_STR(out_NewInternalClient) )
	UPNP_USE_HINT( ARG_BOOL(out_NewEnabled) )
	UPNP_USE_HINT( ARG_STR(out_NewPortMappingDescription) )
	UPNP_USE_HINT( ARG_UI4(out_NewLeaseDuration) )

	/* << USER CODE START >> */
	IN_ARGUMENT *in_NewPortMappingIndex = UPNP_IN_ARG("NewPortMappingIndex");
	OUT_ARGUMENT *out_NewRemoteHost = UPNP_OUT_ARG("NewRemoteHost");
	OUT_ARGUMENT *out_NewExternalPort = UPNP_OUT_ARG("NewExternalPort");
	OUT_ARGUMENT *out_NewProtocol = UPNP_OUT_ARG("NewProtocol");
	OUT_ARGUMENT *out_NewInternalPort = UPNP_OUT_ARG("NewInternalPort");
	OUT_ARGUMENT *out_NewInternalClient = UPNP_OUT_ARG("NewInternalClient");
	OUT_ARGUMENT *out_NewEnabled = UPNP_OUT_ARG("NewEnabled");
	OUT_ARGUMENT *out_NewPortMappingDescription = UPNP_OUT_ARG("NewPortMappingDescription");
	OUT_ARGUMENT *out_NewLeaseDuration = UPNP_OUT_ARG("NewLeaseDuration");
   
    UPNP_PORTMAP *map;
    
    map = upnp_portmap_with_index(context, ARG_UI2(in_NewPortMappingIndex));
    if (!map) {
        return SOAP_SPECIFIED_ARRAY_INDEX_INVALID;
    }
    strcpy(ARG_STR(out_NewRemoteHost), map->remote_host);
    strcpy(ARG_STR(out_NewProtocol), map->protocol);
    strcpy(ARG_STR(out_NewInternalClient), map->internal_client);
    strcpy(ARG_STR(out_NewPortMappingDescription), map->description);
	ARG_UI2(out_NewExternalPort) = map->external_port;
    ARG_UI2(out_NewInternalPort) = map->internal_port;
    ARG_BOOL(out_NewEnabled) = map->enable;
    ARG_UI4(out_NewLeaseDuration) = map->duration;

	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetSpecificPortMappingEntry() */
static int
action_GetSpecificPortMappingEntry
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE *service,
	IN_ARGUMENT *in_argument,
	OUT_ARGUMENT *out_argument
)
{
	UPNP_IN_HINT( IN_ARGUMENT *in_NewRemoteHost = UPNP_IN_ARG("NewRemoteHost"); )
	UPNP_IN_HINT( IN_ARGUMENT *in_NewExternalPort = UPNP_IN_ARG("NewExternalPort"); )
	UPNP_IN_HINT( IN_ARGUMENT *in_NewProtocol = UPNP_IN_ARG("NewProtocol"); )
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewInternalPort = UPNP_OUT_ARG("NewInternalPort"); )
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewInternalClient = UPNP_OUT_ARG("NewInternalClient"); )
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewEnabled = UPNP_OUT_ARG("NewEnabled"); )
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewPortMappingDescription = UPNP_OUT_ARG("NewPortMappingDescription"); )
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewLeaseDuration = UPNP_OUT_ARG("NewLeaseDuration"); )
	
	UPNP_USE_HINT( ARG_STR(in_NewRemoteHost) )
	UPNP_USE_HINT( ARG_UI2(in_NewExternalPort) )
	UPNP_USE_HINT( ARG_STR(in_NewProtocol) )
	UPNP_USE_HINT( ARG_UI2(out_NewInternalPort) )
	UPNP_USE_HINT( ARG_STR(out_NewInternalClient) )
	UPNP_USE_HINT( ARG_BOOL(out_NewEnabled) )
	UPNP_USE_HINT( ARG_STR(out_NewPortMappingDescription) )
	UPNP_USE_HINT( ARG_UI4(out_NewLeaseDuration) )
	
	/* << USER CODE START >> */
	IN_ARGUMENT *in_NewRemoteHost = UPNP_IN_ARG("NewRemoteHost");
	IN_ARGUMENT *in_NewExternalPort = UPNP_IN_ARG("NewExternalPort");
	IN_ARGUMENT *in_NewProtocol = UPNP_IN_ARG("NewProtocol");
	OUT_ARGUMENT *out_NewInternalPort = UPNP_OUT_ARG("NewInternalPort");
	OUT_ARGUMENT *out_NewInternalClient = UPNP_OUT_ARG("NewInternalClient");
	OUT_ARGUMENT *out_NewEnabled = UPNP_OUT_ARG("NewEnabled");
	OUT_ARGUMENT *out_NewPortMappingDescription = UPNP_OUT_ARG("NewPortMappingDescription");
	OUT_ARGUMENT *out_NewLeaseDuration = UPNP_OUT_ARG("NewLeaseDuration");
	
    UPNP_PORTMAP *map;
    map = upnp_portmap_find(context,
							ARG_STR(in_NewRemoteHost), 
							ARG_UI2(in_NewExternalPort),
							ARG_STR(in_NewProtocol));
	
	if (!map) {
		return SOAP_NO_SUCH_ENTRY_IN_ARRAY;
	}
    
	strcpy(ARG_STR(out_NewInternalClient), map->internal_client);
	strcpy(ARG_STR(out_NewPortMappingDescription), map->description);
	ARG_UI2(out_NewInternalPort) = map->internal_port;
	ARG_BOOL(out_NewEnabled) = map->enable;
	ARG_UI4(out_NewLeaseDuration) = map->duration;

	return OK;
}
/* >> AUTO GENERATED FUNCTION */


/* << AUTO GENERATED FUNCTION: action_AddPortMapping() */
static int
action_AddPortMapping
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE *service,
	IN_ARGUMENT *in_argument,
	OUT_ARGUMENT *out_argument
)
{
	UPNP_IN_HINT( IN_ARGUMENT *in_NewRemoteHost = UPNP_IN_ARG("NewRemoteHost"); )
	UPNP_IN_HINT( IN_ARGUMENT *in_NewExternalPort = UPNP_IN_ARG("NewExternalPort"); )
	UPNP_IN_HINT( IN_ARGUMENT *in_NewProtocol = UPNP_IN_ARG("NewProtocol"); )
	UPNP_IN_HINT( IN_ARGUMENT *in_NewInternalPort = UPNP_IN_ARG("NewInternalPort"); )
	UPNP_IN_HINT( IN_ARGUMENT *in_NewInternalClient = UPNP_IN_ARG("NewInternalClient"); )
	UPNP_IN_HINT( IN_ARGUMENT *in_NewEnabled = UPNP_IN_ARG("NewEnabled"); )
	UPNP_IN_HINT( IN_ARGUMENT *in_NewPortMappingDescription = UPNP_IN_ARG("NewPortMappingDescription"); )
	UPNP_IN_HINT( IN_ARGUMENT *in_NewLeaseDuration = UPNP_IN_ARG("NewLeaseDuration"); )
	
	UPNP_USE_HINT( ARG_STR(in_NewRemoteHost) )
	UPNP_USE_HINT( ARG_UI2(in_NewExternalPort) )
	UPNP_USE_HINT( ARG_STR(in_NewProtocol) )
	UPNP_USE_HINT( ARG_UI2(in_NewInternalPort) )
	UPNP_USE_HINT( ARG_STR(in_NewInternalClient) )
	UPNP_USE_HINT( ARG_BOOL(in_NewEnabled) )
	UPNP_USE_HINT( ARG_STR(in_NewPortMappingDescription) )
	UPNP_USE_HINT( ARG_UI4(in_NewLeaseDuration) )
	
	/* << USER CODE START >> */
	IN_ARGUMENT *in_NewRemoteHost = UPNP_IN_ARG("NewRemoteHost");
	IN_ARGUMENT *in_NewExternalPort = UPNP_IN_ARG("NewExternalPort");
	IN_ARGUMENT *in_NewProtocol = UPNP_IN_ARG("NewProtocol");
	IN_ARGUMENT *in_NewInternalPort = UPNP_IN_ARG("NewInternalPort");
	IN_ARGUMENT *in_NewInternalClient = UPNP_IN_ARG("NewInternalClient");
	IN_ARGUMENT *in_NewEnabled = UPNP_IN_ARG("NewEnabled");
	IN_ARGUMENT *in_NewPortMappingDescription = UPNP_IN_ARG("NewPortMappingDescription");
	IN_ARGUMENT *in_NewLeaseDuration = UPNP_IN_ARG("NewLeaseDuration");

	UPNP_STATE_VAR *statevar;
	UPNP_VALUE value = {UPNP_TYPE_UI2, 2};

       int error = upnp_portmap_add(context,
						ARG_STR(in_NewRemoteHost),
						ARG_UI2(in_NewExternalPort), 
						ARG_STR(in_NewProtocol),
						ARG_UI2(in_NewInternalPort),
						ARG_STR(in_NewInternalClient),
						ARG_BOOL(in_NewEnabled),
						ARG_STR(in_NewPortMappingDescription),
						ARG_UI4(in_NewLeaseDuration));
	if (error)
	{
    	    return error;
	}

	/* Update "PortMappingNumberOfEntries" */
	statevar = find_event_var(context, service, "PortMappingNumberOfEntries");
	if (statevar == 0)
		return OK;

	UPNP_UI2(&value) = upnp_portmap_num(context);
	gena_update_event_var(context, service, statevar, &value);
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_DeletePortMapping() */
static int
action_DeletePortMapping
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE *service,
	IN_ARGUMENT *in_argument,
	OUT_ARGUMENT *out_argument
)
{
	UPNP_IN_HINT( IN_ARGUMENT *in_NewRemoteHost = UPNP_IN_ARG("NewRemoteHost"); )
	UPNP_IN_HINT( IN_ARGUMENT *in_NewExternalPort = UPNP_IN_ARG("NewExternalPort"); )
	UPNP_IN_HINT( IN_ARGUMENT *in_NewProtocol = UPNP_IN_ARG("NewProtocol"); )
	
	UPNP_USE_HINT( ARG_STR(in_NewRemoteHost) )
	UPNP_USE_HINT( ARG_UI2(in_NewExternalPort) )
	UPNP_USE_HINT( ARG_STR(in_NewProtocol) )
	
	/* << USER CODE START >> */
	IN_ARGUMENT *in_NewRemoteHost = UPNP_IN_ARG("NewRemoteHost");
	IN_ARGUMENT *in_NewExternalPort = UPNP_IN_ARG("NewExternalPort");
	IN_ARGUMENT *in_NewProtocol = UPNP_IN_ARG("NewProtocol");

	UPNP_STATE_VAR *statevar;
	UPNP_VALUE value = {UPNP_TYPE_UI2, 2};

    int error = upnp_portmap_del(context,
						ARG_STR(in_NewRemoteHost),
						ARG_UI2(in_NewExternalPort),
						ARG_STR(in_NewProtocol));
    if (error) {
        return error;
    }

	/* Update "PortMappingNumberOfEntries" */
	statevar = find_event_var(context, service, "PortMappingNumberOfEntries");
	if (statevar == 0)
		return OK;

	UPNP_UI2(&value) = upnp_portmap_num(context);
	gena_update_event_var(context, service, statevar, &value);
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetExternalIPAddress() */
static int
action_GetExternalIPAddress
(
	UPNP_CONTEXT * context,
	UPNP_SERVICE *service,
	IN_ARGUMENT *in_argument,
	OUT_ARGUMENT *out_argument
)
{
	UPNP_OUT_HINT( OUT_ARGUMENT *out_NewExternalIPAddress = UPNP_OUT_ARG("NewExternalIPAddress"); )
	
	UPNP_USE_HINT( ARG_STR(out_NewExternalIPAddress) )

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewExternalIPAddress = UPNP_OUT_ARG("NewExternalIPAddress");
	
    return statevar_ExternalIPAddress(context, service, out_NewExternalIPAddress->statevar, ARG_VALUE(out_NewExternalIPAddress));
}
/* >> AUTO GENERATED FUNCTION */


/* << TABLE BEGIN */
/*
 * WARNNING: DON'T MODIFY THE FOLLOWING TABLES
 * AND DON'T REMOVE TAG :
 *          "<< TABLE BEGIN"
 *          ">> TABLE END"
 */

#define	STATEVAR_CONNECTIONSTATUS           0
#define	STATEVAR_CONNECTIONTYPE             1
#define	STATEVAR_EXTERNALIPADDRESS          2
#define	STATEVAR_EXTERNALPORT               3
#define	STATEVAR_INTERNALCLIENT             4
#define	STATEVAR_INTERNALPORT               5
#define	STATEVAR_LASTCONNECTIONERROR        6
#define	STATEVAR_NATENABLED                 7
#define	STATEVAR_PORTMAPPINGDESCRIPTION     8
#define	STATEVAR_PORTMAPPINGENABLED         9
#define	STATEVAR_PORTMAPPINGLEASEDURATION   10
#define	STATEVAR_PORTMAPPINGNUMBEROFENTRIES 11
#define	STATEVAR_PORTMAPPINGPROTOCOL        12
#define	STATEVAR_POSSIBLECONNECTIONTYPES    13
#define	STATEVAR_RSIPAVAILABLE              14
#define	STATEVAR_REMOTEHOST                 15
#define	STATEVAR_UPTIME                     16

/* State Variable Table */
UPNP_STATE_VAR statevar_x_wanipconnection[] =
{
	{0, "ConnectionStatus",           UPNP_TYPE_STR,    &statevar_ConnectionStatus,           1},
	{0, "ConnectionType",             UPNP_TYPE_STR,    &statevar_ConnectionType,             0},
	{0, "ExternalIPAddress",          UPNP_TYPE_STR,    &statevar_ExternalIPAddress,          1},
	{0, "ExternalPort",               UPNP_TYPE_UI2,    0,                                    0},
	{0, "InternalClient",             UPNP_TYPE_STR,    0,                                    0},
	{0, "InternalPort",               UPNP_TYPE_UI2,    0,                                    0},
	{0, "LastConnectionError",        UPNP_TYPE_STR,    &statevar_LastConnectionError,        0},
	{0, "NATEnabled",                 UPNP_TYPE_BOOL,   &statevar_NATEnabled,                 0},
	{0, "PortMappingDescription",     UPNP_TYPE_STR,    0,                                    0},
	{0, "PortMappingEnabled",         UPNP_TYPE_BOOL,   0,                                    0},
	{0, "PortMappingLeaseDuration",   UPNP_TYPE_UI4,    0,                                    0},
	{0, "PortMappingNumberOfEntries", UPNP_TYPE_UI2,    &statevar_PortMappingNumberOfEntries, 1},
	{0, "PortMappingProtocol",        UPNP_TYPE_STR,    0,                                    0},
	{0, "PossibleConnectionTypes",    UPNP_TYPE_STR,    &statevar_PossibleConnectionTypes,    1},
	{0, "RSIPAvailable",              UPNP_TYPE_BOOL,   &statevar_RSIPAvailable,              0},
	{0, "RemoteHost",                 UPNP_TYPE_STR,    0,                                    0},
	{0, "Uptime",                     UPNP_TYPE_UI4,    &statevar_Uptime,                     0},
	{0, 0,                            0,                0,                                    0}
};

/* Action Table */
static ACTION_ARGUMENT arg_in_SetConnectionType [] =
{
	{"NewConnectionType",             UPNP_TYPE_STR,    STATEVAR_CONNECTIONTYPE}
};

static ACTION_ARGUMENT arg_out_GetConnectionTypeInfo [] =
{
	{"NewConnectionType",             UPNP_TYPE_STR,    STATEVAR_CONNECTIONTYPE},
	{"NewPossibleConnectionTypes",    UPNP_TYPE_STR,    STATEVAR_POSSIBLECONNECTIONTYPES}
};

static ACTION_ARGUMENT arg_out_GetStatusInfo [] =
{
	{"NewConnectionStatus",           UPNP_TYPE_STR,    STATEVAR_CONNECTIONSTATUS},
	{"NewLastConnectionError",        UPNP_TYPE_STR,    STATEVAR_LASTCONNECTIONERROR},
	{"NewUptime",                     UPNP_TYPE_UI4,    STATEVAR_UPTIME}
};

static ACTION_ARGUMENT arg_out_GetNATRSIPStatus [] =
{
	{"NewRSIPAvailable",              UPNP_TYPE_BOOL,   STATEVAR_RSIPAVAILABLE},
	{"NewNATEnabled",                 UPNP_TYPE_BOOL,   STATEVAR_NATENABLED}
};

static ACTION_ARGUMENT arg_in_GetGenericPortMappingEntry [] =
{
	{"NewPortMappingIndex",           UPNP_TYPE_UI2,    STATEVAR_PORTMAPPINGNUMBEROFENTRIES}
};

static ACTION_ARGUMENT arg_out_GetGenericPortMappingEntry [] =
{
	{"NewRemoteHost",                 UPNP_TYPE_STR,    STATEVAR_REMOTEHOST},
	{"NewExternalPort",               UPNP_TYPE_UI2,    STATEVAR_EXTERNALPORT},
	{"NewProtocol",                   UPNP_TYPE_STR,    STATEVAR_PORTMAPPINGPROTOCOL},
	{"NewInternalPort",               UPNP_TYPE_UI2,    STATEVAR_INTERNALPORT},
	{"NewInternalClient",             UPNP_TYPE_STR,    STATEVAR_INTERNALCLIENT},
	{"NewEnabled",                    UPNP_TYPE_BOOL,   STATEVAR_PORTMAPPINGENABLED},
	{"NewPortMappingDescription",     UPNP_TYPE_STR,    STATEVAR_PORTMAPPINGDESCRIPTION},
	{"NewLeaseDuration",              UPNP_TYPE_UI4,    STATEVAR_PORTMAPPINGLEASEDURATION}
};

static ACTION_ARGUMENT arg_in_GetSpecificPortMappingEntry [] =
{
	{"NewRemoteHost",                 UPNP_TYPE_STR,    STATEVAR_REMOTEHOST},
	{"NewExternalPort",               UPNP_TYPE_UI2,    STATEVAR_EXTERNALPORT},
	{"NewProtocol",                   UPNP_TYPE_STR,    STATEVAR_PORTMAPPINGPROTOCOL}
};

static ACTION_ARGUMENT arg_out_GetSpecificPortMappingEntry [] =
{
	{"NewInternalPort",               UPNP_TYPE_UI2,    STATEVAR_INTERNALPORT},
	{"NewInternalClient",             UPNP_TYPE_STR,    STATEVAR_INTERNALCLIENT},
	{"NewEnabled",                    UPNP_TYPE_BOOL,   STATEVAR_PORTMAPPINGENABLED},
	{"NewPortMappingDescription",     UPNP_TYPE_STR,    STATEVAR_PORTMAPPINGDESCRIPTION},
	{"NewLeaseDuration",              UPNP_TYPE_UI4,    STATEVAR_PORTMAPPINGLEASEDURATION}
};

static ACTION_ARGUMENT arg_in_AddPortMapping [] =
{
	{"NewRemoteHost",                 UPNP_TYPE_STR,    STATEVAR_REMOTEHOST},
	{"NewExternalPort",               UPNP_TYPE_UI2,    STATEVAR_EXTERNALPORT},
	{"NewProtocol",                   UPNP_TYPE_STR,    STATEVAR_PORTMAPPINGPROTOCOL},
	{"NewInternalPort",               UPNP_TYPE_UI2,    STATEVAR_INTERNALPORT},
	{"NewInternalClient",             UPNP_TYPE_STR,    STATEVAR_INTERNALCLIENT},
	{"NewEnabled",                    UPNP_TYPE_BOOL,   STATEVAR_PORTMAPPINGENABLED},
	{"NewPortMappingDescription",     UPNP_TYPE_STR,    STATEVAR_PORTMAPPINGDESCRIPTION},
	{"NewLeaseDuration",              UPNP_TYPE_UI4,    STATEVAR_PORTMAPPINGLEASEDURATION}
};

static ACTION_ARGUMENT arg_in_DeletePortMapping [] =
{
	{"NewRemoteHost",                 UPNP_TYPE_STR,    STATEVAR_REMOTEHOST},
	{"NewExternalPort",               UPNP_TYPE_UI2,    STATEVAR_EXTERNALPORT},
	{"NewProtocol",                   UPNP_TYPE_STR,    STATEVAR_PORTMAPPINGPROTOCOL}
};

static ACTION_ARGUMENT arg_out_GetExternalIPAddress [] =
{
	{"NewExternalIPAddress",          UPNP_TYPE_STR,    STATEVAR_EXTERNALIPADDRESS}
};

UPNP_ACTION action_x_wanipconnection[] =
{
	{"AddPortMapping",              8,  arg_in_AddPortMapping,              0,  0,                                      &action_AddPortMapping},
	{"DeletePortMapping",           3,  arg_in_DeletePortMapping,           0,  0,                                      &action_DeletePortMapping},
	{"ForceTermination",            0,  0,                                  0,  0,                                      &action_ForceTermination},
	{"GetConnectionTypeInfo",       0,  0,                                  2,  arg_out_GetConnectionTypeInfo,          &action_GetConnectionTypeInfo},
	{"GetExternalIPAddress",        0,  0,                                  1,  arg_out_GetExternalIPAddress,           &action_GetExternalIPAddress},
	{"GetGenericPortMappingEntry",  1,  arg_in_GetGenericPortMappingEntry,  8,  arg_out_GetGenericPortMappingEntry,     &action_GetGenericPortMappingEntry},
	{"GetNATRSIPStatus",            0,  0,                                  2,  arg_out_GetNATRSIPStatus,               &action_GetNATRSIPStatus},
	{"GetSpecificPortMappingEntry", 3,  arg_in_GetSpecificPortMappingEntry, 5,  arg_out_GetSpecificPortMappingEntry,    &action_GetSpecificPortMappingEntry},
	{"GetStatusInfo",               0,  0,                                  3,  arg_out_GetStatusInfo,                  &action_GetStatusInfo},
	{"RequestConnection",           0,  0,                                  0,  0,                                      &action_RequestConnection},
	{"SetConnectionType",           1,  arg_in_SetConnectionType,           0,  0,                                      &action_SetConnectionType},
	{0,                             0,  0,                                  0,  0,                                      0}
};
/* >> TABLE END */
