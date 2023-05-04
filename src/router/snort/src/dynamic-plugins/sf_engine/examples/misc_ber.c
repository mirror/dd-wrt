/*
 * BER support functions
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

#include "misc_ber.h"

/* Simple macro used to shift a value A bits to the left */
#ifndef PM_EXP2
#define PM_EXP2(A) 1 << A
#endif


/* ber_get_size

   Interprets the width specifier and returns the value and width
   of the size

   Returns -1 if not enough payload to read size, -2 if the size
   doesn't fit in a u_int32_t, and 0 on success.
*/
int ber_get_size(SFSnortPacket *sp, const u_int8_t *cursor, u_int32_t *total_len, u_int32_t *size) {
   const u_int8_t *end_of_payload;
   u_int32_t size_len;
   int retval;

   end_of_payload = sp->payload + sp->payload_size;
   *total_len = 0;
   *size = 0;

   if(*cursor & 0x80) {
      size_len = *cursor & 0x0F;
      *total_len = size_len + 1;
      cursor++;

      if(cursor + size_len >= end_of_payload)
         return(-1);

      retval = ber_get_int(cursor, size_len, size);

      if(retval < 0)
         return(-2); /* size doesn't fit in u_int32_t */

   } else {
      *size = *cursor;
      *total_len = 1;
   }

   return(0);
}


/* ber_get_int

   Returns the u_int32_t value contained at the pointer after skipping
   preceeding NULs.  Returns an error if the data does not fit into
   a u_int32_t.
*/
int ber_get_int(const u_int8_t *data, u_int32_t data_len, u_int32_t *retvalue) {
   u_int32_t i;
   *retvalue = 0;

   /* Jump over NULLs */
   i = 0;
   while((i < data_len) && (data[i] == 0)) {
      i++;
   }
   if(data_len - i > 4) return(-1); /* Data doesn't fit into u_int32_t */

   /* Now find the actual value */
   for(;i<data_len;i++) {
      *retvalue += data[i] * PM_EXP2(8*(data_len - i - 1));
   }

   return(0);
}


/* ber_get_element

   Fills a BER_ELEMENT structure with ber element data.
   Calling function must verify there is enough payload data before
   using ber_element->data.data_ptr.

   Return values:
    >=0 -- Return value is the number of data bytes available (note
           this may be less than the given size if the payload is not
	   big enough for all of the data)
     <0 -- Incomplete record. -1 means nothing is good.  -2 means the
           type is good but the data size is > 0xFFFFFFFF bytes long.

*/
int ber_get_element(SFSnortPacket *sp, const u_int8_t *cursor, BER_ELEMENT *ber_element) {

   const u_int8_t *end_of_payload;

   int ret_val = 0;

   u_int32_t size_len;

   /* temp storage for return values */
   u_int32_t data_len;

   end_of_payload = sp->payload + sp->payload_size;

   /* minimum size is [type][size][data] such as "|02 01 01|" */
   if(cursor + 3 >= end_of_payload)
      return(-1);

   ber_element->type = *cursor;
   cursor++;

   /* Find the length of the data */
   ret_val = ber_get_size(sp, cursor, &size_len, &data_len);

   /* ret_val < 0 is a fatal error.  However, if ret_val is -2
      it means that the size is > 0xFFFFFFFF.  We want to note
      this occurrence as it can be useful in overflow detection
      such as CVE-2007-1739.  Just be sure to not use any of
      the ber_element values aside from type.  :)
   */
   if(ret_val < 0)
      return(ret_val);

   ber_element->total_len = 1 + size_len + data_len;
   ber_element->data_len = data_len;
   ber_element->data.data_ptr = cursor + size_len;

   /* Return the number of data bytes available */
   if(cursor + size_len + data_len >= end_of_payload)
      return(end_of_payload - ber_element->data.data_ptr);
   else
      return(data_len);
}

