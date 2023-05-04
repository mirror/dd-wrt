/*
 * openldap buffer overflow dos attempt
 * 
 * Copyright (C) 2006 Sourcefire, Inc. All Rights Reserved
 * 
 * Writen by Patrick Mullen <pmullen@sourcefire.com> 
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

/* declare detection functions */
static int ruleVD_OPENLDAPeval(void *p);

static RuleReference ruleVD_OPENLDAPref0 = 
{
    "bugtraq", /* type */
    "20939" /* value */
};
static RuleReference ruleVD_OPENLDAPcve =
{
    "cve", /* type */
    "2006-5779" /* value */
};


static RuleReference *ruleVD_OPENLDAPrefs[] =
{
    &ruleVD_OPENLDAPref0,
    &ruleVD_OPENLDAPcve,
    NULL
};

// missing features ends up having the rule built in a disabled state
//DELETED #ifdef MISSING_DELETED
static ContentInfo ruleVD_OPENLDAPcontent_missing_feature =
{
    (uint8_t *) VRT_RAND_STRING, /* pattern that should not invoke detection */
    0, /* depth */
    0, /* offset */
    CONTENT_BUF_NORMALIZED | CONTENT_FAST_PATTERN, /* flags */
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0 /* increment length*/
};

static RuleOption ruleVD_OPENLDAPmissing_feature =
{
    OPTION_TYPE_CONTENT,
    {
        &ruleVD_OPENLDAPcontent_missing_feature
    }
};
//DELETED #else
//DELETED static FlowFlags ruleVD_OPENLDAPflow =
//DELETED {
//DELETED     FLOW_ESTABLISHED|FLOW_TO_SERVER
//DELETED };
//DELETED 
//DELETED static RuleOption ruleVD_OPENLDAPoption0 =
//DELETED {
//DELETED     OPTION_TYPE_FLOWFLAGS,
//DELETED     {
//DELETED         &ruleVD_OPENLDAPflow
//DELETED     }
//DELETED };
//DELETED 
//DELETED static ContentInfo ruleVD_OPENLDAPcontent =
//DELETED {
//DELETED     (uint8_t *)"|30|",       /* pattern to search for */
//DELETED     1,                      /* depth */
//DELETED     0,                      /* offset */
//DELETED     CONTENT_BUF_NORMALIZED,                      /* flags */
//DELETED     NULL,                   /* holder for boyer/moore info */
//DELETED     NULL,                   /* holder for byte representation of "NetBus" */
//DELETED     0,                      /* holder for length of byte representation */
//DELETED     0                       /* holder of increment length */
//DELETED };
//DELETED 
//DELETED static RuleOption ruleVD_OPENLDAPoption1 =
//DELETED {
//DELETED     OPTION_TYPE_CONTENT,
//DELETED     {
//DELETED         &ruleVD_OPENLDAPcontent
//DELETED     }
//DELETED };
//DELETED #endif

RuleOption *ruleVD_OPENLDAPoptions[] =
{
//DELETED #ifdef MISSING_DELETED
    &ruleVD_OPENLDAPmissing_feature,
//DELETED #else
//DELETED     &ruleVD_OPENLDAPoption0,
//DELETED     &ruleVD_OPENLDAPoption1,
//DELETED #endif
    NULL
};

Rule ruleVD_OPENLDAP = {
   /* rule header */
   {
       IPPROTO_TCP, /* proto */
       "any", /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       HOME_NET, /* DSTIP     */
       "389", /* DSTPORT   */
   },
   /* metadata */
   { 
       3,  /* genid (HARDCODED!!!) */
       13416, /* sigid 69e4e9b3-9ce8-4f40-ab8e-cfeb46a77650 */
       4, /* revision 7f3b4b4a-3cce-4767-b2cf-65f48bdd0f3e */
   
       "attempted-dos", /* classification, generic */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
//DELETED        #ifdef MISSING_DELETED
//DELETED           "DELETED DOS openldap authcid name denial of service attempt - DISABLED",     /* message */
//DELETED        #else
          "DELETED DOS openldap authcid name denial of service attempt",     /* message */
//DELETED        #endif
       ruleVD_OPENLDAPrefs /* ptr to references */
       ,NULL
   },
   ruleVD_OPENLDAPoptions, /* ptr to rule options */
   &ruleVD_OPENLDAPeval, /* ptr to rule detection function */
//DELETED    #ifdef MISSING_DELETED
   1, /* force rule not to load by setting it to initialized. */
//DELETED    #else
//DELETED    0, /* am I initialized yet? */
//DELETED    #endif
   0, /* number of options */
   0  /* don't alert */
};

