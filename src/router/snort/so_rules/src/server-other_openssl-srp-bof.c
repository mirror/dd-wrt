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

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"
#include "so-util.h"

// TLS handshake types
#define HS_SERVER_HELLO 2  // server hello
#define HS_SERVER_KEYX  12 // server key exchange
#define HS_CLIENT_KEYX  16 // client key exchange

// TLS session state
typedef enum {
   Unknown,
   ServerHello,
   ServerKeyX,
   ClientKeyX
} SessionState;

// SRP flow data
typedef struct {
   SessionState state;
   uint16_t prime_len;
} SRPFlowData;

/* declare detection functions */
int rule59879eval(void *p);
int rule59880eval(void *p);

/* declare rule data structures */
/* flow:established, to_client; */
static FlowFlags rule59879flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_CLIENT
};

static RuleOption rule59879option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule59879flow0
   }
};

/* ssl_state:server_hello,server_keyx; */
static PreprocessorOption rule59879ssl_state1 =
{
   "ssl_state",
   "server_hello,server_keyx",
   0,
   NULL,
   NULL,
   NULL
};

static RuleOption rule59879option1 =
{
   OPTION_TYPE_PREPROCESSOR,
   {
      &rule59879ssl_state1
   }
};

// content:"|16 03|", offset 0, depth 2, fast_pattern; 
static ContentInfo rule59879content2 = 
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

static RuleOption rule59879option2 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule59879content2
   }
};

/* references for sid 59879 */
/* reference: cve "2014-3512"; */
static RuleReference rule59879ref1 = 
{
   "cve", /* type */
   "2014-3512" /* value */
};

/* reference: url "www.openssl.org/news/secadv/20140806.txt"; */
static RuleReference rule59879ref2 = 
{
   "url", /* type */
   "www.openssl.org/news/secadv/20140806.txt" /* value */
};

static RuleReference *rule59879refs[] =
{
   &rule59879ref1,
   &rule59879ref2,
   NULL
};

/* metadata for sid 59879 */
/* metadata:service ssl, policy max-detect-ips alert; */
static RuleMetaData rule59879service0 = 
{
   "service ssl"
};

static RuleMetaData rule59879policy1 = 
{
   "policy max-detect-ips alert"
};

static RuleMetaData *rule59879metadata[] =
{
   &rule59879service0,
   &rule59879policy1,
   NULL
};

RuleOption *rule59879options[] =
{
   &rule59879option0,
   &rule59879option1,
   &rule59879option2,
   NULL
};

Rule rule59879 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_TCP, /* proto */
      "$HOME_NET", /* SRCIP     */
      "443", /* SRCPORT   */
      0, /* DIRECTION */
      "$EXTERNAL_NET", /* DSTIP     */
      "any", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      59879, /* sigid */
      1, /* revision */
      "protocol-command-decode", /* classification */
      0,  /* hardcoded priority */
      "SERVER-OTHER OpenSSL SRP ciphersuite detected",     /* message */
      rule59879refs, /* ptr to references */
      rule59879metadata /* ptr to metadata */
   },
   rule59879options, /* ptr to rule options */
   &rule59879eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};


/* flow:established, to_server; */
static FlowFlags rule59880flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule59880option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule59880flow0
   }
};

/* ssl_state:client_keyx; */
static PreprocessorOption rule59880ssl_state1 =
{
   "ssl_state",
   "client_keyx",
   0,
   NULL,
   NULL,
   NULL
};

static RuleOption rule59880option1 =
{
   OPTION_TYPE_PREPROCESSOR,
   {
      &rule59880ssl_state1
   }
};

// content:"|16 03|", offset 0, depth 2, fast_pattern; 
static ContentInfo rule59880content2 = 
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

static RuleOption rule59880option2 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule59880content2
   }
};

/* references for sid 59880 */
/* reference: cve "2014-3512"; */
static RuleReference rule59880ref1 = 
{
   "cve", /* type */
   "2014-3512" /* value */
};

/* reference: url "www.openssl.org/news/secadv/20140806.txt"; */
static RuleReference rule59880ref2 = 
{
   "url", /* type */
   "www.openssl.org/news/secadv/20140806.txt" /* value */
};

static RuleReference *rule59880refs[] =
{
   &rule59880ref1,
   &rule59880ref2,
   NULL
};

