/*
 * This file contains two rules that share common code and data structures for detection.
 *
 */

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
int ruleCVE_2013_3906eval(void *p, uint32_t (* fn_read32)(const uint8_t *p),
                          uint16_t (* fn_read16)(const uint8_t *p),
                          uint8_t (* fn_read32_msb)(const uint8_t *p),
                          RuleOption *loop_option);
int rule28487eval(void *p);
int rule28488eval(void *p);

/* declare rule data structures */
/* flow:established, to_client; */
static FlowFlags ruleCVE_2013_3906flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_CLIENT
};

static RuleOption ruleCVE_2013_3906option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &ruleCVE_2013_3906flow0
   }
};

// file_data;
static CursorInfo ruleCVE_2013_3906file_data1 =
{
   0, /* offset */
   CONTENT_BUF_NORMALIZED /* flags */
};

static RuleOption ruleCVE_2013_3906option1 =
{
#ifndef MISSINGFILEDATA
   OPTION_TYPE_FILE_DATA,
#else
   OPTION_TYPE_SET_CURSOR,
#endif
   {
      &ruleCVE_2013_3906file_data1
   }
};

#ifndef CONTENT_FAST_PATTERN_ONLY
#define CONTENT_FAST_PATTERN_ONLY CONTENT_FAST_PATTERN
#endif

// content:"|02 02 04 00 01 00 00 00|", depth 0, fast_pattern:only; 
static ContentInfo rule28487content2 = 
{
   (uint8_t *) "|02 02 04 00 01 00 00 00|", /* pattern (now in snort content format) */
   0, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN_ONLY|CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule28487option2 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule28487content2
   }
};

// content:"II|2a 00|", depth 0; 
static ContentInfo rule28487content3 = 
{
   (uint8_t *) "II|2a 00|", /* pattern (now in snort content format) */
   0, /* depth */
   0, /* offset */
   CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule28487option3 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule28487content3
   }
};

// content:"|02 02 00 04 00 00 00 01|", depth 0, fast_pattern:only; 
static ContentInfo rule28488content2 = 
{
   (uint8_t *) "|02 02 00 04 00 00 00 01|", /* pattern (now in snort content format) */
   0, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN_ONLY|CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule28488option2 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule28488content2
   }
};

// content:"MM|00 2a|", depth 0; 
static ContentInfo rule28488content3 = 
{
   (uint8_t *) "MM|00 2a|", /* pattern (now in snort content format) */
   0, /* depth */
   0, /* offset */
   CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule28488option3 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule28488content3
   }
};


/* references */
/* reference: cve "2013-3906"; */
static RuleReference ruleCVE_2013_3906ref1 = 
{
   "cve", /* type */
   "2013-3906" /* value */
};

///* reference: url "technet.microsoft.com/en-us/security/bulletin/MS13-XXX"; */
//static RuleReference ruleCVE_2013_3906ref2 = 
//{
//   "url", /* type */
//   "technet.microsoft.com/en-us/security/bulletin/MS13-XXX" /* value */
//};

static RuleReference *ruleCVE_2013_3906refs[] =
{
   &ruleCVE_2013_3906ref1,
//   &ruleCVE_2013_3906ref2,
   NULL
};

/* metadata for CVE_2013_3906 */
/* metadata:service ftp-data, service http, service imap, service pop3, policy balanced-ips drop, policy security-ips drop; */
static RuleMetaData ruleCVE_2013_3906service1 = 
{
   "service ftp-data"
};

static RuleMetaData ruleCVE_2013_3906service2 = 
{
   "service http"
};

static RuleMetaData ruleCVE_2013_3906service3 = 
{
   "service imap"
};

static RuleMetaData ruleCVE_2013_3906service4 = 
{
   "service pop3"
};

static RuleMetaData ruleCVE_2013_3906policy1 = 
{
   "policy balanced-ips drop"
};

static RuleMetaData ruleCVE_2013_3906policy2 = 
{
   "policy security-ips drop"
};

static RuleMetaData ruleCVE_2013_3906policy3 = 
{
   "policy max-detect-ips drop"
};

