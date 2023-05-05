/*
 * Mozilla Network Security Services SSLv2 Server Stack Overflow
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2007-2013 Sourcefire, Inc. All Rights Reserved
 *
 * Written by Patrick Mullen <pmullen@sourcefire.com>
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


int ruleMOZILLA_SSLV2_CMKeval(void *p);

/* flow:established, to_server; */
static FlowFlags ruleMOZILLA_SSLV2_CMKflow0 =
{
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption ruleMOZILLA_SSLV2_CMKoption0 =
{
    OPTION_TYPE_FLOWFLAGS,
    { &ruleMOZILLA_SSLV2_CMKflow0 }
};

/* flowbits:isset "sslv2.server_hello.request"; */
static FlowBitsInfo ruleMOZILLA_SSLV2_CMKflowbits1 =
{
    "sslv2.server_hello.request",
    FLOWBIT_ISSET,
    0, /* flowbits id (SET BY ENGINE) */
    0, /* flags (NOT USED CURRENTLY) */
    NULL, /*group name*/
    0,/*eval*/
    NULL, /*ids*/
    0 /*num_ids*/
};

static RuleOption ruleMOZILLA_SSLV2_CMKoption1 =
{
    OPTION_TYPE_FLOWBIT,
    { &ruleMOZILLA_SSLV2_CMKflowbits1 }
};


/* flowbits:isnotset "sslv2.client_master_key.request"; */
static FlowBitsInfo ruleMOZILLA_SSLV2_CMKflowbits2 =
{
    "sslv2.client_master_key.request",
    FLOWBIT_ISNOTSET,
    0, /* flowbits id (SET BY ENGINE) */
    0, /* flags (NOT USED CURRENTLY) */
    NULL, /*group name*/
    0,/*eval*/
    NULL, /*ids*/
    0 /*num_ids*/
};

static RuleOption ruleMOZILLA_SSLV2_CMKoption2 =
{
    OPTION_TYPE_FLOWBIT,
    { &ruleMOZILLA_SSLV2_CMKflowbits2 }
};

// content:"|02|", depth 1, offset 3;
static ContentInfo ruleMOZILLA_SSLV2_CMKcontent3 =
{
    (u_int8_t *)"|02|", /* pattern (now in snort content format) */
    1, /* depth */
    2, /* offset */
    0, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support  /* ZDNOTE */
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0, /* increment length */
    0,                      /* holder for fp offset */
    0,                      /* holder for fp length */
    0,                      /* holder for fp only */
    NULL, // offset_refId
    NULL, // depth_refId
    NULL, // offset_location
    NULL  // depth_location
};

static RuleOption ruleMOZILLA_SSLV2_CMKoption3 =
{
    OPTION_TYPE_CONTENT,
    { &ruleMOZILLA_SSLV2_CMKcontent3 }
};

/* flowbits:set "sslv2.client_master_key.request"; */
static FlowBitsInfo ruleMOZILLA_SSLV2_CMKflowbits4 =
{
    "sslv2.client_master_key.request",
    FLOWBIT_SET,
    0, /* flowbits id (SET BY ENGINE) */
    0, /* flags (NOT USED CURRENTLY) */
    NULL, /*group name*/
    0,/*eval*/
    NULL, /*ids*/
    0 /*num_ids*/
};

static RuleOption ruleMOZILLA_SSLV2_CMKoption4 =
{
    OPTION_TYPE_FLOWBIT,
    { &ruleMOZILLA_SSLV2_CMKflowbits4 }
};


/* references for.ruleid MOZILLA_SSLV2_CMK */
static RuleReference *ruleMOZILLA_SSLV2_CMKrefs[] =
{
    NULL /* ZDNOTE */
};

RuleOption *ruleMOZILLA_SSLV2_CMKoptions[] =
{
    &ruleMOZILLA_SSLV2_CMKoption0,
    &ruleMOZILLA_SSLV2_CMKoption1,
    &ruleMOZILLA_SSLV2_CMKoption2,
    &ruleMOZILLA_SSLV2_CMKoption3,
    &ruleMOZILLA_SSLV2_CMKoption4,
    NULL
};

Rule ruleMOZILLA_SSLV2_CMK = {
   /* rule header, akin to => tcp $EXTERNAL_NET any -> $HOME_NET $HTTPS_PORTS */
   {
       IPPROTO_TCP, /* proto */
       "$EXTERNAL_NET", /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       "$HOME_NET", /* DSTIP     */
       "443", /* DSTPORT   */
   },
   /* metadata */
   {
       3,  /* genid (HARDCODED!!!) */
       34159, /* XXX sigid */
       7, /* revision */

       "attempted-admin", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "MISC Mozilla Network Security Services SSLv2 stack overflow attempt",     /* message */
       ruleMOZILLA_SSLV2_CMKrefs, /* ptr to references */
       NULL /* Meta data */
   },
   ruleMOZILLA_SSLV2_CMKoptions, /* ptr to rule options */
   &ruleMOZILLA_SSLV2_CMKeval,
   0, /* am I initialized yet? */
   0, /* number of options */
   0,  /* don't alert */
   NULL /* ptr to internal data... setup during rule registration */
};

int ruleMOZILLA_SSLV2_CMKeval(void *p) {
   const u_int8_t *cursor_normal = 0;
   u_int16_t cipher_keybits, clear_key_data_length;

   SFSnortPacket *sp = (SFSnortPacket *) p;

   // flow:established, to_server;
   if(checkFlow(p, ruleMOZILLA_SSLV2_CMKoptions[0]->option_u.flowFlags) > 0 ) {

      // flowbits:isset "sslv2.server_hello.request";
      if(processFlowbits(p, ruleMOZILLA_SSLV2_CMKoptions[1]->option_u.flowBit) > 0) {

         // flowbits:isnotset "sslv2.client_master_key.request";
         if(processFlowbits(p, ruleMOZILLA_SSLV2_CMKoptions[2]->option_u.flowBit) > 0) {

            // content:"|02|", depth 1; offset 2;
            if(contentMatch(p, ruleMOZILLA_SSLV2_CMKoptions[3]->option_u.content, &cursor_normal) > 0) {

               // Set our flowbit
               // flowbits:set "sslv2.client_master_key.request";
               if(processFlowbits(p, ruleMOZILLA_SSLV2_CMKoptions[4]->option_u.flowBit) > 0) {

                  /* Make sure the packet is long enough */
                  if(sp->payload_size < 8)
                     return RULE_NOMATCH;

                  /* Take the important values out of the packet */
                  cipher_keybits = sp->payload[4] << 8;
                  cipher_keybits += sp->payload[5];

                  clear_key_data_length = sp->payload[6] << 8;
                  clear_key_data_length += sp->payload[7];

                  /* Now we do actual detection. */
                  if((cipher_keybits + 7)/8 - clear_key_data_length < 0)
                     return RULE_MATCH;
               }
            }
         }
      }
   }

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
   &ruleMOZILLA_SSLV2_CMK,
   NULL
};

*/

