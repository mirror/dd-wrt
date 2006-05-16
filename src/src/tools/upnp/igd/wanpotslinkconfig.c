/*
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: wanpotslinkconfig.c,v 1.1.1.7 2005/03/07 07:31:12 kanki Exp $
 */

#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"

#define GetDataCompression		NotImplemented
#define SetISPInfo			NotImplemented
#define GetISPInfo			NotImplemented
#define GetDataProtocol			NotImplemented
#define GetCallRetryInfo		NotImplemented
#define GetPlusVTRCommandSupported	NotImplemented
#define SetCallRetryInfo		NotImplemented
#define GetDataModulationSupported	NotImplemented
#define GetFclass			NotImplemented

static VarTemplate StateVariables[] = { 
    { "PlusVTRCommandSupported", "", VAR_BOOL }, 
    { "DataModulationSupported", "", VAR_STRING }, 
    { "Fclass", "", VAR_STRING }, 
    { "NumberOfRetries", "", VAR_LONG }, 
    { "LinkType", "", VAR_STRING }, 
    { "DataCompression", "", VAR_STRING }, 
    { "ISPPhoneNumber", "", VAR_EVENTED|VAR_STRING }, 
    { "DataProtocol", "", VAR_STRING }, 
    { "ISPInfo", "", VAR_STRING }, 
    { "DelayBetweenRetries", "", VAR_LONG }, 
    { 0 } 
};

#define VAR_PlusVTRCommandSupported	0
#define VAR_DataModulationSupported	1
#define VAR_Fclass	2
#define VAR_NumberOfRetries	3
#define VAR_LinkType	4
#define VAR_DataCompression	5
#define VAR_ISPPhoneNumber	6
#define VAR_DataProtocol	7
#define VAR_ISPInfo	8
#define VAR_DelayBetweenRetries	9


static Action _GetDataCompression = { 
    "GetDataCompression", GetDataCompression,
        {
            {"NewDataCompression", VAR_DataCompression, VAR_OUT},
            { 0 }
        }
};

static Action _SetISPInfo = { 
    "SetISPInfo", SetISPInfo,
        {
            {"NewISPPhoneNumber", VAR_ISPPhoneNumber, VAR_IN},
            {"NewISPInfo", VAR_ISPInfo, VAR_IN},
            {"NewLinkType", VAR_LinkType, VAR_IN},
            { 0 }
        }
};

static Action _GetISPInfo = { 
    "GetISPInfo", GetISPInfo,
        {
            {"NewISPPhoneNumber", VAR_ISPPhoneNumber, VAR_OUT},
            {"NewISPInfo", VAR_ISPInfo, VAR_OUT},
            {"NewLinkType", VAR_LinkType, VAR_OUT},
            { 0 }
        }
};

static Action _GetDataProtocol = { 
    "GetDataProtocol", GetDataProtocol,
        {
            {"NewDataProtocol", VAR_DataProtocol, VAR_OUT},
            { 0 }
        }
};

static Action _GetCallRetryInfo = { 
    "GetCallRetryInfo", GetCallRetryInfo,
        {
            {"NewNumberOfRetries", VAR_NumberOfRetries, VAR_OUT},
            {"NewDelayBetweenRetries", VAR_DelayBetweenRetries, VAR_OUT},
            { 0 }
        }
};

static Action _GetPlusVTRCommandSupported = { 
    "GetPlusVTRCommandSupported", GetPlusVTRCommandSupported,
        {
            {"NewPlusVTRCommandSupported", VAR_PlusVTRCommandSupported, VAR_OUT},
            { 0 }
        }
};

static Action _SetCallRetryInfo = { 
    "SetCallRetryInfo", SetCallRetryInfo,
        {
            {"NewNumberOfRetries", VAR_NumberOfRetries, VAR_IN},
            {"NewDelayBetweenRetries", VAR_DelayBetweenRetries, VAR_IN},
            { 0 }
        }
};

static Action _GetDataModulationSupported = { 
    "GetDataModulationSupported", GetDataModulationSupported,
        {
            {"NewDataModulationSupported", VAR_DataModulationSupported, VAR_OUT},
            { 0 }
        }
};

static Action _GetFclass = { 
    "GetFclass", GetFclass,
        {
            {"NewFclass", VAR_Fclass, VAR_OUT},
            { 0 }
        }
};

static PAction Actions[] = {
    &_GetDataCompression,
    &_SetISPInfo,
    &_GetISPInfo,
    &_GetDataProtocol,
    &_GetCallRetryInfo,
    &_GetPlusVTRCommandSupported,
    &_SetCallRetryInfo,
    &_GetDataModulationSupported,
    &_GetFclass,
    NULL
};


ServiceTemplate Template_WANPOTSLinkConfig = {
    "WANPOTSLinkConfig:1",
    NULL,
    NULL,
    NULL,  /* SVCXML */
    ARRAYSIZE(StateVariables)-1, StateVariables,
    Actions
};



