/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2012-2013 Sourcefire, Inc.
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

// alert tcp $EXTERNAL_NET $HTTP_PORTS -> $HOME_NET any (msg:"INDICATOR-OBFUSCATION hidden iframe - potential include of malicious content"; flow:to_client, established; file_data; content:"<iframe "; nocase; content:"width=1"; nocase; distance:0; within:50; content:"height=1"; nocase; distance:-40; within:80; content:"style=visibility|3a|hidden"; nocase; distance:-40; within:80; reference:url,blog.unmaskparasites.com/2009/10/28/evolution-of-hidden-iframes/; sid:1090591;)

static int rule1090591eval(void *p);

static FlowFlags rule1090591flow0 =
{
    FLOW_ESTABLISHED|FLOW_TO_CLIENT
};

static RuleOption rule1090591option0 =
{
    OPTION_TYPE_FLOWFLAGS,
    {
        &rule1090591flow0
    }
};

static CursorInfo rule1090591cursor1 =
{
    0,
    CONTENT_BUF_NORMALIZED,
    NULL, // offset_refId
    NULL  // offset_location
};

static RuleOption rule1090591option1 =
{
    OPTION_TYPE_FILE_DATA,
    {
        &rule1090591cursor1
    }
};

static ContentInfo rule1090591content2 =
{
    (u_int8_t *)"<iframe",
    0,     // depth
    0,     // offset
    CONTENT_NOCASE|CONTENT_BUF_NORMALIZED,  // flags
    NULL,  // holder for boyer/moore PTR
    NULL,  // more holder info - byteform
    0,     // byteform length
    0,     // increment length
    0,     // fast pattern offset
    0,     // fast pattern length
    0,     // fast pattern only
    NULL,  // offset_refId
    NULL,  // depth_refId
    NULL,  // offset_location
    NULL   // depth_location
};

static RuleOption rule1090591option2 =
{
    OPTION_TYPE_CONTENT,
    {
        &rule1090591content2
    }
};

static ContentInfo rule1090591content3 =
{
    (u_int8_t *)"width=1",
    50,    // depth
    0,     // offset
    CONTENT_NOCASE|CONTENT_BUF_NORMALIZED|CONTENT_RELATIVE,  // flags
    NULL,  // holder for boyer/moore PTR
    NULL,  // more holder info - byteform
    0,     // byteform length
    0,     // increment length
    0,     // fast pattern offset
    0,     // fast pattern length
    0,     // fast pattern only
    NULL,  // offset_refId
    NULL,  // depth_refId
    NULL,  // offset_location
    NULL   // depth_location
};

static RuleOption rule1090591option3 =
{
    OPTION_TYPE_CONTENT,
    {
        &rule1090591content3
    }
};

static ContentInfo rule1090591content4 =
{
    (u_int8_t *)"height=1",
    80,    // depth
    -40,   // offset
    CONTENT_NOCASE|CONTENT_BUF_NORMALIZED|CONTENT_RELATIVE,  // flags
    NULL,  // holder for boyer/moore PTR
    NULL,  // more holder info - byteform
    0,     // byteform length
    0,     // increment length
    0,     // fast pattern offset
    0,     // fast pattern length
    0,     // fast pattern only
    NULL,  // offset_refId
    NULL,  // depth_refId
    NULL,  // offset_location
    NULL   // depth_location
};

static RuleOption rule1090591option4 =
{
    OPTION_TYPE_CONTENT,
    {
        &rule1090591content4
    }
};

static ContentInfo rule1090591content5 =
{
    (u_int8_t *)"style=visibility|3a|hidden",
    80,    // depth
    -40,   // offset
    CONTENT_NOCASE|CONTENT_BUF_NORMALIZED|CONTENT_RELATIVE,  // flags
    NULL,  // holder for boyer/moore PTR
    NULL,  // more holder info - byteform
    0,     // byteform length
    0,     // increment length
    0,     // fast pattern offset
    0,     // fast pattern length
    0,     // fast pattern only
    NULL,  // offset_refId
    NULL,  // depth_refId
    NULL,  // offset_location
    NULL   // depth_location
};

static RuleOption rule1090591option5 =
{
    OPTION_TYPE_CONTENT,
    {
        &rule1090591content5
    }
};

static RuleReference rule1090591ref1 =
{
    "url",
    "blog.unmaskparasites.com/2009/10/28/evolution-of-hidden-iframes/"
};

static RuleReference *rule1090591refs[] =
{
    &rule1090591ref1,
    NULL
};

static RuleMetaData rule1090591service1 =
{
    "service http"
};

static RuleMetaData *rule1090591metadata[] =
{
    &rule1090591service1,
    NULL
};

RuleOption *rule1090591options[] =
{
    &rule1090591option0,
    &rule1090591option1,
    &rule1090591option2,
    &rule1090591option3,
    &rule1090591option4,
    &rule1090591option5,
    NULL
};

Rule rule1090591 =
{
    {
        IPPROTO_TCP,       // proto
        "$EXTERNAL_NET",   // source IPs
        "$HTTP_PORTS",     // source ports
        0,                 // direction
        "$HOME_NET",       // destination IPs
        "any",             // destination ports
    },
    {
        3,        // GID
        1090591,  // SID
        1,        // Revision
        NULL,     // classification
        0,        // hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET!
        "INDICATOR-OBFUSCATION hidden iframe - potential include of malicious content",
        rule1090591refs,
        rule1090591metadata
    },
    rule1090591options,
    &rule1090591eval, // use the built in detection function
    0,
    0,
    0,
    NULL
};

static int rule1090591eval(void *p)
{
    const u_int8_t *cursor_raw = NULL;
    SFSnortPacket *sp = (SFSnortPacket *)p;

    if (sp == NULL)
        return RULE_NOMATCH;

    if (sp->payload == NULL)
        return RULE_NOMATCH;

    if (checkFlow(p, rule1090591options[0]->option_u.flowFlags) <= 0)
        return RULE_NOMATCH;

    if (fileData(p, rule1090591options[1]->option_u.cursor, &cursor_raw) <= 0)
        return RULE_NOMATCH;

    if (contentMatch(p, rule1090591options[2]->option_u.content, &cursor_raw) <= 0)
        return RULE_NOMATCH;

    if (contentMatch(p, rule1090591options[3]->option_u.content, &cursor_raw) <= 0)
        return RULE_NOMATCH;

    if (contentMatch(p, rule1090591options[4]->option_u.content, &cursor_raw) <= 0)
        return RULE_NOMATCH;

    if (contentMatch(p, rule1090591options[5]->option_u.content, &cursor_raw) <= 0)
        return RULE_NOMATCH;

    return RULE_MATCH;
}

/*
Rule *rules[] = {
    &rule1090591,
    NULL
};
*/
