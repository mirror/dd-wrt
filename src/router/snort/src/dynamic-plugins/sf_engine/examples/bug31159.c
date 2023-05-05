/*
 * SMTP RCPT-TO overflow detection
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2006-2013 Sourcefire, Inc. All Rights Reserved
 *
 * Writen by Brian Caswell <bmc@sourcefire.com>
 */

#include <ctype.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"

#define _SMTP_RCPTTO_MINSIZE 263
#define _SKIP_SPACE(p, e) while(p < e && isspace(*p)) { p++; }
#define _CHECK(callfunc) i = callfunc; if (i != RULE_MATCH) { return i; }
#define _INBOUNDS(start,end,ptr) (start <= ptr && ptr < end)

/* declare detection functions */
int rule_smtp_rcptto_eval(void *p);

static FlowFlags rule_smtp_rcptto_flow = {
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule_smtp_rcptto_option0 = {
    OPTION_TYPE_FLOWFLAGS,
    { &rule_smtp_rcptto_flow }
};

// content:"RCPT";
static ContentInfo rule_smtp_rcptto_content0  = {
    (u_int8_t *)"RCPT", /* pattern (now in snort content format) */
    0, /* depth */
    0, /* offset */
    CONTENT_RELATIVE|CONTENT_NOCASE|CONTENT_BUF_NORMALIZED, /* flags */
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
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

static RuleOption rule_smtp_rcptto_option1 = {
    OPTION_TYPE_CONTENT,
    { &rule_smtp_rcptto_content0 }
};

// content:"TO:";
static ContentInfo rule_smtp_rcptto_content1  =
{
    (u_int8_t *)"TO:", /* pattern (now in snort content format) */
    3, /* depth */
    0, /* offset */
    CONTENT_RELATIVE|CONTENT_NOCASE|CONTENT_BUF_NORMALIZED, /* flags */
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
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

static RuleOption rule_smtp_rcptto_option2 = {
    OPTION_TYPE_CONTENT,
    { &rule_smtp_rcptto_content1 }
};


RuleOption *rule_smtp_rcptto_options[] =
{
    &rule_smtp_rcptto_option0,
    &rule_smtp_rcptto_option1,
    &rule_smtp_rcptto_option2,
    NULL
};

/* reference:cve,2006-4379; */
static RuleReference rule_smtp_rcptto_ref0 =
{
    "cve", /* type */
    "2006-4379"
};

/* reference:url,www.ipswitch.com/support/imail/releases/im20061.asp; */
static RuleReference rule_smtp_rcptto_ref1 =
{
    "url", /* type */
    "www.ipswitch.com/support/imail/releases/im20061.asp",
};

static RuleReference *rule_smtp_rcptto_refs[] =
{
    &rule_smtp_rcptto_ref0,
    &rule_smtp_rcptto_ref1,
    NULL
};


Rule rule_smtp_rcptto_data = {
   /* rule header, akin to => tcp any any -> any any               */
   {
       IPPROTO_TCP, /* proto */
       "$EXTERNAL_NET", /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       "$SMTP_SERVERS", /* DSTIP     */
       "25", /* DSTPORT   */
   },
   /* metadata */
   {
       3, /* genid (HARDCODED!!!) */
       31159, /* sigid XXX */
       1, /* revision XXX */
       "attempted-admin",                       /* classification, generic */
       0,                                       /* priority, hardcoded */
       "SMTP RCPT TO proxy overflow attempt",   /* message */
       rule_smtp_rcptto_refs,                   /* ptr to references */
       NULL /* Meta data */
   },
   rule_smtp_rcptto_options, /* ptr to rule options */
   &rule_smtp_rcptto_eval, /* ptr to rule detection function */
   0, /* am I initialized yet? */
   0, /* number of options */
   0,  /* don't alert */
   NULL /* ptr to internal data... setup during rule registration */
};


int check_arrows(const u_int8_t *start, const u_int8_t *end) {
    unsigned int count = 0;
    const u_int8_t *ptr = start;

    /* not enough space */
    if (!_INBOUNDS(start, end, ptr + 256)) {
        return RULE_NOMATCH;
    }

    while(_INBOUNDS(start, end, ptr)) {
        if (*ptr == 0x40) { // @
            count++;
        } else if (*ptr == 0x3e) { // >
            break;
        } else if (*ptr == 0x0a || *ptr == 0x0d) { // \r or \n
            return -3; /* invalid RCPT */
        }
        ptr++;
    }

    if (2 <= count && (255 < ptr - start)) {
        return RULE_MATCH;
    }

    return RULE_NOMATCH;
}

int check_no_arrows(const u_int8_t *start, const u_int8_t *end) {
    unsigned int count = 0;
    const u_int8_t *ptr = start;

    while(_INBOUNDS(start, end, ptr)) {
        if (*ptr == 0x40) { // @
            count++;
        } else if (isspace((int)*ptr)) { // stop after spaces
            break;
        }
        ptr++;
    }

    if (2 <= count && (255 < ptr - start)) {
        return RULE_MATCH;
    }

    return RULE_NOMATCH;
}

/* detection functions */
int rule_smtp_rcptto_eval(void *p) {
    int i;
    SFSnortPacket *sp = (SFSnortPacket *) p;
    const u_int8_t *cursor = 0;
    const u_int8_t *start;
    const u_int8_t *end;
    const u_int8_t *to;


    if (NULL == sp)
        return RULE_NOMATCH;

    if (NULL == sp->payload)
        return RULE_NOMATCH;

    if (_SMTP_RCPTTO_MINSIZE > sp->payload_size)
        return RULE_NOMATCH;

    _CHECK(checkFlow(p, rule_smtp_rcptto_options[0]->option_u.flowFlags));

    cursor = sp->payload;
    end = sp->payload + sp->payload_size;

    while(_INBOUNDS(sp->payload, end - _SMTP_RCPTTO_MINSIZE, cursor)) {
        start = NULL;
        to = NULL;

        _CHECK(contentMatch(p, rule_smtp_rcptto_options[1]->option_u.content, &cursor));
        _SKIP_SPACE(cursor, end);
        _CHECK(contentMatch(p, rule_smtp_rcptto_options[2]->option_u.content, &cursor));
        _SKIP_SPACE(cursor, end);

        if (cursor + 255 >= end)
            return RULE_NOMATCH;

        to = cursor;
        while(cursor < end) {
            if (*cursor == 0x3c) { // <
                start = cursor;
                break;
            } else if (*cursor == 0x0a || *cursor == 0x0d) { // \r or \n
                break;
            }
            cursor++;
        }


        if (start != NULL) {
            if (RULE_MATCH == check_arrows(start, end)) {
                return RULE_MATCH;
            }
        } else {
            if (RULE_MATCH == check_no_arrows(to, cursor)) {
                return RULE_MATCH;
            }
        }

        while(_INBOUNDS(sp->payload, end - _SMTP_RCPTTO_MINSIZE, cursor)) {
            if (*cursor == 0x0a)
                break;
            cursor++;
        }
    }

    return RULE_NOMATCH;
}
