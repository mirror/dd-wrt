/*
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pcre.h"
#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"
#include "detection_lib_meta.h"

/*
alert tcp $EXTERNAL_NET any -> $SMTP_SERVERS 25
(msg:"SMTP spoofed MIME-Type auto-execution attempt";
flow:to_server,established;
content:"Content-Type|3A|"; nocase;
content:"audio/"; nocase;
pcre:"/Content-Type\x3A\s+audio\/(x-wav|mpeg|x-midi)/i";
content:"filename="; distance:0; nocase;
pcre:"/filename=[\x22\x27]?.{1,221}\.(vbs|exe|scr|pif|bat)/i";
reference:bugtraq,2524; reference:cve,2001-0154;
reference:url,www.microsoft.com/technet/security/bulletin/MS01-020.mspx;
classtype:attempted-admin; sid:3682; rev:2;)
*/

/* declare detection functions */
int rule3682eval(void *p);

/* declare rule data structures */
/* precompile the stuff that needs pre-compiled */
/* flow for sid 3665 */
/* flow:established, from_server; */
static FlowFlags rule3682flow1 =
{
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule3682option1 =
{
    OPTION_TYPE_FLOWFLAGS,
    { &rule3682flow1 }
};

/* content for sid 3682 */
// content:"Content-Type|3A|"; nocase;
static ContentInfo rule3682content2 =
{
    (u_int8_t *)("Content-Type|3A|"), /* pattern (now in snort content format) */
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

static RuleOption rule3682option2 =
{
    OPTION_TYPE_CONTENT,
    { &rule3682content2 }
};


/* content for sid 3682 */
// content:"audio/"; nocase;
static ContentInfo rule3682content3 =
{
    (u_int8_t *)("audio/"), /* pattern (now in snort content format) */
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

static RuleOption rule3682option3 =
{
    OPTION_TYPE_CONTENT,
    { &rule3682content3 }
};

/* pcre for sid 3682 */
//pcre:"/Content-Type\x3A\s+audio\/(x-wav|mpeg|x-midi)/i";
static PCREInfo rule3682pcre4 =
{
    "Content-Type\\x3A\\s+audio\\/(x-wav|mpeg|x-midi)",               /* expression */
    NULL,                       /* Holder for compiled expr */
    NULL,                       /* Holder for compiled expr extra flags */
    PCRE_CASELESS, /* Compile Flags */
    CONTENT_BUF_NORMALIZED,      /* Flags */
    0 /* offset */
};

static RuleOption rule3682option4 =
{
    OPTION_TYPE_PCRE,
    { &rule3682pcre4 }
};

/* content for sid 3682 */
// content:"filename="; distance:0; nocase;
static ContentInfo rule3682content5 =
{
    (u_int8_t *)("filename="), /* pattern (now in snort content format) */
    0, /* depth */
    0, /* offset */
    CONTENT_BUF_NORMALIZED | CONTENT_NOCASE | CONTENT_RELATIVE, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
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

static RuleOption rule3682option5 =
{
    OPTION_TYPE_CONTENT,
    { &rule3682content5 }
};

/* pcre for sid 3682 */
//pcre:"/filename=[\x22\x27]?.{1,221}\.(vbs|exe|scr|pif|bat)/i";
static PCREInfo rule3682pcre6 =
{
    "filename=[\\x22\\x27]?.{1,221}\\.(vbs|exe|scr|pif|bat)",               /* expression */
    NULL,                       /* Holder for compiled expr */
    NULL,                       /* Holder for compiled expr extra flags */
    PCRE_CASELESS, /* Compile Flags */
    CONTENT_BUF_NORMALIZED,      /* Flags */
    0 /* offset */
};

static RuleOption rule3682option6 =
{
    OPTION_TYPE_PCRE,
    { &rule3682pcre6 }
};

/* references for sid 3682 */
static RuleReference rule3682ref1 =
{
    "bugtraq", /* type */
    "2524" /* value */
};

static RuleReference rule3682ref2 =
{
    "cve", /* type */
    "2001-0154" /* value */
};

static RuleReference rule3682ref3 =
{
    "url", /* type */
    "www.microsoft.com/technet/security/bulletin/MS01-020.mspx" /* value */
};

static RuleReference *rule3682refs[] =
{
    &rule3682ref1,
    &rule3682ref2,
    &rule3682ref3,
    NULL
};

RuleOption *rule3682options[] =
{
    &rule3682option1,
    &rule3682option2,
    &rule3682option3,
    &rule3682option4,
    &rule3682option5,
    &rule3682option6,
    NULL
};

Rule rule3682 = {

   /* rule header, akin to => tcp any any -> any any               */{
       IPPROTO_TCP, /* proto */
       "$EXTERNAL_NET", /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       "$SMTP_SERVERS", /* DSTIP     */
       "25", /* DSTPORT   */
   },
   /* metadata */
   {
       RULE_GID,  /* genid (HARDCODED!!!) */
       3682, /* sigid */
       2, /* revision */

       "attempted-admin", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "SMTP spoofed MIME-Type auto-execution attempt",     /* message */
       rule3682refs, /* ptr to references */
       NULL /* Meta data */
   },
   rule3682options, /* ptr to rule options */
   NULL,                               /* Use internal eval func */
   0,                                  /* Not initialized */
   0,                                  /* Rule option count, used internally */
   0,                                  /* Flag with no alert, used internally */
   NULL /* ptr to internal data... setup during rule registration */
};
