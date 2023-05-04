/*
 * SMB WriteX overflow - CVE-2006-5276
 * 
 * Copyright (C) 2007 Sourcefire, Inc. All Rights Reserved
 * 
 * Writen by Brian Caswell <bmc@sourcefire.com>
 * 
 * Structs from SPP_DCERPC, by Andrew Mullican <amullican@sourcefire.com>
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

// #include <syslog.h>

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef WIN32
#pragma pack(push,smb_hdrs,1)
#else
#pragma pack(1)
#endif

#if defined(__ia64__) || defined(__x86_64__) || defined(__i386__) || defined (M_I86) || defined (_M_IX86) || defined (WIN32)
#define smb_htons(A)  (A)
#define smb_htonl(A)  (A)
#define smb_ntohs(A)  (A)
#define smb_ntohl(A)  (A)
#else
#define smb_htons(A)  ((((uint16_t)(A) & 0xff00) >> 8) | (((uint16_t)(A) & 0x00ff) << 8))
#define smb_htonl(A)  ((((uint32_t)(A) & 0xff000000) >> 24) | (((uint32_t)(A) & 0x00ff0000) >> 8)  | (((uint32_t)(A) & 0x0000ff00) << 8)  | (((uint32_t)(A) & 0x000000ff) << 24))
#define smb_ntohs     smb_htons
#define smb_ntohl     smb_htonl
#endif

typedef struct _andx_header
{
    uint8_t wc; 
    uint8_t next_command;
    uint8_t reserved;
    uint16_t next_command_offset;
} ANDX_HEADER;

typedef struct _writex_header
{
    uint8_t wc;
    uint8_t next_command;
    uint8_t reserved;
    uint16_t next_command_offset;

    uint16_t fid;
    uint32_t offset;
    uint32_t reserved2;

    uint16_t writeMode;

    uint16_t remaining;
    uint16_t dataLengthHigh;
    uint16_t dataLength;
    uint16_t dataOffset;
    uint32_t highOffset;
    uint16_t byteCount;
} WRITEX_HEADER;

typedef struct _smb_header
{
    uint8_t protocol[4];      /* Should always be 0xff,SMB */
    uint8_t command;          /* Command code */

    union
    {
        /* 32 Bits */
        struct {
            uint8_t errClass; /* Error class */
            uint8_t reserved; /* Should be 0 */
            uint16_t err;     /* Error code */
        } dosErr;
        uint32_t ntErrCode;    /* 32-bit Error code */
    } status;

    uint8_t flags;            /* Flags */
    uint16_t flags2;          /* 8 bits weren't enough */

    union
    {
        uint16_t pad[6];      /* Make this 12 bytes long */
        struct
        {
            uint16_t pidHigh; /* Upper 16 bits of PID */
            uint32_t unused;
            uint32_t unusedToo;
        } extra;
    } extended;

    uint16_t tid;             /* Tree ID */
    uint16_t pid;             /* Process ID */
    uint16_t uid;             /* User ID */
    uint16_t mid;             /* Multiplex ID */
} SMB_HEADER;


/* declare detection functions */
int ruleWriteXeval(void *p);

static RuleReference ruleWriteXref1 = 
{
    "cve", 
    "2006-5276"
};

static RuleReference ruleWriteXref2 =
{
    "cve",
    "2008-4114"
};

static RuleReference *ruleWriteXrefs[] =
{
    &ruleWriteXref1,
    &ruleWriteXref2,
    NULL
};

static ContentInfo ruleWriteXcontent1 =
{
    (uint8_t *)"|FF|SMB", /* pattern */
    4, /* depth */
    4, /* offset */
    CONTENT_BUF_NORMALIZED | CONTENT_FAST_PATTERN,
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0 /* byteform length */
};

static RuleOption ruleWriteXoption0 =
{
    OPTION_TYPE_CONTENT,
    {
        &ruleWriteXcontent1
    }
};

RuleOption *ruleWriteXoptions[] =
{
    &ruleWriteXoption0,
    NULL
};

static RuleMetaData ruleWriteXservice1 =
{
    "service netbios-ssn"
};

static RuleMetaData ruleWriteXpolicy2 =
{
   "policy max-detect-ips drop"
};

static RuleMetaData *ruleWriteXmetadata[] =
{
    &ruleWriteXservice1,
    &ruleWriteXpolicy2,
    NULL
};


