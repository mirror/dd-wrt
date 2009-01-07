/* Copyright 2006, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: ipt.c,v 1.1.1.7.30.2 2006/04/12 01:40:34 honor Exp $
 */

#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "wanipc.h"
#include "netconf.h"
#include "bcmnvram.h"
#include "mapmgr.h"
#include <shutils.h>
/*@.@add define to fixed upnp port forward bug*/
#define compare_subnet(A,B,netmask) ( _compare_subnet(A,B,netmask))

void print_rule(const netconf_nat_t *nat_current);
void print_mapping(const mapping_t *m);

/* extern and forward declaration */
netconf_nat_t *parse_dnat(netconf_nat_t *entry, const char *Protocol, 
			  const char *RemoteHost, const char *ExternalStartPort, const char *ExternalEndPort,
			  const char *InternalClient, const char *InternalStartPort, const char *InternalEndPort);

static bool SameMatchInfo(const netconf_nat_t *e1, const netconf_nat_t *e2); 


void print_mapping(const mapping_t *m)
{
    printf("%s (%s)\n", m->desc, ((m->match.flags & NETCONF_DISABLED) ? "disabled" : "enabled"));
    print_rule((netconf_nat_t *)m);
}

void print_rule(const netconf_nat_t *nat_current)
{
    const unsigned char *bytep;

    printf("rule 0x%x ", (unsigned int) nat_current);
    switch (nat_current->match.ipproto) {
    case IPPROTO_TCP:
	printf("TCP  ");
	break;
    case IPPROTO_UDP:
	printf("UDP  ");
	break;
    default:
	printf("unknown <0x%x>  ", nat_current->match.ipproto);
    }
    if (nat_current->target == NETCONF_DNAT)
    {
	printf("DNAT\n");
    }else
    {
	printf("UNNOWN <0x%x>\n", nat_current->target);
    }

    if (strlen(nat_current->match.in.name) > 0) {
	printf("\tinput interface: %s\n", nat_current->match.in.name);
    }
    if (strlen(nat_current->match.out.name) > 0) {
	printf("\toutput interface: %s\n", nat_current->match.out.name);
    }
    
    printf("wan ports:\t%ld - ", 
	   (unsigned long)ntohs(nat_current->match.dst.ports[0]));
    printf("%ld\n", 
	   (unsigned long)ntohs(nat_current->match.dst.ports[1]));
    bytep = (const unsigned char *) &(nat_current->ipaddr.s_addr);
    printf("lan client: %d.%d.%d.%d\n", bytep[0], bytep[1], bytep[2], bytep[3]);
    printf("lan ports:\t%ld - ", 
	   (unsigned long)ntohs(nat_current->ports[0]) );
    printf("%ld\n", 
	   (unsigned long)ntohs(nat_current->ports[1]));
}

static bool SameInternalClient(netconf_nat_t *e1,   netconf_nat_t *e2)
{
    return (e1->ipaddr.s_addr == e2->ipaddr.s_addr);
}

/* Two mappings conflict if the ExternalPort, 
   PortMappingProtocol, and InternalClient are the same, 
   but RemoteHost is different. 
   pp 15, WANIPConection Sevice.
*/
static bool OverlappingRange(netconf_nat_t *e1, netconf_nat_t *e2)
{
    bool overlap = FALSE; /* assume no conflict */

    do {
	if (e1->ports[1] < e2->ports[0])
	    break;
	if (e1->ports[0] > e2->ports[1]) 
	    break;

	overlap = TRUE;
	printf("OverlappingRange.\n");
	printf("rule 1\n");
	print_rule(e1);
	printf("rule 2\n");
	print_rule(e2);
    } while(0);

    printf("%s\n", (overlap ? "OverlappingRange" : "not OverlappingRange"));
    return overlap;
}

