/*
 * C-Code Rule Example
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_snort_plugin_api.h"
#include "detection_lib_meta.h"

/* Forward Declaration */
int rule_abcxyzEval(void *p);

/*
alert tcp $EXTERNAL_NET any -> $HOME_NET 139 (msg:"abcefg...xyz test";
flow:to_server; content:"abc"; content:"xyz"; end_buffer; content: "efg";
sid:0; rev:1;)
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


static FlowFlags flowflags1 =
{
    FLOW_TO_SERVER
};

static RuleOption option1 =
{
    OPTION_TYPE_FLOWFLAGS,
    { &flowflags1 }
};

static ContentInfo content1 =
{                           /* Content */
    (u_int8_t *)"abc",                 /* pattern */
    0,                      /* depth */
    0,                      /* offset */
    CONTENT_FAST_PATTERN, /* Flags */
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

static RuleOption option2 =
{
    OPTION_TYPE_CONTENT,
    { &content1 }
};

static ContentInfo content2 =
{                           /* Content */
    (u_int8_t *)"hij",
    0,                      /* depth */
    0,                     /* offset */
    CONTENT_BUF_RAW | CONTENT_END_BUFFER,
    NULL,                   /* Holder for Boyer Ptr */
    NULL,
    0,
    0, /* increment length*/
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
    (u_int8_t *)"efg",
    0,                      /* depth */
    0,                     /* offset */
    CONTENT_BUF_RAW,
    NULL,                   /* Holder for Boyer Ptr */
    NULL,
    0,
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

static RuleOption *options[] =
{
    &option1,
    &option2,
    &option3,
    &option4,
    NULL
};

Rule rule_abcxyz =
{
    /* A skeleton rule generated from the GUI contains user configured
     * IP & Port info */
    {                               /* IP Info */
        IPPROTO_TCP,                  /* Protocol */
        EXTERNAL_NET,                 /* SRC IP -- pulled fr skeleton rule */
        ANY_PORT,                     /* SRC Port -- pulled fr skeleton rule */
        0,                            /* uni-directional */
        HOME_NET,                 /* DST IP -- pulled fr skeleton rule */
        "139",                          /* DST Port -- pulled fr skeleton rule */
    },

    {                               /* Rule Information */
        RULE_GID,                       /* Gen ID */
        1000000,                           /* Sig ID */
        1,                              /* Rev */

        "attempted-admin",              /* classification */
        0,                              /* Hard Coded Priority (overrides
                                           class priority if non-0) */
        "!! Dynamic !! abcefg...xyz test", /* message */
        refs,                           /* References */
        NULL                            /* Meta data */
    },
    options,                            /* Options */
    NULL,                               /* Use internal eval func */
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
int rule_abcxyzEval(void *p)
{
    //const u_int8_t *uri_cur = 0;
    const u_int8_t *norm_cur = 0;
    if (checkFlow(p, rule_abcxyz.options[0]->option_u.flowFlags)>0)
    {
        if (contentMatch(p, rule_abcxyz.options[1]->option_u.content, &norm_cur)>0)
        {
            if (contentMatch(p, rule_abcxyz.options[2]->option_u.content, &norm_cur)>0)
            {
                if (contentMatch(p, rule_abcxyz.options[3]->option_u.content, &norm_cur)>0)
                {
                    return RULE_MATCH;
                }
            }
        }
    }
    return RULE_NOMATCH;
}
