/*
 *  Copyright 2005, Broadcom Corporation      
 *  All Rights Reserved.      
 *        
 *  THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY      
 *  KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM      
 *  SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS      
 *  FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.      
 *
 *  $Id: lanhostconfig.c,v 1.4 2005/03/07 08:35:32 kanki Exp $
 */

#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "lanhostconfig.h"
#include "bcmnvram.h"

int LANHostConfig_GetVar(struct Service *psvc, int varindex)
{
    struct StateVar *var;

    var = &(psvc->vars[varindex]);

    switch (varindex) {

    case VAR_DomainName: 
	strcpy(var->value, nvram_safe_get("lan_domain"));
	break;

    case VAR_SubnetMask:
	strcpy(var->value, nvram_safe_get("lan_netmask"));
	break;

    case VAR_MinAddress:
	strcpy(var->value, nvram_safe_get("dhcp_start"));
	break;

    case VAR_MaxAddress:
	strcpy(var->value, nvram_safe_get("dhcp_end"));
	break;
    } /* end-switch */

    return TRUE;
}


int LANHostConfig_SetDomainName(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
//       {"NewDomainName", VAR_DomainName, VAR_IN},
{
    uint success = TRUE; /* assume no error will occur */

    nvram_set("lan_domain", ac->params[0].value);
    fprintf(stderr, "LANHostConfig_SetDomainName(): nvram_commit()\n");
//    nvram_commit();
    // restart ?? 
    
    return success;
}
