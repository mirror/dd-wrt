/*
 * Microsoft Windows LSRR RR IP Option Interger Overflow
 * 
 * Copyright (C) 2007 Sourcefire, Inc. All Rights Reserved
 * 
 * Writen by Matthew Watchinski Sourcefire VRT <mwatchinski@sourcefire.com>
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


#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"


/* declare detection functions */
int ruleIPOPTDOSeval(void *p);

/* references for sid IPOPTDOS */
/* reference:url,technet.microsoft.com/en-us/security/bulletin/ms06-007"; */
static RuleReference ruleIPOPTDOSref1 = 
{
    "url", /* type */
    "technet.microsoft.com/en-us/security/bulletin/ms06-032" /* value XXX - update me */
};

static RuleReference ruleIPOPTDOSref2 =
{
    "cve",
    "2006-2379"
};

static RuleReference *ruleIPOPTDOSrefs[] =
{
    &ruleIPOPTDOSref1,
    &ruleIPOPTDOSref2,
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
       EXTERNAL_NET, /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       HOME_NET, /* DSTIP     */
       "any", /* DSTPORT   */
   },
   /* metadata */
   { 
       3,  /* genid (HARDCODED!!!) */
       10127, /* sigid a46614a1-0039-4b11-b99f-f7e9ea7f69d4 */
       4, /* revision d4882b50-6a70-43fd-98e3-b9d6060ce62d */
   
       "attempted-dos", /* classification, generic */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "OS-WINDOWS Microsoft IP Options denial of service",     /* message */
       ruleIPOPTDOSrefs /* ptr to references */
       ,NULL
   },
   ruleIPOPTDOSoptions, /* ptr to rule options */
   &ruleIPOPTDOSeval, /* ptr to rule detection function */
   0, /* am I initialized yet? */
   0, /* number of options */
   0  /* don't alert */
};

/* detection functions */
int ruleIPOPTDOSeval(void *p) {
    uint32_t   i;
    //int j;
    //uint8_t    *ptr, t, tmp;    
    IPOptions    *ipopt;
    uint8_t    *ipoptdata;

    SFSnortPacket *sp = (SFSnortPacket *) p;

    /* Make sure we don't have a NULL packet */
    if (NULL == sp)
        return RULE_NOMATCH;

    /* Make sure we are IPv4 */
    if(sp->ip4_header == NULL)
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

                if((uint8_t)*(ipoptdata) <= 4)
                {
                    return RULE_MATCH;
                }
            }
        }
    }

    //return (0);
    return RULE_NOMATCH;
}
