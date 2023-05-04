/*
 * Use at your own risk.
 *
 * Copyright (C) 2005-2008 Sourcefire, Inc.
 * 
 */

/*
*
*  alert tcp $EXTERNAL_NET $HTTP_PORTS -> $HOME_NET any (msg:"WEB-CLIENT Microsoft GDI EMF malformed file buffer overflow attempt"; flow: to_client, established; content:" EMF"; content:"|01 00 00 00|"; distance: -44; within: 4; content:"EMF+"; distance: 0; reference: cve, 2008-3012; reference:url,technet.microsoft.com/en-us/security/bulletin/MS08-052; metadata: policy balanced-ips drop, policy security-ips drop, service http; classtype: attempted-user; sid:14259;)
*
*/


#include <string.h>

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"


//#define DEBUG
#ifdef DEBUG
#define DEBUG_WRAP(code) code
#else
#define DEBUG_WRAP(code)
#endif



/* declare detection functions */
int rule14259eval(void *p);

/* declare rule data structures */
/* precompile the stuff that needs pre-compiled */
/* flow:established, to_client; */
static FlowFlags rule14259flow0 = 
{
    FLOW_ESTABLISHED|FLOW_TO_CLIENT
};

static RuleOption rule14259option0 =
{
    OPTION_TYPE_FLOWFLAGS,
    {
        &rule14259flow0
    }
};
// content:" EMF"; 
static ContentInfo rule14259content1 = 
{
    (uint8_t *) "%^&YDFTY#jasdfasdf9023412043erASDFQ#!$w3ead043qeffadfaf", //DELETED " EMF", /* pattern (now in snort content format) */
    0, /* depth */
    0, /* offset */
    CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0 /* increment length*/
};

static RuleOption rule14259option1 = 
{
    OPTION_TYPE_CONTENT,
    {
        &rule14259content1
    }
};
//DELETED // content:"|01 00 00 00|", offset -44, depth 4, relative; 
//DELETED static ContentInfo rule14259content2 = 
//DELETED {
//DELETED     (uint8_t *) "|01 00 00 00|", /* pattern (now in snort content format) */
//DELETED     4, /* depth */
//DELETED     -44, /* offset */
//DELETED     CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
//DELETED     NULL, /* holder for boyer/moore PTR */
//DELETED     NULL, /* more holder info - byteform */
//DELETED     0, /* byteform length */
//DELETED     0 /* increment length*/
//DELETED };
//DELETED 
//DELETED static RuleOption rule14259option2 = 
//DELETED {
//DELETED     OPTION_TYPE_CONTENT,
//DELETED     {
//DELETED         &rule14259content2
//DELETED     }
//DELETED };
//DELETED // content:"EMF+", relative; 
//DELETED static ContentInfo rule14259content3 = 
//DELETED {
//DELETED     (uint8_t *) "EMF+", /* pattern (now in snort content format) */
//DELETED     0, /* depth */
//DELETED     0, /* offset */
//DELETED     CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
//DELETED     NULL, /* holder for boyer/moore PTR */
//DELETED     NULL, /* more holder info - byteform */
//DELETED     0, /* byteform length */
//DELETED     0 /* increment length*/
//DELETED };
//DELETED 
//DELETED static RuleOption rule14259option3 = 
//DELETED {
//DELETED     OPTION_TYPE_CONTENT,
//DELETED     {
//DELETED         &rule14259content3
//DELETED     }
//DELETED };

/* references for sid 14259 */
/* reference: cve "2008-3012"; */
static RuleReference rule14259ref1 = 
{
    "cve", /* type */
    "2008-3012" /* value */
};

/* reference: url "technet.microsoft.com/en-us/security/bulletin/MS08-052"; */
static RuleReference rule14259ref2 = 
{
    "url", /* type */
    "technet.microsoft.com/en-us/security/bulletin/MS08-052" /* value */
};

static RuleReference *rule14259refs[] =
{
    &rule14259ref1,
    &rule14259ref2,
    NULL
};
/* metadata for sid 14259 */
/* metadata:service http, policy balanced-ips drop, policy security-ips drop; */
//DELETED static RuleMetaData rule14259service1 = 
//DELETED {
//DELETED     "service http"
//DELETED };
//DELETED 
//DELETED 
//DELETED static RuleMetaData rule14259policy1 = 
//DELETED {
//DELETED     "policy balanced-ips drop"
//DELETED };
//DELETED 
//DELETED static RuleMetaData rule14259policy2 = 
//DELETED {
//DELETED     "policy security-ips drop"
//DELETED };


