/*
 * imail_imap version buffer overflow exploit attempt
 * 
 * Copyright (C) 2007 Sourcefire, Inc. All Rights Reserved
 * 
 * Writen by Patrick Mullen <pmullen@sourcefire.com>
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

#include "so-util_ber.h"

/* declare detection functions */
int ruleIMAIL_LDAPeval(void *p);

static RuleReference ruleIMAIL_LDAPref0 = 
{
    "url", /* type */
    "labs.idefense.com/intelligence/vulnerabilities/display.php?id=74" /* value */
};

static RuleReference ruleIMAIL_LDAPcve =
{
    "cve", /* type */
    "2004-0297"
};

static RuleReference *ruleIMAIL_LDAPrefs[] =
{
    &ruleIMAIL_LDAPref0,
    &ruleIMAIL_LDAPcve,
    NULL
};

static FlowFlags ruleIMAIL_LDAPflow =
{
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption ruleIMAIL_LDAPoption0 =
{
    OPTION_TYPE_FLOWFLAGS,
    {
        &ruleIMAIL_LDAPflow
    }
};

static ContentInfo ruleIMAIL_LDAPcontent =
{
    (uint8_t *)"|30|",                 /* pattern to search for */
    1,                      /* depth */
    0,                      /* offset */
    CONTENT_BUF_NORMALIZED,                      /* flags */
    NULL,                   /* holder for boyer/moore info */
    NULL,                   /* holder for byte representation of "NetBus" */
    0,                      /* holder for length of byte representation */
    0                       /* holder of increment length */
};

static RuleOption ruleIMAIL_LDAPoption1 =
{
    OPTION_TYPE_CONTENT,
    {
        &ruleIMAIL_LDAPcontent
    }
};


RuleOption *ruleIMAIL_LDAPoptions[] =
{
    &ruleIMAIL_LDAPoption0,
    &ruleIMAIL_LDAPoption1,
    NULL
};

static RuleMetaData ruleIMAIL_LDAPservice0 =
{
   "service ldap"
};

static RuleMetaData ruleIMAIL_LDAPpolicy0 =
{
   "policy max-detect-ips drop"
};

static RuleMetaData *ruleIMAIL_LDAPmetadata[] =
{
   &ruleIMAIL_LDAPservice0,
   &ruleIMAIL_LDAPpolicy0,
   NULL
};

Rule ruleIMAIL_LDAP = {
   /* rule header */
   {
       IPPROTO_TCP, /* proto */
       EXTERNAL_NET, /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       HOME_NET, /* DSTIP     */
       "389", /* DSTPORT   */
   },
   /* metadata */
   { 
       3,  /* genid (HARDCODED!!!) */
       10480, /* sigid d056361f-e644-4242-a918-92131e0b523d */
       6, /* revision 9ffa9a9e-3274-4df9-b54e-a1978f964bbd */
   
       "attempted-admin", /* classification, generic */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "SERVER-OTHER imail ldap buffer overflow exploit attempt",     /* message */
       ruleIMAIL_LDAPrefs, /* ptr to references */
       ruleIMAIL_LDAPmetadata /* ptr to metadata */
   },
   ruleIMAIL_LDAPoptions, /* ptr to rule options */
   &ruleIMAIL_LDAPeval, /* ptr to rule detection function */
   0, /* am I initialized yet? */
   0, /* number of options */
   0  /* don't alert */
};


/* detection functions */
int ruleIMAIL_LDAPeval(void *p) {
   uint32_t current_byte = 0;
   uint32_t width, value, lengthwidth;
   int retval;

   uint32_t payload_len;

   const uint8_t *cursor_normal, *end_of_payload;

   SFSnortPacket *sp = (SFSnortPacket *) p;

   BER_ELEMENT ber_element;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   /* call flow match */
   if (checkFlow(sp, ruleIMAIL_LDAPoptions[0]->option_u.flowFlags) <= 0 )
      return RULE_NOMATCH;

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_payload) <= 0)
      return RULE_NOMATCH;

   payload_len = end_of_payload - cursor_normal;

   if(payload_len < 10)   /* Minimum bind request length */
      return RULE_NOMATCH;

   BER_DATA(0x30);

   /* ldap messageID */
   if(cursor_normal + 3 < end_of_payload) {
      if(*cursor_normal != 0x02) {
         return RULE_NOMATCH;
      }
   }

   // if the messageID is longer than 4 bytes, alert!
   retval = ber_extract_int(sp, &cursor_normal, &ber_element);
   if(retval == BER_FAIL)
      return RULE_MATCH;

   /* Bind request */
   BER_DATA(0x60);

   /* ldap version */
   if(cursor_normal + 3 < end_of_payload) {
      if(*cursor_normal != 0x02) {
         return RULE_NOMATCH;
      }
   }

   // If the ldap version is longer than 4 bytes, alert!
   // Also, just alert if somebody's being weird for grins
   retval = ber_extract_int(sp, &cursor_normal, &ber_element);
   if((retval == BER_FAIL) || (ber_element.data.int_val > 9)) {
      return RULE_MATCH;
   }

   return RULE_NOMATCH;
}
