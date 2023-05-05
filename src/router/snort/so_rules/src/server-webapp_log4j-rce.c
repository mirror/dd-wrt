/*
 * Copyright (C) 2021-2022 Cisco and/or its affiliates. All rights reserved.
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

#include <ctype.h>
#include <string.h>

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"

// pcre ovector count (must be a multiple of 3)
#define OVECCOUNT 30

// buffer for unescaped data
#define UNESCAPED_BUF_SIZE 4096
static uint8_t unescaped_buffer[UNESCAPED_BUF_SIZE];

/* declare detection functions */
int DetectLog4jCodeExec(void *p);

/* declare rule data structures */
// flow:to_server,established;
static FlowFlags rule_Log4jCodeExec_flow0 = 
{
   FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule_Log4jCodeExec_option0 =
{
   OPTION_TYPE_FLOWFLAGS,
   {
      &rule_Log4jCodeExec_flow0
   }
};

// flowbits:isnotset,log4j_post_inspected;
static FlowBitsInfo rule_Log4jCodeExec_flowbits1 =
{
   "log4j_post_inspected",
   FLOWBIT_ISNOTSET,
   0,
};

static RuleOption rule_Log4jCodeExec_option1 =
{
   OPTION_TYPE_FLOWBIT,
   {
      &rule_Log4jCodeExec_flowbits1
   }
};

// content:"/"; depth:1; http_uri;
static ContentInfo rule_Log4jCodeExec_content2 = 
{
   (uint8_t *) "/", /* pattern */
   1, /* depth */
   0, /* offset */
   CONTENT_BUF_URI, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule_Log4jCodeExec_option2 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule_Log4jCodeExec_content2
   }
};

// content:"${"; fast_pattern:only; http_client_body;
static ContentInfo rule58802content3 = 
{
   (uint8_t *) "${", /* pattern */
   0, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN_ONLY|CONTENT_BUF_POST, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule58802option3 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule58802content3
   }
};

// content:"$%7b"; fast_pattern:only; http_client_body;
static ContentInfo rule58803content3 = 
{
   (uint8_t *) "$%7b", /* pattern */
   0, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN_ONLY|CONTENT_BUF_POST, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule58803option3 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule58803content3
   }
};

// content:"$%257b"; fast_pattern:only; http_client_body;
static ContentInfo rule58804content3 = 
{
   (uint8_t *) "$%257b", /* pattern */
   0, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN_ONLY|CONTENT_BUF_POST, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule58804option3 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule58804content3
   }
};

// content:"%24{"; fast_pattern:only; http_client_body;
static ContentInfo rule58805content3 = 
{
   (uint8_t *) "%24{", /* pattern */
   0, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN_ONLY|CONTENT_BUF_POST, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule58805option3 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule58805content3
   }
};

// content:"%24%7b"; fast_pattern:only; http_client_body;
static ContentInfo rule58806content3 = 
{
   (uint8_t *) "%24%7b", /* pattern */
   0, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN_ONLY|CONTENT_BUF_POST, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule58806option3 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule58806content3
   }
};

// content:"%24%257b"; fast_pattern:only; http_client_body;
static ContentInfo rule58807content3 = 
{
   (uint8_t *) "%24%257b", /* pattern */
   0, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN_ONLY|CONTENT_BUF_POST, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule58807option3 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule58807content3
   }
};

// content:"%2524{"; fast_pattern:only; http_client_body;
static ContentInfo rule58808content3 = 
{
   (uint8_t *) "%2524{", /* pattern */
   0, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN_ONLY|CONTENT_BUF_POST, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule58808option3 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule58808content3
   }
};

// content:"%2524%7b"; fast_pattern:only; http_client_body;
static ContentInfo rule58809content3 = 
{
   (uint8_t *) "%2524%7b", /* pattern */
   0, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN_ONLY|CONTENT_BUF_POST, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule58809option3 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule58809content3
   }
};

// content:"%2524%257b"; fast_pattern:only; http_client_body;
static ContentInfo rule58810content3 = 
{
   (uint8_t *) "%2524%257b", /* pattern */
   0, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN_ONLY|CONTENT_BUF_POST, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule58810option3 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule58810content3
   }
};

