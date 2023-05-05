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
int rule38346eval(void *p);

/* declare rule data structures */
// ip_proto:132;
static HdrOptCheck rule38346ip_proto0 =
{
   IP_HDR_PROTO,
   CHECK_EQ,
   132,
   0,
   0
};

static RuleOption rule38346option0 =
{
   OPTION_TYPE_HDR_CHECK,
   {
      &rule38346ip_proto0
   }
};

// content:"|01|", offset 12, depth 1; 
static ContentInfo rule38346content1 = 
{
   (uint8_t *) "|01|", /* pattern */
   1, /* depth */
   12, /* offset */
   CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule38346option1 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule38346content1
   }
};

#ifndef CONTENT_FAST_PATTERN_ONLY
#define CONTENT_FAST_PATTERN_ONLY CONTENT_FAST_PATTERN
#endif
// content:"|C0 04|", depth 0, fast_pattern:only; 
static ContentInfo rule38346content2 = 
{
   (uint8_t *) "|C0 04|", /* pattern */
   0, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN_ONLY|CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule38346option2 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule38346content2
   }
};

/* references for sid 38346 */
/* reference: cve "2014-7841"; */
static RuleReference rule38346ref1 = 
{
   "cve", /* type */
   "2014-7841" /* value */
};

static RuleReference *rule38346refs[] =
{
   &rule38346ref1,
   NULL
};

/* metadata for sid 38346 */

static RuleMetaData rule38346policy1 =
{
    "policy max-detect-ips drop"
};

/* metadata:; */
static RuleMetaData *rule38346metadata[] =
{
   &rule38346policy1,
   NULL
};

RuleOption *rule38346options[] =
{
   &rule38346option0,
   &rule38346option1,
   &rule38346option2,
   NULL
};

Rule rule38346 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_IP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "any", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      38346, /* sigid */
      2, /* revision */
      "attempted-dos", /* classification */
      0,  /* hardcoded priority */
      "OS-LINUX Linux kernel SCTP INIT null pointer dereference attempt",     /* message */
      rule38346refs, /* ptr to references */
      rule38346metadata /* ptr to metadata */
   },
   rule38346options, /* ptr to rule options */
   &rule38346eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule38346eval(void *p) {
   const uint8_t *check, *start_of_buffer, *end_of_buffer, *cursor_normal = 0;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   int i;
   uint16_t ptype, plength, addr_type;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   // ip_proto:132;
   if(checkHdrOpt(p, rule38346options[0]->option_u.hdrData) <= 0)
      return RULE_NOMATCH;

   // SCTP Chunk Type INIT (1)
   // content:"|01|", offset 12, depth 1;
   if(contentMatch(p, rule38346options[1]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   
   if(getBuffer(p, CONTENT_BUF_NORMALIZED, &start_of_buffer, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   // RFC 4960 section 3.3.2
   // skip SCTP INIT header
   cursor_normal += 19;

   // parse up to 10 parameters (TLV 4 byte padded)
   for (i = 0; i < 10; i++)
   {
      // make sure we can read type and length (2 bytes each) plus
      // skip 8 bytes then read two more bytes inside the if() statement
      if(cursor_normal + 10 > end_of_buffer)
         return RULE_NOMATCH;

      ptype = read_big_16(cursor_normal); 
      DEBUG_SO(fprintf(stderr,"SCTP INIT parameter type:0x%04X\n", ptype);)

      // RFC 5061 section 4.2.4
      // check parameter (Set Primary Address)
      if(ptype == 0xC004)
      {
         // skip type (2 bytes), length (2 bytes), correlation ID (4 bytes)
         cursor_normal += 8;

         // read first address type, we verified data available before if() block
         addr_type = read_big_16(cursor_normal);

         // if first address type is invalid
         // e.g. NOT 0x0005 for ipv4 or 0x0006 for ipv6
         // then alert
         if(addr_type != 0x0005 && addr_type != 0x0006)
            return RULE_MATCH;

         // only check one Set Primary Address parameter
         return RULE_NOMATCH;
      }

      // No match, so jump to the next parameter
      // Note the cursor is only moved within the if() block, and we never return from it
      // Data availability is verified before the if() block.
      plength = read_big_16(cursor_normal+2);

      // (parameter length is 4 byte padded)
      check = cursor_normal + plength + (plength % 4);

      // overflow check
      if(check <= cursor_normal)
         return RULE_NOMATCH;

      cursor_normal = check;
   }

   return RULE_MATCH;
}

/*
Rule *rules[] = {
    &rule38346,
    NULL
};
*/
