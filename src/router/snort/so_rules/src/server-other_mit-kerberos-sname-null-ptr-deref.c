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
int rule34971eval(void *p);
int rule34972eval(void *p);
static int DetectKrbNullPtrDeref(SFSnortPacket *sp, const uint8_t *cursor_normal, const uint8_t *end_of_buffer);

/* declare rule data structures */
/* flow:to_server; */
static FlowFlags rule34971flow0 = 
{
   FLOW_TO_SERVER
};

static RuleOption rule34971option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule34971flow0
   }
};

/* flow:to_server,established */
static FlowFlags rule34972flow0 =
{
   FLOW_TO_SERVER|FLOW_ESTABLISHED
};

static RuleOption rule34972option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule34972flow0
   }
};

#ifndef CONTENT_FAST_PATTERN_ONLY
#define CONTENT_FAST_PATTERN_ONLY CONTENT_FAST_PATTERN
#endif
// content:"|A2 03 02 01 0A|", depth 0, fast_pattern:only; 
static ContentInfo rule_KerberosNullPtrDeref_content1 = 
{
   (uint8_t *) "|A2 03 02 01 0A|", /* pattern */
   0, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN_ONLY|CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule_KerberosNullPtrDeref_option1 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule_KerberosNullPtrDeref_content1
   }
};

/* references */
/* reference: bugtraq "63555"; */
static RuleReference rule_KerberosNullPtrDeref_ref1 = 
{
   "bugtraq", /* type */
   "63555" /* value */
};

/* reference: cve "2013-1418"; */
static RuleReference rule_KerberosNullPtrDeref_ref2 = 
{
   "cve", /* type */
   "2013-1418" /* value */
};

/* reference: url "osvdb.org/show/osvdb/99508"; */
static RuleReference rule_KerberosNullPtrDeref_ref3 = 
{
   "url", /* type */
   "osvdb.org/show/osvdb/99508" /* value */
};

static RuleReference *rule_KerberosNullPtrDeref_refs[] =
{
   &rule_KerberosNullPtrDeref_ref1,
   &rule_KerberosNullPtrDeref_ref2,
   &rule_KerberosNullPtrDeref_ref3,
   NULL
};

/* metadata */
/* metadata:service kerberos, policy security-ips drop; */
static RuleMetaData rule_KerberosNullPtrDeref_service1 = 
{
   "service kerberos"
};

static RuleMetaData rule_KerberosNullPtrDeref_policy1 = 
{
   "policy security-ips drop"
};

static RuleMetaData *rule_KerberosNullPtrDeref_metadata[] =
{
   &rule_KerberosNullPtrDeref_service1,
   &rule_KerberosNullPtrDeref_policy1,
   NULL
};

RuleOption *rule34971options[] =
{
   &rule34971option0,
   &rule_KerberosNullPtrDeref_option1,
   NULL
};

Rule rule34971 = {
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
      34971, /* sigid */
      1, /* revision */
      "attempted-dos", /* classification */
      0,  /* hardcoded priority */
      "SERVER-OTHER MIT Kerberos KDC as-req sname null pointer dereference attempt",     /* message */
      rule_KerberosNullPtrDeref_refs, /* ptr to references */
      rule_KerberosNullPtrDeref_metadata /* ptr to metadata */
   },
   rule34971options, /* ptr to rule options */
   &rule34971eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

RuleOption *rule34972options[] =
{
   &rule34972option0,
   &rule_KerberosNullPtrDeref_option1,
   NULL
};

Rule rule34972 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_TCP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "88", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      34972, /* sigid */
      1, /* revision */
      "attempted-dos", /* classification */
      0,  /* hardcoded priority */
      "SERVER-OTHER MIT Kerberos KDC as-req sname null pointer dereference attempt",     /* message */
      rule_KerberosNullPtrDeref_refs, /* ptr to references */
      rule_KerberosNullPtrDeref_metadata /* ptr to metadata */
   },
   rule34972options, /* ptr to rule options */
   &rule34972eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

/* detection functions */
int rule34971eval(void *p) {
   const uint8_t *cursor_normal = 0, *end_of_buffer;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   // flow:to_server
   if(checkFlow(p, rule34971options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   return DetectKrbNullPtrDeref(sp, cursor_normal, end_of_buffer);
}

int rule34972eval(void *p) {
   const uint8_t *cursor_normal = 0, *end_of_buffer;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   // flow:to_server,established
   if(checkFlow(p, rule34972options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   // in the TCP case, as-req starts at offset 4
   cursor_normal += 4;

   return DetectKrbNullPtrDeref(sp, cursor_normal, end_of_buffer);
}

static int DetectKrbNullPtrDeref(SFSnortPacket *sp, const uint8_t *cursor_normal, const uint8_t *end_of_buffer) {
   BER_ELEMENT msg_type, test_element;

   BER_DATA(0x6A);            // AS-REQ [10]
   BER_DATA(0x30);            // SEQUENCE [16]
   BER_SKIP(0xA1);            // pvno [1]

   BER_DATA(0xA2);            // msg-type [2]
   BER_EXTRACT_INT(msg_type);

   // make sure msg-type is krb-as-req (10)
   if(msg_type.data.int_val != 10)
      return RULE_NOMATCH;

   // if optional PA-DATA exists, skip it
   if(cursor_normal + 1 > end_of_buffer)
      return RULE_NOMATCH;
   if(*cursor_normal == 0xA3)
      BER_SKIP(0xA3);

   // KDC-REQ-BODY [4] ::= SEQUENCE [16]
   //    kdc-options [0]
   //    cname       [1] 
   //    realm       [2]
   //    sname       [3] 
   BER_DATA(0xA4);  // KDC-REQ-BODY [4]
   BER_DATA(0x30);  // SEQUENCE [16]
   BER_SKIP(0xA0);  // kdc-options [0]

   // if optional cname exists, skip it
   if(cursor_normal + 1 > end_of_buffer)
      return RULE_NOMATCH;
   if(*cursor_normal == 0xA1)
      BER_SKIP(0xA1);

   BER_SKIP(0xA2); // realm [2]

   // check for sname
   if(cursor_normal + 1 > end_of_buffer)
      return RULE_NOMATCH;

   // if the next BER element isn't sname [3], null ptr deref triggered, alert
   if(*cursor_normal != 0xA3)
   {
      DEBUG_SO(fprintf(stderr,"expected sname [3], got BER type: 0x%02X\n", *cursor_normal);)
      return RULE_MATCH;
   }

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule34971,
    &rule34972,
    NULL
};
*/
