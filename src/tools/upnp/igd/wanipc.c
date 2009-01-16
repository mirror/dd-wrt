/*
 * Copyright 2005, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: wanipc.c,v 1.13 2005/03/07 08:35:32 kanki Exp $
 */

#include <sys/sysinfo.h>
#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "wanipc.h"
#include "netconf.h"

#include "bcmnvram.h"
#include "nvparse.h"
#include "mapmgr.h"
#include <shutils.h>


/*
  Code to support the WAN IPConnection service.
*/

extern int GetGenericPortMappingEntry(UFILE *, PService , PAction, pvar_entry_t , int );
extern int GetSpecificPortMappingEntry(UFILE *, PService , PAction, pvar_entry_t , int);
extern int AddPortMapping(UFILE *, PService, PAction, pvar_entry_t, int);
extern int DeletePortMapping(UFILE *, PService, PAction, pvar_entry_t, int);
static int RequestConnection(UFILE *, PService , PAction, pvar_entry_t , int);
static int ForceTermination(UFILE *, PService, PAction, pvar_entry_t , int);
static int SetConnectionType(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs);

extern int igd_config_generation;
extern char * safe_snprintf(char *str, int *len, const char *fmt, ...);

static int WANIPConnectionInit(struct Service *psvc, service_state_t state);
static int WANIPConnection_GetVar(struct Service *psvc, int varindex);
static void WANIPConnection_Update(timer_t t, PService psvc);

bool IsValidIPConfig(const struct in_addr address,
		     const struct in_addr mask,
		     const struct in_addr gateway);
bool IsContiguousSubnet(const struct in_addr mask);
bool IsValidIPAddress(const struct in_addr ip);
bool set_auto_dns(char *AutoDNSEnable, char *ExternalDNSServers);

#define GetConnectionTypeInfo		DefaultAction
#define GetStatusInfo			DefaultAction
#define GetNATRSIPStatus		DefaultAction
#define GetExternalIPAddress		DefaultAction
#define GetIdleDisconnectTime		DefaultAction
#define GetWarnDisconnectDelay		DefaultAction
#define GetAutoDisconnectTime		DefaultAction

static char *PossibleConnectionTypes_allowedValueList[] = { "Unconfigured", "IP_Routed", "IP_Bridged", NULL };
static char *ConnectionStatus_allowedValueList[] = { "Unconfigured", "Connecting", "Authenticating", "Connected", "PendingDisconnect", "Disconnecting", "Disconnected", NULL };
static char *LastConnectionError_allowedValueList[] = { "ERROR_NONE", "ERROR_UNKNOWN", NULL };
static char *PortMappingProtocol_allowedValueList[] = { "TCP", "UDP", NULL };

static VarTemplate StateVariables[] = {
    { "ConnectionType", "IP_Routed", VAR_STRING|VAR_LIST,  (allowedValue) { PossibleConnectionTypes_allowedValueList }  },
    { "PossibleConnectionTypes", "", VAR_EVENTED|VAR_STRING|VAR_LIST,  (allowedValue) { PossibleConnectionTypes_allowedValueList } },
    { "ConnectionStatus", "", VAR_EVENTED|VAR_STRING|VAR_LIST,  (allowedValue) { ConnectionStatus_allowedValueList } },
    { "Uptime", "", VAR_ULONG },
    { "LastConnectionError", "", VAR_STRING|VAR_LIST,  (allowedValue) { LastConnectionError_allowedValueList } },
    { "RSIPAvailable", "1", VAR_BOOL },
    { "NATEnabled", "1", VAR_BOOL },
    { "ExternalIPAddress", "", VAR_EVENTED|VAR_STRING },
    { "PortMappingNumberOfEntries", "", VAR_EVENTED|VAR_USHORT },
    { "PortMappingEnabled", "", VAR_BOOL },
    { "PortMappingLeaseDuration", "", VAR_ULONG },
    { "RemoteHost", "", VAR_STRING },
    { "ExternalPort", "", VAR_USHORT },
    { "InternalPort", "", VAR_USHORT },
    { "PortMappingProtocol", "", VAR_STRING|VAR_LIST,  (allowedValue) { PortMappingProtocol_allowedValueList } },
    { "InternalClient", "", VAR_STRING },
    { "PortMappingDescription", "", VAR_STRING },
    { NULL }
};

