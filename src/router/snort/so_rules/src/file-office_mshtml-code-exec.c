/*
 * Copyright (C) 2021-2021 Cisco and/or its affiliates. All rights reserved.
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

#include <zlib.h>
#include <string.h>

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"
#include "so-util.h"

#define DECOMPRESSED_BUF_SIZE 8192
#define ZLIB_INPUT_CAP 8192
#define SKIPTABLE_SIZE 256

// Boyer-Moore skiptable
static uint32_t skiptable[SKIPTABLE_SIZE];
static uint8_t skiptable_init = 0;

// Boyer-Moore pattern
static const char* pattern = "!x-usc:";
static const int pattern_len = 7;

// buffer for storing uncompressed data
static uint8_t decompressed_buffer[DECOMPRESSED_BUF_SIZE];

/* declare detection functions */
int rule58130eval(void *p);
int rule58131eval(void *p);
static int DetectMSHTMLCodeExec(SFSnortPacket *sp);

/* declare rule data structures */
/* flow:established, to_client; */
static FlowFlags rule58130flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_CLIENT
};

static RuleOption rule58130option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule58130flow0
   }
};

/* flow:established, to_server; */
static FlowFlags rule58131flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule58131option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule58131flow0
   }
};

// file_data;
static CursorInfo rule_MSHTMLCodeExec_file_data1 =
{
   0, /* offset */
   CONTENT_BUF_NORMALIZED /* flags */
};

static RuleOption rule_MSHTMLCodeExec_option1 =
{
#ifndef MISSINGFILEDATA
   OPTION_TYPE_FILE_DATA,
#else
   OPTION_TYPE_SET_CURSOR,
#endif
   {
      &rule_MSHTMLCodeExec_file_data1
   }
};

// content:"PK|03 04|", offset 0, depth 0, fast_pattern, relative; 
static ContentInfo rule_MSHTMLCodeExec_content2 = 
{
   (uint8_t *) "PK|03 04|", /* pattern */
   0, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN|CONTENT_BUF_NORMALIZED|CONTENT_RELATIVE, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule_MSHTMLCodeExec_option2 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule_MSHTMLCodeExec_content2
   }
};

// content:"word/", offset 26, depth 5, nocase, relative; 
static ContentInfo rule_MSHTMLCodeExec_content3 = 
{
   (uint8_t *) "word/", /* pattern */
   5, /* depth */
   26, /* offset */
   CONTENT_NOCASE|CONTENT_BUF_NORMALIZED|CONTENT_RELATIVE, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule_MSHTMLCodeExec_option3 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule_MSHTMLCodeExec_content3
   }
};

// content:"|08 00|", offset -27, depth 2, relative; 
static ContentInfo rule_MSHTMLCodeExec_content4 = 
{
   (uint8_t *) "|08 00|", /* pattern */
   2, /* depth */
   -27, /* offset */
   CONTENT_BUF_NORMALIZED|CONTENT_RELATIVE, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule_MSHTMLCodeExec_option4 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule_MSHTMLCodeExec_content4
   }
};

/* references */
/* reference: cve "2021-40444"; */
static RuleReference rule_MSHTMLCodeExec_ref1 = 
{
   "cve", /* type */
   "2021-40444" /* value */
};

/* reference: url "msrc.microsoft.com/update-guide/vulnerability/CVE-2021-40444"; */
static RuleReference rule_MSHTMLCodeExec_ref2 = 
{
   "url", /* type */
   "msrc.microsoft.com/update-guide/vulnerability/CVE-2021-40444" /* value */
};

static RuleReference *rule_MSHTMLCodeExec_refs[] =
{
   &rule_MSHTMLCodeExec_ref1,
   &rule_MSHTMLCodeExec_ref2,
   NULL
};

/* metadata for sid 58130 */
/* metadata:service ftp-data, service http, service imap, service pop3, policy max-detect-ips drop, policy security-ips drop; */
static RuleMetaData rule58130service0 = 
{
   "service ftp-data"
};