/* metadata for sid 59880 */
/* metadata:service ssl, policy max-detect-ips drop; */
static RuleMetaData rule59880service0 = 
{
   "service ssl"
};

static RuleMetaData rule59880policy1 = 
{
   "policy max-detect-ips drop"
};

static RuleMetaData *rule59880metadata[] =
{
   &rule59880service0,
   &rule59880policy1,
   NULL
};

RuleOption *rule59880options[] =
{
   &rule59880option0,
   &rule59880option1,
   &rule59880option2,
   NULL
};

Rule rule59880 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_TCP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "443", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      59880, /* sigid */
      1, /* revision */
      "attempted-admin", /* classification */
      0,  /* hardcoded priority */
      "SERVER-OTHER OpenSSL SRP heap buffer overflow attempt",     /* message */
      rule59880refs, /* ptr to references */
      rule59880metadata /* ptr to metadata */
   },
   rule59880options, /* ptr to rule options */
   &rule59880eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
static SRPFlowData* getSRPFlowData(SFSnortPacket* sp) {
   SRPFlowData* fd = NULL;

#ifndef BEFORE_2091300
   getRuleData(sp, &(rule59879.info), (void*)(&fd), NULL);
#else
   fd = (SRPFlowData*)getRuleData(sp, rule59879.info.sigID);
#endif

   return fd;
}

static int isSRPServerHello(const uint8_t* cursor_normal,
   const uint8_t* end_of_buffer, SFSnortPacket* sp) {
   // skip:
   //   handshake type (1 byte)
   //   length         (3 bytes)
   //   version        (2 bytes)
   //   random         (32 bytes)
   cursor_normal += 38;

   // check if we can read:
   //   session_id_len (1 byte)
   if(cursor_normal + 1 > end_of_buffer)
      return 0;

   uint8_t session_id_len = *cursor_normal++;

   // check if we can skip session_id_len
   if(session_id_len > end_of_buffer - cursor_normal)
      return 0;

   // skip session_id_len
   cursor_normal += session_id_len;

   // check if we can read:
   //   cipher_suite (2 bytes)
   if(cursor_normal + 2 > end_of_buffer)
      return 0;

   uint16_t cipher_suite = read_big_16(cursor_normal);

   // check for SRP cipher_suite:
   //   0xC01A TLS_SRP_SHA_WITH_3DES_EDE_CBC_SHA
   //   0xC01B TLS_SRP_SHA_RSA_WITH_3DES_EDE_CBC_SHA
   //   0xC01C TLS_SRP_SHA_DSS_WITH_3DES_EDE_CBC_SHA
   //   0xC01D TLS_SRP_SHA_WITH_AES_128_CBC_SHA
   //   0xC01E TLS_SRP_SHA_RSA_WITH_AES_128_CBC_SHA
   //   0xC01F TLS_SRP_SHA_DSS_WITH_AES_128_CBC_SHA
   //   0xC020 TLS_SRP_SHA_WITH_AES_256_CBC_SHA
   //   0xC021 TLS_SRP_SHA_RSA_WITH_AES_256_CBC_SHA
   //   0xC022 TLS_SRP_SHA_DSS_WITH_AES_256_CBC_SHA
   if(cipher_suite < 0xC01A || cipher_suite > 0xC022)
      return 0;

   // SRP cipher_suite found, get the FlowData for this flow
   SRPFlowData* fd = getSRPFlowData(sp);

   // initialize and set the FlowData if it does not exist
   if(!fd)
   {
      fd = (SRPFlowData*)allocRuleData(sizeof(SRPFlowData));

      if(!fd)
         return 0;

#ifndef BEFORE_2091300
      if(storeRuleData(sp, &(rule59879.info), fd, NULL) < 0)
#else
      if(storeRuleData(sp, fd, rule59879.info.sigID, &freeRuleData) < 0)
#endif
      {
         freeRuleData(fd);
         return 0;
      }
   }

   fd->state = ServerHello;
   return 1;
}

static int checkServerKeyX(const uint8_t* cursor_normal,
   const uint8_t* end_of_buffer, SFSnortPacket* sp) {
   // get the FlowData for this flow
   SRPFlowData* fd = getSRPFlowData(sp);

   // if no FlowData, not a SRP session, bail.
   if(!fd)
      return RULE_NOMATCH;

   // check session state
   if(fd->state != ServerHello)
      return RULE_NOMATCH;

   fd->state = ServerKeyX;

   // skip:
   //   handshake type (1 byte)
   //   length         (3 bytes)
   cursor_normal += 4;

   // check if we can read:
   //   prime_len (2 bytes)
   if(cursor_normal + 2 > end_of_buffer)
      return RULE_NOMATCH;

   fd->prime_len = read_big_16(cursor_normal);

   return RULE_NOMATCH;
}

