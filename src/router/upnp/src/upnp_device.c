/*
 * Broadcom UPnP module device specific functions implementation
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: upnp_device.c,v 1.13 2008/08/25 08:28:33 Exp $
 */

#include <upnp.h>
#include <md5.h>

int
upnp_device_attach(UPNP_CONTEXT *context, UPNP_DEVICE *device)
{
	UPNP_INTERFACE	*ifp = context->focus_ifp;
	UPNP_DEVCHAIN	*chain;

	upnp_syslog(LOG_INFO, "%s: attach %s", ifp->ifname, device->root_device_xml);

	/* Check if the device has already attached? */
	for (chain = ifp->device_chain;
	     chain;
	     chain = chain->next) {
		/* Attached, do nothing */
		if (chain->device == device)
			return 0;
	}

	/* Allocate a new one */
	chain = (UPNP_DEVCHAIN *)malloc(sizeof(*chain));
	if (chain == 0)
		return -1;

	chain->ifp = ifp;
	chain->device = device;

	/* Prepend this chain */
	chain->next = ifp->device_chain;
	ifp->device_chain = chain;

	ifp->focus_devchain = chain;

	/* Remove this chain, if open error */
	if ((*device->open)(context) != 0) {
		ifp->device_chain = chain->next;
		ifp->focus_devchain = chain->next;

		free(chain);
		return -1;
	}

	/* Initialize gena event variable */
	gena_init(context);

	/* Send byby here */
	ssdp_byebye(context);

	upnp_sleep(1);

	/* Send alive here */
	ssdp_alive(context);

	return 0;
}

void
upnp_device_detach(UPNP_CONTEXT *context, UPNP_DEVICE *device)
{
	UPNP_INTERFACE	*ifp = context->focus_ifp;
	UPNP_DEVCHAIN	*chain, *prev;

	upnp_syslog(LOG_INFO, "%s: detach %s", ifp->ifname, device->root_device_xml);

	/* Locate the device chain */
	for (prev = 0, chain = ifp->device_chain;
	     chain;
	     prev = chain, chain = chain->next) {
		if (chain->device == device)
			break;
	}
	if (chain == 0)
		return;

	ifp->focus_devchain = chain;

	/* Do device specific stop function */
	(*device->close)(context);

	/* Clear event variables and subscribers */
	gena_shutdown(context);

	/* Send byebye */
	ssdp_byebye(context);

	/* detach it */
	if (prev == 0)
		ifp->device_chain = chain->next;
	else
		prev->next = chain->next;

	/* Free this chain */
	free(chain);
	return;
}

/* Synchonize advertise table uuid */
static int
sync_advertise_uuid(UPNP_DEVICE *device, char *new_uuid, char *name)
{
	UPNP_ADVERTISE *advertise;

	for (advertise = device->advertise_table; advertise->name; advertise++) {
		if (strncmp(name, advertise->name, strlen(advertise->name)) == 0) {
			strcpy(advertise->uuid, new_uuid);
			break;
		}
	}

	return 0;
}

/* Generate the UUID for a UPnPdevice */
void
upnp_gen_uuid(char *uuid, char *deviceType)
{
	unsigned char new_uuid[16];
	unsigned char mac[6];

	MD5_CTX mdContext;

	upnp_osl_primary_lanmac((char *)mac);

	/* Generate hash */
	MD5Init(&mdContext);
	MD5Update(&mdContext, mac, sizeof(mac));
	MD5Update(&mdContext, (unsigned char *)deviceType, strlen(deviceType));
	MD5Final(new_uuid, &mdContext);

	sprintf(uuid,
		"%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
		new_uuid[0],  new_uuid[1],  new_uuid[2],  new_uuid[3],
		new_uuid[4],  new_uuid[5],  new_uuid[6],  new_uuid[7],
		new_uuid[8],  new_uuid[9],  new_uuid[10], new_uuid[11],
		new_uuid[12], new_uuid[13], new_uuid[14], new_uuid[15]);

	uuid[36] = '\0';
	return;
}

