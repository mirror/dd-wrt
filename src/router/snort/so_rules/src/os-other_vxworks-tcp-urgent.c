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

/* declare detection functions */
int rule51111eval(void *p);

/* declare rule data structures */
/* references for sid 51111 */
/* reference: cve "2019-12255"; */
static RuleReference rule51111ref1 = 
{
   "cve", /* type */
   "2019-12255" /* value */
};

/* reference: cve "2019-12260"; */
static RuleReference rule51111ref2 = 
{
   "cve", /* type */
   "2019-12260" /* value */
};

/* reference: cve "2019-12261"; */
static RuleReference rule51111ref3 = 
{
   "cve", /* type */
   "2019-12261" /* value */
};

/* reference: url "www.windriver.com/security/announcements/tcp-ip-network-stack-ipnet-urgent11/"; */
static RuleReference rule51111ref4 = 
{
   "url", /* type */
   "www.windriver.com/security/announcements/tcp-ip-network-stack-ipnet-urgent11/" /* value */
};

static RuleReference *rule51111refs[] =
{
   &rule51111ref1,
   &rule51111ref2,
   &rule51111ref3,
   &rule51111ref4,
   NULL
};

/* metadata for sid 51111 */
static RuleMetaData *rule51111metadata[] =
{
   NULL
};

RuleOption *rule51111options[] =
{
   NULL
};

Rule rule51111 = {
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
      51111, /* sigid */
      1, /* revision */
      "attempted-admin", /* classification */
      0,  /* hardcoded priority */
      "OS-OTHER VxWorks TCP URG memory corruption attempt",     /* message */
      rule51111refs, /* ptr to references */
      rule51111metadata /* ptr to metadata */
   },
   rule51111options, /* ptr to rule options */
   &rule51111eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule51111eval(void *p) {
   SFSnortPacket *sp = (SFSnortPacket *) p;

   uint8_t flags = 0;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->tcp_header == NULL)
      return RULE_NOMATCH;

   // minimize dereferences
   flags = sp->tcp_header->flags;

   if(flags & TCPHEADER_URG)
   {
      // CVE-2019-12255
      // (not inherently malicious, specific to VxWorks)
      if(sp->tcp_header->urgent_pointer == 0)
         return RULE_MATCH;

      // bad flags: URG+SYN
      if(flags & TCPHEADER_SYN)
         return RULE_MATCH;

      // bad flags: URG+FIN
      if(flags & TCPHEADER_FIN)
         return RULE_MATCH;
   }

   // CVE-2019-12260
   // CVE-2019-12261
   if((flags & TCPHEADER_SYN) && (flags & TCPHEADER_FIN))
      return RULE_MATCH;

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule51111,
    NULL
};
*/
