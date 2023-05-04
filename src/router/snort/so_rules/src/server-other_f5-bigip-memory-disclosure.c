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

// tls handshake types
#define HS_CLIENT_HELLO 1 // client hello
#define HS_SERVER_HELLO 2 // server hello

// tls record datatype
typedef struct {
   uint16_t version;        // tls record version
   uint8_t  hs_type;        // tls handshake type
   uint16_t hs_version;     // tls handshake version
   uint8_t  session_id_len; // tls session id length
   uint8_t  session_id_msb; // tls session id msb
} tls_record;

// tls session datatype
typedef struct {
   uint8_t id_len; // session id length
   uint8_t id_msb; // session id msb 
} tls_session_data;

/* declare detection functions */
int rule41547eval(void *p);
int rule41548eval(void *p);

/* declare rule data structures */
/* flow:established, to_server; */
static FlowFlags rule41547flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule41547option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule41547flow0
   }
};

/* ssl_state:client_hello; */
static PreprocessorOption rule41547ssl_state1 = 
{
   "ssl_state",
   "client_hello",
   0,
   NULL,
   NULL,
   NULL
};

static RuleOption rule41547option1 = 
{
   OPTION_TYPE_PREPROCESSOR,
   {
      &rule41547ssl_state1
   }
};

// content:"|16 03|", depth 2, fast_pattern; 
static ContentInfo rule41547content2 = 
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

static RuleOption rule41547option2 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule41547content2
   }
};

/* references for sid 41547 */
/* reference: url "tools.ietf.org/html/rfc5077"; */
static RuleReference rule41547ref1 = 
{
   "url", /* type */
   "tools.ietf.org/html/rfc5077" /* value */
};

static RuleReference *rule41547refs[] =
{
   &rule41547ref1,
   NULL
};

/* metadata for sid 41547 */
/* metadata:service ssl, policy balanced-ips alert, policy security-ips alert; */
static RuleMetaData rule41547service1 = 
{
   "service ssl"
};

static RuleMetaData rule41547policy1 = 
{
   "policy balanced-ips alert"
};

static RuleMetaData rule41547policy2 = 
{
   "policy security-ips alert"
};

static RuleMetaData *rule41547metadata[] =
{
   &rule41547service1,
   &rule41547policy1,
   &rule41547policy2,
   NULL
};

RuleOption *rule41547options[] =
{
   &rule41547option0,
   &rule41547option1,
   &rule41547option2,
   NULL
};

Rule rule41547 = {
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
      41547, /* sigid */
      1, /* revision */
      "protocol-command-decode", /* classification */
      0,  /* hardcoded priority */
      "SERVER-OTHER TLS client hello session resumption detected",     /* message */
      rule41547refs, /* ptr to references */
      rule41547metadata /* ptr to metadata */
   },
   rule41547options, /* ptr to rule options */
   &rule41547eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

/* flow:established, to_client; */
static FlowFlags rule41548flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_CLIENT
};

static RuleOption rule41548option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule41548flow0
   }
};

/* ssl_state:server_hello; */
static PreprocessorOption rule41548ssl_state1 = 
{
   "ssl_state",
   "server_hello",
   0,
   NULL,
   NULL,
   NULL
};

static RuleOption rule41548option1 = 
{
   OPTION_TYPE_PREPROCESSOR,
   {
      &rule41548ssl_state1
   }
};

// content:"|16 03|", depth 2, fast_pattern; 
static ContentInfo rule41548content2 = 
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

static RuleOption rule41548option2 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule41548content2
   }
};

/* references for sid 41548 */
/* reference: cve "2016-9244"; */
static RuleReference rule41548ref1 = 
{
   "cve", /* type */
   "2016-9244" /* value */
};

/* reference: url "support.f5.com/csp/article/K05121675"; */
static RuleReference rule41548ref2 = 
{
   "url", /* type */
   "support.f5.com/csp/article/K05121675" /* value */
};

static RuleReference *rule41548refs[] =
{
   &rule41548ref1,
   &rule41548ref2,
   NULL
};

/* metadata for sid 41548 */
/* metadata:service ssl, policy balanced-ips drop, policy security-ips drop; */
static RuleMetaData rule41548service1 = 
{
   "service ssl"
};

static RuleMetaData rule41548policy1 = 
{
   "policy balanced-ips drop"
};

static RuleMetaData rule41548policy2 = 
{
   "policy security-ips drop"
};

static RuleMetaData *rule41548metadata[] =
{
   &rule41548service1,
   &rule41548policy1,
   &rule41548policy2,
   NULL
};

RuleOption *rule41548options[] =
{
   &rule41548option0,
   &rule41548option1,
   &rule41548option2,
   NULL
};

