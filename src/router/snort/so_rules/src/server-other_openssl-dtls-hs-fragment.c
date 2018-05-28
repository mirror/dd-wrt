/*
 * Copyright (C) 2005-2014 Sourcefire, Inc. All Rights Reserved
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
#include <time.h>

#ifndef READ_BIG_24_INC
#define READ_BIG_24_INC(p) (*(p) << 16)   \
           | (*((uint8_t *)(p) + 1) << 8) \
           | (*((uint8_t *)(p) + 2));     \
           p += 3
#endif

// dtls client->server handshake types
#define HS_CLIENT_HELLO 1
#define HS_CLIENT_KEYX 16
#define HS_CHG_CIPHER_SPEC 20
#define DTLS_HS 22

// dtls handshake fragment datatype
typedef struct {
   struct timeval ts;        // fragment timestamp
   uint8_t hs_type;          // handshake type
   uint16_t msg_seq;         // message sequence
   uint32_t reassembled_len; // reassembled msg length
   uint32_t len;             // fragment length
} dtls_hs_fragment;

#define FRAGMENT_TABLE_LEN 5
#define TIME_WINDOW 2

/* declare detection functions */
int rule31361eval(void *p);

/* declare rule data structures */
/* flow:to_server; */
static FlowFlags rule31361flow0 = 
{
   FLOW_TO_SERVER
};

static RuleOption rule31361option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule31361flow0
   }
};

// content:"|16 FE FF|", depth 3, fast_pattern; 
static ContentInfo rule31361content1 = 
{
   (uint8_t *) "|16 FE FF|", /* pattern (now in snort content format) */
   3, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN|CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule31361option1 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule31361content1
   }
};

/* references for sid 31361 */
/* reference: bugtraq "67900"; */
static RuleReference rule31361ref1 = 
{
   "bugtraq", /* type */
   "67900" /* value */
};

/* reference: cve "2014-0195"; */
static RuleReference rule31361ref2 = 
{
   "cve", /* type */
   "2014-0195" /* value */
};

/* reference: url "osvdb.org/show/osvdb/107730"; */
static RuleReference rule31361ref3 = 
{
   "url", /* type */
   "osvdb.org/show/osvdb/107730" /* value */
};

/* reference: url "www.openssl.org/news/secadv_20140605.txt"; */
static RuleReference rule31361ref4 = 
{
   "url", /* type */
   "www.openssl.org/news/secadv_20140605.txt" /* value */
};

static RuleReference *rule31361refs[] =
{
   &rule31361ref1,
   &rule31361ref2,
   &rule31361ref3,
   &rule31361ref4,
   NULL
};

/* metadata for sid 31361 */
/* metadata:policy balanced-ips drop, policy security-ips drop; */
static RuleMetaData rule31361policy1 = 
{
   "policy balanced-ips drop"
};

static RuleMetaData rule31361policy2 = 
{
   "policy security-ips drop"
};

static RuleMetaData *rule31361metadata[] =
{
   &rule31361policy1,
   &rule31361policy2,
   NULL
};

RuleOption *rule31361options[] =
{
   &rule31361option0,
   &rule31361option1,
   NULL
};

Rule rule31361 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_UDP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "[4433,443]", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      31361, /* sigid */
      2, /* revision */
      "attempted-admin", /* classification */
      0,  /* hardcoded priority */
      "SERVER-OTHER OpenSSL DTLSv1.0 handshake fragment buffer overrun attempt",     /* message */
      rule31361refs, /* ptr to references */
      rule31361metadata /* ptr to metadata */
   },
   rule31361options, /* ptr to rule options */
   &rule31361eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

