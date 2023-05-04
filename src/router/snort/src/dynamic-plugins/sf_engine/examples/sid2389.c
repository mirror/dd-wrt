/*
 * C-Code Rule Example
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pcre.h"
#include "sf_snort_plugin_api.h"
#include "detection_lib_meta.h"

/* Forward Declaration */
int sid2389Eval(void *p);

/*
alert tcp $EXTERNAL_NET any -> $HOME_NET 21
(msg:"FTP RNTO overflow attempt"; flow:to_server,established;
content:"RNTO"; nocase; isdataat:100,relative;
pcre:"/^RNTO\s[^\n]{100}/smi";
reference:bugtraq,8315; reference:cve,2003-0466;
classtype:attempted-admin;
reference:cve,2000-0133; reference:cve,2001-1021; sid:2389; rev:6;)
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
    "8315"                      /* ID */
};

static RuleReference ref2 =
{
    "cve",                  /* System Name */
    "2003-0466"                      /* ID */
};

static RuleReference ref3 =
{
    "cve",                  /* System Name */
    "2000-0133"                      /* ID */
};

static RuleReference ref4 =
{
    "cve",                  /* System Name */
    "2001-1021"                      /* ID */
};


static RuleReference *refs[] =
{
    &ref1,
    &ref2,
    &ref3,
    &ref4,
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
    (u_int8_t *)"RNTO",               /* pattern */
    0,                         /* depth -- ignored */
    0,                          /* offset -- ignored */
    CONTENT_NOCASE | CONTENT_FAST_PATTERN | CONTENT_BUF_NORMALIZED, /* Flags */
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

static PCREInfo pcre1 =
{                           /* PCRE */
    "^RNTO\\s[^\\n]{100}",               /* expression */
    NULL,                       /* Holder for compiled expr */
    NULL,                       /* Holder for compiled expr extra flags */
    PCRE_DOTALL | PCRE_MULTILINE | PCRE_CASELESS, /* Compile Flags */
    CONTENT_BUF_NORMALIZED,      /* Flags */
    0 /* offset */
};
static RuleOption option3 =
{
    OPTION_TYPE_PCRE,
    { &pcre1 }
};

static RuleOption *options[] =
{
    &option1,
    &option2,
    &option3,
    NULL
};

Rule sid2389 =
{
    /* A skeleton rule generated from the GUI contains user configured
     * IP & Port info */
    {                               /* IP Info */
        IPPROTO_TCP,                  /* Protocol */
        EXTERNAL_NET,                 /* SRC IP -- pulled fr skeleton rule */
        ANY_PORT,                     /* SRC Port -- pulled fr skeleton rule */
        0,                            /* uni-directional */
        HOME_NET,                     /* DST IP -- pulled fr skeleton rule */
        "21",                         /* DST Port -- pulled fr skeleton rule */
    },

    {                               /* Rule Information */
        RULE_GID,                       /* Gen ID */
        2389,                           /* Sig ID */
        6,                              /* Rev */

        "attempted-admin",       /* classification */
        0,                              /* Hard Coded Priority (overrides
                                           class priority if non-0) */
        "FTP RNTO overflow attempt", /* message */
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

int sid2389Eval(void *p)
{
    const u_int8_t *norm_cur = 0;
    if (checkFlow(p, sid2389.options[0]->option_u.flowFlags) )
    {
        /* This returns norm_cur to end of matched buffer */
        if (contentMatch(p, sid2389.options[1]->option_u.content, &norm_cur)>0)
        {
            /* Not relative to norm cursor */
            if (pcreMatch(p, sid2389.options[2]->option_u.pcre, NULL))
            {
                return RULE_MATCH;
            }
        }
    }
    return RULE_NOMATCH;
}