static Action _SetConnectionType = {
    "SetConnectionType", SetConnectionType,
   (Param []) {
       {"NewConnectionType", VAR_ConnectionType, VAR_IN},
       { 0 }
    }
};

static Action _GetConnectionTypeInfo = {
    "GetConnectionTypeInfo", GetConnectionTypeInfo,
   (Param []) {
       {"NewConnectionType", VAR_ConnectionType, VAR_OUT},
       {"NewPossibleConnectionTypes", VAR_PossibleConnectionTypes, VAR_OUT},
       { 0 }
    }
};

static Action _RequestConnection = {
    "RequestConnection", RequestConnection,
   (Param []) {
       { 0 }
    }
};

static Action _ForceTermination = {
    "ForceTermination", ForceTermination,
   (Param []) {
       { 0 }
    }
};

static Action _GetStatusInfo = {
    "GetStatusInfo", GetStatusInfo,
   (Param []) {
       {"NewConnectionStatus", VAR_ConnectionStatus, VAR_OUT},
       {"NewLastConnectionError", VAR_LastConnectionError, VAR_OUT},
       {"NewUptime", VAR_Uptime, VAR_OUT},
       { 0 }
    }
};

static Action _GetNATRSIPStatus = {
    "GetNATRSIPStatus", GetNATRSIPStatus,
   (Param []) {
       {"NewRSIPAvailable", VAR_RSIPAvailable, VAR_OUT},
       {"NewNATEnabled", VAR_NATEnabled, VAR_OUT},
       { 0 }
    }
};

static Action _GetGenericPortMappingEntry = {
    "GetGenericPortMappingEntry", GetGenericPortMappingEntry,
   (Param []) {
       {"NewPortMappingIndex", VAR_PortMappingNumberOfEntries, VAR_IN},
       {"NewRemoteHost", VAR_RemoteHost, VAR_OUT},
       {"NewExternalPort", VAR_ExternalPort, VAR_OUT},
       {"NewProtocol", VAR_PortMappingProtocol, VAR_OUT},
       {"NewInternalPort", VAR_InternalPort, VAR_OUT},
       {"NewInternalClient", VAR_InternalClient, VAR_OUT},
       {"NewEnabled", VAR_PortMappingEnabled, VAR_OUT},
       {"NewPortMappingDescription", VAR_PortMappingDescription, VAR_OUT},
       {"NewLeaseDuration", VAR_PortMappingLeaseDuration, VAR_OUT},
       { 0 }
    }
};

static Action _GetSpecificPortMappingEntry = {
    "GetSpecificPortMappingEntry", GetSpecificPortMappingEntry,
   (Param []) {
       {"NewRemoteHost", VAR_RemoteHost, VAR_IN},
       {"NewExternalPort", VAR_ExternalPort, VAR_IN},
       {"NewProtocol", VAR_PortMappingProtocol, VAR_IN},
       {"NewInternalPort", VAR_InternalPort, VAR_OUT},
       {"NewInternalClient", VAR_InternalClient, VAR_OUT},
       {"NewEnabled", VAR_PortMappingEnabled, VAR_OUT},
       {"NewPortMappingDescription", VAR_PortMappingDescription, VAR_OUT},
       {"NewLeaseDuration", VAR_PortMappingLeaseDuration, VAR_OUT},
       { 0 }
    }
};

static Action _AddPortMapping = {
    "AddPortMapping", AddPortMapping,
   (Param []) {
       {"NewRemoteHost", VAR_RemoteHost, VAR_IN},
       {"NewExternalPort", VAR_ExternalPort, VAR_IN},
       {"NewProtocol", VAR_PortMappingProtocol, VAR_IN},
       {"NewInternalPort", VAR_InternalPort, VAR_IN},
       {"NewInternalClient", VAR_InternalClient, VAR_IN},
       {"NewEnabled", VAR_PortMappingEnabled, VAR_IN},
       {"NewPortMappingDescription", VAR_PortMappingDescription, VAR_IN},
       {"NewLeaseDuration", VAR_PortMappingLeaseDuration, VAR_IN},
       { 0 }
    }
};

