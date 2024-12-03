/*
 * Broadcom UPnP module, InternetGatewayDevice_table.c
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: InternetGatewayDevice_table.c,v 1.6 2008/08/25 08:16:14 Exp $
 */
#include <upnp.h>
#include <InternetGatewayDevice.h>

/* << TABLE BEGIN */
/*
 * WARNNING: DON'T MODIFY THE FOLLOWING TABLES
 * AND DON'T REMOVE TAG :
 *          "<< TABLE BEGIN"
 *          ">> TABLE END"
 */

extern UPNP_ACTION action_x_lanhostconfigmanagement[];
extern UPNP_ACTION action_x_layer3forwarding[];
extern UPNP_ACTION action_x_wancommoninterfaceconfig[];
extern UPNP_ACTION action_x_wanipconnection[];

extern UPNP_STATE_VAR statevar_x_lanhostconfigmanagement[];
extern UPNP_STATE_VAR statevar_x_layer3forwarding[];
extern UPNP_STATE_VAR statevar_x_wancommoninterfaceconfig[];
extern UPNP_STATE_VAR statevar_x_wanipconnection[];

static UPNP_SERVICE InternetGatewayDevice_service[] = {
	{ "/control?LANHostConfigManagement", "/event?LANHostConfigManagement",
	  "urn:schemas-upnp-org:service:LANHostConfigManagement", "LANHostCfg1", action_x_lanhostconfigmanagement,
	  statevar_x_lanhostconfigmanagement },
	{ "/control?Layer3Forwarding", "/event?Layer3Forwarding", "urn:schemas-upnp-org:service:Layer3Forwarding", "L3Forwarding1",
	  action_x_layer3forwarding, statevar_x_layer3forwarding },
	{ "/control?WANCommonInterfaceConfig", "/event?WANCommonInterfaceConfig",
	  "urn:schemas-upnp-org:service:WANCommonInterfaceConfig", "WANCommonIFC1", action_x_wancommoninterfaceconfig,
	  statevar_x_wancommoninterfaceconfig },
	{ "/control?WANIPConnection", "/event?WANIPConnection", "urn:schemas-upnp-org:service:WANIPConnection", "WANIPConn1",
	  action_x_wanipconnection, statevar_x_wanipconnection },
	{ 0, 0, 0, 0, 0, 0 }
};

static UPNP_ADVERTISE InternetGatewayDevice_advertise[] = {
	{ "urn:schemas-upnp-org:device:InternetGatewayDevice", "eb9ab5b2-981c-4401-a20e-b7bcde359dbb", ADVERTISE_ROOTDEVICE },
	{ "urn:schemas-upnp-org:service:Layer3Forwarding", "eb9ab5b2-981c-4401-a20e-b7bcde359dbb", ADVERTISE_SERVICE },
	{ "urn:schemas-upnp-org:device:WANDevice", "e1f05c9d-3034-4e4c-af82-17cdfbdcc077", ADVERTISE_DEVICE },
	{ "urn:schemas-upnp-org:service:WANCommonInterfaceConfig", "e1f05c9d-3034-4e4c-af82-17cdfbdcc077", ADVERTISE_SERVICE },
	{ "urn:schemas-upnp-org:device:WANConnectionDevice", "1995cf2d-d4b1-4fdb-bf84-8e59d2066198", ADVERTISE_DEVICE },
	{ "urn:schemas-upnp-org:service:WANIPConnection", "1995cf2d-d4b1-4fdb-bf84-8e59d2066198", ADVERTISE_SERVICE },
	{ "urn:schemas-upnp-org:device:LANDevice", "f9e2cf4a-99e3-429e-b2dd-57e3c03ae597", ADVERTISE_DEVICE },
	{ "urn:schemas-upnp-org:service:LANHostConfigManagement", "f9e2cf4a-99e3-429e-b2dd-57e3c03ae597", ADVERTISE_SERVICE },
	{ 0, "" }
};

extern char xml_InternetGatewayDevice[];
extern char xml_InternetGatewayDevice_real[];
extern char xml_x_lanhostconfigmanagement[];
extern char xml_x_layer3forwarding[];
extern char xml_x_wancommoninterfaceconfig[];
extern char xml_x_wanipconnection[];

static UPNP_DESCRIPTION InternetGatewayDevice_description[] = { { "/InternetGatewayDevice.xml", xml_InternetGatewayDevice_real },
								{ "/x_lanhostconfigmanagement.xml", xml_x_lanhostconfigmanagement },
								{ "/x_layer3forwarding.xml", xml_x_layer3forwarding },
								{ "/x_wancommoninterfaceconfig.xml",
								  xml_x_wancommoninterfaceconfig },
								{ "/x_wanipconnection.xml", xml_x_wanipconnection },
								{ 0, 0 } };

UPNP_DEVICE InternetGatewayDevice = { NULL,
				      "InternetGatewayDevice.xml",
				      InternetGatewayDevice_service,
				      InternetGatewayDevice_advertise,
				      InternetGatewayDevice_description,
				      InternetGatewayDevice_common_init,
				      InternetGatewayDevice_open,
				      InternetGatewayDevice_close,
				      InternetGatewayDevice_request,
				      InternetGatewayDevice_timeout,
				      InternetGatewayDevice_notify };
/* >> TABLE END */
