/*
 * IGMPv3 DoS Vulnerability Detection
 *
 * Copyright (C) 2006 Sourcefire, Inc. All Rights Reserved
 *
 * Writen by Brian Caswell <bmc@sourcefire.com>
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

int ruleIGMPIPOPTDOSeval(void *p);

static HdrOptCheck ruleIGMPIPOPTDOShdr0 =
{
    IP_HDR_PROTO,
    CHECK_EQ,
    IPPROTO_IGMP,
    0,
    0
};

static RuleOption ruleIGMPIPOPTDOSoption0 =
{
    OPTION_TYPE_HDR_CHECK,
    {
        &ruleIGMPIPOPTDOShdr0
    }
};

// content:"RNTO", nocase; 
static ContentInfo ruleIGMPIPOPTDOScontent1 = 
{
    (uint8_t *) "|11|", /* pattern (now in snort content format) */
    1, /* depth */
    0, /* offset */
    CONTENT_BUF_NORMALIZED, /* flags*/
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0  /* increment length */
};

static RuleOption ruleIGMPIPOPTDOSoption1 = 
{
    OPTION_TYPE_CONTENT,
    {
        &ruleIGMPIPOPTDOScontent1
    }
};

/* references for sid IGMPIPOPTDOS */
/* reference:url,technet.microsoft.com/en-us/security/bulletin/ms06-007"; */
static RuleReference ruleIGMPIPOPTDOSref1 = 
{
    "url", /* type */
    "technet.microsoft.com/en-us/security/bulletin/ms06-007" /* value */
};

static RuleReference ruleIGMPIPOPTDOSref2 =
{
    "cve", /* type */
    "2006-0021" /* value */
};

static RuleReference ruleIGMPIPOPTDOSref3 =
{
    "bugtraq", /* type */
    "16645" /* value */
};

static RuleReference *ruleIGMPIPOPTDOSrefs[] =
{
    &ruleIGMPIPOPTDOSref1,
    &ruleIGMPIPOPTDOSref2,
    &ruleIGMPIPOPTDOSref3,
    NULL
};
RuleOption *ruleIGMPIPOPTDOSoptions[] =
{
    &ruleIGMPIPOPTDOSoption0,
    &ruleIGMPIPOPTDOSoption1,
    NULL
};

static RuleMetaData ruleIGMPIPOPTDOSpolicy1 =
{
   "policy max-detect-ips drop"
};

static RuleMetaData *ruleIGMPIPOPTDOSmetadata[] =
{
   &ruleIGMPIPOPTDOSpolicy1,
   NULL
};


Rule ruleIGMPIPOPTDOS = {
   /* rule header, akin to => tcp any any -> any any               */
   {
       IPPROTO_IP, /* proto */
       EXTERNAL_NET, /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       HOME_NET, /* DSTIP     */
       "any", /* DSTPORT   */
   },
   /* metadata */
   { 
       3,  /* genid (HARDCODED!!!) */
       8092, /* sigid  ef742e3c-207d-4049-bfd8-4775ce84178c */
       7, /* revision  5134a9bf-3823-4038-b0dd-79fb4f520908 */
   
       "attempted-dos", /* classification, generic */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "OS-WINDOWS IGMP IP Options validation attempt",     /* message */
       ruleIGMPIPOPTDOSrefs, /* ptr to references */
       ruleIGMPIPOPTDOSmetadata /* ptr to metadata */
   },
   ruleIGMPIPOPTDOSoptions, /* ptr to rule options */
   &ruleIGMPIPOPTDOSeval, /* ptr to rule detection function */
   0, /* am I initialized yet? */
   0, /* number of options */
   0  /* don't alert */
};


/* detection functions */
int ruleIGMPIPOPTDOSeval(void *p) {
    int i = 0;
    uint8_t alert = 0;
    const uint8_t *cursor_normal = 0;
    SFSnortPacket *sp = (SFSnortPacket *) p;
    uint8_t *ip_options_data;

    if (checkHdrOpt(p, ruleIGMPIPOPTDOSoptions[0]->option_u.hdrData)) {
        if (contentMatch(p, ruleIGMPIPOPTDOSoptions[1]->option_u.content, &cursor_normal) > 0) {
            if (sp->ip4_options_data != NULL) {
                ip_options_data = (uint8_t *) sp->ip4_options_data;

                if (sp->ip4_options_length >= 2) {
                    if (*ip_options_data == 0 &&
                        *(ip_options_data+1) == 0) {
                        return RULE_MATCH;
                    }
                }
            }

            for(i=0; i< (int) sp->num_ip_options; i++) {
                if (sp->ip_options[i].option_code == 148) {
                    return RULE_NOMATCH;
                }

                if (sp->ip_options[i].length == 1) {
                    alert++;
                }
            }
            if (alert > 0) {
                return RULE_MATCH;
            }
        }
    }
    return RULE_NOMATCH;
}
