/*
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: igd.c,v 1.4 2005/03/07 08:35:32 kanki Exp $
 */

#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "bcmnvram.h"

#include <signal.h>

extern DeviceTemplate LANDeviceTemplate;
extern DeviceTemplate WANDeviceTemplate;

extern PDevice init_device(PDevice parent, PDeviceTemplate, ...);

void sigusr1_handler(int i);

/* these should be set by OS-specific initialization code. */
char g_wandevs[20];
char g_landevs[20];

Error IGDErrors[] = {
    { SOAP_DISCONNECTINPROGRESS, "DisconnectInProgress" },
    { SOAP_INVALIDCONNECTIONTYPE, "InvalidConnectionType" },
    { SOAP_CONNECTIONALREADYTERMNATED, "ConnectionAlreadyTerminated" },
    { SOAP_CONNECTIONNOTCONFIGURED, "ConnectionNotConfigured" },
    { SOAP_SPECIFIEDARRAYINDEXINVALID, "SpecifiedArrayIndexInvalid" },
    { SOAP_NOSUCHENTRYINARRAY, "NoSuchEntryInArray" },
    { SOAP_CONFLICTINMAPPINGENTRY, "ConflictInMappingEntry" },
    { SOAP_ONLYPERMANENTLEASESSUPPORTED, "OnlyPermanentLeasesSupported" },
    { 0, NULL }
};

int IGDevice_Init(PDevice igdev, device_state_t state, va_list ap)
{
    char *wan_ifname = &g_wandevs[0];
    char *lan_ifname = &g_landevs[0];
    PDevice subdev;

    switch (state) {
    case DEVICE_CREATE:
	soap_register_errors(IGDErrors);

	igdev->friendlyname = "Residential Gateway Device";

#if INCLUDE_LANDEVICE
	/* Include this code if you want a LAN device in your IGD.  If
	   you do, make sure that the LANHOSTCONFIG service is also included
	   in igd_desc.c */
	UPNP_TRACE(("Calling init_device for LANDevice\n"));
	subdev = init_device(igdev, &LANDeviceTemplate, lan_ifname);
	subdev->next = igdev->subdevs;
	igdev->subdevs = subdev;
#endif
	
	/* for each wan interface, create a WANDevice */
	UPNP_TRACE(("Calling init_device for WANDevice\n"));
	subdev = init_device(igdev, &WANDeviceTemplate, wan_ifname);
	subdev->next = igdev->subdevs;
	igdev->subdevs = subdev;

	/* install a signal handler for SIGUSR1 */
	signal(SIGUSR1, sigusr1_handler);

	break;

    case DEVICE_DESTROY:
	break;
    }

    return TRUE;
}


int WANDevice_Init(PDevice pdev, device_state_t state, va_list ap)
{
    PWANDevicePrivateData pdata;
    char *ifname = NULL;

    switch (state) {
    case DEVICE_CREATE:
	ifname = va_arg( ap, char *);
	if (ifname) {
	    pdata = (PWANDevicePrivateData) malloc(sizeof(WANDevicePrivateData));
	    if (pdata) {
		strcpy(pdata->ifname, nvram_safe_get("wan_iface"));
		pdev->opaque = (void *) pdata;
	    }
	}
	break;

    case DEVICE_DESTROY:
	free(pdev->opaque);
	pdev->opaque = NULL;
	break;
    }

    return TRUE;
}


int LANDevice_Init(PDevice pdev, device_state_t state, va_list ap)
{
    PLANDevicePrivateData pdata;
    char *ifname = NULL;

    switch (state) {
    case DEVICE_CREATE:
	ifname = va_arg( ap, char *);
	if (ifname) {
	    pdata = (PLANDevicePrivateData) malloc(sizeof(LANDevicePrivateData));
	    if (pdata) {
		strcpy(pdata->ifname, ifname);
		pdev->opaque = (void *) pdata;
	    }
	}
	break;

    case DEVICE_DESTROY:
	free(pdev->opaque);
	pdev->opaque = NULL;
	break;
    }

    return TRUE;
}

int igd_config_generation = 0;

void bump_generation()
{
    if (++igd_config_generation > 1000)
	igd_config_generation = 0;
}

void sigusr1_handler(int i)
{
    extern void reinit_lan_interface(struct iface *pif);
    extern struct iface *global_lans;
    struct iface *pif;

    printf("upnp reinitialize.\n");
    bump_generation();

    for (pif = global_lans; pif; pif = pif->next) 
	reinit_lan_interface(pif);

    /* re-install a signal handler for SIGUSR1 */
    signal(SIGUSR1, sigusr1_handler);
}