static Action _DeletePortMapping = {
    "DeletePortMapping", DeletePortMapping,
   (Param []) {
       {"NewRemoteHost", VAR_RemoteHost, VAR_IN},
       {"NewExternalPort", VAR_ExternalPort, VAR_IN},
       {"NewProtocol", VAR_PortMappingProtocol, VAR_IN},
       { 0 }
    }
};

static Action _GetExternalIPAddress = {
    "GetExternalIPAddress", GetExternalIPAddress,
   (Param []) {
       {"NewExternalIPAddress", VAR_ExternalIPAddress, VAR_OUT},
       { 0 }
    }
};



static PAction Actions[] = {
    &_SetConnectionType,
    &_GetConnectionTypeInfo,
    &_RequestConnection,
    &_ForceTermination,
    &_GetStatusInfo,
    &_GetNATRSIPStatus,
    &_GetGenericPortMappingEntry,
    &_GetSpecificPortMappingEntry,
    &_AddPortMapping,
    &_DeletePortMapping,
    &_GetExternalIPAddress,
    NULL
};


ServiceTemplate Template_WANIPConnection = {
    "WANIPConnection:1",
    WANIPConnectionInit,	/* service initialization */
    WANIPConnection_GetVar,	/* state variable handler */
    NULL,			/* xml generator */
    ARRAYSIZE(StateVariables)-1, StateVariables,
    Actions, 0,
    "urn:upnp-org:serviceId:WANIPConn"
};

extern void enable_wan();


static int WANIPConnectionInit(struct Service *psvc, service_state_t state)
{
    struct  itimerspec  timer;
    PWANIPConnectionData pdata;
    PWANDevicePrivateData pdevdata;	
    struct sysinfo info;

    switch (state) {
    case SERVICE_CREATE:
	pdevdata = (PWANDevicePrivateData) psvc->device->parent->opaque;
	pdata = (PWANIPConnectionData) malloc(sizeof(WANIPConnectionData));
	if (pdata) {
	    memset(pdata, 0, sizeof(WANIPConnectionData));
	    pdata->connection_status = IP_CONNECTING;
	    //pdata->connected_time = time(NULL);
	    sysinfo(&info);
	    pdata->connected_time = info.uptime;	// by honor, use uptime rather than system time
	    pdata->igd_generation = igd_config_generation;

	    mapmgr_update();
	    pdata->nportmappings = mapmgr_port_map_count();
	    
	    osl_ifaddr(pdevdata->ifname, &pdata->external_ipaddr);
	    psvc->opaque = (void *) pdata;
	    
	    /* once a second we want to update the statistics variables in the WANIPConnection service */
	    memset(&timer, 0, sizeof(timer));
	    timer.it_interval.tv_sec = 2;
	    timer.it_value.tv_sec = 2;
	    pdata->eventhandle = enqueue_event(&timer, (event_callback_t)WANIPConnection_Update, (void *) psvc );
	}
	break;

    case SERVICE_DESTROY:
	pdata = (PWANIPConnectionData) psvc->opaque;
	
	dd_timer_delete(pdata->eventhandle);
	free(pdata);
	break;
    } /* end switch */

    return 0;
}

int need_restart = 0;

void igd_restart_helper()
{
    if (need_restart) {
	need_restart = 0;
	osl_sys_restart();
    }
}

void igd_restart(int secs) 
{
    extern void delayed_call(uint seconds, voidfp_t f);

    need_restart = 1;
    delayed_call(secs, igd_restart_helper);
}

