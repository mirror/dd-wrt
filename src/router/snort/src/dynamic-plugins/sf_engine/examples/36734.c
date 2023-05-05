/*
 * Linux Kernel ICMP Packet Handling Denial of Service
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2007-2013 Sourcefire, Inc. All Rights Reserved
 *
 * Written by Patrick Mullen, Sourcefire VRT <pmullen@sourcefire.com>
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

#include "snort_debug.h"

/* declare detection functions */
static int ruleLINUXICMPDOSeval(void *p);

/* references sid LINUXICMPDOS */
static RuleReference ruleLINUXICMPDOSref1 =
{
    "url",
    "www.kernel.org/pub/linux/kernel/v2.6/ChangeLog-2.6.15.3"
};

static RuleReference ruleLINUXICMPDOSref2 =
{
    "cve",
    "2006-0454"
};

static RuleReference ruleLINUXICMPDOSref3 =
{
    "bugtraq",
    "16532"
};

static RuleReference *ruleLINUXICMPDOSrefs[] =
{
    &ruleLINUXICMPDOSref1,
    &ruleLINUXICMPDOSref2,
    &ruleLINUXICMPDOSref3,
    NULL
};

static HdrOptCheck ruleLINUXICMPDOSprotocheck0 =
{
    IP_HDR_OPTIONS,
    CHECK_EQ,
    0x07,
    0,                             /* bits to ignore */
    0                              /* flags */
};

static RuleOption ruleLINUXICMPDOSoption0 =
{
    OPTION_TYPE_HDR_CHECK,
    {
        &ruleLINUXICMPDOSprotocheck0
    }
};

static HdrOptCheck ruleLINUXICMPDOSprotocheck1 =
{
    IP_HDR_OPTIONS,
    CHECK_EQ,
    0x44,
    0,                             /* bits to ignore */
    0                              /* flags */
};

static RuleOption ruleLINUXICMPDOSoption1 =
{
    OPTION_TYPE_HDR_CHECK,
    {
        &ruleLINUXICMPDOSprotocheck1
    }
};



RuleOption *ruleLINUXICMPDOSoptions[] =
{
    &ruleLINUXICMPDOSoption0,
    &ruleLINUXICMPDOSoption1,
    NULL
};

Rule ruleLINUXICMPDOS = {
   /* rule header, akin to => tcp any any -> any any               */
   {
       IPPROTO_IP, /* proto */
       EXTERNAL_NET,       /* SRCIP     */
       "any",        /* SRCPORT   */
       0,           /* DIRECTION */
       HOME_NET,       /* DSTIP     */
       "any",       /* DSTPORT   */
   },
   /* metadata */
   {
       3,       /* genid (HARDCODED!!!) */
       36734,       /* sid */
       0,       /* revision */
       "denial-of-service", /* classification, generic */
       0,                 /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "BAD-TRAFFIC linux ICMP header dos attempt",     /* message */
       ruleLINUXICMPDOSrefs, /* ptr to references */
       NULL /* Meta data */
   },
   ruleLINUXICMPDOSoptions, /* ptr to rule options */
   &ruleLINUXICMPDOSeval, /* ptr to rule detection function */
   0, /* am I initialized yet? */
   0, /* number of options */
   0, /* don't alert */
   NULL /* ptr to internal data... setup during rule registration */
};


/* detection functions */
static int ruleLINUXICMPDOSeval(void *p) {
   SFSnortPacket *sp = (SFSnortPacket *) p;

   // stuff we're testing
   u_int8_t length;
   u_int8_t pointer;

   // cruft
   int i;

   if(NULL == sp)
      return RULE_NOMATCH;

   //DEBUG_WRAP(printf("Beginning of processing\n"));

   // Leave if not ipopt 0x07 and not ipopt 0x44
   if((checkHdrOpt(p, ruleLINUXICMPDOSoptions[0]->option_u.hdrData) <= 0) &&
      (checkHdrOpt(p, ruleLINUXICMPDOSoptions[1]->option_u.hdrData) <= 0)) {

      //DEBUG_WRAP(printf("not ipopt 0x07 or ipopt 0x44\n"););
      return RULE_NOMATCH;
   }

   //DEBUG_WRAP(printf("Checking ip_options (%d total)\n", sp->num_ip_options));
   for(i = 0; i < (int)sp->num_ip_options; i++) {
      //DEBUG_WRAP(printf("Checking ip_option[%d] (0x%02x)\n", i, sp->ip_options[i].option_code));

      // Don't bother going any farther if there's no option data (is this even possible?)
      if(NULL == sp->ip_options[i].option_data)
         continue;

      if(sp->ip_options[i].option_code == 0x07) {
         //DEBUG_WRAP(printf("ipopt 0x07 inside ip_options[]\n"));

         // length is the length of the option data; doesn't include type or length bytes
         length = sp->ip_options[i].length;

         // make sure there's enough room for pointer
         if(length < 1)
            return RULE_NOMATCH;

         // what's confusing is pointer is from the beginning of the option header, and
         // includes the type and length bytes.  Who writes this crap?
         pointer = sp->ip_options[i].option_data[0];
         //DEBUG_WRAP(printf("length = %d, pointer = %d\n", length, pointer));

         // If pointer points past the end of the option, the data is full and that's okay
         if(pointer >= length + 2)
            return RULE_NOMATCH;

         // If there is not an even multiple of 4 bytes of open space, alert.
         // The (pointer - 1) is because the pointer points after the last address
         // if(((length + 2) - (pointer - 1)) % 4) simplifies to...
         if(((length + 3) - pointer) % 4)
            return RULE_MATCH;

      } else if(sp->ip_options[i].option_code == 0x44) {
         //DEBUG_WRAP(printf("ipopt 0x44 inside ip_options[]\n"));

         length = sp->ip_options[i].length;

         // Make sure there's enough room for pointer and flags
         if(length < 2)
            return RULE_NOMATCH;

         pointer = sp->ip_options[i].option_data[0];
         //DEBUG_WRAP(printf("length = %d, pointer = %d\n", length, pointer));

         if(pointer >= length + 2)
            return RULE_NOMATCH;

         // if no address info included, only need 4 bytes/ea for timestamps
         if(((length + 3) - pointer) % 4)
            return RULE_MATCH;

         // but if there is timestamp + address, we need 8.  The check above
         // is still good because if it fails %4 it'll definitely fail %8.
         if((sp->ip_options[i].option_data[1] & 0x01) && // contains addresses
            (((length + 3) - pointer) % 8))
            return RULE_MATCH;
      }
   }

   //DEBUG_WRAP(printf("End of processing\n"));

   return RULE_NOMATCH;
}
