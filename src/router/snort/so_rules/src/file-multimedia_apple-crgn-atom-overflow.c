/*
 * Copyright (C) 2005-2013 Sourcefire, Inc. All Rights Reserved
 *
 * Written by Sourcefire VRT
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

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"

//#define _DEBUG_
#ifdef _DEBUG_
#define DEBUG_WRAP(code) code
#else
#define DEBUG_WRAP(code)
#endif

/* declare detection functions */
int rule13897eval(void *p);

/* declare rule data structures */
/* flow:established, to_client; */
static FlowFlags rule13897flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_CLIENT
};

static RuleOption rule13897option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule13897flow0
   }
};

/* flowbits:isset "file.quicktime"; */
static FlowBitsInfo rule13897flowbits1 =
{
   "file.quicktime",
   FLOWBIT_ISSET,
   0,
};

static RuleOption rule13897option1 =
{
   OPTION_TYPE_FLOWBIT,
   {
      &rule13897flowbits1
   }
};

// file_data;
static CursorInfo rule13897file_data2 =
{
   0, /* offset */
   CONTENT_BUF_NORMALIZED /* flags */
};

static RuleOption rule13897option2 =
{
#ifndef MISSINGFILEDATA
   OPTION_TYPE_FILE_DATA,
#else
   OPTION_TYPE_SET_CURSOR,
#endif
   {
      &rule13897file_data2
   }
};

// content:"clip", depth 0, fast_pattern; 
static ContentInfo rule13897content3 = 
{
   (uint8_t *) "clip", /* pattern (now in snort content format) */
   0, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN|CONTENT_BUF_NORMALIZED|CONTENT_RELATIVE, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule13897option3 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule13897content3
   }
};

/* references for sid 13897 */
/* reference: bugtraq "28583"; */
static RuleReference rule13897ref1 = 
{
   "bugtraq", /* type */
   "28583" /* value */
};

/* reference: cve "2008-1017"; */
static RuleReference rule13897ref2 = 
{
   "cve", /* type */
   "2008-1017" /* value */
};

static RuleReference *rule13897refs[] =
{
   &rule13897ref1,
   &rule13897ref2,
   NULL
};

/* metadata for sid 13897 */
/* metadata:service http, service imap, service pop3, policy balanced-ips drop, policy security-ips drop; */
static RuleMetaData rule13897service1 = 
{
   "service http"
};

static RuleMetaData rule13897service2 = 
{
   "service imap"
};

static RuleMetaData rule13897service3 = 
{
   "service pop3"
};

static RuleMetaData rule13897policy1 = 
{
   "policy balanced-ips drop"
};

static RuleMetaData rule13897policy2 = 
{
   "policy security-ips drop"
};

static RuleMetaData rule13897policy3 = 
{
   "policy max-detect-ips drop"
};

static RuleMetaData *rule13897metadata[] =
{
   &rule13897service1,
   &rule13897service2,
   &rule13897service3,
   &rule13897policy1,
   &rule13897policy2,
   &rule13897policy3,
   NULL
};

RuleOption *rule13897options[] =
{
   &rule13897option0,
   &rule13897option1,
   &rule13897option2,
   &rule13897option3,
   NULL
};

Rule rule13897 = {
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
      13897, /* sigid */
      9, /* revision */
      "attempted-user", /* classification */
      0,  /* hardcoded priority */
      "FILE-MULTIMEDIA Apple Quicktime crgn atom parsing stack buffer overflow attempt",     /* message */
      rule13897refs, /* ptr to references */
      rule13897metadata /* ptr to metadata */
   },
   rule13897options, /* ptr to rule options */
   &rule13897eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule13897eval(void *p) {
   const uint8_t *cursor_normal = 0;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   const uint8_t *cursor_detect, *end_of_payload;
   uint32_t atom_size,crgn_atom_id;
   uint16_t region_size;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;
   
   // flow:established, to_client;
   if(checkFlow(p, rule13897options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;
   
   // flowbits:isset "file.quicktime";
   if(processFlowbits(p, rule13897options[1]->option_u.flowBit) <= 0)
      return RULE_NOMATCH;
   
   // file_data;
   #ifndef MISSINGFILEDATA
   if(fileData(p, rule13897options[2]->option_u.cursor, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   #else
   if(setCursor(p, rule13897options[2]->option_u.cursor, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   #endif
   
   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_payload) <= 0)
      return RULE_NOMATCH;

   // content:"clip", depth 0, fast_pattern;
   while(contentMatch(p, rule13897options[3]->option_u.content, &cursor_normal) > 0) {

     // Keep cursor_normal in case we need to loop again
     cursor_detect = cursor_normal;

     // check to see if we can read 10 bytes
     if(cursor_detect + 10 > end_of_payload)
        return RULE_NOMATCH;

     // read the atom_size UINT32
     atom_size  = (*cursor_detect++) << 24;   
     atom_size |= (*cursor_detect++) << 16;
     atom_size |= (*cursor_detect++) << 8;
     atom_size |= *cursor_detect++;
  
     // read the crgn_atom_id UINT32
     crgn_atom_id  = (*cursor_detect++) << 24;   
     crgn_atom_id |= (*cursor_detect++) << 16;
     crgn_atom_id |= (*cursor_detect++) << 8;
     crgn_atom_id |= *cursor_detect++; 
  
     // make sure we have required 'crgn'
     if(crgn_atom_id != 0x6372676E)
       continue;
  
     // read the region_size UINT16
     region_size  = (*cursor_detect++) << 8;
     region_size |= *cursor_detect;
  
     DEBUG_WRAP(fprintf(stderr,"REGION SIZE: %d\n",region_size);)
     DEBUG_WRAP(fprintf(stderr,"  ATOM SIZE: %d\n",atom_size);)
  
     // Changed from region_size > (atom_size - 8) to avoid
     // integer underflow from (atom_size - 8)
     if(((uint32_t)region_size + 8) > atom_size) 
       return RULE_MATCH;

      // We have a valid record, but didn't match the vulnerable condition, so skip ahead
      cursor_normal = cursor_detect;
   }

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule13897,
    NULL
};
*/
