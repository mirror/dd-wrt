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

#include <string.h>

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"

#include "so-util_dns.h"
#include "so-util.h"

/* declare detection functions */
int rule35942eval(void *p); // udp
int rule35943eval(void *p); // tcp
static int DetectBindTkeyDos(const uint8_t *cursor_normal, const uint8_t *end_of_buffer);

/* declare rule data structures */
/* flow:to_server; */
static FlowFlags rule35942flow0 = 
{
   FLOW_TO_SERVER
};

static RuleOption rule35942option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule35942flow0
   }
};

/* flow:established, to_server; */
static FlowFlags rule35943flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule35943option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule35943flow0
   }
};

#ifndef CONTENT_FAST_PATTERN_ONLY
#define CONTENT_FAST_PATTERN_ONLY CONTENT_FAST_PATTERN
#endif
// content:"|00 F9|", depth 0, fast_pattern:only; 
static ContentInfo rule_BindTkeyDos_content1 = 
{
   (uint8_t *) "|00 F9|", /* pattern */
   0, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN_ONLY|CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule_BindTkeyDos_option1 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule_BindTkeyDos_content1
   }
};

// content:"|00 01|", offset 4, depth 2; 
static ContentInfo rule35942content2 = 
{
   (uint8_t *) "|00 01|", /* pattern */
   2, /* depth */
   4, /* offset */
   CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule35942option2 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule35942content2
   }
};

// content:"|00 01|", offset 6, depth 2; 
static ContentInfo rule35943content2 = 
{
   (uint8_t *) "|00 01|", /* pattern */
   2, /* depth */
   6, /* offset */
   CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule35943option2 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule35943content2
   }
};

// content:"|00 00 00|", offset 2, depth 3, relative; 
static ContentInfo rule_BindTkeyDos_content3 = 
{
   (uint8_t *) "|00 00 00|", /* pattern */
   3, /* depth */
   2, /* offset */
   CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule_BindTkeyDos_option3 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule_BindTkeyDos_content3
   }
};

/* references */
/* reference: cve "2015-5477"; */
static RuleReference rule_BindTkeyDos_ref1 = 
{
   "cve", /* type */
   "2015-5477" /* value */
};

/* reference: url "kb.isc.org/article/AA-01272"; */
static RuleReference rule_BindTkeyDos_ref2 = 
{
   "url", /* type */
   "kb.isc.org/article/AA-01272" /* value */
};

static RuleReference *rule_BindTkeyDos_refs[] =
{
   &rule_BindTkeyDos_ref1,
   &rule_BindTkeyDos_ref2,
   NULL
};

/* metadata */
/* metadata:policy security-ips drop, service dns; */
static RuleMetaData rule_BindTkeyDos_service1 = 
{
   "service dns"
};

static RuleMetaData rule_BindTkeyDos_policy1 =
{
   "policy security-ips drop"
};

static RuleMetaData rule_BindTkeyDos_policy2 =
{
   "policy max-detect-ips drop"
};

static RuleMetaData *rule_BindTkeyDos_metadata[] =
{
   &rule_BindTkeyDos_service1,
   &rule_BindTkeyDos_policy1,
   &rule_BindTkeyDos_policy2,
   NULL
};

RuleOption *rule35942options[] =
{
   &rule35942option0,
   &rule_BindTkeyDos_option1,
   &rule35942option2,
   &rule_BindTkeyDos_option3,
   NULL
};

Rule rule35942 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_UDP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "53", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      35942, /* sigid */
      3, /* revision */
      "attempted-dos", /* classification */
      0,  /* hardcoded priority */
      "PROTOCOL-DNS ISC BIND TKEY query processing denial of service attempt",     /* message */
      rule_BindTkeyDos_refs, /* ptr to references */
      rule_BindTkeyDos_metadata /* ptr to metadata */
   },
   rule35942options, /* ptr to rule options */
   &rule35942eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

RuleOption *rule35943options[] =
{
   &rule35943option0,
   &rule_BindTkeyDos_option1,
   &rule35943option2,
   &rule_BindTkeyDos_option3,
   NULL
};

Rule rule35943 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_TCP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "53", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      35943, /* sigid */
      3, /* revision */
      "attempted-dos", /* classification */
      0,  /* hardcoded priority */
      "PROTOCOL-DNS ISC BIND TKEY query processing denial of service attempt",     /* message */
      rule_BindTkeyDos_refs, /* ptr to references */
      rule_BindTkeyDos_metadata /* ptr to metadata */
   },
   rule35943options, /* ptr to rule options */
   &rule35943eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

