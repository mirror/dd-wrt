/*
 * MS06-052 PGM Nak list overflow Vulnerability Detection
 *
 * Copyright (C) 2006 Sourcefire, Inc. All Rights Reserved
 *
 * Writen by Brian Caswell, Sourcefire VRT <bmc@sourcefire.com>
 * Research by Lurene Grenier, Sourcefire VRT <lgrenier@sourcefire.com>
 *
 * This file may contain proprietary rules that were created, tested and
 * certified by Sourcefire, Inc. (the "VRT Certified Rules") as well as
 * rules that were created by Sourcefire and other third parties and
 * distributed under the GNU General Public License (the "GPL Rules").  The
 * VRT Certified Rules contained in this file are the property of
 * Sourcefire, Inc. Copyright 2005 Sourcefire, Inc. All Rights Reserved.
 * The GPL Rules created by Sourcefire, Inc. are the property of
 * Sourcefire, Inc. Copyright 2002-2005 Sourcefire, Inc. All Rights
 * Reserved.  All other GPL Rules are owned and copyrighted by their
 * respective owners (please see www.snort.org/contributors for a list of
 * owners and their respective copyrights).  In order to determine what
 * rules are VRT Certified Rules or GPL Rules, please refer to the VRT
 * Certified Rules License Agreement.
 */


#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"

#define PGM_NAK_ERR -1
#define PGM_NAK_OK 0
#define PGM_NAK_VULN 1

typedef struct _PGM_NAK_OPT
{
        unsigned char     type;     /* 02 = vuln */
        unsigned char     len;
        unsigned char     res[2];
        unsigned int      seq[1];   /* could be many many more, but 1 is sufficent */
} PGM_NAK_OPT;

typedef struct _PGM_NAK
{
        unsigned int        seqnum;
        unsigned short      afi1;
        unsigned short      res1;
        unsigned int        src;
        unsigned short      afi2;
        unsigned short      res2;
        unsigned int        multi;
        PGM_NAK_OPT  opt;
} PGM_NAK;

typedef struct _PGM_HEADER
{
    unsigned short srcport;
    unsigned short dstport;
    unsigned char  type;
    unsigned char  opt;
    unsigned short checksum;
    unsigned char  gsd[6];
    unsigned short length;
    PGM_NAK nak;
} PGM_HEADER;

/* declare detection functions */
int rulePGMNAKeval(void *p);

/* references sid PGMNAK */
static RuleReference rulePGMNAKref1 = 
{
    "url", 
    "technet.microsoft.com/en-us/security/bulletin/ms06-052"
};

static RuleReference rulePGMNAKref2 =
{
    "cve",
    "2006-3442"
};

static RuleReference rulePGMNAKref3 =
{
    "bugtraq",
    "19922"
};

static RuleReference *rulePGMNAKrefs[] =
{
    &rulePGMNAKref1,
    &rulePGMNAKref2,
    &rulePGMNAKref3,
    NULL
};

static HdrOptCheck rulePGMNAKprotocheck =
{ 
    IP_HDR_PROTO,
    CHECK_EQ,
    0x71
};

/*
static RuleOption rulePGMNAKproto =
{
    OPTION_TYPE_HDR_CHECK,
    {
        &rulePGMNAKprotocheck
    }
};
*/

RuleOption *rulePGMNAKoptions[] =
{
//    &rulePGMNAKproto,
    NULL
};

Rule rulePGMNAK = {
   /* rule header, akin to => tcp any any -> any any               */
   {
       IPPROTO_IP, /* proto */
       EXTERNAL_NET,       /* SRCIP     */
       "any",        /* SRCPORT   */
       0,           /* DIRECTION */
       HOME_NET,       /* DSTIP     */
       "any",       /* DSTPORT   */
   },
   /* metadata */
   {
       3,       /* genid (HARDCODED!!!) */
       8351,       /* sigid 17ec3335-ff63-46b6-acff-5e755595f00c */
       7,       /* revision e321a2f2-4900-4e28-a35d-c0116ed695c7 */
   
       "attempted-admin", /* classification, generic */
       0,                 /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "OS-WINDOWS PGM nak list overflow attempt",     /* message */
       rulePGMNAKrefs /* ptr to references */
        ,NULL
   },
   rulePGMNAKoptions, /* ptr to rule options */
   &rulePGMNAKeval, /* ptr to rule detection function */
   0, /* am I initialized yet? */
   0, /* number of options */
   0  /* don't alert */
};

#define pgm_cksum_carry(x) (x = (x >> 16) + (x & 0xffff), (~(x + (x >> 16)) & 0xffff))

unsigned short pgm_checksum (const unsigned char *data, int length) {
    unsigned long sum = 0;
    int i;

    for (i = 0; i < length; i++) {
        if (i != 6 && i != 7) {
#if defined(__i386__) || defined (M_I86) || defined (_M_IX86) || defined (WIN32) || (__x86_64__)
			if (i % 2 == 0)
                sum += (((unsigned char *) data)[i]); 
            else 
                sum += ((unsigned char *) data)[i] << 8;
#else
            if (i % 2 == 0)
                sum += (((unsigned char *) data)[i]) << 8;
            else
                sum += ((unsigned char *) data)[i];
#endif
        }
    }
    return pgm_cksum_carry(sum);
}

int pgm_nak_detect (const unsigned char *data, unsigned int length) {
    unsigned int data_left;
    unsigned short checksum;
    PGM_HEADER *header;

    if (NULL == data) {
        return PGM_NAK_ERR;
    }

    /* request must be bigger than 44 bytes to cause vuln */
    if (length <= sizeof(PGM_HEADER)) {
        return PGM_NAK_ERR;
    }

    header = (PGM_HEADER *) data;

    if (8 != header->type) {
        return PGM_NAK_ERR;
    }

    if (2 != header->nak.opt.type) {
        return PGM_NAK_ERR;
    }


    /*
     * alert if the amount of data after the options is more than the length
     * specified.
     */


    data_left = length - 36;
    if (data_left > header->nak.opt.len) {

        /* checksum is expensive... do that only if the length is bad */
        if (header->checksum != 0) {
            checksum = pgm_checksum(data, length);
            if (0 == checksum) {
                if (header->checksum != 1) {
                    return PGM_NAK_ERR;
                }
            } else if (header->checksum != checksum) {
                return PGM_NAK_ERR;
            }
        }

        return PGM_NAK_VULN;
    }

    return PGM_NAK_OK;
}

/* detection functions */
int rulePGMNAKeval(void *p) {
    SFSnortPacket *sp = (SFSnortPacket *) p;
    const uint8_t *beg_of_payload, *end_of_payload;
 

    // return RULE_NOMATCH;
    if (NULL == sp)
        return RULE_NOMATCH;

    if (RULE_NOMATCH == checkHdrOpt(p, &rulePGMNAKprotocheck)) {
        return RULE_NOMATCH;
    }

    if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_payload, &end_of_payload) <= 0)
        return RULE_NOMATCH;

    if (PGM_NAK_VULN == pgm_nak_detect(beg_of_payload, (end_of_payload - beg_of_payload))) {
        return RULE_MATCH;
    }

    return RULE_NOMATCH;
}
