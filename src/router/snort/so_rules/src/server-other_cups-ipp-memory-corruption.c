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

#define IPPERROR -1 

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"

#include "so-util.h"

/* declare detection functions */
int rule26972eval(void *p);

/* declare rule data structures */
/* flow:established, to_server; */
static FlowFlags rule26972flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule26972option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule26972flow0
   }
};

// content:"|01 01 00 0B|", offset 0, depth 4, fast_pattern, http_client_body; 
static ContentInfo rule26972content1 = 
{
   (uint8_t *) "|01 01 00 0B|", /* pattern */
   4, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN|CONTENT_BUF_POST, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule26972option1 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule26972content1
   }
};

/* references for sid 26972 */
/* reference: bugtraq "44530"; */
static RuleReference rule26972ref1 = 
{
   "bugtraq", /* type */
   "44530" /* value */
};

/* reference: cve "2010-2941"; */
static RuleReference rule26972ref2 = 
{
   "cve", /* type */
   "2010-2941" /* value */
};

/* reference: url "lists.apple.com/archives/security-announce/2010/Nov/msg00000.html"; */
static RuleReference rule26972ref3 = 
{
   "url", /* type */
   "lists.apple.com/archives/security-announce/2010/Nov/msg00000.html" /* value */
};

static RuleReference *rule26972refs[] =
{
   &rule26972ref1,
   &rule26972ref2,
   &rule26972ref3,
   NULL
};

/* metadata for sid 26972 */
/* metadata:service ipp, policy max-detect-ips drop; */
static RuleMetaData rule26972service0 = 
{
   "service ipp"
};

static RuleMetaData rule26972policy1 = 
{
   "policy max-detect-ips drop"
};

static RuleMetaData *rule26972metadata[] =
{
   &rule26972service0,
   &rule26972policy1,
   NULL
};

RuleOption *rule26972options[] =
{
   &rule26972option0,
   &rule26972option1,
   NULL
};

Rule rule26972 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_TCP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "631", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      26972, /* sigid */
      4, /* revision */
      "attempted-admin", /* classification */
      0,  /* hardcoded priority */
      "SERVER-OTHER CUPS IPP multi-valued attribute memory corruption attempt",     /* message */
      rule26972refs, /* ptr to references */
      rule26972metadata /* ptr to metadata */
   },
   rule26972options, /* ptr to rule options */
   &rule26972eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

typedef struct {
   uint8_t tag;
   uint16_t name_len;
   uint16_t value_len;
} IPPTAG;

static int parseipptag(const uint8_t **cursor_ref, const uint8_t *end_of_buffer, IPPTAG *tuple) {
   const uint8_t *temp_cursor; 
   // pass in the cursor by reference
   const uint8_t *cursor = *cursor_ref;

   // check if we can read: tag (1 byte) + name_len (2 bytes)
   if(cursor + 3 > end_of_buffer)
      return IPPERROR;

   // store the tag
   tuple->tag = *cursor++;

   // if the tag is another attribute-group (0x01)
   // or end-of-attributes (0x03), return error
   if((tuple->tag == 0x01) || (tuple->tag == 0x03))
      return IPPERROR;

   // store the name_len (2 bytes, BE)
   tuple->name_len  = read_big_16_inc(cursor);

   if(tuple->name_len > 0) {
      // jump the name_len, check for overflow
      temp_cursor = cursor + tuple->name_len;
      if(temp_cursor < cursor)
         return IPPERROR;
      cursor = temp_cursor;
   }

   // check if we can read: value_len (2 bytes)
   if(cursor + 2 > end_of_buffer)
      return IPPERROR;

   // store the value_len (2 bytes, BE)
   tuple->value_len  = read_big_16_inc(cursor);

   // jump the value_len, check for overflow
   temp_cursor = cursor + tuple->value_len;
   if(temp_cursor < cursor)
      return IPPERROR;
   cursor = temp_cursor;

   // store final cursor position
   *cursor_ref = cursor;

   // no error
   return 1;
}

