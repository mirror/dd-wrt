/* policy.c - Access control policies */
 
/* Written 1997,1998 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <atm.h>
#include <atmd.h>

#include "proto.h" /* for "pretty" */
#include "policy.h"


#define COMPONENT "POLICY"


static RULE *rules = NULL;
static RULE **next = &rules;


void add_rule(RULE *rule)
{
    rule->hits = 0;
    rule->next = NULL;
    *next = rule;
    next = &rule->next;
}


int allow(const struct sockaddr_atmsvc *addr,int direction)
{
    RULE *rule;
    char buffer[MAX_ATM_ADDR_LEN+1];
    int count;

    if ((direction & (ACL_IN | ACL_OUT)) == (ACL_IN | ACL_OUT))
	diag(COMPONENT,DIAG_ERROR,"allow: ACL_IN && ACL_OUT");
    count = 0;
    for (rule = rules; rule; rule = rule->next) {
	count++;
	if (!(rule->type & direction)) continue;
	if (!atm_equal((struct sockaddr *) addr,
	  (struct sockaddr *) &rule->addr,rule->mask,AXE_PRVOPT |
	  (rule->mask == -1 ? 0 : AXE_WILDCARD))) continue;
	rule->hits++;
	if (atm2text(buffer,MAX_ATM_ADDR_LEN+1,(struct sockaddr *) addr,
	  pretty) < 0)
	    strcpy(buffer,"<invalid address>");
	diag(COMPONENT,DIAG_DEBUG,"Rule %d: %s call %s %s",count,
	  rule->type & ACL_ALLOW ? "allowed" : "rejected",
	  direction & ACL_IN ? "from" : "to",buffer);
	return rule->type & ACL_ALLOW;
    }
    return 1;
}
