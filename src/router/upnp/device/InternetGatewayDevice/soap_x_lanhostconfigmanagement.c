/*
 * Broadcom UPnP module, soap_x_lanhostconfigmanagement.c
 *
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: soap_x_lanhostconfigmanagement.c,v 1.4 2007/11/23 09:51:55 Exp $
 */
#include <upnp.h>
#include <InternetGatewayDevice.h>

/*
 * WARNNING: PLEASE IMPLEMENT YOUR CODES AFTER 
 *          "<< USER CODE START >>"
 * AND DON'T REMOVE TAG :
 *          "<< AUTO GENERATED FUNCTION: "
 *          ">> AUTO GENERATED FUNCTION"
 *          "<< USER CODE START >>"
 */

/* << AUTO GENERATED FUNCTION: statevar_DHCPServerConfigurable() */
static int statevar_DHCPServerConfigurable(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar,
					   UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_BOOL(value))

	if (!value)
		return SOAP_DEVICE_INTERNAL_ERROR;
	/* << USER CODE START >> */
	int ifid = context->focus_ifp->if_instance;
	char name[32];

	/* Get dhcp server nv name */
	if (ifid == 0) {
		return nvram_match("lan_proto", "dhcp") ? 1 : 0;
	} else
		sprintf(name, "dhcp%d_enable", ifid);

	/* Get value */
	UPNP_BOOL(value) = atoi(nvram_safe_get(name));
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_DHCPRelay() */
static int statevar_DHCPRelay(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar, UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_BOOL(value))
	if (!value)
		return SOAP_DEVICE_INTERNAL_ERROR;

	/* << USER CODE START >> */
	/* We don't support DHCP relay */
	UPNP_BOOL(value) = 0;
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_SubnetMask() */
static int statevar_SubnetMask(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar, UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_STR(value))
	if (!value)
		return SOAP_DEVICE_INTERNAL_ERROR;

	/* << USER CODE START >> */
	int ifid = context->focus_ifp->if_instance;
	char name[32];

	/* Get dhcp server nv name */
	if (ifid == 0)
		strcpy(name, "lan_netmask");
	else
		sprintf(name, "lan%d_netmask", ifid);

	strlcpy(UPNP_STR(value), nvram_safe_get(name), sizeof(UPNP_STR(value)));
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_IPRouters() */
static int statevar_IPRouters(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar, UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_STR(value))
	if (!value)
		return SOAP_DEVICE_INTERNAL_ERROR;

	/* << USER CODE START >> */
	int ifid = context->focus_ifp->if_instance;
	char name[32];

	/* Get dhcp server nv name */
	if (ifid == 0)
		strcpy(name, "lan_ipaddr");
	else
		sprintf(name, "lan%d_ipaddr", ifid);

	strlcpy(UPNP_STR(value), nvram_safe_get(name), sizeof(UPNP_STR(value)));
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_DNSServers() */
static int statevar_DNSServers(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar, UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_STR(value))
	if (!value)
		return SOAP_DEVICE_INTERNAL_ERROR;

	/* << USER CODE START >> */
	//	int	ifid = context->focus_ifp->if_instance;
	char name[32];
	char *dnslist;
	char buf[128];

	char *dns, *p, *next;

	/* Get DNS list */
	if (nvram_match("dnsmasq_enable", "1")) {
		strcpy(name, "lan_ipaddr");
	} else {
		if (!nvram_match("wan_get_dns", "")) {
			strcpy(name, "wan_get_dns");
		} else {
			strcpy(name, "wan_dns");
		}
	}

	dnslist = nvram_safe_get(name);

	/* make token and rebuild the string with comma */
	strlcpy(buf, dnslist, sizeof(buf));

	UPNP_STR(value)[0] = 0;
	for (p = buf;; p = 0) {
		dns = strtok_r(p, " ", &next);
		if (dns == 0)
			break;

		/*
		 * Set UPNP_STR(value) to the following example,
		 *
		 * '10.144.2.1,10.144.10.3,10.144.3.5'.
		 */

		if (UPNP_STR(value)[0] != 0)
			strcat(UPNP_STR(value), ",");

		strcat(UPNP_STR(value), dns);
	}

	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_DomainName() */
