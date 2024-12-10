/*
 * Broadcom UPnP module, xml_InternetGatewayDevice.c
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: xml_InternetGatewayDevice.c,v 1.6 2008/01/29 10:14:07 Exp $
 */
#include <upnp.h>

char xml_InternetGatewayDevice[] = "<?xml version=\"1.0\"?>\n"
				   "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">\n"
				   "<specVersion>\n"
				   "<major>1</major>\n"
				   "<minor>0</minor>\n"
				   "</specVersion>\n"
				   "<device>\n"
				   "<deviceType>urn:schemas-upnp-org:device:InternetGatewayDevice:1</deviceType>\n"
#ifdef HAVE_BUFFALO
				   "<manufacturer>Buffalo Inc.</manufacturer>\n"
				   "<manufacturerURL>http://www.buffalo.jp</manufacturerURL>\n"
				   "<modelDescription>Gateway</modelDescription>\n"
#ifdef HAVE_WHRHPG300N
				   "<friendlyName>Buffalo WHR-HP-G300N</friendlyName>\n"
				   "<modelName>Buffalo WHR-HP-G300N</modelName>\n"
#elif HAVE_WHRHPGN
				   "<friendlyName>Buffalo WHR-HP-GN</friendlyName>\n"
				   "<modelName>Buffalo WHR-HP-GN</modelName>\n"
#elif HAVE_WZRG300NH
				   "<friendlyName>Buffalo WZR-HP-G300NH</friendlyName>\n"
				   "<modelName>Buffalo WZR-HP-G300NH</modelName>\n"
#elif HAVE_WZRG300NH2
				   "<friendlyName>Buffalo WZR-HP-G300NH2</friendlyName>\n"
				   "<modelName>Buffalo WZR-HP-G300NH2</modelName>\n"
#elif HAVE_WZRHPAG300NH
				   "<friendlyName>Buffalo WZR-HP-AG300H</friendlyName>\n"
				   "<modelName>Buffalo WZR-HP-AG300H</modelName>\n"
#elif HAVE_WZRG450
				   "<friendlyName>Buffalo WZR-HP-G450H</friendlyName>\n"
				   "<modelName>Buffalo WZR-HP-G450H</modelName>\n"
#else
				   "<friendlyName>%s:%s</friendlyName>\n"
				   "<modelName>%s</modelName>\n"
#endif
				   "<modelNumber>V30</modelNumber>\n"
				   "<serialNumber>0000001</serialNumber>\n"
				   "<modelURL>http://www.buffalo.jp</modelURL>\n"
#elif HAVE_ANTAIRA
				   "<manufacturer>Antaira</manufacturer>\n"
				   "<manufacturerURL>http://antaira.com</manufacturerURL>\n"
				   "<modelDescription>Gateway</modelDescription>\n"
				   "<friendlyName>Industrial Router</friendlyName>\n"
				   "<modelName>Industrial Router</modelName>\n"
				   "<modelNumber>V30</modelNumber>\n"
				   "<serialNumber>0000001</serialNumber>\n"
				   "<modelURL>http://www.antaira.com</modelURL>\n"
#else
				   "<manufacturer>DD-WRT</manufacturer>\n"
				   "<manufacturerURL>http://www.dd-wrt.com</manufacturerURL>\n"
				   "<modelDescription>Gateway</modelDescription>\n"
				   "<friendlyName>%s:%s</friendlyName>\n"
				   "<modelName>%s</modelName>\n"
				   "<modelNumber>V30</modelNumber>\n"
				   "<serialNumber>0000001</serialNumber>\n"
				   "<modelURL>http://www.dd-wrt.com</modelURL>\n"
#endif
				   "<UDN>uuid:eb9ab5b2-981c-4401-a20e-b7bcde359dbb</UDN>\n"
				   "<serviceList>\n"
				   "<service>\n"
				   "<serviceType>urn:schemas-upnp-org:service:Layer3Forwarding:1</serviceType>\n"
				   "<serviceId>urn:upnp-org:serviceId:L3Forwarding1</serviceId>\n"
				   "<SCPDURL>/x_layer3forwarding.xml</SCPDURL>\n"
				   "<controlURL>/control?Layer3Forwarding</controlURL>\n"
				   "<eventSubURL>/event?Layer3Forwarding</eventSubURL>\n"
				   "</service>\n"
				   "</serviceList>\n"
				   "<deviceList>\n"
				   "<device>\n"
				   "<deviceType>urn:schemas-upnp-org:device:WANDevice:1</deviceType>\n"
				   "<friendlyName>WANDevice</friendlyName>\n"
#ifdef HAVE_BUFFALO
				   "<manufacturer>Buffalo Inc.</manufacturer>\n"
				   "<manufacturerURL>http://www.buffalo.jp</manufacturerURL>\n"
				   "<modelDescription>Gateway</modelDescription>\n"
				   "<modelName>router</modelName>\n"
				   "<modelURL>http://www.buffalo.jp</modelURL>\n"
#elif HAVE_ANTAIRA
				   "<manufacturer>Antaira</manufacturer>\n"
				   "<manufacturerURL>http://www.antaira.com</manufacturerURL>\n"
				   "<modelDescription>Gateway</modelDescription>\n"
				   "<modelName>Industrial Router</modelName>\n"
				   "<modelURL>http://www.antaira.com</modelURL>\n"
