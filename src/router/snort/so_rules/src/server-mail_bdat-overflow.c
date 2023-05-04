/*
 * Use at your own risk.
 *
 * Copyright (C) 2005-2008 Sourcefire, Inc.
 * 

 * Written by Patrick Mullen <pmullen@sourcefire.com> using rules2c as a base

!!! DOES *NOT* USE BUILT-IN DETECTION FUNCTION !!!
Hell, this "base rule" is now horribly out of date.

alert tcp $EXTERNAL_NET any -> $HOME_NET 25 (msg:"SMTP BDAT buffer overflow attempt"; flow:to_server,established; content:"BDAT"; nocase; pcre:"/^\s*BDAT\s+/smiR"; content:"|0a|"; distance:0; metadata:policy security-ips drop; reference:url,technet.microsoft.com/en-us/security/bulletin/ms02-012; reference:cve,2002-0055; reference:bugtraq,4204; classtype:attempted-admin; sid:13718; rev:1;)

 */

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"

/* declare detection functions */
int rule13718eval(void *p);

/* declare rule data structures */
/* precompile the stuff that needs pre-compiled */
/* flow:established, to_server; */
static FlowFlags rule13718flow0 = 
{
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption rule13718option0 =
{
    OPTION_TYPE_FLOWFLAGS,
    {
        &rule13718flow0
    }
};
// content:"BDAT", nocase; 
static ContentInfo rule13718content1 = 
{
    (uint8_t *) "BDAT", /* pattern (now in snort content format) */
    0, /* depth */
    0, /* offset */
    CONTENT_NOCASE|CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0 /* increment length*/
};

static RuleOption rule13718option1 = 
{
    OPTION_TYPE_CONTENT,
    {
        &rule13718content1
    }
};
// pcre:"^BDAT\s+", dotall, multiline, nocase;
static PCREInfo rule13718pcre2 =
{
    "^BDAT\\s+", /* pattern */
    NULL,                               /* holder for compiled pattern */
    NULL,                               /* holder for compiled pattern flags */
    PCRE_CASELESS|PCRE_DOTALL|PCRE_MULTILINE,     /* compile flags */
    CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED     /* content flags */
};

static RuleOption rule13718option2 =
{
    OPTION_TYPE_PCRE,
    {
        &rule13718pcre2
    }
};
// content:"|0A|", relative; 
static ContentInfo rule13718content3 = 
{
    (uint8_t *) "|0A|", /* pattern (now in snort content format) */
    0, /* depth */
    0, /* offset */
    CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED, /* flags */ // XXX - need to add CONTENT_FAST_PATTERN support
    NULL, /* holder for boyer/moore PTR */
    NULL, /* more holder info - byteform */
    0, /* byteform length */
    0 /* increment length*/
};

static RuleOption rule13718option3 = 
{
    OPTION_TYPE_CONTENT,
    {
        &rule13718content3
    }
};

// pcre:"^(QUIT|RSET|BDAT)", relative, nocase;
static PCREInfo rule13718pcre4 =
{
    "^(QUIT|RSET|BDAT)", /* pattern */
    NULL,                               /* holder for compiled pattern */
    NULL,                               /* holder for compiled pattern flags */
    PCRE_CASELESS,     /* compile flags */
    CONTENT_RELATIVE|CONTENT_BUF_NORMALIZED     /* content flags */
};

static RuleOption rule13718option4 =
{
    OPTION_TYPE_PCRE,
    {
        &rule13718pcre4
    }
};


/* references for sid 13718 */
/* reference: bugtraq "4204"; */
static RuleReference rule13718ref1 = 
{
    "bugtraq", /* type */
    "4204" /* value */
};

/* reference: cve "2002-0055"; */
static RuleReference rule13718ref2 = 
{
    "cve", /* type */
    "2002-0055" /* value */
};

/* reference: url "technet.microsoft.com/en-us/security/bulletin/ms02-012"; */
static RuleReference rule13718ref3 = 
{
    "url", /* type */
    "technet.microsoft.com/en-us/security/bulletin/ms02-012" /* value */
};

static RuleReference *rule13718refs[] =
{
    &rule13718ref1,
    &rule13718ref2,
    &rule13718ref3,
    NULL
};
/* metadata for sid 13718 */
/* metadata:policy security-ips drop; */

static RuleMetaData rule13718policy1 = 
{
    "policy security-ips drop"
};

static RuleMetaData rule13718policy2 =
{
    "policy max-detect-ips drop"
};

static RuleMetaData rule13718service1 =
{
    "service smtp"
};

static RuleMetaData *rule13718metadata[] =
{
    &rule13718policy1,
    &rule13718policy2,
    &rule13718service1,
    NULL
};
RuleOption *rule13718options[] =
{
    &rule13718option0,
    &rule13718option1,
    &rule13718option2,
    &rule13718option3,
    &rule13718option4,
    NULL
};

Rule rule13718 = {
   
   /* rule header, akin to => tcp any any -> any any               */{
       IPPROTO_TCP, /* proto */
       "$EXTERNAL_NET", /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       "$HOME_NET", /* DSTIP     */
       "25", /* DSTPORT   */
   },
   /* metadata */
   { 
       3,  /* genid (HARDCODED!!!) */
       13718, /* sigid */
       6, /* revision */
   
       "attempted-admin", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "SERVER-MAIL BDAT buffer overflow attempt",     /* message */
       rule13718refs /* ptr to references */
       ,rule13718metadata
   },
   rule13718options, /* ptr to rule options */
   &rule13718eval, /* DOES NOT use the built in detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule13718eval(void *p) {
    const uint8_t *cursor_normal = 0;
    SFSnortPacket *sp = (SFSnortPacket *) p;

    const uint8_t *end_of_payload;

    char byte_array[6], *parse_helper;
    unsigned int i, chunklen;

    if(sp == NULL)
        return RULE_NOMATCH;

    if(sp->payload == NULL)
        return RULE_NOMATCH;

    // flow:established, to_server;
    if (checkFlow(p, rule13718options[0]->option_u.flowFlags) <= 0 ) 
       return RULE_NOMATCH;

    if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_payload) <= 0)
       return RULE_NOMATCH;

    // we never check our "BDAT" content match because it's a repeat of our pcre and by
    // definition it exists since we passed the fast pattern matcher.
        
    // pcre:"^\s*BDAT\s+", dotall, multiline, nocase;
    while(pcreMatch(p, rule13718options[2]->option_u.pcre, &cursor_normal) > 0) {

       // extract the size
       if(cursor_normal + 5 >= end_of_payload)
          return RULE_NOMATCH;

       // Cursor should be pointing at the length field if this is a valid chunk 
       for(i=0;i<5; i++)
       {
          byte_array[i] = *(cursor_normal+i);
       }

       byte_array[5] = '\0';

       chunklen = strtoul(byte_array, &parse_helper, 10);

       if(byte_array == parse_helper) /* no valid digits */
          continue;                   /* try to find a valid chunk */

       // verify the size isn't bigger than we can possibly verify (3k bytes total)
       if(chunklen > 2900)
          return RULE_NOMATCH;

       // now look for the end of the line
       // content:"|0A|", relative;
       if(contentMatch(p, rule13718options[3]->option_u.content, &cursor_normal) <= 0)
          return RULE_NOMATCH;

       // now add the size to the cursor and check for next command
       cursor_normal += chunklen;

       // This seems fragile, but our original assumption that the bdat block ends with \r\n
       // does not seem correct.  What does seem correct based upon RFC1830 and live data is
       // that the block is followed by another command.  The sane followers are QUIT, RSET,
       // and another BDAT (for multipart BDAT blocks).

       if(cursor_normal + 4 >= end_of_payload) // verify size to avoid false positives
          return RULE_NOMATCH;

       // pcre:"^(QUIT|RSET|BDAT)", relative, nocase;
       if(pcreMatch(p, rule13718options[4]->option_u.pcre, &cursor_normal) <= 0) 
          return RULE_MATCH;
    }

    return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule13718,
    NULL
};
*/
