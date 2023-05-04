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
int rule38347eval(void *p);

/* declare rule data structures */
/* flow:established, to_server; */
static FlowFlags rule38347flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule38347option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule38347flow0
   }
};

// file_data;
static CursorInfo rule38347file_data1 =
{
   0, /* offset */
   CONTENT_BUF_NORMALIZED /* flags */
};

static RuleOption rule38347option1 =
{
#ifndef MISSINGFILEDATA
   OPTION_TYPE_FILE_DATA,
#else
   OPTION_TYPE_SET_CURSOR,
#endif
   {
      &rule38347file_data1
   }
};

// content:"MZ", depth 0; 
static ContentInfo rule38347content2 = 
{
   (uint8_t *) "MZ", /* pattern */
   0, /* depth */
   0, /* offset */
   CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule38347option2 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule38347content2
   }
};

/* byte_jump:size 4, offset 58, relative, endian little; */
static ByteData rule38347byte_jump3 = 
{
   4, /* size */
   0, /* operator, byte_jump doesn't use operator! */
   0, /* value, byte_jump doesn't use value! */
   58, /* offset */
   0, /* multiplier */
   BYTE_LITTLE_ENDIAN|CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED|EXTRACT_AS_BYTE, /* flags */
   0 /* post_offset */
};

static RuleOption rule38347option3 = 
{
   OPTION_TYPE_BYTE_JUMP,
   {
      &rule38347byte_jump3
   }
};

// content:"PE|00 00|", offset -64, depth 4, relative; 
static ContentInfo rule38347content4 = 
{
   (uint8_t *) "PE|00 00|", /* pattern */
   4, /* depth */
   -64, /* offset */
   CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule38347option4 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule38347content4
   }
};

// content:".rsrc|00 00 00|", depth 0, fast_pattern; 
static ContentInfo rule38347content5 = 
{
   (uint8_t *) ".rsrc|00 00 00|", /* pattern */
   0, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN|CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule38347option5 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule38347content5
   }
};

/* references for sid 38347 */
/* reference: bugtraq "66002"; */
static RuleReference rule38347ref1 = 
{
   "bugtraq", /* type */
   "66002" /* value */
};

/* reference: cve "2014-2270"; */
static RuleReference rule38347ref2 = 
{
   "cve", /* type */
   "2014-2270" /* value */
};

/* reference: url "osvdb.org/show/osvdb/104081"; */
static RuleReference rule38347ref3 = 
{
   "url", /* type */
   "osvdb.org/show/osvdb/104081" /* value */
};

static RuleReference *rule38347refs[] =
{
   &rule38347ref1,
   &rule38347ref2,
   &rule38347ref3,
   NULL
};

/* metadata for sid 38347 */
/* metadata:service http, policy security-ips drop; */
static RuleMetaData rule38347service1 = 
{
   "service http"
};

static RuleMetaData rule38347policy1 = 
{
   "policy security-ips drop"
};

static RuleMetaData *rule38347metadata[] =
{
   &rule38347service1,
   &rule38347policy1,
   NULL
};

RuleOption *rule38347options[] =
{
   &rule38347option0,
   &rule38347option1,
   &rule38347option2,
   &rule38347option3,
   &rule38347option4,
   &rule38347option5,
   NULL
};

Rule rule38347 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_TCP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "$HTTP_PORTS", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      38347, /* sigid */
      1, /* revision */
      "attempted-admin", /* classification */
      0,  /* hardcoded priority */
      "FILE-EXECUTABLE PHP libmagic PE out of bounds memory access attempt",     /* message */
      rule38347refs, /* ptr to references */
      rule38347metadata /* ptr to metadata */
   },
   rule38347options, /* ptr to rule options */
   &rule38347eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule38347eval(void *p) {
   const uint8_t *cursor_normal = 0, *beg_of_buffer, *end_of_buffer;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   uint32_t raw_size, raw_addr;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   // flow:established, to_server;
   if(checkFlow(p, rule38347options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;

   // file_data;
   #ifndef MISSINGFILEDATA
   if(fileData(p, rule38347options[1]->option_u.cursor, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   #else
   if(setCursor(p, rule38347options[1]->option_u.cursor, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   #endif

   // content:"MZ", depth 0;
   if(contentMatch(p, rule38347options[2]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   // byte_jump:size 4, offset 58, relative, endian little;
   if(byteJump(p, rule38347options[3]->option_u.byte, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   // content:"PE|00 00|", offset -64, depth 4, relative;
   if(contentMatch(p, rule38347options[4]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   // content:".rsrc|00 00 00|", depth 0, fast_pattern;
   if(contentMatch(p, rule38347options[5]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_buffer, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   // skip VirtualSize (4 bytes) & VirtualAddr (4 bytes)
   cursor_normal += 8;

   // read RawSize (4 bytes) & RawAddr (4 bytes)
   if(cursor_normal + 8 > end_of_buffer)
      return RULE_NOMATCH;

   raw_size = read_little_32_inc(cursor_normal);
   raw_addr = read_little_32(cursor_normal);

   DEBUG_SO(fprintf(stderr,"PE RawSize 0x%08X RawAddr 0x%08X\n",raw_size,raw_addr);)

   if(raw_size + raw_addr > 0x7fffffff)
      return RULE_MATCH;

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule38347,
    NULL
};
*/
