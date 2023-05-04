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
(msg:"content not content test";
content:"abcd"; nocase;
content:!"abcdefgh"; within:8; nocase;
sid:10000001; rev:1;)
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


static ContentInfo content1 =
{                           /* Content */
    (u_int8_t *)"abcd",                   /* pattern */
    0,                      /* depth */
    0,                      /* offset */
    /**** Add this to get it to alert in earlier versions
    CONTENT_FAST_PATTERN | */
    CONTENT_NOCASE | CONTENT_BUF_NORMALIZED, /* flags */
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

static RuleOption option1 =
{
    OPTION_TYPE_CONTENT,
    { &content1 }
};

static ContentInfo content2 =
{                           /* Content */
    (u_int8_t *)"abcdefgh",                    /* pattern */
    8,                      /* depth */
    0,                      /* offset */
    NOT_FLAG | CONTENT_NOCASE | CONTENT_BUF_NORMALIZED | CONTENT_RELATIVE, /* flags */
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
    { &content2 }
};

static RuleOption *options[] =
{
    &option1,
    &option2,
    NULL
};

Rule rule_contentNotContent =
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
        10000001,                       /* Sig ID */
        1,                              /* Rev */

        "attempted-admin",              /* classification */
        0,                              /* Hard Coded Priority (overrides
                                           class priority if non-0) */
        "!! Dynamic !! content not content test", /* message */
        refs,                           /* References */
        NULL                            /* Meta data */
    },
    options,                            /* Options */
    NULL,                               /* Don't Use internal eval func */
    0,                                  /* Not initialized */
    0,                                  /* Rule option count, used internally */
    0,                                  /* Flag with no alert, used internally */
    NULL /* ptr to internal data... setup during rule registration */
};
