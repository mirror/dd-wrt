/*
 * Broadcom UPnP module main loop
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp.c,v 1.17 2008/06/20 05:25:31 Exp $
 */

#include <upnp.h>

/*
 * Variables
 */
int upnp_flag;
UPNP_CONTEXT upnp_context;

/* Shutdown all the UPnP interfaces */
void upnp_shutdown(UPNP_CONTEXT *context)
{
	UPNP_INTERFACE *ifp;

	while ((ifp = context->iflist) != 0) {
		context->focus_ifp = ifp;

		/* Unhook device database */
		while (ifp->device_chain) {
			ifp->focus_devchain = ifp->device_chain;

			upnp_device_detach(context, ifp->device_chain->device);
		}

		/* shutdown interface protocol */
		upnp_http_shutdown(context);

		ssdp_del_multi(context);

		upnp_request_shutdown(context);

		/* Drop this link and free */
		context->iflist = ifp->next;

		free(ifp);
	}

	/* Shutdown the mutlicast receive socket */
	ssdp_shutdown(context);

	upnp_syslog(LOG_INFO, "UPnP daemon stopped");
}

/*
 * Get interface IP addresses and netmasks from interface names
 */
static int get_if_ipaddr(UPNP_INTERFACE *ifp)
{
	char mac[6];

	if (oslib_ifaddr(ifp->ifname, &ifp->ipaddr) != 0)
		return -1;

	if (oslib_netmask(ifp->ifname, &ifp->netmask) != 0)
		return -1;

	if ((oslib_hwaddr(ifp->ifname, mac) != 0) || memcmp(mac, "\0\0\0\0\0\0", 6) == 0)
		return -1;

	return 0;
}

/* Do UPnP interface initialization */
UPNP_INTERFACE *upnp_ifinit(UPNP_CONTEXT *context, char *idxname)
{
	UPNP_INTERFACE *ifp;
	char *name;

	ifp = (UPNP_INTERFACE *)malloc(sizeof(*ifp));
	if (ifp == 0)
		return 0;

	memset(ifp, 0, sizeof(*ifp));

	/* Setup context */
	strtok_r(idxname, "=", &name);

	strlcpy(ifp->ifname, name, sizeof(ifp->ifname));
	ifp->if_instance = atoi(idxname);

	ifp->http_sock = -1;
	ifp->req_sock = -1;

	if (get_if_ipaddr(ifp) != 0) {
		free(ifp);
		return 0;
	}

	/* Do prepend */
	ifp->next = context->iflist;
	context->iflist = ifp;

	return ifp;
}

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

/* UPnP module initialization */
int upnp_init(UPNP_CONTEXT *context)
{
	UPNP_INTERFACE *ifp = 0;
	UPNP_DEVICE *device;
	int i;

	char ifname_list[256];

	char *name, *p, *next;

	sprintf(xml_InternetGatewayDevice_real, xml_InternetGatewayDevice, nvram_safe_get("DD_BOARD"),
		nvram_safe_get("router_name"), nvram_safe_get("DD_BOARD"));
	/* Clear flag */
	upnp_flag = 0;

	/* Clean up */
	memset(context, 0, sizeof(*context));

	upnp_osl_read_config(&context->config);
	oslib_ifname_list(ifname_list);

	/* Do context common initialization */
	context->adv_seconds = time(0);
	context->gena_last_check = time(0);
	context->ssdp_sock = -1;

	for (i = 0; (device = upnp_device_table[i]) != 0; i++) {
		(*device->common_init)(context);

		/*
		 * Give a unique uuid to this router,
		 * create same uuid when restart
		 */
		upnp_device_renew_uuid(context, device);
	}

	/* Create the mutlicast receive socket for all interfaces */
	if (ssdp_init(context) != 0)
		goto error_out;

	for (name = ifname_list, p = name; name && name[0]; name = next, p = 0) {
		strtok_r(p, " ", &next);

		ifp = upnp_ifinit(context, name);
		if (ifp == 0)
			continue;

		context->focus_ifp = ifp;

		/* Perform per interface protocol initialization */
		if (upnp_http_init(context) != 0) {
			upnp_syslog(LOG_ERR, "upnp_http_init::%s init error!", ifp->ifname);
			goto error_out;
		}

		if (ssdp_add_multi(context) == -1) {
			upnp_syslog(LOG_ERR, "ssdp_init::%s error!", ifp->ifname);
			goto error_out;
		}

		if (upnp_request_init(context) != 0) {
			upnp_syslog(LOG_ERR, "upnp_request_init::init error!");
			goto error_out;
		}

		/*
		 * Hook device table to each interface.
		 * The init function of each device
		 * intialize the event variables, and send SSDP ALIVE to each
		 * interface
		 */
		for (i = 0; (device = upnp_device_table[i]) != 0; i++) {
			if (device->attach_mode == DEVICE_ATTACH_ALWAYS)
				upnp_device_attach(context, device);
		}
	}

	/* No interface found, return directly */
	if (context->iflist == 0) {
		upnp_syslog(LOG_ERR, "No UPnP interface specified, bye!");
		goto error_out;
	}

	upnp_syslog(LOG_INFO, "UPnP daemon is ready to run");
	return 0;

error_out:
	upnp_flag = UPNP_FLAG_SHUTDOWN;
	return -1;
}

