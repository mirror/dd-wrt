/*
 * Broadcom UPnP module, xml_x_layer3forwarding.c
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: xml_x_layer3forwarding.c,v 1.5 2007/11/23 09:51:55 Exp $
 */
#include <upnp.h>

char xml_x_layer3forwarding[] = "<?xml version=\"1.0\"?>\n"
				"<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">\n"
				"<specVersion>\n"
				"<major>1</major>\n"
				"<minor>0</minor>\n"
				"</specVersion>\n"
				"<actionList>\n"
				"<action>\n"
				"<name>SetDefaultConnectionService</name>\n"
				"<argumentList>\n"
				"<argument>\n"
				"<name>NewDefaultConnectionService</name>\n"
				"<direction>in</direction>\n"
				"<relatedStateVariable>DefaultConnectionService</relatedStateVariable>\n"
				"</argument>\n"
				"</argumentList>\n"
				"</action>\n"
				"<action>\n"
				"<name>GetDefaultConnectionService</name>\n"
				"<argumentList>\n"
				"<argument>\n"
				"<name>NewDefaultConnectionService</name>\n"
				"<direction>out</direction>\n"
				"<relatedStateVariable>DefaultConnectionService</relatedStateVariable>\n"
				"</argument>\n"
				"</argumentList>\n"
				"</action>\n"
				"</actionList>\n"
				"<serviceStateTable>\n"
				"<stateVariable sendEvents=\"yes\">\n"
				"<name>DefaultConnectionService</name>\n"
				"<dataType>string</dataType>\n"
				"</stateVariable>\n"
				"</serviceStateTable>\n"
				"</scpd>\n"
				"\n";