/* Get the NVRAM variable corresponding to the primary WAN interface. */
char *igd_pri_wan_var(char *prefix, int len, char *var)
{
/*
	int unit;
	char tmp[100];

	for (unit = 0; unit < MAX_NVPARSE; unit ++) {
		snprintf(tmp, sizeof(tmp), "wan%d_primary", unit);
		if (nvram_match(tmp, "1"))
			break;
	}
	if (unit == MAX_NVPARSE)
		unit = 0;
	snprintf(prefix, len, "wan%d_%s", unit, var);
*/
	snprintf(prefix, len, "wan_%s", var);

	return (prefix);
}


/* this is run once a second from WANIPConnection_Update. */
static bool update_connection_status(PService psvc)
{
    PWANIPConnectionData pdata = (PWANIPConnectionData) psvc->opaque;
    PWANDevicePrivateData pdevdata = (PWANDevicePrivateData) psvc->device->parent->opaque;
    ip_conn_t old_status = pdata->connection_status;
    bool wan_connected;
    struct sysinfo info;

    wan_connected = osl_wan_isup(pdevdata->ifname);
    switch (pdata->connection_status) {
    case IP_CONNECTING: 
	{
	    if (wan_connected) {
		pdata->connection_status = IP_CONNECTED;	// for cdrouter_upnp_203/204/220
		//pdata->connected_time = time(NULL);
		sysinfo(&info);
		pdata->connected_time = (time_t)info.uptime;	// by honor, use uptime rather than system time 
	    } else if (pdata->connection_timeout-- <= 0) {
		pdata->connection_status = IP_DISCONNECTED;
	    }
	}
	break;

    case IP_CONNECTED: 
	{
	    if (!wan_connected) {
		pdata->connection_status = IP_DISCONNECTED;
	    } 
	}
	break;
    case IP_DISCONNECTING: 
	{
	    if (wan_connected) {
		pdata->connection_status = IP_CONNECTING;
	    } else {
		pdata->connection_status = IP_DISCONNECTED;
	    }
	}
	break;
	
    case IP_DISCONNECTED: 
	{
	    if (wan_connected) {
		pdata->connection_status = IP_CONNECTED;	// for cdrouter_upnp_203/204/220
	    }
	}
	break;
    default:
	UPNP_ERROR(("%s:%d Unexpected connection_status %d\n", __FILE__, __LINE__, pdata->connection_status));
    }

    if (old_status != pdata->connection_status)
	mark_changed(psvc, VAR_ConnectionStatus);

    return (old_status != pdata->connection_status);
}

/* this is run once a second from WANIPConnection_Update. */
static bool update_external_address(PService psvc)
{
    PWANIPConnectionData pdata = (PWANIPConnectionData) psvc->opaque;
    PWANDevicePrivateData pdevdata = (PWANDevicePrivateData) psvc->device->parent->opaque;
    struct in_addr ipaddr = {0} ;

    if (osl_wan_isup(pdevdata->ifname)) {
        strcpy(pdevdata->ifname, nvram_safe_get("wan_iface"));//add for cdrouter v3.3 upnp module bugs in pptp and pppoe mode 
	osl_ifaddr(pdevdata->ifname, &ipaddr);
    }
    if (pdata->external_ipaddr.s_addr != ipaddr.s_addr) {
	pdata->external_ipaddr = ipaddr;
	mark_changed(psvc, VAR_ExternalIPAddress);
    }

    return (pdata->external_ipaddr.s_addr != ipaddr.s_addr);
}


/* this is run once a second. */
static void WANIPConnection_Update(timer_t t, PService psvc)
{
    PWANIPConnectionData pdata = (PWANIPConnectionData) psvc->opaque;

    if (igd_config_generation != pdata->igd_generation) {
	pdata->igd_generation = igd_config_generation;
	mapmgr_update();
	if (pdata->nportmappings != mapmgr_port_map_count()) {
	    pdata->nportmappings = mapmgr_port_map_count();
	    mark_changed(psvc, VAR_PortMappingNumberOfEntries);
	}
    }

    update_external_address(psvc);
    update_connection_status(psvc);

    if ((psvc->flags & VAR_CHANGED) == VAR_CHANGED) {
	update_all_subscriptions(psvc);
    }
}


