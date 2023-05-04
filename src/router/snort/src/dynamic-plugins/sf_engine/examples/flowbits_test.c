/*
 * C-Code Rule Example
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_snort_plugin_api.h"
#include "detection_lib_meta.h"
#include "sf_snort_packet.h"

/* Forward Declaration */
int testFlowbitsSetEval(void *p);
int testFlowbitsIsSetEval(void *p);
int testFlowbitsToggleEval(void *p);
int testFlowbitsIsNotSetEval(void *p);

/*
alert tcp $HOME_NET any <> [127.0.0.1,127.0.0.2,127.0.0.3,127.1.0.0/24,127.2.0.0/24,127.127.0.0/16] any (msg:"TEST flowbits set"; flags:S; flowbits:set,flowbits.test; classtype:bad-unknown; sid:700001; rev:1;)
alert tcp $HOME_NET any <> [127.0.0.1,127.0.0.2,127.0.0.3,127.1.0.0/24,127.2.0.0/24,127.127.0.0/16] any (msg:"TEST flowbits toggle"; flags:S; flowbits:toggle,flowbits.test; classtype:bad-unknown; sid:700002; rev:1;)

alert tcp $HOME_NET any -> $EXTERNAL_NET $HTTP_PORTS (msg:"TEST http_uri to localhost - flowbit set"; flowbits:isset,flowbits.test; content:"/"; http_uri; classtype:bad-unknown; sid:700003; rev:1;)
alert tcp $HOME_NET any -> $EXTERNAL_NET $HTTP_PORTS (msg:"TEST http_uri to localhost - flowbit not set"; flowbits:isnotset,flowbits.test; content:"/"; http_uri; classtype:bad-unknown; sid:700004; rev:1;)
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
static HdrOptCheck hdrOpt =
{   
    TCP_HDR_FLAGS,         /* field to check */
    CHECK_ATLEASTONE,      /* check that at least one of bits is set */
    TCPHEADER_SYN,         /* SYN */
    0,                     /* bits to ignore */
    0                      /* flags */
};

static RuleOption option1 =
{
    OPTION_TYPE_HDR_CHECK,
    { &hdrOpt }
};

/* flowbits:set "flowbits.test"; */
static FlowBitsInfo flowbitsOptSet =
{
    "flowbits.test",
    FLOWBIT_SET,
    0,
    0, /* flags */
    NULL, /*group name*/
    0,/*eval*/
    NULL, /*ids*/
    0 /*num_ids*/
};

static RuleOption option2 =
{
    OPTION_TYPE_FLOWBIT,
    { &flowbitsOptSet }
};

/* flowbits:isset "flowbits.test"; */
static FlowBitsInfo flowbitsOptIsSet =
{
    "flowbits.test",
    FLOWBIT_ISSET,
    0,
    0, /* flags */
    NULL, /*group name*/
    0,/*eval*/
    NULL, /*ids*/
    0 /*num_ids*/
};

static RuleOption option3 =
{
    OPTION_TYPE_FLOWBIT,
    { &flowbitsOptIsSet }
};

// content:"/", http_uri
static ContentInfo contentOpt =
{
    (u_int8_t *)("/"), /* pattern (now in snort content format) */
    0, /* depth */
    0, /* offset */
    CONTENT_BUF_URI, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
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
    { &contentOpt }
};

/* flowbits:toggle "flowbits.test"; */
static FlowBitsInfo flowbitsOptToggle =
{
    "flowbits.test",
    FLOWBIT_TOGGLE,
    0,
    0, /* flags */
    NULL, /*group name*/
    0,/*eval*/
    NULL, /*ids*/
    0 /*num_ids*/
};

static RuleOption option5 =
{
    OPTION_TYPE_FLOWBIT,
    { &flowbitsOptToggle }
};

/* flowbits:isnotset "flowbits.test"; */
static FlowBitsInfo flowbitsOptIsNotSet =
{
    "flowbits.test",
    FLOWBIT_ISNOTSET,
    0,
    0, /* flags */
    NULL, /*group name*/
    0,/*eval*/
    NULL, /*ids*/
    0 /*num_ids*/
};

static RuleOption option6 =
{
    OPTION_TYPE_FLOWBIT,
    { &flowbitsOptIsNotSet }
};

static RuleOption *setOptions[] =
{
    &option1,
    &option2,
    NULL
};

Rule testFlowbitsSet =
{
    /* A skeleton rule generated from the GUI contains user configured
     * IP & Port info */
    {                               /* IP Info */
        IPPROTO_TCP,                  /* Protocol */
        HOME_NET,                     /* SRC IP -- pulled fr skeleton rule */
        ANY_PORT,                     /* SRC Port -- pulled fr skeleton rule */
        1,                            /* uni-directional */
        "[127.0.0.1,127.0.0.2,127.0.0.3,127.1.0.0/24,127.2.0.0/24,127.127.0.0/16]",                      /* DST IP -- pulled fr skeleton rule */
        ANY_PORT,                     /* DST Port -- pulled fr skeleton rule */
    },

    {                               /* Rule Information */
        RULE_GID,                       /* Gen ID */
        800001,                        /* Sig ID */
        1,                              /* Rev */

        "bad-unknown",                  /* classification */
        0,                              /* Hard Coded Priority (overrides
                                           class priority if non-0) */
        "TEST flowbits set",    /* message */
        NULL,                           /* References */
        NULL                            /* Metadata*/
    },
    setOptions,                            /* Options */
    NULL,   // Use internal &testFlowbitsSetEval,
    0,                                  /* Not initialized */
    0, /* num options -- initialized when rule is registered */
    0, /* Rule Don't alert flag -- initialized via Flowbit no alert */
    NULL /* Pointer to internal Rule Data -- initialized during rule registration */
};