static RuleMetaData *ruleCVE_2013_3906metadata[] =
{
   &ruleCVE_2013_3906service1,
   &ruleCVE_2013_3906service2,
   &ruleCVE_2013_3906service3,
   &ruleCVE_2013_3906service4,
   &ruleCVE_2013_3906policy1,
   &ruleCVE_2013_3906policy2,
   &ruleCVE_2013_3906policy3,
   NULL
};

RuleOption *rule28487options[] =
{
   &ruleCVE_2013_3906option0,
   &ruleCVE_2013_3906option1,
   &rule28487option2,
   &rule28487option3,
   NULL
};

Rule rule28487 = {
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
      28487, /* sigid */
      3, /* revision */
      "attempted-user", /* classification */
      0,  /* hardcoded priority */
      "OS-WINDOWS Microsoft GDI library TIFF handling memory corruption attempt",     /* message */
      ruleCVE_2013_3906refs, /* ptr to references */
      ruleCVE_2013_3906metadata /* ptr to metadata */
   },
   rule28487options, /* ptr to rule options */
   &rule28487eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

RuleOption *rule28488options[] =
{
   &ruleCVE_2013_3906option0,
   &ruleCVE_2013_3906option1,
   &rule28488option2,
   &rule28488option3,
   NULL
};

Rule rule28488 = {
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
      28488, /* sigid */
      3, /* revision */
      "attempted-user", /* classification */
      0,  /* hardcoded priority */
      "OS-WINDOWS Microsoft GDI library TIFF handling memory corruption attempt",     /* message */
      ruleCVE_2013_3906refs, /* ptr to references */
      ruleCVE_2013_3906metadata /* ptr to metadata */
   },
   rule28488options, /* ptr to rule options */
   &rule28488eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
// function to check the most significant byte little endian
static inline uint8_t read_little_32_msb(const uint8_t *p)
{
   return *p; 
}

// function to check to most significant byte big endian
static inline uint8_t read_big_32_msb(const uint8_t *p)
{
   return *(p + 3);
}

int rule28487eval(void *p) {
   // little endian tiff with jpeg rec in IFD
   DEBUG_SO(fprintf(stderr,"Little Endian TIFF\n");)

   return ruleCVE_2013_3906eval(p, &read_little_32, &read_little_16, &read_little_32_msb, rule28487options[3]);
}

int rule28488eval(void *p) {
   // little endian tiff with jpeg rec in IFD
   DEBUG_SO(fprintf(stderr,"Big Endian TIFF\n");)

   return ruleCVE_2013_3906eval(p, &read_big_32, &read_big_16, &read_big_32_msb, rule28488options[3]);
}