static int WANIPConnection_GetVar(struct Service *psvc, int varindex)
{
    PWANIPConnectionData data = psvc->opaque;
    struct StateVar *var;
    time_t now;
    char **p, *cur;
    int len;
    struct sysinfo info;

    var = &(psvc->vars[varindex]);

    switch (varindex) {
    case VAR_PossibleConnectionTypes:
	cur = var->value;
	len = sizeof(var->value);
	for (p = PossibleConnectionTypes_allowedValueList; *p; p++) {
	    if (cur != var->value)
		cur = safe_snprintf(cur, &len, ",");
	    cur = safe_snprintf(cur, &len, "%s", *p);
	}
	break;

    case VAR_Uptime:
	if (data->connection_status != IP_CONNECTED) {
	    sprintf(var->value, "%d", 0);
	} else {
	    sysinfo(&info);
	    sprintf(var->value, "%d", (u_int32) info.uptime - (u_int32) data->connected_time);	// by honor, use uptime rather than system time
	}
	break;

    case VAR_ConnectionStatus:
	sprintf(var->value, "%s", ConnectionStatus_allowedValueList[data->connection_status]);
	break;

    case VAR_PortMappingNumberOfEntries:
	sprintf(var->value, "%d", data->nportmappings);
	break;

    case VAR_ExternalIPAddress: {
	    const unsigned char *bytep;

	    bytep = (const unsigned char *) &(data->external_ipaddr.s_addr);
	    sprintf(var->value, "%d.%d.%d.%d", bytep[0], bytep[1], bytep[2], bytep[3]);
        }
	break;
    } /* end-switch */

    return TRUE;
}


static int ForceTermination(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
    uint success = TRUE; /* assume no error will occur */
    PWANIPConnectionData pdata = (PWANIPConnectionData) psvc->opaque;
    PWANDevicePrivateData pdevdata = (PWANDevicePrivateData) psvc->device->parent->opaque;
    char wanproto[100];

    if(nvram_invmatch("upnp_internet_dis","1")) {
	cprintf("Cann't disable internet from UPnP. (upnp_disable_dis=1)\n");
	return FALSE;
    }

    /* Our ConnectionType is always IP_Routed, so I don't need to check that here. */
    if (pdata->connection_status == IP_DISCONNECTED) {
	soap_error(uclient, SOAP_CONNECTIONALREADYTERMNATED);
	success = FALSE;
    } else if (pdata->connection_status != IP_CONNECTED 
	       && pdata->connection_status != IP_CONNECTING) {
	soap_error(uclient, SOAP_DISCONNECTINPROGRESS);
	success = FALSE;
    } else { 
	igd_pri_wan_var(wanproto, sizeof(wanproto), "proto");
	/* Save the wan_proto into NVRAM to restore when the igd is enabled */
	nvram_set("upnp_wan_proto", nvram_safe_get(wanproto));
	if(nvram_match("upnp_wan_proto", "dhcp"))
		osl_igd_disable("");
	//nvram_set(wanproto, "disabled");
	//nvram_commit();
	igd_restart(1);

    }
    
    return success;
}

static int RequestConnection(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
    uint success = TRUE; /* assume no error will occur */
    PWANIPConnectionData pdata = psvc->opaque;
    char wanproto[100], *str;

    if (pdata->connection_status == IP_UNCONFIGURED) {
	soap_error(uclient, SOAP_CONNECTIONNOTCONFIGURED);
	success = FALSE;
    } else if (pdata->connection_status == IP_DISCONNECTED) {
	igd_pri_wan_var(wanproto, sizeof(wanproto), "proto");


	/* Save the wan_proto into NVRAM to restore when the igd is enabled */
	if ((str = nvram_get("upnp_wan_proto")) == NULL)
	    nvram_set(wanproto, "dhcp");	/* pick dhcp as defualt */
	else {
	    if(strcmp(str, "")) {
	    	nvram_set(wanproto, str);
		nvram_set("upnp_wan_proto", "");
		//nvram_commit();
	    }
	}
	igd_restart(1);
	pdata->connection_status = IP_CONNECTING;
	pdata->connection_timeout = CONNECTION_TIMEOUT;
	mark_changed(psvc, VAR_ConnectionStatus);
    }
    mark_changed(psvc, VAR_ConnectionStatus);

    return success;
}


