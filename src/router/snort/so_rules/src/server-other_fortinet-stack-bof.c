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
int rule34967eval(void *p);

/* declare rule data structures */
/* flow:established, to_server; */
static FlowFlags rule34967flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule34967option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule34967flow0
   }
};

// content:"|80 06|", offset 4, depth 2, fast_pattern; 
static ContentInfo rule34967content1 = 
{
   (uint8_t *) "|80 06|", /* pattern */
   2, /* depth */
   4, /* offset */
   CONTENT_FAST_PATTERN|CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule34967option1 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule34967content1
   }
};

/* references for sid 34967 */
/* reference: bugtraq "73206"; */
static RuleReference rule34967ref1 = 
{
   "bugtraq", /* type */
   "73206" /* value */
};

/* reference: cve "2015-2281"; */
static RuleReference rule34967ref2 = 
{
   "cve", /* type */
   "2015-2281" /* value */
};

/* reference: url "osvdb.org/show/osvdb/119719"; */
static RuleReference rule34967ref3 = 
{
   "url", /* type */
   "osvdb.org/show/osvdb/119719" /* value */
};

static RuleReference *rule34967refs[] =
{
   &rule34967ref1,
   &rule34967ref2,
   &rule34967ref3,
   NULL
};

/* metadata for sid 34967 */
/* metadata:policy security-ips drop; */
static RuleMetaData rule34967policy1 = 
{
   "policy security-ips drop"
};

/* metadata:policy max-detect-ips drop; */
static RuleMetaData rule34967policy2 =
{
   "policy max-detect-ips drop"
};

static RuleMetaData *rule34967metadata[] =
{
   &rule34967policy1,
   &rule34967policy2,
   NULL
};

RuleOption *rule34967options[] =
{
   &rule34967option0,
   &rule34967option1,
   NULL
};

Rule rule34967 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_TCP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "8000", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      34967, /* sigid */
      2, /* revision */
      "attempted-admin", /* classification */
      0,  /* hardcoded priority */
      "SERVER-OTHER Fortinet FSSO stack buffer overflow attempt",     /* message */
      rule34967refs, /* ptr to references */
      rule34967metadata /* ptr to metadata */
   },
   rule34967options, /* ptr to rule options */
   &rule34967eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule34967eval(void *p) {
   const uint8_t *cursor_normal = 0, *beg_of_buffer, *end_of_buffer, *check;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   int i;
   uint32_t chunk_len;
   uint8_t chunk_type;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;
   
   // flow:established, to_server;
   if(checkFlow(p, rule34967options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;
   
   // content:"|80 06|", offset 4, depth 2, fast_pattern;
   if(contentMatch(p, rule34967options[1]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_buffer, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   // check first 10 chunks
   for(i=0; i<10; i++)
   {
      // read chunk_len (4 bytes) & chunk_type (1 byte)
      if(cursor_normal + 5 > end_of_buffer)
         return RULE_NOMATCH;

      chunk_len = read_big_32(cursor_normal);
      chunk_type = *(cursor_normal + 4);

      // chunk_len must be at least 6
      // 4 byte chunk_size
      // 1 byte chunk_type
      // 1 byte chunk_data_type
      if(chunk_len < 6)
         return RULE_NOMATCH;

      DEBUG_SO(fprintf(stderr,"FSSO type:0x%02X len:0x%08X\n",chunk_type,chunk_len);)

      switch(chunk_type)
      {
         case 0x11:
            if(chunk_len > 0x38)
               return RULE_MATCH;
            break;
         case 0x12:
            if(chunk_len > 0x6C)
               return RULE_MATCH;
            break;
         case 0x13:
            if(chunk_len > 0x60)
               return RULE_MATCH;
            break;
         default:
            break;
      }

      // skip chunk
      check = cursor_normal + chunk_len;

      // overflow check
      if(check < cursor_normal)
         return RULE_NOMATCH;

      cursor_normal = check;
   }


   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule34967,
    NULL
};
*/
