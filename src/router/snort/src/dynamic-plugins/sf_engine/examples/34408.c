/*
 * Altrium Software MERCUR IMAPD NTLMSSP Command Handling memory corruption attempt
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2007-2013 Sourcefire, Inc. All Rights Reserved
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"

#include "imap_base64_decode.h"

/* declare detection functions */
static int ruleMERCUR_IMAPD_NTLMSSPeval(void *);

/* declare references */
static RuleReference ruleMERCUR_IMAPD_NTLMSSPref0 = {
    "url", /* type */
    "secunia.com/advisories/24596" /* value */
};

static RuleReference ruleMERCUR_IMAPD_NTLMSSPcve = {
    "bugtraq", /* type */
    "23058"
};

static RuleReference *ruleMERCUR_IMAPD_NTLMSSPrefs[] = {
    &ruleMERCUR_IMAPD_NTLMSSPref0,
    &ruleMERCUR_IMAPD_NTLMSSPcve,
    NULL
};

/* Declare rule options */
static FlowFlags ruleMERCUR_IMAPD_NTLMSSPflow = {
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption ruleMERCUR_IMAPD_NTLMSSPoption0 = {
    OPTION_TYPE_FLOWFLAGS,
    {
        &ruleMERCUR_IMAPD_NTLMSSPflow
    }
};

static ContentInfo ruleMERCUR_IMAPD_NTLMSSPcontent1 =
{
    (u_int8_t *)"TlRMTVNT", /* pattern to search for */
    8,                      /* depth */
    0,                      /* offset */
    CONTENT_BUF_NORMALIZED, /* flags */
    NULL,                   /* holder for boyer/moore info */
    NULL,                   /* holder for byte representation of something */
    0,                      /* holder for length of byte representation */
    0,                      /* holder of increment length */
    0,                      /* holder for fp offset */
    0,                      /* holder for fp length */
    0,                      /* holder for fp only */
    NULL, // offset_refId
    NULL, // depth_refId
    NULL, // offset_location
    NULL  // depth_location
};

static RuleOption ruleMERCUR_IMAPD_NTLMSSPoption1 = {
    OPTION_TYPE_CONTENT,
    {
        &ruleMERCUR_IMAPD_NTLMSSPcontent1
    }
};

static ContentInfo ruleMERCUR_IMAPD_NTLMSSPcontent2 =
{
    (u_int8_t *)"AAAA",     /* pattern to search for */
    4,                      /* depth */
    4,                      /* offset */
    CONTENT_BUF_NORMALIZED | CONTENT_RELATIVE,       /* flags */
    NULL,                   /* holder for boyer/moore info */
    NULL,                   /* holder for byte representation of something */
    0,                      /* holder for length of byte representation */
    0,                      /* holder of increment length */
    0,                      /* holder for fp offset */
    0,                      /* holder for fp length */
    0,                      /* holder for fp only */
    NULL, // offset_refId
    NULL, // depth_refId
    NULL, // offset_location
    NULL  // depth_location
};

static RuleOption ruleMERCUR_IMAPD_NTLMSSPoption2 = {
    OPTION_TYPE_CONTENT,
    {
        &ruleMERCUR_IMAPD_NTLMSSPcontent2
    }
};


RuleOption *ruleMERCUR_IMAPD_NTLMSSPoptions[] = {
    &ruleMERCUR_IMAPD_NTLMSSPoption0,
    &ruleMERCUR_IMAPD_NTLMSSPoption1,
    &ruleMERCUR_IMAPD_NTLMSSPoption2,
    NULL
};


/* Rule definition */

Rule ruleMERCUR_IMAPD_NTLMSSP = {
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
       34408, /* XXX sigid */
       1, /* revision  */
       "attempted-admin", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "IMAP Altrium Software MERCUR IMAPD NTLMSSP command handling memory corruption attempt",    /* message */
       ruleMERCUR_IMAPD_NTLMSSPrefs, /* ptr to references */
       NULL /* Meta data */
   },
   ruleMERCUR_IMAPD_NTLMSSPoptions, /* ptr to rule options */
   &ruleMERCUR_IMAPD_NTLMSSPeval, /* ptr to rule detection function */
   0, /* am I initialized yet? */
   0, /* number of options */
   0,  /* don't alert */
   NULL /* ptr to internal data... setup during rule registration */
};


/* Detection functions */

static int ruleMERCUR_IMAPD_NTLMSSPeval(void *p) {
   SFSnortPacket *sp = (SFSnortPacket *)p;

   const u_int8_t *cursor;

   int16_t lm_x;

   int n;  /* cruft */

   /* Data for holding our base64 data */
   u_int8_t decoded_data[16];
   u_int32_t num_bytes_extracted;


   /* General sanity checking */
  if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   if(sp->payload_size < 32)
      return RULE_NOMATCH;

   /* call flow match */
   if (checkFlow(sp, ruleMERCUR_IMAPD_NTLMSSPoptions[0]->option_u.flowFlags) <= 0 )
      return RULE_NOMATCH;

   /* call first content match */
   if (contentMatch(sp, ruleMERCUR_IMAPD_NTLMSSPoptions[1]->option_u.content, &cursor) <= 0) {
      return RULE_NOMATCH;
   }

   /* call second content match */
   if (contentMatch(sp, ruleMERCUR_IMAPD_NTLMSSPoptions[2]->option_u.content, &cursor) <= 0) {
      return RULE_NOMATCH;
   }

   /* Decode the part containing "/P.\x03/" to ensure the proper header and message type */
   n = base64decode(&(sp->payload[8]), 4, decoded_data, sizeof(decoded_data), &num_bytes_extracted);

   if((n < 0) || (num_bytes_extracted < 3))
      return RULE_NOMATCH;

   /* verify contents */
   if((decoded_data[0] != 'P') || (decoded_data[2] != 0x03))
      return RULE_NOMATCH;

   /* Now decode the part containing LM_X */
   n = base64decode(&(sp->payload[24]), 8, decoded_data, sizeof(decoded_data), &num_bytes_extracted);

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
   &ruleMERCUR_IMAPD_NTLMSSP,
   NULL
};
*/
