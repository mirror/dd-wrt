/*
 * WEB-CLIENT Sun JDK image parsing library ICC buffer overflow attempt
 * 
 * Copyright (C) 2007 Sourcefire, Inc. All Rights Reserved
 * 
 * Writen by Patrick Mullen <pmullen@sourcefire.com>
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

#include "so-util.h"

/* declare detection functions */
int rule15328eval(void *p);

static RuleReference rule15328ref0 = 
{
    "url", /* type */
    "scary.beasts.org/security/CESA-2006-004.html" /* value */
};
static RuleReference rule15328ref1 =
{
    "cve", /* type */
    "2007-2788"
};
static RuleReference rule15328ref2 =
{
    "bugtraq", /* type */
    "24004"
};

static RuleReference *rule15328refs[] =
{
    &rule15328ref0,
    &rule15328ref1,
    &rule15328ref2,
    NULL
};

static FlowFlags rule15328flow =
{
    FLOW_ESTABLISHED|FLOW_TO_CLIENT
};

static RuleOption rule15328option0 =
{
    OPTION_TYPE_FLOWFLAGS,
    {
        &rule15328flow
    }
};

static ContentInfo rule15328content0 =
{
    (uint8_t *)"ICC_PROFILE|00|",      /* pattern to search for */
    0,                      /* depth */
    0,                      /* offset */
    CONTENT_BUF_NORMALIZED,                      /* flags */
    NULL,                   /* holder for boyer/moore info */
    NULL,                   /* holder for byte representation of "NetBus" */
    0,                      /* holder for length of byte representation */
    0                       /* holder of increment length */
};

static RuleOption rule15328option1 =
{
    OPTION_TYPE_CONTENT,
    {
        &rule15328content0
    }
};

static ContentInfo rule15328content1 =
{
    (uint8_t *)"|FF E2|",              /* pattern to search for */
    2,                      /* depth */
    -16,                    /* offset */
    CONTENT_BUF_NORMALIZED | CONTENT_RELATIVE,         /* flags */
    NULL,                   /* holder for boyer/moore info */
    NULL,                   /* holder for byte representation of "NetBus" */
    0,                      /* holder for length of byte representation */
    0                       /* holder of increment length */
};

static RuleOption rule15328option2 =
{
    OPTION_TYPE_CONTENT,
    {
        &rule15328content1
    }
};

static ContentInfo rule15328content2 =
{
    (uint8_t *)"acsp",                 /* pattern to search for */
    4,                      /* depth */
    52,                     /* offset */
    CONTENT_BUF_NORMALIZED | CONTENT_RELATIVE,         /* flags */
    NULL,                   /* holder for boyer/moore info */
    NULL,                   /* holder for byte representation of "NetBus" */
    0,                      /* holder for length of byte representation */
    0                       /* holder of increment length */
};

static RuleOption rule15328option3 =
{
    OPTION_TYPE_CONTENT,
    {
        &rule15328content2
    }
};

/* Search to find additional ICC chunks */
static ContentInfo rule15328content3 =
{
    (uint8_t *)"ICC_PROFILE|00|",      /* pattern to search for */
    0,                      /* depth */
    0,                      /* offset */
    CONTENT_BUF_NORMALIZED | CONTENT_RELATIVE,         /* flags */
    NULL,                   /* holder for boyer/moore info */
    NULL,                   /* holder for byte representation of "NetBus" */
    0,                      /* holder for length of byte representation */
    0                       /* holder of increment length */
};

static RuleOption rule15328option4 =
{
    OPTION_TYPE_CONTENT,
    {
        &rule15328content3
    }
};

/* metadata for sid 15328 */
/* metadata:policy max-detect-ips drop; */
static RuleMetaData rule15328policy1 = 
{
   "policy max-detect-ips drop"
};

static RuleMetaData *rule15328metadata[] =
{
   &rule15328policy1,
   NULL
};

RuleOption *rule15328options[] =
{
    &rule15328option0,
    &rule15328option1,
    &rule15328option2,
    &rule15328option3,
    &rule15328option4,
    NULL
};

Rule rule15328 = {
   /* rule header */
   {
       IPPROTO_TCP, /* proto */
       HOME_NET, /* SRCIP     */
       "80", /* SRCPORT   */
       0, /* DIRECTION */
       EXTERNAL_NET, /* DSTIP     */
       "any", /* DSTPORT   */
   },
   /* metadata */
   { 
       3,  /* genid (HARDCODED!!!) */
       15328, /* sigid */
       6, /* revision  */
   
       "attempted-user", /* classification, generic */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "FILE-JAVA Sun JDK image parsing library ICC buffer overflow attempt",     /* message */
       rule15328refs, /* ptr to references */
       rule15328metadata /* ptr to metadata */
   },
   rule15328options, /* ptr to rule options */
   &rule15328eval, /* ptr to rule detection function */
   0, /* am I initialized yet? */
   0, /* number of options */
   0  /* don't alert */
};