/* detection functions */
int rule35942eval(void *p) {
   const uint8_t *cursor_normal = 0, *end_of_buffer;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;
   
   // flow:to_server;
   if(checkFlow(p, rule35942options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;
   
   // content:"|00 01|", offset 4, depth 2;
   if(contentMatch(p, rule35942options[2]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   
   // content:"|00 00 00|", offset 2, depth 3, relative;
   if(contentMatch(p, rule35942options[3]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   // move cursor to flags
   cursor_normal += 2;

   return DetectBindTkeyDos(cursor_normal, end_of_buffer);
}

int rule35943eval(void *p) {
   const uint8_t *cursor_normal = 0, *end_of_buffer;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;
   
   // flow:established, to_server;
   if(checkFlow(p, rule35943options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;
   
   // content:"|00 01|", offset 6, depth 2;
   if(contentMatch(p, rule35943options[2]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   
   // content:"|00 00 00|", offset 2, depth 3, relative;
   if(contentMatch(p, rule35943options[3]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   // move cursor to flags
   // in the TCP case, flags are at offset 4
   cursor_normal += 4;

   return DetectBindTkeyDos(cursor_normal, end_of_buffer);
}

static int DetectBindTkeyDos(const uint8_t *cursor_normal, const uint8_t *end_of_buffer) {
   const uint8_t *query_name, *additional_name, *check;
   uint16_t flags, num_of_answers, answer_data_len,
            num_of_additional, additional_rr_type,
            additional_data_len;
   unsigned int i, query_name_len, additional_name_len;

   // check if we can read flags (2 bytes)
   // skip question number (2 bytes)
   // read answer number (2 bytes)
   // skip authority RR number (2 bytes)
   // and read additional RR number (2 bytes)
   if(cursor_normal + 10 > end_of_buffer)
      return RULE_NOMATCH;

   flags = read_big_16_inc(cursor_normal);

   // flags
   //
   // mask:
   // 0b1111101000001111 = 0xFA0F
   //   ^^   ^^^    ^   
   //   ||   |||    |   
   //   ||   |||    `- reply code (0000 = no error)
   //   ||   ||`- recursion and others
   //   ||   |`- truncated (0 = not truncated)
   //   ||   `- authoritative
   //   |`- opcode (0000 = standard query)
   //   `- response (0 = query)
   //
   if((flags & 0xFA0F) != 0)
      return RULE_NOMATCH;

   // skip question number (we limit it to 1)
   cursor_normal += 2;

   // get the number of answers
   num_of_answers = read_big_16_inc(cursor_normal);

   // if num_of_answers > 5, bail
   if(num_of_answers > 5)
      return RULE_NOMATCH;

   // skip authority RR number
   cursor_normal += 2;

   // get the number of additional RRs
   num_of_additional = read_big_16_inc(cursor_normal);

   // if num_of_additional > 5, bail
   if(num_of_additional > 5)
      return RULE_NOMATCH;

   // store start of query name
   query_name = cursor_normal;

   // skip question Name (we limit to 1)
   if(dns_skip_name(&cursor_normal, end_of_buffer) != DNS_SUCCESS)
      return RULE_NOMATCH;

   // store size of query name
   query_name_len = cursor_normal - query_name;

   // only compare up to 255 bytes 
   if(query_name_len > 255)
      return RULE_NOMATCH;

   // check that we can read the query type 
   if(cursor_normal + 2 > end_of_buffer)
      return RULE_NOMATCH;

   // verify that the query type is TKEY (0x00F9)
   // checked "backwards" to drop out faster since first byte is usually 0x00
   if((cursor_normal[1] != 0xF9) || (cursor_normal[0] != 0x00))
      return RULE_NOMATCH;

   // skip type & class
   cursor_normal += 4;

   // go to the end of the answer section (up to 5)
   for(i=0; i < num_of_answers; i++)
   {
      // skip answer
      if(dns_skip_name(&cursor_normal, end_of_buffer) != DNS_SUCCESS)
         return RULE_NOMATCH;

      // skip type, class, TTL
      cursor_normal += 8;

      // make sure we can read the answer data length
      if(cursor_normal + 2 > end_of_buffer)
         return RULE_NOMATCH;

      // read the answer data length
      answer_data_len = read_big_16_inc(cursor_normal);

      // jump the answer data length
      check = cursor_normal + answer_data_len;

      // integer overflow check
      if(check < cursor_normal)
         return RULE_NOMATCH;

      cursor_normal = check;
   }

   // parse additional RRs (up to 5)
   for(i=0; i < num_of_additional; i++)
   {
      // store start of additional name
      additional_name = cursor_normal;

      // skip Additional RR Name
      if(dns_skip_name(&cursor_normal, end_of_buffer) != DNS_SUCCESS)
         return RULE_NOMATCH;

      // calculate size of additional RR name
      additional_name_len = cursor_normal - additional_name;

      // verify we can read Type (2 bytes)
      // skip class & TTL (or EDNS data) (6 bytes)
      // and read data length (2 bytes)
      if(cursor_normal + 10 > end_of_buffer)
         return RULE_NOMATCH;

      // read the additional RR type
      additional_rr_type = read_big_16_inc(cursor_normal);

      // skip class & TTL (or EDNS data)
      cursor_normal += 6;

      // read additional RR data length
      additional_data_len = read_big_16_inc(cursor_normal);

      // jump the additional RR data length
      check = cursor_normal + additional_data_len;

      // integer overflow check
      if(check < cursor_normal)
         return RULE_NOMATCH;

      cursor_normal = check;

      // skip RR if type is:
      //  TKEY (0x00F9) (expected normal)
      //   -- or --
      //  OPT  (0x0029) (not in vulnerable msg->section)
      //  TSIG (0x00FA) (not in vulnerable msg->section)
      // (1st condition of vuln)
      if(additional_rr_type == 0x00F9 ||
         additional_rr_type == 0x0029 ||
         additional_rr_type == 0x00FA)
         continue;

      // if we skipped a pointer to the Query, then the
      // Additional RR Name is equal to the Query Name
      // (2nd condition of the vuln), alert.
      if(additional_name_len == 2)
         if((additional_name[0] == 0xC0) && (additional_name[1] == 0x0C))
            return RULE_MATCH;   

      // verify dns_skip_name skipped an Additional RR Name
      // with the same size as the Query Name
      //
      // (if the sizes are different, they can't be the same)
      if(query_name_len != additional_name_len)
         continue;

      // finally, verify the Additional RR Name is
      // equal to the Query Name for uncompressed names
      // (2nd condition of vuln), alert.
      if(memcmp(query_name, additional_name, query_name_len) == 0)
         return RULE_MATCH;
   }

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule35942,
    &rule35943,
    NULL
};
*/
