/*
 * MISC OpenLDAP LDAP Server BIND Request DoS attempt
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
int ruleOPENLDAP_BIND_DOSeval(void *p);

static RuleReference ruleOPENLDAP_BIND_DOSref1 =
{
    "cve", /* type */
    "2006-5779"
};
static RuleReference ruleOPENLDAP_BIND_DOSref2 =
{
    "bugtraq", /* type */
    "20939"
};

static RuleReference *ruleOPENLDAP_BIND_DOSrefs[] =
{
    &ruleOPENLDAP_BIND_DOSref1,
    &ruleOPENLDAP_BIND_DOSref2,
    NULL
};

static FlowFlags ruleOPENLDAP_BIND_DOSflow =
{
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption ruleOPENLDAP_BIND_DOSoption0 =
{
    OPTION_TYPE_FLOWFLAGS,
    {
        &ruleOPENLDAP_BIND_DOSflow
    }
};

static ContentInfo ruleOPENLDAP_BIND_DOScontent0 =
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

static RuleOption ruleOPENLDAP_BIND_DOSoption1 =
{
    OPTION_TYPE_CONTENT,
    {
        &ruleOPENLDAP_BIND_DOScontent0
    }
};

static ContentInfo ruleOPENLDAP_BIND_DOScontent1 =
{
    (uint8_t *)"CRAM-MD5",                 /* pattern to search for */
    8,                      /* depth */
    0,                      /* offset */
    CONTENT_BUF_NORMALIZED | CONTENT_NOCASE | CONTENT_RELATIVE,                      /* flags */
    NULL,                   /* holder for boyer/moore info */
    NULL,                   /* holder for byte representation of "NetBus" */
    0,                      /* holder for length of byte representation */
    0                       /* holder of increment length */
};

static RuleOption ruleOPENLDAP_BIND_DOSoption2 =
{
    OPTION_TYPE_CONTENT,
    {
        &ruleOPENLDAP_BIND_DOScontent1
    }
};

RuleOption *ruleOPENLDAP_BIND_DOSoptions[] =
{
    &ruleOPENLDAP_BIND_DOSoption0,
    &ruleOPENLDAP_BIND_DOSoption1,
    &ruleOPENLDAP_BIND_DOSoption2,
    NULL
};

static RuleMetaData ruleOPENLDAP_BIND_DOSservice1 =
{
    "service ldap"
};

static RuleMetaData ruleOPENLDAP_BIND_DOSpolicy1 =
{
    "policy max-detect-ips drop"
};

static RuleMetaData *ruleOPENLDAP_BIND_DOSmetadata[] =
{
    &ruleOPENLDAP_BIND_DOSservice1,
    &ruleOPENLDAP_BIND_DOSpolicy1,
    NULL
};

Rule ruleOPENLDAP_BIND_DOS = {
   /* rule header */
   {
       IPPROTO_TCP, /* proto */
       "any", /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       HOME_NET, /* DSTIP     */
       "389", /* DSTPORT   */
   },
   /* metadata */
   { 
       3,  /* genid (HARDCODED!!!) */
       13425, /* sigid d5ad0160-8938-4d47-b51f-b1aa6b382172 */
       6, /* revision c109ff0a-9630-4d2c-9da4-a3b557ea698e */
   
       "denial-of-service", /* classification, generic */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "SERVER-OTHER openldap server bind request denial of service attempt",     /* message */
       ruleOPENLDAP_BIND_DOSrefs /* ptr to references */
        ,ruleOPENLDAP_BIND_DOSmetadata
   },
   ruleOPENLDAP_BIND_DOSoptions, /* ptr to rule options */
   &ruleOPENLDAP_BIND_DOSeval, /* ptr to rule detection function */
   0, /* am I initialized yet? */
   0, /* number of options */
   0  /* don't alert */
};


/* detection functions */

int ruleOPENLDAP_BIND_DOSeval(void *p) {
   int retval;
   uint32_t size_len, size;

   const uint8_t *cursor_normal, *end_of_payload; /*, *end_of_payload;*/

   SFSnortPacket *sp = (SFSnortPacket *) p;

   BER_ELEMENT element;


   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_payload) <= 0)
      return RULE_NOMATCH;

   if((end_of_payload - cursor_normal) < 285)   /* Minimum malicious BIND request length */
      return RULE_NOMATCH;

   /* call flow match */
   if (checkFlow(sp, ruleOPENLDAP_BIND_DOSoptions[0]->option_u.flowFlags) <= 0 )
      return RULE_NOMATCH;

   /* call content match */
   if (contentMatch(sp, ruleOPENLDAP_BIND_DOSoptions[1]->option_u.content, &cursor_normal) <= 0) {
      return RULE_NOMATCH;
   }

   /* our contentMatch already assures us the first byte is \x30, so we're pointing at the size */

   /* Begin packet structure processing */
   retval = ber_get_size(p, cursor_normal, &size_len, &size); // message length

   if(retval < 0)
      return(RULE_NOMATCH);

   cursor_normal += size_len;

   /* Message number (only care about width of the specifier) */
   retval = ber_get_element(p, cursor_normal, &element);

   if(retval < 0 || element.type != 0x02)
      return(RULE_NOMATCH);

   cursor_normal += element.total_len;

   /* BIND request */
   retval = ber_get_element(p, cursor_normal, &element);

   if(retval < 0 || element.type != 0x60)
      return RULE_NOMATCH;

   /* We're inside the BIND request.  Now we need to parse the internals */
   cursor_normal = element.data.data_ptr;

   /* bind version */
   retval = ber_get_element(p, cursor_normal, &element);
   if(retval < 0 || element.type != 0x02) 
      return RULE_NOMATCH;

   cursor_normal += element.total_len;

   /* DN */
   retval = ber_get_element(p, cursor_normal, &element);
   if(retval < 0 || element.type != 0x04)
      return RULE_NOMATCH;

   cursor_normal += element.total_len;

   /* SASL authtype request */
   retval = ber_get_element(p, cursor_normal, &element);

   if(retval < 0 || element.type != 0xa3)
      return RULE_NOMATCH;

   /* We're inside the SASL BIND request.  Now we need to parse the internals */
   cursor_normal = element.data.data_ptr;

   /* SASL auth mechanism */
   retval = ber_get_element(p, cursor_normal, &element);
   if((retval < 0) || (element.type != 0x04) || (element.data_len != 8))
      return RULE_NOMATCH;

   /* call content match "CRAM-MD5" */
   /* This will modify element.data.data_ptr, but we don't care */
   if(contentMatch(sp, ruleOPENLDAP_BIND_DOSoptions[1]->option_u.content, &(element.data.data_ptr)) <= 0) {
      return RULE_NOMATCH;
   }

   cursor_normal += element.total_len;

   /* Credentials
      For our check, we need to have 255 bytes in the actual buffer, so we need to
      verify the number of bytes present, not just the data_len reported by the
      BER element.
   */
   retval = ber_get_element(p, cursor_normal, &element);

   if((retval < 255) || (element.type != 0x04))
      return RULE_NOMATCH;

   /* Here's the actual exploit detection -- see if there's a space at 255 bytes */
   if(element.data.data_ptr[254] == ' ')
      return RULE_MATCH;

   return RULE_NOMATCH; 
}

/*
Rule *rules[] = {
    &ruleOPENLDAP_BIND_DOS,
    NULL
};

*/
