
/*
 * VRT RULES
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_snort_plugin_api.h"
#include "detection_lib_meta.h"

/* declare detection functions */
int rule2489eval(void *p);

/*
alert tcp $EXTERNAL_NET any -> $HOME_NET 80 (msg:"EXPLOIT esignal STREAMQUOTE buffer overflow attempt";
flow:to_server,established; content:"<STREAMQUOTE>"; nocase; isdataat:1024,relative;
content:!"</STREAMQUOTE>"; within:1054; nocase; reference:bugtraq,9978; classtype:attempted-admin;
sid:2489; rev:2;)
*/

/* declare rule data structures */
/* precompile the stuff that needs pre-compiled */
/* flow for sid 2489 */
/* flow:established, to_server; */
static FlowFlags rule2489flow1 =
{
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule2489option1 =
{
    OPTION_TYPE_FLOWFLAGS,
    { &rule2489flow1 }
};


/* content for sid 2489 */
static ContentInfo rule2489content2 =
{
    (u_int8_t *)"<STREAMQUOTE>", /* pattern */
    0, /* depth */
    0, /* offset */
    CONTENT_FAST_PATTERN|CONTENT_NOCASE|CONTENT_BUF_NORMALIZED, /* flags */
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

static RuleOption rule2489option2 =
{
    OPTION_TYPE_CONTENT,
    { &rule2489content2 }
};

static CursorInfo rule2489cursor3 =
{
    1024,
    CONTENT_BUF_NORMALIZED,
    NULL, // offset_refId
    NULL  // offset_location
};

static RuleOption rule2489option3 =
{
    OPTION_TYPE_CURSOR,
    { &rule2489cursor3 }
};

/* content for sid 2489 */
static ContentInfo rule2489content4 =
{
    (u_int8_t *)"</STREAMQUOTE>", /* pattern */
    1054, /* depth */
    0, /* offset */
    CONTENT_NOCASE|CONTENT_BUF_NORMALIZED | NOT_FLAG, /* flags */
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

static RuleOption rule2489option4 =
{
    OPTION_TYPE_CONTENT,
    { &rule2489content4 }
};

/* references for sid 2489 */
static RuleReference *rule2489refs[] =
{
    NULL
};

RuleOption *rule2489options[] =
{
    &rule2489option1,
    &rule2489option2,
    &rule2489option3,
    &rule2489option4,
    NULL
};

Rule rule2489 = {

   /* rule header, akin to => tcp any any -> any any               */{
       IPPROTO_TCP, /* proto */
       "$EXTERNAL_NET", /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       "$HOME_NET", /* DSTIP     */
       "80", /* DSTPORT   */
   },
   /* metadata */
   {
       RULE_GID,  /* genid (HARDCODED!!!) */
       2489, /* sigid */
       2, /* revision */

       "attempted-admin", /* classification XXX NOT PROVIDED BY GRAMMAR YET! */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "!! Dynamic !!  EXPLOIT esignal STREAMQUOTE buffer overflow attempt",     /* message */
       rule2489refs, /* ptr to references */
       NULL /* metadata */
   },
   rule2489options, /* ptr to rule options */
    NULL,                               /* Use internal eval func */
    0,                                  /* Not initialized */
    0,                                  /* Rule option count, used internally */
    0,                                  /* Flag with no alert, used internally */
    NULL /* ptr to internal data... setup during rule registration */
};


/*
alert tcp $EXTERNAL_NET any -> $HOME_NET 80 (msg:"EXPLOIT esignal STREAMQUOTE buffer overflow attempt";
flow:to_server,established; content:"<STREAMQUOTE>"; nocase; isdataat:1024,relative;
content:!"</STREAMQUOTE>"; within:1054; nocase; reference:bugtraq,9978; classtype:attempted-admin;
sid:2489; rev:2;)
*/


/* detection functions */

int rule2489eval(void *p) {
    /* cursors, formally known as doe_ptr */
    const u_int8_t *cursor_normal = 0;

    /* flow:established, to_server; */
    if (checkFlow(p, rule2489options[0]->option_u.flowFlags)) {
        if (contentMatch(p, rule2489options[1]->option_u.content, &cursor_normal) > 0) {
            if (checkCursor(p, rule2489options[2]->option_u.cursor, cursor_normal) > 0 ) {
                if (contentMatch(p, rule2489options[3]->option_u.content, &cursor_normal) <= 0) {
                    return RULE_MATCH;
                }

            }
        }
    }
    return RULE_NOMATCH;
}

