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

/* declare detection functions */
int rule29944eval(void *p);
int rule29945eval(void *p);
static int DetectChunkOverflow(SFSnortPacket *sp);

/* declare rule data structures */
/* flow:established, to_client; */
static FlowFlags rule29944flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_CLIENT
};

static RuleOption rule29944option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule29944flow0
   }
};

/* flow:established, to_server; */
static FlowFlags rule29945flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule29945option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule29945flow0
   }
};

// file_data;
static CursorInfo rule_PNGChunkLenOverflow_file_data0 =
{
   0, /* offset */
   CONTENT_BUF_NORMALIZED /* flags */
};

static RuleOption rule_PNGChunkLenOverflow_option0 =
{
#ifndef MISSINGFILEDATA
   OPTION_TYPE_FILE_DATA,
#else
   OPTION_TYPE_SET_CURSOR,
#endif
   {
      &rule_PNGChunkLenOverflow_file_data0
   }
};

// content:"|89|PNG|0D 0A 1A 0A|", depth 0, fast_pattern; 
static ContentInfo rule_PNGChunkLenOverflow_content1 = 
{
   (uint8_t *) "|89|PNG|0D 0A 1A 0A|", /* pattern (now in snort content format) */
   0, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN|CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule_PNGChunkLenOverflow_option1 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule_PNGChunkLenOverflow_content1
   }
};

/* references */
/* reference: bugtraq "18385"; */
static RuleReference rule_PNGChunkLenOverflow_ref1 = 
{
   "bugtraq", /* type */
   "18385" /* value */
};

/* reference: cve "2006-0025"; */
static RuleReference rule_PNGChunkLenOverflow_ref2 = 
{
   "cve", /* type */
   "2006-0025" /* value */
};

/* reference: cve "2009-2501"; */
static RuleReference rule_PNGChunkLenOverflow_ref3 = 
{
   "cve", /* type */
   "2009-2501" /* value */
};

/* reference: cve "2012-5470"; */
static RuleReference rule_PNGChunkLenOverflow_ref4 = 
{
   "cve", /* type */
   "2012-5470" /* value */
};

/* reference: cve "2013-1331"; */
static RuleReference rule_PNGChunkLenOverflow_ref5 = 
{
   "cve", /* type */
   "2013-1331" /* value */
};

/* reference: url "technet.microsoft.com/en-us/security/bulletin/ms06-024"; */
static RuleReference rule_PNGChunkLenOverflow_ref6 = 
{
   "url", /* type */
   "technet.microsoft.com/en-us/security/bulletin/ms06-024" /* value */
};

/* reference: url "technet.microsoft.com/en-us/security/bulletin/ms09-062"; */
static RuleReference rule_PNGChunkLenOverflow_ref7 = 
{
   "url", /* type */
   "technet.microsoft.com/en-us/security/bulletin/ms09-062" /* value */
};

/* reference: url "technet.microsoft.com/en-us/security/bulletin/ms13-051"; */
static RuleReference rule_PNGChunkLenOverflow_ref8 = 
{
   "url", /* type */
   "technet.microsoft.com/en-us/security/bulletin/ms13-051" /* value */
};

static RuleReference *rule_PNGChunkLenOverflow_refs[] =
{
   &rule_PNGChunkLenOverflow_ref1,
   &rule_PNGChunkLenOverflow_ref2,
   &rule_PNGChunkLenOverflow_ref3,
   &rule_PNGChunkLenOverflow_ref4,
   &rule_PNGChunkLenOverflow_ref5,
   &rule_PNGChunkLenOverflow_ref6,
   &rule_PNGChunkLenOverflow_ref7,
   &rule_PNGChunkLenOverflow_ref8,
   NULL
};

/* metadata */
static RuleMetaData rule_PNGChunkLenOverflow_policy1 = 
{
   "policy balanced-ips drop"
};

static RuleMetaData rule_PNGChunkLenOverflow_policy2 = 
{
   "policy security-ips drop"
};

static RuleMetaData rule_PNGChunkLenOverflow_policy3 =
{
   "policy max-detect-ips drop"
};

/* metadata for sid 29944 */
/* metadata:service http, service imap, service pop3, policy balanced-ips drop, policy security-ips drop; */
static RuleMetaData rule29944service1 = 
{
   "service ftp-data"
};

static RuleMetaData rule29944service2 = 
{
   "service http"
};

static RuleMetaData rule29944service3 = 
{
   "service imap"
};

static RuleMetaData rule29944service4 = 
{
   "service pop3"
};

static RuleMetaData *rule29944metadata[] =
{
   &rule29944service1,
   &rule29944service2,
   &rule29944service3,
   &rule29944service4,
   &rule_PNGChunkLenOverflow_policy1,
   &rule_PNGChunkLenOverflow_policy2,
   &rule_PNGChunkLenOverflow_policy3,
   NULL
};

/* metadata for sid 29945 */
/* metadata:service smtp, policy balanced-ips drop, policy security-ips drop; */
static RuleMetaData rule29945service1 = 
{
   "service smtp"
};

static RuleMetaData *rule29945metadata[] =
{
   &rule29945service1,
   &rule_PNGChunkLenOverflow_policy1,
   &rule_PNGChunkLenOverflow_policy2,
   &rule_PNGChunkLenOverflow_policy3,
   NULL
};

RuleOption *rule29944options[] =
{
   &rule29944option0,
   &rule_PNGChunkLenOverflow_option0,
   &rule_PNGChunkLenOverflow_option1,
   NULL
};