Rule rule41548 = {
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
      41548, /* sigid */
      1, /* revision */
      "attempted-recon", /* classification */
      0,  /* hardcoded priority */
      "SERVER-OTHER F5 BIG-IP TLS session ticket implementation uninitialized memory disclosure attempt",     /* message */
      rule41548refs, /* ptr to references */
      rule41548metadata /* ptr to metadata */
   },
   rule41548options, /* ptr to rule options */
   &rule41548eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule41547eval(void *p) {
   const uint8_t *cursor_normal = 0, *end_of_buffer = 0;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   tls_record record = {0};
   tls_session_data* tls_session = NULL;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;
   
   // flow:established, to_server;
   if(checkFlow(p, rule41547options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;
   
   // ssl_state:client_hello;
   if(preprocOptionEval(p, rule41547options[1]->option_u.preprocOpt, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   
   // content:"|16 03|", depth 2, fast_pattern;
   if(contentMatch(p, rule41547options[2]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   if(getBuffer(p, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   // TLS record:
   //   type    (1 byte)
   //   version (2 bytes)
   //   length  (2 bytes)
   //   Handshake Protocol record:
   //     handshake type    (1 byte)
   //     length            (3 bytes)
   //     version           (2 bytes)
   //     random            (32 bytes)
   //     session id_len    (1 byte)
   //     session id_msb    (1 byte)
   if(cursor_normal + 45 > end_of_buffer)
      return RULE_NOMATCH;

   record.session_id_len = *(cursor_normal+43);

   // if it is an initial session (id_len == 0) (true in most cases)
   // or vuln impossible (id_len >= 32), bail.
   if(record.session_id_len == 0 || record.session_id_len >= 32)
      return RULE_NOMATCH;

   // check tls and hs fields for validity
   record.version    = read_big_16(cursor_normal+1); 
   record.hs_type    = *(cursor_normal+5);
   record.hs_version = read_big_16(cursor_normal+9);

   switch(record.version) {
   case 0x0300: // SSLv3
   case 0x0301: // TLSv1.0
   case 0x0302: // TLSv1.1
   case 0x0303: // TLSv1.2
      break;
   default:
      // unknown tls version, bail.
      return RULE_NOMATCH;
   }

   if(record.hs_type != HS_CLIENT_HELLO)
      return RULE_NOMATCH;

   switch(record.hs_version) {
   case 0x0300: // SSLv3
   case 0x0301: // TLSv1.0
   case 0x0302: // TLSv1.1
   case 0x0303: // TLSv1.2
      break;
   default:
      // unknown handshake version, bail.
      return RULE_NOMATCH;
   }

   // at this point we know we are in a session resumption client
   // hello packet with a session_id_len in the vulnerable range
   record.session_id_msb = *(cursor_normal+44);

   // retrieve the tls_session for this stream
   tls_session = (tls_session_data*)getRuleData(sp, rule41547.info.sigID);

   // allocate and initalize the tls_session if it does not exist
   if(!tls_session)
   {
      tls_session = (tls_session_data*)allocRuleData(sizeof(tls_session_data));

      if(tls_session == NULL)
         return RULE_NOMATCH;

      if(storeRuleData(sp, tls_session, rule41547.info.sigID, &freeRuleData) < 0)
      {
         freeRuleData(tls_session);
         return RULE_NOMATCH;
      }
   }

   // store session id_len and id_msb in tls_session data for future checks
   tls_session->id_len = record.session_id_len;
   tls_session->id_msb = record.session_id_msb;

   return RULE_NOMATCH;
}

int rule41548eval(void *p) {
   const uint8_t *cursor_normal = 0, *end_of_buffer = 0;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   tls_record record = {0};
   tls_session_data* tls_session = NULL;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;
   
   // flow:established, to_client;
   if(checkFlow(p, rule41548options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;
   
   // ssl_state:server_hello;
   if(preprocOptionEval(p, rule41548options[1]->option_u.preprocOpt, &cursor_normal) <= 0)
      return RULE_NOMATCH;
   
   // content:"|16 03|", depth 2, fast_pattern;
   if(contentMatch(p, rule41548options[2]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   if(getBuffer(p, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   // TLS record:
   //   type    (1 byte)
   //   version (2 bytes)
   //   length  (2 bytes)
   //   Handshake Protocol record:
   //     handshake type    (1 byte)
   //     length            (3 bytes)
   //     version           (2 bytes)
   //     random            (32 bytes)
   //     session id_len    (1 byte)
   //     session id_msb    (1 byte)
   if(cursor_normal + 45 > end_of_buffer)
      return RULE_NOMATCH;

   record.session_id_len = *(cursor_normal+43);

   // F5 BIG-IP always sets session_id_len to 32
   if(record.session_id_len != 32)
      return RULE_NOMATCH;

   // retrieve the tls_session for this stream
   tls_session = (tls_session_data*)getRuleData(sp, rule41547.info.sigID);

   // if no tls_session data, server is defining a new session, bail.
   if(!tls_session)
   {
      DEBUG_SO(fprintf(stderr,"no tls_session data, server defining new session, bailing.\n");)
      return RULE_NOMATCH;
   }

   // check tls and hs fields for validity
   record.version    = read_big_16(cursor_normal+1); 
   record.hs_type    = *(cursor_normal+5);
   record.hs_version = read_big_16(cursor_normal+9);

   switch(record.version) {
   case 0x0300: // SSLv3
   case 0x0301: // TLSv1.0
   case 0x0302: // TLSv1.1
   case 0x0303: // TLSv1.2
      break;
   default:
      // unknown tls version, bail.
      return RULE_NOMATCH;
   }

   if(record.hs_type != HS_SERVER_HELLO)
      return RULE_NOMATCH;

   switch(record.hs_version) {
   case 0x0300: // SSLv3
   case 0x0301: // TLSv1.0
   case 0x0302: // TLSv1.1
   case 0x0303: // TLSv1.2
      break;
   default:
      // unknown handshake version, bail.
      return RULE_NOMATCH;
   }

   record.session_id_msb = *(cursor_normal+44);

   DEBUG_SO(fprintf(stderr,"tls_session->id_len   = %d\n",tls_session->id_len);)
   DEBUG_SO(fprintf(stderr,"record.session_id_len = %d\n",record.session_id_len);)
   DEBUG_SO(fprintf(stderr,"tls_session->id_msb   = %02X\n",tls_session->id_msb);)
   DEBUG_SO(fprintf(stderr,"record.session_id_msb = %02X\n\n",record.session_id_msb);)

   // server is resuming a previous session, check vuln condition.
   if(tls_session->id_len < record.session_id_len)
      if(tls_session->id_msb == record.session_id_msb)
         return RULE_MATCH;
   
   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule41547,
    &rule41548,
    NULL
};
*/
