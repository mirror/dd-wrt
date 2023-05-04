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
int rule15433eval(void *p);
int rule52444eval(void *p);
static int DetectWinampIntOverflow(SFSnortPacket *sp);
static int CheckStringBlock(const uint8_t **cursor, const uint8_t *end);

/* declare rule data structures */
/* flow:established, to_client; */
static FlowFlags rule15433flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_CLIENT
};

static RuleOption rule15433option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule15433flow0
   }
};

/* flow:established, to_server; */
static FlowFlags rule52444flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule52444option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule52444flow0
   }
};

// file_data;
static CursorInfo rule_WinampIntOverflow_file_data1 =
{
   0, /* offset */
   CONTENT_BUF_NORMALIZED /* flags */
};

static RuleOption rule_WinampIntOverflow_option1 =
{
#ifndef MISSINGFILEDATA
   OPTION_TYPE_FILE_DATA,
#else
   OPTION_TYPE_SET_CURSOR,
#endif
   {
      &rule_WinampIntOverflow_file_data1
   }
};

// content:"FG|03 04 17 00 00 00|", offset 0, depth 8, fast_pattern; 
static ContentInfo rule_WinampIntOverflow_content2 = 
{
   (uint8_t *) "FG|03 04 17 00 00 00|", /* pattern */
   8, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN|CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule_WinampIntOverflow_option2 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule_WinampIntOverflow_content2
   }
};

// byte_jump:size 4, offset 0, multiplier 16, relative, endian little;
static ByteData rule_WinampIntOverflow_byte_jump3 = 
{
   4, /* size */
   0, /* operator, byte_jump doesn't use operator! */
   0, /* value, byte_jump doesn't use value! */
   0, /* offset */
   16, /* multiplier */
   BYTE_LITTLE_ENDIAN|CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED|EXTRACT_AS_BYTE, /* flags */
   0, /* post_offset */
   NULL, /* offset_refId */
   NULL, /* value_refId */
   NULL, /* offset_location */
   NULL  /* value_location */
};

static RuleOption rule_WinampIntOverflow_option3 = 
{
   OPTION_TYPE_BYTE_JUMP,
   {
      &rule_WinampIntOverflow_byte_jump3
   }
};

// byte_jump:size 4, offset 0, multiplier 14, relative, endian little;
static ByteData rule_WinampIntOverflow_byte_jump4 = 
{
   4, /* size */
   0, /* operator, byte_jump doesn't use operator! */
   0, /* value, byte_jump doesn't use value! */
   0, /* offset */
   14, /* multiplier */
   BYTE_LITTLE_ENDIAN|CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED|EXTRACT_AS_BYTE, /* flags */
   0, /* post_offset */
   NULL, /* offset_refId */
   NULL, /* value_refId */
   NULL, /* offset_location */
   NULL  /* value_location */
};

static RuleOption rule_WinampIntOverflow_option4 = 
{
   OPTION_TYPE_BYTE_JUMP,
   {
      &rule_WinampIntOverflow_byte_jump4
   }
};

/* references */
/* reference: bugtraq "35052"; */
static RuleReference rule_WinampIntOverflow_ref1 = 
{
   "bugtraq", /* type */
   "35052" /* value */
};

/* reference: cve "2009-1831"; */
static RuleReference rule_WinampIntOverflow_ref2 = 
{
   "cve", /* type */
   "2009-1831" /* value */
};

static RuleReference *rule_WinampIntOverflow_refs[] =
{
   &rule_WinampIntOverflow_ref1,
   &rule_WinampIntOverflow_ref2,
   NULL
};

/* metadata for sid 15433 */
/* metadata:service ftp-data, service http, service imap, service pop3, policy max-detect-ips drop; */
static RuleMetaData rule15433service0 = 
{
   "service ftp-data"
};

static RuleMetaData rule15433service1 = 
{
   "service http"
};

static RuleMetaData rule15433service2 = 
{
   "service imap"
};

static RuleMetaData rule15433service3 = 
{
   "service pop3"
};

static RuleMetaData rule15433policy4 = 
{
   "policy max-detect-ips drop"
};

static RuleMetaData *rule15433metadata[] =
{
   &rule15433service0,
   &rule15433service1,
   &rule15433service2,
   &rule15433service3,
   &rule15433policy4,
   NULL
};

/* metadata for sid 52444 */
/* metadata:service smtp, policy max-detect-ips drop; */
static RuleMetaData rule52444service0 = 
{
   "service smtp"
};

