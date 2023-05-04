/*
 * SMTP Microsoft Exchange Server MIME base64 decoding code execution attempt
 * 
 * Copyright (C) 2007 Sourcefire, Inc. All Rights Reserved
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
int ruleEXCHANGE_BASE64_DECODEeval(void *p);

static RuleReference ruleEXCHANGE_BASE64_DECODEref0 = 
{
    "url", /* type */
    "technet.microsoft.com/en-us/security/bulletin/MS07-026" /* value */
};
static RuleReference ruleEXCHANGE_BASE64_DECODEref1 =
{
    "cve", /* type */
    "2007-0213"
};
static RuleReference ruleEXCHANGE_BASE64_DECODEref2 =
{
    "bugtraq", /* type */
    "23809"
};

static RuleReference *ruleEXCHANGE_BASE64_DECODErefs[] =
{
    &ruleEXCHANGE_BASE64_DECODEref0,
    &ruleEXCHANGE_BASE64_DECODEref1,
    &ruleEXCHANGE_BASE64_DECODEref2,
    NULL
};

static FlowFlags ruleEXCHANGE_BASE64_DECODEflow =
{
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption ruleEXCHANGE_BASE64_DECODEoption0 =
{
    OPTION_TYPE_FLOWFLAGS,
    {
        &ruleEXCHANGE_BASE64_DECODEflow
    }
};

static ContentInfo ruleEXCHANGE_BASE64_DECODEcontent =
{
    (uint8_t *)"Content-Transfer-Encoding", /* pattern to search for */
    0,                      /* depth */
    0,                      /* offset */
    CONTENT_FAST_PATTERN | CONTENT_BUF_NORMALIZED | CONTENT_NOCASE,         /* flags */
    NULL,                   /* holder for boyer/moore info */
    NULL,                   /* holder for byte representation of "NetBus" */
    0,                      /* holder for length of byte representation */
    0                       /* holder of increment length */
};

static RuleOption ruleEXCHANGE_BASE64_DECODEoption1 =
{
    OPTION_TYPE_CONTENT,
    {
        &ruleEXCHANGE_BASE64_DECODEcontent
    }
};

static PCREInfo ruleEXCHANGE_BASE64_DECODEpcre0 =
{
    "^Content-Transfer-Encoding:\\s*base64\\s*$", /* pattern to search for */
    NULL,                                         /* compiled_expr */
    0,                                            /* compiled_extra */
    PCRE_CASELESS | PCRE_MULTILINE,  /* compile_flags */
    CONTENT_BUF_NORMALIZED            /* flags:  must include a CONTENT_BUF_X */
};

static RuleOption ruleEXCHANGE_BASE64_DECODEoption2 =
{
    OPTION_TYPE_PCRE,
    {
        &ruleEXCHANGE_BASE64_DECODEpcre0
    }
};


/* Second PCRE just like above but with CONTENT_RELATIVE so we can find
   additional base64 sections if they exist.
*/
static PCREInfo ruleEXCHANGE_BASE64_DECODEpcre1 =
{
    "^Content-Transfer-Encoding:\\s*base64\\s*$", /* pattern to search for */
    NULL,                                         /* compiled_expr */
    0,                                            /* compiled_extra */
    PCRE_CASELESS | PCRE_MULTILINE,               /* compile_flags */
    CONTENT_BUF_NORMALIZED | CONTENT_RELATIVE            /* flags:  must include a CONTENT_BUF_X */
};

static RuleOption ruleEXCHANGE_BASE64_DECODEoption3 =
{
    OPTION_TYPE_PCRE,
    {
        &ruleEXCHANGE_BASE64_DECODEpcre1
    }
};

RuleOption *ruleEXCHANGE_BASE64_DECODEoptions[] =
{
    &ruleEXCHANGE_BASE64_DECODEoption0,
    &ruleEXCHANGE_BASE64_DECODEoption1,
    &ruleEXCHANGE_BASE64_DECODEoption2,
    &ruleEXCHANGE_BASE64_DECODEoption3,
    NULL
};

static RuleMetaData ruleEXCHANGE_BASE64_DECODEservice1 =
{
    "service smtp"
};

static RuleMetaData ruleEXCHANGE_BASE64_DECODEpolicy1 =
{
    "policy max-detect-ips drop"
};

static RuleMetaData *ruleEXCHANGE_BASE64_DECODEmetadata[] =
{
    &ruleEXCHANGE_BASE64_DECODEservice1,
    &ruleEXCHANGE_BASE64_DECODEpolicy1,
    NULL
};

Rule ruleEXCHANGE_BASE64_DECODE = {
   /* rule header */
   {
       IPPROTO_TCP, /* proto */
       "any", /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       HOME_NET, /* DSTIP     */
       "25", /* DSTPORT   */
   },
   /* metadata */
   { 
       3,  /* genid (HARDCODED!!!) */
       12028, /* sigid 82b97da7-adc8-46ae-a76f-1d5c34c8e8d7 */
       8, /* revision d6953e77-f2f9-4137-b00e-89673e66fac8 */
   
       "attempted-admin", /* classification, generic */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "SERVER-MAIL Microsoft Exchange Server MIME base64 decoding code execution attempt",     /* message */
       ruleEXCHANGE_BASE64_DECODErefs /* ptr to references */
        ,ruleEXCHANGE_BASE64_DECODEmetadata
   },
   ruleEXCHANGE_BASE64_DECODEoptions, /* ptr to rule options */
   &ruleEXCHANGE_BASE64_DECODEeval, /* ptr to rule detection function */
   0, /* am I initialized yet? */
   0, /* number of options */
   0  /* don't alert */
};


/* detection function */

int ruleEXCHANGE_BASE64_DECODEeval(void *p) {
   /* data pointers */
   const uint8_t *cursor_normal, *beg_of_payload, *end_of_payload, *line_ptr, *next_line_ptr;

   /* state variable */
   int in_base64_content, num_short_lines, cr;

   int line_text_len;

   SFSnortPacket *sp = (SFSnortPacket *) p;

   if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   /* call flow match */
   if (checkFlow(sp, ruleEXCHANGE_BASE64_DECODEoptions[0]->option_u.flowFlags) <= 0 )
      return RULE_NOMATCH;

// This is our fast pattern, so we know it's here.  Plus, it's repeated in our PCRE.
// In theory, we could leave the content match then make the pcre relative and it
// could possible eek out a little more speed, but this should be fine.
//   /* call content match */
//   if (contentMatch(sp, ruleEXCHANGE_BASE64_DECODEoptions[1]->option_u.content, &cursor_normal) <= 0) {
//      return RULE_NOMATCH;
//   }

   /* call pcre match */
   if (pcreMatch(sp, ruleEXCHANGE_BASE64_DECODEoptions[2]->option_u.pcre, &cursor_normal) <= 0) {
      return RULE_NOMATCH;
   }
 
   /* At this point, cursor_normal is pointing to just past the base64
    * content-transfer-encoding tag.  So for now, we just need to read the
    * data looking for the malicious lines and stop when we get to the
    * end of the packet or /^--/, which denotes the end of the boundary.
    * 
    * If we don't flag, then we need to step through the rest of the
    * message looking for another base64-encoded C-T-E and look for
    * malicious data before /^--/ or EOF.  Rinse and repeat.
    */

   /* Since we're here, we are in a base64 content boundary.  Set things
    * up so we can use the same code for the rest of the message.
    */

   in_base64_content = 1;
   num_short_lines = 0;

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_payload, &end_of_payload) <= 0)
      return RULE_NOMATCH;

   while(cursor_normal < end_of_payload) {
      if(in_base64_content) {
         /* Find the end of the line and set status variables */
         line_ptr = cursor_normal;
         line_text_len = 0;
         cr = 0;

         while(line_ptr < end_of_payload) {
            if(*line_ptr == '\r') {
               if(cr == 1) {       // We got two CRs in a row
                  line_text_len++; // Add the first to the count and keep going
               }
               cr = 1;
            }  else if(*line_ptr == '\n') {
               break;  // Found end of line, continue processing
            }  else {
               if(cr == 1) {  // This means we have a bare CR, just
                              // add it to the count and keep going
                  line_text_len++;
                  cr = 0;
               }
               line_text_len++;
            }
            line_ptr++;
         }

         // Move pointer after end of this line.  The loop will take care
         // of us stepping off of the payload.
         next_line_ptr = line_ptr + 1;

         // Check if base64 content is ending
         if((line_text_len >= 2) && (cursor_normal[0] == '-' &&
                                     cursor_normal[1] == '-')) {
            in_base64_content = 0;
            num_short_lines = 0;
         }  else {
            if(line_text_len == 1 || line_text_len == 2) {
               num_short_lines++;
               if(num_short_lines == 4) 
                  return RULE_MATCH;
            }  else {
               num_short_lines = 0;
            }
         }

         cursor_normal = next_line_ptr;

      }  else { /* !in_base64_content */
         // Find the next base64 content the easy way
         if(pcreMatch(sp, ruleEXCHANGE_BASE64_DECODEoptions[3]->option_u.pcre, &cursor_normal) <= 0)
            return RULE_NOMATCH;

         // Another base64 section was found, set up for another loop
         in_base64_content = 1;
         num_short_lines = 0;
      }
   } 

   return RULE_NOMATCH;
}      
            

