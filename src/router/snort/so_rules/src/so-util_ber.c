/*
 * BER support functions
 * 
 * Copyright (C) 2007 Sourcefire, Inc. All Rights Reserved
 * 
 * Written by Patrick Mullen <pmullen@sourcefire.com> 
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

#include "so-util_ber.h"

//#define DEBUG
#ifdef DEBUG
#define DEBUG_SO(code) code
#else
#define DEBUG_SO(code)
#endif

/* Simple macro used to shift a value A bits to the left */
#ifndef PM_EXP2
#define PM_EXP2(A) 1 << A
#endif


/* ber_get_size
 
   Interprets the width specifier and returns the value and width
   of the size

   Returns BER_FAIL if not enough payload to read size, BER_PARTIAL_DATA if the size
   doesn't fit in a uint32_t, and 0 on success.
*/
int ber_get_size(SFSnortPacket *sp, const uint8_t *cursor, uint32_t *total_len, uint32_t *size) {
   const uint8_t *end_of_payload, *beg_of_payload; 
   uint32_t size_len;
   int retval;

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_payload, &end_of_payload) <= 0)
      return BER_FAIL;

   *total_len = 0;
   *size = 0;

   if((cursor >= end_of_payload) || (cursor < beg_of_payload))
      return BER_FAIL;

   if(*cursor & 0x80) {
      size_len = *cursor & 0x7F;
      *total_len = size_len + 1;
      cursor++;

      if(cursor + size_len >= end_of_payload)
         return BER_FAIL;

      retval = ber_get_int(cursor, size_len, size);

      if(retval < 0)
         return BER_PARTIAL_DATA; /* size doesn't fit in uint32_t */

   } else {
      *size = *cursor;
      *total_len = 1;
   }

   return BER_SUCCESS;
}


/* ber_get_int

   Returns the uint32_t value contained at the pointer after skipping
   preceeding NULs.  Returns an error if the data does not fit into
   a uint32_t.
*/
int ber_get_int(const uint8_t *data, uint32_t data_len, uint32_t *retvalue) {
   uint32_t i;      
   *retvalue = 0;

   /* Jump over NULLs */
   i = 0;
   while((i < data_len) && (data[i] == 0)) {
      i++;
   }
   if(data_len - i > 4) return BER_FAIL; /* Data doesn't fit into uint32_t */

   /* Now find the actual value */
   for(;i<data_len;i++) {
      *retvalue += data[i] * PM_EXP2(8*(data_len - i - 1));
   }

   return BER_SUCCESS;
}


/* ber_get_element

   Fills a BER_ELEMENT structure with ber element data.
   Calling function must verify there is enough payload data before
   using ber_element->data.data_ptr.  
   
   Return values:
    >=0 -- Return value is the number of data bytes available (note
           this may be less than the given size if the payload is not
	   big enough for all of the data)
     <0 -- Incomplete record. BER_FAIL means nothing is good.  BER_PARTIAL_DATA means the
           type is good but the data size is > 0xFFFFFFFF bytes long.

*/
int ber_get_element(SFSnortPacket *sp, const uint8_t *cursor, BER_ELEMENT *ber_element) {

   const uint8_t *end_of_payload, *beg_of_payload;

   uint32_t type = 0; // Temporary type holder
   uint32_t bits_stored = 0; // for multibyte type encoding

   int ret_val = 0;

   uint32_t size_len;

   /* temp storage for return values */
   uint32_t data_len;

   if(getBuffer(sp, CONTENT_BUF_NORMALIZED, &beg_of_payload, &end_of_payload) <= 0)
      return BER_FAIL;

   /* minimum size is [type][size][data] such as "|02 01 01|" */
   if((cursor + 3 >= end_of_payload) || (cursor < beg_of_payload))
      return BER_FAIL;

   // This is the usual case, so we'll do this initially   
   ber_element->type = *cursor;

   if((*cursor & 0x1F) == 0x1F) { // Multibyte encoding of type
      // Technically, bits 7-6 signify if type is application defined
      // and bit 5 signifies if it's constructed or primitive, but we're ignoring them

      cursor++;

      // MSB = 1 means more bytes follow, MSB = 0 means last byte
      while(cursor < end_of_payload && (*cursor & 0x80)) {
         if(bits_stored > 0) {
            type <<= 7; // Shift our current value up (remember msb is a flag, so only shift 7)
            bits_stored += 7;
            type |= *cursor & 0x7F;
         } else {
            if(*cursor & 0x7F) { // Ensure we aren't just zero padding here
               type = *cursor & 0x7F;
               bits_stored = 7;
            }
         }
         cursor++;
      }

      if(cursor < end_of_payload) {
         type <<= 7;
         bits_stored += 7;
         type |= *cursor++; // No need for the mask because MSB is 0 
      }

      DEBUG_SO(fprintf(stderr, "type=0x%08x, bits_stored=%d\n", type, bits_stored);)
      if(bits_stored > 32) {
         return BER_FAIL;
      }

      ber_element->type = type;

   } else {
      cursor++;
   }

   /* Find the length of the data */
   ret_val = ber_get_size(sp, cursor, &size_len, &data_len);

   /* ret_val < 0 is a fatal error.  However, if ret_val is BER_PARTIAL_DATA
      it means that the size is > 0xFFFFFFFF.  We want to note
      this occurrence as it can be useful in overflow detection
      such as CVE-2007-1739.  Just be sure to not use any of
      the ber_element values aside from type.  :)
   */
   if(ret_val < 0)
      return(ret_val);

   // Some pieces of detection need the given total size, which could
   // result in integer overflow (data overreads handled later, but this
   // helps with that, too).
   ber_element->specified_total_len = 1 + size_len + data_len;
   ber_element->total_len = ber_element->specified_total_len; // fixed below if necessary

   ber_element->data_len = data_len;
   ber_element->data.data_ptr = cursor + size_len;

   /* Return the number of data bytes available */
   /* Fixed for detection of integer overflow via data_len */
   if((cursor + size_len + data_len >= end_of_payload) || 
                  (1 + size_len + data_len <= size_len)) {
      // type byte + size len + amount of bytes available (parens for clarity)
      ber_element->total_len = 1 + size_len + (end_of_payload - ber_element->data.data_ptr);

      // Kludge to get around int overflow from data_len and still give
      // something useful back.
      if(ber_element->specified_total_len < 1 + size_len)
         ber_element->specified_total_len = data_len;

      return(end_of_payload - ber_element->data.data_ptr);
   } else {
      return(data_len);
   }
}
     

