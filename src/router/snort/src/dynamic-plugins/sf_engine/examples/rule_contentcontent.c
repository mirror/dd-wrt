/*
 * C-Code Rule Example
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_snort_plugin_api.h"
#include "detection_lib_meta.h"

/* Forward Declaration */
int rule_abcContentX2Eval(void *p);

/*
alert tcp any any -> any any
(msg:"abcdefgghi test";
content:"ab"; nocase;
content:"d"; within:2; nocase;
content:"ef"; within:2; nocase;
content:"ghi"; distance:1; within:4; nocase;
sid:1000001; rev:1;)
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

static RuleReference *refs[] =
{
    NULL
};


#if 0
static FlowFlags flowflags1 =
{
    FLOW_TO_SERVER
};

static RuleOption option1 =
{
    OPTION_TYPE_FLOWFLAGS,
    { &flowflags1 }
};
#endif

static ContentInfo content1 =
{                           /* Content */
    (u_int8_t *)"ab",                   /* pattern */
    0,                      /* depth */
    0,                      /* offset */
    CONTENT_FAST_PATTERN | CONTENT_BUF_RAW | CONTENT_NOCASE, /* Flags */
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

static ContentInfo content2 =
{                           /* Content */
    (u_int8_t *)"d",                    /* pattern */
    2,                      /* depth */
    0,                      /* offset */
    CONTENT_BUF_RAW | CONTENT_NOCASE| CONTENT_RELATIVE, /* Flags */
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

static RuleOption option3 =
{
    OPTION_TYPE_CONTENT,
    { &content2 }
};

static ContentInfo content3 =
{                           /* Content */
    (u_int8_t *)"ef",
    2,                      /* depth */
    0,                     /* offset */
    CONTENT_BUF_RAW | CONTENT_NOCASE | CONTENT_RELATIVE,
    NULL,                   /* Holder for Boyer Ptr */
    NULL,
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

static RuleOption option4 =
{
    OPTION_TYPE_CONTENT,
    { &content3 }
};

static ContentInfo content4 =
{                           /* Content */
    (u_int8_t *)"ghi",
    4,                      /* depth */
    1,                     /* offset */
    CONTENT_BUF_RAW | CONTENT_NOCASE | CONTENT_RELATIVE,
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

static RuleOption option5 =
{
    OPTION_TYPE_CONTENT,
    { &content4 }
};

static RuleOption *options[] =
{
    &option2,
    &option3,
    &option4,
    &option5,
    NULL
};

Rule rule_abcContentX2 =
{
    /* A skeleton rule generated from the GUI contains user configured
     * IP & Port info */
    {                               /* IP Info */
        IPPROTO_TCP,                  /* Protocol */
        ANY_NET,                 /* SRC IP -- pulled fr skeleton rule */
        ANY_PORT,                     /* SRC Port -- pulled fr skeleton rule */
        0,                            /* uni-directional */
        ANY_NET,                 /* DST IP -- pulled fr skeleton rule */
        ANY_PORT,                          /* DST Port -- pulled fr skeleton rule */
    },

    {                               /* Rule Information */
        RULE_GID,                       /* Gen ID */
        1000001,                           /* Sig ID */
        1,                              /* Rev */

        "attempted-admin",              /* classification */
        0,                              /* Hard Coded Priority (overrides
                                           class priority if non-0) */
        "!! Dynamic !! abcdefgghi test", /* message */
        refs,                           /* References */
        NULL                            /* Meta data */
    },
    options,                            /* Options */
    NULL,                               /* Don't Use internal eval func */
    /* Use the eval Func below instead to see the failure case,
     * when the 2nd instance of the first content is the
     * one to use for relative matches for other options. */
    /* rule_abcContentX2Eval,*/         /* Don't Use internal eval func */
    0,                                  /* Not initialized */
    0,                                  /* Rule option count, used internally */
    0,                                  /* Flag with no alert, used internally */
    NULL /* ptr to internal data... setup during rule registration */
};
/*
alert tcp $EXTERNAL_NET any -> $HOME_NET 139 (msg:"abcefg...xyz test";
flow:to_server; content:"abc"; content:"xyz"; end_buffer; content: "efg";
sid:0; rev:1;)
*/
int rule_abcContentX2Eval(void *p)
{
    //const u_int8_t *uri_cur = 0;
    const u_int8_t *norm_cur = 0;
    //if (checkFlow(p, rule_abcContentX2.options[0]->option_u.flowFlags)>0)
    {
        if (contentMatch(p, rule_abcContentX2.options[1]->option_u.content, &norm_cur)>0)
        {
            if (contentMatch(p, rule_abcContentX2.options[2]->option_u.content, &norm_cur)>0)
            {
                if (contentMatch(p, rule_abcContentX2.options[3]->option_u.content, &norm_cur)>0)
                {
                    return RULE_MATCH;
                }
            }
        }
    }
    return RULE_NOMATCH;
}