static void DelPortMapping_Alerm(timer_t t, int i)
{
    cprintf("Delete port_forward%d due to expire (%d seconds)\n", i);

    mapmgr_delete_port_map(i);
    timer_delete(t);
}

int AddPortMapping( UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
/* {"NewRemoteHost", VAR_RemoteHost, VAR_IN},				*/
/* {"NewExternalPort", VAR_ExternalPort, VAR_IN},			*/
/* {"NewProtocol", VAR_PortMappingProtocol, VAR_IN},			*/
/* {"NewInternalPort", VAR_InternalPort, VAR_IN},			*/
/* {"NewInternalClient", VAR_InternalClient, VAR_IN},			*/
/* {"NewEnabled", VAR_PortMappingEnabled, VAR_IN},			*/
/* {"NewPortMappingDescription", VAR_PortMappingDescription, VAR_IN},	*/
/* {"NewLeaseDuration", VAR_PortMappingLeaseDuration, VAR_IN},		*/
{
    int i, parse_status, status = 0;
    char *LeaseDuration =  ac->params[7].value;
    char *Description = ac->params[6].value;
    bool bEnabled = (bool) atoi(ac->params[5].value);
    bool found_match;
    struct  itimerspec  timer;
    netconf_nat_t e;
    mapping_t mapping;
    struct in_addr lanipaddr, lanmask;
    inet_aton(nvram_safe_get("lan_netmask"),(void *)&lanmask); 
    inet_aton(nvram_safe_get("lan_ipaddr"),(void *)&lanipaddr); 

    memset(&e, 0 , sizeof(netconf_nat_t));
    memset(&mapping, 0 , sizeof(mapping_t));

    if(nvram_invmatch("upnp_config","1")) {
	cprintf("Cann't add rule from upnp\n");
	return SOAP_ACTION_FAILED;
    }

    do {
#if 0
	if (atoi(LeaseDuration) != 0) {
	    status = SOAP_ONLYPERMANENTLEASESSUPPORTED;
	    continue;
	} 
#endif
	parse_status = (int) parse_dnat(&e, 
					ac->params[2].value, /* NewProtocol */
					ac->params[0].value, /* NewRemoteHost */
					ac->params[1].value, NULL, /* NewExternalPort */
					ac->params[4].value, /* NewInternalClient */
					ac->params[3].value, NULL /* NewInternalPort */
	);
	if (!parse_status) {
	    status = SOAP_INVALID_ARGS;
	    continue;
	}

	/* check the IP Address of this mapping */
	if (!IsValidIPConfig(e.ipaddr, lanmask, lanipaddr)) {
		printf("This is \"not\" a valid mapping IP address\n");
		continue;
	}
	
	found_match = FALSE;
	for (i = 0; mapmgr_get_port_map(i, &mapping); i++) {
	    if (SameMatchInfo(&e, (netconf_nat_t*)&mapping)) {
		found_match = TRUE;
		break;
	    }
	}
	
	if (found_match) {
	    if (!SameInternalClient(&e, &mapping)) {
		// we already have that mapping, but a different target...error. 
		status = SOAP_CONFLICTINMAPPINGENTRY;
		continue;
	    } else {
		mapmgr_delete_port_map(i);
	    }
	} 
	
	/* Enabled/disable this mapping according to the "NewEnabled" parameter. */
	if (bEnabled) {
	    e.match.flags &= ~NETCONF_DISABLED;
	} else {
	    e.match.flags |= NETCONF_DISABLED;
	}
	
	strncpy(e.desc, Description, sizeof(e.desc));
		
	mapmgr_add_port_map(&e);
	//if (!mapmgr_add_port_map(&e)) {
	//	printf("here return soap failed\n");
	//	return SOAP_ACTION_FAILED;
	//}

	// For cdrouter_upnp_50
	if (atoi(LeaseDuration) != 0) {
	    memset(&timer, 0, sizeof(timer));
	    timer.it_interval.tv_sec = atoi(LeaseDuration);
	    timer.it_value.tv_sec = atoi(LeaseDuration);
	    enqueue_event(&timer, (event_callback_t)DelPortMapping_Alerm, (void *) i);
	}

    } while(0);

    if (status) 
	soap_error( uclient, status );

    return (status == 0);
}


int DeletePortMapping( UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
/*  {"NewRemoteHost", VAR_RemoteHost, VAR_IN},		*/
/*  {"NewExternalPort", VAR_ExternalPort, VAR_IN},	*/
/*  {"NewProtocol", VAR_PortMappingProtocol, VAR_IN},	*/
{
    int i, parse_status, status = 0;
    netconf_nat_t e;
    mapping_t mapping;

    if(nvram_invmatch("upnp_config","1")) {
	cprintf("Cann't del rule from upnp\n");
	return SOAP_ACTION_FAILED;
    }
    
    parse_status = (int) parse_dnat(&e, 
				    ac->params[2].value,	/* NewProtocol */
				    ac->params[0].value,	/* NewRemoteHost */
				    ac->params[1].value,	NULL, /* NewExternalPort */
				    NULL,		/* NewInternalClient */
				    NULL, NULL 		/* NewInternalPort */ );
    if (!parse_status) {
	status = SOAP_INVALID_ARGS;
    } else {
	status = SOAP_NOSUCHENTRYINARRAY;
	for (i = 0; mapmgr_get_port_map(i, &mapping); i++) {
	    if (SameMatchInfo(&e, (netconf_nat_t*)&mapping)) {
		mapmgr_delete_port_map(i);
		status = 0; /* SUCCESS! */
	    }
	}
    }	

    if (status) 
	soap_error( uclient, status );

    return (status == 0);
}



int GetGenericPortMappingEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
/* {"NewPortMappingIndex", VAR_PortMappingNumberOfEntries, VAR_IN}, 	*/
/* {"NewRemoteHost", VAR_RemoteHost, VAR_OUT}, 				*/
/* {"NewExternalPort", VAR_ExternalPort, VAR_OUT}, 			*/
/* {"NewProtocol", VAR_PortMappingProtocol, VAR_OUT}, 			*/
/* {"NewInternalPort", VAR_InternalPort, VAR_OUT}, 			*/
/* {"NewInternalClient", VAR_InternalClient, VAR_OUT}, 			*/
/* {"NewEnabled", VAR_PortMappingEnabled, VAR_OUT}, 			*/
/* {"NewPortMappingDescription", VAR_PortMappingDescription, VAR_OUT},	*/
/* {"NewLeaseDuration", VAR_PortMappingLeaseDuration, VAR_OUT}, 	*/
{
    int success = TRUE;
    char *PortMappingIndex = ac->params[0].value;
    mapping_t mapping;
    u_int32 i;
    const unsigned char *bytep;
    static char RemoteHost[16];
    static char ExternalPort[7];
    static char InternalClient[16];
    static char InternalPort[7];
    static char Enabled[2];
    static char Description[60];

    i = atoi(PortMappingIndex);
    if (mapmgr_get_port_map(i, &mapping)) {
	RemoteHost[0] = '\0';
	if (mapping.match.dst.ipaddr.s_addr) {
	    bytep = (const unsigned char *) &(mapping.match.dst.ipaddr);
	    snprintf(RemoteHost, sizeof(RemoteHost), 
		     "%d.%d.%d.%d", bytep[0], bytep[1], bytep[2], bytep[3]);
	}

	InternalClient[0] = '\0';
	if (mapping.ipaddr.s_addr) {
	    bytep = (const unsigned char *) &(mapping.ipaddr);
	    snprintf(InternalClient, sizeof(InternalClient), 
		     "%d.%d.%d.%d", bytep[0], bytep[1], bytep[2], bytep[3]);
	}

	snprintf(ExternalPort, sizeof(ExternalPort), "%d", ntohs(mapping.match.dst.ports[0]));
	snprintf(InternalPort, sizeof(InternalPort), "%d", ntohs(mapping.ports[0]));
	snprintf(Enabled, sizeof(Enabled), "%d", ((mapping.match.flags & NETCONF_DISABLED) ? 0 : 1));
	snprintf(Description, sizeof(Description), "%s", mapping.desc);

	ac->params[1].value = RemoteHost;
	ac->params[2].value = ExternalPort;
	ac->params[3].value = (mapping.match.ipproto == IPPROTO_TCP ? "TCP" :  "UDP" );
	ac->params[4].value = InternalPort;
	ac->params[5].value = InternalClient;
	ac->params[6].value = Enabled;
	ac->params[7].value = Description;
	ac->params[8].value = "0";
    } else {
	soap_error( uclient, SOAP_SPECIFIEDARRAYINDEXINVALID );
	success = FALSE;
    }

    return success;
}


int GetSpecificPortMappingEntry(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
/* {"NewRemoteHost", VAR_RemoteHost, VAR_IN},				*/
/* {"NewExternalPort", VAR_ExternalPort, VAR_IN},			*/
/* {"NewProtocol", VAR_PortMappingProtocol, VAR_IN},			*/
/* {"NewInternalPort", VAR_InternalPort, VAR_OUT},			*/
/* {"NewInternalClient", VAR_InternalClient, VAR_OUT},			*/
/* {"NewEnabled", VAR_PortMappingEnabled, VAR_OUT},			*/
/* {"NewPortMappingDescription", VAR_PortMappingDescription, VAR_OUT},	*/
/* {"NewLeaseDuration", VAR_PortMappingLeaseDuration, VAR_OUT},		*/
{
    int i, parse_status, status;
    netconf_nat_t e;
    mapping_t mapping;
    const unsigned char *bytep;
    static char InternalClient[16];
    static char InternalPort[7];
    static char Enabled[2];
    static char Description[60];

    parse_status = (int) parse_dnat(&e, 
			       ac->params[2].value,	/* NewProtocol */
			       ac->params[0].value,	/* NewRemoteHost */
			       ac->params[1].value, NULL,	/* NewExternalPort */
			       NULL,			/* NewInternalClient */
			       NULL, NULL		/* NewInternalPort */ );

    if (!parse_status) {
	status = SOAP_INVALID_ARGS;
    } else {
	status = SOAP_NOSUCHENTRYINARRAY;
	for (i = 0; mapmgr_get_port_map(i, &mapping); i++) {
	    if (SameMatchInfo(&e, (netconf_nat_t*)&mapping)) {
		InternalClient[0] = '\0';
		if (mapping.ipaddr.s_addr) {
		    bytep = (const unsigned char *) &(mapping.ipaddr);
		    snprintf(InternalClient, sizeof(InternalClient), 
			     "%d.%d.%d.%d", bytep[0], bytep[1], bytep[2], bytep[3]);
		}

		snprintf(InternalPort, sizeof(InternalPort), "%d", ntohs(mapping.ports[0]));
		snprintf(Enabled, sizeof(Enabled), "%d", ((mapping.match.flags & NETCONF_DISABLED) ? 0 : 1));
		strncpy(Description, mapping.desc, sizeof(Description));

		ac->params[3].value = InternalPort;
		ac->params[4].value = InternalClient;
		ac->params[5].value = Enabled;
		ac->params[6].value = Description;
		ac->params[7].value = "0";
		status = 0; /* SUCCESS! */
		break;
	    }
	}
    }

    if (status) 
	soap_error( uclient, status );

    return (status == 0);
}

static bool SameMatchInfo(const netconf_nat_t *e1, const netconf_nat_t *e2)
{
    bool matched = FALSE;

    do {
	if (e1->match.ipproto != e2->match.ipproto) {
	    break;
	}

	if (e1->match.dst.ipaddr.s_addr && 
	    ( e1->match.dst.netmask.s_addr != e2->match.dst.netmask.s_addr 
	      || e1->match.dst.ipaddr.s_addr != e2->match.dst.ipaddr.s_addr) ) {
	    break;
	}
	
	if (e1->match.dst.ports[0] != 0 
	    && e1->match.dst.ports[0] != e2->match.dst.ports[0]) {
	    break;
	}
	if (e1->match.dst.ports[1] 
	    && (e1->match.dst.ports[1] != e2->match.dst.ports[1])) {
	    break;
	}
	matched = TRUE;
    } while (0);

    return matched;
}

/*@.@ fixed upnp prot forware bug*/
int _compare_subnet(unsigned int IP1, unsigned int IP2, unsigned int netmask) 
{ 
     return (IP1 & netmask) == (IP2 & netmask); 
} 
/*@.@ end*/

netconf_nat_t *parse_dnat(netconf_nat_t *entry, const char *Protocol, 
			  const char *RemoteHost, const char *ExternalStartPort, const char *ExternalEndPort,
			  const char *InternalClient, const char *InternalStartPort, const char *InternalEndPort)
{
    /*@.@ fixed upnp port forward bug*/
    unsigned int lan_ip = 0, lan_mask = 0, internel_ip = 0; 
    int ret = 0; 
    /*@.@ end*/
    // it is always an error to not have these two arguments.
    if (!Protocol || !RemoteHost || !ExternalStartPort)
	return NULL;

    memset(entry, 0, sizeof(netconf_nat_t));

    // accept from any port
    entry->match.src.ports[0] = 0;
    entry->match.src.ports[1] = htons(0xffff);

    // parse the external ip address
    if (strlen(RemoteHost)) {
	inet_aton("255.255.255.255", &entry->match.dst.netmask);
	inet_aton(RemoteHost , &entry->match.dst.ipaddr);
    }

    // parse the external ports
    if (!ExternalEndPort) 
	ExternalEndPort = ExternalStartPort;
    entry->match.dst.ports[0] = htons(atoi(ExternalStartPort));
    entry->match.dst.ports[1] = htons(atoi(ExternalEndPort));
    if (entry->match.dst.ports[0] > entry->match.dst.ports[1])
	return NULL;

    // parse the specification of the internal NAT client.
    entry->target = NETCONF_DNAT;

    if (InternalClient && InternalStartPort) {

	// parse the internal ip address.
	inet_aton(InternalClient, (struct in_addr *)&entry->ipaddr);
	/*@.@ fixed upnp forward bug*/
        inet_aton(InternalClient,(void *)&internel_ip); 
        inet_aton(nvram_safe_get("lan_ipaddr"),(void *)&lan_ip); 
        inet_aton(nvram_safe_get("lan_netmask"),(void *)&lan_mask); 
#if 0
        ret = compare_subnet(internel_ip,lan_ip,lan_mask); 
        if (ret == 0) 
        { 
              return NULL; 
        } 
#endif
	/*@.@ end*/
	// parse the internal port number
	if (!InternalEndPort) 
	    InternalEndPort = InternalStartPort;
	entry->ports[0] = htons(atoi(InternalStartPort));
	entry->ports[1] = htons(atoi(InternalEndPort));

	/* check that end port >= start port. */
	if (entry->ports[0] > entry->ports[1])
	    return NULL;

	/* check that internal port range is the same size as the external port range. */
	if ((entry->ports[1]-entry->ports[0]) 
	    != (entry->match.dst.ports[1]-entry->match.dst.ports[0]))
	    return NULL;
    }

    if (strcasecmp(Protocol, "TCP") == 0) {
	entry->match.ipproto = IPPROTO_TCP;
    } else if (strcasecmp(Protocol, "UDP") == 0) {
	entry->match.ipproto = IPPROTO_UDP;
    }


    return entry;

}

