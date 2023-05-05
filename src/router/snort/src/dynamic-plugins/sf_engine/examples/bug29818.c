/*
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2006-2013 Sourcefire, Inc.
 *
 * Written by Brian Caswell <bmc@sourcefire.com>
 */

#include <string.h>
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_snort_plugin_api.h"
#include "detection_lib_meta.h"
#include "sf_snort_packet.h"

/* declare detection functions */
int ruleDHCPCATeval(void *p);

/* content for sid 2257 */
static ContentInfo ruleDHCPCATcontent1 =
{
    (u_int8_t *)("|63 82 53 63|"), /* pattern */
    4, /* depth */
    236, /* offset */
    CONTENT_FAST_PATTERN | CONTENT_BUF_RAW,
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

static RuleOption ruleDHCPCAToption1 =
{
    OPTION_TYPE_CONTENT,
    { &ruleDHCPCATcontent1 }
};

/* references for sid 2257 */
static RuleReference *ruleDHCPCATrefs[] =
{
    NULL
};

RuleOption *ruleDHCPCAToptions[] =
{
    &ruleDHCPCAToption1,
    NULL
};


Rule ruleDHCPCAT = {
   /* rule header, akin to => tcp any any -> any any               */{
       IPPROTO_UDP, /* proto */
       "any", /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       "any", /* DSTIP     */
       "68", /* DSTPORT   */
   },
   /* metadata */
   {
       RULE_GID,  /* genid (HARDCODED!!!) */
       7196, /* sigid */
       1, /* revision */

       "attempted-admin", /* classification XXX NOT PROVIDED BY GRAMMAR YET! */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "MISC DHCP option overflow attempt",     /* message */
       ruleDHCPCATrefs, /* ptr to references */
       NULL /* Meta data */
   },
   ruleDHCPCAToptions, /* ptr to rule options */
   ruleDHCPCATeval,                               /* Use internal eval func */
   0,                                  /* Not initialized */
   0,                                  /* Rule option count, used internally */
   0,                                  /* Flag with no alert, used internally */
   NULL /* ptr to internal data... setup during rule registration */
};

/* detection functions */
int ruleDHCPCATeval(void *p) {
    const u_int8_t *end;
    const u_int8_t *ptr;
    unsigned short type;
    unsigned short size;
    unsigned short sizes[255];
    SFSnortPacket *sp = (SFSnortPacket *) p;
    const u_int8_t *cursor_normal = 0;

    if (NULL == sp)
        return RULE_NOMATCH;

    if (NULL == sp->payload)
        return RULE_NOMATCH;

    /* offset for cookie + 2 options */
    if (740 > sp->payload_size)
        return RULE_NOMATCH;

    if (contentMatch(p, ruleDHCPCAToptions[0]->option_u.content, &cursor_normal)) {
        end = sp->payload + sp->payload_size;
        ptr = sp->payload + 240;

        memset(sizes, 0, sizeof(sizes));

        while (ptr + 2 < end)
        {
            type = (((u_int8_t) *(ptr))&0xFF);
            size = (((u_int8_t) *(ptr+1))&0xFF);
            if ((sizes[type] += size) > 500) {
                return RULE_MATCH;
            }
            ptr += 2 + size;
        }
    }

    return RULE_NOMATCH;
}

#if 0
Rule *rules[] = {
    &ruleDHCPCAT,
    NULL
};
#endif
