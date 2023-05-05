/*
 * squid_ntlm_authentication buffer overflow exploit attempt
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2006-2013 Sourcefire, Inc. All Rights Reserved
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "pcre.h"
#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"

#include <string.h>

/* declare detection functions */
int ruleSQUID_NTLM_AUTHeval(void *);

/* declare references */
static RuleReference ruleSQUID_NTLM_AUTHref0 = {
    "url", /* type */
    "www.idefense.com/application/poi/display?id=107" /* value */
};

static RuleReference ruleSQUID_NTLM_AUTHcve = {
    "cve", /* type */
    "2004-0541"
};

static RuleReference *ruleSQUID_NTLM_AUTHrefs[] = {
    &ruleSQUID_NTLM_AUTHref0,
    &ruleSQUID_NTLM_AUTHcve,
    NULL
};

/* Declare rule options */
static FlowFlags ruleSQUID_NTLM_AUTHflow = {
    FLOW_ESTABLISHED|FLOW_TO_SERVER
};

static RuleOption ruleSQUID_NTLM_AUTHoption0 = {
    OPTION_TYPE_FLOWFLAGS,
    {
        &ruleSQUID_NTLM_AUTHflow
    }
};

static ContentInfo ruleSQUID_NTLM_AUTHcontent =
{
    (u_int8_t *)"Proxy-Authorization:",/* pattern to search for */
    0,                      /* depth */
    0,                      /* offset */
    CONTENT_NOCASE,         /* flags */
    NULL,                   /* holder for boyer/moore info */
    NULL,                   /* holder for byte representation of "NetBus" */
    0,                      /* holder for length of byte representation */
    0,                      /* holder of increment length */
    0,                      /* holder for fp offset */
    0,                      /* holder for fp length */
    0,                      /* holder for fp only */
    NULL, // offset_refId
    NULL, // depth_refId
    NULL, // offset_location
    NULL  // depth_location
};

static RuleOption ruleSQUID_NTLM_AUTHoption1 = {
    OPTION_TYPE_CONTENT,
    {
        &ruleSQUID_NTLM_AUTHcontent
    }
};

static PCREInfo ruleSQUID_NTLM_AUTHpcre =
{
    "^Proxy-Authorization:\\s*NTLM\\s+", /* pattern to search for */
    NULL,                               /* holder for compiled pattern */
    NULL,                               /* holder for compiled pattern flags */
    PCRE_CASELESS | PCRE_DOTALL | PCRE_MULTILINE,     /* compile flags */
    CONTENT_BUF_NORMALIZED,     /* content flags */
    0 /* offset */
};

static RuleOption ruleSQUID_NTLM_AUTHoption2 = {
    OPTION_TYPE_PCRE,
    {
        &ruleSQUID_NTLM_AUTHpcre
    }
};

RuleOption *ruleSQUID_NTLM_AUTHoptions[] = {
    &ruleSQUID_NTLM_AUTHoption0,
    &ruleSQUID_NTLM_AUTHoption1,
    &ruleSQUID_NTLM_AUTHoption2,
    NULL
};


/* Rule definition */

Rule ruleSQUID_NTLM_AUTH = {
   /* rule header */
   {
       IPPROTO_TCP, /* proto */
       "any", /* SRCIP     */
       "any", /* SRCPORT   */
       0, /* DIRECTION */
       HOME_NET, /* DSTIP */
       "3128", /* DSTPORT */
   },
   /* metadata */
   {
       3,  /* genid (HARDCODED!!!) */
       7128, /* XXX sigid */
       1, /* revision  */
       "attempted-user", /* classification */
       0,  /* hardcoded priority XXX NOT PROVIDED BY GRAMMAR YET! */
       "WEB-MISC squid NTLM Authorization buffer overflow exploit attempt",     /* message */
       ruleSQUID_NTLM_AUTHrefs, /* ptr to references */
       NULL /* Meta data */
   },
   ruleSQUID_NTLM_AUTHoptions, /* ptr to rule options */
   &ruleSQUID_NTLM_AUTHeval, /* ptr to rule detection function */
   0, /* am I initialized yet? */
   0, /* number of options */
   0,  /* don't alert */
   NULL /* ptr to internal data... setup during rule registration */
};


/* Detection functions */