static RuleOption *issetOptions[] =
{
    &option3,
    &option4,
    NULL
};

Rule testFlowbitsIsSet =
{
    /* A skeleton rule generated from the GUI contains user configured
     * IP & Port info */
    {                               /* IP Info */
        IPPROTO_TCP,                  /* Protocol */
        HOME_NET,                     /* SRC IP -- pulled fr skeleton rule */
        ANY_PORT,                     /* SRC Port -- pulled fr skeleton rule */
        0,                            /* uni-directional */
        EXTERNAL_NET,                 /* DST IP -- pulled fr skeleton rule */
        HTTP_PORTS,                   /* DST Port -- pulled fr skeleton rule */
    },

    {                               /* Rule Information */
        RULE_GID,                       /* Gen ID */
        800003,                         /* Sig ID */
        1,                              /* Rev */

        "bad-unknown",                  /* classification */
        0,                              /* Hard Coded Priority (overrides
                                           class priority if non-0) */
        "TEST http_uri to localhost - flowbit isset",    /* message */
        NULL,                           /* References */
        NULL                            /* Metadata*/
    },
    issetOptions,                            /* Options */
    NULL,   // Use internal &testFlowbitsIsSetEval,
    0,                                  /* Not initialized */
    0, /* num options -- initialized when rule is registered */
    0, /* Rule Don't alert flag -- initialized via Flowbit no alert */
    NULL /* Pointer to internal Rule Data -- initialized during rule registration */
};

static RuleOption *toggleOptions[] =
{
    &option1,
    &option5,
    NULL
};

Rule testFlowbitsToggle =
{
    /* A skeleton rule generated from the GUI contains user configured
     * IP & Port info */
    {                               /* IP Info */
        IPPROTO_TCP,                  /* Protocol */
        HOME_NET,                     /* SRC IP -- pulled fr skeleton rule */
        ANY_PORT,                     /* SRC Port -- pulled fr skeleton rule */
        1,                            /* uni-directional */
        "[127.0.0.1,127.0.0.2,127.0.0.3,127.1.0.0/24,127.2.0.0/24,127.127.0.0/16]",                      /* DST IP -- pulled fr skeleton rule */
        ANY_PORT,                    /* DST Port -- pulled fr skeleton rule */
    },

    {                               /* Rule Information */
        RULE_GID,                       /* Gen ID */
        800002,                        /* Sig ID */
        1,                              /* Rev */

        "bad-unknown",                  /* classification */
        0,                              /* Hard Coded Priority (overrides
                                           class priority if non-0) */
        "TEST flowbits toggle",    /* message */
        NULL,                           /* References */
        NULL                            /* Metadata*/
    },
    toggleOptions,                            /* Options */
    NULL,   // Use internal &testFlowbitsToggleEval,
    0,                                  /* Not initialized */
    0, /* num options -- initialized when rule is registered */
    0, /* Rule Don't alert flag -- initialized via Flowbit no alert */
    NULL /* Pointer to internal Rule Data -- initialized during rule registration */
};

static RuleOption *isnotsetOptions[] =
{
    &option6,
    &option4,
    NULL
};

Rule testFlowbitsIsNotSet =
{
    /* A skeleton rule generated from the GUI contains user configured
     * IP & Port info */
    {                               /* IP Info */
        IPPROTO_TCP,                  /* Protocol */
        HOME_NET,                     /* SRC IP -- pulled fr skeleton rule */
        ANY_PORT,                     /* SRC Port -- pulled fr skeleton rule */
        0,                            /* uni-directional */
        EXTERNAL_NET,                 /* DST IP -- pulled fr skeleton rule */
        HTTP_PORTS,                   /* DST Port -- pulled fr skeleton rule */
    },

    {                               /* Rule Information */
        RULE_GID,                       /* Gen ID */
        800004,                        /* Sig ID */
        1,                              /* Rev */

        "bad-unknown",                  /* classification */
        0,                              /* Hard Coded Priority (overrides
                                           class priority if non-0) */
        "TEST http_uri to localhost - flowbit not set",    /* message */
        NULL,                           /* References */
        NULL                            /* Metadata*/
    },
    isnotsetOptions,                            /* Options */
    NULL,   // Use internal &testFlowbitsIsNotSetEval,
    0,                                  /* Not initialized */
    0, /* num options -- initialized when rule is registered */
    0, /* Rule Don't alert flag -- initialized via Flowbit no alert */
    NULL /* Pointer to internal Rule Data -- initialized during rule registration */
};

int testFlowbitsSetEval(void *p)
{
    /* XXX: Do nothing, just match */
    return RULE_MATCH;
}

int testFlowbitsIsSetEval(void *p)
{
    /* XXX: Do nothing, just match */
    return RULE_MATCH;
}

int testFlowbitsToggleEval(void *p)
{
    /* XXX: Do nothing, just match */
    return RULE_MATCH;
}

int testFlowbitsIsNotSetEval(void *p)
{
    /* XXX: Do nothing, just match */
    return RULE_MATCH;
}

