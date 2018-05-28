/*
 * Support functions to read little-endian or big-endian binary data
 * 
 * Copyright (C) 2007 Sourcefire, Inc. All Rights Reserved
 * 
 * Writen by Monica Sojeong Hong <shong@sourcefire.com>
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

#include "so-util.h"

uint64_t read_big_64(const uint8_t *p)
{
   uint64_t ret = 0;
   
   ret  = ((uint64_t)*p++) << 56;
   ret |= ((uint64_t)*p++) << 48;
   ret |= ((uint64_t)*p++) << 40;
   ret |= ((uint64_t)*p++) << 32;
   ret |= ((uint64_t)*p++) << 24;
   ret |= *p++ << 16;
   ret |= *p++ << 8;
   ret |= *p;

   return(ret);
}

uint32_t read_big_32(const uint8_t *p)
{
   uint32_t ret = 0;

   ret  = *p++ << 24;
   ret |= *p++ << 16;
   ret |= *p++ << 8;
   ret |= *p;

   return(ret);
}

uint16_t read_big_16(const uint8_t *p)
{
    return (*p << 8) | *(p+1);
}

uint64_t read_little_64(const uint8_t *p)
{
   uint64_t ret = 0;

   ret  = *p++;
   ret |= *p++ << 8;
   ret |= *p++ << 16;
   ret |= ((uint64_t)*p++) << 24;
   ret |= ((uint64_t)*p++) << 32;
   ret |= ((uint64_t)*p++) << 40;
   ret |= ((uint64_t)*p++) << 48;
   ret |= ((uint64_t)*p) << 56;

   return(ret);
}

uint32_t read_little_32(const uint8_t *p)
{
   uint32_t ret = 0;

   ret  = *p++;
   ret |= *p++ << 8;
   ret |= *p++ << 16;
   ret |= *p << 24;

   return(ret);
}

uint16_t read_little_16(const uint8_t *p)
{
    return (*(p+1) << 8) | *p;
}
