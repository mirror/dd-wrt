/*
    Copyright 2005, Broadcom Corporation      
    All Rights Reserved.      
          
    THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY      
    KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM      
    SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS      
    FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.      
*/

#include <stdarg.h>

#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"

extern char *getsvcval(PService psvc, int i);
extern void soap_response(UFILE*, const char *, const char *, const char *, pvar_entry_t, int);

PService init_service(PServiceTemplate svctmpl, PDevice pdev)
{
    PService psvc;
    PFSVCINIT func;
    int i, table_size;

    UPNP_TRACE(("Initializing service \"%s\".\r\n", svctmpl->name));

    if (svctmpl->schema == NULL)
	svctmpl->schema = "schemas-upnp-org";

    psvc = (PService) malloc(sizeof(Service));
    memset(psvc, 0, sizeof(Service));

    // store pointers to the template and the device into the service.
    psvc->template = svctmpl;
    psvc->device = pdev;
    psvc->instance = ++svctmpl->count;

    // allocate and initialize the service's state variables.
    table_size = svctmpl->nvariables * sizeof(StateVar);
    psvc->vars = (StateVar *) malloc(table_size);
    memset(psvc->vars, 0, table_size);

    for (i = 0; i < svctmpl->nvariables; i++) {
	strcpy(psvc->vars[i].value, svctmpl->variables[i].value);
	psvc->vars[i].flags = svctmpl->variables[i].flags;
	assert((psvc->vars[i].flags & VAR_CHANGED) == 0);
    }

    // call the service's intialization function, if defined.
    if ((func = svctmpl->svcinit) != NULL) {
	(*func)(psvc, SERVICE_CREATE);
    }

    return psvc;
}

void destroy_service(PService psvc)
{
    PFSVCINIT func;

    UPNP_TRACE(("Destroying service \"%s\".\r\n", psvc->template->name));

    // call the service's intialization function, if defined.
    if ((func = psvc->template->svcinit) != NULL) {
	(*func)(psvc, SERVICE_DESTROY);
    }

    free(psvc->vars);
    free(psvc);
}


void mark_changed(PService psvc, int varindex) 
{
    assert(varindex >= 0 && varindex <= psvc->template->nvariables);

    psvc->vars[varindex].flags |= VAR_CHANGED;
    psvc->flags |= VAR_CHANGED;
}


/* Print an XML device description for a device and all its subdevices.
   We used to just print the static XML device description from a file, but now that the 
   IGD is more dynamic and can adjust to different gateway configurations,
   we must dynamically generate the XML.
 */
