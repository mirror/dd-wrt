/*
 * Copyright (C) 2005-2013 Sourcefire, Inc. All Rights Reserved
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

#include "so-util.h"

//#define DEBUG
#ifdef DEBUG
#define DEBUG_SO(code) code
#else
#define DEBUG_SO(code)
#endif

/* declare detection functions */
int rule24973eval(void *p);

/* declare rule data structures */
/* flow:established, to_client; */
static FlowFlags rule24973flow0 = 
{
    FLOW_ESTABLISHED|FLOW_TO_CLIENT
};

static RuleOption rule24973option0 =
{
    OPTION_TYPE_FLOWFLAGS,
    {
        &rule24973flow0
    }
};
/* flowbits:isset "smb.trans2.fileinfo"; */
static FlowBitsInfo rule24973flowbits1 =
{
    "smb.trans2.fileinfo",
    FLOWBIT_ISSET,
    0,
};

static RuleOption rule24973option1 =
{
    OPTION_TYPE_FLOWBIT,
    {
        &rule24973flowbits1
    }
};
// content:"|FF|SMB2|00 00 00 00|", offset 4, depth 9, fast_pattern; 
static ContentInfo rule24973content2 = 
{
    (uint8_t *) "|FF|SMB2|00 00 00 00|", /* pattern (now in snort content format) */
    9, /* depth */
    4, /* offset */
    CONTENT_FAST_PATTERN|CONTENT_BUF_NORMALIZED, /* flags */
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0 /* increment length*/
};

static RuleOption rule24973option2 = 
{
    OPTION_TYPE_CONTENT,
    {
        &rule24973content2
    }
};
/* byte_test:size 1, value 128, operator &, relative; */
static ByteData rule24973byte_test3 = 
{
    1, /* size */
    CHECK_AND, /* operator */
    128, /* value */
    0, /* offset */
    0, /*multiplier */
    BYTE_BIG_ENDIAN|CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED|EXTRACT_AS_BYTE /* flags */
};

static RuleOption rule24973option3 = 
{
    OPTION_TYPE_BYTE_TEST,
    {
        &rule24973byte_test3
    }
};

/* flowbits:unset "smb.trans2,fileinfo"; */
static FlowBitsInfo rule24973flowbits4 =
{
    "smb.trans2.fileinfo",
    FLOWBIT_UNSET,
    0,
};

static RuleOption rule24973option4 =
{
    OPTION_TYPE_FLOWBIT,
    {
        &rule24973flowbits4
    }
};

// content:"|00 00 00 00 00 00 00 00 00 00|", offset 5, depth 10, relative;
static ContentInfo rule24973content5 = 
{
    (uint8_t *) "|00 00 00 00 00 00 00 00 00 00|", /* pattern (now in snort content format) */
    10, /* depth */
    5, /* offset */
    CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED, /* flags */
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0 /* increment length*/
};

static RuleOption rule24973option5 = 
{
    OPTION_TYPE_CONTENT,
    {
        &rule24973content5
    }
};

// content:"|00 00|", offset 13, depth 2, relative;
static ContentInfo rule24973content6 = 
{
    (uint8_t *) "|00 00|", /* pattern (now in snort content format) */
    2, /* depth */
    13, /* offset */
    CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED, /* flags */
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0 /* increment length*/
};

static RuleOption rule24973option6 = 
{
    OPTION_TYPE_CONTENT,
    {
        &rule24973content6
    }
};


/* references for sid 24973 */
/* reference: cve "2012-4774"; */
static RuleReference rule24973ref1 = 
{
    "cve", /* type */
    "2012-4774" /* value */
};

/* reference: url "technet.microsoft.com/en-us/security/bulletin/MS12-081"; */
static RuleReference rule24973ref2 = 
{
    "url", /* type */
    "technet.microsoft.com/en-us/security/bulletin/MS12-081" /* value */
};

static RuleReference *rule24973refs[] =
{
    &rule24973ref1,
    &rule24973ref2,
    NULL
};
/* metadata for sid 24973 */
/* metadata:service netbios-ssn, policy security-ips drop; */
static RuleMetaData rule24973service1 = 
{
    "service netbios-ssn"
};