static RuleMetaData *rule14259metadata[] =
{
//DELETED     &rule14259service1,
//DELETED     &rule14259policy1,
//DELETED     &rule14259policy2,
    NULL
};
RuleOption *rule14259options[] =
{
    &rule14259option0,
    &rule14259option1,
//DELETED     &rule14259option2,
//DELETED     &rule14259option3,
    NULL
};

Rule rule14259 = {
   
   /* rule header, akin to => tcp any any -> any any               */{
       IPPROTO_TCP, /* proto */
       "$EXTERNAL_NET", /* SRCIP     */
       "$HTTP_PORTS", /* SRCPORT   */
       0, /* DIRECTION */
       "$HOME_NET", /* DSTIP     */
       "any", /* DSTPORT   */
   },
   /* metadata */
   { 
       3,  /* genid (HARDCODED!!!) */
       14259, /* sigid */
       6, /* revision */
       "attempted-user", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "DELETED WEB-CLIENT Microsoft GDI EMF malformed file buffer overflow attempt",     /* message */
       rule14259refs /* ptr to references */
       ,rule14259metadata
   },
   rule14259options, /* ptr to rule options */
   &rule14259eval, /* use the built in detection function */
//DELETED    0 /* am I initialized yet? */
   1
};


/* detection functions */
int rule14259eval(void *p) {
   const uint8_t *cursor_normal = 0;
   const uint8_t *beg_of_payload, *end_of_payload, *end_of_record;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   return RULE_NOMATCH;

//DELETED    uint32_t size;
//DELETED 
//DELETED    DEBUG_WRAP(printf("Starting run...\n"));
//DELETED 
//DELETED    if(sp == NULL)
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    if(sp->payload == NULL)
//DELETED       return RULE_NOMATCH;
//DELETED     
//DELETED    // flow:established, to_client;
//DELETED    if(checkFlow(p, rule14259options[0]->option_u.flowFlags) <= 0 )
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    // content:" EMF";
//DELETED    if(contentMatch(p, rule14259options[1]->option_u.content, &cursor_normal) <= 0)
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    // content:"|01 00 00 00|", offset -44, depth 4, relative;
//DELETED    if(contentMatch(p, rule14259options[2]->option_u.content, &cursor_normal) <= 0)
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    DEBUG_WRAP(printf("verified EMF header\n"));
//DELETED 
//DELETED    // We've verified the EMF header, now loop finding the EMP+ tag
//DELETED    if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_payload, &end_of_payload) <= 0)
//DELETED       return RULE_NOMATCH;
//DELETED 
//DELETED    // content:"EMF+", relative;
//DELETED    while(contentMatch(p, rule14259options[3]->option_u.content, &cursor_normal) > 0) {
//DELETED       DEBUG_WRAP(printf("Content Match EMF+\n"));
//DELETED 
//DELETED       // How big is the record we're checking? 
//DELETED       // no underflow check due to EMF header's size
//DELETED       size = *(cursor_normal - 8);
//DELETED       size |= *(cursor_normal - 7) << 8;
//DELETED       size |= *(cursor_normal - 6) << 16;
//DELETED       size |= *(cursor_normal - 5) << 24;
//DELETED 
//DELETED       DEBUG_WRAP(printf("Size is %d\n", size));
//DELETED 
//DELETED       // Calculate our end of record
//DELETED       end_of_record = ((cursor_normal + size > end_of_payload) || (cursor_normal + size < cursor_normal)) ? 
//DELETED                       end_of_payload : cursor_normal + size;
//DELETED 
//DELETED       cursor_normal += 4;    // jump over the size field
//DELETED       // Now, find the NaN sequence:  0x0000c0ff
//DELETED       while(cursor_normal + 4 < end_of_record) {
//DELETED          // check for 0xFFC00000 in little-endian format
//DELETED          if(memcmp(cursor_normal, "\x00\x00\xc0\xff", 4) == 0) 
//DELETED             return RULE_MATCH;
//DELETED          cursor_normal += 4;
//DELETED       }
//DELETED    }
//DELETED    return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule14259,
    NULL
};
*/

