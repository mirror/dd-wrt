/*
 * C-Code Rule Example
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_snort_plugin_api.h"
#include "detection_lib_meta.h"

/* Forward Declaration */
int sid2519Eval(void *p);

/*
alert tcp $EXTERNAL_NET any -> $SMTP_SERVERS 465 (msg:"SMTP Client_Hello overflow attempt";
flow:to_server,established; content:"|01|"; depth:1; offset:2; byte_test:2,>,0,6;
byte_test:2,!,0,8; byte_test:2,!,16,8; byte_test:2,>,20,10; content:"|8F|"; depth:1;
offset:11; byte_test:2,>,32768,0,relative; reference:bugtraq,10116; reference:cve,2003-0719;
reference:url,www.microsoft.com/technet/security/bulletin/MS04-011.mspx;
classtype:attempted-admin; sid:2519; rev:9;)
 *
*/

/* The GUI will run snort with a special flag to extract this information,
 * and generate a skeleton rule file with
 *   SRC IP/PORT info,
 *   DST IP/PORT info,
 *   Message
 *   SID
 *   GID
 *   REV
 *   References
 *
 * This skeleton rule file will be read as a normal rules file (just no
 * rule matching options specified.  When the shared library is loaded,
 * it will access the loaded rule info (by SID/GID hash) and update the
 * rest of the rule structure for processing (ie, handle any flowbits
 * checks, content & pcre premung'ing, & fast pattern registration.
 *
 * A check will be made for the rule revision.  Rule data will different
 * revision info, or non-existant SID/GIDs will cause a snort startup error.
 *
 */
static RuleReference ref1 =
{
    "bugtraq",                  /* System Name */
    "10116"                      /* ID */
};

static RuleReference ref2 =
{
    "cve",                      /* System Name */
    "2003-0719"                 /* ID */
};

static RuleReference ref3 =
{
    "url",                      /* System Name */
    "www.microsoft.com/technet/security/bulletin/MS04-011.mspx"
};

static RuleReference *refs[] =
{
    &ref1,
    &ref2,
    &ref3,
    NULL
};


static FlowFlags flowflags1 =
{
    FLOW_TO_SERVER | FLOW_ESTABLISHED
};

static RuleOption option1 =
{
    OPTION_TYPE_FLOWFLAGS,
    { &flowflags1 }
};