static RuleMetaData rule24973policy1 = 
{
    "policy security-ips drop"
};

static RuleMetaData rule24973policy2 =
{   
    "policy max-detect-ips drop"
};

static RuleMetaData *rule24973metadata[] =
{
    &rule24973service1,
    &rule24973policy1,
    &rule24973policy2,
    NULL
};

RuleOption *rule24973options[] =
{
    &rule24973option0,
    &rule24973option1,
    &rule24973option2,
    &rule24973option3,
    &rule24973option4,
    &rule24973option5,
    &rule24973option6,
    NULL
};

Rule rule24973 = {
   /* rule header, akin to => tcp any any -> any any */
   {
       IPPROTO_TCP, /* proto */
       "$EXTERNAL_NET", /* SRCIP     */
       "[139,445]", /* SRCPORT   */
       0, /* DIRECTION */
       "$HOME_NET", /* DSTIP     */
       "any", /* DSTPORT   */
   },
   /* metadata */
   { 
       3,  /* genid */
       24973, /* sigid */
       9, /* revision */
       "attempted-admin", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "NETBIOS SMB Trans2 FIND_FIRST2 response file name length overflow attempt",     /* message */
       rule24973refs /* ptr to references */
       ,rule24973metadata
   },
   rule24973options, /* ptr to rule options */
   &rule24973eval, /* use the built in detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule24973eval(void *p) {
    const uint8_t *cursor_normal  = 0;
    const uint8_t *cursor_detect  = 0;
    const uint8_t *beg_of_payload = 0;
    const uint8_t *end_of_payload = 0;
    const uint8_t *last_entry_location = 0;
    const uint8_t *check = 0;
    SFSnortPacket *sp = (SFSnortPacket *) p;

    uint8_t  unicode_flag = 0;
    uint16_t search_count = 0;
    uint16_t parameter_offset = 0;
    uint16_t data_offset = 0;
    uint16_t last_entry_offset = 0;
    int i;

    uint32_t next_entry_offset = 0;
    uint32_t filename_length = 0;
    uint32_t end_of_entries = 0;
    uint32_t max_filename_length = 0;

    if(sp == NULL)
        return RULE_NOMATCH;

    if(sp->payload == NULL)
        return RULE_NOMATCH;
    
    // flow:established, to_client;
    if (checkFlow(p, rule24973options[0]->option_u.flowFlags) <= 0 ) {
	return RULE_NOMATCH;
    }

    // verify we have seen a fileinfo request
    // flowbits:isset "smb.trans2.fileinfo";
    if (processFlowbits(p, rule24973options[1]->option_u.flowBit) <= 0) {
	return RULE_NOMATCH;
    }

    // verify SMB, Trans2, and STATUS_SUCCESS
    // content:"|FF|SMB2|00 00 00 00|", offset 4, depth 9, fast_pattern;
    if (contentMatch(p, rule24973options[2]->option_u.content, &cursor_normal) <= 0) {
	return RULE_NOMATCH;
    }

    // verify we are looking at a response
    // byte_test:size 1, value 128, operator &, relative;
    if (byteTest(p, rule24973options[3]->option_u.byte, cursor_normal) <= 0) {
	return RULE_NOMATCH;
    }

    // Now unset the flowbit for the request
    if (processFlowbits(p, rule24973options[4]->option_u.flowBit) <= 0) {
        return RULE_NOMATCH;
    }

    // Get the beginning of payload and end of payload positions.
    if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_payload, &end_of_payload) <= 0) {
        return RULE_NOMATCH;
    }

    // check if we can read unicode_flag
    if(cursor_normal + 3 > end_of_payload)
        return RULE_NOMATCH;
        
    // read unicode flag
    unicode_flag = cursor_normal[2] & 0x80;

    // vulnerability is > 259 characters
    if(unicode_flag)
        max_filename_length = 518;
    else
        max_filename_length = 259;

    // verify reserved bytes
    // content:"|00 00 00 00 00 00 00 00 00 00|", offset 5, depth 10, relative, fast_pattern;
    if (contentMatch(p, rule24973options[5]->option_u.content, &cursor_normal) <= 0) {
	return RULE_NOMATCH;
    }

    // verify reserved bytes
    // content:"|00 00|", offset 13, depth 2, relative;
    if (contentMatch(p, rule24973options[6]->option_u.content, &cursor_normal) <= 0) {
	return RULE_NOMATCH;
    }

    // verify we can read:
    //    SMB parameter offset (distance:2 within:2)
    //    SMB data offset (distance:8 within:2)
    //
    if(cursor_normal + 10 > end_of_payload) {
       return RULE_NOMATCH;
    }

    // read the parameter offset (+4 for NetBIOS header)
    parameter_offset = read_little_16(cursor_normal + 2) + 4;

    // get data_offset (+4 for NetBIOS header)
    data_offset = read_little_16(cursor_normal + 8) + 4;

    // calculate position of search count
    // search count 2 bytes into parameters
    check = beg_of_payload + parameter_offset + 2;

    // overflow check
    if(check <= beg_of_payload)
        return RULE_NOMATCH;

    // overread check
    if(check > end_of_payload)
       return RULE_NOMATCH;

    // jump to search count
    cursor_normal = check;

    // verify we can read the search count
    if(cursor_normal + 2 > end_of_payload) {
	return RULE_NOMATCH;
    }

    // read the number of entries to check: search_count
    search_count = read_little_16(cursor_normal);

    // if we don't have any entries to check, return NOMATCH
    if(search_count == 0) {
	return RULE_NOMATCH;
    }

    // set the maximum entries we will loop through
    if(search_count > 10)
 	search_count = 10;

    // jump to the error offset
    cursor_normal += 4;

    // verify we can read the error offset (len:2), last entry offset (len:2)
    if(cursor_normal + 4 > end_of_payload) {
	return RULE_NOMATCH;
    }

    // verify there is no error
    if(*((uint16_t*)(cursor_normal)) != 0) // Byte order doesn't matter
        return RULE_NOMATCH;

    // read the last_entry_offset
    last_entry_offset = read_little_16(cursor_normal + 2);

    // at last_entry_offset+4 there should be a 4 byte value: end_of_entries
    // that should be |00 00 00 00|, check to see if we can read this value
    last_entry_location = beg_of_payload + data_offset + last_entry_offset;

    if(last_entry_location + 4 > end_of_payload) {
	return RULE_NOMATCH;
    }

    if(last_entry_location + 4 < beg_of_payload) {
	return RULE_NOMATCH;
    }

    // read end_of_entries
    end_of_entries = read_little_32(last_entry_location);

    // verify the last entry's next_entry_offset (end_of_entries) is 0
    if(end_of_entries != 0) {
	return RULE_NOMATCH;
    }

    DEBUG_SO( fprintf(stderr,"search_count = %d\n",search_count); )

    // position the cursor at the first entry length value
    check = beg_of_payload + data_offset;

    // overflow check
    if(check <= beg_of_payload)
        return RULE_NOMATCH;

    // overread check
    if(check > end_of_payload)
       return RULE_NOMATCH;

    // jump to first entry length value 
    cursor_normal = check;

    for(i=0; i<search_count; i++)
    {
	cursor_detect = cursor_normal;

    	// check if we can read the entry length (32 bit LE)
        if((cursor_detect + 68 > end_of_payload) || (cursor_normal < beg_of_payload)){
	    return RULE_NOMATCH;
        }

	// read the entry length, we use this later to get to the next entry
	next_entry_offset = read_little_32(cursor_detect);

	DEBUG_SO( fprintf(stderr,"got next_entry_offset = %u\n",next_entry_offset); )

        // position the cursor at the filename length
	cursor_detect += 60;

	// read the filename length
	filename_length = read_little_32(cursor_detect);
	DEBUG_SO( fprintf(stderr,"got filename_length = %u\n",filename_length); )

	// check for the vulnerability condition
	if(filename_length > max_filename_length)
 	    return RULE_MATCH;

        // calculate next entry position
        cursor_normal = cursor_normal + next_entry_offset;
    }

    return RULE_NOMATCH;
}
/*
Rule *rules[] = {
    &rule24973,
    NULL
};
*/
