/*
 * Base64 Decoding Routines
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2007-2013 Sourcefire, Inc. All Rights Reserved
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

#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"
#include "imap_base64_decode.h"

/* Our lookup table for decoding base64 */
unsigned char decode64tab[256] = {
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
 * non-whitespace.  This is especially helpful for HTML.
*/
int unfold_header(u_int8_t *inbuf, u_int32_t inbuf_size, u_int8_t *outbuf,
                  u_int32_t outbuf_size, u_int32_t *output_bytes) {
   u_int8_t *cursor, *endofinbuf;
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
 * data is valid up until the point you care about.  Note base64 data does NOT have to end with
 * '=' and won't if the number of bytes of input data is evenly divisible by 3.
*/
int base64decode(const u_int8_t *inbuf, u_int32_t inbuf_size, u_int8_t *outbuf, u_int32_t outbuf_size, u_int32_t *bytes_written) {
   const u_int8_t *cursor, *endofinbuf;
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
      if(decode64tab[*cursor] != 100) {
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
            tableval_a = decode64tab[base64data[0]];
            tableval_b = decode64tab[base64data[1]];
            tableval_c = decode64tab[base64data[2]];
            tableval_d = decode64tab[base64data[3]];

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

