/*
 * Copyright (C) 2021-2021 Sourcefire, Inc. All Rights Reserved
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

#include "so-util_dns.h"
#include "so-util.h"

/* declare detection functions */
int rule57953eval(void *p);

/* declare rule data structures */
/* flow:to_client; */
static FlowFlags rule57953flow0 = 
{
   FLOW_TO_CLIENT
};

static RuleOption rule57953option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule57953flow0
   }
};

// content:"|00 01|", offset 4, depth 2; 
static ContentInfo rule57953content1 = 
{
   (uint8_t *) "|00 01|", /* pattern */
   2, /* depth */
   4, /* offset */
   CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule57953option1 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule57953content1
   }
};

// content:"|00 2E 00 01|", offset 0, depth 0, fast_pattern, relative; 
static ContentInfo rule57953content2 = 
{
   (uint8_t *) "|00 2E 00 01|", /* pattern */
   0, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN|CONTENT_BUF_NORMALIZED|CONTENT_RELATIVE, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule57953option2 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule57953content2
   }
};

// content:"|00 27|", offset 6, depth 2, relative; 
static ContentInfo rule57953content3 = 
{
   (uint8_t *) "|00 27|", /* pattern */
   2, /* depth */
   6, /* offset */
   CONTENT_BUF_NORMALIZED|CONTENT_RELATIVE, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule57953option3 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule57953content3
   }
};

/* references for sid 57953 */
/* reference: cve "2016-1286"; */
static RuleReference rule57953ref1 = 
{
   "cve", /* type */
   "2016-1286" /* value */
};

/* reference: url "kb.isc.org/docs/aa-01353"; */
static RuleReference rule57953ref2 = 
{
   "url", /* type */
   "kb.isc.org/docs/aa-01353" /* value */
};

static RuleReference *rule57953refs[] =
{
   &rule57953ref1,
   &rule57953ref2,
   NULL
};

/* metadata for sid 57953 */
/* metadata:service dns, policy max-detect-ips drop; */
static RuleMetaData rule57953service0 = 
{
   "service dns"
};

static RuleMetaData rule57953policy1 = 
{
   "policy max-detect-ips drop"
};

static RuleMetaData *rule57953metadata[] =
{
   &rule57953service0,
   &rule57953policy1,
   NULL
};

RuleOption *rule57953options[] =
{
   &rule57953option0,
   &rule57953option1,
   &rule57953option2,
   &rule57953option3,
   NULL
};

Rule rule57953 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_UDP, /* proto */
      "any", /* SRCIP     */
      "53", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "any", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      57953, /* sigid */
      1, /* revision */
      "attempted-dos", /* classification */
      0,  /* hardcoded priority */
      "PROTOCOL-DNS ISC BIND RRSIG response processing denial of service attempt",     /* message */
      rule57953refs, /* ptr to references */
      rule57953metadata /* ptr to metadata */
   },
   rule57953options, /* ptr to rule options */
   &rule57953eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

#define DNS_TYPE_RRSIG 0x002E
#define DNS_TYPE_DNAME 0x0027

/* detection functions */
int rule57953eval(void *p) {
    const uint8_t *cursor_normal = 0, *end_of_buffer;
    SFSnortPacket *sp = (SFSnortPacket *) p;
 
    if(sp == NULL)
        return RULE_NOMATCH;
 
    if(sp->payload == NULL)
        return RULE_NOMATCH;
    
    // flow:to_client;
    if(checkFlow(p, rule57953options[0]->option_u.flowFlags) <= 0)
        return RULE_NOMATCH;
    
    // content:"|00 01|", offset 4, depth 2;
    if(contentMatch(p, rule57953options[1]->option_u.content, &cursor_normal) <= 0)
        return RULE_NOMATCH;
    
    // content:"|00 2E 00 01|", offset 0, depth 0, fast_pattern, relative;
    if(contentMatch(p, rule57953options[2]->option_u.content, &cursor_normal) <= 0)
        return RULE_NOMATCH;
    
    // content:"|00 27|", offset 6, depth 2, relative;
    if(contentMatch(p, rule57953options[3]->option_u.content, &cursor_normal) <= 0)
        return RULE_NOMATCH;
 
    if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_buffer) <= 0)
        return RULE_NOMATCH;

    // skip txid (2 bytes)
    cursor_normal += 2;

    // check if we can:
    //  read flags (2 bytes)
    //  skip question number (2 bytes)
    //  read answer number (2 bytes)
    //  skip authority RR number (2 bytes)
    //  skip additional RR number (2 bytes)
    if(cursor_normal + 10 > end_of_buffer)
        return RULE_NOMATCH;

    uint16_t flags = read_big_16_inc(cursor_normal);

    // flags
    //
    // mask:
    // 0b1111101000001111 = 0xFA0F
    //   ^^   ^^^    ^
    //   ||   |||    |
    //   ||   |||    `- reply code (0000 = no error)
    //   ||   ||`- recursion and others
    //   ||   |`- truncated (0 = not truncated)
    //   ||   `- authoritative
    //   |`- opcode (0000 = standard query)
    //   `- response (1 = response)
    //
    if((flags & 0xFA0F) != 0x8000)
        return RULE_NOMATCH;

    // skip question number (we limit it to 1)
    cursor_normal += 2;

    // get the number of answers
    uint16_t num_answers = read_big_16_inc(cursor_normal);

    // if num_answers > 5, bail
    if(num_answers > 5)
        return RULE_NOMATCH;

    // skip:
    //  authority RR number (2 bytes)
    //  additional RR number (2 bytes)
    cursor_normal += 4;

    // skip query Name (we limit to 1)
    if(dns_skip_name(&cursor_normal, end_of_buffer) != DNS_SUCCESS)
        return RULE_NOMATCH;

    // skip:
    //  query type (2 bytes)
    //  query class (2 bytes)
    cursor_normal += 4;

    int dname_sig = 0;
    int dname_rec = 0;

    for(unsigned i = 0; i < num_answers; ++i)
    {
        // skip answer
        if(dns_skip_name(&cursor_normal, end_of_buffer) != DNS_SUCCESS)
            return RULE_NOMATCH;

        // check if we can:
        //  read type (2 bytes)
        //  skip class (2 bytes)
        //  skip TTL (4 bytes)
        //  read length (2 bytes)
        if(cursor_normal + 10 > end_of_buffer)
            return RULE_NOMATCH;

        uint16_t type = read_big_16_inc(cursor_normal);

        // skip:
        //  class (2 bytes)
        //  TTL (4 bytes)
        cursor_normal += 6;

        uint16_t length = read_big_16_inc(cursor_normal);

        switch(type)
        {
        case DNS_TYPE_RRSIG:
        {
            // check if we can read:
            //  type covered (2 bytes)
            if(cursor_normal + 2 > end_of_buffer)
                return RULE_NOMATCH;

            uint16_t type_covered = read_big_16(cursor_normal);

            if(type_covered == DNS_TYPE_DNAME)
                dname_sig = 1;

            break;
        }
        case DNS_TYPE_DNAME:
            dname_rec = 1;
            break;
        default:
            break;
        }

        // check if we can jump length
        if(length > end_of_buffer - cursor_normal)
            return RULE_NOMATCH;

        cursor_normal += length;
    }

    // if DNAME is signed but not present, alert.
    if(dname_sig && !dname_rec)
        return RULE_MATCH;

    return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule57953,
    NULL
};
*/
