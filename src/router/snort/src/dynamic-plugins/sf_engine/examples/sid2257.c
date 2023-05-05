
/*
 * VRT RULES
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
 *
 */

/*
alert udp $EXTERNAL_NET any -> $HOME_NET 135 (msg:"NETBIOS DCERPC Messenger Service buffer overflow attempt";
content:"|04 00|"; depth:2; byte_test:1,>,15,2,relative; byte_jump:4,86,little,align,relative;
byte_jump:4,8,little,align,relative; byte_test:4,>,1024,0,little,relative; reference:bugtraq,8826;
reference:cve,2003-0717; reference:url,www.microsoft.com/technet/security/bulletin/MS03-043.mspx;
classtype:attempted-admin; reference:nessus,11888; reference:nessus,11890; sid:2257; rev:8;)
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_snort_plugin_api.h"
#include "detection_lib_meta.h"

/* declare detection functions */
int rule2257eval(void *p);


/* content for sid 2257 */
static ContentInfo rule2257content1 =
{
    (u_int8_t *)"|04 00|", /* pattern */
    2, /* depth */
    0, /* offset */
    CONTENT_FAST_PATTERN | CONTENT_BUF_RAW,
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0, /* increment length */
    0,                      /* holder for fp offset */
    0,                      /* holder for fp length */
    0,                      /* holder for fp only */
    NULL, // offset_refId
    NULL, // depth_refId
    NULL, // offset_location
    NULL  // depth_location
};

static RuleOption rule2257option1 =
{
    OPTION_TYPE_CONTENT,
    { &rule2257content1 }
};

static ByteData rule2257byte2 =
{
    1,                      /* bytes */
    CHECK_GT,          /* operator */
    15,                      /* value to compare against */
    2,                      /* offset */
    0,                      /* multiplier */
    EXTRACT_AS_BYTE | CONTENT_RELATIVE | CONTENT_BUF_RAW,
    0, /* offset */
    NULL, // offset_refId
    NULL, // value_refId
    NULL, // offset_location
    NULL  // value_location
};

static RuleOption rule2257option2 =
{
    OPTION_TYPE_BYTE_TEST,
    { &rule2257byte2 }
};

static ByteData rule2257byte3 =
{
    4,                      /* bytes */
    0,                      /* operator */
    0,                      /* value to compare against */
    86,                     /* offset */
    0,                      /* multiplier */
    EXTRACT_AS_BYTE | BYTE_LITTLE_ENDIAN | JUMP_ALIGN | CONTENT_RELATIVE | CONTENT_BUF_RAW,
    0, /* offset */
    NULL, // offset_refId
    NULL, // value_refId
    NULL, // offset_location
    NULL  // value_location
};

static RuleOption rule2257option3 =
{
    OPTION_TYPE_BYTE_JUMP,
    { &rule2257byte3 }
};

static ByteData rule2257byte4 =
{
    4,                      /* bytes */
    0,                      /* operator */
    0,                      /* value to compare against */
    8,                      /* offset */
    0,                      /* multiplier */
    EXTRACT_AS_BYTE | BYTE_LITTLE_ENDIAN | JUMP_ALIGN | CONTENT_RELATIVE | CONTENT_BUF_RAW,
    0, /* offset */
    NULL, // offset_refId
    NULL, // value_refId
    NULL, // offset_location
    NULL  // value_location
};

static RuleOption rule2257option4 =
{
    OPTION_TYPE_BYTE_JUMP,
    { &rule2257byte4 }
};

static ByteData rule2257byte5 =
{
    4,                      /* bytes */
    CHECK_GT,          /* operator */
    1024,                      /* value to compare against */
    0,                      /* offset */
    0,                      /* multiplier */
    EXTRACT_AS_BYTE | BYTE_LITTLE_ENDIAN | CONTENT_RELATIVE | CONTENT_BUF_RAW,
    0, /* offset */
    NULL, // offset_refId
    NULL, // value_refId
    NULL, // offset_location
    NULL  // value_location
};

static RuleOption rule2257option5 =
{
    OPTION_TYPE_BYTE_TEST,
    { &rule2257byte5 }
};

/* references for sid 2257 */
static RuleReference *rule2257refs[] =
{
    NULL
};

RuleOption *rule2257options[] =
{
    &rule2257option1,
    &rule2257option2,
    &rule2257option3,
    &rule2257option4,
    &rule2257option5,
    NULL
};


Rule rule2257 = {

   /* rule header, akin to => tcp any any -> any any               */{
       IPPROTO_UDP, /* proto */
       "$EXTERNAL_NET", /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       "$HOME_NET", /* DSTIP     */
       "135", /* DSTPORT   */
   },
   /* metadata */
   {
       RULE_GID,  /* genid (HARDCODED!!!) */
       2257, /* sigid */
       1, /* revision */

       "attempted-admin", /* classification XXX NOT PROVIDED BY GRAMMAR YET! */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "!!Dynamic!! NETBIOS DCERPC Messenger Service buffer overflow attempt",     /* message */
       rule2257refs, /* ptr to references */
       NULL /* metadata */
   },
   rule2257options, /* ptr to rule options */
    NULL,                               /* Use internal eval func */
    0,                                  /* Not initialized */
    0,                                  /* Rule option count, used internally */
    0,                                  /* Flag with no alert, used internally */
    NULL /* ptr to internal data... setup during rule registration */
};
/*
alert udp $EXTERNAL_NET any -> $HOME_NET 135 (msg:"NETBIOS DCERPC Messenger Service buffer overflow attempt";
content:"|04 00|"; depth:2; byte_test:1,>,15,2,relative; byte_jump:4,86,little,align,relative;
byte_jump:4,8,little,align,relative; byte_test:4,>,1024,0,little,relative; reference:bugtraq,8826;
reference:cve,2003-0717; reference:url,www.microsoft.com/technet/security/bulletin/MS03-043.mspx;
classtype:attempted-admin; reference:nessus,11888; reference:nessus,11890; sid:2257; rev:8;)
*/

/* detection functions */

int rule2257eval(void *p) {
    /* cursors, formally known as doe_ptr */
    const u_int8_t *cursor_normal = 0;

    if (contentMatch(p, rule2257options[0]->option_u.content, &cursor_normal)) {
        if (byteTest(p, rule2257options[1]->option_u.byte, cursor_normal) > 0) {
            if(byteJump(p, rule2257options[2]->option_u.byte, &cursor_normal)) {
                if(byteJump(p, rule2257options[3]->option_u.byte, &cursor_normal)) {
                    if (byteTest(p, rule2257options[4]->option_u.byte, cursor_normal) > 0) {
                        return RULE_MATCH;
                    }
                }
            }
        }
    }
    return RULE_NOMATCH;
}

