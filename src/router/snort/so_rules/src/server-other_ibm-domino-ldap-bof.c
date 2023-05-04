/*
 * Copyright (C) 2005-2015 Sourcefire, Inc. All Rights Reserved
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

#include "so-util_ber.h"

/* declare detection functions */
int rule36153eval(void *p);

/* declare rule data structures */
/* flow:established, to_server; */
static FlowFlags rule36153flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule36153option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule36153flow0
   }
};

// content:"|3B|", fast_pattern:only; 
static ContentInfo rule36153content1 = 
{
   (uint8_t *) "|3B|", /* pattern */
   0, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN_ONLY|CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule36153option1 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule36153content1
   }
};

/* references for sid 36153 */
/* reference: bugtraq "73911"; */
static RuleReference rule36153ref1 = 
{
   "bugtraq", /* type */
   "73911" /* value */
};

/* reference: cve "2015-0117"; */
static RuleReference rule36153ref2 = 
{
   "cve", /* type */
   "2015-0117" /* value */
};

/* reference: url "osvdb.org/show/osvdb/120075"; */
static RuleReference rule36153ref3 = 
{
   "url", /* type */
   "osvdb.org/show/osvdb/120075" /* value */
};

static RuleReference *rule36153refs[] =
{
   &rule36153ref1,
   &rule36153ref2,
   &rule36153ref3,
   NULL
};

/* metadata for sid 36153 */
/* metadata:service ldap, policy balanced-ips drop, policy security-ips drop; */
static RuleMetaData rule36153service1 = 
{
   "service ldap"
};

static RuleMetaData rule36153policy1 = 
{
   "policy balanced-ips drop"
};

static RuleMetaData rule36153policy2 = 
{
   "policy security-ips drop"
};

static RuleMetaData rule36153policy3 = 
{
   "policy max-detect-ips drop"
};

static RuleMetaData *rule36153metadata[] =
{
   &rule36153service1,
   &rule36153policy1,
   &rule36153policy2,
   &rule36153policy3,
   NULL
};

RuleOption *rule36153options[] =
{
   &rule36153option0,
   &rule36153option1,
   NULL
};

Rule rule36153 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_TCP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "389", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      36153, /* sigid */
      3, /* revision */
      "attempted-admin", /* classification */
      0,  /* hardcoded priority */
      "SERVER-OTHER IBM Domino LDAP server ModifyRequest stack buffer overflow attempt",     /* message */
      rule36153refs, /* ptr to references */
      rule36153metadata /* ptr to metadata */
   },
   rule36153options, /* ptr to rule options */
   &rule36153eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule36153eval(void *p) {
   const uint8_t *cursor_normal = 0, *end_of_buffer,
                 *next_mod_item, *next_msg;
   SFSnortPacket *sp = (SFSnortPacket *) p;
   BER_ELEMENT msg, req, mod, attribute;
   int i,j, bytes_available, stop;
   unsigned int subtag_len;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;
   
   // flow:established, to_server;
   if(checkFlow(p, rule36153options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;
   
   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   // LDAPMessage ::= SEQUENCE [16]
   if(ber_get_element(sp, cursor_normal, &msg) < 0)
      return RULE_NOMATCH;

   if(msg.type != 0x30)
      return RULE_NOMATCH;

   // calculate position of next msg
   next_msg = cursor_normal + msg.total_len;

   // integer overflow check
   if(next_msg < cursor_normal)
      return RULE_NOMATCH;

   // move cursor to messageID
   cursor_normal = msg.data.data_ptr;

   // messageID [2]
   BER_SKIP(0x02);

   // Request ::= APPLICATION [6]
   if(ber_get_element(sp, cursor_normal, &req) < 0)
      return RULE_NOMATCH;

   // move cursor to LDAPDN
   cursor_normal = req.data.data_ptr; 

   if(req.type != 0x66)
   {
      // check second msg for modifyRequest
      cursor_normal = next_msg;
      BER_DATA(0x30); // LDAPMessage 
      BER_SKIP(0x02); // messageID
      BER_DATA(0x66); // Request
   }

   BER_SKIP(0x04);   // LDAPDN [4]
   BER_DATA(0x30);   // modification items ::= SEQUENCE [16]

   // check up to 5 modification items 
   for(i=0; i<5; i++, cursor_normal = next_mod_item)
   {
      // modification item ::= SEQUENCE [16]
      if(ber_get_element(sp, cursor_normal, &mod) < 0)
         return RULE_NOMATCH;

      if(mod.type != 0x30)
         return RULE_NOMATCH;

      DEBUG_SO(fprintf(stderr,"mod item [0x%04X]\n", mod.data_len);)

      // calculate position of next mod item
      next_mod_item = cursor_normal + mod.total_len;

      // integer overflow check
      if(next_mod_item < cursor_normal)
         return RULE_NOMATCH;

      // if modification item data_len < 256 skip
      if(mod.data_len < 256)
         continue;

      // move cursor to operation 
      cursor_normal = mod.data.data_ptr;

      BER_SKIP(0x0A); // operation [10]
      BER_DATA(0x30); // modification ::= SEQUENCE [16]

      // attribute [4]
      bytes_available = ber_get_element(sp, cursor_normal, &attribute);

      if(bytes_available <= 0)
         return RULE_NOMATCH;

      if(attribute.type != 0x04)
         return RULE_NOMATCH;

      DEBUG_SO(fprintf(stderr," attribute [0x%04X]\n", attribute.data_len);)

      // if the attribute len is < 256
      // it cannot possibly contain the vuln, skip
      if(attribute.data_len < 256)
         continue;

      // calculate when we will stop checking bytes
      // we are guaranteed by the above that
      // attribute.data_len >= 256, thus stop
      // will at least be 0
      stop = attribute.data_len - 256;

      // move cursor to the attribute data
      cursor_normal = attribute.data.data_ptr;

      // limit stop to how much data we have available
      if(stop > bytes_available)
         stop = bytes_available;

      // Vuln is for there to be more than 256 bytes after the semicolon,
      // so if we find a semicolon before there are less than 256 bytes
      // left in the buffer, it's an exploit condition
      for(j=0; j < stop; j++)
      {
         if(cursor_normal[j] == ';')
            return RULE_MATCH;
      }
   }

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule36153,
    NULL
};
*/
