/*
 * Copyright (C) 2022-2022 Cisco and/or its affiliates. All rights reserved.
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

#include <string.h>

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"

#include "so-util.h"
#include "so-util_ber.h"

// TLS record types
#define TLS_HS 22

// TLS handshake types
#define HS_CERT 11

// ASN.1 time types
#define ASN1_TIME_UTC 23
#define ASN1_TIME_GEN 24

/* declare detection functions */
int rule59646eval(void *p);

/* declare rule data structures */
/* flow:to_client,established; */
static FlowFlags rule59646flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_CLIENT
};

static RuleOption rule59646option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule59646flow0
   }
};

/* ssl_state:server_hello,server_keyx; */
static PreprocessorOption rule59646ssl_state1 =
{
   "ssl_state",
   "server_hello,server_keyx",
   0,
   NULL,
   NULL,
   NULL
};

static RuleOption rule59646option1 =
{
   OPTION_TYPE_PREPROCESSOR,
   {
      &rule59646ssl_state1
   }
};

// content:"|16 03|", offset 0, depth 2, fast_pattern; 
static ContentInfo rule59646content2 = 
{
   (uint8_t *) "|16 03|", /* pattern */
   2, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN|CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule59646option2 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule59646content2
   }
};

/* references for sid 59646 */
/* reference: cve "2015-1789"; */
static RuleReference rule59646ref1 = 
{
   "cve", /* type */
   "2015-1789" /* value */
};

/* reference: url "www.openssl.org/news/secadv/20150611.txt"; */
static RuleReference rule59646ref2 = 
{
   "url", /* type */
   "www.openssl.org/news/secadv/20150611.txt" /* value */
};

static RuleReference *rule59646refs[] =
{
   &rule59646ref1,
   &rule59646ref2,
   NULL
};

/* metadata for sid 59646 */
/* metadata:service ssl, policy max-detect-ips drop; */
static RuleMetaData rule59646service0 = 
{
   "service ssl"
};

static RuleMetaData rule59646policy1 = 
{
   "policy max-detect-ips drop"
};

static RuleMetaData *rule59646metadata[] =
{
   &rule59646service0,
   &rule59646policy1,
   NULL
};

RuleOption *rule59646options[] =
{
   &rule59646option0,
   &rule59646option1,
   &rule59646option2,
   NULL
};

Rule rule59646 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_TCP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "443", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "any", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      59646, /* sigid */
      1, /* revision */
      "attempted-dos", /* classification */
      0,  /* hardcoded priority */
      "SERVER-OTHER OpenSSL X509_cmp_time out of bounds read attempt",     /* message */
      rule59646refs, /* ptr to references */
      rule59646metadata /* ptr to metadata */
   },
   rule59646options, /* ptr to rule options */
   &rule59646eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

/* detection functions */
static int hasSign(const uint8_t* buf, size_t len) {
   if(len == 0)
       return 0;

   if(memchr(buf, '+', len))
       return 1;

   if(memchr(buf, '-', len))
       return 1;

   return 0;
}

int rule59646eval(void *p) {
   const uint8_t *cursor_normal = NULL, *beg_of_buffer, *end_of_buffer;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   // flow:to_client,established;
   if(checkFlow(p, rule59646options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;

   // ssl_state:server_hello,server_keyx;
   if(preprocOptionEval(p, rule59646options[1]->option_u.preprocOpt, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   // content:"|16 03|", offset 0, depth 2, fast_pattern;
   if(contentMatch(p, rule59646options[2]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_buffer, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   cursor_normal = beg_of_buffer;

   // check if we can read the record length and type
   if(cursor_normal + 6 > end_of_buffer)
      return RULE_NOMATCH;

   // skip the first HS if it is not a cert
   if(cursor_normal[5] != HS_CERT) {
     // skip:
     //  TLS Type (1 byte)
     //  TLS Version (2 byte)
     cursor_normal += 3;

     // read the record length
     uint16_t rec_len = read_big_16_inc(cursor_normal);

     // check if we can jump rec_len
     if(rec_len > end_of_buffer - cursor_normal)
        return RULE_NOMATCH;

     cursor_normal += rec_len;

     // check if we can read the next HS 
     if(cursor_normal + 6 > end_of_buffer)
        return RULE_NOMATCH;

     // verify we landed on a cert
     if(cursor_normal[5] != HS_CERT || cursor_normal[0] != TLS_HS)
        return RULE_NOMATCH;
   }

   // skip:
   //  TLS Type (1 byte)
   //  Handshake Version (2 bytes)
   //  Handshake Length (2 bytes)
   //  Handshake Type (1 bytes)
   //  HS Proto Length (3 bytes)
   //  Certificates Length (3 bytes)
   //  and 1st Certificate Length (3 bytes)
   cursor_normal += 15;

   BER_DATA(0x30); // Certificate ::= SEQUENCE 0x30
   BER_DATA(0x30); //    tbsCertificate ::= SEQUENCE 0x30
   BER_SKIP(0xA0); //       version      0xA0
   BER_SKIP(0x02); //       serialNumber 0x02
   BER_SKIP(0x30); //       signature    SEQUENCE 0x30
   BER_SKIP(0x30); //       issuer       SEQUENCE 0x30
   BER_DATA(0x30); //       validity ::= SEQUENCE 0x30
   
   // validity ::= SEQUENCE {
   //    notBefore Time
   //    notAfter  Time
   // }
   //
   // Time ::= CHOICE {
   //    utcTime      UTCTime         0x17
   //    generalTime  GeneralizedTime 0x18
   // }

   // check the notBefore and notAfter times
   for(int i = 0; i < 2; i++)
   {
      BER_ELEMENT time;

      if(ber_get_element(sp, cursor_normal, &time) < 0)
         return RULE_NOMATCH;
  
      // time must be one of the supported types
      if(time.type != ASN1_TIME_UTC && time.type != ASN1_TIME_GEN)
         return RULE_NOMATCH;

      // move cursor to the time data
      cursor_normal = time.data.data_ptr;

      // sanity check
      if(cursor_normal > end_of_buffer || cursor_normal < beg_of_buffer)
          return RULE_NOMATCH;

      uint32_t length = time.data_len;

      if(length == 0)
         continue;

      // check if we can read length
      if(length > end_of_buffer - cursor_normal)
         return RULE_NOMATCH;

      uint32_t offset = 0;

      if(length > 4)
         offset = length - 4;

      // if the offset sign is in the last 4 bytes, alert
      if(hasSign(cursor_normal + offset, length - offset))
         return RULE_MATCH;

      // jump length
      cursor_normal += length;
   }

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule59646,
    NULL
};
*/