#else
				   "<manufacturer>DD-WRT</manufacturer>\n"
				   "<manufacturerURL>http://www.dd-wrt.com</manufacturerURL>\n"
				   "<modelDescription>Gateway</modelDescription>\n"
				   "<modelName>router</modelName>\n"
				   "<modelURL>http://www.dd-wrt.com</modelURL>\n"
#endif
				   "<UDN>uuid:e1f05c9d-3034-4e4c-af82-17cdfbdcc077</UDN>\n"
				   "<serviceList>\n"
				   "<service>\n"
				   "<serviceType>urn:schemas-upnp-org:service:WANCommonInterfaceConfig:1</serviceType>\n"
				   "<serviceId>urn:upnp-org:serviceId:WANCommonIFC1</serviceId>\n"
				   "<SCPDURL>/x_wancommoninterfaceconfig.xml</SCPDURL>\n"
				   "<controlURL>/control?WANCommonInterfaceConfig</controlURL>\n"
				   "<eventSubURL>/event?WANCommonInterfaceConfig</eventSubURL>\n"
				   "</service>\n"
				   "</serviceList>\n"
				   "<deviceList>\n"
				   "<device>\n"
				   "<deviceType>urn:schemas-upnp-org:device:WANConnectionDevice:1</deviceType>\n"
				   "<friendlyName>WAN Connection Device</friendlyName>\n"
#ifdef HAVE_BUFFALO
				   "<manufacturer>Buffalo Inc.</manufacturer>\n"
				   "<manufacturerURL>http://www.buffalo.jp</manufacturerURL>\n"
				   "<modelDescription>Gateway</modelDescription>\n"
				   "<modelName>router</modelName>\n"
				   "<modelURL>http://www.buffalo.jp</modelURL>\n"
#elif HAVE_ANTAIRA
				   "<manufacturer>Antaira</manufacturer>\n"
				   "<manufacturerURL>http://www.antaira.com</manufacturerURL>\n"
				   "<modelDescription>Gateway</modelDescription>\n"
				   "<modelName>Industrial Router</modelName>\n"
				   "<modelURL>http://www.antaira.com</modelURL>\n"
#else
				   "<manufacturer>DD-WRT</manufacturer>\n"
				   "<manufacturerURL>http://www.dd-wrt.com</manufacturerURL>\n"
				   "<modelDescription>Gateway</modelDescription>\n"
				   "<modelName>router</modelName>\n"
				   "<modelURL>http://www.dd-wrt.com</modelURL>\n"
#endif
				   "<UDN>uuid:1995cf2d-d4b1-4fdb-bf84-8e59d2066198</UDN>\n"
				   "<serviceList>\n"
				   "<service>\n"
				   "<serviceType>urn:schemas-upnp-org:service:WANIPConnection:1</serviceType>\n"
				   "<serviceId>urn:upnp-org:serviceId:WANIPConn1</serviceId>\n"
				   "<SCPDURL>/x_wanipconnection.xml</SCPDURL>\n"
				   "<controlURL>/control?WANIPConnection</controlURL>\n"
				   "<eventSubURL>/event?WANIPConnection</eventSubURL>\n"
				   "</service>\n"
				   "</serviceList>\n"
				   "</device>\n"
				   "</deviceList>\n"
				   "</device>\n"
				   "<device>\n"
				   "<deviceType>urn:schemas-upnp-org:device:LANDevice:1</deviceType>\n"
				   "<friendlyName>LANDevice</friendlyName>\n"
#ifdef HAVE_ANTAIRA
				   "<manufacturer>Antaira</manufacturer>\n"
				   "<manufacturerURL>http://www.antaira.com</manufacturerURL>\n"
				   "<modelDescription>Gateway</modelDescription>\n"
				   "<modelName>Industrial Router</modelName>\n"
				   "<modelURL>http://www.antaira.com</modelURL>\n"
#else
				   "<manufacturer>DD-WRT</manufacturer>\n"
				   "<manufacturerURL>https://www.dd-wrt.com</manufacturerURL>\n"
				   "<modelDescription>Gateway</modelDescription>\n"
				   "<modelName>router</modelName>\n"
				   "<modelURL>https://www.dd-wrt.com</modelURL>\n"
#endif
				   "<UDN>uuid:f9e2cf4a-99e3-429e-b2dd-57e3c03ae597</UDN>\n"
				   "<serviceList>\n"
				   "<service>\n"
				   "<serviceType>urn:schemas-upnp-org:service:LANHostConfigManagement:1</serviceType>\n"
				   "<serviceId>urn:upnp-org:serviceId:LANHostCfg1</serviceId>\n"
				   "<SCPDURL>/x_lanhostconfigmanagement.xml</SCPDURL>\n"
				   "<controlURL>/control?LANHostConfigManagement</controlURL>\n"
				   "<eventSubURL>/event?LANHostConfigManagement</eventSubURL>\n"
				   "</service>\n"
				   "</serviceList>\n"
				   "</device>\n"
				   "</deviceList>\n"
				   "<presentationURL>http://255.255.255.255</presentationURL>\n"
				   "</device>\n"
				   "</root>\n"
				   "\n";

char xml_InternetGatewayDevice_real[sizeof(xml_InternetGatewayDevice) + 128];