// pcre:"/\x24\x7b(jndi|[^\x7d\x80-\xff]*?\x24\x7b[^\x7d\x80-\xff]*?\x3a[^\x7d]*?\x7d)/i";
static PCREInfo rule_Log4jCodeExec_pcre4 =
{
   "\\x24\\x7b(jndi|[^\\x7d\\x80-\\xff]*?\\x24\\x7b[^\\x7d\\x80-\\xff]*?\\x3a[^\\x7d]*?\\x7d)", /* pattern */
   NULL,                               /* holder for compiled pattern */
   NULL,                               /* holder for compiled pattern flags */
   PCRE_CASELESS,     /* compile flags */
   CONTENT_BUF_NORMALIZED     /* content flags */
};

static RuleOption rule_Log4jCodeExec_option4 =
{
   OPTION_TYPE_PCRE,
   {
      &rule_Log4jCodeExec_pcre4
   }
};

// flowbits:set,log4j_post_inspected;
static FlowBitsInfo rule_Log4jCodeExec_flowbits5 =
{
   "log4j_post_inspected",
   FLOWBIT_SET,
   0,
};

static RuleOption rule_Log4jCodeExec_option5 =
{
   OPTION_TYPE_FLOWBIT,
   {
      &rule_Log4jCodeExec_flowbits5
   }
};

/* references */
/* reference: url "blog.talosintelligence.com/2021/12/apache-log4j-rce-vulnerability.html"; */
static RuleReference rule_Log4jCodeExec_ref1 = 
{
   "url", /* type */
   "blog.talosintelligence.com/2021/12/apache-log4j-rce-vulnerability.html" /* value */
};

/* reference: cve "2021-44228"; */
static RuleReference rule_Log4jCodeExec_ref2 = 
{
   "cve", /* type */
   "2021-44228" /* value */
};

/* reference: cve "2021-44832"; */
static RuleReference rule_Log4jCodeExec_ref3 = 
{
   "cve", /* type */
   "2021-44832" /* value */
};

/* reference: cve "2021-45046"; */
static RuleReference rule_Log4jCodeExec_ref4 = 
{
   "cve", /* type */
   "2021-45046" /* value */
};

/* reference: cve "2021-45105"; */
static RuleReference rule_Log4jCodeExec_ref5 = 
{
   "cve", /* type */
   "2021-45105" /* value */
};

static RuleReference *rule_Log4jCodeExec_refs[] =
{
   &rule_Log4jCodeExec_ref1,
   &rule_Log4jCodeExec_ref2,
   &rule_Log4jCodeExec_ref3,
   &rule_Log4jCodeExec_ref4,
   &rule_Log4jCodeExec_ref5,
   NULL
};

/* metadata */
/* metadata:service http, policy balanced-ips drop, policy connectivity-ips drop, policy max-detect-ips drop, policy security-ips drop, ruleset community; */
static RuleMetaData rule_Log4jCodeExec_service0 = 
{
   "service http"
};

static RuleMetaData rule_Log4jCodeExec_policy1 = 
{
   "policy balanced-ips drop"
};

static RuleMetaData rule_Log4jCodeExec_policy2 = 
{
   "policy connectivity-ips drop"
};

static RuleMetaData rule_Log4jCodeExec_policy3 = 
{
   "policy max-detect-ips drop"
};

static RuleMetaData rule_Log4jCodeExec_policy4 = 
{
   "policy security-ips drop"
};

static RuleMetaData rule_Log4jCodeExec_ruleset5 = 
{
   "ruleset community"
};

static RuleMetaData *rule_Log4jCodeExec_metadata[] =
{
   &rule_Log4jCodeExec_service0,
   &rule_Log4jCodeExec_policy1,
   &rule_Log4jCodeExec_policy2,
   &rule_Log4jCodeExec_policy3,
   &rule_Log4jCodeExec_policy4,
   &rule_Log4jCodeExec_ruleset5,
   NULL
};

RuleOption *rule58802options[] =
{
   &rule_Log4jCodeExec_option0,
   &rule_Log4jCodeExec_option1,
   &rule_Log4jCodeExec_option2,
   &rule58802option3,
   &rule_Log4jCodeExec_option4,
   &rule_Log4jCodeExec_option5,
   NULL
};

RuleOption *rule58803options[] =
{
   &rule_Log4jCodeExec_option0,
   &rule_Log4jCodeExec_option1,
   &rule_Log4jCodeExec_option2,
   &rule58803option3,
   &rule_Log4jCodeExec_option4,
   &rule_Log4jCodeExec_option5,
   NULL
};