/* detection functions */
int check_msg_seq(SFSnortPacket *sp, dtls_hs_fragment *fragment,
                  dtls_hs_fragment **fragment_table_ptr) {
   dtls_hs_fragment *previous_fragment, *fragment_table = *fragment_table_ptr;

   // check if fragment sequence is within range
   if(fragment->msg_seq >= FRAGMENT_TABLE_LEN)
      return RULE_NOMATCH;

   if(!fragment_table)
   {
      // retrieve the fragment table for this stream
      fragment_table = (dtls_hs_fragment*)getRuleData(sp, rule31361.info.sigID);

      // allocate and initalize the fragment table if it does not exist
      if(!fragment_table)
      {
         fragment_table = (dtls_hs_fragment*)allocRuleData(sizeof(dtls_hs_fragment)*FRAGMENT_TABLE_LEN);

         if(fragment_table == NULL)
            return RULE_NOMATCH;

         if(storeRuleData(sp, fragment_table, rule31361.info.sigID, &freeRuleData) < 0)
         {
            freeRuleData(fragment_table);
            return RULE_NOMATCH;
         }
      }

      // store the fragment_table for subsequent checks
      *fragment_table_ptr = fragment_table;
   }

   // lookup previous fragment
   previous_fragment = &fragment_table[fragment->msg_seq];

   // check if we have seen a fragment of this sequence before
   if(previous_fragment->reassembled_len != 0)
   {
      // We have seen a fragment of this sequence in this
      // stream. Check for the vulnerabile condition, if
      // the fragment type differs or if it was encountered
      // outside the time window, replace the entry in the
      // table with this fragment.
      if((fragment->hs_type == previous_fragment->hs_type) &&
         (sp->pkt_header->ts.tv_sec <= (previous_fragment->ts.tv_sec + TIME_WINDOW)) &&
         (fragment->reassembled_len != previous_fragment->reassembled_len))
         return RULE_MATCH;
   }

   // Add fragment's information to the table for future checks
   previous_fragment->ts = sp->pkt_header->ts;
   previous_fragment->hs_type = fragment->hs_type;
   previous_fragment->msg_seq = fragment->msg_seq;
   previous_fragment->reassembled_len = fragment->reassembled_len;
   
   return RULE_NOMATCH;
}

int rule31361eval(void *p) {
   const uint8_t *cursor_normal = 0, *end_of_buffer,
                 *end_of_rec_pos, *check;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   dtls_hs_fragment fragment, *fragment_table = NULL;
   uint16_t dtls_record_len;
   uint8_t dtls_record_type;
   int i,j;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   // flow:to_server;
   if(checkFlow(p, rule31361options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;

   // content:"|16 FE FF|", depth 3, fast_pattern;
   if(contentMatch(p, rule31361options[1]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   if(getBuffer(p, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   // check up to 2 DTLS records
   for(i=0; i<2; i++)
   {
      // make sure we can read the DTLS record type and length
      if(cursor_normal + 13 > end_of_buffer)
         return RULE_NOMATCH;

      // read the dtls record type
      dtls_record_type = *cursor_normal;

      // skip version, epoch, sequence number
      cursor_normal += 11;

      // read the DTLS record length (2 byte big-endian)
      dtls_record_len = read_big_16_inc(cursor_normal);

      // calculate the end of record position
      end_of_rec_pos = cursor_normal + dtls_record_len;

      // overflow check
      if(end_of_rec_pos < cursor_normal)
         return RULE_NOMATCH;

      // if the DTLS record is not a handshake, skip it.
      if(dtls_record_type != DTLS_HS)
      {
         cursor_normal = end_of_rec_pos;
         continue;
      }

      // check up to 3 dtls handshake fragments per DTLS record
      for(j=0; j<3; j++)
      {
         // make sure we can read the dtls fragment header
         if(cursor_normal + 12 > end_of_buffer)
            return RULE_NOMATCH;

         // read handshake type
         fragment.hs_type = *cursor_normal++;

         // make sure we are inspecting a client hs_type 
         if(fragment.hs_type != HS_CLIENT_HELLO &&
            fragment.hs_type != HS_CLIENT_KEYX &&
            fragment.hs_type != HS_CHG_CIPHER_SPEC)
            return RULE_NOMATCH;

         // read reassembled_len (3 byte big-endian)
         fragment.reassembled_len = READ_BIG_24_INC(cursor_normal);

         // read message sequence (2 byte big-endian)
         fragment.msg_seq = read_big_16_inc(cursor_normal);

         // skip fragment offset (3 byte big-endian)
         cursor_normal += 3;

         // read fragment len (3 byte big-endian)
         fragment.len = READ_BIG_24_INC(cursor_normal);

         // while this is invalid, it is handled properly by OpenSSL
         if(fragment.len > fragment.reassembled_len)
            return RULE_NOMATCH;

         // check if message is fragmented
         if(fragment.len < fragment.reassembled_len)
            if(check_msg_seq(sp, &fragment, &fragment_table) == RULE_MATCH)
               return RULE_MATCH;

         // skip fragment length
         check = cursor_normal + fragment.len;

         // overflow check
         if(check < cursor_normal)
            return RULE_NOMATCH;

         cursor_normal = check;

         // check if we went past the end_of_rec_pos
         if(cursor_normal > end_of_rec_pos)
            return RULE_NOMATCH;

         // if we landed on the end of the dtls
         // record then break the inner loop
         if(cursor_normal == end_of_rec_pos)
            break;
      }
   }

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule31361,
    NULL
};
*/
