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

//#define DEBUG
#ifdef DEBUG
#define DEBUG_SO(code) code
#else
#define DEBUG_SO(code)
#endif

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"
#include "so-util.h"

/* declare detection functions */
int rule33053eval(void *p);

/* declare rule data structures */
// content:"|01|", depth 1, fast_pattern; 
static ContentInfo rule33053content0 = 
{
   (uint8_t *) "|01|", /* pattern */
   1, /* depth */
   0, /* offset */
   CONTENT_FAST_PATTERN|CONTENT_BUF_NORMALIZED, /* flags */
   NULL, /* holder for boyer/moore PTR */
   NULL, /* more holder info - byteform */
   0, /* byteform length */
   0 /* increment length*/
};

static RuleOption rule33053option0 = 
{
   OPTION_TYPE_CONTENT,
   {
      &rule33053content0
   }
};

/* references for sid 33053 */
/* reference: cve "2015-0015"; */
static RuleReference rule33053ref1 = 
{
   "cve", /* type */
   "2015-0015" /* value */
};

/* reference: url "technet.microsoft.com/en-us/security/bulletin/MS15-007"; */
static RuleReference rule33053ref2 = 
{
   "url", /* type */
   "technet.microsoft.com/en-us/security/bulletin/MS15-007" /* value */
};

/* reference: cve "2016-0050"; */
static RuleReference rule33053ref3 = 
{
   "cve", /* type */
   "2016-0050" /* value */
};

/* reference: url "technet.microsoft.com/en-us/security/bulletin/MS16-021"; */
static RuleReference rule33053ref4 = 
{
   "url", /* type */
   "technet.microsoft.com/en-us/security/bulletin/MS16-021" /* value */
};

static RuleReference *rule33053refs[] =
{
   &rule33053ref1,
   &rule33053ref2,
   &rule33053ref3,
   &rule33053ref4,
   NULL
};

/* metadata for sid 33053 */
/* metadata:service radius, policy security-ips alert; */
static RuleMetaData rule33053service1 = 
{
   "service radius"
};

static RuleMetaData rule33053policy1 = 
{
   "policy security-ips drop"
};

static RuleMetaData rule33053policy2 =
{
   "policy max-detect-ips drop"
};

static RuleMetaData *rule33053metadata[] =
{
   &rule33053service1,
   &rule33053policy1,
   &rule33053policy2,
   NULL
};

RuleOption *rule33053options[] =
{
   &rule33053option0,
   NULL
};

Rule rule33053 = {
   /* rule header, akin to => tcp any any -> any any */
   {
      IPPROTO_UDP, /* proto */
      "any", /* SRCIP     */
      "any", /* SRCPORT   */
      0, /* DIRECTION */
      "$HOME_NET", /* DSTIP     */
      "[1812,1645]", /* DSTPORT   */
   },
   /* metadata */
   { 
      3,  /* genid */
      33053, /* sigid */
      5, /* revision */
      "attempted-dos", /* classification */
      0,  /* hardcoded priority */
      "OS-WINDOWS Microsoft RADIUS Server invalid access-request username denial of service attempt",     /* message */
      rule33053refs, /* ptr to references */
      rule33053metadata /* ptr to metadata */
   },
   rule33053options, /* ptr to rule options */
   &rule33053eval, /* uncomment to use custom detection function */
   0 /* am I initialized yet? */
};


/* detection functions */
int rule33053eval(void *p) {
   const uint8_t *check, *end_of_buffer, *cursor_normal = 0;
   SFSnortPacket *sp = (SFSnortPacket *) p;

   int i, j, bad_char_seen = 0;
   uint8_t atype, alength;
   uint16_t request_length;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;
   
   // content:"|01|", depth 1, fast_pattern;
   if(contentMatch(p, rule33053options[0]->option_u.content, &cursor_normal) <= 0)
      return RULE_NOMATCH;

   if(getBuffer(p, CONTENT_BUF_NORMALIZED, &cursor_normal, &end_of_buffer) <= 0)
      return RULE_NOMATCH;

   // RFC 2865 Page 17
   // code (1 byte), pkt identifier (1 byte)
   // length (2 bytes), authenticator (16 bytes)

   // check if we can read the request length
   if(cursor_normal + 4 > end_of_buffer)
      return RULE_NOMATCH;

   // read the request length
   request_length = read_big_16(cursor_normal + 2);

   // jump the request length and check that we land on end_of_buffer
   if((cursor_normal + request_length) != end_of_buffer)
      return RULE_NOMATCH;

   // skip code (1 byte), pkt identifier (1 byte),
   // length (2 bytes), authenticator (16 bytes)
   cursor_normal += 20;

   // parse up to 10 attributes (TLV)
   for(i = 0; i < 10; i++)
   {
      // make sure we can read type and length (1 byte each)
      if(cursor_normal + 2 > end_of_buffer)
         return RULE_NOMATCH;

      atype = *cursor_normal;
      alength = *(cursor_normal+1);

      DEBUG_SO(fprintf(stderr,"radius attribute type:0x%02X len:0x%02X\n",atype,alength);)

      // make sure we can read value 
      check = cursor_normal + alength;

      // overflow check
      if(check <= cursor_normal)
         return RULE_NOMATCH;

      // overread check
      if(check > end_of_buffer)
         return RULE_NOMATCH;

      if(atype == 0x01)
      {
         // restrict how many bytes we will check
         // in the User-Name Attribute Value
         if(alength > 25)
            alength = 25;

         // User-Name Attribute, check for bad chars, if 2 are present, alert.
         // we start at index 2 because alength includes the Type and Length
         for(j = 2; j < alength; j++)
         {
            switch(cursor_normal[j]) {
            case 0:
               // CVE-2016-0050
               // null byte in the value, alert.
               return RULE_MATCH;
            case '(':
            case ')':
            case '*':
            case '/':
            case '\\':
               // CVE-2015-0015
               // if we have seen 2 bad chars, alert.
               if(bad_char_seen)
                  return RULE_MATCH;

               bad_char_seen = 1;
               break;
            default:
               break;
            }
         }

         // only check one User-Name attribute
         return RULE_NOMATCH;
      }

      cursor_normal = check;      
   }

   return RULE_NOMATCH;
}

/*
Rule *rules[] = {
    &rule33053,
    NULL
};
*/
