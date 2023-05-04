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

#include "so-util.h"

/* declare detection functions */
int rule37675eval(void *p);

/* declare rule data structures */
/* flow:to_server; */
static FlowFlags rule_CiscoIkeBof_flow0 = 
{
   FLOW_TO_SERVER
};

static RuleOption rule_CiscoIkeBof_option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule_CiscoIkeBof_flow0
   }
};

#ifndef CONTENT_FAST_PATTERN_ONLY
#define CONTENT_FAST_PATTERN_ONLY CONTENT_FAST_PATTERN
#endif

// content:"|84|", fast_pattern:only; 
static ContentInfo rule_CiscoIkeBof_content1 = 
{
   (uint8_t *) "|84|", /* pattern */
   0, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN_ONLY|CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule_CiscoIkeBof_option1 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule_CiscoIkeBof_content1
   }
};

/* references */
/* reference: cve "2016-1287"; */
static RuleReference rule_CiscoIkeBof_ref1 = 
{
   "cve", /* type */
   "2016-1287" /* value */
};

/* reference: cve "2016-1344"; */
static RuleReference rule_CiscoIkeBof_ref2 = 
{
   "cve", /* type */
   "2016-1344" /* value */
};

/* reference: cve "2016-6381"; */
static RuleReference rule_CiscoIkeBof_ref3 = 
{
   "cve", /* type */
   "2016-6381" /* value */
};

/* reference: url "tools.cisco.com/security/center/content/CiscoSecurityAdvisory/cisco-sa-20160210-asa-ike"; */
static RuleReference rule_CiscoIkeBof_ref4 = 
{
   "url", /* type */
   "tools.cisco.com/security/center/content/CiscoSecurityAdvisory/cisco-sa-20160210-asa-ike" /* value */
};

/* reference: url "tools.cisco.com/security/center/content/CiscoSecurityAdvisory/cisco-sa-20160323-ios-ikev2"; */
static RuleReference rule_CiscoIkeBof_ref5 = 
{
   "url", /* type */
   "tools.cisco.com/security/center/content/CiscoSecurityAdvisory/cisco-sa-20160323-ios-ikev2" /* value */
};

/* reference: url "tools.cisco.com/security/center/content/CiscoSecurityAdvisory/cisco-sa-20160928-ios-ikev1"; */
static RuleReference rule_CiscoIkeBof_ref6 = 
{
   "url", /* type */
   "tools.cisco.com/security/center/content/CiscoSecurityAdvisory/cisco-sa-20160928-ios-ikev1" /* value */
};

static RuleReference *rule_CiscoIkeBof_refs[] =
{
   &rule_CiscoIkeBof_ref1,
   &rule_CiscoIkeBof_ref2,
   &rule_CiscoIkeBof_ref3,
   &rule_CiscoIkeBof_ref4,
   &rule_CiscoIkeBof_ref5,
   &rule_CiscoIkeBof_ref6,
   NULL
};

/* metadata */
/* metadata:policy security-ips drop; */
static RuleMetaData rule_CiscoIkeBof_policy1 = 
{
   "policy security-ips drop"
};

static RuleMetaData *rule_CiscoIkeBof_metadata[] =
{
   &rule_CiscoIkeBof_policy1,
   NULL
};

RuleOption *rule37675options[] =
{
   &rule_CiscoIkeBof_option0,
   &rule_CiscoIkeBof_option1,
   NULL
};

Rule rule37675 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_UDP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "[500,4500]", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      37675, /* sigid */
      3, /* revision */
      "attempted-admin", /* classification */
      0,  /* hardcoded priority */
      "SERVER-OTHER Cisco IOS invalid IKE fragment length memory corruption or exhaustion attempt",     /* message */
      rule_CiscoIkeBof_refs, /* ptr to references */
      rule_CiscoIkeBof_metadata /* ptr to metadata */
   },
   rule37675options, /* ptr to rule options */
   &rule37675eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

/* detection functions */
int rule37675eval(void *p) {
   const uint8_t *cursor_normal = 0, *end_of_buffer;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   int i;
   const uint8_t *next_payload_pos;
   uint8_t payload_type, next_payload_type;
   uint16_t payload_length;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   // flow:to_server
   if(checkFlow(p, rule37675options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;

   // content:"|84|", fast_pattern:only;
   // if(contentMatch(p, rule37675options[1]->option_u.content, &cursor_normal) <= 0)
   //    return RULE_NOMATCH;
  
   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   // skip non-ESP marker for nat-tunneled ISAKMP
   if(sp->dst_port == 4500)
      cursor_normal += 4;

   // check if we can read ISAKMP header and first payload
   if(cursor_normal + 32 > end_of_buffer)
      return RULE_NOMATCH;

   // Check for IKEv1 (0x10) or IKEv2 (0x20)
   if((*(cursor_normal + 17) != 0x10) && (*(cursor_normal + 17) != 0x20))
      return RULE_NOMATCH;

   DEBUG_SO(fprintf(stderr,"ISAKMP Request:\n");)

   // move cursor to first payload type
   cursor_normal += 16;

   // read first payload type
   payload_type = *cursor_normal;

   // move to first payload
   cursor_normal += 12;

   // check up to 20 ISAKMP payloads 
   for(i = 0; i < 20; i++)
   {
      // We verify data availability above for first loop, below for subsequent loops
      next_payload_type = *cursor_normal;
      payload_length = read_big_16(cursor_normal + 2);

      DEBUG_SO(fprintf(stderr,"  payload: type 0x%02X len 0x%04X\n",payload_type,payload_length);)

      // Cisco-Fragmentation Payload (0x84)
      if(payload_type == 0x84)
      {
         // CVE-2016-1287:
         //  check for heap buffer overflow condition
         if(payload_length < 8)
            return RULE_MATCH;

         // CVE-2016-6381:
         //  check for memory exhaustion condition
         if(payload_length >= 36)
         {
            // verify we can read:
            //    Cisco Fragmentation Header  (8  bytes)
            //    ISAKMP Fragment             (28 bytes)
            if(cursor_normal + 36 > end_of_buffer)
               return RULE_NOMATCH;

            // verify we have a valid ISAKMP fragment version
            // and if ISAKMP frag length > INT32_MAX, alert.
            if((cursor_normal[25] == 0x10) || (cursor_normal[25] == 0x20))
               if((cursor_normal[32] & 0x80) == 0x80)
                  return RULE_MATCH;
         }
      }

      // no next payload, bail
      if(next_payload_type == 0)
         return RULE_NOMATCH;

      // calculate next payload position
      next_payload_pos = cursor_normal + payload_length;

      // integer overflow / zero-length payload check
      if(next_payload_pos <= cursor_normal)
         return RULE_NOMATCH;

      // check next payload
      payload_type = next_payload_type;
      cursor_normal = next_payload_pos;

      // verify we can read:
      //    next payload type (1 byte)
      //    critical          (1 byte)
      //    payload length    (2 byte BE)
      if(cursor_normal + 4 > end_of_buffer)
         return RULE_NOMATCH;
   }
   
   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule37675,
    NULL
};
*/
