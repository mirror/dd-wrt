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
int rule41372eval(void *p);
int rule41373eval(void *p);
static int DetectOitMemoryCorruption(SFSnortPacket *sp);

/* declare rule data structures */
/* flow:established, to_client; */
static FlowFlags rule41372flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_CLIENT
};

static RuleOption rule41372option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule41372flow0
   }
};

/* flow:established, to_server; */
static FlowFlags rule41373flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule41373option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule41373flow0
   }
};

/* flowbits:isset "file.gif"; */
static FlowBitsInfo rule_OitMemoryCorruption_flowbits1 =
{
   "file.gif",
   FLOWBIT_ISSET,
   0,
};

static RuleOption rule_OitMemoryCorruption_option1 =
{
   OPTION_TYPE_FLOWBIT,
   {
      &rule_OitMemoryCorruption_flowbits1
   }
};

// file_data;
static CursorInfo rule_OitMemoryCorruption_file_data2 =
{
   0, /* offset */
   CONTENT_BUF_NORMALIZED /* flags */
};

static RuleOption rule_OitMemoryCorruption_option2 =
{
#ifndef MISSINGFILEDATA
   OPTION_TYPE_FILE_DATA,
#else
   OPTION_TYPE_SET_CURSOR,
#endif
   {
      &rule_OitMemoryCorruption_file_data2
   }
};

// content:"GIF8", depth 4, fast_pattern; 
static ContentInfo rule_OitMemoryCorruption_content3 = 
{
   (uint8_t *) "GIF8", /* pattern */
   4, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN|CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule_OitMemoryCorruption_option3 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule_OitMemoryCorruption_content3
   }
};

/* references */
/* reference: url "www.talosintel.com/vulnerability-reports/"; */
static RuleReference rule_OitMemoryCorruption_ref1 = 
{
   "url", /* type */
   "http://www.talosintelligence.com/vulnerability-reports/" /* value */
};

static RuleReference *rule_OitMemoryCorruption_refs[] =
{
   &rule_OitMemoryCorruption_ref1,
   NULL
};

/* metadata */
/* metadata:service ftp-data, service http, service imap, service pop3, policy security-ips drop; */
static RuleMetaData rule_OitMemoryCorruption_service1 = 
{
   "service ftp-data"
};

static RuleMetaData rule_OitMemoryCorruption_service2 = 
{
   "service http"
};

static RuleMetaData rule_OitMemoryCorruption_service3 = 
{
   "service imap"
};

static RuleMetaData rule_OitMemoryCorruption_service4 = 
{
   "service pop3"
};

static RuleMetaData rule_OitMemoryCorruption_policy1 = 
{
   "policy security-ips drop"
};

static RuleMetaData *rule_OitMemoryCorruption_metadata[] =
{
   &rule_OitMemoryCorruption_service1,
   &rule_OitMemoryCorruption_service2,
   &rule_OitMemoryCorruption_service3,
   &rule_OitMemoryCorruption_service4,
   &rule_OitMemoryCorruption_policy1,
   NULL
};

RuleOption *rule41372options[] =
{
   &rule41372option0,
   &rule_OitMemoryCorruption_option1,
   &rule_OitMemoryCorruption_option2,
   &rule_OitMemoryCorruption_option3,
   NULL
};

Rule rule41372 = {
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
      41372, /* sigid */
      1, /* revision */
      "attempted-admin", /* classification */
      0,  /* hardcoded priority */
      "FILE-IMAGE Oracle Outside In libvs_gif out of bounds write attempt",     /* message */
      rule_OitMemoryCorruption_refs, /* ptr to references */
      rule_OitMemoryCorruption_metadata /* ptr to metadata */
   },
   rule41372options, /* ptr to rule options */
   &rule41372eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

RuleOption *rule41373options[] =
{
   &rule41373option0,
   &rule_OitMemoryCorruption_option1,
   &rule_OitMemoryCorruption_option2,
   &rule_OitMemoryCorruption_option3,
   NULL
};

