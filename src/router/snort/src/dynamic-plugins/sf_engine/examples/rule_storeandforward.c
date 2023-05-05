/*
 * C-Code Rule Example
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_snort_plugin_api.h"
#include "detection_lib_meta.h"

/* Forward Declaration */
int rule_smbWriteAndXEval(void *p);

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
    (u_int8_t *)"|ff|SMB/",                   /* pattern */
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

static RuleOption *options[] =
{
    &option2,
    NULL
};

Rule rule_smbWriteAndX =
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
        2000002,                           /* Sig ID */
        1,                              /* Rev */

        "attempted-admin",              /* classification */
        0,                              /* Hard Coded Priority (overrides
                                           class priority if non-0) */
        "!! Dynamic !! SMB Write AndX test", /* message */
        refs,                           /* References */
        NULL                            /* metadata */
    },
    options,                            /* Options */
    rule_smbWriteAndXEval,               /* Don't Use internal eval func */
    0,                                  /* Not initialized */
    0,                                  /* Rule option count, used internally */
    0,                                  /* Flag with no alert, used internally */
    NULL /* ptr to internal data... setup during rule registration */
};

static const RuleInformation info = {
	0,
	2000002,
	0,
};

int rule_smbWriteAndXEval(void *p)
{
    //const u_int8_t *uri_cur = 0;
    const u_int8_t *norm_cur = 0;
    int x, *data = NULL;

    if (contentMatch(p, rule_smbWriteAndX.options[0]->option_u.content, &norm_cur)== CONTENT_MATCH)
    {
        getRuleData(p, &info, (void**)&data, NULL);
        if (!data)
        {
            data = allocRuleData(sizeof(int));
            if (data == NULL)
                return RULE_NOMATCH;

            if (storeRuleData(p, &info, data, NULL) != RULE_MATCH)
            {
                freeRuleData(data);
                return RULE_NOMATCH;
            }
        }
        x = *data;
        x++;
        *data = x;
    }

    if (data && (*data > 50))
    {
        *data = 0;
        return RULE_MATCH;
    }

    return RULE_NOMATCH;
}