RuleOption *rule58804options[] =
{
   &rule_Log4jCodeExec_option0,
   &rule_Log4jCodeExec_option1,
   &rule_Log4jCodeExec_option2,
   &rule58804option3,
   &rule_Log4jCodeExec_option4,
   &rule_Log4jCodeExec_option5,
   NULL
};

RuleOption *rule58805options[] =
{
   &rule_Log4jCodeExec_option0,
   &rule_Log4jCodeExec_option1,
   &rule_Log4jCodeExec_option2,
   &rule58805option3,
   &rule_Log4jCodeExec_option4,
   &rule_Log4jCodeExec_option5,
   NULL
};

RuleOption *rule58806options[] =
{
   &rule_Log4jCodeExec_option0,
   &rule_Log4jCodeExec_option1,
   &rule_Log4jCodeExec_option2,
   &rule58806option3,
   &rule_Log4jCodeExec_option4,
   &rule_Log4jCodeExec_option5,
   NULL
};

RuleOption *rule58807options[] =
{
   &rule_Log4jCodeExec_option0,
   &rule_Log4jCodeExec_option1,
   &rule_Log4jCodeExec_option2,
   &rule58807option3,
   &rule_Log4jCodeExec_option4,
   &rule_Log4jCodeExec_option5,
   NULL
};

RuleOption *rule58808options[] =
{
   &rule_Log4jCodeExec_option0,
   &rule_Log4jCodeExec_option1,
   &rule_Log4jCodeExec_option2,
   &rule58808option3,
   &rule_Log4jCodeExec_option4,
   &rule_Log4jCodeExec_option5,
   NULL
};

RuleOption *rule58809options[] =
{
   &rule_Log4jCodeExec_option0,
   &rule_Log4jCodeExec_option1,
   &rule_Log4jCodeExec_option2,
   &rule58809option3,
   &rule_Log4jCodeExec_option4,
   &rule_Log4jCodeExec_option5,
   NULL
};

RuleOption *rule58810options[] =
{
   &rule_Log4jCodeExec_option0,
   &rule_Log4jCodeExec_option1,
   &rule_Log4jCodeExec_option2,
   &rule58810option3,
   &rule_Log4jCodeExec_option4,
   &rule_Log4jCodeExec_option5,
   NULL
};

Rule rule58802 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_TCP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "$HTTP_PORTS", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      58802, /* sigid */
      4, /* revision */
      "attempted-user", /* classification */
      0,  /* hardcoded priority */
      "SERVER-WEBAPP Apache Log4j logging remote code execution attempt",     /* message */
      rule_Log4jCodeExec_refs, /* ptr to references */
      rule_Log4jCodeExec_metadata /* ptr to metadata */
   },
   rule58802options, /* ptr to rule options */
   &DetectLog4jCodeExec, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

Rule rule58803 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_TCP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "$HTTP_PORTS", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      58803, /* sigid */
      4, /* revision */
      "attempted-user", /* classification */
      0,  /* hardcoded priority */
      "SERVER-WEBAPP Apache Log4j logging remote code execution attempt",     /* message */
      rule_Log4jCodeExec_refs, /* ptr to references */
      rule_Log4jCodeExec_metadata /* ptr to metadata */
   },
   rule58803options, /* ptr to rule options */
   &DetectLog4jCodeExec, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

Rule rule58804 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_TCP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "$HTTP_PORTS", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      58804, /* sigid */
      4, /* revision */
      "attempted-user", /* classification */
      0,  /* hardcoded priority */
      "SERVER-WEBAPP Apache Log4j logging remote code execution attempt",     /* message */
      rule_Log4jCodeExec_refs, /* ptr to references */
      rule_Log4jCodeExec_metadata /* ptr to metadata */
   },
   rule58804options, /* ptr to rule options */
   &DetectLog4jCodeExec, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

Rule rule58805 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_TCP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "$HTTP_PORTS", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      58805, /* sigid */
      4, /* revision */
      "attempted-user", /* classification */
      0,  /* hardcoded priority */
      "SERVER-WEBAPP Apache Log4j logging remote code execution attempt",     /* message */
      rule_Log4jCodeExec_refs, /* ptr to references */
      rule_Log4jCodeExec_metadata /* ptr to metadata */
   },
   rule58805options, /* ptr to rule options */
   &DetectLog4jCodeExec, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

Rule rule58806 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_TCP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "$HTTP_PORTS", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      58806, /* sigid */
      4, /* revision */
      "attempted-user", /* classification */
      0,  /* hardcoded priority */
      "SERVER-WEBAPP Apache Log4j logging remote code execution attempt",     /* message */
      rule_Log4jCodeExec_refs, /* ptr to references */
      rule_Log4jCodeExec_metadata /* ptr to metadata */
   },
   rule58806options, /* ptr to rule options */
   &DetectLog4jCodeExec, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

