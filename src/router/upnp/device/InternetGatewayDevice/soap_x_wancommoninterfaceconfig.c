/*
 * Broadcom UPnP module, soap_x_wancommoniniterfaceconfig.c
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: soap_x_wancommoninterfaceconfig.c,v 1.5 2007/11/23 09:51:55 Exp $
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

/* << AUTO GENERATED FUNCTION: statevar_WANAccessType() */
static int statevar_WANAccessType(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar, UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_STR(value))
	if (!value)
	    return SOAP_DEVICE_INTERNAL_ERROR;

	UPNP_CONST_HINT(char *pDSL = "DSL";)
	UPNP_CONST_HINT(char *pPOTS = "POTS";)
	UPNP_CONST_HINT(char *pCable = "Cable";)
	UPNP_CONST_HINT(char *pEthernet = "Ethernet";)
	UPNP_CONST_HINT(char *pOther = "Other";)

	/* << USER CODE START >> */
	strcpy(UPNP_STR(value), "Ethernet");
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_Layer1UpstreamMaxBitRate() */
static int statevar_Layer1UpstreamMaxBitRate(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar,
					     UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_UI4(value))
	if (!value)
	    return SOAP_DEVICE_INTERNAL_ERROR;

	/* << USER CODE START >> */
	unsigned long tx, rx;

	upnp_osl_wan_max_bitrates(&rx, &tx);

	UPNP_UI4(value) = tx;
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_Layer1DownstreamMaxBitRate() */
static int statevar_Layer1DownstreamMaxBitRate(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar,
					       UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_UI4(value))
	if (!value)
	    return SOAP_DEVICE_INTERNAL_ERROR;

	/* << USER CODE START >> */
	unsigned long rx, tx;

	upnp_osl_wan_max_bitrates(&rx, &tx);

	UPNP_UI4(value) = rx;
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_PhysicalLinkStatus() */
static int statevar_PhysicalLinkStatus(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar, UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_STR(value))
	if (!value)
	    return SOAP_DEVICE_INTERNAL_ERROR;

	UPNP_CONST_HINT(char *pUp = "Up";)
	UPNP_CONST_HINT(char *pDown = "Down";)
	UPNP_CONST_HINT(char *pInitializing = "Initializing";)
	UPNP_CONST_HINT(char *pUnavailable = "Unavailable";)

	/* << USER CODE START >> */
	int status;

	status = upnp_osl_wan_link_status();

	strcpy(UPNP_STR(value), (status ? "Up" : "Down"));
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_EnabledForInternet() */
static int statevar_EnabledForInternet(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar, UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_BOOL(value))
	if (!value)
	    return SOAP_DEVICE_INTERNAL_ERROR;

	/* << USER CODE START >> */
	/*
	 * Always return TRUE ??
	 * Maybe we make VISTA happy, and we
	 * have to block the action from guest zone, too.
	 */
	UPNP_BOOL(value) = 1;
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_MaximumActiveConnections() */
static int statevar_MaximumActiveConnections(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar,
					     UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_UI2(value))
	if (!value)
	    return SOAP_DEVICE_INTERNAL_ERROR;

	/* << USER CODE START >> */
	UPNP_UI2(value) = 1;
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_NumberOfActiveConnections() */
static int statevar_NumberOfActiveConnections(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar,
					      UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_UI2(value))
	if (!value)
	    return SOAP_DEVICE_INTERNAL_ERROR;

	/* << USER CODE START >> */
	UPNP_UI2(value) = 1;
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_TotalBytesSent() */
static int statevar_TotalBytesSent(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar, UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_UI4(value))
	if (!value)
	    return SOAP_DEVICE_INTERNAL_ERROR;

	/* << USER CODE START >> */
	if_stats_t stats;

	upnp_osl_wan_ifstats(&stats);

	UPNP_UI4(value) = stats.tx_bytes;
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_TotalBytesReceived() */
static int statevar_TotalBytesReceived(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar, UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_UI4(value))
	if (!value)
	    return SOAP_DEVICE_INTERNAL_ERROR;

	/* << USER CODE START >> */
	if_stats_t stats;

	upnp_osl_wan_ifstats(&stats);

	UPNP_UI4(value) = stats.rx_bytes;
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_TotalPacketsSent() */
static int statevar_TotalPacketsSent(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar, UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_UI4(value))
	if (!value)
	    return SOAP_DEVICE_INTERNAL_ERROR;

	/* << USER CODE START >> */
	if_stats_t stats;

	upnp_osl_wan_ifstats(&stats);

	UPNP_UI4(value) = stats.tx_packets;
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_TotalPacketsReceived() */
static int statevar_TotalPacketsReceived(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar, UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_UI4(value))
	if (!value)
	    return SOAP_DEVICE_INTERNAL_ERROR;

	/* << USER CODE START >> */
	if_stats_t stats;

	upnp_osl_wan_ifstats(&stats);

	UPNP_UI4(value) = stats.rx_packets;
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_ActiveConnectionDeviceContainer() */
static int statevar_ActiveConnectionDeviceContainer(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar,
						    UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_STR(value))
	if (!value)
	    return SOAP_DEVICE_INTERNAL_ERROR;

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
	sprintf(UPNP_STR(value), "uuid:%s:WANConnectionDevice:1", wan_advertise->uuid);

	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_ActiveConnectionServiceID() */
