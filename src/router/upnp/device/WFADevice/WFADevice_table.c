/*
 * Broadcom UPnP module, WFADevice_table.c
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: WFADevice_table.c,v 1.6 2008/08/25 08:17:40 Exp $
 */
#include <upnp.h>
#include <WFADevice.h>

/* << TABLE BEGIN */
/*
 * WARNNING: DON'T MODIFY THE FOLLOWING TABLES
 * AND DON'T REMOVE TAG :
 *          "<< TABLE BEGIN"
 *          ">> TABLE END"
 */

extern UPNP_ACTION action_x_wfawlanconfig[];

extern UPNP_STATE_VAR statevar_x_wfawlanconfig[];

static UPNP_SERVICE WFADevice_service[] = { { "/control?WFAWLANConfig", "/event?WFAWLANConfig",
					      "urn:schemas-wifialliance-org:service:WFAWLANConfig", "WFAWLANConfig1",
					      action_x_wfawlanconfig, statevar_x_wfawlanconfig },
					    { 0, 0, 0, 0, 0, 0 } };
static UPNP_ADVERTISE WFADevice_advertise[] = {
	{ "urn:schemas-wifialliance-org:device:WFADevice", "00010203-0405-0607-0809-0a0b0c0d0ebb", ADVERTISE_ROOTDEVICE },
	{ "urn:schemas-wifialliance-org:service:WFAWLANConfig", "00010203-0405-0607-0809-0a0b0c0d0ebb", ADVERTISE_SERVICE },
	{
		0,
		"",
	}
};

extern char xml_WFADevice[];
extern char xml_x_wfawlanconfig[];

static UPNP_DESCRIPTION WFADevice_description[] = { { "/WFADevice.xml", xml_WFADevice },
						    { "/x_wfawlanconfig.xml", xml_x_wfawlanconfig },
						    { 0, 0 } };

UPNP_DEVICE WFADevice = {
	NULL,		"WFADevice.xml", WFADevice_service, WFADevice_advertise, WFADevice_description, WFADevice_common_init,
	WFADevice_open, WFADevice_close, WFADevice_request, WFADevice_timeout,	 WFADevice_notify
};
/* >> TABLE END */