/* Give a unique uuid to this rootdevice */
int
upnp_device_renew_uuid(UPNP_CONTEXT *context, UPNP_DEVICE *device)
{
	char *s, *p;
	char deviceType[256];
	char serviceType[256];
	char new_uuid[64];

	UPNP_DESCRIPTION *desc;

	/* Find root device */
	for (desc = device->description_table; desc; desc++) {
		if (strcmp(device->root_device_xml, desc->name+1) == 0)
			break;
	}
	if (desc == 0) {
		upnp_syslog(LOG_ERR, "Root device can not find it's xml.");
		return -1;
	}

	/*
	 * Update UUID of the root device xml
	 * for each AP/router. By using the MAC address
	 * and the value between <deviceType>...</deviceType>, we can
	 * generate a unique UUID.
	 */
	for (p = desc->xml; *p != 0; p++) {
		/* Search for <deviceType> */
		if (strncmp(p, DEVICE_BTAG, strlen(DEVICE_BTAG)) == 0) {

			p += strlen(DEVICE_BTAG);

			/* Find the balanced </deviceType> */
			s = strstr(p, DEVICE_ETAG);
			if (s == 0 || (s-p) > sizeof(deviceType)-1) {
				upnp_syslog(LOG_ERR,
					"Parse format:<deviceType>...</deviceType> error.\n");
				return -1;
			}

			/* Save deviceType */
			memcpy(deviceType, p, s-p);
			deviceType[s-p] = '\0';
			p = s + strlen(DEVICE_ETAG);
		}

		/*
		 * For example,
		 * <deviceType>urn:schemas-upnp-org:device:InternetGatewayDevice:1</deviceType>
		 * the value between <UDN>...</UDN> is the UUID we have to replace.
		 * The UUID format is such as, uuid:eb9ab5b2-981c-4401-a20e-b7bcde359dbb.
		 * By generate the same UUID size, we don't have to do memory ajustment.
		 */
		if (strncmp(p, UDN_BTAG, strlen(UDN_BTAG)) == 0) {

			p += strlen(UDN_BTAG);

			/* Find the balanced </UDN> */
			s = strstr(p, UDN_ETAG);
			if (s == 0 || (s-p) > sizeof(deviceType)-1) {
				upnp_syslog(LOG_ERR, "Parse format:<UDN>...</UDN> error.\n");
				return -1;
			}

			p += 5; /* "uuid:" is 5 char */
			if ((s-p) != 36) {
				upnp_syslog(LOG_ERR, "UUID length is not 36.\n");
				return -1;
			}

			/* Replace uuid */
			upnp_gen_uuid(new_uuid, deviceType);

			memcpy(p, new_uuid, 36);  /* uuid is 36 characters. */
			p = p + 36;				  /* 46 = 10 + 36 */

			/*
			 * Afert the UUID of device in description XML is changed,
			 * we have to sync this new UUID to the advertisement table.
			 */
			sync_advertise_uuid(device, new_uuid, deviceType);
		}

		/*
		 * The SSDP broadcasts not only the device information but
		 * also all the services of this device, which shares the
		 * UUID of that device.
		 * As a result, we have to find out all the <serviceType>'s
		 * of this <deviceType>, and sync the device UUID to the
		 * advertisement table
		 */
		if (strncmp(p, SERVICE_BTAG, strlen(SERVICE_BTAG)) == 0) {

			p += strlen(SERVICE_BTAG);

			/* Find the balanced </serviceType> */
			s = strstr(p, SERVICE_ETAG);
			if (s == 0 || (s-p) > sizeof(serviceType)-1) {
				upnp_syslog(LOG_ERR,
					"Parse format:<serviceType>...</serviceType> error.\n");
				return -1;
			}

			/* Save serviceType */
			strncpy(serviceType, p, s-p);
			serviceType[s-p] = '\0';
			p = s + strlen(SERVICE_ETAG);

			/*
			 * Found a <serviceType>...</serviceType>.
			 * For example,
			 * <serviceType>urn:schemas-upnp-org:service:Layer3Forwarding:1
			 * </serviceType>
			 * is the service of
			 * <deviceType>urn:schemas-upnp-org:device:InternetGatewayDevice:1
			 * </deviceType>.
			 * We have to search the
			 * "urn:schemas-upnp-org:service:Layer3Forwarding:1"
			 * in the advertisement table, and replace its UUID with the UUID of
			 * "urn:schemas-upnp-org:device:InternetGatewayDevice:1".
			 */
			sync_advertise_uuid(device, new_uuid, serviceType);
		}
	}

	return 0;
}
