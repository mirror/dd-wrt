/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2010-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"

//alert tcp $EXTERNAL_NET any -> $HOME_NET 1024: (msg:"mqqm QMDeleteObject overflow attempt"; flow:established,to_server; dce_iface:fdb3a030-065f-11d1-bb9b-00a024ea5525; dce_opnum:9; dce_stub_data; content:"|01 00 00 00|"; within:4; distance:4; content:"|03 00 00 00|"; within:4; distance:4; byte_test:4,>,256,8,dce; reference:cve,2005-0059; reference:url,www.microsoft.com/technet/security/Bulletin/MS05-017.mspx; classtype:protocol-command-decode; sid:1000028;)

/* declare detection functions */
//int rule1000028eval(void *p);

static FlowFlags rule1000028flow0 =
{
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule1000028option0 =
{
    OPTION_TYPE_FLOWFLAGS,
    {
        &rule1000028flow0
    }
};

static PreprocessorOption rule1000028preproc1 =
{
    "dce_iface",
    "fdb3a030-065f-11d1-bb9b-00a024ea5525",
    0,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static RuleOption rule1000028option1 =
{
    OPTION_TYPE_PREPROCESSOR,
    {
        &rule1000028preproc1
    }
};

static PreprocessorOption rule1000028preproc2 =
{
    "dce_opnum",
    "9",
    0,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static RuleOption rule1000028option2 =
{
    OPTION_TYPE_PREPROCESSOR,
    {
        &rule1000028preproc2
    }
};

static PreprocessorOption rule1000028preproc3 =
{
    "dce_stub_data",
    NULL,
    0,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static RuleOption rule1000028option3 =
{
    OPTION_TYPE_PREPROCESSOR,
    {
        &rule1000028preproc3
    }
};

static ContentInfo rule1000028content4 =
{
    (u_int8_t *)"|01 00 00 00|", /* pattern (now in snort content format) */
    4, /* depth */
    4, /* offset */
    CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0, /* increment length*/
    0, /* fast pattern offset */
    0, /* fast pattern length */
    0, /* fast pattern only */
    NULL, // offset_refId
    NULL, // depth_refId
    NULL, // offset_location
    NULL  // depth_location
};

static RuleOption rule1000028option4 =
{
    OPTION_TYPE_CONTENT,
    {
        &rule1000028content4
    }
};

static ContentInfo rule1000028content5 =
{
    (u_int8_t *)"|03 00 00 00|", /* pattern (now in snort content format) */
    4, /* depth */
    4, /* offset */
    CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0, /* increment length*/
    0, /* fast pattern offset */
    0, /* fast pattern length */
    0, /* fast pattern only */
    NULL, // offset_refId
    NULL, // depth_refId
    NULL, // offset_location
    NULL  // depth_location
};

static RuleOption rule1000028option5 =
{
    OPTION_TYPE_CONTENT,
    {
        &rule1000028content5
    }
};

static PreprocessorOption rule1000028preproc6 =
{
    "byte_test dce",
    "4,>,256,8,dce",
    0,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static RuleOption rule1000028option6 =
{
    OPTION_TYPE_PREPROCESSOR,
    {
        &rule1000028preproc6
    }
};

/* references for sid 1000028 */
/* reference: cve "2008-4835"; */
static RuleReference rule1000028ref1 =
{
    "cve", /* type */
    "2005-0059" /* value */
};

/* reference: url "www.microsoft.com/technet/security/bulletin/MS09-001.mspx"; */
static RuleReference rule1000028ref2 =
{
    "url", /* type */
    "www.microsoft.com/technet/security/Bulletin/MS05-017.mspx"
};

static RuleReference *rule1000028refs[] =
{
    &rule1000028ref1,
    &rule1000028ref2,
    NULL
};

/* metadata for sid 1000028 */
/* metadata:service netbios-ssn; */
static RuleMetaData rule1000028service1 =
{
    "service netbios-ssn"
};

static RuleMetaData *rule1000028metadata[] =
{
    &rule1000028service1,
    NULL
};

RuleOption *rule1000028options[] =
{
    &rule1000028option0,
    &rule1000028option1,
    &rule1000028option2,
    &rule1000028option3,
    &rule1000028option4,
    &rule1000028option5,
    &rule1000028option6,
    NULL
};

Rule rule1000028 =
{
    {
        /* rule header, akin to => tcp any any -> any any */
        IPPROTO_TCP, /* proto */
        "$EXTERNAL_NET", /* SRCIP     */
        "any", /* SRCPORT   */
        0, /* DIRECTION */
        "$HOME_NET", /* DSTIP     */
        "1024:", /* DSTPORT   */
    },
    /* metadata */
    {
        3,  /* genid (HARDCODED!!!) */
        1000028, /* sigid */
        1, /* revision */
        "protocol-command-decode", /* classification */
        0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
        "mqqm QMDeleteObject overflow attempt",
        rule1000028refs, /* ptr to references */
        rule1000028metadata
    },
    rule1000028options, /* ptr to rule options */
    NULL, // &rule1000028eval, /* use the built in detection function */
    0, /* am I initialized yet? */
    0,
    0,
    NULL
};

/*
Rule *rules[] = {
    &rule1000028,
    NULL
};
*/
