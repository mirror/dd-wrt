/*
 * MySQL COM_TABLE_DUMP Function Stack Overflow
 *
 * Copyright (C) 2007 Sourcefire, Inc. All Rights Reserved
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
    {
        &ruleMYSQL_COM_TABLE_DUMPflow0
    }
};

// content:"|13|", depth 1, offset 4;
static ContentInfo ruleMYSQL_COM_TABLE_DUMPcontent1 =
{
    (uint8_t *)"|13|", /* pattern (now in snort content format) */
    1, /* depth */
    4, /* offset */
    CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0 /* byteform length */
};

static RuleOption ruleMYSQL_COM_TABLE_DUMPoption1 =
{
    OPTION_TYPE_CONTENT,
    {
        &ruleMYSQL_COM_TABLE_DUMPcontent1
    }
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

static RuleReference ruleMYSQL_COM_TABLE_DUMPref3 =
{
   "cve", /* type */
   "2006-1516" /* value */
};

static RuleReference ruleMYSQL_COM_TABLE_DUMPref4 =
{
   "cve", /* type */
   "2006-1517" /* value */
};

static RuleReference *ruleMYSQL_COM_TABLE_DUMPrefs[] =
{
   &ruleMYSQL_COM_TABLE_DUMPref0,
   &ruleMYSQL_COM_TABLE_DUMPref1,
   &ruleMYSQL_COM_TABLE_DUMPref2,
   &ruleMYSQL_COM_TABLE_DUMPref3,
   &ruleMYSQL_COM_TABLE_DUMPref4,
   NULL
};

RuleOption *ruleMYSQL_COM_TABLE_DUMPoptions[] =
{
    &ruleMYSQL_COM_TABLE_DUMPoption0,
    &ruleMYSQL_COM_TABLE_DUMPoption1,
    NULL
};

static RuleMetaData ruleMYSQL_COM_TABLE_DUMPservice1 =
{
    "service mysql"
};

static RuleMetaData ruleMYSQL_COM_TABLE_DUMPpolicy1 =
{
    "policy max-detect-ips drop"
};

static RuleMetaData *ruleMYSQL_COM_TABLE_DUMPmetadata[] =
{
    &ruleMYSQL_COM_TABLE_DUMPservice1,
    &ruleMYSQL_COM_TABLE_DUMPpolicy1,
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
       11619, /* sigid a0b81b8c-ff1d-4af3-86b9-37a6a04cc885 */
       7, /* revision b591b0e0-40e1-4e9b-b864-e4c71544c3d1 */

       "attempted-admin", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "SERVER-MYSQL MySQL COM_TABLE_DUMP Function Stack Overflow attempt",     /* message */
       ruleMYSQL_COM_TABLE_DUMPrefs /* ptr to references */
        ,ruleMYSQL_COM_TABLE_DUMPmetadata
   },
   ruleMYSQL_COM_TABLE_DUMPoptions, /* ptr to rule options */
   &ruleMYSQL_COM_TABLE_DUMPeval, 
   0, /* am I initialized yet? */
   0, /* number of options */
   0,  /* don't alert */
   NULL /* ptr to internal data... setup during rule registration */
};

int ruleMYSQL_COM_TABLE_DUMPeval(void *p) {
   const uint8_t *cursor_normal = 0, *beg_of_payload = 0, *end_of_payload = 0;
   uint32_t packet_length;
   uint8_t db_name_length, table_name_length;

   SFSnortPacket *sp = (SFSnortPacket *) p; 

   // flow:established, to_server;
   if(checkFlow(p, ruleMYSQL_COM_TABLE_DUMPoptions[0]->option_u.flowFlags) > 0 ) {

      /* Make sure the packet is long enough */
      if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_payload, &end_of_payload) <= 0)
         return RULE_NOMATCH;

      if((end_of_payload - beg_of_payload) < 6)
         return RULE_NOMATCH;


      // content:"|13|", depth 1; offset 4;
      if(contentMatch(p, ruleMYSQL_COM_TABLE_DUMPoptions[1]->option_u.content, &cursor_normal) > 0) {

         // Grab the size of the "packet" -- this is the size (in bytes) of
         // data after the message number byte, including the command byte
         // Packet size is 3 bytes, little endian
         packet_length = beg_of_payload[0];
         packet_length += beg_of_payload[1] << 8;
         packet_length += beg_of_payload[2] << 16;

         // Grab the length of the DB Name
         db_name_length = beg_of_payload[5];

         // if db name length > packet length (+2 for length and command
         // bytes), flag
         if((db_name_length + 2) > packet_length) {
            return RULE_MATCH;
         }  else {
            // else grab table name length
            // table_name_length is at offset of db_name + db_name_length 
            // + size byte
            if((end_of_payload - beg_of_payload) < 5 + db_name_length + 2) 
               return RULE_NOMATCH;

            table_name_length = beg_of_payload[5 + db_name_length + 1]; 

            // if table name length + db name length + size bytes
            // + command byte > packet length, flag
            if((db_name_length + table_name_length + 3) > packet_length)
               return RULE_MATCH;
         }
      }  
   }  

   return RULE_NOMATCH;
}