/* Our lookup table for decoding base64 */
unsigned char squid_decode64tab[256] = {
        100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
        100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
        100,100,100,100,100,100,100,100,100,100,100,62 ,100,100,100, 63,
         52, 53, 54, 55, 56, 57, 58, 59, 60, 61,100,100,100, 99,100,100,
        100,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
         15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,100,100,100,100,100,
        100, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
         41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,100,100,100,100,100,
        100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
        100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
        100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
        100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
        100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
        100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
        100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,
        100,100,100,100,100,100,100,100,100,100,100,100,100,100,100,100};

/* Given a string, removes header folding (\r\n followed by linear whitespace)
 * and exits when the end of a header is found, defined as \n followed by a
 * non-whitespace.
*/
static int unfold_header(
   const u_int8_t *inbuf, u_int32_t inbuf_size, u_int8_t *outbuf,
   u_int32_t outbuf_size, u_int32_t *output_bytes
) {
   const u_int8_t *cursor, *endofinbuf;
   u_int8_t *outbuf_ptr;

   u_int32_t n = 0;

   int httpheaderfolding = 0;

   cursor = inbuf;
   endofinbuf = inbuf + inbuf_size;
   outbuf_ptr = outbuf;

   /* Keep adding chars until we get to the end of the line.  If we get to the
      end of the line and the next line starts with a tab or space, add the space
      to the buffer and keep reading.  If the next line does not start with a
      tab or space, stop reading because that's the end of the header. */
   while((cursor < endofinbuf) && (n < outbuf_size)) {
      if(((*cursor == ' ') || (*cursor == '\t')) && (httpheaderfolding != 2)) {
         /* Spaces are valid except after CRs */
         *outbuf_ptr++ = *cursor;
         httpheaderfolding = 0;
      }  else if((*cursor == '\n') && (httpheaderfolding != 1)) {
         /* Can't have multiple LFs in a row, but if we get one it
            needs to be followed by at least one space */
         httpheaderfolding = 1;
      }  else if((*cursor == '\r') && !httpheaderfolding) {
         /* CR needs to be followed by LF and can't start a line */
         httpheaderfolding = 2;
      }  else if(!httpheaderfolding) {
         *outbuf_ptr++ = *cursor;
         n++;
      }  else {
         /* We have reached the end of the header */
         /* Unless we get multiple CRs, which is suspicious, but not for us to decide */
         break;
      }
      cursor++;
   }

   *output_bytes = outbuf_ptr - outbuf;
   return(0);
}


/* base64decode assumes the input data terminates with '=' and/or at the end of the input buffer
 * at inbuf_size.  If extra characters exist within inbuf before inbuf_size is reached, it will
 * happily decode what it can and skip over what it can't.  This is consistent with other decoders
 * out there.  So, either terminate the string, set inbuf_size correctly, or at least be sure the
 * data is valid up until the point you care about.
*/
int squid_base64decode(u_int8_t *inbuf, u_int32_t inbuf_size, u_int8_t *outbuf, u_int32_t outbuf_size, u_int32_t *bytes_written) {
   u_int8_t *cursor, *endofinbuf;
   u_int8_t *outbuf_ptr;
   u_int8_t base64data[4], *base64data_ptr; /* temporary holder for current base64 chunk */
   u_int8_t tableval_a, tableval_b, tableval_c, tableval_d;

   u_int32_t n;
   u_int32_t max_base64_chars;  /* The max number of decoded base64 chars that fit into outbuf */

   int error = 0;

   /* This algorithm will waste up to 4 bytes but we really don't care.
      At the end we're going to copy the exact number of bytes requested. */
   max_base64_chars = (outbuf_size / 3) * 4 + 4; /* 4 base64 bytes gives 3 data bytes, plus
                                                    an extra 4 to take care of any rounding */

   base64data_ptr = base64data;
   endofinbuf = inbuf + inbuf_size;

   /* Strip non-base64 chars from inbuf and decode */
   n = 0;
   *bytes_written = 0;
   cursor = inbuf;
   outbuf_ptr = outbuf;
   while((cursor < endofinbuf) && (n < max_base64_chars)) {
      if(squid_decode64tab[*cursor] != 100) {
         *base64data_ptr++ = *cursor;
         n++;  /* Number of base64 bytes we've stored */
         if(!(n % 4)) {
            /* We have four databytes upon which to operate */

            if((base64data[0] == '=') || (base64data[1] == '=')) {
               /* Error in input data */
               error = 1;
               break;
            }

            /* retrieve values from lookup table */
            tableval_a = squid_decode64tab[base64data[0]];
            tableval_b = squid_decode64tab[base64data[1]];
            tableval_c = squid_decode64tab[base64data[2]];
            tableval_d = squid_decode64tab[base64data[3]];

            if(*bytes_written < outbuf_size) {
               *outbuf_ptr++ = (tableval_a << 2) | (tableval_b >> 4);
               (*bytes_written)++;
            }

            if((base64data[2] != '=') && (*bytes_written < outbuf_size)) {
               *outbuf_ptr++ = (tableval_b << 4) | (tableval_c >> 2);
               (*bytes_written)++;
            }  else {
               break;
            }

            if((base64data[3] != '=') && (*bytes_written < outbuf_size)) {
               *outbuf_ptr++ = (tableval_c << 6) | tableval_d;
               (*bytes_written)++;
            }  else {
               break;
            }

            /* Reset our decode pointer for the next group of four */
            base64data_ptr = base64data;
         }
      }
      cursor++;
   }

   if(error)
      return(-1);
   else
      return(0);
}