void service_xml(PService psvc, UFILE *up)
{
    const char *type2str(vartype_t type);
    PFSVCXML func;
    PVarTemplate pv;
    PAction *ac;
    PParam pa;
    const char *tstr;

    uprintf(up, 
	    "<?xml version=\"1.0\"?>\r\n"
	    "<scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">\r\n"
	    "  <specVersion>\r\n"
	    "    <major>1</major>\r\n"
	    "    <minor>0</minor>\r\n"
	    "  </specVersion>\r\n");

    // call the service's xml function, if defined.
    if ((func = psvc->template->svcxml) != NULL) {
	(*func)(psvc, up);
    }

    uprintf(up, "<actionList>\r\n");
    if (psvc->template->actions) {
	for (ac = psvc->template->actions; *ac; ac++) {
	    uprintf(up, "<action>\r\n");
	    uprintf(up, "<name>%s</name>\r\n", (*ac)->name);

	    /* don't print any <argumentList> if there are no args. */
	    if ((pa = (*ac)->params) && pa->name) {
		uprintf(up, "<argumentList>\r\n");
		while (pa->name) {
		    uprintf(up, "<argument>\r\n");
		    uprintf(up, "<name>%s</name>\r\n", pa->name);
		    uprintf(up, "<relatedStateVariable>%s</relatedStateVariable>\r\n",
			    psvc->template->variables[pa->related].name);
		    uprintf(up, "<direction>%s</direction>\r\n",
			    (pa->flags == VAR_OUT ? "out" : "in"));
		    uprintf(up, "</argument>\r\n");

		    pa++;
		}
		uprintf(up, "</argumentList>\r\n");
	    }
	    uprintf(up, "</action>\r\n");
	}
    }
    uprintf(up, "</actionList>\r\n");
	     
    uprintf(up, "<serviceStateTable>\r\n");
    for (pv = psvc->template->variables; pv->name; pv++) {
	uprintf(up, "<stateVariable sendEvents=\"%s\">\r\n", 
		(pv->flags & VAR_EVENTED ? "yes" : "no"));
	uprintf(up, "  <name>%s</name>\r\n", pv->name);
	tstr = type2str(pv->flags&VAR_TYPE_MASK);
	if (tstr == NULL) {
	    UPNP_ERROR(("unknown type - %s \"%s\"\r\n", 
			psvc->template->name, pv->name));
	} else {
	    uprintf(up, "  <dataType>%s</dataType>\r\n", tstr);
	}

	if ((pv->flags & VAR_LIST) && pv->allowed.list ) {
	    char ** avl;
	    uprintf(up, "    <allowedValueList>\r\n");
	    for (avl = pv->allowed.list; *avl; avl++) {
		uprintf(up, "      <allowedValue>%s</allowedValue>\r\n", *avl);
	    }
	    uprintf(up, "    </allowedValueList>\r\n");
	} else if ((pv->flags & VAR_RANGE) && pv->allowed.range ) {
	    PallowedValueRange avr =  pv->allowed.range;
	    uprintf(up, "    <allowedValueRange>\r\n");
	    uprintf(up, "    <minimum>%s</minimum>\r\n", avr->minimum);
	    uprintf(up, "    <maximum>%s</maximum>\r\n", avr->minimum);
	    uprintf(up, "    <step>%s</step>\r\n", avr->step);
	    uprintf(up, "    </allowedValueRange>\r\n");
	}

	uprintf(up, "</stateVariable>\r\n");
    }
    uprintf(up, "</serviceStateTable>\r\n");

    
    uprintf(up, "</scpd>\r\n");
}


const char *type2str(vartype_t type)
{
    char *str;

    switch (type) {
    case VAR_UBYTE:
	str = "ui1";
	break;
    case VAR_USHORT:
	str = "ui2";
	break;
    case VAR_ULONG:
	str = "ui4";
	break;
    case VAR_BYTE:
	str = "i1";
	break;
    case VAR_SHORT:
	str = "i2";
	break;
    case VAR_LONG:
	str = "i4";
	break;
    case VAR_STRING:
	str = "string";
	break;
    case VAR_BOOL:
	str = "boolean";
	break;
    default:
	str = NULL;
    }
    return str;
}



// NOT IMPLEMENTED
int NotImplemented(UFILE *up, PService psvc, PAction ac, pvar_entry_t args, int nargs)
{
    UPNP_ERROR(("Action \"%s\" not implemented, svc \"%s\"\r\n", ac->name, psvc->template->name));

    soap_error( up, SOAP_INVALID_ACTION );

    /* indicate that we have already handled the response.
       (see the handling of return code from 'func' in invoke(). */
    return FALSE;
}



/* The default action is to reflect the contents of the related variables to all OUT parameters.
   That is, if an action has an OUT param, return the value of the <relatedVariable> for that param.
   Do the same for any other out params.

   The value of related variables is obtained by calling getsvcval().
*/
int DefaultAction(UFILE *up, PService psvc, PAction ac, 
		  pvar_entry_t args, int nargs)
{
    PParam pa;

    for (pa = ac->params; pa->name; pa++) {
	if (pa->flags & VAR_OUT) {
	    pa->value = getsvcval(psvc, pa->related);
	}
    }

    return TRUE;
}