static int checkClientKeyX(const uint8_t* cursor_normal,
   const uint8_t* end_of_buffer, SFSnortPacket* sp) {
   // get the FlowData for this flow
   SRPFlowData* fd = getSRPFlowData(sp);

   // if no FlowData, not a SRP session, bail.
   if(!fd)
      return RULE_NOMATCH;

   // check session state
   if(fd->state != ServerKeyX)
      return RULE_NOMATCH;

   fd->state = ClientKeyX;

   // skip:
   //   handshake type (1 byte)
   //   length         (3 bytes)
   cursor_normal += 4;

   // check if we can read:
   //   pub_val_len (2 bytes)
   if(cursor_normal + 2 > end_of_buffer)
      return RULE_NOMATCH;

   uint16_t pub_val_len = read_big_16(cursor_normal);

   // if pub_val_len > prime_len, alert.
   if(pub_val_len > fd->prime_len)
      return RULE_MATCH;

   return RULE_NOMATCH;
}

static int checkSRPHeapOverflow(const uint8_t* cursor_normal,
   const uint8_t* end_of_buffer, SFSnortPacket* sp) {
   // check up to 5 records
   for(int i = 0; i < 5; i++) {
      // TLS record:
      //   type    (1 byte)
      //   version (2 bytes)
      //   length  (2 bytes)
      //   Handshake Protocol record:
      //     handshake type   (1 byte)
      if(cursor_normal + 6 > end_of_buffer)
         return RULE_NOMATCH;

      // skip type (1 byte)
      cursor_normal += 1;

      // read version (2 bytes)
      uint16_t version = read_big_16_inc(cursor_normal);

      // check version
      //   0x0301 TLSv1.0
      //   0x0302 TLSv1.1
      //   0x0303 TLSv1.2
      if(version < 0x0301 || version > 0x0303)
         return RULE_NOMATCH;

      // read length (2 bytes)
      uint16_t length = read_big_16_inc(cursor_normal);

      // read handshake type (1 byte)
      uint8_t hs_type = *cursor_normal;

      switch(hs_type)
      {
      case HS_SERVER_HELLO:
         if(!isSRPServerHello(cursor_normal, end_of_buffer, sp))
            return RULE_NOMATCH;
         else
            break;
      case HS_SERVER_KEYX:
         return checkServerKeyX(cursor_normal, end_of_buffer, sp);
      case HS_CLIENT_KEYX:
         return checkClientKeyX(cursor_normal, end_of_buffer, sp);
      default:
         break;
      }

      // check if we can skip length
      if(length > end_of_buffer - cursor_normal)
         return RULE_NOMATCH;

      // skip length
      cursor_normal += length;
   }

   return RULE_NOMATCH;
}

int rule59879eval(void *p) {
   const uint8_t *cursor_normal = NULL, *end_of_buffer;
   SFSnortPacket *sp = (SFSnortPacket*)p;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   // flow:established, to_client;
   if(checkFlow(p, rule59879options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;

   // ssl_state:server_hello,server_keyx;
   if(preprocOptionEval(p, rule59879options[1]->option_u.preprocOpt, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   // content:"|16 03|", offset 0, depth 2, fast_pattern;
   if(contentMatch(p, rule59879options[2]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   if(getBuffer(p, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   return checkSRPHeapOverflow(cursor_normal, end_of_buffer, sp);
}

int rule59880eval(void *p) {
   const uint8_t *cursor_normal = NULL, *end_of_buffer;
   SFSnortPacket *sp = (SFSnortPacket*)p;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   // flow:established, to_server;
   if(checkFlow(p, rule59880options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;

   // ssl_state:client_keyx;
   if(preprocOptionEval(p, rule59880options[1]->option_u.preprocOpt, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   // content:"|16 03|", offset 0, depth 2, fast_pattern;
   if(contentMatch(p, rule59880options[2]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   if(getBuffer(p, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   return checkSRPHeapOverflow(cursor_normal, end_of_buffer, sp);
}

/*
Rule *rules[] = {
    &rule59879,
    &rule59880,
    NULL
};
*/
