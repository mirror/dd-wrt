/*
 * Copyright (C) 2005-2013 Sourcefire, Inc. All Rights Reserved
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

//#define DEBUG
#ifdef DEBUG
#define DEBUG_SO(code) code
#else
#define DEBUG_SO(code)
#endif

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"

#include "so-util_ber.h"

/* declare detection functions */
int rule27906eval(void *p);

/* declare rule data structures */
/* flow:to_server; */
static FlowFlags rule27906flow0 = 
{
   FLOW_TO_SERVER
};

static RuleOption rule27906option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule27906flow0
   }
};

#ifndef CONTENT_FAST_PATTERN_ONLY
#define CONTENT_FAST_PATTERN_ONLY CONTENT_FAST_PATTERN
#endif
// content:"|1b 00|", depth 0, fast_pattern:only; 
static ContentInfo rule27906content1 = 
{
   (uint8_t *) "|1b 00|", /* pattern (now in snort content format) */
   0, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN_ONLY|CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule27906option1 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule27906content1
   }
};

/* references for sid 27906 */
/* reference: cve "2013-1416"; */
static RuleReference rule27906ref1 = 
{
   "cve", /* type */
   "2013-1416" /* value */
};

/* reference: url "web.mit.edu/kerberos/krb5-1.10/"; */
static RuleReference rule27906ref2 = 
{
   "url", /* type */
   "web.mit.edu/kerberos/krb5-1.10/" /* value */
};

static RuleReference *rule27906refs[] =
{
   &rule27906ref1,
   &rule27906ref2,
   NULL
};

/* metadata for sid 27906 */
/* metadata:service kerberos, policy balanced-ips drop, policy security-ips drop; */
static RuleMetaData rule27906service1 = 
{
   "service kerberos"
};

//static RuleMetaData rule27906policy1 = 
//{
//   "policy balanced-ips drop"
//};

static RuleMetaData rule27906policy2 = 
{
   "policy security-ips drop"
};

static RuleMetaData rule27906policy3 =
{
   "policy max-detect-ips drop"
};

static RuleMetaData *rule27906metadata[] =
{
   &rule27906service1,
//   &rule27906policy1,
   &rule27906policy2,
   &rule27906policy3,
   NULL
};

RuleOption *rule27906options[] =
{
   &rule27906option0,
   &rule27906option1,
   NULL
};

Rule rule27906 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_UDP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "88", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      27906, /* sigid */
      3, /* revision */
      "attempted-admin", /* classification */
      0,  /* hardcoded priority */
      "SERVER-OTHER MIT Kerberos KDC prep_reprocess_req null pointer dereference attempt",     /* message */
      rule27906refs, /* ptr to references */
      rule27906metadata /* ptr to metadata */
   },
   rule27906options, /* ptr to rule options */
   &rule27906eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule27906eval(void *p) {
   const uint8_t *cursor_normal = 0, *end_of_payload;
   SFSnortPacket *sp = (SFSnortPacket *) p;
   BER_ELEMENT kerberos_string;
   int i;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;
   
   // flow:to_server;
   if(checkFlow(p, rule27906options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_payload) <= 0)
      return RULE_NOMATCH;

   BER_DATA(0x6c);          // KDC-REQ [12]
   BER_DATA(0x30);          // SEQUENCE [16]
   BER_SKIP(0xA1);          // pvno [1]
   BER_SKIP(0xA2);          // msg-type [2]

   // if optional PA-DATA exists, skip it
   // 10 1 00011 (context-specific, structured, tag number 3)
   if(cursor_normal+1 > end_of_payload)
      return RULE_NOMATCH;
   if(*cursor_normal == 0xA3)
      BER_SKIP(0xA3);

   // KDC-REQ-BODY [4] ::= SEQUENCE [16]
   //    kdc-options [0] 
   //    realm       [2]  server's realm
   //    sname       [3]  PrincipalName
   BER_DATA(0xA4);  // KDC-REQ-BODY
   BER_DATA(0x30);  // SEQUENCE
   BER_SKIP(0xA0);  // kdc-options
   BER_SKIP(0xA2);  // realm

   // PrincipalName [3] ::= SEQUENCE [16]
   //    name-type   [0] Int32
   //    name-string [1] SEQUENCE [16] of KerberosString
   BER_DATA(0xA3);
   BER_DATA(0x30);
   BER_SKIP(0xA0);
   BER_DATA(0xA1);
   BER_DATA(0x30);

   // check up to 20 strings for the vulnerable condition
   for(i=0; (ber_get_element(sp, cursor_normal, &kerberos_string) >= 0) && (i < 20); i++) {
      // verify we are looking at a string element
      if(kerberos_string.type != 0x1b)
         return RULE_NOMATCH;

      DEBUG_SO(fprintf(stderr,"kerberos_string:\n data_len = 0x%02x\n",kerberos_string.data_len);)

      // vulnerable condition is kerberos_string.data_len == 0
      if(kerberos_string.data_len == 0)
         return RULE_MATCH;

      // Move to the end of the current element.  Guaranteed to move us forward in the packet.
      cursor_normal += kerberos_string.total_len;
   }

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule27906,
    NULL
};
*/