//DELETED 
//DELETED /* detection functions */
//DELETED 
//DELETED /* process_val
//DELETED 
//DELETED    Returns the uint32_t value contained at the pointer after skipping
//DELETED    preceeding NULs.  Returns an error if the data does not fit into
//DELETED    a uint32_t.
//DELETED */
//DELETED static int process_val(const uint8_t *data, uint32_t data_len, uint32_t *retvalue) {
//DELETED    uint32_t actual_data_len, i;      
//DELETED    *retvalue = 0;
//DELETED 
//DELETED    /* Jump over NULLs */
//DELETED    i = 0;
//DELETED    while((i < data_len) && (data[i] == 0)) {
//DELETED       i++;
//DELETED    }
//DELETED    actual_data_len = data_len - i; 
//DELETED    if(actual_data_len > 4) return(-1); /* Data doesn't fit into uint32_t */
//DELETED 
//DELETED    /* Now find the actual value */
//DELETED    for(;i<data_len;i++) {
//DELETED       *retvalue += data[i]<<(8*(data_len - i - 1));
//DELETED    }
//DELETED 
//DELETED    return(0);
//DELETED }
//DELETED 
//DELETED 
//DELETED /* skip_over_data
//DELETED  
//DELETED    Given an SFSnortPacket and a pointer to an index into the data,
//DELETED    this function will parse the size field at that index and move
//DELETED    the index to point after the size field and the data it describes.
//DELETED 
//DELETED    Size fields are as described in BER encoding.
//DELETED */
//DELETED static int skip_over_data(SFSnortPacket *sp, uint32_t *current_byte) {
//DELETED    uint32_t width = 0, value = 0;
//DELETED    int retval = 0;
//DELETED 
//DELETED    const uint8_t *beg_of_payload, *end_of_payload;
//DELETED    uint32_t payload_len;
//DELETED 
//DELETED    if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_payload, &end_of_payload) <= 0)
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    payload_len = end_of_payload - beg_of_payload;
//DELETED 
//DELETED    if(beg_of_payload[*current_byte] & 0x80) {
//DELETED       width = beg_of_payload[*current_byte] & 0x0F;
//DELETED       (*current_byte)++;
//DELETED 
//DELETED       if(*current_byte >= payload_len - width)
//DELETED          return(-1); 
//DELETED 
//DELETED       retval = process_val(&(beg_of_payload[*current_byte]), width, &value);
//DELETED       if(retval < 0)
//DELETED          return(-1);            /* width is > 4 */
//DELETED       *current_byte += width;   /* width of data width specifier */
//DELETED       *current_byte += value;   /* width of data itself */
//DELETED    }  else {
//DELETED       *current_byte += beg_of_payload[*current_byte] + 1;
//DELETED    }
//DELETED 
//DELETED    return(0);
//DELETED }


