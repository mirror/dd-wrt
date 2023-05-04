/*
 * C-Code Rule Example
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"
#include "detection_lib_meta.h"

/* Forward Declaration */
int sid272Eval(void *p);

/*
alert ip $EXTERNAL_NET any -> $HOME_NET any
(msg:"DOS IGMP dos attack"; fragbits:M+; ip_proto:2;
reference:bugtraq,514; reference:cve,1999-0918;
classtype:attempted-dos; sid:272; rev:9;)
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
    "514"                      /* ID */
};

static RuleReference ref2 =
{
    "cve",                  /* System Name */
    "1999-0918"                      /* ID */
};

static RuleReference *refs[] =
{
    &ref1,
    &ref2,
    NULL
};

static HdrOptCheck hdrchk1 =
{                               /* Header Option */
    IP_HDR_FRAGBITS,               /* field to check */
    CHECK_ATLEASTONE,              /* check that at least one of bits is set */
    IP_MOREFRAGS,                  /* more fragments */
    0,                             /* bits to ignore */
    0                              /* flags */
};

static RuleOption option1 =
{
    OPTION_TYPE_HDR_CHECK,
    { &hdrchk1 }
};

static HdrOptCheck hdrchk2 =
{                               /* Header Option */
    IP_HDR_PROTO,               /* field to check */
    CHECK_EQ,                   /* check that it equals */
    2,                          /* 2 */
    0,                             /* bits to ignore */
    0                              /* flags */
};

static RuleOption option2 =
{
    OPTION_TYPE_HDR_CHECK,
    { &hdrchk2 }
};

static RuleOption *options[] =
{
    &option1,
    &option2,
    NULL
};

Rule sid272 =
{
    /* A skeleton rule generated from the GUI contains user configured
     * IP & Port info */
    {                               /* IP Info */
        IPPROTO_IP,                  /* Protocol */
        EXTERNAL_NET,                 /* SRC IP -- pulled fr skeleton rule */
        ANY_PORT,                     /* SRC Port -- pulled fr skeleton rule */
        0,                            /* uni-directional */
        HOME_NET,                     /* DST IP -- pulled fr skeleton rule */
        ANY_PORT,                         /* DST Port -- pulled fr skeleton rule */
    },

    {                               /* Rule Information */
        RULE_GID,                       /* Gen ID */
        272,                           /* Sig ID */
        9,                              /* Rev */

        "attempted-dos",       /* classification */
        0,                              /* Hard Coded Priority (overrides
                                           class priority if non-0) */
        "DOS IGMP dos attack", /* message */
        refs,                            /* References */
        NULL /* metadata */
    },
    options,                            /* Options */
    NULL,                               /* Use internal eval func */
    0,                                  /* Not initialized */
    0,                                  /* Rule option count, used internally */
    0,                                  /* Flag with no alert, used internally */
    NULL /* ptr to internal data... setup during rule registration */
};

int sid272Eval(void *p)
{
    if (checkHdrOpt(p, sid272.options[0]->option_u.hdrData) > 0 )
    {
        if (checkHdrOpt(p, sid272.options[1]->option_u.hdrData) > 0 )
        {
            return RULE_MATCH;
        }
    }
    return RULE_NOMATCH;
}