Rule rule29944 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_TCP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "$FILE_DATA_PORTS", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "any", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      29944, /* sigid */
      4, /* revision */
      "attempted-user", /* classification */
      0,  /* hardcoded priority */
      "FILE-IMAGE Microsoft Multiple Products potentially malicious PNG detected - large or invalid chunk size",     /* message */
      rule_PNGChunkLenOverflow_refs, /* ptr to references */
      rule29944metadata /* ptr to metadata */
   },
   rule29944options, /* ptr to rule options */
   &rule29944eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

RuleOption *rule29945options[] =
{
   &rule29945option0,
   &rule_PNGChunkLenOverflow_option0,
   &rule_PNGChunkLenOverflow_option1,
   NULL
};

Rule rule29945 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_TCP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$SMTP_SERVERS", /* DSTIP     */
      "25", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      29945, /* sigid */
      4, /* revision */
      "attempted-user", /* classification */
      0,  /* hardcoded priority */
      "FILE-IMAGE Microsoft Multiple Products potentially malicious PNG detected - large or invalid chunk size",     /* message */
      rule_PNGChunkLenOverflow_refs, /* ptr to references */
      rule29945metadata /* ptr to metadata */
   },
   rule29945options, /* ptr to rule options */
   &rule29945eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule29944eval(void *p) {
   SFSnortPacket *sp = (SFSnortPacket *) p;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   // flow:established, to_client;
   if(checkFlow(p, rule29944options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;

   return DetectChunkOverflow(sp);
}

int rule29945eval(void *p) {
   SFSnortPacket *sp = (SFSnortPacket *) p;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;
   
   // flow:established, to_server;
   if(checkFlow(p, rule29945options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;
   
   return DetectChunkOverflow(sp);
}

static int DetectChunkOverflow(SFSnortPacket *sp) {
   const uint8_t *cursor_normal = 0, *beg_of_payload, *end_of_payload, *end_of_chunk;
   uint32_t chunk_size, chunk_type, extended_name;
   int i;

   // file_data;
   #ifndef MISSINGFILEDATA
   if(fileData(sp, rule29944options[1]->option_u.cursor, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   #else
   if(setCursor(sp, rule29944options[1]->option_u.cursor, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   #endif
   
   // content:"|89|PNG|0D 0A 1A 0A|", depth 0, fast_pattern;
   if(contentMatch(sp, rule29944options[2]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   
   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_payload, &end_of_payload) <= 0)
      return RULE_NOMATCH;

   // Parse up to 10 PNG chunks, need room
   // for 4 byte chunk size, 4 byte chunk type
   for(i=0; (i < 10) && (cursor_normal + 8 < end_of_payload); i++) {

      // read 4 byte chunk size
      chunk_size = read_big_32_inc(cursor_normal);

      // read 4 byte chunk type
      chunk_type = read_big_32(cursor_normal);

      // chunk processing
      switch(chunk_type) {

         case 0x74455874 : // tEXt

            DEBUG_SO(fprintf(stderr,"tEXt chunk found\n size = 0x%X\n",chunk_size);)
            // CVE-2013-1331
            if(chunk_size > 0x7FFFFFFF)
               return RULE_MATCH;

            break;
/*
         case 0x7a545874 : // zTXt
         case 0x69545874 : // iTXt
         case 0x74494d45 : // tIME
         case 0x73504c54 : // sPLT
         case 0x70485973 : // pHYs
         case 0x68495354 : // hIST
         case 0x624b4744 : // bKGD
         case 0x73524742 : // sRGB
         case 0x73424954 : // sBIT
         case 0x69434350 : // iCCP

            DEBUG_SO(fprintf(stderr,"%c%c",*cursor_normal,*(cursor_normal+1));)
            DEBUG_SO(fprintf(stderr,"%c%c chunk found\n",*(cursor_normal+2),*(cursor_normal+3));)
            DEBUG_SO(fprintf(stderr," size = 0x%X\n",chunk_size);)

            // CVE-2006-0025
            if(chunk_size > 10000)
               return RULE_MATCH;

            break;

         case 0x6348524d : // cHRM
         case 0x74524e53 : // tRNS

            DEBUG_SO(fprintf(stderr,"%c%c",*cursor_normal,*(cursor_normal+1));)
            DEBUG_SO(fprintf(stderr,"%c%c chunk found\n",*(cursor_normal+2),*(cursor_normal+3));)
            DEBUG_SO(fprintf(stderr," size = 0x%X\n",chunk_size);)

            // CVE-2006-0025
            if(chunk_size > 7000)
               return RULE_MATCH;

            break;
*/
         case 0x49454e44 : // IEND

            DEBUG_SO(fprintf(stderr,"IEND chunk, End of PNG\n");)
            return RULE_NOMATCH;

         default :

            DEBUG_SO(fprintf(stderr,"Skipping %c%c%c%c chunk\n",*cursor_normal,*(cursor_normal+1),*(cursor_normal+2),*(cursor_normal+3));)
            break;

      }

      // +4 byte chunk_type, +4 byte CRC
      end_of_chunk = cursor_normal + chunk_size + 8; 

      // Verify the ptr did not wrap (integer overflow check)
      // Also an "always marching forward" check
      if(end_of_chunk <= cursor_normal)
         return RULE_NOMATCH;

      // We verify cursor_normal is in range at the top of the loop
      cursor_normal = end_of_chunk;
   }

   return RULE_NOMATCH;
}


/*
Rule *rules[] = {
    &rule29944,
    NULL
};
*/

/*
Rule *rules[] = {
    &rule29945,
    NULL
};
*/