static ContentInfo content1 =
{                           /* Content */
    (u_int8_t *)"|01|",                 /* pattern */
    1,                      /* depth */
    2,                      /* offset */
    CONTENT_NOCASE | CONTENT_FAST_PATTERN, /* Flags */
    NULL,                   /* Holder for Boyer Ptr */
    NULL,
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

static RuleOption option2 =
{
    OPTION_TYPE_CONTENT,
    { &content1 }
};

static ByteData byte1 =
{                           /* Byte test */
    2,                      /* bytes */
    CHECK_GT,          /* operator */
    0,                      /* value to compare against */
    6,                      /* offset */
    0,                      /* multiplier */
    EXTRACT_AS_BYTE | CONTENT_BUF_RAW,                      /* flags */
    0, /* offset */
    NULL, // offset_refId
    NULL, // value_refId
    NULL, // offset_location
    NULL  // value_location
};

static RuleOption option3 =
{
    OPTION_TYPE_BYTE_TEST,
    { &byte1 }
};

static ByteData byte2 =
{                           /* Byte test */
    2,                      /* bytes */
    CHECK_NEQ,         /* operator */
    0,                      /* value to compare against */
    8,                      /* offset */
    0,                      /* multiplier */
    EXTRACT_AS_BYTE | CONTENT_BUF_RAW,                      /* flags */
    0, /* offset */
    NULL, // offset_refId
    NULL, // value_refId
    NULL, // offset_location
    NULL  // value_location
};

static RuleOption option4 =
{
    OPTION_TYPE_BYTE_TEST,
    { &byte2 }
};

static ByteData byte3 =
{                           /* Byte test */
    2,                      /* bytes */
    CHECK_NEQ,         /* operator */
    16,                     /* value to compare against */
    8,                      /* offset */
    0,                      /* multiplier */
    EXTRACT_AS_BYTE | CONTENT_BUF_RAW,                      /* flags */
    0, /* offset */
    NULL, // offset_refId
    NULL, // value_refId
    NULL, // offset_location
    NULL  // value_location
};

static RuleOption option5 =
{
    OPTION_TYPE_BYTE_TEST,
    { &byte3 }
};

static ByteData byte4 =
{                           /* Byte test */
    2,                      /* bytes */
    CHECK_GT,          /* operator */
    20,                     /* value to compare against */
    10,                     /* offset */
    0,                      /* multiplier */
    EXTRACT_AS_BYTE | CONTENT_BUF_RAW,                      /* flags */
    0, /* offset */
    NULL, // offset_refId
    NULL, // value_refId
    NULL, // offset_location
    NULL  // value_location
};

static RuleOption option6 =
{
    OPTION_TYPE_BYTE_TEST,
    { &byte4 }
};

static ContentInfo content2 =
{                           /* Content */
    (u_int8_t *)"|8F|",
    1,                      /* depth */
    11,                     /* offset */
    CONTENT_RELATIVE | CONTENT_BUF_NORMALIZED,           /* Flags */
    NULL,                   /* Holder for Boyer Ptr */
    NULL,
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

static RuleOption option7 =
{
    OPTION_TYPE_CONTENT,
    { &content2 }
};

static ByteData byte5 =
{                           /* Byte test */
    2,                      /* bytes */
    CHECK_GT,          /* operator */
    32768,                  /* value to compare against */
    0,                      /* offset */
    0,                      /* multiplier */
    EXTRACT_AS_BYTE | CONTENT_RELATIVE | CONTENT_BUF_RAW,       /* flags */
    0, /* offset */
    NULL, // offset_refId
    NULL, // value_refId
    NULL, // offset_location
    NULL  // value_location
};

static RuleOption option8 =
{
    OPTION_TYPE_BYTE_TEST,
    { &byte5 }
};

static RuleOption *options[] =
{
    &option1,
    &option2,
    &option3,
    &option4,
    &option5,
    &option6,
    &option7,
    &option8,
    NULL
};

Rule sid2519 =
{
    /* A skeleton rule generated from the GUI contains user configured
     * IP & Port info */
    {                               /* IP Info */
        IPPROTO_TCP,                  /* Protocol */
        EXTERNAL_NET,                 /* SRC IP -- pulled fr skeleton rule */
        ANY_PORT,                     /* SRC Port -- pulled fr skeleton rule */
        0,                            /* uni-directional */
        SMTP_SERVERS,                 /* DST IP -- pulled fr skeleton rule */
        "465",                          /* DST Port -- pulled fr skeleton rule */
    },

    {                               /* Rule Information */
        RULE_GID,                       /* Gen ID */
        2519,                           /* Sig ID */
        9,                              /* Rev */

        "attempted-admin",              /* classification */
        0,                              /* Hard Coded Priority (overrides
                                           class priority if non-0) */
        "SMTP Client_Hello overflow attempt", /* message */
        refs,                           /* References */
        NULL /* metadata */
    },
    options,                            /* Options */
    NULL,                               /* Use internal eval func */
    0,                                  /* Not initialized */
    0,                                  /* Rule option count, used internally */
    0,                                  /* Flag with no alert, used internally */
    NULL /* ptr to internal data... setup during rule registration */
};

int sid2519Eval(void *p)
{
    //const u_int8_t *uri_cur = 0;
    const u_int8_t *norm_cur = 0;
    if (checkFlow(p, sid2519.options[0]->option_u.flowFlags) )
    {
        if (contentMatch(p, sid2519.options[1]->option_u.content, &norm_cur)>0)
        {
            if(byteTest(p, sid2519.options[2]->option_u.byte, norm_cur))
            {
                if(byteTest(p, sid2519.options[3]->option_u.byte, norm_cur))
                {
                    if(byteTest(p, sid2519.options[4]->option_u.byte, norm_cur))
                    {
                        if(byteTest(p, sid2519.options[5]->option_u.byte, norm_cur))
                        {
                            if (contentMatch(p, sid2519.options[6]->option_u.content, &norm_cur)>0)
                            {
                                if(byteTest(p, sid2519.options[7]->option_u.byte, norm_cur))
                                {

                                    return RULE_MATCH;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return RULE_NOMATCH;
}
