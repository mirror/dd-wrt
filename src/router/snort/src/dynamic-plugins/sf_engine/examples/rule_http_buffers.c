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
static FlowFlags httpBufferTestflow =
{
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption httpBufferTestoption0 =
{
    OPTION_TYPE_FLOWFLAGS,
    { &httpBufferTestflow }
};

/* content:"www.walmart.com"; http_header; */
static ContentInfo httpBufferTestcontent_1 =
{
    (u_int8_t*)"www.walmart.com",  /* pattern */
    0, /* depth */
    0, /* offset */
    CONTENT_FAST_PATTERN | CONTENT_BUF_HEADER, /* flags */
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

static RuleOption httpBufferTestoption1 =
{
    OPTION_TYPE_CONTENT,
    { &httpBufferTestcontent_1 }
};

/* content:"com.wm.visitor="; http_cookie; offset:8; */
static ContentInfo httpBufferTestcontent_2 =
{
    (u_int8_t*)"com.wm.visitor=",  /* pattern */
    0, /* depth */
    8, /* offset */
    CONTENT_BUF_COOKIE, /* flags */
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

static RuleOption httpBufferTestoption2 =
{
    OPTION_TYPE_CONTENT,
    { &httpBufferTestcontent_2 }
};

/* content:"com.wm.anoncart="; http_cookie; */
static ContentInfo httpBufferTestcontent_3 =
{
    (u_int8_t*)"com.wm.anoncart=",  /* pattern */
    0, /* depth */
    0, /* offset */
    CONTENT_BUF_COOKIE, /* flags */
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

static RuleOption httpBufferTestoption3 =
{
    OPTION_TYPE_CONTENT,
    { &httpBufferTestcontent_3 }
};

static RuleReference *httpBufferTestrefs[] =
{
    NULL
};

static RuleMetaData httpBufferTestMeta_1 =
{
    "service http"
};

static RuleMetaData *httpBufferTestMetadata[] =
{
    &httpBufferTestMeta_1,
    NULL
};

RuleOption *httpBufferTestoptions[] =
{
    &httpBufferTestoption0,
    &httpBufferTestoption1,
    &httpBufferTestoption2,
    &httpBufferTestoption3,
    NULL
};

Rule rule_httpBufferTest = {

   /* rule header, akin to => tcp any any -> any any               */{
       IPPROTO_TCP, /* proto */
       "$EXTERNAL_NET", /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       "$HOME_NET", /* DSTIP     */
       "80", /* DSTPORT   */
   },
   /* rule info metadata */
   {
       3,  /* genid (HARDCODED!!!) */
       99999999, /* sigid */
       1, /* revision */

       "attempted-recon", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "Cookie Test",     /* message */
       httpBufferTestrefs, /* ptr to references */
       httpBufferTestMetadata, /* ptr to references */
   },
   httpBufferTestoptions, /* ptr to rule options */
   NULL, /* use the built in detection function */
   0, /* am I initialized yet? */
   0, /* num options -- place holder */
   0, /* no alert flag -- place holder */
   NULL  /* rule data ptr -- place holder */
};