Rule ruleWriteX = {
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
       3,   /* genid (HARDCODED!!!) */
       10161,   /* sigid 854383f6-1272-4126-af13-273c4f4df425 */
       9,   /* revision  8c14a28e-2bbd-4c96-8e45-ce42decf3746 */
       "attempted-admin", /* classification, generic */
       0,   /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "NETBIOS SMB write_andx overflow attempt", /* message */
       ruleWriteXrefs /* ptr to references */
        ,ruleWriteXmetadata
   },
   ruleWriteXoptions, /* ptr to rule options */
   &ruleWriteXeval, /* ptr to rule detection function */
   0, /* am I initialized yet? */
   0, /* number of options */
   0  /* don't alert */
};

int process_writex (char *data, int size, int offset)
{
    WRITEX_HEADER *writex;
    uint32_t writex_len;
    uint16_t data_offset;
    
    if (size < offset + sizeof(WRITEX_HEADER)) {
        return -7;
    }

    writex = (WRITEX_HEADER *) (data + offset);
    data_offset = smb_ntohs(writex->dataOffset);
    
    if (offset > data_offset) {
        return 3;
    }

    writex_len = data_offset - offset;

    /* 
     * grr.  1 byte of pad is common.  1 byte of pad causes overwrite.
     * 1 byte overwrite can be exploitable.  Can't alert on 1 byte overwrite
     * cause that would cause us to alert on normal traffic.
     */
    if (writex_len > sizeof(WRITEX_HEADER) + 1) {
//        syslog(LOG_NOTICE, "writing %d to a buffer of size %d", writex_len, sizeof(WRITEX_HEADER));
//        printf("EVIL! %d > %d\n", writex_len, sizeof(WRITEX_HEADER));
        return 1;
    }

    return 0;
}

int process_command (char *data, int size, uint8_t command, int offset)
{
    ANDX_HEADER *andx;
    int ret = 0;
    uint16_t next_command_offset;
    
    if (offset > size)
        return -3;

    if (command == 0x2F) {
        ret = process_writex(data, size, offset);
        if (0 != ret) 
            return ret;
    }

    if (command == 0x24 || command == 0x2D || command == 0x2E || command == 0x2F ||
        command == 0x73 || command == 0x74 || command == 0x75 || command == 0xA2) {
        if (size < offset + sizeof(ANDX_HEADER)) {
            return -4;
        }
        andx = (ANDX_HEADER *) (data + offset);
        if (andx->next_command == 0xFF) // no more commands
        {
            return -5;
        }

        next_command_offset = smb_ntohs(andx->next_command_offset);

        if (next_command_offset > size) {
            return -6;
        }

        /* XXX - handle someone looping us around as the overflow */
        if (next_command_offset <= offset) {
            return 2;
        }

        return process_command(data, size, andx->next_command, next_command_offset);
    }

    return 0;
}

int process_packet(char *data, int size)
{
    SMB_HEADER *smb;
    
    if (size < (sizeof(SMB_HEADER) + 4)) {
        return -1;
    }
    data = data + 4;
    size = size - 4;

    smb = (SMB_HEADER *) (data);
/* rule option magic! */
//    if (0 != memcmp(smb->protocol, "\xFFSMB", 4)) {
//        return -2;
//    }

    return process_command(data, size, smb->command, 32);
}

/* detection functions */
int ruleWriteXeval(void *p) {
    int ret = 0;
    const uint8_t *cursor = 0, *beg_of_payload = 0, *end_of_payload = 0;

    SFSnortPacket *sp = (SFSnortPacket *) p;

    if (NULL == sp)
        return RULE_NOMATCH;

    if (NULL == sp->payload) 
        return RULE_NOMATCH;

    if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_payload, &end_of_payload) <= 0)
        return RULE_NOMATCH;

    /* minsize = 4 + sizeof(SMB_HEADER) + sizeof(WRITEX) */
    /* minsize = 4 + 32 +                 31 */
 
    if (67 > (end_of_payload - beg_of_payload)) {
        return RULE_NOMATCH;
    }

    /* 
     * this should use the API, but the API might do something additional, so
     * we do just like dcerpc does...
     */
    if(sp->flags & FLAG_FROM_SERVER)
        return RULE_NOMATCH;

    if (!(contentMatch(p, ruleWriteXoptions[0]->option_u.content, &cursor) > 0))
        return RULE_NOMATCH;

    ret = process_packet((char *)beg_of_payload, (int ) (end_of_payload - beg_of_payload));

    if (ret > 0)
        return RULE_MATCH;

    return RULE_NOMATCH;
}
