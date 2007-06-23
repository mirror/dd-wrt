/*
 *  Copyright 2005, Broadcom Corporation      
 *  All Rights Reserved.      
 *        
 *  THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY      
 *  KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM      
 *  SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS      
 *  FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.      
 *
 *  $Id: wanppp.c,v 1.1.1.7 2005/03/07 07:31:12 kanki Exp $
 */

#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"
#include "igd.h"
#include "wanppp.h"
#include "bcmnvram.h"

#if defined(linux)
#define    pppoe_up()
#define    pppoe_down()
#else
extern void pppoe_up();
extern void pppoe_down();
#endif


extern void igd_restart(int secs);
extern char * safe_snprintf(char *str, int *len, const char *fmt, ...);

extern char *WANPPPConnection_PossibleConnectionTypes_allowedValueList[];

int WANPPPConnection_GetVar(struct Service *psvc, int varindex)
{
    struct StateVar *var;
    char **p, *cur;
    int len;

    var = &(psvc->vars[varindex]);

    switch (varindex) {
    case VAR_PossibleConnectionTypes:
	cur = var->value;
	len = sizeof(var->value);
	for (p = WANPPPConnection_PossibleConnectionTypes_allowedValueList; *p; p++) {
	    if (cur != var->value)
		cur = safe_snprintf(cur, &len, ",");
	    cur = safe_snprintf(cur, &len, "%s", *p);
	}
	break;
    case VAR_UserName:
	strcpy(var->value, nvram_safe_get("pppoe_username"));
	break;
    case VAR_Password:
	strcpy(var->value, nvram_safe_get("pppoe_passwd"));
	break;
    case VAR_IdleDisconnectTime:
	strcpy(var->value, nvram_safe_get("pppoe_idletime"));
	break;
    case VAR_ExternalIPAddress: 
	strcpy(var->value, nvram_safe_get("wan_ipaddr"));
	break;
    }
    return TRUE;
}


int WANPPPConnection_ConfigureConnection(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
//       {"NewUserName", VAR_UserName, VAR_IN},
//       {"NewPassword", VAR_Password, VAR_IN},
{
    uint success = TRUE; /* assume no error will occur */

    if (strlen(ac->params[0].value) > 32 || strlen(ac->params[1].value) > 32) {
	soap_error( uclient, SOAP_INVALID_ARGS);
	success = FALSE;
    } else {
	nvram_set("pppoe_username", ac->params[0].value);
	nvram_set("pppoe_passwd", ac->params[1].value);
	//nvram_commit();
    }

    return success;
}


int WANPPPConnection_SetIdleDisconnectTime(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
//       {"NewIdleDisconnectTime", VAR_IdleDisconnectTime, VAR_IN},
{
    uint success = TRUE; /* assume no error will occur */

    nvram_set("pppoe_idletime", ac->params[0].value);
//    nvram_commit();

    return success;
}


int WANPPPConnection_RequestConnection(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
    char tmp[100];

    /* should probably confirm that the pppoe_username and ppoe_passwd
       are set here. */
    nvram_set(igd_pri_wan_var(tmp, sizeof(tmp), "proto"), "pppoe");
//    nvram_commit();
    pppoe_up();
    
    return TRUE;
}

int WANPPPConnection_ForceTermination(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
    pppoe_down();
    
    return TRUE;
}


int WANPPPConnection_SetConnectionType(UFILE *uclient, PService psvc, PAction ac, pvar_entry_t args, int nargs)
/*     "SetConnectionType", WANPPPConnection_SetConnectionType, */
{
    snprintf(psvc->vars[VAR_ConnectionType].value, sizeof(psvc->vars[VAR_ConnectionType].value), ac->params[0].value);

    return TRUE;
}
