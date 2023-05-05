/*
 * C-Code Rule Example
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_snort_plugin_api.h"
#include "detection_lib_meta.h"

/* Forward Declaration */
int testSidEval(void *p);

/*
alert tcp any any -> any any (msg:"Tortuga Dynamic Rule Test"; content:"AAABBB"; nocase; sid:123456; rev:0;)
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
static ContentInfo content1 =
{                           /* Content */
    (u_int8_t *)"AAABBB",                   /* pattern */
    0,                         /* depth -- ignored */
    0,                          /* offset -- ignored */
    CONTENT_NOCASE | CONTENT_FAST_PATTERN, /* Flags */
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

static RuleOption option1 =
{
    OPTION_TYPE_CONTENT,
    { &content1 }
};

static RuleOption *options[] =
{
    &option1,
    NULL
};

Rule testSid =
{
    /* A skeleton rule generated from the GUI contains user configured
     * IP & Port info */
    {                               /* IP Info */
        IPPROTO_TCP,                  /* Protocol */
        ANY_NET,                      /* SRC IP -- pulled fr skeleton rule */
        ANY_PORT,                     /* SRC Port -- pulled fr skeleton rule */
        0,                            /* uni-directional */
        ANY_NET,                      /* DST IP -- pulled fr skeleton rule */
        ANY_PORT,                     /* DST Port -- pulled fr skeleton rule */
    },

    {                               /* Rule Information */
        RULE_GID,                       /* Gen ID */
        123456,                         /* Sig ID */
        1,                              /* Rev */

        "web-application-attack",       /* classification */
        0,                              /* Hard Coded Priority (overrides
                                           class priority if non-0) */
        "Tortuga Dynamic Rule Test",    /* message */
        NULL,                           /* References */
        NULL                            /* Metadata*/
    },
    options,                            /* Options */
    &testSidEval,
    0,                                  /* Not initialized */
    0, /* num options -- initialized when rule is registered */
    0, /* Rule Don't alert flag -- initialized via Flowbit no alert */
    NULL /* Pointer to internal Rule Data -- initialized during rule registration */
};

int testSidEval(void *p)
{
    /* XXX: Do nothing, just match */
    //free(0x01234567);
    //printf("Div By 0 %d\n", 1/0);

    return RULE_MATCH;
}