/*
  IsValidIPAddress()  -- Reject the IP addresses that are reserved for
  other things
*/
bool IsValidIPAddress(const struct in_addr ip)
{
   if( (ip.s_addr               ==          0) || // 0.0.0.0 is of course not valid:)
       (ip.s_addr & ntohl(0xFF000000)) ==  ntohl(0x7F000000) || // 127.x.x.x, loop-back
       (ip.s_addr & ntohl(0xFF000000)) >=  ntohl(0xE0000000)  ) // 224-239.x.x.x == multicasting
   {                                              // 240-254.x.x.x == reserved, experimental
                                                  // 255.x.x.x     == broadcast
       UPNP_TRACE(("Is valid ip Address, false, 0x%x\n", (uint) ip.s_addr));
       return FALSE;
   }
   UPNP_TRACE(("Is valid ip Address, true, 0x%x\n", (uint) ip.s_addr));
   return TRUE;
}

   
/*
  IsContiguousSubnet() - verify that a subnet mask no holes in it.
*/
bool IsContiguousSubnet(const struct in_addr mask)
{
    // Convert aaa.bbb.ccc.ddd into ddd.ccc.bbb.aaa
    bool bContinuous;
    uint32 i, dwContiguousMask;
    uint32 hmask = ntohl(mask.s_addr);

    // Find out where the first '1' is in binary going right to left
    dwContiguousMask = 0;
    for (i = 0; i < sizeof(mask.s_addr)*8; i++) {
        dwContiguousMask |= (1<<i);

        if (dwContiguousMask & hmask) {
            break;
	}
    }

    // At this point, dwContiguousMask is 000...0111...  If we inverse it,
    // we get a mask that can be or'd with dwMask to fill in all of
    // the holes.
    dwContiguousMask = hmask | ~dwContiguousMask;

    bContinuous = (hmask == dwContiguousMask);
    UPNP_TRACE(("Is contiguous subnet, %s\n", (bContinuous ? "true" : "false")));
    return bContinuous;

}


///////////////////////////////////////////////////////////////////////////////
// IsValidIPConfig()  -- Determine if the given IPAddress, mask, and gateway
//                       make sense.
///////////////////////////////////////////////////////////////////////////////
bool IsValidIPConfig(const struct in_addr address,
		     const struct in_addr mask,
		     const struct in_addr gateway)
{
    bool bIsValid = FALSE;

    do {
	// First make sure that the address, and the gateway don't have 
	// reserved addresses.
	if (!IsValidIPAddress(address) || !IsValidIPAddress(gateway)) {
	    UPNP_TRACE(("Is valid IP config, address, or gateway was invalid (reserved) IP\n"));
	    break;
	}

	if (address.s_addr == gateway.s_addr) {
	    UPNP_TRACE(("Is valid IP config, mapping IP cannot be the same as gateway IP\n"));
	    break;
	}

	if (!IsContiguousSubnet(mask)) {
	    UPNP_TRACE(("Is valid IP Config, mask was not contiguous\n"));
	    break;
	}

	if ((address.s_addr & mask.s_addr) != (gateway.s_addr & mask.s_addr)) {
	    UPNP_TRACE(("ip and gw not on the same subnet, not valid ip config\n"));
	    break;
	}

	// Now check to make sure that the ip address is not at 0, or 255.
	if (address.s_addr == (address.s_addr & mask.s_addr)) {
	    UPNP_TRACE(("You can't have an IP address at zero(s) relative to subnet mask\n"));
	    break;
	}

	if ( (~mask.s_addr) + (address.s_addr & mask.s_addr) == address.s_addr) {
	    UPNP_TRACE(("You can't have an IP address at 255\n"));
	    break;
	}
	bIsValid = TRUE;
    } while (0);

   return bIsValid;
}


static int SetConnectionType(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
    snprintf(psvc->vars[VAR_ConnectionType].value, sizeof(psvc->vars[VAR_ConnectionType].value), ac->params[0].value);

    return TRUE;
}