/* Time out handler of SSDP, GEAN and all the devices */
void upnp_timeout(UPNP_CONTEXT *context)
{
	UPNP_INTERFACE *ifp;
	UPNP_DEVCHAIN *chain;

	time_t now = time(0);
	int update_ssdp = ((u_long)(now - context->adv_seconds) > context->config.adv_time);
	int update_gena = ((u_long)(now - context->gena_last_check) >= GENA_TIMEOUT);

	/* Add device timer here */
	for (ifp = context->iflist; ifp; ifp = ifp->next) {
		/* Set the focus inteface for further reference */
		context->focus_ifp = ifp;

		/* loop for each device to check timeout */
		for (chain = ifp->device_chain; chain; chain = chain->next) {
			ifp->focus_devchain = chain;

			/* check for advertisement interval */
			if (update_ssdp)
				ssdp_timeout(context);

			/* check for subscription expirations every 30 seconds */
			if (update_gena)
				gena_timeout(context);

			/* Check device timeout */
			if (chain->device->timeout)
				(*chain->device->timeout)(context, now);
		}
	}

	/* Update ssdp timer and gena timer */
	if (update_ssdp)
		context->adv_seconds = now;

	if (update_gena)
		context->gena_last_check = now;

	return;
}

/*
 * Accept a connection and to handle the new http connection.
 */
static void upnp_http_accept(UPNP_CONTEXT *context)
{
	struct sockaddr_in addr;
	socklen_t addr_len;
	int ns;

	/* accept new upnp_http socket */
	addr_len = sizeof(struct sockaddr_in);
	ns = accept(context->focus_ifp->http_sock, (struct sockaddr *)&addr, &addr_len);
	if (ns == -1)
		return;

	upnp_http_process(context, ns);
	return;
}

/* Dispatch UPnP incoming messages. */
int upnp_dispatch(UPNP_CONTEXT *context)
{
	UPNP_INTERFACE *ifp;
	struct timeval tv = { 1, 0 }; /* timed out every second */
	int n;
	fd_set fds;

	FD_ZERO(&fds);

	/* Set select sockets */
	FD_SET(context->ssdp_sock, &fds);

	for (ifp = context->iflist; ifp; ifp = ifp->next) {
		FD_SET(ifp->http_sock, &fds);
		FD_SET(ifp->req_sock, &fds);
	}

	/* select sockets */
	n = select(FD_SETSIZE, &fds, (fd_set *)NULL, (fd_set *)NULL, &tv);
	if (n > 0) {
		/* process ssdp multicast packet */
		if (FD_ISSET(context->ssdp_sock, &fds)) {
			ssdp_process(context, context->ssdp_sock);
		}

		/* process upnp_http */
		for (ifp = context->iflist; ifp; ifp = ifp->next) {
			context->focus_ifp = ifp;

			/* process http */
			if (FD_ISSET(ifp->http_sock, &fds)) {
				upnp_http_accept(context);
			}

			/* process message */
			if (FD_ISSET(ifp->req_sock, &fds)) {
				upnp_request_handler(context);
			}
		}
	}

	upnp_timeout(context);

	return 0;
}

/* 
 * UPnP portable main loop.
 * It initializes the UPnP protocol and event variables.
 * And loop handler the UPnP incoming requests.
 */
void upnp_mainloop()
{
	UPNP_CONTEXT *context = &upnp_context;

	/* initialize upnp */
	upnp_init(context);

	/* main loop */
	while (1) {
		switch (upnp_flag) {
		case UPNP_FLAG_SHUTDOWN:
			upnp_shutdown(context);

			upnp_syslog(LOG_INFO, "UPnP shutdown!");
			return;

		case UPNP_FLAG_RESTART:
			upnp_shutdown(context);
			upnp_init(context);

			upnp_syslog(LOG_INFO, "UPnP restart!");
			break;

		case 0:
		default:
			upnp_dispatch(context);
			break;
		}
	}

	return;
}

void upnp_stop_handler()
{
	upnp_flag = UPNP_FLAG_SHUTDOWN;
}

void upnp_restart_handler()
{
	upnp_flag = UPNP_FLAG_RESTART;
}