int ruleSQUID_NTLM_AUTHeval(void *p) {
   SFSnortPacket *sp = (SFSnortPacket *)p;

   const u_int8_t *cursor;

   int n;  /* cruft */

   /* NTLMSSP AUTH header, before size information (after base64 decode) */
   /* ntlmssp_auth_base64_header is the binary decode of "TlRMTVNTUAADAAAA" */
   unsigned char ntlmssp_auth_base64_header[] = "\x4e\x54\x4c\x4d\x53\x53\x50\x00\x03\x00\x00\x00";

   /* Data for holding our base64 data */
   unsigned char base64data[8192], decoded_data[16];
   u_int32_t remaining_bytes, output_bytes, num_bytes_extracted;

   /* Fields we are retrieving from the NTLMSSP_AUTH data */
   unsigned int password_len, password_max_len;

   /* General sanity checking */
  if(sp == NULL)
      return RULE_NOMATCH;

   if(sp->payload == NULL)
      return RULE_NOMATCH;

   /* call flow match */
   if (checkFlow(sp, ruleSQUID_NTLM_AUTHoptions[0]->option_u.flowFlags) <= 0 )
      return RULE_NOMATCH;

   /* call content match */
   if (contentMatch(sp, ruleSQUID_NTLM_AUTHoptions[1]->option_u.content, &cursor) <= 0) {
      return RULE_NOMATCH;
   }

   /* call pcre match */
   if (pcreMatch(p, ruleSQUID_NTLM_AUTHoptions[2]->option_u.pcre, &cursor) <= 0) {
      return RULE_NOMATCH;
   }

   remaining_bytes = sp->payload_size - (cursor - sp->payload);
   /* unfold the header */
   unfold_header(cursor, remaining_bytes, base64data, sizeof(base64data), &output_bytes);

   /* Decode the base64 data */
   n = squid_base64decode(base64data, output_bytes, decoded_data, sizeof(decoded_data), &num_bytes_extracted);

   if((n < 0) || (num_bytes_extracted < 16))
      return RULE_NOMATCH;

   /* Compare the static portion of the buffer. memcmp != 0 if they are different */
   if(memcmp(decoded_data, ntlmssp_auth_base64_header, 12))
      return RULE_NOMATCH;

   /* Read the password length values, stored in the data as little-endian. */
   password_len = decoded_data[12];
   password_len += decoded_data[13] << 8;
   password_max_len = decoded_data[14];
   password_max_len += decoded_data[15] << 8;

   /* Both NTLMv1 and NTLMv2 use 24 bytes in their
      challenge response (http://en.wikipedia.org/wiki/NTLM).
      Plus, that's the size of the buffer in squid's NTLM
      module (25 bytes = 24 bytes data + NUL) */
   if((password_len > 24) || (password_max_len > 24))
      return RULE_MATCH;

   return RULE_NOMATCH;
}


#if 0
Rule *rules[] = {
   &ruleSQUID_NTLM_AUTH,
   NULL
};
#endif
