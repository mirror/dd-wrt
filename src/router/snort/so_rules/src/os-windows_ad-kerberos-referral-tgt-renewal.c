/* DOES NOT USE BUILT-IN DETECTION FUNCTION! XXX
 * alert tcp $EXTERNAL_NET any -> $HOME_NET 88 (msg:"DOS Active Directory Kerberos referral TGT renewal DoS attempt"; flow:to_server,established; content:"|a1 03 02 01 05 a2 03 02 01 0c|"; depth:22; content:"|a0 07 03 05 00 00 00 00 02|"; distance:0; within:9; fast_pattern; metadata: service kerberos; reference:url,technet.microsoft.com/en-us/security/bulletin/MS10-014; reference:cve,2010-0035; classtype:attempted-dos; sid:16394;)
*/
/* copyright sourcefire 2010.  written by patrick mullen <pmullen@sourcefire.com>
*/
#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"

#include "so-util_ber.h"
#include <stdio.h>
#include <string.h>

//#define DEBUG
#ifdef DEBUG
#define DEBUG_SO(code) code
#else
#define DEBUG_SO(code)
#endif

/* declare detection functions */
int rule16394eval(void *p);

/* declare rule data structures */
/* flow:established, to_server; */
static FlowFlags rule16394flow0 = 
{
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule16394option0 =
{
    OPTION_TYPE_FLOWFLAGS,
    {
        &rule16394flow0
    }
};
// content:"|A1 03 02 01 05 A2 03 02 01 0C|", payload raw, depth 22; 
static ContentInfo rule16394content1 = 
{
    (uint8_t *) "|A1 03 02 01 05 A2 03 02 01 0C|", /* pattern (now in snort content format) */
    22, /* depth */
    0, /* offset */
    CONTENT_BUF_NORMALIZED, /* flags */
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0 /* increment length*/
};

static RuleOption rule16394option1 = 
{
    OPTION_TYPE_CONTENT,
    {
        &rule16394content1
    }
};
#ifndef CONTENT_FAST_PATTERN
#define CONTENT_FAST_PATTERN 0
#endif
// content:"|A0 07 03 05 00 00 00 00 02|", payload raw, depth 9, relative, fast_pattern; 
static ContentInfo rule16394content2 = 
{
    (uint8_t *) "|A0 07 03 05 00 00 00 00 02|", /* pattern (now in snort content format) */
    9, /* depth */
    0, /* offset */
    CONTENT_RELATIVE|CONTENT_FAST_PATTERN|CONTENT_BUF_NORMALIZED, /* flags */
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0 /* increment length*/
};

static RuleOption rule16394option2 = 
{
    OPTION_TYPE_CONTENT,
    {
        &rule16394content2
    }
};

/* references for sid 16394 */
/* reference: cve "2010-0035"; */
static RuleReference rule16394ref1 = 
{
    "cve", /* type */
    "2010-0035" /* value */
};

/* reference: url "technet.microsoft.com/en-us/security/bulletin/MS10-014"; */
static RuleReference rule16394ref2 = 
{
    "url", /* type */
    "technet.microsoft.com/en-us/security/bulletin/MS10-014" /* value */
};

static RuleReference *rule16394refs[] =
{
    &rule16394ref1,
    &rule16394ref2,
    NULL
};

/* metadata for sid 16394 */
/* metadata:service kerberos; */
static RuleMetaData rule16394service1 = 
{
    "service kerberos"
};

static RuleMetaData *rule16394metadata[] =
{
    &rule16394service1,
    NULL
};

RuleOption *rule16394options[] =
{
    &rule16394option0,
    &rule16394option1,
    &rule16394option2,
    NULL
};

Rule rule16394 = {
   /* rule header, akin to => tcp any any -> any any */
   {
       IPPROTO_TCP, /* proto */
       "$EXTERNAL_NET", /* SRCIP     */
       "any", /* SRCPORT   */
   
       0, /* DIRECTION */
       "$HOME_NET", /* DSTIP     */
   
       "88", /* DSTPORT   */
   },
   /* metadata */
   { 
       3,  /* genid */
       16394, /* sigid */
       5, /* revision */
       "attempted-dos", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "OS-WINDOWS Active Directory Kerberos referral TGT renewal DoS attempt",     /* message */
       rule16394refs /* ptr to references */
       ,rule16394metadata
   },
   rule16394options, /* ptr to rule options */
   &rule16394eval, /* replace with NULL to use the built in detection function */
   0 /* am I initialized yet? */
};

/* detection functions */
int rule16394eval(void *p) {
   const uint8_t *cursor_normal = 0;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   const uint8_t *cursor_padata;

   BER_ELEMENT ber_element;
   int retval;

   uint32_t renew_realm_len;
   const uint8_t *renew_realm_str;

   uint32_t ticket_realm_len;
   const uint8_t *ticket_realm_str;

   DEBUG_SO(int i);

   DEBUG_SO(printf("rule16394eval enter\n"));

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;
   
   // flow:established, to_server;
   if(checkFlow(p, rule16394options[0]->option_u.flowFlags) <= 0)
      return RULE_NOMATCH;

   // For speed and for ease of programming, we take advantage of some content matches
   // here in some places at the expense of false negatives.
   // But they do also to give us a nice content match.

   // This content match skips over a few items then matches on Kerberos protocol
   // version (pvno) 5 and mesg type TGS-REQ (12)
   // content:"|A1 03 02 01 05 A2 03 02 01 0C|", payload raw, depth 22, fast_pattern;
   if(contentMatch(p, rule16394options[1]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   // Save the pointer so we can come back here, and we're going to jump ahead to make
   // sure this is a renew packet as well as store the pointer to and size of the
   // renewal realm
   cursor_padata = cursor_normal;

   BER_SKIP(0xa3); // This is a wrapper to ber_skip_element() that NOMATCH's on failure

   // We should now be at the start of the KDC_REQ_BODY

   BER_DATA(0xa4); // This is a wrapper to ber_point_to_data() that NOMATCH's on failure
   BER_DATA(0x30); 

   // We're going to cheat again here and do a quick content match.
   // Plus, this is our fast pattern match.
   // content:"|A0 07 03 05 00 00 00 00 02|", payload raw, depth 9, relative, fast_pattern;
   if(contentMatch(p, rule16394options[2]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   BER_DATA(0xa2);
 
   // Grab the piece of data we've been looking for
   retval = ber_get_element(sp, cursor_normal, &ber_element);

   // Type 0x1b is essentially a string.  Don't know why it's not type 0x04. 
   // Negative return value means error.  0 means 0 data bytes and therefore useless.
   // Also make sure there is the full amount of data present.
   if((retval <= 0) || (ber_element.type != 0x1b) || (retval < ber_element.data_len))
      return RULE_NOMATCH;

   renew_realm_len = ber_element.data_len;
   renew_realm_str = ber_element.data.data_ptr;

   DEBUG_SO(for(i=0; i<renew_realm_len; i++) printf("%c", renew_realm_str[i]); printf("\n"));

   // Now that we have our renew_realm info and we've verified the data is complete,
   // Let's go back to the beginning and get our ticket_realm information
   cursor_normal = cursor_padata;

   // This is a very long list.  Glad we made sure we were in a renewal packet and
   // had all of the data we need before we bother with this.  :)
   BER_DATA(0xa3);
   BER_DATA(0x30);
   BER_DATA(0x30); 
   BER_SKIP(0xa1);
   BER_DATA(0xa2);
   BER_DATA(0x04);
   BER_DATA(0x6e);
   BER_DATA(0x30);
   BER_SKIP(0xa0);
   BER_SKIP(0xa1);
   BER_SKIP(0xa2);
   BER_DATA(0xa3);
   BER_DATA(0x61);
   BER_DATA(0x30);
   BER_SKIP(0xa0);
   BER_DATA(0xa1);

   // Same code and checks as above for our ticket/home realm
   retval = ber_get_element(sp, cursor_normal, &ber_element);
   if((retval <= 0) || (ber_element.type != 0x1b) || (retval < ber_element.data_len))
      return RULE_NOMATCH;

   ticket_realm_len = ber_element.data_len;
   ticket_realm_str = ber_element.data.data_ptr;

   DEBUG_SO(for(i=0; i<ticket_realm_len; i++) printf("%c", ticket_realm_str[i]); printf("\n"));

   // Match if the realm names are different (either lens are diff or value is diff)
   if(ticket_realm_len != renew_realm_len || memcmp(ticket_realm_str, renew_realm_str, ticket_realm_len))
      return RULE_MATCH;

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule16394,
    NULL
};
*/

