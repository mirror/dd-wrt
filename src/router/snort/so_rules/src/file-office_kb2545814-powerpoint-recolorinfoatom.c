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
int rule18949eval(void *p);
int rule49939eval(void *p);
int DetectPPTMemoryCorruption(SFSnortPacket *sp);

/* declare rule data structures */
/* flow:established, to_client; */
static FlowFlags rule18949flow0 =
{
    FLOW_ESTABLISHED|FLOW_TO_CLIENT
};

static RuleOption rule18949option0 =
{
    OPTION_TYPE_FLOWFLAGS,
    {
        &rule18949flow0
    }
};

/* flow:established, to_server; */
static FlowFlags rule49939flow0 =
{
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule49939option0 =
{
    OPTION_TYPE_FLOWFLAGS,
    {
        &rule49939flow0
    }
};

/* flowbits:isset "file.ppt"; */
static FlowBitsInfo rule_PPTMemoryCorruption_flowbits1 =
{
    "file.ppt",
    FLOWBIT_ISSET,
    0,
};

static RuleOption rule_PPTMemoryCorruption_option1 =
{
    OPTION_TYPE_FLOWBIT,
    {
        &rule_PPTMemoryCorruption_flowbits1
    }
};

// file_data;
static CursorInfo rule_PPTMemoryCorruption_file_data2 =
{
   0, /* offset */
   CONTENT_BUF_NORMALIZED /* flags */
};

static RuleOption rule_PPTMemoryCorruption_option2 =
{
#ifndef MISSINGFILEDATA
   OPTION_TYPE_FILE_DATA,
#else
   OPTION_TYPE_SET_CURSOR,
#endif
   {
      &rule_PPTMemoryCorruption_file_data2
   }
};

// content:"|0F 00 11 F0|", depth 0, fast_pattern; 
static ContentInfo rule_PPTMemoryCorruption_content3 =
{
    (uint8_t *) "|0F 00 11 F0|", /* pattern (now in snort content format) */
    0, /* depth */
    0, /* offset */
    CONTENT_FAST_PATTERN|CONTENT_BUF_NORMALIZED|CONTENT_RELATIVE, /* flags */
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0 /* increment length*/
};

static RuleOption rule_PPTMemoryCorruption_option3 =
{
    OPTION_TYPE_CONTENT,
    {
        &rule_PPTMemoryCorruption_content3
    }
};

// content:"|00 00 E7 0F|", offset 0, depth 100, relative; 
static ContentInfo rule_PPTMemoryCorruption_content4 =
{
    (uint8_t *) "|00 00 E7 0F|", /* pattern (now in snort content format) */
    100, /* depth */
    0, /* offset */
    CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED, /* flags */
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0 /* increment length*/
};

static RuleOption rule_PPTMemoryCorruption_option4 =
{
    OPTION_TYPE_CONTENT,
    {
        &rule_PPTMemoryCorruption_content4
    }
};

/* references */
/* reference: cve "2011-1270"; */
static RuleReference rule_PPTMemoryCorruption_ref1 =
{
    "cve", /* type */
    "2011-1270" /* value */
};

/* reference: url "technet.microsoft.com/en-us/security/bulletin/MS11-036"; */
static RuleReference rule_PPTMemoryCorruption_ref2 =
{
    "url", /* type */
    "technet.microsoft.com/en-us/security/bulletin/MS11-036" /* value */
};

static RuleReference *rule_PPTMemoryCorruption_refs[] =
{
    &rule_PPTMemoryCorruption_ref1,
    &rule_PPTMemoryCorruption_ref2,
    NULL
};

/* metadata for sid 18949 */
/* metadata:policy max-detect-ips drop, service http; */
static RuleMetaData rule18949service1 =
{
   "service ftp-data"
};

static RuleMetaData rule18949service2 =
{
   "service http"
};

static RuleMetaData rule18949service3 =
{
   "service imap"
};

static RuleMetaData rule18949service4 =
{
   "service pop3"
};

static RuleMetaData rule18949policy1 =
{
   "policy max-detect-ips drop"
};

static RuleMetaData *rule18949metadata[] =
{
    &rule18949service1,
    &rule18949service2,
    &rule18949service3,
    &rule18949service4,
    &rule18949policy1,
    NULL
};

/* metadata for sid 49939 */
/* metadata:policy max-detect-ips drop, service http; */
static RuleMetaData rule49939service1 =
{
   "service smtp"
};

static RuleMetaData rule49939policy1 =
{
   "policy max-detect-ips drop"
};

static RuleMetaData *rule49939metadata[] =
{
    &rule49939service1,
    &rule49939policy1,
    NULL
};

RuleOption *rule18949options[] =
{
    &rule18949option0,
    &rule_PPTMemoryCorruption_option1,
    &rule_PPTMemoryCorruption_option2,
    &rule_PPTMemoryCorruption_option3,
    &rule_PPTMemoryCorruption_option4,
    NULL
};

Rule rule18949 = {
   /* rule header, akin to => tcp any any -> any any */
   {
       IPPROTO_TCP, /* proto */
       "$EXTERNAL_NET", /* SRCIP     */
       "$FILE_DATA_PORTS", /* SRCPORT   */
       0, /* DIRECTION */
       "$HOME_NET", /* DSTIP     */
       "any", /* DSTPORT   */
   },
   /* metadata */
   {
       3,  /* genid */
       18949, /* sigid */
       8, /* revision */
       "attempted-user", /* classification */
       0,  /* hardcoded priority */
       "FILE-OFFICE Microsoft Office PowerPoint malformed RecolorInfoAtom out of bounds read attempt",     /* message */
       rule_PPTMemoryCorruption_refs, /* ptr to references */
       rule18949metadata
   },
   rule18949options, /* ptr to rule options */
   &rule18949eval, /* DOES NOT use the built in detection function */
   0 /* am I initialized yet? */
};

RuleOption *rule49939options[] =
{
    &rule49939option0,
    &rule_PPTMemoryCorruption_option1,
    &rule_PPTMemoryCorruption_option2,
    &rule_PPTMemoryCorruption_option3,
    &rule_PPTMemoryCorruption_option4,
    NULL
};

Rule rule49939 = {
   /* rule header, akin to => tcp any any -> any any */
   {
       IPPROTO_TCP, /* proto */
       "$EXTERNAL_NET", /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       "$SMTP_SERVERS", /* DSTIP     */
       "25", /* DSTPORT   */
   },
   /* metadata */
   {
       3,  /* genid */
       49939, /* sigid */
       1, /* revision */
       "attempted-user", /* classification */
       0,  /* hardcoded priority */
       "FILE-OFFICE Microsoft Office PowerPoint malformed RecolorInfoAtom out of bounds read attempt",     /* message */
       rule_PPTMemoryCorruption_refs, /* ptr to references */
       rule49939metadata
   },
   rule49939options, /* ptr to rule options */
   &rule49939eval, /* DOES NOT use the built in detection function */
   0 /* am I initialized yet? */
};

/* detection functions */
int rule18949eval(void *p) {
   SFSnortPacket *sp = (SFSnortPacket *) p;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   // flow:established, to_client;
   if(checkFlow(p, rule18949options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;

   return DetectPPTMemoryCorruption(sp);
}

int rule49939eval(void *p) {
   SFSnortPacket *sp = (SFSnortPacket *) p;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   // flow:established, to_server;
   if(checkFlow(p, rule49939options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;

   return DetectPPTMemoryCorruption(sp);
}

int DetectPPTMemoryCorruption(SFSnortPacket *sp) {
   const uint8_t *cursor_normal = 0;
   const uint8_t *end_of_buffer, *cursor_detect;

   uint32_t header_len;
   uint16_t cColors, cFills;

   if(processFlowbits(sp, rule18949options[1]->option_u.flowBit) <= 0)
      return RULE_NOMATCH;

   // file_data;
   #ifndef MISSINGFILEDATA
   if(fileData(sp, rule18949options[2]->option_u.cursor, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   #else
   if(setCursor(sp, rule18949options[2]->option_u.cursor, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   #endif

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   // content:"|0F 00 11 F0|", depth 0, fast_pattern;
   while(contentMatch(sp, rule18949options[3]->option_u.content, &cursor_normal) > 0) {
      DEBUG_SO(fprintf(stderr,"DetectPPTMemoryCorruption first content passed\n");)

      cursor_detect = cursor_normal;

      // content:"|00 00 E7 0F|", offset 0, depth 100, relative;
      if(contentMatch(sp, rule18949options[4]->option_u.content, &cursor_detect) > 0) {
         DEBUG_SO(fprintf(stderr,"DetectPPTMemoryCorruption second content passed\n");)

         if(cursor_detect + 10 > end_of_buffer) 
            return RULE_NOMATCH;

         header_len = read_little_32_inc(cursor_detect);

         cursor_detect += 2; // Skip flags

         cColors = read_little_16_inc(cursor_detect);

         cFills = read_little_16_inc(cursor_detect);

         DEBUG_SO(fprintf(stderr,"header_len = 0x%04x, cColors = 0x%02x, cFills = 0x%02x\n", header_len, cColors, cFills);)

         // Int overflow not possible.  Max value 0x57FFB4
         if(header_len < (0x0c + (cColors + cFills) * 0x2c))
            return RULE_MATCH;
      }
   }
   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule18949,
    &rule49939,
    NULL
};
*/