static int statevar_DomainName(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar, UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_STR(value))
	if (!value)
		return SOAP_DEVICE_INTERNAL_ERROR;

	/* << USER CODE START >> */
	strlcpy(UPNP_STR(value), nvram_safe_get("wan_domain"), sizeof(UPNP_STR(value)));
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_MinAddress() */
static int statevar_MinAddress(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar, UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_STR(value))
	if (!value)
		return SOAP_DEVICE_INTERNAL_ERROR;

	/* << USER CODE START >> */
	int ifid = context->focus_ifp->if_instance;
	char name[32];

	/* Get dhcp server nv name */
	if (ifid == 0)
		strcpy(name, "dhcp_start");
	else
		sprintf(name, "dhcp%d_start", ifid);

	strlcpy(UPNP_STR(value), nvram_safe_get(name), sizeof(UPNP_STR(value)));
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_MaxAddress() */
static int statevar_MaxAddress(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar, UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_STR(value))
	if (!value)
		return SOAP_DEVICE_INTERNAL_ERROR;

	/* << USER CODE START >> */
	int ifid = context->focus_ifp->if_instance;
	char name[32];

	/* Get dhcp server nv name */
	if (ifid == 0)
		strcpy(name, "dhcp_end");
	else
		sprintf(name, "dhcp%d_end", ifid);

	strlcpy(UPNP_STR(value), nvram_safe_get(name), sizeof(UPNP_STR(value)));
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: statevar_ReservedAddresses() */
static int statevar_ReservedAddresses(UPNP_CONTEXT *context, UPNP_SERVICE *service, UPNP_STATE_VAR *statevar, UPNP_VALUE *value)
{
	UPNP_USE_HINT(UPNP_STR(value))
	if (!value)
		return SOAP_DEVICE_INTERNAL_ERROR;

	/* << USER CODE START >> */
	strcpy(UPNP_STR(value), "");
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_SetDHCPServerConfigurable() */
static int action_SetDHCPServerConfigurable(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument,
					    OUT_ARGUMENT *out_argument)
{
	UPNP_IN_HINT(IN_ARGUMENT *in_NewDHCPServerConfigurable = UPNP_IN_ARG("NewDHCPServerConfigurable");)

	UPNP_USE_HINT(ARG_BOOL(in_NewDHCPServerConfigurable))

	/* << USER CODE START >> */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetDHCPServerConfigurable() */
static int action_GetDHCPServerConfigurable(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument,
					    OUT_ARGUMENT *out_argument)
{
	UPNP_OUT_HINT(OUT_ARGUMENT *out_NewDHCPServerConfigurable = UPNP_OUT_ARG("NewDHCPServerConfigurable");)

	UPNP_USE_HINT(ARG_BOOL(out_NewDHCPServerConfigurable))

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewDHCPServerConfigurable = UPNP_OUT_ARG("NewDHCPServerConfigurable");

	return statevar_DHCPServerConfigurable(context, service, out_NewDHCPServerConfigurable->statevar,
					       ARG_VALUE(out_NewDHCPServerConfigurable));
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_SetDHCPRelay() */
static int action_SetDHCPRelay(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument, OUT_ARGUMENT *out_argument)
{
	UPNP_IN_HINT(IN_ARGUMENT *in_NewDHCPRelay = UPNP_IN_ARG("NewDHCPRelay");)

	UPNP_USE_HINT(ARG_BOOL(in_NewDHCPRelay))

	/* << USER CODE START >> */
	/* Not support */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetDHCPRelay() */
static int action_GetDHCPRelay(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument, OUT_ARGUMENT *out_argument)
{
	UPNP_OUT_HINT(OUT_ARGUMENT *out_NewDHCPRelay = UPNP_OUT_ARG("NewDHCPRelay");)

	UPNP_USE_HINT(ARG_BOOL(out_NewDHCPRelay))

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewDHCPRelay = UPNP_OUT_ARG("NewDHCPRelay");
	if (!out_NewDHCPRelay)
		return SOAP_DEVICE_INTERNAL_ERROR;

	return statevar_DHCPRelay(context, service, out_NewDHCPRelay->statevar, ARG_VALUE(out_NewDHCPRelay));
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_SetSubnetMask() */
static int action_SetSubnetMask(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument, OUT_ARGUMENT *out_argument)
{
	UPNP_IN_HINT(IN_ARGUMENT *in_NewSubnetMask = UPNP_IN_ARG("NewSubnetMask");)

	UPNP_USE_HINT(ARG_STR(in_NewSubnetMask))

	/* << USER CODE START >> */
	/* Not implemented for security reason */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetSubnetMask() */
static int action_GetSubnetMask(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument, OUT_ARGUMENT *out_argument)
{
	UPNP_OUT_HINT(OUT_ARGUMENT *out_NewSubnetMask = UPNP_OUT_ARG("NewSubnetMask");)

	UPNP_USE_HINT(ARG_STR(out_NewSubnetMask))

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewSubnetMask = UPNP_OUT_ARG("NewSubnetMask");
	if (!out_NewSubnetMask)
		return SOAP_DEVICE_INTERNAL_ERROR;

	return statevar_SubnetMask(context, service, out_NewSubnetMask->statevar, ARG_VALUE(out_NewSubnetMask));
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_SetIPRouter() */
static int action_SetIPRouter(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument, OUT_ARGUMENT *out_argument)
{
	UPNP_IN_HINT(IN_ARGUMENT *in_NewIPRouters = UPNP_IN_ARG("NewIPRouters");)

	UPNP_USE_HINT(ARG_STR(in_NewIPRouters))

	/* << USER CODE START >> */
	/* Not implemented for security reason */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_DeleteIPRouter() */
static int action_DeleteIPRouter(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument, OUT_ARGUMENT *out_argument)
{
	UPNP_IN_HINT(IN_ARGUMENT *in_NewIPRouters = UPNP_IN_ARG("NewIPRouters");)

	UPNP_USE_HINT(ARG_STR(in_NewIPRouters))

	/* << USER CODE START >> */
	/* Not implemented for security reason */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetIPRoutersList() */
static int action_GetIPRoutersList(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument,
				   OUT_ARGUMENT *out_argument)
{
	UPNP_OUT_HINT(OUT_ARGUMENT *out_NewIPRouters = UPNP_OUT_ARG("NewIPRouters");)

	UPNP_USE_HINT(ARG_STR(out_NewIPRouters))

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewIPRouters = UPNP_OUT_ARG("NewIPRouters");
	if (!out_NewIPRouters)
		return SOAP_DEVICE_INTERNAL_ERROR;

	return statevar_IPRouters(context, service, out_NewIPRouters->statevar, ARG_VALUE(out_NewIPRouters));
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_SetDomainName() */
static int action_SetDomainName(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument, OUT_ARGUMENT *out_argument)
{
	UPNP_IN_HINT(IN_ARGUMENT *in_NewDomainName = UPNP_IN_ARG("in_NewDomainName");)

	UPNP_USE_HINT(ARG_STR(in_NewDomainName))

	/* << USER CODE START >> */
	/* Not implemented for security reason */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetDomainName() */
static int action_GetDomainName(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument, OUT_ARGUMENT *out_argument)
{
	UPNP_OUT_HINT(OUT_ARGUMENT *out_NewDomainName = UPNP_OUT_ARG("NewDomainName");)

	UPNP_USE_HINT(ARG_STR(out_NewDomainName))

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewDomainName = UPNP_OUT_ARG("NewDomainName");
	if (!out_NewDomainName)
		return SOAP_DEVICE_INTERNAL_ERROR;

	return statevar_DomainName(context, service, out_NewDomainName->statevar, ARG_VALUE(out_NewDomainName));
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_SetAddressRange() */
static int action_SetAddressRange(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument,
				  OUT_ARGUMENT *out_argument)
{
	UPNP_IN_HINT(IN_ARGUMENT *in_NewMinAddress = UPNP_IN_ARG("NewMinAddress");)
	UPNP_IN_HINT(IN_ARGUMENT *in_NewMaxAddress = UPNP_IN_ARG("NewMaxAddress");)

	UPNP_USE_HINT(ARG_STR(VALOF(in_NewMinAddress)))
	UPNP_USE_HINT(ARG_STR(VALOF(in_NewMaxAddress)))

	/* << USER CODE START >> */
	/* Not implemented for security reason */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetAddressRange() */
static int action_GetAddressRange(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument,
				  OUT_ARGUMENT *out_argument)
{
	UPNP_OUT_HINT(OUT_ARGUMENT *out_NewMinAddress = UPNP_OUT_ARG("NewMinAddress");)
	UPNP_OUT_HINT(OUT_ARGUMENT *out_NewMaxAddress = UPNP_OUT_ARG("NewMaxAddress");)

	UPNP_USE_HINT(ARG_STR(out_NewMinAddress))
	UPNP_USE_HINT(ARG_STR(out_NewMaxAddress))

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewMinAddress = UPNP_OUT_ARG("NewMinAddress");
	OUT_ARGUMENT *out_NewMaxAddress = UPNP_OUT_ARG("NewMaxAddress");
	if (!out_NewMinAddress)
		return SOAP_DEVICE_INTERNAL_ERROR;
	if (!out_NewMaxAddress)
		return SOAP_DEVICE_INTERNAL_ERROR;

	statevar_MinAddress(context, service, out_NewMinAddress->statevar, ARG_VALUE(out_NewMinAddress));
	statevar_MaxAddress(context, service, out_NewMaxAddress->statevar, ARG_VALUE(out_NewMaxAddress));

	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_SetReservedAddress() */
static int action_SetReservedAddress(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument,
				     OUT_ARGUMENT *out_argument)
{
	UPNP_IN_HINT(IN_ARGUMENT *in_NewReservedAddresses = UPNP_IN_ARG("NewReservedAddresses");)

	UPNP_USE_HINT(ARG_STR(in_NewReservedAddresses))

	/* << USER CODE START >> */
	/* Not implemented for security reason */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_DeleteReservedAddress() */
static int action_DeleteReservedAddress(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument,
					OUT_ARGUMENT *out_argument)
{
	UPNP_IN_HINT(IN_ARGUMENT *in_NewReservedAddresses = UPNP_IN_ARG("NewReservedAddresses");)

	UPNP_USE_HINT(ARG_STR(in_NewReservedAddresses))

	/* << USER CODE START >> */
	/* Not implemented for security reason */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetReservedAddresses() */
static int action_GetReservedAddresses(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument,
				       OUT_ARGUMENT *out_argument)
{
	UPNP_OUT_HINT(OUT_ARGUMENT *out_NewReservedAddresses = UPNP_OUT_ARG("out_NewReservedAddresses");)

	UPNP_USE_HINT(ARG_STR(out_NewReservedAddresses))

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewReservedAddresses = UPNP_OUT_ARG("out_NewReservedAddresses");
	if (!out_NewReservedAddresses)
		return SOAP_DEVICE_INTERNAL_ERROR;

	return statevar_ReservedAddresses(context, service, out_NewReservedAddresses->statevar,
					  ARG_VALUE(out_NewReservedAddresses));
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_SetDNSServer() */
static int action_SetDNSServer(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument, OUT_ARGUMENT *out_argument)
{
	UPNP_IN_HINT(IN_ARGUMENT *in_NewDNSServers = UPNP_IN_ARG("NewDNSServers");)

	UPNP_USE_HINT(ARG_STR(in_NewDNSServers))

	/* << USER CODE START >> */
	/* Not implemented for security reason */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_DeleteDNSServer() */
static int action_DeleteDNSServer(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument,
				  OUT_ARGUMENT *out_argument)
{
	UPNP_IN_HINT(IN_ARGUMENT *in_NewDNSServers = UPNP_IN_ARG("NewDNSServers");)

	UPNP_USE_HINT(ARG_STR(in_NewDNSServers))

	/* << USER CODE START >> */
	/* Not implemented for security reason */
	return OK;
}
/* >> AUTO GENERATED FUNCTION */

/* << AUTO GENERATED FUNCTION: action_GetDNSServers() */
static int action_GetDNSServers(UPNP_CONTEXT *context, UPNP_SERVICE *service, IN_ARGUMENT *in_argument, OUT_ARGUMENT *out_argument)
{
	UPNP_OUT_HINT(OUT_ARGUMENT *out_NewDNSServers = UPNP_OUT_ARG("NewDNSServers");)

	UPNP_USE_HINT(ARG_STR(out_NewDNSServers))

	/* << USER CODE START >> */
	OUT_ARGUMENT *out_NewDNSServers = UPNP_OUT_ARG("NewDNSServers");
	if (!out_NewDNSServers)
		return SOAP_DEVICE_INTERNAL_ERROR;

	return statevar_DNSServers(context, service, out_NewDNSServers->statevar, ARG_VALUE(out_NewDNSServers));
}
/* >> AUTO GENERATED FUNCTION */

/* << TABLE BEGIN */
/*
 * WARNNING: DON'T MODIFY THE FOLLOWING TABLES
 * AND DON'T REMOVE TAG :
 *          "<< TABLE BEGIN"
 *          ">> TABLE END"
 */

#define STATEVAR_DHCPRELAY 0
#define STATEVAR_DHCPSERVERCONFIGURABLE 1
#define STATEVAR_DNSSERVERS 2
#define STATEVAR_DOMAINNAME 3
#define STATEVAR_IPROUTERS 4
#define STATEVAR_MAXADDRESS 5
#define STATEVAR_MINADDRESS 6
#define STATEVAR_RESERVEDADDRESSES 7
#define STATEVAR_SUBNETMASK 8

/* State Variable Table */
UPNP_STATE_VAR statevar_x_lanhostconfigmanagement[] = { { 0, "DHCPRelay", UPNP_TYPE_BOOL, &statevar_DHCPRelay, 0 },
							{ 0, "DHCPServerConfigurable", UPNP_TYPE_BOOL,
							  &statevar_DHCPServerConfigurable, 0 },
							{ 0, "DNSServers", UPNP_TYPE_STR, &statevar_DNSServers, 0 },
							{ 0, "DomainName", UPNP_TYPE_STR, &statevar_DomainName, 0 },
							{ 0, "IPRouters", UPNP_TYPE_STR, &statevar_IPRouters, 0 },
							{ 0, "MaxAddress", UPNP_TYPE_STR, &statevar_MaxAddress, 0 },
							{ 0, "MinAddress", UPNP_TYPE_STR, &statevar_MinAddress, 0 },
							{ 0, "ReservedAddresses", UPNP_TYPE_STR, &statevar_ReservedAddresses, 0 },
							{ 0, "SubnetMask", UPNP_TYPE_STR, &statevar_SubnetMask, 0 },
							{ 0, 0, 0, 0, 0 } };

/* Action Table */
static ACTION_ARGUMENT arg_in_SetDHCPServerConfigurable[] = { { "NewDHCPServerConfigurable", UPNP_TYPE_BOOL,
								STATEVAR_DHCPSERVERCONFIGURABLE } };

static ACTION_ARGUMENT arg_out_GetDHCPServerConfigurable[] = { { "NewDHCPServerConfigurable", UPNP_TYPE_BOOL,
								 STATEVAR_DHCPSERVERCONFIGURABLE } };

static ACTION_ARGUMENT arg_in_SetDHCPRelay[] = { { "NewDHCPRelay", UPNP_TYPE_BOOL, STATEVAR_DHCPRELAY } };

static ACTION_ARGUMENT arg_out_GetDHCPRelay[] = { { "NewDHCPRelay", UPNP_TYPE_BOOL, STATEVAR_DHCPRELAY } };

static ACTION_ARGUMENT arg_in_SetSubnetMask[] = { { "NewSubnetMask", UPNP_TYPE_STR, STATEVAR_SUBNETMASK } };

static ACTION_ARGUMENT arg_out_GetSubnetMask[] = { { "NewSubnetMask", UPNP_TYPE_STR, STATEVAR_SUBNETMASK } };

static ACTION_ARGUMENT arg_in_SetIPRouter[] = { { "NewIPRouters", UPNP_TYPE_STR, STATEVAR_IPROUTERS } };

static ACTION_ARGUMENT arg_in_DeleteIPRouter[] = { { "NewIPRouters", UPNP_TYPE_STR, STATEVAR_IPROUTERS } };

static ACTION_ARGUMENT arg_out_GetIPRoutersList[] = { { "NewIPRouters", UPNP_TYPE_STR, STATEVAR_IPROUTERS } };

static ACTION_ARGUMENT arg_in_SetDomainName[] = { { "NewDomainName", UPNP_TYPE_STR, STATEVAR_DOMAINNAME } };

static ACTION_ARGUMENT arg_out_GetDomainName[] = { { "NewDomainName", UPNP_TYPE_STR, STATEVAR_DOMAINNAME } };

static ACTION_ARGUMENT arg_in_SetAddressRange[] = { { "NewMinAddress", UPNP_TYPE_STR, STATEVAR_MINADDRESS },
						    { "NewMaxAddress", UPNP_TYPE_STR, STATEVAR_MAXADDRESS } };

static ACTION_ARGUMENT arg_out_GetAddressRange[] = { { "NewMinAddress", UPNP_TYPE_STR, STATEVAR_MINADDRESS },
						     { "NewMaxAddress", UPNP_TYPE_STR, STATEVAR_MAXADDRESS } };

static ACTION_ARGUMENT arg_in_SetReservedAddress[] = { { "NewReservedAddresses", UPNP_TYPE_STR, STATEVAR_RESERVEDADDRESSES } };

static ACTION_ARGUMENT arg_in_DeleteReservedAddress[] = { { "NewReservedAddresses", UPNP_TYPE_STR, STATEVAR_RESERVEDADDRESSES } };

static ACTION_ARGUMENT arg_out_GetReservedAddresses[] = { { "NewReservedAddresses", UPNP_TYPE_STR, STATEVAR_RESERVEDADDRESSES } };

static ACTION_ARGUMENT arg_in_SetDNSServer[] = { { "NewDNSServers", UPNP_TYPE_STR, STATEVAR_DNSSERVERS } };

static ACTION_ARGUMENT arg_in_DeleteDNSServer[] = { { "NewDNSServers", UPNP_TYPE_STR, STATEVAR_DNSSERVERS } };

static ACTION_ARGUMENT arg_out_GetDNSServers[] = { { "NewDNSServers", UPNP_TYPE_STR, STATEVAR_DNSSERVERS } };

UPNP_ACTION action_x_lanhostconfigmanagement[] = {
	{ "DeleteDNSServer", 1, arg_in_DeleteDNSServer, 0, 0, &action_DeleteDNSServer },
	{ "DeleteIPRouter", 1, arg_in_DeleteIPRouter, 0, 0, &action_DeleteIPRouter },
	{ "DeleteReservedAddress", 1, arg_in_DeleteReservedAddress, 0, 0, &action_DeleteReservedAddress },
	{ "GetAddressRange", 0, 0, 2, arg_out_GetAddressRange, &action_GetAddressRange },
	{ "GetDHCPRelay", 0, 0, 1, arg_out_GetDHCPRelay, &action_GetDHCPRelay },
	{ "GetDHCPServerConfigurable", 0, 0, 1, arg_out_GetDHCPServerConfigurable, &action_GetDHCPServerConfigurable },
	{ "GetDNSServers", 0, 0, 1, arg_out_GetDNSServers, &action_GetDNSServers },
	{ "GetDomainName", 0, 0, 1, arg_out_GetDomainName, &action_GetDomainName },
	{ "GetIPRoutersList", 0, 0, 1, arg_out_GetIPRoutersList, &action_GetIPRoutersList },
	{ "GetReservedAddresses", 0, 0, 1, arg_out_GetReservedAddresses, &action_GetReservedAddresses },
	{ "GetSubnetMask", 0, 0, 1, arg_out_GetSubnetMask, &action_GetSubnetMask },
	{ "SetAddressRange", 2, arg_in_SetAddressRange, 0, 0, &action_SetAddressRange },
	{ "SetDHCPRelay", 1, arg_in_SetDHCPRelay, 0, 0, &action_SetDHCPRelay },
	{ "SetDHCPServerConfigurable", 1, arg_in_SetDHCPServerConfigurable, 0, 0, &action_SetDHCPServerConfigurable },
	{ "SetDNSServer", 1, arg_in_SetDNSServer, 0, 0, &action_SetDNSServer },
	{ "SetDomainName", 1, arg_in_SetDomainName, 0, 0, &action_SetDomainName },
	{ "SetIPRouter", 1, arg_in_SetIPRouter, 0, 0, &action_SetIPRouter },
	{ "SetReservedAddress", 1, arg_in_SetReservedAddress, 0, 0, &action_SetReservedAddress },
	{ "SetSubnetMask", 1, arg_in_SetSubnetMask, 0, 0, &action_SetSubnetMask },
	{ 0, 0, 0, 0, 0, 0 }
};
/* >> TABLE END */
