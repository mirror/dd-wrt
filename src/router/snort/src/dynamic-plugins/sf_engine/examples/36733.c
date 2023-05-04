/*
 * Apache auth_ldap_log_reason format string vulnerabilty
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2007-2013 Sourcefire, Inc. All Rights Reserved
 *
 * Writen by Patrick Mullen, Sourcefire VRT <pmullen@sourcefire.com>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pcre.h"
#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"
#include "web-misc_base64_decode.h"

#include "snort_debug.h"

/* declare detection functions */
static int ruleAPACHEAUTHLDAPeval(void *p);

/* declare rule data structures */
/* precompile the stuff that needs pre-compiled */
/* flow:established,to_client; */
static FlowFlags ruleAPACHEAUTHLDAPflow0 =
{
    FLOW_ESTABLISHED | FLOW_TO_SERVER
};

static RuleOption ruleAPACHEAUTHLDAPoption0 =
{
    OPTION_TYPE_FLOWFLAGS,
    {
        &ruleAPACHEAUTHLDAPflow0
    }
};

// content:"Authorization:";
static ContentInfo ruleAPACHEAUTHLDAPcontent0 =
{
    (u_int8_t *)"Authorization:", /* pattern (now in snort content format) */
    0, /* depth */
    0, /* offset */
    CONTENT_BUF_NORMALIZED | CONTENT_NOCASE, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0, /* increment length*/
    0,                      /* holder for fp offset */
    0,                      /* holder for fp length */
    0,                      /* holder for fp only */
    NULL, // offset_refId
    NULL, // depth_refId
    NULL, // offset_location
    NULL  // depth_location
};

static RuleOption ruleAPACHEAUTHLDAPoption1 =
{
    OPTION_TYPE_CONTENT,
    {
        &ruleAPACHEAUTHLDAPcontent0
    }
};

// pcre:"/^Authorization:\s*Basic/mi";
static PCREInfo ruleAPACHEAUTHLDAPpcre0 =
{
    "^Authorization:\\s*Basic\\s+", /* pattern (now in snort content format) */
    0, /* compiled expression */
    0, /* compiled extra */
    PCRE_CASELESS | PCRE_MULTILINE, /* compile flags */
    CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
    0 /* offset */
};

static RuleOption ruleAPACHEAUTHLDAPoption2 =
{
    OPTION_TYPE_PCRE,
    {
        &ruleAPACHEAUTHLDAPpcre0
    }
};

// pcre:"/%[0-9]*\.?[0-9]*[:formatspecifiers:]/";
static PCREInfo ruleAPACHEAUTHLDAPpcre1 =
{
//    "%[-# +'I]*[0-9]*\\.?[0-9]*[qjzthdiouxefgcrslnp]", /* pattern (now in snort content format) */ // ZDNOTE
    "%.+%.", 	/* regex.  The above is technically more correct, but this is faster and good enough */
    0, /* compiled expression */
    0, /* compiled extra */
    0, /* compile flags */
    CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
    0 /* offset */
};

static RuleOption ruleAPACHEAUTHLDAPoption3 =
{
    OPTION_TYPE_PCRE,
    {
        &ruleAPACHEAUTHLDAPpcre1
    }
};


/* references for.ruleid APACHEAUTHLDAP */
static RuleReference ruleAPACHEAUTHLDAPref0 =
{
    "cve", /* type */
    "2006-0150" /* value */
};

static RuleReference ruleAPACHEAUTHLDAPref1 =
{
    "bugtraq", /* type */
    "16177" /* value */
};

static RuleReference *ruleAPACHEAUTHLDAPrefs[] =
{
    &ruleAPACHEAUTHLDAPref0,
    &ruleAPACHEAUTHLDAPref1,
    NULL
};

RuleOption *ruleAPACHEAUTHLDAPoptions[] =
{
    &ruleAPACHEAUTHLDAPoption0,
    &ruleAPACHEAUTHLDAPoption1,
    &ruleAPACHEAUTHLDAPoption2,
    &ruleAPACHEAUTHLDAPoption3,
    NULL
};


