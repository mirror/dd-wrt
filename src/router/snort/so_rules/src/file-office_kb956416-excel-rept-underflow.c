/* DOES NOT USE BUILT-IN DETECTION FUNCTION !!
alert tcp $EXTERNAL_NET $HTTP_PORTS -> $HOME_NET any (msg:"WEB-CLIENT Excel rept integer underflow attempt"; flow:to_client,established; flowbits:isset, file.xls; content:"|41 1e|"; content:"|06 00|"; metadata: policy balanced-ips drop, policy security-ips drop, service http; classtype:attempted-user; reference:cve,2008-4019; reference:url,technet.microsoft.com/en-us/security/bulletin/MS08-057; sid:14655;)*/
/*
 * Use at your own risk.
 *
 * Copyright (C) 2005-2008 Sourcefire, Inc.
 * 
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
int rule14655eval(void *p);

/* declare rule data structures */
/* precompile the stuff that needs pre-compiled */
/* flow:established, to_client; */
static FlowFlags rule14655flow0 = 
{
    FLOW_ESTABLISHED|FLOW_TO_CLIENT
};

static RuleOption rule14655option0 =
{
    OPTION_TYPE_FLOWFLAGS,
    {
        &rule14655flow0
    }
};
/* flowbits:isset "file.xls"; */
static FlowBitsInfo rule14655flowbits1 =
{
    "file.xls",
    FLOWBIT_ISSET,
    0,
};

static RuleOption rule14655option1 =
{
    OPTION_TYPE_FLOWBIT,
    {
        &rule14655flowbits1
    }
};
// content:"A|1E|"; 
static ContentInfo rule14655content2 = 
{
    (uint8_t *) "A|1E|", /* pattern (now in snort content format) */
    0, /* depth */
    0, /* offset */
    CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0 /* increment length*/
};

static RuleOption rule14655option2 = 
{
    OPTION_TYPE_CONTENT,
    {
        &rule14655content2
    }
};
// content:"|06 00|"; 
static ContentInfo rule14655content3 = 
{
    (uint8_t *) "|06 00|", /* pattern (now in snort content format) */
    0, /* depth */
    0, /* offset */
    CONTENT_BUF_NORMALIZED | CONTENT_RELATIVE, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0 /* increment length*/
};

static RuleOption rule14655option3 = 
{
    OPTION_TYPE_CONTENT,
    {
        &rule14655content3
    }
};

/* references for sid 14655 */
/* reference: cve "2008-4019"; */
static RuleReference rule14655ref1 = 
{
    "cve", /* type */
    "2008-4019" /* value */
};

/* reference: url "technet.microsoft.com/en-us/security/bulletin/MS08-057"; */
static RuleReference rule14655ref2 = 
{
    "url", /* type */
    "technet.microsoft.com/en-us/security/bulletin/MS08-057" /* value */
};

static RuleReference *rule14655refs[] =
{
    &rule14655ref1,
    &rule14655ref2,
    NULL
};
/* metadata for sid 14655 */
/* metadata:service http, policy balanced-ips drop, policy security-ips drop; */
static RuleMetaData rule14655service1 = 
{
    "service http"
};

static RuleMetaData rule14655policy1 = 
{
    "policy max-detect-ips drop"
};

static RuleMetaData *rule14655metadata[] =
{
    &rule14655service1,
    &rule14655policy1,
    NULL
};
RuleOption *rule14655options[] =
{
    &rule14655option0,
    &rule14655option1,
    &rule14655option2,
    &rule14655option3,
    NULL
};

Rule rule14655 = {
   
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
       14655, /* sigid */
       14, /* revision */
   
       "attempted-user", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "FILE-OFFICE Excel rept integer underflow attempt",     /* message */
       rule14655refs /* ptr to references */
       ,rule14655metadata
   },
   rule14655options, /* ptr to rule options */
   &rule14655eval, /* use the built in detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule14655eval(void *p) {
   const uint8_t *cursor_normal = 0;
   const uint8_t *cursor_detect = 0;
   const uint8_t *end_of_payload = 0;
   const uint8_t *function_type_pos = 0;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   uint32_t structure_length = 0;
   uint32_t string_length = 0;
   uint32_t function_type = 0;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;
    
   // flow:established, to_client;
   if(checkFlow(p, rule14655options[0]->option_u.flowFlags) <= 0 )
      return RULE_NOMATCH;

   // flowbits:isset "file.xls";
   if(processFlowbits(p, rule14655options[1]->option_u.flowBit) <= 0)
      return RULE_NOMATCH;

   // content:"A|1E|";
   /* This content match is in the fast pattern matcher so it's guaranteed to be
    * in the packet, but it's not needed for our detection so we're skipping it.
   */
   //if(contentMatch(p, rule14655options[2]->option_u.content, &cursor_normal) <= 0)
   //   return RULE_NOMATCH;

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_payload) <= 0)
      return RULE_NOMATCH;

   // content:"|06 00|";
   while(contentMatch(p, rule14655options[3]->option_u.content, &cursor_normal) > 0) {

     // jump to the byte before structure_length
     // this lines us up to check the function type
     // and the first function argument
     cursor_detect = cursor_normal + 21;

     // check if we can read the structure length (distance 22, len 2)
     // the first function argument id (distance 24, len 1) land on
     // the byte after it.
     if(cursor_detect + 4 > end_of_payload)
       return RULE_NOMATCH;

     // if the first function argument is not a string (0x17) then skip.
     if(*(cursor_detect + 3) != 0x17)
       continue;

     // read the structure_length
     structure_length = read_little_16(cursor_detect + 1);

     // check if we can read the function type
     function_type_pos = cursor_detect + structure_length;
     if(function_type_pos + 2 > end_of_payload)
       return RULE_NOMATCH;

     // buffer overflow check
     if(function_type_pos < cursor_detect)
       return RULE_NOMATCH;
 
     // read the function_type
     function_type = read_little_16(function_type_pos);

     // if the function type at the end of the structure
     // is not 0x1e41 little endian, or REPT(), then skip.
     if(function_type != 0x1e41)
       continue;

     DEBUG_SO(fprintf(stderr,"Got function_type = 0x%4x\n",function_type);)
     
     // We have a REPT function and the first function argument
     // is a string, check if we can read the string length and
     // land on the following byte.
     cursor_detect += 4;
     if(cursor_detect + 3 > end_of_payload)
       return RULE_NOMATCH;

     // increment cursor_detect to point to the first byte
     // in string_length,  store string_length (LE byte order)
     string_length = read_little_16(cursor_detect);
     cursor_detect += 2;

     // move cursor_detect to the second function argument id
     cursor_detect += string_length;

     // check if we can read the second function argument id
     if(cursor_detect + 1 > end_of_payload)
       return RULE_NOMATCH;

     // if the function argument is not a pointer to another cell (0x44) then skip 
     if(*cursor_detect != 0x44)
       continue;

     // set cursor_detect to the position of the first byte in function_type
     cursor_detect += 5;

     // make sure its not beyond the packet payload
     if(cursor_detect + 1 > end_of_payload)
       return RULE_NOMATCH;

     // if function_type_pos equals the sum of
     // walking the entire strutcture, then alert.
     if(cursor_detect == function_type_pos)
       return RULE_MATCH;

     // At this point we're pretty sure we actually processed a record, so skip ahead
     cursor_normal = cursor_detect;
   }

   return RULE_NOMATCH;
}
/*
Rule *rules[] = {
    &rule14655,
    NULL
};
*/