//
// Set 1:
// 0x35, 0x36, 0x41, 0x42, 0x44..0x49
//
// Set 2:
// 0x37..0x40, 0x43
//

static int classifytag(uint8_t tag) {

   if(((tag >= 0x37) && (tag <= 0x40)) || (tag == 0x43))   /* 37-40, 43 */
      return 2;
   else if ((tag >= 0x35) && (tag <= 0x49)) /* 35, 36, 41, 42, 44-49, Note: 37-40, 43 caught above */
      return 1;
   else
      return IPPERROR;

}

/* detection functions */
int rule26972eval(void *p) {
   const uint8_t *cursor_normal = 0, *beg_of_buffer, *end_of_buffer;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   IPPTAG tuple;
   int i, base_class_type, additional_class_type;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   DEBUG_SO(fprintf(stderr,"rule26972 enter\n");)
   
   // flow:established, to_server;
   if(checkFlow(p, rule26972options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;
 
   // Match the version and operation-id  
   // content:"|01 01 00 0B|", offset 0, depth 4, fast_pattern, http_client_body;
   if(contentMatch(p, rule26972options[1]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   if(getBuffer(sp, CONTENT_BUF_POST, &beg_of_buffer, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   // skip request id (4 bytes)
   cursor_normal += 4;

   // attribute-group (1 byte)
   if(cursor_normal + 10 > end_of_buffer) // extra bytes for minimum size req'd to exploit
      return RULE_NOMATCH;

   // verify we have an attribute-group (0x01)
   if(*cursor_normal++ != 0x01)
      return RULE_NOMATCH;

   // Now we want to parse through the following structure:
   //
   //          Tag: 1 byte   |XX|
   //  Name Length: 2 bytes  |XX XX| = A
   //         Name: A bytes
   // Value Length: 2 bytes  |XX XX| = B
   //        Value: B bytes
   //         **optional**
   //          Tag: 1 byte   |XX|
   //  Name Length: 2 bytes  |XX XX| = C
   //  (if C == 0, same name as above, aka "additional-value")
   //               [...]
   //  0x03 (end of attributes) || 0x01 (attribute-group)
   if(parseipptag(&cursor_normal, end_of_buffer, &tuple) == IPPERROR)
      return RULE_NOMATCH;

   // the first name_length in a tag-name structure must not be 0
   // subsequent name_lengths may be 0, indicating additional values
   // for the attribute in the nearest preceding name see: RFC 2910
   if(tuple.name_len == 0)
      return RULE_NOMATCH;

   // classify the tag type, if we don't know the type, NOMATCH
   if((base_class_type = classifytag(tuple.tag)) == IPPERROR)
      return RULE_NOMATCH;
   DEBUG_SO(fprintf(stderr,"ipptag [0x%02x] (base class %d)\n",tuple.tag,base_class_type);)

   // check up to 25 tag-name structures
   for(i = 0; i < 25; i++)
   {
      if(parseipptag(&cursor_normal, end_of_buffer, &tuple) == IPPERROR)
         return RULE_NOMATCH;

      // if the name_length is not 0, we just parsed
      // a new 'base' structure, classify this and continue
      if(tuple.name_len != 0)
      {
         if((base_class_type = classifytag(tuple.tag)) == IPPERROR)
            return RULE_NOMATCH;
         DEBUG_SO(fprintf(stderr,"ipptag [0x%02x] (base class %d)\n",tuple.tag,base_class_type);)
         continue;
      }

      // classify the additional tag type
      if((additional_class_type = classifytag(tuple.tag)) == IPPERROR)
         return RULE_NOMATCH;
      DEBUG_SO(fprintf(stderr,"   ipptag [0x%02x] (class %d)\n",tuple.tag,additional_class_type);)

      // if the tuple class types differ
      // then the vulnerability condition
      // has been met, alert.
      if(base_class_type != additional_class_type)
         return RULE_MATCH;
   }

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule26972,
    NULL
};
*/
