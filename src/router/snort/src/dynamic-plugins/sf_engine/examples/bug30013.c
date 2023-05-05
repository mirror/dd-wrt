/*
 * Winny P2P Application Detection
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2006-2013 Sourcefire, Inc. All Rights Reserved
 *
 * Writen by Lurene Grenier <lurene.grenier> & Brian Caswell <bmc@sourcefire.com>
 *
 * This file may contain proprietary rules that were created, tested and
 * certified by Sourcefire, Inc. (the "VRT Certified Rules") as well as
 * rules that were created by Sourcefire and other third parties and
 * distributed under the GNU General Public License (the "GPL Rules").  The
 * VRT Certified Rules contained in this file are the property of
 * Sourcefire, Inc. Copyright 2005 Sourcefire, Inc. All Rights Reserved.
 * The GPL Rules created by Sourcefire, Inc. are the property of
 * Sourcefire, Inc. Copyright 2002-2005 Sourcefire, Inc. All Rights
 * Reserved.  All other GPL Rules are owned and copyrighted by their
 * respective owners (please see www.snort.org/contributors for a list of
 * owners and their respective copyrights).  In order to determine what
 * rules are VRT Certified Rules or GPL Rules, please refer to the VRT
 * Certified Rules License Agreement.
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"


/* declare detection functions */
int ruleIPOPTDOSeval(void *p);

/* references for sid IPOPTDOS */
/* reference:url,www.microsoft.com/technet/security/bulletin/ms06-007.mspx"; */
static RuleReference ruleIPOPTDOSref1 =
{
    "url", /* type */
    "www.microsoft.com/technet/security/bulletin/ms06-032.mspx" /* value XXX - update me */
};

static RuleReference *ruleIPOPTDOSrefs[] =
{
    &ruleIPOPTDOSref1,
    NULL
};
RuleOption *ruleIPOPTDOSoptions[] =
{
    NULL
};

Rule ruleIPOPTDOS = {
   /* rule header, akin to => tcp any any -> any any               */
   {
       IPPROTO_ICMP, /* proto */
       "any", /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       "any", /* DSTIP     */
       "any", /* DSTPORT   */
   },
   /* metadata */
   {
       3,  /* genid (HARDCODED!!!) */
       7022, /* sigid da1421ff-dcf4-46af-bed7-154cc8a1fca5 */
       1, /* revision a8d3af9a-ef7e-47f2-bf2b-11aae4cedb3e */

       "attempted-dos", /* classification, generic */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "DOS Microsoft IP Options denial of service",     /* message */
       ruleIPOPTDOSrefs, /* ptr to references */
       NULL /* Meta data */
   },
   ruleIPOPTDOSoptions, /* ptr to rule options */
   &ruleIPOPTDOSeval, /* ptr to rule detection function */
   0, /* am I initialized yet? */
   0, /* number of options */
   0, /* don't alert */
   NULL /* ptr to internal data... setup during rule registration */
};

/* detection functions */
int ruleIPOPTDOSeval(void *p) {
    u_int32_t   i;
    //int j;
    //u_int8_t    *ptr, t, tmp;
    IPOptions    *ipopt;
    u_int8_t    *ipoptdata;

    SFSnortPacket *sp = (SFSnortPacket *) p;

    /* Make sure we don't have a NULL packet */
    if (NULL == sp)
        return RULE_NOMATCH;

    /* Make sure we have an icmp packet */
    if (sp->ip4_header->proto != 1)
    {
        return RULE_NOMATCH;
    }

    /* Make sure we have an icmp packet with ip options */
    if (sp->ip4_options_length == 0)
    {
        return RULE_NOMATCH;
    }

    //printf("Number of ip options %d\n", sp->num_ip_options);

    for(i=0; i < sp->num_ip_options; i++)
    {
        ipopt = &(sp->ip_options[i]);
        //printf("\n\n\n\n******I'm on options code %d\n\n\n\n\n********",ipopt->option_code);
        /* Only care about LSRR and SRR codes */
        if(ipopt->option_code == 131 || ipopt->option_code == 137)
        {
            /* Length of option must be more than 0 */
            if(ipopt->length > 0)
            {
                ipoptdata = ipopt->option_data;

                if((u_int8_t)*(ipoptdata) <= 4)
                {
                    return RULE_MATCH;
                }
            }
        }
    }

    //return (0);
    return RULE_NOMATCH;
}

#if 0
Rule *rules[] = {
    &ruleIPOPTDOS,
    NULL
};
#endif