static int statevar_ActiveConnectionServiceID(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar,
					      UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_STR(value))
	if (!value)
	    return SOAP_DEVICE_INTERNAL_ERROR;

	/* << USER CODE START >> */
	UPNP_SERVICE *wan_service = 0;

	wan_service = get_service(context, "/control?WANIPConnection");
	if (!wan_service)
		return SOAP_DEVICE_INTERNAL_ERROR;

	sprintf(UPNP_STR(value), "urn:upnp-org:serviceId:%s", wan_service->service_id);

	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_SetEnabledForInternet() */
static int action_SetEnabledForInternet(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument,
					OUT_ARGUMENT *out_argument)
{
	UPNP_IN_HINT(IN_ARGUMENT *in_NewEnabledForInternet = UPNP_IN_ARG("NewEnabledForInternet");)

	UPNP_USE_HINT(ARG_BOOL(in_NewEnabledForInternet))

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetEnabledForInternet() */
static int action_GetEnabledForInternet(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument,
					OUT_ARGUMENT *out_argument)
{
	UPNP_OUT_HINT(OUT_ARGUMENT *out_NewEnabledForInternet = UPNP_OUT_ARG("NewEnabledForInternet");)

	UPNP_USE_HINT(ARG_BOOL(out_NewEnabledForInternet))

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewEnabledForInternet = UPNP_OUT_ARG("NewEnabledForInternet");

	if (!out_NewEnabledForInternet)
		return SOAP_DEVICE_INTERNAL_ERROR;

	return statevar_EnabledForInternet(context, service, out_NewEnabledForInternet->statevar,
					   ARG_VALUE(out_NewEnabledForInternet));
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetCommonLinkProperties() */
static int action_GetCommonLinkProperties(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument,
					  OUT_ARGUMENT *out_argument)
{
	UPNP_OUT_HINT(OUT_ARGUMENT *out_NewWANAccessType = UPNP_OUT_ARG("NewWANAccessType");)
	UPNP_OUT_HINT(OUT_ARGUMENT *out_NewLayer1UpstreamMaxBitRate = UPNP_OUT_ARG("NewLayer1UpstreamMaxBitRate");)
	UPNP_OUT_HINT(OUT_ARGUMENT *out_NewLayer1DownstreamMaxBitRate = UPNP_OUT_ARG("NewLayer1DownstreamMaxBitRate");)
	UPNP_OUT_HINT(OUT_ARGUMENT *out_NewPhysicalLinkStatus = UPNP_OUT_ARG("NewPhysicalLinkStatus");)

	UPNP_USE_HINT(ARG_STR(out_NewWANAccessType))
	UPNP_USE_HINT(ARG_UI4(out_NewLayer1UpstreamMaxBitRate))
	UPNP_USE_HINT(ARG_UI4(out_NewLayer1DownstreamMaxBitRate))
	UPNP_USE_HINT(ARG_STR(out_NewPhysicalLinkStatus))

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewWANAccessType = UPNP_OUT_ARG("NewWANAccessType");
	OUT_ARGUMENT *out_NewLayer1UpstreamMaxBitRate = UPNP_OUT_ARG("NewLayer1UpstreamMaxBitRate");
	OUT_ARGUMENT *out_NewLayer1DownstreamMaxBitRate = UPNP_OUT_ARG("NewLayer1DownstreamMaxBitRate");
	OUT_ARGUMENT *out_NewPhysicalLinkStatus = UPNP_OUT_ARG("NewPhysicalLinkStatus");

	if (!out_NewWANAccessType)
		return SOAP_DEVICE_INTERNAL_ERROR;
	if (!out_NewLayer1UpstreamMaxBitRate)
		return SOAP_DEVICE_INTERNAL_ERROR;
	if (!out_NewLayer1DownstreamMaxBitRate)
		return SOAP_DEVICE_INTERNAL_ERROR;
	if (!out_NewPhysicalLinkStatus)
		return SOAP_DEVICE_INTERNAL_ERROR;

	int status;
	unsigned long rx, tx;

	status = upnp_osl_wan_link_status();
	upnp_osl_wan_max_bitrates(&rx, &tx);

	strcpy(ARG_STR(out_NewWANAccessType), "Ethernet");
	strcpy(ARG_STR(out_NewPhysicalLinkStatus), (status ? "Up" : "Down"));
	ARG_UI4(out_NewLayer1UpstreamMaxBitRate) = tx; /* 100 Mbps */
	ARG_UI4(out_NewLayer1DownstreamMaxBitRate) = rx; /* 100 Mbps */

	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetMaximumActiveConnections() */
static int action_GetMaximumActiveConnections(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument,
					      OUT_ARGUMENT *out_argument)
{
	UPNP_OUT_HINT(OUT_ARGUMENT *out_NewMaximumActiveConnections = UPNP_OUT_ARG("NewMaximumActiveConnections");)

	UPNP_USE_HINT(ARG_UI2(out_NewMaximumActiveConnections))

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewMaximumActiveConnections = UPNP_OUT_ARG("NewMaximumActiveConnections");
	if (!out_NewMaximumActiveConnections)
		return SOAP_DEVICE_INTERNAL_ERROR;

	return statevar_MaximumActiveConnections(context, service, out_NewMaximumActiveConnections->statevar,
						 ARG_VALUE(out_NewMaximumActiveConnections));
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetTotalBytesSent() */
static int action_GetTotalBytesSent(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument,
				    OUT_ARGUMENT *out_argument)
{
	UPNP_OUT_HINT(OUT_ARGUMENT *out_NewTotalBytesSent = UPNP_OUT_ARG("NewTotalBytesSent");)

	UPNP_USE_HINT(ARG_UI4(out_NewTotalBytesSent))

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewTotalBytesSent = UPNP_OUT_ARG("NewTotalBytesSent");

	return statevar_TotalBytesSent(context, service, out_NewTotalBytesSent->statevar, ARG_VALUE(out_NewTotalBytesSent));
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetTotalPacketsSent() */
static int action_GetTotalPacketsSent(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument,
				      OUT_ARGUMENT *out_argument)
{
	UPNP_OUT_HINT(OUT_ARGUMENT *out_NewTotalPacketsSent = UPNP_OUT_ARG("NewTotalPacketsSent");)

	UPNP_USE_HINT(ARG_UI4(out_NewTotalPacketsSent))

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewTotalPacketsSent = UPNP_OUT_ARG("NewTotalPacketsSent");
	if (!out_NewTotalPacketsSent)
		return SOAP_DEVICE_INTERNAL_ERROR;

	return statevar_TotalPacketsSent(context, service, out_NewTotalPacketsSent->statevar, ARG_VALUE(out_NewTotalPacketsSent));
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetTotalBytesReceived() */
static int action_GetTotalBytesReceived(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument,
					OUT_ARGUMENT *out_argument)
{
	UPNP_OUT_HINT(OUT_ARGUMENT *out_NewTotalBytesReceived = UPNP_OUT_ARG("NewTotalBytesReceived");)

	UPNP_USE_HINT(ARG_UI4(out_NewTotalBytesReceived))

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewTotalBytesReceived = UPNP_OUT_ARG("NewTotalBytesReceived");
	if (!out_NewTotalBytesReceived)
		return SOAP_DEVICE_INTERNAL_ERROR;

	return statevar_TotalBytesReceived(context, service, out_NewTotalBytesReceived->statevar,
					   ARG_VALUE(out_NewTotalBytesReceived));
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetTotalPacketsReceived() */
static int action_GetTotalPacketsReceived(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument,
					  OUT_ARGUMENT *out_argument)
{
	UPNP_OUT_HINT(OUT_ARGUMENT *out_NewTotalPacketsReceived = UPNP_OUT_ARG("NewTotalPacketsReceived");)

	UPNP_USE_HINT(ARG_UI4(out_NewTotalPacketsReceived))

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewTotalPacketsReceived = UPNP_OUT_ARG("NewTotalPacketsReceived");

	if (!out_NewTotalPacketsReceived)
		return SOAP_DEVICE_INTERNAL_ERROR;

	return statevar_TotalPacketsReceived(context, service, out_NewTotalPacketsReceived->statevar,
					     ARG_VALUE(out_NewTotalPacketsReceived));
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetActiveConnections() */
static int action_GetActiveConnections(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument,
				       OUT_ARGUMENT *out_argument)
{
	UPNP_IN_HINT(IN_ARGUMENT *in_NewActiveConnectionIndex = UPNP_IN_ARG("NewActiveConnectionIndex");)
	UPNP_OUT_HINT(OUT_ARGUMENT *out_NewActiveConnDeviceContainer = UPNP_OUT_ARG("NewActiveConnDeviceContainer");)
	UPNP_OUT_HINT(OUT_ARGUMENT *out_NewActiveConnectionServiceID = UPNP_OUT_ARG("NewActiveConnectionServiceID");)

	UPNP_USE_HINT(ARG_UI2(in_NewActiveConnectionIndex))
	UPNP_USE_HINT(ARG_STR(out_NewActiveConnDeviceContainer))
	UPNP_USE_HINT(ARG_STR(out_NewActiveConnectionServiceID))

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewActiveConnDeviceContainer = UPNP_OUT_ARG("NewActiveConnDeviceContainer");
	OUT_ARGUMENT *out_NewActiveConnectionServiceID = UPNP_OUT_ARG("NewActiveConnectionServiceID");

	if (!out_NewActiveConnDeviceContainer)
		return SOAP_DEVICE_INTERNAL_ERROR;
	if (!out_NewActiveConnectionServiceID)
		return SOAP_DEVICE_INTERNAL_ERROR;

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
	sprintf(ARG_STR(out_NewActiveConnDeviceContainer), "uuid:%s:WANConnectionDevice:1", wan_advertise->uuid);

	sprintf(ARG_STR(out_NewActiveConnectionServiceID), "urn:upnp-org:serviceId:%s", wan_service->service_id);

	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << TABLE BEGIN */
/*
 * WARNNING: DON'T MODIFY THE FOLLOWING TABLES
 * AND DON'T REMOVE TAG :
 *          "<< TABLE BEGIN"
 *          ">> TABLE END"
 */

#define STATEVAR_ACTIVECONNECTIONDEVICECONTAINER 0
#define STATEVAR_ACTIVECONNECTIONSERVICEID 1
#define STATEVAR_ENABLEDFORINTERNET 2
#define STATEVAR_LAYER1DOWNSTREAMMAXBITRATE 3
#define STATEVAR_LAYER1UPSTREAMMAXBITRATE 4
#define STATEVAR_MAXIMUMACTIVECONNECTIONS 5
#define STATEVAR_NUMBEROFACTIVECONNECTIONS 6
#define STATEVAR_PHYSICALLINKSTATUS 7
#define STATEVAR_TOTALBYTESRECEIVED 8
#define STATEVAR_TOTALBYTESSENT 9
#define STATEVAR_TOTALPACKETSRECEIVED 10
#define STATEVAR_TOTALPACKETSSENT 11
#define STATEVAR_WANACCESSTYPE 12

/* State Variable Table */
UPNP_STATE_VAR statevar_x_wancommoninterfaceconfig[] = {
	{ 0, "ActiveConnectionDeviceContainer", UPNP_TYPE_STR, &statevar_ActiveConnectionDeviceContainer, 0 },
	{ 0, "ActiveConnectionServiceID", UPNP_TYPE_STR, &statevar_ActiveConnectionServiceID, 0 },
	{ 0, "EnabledForInternet", UPNP_TYPE_BOOL, &statevar_EnabledForInternet, 1 },
	{ 0, "Layer1DownstreamMaxBitRate", UPNP_TYPE_UI4, &statevar_Layer1DownstreamMaxBitRate, 0 },
	{ 0, "Layer1UpstreamMaxBitRate", UPNP_TYPE_UI4, &statevar_Layer1UpstreamMaxBitRate, 0 },
	{ 0, "MaximumActiveConnections", UPNP_TYPE_UI2, &statevar_MaximumActiveConnections, 0 },
	{ 0, "NumberOfActiveConnections", UPNP_TYPE_UI2, &statevar_NumberOfActiveConnections, 0 },
	{ 0, "PhysicalLinkStatus", UPNP_TYPE_STR, &statevar_PhysicalLinkStatus, 1 },
	{ 0, "TotalBytesReceived", UPNP_TYPE_UI4, &statevar_TotalBytesReceived, 0 },
	{ 0, "TotalBytesSent", UPNP_TYPE_UI4, &statevar_TotalBytesSent, 0 },
	{ 0, "TotalPacketsReceived", UPNP_TYPE_UI4, &statevar_TotalPacketsReceived, 0 },
	{ 0, "TotalPacketsSent", UPNP_TYPE_UI4, &statevar_TotalPacketsSent, 0 },
	{ 0, "WANAccessType", UPNP_TYPE_STR, &statevar_WANAccessType, 0 },
	{ 0, 0, 0, 0, 0 }
};

/* Action Table */
static ACTION_ARGUMENT arg_in_SetEnabledForInternet[] = { { "NewEnabledForInternet", UPNP_TYPE_BOOL,
							    STATEVAR_ENABLEDFORINTERNET } };

static ACTION_ARGUMENT arg_out_GetEnabledForInternet[] = { { "NewEnabledForInternet", UPNP_TYPE_BOOL,
							     STATEVAR_ENABLEDFORINTERNET } };

static ACTION_ARGUMENT arg_out_GetCommonLinkProperties[] = {
	{ "NewWANAccessType", UPNP_TYPE_STR, STATEVAR_WANACCESSTYPE },
	{ "NewLayer1UpstreamMaxBitRate", UPNP_TYPE_UI4, STATEVAR_LAYER1UPSTREAMMAXBITRATE },
	{ "NewLayer1DownstreamMaxBitRate", UPNP_TYPE_UI4, STATEVAR_LAYER1DOWNSTREAMMAXBITRATE },
	{ "NewPhysicalLinkStatus", UPNP_TYPE_STR, STATEVAR_PHYSICALLINKSTATUS }
};

static ACTION_ARGUMENT arg_out_GetMaximumActiveConnections[] = { { "NewMaximumActiveConnections", UPNP_TYPE_UI2,
								   STATEVAR_MAXIMUMACTIVECONNECTIONS } };

static ACTION_ARGUMENT arg_out_GetTotalBytesSent[] = { { "NewTotalBytesSent", UPNP_TYPE_UI4, STATEVAR_TOTALBYTESSENT } };

static ACTION_ARGUMENT arg_out_GetTotalPacketsSent[] = { { "NewTotalPacketsSent", UPNP_TYPE_UI4, STATEVAR_TOTALPACKETSSENT } };

static ACTION_ARGUMENT arg_out_GetTotalBytesReceived[] = { { "NewTotalBytesReceived", UPNP_TYPE_UI4,
							     STATEVAR_TOTALBYTESRECEIVED } };

static ACTION_ARGUMENT arg_out_GetTotalPacketsReceived[] = { { "NewTotalPacketsReceived", UPNP_TYPE_UI4,
							       STATEVAR_TOTALPACKETSRECEIVED } };

static ACTION_ARGUMENT arg_in_GetActiveConnections[] = { { "NewActiveConnectionIndex", UPNP_TYPE_UI2,
							   STATEVAR_NUMBEROFACTIVECONNECTIONS } };

static ACTION_ARGUMENT arg_out_GetActiveConnections[] = {
	{ "NewActiveConnDeviceContainer", UPNP_TYPE_STR, STATEVAR_ACTIVECONNECTIONDEVICECONTAINER },
	{ "NewActiveConnectionServiceID", UPNP_TYPE_STR, STATEVAR_ACTIVECONNECTIONSERVICEID }
};

UPNP_ACTION action_x_wancommoninterfaceconfig[] = {
	{ "GetActiveConnections", 1, arg_in_GetActiveConnections, 2, arg_out_GetActiveConnections, &action_GetActiveConnections },
	{ "GetCommonLinkProperties", 0, 0, 4, arg_out_GetCommonLinkProperties, &action_GetCommonLinkProperties },
	{ "GetEnabledForInternet", 0, 0, 1, arg_out_GetEnabledForInternet, &action_GetEnabledForInternet },
	{ "GetMaximumActiveConnections", 0, 0, 1, arg_out_GetMaximumActiveConnections, &action_GetMaximumActiveConnections },
	{ "GetTotalBytesReceived", 0, 0, 1, arg_out_GetTotalBytesReceived, &action_GetTotalBytesReceived },
	{ "GetTotalBytesSent", 0, 0, 1, arg_out_GetTotalBytesSent, &action_GetTotalBytesSent },
	{ "GetTotalPacketsReceived", 0, 0, 1, arg_out_GetTotalPacketsReceived, &action_GetTotalPacketsReceived },
	{ "GetTotalPacketsSent", 0, 0, 1, arg_out_GetTotalPacketsSent, &action_GetTotalPacketsSent },
	{ "SetEnabledForInternet", 1, arg_in_SetEnabledForInternet, 0, 0, &action_SetEnabledForInternet },
	{ 0, 0, 0, 0, 0, 0 }
};
/* >> TABLE END */