Rule rule58807 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_TCP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "$HTTP_PORTS", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      58807, /* sigid */
      4, /* revision */
      "attempted-user", /* classification */
      0,  /* hardcoded priority */
      "SERVER-WEBAPP Apache Log4j logging remote code execution attempt",     /* message */
      rule_Log4jCodeExec_refs, /* ptr to references */
      rule_Log4jCodeExec_metadata /* ptr to metadata */
   },
   rule58807options, /* ptr to rule options */
   &DetectLog4jCodeExec, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

Rule rule58808 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_TCP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "$HTTP_PORTS", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      58808, /* sigid */
      4, /* revision */
      "attempted-user", /* classification */
      0,  /* hardcoded priority */
      "SERVER-WEBAPP Apache Log4j logging remote code execution attempt",     /* message */
      rule_Log4jCodeExec_refs, /* ptr to references */
      rule_Log4jCodeExec_metadata /* ptr to metadata */
   },
   rule58808options, /* ptr to rule options */
   &DetectLog4jCodeExec, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

Rule rule58809 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_TCP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "$HTTP_PORTS", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      58809, /* sigid */
      4, /* revision */
      "attempted-user", /* classification */
      0,  /* hardcoded priority */
      "SERVER-WEBAPP Apache Log4j logging remote code execution attempt",     /* message */
      rule_Log4jCodeExec_refs, /* ptr to references */
      rule_Log4jCodeExec_metadata /* ptr to metadata */
   },
   rule58809options, /* ptr to rule options */
   &DetectLog4jCodeExec, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

Rule rule58810 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_TCP, /* proto */
      "$EXTERNAL_NET", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "$HTTP_PORTS", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      58810, /* sigid */
      4, /* revision */
      "attempted-user", /* classification */
      0,  /* hardcoded priority */
      "SERVER-WEBAPP Apache Log4j logging remote code execution attempt",     /* message */
      rule_Log4jCodeExec_refs, /* ptr to references */
      rule_Log4jCodeExec_metadata /* ptr to metadata */
   },
   rule58810options, /* ptr to rule options */
   &DetectLog4jCodeExec, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};

/* detection functions */
static uint8_t xtob(const uint8_t c)
{
    if(isdigit(c))
        return c - '0';

    if(isupper(c))
        return c - 'A' + 10;

    return c - 'a' + 10;
}

static int findchr(const uint8_t* buf, const size_t len,
    char c, size_t offset, size_t* pos)
{
    if(offset >= len)
        return 0;

    size_t sz = len - offset;

    buf += offset;

    const uint8_t* ptr = (const uint8_t*)memchr(buf, c, sz);

    if(!ptr)
        return 0;

    *pos = (size_t)(ptr - buf);

    return 1;
}

static void replacechr(uint8_t* buf, const size_t len,
    char c, char r)
{
    size_t offset = 0, pos = 0;

    while(findchr(buf, len, c, offset, &pos))
    {
        offset += pos;
        buf[offset] = (uint8_t)r;
        offset += 1;
    }
}

static size_t unescape(const uint8_t* buf, const size_t buf_len,
    uint8_t* out, const size_t out_len)
{
    if(buf_len == 0 || buf_len > out_len)
        return 0;

    size_t buf_offset = 0, out_offset = 0, pos = 0;

    while(findchr(buf, buf_len, '%', buf_offset, &pos))
    {
        if(pos > 0)
        {
            memcpy(out + out_offset, buf + buf_offset, pos);
            out_offset += pos;
            buf_offset += pos;
        }

        size_t remain = buf_len - buf_offset;

        if(remain < 3)
            break;

        // %XX
        uint8_t h = buf[buf_offset + 1];
        uint8_t l = buf[buf_offset + 2];

        if(!isxdigit(h) || !isxdigit(l))
        {
            // skip
            out[out_offset++] = '%';
            buf_offset++;
            continue;
        }

        uint8_t b = (uint8_t)((xtob(h) << 4) + xtob(l));

        // check for double encoding
        if(b == '%' && remain >= 5)
        {
            // %25XX
            h = buf[buf_offset + 3];
            l = buf[buf_offset + 4];

            if(isxdigit(h) && isxdigit(l))
            {
                b = (uint8_t)((xtob(h) << 4) + xtob(l));

                out[out_offset++] = b;
                buf_offset += 5;
                continue;
            }
        }

        out[out_offset++] = b;
        buf_offset += 3;
    }

    if(buf_offset < buf_len)
    {
        size_t remain = buf_len - buf_offset;
        memcpy(out + out_offset, buf + buf_offset, remain);
        out_offset += remain;
        buf_offset += remain;
    }

    replacechr(out, out_offset, '+', ' ');

    return out_offset;
}

