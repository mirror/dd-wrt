/*
 * MySQL COM_TABLE_DUMP Function Stack Overflow
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


int ruleMYSQL_COM_TABLE_DUMPeval(void *p);

/* flow:established, to_server; */
static FlowFlags ruleMYSQL_COM_TABLE_DUMPflow0 =
{
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption ruleMYSQL_COM_TABLE_DUMPoption0 =
{
    OPTION_TYPE_FLOWFLAGS,
    { &ruleMYSQL_COM_TABLE_DUMPflow0 }
};

// content:"|13|", depth 1, offset 4;
static ContentInfo ruleMYSQL_COM_TABLE_DUMPcontent1 =
{
    (u_int8_t *)"|13|", /* pattern (now in snort content format) */
    1, /* depth */
    4, /* offset */
    0, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
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

static RuleOption ruleMYSQL_COM_TABLE_DUMPoption1 =
{
    OPTION_TYPE_CONTENT,
    { &ruleMYSQL_COM_TABLE_DUMPcontent1 }
};


/* references for.ruleid MYSQL_COM_TABLE_DUMP */
static RuleReference ruleMYSQL_COM_TABLE_DUMPref0 =
{
   "cve", /* type */
   "2006-1518" /* value */
};

static RuleReference ruleMYSQL_COM_TABLE_DUMPref1 =
{
   "bugtraq", /* type */
   "17780" /* value */
};

static RuleReference ruleMYSQL_COM_TABLE_DUMPref2 =
{
   "url", /* type */
   "www.wisec.it/vulns.php?page=8" /* value */
};


static RuleReference *ruleMYSQL_COM_TABLE_DUMPrefs[] =
{
   &ruleMYSQL_COM_TABLE_DUMPref0,
   &ruleMYSQL_COM_TABLE_DUMPref1,
   &ruleMYSQL_COM_TABLE_DUMPref2,
   NULL
};



RuleOption *ruleMYSQL_COM_TABLE_DUMPoptions[] =
{
    &ruleMYSQL_COM_TABLE_DUMPoption0,
    &ruleMYSQL_COM_TABLE_DUMPoption1,
    NULL
};


Rule ruleMYSQL_COM_TABLE_DUMP = {
   /* rule header, akin to => tcp $EXTERNAL_NET any -> $HOME_NET 3306 */
   {
       IPPROTO_TCP, /* proto */
       "$EXTERNAL_NET", /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       "$HOME_NET", /* DSTIP     */
       "3306", /* DSTPORT   */
   },
   /* metadata */
   {
       3,  /* genid (HARDCODED!!!) */
       34313, /* XXX sigid */
       7, /* revision */

       "attempted-admin", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "MISC MySQL COM_TABLE_DUMP Function Stack Overflow attempt",     /* message */
       ruleMYSQL_COM_TABLE_DUMPrefs, /* ptr to references */
       NULL /* Meta data */
   },
   ruleMYSQL_COM_TABLE_DUMPoptions, /* ptr to rule options */
   &ruleMYSQL_COM_TABLE_DUMPeval,
   0, /* am I initialized yet? */
   0, /* number of options */
   0,  /* don't alert */
   NULL /* ptr to internal data... setup during rule registration */
};

int ruleMYSQL_COM_TABLE_DUMPeval(void *p) {
   const u_int8_t *cursor_normal = 0;
   u_int32_t packet_length;
   u_int8_t db_name_length, table_name_length;

   SFSnortPacket *sp = (SFSnortPacket *) p;

   /* Make sure the packet is long enough */
   if(sp->payload_size < 6)
      return RULE_NOMATCH;

   // flow:established, to_server;
   if(checkFlow(p, ruleMYSQL_COM_TABLE_DUMPoptions[0]->option_u.flowFlags) > 0 ) {

      // content:"|13|", depth 1; offset 4;
      if(contentMatch(p, ruleMYSQL_COM_TABLE_DUMPoptions[1]->option_u.content, &cursor_normal) > 0) {

         // Grab the size of the "packet" -- this is the size (in bytes) of
         // data after the message number byte, including the command byte
         // Packet size is 3 bytes, little endian
         packet_length = sp->payload[0];
         packet_length += sp->payload[1] << 8;
         packet_length += sp->payload[2] << 16;

         // Grab the length of the DB Name
         db_name_length = sp->payload[5];

         // if db name length > packet length (+2 for length and command
         // bytes), flag
         if(((u_int32_t)db_name_length + 2) > packet_length) {
            return RULE_MATCH;
         }  else {
            // else grab table name length
            // table_name_length is at offset of db_name + db_name_length
            // + size byte
            if(sp->payload_size < 5 + db_name_length + 2)
               return RULE_NOMATCH;

            table_name_length = sp->payload[5 + db_name_length + 1];

            // if table name length + db name length + size bytes
            // + command byte > packet length, flag
            if(((u_int32_t)db_name_length + (u_int32_t)table_name_length + 3) > packet_length)
               return RULE_MATCH;
         }
      }
   }

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
   &ruleMYSQL_COM_TABLE_DUMP,
   NULL
};

*/

