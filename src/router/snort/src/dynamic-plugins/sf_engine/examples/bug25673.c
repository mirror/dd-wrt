/*
 * VRT RULES
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2006-2013 Sourcefire, Inc.
 *
 * Writen by Lurene Grenier <lurene.grenier> & Brian Caswell <bmc@sourcefire.com>
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"


/* declare detection functions */
int ruleWINNYeval(void *p);

/* references for sid WINNY */
/* reference:url,www.microsoft.com/technet/security/bulletin/ms06-007.mspx"; */
static RuleReference ruleWINNYref1 =
{
    "url", /* type */
    "en.wikipedia.org/wiki/Winny" /* value XXX - update me */
};

static RuleReference *ruleWINNYrefs[] =
{
    &ruleWINNYref1,
    NULL
};

static FlowFlags ruleWINNYflow1 =
{
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption ruleWINNYoption1 =
{
    OPTION_TYPE_FLOWFLAGS,
    { &ruleWINNYflow1 }
};

RuleOption *ruleWINNYoptions[] =
{
    &ruleWINNYoption1,
    NULL
};

Rule ruleWINNY = {
   /* rule header, akin to => tcp any any -> any any               */
   {
       IPPROTO_TCP, /* proto */
       "any", /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       "any", /* DSTIP     */
       "any", /* DSTPORT   */
   },
   /* metadata */
   {
       3,  /* genid (HARDCODED!!!) */
       7019, /* sigid */
       1, /* revision */

       "policy-violation", /* classification, generic */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "P2P WinNY connection attempt",     /* message */
       ruleWINNYrefs, /* ptr to references */
       NULL /* Meta data */
   },
   ruleWINNYoptions, /* ptr to rule options */
   &ruleWINNYeval, /* ptr to rule detection function */
   0, /* am I initialized yet? */
   0, /* number of options */
   0, /* don't alert */
   NULL /* ptr to internal data... setup during rule registration */
};

/* detection functions */
int ruleWINNYeval(void *p) {
    SFSnortPacket *sp = (SFSnortPacket *) p;

    if (checkFlow(p, ruleWINNYoptions[0]->option_u.flowFlags))
    {

        if (NULL == sp)
            return RULE_NOMATCH;

        if (NULL == sp->payload)
            return RULE_NOMATCH;

        if (11 != sp->payload_size)
            return RULE_NOMATCH;

        /* Pass in key location, key length, encrypted data location, data expected, data length */
        return MatchDecryptedRC4(&sp->payload[2], 4, &sp->payload[6], (u_int8_t *)("\x1\x0\x0\x0\x61"), 5);
    }

    return RULE_NOMATCH;
}

