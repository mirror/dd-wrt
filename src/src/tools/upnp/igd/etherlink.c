/*
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: etherlink.c,v 1.1.1.7 2005/03/07 07:31:12 kanki Exp $
 */

#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"

extern void soap_response(int, const char *, const char *,  pvar_entry_t, int);
extern int DefaultAction(int, PService, PAction ac, pvar_entry_t, int);

#define GetEthernetLinkStatus DefaultAction

char *EthernetLinkStatus_allowedValueList[] = {  "Up", "Down", "Unavailable", 0 };

static VarTemplate StateVariables[] = { 
    { "EthernetLinkStatus", "", VAR_EVENTED|VAR_STRING|VAR_LIST,  (allowedValue) { EthernetLinkStatus_allowedValueList } }, 
    { 0 } 
};

#define VAR_EthernetLinkStatus	0

static Action _GetEthernetLinkStatus = { 
    "GetEthernetLinkStatus", GetEthernetLinkStatus,
        {
            {"NewEthernetLinkStatus", VAR_EthernetLinkStatus, VAR_OUT},
            { 0 }
        }
};

static PAction Actions[] = {
    &_GetEthernetLinkStatus,
    NULL
};

ServiceTemplate Template_WANEthernetLinkConfig = {
    "WANEthernetLinkConfig:1",
    NULL,
    NULL,
    NULL,   /* SVCXML */
    ARRAYSIZE(StateVariables)-1, StateVariables,
    Actions
};





