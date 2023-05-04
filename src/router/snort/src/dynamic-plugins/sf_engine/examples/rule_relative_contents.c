/*
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2008-2013 Sourcefire, Inc.
 *
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"


/* declare rule data structures */
/* precompile the stuff that needs pre-compiled */
/* flow:established, to_server; */
static FlowFlags relativeContentTestflow =
{
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption relativeContentTestoption0 =
{
    OPTION_TYPE_FLOWFLAGS,
    { &relativeContentTestflow }
};

/* content:"XYZ"; */
static ContentInfo relativeContentTestcontent_1 =
{
    (u_int8_t*)"XYZ",  /* pattern */
    0, /* depth */
    0, /* offset */
    CONTENT_BUF_NORMALIZED | CONTENT_FAST_PATTERN, /* flags */
    NULL, /* boyer ptr -- place holder */
    NULL, /* pattern byte form -- place holder */
    0, /* pattern byte form length -- place holder */
    0, /* increment length -- place holder */
    0,                      /* holder for fp offset */
    0,                      /* holder for fp length */
    0,                      /* holder for fp only */
    NULL, // offset_refId
    NULL, // depth_refId
    NULL, // offset_location
    NULL  // depth_location
};

static RuleOption relativeContentTestoption1 =
{
    OPTION_TYPE_CONTENT,
    { &relativeContentTestcontent_1 }
};

/* content:"RSTU"; offset:-7; depth:4; */
static ContentInfo relativeContentTestcontent_2 =
{
    (u_int8_t*)"RSTU",  /* pattern */
    4, /* depth */
    -9, /* offset */
    CONTENT_BUF_NORMALIZED | CONTENT_RELATIVE, /* flags */
    NULL, /* boyer ptr -- place holder */
    NULL, /* pattern byte form -- place holder */
    0, /* pattern byte form length -- place holder */
    0, /* increment length -- place holder */
    0,                      /* holder for fp offset */
    0,                      /* holder for fp length */
    0,                      /* holder for fp only */
    NULL, // offset_refId
    NULL, // depth_refId
    NULL, // offset_location
    NULL  // depth_location
};

static RuleOption relativeContentTestoption2 =
{
    OPTION_TYPE_CONTENT,
    { &relativeContentTestcontent_2 }
};

/* byte_test:1,=,86,0,relative; */
static ByteData relativeContentTestbytetest_3 =
{
    1, /* size */
    CHECK_EQ, /* operator */
    86, /* value */
    0, /* offset */
    0, /* multiplier */
    CONTENT_BUF_NORMALIZED | CONTENT_RELATIVE | EXTRACT_AS_BYTE, /* flags */
    0, /* offset */
    NULL, // offset_refId
    NULL, // value_refId
    NULL, // offset_location
    NULL  // value_location
};

static RuleOption relativeContentTestoption3 =
{
    OPTION_TYPE_BYTE_TEST,
    { &relativeContentTestbytetest_3 }
};

/* byte_test:1,=,86,1,relative; */
static ByteData relativeContentTestbytetest_4 =
{
    1, /* size */
    CHECK_EQ, /* operator */
    86, /* value */
    1, /* offset */
    0, /* multiplier */
    CONTENT_BUF_NORMALIZED | CONTENT_RELATIVE | EXTRACT_AS_BYTE, /* flags */
    0, /* offset */
    NULL, // offset_refId
    NULL, // value_refId
    NULL, // offset_location
    NULL  // value_location
};

static RuleOption relativeContentTestoption4 =
{
    OPTION_TYPE_BYTE_TEST,
    { &relativeContentTestbytetest_4 }
};

static RuleReference *relativeContentTestrefs[] =
{
    NULL
};

RuleOption *relativeContentTestoptions[] =
{
    &relativeContentTestoption0,
    &relativeContentTestoption1,
    &relativeContentTestoption2,
    &relativeContentTestoption3,
    NULL
};

Rule rule_relativeContentTest =
{
   /* rule header, akin to => tcp any any -> any any               */
   {
       IPPROTO_TCP, /* proto */
       "any", /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       "any", /* DSTIP     */
       "12345", /* DSTPORT   */
   },
   /* rule info metadata */
   {
       3,  /* genid (HARDCODED!!!) */
       1234512345, /* sigid */
       1, /* revision */
       NULL, /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "Relative Content Test",     /* message */
       relativeContentTestrefs, /* ptr to references */
       NULL, /* ptr to metadata */
   },
   relativeContentTestoptions, /* ptr to rule options */
   NULL, /* use the built in detection function */
   0, /* am I initialized yet? */
   0, /* num options -- place holder */
   0, /* no alert flag -- place holder */
   NULL /* rule data ptr -- place holder */
};

RuleOption *relativeContent2Testoptions[] =
{
    &relativeContentTestoption0,
    &relativeContentTestoption1,
    &relativeContentTestoption2,
    &relativeContentTestoption4,
    NULL
};

Rule rule_relativeContentTest2 =
{
    /* rule header, akin to => tcp any any -> any any               */
    {
        IPPROTO_TCP, /* proto */
        "any", /* SRCIP     */
        "any", /* SRCPORT   */
        0, /* DIRECTION */
        "any", /* DSTIP     */
        "12345", /* DSTPORT   */
    },
    /* rule info metadata */
    {
        3,  /* genid (HARDCODED!!!) */
        1234511111, /* sigid */
        1, /* revision */
        NULL, /* classification */
        0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
        "Relative Content Test (infinite)",     /* message */
        relativeContentTestrefs, /* ptr to references */
        NULL, /* ptr to metadata */
    },
    relativeContent2Testoptions, /* ptr to rule options */
    NULL, /* use the built in detection function */
    0, /* am I initialized yet? */
    0, /* num options -- place holder */
    0, /* no alert flag -- place holder */
    NULL /* rule data ptr -- place holder */
};

#if 0
Rule *rules[] =
{
    &rule_relativeContentTest,
    &rule_relativeContentTest2,
    NULL
};
#endif
