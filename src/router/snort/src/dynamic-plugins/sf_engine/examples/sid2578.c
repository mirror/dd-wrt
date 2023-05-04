
/*
 * VRT RULES
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
 *
 */

/*
exploit.rules:alert udp $EXTERNAL_NET any -> $HOME_NET 88 (msg:"EXPLOIT kerberos principal name overflow UDP";
content:"|6A|"; depth:1; content:"|01 A1|"; asn1:oversize_length 1024,relative_offset -1;
reference:url,web.mit.edu/kerberos/www/advisories/MITKRB5-SA-2003-005-buf.txt; classtype:attempted-admin;
sid:2578; rev:1;)
*/
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_snort_plugin_api.h"
#include "detection_lib_meta.h"

/* declare detection functions */
int rule2578eval(void *p);


/* content for sid 2578 */
static ContentInfo rule2578content1 =
{
    (u_int8_t *)"|6A|", /* pattern */
    1, /* depth */
    0, /* offset */
    CONTENT_FAST_PATTERN | CONTENT_BUF_RAW, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
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

static RuleOption rule2578option1 =
{
    OPTION_TYPE_CONTENT,
    { &rule2578content1 }
};

/* content for sid 2578 */
static ContentInfo rule2578content2 =
{
    (u_int8_t *)"|01 A1|", /* pattern */
    0, /* depth */
    0, /* offset */
    CONTENT_BUF_RAW, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
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

static RuleOption rule2578option2 =
{
    OPTION_TYPE_CONTENT,
    { &rule2578content2 }
};


/* content for sid 2578 */
static Asn1Context rule2578content3 =
{
    0,
    0,
    0,
    1,
    1024,
    -1,
    ASN1_REL_OFFSET,
    0 /* flags */
};

static RuleOption rule2578option3 =
{
    OPTION_TYPE_ASN1,
    { &rule2578content3 }
};

/* references for sid 2578 */
static RuleReference *rule2578refs[] =
{
    NULL
};

RuleOption *rule2578options[] =
{
    &rule2578option1,
    &rule2578option2,
    &rule2578option3,
    NULL
};


Rule rule2578 = {

   /* rule header, akin to => tcp any any -> any any               */{
       IPPROTO_UDP, /* proto */
       "$EXTERNAL_NET", /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       "$HOME_NET", /* DSTIP     */
       "88", /* DSTPORT   */
   },
   /* metadata */
   {
       RULE_GID,  /* genid (HARDCODED!!!) */
       2578, /* sigid */
       1, /* revision */

       "attempted-admin", /* classification XXX NOT PROVIDED BY GRAMMAR YET! */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "!!Dynamic!! EXPLOIT kerberos principal name overflow UDP",     /* message */
       rule2578refs, /* ptr to references */
       NULL /* metadata */
   },
   rule2578options, /* ptr to rule options */
    NULL,                               /* Use internal eval func */
    0,                                  /* Not initialized */
    0,                                  /* Rule option count, used internally */
    0,                                  /* Flag with no alert, used internally */
    NULL /* ptr to internal data... setup during rule registration */
};



/* detection functions */

int rule2578eval(void *p) {
    /* cursors, formally known as doe_ptr */
    const u_int8_t *cursor_normal = 0;

    if (contentMatch(p, rule2578options[0]->option_u.content, &cursor_normal)) {
        if (contentMatch(p, rule2578options[1]->option_u.content, &cursor_normal) > 0) {
            if(detectAsn1(p, rule2578options[2]->option_u.asn1, cursor_normal)) {
                return RULE_MATCH;
            }
        }
    }
    return RULE_NOMATCH;
}

