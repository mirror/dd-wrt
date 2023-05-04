/*
 * Altrium Software MERCUR IMAPD NTLMSSP Command Handling memory corruption attempt
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

#include "so-util_base64-decode.h"

/* declare detection functions */
static int rule13921eval(void *);

/* declare references */
static RuleReference rule13921ref0 = {
    "url", /* type */
    "secunia.com/advisories/24596" /* value */
};

static RuleReference rule13921bugtraq = {
    "bugtraq", /* type */
    "23058"
};

static RuleReference rule13921cve = {
    "cve", /* type */
    "2007-1578"
};

static RuleReference *rule13921refs[] = {
    &rule13921ref0,
    &rule13921bugtraq,
    &rule13921cve,
    NULL
};

/* Declare rule options */
static FlowFlags rule13921flow = {
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule13921option0 = {
    OPTION_TYPE_FLOWFLAGS,
    {
        &rule13921flow
    }
};

static ContentInfo rule13921content1 = {
    (uint8_t *)"TlRMTVNT", /* pattern to search for */
    8,                      /* depth */
    0,                      /* offset */
    CONTENT_BUF_NORMALIZED, /* flags */
    NULL,                   /* holder for boyer/moore info */
    NULL,                   /* holder for byte representation of something */
    0,                      /* holder for length of byte representation */
    0                       /* holder of increment length */
};

static RuleOption rule13921option1 = {
    OPTION_TYPE_CONTENT,
    {
        &rule13921content1
    }
};

static ContentInfo rule13921content2 = {
    (uint8_t *)"AAAA",     /* pattern to search for */
    4,                      /* depth */
    4,                      /* offset */
    CONTENT_BUF_NORMALIZED | CONTENT_RELATIVE,       /* flags */
    NULL,                   /* holder for boyer/moore info */
    NULL,                   /* holder for byte representation of something */
    0,                      /* holder for length of byte representation */
    0                       /* holder of increment length */
};

static RuleOption rule13921option2 = {
    OPTION_TYPE_CONTENT,
    {
        &rule13921content2
    }
};


RuleOption *rule13921options[] = {
    &rule13921option0,
    &rule13921option1,
    &rule13921option2, 
    NULL
};

static RuleMetaData rule13921service1 =
{
    "service imap"
};

static RuleMetaData rule13921policy1 =
{
    "policy max-detect-ips drop"
};

static RuleMetaData *rule13921metadata[] =
{
    &rule13921service1,
    &rule13921policy1,
    NULL
};

/* Rule definition */

Rule rule13921 = {
   /* rule header */
   {
       IPPROTO_TCP, /* proto */
       "any", /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       HOME_NET, /* DSTIP */
       "143", /* DSTPORT */
   },
   /* metadata */
   { 
       3,  /* genid (HARDCODED!!!) */
       13921, /* sigid */
       7, /* revision */
       "attempted-admin", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "SERVER-MAIL Altrium Software MERCUR IMAPD NTLMSSP command handling memory corruption attempt",     /* message */
       rule13921refs /* ptr to references */
       ,rule13921metadata 
   },
   rule13921options, /* ptr to rule options */
   &rule13921eval, /* ptr to rule detection function */
   0, /* am I initialized yet? */
   0, /* number of options */
   0  /* don't alert */
};


/* Detection functions */

static int rule13921eval(void *p) {
   SFSnortPacket *sp = (SFSnortPacket *)p;

   const uint8_t *cursor, *beg_of_payload, *end_of_payload;

   int16_t lm_x;  

   int n;  /* cruft */

   /* Data for holding our base64 data */
   uint8_t decoded_data[16];
   uint32_t num_bytes_extracted;


   /* General sanity checking */
  if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_payload, &end_of_payload) <= 0)
      return RULE_NOMATCH;

   if((end_of_payload - beg_of_payload) < 32)
      return RULE_NOMATCH;

   /* call flow match */
   if (checkFlow(sp, rule13921options[0]->option_u.flowFlags) <= 0 )
      return RULE_NOMATCH;

   /* call first content match */
   if (contentMatch(sp, rule13921options[1]->option_u.content, &cursor) <= 0) {
      return RULE_NOMATCH;
   }

   /* call second content match */
   if (contentMatch(sp, rule13921options[2]->option_u.content, &cursor) <= 0) {
      return RULE_NOMATCH;
   }

   /* Decode the part containing "/P.\x03/" to ensure the proper header and message type */
   n = base64decode(&(beg_of_payload[8]), 4, decoded_data, sizeof(decoded_data), &num_bytes_extracted);

   if((n < 0) || (num_bytes_extracted < 3))
      return RULE_NOMATCH;

   /* verify contents */
   if((decoded_data[0] != 'P') || (decoded_data[2] != 0x03))
      return RULE_NOMATCH;

   /* Now decode the part containing LM_X */
   n = base64decode(&(beg_of_payload[24]), 8, decoded_data, sizeof(decoded_data), &num_bytes_extracted);

   if((n < 0) || (num_bytes_extracted < 6))
      return RULE_NOMATCH;

   /* Extract LM_X, a signed 16-bit entity in little-endian format */
   lm_x = decoded_data[2];
   lm_x += decoded_data[3] << 8; 

   if((lm_x < 0) || (lm_x > 56))
      return RULE_MATCH;

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
   &rule13921,
   NULL
};
*/
