/*
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: layer3.c,v 1.1.1.7 2005/03/07 07:31:12 kanki Exp $
 */

#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"

static int Layer3Forwarding_Init(PService, service_state_t);
static int Layer3_GetVar(struct Service *psvc, int varindex);
static int SetDefaultConnectionService(UFILE *, PService psvc, PAction ac, pvar_entry_t args, int nargs);

static PDevice find_dev_by_udn(const char *udn);
static PService find_svc_by_type(PDevice pdev, char *type);
static PService find_svc_by_name(PDevice pdev, const char *name);

#define GetDefaultConnectionService		DefaultAction

static VarTemplate StateVariables[] = { 
    { "DefaultConnectionService", "", VAR_EVENTED|VAR_STRING }, 
    { 0 } 
};

#define VAR_DefaultConnectionService	0


static Action _GetDefaultConnectionService = { 
    "GetDefaultConnectionService", GetDefaultConnectionService,
        (Param []) {
            {"NewDefaultConnectionService", VAR_DefaultConnectionService, VAR_OUT},
            { 0 }
        }
};

static Action _SetDefaultConnectionService = { 
    "SetDefaultConnectionService", SetDefaultConnectionService,
        (Param []) {
            {"NewDefaultConnectionService", VAR_DefaultConnectionService, VAR_IN},
            { 0 }
        }
};

static PAction Actions[] = {
    &_GetDefaultConnectionService,
    &_SetDefaultConnectionService,
    NULL
};


ServiceTemplate Template_Layer3Forwarding = {
    "Layer3Forwarding:1",
    Layer3Forwarding_Init, 
    Layer3_GetVar,
    NULL,   /* SVCXML */
    ARRAYSIZE(StateVariables)-1, StateVariables,
    Actions
};

struct Layer3Forwarding {
    struct Service *default_svc;
};


static int Layer3Forwarding_Init(PService psvc, service_state_t state)
{
    struct Layer3Forwarding *pdata;

    switch (state) {
    case SERVICE_CREATE:
	pdata = (struct Layer3Forwarding *) malloc(sizeof(struct Layer3Forwarding));
	if (pdata) {
	    memset(pdata, 0, sizeof(struct Layer3Forwarding));
	    pdata->default_svc = NULL;
	    psvc->opaque = (void *) pdata;
	}
	break;
    case SERVICE_DESTROY:
	pdata = (struct Layer3Forwarding *) psvc->opaque;
	free(pdata);
	break;
    } /* end switch */

    return 0;
}


int Layer3_GetVar(struct Service *psvc, int varindex)
{
    struct StateVar *var;
    struct Layer3Forwarding *data = psvc->opaque;

    var = &(psvc->vars[varindex]);

    switch (varindex) {
    case VAR_DefaultConnectionService:
	if (data->default_svc != NULL) {
	    sprintf(var->value, "urn:schemas-upnp-org:service:%s:1,%s", data->default_svc->template->name, 
		    data->default_svc->device->udn);
	} else {
	    strcpy(var->value, "");
	}
	break;
    } /* end-switch */

    return TRUE;
}

static int SetDefaultConnectionService(UFILE *uclient, PService psvc, 
					PAction ac, pvar_entry_t args, int nargs)
{
    int success = TRUE;
    struct Layer3Forwarding *data = psvc->opaque;
    char *type, *udn;
    struct Service *csvc;
    struct Device *pdev;

    udn = ac->params[0].value;
    type = strsep(&udn, ",");
    
    if (udn == NULL || (pdev = find_dev_by_udn(udn)) == NULL) {
	soap_error(uclient, SOAP_INVALIDDEVICEUUID);
	success = FALSE;
    } else if (type == NULL || (csvc = find_svc_by_type(pdev, type)) == NULL) {
	soap_error(uclient, SOAP_INVALIDSERVICEID);
	success = FALSE;
    } else {
	data->default_svc = csvc;
	mark_changed(psvc, VAR_DefaultConnectionService);
    }

    return success;
}

static PDevice find_dev_by_udn(const char *udn)
{
    PDevice pdev = NULL;
    
    forall_devices(pdev) {
	if (strcmp(pdev->udn, udn) == 0) {
	    break;
	}
    }
    
    return pdev;
}


static PService find_svc_by_type(PDevice pdev, char *type)
{
    char *name = NULL, *p;
    PService psvc = NULL;

    p = rindex(type, ':');
    if (p != 0) {
	*p = '\0';
	p = rindex(type, ':');
	if (p != 0)
	    name = p+1;
    }

    if (name) 
	psvc = find_svc_by_name(pdev, name);

    return psvc;
}

static PService find_svc_by_name(PDevice pdev, const char *name)
{
    PService psvc;

    forall_services(pdev, psvc) {
	if (strcmp(psvc->template->name, name) == 0) {
	    break;
	}
    }

    return psvc;
}
