/*
 * MISC IBM Lotus Domino LDAP server invalide DN message buffer overflow attempt
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

#ifndef PM_EXP2
#define PM_EXP2(A) 1 << A
#endif

/* declare detection functions */
int ruleDOMINO_LDAP_INVALID_DNeval(void *p);

static RuleReference ruleDOMINO_LDAP_INVALID_DNref0 =
{
    "url", /* type */
    "www-1.ibm.com/support/docview.wss?uid=swg21257248" /* value */
};
static RuleReference ruleDOMINO_LDAP_INVALID_DNref1 =
{
    "cve", /* type */
    "CVE-2007-1739"
};
static RuleReference ruleDOMINO_LDAP_INVALID_DNref2 =
{
    "bugtraq", /* type */
    "23174"
};

static RuleReference *ruleDOMINO_LDAP_INVALID_DNrefs[] =
{
    &ruleDOMINO_LDAP_INVALID_DNref0,
    &ruleDOMINO_LDAP_INVALID_DNref1,
    &ruleDOMINO_LDAP_INVALID_DNref2,
    NULL
};

static FlowFlags ruleDOMINO_LDAP_INVALID_DNflow =
{
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption ruleDOMINO_LDAP_INVALID_DNoption0 =
{
    OPTION_TYPE_FLOWFLAGS,
    {
        &ruleDOMINO_LDAP_INVALID_DNflow
    }
};

static ContentInfo ruleDOMINO_LDAP_INVALID_DNcontent =
{
    (u_int8_t *)"|30|",                 /* pattern to search for */
    1,                      /* depth */
    0,                      /* offset */
    0,                      /* flags */
    NULL,                   /* holder for boyer/moore info */
    NULL,                   /* holder for byte representation of "NetBus" */
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

static RuleOption ruleDOMINO_LDAP_INVALID_DNoption1 =
{
    OPTION_TYPE_CONTENT,
    {
        &ruleDOMINO_LDAP_INVALID_DNcontent
    }
};


RuleOption *ruleDOMINO_LDAP_INVALID_DNoptions[] =
{
    &ruleDOMINO_LDAP_INVALID_DNoption0,
    &ruleDOMINO_LDAP_INVALID_DNoption1,
    NULL
};

Rule ruleDOMINO_LDAP_INVALID_DN = {
   /* rule header */
   {
       IPPROTO_TCP, /* proto */
       "any", /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       HOME_NET, /* DSTIP     */
       "389", /* DSTPORT   */
   },
   /* metadata */
   {
       3,  /* genid (HARDCODED!!!) */
       34427, /*XXX sigid */
       1, /* revision  */

       "attempted-admin", /* classification, generic */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "MISC IBM Lotus Domino LDAP server invalide DN message buffer overflow attempt",     /* message */
       ruleDOMINO_LDAP_INVALID_DNrefs, /* ptr to references */
       NULL /* Meta data */
   },
   ruleDOMINO_LDAP_INVALID_DNoptions, /* ptr to rule options */
   &ruleDOMINO_LDAP_INVALID_DNeval, /* ptr to rule detection function */
   0, /* am I initialized yet? */
   0, /* number of options */
   0, /* don't alert */
   NULL /* ptr to internal data... setup during rule registration */
};


/* detection functions */

int process_val(const u_int8_t *data, u_int32_t data_len, u_int32_t *retvalue, u_int32_t *actual_data_len) {
   u_int32_t i;
   *actual_data_len = 0;
   *retvalue = 0;

   /* Jump over NULLs */
   i = 0;
   while((i < data_len) && (data[i] == 0)) {
      i++;
   }
   *actual_data_len = data_len - i;
   if(*actual_data_len > 4 || *actual_data_len == 0) return(-1);
                                       /* We can't store more than a u_int32_t */

   /* Now find the actual value */
   for(;i<data_len;i++) {
      *retvalue += data[i] * PM_EXP2(8*(data_len - i - 1));
   }

   return(0);
}


int ruleDOMINO_LDAP_INVALID_DNeval(void *p) {
   u_int32_t current_byte = 0;
   u_int32_t width, value, lengthwidth, value_len;
   int retval;

   const u_int8_t *cursor_normal;

   SFSnortPacket *sp = (SFSnortPacket *) p;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   if(sp->payload_size < 11)   /* Minimum add request length */
      return RULE_NOMATCH;

   /* call flow match */
   if (checkFlow(sp, ruleDOMINO_LDAP_INVALID_DNoptions[0]->option_u.flowFlags) <= 0 )
      return RULE_NOMATCH;

   /* call content match */
   if (contentMatch(sp, ruleDOMINO_LDAP_INVALID_DNoptions[1]->option_u.content, &cursor_normal) <= 0) {
      return RULE_NOMATCH;
   }

   /* our contentMatch already assures us the first byte is \x30, so just jump over it */
   current_byte++;

   /* Begin packet structure processing */
   /* Packet length (only care about width of the specifier) */
   if(sp->payload[current_byte] & 0x80) {
      current_byte += sp->payload[current_byte] & 0x0F;  /* Does imail do this properly? */
   }
   current_byte++;

   /* Message number (only care about width of the specifier) */
   if((u_int32_t)sp->payload_size < current_byte + 8)
      return RULE_NOMATCH;

   if(sp->payload[current_byte] != 0x02) /* Int data type */
      return RULE_NOMATCH;
   current_byte++;

   /* int width specifier */
   if(sp->payload[current_byte] & 0x80) {
      width = sp->payload[current_byte] & 0x0F;
      current_byte++;

      if((u_int32_t)sp->payload_size < current_byte + width)
         return RULE_NOMATCH;

      retval = process_val(&(sp->payload[current_byte]), width, &value, &value_len);
      if(retval < 0)
         return RULE_NOMATCH;  /* width is either 0 or > 4 */
      current_byte += width;   /* width of data width specifier */
      current_byte += value;   /* width of data itself */
   }  else {
      current_byte += sp->payload[current_byte] + 1;
   }

   if((u_int32_t)sp->payload_size < current_byte + 5)
      return RULE_NOMATCH;

   /* Message Add request */
   if(sp->payload[current_byte] != 0x68)
      return RULE_NOMATCH;

   current_byte++;

   /* Message length  (only care about width of the specifier) */
   if(sp->payload[current_byte] & 0x80) {
      current_byte += sp->payload[current_byte] & 0x0F;
   }
   current_byte++;

   if((u_int32_t)sp->payload_size < current_byte + 2)
      return RULE_NOMATCH;

   /* Here is the actual exploit detection.  We want to determine
      the length of the DN and flag if it's greater than 0xFFFF bytes */

   /* Make sure we're looking at a string */
   if(sp->payload[current_byte] != 0x04) /* string data type */
      return RULE_NOMATCH;
   current_byte++;

   /* Find the length of the string */
   if(sp->payload[current_byte] & 0x80) {
      lengthwidth = sp->payload[current_byte] & 0x0F;
      current_byte++;

      if((u_int32_t)sp->payload_size < current_byte + lengthwidth)
         return RULE_NOMATCH;

      retval = process_val(&(sp->payload[current_byte]), lengthwidth, &value, &value_len);

      /* retval will be -1 as this is an error condition, but value_len still contains
         useful data in this case. If the value is > 4 bytes long, it's greater than 0xFFFF  */
      if(value_len > 4)
         return RULE_MATCH;

      if((retval == 0) && (value > 0xFFFF))
         return RULE_MATCH;
   }  else {
      return RULE_NOMATCH;  /* clearly too short if it fits in 7 bits */
   }

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &ruleDOMINO_LDAP_INVALID_DN,
    NULL
};

*/
