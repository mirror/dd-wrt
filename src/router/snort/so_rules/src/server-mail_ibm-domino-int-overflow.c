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
#include <stdlib.h>

/* declare detection functions */
int rule42438eval(void *p);

/* declare rule data structures */
/* flow:established, to_server; */
static FlowFlags rule42438flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule42438option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule42438flow0
   }
};

// file_data;
static CursorInfo rule42438file_data1 =
{
   0, /* offset */
   CONTENT_BUF_NORMALIZED /* flags */
};

static RuleOption rule42438option1 =
{
#ifndef MISSINGFILEDATA
   OPTION_TYPE_FILE_DATA,
#else
   OPTION_TYPE_SET_CURSOR,
#endif
   {
      &rule42438file_data1
   }
};

// content:"BM", depth 2; 
static ContentInfo rule42438content2 = 
{
   (uint8_t *) "BM", /* pattern */
   2, /* depth */
   0, /* offset */
   CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule42438option2 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule42438content2
   }
};

// content:"|00 00 00 00|", offset 4, depth 4, relative, fast_pattern; 
static ContentInfo rule42438content3 = 
{
   (uint8_t *) "|00 00 00 00|", /* pattern */
   4, /* depth */
   4, /* offset */
   CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule42438option3 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule42438content3
   }
};

// content:"|28 00 00 00|", offset 4, depth 4, relative; 
static ContentInfo rule42438content4 = 
{
   (uint8_t *) "|28 00 00 00|", /* pattern */
   4, /* depth */
   4, /* offset */
   CONTENT_RELATIVE|CONTENT_FAST_PATTERN|CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule42438option4 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule42438content4
   }
};

/* references for sid 42438 */
/* reference: bugtraq "74597"; */
static RuleReference rule42438ref1 = 
{
   "bugtraq", /* type */
   "74597" /* value */
};

/* reference: cve "2015-1902"; */
static RuleReference rule42438ref2 = 
{
   "cve", /* type */
   "2015-1902" /* value */
};

/* reference: url "osvdb.org/show/osvdb/122079"; */
static RuleReference rule42438ref3 = 
{
   "url", /* type */
   "osvdb.org/show/osvdb/122079" /* value */
};

static RuleReference *rule42438refs[] =
{
   &rule42438ref1,
   &rule42438ref2,
   &rule42438ref3,
   NULL
};

/* metadata for sid 42438 */
/* metadata:service smtp, policy balanced-ips drop, policy security-ips drop; */
static RuleMetaData rule42438service1 = 
{
   "service smtp"
};

static RuleMetaData rule42438policy1 = 
{
   "policy balanced-ips drop"
};

static RuleMetaData rule42438policy2 = 
{
   "policy security-ips drop"
};

static RuleMetaData *rule42438metadata[] =
{
   &rule42438service1,
   &rule42438policy1,
   &rule42438policy2,
   NULL
};

RuleOption *rule42438options[] =
{
   &rule42438option0,
   &rule42438option1,
   &rule42438option2,
   &rule42438option3,
   &rule42438option4,
   NULL
};

Rule rule42438 = {
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
      42438, /* sigid */
      2, /* revision */
      "attempted-admin", /* classification */
      0,  /* hardcoded priority */
      "SERVER-MAIL IBM Domino BMP parsing integer overflow attempt",     /* message */
      rule42438refs, /* ptr to references */
      rule42438metadata /* ptr to metadata */
   },
   rule42438options, /* ptr to rule options */
   &rule42438eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule42438eval(void *p) {
   const uint8_t *cursor_normal = 0, *beg_of_buffer, *end_of_buffer;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   uint32_t biWidth, biHeight;
   uint16_t biBitCount;

   uint64_t check;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;
   
   // flow:established, to_server;
   if(checkFlow(p, rule42438options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;
   
   // file_data;
   #ifndef MISSINGFILEDATA
   if(fileData(p, rule42438options[1]->option_u.cursor, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   #else
   if(setCursor(p, rule42438options[1]->option_u.cursor, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   #endif
   
   // content:"BM", depth 2;
   if(contentMatch(p, rule42438options[2]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   
   // content:"|00 00 00 00|", offset 4, depth 4, relative;
   if(contentMatch(p, rule42438options[3]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   
   // content:"|28 00 00 00|", offset 4, depth 4, relative, fast_pattern;
   if(contentMatch(p, rule42438options[4]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_buffer, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   // check if we can read
   // biWidth (4 bytes)
   // biHeight (4 bytes)
   // skip biPlanes (2 bytes)
   // biBitCount (2 bytes)
   //
   if(cursor_normal + 12 > end_of_buffer)
      return RULE_NOMATCH;

   biWidth = read_little_32_inc(cursor_normal);
   biHeight = read_little_32_inc(cursor_normal);

   // handle bitmap orientation 
   biWidth = (uint32_t)abs((int)biWidth);
   biHeight = (uint32_t)abs((int)biHeight);

   // skip biPlanes
   cursor_normal += 2;

   biBitCount = read_little_16(cursor_normal);

   // check for integer overflow condition
   check = (uint64_t)(biWidth & 0xFFFF) * (biBitCount >> 3) * biHeight;

   DEBUG_SO(fprintf(stderr,"check: %lu\n",check);)

   if(check > UINT32_MAX)
      return RULE_MATCH;

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule42438,
    NULL
};
*/