#ifdef DEBUG
static void hexdump(const uint8_t* buf, const size_t len)
{
    uint8_t asciigraph[17] = {0};
    size_t i = 0;

    while(i < len)
    {
        if(i % 16 == 0)
            fprintf(stderr, "0x%04zx  ", i);

        fprintf(stderr, "%02x ", buf[i]);

        asciigraph[i % 16] = isprint(buf[i]) ? buf[i] : '.';

        if(i % 16 == 15)
        {
            fprintf(stderr, " |%s|\n", asciigraph);
            memset(asciigraph, 0, 17);
        }

        i++;
    }

    if(i % 16 != 0)
    {
        fprintf(stderr, "%*s", (int)((16 - (i % 16)) * 3), " ");
        fprintf(stderr, " |%s|\n", asciigraph);
    }
}
#endif

int DetectLog4jCodeExec(void *p) {
   const uint8_t *cursor_normal = 0, *end_of_buffer;
   SFSnortPacket *sp = (SFSnortPacket *)p;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   // flow:to_server,established;
   if(checkFlow(p, rule58802options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;

   // only unescape http_client_body once
   // flowbits:isnotset,log4j_post_inspected;
   if(processFlowbits(p, rule58802options[1]->option_u.flowBit) <= 0)
      return RULE_NOALERT;

   // http_inspect sanity check
   // content:"/"; depth:1; http_uri;
   if(contentMatch(p, rule58802options[2]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   // "${" permutations fast_pattern:only;
   //
   // raw,*
   // -----
   // "${"         raw,raw
   // "$%7b"       raw,encoded
   // "$%257b"     raw,double encoded
   // 
   // encoded,*
   // ---------
   // "%24{"       encoded,raw
   // "%24%7b"     encoded,encoded
   // "%24%257b"   encoded,double encoded
   // 
   // double encoded,*
   // ----------------
   // "%2524{"     double encoded,raw
   // "%2524%7b"   double encoded,encoded
   // "%2524%257b" double encoded,double encoded

   if(getBuffer(sp, CONTENT_BUF_POST, &cursor_normal, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   const uint8_t* buf = cursor_normal;
   size_t buf_len = end_of_buffer - cursor_normal;

   // cap input to unescape
   if(buf_len > UNESCAPED_BUF_SIZE)
      buf_len = UNESCAPED_BUF_SIZE;

   DEBUG_SO(fprintf(stderr,"buffer:\n");)
   DEBUG_SO(hexdump(buf, buf_len);)

   size_t unescaped_len =
      unescape(buf, buf_len, unescaped_buffer, UNESCAPED_BUF_SIZE);

   if(unescaped_len == 0)
      return RULE_NOMATCH;

   DEBUG_SO(fprintf(stderr,"\nunescaped buffer:\n");)
   DEBUG_SO(hexdump(unescaped_buffer, unescaped_len);)

   int ovector[OVECCOUNT] = {0};

   // pcre:"/\x24\x7b(jndi|[^\x7d\x80-\xff]*?\x24\x7b[^\x7d\x80-\xff]*?\x3a[^\x7d]*?\x7d)/i";
   int match = pcreExecWrapper(
      rule58802options[4]->option_u.pcre, // PCREInfo
      (const char*)unescaped_buffer,      // subject buffer
      unescaped_len, // subject length
      0,             // offset
      0,             // options
      ovector,       // output vector
      OVECCOUNT      // output vector count
   );

   // flowbits:set,log4j_post_inspected;
   processFlowbits(p, rule58802options[5]->option_u.flowBit);

   if(!match)
      return RULE_NOMATCH;

   DEBUG_SO(fprintf(stderr,"\nmatch found at offset: %d\n",ovector[0]);)

   return RULE_MATCH;
}

/*
Rule *rules[] = {
    &rule58802,
    &rule58803,
    &rule58804,
    &rule58805,
    &rule58806,
    &rule58807,
    &rule58808,
    &rule58809,
    &rule58810,
    NULL
};
*/