int ruleCVE_2013_3906eval(void *p, uint32_t (* fn_read32)(const uint8_t *p),
                          uint16_t (* fn_read16)(const uint8_t *p),
                          uint8_t (* fn_read32_msb)(const uint8_t *p),
                          RuleOption *loop_option) {

   const uint8_t *cursor_normal = 0;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   const uint8_t *beg_of_buffer, *end_of_buffer; 
   const uint8_t *exif_magic_pos;
   const uint8_t *cursor_start;
   const uint8_t *cursor_IFD_table, *cursor_IFD_entry;
   uint32_t offset_IFD_table;
   uint16_t num_IFD_entry, tag_ID;
   uint32_t offset_strip_counts;
   uint32_t strip_counts_val;
   const uint8_t *cursor_strip_counts;
   uint32_t num_strip_counts;

   uint16_t i, j;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;
   
   // flow:established, to_client;
   if(checkFlow(p, rule28487options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;
   
   // file_data;
   #ifndef MISSINGFILEDATA
   if(fileData(p, rule28487options[1]->option_u.cursor, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   #else
   if(setCursor(p, rule28487options[1]->option_u.cursor, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   #endif
   
   // the TIFF header preceeds the jpeg rec in the IFD and may
   // be either "II|2a 00|" or "MM|00 2a|" (determined through fast_pattern, in rule eval() funcs)
   if(contentMatch(p, loop_option->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   if(getBuffer(p, CONTENT_BUF_NORMALIZED, &beg_of_buffer, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   // make sure we are not in an EXIF
   // header "Exif"; distance:-10; within:4;
   exif_magic_pos = cursor_normal - 10;

   if(exif_magic_pos >= beg_of_buffer)
   {
      // if we are looking at an EXIF header, bail 
      if(read_big_32(exif_magic_pos) == 0x45786966)
         return RULE_NOMATCH;
   }

   // store the start of the TIFF header
   cursor_start = cursor_normal - 4;

   // check to see if we can read the IFD offset
   if(cursor_normal + 4 > end_of_buffer)
      return RULE_NOMATCH;

   // get the IFD offset
   offset_IFD_table = (*fn_read32)(cursor_normal);
   cursor_IFD_table = cursor_start + offset_IFD_table;
   DEBUG_SO(fprintf(stderr,"offset_IFD_table = 0x%08x\n",offset_IFD_table);)

   // check for integer overflow
   if(cursor_IFD_table <= cursor_start)
      return RULE_NOMATCH;

   // check if we can read the number of entries in the IFD 
   if(cursor_IFD_table + 2 > end_of_buffer)
      return RULE_NOMATCH;

   num_IFD_entry = (*fn_read16)(cursor_IFD_table);
   cursor_IFD_entry = cursor_IFD_table + 2;
   DEBUG_SO(fprintf(stderr,"num_IFD_entry = 0x%04x\n",num_IFD_entry);)

   // look for a StripByteCounts rec (0x0117) in up to 15 IFD entries
   if(num_IFD_entry > 15)
      num_IFD_entry = 15;

   strip_counts_val = 0;
   num_strip_counts = 0;

   for(i = 0; i < num_IFD_entry; i++)
   {
      // check to see if we can read the IFD
      if(cursor_IFD_entry + 12 > end_of_buffer)
         return RULE_NOMATCH;

      tag_ID = (*fn_read16)(cursor_IFD_entry);
      DEBUG_SO(fprintf(stderr,"Tag ID : 0x%04x\n",tag_ID);)

      if(tag_ID == 0x0117)
      {
         cursor_IFD_entry += 4; // move the cursor to point to num_strip_counts
         num_strip_counts = (*fn_read32)(cursor_IFD_entry);
         cursor_IFD_entry += 4; // move the cursor to point to offset
         strip_counts_val = (*fn_read32)(cursor_IFD_entry); 
         DEBUG_SO(fprintf(stderr,"num_strip_counts : 0x%08x\n",num_strip_counts);)
         DEBUG_SO(fprintf(stderr,"strip_counts_val : 0x%08x\n",strip_counts_val);)
         break;
      }
      else
      {
         cursor_IFD_entry += 12;
      }
   }

   if(num_strip_counts == 0)
   {
      // no (0x0117) rec found, or no val, bail
      return RULE_NOMATCH;
   }
   if(num_strip_counts == 1)
   {
      // trivial, strip_counts_val is the val to check
      // if the MSB is not 0, then the jpegStreamSize is sufficently large to alert
      if(strip_counts_val & 0xff000000)
         return RULE_MATCH;
   }
   else
   {
      // strip_counts_val is an offset to an array of strip counts
      offset_strip_counts = strip_counts_val;

      cursor_strip_counts = cursor_start + offset_strip_counts;

      // check for integer overflow
      if(cursor_strip_counts <= cursor_start)
         return RULE_NOMATCH;

      // check up to 15 StripByteCounts values for the vuln condition
      if(num_strip_counts > 15)
         num_strip_counts = 15;

      for(j = 0; j < num_strip_counts; j++)
      {
         if(cursor_strip_counts + 4 > end_of_buffer)
            return RULE_NOMATCH;

         // if the MSB is not 0, then the jpegStreamSize is sufficently large to alert
         if((*fn_read32_msb)(cursor_strip_counts) != 0)
            return RULE_MATCH;
      }
   }

   return RULE_NOMATCH;
}