/* detection function */

int rule15328eval(void *p) {
   /* data pointers */
   const uint8_t *cursor_normal, *beg_of_payload, *end_of_payload, *end_of_segment, *after_icc_profile_string;

   /* data */
   /* uint8_t chunk_num, num_chunks; */ /* For multiple chunks (not implemented) */
   uint16_t segment_length;
   uint32_t tagcount, tagsize;

   /* cruft */
   int i;

   SFSnortPacket *sp = (SFSnortPacket *) p;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_payload, &end_of_payload) <= 0)
      return RULE_NOMATCH;

   if((end_of_payload - beg_of_payload) < 0xA4)   /* Minimum packet length to have enough data */
      return RULE_NOMATCH;    

   /* call flow match */
   if(checkFlow(sp, rule15328options[0]->option_u.flowFlags) <= 0 )
      return RULE_NOMATCH;

   /* call content match "ICC_PROFILE|00|" */
   if(contentMatch(sp, rule15328options[1]->option_u.content, &cursor_normal) <= 0) {
      return RULE_NOMATCH;
   }

   if(cursor_normal < beg_of_payload + 16) {
      // There isn't enough room to find the segment header, so search for a new "ICC_PROFILE|00|"
      if(contentMatch(sp, rule15328options[4]->option_u.content, &cursor_normal) <= 0)
         return RULE_NOMATCH;
   }

   /* We do a do..while loop to reuse all of the same code.  The contentMatch for the while()
      is just like the initial contentMatch except it's relative.
   */
   do {
      after_icc_profile_string = cursor_normal;  /* stored for a pointer reset if needed later */

      // 1+1+0x80+4+12 for chunk num, num of chunks, ICC header, tag count, first tag
      if(cursor_normal + 0x92 > end_of_payload) 
         return RULE_NOMATCH;

      /* call content match "|FF E2|" */
      if(contentMatch(sp, rule15328options[2]->option_u.content, &cursor_normal) <= 0)
         continue;   // Loop again to see if we can find a new ICC_PROFILE

      /* Segment length is right after segment identifier, 16-bit, little-endian */
      segment_length = read_little_16(cursor_normal);

      /* length includes length field but not the segment identifier */
      end_of_segment = cursor_normal + segment_length; 
                                      
      /* call content match "ascp" */
      if(contentMatch(sp, rule15328options[3]->option_u.content, &cursor_normal) <= 0) {
         /* If we don't find "ascp", we need to move the cursor back to where it was after
            "ICC_PROFILE|00|" to prevent an infinite loop.  This is preferable to restructuring
            the loop because we really want to search on "ICC..." because it's a faster search
            and has way less false matches than "|FF E2|".
         */
         cursor_normal = after_icc_profile_string; 
         continue;   // Loop again to see if we can find a new ICC_PROFILE
      }

      /* At this point, we definitely have an ICC profile */

      /* Get the chunk number and total number of chunks */
      /* The content match above ensures the reads below are in the payload */
      /* NOTE: We don't handle multiple chunks.  This is left here in case later
       * we do.  
       * chunk_num = *(cursor_normal - 42);
       * num_chunks = *(cursor_normal - 41);
      */

      /* Get the Tag Count */
      tagcount = ntohl(*((uint32_t*)(cursor_normal + 88)));

      /* Point cursor_normal at the tag table */
      cursor_normal += 92;

      /* This does not account for tag tables that span icc profile chunks! */
      for(i=0; (i < tagcount) && (cursor_normal + 12 < end_of_payload) && 
                                 (cursor_normal + 12 < end_of_segment); i++) {
         tagsize = ntohl(*((uint32_t*)(cursor_normal + 8)));

         if(tagsize > 0xFFFFFFF7)
            return RULE_MATCH;

         cursor_normal += 12;  // Next tag
      }

   /* Try to find another "ICC_PROFILE|00|" string and loop again */
   } while(contentMatch(sp, rule15328options[4]->option_u.content, &cursor_normal) > 0);

   return RULE_NOMATCH;
}      
            

/*
Rule *rules[] = {
    &rule15328,
    NULL
};
*/