static RuleMetaData rule52444policy1 = 
{
   "policy max-detect-ips drop"
};

static RuleMetaData *rule52444metadata[] =
{
   &rule52444service0,
   &rule52444policy1,
   NULL
};

RuleOption *rule15433options[] =
{
   &rule15433option0,
   &rule_WinampIntOverflow_option1,
   &rule_WinampIntOverflow_option2,
   &rule_WinampIntOverflow_option3,
   &rule_WinampIntOverflow_option4,
   NULL
};

RuleOption *rule52444options[] =
{
   &rule52444option0,
   &rule_WinampIntOverflow_option1,
   &rule_WinampIntOverflow_option2,
   &rule_WinampIntOverflow_option3,
   &rule_WinampIntOverflow_option4,
   NULL
};

Rule rule15433 = {
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
      15433, /* sigid */
      8, /* revision */
      "attempted-user", /* classification */
      0,  /* hardcoded priority */
      "FILE-OTHER Winamp MAKI parsing integer overflow attempt",     /* message */
      rule_WinampIntOverflow_refs, /* ptr to references */
      rule15433metadata /* ptr to metadata */
   },
   rule15433options, /* ptr to rule options */
   &rule15433eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

Rule rule52444 = {
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
      52444, /* sigid */
      1, /* revision */
      "attempted-user", /* classification */
      0,  /* hardcoded priority */
      "FILE-OTHER Winamp MAKI parsing integer overflow attempt",     /* message */
      rule_WinampIntOverflow_refs, /* ptr to references */
      rule52444metadata /* ptr to metadata */
   },
   rule52444options, /* ptr to rule options */
   &rule52444eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule15433eval(void *p) {
   SFSnortPacket *sp = (SFSnortPacket *) p;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;
   
   // flow:established, to_client;
   if(checkFlow(p, rule15433options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;

   return DetectWinampIntOverflow(sp);
}

int rule52444eval(void *p) {
   SFSnortPacket *sp = (SFSnortPacket *) p;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;
   
   // flow:established, to_server;
   if(checkFlow(p, rule52444options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;

   return DetectWinampIntOverflow(sp);
}

static int DetectWinampIntOverflow(SFSnortPacket *sp) {
   const uint8_t *cursor_normal = 0, *beg_of_buffer, *end_of_buffer;
   int result;

   // file_data;
   #ifndef MISSINGFILEDATA
   if(fileData(sp, rule15433options[1]->option_u.cursor, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   #else
   if(setCursor(sp, rule15433options[1]->option_u.cursor, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   #endif

   // content:"FG|03 04 17 00 00 00|", offset 0, depth 8, fast_pattern;
   if(contentMatch(sp, rule15433options[2]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   // byte_jump:size 4, offset 0, multiplier 16, relative, endian little;
   if(byteJump(sp, rule15433options[3]->option_u.byte, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_buffer, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   // check function names
   result = CheckStringBlock(&cursor_normal, end_of_buffer);

   if(result < 0)
      return RULE_NOMATCH;
   else if(result == RULE_MATCH)
      return RULE_MATCH;

   // skip the data block
   // byte_jump:size 4, offset 0, multiplier 14, relative, endian little;
   if(byteJump(sp, rule15433options[4]->option_u.byte, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   // check general strings
   if(CheckStringBlock(&cursor_normal, end_of_buffer) == RULE_MATCH)
      return RULE_MATCH;

   return RULE_NOMATCH;
}

static int CheckStringBlock(const uint8_t **cursor, const uint8_t *end) {
   const uint8_t *cursor_temp;
   uint32_t str_count, i;
   uint16_t str_len;

   cursor_temp = *cursor;

   if(cursor_temp + 4 <= end)
   {
      str_count = read_little_32_inc(cursor_temp);

      DEBUG_SO(fprintf(stderr,"str_count 0x%08x\n", str_count);)

      for(i = 0; (i < str_count) && (cursor_temp + 6 <= end); i++)
      {
         cursor_temp += 4; // jump to the string length field

         str_len = read_little_16_inc(cursor_temp);

         DEBUG_SO(fprintf(stderr,"str_len 0x%04x\n", str_len);)
   
         if(str_len >= 0x8000) // malicious condition
            return RULE_MATCH;

         // check if we can jump str_len
         if(str_len > end - cursor_temp)
            return -1;

         cursor_temp += str_len;
      }
   }

   *cursor = cursor_temp;

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule15433,
    &rule52444,
    NULL
};
*/