/* Detection algorithm --
   We're looking for more than 0x0101 (257) bytes of data in the authentication
   mechanism data field.  To do this, we minimally parse LDAP bind packets to
   get to the data and ensure we encounter the correct data types along the way.

   0x30 - Universal Sequence
   [Message size] - no data type, just a size
   [Message ID] - int data type (0x02)

   0x60 - Bind request
   [Bind Request size] - no data type, just a size
   [LDAP version] - int data type (0x02)
   [DN (username)] - string data type (0x04)

   0xa3 - Extended Auth Type, SASL
   [Auth Data Size] - no data type, just a size
   [Mechanism Name] - string data type
   [Mechanism Data] - string data type -- if > 0x0101 bytes long, RULE_MATCH

   Note we don't actually care if the data is present in this
   particular packet, which reduces evasion possibilities.
*/
static int ruleVD_OPENLDAPeval(void *p) {
//DELETED #ifdef MISSING_DELETED
    return RULE_NOMATCH; /* always fail */
//DELETED #else
//DELETED    uint32_t current_byte = 0;
//DELETED    uint32_t width, value;
//DELETED    int retval;
//DELETED 
//DELETED    const uint8_t *cursor_normal, *beg_of_payload, *end_of_payload;
//DELETED    uint32_t payload_len;
//DELETED 
//DELETED    SFSnortPacket *sp = (SFSnortPacket *) p;
//DELETED 
//DELETED    if(sp == NULL)
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    if(sp->payload == NULL)
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    /* call flow match */
//DELETED    if (checkFlow(sp, ruleVD_OPENLDAPoptions[0]->option_u.flowFlags) <= 0 )
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    /* call content match */
//DELETED    if (contentMatch(sp, ruleVD_OPENLDAPoptions[1]->option_u.content, &cursor_normal) <= 0) {
//DELETED       return RULE_NOMATCH;
//DELETED    }
//DELETED 
//DELETED    if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_payload, &end_of_payload) <= 0)
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    payload_len = end_of_payload - beg_of_payload;
//DELETED 
//DELETED    if(payload_len <= 26)   /* Minimum SASL bind request length (minus the auth data) */
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    /* our contentMatch already assures us the first byte is \x30, so just jump over it */
//DELETED    current_byte++;
//DELETED 
//DELETED    /* Begin packet structure processing */
//DELETED    /* Packet length (only care about width of the specifier) */
//DELETED    if(beg_of_payload[current_byte] & 0x80) {
//DELETED       current_byte += beg_of_payload[current_byte] & 0x0F; 
//DELETED    }
//DELETED    current_byte++;
//DELETED 
//DELETED    /* Message number (only care about width of the specifier) */
//DELETED    if(current_byte >= payload_len - 22)  
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    if(beg_of_payload[current_byte] != 0x02) /* Int data type */
//DELETED       return RULE_NOMATCH;
//DELETED    current_byte++;
//DELETED 
//DELETED    /* Skip over int width and the int value */
//DELETED    if(skip_over_data(sp, &current_byte) < 0)
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    if(current_byte >= payload_len - 19) 
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    /* Bind Request */
//DELETED    if(beg_of_payload[current_byte] != 0x60) 
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    current_byte++;
//DELETED 
//DELETED    /* Message length  (only care about width of the specifier) */
//DELETED    if(beg_of_payload[current_byte] & 0x80) {
//DELETED       current_byte += beg_of_payload[current_byte] & 0x0F; 
//DELETED    }
//DELETED    current_byte++;
//DELETED 
//DELETED    /* ldap version */
//DELETED    if(current_byte >= payload_len - 15)
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    /* ldap version */
//DELETED    if(beg_of_payload[current_byte] != 0x02) /* Int data type */
//DELETED       return RULE_NOMATCH;
//DELETED    current_byte++;
//DELETED 
//DELETED    /* Skip over int width and the int value */
//DELETED    if(skip_over_data(sp, &current_byte) < 0)
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    if(current_byte >= payload_len - 12)  
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    /* user name (DN) */
//DELETED    /* 0x04 - string data type */
//DELETED    if(beg_of_payload[current_byte] != 0x04) /* string data type */
//DELETED       return RULE_NOMATCH;
//DELETED    current_byte++;
//DELETED 
//DELETED    /* Skip over string length specifier and the string */
//DELETED    if(skip_over_data(sp, &current_byte) < 0)
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    if(current_byte >= payload_len - 10) 
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    /* 0xA3 - Auth type: SASL */
//DELETED    if(beg_of_payload[current_byte] != 0xA3)
//DELETED       return RULE_NOMATCH;
//DELETED    current_byte++;
//DELETED 
//DELETED    /* Auth data length - only care about width of specifier */
//DELETED    if(beg_of_payload[current_byte] & 0x80) {
//DELETED       current_byte += beg_of_payload[current_byte] & 0x0F;
//DELETED    }
//DELETED    current_byte++;
//DELETED 
//DELETED    if(current_byte >= payload_len - 6)  
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    /* Auth Mechanism */
//DELETED    /* 0x04 - string data type */
//DELETED    if(beg_of_payload[current_byte] != 0x04)  /* string data type */
//DELETED       return RULE_NOMATCH;
//DELETED    current_byte++;
//DELETED 
//DELETED    /* Skip over string length specifier and the string */
//DELETED    /* String value can be anything */
//DELETED    if(skip_over_data(sp, &current_byte) < 0)
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    if(current_byte >= payload_len - 4) 
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    /* Auth data */
//DELETED    /* 0x04 - string data type */
//DELETED    if(beg_of_payload[current_byte] != 0x04)  /* string data type */
//DELETED       return RULE_NOMATCH;
//DELETED    current_byte++;
//DELETED 
//DELETED    /* Here we can't just jump over the value because it's what
//DELETED       we're looking for. */
//DELETED    /* length of string -- if 0x0400 (1024 dec) it's an exploit attempt */
//DELETED    if(beg_of_payload[current_byte] & 0x80) {
//DELETED       width = beg_of_payload[current_byte] & 0x0F;
//DELETED       current_byte++;
//DELETED 
//DELETED       if(current_byte >= payload_len - width)
//DELETED          return RULE_NOMATCH;
//DELETED 
//DELETED       retval = process_val(&(beg_of_payload[current_byte]), width, &value);
//DELETED       if(retval < 0)
//DELETED          return RULE_NOMATCH;  /* width is either 0 or > 4 */
//DELETED       current_byte += width;   /* width of data width specifier */
//DELETED       /* value equals the length of the string */
//DELETED    }  else {
//DELETED       value = beg_of_payload[current_byte];  /* length of the string */
//DELETED       current_byte++;
//DELETED    }
//DELETED 
//DELETED    if(value > 0x0101)    /* minimum length determined through testing */
//DELETED       return RULE_MATCH;
//DELETED 
//DELETED    return RULE_NOMATCH;
//DELETED #endif
}

/*
Rule *rules[] = {
    &ruleVD_OPENLDAP,
    NULL
};
*/