/* ber_extract_int_val

   Fills a BER_ELEMENT structure's int_val.
   Calling function must verify there is enough payload data before
   calling this function.  It is assumed that data_ptr has enough
   data to satisfy the value of data_len.

   Return values:
      0 -- Success
     <0 -- Error.  Probably either data_type isn't 0x02 or the data
           doesn't fit into a uint32_t.
*/
int ber_extract_int_val(BER_ELEMENT *ber_element) {

   if(ber_element->type != 0x02)
      return BER_PARTIAL_DATA;

   return(ber_get_int(ber_element->data.data_ptr,
                      ber_element->data_len,
                      &(ber_element->data.int_val)));
}


/* ber_skip_element

   If the element is the specified type, sets the pointer in the
   parameter list to the byte after the element.

   Return values:
      0 -- Success
     <0 -- Error. The read failed, the type is incorrect, or not the
           entire element is available in the packet.
*/
int ber_skip_element(SFSnortPacket *sp, const uint8_t **cursor, uint32_t type) {
   int retval;   
   BER_ELEMENT ber_element;

   retval = ber_get_element(sp, *cursor, &ber_element);

   if((retval < 0) || (ber_element.type != type) || (retval != ber_element.data_len)) {
      return BER_FAIL;
   } else {
      (*cursor) += ber_element.total_len;
      return BER_SUCCESS;
   }
}


/* ber_point_to_data

   If the element is the specified type, sets the pointer in
   the parameter list to the first byte of data.  Note no
   checks are made to ensure the pointer is still within the
   packet.  DO NOT USE THE UPDATED CURSOR W/OUT VERIFICATION.

   Return values:
      0 -- Success
     <0 -- Error.  The read failed or the type is incorrect.
*/
int ber_point_to_data(SFSnortPacket *sp, const uint8_t **cursor, uint32_t type) {
   int retval;
   BER_ELEMENT ber_element;

   retval = ber_get_element(sp, *cursor, &ber_element);

   if(retval < 0 || ber_element.type != type) {
      return BER_FAIL;
   } else {
      (*cursor) = ber_element.data.data_ptr;
      return BER_SUCCESS;
   }
}

/* ber_extract_int

   Extracts the current element.  If it's an integer, it
   populates the data.int_val element.

   Upon success, moves the cursor to the byte after the current element
   Return values:
      0 -- Success (ber_element is an integer with data.int_val set
     <0 -- Error (not same value, not enough data, not uint32_t, ...
*/
int ber_extract_int(SFSnortPacket *sp, const uint8_t **cursor, BER_ELEMENT *ber_element) {
   int retval;

   DEBUG_SO(fprintf(stderr, "ber_extract_int enter\n");)

   retval = ber_get_element(sp, *cursor, ber_element);
   DEBUG_SO(fprintf(stderr, "type=0x%02x retval=%d, data_len=0x%08x(%d)\n", ber_element->type, retval, ber_element->data_len, ber_element->data_len);)
   if(retval < 0 || retval != ber_element->data_len) // The type is verified in ber_extract_int_val()
      return BER_FAIL;

   retval = ber_extract_int_val(ber_element);
   DEBUG_SO(fprintf(stderr,"retval=%d, data.int_val=0x%08x(%d)\n", retval, ber_element->data.int_val, ber_element->data.int_val);)
   if(retval < 0)
      return BER_FAIL;

   (*cursor) += ber_element->total_len;

   return BER_SUCCESS;
}