static RuleMetaData rule58130service1 = 
{
   "service http"
};

static RuleMetaData rule58130service2 = 
{
   "service imap"
};

static RuleMetaData rule58130service3 = 
{
   "service pop3"
};

static RuleMetaData rule58130policy4 = 
{
   "policy max-detect-ips drop"
};

static RuleMetaData rule58130policy5 = 
{
   "policy security-ips drop"
};

static RuleMetaData *rule58130metadata[] =
{
   &rule58130service0,
   &rule58130service1,
   &rule58130service2,
   &rule58130service3,
   &rule58130policy4,
   &rule58130policy5,
   NULL
};

/* metadata for sid 58131 */
/* metadata:service smtp, policy max-detect-ips drop, policy security-ips drop; */
static RuleMetaData rule58131service0 = 
{
   "service smtp"
};

static RuleMetaData rule58131policy1 = 
{
   "policy max-detect-ips drop"
};

static RuleMetaData rule58131policy2 = 
{
   "policy security-ips drop"
};

static RuleMetaData *rule58131metadata[] =
{
   &rule58131service0,
   &rule58131policy1,
   &rule58131policy2,
   NULL
};

RuleOption *rule58130options[] =
{
   &rule58130option0,
   &rule_MSHTMLCodeExec_option1,
   &rule_MSHTMLCodeExec_option2,
   &rule_MSHTMLCodeExec_option3,
   &rule_MSHTMLCodeExec_option4,
   NULL
};

RuleOption *rule58131options[] =
{
   &rule58131option0,
   &rule_MSHTMLCodeExec_option1,
   &rule_MSHTMLCodeExec_option2,
   &rule_MSHTMLCodeExec_option3,
   &rule_MSHTMLCodeExec_option4,
   NULL
};

Rule rule58130 = {
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
      58130, /* sigid */
      1, /* revision */
      "attempted-user", /* classification */
      0,  /* hardcoded priority */
      "FILE-OFFICE Microsoft MSHTML code execution attempt",     /* message */
      rule_MSHTMLCodeExec_refs, /* ptr to references */
      rule58130metadata /* ptr to metadata */
   },
   rule58130options, /* ptr to rule options */
   &rule58130eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

Rule rule58131 = {
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
      58131, /* sigid */
      1, /* revision */
      "attempted-user", /* classification */
      0,  /* hardcoded priority */
      "FILE-OFFICE Microsoft MSHTML code execution attempt",     /* message */
      rule_MSHTMLCodeExec_refs, /* ptr to references */
      rule58131metadata /* ptr to metadata */
   },
   rule58131options, /* ptr to rule options */
   &rule58131eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

/* detection functions */
static uint8_t* boyer_moore(uint8_t* haystack, uint32_t haystack_len,
                            uint8_t* needle,   uint32_t needle_len)
{
   uint32_t last, scan = 0;

   // sanity checks
   if(haystack_len == 0 || needle_len == 0)
      return NULL;

   // needle must fit in haystack
   if(haystack_len < needle_len)
      return NULL;
 
   // store the last needle byte position
   last = needle_len - 1;

   // Initialize the skip table if needed
   //
   // this can be done once per SO since the
   // skiptable is based entirely on the needle
   if(skiptable_init == 0)
   {
      // If we encounter a byte that does not occur
      // in the needle, we can safely skip ahead for
      // the entire length of the needle
      for(scan = 0; scan < SKIPTABLE_SIZE; scan++)
         skiptable[scan] = needle_len;
 
      // populate the skip table with the analysis of the needle
      for(scan = 0; scan < last; scan++)
         skiptable[needle[scan]] = last - scan;

      skiptable_init = 1;
   }
 
   // Search while needle can fit in haystack
   //
   while(haystack_len >= needle_len)
   {
      // scan from the end of the needle
      for(scan = last; haystack[scan] == needle[scan]; scan--)
         if(scan == 0) // if all bytes match, we've found it
            return haystack;

      // otherwise, skip bytes indicated in skip table
      //
      // we get the skip value from the last byte of needle
      // no matter where we didn't match
      haystack_len -= skiptable[haystack[last]];
      haystack += skiptable[haystack[last]];
   }

   // needle not in haystack 
   return NULL;
}

