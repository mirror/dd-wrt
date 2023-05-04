/*
 * C-Code Rule Example
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_snort_plugin_api.h"
#include "detection_lib_meta.h"

/* Forward Declaration */
int sid1970Eval(void *p);

/*
alert tcp $EXTERNAL_NET any -> $HOME_NET $HTTP_PORTS (msg:"WEB-IIS MDAC Content-Type overflow attempt"; flow:to_server,established; uricontent:"/msadcs.dll"; nocase; content:"Content-Type|3A|"; nocase; isdataat:50,relative; content:!"|0A|"; within:50; reference:bugtraq,6214; reference:cve,2002-1142; reference:url,www.foundstone.com/knowledge/randd-advisories-display.html?id=337; classtype:web-application-attack; sid:1970; rev:9;)
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
    "6214"                      /* ID */
};

static RuleReference ref2 =
{
    "cve",                      /* System Name */
    "2002-1142"                 /* ID */
};

static RuleReference ref3 =
{
    "url",                      /* System Name */
    "www.foundstone.com/knowledge/randd-advisories-display.html?id=337"
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
    (u_int8_t *)"msadcs.dll",               /* pattern */
    0,                         /* depth -- ignored */
    0,                          /* offset -- ignored */
    CONTENT_NOCASE | CONTENT_FAST_PATTERN | CONTENT_BUF_URI, /* Flags */
    NULL,                       /* Holder for Boyer Ptr */
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
    (u_int8_t *)"|3A|",                     /* pattern */
    0,                         /* depth -- ignored */
    0,                          /* offset -- ignored */
    CONTENT_NOCASE | CONTENT_BUF_NORMALIZED, /* Flags */
    NULL,                       /* Holder for Boyer Ptr */
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
    (u_int8_t *)"|0A|",
    50,                         /* depth */
    0,                          /* offset -- ignored */
    CONTENT_RELATIVE | CONTENT_BUF_NORMALIZED,           /* Flags */
    NULL,                       /* Holder for Boyer Ptr */
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

Rule sid1970 =
{
    /* A skeleton rule generated from the GUI contains user configured
     * IP & Port info */
    {                               /* IP Info */
        IPPROTO_TCP,                  /* Protocol */
        EXTERNAL_NET,                 /* SRC IP -- pulled fr skeleton rule */
        ANY_PORT,                     /* SRC Port -- pulled fr skeleton rule */
        0,                            /* uni-directional */
        HOME_NET,                     /* DST IP -- pulled fr skeleton rule */
        HTTP_PORTS,                   /* DST Port -- pulled fr skeleton rule */
    },

    {                               /* Rule Information */
        RULE_GID,                       /* Gen ID */
        1970,                           /* Sig ID */
        9,                              /* Rev */

        "web-application-attack",       /* classification */
        0,                              /* Hard Coded Priority (overrides
                                           class priority if non-0) */
        "WEB-IIS MDAC Content Type overflow attempt", /* message */
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

int sid1970Eval(void *p)
{
    const u_int8_t *uri_cur = 0;
    const u_int8_t *norm_cur = 0;
    if (checkFlow(p, sid1970.options[0]->option_u.flowFlags) )
    {
        /* This returns uri_cur to end of matched buffer */
        if (contentMatch(p, sid1970.options[1]->option_u.content, &uri_cur)>0)
        {
            /* This returns norm_cur to end of matched buffer */
            if(contentMatch(p, sid1970.options[2]->option_u.content, &norm_cur)>0)
            {
                /* The next content implicitly checks cursor is not
                 * beyond p's NORMALIZED buffer because of
                 * CONTENT_RELATIVE flag in contents[2]. */
                if (!contentMatch(p, sid1970.options[3]->option_u.content, &norm_cur))
                {
                    return RULE_MATCH;
                }
            }
        }
    }
    return RULE_NOMATCH;
}