Rule rule41373 = {
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
      41373, /* sigid */
      1, /* revision */
      "attempted-admin", /* classification */
      0,  /* hardcoded priority */
      "FILE-IMAGE Oracle Outside In libvs_gif out of bounds write attempt",     /* message */
      rule_OitMemoryCorruption_refs, /* ptr to references */
      rule_OitMemoryCorruption_metadata /* ptr to metadata */
   },
   rule41373options, /* ptr to rule options */
   &rule41373eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule41372eval(void *p) {
   SFSnortPacket *sp = (SFSnortPacket *) p;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   // flow:established, to_client;
   if(checkFlow(p, rule41372options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;

   return DetectOitMemoryCorruption(sp);
}

int rule41373eval(void *p) {
   SFSnortPacket *sp = (SFSnortPacket *) p;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   // flow:established, to_server;
   if(checkFlow(p, rule41373options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;

   return DetectOitMemoryCorruption(sp);
}

static int DetectOitMemoryCorruption(SFSnortPacket *sp) { 
   const uint8_t *cursor_normal = 0, *end_of_buffer, *cursor_tmp;
   uint8_t fields, block_type, block_size;
   uint32_t gct_size;
   uint16_t image_width, image_height;
   int i, j;

   // flowbits:isset "file.gif";
   if(processFlowbits(sp, rule41372options[1]->option_u.flowBit) <= 0)
      return RULE_NOALERT;
   
   // file_data;
   #ifndef MISSINGFILEDATA
   if(fileData(sp, rule41372options[2]->option_u.cursor, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   #else
   if(setCursor(sp, rule41372options[2]->option_u.cursor, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   #endif
   
   // content:"GIF8", depth 4, fast_pattern;
   if(contentMatch(sp, rule41372options[3]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   DEBUG_SO(fprintf(stderr,"[GIF]\n");)

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   // skip GIF header
   cursor_normal += 10;

   // make sure we can read packed_fields (1 byte)
   if(cursor_normal + 1 > end_of_buffer)
      return RULE_NOMATCH;

   // read packed fields
   fields = *cursor_normal;

   // skip the rest of the screen descriptor
   cursor_normal += 3;

   // if we have a GCT, skip it.
   if(fields & 0x80)
   {
      // calculate the size of the GCT
      gct_size = 3 * (1 << ((fields & 0x07) + 1));

      DEBUG_SO(fprintf(stderr," GCT size: %d\n",gct_size);)

      // skip the GCT
      cursor_tmp = cursor_normal + gct_size;

      // integer overflow check
      if(cursor_tmp < cursor_normal)
         return RULE_NOMATCH;

      cursor_normal = cursor_tmp;
   }

   // skip up to 5 data blocks
   // looking for the ImageDescriptor
   for(i=0; i<5; i++)
   {
      // make sure we can read data block
      // type, size, ImageWidth, and ImageHeight
      if(cursor_normal + 9 > end_of_buffer)
         return RULE_NOMATCH;

      block_type = *cursor_normal;

      // found the ImageDescriptor
      if(block_type == 0x2C)
      {
         // order doesn't matter
         image_width = *(uint16_t*)(cursor_normal + 5);
         image_height = *(uint16_t*)(cursor_normal + 7);

         // check vuln condition
         if((image_width == 0xFFFF) && (image_height != 0xFFFF))
            return RULE_MATCH;

         // only one ImageDescriptor
         return RULE_NOMATCH;
      }

      // block type is not an extension, bail.
      if(block_type != 0x21)
         return RULE_NOMATCH;

      // read block_size (verified we can above)
      block_size = *(cursor_normal + 2);

      DEBUG_SO(fprintf(stderr," block 0x%02X size: %d\n",block_type,block_size);)

      // calculate the end of block position
      cursor_tmp = cursor_normal + block_size + 3;

      // integer overflow check
      if(cursor_tmp < cursor_normal)
         return RULE_NOMATCH;

      // skip the block
      cursor_normal = cursor_tmp;

      // process up to 5 data sub-blocks
      for(j=0; j<5; j++)
      {
         // make sure we can read data sub-block size
         if(cursor_normal + 1 > end_of_buffer)
            return RULE_NOMATCH;

         block_size = *cursor_normal++;

         DEBUG_SO(fprintf(stderr,"  sub-block size: %d\n",block_size);) 

         // if sub block_size is 0, end of block 
         if(block_size == 0)
            break;

         // calculate the end of sub-block position
         cursor_tmp = cursor_normal + block_size;

         // integer overflow check
         if(cursor_tmp < cursor_normal)
            return RULE_NOMATCH;

         // skip the sub-block
         cursor_normal = cursor_tmp;
      }
   }

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule41372,
    &rule41373,
    NULL
};
*/