Rule ruleAPACHEAUTHLDAP = {
   /* rule header, akin to => tcp any any -> any any */
   {
       IPPROTO_TCP, /* proto */
       EXTERNAL_NET, /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       HTTP_SERVERS, /* DSTIP     */
       HTTP_PORTS, /* DSTPORT   */
   },
   /* metadata */
   {
       3,  /* genid (HARDCODED!!!) */
       36733, /* sid */
       0, /* revision */
       "attempted-user", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "WEB-MISC Apache auth_ldap_log_reason format string attempt",     /* message */
       ruleAPACHEAUTHLDAPrefs, /* ptr to references */
       NULL /* metadata */
   },
   ruleAPACHEAUTHLDAPoptions, /* ptr to rule options */
   &ruleAPACHEAUTHLDAPeval, /* use the built in detection function */
   0, /* am I initialized yet? */
   0, /* number of options */
   0,  /* don't alert */
   NULL /* ptr to internal data... setup during rule registration */
};

/* detection functions */
static int ruleAPACHEAUTHLDAPeval(void *p) {
   const u_int8_t *cursor = 0;
   const u_int8_t *end_of_payload = 0;

   SFSnortPacket *sp = (SFSnortPacket *) p;

   // Base64 stuff
   u_int8_t base64buf[64], decodedbuf[64];
   u_int32_t inputchars, base64bytes, decodedbytes;

   // manual pcre stuff
   int result;
   int ovector[3];  // Needs to be a multiple of 3

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   // Arbitrary number trying to denote the smallest packet size possible
   // GET / HTTP/1.0\nAuthorization:Basic JXglbg==\n\n is 45 bytes
   if(sp->payload_size < 45)
      return RULE_NOMATCH;

   end_of_payload = sp->payload + sp->payload_size;  // ZDNOTE fix warning

   //DEBUG_WRAP(printf("ruleAPACHEAUTHLDAPeval start\n"));

   // flow:established,to_server;
   if(checkFlow(p, ruleAPACHEAUTHLDAPoptions[0]->option_u.flowFlags) <= 0 )
      return RULE_NOMATCH;

   //DEBUG_WRAP(printf("passed flow check\n"));

   // content:"Authorization:";
   if(contentMatch(p, ruleAPACHEAUTHLDAPoptions[1]->option_u.content, &cursor) <= 0)
      return RULE_NOMATCH;

   //DEBUG_WRAP(printf("found content:\"Authorization:\" %p\n", cursor));

   // pcre:"/^Authorization:\s*Basic\s+/mi"
   if(pcreMatch(p, ruleAPACHEAUTHLDAPoptions[2]->option_u.pcre, &cursor) <= 0)
      return RULE_NOMATCH;

   //DEBUG_WRAP(printf("found pcre:\"/^Authorization:\\s*Basic\\s+/mi\" %p\n", cursor));

   // At this point, cursor should point to the start of the auth data
   inputchars = (end_of_payload > cursor + 64) ? 64 : end_of_payload - cursor;
   if(web_misc_unfold_header(cursor, inputchars, base64buf, sizeof(base64buf), &base64bytes) != 0)
      return RULE_NOMATCH;

   //DEBUG_WRAP(printf("Successfully unfolded header (%s)(%d)\n", base64buf, base64bytes));

   if(web_misc_base64decode(base64buf, base64bytes, decodedbuf, sizeof(decodedbuf), &decodedbytes) != 0)
      return RULE_NOMATCH;

   //DEBUG_WRAP(printf("Successfully base64 decoded (%s)(%d)\n", decodedbuf, decodedbytes));

   // Now run our regex on the base64 decoding to find an attack
   result = pcreExecWrapper(ruleAPACHEAUTHLDAPoptions[3]->option_u.pcre,
                      (char *)decodedbuf,        // subject string
                      decodedbytes,      // subject length
                      0,                 // start offset
                      0,                 // options (handled at compile time)
                      ovector,           // ovector for storing result substrings
                      sizeof(ovector)/sizeof(int));  // size of ovector

   //DEBUG_WRAP(printf("result = %d\n", result));

   if(result >= 0)
      return RULE_MATCH;

   return RULE_NOMATCH;
}

