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
int rule51369eval(void *p);

/* declare rule data structures */
/* flow:established, to_server; */
static FlowFlags rule51369flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule51369option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule51369flow0
   }
};

// content:"|03|", offset 0, depth 1; 
static ContentInfo rule51369content1 = 
{
   (uint8_t *) "|03|", /* pattern */
   1, /* depth */
   0, /* offset */
   CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule51369option1 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule51369content1
   }
};

// content:"|02 F0|", offset 3, depth 2, fast_pattern, relative; 
static ContentInfo rule51369content2 = 
{
   (uint8_t *) "|02 F0|", /* pattern */
   2, /* depth */
   3, /* offset */
   CONTENT_FAST_PATTERN|CONTENT_BUF_NORMALIZED|CONTENT_RELATIVE, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule51369option2 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule51369content2
   }
};

// content:"|E1|", offset 19, depth 1, relative; 
static ContentInfo rule51369content3 = 
{
   (uint8_t *) "|E1|", /* pattern */
   1, /* depth */
   19, /* offset */
   CONTENT_BUF_NORMALIZED|CONTENT_RELATIVE, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule51369option3 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule51369content3
   }
};

/* references for sid 51369 */
/* reference: cve "2019-1181"; */
static RuleReference rule51369ref1 = 
{
   "cve", /* type */
   "2019-1181" /* value */
};

/* reference: cve "2019-1182"; */
static RuleReference rule51369ref2 = 
{
   "cve", /* type */
   "2019-1182" /* value */
};

/* reference: url "portal.msrc.microsoft.com/en-us/security-guidance/advisory/CVE-2019-1181"; */
static RuleReference rule51369ref3 = 
{
   "url", /* type */
   "portal.msrc.microsoft.com/en-us/security-guidance/advisory/CVE-2019-1181" /* value */
};

/* reference: url "portal.msrc.microsoft.com/en-us/security-guidance/advisory/CVE-2019-1182"; */
static RuleReference rule51369ref4 = 
{
   "url", /* type */
   "portal.msrc.microsoft.com/en-us/security-guidance/advisory/CVE-2019-1182" /* value */
};

static RuleReference *rule51369refs[] =
{
   &rule51369ref1,
   &rule51369ref2,
   &rule51369ref3,
   &rule51369ref4,
   NULL
};

/* metadata for sid 51369 */
/* metadata:service rdp, policy balanced-ips drop, policy connectivity-ips drop, policy max-detect-ips drop, policy security-ips drop; */
static RuleMetaData rule51369service0 = 
{
   "service rdp"
};

static RuleMetaData rule51369policy1 = 
{
   "policy balanced-ips drop"
};

static RuleMetaData rule51369policy2 = 
{
   "policy connectivity-ips drop"
};

static RuleMetaData rule51369policy3 = 
{
   "policy max-detect-ips drop"
};

static RuleMetaData rule51369policy4 = 
{
   "policy security-ips drop"
};

static RuleMetaData *rule51369metadata[] =
{
   &rule51369service0,
   &rule51369policy1,
   &rule51369policy2,
   &rule51369policy3,
   &rule51369policy4,
   NULL
};

RuleOption *rule51369options[] =
{
   &rule51369option0,
   &rule51369option1,
   &rule51369option2,
   &rule51369option3,
   NULL
};

Rule rule51369 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_TCP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "3389", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      51369, /* sigid */
      2, /* revision */
      "attempted-admin", /* classification */
      0,  /* hardcoded priority */
      "OS-WINDOWS Microsoft Windows RDP DecompressUnchopper integer overflow attempt",     /* message */
      rule51369refs, /* ptr to references */
      rule51369metadata /* ptr to metadata */
   },
   rule51369options, /* ptr to rule options */
   &rule51369eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule51369eval(void *p) {
   SFSnortPacket *sp = (SFSnortPacket *) p;

   const uint8_t *beg_of_buffer, *end_of_buffer,
                 *cursor_tmp, *cursor_normal = 0;

   int i;
   uint16_t num_frames;
   uint32_t uncompressed_len, frame_len,
            total, check, bytes_left;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   // flow:established, to_server;
   if(checkFlow(p, rule51369options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;

   // match:
   //  |03| TPKT Version: 3

   // content:"|03|", offset 0, depth 1;
   if(contentMatch(p, rule51369options[1]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   // skip:
   //  reserved(1), length(2)
   // match:
   //  |02| X.224 Length: 2
   //  |F0| X.224 PDU Type: DT Data

   // content:"|02 F0|", offset 3, depth 2, fast_pattern, relative;
   if(contentMatch(p, rule51369options[2]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   // skip:
   //  flags(1), unknown(1), initiator(2),
   //  channel_id(2), priority(1), unknown(2),
   //  chan_pdu_len(4), chan_flags(4), unknown(2)
   //   = (19 bytes)
   // match:
   //  |E1| Multipart Frame Marker

   // content:"|E1|", offset 19, depth 1, relative;
   if(contentMatch(p, rule51369options[3]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_buffer, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   // check if we can read:
   //  num_frames       (2 bytes)
   //  uncompressed_len (4 bytes)
   if(cursor_normal + 6 > end_of_buffer)
      return RULE_NOMATCH;

   num_frames = read_little_16_inc(cursor_normal);
   uncompressed_len = read_little_32_inc(cursor_normal);

   // check for CVE-2019-1182
   if(uncompressed_len >= 0xFFFFE000)
      return RULE_MATCH;

   // limit num_frames
   if(num_frames > 25)
      num_frames = 25;

   for(total = 0, i = 0; i < num_frames; i++)
   {
      // check if we can read:
      //  frame_len (4 bytes)
      if(cursor_normal + 4 > end_of_buffer)
         return RULE_NOMATCH;

      frame_len = read_little_32(cursor_normal);

      // check for CVE-2019-1181
      check = frame_len + 4;

      if(check < frame_len)
         return RULE_MATCH;

      frame_len = check;

      // check for CVE-2019-1181
      check = total + frame_len;

      if(check < total)
         return RULE_MATCH;

      total = check;

      // integer overflow check
      bytes_left = end_of_buffer - cursor_normal;

      if(frame_len > bytes_left)
         return RULE_NOMATCH;

      // jump to next frame
      cursor_normal += frame_len;
   }

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule51369,
    NULL
};
*/