int rule58130eval(void *p) {
   SFSnortPacket *sp = (SFSnortPacket *) p;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   // flow:established, to_client;
   if(checkFlow(p, rule58130options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;

   return DetectMSHTMLCodeExec(sp);
}

int rule58131eval(void *p) {
   SFSnortPacket *sp = (SFSnortPacket *) p;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   // flow:established, to_server;
   if(checkFlow(p, rule58131options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;

   return DetectMSHTMLCodeExec(sp);
}

static int DetectMSHTMLCodeExec(SFSnortPacket *sp) {
   const uint8_t *cursor_normal = 0, *end_of_buffer;

   z_stream stream;

   // file_data;
   #ifndef MISSINGFILEDATA
   if(fileData(sp, rule58130options[1]->option_u.cursor, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   #else
   if(setCursor(sp, rule58130options[1]->option_u.cursor, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   #endif

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   // scan up to 25 files
   for(int i = 0; i < 25; i++)
   {
      // content:"PK|03 04|", offset 0, depth 0, fast_pattern, relative;
      if(contentMatch(sp, rule58130options[2]->option_u.content, &cursor_normal) <= 0)
         break;

      const uint8_t* cursor_detect = cursor_normal;

      // content:"word/", offset 26, depth 5, nocase, relative;
      if(contentMatch(sp, rule58130options[3]->option_u.content, &cursor_detect) <= 0)
         continue;
      
      // content:"|08 00|", offset -27, depth 2, relative;
      if(contentMatch(sp, rule58130options[4]->option_u.content, &cursor_detect) <= 0)
         continue;

      // check if we can:
      //  skip mod time           (2 bytes)
      //  skip mod date           (2 bytes)
      //  skip CRC32              (4 bytes)
      //  skip compressed size    (4 bytes)
      //  skip uncompressed size  (4 bytes)
      //  read filename length    (2 bytes)
      //  read extra length       (2 bytes)
      //   = (20 bytes)
      if(cursor_detect + 20 > end_of_buffer)
         return RULE_NOMATCH;

      cursor_detect += 16;

      uint16_t filename_len = read_little_16_inc(cursor_detect);
      uint16_t extra_len = read_little_16_inc(cursor_detect);

      // check if we can jump filename_len and extra_len
      uint32_t jump = filename_len + extra_len;

      if(jump > end_of_buffer - cursor_detect)
         return RULE_NOMATCH;

      // jump filename_len and extra_len
      cursor_detect += jump;

      uint32_t available_bytes = end_of_buffer - cursor_detect;

      // cap input to zlib
      if(available_bytes > ZLIB_INPUT_CAP)
         available_bytes = ZLIB_INPUT_CAP;

      // setup zlib stream
      memset(&stream, 0, sizeof(stream));

      stream.avail_in  = available_bytes;
      stream.next_in   = (uint8_t*)cursor_detect;
      stream.avail_out = sizeof(decompressed_buffer);
      stream.next_out  = decompressed_buffer;

      // init zlib
      if(inflateInit2(&stream, -MAX_WBITS) != Z_OK)
         return RULE_NOMATCH;

      // decompress
      int ret = inflate(&stream, Z_SYNC_FLUSH);

      if(ret != Z_OK && ret != Z_STREAM_END)
      {
         inflateEnd(&stream);
         return RULE_NOMATCH;
      }

      // decompression successful
      uint32_t decompressed_len = stream.total_out;

      // shutdown zlib
      inflateEnd(&stream);

      // sanity check
      if(decompressed_len > sizeof(decompressed_buffer))
         return RULE_NOMATCH;

      // search for MSHTML code exec
      uint8_t* pos = boyer_moore(decompressed_buffer, decompressed_len,
         (uint8_t*)pattern, pattern_len);

      if(pos != NULL)
         return RULE_MATCH;
   }

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule58130,
    &rule58131,
    NULL
};
*/
