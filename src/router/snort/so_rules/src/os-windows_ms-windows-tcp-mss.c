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

// Ignore reassembled packets
#ifndef REASSEMBLED_PACKET_FLAGS
#define REASSEMBLED_PACKET_FLAGS (FLAG_REBUILT_STREAM|FLAG_SMB_SEG|FLAG_DCE_SEG|FLAG_DCE_FRAG|FLAG_SMB_TRANS)
#endif

/* declare detection functions */
int rule26877eval(void *p);

/* declare rule data structures */
/* flow:to_server; */
static FlowFlags rule26877flow0 = 
{
   FLOW_TO_SERVER
};

static RuleOption rule26877option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule26877flow0
   }
};


// tcp_flags:syn;
static HdrOptCheck rule26877tcp_flags1 =
{
   TCP_HDR_FLAGS,
   CHECK_EQ,
   TCPHEADER_SYN,
   0,
   0
};

static RuleOption rule26877option1 =
{
   OPTION_TYPE_HDR_CHECK,
   {
      &rule26877tcp_flags1
   }
};

/* references for sid 26877 */
/* reference: cve "2013-3138"; */
static RuleReference rule26877ref1 = 
{
   "cve", /* type */
   "2013-3138" /* value */
};

/* reference: url "technet.microsoft.com/en-us/security/bulletin/MS13-049"; */
static RuleReference rule26877ref2 = 
{
   "url", /* type */
   "technet.microsoft.com/en-us/security/bulletin/MS13-049" /* value */
};

static RuleReference *rule26877refs[] =
{
   &rule26877ref1,
   &rule26877ref2,
   NULL
};

/* metadata for sid 26877 */
/* metadata:; */
static RuleMetaData *rule26877metadata[] =
{
   NULL
};

RuleOption *rule26877options[] =
{
   &rule26877option0,
   &rule26877option1,
   NULL
};

Rule rule26877 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_TCP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "any", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      26877, /* sigid */
      4, /* revision */
      "attempted-dos", /* classification */
      0,  /* hardcoded priority */
      "OS-WINDOWS Microsoft Windows TCPRecomputeMss denial of service attempt",     /* message */
      rule26877refs, /* ptr to references */
      rule26877metadata /* ptr to metadata */
   },
   rule26877options, /* ptr to rule options */
   &rule26877eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule26877eval(void *p)
{
   SFSnortPacket *sp = (SFSnortPacket *) p;

   TCPOptions *tcp_options;
   uint32_t num_tcp_options;

   uint32_t mss_val;
   uint8_t option_length;

   uint32_t i;

   DEBUG_SO(fprintf(stderr, "rule26877eval enter\n");)

   if(sp == NULL)
      return RULE_NOMATCH;

   // Don't look at application layer reassembled packets
   if(sp->flags & REASSEMBLED_PACKET_FLAGS)
      return RULE_NOMATCH;

   // tcp_flags:syn;
   if(checkHdrOpt(p, rule26877options[1]->option_u.hdrData) <= 0)
      return RULE_NOMATCH;

   // flow:to_server;
   if(checkFlow(p, rule26877options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;

   if(sp->tcp_header == NULL)
      return RULE_NOMATCH;

   num_tcp_options = sp->num_tcp_options;
   tcp_options = sp->tcp_options; //&(sp->tcp_options[0]);

   DEBUG_SO(fprintf(stderr, "%d tcp options\n", num_tcp_options);)

   if(num_tcp_options > 5)
      num_tcp_options = 5;

   // Parse up to 5 TCP options
   for(i = 0; i < num_tcp_options; i++)
   {

      DEBUG_SO(fprintf(stderr, "option type %d\n", tcp_options[i].option_code);)

      // Only care about MSS TCP option
      if(tcp_options[i].option_code != TCPOPT_MSS)
         continue;

      option_length = tcp_options[i].length;

      // vuln condition is MSS >= 0xfecc
      if(option_length < 2) {

         // MSS can't be >= 0xfecc if less than 2 bytes
         return RULE_NOMATCH;

      } else if(option_length > 4) {

         // MSS is more than 32 bits, someone is messing with us
         return RULE_MATCH;

      } else {

         uint8_t *option_data = tcp_options[i].option_data;

         // MSS is between 2 and 4 bytes inclusive
         if(option_length > 2) {

            // if 3 or 4 byte mss_val and first byte is non-zero
             if(*option_data++)
                return RULE_MATCH;

             if(option_length > 3)
                if(*option_data++)
                   return RULE_MATCH;

         }

         mss_val = read_big_16(option_data);

         DEBUG_SO(fprintf(stderr, "MSS opt index %d, length = %d, value = 0x%04X\n", i, option_length, mss_val);)

         // vulnerability condition is >= 0xfecc, however according to
         // research here: wand.net.nz/sites/default/files/mss_ict11.pdf
         // "An Analysis of TCP Maximum Segment Sizes" most MSS values
         // are never higher than 8961. 2016-11-10 - due to FPs, doubling
         // this number because time marches on and TCP MSSes seem to have
         // gotten bigger.  This is still proactive and well below the
         // vuln condition
         //
         if(mss_val >= 17922)
            return RULE_MATCH;

         // Only check first MSS value
         return RULE_NOMATCH;

      }
   }

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule26877,
    NULL
};
*/
